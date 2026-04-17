// SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
//
// SPDX-License-Identifier: BSD-3-Clause

#include "mastodonpostactions.h"

#include "mastodonauthutils.h"

#include <Accounts/Account>
#include <Accounts/AccountService>
#include <Accounts/Manager>
#include <Accounts/Service>

#include <SignOn/AuthSession>
#include <SignOn/Error>
#include <SignOn/Identity>
#include <SignOn/SessionData>

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QVariantMap>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtDebug>

namespace {
    const char *const MicroblogServiceName = "mastodon-microblog";
}

MastodonPostActions::MastodonPostActions(QObject *parent)
    : QObject(parent)
    , m_accountManager(new Accounts::Manager(this))
{
}

void MastodonPostActions::favourite(int accountId, const QString &statusId)
{
    performAction(accountId, statusId, QStringLiteral("favourite"));
}

void MastodonPostActions::unfavourite(int accountId, const QString &statusId)
{
    performAction(accountId, statusId, QStringLiteral("unfavourite"));
}

void MastodonPostActions::boost(int accountId, const QString &statusId)
{
    performAction(accountId, statusId, QStringLiteral("reblog"));
}

void MastodonPostActions::unboost(int accountId, const QString &statusId)
{
    performAction(accountId, statusId, QStringLiteral("unreblog"));
}

void MastodonPostActions::performAction(int accountId, const QString &statusId, const QString &action)
{
    const QString trimmedStatusId = statusId.trimmed();
    if (accountId <= 0 || trimmedStatusId.isEmpty() || action.isEmpty()) {
        emit actionFailed(accountId, trimmedStatusId, action, QStringLiteral("Invalid action request"));
        return;
    }

    const QString key = actionKey(accountId, trimmedStatusId, action);
    if (m_pendingActions.contains(key)) {
        return;
    }

    Accounts::Account *account = Accounts::Account::fromId(m_accountManager, accountId, this);
    if (!account) {
        emit actionFailed(accountId, trimmedStatusId, action, QStringLiteral("Unable to load account"));
        return;
    }

    const Accounts::Service service(m_accountManager->service(QString::fromLatin1(MicroblogServiceName)));
    if (!service.isValid()) {
        account->deleteLater();
        emit actionFailed(accountId, trimmedStatusId, action, QStringLiteral("Invalid account service"));
        return;
    }

    account->selectService(service);
    SignOn::Identity *identity = account->credentialsId() > 0
            ? SignOn::Identity::existingIdentity(account->credentialsId())
            : 0;
    if (!identity) {
        account->deleteLater();
        emit actionFailed(accountId, trimmedStatusId, action, QStringLiteral("Missing account credentials"));
        return;
    }

    Accounts::AccountService accountService(account, service);
    const QString method = accountService.authData().method();
    const QString mechanism = accountService.authData().mechanism();
    SignOn::AuthSession *session = identity->createSession(method);
    if (!session) {
        identity->deleteLater();
        account->deleteLater();
        emit actionFailed(accountId, trimmedStatusId, action, QStringLiteral("Unable to create auth session"));
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
    session->setProperty("action", action);
    session->setProperty("statusId", trimmedStatusId);
    session->setProperty("accountId", accountId);

    m_pendingActions.insert(key);
    session->process(SignOn::SessionData(signonSessionData), mechanism);
}

void MastodonPostActions::signOnResponse(const SignOn::SessionData &responseData)
{
    QObject *sessionObject = sender();
    SignOn::AuthSession *session = qobject_cast<SignOn::AuthSession *>(sessionObject);
    if (!session) {
        return;
    }

    const int accountId = session->property("accountId").toInt();
    const QString statusId = session->property("statusId").toString();
    const QString action = session->property("action").toString();
    const QString key = actionKey(accountId, statusId, action);

    const QVariantMap data = MastodonAuthUtils::responseDataToMap(responseData);
    const QString accessToken = MastodonAuthUtils::accessToken(data);

    Accounts::Account *account = session->property("account").value<Accounts::Account *>();
    const QString apiHost = account
            ? MastodonAuthUtils::normalizeApiHost(account->value(QStringLiteral("api/Host")).toString())
            : QString();

    if (accessToken.isEmpty() || apiHost.isEmpty()) {
        m_pendingActions.remove(key);
        emit actionFailed(accountId, statusId, action, QStringLiteral("Missing access token"));
        releaseSignOnObjects(sessionObject);
        return;
    }

    releaseSignOnObjects(sessionObject);
    executeActionRequest(accountId, statusId, action, apiHost, accessToken);
}

void MastodonPostActions::signOnError(const SignOn::Error &error)
{
    QObject *sessionObject = sender();
    SignOn::AuthSession *session = qobject_cast<SignOn::AuthSession *>(sessionObject);
    if (!session) {
        return;
    }

    const int accountId = session->property("accountId").toInt();
    const QString statusId = session->property("statusId").toString();
    const QString action = session->property("action").toString();
    const QString key = actionKey(accountId, statusId, action);
    m_pendingActions.remove(key);

    emit actionFailed(accountId, statusId, action, error.message());
    releaseSignOnObjects(sessionObject);
}

void MastodonPostActions::executeActionRequest(int accountId,
                                               const QString &statusId,
                                               const QString &action,
                                               const QString &apiHost,
                                               const QString &accessToken)
{
    const QString encodedStatusId = QString::fromLatin1(QUrl::toPercentEncoding(statusId));
    QUrl url(apiHost + QStringLiteral("/api/v1/statuses/")
             + encodedStatusId + QStringLiteral("/") + action);

    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QStringLiteral("Bearer %1").arg(accessToken).toUtf8());

    QNetworkReply *reply = m_networkAccessManager.post(request, QByteArray());
    if (!reply) {
        const QString key = actionKey(accountId, statusId, action);
        m_pendingActions.remove(key);
        emit actionFailed(accountId, statusId, action, QStringLiteral("Failed to start request"));
        return;
    }

    reply->setProperty("accountId", accountId);
    reply->setProperty("statusId", statusId);
    reply->setProperty("action", action);
    connect(reply, SIGNAL(finished()), this, SLOT(actionFinishedHandler()));
}

void MastodonPostActions::actionFinishedHandler()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }

    const int accountId = reply->property("accountId").toInt();
    const QString statusId = reply->property("statusId").toString();
    const QString action = reply->property("action").toString();
    const QString key = actionKey(accountId, statusId, action);

    const QByteArray data = reply->readAll();
    const bool hasError = reply->error() != QNetworkReply::NoError;
    const QString errorString = reply->errorString();
    reply->deleteLater();

    m_pendingActions.remove(key);

    if (hasError) {
        emit actionFailed(accountId, statusId, action, errorString);
        return;
    }

    int favouritesCount = -1;
    int reblogsCount = -1;
    bool favourited = false;
    bool reblogged = false;

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error == QJsonParseError::NoError && document.isObject()) {
        const QJsonObject object = document.object();
        QJsonObject metricsObject = object;

        const auto jsonObjectId = [](const QJsonObject &obj) -> QString {
            return obj.value(QStringLiteral("id")).toVariant().toString().trimmed();
        };
        const QString requestedStatusId = statusId.trimmed();

        const QJsonValue reblogValue = object.value(QStringLiteral("reblog"));
        if (reblogValue.isObject() && !reblogValue.isNull()) {
            const QJsonObject reblogObject = reblogValue.toObject();
            const QString nestedReblogId = jsonObjectId(reblogObject);

            if (nestedReblogId == requestedStatusId) {
                metricsObject = reblogObject;
            }
        }

        favouritesCount = metricsObject.value(QStringLiteral("favourites_count")).toInt(-1);
        if (favouritesCount < 0) {
            favouritesCount = object.value(QStringLiteral("favourites_count")).toInt(-1);
        }

        reblogsCount = metricsObject.value(QStringLiteral("reblogs_count")).toInt(-1);
        if (reblogsCount < 0) {
            reblogsCount = object.value(QStringLiteral("reblogs_count")).toInt(-1);
        }

        if (metricsObject.contains(QStringLiteral("favourited"))) {
            favourited = metricsObject.value(QStringLiteral("favourited")).toBool(false);
        } else {
            favourited = object.value(QStringLiteral("favourited")).toBool(false);
        }

        if (metricsObject.contains(QStringLiteral("reblogged"))) {
            reblogged = metricsObject.value(QStringLiteral("reblogged")).toBool(false);
        } else {
            reblogged = object.value(QStringLiteral("reblogged")).toBool(false);
        }

    }

    emit actionSucceeded(accountId, statusId, action,
                         favouritesCount, reblogsCount,
                         favourited, reblogged);
}

void MastodonPostActions::releaseSignOnObjects(QObject *sessionObject)
{
    SignOn::AuthSession *session = qobject_cast<SignOn::AuthSession *>(sessionObject);
    if (!session) {
        return;
    }

    Accounts::Account *account = session->property("account").value<Accounts::Account *>();
    SignOn::Identity *identity = session->property("identity").value<SignOn::Identity *>();

    session->disconnect(this);
    if (identity) {
        identity->destroySession(session);
        identity->deleteLater();
    }
    if (account) {
        account->deleteLater();
    }
}

QString MastodonPostActions::actionKey(int accountId, const QString &statusId, const QString &action) const
{
    return QString::number(accountId) + QLatin1Char(':') + statusId + QLatin1Char(':') + action;
}
