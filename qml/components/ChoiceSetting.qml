import QtQuick 2.0
import Sailfish.Silica 1.0
import "../pages/utils.js" as Utils

Setting {
    property var choices
    property string mime_type

    property var limited_choices: Utils.limitChoices(name, choices, mime_type)

    ValueButton {
        enabled: valid
        anchors.verticalCenter: parent.verticalCenter
        label: prettyName
        value: Utils.ippName(name, choice ? choice : default_choice)
        onClicked: parent.clicked()
    }

    property var menu: ContextMenu {
        id: menu
        enabled: valid
        Repeater {
            model: limited_choices
            MenuItem {
                text: Utils.ippName(name, limited_choices[index])
                onClicked:
                {
                    choice = limited_choices[index];
                }
            }
        }

    }


}
