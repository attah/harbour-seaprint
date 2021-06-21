import QtQuick 2.6
import Sailfish.Silica 1.0
import seaprint.mimer 1.0
import seaprint.ippmsg 1.0
import seaprint.convertchecker 1.0
import "utils.js" as Utils
import "../components"

Page {
    allowedOrientations: Orientation.All

    id: page
    property alias printer: settingsColumn.printer
    property alias jobParams: settingsColumn.jobParams
    property alias selectedFile: settingsColumn.selectedFile
    property alias selectedFileType: settingsColumn.selectedFileType

    Connections {
        target: wifi
        onConnectedChanged: {
            if(!wifi.connected) {
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

            MenuLabel {
                text:  qsTr("Default settings for %1 on this printer").arg(db.simplifyType(selectedFileType).translatable)
                visible: printer.attrs.hasOwnProperty("printer-uuid")
            }

            MenuItem {
                text: qsTr("Clear default settings")
                visible: printer.attrs.hasOwnProperty("printer-uuid")
                onClicked: {
                    db.removeJobSettings(printer.attrs["printer-uuid"].value, selectedFileType);
                    pageStack.pop();
                }
            }

            MenuItem {
                text: qsTr("Save default settings")
                visible: printer.attrs.hasOwnProperty("printer-uuid")
                onClicked: {
                    var tmp = jobParams;
                    // Varies between documents, would be confusing to save
                    tmp["page-ranges"] = undefined;
                    db.setJobSettings(printer.attrs["printer-uuid"].value, selectedFileType, JSON.stringify(tmp))
                }
            }

            MenuItem {
                text: qsTr("Print")
                onClicked: {
                    console.log(JSON.stringify(jobParams))
                    pageStack.replace(Qt.resolvedUrl("BusyPage.qml"),{printer:printer},
                                      PageStackAction.Immediate)
                    printer.print(jobParams, page.selectedFile)
                }
            }
        }

        VerticalScrollDecorator {}

        SettingsColumn {
            id: settingsColumn
            width: parent.width

            PageHeader {
                id: pageHeader
                title: printer.attrs["printer-name"].value
                description: Utils.basename(selectedFile)
            }

            ChoiceSetting {
                tag: IppMsg.Keyword
                name: "sides"
                prettyName: qsTr("Sides")
                mime_type: selectedFileType
            }
            ChoiceSetting {
                tag: IppMsg.Keyword
                name: "media"
                prettyName: qsTr("Print media")
                mime_type: selectedFileType
                preferred_choices: printer.attrs.hasOwnProperty("media-ready") ? printer.attrs["media-ready"].value : []
                preferred_choice_suffix: qsTr("(loaded)")
            }
            IntegerSetting {
                tag: IppMsg.Integer
                name: "copies"
                prettyName: qsTr("Copies")
            }
            ChoiceSetting {
                tag: IppMsg.Keyword
                name: "multiple-document-handling"
                prettyName: qsTr("Collated copies")
                mime_type: selectedFileType
            }
            RangeSetting {
                tag: IppMsg.IntegerRange
                name: "page-ranges"
                prettyName: qsTr("Page range")
                valid: (_valid || ConvertChecker.pdf) &&
                       (selectedFileType == "application/pdf" || Mimer.isOffice(selectedFileType))

                property var pdfpages: ConvertChecker.pdfPages(selectedFile)
                high: pdfpages == 0 ? 65535 : pdfpages
            }
            ChoiceSetting {
                tag: IppMsg.Keyword
                name: "print-color-mode"
                prettyName: qsTr("Color mode")
                mime_type: selectedFileType
            }
            ChoiceSetting {
                tag: IppMsg.Enum
                name: "print-quality"
                prettyName: qsTr("Quality")
                mime_type: selectedFileType
            }
            ChoiceSetting {
                tag: IppMsg.Resolution
                name: "printer-resolution"
                prettyName: qsTr("Resolution")
                mime_type: selectedFileType
            }
            ChoiceSetting {
                tag: IppMsg.MimeMediaType
                name: "document-format"
                prettyName: qsTr("Transfer format")
                mime_type: selectedFileType
            }
            ChoiceSetting {
                tag: IppMsg.Keyword
                name: "media-source"
                prettyName: qsTr("Media source")
                mime_type: selectedFileType
            }
            MediaColSetting {
                tag: IppMsg.BeginCollection
                name: "media-col"
                prettyName: qsTr("Zero margins")
            }
        }
    }
}
