//#  GCFFsm.cc: implementation of Finite State Machine.
//#
//#  Copyright (C) 2002-2003
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

#include <GCF_Fsm.h>
#include <GTM_Defines.h>

GCFDummyPort GCFFsm::_gcfPort(0, "GCF", F_FSM_PROTOCOL);

void GCFFsm::initFsm()
{
  GCFEvent e;
  e.signal = F_ENTRY_SIG;
  (void)(this->*_state)(e, _gcfPort); // entry signal
  e.signal = F_INIT_SIG;
  if (GCFEvent::HANDLED != (this->*_state)(e, _gcfPort)) // initial transition
  {
    cerr << "Fsm::init: initial transition F_SIGNAL(F_FSM_PROTOCOL, F_INIT_SIG) not handled." << endl;
    exit(1); // EXIT
  }
}

void GCFFsm::tran(State target, const char* from, const char* to)
{
  LOFAR_LOG_TRACE(TM_STDOUT_LOGGER, (
      "State transition to %s <<== %s",
      to,
      from));

  GCFEvent e;
  e.signal = F_EXIT_SIG;
  (void)(this->*_state)(e, _gcfPort); // exit signal

  _state = target; // state transition
  
  e.signal = F_ENTRY_SIG;
  (void)(this->*_state)(e, _gcfPort); // entry signal
}
