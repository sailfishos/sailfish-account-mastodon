/*
 * SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
 * SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
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
