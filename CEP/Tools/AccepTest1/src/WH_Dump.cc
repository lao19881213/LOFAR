//#  WH_Dump.cc: Output workholder for BG Correlator application
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


#include <stdio.h>           // for sprintf

// General includes
#include <Common/LofarLogger.h>
#include <Common/KeyValueMap.h>

// Application specific includes
#include <WH_Dump.h>
#include <DH_Vis.h>
#include <DH_Empty.h> 

using namespace LOFAR;


WH_Dump::WH_Dump(const string& name,
		 const int    elements, 
		 const int    channels) 
  : WorkHolder(1, 0, name, "WH_Dump"),
    itsNelements (elements),
    itsNchannels (channels)
{
  getDataManager().addInDataHolder(0, new DH_Vis("in_", 
						 itsNelements, itsNchannels));
}

WH_Dump::~WH_Dump() {

}

WorkHolder* WH_Dump::construct (const string& name, 
				const int    elements, 
				const int    channels) {
  return new WH_Dump(name, elements, channels);
}

WH_Dump* WH_Dump::make(const string& name) {

  return new WH_Dump(name, itsNelements, itsNchannels);
}

void WH_Dump::process() {

  // negative indices are invalid. This are results calculated from 
  // uninitialized values.
  cout << "COR [" << -2+itsIndex++ <<"]: "<< *((DH_Vis*)getDataManager().getInHolder(0))->getBuffer() << endl;
}

void WH_Dump::dump() {
}
		 
