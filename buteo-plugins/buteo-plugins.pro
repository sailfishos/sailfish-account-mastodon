# SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
# SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
#
# SPDX-License-Identifier: BSD-3-Clause

TEMPLATE = subdirs
SUBDIRS += \
    buteo-common \
    buteo-sync-plugin-mastodon-posts \
    buteo-sync-plugin-mastodon-notifications

buteo-sync-plugin-mastodon-posts.depends = buteo-common
buteo-sync-plugin-mastodon-notifications.depends = buteo-common
