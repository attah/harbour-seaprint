#include <QtQuick>

#include <sailfishapp.h>
#include <seaprint_version.h>
#include <src/ippdiscovery.h>
#include <src/ippprinter.h>
#include <src/mimer.h>
#include <src/convertchecker.h>

#define PPM2PWG_MAIN ppm2pwg_main
#include <ppm2pwg/ppm2pwg.cpp>

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
    if(argc >= 1 && QString("ppm2pwg") == argv[1])
    {
        return ppm2pwg_main(argc-1, &(argv[1]));
    }


    QGuiApplication* app = SailfishApp::application(argc, argv);

    app->setApplicationVersion(QStringLiteral(SEAPRINT_VERSION));

    qmlRegisterSingletonType<IppDiscovery>("seaprint.ippdiscovery", 1, 0, "IppDiscovery", singletontype_provider<IppDiscovery>);
    qmlRegisterSingletonType<Mimer>("seaprint.mimer", 1, 0, "Mimer", singletontype_provider<Mimer>);
    qmlRegisterSingletonType<ConvertChecker>("seaprint.convertchecker", 1, 0, "ConvertChecker", singletontype_provider<ConvertChecker>);
    qmlRegisterType<IppPrinter>("seaprint.ippprinter", 1, 0, "IppPrinter");

    QQuickView* view = SailfishApp::createView();

    view->engine()->addImportPath(SailfishApp::pathTo("qml/pages").toString());
    view->engine()->addImageProvider(QLatin1String("ippdiscovery"), IppDiscovery::instance());

    view->setSource(SailfishApp::pathToMainQml());
    view->show();
    return app->exec();

}
