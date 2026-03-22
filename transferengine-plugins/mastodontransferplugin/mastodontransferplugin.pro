# SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
# SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
#
# SPDX-License-Identifier: BSD-3-Clause

TEMPLATE = lib
TARGET = $$qtLibraryTarget(mastodontransferplugin)
CONFIG += plugin
DEPENDPATH += .
INCLUDEPATH += ..
INCLUDEPATH += ../../common

QT -= gui
QT += network

CONFIG += link_pkgconfig
PKGCONFIG += nemotransferengine-qt5 accounts-qt5 sailfishaccounts libsignon-qt5

HEADERS += mastodontransferplugin.h \
           mastodonuploader.h \
           ../mastodonshareservicestatus.h \
           mastodonapi.h

SOURCES += mastodontransferplugin.cpp \
           mastodonuploader.cpp \
           ../mastodonshareservicestatus.cpp \
           mastodonapi.cpp

target.path = $$[QT_INSTALL_LIBS]/nemo-transferengine/plugins/transfer

INSTALLS += target
