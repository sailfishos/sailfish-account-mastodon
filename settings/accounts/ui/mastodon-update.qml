/*
 * Copyright (C) 2013-2026 Jolla Ltd.
 */

import QtQuick 2.0
import Sailfish.Silica 1.0
import Sailfish.Accounts 1.0
import com.jolla.settings.accounts 1.0
import com.jolla.settings.accounts.mastodon 1.0

AccountCredentialsAgent {
    id: root

    property bool _started

    readonly property string callbackUri: "http://ipv4.jolla.com/online/status.html"

    function normalizeApiHost(rawHost) {
        var host = rawHost ? rawHost.trim() : ""
        if (host.length === 0) {
            host = "https://mastodon.social"
        }

        host = host.replace(/^https?:\/\//i, "")
        var pathSeparator = host.indexOf("/")
        if (pathSeparator !== -1) {
            host = host.substring(0, pathSeparator)
        }
        host = host.replace(/\/+$/, "")

        if (host.length === 0) {
            host = "mastodon.social"
        }
        return "https://" + host.toLowerCase()
    }

    function _valueFromServiceConfig(config, key) {
        return config && config[key] ? config[key].toString() : ""
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

        var host = apiHost.replace(/^https?:\/\//i, "")
        if (host.length === 0) {
            return ""
        }

        return "@" + value + "@" + host
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

    function _isMastodonAccountId(value) {
        var text = value ? value.toString().trim() : ""
        return /^@[^@]+@[^@]+$/.test(text)
    }

    function _completeUpdate() {
        root.credentialsUpdated(root.accountId)
        root.goToEndDestination()
    }

    function _saveDescription(description) {
        if (description.length > 0) {
            account.setConfigurationValue("", "description", description)
            if (_isMastodonAccountId(description)) {
                account.setConfigurationValue("", "default_credentials_username", description)
            }
        }
        account.sync()
        _completeUpdate()
    }

    function _updateDescription(responseData) {
        var config = account.configurationValues("mastodon-microblog")
        var apiHost = normalizeApiHost(_valueFromServiceConfig(config, "api/Host"))
        var description = _formatMastodonAccountId(_extractAccountName(responseData), apiHost)
        if (description.length > 0) {
            _saveDescription(description)
            return
        }

        var accessToken = _extractAccessToken(responseData)
        if (accessToken.length === 0) {
            _completeUpdate()
            return
        }

        var xhr = new XMLHttpRequest()
        xhr.onreadystatechange = function() {
            if (xhr.readyState !== XMLHttpRequest.DONE) {
                return
            }

            var fetchedDescription = ""
            if (xhr.status >= 200 && xhr.status < 300) {
                try {
                    var response = JSON.parse(xhr.responseText)
                    fetchedDescription = _formatMastodonAccountId(_extractAccountName(response), apiHost)
                } catch (err) {
                }
            }

            if (fetchedDescription.length > 0) {
                _saveDescription(fetchedDescription)
            } else {
                _completeUpdate()
            }
        }

        xhr.open("GET", apiHost + "/api/v1/accounts/verify_credentials")
        xhr.setRequestHeader("Authorization", "Bearer " + accessToken)
        xhr.send()
    }

    function _startUpdate() {
        if (_started || initialPage.status !== PageStatus.Active || account.status !== Account.Initialized) {
            return
        }

        var config = account.configurationValues("mastodon-microblog")
        var apiHost = normalizeApiHost(_valueFromServiceConfig(config, "api/Host"))
        var oauthHost = _valueFromServiceConfig(config, "auth/oauth2/web_server/Host")
        if (oauthHost.length === 0) {
            oauthHost = apiHost.replace(/^https?:\/\//i, "")
        }

        var clientId = _valueFromServiceConfig(config, "auth/oauth2/web_server/ClientId")
        var clientSecret = _valueFromServiceConfig(config, "auth/oauth2/web_server/ClientSecret")
        if (clientId.length === 0 || clientSecret.length === 0) {
            //% "Missing Mastodon OAuth client credentials"
            credentialsUpdateError(qsTrId("settings-accounts-mastodon-la-missing_client_credentials"))
            return
        }

        _started = true

        var sessionData = {
            "ClientId": clientId,
            "ClientSecret": clientSecret,
            "Host": oauthHost,
            "AuthPath": "oauth/authorize",
            "TokenPath": "oauth/token",
            "ResponseType": "code",
            "Scope": ["read", "write"],
            "RedirectUri": callbackUri
        }
        initialPage.prepareAccountCredentialsUpdate(account, root.accountProvider, "mastodon-microblog", sessionData)
    }

    Account {
        id: account
        identifier: root.accountId

        onStatusChanged: {
            root._startUpdate()
        }
    }

    initialPage: OAuthAccountSetupPage {
        onStatusChanged: {
            root._startUpdate()
        }

        onAccountCredentialsUpdated: {
            root._updateDescription(responseData)
        }

        onAccountCredentialsUpdateError: {
            root.credentialsUpdateError(errorMessage)
        }

        onPageContainerChanged: {
            if (pageContainer == null) {
                cancelSignIn()
            }
        }
    }
}
