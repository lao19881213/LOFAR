//# SubbandProc.h
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

#ifndef LOFAR_GPUPROC_CUDA_SUBBAND_PROC_H
#define LOFAR_GPUPROC_CUDA_SUBBAND_PROC_H

#include <string>
#include <map>

#include <Common/Timer.h>
#include <CoInterface/Parset.h>
#include <CoInterface/Pool.h>
#include <CoInterface/SmartPtr.h>
#include <CoInterface/BlockID.h>
#include <CoInterface/Config.h>
#include <CoInterface/SubbandMetaData.h>
#include <GPUProc/PerformanceCounter.h>
#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/MultiDimArrayHostBuffer.h>

// \file
// TODO: Update documentation

namespace LOFAR
{
  namespace Cobalt
  {
    //   Collect all inputData for the correlatorSubbandProc item:
    //    \arg inputsamples
    //    \arg delays
    //    \arg phaseOffSets
    //    \arg flags
    // It also contains a read function parsing all this data from an input stream.   
    class SubbandProcInputData
    {
    public:

      // The set of GPU buffers to link our host buffers to.
      // Device buffers may be reused between different pairs of kernels,
      // since device memory size is a concern.
      struct DeviceBuffers
      {
        gpu::DeviceMemory delaysAtBegin;
        gpu::DeviceMemory delaysAfterEnd;
        gpu::DeviceMemory phase0s;
        // We don't have tabDelays here, as it is only for bf.
        // It is transferred to devBeamFormerDelays declared in the bf SubbandProc,
        // similar to the bandpass correction and FIR filter weights (also not here).
        boost::shared_ptr<gpu::DeviceMemory> inputSamples;

        DeviceBuffers(size_t inputSamplesSize, size_t delaysSize, 
                      size_t phase0sSize, gpu::Context &context) :
          delaysAtBegin(context, delaysSize),
          delaysAfterEnd(context, delaysSize),
          phase0s(context, phase0sSize),
          inputSamples(new gpu::DeviceMemory(context, inputSamplesSize))
        {
        }
      };

      // Which block this InputData represents
      struct BlockID blockID;

      // Delays are computed and applied in double precision,
      // otherwise the to be computed phase shifts become too inprecise.

      //!< Whole sample delays at the start of the workitem      
      MultiDimArrayHostBuffer<double, 3> delaysAtBegin;

      //!< Whole sample delays at the end of the workitem      
      MultiDimArrayHostBuffer<double, 3> delaysAfterEnd;

      //!< Remainder of delays
      MultiDimArrayHostBuffer<double, 2> phase0s;

      //!< Delays for TABs (aka pencil beams) after station beam correction
      MultiDimArrayHostBuffer<double, 3> tabDelays;

      // inputdata with flagged data set to zero
      MultiDimArrayHostBuffer<char, 4> inputSamples;

      // The input flags
      MultiDimArray<SparseSet<unsigned>, 1> inputFlags;

      // CPU-side holder for the Meta Data
      std::vector<SubbandMetaData> metaData; // [station]

      // Create the inputData object we need shared host/device memory on the
      // supplied devicequeue
      SubbandProcInputData(size_t n_beams, size_t n_stations, 
                           size_t n_polarizations, size_t n_coherent_tabs, 
                           size_t n_samples, size_t bytes_per_complex_sample,
                           gpu::Context &context,
                           unsigned int hostBufferFlags = 0)
        :
        delaysAtBegin(boost::extents[n_beams][n_stations][n_polarizations],
                       context, hostBufferFlags),
        delaysAfterEnd(boost::extents[n_beams][n_stations][n_polarizations],
                       context, hostBufferFlags),
        phase0s(boost::extents[n_stations][n_polarizations],
                       context, hostBufferFlags),
        tabDelays(boost::extents[n_beams][n_stations][n_coherent_tabs],
                       context, hostBufferFlags),
        inputSamples(boost::extents[n_stations][n_samples][n_polarizations][bytes_per_complex_sample],
                       context, hostBufferFlags), // TODO: The size of the buffer is NOT validated
        inputFlags(boost::extents[n_stations]),
        metaData(n_stations)
      {
      }

      // Short-hand constructor pulling all relevant values from a Parset
      SubbandProcInputData(const Parset &ps,
                           gpu::Context &context,
                           unsigned int hostBufferFlags = 0)
        :
        delaysAtBegin(boost::extents[ps.settings.SAPs.size()][ps.settings.antennaFields.size()][NR_POLARIZATIONS],
                       context, hostBufferFlags),
        delaysAfterEnd(boost::extents[ps.settings.SAPs.size()][ps.settings.antennaFields.size()][NR_POLARIZATIONS],
                       context, hostBufferFlags),
        phase0s(boost::extents[ps.settings.antennaFields.size()][NR_POLARIZATIONS],
                       context, hostBufferFlags),
        tabDelays(boost::extents[ps.settings.SAPs.size()][ps.settings.antennaFields.size()][ps.settings.beamFormer.maxNrCoherentTABsPerSAP()],
                       context, hostBufferFlags),
        inputSamples(boost::extents[ps.settings.antennaFields.size()][ps.settings.blockSize][NR_POLARIZATIONS][ps.nrBytesPerComplexSample()],
                       context, hostBufferFlags), // TODO: The size of the buffer is NOT validated
        inputFlags(boost::extents[ps.settings.antennaFields.size()]),
        metaData(ps.settings.antennaFields.size())
      {
      }

      // process the given meta data 
      void applyMetaData(const Parset &ps, unsigned station,
                         unsigned SAP, const SubbandMetaData &metaData);

      // set all flagged inputSamples to zero.
      void flagInputSamples(unsigned station, const SubbandMetaData& metaData);
    };

    class SubbandProcOutputData
    {
    public:
      struct BlockID blockID;

      // Need a virtual destructor to make type polymorphic
      virtual ~SubbandProcOutputData() {}
    };

    /*
     * The SubbandProc does the following transformation:
     *   SubbandProcInputData -> SubbandProcOutputData
     *
     * The SubbandProcInputData represents one block of one subband
     * of input data, and the SubbandProcOutputData (for example) the complex
     * visibilities of such a block.
     *
     * For both input and output, a fixed set of objects is created,
     * tied to the GPU specific for the SubbandProc, for increased
     * performance. The objects are recycled by using Pool objects.
     *
     * The data flows as follows:
     *
     *   // Fetch the next input object to fill
     *   SmartPtr<SubbandProcInputData> input = queue.inputPool.free.remove();
     *
     *   // Provide input
     *   receiveInput(input);
     *
     *   // Annotate input
     *   input->blockID.block = block;
     *   input->blockID.globalSubbandIdx = subband;
     *   input->blockID.localSubbandIdx  = subbandIdx;
     *
     *   // Fetch the next output object to fill
     *   SmartPtr<SubbandProcOutputData> output = queue.outputPool.free.remove();
     *
     *   // Process block
     *   queue.doSubband(input, output);
     *
     *   // Give back input and output objects to queue
     *   queue.inputPool.free.append(input);
     *   queue.outputPool.free.append(output);
     *
     *   The queue.inputPool.filled and queue.outputPool.filled can be used to
     *   temporarily store filled input and output objects. Such is needed to
     *   obtain parallellism (i.e. read/process/write in separate threads).
     */
    class SubbandProc {
    public:
      SubbandProc(const Parset &ps, gpu::Context &context,
                  size_t nrSubbandsPerSubbandProc = 1);
      virtual ~SubbandProc();

      // TODO: clean up access by Pipeline class and move under protected
      std::map<std::string, SmartPtr<NSTimer> > timers;

      class Flagger
      {
      public:
        // 1.1 Convert the flags per station to channel flags, change time scale
        // if nchannel > 1
        static void convertFlagsToChannelFlags(Parset const &parset,
          MultiDimArray<SparseSet<unsigned>, 1> const &inputFlags,
          MultiDimArray<SparseSet<unsigned>, 2> &flagsPerChannel);

        // 1.3 Get the LOG2 of the input. Used to speed up devisions by 2
        static unsigned log2(unsigned n);
      };

      // A pool of input data, to allow items to be filled and
      // computed on in parallel.
      Pool<SubbandProcInputData> inputPool;

      // A pool of input data, that has been pre processed.
      Pool<SubbandProcInputData> processPool;

      // A pool of output data, to allow items to be filled
      // and written in parallel.
      Pool<SubbandProcOutputData> outputPool;

      // Correlate the data found in the input data buffer
      virtual void processSubband(SubbandProcInputData &input, SubbandProcOutputData &output) = 0;

      // Do post processing on the CPU.
      // \return Whether output must be sent to the output processor or
      // not. This feature is needed to do long-time integration (longer than
      // the maximum block size that can be processed on a GPU).
      virtual bool postprocessSubband(SubbandProcOutputData &output) = 0;

    protected:
      const Parset &ps;
      const size_t nrSubbandsPerSubbandProc;

      gpu::Stream queue;

      void addTimer(const std::string &name);

      // Returns the number of output elements to create to get a smooth
      // running pipeline.
      size_t nrOutputElements() const;
    };
  }
}

#endif

