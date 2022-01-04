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

    Connections {
        target: printer
        onAttrsChanged: {
            if(Object.keys(printer.attrs).length === 0) {
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
                text: qsTr("Reset default settings")
                visible: printer.attrs.hasOwnProperty("printer-uuid")
                onClicked: {
                    db.removeJobSettings(printer.attrs["printer-uuid"].value, selectedFileType);
                    pageStack.pop();
                }
            }

            MenuItem {
                text: qsTr("Save as default settings")
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
                title: Utils.unknownForEmptyString(printer.attrs["printer-name"].value)
                description: Utils.basename(selectedFile)
            }

            ChoiceSetting {
                tag: IppMsg.Keyword
                name: "sides"
                prettyName: qsTr("Sides")
            }
            ChoiceSetting {
                tag: IppMsg.Keyword
                name: "media"
                prettyName: qsTr("Print media")
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
            }
            RangeSetting {
                tag: IppMsg.IntegerRange
                name: "page-ranges"
                prettyName: qsTr("Page range")
                valid: (selectedFileType == Mimer.PDF || Mimer.isOffice(selectedFileType) || (_valid && selectedFileType == Mimer.Postscript))

                property var pdfpages: ConvertChecker.pdfPages(selectedFile)
                high: pdfpages == 0 ? 65535 : pdfpages
            }
            ChoiceSetting {
                tag: IppMsg.Integer
                name: "number-up"
                prettyName: qsTr("Pages per page")
                valid: _valid && !Mimer.isImage(selectedFileType)
                DependentOn {
                    target: transferFormatSetting
                    values: [Mimer.PDF, Mimer.Postscript]
                }
            }
            ChoiceSetting {
                tag: IppMsg.Keyword
                name: "print-color-mode"
                prettyName: qsTr("Color mode")
            }
            ChoiceSetting {
                tag: IppMsg.Enum
                name: "print-quality"
                prettyName: qsTr("Quality")
            }
            ChoiceSetting {
                tag: IppMsg.Resolution
                name: "printer-resolution"
                prettyName: qsTr("Resolution")
            }
            ChoiceSetting {
                tag: IppMsg.Keyword
                name: "print-scaling"
                prettyName: qsTr("Scaling")
                valid: _valid && selectedFileType == Mimer.JPEG
                DependentOn {
                    target: transferFormatSetting
                    values: [Mimer.JPEG]
                }
            }
            ChoiceSetting {
                id: transferFormatSetting
                tag: IppMsg.MimeMediaType
                name: "document-format"
                prettyName: qsTr("Transfer format")
            }

            BarButton {
                id: mediaButton
                text: qsTr("Media handling")
            }
            ChoiceSetting {
                visible:  mediaButton.active
                tag: IppMsg.Keyword
                name: "media-type"
                subkey: "media-col"
                prettyName: qsTr("Media type")
            }
            ChoiceSetting {
                visible:  mediaButton.active
                tag: IppMsg.Keyword
                name: "media-source"
                subkey: "media-col"
                prettyName: qsTr("Media source")
            }
            ChoiceSetting {
                visible:  mediaButton.active
                tag: IppMsg.Keyword
                name: "output-bin"
                prettyName: qsTr("Output bin")
            }

            BarButton {
                id: marginsButton
                text: qsTr("Margins")
            }
            ChoiceSetting {
                visible: marginsButton.active
                tag: IppMsg.Integer
                name: "media-top-margin"
                subkey: "media-col"
                prettyName: qsTr("Top")
            }
            ChoiceSetting {
                visible: marginsButton.active
                tag: IppMsg.Integer
                name: "media-bottom-margin"
                subkey: "media-col"
                prettyName: qsTr("Bottom")
            }
            ChoiceSetting {
                visible: marginsButton.active
                tag: IppMsg.Integer
                name: "media-left-margin"
                subkey: "media-col"
                prettyName: qsTr("Left")
            }
            ChoiceSetting {
                visible: marginsButton.active
                tag: IppMsg.Integer
                name: "media-right-margin"
                subkey: "media-col"
                prettyName: qsTr("Right")
            }
        }
    }
}
