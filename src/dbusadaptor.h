#ifndef DBUSADAPTOR_H
#define DBUSADAPTOR_H

#include <QObject>
#include <QtDBus/QDBusAbstractAdaptor>
#include <QQuickView>

class DBusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Application")

public:
    DBusAdaptor(QQuickView *view);
    ~DBusAdaptor();

public Q_SLOTS:
    Q_NOREPLY void Open(const QStringList &uris, const QVariantMap &platformData);
    Q_NOREPLY void Activate(const QVariantMap &platformData);

private:
    QQuickView* _view;

};

#endif // DBUSADAPTOR_H
