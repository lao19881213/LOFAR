//  Station.cc:
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//////////////////////////////////////////////////////////////////////

// OnLineProto specific include
#include "OnLineProto/Station.h"

namespace LOFAR
{
   Station::Station (int ID, float x, float y, float z);
     {
       itsX = x;
       itsY = y;
       itsZ = z;  
       itsID = ID;
     }
   
   Station::Station (const Station& s);
     {
       s.itsX = itsX;
       s.itsY = itsY;
       s.itsZ = itsY;  
       s.itsID = itsID;       
     }
      
   Station::~Station()
     {
     }
}// namespace LOFAR
