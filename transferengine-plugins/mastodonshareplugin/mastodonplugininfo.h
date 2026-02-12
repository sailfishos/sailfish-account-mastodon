/*
 * Copyright (C) 2013-2026 Jolla Ltd.
 */

#ifndef MASTODONPLUGININFO_H
#define MASTODONPLUGININFO_H

#include <sharingplugininfo.h>
#include <QStringList>

class MastodonShareServiceStatus;

class MastodonPluginInfo : public SharingPluginInfo
{
    Q_OBJECT

public:
    MastodonPluginInfo();
    ~MastodonPluginInfo();

    QList<SharingMethodInfo> info() const;
    void query();

private Q_SLOTS:
    void serviceReady();

private:
    MastodonShareServiceStatus *m_mastodonShareServiceStatus;
    QList<SharingMethodInfo> m_info;
    QStringList m_capabilities;
};

#endif // MASTODONPLUGININFO_H
