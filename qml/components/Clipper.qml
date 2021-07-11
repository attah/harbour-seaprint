import QtQuick 2.6
import Sailfish.Silica 1.0

MouseArea {
    anchors.fill: parent
    onPressAndHold: {
        Clipboard.text = parent.text
        notifier.notify(qsTr("Copied to clipboard"))
    }
}
