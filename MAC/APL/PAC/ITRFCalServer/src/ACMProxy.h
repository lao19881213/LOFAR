//#  -*- mode: c++ -*-
//#  ACMProxy.h: class definition for the ACMProxy task.
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
//#  $Id: ACMProxy.h 10791 2008-01-08 14:12:35Z overeem $

#ifndef ACMPROXY_H_
#define ACMPROXY_H_

#include <APL/LBA_Calibration/lba_calibration.h>	// the matlab stuff
#include <APL/RSP_Protocol/XCStatistics.h>
#include <APL/RTCCommon/Timestamp.h>
#include <APL/RTCCommon/ResourceCache.h>

#include <GCF/TM/GCF_Control.h>	// The lot: Task, Port, Fsm, Timer etc.

namespace LOFAR {
  typedef		void*		memptr_t;
  using RTC::ResourceCache;
  namespace CAL {

class ACMProxy : public GCFTask
{
public:
	// The constructor of the ACMProxy task.
	// @param name The name of the task. The name is used for looking
	// up connection establishment information using the GTMNameService and
	// GTMTopologyService classes.
	ACMProxy(string name, ResourceCache&	theACCs);
	virtual ~ACMProxy();

	// All necessary connections established?
	bool isEnabled();

	/*@{*/
	// States
	GCFEvent::TResult initial      (GCFEvent& e, GCFPortInterface& port);
	GCFEvent::TResult idle         (GCFEvent& e, GCFPortInterface& port);
	GCFEvent::TResult initializing (GCFEvent& e, GCFPortInterface& port);
	GCFEvent::TResult receiving    (GCFEvent& e, GCFPortInterface& port);
	GCFEvent::TResult unsubscribing(GCFEvent& e, GCFPortInterface& port);
	/*@}*/

private:
	ResourceCache&	itsACCs;			// pointer to a pointer to a mwArray containing the MatlabACC
										// the ACMProxy may use.
	// Port to the RSPDriver.
	GCFPort 		m_rspdriver;		// connection to the RSPDriver
	memptr_t		m_handle; 			// handle for the UPDXCSTATS events

	RTC::Timestamp  m_starttime; 		// first ACM will be received at this time
	int     		m_request_subband;  // current index for request subband
	int     		m_update_subband;   // current index for update subband
	int				m_n_subbands;		// the number if subband that must be calibrated.
	int     		m_nrcus;            // the number of RCU's in the station (as reported by the RSPDriver)
	int     		m_nrspboards;       // the number of RSP boards in the station (as reported by the RSPDriver)
};

  }; // namespace CAL
}; // namespace LOFAR
     
#endif /* ACMPROXY_H_ */
