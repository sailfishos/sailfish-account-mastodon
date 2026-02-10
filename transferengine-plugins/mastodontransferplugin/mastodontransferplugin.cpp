#include "mastodontransferplugin.h"
#include "mastodonuploader.h"

#include <QtPlugin>
#include <QNetworkAccessManager>

MastodonTransferPlugin::MastodonTransferPlugin()
    : QObject(), TransferPluginInterface()
    , m_qnam(new QNetworkAccessManager(this))
{
}

MastodonTransferPlugin::~MastodonTransferPlugin()
{
}

MediaTransferInterface *MastodonTransferPlugin::transferObject()
{
    return new MastodonUploader(m_qnam, this);
}

QString MastodonTransferPlugin::pluginId() const
{
    return QLatin1String("Mastodon");
}
