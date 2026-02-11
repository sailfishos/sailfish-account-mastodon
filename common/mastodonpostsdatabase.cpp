/*
 * Copyright (C) 2013-2026 Jolla Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "mastodonpostsdatabase.h"

static const char *DB_NAME = "mastodon.db";
static const char *ACCOUNT_NAME_KEY = "account_name";
static const char *URL_KEY = "url";
static const char *BOOSTED_BY_KEY = "boosted_by";
static const char *REPLIES_COUNT_KEY = "replies_count";
static const char *FAVOURITES_COUNT_KEY = "favourites_count";
static const char *REBLOGS_COUNT_KEY = "reblogs_count";
static const char *INSTANCE_URL_KEY = "instance_url";

MastodonPostsDatabase::MastodonPostsDatabase()
    : AbstractSocialPostCacheDatabase(QStringLiteral("mastodon"), QLatin1String(DB_NAME))
{
}

MastodonPostsDatabase::~MastodonPostsDatabase()
{
}

void MastodonPostsDatabase::addMastodonPost(
        const QString &identifier,
        const QString &name,
        const QString &accountName,
        const QString &body,
        const QDateTime &timestamp,
        const QString &icon,
        const QList<QPair<QString, SocialPostImage::ImageType> > &images,
        const QString &url,
        const QString &boostedBy,
        int repliesCount,
        int favouritesCount,
        int reblogsCount,
        const QString &instanceUrl,
        int account)
{
    QVariantMap extra;
    extra.insert(ACCOUNT_NAME_KEY, accountName);
    extra.insert(URL_KEY, url);
    extra.insert(BOOSTED_BY_KEY, boostedBy);
    extra.insert(REPLIES_COUNT_KEY, repliesCount);
    extra.insert(FAVOURITES_COUNT_KEY, favouritesCount);
    extra.insert(REBLOGS_COUNT_KEY, reblogsCount);
    extra.insert(INSTANCE_URL_KEY, instanceUrl);
    addPost(identifier, name, body, timestamp, icon, images, extra, account);
}

QString MastodonPostsDatabase::accountName(const SocialPost::ConstPtr &post)
{
    if (post.isNull()) {
        return QString();
    }
    return post->extra().value(ACCOUNT_NAME_KEY).toString();
}

QString MastodonPostsDatabase::url(const SocialPost::ConstPtr &post)
{
    if (post.isNull()) {
        return QString();
    }
    return post->extra().value(URL_KEY).toString();
}

QString MastodonPostsDatabase::boostedBy(const SocialPost::ConstPtr &post)
{
    if (post.isNull()) {
        return QString();
    }
    return post->extra().value(BOOSTED_BY_KEY).toString();
}

int MastodonPostsDatabase::repliesCount(const SocialPost::ConstPtr &post)
{
    if (post.isNull()) {
        return 0;
    }
    return post->extra().value(REPLIES_COUNT_KEY).toInt();
}

int MastodonPostsDatabase::favouritesCount(const SocialPost::ConstPtr &post)
{
    if (post.isNull()) {
        return 0;
    }
    return post->extra().value(FAVOURITES_COUNT_KEY).toInt();
}

int MastodonPostsDatabase::reblogsCount(const SocialPost::ConstPtr &post)
{
    if (post.isNull()) {
        return 0;
    }
    return post->extra().value(REBLOGS_COUNT_KEY).toInt();
}

QString MastodonPostsDatabase::instanceUrl(const SocialPost::ConstPtr &post)
{
    if (post.isNull()) {
        return QString();
    }
    return post->extra().value(INSTANCE_URL_KEY).toString();
}
