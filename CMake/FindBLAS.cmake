# $Id$
#
# Copyright (C) 2008-2009
# ASTRON (Netherlands Foundation for Research in Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

# A tiny wrapper around the FindBLAS.cmake macro that comes with CMake. Its
# purpose is to wrap the enable_langauage(Fortran) command. If BLAS is
# required, then the Fortran compiler must also be available. Otherwise, it is
# not an error if a Fortran compiler is missing.

# Enable the Fortran compiler, if that has not been done yet.
if(NOT CMAKE_Fortran_COMPILER_WORKS)
  if(BLAS_FIND_REQUIRED)
    enable_language(Fortran)
  else(BLAS_FIND_REQUIRED)
    enable_language(Fortran OPTIONAL)
  endif(BLAS_FIND_REQUIRED)
endif(NOT CMAKE_Fortran_COMPILER_WORKS)

# If we have a working Fortran compiler, call the "real" FindBLAS module;
# otherwise display a diagnostic message.
if(CMAKE_Fortran_COMPILER_WORKS)
  include(${CMAKE_ROOT}/Modules/FindBLAS.cmake)
else(CMAKE_Fortran_COMPILER_WORKS)
  if(BLAS_FIND_REQUIRED)
    message(SEND_ERROR "FindBLAS.cmake requires a working Fortran compiler!")
  else(BLAS_FIND_REQUIRED)
    message(STATUS "FindBLAS.cmake requires a working Fortran compiler!")
  endif(BLAS_FIND_REQUIRED)
endif(CMAKE_Fortran_COMPILER_WORKS)
