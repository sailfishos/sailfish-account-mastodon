// SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
// SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
//
// SPDX-License-Identifier: BSD-3-Clause

#include <QQmlExtensionPlugin>
#include <QtQml>

#include "mastodonpostactions.h"
#include "mastodonpostsmodel.h"

class JollaEventsviewMastodonPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.jolla.eventsview.mastodon")

public:
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("com.jolla.eventsview.mastodon"));
        qmlRegisterType<MastodonPostsModel>(uri, 1, 0, "MastodonPostsModel");
        qmlRegisterType<MastodonPostActions>(uri, 1, 0, "MastodonPostActions");
    }
};

#include "plugin.moc"
