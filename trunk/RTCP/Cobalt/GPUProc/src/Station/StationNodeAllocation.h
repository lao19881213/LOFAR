//# StationNodeAllocation.h: Manages which stations are received on which
//#                          node.
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#ifndef LOFAR_GPUPROC_STATION_NODE_ALLOCATION_H
#define LOFAR_GPUPROC_STATION_NODE_ALLOCATION_H

#include <string>
#include <vector>

#include <CoInterface/Parset.h>
#include <CoInterface/SmartPtr.h>
#include <Stream/Stream.h>
#include <InputProc/Buffer/StationID.h>

namespace LOFAR
{
  namespace Cobalt
  {

    class StationNodeAllocation
    {
    public:
      StationNodeAllocation( const StationID &stationID, const Parset &parset, int mpi_rank, int mpi_size );

      // Returns whether data for this station is received on this node
      bool receivedHere() const;

    private:
      const StationID stationID;
      const Parset &parset;

      // Index of the station in parset.settings.stations
      const ssize_t stationIdx;

      // Dimensions of MPI run, to do round-robin allocation
      const int mpi_rank;
      const int mpi_size;

      // Returns the rank of the MPI node that should receive this station
      int receiverRank() const;
    };
  } // namespace Cobalt
} // namespace LOFAR

#endif

