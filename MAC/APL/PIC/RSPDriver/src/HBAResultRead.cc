//#  HBAResultRead.cc: implementation of the HBAResultRead class
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <APL/RSP_Protocol/EPA_Protocol.ph>

#include <APL/RTCCommon/PSAccess.h>
#include <string.h>

#include "StationSettings.h"
#include "HBAProtocolWrite.h"
#include "HBAResultRead.h"
#include "Cache.h"

#include <netinet/in.h>

using namespace LOFAR;
using namespace RSP;
using namespace EPA_Protocol;

HBAResultRead::HBAResultRead(GCFPortInterface& board_port, int board_id)
  : SyncAction(board_port, board_id, StationSettings::instance()->nrBlpsPerBoard())
{
  memset(&m_hdr, 0, sizeof(MEPHeader));
}

HBAResultRead::~HBAResultRead()
{
  /* TODO: delete event? */
}

void HBAResultRead::sendrequest()
{
  uint8 global_rcu = (getBoardId() * StationSettings::instance()->nrRcusPerBoard()) + getCurrentIndex();

  // skip update if the RCU settings have not been applied yet
  if (RTC::RegisterState::READ != Cache::getInstance().getState().hbaprotocol().get(global_rcu)) {
    setContinue(true);
    return;
  }

  // set appropriate header
  MEPHeader::FieldsType hdr;
  if (0 == global_rcu % MEPHeader::N_POL) {
    hdr = MEPHeader::RCU_RESULTX_HDR;
  } else {
    hdr = MEPHeader::RCU_RESULTY_HDR;
  }

  EPAReadEvent rcuresult;
  rcuresult.hdr.set(hdr, 1 << (getCurrentIndex() / MEPHeader::N_POL), MEPHeader::READ, sizeof(HBAProtocolWrite::i2c_result));
  
  m_hdr = rcuresult.hdr; // remember header to match with ack
  getBoardPort().send(rcuresult);
}

void HBAResultRead::sendrequest_status()
{
  // intentionally left empty
}

GCFEvent::TResult HBAResultRead::handleack(GCFEvent& event, GCFPortInterface& /*port*/)
{
  if (EPA_RCU_RESULT != event.signal)
  {
    LOG_WARN("HBAResultRead::handleack:: unexpected ack");
    return GCFEvent::NOT_HANDLED;
  }
  
  EPARcuResultEvent ack(event);

  uint8 global_rcu = (getBoardId() * StationSettings::instance()->nrRcusPerBoard()) + getCurrentIndex();

  if (!ack.hdr.isValidAck(m_hdr))
  {
    LOG_ERROR("HBAResultRead::handleack: invalid ack");
    Cache::getInstance().getState().hbaprotocol().read_error(global_rcu);
    return GCFEvent::HANDLED;
  }

  // reverse and copy control bytes into i2c_result
  RCUSettings::Control& rcucontrol = Cache::getInstance().getBack().getRCUSettings()()((global_rcu));
  uint32 control = htonl(rcucontrol.getRaw());
  memcpy(HBAProtocolWrite::i2c_result + 1, &control, 3);

  if (0 == memcmp(HBAProtocolWrite::i2c_result, ack.result, sizeof(HBAProtocolWrite::i2c_result))) {
    Cache::getInstance().getState().hbaprotocol().read_ack(global_rcu);
  } else {
    LOG_WARN("HBAResultRead::handleack: unexpected I2C result response");
    Cache::getInstance().getState().hbaprotocol().read_error(global_rcu);
  }

  return GCFEvent::HANDLED;
}
