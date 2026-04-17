/*
 * SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
 * SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef MASTODONNOTIFICATIONSPLUGIN_H
#define MASTODONNOTIFICATIONSPLUGIN_H

#include "socialdbuteoplugin.h"

#include <buteosyncfw5/SyncPluginLoader.h>

class Q_DECL_EXPORT MastodonNotificationsPlugin : public SocialdButeoPlugin
{
    Q_OBJECT

public:
    MastodonNotificationsPlugin(const QString& pluginName,
                                const Buteo::SyncProfile& profile,
                                Buteo::PluginCbInterface *cbInterface);
    ~MastodonNotificationsPlugin();

protected:
    SocialNetworkSyncAdaptor *createSocialNetworkSyncAdaptor() override;
};

class MastodonNotificationsPluginLoader : public Buteo::SyncPluginLoader
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.sailfishos.plugins.sync.MastodonNotificationsPluginLoader")
    Q_INTERFACES(Buteo::SyncPluginLoader)

public:
    Buteo::ClientPlugin* createClientPlugin(const QString& pluginName,
                                            const Buteo::SyncProfile& profile,
                                            Buteo::PluginCbInterface* cbInterface) override;
};

#endif // MASTODONNOTIFICATIONSPLUGIN_H
