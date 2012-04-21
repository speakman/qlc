include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = hidinput

INCLUDEPATH += ../interfaces
CONFIG      += plugin

HEADERS += ../interfaces/qlcioplugin.h
HEADERS += configurehidinput.h \
           hiddevice.h \
           hideventdevice.h \
           hidinput.h \
           hidjsdevice.h \
           hidpoller.h

FORMS += configurehidinput.ui

SOURCES += configurehidinput.cpp \
           hiddevice.cpp \
           hideventdevice.cpp \
           hidinput.cpp \
           hidjsdevice.cpp \
           hidpoller.cpp

TRANSLATIONS += HID_Input_fi_FI.ts
TRANSLATIONS += HID_Input_de_DE.ts
TRANSLATIONS += HID_Input_es_ES.ts
TRANSLATIONS += HID_Input_fr_FR.ts
TRANSLATIONS += HID_Input_it_IT.ts

target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target
