/*
 * SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
 * SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef SOCIALDBUTEOPLUGIN_H
#define SOCIALDBUTEOPLUGIN_H

#include <QtCore/qglobal.h>
#include "buteosyncfw_p.h"

/*
   Datatype-specific implementations of this class
   allow per-account sync profiles for that data type.
*/

class SocialNetworkSyncAdaptor;
class Q_DECL_EXPORT SocialdButeoPlugin : public Buteo::ClientPlugin
{
    Q_OBJECT

protected:
    virtual SocialNetworkSyncAdaptor *createSocialNetworkSyncAdaptor() = 0;

public:
    SocialdButeoPlugin(const QString& pluginName,
                       const Buteo::SyncProfile& profile,
                       Buteo::PluginCbInterface *cbInterface,
                       const QString &socialServiceName,
                       const QString &dataTypeName);
    virtual ~SocialdButeoPlugin();

    bool init() override;
    bool uninit() override;
    bool startSync() override;
    void abortSync(Sync::SyncStatus status = Sync::SYNC_ABORTED) override;
    Buteo::SyncResults getSyncResults() const override;
    bool cleanUp() override;

public Q_SLOTS:
    void connectivityStateChanged(Sync::ConnectivityType type, bool state) override;

private Q_SLOTS:
    void syncStatusChanged();

protected:
    QList<Buteo::SyncProfile*> ensurePerAccountSyncProfilesExist();

private:
    void updateResults(const Buteo::SyncResults &results);
    Buteo::SyncResults m_syncResults;
    Buteo::ProfileManager m_profileManager;
    SocialNetworkSyncAdaptor *m_socialNetworkSyncAdaptor;
    QString m_socialServiceName;
    QString m_dataTypeName;
    int m_profileAccountId;
};

#endif // SOCIALDBUTEOPLUGIN_H
