import QtQuick 2.6
import Sailfish.Silica 1.0
import Sailfish.Lipstick 1.0
import Sailfish.TransferEngine 1.0

Item {
    id: root

    property var shareAction
    property string mimeType: {
        if (shareAction && shareAction.mimeType) {
            return shareAction.mimeType
        }
        if (shareAction && shareAction.resources
                && shareAction.resources.length > 0
                && shareAction.resources[0]
                && shareAction.resources[0].type) {
            return shareAction.resources[0].type
        }
        return ""
    }
    property bool textShare: mimeType === "text/x-url" || mimeType === "text/plain"

    width: parent ? parent.width : 0
    height: previewLoader.item ? previewLoader.item.height : 0

    Loader {
        id: previewLoader

        anchors.fill: parent
        sourceComponent: root.textShare ? postPreview : imagePreview
    }

    Component {
        id: imagePreview

        ShareFilePreview {
            shareAction: root.shareAction
            metadataStripped: true
            descriptionPlaceholderText: qsTr("Write a post")
        }
    }

    Component {
        id: postPreview

        SilicaFlickable {
            id: postRoot

            width: parent.width
            height: contentHeight
            contentHeight: contentColumn.height

            Component.onCompleted: {
                sailfishTransfer.loadConfiguration(root.shareAction.toConfiguration())
                statusTextField.forceActiveFocus()
                statusTextField.cursorPosition = statusTextField.text.length
            }

            SailfishTransfer {
                id: sailfishTransfer
            }

            Column {
                id: contentColumn

                width: parent.width

                TextArea {
                    id: linkTextField

                    width: parent.width
                    //% "Link"
                    label: qsTrId("sailfishshare-la-link")
                    placeholderText: label
                    visible: sailfishTransfer.content.type === "text/x-url"
                    text: sailfishTransfer.content.data || sailfishTransfer.content.status || ""
                }

                TextArea {
                    id: statusTextField

                    width: parent.width
                    //% "Status update"
                    label: qsTrId("sailfishshare-la-status_update")
                    placeholderText: label
                    text: {
                        var title = sailfishTransfer.content.name || sailfishTransfer.content.linkTitle || ""
                        if (linkTextField.visible) {
                            return title
                        }
                        var body = sailfishTransfer.content.data || sailfishTransfer.content.status || ""
                        if (title.length > 0 && body.length > 0) {
                            return title + ": " + body
                        }
                        return title + body
                    }
                }

                SystemDialogIconButton {
                    id: postButton

                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width / 2
                    iconSource: "image://theme/icon-m-share"
                    bottomPadding: Theme.paddingLarge
                    _showPress: false

                    //: Post a social network account status update
                    //% "Post"
                    text: qsTrId("sailfishshare-la-post_status")

                    onClicked: {
                        var status = statusTextField.text || ""
                        var link = linkTextField.visible ? (linkTextField.text || "") : ""
                        if (link.length > 0 && status.indexOf(link) === -1) {
                            status = status.length > 0 ? (status + "\n" + link) : link
                        }

                        sailfishTransfer.userData = {
                            "accountId": sailfishTransfer.transferMethodInfo.accountId,
                            "status": status
                        }
                        sailfishTransfer.mimeType = linkTextField.visible ? "text/x-url" : "text/plain"
                        sailfishTransfer.start()
                        root.shareAction.done()
                    }
                }
            }
        }
    }
}
