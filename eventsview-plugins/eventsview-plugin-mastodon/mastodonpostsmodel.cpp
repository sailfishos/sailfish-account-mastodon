// SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
// SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "mastodonpostsmodel.h"

namespace {
QVariantMap socialPostImageData(const SocialPostImage::ConstPtr &image)
{
    QVariantMap data;
    if (image.isNull()) {
        return data;
    }

    data.insert(QStringLiteral("url"), image->url());
    data.insert(QStringLiteral("type"), image->type() == SocialPostImage::Video
                ? QStringLiteral("video") : QStringLiteral("photo"));

    return data;
}

QVariantList socialPostImageDataList(const QList<SocialPostImage::ConstPtr> &images)
{
    QVariantList list;
    Q_FOREACH (const SocialPostImage::ConstPtr &image, images) {
        list.append(socialPostImageData(image));
    }
    return list;
}

QVariantList socialPostAccountIdList(const QList<int> &accounts)
{
    QVariantList list;
    Q_FOREACH (int account, accounts) {
        list.append(account);
    }
    return list;
}

void appendCommonPostFields(QMap<int, QVariant> *rowData, const SocialPost::ConstPtr &post)
{
    if (!rowData || post.isNull()) {
        return;
    }

    rowData->insert(MastodonPostsModel::MastodonId, post->identifier());
    rowData->insert(MastodonPostsModel::Name, post->name());
    rowData->insert(MastodonPostsModel::Body, post->body());
    rowData->insert(MastodonPostsModel::Timestamp, post->timestamp());
    rowData->insert(MastodonPostsModel::Icon, post->icon());
    rowData->insert(MastodonPostsModel::Images, socialPostImageDataList(post->images()));
    rowData->insert(MastodonPostsModel::Accounts, socialPostAccountIdList(post->accounts()));
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

        appendCommonPostFields(&eventMap, post);
        eventMap.insert(MastodonPostsModel::AccountName, accountName);
        eventMap.insert(MastodonPostsModel::Acct, accountName);
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
