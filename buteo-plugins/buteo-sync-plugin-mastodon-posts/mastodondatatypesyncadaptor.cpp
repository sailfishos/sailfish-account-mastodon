/****************************************************************************
 **
 ** Copyright (C) 2013-2026 Jolla Ltd.
 **
 ** This program/library is free software; you can redistribute it and/or
 ** modify it under the terms of the GNU Lesser General Public License
 ** version 2.1 as published by the Free Software Foundation.
 **
 ** This program/library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 ** Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public
 ** License along with this program/library; if not, write to the Free
 ** Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 ** 02110-1301 USA
 **
 ****************************************************************************/

#include "mastodondatatypesyncadaptor.h"
#include "mastodonauthutils.h"
#include "trace.h"

#include <QtCore/QVariantMap>
#include <QtNetwork/QNetworkRequest>

// libaccounts-qt5
#include <Accounts/Manager>
#include <Accounts/Account>
#include <Accounts/Service>
#include <Accounts/AccountService>

// libsignon-qt5
#include <SignOn/Identity>
#include <SignOn/AuthSession>
#include <SignOn/SessionData>

MastodonDataTypeSyncAdaptor::MastodonDataTypeSyncAdaptor(
        SocialNetworkSyncAdaptor::DataType dataType,
        QObject *parent)
    : SocialNetworkSyncAdaptor(QStringLiteral("mastodon"), dataType, 0, parent)
{
}

MastodonDataTypeSyncAdaptor::~MastodonDataTypeSyncAdaptor()
{
}

void MastodonDataTypeSyncAdaptor::sync(const QString &dataTypeString, int accountId)
{
    if (dataTypeString != SocialNetworkSyncAdaptor::dataTypeName(m_dataType)) {
        qCWarning(lcSocialPlugin) << "Mastodon" << SocialNetworkSyncAdaptor::dataTypeName(m_dataType)
                                  << "sync adaptor was asked to sync" << dataTypeString;
        setStatus(SocialNetworkSyncAdaptor::Error);
        return;
    }

    setStatus(SocialNetworkSyncAdaptor::Busy);
    updateDataForAccount(accountId);
    qCDebug(lcSocialPlugin) << "successfully triggered sync with profile:" << m_accountSyncProfile->name();
}

void MastodonDataTypeSyncAdaptor::updateDataForAccount(int accountId)
{
    Accounts::Account *account = Accounts::Account::fromId(m_accountManager, accountId, this);
    if (!account) {
        qCWarning(lcSocialPlugin) << "existing account with id" << accountId << "couldn't be retrieved";
        setStatus(SocialNetworkSyncAdaptor::Error);
        return;
    }

    incrementSemaphore(accountId);
    signIn(account);
}

QString MastodonDataTypeSyncAdaptor::apiHost(int accountId) const
{
    return m_apiHosts.value(accountId, QStringLiteral("https://mastodon.social"));
}

void MastodonDataTypeSyncAdaptor::errorHandler(QNetworkReply::NetworkError err)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }

    const int accountId = reply->property("accountId").toInt();
    const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    qCWarning(lcSocialPlugin) << SocialNetworkSyncAdaptor::dataTypeName(m_dataType)
                              << "request with account" << accountId
                              << "experienced error:" << err
                              << "HTTP:" << httpStatus;

    reply->setProperty("isError", QVariant::fromValue<bool>(true));

    if (httpStatus == 401 || err == QNetworkReply::AuthenticationRequiredError) {
        Accounts::Account *account = Accounts::Account::fromId(m_accountManager, accountId, this);
        if (account) {
            setCredentialsNeedUpdate(account);
        }
    }
}

void MastodonDataTypeSyncAdaptor::sslErrorsHandler(const QList<QSslError> &errs)
{
    QString sslerrs;
    foreach (const QSslError &e, errs) {
        sslerrs += e.errorString() + QLatin1String("; ");
    }
    if (!sslerrs.isEmpty()) {
        sslerrs.chop(2);
    }

    qCWarning(lcSocialPlugin) << SocialNetworkSyncAdaptor::dataTypeName(m_dataType)
                              << "request with account" << sender()->property("accountId").toInt()
                              << "experienced ssl errors:" << sslerrs;
    sender()->setProperty("isError", QVariant::fromValue<bool>(true));
}

void MastodonDataTypeSyncAdaptor::setCredentialsNeedUpdate(Accounts::Account *account)
{
    qCInfo(lcSocialPlugin) << "sociald:Mastodon: setting CredentialsNeedUpdate to true for account:" << account->id();
    Accounts::Service srv(m_accountManager->service(syncServiceName()));
    account->selectService(srv);
    account->setValue(QStringLiteral("CredentialsNeedUpdate"), QVariant::fromValue<bool>(true));
    account->setValue(QStringLiteral("CredentialsNeedUpdateFrom"), QVariant::fromValue<QString>(QString::fromLatin1("sociald-mastodon")));
    account->selectService(Accounts::Service());
    account->syncAndBlock();
}

void MastodonDataTypeSyncAdaptor::signIn(Accounts::Account *account)
{
    const int accountId = account->id();
    if (!checkAccount(account)) {
        decrementSemaphore(accountId);
        return;
    }

    Accounts::Service srv(m_accountManager->service(syncServiceName()));
    account->selectService(srv);
    SignOn::Identity *identity = account->credentialsId() > 0
            ? SignOn::Identity::existingIdentity(account->credentialsId())
            : 0;
    if (!identity) {
        qCWarning(lcSocialPlugin) << "account" << accountId << "has no valid credentials, cannot sign in";
        decrementSemaphore(accountId);
        return;
    }

    Accounts::AccountService accSrv(account, srv);
    const QString method = accSrv.authData().method();
    const QString mechanism = accSrv.authData().mechanism();
    SignOn::AuthSession *session = identity->createSession(method);
    if (!session) {
        qCWarning(lcSocialPlugin) << "could not create signon session for account" << accountId;
        identity->deleteLater();
        decrementSemaphore(accountId);
        return;
    }

    QVariantMap signonSessionData = accSrv.authData().parameters();
    MastodonAuthUtils::addSignOnSessionParameters(account, &signonSessionData);

    connect(session, SIGNAL(response(SignOn::SessionData)),
            this, SLOT(signOnResponse(SignOn::SessionData)),
            Qt::UniqueConnection);
    connect(session, SIGNAL(error(SignOn::Error)),
            this, SLOT(signOnError(SignOn::Error)),
            Qt::UniqueConnection);

    session->setProperty("account", QVariant::fromValue<Accounts::Account*>(account));
    session->setProperty("identity", QVariant::fromValue<SignOn::Identity*>(identity));
    session->process(SignOn::SessionData(signonSessionData), mechanism);
}

void MastodonDataTypeSyncAdaptor::signOnError(const SignOn::Error &error)
{
    SignOn::AuthSession *session = qobject_cast<SignOn::AuthSession*>(sender());
    Accounts::Account *account = session->property("account").value<Accounts::Account*>();
    SignOn::Identity *identity = session->property("identity").value<SignOn::Identity*>();
    const int accountId = account->id();

    qCWarning(lcSocialPlugin) << "credentials for account with id" << accountId
                              << "couldn't be retrieved:" << error.type() << error.message();

    if (error.type() == SignOn::Error::UserInteraction) {
        setCredentialsNeedUpdate(account);
    }

    session->disconnect(this);
    identity->destroySession(session);
    identity->deleteLater();
    account->deleteLater();

    setStatus(SocialNetworkSyncAdaptor::Error);
    decrementSemaphore(accountId);
}

void MastodonDataTypeSyncAdaptor::signOnResponse(const SignOn::SessionData &responseData)
{
    const QVariantMap data = MastodonAuthUtils::responseDataToMap(responseData);

    QString accessToken;
    SignOn::AuthSession *session = qobject_cast<SignOn::AuthSession*>(sender());
    Accounts::Account *account = session->property("account").value<Accounts::Account*>();
    SignOn::Identity *identity = session->property("identity").value<SignOn::Identity*>();
    const int accountId = account->id();

    accessToken = MastodonAuthUtils::accessToken(data);
    if (accessToken.isEmpty()) {
        qCWarning(lcSocialPlugin) << "signon response for account with id" << accountId
                                  << "contained no access token; keys:" << data.keys();
    }

    m_apiHosts.insert(accountId, MastodonAuthUtils::normalizeApiHost(account->value(QStringLiteral("api/Host")).toString()));

    session->disconnect(this);
    identity->destroySession(session);
    identity->deleteLater();
    account->deleteLater();

    if (!accessToken.isEmpty()) {
        beginSync(accountId, accessToken);
    } else {
        setStatus(SocialNetworkSyncAdaptor::Error);
    }

    decrementSemaphore(accountId);
}
