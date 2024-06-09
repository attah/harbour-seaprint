import QtQuick 2.6
import Sailfish.Silica 1.0

Setting {
    property int high
    property int choice_low: 1
    property int choice_high: 0
    property bool acceptRangeList: false

    property bool suppressChange: false

    function update_choice() {
        if (choice_high == 0)
        {
            setting.value = undefined;
        }
        else
        {
            setting.value = [({low: choice_low, high: choice_high})];
        }
    }

    onChoice_highChanged: {
        if(!suppressChange)
        {
            if(choice_high < choice_low)
            {
                low_slider.value = choice_high > 0 ? choice_high : 1;
            }
            update_choice()
        }
    }
    onChoice_lowChanged: {
        if(!suppressChange)
        {
            if(choice_low > choice_high)
            {
                high_slider.value = choice_low
            }
            update_choice()
        }
    }

    displayValue: setting.pretty != "" ? setting.pretty : qsTr("all")

    menu: ContextMenu {
        MenuItem {
            Slider
            {
                id: low_slider
                minimumValue: 1
                maximumValue: high > 50 ? 50 : high
                width: parent.width
                stepSize: 1
                onValueChanged:
                {
                    choice_low = value;
                }
            }
            IconButton
            {
                anchors.right: parent.right
                icon.source: "image://theme/icon-s-edit"
                onClicked: {var dialog = pageStack.push(Qt.resolvedUrl("IntegerInputDialog.qml"),
                                                        {title: prettyName + " (" + qsTr("Low") + ")",
                                                         min: 1, max: high});
                            dialog.accepted.connect(function() {
                                choice_low = dialog.value;
                            })
                }
            }

        }

        MenuItem {
            Slider
            {
                id: high_slider
                minimumValue: 0
                maximumValue: high > 50 ? 50 : high
                width: parent.width
                stepSize: 1
                onValueChanged:
                {
                    choice_high = value;
                }
            }
            IconButton
            {
                anchors.right: parent.right
                icon.source: "image://theme/icon-s-edit"
                onClicked: {var dialog = pageStack.push(Qt.resolvedUrl("IntegerInputDialog.qml"),
                                                        {title: prettyName + " (" + qsTr("High") + ")",
                                                         min: 1, max: high});
                            dialog.accepted.connect(function() {
                                choice_high = dialog.value;
                            })
                }
            }

        }

        MenuItem {
            visible: acceptRangeList
            text: qsTr("Advanced")
            onClicked: {var dialog = pageStack.push(Qt.resolvedUrl("RangeListInputDialog.qml"),
                                                    {title: prettyName});
                        dialog.accepted.connect(function() {
                            setting.value = dialog.value;
                        })
            }
        }

    }


}
