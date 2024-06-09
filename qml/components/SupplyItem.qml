import QtQuick 2.6
import Sailfish.Silica 1.0
import "../components"

    Row
    {
        id: item
        property string name: ""
        property string type: ""
        property var colors: ["white"]
        property int percentage: 100
        property bool isLow: false

        spacing: Theme.paddingMedium
        bottomPadding: Theme.paddingMedium

        CylinderGraph
        {
            anchors.verticalCenter: parent.verticalCenter
            colors: parent.colors
            value: percentage/100
        }
        Label
        {
            anchors.verticalCenter: parent.verticalCenter
            width: Theme.itemSizeExtraSmall
            text: ""+percentage+"%"
            color: isLow ? "red" : Theme.highlightColor
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


