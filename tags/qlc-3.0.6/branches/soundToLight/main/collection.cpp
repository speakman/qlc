/*
  Q Light Controller
  collection.cpp

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
#include <QString>
#include <QDebug>
#include <QFile>
#include <QList>
#include <QtXml>

#include "common/qlcfile.h"

#include "mastertimer.h"
#include "collection.h"
#include "function.h"
#include "doc.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Collection::Collection(QObject* parent) : Function(parent)
{
	setName(tr("New Collection"));

	Doc* doc = qobject_cast <Doc*> (parent);
	if (doc != NULL)
	{
		/* Listen to function removals so that they can be removed from
		   this collection as well. Parent might not always be Doc,
		   but an editor dialog, for example. Such collections cannot
		   be run, though. */
		connect(doc, SIGNAL(functionRemoved(t_function_id)),
			this, SLOT(slotFunctionRemoved(t_function_id)));
	}
}

Collection::~Collection()
{
	m_steps.clear();
}

/*****************************************************************************
 * Function type
 *****************************************************************************/

Function::Type Collection::type() const
{
	return Function::Collection;
}

/*****************************************************************************
 * Copying
 *****************************************************************************/

Function* Collection::createCopy(Doc* doc)
{
    Q_ASSERT(doc != NULL);

	Function* copy = new Collection(doc);
	Q_ASSERT(copy != NULL);
	if (copy->copyFrom(this) == false)
	{
		delete copy;
		copy = NULL;
	}
	else if (doc->addFunction(copy) == false)
	{
		delete copy;
		copy = NULL;
	}
	else
	{
		copy->setName(tr("Copy of %1").arg(name()));
	}

	return copy;
}

bool Collection::copyFrom(const Function* function)
{
	const Collection* coll = qobject_cast<const Collection*> (function);
	if (coll == NULL)
		return false;

	m_steps.clear();
	m_steps = coll->m_steps;

	bool result = Function::copyFrom(function);

	emit changed(m_id);

	return result;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool Collection::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;
	int i = 0;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(wksp_root != NULL);

        /* Function tag */
        root = doc->createElement(KXMLQLCFunction);
        wksp_root->appendChild(root);

	root.setAttribute(KXMLQLCFunctionID, id());
	root.setAttribute(KXMLQLCFunctionType, Function::typeToString(type()));
	root.setAttribute(KXMLQLCFunctionName, name());

	/* Steps */
	QListIterator <t_function_id> it(m_steps);
	while (it.hasNext() == true)
	{
		/* Step tag */
		tag = doc->createElement(KXMLQLCFunctionStep);
		root.appendChild(tag);

		/* Step number */
		tag.setAttribute(KXMLQLCFunctionNumber, i++);

		/* Step Function ID */
		str.setNum(it.next());
		text = doc->createTextNode(str);
		tag.appendChild(text);
	}

	return true;
}

bool Collection::loadXML(const QDomElement* root)
{
	t_fixture_id step_fxi = KNoID;

	QDomNode node;
	QDomElement tag;

	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCFunction)
	{
		qDebug() << "Function node not found!";
		return false;
	}

	/* Load collection contents */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();

		if (tag.tagName() == KXMLQLCFunctionStep)
		{
			step_fxi = tag.text().toInt();
			m_steps.append(step_fxi);
		}
		else
		{
			qDebug() << "Unknown collection tag:" << tag.tagName();
		}

		node = node.nextSibling();
	}

	return true;
}

/*****************************************************************************
 * Contents
 *****************************************************************************/

void Collection::addItem(t_function_id id)
{
	m_steps.append(id);
	emit changed(m_id);
}

void Collection::removeItem(t_function_id id)
{
	m_steps.takeAt(m_steps.indexOf(id));
	emit changed(m_id);
}

void Collection::slotFunctionRemoved(t_function_id function)
{
	if (m_steps.removeAll(function) > 0)
		emit changed(m_id);
}

/*****************************************************************************
 * Running
 *****************************************************************************/

void Collection::arm()
{
}

void Collection::disarm()
{
}

void Collection::stop(MasterTimer* timer)
{
	Doc* doc = qobject_cast <Doc*> (parent());
	Q_ASSERT(doc != NULL);

	/* TODO: this stops these functions, regardless of whether they
	   were started by this collection or not */
	QListIterator <t_function_id> it(m_steps);
	while (it.hasNext() == true)
	{
		Function* function = doc->function(it.next());
		Q_ASSERT(function != NULL);
		function->stop(timer);
	}

	Function::stop(timer);
}

void Collection::start(MasterTimer* timer)
{
	Q_ASSERT(timer != NULL);

	Doc* doc = qobject_cast <Doc*> (parent());
	Q_ASSERT(doc != NULL);

	m_childCount = 0;

	QListIterator <t_function_id> it(m_steps);
	while (it.hasNext() == true)
	{
		Function* function;
		function = doc->function(it.next());
		Q_ASSERT(function != NULL);

		m_childCountMutex.lock();
		m_childCount++;
		m_childCountMutex.unlock();

		connect(function, SIGNAL(stopped(t_function_id)),
			this, SLOT(slotChildStopped(t_function_id)));

		function->start(timer);
	}

	Function::start(timer);
}

bool Collection::write(QByteArray* universes)
{
	Q_UNUSED(universes);

	if (m_childCount == 0)
		return false;
	else
		return true;
}

void Collection::slotChildStopped(t_function_id fid)
{
	Doc* doc = qobject_cast <Doc*> (parent());
	Q_ASSERT(doc != NULL);

	Function* function;
	function = doc->function(fid);
	Q_ASSERT(function != NULL);

	disconnect(function, SIGNAL(stopped(t_function_id)),
		   this, SLOT(slotChildStopped(t_function_id)));

	m_childCountMutex.lock();
	m_childCount--;
	m_childCountMutex.unlock();
}
