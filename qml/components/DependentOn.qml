import QtQuick 2.6

Item {
    id: dependentOn
    property var target
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
        target: dependentOn.target.setting
        onChanged:
        {
            if(parent.setting.isSet && overlap.indexOf(target.value) == -1)
            {
                parent.reset()
                parent.highlight()
            }
        }
    }
    Connections {
        target: parent.setting
        onChanged:
        {
            if(parent.setting.value != undefined && overlap.indexOf(targetConnection.target.value) == -1)
            {
                targetConnection.target.value = overlap[0]
                targetConnection.target.highlight()
            }
        }
    }
}
