import QtQuick 2.0
import Sailfish.Silica 1.0

Setting {
    id: settingEntry
    height: button.height +  menu.height

    property int low
    property int high

    ValueButton {
        id: button
        enabled: valid
        label: prettyName
        value: choice ? choice : default_choice
        onClicked: parent.clicked()
        Component.onCompleted: {
            console.log("button height: " + button.height)
        }
    }

    onClicked: menu.open(settingEntry)

    ContextMenu {
        id: menu
        enabled: valid
        MenuItem {
            Slider
            {
                minimumValue: low
                maximumValue: high < 100 ? high : 100
                width: parent.width
                stepSize: 1
                value: choice ? choice : default_choice
                onValueChanged:
                {
                    choice = value;
                }
            }
            IconButton
            {
                anchors.right: parent.right
                icon.source: "image://theme/icon-s-edit"
                onClicked: {var dialog = pageStack.push(Qt.resolvedUrl("IntegerInputDialog.qml"),
                                                        {value: choice, title: prettyName,
                                                         min: low, max: high});
                            dialog.accepted.connect(function() {
                                choice = dialog.value;
                            })
                }
            }

        }

    }


}
