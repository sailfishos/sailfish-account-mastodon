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

#include "mastodonnotificationssyncadaptor.h"
#include "mastodontextutils.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QLoggingCategory>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QtNetwork/QNetworkRequest>

// libaccounts-qt5
#include <Accounts/Account>
#include <Accounts/Manager>
#include <Accounts/Service>

#include <notification.h>

#include <algorithm>

#define OPEN_URL_ACTION(openUrl)            \
    Notification::remoteAction(             \
        "default",                          \
        "",                                 \
        "org.sailfishos.fileservice",       \
        "/",                                \
        "org.sailfishos.fileservice",       \
        "openUrl",                          \
        QVariantList() << openUrl           \
    )

namespace {
    Q_LOGGING_CATEGORY(lcMastodonNotifications, "buteo.plugin.mastodon.notifications", QtWarningMsg)

    const char *const NotificationCategory = "x-nemo.social.mastodon.notification";
    const char *const NotificationIdHint = "x-nemo.sociald.notification-id";
    const char *const LastFetchedNotificationIdKey = "LastFetchedNotificationId";
    const int NotificationsPageLimit = 80;

    //% "mentioned you"
    const char *const TrIdMentionedYou = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-mentioned_you");
    //% "boosted your post"
    const char *const TrIdBoostedYourPost = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-boosted_your_post");
    //% "favourited your post"
    const char *const TrIdFavouritedYourPost = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-favourited_your_post");
    //% "started following you"
    const char *const TrIdStartedFollowingYou = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-started_following_you");
    //% "requested to follow you"
    const char *const TrIdRequestedToFollowYou = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-requested_to_follow_you");
    //% "interacted with your poll"
    const char *const TrIdInteractedWithYourPoll = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-interacted_with_your_poll");
    //% "posted"
    const char *const TrIdPosted = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-posted");
    //% "updated a post"
    const char *const TrIdUpdatedPost = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-updated_post");
    //% "signed up"
    const char *const TrIdSignedUp = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-signed_up");
    //% "reported an account"
    const char *const TrIdReportedAccount = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-reported_account");
    //% "received a moderation warning"
    const char *const TrIdReceivedModerationWarning = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-received_moderation_warning");
    //% "quoted your post"
    const char *const TrIdQuotedYourPost = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-quoted_your_post");
    //% "updated a post that quoted you"
    const char *const TrIdUpdatedQuotedPost = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-updated_quoted_post");
    //% "sent you a notification"
    const char *const TrIdSentNotification = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-sent_notification");

    //% "An admin blocked an instance"
    const char *const TrIdAdminBlockedInstance = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-admin_blocked_instance");
    //% "An admin blocked %1"
    const char *const TrIdAdminBlockedTarget = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-admin_blocked_target");
    //% "You blocked an instance"
    const char *const TrIdYouBlockedInstance = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-you_blocked_instance");
    //% "You blocked %1"
    const char *const TrIdYouBlockedTarget = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-you_blocked_target");
    //% "An account was suspended"
    const char *const TrIdAccountSuspended = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-account_suspended");
    //% "%1 was suspended"
    const char *const TrIdTargetSuspended = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-target_suspended");
    //% "Some follow relationships were severed"
    const char *const TrIdRelationshipsSevered = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-relationships_severed");
    //% "%1 (%2 followers, %3 following removed)"
    const char *const TrIdRelationshipsSummary = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-relationships_summary");

    //% "A moderator sent you a warning"
    const char *const TrIdModeratorWarningNone = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-moderator_warning_none");
    //% "A moderator disabled your account"
    const char *const TrIdModeratorWarningDisable = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-moderator_warning_disable");
    //% "A moderator marked specific posts as sensitive"
    const char *const TrIdModeratorWarningSpecificSensitive = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-moderator_warning_specific_sensitive");
    //% "A moderator deleted specific posts"
    const char *const TrIdModeratorWarningDeletePosts = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-moderator_warning_delete_posts");
    //% "A moderator marked all your posts as sensitive"
    const char *const TrIdModeratorWarningAllSensitive = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-moderator_warning_all_sensitive");
    //% "A moderator limited your account"
    const char *const TrIdModeratorWarningSilence = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-moderator_warning_silence");
    //% "A moderator suspended your account"
    const char *const TrIdModeratorWarningSuspend = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-moderator_warning_suspend");

    //% "Mastodon"
    const char *const TrIdMastodon = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-mastodon");
    //% "New notification"
    const char *const TrIdNewNotification = QT_TRID_NOOP("lipstick-jolla-home-la-mastodon-notification-new_notification");

    QString displayNameForAccount(const QJsonObject &account)
    {
        const QString displayName = account.value(QStringLiteral("display_name")).toString().trimmed();
        if (!displayName.isEmpty()) {
            return displayName;
        }

        const QString username = account.value(QStringLiteral("username")).toString().trimmed();
        if (!username.isEmpty()) {
            return username;
        }

        return account.value(QStringLiteral("acct")).toString().trimmed();
    }

    QString actionText(const QString &type)
    {
        if (type == QLatin1String("mention")) {
            return qtTrId(TrIdMentionedYou);
        } else if (type == QLatin1String("reblog")) {
            return qtTrId(TrIdBoostedYourPost);
        } else if (type == QLatin1String("favourite")) {
            return qtTrId(TrIdFavouritedYourPost);
        } else if (type == QLatin1String("follow")) {
            return qtTrId(TrIdStartedFollowingYou);
        } else if (type == QLatin1String("follow_request")) {
            return qtTrId(TrIdRequestedToFollowYou);
        } else if (type == QLatin1String("poll")) {
            return qtTrId(TrIdInteractedWithYourPoll);
        } else if (type == QLatin1String("status")) {
            return qtTrId(TrIdPosted);
        } else if (type == QLatin1String("update")) {
            return qtTrId(TrIdUpdatedPost);
        } else if (type == QLatin1String("admin.sign_up")) {
            return qtTrId(TrIdSignedUp);
        } else if (type == QLatin1String("admin.report")) {
            return qtTrId(TrIdReportedAccount);
        } else if (type == QLatin1String("moderation_warning")) {
            return qtTrId(TrIdReceivedModerationWarning);
        } else if (type == QLatin1String("quote")) {
            return qtTrId(TrIdQuotedYourPost);
        } else if (type == QLatin1String("quoted_update")) {
            return qtTrId(TrIdUpdatedQuotedPost);
        }

        return qtTrId(TrIdSentNotification);
    }

    bool useSystemSummary(const QString &notificationType)
    {
        return notificationType == QLatin1String("severed_relationships")
                || notificationType == QLatin1String("moderation_warning");
    }

    QString severedRelationshipsText(const QJsonObject &eventObject)
    {
        const QString eventType = eventObject.value(QStringLiteral("type")).toString();
        const QString targetName = eventObject.value(QStringLiteral("target_name")).toString().trimmed();
        const int followersCount = eventObject.value(QStringLiteral("followers_count")).toInt();
        const int followingCount = eventObject.value(QStringLiteral("following_count")).toInt();

        QString action;
        if (eventType == QLatin1String("domain_block")) {
            action = targetName.isEmpty()
                    ? qtTrId(TrIdAdminBlockedInstance)
                    : qtTrId(TrIdAdminBlockedTarget).arg(targetName);
        } else if (eventType == QLatin1String("user_domain_block")) {
            action = targetName.isEmpty()
                    ? qtTrId(TrIdYouBlockedInstance)
                    : qtTrId(TrIdYouBlockedTarget).arg(targetName);
        } else if (eventType == QLatin1String("account_suspension")) {
            action = targetName.isEmpty()
                    ? qtTrId(TrIdAccountSuspended)
                    : qtTrId(TrIdTargetSuspended).arg(targetName);
        } else {
            action = qtTrId(TrIdRelationshipsSevered);
        }

        const int affectedCount = followersCount + followingCount;
        if (affectedCount <= 0) {
            return action;
        }

        return qtTrId(TrIdRelationshipsSummary)
                .arg(action)
                .arg(followersCount)
                .arg(followingCount);
    }

    QString moderationWarningText(const QJsonObject &warningObject)
    {
        const QString warningText = warningObject.value(QStringLiteral("text")).toString().trimmed();
        if (!warningText.isEmpty()) {
            return warningText;
        }

        const QString action = warningObject.value(QStringLiteral("action")).toString();
        if (action == QLatin1String("none")) {
            return qtTrId(TrIdModeratorWarningNone);
        } else if (action == QLatin1String("disable")) {
            return qtTrId(TrIdModeratorWarningDisable);
        } else if (action == QLatin1String("mark_statuses_as_sensitive")) {
            return qtTrId(TrIdModeratorWarningSpecificSensitive);
        } else if (action == QLatin1String("delete_statuses")) {
            return qtTrId(TrIdModeratorWarningDeletePosts);
        } else if (action == QLatin1String("sensitive")) {
            return qtTrId(TrIdModeratorWarningAllSensitive);
        } else if (action == QLatin1String("silence")) {
            return qtTrId(TrIdModeratorWarningSilence);
        } else if (action == QLatin1String("suspend")) {
            return qtTrId(TrIdModeratorWarningSuspend);
        }

        return QString();
    }

    bool hasActiveNotificationsForAccount(int accountId)
    {
        bool hasActiveNotifications = false;
        const QList<QObject *> notifications = Notification::notifications();
        foreach (QObject *object, notifications) {
            Notification *notification = qobject_cast<Notification *>(object);
            if (notification
                    && notification->category() == QLatin1String(NotificationCategory)
                    && notification->hintValue("x-nemo.sociald.account-id").toInt() == accountId) {
                hasActiveNotifications = true;
            }

            delete object;
        }

        return hasActiveNotifications;
    }

    QString authorizeInteractionUrl(const QString &apiHost, const QString &targetUrl)
    {
        const QUrl parsedApiHost(apiHost);
        const QUrl parsedTargetUrl(targetUrl);
        if (!parsedApiHost.isValid()
                || parsedApiHost.scheme().isEmpty()
                || parsedApiHost.host().isEmpty()
                || !parsedTargetUrl.isValid()
                || parsedTargetUrl.scheme().isEmpty()
                || parsedTargetUrl.host().isEmpty()) {
            return targetUrl;
        }

        // Links on the account's own instance should open directly.
        const bool sameScheme = QString::compare(parsedApiHost.scheme(), parsedTargetUrl.scheme(), Qt::CaseInsensitive) == 0;
        const bool sameHost = QString::compare(parsedApiHost.host(), parsedTargetUrl.host(), Qt::CaseInsensitive) == 0;
        const int apiPort = parsedApiHost.port(parsedApiHost.scheme() == QLatin1String("https") ? 443 : 80);
        const int targetPort = parsedTargetUrl.port(parsedTargetUrl.scheme() == QLatin1String("https") ? 443 : 80);
        if (sameScheme && sameHost && apiPort == targetPort) {
            return targetUrl;
        }

        QUrl authorizeUrl(parsedApiHost);
        authorizeUrl.setPath(QStringLiteral("/authorize_interaction"));
        authorizeUrl.setQuery(QStringLiteral("uri=") + QString::fromUtf8(QUrl::toPercentEncoding(targetUrl)));
        return authorizeUrl.toString();
    }

}

MastodonNotificationsSyncAdaptor::MastodonNotificationsSyncAdaptor(QObject *parent)
    : MastodonNotificationsDataTypeSyncAdaptor(SocialNetworkSyncAdaptor::Notifications, parent)
{
    setInitialActive(true);
}

MastodonNotificationsSyncAdaptor::~MastodonNotificationsSyncAdaptor()
{
}

QString MastodonNotificationsSyncAdaptor::syncServiceName() const
{
    return QStringLiteral("mastodon-notifications");
}

QString MastodonNotificationsSyncAdaptor::authServiceName() const
{
    return QStringLiteral("mastodon-microblog");
}

void MastodonNotificationsSyncAdaptor::purgeDataForOldAccount(int oldId, SocialNetworkSyncAdaptor::PurgeMode)
{
    closeAccountNotifications(oldId);

    m_pendingSyncStates.remove(oldId);
    m_lastMarkedReadIds.remove(oldId);
    saveLastFetchedId(oldId, QString());
}

void MastodonNotificationsSyncAdaptor::beginSync(int accountId, const QString &accessToken)
{
    m_pendingSyncStates.remove(accountId);
    requestUnreadMarker(accountId, accessToken);
}

void MastodonNotificationsSyncAdaptor::finalize(int accountId)
{
    if (syncAborted()) {
        qCInfo(lcMastodonNotifications) << "sync aborted, won't update notifications";
    }

    Q_UNUSED(accountId)
}

QString MastodonNotificationsSyncAdaptor::sanitizeContent(const QString &content)
{
    return MastodonTextUtils::sanitizeContent(content);
}

QDateTime MastodonNotificationsSyncAdaptor::parseTimestamp(const QString &timestampString)
{
    return MastodonTextUtils::parseTimestamp(timestampString);
}

int MastodonNotificationsSyncAdaptor::compareNotificationIds(const QString &left, const QString &right)
{
    if (left == right) {
        return 0;
    }

    bool leftOk = false;
    bool rightOk = false;
    const qulonglong leftValue = left.toULongLong(&leftOk);
    const qulonglong rightValue = right.toULongLong(&rightOk);
    if (leftOk && rightOk) {
        return leftValue < rightValue ? -1 : 1;
    }

    if (left.size() != right.size()) {
        return left.size() < right.size() ? -1 : 1;
    }
    return left < right ? -1 : 1;
}

QString MastodonNotificationsSyncAdaptor::notificationObjectKey(int accountId, const QString &notificationId)
{
    return QString::number(accountId) + QLatin1Char(':') + notificationId;
}

QString MastodonNotificationsSyncAdaptor::loadLastFetchedId(int accountId) const
{
    Accounts::Account *account = Accounts::Account::fromId(m_accountManager, accountId, 0);
    if (!account) {
        return QString();
    }

    Accounts::Service service(m_accountManager->service(syncServiceName()));
    account->selectService(service);
    const QString lastFetchedId = account->value(QString::fromLatin1(LastFetchedNotificationIdKey)).toString().trimmed();
    account->deleteLater();

    return lastFetchedId;
}

void MastodonNotificationsSyncAdaptor::saveLastFetchedId(int accountId, const QString &lastFetchedId)
{
    Accounts::Account *account = Accounts::Account::fromId(m_accountManager, accountId, 0);
    if (!account) {
        return;
    }

    Accounts::Service service(m_accountManager->service(syncServiceName()));
    account->selectService(service);
    const QString storedId = account->value(QString::fromLatin1(LastFetchedNotificationIdKey)).toString().trimmed();
    if (storedId != lastFetchedId) {
        account->setValue(QString::fromLatin1(LastFetchedNotificationIdKey), lastFetchedId);
        account->syncAndBlock();
    }

    account->deleteLater();
}

void MastodonNotificationsSyncAdaptor::requestUnreadMarker(int accountId, const QString &accessToken)
{
    QUrl url(apiHost(accountId) + QStringLiteral("/api/v1/markers"));

    QUrlQuery query(url);
    query.addQueryItem(QStringLiteral("timeline[]"), QStringLiteral("notifications"));
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QStringLiteral("Bearer %1").arg(accessToken).toUtf8());

    QNetworkReply *reply = m_networkAccessManager->get(request);
    if (reply) {
        reply->setProperty("accountId", accountId);
        reply->setProperty("accessToken", accessToken);
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(errorHandler(QNetworkReply::NetworkError)));
        connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrorsHandler(QList<QSslError>)));
        connect(reply, SIGNAL(finished()), this, SLOT(finishedUnreadMarkerHandler()));

        incrementSemaphore(accountId);
        setupReplyTimeout(accountId, reply);
    } else {
        qCWarning(lcMastodonNotifications) << "unable to request notifications marker from Mastodon account with id" << accountId;
    }
}

void MastodonNotificationsSyncAdaptor::finishedUnreadMarkerHandler()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }

    const bool isError = reply->property("isError").toBool();
    const int accountId = reply->property("accountId").toInt();
    const QString accessToken = reply->property("accessToken").toString();
    const QByteArray replyData = reply->readAll();

    disconnect(reply);
    reply->deleteLater();
    removeReplyTimeout(accountId, reply);

    bool ok = false;
    const QJsonObject markerObject = parseJsonObjectReplyData(replyData, &ok);
    if (isError || !ok) {
        qCWarning(lcMastodonNotifications) << "unable to parse notifications marker data from request with account"
                                  << accountId << ", got:" << QString::fromUtf8(replyData);
        PendingSyncState fallbackState;
        fallbackState.accessToken = accessToken;
        fallbackState.lastFetchedId = loadLastFetchedId(accountId);
        m_pendingSyncStates.insert(accountId, fallbackState);
        requestNotifications(accountId, accessToken, fallbackState.lastFetchedId);
        decrementSemaphore(accountId);
        return;
    }

    const QString markerId = markerObject.value(QStringLiteral("notifications"))
            .toObject()
            .value(QStringLiteral("last_read_id"))
            .toVariant()
            .toString()
            .trimmed();

    PendingSyncState state;
    state.accessToken = accessToken;
    state.unreadFloorId = markerId;
    state.lastFetchedId = loadLastFetchedId(accountId);
    if (state.lastFetchedId.isEmpty() && !markerId.isEmpty()) {
        // On first run, use the server unread marker floor to avoid historical flood.
        state.lastFetchedId = markerId;
    }
    m_pendingSyncStates.insert(accountId, state);
    requestNotifications(accountId, accessToken, markerId);

    decrementSemaphore(accountId);
}

void MastodonNotificationsSyncAdaptor::requestNotifications(int accountId,
                                                            const QString &accessToken,
                                                            const QString &minId,
                                                            const QString &maxId)
{
    QUrl url(apiHost(accountId) + QStringLiteral("/api/v1/notifications"));

    QUrlQuery query(url);
    query.addQueryItem(QStringLiteral("limit"), QString::number(NotificationsPageLimit));
    if (!minId.isEmpty()) {
        query.addQueryItem(QStringLiteral("min_id"), minId);
    }
    if (!maxId.isEmpty()) {
        query.addQueryItem(QStringLiteral("max_id"), maxId);
    }
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QStringLiteral("Bearer %1").arg(accessToken).toUtf8());

    QNetworkReply *reply = m_networkAccessManager->get(request);
    if (reply) {
        reply->setProperty("accountId", accountId);
        reply->setProperty("accessToken", accessToken);
        reply->setProperty("minId", minId);
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(errorHandler(QNetworkReply::NetworkError)));
        connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrorsHandler(QList<QSslError>)));
        connect(reply, SIGNAL(finished()), this, SLOT(finishedNotificationsHandler()));

        incrementSemaphore(accountId);
        setupReplyTimeout(accountId, reply);
    } else {
        qCWarning(lcMastodonNotifications) << "unable to request notifications from Mastodon account with id" << accountId;
    }
}

void MastodonNotificationsSyncAdaptor::requestMarkRead(int accountId,
                                                       const QString &accessToken,
                                                       const QString &lastReadId)
{
    QUrl url(apiHost(accountId) + QStringLiteral("/api/v1/markers"));
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QStringLiteral("Bearer %1").arg(accessToken).toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("notifications[last_read_id]"), lastReadId);
    const QByteArray payload = query.toString(QUrl::FullyEncoded).toUtf8();

    QNetworkReply *reply = m_networkAccessManager->post(request, payload);
    if (reply) {
        reply->setProperty("accountId", accountId);
        reply->setProperty("lastReadId", lastReadId);
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(errorHandler(QNetworkReply::NetworkError)));
        connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrorsHandler(QList<QSslError>)));
        connect(reply, SIGNAL(finished()), this, SLOT(finishedMarkReadHandler()));

        incrementSemaphore(accountId);
        setupReplyTimeout(accountId, reply);
    } else {
        qCWarning(lcMastodonNotifications) << "unable to update notifications marker for Mastodon account with id" << accountId;
    }
}

void MastodonNotificationsSyncAdaptor::finishedNotificationsHandler()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }

    const bool isError = reply->property("isError").toBool();
    const int accountId = reply->property("accountId").toInt();
    const QString accessToken = reply->property("accessToken").toString();
    const QString minId = reply->property("minId").toString();
    const QByteArray replyData = reply->readAll();

    disconnect(reply);
    reply->deleteLater();
    removeReplyTimeout(accountId, reply);

    PendingSyncState state = m_pendingSyncStates.value(accountId);
    if (state.accessToken.isEmpty()) {
        state.accessToken = accessToken;
    }
    if (state.unreadFloorId.isEmpty() && !minId.isEmpty()) {
        state.unreadFloorId = minId;
    }
    if (state.lastFetchedId.isEmpty() && !state.unreadFloorId.isEmpty()) {
        state.lastFetchedId = state.unreadFloorId;
    } else if (state.lastFetchedId.isEmpty() && !minId.isEmpty()) {
        state.lastFetchedId = minId;
    }

    bool ok = false;
    const QJsonArray notifications = parseJsonArrayReplyData(replyData, &ok);
    if (!isError && ok) {
        if (!notifications.size()) {
            qCDebug(lcMastodonNotifications) << "no notifications received for account" << accountId;
            m_pendingSyncStates.remove(accountId);
            decrementSemaphore(accountId);
            return;
        }

        QString pageMinNotificationId;

        foreach (const QJsonValue &notificationValue, notifications) {
            const QJsonObject notificationObject = notificationValue.toObject();
            if (notificationObject.isEmpty()) {
                continue;
            }

            const QString notificationId = notificationObject.value(QStringLiteral("id")).toVariant().toString();
            if (notificationId.isEmpty()) {
                continue;
            }

            if (pageMinNotificationId.isEmpty()
                    || compareNotificationIds(notificationId, pageMinNotificationId) < 0) {
                pageMinNotificationId = notificationId;
            }

            if (state.maxFetchedId.isEmpty()
                    || compareNotificationIds(notificationId, state.maxFetchedId) > 0) {
                state.maxFetchedId = notificationId;
            }

            if (!state.lastFetchedId.isEmpty()
                    && compareNotificationIds(notificationId, state.lastFetchedId) <= 0) {
                continue;
            }

            const QString notificationType = notificationObject.value(QStringLiteral("type")).toString();
            const QJsonObject actorObject = notificationObject.value(QStringLiteral("account")).toObject();
            const QJsonValue statusValue = notificationObject.value(QStringLiteral("status"));
            const QJsonObject statusObject = statusValue.isObject() && !statusValue.isNull()
                    ? statusValue.toObject()
                    : QJsonObject();
            const QJsonObject eventObject = notificationObject.value(QStringLiteral("event")).toObject();
            const QJsonObject warningObject = notificationObject.value(QStringLiteral("moderation_warning")).toObject();

            QDateTime eventTimestamp = parseTimestamp(notificationObject.value(QStringLiteral("created_at")).toString());
            if (!eventTimestamp.isValid()) {
                eventTimestamp = parseTimestamp(statusObject.value(QStringLiteral("created_at")).toString());
            }
            if (!eventTimestamp.isValid()) {
                continue;
            }

            const QString displayName = displayNameForAccount(actorObject);
            const QString accountName = actorObject.value(QStringLiteral("acct")).toString();

            const QString statusBody = sanitizeContent(statusObject.value(QStringLiteral("content")).toString());
            const QString action = actionText(notificationType);
            QString body;
            if (notificationType == QLatin1String("severed_relationships")) {
                body = severedRelationshipsText(eventObject);
            } else if (notificationType == QLatin1String("moderation_warning")) {
                const QString warningText = moderationWarningText(warningObject);
                body = warningText.isEmpty()
                        ? action
                        : QStringLiteral("%1: %2").arg(action, warningText);
            } else if (notificationType == QLatin1String("mention")
                    || notificationType == QLatin1String("status")
                    || notificationType == QLatin1String("update")
                    || notificationType == QLatin1String("quote")
                    || notificationType == QLatin1String("quoted_update")) {
                body = statusBody.isEmpty() ? action : statusBody;
            } else {
                body = statusBody.isEmpty() ? action : QStringLiteral("%1: %2").arg(action, statusBody);
            }

            const QString statusId = statusObject.value(QStringLiteral("id")).toVariant().toString();

            QString url = statusObject.value(QStringLiteral("url")).toString();
            if (url.isEmpty()) {
                url = statusObject.value(QStringLiteral("uri")).toString();
            }
            if (url.isEmpty()) {
                url = actorObject.value(QStringLiteral("url")).toString();
            }
            if (url.isEmpty() && !accountName.isEmpty() && !statusId.isEmpty()) {
                url = QStringLiteral("%1/@%2/%3").arg(apiHost(accountId), accountName, statusId);
            } else if (url.isEmpty() && !accountName.isEmpty()) {
                url = QStringLiteral("%1/@%2").arg(apiHost(accountId), accountName);
            }
            if (useSystemSummary(notificationType)) {
                url.clear();
            }

            PendingNotification pendingNotification;
            pendingNotification.notificationId = notificationId;
            pendingNotification.summary = useSystemSummary(notificationType)
                    ? qtTrId(TrIdMastodon)
                    : displayName;
            pendingNotification.body = body;
            pendingNotification.link = url;
            pendingNotification.timestamp = eventTimestamp;
            state.pendingNotifications.insert(notificationId, pendingNotification);
        }

        const QString historyBoundaryId = !state.unreadFloorId.isEmpty()
                ? state.unreadFloorId
                : state.lastFetchedId;
        if (notifications.size() >= NotificationsPageLimit
                && !pageMinNotificationId.isEmpty()
                && !historyBoundaryId.isEmpty()
                && compareNotificationIds(pageMinNotificationId, historyBoundaryId) > 0) {
            m_pendingSyncStates.insert(accountId, state);
            requestNotifications(accountId, state.accessToken, historyBoundaryId, pageMinNotificationId);
            decrementSemaphore(accountId);
            return;
        }

        if (state.pendingNotifications.size() > 0) {
            QStringList notificationIds = state.pendingNotifications.keys();
            std::sort(notificationIds.begin(), notificationIds.end(), [](const QString &left, const QString &right) {
                return compareNotificationIds(left, right) > 0;
            });

            foreach (const QString &notificationId, notificationIds) {
                const PendingNotification pendingNotification = state.pendingNotifications.value(notificationId);
                publishSystemNotification(accountId, pendingNotification);
            }
        }

        if (!state.maxFetchedId.isEmpty()
                && (state.lastFetchedId.isEmpty()
                    || compareNotificationIds(state.maxFetchedId, state.lastFetchedId) > 0)) {
            saveLastFetchedId(accountId, state.maxFetchedId);
        }

        const QString markerId = !state.maxFetchedId.isEmpty()
                ? state.maxFetchedId
                : state.lastFetchedId;
        const QString currentMarkerId = m_lastMarkedReadIds.value(accountId);
        if (!markerId.isEmpty()
                && !state.accessToken.isEmpty()
                && !hasActiveNotificationsForAccount(accountId)
                && (currentMarkerId.isEmpty()
                    || compareNotificationIds(markerId, currentMarkerId) > 0)) {
            requestMarkRead(accountId, state.accessToken, markerId);
        }
    } else {
        qCWarning(lcMastodonNotifications) << "unable to parse notifications data from request with account" << accountId
                                  << ", got:" << QString::fromUtf8(replyData);
    }

    m_pendingSyncStates.remove(accountId);
    decrementSemaphore(accountId);
}

void MastodonNotificationsSyncAdaptor::finishedMarkReadHandler()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }

    const bool isError = reply->property("isError").toBool();
    const int accountId = reply->property("accountId").toInt();
    const QString lastReadId = reply->property("lastReadId").toString();
    const QByteArray replyData = reply->readAll();

    disconnect(reply);
    reply->deleteLater();
    removeReplyTimeout(accountId, reply);

    bool ok = false;
    parseJsonObjectReplyData(replyData, &ok);
    if (!isError && ok) {
        const QString currentMarkerId = m_lastMarkedReadIds.value(accountId);
        if (currentMarkerId.isEmpty() || compareNotificationIds(lastReadId, currentMarkerId) > 0) {
            m_lastMarkedReadIds.insert(accountId, lastReadId);
        }
    } else {
        qCWarning(lcMastodonNotifications) << "unable to update notifications marker for account" << accountId
                                  << ", got:" << QString::fromUtf8(replyData);
    }

    decrementSemaphore(accountId);
}

void MastodonNotificationsSyncAdaptor::publishSystemNotification(int accountId,
                                                                 const PendingNotification &notificationData)
{
    Notification *notification = createNotification(accountId, notificationData.notificationId);
    notification->setItemCount(1);
    notification->setTimestamp(notificationData.timestamp.isValid()
                               ? notificationData.timestamp
                               : QDateTime::currentDateTimeUtc());
    notification->setSummary(notificationData.summary.isEmpty()
                             ? qtTrId(TrIdMastodon)
                             : notificationData.summary);
    notification->setBody(notificationData.body.isEmpty()
                          ? qtTrId(TrIdNewNotification)
                          : notificationData.body);
    notification->setPreviewSummary(notificationData.summary);
    notification->setPreviewBody(notificationData.body);

    const QString openUrl = notificationData.link.isEmpty()
            ? apiHost(accountId) + QStringLiteral("/notifications")
            : notificationData.link;
    const QUrl parsedOpenUrl(openUrl);
    const QString fallbackUrl = apiHost(accountId) + QStringLiteral("/notifications");
    const QString safeOpenUrl = parsedOpenUrl.isValid()
            && !parsedOpenUrl.scheme().isEmpty()
            && !parsedOpenUrl.host().isEmpty()
            ? openUrl
            : fallbackUrl;
    notification->setRemoteAction(OPEN_URL_ACTION(authorizeInteractionUrl(apiHost(accountId), safeOpenUrl)));
    notification->publish();
    if (notification->replacesId() == 0) {
        qCWarning(lcMastodonNotifications) << "failed to publish Mastodon notification"
                                  << notificationData.notificationId;
    }
}

void MastodonNotificationsSyncAdaptor::closeAccountNotifications(int accountId,
                                                                 const QSet<QString> &keepNotificationIds)
{
    QStringList cachedKeys = m_notificationObjects.keys();
    foreach (const QString &objectKey, cachedKeys) {
        Notification *notification = m_notificationObjects.value(objectKey);
        if (!notification
                || notification->hintValue("x-nemo.sociald.account-id").toInt() != accountId) {
            continue;
        }

        const QString notificationId = notification->hintValue(NotificationIdHint).toString();
        if (!notificationId.isEmpty() && keepNotificationIds.contains(notificationId)) {
            continue;
        }

        notification->close();
        m_notificationObjects.remove(objectKey);
        notification->deleteLater();
    }

    QList<QObject *> notifications = Notification::notifications();
    foreach (QObject *object, notifications) {
        Notification *notification = qobject_cast<Notification *>(object);
        if (!notification) {
            delete object;
            continue;
        }

        if (notification->category() == QLatin1String(NotificationCategory)
                && notification->hintValue("x-nemo.sociald.account-id").toInt() == accountId) {
            const QString notificationId = notification->hintValue(NotificationIdHint).toString();
            if (notificationId.isEmpty() || !keepNotificationIds.contains(notificationId)) {
                notification->close();
            }
        }

        if (notification->parent() != this) {
            delete notification;
        }
    }
}

Notification *MastodonNotificationsSyncAdaptor::createNotification(int accountId, const QString &notificationId)
{
    const QString objectKey = notificationObjectKey(accountId, notificationId);
    Notification *notification = m_notificationObjects.value(objectKey);
    if (!notification) {
        notification = findNotification(accountId, notificationId);
    }
    if (!notification) {
        notification = new Notification(this);
    } else if (notification->parent() != this) {
        notification->setParent(this);
    }

    notification->setAppName(QStringLiteral("Mastodon"));
    notification->setAppIcon(QStringLiteral("icon-l-mastodon"));
    notification->setHintValue("x-nemo.sociald.account-id", accountId);
    notification->setHintValue(NotificationIdHint, notificationId);
    notification->setHintValue("x-nemo-feedback", QStringLiteral("social"));
    notification->setHintValue("x-nemo-priority", 100); // Show on lockscreen
    notification->setCategory(QLatin1String(NotificationCategory));

    m_notificationObjects.insert(objectKey, notification);

    return notification;
}

Notification *MastodonNotificationsSyncAdaptor::findNotification(int accountId, const QString &notificationId)
{
    Notification *notification = 0;
    QList<QObject *> notifications = Notification::notifications();
    foreach (QObject *object, notifications) {
        Notification *castedNotification = qobject_cast<Notification *>(object);
        if (castedNotification
                && castedNotification->category() == QLatin1String(NotificationCategory)
                && castedNotification->hintValue("x-nemo.sociald.account-id").toInt() == accountId
                && castedNotification->hintValue(NotificationIdHint).toString() == notificationId) {
            notification = castedNotification;
            break;
        }
    }

    if (notification) {
        notifications.removeAll(notification);
    }

    qDeleteAll(notifications);

    return notification;
}
