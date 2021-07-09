import QtQuick 2.0
import Sailfish.Silica 1.0

BackgroundItem {
    id: barButton

    property alias text: label.text
    property bool active: false
    highlightedColor: Theme.rgba(Theme.highlightBackgroundFromColor(Theme.highlightBackgroundColor, Theme.colorScheme), Theme.highlightBackgroundOpacity)

    onClicked:
    {
        active = !active
    }

    function _color(alpha) {
        alpha = alpha*0.25
        return Theme.rgba(Theme.highlightBackgroundColor, alpha)
    }

    Rectangle {
        x: -1
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: _color(1) }
            GradientStop { position: 0.2; color: _color(0.6) }
            GradientStop { position: 0.8; color: _color(0.4) }
            GradientStop { position: 1.0; color: "transparent" }
        }
    }

    Label {
        id: label
        leftPadding: Theme.paddingLarge
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        color: barButton.highlighted ? Theme.highlightColor : Theme.primaryColor
    }

    Image {
        id: image
        anchors.right: parent.right
        anchors.rightMargin: Theme.paddingMedium
        anchors.verticalCenter: parent.verticalCenter
        source: active ? "image://theme/icon-m-down" : "image://theme/icon-m-right"
    }

}
