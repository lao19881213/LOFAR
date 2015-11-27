//#  BypassSettings.h: implementation of the BypassSettings class
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

#include <APL/RSP_Protocol/BypassSettings.h>
#include <APL/RTCCommon/MarshallBlitz.h>

using namespace std;
using namespace blitz;
namespace LOFAR {
  namespace RSP_Protocol {


size_t BypassSettings::getSize() const
{
  return MSH_size(m_registers);
}

size_t BypassSettings::pack  (char* buffer) const
{
  size_t offset = 0;
  return MSH_pack(buffer, offset, m_registers);
}

size_t BypassSettings::unpack(const char *buffer)
{
  size_t offset = 0;
  return MSH_unpack(buffer, offset, m_registers);
}

  } // namespace RSP_Protocol
} // namespace LOFAR
