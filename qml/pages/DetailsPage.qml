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
            visible: printer.attrs.hasOwnProperty("identify-actions-supported")

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
                description: Utils.unknownForEmptyString(printer.attrs["printer-name"].value)
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
                text: Utils.unknownForEmptyString(printer.attrs["printer-make-and-model"].value)
            }

            SectionHeader {
                text: qsTr("Location")
                visible: Utils.existsAndNotEmpty("printer-location", printer)
            }

            Label {
                text: printer.attrs["printer-location"].value
                visible: Utils.existsAndNotEmpty("printer-location", printer)
            }


            SectionHeader {
                text: qsTr("Printer status")
            }

            Label {
                text: Utils.ippName("printer-state", printer.attrs["printer-state"].value)
            }

            Repeater
            {
                model: printer.attrs["printer-state-reasons"].value
                Label {
                    text: Utils.ippName("printer-state-reasons", printer.attrs["printer-state-reasons"].value[index])
                }
            }

            Label {
                text: printer.attrs["printer-state-message"].value
                visible: printer.attrs.hasOwnProperty("printer-state-message")
            }

            SectionHeader {
                text: qsTr("Supply status")
                visible: printer.attrs.hasOwnProperty("marker-colors")
            }

            Repeater
            {
                model: printer.attrs["marker-colors"].value.length

                SupplyItem
                {
                    color: printer.attrs["marker-colors"].value[index]
                    level: printer.attrs["marker-levels"].value[index]
                    high_level: printer.attrs["marker-high-levels"].value[index]
                    low_level: printer.attrs["marker-low-levels"].value[index]
                    name: printer.attrs["marker-names"].value[index]
                    type: printer.attrs["marker-types"].value[index]

                }
            }

            SectionHeader {
                text: qsTr("IPP versions")
                visible: printer.attrs.hasOwnProperty("ipp-versions-supported")
            }


            Label
            {
                text: printer.attrs["ipp-versions-supported"].value.join(" ")
                visible: printer.attrs.hasOwnProperty("ipp-versions-supported")

            }

            SectionHeader {
                text: qsTr("IPP features")
                visible: printer.attrs.hasOwnProperty("ipp-features-supported")
            }


            Repeater
            {
                model: printer.attrs["ipp-features-supported"].value.length
                Label
                {
                    text: printer.attrs["ipp-features-supported"].value[index]
                }

            }

            SectionHeader {
                text: qsTr("Firmware")
                visible: printer.attrs.hasOwnProperty("marker-colors")
            }

            Repeater
            {
                model: printer.attrs["printer-firmware-name"].value.length
                Label
                {
                    text: printer.attrs["printer-firmware-name"].value[index]+": "
                          + printer.attrs["printer-firmware-string-version"].value[index]
                }

            }

            SectionHeader {
                text: qsTr("Performance")
                visible: printer.attrs.hasOwnProperty("pages-per-minute") || printer.attrs.hasOwnProperty("pages-per-minute-color")
            }

            Label
            {
                text: ""+printer.attrs["pages-per-minute"].value+" "+qsTr("pages/min")
                visible: printer.attrs.hasOwnProperty("pages-per-minute")

            }

            Label
            {
                text: ""+printer.attrs["pages-per-minute-color"].value+" "+qsTr("pages/min (color)")
                visible: printer.attrs.hasOwnProperty("pages-per-minute-color")

            }

        }
    }
}
