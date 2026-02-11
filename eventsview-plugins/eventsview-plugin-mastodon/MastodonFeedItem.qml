/****************************************************************************
 **
 ** Copyright (C) 2013-2026 Jolla Ltd.
 **
 ****************************************************************************/

import QtQuick 2.0
import Sailfish.Silica 1.0
import Sailfish.TextLinking 1.0
import "shared"

SocialMediaFeedItem {
    id: item

    property variant imageList
    property int likeCount: item.intValue("favouritesCount", "likeCount", "favoriteCount")
    property int commentCount: item.intValue("repliesCount", "commentCount")
    property int boostCount: item.intValue("reblogsCount", "boostCount", "repostsCount")

    property string _booster: item.stringValue("boostedBy", "rebloggedBy", "retweeter")
    property string _displayName: item.stringValue("name", "displayName", "display_name")
    property string _accountName: item.stringValue("accountName", "acct", "screenName", "username")
    property string _bodyText: item.stringValue("body", "content", "text")

    timestamp: model.timestamp
    onRefreshTimeCountChanged: formattedTime = Format.formatDate(model.timestamp, Format.TimeElapsed)

    avatar.y: item._booster.length > 0
              ? topMargin + boosterIcon.height + Theme.paddingSmall
              : topMargin
    contentHeight: Math.max(content.y + content.height, avatar.y + avatar.height) + bottomMargin
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

        Text {
            width: parent.width
            height: previewRow.visible ? implicitHeight + Theme.paddingMedium : implicitHeight    // add padding below
            maximumLineCount: 1
            elide: Text.ElideRight
            wrapMode: Text.Wrap
            color: item.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
            font.pixelSize: Theme.fontSizeExtraSmall
            text: item.metadataText()
            textFormat: Text.PlainText
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

    function metadataText() {
        var parts = []
        parts.push("↩ " + item.commentCount)
        parts.push("★ " + item.likeCount)
        parts.push("↻ " + item.boostCount)
        if (item.formattedTime.length > 0) {
            parts.push(item.formattedTime)
        }
        return parts.join(" | ")
    }
}
