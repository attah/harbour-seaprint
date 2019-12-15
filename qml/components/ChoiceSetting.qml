import QtQuick 2.0
import Sailfish.Silica 1.0

Setting {
    property var choices

    function prettifyChoice(name, value)
    {
        switch(name) {
        case "print-quality":
            switch(value) {
            case 3:
                return qsTr("draft");
            case 4:
                return qsTr("normal");
            case 5:
                return qsTr("high");
            default:
                return qsTr("unknown quality ")+value
            }
        case "orientation-requested":
            switch(value) {
            case 3:
                return qsTr("portrait");
            case 4:
                return qsTr("landscape");
            case 5:
                return qsTr("reverse landscape");
            case 6:
                return qsTr("reverse portrait");
            default:
                return qsTr("unknown orientation ")+value
            }
        case "printer-resolution":

            var units = "";
            if(value.units==3) {
                units=qsTr("dpi");
            } else if (units==4){
                units=qsTr("dots/cm")
            }
            return ""+value.x+"x"+value.y+units;
        }
        return value;
    }
    ValueButton {
        enabled: valid
        anchors.verticalCenter: parent.verticalCenter
        label: prettyName
        value: prettifyChoice(name, choice ? choice : default_choice)
        onClicked: parent.clicked()
    }

    property var menu: ContextMenu {
        id: menu
        Repeater {
            model: choices
            MenuItem {
                text: prettifyChoice(name, choices[index])
                onClicked:
                {
                    choice = choices[index];
                }
            }
        }

    }


}
