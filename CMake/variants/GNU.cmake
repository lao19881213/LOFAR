# Definitions file for the GNU compiler suite.
#
# $Id$


# Compiler suite
set(LOFAR_COMPILER_SUITES GNU)

# Build variants
set(LOFAR_BUILD_VARIANTS DEBUG OPT OPT3 OPTARCH)

# GNU compiler suite
set(GNU_COMPILERS GNU_C GNU_CXX GNU_Fortran GNU_ASM)
set(GNU_C         /usr/bin/gcc)      # GNU C compiler
set(GNU_CXX       /usr/bin/g++)      # GNU C++ compiler
set(GNU_Fortran   /usr/bin/gfortran) # GNU Fortran compiler
set(GNU_ASM       /usr/bin/gcc)      # GNU assembler

set(GNU_C_FLAGS          "-W -Wall -Wno-unknown-pragmas")
set(GNU_C_FLAGS_DEBUG    "-g")
set(GNU_C_FLAGS_OPT      "-g -O2")
set(GNU_C_FLAGS_OPT3     "-g -O3")
set(GNU_C_FLAGS_OPTARCH  "-g -O3 -march=native")
set(GNU_CXX_FLAGS        "-W -Wall -Woverloaded-virtual -Wno-unknown-pragmas")
set(GNU_CXX_FLAGS_DEBUG  "-g")
set(GNU_CXX_FLAGS_OPT    "-g -O2")
set(GNU_CXX_FLAGS_OPT3   "-g -O3")
set(GNU_CXX_FLAGS_OPTARCH "-g -O3 -march=native")
set(GNU_EXE_LINKER_FLAGS)
set(GNU_EXE_LINKER_FLAGS_DEBUG)
set(GNU_EXE_LINKER_FLAGS_OPT)
set(GNU_EXE_LINKER_FLAGS_OPT3)
set(GNU_EXE_LINKER_FLAGS_OPTARCH)
set(GNU_SHARED_LINKER_FLAGS)
set(GNU_SHARED_LINKER_FLAGS_DEBUG)
set(GNU_SHARED_LINKER_FLAGS_OPT)
set(GNU_SHARED_LINKER_FLAGS_OPT3)
set(GNU_SHARED_LINKER_FLAGS_OPTARCH3)
set(GNU_COMPILE_DEFINITIONS)
set(GNU_COMPILE_DEFINITIONS_DEBUG 
                         "-DLOFAR_DEBUG -DENABLE_DBGASSERT -DENABLE_TRACER")
set(GNU_COMPILE_DEFINITIONS_OPT)
set(GNU_COMPILE_DEFINITIONS_OPT3
                         "-DNDEBUG -DDISABLE_DEBUG_OUTPUT")
set(GNU_COMPILE_DEFINITIONS_OPTARCH
                         "-DNDEBUG -DDISABLE_DEBUG_OUTPUT")

