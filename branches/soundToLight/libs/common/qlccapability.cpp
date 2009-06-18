/*
  Q Light Controller
  qlccapability.cpp

  Copyright (C) Heikki Junnila

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

#include <QString>
#include <QDebug>
#include <QFile>
#include <QtXml>

#include "qlccapability.h"
#include "qlctypes.h"

QLCCapability::QLCCapability(t_value min, t_value max, const QString& name)
{
	m_min = min;
	m_max = max;
	m_name = name;
}

QLCCapability::QLCCapability(const QLCCapability* capability)
{
	m_min = KChannelValueMin;
	m_max = KChannelValueMax;
	m_name = QString::null;

	if (capability != NULL)
		*this = *capability;
}

QLCCapability::~QLCCapability()
{
}

QLCCapability& QLCCapability::operator=(const QLCCapability& capability)
{
	if (this != &capability)
	{
		m_min = capability.m_min;
		m_max = capability.m_max;
		m_name = capability.m_name;
	}

	return *this;
}

bool QLCCapability::operator<(const QLCCapability& capability) const
{
	if (m_min < capability.m_min)
		return true;
	else
		return false;
}

bool QLCCapability::overlaps(const QLCCapability& cap)
{
	if (m_min >= cap.min() && m_min <= cap.max())
		return true;
	else if (m_max >= cap.min() && m_max <= cap.max())
		return true;
	else if (cap.min() >= m_min && cap.min() <= m_max)
		return true;
	else if (cap.max() >= m_min && cap.max() <= m_max)
		return true;
	else
		return false;
}

bool QLCCapability::saveXML(QDomDocument* doc, QDomElement* root)
{
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	/* QLCCapability entry */
	tag = doc->createElement(KXMLQLCCapability);
	root->appendChild(tag);

	/* Min limit attribute */
	str.setNum(m_min);
	tag.setAttribute(KXMLQLCCapabilityMin, str);

	/* Max limit attribute */
	str.setNum(m_max);
	tag.setAttribute(KXMLQLCCapabilityMax, str);

	/* Name value */
	text = doc->createTextNode(m_name);
	tag.appendChild(text);

	return true;
}

bool QLCCapability::loadXML(const QDomElement* root)
{
	t_value min;
	t_value max;
	QString str;

	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCCapability)
	{
		qWarning() << "Capability node not found.";
		return false;
	}

	/* Get low limit attribute (critical) */
	str = root->attribute(KXMLQLCCapabilityMin);
	if (str == QString::null)
	{
		qWarning() << "Capability has no minimum limit.";
		return false;
	}
	else
	{
		min = CLAMP(str.toInt(), 0, KChannelValueMax);
	}

	/* Get high limit attribute (critical) */
	str = root->attribute(KXMLQLCCapabilityMax);
	if (str == QString::null)
	{
		qWarning() << "Capability has no maximum limit.";
		return false;
	}
	else
	{
		max = CLAMP(str.toInt(), 0, KChannelValueMax);
	}

	if (min <= max)
	{
		setName(root->text());
		setMin(min);
		setMax(max);
		return true;
	}
	else
	{
		qWarning() << "Capability min(" << min
			   << ") is greater than max(" << max << ").";
		return false;
	}
}