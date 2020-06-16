import QtQuick 2.0
import Sailfish.Silica 1.0
import seaprint.convertchecker 1.0
import seaprint.ippmsg 1.0
import "../pages/utils.js" as Utils

Setting {
    property var printer

    Component.onCompleted: {
        if((printer.attrs["media-left-margin-supported"].value.indexOf(0) != -1) &&
           (printer.attrs["media-right-margin-supported"].value.indexOf(0) != -1) &&
           (printer.attrs["media-top-margin-supported"].value.indexOf(0) != -1) &&
           (printer.attrs["media-bottom-margin-supported"].value.indexOf(0) != -1))
        {
            valid = true
        }
    }

    ValueButton {
        enabled: valid
        anchors.verticalCenter: parent.verticalCenter
        label: prettyName
        value: choice ? qsTr("true") : qsTr("false")
        onClicked: parent.clicked()
    }

    property var menu: ContextMenu {
        id: menu
        enabled: true
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
