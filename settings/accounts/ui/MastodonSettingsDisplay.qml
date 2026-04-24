// SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
//
// SPDX-License-Identifier: BSD-3-Clause

import QtQuick 2.0
import Sailfish.Silica 1.0
import Sailfish.Accounts 1.0
import com.jolla.settings.accounts 1.0
import com.jolla.settings.accounts.mastodon 1.0
import Nemo.Configuration 1.0

StandardAccountSettingsDisplay {
    id: root

    property bool postsServiceEnabled
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

        return providerDisplayName.length > 0 ? providerDisplayName
                                              : //% "Mastodon"
                                                qsTrId("settings-accounts-mastodon-la-provider_name")
    }

    onAboutToSaveAccount: {
        settingsLoader.updateAllSyncProfiles()

        var storedDescriptionValue = root.account.configurationValues("")["description"]
        var storedDescription = storedDescriptionValue ? storedDescriptionValue.toString().trim() : ""
        var storedCredentialsUserName = root.account.defaultCredentialsUserName
                ? root.account.defaultCredentialsUserName.toString().trim()
                : ""
        var editedDescription = root.account.displayName
                ? root.account.displayName.toString().trim()
                : ""
        var providerDisplayName = _providerDisplayName()
        if (editedDescription === providerDisplayName) {
            // Avoid clobbering stored handle if displayName temporarily reverts to provider name.
            editedDescription = storedDescription.length > 0 ? storedDescription : storedCredentialsUserName
        }

        if (storedDescription !== editedDescription) {
            root.account.setConfigurationValue("", "description", editedDescription)
        }

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
                text: model.serviceName === "mastodon-microblog"
                        ? //% "Posts"
                          qsTrId("settings-accounts-mastodon-la-service_posts")
                        : (model.serviceName === "mastodon-notifications"
                           ? //% "Notifications"
                             qsTrId("settings-accounts-mastodon-la-service_notifications")
                           : model.displayName)
                description: model.serviceName === "mastodon-microblog"
                        ? //% "Show Mastodon posts in the Events view."
                          qsTrId("settings-accounts-mastodon-la-service_posts_description")
                        : (model.serviceName === "mastodon-notifications"
                           ? //% "Show Mastodon notifications."
                             qsTrId("settings-accounts-mastodon-la-service_notifications_description")
                           : "")
                visible: text.length > 0
                onCheckedChanged: {
                    if (model.serviceName === "mastodon-microblog") {
                        root.postsServiceEnabled = checked
                    }
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

            //% "Sync Mastodon feed automatically"
            text: qsTrId("settings-accounts-mastodon-la-auto_sync_feed")
            //% "Fetch new posts periodically when browsing Events Mastodon feed."
            description: qsTrId("settings-accounts-mastodon-la-auto_sync_feed_description")
            enabled: root.postsServiceEnabled

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
