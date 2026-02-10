TEMPLATE = lib

QT -= gui
QT += sql

CONFIG += link_pkgconfig
PKGCONFIG += socialcache

TARGET = mastodoncommon
TARGET = $$qtLibraryTarget($$TARGET)

HEADERS += \
    $$PWD/mastodonpostsdatabase.h

SOURCES += \
    $$PWD/mastodonpostsdatabase.cpp

TARGETPATH = $$[QT_INSTALL_LIBS]
target.path = $$TARGETPATH

INSTALLS += target
