TEMPLATE = aux

OTHER_FILES += \
    $$PWD/providers/mastodon.provider \
    $$PWD/services/mastodon-microblog.service \
    $$PWD/services/mastodon-sharing.service \
    $$PWD/ui/MastodonSettingsDisplay.qml \
    $$PWD/ui/mastodon.qml \
    $$PWD/ui/mastodon-settings.qml \
    $$PWD/ui/mastodon-update.qml

provider.files += $$PWD/providers/mastodon.provider
provider.path = /usr/share/accounts/providers/

services.files += \
    $$PWD/services/mastodon-microblog.service \
    $$PWD/services/mastodon-sharing.service
services.path = /usr/share/accounts/services/

ui.files += \
    $$PWD/ui/MastodonSettingsDisplay.qml \
    $$PWD/ui/mastodon.qml \
    $$PWD/ui/mastodon-settings.qml \
    $$PWD/ui/mastodon-update.qml
ui.path = /usr/share/accounts/ui/

INSTALLS += provider services ui
