//# Add.cc: Add 2 or more nodes
//#
//# Copyright (C) 2003
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

#include <MEQ/Add.h>

namespace Meq {    


Add::Add()
{}

Add::~Add()
{}

Vells Add::evaluate (const Request&, const LoShape&,
		     const vector<Vells*>& values)
{
  /*
  if (values.empty()) {
    return Vells(0.);
  } else {
    Vells result(values[0]->clone());
    result.makeTemp (true);
    for (uint i=1; i<values.size(); i++) {
      // Note that result is a temporary Vells, so the addition is
      // effectively += if the types and sizes match.
      result = result + *(values[i]);
    }
    return result;
  }
  */
  Vells result(0.);
  result.makeTemp (true);
  for (uint i=0; i<values.size(); i++) {
    if ((result.isComplex()  ||  values[i]->isReal())
    &&  result.nx() >= values[i]->nx()  &&  result.ny() >= values[i]->ny()) {
      result += *(values[i]);
    } else {
      result = result + *(values[i]);
    }
  }
  return result;
}


} // namespace Meq
