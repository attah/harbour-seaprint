import QtQuick 2.0
import Sailfish.Silica 1.0
import "utils.js" as Utils

Page {
    property var printer
    backNavigation: false
    Connections {
        target: printer
        onJobFinished: {
            var msg = ""
            if (printer.jobAttrs.hasOwnProperty("job-state-message") && printer.jobAttrs["job-state-message"].value != "") {
                msg = printer.jobAttrs["job-state-message"].value
            }
            else if (printer.jobAttrs.hasOwnProperty("job-state")) {
                msg = Utils.ippName("job-state", printer.jobAttrs["job-state"].value)
            }
            pageStack.pop()
            if(status == true) {
                notifier.notify(qsTr("Print success: ") + msg)
            }
            else {
                notifier.notify(qsTr("Print failed: ") + msg)
            }
        }
    }

    BusyLabel {
        text: printer.busyMessage
        running: true
    }
}
