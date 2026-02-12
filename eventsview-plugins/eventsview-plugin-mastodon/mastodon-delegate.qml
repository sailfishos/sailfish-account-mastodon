/****************************************************************************
 **
 ** Copyright (C) 2013-2026 Jolla Ltd.
 **
 ****************************************************************************/

import QtQuick 2.0
import Sailfish.Silica 1.0
import org.nemomobile.socialcache 1.0
import com.jolla.eventsview.mastodon 1.0
import QtQml.Models 2.1
import "shared"

SocialMediaAccountDelegate {
    id: delegateItem

    //: Mastodon posts
    //% "Posts"
    headerText: qsTrId("lipstick-jolla-home-la-mastodon_posts")
    headerIcon: "image://theme/icon-l-mastodon"
    showRemainingCount: false

    services: ["Posts"]
    socialNetwork: 9
    dataType: SocialSync.Posts
    providerName: "mastodon"

    MastodonPostActions {
        id: mastodonPostActions
    }

    model: MastodonPostsModel {
        onCountChanged: {
            if (count > 0) {
                if (!updateTimer.running) {
                    shortUpdateTimer.start()
                }
            } else {
                shortUpdateTimer.stop()
            }
        }
    }

    delegate: MastodonFeedItem {
        downloader: delegateItem.downloader
        imageList: delegateItem.variantRole(model, ["images", "mediaAttachments", "media"])
        avatarSource: delegateItem.convertUrl(delegateItem.stringRole(model, ["icon", "avatar", "avatarUrl"]))
        fallbackAvatarSource: delegateItem.stringRole(model, ["icon", "avatar", "avatarUrl"])
        resolvedStatusUrl: delegateItem.statusUrl(model)
        postId: delegateItem.stringRole(model, ["mastodonId", "statusId", "id", "twitterId"])
        postActions: mastodonPostActions
        accountId: delegateItem.firstAccountId(model)

        onTriggered: Qt.openUrlExternally(resolvedStatusUrl)

        Component.onCompleted: {
            refreshTimeCount = Qt.binding(function() { return delegateItem.refreshTimeCount })
            connectedToNetwork = Qt.binding(function() { return delegateItem.connectedToNetwork })
            eventsColumnMaxWidth = Qt.binding(function() { return delegateItem.eventsColumnMaxWidth })
        }
    }
    //% "Show more in Mastodon"
    expandedLabel: qsTrId("lipstick-jolla-home-la-show-more-in-mastodon")

    onHeaderClicked: Qt.openUrlExternally("https://mastodon.social/explore")
    onExpandedClicked: Qt.openUrlExternally("https://mastodon.social/explore")

    onViewVisibleChanged: {
        if (viewVisible) {
            delegateItem.resetHasSyncableAccounts()
            delegateItem.model.refresh()
            if (delegateItem.hasSyncableAccounts && !updateTimer.running) {
                shortUpdateTimer.start()
            }
        } else {
            shortUpdateTimer.stop()
        }
    }

    onConnectedToNetworkChanged: {
        if (viewVisible) {
            if (!updateTimer.running) {
                shortUpdateTimer.start()
            }
        }
    }

    // The Mastodon feed is updated 3 seconds after the feed view becomes visible,
    // unless it has been updated during last 60 seconds. After that it will be updated
    // periodically in every 60 seconds as long as the feed view is visible.

    Timer {
        id: shortUpdateTimer

        interval: 3000
        onTriggered: {
            delegateItem.sync()
            updateTimer.start()
        }
    }

    Timer {
        id: updateTimer

        interval: 60000
        repeat: true
        onTriggered: {
            if (delegateItem.viewVisible) {
                delegateItem.sync()
            } else {
                stop()
            }
        }
    }

    function variantRole(modelData, roleNames) {
        for (var i = 0; i < roleNames.length; ++i) {
            var value = modelData[roleNames[i]]
            if (typeof value !== "undefined" && value !== null) {
                return value
            }
        }
        return undefined
    }

    function stringRole(modelData, roleNames) {
        for (var i = 0; i < roleNames.length; ++i) {
            var value = modelData[roleNames[i]]
            if (typeof value === "undefined" || value === null) {
                continue
            }
            value = String(value)
            if (value.length > 0) {
                return value
            }
        }
        return ""
    }

    function statusUrl(modelData) {
        var directUrl = stringRole(modelData, ["url", "link", "uri"])
        if (directUrl.length > 0) {
            return directUrl
        }

        var instanceUrl = stringRole(modelData, ["instanceUrl", "serverUrl", "baseUrl"])
        if (instanceUrl.length === 0) {
            instanceUrl = "https://mastodon.social"
        }
        while (instanceUrl.length > 0 && instanceUrl.charAt(instanceUrl.length - 1) === "/") {
            instanceUrl = instanceUrl.slice(0, instanceUrl.length - 1)
        }

        var accountName = stringRole(modelData, ["accountName", "acct", "screenName", "username"])
        var statusId = stringRole(modelData, ["mastodonId", "statusId", "id", "twitterId"])
        if (accountName.length > 0 && statusId.length > 0) {
            while (accountName.length > 0 && accountName.charAt(0) === "@") {
                accountName = accountName.substring(1)
            }
            return instanceUrl + "/@" + accountName + "/" + statusId
        }

        return instanceUrl + "/explore"
    }

    function convertUrl(source) {
        if (source.indexOf("_normal.") !== -1) {
            return source.replace("_normal.", "_bigger.")
        } else if (source.indexOf("_mini.") !== -1) {
            return source.replace("_mini.", "_bigger.")
        }
        return source
    }

    function firstAccountId(modelData) {
        var accounts = modelData.accounts
        if (accounts && accounts.length > 0) {
            var accountId = Number(accounts[0])
            if (!isNaN(accountId)) {
                return accountId
            }
        }
        return -1
    }
}
