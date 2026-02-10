TEMPLATE = lib
TARGET = $$qtLibraryTarget(mastodontransferplugin)
CONFIG += plugin
DEPENDPATH += .
INCLUDEPATH += ..

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
