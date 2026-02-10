#include "mastodonplugininfo.h"
#include "mastodonshareservicestatus.h"

MastodonPluginInfo::MastodonPluginInfo()
    : SharingPluginInfo()
    , m_mastodonShareServiceStatus(new MastodonShareServiceStatus(this))
{
    m_capabilities << QLatin1String("image/jpeg")
                   << QLatin1String("image/png");

    connect(m_mastodonShareServiceStatus, &MastodonShareServiceStatus::serviceReady,
            this, &MastodonPluginInfo::serviceReady);
    connect(m_mastodonShareServiceStatus, &MastodonShareServiceStatus::serviceError,
            this, &MastodonPluginInfo::infoError);
}

MastodonPluginInfo::~MastodonPluginInfo()
{
}

QList<SharingMethodInfo> MastodonPluginInfo::info() const
{
    return m_info;
}

void MastodonPluginInfo::query()
{
    m_mastodonShareServiceStatus->queryStatus(MastodonShareServiceStatus::PassiveMode);
}

void MastodonPluginInfo::serviceReady()
{
    m_info.clear();

    for (int i = 0; i < m_mastodonShareServiceStatus->count(); ++i) {
        SharingMethodInfo info;

        const MastodonShareServiceStatus::AccountDetails details = m_mastodonShareServiceStatus->details(i);
        info.setDisplayName(details.providerName);
        info.setSubtitle(details.displayName);
        info.setAccountId(details.accountId);

        info.setMethodId(QLatin1String("Mastodon"));
        info.setMethodIcon(QLatin1String("image://theme/icon-l-mastodon"));
        info.setShareUIPath(QLatin1String("/usr/share/nemo-transferengine/plugins/sharing/MastodonShareImage.qml"));
        info.setCapabilities(m_capabilities);

        m_info << info;
    }

    emit infoReady();
}
