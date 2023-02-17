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

void DBusAdaptor::Open(const QStringList &uris, const QVariantMap &)
{
    if (!uris.isEmpty()) {
        QMetaObject::invokeMethod(_view->rootObject(), "openFile", Q_ARG(QVariant, uris.at(0)));
    }
    QMetaObject::invokeMethod(_view->rootObject(), "activate");
}

void DBusAdaptor::Activate(const QVariantMap&)
{
    QMetaObject::invokeMethod(_view->rootObject(), "activate");
}
