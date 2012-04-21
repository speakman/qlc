/*
  Q Light Controller
  inputmap.cpp

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

#ifdef WIN32
#	include <Windows.h>
#endif

#include <QCoreApplication>
#include <QPluginLoader>
#include <QStringList>
#include <QSettings>
#include <QDebug>
#include <QList>
#include <QtXml>
#include <QDir>

#include "qlcinputchannel.h"
#include "hotplugmonitor.h"
#include "qlcinputsource.h"
#include "qlcioplugin.h"
#include "inputpatch.h"
#include "qlcconfig.h"
#include "inputmap.h"
#include "qlcfile.h"
#include "qlci18n.h"
#include "doc.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

InputMap::InputMap(Doc* doc, quint32 universes)
    : QObject(doc)
    , m_universes(universes)
    , m_editorUniverse(0)
{
    initPatch();

    connect(doc->ioPluginCache(), SIGNAL(pluginConfigurationChanged(QLCIOPlugin*)),
            this, SLOT(slotPluginConfigurationChanged(QLCIOPlugin*)));
}

InputMap::~InputMap()
{
    /* Clear patching table so that when it gets out of scope AFTER this
       destructor is run, it won't attempt to do close() on already-deleted
       plugin pointers. */
    for (quint32 i = 0; i < m_universes; i++)
    {
        delete m_patch[i];
        m_patch[i] = NULL;
    }

    while (m_profiles.isEmpty() == false)
        delete m_profiles.takeFirst();
}

Doc* InputMap::doc() const
{
    return qobject_cast<Doc*> (parent());
}

/*****************************************************************************
 * Universes
 *****************************************************************************/

quint32 InputMap::invalidUniverse()
{
    return UINT_MAX;
}

quint32 InputMap::universes() const
{
    return m_universes;
}

quint32 InputMap::editorUniverse() const
{
    return m_editorUniverse;
}

void InputMap::setEditorUniverse(quint32 uni)
{
    if (uni < universes())
        m_editorUniverse = uni;
    else
        m_editorUniverse = 0;
}

quint32 InputMap::invalidChannel()
{
    return UINT_MAX;
}

/*****************************************************************************
 * Input data
 *****************************************************************************/

bool InputMap::feedBack(quint32 universe, quint32 channel, uchar value)
{
    if (universe >= quint32(m_patch.size()))
        return false;

    InputPatch* patch = m_patch[universe];
    Q_ASSERT(patch != NULL);

    if (patch->plugin() != NULL && patch->feedbackEnabled() == true)
    {
        //! @todo Feedback
        //patch->plugin()->sendFeedBack(patch->input(), channel, value);
        Q_UNUSED(channel);
        Q_UNUSED(value);
        return true;
    }
    else
    {
        return false;
    }
}

void InputMap::slotPluginConfigurationChanged(QLCIOPlugin* plugin)
{
    for (quint32 i = 0; i < universes(); i++)
    {
        InputPatch* ip = patch(i);
        Q_ASSERT(ip != NULL);
        if (ip->plugin() == plugin)
            ip->reconnect();
    }

    emit pluginConfigurationChanged(plugin->name());
}

/*****************************************************************************
 * Patch
 *****************************************************************************/

void InputMap::initPatch()
{
    for (quint32 i = 0; i < m_universes; i++)
    {
        InputPatch* patch = new InputPatch(i, this);
        m_patch.insert(i, patch);
        connect(patch, SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SIGNAL(inputValueChanged(quint32,quint32,uchar)));
    }
}

bool InputMap::setPatch(quint32 universe, const QString& pluginName,
                        quint32 input, bool enableFeedback,
                        const QString& profileName)
{
    /* Check that the universe that we're doing mapping for is valid */
    if (universe >= m_universes)
    {
        qWarning() << Q_FUNC_INFO << "Universe" << universe << "out of bounds.";
        return false;
    }

    /* Don't care if plugin or profile is NULL. It must be possible to
       clear the patch completely. */
    m_patch[universe]->set(doc()->ioPluginCache()->plugin(pluginName), input,
                           enableFeedback, profile(profileName));

    return true;
}

InputPatch* InputMap::patch(quint32 universe) const
{
    if (universe < m_universes)
        return m_patch[universe];
    else
        return NULL;
}

quint32 InputMap::mapping(const QString& pluginName, quint32 input) const
{
    for (quint32 uni = 0; uni < universes(); uni++)
    {
        const InputPatch* p = patch(uni);
        if (p->pluginName() == pluginName && p->input() == input)
            return uni;
    }

    return InputMap::invalidUniverse();
}

/*****************************************************************************
 * Plugins
 *****************************************************************************/

QStringList InputMap::pluginNames()
{
    QListIterator <QLCIOPlugin*> it(doc()->ioPluginCache()->plugins());
    QStringList list;

    while (it.hasNext() == true)
        list.append(it.next()->name());

    return list;
}

QStringList InputMap::pluginInputs(const QString& pluginName)
{
    QLCIOPlugin* ip = doc()->ioPluginCache()->plugin(pluginName);
    if (ip == NULL)
        return QStringList();
    else
        return ip->inputs();
}

void InputMap::configurePlugin(const QString& pluginName)
{
    QLCIOPlugin* inputPlugin = doc()->ioPluginCache()->plugin(pluginName);
    if (inputPlugin != NULL)
        inputPlugin->configure();
}

bool InputMap::canConfigurePlugin(const QString& pluginName)
{
    QLCIOPlugin* inputPlugin = doc()->ioPluginCache()->plugin(pluginName);
    if (inputPlugin != NULL)
        return inputPlugin->canConfigure();
    else
        return false;
}

QString InputMap::pluginStatus(const QString& pluginName, quint32 input)
{
    QLCIOPlugin* inputPlugin = NULL;
    QString info;

    if (pluginName.isEmpty() == false)
        inputPlugin = doc()->ioPluginCache()->plugin(pluginName);

    if (inputPlugin != NULL)
    {
        info = inputPlugin->inputInfo(input);
    }
    else
    {
        /* Nothing selected */
        info += QString("<HTML><HEAD></HEAD><BODY>");
        info += QString("<H3>%1</H3>").arg(tr("Nothing selected"));
        info += QString("</BODY></HTML>");
    }

    return info;
}

/*****************************************************************************
 * Profiles
 *****************************************************************************/

void InputMap::loadProfiles(const QDir& dir)
{
    if (dir.exists() == false || dir.isReadable() == false)
        return;

    /* Go thru all found file entries and attempt to load an input
       profile from each of them. */
    QStringListIterator it(dir.entryList());
    while (it.hasNext() == true)
    {
        QLCInputProfile* prof;
        QString path;

        path = dir.absoluteFilePath(it.next());
        prof = QLCInputProfile::loader(path);
        if (prof != NULL)
        {
            /* Check for duplicates */
            if (profile(prof->name()) == NULL)
                addProfile(prof);
            else
                delete prof;
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unable to find an input profile from" << path;
        }
    }
}

QStringList InputMap::profileNames()
{
    QStringList list;
    QListIterator <QLCInputProfile*> it(m_profiles);
    while (it.hasNext() == true)
        list << it.next()->name();
    return list;
}

QLCInputProfile* InputMap::profile(const QString& name)
{
    QListIterator <QLCInputProfile*> it(m_profiles);
    while (it.hasNext() == true)
    {
        QLCInputProfile* profile = it.next();
        if (profile->name() == name)
            return profile;
    }

    return NULL;
}

bool InputMap::addProfile(QLCInputProfile* profile)
{
    Q_ASSERT(profile != NULL);

    /* Don't add the same profile twice */
    if (m_profiles.contains(profile) == false)
    {
        m_profiles.append(profile);
        return true;
    }
    else
    {
        return false;
    }
}

bool InputMap::removeProfile(const QString& name)
{
    QLCInputProfile* profile;
    QMutableListIterator <QLCInputProfile*> it(m_profiles);
    while (it.hasNext() == true)
    {
        profile = it.next();
        if (profile->name() == name)
        {
            it.remove();
            delete profile;
            return true;
        }
    }

    return false;
}

bool InputMap::inputSourceNames(const QLCInputSource& src,
                                QString& uniName, QString& chName) const
{
    if (src.isValid() == false)
        return false;

    InputPatch* pat = this->patch(src.universe());
    if (pat == NULL || pat->plugin() == NULL)
    {
        /* There is no patch for the given universe */
        return false;
    }

    QLCInputProfile* profile = pat->profile();
    if (profile == NULL)
    {
        /* There is no profile. Display plugin name and channel number. */
        uniName = QString("%1: %2").arg(src.universe() + 1).arg(pat->plugin()->name());
        chName = QString("%1: ?").arg(src.channel() + 1);
    }
    else
    {
        QLCInputChannel* ich;
        QString name;

        /* Display profile name for universe */
        uniName = QString("%1: %2").arg(src.universe() + 1).arg(profile->name());

        /* User can input the channel number by hand, so put something
           rational to the channel name in those cases as well. */
        ich = profile->channel(src.channel());
        if (ich != NULL)
            name = ich->name();
        else
            name = QString("?");

        /* Display channel name */
        chName = QString("%1: %2").arg(src.channel() + 1).arg(name);
    }

    return true;
}

QDir InputMap::systemProfileDirectory()
{
    QDir dir;

#ifdef __APPLE__
    dir.setPath(QString("%1/../%2").arg(QCoreApplication::applicationDirPath())
                              .arg(INPUTPROFILEDIR));
#else
    dir.setPath(INPUTPROFILEDIR);
#endif

    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtInputProfile));
    return dir;
}

QDir InputMap::userProfileDirectory()
{
    QDir dir;

#ifdef Q_WS_X11
    // If the current user is root, return the system profile dir.
    // Otherwise return the user's home dir.
    if (geteuid() == 0)
        dir = QDir(INPUTPROFILEDIR);
    else
        dir.setPath(QString("%1/%2").arg(getenv("HOME")).arg(USERINPUTPROFILEDIR));
#elif __APPLE__
    /* User's input profile directory on OSX */
    dir.setPath(QString("%1/%2").arg(getenv("HOME")).arg(USERINPUTPROFILEDIR));
#else
    /* User's input profile directory on Windows */
    LPTSTR home = (LPTSTR) malloc(256 * sizeof(TCHAR));
    GetEnvironmentVariable(TEXT("UserProfile"), home, 256);
    dir.setPath(QString("%1/%2")
                    .arg(QString::fromUtf16(reinterpret_cast<ushort*> (home)))
                    .arg(USERINPUTPROFILEDIR));
    free(home);
#endif

    /* Ensure that the selected profile directory exists */
    if (dir.exists() == false)
        dir.mkpath(".");

    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtInputProfile));
    return dir;
}

/*****************************************************************************
 * Defaults
 *****************************************************************************/

void InputMap::loadDefaults()
{
    bool feedbackEnabled;
    QString profileName;
    QSettings settings;
    QString plugin;
    QVariant value;
    QString input;
    QString key;

    /* Editor universe */
    key = QString("/inputmap/editoruniverse/");
    value = settings.value(key);
    if (value.isValid() == true)
        setEditorUniverse(value.toInt());

    for (quint32 i = 0; i < m_universes; i++)
    {
        /* Plugin name */
        key = QString("/inputmap/universe%2/plugin/").arg(i);
        plugin = settings.value(key).toString();

        /* Plugin input */
        key = QString("/inputmap/universe%2/input/").arg(i);
        input = settings.value(key).toString();

        /* Input profile */
        key = QString("/inputmap/universe%2/profile/").arg(i);
        profileName = settings.value(key).toString();

        /* Feedback enable */
        key = QString("/inputmap/universe%2/feedbackEnabled/").arg(i);
        if (settings.value(key).isValid() == true)
            feedbackEnabled = settings.value(key).toBool();
        else
            feedbackEnabled = true;

        /* Do the mapping */
        if (plugin.length() > 0 && input.length() > 0)
        {
            /* Check that the same plugin & input are not mapped
               to more than one universe at a time. */
            quint32 m = mapping(plugin, input.toInt());
            if (m == InputMap::invalidUniverse() || m == i)
            {
                setPatch(i, plugin, input.toInt(),
                         feedbackEnabled, profileName);
            }
        }
    }
}

void InputMap::saveDefaults()
{
    QSettings settings;
    QString key;
    QString str;

    for (quint32 i = 0; i < m_universes; i++)
    {
        InputPatch* pat = patch(i);
        Q_ASSERT(pat != NULL);

        /* Editor universe */
        key = QString("/inputmap/editoruniverse/");
        settings.setValue(key, m_editorUniverse);

        if (pat->plugin() != NULL)
        {
            /* Plugin name */
            key = QString("/inputmap/universe%2/plugin/").arg(i);
            settings.setValue(key, pat->plugin()->name());

            /* Plugin input */
            key = QString("/inputmap/universe%2/input/").arg(i);
            settings.setValue(key, str.setNum(pat->input()));

            /* Input profile */
            key = QString("/inputmap/universe%2/profile/").arg(i);
            settings.setValue(key, pat->profileName());

            /* Feedback enable */
            key = QString("/inputmap/universe%2/feedbackEnabled/").arg(i);
            settings.setValue(key, pat->feedbackEnabled());
        }
        else
        {
            /* Plugin name */
            key = QString("/inputmap/universe%2/plugin/").arg(i);
            settings.setValue(key, "");

            /* Plugin input */
            key = QString("/inputmap/universe%2/input/").arg(i);
            settings.setValue(key, "");

            /* Input profile */
            key = QString("/inputmap/universe%2/profile/").arg(i);
            settings.setValue(key, "");

            /* Feedback enable */
            key = QString("/inputmap/universe%2/feedbackEnabled/").arg(i);
            settings.setValue(key, true);
        }
    }
}
