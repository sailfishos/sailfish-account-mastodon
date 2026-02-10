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
