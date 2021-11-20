import QtQuick 2.6
import Sailfish.Silica 1.0
import seaprint.convertchecker 1.0
import "../components"

Page {
    id: aboutPage

    allowedOrientations: Orientation.All

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

            SectionHeader { text: qsTr("About") + " SeaPrint" }

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
                text: "SeaPrint v" + Qt.application.version
            }

            AboutLabel {
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                text: "© 2019-2021 Anton Thomasson"
            }


            AboutLabel {
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                text: qsTr("Network printing for Sailfish OS.")
            }

            SectionHeader { text: qsTr("Icons") }

            AboutLabel {
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("Icon made by Gregguh.")
            }

            SectionHeader { text: qsTr("General contributions") }
            AboutLabel {
                font.pixelSize: Theme.fontSizeSmall
                text: "karry\n"+
                      "piggz\n"+
                      "Rudi Timmermans"
            }

            SectionHeader { text: qsTr("Special thanks to") }
            AboutLabel {
                font.pixelSize: Theme.fontSizeSmall
                text: "dcaliste"
            }

            SectionHeader { text: qsTr("Translations") }

            AboutLabel {
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("Chinese")+" - dashinfantry\n"+
                      qsTr("Dutch")+" - JSEHV, Rudi Timmermans\n"+
                      qsTr("French")+" - ensag-dev, Quentí\n"+
                      qsTr("German")+" -  black-sheep-dev\n"+
                      qsTr("Polish")+" - atlochowski\n"+
                      qsTr("Spanish")+" - carmenfdezb"
            }

            SectionHeader { text: qsTr("Optional dependencies") }

            AboutLabel {
                font.pixelSize: Theme.fontSizeSmall
                text: "calligraconverter - "+(ConvertChecker.calligra ? qsTr("Installed") : qsTr("Not installed"))
            }

            SectionHeader { text: qsTr("Donations") }

            AboutLabel {
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
                text: "paypal.me/AntonThomasson"
                font.underline: true
                MouseArea {
                    anchors.fill:parent
                    onClicked: Qt.openUrlExternally("https://"+parent.text)
                }
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
