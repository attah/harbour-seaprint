#include "dbusadaptor.h"

#include <QtQuick>

DBusAdaptor::DBusAdaptor(QQuickView *view)
    : QDBusAbstractAdaptor(view)
    , _view(view)
{
}

DBusAdaptor::~DBusAdaptor()
{
}

void DBusAdaptor::Open(const QStringList& uris, const QVariantMap& platformData)
{
    qDebug() << platformData;
    if (!uris.isEmpty() &&  uris[0] != "") {
        QMetaObject::invokeMethod(_view->rootObject(), "openFile", Q_ARG(QVariant, uris.at(0)));
    }
    else if(platformData.contains("data"))
    {
        QTemporaryFile tmpfile(QDir::tempPath() + "/" + (platformData.contains("name") ? platformData["name"].toString() : "seaprint"));
        tmpfile.setAutoRemove(false);
        tmpfile.open();
        tmpfile.write(platformData["data"].toString().toUtf8());
        tmpfile.close();
        QMetaObject::invokeMethod(_view->rootObject(), "openFile", Q_ARG(QVariant, tmpfile.fileName()));
    }
    QMetaObject::invokeMethod(_view->rootObject(), "activate");
}

void DBusAdaptor::Activate(const QVariantMap&)
{
    QMetaObject::invokeMethod(_view->rootObject(), "activate");
}
