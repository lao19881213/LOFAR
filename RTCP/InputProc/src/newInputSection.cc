#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/Thread/Mutex.h>
#include <Stream/Stream.h>
#include <Stream/SocketStream.h>
#include <Interface/MultiDimArray.h>
#include <Interface/Stream.h>
#include "WallClockTime.h"
#include "SharedMemory.h"
#include "Ranges.h"
#include "OMPThread.h"
#include "StationID.h"
#include "BufferSettings.h"
#include "SampleType.h"
#include "SampleBuffer.h"
#include "SampleBufferReader.h"
#include "Generator.h"
#include "PacketsToBuffer.h"
#include "mpi.h"

#include <vector>
#include <omp.h>
#include <string>
#include <boost/format.hpp>

#define DURATION 60 
#define BLOCKSIZE 0.005
#define NRSTATIONS 3

using namespace LOFAR;
using namespace RTCP;

Mutex MPIMutex;

//#define USE_RMA

#ifdef USE_RMA
#include "obsolete/MPI_RMA.h"
#else
#include "MPITransferStations.h"
#endif


int main( int argc, char **argv )
{
  size_t clock = 200*1000*1000;

  typedef SampleType<i16complex> SampleT;
  const TimeStamp from(time(0L) + 1, 0, clock);
  const TimeStamp to(time(0L) + 1 + DURATION, 0, clock);
  const size_t blockSize = BLOCKSIZE * clock / 1024;
  std::map<unsigned, std::vector<size_t> > beamlets;

  struct StationID stationID("RS106", "LBA", clock, 16);
  struct BufferSettings settings;

  settings.station = stationID;
  settings.nrBeamlets = 244;
  settings.nrBoards = 4;

  settings.nrSamples = (5 * stationID.clock / 1024);// & ~0xFL;
  settings.nrFlagRanges = 64;

  settings.dataKey = stationID.hash();

  INIT_LOGGER(argv[0]);

  if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
    LOG_ERROR_STR("MPI_Init failed");
    return 1;
  }

  int nrHosts, rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nrHosts);

#ifdef USE_RMA
  int nrStations = NRSTATIONS;
#else
  int nrStations = NRSTATIONS;
#endif

  for (unsigned i = 0; i < 244; ++i)
    beamlets[nrStations + i % (nrHosts - nrStations)].push_back(i);

  if (rank > nrStations - 1) {
    // receiver
    LOG_INFO_STR("Receiver " << rank << " starts, handling " << beamlets[rank].size() << " subbands from " << nrStations << " stations." );

#ifdef USE_RMA
    std::vector<struct BufferSettings> stations(nrStations, settings);

    {
      MPISharedBufferReader<SampleT> receiver(stations, from, to, blockSize, beamlets[rank]);

      receiver.process(0.0);
    }  
#else
    std::vector<int> stationRanks(nrStations);

    for (size_t i = 0; i < stationRanks.size(); i++)
      stationRanks[i] = i;

    {
      MPIReceiveStations<SampleT> receiver(settings, stationRanks, beamlets[rank], blockSize);

      for(size_t block = 0; block < (to-from)/blockSize + 1; ++block) {
        receiver.receiveBlock();

        //LOG_INFO_STR("Receiver " << rank << " received block " << block);
      }
    }  
#endif
    LOG_INFO_STR("Receiver " << rank << " done");

    MPI_Finalize();
    return 0;
  }

  omp_set_nested(true);
  omp_set_num_threads(32);
  OMPThread::init();

  std::vector<std::string> inputStreams(4);
  inputStreams[0] = "udp:127.0.0.1:4346";
  inputStreams[1] = "udp:127.0.0.1:4347";
  inputStreams[2] = "udp:127.0.0.1:4348";
  inputStreams[3] = "udp:127.0.0.1:4349";

  if(rank == 0) {
    PacketsToBuffer< SampleT > station( settings, inputStreams );
    Generator generator( settings, inputStreams );

    #pragma omp parallel sections num_threads(4)
    {
      #pragma omp section
      { station.process(); }

      #pragma omp section
      { generator.process(); }

      #pragma omp section
      { sleep(DURATION + 1); station.stop(); sleep(1); generator.stop(); }

      #pragma omp section
      {
        struct StationID lookup("RS106", "HBA0");
        struct BufferSettings s(stationID);

        LOG_INFO_STR("Detected " << s);
#ifdef USE_RMA
        MPISharedBuffer<SampleT> streamer(s);
#else
        #pragma omp parallel for num_threads(nrHosts - nrStations)
        for (int i = nrStations; i < nrHosts; ++i) {
          LOG_INFO_STR("Connecting to receiver " << i );
          MPISendStation< SampleT > streamer(s, from, to, blockSize, beamlets[i], i );

          LOG_INFO_STR("Sending to receiver " << i );
          streamer.process( 0.0 );
        }
#endif
      }
    }
  } else {
      struct StationID lookup("RS106", "HBA0");
      struct BufferSettings s(stationID);

      LOG_INFO_STR("Detected " << s);
#ifdef USE_RMA
      MPISharedBuffer<SampleT> streamer(s);
#else
      #pragma omp parallel for num_threads(nrHosts - nrStations)
      for (int i = nrStations; i < nrHosts; ++i) {
        LOG_INFO_STR("Connecting to receiver " << i );
        MPISendStation< SampleT > streamer(s, from, to, blockSize, beamlets[i], i );

        LOG_INFO_STR("Sending to receiver " << i );
        streamer.process( 0.0 );
      }
#endif
  }

  MPI_Finalize();
}
