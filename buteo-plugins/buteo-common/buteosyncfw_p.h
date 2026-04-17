/*
 * SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
 * SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef SOCIALD_BUTEOSYNCFW_P_H
#define SOCIALD_BUTEOSYNCFW_P_H

#include <SyncCommonDefs.h>
#include <SyncPluginBase.h>
#include <ProfileManager.h>
#include <ClientPlugin.h>
#include <SyncResults.h>
#include <ProfileEngineDefs.h>
#include <SyncProfile.h>
#include <Profile.h>
#include <PluginCbInterface.h>

#ifndef SOCIALD_TEST_DEFINE
#define PRIVILEGED_DATA_DIR QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QLatin1String("/.local/share/system/privileged")
#endif

#endif // SOCIALD_BUTEOSYNCFW_P_H
