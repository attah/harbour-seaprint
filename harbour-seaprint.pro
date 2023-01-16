# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If name defined in TARGET is changed, the following needs to be done
# to match new name:
#   - corresponding QML filename must be changed
#   - desktop icon filename must be changed
#   - desktop filename must be changed
#   - icon definition filename in desktop file must be changed
#   - translation filenames have to be changed

# The name of your application
TARGET = harbour-seaprint

i18n.path = /usr/share/harbour-seaprint/translations
i18n.files = translations/harbour-seaprint-de.qm \
             translations/harbour-seaprint-zh_CN.qm \
             translations/harbour-seaprint-fr.qm \
             translations/harbour-seaprint-es.qm \
             translations/harbour-seaprint-nl.qm \
             translations/harbour-seaprint-pl.qm

INSTALLS += i18n

# automatic generation of the translation .qm files from .ts files
system(lrelease $$PWD/translations/*.ts)

CONFIG += sailfishapp
QT += svg
PKGCONFIG += mlite5 libcurl poppler-glib glib-2.0 cairo libjpeg
LIBS += -lcurl -lglib-2.0 -lgobject-2.0 -ldl
DEFINES += MADNESS=1
DEFINES += PDF_CREATOR='\\"SeaPrint\ $$VERSION\\"'
DEFINES += SEAPRINT_VERSION='\\"$$VERSION\\"'

SOURCES += src/harbour-seaprint.cpp \
    src/rangelistchecker.cpp \
    src/convertchecker.cpp \
    src/curlrequester.cpp \
    src/imageitem.cpp \
    src/ippdiscovery.cpp \
    src/ippmsg.cpp \
    src/ippprinter.cpp \
    src/mimer.cpp \
    ppm2pwg/printparameters.cpp \
    ppm2pwg/ppm2pwg.cpp \
    ppm2pwg/pdf2printable.cpp \
    ppm2pwg/baselinify.cpp \
    ppm2pwg/bytestream/bytestream.cpp \
    src/overrider.cpp \
    src/printerworker.cpp \
    src/settings.cpp

DISTFILES += qml/harbour-seaprint.qml \
    qml/cover/CoverPage.qml \
    qml/components/*.qml \
    qml/pages/*.qml \
    qml/pages/*.js \
    qml/pages/*svg \
    qml/pages/*png \
    rpm/harbour-seaprint.changes.in \
    rpm/harbour-seaprint.changes.run.in \
    rpm/harbour-seaprint.spec \
    translations/*.ts \
    harbour-seaprint.desktop

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n

# German translation is enabled as an example. If you aren't
# planning to localize your app, remember to comment out the
# following TRANSLATIONS line. And also do not forget to
# modify the localized app name in the the .desktop file.
TRANSLATIONS += translations/harbour-seaprint-de.ts \
                translations/harbour-seaprint-zh_CN.ts \
                translations/harbour-seaprint-fr.ts \
                translations/harbour-seaprint-es.ts \
                translations/harbour-seaprint-nl.ts \
                translations/harbour-seaprint-pl.ts

HEADERS += \
    src/rangelistchecker.h \
    src/convertchecker.h \
    src/curlrequester.h \
    src/imageitem.h \
    src/ippdiscovery.h \
    src/ippmsg.h \
    src/ippprinter.h \
    src/mimer.h \
    ppm2pwg/ppm2pwg.h \
    ppm2pwg/pdf2printable.h \
    ppm2pwg/baselinify.h \
    ppm2pwg/madness.h \
    ppm2pwg/printparameters.h \
    ppm2pwg/PwgPgHdr.h \
    ppm2pwg/PwgPgHdr.codable \
    ppm2pwg/UrfPgHdr.h \
    ppm2pwg/UrfPgHdr.codable \
    ppm2pwg/bytestream/bytestream.h \
    ppm2pwg/bytestream/codable.h \
    src/overrider.h \
    src/papersizes.h \
    src/printerworker.h \
    src/settings.h

INCLUDEPATH += ppm2pwg \
               ppm2pwg/bytestream
