# SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
# SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
#
# SPDX-License-Identifier: BSD-3-Clause

TARGET = mastodon-notifications-client

QT -= gui

include($$PWD/../../common/common.pri)

CONFIG += link_pkgconfig
PKGCONFIG += \
    buteosocialcommon \
    accounts-qt5 \
    buteosyncfw5 \
    libsignon-qt5 \
    nemonotifications-qt5

INCLUDEPATH += $$PWD

SOURCES += \
    $$PWD/mastodondatatypesyncadaptor.cpp \
    $$PWD/mastodonnotificationsplugin.cpp \
    $$PWD/mastodonnotificationssyncadaptor.cpp

HEADERS += \
    $$PWD/mastodondatatypesyncadaptor.h \
    $$PWD/mastodonnotificationsplugin.h \
    $$PWD/mastodonnotificationssyncadaptor.h

OTHER_FILES += \
    $$PWD/mastodon-notifications.xml \
    $$PWD/mastodon.Notifications.xml

TEMPLATE = lib
CONFIG += plugin
target.path = $$[QT_INSTALL_LIBS]/buteo-plugins-qt5/oopp

sync.path = /etc/buteo/profiles/sync
sync.files = mastodon.Notifications.xml

client.path = /etc/buteo/profiles/client
client.files = mastodon-notifications.xml

INSTALLS += target sync client
