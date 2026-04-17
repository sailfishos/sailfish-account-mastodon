/*
 * SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
 * SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MASTODONSHAREPLUGIN_H
#define MASTODONSHAREPLUGIN_H

#include <QtCore/QObject>

#include <sharingplugininterface.h>

class Q_DECL_EXPORT MastodonSharePlugin : public QObject, public SharingPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.sailfishos.share.plugin.mastodon")
    Q_INTERFACES(SharingPluginInterface)

public:
    MastodonSharePlugin();
    ~MastodonSharePlugin();

    SharingPluginInfo *infoObject();
    QString pluginId() const;
};

#endif // MASTODONSHAREPLUGIN_H
