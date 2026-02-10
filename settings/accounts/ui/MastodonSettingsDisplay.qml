import QtQuick 2.0
import Sailfish.Silica 1.0
import Sailfish.Accounts 1.0
import com.jolla.settings.accounts 1.0
import org.nemomobile.configuration 1.0

StandardAccountSettingsDisplay {
    id: root

    settingsModified: true

    onAboutToSaveAccount: {
        settingsLoader.updateAllSyncProfiles()

        if (eventsSyncSwitch.checked !== root.account.configurationValues("")["FeedViewAutoSync"]) {
            root.account.setConfigurationValue("", "FeedViewAutoSync", eventsSyncSwitch.checked)
        }
    }

    StandardAccountSettingsLoader {
        id: settingsLoader
        account: root.account
        accountProvider: root.accountProvider
        accountManager: root.accountManager
        autoEnableServices: root.autoEnableAccount

        onSettingsLoaded: {
            syncServicesRepeater.model = syncServices
            otherServicesDisplay.serviceModel = otherServices

            var autoSync = root.account.configurationValues("")["FeedViewAutoSync"]
            var isNewAccount = root.autoEnableAccount
            eventsSyncSwitch.checked = (isNewAccount || autoSync === true)
        }
    }

    Column {
        id: syncServicesDisplay
        width: parent.width

        SectionHeader {
            //: Options for data to be downloaded from a remote server
            //% "Download"
            text: qsTrId("settings-accounts-la-download_options")
        }

        Repeater {
            id: syncServicesRepeater
            TextSwitch {
                checked: model.enabled
                text: model.displayName
                visible: text.length > 0
                onCheckedChanged: {
                    if (checked) {
                        root.account.enableWithService(model.serviceName)
                    } else {
                        root.account.disableWithService(model.serviceName)
                    }
                }
            }
        }

        TextSwitch {
            id: eventsSyncSwitch

            text: "Sync Mastodon feed automatically"
            description: "Fetch new posts periodically when browsing Events Mastodon feed."

            onCheckedChanged: {
                autoSyncConf.value = checked
            }
        }
    }

    ConfigurationValue {
        id: autoSyncConf
        key: "/desktop/lipstick-jolla-home/events/auto_sync_feeds/" + root.account.identifier
    }

    AccountServiceSettingsDisplay {
        id: otherServicesDisplay
        enabled: root.accountEnabled

        onUpdateServiceEnabledStatus: {
            if (enabled) {
                root.account.enableWithService(serviceName)
            } else {
                root.account.disableWithService(serviceName)
            }
        }
    }
}
