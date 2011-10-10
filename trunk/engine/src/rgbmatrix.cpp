/*
  Q Light Controller
  rgbmatrix.cpp

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

#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDomText>
#include <QDebug>
#include <cmath>

#include "fixturegroup.h"
#include "genericfader.h"
#include "fadechannel.h"
#include "rgbmatrix.h"
#include "qlcmacros.h"
#include "doc.h"

#define KXMLQLCRGBMatrixPattern "Pattern"
#define KXMLQLCRGBMatrixMonoColor "MonoColor"
#define KXMLQLCRGBMatrixFixtureGroup "FixtureGroup"

#define KPatternOutwardBox  "Outward Box"
#define KPatternFullRows    "Full Rows"
#define KPatternFullColumns "Full Columns"

/****************************************************************************
 * Initialization
 ****************************************************************************/

RGBMatrix::RGBMatrix(Doc* doc)
    : Function(doc, Function::RGBMatrix)
    , m_fixtureGroup(FixtureGroup::invalidId())
    , m_pattern(RGBMatrix::OutwardBox)
    , m_monoColor(Qt::red)
    , m_fader(NULL)
{
    setName(tr("New RGB Matrix"));
    setBus(Bus::defaultHold());
    setFadeBus(Bus::defaultFade());
}

RGBMatrix::~RGBMatrix()
{
}

/****************************************************************************
 * Copying
 ****************************************************************************/

Function* RGBMatrix::createCopy(Doc* doc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new RGBMatrix(doc);
    if (copy->copyFrom(this) == false || doc->addFunction(copy) == false)
    {
        delete copy;
        copy = NULL;
    }

    return copy;
}

bool RGBMatrix::copyFrom(const Function* function)
{
    const RGBMatrix* mtx = qobject_cast<const RGBMatrix*> (function);
    if (mtx == NULL)
        return false;

    m_fixtureGroup = mtx->m_fixtureGroup;
    m_pattern = mtx->m_pattern;
    m_fadeBus = mtx->m_fadeBus;
    m_monoColor = mtx->m_monoColor;

    return Function::copyFrom(function);
}

/****************************************************************************
 * Fixtures
 ****************************************************************************/

void RGBMatrix::setFixtureGroup(quint32 id)
{
    m_fixtureGroup = id;
}

quint32 RGBMatrix::fixtureGroup() const
{
    return m_fixtureGroup;
}

/****************************************************************************
 * Pattern
 ****************************************************************************/

QStringList RGBMatrix::patternNames()
{
    QStringList list;
    list << patternToString(RGBMatrix::OutwardBox);
    list << patternToString(RGBMatrix::FullRows);
    list << patternToString(RGBMatrix::FullColumns);
    return list;
}

void RGBMatrix::setPattern(const RGBMatrix::Pattern& pat)
{
    m_pattern = pat;
}

RGBMatrix::Pattern RGBMatrix::pattern() const
{
    return m_pattern;
}

RGBMap RGBMatrix::colorMap(quint32 step, quint32 totalSteps) const
{
    FixtureGroup* grp = doc()->fixtureGroup(fixtureGroup());
    if (grp == NULL)
        return RGBMap();

    RGBMap map(grp->size().height());
    for (int y = 0; y < map.size(); y++)
        map[y].fill(Qt::black, grp->size().width());

    switch (pattern())
    {
    case OutwardBox:
        outwardBox(step, totalSteps, direction(), grp->size(), monoColor(), map);
        break;
    case FullRows:
        fullRows(step, totalSteps, direction(), grp->size(), monoColor(), map);
        break;
    case FullColumns:
        fullColumns(step, totalSteps, direction(), grp->size(), monoColor(), map);
        break;
    default:
        break;
    }

    return map;
}

RGBMatrix::Pattern RGBMatrix::stringToPattern(const QString& str)
{
    if (str == KPatternOutwardBox)
        return OutwardBox;
    else if (str == KPatternFullRows)
        return FullRows;
    else if (str == KPatternFullColumns)
        return FullColumns;
    else
        return OutwardBox;
}

QString RGBMatrix::patternToString(RGBMatrix::Pattern pat)
{
    switch (pat)
    {
    case RGBMatrix::OutwardBox:
        return KPatternOutwardBox;
    case RGBMatrix::FullRows:
        return KPatternFullRows;
    case RGBMatrix::FullColumns:
        return KPatternFullColumns;
    default:
        return KPatternOutwardBox;
    }
}

void RGBMatrix::outwardBox(qreal step, qreal totalSteps, Function::Direction direction,
                           const QSize& size, const QColor& color, RGBMap& map)
{
    qreal scale = 0;
    if (totalSteps > 0)
        scale = step / totalSteps;

    if (direction == Function::Backward)
        scale = 1.0 - scale;

    // Make sure we don't go beyond $map's boundaries
    qreal left   = ceil(qreal(size.width()) / qreal(2));
    left         = left - (left * scale);
    left         = CLAMP(left, 0, size.width() - 1);

    qreal right  = floor(qreal(size.width()) / qreal(2));
    right        = right + ((size.width() - right) * scale);
    right        = CLAMP(right, 0, size.width() - 1);

    qreal top    = ceil(qreal(size.height()) / qreal(2));
    top          = top - (top * scale);
    top          = CLAMP(top, 0, size.height() - 1);

    qreal bottom = floor(qreal(size.height()) / qreal(2));
    bottom       = bottom + ((size.height() - bottom) * scale);
    bottom       = CLAMP(bottom, 0, size.height() - 1);

    for (int i = left; i <= right; i++) {
        map[top][i] = color;
        map[bottom][i] = color;
    }

    for (int i = top; i <= bottom; i++) {
        map[i][left] = color;
        map[i][right] = color;
    }
}

void RGBMatrix::fullRows(qreal step, qreal totalSteps, Function::Direction direction,
                         const QSize& size, const QColor& color, RGBMap& map)
{
    qreal scale = 0;
    if (totalSteps > 0)
        scale = step / totalSteps;

    if (direction == Function::Backward)
        scale = 1.0 - scale;

    // Make sure we don't go beyond $map's boundaries
    qreal left   = qreal(0);
    qreal right  = qreal(size.width() - 1);
    qreal top    = MIN(scale * qreal(size.height()), qreal(size.height() - 1));
    qreal bottom = top;

    for (int i = left; i <= right; i++) {
        map[top][i] = color;
        map[bottom][i] = color;
    }
}

void RGBMatrix::fullColumns(qreal step, qreal totalSteps, Function::Direction direction,
                            const QSize& size, const QColor& color, RGBMap& map)
{
    qreal scale = 0;
    if (totalSteps > 0)
        scale = step / totalSteps;

    if (direction == Function::Backward)
        scale = 1.0 - scale;

    // Make sure we don't go beyond $map's boundaries
    qreal left   = MIN(scale * qreal(size.width()), qreal(size.width() - 1));
    qreal right  = left;
    qreal top    = qreal(0);
    qreal bottom = qreal(size.height() - 1);

    for (int i = top; i <= bottom; i++) {
        map[i][left] = color;
        map[i][right] = color;
    }
}

/****************************************************************************
 * Colour
 ****************************************************************************/

void RGBMatrix::setMonoColor(const QColor& c)
{
    m_monoColor = c;
}

QColor RGBMatrix::monoColor() const
{
    return m_monoColor;
}

/****************************************************************************
 * Fade bus
 ****************************************************************************/

void RGBMatrix::setFadeBus(quint32 id)
{
    m_fadeBus = id;
}

quint32 RGBMatrix::fadeBus() const
{
    return m_fadeBus;
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool RGBMatrix::loadXML(const QDomElement* root)
{
    QDomNode node;
    QDomElement tag;

    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root->attribute(KXMLQLCFunctionType) != typeToString(Function::RGBMatrix))
    {
        qWarning() << Q_FUNC_INFO << "Function is not an RGB matrix";
        return false;
    }

    /* Load matrix contents */
    node = root->firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();

        if (tag.tagName() == KXMLQLCBus)
        {
            if (tag.attribute(KXMLQLCBusRole) == KXMLQLCBusHold)
                setBus(tag.text().toUInt());
            else if (tag.attribute(KXMLQLCBusRole) == KXMLQLCBusFade)
                setFadeBus(tag.text().toUInt());
            else
                qWarning() << Q_FUNC_INFO << "Unrecognized bus role:" << tag.attribute(KXMLQLCBusRole);
        }
        else if (tag.tagName() == KXMLQLCRGBMatrixPattern)
        {
            setPattern(stringToPattern(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCRGBMatrixFixtureGroup)
        {
            setFixtureGroup(tag.text().toUInt());
        }
        else if (tag.tagName() == KXMLQLCFunctionDirection)
        {
            setDirection(Function::stringToDirection(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCFunctionRunOrder)
        {
            setRunOrder(Function::stringToRunOrder(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCRGBMatrixMonoColor)
        {
            setMonoColor(QColor::fromRgb(QRgb(tag.text().toUInt())));
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown RGB matrix tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool RGBMatrix::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    QDomElement root;
    QDomElement tag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    /* Function tag */
    root = doc->createElement(KXMLQLCFunction);
    wksp_root->appendChild(root);

    root.setAttribute(KXMLQLCFunctionID, id());
    root.setAttribute(KXMLQLCFunctionType, Function::typeToString(type()));
    root.setAttribute(KXMLQLCFunctionName, name());

    /* Fade bus */
    tag = doc->createElement(KXMLQLCBus);
    root.appendChild(tag);
    tag.setAttribute(KXMLQLCBusRole, KXMLQLCBusFade);
    str.setNum(fadeBus());
    text = doc->createTextNode(str);
    tag.appendChild(text);

    /* Hold bus */
    tag = doc->createElement(KXMLQLCBus);
    root.appendChild(tag);
    tag.setAttribute(KXMLQLCBusRole, KXMLQLCBusHold);
    str.setNum(bus());
    text = doc->createTextNode(str);
    tag.appendChild(text);

    /* Pattern */
    tag = doc->createElement(KXMLQLCRGBMatrixPattern);
    root.appendChild(tag);
    text = doc->createTextNode(patternToString(pattern()));
    tag.appendChild(text);

    /* Mono Color */
    tag = doc->createElement(KXMLQLCRGBMatrixMonoColor);
    root.appendChild(tag);
    text = doc->createTextNode(QString::number(monoColor().rgb()));
    tag.appendChild(text);

    /* Fixture Group */
    tag = doc->createElement(KXMLQLCRGBMatrixFixtureGroup);
    root.appendChild(tag);
    text = doc->createTextNode(QString::number(fixtureGroup()));
    tag.appendChild(text);

    /* Direction */
    tag = doc->createElement(KXMLQLCFunctionDirection);
    root.appendChild(tag);
    text = doc->createTextNode(Function::directionToString(direction()));
    tag.appendChild(text);

    /* Run order */
    tag = doc->createElement(KXMLQLCFunctionRunOrder);
    root.appendChild(tag);
    text = doc->createTextNode(Function::runOrderToString(runOrder()));
    tag.appendChild(text);

    return true;
}

/****************************************************************************
 * Running
 ****************************************************************************/

void RGBMatrix::preRun(MasterTimer* timer)
{
    Q_UNUSED(timer);
    m_direction = direction();

    Q_ASSERT(m_fader == NULL);
    m_fader = new GenericFader(doc());

    Function::preRun(timer);
}

void RGBMatrix::write(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(timer);
    Q_UNUSED(universes);

    incrementElapsed();

    FixtureGroup* grp = doc()->fixtureGroup(fixtureGroup());
    if (grp == NULL)
        return;

    RGBMap map = colorMap(elapsed() % busValue(), busValue());

    for (int y = 0; y < map.size(); y++)
    {
        for (int x = 0; x < map[y].size(); x++)
        {
            QLCPoint pt(x, y);
            Fixture* fxi = doc()->fixture(grp->fixture(pt));
            if (fxi == NULL)
                continue;

            QList <quint32> channels = fxi->rgbChannels();
            if (channels.isEmpty() == false)
            {
                FadeChannel fc;
                fc.setBus(fadeBus());
                fc.setFixture(fxi->id());

                fc.setChannel(channels.takeFirst());
                fc.setTarget(map[y][x].red());
                insertStartValues(m_fader, fc);

                if (m_fader->channels().contains(fc) == false ||
                    m_fader->channels()[fc].target() != fc.target())
                {
                    m_fader->add(fc);
                }

                fc.setChannel(channels.takeFirst());
                fc.setTarget(map[y][x].green());
                insertStartValues(m_fader, fc);

                if (m_fader->channels().contains(fc) == false ||
                    m_fader->channels()[fc].target() != fc.target())
                {
                    m_fader->add(fc);
                }

                fc.setChannel(channels.takeFirst());
                fc.setTarget(map[y][x].blue());
                insertStartValues(m_fader, fc);

                if (m_fader->channels().contains(fc) == false ||
                    m_fader->channels()[fc].target() != fc.target())
                {
                    m_fader->add(fc);
                }
            }
            else
            {
                channels = fxi->cmyChannels();
                if (channels.isEmpty() == true)
                    continue;

                FadeChannel fc;
                fc.setBus(fadeBus());
                fc.setFixture(fxi->id());

                fc.setChannel(channels.takeFirst());
                fc.setTarget(map[y][x].cyan());
                insertStartValues(m_fader, fc);
                m_fader->add(fc);

                fc.setChannel(channels.takeFirst());
                fc.setTarget(map[y][x].magenta());
                insertStartValues(m_fader, fc);
                m_fader->add(fc);

                fc.setChannel(channels.takeFirst());
                fc.setTarget(map[y][x].yellow());
                insertStartValues(m_fader, fc);
                m_fader->add(fc);
            }
        }
    }

    m_fader->write(universes);

    if (elapsed() >= busValue())
    {
        if (runOrder() == Function::PingPong)
        {
            if (m_direction == Function::Backward)
                m_direction = Function::Forward;
            else
                m_direction = Function::Backward;

            //resetElapsed();
        }
        else if (runOrder() == Function::SingleShot)
        {
            stop();
        }
        else // if (runOrder() == Function::Loop)
        {
            //resetElapsed();
        }
    }
}

void RGBMatrix::postRun(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(timer);
    Q_UNUSED(universes);

    delete m_fader;
    m_fader = NULL;

    Function::postRun(timer, universes);
}

void RGBMatrix::insertStartValues(const GenericFader* fader, FadeChannel& fc)
{
    Q_ASSERT(fader != NULL);

    if (fader->channels().contains(fc) == true)
    {
        FadeChannel old = fader->channels()[fc];
        fc.setCurrent(old.current());
        fc.setStart(old.current());
    }
    else
    {
        fc.setCurrent(0);
        fc.setStart(0);
    }
}

