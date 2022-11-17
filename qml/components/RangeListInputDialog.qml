import QtQuick 2.6
import Sailfish.Silica 1.0
import seaprint.rangelistchecker 1.0

Dialog {
    id: dialog
    allowedOrientations: Orientation.All

    property string title
    property var value
    canAccept: valueField.acceptableInput

    Column {
        width: parent.width

        DialogHeader { }

        TextField {
            id: valueField
            acceptableInput: Object.keys(value).length !== 0
            width: parent.width
            placeholderText: "1,2,17-42"
            label: title
            focus: true
            labelVisible: true
            onTextChanged: { dialog.value = RangeListChecker.parse(text) }
        }
    }

}
