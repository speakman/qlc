/*
  Q Light Controller
  rgbtext.cpp

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
#include <QPainter>
#include <QImage>
#include <QDebug>

#include "rgbtext.h"

#define KXMLQLCRGBTextContent        "Content"
#define KXMLQLCRGBTextFont           "Font"
#define KXMLQLCRGBTextAnimationStyle "Animation"

RGBText::RGBText()
    : RGBAlgorithm()
    , m_text(" Q LIGHT CONTROLLER ")
    , m_animationStyle(Horizontal)
    , m_xOffset(0)
    , m_yOffset(0)
{
}

RGBText::RGBText(const RGBText& t)
    : RGBAlgorithm()
    , m_text(t.text())
    , m_font(t.font())
    , m_animationStyle(t.animationStyle())
    , m_xOffset(t.xOffset())
    , m_yOffset(t.yOffset())
{
}

RGBText::~RGBText()
{
}

RGBAlgorithm* RGBText::clone() const
{
    RGBText* txt = new RGBText(*this);
    return static_cast<RGBAlgorithm*> (txt);
}

/****************************************************************************
 * Text & Font
 ****************************************************************************/

void RGBText::setText(const QString& str)
{
    m_text = str;
}

QString RGBText::text() const
{
    return m_text;
}

void RGBText::setFont(const QFont& font)
{
    m_font = font;
}

QFont RGBText::font() const
{
    return m_font;
}

/****************************************************************************
 * Animation
 ****************************************************************************/

void RGBText::setAnimationStyle(RGBText::AnimationStyle ani)
{
    m_animationStyle = ani;
}

RGBText::AnimationStyle RGBText::animationStyle() const
{
    return m_animationStyle;
}

QString RGBText::animationStyleToString(RGBText::AnimationStyle ani)
{
    switch (ani)
    {
    default:
    case StaticLetters:
        return QString("Letters");
    case Horizontal:
        return QString("Horizontal");
    case Vertical:
        return QString("Vertical");
    }
}

RGBText::AnimationStyle RGBText::stringToAnimationStyle(const QString& str)
{
    if (str == QString("Horizontal"))
        return Horizontal;
    else if (str == QString("Vertical"))
        return Vertical;
    else
        return StaticLetters;
}

QStringList RGBText::animationStyles()
{
    QStringList list;
    list << animationStyleToString(StaticLetters);
    list << animationStyleToString(Horizontal);
    list << animationStyleToString(Vertical);
    return list;
}

void RGBText::setXOffset(int offset)
{
    m_xOffset = offset;
}

int RGBText::xOffset() const
{
    return m_xOffset;
}

void RGBText::setYOffset(int offset)
{
    m_yOffset = offset;
}

int RGBText::yOffset() const
{
    return m_yOffset;
}

int RGBText::scrollingTextSteps(const QSize& size) const
{
    // Estimate the text length in pixels. Available matrix size doesn't matter.
    Q_UNUSED(size);

    QFontMetrics fm(m_font);
    if (animationStyle() == Vertical)
        return m_text.length() * fm.ascent();
    else
        return fm.width(m_text);
}

RGBMap RGBText::renderScrollingText(const QSize& size, uint rgb, int step) const
{
    QImage image;

    if (animationStyle() == Horizontal)
        image = QImage(scrollingTextSteps(size), size.height(), QImage::Format_RGB32);
    else
        image = QImage(size.width(), scrollingTextSteps(size), QImage::Format_RGB32);
    image.fill(0);

    QPainter p(&image);
    p.setRenderHint(QPainter::TextAntialiasing, false);
    p.setRenderHint(QPainter::Antialiasing, false);
    p.setFont(m_font);
    p.setPen(QColor(rgb));

    if (animationStyle() == Vertical)
    {
        QFontMetrics fm(m_font);
        QRect rect(0, 0, image.width(), image.height());

        for (int i = 0; i < m_text.length(); i++)
        {
            rect.setY((i * fm.ascent()) + yOffset());
            rect.setX(xOffset());
            rect.setHeight(fm.ascent());
            p.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, m_text.mid(i, 1));
        }
    }
    else
    {
        // Draw the whole text each time
        QRect rect(xOffset(), yOffset(), image.width(), image.height());
        p.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, m_text);
    }
    p.end();

    // Treat the RGBMap as a "window" on top of the fully-drawn text and pick the
    // correct pixels according to $step.
    RGBMap map(size.height());
    for (int y = 0; y < size.height(); y++)
    {
        map[y].resize(size.width());
        for (int x = 0; x < size.width(); x++)
        {
            if (animationStyle() == Horizontal)
            {
                if (step + x >= image.width())
                    map[y][x] = QRgb(0);
                else
                    map[y][x] = image.pixel(step + x, y);
            }
            else
            {
                if (step + y >= image.height())
                    map[y][x] = QRgb(0);
                else
                    map[y][x] = image.pixel(x, step + y);
            }
        }
    }

    return map;
}

RGBMap RGBText::renderStaticLetters(const QSize& size, uint rgb, int step) const
{
    QImage image(size, QImage::Format_RGB32);
    image.fill(0);

    QPainter p(&image);
    p.setRenderHint(QPainter::TextAntialiasing, false);
    p.setRenderHint(QPainter::Antialiasing, false);
    p.setFont(m_font);

    p.setPen(QColor(rgb));
    // Draw one letter at a time
    QRect rect(xOffset(), yOffset(), size.width(), size.height());
    p.drawText(rect, Qt::AlignCenter, m_text.mid(step, 1));
    p.end();

    RGBMap map(size.height());
    for (int y = 0; y < size.height(); y++)
    {
        map[y].resize(size.width());
        for (int x = 0; x < size.width(); x++)
            map[y][x] = image.pixel(x, y);
    }

    return map;
}

/****************************************************************************
 * RGBAlgorithm
 ****************************************************************************/

int RGBText::rgbMapStepCount(const QSize& size)
{
    if (animationStyle() == StaticLetters)
        return m_text.length();
    else
        return scrollingTextSteps(size);
}

RGBMap RGBText::rgbMap(const QSize& size, uint rgb, int step)
{
    if (animationStyle() == StaticLetters)
        return renderStaticLetters(size, rgb, step);
    else
        return renderScrollingText(size, rgb, step);
}

QString RGBText::name() const
{
    return QString("Text");
}

QString RGBText::author() const
{
    return QString("Heikki Junnila");
}

int RGBText::apiVersion() const
{
    return 1;
}

RGBAlgorithm::Type RGBText::type() const
{
    return RGBAlgorithm::Text;
}

bool RGBText::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCRGBAlgorithm)
    {
        qWarning() << Q_FUNC_INFO << "RGB Algorithm node not found";
        return false;
    }

    if (root.attribute(KXMLQLCRGBAlgorithmType) != KXMLQLCRGBText)
    {
        qWarning() << Q_FUNC_INFO << "RGB Algorithm is not Text";
        return false;
    }

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCRGBTextContent)
        {
            setText(tag.text());
        }
        else if (tag.tagName() == KXMLQLCRGBTextFont)
        {
            QFont font;
            if (font.fromString(tag.text()) == true)
                setFont(font);
            else
                qWarning() << Q_FUNC_INFO << "Invalid font:" << tag.text();
        }
        else if (tag.tagName() == KXMLQLCRGBTextAnimationStyle)
        {
            setAnimationStyle(stringToAnimationStyle(tag.text()));
        }

        node = node.nextSibling();
    }

    return true;
}

bool RGBText::saveXML(QDomDocument* doc, QDomElement* mtx_root) const
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(mtx_root != NULL);

    QDomElement root = doc->createElement(KXMLQLCRGBAlgorithm);
    root.setAttribute(KXMLQLCRGBAlgorithmType, KXMLQLCRGBText);
    mtx_root->appendChild(root);

    QDomElement content = doc->createElement(KXMLQLCRGBTextContent);
    QDomText contentText = doc->createTextNode(m_text);
    content.appendChild(contentText);
    root.appendChild(content);

    QDomElement font = doc->createElement(KXMLQLCRGBTextFont);
    QDomText fontText = doc->createTextNode(m_font.toString());
    font.appendChild(fontText);
    root.appendChild(font);

    QDomElement ani = doc->createElement(KXMLQLCRGBTextAnimationStyle);
    QDomText aniText = doc->createTextNode(animationStyleToString(animationStyle()));
    ani.appendChild(aniText);
    root.appendChild(ani);

    return true;
}
