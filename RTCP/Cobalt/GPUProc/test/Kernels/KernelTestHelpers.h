//# KernelTestHelpers.h: test Kernels/DelayAndBandPassKernel class
//# Copyright (C) 2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
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

#include <CoInterface/Parset.h>


// print usage of the kernel test.
void usage(char const *testName);

// Helper struct to collect some kernel/pipeline parameters collected from the 
// command line
struct KernelParameters
{
  unsigned nrTabs;
  unsigned nrChannels;
  unsigned idxGPU;
  unsigned nStation;
  unsigned nTimeBlocks;
  bool parameterParsed;

  KernelParameters();
  void print();
};

// Parse kernelParameters from the commandline and initialize a valid Parset
// with the parsed arguments. Returns values also as struct.
KernelParameters parseCommandlineParameters(
      int argc, char *argv[], 
      LOFAR::Cobalt::Parset &ps, const char *testName);
