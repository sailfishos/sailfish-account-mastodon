# SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
# SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
#
# SPDX-License-Identifier: BSD-3-Clause

TEMPLATE = lib
TARGET = $$qtLibraryTarget(mastodonshareplugin)
CONFIG += plugin
DEPENDPATH += .
INCLUDEPATH += ..
INCLUDEPATH += ../../common

QT -= gui

CONFIG += link_pkgconfig
PKGCONFIG += nemotransferengine-qt5 accounts-qt5 sailfishaccounts libsignon-qt5

HEADERS += mastodonshareplugin.h \
           mastodonplugininfo.h \
           ../mastodonshareservicestatus.h

SOURCES += mastodonshareplugin.cpp \
           mastodonplugininfo.cpp \
           ../mastodonshareservicestatus.cpp

target.path = $$[QT_INSTALL_LIBS]/nemo-transferengine/plugins/sharing

OTHER_FILES += *.qml

shareui.files = MastodonSharePost.qml
shareui.path = /usr/share/nemo-transferengine/plugins/sharing

INSTALLS += target shareui
