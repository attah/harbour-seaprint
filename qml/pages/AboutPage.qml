import QtQuick 2.0
import Sailfish.Silica 1.0
import "../components"

Page {
    id: aboutPage

    allowedOrientations: Orientation.Portrait | Orientation.Landscape | Orientation.LandscapeInverted

    SilicaFlickable {
        id: aboutFlickable
        anchors.fill: parent
        contentHeight: column.height

        PullDownMenu
        {
            MenuItem
            {
                text: qsTr("GitHub repository")
                onClicked: Qt.openUrlExternally("https://github.com/attah/harbour-seaprint")
            }

            MenuItem
            {
                text: qsTr("Report an Issue")
                onClicked: Qt.openUrlExternally("https://github.com/attah/harbour-seaprint/issues")
            }
        }

        Column {
            id: column
            anchors {
                top: parent.top
                horizontalCenter: parent.horizontalCenter
            }
            width: Math.min(Screen.width, aboutFlickable.width)
            spacing: Theme.paddingLarge

            Item {
                width: parent.width
                height: Theme.paddingLarge
            }

            SectionHeader { text: qsTr("What's") + " SeaPrint?" }

            Image {
                anchors.horizontalCenter: parent.horizontalCenter
                source: Qt.resolvedUrl("/usr/share/icons/hicolor/172x172/apps/harbour-seaprint.png")
                width: Theme.iconSizeExtraLarge
                height: Theme.iconSizeExtraLarge
                smooth: true
                asynchronous: true
            }
            AboutLabel {
                font.pixelSize: Theme.fontSizeLarge
                color: Theme.highlightColor
                text: "SeaPrint 0.2-1"
            }
            AboutLabel {
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                text: "© 2019 Anton Thomasson"
            }
            AboutLabel {
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                text: qsTr("A simple tool to print PDF file or JPG file on Sailfish OS.")
            }

            SectionHeader { text: qsTr("Translations") }

            AboutLabel {
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("Chinese")+" - dashinfantry"
            }

            SectionHeader { text: qsTr("Licensing") }

            AboutLabel {
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("Source code is available at GitHub. Translations, bug reports and other contributions are welcome!")
            }
            Button {
                text: "GitHub"
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: Qt.openUrlExternally("https://github.com/attah/harbour-seaprint")
            }
            AboutLabel {
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                text: qsTr("SeaPrint is free software released under the GNU General Public License (GPL), version 3 or later.")
            }
          }
        }
      }
