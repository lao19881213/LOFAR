//#  ProcRuler.h: one line description
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

#ifndef ACC_PROCRULER_H
#define ACC_PROCRULER_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/lofar_map.h>
#include <ACC/ProcRule.h>

namespace LOFAR {
  namespace ACC {

//# Forward Declarations
//class forward;


// Description of class.
class ProcRuler
{
public:
	typedef map<string, ProcRule>::iterator		iterator;

	ProcRuler();
	~ProcRuler();

	void add   (const	ProcRule&	aProcRule);
	void remove(const	string&	    aProcName);

	bool start(const string& aProcName);
	bool stop (const string& aProcName);

	bool startAll();
	bool stopAll();

	void markAsStopped(const string& aProcName);

	void show();	// TEMP
private:
	// Copying is not allowed
	ProcRuler(const ProcRuler&	that);
	ProcRuler& operator=(const ProcRuler& that);

	//# Datamembers
	map<string, ProcRule>		itsProcPool;
};

  } // namespace ACC
} // namespace LOFAR

#endif
