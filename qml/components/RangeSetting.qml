import QtQuick 2.0
import Sailfish.Silica 1.0

Setting {

    property int high
    property int choice_low: 1
    property int choice_high: 0


    function update_choice() {
        choice = new Object({low: choice_low, high: choice_high});
    }

    onChoice_highChanged: {
        if(choice_high < choice_low)
        {
            low_slider.value = choice_high > 0 ? choice_high : 1;
        }
        else
        {
            update_choice()
        }
    }
    onChoice_lowChanged: {
        if(choice_low > choice_high)
        {
            high_slider.value = choice_low
        }
        else
        {
            update_choice()
        }
    }

    ValueButton {
        enabled: valid
        anchors.verticalCenter: parent.verticalCenter
        label: prettyName
        value: choice_high==0 ? qsTr("all") : ""+choice_low+" - "+choice_high
        onClicked: parent.clicked()
    }

    property var menu: ContextMenu {
        id: menu
        enabled: valid

        MenuItem {
            Slider
            {
                id: low_slider
                minimumValue: 1
                maximumValue: high
                width: parent.width
                stepSize: 1
                value: choice_low
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
                                                        {value: choice, title: prettyName,
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
                maximumValue: high
                width: parent.width
                stepSize: 1
                value: choice_high
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
                                                        {value: choice, title: prettyName,
                                                         min: 1, max: high});
                            dialog.accepted.connect(function() {
                                choice_high = dialog.value;
                            })
                }
            }

        }

    }


}
