import QtQuick 2.6
import Sailfish.Silica 1.0
import seaprint.convertchecker 1.0

import "../components"

Page {
    allowedOrientations: Orientation.All
    backNavigation: false

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: col.implicitHeight

    Column {
        id:col
        y: Theme.paddingLarge
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - 2*Theme.paddingLarge
        spacing: Theme.paddingLarge

        Label {
            width: parent.width
            wrapMode: Text.WordWrap
            font.bold: true

            text: qsTr("Optional dependencies are not installed!")
        }

        Label {
            width: parent.width
            wrapMode: Text.WordWrap

            text: qsTr("In order to print PDF files with printers that do not support PDF natively,"
                       +" you need the package \"poppler-utils\"."
                       +" This enables SeaPrint to convert to formats that are more likely to be accepted.")
        }

        Label {
            visible: appWin.expectCalligra
            width: parent.width
            wrapMode: Text.WordWrap

            text: qsTr("In order to print \"office\" documents like odt and doc(x) you <i>also</i> need "+
                       "\"calligraconverter\" from the \"calligra\" package (version 3.2.1+, circa Sailfish OS 4.2).")
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

            text: qsTr("Install poppler-utils:") + " " + (ConvertChecker.pdf ? qsTr("(done)") : "")
        }

        Label {
            width: parent.width
            wrapMode: Text.WordWrap
            font.family: "monospace"

            text: "pkcon install poppler-utils"
            Clipper {}
        }

        Label {
            visible: appWin.expectCalligra
            width: parent.width
            wrapMode: Text.WordWrap

            text: qsTr("Install calligra:") + " " + (ConvertChecker.calligra ? qsTr("(done)") : "")
        }

        Label {
            visible: appWin.expectCalligra
            width: parent.width
            wrapMode: Text.WordWrap
            font.family: "monospace"

            text: "pkcon install calligra"
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

        Item{
            width: parent.width
            height: Theme.itemSizeMedium
        }
    }
    }

}
