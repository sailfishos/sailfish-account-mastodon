/*
 * SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
 * SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef MASTODONPOSTSDATABASE_H
#define MASTODONPOSTSDATABASE_H

#include <socialcache/abstractsocialpostcachedatabase.h>

class MastodonPostsDatabase: public AbstractSocialPostCacheDatabase
{
    Q_OBJECT
public:
    MastodonPostsDatabase();
    ~MastodonPostsDatabase();

    void addMastodonPost(const QString &identifier, const QString &name,
                         const QString &accountName, const QString &body,
                         const QDateTime &timestamp,
                         const QString &icon,
                         const QList<QPair<QString, SocialPostImage::ImageType> > &images,
                         const QString &url, const QString &boostedBy,
                         int repliesCount, int favouritesCount, int reblogsCount,
                         bool favourited, bool reblogged,
                         const QString &instanceUrl,
                         int account);

    static QString accountName(const SocialPost::ConstPtr &post);
    static QString url(const SocialPost::ConstPtr &post);
    static QString boostedBy(const SocialPost::ConstPtr &post);
    static int repliesCount(const SocialPost::ConstPtr &post);
    static int favouritesCount(const SocialPost::ConstPtr &post);
    static int reblogsCount(const SocialPost::ConstPtr &post);
    static bool favourited(const SocialPost::ConstPtr &post);
    static bool reblogged(const SocialPost::ConstPtr &post);
    static QString instanceUrl(const SocialPost::ConstPtr &post);
};

#endif // MASTODONPOSTSDATABASE_H
