// SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
// SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "mastodonpostsdatabase.h"

static const char *DB_NAME = "mastodon.db";
static const char *ACCOUNT_NAME_KEY = "account_name";
static const char *URL_KEY = "url";
static const char *BOOSTED_BY_KEY = "boosted_by";
static const char *REPLIES_COUNT_KEY = "replies_count";
static const char *FAVOURITES_COUNT_KEY = "favourites_count";
static const char *REBLOGS_COUNT_KEY = "reblogs_count";
static const char *FAVOURITED_KEY = "favourited";
static const char *REBLOGGED_KEY = "reblogged";
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
        bool favourited,
        bool reblogged,
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
    extra.insert(FAVOURITED_KEY, favourited);
    extra.insert(REBLOGGED_KEY, reblogged);
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

bool MastodonPostsDatabase::favourited(const SocialPost::ConstPtr &post)
{
    if (post.isNull()) {
        return false;
    }
    return post->extra().value(FAVOURITED_KEY).toBool();
}

bool MastodonPostsDatabase::reblogged(const SocialPost::ConstPtr &post)
{
    if (post.isNull()) {
        return false;
    }
    return post->extra().value(REBLOGGED_KEY).toBool();
}

QString MastodonPostsDatabase::instanceUrl(const SocialPost::ConstPtr &post)
{
    if (post.isNull()) {
        return QString();
    }
    return post->extra().value(INSTANCE_URL_KEY).toString();
}
