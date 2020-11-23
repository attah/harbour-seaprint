import QtQuick 2.0
import Sailfish.Silica 1.0
import seaprint.convertchecker 1.0
import "../pages/utils.js" as Utils

Setting {
    height: button.height +  menu.height

    property var choices
    property string mime_type

    property var limited_choices: Utils.limitChoices(name, choices, mime_type, ConvertChecker)

    property int num_large_choices: 8

    ValueButton {
        id: button
        enabled: valid
        label: prettyName
        value: Utils.ippName(name, choice ? choice : default_choice)
        onClicked: parent.clicked()
    }

    onClicked: {
        if(limited_choices.length>num_large_choices)
        {
            var dialog = pageStack.push("LargeChoiceDialog.qml",
                                        {name:name, choice: choice ? choice : default_choice, choices: limited_choices})
            dialog.accepted.connect(function() {
                                        choice = dialog.choice
                                    })
        }
        else
        {
            menu.open(this)
        }
    }

    menu: ContextMenu {
        enabled: valid && limited_choices.length <= num_large_choices
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
