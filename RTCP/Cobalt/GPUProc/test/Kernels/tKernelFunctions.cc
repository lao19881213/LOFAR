//# tKernelFunctions.cc: test FIR_FilterKernel class
//#
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

#include <lofar_config.h>

#include <GPUProc/Kernels/FIR_FilterKernel.h>
#include <CoInterface/Parset.h>
#include <CoInterface/BlockID.h>
#include <Common/lofar_complex.h>
#include <sstream>

#include <UnitTest++.h>
#include <memory>
#include <GPUProc/PerformanceCounter.h>
using namespace LOFAR::Cobalt;
using namespace LOFAR;
using namespace std;

TEST(tKernelFunctions)
{
  // **************************************
  // Set up sut
  Parset ps;
  ps.add("Observation.nrBitsPerSample", "8");
  ps.add("Observation.VirtualInstrument.stationList", "[RS000]");
  ps.add("Observation.antennaSet", "LBA_INNER");
  ps.add("Observation.Dataslots.RS000LBA.RSPBoardList", "[0]");
  ps.add("Observation.Dataslots.RS000LBA.DataslotList", "[0]");
  ps.add("Observation.nrBeams", "1");
  ps.add("Observation.Beam[0].subbandList", "[0]");
  ps.add("Cobalt.blockSize", "262144");
  ps.add("Cobalt.Correlator.nrChannelsPerSubband", "64");
  ps.add("Observation.DataProducts.Output_Correlated.enabled", "true");
  ps.add("Observation.DataProducts.Output_Correlated.filenames", "[L12345_SAP000_SB000_uv.MS]");
  ps.add("Observation.DataProducts.Output_Correlated.locations", "[localhost:.]");
  ps.updateSettings();
  
  FIR_FilterKernel::Parameters params(ps,
    ps.settings.antennaFields.size(),
    true,
    1,
    ps.settings.correlator.nrChannels,
    1.0f);

  KernelFactory<FIR_FilterKernel> factory(params);

  gpu::Device device(gpu::Platform().devices()[0]);
  gpu::Context context(device);
  gpu::Stream stream(context);

  gpu::DeviceMemory
    dInput(context, factory.bufferSize(FIR_FilterKernel::INPUT_DATA)),
    dOutput(context, factory.bufferSize(FIR_FilterKernel::OUTPUT_DATA)),
    dCoeff(context,  factory.bufferSize(FIR_FilterKernel::FILTER_WEIGHTS)),
    dHistory(context,  factory.bufferSize(FIR_FilterKernel::HISTORY_DATA));

  gpu::HostMemory
    hInput(context, dInput.size()),
    hOutput(context, dOutput.size());

  cout << "dInput.size() = " << dInput.size() << endl;
  cout << "dOutput.size() = " << dOutput.size() << endl;

  // hInput.get<i8complex>()[2176] = i8complex(1,0);

  i8complex* ibuf = hInput.get<i8complex>();
  for(size_t i = 1922; i < 1923; ++i) {
    ibuf[i] = i8complex(1,0);
  }

  stream.writeBuffer(dInput, hInput);

  auto_ptr<FIR_FilterKernel> kernel(factory.create(stream, dInput, dOutput));

  // **************************************
  // excercise it
  BlockID blockId;                      // create a dummy block-ID struct
  kernel->enqueue(blockId,  0); // insert in kernel queue

  stream.readBuffer(hOutput, dOutput);
  stream.synchronize();
}

int main()
{
  INIT_LOGGER("tKernelFunctions");
  try {
    gpu::Platform pf;
    return UnitTest::RunAllTests() > 0;
  } catch (gpu::GPUException& e) {
    cerr << "No GPU device(s) found. Skipping tests." << endl;
    return 3;
  }

}

