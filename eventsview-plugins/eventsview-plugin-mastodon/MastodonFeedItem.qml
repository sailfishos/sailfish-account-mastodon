/****************************************************************************
 **
 ** Copyright (C) 2013-2026 Jolla Ltd.
 **
 ****************************************************************************/

import QtQuick 2.0
import Sailfish.Silica 1.0
import Sailfish.Share 1.0
import Sailfish.TextLinking 1.0
import org.nemomobile.lipstick 0.1
import "shared"

SocialMediaFeedItem {
    id: item

    property variant imageList
    property string resolvedStatusUrl: item.stringValue("url", "link", "uri")
    property string postId
    property QtObject postActions
    property int likeCount: item.intValue("favouritesCount", "likeCount", "favoriteCount")
    property int commentCount: item.intValue("repliesCount", "commentCount")
    property int boostCount: item.intValue("reblogsCount", "boostCount", "repostsCount")
    property bool favourited: !!model.favourited
    property bool reblogged: !!model.reblogged
    property int _likeCountOverride: -1
    property int _boostCountOverride: -1
    property int _favouritedOverride: -1
    property int _rebloggedOverride: -1
    property bool isFavourited: _favouritedOverride >= 0 ? _favouritedOverride === 1 : favourited
    property bool isReblogged: _rebloggedOverride >= 0 ? _rebloggedOverride === 1 : reblogged
    readonly property bool housekeeping: Lipstick.compositor.eventsLayer.housekeeping
    readonly property bool lockScreenActive: Lipstick.compositor.lockScreenLayer.deviceIsLocked
    property bool _pendingOpenActionMenu: false
    property bool _contextMenuOpen: false
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

    Image {
        id: boosterIcon

        anchors {
            right: avatar.right
            top: parent.top
            topMargin: item.topMargin
        }
        visible: item._booster.length > 0
        source: "image://theme/icon-s-repost" + (item.highlighted ? "?" + Theme.highlightColor : "")
    }

    Text {
        anchors {
            left: content.left
            right: content.right
            verticalCenter: boosterIcon.verticalCenter
        }
        elide: Text.ElideRight
        font.pixelSize: Theme.fontSizeExtraSmall
        color: item.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
        textFormat: Text.PlainText
        visible: text.length > 0

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

        LinkedText {
            width: parent.width
            elide: Text.ElideRight
            wrapMode: Text.Wrap
            font.pixelSize: Theme.fontSizeSmall
            shortenUrl: true
            color: item.highlighted ? Theme.highlightColor : Theme.primaryColor
            linkColor: Theme.highlightColor
            plainText: item._bodyText
        }

        Row {
            id: metadataRow

            width: parent.width
            height: previewRow.visible ? implicitHeight + Theme.paddingMedium : implicitHeight    // add padding below
            spacing: Theme.paddingSmall

            readonly property color passiveColor: item.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
            readonly property color activeColor: Theme.highlightColor

            Label {
                font.pixelSize: Theme.fontSizeExtraSmall
                text: "↩ " + item.commentCount
                color: metadataRow.passiveColor
            }

            Label {
                font.pixelSize: Theme.fontSizeExtraSmall
                text: "|"
                color: metadataRow.passiveColor
            }

            Label {
                font.pixelSize: Theme.fontSizeExtraSmall
                text: "★ " + (item._likeCountOverride >= 0 ? item._likeCountOverride : item.likeCount)
                color: item.isFavourited ? metadataRow.activeColor : metadataRow.passiveColor
            }

            Label {
                font.pixelSize: Theme.fontSizeExtraSmall
                text: "|"
                color: metadataRow.passiveColor
            }

            Label {
                font.pixelSize: Theme.fontSizeExtraSmall
                text: "↻ " + (item._boostCountOverride >= 0 ? item._boostCountOverride : item.boostCount)
                color: item.isReblogged ? metadataRow.activeColor : metadataRow.passiveColor
            }

            Label {
                visible: item.formattedTime.length > 0
                font.pixelSize: Theme.fontSizeExtraSmall
                text: "|"
                color: metadataRow.passiveColor
            }

            Label {
                visible: item.formattedTime.length > 0
                width: Math.max(0, metadataRow.width - x)
                truncationMode: TruncationMode.Fade
                font.pixelSize: Theme.fontSizeExtraSmall
                text: item.formattedTime
                color: metadataRow.passiveColor
            }
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

    function stringValue() {
        for (var i = 0; i < arguments.length; ++i) {
            var value = model[arguments[i]]
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

    function intValue() {
        for (var i = 0; i < arguments.length; ++i) {
            var value = model[arguments[i]]
            if (typeof value === "undefined" || value === null) {
                continue
            }
            var number = Number(value)
            if (!isNaN(number)) {
                return Math.max(0, Math.floor(number))
            }
        }
        return 0
    }

    function actionPostId() {
        if (item.postId.length > 0) {
            return item.postId
        }
        return item.stringValue("mastodonId", "statusId", "id", "twitterId")
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

        ContextMenu {
            id: actionMenu
            property bool menuOpen: height > 0
            property bool wasOpened: false
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

            onPositionChanged: {
                horizontalActions.xPos = _contentColumn.mapFromItem(actionMenu, mouse.x, mouse.y).x
            }

            onMenuOpenChanged: {
                if (menuOpen) {
                    wasOpened = true
                    item._contextMenuOpen = true
                } else if (wasOpened) {
                    item._contextMenuOpen = false
                    destroy()
                    item._actionMenu = null
                }
            }

            Item {
                id: horizontalActions

                // Makes Silica treat this custom row as a context-menu item.
                property int __silica_menuitem
                property bool down
                property bool highlighted
                signal clicked

                property real xPos: 0
                property int hoveredIndex: -1
                readonly property bool actionEnabled: item.postActions
                                                    && item.actionPostId().length > 0
                                                    && item.actionAccountId() >= 0
                                                    && !item.lockScreenActive
                                                    && !item.housekeeping

                width: parent.width
                height: Theme.itemSizeMedium

                onXPosChanged: hoveredIndex = xPos < width / 2 ? 0 : 1
                onDownChanged: if (!down) hoveredIndex = -1

                onClicked: {
                    xPos = _contentColumn.mapFromItem(actionMenu, actionMenu.mouseX, actionMenu.mouseY).x
                    var index = hoveredIndex >= 0 ? hoveredIndex : (xPos < width / 2 ? 0 : 1)
                    if (!actionEnabled) {
                        return
                    }
                    var postId = item.actionPostId()
                    var accountId = item.actionAccountId()
                    if (index === 0) {
                        if (item.isFavourited) {
                            item.postActions.unfavourite(accountId, postId)
                        } else {
                            item.postActions.favourite(accountId, postId)
                        }
                    } else {
                        if (item.isReblogged) {
                            item.postActions.unboost(accountId, postId)
                        } else {
                            item.postActions.boost(accountId, postId)
                        }
                    }
                }

                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    x: horizontalActions.hoveredIndex === 1 ? parent.width / 2 : 0
                    width: parent.width / 2
                    height: parent.height
                    visible: horizontalActions.down && horizontalActions.hoveredIndex >= 0
                    color: Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity)
                }

                Row {
                    anchors.fill: parent

                    Label {
                        width: parent.width / 2
                        height: parent.height
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: Theme.fontSizeExtraLarge
                        text: "★"
                        color: horizontalActions.actionEnabled
                               ? (item.isFavourited
                                  ? Theme.highlightColor
                                  : ((horizontalActions.down && horizontalActions.hoveredIndex === 0)
                                     || (horizontalActions.highlighted && horizontalActions.hoveredIndex === 0)
                                     ? Theme.secondaryHighlightColor : Theme.primaryColor))
                               : Theme.rgba(Theme.secondaryColor, 0.4)
                    }

                    Label {
                        width: parent.width / 2
                        height: parent.height
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: Theme.fontSizeExtraLarge
                        text: "↻"
                        color: horizontalActions.actionEnabled
                               ? (item.isReblogged
                                  ? Theme.highlightColor
                                  : ((horizontalActions.down && horizontalActions.hoveredIndex === 1)
                                     || (horizontalActions.highlighted && horizontalActions.hoveredIndex === 1)
                                     ? Theme.secondaryHighlightColor : Theme.primaryColor))
                               : Theme.rgba(Theme.secondaryColor, 0.4)
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
