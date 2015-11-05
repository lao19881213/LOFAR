//# ShapeletSource.cc: Class holding the expressions defining a gauss source
//#
//# Copyright (C) 2002
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
//# $Id: ShapeletSource.cc 14789 2010-01-13 12:39:15Z zwieten $

#include <lofar_config.h>
#include <BBSKernel/Expr/ShapeletSource.h>
#include <BBSKernel/ParmManager.h>
#include <BBSKernel/Expr/ExprParm.h>
#include <BBSKernel/Expr/ExprAdaptors.h>
#include <BBSKernel/Expr/Scope.h>
#include <BBSKernel/Expr/ShapeletCoherence.h>
#include <BBSKernel/Expr/SpectralIndex.h>
#include <BBSKernel/Expr/StokesRM.h>

#include <Common/lofar_sstream.h>
#include <ParmDB/SourceInfo.h>

#include <Common/LofarLogger.h>
#include <Common/lofar_iostream.h>

namespace LOFAR
{
namespace BBS
{

ShapeletSource::ShapeletSource(const SourceInfo &source, Scope &scope)
    :   Source(source, scope)
{
    ASSERT(source.getType() == SourceInfo::SHAPELET);

    // Stokes vector.
    const unsigned int nCoeff = source.getSpectralIndexNTerms();

    vector<Expr<Scalar>::Ptr> coeff;
    coeff.reserve(nCoeff);
    for(unsigned int i = 0; i < nCoeff; ++i)
    {
        ostringstream oss;
        oss << "SpectralIndex:" << i << ":" << name();
        coeff.push_back(scope(SKY, oss.str()));
    }

    const double refFreq = source.getSpectralIndexRefFreq();
    ExprParm::Ptr refStokes = scope(SKY, "I:" + name());
    Expr<Scalar>::Ptr stokesI = Expr<Scalar>::Ptr(new SpectralIndex(refFreq,
        refStokes, coeff.begin(), coeff.end()));
    ExprParm::Ptr stokesV = scope(SKY, "V:" + name());

    if(source.getUseRotationMeasure())
    {
        ExprParm::Ptr polFraction = scope(SKY, "PolarizedFraction:" + name());
        ExprParm::Ptr polAngle = scope(SKY, "PolarizationAngle:" + name());
        ExprParm::Ptr rm = scope(SKY, "RotationMeasure:" + name());

        itsStokesVector = StokesRM::Ptr(new StokesRM(stokesI, stokesV,
            polFraction, polAngle, rm));
    }
    else
    {
        ExprParm::Ptr stokesQ = scope(SKY, "Q:" + name());
        ExprParm::Ptr stokesU = scope(SKY, "U:" + name());

        AsExpr<Vector<4> >::Ptr stokes(new AsExpr<Vector<4> >());
        stokes->connect(0, stokesI);
        stokes->connect(1, stokesQ);
        stokes->connect(2, stokesU);
        stokes->connect(3, stokesV);

        itsStokesVector = stokes;
    }

    // Shapelet scales.
    itsShapeletScaleI=source.getShapeletScaleI();
    itsShapeletScaleQ=source.getShapeletScaleQ();
    itsShapeletScaleU=source.getShapeletScaleU();
    itsShapeletScaleV=source.getShapeletScaleV();

    // Shapelet coefficients.
    itsShapeletCoeffI=source.getShapeletCoeffI();
    itsShapeletCoeffQ=source.getShapeletCoeffQ();
    itsShapeletCoeffU=source.getShapeletCoeffU();
    itsShapeletCoeffV=source.getShapeletCoeffV();
}

Expr<JonesMatrix>::Ptr
ShapeletSource::coherence(const baseline_t&,
    const Expr<Vector<3> >::ConstPtr &uvwLHS,
    const Expr<Vector<3> >::ConstPtr &uvwRHS) const
{
    return Expr<JonesMatrix>::Ptr(new ShapeletCoherence(itsStokesVector,
        itsShapeletScaleI, itsShapeletCoeffI, uvwLHS, uvwRHS));
}

} // namespace BBS
} // namespace LOFAR
