//#  StopCepCmd.cc: implementation of the StopCepCmd class
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>

#include "StopCepCmd.h"


using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;	
using namespace TBB;

//--Constructors for a VersionCmd object.--------------------------------------
StopCepCmd::StopCepCmd()
{
	TS = TbbSettings::instance();
	
	setWaitAck(true);
}
  
//--Destructor for GetVersions.------------------------------------------------
StopCepCmd::~StopCepCmd() { }

// ----------------------------------------------------------------------------
bool StopCepCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_STOP_CEP)||(event.signal == TP_STOP_CEP_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void StopCepCmd::saveTbbEvent(GCFEvent& event)
{
	TBBStopCepEvent tbb_event(event);
	
	setBoards(tbb_event.boardmask);

	nextBoardNr(); // select first board
}

// ----------------------------------------------------------------------------
void StopCepCmd::sendTpEvent()
{
	TPStopCepEvent tp_event;
	tp_event.opcode = oc_STOP_CEP;
	tp_event.status = 0;

	TS->boardPort(getBoardNr()).send(tp_event);
	TS->setBoardUsed(getBoardNr());
	TS->boardPort(getBoardNr()).setTimer(TS->timeout());
}

// ----------------------------------------------------------------------------
void StopCepCmd::saveTpAckEvent(GCFEvent& event)
{
	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		setStatus(getBoardNr(), TBB_TIME_OUT);
	}	else {
		TPStopCepAckEvent tp_ack(event);
		LOG_DEBUG_STR(formatString("Received StopCepAck from boardnr[%d]", getBoardNr()));
		
		if (tp_ack.status != 0) {
			setStatus(getBoardNr(), (tp_ack.status << 24));
		}
	}
	nextBoardNr();
}

// ----------------------------------------------------------------------------
void StopCepCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBStopCepAckEvent tbb_ack;
	
	for (int32 i = 0; i < MAX_N_TBBOARDS; i++) { 
		tbb_ack.status_mask[i] = getStatus(i);
	}
	
	if (clientport->isConnected()) { clientport->send(tbb_ack); }
}
