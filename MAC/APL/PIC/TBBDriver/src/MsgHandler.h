//#  -*- mode: c++ -*-
//#
//#  MsgHandler.h: TBB Driver boardaction class
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



#ifndef MSGHANDLER_H_
#define MSGHANDLER_H_

#include <Common/LofarTypes.h>
#include <GCF/TM/GCF_Control.h>
#include <Common/lofar_set.h>

#include <APL/TBB_Protocol/TBB_Protocol.ph>
#include "TP_Protocol.ph"
#include "DriverSettings.h"

namespace LOFAR {
	namespace TBB {
	
		class MsgHandler
		{
	
			public:
				// Constructors for a MsgHandler object.
				MsgHandler();

				// Destructor for MsgHandler. */
				~MsgHandler();
				
				void addClient(GCFPortInterface& port);
				
				void removeClient(GCFPortInterface& port);
				
				void sendTrigger(GCFEvent& event, int boardnr);
				
				void sendError(GCFEvent& event);
				
				void sendBoardChange(uint32 activeboards);
					
				void sendMessage(GCFEvent& event);

			protected:
				
			private:
				TbbSettings *TS;
				
				std::list<GCFPortInterface*> itsClientMsgList;  // list of clients witch receive messages
				
				TBBTriggerEvent			*itsTriggerE;
				TBBErrorEvent				*itsErrorE;
				TBBBoardchangeEvent	*itsBoardchangeE;
					
		};
	} // end TBB namespace
} // end LOFAR namespace

#endif /* MSGHANDLER_H_ */
