//#  DH_Weight.cc:
//#
//#  Copyright (C) 2002
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
//#

#include <StationSim/DH_Weight.h>


DH_Weight::DH_Weight (const string& name, unsigned int nbeam)
: DataHolder    (name, "DH_Weight"),
  itsDataPacket (0),
  itsBuffer     (0),
  itsNbeam      (nbeam)
{}

DH_Weight::~DH_Weight()
{
  deallocate (itsDataPacket);
}

void DH_Weight::preprocess()
{
  // First delete possible buffers.
  postprocess();
  // Determine the number of bytes needed for DataPacket and buffer.
  unsigned int size = sizeof(DataPacket) + itsNbeam * sizeof(BufferType);
  itsDataPacket = allocate(size);
  // Fill in the data packet pointer and initialize the memory.
  DataPacket* dpPtr = static_cast<DataPacket*>(itsDataPacket);
  *dpPtr = DataPacket();
  // Fill in the buffer pointer and initialize the buffer.
  char* ptr = static_cast<char*>(itsDataPacket);
  itsBuffer = (BufferType*)(ptr + sizeof(DataPacket));
  for (unsigned int i=0; i<itsNbeam; i++) {
    itsBuffer[i] = 1;
  }
  // Initialize base class.
  setDataPacket (dpPtr, size);
}

void DH_Weight::postprocess()
{
  deallocate (itsDataPacket);
  itsBuffer = 0;
  setDefaultDataPacket();
}
