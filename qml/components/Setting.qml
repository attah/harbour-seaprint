import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    height: button.height + (menu != undefined ? menu.height : 0)
    width: parent.width

    property string name
    property string prettyName
    property int tag
    property bool valid: false

    property var choice
    property var default_choice

    signal clicked()
    onClicked: {
        if (hasMenu)
        {
            menu.open(this)
        }
    }

    property alias displayValue: button.value

    ValueButton {
        id: button
        enabled: valid
        label: prettyName
        onClicked: parent.clicked()
    }

    property var menu
    property bool hasMenu: true

}
