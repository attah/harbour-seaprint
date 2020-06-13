import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    SilicaFlickable {
        anchors.fill:  parent
        contentHeight: column.height

        Column {
            id: column
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width - 2*Theme.paddingLarge
            spacing: Theme.paddingLarge

            PageHeader{}

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
                text: qsTr("Always convert to raster format")
                description: qsTr("Force conversion to PWG/URF raster format. This is mainly intended for testing.")
                checked: alwaysConvertSetting.value
                onCheckedChanged: {
                    alwaysConvertSetting.value = checked
                }
            }

            TextSwitch {
                text: qsTr("Remove redundant attributes")
                description: qsTr("Remove redundant IPP attributes, if they are also conveyed in the transfer format. Some printers reject the job even if the settings are consistent.")
                checked: removeRedundantConvertAttrsSetting.value
                onCheckedChanged: {
                    removeRedundantConvertAttrsSetting.value = checked
                }
            }

        }
    }
}
