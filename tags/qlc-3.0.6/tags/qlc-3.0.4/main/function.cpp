/*
  Q Light Controller
  function.cpp

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

#include <QApplication>
#include <QMessageBox>
#include <QString>
#include <QDebug>
#include <QIcon>
#include <QtXml>

#include "common/qlcfile.h"

#include "mastertimer.h"
#include "collection.h"
#include "function.h"
#include "chaser.h"
#include "scene.h"
#include "efx.h"
#include "bus.h"
#include "doc.h"

#define KInvalidFunctionID -1

const QString KSceneString      (      "Scene" );
const QString KChaserString     (     "Chaser" );
const QString KEFXString        (        "EFX" );
const QString KCollectionString ( "Collection" );
const QString KUndefinedString  (  "Undefined" );

const QString KLoopString       (       "Loop" );
const QString KPingPongString   (   "PingPong" );
const QString KSingleShotString ( "SingleShot" );

const QString KBackwardString   (   "Backward" );
const QString KForwardString    (    "Forward" );

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Function::Function(QObject* parent) : QObject(parent)
{
	m_id = Function::invalidId();
	m_name = QString::null;
	m_runOrder = Loop;
	m_direction = Forward;
	m_busID = Bus::defaultFade();
	m_flashing = false;
	m_running = false;
}

Function::~Function()
{
}

/*****************************************************************************
 * Copying
 *****************************************************************************/

bool Function::copyFrom(const Function* function)
{
	/* Don't copy the function's parent */

	if (function == NULL)
		return false;

	setName(function->name());
	setRunOrder(function->runOrder());
	setDirection(function->direction());
	setBus(function->busID());

	return true;
}

/*****************************************************************************
 * ID
 *****************************************************************************/

void Function::setID(t_function_id id)
{
	/* Don't set doc modified status or emit changed signal, because this
	   function is called only once during function creation. */
	m_id = id;
}

t_function_id Function::invalidId()
{
	return KInvalidFunctionID;
}

/*****************************************************************************
 * Name
 *****************************************************************************/

void Function::setName(const QString& name)
{
	m_name = QString(name);
}

/*****************************************************************************
 * Type
 *****************************************************************************/

QString Function::typeString() const
{
	return Function::typeToString(type());
}

QString Function::typeToString(const Type& type)
{
	switch (type)
	{
	case Scene:
		return KSceneString;
	case Chaser:
		return KChaserString;
	case EFX:
		return KEFXString;
	case Collection:
		return KCollectionString;
	case Undefined:
	default:
		return KUndefinedString;
	}
}

Function::Type Function::stringToType(const QString& string)
{
	if (string == KSceneString)
		return Scene;
	else if (string == KChaserString)
		return Chaser;
	else if (string == KEFXString)
		return EFX;
	else if (string == KCollectionString)
		return Collection;
	else
		return Undefined;
}

QIcon Function::icon() const
{
	switch (type())
	{
		case Scene:
			return QIcon(":/scene.png");
		case Chaser:
			return QIcon(":/chaser.png");
		case EFX:
			return QIcon(":/efx.png");
		case Collection:
			return QIcon(":/collection.png");
		default:
			return QIcon(":/function.png");
	}
}

/*****************************************************************************
 * Running order
 *****************************************************************************/

void Function::setRunOrder(const Function::RunOrder& order)
{
	if (order == Loop || order == SingleShot || order == PingPong)
		m_runOrder = order;
	else
		m_runOrder = Loop;
}

QString Function::runOrderToString(const RunOrder& order)
{
	switch (order)
	{
	default:
	case Loop:
		return KLoopString;
		break;

	case PingPong:
		return KPingPongString;
		break;

	case SingleShot:
		return KSingleShotString;
		break;
	}
}

Function::RunOrder Function::stringToRunOrder(const QString& str)
{
	if (str == KPingPongString)
		return PingPong;
	else if (str == KSingleShotString)
		return SingleShot;
	else
		return Loop;
}

/*****************************************************************************
 * Direction
 *****************************************************************************/

void Function::setDirection(const Function::Direction& dir)
{
	if (dir == Forward || dir == Backward)
		m_direction = dir;
	else
		m_direction = Forward;
}

QString Function::directionToString(const Direction& dir)
{
	switch (dir)
	{
	default:
	case Forward:
		return KForwardString;
		break;

	case Backward:
		return KBackwardString;
		break;
	}
}

Function::Direction Function::stringToDirection(const QString& str)
{
	if (str == KBackwardString)
		return Backward;
	else
		return Forward;
}

/*****************************************************************************
 * Bus
 *****************************************************************************/

void Function::setBus(quint32 id)
{
	if (id < Bus::count() && type() != Collection)
		m_busID = id;
}

/*****************************************************************************
 * Fixtures
 *****************************************************************************/

void Function::slotFixtureRemoved(t_fixture_id fid)
{
	Q_UNUSED(fid);
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool Function::loader(const QDomElement* root, Doc* doc)
{
	Q_ASSERT(root != NULL);
	Q_ASSERT(doc != NULL);

	if (root->tagName() != KXMLQLCFunction)
	{
		qWarning("Function node not found!");
		return false;
	}

	/* Get common information from the tag's attributes */
	t_function_id id = root->attribute(KXMLQLCFunctionID).toInt();
	QString name = root->attribute(KXMLQLCFunctionName);
	Type type = Function::stringToType(root->attribute(KXMLQLCFunctionType));

	/* Check for ID validity before creating the function */
	if (id < 0 || id >= KFunctionArraySize)
	{
		qWarning() << "Function ID" << id << "out of bounds.";
		return false;
	}

	/* Create a new function according to the type */
	Function* function = NULL;
	if (type == Function::Scene)
	        function = new class Scene(doc);
	else if (type == Function::Chaser)
	        function = new class Chaser(doc);
	else if (type == Function::Collection)
		function = new class Collection(doc);
	else if (type == Function::EFX)
		function = new class EFX(doc);
	else
		return false;

	function->setName(name);
	if (function->loadXML(root) == true)
	{
		if (doc->addFunction(function, id) == true)
		{
			/* Success */
			return true;
		}
		else
		{
			qWarning() << "Function" << name
				   << "cannot be created.";
			delete function;
			return false;
		}
	}
	else
	{
		qWarning() << "Function" << name << "cannot be loaded.";
		delete function;
		return false;
	}
}

/*****************************************************************************
 * Flash
 *****************************************************************************/

void Function::flash(QByteArray* universes)
{
	Q_UNUSED(universes);

	if (m_flashing == false)
	{
		m_flashing = true;
		emit flashing(m_id, true);
	}
}

void Function::unFlash(QByteArray* universes)
{
	Q_UNUSED(universes);

	if (m_flashing == true)
	{
		m_flashing = false;
		emit flashing(m_id, false);
	}
}

/*****************************************************************************
 * Running
 *****************************************************************************/

void Function::start(MasterTimer* timer)
{
	if (isRunning() == false)
	{
		Q_ASSERT(timer != NULL);

		m_elapsed = 0;
		m_running = true;
		timer->startFunction(this);
		emit running(m_id);
	}
}

void Function::stop(MasterTimer* timer)
{
	if (isRunning() == true)
	{
		Q_ASSERT(timer != NULL);

		timer->stopFunction(this);
		m_running = false;
		emit stopped(m_id);
	}
}
