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
#include <QtCore/QVariantMap>

namespace {

static const char *URL_KEY = "url";
static const char *TYPE_KEY = "type";
static const char *TYPE_PHOTO = "photo";
static const char *TYPE_VIDEO = "video";

QVariantMap createImageData(const SocialPostImage::ConstPtr &image)
{
    QVariantMap imageData;
    imageData.insert(QLatin1String(URL_KEY), image->url());
    switch (image->type()) {
    case SocialPostImage::Video:
        imageData.insert(QLatin1String(TYPE_KEY), QLatin1String(TYPE_VIDEO));
        break;
    default:
        imageData.insert(QLatin1String(TYPE_KEY), QLatin1String(TYPE_PHOTO));
        break;
    }
    return imageData;
}

}

MastodonPostsModel::MastodonPostsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(&m_database, &AbstractSocialPostCacheDatabase::postsChanged,
            this, &MastodonPostsModel::postsChanged);
    connect(&m_database, SIGNAL(accountIdFilterChanged()),
            this, SIGNAL(accountIdFilterChanged()));
}

int MastodonPostsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_data.count();
}

QVariant MastodonPostsModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    if (!index.isValid() || row < 0 || row >= m_data.count()) {
        return QVariant();
    }

    return m_data.at(row).value(role);
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
    roleNames.insert(Favourited, "favourited");
    roleNames.insert(Reblogged, "reblogged");
    roleNames.insert(InstanceUrl, "instanceUrl");
    roleNames.insert(Accounts, "accounts");
    return roleNames;
}

QVariantList MastodonPostsModel::accountIdFilter() const
{
    return m_database.accountIdFilter();
}

void MastodonPostsModel::setAccountIdFilter(const QVariantList &accountIds)
{
    m_database.setAccountIdFilter(accountIds);
}

void MastodonPostsModel::refresh()
{
    m_database.refresh();
}

void MastodonPostsModel::postsChanged()
{
    QList<RowData> data;
    QList<SocialPost::ConstPtr> postsData = m_database.posts();
    Q_FOREACH (const SocialPost::ConstPtr &post, postsData) {
        RowData eventMap;
        const QString accountName = m_database.accountName(post);
        const QString postUrl = m_database.url(post);
        const QString boostedBy = m_database.boostedBy(post);
        const int repliesCount = m_database.repliesCount(post);
        const int favouritesCount = m_database.favouritesCount(post);
        const int reblogsCount = m_database.reblogsCount(post);
        const bool favourited = m_database.favourited(post);
        const bool reblogged = m_database.reblogged(post);

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
        eventMap.insert(MastodonPostsModel::Favourited, favourited);
        eventMap.insert(MastodonPostsModel::Reblogged, reblogged);
        eventMap.insert(MastodonPostsModel::InstanceUrl, m_database.instanceUrl(post));

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

    const int oldCount = m_data.count();
    beginResetModel();
    m_data = data;
    endResetModel();
    if (oldCount != m_data.count()) {
        emit countChanged();
    }
}
