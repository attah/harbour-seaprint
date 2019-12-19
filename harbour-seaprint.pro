TARGET = harbour-seaprint

CONFIG += sailfishapp

SOURCES += src/harbour-seaprint.cpp \
    src/ippdiscovery.cpp \
    src/bytestream.cpp \
    src/ippmsg.cpp \
    src/ippprinter.cpp


DISTFILES += qml/harbour-seaprint.qml \
    qml/cover/CoverPage.qml \
    qml/components/*.qml \
    qml/pages/*.qml \
    qml/pages/*.js \
    qml/pages/*svg \
    rpm/harbour-seaprint.changes.in \
    rpm/harbour-seaprint.changes.run.in \
    rpm/harbour-seaprint.spec \
    rpm/harbour-seaprint.yaml \
    translations/*.ts \
    harbour-seaprint.desktop

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

CONFIG += sailfishapp_i18n

TRANSLATIONS += \
    translations/harbour-seaprint-de.ts \
    translations/harbour-seaprint-zh_CN.ts

HEADERS += \
    src/ippdiscovery.h \
    src/bytestream.h \
    src/ippmsg.h \
    src/ippprinter.h
