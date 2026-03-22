/*
 * SPDX-FileCopyrightText: 2013 - 2026 Jolla Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "mastodonshareplugin.h"
#include "mastodonplugininfo.h"

#include <QtPlugin>

MastodonSharePlugin::MastodonSharePlugin()
    : QObject(), SharingPluginInterface()
{
}

MastodonSharePlugin::~MastodonSharePlugin()
{
}

SharingPluginInfo *MastodonSharePlugin::infoObject()
{
    return new MastodonPluginInfo;
}

QString MastodonSharePlugin::pluginId() const
{
    return QLatin1String("Mastodon");
}
