import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground {
    Icon {
        anchors.centerIn: parent
        height: parent.width * 0.9
        width: parent.width * 0.9
        opacity: 0.33
        source: "../pages/icon-seaprint-nobg-bw.svg"
    }

    Column {
        width: parent.width
        anchors.verticalCenter: parent.verticalCenter
        Label {
            id: msgLabel
            anchors.horizontalCenter: parent.horizontalCenter
            text: appWin.busyMessage != "" ? appWin.busyMessage : qsTr("SeaPrint")
        }
        Label {
            id: progressLabel
            anchors.horizontalCenter: parent.horizontalCenter
            text: appWin.progress
        }
    }


}
