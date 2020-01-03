import QtQuick 2.0
import Sailfish.Silica 1.0
import "../pages/utils.js" as Utils

Setting {
    property var choices

    ValueButton {
        enabled: valid
        anchors.verticalCenter: parent.verticalCenter
        label: prettyName
        value: Utils.ippName(name, choice ? choice : default_choice)
        onClicked: parent.clicked()
    }

    property var menu: ContextMenu {
        id: menu
        Repeater {
            model: choices
            MenuItem {
                text: Utils.ippName(name, choices[index])
                onClicked:
                {
                    choice = choices[index];
                }
            }
        }

    }


}
