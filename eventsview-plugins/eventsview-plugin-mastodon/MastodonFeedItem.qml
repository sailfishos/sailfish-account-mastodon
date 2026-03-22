// SPDX-FileCopyrightText: 2019 - 2023 Jolla Ltd.
// SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
//
// SPDX-License-Identifier: BSD-3-Clause

import QtQuick 2.0
import Sailfish.Silica 1.0
import Sailfish.Share 1.0
import org.nemomobile.lipstick 0.1
import Sailfish.SocialFeed 1.0

SocialMediaFeedItem {
    id: item

    property variant imageList
    property string resolvedStatusUrl: model && model.url ? model.url.toString() : ""
    property string postId
    property QtObject postActions
    property int likeCount: model && model.favouritesCount ? model.favouritesCount : 0
    property int commentCount: model && model.repliesCount ? model.repliesCount : 0
    property int boostCount: model && model.reblogsCount ? model.reblogsCount : 0
    property bool favourited: model ? !!model.favourited : false
    property bool reblogged: model ? !!model.reblogged : false
    property int _likeCountOverride: -1
    property int _boostCountOverride: -1
    property int _favouritedOverride: -1
    property int _rebloggedOverride: -1
    property bool isFavourited: _favouritedOverride >= 0 ? _favouritedOverride === 1 : favourited
    property bool isReblogged: _rebloggedOverride >= 0 ? _rebloggedOverride === 1 : reblogged
    readonly property bool housekeeping: Lipstick.compositor.eventsLayer.housekeeping
    readonly property bool lockScreenActive: Lipstick.compositor.lockScreenLayer.deviceIsLocked
    property bool _pendingOpenActionMenu
    property bool _contextMenuOpen
    property var _actionMenu
    property real _contextMenuHeight: (_contextMenuOpen && _actionMenu) ? _actionMenu.height : 0

    property string _booster: model && model.boostedBy ? model.boostedBy.toString() : ""
    property string _displayName: model && model.name ? model.name.toString() : ""
    property string _accountName: model && model.accountName ? model.accountName.toString() : ""
    property string _bodyText: model && model.body ? model.body.toString() : ""
    //: Action label shown in Mastodon interaction menu.
    //% "Favourite"
    readonly property string _favouriteActionText: qsTrId("lipstick-jolla-home-la-mastodon_favourite")
    //: Action label shown in Mastodon interaction menu when the post is already favourited.
    //% "Unfavourite"
    readonly property string _unfavouriteActionText: qsTrId("lipstick-jolla-home-la-mastodon_unfavourite")
    //: Action label shown in Mastodon interaction menu.
    //% "Boost"
    readonly property string _boostActionText: qsTrId("lipstick-jolla-home-la-mastodon_boost")
    //: Action label shown in Mastodon interaction menu when the post is already boosted.
    //% "Undo boost"
    readonly property string _unboostActionText: qsTrId("lipstick-jolla-home-la-mastodon_unboost")
    //: Action label shown in Mastodon interaction menu.
    //% "Share"
    readonly property string _shareActionText: qsTrId("lipstick-jolla-home-la-mastodon_share")
    //: Link title used when sharing a Mastodon post.
    //% "Post from Mastodon"
    readonly property string _shareLinkTitle: qsTrId("lipstick-jolla-home-la-mastodon_share_link_title")
    property var _shareAction: ShareAction {
        title: item._shareActionText
    }

    timestamp: model.timestamp
    onRefreshTimeCountChanged: formattedTime = Format.formatDate(model.timestamp, Format.TimeElapsed)
    onLockScreenActiveChanged: {
        if (lockScreenActive && _actionMenu) {
            _actionMenu.close()
        }
    }
    onPressAndHold: function(mouse) {
        if (mouse) {
            mouse.accepted = true
        }
        _pendingOpenActionMenu = !lockScreenActive
                                 && postActions
                                 && actionPostId().length > 0
                                 && actionAccountId() >= 0
        openActionMenuTimer.restart()
    }
    onHousekeepingChanged: {
        if (housekeeping && _pendingOpenActionMenu) {
            Lipstick.compositor.eventsLayer.setHousekeeping(false)
        }
    }
    Component.onDestruction: {
        if (_actionMenu) {
            _actionMenu.destroy()
            _actionMenu = null
        }
    }

    avatar.y: item._booster.length > 0
              ? topMargin + boosterIcon.height + Theme.paddingSmall
              : topMargin
    contentHeight: Math.max(content.y + content.height, avatar.y + avatar.height) + bottomMargin + _contextMenuHeight
    topMargin: item._booster.length > 0 ? Theme.paddingMedium : Theme.paddingLarge
    userRemovable: false

    SocialReshareIcon {
        id: boosterIcon

        anchors {
            right: avatar.right
            top: parent.top
            topMargin: item.topMargin
        }
        visible: item._booster.length > 0
        highlighted: item.highlighted
        iconSource: "image://theme/icon-s-repost"
    }

    SocialReshareText {
        anchors {
            left: content.left
            right: content.right
            verticalCenter: boosterIcon.verticalCenter
        }
        highlighted: item.highlighted
        text: item._booster.length > 0
              ? //: Shown above a post that is boosted by another user. %1 = name of user who boosted
                //% "%1 boosted"
                qsTrId("lipstick-jolla-home-la-boosted_by").arg(item._booster)
              : ""
    }

    Column {
        id: content

        anchors {
            left: avatar.right
            leftMargin: Theme.paddingMedium
            top: avatar.top
        }
        width: parent.width - x

        Label {
            width: parent.width
            truncationMode: TruncationMode.Fade
            text: item._displayName
            color: item.highlighted ? Theme.highlightColor : Theme.primaryColor
            textFormat: Text.PlainText
        }

        Label {
            width: parent.width
            truncationMode: TruncationMode.Fade
            text: item._accountName.length > 0 && item._accountName.charAt(0) !== "@"
                  ? "@" + item._accountName
                  : item._accountName
            font.pixelSize: Theme.fontSizeSmall
            color: item.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
            textFormat: Text.PlainText
        }

        LinkedLabel {
            width: parent.width
            elide: Text.ElideRight
            wrapMode: Text.Wrap
            font.pixelSize: Theme.fontSizeSmall
            shortenUrl: true
            color: item.highlighted ? Theme.highlightColor : Theme.primaryColor
            linkColor: Theme.highlightColor
            plainText: item._bodyText
        }

        SocialPostMetadataRow {
            id: metadataRow

            width: parent.width
            highlighted: item.highlighted
            commentCount: item.commentCount
            likeCount: item._likeCountOverride >= 0 ? item._likeCountOverride : item.likeCount
            repostCount: item._boostCountOverride >= 0 ? item._boostCountOverride : item.boostCount
            liked: item.isFavourited
            reposted: item.isReblogged
            timeText: item.formattedTime
            addBottomPadding: previewRow.visible
        }

        SocialMediaPreviewRow {
            id: previewRow

            width: parent.width + Theme.horizontalPageMargin   // extend to right edge of notification area
            imageList: item.imageList
            downloader: item.downloader
            accountId: item.accountId
            connectedToNetwork: item.connectedToNetwork
            highlighted: item.highlighted
            eventsColumnMaxWidth: item.eventsColumnMaxWidth - item.avatar.width
        }
    }

    function actionPostId() {
        if (item.postId.length > 0) {
            return item.postId
        }
        return model && model.mastodonId ? model.mastodonId.toString() : ""
    }

    function actionAccountId() {
        var parsed = Number(item.accountId)
        return isNaN(parsed) ? -1 : parsed
    }

    function shareStatusUrl() {
        return model && model.url ? model.url.toString() : ""
    }

    function topLevelParent() {
        var p = item
        while (p && p.parent) {
            p = p.parent
        }
        return p
    }

    function openActionMenu() {
        if (_actionMenu) {
            _actionMenu.destroy()
            _actionMenu = null
        }

        var parentItem = topLevelParent()
        _actionMenu = actionMenuComponent.createObject(parentItem)
        if (_actionMenu) {
            _actionMenu.open(item)
        }
    }

    Connections {
        target: item.postActions ? item.postActions : null

        onActionSucceeded: {
            if (accountId !== item.actionAccountId() || statusId !== item.actionPostId()) {
                return
            }

            if (favouritesCount >= 0) {
                item._likeCountOverride = favouritesCount
            }
            if (reblogsCount >= 0) {
                item._boostCountOverride = reblogsCount
            }
            item._favouritedOverride = favourited ? 1 : 0
            item._rebloggedOverride = reblogged ? 1 : 0
            item._contextMenuOpen = false

            if (item._accountDelegate) {
                item._accountDelegate.sync()
            }
        }

        onActionFailed: {
            if (accountId !== item.actionAccountId() || statusId !== item.actionPostId()) {
                return
            }
            console.warn("Mastodon action failed:", action, errorMessage)
            item._contextMenuOpen = false
        }
    }

    Component {
        id: actionMenuComponent

        SocialInteractionContextMenu {
            id: actionMenu
            z: 10000
            mapSourceItem: _contentColumn
            actionEnabled: item.postActions
                           && item.actionPostId().length > 0
                           && item.actionAccountId() >= 0
                           && !item.lockScreenActive
                           && !item.housekeeping
            interactionItems: [
                {
                    name: "like",
                    // U+2605 BLACK STAR
                    symbol: "\u2605",
                    active: item.isFavourited,
                    inactiveText: item._favouriteActionText,
                    activeText: item._unfavouriteActionText
                },
                {
                    name: "reblog",
                    // U+21BB CLOCKWISE OPEN CIRCLE ARROW
                    symbol: "\u21BB",
                    active: item.isReblogged,
                    inactiveText: item._boostActionText,
                    activeText: item._unboostActionText
                },
                {
                    name: "share",
                    // U+260D OPPOSITION (ironic doncha think)
                    symbol: "\u260D",
                    active: false,
                    inactiveText: item._shareActionText,
                    activeText: item._shareActionText
                }
            ]

            onInteractionMenuOpened: item._contextMenuOpen = true
            onInteractionMenuClosed: {
                item._contextMenuOpen = false
                destroy()
                item._actionMenu = null
            }

            onInteractionTriggered: function(actionName) {
                if (!actionEnabled) {
                    return
                }
                var postId = item.actionPostId()
                var accountId = item.actionAccountId()
                if (actionName === "like") {
                    if (item.isFavourited) {
                        item.postActions.unfavourite(accountId, postId)
                    } else {
                        item.postActions.favourite(accountId, postId)
                    }
                } else if (actionName === "reblog") {
                    if (item.isReblogged) {
                        item.postActions.unboost(accountId, postId)
                    } else {
                        item.postActions.boost(accountId, postId)
                    }
                } else if (actionName === "share") {
                    var shareUrl = item.shareStatusUrl()
                    if (shareUrl.length === 0) {
                        return
                    }
                    item._shareAction.resources = [{
                        "data": shareUrl,
                        "linkTitle": item._shareLinkTitle,
                        "type": "text/x-url"
                    }]
                    item._shareAction.trigger()
                }
            }
        }
    }

    Timer {
        id: openActionMenuTimer

        interval: 0
        repeat: false
        onTriggered: {
            if (item.lockScreenActive) {
                item._pendingOpenActionMenu = false
                return
            }
            Lipstick.compositor.eventsLayer.setHousekeeping(false)
            if (item._pendingOpenActionMenu) {
                item._contextMenuOpen = false
                item.openActionMenu()
            }
            item._pendingOpenActionMenu = false
        }
    }
}
