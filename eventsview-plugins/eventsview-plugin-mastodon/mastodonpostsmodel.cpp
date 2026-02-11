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

#include "mastodonpostsmodel.h"
#include "abstractsocialcachemodel_p.h"
#include "mastodonpostsdatabase.h"
#include "postimagehelper_p.h"

class MastodonPostsModelPrivate: public AbstractSocialCacheModelPrivate
{
public:
    explicit MastodonPostsModelPrivate(MastodonPostsModel *q);

    MastodonPostsDatabase database;

private:
    Q_DECLARE_PUBLIC(MastodonPostsModel)
};

MastodonPostsModelPrivate::MastodonPostsModelPrivate(MastodonPostsModel *q)
    : AbstractSocialCacheModelPrivate(q)
{
}

MastodonPostsModel::MastodonPostsModel(QObject *parent)
    : AbstractSocialCacheModel(*(new MastodonPostsModelPrivate(this)), parent)
{
    Q_D(MastodonPostsModel);

    connect(&d->database, &AbstractSocialPostCacheDatabase::postsChanged,
            this, &MastodonPostsModel::postsChanged);
    connect(&d->database, SIGNAL(accountIdFilterChanged()),
            this, SIGNAL(accountIdFilterChanged()));
}

QHash<int, QByteArray> MastodonPostsModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames.insert(MastodonId, "mastodonId");
    roleNames.insert(Name, "name");
    roleNames.insert(AccountName, "accountName");
    roleNames.insert(Acct, "acct");
    roleNames.insert(Body, "body");
    roleNames.insert(Timestamp, "timestamp");
    roleNames.insert(Icon, "icon");
    roleNames.insert(Images, "images");
    roleNames.insert(Url, "url");
    roleNames.insert(Link, "link");
    roleNames.insert(BoostedBy, "boostedBy");
    roleNames.insert(RebloggedBy, "rebloggedBy");
    roleNames.insert(RepliesCount, "repliesCount");
    roleNames.insert(FavouritesCount, "favouritesCount");
    roleNames.insert(ReblogsCount, "reblogsCount");
    roleNames.insert(InstanceUrl, "instanceUrl");
    roleNames.insert(Accounts, "accounts");
    return roleNames;
}

QVariantList MastodonPostsModel::accountIdFilter() const
{
    Q_D(const MastodonPostsModel);

    return d->database.accountIdFilter();
}

void MastodonPostsModel::setAccountIdFilter(const QVariantList &accountIds)
{
    Q_D(MastodonPostsModel);

    d->database.setAccountIdFilter(accountIds);
}

void MastodonPostsModel::refresh()
{
    Q_D(MastodonPostsModel);

    d->database.refresh();
}

void MastodonPostsModel::postsChanged()
{
    Q_D(MastodonPostsModel);

    SocialCacheModelData data;
    QList<SocialPost::ConstPtr> postsData = d->database.posts();
    Q_FOREACH (const SocialPost::ConstPtr &post, postsData) {
        QMap<int, QVariant> eventMap;
        const QString accountName = d->database.accountName(post);
        const QString postUrl = d->database.url(post);
        const QString boostedBy = d->database.boostedBy(post);
        const int repliesCount = d->database.repliesCount(post);
        const int favouritesCount = d->database.favouritesCount(post);
        const int reblogsCount = d->database.reblogsCount(post);

        eventMap.insert(MastodonPostsModel::MastodonId, post->identifier());
        eventMap.insert(MastodonPostsModel::Name, post->name());
        eventMap.insert(MastodonPostsModel::AccountName, accountName);
        eventMap.insert(MastodonPostsModel::Acct, accountName);
        eventMap.insert(MastodonPostsModel::Body, post->body());
        eventMap.insert(MastodonPostsModel::Timestamp, post->timestamp());
        eventMap.insert(MastodonPostsModel::Icon, post->icon());
        eventMap.insert(MastodonPostsModel::Url, postUrl);
        eventMap.insert(MastodonPostsModel::Link, postUrl);
        eventMap.insert(MastodonPostsModel::BoostedBy, boostedBy);
        eventMap.insert(MastodonPostsModel::RebloggedBy, boostedBy);
        eventMap.insert(MastodonPostsModel::RepliesCount, repliesCount);
        eventMap.insert(MastodonPostsModel::FavouritesCount, favouritesCount);
        eventMap.insert(MastodonPostsModel::ReblogsCount, reblogsCount);
        eventMap.insert(MastodonPostsModel::InstanceUrl, d->database.instanceUrl(post));

        QVariantList images;
        Q_FOREACH (const SocialPostImage::ConstPtr &image, post->images()) {
            images.append(createImageData(image));
        }
        eventMap.insert(MastodonPostsModel::Images, images);

        QVariantList accountsVariant;
        Q_FOREACH (int account, post->accounts()) {
            accountsVariant.append(account);
        }
        eventMap.insert(MastodonPostsModel::Accounts, accountsVariant);
        data.append(eventMap);
    }

    updateData(data);
}
