import QtQuick 2.0
import Sailfish.Silica 1.0


Page {
    id: page
    allowedOrientations: Orientation.All

    property var printer

    Component.onCompleted: {
        printer.getJobs()
        console.log(JSON.stringify(printer.jobs))
    }

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent

        // PullDownMenu and PushUpMenu must be declared in SilicaFlickable, SilicaListView or SilicaGridView
        PullDownMenu {
            MenuItem {
                text: qsTr("Remove all")
                onClicked: {
                    console.log("todo")
                }
            }
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
                title: printer.name

            }

            delegate: ListItem {

                Label {
                    leftPadding: Theme.horizontalPageMargin
                    anchors.verticalCenter: parent.verticalCenter
                    text: printer.jobs[index]["job-id"].value
                    Component.onCompleted: console.log(JSON.stringify(printer.jobs))
                }
            }

        }

    }
}
