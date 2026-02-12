/*
 * Copyright (C) 2013-2026 Jolla Ltd.
 */

#include "mastodonshareservicestatus.h"
#include "mastodonauthutils.h"

#include <Accounts/Account>
#include <Accounts/AccountService>
#include <Accounts/Manager>
#include <Accounts/Service>

#include <SignOn/AuthSession>
#include <SignOn/Error>
#include <SignOn/Identity>
#include <SignOn/SessionData>

#include <QtCore/QUrl>
#include <QtCore/QVariantMap>
#include <QtDebug>

MastodonShareServiceStatus::MastodonShareServiceStatus(QObject *parent)
    : QObject(parent)
    , m_auth(new AccountAuthenticator(this))
    , m_accountManager(new Accounts::Manager(this))
    , m_serviceName(QStringLiteral("mastodon-sharing"))
{
}

void MastodonShareServiceStatus::signIn(int accountId)
{
    Accounts::Account *account = Accounts::Account::fromId(m_accountManager, accountId, this);
    if (!account) {
        qWarning() << Q_FUNC_INFO << "Failed to retrieve account for id:" << accountId;
        setAccountDetailsState(accountId, Error);
        return;
    }

    const Accounts::Service service(m_accountManager->service(m_serviceName));
    if (!service.isValid()) {
        qWarning() << Q_FUNC_INFO << "Invalid auth service" << m_serviceName;
        account->deleteLater();
        setAccountDetailsState(accountId, Error);
        return;
    }

    account->selectService(service);

    SignOn::Identity *identity = account->credentialsId() > 0
            ? SignOn::Identity::existingIdentity(account->credentialsId())
            : 0;
    if (!identity) {
        qWarning() << Q_FUNC_INFO << "account" << accountId << "has no valid credentials";
        account->deleteLater();
        setAccountDetailsState(accountId, Error);
        return;
    }

    Accounts::AccountService accountService(account, service);
    const QString method = accountService.authData().method();
    const QString mechanism = accountService.authData().mechanism();

    SignOn::AuthSession *session = identity->createSession(method);
    if (!session) {
        qWarning() << Q_FUNC_INFO << "could not create signon session for account" << accountId;
        identity->deleteLater();
        account->deleteLater();
        setAccountDetailsState(accountId, Error);
        return;
    }

    QVariantMap signonSessionData = accountService.authData().parameters();

    MastodonAuthUtils::addSignOnSessionParameters(account, &signonSessionData);

    connect(session, SIGNAL(response(SignOn::SessionData)),
            this, SLOT(signOnResponse(SignOn::SessionData)),
            Qt::UniqueConnection);
    connect(session, SIGNAL(error(SignOn::Error)),
            this, SLOT(signOnError(SignOn::Error)),
            Qt::UniqueConnection);

    session->setProperty("account", QVariant::fromValue<Accounts::Account *>(account));
    session->setProperty("identity", QVariant::fromValue<SignOn::Identity *>(identity));
    session->process(SignOn::SessionData(signonSessionData), mechanism);
}

void MastodonShareServiceStatus::signOnResponse(const SignOn::SessionData &responseData)
{
    const QVariantMap data = MastodonAuthUtils::responseDataToMap(responseData);

    SignOn::AuthSession *session = qobject_cast<SignOn::AuthSession *>(sender());
    Accounts::Account *account = session->property("account").value<Accounts::Account *>();
    SignOn::Identity *identity = session->property("identity").value<SignOn::Identity *>();
    const int accountId = account ? account->id() : 0;

    QString accessToken = MastodonAuthUtils::accessToken(data);

    if (accountId > 0 && m_accountIdToDetailsIdx.contains(accountId)) {
        AccountDetails &accountDetails(m_accountDetails[m_accountIdToDetailsIdx[accountId]]);
        accountDetails.accessToken = accessToken;
        setAccountDetailsState(accountId, accessToken.isEmpty() ? Error : Populated);
    }

    session->disconnect(this);
    if (identity) {
        identity->destroySession(session);
        identity->deleteLater();
    }
    if (account) {
        account->deleteLater();
    }
}

void MastodonShareServiceStatus::signOnError(const SignOn::Error &error)
{
    SignOn::AuthSession *session = qobject_cast<SignOn::AuthSession *>(sender());
    Accounts::Account *account = session->property("account").value<Accounts::Account *>();
    SignOn::Identity *identity = session->property("identity").value<SignOn::Identity *>();
    const int accountId = account ? account->id() : 0;

    qWarning() << Q_FUNC_INFO << "failed to retrieve credentials for account" << accountId
               << error.type() << error.message();

    if (accountId > 0 && error.type() == SignOn::Error::UserInteraction) {
        setCredentialsNeedUpdate(accountId, m_serviceName);
    }

    session->disconnect(this);
    if (identity) {
        identity->destroySession(session);
        identity->deleteLater();
    }
    if (account) {
        account->deleteLater();
    }

    if (accountId > 0) {
        setAccountDetailsState(accountId, Error);
    }
}

void MastodonShareServiceStatus::setAccountDetailsState(int accountId, AccountDetailsState state)
{
    if (!m_accountIdToDetailsIdx.contains(accountId)) {
        return;
    }

    m_accountDetailsState[accountId] = state;

    bool anyWaiting = false;
    bool anyPopulated = false;
    Q_FOREACH (int id, m_accountDetailsState.keys()) {
        AccountDetailsState accountState = m_accountDetailsState.value(id, Waiting);
        if (accountState == Waiting) {
            anyWaiting = true;
        } else if (accountState == Populated) {
            anyPopulated = true;
        }
    }

    if (!anyWaiting) {
        if (anyPopulated) {
            emit serviceReady();
        } else {
            emit serviceError(QStringLiteral("Unable to retrieve Mastodon account credentials"));
        }
    }
}

int MastodonShareServiceStatus::count() const
{
    return m_accountDetails.count();
}

bool MastodonShareServiceStatus::setCredentialsNeedUpdate(int accountId, const QString &serviceName)
{
    return m_auth->setCredentialsNeedUpdate(accountId, serviceName);
}

void MastodonShareServiceStatus::queryStatus(QueryStatusMode mode)
{
    m_accountDetails.clear();
    m_accountIdToDetailsIdx.clear();
    m_accountDetailsState.clear();

    bool signInActive = false;
    Q_FOREACH (Accounts::AccountId id, m_accountManager->accountList()) {
        Accounts::Account *acc = m_accountManager->account(id);

        if (!acc) {
            qWarning() << Q_FUNC_INFO << "Failed to get account for id:" << id;
            continue;
        }

        acc->selectService(Accounts::Service());

        const Accounts::Service service(m_accountManager->service(m_serviceName));
        const Accounts::ServiceList services = acc->services();
        bool serviceFound = false;
        Q_FOREACH (const Accounts::Service &s, services) {
            if (s.name() == m_serviceName) {
                serviceFound = true;
                break;
            }
        }

        if (!service.isValid() || !serviceFound) {
            continue;
        }

        const bool accountEnabled = acc->enabled();
        acc->selectService(service);
        const bool shareServiceEnabled = acc->enabled();
        if (!accountEnabled || !shareServiceEnabled) {
            acc->selectService(Accounts::Service());
            continue;
        }

        if (acc->value(QStringLiteral("CredentialsNeedUpdate")).toBool()) {
            qWarning() << Q_FUNC_INFO << "Credentials need update for account id:" << id;
            acc->selectService(Accounts::Service());
            continue;
        }

        if (!m_accountIdToDetailsIdx.contains(id)) {
            AccountDetails details;
            details.accountId = id;
            details.apiHost = MastodonAuthUtils::normalizeApiHost(acc->value(QStringLiteral("api/Host")).toString());

            QUrl apiUrl(details.apiHost);
            details.providerName = apiUrl.host();
            if (details.providerName.isEmpty()) {
                details.providerName = details.apiHost;
                if (details.providerName.startsWith(QLatin1String("https://"))) {
                    details.providerName.remove(0, 8);
                } else if (details.providerName.startsWith(QLatin1String("http://"))) {
                    details.providerName.remove(0, 7);
                }
                const int separator = details.providerName.indexOf(QLatin1Char('/'));
                if (separator > 0) {
                    details.providerName.truncate(separator);
                }
            }

            details.displayName = acc->displayName();

            m_accountIdToDetailsIdx.insert(id, m_accountDetails.size());
            m_accountDetails.append(details);
        }

        if (mode == SignInMode) {
            signInActive = true;
            m_accountDetailsState.insert(id, Waiting);
            signIn(id);
        }

        acc->selectService(Accounts::Service());
    }

    if (!signInActive) {
        emit serviceReady();
    }
}

MastodonShareServiceStatus::AccountDetails MastodonShareServiceStatus::details(int index) const
{
    if (index < 0 || index >= m_accountDetails.size()) {
        qWarning() << Q_FUNC_INFO << "Index out of range";
        return AccountDetails();
    }

    return m_accountDetails.at(index);
}

MastodonShareServiceStatus::AccountDetails MastodonShareServiceStatus::detailsByIdentifier(int accountIdentifier) const
{
    if (!m_accountIdToDetailsIdx.contains(accountIdentifier)) {
        qWarning() << Q_FUNC_INFO << "No details known for account with identifier" << accountIdentifier;
        return AccountDetails();
    }

    return m_accountDetails[m_accountIdToDetailsIdx[accountIdentifier]];
}
