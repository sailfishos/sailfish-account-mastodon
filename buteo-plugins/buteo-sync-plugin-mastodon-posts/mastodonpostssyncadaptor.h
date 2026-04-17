/*
 * SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
 * SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

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
