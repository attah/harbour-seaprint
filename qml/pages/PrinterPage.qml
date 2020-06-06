import QtQuick 2.0
import Sailfish.Silica 1.0
import seaprint.mimer 1.0
import "utils.js" as Utils
import Nemo.Configuration 1.0

Page {
    id: page
    property var printer
    property var jobParams: new Object();
    property string selectedFile

    ConfigurationValue
    {
        id: alwaysConvert
        key: "/apps/harbour-seaprint/settings/always_convert"
        defaultValue: false
    }

    ConfigurationValue
    {
        id: forceIncluDeDocumentFormat
        key: "/apps/harbour-seaprint/settings/force_include_document_format"
        defaultValue: false
    }

    ConfigurationValue
    {
        id: removeDuplexAttributesForRaster
        key: "/apps/harbour-seaprint/settings/remove_duplex_attribute_for_raster"
        defaultValue: false
    }

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
                                      alwaysConvert.value, forceIncluDeDocumentFormat.value, removeDuplexAttributesForRaster.value)
                    }
                }
            }

        ListModel {
            id:mod
            ListElement {name: "sides";                     prettyName: qsTr("Sides");              tag: 0x23}
            ListElement {name: "media";                     prettyName: qsTr("Print media");        tag: 0x44}
            ListElement {name: "copies";                    prettyName: qsTr("Copies");             tag: 0x21}
//            ListElement {name: "page-ranges";             prettyName: qsTr("Page range");         tag: 0x33}
            ListElement {name: "print-color-mode";          prettyName: qsTr("Color mode");         tag: 0x23}
//            ListElement {name: "orientation-requested";   prettyName: qsTr("Orientation");        tag: 0x23}
            ListElement {name: "print-quality";             prettyName: qsTr("Quality");            tag: 0x23}
            ListElement {name: "printer-resolution";        prettyName: qsTr("Resolution");         tag: 0x32}
            ListElement {name: "document-format";           prettyName: qsTr("Transfer format");    tag: 0x49}
            ListElement {name: "media-source";              prettyName: qsTr("Media source");       tag: 0x44}
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
                }

                Component.onCompleted: {
                    switch(tag) {
                    case 0x21:
                        loader.setSource("../components/IntegerSetting.qml",
                                         {name: name,
                                          prettyName: prettyName,
                                          tag: tag,
                                          valid: printer.attrs.hasOwnProperty(name+"-supported"),
                                          low: printer.attrs[name+"-supported"].value.low,
                                          high: printer.attrs[name+"-supported"].value.high,
                                          default_choice: printer.attrs[name+"-default"].value
                                         })
                        break
                    case 0x33:
                        loader.setSource("../components/RangeSetting.qml",
                                         {name: name,
                                          prettyName: prettyName,
                                          tag: tag,
                                          valid: false //TODO printer.attrs.hasOwnProperty(name+"-supported"),
                                         })
                        break
                    case 0x32:
                    case 0x23:
                    case 0x44:
                    case 0x49:
                        loader.setSource("../components/ChoiceSetting.qml",
                                         {name: name,
                                          prettyName: prettyName,
                                          tag: tag,
                                          valid: printer.attrs.hasOwnProperty(name+"-supported"),
                                          choices: name == "document-format" ? printer.attrs[name+"-supported"].value.concat(printer.additionalDocumentFormats)
                                                                             : printer.attrs[name+"-supported"].value,
                                          default_choice: printer.attrs[name+"-default"].value,
                                          mime_type: Mimer.get_type(selectedFile)
                                         })
                        break
                    }
                }

                onLoaderItemChanged: {
                    menu = loaderItem.menu
                    loaderItem.clicked.connect(function() {
                        openMenu()
                    })
                    loaderItem.choiceMade.connect(function(tag, choice) {
                        console.log("choice changed", tag, JSON.stringify(choice))
                        jobParams[name] = {tag: tag, value: choice};
                        console.log(JSON.stringify(jobParams));
                    })
                }

            }
            VerticalScrollDecorator {}
        }

    }

}
