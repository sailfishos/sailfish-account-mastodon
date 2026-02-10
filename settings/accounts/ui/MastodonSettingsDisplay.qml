import QtQuick 2.0
import Sailfish.Silica 1.0
import Sailfish.Accounts 1.0
import com.jolla.settings.accounts 1.0
import org.nemomobile.configuration 1.0

StandardAccountSettingsDisplay {
    id: root

    settingsModified: true

    function refreshDescriptionEditor() {
        var description = root.account.configurationValues("")["description"]
        var descriptionValue = description ? description.toString().trim() : ""
        var credentialsUserName = root.account.defaultCredentialsUserName
                ? root.account.defaultCredentialsUserName.toString().trim()
                : ""
        if (descriptionValue.length > 0 && credentialsUserName !== descriptionValue) {
            root.account.setConfigurationValue("", "default_credentials_username", descriptionValue)
        }

        // Reuse the standard "Description" field as the account handle editor.
        if (descriptionValue.length > 0) {
            root.account.displayName = descriptionValue
        } else if (credentialsUserName.length > 0) {
            root.account.displayName = credentialsUserName
        } else {
            root.account.displayName = ""
        }
    }

    function _providerDisplayName() {
        var providerDisplayName = root.accountProvider && root.accountProvider.displayName
                ? root.accountProvider.displayName.toString().trim()
                : ""
        return providerDisplayName.length > 0 ? providerDisplayName : "Mastodon"
    }

    onAboutToSaveAccount: {
        settingsLoader.updateAllSyncProfiles()

        var editedDescription = root.account.displayName
                ? root.account.displayName.toString().trim()
                : ""
        var providerDisplayName = _providerDisplayName()
        if (editedDescription === providerDisplayName) {
            editedDescription = ""
        }

        var storedDescriptionValue = root.account.configurationValues("")["description"]
        var storedDescription = storedDescriptionValue ? storedDescriptionValue.toString().trim() : ""
        if (storedDescription !== editedDescription) {
            root.account.setConfigurationValue("", "description", editedDescription)
        }

        var storedCredentialsUserName = root.account.defaultCredentialsUserName
                ? root.account.defaultCredentialsUserName.toString().trim()
                : ""
        if (storedCredentialsUserName !== editedDescription) {
            root.account.setConfigurationValue("", "default_credentials_username", editedDescription)
        }

        // Keep account list title fixed to provider name.
        root.account.displayName = providerDisplayName

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

            refreshDescriptionEditor()

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
                description: model.serviceName === "mastodon-microblog"
                        ? "Show Mastodon posts in the Events view."
                        : ""
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
