/*
 * Copyright (C) 2013-2026 Jolla Ltd.
 */

#ifndef MASTODONUPLOADER_H
#define MASTODONUPLOADER_H

#include <QtNetwork/QNetworkAccessManager>

#include <mediatransferinterface.h>

#include "mastodonshareservicestatus.h"

class MastodonApi;

class MastodonUploader : public MediaTransferInterface
{
    Q_OBJECT

public:
    MastodonUploader(QNetworkAccessManager *qnam, QObject *parent = 0);
    ~MastodonUploader();

    QString displayName() const;
    QUrl serviceIcon() const;
    bool cancelEnabled() const;
    bool restartEnabled() const;

public Q_SLOTS:
    void start();
    void cancel();

private Q_SLOTS:
    void startUploading();
    void transferFinished();
    void transferProgress(qreal progress);
    void transferError();
    void transferCanceled();
    void credentialsExpired();

protected:
    void setStatus(MediaTransferInterface::TransferStatus status);

private:
    void ensureApi();
    void postImage();
    void postStatus();

    MastodonApi *m_api;
    MastodonShareServiceStatus *m_mastodonShareServiceStatus;
    QNetworkAccessManager *m_qnam;
    MastodonShareServiceStatus::AccountDetails m_accountDetails;
    bool m_useTmpFile;
    QString m_filePath;
};

#endif // MASTODONUPLOADER_H
