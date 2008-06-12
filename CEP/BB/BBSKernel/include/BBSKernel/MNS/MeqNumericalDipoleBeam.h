//# MeqNumericalDipoleBeam.h: Implementation of J.P. Hamaker's memo
//# "Mathematical-physical analysis of the generic dual-dipole antenna"
//#
//# Copyright (C) 2008
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef MNS_MEQNUMERICALDIPOLEBEAM_H
#define MNS_MEQNUMERICALDIPOLEBEAM_H

#include <BBSKernel/MNS/MeqExpr.h>
#include <BBSKernel/MNS/MeqJonesExpr.h>
#include <BBSKernel/MNS/MeqJonesResult.h>

#include <Common/lofar_smartptr.h>
#include <Common/lofar_complex.h>

#include <boost/multi_array.hpp>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// \addtogroup MNS
// @{

class MeqNumericalDipoleBeam: public MeqJonesExprRep
{
public:
    enum
    {
        IN_AZEL,
        IN_ORIENTATION,
        N_InputPort
    } InputPort;
    
    typedef boost::multi_array<dcomplex, 4> BeamCoeffType;

    MeqNumericalDipoleBeam(shared_ptr<const BeamCoeffType> coeff, MeqExpr azel,
        MeqExpr orientation);

    virtual MeqJonesResult getJResult(const MeqRequest &request);

    void evaluate(const MeqRequest &request, const MeqMatrix &in_az,
        const MeqMatrix &in_el, const MeqMatrix &in_orientation,
        MeqMatrix &out_E11, MeqMatrix &out_E12, 
        MeqMatrix &out_E21, MeqMatrix &out_E22);

private:
    inline void evalProjectionMatrix(size_t harmonic,
        const vector<double> &theta, const vector<double> &freq, dcomplex P[]);

#ifdef EXPR_GRAPH
    virtual std::string getLabel();
#endif

    shared_ptr<const BeamCoeffType>   itsCoeff;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
