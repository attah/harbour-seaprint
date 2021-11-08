#include <QtQuick>

#include <sailfishapp.h>
#include <seaprint_version.h>
#include <src/ippdiscovery.h>
#include <src/ippprinter.h>
#include <src/mimer.h>
#include <src/convertchecker.h>
#include <src/svgprovider.h>
#include <src/settings.h>

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
    QGuiApplication* app = SailfishApp::application(argc, argv);

    app->setOrganizationName(QStringLiteral("net.attah"));
    app->setApplicationName(QStringLiteral("seaprint"));
    app->setApplicationVersion(QStringLiteral(SEAPRINT_VERSION));

    qmlRegisterSingletonType<IppDiscovery>("seaprint.ippdiscovery", 1, 0, "IppDiscovery", singletontype_provider<IppDiscovery>);
    qmlRegisterSingletonType<Mimer>("seaprint.mimer", 1, 0, "Mimer", singletontype_provider<Mimer>);
    qmlRegisterSingletonType<ConvertChecker>("seaprint.convertchecker", 1, 0, "ConvertChecker", singletontype_provider<ConvertChecker>);
    qmlRegisterSingletonType<ConvertChecker>("seaprint.settings", 1, 0, "SeaPrintSettings", singletontype_provider<Settings>);
    qmlRegisterType<IppPrinter>("seaprint.ippprinter", 1, 0, "IppPrinter");
    qmlRegisterUncreatableType<IppMsg>("seaprint.ippmsg", 1, 0, "IppMsg", "Only used to supply an enum type");

    QQuickView* view = SailfishApp::createView();

    view->engine()->addImportPath(SailfishApp::pathTo("qml/pages").toString());
    view->engine()->addImageProvider(QLatin1String("ippdiscovery"), IppDiscovery::instance());
    view->engine()->addImageProvider(QLatin1String("svg"), SvgProvider::instance());

    view->setSource(SailfishApp::pathToMainQml());
    view->show();
    return app->exec();
}
