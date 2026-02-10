import QtQuick 2.0
import Sailfish.Silica 1.0
import Sailfish.Accounts 1.0
import com.jolla.settings.accounts 1.0

AccountSettingsAgent {
    id: root

    property string accountSubtitle: {
        var credentialsUserName = account.defaultCredentialsUserName
                ? account.defaultCredentialsUserName.toString().trim()
                : ""
        if (credentialsUserName.length > 0) {
            return credentialsUserName
        }
        return account.displayName
    }

    Account {
        id: account
        identifier: root.accountId
    }

    initialPage: Page {
        onPageContainerChanged: {
            if (pageContainer == null && !credentialsUpdater.running) {
                root.delayDeletion = true
                settingsDisplay.saveAccount()
            }
        }

        Component.onDestruction: {
            if (status == PageStatus.Active) {
                settingsDisplay.saveAccount(true)
            }
        }

        AccountCredentialsUpdater {
            id: credentialsUpdater
        }

        SilicaFlickable {
            anchors.fill: parent
            contentHeight: header.height + settingsDisplay.height + Theme.paddingLarge

            StandardAccountSettingsPullDownMenu {
                visible: settingsDisplay.accountValid
                allowSync: true
                onCredentialsUpdateRequested: {
                    credentialsUpdater.replaceWithCredentialsUpdatePage(root.accountId)
                }
                onAccountDeletionRequested: {
                    root.accountDeletionRequested()
                    pageStack.pop()
                }
                onSyncRequested: {
                    settingsDisplay.saveAccountAndSync()
                }
            }

            PageHeader {
                id: header
                title: root.accountsHeaderText
                description: root.accountSubtitle
            }

            MastodonSettingsDisplay {
                id: settingsDisplay
                anchors.top: header.bottom
                accountManager: root.accountManager
                accountProvider: root.accountProvider
                accountId: root.accountId

                onAccountSaveCompleted: {
                    root.delayDeletion = false
                }
            }

            VerticalScrollDecorator {}
        }
    }
}
