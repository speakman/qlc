/*
  Q Light Controller
  doc.h
  
  Copyright (C) 2000, 2001, 2002 Heikki Junnila
  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef DOC_H
#define DOC_H

#include <qobject.h>
#include <qlist.h>
#include "function.h"
#include "dmxdevice.h"
#include "deviceclass.h"
#include "bus.h"
#include "classes.h"

class JoystickPlugin;

class Doc : public QObject
{
  Q_OBJECT

 public:
  Doc();
  ~Doc();

  void init();

  //
  // Document handling stuff
  //
  void newDocument();
  bool isModified() { return m_modified; }
  void setModified(bool modified);

  QString workspaceFileName() { return m_workspaceFileName; }
  bool loadWorkspaceAs(QString &);
  bool saveWorkspaceAs(QString &);
  bool saveWorkspace();

  // Read file contents to a list of strings
  bool readFileToList(QString &fileName, QList<QString> &list);

  // Return the pointer to our DMX interface object
  DMX* dmx() const { return m_dmx; }

  //
  // DMX Channels
  //
  bool isDMXAddressSpaceFree(unsigned short address, unsigned short channels);
  bool allocateDMXAddressSpace(unsigned short address, unsigned short channels);
  bool freeDMXAddressSpace(unsigned short address, unsigned short channels);
  unsigned short findNextFreeDMXAddress(unsigned short channels);
  DMXChannel* dmxChannel(unsigned short channel);

  //
  // DMX Devices
  //
  void addDevice(DMXDevice*);
  bool removeDevice(DMXDevice*);
  QList <DMXDevice> *deviceList() { return &m_deviceList; }

  // Search functions
  DMXDevice* searchDevice(int id);
  DMXDevice* searchDevice(const QString &name);

  //
  // Functions
  //
  // Function list
  QList<Function> functions() { return m_functions; }

  // Function list operations
  Function* searchFunction(const QString &fname);
  void addFunction(const Function* function);
  void removeFunction(const QString &functionString);

  //
  // Device classes
  //
  // Read device class definition files
  bool readDeviceClasses();

  // Container for device classes
  QList <DeviceClass> *deviceClassList() { return &m_deviceClassList; }

  // Search for manuf + model
  DeviceClass* searchDeviceClass(const QString &manufacturer, const QString &model);
  DeviceClass* searchDeviceClassByID(int id);

  //
  // Bus
  //
  // System bus list
  QList <Bus> *busList()  { return &m_busList; }

  // Bus handling functions
  Bus* searchBus(unsigned int id);
  Bus* searchBus(QString name);
  void addBus(Bus* bus);
  void removeBus(unsigned int id, bool deleteBus = true);

  //
  // Input devices
  //
  // Input device list
  QList <Joystick> *inputDeviceList() { return &m_inputDeviceList; }

  // Input device list operations
  Joystick* searchInputDevice(QString fdName);
  Joystick* searchInputDevice(unsigned int id);
  void addInputDevice(Joystick* j);
  void removeInputDevice(Joystick* j);
  JoystickPlugin* joystickPlugin() { return m_joystickPlugin; }

 signals:
  void deviceListChanged();
  void deviceRemoved(int);
  void newDocumentClicked();

 private:
  void initializeDMXChannels();
  void initializeDMX();
  void initializeJoystickPlugin();

  void dumpDeviceClass(DeviceClass* dc);
  DeviceClass* createDeviceClass(QList<QString> &list);
  DMXDevice* createDevice(QList<QString> &list);
  Function* createFunction(QList<QString> &list);
  void createFunctionContents(QList<QString> &list);
  void createJoystickContents(QList<QString> &list);

 private:
  DMXChannel* m_DMXAddressAllocation[512];
  DMX* m_dmx;

  QString m_workspaceFileName;
  bool m_modified;

  JoystickPlugin* m_joystickPlugin;

  QList <Function> m_functions;
  QList <DMXDevice> m_deviceList;
  QList <DeviceClass> m_deviceClassList;
  QList <Bus> m_busList;
  QList <Joystick> m_inputDeviceList;

};

#endif

