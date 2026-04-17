/*
 * SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
 * SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MASTODONAPI_H
#define MASTODONAPI_H

#include <QtCore/QMap>
#include <QtCore/QObject>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

class MastodonApi : public QObject
{
    Q_OBJECT

public:
    enum API_CALL {
        NONE,
        UPLOAD_MEDIA,
        POST_STATUS
    };

    explicit MastodonApi(QNetworkAccessManager *qnam, QObject *parent = 0);
    ~MastodonApi();

    bool uploadImage(const QString &filePath,
                     const QString &statusText,
                     const QString &mimeType,
                     const QString &apiHost,
                     const QString &accessToken);
    bool postStatus(const QString &statusText,
                    const QString &apiHost,
                    const QString &accessToken);

    void cancelUpload();

Q_SIGNALS:
    void transferProgressUpdated(qreal progress);
    void transferFinished();
    void transferError();
    void transferCanceled();
    void credentialsExpired();

private Q_SLOTS:
    void replyError(QNetworkReply::NetworkError error);
    void finished();
    void uploadProgress(qint64 received, qint64 total);

private:
    bool postStatusInternal(const QString &mediaId);
    void finishTransfer(QNetworkReply::NetworkError error, int httpCode, const QByteArray &data);

    QMap<QNetworkReply*, API_CALL> m_replies;
    bool m_cancelRequested;
    QNetworkAccessManager *m_qnam;
    QString m_accessToken;
    QString m_apiHost;
    QString m_statusText;
};

#endif // MASTODONAPI_H
