import QtQuick 2.6
import Sailfish.Silica 1.0
import "../components"

    Row
    {
        id: item
        property var color: "white"
        property int level: 100
        property int high_level: 100
        property int low_level: 10
        property string name: ""
        property string type: ""
        property real value_fraction: level/(high_level == 0 ? 100 : high_level)

        spacing: Theme.paddingMedium
        bottomPadding: Theme.paddingMedium

        CylinderGraph
        {
            anchors.verticalCenter: parent.verticalCenter
            color: parent.color
            value: value_fraction
        }
        Label
        {
            anchors.verticalCenter: parent.verticalCenter
            width: Theme.itemSizeExtraSmall
            text: ""+Math.round(100*value_fraction)+"%"
            color: level <= low_level ? "red" : Theme.highlightColor
            font.pixelSize: Theme.fontSizeExtraSmall
        }
        Column
        {
            anchors.verticalCenter: parent.verticalCenter
            Label
            {
                text: name != "" ? name : qsTr("Unnamed supply")
                font.pixelSize: Theme.fontSizeExtraSmall
            }
            Label
            {
                text: type != "" ? type :  qsTr("Unknown type")
                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.secondaryColor
            }
        }
    }


