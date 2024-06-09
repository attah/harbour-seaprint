import QtQuick 2.6
import Sailfish.Silica 1.0
import seaprint.mimer 1.0
import seaprint.convertchecker 1.0
import seaprint.ippprinter 1.0
import "utils.js" as Utils
import "../components"

Page {
    allowedOrientations: Orientation.All

    id: page
    property alias printer: settingsColumn.printer
    property string selectedFile
    property string selectedFileType: Mimer.get_type(selectedFile)


    Connections {
        target: wifi
        onConnectedChanged: {
            if(!wifi.connected) {
                pageStack.pop()
            }
        }
    }

    Connections {
        target: printer
        onDataChanged: {
            if(!printer.isOk) {
                pageStack.pop()
            }
        }
    }

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent
        contentHeight: settingsColumn.height

        // PullDownMenu and PushUpMenu must be declared in SilicaFlickable, SilicaListView or SilicaGridView
        PullDownMenu {

//            MenuLabel {
//                text:  qsTr("Default settings for %1 on this printer").arg(db.simplifyType(selectedFileType).translatable)
//                visible: printer.attrs.hasOwnProperty("printer-uuid")
//            }
//            MenuItem {
//                text: qsTr("Reset default settings")
//                visible: printer.attrs.hasOwnProperty("printer-uuid")
//                onClicked: {
//                    db.removeJobSettings(printer.attrs["printer-uuid"].value, selectedFileType);
//                    pageStack.pop();
//                }
//            }

//            MenuItem {
//                text: qsTr("Save as default settings")
//                visible: printer.attrs.hasOwnProperty("printer-uuid")
//                onClicked: {
//                    var tmp = jobParams;
//                    // Varies between documents, would be confusing to save
//                    delete tmp["page-ranges"];
//                    db.setJobSettings(printer.attrs["printer-uuid"].value, selectedFileType, JSON.stringify(tmp))
//                }
//            }

            MenuItem {
                text: qsTr("Print")
                onClicked: {
                    pageStack.replace(Qt.resolvedUrl("BusyPage.qml"),{printer:printer},
                                      PageStackAction.Immediate)
                    printer.print(page.selectedFile)
                }
            }
        }

        VerticalScrollDecorator {}

        Column {
            id: settingsColumn
            width: parent.width

            property IppPrinter printer
            property alias selectedFileType: page.selectedFileType

            PageHeader {
                id: pageHeader
                title: Utils.unknownForEmptyString(printer.name)
                description: Utils.basename(selectedFile)
            }

            ChoiceSetting {
                setting: printer.sides
                prettyName: qsTr("Sides")
            }
            ChoiceSetting {
                setting: printer.media
                prettyName: qsTr("Print media")
                preferred_choices: setting.preferredChoices
                preferred_choice_suffix: qsTr("(loaded)")
            }
            IntegerSetting {
                setting: printer.copies
                prettyName: qsTr("Copies")
                valid: _valid || Utils.supports_raster(printer)
                minimum_high: 99
            }
            ChoiceSetting {
                setting: printer.multipleDocumentHandling
                prettyName: qsTr("Collated copies")
            }
            RangeSetting {
                setting: printer.pageRanges
                prettyName: qsTr("Page range")
                valid: (selectedFileType == Mimer.PDF || Mimer.isOffice(selectedFileType) || (_valid && selectedFileType == Mimer.Postscript))

                property var pdfpages: ConvertChecker.pdfPages(selectedFile)
                high: pdfpages == 0 ? 65535 : pdfpages
                acceptRangeList: true
            }
            ChoiceSetting {
                setting: printer.numberUp
                prettyName: qsTr("Pages per page")
                valid: _valid && !Mimer.isImage(selectedFileType)
                DependentOn {
                    target: transferFormatSetting
                    values: [Mimer.PDF, Mimer.Postscript]
                }
            }
            ChoiceSetting {
                setting: printer.colorMode
                prettyName: qsTr("Color mode")
            }
            ChoiceSetting {
                setting: printer.printQuality
                prettyName: qsTr("Quality")
            }
            ChoiceSetting {
                setting: printer.resolution
                prettyName: qsTr("Resolution")
            }
            ChoiceSetting {
                setting: printer.scaling
                prettyName: qsTr("Scaling")
                valid: _valid && Mimer.isImage(selectedFileType) && selectedFileType != Mimer.SVG
                DependentOn {
                    target: transferFormatSetting
                    values: [Mimer.JPEG, Mimer.PNG]
                }
            }
            ChoiceSetting {
                id: transferFormatSetting
                setting: printer.documentFormat
                prettyName: qsTr("Transfer format")
            }

            BarButton {
                id: mediaButton
                text: qsTr("Media handling")
            }
            ChoiceSetting {
                visible:  mediaButton.active
                setting: printer.mediaType
                prettyName: qsTr("Media type")
            }
            ChoiceSetting {
                visible:  mediaButton.active
                setting: printer.mediaSource
                prettyName: qsTr("Media source")
            }
            ChoiceSetting {
                visible:  mediaButton.active
                setting: printer.outputBin
                prettyName: qsTr("Output bin")
            }

            BarButton {
                id: marginsButton
                text: qsTr("Margins")
            }
            ChoiceSetting {
                visible: marginsButton.active
                setting: printer.topMargin
                prettyName: qsTr("Top")
                valid: _valid && Mimer.isImage(selectedFileType)
            }
            ChoiceSetting {
                visible: marginsButton.active
                setting: printer.bottomMargin
                prettyName: qsTr("Bottom")
                valid: _valid && Mimer.isImage(selectedFileType)
            }
            ChoiceSetting {
                visible: marginsButton.active
                setting: printer.leftMargin
                prettyName: qsTr("Left")
                valid: _valid && Mimer.isImage(selectedFileType)
            }
            ChoiceSetting {
                visible: marginsButton.active
                setting: printer.rightMargin
                prettyName: qsTr("Right")
                valid: _valid && Mimer.isImage(selectedFileType)
            }
        }
    }
}
