/*
 * SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
 * SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef SOCIALD_QNAMFACTORY_P_H
#define SOCIALD_QNAMFACTORY_P_H

#include <QNetworkAccessManager>

class SocialdNetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT

public:
    SocialdNetworkAccessManager(QObject *parent = 0);

protected:
    QNetworkReply *createRequest(QNetworkAccessManager::Operation op,
                                 const QNetworkRequest &req,
                                 QIODevice *outgoingData = 0) override;
};

#endif
