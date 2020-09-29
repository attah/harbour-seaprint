import QtQuick 2.6
import Sailfish.Silica 1.0

    Row
    {
        id: item
        property var color: "white"
        property int level: 100
        property int high_level: 100
        property int low_level: 0
        property string name: qsTr("Unknown supply")
        property string type: qsTr("Unknown type")

        spacing: Theme.paddingMedium
        bottomPadding: Theme.paddingMedium

        Icon
        {
            anchors.verticalCenter: parent.verticalCenter
            source: "image://theme/icon-s-clear-opaque-background"
            color: item.color
        }
        Label
        {
            anchors.verticalCenter: parent.verticalCenter
            width: Theme.itemSizeSmall
            text: ""+(100*level/high_level)+"%"
            color: Theme.highlightColor
        }
        Column
        {
            leftPadding: Theme.paddingMedium

            Label
            {
                text: name
                font.pixelSize: Theme.fontSizeExtraSmall
            }
            Label
            {
                text: type
                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.secondaryColor
            }
        }
    }


