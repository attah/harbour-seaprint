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
            inputMethodHints: Qt.ImhUrlCharactersOnly | Qt.ImhNoAutoUppercase
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

        Item {
            width: 1
            height: 2*Theme.paddingLarge
        }

        Row {
            visible: valueField.text.indexOf(":9100") != -1
            x: Theme.paddingLarge

            width: parent.width-2*Theme.paddingLarge
            spacing: Theme.paddingMedium

            Icon {
                id: warningIcon
                source: "image://theme/icon-m-warning"
                anchors.verticalCenter: parent.verticalCenter
            }

            Label {
                width: parent.width-warningIcon.width-Theme.paddingMedium
                anchors.verticalCenter: parent.verticalCenter
                color: Theme.highlightColor
                wrapMode: Text.WordWrap
                text: qsTr("Port 9100 is not used for IPP.")
            }
        }

        Row {
            id: suffixWarning
            visible: canAccept && !printer.correctSuffix
            x: Theme.paddingLarge

            width: parent.width-2*Theme.paddingLarge
            spacing: Theme.paddingMedium

            Icon {
                id: warningIcon2
                source: "image://theme/icon-m-warning"
                anchors.verticalCenter: parent.verticalCenter
            }

            Label {
                width: parent.width-warningIcon2.width-Theme.paddingMedium
                anchors.verticalCenter: parent.verticalCenter
                color: Theme.highlightColor
                wrapMode: Text.WordWrap
                text: qsTr("The uri suffix is not in the printer's supported list.")+" "+
                      qsTr("It might not accept print jobs on this address.")+" "+
                      qsTr("Consider using a suffix like \"/ipp/print\".")
            }
        }

        Item {
            width: 1
            height: 2*Theme.paddingLarge
        }

        Label
        {
            x: Theme.paddingLarge
            visible: suffixWarning.visible && printer.suffixes.length != 0
            text: qsTr("The printer/server lists these suffixes:")
        }

        Repeater
        {
            model: suffixWarning.visible ? printer.suffixes : 0
            Label
            {
                x: Theme.paddingLarge
                text: printer.suffixes[index]
            }

        }

    }

    onDone: {
        if (result == DialogResult.Accepted) {
            value = valueField.text
            ssid = wifi.ssid
        }
    }

}
