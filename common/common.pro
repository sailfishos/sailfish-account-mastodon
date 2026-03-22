# SPDX-FileCopyrightText: 2013 - 2026 Jolla Ltd.
#
# SPDX-License-Identifier: BSD-3-Clause

TEMPLATE = lib

QT -= gui
QT += sql

CONFIG += link_pkgconfig
PKGCONFIG += socialcache

TARGET = mastodoncommon
TARGET = $$qtLibraryTarget($$TARGET)

HEADERS += \
    $$PWD/mastodonauthutils.h \
    $$PWD/mastodontextutils.h \
    $$PWD/mastodonpostsdatabase.h

SOURCES += \
    $$PWD/mastodonpostsdatabase.cpp

TARGETPATH = $$[QT_INSTALL_LIBS]
target.path = $$TARGETPATH

INSTALLS += target
