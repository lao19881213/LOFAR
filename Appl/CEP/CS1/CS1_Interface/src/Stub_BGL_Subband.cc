//# Stub_BGL_Subband.cc: Stub for connection to DFTServer and DFTRequest
//#
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


#include <lofar_config.h>

#include <CS1_Interface/Stub_BGL_Subband.h>
#include <Transport/TH_Socket.h>
#include <Transport/Connection.h>
#include <iostream>


using namespace LOFAR;

namespace LOFAR { 

Stub_BGL_Subband::Stub_BGL_Subband(bool iAmOnBGL, const ACC::APS::ParameterSet &pSet)
: Stub_BGL(iAmOnBGL, iAmOnBGL, pSet),
  servers(pSet.getStringVector("Connections.Input_BGLProc.ServerHosts")),
  services(pSet.getStringVector("Connections.Input_BGLProc.Ports"))
{
  ASSERTSTR(itsNrSubbands <= servers.size(),
	"Connections.Subband.ServerHosts does not contain enough hosts");
  ASSERTSTR(itsNrSlavesPerSubband <= services.size(),
	"Connections.Subband.Ports does not contain enough ports");
}


TransportHolder *Stub_BGL_Subband::newClientTH(unsigned subband, unsigned slave)
{
  return new TH_Socket(servers[subband], services[slave], false, Socket::TCP, false);
}


TransportHolder *Stub_BGL_Subband::newServerTH(unsigned, unsigned slave)
{
  return new TH_Socket(services[slave], false, Socket::TCP, 5, false);
}

} //namespace
