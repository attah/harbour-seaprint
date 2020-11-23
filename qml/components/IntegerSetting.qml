import QtQuick 2.0
import Sailfish.Silica 1.0

Setting {
    property int low
    property int high

    displayValue: choice ? choice : default_choice

    menu: ContextMenu {
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
