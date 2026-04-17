// SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
// SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "mastodonnotificationsplugin.h"
#include "mastodonnotificationssyncadaptor.h"
#include "socialnetworksyncadaptor.h"

#include <QCoreApplication>
#include <QLocale>
#include <QTranslator>

namespace {
class AppTranslator : public QTranslator
{
public:
    explicit AppTranslator(QObject *parent)
        : QTranslator(parent)
    {
        qApp->installTranslator(this);
    }

    ~AppTranslator() override
    {
        qApp->removeTranslator(this);
    }
};

void ensureNotificationTranslations()
{
    static bool initialized = false;
    if (initialized) {
        return;
    }

    QCoreApplication *app = QCoreApplication::instance();
    if (!app) {
        return;
    }

    AppTranslator *engineeringEnglish = new AppTranslator(app);
    engineeringEnglish->load(QStringLiteral("lipstick-jolla-home-mastodon-notifications_eng_en"),
                             QStringLiteral("/usr/share/translations"));

    AppTranslator *translator = new AppTranslator(app);
    translator->load(QLocale(),
                     QStringLiteral("lipstick-jolla-home-mastodon-notifications"),
                     QStringLiteral("-"),
                     QStringLiteral("/usr/share/translations"));

    initialized = true;
}
}

MastodonNotificationsPlugin::MastodonNotificationsPlugin(const QString& pluginName,
                                         const Buteo::SyncProfile& profile,
                                         Buteo::PluginCbInterface *callbackInterface)
    : SocialdButeoPlugin(pluginName, profile, callbackInterface,
                         QStringLiteral("mastodon"),
                         SocialNetworkSyncAdaptor::dataTypeName(SocialNetworkSyncAdaptor::Notifications))
{
    ensureNotificationTranslations();
}

MastodonNotificationsPlugin::~MastodonNotificationsPlugin()
{
}

SocialNetworkSyncAdaptor *MastodonNotificationsPlugin::createSocialNetworkSyncAdaptor()
{
    return new MastodonNotificationsSyncAdaptor(this);
}

Buteo::ClientPlugin* MastodonNotificationsPluginLoader::createClientPlugin(
        const QString& pluginName,
        const Buteo::SyncProfile& profile,
        Buteo::PluginCbInterface* cbInterface)
{
    return new MastodonNotificationsPlugin(pluginName, profile, cbInterface);
}
