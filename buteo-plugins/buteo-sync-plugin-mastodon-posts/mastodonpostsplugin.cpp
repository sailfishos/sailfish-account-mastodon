// SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
// SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "mastodonpostsplugin.h"
#include "mastodonpostssyncadaptor.h"
#include "socialnetworksyncadaptor.h"

MastodonPostsPlugin::MastodonPostsPlugin(const QString& pluginName,
                                         const Buteo::SyncProfile& profile,
                                         Buteo::PluginCbInterface *callbackInterface)
    : SocialdButeoPlugin(pluginName, profile, callbackInterface,
                         QStringLiteral("mastodon"),
                         SocialNetworkSyncAdaptor::dataTypeName(SocialNetworkSyncAdaptor::Posts))
{
}

MastodonPostsPlugin::~MastodonPostsPlugin()
{
}

SocialNetworkSyncAdaptor *MastodonPostsPlugin::createSocialNetworkSyncAdaptor()
{
    return new MastodonPostsSyncAdaptor(this);
}

Buteo::ClientPlugin* MastodonPostsPluginLoader::createClientPlugin(
        const QString& pluginName,
        const Buteo::SyncProfile& profile,
        Buteo::PluginCbInterface* cbInterface)
{
    return new MastodonPostsPlugin(pluginName, profile, cbInterface);
}
