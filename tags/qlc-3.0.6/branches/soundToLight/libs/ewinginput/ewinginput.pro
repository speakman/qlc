include(../../variables.pri)

TEMPLATE 	= lib
LANGUAGE 	= C++
TARGET 		= ewinginput

INCLUDEPATH	+= . ../../libs/
CONFIG          += plugin
QT		+= network

target.path	= $$INPUTPLUGINDIR
!macx:INSTALLS	+= target

win32 {
	# Qt Libraries
	qtnetwork.path  = $$LIBSDIR
	qtnetwork.files = $$(QTDIR)/bin/QtNetwork4.dll
	INSTALLS	+= qtnetwork
}

macx:DESTDIR    = ../../main/qlc.app/Contents/Plugins/input

# Input
HEADERS += ewinginput.h \
	   eplaybackwing.h \
	   eshortcutwing.h \
	   eprogramwing.h \
	   ewing.h

SOURCES += ewinginput.cpp \
	   eplaybackwing.cpp \
	   eshortcutwing.cpp \
	   eprogramwing.cpp \
	   ewing.cpp
