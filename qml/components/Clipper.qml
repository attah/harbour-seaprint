import QtQuick 2.0
import Sailfish.Silica 1.0

MouseArea {
    anchors.fill: parent
    onPressAndHold: {
        Clipboard.text = parent.text
        notifier.notify(qsTr("Copied to clipboard"))
    }
}
