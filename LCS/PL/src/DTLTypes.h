//#  DTLTypes.h: definition of DTL specific types used within PL
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LCS_PL_DTLTYPES_H
#define LCS_PL_DTLTYPES_H

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#if !defined(HAVE_DTL)
#error "DTL library is required"
#endif

//# Includes
#include <dtl/DTL.h>

namespace LCS
{
  namespace PL
  {

    //# Forward declaration
    template<typename T> struct DBRep;

    // The BCA template class is a helper class. It provides a generic
    // interface for operator() by defining a typedef for DBRep<T>.
    template<typename T> 
    class BCA
    {
    public:
      typedef DBRep<T> DataObj;
      void operator()(dtl::BoundIOs& boundIOs, DataObj& rowbuf);
    };

    // The BPA template class is a helper class. It provides a generic
    // interface for operator() by defining a typedef for DBRep<T>.
    template<typename T> 
    class BPA
    {
    public:
      typedef DBRep<T> ParamObj ;
      void operator()(dtl::BoundIOs& boundIOs, ParamObj& paramObj);
    };

  } // namespace PL
  
} // namespace LCS

#endif
