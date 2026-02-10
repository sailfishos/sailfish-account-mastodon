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

#ifndef MASTODONNOTIFICATIONSSYNCADAPTOR_H
#define MASTODONNOTIFICATIONSSYNCADAPTOR_H

#include "mastodondatatypesyncadaptor.h"

#include <QtCore/QDateTime>
#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QtNetwork/QNetworkReply>

class Notification;

class MastodonNotificationsSyncAdaptor : public MastodonNotificationsDataTypeSyncAdaptor
{
    Q_OBJECT

public:
    MastodonNotificationsSyncAdaptor(QObject *parent);
    ~MastodonNotificationsSyncAdaptor();

    QString syncServiceName() const override;

protected:
    void purgeDataForOldAccount(int oldId, SocialNetworkSyncAdaptor::PurgeMode mode) override;
    void beginSync(int accountId, const QString &accessToken) override;
    void finalize(int accountId) override;

private:
    struct PendingNotification {
        QString notificationId;
        QString summary;
        QString body;
        QString link;
        QDateTime timestamp;
    };

    struct PendingSyncState {
        QString accessToken;
        QString minReadId;
        QHash<QString, PendingNotification> pendingNotifications;
    };

    static QString sanitizeContent(const QString &content);
    static QDateTime parseTimestamp(const QString &timestampString);
    static int compareNotificationIds(const QString &left, const QString &right);

    void requestUnreadMarker(int accountId, const QString &accessToken);
    void requestNotifications(int accountId,
                              const QString &accessToken,
                              const QString &minId,
                              const QString &maxId = QString());
    void requestMarkRead(int accountId, const QString &accessToken, const QString &lastReadId);
    void publishSystemNotification(int accountId, const PendingNotification &notificationData);
    Notification *createNotification(int accountId, const QString &notificationId);
    Notification *findNotification(int accountId, const QString &notificationId);
    void closeAccountNotifications(int accountId, const QSet<QString> &keepNotificationIds = QSet<QString>());
    static QString notificationObjectKey(int accountId, const QString &notificationId);
    void markReadFromNotification(Notification *notification);

private Q_SLOTS:
    void finishedUnreadMarkerHandler();
    void finishedNotificationsHandler();
    void finishedMarkReadHandler();
    void notificationClosedWithReason(uint reason);

private:
    QHash<int, PendingSyncState> m_pendingSyncStates;
    QHash<int, QString> m_accessTokens;
    QHash<int, QString> m_lastMarkedReadIds;
    QHash<QString, Notification *> m_notificationObjects;
};

#endif // MASTODONNOTIFICATIONSSYNCADAPTOR_H
