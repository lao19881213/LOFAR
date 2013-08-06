//# tDelayAndBandPass.cc: test delay and bandpass CUDA kernel
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

#include <cstdlib>
#include <cmath> 
#include <cassert>
#include <string>
#include <sstream>
#include <typeinfo>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <UnitTest++.h>

#include <Common/LofarLogger.h>
#include <Common/LofarTypes.h>
#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/MultiDimArrayHostBuffer.h>

using namespace std;
using namespace LOFAR::Cobalt;

using LOFAR::i16complex;
using LOFAR::i8complex;

static gpu::Stream *stream;

// default compile definitions
const unsigned NR_STATIONS = 2;
const unsigned NR_CHANNELS = 16;
const unsigned NR_SAMPLES_PER_CHANNEL = 64;
const unsigned NR_SAMPLES_PER_SUBBAND = NR_SAMPLES_PER_CHANNEL * NR_CHANNELS;
const unsigned NR_BITS_PER_SAMPLE = 8;
const unsigned NR_POLARIZATIONS = 2;

const unsigned NR_SAPS = 8;
const float SUBBAND_BANDWIDTH = 0.0f * NR_CHANNELS; // should be a multiple of NR_CHANNELS
const bool BANDPASS_CORRECTION = true;
const bool DELAY_COMPENSATION = false;
const bool DO_TRANSPOSE = true;


// Initialize input AND output before calling runKernel().
// We copy both to the GPU, to make sure the final output is really from the kernel.
// T is an LCS i*complex type, or complex<float> when #chnl > 1.
template <typename T>
void runKernel(gpu::Function kfunc,
               MultiDimArrayHostBuffer<complex<float>, 4>& correctedData, // data output
               MultiDimArrayHostBuffer<T,     4>& filteredData, // data input
               MultiDimArrayHostBuffer<float, 3>& delaysAtBegin,
               MultiDimArrayHostBuffer<float, 3>& delaysAfterEnd,
               MultiDimArrayHostBuffer<float, 2>& phaseOffsets,
               MultiDimArrayHostBuffer<float, 1>& bandPassFactors,
               float subbandFrequency, unsigned beam)
{
  gpu::Context ctx(stream->getContext());

  gpu::DeviceMemory devCorrected      (ctx, correctedData.size());
  gpu::DeviceMemory devFiltered       (ctx, filteredData.size());
  gpu::DeviceMemory devDelaysAtBegin  (ctx, delaysAtBegin.size());
  gpu::DeviceMemory devDelaysAfterEnd (ctx, delaysAfterEnd.size());
  gpu::DeviceMemory devPhaseOffsets   (ctx, phaseOffsets.size());
  gpu::DeviceMemory devBandPassFactors(ctx, bandPassFactors.size());

  kfunc.setArg(0, devCorrected);
  kfunc.setArg(1, devFiltered);
  kfunc.setArg(2, subbandFrequency);
  kfunc.setArg(3, beam);
  kfunc.setArg(4, devDelaysAtBegin);
  kfunc.setArg(5, devDelaysAfterEnd);
  kfunc.setArg(6, devPhaseOffsets);
  kfunc.setArg(7, devBandPassFactors);

  gpu::Grid globalWorkSize(1, NR_CHANNELS == 1 ? 1 : NR_CHANNELS / 16, NR_STATIONS);  
  gpu::Block localWorkSize(256, 1, 1); 

  // Overwrite devOutput, so result verification is more reliable.
  stream->writeBuffer(devCorrected,       correctedData);

  stream->writeBuffer(devFiltered,        filteredData);
  stream->writeBuffer(devDelaysAtBegin,   delaysAtBegin);
  stream->writeBuffer(devDelaysAfterEnd,  delaysAfterEnd);
  stream->writeBuffer(devPhaseOffsets,    phaseOffsets);
  stream->writeBuffer(devBandPassFactors, bandPassFactors);

  stream->launchKernel(kfunc, globalWorkSize, localWorkSize);
  stream->readBuffer(correctedData, devCorrected);
  stream->synchronize(); // wait until transfer completes
}

gpu::Function initKernel(gpu::Context ctx, const CompileDefinitions& defs)
{
  // Compile to ptx. Copies the kernel to the current dir
  // (also the complex header, needed for compilation).
  string kernelPath("DelayAndBandPass.cu");
  CompileFlags flags(defaultCompileFlags());
  vector<gpu::Device> devices(1, gpu::Device(0));
  string ptx(createPTX(kernelPath, defs, flags, devices));
  gpu::Module module(createModule(ctx, kernelPath, ptx));
  gpu::Function kfunc(module, "applyDelaysAndCorrectBandPass");

  return kfunc;
}

CompileDefinitions getDefaultCompileDefinitions()
{
  CompileDefinitions defs;

  defs["NR_STATIONS"]            = boost::lexical_cast<string>(NR_STATIONS);
  defs["NR_CHANNELS"]            = boost::lexical_cast<string>(NR_CHANNELS);
  defs["NR_SAMPLES_PER_CHANNEL"] = boost::lexical_cast<string>(NR_SAMPLES_PER_CHANNEL);
  defs["NR_SAMPLES_PER_SUBBAND"] = boost::lexical_cast<string>(NR_SAMPLES_PER_SUBBAND);
  defs["NR_BITS_PER_SAMPLE"]     = boost::lexical_cast<string>(NR_BITS_PER_SAMPLE);
  defs["NR_POLARIZATIONS"]       = boost::lexical_cast<string>(NR_POLARIZATIONS);

  defs["NR_SAPS"]                = boost::lexical_cast<string>(NR_SAPS);
  // SUBBAND_BANDWIDTH must be printed as a float c-string.
  // Could use boost::format() to enforce more precision.
  defs["SUBBAND_BANDWIDTH"]      = boost::lexical_cast<string>(SUBBAND_BANDWIDTH) + 'f';
  if (BANDPASS_CORRECTION)
    defs["BANDPASS_CORRECTION"]  = "1";
  if (DELAY_COMPENSATION)
    defs["DELAY_COMPENSATION"]   = "1";
  if (DO_TRANSPOSE)
    defs["DO_TRANSPOSE"]         = "1";

  return defs;
}

// T is an LCS i*complex type, or complex<float> when #chnl > 1. It is the value type of the data input array.
template <typename T>
vector<complex<float> > runTest(
                const CompileDefinitions& compileDefs,
                float subbandFrequency,
                unsigned beam,
                float delayBegin,
                float delayEnd,
                float phaseOffset,
                float bandPassFactor)
{
  gpu::Context ctx(stream->getContext());

  // Don't use the Kernel class helpers to retrieve buffer sizes,
  // because we test the kernel, not the Kernel class.
  MultiDimArrayHostBuffer<complex<float>, 4>* correctedData; // data output
  CompileDefinitions::const_iterator cit1(compileDefs.find("DO_TRANSPOSE"));
  if (cit1 != compileDefs.end())
    correctedData = new MultiDimArrayHostBuffer<complex<float>, 4>(boost::extents[NR_STATIONS][NR_SAMPLES_PER_CHANNEL][NR_CHANNELS][NR_POLARIZATIONS], ctx);
  else // no transpose
    correctedData = new MultiDimArrayHostBuffer<complex<float>, 4>(boost::extents[NR_STATIONS][NR_POLARIZATIONS][NR_SAMPLES_PER_CHANNEL][NR_CHANNELS], ctx);

  MultiDimArrayHostBuffer<T, 4>* filteredData; // data input
  CompileDefinitions::const_iterator cit2(compileDefs.find("NR_CHANNELS"));
  assert(cit2 != compileDefs.end());
  unsigned nchnl = boost::lexical_cast<unsigned>(cit2->second);
  if (nchnl == 1) // integer input data (FIR+FFT skipped)
    filteredData = new MultiDimArrayHostBuffer<T, 4>             (boost::extents[NR_STATIONS][NR_SAMPLES_PER_CHANNEL][NR_CHANNELS][NR_POLARIZATIONS], ctx);
  else // specify complex<float>, which T must be too in this case
    filteredData = new MultiDimArrayHostBuffer<complex<float>, 4>(boost::extents[NR_STATIONS][NR_POLARIZATIONS][NR_SAMPLES_PER_CHANNEL][NR_CHANNELS], ctx);

  MultiDimArrayHostBuffer<float, 3> delaysAtBegin  (boost::extents[NR_SAPS][NR_STATIONS][NR_POLARIZATIONS], ctx);
  MultiDimArrayHostBuffer<float, 3> delaysAfterEnd (boost::extents[NR_SAPS][NR_STATIONS][NR_POLARIZATIONS], ctx);
  MultiDimArrayHostBuffer<float, 2> phaseOffsets   (boost::extents[NR_STATIONS][NR_POLARIZATIONS], ctx);
  MultiDimArrayHostBuffer<float, 1> bandPassFactors(boost::extents[NR_CHANNELS], ctx);

  // set inputs
  for (size_t i = 0; i < filteredData->num_elements(); i++) {
    filteredData->origin()[i].real() = 1.0f;
    filteredData->origin()[i].imag() = 1.0f;
  }
  for (size_t i = 0; i < delaysAtBegin.num_elements(); i++) {
    delaysAtBegin.origin()[i] = delayBegin;
  }
  for (size_t i = 0; i < delaysAfterEnd.num_elements(); i++) {
    delaysAfterEnd.origin()[i] = delayEnd;
  }
  for (size_t i = 0; i < phaseOffsets.num_elements(); i++) {
    phaseOffsets.origin()[i] = phaseOffset;
  }
  for (size_t i = 0; i < bandPassFactors.num_elements(); i++) {
    bandPassFactors.origin()[i] = bandPassFactor;
  }

  // set output for proper verification later
  for (size_t i = 0; i < correctedData->num_elements(); i++) {
    correctedData->origin()[i].real() = 42.0f;
    correctedData->origin()[i].imag() = 42.0f;
  }

  gpu::Function kfunc(initKernel(ctx, compileDefs));

  runKernel(kfunc, *correctedData, *filteredData,
            delaysAtBegin, delaysAfterEnd, phaseOffsets, bandPassFactors,
            subbandFrequency, beam);

  delete filteredData;

  // Tests that use this function only check the first and last 2 output floats.
  const unsigned nrResultVals = 2;
  assert(correctedData->num_elements() >= nrResultVals * sizeof(complex<float>) / sizeof(float));
  vector<complex<float> > outputrv(nrResultVals);
  outputrv[0] = correctedData->origin()[0];
  outputrv[1] = correctedData->origin()[correctedData->num_elements() - 1];
  delete correctedData;
  return outputrv;
}

TEST(BandPass)
{
  // ***********************************************************
  // Test if the bandpass correction factor is applied correctly in isolation
  float bandPassFactor = 2.0f;

  CompileDefinitions defs(getDefaultCompileDefinitions());

  // The input samples are all ones
  // After correction, multiply with 2.
  // The first and the last complex values are retrieved. They should be scaled with the bandPassFactor == 2
  vector<complex<float> > results(runTest<complex<float> >(
                    defs,
                    0.0f, // sb freq
                    0U,   // beam
                    0.0f, // delays begin
                    0.0f, // delays end
                    0.0f, // phase offsets
                    bandPassFactor)); // bandpass factor

  CHECK_CLOSE(2.0, results[0].real(), 0.00001);
  CHECK_CLOSE(2.0, results[0].imag(), 0.00001);
  CHECK_CLOSE(2.0, results[1].real(), 0.00001);
  CHECK_CLOSE(2.0, results[1].imag(), 0.00001);
}

TEST(PhaseOffsets)
{
  //**********************************************************************
  // Delaycompensation but only for the phase ofsets:
  // All computations the drop except the phase ofset of 1,0 which is fed into a cosisin (or sincos)
  // cosisin(pi) = -1
  CompileDefinitions defs(getDefaultCompileDefinitions());
  defs["DELAY_COMPENSATION"] = "1";
  defs["SUBBAND_BANDWIDTH"]  = "1.0f";

  vector<complex<float> > results(runTest<complex<float> >(
                    defs,
                    1.0f,   // sb freq
                    0U,     // beam
                    0.0f,   // delays begin  
                    0.0f,   // delays end
                    M_PI,   // phase offsets
                    1.0f)); // bandpass factor

  CHECK_CLOSE(-1.0, results[0].real(), 0.00001);
  CHECK_CLOSE(-1.0, results[0].imag(), 0.00001);
  CHECK_CLOSE(-1.0, results[1].real(), 0.00001);
  CHECK_CLOSE(-1.0, results[1].imag(), 0.00001);
}

SUITE(DelayCompensation)
{
  TEST(ConstantDelay)
  {
    //****************************************************************************
    // delays  begin and end both 1 no phase offset frequency 1 width 1
    // frequency = subbandFrequency - .5f * SUBBAND_BANDWIDTH + (channel + minor) * (SUBBAND_BANDWIDTH / NR_CHANNELS)
    //  (delaysbegin * - 2 * pi ) * (frequency == 0.5) == -3.14
    // cosisin(-3.14159+0 i) == -1
    CompileDefinitions defs(getDefaultCompileDefinitions());
    defs["DELAY_COMPENSATION"] = "1";
    defs["SUBBAND_BANDWIDTH"]  = "1.0f";

    vector<complex<float> > results(runTest<complex<float> >(
                      defs,
                      1.0f,   // sb freq
                      0U,     // beam
                      1.0f,   // delays begin  
                      1.0f,   // delays end
                      0.0f,   // phase offsets
                      1.0f)); // bandpass factor

    CHECK_CLOSE(-1.0, results[0].real(), 0.00001);
    CHECK_CLOSE(-1.0, results[0].imag(), 0.00001);

    // For verification: for the following vals, the kernel computes:
    // frequency = 1.0 - 0.5*1.0 + (0 + 15) * (1.0 / 16) = 0.5 + 15/16 = 1.4375
    // phiBegin = -2.0 * 3.1415 * delayAtBegin = -6.8232 * 1.0 = -6.8232
    // deltaPhi = (phiEnd - phiBegin) / 64 = 0
    // myPhiBegin = (-6.8232 + major (= offset within block of 16 samples) * deltaPhi) * frequency + phaseOffset
    //            = -6.8232 * 1.4375 + 0.0 = -9.032086
    // myPhiDelta = 16 (= time step) * deltaPhi * frequency = 0
    // vX = ( cos(myPhiBegin.x), sin(myPhiBegin.x) ) = (-0.923882, -0.382677)
    // vY = idem (as delays begin == delays end)
    // dvX = ( cos(myPhiDelta.x), sin(myPhiDelta.x) ) = (1, 0)
    // dvY = idem
    // (vX, vY) *= weight (*1.0)
    // sampleX = sampleY = (1.0, 1.0)
    // After 64/16 rounds, (vX, vY) have been updated 64/16 times with (dvX, dvY).
    //   In this case, (dvX, dvY) stays (1, 0), so for the last sample, we get:
    // sampleY = cmul(sampleY, vY) = -0.923882 - -0.382677 = -0.541205 (~ -0.541196) (real)
    //                             = -0.923882 + -0.382677 = -1.306559 (~ -1.30656)  (imag)
    CHECK_CLOSE(-0.541196, results[1].real(), 0.00001);
    CHECK_CLOSE(-1.30656 , results[1].imag(), 0.00001);
  }

  TEST(SlopedDelay)
  {
    //****************************************************************************
    // delays  begin 1 and end 0 no phase offset frequency 1 width 1
    // frequency = subbandFrequency - .5f * SUBBAND_BANDWIDTH + (channel + minor) * (SUBBAND_BANDWIDTH / NR_CHANNELS)
    //  (delaysbegin * - 2 * pi ) * (frequency == 0.5) == -3.14
    // cosisin(-3.14159+0 i) == -1
    // The later sets of samples are calculate as:
    // vX = vX * dvX;  The delays are multiplied because we are calculating with exponents
    // Ask john Romein for more details
    CompileDefinitions defs(getDefaultCompileDefinitions());
    defs["DELAY_COMPENSATION"] = "1";
    defs["SUBBAND_BANDWIDTH"]  = "1.0f";

    vector<complex<float> > results(runTest<complex<float> >(
                      defs,
                      1.0f,   // sb freq
                      0U,     // beam
                      1.0f,   // delays begin  
                      0.0f,   // delays end
                      0.0f,   // phase offsets
                      1.0f)); // bandpass factor

    CHECK_CLOSE(-1.0,     results[0].real(), 0.00001);
    CHECK_CLOSE(-1.0,     results[0].imag(), 0.00001);
    CHECK_CLOSE(1.130720, results[1].real(), 0.00001);
    CHECK_CLOSE(0.849399, results[1].imag(), 0.00001);
  }
}

TEST(AllAtOnce)
{
  //****************************************************************************
  // delays  begin 1 and end 0 no phase offset frequency 1 width 1
  // frequency = subbandFrequency - .5f * SUBBAND_BANDWIDTH + (channel + minor) * (SUBBAND_BANDWIDTH / NR_CHANNELS)
  //  (delaysbegin * - 2 * pi ) * (frequency == 0.5) == -3.14
  // cosisin(-3.14159+0 i) == -1
  // The later sets of samples are calculate as:
  // vX = vX * dvX;  The delays are multiplied because we are calculating with exponents
  // Ask john Romein for more details
  // In this test the phase offsets are also compensated
  CompileDefinitions defs(getDefaultCompileDefinitions());
  defs["DELAY_COMPENSATION"] = "1";
  defs["SUBBAND_BANDWIDTH"]  = "1.0f";

  vector<complex<float> > results(runTest<complex<float> >(
                    defs,
                    1.0f,   // sb freq
                    0U,     // beam
                    1.0f,   // delays begin  
                    0.0f,   // delays end
                    1.0f,   // phase offsets (correct with e = 2.71828)
                    2.0f)); // bandpass factor (weights == 2)

  CHECK_CLOSE( 0.602337, results[0].real(), 0.00001);
  CHECK_CLOSE(-2.763550, results[0].imag(), 0.00001);
  CHECK_CLOSE(-0.207632, results[1].real(), 0.00001);
  CHECK_CLOSE( 2.820790, results[1].imag(), 0.00001);
}



gpu::Stream initDevice()
{
  // Set up device (GPU) environment
  try {
    gpu::Platform pf;
    cout << "Detected " << pf.size() << " GPU devices" << endl;
  } catch (gpu::CUDAException& e) {
    cerr << e.what() << endl;
    exit(3); // test skipped
  }
  gpu::Device device(0);
  vector<gpu::Device> devices(1, device);
  gpu::Context ctx(device);
  gpu::Stream cuStream(ctx);

  return cuStream;
}

int main()
{
  INIT_LOGGER("tDelayAndBandPass");

  // init global(s): device, context/stream.
  gpu::Stream strm(initDevice());
  stream = &strm;

  int exitStatus = UnitTest::RunAllTests();
  return exitStatus > 0 ? 1 : 0;
}

