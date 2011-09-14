/*
  Q Light Controller
  script.cpp

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

#include <QDomDocument>
#include <QDomElement>
#include <QDomText>
#include <QDebug>
#include <QUrl>

#include "universearray.h"
#include "mastertimer.h"
#include "qlcmacros.h"
#include "script.h"
#include "doc.h"

#define KXMLQLCScriptContents "Contents"

const QString Script::startFunctionCmd = QString("startfunction");
const QString Script::stopFunctionCmd = QString("stopfunction");

const QString Script::waitCmd = QString("wait");
const QString Script::waitKeyCmd = QString("waitkey");

const QString Script::setHtpCmd = QString("sethtp");
const QString Script::setLtpCmd = QString("setltp");
const QString Script::setFixtureCmd = QString("setfixture");

const QString Script::labelCmd = QString("label");
const QString Script::jumpCmd = QString("jump");

/****************************************************************************
 * Initialization
 ****************************************************************************/

Script::Script(Doc* doc) : Function(doc)
    , m_stopOwnfunctionsAtEnd(false)
    , m_currentCommand(0)
    , m_waitCount(0)
{
    setName(tr("New Script"));
}

Script::~Script()
{
}

Function::Type Script::type() const
{
    return Function::Script;
}

Function* Script::createCopy(Doc* doc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new Script(doc);
    if (copy->copyFrom(this) == false || doc->addFunction(copy) == false)
    {
        delete copy;
        copy = NULL;
    }

    return copy;
}

bool Script::copyFrom(const Function* function)
{
    const Script* script = qobject_cast<const Script*> (function);
    if (script == NULL)
        return false;

    setData(script->data());

    return Function::copyFrom(function);
}

/****************************************************************************
 * Script data
 ****************************************************************************/

bool Script::setData(const QString& str)
{
    m_data = str;
    return true;
}

QString Script::data() const
{
    return m_data;
}

void Script::setStopOwnFunctionsAtEnd(bool stop)
{
    m_stopOwnfunctionsAtEnd = stop;
}

bool Script::stopOwnFunctionsAtEnd() const
{
    return m_stopOwnfunctionsAtEnd;
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool Script::loadXML(const QDomElement* root)
{
    QDomNode node;
    QDomElement tag;

    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root->attribute(KXMLQLCFunctionType) != typeToString(Function::Script))
    {
        qWarning() << Q_FUNC_INFO << root->attribute(KXMLQLCFunctionType)
                   << "is not a script";
        return false;
    }

    /* Load script contents */
    node = root->firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();

        if (tag.tagName() == KXMLQLCBus)
        {
            /* Bus */
            setBus(tag.text().toUInt());
        }
        else if (tag.tagName() == KXMLQLCFunctionDirection)
        {
            /* Direction */
            setDirection(Function::stringToDirection(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCFunctionRunOrder)
        {
            /* Run Order */
            setRunOrder(Function::stringToRunOrder(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCScriptContents)
        {
            setData(QUrl::fromPercentEncoding(tag.text().toUtf8()));
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown script tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool Script::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    QDomElement root;
    QDomElement tag;
    QDomText text;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    /* Function tag */
    root = doc->createElement(KXMLQLCFunction);
    wksp_root->appendChild(root);

    root.setAttribute(KXMLQLCFunctionID, id());
    root.setAttribute(KXMLQLCFunctionType, Function::typeToString(type()));
    root.setAttribute(KXMLQLCFunctionName, name());

    /* Speed bus */
    tag = doc->createElement(KXMLQLCBus);
    root.appendChild(tag);
    tag.setAttribute(KXMLQLCBusRole, KXMLQLCBusFade);
    text = doc->createTextNode(QString::number(busID()));
    tag.appendChild(text);

    /* Direction */
    tag = doc->createElement(KXMLQLCFunctionDirection);
    root.appendChild(tag);
    text = doc->createTextNode(Function::directionToString(m_direction));
    tag.appendChild(text);

    /* Run order */
    tag = doc->createElement(KXMLQLCFunctionRunOrder);
    root.appendChild(tag);
    text = doc->createTextNode(Function::runOrderToString(m_runOrder));
    tag.appendChild(text);

    /* Contents */
    tag = doc->createElement(KXMLQLCScriptContents);
    root.appendChild(tag);
    text = doc->createTextNode(QUrl::toPercentEncoding(m_data));
    tag.appendChild(text);

    return true;
}

/****************************************************************************
 * Running
 ****************************************************************************/

void Script::arm()
{
    m_labels.clear();
    m_lines.clear();

    // Construct individual code lines from the data
    if (m_data.isEmpty() == false)
    {
        QStringList lines = m_data.split(QRegExp("(\r\n|\n\r|\r|\n)"), QString::KeepEmptyParts);
        foreach (QString line, lines)
            m_lines << tokenizeLine(line + QString("\n"));
    }

    // Map all labels to their individual line numbers for fast jumps
    for (int i = 0; i < m_lines.size(); i++)
    {
        QStringList tokens = m_lines[i];
        if (tokens.isEmpty() == false &&
            tokens[0].simplified().startsWith(Script::labelCmd) == true)
        {
            QStringList command = tokens[0].split(":");
            if (command.size() == 2)
                m_labels[command[1]] = i;
        }
    }
}

void Script::disarm()
{
    // Waste not
    m_lines.clear();
    m_labels.clear();
}

void Script::preRun(MasterTimer* timer)
{
    m_waitCount = 0;
    m_currentCommand = 0;
    m_startedFunctions.clear();

    Function::preRun(timer);
}

void Script::write(MasterTimer* timer, UniverseArray* universes)
{
    incrementElapsed();

    if (stopped() == false && waiting() == false)
    {
        while (m_currentCommand < m_lines.size() && stopped() == false)
        {
            bool continueLoop = executeCommand(m_currentCommand, timer, universes);
            m_currentCommand++;
            if (continueLoop == false)
                break;
        }

        // In case wait() is the last command, don't stop the script prematurely
        if (m_currentCommand >= m_lines.size() && m_waitCount == 0)
            stop();
    }
}

void Script::postRun(MasterTimer* timer, UniverseArray* universes)
{
    if (m_stopOwnfunctionsAtEnd == true)
    {
        // Stop all functions started by this script
        foreach (Function* function, m_startedFunctions)
            function->stop();
        m_startedFunctions.clear();
    }

    Function::postRun(timer, universes);
}

bool Script::waiting()
{
    if (m_waitCount > 0)
    {
        m_waitCount--;
        return true;
    }
    else
    {
        return false;
    }
}

bool Script::executeCommand(int index, MasterTimer* timer, UniverseArray* universes)
{
    if (index < 0 || index >= m_lines.size())
    {
        qWarning() << "Invalid command index:" << index;
        return false;
    }

    QStringList tokens = m_lines[index];
    if (tokens.isEmpty() == true)
        return true;

    bool continueLoop = true;
    QString error;
    QStringList command = tokens[0].split(":");
    if (command.size() < 2)
    {
        error = QString("Syntax error");
    }
    else if (command[0] == Script::startFunctionCmd)
    {
        error = handleStartFunction(command, timer);
    }
    else if (command[0] == Script::stopFunctionCmd)
    {
        error = handleStopFunction(command);
    }
    else if (command[0] == Script::waitCmd)
    {
        error = handleWait(command);
    }
    else if (command[0] == Script::waitKeyCmd)
    {
        error = handleWaitKey(command);
    }
    else if (command[0] == Script::setHtpCmd || command[0] == Script::setLtpCmd)
    {
        error = handleSetHtpLtp(command, tokens, universes);
    }
    else if (command[0] == Script::setFixtureCmd)
    {
        error = handleSetFixture(command, tokens, universes);
    }
    else if (command[0] == Script::labelCmd)
    {
        error = handleLabel(command);
    }
    else if (command[0] == Script::jumpCmd)
    {
        // Jumping can cause an infinite non-waiting loop, causing starvation
        // among other functions. Therefore, the script must relinquish its
        // time slot after each jump. If there is no error in jumping, the jump
        // must have happened.
        error = handleJump(command);
        if (error.isEmpty() == true)
            continueLoop = false;
    }
    else
    {
        error = QString("Unknown command: %1").arg(command[0]);
    }

    if (error.isEmpty() == false)
        qWarning() << QString("%1: %2: %3").arg(name()).arg(index).arg(error);

    return continueLoop;
}

QString Script::handleStartFunction(const QStringList& command, MasterTimer* timer)
{
    qDebug() << Q_FUNC_INFO;

    bool ok = false;
    quint32 id = command[1].toUInt(&ok);
    if (ok == false)
        return QString("Invalid function ID: %1").arg(command[1]);

    Doc* doc = qobject_cast<Doc*> (parent());
    Q_ASSERT(doc != NULL);

    Function* function = doc->function(id);
    if (function != NULL)
    {
        if (function->stopped() == true)
            timer->startFunction(function, true);
        else
            qWarning() << "Function (" << function->name() << ") is already running.";

        m_startedFunctions << function;
        return QString();
    }
    else
    {
        return QString("No such function (ID %1)").arg(id);
    }
}

QString Script::handleStopFunction(const QStringList& command)
{
    qDebug() << Q_FUNC_INFO;

    bool ok = false;
    quint32 id = command[1].toUInt(&ok);
    if (ok == false)
        return QString("Invalid function ID: %1").arg(command[1]);

    Doc* doc = qobject_cast<Doc*> (parent());
    Q_ASSERT(doc != NULL);

    Function* function = doc->function(id);
    if (function != NULL)
    {
        if (function->stopped() == false)
            function->stop();
        else
            qWarning() << "Function (" << function->name() << ") is not running.";

        m_startedFunctions.removeAll(function);
        return QString();
    }
    else
    {
        return QString("No such function (ID %1)").arg(id);
    }
}

QString Script::handleWait(const QStringList& command)
{
    qDebug() << Q_FUNC_INFO;

    double time = 0;
    bool ok = false;

    time = command[1].toDouble(&ok);
    if (ok == false)
        return QString("Invalid wait time: %1").arg(command[1]);

    m_waitCount = time * MasterTimer::frequency();

    return QString();
}

QString Script::handleWaitKey(const QStringList& command)
{
    qDebug() << Q_FUNC_INFO;

    Q_UNUSED(command);
    return QString("Not implemented yet");
}

QString Script::handleSetHtpLtp(const QStringList& command, const QStringList& tokens,
                                UniverseArray* universes)
{
    qDebug() << Q_FUNC_INFO;

    bool ok = false;
    int addr = 0;
    int uni = 1;
    uchar value = 0;

    addr = command[1].toUInt(&ok);
    if (ok == false)
        return QString("Invalid address: %1").arg(command[1]);

    for (int tok = 1; tok < tokens.size(); tok++)
    {
        QStringList list = tokens[tok].split(":", QString::SkipEmptyParts);
        list[0] = list[0].toLower().trimmed();
        if (list.size() == 2)
        {
            ok = false;
            if (list[0] == "val" || list[0] == "value")
                value = uchar(list[1].toUInt(&ok));
            else if (list[0] == "uni" || list[0] == "universe")
                uni = list[1].toUInt(&ok);
            else
                return QString("Unrecognized keyword: %1").arg(list[0]);

            if (ok == false)
                return QString("Invalid value (%1) for keyword: %2").arg(list[1]).arg(list[0]);
        }
    }

    // So is this LTP or HTP?
    QLCChannel::Group group;
    if (command[0] == Script::setHtpCmd)
        group = QLCChannel::Intensity;
    else
        group = QLCChannel::NoGroup;

    int channel = (uni - 1) * 512;
    channel += addr - 1;
    if (channel >= 0 && channel < universes->size())
    {
        universes->write(channel, value, group);
        return QString();
    }
    else
    {
        return QString("Invalid address: %1 or universe: %2").arg(addr).arg(uni);
    }
}

QString Script::handleSetFixture(const QStringList& command, const QStringList& tokens,
                                 UniverseArray* universes)
{
    qDebug() << Q_FUNC_INFO;

    bool ok = false;
    quint32 id = 0;
    quint32 ch = 0;
    uchar value = 0;

    id = command[1].toUInt(&ok);
    if (ok == false)
        return QString("Invalid fixture (ID: %1)").arg(command[1]);

    for (int tok = 1; tok < tokens.size(); tok++)
    {
        QStringList list = tokens[tok].split(":", QString::SkipEmptyParts);
        list[0] = list[0].toLower().trimmed();
        if (list.size() == 2)
        {
            ok = false;
            if (list[0] == "val" || list[0] == "value")
                value = uchar(list[1].toUInt(&ok));
            else if (list[0] == "ch" || list[0] == "channel")
                ch = list[1].toUInt(&ok) - 1;
            else
                return QString("Unrecognized keyword: %1").arg(list[0]);

            if (ok == false)
                return QString("Invalid value (%1) for keyword: %2").arg(list[1]).arg(list[0]);
        }
    }

    Doc* doc = qobject_cast<Doc*> (parent());
    Q_ASSERT(doc != NULL);

    Fixture* fxi = doc->fixture(id);
    if (fxi != NULL)
    {
        if (ch < fxi->channels())
        {
            const QLCChannel* channel = fxi->channel(ch);
            Q_ASSERT(channel != NULL);

            int address = fxi->universeAddress() + ch;
            if (address < universes->size())
            {
                universes->write(address, value, channel->group());
                return QString();
            }
            else
            {
                return QString("Invalid address: %1").arg(address);
            }
        }
        else
        {
            return QString("Fixture (%1) has no channel number %2").arg(fxi->name()).arg(ch);
        }
    }
    else
    {
        return QString("No such fixture (ID: %1)").arg(id);
    }
}

QString Script::handleLabel(const QStringList& command)
{
    qDebug() << Q_FUNC_INFO;
    Q_UNUSED(command);
    return QString();
}

QString Script::handleJump(const QStringList& command)
{
    qDebug() << Q_FUNC_INFO;

    if (m_labels.contains(command[1]) == true)
    {
        int lineNumber = m_labels[command[1]];
        Q_ASSERT(lineNumber >= 0 && lineNumber < m_lines.size());
        m_currentCommand = lineNumber;
        return QString();
    }
    else
    {
        return QString("No such label: %1").arg(command[1]);
    }
}

QStringList Script::tokenizeLine(const QString& str, bool* ok)
{
    QStringList tokens;
    int left = 0;
    int right = 0;
    QString keyword;
    QString value;

    if (str.simplified().startsWith("//") == true || str.simplified().isEmpty() == true)
    {
        tokens = QStringList(); // Return an empty string list for commented lines
    }
    else
    {
        // Truncate everything after the first comment sign
        QString line = str;
        left = line.indexOf("//");
        if (left != -1)
            line.truncate(left);

        left = 0;
        while (left < line.length())
        {
            // Find the next colon to get the keyword
            right = line.indexOf(":", left);
            if (right == -1)
            {
                qDebug() << "Syntax error:" << line.mid(left);
                if (ok != NULL)
                    *ok = false;
                break;
            }
            else
            {
                // Keyword found
                keyword = line.mid(left, right - left);
                left = right + 1;
            }

            // Find the next whitespace to get the value
            right = line.indexOf(QRegExp("\\s"), left);
            if (right == -1)
            {
                qDebug() << "Syntax error:" << line.mid(left);
                if (ok != NULL)
                    *ok = false;
                break;
            }
            else
            {
                // Value found
                value = line.mid(left, right - left);
                left = right + 1;
            }

            tokens << QString(keyword + ":" + value);
        }
    }

    if (ok != NULL)
        *ok = true;

    return tokens;
}
