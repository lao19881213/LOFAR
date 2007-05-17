//#  ActiveObs.h: Implements a state machine for an active observation
//#
//#  Copyright (C) 2006
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

#ifndef LOFAR_STATIONCU_ACTIVEOBSERVATION_H
#define LOFAR_STATIONCU_ACTIVEOBSERVATION_H

// \file
// Implements a state machine for an active observation

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <GCF/TM/GCF_Fsm.h>
#include <GCF/TM/GCF_Event.h>
#include <GCF/TM/GCF_Port.h>
#include <GCF/TM/GCF_TimerPort.h>
#include <APL/APLCommon/Observation.h>
#include <APL/APLCommon/CTState.h>
#include <APL/APLCommon/PropertySetAnswerHandlerInterface.h>
#include <APL/APLCommon/PropertySetAnswer.h>
#include <GCF/PAL/GCF_MyPropertySet.h>
#include <GCF/PAL/GCF_Answer.h>


// Avoid 'using namespace' in headerfiles

namespace LOFAR {
	using GCF::TM::GCFFsm;
	using GCF::TM::GCFEvent;
	using GCF::TM::GCFPortInterface;
	using GCF::TM::GCFTask;
	using GCF::TM::GCFTimerPort;
	using GCF::PAL::GCFMyPropertySet;
	using APLCommon::CTState;
	namespace StationCU {

// \addtogroup package
// @{

// class_description
// The ActiveObs class manages the states of the BeamController and the
// CalibrationController. The ActiveObs is implemented as a FSM to simplify
// the (code of the) StationController task.
// The ActiveObs objects are created and destroyed by the StationController task.
// The ActiveObs class can instruct the ChildControl task directly, the responses
// are captured in the StationController task that will forward them.
//
class ActiveObs : public GCFFsm,
				  APLCommon::PropertySetAnswerHandlerInterface
{
public:
	ActiveObs (const string&			name,
			   State					initial,
			   ACC::APS::ParameterSet*	aPS,
			   GCFTask&					task);
	virtual ~ActiveObs();
	virtual void handlePropertySetAnswer(GCFEvent&	answer);

	void					start()		{ initFsm();	}
	bool					isReady()	const { return (itsReadyFlag); }
	bool					inSync()	const { return (itsCurState == itsReqState); }
	CTState::CTstateNr		curState()	const { return (itsCurState); }
	string					getName()	const { return (itsName); }
	APLCommon::Observation*	obsPar()	{ return (&itsObsPar); }

	ostream& print (ostream& os) const;

	GCFEvent::TResult	initial	   (GCFEvent&	event, GCFPortInterface&	port);
	GCFEvent::TResult	starting   (GCFEvent&	event, GCFPortInterface&	port);
	GCFEvent::TResult	connected  (GCFEvent&	event, GCFPortInterface&	port);
	GCFEvent::TResult	standby    (GCFEvent&	event, GCFPortInterface&	port);
	GCFEvent::TResult	operational(GCFEvent&	event, GCFPortInterface&	port);
	GCFEvent::TResult	stopping   (GCFEvent&	event, GCFPortInterface&	port);

	uint32						itsStopTimerID;

private:
	// Default construction and copying is not allowed
	ActiveObs();
	ActiveObs(const ActiveObs&	that);
	ActiveObs& operator=(const ActiveObs& that);

	//# --- Datamembers ---
	APLCommon::PropertySetAnswer	itsPropertySetAnswer;
	GCFMyPropertySet*				itsPropertySet;
	GCFTimerPort*					itsPropSetTimer;

	string						itsName;
	int32						itsInstanceNr;
	APLCommon::Observation		itsObsPar;
	bool						itsBeamCntlrReady;
	bool						itsCalCntlrReady;
	string						itsBeamCntlrName;
	string						itsCalCntlrName;
	bool						itsReadyFlag;
	CTState::CTstateNr			itsReqState;
	CTState::CTstateNr			itsCurState;
};

//#
//# operator<<
//#
inline ostream& operator<< (ostream& os, const ActiveObs& anActiveObs)
{	
	return (anActiveObs.print(os));
}

// @}
  } // namespace StationCU
} // namespace LOFAR

#endif
