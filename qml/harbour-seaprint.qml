import QtQuick 2.0
import Sailfish.Silica 1.0
import QtQuick.LocalStorage 2.0
import Nemo.Notifications 1.0
import Nemo.Configuration 1.0
import "pages"

ApplicationWindow
{
    initialPage: Component { FirstPage { selectedFile: Qt.application.arguments[1] ? Qt.application.arguments[1] : "" } }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations

    Item {
        id: db
        property var db_conn

        Component.onCompleted: {
            db_conn = LocalStorage.openDatabaseSync("SeaprintDB", "1.0", "Seaprint storage", 100000)
            db_conn.transaction(function (tx) {
                tx.executeSql('CREATE TABLE IF NOT EXISTS Favourites (ssid STRING, url STRING)');

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
                    console.log(res.rows.item(i).url)
                    favs.push(res.rows.item(i).url);
                }
            });
            console.log(ssid, favs);
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
    }

    Notification {
        id: notifier

        expireTimeout: 4000

        function notify(data) {
            console.log("notifyMessage", data)
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
        property int expectedValue: 1
    }

    ConfigurationValue
    {
        id: alwaysConvertSetting
        key: "/apps/harbour-seaprint/settings/always-convert"
        defaultValue: false
    }

    ConfigurationValue
    {
        id: forceIncluDeDocumentFormatSetting
        key: "/apps/harbour-seaprint/settings/force-include-document-format"
        defaultValue: false
    }

    ConfigurationValue
    {
        id: removeRedundantConvertAttrsSetting
        key: "/apps/harbour-seaprint/settings/remove-redundant-convert-attributes"
        defaultValue: true
    }

    ConfigurationValue
    {
        id: considerAdditionalFormatsSetting
        key: "/apps/harbour-seaprint/settings/consider-additional-formats"
        defaultValue: true
    }
}

