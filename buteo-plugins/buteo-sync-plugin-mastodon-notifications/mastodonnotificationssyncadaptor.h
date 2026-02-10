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
#include <QtNetwork/QNetworkReply>

#include "mastodonnotificationsdatabase.h"
#include <socialcache/socialimagesdatabase.h>

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
    struct PendingSyncState {
        QString accessToken;
        QString minReadId;
        QString maxNotificationId;
        int newNotificationCount;
        QString singleSummary;
        QString singleBody;
        QString singleLink;
        QDateTime singleTimestamp;
        bool dbCleared;

        PendingSyncState()
            : newNotificationCount(0)
            , dbCleared(false)
        {
        }
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
    void publishSystemNotification(int accountId, const PendingSyncState &state);
    Notification *createNotification(int accountId);
    Notification *findNotification(int accountId);
    void markReadFromNotification(Notification *notification);

private Q_SLOTS:
    void finishedUnreadMarkerHandler();
    void finishedNotificationsHandler();
    void finishedMarkReadHandler();
    void notificationClosedWithReason(uint reason);

private:
    MastodonNotificationsDatabase m_db;
    SocialImagesDatabase m_imageCacheDb;
    QHash<int, PendingSyncState> m_pendingSyncStates;
    QHash<int, QString> m_accessTokens;
    QHash<int, QString> m_lastMarkedReadIds;
};

#endif // MASTODONNOTIFICATIONSSYNCADAPTOR_H
