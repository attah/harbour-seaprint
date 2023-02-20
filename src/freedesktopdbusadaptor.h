#ifndef FREEDESKTOPDBUSADAPTOR_H
#define FREEDESKTOPDBUSADAPTOR_H

#include "dbusadaptorbase.h"

class FreedesktopDBusAdaptor : public QDBusAbstractAdaptor, public DBusAdaptorBase
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Application")

public:
    FreedesktopDBusAdaptor(QQuickView *view);
    ~FreedesktopDBusAdaptor();

public Q_SLOTS:
    Q_NOREPLY void Open(const QStringList& uris, const QVariantMap& platformData);
    Q_NOREPLY void Activate(const QVariantMap& platformData);

};

#endif // FREEDESKTOPDBUSADAPTOR_H
