/*
  Q Light Controller
  vcspeeddialproperties.cpp

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

#include "vcspeeddialproperties.h"
#include "functionselection.h"
#include "vcspeeddial.h"
#include "doc.h"

#define COL_NAME 0
#define COL_TYPE 1
#define PROP_ID  Qt::UserRole

VCSpeedDialProperties::VCSpeedDialProperties(VCSpeedDial* dial, Doc* doc)
    : QDialog(dial)
    , m_dial(dial)
    , m_doc(doc)
{
    Q_ASSERT(dial != NULL);
    Q_ASSERT(doc != NULL);

    setupUi(this);

    /* Name */
    m_nameEdit->setText(m_dial->caption());

    /* Functions */
    foreach (quint32 id, m_dial->functions())
        createFunctionItem(id);

    /* Speed types */
    if (dial->speedTypes() & VCSpeedDial::FadeIn)
        m_fadeInCheck->setChecked(true);
    if (dial->speedTypes() & VCSpeedDial::FadeOut)
        m_fadeOutCheck->setChecked(true);
    if (dial->speedTypes() & VCSpeedDial::Duration)
        m_durationCheck->setChecked(true);
}

VCSpeedDialProperties::~VCSpeedDialProperties()
{
}

void VCSpeedDialProperties::accept()
{
    /* Name */
    m_dial->setCaption(m_nameEdit->text());

    /* Functions */
    m_dial->setFunctions(functions());

    /* Speed types */
    VCSpeedDial::SpeedTypes types = 0;
    if (m_fadeInCheck->isChecked() == true)
        types |= VCSpeedDial::FadeIn;
    if (m_fadeOutCheck->isChecked() == true)
        types |= VCSpeedDial::FadeOut;
    if (m_durationCheck->isChecked() == true)
        types |= VCSpeedDial::Duration;
    m_dial->setSpeedTypes(types);

    QDialog::accept();
}

void VCSpeedDialProperties::slotAddClicked()
{
    FunctionSelection fs(this, m_doc);
    fs.setMultiSelection(true);
    fs.setDisabledFunctions(functions().toList());
    if (fs.exec() == QDialog::Accepted)
    {
        foreach (quint32 id, fs.selection())
            createFunctionItem(id);
    }
}

void VCSpeedDialProperties::slotRemoveClicked()
{
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    while (it.hasNext() == true)
        delete it.next();
}

QSet <quint32> VCSpeedDialProperties::functions() const
{
    QSet <quint32> set;
    for (int i = 0; i < m_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_tree->topLevelItem(i);
        Q_ASSERT(item != NULL);

        QVariant var = item->data(COL_NAME, PROP_ID);
        if (var.isValid() == true)
            set << var.toUInt();
    }

    return set;
}

void VCSpeedDialProperties::createFunctionItem(quint32 id)
{
    Function* function = m_doc->function(id);
    if (function != NULL)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
        item->setText(COL_NAME, function->name());
        item->setText(COL_TYPE, function->typeString());
        item->setData(COL_NAME, PROP_ID, id);
    }
}
