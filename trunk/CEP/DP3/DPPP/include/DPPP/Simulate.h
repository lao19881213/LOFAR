//# Simulate.h: Simulate visibilities for a patch of sources.
//#
//# Copyright (C) 2012
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
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

#ifndef DPPP_SIMULATE_H
#define DPPP_SIMULATE_H

// \file
// Simulate visibilities for a patch of sources.

#include <DPPP/Baseline.h>
#include <DPPP/Cursor.h>
#include <DPPP/Patch.h>
#include <DPPP/Position.h>
#include <Common/lofar_complex.h>

//# Forward Declaration.
namespace casa
{
  template<typename T> class Vector;
  template<typename T> class Matrix;
}

namespace LOFAR
{
namespace DPPP
{

  // \addtogroup NDPPP
  // @{

  // Setup the splitting of the baseline UVWs into station UVWs. It returns
  // the indices of the baselines needed to split the baseline UVWs into
  // station UVWs. They are in such an order that the UVW of a station is known
  // before used in another baseline to derive the UVW of the other station.
  // It can handle cases where baselines occur in disjoint station groups
  // like 0-1, 0-2, 1-2 and 3-4, 4-5, 5-6.
  // Note that the first station of a group gets UVW=0. All other station UVWs
  // are relative to it using the baseline UVWs.
  // Also note that nr of groups can be derived from the size of the returned
  // vector (because it contains no entry for the first antenna in a group).
  vector<int> nsetupSplitUVW (uint nant, const casa::Vector<int>& ant1,
                              const casa::Vector<int>& ant2);

  // Do the actual splitting of baseline UVWs into station UVWs using
  // the index vector generated by setupSplitUVW.
  void nsplitUVW (const vector<int>& blindex,
                  const vector<Baseline>& baselines,
                  const casa::Matrix<double>& uvwbl,
                  casa::Matrix<double>& uvwant);

// Split baseline UVW coordinates into station UVW coordinates (by assuming the
// station with index 0 has UVW coordinates of (0, 0, 0)).
//
// \param[in]   nStation
// The number of stations.
// \param[in]   nBaseline
// The number of baselines.
// \param[in]   baselines
// A cursor for a 1-D buffer of baselines of shape (\p nBaseline).
// \param[in]   uvw
// A cursor for a 2-D buffer of UVW coordinates of shape (\p nBaseline, 3).
// \param[in]   split
// A cursor for a 2-D buffer of station UVW coordinates of shape
// (\p nStation, 3).
void splitUVW(size_t nStation, size_t nBaseline,
    const_cursor<Baseline> baselines, const_cursor<double> uvw,
    cursor<double> split);

// Transform UVW coordinates from phase reference position \p from to phase
// reference position \p to. The transformation is performed in place.
//
// \param[in]   from
// Current phase reference position for the UVW coordinates.
// \param[in]   to
// New phase reference position for the UVW coordinates.
// \param[in]   nUVW
// The number of UVW coordinates to transform.
// \param[in]   uvw
// A 2-D buffer of UVW coordinates of shape (\p UVW, 3).
void rotateUVW(const Position &from, const Position &to, size_t nUVW,
    double *uvw);

// Simulate visibilities for a patch of sources. The computed visibilities are
// added to \p vis.
//
// \param[in]   reference
// Phase reference position.
// \param[in]   patch
// Patch of sources to simulate visibilities for.
// \param[in]   nStation
// The number of stations.
// \param[in]   nBaseline
// The number of baselines.
// \param[in]   nChannel
// The number of frequency channels.
// \param[in]   baselines
// A cursor for a 1-D buffer of baselines of shape (\p nBaseline).
// \param[in]   freq
// A cursor for a 1-D buffer of channel frequencies of shape (\p nChannel).
// \param[in]   uvw
// A cursor for a 2-D buffer of station UVW coordinates of shape
// (\p nStation, 3).
// \param[in]   buffer
// A cursor for a 3-D buffer of shape (\p nBaseline, \p nChannel, 4) into which
// the simulated visibilities will be written.
void simulate(const Position &reference, const Patch::ConstPtr &patch,
    size_t nStation, size_t nBaseline, size_t nChannel,
    const_cursor<Baseline> baselines, const_cursor<double> freq,
    const_cursor<double> uvw, cursor<dcomplex> buffer);
// @}

} //# namespace DPPP
} //# namespace LOFAR

#endif
