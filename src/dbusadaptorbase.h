#ifndef DBUSADAPTORBASE_H
#define DBUSADAPTORBASE_H

#include <QObject>
#include <QtDBus/QDBusAbstractAdaptor>
#include <QQuickView>

class DBusAdaptorBase
{
public:
    DBusAdaptorBase(QQuickView* view)
    : _view(view)
    {}

void doActivate();
void doOpen(const QString& fileName);
void doCreateFileAndOpen(const QString& fileContents, const QString& name);

protected:
    QQuickView* _view;

};

#endif // DBUSADAPTORBASE_H
