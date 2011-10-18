include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = palettegenerator_test

QT      += testlib xml
CONFIG  -= app_bundle

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += palettegenerator_test.cpp
HEADERS += palettegenerator_test.h
