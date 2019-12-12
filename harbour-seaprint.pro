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

CONFIG += sailfishapp

SOURCES += src/harbour-seaprint.cpp \
    src/ippdiscovery.cpp \
    src/bytestream.cpp \
    src/ippmsg.cpp \
    src/ippprinter.cpp


DISTFILES += qml/harbour-seaprint.qml \
    qml/cover/CoverPage.qml \
    qml/pages/*.qml \
    qml/pages/*.js \
    *svg \
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
TRANSLATIONS += translations/harbour-seaprint-de.ts

HEADERS += \
    src/ippdiscovery.h \
    src/bytestream.h \
    src/ippmsg.h \
    src/ippprinter.h
