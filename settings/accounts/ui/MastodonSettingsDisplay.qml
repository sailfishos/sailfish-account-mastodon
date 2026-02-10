import QtQuick 2.0
import Sailfish.Silica 1.0
import Sailfish.Accounts 1.0
import com.jolla.settings.accounts 1.0
import org.nemomobile.configuration 1.0

StandardAccountSettingsDisplay {
    id: root

    settingsModified: true

    function _displayNameFromApiHost(apiHost) {
        var host = apiHost ? apiHost.toString().trim() : ""
        host = host.replace(/^https?:\/\//i, "")
        var pathSeparator = host.indexOf("/")
        if (pathSeparator !== -1) {
            host = host.substring(0, pathSeparator)
        }
        return host
    }

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

            var credentialsUserName = root.account.defaultCredentialsUserName
                    ? root.account.defaultCredentialsUserName.toString().trim()
                    : ""
            if (credentialsUserName.length > 0 && root.account.displayName !== credentialsUserName) {
                root.account.displayName = credentialsUserName
            } else if ((!root.account.displayName || root.account.displayName.toString().trim().length === 0)) {
                var fallback = _displayNameFromApiHost(root.account.configurationValues("")["api/Host"])
                if (fallback.length > 0) {
                    root.account.displayName = fallback
                }
            }

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
