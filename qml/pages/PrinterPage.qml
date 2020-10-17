import QtQuick 2.0
import Sailfish.Silica 1.0
import seaprint.mimer 1.0
import seaprint.ippmsg 1.0
import seaprint.convertchecker 1.0
import "utils.js" as Utils

Page {
    allowedOrientations: Orientation.All

    id: page
    property var printer
    property var jobParams: new Object();
    property string selectedFile

    Component.onCompleted: {
        console.log(JSON.stringify(printer.attrs))
    }

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
            anchors.fill: parent

            // PullDownMenu and PushUpMenu must be declared in SilicaFlickable, SilicaListView or SilicaGridView
            PullDownMenu {
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

        ListModel {
            id:mod
            ListElement {name: "sides";                       prettyName: qsTr("Sides");              tag: IppMsg.Keyword}
            ListElement {name: "media";                       prettyName: qsTr("Print media");        tag: IppMsg.Keyword}
            ListElement {name: "copies";                      prettyName: qsTr("Copies");             tag: IppMsg.Integer}
            ListElement {name: "multiple-document-handling";  prettyName: qsTr("Collated copies");    tag: IppMsg.Keyword}
            ListElement {name: "page-ranges";                 prettyName: qsTr("Page range");         tag: IppMsg.IntegerRange}
            ListElement {name: "print-color-mode";            prettyName: qsTr("Color mode");         tag: IppMsg.Keyword}
//            ListElement {name: "orientation-requested";     prettyName: qsTr("Orientation");        tag: IppMsg.Enum}
            ListElement {name: "print-quality";               prettyName: qsTr("Quality");            tag: IppMsg.Enum}
            ListElement {name: "printer-resolution";          prettyName: qsTr("Resolution");         tag: IppMsg.Resolution}
            ListElement {name: "document-format";             prettyName: qsTr("Transfer format");    tag: IppMsg.MimeMediaType}
            ListElement {name: "media-source";                prettyName: qsTr("Media source");       tag: IppMsg.Keyword}
            ListElement {name: "media-col";                   prettyName: qsTr("Zero margins");       tag: IppMsg.BeginCollection}
        }

        SilicaListView {
            id: listView
            model: mod
            clip: true

            anchors.fill: parent
            width: parent.width
            header: PageHeader {
                id: pageHeader
                title: printer.attrs["printer-name"].value
                description: selectedFile

            }



            delegate: ListItem {
                id: delegate
                property alias loaderItem: loader.item

                openMenuOnPressAndHold: false

                Loader {
                    id: loader
                    anchors.fill: parent

                    onLoaded: {
                        delegate.visible = true
                        if(loaderItem.menu.enabled)
                        {
                            menu = loaderItem.menu
                            loaderItem.clicked.connect(openMenu)
                        }
                        loaderItem.choiceMade.connect(function(tag, choice) {
                            console.log("choice changed", tag, JSON.stringify(choice))
                            if(choice != undefined)
                            {
                                jobParams[name] = {tag: tag, value: choice};
                            }
                            else
                            {
                                jobParams[name] = undefined
                            }
                            console.log(JSON.stringify(jobParams));
                        })
                    }
                }

                Component.onCompleted: {
                    switch(tag) {
                    case IppMsg.Integer:
                        var valid = printer.attrs.hasOwnProperty(name+"-supported")
                        loader.setSource("../components/IntegerSetting.qml",
                                         {name: name,
                                          prettyName: prettyName,
                                          tag: tag,
                                          valid: valid,
                                          low: valid ? printer.attrs[name+"-supported"].value.low : 0,
                                          high: valid ? printer.attrs[name+"-supported"].value.high : 0,
                                          default_choice: printer.attrs.hasOwnProperty(name+"-default") ? printer.attrs[name+"-default"].value : undefined
                                         })
                        break
                    case IppMsg.IntegerRange:
                        var valid = (printer.attrs.hasOwnProperty(name+"-supported") || ConvertChecker.pdf)&&
                                    name=="page-ranges" && Mimer.get_type(selectedFile) == "application/pdf";
                        var pdfpages = ConvertChecker.pdfPages(selectedFile);
                        loader.setSource("../components/RangeSetting.qml",
                                         {name: name,
                                          prettyName: prettyName,
                                          tag: tag,
                                          valid: valid,
                                          high: name=="page-ranges" ? (pdfpages == 0 ? 65535 : pdfpages)  : 0
                                         })
                        break
                    case IppMsg.Resolution:
                    case IppMsg.Enum:
                    case IppMsg.Keyword:
                    case IppMsg.MimeMediaType:
                        var valid = printer.attrs.hasOwnProperty(name+"-supported")
                        loader.setSource("../components/ChoiceSetting.qml",
                                         {name: name,
                                          prettyName: prettyName,
                                          tag: tag,
                                          valid: valid,
                                          choices: valid ? printer.attrs[name+"-supported"].value : [],
                                          default_choice: printer.attrs.hasOwnProperty(name+"-default") ? printer.attrs[name+"-default"].value : "",
                                          mime_type: Mimer.get_type(selectedFile)
                                         })
                        break
                    case IppMsg.BeginCollection:
                        loader.setSource("../components/MediaColSetting.qml",
                                         {name: name,
                                          prettyName: prettyName,
                                          tag: tag,
                                          valid: false,
                                          printer: printer
                                         })
                    }
                }

            }
            VerticalScrollDecorator {}
        }

    }

}
