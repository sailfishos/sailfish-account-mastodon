# SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
# SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
#
# SPDX-License-Identifier: BSD-3-Clause

TEMPLATE = lib

TARGET = mastodonbuteocommon
TARGET = $$qtLibraryTarget($$TARGET)

QT -= gui
QT += network dbus
CONFIG += link_pkgconfig
PKGCONFIG += accounts-qt5 buteosyncfw5 socialcache

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/buteosyncfw_p.h \
    $$PWD/socialdbuteoplugin.h \
    $$PWD/socialnetworksyncadaptor.h \
    $$PWD/socialdnetworkaccessmanager_p.h \
    $$PWD/trace.h

SOURCES += \
    $$PWD/socialdbuteoplugin.cpp \
    $$PWD/socialnetworksyncadaptor.cpp \
    $$PWD/socialdnetworkaccessmanager_p.cpp \
    $$PWD/trace.cpp

TARGETPATH = $$[QT_INSTALL_LIBS]
target.path = $$TARGETPATH

INSTALLS += target
