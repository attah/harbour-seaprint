#include <QtQuick>

#include <sailfishapp.h>
#include <src/ippdiscovery.h>
#include <src/ippprinter.h>
#include <src/imageitem.h>
#include <src/mimer.h>
#include <src/convertchecker.h>
#include <src/settings.h>

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
    qmlRegisterSingletonType<ConvertChecker>("seaprint.settings", 1, 0, "SeaPrintSettings", singletontype_provider<Settings>);
    qmlRegisterType<IppPrinter>("seaprint.ippprinter", 1, 0, "IppPrinter");
    qmlRegisterType<ImageItem>("seaprint.imageitem", 1, 0, "ImageItem");
    qmlRegisterUncreatableType<IppMsg>("seaprint.ippmsg", 1, 0, "IppMsg", "Only used to supply an enum type");

    QQuickView* view = SailfishApp::createView();

    view->engine()->addImportPath(SailfishApp::pathTo("qml/pages").toString());

    view->setSource(SailfishApp::pathToMainQml());
    view->show();
    return app->exec();
}
