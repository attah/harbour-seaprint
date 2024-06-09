import QtQuick 2.6
import Sailfish.Silica 1.0
import "utils.js" as Utils

Page {
    property bool busyPage: true

    allowedOrientations: Orientation.All

    Component.onCompleted: {
        appWin.busyMessage = printer.busyMessage
    }

    property var printer
    backNavigation: false
    Connections {
        target: printer
        onJobFinished: {
            pageStack.pop()
            if(errMsg == "")
            {
                notifier.notify(qsTr("Print success"))
            }
            else
            {
                notifier.notify(qsTr("Print failed: " + errMsg))
            }
        }
        onBusyMessageChanged: {
            appWin.busyMessage = printer.busyMessage
        }

        onProgressChanged: {
            appWin.progress = printer.progress
        }
    }

    BusyLabel {
        text: printer.busyMessage+"\n"+printer.progress;
        running: true
    }

    Component.onDestruction: {
        appWin.busyMessage = ""
        appWin.progress = ""
    }
}
