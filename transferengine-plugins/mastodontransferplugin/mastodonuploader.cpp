/*
 * SPDX-FileCopyrightText: 2013 - 2026 Jolla Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "mastodonuploader.h"
#include "mastodonapi.h"

#include <imageoperation.h>
#include <mediaitem.h>

#include <QtCore/QFile>
#include <QtCore/QMimeDatabase>
#include <QtCore/QMimeType>

#include <QtDebug>

MastodonUploader::MastodonUploader(QNetworkAccessManager *qnam, QObject *parent)
    : MediaTransferInterface(parent)
    , m_api(0)
    , m_mastodonShareServiceStatus(0)
    , m_qnam(qnam)
    , m_useTmpFile(false)
{
}

MastodonUploader::~MastodonUploader()
{
}

QString MastodonUploader::displayName() const
{
    return tr("Mastodon");
}

QUrl MastodonUploader::serviceIcon() const
{
    return QUrl(QStringLiteral("image://theme/icon-l-mastodon"));
}

bool MastodonUploader::cancelEnabled() const
{
    return true;
}

bool MastodonUploader::restartEnabled() const
{
    return true;
}

void MastodonUploader::start()
{
    if (!mediaItem()) {
        qWarning() << Q_FUNC_INFO << "NULL MediaItem. Can't continue";
        setStatus(MediaTransferInterface::TransferInterrupted);
        return;
    }

    if (!m_mastodonShareServiceStatus) {
        m_mastodonShareServiceStatus = new MastodonShareServiceStatus(this);
        connect(m_mastodonShareServiceStatus, &MastodonShareServiceStatus::serviceReady,
                this, &MastodonUploader::startUploading);
        connect(m_mastodonShareServiceStatus, &MastodonShareServiceStatus::serviceError,
                this, [this] (const QString &) {
            transferError();
        });
    }

    m_mastodonShareServiceStatus->queryStatus();
}

void MastodonUploader::cancel()
{
    if (m_api) {
        m_api->cancelUpload();
    } else {
        qWarning() << Q_FUNC_INFO << "Can't cancel. NULL MastodonApi object!";
    }
}

void MastodonUploader::startUploading()
{
    if (!m_mastodonShareServiceStatus) {
        qWarning() << Q_FUNC_INFO << "NULL MastodonShareServiceStatus object!";
        return;
    }

    const quint32 accountId = mediaItem()->value(MediaItem::AccountId).toInt();
    m_accountDetails = m_mastodonShareServiceStatus->detailsByIdentifier(accountId);
    if (m_accountDetails.accountId <= 0 || m_accountDetails.accessToken.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Mastodon account details missing for id" << accountId;
        transferError();
        return;
    }

    const QString mimeType = mediaItem()->value(MediaItem::MimeType).toString();
    if (mimeType.startsWith(QLatin1String("image/"))
            || mimeType.startsWith(QLatin1String("video/"))) {
        postImage();
    } else if (mimeType.contains(QLatin1String("text/plain"))
               || mimeType.contains(QLatin1String("text/x-url"))) {
        postStatus();
    } else {
        qWarning() << Q_FUNC_INFO << "Unsupported mime type:" << mimeType;
        setStatus(MediaTransferInterface::TransferInterrupted);
    }
}

void MastodonUploader::transferFinished()
{
    setStatus(MediaTransferInterface::TransferFinished);
}

void MastodonUploader::transferProgress(qreal progress)
{
    setProgress(progress);
}

void MastodonUploader::transferError()
{
    setStatus(MediaTransferInterface::TransferInterrupted);
    qWarning() << Q_FUNC_INFO << "Transfer interrupted";
}

void MastodonUploader::transferCanceled()
{
    setStatus(MediaTransferInterface::TransferCanceled);
}

void MastodonUploader::credentialsExpired()
{
    const quint32 accountId = mediaItem()->value(MediaItem::AccountId).toInt();
    m_mastodonShareServiceStatus->setCredentialsNeedUpdate(accountId, QStringLiteral("mastodon-sharing"));
}

void MastodonUploader::setStatus(MediaTransferInterface::TransferStatus status)
{
    const bool finished = (status == TransferCanceled
                           || status == TransferInterrupted
                           || status == TransferFinished);
    if (m_useTmpFile && finished) {
        QFile::remove(m_filePath);
        m_useTmpFile = false;
        m_filePath.clear();
    }

    MediaTransferInterface::setStatus(status);
}

void MastodonUploader::postImage()
{
    m_useTmpFile = false;
    m_filePath.clear();
    const QString sourceFile = mediaItem()->value(MediaItem::Url).toUrl().toLocalFile();
    if (sourceFile.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Empty source file";
        setStatus(MediaTransferInterface::TransferInterrupted);
        return;
    }

    QMimeDatabase db;
    const QMimeType mime = db.mimeTypeForFile(sourceFile);
    const bool isImage = mediaItem()->value(MediaItem::MimeType).toString().startsWith(QLatin1String("image/"));
    const bool isJpeg = isImage && mime.name() == QLatin1String("image/jpeg");

    if (isJpeg && mediaItem()->value(MediaItem::MetadataStripped).toBool()) {
        m_useTmpFile = true;
        m_filePath = ImageOperation::removeImageMetadata(sourceFile);
        if (m_filePath.isEmpty()) {
            qWarning() << Q_FUNC_INFO << "Failed to remove metadata";
            MediaTransferInterface::setStatus(MediaTransferInterface::TransferInterrupted);
            return;
        }
    }

    const qreal scale = mediaItem()->value(MediaItem::ScalePercent).toReal();
    if (isImage && 0 < scale && scale < 1) {
        m_useTmpFile = true;
        m_filePath = ImageOperation::scaleImage(sourceFile, scale, m_filePath);
        if (m_filePath.isEmpty()) {
            qWarning() << Q_FUNC_INFO << "Failed to scale image";
            MediaTransferInterface::setStatus(MediaTransferInterface::TransferInterrupted);
            return;
        }
    }

    if (!m_useTmpFile) {
        m_filePath = sourceFile;
    }

    ensureApi();

    const bool ok = m_api->uploadImage(m_filePath,
                                       mediaItem()->value(MediaItem::Description).toString(),
                                       mediaItem()->value(MediaItem::MimeType).toString(),
                                       m_accountDetails.apiHost,
                                       m_accountDetails.accessToken);
    if (ok) {
        setStatus(MediaTransferInterface::TransferStarted);
    } else {
        setStatus(MediaTransferInterface::TransferInterrupted);
        qWarning() << Q_FUNC_INFO << "Failed to upload media";
    }
}

void MastodonUploader::postStatus()
{
    ensureApi();

    const QVariantMap userData = mediaItem()->value(MediaItem::UserData).toMap();
    QString statusText = userData.value(QStringLiteral("status")).toString().trimmed();
    if (statusText.isEmpty()) {
        statusText = mediaItem()->value(MediaItem::Description).toString().trimmed();
    }
    if (statusText.isEmpty()) {
        statusText = mediaItem()->value(MediaItem::ContentData).toString().trimmed();
    }

    if (statusText.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Failed to resolve status text";
        setStatus(MediaTransferInterface::TransferInterrupted);
        return;
    }

    const bool ok = m_api->postStatus(statusText,
                                      m_accountDetails.apiHost,
                                      m_accountDetails.accessToken);
    if (ok) {
        setStatus(MediaTransferInterface::TransferStarted);
    } else {
        setStatus(MediaTransferInterface::TransferInterrupted);
        qWarning() << Q_FUNC_INFO << "Failed to post status";
    }
}

void MastodonUploader::ensureApi()
{
    if (!m_api) {
        m_api = new MastodonApi(m_qnam, this);
        connect(m_api, &MastodonApi::transferProgressUpdated,
                this, &MastodonUploader::transferProgress);
        connect(m_api, &MastodonApi::transferFinished,
                this, &MastodonUploader::transferFinished);
        connect(m_api, &MastodonApi::transferError,
                this, &MastodonUploader::transferError);
        connect(m_api, &MastodonApi::transferCanceled,
                this, &MastodonUploader::transferCanceled);
        connect(m_api, &MastodonApi::credentialsExpired,
                this, &MastodonUploader::credentialsExpired);
    }
}
