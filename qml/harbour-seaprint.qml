import QtQuick 2.0
import Sailfish.Silica 1.0
import QtQuick.LocalStorage 2.0
import Nemo.Notifications 1.0
import "pages"

ApplicationWindow
{
    initialPage: Component { FirstPage { } }
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
            db_conn.transaction(function (tx) {
                tx.executeSql('REPLACE INTO Favourites VALUES(?, ?)', [ssid, url] );
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
}

