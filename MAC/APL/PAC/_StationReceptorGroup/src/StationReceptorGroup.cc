//#  StationReceptorGroup.cc: Implementation of the StationReceptorGroup task
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
#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <boost/shared_ptr.hpp>
#include <Common/lofar_sstream.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVBool.h>
#include <GCF/GCF_PVDynArr.h>
#include <APLCommon/APLUtilities.h>
#include <StationReceptorGroup/StationReceptorGroup.h>
#include <GCF/ParameterSet.h>
#include "CAL_Protocol.ph"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;

namespace LOFAR
{
  using namespace ACC;
  using namespace APLCommon;
  
namespace ASR
{
INIT_TRACER_CONTEXT(StationReceptorGroup,LOFARLOGGER_PACKAGE);

const char TYPE_LCU_PIC_RCU[] = "TLcuPicRCU";
const char PROPNAME_FUNCTIONALITY[] = "functionality";
const char SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN[] = "PIC_Rack%d_SubRack%d_Board%d_AP%d_RCU%d";
const char PARAM_N_RACKS[]                     = "mac.apl.ara.N_RACKS";
const char PARAM_N_SUBRACKS_PER_RACK[]         = "mac.apl.ara.N_SUBRACKS_PER_RACK";
const char PARAM_N_BOARDS_PER_SUBRACK[]        = "mac.apl.ara.N_BOARDS_PER_SUBRACK";
const char PARAM_N_APS_PER_BOARD[]             = "mac.apl.ara.N_APS_PER_BOARD";
const char PARAM_N_RCUS_PER_AP[]               = "mac.apl.ara.N_RCUS_PER_AP";

// Logical Device version
const string StationReceptorGroup::SRG_VERSION = string("1.0");
string StationReceptorGroup::m_CALserverName("CALServer");

StationReceptorGroup::StationReceptorGroup(const string& taskName, 
                                     const string& parameterFile, 
                                     GCFTask* pStartDaemon) :
  LogicalDevice(taskName,parameterFile,pStartDaemon,SRG_VERSION),
  m_CALclient(*this, m_CALserverName, GCFPortInterface::SAP, CAL_PROTOCOL),
  m_rcuMap(),
  m_rcuFunctionalityMap(),
  m_n_racks(1),
  m_n_subracks_per_rack(1),
  m_n_boards_per_subrack(1),
  m_n_aps_per_board(4),
  m_n_rcus_per_ap(2),
  m_n_rcus(8),
  m_rcusPropertiesAvailableTimer(0),
  m_connectTimer(0)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());

  registerProtocol(CAL_PROTOCOL, CAL_PROTOCOL_signalnames);

  GCF::ParameterSet::instance()->adoptFile(GCF::ParameterSet::instance()->getSearchPath() + string("RegisterAccess.conf"));

  m_n_racks               = GCF::ParameterSet::instance()->getInt(PARAM_N_RACKS);
  m_n_subracks_per_rack   = GCF::ParameterSet::instance()->getInt(PARAM_N_SUBRACKS_PER_RACK);
  m_n_boards_per_subrack  = GCF::ParameterSet::instance()->getInt(PARAM_N_BOARDS_PER_SUBRACK);
  m_n_aps_per_board       = GCF::ParameterSet::instance()->getInt(PARAM_N_APS_PER_BOARD);
  m_n_rcus_per_ap         = GCF::ParameterSet::instance()->getInt(PARAM_N_RCUS_PER_AP);
  m_n_rcus = m_n_racks*
             m_n_subracks_per_rack*
             m_n_boards_per_subrack*
             m_n_aps_per_board*
             m_n_rcus_per_ap;
}


StationReceptorGroup::~StationReceptorGroup()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

bool StationReceptorGroup::_isCALclientPort(GCFPortInterface& port) const
{
  return (&port == &m_CALclient); // comparing two pointers. yuck?
}

void StationReceptorGroup::getRCURelativeNumbers(int rcuNr,int& rackRelativeNr,int& subRackRelativeNr,int& boardRelativeNr,int& apRelativeNr,int& rcuRelativeNr)
{
  rackRelativeNr    = rcuNr / (m_n_rcus_per_ap*m_n_aps_per_board*m_n_boards_per_subrack*m_n_subracks_per_rack);
  subRackRelativeNr = ( rcuNr / (m_n_rcus_per_ap*m_n_aps_per_board*m_n_boards_per_subrack) ) % m_n_subracks_per_rack;
  boardRelativeNr   = ( rcuNr / (m_n_rcus_per_ap*m_n_aps_per_board) ) % m_n_boards_per_subrack;
  apRelativeNr      = ( rcuNr / (m_n_rcus_per_ap) ) % m_n_aps_per_board;
  rcuRelativeNr     = ( rcuNr % m_n_rcus_per_ap);
}

int StationReceptorGroup::getRCUHardwareNr(const string& property)
{
  int rcu=-1;
  // strip property and systemname, propertyset name remains
  int posBegin=property.find_first_of(":")+1;
  int posEnd=property.find_last_of(".");
  string propertySetName = property.substr(posBegin,posEnd-posBegin);
  
  bool rcuFound=false;
  TRCUMap::iterator it = m_rcuMap.begin();
  while(!rcuFound && it != m_rcuMap.end())
  {
    if(propertySetName == it->second->getScope())
    {
      rcuFound=true;
      rcu = it->first;
    }
    else
    {
      ++it;
    }
  }
  return rcu;
}

bool StationReceptorGroup::rcuPropsAvailable()
{
  return (m_rcuFunctionalityMap.size() == m_n_rcus);
   
}

bool StationReceptorGroup::checkQuality()
{
  uint16 maxRcusDefect = m_parameterSet.getUint16(string("maxRcusDefect"));
  uint16 rcusDefect(0);
  for(TRCUFunctionalityMap::iterator it=m_rcuFunctionalityMap.begin();it!=m_rcuFunctionalityMap.end();++it)
  {
    if(!it->second)
    {
      rcusDefect++;
      LOG_DEBUG(formatString("RCU %d non-functional",it->first));
    }
  }
  LOG_DEBUG(formatString("Total RCUs defect: %d",rcusDefect));
  return (rcusDefect <= maxRcusDefect);
}

uint8 StationReceptorGroup::getRcuControlValue(const string& bandselection)
{
  uint8 rcuControlValue(0xB9);
  if(bandselection == string("LB_10_90"))
  {
    rcuControlValue=0xB9;
  }
  else if(bandselection == string("HB_110_190"))
  {
    rcuControlValue=0xC6;
  }
  else if(bandselection == string("HB_170_230"))
  {
    rcuControlValue=0xCE;
  }
  else if(bandselection == string("HB_210_250"))
  {
    rcuControlValue=0xD6;
  }
  return rcuControlValue;
}

void StationReceptorGroup::concrete_handlePropertySetAnswer(GCFEvent& answer)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(answer)).c_str());
  switch(answer.signal)
  {
    case F_SUBSCRIBED:
    {
      break;
    }
    case F_UNSUBSCRIBED:
    {
      break;
    }
    case F_VGETRESP:
    {
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);
      
      // check if it is a functionality
      if(strstr(pPropAnswer->pPropName, PROPNAME_FUNCTIONALITY) != 0)
      {
        // add the functionality state to the internal cache
        int rcu = getRCUHardwareNr(pPropAnswer->pPropName);
        if(rcu >= 0)
        {
          bool functional = ((GCFPVBool*)pPropAnswer->pValue)->getValue();
          m_rcuFunctionalityMap[static_cast<uint16>(rcu)] = functional;
        }
      }
      break;
    }
    case F_VCHANGEMSG:
    {
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);
      
      // check if it is a functionality
      if(strstr(pPropAnswer->pPropName, PROPNAME_FUNCTIONALITY) != 0)
      {
        LOG_DEBUG("functionality property changed");
        // add the functionality to the internal cache
        int rcu = getRCUHardwareNr(pPropAnswer->pPropName);
        if(rcu >= 0)
        {
          bool functional = ((GCFPVBool*)pPropAnswer->pValue)->getValue();
          m_rcuFunctionalityMap[static_cast<uint16>(rcu)] = functional;
          LOG_DEBUG(formatString("RCU %d functionality: %s",rcu,(functional?"true":"false")));
        }
        if(getLogicalDeviceState() == LOGICALDEVICE_STATE_ACTIVE)
        {
          // check functionality state of RCU's
          if(!checkQuality())
          {
            _doStateTransition(LOGICALDEVICE_STATE_SUSPENDED,LD_RESULT_LOW_QUALITY);
          }
        }
      }
      break;
    }
    case F_EXTPS_LOADED:
    {
      break;
    }
    case F_EXTPS_UNLOADED:
    {
      break;
    }
    case F_PS_CONFIGURED:
    {
      break;
    }
    case F_MYPS_ENABLED:
    {
      break;
    }
    case F_MYPS_DISABLED:
    {
      break;
    }
    case F_SERVER_GONE:
    {
      break;
    }
    default:
      break;
  }
}

GCFEvent::TResult StationReceptorGroup::concrete_initial_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  newState=LOGICALDEVICE_STATE_NOSTATE;
  switch (event.signal)
  {
    case F_ENTRY:
    {
      break;
    }
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }  
  return status;
}

GCFEvent::TResult StationReceptorGroup::concrete_idle_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  newState=LOGICALDEVICE_STATE_NOSTATE;
  switch (event.signal)
  {
    case F_ENTRY:
    {
      break;
    }
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }  
  return status;
}

GCFEvent::TResult StationReceptorGroup::concrete_claiming_state(GCFEvent& event, GCFPortInterface& p, TLogicalDeviceState& /*newState*/, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      bool res=m_CALclient.open(); // need this otherwise GTM_Sockethandler is not called
      LOG_DEBUG(formatString("m_CALclient.open() returned %s",(res?"true":"false")));
      if(!res)
      {
        m_connectTimer = m_CALclient.setTimer((long)3);
      }  
      break;
    }
    
    case F_CONNECTED:
    {
      if(_isCALclientPort(p))
      {
        LOG_DEBUG(formatString("port '%s' connected", p.getName().c_str()));
        if (p.isConnected())
        {
          m_rcusPropertiesAvailableTimer = p.setTimer((long)1); // wait a second before checking if all status properties are available
        }
      }
      break;
    }
    
    case F_DISCONNECTED:
    {
      if(_isCALclientPort(p))
      {
        m_connectTimer = m_CALclient.setTimer((long)3); // try again in 3 seconds
        LOG_WARN(formatString("port '%s' disconnected, retry in 3 seconds...", p.getName().c_str()));
        m_CALclient.close();
      }
      break;
    }

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult StationReceptorGroup::concrete_claimed_state(GCFEvent& event, GCFPortInterface& p, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  newState=LOGICALDEVICE_STATE_NOSTATE;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      break;
    }
    
    case F_DISCONNECTED:
    {
      if(_isCALclientPort(p))
      {
        LOG_ERROR(formatString("port '%s' disconnected", p.getName().c_str()));
        newState=LOGICALDEVICE_STATE_CLAIMING;
      }
      break;
    }
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult StationReceptorGroup::concrete_preparing_state(GCFEvent& event, GCFPortInterface& p, TLogicalDeviceState& newState, TLDResult& errorCode)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case CAL_STARTACK:
    {
      CALStartackEvent ack(event);
      if (SUCCESS == ack.status)
      {
        newState=LOGICALDEVICE_STATE_SUSPENDED;
      }
      else
      {
        LOG_FATAL(formatString("Unable to start calibration for SRG %s: %d",ack.name.c_str(),ack.status));
        newState=LOGICALDEVICE_STATE_CLAIMING;
        errorCode = LD_RESULT_STARTCAL_ERROR;
      }
      break;
    }

    case F_DISCONNECTED:
    {
      if(_isCALclientPort(p))
      {
        LOG_ERROR(formatString("port '%s' disconnected", p.getName().c_str()));
        newState=LOGICALDEVICE_STATE_CLAIMING;
      }
      break;
    }
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult StationReceptorGroup::concrete_active_state(GCFEvent& event, GCFPortInterface& p, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_ENTRY:
    {
      break;
    }
          
    case F_DISCONNECTED:
    {
      if(_isCALclientPort(p))
      {
        LOG_ERROR(formatString("port '%s' disconnected", p.getName().c_str()));
        _doStateTransition(LOGICALDEVICE_STATE_SUSPENDED,LD_RESULT_LOW_QUALITY);
      }
      break;
    }
    
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }  
  return status;
}

GCFEvent::TResult StationReceptorGroup::concrete_releasing_state(GCFEvent& event, GCFPortInterface& /*p*/, TLogicalDeviceState& newState, TLDResult& /*errorCode*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case CAL_STOPACK:
    {
      CALStopackEvent ack(event);
      if (SUCCESS != ack.status)
      {
        LOG_FATAL(formatString("Unable to stop calibration for SRG %s: %d",ack.name.c_str(),ack.status));
      }
      newState=LOGICALDEVICE_STATE_GOINGDOWN;
      break;
    }

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

void StationReceptorGroup::concreteClaim(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  // load propertysets for the rcus
  try
  {
    vector<uint16> rcuVector = m_parameterSet.getUint16Vector(string("rcus"));
    for(vector<uint16>::iterator it=rcuVector.begin();it!=rcuVector.end();++it)
    {
      char scopeString[300];
      int rackRelativeNr,subRackRelativeNr,boardRelativeNr,apRelativeNr,rcuRelativeNr;
      getRCURelativeNumbers((*it),rackRelativeNr,subRackRelativeNr,boardRelativeNr,apRelativeNr,rcuRelativeNr);
      sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rackRelativeNr,subRackRelativeNr,boardRelativeNr,apRelativeNr,rcuRelativeNr);

      boost::shared_ptr<GCFExtPropertySet> propSetPtr(new GCFExtPropertySet(scopeString,TYPE_LCU_PIC_RCU,&m_propertySetAnswer));
      propSetPtr->load();
      propSetPtr->subscribeProp(string(PROPNAME_FUNCTIONALITY));
      propSetPtr->requestValue(string(PROPNAME_FUNCTIONALITY));
      m_rcuMap[(*it)]=propSetPtr;
    }
  }
  catch(Exception &e)
  {
    LOG_FATAL(formatString("Error claiming SRG: %s",e.message().c_str()));
  }
}

void StationReceptorGroup::concretePrepare(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());

  try
  {  
    // send CALStartEvent
    LOG_INFO(formatString("Sending CALSTART to CAL server"));
    CALStartEvent calStartEvent;
    calStartEvent.name = getName();
    calStartEvent.parent = m_parameterSet.getString(string("antennaArray"));
    calStartEvent.subset.reset(); // reset every bit
    for(TRCUMap::iterator it=m_rcuMap.begin();it!=m_rcuMap.end();++it)
    {
      calStartEvent.subset.set(it->first);
    }
    calStartEvent.sampling_frequency = m_parameterSet.getDouble(string("frequency"));
    calStartEvent.nyquist_zone = m_parameterSet.getInt16(string("nyquistZone"));
    calStartEvent.rcucontrol.value = getRcuControlValue(m_parameterSet.getString(string("bandSelection")));
    m_CALclient.send(calStartEvent);
  }
  catch(Exception &e)
  {
    LOG_FATAL(formatString("Error preparing SRG: %s",e.message().c_str()));
  }
}

void StationReceptorGroup::concreteResume(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void StationReceptorGroup::concreteSuspend(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void StationReceptorGroup::concreteRelease(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  // send CALStopEvent
  LOG_INFO(formatString("Sending CALSTOP to CAL server"));
  CALStopEvent calStopEvent;
  calStopEvent.name = getName();
  m_CALclient.send(calStopEvent);

  // and unsubscribe from RCU functionality props
  //  for(TRCUMap::iterator it=m_rcuMap.begin();it!=m_rcuMap.end();++it)
  //  {
  //    it->second->unsubscribeProp(PROPNAME_FUNCTIONALITY);
  //    it->second->unload();
  //  }
}

void StationReceptorGroup::concreteParentDisconnected(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void StationReceptorGroup::concreteChildDisconnected(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

void StationReceptorGroup::concreteHandleTimers(GCFTimerEvent& timerEvent, GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  if(timerEvent.id == m_rcusPropertiesAvailableTimer)
  {
    if(!rcuPropsAvailable())
    {
      m_rcusPropertiesAvailableTimer = port.setTimer((long)1);
    }
    else
    {
      // subscribe to RCU functionality states
      // check functionality
      if(checkQuality())
      {
        // enter claimed state
        _doStateTransition(LOGICALDEVICE_STATE_CLAIMED);
      }
      else
      {
        _doStateTransition(LOGICALDEVICE_STATE_IDLE,LD_RESULT_LOW_QUALITY);
      }
    }
  }
  else if(timerEvent.id == m_connectTimer)
  {
    m_CALclient.open();
  }
}

void StationReceptorGroup::concreteAddExtraKeys(ACC::APS::ParameterSet& psSubset)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
}

}; // namespace VIC
}; // namespace LOFAR

