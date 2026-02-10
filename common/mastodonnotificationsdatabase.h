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

#ifndef MASTODONNOTIFICATIONSDATABASE_H
#define MASTODONNOTIFICATIONSDATABASE_H

#include <socialcache/abstractsocialpostcachedatabase.h>

class MastodonNotificationsDatabase: public AbstractSocialPostCacheDatabase
{
    Q_OBJECT
public:
    MastodonNotificationsDatabase();
    ~MastodonNotificationsDatabase();

    void addMastodonNotification(const QString &identifier, const QString &name,
                                 const QString &accountName, const QString &body,
                                 const QDateTime &timestamp,
                                 const QString &icon,
                                 const QList<QPair<QString, SocialPostImage::ImageType> > &images,
                                 const QString &url, const QString &boostedBy,
                                 const QString &instanceUrl,
                                 int account);

    static QString accountName(const SocialPost::ConstPtr &post);
    static QString url(const SocialPost::ConstPtr &post);
    static QString boostedBy(const SocialPost::ConstPtr &post);
    static QString instanceUrl(const SocialPost::ConstPtr &post);
};

#endif // MASTODONNOTIFICATIONSDATABASE_H
