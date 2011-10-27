include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = fixturegroup_test

QT      += testlib xml
CONFIG  -= app_bundle

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += fixturegroup_test.cpp
HEADERS += fixturegroup_test.h