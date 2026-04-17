// SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
// SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
//
// SPDX-License-Identifier: BSD-3-Clause

#include "mastodontransferplugin.h"
#include "mastodonuploader.h"

#include <QtPlugin>
#include <QNetworkAccessManager>

MastodonTransferPlugin::MastodonTransferPlugin()
    : QObject(), TransferPluginInterface()
    , m_qnam(new QNetworkAccessManager(this))
{
}

MastodonTransferPlugin::~MastodonTransferPlugin()
{
}

MediaTransferInterface *MastodonTransferPlugin::transferObject()
{
    return new MastodonUploader(m_qnam, this);
}

QString MastodonTransferPlugin::pluginId() const
{
    return QLatin1String("Mastodon");
}
