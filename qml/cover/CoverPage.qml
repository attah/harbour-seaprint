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

    Label {
        id: label
        anchors.centerIn: parent
        text: qsTr("Seaprint")
    }
}
