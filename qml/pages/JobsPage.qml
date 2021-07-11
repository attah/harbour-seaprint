import QtQuick 2.6
import Sailfish.Silica 1.0
import "utils.js" as Utils

Page {
    id: page
    allowedOrientations: Orientation.All

    property var printer

    Component.onCompleted: {
        printer.getJobs()
        console.log(JSON.stringify(printer.jobs))
    }

    Connections {
        target: printer
        onCancelStatus: {
            if(status != true) {
                notifier.notify(qsTr("Cancelling job failed"))
            }
        }
    }

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent

        // PullDownMenu and PushUpMenu must be declared in SilicaFlickable, SilicaListView or SilicaGridView
        PullDownMenu {
//            MenuItem {
//                text: qsTr("Remove all")
//                onClicked: {
//                    console.log("todo")
//                }
//            }
            MenuItem {
                text: qsTr("Refresh")
                onClicked: {
                    console.log(JSON.stringify(printer.jobs))
                    printer.getJobs();
                }
            }
        }

        SilicaListView {
            anchors.fill: parent
            id: listView

            model: printer.jobs.length

            header: PageHeader {
                id: pageHeader
                title: printer.attrs["printer-name"].value
                description: printer.jobs.length==1 ?  printer.jobs.length+" "+qsTr("job") : printer.jobs.length+" "+qsTr("jobs")
            }

            delegate: ListItem {
                id: jobDelegate

                Label {
                    id: idLabel
                    leftPadding: Theme.horizontalPageMargin
                    anchors.verticalCenter: parent.verticalCenter
                    color: Theme.highlightColor
                    text: printer.jobs[index]["job-id"].value
                    Component.onCompleted: console.log(JSON.stringify(printer.jobs))
                }

                Column {
                    anchors.left: idLabel.right
                    anchors.leftMargin: Theme.horizontalPageMargin
                    anchors.verticalCenter: parent.verticalCenter
                    Label {
                        text: printer.jobs[index]["job-name"] ? printer.jobs[index]["job-name"].value : qsTr("Untitled job")
                        Component.onCompleted: console.log(JSON.stringify(printer.jobs))
                    }
                    Label {
                        font.pixelSize: Theme.fontSizeTiny
                        color: Theme.secondaryColor
                        property var msg: printer.jobs[index]["job-printer-state-message"] && printer.jobs[index]["job-printer-state-message"].value != ""
                                         ? " ("+printer.jobs[index]["job-printer-state-message"].value+")" : ""
                        text: Utils.ippName("job-state", printer.jobs[index]["job-state"].value)+msg
                    }
                }

                RemorseItem {
                    id: cancelRemorse
                }

                menu: ContextMenu {
                    MenuItem {
                        text: qsTr("Cancel job")
                        onClicked: {
                            cancelRemorse.execute(jobDelegate, qsTr("Cancelling job"),
                                                  function() {printer.cancelJob(printer.jobs[index]["job-id"].value) })
                        }
                    }
                }
            }

        }

    }
}
