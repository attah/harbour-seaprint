import QtQuick 2.6
import Sailfish.Silica 1.0
import "utils.js" as Utils
import "../components"

Page {
    allowedOrientations: Orientation.All
    property var printer

    SilicaFlickable {
        id: detailsFlickable
        anchors.fill: parent
        contentHeight: col.height

        PullDownMenu {
            visible: printer.identifySupported

            MenuItem {
                id: identifyLabel
                text:  qsTr("Identify printer")
                onClicked: printer.identify()
            }
        }


        Column {
            id: col

            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(Screen.width, detailsFlickable.width)-2*Theme.paddingLarge
            PageHeader {
                title: qsTr("Details")
                description: Utils.unknownForEmptyString(printer.name)
            }

            // TODO:
            // location?
            // message from operator?
            // organization /-unit
            // supply info uri?
            // identify button?

            SectionHeader {
                text: qsTr("Make and model")
            }

            Label {
                text: Utils.unknownForEmptyString(printer.makeAndModel)
            }

            SectionHeader {
                text: qsTr("Location")
                visible: printer.location != ""
            }

            Label {
                text: printer.location
                visible: printer.location != ""
            }


            SectionHeader {
                text: qsTr("Printer status")
            }

            Label {
                text: Utils.ippName("printer-state", printer.state)
            }

            Repeater
            {
                model: printer.stateReasons
                Label {
                    text: Utils.ippName("printer-state-reasons", printer.stateReasons[index])
                }
            }

            Label {
                text: printer.stateMessage
                visible: printer.stateMessage != ""
            }

            SectionHeader {
                text: qsTr("Supply status")
                visible: printer.supplies.length != 0
            }

            Repeater
            {
                model: printer.supplies.length

                SupplyItem
                {
                    name: printer.supplies[index].name
                    type: printer.supplies[index].type
                    colors: printer.supplies[index].colors
                    percentage: printer.supplies[index].percentage
                    isLow: printer.supplies[index].isLow
                }
            }

            SectionHeader {
                text: qsTr("IPP versions")
                visible: printer.ippVersions.length != 0
            }


            Label
            {
                text: printer.ippVersions.join(" ")
                visible: printer.ippVersions.length != 0
            }

            SectionHeader {
                text: qsTr("IPP features")
                visible: printer.ippFeatures.length != 0
            }


            Repeater
            {
                model: printer.ippFeatures.length
                Label
                {
                    text: printer.ippFeatures[index]
                }
            }

            SectionHeader {
                text: qsTr("Firmware")
                visible: printer.firmware.length != 0
            }

            Repeater
            {
                model: printer.firmware.length
                Label
                {
                    text: printer.firmware[index].name + ": " + printer.firmware[index].version
                }

            }

            SectionHeader {
                text: qsTr("Performance")
                visible: printer.pagesPerMinute != 0 || printer.pagesPerMinuteColor != 0
            }

            Label
            {
                text: ""+printer.pagesPerMinute+" "+qsTr("pages/min")
                visible: printer.pagesPerMinute != 0

            }

            Label
            {
                text: ""+printer.pagesPerMinuteColor+" "+qsTr("pages/min (color)")
                visible: printer.pagesPerMinuteColor != 0
            }

        }
    }
}
