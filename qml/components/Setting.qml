import QtQuick 2.6
import Sailfish.Silica 1.0

Item {
    height: button.height + (menu != undefined ? menu.height : 0) + resetMenu.height
    width: parent.width

    property string name
    property string prettyName
    property int tag
    property string subkey: ""
    property bool _valid: parent.isValid(name)
    property bool valid: _valid

    property var choice
    property var default_choice: parent.getDefaultChoice(name)

    Component.onCompleted: parent.setInitialChoice(this)

    onChoiceChanged: parent.choiceMade(this)

    signal clicked()
    onClicked: {
        if (hasMenu)
        {
            menu.open(this)
        }
    }

    signal pressAndHold()
    onPressAndHold: {
        resetMenu.open(this)
    }

    property alias displayValue: button.value

    ValueButton {
        id: button
        enabled: valid
        label: prettyName
        onClicked: parent.clicked()
        onPressAndHold: parent.pressAndHold()
        valueColor: choice != undefined ? Theme.highlightColor : Theme.secondaryHighlightColor
    }

    property var menu
    property bool hasMenu: true

    function reset()
    {
        choice = undefined
    }

    ContextMenu {
        id: resetMenu

        MenuItem {
            text: qsTr("Reset")
            onClicked: reset()
        }
    }

}
