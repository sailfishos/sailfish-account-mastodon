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
#include "trace.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QRegularExpression>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QtNetwork/QNetworkRequest>

#include <notification.h>

#include <algorithm>

#define OPEN_BROWSER_ACTION(openUrlArgs)    \
    Notification::remoteAction(             \
        "default",                          \
        "",                                 \
        "org.sailfishos.browser",           \
        "/",                                \
        "org.sailfishos.browser",           \
        "openUrl",                          \
        QVariantList() << openUrlArgs       \
    )

namespace {
    const char *const NotificationCategory = "x-nemo.social.mastodon.notification";
    const char *const LastReadIdProperty = "mastodonLastReadId";
    const char *const NotificationIdHint = "x-nemo.sociald.notification-id";
    const int NotificationsPageLimit = 80;
    const uint NotificationDismissedReason = 1;

    QString decodeHtmlEntities(QString text)
    {
        text.replace(QStringLiteral("&quot;"), QStringLiteral("\""));
        text.replace(QStringLiteral("&apos;"), QStringLiteral("'"));
        text.replace(QStringLiteral("&lt;"), QStringLiteral("<"));
        text.replace(QStringLiteral("&gt;"), QStringLiteral(">"));
        text.replace(QStringLiteral("&amp;"), QStringLiteral("&"));
        text.replace(QStringLiteral("&nbsp;"), QStringLiteral(" "));

        static const QRegularExpression decimalEntity(QStringLiteral("&#(\\d+);"));
        QRegularExpressionMatch match;
        int index = 0;
        while ((index = text.indexOf(decimalEntity, index, &match)) != -1) {
            const uint value = match.captured(1).toUInt();
            const QString replacement = value > 0 ? QString(QChar(value)) : QString();
            text.replace(index, match.capturedLength(0), replacement);
            index += replacement.size();
        }

        static const QRegularExpression hexEntity(QStringLiteral("&#x([0-9a-fA-F]+);"));
        index = 0;
        while ((index = text.indexOf(hexEntity, index, &match)) != -1) {
            bool ok = false;
            const uint value = match.captured(1).toUInt(&ok, 16);
            const QString replacement = ok && value > 0 ? QString(QChar(value)) : QString();
            text.replace(index, match.capturedLength(0), replacement);
            index += replacement.size();
        }

        return text;
    }

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
            return QStringLiteral("mentioned you");
        } else if (type == QLatin1String("reblog")) {
            return QStringLiteral("boosted your post");
        } else if (type == QLatin1String("favourite")) {
            return QStringLiteral("favourited your post");
        } else if (type == QLatin1String("follow")) {
            return QStringLiteral("started following you");
        } else if (type == QLatin1String("follow_request")) {
            return QStringLiteral("requested to follow you");
        } else if (type == QLatin1String("poll")) {
            return QStringLiteral("interacted with your poll");
        } else if (type == QLatin1String("status")) {
            return QStringLiteral("posted");
        } else if (type == QLatin1String("update")) {
            return QStringLiteral("updated a post");
        }

        return QStringLiteral("sent you a notification");
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
    return QStringLiteral("mastodon-microblog");
}

void MastodonNotificationsSyncAdaptor::purgeDataForOldAccount(int oldId, SocialNetworkSyncAdaptor::PurgeMode)
{
    closeAccountNotifications(oldId);

    m_pendingSyncStates.remove(oldId);
    m_accessTokens.remove(oldId);
    m_lastMarkedReadIds.remove(oldId);
}

void MastodonNotificationsSyncAdaptor::beginSync(int accountId, const QString &accessToken)
{
    m_accessTokens.insert(accountId, accessToken);
    m_pendingSyncStates.remove(accountId);
    requestUnreadMarker(accountId, accessToken);
}

void MastodonNotificationsSyncAdaptor::finalize(int accountId)
{
    if (syncAborted()) {
        qCInfo(lcSocialPlugin) << "sync aborted, won't update notifications";
    }

    Q_UNUSED(accountId)
}

QString MastodonNotificationsSyncAdaptor::sanitizeContent(const QString &content)
{
    QString plain = content;
    plain.replace(QRegularExpression(QStringLiteral("<\\s*br\\s*/?\\s*>"), QRegularExpression::CaseInsensitiveOption), QStringLiteral("\n"));
    plain.replace(QRegularExpression(QStringLiteral("<\\s*/\\s*p\\s*>"), QRegularExpression::CaseInsensitiveOption), QStringLiteral("\n"));
    plain.remove(QRegularExpression(QStringLiteral("<[^>]+>"), QRegularExpression::CaseInsensitiveOption));

    return decodeHtmlEntities(plain).trimmed();
}

QDateTime MastodonNotificationsSyncAdaptor::parseTimestamp(const QString &timestampString)
{
    QDateTime timestamp;

#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
    timestamp = QDateTime::fromString(timestampString, Qt::ISODateWithMs);
    if (timestamp.isValid()) {
        return timestamp;
    }
#endif

    timestamp = QDateTime::fromString(timestampString, Qt::ISODate);
    if (timestamp.isValid()) {
        return timestamp;
    }

    const int timeSeparator = timestampString.indexOf(QLatin1Char('T'));
    const int fractionSeparator = timestampString.indexOf(QLatin1Char('.'), timeSeparator + 1);
    if (timeSeparator > -1 && fractionSeparator > -1) {
        int timezoneSeparator = timestampString.indexOf(QLatin1Char('Z'), fractionSeparator + 1);
        if (timezoneSeparator == -1) {
            timezoneSeparator = timestampString.indexOf(QLatin1Char('+'), fractionSeparator + 1);
        }
        if (timezoneSeparator == -1) {
            timezoneSeparator = timestampString.indexOf(QLatin1Char('-'), fractionSeparator + 1);
        }

        QString stripped = timestampString;
        if (timezoneSeparator > -1) {
            stripped.remove(fractionSeparator, timezoneSeparator - fractionSeparator);
        } else {
            stripped.truncate(fractionSeparator);
        }

        timestamp = QDateTime::fromString(stripped, Qt::ISODate);
    }

    return timestamp;
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
        qCWarning(lcSocialPlugin) << "unable to request notifications marker from Mastodon account with id" << accountId;
    }
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
        qCWarning(lcSocialPlugin) << "unable to request notifications from Mastodon account with id" << accountId;
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

    QString minReadId;
    bool ok = false;
    const QJsonObject markerObject = parseJsonObjectReplyData(replyData, &ok);
    if (!isError && ok) {
        minReadId = markerObject.value(QStringLiteral("notifications"))
                .toObject()
                .value(QStringLiteral("last_read_id"))
                .toVariant()
                .toString()
                .trimmed();
    } else {
        qCWarning(lcSocialPlugin) << "unable to parse notifications marker data from request with account" << accountId
                                  << ", got:" << QString::fromUtf8(replyData);
        decrementSemaphore(accountId);
        return;
    }

    PendingSyncState state;
    state.accessToken = accessToken;
    state.minReadId = minReadId;
    m_pendingSyncStates.insert(accountId, state);

    requestNotifications(accountId, accessToken, minReadId);
    decrementSemaphore(accountId);
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
        state.minReadId = minId;
    }

    bool ok = false;
    const QJsonArray notifications = parseJsonArrayReplyData(replyData, &ok);
    if (!isError && ok) {
        if (!notifications.size()) {
            closeAccountNotifications(accountId);
            qCDebug(lcSocialPlugin) << "no notifications received for account" << accountId;
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

            const QString notificationType = notificationObject.value(QStringLiteral("type")).toString();
            const QJsonObject actorObject = notificationObject.value(QStringLiteral("account")).toObject();
            const QJsonValue statusValue = notificationObject.value(QStringLiteral("status"));
            const QJsonObject statusObject = statusValue.isObject() && !statusValue.isNull()
                    ? statusValue.toObject()
                    : QJsonObject();

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
            if (notificationType == QLatin1String("mention")
                    || notificationType == QLatin1String("status")
                    || notificationType == QLatin1String("update")) {
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

            PendingNotification pendingNotification;
            pendingNotification.notificationId = notificationId;
            pendingNotification.summary = displayName;
            pendingNotification.body = body;
            pendingNotification.link = url;
            pendingNotification.timestamp = eventTimestamp;
            state.pendingNotifications.insert(notificationId, pendingNotification);
        }

        if (notifications.size() >= NotificationsPageLimit
                && !pageMinNotificationId.isEmpty()
                && (state.minReadId.isEmpty() || compareNotificationIds(pageMinNotificationId, state.minReadId) > 0)) {
            m_pendingSyncStates.insert(accountId, state);
            requestNotifications(accountId, state.accessToken, state.minReadId, pageMinNotificationId);
            decrementSemaphore(accountId);
            return;
        }

        if (state.pendingNotifications.size() > 0) {
            QStringList notificationIds = state.pendingNotifications.keys();
            std::sort(notificationIds.begin(), notificationIds.end(), [](const QString &left, const QString &right) {
                return compareNotificationIds(left, right) > 0;
            });

            QSet<QString> keepNotificationIds;
            foreach (const QString &notificationId, notificationIds) {
                const PendingNotification pendingNotification = state.pendingNotifications.value(notificationId);
                publishSystemNotification(accountId, pendingNotification);
                keepNotificationIds.insert(notificationId);
            }
            closeAccountNotifications(accountId, keepNotificationIds);
        }
    } else {
        qCWarning(lcSocialPlugin) << "unable to parse notifications data from request with account" << accountId
                                  << ", got:" << QString::fromUtf8(replyData);
    }

    m_pendingSyncStates.remove(accountId);
    decrementSemaphore(accountId);
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
        qCWarning(lcSocialPlugin) << "unable to mark notifications read for Mastodon account with id" << accountId;
    }
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
        m_lastMarkedReadIds.insert(accountId, lastReadId);
    } else {
        qCWarning(lcSocialPlugin) << "unable to mark notifications read for account" << accountId
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
                             ? QStringLiteral("Mastodon")
                             : notificationData.summary);
    notification->setBody(notificationData.body.isEmpty()
                          ? QStringLiteral("New notification")
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
    QStringList openUrlArgs;
    openUrlArgs << safeOpenUrl;

    notification->setProperty(LastReadIdProperty, notificationData.notificationId);
    notification->setUrgency(Notification::Low);
    notification->setRemoteAction(OPEN_BROWSER_ACTION(openUrlArgs));
    notification->publish();
    if (notification->replacesId() == 0) {
        qCWarning(lcSocialPlugin) << "failed to publish Mastodon notification"
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

void MastodonNotificationsSyncAdaptor::notificationClosedWithReason(uint reason)
{
    if (reason == NotificationDismissedReason) {
        markReadFromNotification(qobject_cast<Notification *>(sender()));
    }
}

void MastodonNotificationsSyncAdaptor::markReadFromNotification(Notification *notification)
{
    if (!notification) {
        return;
    }

    const int accountId = notification->hintValue("x-nemo.sociald.account-id").toInt();
    const QString lastReadId = notification->property(LastReadIdProperty).toString().trimmed();
    if (accountId <= 0 || lastReadId.isEmpty()) {
        return;
    }

    if (m_lastMarkedReadIds.value(accountId) == lastReadId) {
        return;
    }

    const QString accessToken = m_accessTokens.value(accountId).trimmed();
    if (accessToken.isEmpty()) {
        qCWarning(lcSocialPlugin) << "cannot mark notifications read for account" << accountId
                                  << "- missing access token";
        return;
    }

    notification->setProperty(LastReadIdProperty, QString());
    requestMarkRead(accountId, accessToken, lastReadId);
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
    notification->setCategory(QLatin1String(NotificationCategory));

    connect(notification, SIGNAL(closed(uint)), this, SLOT(notificationClosedWithReason(uint)), Qt::UniqueConnection);
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
