import QtQuick 2.6
import Sailfish.Silica 1.0
import "../pages/utils.js" as Utils

Dialog {
    id: dialog
    allowedOrientations: Orientation.All

    property string name
    property string choice
    property string new_choice: choice
    property var choices
    property var preferred_choices
    property string preferred_choice_suffix
    canAccept: false

    SilicaListView
    {
        anchors.fill: parent

        header: DialogHeader {}

        model: choices
        delegate: BackgroundItem {
            onClicked: {
                new_choice=choices[index]
                dialog.canAccept = true
            }
            Label {
                x: Theme.paddingLarge
                anchors.verticalCenter: parent.verticalCenter
                highlighted: choices[index]==new_choice
                text: Utils.ippName(name, choices[index])
                      + (Utils.has(preferred_choices, choices[index]) ? " "+preferred_choice_suffix : "")
            }
        }
    }

    onDone: {
        if (result == DialogResult.Accepted) {
            choice = new_choice
        }
    }
}
