/*
  Q Light Controller
  functioncollection.h
  
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

#ifndef FUNCTIONCOLLECTION_H
#define FUNCTIONCOLLECTION_H

#include <qvaluelist.h>
#include "function.h"

class QDomDocument;

class FunctionCollection : public Function
{
 public:
	FunctionCollection();
	virtual ~FunctionCollection();

	void copyFrom(FunctionCollection* fc, bool append = false);

	QValueList <t_function_id> *steps() { return &m_steps; }

	bool setFixtureInstance(t_fixture_id) { return false; }

	bool addItem(t_function_id);
	bool removeItem(t_function_id);

	bool saveXML(QDomDocument* doc, QDomElement* wksp_root);
	bool loadXML(QDomDocument* doc, QDomElement* root);

	void speedChange();

	void arm();
	void disarm();
	void cleanup();
	void stop();
	void childFinished();

 protected:
	void init();
	void run();

 private:
	QValueList <t_function_id> m_steps;

	int m_childCount;
	QMutex m_childCountMutex;
};

#endif
