//#  BufferController.cc: Control cyclic buffers for subbands and metadata
//#
//#  Copyright (C) 2002-2005
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
#include <lofar_config.h>

#include <math.h>
#include <stdlib.h>
#include <CEPFrame/Lock.h>
#include <TFC_InputSection/BufferController.h>
#include <Common/hexdump.h>

namespace LOFAR {


BufferIndex::BufferIndex(int maxidx) :
  itsIndex(0),
  itsMaxIndex(maxidx)
{
}

void BufferIndex::checkIndex()
{
  if (itsIndex >= itsMaxIndex) {
    itsIndex -= itsMaxIndex;
  }
  else if (itsIndex < 0) {
    itsIndex += itsMaxIndex;
  }
}

int BufferIndex::getIndex()
{
  return itsIndex;
}

void BufferIndex::operator+= (int increment)
{
  itsIndex += increment;
  checkIndex();
}

void BufferIndex::operator++ (int)
{
  itsIndex++;
  checkIndex();
}

int BufferIndex::operator+ (int increment)
{
  return itsIndex + increment;
}

void BufferIndex::operator-= (int decrement)
{
  itsIndex -= decrement;
  checkIndex();
}

void BufferIndex::operator-- (int)
{
  itsIndex--;
  checkIndex();
}

int BufferIndex::operator- (BufferIndex& other)
{
  int increment = itsIndex-other.itsIndex;  
  if (increment < 0) {
    return increment + itsMaxIndex;
  }
  return increment; 
}

bool BufferIndex::operator== (BufferIndex& other)
{
  return (itsIndex == other.itsIndex);
}


BufferController::BufferController(int buffersize, int nsubbands)
  : itsBufferSize(buffersize),
    itsNSubbands(nsubbands),
    itsHead(buffersize),
    itsTail(buffersize),
    itsOldHead(buffersize),
    itsOldTail(buffersize),
    itsOverwritingAllowed(true)
{
  // create metadata buffer
  itsMetadataBuffer = new MetadataType[itsBufferSize];
  
  // create subbands buffer
  for (int s=0; s<itsNSubbands; s++) {
    itsSubbandBuffer.push_back(new SubbandType[itsBufferSize]);
  }
  
  // init buffer lock and -triggers
  pthread_mutex_init(&buffer_mutex, NULL);
  pthread_cond_init (&data_available,  NULL);
  pthread_cond_init (&space_available, NULL);
}

BufferController::~BufferController()
{
  vector<SubbandType*>::iterator sit = itsSubbandBuffer.begin();
  for (; sit!=itsSubbandBuffer.end(); sit++) {
    delete *sit;
  }
  itsSubbandBuffer.clear();

  delete itsMetadataBuffer;
}
 
timestamp_t BufferController::getOldestStamp()
{
  int bid;

  pthread_mutex_lock(&buffer_mutex);

  // wait until at least one element is available
  while (getCount() <= 0) 
  {
    pthread_cond_wait(&data_available, &buffer_mutex);
  }
  // CONDITION: Count > 0
  bid = itsTail.getIndex();

  pthread_mutex_unlock(&buffer_mutex);

  return itsMetadataBuffer[bid].timestamp;
}

timestamp_t BufferController::getNewestStamp()
{
  int bid;

  pthread_mutex_lock(&buffer_mutex);

  // wait until at least one element is available
  while (getCount() <= 0) 
  {
    pthread_cond_wait(&data_available, &buffer_mutex);
  }
  // CONDITION: Count > 0

  // this method is called before the first read,
  // so it is allowed to move the tails;
  bid = itsOldHead.getIndex() - 1;

  pthread_mutex_unlock(&buffer_mutex);

  return itsMetadataBuffer[bid].timestamp;
}

int BufferController::setReadOffset(int offset)
{
  int bid;

  pthread_mutex_lock(&buffer_mutex);

  // wait until enough data becomes available
  while (getCount() - offset < MIN_COUNT)
  {
    pthread_cond_wait(&data_available, &buffer_mutex);
  }
  
  // This method is called when there is no reader,
  // so tail == oldTail
  itsTail += offset;
  itsOldTail = itsTail;
  bid = itsTail.getIndex();
   
  pthread_mutex_unlock(&buffer_mutex);

  return bid;
}

int BufferController::setRewriteOffset(int offset)
{

  pthread_mutex_lock(&buffer_mutex);

  // check if there are enough elements in buffer
  if (offset >= getCount()) {
    return -1;
  } 
  // CONDITION: offset + nelements > Count
   
  itsOldHead = itsTail + offset;

  pthread_mutex_unlock(&buffer_mutex);

  return itsOldHead.getIndex();
}

int BufferController::getWritePtr()
{
  int bid;

  pthread_mutex_lock(&buffer_mutex);
  
  // wait until space becomes available
  while ((itsHead - itsOldTail >= MAX_COUNT) && !itsOverwritingAllowed)
  {
    pthread_cond_wait(&space_available, &buffer_mutex);
  }
  
  // CONDITION: Count < MAX_COUNT 
  bid = itsHead.getIndex();
  itsHead++;

  // if allowed, overwrite previous written elements
  if (itsHead == itsTail && itsOverwritingAllowed) {
    // push tail forwards
    itsTail++;
    itsOldTail++;
  }
  
  pthread_mutex_unlock(&buffer_mutex);

  return bid;
}

int BufferController::getReadPtr()
{
  int bid;

  pthread_mutex_lock(&buffer_mutex);

  // overwriting not allowed when reading is started
  itsOverwritingAllowed = false;
  
  // wait until enough elements are available
  while (getCount() < MIN_COUNT) 
  {
    pthread_cond_wait(&data_available, &buffer_mutex);
  }
  
  // CONDITION: Count >= MIN_COUNT 
  bid = itsTail.getIndex();
  itsTail++;
  
  pthread_mutex_unlock(&buffer_mutex);
  
  return bid;
}

void BufferController::releaseWriteBlock()
{
  pthread_mutex_lock(&buffer_mutex);
  
  // synchronize writepointers
  itsOldHead = itsHead;

  // signal that data has become available 
  pthread_cond_broadcast(&data_available);

  pthread_mutex_unlock(&buffer_mutex);
 
}

void BufferController::releaseReadBlock()
{
  pthread_mutex_lock(&buffer_mutex);
  
  // synchronize readpointers
  itsOldTail = itsTail;
  
  // signal that space has become available
  pthread_cond_broadcast(&space_available);

  pthread_mutex_unlock(&buffer_mutex);
}

void BufferController::releaseRewriteBlock()
{
  pthread_mutex_lock(&buffer_mutex);
  
  // synchronize writepointers
  itsOldHead = itsHead;

  pthread_mutex_unlock(&buffer_mutex);
 
}

timestamp_t BufferController::startBufferRead()
{
  // start reading so overwriting not allowed anymore 
  itsOverwritingAllowed = false;
 
  // get oldest stamp
  timestamp_t oldestStamp = getOldestStamp();

  // get newest stamp
  timestamp_t newestStamp = getNewestStamp();

  // set offset
  setReadOffset(newestStamp - oldestStamp);

  return newestStamp;
}

void BufferController::startBufferRead(timestamp_t stamp)
{
  // start reading so overwriting not allowed anymore 
  itsOverwritingAllowed = false;
  
  // get oldest stamp
  timestamp_t oldestStamp = getOldestStamp();

  // set offset
  setReadOffset(stamp - oldestStamp);
}

void BufferController::getElements(vector<SubbandType*> buf, int& invalidcount, timestamp_t startstamp, int nelements)
{
  // get oldest timestamp
  timestamp_t oldestStamp = getOldestStamp();
  
  // calculate offset
  int offset = startstamp - oldestStamp;

  // check offset
  ASSERTSTR(std::abs(offset) <= MAX_OFFSET , 
	       "BufferController: timestamp offset invalid");

  // set offset, get startindex
  int sid = setReadOffset(offset);

  // get metadata for requested block
  invalidcount = 0;
  int bid;
  for (int m=0; m<nelements; m++) {
    bid = getReadPtr(); 
    if (itsMetadataBuffer[bid].invalid != 0) {
      invalidcount++;
    }
  }
  
  // get subbands for requested block
  int n1, n2;
  for (int s=0; s<itsNSubbands; s++) {
    if (sid + nelements  > itsBufferSize) {
      // do copy in 2 blocks because end of subband data array will be crossed 
      n1 = itsBufferSize - sid;
      memcpy(buf[s], &(itsSubbandBuffer[s][sid]), n1*sizeof(SubbandType));

      n2 = nelements - n1;
      memcpy(&(buf[s][n1]), &(itsSubbandBuffer[s][0]), n2*sizeof(SubbandType));
    }
    else {
      // copy can be executed in one block
      memcpy(buf[s], &(itsSubbandBuffer[s][sid]), nelements*sizeof(SubbandType));
    }   
  }
 
  // reading is done, free block
  releaseReadBlock();
}

void BufferController::writeElements(SubbandType* buf, timestamp_t rspstamp)
{
  // write the metadata
  int bid;
  bid = getWritePtr();
  itsMetadataBuffer[bid].invalid = 0;
  itsMetadataBuffer[bid].timestamp = rspstamp;
  
  
  // write the subbanddata
  for (int s=0; s<itsNSubbands; s++) {
    memcpy(&(itsSubbandBuffer[s][bid]), &buf[s], sizeof(SubbandType));
  }
 
  // writing is done, free block
  releaseWriteBlock();
}

void BufferController::writeDummy(SubbandType* dum, timestamp_t startstamp, int nelements)
{
  int bid, sid;

  // write the metadata for this dummy block
  for (int i=0; i<nelements; i++) {
    bid = getWritePtr();
    itsMetadataBuffer[bid].invalid = 0;
    itsMetadataBuffer[bid].timestamp = startstamp;
    if (i==0) {
      sid = bid;
    }
    startstamp++;
  }

  // write the subbanddata
  for (int s=0; s<itsNSubbands; s++) {
    if (sid + nelements  > itsBufferSize) {
      // do copy in 2 blocks because end of subband data array will be crossed 
      int n1 = itsBufferSize - sid;
      memcpy(&(itsSubbandBuffer[s][sid]), dum, n1*sizeof(SubbandType));

      int n2 = nelements - n1;
      memcpy(&(itsSubbandBuffer[s][0]), &dum[n1], n2*sizeof(SubbandType));
    }
    else {
      // copy can be executed in one block
      memcpy(&(itsSubbandBuffer[s][sid]), dum, nelements*sizeof(SubbandType));
    }
  }
}

bool BufferController::rewriteElements(SubbandType* buf, timestamp_t startstamp)
{
  // get oldest timestamp
  timestamp_t oldestStamp = getOldestStamp();

  // calculate offset
  int offset = startstamp - oldestStamp;

  // set offset, get startindexr
  int bid = setRewriteOffset(offset);
  if (bid == -1) {
    return false;
  }
 
  // rewrite the metadata 
  itsMetadataBuffer[bid].invalid = 0;
  
  // rewrite the subbanddata
  for (int s=0; s<itsNSubbands; s++) {
    memcpy(&(itsSubbandBuffer[s][bid]), &buf[s], sizeof(SubbandType));
  }
 
  // rewriting is done, free block
  releaseRewriteBlock();

  return true;
}


int BufferController::getCount()
{
  return itsOldHead - itsTail;
}

}
