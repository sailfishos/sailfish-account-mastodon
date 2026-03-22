# SPDX-FileCopyrightText: 2013 - 2026 Jolla Ltd.
#
# SPDX-License-Identifier: BSD-3-Clause

TEMPLATE = subdirs
SUBDIRS += \
    buteo-common \
    buteo-sync-plugin-mastodon-posts \
    buteo-sync-plugin-mastodon-notifications

buteo-sync-plugin-mastodon-posts.depends = buteo-common
buteo-sync-plugin-mastodon-notifications.depends = buteo-common
