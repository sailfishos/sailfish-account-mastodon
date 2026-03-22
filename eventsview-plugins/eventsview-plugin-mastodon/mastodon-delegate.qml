// SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
// SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
//
// SPDX-License-Identifier: BSD-3-Clause

import QtQuick 2.0
import Sailfish.Silica 1.0
import org.nemomobile.socialcache 1.0
import com.jolla.eventsview.mastodon 1.0
import QtQml.Models 2.1
import Sailfish.SocialFeed 1.0

SocialMediaAccountDelegate {
    id: delegateItem
    property string instanceHomeUrl: ""

    //: Mastodon posts
    //% "Posts"
    headerText: qsTrId("lipstick-jolla-home-la-mastodon_posts")
    headerIcon: "image://theme/icon-l-mastodon"
    showRemainingCount: false

    services: ["Posts"]
    socialNetwork: SocialSync.Mastodon
    dataType: SocialSync.Posts
    providerName: "mastodon"
    periodicSyncLoopEnabled: true

    MastodonPostActions {
        id: mastodonPostActions
    }

    model: MastodonPostsModel {}

    delegate: MastodonFeedItem {
        downloader: delegateItem.downloader
        imageList: model.images
        avatarSource: model.icon
        fallbackAvatarSource: model.icon
        resolvedStatusUrl: delegateItem.authorizeInteractionUrl(model)
        postId: model.mastodonId
        postActions: mastodonPostActions
        accountId: delegateItem.firstAccountId(model, -1)

        onTriggered: {
            if (resolvedStatusUrl.length > 0) {
                Qt.openUrlExternally(resolvedStatusUrl)
            }
        }

        Component.onCompleted: {
            delegateItem.instanceHomeUrl = statusUrl({instanceUrl: model.instanceUrl})
            refreshTimeCount = Qt.binding(function() { return delegateItem.refreshTimeCount })
            connectedToNetwork = Qt.binding(function() { return delegateItem.connectedToNetwork })
            eventsColumnMaxWidth = Qt.binding(function() { return delegateItem.eventsColumnMaxWidth })
        }
    }
    //% "Show more in Mastodon"
    expandedLabel: qsTrId("lipstick-jolla-home-la-show-more-in-mastodon")

    onHeaderClicked: {
        if (delegateItem.instanceHomeUrl.length > 0) {
            Qt.openUrlExternally(delegateItem.instanceHomeUrl)
        }
    }
    onExpandedClicked: {
        if (delegateItem.instanceHomeUrl.length > 0) {
            Qt.openUrlExternally(delegateItem.instanceHomeUrl)
        }
    }

    onViewVisibleChanged: {
        if (viewVisible) {
            delegateItem.resetHasSyncableAccounts()
            delegateItem.model.refresh()
            if (delegateItem.hasSyncableAccounts) {
                delegateItem.startPeriodicSyncLoop()
            }
        } else {
            delegateItem.stopPeriodicSyncLoop()
        }
    }

    onConnectedToNetworkChanged: {
        if (viewVisible) {
            delegateItem.startPeriodicSyncLoop()
        }
    }

    Connections {
        target: delegateItem.model

        onCountChanged: {
            if (target.count === 0) {
                delegateItem.instanceHomeUrl = ""
            }
        }
    }

    function statusUrl(modelData) {
        var directUrl = modelData && modelData.url ? modelData.url.toString() : ""
        if (directUrl.length > 0) {
            return directUrl
        }

        var instanceUrl = modelData && modelData.instanceUrl ? modelData.instanceUrl.toString() : ""
        instanceUrl = stripTrailingSlashes(instanceUrl)
        if (instanceUrl.length === 0) {
            return ""
        }

        var accountName = modelData && modelData.accountName ? modelData.accountName.toString() : ""
        var statusId = modelData && modelData.mastodonId ? modelData.mastodonId.toString() : ""
        if (accountName.length > 0 && statusId.length > 0) {
            accountName = trimLeadingCharacter(accountName, "@")
            return instanceUrl + "/@" + accountName + "/" + statusId
        }

        return instanceUrl + "/explore"
    }

    function authorizeInteractionUrl(modelData) {
        var targetUrl = statusUrl(modelData)
        if (targetUrl.length === 0) {
            return targetUrl
        }

        var instanceUrl = modelData && modelData.instanceUrl ? modelData.instanceUrl.toString() : ""
        if (instanceUrl.length === 0) {
            return targetUrl
        }
        instanceUrl = stripTrailingSlashes(instanceUrl)

        // Links on the user's own instance should open directly.
        var sameServer = /^([a-z][a-z0-9+.-]*):\/\/([^\/?#]+)/i
        var targetMatch = targetUrl.match(sameServer)
        var instanceMatch = instanceUrl.match(sameServer)
        if (targetMatch && instanceMatch
                && targetMatch.length > 2
                && instanceMatch.length > 2
                && targetMatch[1].toLowerCase() === instanceMatch[1].toLowerCase()
                && targetMatch[2].toLowerCase() === instanceMatch[2].toLowerCase()) {
            return targetUrl
        }

        return instanceUrl + "/authorize_interaction?uri=" + encodeURIComponent(targetUrl)
    }

    function firstAccountId(modelData, defaultValue) {
        var fallback = typeof defaultValue === "undefined" ? -1 : Number(defaultValue)
        var accounts = modelData ? modelData.accounts : undefined
        if (!accounts || accounts.length <= 0) {
            return fallback
        }

        var accountId = Number(accounts[0])
        return isNaN(accountId) ? fallback : accountId
    }

    function stripTrailingSlashes(value) {
        value = String(value || "")
        while (value.length > 0 && value.charAt(value.length - 1) === "/") {
            value = value.slice(0, value.length - 1)
        }
        return value
    }

    function trimLeadingCharacter(value, character) {
        value = String(value || "")
        if (!character || character.length === 0) {
            return value
        }

        while (value.length > 0 && value.charAt(0) === character) {
            value = value.substring(1)
        }
        return value
    }
}
