import QtQuick 2.6
import Sailfish.Silica 1.0

Item {
    height: button.height + (menu != undefined ? menu.height : 0) + resetMenu.height
    width: parent.width

    property string prettyName
    property var setting
    property bool valid: _valid
    property bool _valid: true

    property var strings: parent.printer.strings

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
        valueColor: setting.isSet ? Theme.highlightColor : Theme.secondaryHighlightColor
    }

    property var menu
    property bool hasMenu: true

    function reset()
    {
        setting.value = undefined
    }

    ContextMenu {
        id: resetMenu

        MenuItem {
            text: qsTr("Reset")
            onClicked: reset()
        }
    }

    function highlight()
    {
        highlightAnimation.start()
    }

    NumberAnimation
    {
        id: highlightAnimation
        target: fillRectangle
        property: "opacity"
        from: 0.5
        to: 0
        duration: 1000
    }

    Rectangle {
        id: fillRectangle
        anchors.fill: parent
        color: Theme.highlightBackgroundColor
        opacity: 0
    }

}
