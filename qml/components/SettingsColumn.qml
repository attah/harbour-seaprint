import QtQuick 2.6
import Sailfish.Silica 1.0
import seaprint.mimer 1.0
import seaprint.ippprinter 1.0

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
        if(jobParams.hasOwnProperty(setting.name))
        {
            if(setting.valid)
            {
                setting.choice = jobParams[setting.name].value;
            }
            else
            { // Clear jobParams of invalid settings
                jobParams[setting.name] = undefined;
            }
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
            if(setting.choice != undefined)
            {
                jobParams[setting.name] = {tag: setting.tag, value: setting.choice};
            }
            else
            {
                jobParams[setting.name] = undefined;
            }
            console.log(JSON.stringify(jobParams));
        }


}
