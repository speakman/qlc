/*
  Q Light Controller
  vclabel.cpp

  Copyright (c) Heikki Junnila, Stefan Krumm

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

#include <QPaintEvent>
#include <QPainter>
#include <QString>
#include <QDebug>
#include <QStyle>
#include <QSize>
#include <QtXml>

#include "qlcfile.h"

#include "virtualconsole.h"
#include "mastertimer.h"
#include "inputmap.h"
#include "vclabel.h"
#include "doc.h"

VCLabel::VCLabel(QWidget* parent, Doc* doc, OutputMap* outputMap, InputMap* inputMap,
                 MasterTimer* masterTimer)
    : VCWidget(parent, doc, outputMap, inputMap, masterTimer)
{
    /* Set the class name "VCLabel" as the object name as well */
    setObjectName(VCLabel::staticMetaObject.className());

    setCaption(tr("Label"));
    resize(QSize(100, 30));
}

VCLabel::~VCLabel()
{
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

VCWidget* VCLabel::createCopy(VCWidget* parent)
{
    Q_ASSERT(parent != NULL);

    VCLabel* label = new VCLabel(parent, m_doc, m_outputMap, m_inputMap, m_masterTimer);
    if (label->copyFrom(this) == false)
    {
        delete label;
        label = NULL;
    }

    return label;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCLabel::loadXML(const QDomElement* root)
{
    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCVCLabel)
    {
        qWarning() << Q_FUNC_INFO << "Label node not found";
        return false;
    }

    /* Caption */
    setCaption(root->attribute(KXMLQLCVCCaption));

    /* Children */
    QDomNode node = root->firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCWindowState)
        {
            int x = 0, y = 0, w = 0, h = 0;
            bool visible = false;
            loadXMLWindowState(&tag, &x, &y, &w, &h, &visible);
            setGeometry(x, y, w, h);
        }
        else if (tag.tagName() == KXMLQLCVCAppearance)
        {
            loadXMLAppearance(&tag);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown label tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool VCLabel::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
    QDomElement root;
    QDomElement tag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(vc_root != NULL);

    /* VC Label entry */
    root = doc->createElement(KXMLQLCVCLabel);
    vc_root->appendChild(root);

    /* Caption */
    root.setAttribute(KXMLQLCVCCaption, caption());

    /* Window state */
    saveXMLWindowState(doc, &root);

    /* Appearance */
    saveXMLAppearance(doc, &root);

    return true;
}

/****************************************************************************
 * Drawing
 ****************************************************************************/

void VCLabel::paintEvent(QPaintEvent* e)
{
    bool enabled = false;
    if (mode() == Doc::Operate)
        enabled = true;

    QPainter painter(this);
    style()->drawItemText(&painter, rect(), Qt::AlignCenter, palette(),
                          enabled, caption(), foregroundRole());
    painter.end();

    VCWidget::paintEvent(e);
}

