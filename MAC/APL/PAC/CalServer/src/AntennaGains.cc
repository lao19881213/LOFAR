//#  -*- mode: c++ -*-
//#  AntennaGains.cc: implementation of the AntennaGains class.
//#
//#  Copyright (C) 2002-2004
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

#include "AntennaGains.h"
#include "Marshalling.h"
#include <Common/LofarTypes.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace CAL;

AntennaGains::AntennaGains()
{
#ifdef USE_CAL_THREAD
  pthread_mutex_init(&m_mutex, 0);
#endif
  lock(); m_done = false; unlock();
}

AntennaGains::AntennaGains(int nantennas, int npol, int nsubbands)
{
#ifdef USE_CAL_THREAD
  pthread_mutex_init(&m_mutex, 0);
#endif
  lock(); m_done = false; unlock();

  if (nantennas < 0 || npol < 0 || nsubbands < 0)
    {
      nantennas = 0;
      npol = 0;
      nsubbands = 0;
    }
   
  m_gains.resize(nantennas, npol, nsubbands);
  m_gains = 1;

  m_quality.resize(nantennas, npol, nsubbands);
  m_quality = 1;
}

AntennaGains::~AntennaGains()
{}

unsigned int AntennaGains::getSize()
{
  return 
      MSH_ARRAY_SIZE(m_gains, complex<double>)
    + MSH_ARRAY_SIZE(m_quality, double)
    + sizeof(bool);
}

unsigned int AntennaGains::pack(void* buffer)
{
  unsigned int offset = 0;

  lock();
  MSH_PACK_ARRAY(buffer, offset, m_gains, complex<double>);
  MSH_PACK_ARRAY(buffer, offset, m_quality, double);
  memcpy((char*)buffer + offset, &m_done, sizeof(bool));
  offset += sizeof(bool);
  unlock();

  return offset;
}

unsigned int AntennaGains::unpack(void* buffer)
{
  unsigned int offset = 0;

  lock();
  MSH_UNPACK_ARRAY(buffer, offset, m_gains, complex<double>, 3);
  MSH_UNPACK_ARRAY(buffer, offset, m_quality, double, 3);
  memcpy(&m_done, (char*)buffer + offset, sizeof(bool));
  offset += sizeof(bool);
  unlock();

  return offset;
}

AntennaGains& AntennaGains::copy(AntennaGains& source)
{
  lock(); source.lock();
  m_gains.resize(source.m_gains.shape());
  m_gains = source.m_gains.copy();

  m_quality.resize(source.m_quality.shape());
  m_quality = source.m_quality.copy();

  m_done = source.m_done;
  source.unlock(); unlock();

  return *this;
}

