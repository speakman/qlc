include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = qlcfixturemode_test

QT      += testlib xml
CONFIG  -= app_bundle

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += qlcfixturemode_test.cpp
HEADERS += qlcfixturemode_test.h
