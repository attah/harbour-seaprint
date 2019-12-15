import QtQuick 2.0
import Sailfish.Silica 1.0

Setting {

    //TODO

    property int low
    property int high
    property int choice_low
    property int choice_high

    ValueButton {
        enabled: valid
        anchors.verticalCenter: parent.verticalCenter
        label: prettyName
        value: choice ? choice : default_choice
        onClicked: parent.clicked()
    }

    property var menu: ContextMenu {
        id: menu
        MenuItem {

        }

    }


}
