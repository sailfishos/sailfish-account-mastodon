// SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
// SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
//
// SPDX-License-Identifier: BSD-3-Clause

import QtQuick 2.0
import Sailfish.Silica 1.0
import Sailfish.Accounts 1.0
import com.jolla.settings.accounts 1.0
import com.jolla.settings.accounts.mastodon 1.0

AccountCreationAgent {
    id: root

    property Item _oauthPage
    property Item _settingsDialog
    property QtObject _accountSetup

    property string _pendingApiHost
    property bool _registering

    readonly property string callbackUri: "http://ipv4.jolla.com/online/status.html"
    readonly property string defaultApiHost: "https://mastodon.social"

    function normalizeApiHost(rawHost) {
        var host = rawHost ? rawHost.trim() : ""
        if (host.length === 0) {
            return ""
        }

        host = host.replace(/^https?:\/\//i, "")
        var pathSeparator = host.indexOf("/")
        if (pathSeparator !== -1) {
            host = host.substring(0, pathSeparator)
        }
        host = host.replace(/\/+$/, "")

        if (host.length === 0) {
            return ""
        }
        return "https://" + host.toLowerCase()
    }

    function oauthHost(apiHost) {
        return apiHost.replace(/^https?:\/\//i, "")
    }

    function _displayName(apiHost) {
        return oauthHost(apiHost)
    }

    function _fallbackDisplayName(apiHost) {
        var display = _displayName(apiHost)
        if (display.length > 0) {
            return display
        }
        return _displayName(defaultApiHost)
    }

    function _showRegistrationError(message, busyPage) {
        _registering = false
        accountCreationError(message)
        if (busyPage) {
            busyPage.state = "info"
            busyPage.infoDescription = message
            busyPage.infoExtraDescription = ""
            busyPage.infoButtonText = ""
        }
    }

    function _showOAuthPage(context) {
        _registering = false
        if (_oauthPage != null) {
            _oauthPage.cancelSignIn()
            _oauthPage.destroy()
        }
        _oauthPage = oAuthComponent.createObject(root, { "context": context })
        pageStack.replace(_oauthPage)
    }

    function _registerClientApplication(apiHost, busyPage) {
        if (_registering) {
            return
        }
        _registering = true

        var xhr = new XMLHttpRequest()
        xhr.onreadystatechange = function() {
            if (xhr.readyState !== XMLHttpRequest.DONE) {
                return
            }

            if (xhr.status < 200 || xhr.status >= 300) {
                //% "Failed to register Mastodon app for %1"
                _showRegistrationError(qsTrId("settings-accounts-mastodon-la-register_app_failed").arg(apiHost), busyPage)
                return
            }

            var response
            try {
                response = JSON.parse(xhr.responseText)
            } catch (err) {
                //% "Invalid Mastodon app registration response"
                _showRegistrationError(qsTrId("settings-accounts-mastodon-la-invalid_app_registration_response"), busyPage)
                return
            }

            if (!response.client_id || !response.client_secret) {
                //% "Mastodon app registration did not return credentials"
                _showRegistrationError(qsTrId("settings-accounts-mastodon-la-app_registration_missing_credentials"), busyPage)
                return
            }

            _showOAuthPage({
                "apiHost": apiHost,
                "oauthHost": oauthHost(apiHost),
                "clientId": response.client_id,
                "clientSecret": response.client_secret
            })
        }

        var postData = []
        //% "Mastodon in SailfishOS"
        postData.push("client_name=" + encodeURIComponent(qsTrId("settings-accounts-mastodon-la-client_name")))
        postData.push("redirect_uris=" + encodeURIComponent(callbackUri))
        postData.push("scopes=" + encodeURIComponent("read write"))
        postData.push("website=" + encodeURIComponent("https://sailfishos.org"))

        xhr.open("POST", apiHost + "/api/v1/apps")
        xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded")
        xhr.send(postData.join("&"))
    }

    function _extractAccountName(responseData) {
        if (!responseData) {
            return ""
        }

        var candidates = [
            "AccountUsername",
            "UserName",
            "user_name",
            "acct",
            "username",
            "preferred_username",
            "login",
            "ScreenName"
        ]
        for (var i = 0; i < candidates.length; ++i) {
            var value = responseData[candidates[i]]
            if (value) {
                var userName = value.toString().trim()
                if (userName.length > 0) {
                    return userName
                }
            }
        }

        return ""
    }

    function _formatMastodonAccountId(accountName, apiHost) {
        var value = accountName ? accountName.toString().trim() : ""
        if (value.length === 0) {
            return ""
        }

        value = value.replace(/^@+/, "")
        if (value.indexOf("@") !== -1) {
            return "@" + value
        }

        var host = oauthHost(apiHost)
        if (host.length === 0) {
            return ""
        }

        return "@" + value + "@" + host
    }

    function _isMastodonAccountId(value) {
        var text = value ? value.toString().trim() : ""
        return /^@[^@]+@[^@]+$/.test(text)
    }

    function _extractAccessToken(responseData) {
        if (!responseData) {
            return ""
        }

        var token = responseData["AccessToken"]
        if (!token || token.toString().trim().length === 0) {
            token = responseData["access_token"]
        }
        return token ? token.toString().trim() : ""
    }

    function _handleAccountCreated(accountId, context, responseData) {
        var props = {
            "accountId": accountId,
            "apiHost": context.apiHost,
            "oauthHost": context.oauthHost,
            "clientId": context.clientId,
            "clientSecret": context.clientSecret,
            "accessToken": _extractAccessToken(responseData),
            "accountDescription": _formatMastodonAccountId(_extractAccountName(responseData), context.apiHost)
        }
        _accountSetup = accountSetupComponent.createObject(root, props)
        _accountSetup.done.connect(function() {
            accountCreated(accountId)
            _goToSettings(accountId)
        })
        _accountSetup.error.connect(function() {
            //% "Failed to finish Mastodon account setup"
            accountCreationError(qsTrId("settings-accounts-mastodon-la-account_setup_failed"))
        })
    }

    function _goToSettings(accountId) {
        if (_settingsDialog != null) {
            _settingsDialog.destroy()
        }
        _settingsDialog = settingsComponent.createObject(root, {"accountId": accountId})
        pageStack.replace(_settingsDialog)
    }

    initialPage: Dialog {
        id: setupDialog

        property string normalizedHost: root.normalizeApiHost(instanceField.text)

        canAccept: !root._registering
        acceptDestinationAction: PageStackAction.Push
        acceptDestination: busyComponent

        onAccepted: {
            root._pendingApiHost = normalizedHost.length > 0 ? normalizedHost : root.defaultApiHost
        }

        DialogHeader {
            id: header

            //% "Sign in"
            acceptText: qsTrId("settings-accounts-common-bt-sign_in")
        }

        Column {
            anchors.top: header.bottom
            anchors.topMargin: Theme.paddingLarge
            spacing: Theme.paddingLarge
            width: parent.width

            Row {
                x: Theme.horizontalPageMargin
                width: parent.width - x * 2
                spacing: Theme.paddingMedium

                Image {
                    id: promptIcon

                    width: Theme.iconSizeMedium
                    height: Theme.iconSizeMedium
                    source: "image://theme/icon-l-mastodon"
                    fillMode: Image.PreserveAspectFit
                    sourceSize.width: width
                    sourceSize.height: height
                }

                Label {
                    width: parent.width - promptIcon.width - parent.spacing
                    wrapMode: Text.Wrap
                    color: Theme.highlightColor
                    //: Prompt shown in account setup before OAuth sign-in.
                    //% "Enter your Mastodon server, then sign in."
                    text: qsTrId("settings-accounts-mastodon-la-enter_server_then_sign_in")
                }
            }

            TextField {
                id: instanceField

                x: Theme.horizontalPageMargin
                width: parent.width - x * 2
                //% "Server"
                label: qsTrId("settings-accounts-mastodon-la-server")
                placeholderText: "mastodon.social"
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhUrlCharactersOnly
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: {
                    if (setupDialog.canAccept) {
                        setupDialog.accept()
                    }
                }
            }
        }
    }

    Component {
        id: busyComponent
        AccountBusyPage {
            //% "Preparing Mastodon sign-in..."
            busyDescription: qsTrId("settings-accounts-mastodon-la-preparing_sign_in")
            onStatusChanged: {
                if (status === PageStatus.Active && root._pendingApiHost.length > 0) {
                    root._registerClientApplication(root._pendingApiHost, this)
                }
            }
        }
    }

    Component {
        id: oAuthComponent
        OAuthAccountSetupPage {
            property var context

            Component.onCompleted: {
                var sessionData = {
                    "ClientId": context.clientId,
                    "ClientSecret": context.clientSecret,
                    "Host": context.oauthHost,
                    "AuthPath": "oauth/authorize",
                    "TokenPath": "oauth/token",
                    "ResponseType": "code",
                    "Scope": ["read", "write"],
                    "RedirectUri": root.callbackUri
                }
                prepareAccountCreation(root.accountProvider, "mastodon-microblog", sessionData)
            }

            onAccountCreated: {
                root._handleAccountCreated(accountId, context, responseData)
            }

            onAccountCreationError: {
                root.accountCreationError(errorMessage)
            }
        }
    }

    Component {
        id: accountSetupComponent
        QtObject {
            id: accountSetup

            property int accountId
            property string apiHost
            property string oauthHost
            property string clientId
            property string clientSecret
            property string accessToken
            property string accountDescription
            property bool hasConfigured

            signal done()
            signal error()

            property Account newAccount: Account {
                identifier: accountSetup.accountId

                onStatusChanged: {
                    if (status === Account.Initialized || status === Account.Synced) {
                        if (!accountSetup.hasConfigured) {
                            accountSetup.configure()
                        } else {
                            accountSetup.done()
                        }
                    } else if (status === Account.Invalid && accountSetup.hasConfigured) {
                        accountSetup.error()
                    }
                }
            }

            function configure() {
                hasConfigured = true

                var services = ["mastodon-microblog", "mastodon-notifications", "mastodon-sharing"]
                var providerDisplayName = root.accountProvider && root.accountProvider.displayName
                        ? root.accountProvider.displayName.toString().trim()
                        : ""
                if (providerDisplayName.length === 0) {
                    //% "Mastodon"
                    providerDisplayName = qsTrId("settings-accounts-mastodon-la-provider_name")
                }
                newAccount.displayName = providerDisplayName

                newAccount.setConfigurationValue("", "api/Host", apiHost)
                newAccount.setConfigurationValue("", "FeedViewAutoSync", true)
                if (accountDescription.length > 0) {
                    newAccount.setConfigurationValue("", "description", accountDescription)
                    if (root._isMastodonAccountId(accountDescription)) {
                        newAccount.setConfigurationValue("", "default_credentials_username", accountDescription)
                    }
                } else {
                    var hostDisplayName = root._fallbackDisplayName(apiHost)
                    if (hostDisplayName.length > 0) {
                        newAccount.setConfigurationValue("", "description", hostDisplayName)
                    }
                }

                for (var i = 0; i < services.length; ++i) {
                    var service = services[i]
                    newAccount.setConfigurationValue(service, "api/Host", apiHost)
                    newAccount.setConfigurationValue(service, "auth/oauth2/web_server/Host", oauthHost)
                    newAccount.setConfigurationValue(service, "auth/oauth2/web_server/AuthPath", "oauth/authorize")
                    newAccount.setConfigurationValue(service, "auth/oauth2/web_server/TokenPath", "oauth/token")
                    newAccount.setConfigurationValue(service, "auth/oauth2/web_server/ResponseType", "code")
                    newAccount.setConfigurationValue(service, "auth/oauth2/web_server/RedirectUri", root.callbackUri)
                    newAccount.setConfigurationValue(service, "auth/oauth2/web_server/Scope", ["read", "write"])
                    newAccount.setConfigurationValue(service, "auth/oauth2/web_server/ClientId", clientId)
                    newAccount.setConfigurationValue(service, "auth/oauth2/web_server/ClientSecret", clientSecret)
                }

                for (var j = 0; j < services.length; ++j) {
                    newAccount.enableWithService(services[j])
                }

                if (accountDescription.length > 0 || accessToken.length === 0) {
                    newAccount.sync()
                    return
                }

                var xhr = new XMLHttpRequest()
                xhr.onreadystatechange = function() {
                    if (xhr.readyState !== XMLHttpRequest.DONE) {
                        return
                    }

                    if (xhr.status >= 200 && xhr.status < 300) {
                        try {
                            var response = JSON.parse(xhr.responseText)
                            var fetchedDescription = root._formatMastodonAccountId(root._extractAccountName(response), apiHost)
                            if (fetchedDescription.length > 0) {
                                accountDescription = fetchedDescription
                                newAccount.setConfigurationValue("", "description", fetchedDescription)
                                if (root._isMastodonAccountId(fetchedDescription)) {
                                    newAccount.setConfigurationValue("", "default_credentials_username", fetchedDescription)
                                }
                            }
                        } catch (err) {
                        }
                    }

                    newAccount.sync()
                }

                xhr.open("GET", apiHost + "/api/v1/accounts/verify_credentials")
                xhr.setRequestHeader("Authorization", "Bearer " + accessToken)
                xhr.send()
            }
        }
    }

    Component {
        id: settingsComponent
        Dialog {
            property alias accountId: settingsDisplay.accountId

            acceptDestination: root.endDestination
            acceptDestinationAction: root.endDestinationAction
            acceptDestinationProperties: root.endDestinationProperties
            acceptDestinationReplaceTarget: root.endDestinationReplaceTarget
            backNavigation: false

            onAccepted: {
                root.delayDeletion = true
                settingsDisplay.saveAccountAndSync()
            }

            SilicaFlickable {
                anchors.fill: parent
                contentHeight: header.height + settingsDisplay.height

                DialogHeader {
                    id: header
                }

                MastodonSettingsDisplay {
                    id: settingsDisplay

                    anchors.top: header.bottom
                    accountManager: root.accountManager
                    accountProvider: root.accountProvider
                    autoEnableAccount: true

                    onAccountSaveCompleted: {
                        root.delayDeletion = false
                    }
                }

                VerticalScrollDecorator {}
            }
        }
    }
}
