import QtQuick 2.6
import Sailfish.Silica 1.0

Dialog {
    id: dialog
    allowedOrientations: Orientation.All

    property int value
    property int min;
    property int max;
    property string title
    canAccept: valueField.acceptableInput

    Column {
        width: parent.width

        DialogHeader { }

        TextField {
            id: valueField
            validator: IntValidator{bottom: min; top: max;}
            width: parent.width
            placeholderText: ""+min+"-"+max
            label: title
            focus: true
            labelVisible: true
            inputMethodHints: Qt.ImhDigitsOnly

        }
    }

    onDone: {
        if (result == DialogResult.Accepted) {
            value = valueField.text
        }
    }

}
