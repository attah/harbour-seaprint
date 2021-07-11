import QtQuick 2.6
import Nemo.DBus 2.0


Item {
    id: checker

    property bool connected: false
    property var ssid

    DBusInterface {
        bus: DBus.SystemBus

        service: 'net.connman'
        path: '/'
        iface: 'net.connman.Manager'

        signalsEnabled: true

        Component.onCompleted: go()

        function servicesChanged() {
            console.log("services changed");
            go();
        }

        function go() {
            console.log("go!")
            call("GetServices", undefined,
                 function(result) {
                     for (var i = 0; i < result.length; i++) {
                         var entry = result[i][1];
                         if(entry.Type == "wifi" && (entry.State == "online" || entry.State == "ready")) {
                             if(checker.ssid != entry.Name) {
                                 // For whatever reason, the onchanged signal triggers when there isn't really a change
                                 // so don't update the ssid if it is the same
                                 checker.ssid = entry.Name;
                             }
                             checker.connected = true;
                             return;
                         }
                     }
                     checker.ssid = undefined;
                     checker.connected = false;
                 },
                 function(error, message) {
                     console.log('call failed', error, 'message:', message);
                 })
        }

    }
}
