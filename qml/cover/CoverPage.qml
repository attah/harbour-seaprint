import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground {
    Icon {
        anchors.centerIn: parent
        height: parent.width
        width: parent.width
        sourceSize.height: height
        sourceSize.width: width
        opacity: 0.4
        source: "../pages/icon-seaprint-nobg-bw.png"
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
