//# MSLofarField.cc: MS FIELD subtable with LOFAR extensions
//# Copyright (C) 2011
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
//#
//# @author Ger van Diepen

#include <lofar_config.h>
#include <MSLofar/MSLofarField.h>
#include <measures/Measures/MPosition.h>

using namespace casa;

namespace LOFAR {

  MSLofarField::MSLofarField()
  {}

  MSLofarField::MSLofarField (const String& tableName,
                              Table::TableOption option) 
    : MSField (tableName, option)
  {}

  MSLofarField::MSLofarField (SetupNewTable& newTab, uInt nrrow,
                              Bool initialize)
    : MSField (newTab, nrrow, initialize)
  {}

  MSLofarField::MSLofarField (const Table& table)
    : MSField (table)
  {}

  MSLofarField::MSLofarField (const MSLofarField& that)
    : MSField (that)
  {}

  MSLofarField::~MSLofarField()
  {}

  MSLofarField& MSLofarField::operator= (const MSLofarField& that)
  {
    MSField::operator= (that);
    return *this;
  }

  TableDesc MSLofarField::requiredTableDesc()
  {
    TableDesc td (MSField::requiredTableDesc());
    MSLofarTable::addColumn (td, "LOFAR_TILE_BEAM_DIR", TpArrayDouble,
                             "HBA tile beam direction (polynomial in timw)",
                             "rad", "Direction");
    return td;
  }

} //# end namespace
