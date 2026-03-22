# SPDX-FileCopyrightText: 2013 - 2026 Jolla Ltd.
#
# SPDX-License-Identifier: BSD-3-Clause

TARGET = mastodon-notifications-client

QT -= gui

include($$PWD/../buteo-common/buteo-common.pri)
include($$PWD/../../common/common.pri)

TS_FILE = $$OUT_PWD/lipstick-jolla-home-mastodon-notifications.ts
EE_QM = $$OUT_PWD/lipstick-jolla-home-mastodon-notifications_eng_en.qm

ts.commands += lupdate $$PWD -ts $$TS_FILE
ts.CONFIG += no_check_exist no_link
ts.output = $$TS_FILE
ts.input = .

ts_install.files = $$TS_FILE
ts_install.path = /usr/share/translations/source
ts_install.CONFIG += no_check_exist

engineering_english.commands += lrelease -idbased $$TS_FILE -qm $$EE_QM
engineering_english.CONFIG += no_check_exist no_link
engineering_english.depends = ts
engineering_english.input = $$TS_FILE
engineering_english.output = $$EE_QM

engineering_english_install.path = /usr/share/translations
engineering_english_install.files = $$EE_QM
engineering_english_install.CONFIG += no_check_exist

QMAKE_EXTRA_TARGETS += ts engineering_english
PRE_TARGETDEPS += ts engineering_english

CONFIG += link_pkgconfig
PKGCONFIG += mlite5 nemonotifications-qt5

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

INSTALLS += target sync client ts_install engineering_english_install
