import QtQuick 2.6
import Sailfish.Silica 1.0
import "../pages/utils.js" as Utils

Setting {
    property var choices: parent.getChoices(name)
    property var preferred_choices: []
    property string preferred_choice_suffix: ""

    property var actual_choices: Utils.fixupChoices(name, choices, parent.selectedFileType)

    property int num_large_choices: 8

    displayValue: Utils.ippName(name, choice != undefined ? choice : default_choice)

    onClicked: {
        if(actual_choices.length>num_large_choices)
        {
            var dialog = pageStack.push("LargeChoiceDialog.qml",
                                        {name:name, choice: choice != undefined ? choice : default_choice, choices: actual_choices,
                                         preferred_choices: preferred_choices, preferred_choice_suffix: preferred_choice_suffix})
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
        Repeater {
            model: actual_choices.length>num_large_choices ? 0 : actual_choices
            MenuItem {
                text: Utils.ippName(name, actual_choices[index])
                      + (Utils.has(preferred_choices, actual_choices[index]) ? " "+preferred_choice_suffix : "")
                onClicked:
                {
                    choice = actual_choices[index];
                }
            }
        }

    }

    hasMenu: !actual_choices.length>num_large_choices


}
