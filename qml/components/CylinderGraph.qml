import QtQuick 2.6
import Sailfish.Silica 1.0

Item {
    id: cylinderGraph
    property color color
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
        rotation: 90
        transformOrigin: Item.Center

        gradient: Gradient {
            GradientStop { position: 0.0; color: "#80000000"}
            GradientStop { position: 0.2; color: "#40FFFFFF"}
            GradientStop { position: 0.4; color: "#00000000"}
            GradientStop { position: 0.8; color: "#00000000"}
            GradientStop { position: 0.9; color: "#40FFFFFF"}
            GradientStop { position: 1.0; color: "#80000000"}
        }
    }
    Rectangle {
        width: Theme.itemSizeExtraSmall*parent.value
        height: (Theme.itemSizeExtraSmall/3)
        rotation: 90
        transformOrigin: Item.Center

        gradient: Gradient {
            GradientStop { position: 0.0; color: withOpacity(0.2, cylinderGraph.color)}
            GradientStop { position: 0.1; color: cylinderGraph.color}
            GradientStop { position: 0.9; color: cylinderGraph.color}
            GradientStop { position: 1.0; color: withOpacity(0.2, cylinderGraph.color)}
        }

        anchors.horizontalCenter: effect.horizontalCenter
        anchors.bottom: effect.bottom
        anchors.bottomMargin: -1*(Theme.itemSizeExtraSmall-Theme.itemSizeExtraSmall*parent.value)/2
//        color: cylinderGraph.color
//        opacity: 0.7
    }

    function withOpacity(opacity, color) {
        Qt.rgba(color.r, color.g, color.b, opacity)
    }
}
