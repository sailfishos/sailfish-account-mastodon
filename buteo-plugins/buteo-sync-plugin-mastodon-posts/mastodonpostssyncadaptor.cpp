// SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
// SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "mastodonpostssyncadaptor.h"
#include "trace.h"
#include "mastodontextutils.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QtNetwork/QNetworkRequest>

namespace {
    QString displayNameForAccount(const QJsonObject &account)
    {
        const QString displayName = account.value(QStringLiteral("display_name")).toString().trimmed();
        if (!displayName.isEmpty()) {
            return displayName;
        }

        const QString username = account.value(QStringLiteral("username")).toString().trimmed();
        if (!username.isEmpty()) {
            return username;
        }

        return account.value(QStringLiteral("acct")).toString().trimmed();
    }
}

MastodonPostsSyncAdaptor::MastodonPostsSyncAdaptor(QObject *parent)
    : MastodonDataTypeSyncAdaptor(SocialNetworkSyncAdaptor::Posts, parent)
{
    setInitialActive(m_db.isValid());
}

MastodonPostsSyncAdaptor::~MastodonPostsSyncAdaptor()
{
}

QString MastodonPostsSyncAdaptor::syncServiceName() const
{
    return QStringLiteral("mastodon-microblog");
}

void MastodonPostsSyncAdaptor::purgeDataForOldAccount(int oldId, SocialNetworkSyncAdaptor::PurgeMode)
{
    m_db.removePosts(oldId);
    m_db.commit();
    m_db.wait();
    m_db.refresh();
    m_db.wait();

    purgeCachedImages(&m_imageCacheDb, oldId);
}

void MastodonPostsSyncAdaptor::beginSync(int accountId, const QString &accessToken)
{
    requestPosts(accountId, accessToken);
}

void MastodonPostsSyncAdaptor::finalize(int accountId)
{
    if (syncAborted()) {
        qCInfo(lcSocialPlugin) << "sync aborted, won't commit database changes";
    } else {
        m_db.commit();
        m_db.wait();
        m_db.refresh();
        m_db.wait();
        purgeExpiredImages(&m_imageCacheDb, accountId);
    }
}

QString MastodonPostsSyncAdaptor::sanitizeContent(const QString &content)
{
    return MastodonTextUtils::sanitizeContent(content);
}

QDateTime MastodonPostsSyncAdaptor::parseTimestamp(const QString &timestampString)
{
    return MastodonTextUtils::parseTimestamp(timestampString);
}

void MastodonPostsSyncAdaptor::requestPosts(int accountId, const QString &accessToken)
{
    QUrl url(apiHost(accountId) + QStringLiteral("/api/v1/timelines/home"));

    QUrlQuery query(url);
    query.addQueryItem(QStringLiteral("limit"), QStringLiteral("20"));
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QStringLiteral("Bearer %1").arg(accessToken).toUtf8());

    QNetworkReply *reply = m_networkAccessManager->get(request);
    if (reply) {
        reply->setProperty("accountId", accountId);
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(errorHandler(QNetworkReply::NetworkError)));
        connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrorsHandler(QList<QSslError>)));
        connect(reply, SIGNAL(finished()), this, SLOT(finishedPostsHandler()));

        incrementSemaphore(accountId);
        setupReplyTimeout(accountId, reply);
    } else {
        qCWarning(lcSocialPlugin) << "unable to request home timeline posts from Mastodon account with id" << accountId;
    }
}

void MastodonPostsSyncAdaptor::finishedPostsHandler()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }

    const bool isError = reply->property("isError").toBool();
    const int accountId = reply->property("accountId").toInt();
    QByteArray replyData = reply->readAll();

    disconnect(reply);
    reply->deleteLater();
    removeReplyTimeout(accountId, reply);

    bool ok = false;
    QJsonArray statuses = parseJsonArrayReplyData(replyData, &ok);
    if (!isError && ok) {
        m_db.removePosts(accountId);

        if (!statuses.size()) {
            qCDebug(lcSocialPlugin) << "no feed posts received for account" << accountId;
            decrementSemaphore(accountId);
            return;
        }

        const int sinceSpan = m_accountSyncProfile
                ? m_accountSyncProfile->key(Buteo::KEY_SYNC_SINCE_DAYS_PAST, QStringLiteral("7")).toInt()
                : 7;

        foreach (const QJsonValue &statusValue, statuses) {
            const QJsonObject statusObject = statusValue.toObject();
            if (statusObject.isEmpty()) {
                continue;
            }

            QJsonObject postObject = statusObject;
            QString boostedBy;
            if (statusObject.contains(QStringLiteral("reblog"))
                    && statusObject.value(QStringLiteral("reblog")).isObject()
                    && !statusObject.value(QStringLiteral("reblog")).isNull()) {
                boostedBy = displayNameForAccount(statusObject.value(QStringLiteral("account")).toObject());
                postObject = statusObject.value(QStringLiteral("reblog")).toObject();
            }

            QDateTime eventTimestamp = parseTimestamp(statusObject.value(QStringLiteral("created_at")).toString());
            if (!eventTimestamp.isValid()) {
                eventTimestamp = parseTimestamp(postObject.value(QStringLiteral("created_at")).toString());
            }
            if (!eventTimestamp.isValid()) {
                continue;
            }

            if (eventTimestamp.daysTo(QDateTime::currentDateTime()) > sinceSpan) {
                continue;
            }

            const QJsonObject account = postObject.value(QStringLiteral("account")).toObject();
            const QString displayName = displayNameForAccount(account);
            const QString accountName = account.value(QStringLiteral("acct")).toString();
            QString icon = account.value(QStringLiteral("avatar_static")).toString();
            if (icon.isEmpty()) {
                icon = account.value(QStringLiteral("avatar")).toString();
            }

            QString identifier = postObject.value(QStringLiteral("id")).toVariant().toString();
            if (identifier.isEmpty()) {
                continue;
            }

            QString url = postObject.value(QStringLiteral("url")).toString();
            if (url.isEmpty() && !accountName.isEmpty()) {
                url = QStringLiteral("%1/@%2/%3").arg(apiHost(accountId), accountName, identifier);
            }

            const QString body = sanitizeContent(postObject.value(QStringLiteral("content")).toString());
            const int repliesCount = postObject.value(QStringLiteral("replies_count")).toInt();
            const int favouritesCount = postObject.value(QStringLiteral("favourites_count")).toInt();
            const int reblogsCount = postObject.value(QStringLiteral("reblogs_count")).toInt();
            const bool favourited = postObject.value(QStringLiteral("favourited")).toBool();
            const bool reblogged = postObject.value(QStringLiteral("reblogged")).toBool();

            QList<QPair<QString, SocialPostImage::ImageType> > imageList;
            const QJsonArray mediaAttachments = postObject.value(QStringLiteral("media_attachments")).toArray();
            foreach (const QJsonValue &attachmentValue, mediaAttachments) {
                const QJsonObject attachment = attachmentValue.toObject();
                const QString mediaType = attachment.value(QStringLiteral("type")).toString();

                QString mediaUrl;
                SocialPostImage::ImageType imageType = SocialPostImage::Invalid;
                if (mediaType == QLatin1String("image")) {
                    mediaUrl = attachment.value(QStringLiteral("url")).toString();
                    imageType = SocialPostImage::Photo;
                } else if (mediaType == QLatin1String("video") || mediaType == QLatin1String("gifv")) {
                    mediaUrl = attachment.value(QStringLiteral("preview_url")).toString();
                    if (mediaUrl.isEmpty()) {
                        mediaUrl = attachment.value(QStringLiteral("url")).toString();
                    }
                    imageType = SocialPostImage::Video;
                }

                if (!mediaUrl.isEmpty() && imageType != SocialPostImage::Invalid) {
                    imageList.append(qMakePair(mediaUrl, imageType));
                }
            }

            m_db.addMastodonPost(identifier,
                                displayName,
                                accountName,
                                body,
                                eventTimestamp,
                                icon,
                                imageList,
                                url,
                                boostedBy,
                                repliesCount,
                                favouritesCount,
                                reblogsCount,
                                favourited,
                                reblogged,
                                apiHost(accountId),
                                accountId);
        }
    } else {
        qCWarning(lcSocialPlugin) << "unable to parse event feed data from request with account" << accountId
                                  << ", got:" << QString::fromUtf8(replyData);
    }

    decrementSemaphore(accountId);
}
