# SPDX-FileCopyrightText: 2013 - 2026 Jolla Ltd.
#
# SPDX-License-Identifier: BSD-3-Clause

TEMPLATE = subdirs
SUBDIRS += \
    common \
    settings \
    transferengine-plugins \
    buteo-plugins \
    eventsview-plugins \
    icons

buteo-plugins.depends = common
transferengine-plugins.depends = common
eventsview-plugins.depends = common

OTHER_FILES += rpm/sailfish-account-mastodon.spec
