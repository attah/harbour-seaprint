import QtQuick 2.0
import Sailfish.Silica 1.0
import seaprint.ippmsg 1.0

Setting {
    valid: ((parent.printer.attrs["media-left-margin-supported"].value.indexOf(0) != -1) &&
            (parent.printer.attrs["media-right-margin-supported"].value.indexOf(0) != -1) &&
            (parent.printer.attrs["media-top-margin-supported"].value.indexOf(0) != -1) &&
            (parent.printer.attrs["media-bottom-margin-supported"].value.indexOf(0) != -1))

    displayValue: choice ? qsTr("true") : qsTr("false")

    menu: ContextMenu {
        id: menu
        MenuItem {
            text: qsTr("true")
            onClicked:
            {
                choice = {"media-left-margin":    {"tag": IppMsg.Integer, "value": 0},
                          "media-right-margin":   {"tag": IppMsg.Integer, "value": 0},
                          "media-top-margin":     {"tag": IppMsg.Integer, "value": 0},
                          "media-bottom-margin":  {"tag": IppMsg.Integer, "value": 0}}
            }
        }
        MenuItem {
            text: qsTr("false")
            onClicked:
            {
                choice = undefined
            }
        }

    }


}
