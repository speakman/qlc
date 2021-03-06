/*
  Q Light Controller
  dmx4linuxout.h
  
  Copyright (c) Heikki Junnila
  
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

#ifndef DMX4LINUXOUT_H
#define DMX4LINUXOUT_H

#include <QWidget>
#include <QString>
#include <QMutex>
#include <QFile>

#include "common/qlcoutplugin.h"
#include "common/qlctypes.h"

class ConfigureDMX4LinuxOut;

class DMX4LinuxOut : public QObject, public QLCOutPlugin
{
	Q_OBJECT
	Q_INTERFACES(QLCOutPlugin)

	friend class ConfigureDMX4LinuxOut;

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	void init();

	/*********************************************************************
	 * Open/close
	 *********************************************************************/
public:
	void open(t_output output = 0);
	void close(t_output output = 0);
	QStringList outputs();

protected:
	/** File handle for /dev/dmx */
	QFile m_file;

	/*********************************************************************
	 * Name
	 *********************************************************************/
public:
	QString name();

	/*********************************************************************
	 * Configuration
	 *********************************************************************/
public:
	void configure();

	/*********************************************************************
	 * Status
	 *********************************************************************/
public:
	QString infoText(t_output output = KOutputInvalid);

	/*********************************************************************
	 * Value read/write
	 *********************************************************************/
public:
	void writeChannel(t_output output, t_channel channel, t_value value);
	void writeRange(t_output output, t_channel address, t_value* values,
			t_channel num);

	void readChannel(t_output output, t_channel channel, t_value* value);
	void readRange(t_output output, t_channel address, t_value* values,
		       t_channel num);

protected:
	t_value m_values[512];
	QMutex m_mutex;
};

#endif
