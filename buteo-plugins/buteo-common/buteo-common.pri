INCLUDEPATH += $$PWD
DEPENDPATH += .

QT += dbus

CONFIG += link_pkgconfig
PKGCONFIG += accounts-qt5 buteosyncfw5 socialcache libsignon-qt5 libsailfishkeyprovider

LIBS += -L$$PWD -lmastodonbuteocommon
