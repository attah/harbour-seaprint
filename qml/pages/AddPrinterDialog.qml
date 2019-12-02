import QtQuick 2.0
import Sailfish.Silica 1.0
import seaprint.ippprinter 1.0

Dialog {
    id: dialog

    property string value
    property string ssid
    canAccept: printer_label.text != ""

    Column {
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
                text: qsTr("On WiFI:")
            }
            Label {
                id: ssid_label
                color: Theme.secondaryColor
                text: ssid
            }
        }
        Row {
            x: Theme.paddingLarge
            spacing: Theme.paddingMedium

            Label {
                id: found_label
                text: printer_label.text != "" ? qsTr("Found:") : qsTr("No printer found")
            }
            Label {
                id: printer_label
                color: Theme.secondaryColor
                text: ""
            }
        }
        IppPrinter {
            id: printer
            url: valueField.text
            onAttrsChanged: {
                if(printer.attrs["printer-name"]) {
                    printer_label.text = printer.attrs["printer-name"].value
                }
                else
                {
                    printer_label.text = ""
                }

            }
        }

    }

    onDone: {
        if (result == DialogResult.Accepted) {
            value = valueField.text
        }
    }

}
