//# Delays.h: Calculate delay compensation for all stations.
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

#ifndef LOFAR_INPUTPROC_DELAYS_H
#define LOFAR_INPUTPROC_DELAYS_H

// \file
// Calculate delay compensation for all stations.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <string>
#include <vector>
#include <map>

#include <Common/LofarTypes.h>
#include <CoInterface/MultiDimArray.h>
#include <CoInterface/Parset.h>
#include <CoInterface/SubbandMetaData.h>
#include <CoInterface/SmartPtr.h>
#include <InputProc/RSPTimeStamp.h>

#ifdef HAVE_CASACORE
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <casa/Quanta/MVDirection.h>
#include <casa/Quanta/MVPosition.h>
#include <casa/Quanta/MVEpoch.h>
#endif

namespace LOFAR
{
  namespace Cobalt
  {

    // Speed of light in vacuum, in m/s.
    const double speedOfLight = 299792458.0;

    // Workholder for calculating the delay compensation that must be applied
    // per beam per station. We start by calculating the path length
    // difference for beam \f$\mathbf{b}_i\f$ between station \f$j\f$ at
    // position \f$\mathbf{p}_j\f$ and the reference station 0 at position
    // \f$\mathbf{p}_0\f$.
    // \f[
    //   d_{ij} - d_{i0} = \mathbf{b}_i \cdot \mathbf{p}_j
    //                   - \mathbf{b}_i \cdot \mathbf{p}_0
    //                   = \mathbf{b}_i \cdot (\mathbf{p}_j - \mathbf{p}_0)
    // \f]
    // The choice of reference station is arbitrary, so we simply choose the
    // first station from the parameter set. From the equation above it is
    // clear that we can reduce the number of dot products if we precalculate
    // the position difference vectors \f$\mathbf{p}_j - \mathbf{p}_0$\f,
    // which we will store in \c			itsPositionDiffs.
    //
    // The geometrical delay is easily obtained by dividing the path length
    // difference by the speed of light in vacuum. We don't need to know the
    // speed of light in the atmosphere, because the AZEL directions that
    // we've calculated are valid for vacuum (only!). This is the delay that
    // must be compensated for.
    //
    // The calculated delay compensation must be split into a coarse (whole
    // sample) delay and a fine (subsample) delay.  The coarse delay will be
    // applied in the input section as a true time delay, by shifting the
    // input samples. The fine delay will be applied in the correlator as a
    // phase shift in each frequency channel.
    class Delays
    {
    public:
      Delays(const Parset &ps, size_t stationIdx, const TimeStamp &from, size_t increment);
      ~Delays();

      // Output structures for adjusted directions and delays
      struct Delay {
        double  direction[3];
        double  delay;
        double  clockCorrection;

        double  totalDelay() const {
          return delay + clockCorrection;
        }

        bool operator==(const Delay &other) const {
          return direction[0] == other.direction[0] &&
                 direction[1] == other.direction[1] &&
                 direction[2] == other.direction[2] &&
                 delay == other.delay &&
                 clockCorrection == other.clockCorrection;
        }
      };

      struct BeamDelays {
        struct Delay              SAP;

        // Delays for all coherent (!) TABs
        std::vector<struct Delay> TABs;

        bool operator==(const BeamDelays &other) const {
          return SAP == other.SAP && TABs == other.TABs;
        }
      };

      class AllDelays {
      public:
        AllDelays(const Parset &ps);

        // All delays for all SAPs (and their TABs)
        std::vector<struct BeamDelays> SAPs;

        bool operator==(const AllDelays &other) const {
          return SAPs == other.SAPs;
        }

      private:
        // Don't allow construction with illegal dimensions
        AllDelays();
      };

      // Get the set of directions (ITRF) and delays for the beams
      void getNextDelays( AllDelays &result );

      /*
       * Convert the (delaysAtBegin, delaysAfterEnd) delays pair to all delays
       * required in metaDatas, and to the read_offsets at which the data should
       * be read from the circular buffer.
       *
       * Essentially, the read_offset for each subband is 
       *     SAP.delay / sampleDuration
       * and in the metaData the remainders will be recorded, that is,
       *     SAP.delay % sampleDuration.
       */
      void generateMetaData( const AllDelays &delaysAtBegin, const AllDelays &delaysAfterEnd, const std::vector<size_t> &subbands, std::vector<SubbandMetaData> &metaDatas, std::vector<ssize_t> &read_offsets );

    private:
      const Parset &parset;
      const size_t stationIdx;
      const TimeStamp from;
      const size_t increment;
      TimeStamp currentTime;

      // Test whether the conversion engine actually works.
      bool test();

      // Initialise computing/converting engines
      void init();

      // Computes the delays for a specific moment in time and stores them
      // in `result'.
      void calcDelays( const TimeStamp &timestamp, AllDelays &result );

      // Returns the clock correction delay to add for this station
      double clockCorrection() const;

#ifdef HAVE_CASACORE
      casa::MVEpoch                       toUTC( const TimeStamp &timestamp ) const;

      // Converts a sky direction to a direction and delay
      struct Delay convert( casa::MDirection::Convert &converter, const casa::MVDirection &direction ) const;

      casa::MeasFrame frame;

      std::vector<casa::MDirection::Types> directionTypes; // [sap]
      std::map<casa::MDirection::Types, casa::MDirection::Convert> converters; // [type]

      // Station to reference station position difference vector.
      casa::MVPosition phasePositionDiff;
#endif
    };

  } // namespace Cobalt
} // namespace LOFAR

#endif

