//# SymbolTable.cc: one line description
//#
//# Copyright (C) 2002-2008
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/SymbolTable.h>
#include <cstdlib>

#ifdef HAVE_BFD

namespace LOFAR
{

#if defined(__linux__)
  static const char* bfdFile = "/proc/self/exe";
#elif defined(__sun__)
  static const char* bfdFile = "/proc/self/object/a.out";
#else
# error "Alias for process's executable file, like /proc/self/exe \
on linux, must be present."
#endif


  SymbolTable::SymbolTable() :
    itsBfd(0),
    itsSymbols(0)
  {
    init() && read();
  }


  SymbolTable::~SymbolTable()
  {
    cleanup();
  }


  SymbolTable& SymbolTable::instance()
  {
    static SymbolTable symTab;
    return symTab;
  }


  bool SymbolTable::init()
  {
    bfd_init();
    if ((itsBfd = bfd_openr(bfdFile,0)) == 0) {
      bfd_perror(bfdFile);
      return false;
    }
    if (!bfd_check_format(itsBfd, bfd_object)) {
      bfd_perror(bfdFile);
      return false;
    }
    return true;
  }
 

  bool SymbolTable::read()
  {
    if ((bfd_get_file_flags(itsBfd) & HAS_SYMS) == 0) {
      bfd_perror(bfdFile);
      return true;
    }
    unsigned int size;
    long symcount;

    // circumvent strict-aliasing rules by casting through a union
    union {
      asymbol ***src;
      void **dest;
    } symbolUnion = { &itsSymbols };

    symcount = bfd_read_minisymbols(itsBfd, false, symbolUnion.dest, &size);
    if (symcount == 0) {
      symcount = bfd_read_minisymbols(itsBfd, true, symbolUnion.dest, &size);
    }
    if (symcount < 0) {
      bfd_perror(bfdFile);
      return false;
    }
    return true;
  }


  void SymbolTable::cleanup()
  {
    if (itsSymbols) {
      free(itsSymbols);
      itsSymbols = 0;
    }
    if (itsBfd) {
      bfd_close(itsBfd);
      itsBfd = 0;
    }
  }

} // namespace LOFAR

#endif /* HAVE_BFD */
