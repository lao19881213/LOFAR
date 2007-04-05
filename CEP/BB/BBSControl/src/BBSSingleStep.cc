//#  BBSSingleStep.cc: 
//#
//#  Copyright (C) 2002-2007
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

#include <BBSControl/BBSSingleStep.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/CommandHandler.h>
#include <Common/StreamUtil.h>
#include <APS/ParameterSet.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
  namespace BBS
  {
    using ACC::APS::ParameterSet;
    using LOFAR::operator<<;

    //##--------   P u b l i c   m e t h o d s   --------##//

    BBSSingleStep::~BBSSingleStep()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    }


    void BBSSingleStep::accept(CommandHandler &handler) const
    {
        handler.handle(*this);
    }


    void BBSSingleStep::print(ostream& os) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      BBSStep::print(os);
      Indent id;
      os << endl << indent << "Output data: " << itsOutputData;
    }


    //##--------   P r o t e c t e d   m e t h o d s   --------##//

    BBSSingleStep::BBSSingleStep(const BBSStep* parent) :
      BBSStep(parent)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
    }


    BBSSingleStep::BBSSingleStep(const string& name, 
				 const ParameterSet& parset,
				 const BBSStep* parent) :
      BBSStep(name, parset, parent)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Create a subset of \a parset, containing only the relevant keys for
      // the current BBSSingleStep.
      ParameterSet ps(parset.makeSubset("Step." + name + "."));

      // Get the name of the data column to write to
      itsOutputData = ps.getString("OutputData");
    }


    void BBSSingleStep::write(BlobOStream& bos) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      BBSStep::write(bos);
      bos << itsOutputData;
    }


    void BBSSingleStep::read(BlobIStream& bis)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      BBSStep::read(bis);
      bis >> itsOutputData;
    }


  } // namespace BBS

} // namespace LOFAR
