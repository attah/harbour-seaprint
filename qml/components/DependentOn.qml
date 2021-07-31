import QtQuick 2.6

Item {
    property alias target: targetConnection.target
    property var values
    property var overlap: []

    Component.onCompleted:
    {
        for(var i in values)
        {
            if(target.actual_choices.indexOf(values[i]) != -1)
            {
                overlap.push(values[i])
            }
        }
        parent.valid = parent.valid && overlap.length != 0
    }

    Connections {
        id: targetConnection
        onChoiceChanged:
        {
            if(parent.choice != undefined && overlap.indexOf(target.choice) == -1)
            {
                parent.reset()
                parent.highlight()
            }
        }
    }
    Connections {
        target: parent
        onChoiceChanged:
        {
            if(parent.choice != undefined && overlap.indexOf(targetConnection.target.choice) == -1)
            {
                targetConnection.target.choice = overlap[0]
                targetConnection.target.highlight()
            }
        }
    }
}
