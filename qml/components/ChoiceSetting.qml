import QtQuick 2.6
import Sailfish.Silica 1.0
import "../pages/utils.js" as Utils

Setting {
    property var choices: setting.choices
    property var preferred_choices: []
    property string preferred_choice_suffix: ""

    property var actual_choices: Utils.fixupChoices(setting.ippName, choices, parent.selectedFileType, parent.printer)

    property int num_large_choices: 8

    displayValue: Utils.ippName(setting.ippName, setting.value, strings)

    onClicked: {
        console.log("n", JSON.stringify(setting.ippName))
        console.log("c", JSON.stringify(choices))
        console.log("ac", JSON.stringify(actual_choices))

        if(hasMenu)
        {
            menu.open(this)
        }
        else if(actual_choices.length != 0)
        {
            var dialog = pageStack.push("LargeChoiceDialog.qml",
                                        {name:setting.ippName, choice: setting.value, choices: actual_choices,
                                         preferred_choices: preferred_choices, preferred_choice_suffix: preferred_choice_suffix,
                                         strings: strings})
            dialog.accepted.connect(function() {
                                        setting.value = dialog.choice
                                    })
        }
    }

    menu: ContextMenu {
        Repeater {
            model: actual_choices.length>num_large_choices ? 0 : actual_choices
            MenuItem {
                text: Utils.ippName(setting.ippName, actual_choices[index], strings)
                      + (Utils.has(preferred_choices, actual_choices[index]) ? " "+preferred_choice_suffix : "")
                onClicked:
                {
                    setting.value = actual_choices[index];
                }
            }
        }

    }

    hasMenu: actual_choices.length != 0 && (actual_choices.length <= num_large_choices)
}
