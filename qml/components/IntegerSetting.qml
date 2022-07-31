import QtQuick 2.6
import Sailfish.Silica 1.0

Setting {
    property int minimum_high: 0
    property int low: _valid ? parent.printer.attrs[name+"-supported"].value.low : (minimum_high != 0) ? 1 : 0
    property int high: _valid ? ensure_minimum(parent.printer.attrs[name+"-supported"].value.high) : minimum_high

    function ensure_minimum(orig)
    {
        if(orig < minimum_high)
        {
            return minimum_high;
        }
        else
        {
            return orig;
        }
    }

    property bool suppressChange: false

    displayValue: choice != undefined ? choice : default_choice

    onChoiceChanged: {
        if(choice == undefined)
        {
            suppressChange = true;
            slider.value = slider.minimumValue;
            suppressChange = false;
        }
    }

    menu: ContextMenu {
        MenuItem {
            Slider
            {
                id: slider
                minimumValue: low
                maximumValue: high < 50 ? high : 50
                width: parent.width
                stepSize: 1
                onValueChanged:
                {
                    if(!suppressChange)
                    {
                        choice = value;
                    }
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
