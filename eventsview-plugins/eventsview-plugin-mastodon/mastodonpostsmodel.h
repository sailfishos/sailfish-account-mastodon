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

#ifndef MASTODONPOSTSMODEL_H
#define MASTODONPOSTSMODEL_H

#include "abstractsocialcachemodel.h"

class MastodonPostsModelPrivate;

class MastodonPostsModel: public AbstractSocialCacheModel
{
    Q_OBJECT
    Q_PROPERTY(QVariantList accountIdFilter READ accountIdFilter WRITE setAccountIdFilter NOTIFY accountIdFilterChanged)

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
        InstanceUrl,
        Accounts
    };

    explicit MastodonPostsModel(QObject *parent = 0);
    QHash<int, QByteArray> roleNames() const;

    QVariantList accountIdFilter() const;
    void setAccountIdFilter(const QVariantList &accountIds);

    void refresh();

signals:
    void accountIdFilterChanged();

private slots:
    void postsChanged();

private:
    Q_DECLARE_PRIVATE(MastodonPostsModel)
};

#endif // MASTODONPOSTSMODEL_H
