//  Profiler.h:
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//  $Log$
//  Revision 1.1.1.1  2003/02/21 11:14:36  schaaf
//  copy from BaseSim tag "CEPFRAME"
//
//  Revision 1.8  2001/10/26 10:06:27  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.7  2001/03/23 10:00:40  gvd
//  Improved documentation and test programs
//  Added clearEventCount function to Step
//
//  Revision 1.6  2001/03/01 13:15:47  gvd
//  Added type argument in DataHolder constructor which is used in
//  the connect functions to check if the DH types match
//  Improved the simulator parser
//  Improved documentation
//
//  Revision 1.5  2001/02/05 14:53:05  loose
//  Added GPL headers
//
//////////////////////////////////////////////////////////////////////

#ifndef BASESIM_PROFILER_H
#define BASESIM_PROFILER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

namespace LOFAR
{

/**
   The Profiler class is used to profile the programme during execution.
   When HAVE_MPI_PROFILER is not defined the class is de-activated by using
   dummy inlined methods.
   If active, the MPE library is used to track wall-clock time spent
   in a particular user defined state.
*/

class Profiler
{
public:

  /// Initialise the Profiler; Go into default de-activated state.
  static void init();

  /** Define a state with name and a colour for the graphical
      representation.
      An identifier for the state is returned. This identified must be 
      passed with the enter/leave State() calls.
  */
  static int defineState (const char* name="noname",
			  const char* color="yellow");

  /// Flag entering a particular state.
  static void enterState (int astate);

  /// Flag leaving a particular state.
  static void leaveState (int astate);

  /// Switch on the Profiler.
  static void activate();

  /// Switch off the Profiler.
  static void deActivate();

private:
  static int  theirNextFreeState;
  static bool theirIsActive;       
  
};

#ifndef HAVE_MPI_PROFILER
inline void Profiler::init() {};
inline int  Profiler::defineState (const char*, const char*) {return 0;}
inline void Profiler::enterState (int) {};
inline void Profiler::leaveState (int) {};
inline void Profiler::activate() {};
inline void Profiler::deActivate() {};
#endif

}

#endif
