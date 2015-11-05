//#  SPUStatus.h: implementation of the SPUStatus class
//#
//#  Copyright (C) 2008
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <APL/RSP_Protocol/SPUStatus.h>
#include <APL/RSP_Protocol/MEPHeader.h>
#include <blitz/array.h>

#include <APL/RTCCommon/MarshallBlitz.h>
#include <APL/RSP_Protocol/EPA_Protocol.ph>

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace RSP_Protocol;
using namespace EPA_Protocol;

unsigned int SPUStatus::getSize()
{
  return MSH_ARRAY_SIZE(itsSPUStatus, EPA_Protocol::SPUBoardStatus);
}

unsigned int SPUStatus::pack  (void* buffer)
{
  unsigned int offset = 0;
  
  MSH_PACK_ARRAY(buffer, offset, itsSPUStatus, EPA_Protocol::SPUBoardStatus);

  return offset;
}

unsigned int SPUStatus::unpack(void *buffer)
{
  unsigned int offset = 0;
  
  MSH_UNPACK_ARRAY(buffer, offset, itsSPUStatus, EPA_Protocol::SPUBoardStatus, 1);

  return offset;
}
