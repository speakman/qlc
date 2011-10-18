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

#include <QObject>
#include <QtTest>
#include <QMenu>

#define protected public
#define private public

#include "qlcfixturedefcache.h"
#include "qlcinputsource.h"
#include "virtualconsole.h"
#include "vcbutton_test.h"
#include "mastertimer.h"
#include "qlcmacros.h"
#include "outputmap.h"
#include "inputmap.h"
#include "vcbutton.h"
#include "vcframe.h"
#include "scene.h"
#include "doc.h"
#include "bus.h"

#undef private
#undef protected

void VCButton_Test::initTestCase()
{
    m_doc = NULL;
    Bus::init(this);
}

void VCButton_Test::init()
{
    m_doc = new Doc(this);
    m_doc->masterTimer()->start();

    Fixture* fxi = new Fixture(m_doc);
    fxi->setChannels(4);
    m_doc->addFixture(fxi);
}

void VCButton_Test::cleanup()
{
    delete m_doc;
    m_doc = NULL;
}

void VCButton_Test::initial()
{
    QWidget w;

    VCButton btn(&w, m_doc);
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
    QWidget w;

    VCButton btn(&w, m_doc);
    btn.setFunction(42);
    QCOMPARE(btn.function(), Function::invalidId());

    Scene* s = new Scene(m_doc);
    s->setName("Test1");
    m_doc->addFunction(s);
    btn.setFunction(s->id());
    QCOMPARE(btn.function(), s->id());
    QCOMPARE(btn.toolTip(), QString("Test1"));

    Scene* s2 = new Scene(m_doc);
    s2->setName("Test2");
    m_doc->addFunction(s2);
    btn.setFunction(s2->id());
    QCOMPARE(btn.function(), s2->id());
    QCOMPARE(btn.toolTip(), QString("Test2"));

    btn.setFunction(s2->id() + 1);
    QCOMPARE(btn.function(), Function::invalidId());
    QCOMPARE(btn.toolTip(), QString());

    btn.setFunction(s2->id());
    QCOMPARE(btn.function(), s2->id());
    QCOMPARE(btn.toolTip(), QString("Test2"));

    m_doc->deleteFunction(s2->id());
    QCOMPARE(btn.function(), Function::invalidId());
    QCOMPARE(btn.toolTip(), QString());

    m_doc->deleteFunction(s->id());
    QCOMPARE(btn.function(), Function::invalidId());
    QCOMPARE(btn.toolTip(), QString());
}

void VCButton_Test::action()
{
    QWidget w;

    VCButton btn(&w, m_doc);
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
    QWidget w;

    VCButton btn(&w, m_doc);
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
    QWidget w;

    VCButton btn(&w, m_doc);
    m_doc->resetModified();
    btn.setBackgroundColor(QColor(Qt::red));
    QCOMPARE(btn.backgroundColor(), QColor(Qt::red));
    QCOMPARE(btn.palette().color(QPalette::Button), QColor(Qt::red));
    QCOMPARE(m_doc->isModified(), true);
    QVERIFY(btn.foregroundColor() != QColor(Qt::red));
}

void VCButton_Test::fgcolor()
{
    QWidget w;

    VCButton btn(&w, m_doc);
    m_doc->resetModified();
    btn.setForegroundColor(QColor(Qt::red));
    QCOMPARE(btn.foregroundColor(), QColor(Qt::red));
    QCOMPARE(btn.palette().color(QPalette::ButtonText), QColor(Qt::red));
    QCOMPARE(btn.palette().color(QPalette::WindowText), QColor(Qt::red));
    QCOMPARE(m_doc->isModified(), true);
    QVERIFY(btn.backgroundColor() != QColor(Qt::red));
}

void VCButton_Test::resetColors()
{
    QWidget w;

    VCButton btn(&w, m_doc);

    btn.setForegroundColor(QColor(Qt::red));
    btn.setBackgroundColor(QColor(Qt::blue));
    m_doc->resetModified();
    btn.resetForegroundColor();
    QCOMPARE(btn.foregroundColor(), w.palette().color(QPalette::WindowText));
    QCOMPARE(btn.backgroundColor(), QColor(Qt::blue));
    QCOMPARE(m_doc->isModified(), true);

    btn.resetForegroundColor();
    QCOMPARE(btn.foregroundColor(), w.palette().color(QPalette::WindowText));
    QCOMPARE(btn.backgroundColor(), QColor(Qt::blue));

    btn.setForegroundColor(QColor(Qt::red));
    btn.setBackgroundColor(QColor(Qt::blue));
    m_doc->resetModified();
    btn.resetBackgroundColor();
    QCOMPARE(btn.backgroundColor(), w.palette().color(QPalette::Button));
    QCOMPARE(btn.foregroundColor(), QColor(Qt::red));
    QCOMPARE(m_doc->isModified(), true);

    btn.resetBackgroundColor();
    QCOMPARE(btn.backgroundColor(), w.palette().color(QPalette::Button));
    QCOMPARE(btn.foregroundColor(), QColor(Qt::red));
}

void VCButton_Test::icon()
{
    QWidget w;

    VCButton btn(&w, m_doc);
    m_doc->resetModified();
    btn.setIcon("../../../gfx/qlc.png");
    QCOMPARE(btn.icon(), QString("../../../gfx/qlc.png"));
    QCOMPARE(m_doc->isModified(), true);

    m_doc->resetModified();
    btn.slotResetIcon();
    QCOMPARE(btn.icon(), QString());
    QCOMPARE(m_doc->isModified(), true);
}

void VCButton_Test::on()
{
    QWidget w;

    VCButton btn(&w, m_doc);
    btn.setInputSource(QLCInputSource(0, 1));

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
    QWidget w;

    VCButton btn(&w, m_doc);
    QCOMPARE(btn.keySequence(), QKeySequence());

    QKeySequence seq(QKeySequence::Copy);
    btn.setKeySequence(seq);
    QCOMPARE(btn.keySequence(), seq);

    seq = QKeySequence(QKeySequence::Undo);
    QVERIFY(btn.keySequence() != seq);
    btn.setKeySequence(seq);
    QCOMPARE(btn.keySequence(), seq);
}

void VCButton_Test::copy()
{
    QWidget w;

    Scene* sc = new Scene(m_doc);
    m_doc->addFunction(sc);

    VCButton btn(&w, m_doc);
    btn.setCaption("Foobar");
    btn.setIcon("../../../gfx/qlc.png");
    btn.setFunction(sc->id());
    btn.setAction(VCButton::Flash);
    btn.setKeySequence(QKeySequence(QKeySequence::Undo));
    btn.setAdjustIntensity(true);
    btn.setIntensityAdjustment(0.2);

    VCFrame parent(&w, m_doc);
    VCButton* copy = qobject_cast<VCButton*> (btn.createCopy(&parent));
    QVERIFY(copy != NULL);
    QCOMPARE(copy->caption(), QString("Foobar"));
    QCOMPARE(copy->icon(), QString("../../../gfx/qlc.png"));
    QCOMPARE(copy->function(), sc->id());
    QCOMPARE(copy->action(), VCButton::Flash);
    QCOMPARE(copy->keySequence(), QKeySequence(QKeySequence::Undo));
    QCOMPARE(copy->adjustIntensity(), true);
    QCOMPARE(copy->intensityAdjustment(), qreal(0.2));
    delete copy;
}

void VCButton_Test::load()
{
    QWidget w;

    Scene* sc = new Scene(m_doc);
    m_doc->addFunction(sc);

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

    VCButton btn(&w, m_doc);
    QCOMPARE(btn.loadXML(&root), true);
    QCOMPARE(btn.caption(), QString("Pertti"));
    QCOMPARE(btn.icon(), QString("../../../gfx/qlc.png"));
    QCOMPARE(btn.function(), sc->id());
    QCOMPARE(btn.action(), VCButton::Flash);
    QCOMPARE(btn.keySequence(), QKeySequence(QKeySequence::Copy));
    QCOMPARE(btn.adjustIntensity(), true);
    QCOMPARE(btn.intensityAdjustment(), qreal(0.6));
    QCOMPARE(btn.pos(), QPoint(20, 20));
    QCOMPARE(btn.size(), QSize(60, 60));

    intensity.setAttribute("Adjust", "False");
    QCOMPARE(btn.loadXML(&root), true);
    QCOMPARE(btn.caption(), QString("Pertti"));
    QCOMPARE(btn.icon(), QString("../../../gfx/qlc.png"));
    QCOMPARE(btn.function(), sc->id());
    QCOMPARE(btn.action(), VCButton::Flash);
    QCOMPARE(btn.keySequence(), QKeySequence(QKeySequence::Copy));
    QCOMPARE(btn.adjustIntensity(), false);
    QCOMPARE(btn.intensityAdjustment(), qreal(0.6));
    QCOMPARE(btn.pos(), QPoint(20, 20));
    QCOMPARE(btn.size(), QSize(60, 60));

    root.setTagName("Buton");
    QCOMPARE(btn.loadXML(&root), false);
}

void VCButton_Test::save()
{
    QWidget w;

    Scene* sc = new Scene(m_doc);
    m_doc->addFunction(sc);

    VCButton btn(&w, m_doc);
    btn.setCaption("Foobar");
    btn.setIcon("../../../gfx/qlc.png");
    btn.setFunction(sc->id());
    btn.setAction(VCButton::Flash);
    btn.setKeySequence(QKeySequence(QKeySequence::Undo));
    btn.setAdjustIntensity(true);
    btn.setIntensityAdjustment(0.2);

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("Root");
    xmldoc.appendChild(root);

    int function = 0, action = 0, key = 0, intensity = 0, wstate = 0, appearance = 0;
    QCOMPARE(btn.saveXML(&xmldoc, &root), true);
    QDomElement tag = root.firstChild().toElement();
    QCOMPARE(tag.tagName(), QString("Button"));
    QCOMPARE(tag.attribute("Icon"), QString("../../../gfx/qlc.png"));
    QCOMPARE(tag.attribute("Caption"), QString("Foobar"));
    QDomNode node = tag.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == "Function")
        {
            function++;
            QCOMPARE(tag.attribute("ID"), QString::number(sc->id()));
        }
        else if (tag.tagName() == "Action")
        {
            action++;
            QCOMPARE(tag.text(), QString("Flash"));
        }
        else if (tag.tagName() == "Key")
        {
            key++;
            QCOMPARE(tag.text(), QKeySequence(QKeySequence::Undo).toString());
        }
        else if (tag.tagName() == "Intensity")
        {
            intensity++;
            QCOMPARE(tag.attribute("Adjust"), QString("True"));
            QCOMPARE(tag.text(), QString("20"));
        }
        else if (tag.tagName() == "WindowState")
        {
            wstate++;
        }
        else if (tag.tagName() == "Appearance")
        {
            appearance++;
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(tag.tagName()).toUtf8().constData());
        }

        node = node.nextSibling();
    }

    QCOMPARE(function, 1);
    QCOMPARE(action, 1);
    QCOMPARE(key, 1);
    QCOMPARE(intensity, 1);
    QCOMPARE(wstate, 1);
    QCOMPARE(appearance, 1);
}

void VCButton_Test::customMenu()
{
    QWidget w;

    VCButton btn(&w, m_doc);
    QMenu* menu = btn.customMenu(NULL);
    QVERIFY(menu != NULL);
    QCOMPARE(menu->title(), tr("Icon"));
    QCOMPARE(menu->actions().size(), 2);
    QCOMPARE(menu->actions()[0], btn.m_chooseIconAction);
    QCOMPARE(menu->actions()[1], btn.m_resetIconAction);
    delete menu;
}

void VCButton_Test::toggle()
{
    QWidget w;

    Scene* sc = new Scene(m_doc);
    sc->setValue(0, 0, 255);
    sc->setFadeInSpeed(1000);
    sc->setFadeOutSpeed(1000);
    m_doc->addFunction(sc);

    VCButton btn(&w, m_doc);
    btn.setCaption("Foobar");
    btn.setFunction(sc->id());
    btn.setAction(VCButton::Toggle);
    btn.setKeySequence(QKeySequence(QKeySequence::Undo));
    btn.setAdjustIntensity(true);
    btn.setIntensityAdjustment(0.2);

    // Mouse button press in design mode doesn't toggle the function
    QCOMPARE(m_doc->mode(), Doc::Design);
    QMouseEvent ev(QEvent::MouseButtonPress, QPoint(0, 0), Qt::LeftButton, 0, 0);
    btn.mousePressEvent(&ev);
    QCOMPARE(m_doc->masterTimer()->m_functionList.size(), 0);
    ev = QMouseEvent(QEvent::MouseButtonRelease, QPoint(0, 0), Qt::LeftButton, 0, 0);
    btn.mouseReleaseEvent(&ev);
    QCOMPARE(m_doc->masterTimer()->m_functionList.size(), 0);

    // Mouse button press in operate mode should toggle the function
    m_doc->setMode(Doc::Operate);
    btn.slotKeyPressed(QKeySequence(QKeySequence::Undo));
    QCOMPARE(m_doc->masterTimer()->m_functionList.size(), 1);
    QCOMPARE(m_doc->masterTimer()->m_functionList[0], sc);
    QCOMPARE(sc->intensity(), btn.intensityAdjustment());
    btn.slotKeyReleased(QKeySequence(QKeySequence::Undo));
    QTest::qWait(100); // Allow MasterTimer to take the function under execution
    QCOMPARE(sc->stopped(), false);
    QCOMPARE(btn.isOn(), true);

    ev = QMouseEvent(QEvent::MouseButtonPress, QPoint(0, 0), Qt::LeftButton, 0, 0);
    btn.mousePressEvent(&ev);
    QCOMPARE(sc->m_stop, true);
    QCOMPARE(btn.isOn(), true);

    btn.slotFunctionStopped(sc->id());
    QCOMPARE(btn.isOn(), false);
    VCButton another(&w, m_doc);
    QVERIFY(btn.palette().color(QPalette::Button) != another.palette().color(QPalette::Button));
    QTest::qWait(500);
    QVERIFY(btn.palette().color(QPalette::Button) == another.palette().color(QPalette::Button));

    ev = QMouseEvent(QEvent::MouseButtonRelease, QPoint(0, 0), Qt::LeftButton, 0, 0);
    btn.mouseReleaseEvent(&ev);
}

void VCButton_Test::flash()
{
    QWidget w;

    Scene* sc = new Scene(m_doc);
    m_doc->addFunction(sc);

    VCButton btn(&w, m_doc);
    btn.setCaption("Foobar");
    btn.setFunction(sc->id());
    btn.setAction(VCButton::Flash);
    btn.setKeySequence(QKeySequence(QKeySequence::Undo));
    btn.setAdjustIntensity(false);
    btn.setIntensityAdjustment(0.2);

    QSignalSpy spy(sc, SIGNAL(flashing(quint32,bool)));

    m_doc->setMode(Doc::Operate);
    btn.slotKeyPressed(QKeySequence(QKeySequence::Undo));
    QCOMPARE(m_doc->masterTimer()->m_functionList.size(), 0);
    QCOMPARE(btn.isOn(), true);
    QCOMPARE(sc->intensity(), qreal(1.0));
    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy[0][0].toUInt(), sc->id());
    QCOMPARE(spy[0][1].toBool(), true);

    btn.slotKeyReleased(QKeySequence(QKeySequence::Undo));
    QCOMPARE(btn.isOn(), false);
    QCOMPARE(spy.size(), 2);
    QCOMPARE(spy[1][0].toUInt(), sc->id());
    QCOMPARE(spy[1][1].toBool(), false);

    btn.slotFunctionFlashing(sc->id() + 1, true);
    QCOMPARE(btn.isOn(), false);
}

void VCButton_Test::input()
{
    QWidget w;

    Scene* sc = new Scene(m_doc);
    sc->setValue(0, 0, 255);
    sc->setFadeInSpeed(1000);
    sc->setFadeOutSpeed(1000);
    m_doc->addFunction(sc);

    VCButton btn(&w, m_doc);
    btn.setCaption("Foobar");
    btn.setFunction(sc->id());
    btn.setAction(VCButton::Flash);
    btn.setAdjustIntensity(true);
    btn.setIntensityAdjustment(1.0);
    btn.setInputSource(QLCInputSource(0, 0));

    btn.slotInputValueChanged(0, 0, 255);
    QCOMPARE(btn.isOn(), false);

    m_doc->setMode(Doc::Operate);

    btn.slotInputValueChanged(0, 0, 255);
    QCOMPARE(btn.isOn(), true);

    btn.slotInputValueChanged(0, 0, 1);
    QCOMPARE(btn.isOn(), true);

    btn.slotInputValueChanged(0, 0, 0);
    QCOMPARE(btn.isOn(), false);

    btn.slotInputValueChanged(0, 0, 0);
    QCOMPARE(btn.isOn(), false);

    btn.slotInputValueChanged(0, 0, 1);
    QCOMPARE(btn.isOn(), true);

    btn.slotInputValueChanged(0, 0, 0);
    QCOMPARE(btn.isOn(), false);

    m_doc->setMode(Doc::Design);
    btn.setAction(VCButton::Toggle);
    m_doc->setMode(Doc::Operate);

    btn.slotInputValueChanged(0, 0, 255);
    QTest::qWait(100);
    QCOMPARE(btn.isOn(), true);
    QCOMPARE(sc->intensity(), btn.intensityAdjustment());

    btn.slotInputValueChanged(0, 0, 0);
    QCOMPARE(sc->m_stop, false);

    btn.slotInputValueChanged(0, 0, 255);
    QCOMPARE(sc->m_stop, true);
}

void VCButton_Test::paint()
{
    QWidget w;

    VCButton btn(&w, m_doc);

    w.show();
    btn.show();

    QTest::qWait(1);

    btn.setOn(true);
    btn.update();
    QTest::qWait(1);
    btn.setOn(false);
    btn.update();
    QTest::qWait(1);
    btn.setIcon("../../../gfx/qlc.png");
    btn.update();
    QTest::qWait(1);
    btn.setCaption("Foobar");
    btn.update();
    QTest::qWait(1);
    btn.setAction(VCButton::Flash);
    btn.update();
    QTest::qWait(1);
    m_doc->setMode(Doc::Operate);
    btn.update();
    QTest::qWait(1);
}

QTEST_MAIN(VCButton_Test)
