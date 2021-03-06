/*
  Q Light Controller
  doc.h

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

#ifndef DOC_H
#define DOC_H

#include <QObject>
#include <QList>
#include <QFile>
#include <QMap>

#include "qlcfixturedefcache.h"
#include "ioplugincache.h"
#include "fixturegroup.h"
#include "mastertimer.h"
#include "outputmap.h"
#include "inputmap.h"
#include "function.h"
#include "fixture.h"

class QDomDocument;
class QString;

#define KXMLQLCEngine "Engine"

class Doc : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Doc)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /**
     * Create a new Doc instance for the given parent.
     *
     * @param parent The parent object who owns the Doc instance
     * @param outputUniverses Number of output (DMX) universes
     * @param inputUniverses Number of input universes
     */
    Doc(QObject* parent, int outputUniverses = 4, int inputUniverses = 4);

    /** Destructor */
    ~Doc();

    /** Remove all functions and fixtures from the doc, signalling each removal. */
    void clearContents();

signals:
    /** Emitted when clearContents() is called, before actually doing anything. */
    void clearing();

    /** Emitted when clearContents() has finished. */
    void cleared();

    /*********************************************************************
     * Engine components
     *********************************************************************/
public:
    /** Get the fixture definition cache object */
    QLCFixtureDefCache* fixtureDefCache() const;

    /** Get the I/O plugin cache object */
    IOPluginCache* ioPluginCache() const;

    /** Get the DMX output map object */
    OutputMap* outputMap() const;

    /** Get the MasterTimer object that runs the show */
    MasterTimer* masterTimer() const;

    /** Get the input map object */
    InputMap* inputMap() const;

private:
    QLCFixtureDefCache* m_fixtureDefCache;
    IOPluginCache* m_ioPluginCache;
    OutputMap* m_outputMap;
    MasterTimer* m_masterTimer;
    InputMap* m_inputMap;

    /*********************************************************************
     * Main operating mode
     *********************************************************************/
public:
    enum Mode
    {
        Design  = 0, //! Editing allowed
        Operate = 1  //! Running allowed, editing disabled
    };

    /** Change the main operating mode. See enum Mode for more information. */
    void setMode(Mode mode);

    /** Get the main operating mode. See enum Mode for more information. */
    Mode mode() const;

    /**
     * Enable/disable kiosk mode. This doesn't do practically anything by itself.
     * It's mostly a convenient helper for engine components to detect if kiosk
     * mode is on.
     */
    void setKiosk(bool kiosk);

    /** Check, if kiosk mode is enabled or not. */
    bool isKiosk() const;

signals:
    /** Tells that the current operating mode has changed */
    void modeChanged(Doc::Mode mode);

protected:
    Mode m_mode;
    bool m_kiosk;

    /*********************************************************************
     * Modified status
     *********************************************************************/
public:
    /**
     * Check, whether Doc has been modified (and is in need of saving)
     */
    bool isModified() const;

    /**
     * Set Doc into modified state (i.e. it is in need of saving)
     */
    void setModified();

    /**
     * Reset Doc's modified state (i.e. it is no longer in need of saving)
     */
    void resetModified();

signals:
    /** Signal that this Doc has been modified (or unmodified) */
    void modified(bool state);

protected:
    /** Modified status (true; needs saving, false; does not) */
    bool m_modified;

    /*********************************************************************
     * Fixture Instances
     *********************************************************************/
public:
    /**
     * Add the given fixture to doc's fixture array.
     * If id == Fixture::invalidId(), doc assigns the fixture a new ID and
     * takes ownership, unless there is no more room for more fixtures.
     *
     * If id != Fixture::invalidId(), doc attempts to put the fixture at
     * that exact index, unless another fixture already occupies it.
     *
     * @param fixture The fixture to add
     * @param id The requested ID for the fixture
     * @return true if the fixture was successfully added to doc,
     *         otherwise false.
     */
    bool addFixture(Fixture* fixture, quint32 id = Fixture::invalidId());

    /**
     * Delete the given fixture instance from Doc
     *
     * @param id The ID of the fixture instance to delete
     */
    bool deleteFixture(quint32 id);

    /**
     * Get the fixture instance that has the given ID
     *
     * @param id The ID of the fixture to get
     */
    Fixture* fixture(quint32 id) const;

    /**
     * Get a list of fixtures
     */
    QList <Fixture*> fixtures() const;

    /**
     * Get the fixture that occupies the given DMX address. If multiple fixtures
     * occupy the same address, the one that has been last modified is returned.
     *
     * @param universeAddress The universe & address of the fixture to look for
     * @return The fixture ID or Fixture::invalidId() if not found
     */
    quint32 fixtureForAddress(quint32 universeAddress) const;

    /**
     * Get the total power consumption of all fixtures in the current
     * workspace.
     *
     * @param[out] fuzzy Number of fixtures without power consumption value
     * @return Total power consumption
     */
    int totalPowerConsumption(int& fuzzy) const;

protected:
    /**
     * Create a new fixture ID
     */
    quint32 createFixtureId();

signals:
    /** Signal that a fixture has been added */
    void fixtureAdded(quint32 fxi_id);

    /** Signal that a fixture has been removed */
    void fixtureRemoved(quint32 fxi_id);

    /** Signal that a fixture's properties have changed */
    void fixtureChanged(quint32 fxi_id);

private slots:
    /** Catch fixture property changes */
    void slotFixtureChanged(quint32 fxi_id);

protected:
    /** Fixtures */
    QMap <quint32,Fixture*> m_fixtures;

    /** Addresses occupied by fixtures */
    QHash <quint32,quint32> m_addresses;

    /** Latest assigned fixture ID */
    quint32 m_latestFixtureId;

    /*********************************************************************
     * Fixture groups
     *********************************************************************/
public:
    /** Add a new fixture group. Doc takes ownership of the group. */
    bool addFixtureGroup(FixtureGroup* grp, quint32 id = FixtureGroup::invalidId());

    /**
     * Remove and delete a fixture group. Doesn't destroy group's fixtures.
     * The group pointer is invalid after this call.
     */
    bool deleteFixtureGroup(quint32 id);

    /** Get a fixture group by id */
    FixtureGroup* fixtureGroup(quint32 id) const;

    /** Get a list of Doc's fixture groups */
    QList <FixtureGroup*> fixtureGroups() const;

signals:
    void fixtureGroupAdded(quint32 id);
    void fixtureGroupRemoved(quint32 id);
    void fixtureGroupChanged(quint32 id);

private slots:
    /** Catch fixture group property changes */
    void slotFixtureGroupChanged(quint32 id);

private:
    /** Create a new fixture group ID */
    quint32 createFixtureGroupId();

private:
    /** Fixture Groups */
    QMap <quint32,FixtureGroup*> m_fixtureGroups;

    /** Latest assigned fixture group ID */
    quint32 m_latestFixtureGroupId;

    /*********************************************************************
     * Functions
     *********************************************************************/
public:
    /**
     * Add the given function to doc's function array.
     * If id == Function::invalidId(), doc assigns the function a new ID
     * and takes ownership, unless there is no more room for more functions.
     *
     * If id != Function::invalidId(), doc attempts to put the function at
     * that exact index, unless another function already occupies it.
     *
     * @param function The function to add
     * @param id The requested ID for the function
     * @return true if the function was successfully added to doc, otherwise false.
     */
    bool addFunction(Function* function, quint32 id = Function::invalidId());

    /**
     * Get a list of currently available functions
     *
     * @return List of functions
     */
    QList <Function*> functions() const;

    /**
     * Delete the given function
     *
     * @param id The ID of the function to delete
     * @return true if the function was found and deleted
     */
    bool deleteFunction(quint32 id);

    /**
     * Get a function that has the given ID
     *
     * @param id The ID of the function to get
     * @return A function at the given ID or NULL if not found
     */
    Function* function(quint32 id) const;

protected:
    /**
     * Create a new function Id
     */
    quint32 createFunctionId();

    /**
     * Assign the given function ID to the function, place the function
     * at the same index in m_functionArray, increase function count and
     * emit functionAdded() signal.
     *
     * @param function The function to assign
     * @param id The ID to assign to the function
     */
    void assignFunction(Function* function, quint32 id);

private slots:
    /** Slot that catches function change signals */
    void slotFunctionChanged(quint32 fid);

signals:
    /** Signal that a function has been added */
    void functionAdded(quint32 function);

    /** Signal that a function has been removed */
    void functionRemoved(quint32 function);

    /** Signal that a function has been changed */
    void functionChanged(quint32 function);

protected:
    /** Functions */
    QMap <quint32,Function*> m_functions;

    /** Latest assigned function ID */
    quint32 m_latestFunctionId;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /**
     * Load contents from the given XML document
     *
     * @param root The Engine XML root node to load from
     * @return true if successful, otherwise false
     */
    bool loadXML(const QDomElement& root);

    /**
     * Save contents to the given XML file.
     *
     * @param doc The XML document to save to
     * @param wksp_root The workspace root node to save under
     * @return true if successful, otherwise false
     */
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

private:
    /**
     * Calls postLoad() for each Function after everything has been loaded
     * to do post-load cleanup & mappings.
     */
    void postLoad();
};

#endif
