# SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
# SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
#
# SPDX-License-Identifier: BSD-3-Clause

TEMPLATE = lib
TARGET = mastodonaccountstranslationsplugin
TARGET = $$qtLibraryTarget($$TARGET)

MODULENAME = com/jolla/settings/accounts/mastodon
TARGETPATH = $$[QT_INSTALL_QML]/$$MODULENAME

QT += qml
CONFIG += plugin

SOURCES += plugin.cpp

target.path = $$TARGETPATH
qmldir.files = qmldir
qmldir.path = $$TARGETPATH

INSTALLS += target qmldir
