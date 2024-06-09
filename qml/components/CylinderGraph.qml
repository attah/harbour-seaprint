import QtQuick 2.6
import Sailfish.Silica 1.0

Item {
    id: cylinderGraph
    property var colors
    property real value
    width: Theme.itemSizeSmall/2
    height: Theme.itemSizeSmall
    anchors.verticalCenter: parent.verticalCenter

    Rectangle {
        id: effect
        z: 1
        anchors.centerIn: parent

        width: Theme.itemSizeExtraSmall
        height: Theme.itemSizeExtraSmall/3
        rotation: 270
        transformOrigin: Item.Center

        gradient: Gradient {
            GradientStop { position: 0.0; color: "#40888888"}
            GradientStop { position: 0.1; color: "#20FFFFFF"}
            GradientStop { position: 0.2; color: "#00000000"}
            GradientStop { position: 0.6; color: "#00000000"}
            GradientStop { position: 0.8; color: "#20FFFFFF"}
            GradientStop { position: 1.0; color: "#40888888"}
        }
    }
    Rectangle {
        id: fill
        width: Theme.itemSizeExtraSmall*parent.value
        height: (Theme.itemSizeExtraSmall/3)
        rotation: 270
        transformOrigin: Item.Center

        gradient: Gradient {
            id: gradient
        }

        Component
        {
            id: stopComponent
            GradientStop {}
        }

        Component.onCompleted: {
            var stops = [];

            stops.push(stopComponent.createObject(fill, {"position": 0.0, "color": "#00000000"}));
            stops.push(stopComponent.createObject(fill, {"position": 0.1, "color": colors[0]}));

            for(var i = 0; i < colors.length; i++)
            {
                var pos = 0.1+(i+0.5)*(0.8/colors.length);
                stops.push(stopComponent.createObject(fill, {"position": pos, "color": colors[i]}));
            }

            stops.push(stopComponent.createObject(fill, {"position": 0.9, "color": colors[colors.length-1]}));
            stops.push(stopComponent.createObject(fill, {"position": 1.0, "color": "#00000000"}));

            gradient.stops = stops;

        }

        anchors.horizontalCenter: effect.horizontalCenter
        anchors.bottom: effect.bottom
        anchors.bottomMargin: -1*(effect.width-width)/2
    }
}
