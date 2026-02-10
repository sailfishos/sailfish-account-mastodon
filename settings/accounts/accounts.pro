TEMPLATE = aux

TS_FILE = $$OUT_PWD/settings-accounts-mastodon.ts
EE_QM = $$OUT_PWD/settings-accounts-mastodon_eng_en.qm

ts.commands += lupdate $$PWD/ui -ts $$TS_FILE
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

INSTALLS += provider services ui ts_install engineering_english_install
