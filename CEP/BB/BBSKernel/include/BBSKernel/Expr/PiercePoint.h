//# PiercePoint.h: Pierce point for a direction (az,el) on the sky.
//#
//# Copyright (C) 2007
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

#ifndef EXPR_PIERCEPOINT_H
#define EXPR_PIERCEPOINT_H

#include <BBSKernel/Instrument.h>
#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/ResultVec.h>

#ifdef EXPR_GRAPH
#include <Common/lofar_string.h>
#endif

namespace LOFAR
{
namespace BBS
{
  
// \ingroup Expr
// @{

const double iono_height=400000.; //height (m) 

class PiercePoint: public ExprRep
{
public:
    PiercePoint(const Station &station, const Expr &direction);
    ResultVec getResultVec(const Request &request);
    
private:
    void evaluate(const Request &request, const Matrix &in_az,
        const Matrix &in_el, Matrix &out_x, Matrix &out_y, Matrix &out_z,
        Matrix &out_alpha);

#ifdef EXPR_GRAPH
    virtual std::string getLabel();
#endif

    Station                 itsStation;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
