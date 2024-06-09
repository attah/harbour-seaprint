import QtQuick 2.6
import Sailfish.Silica 1.0

Setting {
    property int minimum_high: 0
    property int low: _valid ? setting.low : 0
    property int high: _valid ? ensure_minimum(setting.high) : 0

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

    displayValue: setting.value

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
                    setting.value = value;
                }
            }
            IconButton
            {
                anchors.right: parent.right
                icon.source: "image://theme/icon-s-edit"
                onClicked: {var dialog = pageStack.push(Qt.resolvedUrl("IntegerInputDialog.qml"),
                                                        {title: prettyName,
                                                         min: low, max: high});
                            dialog.accepted.connect(function() {
                                setting.value = dialog.value;
                            })
                }
            }

        }

    }


}
