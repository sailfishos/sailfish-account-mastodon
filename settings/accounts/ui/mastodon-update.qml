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
            credentialsUpdateError("Missing Mastodon OAuth client credentials")
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
            root.credentialsUpdated(root.accountId)
            root.goToEndDestination()
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
