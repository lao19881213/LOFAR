//#  AVTTestTask.cc: Implementation of the Virtual Telescope test
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

#include "AVTTest.h"
#include "AVTTestTask.h"

#include <stdio.h>

const string sTaskName    = "AVTTest";
const string sSBFName     = "AVTTestSBF1";
const string sSBFAPCName  = "AVTTestSBFAPC";
const string sSBFAPCScope = "AVTTestSBFAPCScope";
const string sVTName      = "AVTTestVT1";
const string sVTAPCName   = "AVTTestVTAPC";
const string sVTAPCScope  = "AVTTestVTAPCScope";
const string sBSName      = "BeamServer";

const TPropertySet primaryPropertySetSBF =
{
};
 
const TPropertySet primaryPropertySetVT = 
{
};

AVTTestTask::AVTTestTask(AVTTestTask& tester) :
  GCFTask((State)&AVTTestTask::initial, sTaskName),
  m_beamformer(sSBFName,primaryPropertySetSBF,sSBFAPCName,sSBFAPCScope,sBSName),
  m_virtualTelescope(sVTName,primaryPropertySetVT,sVTAPCName,sVTAPCScope,m_beamformer),
  m_tester(tester)
{
}

AVTTestTask::~AVTTestTask()
{
}

GCFEvent::TResult AVTTestTask::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT_SIG:
      m_beamformer.getPort().init(m_beamformer, "AVTTestClient", GCFPortInterface::SAP, LOGICALDEVICE_PROTOCOL);
      m_virtualTelescope.getPort().init(m_virtualTelescope, "AVTTestClient", GCFPortInterface::SAP, LOGICALDEVICE_PROTOCOL);
      TRAN(AVTTestTask::test1);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case: set antenna orhtogonalization weights
 */
GCFEvent::TResult AVTTestTask::test1(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
      // ignore during first increments
      m_tester._avttest(true);
      TRAN(AVTTestTask::test2);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case: initialize EPA waveform generator
 */
GCFEvent::TResult AVTTestTask::test2(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
      m_tester._avttest(true);
      TRAN(AVTTestTask::test3);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case: create beamformer
 */
GCFEvent::TResult AVTTestTask::test3(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
      m_tester._avttest(true);
      TRAN(AVTTestTask::test4);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case: select beamlets
 */
GCFEvent::TResult AVTTestTask::test4(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
      m_tester._avttest(true);
      TRAN(AVTTestTask::test5);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case: select subbands
 */
GCFEvent::TResult AVTTestTask::test5(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
      m_tester._avttest(true);
      TRAN(AVTTestTask::test6);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case: set beam direction
 */
GCFEvent::TResult AVTTestTask::test6(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
      m_tester._avttest(true);
      TRAN(AVTTestTask::test7);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case: monitor statistics
 */
GCFEvent::TResult AVTTestTask::test7(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
      m_tester._avttest(true);
      TRAN(AVTTestTask::test8);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case: cleanup
 */
GCFEvent::TResult AVTTestTask::test8(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
      m_tester._avttest(true);
      TRAN(AVTTestTask::test9);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case: tbd
 */
GCFEvent::TResult AVTTestTask::test9(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
      m_tester._avttest(true);
      TRAN(AVTTestTask::finished);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}


/* 
 * End of all tests
 */
GCFEvent::TResult AVTTestTask::finished(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
      GCFTask::stop();
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

