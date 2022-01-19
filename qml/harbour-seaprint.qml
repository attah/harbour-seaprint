import QtQuick 2.6
import Sailfish.Silica 1.0
import QtQuick.LocalStorage 2.0
import Nemo.Notifications 1.0
import Nemo.Configuration 1.0
import seaprint.mimer 1.0
import seaprint.settings 1.0

import "pages"
import "components"

ApplicationWindow
{
    id: appWin

    property bool expectCalligra: true

    initialPage: Component { FirstPage { selectedFile: Qt.application.arguments[1] ? Qt.application.arguments[1] : "" } }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: Orientation.All

    property string busyMessage: ""
    property string progress: ""

    WifiChecker {
        id: wifi
    }

    Item {
        id: db
        property var db_conn

        Component.onCompleted: {
            db_conn = LocalStorage.openDatabaseSync("SeaprintDB", "1.0", "Seaprint storage", 100000)
            db_conn.transaction(function (tx) {
                tx.executeSql('CREATE TABLE IF NOT EXISTS Favourites (ssid STRING, url STRING)');
                tx.executeSql('CREATE TABLE IF NOT EXISTS JobSettings (uuid STRING, type STRING, data STRING)');
            });
        }

        function addFavourite(ssid, url) {
            if(isFavourite(ssid, url))
                return;
            db_conn.transaction(function (tx) {
                tx.executeSql('INSERT INTO Favourites VALUES(?, ?)', [ssid, url] );
            });
        }

        function getFavourites(ssid) {
            var favs = [];
            db_conn.transaction(function (tx) {
                var res = tx.executeSql('SELECT * FROM Favourites WHERE ssid=?', [ssid]);
                for (var i = 0; i < res.rows.length; i++) {
                    favs.push(res.rows.item(i).url);
                }
            });
            return favs
        }

        function isFavourite(ssid, url) {
            var isfav = false;
            db_conn.transaction(function (tx) {
                var res = tx.executeSql('SELECT * FROM Favourites WHERE ssid=? AND url=?', [ssid, url]);
                if (res.rows.length > 0) {
                    isfav = true;
                }
            });
            return isfav
        }

        function removeFavourite(ssid, url) {
            db_conn.transaction(function (tx) {
                tx.executeSql('DELETE FROM Favourites WHERE ssid=? AND url=?', [ssid, url] );
            });
        }

        function simplifyType(mimetype) {
            if(Mimer.isImage(mimetype))
            {
                return {simple: "image", translatable: qsTr("images")};
            }
            else
            {
                return {simple: "document", translatable: qsTr("documents")};

            }
        }

        function setJobSettings(uuid, mimetype, settings) {
            var type = simplifyType(mimetype).simple;
            db_conn.transaction(function (tx) {
                tx.executeSql('DELETE FROM JobSettings WHERE uuid=? AND type=?', [uuid, type] );
                tx.executeSql('INSERT INTO JobSettings VALUES(?, ?, ?)', [uuid, type, settings] );
            });
        }

        function getJobSettings(uuid, mimetype) {
            var type = simplifyType(mimetype).simple;
            var settings = "{}";
            db_conn.transaction(function (tx) {
                var res = tx.executeSql('SELECT * FROM JobSettings WHERE uuid=? AND type=?', [uuid, type]);
                if (res.rows.length)
                {
                    settings = res.rows.item(0).data
                }
            });
            return settings
        }

        function removeJobSettings(uuid, mimetype) {
            var type = simplifyType(mimetype).simple;
            db_conn.transaction(function (tx) {
                tx.executeSql('DELETE FROM JobSettings WHERE uuid=? AND type=?', [uuid, type] );
            });
        }
    }

    Notification {
        id: notifier

        expireTimeout: 4000

        function notify(data) {
            body = data
            previewBody = data
            publish()
        }
    }

    ConfigurationValue
    {
        id: nagScreenSetting
        key: "/apps/harbour-seaprint/settings/nag-screen"
        defaultValue: 0
        property int expectedValue: expectCalligra ? 2 : 1
    }

    ConfigurationValue
    {
        id: considerAdditionalFormatsSetting
        key: "/apps/harbour-seaprint/settings/consider-additional-formats"
        defaultValue: true
    }

    ConfigurationValue
    {
        id: ignoreSslErrorsSetting
        key: SeaPrintSettings.ignoreSslErrorsPath
        defaultValue: SeaPrintSettings.ignoreSslErrorsDefault
    }

    ConfigurationValue
    {
        id: debugLogSetting
        key: SeaPrintSettings.debugLogPath
        defaultValue: SeaPrintSettings.debugLogDefault
    }

}

