import QtQuick 2.0
import Sailfish.Silica 1.0
import seaprint.ippprinter 1.0

Dialog {
    id: dialog
    allowedOrientations: Orientation.All

    property string value
    property string ssid
    property var printerName: false
    canAccept: printerName != false

    Connections {
        target: wifi
        onConnectedChanged: {
            if(!wifi.connected) {
                dialog.reject()
            }
        }
    }

    Component.onCompleted: {
        valueField.forceActiveFocus()
    }

    IppPrinter {
        id: printer
        url: valueField.text
        onAttrsChanged: {
            if(printer.attrs["printer-name"]) {
                printerName = printer.attrs["printer-name"].value == "" ? qsTr("Unknown") : printer.attrs["printer-name"].value
            }
            else
            {
                printerName = false
            }

        }
    }

    Column {
        id: col
        width: parent.width

        DialogHeader {
            title: qsTr("Add printer")
        }

        TextField {
            id: valueField
            width: parent.width
            placeholderText: "192.168.1.1/ipp/print"

            label: title
        }
        Row {
            x: Theme.paddingLarge
            spacing: Theme.paddingMedium

            Label {
                id: wifi_label
                text: qsTr("On WiFi:")
            }
            Label {
                id: ssid_label
                color: Theme.secondaryColor
                text: wifi.ssid
            }
        }
        Row {
            x: Theme.paddingLarge
            spacing: Theme.paddingMedium

            Label {
                id: found_label
                text: printerName != false ? qsTr("Found:") : qsTr("No printer found")
            }
            Label {
                id: printer_label
                color: Theme.secondaryColor
                text: printerName ? printerName : ""
            }
        }
    }

    Row {
        visible: valueField.text.indexOf(":9100") != -1

        anchors.top: col.bottom
        anchors.topMargin: Theme.paddingLarge*2
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width-2*Theme.paddingLarge
        spacing: Theme.paddingMedium

        Icon {
            id: warningIcon
            source: "image://theme/icon-m-warning"
            anchors.verticalCenter: parent.verticalCenter
        }

        Label {
            width: parent.width-warningIcon.width-Theme.paddingMedium
            color: Theme.highlightColor
            wrapMode: Text.WordWrap
            text: qsTr("Port 9100 is not used for IPP.")+"\n"+qsTr("It is used for raw PCL/PDL, which is not supported.")
        }
    }

    onDone: {
        if (result == DialogResult.Accepted) {
            value = valueField.text
            ssid = wifi.ssid
        }
    }

}
