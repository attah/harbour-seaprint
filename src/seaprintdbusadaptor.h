#ifndef SEAPRINTDBUSADAPTOR_H
#define SEAPRINTDBUSADAPTOR_H

#include "dbusadaptorbase.h"

class SeaPrintDBusAdaptor : public QDBusAbstractAdaptor, public DBusAdaptorBase
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "net.attah.seaprint")

public:
    SeaPrintDBusAdaptor(QQuickView *view);
    ~SeaPrintDBusAdaptor();

public Q_SLOTS:
    Q_NOREPLY void Open(const QString& fileName);
    Q_NOREPLY void OpenPlaintext(const QString& fileContents, const QString& name);

};

#endif // SEAPRINTDBUSADAPTOR_H
