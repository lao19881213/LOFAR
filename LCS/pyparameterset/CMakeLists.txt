# $Id$

lofar_package(pyparameterset 1.0 DEPENDS pytools)

include(LofarFindPackage)
lofar_find_package(Boost REQUIRED COMPONENTS python)
lofar_find_package(Python 2.6 REQUIRED)

add_subdirectory(src)
add_subdirectory(test)
