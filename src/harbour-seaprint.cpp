#include <QtQuick>

#include <sailfishapp.h>
#include <src/ippdiscovery.h>
#include <src/ippprinter.h>

int main(int argc, char *argv[])
{
    qmlRegisterType<IppDiscovery>("seaprint.ippdiscovery", 1, 0, "IppDiscovery");
    qmlRegisterType<IppPrinter>("seaprint.ippprinter", 1, 0, "IppPrinter");

    // SailfishApp::main() will display "qml/harbour-printtool.qml", if you need more
    // control over initialization, you can use:
    //
    //   - SailfishApp::application(int, char *[]) to get the QGuiApplication *
    //   - SailfishApp::createView() to get a new QQuickView * instance
    //   - SailfishApp::pathTo(QString) to get a QUrl to a resource file
    //   - SailfishApp::pathToMainQml() to get a QUrl to the main QML file
    //
    // To display the view, call "show()" (will show fullscreen on device).

    return SailfishApp::main(argc, argv);
}
