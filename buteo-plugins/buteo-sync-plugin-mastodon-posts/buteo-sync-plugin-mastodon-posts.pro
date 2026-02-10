TARGET = mastodon-posts-client

QT -= gui

include($$PWD/../buteo-common/buteo-common.pri)
include($$PWD/../../common/common.pri)

CONFIG += link_pkgconfig
PKGCONFIG += mlite5 nemonotifications-qt5

INCLUDEPATH += $$PWD

SOURCES += \
    $$PWD/mastodondatatypesyncadaptor.cpp \
    $$PWD/mastodonpostsplugin.cpp \
    $$PWD/mastodonpostssyncadaptor.cpp

HEADERS += \
    $$PWD/mastodondatatypesyncadaptor.h \
    $$PWD/mastodonpostsplugin.h \
    $$PWD/mastodonpostssyncadaptor.h

OTHER_FILES += \
    $$PWD/mastodon-posts.xml \
    $$PWD/mastodon.Posts.xml

TEMPLATE = lib
CONFIG += plugin
target.path = $$[QT_INSTALL_LIBS]/buteo-plugins-qt5/oopp

sync.path = /etc/buteo/profiles/sync
sync.files = mastodon.Posts.xml

client.path = /etc/buteo/profiles/client
client.files = mastodon-posts.xml

INSTALLS += target sync client
