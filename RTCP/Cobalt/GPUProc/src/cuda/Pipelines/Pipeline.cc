//# Pipeline.cc
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

#include "Pipeline.h"

#include <boost/format.hpp>

#include <Common/LofarLogger.h>
#include <Common/Timer.h>
#include <Common/lofar_iomanip.h>
#include <ApplCommon/PosixTime.h>
#include <Stream/Stream.h>
#include <Stream/FileStream.h>
#include <Stream/NullStream.h>

#include <CoInterface/Stream.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/Kernels/Kernel.h>
#include <GPUProc/SubbandProcs/SubbandProc.h>
#include <InputProc/SampleType.h>
#include <InputProc/RSPTimeStamp.h>

#ifdef HAVE_MPI
#include <InputProc/Transpose/MPIReceiveStations.h>
#include <InputProc/Transpose/MPIProtocol.h>
#else
#include <GPUProc/Station/StationInput.h>
#endif

#include <cmath>

#define NR_WORKQUEUES_PER_DEVICE  1

// Actually do any processing.
//
// If not set, station input is received but discarded immediately.
#define DO_PROCESSING

namespace LOFAR
{
  namespace Cobalt
  {


    Pipeline::Pipeline(const Parset &ps, const std::vector<size_t> &subbandIndices, const std::vector<gpu::Device> &devices)
      :
      ps(ps),
      devices(devices),
      subbandIndices(subbandIndices),
      processingSubband0(std::find(subbandIndices.begin(), subbandIndices.end(), 0U) != subbandIndices.end()),
      workQueues(std::max(1UL, (profiling ? 1 : NR_WORKQUEUES_PER_DEVICE) * devices.size())),
      nrSubbandsPerSubbandProc(
        (subbandIndices.size() + workQueues.size() - 1) / workQueues.size()),
      performance(devices.size()),
      writePool(subbandIndices.size())
    {
      ASSERTSTR(!devices.empty(), "Not bound to any GPU!");
    }

    Pipeline::~Pipeline()
    {
    }

    template<typename SampleT> void Pipeline::MPIData::allocate( size_t nrStations, size_t nrBeamlets, size_t nrSamples )
    {
      data     = (char*)mpiAllocator.allocate( nrStations * nrBeamlets * nrSamples * sizeof(SampleT));
      metaData = (char*)mpiAllocator.allocate( nrStations * nrBeamlets * sizeof(struct MPIProtocol::MetaData));
    }

    template void Pipeline::MPIData::allocate< SampleType<i16complex> >( size_t nrStations, size_t nrBeamlets, size_t nrSamples );
    template void Pipeline::MPIData::allocate< SampleType<i8complex> >( size_t nrStations, size_t nrBeamlets, size_t nrSamples );
    template void Pipeline::MPIData::allocate< SampleType<i4complex> >( size_t nrStations, size_t nrBeamlets, size_t nrSamples );

    template<typename SampleT> void Pipeline::receiveInput( size_t nrBlocks )
    {
      // Need SubbandProcs to send work to
      ASSERT(workQueues.size() > 0);

      NSTimer receiveTimer("MPI: Receive station data", true, true);

      // The length of a block in samples
      size_t blockSize = ps.nrSamplesPerSubband();

      // RECEIVE: Set up to receive our subbands as indicated by subbandIndices
#ifdef HAVE_MPI
      MPIReceiveStations receiver(ps.nrStations(), subbandIndices, blockSize);

      for (size_t i = 0; i < 4; i++) {
        SmartPtr<struct MPIData> mpiData = new MPIData;

        mpiData->allocate<SampleT>(ps.nrStations(), subbandIndices.size(), blockSize);

        mpiPool.free.append(mpiData);
      }

#else
      DirectInput &receiver = DirectInput::instance();
#endif

      // Receive input from StationInput::sendInputToPipeline.
      //
      // Start processing from block -1, and don't process anything if the
      // observation is empty.
      for (ssize_t block = -1; nrBlocks > 0 && block < ssize_t(nrBlocks); block++) {
        // Receive the samples of all subbands from the stations for this
        // block.

        LOG_INFO_STR("[block " << block << "] Collecting input buffers");

        SmartPtr<struct MPIData> mpiData = mpiPool.free.remove();

        mpiData->block = block;

        MultiDimArray<SampleT,3> data(
          boost::extents[ps.nrStations()][subbandIndices.size()][ps.nrSamplesPerSubband()],
          (SampleT*)mpiData->data.get(), false);

        MultiDimArray<struct MPIProtocol::MetaData,2> metaData(
          boost::extents[ps.nrStations()][subbandIndices.size()],
          (struct MPIProtocol::MetaData*)mpiData->metaData.get(), false);

        // Receive all subbands from all stations
        LOG_INFO_STR("[block " << block << "] Receive input");

        if (block > 2) receiveTimer.start();
        receiver.receiveBlock<SampleT>(data, metaData);
        if (block > 2) receiveTimer.stop();

        if (processingSubband0)
          LOG_INFO_STR("[block " << block << "] Input received");
        else
          LOG_INFO_STR("[block " << block << "] Input received");

        mpiPool.filled.append(mpiData);
      }

      // Signal end of input
      mpiPool.filled.append(NULL);
    }

    template void Pipeline::receiveInput< SampleType<i16complex> >( size_t nrBlocks );
    template void Pipeline::receiveInput< SampleType<i8complex> >( size_t nrBlocks );
    template void Pipeline::receiveInput< SampleType<i4complex> >( size_t nrBlocks );

    void Pipeline::receiveInput( size_t nrBlocks )
    {
      switch (ps.nrBitsPerSample()) {
      default:
      case 16:
        receiveInput< SampleType<i16complex> >(nrBlocks);
        break;
      case 8:
        receiveInput< SampleType<i8complex> >(nrBlocks);
        break;
      case 4:
        receiveInput< SampleType<i4complex> >(nrBlocks);
        break;
      }
    }


    void Pipeline::allocateResources()
    {
      for (size_t i = 0; i < writePool.size(); i++) {
        // Allow 10 blocks to be in the best-effort queue.
        // TODO: make this dynamic based on memory or time
        writePool[i].bequeue = new BestEffortQueue< SmartPtr<SubbandProcOutputData> >(3, ps.realTime());
      }
    }


    void Pipeline::processObservation()
    {

      LOG_INFO("----- Allocating resources");
      allocateResources();


      //sections = program segments defined by the following omp section directive
      //           are distributed for parallel execution among available threads
      //parallel = directive explicitly instructs the compiler to parallelize the chosen block of code.
      //  The two sections in this function are done in parallel with a seperate set of threads.
#     pragma omp parallel sections num_threads(6)
      {
        /*
         * BLOCK OF SUBBANDS -> MPIQUEUE
         */
#       pragma omp section
        {
          size_t nrBlocks = floor((ps.settings.stopTime - ps.settings.startTime) / ps.settings.blockDuration());

          receiveInput(nrBlocks);
        }

        /*
         * MPIQUEUE -> WORKQUEUE INPUTPOOL
         */
#       pragma omp section
        {
          transposeInput();
        }

        /*
         * WORKQUEUE INPUTPOOL -> PROCESSPOOL
         *
         * Perform pre-processing, one thread per workQueue.
         */
#       pragma omp section
        {
#         pragma omp parallel for num_threads(workQueues.size())
          for (size_t i = 0; i < workQueues.size(); ++i) {
            SubbandProc &queue = *workQueues[i];

            // run the queue
            preprocessSubbands(queue);

            queue.processPool.filled.append(NULL);
          }
        }


        /*
         * WORKQUEUE INPUTPOOL -> WORKQUEUE OUTPUTPOOL
         *
         * Perform GPU processing, one thread per workQueue.
         */
#       pragma omp section
        {
#         pragma omp parallel for num_threads(workQueues.size())
          for (size_t i = 0; i < workQueues.size(); ++i) 
          {
            SubbandProc &queue = *workQueues[i];

            // run the queue
            //queue.timers["CPU - total"]->start();
            processSubbands(queue);
            //queue.timers["CPU - total"]->stop();

            // Signal end of output
            queue.outputPool.filled.append(NULL);
          }
        }

        /*
         * WORKQUEUE OUTPUTPOOL -> WRITEPOOL
         *
         * Perform post-processing, one thread per workQueue.
         */
#       pragma omp section
        {
#         pragma omp parallel for num_threads(workQueues.size())
          for (size_t i = 0; i < workQueues.size(); ++i) {
            SubbandProc &queue = *workQueues[i];

            // run the queue
            postprocessSubbands(queue);
          }

          // Signal end of output
          for (size_t i = 0; i < writePool.size(); ++i) {
            writePool[i].bequeue->noMore();
          }
        }

        /*
         * WRITEPOOL -> STORAGE STREAMS (best effort)
         */
#       pragma omp section
        {
#         pragma omp parallel for num_threads(writePool.size())
          for (size_t i = 0; i < writePool.size(); ++i) {
            writeOutput(subbandIndices[i], writePool[i]);
          }
        }
      }

      // gather performance figures
      for (size_t i = 0; i < workQueues.size(); ++i ) {
        performance.addQueue(*workQueues[i]);
      }

      // log performance figures
      performance.log(workQueues.size());
    }


    template<typename SampleT>
    void Pipeline::transposeInput()
    {
      SmartPtr<struct MPIData> input;

      NSTimer copyTimer("transpose input data", true, true);

      // Keep fetching input objects until end-of-output
      while ((input = mpiPool.filled.remove()) != NULL) {
        const ssize_t block = input->block;

        vector<size_t> nrFlaggedSamples(ps.nrStations(), 0);

#ifdef DO_PROCESSING
        MultiDimArray<SampleT,3> data(
          boost::extents[ps.nrStations()][subbandIndices.size()][ps.nrSamplesPerSubband()],
          (SampleT*)input->data.get(), false);

        MultiDimArray<struct MPIProtocol::MetaData,2> metaData(
          boost::extents[ps.nrStations()][subbandIndices.size()],
          (struct MPIProtocol::MetaData*)input->metaData.get(), false);

        // The set of InputData objects we're using for this block.
        vector< SmartPtr<SubbandProcInputData> > inputDatas(subbandIndices.size());

        for (size_t subbandIdx = 0; subbandIdx < subbandIndices.size(); ++subbandIdx) {
          // Fetch an input object to store this subband.
          SubbandProc &queue = *workQueues[subbandIdx % workQueues.size()];

          // Fetch an input object to fill from the selected queue.
          SmartPtr<SubbandProcInputData> subbandData = queue.inputPool.free.remove();

          // Annotate the block
          struct BlockID id;
          id.block                 = block;
          id.globalSubbandIdx      = subbandIndices[subbandIdx];
          id.localSubbandIdx       = subbandIdx;
          id.subbandProcSubbandIdx = subbandIdx / workQueues.size();
          subbandData->blockID = id;

          copyTimer.start();
          for (size_t stat = 0; stat < ps.nrStations(); ++stat) {
            // Copy the data
#if 1
            memcpy(&subbandData->inputSamples[stat][0][0][0],
                   &data[stat][subbandIdx][0],
                   ps.nrSamplesPerSubband() * sizeof(SampleT));
#endif
            // Copy the metadata
            subbandData->metaData[stat] = metaData[stat][subbandIdx];

            nrFlaggedSamples[stat] += subbandData->metaData[stat].flags.count();
          }
          copyTimer.stop();

          queue.inputPool.filled.append(subbandData);
        }
#endif

        mpiPool.free.append(input);
        ASSERT(!input);

        // Report flags per station
        stringstream flagStr;  // stations with >0% flags
        stringstream cleanStr; // stations with  0% flags

        for (size_t stat = 0; stat < ps.nrStations(); ++stat) {
          const double flagPerc = 100.0 * nrFlaggedSamples[stat] / subbandIndices.size() / ps.nrSamplesPerSubband();

          if (flagPerc == 0.0)
            cleanStr << str(boost::format("%s, ") % ps.settings.stations[stat].name);
          else
            flagStr << str(boost::format("%s: %.1f%%, ") % ps.settings.stations[stat].name % flagPerc);
        }

        LOG_DEBUG_STR("[block " << block << "] No flagging: " << cleanStr.str());

        if (!flagStr.str().empty()) {
          LOG_WARN_STR("[block " << block << "] Flagging:    " << flagStr.str());
        }

        LOG_DEBUG_STR("[block " << block << "] Forwarded input to pre processing");
      }

      // Signal end of input
      for (size_t i = 0; i < workQueues.size(); ++i) {
        workQueues[i]->inputPool.filled.append(NULL);
      }
    }

    template void Pipeline::transposeInput< SampleType<i16complex> >();
    template void Pipeline::transposeInput< SampleType<i8complex> >();
    template void Pipeline::transposeInput< SampleType<i4complex> >();

    void Pipeline::transposeInput()
    {
      switch (ps.nrBitsPerSample()) {
      default:
      case 16:
        transposeInput< SampleType<i16complex> >();
        break;
      case 8:
        transposeInput< SampleType<i8complex> >();
        break;
      case 4:
        transposeInput< SampleType<i4complex> >();
        break;
      }
    }


    void Pipeline::preprocessSubbands(SubbandProc &workQueue)
    {
      SmartPtr<SubbandProcInputData> input;

      NSTimer preprocessTimer("preprocess", true, true);

      // Keep fetching input objects until end-of-output
      while ((input = workQueue.inputPool.filled.remove()) != NULL) {
        const struct BlockID &id = input->blockID;

        LOG_DEBUG_STR("[" << id << "] Pre processing start");

        /* PREPROCESS START */
        preprocessTimer.start();

        const unsigned SAP = ps.settings.subbands[id.globalSubbandIdx].SAP;

        // Translate the metadata as provided by receiver
        for (size_t stat = 0; stat < ps.nrStations(); ++stat) {
          input->applyMetaData(ps, stat, SAP, input->metaData[stat]);
        }

        preprocessTimer.stop();
        /* PREPROCESS END */

        // Hand off output to processing
        workQueue.processPool.filled.append(input);
        ASSERT(!input);

        LOG_DEBUG_STR("[" << id << "] Forwarded input to processing");
      }
    }


    void Pipeline::processSubbands(SubbandProc &workQueue)
    {
      SmartPtr<SubbandProcInputData> input;

      NSTimer processTimer("process", true, true);

      // Keep fetching input objects until end-of-input
      while ((input = workQueue.processPool.filled.remove()) != NULL) {
        const struct BlockID id = input->blockID;

        LOG_DEBUG_STR("[" << id << "] Processing start");

        // Also fetch an output object to store results
        SmartPtr<SubbandProcOutputData> output = workQueue.outputPool.free.remove();

        // Only _we_ signal end-of-data, so we should _never_ receive it
        ASSERT(output != NULL); 

        output->blockID = id;

        // Perform calculations
        processTimer.start();
        workQueue.processSubband(*input, *output);
        processTimer.stop();

        if (id.block < 0) {
          // Ignore block; only used to initialize FIR history samples
          workQueue.outputPool.free.append(output);
        } else {
          // Hand off output to post processing
          workQueue.outputPool.filled.append(output);
        }
        ASSERT(!output);

        // Give back input data for a refill
        workQueue.inputPool.free.append(input);
        ASSERT(!input);

        LOG_DEBUG_STR("[" << id << "] Forwarded output to post processing");
      }
    }


    void Pipeline::postprocessSubbands(SubbandProc &workQueue)
    {
      SmartPtr<SubbandProcOutputData> output;

      NSTimer postprocessTimer("postprocess", true, true);

      size_t nrBlocksForwarded = 0;
      size_t nrBlocksDropped = 0;
      time_t lastLogTime = 0;

      // Keep fetching output objects until end-of-output
      while ((output = workQueue.outputPool.filled.remove()) != NULL) {
        const struct BlockID id = output->blockID;

        LOG_DEBUG_STR("[" << id << "] Post processing start");

        postprocessTimer.start();
        bool handOffOutput = workQueue.postprocessSubband(*output);
        postprocessTimer.stop();

        if (!handOffOutput) {
          workQueue.outputPool.free.append(output);
          ASSERT(!output);
          continue;
        }

        // Hand off output, force in-order as Storage expects it that way
        struct Output &pool = writePool[id.localSubbandIdx];

        if (pool.bequeue->append(output)) {
          nrBlocksForwarded++;
        } else {
          nrBlocksDropped++;
          // LOG_WARN_STR("[block " << block << "] Dropped for subband " <<
          //              globalSubbandIdx);
          // Give back to queue
          workQueue.outputPool.free.append(output);
        }
        ASSERT(!output);

        LOG_DEBUG_STR("[" << id << "] Forwarded output to writer");

        // Log every 5 seconds (note: time() returns time in sec.)
        if (time(0) > lastLogTime + 5) {
          lastLogTime = time(0);
          LOG_INFO_STR("Forwarded " << nrBlocksForwarded << 
                       " blocks, dropped " << nrBlocksDropped << " blocks");
        }
      }
    }


    Pipeline::Performance::Performance(size_t nrGPUs):
      nrGPUs(nrGPUs)
    {
    }


    void Pipeline::Performance::addQueue(SubbandProc &/*queue*/)
    {
      ScopedLock sl(totalsMutex);

      //// add performance counters
      //for (map<string, SmartPtr<PerformanceCounter> >::iterator i = queue.counters.begin(); i != queue.counters.end(); ++i) {

      //  const string &name = i->first;
      //  PerformanceCounter *counter = i->second.get();

      //  counter->waitForAllOperations();

      //  total_counters[name] += counter->getTotal();
      //}

      //// add timers
      //for (map<string, SmartPtr<NSTimer> >::iterator i = queue.timers.begin(); i != queue.timers.end(); ++i) {

      //  const string &name = i->first;
      //  NSTimer *timer = i->second.get();

      //  if (!total_timers[name])
      //    total_timers[name] = new NSTimer(name, false, false);

      //  *total_timers[name] += *timer;
      //}
    }
    
    void Pipeline::Performance::log(size_t /*nrSubbandProcs*/)
    {
      //// Group figures based on their prefix before " - ", so "compute - FIR"
      //// belongs to group "compute".
      //map<string, PerformanceCounter::figures> counter_groups;

      //for (map<string, PerformanceCounter::figures>::const_iterator i = total_counters.begin(); i != total_counters.end(); ++i) {
      //  size_t n = i->first.find(" - ");

      //  // discard counters without group
      //  if (n == string::npos)
      //    continue;

      //  // determine group name
      //  string group = i->first.substr(0, n);

      //  // add to group
      //  counter_groups[group] += i->second;
      //}

      //// Log all performance totals at DEBUG level
      //for (map<string, PerformanceCounter::figures>::const_iterator i = total_counters.begin(); i != total_counters.end(); ++i) {
      //  LOG_DEBUG_STR(i->second.log(i->first));
      //}

      //for (map<string, SmartPtr<NSTimer> >::const_iterator i = total_timers.begin(); i != total_timers.end(); ++i) {
      //  LOG_DEBUG_STR(*(i->second));
      //}

      //// Log all group totals at INFO level
      //for (map<string, PerformanceCounter::figures>::const_iterator i = counter_groups.begin(); i != counter_groups.end(); ++i) {
      //  LOG_INFO_STR(i->second.log(i->first));
      //}

      //// Log specific performance figures for regression tests at INFO level
      //double wall_seconds = total_timers["CPU - total"]->getAverage();
      //double gpu_seconds = counter_groups["compute"].runtime / nrGPUs;
      //double spin_seconds = total_timers["GPU - wait"]->getAverage();
      //double input_seconds = total_timers["CPU - read input"]->getElapsed() / nrSubbandProcs;
      //double cpu_seconds = total_timers["CPU - process"]->getElapsed() / nrSubbandProcs;
      //double postprocess_seconds = total_timers["CPU - postprocess"]->getElapsed() / nrSubbandProcs;

      //LOG_INFO_STR("Wall seconds spent processing        : " << fixed << setw(8) << setprecision(3) << wall_seconds);
      //LOG_INFO_STR("GPU  seconds spent computing, per GPU: " << fixed << setw(8) << setprecision(3) << gpu_seconds);
      //LOG_INFO_STR("Spin seconds spent polling, per block: " << fixed << setw(8) << setprecision(3) << spin_seconds);
      //LOG_INFO_STR("CPU  seconds spent on input,   per SbProc: " << fixed << setw(8) << setprecision(3) << input_seconds);
      //LOG_INFO_STR("CPU  seconds spent processing, per SbProc: " << fixed << setw(8) << setprecision(3) << cpu_seconds);
      //LOG_INFO_STR("CPU  seconds spent postprocessing, per SbProc: " << fixed << setw(8) << setprecision(3) << postprocess_seconds);
    }

  }
}

