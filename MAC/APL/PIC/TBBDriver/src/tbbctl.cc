//#
//#  tbbctl.cc: command line interface to the TBBDriver
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
//#


#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>

#include <MACIO/MACServiceInfo.h>

#include <APL/TBB_Protocol/TBB_Protocol.ph>

#include <Common/lofar_iostream.h>
#include <Common/lofar_sstream.h>
#include <getopt.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <Common/lofar_set.h>
#include <time.h>

//#include <APL/RTCCommon/gnuplot_i.h>

#include <netinet/in.h>    
#include <net/ethernet.h>  


#include "tbbctl.h"

using namespace std;
//using namespace blitz;
using namespace LOFAR;
using namespace TBB_Protocol;
using namespace TbbCtl;

static const long DELAY = 5; 

//---- ALLOC  ----------------------------------------------------------------
AllocCmd::AllocCmd(GCFPortInterface& port) : Command(port)
{
	cout << endl;
	cout << "== TBB ================================================== allocate memory ====" << endl;	
	cout << endl;
}

//-----------------------------------------------------------------------------
void AllocCmd::send()
{
	TBBAllocEvent event;
	
	if (isSelectionDone()) event.rcu_mask = getRcuMask(); // if select cmd is used
	
	itsPort.send(event);
	itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult AllocCmd::ack(GCFEvent& e)
{
	TBBAllocAckEvent ack(e);
	cout << "TBB  Info" << endl;
	cout << "---  -------------------------------------------------------" << endl;
	int32 bnr = 0;
	int32 oldbnr = -1;
	for (int cnr=0; cnr < getMaxSelections(); cnr++) {
		bnr = static_cast<int32>(cnr / 16);
		
		if (bnr != oldbnr) {
			if ((ack.status_mask[bnr] & TBB_SUCCESS) || (ack.status_mask[bnr] & TBB_RCU_COMM_ERROR)) {
				cout << formatString(" %2d memory allocated for selected rcu's", bnr) << endl;
			}	else {
				cout << formatString(" %2d %s", bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()) << endl;
			}
		}
		
		if (isSelected(cnr) && !(ack.status_mask[bnr] & TBB_NO_BOARD) ) {
			if (ack.rcu_mask.test(cnr)) {
				if (ack.status_mask[bnr] & TBB_SUCCESS) {
					cout << formatString("     ERROR, Rcu-%d NOT in correct state",cnr) << endl;
				} else {
					cout << formatString("     ERROR, Rcu-%d  %s",cnr,getDriverErrorStr(ack.status_mask[bnr] & 0xFFFF0000).c_str()) << endl;
				}
			}
		}
		oldbnr = bnr;
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- CHANNELINFO --------------------------------------------------------------
ChannelInfoCmd::ChannelInfoCmd(GCFPortInterface& port) : Command(port)
{
	cout << endl;
	cout << "== TBB ===================================================== rcu info =======" << endl;
	cout << endl;
}

//-----------------------------------------------------------------------------
void ChannelInfoCmd::send()
{
	TBBRcuInfoEvent event;
	itsPort.send(event);
	itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ChannelInfoCmd::ack(GCFEvent& e)
{
	TBBRcuInfoAckEvent ack(e);
	
	int32 bnr = 0;
	int32 oldbnr = -1;
	for (int rcu=0; rcu < getMaxSelections(); rcu++) {
	 		bnr = static_cast<int32>(rcu / 16);
			if (ack.rcu_state[rcu] != 'F') {
				if (isSelected(rcu) ) {
					if (bnr != oldbnr) {
						cout << "Rcu  Board  Input  State  Start-address  Size[pages]" << endl;
						cout << "---  -----  -----  -----  -------------  -----------" << endl;
					}
					cout << formatString(" %2d    %2d     %2d      %c     0x%08X  %11u %s",
							rcu, ack.rcu_on_board[rcu], ack.rcu_on_input[rcu],
							(char)ack.rcu_state[rcu],	ack.rcu_start_addr[rcu], ack.rcu_pages[rcu],
							getBoardErrorStr(ack.rcu_status[rcu]).c_str()) << endl;
				}
			}
			oldbnr = bnr;
	}
	cout << endl;
	cout << " *State:  F = Free, A = Allocated, R = Recording, S = Stopped, E = Error" << endl;
	cout << " *Only NOT Free rcu's are listed " << endl;
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- FREE ----------------------------------------------------------------
FreeCmd::FreeCmd(GCFPortInterface& port) : Command(port)
{
	cout << endl;
	cout << "== TBB =================== discard buffer allocation and disable rcu's ====" << endl;
	cout << endl;
}

//-----------------------------------------------------------------------------
void FreeCmd::send()
{
	TBBFreeEvent event;
	
	if (isSelectionDone()) event.rcu_mask = getRcuMask(); // if select cmd is used
	
	itsPort.send(event);
	itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult FreeCmd::ack(GCFEvent& e)
{
	TBBFreeAckEvent ack(e);
	cout << "TBB  Info" << endl;
	cout << "---  -------------------------------------------------------" << endl;
	int32 bnr = 0;
	int32 oldbnr = -1;
	for (int cnr=0; cnr < getMaxSelections(); cnr++) {
		bnr = static_cast<int32>(cnr / 16);
		
		if (bnr != oldbnr) {
			if (ack.status_mask[bnr] & TBB_SUCCESS) {
				cout << formatString(" %2d  buffer dischard and channel disabled for selected rcu's", bnr) << endl;
			} else {
				cout << formatString(" %2d  %s", bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()) << endl;
			}
		}
		if (isSelected(cnr) && !(ack.status_mask[bnr] & TBB_NO_BOARD) ) {
			if (ack.rcu_mask.test(cnr)) {
				if (ack.status_mask[bnr] & TBB_SUCCESS) {
					cout << formatString("     ERROR, Rcu-%d NOT in correct state",cnr) << endl;
				} else {
					cout << formatString("     ERROR, Rcu-%d  %s",cnr,getDriverErrorStr(ack.status_mask[bnr]).c_str()) << endl;
				}
			}
		}
		oldbnr = bnr;
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- RECORD ----------------------------------------------------------------
RecordCmd::RecordCmd(GCFPortInterface& port) : Command(port)
{
	cout << endl;
	cout << "== TBB ============================= start recording on selected rcu's ====" << endl;
	cout << endl;
}

//-----------------------------------------------------------------------------
void RecordCmd::send()
{
	TBBRecordEvent event;
	
	if (isSelectionDone()) event.rcu_mask = getRcuMask(); // if select cmd is used
	
	itsPort.send(event);
	itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult RecordCmd::ack(GCFEvent& e)
{
	TBBRecordAckEvent ack(e);
	cout << "TBB  Info" << endl;
	cout << "---  -------------------------------------------------------" << endl;
	int32 bnr = 0;
	int32 oldbnr = -1;
	for (int cnr=0; cnr < getMaxSelections(); cnr++) {
		bnr = static_cast<int32>(cnr / 16);
		
		if (bnr != oldbnr) {
			if (ack.status_mask[bnr] & TBB_SUCCESS) {
				cout << formatString(" %2d  recording started for selected rcu's", bnr) << endl;
			} else {
				cout << formatString(" %2d  %s", bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()) << endl;
			}
		}
		
		if (isSelected(cnr) && !(ack.status_mask[bnr] & TBB_NO_BOARD) ) {
			if (ack.rcu_mask.test(cnr)) {
				if (ack.status_mask[bnr] & TBB_SUCCESS) {
					//cout << formatString("      ERROR, Rcu-%d NOT in correct state",cnr) << endl;
					//cout  << endl;
				} else {
					cout << formatString("      ERROR, Rcu-%d  %s",cnr,getDriverErrorStr(ack.status_mask[bnr]).c_str()) << endl;
				}
			}
		}
		oldbnr = bnr;
	}
	
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- STOP -------------------------------------------------------------------
StopCmd::StopCmd(GCFPortInterface& port) : Command(port)
{
	cout << endl;
	cout << "== TBB ============================== stop recording on selected rcu's ====" << endl;	
	cout << endl;
}

//-----------------------------------------------------------------------------
void StopCmd::send()
{
	TBBStopEvent event;
	if (isSelectionDone()) event.rcu_mask = getRcuMask(); // if select cmd is used
	
	itsPort.send(event);
	itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult StopCmd::ack(GCFEvent& e)
{
	TBBStopAckEvent ack(e);
	cout << "TBB  Info" << endl;
	cout << "---  -------------------------------------------------------" << endl;
	int32 bnr = 0;
	int32 oldbnr = -1;
	for (int cnr=0; cnr < getMaxSelections(); cnr++) {
		bnr = static_cast<int32>(cnr / 16);
		
		if (bnr != oldbnr) {
			if (ack.status_mask[bnr] & TBB_SUCCESS) {
				cout << formatString(" %2d  recording stopped for selected rcu's", bnr) << endl;
			} else {
				cout << formatString(" %2d  %s", bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()) << endl;
			}
		}
		
		if (isSelected(cnr) && !(ack.status_mask[bnr] & TBB_NO_BOARD) ) {
			if (ack.rcu_mask.test(cnr)) {
				if (!(ack.status_mask[bnr] & TBB_SUCCESS)) {
					cout << formatString("      ERROR, Rcu-%d  %s",cnr,getDriverErrorStr(ack.status_mask[bnr]).c_str()) << endl;
				}
			}
		}
		oldbnr = bnr;
	}
	
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- TRIGGERSETTINGS --------------------------------------------------------
TriggerSettingsCmd::TriggerSettingsCmd(GCFPortInterface& port) : Command(port)
{
	cout << endl;
	cout << "== TBB ===================================================== rcu info =======" << endl;
	cout << endl;
}

//-----------------------------------------------------------------------------
void TriggerSettingsCmd::send()
{
	TBBTrigSettingsEvent event;
	itsPort.send(event);
	itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult TriggerSettingsCmd::ack(GCFEvent& e)
{
	TBBTrigSettingsAckEvent ack(e);
	
	int32 bnr = 0;
	int32 oldbnr = -1;
	for (int rcu=0; rcu < getMaxSelections(); rcu++) {
 		bnr = static_cast<int32>(rcu / 16);
		if (isSelected(rcu) ) {
			if (bnr != oldbnr) {
				cout << "Rcu  Board  level   start   stop    filter  window  c0      c1      c2      c3" << endl;
				cout << "---  -----  ------  ------  ------  ------  ------  ------  ------  ------  ------" << endl;
			}
			cout << formatString("%3d  %5d  %6d  %6d  %6d  %6d  %6d  %6d  %6d  %6d  %6d",
					rcu, bnr, ack.setup[rcu].level, ack.setup[rcu].start_mode, ack.setup[rcu].stop_mode,
					ack.setup[rcu].filter_select, ack.setup[rcu].window, ack.coefficients[rcu].c0, 
					ack.coefficients[rcu].c1, ack.coefficients[rcu].c2, ack.coefficients[rcu].c3) << endl;
		}
		oldbnr = bnr;
	}
	cout << endl;
	
	setCmdDone(true);
	return(GCFEvent::HANDLED);
}

//---- TRIGRELEASE ------------------------------------------------------------
TrigReleaseCmd::TrigReleaseCmd(GCFPortInterface& port) : Command(port)
{
	cout << endl;
	cout << "== TBB ============================ release trigger for selected rcu's ====" << endl;
	cout << endl;
}

//-----------------------------------------------------------------------------
void TrigReleaseCmd::send()
{
	TBBTrigReleaseEvent event;
	if (isSelectionDone()) {
		event.rcu_stop_mask = getRcuMask();
		event.rcu_start_mask = getRcuMask();
	}
	itsPort.send(event);
	itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult TrigReleaseCmd::ack(GCFEvent& e)
{
	TBBTrigReleaseAckEvent ack(e);
	cout << "TBB  Info" << endl;
	cout << "---  -------------------------------------------------------" << endl;  
	int32 bnr = 0;
	int32 oldbnr = -1;  
	for (int cnr=0; cnr < getMaxSelections(); cnr++) {
		bnr = static_cast<int32>(cnr / 16);
		
		if (bnr != oldbnr) {
			//cout << formatString("status[%d]%08x",bnr, ack.status_mask[bnr]);
			if (ack.status_mask[bnr] & TBB_SUCCESS) {
				cout << formatString(" %2d  trigger detection released for selected rcu's",bnr ) << endl;
			}	else {	
				cout << formatString(" %2d  %s",bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()) << endl;
			}
		}
		oldbnr = bnr;
	}
	
	setCmdDone(true);
	return(GCFEvent::HANDLED);
}

//---- TRIGGENERATE -----------------------------------------------------------
TrigGenerateCmd::TrigGenerateCmd(GCFPortInterface& port) : Command(port)
{
	cout << endl;
	cout << "== TBB ============================ generate trigger on selected rcu's ====" << endl;
	cout << endl;
	cout << "RCU   seq_nr      time        sample       sum          samples     peak        pwr_before  pwr_after" << endl;
	cout << "---   ---------   ---------   ----------   ----------   ---------   ---------   ---------   ---------" << endl;  
}

//-----------------------------------------------------------------------------
void TrigGenerateCmd::send()
{
	setCmdSendNext(false);
	TBBSubscribeEvent subscribe;
	subscribe.triggers = true;
	subscribe.hardware = true;
  itsPort.send(subscribe);
	
	TBBTrigGenerateEvent event;
	if (isSelectionDone()) {
		event.rcu_mask = getRcuMask();
	}	
	itsPort.send(event);
	itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult TrigGenerateCmd::ack(GCFEvent& e)
{
	if (e.signal == TBB_TRIG_GENERATE_ACK) {
		TBBTrigGenerateAckEvent ack(e);
	}
	if (e.signal == TBB_SUBSCRIBE_ACK) {
	}
	if (e.signal == TBB_TRIGGER) {
		TBBTriggerEvent trig(e);
		cout << formatString(" %2d   %9u   %9u   %10u   %10u   %9u   %9u   %9u   %9u",
												 trig.rcu,
												 trig.sequence_nr,
												 trig.time,
												 trig.sample_nr,
												 trig.trigger_sum,
												 trig.trigger_samples,
												 trig.peak_value,
												 trig.power_before,
												 trig.power_after
												) << endl;
	}
	//setCmdDone(true);
	return(GCFEvent::HANDLED);
}

//---- TRIGSETUP --------------------------------------------------------------
TrigSetupCmd::TrigSetupCmd(GCFPortInterface& port) : Command(port),
	itsLevel(0), itsStartMode(0), itsStopMode(0),itsFilter(0), itsWindow(0), itsOperatingMode(0), itsTriggerMode(0)
{
	cout << endl;
	cout << "== TBB ============================ trigger system setup for selected rcu's ====" << endl;
	cout << endl;
}

//-----------------------------------------------------------------------------
void TrigSetupCmd::send()
{
	TBBTrigSetupEvent event;
	if (isSelectionDone()) {
		for (int cnr=0; cnr < getMaxSelections(); cnr++) {	
			event.setup[cnr].level = itsLevel;
			event.setup[cnr].start_mode = itsStartMode;
			event.setup[cnr].stop_mode = itsStopMode;
			event.setup[cnr].filter_select = itsFilter;
			event.setup[cnr].window = itsWindow;
			event.setup[cnr].operating_mode = itsOperatingMode;
			event.trigger_mode = itsTriggerMode;
		}
	}
	itsPort.send(event);
	itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult TrigSetupCmd::ack(GCFEvent& e)
{
	if (e.signal == TBB_TRIG_SETUP_ACK) {
		TBBTrigSetupAckEvent ack(e);
		cout << "TBB  Info" << endl;
		cout << "---  -------------------------------------------------------" << endl;
		int32 bnr = 0;
		int32 oldbnr = -1;  
		for (int cnr=0; cnr < getMaxSelections(); cnr++) {
			bnr = static_cast<int32>(cnr / 16);
			
			if (bnr != oldbnr) {
				if (ack.status_mask[bnr] & TBB_SUCCESS) {
					cout << formatString(" %2d  setup trigger system for all rcu's",bnr ) << endl;
				}	else {	
					cout << formatString(" %2d  %s",bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()) << endl;
				}
			}
			oldbnr = bnr;
		}
		setCmdDone(true);
	}
	return(GCFEvent::HANDLED);
}

//---- TRIGCOEFFICIENTS -------------------------------------------------------
TrigCoefficientCmd::TrigCoefficientCmd(GCFPortInterface& port) : Command(port),
	itsC0(0), itsC1(0), itsC2(0), itsC3(0)
{
	cout << endl;
	cout << "== TBB ============================ setup trigger coeffients for selected rcu's ====" << endl;
	cout << endl;
}

//-----------------------------------------------------------------------------
void TrigCoefficientCmd::send()
{
	TBBTrigCoefEvent event;
	if (isSelectionDone()) {
		for (int cnr=0; cnr < getMaxSelections(); cnr++) {
			event.coefficients[cnr].c0 = itsC0;
			event.coefficients[cnr].c1 = itsC1;
			event.coefficients[cnr].c2 = itsC2;
			event.coefficients[cnr].c3 = itsC3;
		}
	}
	itsPort.send(event);
	itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult TrigCoefficientCmd::ack(GCFEvent& e)
{
	TBBTrigCoefAckEvent ack(e);
	cout << "TBB  Info" << endl;
	cout << "---  -------------------------------------------------------" << endl;  
	int32 bnr = 0;
	int32 oldbnr = -1;  
	for (int cnr=0; cnr < getMaxSelections(); cnr++) {
		bnr = static_cast<int32>(cnr / 16);
		
		if (bnr != oldbnr) {
			if (ack.status_mask[bnr] & TBB_SUCCESS) {
				cout << formatString(" %2d  setup trigger system for all rcu's",bnr ) << endl;
			}	else {	
				cout << formatString(" %2d  %s",bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()) << endl;
			}
		}
		oldbnr = bnr;
	}
	setCmdDone(true);
	return(GCFEvent::HANDLED);
}

//---- TRIGINFO ---------------------------------------------------------------
TrigInfoCmd::TrigInfoCmd(GCFPortInterface& port) : Command(port)
{
	cout << endl;
	cout << "== TBB ============================ trigger info for selected rcu's ====" << endl;
	cout << endl;
}

//-----------------------------------------------------------------------------
void TrigInfoCmd::send()
{
	TBBTrigInfoEvent event;
	if (isSelectionDone()) {
		event.rcu = getRcu();
	}
	itsPort.send(event);
	itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult TrigInfoCmd::ack(GCFEvent& e)
{
	TBBTrigInfoAckEvent ack(e);
	cout << "RCU  Info" << endl;
	cout << "---  -------------------------------------------------------" << endl;  
	if (ack.status_mask & TBB_SUCCESS) {
		cout << formatString(" %2d  sequence   : %lu",getRcu(),ack.sequence_nr) << endl;
		cout << formatString("      time       : %lu",ack.time) << endl;
		cout << formatString("      sample     : %lu",ack.sample_nr) << endl;
		cout << formatString("      sum        : %lu",ack.trigger_sum) << endl;
		cout << formatString("      samples    : %lu",ack.trigger_samples) << endl;
		cout << formatString("      peak value : %lu",ack.peak_value) << endl;
		cout << formatString("      pwr before : %u",ack.power_before) << endl;
		cout << formatString("      pwr after  : %u",ack.power_after) << endl;
	}	else {	
		cout << formatString(" %2d  %s",getRcu(), getDriverErrorStr(ack.status_mask).c_str()) << endl;
	}
	setCmdDone(true);
	return(GCFEvent::HANDLED);
}

//---- LISTEN ---------------------------------------------------------------
ListenCmd::ListenCmd(GCFPortInterface& port) : Command(port),
	itsCmdStage(0)
{
	cout << endl;
	cout << "== TBB ============================ listen for triggers on selected rcu's ====" << endl;
	cout << endl;
	 
}

//-----------------------------------------------------------------------------
void ListenCmd::send()
{
	switch (itsCmdStage) {
		case 0: {
			cout << "-stop all trigger systems" << endl;
			TBBTrigReleaseEvent release;
			release.rcu_stop_mask.set();
			release.rcu_start_mask.reset();
			itsPort.send(release);
		} break;
		
		case 1: {
			cout << "-stopped recording on all allocated rcu's" << endl;
			TBBStopEvent stop;
			stop.rcu_mask = getRcuMask();
			itsPort.send(stop);	
		} break;

		case 2: {
			cout << "-restarted recording on all allocated rcu's" << endl;
			TBBRecordEvent record;
			record.rcu_mask = getRcuMask(); // if select cmd is used
			itsPort.send(record);	
		} break;

		case 3: {
			TBBSubscribeEvent subscribe;
			subscribe.triggers = true;
			subscribe.hardware = true;
      itsPort.send(subscribe);
		} break;
		
		case 4: {
			cout << "-released all trigger systems, and waiting  (STOP WAITING WITH CTRL-C)" << endl;
			setCmdSendNext(false);
			TBBTrigReleaseEvent release;
			release.rcu_stop_mask.reset();
			release.rcu_start_mask = getRcuMask();
			itsPort.send(release);
			cout << endl;
	cout << "RCU   seq_nr      time         sample       sum          samples     peak        pwr_before  pwr_after" << endl;
	cout << "---   ---------   ----------   ----------   ----------   ---------   ---------   ---------   ---------" << endl; 
		} break;
		
		


		default: {
		} break;
	}
	itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ListenCmd::ack(GCFEvent& e)
{
	static int nummer;
	if (e.signal == TBB_STOP_ACK) {
		TBBTrigReleaseAckEvent ack(e);
		if (itsCmdStage == 1) { itsCmdStage = 2; }
	}
	if (e.signal == TBB_RECORD_ACK) {
		TBBTrigReleaseAckEvent ack(e);
		itsCmdStage = 3;
	}
	if (e.signal == TBB_SUBSCRIBE_ACK) {
		itsCmdStage = 4;
		nummer = 0;
	}
	if (e.signal == TBB_TRIG_RELEASE_ACK) {
		TBBTrigReleaseAckEvent ack(e);
		if (itsCmdStage == 0) itsCmdStage = 1;
		if (itsCmdStage == 4) itsCmdStage = 5;
	}
		
	if (e.signal == TBB_TRIGGER) {
		
			TBBTriggerEvent trig(e);
			nummer++;	
			cout << formatString(" %2d   %9u   %9u   %10u   %10u   %9u   %9u   %9u   %9u   (stopped) %6d",
												 trig.rcu,
												 trig.sequence_nr,
												 trig.time,
												 trig.sample_nr,
												 trig.trigger_sum,
												 trig.trigger_samples,
												 trig.peak_value,
												 trig.power_before,
												 trig.power_after,
												 nummer
												) << endl;
												
			if (itsListenMode == TBB_LISTEN_ONE_SHOT) {
				TBBStopEvent event;
				event.rcu_mask.set(trig.rcu);
				itsPort.send(event);
			} else {
				TBBTrigReleaseEvent release;
				release.rcu_stop_mask.set(trig.rcu);
				release.rcu_start_mask.set(trig.rcu);
				itsPort.send(release);
			}									
	}	
	//setCmdDone(true);
	return(GCFEvent::HANDLED);
}

//---- READ -------------------------------------------------------------------
ReadCmd::ReadCmd(GCFPortInterface& port) : Command(port),
	itsSecondsTime(0),itsSampleTime(0),itsPrePages(0),itsPostPages(0)
{
	cout << endl;
	cout << "== TBB ==============  transfer data to CEP for all selected rcu ====" << endl;	
	cout << endl;
}

//-----------------------------------------------------------------------------
void ReadCmd::send()
{
	TBBReadEvent event;
	event.rcu = (uint32)getRcu();
	event.secondstime = itsSecondsTime;
	event.sampletime = itsSampleTime;
	event.prepages = itsPrePages;
	event.postpages = itsPostPages;
	itsPort.send(event);
	itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ReadCmd::ack(GCFEvent& e)
{
	TBBReadAckEvent ack(e);
	cout << "RCU  Info" << endl;
	cout << "---  -------------------------------------------------------" << endl;  
	for (int cnr=0; cnr < getMaxSelections(); cnr++) {
		if (isSelected(cnr) ) {
			if (ack.status_mask & TBB_SUCCESS) {
				cout << formatString(" %2d  tranfering data to CEP",cnr ) << endl;
			}	else {			
				cout << formatString(" %2d  %s",cnr, getDriverErrorStr(ack.status_mask).c_str()) << endl;
			}
		}
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- MODE --------------------------------------------------------------------
ModeCmd::ModeCmd(GCFPortInterface& port) : Command(port)
	,itsRecMode(0)
{
	cout << endl;
	cout << "== TBB ===================================== set mode command ===============" << endl;
	cout << endl;
}

//-----------------------------------------------------------------------------
void ModeCmd::send()
{
	TBBModeEvent event;
	event.rec_mode = itsRecMode;
	itsPort.send(event);
	itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ModeCmd::ack(GCFEvent& e)
{
	TBBModeAckEvent ack(e);
	cout << "TBB  Info" << endl;
	cout << "---  -------------------------------------------------------" << endl;  
	for (int i = 0; i < MAX_N_TBBBOARDS; i++) {
		if (ack.status_mask[i] & TBB_SUCCESS) {
			cout << formatString(" %2d  mode set and UDP/IP configured", i) << endl;
		}	else {			
			cout << formatString(" %2d  %s",i, getDriverErrorStr(ack.status_mask[i]).c_str()) << endl;
		}
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- VERSION ----------------------------------------------------------------
VersionCmd::VersionCmd(GCFPortInterface& port) : Command(port)
{
	cout << endl;
	cout << "== TBB ====================================== ID and version information ====" << endl;
	cout << endl;
}

//-----------------------------------------------------------------------------
void VersionCmd::send()
{
  TBBVersionEvent event;
  event.boardmask = getBoardMask();
  itsPort.send(event);
  itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult VersionCmd::ack(GCFEvent& e)
{
  TBBVersionAckEvent ack(e);
	
	cout << formatString("TBBDriver software version %3.2f",(ack.driverversion / 100.)) << endl;
	cout << endl;
	cout << "TBB  ID  Software   Board    TP0      MP0      MP1      MP2      MP3" << endl;
	cout << "---  --  --------  -------  -------  -------  -------  -------  -------" << endl;
	for (int bnr=0; bnr < getMaxSelections(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status_mask[bnr] & TBB_SUCCESS) {
				cout << formatString(" %2u  %2u   V%5.1f    V%4.1f    V%4.1f    V%4.1f    V%4.1f    V%4.1f    V%4.1f",
	          bnr,
	          ack.boardid[bnr],
						(ack.swversion[bnr] / 10.),
						(ack.boardversion[bnr] / 10.),
						(ack.tpversion[bnr] / 10.),
						(ack.mp0version[bnr] / 10.),
						(ack.mp1version[bnr] / 10.),
						(ack.mp2version[bnr] / 10.),
						(ack.mp3version[bnr] / 10.)) << endl;
			}	else {
					cout << formatString(" %2u  %s",bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()) << endl;	
      }
  	}
	}
	
  setCmdDone(true);

  return(GCFEvent::HANDLED);
}

//---- SIZE -------------------------------------------------------------------
SizeCmd::SizeCmd(GCFPortInterface& port) : Command(port)
{
  cout << endl;
  cout << "== TBB ==================================== installed memory information ====" << endl;	
  cout << endl;
}

//-----------------------------------------------------------------------------
void SizeCmd::send()
{
  TBBSizeEvent event;
  event.boardmask = getBoardMask();
  itsPort.send(event);
  itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult SizeCmd::ack(GCFEvent& e)
{
  TBBSizeAckEvent ack(e);
	cout << "TBB  pages     Total memory " << endl;
	cout << "---  --------  ------------" << endl; 
	for (int bnr=0; bnr < getMaxSelections(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status_mask[bnr] & TBB_SUCCESS) {
				double gbyte = ((double)ack.npages[bnr] * 2048.) / (1024. * 1024. * 1024.); 
				cout << formatString(" %2d  %8d  %6.3f GByte",bnr,ack.npages[bnr],gbyte) << endl;
			}	else {	
				cout << formatString(" %2d  %s",bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()) << endl;
      }
  	}
	}
	cout << endl;
	cout << "1 page = 2048 Bytes" << endl;
	
  setCmdDone(true);

  return(GCFEvent::HANDLED);
}

//---- STATUS -----------------------------------------------------------------
StatusCmd::StatusCmd(GCFPortInterface& port) : Command(port)
{
  cout << endl;
  cout << "== TBB ============================= voltage and temperature information ====" << endl;	
	cout << endl;
}

//-----------------------------------------------------------------------------
void StatusCmd::send()
{
  TBBStatusEvent event;
  event.boardmask = getBoardMask();
  itsPort.send(event);
  itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult StatusCmd::ack(GCFEvent& e)
{
  TBBStatusAckEvent ack(e);
    
	cout << "TBB  Voltage 1.2  Voltage 2.5  Voltage 3.3  Temp PCB  Temp TP   Temp MP0  Temp MP1  Temp MP2  Temp MP3" << endl;
	cout << "---  -----------  -----------  -----------  --------  --------  --------  --------  --------  --------" << endl;
	for (int bnr=0; bnr < getMaxSelections(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status_mask[bnr] & TBB_SUCCESS) {
				
				cout << formatString(" %2d     %4.2f V       %4.2f V       %4.2f V    %3u 'C    %3u 'C    %3u 'C    %3u 'C    %3u 'C    %3u 'C",
						bnr,
						((double)ack.V12[bnr] * (2.5 / 192.)),	// MAX6652 pin-2:  2.5 / 192	= 0.0130 / count
						((double)ack.V25[bnr] * (3.3 / 192.)),	// MAX6652 pin-3:  3.3 / 192	= 0.0172 / count
						((double)ack.V33[bnr] * (5.0 / 192.)),	// MAX6652 pin-1:  5.0 / 192	= 0.0625 / count
						ack.Tpcb[bnr],
						ack.Ttp[bnr],
						ack.Tmp0[bnr],
						ack.Tmp1[bnr],
						ack.Tmp2[bnr],
						ack.Tmp3[bnr] ) << endl;
			}	else {	
				cout << formatString(" %2d  %s",bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()) << endl;
      }
  	}
	}
	
  setCmdDone(true);

  return(GCFEvent::HANDLED);
}

//---- CLEAR -------------------------------------------------------------------
ClearCmd::ClearCmd(GCFPortInterface& port) : Command(port)
{
	cout << endl;
	cout << "== TBB =============================================== clear in progress ====" << endl;
	cout << " wait 5 seconds" << endl;
	cout << endl;
}

//-----------------------------------------------------------------------------
void ClearCmd::send()
{
	TBBClearEvent event;
	event.boardmask = getBoardMask();
	itsPort.send(event);
	itsPort.setTimer((long)5);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ClearCmd::ack(GCFEvent& e)
{
	TBBClearAckEvent ack(e);
	cout << "TBB  Info" << endl;
	cout << "---  -------------------------------------------------------" << endl;    
	for (int bnr=0; bnr < getMaxSelections(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status_mask[bnr] & TBB_SUCCESS) {
				cout << formatString(" %2d  board is cleared",bnr ) << endl;
			}	else {	
				cout << formatString(" %2d  %s",bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()) << endl;
			}
		}
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- RESET -------------------------------------------------------------------
ResetCmd::ResetCmd(GCFPortInterface& port) : Command(port)
{
	cout << endl;
	cout << "== TBB =============================================== reset in progress ====" << endl;
	cout << " reset can take 20 seconds" << endl;
	cout << endl;
}

//-----------------------------------------------------------------------------
void ResetCmd::send()
{
	TBBResetEvent event;
	event.boardmask = getBoardMask();
	itsPort.send(event);
	itsPort.setTimer(20.0);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ResetCmd::ack(GCFEvent& e)
{
	TBBResetAckEvent ack(e);
	cout << "TBB  Info" << endl;
	cout << "---  -------------------------------------------------------" << endl;    
	for (int bnr=0; bnr < getMaxSelections(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status_mask[bnr] & TBB_SUCCESS) {
				cout << formatString(" %2d  board is reset",bnr ) << endl;
			}	else {	
				cout << formatString(" %2d  %s",bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()) << endl;
			}
		}
	}
	cout << " setting up the boards can take 10 seconds" << endl;
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- CONFIG -------------------------------------------------------------------
ConfigCmd::ConfigCmd(GCFPortInterface& port) : Command(port)
{
	cout << endl;
	cout << "== TBB ========================================= reconfigure TP and MP's ====" << endl;
	cout << " wait 5 seconds" << endl;
	cout << endl;
}

//-----------------------------------------------------------------------------
void ConfigCmd::send()
{
	TBBConfigEvent event;
	event.boardmask = getBoardMask();
	event.imagenr = itsImage;
	itsPort.send(event);
	itsPort.setTimer((long)5);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ConfigCmd::ack(GCFEvent& e)
{
	TBBConfigAckEvent ack(e);
	cout << "TBB  Info" << endl;
	cout << "---  -------------------------------------------------------" << endl;    
	for (int bnr=0; bnr < getMaxSelections(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status_mask[bnr] & TBB_SUCCESS) {
				cout << formatString(" %2d  reconfigured TP and MP's",bnr ) << endl;
			}	else {	
				cout << formatString(" %2d  %s",bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()) << endl;
			}
		}
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- ARP -------------------------------------------------------------------
ArpCmd::ArpCmd(GCFPortInterface& port) : Command(port)
{
	cout << endl;
	cout << "== TBB ========================================= sending ARP message to CEP ====" << endl;
	cout << endl;
}

//-----------------------------------------------------------------------------
void ArpCmd::send()
{
	TBBArpEvent event;
	event.boardmask = getBoardMask();
	itsPort.send(event);
	itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ArpCmd::ack(GCFEvent& e)
{
	TBBArpAckEvent ack(e);
	cout << "TBB  Info" << endl;
	cout << "---  -------------------------------------------------------" << endl;    
	for (int bnr=0; bnr < getMaxSelections(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status_mask[bnr] & TBB_SUCCESS) {
				cout << formatString(" %2d  send 1 arp message",bnr ) << endl;
			}	else {	
				cout << formatString(" %2d  %s",bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()) << endl;
			}
		}
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- ARPMODE -------------------------------------------------------------------
ArpModeCmd::ArpModeCmd(GCFPortInterface& port) : Command(port)
  ,itsMode(0)
{
	cout << endl;
	cout << "== TBB ========================================= set CEP ARP mode ====" << endl;
	cout << endl;
}

//-----------------------------------------------------------------------------
void ArpModeCmd::send()
{
	TBBArpModeEvent event;
	event.boardmask = getBoardMask();
	event.mode = itsMode;
	itsPort.send(event);
	itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ArpModeCmd::ack(GCFEvent& e)
{
	TBBArpModeAckEvent ack(e);
	cout << "TBB  Info" << endl;
	cout << "---  -------------------------------------------------------" << endl;    
	for (int bnr=0; bnr < getMaxSelections(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status_mask[bnr] & TBB_SUCCESS) {
				cout << formatString(" %2d  arp mode set to %d",bnr, itsMode ) << endl;
			}	else {	
				cout << formatString(" %2d  %s",bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()) << endl;
			}
		}
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- STOPCEP -------------------------------------------------------------------
StopCepCmd::StopCepCmd(GCFPortInterface& port) : Command(port)
{
	cout << endl;
	cout << "== TBB ========================================= stop sending messages to CEP ====" << endl;
	cout << endl;
}

//-----------------------------------------------------------------------------
void StopCepCmd::send()
{
	TBBStopCepEvent event;
	event.boardmask = getBoardMask();
	itsPort.send(event);
	itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult StopCepCmd::ack(GCFEvent& e)
{
	TBBStopCepAckEvent ack(e);
	cout << "TBB  Info" << endl;
	cout << "---  -------------------------------------------------------" << endl;    
	for (int bnr=0; bnr < getMaxSelections(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status_mask[bnr] & TBB_SUCCESS) {
				cout << formatString(" %2d  CEP messages stopped",bnr ) << endl;
			}	else {	
				cout << formatString(" %2d  %s",bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()) << endl;
			}
		}
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- TEMPLIMIT -------------------------------------------------------------------
TempLimitCmd::TempLimitCmd(GCFPortInterface& port) : Command(port)
  , itsLimitHigh(0),itsLimitLow(0)
{
	cout << endl;
	cout << "== TBB ========================================= set temperature limits ====" << endl;
	cout << endl;
}

//-----------------------------------------------------------------------------
void TempLimitCmd::send()
{
	TBBTempLimitEvent event;
	event.boardmask = getBoardMask();
	event.high = itsLimitHigh;
	event.low = itsLimitLow;
	itsPort.send(event);
	itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult TempLimitCmd::ack(GCFEvent& e)
{
	TBBTempLimitAckEvent ack(e);
	cout << "TBB  Info" << endl;
	cout << "---  -------------------------------------------------------" << endl;    
	for (int bnr=0; bnr < getMaxSelections(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status_mask[bnr] & TBB_SUCCESS) {
				cout << formatString(" %2d  Temp limit set %d .. %d ",bnr, itsLimitLow, itsLimitHigh ) << endl;
			}	else {	
				cout << formatString(" %2d  %s",bnr, getDriverErrorStr(ack.status_mask[bnr]).c_str()) << endl;
			}
		}
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- ERASE IMAGE --------------------------------------------------------------
ErasefCmd::ErasefCmd(GCFPortInterface& port) : Command(port)
	,itsPage(0)
{
	cout << endl;
	cout << "== TBB ===================================================== erase flash ====" << endl;
	cout << endl;
	cout << "TBB  Info" << endl;
	cout << "---  -------------------------------------------------------" << endl;  

}

//-----------------------------------------------------------------------------
void ErasefCmd::send()
{
	TBBEraseImageEvent event;
	event.board = getBoard();
	event.image = itsPage;
	cout << formatString(" %2d  erasing flash memory of image %d", getBoard(), itsPage) << endl;
	cout << endl;
	cout << "     erasing will take about 8 seconds" << endl;	
	itsPort.send(event);
	itsPort.setTimer((long)16);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ErasefCmd::ack(GCFEvent& e)
{
	TBBEraseImageAckEvent ack(e);
	  
	if (ack.status_mask & TBB_SUCCESS) {
		cout << formatString(" %2d  image is erased",getBoard()) << endl;
	}	else {	
		cout << formatString(" %2d  %s",getBoard(), getDriverErrorStr(ack.status_mask).c_str()) << endl;
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- READ IMAGE --------------------------------------------------------------
ReadfCmd::ReadfCmd(GCFPortInterface& port) : Command(port)
	,itsPage(0)
{
	//memset(itsFileName,'\0',sizeof(itsFileName));
	//memset(itsPageData,0,sizeof(itsPageData));
	cout << endl;
	cout << "== TBB ====================================================== read flash ====" << endl;
	cout << endl;
	cout << "TBB  Info" << endl;
	cout << "---  -------------------------------------------------------" << endl;  
}

//-----------------------------------------------------------------------------
void ReadfCmd::send()
{
	TBBReadImageEvent event;
	event.board = getBoard();
	event.image = itsPage;
	cout << formatString(" %2d  reading flash memory of image %d", getBoard(), itsPage) << endl;
	cout << endl;
	cout << "     reading will take about 12 seconds" << endl;	
	itsPort.send(event);
	itsPort.setTimer((long)24);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ReadfCmd::ack(GCFEvent& e)
{
	TBBReadImageAckEvent ack(e);

		if (ack.status_mask & TBB_SUCCESS) {
			cout << formatString(" %2d  image is read",getBoard()) << endl;
		}	else {	
			cout << formatString(" %2d  %s",getBoard(), getDriverErrorStr(ack.status_mask).c_str()) << endl;
		}

		
		setCmdDone(true);
	

	return(GCFEvent::HANDLED);
}

//---- WRITE IMAGE -----------------------------------------------------------
WritefCmd::WritefCmd(GCFPortInterface& port) : Command(port)
	,itsPage(0)
{
	memset(itsFileNameTp,'\0',sizeof(itsFileNameTp));
	memset(itsFileNameMp,'\0',sizeof(itsFileNameMp));
	cout << endl;
	cout << "== TBB ===================================================== write flash ====" << endl;
	cout << endl;
	cout << "TBB  Info" << endl;
	cout << "---  -------------------------------------------------------" << endl;  
}

//-----------------------------------------------------------------------------
void WritefCmd::send()
{
	TBBWriteImageEvent event;
	event.board = getBoard();
	event.image = itsPage;
	event.version = static_cast<int32>(round(itsVersion * 10.));
	memcpy(event.filename_tp,itsFileNameTp,sizeof(itsFileNameTp));
	memcpy(event.filename_mp,itsFileNameMp,sizeof(itsFileNameMp));
	cout << formatString(" %2d  writing flash memory of image %d", getBoard(), itsPage) << endl;
	cout << endl;
	cout << "     writing will take about 25 seconds" << endl;	
	itsPort.send(event);
	itsPort.setTimer((long)60);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult WritefCmd::ack(GCFEvent& e)
{
	TBBWriteImageAckEvent ack(e);
	if (ack.status_mask & TBB_SUCCESS) {
		cout << formatString(" %2d  image is written",getBoard()) << endl;
	}	else {	
		cout << formatString(" %2d  %s",getBoard(), getDriverErrorStr(ack.status_mask).c_str()) << endl;
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- IMAGE INFO -------------------------------------------------------------
ImageInfoCmd::ImageInfoCmd(GCFPortInterface& port) : Command(port)
{
	cout << endl;
	cout << "== TBB ===================================================== image info ====" << endl;
	cout << endl;
}

//-----------------------------------------------------------------------------
void ImageInfoCmd::send()
{
	TBBImageInfoEvent event;
	event.board = getBoard();
	itsPort.send(event);
	itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ImageInfoCmd::ack(GCFEvent& e)
{
	TBBImageInfoAckEvent ack(e);
	cout << formatString("Reading image information from TBB %d", getBoard()) << endl;
	cout << endl;
	cout << "IMAGE   SW       Flash date_time        TP file name       MP file name" << endl;
	cout << "-----   ------   --------------------   ----------------   ----------------" << endl;  
	if (ack.status_mask & TBB_SUCCESS) {
		for (int image = 0; image < 16; image++) {	
			if (ack.write_date[image] == 0xFFFFFFFF) {
				//cout << formatString("  %2d   no information",image) << endl;
				cout << formatString("  %2d     free",image) << endl;
			} else {
				time_t write_time;
				struct tm t;
				double version;
				
				write_time = static_cast<time_t>(ack.write_date[image]);
				t = *gmtime(&write_time);
				version = static_cast<double>(ack.image_version[image] / 10.);
				cout << formatString("  %2d    V%5.1lf   %02d-%02d-%4d  %2d:%02d:%02d   %-16.16s   %-16.16s",
									 	image,
										version, 
									 	t.tm_mday,t.tm_mon+1,t.tm_year+1900,
										t.tm_hour,t.tm_min,t.tm_sec,
										ack.tp_file_name[image],
										ack.mp_file_name[image]) << endl;
				//cout << "TP: " << ack.tp_file_name[image] <<  ",  MP: " << ack.mp_file_name[image] << endl;
			}
		}
	}	else {	
		cout << formatString("TBB %2d  %s",getBoard(), getDriverErrorStr(ack.status_mask).c_str()) << endl;
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}



//---- READW -------------------------------------------------------------------
ReadwCmd::ReadwCmd(GCFPortInterface& port) : Command(port)
{
	cout << endl;
	cout << "== TBB ======================================================= read DDR2 ====" << endl;
	cout << endl;
	cout << "TBB  Info" << endl;
	cout << "---  -------------------------------------------------------" << endl;  
}

//-----------------------------------------------------------------------------
void ReadwCmd::send()
{
	TBBReadwEvent event;
	event.board = getBoard();
	event.mp = itsMp;
	event.addr = itsAddr;
	itsPort.send(event);
	itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ReadwCmd::ack(GCFEvent& e)
{
	TBBReadwAckEvent ack(e);
	for (int bnr=0; bnr < getMaxSelections(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status_mask & TBB_SUCCESS) {
				cout << formatString(" %2d  MP[%u] Addr[0x%08X]  [0x%08X] [0x%08X]"
						,bnr, itsMp, itsAddr, ack.wordlo, ack.wordhi ) << endl;
			}	else {	
				cout << formatString(" %2d  %s"
						,bnr, getDriverErrorStr(ack.status_mask).c_str()) << endl;
				itsAddr = itsStopAddr - 1;
			}
		}
	}
	//if (itsAddr == (itsStopAddr - 1))
	
	itsAddr++;
	if (itsAddr >= itsStopAddr) { 
		setCmdDone(true); 
	}	else { 
		//itsPort.setTimer((long)0);
	}
	return(GCFEvent::HANDLED);
}

//---- WRITEW -------------------------------------------------------------------
WritewCmd::WritewCmd(GCFPortInterface& port) : Command(port)
{
	cout << endl;
	cout << "== TBB ====================================================== write DDR2 ====" << endl;
	cout << endl;
}

//-----------------------------------------------------------------------------
void WritewCmd::send()
{
	TBBWritewEvent event;
	event.board = getBoard();
	event.mp = itsMp;
	event.addr = itsAddr;
	event.wordlo = itsWordLo;
	event.wordhi = itsWordHi;
	itsPort.send(event);
	itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult WritewCmd::ack(GCFEvent& e)
{
	TBBWritewAckEvent ack(e);
	cout << "TBB  Info" << endl;
	cout << "---  -------------------------------------------------------" << endl;    
	for (int bnr=0; bnr < getMaxSelections(); bnr++) {
		if (isSelected(bnr) ) {
			if (ack.status_mask & TBB_SUCCESS) {
				cout << formatString(" %2d  DDR2 write TBB_SUCCESS",bnr ) << endl;
			}	else {	
				cout << formatString(" %2d  %s",bnr, getDriverErrorStr(ack.status_mask).c_str()) << endl;
			}
		}
	}
	
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- TESTDDR ----------------------------------------------------------------
TestDdrCmd::TestDdrCmd(GCFPortInterface& port) : Command(port),
	itsCmdStage(0),itsMp(0), itsAddr(0), itsAddrLine(0), itsTestPatern(1), itsWordLo(0), itsWordHi(0),
	itsAddrLineErrors(0), itsDataLineErrors(0)
{
	cout << endl;
	cout << "==== TBB-board, test ddr =================================================" << endl;	
	cout << endl;
}

//-----------------------------------------------------------------------------
void TestDdrCmd::send()
{
	if ((itsCmdStage == 0)&&(itsMp == 0)&&(itsAddrLine == 0)) {
		cout << formatString("Testing DDR memory address lines of board %d",getBoard()) << endl;
		cout << endl;
		cout << "Mp  Addr   Write  Read                                 " << endl;
		cout << "Nr  Line   DDR    DDR     Info                         " << endl;
		cout << "--  -----  -----  -----   -----------------------------" << endl;
	}
	
	if ((itsCmdStage == 2)&&(itsMp == 0)&&(itsTestPatern == 1)) {
		cout << endl;
		cout << formatString("Testing DDR memory data lines of board %d",getBoard()) << endl;
		cout << endl;
		cout << "Mp  Write     Read      Write      Read                              " << endl;
		cout << "Nr  WordLow   WordLow   WordHigh   WordHigh  Info                    " << endl;
		cout << "--  --------  --------  --------   --------  ------------------------" << endl;
	}
	switch (itsCmdStage) {
			
		case 0: {
			TBBWritewEvent event;
			event.board = getBoard();
			event.mp = itsMp;
			event.addr = itsAddr;
			event.wordlo = itsAddrLine;
			event.wordhi = (itsMp << 8) + itsAddrLine;
			itsPort.send(event);
		} break;
		
		case 1: {
			TBBReadwEvent event;
			event.board = getBoard();
			event.mp = itsMp;
			event.addr = itsAddr;
			itsPort.send(event);
		} break;
		
		case 2: {
			TBBWritewEvent event;
			event.board = getBoard();
			event.mp = itsMp;
			event.addr = itsAddr;
			event.wordlo = itsTestPatern;
			event.wordhi = ~itsTestPatern;
			itsPort.send(event);	
		} break;
		
		case 3: {
			TBBReadwEvent event;
			event.board = getBoard();
			event.mp = itsMp;
			event.addr = itsAddr;
			itsPort.send(event);
		} break;
		
		default: break;
	}
	
	itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult TestDdrCmd::ack(GCFEvent& e)
{
	switch (itsCmdStage) {
		case 0: {
			TBBWritewAckEvent ack(e);
			if (!(ack.status_mask & TBB_SUCCESS)) {
				cout << formatString(" %2d  %s",getBoard(), getDriverErrorStr(ack.status_mask).c_str()) << endl;
			}
			itsAddr = (1 << itsAddrLine);
			itsAddrLine++;
			if (itsAddrLine == 28) {
				itsAddr = 0;
				itsAddrLine = 0;
				itsCmdStage = 1;
			}  		
		}break;
		
		case 1: {
			TBBReadwAckEvent ack(e);
			if (ack.status_mask & TBB_SUCCESS) {
				if (ack.wordlo == itsAddrLine) {
					cout << formatString("%2d  %5d  %5d  %5d   SAME",
						itsMp, itsAddrLine, itsAddrLine, ack.wordlo) << endl;
				} else {
					cout << formatString("%2d  %5d  %5d  %5d   FAULT",
						itsMp, itsAddrLine, itsAddrLine, ack.wordlo ) << endl;
					itsAddrLineErrors++;
				}
			} else {
				cout << formatString(" %2d  %s",getBoard(), getDriverErrorStr(ack.status_mask).c_str()) << endl;
			}
			itsAddr = (1 << itsAddrLine);
			itsAddrLine++;
			if (itsAddrLine == 28) {
				itsAddr = 0;
				itsAddrLine = 0;
				itsCmdStage = 0;
				itsMp++;
				if (itsMp == 4) {
					itsMp = 0;
					itsAddr = 0;
					itsCmdStage = 2;
					//setCmdDone(true);
				}
			}
		} break;
		
		case 2: {
			TBBWritewAckEvent ack(e);
			if (!(ack.status_mask & TBB_SUCCESS)) {
				cout << formatString(" %2d  %s",getBoard(), getDriverErrorStr(ack.status_mask).c_str()) << endl;
			}
			itsAddr++;
			itsTestPatern <<= 1;
			//if (itsTestPatern == 0) itsTestPatern == 1;
				
			if (itsTestPatern == 0x00000000) {
				itsAddr = 0;
				itsTestPatern = 1;
				itsCmdStage = 3;
			}  		
		}break;
		
		case 3: {
			TBBReadwAckEvent ack(e);
			if (ack.status_mask & TBB_SUCCESS) {
				if ((ack.wordlo == itsTestPatern)&&(ack.wordhi == ~itsTestPatern)) {
					cout << formatString("%2d  %08X  %08X  %08X  %08X   SAME",
						itsMp, itsTestPatern, ack.wordlo, ~itsTestPatern, ack.wordhi) << endl;
				} else {
					cout << formatString("%2d  %08X  %08X  %08X  %08X   FAULT",
						itsMp, itsTestPatern, ack.wordlo, ~itsTestPatern, ack.wordhi) << endl;
					itsDataLineErrors++;
				}
			} else {
				cout << formatString(" %2d  %s",getBoard(), getDriverErrorStr(ack.status_mask).c_str()) << endl;
			}
			itsAddr++;
			itsTestPatern <<= 1;
			//if (itsTestPatern == 0) itsTestPatern == 1;
				
			if (itsTestPatern == 0x00000000) {
				itsAddr = 0;
				itsTestPatern = 1;
				itsCmdStage = 2;
				itsMp++;
				if (itsMp == 4) {
					cout << formatString("  == summary ==") << endl;
					if (itsAddrLineErrors != 0) {
						cout << formatString("  %d  Addressline ERRORS",itsAddrLineErrors) << endl;
					} else {
						cout << formatString("  All Addresslines OK") << endl;
					}
					if (itsDataLineErrors != 0) {
						cout << formatString("  %d  Dataline ERRORS",itsDataLineErrors) << endl;
					} else {
						cout << formatString("  All Datalines OK") << endl;
					}
					setCmdDone(true);
				}
			}
		} break;
		
		
		default: break;
	}
	return(GCFEvent::HANDLED);
}

//---- READR -------------------------------------------------------------------
ReadrCmd::ReadrCmd(GCFPortInterface& port) : Command(port),
	itsMp(0), itsPid(0), itsRegId(0)
{
	cout << endl;
	cout << "==== TBB-board, read register ================================================" << endl;
}

//-----------------------------------------------------------------------------
void ReadrCmd::send()
{
	TBBReadrEvent event;
	event.board = (int32)getBoard();
	event.mp = itsMp;
	event.pid = itsPid;
	event.regid = itsRegId;
	
	itsPort.send(event);
	itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ReadrCmd::ack(GCFEvent& e)
{
	TBBReadrAckEvent ack(e);
	
	cout <<   "Board : " <<  getBoard() << endl;
	if (ack.status_mask & TBB_SUCCESS) {
		cout << "MP    : " << itsMp << endl;
		cout << "PID   : " << itsPid << endl;
		cout << "REGID : " << itsRegId << endl;
		cout << "Received data : "; 
		        
		for (int i = 0; i < (REG_SIZE[itsPid][itsRegId]); i++) {
			if (i % 4 == 0) {
				if (i != 0) cout << endl << "                ";
				cout << formatString("0x%02x: ",(i * 4));
			}
			cout << formatString("%08x ",ack.data[i]);
		}
	} else {
		cout << "ERROR : " << getDriverErrorStr(ack.status_mask).c_str();	
	}
	
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- WRITER -------------------------------------------------------------------
WriterCmd::WriterCmd(GCFPortInterface& port) : Command(port),
	itsMp(0), itsPid(0), itsRegId(0)
{
	for (int i = 0; i < 16; i++) {
		itsData[i] = 0;
	}
	cout << endl;
	cout << "==== TBB-board, write register ===============================================" << endl;	
	cout << endl;
}

//-----------------------------------------------------------------------------
void WriterCmd::send()
{
	TBBWriterEvent event;
	event.board = (int32)getBoard();
	event.mp = itsMp;
	event.pid = itsPid;
	event.regid = itsRegId;
	for (int i = 0; i < (REG_SIZE[itsPid][itsRegId]); i++) {
		event.data[i] = itsData[i];
	}
	itsPort.send(event);
	itsPort.setTimer(DELAY);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult WriterCmd::ack(GCFEvent& e)
{
	TBBWriterAckEvent ack(e);
	
	cout <<   "Board : " <<  getBoard() << endl;
	if (ack.status_mask & TBB_SUCCESS) {
		cout << "MP    : " << itsMp << endl;
		cout << "PID   : " << itsPid << endl;
		cout << "REGID : " << itsRegId << endl;
		cout << "Transmitted data : "; 
		for (int i = 0; i < (REG_SIZE[itsPid][itsRegId]); i++) {
			if (i % 4 == 0) {
				if (i != 0) cout << endl << "                ";
				cout << formatString("0x%02x: ",(i * 4));
			}
			cout << formatString("%08x ",itsData[i]);
		}
	}	else {	
		cout << getDriverErrorStr(ack.status_mask).c_str();
	}
	setCmdDone(true);

	return(GCFEvent::HANDLED);
}

//---- EADPAGE ------------------------------------------------------------------
ReadPageCmd::ReadPageCmd(GCFPortInterface& port) : Command(port),
	itsRcu(0),itsStartPage(0),itsPages(1),itsCmdStage(0),itsPage(0),itsTotalSize(0),itsStartAddr(0),itsSize(0),itsBoard(0),itsMp(0),
	itsStationId(0),itsRspId(0),itsRcuId(0),itsSampleFreq(0),itsTime(0),itsSampleNr(0),itsSamplesPerFrame(0),
	itsFreqBands(0),itsTotalSamples(0),itsTotalBands(0), itsBandNr(0), itsSliceNr(0)
{
	for (int i = 0; i < 512; i++) itsData[i] = 0;
	cout << endl;
	cout << "==== TBB-board, readx register ================================================" << endl;
	cout << endl;
}
//-----------------------------------------------------------------------------
void ReadPageCmd::send()
{
	
	switch (itsCmdStage) {
		
		case 0: {	// get info about allocated RCU's
			TBBRcuInfoEvent send;
			itsPort.send(send);
			itsPort.setTimer(DELAY);
		} break;
		
		case 1: {	// get total memmory size
  		TBBSizeEvent send;
  		send.boardmask = (1 << itsBoard);
  		itsPort.send(send);
  		itsPort.setTimer(DELAY);
		} break;
		
		case 2: {	// write page address to board
			TBBWriterEvent send;
			
			itsMp = static_cast<int32>((itsStartPage + itsPage) / itsTotalSize);	// MP of requested addr
			
			send.board = itsBoard;
			send.mp = itsMp;
			send.pid = PID6;
			send.regid = REGID1;
			send.data[0] = (itsStartPage + itsPage);
			send.data[1] = 0; 
			send.data[2] = 0;
			itsPort.send(send);
			itsPort.setTimer(DELAY);
			
			LOG_DEBUG_STR(formatString("Writer[%x][%x][%x][%x]", 
				send.mp,send.pid,send.regid,send.data[0]));
		} break;
		
		case 3: { // write page-read-cmd to board
			TBBWriterEvent send;
			send.board = itsBoard;
			send.mp = itsMp;
			send.pid = PID6;
			send.regid = REGID0;
			send.data[0] = PAGEREAD;
			send.data[1] = 0; 
			send.data[2] = 0;
			itsPort.send(send);
			itsPort.setTimer(DELAY);
			
			LOG_DEBUG_STR(formatString("Writer[%x][%x][%x][%x]", 
				send.mp,send.pid,send.regid,send.data[0]));
		} break;
		
		case 4: { // read first part of frame
			TBBReadxEvent send;
			send.board = itsBoard;	
			send.mp = itsMp;
			send.pid = PID6;
			send.regid = REGID2;
			send.pagelength = 256;
			send.pageaddr = 0;
			itsPort.send(send);
			itsPort.setTimer(DELAY);
			
			LOG_DEBUG_STR(formatString("Readx[%x][%x][%x][%x][%x]", 
				send.mp,send.pid,send.regid,send.pagelength,send.pageaddr));
		} break;
		
		case 5: {	// read second part of frame
			TBBReadxEvent send;
			send.board = itsBoard;	
			send.mp = itsMp;
			send.pid = PID6;
			send.regid = REGID2;
			send.pagelength = 256;
			send.pageaddr = 256;
			itsPort.send(send);
			itsPort.setTimer(DELAY);
			
			LOG_DEBUG_STR(formatString("Readx[%x][%x][%x][%x][%x]", 
				send.mp,send.pid,send.regid,send.pagelength,send.pageaddr));
		} break;
		
		default:{
		} break;
	}
}

//-----------------------------------------------------------------------------
GCFEvent::TResult ReadPageCmd::ack(GCFEvent& e)
{
	char basefilename[PATH_MAX];
	char filename[PATH_MAX];
	char timestring[256];

	int16 val[1400];
	
	double 				bar_size = 50;
	static double	bar_interval = 1;
	static double	bar_buff = 0;
		
	switch (itsCmdStage) {
		
		case 0: {
			TBBRcuInfoAckEvent ack(e);
			itsRcu = getRcu();
			itsState = ack.rcu_state[itsRcu];
			itsStartAddr = ack.rcu_start_addr[itsRcu];
			itsSize = ack.rcu_pages[itsRcu];
			itsBoard = ack.rcu_on_board[itsRcu];
			//itsMp = static_cast<int32>(ack.rcu_on_input[itsRcu] / 4);
			//cout << formatString("Rcu-%d Board[%d] Mp[%d]",itsRcu,itsBoard,itsMp) << endl;
			
			itsStartPage += itsStartAddr;
			
			if (itsState == 'F') {
				cout << "Rcu not allocated" << endl;
				itsCmdStage = 10;
			}
		} break;
				
		case 1: {
  		TBBSizeAckEvent ack(e);
  		if (!(ack.status_mask[itsBoard] & TBB_SUCCESS)) {
				cout << formatString("%s",getDriverErrorStr(ack.status_mask[itsBoard]).c_str()) << endl;
				itsCmdStage = 10;
			} else {
				itsTotalSize = ack.npages[itsBoard] / 4;
				
				if ((itsStartPage < itsStartAddr) || ((itsStartPage + itsPages) > (itsStartAddr + itsSize))) {
					cout << formatString("Requested Page belongs not to rcu %d", itsRcu) << endl;			 
				} else {
					//cout << formatString("StartPage = %u ",itsStartPage) << endl;			 
				}
			}
		} break;
		
		case 2: {
			TBBWriterAckEvent ack(e);
			if (!(ack.status_mask & TBB_SUCCESS)) {
				cout << formatString("%s",getDriverErrorStr(ack.status_mask).c_str()) << endl;
				itsCmdStage = 10;
			}
		} break;
		
		case 3: {
			TBBWriterAckEvent ack(e);
			if (!(ack.status_mask & TBB_SUCCESS)) {
				cout << formatString("%s",getDriverErrorStr(ack.status_mask).c_str()) << endl;
				itsCmdStage = 10;
			}
		} break;
		
		case 4: {
			TBBReadxAckEvent ack(e);
			if (!(ack.status_mask & TBB_SUCCESS)) {
				cout << formatString("%s", getDriverErrorStr(ack.status_mask).c_str()) << endl;
				itsCmdStage = 10;
		  }
		  
		  // memcpy(itsData, ack.pagedata, sizeof(ack.pagedata));
			for (int32 dn = 0; dn < 256; dn++) { 
				itsData[dn] = ack.pagedata[dn];
			}
		} break;
		
		case 5: {
			TBBReadxAckEvent ack(e);
			if (!(ack.status_mask & TBB_SUCCESS)) {
				cout << formatString("%s", getDriverErrorStr(ack.status_mask).c_str()) << endl;
				itsCmdStage = 10;
		  }
			for (int32 dn = 0; dn < 256; dn++) { 
				itsData[256 + dn] = ack.pagedata[dn];
			}
		} break;
	}
	
	// if error in communication stop
	if (itsCmdStage == 10) {
		setCmdDone(true);
		return(GCFEvent::HANDLED);
	}
	
	itsCmdStage++;
	if (itsCmdStage > 5) { // if stage > 5, received 1 frame
				
		bar_buff += 1;
		if (itsPage == 0) {
						
			itsStationId = static_cast<int>(itsData[0] & 0xFF);
			itsRspId = static_cast<int>((itsData[0] >> 8) & 0xFF);
			itsRcuId = static_cast<int>((itsData[0] >> 16) & 0xFF);
			itsSampleFreq = static_cast<int>((itsData[0] >> 24) & 0xFF);
			itsTime = static_cast<time_t>(itsData[2]);
			itsFreqBands = static_cast<int>((itsData[4] >> 16) & 0xFFFF);
			
			if (itsFreqBands == 0) {
				itsSampleNr = static_cast<uint32>(itsData[3]);
			} else {
				itsBandNr = static_cast<int>(itsData[3] & 0x03FF);
				itsSliceNr = static_cast<int>(itsData[3] >> 10);
			}
			strftime(timestring, 255, "%Y%m%d_%H%M%S", gmtime(&itsTime));
			
			cout << "Station ID      : " << itsStationId << endl;
			cout << "RSP ID          : " << itsRspId << endl;
			cout << "RCU ID          : " << itsRcuId << endl;
			cout << "Sample freq     : " << itsSampleFreq << " MHz" << endl;
			if (itsTime < 0) {
				cout << "Time            : invalid" << endl;
			} else {
				cout << "Time of 1e sample/slice    : " << timestring << " (" << (uint32)itsTime << ")" << endl ;
			}
			if (itsFreqBands > 0) {
				cout << "Slice number of 1e slice   : " << itsSliceNr << endl;
				cout << "Band number of 1e sample   : " << itsBandNr << endl;
				cout << "Number of bands            : " << itsFreqBands << endl;
				cout << "Data file format           : binary complex(int16 Re, int16 Im)" << endl;
			}	else {
				cout << "Sample number of 1e sample : " << itsSampleNr << endl;
				cout << "Data file format           : binary  int16" << endl;
			}
			
			// print size of progressbar on screen
			bar_interval = itsPages / bar_size;
			int recvtime = static_cast<int>((1043. / 262144.) * itsPages);  // measured 262144 pages in 1043 seconds
			int hours = recvtime / (60 * 60);
			int mins = (recvtime - (hours * 60 * 60)) / 60;
			int secs = recvtime - (hours * 60 * 60) - (mins * 60) + 1; // 1 second for overhead
			cout << endl;
			cout << "          Estimated receive time " << formatString("%d:%02d:%02d",hours, mins, secs) << endl;
			cout << "Progress |";
			for (int i = 0; i < bar_size; i++) {
				cout << "-";
			}
			cout << "|" << endl ;
			cout <<   "         |" << flush;
			
			snprintf(basefilename, PATH_MAX, "%s_%s_%02d%02d",(itsFreqBands == 0)?"rw":"sb",timestring,itsStationId,itsRcuId);
		}
		itsSamplesPerFrame = static_cast<uint32>(itsData[4] & 0xFFFF);
		
		// print receive progress on screen
		if (bar_interval < 1.) {
			int count = static_cast<int>(ceil(bar_buff / bar_interval));
			for (int i = 0; i < count; i++) {
				cout << "x" << flush;
			}
			bar_buff -= (count * bar_interval); 
		} else {
			if (bar_buff >= bar_interval) {
				cout << "x" << flush;
				bar_buff -= bar_interval;
			}
		}
								
		int sample_cnt = 0;
		int val_cnt = 0;
		int data_cnt = 22; // 22 = startadress of data in frame
		
		// extract payload
		if (itsFreqBands > 0) {	// if itsFreqBands > 0 then it's SPECTRAL data
			itsTotalSamples += itsSamplesPerFrame;

			// convert uint32 to complex int16
			while (sample_cnt < itsSamplesPerFrame) {
					val[val_cnt++] = static_cast<int16>(itsData[data_cnt] & 0xFFFF);	// re part
					val[val_cnt++] = static_cast<int16>((itsData[data_cnt++] >> 16) & 0xFFFF);	// im part
					sample_cnt++;
			}
		}	else {	// if itsFreqBands = 0, it's RAW data

			uint32 data[3];
			itsTotalSamples += itsSamplesPerFrame;

			// 1e convert uint32 to int12
			while (sample_cnt < itsSamplesPerFrame) {
				// get 96 bits from received data
				data[0] = itsData[data_cnt++];
				data[1] = itsData[data_cnt++];
				data[2] = itsData[data_cnt++];
				
				// extract 8 values of 12 bit
				val[val_cnt++] = static_cast<int16>(  data[0] & 0x00000FFF);
				val[val_cnt++] = static_cast<int16>(( data[0] & 0x00FFF000) >> 12);
				val[val_cnt++] = static_cast<int16>(((data[0] & 0xFF000000) >> 24) | ((data[1] & 0x0000000F) << 8));
				val[val_cnt++] = static_cast<int16>(( data[1] & 0x0000FFF0) >> 4 );
				val[val_cnt++] = static_cast<int16>(( data[1] & 0x0FFF0000) >> 16);
				val[val_cnt++] = static_cast<int16>(((data[1] & 0xF0000000) >> 28) | ((data[2] & 0x000000FF) << 4));
				val[val_cnt++] = static_cast<int16>(( data[2] & 0x000FFF00) >> 8);
				val[val_cnt++] = static_cast<int16>(( data[2] & 0xFFF00000) >> 20);
				
				sample_cnt += 8;
			}
			
			// 2e convert all received samples from signed 12bit to signed 16bit
			for (int cnt = 0; cnt < val_cnt; cnt++) {
				if (val[cnt] & 0x0800) val[cnt] |= 0xF000;
			}
		}
		
		// write all data to file
		FILE* file;
				
		if (val_cnt > 0) {
			// save unpacked frame to file
			snprintf(filename, PATH_MAX, "%s.dat",basefilename);
			file = fopen(filename,"a");
			fwrite(&itsData[0],sizeof(uint32),22,file);		// frame header 88 bytes (4 x 22)
			fwrite(&val[0],sizeof(int16),val_cnt,file);		// payload
			fwrite(&itsData[509],sizeof(uint32),1,file);	// payload CRC 4 bytes (4 x 1)
			fclose(file);
		}
		
		itsPage++;
		if ((itsPage < itsPages) && (itsPage < itsSize)) { 
				itsCmdStage = 2;
		}	else {
			// last page received, print last info on screen
			cout << "|" << endl;
			cout << endl;
			cout << "Total received pages  : " << itsPage;
			if (itsPage > 1) {
				cout << " (page " << itsStartPage << " .. " << (itsStartPage + itsPage - 1) << ")" << endl;
			} else {
				cout << " (page " << itsStartPage << ")" << endl;
			}
			cout << "Total received samples: " << itsTotalSamples << endl;

			if (val_cnt > 0) {
				cout << "Filename        : " << basefilename << ".nfo" << endl;
				cout << "                : " << basefilename << ".dat" << endl;
				cout << endl;
				cout << "Each frame exists of:" << endl;
				cout << "  HEADER(88 bytes) + PAYLOAD(see header for size) + PAYLOAD_CRC(4 bytes)" << endl;
			} else {
				cout << "Filename        : NO DATA IN FRAME" << endl;
			}
			
			
			
			// save page information to file
			strftime(timestring, 255, "%Y-%m-%d  %H:%M:%S", gmtime(&itsTime));

			snprintf(filename, PATH_MAX, "%s.nfo",basefilename);
			file = fopen(filename,"w");		
			
			fprintf(file,  "Station ID      : %d",itsStationId);
			fprintf(file,  "RSP ID          : %d",itsRspId);
			fprintf(file,  "RCU ID          : %d",itsRcuId);
			fprintf(file,  "Sample freq     : %d MHz",itsSampleFreq);
			if (itsTime < 0) {
				fprintf(file,"Time            : invalid");
			} else {
				fprintf(file,"Time of 1e sample/slice  : %s (%u)",timestring,(uint32)itsTime);
			}
			if (itsFreqBands > 0) {
				fprintf(file,"Slice number of 1e slice : %d",itsSliceNr);
				fprintf(file,"Band number of 1e sample : %d",itsBandNr);
				fprintf(file,"Number of bands          : %d",itsFreqBands);
				fprintf(file,"Data file format         : binary complex(int16 Re, int16 Im)");
			}	else {
				fprintf(file,"Sample number of 1e sample : %u",itsSampleNr);
				fprintf(file,"Total number of samples    : %u",itsTotalSamples);
				fprintf(file,"Data file format           : binary  int16");
				fprintf(file," ");
			}
			fclose(file);
		}
	}

	if (itsPage == itsPages) {
		setCmdDone(true);
	}

	return(GCFEvent::HANDLED);
}

//====END OF TBB COMMANDS========================================================================== 


//---- HELP --------------------------------------------------------------------
void TBBCtl::commandHelp(int level)
{
	cout << endl;
	cout << endl;
	cout << "> > > > tbbctl COMMAND USAGE > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > >" << endl;
	cout << endl;
	cout << " #  --command                : all boards or active rcu's are selected, and will be displayed" << endl;
	cout << " #  --command --select=<set> : only information for all selected boards or rcu's is displayed"  << endl;
	cout << " #    Example: --select=0,1,4  or  --select=0:6  or  --select=0,1,2,8:11" << endl;
	cout << endl;
	cout << "______| CHANNEL |_________________________________________________________________________________________________________" << endl;
	cout << " tbbctl --rcuinfo [--select=<set>]                                  # list rcu info, only allocated rcu's are listed" << endl;
	cout << " tbbctl --alloc [--select=<set>]                                    # allocate memmory locations for selected rcu's" << endl;
	cout << " tbbctl --free [--select=<set>]                                     # free memmory locations for selected rcu's" << endl;
	cout << " tbbctl --record [--select=<set>]                                   # start recording on selected rcu's" << endl;
	cout << " tbbctl --stop [--select=<set>]                                     # stop recording on selected rcu's" << endl;
	cout << " tbbctl --mode=[transient | subbands]                               # set mode to configure UDP/IP header for CEP tranport" << endl;
	cout << "        # before using: --read, --arp or --arpmode," << endl;   
	cout << "        # first use --mode to setup UDP/IP header" << endl;
	cout << " tbbctl --read=rcunr,secondstime,sampletime,prepages,postpages      # transfer recorded data from rcunr to CEP, " << endl;
	cout << " tbbctl --stopcep [--select=<set>]                                  # stop sending CEP messages" << endl; 
	cout << " tbbctl --arp [--select=<set>]                                      # send 1 arp message" << endl; 
	cout << " tbbctl --arpmode=mode [--select=<set>]                             # set arp mode, 0=manual, 1=auto" << endl; 
	cout << endl;
	cout << "______| TRIGGER |_________________________________________________________________________________________________________" << endl;
	cout << " tbbctl --settings [--select=<set>]                                 # list trigger settings for selected rcu's" << endl;
	cout << " tbbctl --release [--select=<set>]                                  # release trigger system for selected rcu's" << endl;
	cout << " tbbctl --generate [--select=<set>]                                 # generate a trigger for selected rcu's" << endl;	
	cout << " tbbctl --setup=level,start,stop,filter,window,mode [--select=<set>]# setup trigger system for selected rcu's, mode = 0/1" << endl;
	cout << " tbbctl --coef=c0,c1,c2,c3 [--select=<set>]                         # set trigger coeffients for elected rcu's" << endl;
	cout << " tbbctl --triginfo=rcu                                              # get trigger info for selected rcu" << endl;
	cout << " tbbctl --listen                                                    # put in listen mode, all triggers will be listed" << endl;
	cout << endl;
	cout << "______| INFO |____________________________________________________________________________________________________________" << endl;
	cout << " tbbctl --version [--select=<set>]                                  # get version information from selected boards" << endl;
	cout << " tbbctl --status [--select=<set>]                                   # get status information from selected boards" << endl;	
	cout << " tbbctl --size [--select=<set>]                                     # get installed memory size from selected boards" << endl;
	cout << endl;
	cout << "______| DDR |_____________________________________________________________________________________________________________" << endl;
	cout << " tbbctl --readpage=rcunr,startpage,npages                           # ddr read npages from rcunr, starting on startpage" << endl;
	if (level == 3) {
		cout << " tbbctl --testddr=board                                             # ddr memory test, adress and data lines" << endl;
		cout << " tbbctl --readddr=board,mp,addr,size                                # ddr read, (size x 2) words starting on addr" << endl;
		cout << " tbbctl --writeddr=board,mp,addr,wordL,wordH                        # ddr write, 2 words to starting on addr" << endl;
		cout << endl;
		cout << "______| MP-REGISTER |_____________________________________________________________________________________________________" << endl;
		cout << " tbbctl --readreg=board,mp,pid,regid                                # register read" << endl;
		cout << " tbbctl --writereg=board,mp,pid,regid,data(hex input)               # register write" << endl;
		cout << endl;
		cout << "______| FLASH |___________________________________________________________________________________________________________" << endl;
		cout << " tbbctl --eraseimage=board,image                                    # erase image from flash" << endl;
		cout << " tbbctl --readimage=board,image                                     # read image from flash to file" << endl;
		cout << " tbbctl --writeimage=boardnr,imagenr,version,tpfilename,mpfilename  # write tp and mp file to imagenr on boardnr" << endl;
		cout << "                                                                    # version is the sw version of the image stored" << endl;
	} 
	if (level == 1) {
		cout << endl;
		cout << "______| FLASH |___________________________________________________________________________________________________________" << endl;
	}
	cout << " tbbctl --imageinfo=board                                           # read info from all images on board" << endl;
	cout << " tbbctl --config=imagenr [--select=<set>]                           # reconfigure TP and MP's with imagenr [0..31] on " << endl;
	cout << "                                                                    # selected boards" << endl;
	cout << "__________________________________________________________________________________________________________________________" << endl;
	cout << " tbbctl --templimits=high,low [--select=<set>]                      # set temp limts for fan control" << endl; 
	cout << " tbbctl --clear [--select=<set>]                                    # clear selected board" << endl;
	cout << " tbbctl --reset [--select=<set>]                                    # reset to factory images on selected boards" << endl;
	cout << endl;
	cout << " tbbctl --help                                                      # this help screen" << endl;
	cout << "< < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < " << endl;
	cout << endl;
}
//-----------------------------------------------------------------------------

//====END OF COMMANDS==========================================================================
//-----------------------------------------------------------------------------
TBBCtl::TBBCtl(string name, int argc, char** argv): GCFTask((State)&TBBCtl::initial, name),
	itsCommand(0),itsActiveBoards(0),itsMaxBoards(0),itsMaxChannels(0),itsArgc(argc),itsArgv(argv)
{
	for(int boardnr = 0; boardnr < MAX_N_TBBBOARDS; boardnr++) {
		itsMemory[boardnr] = 0;
	}
  registerProtocol (TBB_PROTOCOL,      TBB_PROTOCOL_STRINGS);
	itsServerPort.init(*this, MAC_SVCMASK_TBBDRIVER, GCFPortInterface::SAP, TBB_PROTOCOL);
	itsCmdTimer = new GCFTimerPort(*this, "AliveTimer");
}

//-----------------------------------------------------------------------------
TBBCtl::~TBBCtl()
{
	if (itsCommand)		{ delete itsCommand; }
  if (itsCmdTimer)	{ delete itsCmdTimer; }
}

//-----------------------------------------------------------------------------
GCFEvent::TResult TBBCtl::initial(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal) {
    case F_INIT: {
  	} break;

    case F_ENTRY: {
    	if (!itsServerPort.isConnected()) {
      	itsServerPort.open();
      	itsServerPort.setTimer(DELAY);
      }
    } break;

    case F_CONNECTED: {
      if (itsServerPort.isConnected()) {
        itsServerPort.cancelAllTimers();
        TBBGetConfigEvent getconfig;
        itsServerPort.send(getconfig);
       }
    } break;

		case TBB_DRIVER_BUSY_ACK: {
    	cout << endl << "=x=x=   DRIVER BUSY, try again   =x=x=" << endl << flush;
    	GCFTask::stop();    
    }

    case TBB_GET_CONFIG_ACK: {
      TBBGetConfigAckEvent ack(e);
      itsMaxBoards		= ack.max_boards;
      itsActiveBoards	= ack.active_boards_mask;
			itsMaxChannels = itsMaxBoards * 16;
      //cout << endl;
      //cout << formatString("Max nr of TBB boards = %d",itsMaxBoards) << endl;
			//cout << formatString("Max nr of Channels   = %d",itsMaxChannels)//cout << endl;;
			//cout << endl;
			//cout << formatString("Active boards mask   = 0x%03X",itsActiveBoards)//cout << endl;;
			
			// send subscribe 
      TBBSubscribeEvent subscribe;
			subscribe.triggers = false;
			subscribe.hardware = true;
      itsServerPort.send(subscribe);
    } break;
		
		case TBB_SUBSCRIBE_ACK: {
			TRAN(TBBCtl::docommand);
		} break;
		
    case F_DISCONNECTED: {
      //port.setTimer(DELAY);
      port.close();
    } break;

    case F_TIMER: {
      // try again
      cout << "   =x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=" << endl;
      cout << "   =x=         TBBDriver is NOT responding           =x=" << endl;
		cout << "   =x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=x=" << endl;
      //itsServerPort.open();
      exit(EXIT_FAILURE);
    } break;

    default: {
      status = GCFEvent::NOT_HANDLED;
    } break;
  }

  return(status);
}

//-----------------------------------------------------------------------------
GCFEvent::TResult TBBCtl::docommand(GCFEvent& e, GCFPortInterface& port)
{
  //cout << "docommand signal:" << eventName(e) << endl;
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal) {
    case F_INIT: {
  	} break;
  	
    case F_ENTRY: {
	  	// reparse options
      itsCommand = parse_options(itsArgc, itsArgv);
      if (itsCommand == 0) {
        cout << "Warning: no command specified." << endl;
        exit(EXIT_FAILURE);
      }
      itsCommand->send();
    } break;

    case F_CONNECTED: {
    } break;
    
    case F_DISCONNECTED: {
      port.close();
      cout << formatString("Error: port '%s' disconnected.",port.getName().c_str()) << endl;
      exit(EXIT_FAILURE);
    } break;
		
		case F_TIMER: {
			if (&port == itsCmdTimer) {
				itsCommand->send();
			} else {
				cout << "Timeout, tbbctl stopped" << endl;
				cout << endl;
				TBBUnsubscribeEvent unsubscribe;
      	itsServerPort.send(unsubscribe);
    		GCFTask::stop();
    	}
		} break;
    
    case TBB_BOARDCHANGE: {
    } break;
    
    case TBB_DRIVER_BUSY_ACK: {
    	cout << "DRIVER BUSY, try again" << endl << flush;
    	GCFTask::stop();    
    }
    		
    case TBB_ALLOC_ACK:
    case TBB_RCU_INFO_ACK:
    case TBB_FREE_ACK:
    case TBB_RECORD_ACK:
    case TBB_STOP_ACK:
    case TBB_TRIG_SETTINGS_ACK:	
    case TBB_TRIG_RELEASE_ACK:
    case TBB_TRIG_SETUP_ACK:	
    case TBB_TRIG_COEF_ACK:
    case TBB_TRIG_GENERATE_ACK:
    case TBB_TRIG_INFO_ACK:
    case TBB_TRIGGER:
    case TBB_READ_ACK:
    case TBB_MODE_ACK:
    case TBB_VERSION_ACK:
    case TBB_SIZE_ACK:
    case TBB_STATUS_ACK: 
    case TBB_CLEAR_ACK:
    case TBB_RESET_ACK:
    case TBB_CONFIG_ACK:
    case TBB_ERASE_IMAGE_ACK:
    case TBB_READ_IMAGE_ACK:
    case TBB_WRITE_IMAGE_ACK:
    case TBB_IMAGE_INFO_ACK:
    case TBB_READW_ACK:
    case TBB_WRITEW_ACK:
    case TBB_READR_ACK:
    case TBB_WRITER_ACK:
    case TBB_READX_ACK:
    case TBB_SUBSCRIBE_ACK:
    case TBB_ARP_ACK:
		case TBB_ARP_MODE_ACK:
		case TBB_STOP_CEP_ACK:
		case TBB_TEMP_LIMIT_ACK: {	
    	itsServerPort.cancelAllTimers();
    	status = itsCommand->ack(e); // handle the acknowledgement
    	if (!itsCommand->isCmdDone() && itsCommand->isCmdSendNext()) {
    		// not done send next command
    		itsCmdTimer->setTimer((long)0);
    	}
    } break;

    default: {
      cout << "Error: unhandled event." << endl;
      cout << formatString("Error: unhandled event. %d",e.signal) << endl;
      //TBBUnsubscribeEvent unsubscribe;
      //itsServerPort.send(unsubscribe);
      GCFTask::stop();
    } break;
  }
	
	if (itsCommand->isCmdDone()) {
  	//TBBUnsubscribeEvent unsubscribe;
    //itsServerPort.send(unsubscribe);
    cout << flush;
    GCFTask::stop();
  }
	 	 
  return(status);
}

//-----------------------------------------------------------------------------
Command* TBBCtl::parse_options(int argc, char** argv)
{
  Command*	command = 0;
  std::list<int> select;
  
  optind = 0; // reset option parsing
  //opterr = 0; // no error reporting to stderr
  
  while(1) {
  	static struct option long_options[] = {
			{ "select",			required_argument,	0,	'l' }, 
			{ "alloc",			no_argument,				0,	'a' }, 
			{ "rcuinfo",		no_argument,				0,	'i' }, 
			{ "free",				no_argument,				0,	'f' }, 
		  { "record",			no_argument,				0,	'r' }, 
		  { "stop",				no_argument,				0,	's' },
		  { "settings",		no_argument,				0,	'b' }, 
		  { "release",		no_argument,				0,	'e' }, 
		  { "generate",		no_argument,				0,	'g' }, 
		  { "setup",			required_argument,	0,	't' }, 
		  { "coef",				required_argument,	0,	'c' }, 
		  { "triginfo",		required_argument,	0,	'I' }, 
		  { "listen",     optional_argument,	0,	'L' }, 
			{ "read",				required_argument,	0,	'R' }, 
			{ "mode",				required_argument,	0,	'm' }, 
			{ "version",		no_argument,				0,	'v' }, 
			{ "size",				no_argument,				0,	'z' }, 
			{ "status",			no_argument,				0,	'A' }, 
			{ "readpage",		required_argument,	0,	'p' }, 
      { "arp",			  no_argument,				0,	'D' },
      { "arpmode",		required_argument,	0,	'E' },
      { "stopcep",		no_argument,				0,	'F' },
      { "templimits",	required_argument,	0,	'G' },
			{ "clear",			no_argument,				0,	'C' }, 
			{ "reset",			no_argument,				0,	'Z' }, 
			{ "config",			required_argument,	0,	'S' }, 
		  { "eraseimage",	required_argument,	0,	'1' }, 
		  { "readimage",	required_argument,	0,	'2' }, 
		  { "writeimage",	required_argument,	0,	'3' }, 
			{ "imageinfo",	required_argument,	0,	'8' }, 
		  { "readddr",		required_argument,	0,	'4' }, 
		  { "writeddr",		required_argument,	0,	'5' }, 
		  { "testddr",		required_argument,	0,	'9' }, 
		  { "readreg",		required_argument,	0,	'6' }, 
		  { "writereg",		required_argument,	0,	'7' }, 
			{ "help",				no_argument,				0,	'h' }, 
			{ "expert",			no_argument,				0,	'X' },
		  { 0, 					  0, 									0,	0 },
		};

    int option_index = 0;
    int c = getopt_long(argc, argv,
												"l:afrsbegt:c:I:L::R:m:vzAp:DE:FG:CZS:1:2:3:8:4:5:9:6:7:hX",
				long_options, &option_index);
		
    if (c == -1) {
			break;
		}

		switch (c) {
			
			case 'l': { 	// --select
	  		if (!command->isSelectionDone()) {
  	  		if (optarg) {
  	      	if (!command) {
  		  			cout << "Error: 'command' argument should come before --select argument" << endl;
  		  			exit(EXIT_FAILURE);
  					}
  					select = strtolist(optarg, command->getMaxSelections());
  											
  	      	if (select.empty()) {
  		  			cout << "Error: invalid or missing '--select' option" << endl;
  		  			exit(EXIT_FAILURE);
  					}	else {
  						command->setSelected(true);
  					}
  	    	}	else {
  	      	cout << "Error: option '--select' requires an argument" << endl;
  	      	exit(EXIT_FAILURE);
  	      }
				} else {
	      	cout << "Error: channels already selected" << endl;
				}
	    } break;
	    
	    case 'a': { 	// --alloc
				if (command) delete command;
				AllocCmd* alloccmd = new AllocCmd(itsServerPort);
				command = alloccmd;
				command->setCmdType(RCUCMD);
			}	break;
			
			case 'i': { 	// --channelinfo
				if (command) delete command;
				ChannelInfoCmd* channelinfocmd = new ChannelInfoCmd(itsServerPort);
				command = channelinfocmd;
				command->setCmdType(RCUCMD);
			}	break;
			
			case 'f': { 	// --free
				if (command) delete command;
				FreeCmd* freecmd = new FreeCmd(itsServerPort);
				command = freecmd;
				command->setCmdType(RCUCMD);
			}	break;
			
			case 'r': { 	// --record
				if (command) delete command;
				RecordCmd* recordcmd = new RecordCmd(itsServerPort);
				command = recordcmd;
				command->setCmdType(RCUCMD);
			}	break;
			
			case 's': { 	// --stop
				if (command) delete command;
				StopCmd* stopcmd = new StopCmd(itsServerPort);
				command = stopcmd;
				command->setCmdType(RCUCMD);
			}	break;
			
			case 'b': {
				if (command) delete command;
				TriggerSettingsCmd* trigsettingscmd = new TriggerSettingsCmd(itsServerPort);
				command = trigsettingscmd;
				command->setCmdType(RCUCMD);	
			} break;
			
			case 'e': { 	// --trigrelease
				if (command) delete command;
				TrigReleaseCmd* trigreleasecmd = new TrigReleaseCmd(itsServerPort);
				command = trigreleasecmd;
				if (optarg) {
					int rcu = 0;
					int numitems = sscanf(optarg, "%d",&rcu);
					if (numitems == 0 || numitems == EOF || rcu < 0 || rcu >= MAX_N_RCUS) {
						cout << "Error: invalid number of arguments. Should be of the format " << endl;
						cout << "       '--trigclr=rcu' " << endl;  
						exit(EXIT_FAILURE);
					}
					select.clear();
		  		select.push_back(rcu);
					command->setSelected(true);
				}
				
				command->setCmdType(RCUCMD);
			}	break;
			
			case 'g': { 	// --triggenerate
				if (command) delete command;
				TrigGenerateCmd* triggeneratecmd = new TrigGenerateCmd(itsServerPort);
				command = triggeneratecmd;
				command->setCmdType(RCUCMD);
			}	break;
			
			case 't': { 	// --trigsetup
				if (command) delete command;
				TrigSetupCmd* trigsetupcmd = new TrigSetupCmd(itsServerPort);
				command = trigsetupcmd;
				
				if (optarg) {
					uint32 level = 0;
					uint32 start = 0;
					uint32 stop = 0;
					uint32 filter = 0;
					uint32 window = 0;
					uint32 triggermode = 0;
					
					int numitems = sscanf(optarg, "%u,%u,%u,%u,%u,%u",&level, &start, &stop, &filter, &window, &triggermode);
					// check if valid arguments
					if (	numitems < 6 || numitems == EOF 
							|| level < 1 || level > 255 
							|| start < 1 || start > 15 
							|| stop < 1 || stop > 15 
							|| window > 6
							|| triggermode > 1)
					{
						cout << "Error: invalid number of arguments. Should be of the format " << endl;
						cout << "       '--trigsetup=level, start, stop, filter, window, mode' (use decimal values)" << endl;  
						cout << "       level=1..255,  start=1..15,  stop=1..15,  filter=0(on) or 1(off)" << endl;
						cout << "       window=0..6, mode=0(single shot) or 1(continues, don't use this mode)" << endl;
						exit(EXIT_FAILURE);
					}
					
					//================================================
					// set triggermode to single shot, continues mode
					// will give more than 6000 triggers per second per
					// board, if level is chosen to low, the driver can't
					// handle that amount of trigger per board.
					if (triggermode == 1) {
						cout << "mode=1 can generate to many triggers, auto corrected to mode=0(single-shot)" << endl;
					}	
					triggermode = 0;
					//================================================
					
										
					trigsetupcmd->setLevel(static_cast<uint16>(level));
					trigsetupcmd->setStartMode(static_cast<uint8>(start));
					trigsetupcmd->setStopMode(static_cast<uint8>(stop));
					trigsetupcmd->setFilter(static_cast<uint8>(filter));
					trigsetupcmd->setWindow(static_cast<uint8>(window));
					trigsetupcmd->setOperatingMode(0);
					trigsetupcmd->setTriggerMode(static_cast<uint16>(triggermode));
				}
							
				command->setCmdType(RCUCMD);
			}	break;
			
			case 'c': { 	// --trigcoef
				if (command) delete command;
				TrigCoefficientCmd* trigcoefficientcmd = new TrigCoefficientCmd(itsServerPort);
				command = trigcoefficientcmd;
				if (optarg) {
					uint32 c0 = 0;
					uint32 c1 = 0;
					uint32 c2 = 0;
					uint32 c3 = 0;
					int numitems = sscanf(optarg, "%u,%u,%u,%u",&c0, &c1, &c2, &c3);
					if (numitems < 4 || numitems == EOF) {
						cout << "Error: invalid number of arguments. Should be of the format " << endl;
						cout << "       '--trigcoef=c0, c1, c2, c3'   (use decimal values)" << endl;
						cout << "       co, c1, c2, c3=16bit value"<< endl; 
						exit(EXIT_FAILURE);
					}
					trigcoefficientcmd->setC0(static_cast<uint16>(c0));
					trigcoefficientcmd->setC1(static_cast<uint16>(c1));
					trigcoefficientcmd->setC2(static_cast<uint16>(c2));
					trigcoefficientcmd->setC3(static_cast<uint16>(c3));
				}
				command->setCmdType(RCUCMD);
			}	break;
			
			case 'I': { 	// --triginfo
				if (command) delete command;
				TrigInfoCmd* triginfocmd = new TrigInfoCmd(itsServerPort);
				command = triginfocmd;
				if (optarg) {
					int rcu = 0;
					int numitems = sscanf(optarg, "%d",&rcu);
					if (numitems < 1 || numitems == EOF) {
						cout << "Error: invalid number of arguments. Should be of the format " << endl;
						cout << "       '--triginfo=rcu' " << endl; 
						cout << "       rcu=0.." << (MAX_N_RCUS - 1) << endl;  
						exit(EXIT_FAILURE);
					}
					select.clear();
		  		select.push_back(rcu);
					command->setSelected(true);
				}
				command->setCmdType(RCUCMD);
			}	break;
			
			case 'L': { 	// --listen
				if (command) delete command;
				ListenCmd* listencmd = new ListenCmd(itsServerPort);
				command = listencmd;
				char listen_mode[64];
				if (optarg) {
					strcpy(listen_mode,"one_shot");
					sscanf(optarg, "%s", listen_mode);
					if ((strcmp(listen_mode,"one_shot") != 0 && strcmp(listen_mode,"continues") != 0)) {
						cout << "Error: invalid argument. Should be of the format " << endl;
						cout << "       '--listen=[one_shot | continues]'" << endl;
						exit(EXIT_FAILURE);
					}
				}
				if (strcmp(listen_mode,"one_shot") == 0) {
					listencmd->setListenMode(TBB_LISTEN_ONE_SHOT);
				}
				if (strcmp(listen_mode,"continues") == 0) {
					listencmd->setListenMode(TBB_LISTEN_CONTINUES);
				}
				command->setCmdType(RCUCMD);
			}	break;
			
			case 'R': { 	// --read
				if (command) delete command;
				ReadCmd* readcmd = new ReadCmd(itsServerPort);
				command = readcmd;
				
				if (optarg) {
					int rcu = 0;
					uint32 secondstime = 0;
					uint32 sampletime = 0;
					uint32 prepages = 0;
					uint32 postpages = 0;
					int numitems = sscanf(optarg, "%d,%u,%u,%u,%u",
						&rcu, &secondstime, &sampletime, &prepages, &postpages);
					if (numitems < 5 || numitems == EOF || rcu < 0 || rcu >= MAX_N_RCUS) {
						cout << "Error: invalid number of arguments. Should be of the format " << endl;
						cout << "       '--read=rcu,secondstime,sampletime,prepages,postpages' " << endl;
						cout << "       rcu=0.." << (MAX_N_RCUS - 1) << ",  time = secondstime + (sampletime * sample-interval) "<< endl;  
						exit(EXIT_FAILURE);
					}
					readcmd->setSecondsTime(secondstime);
					readcmd->setSampleTime(sampletime);
					readcmd->setPrePages(prepages);
					readcmd->setPostPages(postpages);
					
					select.clear();
		  		select.push_back(rcu);
					command->setSelected(true);
				} 
				command->setCmdType(RCUCMD);
			}	break;
			
			case 'm': { 	// --mode
				if (command) delete command;
				ModeCmd* modecmd = new ModeCmd(itsServerPort);
				command = modecmd;
				if (optarg) {
					char rec_mode[64];
					int numitems = sscanf(optarg, "%s",
						rec_mode);
					if (numitems < 1 || numitems == EOF ||(strcmp(rec_mode,"transient") != 0 && strcmp(rec_mode,"subbands") != 0)) {
						cout << "Error: invalid number of arguments. Should be of the format " << endl;
						cout << "       '--udp=rec_mode'" << endl;
						cout << "       rec_mode=transient or subbands" << endl;  
						exit(EXIT_FAILURE);
					}
					
					if (strcmp(rec_mode,"transient") == 0)
						modecmd->setRecMode(TBB_MODE_TRANSIENT);
					if (strcmp(rec_mode,"subbands") == 0)
						modecmd->setRecMode(TBB_MODE_SUBBANDS);
				} 
				command->setCmdType(BOARDCMD);
			}	break;									
						
			case 'v': { 	// --version
	  		if (command) delete command;
	    	VersionCmd* versioncmd = new VersionCmd(itsServerPort);
	  		command = versioncmd;
				command->setCmdType(BOARDCMD);
	  	}	break;	
	  	
	  	case 'z': { 	// --size
	  		if (command) delete command;
	    	SizeCmd* sizecmd = new SizeCmd(itsServerPort);
	  		command = sizecmd;
				command->setCmdType(BOARDCMD);
	  	}	break;
	  	
	  	case 'A': { 	// --status
	  		if (command) delete command;
	    	StatusCmd* statuscmd = new StatusCmd(itsServerPort);
	  		command = statuscmd;
				command->setCmdType(BOARDCMD);
	  	}	break;
			
			case 'C': { 	// --clear
				if (command) delete command;
				ClearCmd* clearcmd = new ClearCmd(itsServerPort);
				command = clearcmd;
				command->setCmdType(BOARDCMD);
			}	break;
			
			case 'Z': { 	// --reset
				if (command) delete command;
				ResetCmd* resetcmd = new ResetCmd(itsServerPort);
				command = resetcmd;
				command->setCmdType(BOARDCMD);
			}	break;
		
			case 'D': { 	// --arp
				if (command) delete command;
				ArpCmd* arpcmd = new ArpCmd(itsServerPort);
				command = arpcmd;
				command->setCmdType(BOARDCMD);
			}	break;
			
			case 'E': { 	// --arpmode
				if (command) delete command;
				ArpModeCmd* arpmodecmd = new ArpModeCmd(itsServerPort);
				command = arpmodecmd;
				if (optarg) {
					int32 mode = 0;
					int numitems = sscanf(optarg, "%d", &mode);
					if (numitems == 0 || numitems == EOF || mode < 0 || mode > 1) {
						cout << "Error: invalid mode value. Should be of the format " << endl;
						cout << "       '--arpmode=mode'" << endl;
						cout << "       mode=0 or 1" << endl; 
						exit(EXIT_FAILURE);
					}
					arpmodecmd->setMode((uint32)mode);
				} 
				command->setCmdType(BOARDCMD);
			}	break;
			
			case 'F': { 	// --stopcep
				if (command) delete command;
				StopCepCmd* stopcepcmd = new StopCepCmd(itsServerPort);
				command = stopcepcmd;
				command->setCmdType(BOARDCMD);
			}	break;
			
			case 'G': { 	// --templimits
				if (command) delete command;
				TempLimitCmd* templimitscmd = new TempLimitCmd(itsServerPort);
				command = templimitscmd;
				if (optarg) {
					int32 high = 0;
					int32 low = 0;
					int numitems = sscanf(optarg, "%d,%d", &high, &low);
					if (numitems == 0 || numitems == EOF) {
						cout << "Error: invalid number of values. Should be of the format " << endl;
						cout << "       '--templimits=high,low'" << endl;
						exit(EXIT_FAILURE);
					}
					templimitscmd->setLimitHigh((uint32)high);
					templimitscmd->setLimitLow((uint32)low);
				} 
				command->setCmdType(BOARDCMD);
			}	break;
			
			
			
			case 'S': { 	// --config
				if (command) delete command;
				ConfigCmd* configcmd = new ConfigCmd(itsServerPort);
				command = configcmd;
				
				if (optarg) {
					int32 imagenr = 0;
					int numitems = sscanf(optarg, "%d", &imagenr);
					if (numitems == 0 || numitems == EOF || imagenr < 0 || imagenr >= FL_N_IMAGES) {
						cout << "Error: invalid image value. Should be of the format " << endl;
						cout << "       '--config=imagenr'" << endl;
						cout << "       imagenr=0..15" << endl; 
						exit(EXIT_FAILURE);
					}
					configcmd->setImage((uint32)imagenr);
				} 
				
				command->setCmdType(BOARDCMD);
			}	break;
			
			case 'p': { 	// --readpage
				if (command) delete command;
				ReadPageCmd* readddrcmd = new ReadPageCmd(itsServerPort);
				command = readddrcmd;
				
				if (optarg) {
					int32 rcu = 0;
					uint32 startpage = 0;
					uint32 pages = 0;
					
					int numitems = sscanf(optarg, "%d,%u,%u", &rcu,&startpage,&pages);
					
					if (numitems < 3 || numitems == EOF || rcu < 0 || rcu >= MAX_N_RCUS) {
						cout << "Error: invalid readpage value's. Should be of the format " << endl;
						cout << "       '--readpage=rcu, startpage, pages'" << endl;
						cout << "       rcu=0.." << (MAX_N_RCUS - 1) << endl; 
						exit(EXIT_FAILURE);
					}
					readddrcmd->setStartPage(startpage);
					readddrcmd->setPages(pages);
					select.clear();
		  		select.push_back(rcu);
		  		command->setSelected(true);
				}	
				command->setCmdType(BOARDCMD);
			}	break;			
						
			case '1': { 	// --erasef
				if (command) delete command;
				ErasefCmd* erasefcmd = new ErasefCmd(itsServerPort);
				command = erasefcmd;
				if (optarg) {
					int board = 0;
					int page = 0;
					int numitems = sscanf(optarg, "%d,%d", &board, &page);
					
					if (numitems < 2 || numitems == EOF || page < 0 || page >= FL_N_IMAGES	|| board < 0 || board >= MAX_N_TBBBOARDS) {
						cout << "Error: invalid page value. Should be of the format " << endl;
						cout <<	"       '--eraseimage=board, image'" << endl;
						cout << "       board=0.." << (MAX_N_TBBBOARDS - 1) << ", image=0..15" << endl; 
						exit(EXIT_FAILURE);
					}
					erasefcmd->setPage(page);
					select.clear();
					select.push_back(board);
					command->setSelected(true);
				} 
				command->setCmdType(BOARDCMD);
			}	break;
			
			case '2': { 	// --readf
				if (command) delete command;
				ReadfCmd* readfcmd = new ReadfCmd(itsServerPort);
				command = readfcmd;
				if (optarg) {
					int board = 0;
					int page = 0;
					int numitems = sscanf(optarg, "%d,%d", &board, &page);
					
					if (numitems < 2 || numitems == EOF || page < 0 || page >= FL_N_IMAGES	|| board < 0 || board >= MAX_N_TBBBOARDS) {
						cout << "Error: invalid image value. Should be of the format " << endl;
						cout << "       '--readimage=board, image'"<< endl;
						cout << "       board=0.." << (MAX_N_TBBBOARDS - 1) << ", image=0..31" << endl;  
						exit(EXIT_FAILURE);
					}
					readfcmd->setPage(page);
					select.clear();
					select.push_back(board);
					command->setSelected(true);
				} 
				command->setCmdType(BOARDCMD);
			}	break;
			
			case '3': { 	// --writef
				if (command) delete command;
				WritefCmd* writefcmd = new WritefCmd(itsServerPort);
				command = writefcmd;
				if (optarg) {
					int board = 0;
					int page = 0;
					double version = 0;
					char filename_tp[64];
					char filename_mp[64];
					memset(filename_tp,0,64);
					memset(filename_mp,0,64);
					
					int numitems = sscanf(optarg, "%d,%d,%lf,%63[^,],%63[^,]", &board, &page, &version, filename_tp, filename_mp);
					if (numitems < 5 || numitems == EOF || page < 0 || page >= FL_N_IMAGES	|| board < 0 || board >= MAX_N_TBBBOARDS) {
						cout << "Error: invalid values. Should be of the format " << endl;
						cout << "       '--writeimage=board, image, file-tp, file-mp'"<< endl;
						cout << "       board=0.." << (MAX_N_TBBBOARDS - 1) << ", image=0..31" << endl;  
						exit(EXIT_FAILURE);
					}
					
					writefcmd->setPage(page);
					writefcmd->setVersion(version);
					writefcmd->setFileNameTp(filename_tp);
					writefcmd->setFileNameMp(filename_mp);

					select.clear();
					select.push_back(board);
					command->setSelected(true);
				} 
				command->setCmdType(BOARDCMD);
			}	break;
			
			case '8': { // --imageinfo
				if (command) delete command;
				ImageInfoCmd* imageinfocmd = new ImageInfoCmd(itsServerPort);
				command = imageinfocmd;
				if (optarg) {
					int board = 0;
					int numitems = sscanf(optarg, "%d", &board);
					if (numitems < 1 || numitems == EOF || board < 0 || board >= MAX_N_TBBBOARDS) {
						cout << "Error: invalid values. Should be of the format " << endl;
						cout <<	"       '--imageinfo=board'"<< endl;
						cout << "       board=0.." << (MAX_N_TBBBOARDS - 1) << endl;   
						exit(EXIT_FAILURE);
					}
					select.clear();
					select.push_back(board);
					command->setSelected(true);
				} 
				command->setCmdType(BOARDCMD);
			} break;
			
			case '4': { 	// --readw
				if (command) delete command;
				ReadwCmd* readwcmd = new ReadwCmd(itsServerPort);
				command = readwcmd;
				
				if (optarg) {
					int board = 0;
					uint32 mp = 0;
					uint32 startaddr = 0;
					uint32 size = 0;
					
					int numitems = sscanf(optarg, "%d,%x,%x,%x", &board,&mp,&startaddr, &size);
					
					if (numitems < 3 || numitems == EOF || board < 0 || board >= MAX_N_TBBBOARDS || mp > 3) {
						cout << "Error: invalid read ddr value. Should be of the format " << endl;
						cout <<	"       '--readw=board, mp, addr'" << endl;
						cout << "       board=0.." << (MAX_N_TBBBOARDS - 1) << ",  mp=0..3,  addr=0x.." << endl;  
						exit(EXIT_FAILURE);
					}
					readwcmd->setMp(mp);
					readwcmd->setStartAddr(startaddr);
					readwcmd->setStopAddr(startaddr+size);
					select.clear();
		  		select.push_back(board);
		  		command->setSelected(true);
				} 
				command->setCmdType(BOARDCMD);
			}	break;
			
			case '5': { 	// --writew
				if (command) delete command;
				WritewCmd* writewcmd = new WritewCmd(itsServerPort);
				command = writewcmd;
				if (optarg) {
					int32 board = 0;
					int32 mp = 0;
					uint32 addr = 0;
					uint32 wordlo = 0;
					uint32 wordhi = 0;
					
					int numitems = sscanf(optarg, "%d,%d,%x,%x,%x", &board,&mp,&addr,&wordlo,&wordhi);
					if (numitems < 5 || numitems == EOF || board < 0 || board >= MAX_N_TBBBOARDS || mp > 3) {
						cout << "Error: invalid write ddr value. Should be of the format " << endl;
						cout <<	"       '--writew=board, mp, addr, wordlo, wordhi'"<< endl; 
						cout << "       board=0.." << (MAX_N_TBBBOARDS - 1) << ",  mp=0..3,  addr=0x..,  wordlo=0x..,  wordhi=0x.." << endl;  
						exit(EXIT_FAILURE);
					}
					writewcmd->setMp(mp);
					writewcmd->setAddr(addr);
					writewcmd->setWordLo(wordlo);
					writewcmd->setWordHi(wordhi);
					select.clear();
		  		select.push_back(board);
		  		command->setSelected(true);
				}	
				command->setCmdType(BOARDCMD);
			}	break;
			
			case '9': { 	// --testddr
				if (command) delete command;
				TestDdrCmd* testddrcmd = new TestDdrCmd(itsServerPort);
				command = testddrcmd;
				if (optarg) {
					int32 board = 0;
										
					int numitems = sscanf(optarg, "%d", &board);
					if (numitems < 1 || numitems == EOF || board < 0 || board >= MAX_N_TBBBOARDS) {
						cout << "Error: invalid write ddr value. Should be of the format " << endl;
						cout << "       '--testddr=board' "<< endl;  
						cout << "       board=0.." << (MAX_N_TBBBOARDS - 1) << endl;
						exit(EXIT_FAILURE);
					}
					select.clear();
		  		select.push_back(board);
		  		command->setSelected(true);
				}	
				command->setCmdType(BOARDCMD);
			}	break;
						
			case '6': { 	// --readr
				if (command) delete command;
				ReadrCmd* readrcmd = new ReadrCmd(itsServerPort);
				command = readrcmd;
				
				if (optarg) {
					int32 board = 0;
					uint32 mp = 0;
					uint32 pid = 0;
					uint32 regid = 0;
										
					int numitems = sscanf(optarg, "%d,%u,%u,%u", &board, &mp, &pid, &regid);
					
					if (numitems < 4 || numitems == EOF || board < 0 || board >= MAX_N_TBBBOARDS || mp > 3 || pid > 7 || regid > 7) {
						cout << "Error: invalid read register value. Should be of the format" << endl;
						cout << "       '--readreg=board, mp, pid, regid'" << endl;
						cout << "       board=0.." << (MAX_N_TBBBOARDS - 1) << ",  mp=0..3,  pid=0..7,  regid=0..7" << endl;  
						exit(EXIT_FAILURE);
					}
					if ((REG_TABLE_3[pid][regid] == REG_WRITE_ONLY) || (REG_TABLE_3[pid][regid] == REG_NOT_USED)) {
						cerr << "reading not posible on selected register" << endl;
						exit(EXIT_FAILURE);
					}
					readrcmd->setMp(mp);
					readrcmd->setPid(pid);
					readrcmd->setRegId(regid);
					
					select.clear();
					select.push_back(board);
					command->setSelected(true);
				}	
				
				command->setCmdType(BOARDCMD);
			}	break;
			
			case '7': { 	// --writer
				if (command) delete command;
				WriterCmd* writercmd = new WriterCmd(itsServerPort);
				command = writercmd;
				
				if (optarg) {
					int32 board = 0;
					uint32 mp = 0;
					uint32 pid = 0;
					uint32 regid = 0;
					char datastr[256];
					
					int numitems = sscanf(optarg, "%d,%u,%u,%u,%s", &board, &mp, &pid, &regid, datastr);
					if (numitems < 5 || numitems == EOF || board < 0 || board >= MAX_N_TBBBOARDS || mp > 3 || pid > 7 || regid > 7) {
						cout << "Error: invalid write register value. Should be of the format " << endl;
						cout <<	"       '--writereg=board, mp, pid, regid, data1, data2 etc.'" << endl;
						cout << "       board=0.." << (MAX_N_TBBBOARDS - 1) << ",  mp=0..3,  pid=0..7,  regid=0..7" << endl;   
						exit(EXIT_FAILURE);
					}
					if ((REG_TABLE_3[pid][regid] == REG_READ_ONLY) || (REG_TABLE_3[pid][regid] == REG_NOT_USED)) {
						cerr << "writing not posible on selected register" << endl;
						exit(EXIT_FAILURE);
					}
					writercmd->setMp(mp);
					writercmd->setPid(pid);
					writercmd->setRegId(regid);
					
					char* dstr;
					uint32 val = 0;
					int wordcnt = 0;

  				dstr = strtok (datastr," ,");
  				while (dstr != NULL)
  				{
 						val = strtol(dstr,NULL,16);
   					//cout << formatString("%08x ",val);
    				writercmd->setData(wordcnt,val);
   					wordcnt++;
    				dstr = strtok (NULL, " ,");
  				}
					cout << endl;
					select.clear();
					select.push_back(board);
					command->setSelected(true);
				}	
				
				command->setCmdType(BOARDCMD);
			}	break;
						
			case 'h':
			case '?': {
				commandHelp(1);
				exit(0);
			} break;
			
			case 'X':	{
				commandHelp(3);
				exit(0);
			} break;
			
			default:
			{
				commandHelp(1);
				exit(EXIT_FAILURE);
			} break;
		}
	}

	if (command) {
		if (!command->isSelectionDone()) {	// --select not used, select all
			select.clear();
			for (int i = 0; i < command->getMaxSelections(); i++) {
				select.push_back(i);	
			}
			command->setSelected(true);
		}
	  command->setSelect(select);
	}
  return(command);
}

//-----------------------------------------------------------------------------
std::list<int> TBBCtl::strtolist(const char* str, int max)
{
  string inputstring(str);
  char* start = (char*)inputstring.c_str();
  char* end   = 0;
  bool  range = false;
  long prevval = 0;
	std::list<int> resultset;
			
	resultset.clear();

  while(start) {
    long val = strtol(start, &end, 10); // read decimal numbers
    start = (end ? (*end ? end + 1 : 0) : 0); // determine next start
    if (val >= max || val < 0) {
      cout << formatString("Error: value %ld out of range",val) << endl;
			resultset.clear();
      return(resultset);
    }

    if (end) {
      switch (*end) {
        case ',':
        case 0:
        {
          if (range) {
            if (0 == prevval && 0 == val) {
              val = max - 1;
            }
            if (val < prevval) {
              cout << "Error: invalid range specification" << endl;
							resultset.clear();
							return(resultset);
						}
            for(long i = prevval; i <= val; i++)
              resultset.push_back(i);
          } else {
						resultset.push_back(val);
          }
          range = false;
        } break;

        case ':': {
          range = true;
        } break;

        default: {
          cout << formatString("Error: invalid character %c",*end) << endl;
					resultset.clear();
					return(resultset);
				} break;
      }
    }
    prevval = val;
  }

  return(resultset);
}

//-----------------------------------------------------------------------------
void TBBCtl::mainloop()
{
  start(); // make initial transition
  GCFTask::run();
	
	TBBUnsubscribeEvent unsubscribe;
	itsServerPort.send(unsubscribe);
}

//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  GCFTask::init(argc, argv);
  
  LOG_DEBUG(formatString("Program %s has started", argv[0]));

  TBBCtl tbbctl("tbbctl", argc, argv);

  try {
    tbbctl.mainloop();
  }
  catch (Exception e) {
    cout << "Exception: " << e.text() << endl;
		cout << endl;
		cout << "== abnormal termination of tbbctl ============================================" << endl;
    exit(EXIT_FAILURE);
  }
	cout << endl;
	cout << "== normal termination of tbbctl ==============================================" << endl;

  return(0);
}

