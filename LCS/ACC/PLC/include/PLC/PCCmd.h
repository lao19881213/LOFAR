//#  PCCmd.h: Values and names of the PC commands.
//#
//#  Copyright (C) 2007
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
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

#ifndef LOFAR_PLC_PC_CMD_H
#define LOFAR_PLC_PC_CMD_H

// \file
// Values and names of the PLC commands

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>

namespace LOFAR {
  namespace ACC {
    namespace PLC {
// \addtogroup PLC
// @{


// The PCCmd enumeration is a list of command(numbers) that are used to tell 
// the ProcControl server-side (= application process) what command should be
// executed.
enum PCCmd {    PCCmdNone = 0, 
				PCCmdBoot = 100,  PCCmdQuit, 
				PCCmdDefine,      PCCmdInit,
				PCCmdPause,       PCCmdRun,
				PCCmdRelease,
				PCCmdSnapshot,    PCCmdRecover, 
				PCCmdReinit, 	  PCCmdParams,
				PCCmdInfo,        PCCmdAnswer,
				PCCmdReport,
				PCCmdAsync,
				PCCmdResult = 0x1000
};

#define	PAUSE_OPTION_NOW	"now"
#define PAUSE_OPTION_ASAP	"asap"
#define PAUSE_OPTION_TIME	"timestamp="

// Return the name of the given command.
string 	PCCmdName	(PCCmd		PCCmdValue);

// Return whether or not the value represents a result.
inline	bool	isPLCresult	(PCCmd		PCCmdValue)
{
	return (PCCmdValue & PCCmdResult);
}

// Return PCCmd to which the given result belongs.
inline PCCmd	orgPCCmd	(PCCmd		cmdOrResult)
{
	return ((PCCmd)(cmdOrResult & (uint16)~PCCmdResult));
}

// @} addgroup
    } // namespace PLC
  } // namespace ACC
} // namespace LOFAR

#endif
