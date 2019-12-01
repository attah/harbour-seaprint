import QtQuick 2.0
import Sailfish.Silica 1.0
import seaprint.ippprinter 1.0

Dialog {
    id: dialog

    property string value
    property string title
    canAccept: label.text != label._default

    Column {
        width: parent.width

        DialogHeader { }

        TextField {
            id: valueField
            width: parent.width
            placeholderText: "192.168.1.1/ipp/print"

            label: title
        }
        Label {
            id: label
            x: Theme.paddingLarge
            property string _default: "No printer found"
            text: _default
        }
        IppPrinter {
            id: printer
            url: valueField.text
            onAttrsChanged: {
                if(printer.attrs["printer-name"]) {
                    label.text = "Found: "+printer.attrs["printer-name"].value
                }
                else
                {
                    label.text = label._default
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
