/* Generator.cc
 * Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
 * P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
 *
 * This file is part of the LOFAR software suite.
 * The LOFAR software suite is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The LOFAR software suite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id: $
 */

#include <lofar_config.h>

#include "Generator.h"

#include <boost/format.hpp>

#include <Common/LofarLogger.h>
#include <Stream/Stream.h>
#include <CoInterface/SmartPtr.h>
#include <CoInterface/Stream.h>


namespace LOFAR
{
  namespace Cobalt
  {

    Generator::Generator( const BufferSettings &settings, const std::vector<std::string> &streamDescriptors )
      :
      RSPBoards(str(boost::format("[station %s %s] [Generator] ") % settings.station.stationName % settings.station.antennaField), streamDescriptors.size()),
      settings(settings),
      streamDescriptors(streamDescriptors),
      nrSent(nrBoards, 0)
    {
      LOG_INFO_STR( logPrefix << "Initialised" );
    }

    void Generator::makePacket( size_t boardNr, struct RSP &packet, const TimeStamp &timestamp )
    {
      // configure the packet header
      packet.header.version = 3; // we emulate BDI 6.0

      packet.header.sourceInfo1 =
        (boardNr & 0x1F) | (settings.station.clockMHz == 200 ? 1 << 7 : 0);

      switch (settings.station.bitMode) {
      case 16:
        packet.header.sourceInfo2 = 0;
        break;

      case 8:
        packet.header.sourceInfo2 = 1;
        break;

      case 4:
        packet.header.sourceInfo2 = 2;
        break;
      }

      packet.header.nrBeamlets = settings.nrBeamletsPerBoard;
      packet.header.nrBlocks = 16;

      packet.header.timestamp = timestamp.getSeqId();
      packet.header.blockSequenceNumber = timestamp.getBlockId();

      // insert data that is different for each packet
      int64 data = timestamp;

      memset(packet.payload.data, data & 0xFF, sizeof packet.payload.data);

      // verify whether the packet really reflects what we intended
      ASSERT(packet.rspBoard()     == boardNr);
      ASSERT(packet.payloadError() == false);
      ASSERT(packet.bitMode()      == settings.station.bitMode);
      ASSERT(packet.clockMHz()     == settings.station.clockMHz);
    }

    void Generator::processBoard( size_t nr )
    {
      const std::string logPrefix(str(boost::format("[station %s %s board %u] [Generator] ") % settings.station.stationName % settings.station.antennaField % nr));

      try {
        LOG_INFO_STR( logPrefix << "Connecting to " << streamDescriptors[nr] );
        SmartPtr<Stream> s = createStream(streamDescriptors[nr], false);

        LOG_INFO_STR( logPrefix << "Start" );

        TimeStamp current(time(0L) + 1, 0, settings.station.clockMHz * 1000000);
        for(;; ) {
          struct RSP packet;

          // generate packet
          makePacket( nr, packet, current );

          ASSERT(packet.packetSize() <= sizeof packet);

          // wait until it is due
          if (!waiter.waitUntil(current))
            break;

          // send packet
          try {
            s->write(&packet, packet.packetSize());
          } catch (SystemCallException &ex) {
            // UDP can return ECONNREFUSED or EINVAL if server does not have its port open
            if (ex.error != ECONNREFUSED && ex.error != EINVAL)
              throw;
          }

          nrSent[nr]++;

          current += packet.header.nrBlocks;
        }
      } catch (Stream::EndOfStreamException &ex) {
        LOG_INFO_STR( logPrefix << "End of stream");
      } catch (SystemCallException &ex) {
        if (ex.error == EINTR)
          LOG_INFO_STR( logPrefix << "Aborted: " << ex.what());
        else
          LOG_ERROR_STR( logPrefix << "Caught Exception: " << ex);
      } catch (Exception &ex) {
        LOG_ERROR_STR( logPrefix << "Caught Exception: " << ex);
      }

      LOG_INFO_STR( logPrefix << "End");
    }

    void Generator::logStatistics()
    {
      for( size_t nr = 0; nr < nrBoards; nr++ ) {
        const std::string logPrefix(str(boost::format("[station %s %s board %u] [Generator] ") % settings.station.stationName % settings.station.antennaField % nr));

        LOG_INFO_STR( logPrefix << nrSent[nr] << " packets sent.");

        nrSent[nr] = 0;
      }
    }

  }
}

