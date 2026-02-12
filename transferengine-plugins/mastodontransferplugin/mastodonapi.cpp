#include "mastodonapi.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtNetwork/QHttpMultiPart>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>

#include <QtNetwork/QNetworkRequest>

#include <QtDebug>

MastodonApi::MastodonApi(QNetworkAccessManager *qnam, QObject *parent)
    : QObject(parent)
    , m_qnam(qnam)
{
}

MastodonApi::~MastodonApi()
{
}

QString MastodonApi::normalizeApiHost(const QString &rawHost)
{
    QString host = rawHost.trimmed();
    if (host.isEmpty()) {
        host = QStringLiteral("https://mastodon.social");
    }

    if (!host.startsWith(QLatin1String("https://"))
            && !host.startsWith(QLatin1String("http://"))) {
        host.prepend(QStringLiteral("https://"));
    }

    QUrl url(host);
    if (!url.isValid() || url.host().isEmpty()) {
        return QStringLiteral("https://mastodon.social");
    }

    QString normalized = QString::fromLatin1(url.toEncoded(QUrl::RemovePath
                                                            | QUrl::RemoveQuery
                                                            | QUrl::RemoveFragment));
    if (normalized.endsWith(QLatin1Char('/'))) {
        normalized.chop(1);
    }

    return normalized;
}

bool MastodonApi::uploadImage(const QString &filePath,
                              const QString &statusText,
                              const QString &mimeType,
                              const QString &apiHost,
                              const QString &accessToken)
{
    QFile file(filePath);
    if (filePath.isEmpty() || !file.open(QIODevice::ReadOnly)) {
        qWarning() << Q_FUNC_INFO << "error opening file:" << filePath;
        return false;
    }

    m_apiHost = normalizeApiHost(apiHost);
    m_accessToken = accessToken;
    m_statusText = statusText;

    if (m_accessToken.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "missing access token";
        return false;
    }

    const QByteArray imageData = file.readAll();
    const QFileInfo fileInfo(filePath);

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant(QStringLiteral("form-data; name=\"file\"; filename=\"%1\"")
                                .arg(fileInfo.fileName())));
    if (!mimeType.isEmpty()) {
        filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(mimeType));
    }
    filePart.setBody(imageData);
    multiPart->append(filePart);

    QNetworkRequest request(QUrl(m_apiHost + QStringLiteral("/api/v1/media")));
    request.setRawHeader(QByteArrayLiteral("Authorization"),
                         QByteArrayLiteral("Bearer ") + m_accessToken.toUtf8());

    QNetworkReply *reply = m_qnam->post(request, multiPart);
    if (!reply) {
        delete multiPart;
        return false;
    }

    multiPart->setParent(reply);
    m_replies.insert(reply, UPLOAD_MEDIA);

    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(replyError(QNetworkReply::NetworkError)));
    connect(reply, &QNetworkReply::uploadProgress,
            this, &MastodonApi::uploadProgress);
    connect(reply, &QNetworkReply::finished,
            this, &MastodonApi::finished);

    return true;
}

bool MastodonApi::postStatus(const QString &statusText,
                             const QString &apiHost,
                             const QString &accessToken)
{
    m_apiHost = normalizeApiHost(apiHost);
    m_accessToken = accessToken;
    m_statusText = statusText;

    if (m_accessToken.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "missing access token";
        return false;
    }

    return postStatusInternal(QString());
}

bool MastodonApi::postStatusInternal(const QString &mediaId)
{
    if (m_statusText.trimmed().isEmpty() && mediaId.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "status and media id are empty";
        return false;
    }

    QUrlQuery query;
    if (!m_statusText.isEmpty()) {
        query.addQueryItem(QStringLiteral("status"), m_statusText);
    }
    if (!mediaId.isEmpty()) {
        query.addQueryItem(QStringLiteral("media_ids[]"), mediaId);
    }

    const QByteArray postData = query.query(QUrl::FullyEncoded).toUtf8();

    QNetworkRequest request(QUrl(m_apiHost + QStringLiteral("/api/v1/statuses")));
    request.setRawHeader(QByteArrayLiteral("Authorization"),
                         QByteArrayLiteral("Bearer ") + m_accessToken.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QVariant(QStringLiteral("application/x-www-form-urlencoded")));

    QNetworkReply *reply = m_qnam->post(request, postData);
    if (!reply) {
        return false;
    }

    m_replies.insert(reply, POST_STATUS);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(replyError(QNetworkReply::NetworkError)));
    connect(reply, &QNetworkReply::finished,
            this, &MastodonApi::finished);

    return true;
}

void MastodonApi::cancelUpload()
{
    if (m_replies.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "can't cancel upload";
        return;
    }

    const QList<QNetworkReply*> replies = m_replies.keys();
    Q_FOREACH (QNetworkReply *reply, replies) {
        reply->abort();
    }
    m_replies.clear();
}

void MastodonApi::replyError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)
}

void MastodonApi::uploadProgress(qint64 sent, qint64 total)
{
    if (total > 0) {
        emit transferProgressUpdated(sent / static_cast<qreal>(total));
    }
}

void MastodonApi::finished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply || !m_replies.contains(reply)) {
        return;
    }

    const API_CALL apiCall = m_replies.take(reply);
    const QByteArray data = reply->readAll();
    const int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QNetworkReply::NetworkError error = reply->error();

    reply->deleteLater();

    if (apiCall == UPLOAD_MEDIA) {
        if (error != QNetworkReply::NoError || httpCode < 200 || httpCode >= 300) {
            finishTransfer(error == QNetworkReply::NoError ? QNetworkReply::UnknownNetworkError : error,
                           httpCode,
                           data);
            return;
        }

        QString mediaId;
        const QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            const QJsonValue idValue = doc.object().value(QStringLiteral("id"));
            if (idValue.isString()) {
                mediaId = idValue.toString();
            } else if (idValue.isDouble()) {
                mediaId = QString::number(static_cast<qint64>(idValue.toDouble()));
            }
        }

        if (!postStatusInternal(mediaId)) {
            qWarning() << Q_FUNC_INFO << "unable to create mastodon status";
            emit transferError();
        }
        return;
    }

    if (apiCall == POST_STATUS) {
        finishTransfer(error, httpCode, data);
        return;
    }

    emit transferError();
}

void MastodonApi::finishTransfer(QNetworkReply::NetworkError error, int httpCode, const QByteArray &data)
{
    if (httpCode == 401) {
        emit credentialsExpired();
    }

    if (error != QNetworkReply::NoError) {
        if (error == QNetworkReply::OperationCanceledError) {
            emit transferCanceled();
            return;
        }

        qWarning() << Q_FUNC_INFO << "network error:" << error << "httpCode:" << httpCode << "data:" << data;
        emit transferError();
        return;
    }

    if (httpCode < 200 || httpCode >= 300) {
        qWarning() << Q_FUNC_INFO << "http error:" << httpCode << "data:" << data;
        emit transferError();
        return;
    }

    emit transferFinished();
}
