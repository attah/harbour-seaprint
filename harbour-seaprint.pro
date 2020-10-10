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

# Write version file
VERSION_H = \
"$${LITERAL_HASH}ifndef SEAPRINT_VERSION" \
"$${LITERAL_HASH}   define SEAPRINT_VERSION \"$$VERSION\"" \
"$${LITERAL_HASH}endif"
write_file($$$$OUT_PWD/seaprint_version.h, VERSION_H)

SOURCES += src/harbour-seaprint.cpp \
    src/convertchecker.cpp \
    src/convertworker.cpp \
    src/ippdiscovery.cpp \
    src/ippmsg.cpp \
    src/ippprinter.cpp \
    src/mimer.cpp \
    ppm2pwg/ppm2pwg.cpp \
    ppm2pwg/bytestream/bytestream.cpp

DISTFILES += qml/harbour-seaprint.qml \
    qml/components/CylinderGraph.qml \
    qml/components/LargeChoiceDialog.qml \
    qml/components/SupplyItem.qml \
    qml/cover/CoverPage.qml \
    qml/components/*.qml \
    qml/pages/*.qml \
    qml/pages/*.js \
    qml/pages/*svg \
    qml/pages/BusyPage.qml \
    qml/pages/DebugPage.qml \
    qml/pages/DetailsPage.qml \
    qml/pages/NagScreen.qml \
    qml/pages/SettingsPage.qml \
    rpm/harbour-seaprint.changes.in \
    rpm/harbour-seaprint.changes.run.in \
    rpm/harbour-seaprint.spec \
    rpm/harbour-seaprint.yaml \
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
    src/convertchecker.h \
    src/convertworker.h \
    src/ippdiscovery.h \
    src/ippmsg.h \
    src/ippprinter.h \
    src/mimer.h \
    ppm2pwg/pwg_pghdr_codable.h \
    ppm2pwg/urf_pghdr_codable.h \
    ppm2pwg/bytestream/bytestream.h \
    ppm2pwg/bytestream/codable.h \
    src/papersizes.h

INCLUDEPATH += ppm2pwg \
               ppm2pwg/bytestream
