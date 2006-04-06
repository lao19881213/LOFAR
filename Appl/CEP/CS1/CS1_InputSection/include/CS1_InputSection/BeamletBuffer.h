//#  BeamletBuffer.h: a cyclic buffer that holds the beamlets from the rspboards
//#
//#  Copyright (C) 2006
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_CS1_INPUTSECTION_BEAMLETBUFFER_H
#define LOFAR_CS1_INPUTSECTION_BEAMLETBUFFER_H

// \file
// a cyclic buffer that holds the beamlets from the rspboards

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/lofar_vector.h>
#include <Common/lofar_complex.h>
#include <Common/Timer.h>
#include <CS1_InputSection/LockedRange.h>
#include <CS1_Interface/RSPTimeStamp.h>

namespace LOFAR 
{
  namespace CS1 
  {

    // \addtogroup CS1_InputSection
    // @{

    class Beamlet {
      i16complex Xpol;
      i16complex Ypol;
    };

    // A BeamletBuffer can hold the beamlets coming from the rspboards
    // It is implemented as a cyclic buffer (using the mapTime2Index method).
    // Locking is done using a LockedRange
    // This buffer also reshuffles the data. It comes in in packets of different subbands per timestep.
    // The data leaves as a time series per subband.
    class BeamletBuffer
    {
    public:
      BeamletBuffer(uint bufferSize, uint nSubbands, uint history, uint readWriteDelay);
      ~BeamletBuffer();

      // write elements in the buffer, return value is number of succesfully written elements
      uint writeElements(Beamlet* data, TimeStamp begin, uint nElements, uint stride);
      // get elements out of the buffer, return value is number of valid elements
      uint getElements(vector<Beamlet*> buffers, TimeStamp begin, uint nElements);

      TimeStamp startBufferRead();
      TimeStamp startBufferRead(TimeStamp);

      void setAllowOverwrite(bool o) {itsLockedRange.setOverwriting(o);};

      void clear() {itsLockedRange.clear();};

    private:
      // Copying is not allowed
      BeamletBuffer (const BeamletBuffer& that);
      BeamletBuffer& operator= (const BeamletBuffer& that);

      // Needed for mapping a timestamp to a place in the buffer
      TimeStamp time0;
      uint mapTime2Index(TimeStamp time) const { return (time - time0) % itsSize; };

      //# Datamembers
      vector<Beamlet *> itsSBBuffers;
      bool* itsInvalidFlags;
      uint itsNSubbands;
      uint itsSize;

      TimeStamp itsHighestWritten;
      
      LockedRange<TimeStamp, int> itsLockedRange;

      // These are for statistics
      uint itsDroppedItems;
      uint itsDummyItems;

      NSTimer itsWriteTimer;
      NSTimer itsReadTimer;

    };

    // @}

  } // namespace CS1
} // namespace LOFAR

#endif
