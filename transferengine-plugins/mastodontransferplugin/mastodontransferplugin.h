/*
 * Copyright (C) 2013-2026 Jolla Ltd.
 */

#ifndef MASTODONTRANSFERPLUGIN_H
#define MASTODONTRANSFERPLUGIN_H

#include <QtCore/QObject>

#include <transferplugininterface.h>

class QNetworkAccessManager;

class Q_DECL_EXPORT MastodonTransferPlugin : public QObject, public TransferPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.sailfishos.transfer.plugin.mastodon")
    Q_INTERFACES(TransferPluginInterface)

public:
    MastodonTransferPlugin();
    ~MastodonTransferPlugin();

    MediaTransferInterface *transferObject();
    QString pluginId() const;

private:
    QNetworkAccessManager *m_qnam;
};

#endif // MASTODONTRANSFERPLUGIN_H
