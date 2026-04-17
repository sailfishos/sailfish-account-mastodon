/*
 * SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
 * SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef MASTODONPOSTSMODEL_H
#define MASTODONPOSTSMODEL_H

#include "mastodonpostsdatabase.h"
#include <QtCore/QAbstractListModel>
#include <QtCore/QMap>

class MastodonPostsModel: public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QVariantList accountIdFilter READ accountIdFilter WRITE setAccountIdFilter NOTIFY accountIdFilterChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum MastodonPostsRole {
        MastodonId = 0,
        Name,
        AccountName,
        Acct,
        Body,
        Timestamp,
        Icon,
        Images,
        Url,
        Link,
        BoostedBy,
        RebloggedBy,
        RepliesCount,
        FavouritesCount,
        ReblogsCount,
        Favourited,
        Reblogged,
        InstanceUrl,
        Accounts
    };

    explicit MastodonPostsModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QVariantList accountIdFilter() const;
    void setAccountIdFilter(const QVariantList &accountIds);

    Q_INVOKABLE void refresh();

signals:
    void accountIdFilterChanged();
    void countChanged();

private slots:
    void postsChanged();

private:
    typedef QMap<int, QVariant> RowData;
    QList<RowData> m_data;
    MastodonPostsDatabase m_database;
};

#endif // MASTODONPOSTSMODEL_H
