#include "freedesktopdbusadaptor.h"

#include <QtQuick>

FreedesktopDBusAdaptor::FreedesktopDBusAdaptor(QQuickView *view)
    : QDBusAbstractAdaptor(view)
    , DBusAdaptorBase(view)
{
}

FreedesktopDBusAdaptor::~FreedesktopDBusAdaptor()
{
}

void FreedesktopDBusAdaptor::Open(const QStringList& uris, const QVariantMap& platformData)
{
    qDebug() << platformData;
    if (!uris.isEmpty() &&  uris[0] != "") {
        doOpen(uris.at(0));
    }
    else if(platformData.contains("data"))
    {
        doCreateFileAndOpen(platformData["data"].toString(),
                            (platformData.contains("name") ? platformData["name"].toString() : "seaprint"));
    }
}

void FreedesktopDBusAdaptor::Activate(const QVariantMap&)
{
    QMetaObject::invokeMethod(_view->rootObject(), "activate");
}
