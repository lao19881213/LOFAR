//# UHEP_BeamFormerKernel.cc
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#include <lofar_config.h>

#include "UHEP_BeamFormerKernel.h"

#include <algorithm>

#include <Common/lofar_complex.h>

#include <GPUProc/global_defines.h>

namespace LOFAR
{
  namespace Cobalt
  {
    UHEP_BeamFormerKernel::UHEP_BeamFormerKernel(const Parset &ps, cl::Program &program, cl::Buffer &devComplexVoltages, cl::Buffer &devInputSamples, cl::Buffer &devBeamFormerWeights)
      :
      Kernel(ps, program, "complexVoltages")
    {
      setArg(0, devComplexVoltages);
      setArg(1, devInputSamples);
      setArg(2, devBeamFormerWeights);

#if 1
      globalWorkSize = cl::NDRange(NR_POLARIZATIONS, ps.nrTABs(0), ps.nrSubbands());
      localWorkSize = cl::NDRange(NR_POLARIZATIONS, ps.nrTABs(0), 1);

      size_t count = ps.nrSubbands() * (ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1) * NR_POLARIZATIONS;
      size_t nrWeightsBytes = ps.settings.antennaFields.size() * ps.nrTABs(0) * ps.nrSubbands() * NR_POLARIZATIONS * sizeof(std::complex<float>);
      size_t nrSampleBytes = count * ps.settings.antennaFields.size() * ps.nrBytesPerComplexSample();
      size_t nrComplexVoltagesBytesPerPass = count * ps.nrTABs(0) * sizeof(std::complex<float>);
      unsigned nrPasses = std::max((ps.settings.antennaFields.size() + 6) / 16, 1U);
      nrOperations = count * ps.settings.antennaFields.size() * ps.nrTABs(0) * 8;
      nrBytesRead = nrWeightsBytes + nrSampleBytes + (nrPasses - 1) * nrComplexVoltagesBytesPerPass;
      nrBytesWritten = nrPasses * nrComplexVoltagesBytesPerPass;
#else
      ASSERT(ps.nrTABs(0) % 3 == 0);
      ASSERT(ps.settings.antennaFields.size() % 6 == 0);
      unsigned nrThreads = NR_POLARIZATIONS * (ps.nrTABs(0) / 3) * (ps.settings.antennaFields.size() / 6);
      globalWorkSize = cl::NDRange(nrThreads, ps.nrSubbands());
      localWorkSize = cl::NDRange(nrThreads, 1);
      //globalWorkSize = cl::NDRange(ps.settings.antennaFields.size() / 6, ps.nrTABs(0) / 3, ps.nrSubbands());
      //localWorkSize  = cl::NDRange(ps.settings.antennaFields.size() / 6, ps.nrTABs(0) / 3, 1);

      size_t count = ps.nrSubbands() * (ps.nrSamplesPerChannel() + NR_STATION_FILTER_TAPS - 1) * NR_POLARIZATIONS;
      size_t nrWeightsBytes = ps.settings.antennaFields.size() * ps.nrTABs(0) * ps.nrSubbands() * NR_POLARIZATIONS * sizeof(std::complex<float>);
      size_t nrSampleBytes = count * ps.settings.antennaFields.size() * ps.nrBytesPerComplexSample();
      size_t nrComplexVoltagesBytes = count * ps.nrTABs(0) * sizeof(std::complex<float>);
      nrOperations = count * ps.settings.antennaFields.size() * ps.nrTABs(0) * 8;
      nrBytesRead = nrWeightsBytes + nrSampleBytes;
      nrBytesWritten = nrComplexVoltagesBytes;
#endif
    }
  }
}

