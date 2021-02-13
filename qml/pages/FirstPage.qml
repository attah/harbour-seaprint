import QtQuick 2.0
import Sailfish.Silica 1.0
import Sailfish.Pickers 1.0
import seaprint.ippdiscovery 1.0
import seaprint.convertchecker 1.0
import seaprint.ippprinter 1.0
import seaprint.mimer 1.0
import "utils.js" as Utils
import "../components"
import Nemo.DBus 2.0

Page {
    id: page
    allowedOrientations: Orientation.All

    property string selectedFile: ""
    property string selectedFileType

    WifiChecker {
        id: wifi
        onConnectedChanged: {
            console.log("conn", connected, ssid)
            if(connected) {
                var favourites = db.getFavourites(ssid);
                console.log(favourites);
                IppDiscovery.favourites = favourites;
            }
            else {
                IppDiscovery.favourites = []
            }

        }

        property bool initialSSIDchange: true

        onSsidChanged: {
            console.log("ssid changed", ssid);
            if(!initialSSIDchange)
            {
                IppDiscovery.reset();
            }
            initialSSIDchange = false;
        }
    }

    Label {
        text: qsTr("Not on WiFi")
        color: Theme.highlightColor
        anchors.centerIn: parent
        visible: !wifi.connected
    }

    property bool initialized: false
    onVisibleChanged: {
        if (visible)
        {
            if(initialized)
                page.refreshed()
            initialized = true
        }
    }

    signal refreshed()

    Component.onCompleted: {
        IppDiscovery.discover();
        if(selectedFile != "")
        {
            var type = Mimer.get_type(selectedFile);
            console.log(type);
            selectedFileType = type;
        }
    }

    property bool nagged: false

    onStatusChanged: {
        if(status==PageStatus.Active && !nagged && nagScreenSetting.value != nagScreenSetting.expectedValue)
        {
            console.log("Can convert from PDF:", ConvertChecker.pdf)
            if(!ConvertChecker.pdf)
            {
                nagged=true
                pageStack.push(Qt.resolvedUrl("NagScreen.qml"))
            }
        }
    }

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent

        // PullDownMenu and PushUpMenu must be declared in SilicaFlickable, SilicaListView or SilicaGridView
        PullDownMenu {
            MenuItem {
                text: qsTr("About SeaPrint")
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
            }
            MenuItem {
                text: qsTr("Settings")
                onClicked: pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
            }
            MenuItem {
                text: qsTr("Add by URL")
                enabled: wifi.connected
                onClicked: {
                    var dialog = pageStack.push(Qt.resolvedUrl("AddPrinterDialog.qml"),
                                                {ssid: wifi.ssid, title: qsTr("URL")});
                        dialog.accepted.connect(function() {
                            console.log("add", wifi.ssid, dialog.value);
                            db.addFavourite(wifi.ssid, dialog.value);
                            IppDiscovery.favourites = db.getFavourites(wifi.ssid);
                    })
                }
            }
            MenuItem {
                text: qsTr("Refresh")
                onClicked: {
                    IppDiscovery.discover();
                    page.refreshed();
                }
            }
        }

        SilicaListView {
            anchors.fill: parent
            id: listView
            model: IppDiscovery
            spacing: Theme.paddingSmall


            header: PageHeader {
                id: pageHeader
                title: "SeaPrint"
                description: qsTr("Available printers")
            }

            delegate: ListItem {
                id: delegate
                contentItem.height: visible ? Math.max(column.implicitHeight, Theme.itemSizeLarge+2*Theme.paddingMedium) : 0

                IppPrinter {
                    id: printer
                    url: model.display
                }

                visible: Object.keys(printer.attrs).length !== 0

                property string name: printer.attrs["printer-name"].value != "" ? printer.attrs["printer-name"].value : qsTr("Unknown")
                property bool canPrint: Utils.supported_formats(printer, ConvertChecker, considerAdditionalFormatsSetting.value).mimetypes.indexOf(selectedFileType) != -1

                Connections {
                    target: printer
                    onAttrsChanged: {
                        if(Object.keys(printer.attrs).length === 0) {
                            delegate.visible = false
                        }
                        else {
                            delegate.visible = true
                        }
                    }
                }


                Connections {
                    target: page
                    onRefreshed: {
                        console.log("onRefreshed")
                        printer.refresh()
                    }
                }

                Timer
                {
                    id: debugCountReset
                    interval: 666
                    repeat: false
                    onTriggered:
                    {
                        debugCount = 0;
                    }
                }

                property int debugCount: 0

                onClicked: {

                    if(++debugCount == 5)
                    {
                        pageStack.push(Qt.resolvedUrl("DebugPage.qml"), {printer: printer})
                        return;
                    }
                    debugCountReset.restart();
                    if(!canPrint)
                        return;
                    if(selectedFile != "")
                    {
                        pageStack.push(Qt.resolvedUrl("PrinterPage.qml"), {printer: printer, selectedFile: selectedFile})
                    }
                    else
                    {
                        notifier.notify(qsTr("No file selected"))
                    }
                }

                Image {
                    id: icon
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.topMargin: Theme.paddingMedium
                    anchors.leftMargin: Theme.paddingMedium

                    height: Theme.itemSizeLarge
                    width: Theme.itemSizeLarge
                    source: printer.attrs["printer-icons"] ? "image://ippdiscovery/"+Utils.selectIcon(printer.attrs["printer-icons"].value)
                                                           : "icon-seaprint-nobg.svg"
                    // Some printers serve their icons over https with invalid certs...
                    onStatusChanged: if (status == Image.Error) source = "icon-seaprint-nobg.svg"

                }

                Image {
                    id: warningIcon
                    source: "image://theme/icon-lock-warning"
                    anchors.bottom: icon.bottom
                    anchors.right: icon.right
                    visible: Utils.isWaringState(printer)
                }

                Column {
                    id: column
                    anchors.left: icon.right
                    anchors.leftMargin: Theme.paddingMedium

                    Label {
                        id: name_label
                        color: canPrint || selectedFile == "" ? Theme.primaryColor : Theme.secondaryColor
                        text: name
                    }

                    Label {
                        id: mm_label
                        color: canPrint || selectedFile == "" ? Theme.primaryColor : Theme.secondaryColor
                        font.pixelSize: Theme.fontSizeExtraSmall
                        text: printer.attrs["printer-make-and-model"].value
                    }

                    Label {
                        id: uri_label
                        color: canPrint || selectedFile == "" ? Theme.highlightColor : Theme.secondaryColor
                        font.pixelSize: Theme.fontSizeTiny
                        text: printer.url
                    }

                    Row {
                        spacing: Theme.paddingMedium
                        Label {
                            id: format_unsupported_label
                            visible:  format_label.text == ""
                            color: "red"
                            font.pixelSize: Theme.fontSizeExtraSmall
                            text: qsTr("No compatible formats supported")
                        }
                        Label {
                            id: format_label
                            color: selectedFile == "" ? Theme.secondaryColor : canPrint ? Theme.primaryColor : "red"
                            font.pixelSize: Theme.fontSizeExtraSmall
                            text: Utils.supported_formats(printer, ConvertChecker, considerAdditionalFormatsSetting.value).supported
                        }
                    }

                }

                RemorseItem {
                    id: removeRemorse
                }

                menu: ContextMenu {
                    MenuItem {
                        text: qsTr("View jobs")
                        onClicked:  pageStack.push(Qt.resolvedUrl("JobsPage.qml"), {printer: printer})
                    }
                    MenuItem {
                        text: qsTr("Printer details")
                        onClicked:  pageStack.push(Qt.resolvedUrl("DetailsPage.qml"), {printer: printer})
                    }
                    MenuItem {
                        text: qsTr("Remove printer")
                        visible: db.isFavourite(wifi.ssid, model.display)
                        onClicked: {
                            removeRemorse.execute(delegate, qsTr("Removing printer"),
                                                  function() {db.removeFavourite(wifi.ssid, model.display);
                                                              IppDiscovery.favourites = db.getFavourites(wifi.ssid)})
                        }
                    }
                }

            }
            footer: Item {
                id: footer
                width: parent.width
                height: fileDock.height
            }
        }
    }
    DockedPanel {
        id: fileDock
        open: true
        height: fileLabel.height+folderButton.height+3*Theme.paddingLarge
        width: parent.width
        dock: Dock.Bottom

        Label {
            id: fileLabel
            width: parent.width-2*Theme.paddingLarge

            anchors {
                top: parent.top
                topMargin: Theme.paddingLarge
                horizontalCenter: parent.horizontalCenter
            }

            horizontalAlignment: contentWidth > width ? Text.AlignRight : Text.AlignHCenter
            truncationMode: TruncationMode.Fade
            text: selectedFile != "" ? selectedFile : qsTr("No file selected")
        }


        Row {
            width: parent.width
            anchors.bottom: parent.bottom
            anchors.bottomMargin: Theme.paddingLarge
            IconButton {
                id: folderButton
                icon.source: "image://theme/icon-m-file-folder"
                width: parent.width/2
                onClicked: pageStack.push(filePickerPage)
            }
            IconButton {
                icon.source: "image://theme/icon-m-file-image"
                width: parent.width/2
                onClicked: pageStack.push(imagePickerPage)
            }

        }
        Component {
            id: filePickerPage
            FilePickerPage {
                allowedOrientations: Orientation.All

                title: qsTr("Choose file")
                showSystemFiles: false
                nameFilters: ["*.pdf", "*.jpg", "*.jpeg", "*.ps"]

                onSelectedContentPropertiesChanged: {
                    page.selectedFile = selectedContentProperties.filePath
                    page.selectedFileType = Mimer.get_type(selectedContentProperties.filePath)
                }
            }
        }
        Component {
            id: imagePickerPage
            ImagePickerPage {
                allowedOrientations: Orientation.All

                onSelectedContentPropertiesChanged: {
                    page.selectedFile = selectedContentProperties.filePath
                    page.selectedFileType = Mimer.get_type(selectedContentProperties.filePath)
                }
            }
        }
    }
}
