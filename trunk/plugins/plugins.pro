TEMPLATE = subdirs

SUBDIRS              += enttecdmxusbout
SUBDIRS              += peperoniout
SUBDIRS              += udmxout
SUBDIRS              += midi
#unix:SUBDIRS         += olaout
!macx:!win32:SUBDIRS += dmx4linux
!macx:SUBDIRS        += vellemanout
SUBDIRS              += enttecwing
!macx:!win32:SUBDIRS += hid
