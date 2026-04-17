// SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
// SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "socialdnetworkaccessmanager_p.h"

/* The default implementation is just a normal QNetworkAccessManager */

SocialdNetworkAccessManager::SocialdNetworkAccessManager(QObject *parent)
    : QNetworkAccessManager(parent)
{
}

QNetworkReply *SocialdNetworkAccessManager::createRequest(
                                 QNetworkAccessManager::Operation op,
                                 const QNetworkRequest &req,
                                 QIODevice *outgoingData)
{
    return QNetworkAccessManager::createRequest(op, req, outgoingData);
}
