//# SubbandWriter.h: Write visibilites and beam-formed data
//# Copyright (C) 2008-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_STORAGE_SUBBANDWRITER_H
#define LOFAR_STORAGE_SUBBANDWRITER_H

#include <string>

#include <CoInterface/OutputTypes.h>
#include <CoInterface/Parset.h>
#include <CoInterface/Pool.h>
#include <CoInterface/SmartPtr.h>
#include <CoInterface/StreamableData.h>
#include <CoInterface/FinalMetaData.h>
#include "InputThread.h"
#include "OutputThread.h"

namespace LOFAR
{
  namespace Cobalt
  {
    /*
     * SubbandWriter is responsible for completely handling the reception
     * and writing of one subband of correlated visibilities.
     *
     * It maintains an InputThread and SubbandOutputThread, connected by
     * an internal Pool<> of data blocks.
     */
    class SubbandWriter
    {
    public:
      SubbandWriter(const Parset &parset,
                    unsigned streamNr,
                    RTmetadata &mdLogger,
                    const std::string &mdKeyPrefix,
                    const std::string &logPrefix);

      void process();

      void fini(const FinalMetaData &finalMetaData);

      ParameterSet feedbackLTA() const;

      unsigned streamNr() const { return itsStreamNr; }

    private:
      static const unsigned maxReceiveQueueSize = 60;

      const unsigned itsStreamNr;

      Pool<StreamableData> itsOutputPool;

      InputThread itsInputThread;
      SubbandOutputThread itsOutputThread;
    };
  } // namespace Cobalt
} // namespace LOFAR

#endif

