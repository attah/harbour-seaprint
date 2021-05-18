import QtQuick 2.6
import Sailfish.Silica 1.0
import Sailfish.Pickers 1.0
import seaprint.ippdiscovery 1.0
import seaprint.convertchecker 1.0
import seaprint.ippprinter 1.0
import seaprint.mimer 1.0
import "utils.js" as Utils

Page {
    id: page
    allowedOrientations: Orientation.All

    property string selectedFile: ""
    property string selectedFileType

    Connections {
        target: wifi
        onConnectedChanged: {
            console.log("conn", wifi.connected, wifi.ssid)
            if(wifi.connected) {
                var favourites = db.getFavourites(wifi.ssid);
                console.log(favourites);
                IppDiscovery.favourites = favourites;
            }
            else {
                IppDiscovery.favourites = []
            }

        }

        property bool initialSSIDchange: true

        onSsidChanged: {
            console.log("ssid changed", wifi.ssid);
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
            console.log("Can convert from Office:", ConvertChecker.calligra)
            if(!ConvertChecker.pdf || (expectCalligra && !ConvertChecker.calligra))
            {
                nagged=true
                pageStack.push(Qt.resolvedUrl("NagScreen.qml"))
            }
        }
    }

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.top: parent.top
        width: parent.width
        anchors.bottom: fileDock.top
        clip: true

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
                    var dialog = pageStack.push(Qt.resolvedUrl("AddPrinterDialog.qml"));
                        dialog.accepted.connect(function() {
                            console.log("add", wifi.ssid, dialog.value);
                            db.addFavourite(wifi.ssid, dialog.value);
                            IppDiscovery.favourites = db.getFavourites(dialog.ssid);
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
            id: listView
            anchors.fill: parent
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
                property var supported_formats: Utils.supported_formats(printer, ConvertChecker, considerAdditionalFormatsSetting.value)
                property bool canPrint: supported_formats.mimetypes.indexOf(selectedFileType) != -1

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

                function maybeGetParams()
                {
                    if(printer.attrs.hasOwnProperty("printer-uuid"))
                    {
                        return JSON.parse(db.getJobSettings(printer.attrs["printer-uuid"].value, selectedFileType));
                    }
                    else
                    {
                        return new Object();
                    }
                }


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
                        pageStack.push(Qt.resolvedUrl("PrinterPage.qml"), {printer: printer, selectedFile: selectedFile, jobParams: maybeGetParams()})
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
                    sourceSize.height: height
                    sourceSize.width: width
                    source: printer.attrs["printer-icons"] ? "image://ippdiscovery/"+Utils.selectIcon(printer.attrs["printer-icons"].value)
                                                           : "image://svg/qml/pages/icon-seaprint-nobg.svg"
                    // Some printers serve their icons over https with invalid certs...
                    onStatusChanged: if (status == Image.Error) source = "image://svg/qml/pages/icon-seaprint-nobg.svg"

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
                        text: Utils.unknownForEmptyString(printer.attrs["printer-make-and-model"].value)
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
                            visible: !supported_formats.pdf && !supported_formats.postscript && !supported_formats.office && !supported_formats.images
                            color: "red"
                            font.pixelSize: Theme.fontSizeExtraSmall
                            text: qsTr("No compatible formats supported")
                        }

                        HighlightImage {
                            height: Theme.itemSizeExtraSmall/2
                            width: Theme.itemSizeExtraSmall/2
                            visible: supported_formats.pdf
                            highlightColor: "red"
                            highlighted: !(selectedFile == "" || canPrint)
                            source: "image://theme/icon-m-file-pdf"
                        }
                        HighlightImage {
                            height: Theme.itemSizeExtraSmall/2
                            width: Theme.itemSizeExtraSmall/2
                            visible: supported_formats.postscript
                            highlightColor: "red"
                            highlighted: !(selectedFile == "" || canPrint)
                            source: "image://theme/icon-m-file-other"

                        }
                        HighlightImage {
                            height: Theme.itemSizeExtraSmall/2
                            width: Theme.itemSizeExtraSmall/2
                            visible: supported_formats.office
                            highlightColor: "red"
                            highlighted: !(selectedFile == "" || canPrint)
                            source: "image://theme/icon-m-file-formatted"
                        }
                        HighlightImage {
                            height: Theme.itemSizeExtraSmall/2
                            width: Theme.itemSizeExtraSmall/2
                            visible: supported_formats.images
                            highlightColor: "red"
                            highlighted: !(selectedFile == "" || canPrint)
                            source: "image://theme/icon-m-file-image"
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
        }
    }
    DockedPanel {
        id: fileDock
        open: true
        height: panelColumn.implicitHeight
        width: parent.width
        dock: Dock.Bottom

        Column {
            id: panelColumn
            width: parent.width
            spacing: Theme.paddingLarge
            topPadding: page.isPortrait && !warningRow.visible ? 2*Theme.paddingLarge : Theme.paddingLarge
            bottomPadding: page.isPortrait ? 2*Theme.paddingLarge : Theme.paddingLarge

            Row {
                id: warningRow
                anchors.horizontalCenter: parent.horizontalCenter
                visible: Mimer.isOffice(page.selectedFileType)

                HighlightImage {
                    source: "image://theme/icon-s-warning"
                    highlighted: true
                    highlightColor: Theme.highlightColor
                }
                Label
                {
                    text: qsTr("This format may not render correctly")
                    font.pixelSize: Theme.fontSizeExtraSmall
                    color: Theme.highlightColor
                }
            }

            Label {
                id: fileLabel
                width: parent.width-2*Theme.paddingLarge

                horizontalAlignment: contentWidth > width ? Text.AlignRight : Text.AlignHCenter
                truncationMode: TruncationMode.Fade
                text: selectedFile != "" ? selectedFile : qsTr("No file selected")
            }


            Row {
                width: parent.width
                IconButton {
                    id: folderButton
                    icon.source: "image://theme/icon-m-file-document"
                    width: parent.width/2
                    onClicked: pageStack.push(documentPickerPage)
                }
                IconButton {
                    id: imageButton
                    icon.source: "image://theme/icon-m-file-image"
                    width: parent.width/2
                    onClicked: pageStack.push(imagePickerPage)
                }

            }

        }
        Component {
            id: documentPickerPage
            DocumentPickerPage {
                allowedOrientations: Orientation.All

                Component.onCompleted: {
                    var thingy = Qt.createComponent(ConvertChecker.calligra ? "../components/DocumentFilterOffice.notqml" : "../components/DocumentFilter.notqml");
                    if (thingy.status == Component.Ready) {
                        _contentModel.contentFilter = thingy.createObject(this);
                    }
                }

                title: qsTr("Choose file")

                onSelectedContentPropertiesChanged: {
                    var mimeType = Mimer.get_type(selectedContentProperties.filePath)
                    if(mimeType == "application/pdf" || mimeType == "application/postscript" || Mimer.isOffice(mimeType))
                    {
                        page.selectedFile = selectedContentProperties.filePath
                        page.selectedFileType = mimeType
                    }
                    else
                    {
                        console.log("UNSUPPORTED", mimeType);
                        notifier.notify(qsTr("Unsupported document format"))
                        page.selectedFile = ""
                        page.selectedFileType = ""
                    }
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
