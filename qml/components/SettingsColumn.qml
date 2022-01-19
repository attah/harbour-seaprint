import QtQuick 2.6
import Sailfish.Silica 1.0
import seaprint.mimer 1.0
import seaprint.ippprinter 1.0
import seaprint.ippmsg 1.0

Column {
    id: settingsColumn

    property var printer
    property var jobParams
    property string selectedFile
    property string selectedFileType: Mimer.get_type(selectedFile)

    function isValid(name) {
        return printer.attrs.hasOwnProperty(name+"-supported");
    }
    function setInitialChoice(setting) {
        if(setting.valid)
        {
            if(setting.subkey == "")
            {
                if(jobParams.hasOwnProperty(setting.name))
                {
                    setting.choice = jobParams[setting.name].value;
                }
            }
            else
            {
                if(jobParams.hasOwnProperty(setting.subkey) &&jobParams[setting.subkey].value.hasOwnProperty(setting.name))
                {
                    setting.choice = jobParams[setting.subkey].value[setting.name].value;
                }
            }
        }
        else
        { // Clear jobParams of invalid settings
            delete jobParams[setting.name];
        }

    }
    function getChoices(name) {
        return isValid(name) ? printer.attrs[name+"-supported"].value : [];
    }
    function getDefaultChoice(name) {
        return printer.attrs.hasOwnProperty(name+"-default") ? printer.attrs[name+"-default"].value : undefined;
    }
    function choiceMade(setting)
    {
        if(setting.subkey == "")
        {
            if(setting.choice != undefined)
            {
                jobParams[setting.name] = {tag: setting.tag, value: setting.choice};
            }
            else
            {
                delete jobParams[setting.name];
            }
        }
        else
        {
            var tmpObj = Object();
            if(jobParams.hasOwnProperty(setting.subkey))
            {
                tmpObj = jobParams[setting.subkey].value;
            }

            if(setting.choice != undefined)
            {
                tmpObj[setting.name] = {tag: setting.tag, value: setting.choice};
            }
            else
            {
                delete tmpObj[setting.name];
            }

            if(Object.keys(tmpObj).length != 0)
            {
                jobParams[setting.subkey] = {tag: IppMsg.BeginCollection, value: tmpObj};
            }
            else
            {
                delete jobParams[setting.subkey];
            }
        }
//        console.log(JSON.stringify(jobParams));
    }
}
