/*
  Q Light Controller
  ewing.cpp

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

#include <QHostAddress>
#include <QMessageBox>
#include <QByteArray>
#include <QString>

#include "ewing.h"

/****************************************************************************
 * Header data common to all wings
 ****************************************************************************/

#define EWING_BYTE_HEADER   0 /* 4 bytes */
#define EWING_HEADER_SIZE   4
#define EWING_HEADER_OUTPUT "WODD"
#define EWING_HEADER_INPUT "WIDD"

/****************************************************************************
 * Status data common to all wings
 ****************************************************************************/

#define EWING_BYTE_FIRMWARE   4 /* Firmware version, 8bit value (0-255) */

#define EWING_BYTE_FLAGS      5 /* Wing flags */
#define EWING_FLAGS_MASK_PGUP 1 << 7
#define EWING_FLAGS_MASK_PGDN 1 << 6
#define EWING_FLAGS_MASK_BACK 1 << 5 /* Playback Wing only */
#define EWING_FLAGS_MASK_GO   1 << 4 /* Playback Wing only */
#define EWING_FLAGS_MASK_TYPE 0x3

/** ENTTEC wings send data thru UDP port 3330 */
const int EWing::UDPPort = 3330;

/****************************************************************************
 * Initialization
 ****************************************************************************/

EWing::EWing(QObject* parent, const QHostAddress& address,
	     const QByteArray& data) : QObject(parent)
{
	m_address = address;
	m_type = resolveType(data);
	m_firmware = resolveFirmware(data);
}

EWing::~EWing()
{
}

bool EWing::isOutputData(const QByteArray& data)
{
	/* Check, if there's enough bytes for the header */
	if (data.size() < EWING_HEADER_SIZE)
		return false;

	QByteArray header(data.mid(EWING_BYTE_HEADER, EWING_HEADER_SIZE));
	return (header == EWING_HEADER_OUTPUT);
}

/****************************************************************************
 * Wing data
 ****************************************************************************/

EWing::Type EWing::resolveType(const QByteArray& data)
{
	/* Check, if there's enough bytes for wing flags */
	if (data.size() < EWING_BYTE_FLAGS)
		return Unknown;

	unsigned char flags = data[EWING_BYTE_FLAGS];
	return EWing::Type(flags & EWING_FLAGS_MASK_TYPE);
}

unsigned char EWing::resolveFirmware(const QByteArray& data)
{
	/* Check, if there's enough bytes for wing flags */
	if (data.size() < EWING_BYTE_FIRMWARE)
		return 0;

	return data[EWING_BYTE_FIRMWARE];
}

/****************************************************************************
 * Input data
 ****************************************************************************/

unsigned char EWing::cacheValue(int channel)
{
	if (m_values.size() < channel)
		return 0;
	else
		return m_values[channel];
}

void EWing::setCacheValue(int channel, char value)
{
	Q_ASSERT(m_values.size() < channel);

	if (m_values[channel] != value && channel != EWING_INVALID_CHANNEL)
	{
		m_values[channel] = value;
		emit valueChanged(channel, t_input_value(value));
	}
}

void EWing::feedBack(t_input_channel channel, t_input_value value)
{
	Q_UNUSED(channel);
	Q_UNUSED(value);
}
