//#  -*- mode: c++ -*-
//#
//#  SetWeightsCmd.h: Beamformer Weights settings command.
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

#ifndef SETWEIGHTSCMD_H_
#define SETWEIGHTSCMD_H_

#include "Command.h"
#include "RSP_Protocol.ph"

#include <Common/LofarTypes.h>
#include <GCF/GCF_Control.h>

namespace RSP
{
  class SetWeightsCmd : public Command
  {
    public:
      /**
       * Constructors for a SetWeightsCmd object.
       */
      SetWeightsCmd(GCFEvent& event, GCFPortInterface& port, Operation oper);
	  
      /* Destructor for SetWeightsCmd. */
      virtual ~SetWeightsCmd();

      /**
       * Acknowledge the command by sending the appropriate
       * response on m_port.
       */
      virtual void ack(CacheBuffer& cache);

      /**
       * Make necessary changes to the cache for the next synchronization.
       * Any changes will be sent to the RSP boards.
       */
      virtual void apply(CacheBuffer& cache);

      /**
       * Complete the command by sending the appropriate response on
       * the m_answerport;
       */
      virtual void complete(CacheBuffer& cache);

      /*@{*/
      /**
       * get timestamp of the event
       */
      virtual const Timestamp& getTimestamp() const;
      virtual void setTimestamp(const Timestamp& timestamp);
      /*@}*/

    private:
      SetWeightsCmd();
      RSPSetweightsEvent* m_event;
  };
};
     
#endif /* SETWEIGHTSCMD_H_ */
