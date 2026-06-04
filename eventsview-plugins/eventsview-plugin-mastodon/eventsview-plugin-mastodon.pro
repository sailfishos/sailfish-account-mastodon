# SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
# SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
#
# SPDX-License-Identifier: BSD-3-Clause

TEMPLATE = lib
TARGET = jollaeventsviewmastodonplugin
TARGET = $$qtLibraryTarget($$TARGET)

MODULENAME = com/jolla/eventsview/mastodon
TARGETPATH = $$[QT_INSTALL_QML]/$$MODULENAME

QT -= gui
QT += qml network
CONFIG += plugin link_pkgconfig
PKGCONFIG += socialcache accounts-qt5 libsignon-qt5 sailfishaccounts

include($$PWD/../../common/common.pri)

HEADERS += \
    mastodonpostactions.h \
    mastodonpostsmodel.h

SOURCES += \
    mastodonpostactions.cpp \
    mastodonpostsmodel.cpp \
    plugin.cpp

qml.files = mastodon-delegate.qml MastodonFeedItem.qml
qml.path = /usr/share/lipstick/eventfeed/

import.files = qmldir
import.path = $$TARGETPATH
target.path = $$TARGETPATH

OTHER_FILES += $$qml.files $$import.files

INSTALLS += target import qml
