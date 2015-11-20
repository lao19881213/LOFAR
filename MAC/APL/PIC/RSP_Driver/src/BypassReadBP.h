//#  -*- mode: c++ -*-
//#
//#  BypassRead.h: Synchronize rcu settings with RSP hardware.
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
//#  $Id: BypassRead.h 30919 2015-02-05 15:26:22Z amesfoort $

#ifndef BYPASSREADBP_H_
#define BYPASSREADBP_H_

#include <APL/RSP_Protocol/MEPHeader.h>

#include "SyncAction.h"

namespace LOFAR {
  namespace RSP {

class BypassReadBP : public SyncAction
{
public:
	// Constructors for a BypassRead object.
	BypassReadBP(GCFPortInterface& board_port, int board_id);

	// Destructor for BypassRead.
	virtual ~BypassReadBP();

	// Send the write message.
	virtual void sendrequest();

	// Send the read request.
	virtual void sendrequest_status();

	// Handle the read result.
	virtual GCFEvent::TResult handleack(GCFEvent& event, GCFPortInterface& port);

private:
	EPA_Protocol::MEPHeader m_hdr;
};

  }; // namespace RSP
}; // namespace LOFAR
     
#endif /* BypassREADBP_H_ */
