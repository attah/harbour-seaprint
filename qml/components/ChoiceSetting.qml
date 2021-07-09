import QtQuick 2.0
import Sailfish.Silica 1.0
import seaprint.convertchecker 1.0
import seaprint.mimer 1.0
import "../pages/utils.js" as Utils

Setting {
    property var choices: parent.getChoices(name)
    property var preferred_choices: []
    property string preferred_choice_suffix: ""

    property var limited_choices: Utils.limitChoices(name, choices, parent.selectedFileType, ConvertChecker)

    property int num_large_choices: 8

    displayValue: Utils.ippName(name, choice ? choice : default_choice)

    onClicked: {
        if(limited_choices.length>num_large_choices)
        {
            var dialog = pageStack.push("LargeChoiceDialog.qml",
                                        {name:name, choice: choice ? choice : default_choice, choices: limited_choices,
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
            model: limited_choices.length>num_large_choices ? 0 : limited_choices
            MenuItem {
                text: Utils.ippName(name, limited_choices[index])
                      + (Utils.has(preferred_choices, limited_choices[index]) ? " "+preferred_choice_suffix : "")
                onClicked:
                {
                    choice = limited_choices[index];
                }
            }
        }

    }

    hasMenu: !limited_choices.length>num_large_choices


}
