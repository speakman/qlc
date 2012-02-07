/*
  Q Light Controller
  speeddialwidget.cpp

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

#include <QSettings>
#include <QGroupBox>
#include <QLineEdit>
#include <QLayout>
#include <QDebug>

#include "speeddialwidget.h"
#include "mastertimer.h"
#include "speeddial.h"
#include "apputil.h"

#define SETTINGS_GEOMETRY "speeddialwidget/geometry"

SpeedDialWidget::SpeedDialWidget(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
    , m_fadeIn(NULL)
    , m_fadeOut(NULL)
    , m_duration(NULL)
    , m_optionalTextGroup(NULL)
    , m_optionalTextEdit(NULL)
{
    new QVBoxLayout(this);

    /* Create dials */
    m_fadeIn = new SpeedDial(this);
    m_fadeIn->setTitle(tr("Fade In"));
    layout()->addWidget(m_fadeIn);
    connect(m_fadeIn, SIGNAL(valueChanged(uint)), this, SIGNAL(fadeInChanged(uint)));

    m_fadeOut = new SpeedDial(this);
    m_fadeOut->setTitle(tr("Fade Out"));
    layout()->addWidget(m_fadeOut);
    connect(m_fadeOut, SIGNAL(valueChanged(uint)), this, SIGNAL(fadeOutChanged(uint)));

    m_duration = new SpeedDial(this);
    m_duration->setTitle(tr("Duration"));
    layout()->addWidget(m_duration);
    connect(m_duration, SIGNAL(valueChanged(uint)), this, SIGNAL(durationChanged(uint)));

    /* Optional text */
    m_optionalTextGroup = new QGroupBox(this);
    layout()->addWidget(m_optionalTextGroup);
    new QVBoxLayout(m_optionalTextGroup);
    m_optionalTextEdit = new QLineEdit(m_optionalTextGroup);
    m_optionalTextGroup->layout()->addWidget(m_optionalTextEdit);
    m_optionalTextGroup->setVisible(false);
    connect(m_optionalTextEdit, SIGNAL(textEdited(const QString&)),
            this, SIGNAL(optionalTextEdited(const QString&)));

    /* Position */
    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        this->restoreGeometry(var.toByteArray());
    AppUtil::ensureWidgetIsVisible(this);
}

SpeedDialWidget::~SpeedDialWidget()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

/****************************************************************************
 * Speed settings
 ****************************************************************************/

void SpeedDialWidget::setFadeInSpeed(uint ms)
{
    m_fadeIn->setValue(ms);
}

uint SpeedDialWidget::fadeIn() const
{
    return m_fadeIn->value();
}

void SpeedDialWidget::setFadeOutSpeed(uint ms)
{
    m_fadeOut->setValue(ms);
}

uint SpeedDialWidget::fadeOut() const
{
    return m_fadeOut->value();
}

void SpeedDialWidget::setDuration(uint ms)
{
    m_duration->setValue(ms);
}

uint SpeedDialWidget::duration() const
{
    return m_duration->value();
}

/************************************************************************
 * Optional text
 ************************************************************************/

void SpeedDialWidget::setOptionalTextTitle(const QString& title)
{
    m_optionalTextGroup->setTitle(title);
    m_optionalTextGroup->setVisible(!title.isEmpty());
}

QString SpeedDialWidget::optionalTextTitle() const
{
    return m_optionalTextGroup->title();
}

void SpeedDialWidget::setOptionalText(const QString& text)
{
    m_optionalTextEdit->setText(text);
}

QString SpeedDialWidget::optionalText() const
{
    return m_optionalTextEdit->text();
}
