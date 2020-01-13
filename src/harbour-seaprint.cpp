#include <QtQuick>

#include <sailfishapp.h>
#include <seaprint_version.h>
#include <src/ippdiscovery.h>
#include <src/ippprinter.h>

static QObject* ippdiscovery_singletontype_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    IppDiscovery *ippdiscovery = IppDiscovery::instance();
    return ippdiscovery;

}

int main(int argc, char *argv[])
{
    QGuiApplication* app = SailfishApp::application(argc, argv);

    app->setApplicationVersion(QStringLiteral(SEAPRINT_VERSION));

    qmlRegisterSingletonType<IppDiscovery>("seaprint.ippdiscovery", 1, 0, "IppDiscovery", ippdiscovery_singletontype_provider);
    qmlRegisterType<IppPrinter>("seaprint.ippprinter", 1, 0, "IppPrinter");

    QQuickView* view = SailfishApp::createView();

    view->engine()->addImportPath(SailfishApp::pathTo("qml/pages").toString());
    view->engine()->addImageProvider(QLatin1String("ippdiscovery"), IppDiscovery::instance());

    view->setSource(SailfishApp::pathToMainQml());
    view->show();
    return app->exec();

}
