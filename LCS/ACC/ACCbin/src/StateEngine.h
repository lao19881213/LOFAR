//#  StateEngine.h: (internal) sequence definitions of the command states.
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef ACC_STATEENGINE_H
#define ACC_STATEENGINE_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/Exception.h>
#include <ACC/DH_ApplControl.h>
#include <ACC/ParameterSet.h>

namespace LOFAR {
  namespace ACC {

// define application controller states.
enum ACState {  StateNone = 0, 
		StatePowerUpNodes, StatePowerDownNodes, StateCreatePSubset,
	    StateStartupAppl,  StateDefineCmd,      StateInitCmd,
	    StateRunCmd,       StatePauseCmd,       StateRecoverCmd, 
		StateSnapshotCmd,  StateReinitCmd,      StateInfoCmd,
		StateQuitCmd,      StateKillAppl,
		NR_OF_STATES
};

// Description of class.
class StateEngine
{
public:
	StateEngine(const ParameterSet*	aPS);
	~StateEngine();
	
	// Reset to original state.
	void reset();

	// Start a new state sequence. When the requested state is not a start of
	// a sequence an exception is thrown.
	ACState  startSequence(ACCmd  aStartPoint)		throw (Exception);

	// Get current state.
	ACState getState();

	// Current state is finished, hop the next state and return its value.
	ACState nextState();

	// Report the current state is ready.
	void	ready();

	// Ask is the next state is waiting.
	bool	isNextStateWaiting();

	// Command for handling the state expire timer
	void setStateLifeTime	 (time_t		anInterval);
	void resetStateExpireTime();
	bool IsStateExpired      ();

private:
	StateEngine();

	uint16		itsSequence;
	uint16		itsStepNr;
	bool		itsWantNewState;
	
	// GMT time the current state expires
	time_t	itsStateExpireTime;		

	// (constant) timer values for the states
	time_t	itsPowerUpNodesTime;
	time_t	itsPowerDownNodesTime;
	time_t	itsCreatePSubsetTime;
    time_t	itsStartupApplTime;
	time_t	itsDefineCmdTime;
	time_t	itsInitCmdTime;
    time_t	itsRunCmdTime;
	time_t	itsPauseCmdTime;
	time_t	itsRecoverCmdTime;
	time_t	itsSnapshotCmdTime;
	time_t	itsReinitCmdTime;
	time_t	itsInfoCmdTime;
	time_t	itsQuitCmdTime;
	time_t	itsKillApplTime;
};

inline void StateEngine::ready()
{
	LOG_TRACE_STAT ("StateEngine:ready");
	itsWantNewState = true;
}

inline bool StateEngine::isNextStateWaiting()
{
	return (itsWantNewState);
}

inline void StateEngine::setStateLifeTime(time_t		anInterval)
{
	LOG_DEBUG_STR("State lifetime = " << anInterval);
	itsStateExpireTime = time(0) + anInterval;
}

inline void StateEngine::resetStateExpireTime()
{
	itsStateExpireTime = 0;
}

inline bool StateEngine::IsStateExpired()
{
	return (itsStateExpireTime && (itsStateExpireTime < time(0)));
}



  } // namespace ACC
} // namespace LOFAR

#endif
