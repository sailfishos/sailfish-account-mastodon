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

#ifndef MASTODONPOSTSSYNCADAPTOR_H
#define MASTODONPOSTSSYNCADAPTOR_H

#include "mastodondatatypesyncadaptor.h"

#include <QtCore/QDateTime>
#include <QtNetwork/QNetworkReply>

#include "mastodonpostsdatabase.h"
#include <socialcache/socialimagesdatabase.h>

class MastodonPostsSyncAdaptor : public MastodonDataTypeSyncAdaptor
{
    Q_OBJECT

public:
    MastodonPostsSyncAdaptor(QObject *parent);
    ~MastodonPostsSyncAdaptor();

    QString syncServiceName() const override;

protected:
    void purgeDataForOldAccount(int oldId, SocialNetworkSyncAdaptor::PurgeMode mode) override;
    void beginSync(int accountId, const QString &accessToken) override;
    void finalize(int accountId) override;

private:
    static QString sanitizeContent(const QString &content);
    static QDateTime parseTimestamp(const QString &timestampString);

    void requestPosts(int accountId, const QString &accessToken);

private Q_SLOTS:
    void finishedPostsHandler();

private:
    MastodonPostsDatabase m_db;
    SocialImagesDatabase m_imageCacheDb;
};

#endif // MASTODONPOSTSSYNCADAPTOR_H
