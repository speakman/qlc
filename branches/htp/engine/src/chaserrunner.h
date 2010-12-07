/*
  Q Light Controller
  chaserrunner.h

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

#ifndef CHASERRUNNER_H
#define CHASERRUNNER_H

#include <QList>
#include <QMap>

#include "function.h"

class UniverseArray;
class FadeChannel;
class Scene;
class Doc;

class ChaserRunner
{
public:
    ChaserRunner(Doc* doc, QList <Scene*> steps, quint32 holdBusId,
                 Function::Direction direction, Function::RunOrder runOrder);
    ~ChaserRunner();

    /**
     * Set a flag indicating that m_holdBusId has been tapped.
     */
    void tap();

    /**
     * Reset the runner to a state where nothing has been run yet.
     */
    void reset();

    /**
     * Write the current step to $universes. This method returns false only
     * if there are no steps at all or SingleShot has been completed.
     *
     * @param universes UniverseArray to write values to
     * @return true if the chaser should continue, otherwise false
     */
    bool write(UniverseArray* universes);

private:
    /** Ran at each end of m_steps. Returns false only when SingleShot has been
        completed. */
    bool roundCheck();

    /**
     * Create FadeChannel map for the currently active scene. If $handover
     * == true, then the created FadeChannels' starting values are taken from
     * the old m_channelMap's current values. If the old map doesn't contain
     * a FadeChannel for a new channel, then the new FadeChannel will start
     * from whatever is currently in $universes[address]. This handover must
     * be done for HTP channels to work since UniverseArray's intensity
     * channels are always reset to zero before MasterTimer starts making
     * Function::write() calls. If this handover isn't done, all intensity
     * channels would always fade from 0 to the target value.
     *
     * @param universes Current UniverseArray
     * @param handover See above description.
     */
    QMap <quint32,FadeChannel> createFadeChannels(const UniverseArray* universes,
                                                  bool handover = false) const;

    /************************************************************************
     * Constant parameters
     ************************************************************************/
private:
    const Doc* m_doc;
    const QList <Scene*> m_steps; //! List of steps to go thru
    const quint32 m_holdBusId;
    const Function::Direction m_originalDirection; //! Set during constructor
    const Function::RunOrder m_runOrder;

    /************************************************************************
     * Run-time parameters
     ************************************************************************/
private:
    Function::Direction m_direction; //! Run-time direction
    QMap <quint32,FadeChannel> m_channelMap; //! Current step channels
    quint32 m_elapsed; //! Elapsed timer ticks (==write() calls)
    bool m_tap; //! Tracks bus button taps
    int m_currentStep; //! Current step from m_steps
};

#endif
