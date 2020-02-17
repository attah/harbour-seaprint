import QtQuick 2.0
import Sailfish.Silica 1.0
import "utils.js" as Utils

Page {
    id: page
    property var printer
    property var jobParams: new Object();
    property string selectedFile

    Component.onCompleted: {
        console.log(JSON.stringify(printer.attrs))
    }

    Connections {
        target: printer
        onJobAttrsFinished: {
            var msg = printer.jobAttrs["job-state-message"] && printer.jobAttrs["job-state-message"].value != ""
                    ? printer.jobAttrs["job-state-message"].value : Utils.ippName("job-state", printer.jobAttrs["job-state"].value)
            if(status == true) {
                notifier.notify(qsTr("Print success: ") + msg)
                pageStack.pop() //or replace?
            }
            else {
                notifier.notify(qsTr("Print failed: ") + msg)
            }
        }
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
                        printer.print(jobParams, page.selectedFile)

                    }
                }
            }

        ListModel {
            id:mod
            ListElement {name: "sides";                   prettyName: qsTr("Sides");       tag: 0x23}
            ListElement {name: "copies";                  prettyName: qsTr("Copies");      tag: 0x21}
//            ListElement {name: "page-ranges";             prettyName: qsTr("Page range");  tag: 0x33}
            ListElement {name: "print-color-mode";        prettyName: qsTr("Color mode");  tag: 0x23}
//            ListElement {name: "orientation-requested";   prettyName: qsTr("Orientation"); tag: 0x23}
            ListElement {name: "print-quality";           prettyName: qsTr("Quality");     tag: 0x23}
            ListElement {name: "printer-resolution";      prettyName: qsTr("Resolution");  tag: 0x32}
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
                    console.log("handling", tag, name, prettyName, JSON.stringify(printer.attrs), JSON.stringify(printer.attrs[name+"-supported"]), JSON.stringify(printer.attrs[name+"-default"]))
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
                        loader.setSource("../components/ChoiceSetting.qml",
                                         {name: name,
                                          prettyName: prettyName,
                                          tag: tag,
                                          valid: printer.attrs.hasOwnProperty(name+"-supported"),
                                          choices: printer.attrs[name+"-supported"].value,
                                          default_choice: printer.attrs[name+"-default"].value
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
