//# IncoherentStokesTransposeKernel.cc
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

#include "IncoherentStokesTransposeKernel.h"

#include <CoInterface/Align.h>
#include <CoInterface/Config.h>
#include <Common/lofar_complex.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

namespace LOFAR
{
  namespace Cobalt
  {
    using boost::format;
    using boost::lexical_cast;

    const string IncoherentStokesTransposeKernel::theirSourceFile = 
      "IncoherentStokesTranspose.cu";
    const string IncoherentStokesTransposeKernel::theirFunction = 
      "transpose";

    IncoherentStokesTransposeKernel::Parameters::Parameters(const Parset& ps) :
      Kernel::Parameters("incoherentStokesTranspose"),
      nrStations(ps.settings.antennaFields.size()),
      nrChannels(ps.settings.beamFormer.nrHighResolutionChannels),
      nrSamplesPerChannel(ps.settings.blockSize / nrChannels),

      tileSize(16)
    {
      dumpBuffers = 
        ps.getBool("Cobalt.Kernels.IncoherentStokesTransposeKernel.dumpOutput",
                   false);
      dumpFilePattern = 
        str(format("L%d_SB%%03d_BL%%03d_IncoherentStokesTransposeKernel.dat") % 
            ps.settings.observationID);
    }

    size_t IncoherentStokesTransposeKernel::Parameters::bufferSize(BufferType bufferType) const
    {
      switch (bufferType) {
      case IncoherentStokesTransposeKernel::INPUT_DATA:
      case IncoherentStokesTransposeKernel::OUTPUT_DATA:
        return 
          (size_t) nrStations * 
          nrChannels * nrSamplesPerChannel * 
          NR_POLARIZATIONS * sizeof(std::complex<float>);
      default:
        THROW(GPUProcException, "Invalid bufferType (" << bufferType << ")");
      }
    }

    IncoherentStokesTransposeKernel::
    IncoherentStokesTransposeKernel(const gpu::Stream& stream,
                                    const gpu::Module& module,
                                    const Buffers& buffers,
                                    const Parameters& params) :
      CompiledKernel(stream, gpu::Function(module, theirFunction), buffers, params)
    {
      setArg(0, buffers.output);
      setArg(1, buffers.input);

      // LOG_DEBUG_STR("align(params.nrSamplesPerChannel, params.tileSize) = "
      //               << "align(" << params.nrSamplesPerChannel
      //               << ", " << params.tileSize << ") = " 
      //               << align(params.nrSamplesPerChannel, params.tileSize));
      // LOG_DEBUG_STR("align(params.nrChannels, params.tileSize)) = "
      //               << "align(" << params.nrChannels 
      //               << ", " << params.tileSize << ") = "
      //               << align(params.nrChannels, params.tileSize));

      setEnqueueWorkSizes(
        gpu::Grid(align(params.nrSamplesPerChannel, params.tileSize), 
                  align(params.nrChannels, params.tileSize)),
        gpu::Block(params.tileSize, params.tileSize));
    }

    //--------  Template specializations for KernelFactory  --------//

    template<> CompileDefinitions
    KernelFactory<IncoherentStokesTransposeKernel>::compileDefinitions() const
    {
      CompileDefinitions defs =
        KernelFactoryBase::compileDefinitions(itsParameters);

      defs["NR_STATIONS"] = 
        lexical_cast<string>(itsParameters.nrStations);
      defs["NR_CHANNELS"] = 
        lexical_cast<string>(itsParameters.nrChannels);
      defs["NR_SAMPLES_PER_CHANNEL"] = 
        lexical_cast<string>(itsParameters.nrSamplesPerChannel);

      defs["TILE_SIZE"] = lexical_cast<string>(itsParameters.tileSize);
      return defs;
    }

  }
}

