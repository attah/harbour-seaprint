import QtQuick 2.6
import Sailfish.Silica 1.0
import "utils.js" as Utils

Page {

    id: aboutPage

    allowedOrientations: Orientation.All

    property var printer

    SilicaFlickable {
        id: flick
        anchors.fill : parent
        contentHeight: textArea.height + header.height
        contentWidth : parent.width - (2.0 * Theme.paddingLarge)

        PageHeader {
            id: header
            title: Utils.unknownForEmptyString(printer.attrs["printer-name"].value)
        }

        TextArea {
            id: textArea

            readOnly: true
            width: parent.width
            anchors.top: header.bottom
            anchors.bottom: flick.bottom
            font.family: "courier"
            font.pixelSize: Theme.fontSizeTiny
            wrapMode: TextEdit.Wrap
            text: JSON.stringify(printer.attrs, null, "  ")

            onPressAndHold: {
                selectAll()
                copy()
                deselect()
                notifier.notify(qsTr("Copied to clipboard"))
            }
        }
    }
}
