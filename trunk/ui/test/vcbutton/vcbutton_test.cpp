/*
  Q Light Controller
  vcbutton_test.cpp

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

#include <QtTest>

#define protected public
#define private public
#include "virtualconsole.h"
#include "vcbutton.h"
#include "vcframe.h"
#undef private
#undef protected

#include "qlcfixturedefcache.h"
#include "vcbutton_test.h"
#include "mastertimer.h"
#include "qlcmacros.h"
#include "outputmap.h"
#include "inputmap.h"
#include "scene.h"
#include "doc.h"
#include "bus.h"

void VCButton_Test::initTestCase()
{
    Bus::init(this);
}

void VCButton_Test::initial()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    VCButton btn(&w, &doc, &om, &im, &mt);
    QCOMPARE(btn.objectName(), QString("VCButton"));
    QCOMPARE(btn.frameStyle(), (int) KVCFrameStyleNone);
    QCOMPARE(btn.caption(), QString());
    QCOMPARE(btn.size(), QSize(50, 50));
    QCOMPARE(btn.function(), Function::invalidId());
    QCOMPARE(btn.intensityAdjustment(), qreal(1.0));
    QCOMPARE(btn.adjustIntensity(), false);
    QCOMPARE(btn.isOn(), false);
    QCOMPARE(btn.action(), VCButton::Toggle);
    QCOMPARE(btn.icon(), QString());
    QVERIFY(btn.m_chooseIconAction != NULL);
    QVERIFY(btn.m_resetIconAction != NULL);

    // Only for coverage
    btn.setBackgroundImage(QString());
}

void VCButton_Test::function()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    VCButton btn(&w, &doc, &om, &im, &mt);
    btn.setFunction(42);
    QCOMPARE(btn.function(), Function::invalidId());

    Scene* s = new Scene(&doc);
    s->setName("Test1");
    doc.addFunction(s);
    btn.setFunction(s->id());
    QCOMPARE(btn.function(), s->id());
    QCOMPARE(btn.toolTip(), QString("Test1"));

    Scene* s2 = new Scene(&doc);
    s2->setName("Test2");
    doc.addFunction(s2);
    btn.setFunction(s2->id());
    QCOMPARE(btn.function(), s2->id());
    QCOMPARE(btn.toolTip(), QString("Test2"));

    btn.setFunction(s2->id() + 1);
    QCOMPARE(btn.function(), Function::invalidId());
    QCOMPARE(btn.toolTip(), QString());

    btn.setFunction(s2->id());
    QCOMPARE(btn.function(), s2->id());
    QCOMPARE(btn.toolTip(), QString("Test2"));

    doc.deleteFunction(s2->id());
    QCOMPARE(btn.function(), Function::invalidId());
    QCOMPARE(btn.toolTip(), QString());

    doc.deleteFunction(s->id());
    QCOMPARE(btn.function(), Function::invalidId());
    QCOMPARE(btn.toolTip(), QString());
}

void VCButton_Test::action()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    VCButton btn(&w, &doc, &om, &im, &mt);
    btn.setAction(VCButton::Flash);
    QCOMPARE(btn.action(), VCButton::Flash);
    btn.setAction(VCButton::Toggle);
    QCOMPARE(btn.action(), VCButton::Toggle);

    QCOMPARE(VCButton::actionToString(VCButton::Toggle), QString("Toggle"));
    QCOMPARE(VCButton::actionToString(VCButton::Flash), QString("Flash"));
    QCOMPARE(VCButton::actionToString((VCButton::Action) 31337), QString("Toggle"));
    QCOMPARE(VCButton::stringToAction("Toggle"), VCButton::Toggle);
    QCOMPARE(VCButton::stringToAction("Flash"), VCButton::Flash);
    QCOMPARE(VCButton::stringToAction("Foobar"), VCButton::Toggle);
}

void VCButton_Test::intensity()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    VCButton btn(&w, &doc, &om, &im, &mt);
    btn.setAdjustIntensity(true);
    QCOMPARE(btn.adjustIntensity(), true);
    btn.setAdjustIntensity(false);
    QCOMPARE(btn.adjustIntensity(), false);

    for (qreal i = -0.5; i < 1.2; i += 0.01)
    {
        btn.setIntensityAdjustment(i);
        QCOMPARE(btn.intensityAdjustment(), CLAMP(i, qreal(0.0), qreal(1.0)));
    }
}

void VCButton_Test::bgcolor()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    VCButton btn(&w, &doc, &om, &im, &mt);
    doc.resetModified();
    btn.setBackgroundColor(QColor(Qt::red));
    QCOMPARE(btn.backgroundColor(), QColor(Qt::red));
    QCOMPARE(btn.palette().color(QPalette::Button), QColor(Qt::red));
    QCOMPARE(doc.isModified(), true);
    QVERIFY(btn.foregroundColor() != QColor(Qt::red));
}

void VCButton_Test::fgcolor()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    VCButton btn(&w, &doc, &om, &im, &mt);
    doc.resetModified();
    btn.setForegroundColor(QColor(Qt::red));
    QCOMPARE(btn.foregroundColor(), QColor(Qt::red));
    QCOMPARE(btn.palette().color(QPalette::ButtonText), QColor(Qt::red));
    QCOMPARE(btn.palette().color(QPalette::WindowText), QColor(Qt::red));
    QCOMPARE(doc.isModified(), true);
    QVERIFY(btn.backgroundColor() != QColor(Qt::red));
}

void VCButton_Test::resetColors()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    VCButton btn(&w, &doc, &om, &im, &mt);

    btn.setForegroundColor(QColor(Qt::red));
    btn.setBackgroundColor(QColor(Qt::blue));
    doc.resetModified();
    btn.resetForegroundColor();
    QCOMPARE(btn.foregroundColor(), w.palette().color(QPalette::WindowText));
    QCOMPARE(btn.backgroundColor(), QColor(Qt::blue));
    QCOMPARE(doc.isModified(), true);

    btn.resetForegroundColor();
    QCOMPARE(btn.foregroundColor(), w.palette().color(QPalette::WindowText));
    QCOMPARE(btn.backgroundColor(), QColor(Qt::blue));

    btn.setForegroundColor(QColor(Qt::red));
    btn.setBackgroundColor(QColor(Qt::blue));
    doc.resetModified();
    btn.resetBackgroundColor();
    QCOMPARE(btn.backgroundColor(), w.palette().color(QPalette::Button));
    QCOMPARE(btn.foregroundColor(), QColor(Qt::red));
    QCOMPARE(doc.isModified(), true);

    btn.resetBackgroundColor();
    QCOMPARE(btn.backgroundColor(), w.palette().color(QPalette::Button));
    QCOMPARE(btn.foregroundColor(), QColor(Qt::red));
}

void VCButton_Test::icon()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    VCButton btn(&w, &doc, &om, &im, &mt);
    doc.resetModified();
    btn.setIcon("../../../gfx/qlc.png");
    QCOMPARE(btn.icon(), QString("../../../gfx/qlc.png"));
    QCOMPARE(doc.isModified(), true);

    doc.resetModified();
    btn.slotResetIcon();
    QCOMPARE(btn.icon(), QString());
    QCOMPARE(doc.isModified(), true);
}

void VCButton_Test::on()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    VCButton btn(&w, &doc, &om, &im, &mt);
    btn.setInputSource(0, 1);

    QCOMPARE(btn.isOn(), false);

    btn.setOn(false);
    QCOMPARE(btn.isOn(), false);

    btn.setOn(true);
    QCOMPARE(btn.isOn(), true);

    btn.setOn(true);
    QCOMPARE(btn.isOn(), true);

    btn.setOn(false);
    QCOMPARE(btn.isOn(), false);
}

void VCButton_Test::keySequence()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    VCButton btn(&w, &doc, &om, &im, &mt);
    QCOMPARE(btn.keySequence(), QKeySequence());

    QKeySequence seq(QKeySequence::Copy);
    btn.setKeySequence(seq);
    QCOMPARE(btn.keySequence(), seq);

    seq = QKeySequence(QKeySequence::Undo);
    QVERIFY(btn.keySequence() != seq);
    btn.setKeySequence(seq);
    QCOMPARE(btn.keySequence(), seq);
}

void VCButton_Test::load()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    Scene* sc = new Scene(&doc);
    doc.addFunction(sc);

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("Button");
    root.setAttribute("Caption", "Pertti");
    root.setAttribute("Icon", "../../../gfx/qlc.png");
    xmldoc.appendChild(root);

    QDomElement wstate = xmldoc.createElement("WindowState");
    wstate.setAttribute("X", "20");
    wstate.setAttribute("Y", "20");
    wstate.setAttribute("Width", "60");
    wstate.setAttribute("Height", "60");
    wstate.setAttribute("Visible", "True");
    root.appendChild(wstate);

    QDomElement appearance = xmldoc.createElement("Appearance");
    root.appendChild(appearance);

    QDomElement function = xmldoc.createElement("Function");
    function.setAttribute("ID", QString::number(sc->id()));
    root.appendChild(function);

    QDomElement input = xmldoc.createElement("Input");
    root.appendChild(input);

    QDomElement action = xmldoc.createElement("Action");
    QDomText actionText = xmldoc.createTextNode("Flash");
    action.appendChild(actionText);
    root.appendChild(action);

    QDomElement key = xmldoc.createElement("Key");
    QDomText keyText = xmldoc.createTextNode(QKeySequence(QKeySequence::Copy).toString());
    key.appendChild(keyText);
    root.appendChild(key);

    QDomElement intensity = xmldoc.createElement("Intensity");
    intensity.setAttribute("Adjust", "True");
    QDomText intensityText = xmldoc.createTextNode("60");
    intensity.appendChild(intensityText);
    root.appendChild(intensity);

    QDomElement foo = xmldoc.createElement("Foo");
    root.appendChild(foo);

    VCButton btn(&w, &doc, &om, &im, &mt);
    QCOMPARE(btn.loadXML(&root), true);
    QCOMPARE(btn.function(), sc->id());
    QCOMPARE(btn.action(), VCButton::Flash);
    QCOMPARE(btn.keySequence(), QKeySequence(QKeySequence::Copy));
    QCOMPARE(btn.adjustIntensity(), true);
    QCOMPARE(btn.intensityAdjustment(), qreal(0.6));

    intensity.setAttribute("Adjust", "False");
    QCOMPARE(btn.loadXML(&root), true);
    QCOMPARE(btn.function(), sc->id());
    QCOMPARE(btn.action(), VCButton::Flash);
    QCOMPARE(btn.keySequence(), QKeySequence(QKeySequence::Copy));
    QCOMPARE(btn.adjustIntensity(), false);
    QCOMPARE(btn.intensityAdjustment(), qreal(0.6));

    root.setTagName("Buton");
    QCOMPARE(btn.loadXML(&root), false);
}

QTEST_MAIN(VCButton_Test)
