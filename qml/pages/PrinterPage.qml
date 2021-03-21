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
    property var printer
    property var jobParams
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

    function choiceMade(setting)
    {
        if(setting.choice != undefined)
        {
            jobParams[setting.name] = {tag: setting.tag, value: setting.choice};
        }
        else
        {
            jobParams[setting.name] = undefined;
        }
        console.log(JSON.stringify(jobParams));
    }

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent
        contentHeight: settingColumn.height

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
                    printer.print(jobParams, page.selectedFile,
                                  alwaysConvertSetting.value,
                                  alwaysUseMediaColSetting.value)
                }
            }
        }

        VerticalScrollDecorator {}

        Column {
            id: settingColumn
            width: parent.width

            PageHeader {
                id: pageHeader
                title: printer.attrs["printer-name"].value
                description: Utils.basename(selectedFile)
            }

            Item {
                id: utils

                function isValid(name) {
                    return printer.attrs.hasOwnProperty(name+"-supported");
                }
                function setInitialChoice(setting) {
                    if(jobParams.hasOwnProperty(setting.name))
                    {
                        if(setting.valid)
                        {
                            setting.choice = jobParams[setting.name].value;
                        }
                        else
                        { // Clear jobParams of invalid settings
                            jobParams[setting.name] = undefined;
                        }
                    }
                }
                function getChoices(name) {
                    return isValid(name) ? printer.attrs[name+"-supported"].value : [];
                }
                function getDefaultChoice(name) {
                    return printer.attrs.hasOwnProperty(name+"-default") ? printer.attrs[name+"-default"].value : undefined;
                }
            }

            ChoiceSetting {
                tag: IppMsg.Keyword
                name: "sides"
                prettyName: qsTr("Sides")
                valid: utils.isValid(name)
                choices: utils.getChoices(name)
                default_choice: utils.getDefaultChoice(name)
                mime_type: selectedFileType

                onChoiceChanged: page.choiceMade(this)
                Component.onCompleted: utils.setInitialChoice(this)
            }
            ChoiceSetting {
                tag: IppMsg.Keyword
                name: "media"
                prettyName: qsTr("Print media")
                valid: utils.isValid(name)
                choices: utils.getChoices(name)
                default_choice: utils.getDefaultChoice(name)
                mime_type: selectedFileType

                onChoiceChanged: page.choiceMade(this)
                Component.onCompleted: utils.setInitialChoice(this)
            }
            IntegerSetting {
                tag: IppMsg.Integer
                name: "copies"
                prettyName: qsTr("Copies")
                valid: utils.isValid(name)
                low: valid ? printer.attrs[name+"-supported"].value.low : 0
                high: valid ? printer.attrs[name+"-supported"].value.high : 0
                default_choice: utils.getDefaultChoice(name)

                onChoiceChanged: page.choiceMade(this)
                Component.onCompleted: utils.setInitialChoice(this)
            }
            ChoiceSetting {
                tag: IppMsg.Keyword
                name: "multiple-document-handling"
                prettyName: qsTr("Collated copies")
                valid: utils.isValid(name)
                choices: utils.getChoices(name)
                default_choice: utils.getDefaultChoice(name)
                mime_type: selectedFileType

                onChoiceChanged: page.choiceMade(this)
                Component.onCompleted: utils.setInitialChoice(this)
            }
            RangeSetting {
                tag: IppMsg.IntegerRange
                name: "page-ranges"
                prettyName: qsTr("Page range")
                valid: (utils.isValid(name) || ConvertChecker.pdf) &&
                       (selectedFileType == "application/pdf" || Mimer.isOffice(selectedFileType))

                property var pdfpages: ConvertChecker.pdfPages(selectedFile)
                high: pdfpages == 0 ? 65535 : pdfpages

                onChoiceChanged: page.choiceMade(this)
                Component.onCompleted: utils.setInitialChoice(this)
            }
            ChoiceSetting {
                tag: IppMsg.Keyword
                name: "print-color-mode"
                prettyName: qsTr("Color mode")
                valid: utils.isValid(name)
                choices: utils.getChoices(name)
                default_choice: utils.getDefaultChoice(name)
                mime_type: selectedFileType

                onChoiceChanged: page.choiceMade(this)
                Component.onCompleted: utils.setInitialChoice(this)
            }
            ChoiceSetting {
                tag: IppMsg.Enum
                name: "print-quality"
                prettyName: qsTr("Quality")
                valid: utils.isValid(name)
                choices: utils.getChoices(name)
                default_choice: utils.getDefaultChoice(name)
                mime_type: selectedFileType

                onChoiceChanged: page.choiceMade(this)
                Component.onCompleted: utils.setInitialChoice(this)
            }
            ChoiceSetting {
                tag: IppMsg.Resolution
                name: "printer-resolution"
                prettyName: qsTr("Resolution")
                valid: utils.isValid(name)
                choices: utils.getChoices(name)
                default_choice: utils.getDefaultChoice(name)
                mime_type: selectedFileType

                onChoiceChanged: page.choiceMade(this)
                Component.onCompleted: utils.setInitialChoice(this)
            }
            ChoiceSetting {
                tag: IppMsg.MimeMediaType
                name: "document-format"
                prettyName: qsTr("Transfer format")
                valid: utils.isValid(name)
                choices: utils.getChoices(name)
                default_choice: utils.getDefaultChoice(name)
                mime_type: selectedFileType

                onChoiceChanged: page.choiceMade(this)
                Component.onCompleted: utils.setInitialChoice(this)
            }
            ChoiceSetting {
                tag: IppMsg.Keyword
                name: "media-source"
                prettyName: qsTr("Media source")
                valid: utils.isValid(name)
                choices: utils.getChoices(name)
                default_choice: utils.getDefaultChoice(name)
                mime_type: selectedFileType

                onChoiceChanged: page.choiceMade(this)
                Component.onCompleted: utils.setInitialChoice(this)
            }
            MediaColSetting {
                tag: IppMsg.BeginCollection
                name: "media-col"
                prettyName: qsTr("Zero margins")
                valid: false
                printer: page.printer

                onChoiceChanged: page.choiceMade(this)
                Component.onCompleted: utils.setInitialChoice(this)
            }
        }
    }
}
