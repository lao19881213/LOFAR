//# AntennaModelHBA.h: HBA antenna model interface definitions.
//#
//# Copyright (C) 2013
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

#ifndef LOFAR_STATIONRESPONSE_ANTENNAMODELHBA_H
#define LOFAR_STATIONRESPONSE_ANTENNAMODELHBA_H

// \file
// HBA antenna model interface definitions.

#include <StationResponse/Types.h>
#include <Common/lofar_smartptr.h>

namespace LOFAR
{
namespace StationResponse
{

// \addtogroup StationResponse
// @{

class AntennaModelHBA
{
public:
    typedef shared_ptr<AntennaModelHBA>         Ptr;
    typedef shared_ptr<const AntennaModelHBA>   ConstPtr;

    virtual ~AntennaModelHBA();

    virtual matrix22c_t response(real_t freq, const vector3r_t &direction,
        const vector3r_t &direction0) const;

    virtual diag22c_t arrayFactor(real_t freq, const vector3r_t &direction,
        const vector3r_t &direction0) const;

    virtual raw_response_t rawResponse(real_t freq, const vector3r_t &direction,
        const vector3r_t &direction0) const;

    virtual raw_array_factor_t rawArrayFactor(real_t freq,
        const vector3r_t &direction, const vector3r_t &direction0) const = 0;

    virtual matrix22c_t elementResponse(real_t freq,
        const vector3r_t &direction) const = 0;
};

// @}

} //# namespace StationResponse
} //# namespace LOFAR

#endif
