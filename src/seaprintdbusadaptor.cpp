#include "seaprintdbusadaptor.h"
#include <QtQuick>

SeaPrintDBusAdaptor::SeaPrintDBusAdaptor(QQuickView *view)
    : QDBusAbstractAdaptor(view)
    , DBusAdaptorBase(view)
{
}

SeaPrintDBusAdaptor::~SeaPrintDBusAdaptor()
{
}

void SeaPrintDBusAdaptor::Open(const QString& fileName)
{
    doOpen(fileName);
}
void SeaPrintDBusAdaptor::OpenPlaintext(const QString& fileContents, const QString& name)
{
    doCreateFileAndOpen(fileContents, name);
}
