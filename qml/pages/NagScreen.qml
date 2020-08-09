import QtQuick 2.0
import Sailfish.Silica 1.0
import "../components"

Page {
    backNavigation: false

    Column {
        y: Theme.paddingLarge
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - 2*Theme.paddingLarge
        spacing: Theme.paddingLarge

        Label {
            width: parent.width
            wrapMode: Text.WordWrap

            text: qsTr("Optional dependencies are not installed!")
        }

        Label {
            width: parent.width
            wrapMode: Text.WordWrap

            text: qsTr("In order to print PDF files with printers that does not support PDF natively,"
                       +" you need utilities from the package \"poppler-utils\"."
                       +" This enables SeaPrint to convert to formats that they are more likely to accept.")
        }

        Label {
            width: parent.width
            wrapMode: Text.WordWrap

            text: qsTr("To install, enable developer mode, and open a terminal.")
        }

        Label {
            width: parent.width
            wrapMode: Text.WordWrap

            text: qsTr("Become root:")
        }

        Label {
            width: parent.width
            wrapMode: Text.WordWrap
            font.family: "monospace"

            text: "devel-su"
            Clipper {}

        }

        Label {
            width: parent.width
            wrapMode: Text.WordWrap

            text: qsTr("Install poppler-utils:")
        }

        Label {
            width: parent.width
            wrapMode: Text.WordWrap
            font.family: "monospace"

            text: "pkcon install poppler-utils"
            Clipper {}
        }

        Item{
            width: parent.width
            height: Theme.itemSizeMedium
        }

        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Dismiss")
            onClicked: pageStack.pop()
        }

        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Don't show again")
            onClicked: {
                nagScreenSetting.value = nagScreenSetting.expectedValue
                pageStack.pop()
            }
        }
    }

}
