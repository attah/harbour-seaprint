import QtQuick 2.6
import Sailfish.Silica 1.0

Page {

    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill:  parent
        contentHeight: column.height

        Column {
            id: column
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width - 2*Theme.paddingLarge
            spacing: Theme.paddingLarge

            PageHeader{
                title: qsTr("Settings")
            }

            TextSwitch {
                text: qsTr("Show nag-screen")
                description: qsTr("Display the warning page about optional dependencies not being installed, if they are not installed.")
                checked: nagScreenSetting.value != nagScreenSetting.expectedValue
                onCheckedChanged: {
                    if(checked)
                    {
                        nagScreenSetting.value=0
                    }
                    else
                    {
                        nagScreenSetting.value = nagScreenSetting.expectedValue
                    }
                }
            }

            TextSwitch {
                text: qsTr("Consider additional formats")
                description: qsTr("Some printers support more formats than they advertise correctly. However, additional formats can be inferred from other attributes. Mainly relevant for Postscript-compatible printers that also support PDF.")
                checked: considerAdditionalFormatsSetting.value
                onCheckedChanged: {
                    considerAdditionalFormatsSetting.value = checked
                }
            }

            TextSwitch {
                text: qsTr("Ignore SSL errors")
                description: qsTr("In order to work with self-signed certificates of printers and CUPS instances, SSL errors needs to be ignored.")
                checked: ignoreSslErrorsSetting.value
                onCheckedChanged: {
                    ignoreSslErrorsSetting.value = checked
                }
            }

            TextSwitch {
                text: qsTr("Enable debug log")
                description: qsTr("Print debug log messages to console (if launched from there) or journal if launched normally. Takes effect on app (re-)start.")
                checked: debugLogSetting.value
                onCheckedChanged: {
                    debugLogSetting.value = checked
                }
            }

            TextSwitch {
                text: qsTr("Allow external connections")
                description: qsTr("Allow SeaPrint to fetch recources like icons and option name dictionaries from addresses that are not the printer itself. Some printer makers store icons on their website as opposed to locally on the printer.")
                checked: allowExternalConnectionsSetting.value
                onCheckedChanged: {
                    allowExternalConnectionsSetting.value = checked
                }
            }
        }
    }
}
