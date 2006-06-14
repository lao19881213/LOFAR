//#  lofar_smartptr.h: basic header for the Smartptr package
//#
//#  Copyright (C) 2002
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

#ifndef LOFAR_COMMON_SMARTPTR_H
#define LOFAR_COMMON_SMARTPTR_H

// \file

#if !defined(USE_THREADS)
# define BOOST_DISABLE_THREADS
#endif

#include <boost/smart_ptr.hpp>

namespace LOFAR
{
  using boost::scoped_ptr;
  using boost::scoped_array;
  using boost::shared_ptr;
  using boost::shared_array;
  using boost::weak_ptr;
  using boost::intrusive_ptr;
  using boost::enable_shared_from_this;
}

#endif
