#include <QtQuick>
#include <QVariant>
#include <QDBusConnection>

#include <sailfishapp.h>
#include <src/ippdiscovery.h>
#include <src/ippprinter.h>
#include <src/imageitem.h>
#include <src/mimer.h>
#include <src/convertchecker.h>
#include <src/settings.h>
#include <src/rangelistchecker.h>
#include <src/dbusadaptor.h>
#include "argget.h"
#include <iostream>

Q_DECLARE_METATYPE(CURLcode)
Q_DECLARE_METATYPE(Bytestream)
Q_DECLARE_METATYPE(PrintParameters)

template <class T>
static QObject* singletontype_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    T *inst = T::instance();
    return inst;

}

int main(int argc, char *argv[])
{
    bool prestart = false;
    std::string FileName;
    SwitchArg<bool> pretsartOpt(prestart, {"--prestart"}, "Run prestart");

    PosArg fileArg(FileName, "File to print", true);
    ArgGet args({&pretsartOpt}, {&fileArg});
    if(!args.get_args(argc, argv))
    {
        return 1;
    }

    qRegisterMetaType<CURLcode>();
    qRegisterMetaType<Bytestream>();
    qRegisterMetaType<PrintParameters>();

    // Turn on/off logging according to setting
    QLoggingCategory::defaultCategory()->setEnabled(QtMsgType::QtDebugMsg, Settings::instance()->debugLog());

    QGuiApplication* app = SailfishApp::application(argc, argv);

    app->setOrganizationName(QStringLiteral("net.attah"));
    app->setApplicationName(QStringLiteral("seaprint"));
    app->setApplicationVersion(QStringLiteral(SEAPRINT_VERSION));

    qmlRegisterSingletonType<IppDiscovery>("seaprint.ippdiscovery", 1, 0, "IppDiscovery", singletontype_provider<IppDiscovery>);
    qmlRegisterSingletonType<Mimer>("seaprint.mimer", 1, 0, "Mimer", singletontype_provider<Mimer>);
    qmlRegisterSingletonType<ConvertChecker>("seaprint.convertchecker", 1, 0, "ConvertChecker", singletontype_provider<ConvertChecker>);
    qmlRegisterSingletonType<Settings>("seaprint.settings", 1, 0, "SeaPrintSettings", singletontype_provider<Settings>);
    qmlRegisterSingletonType<RangeListChecker>("seaprint.rangelistchecker", 1, 0, "RangeListChecker", singletontype_provider<RangeListChecker>);

    qmlRegisterType<IppPrinter>("seaprint.ippprinter", 1, 0, "IppPrinter");
    qmlRegisterType<ImageItem>("seaprint.imageitem", 1, 0, "ImageItem");
    qmlRegisterUncreatableType<IppMsg>("seaprint.ippmsg", 1, 0, "IppMsg", "Only used to supply an enum type");

    QQuickView* view = SailfishApp::createView();

    view->engine()->addImportPath(SailfishApp::pathTo("qml/pages").toString());
    view->setSource(SailfishApp::pathToMainQml());

    DBusAdaptor dbus(view);

    if (!QDBusConnection::sessionBus().registerObject("/net/attah/seaprint", view))
        qWarning() << "Could not register /net/attah/seaprint D-Bus object.";

    if (!QDBusConnection::sessionBus().registerService("net.attah.seaprint"))
        qWarning() << "Could not register net.attah.seaprint D-Bus service.";

    if(!FileName.empty())
    {
        qDebug() << "Opening" << FileName.c_str();
        QVariant fileNameVariant(FileName.c_str());
        QMetaObject::invokeMethod(view->rootObject(), "openFile", Q_ARG(QVariant, fileNameVariant));
    }
    if(!prestart)
    {
        view->show();
    }
    return app->exec();
}
