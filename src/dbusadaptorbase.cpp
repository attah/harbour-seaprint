#include "dbusadaptorbase.h"
#include <QtQuick>

void DBusAdaptorBase::doActivate()
{
    QMetaObject::invokeMethod(_view->rootObject(), "activate");
}

void DBusAdaptorBase::doOpen(const QString& fileName)
{
    QMetaObject::invokeMethod(_view->rootObject(), "openFile", Q_ARG(QVariant, fileName));
    doActivate();
}

void DBusAdaptorBase::doCreateFileAndOpen(const QString& fileContents, const QString& name)
{
    QTemporaryFile tmpfile(QDir::tempPath() + "/" + name);
    tmpfile.setAutoRemove(false);
    tmpfile.open();
    tmpfile.write(fileContents.toUtf8());
    tmpfile.close();
    QMetaObject::invokeMethod(_view->rootObject(), "openFile", Q_ARG(QVariant, tmpfile.fileName()));

    doActivate();
}
