//# ParmTable.h: Object to hold parameters in a table.
//#
//# Copyright (C) 2002
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#if !defined(MNS_PARMTABLE_H)
#define MNS_PARMTABLE_H

//# Includes
#include <PSS3/MNS/MeqPolc.h>
#include <PSS3/MNS/MeqSourceList.h>
#include <Common/lofar_vector.h>

//# Forward Declarations
class MeqDomain;
template<class T> class Vector;


class ParmTableRep
{
public:
  ParmTableRep()
    {}

  virtual ~ParmTableRep()
    {}

  // Get the parameter values for the given parameter and domain.
  // The matchDomain argument is set telling if the found parameter
  // matches the domain exactly.
  // Note that the requested domain may contain multiple polcs.
  virtual vector<MeqPolc> getPolcs (const string& parmName,
				    int sourceNr, int station,
				    const MeqDomain& domain) = 0;

  // Get the initial polynomial coefficients for the given parameter.
  virtual MeqPolc getInitCoeff (const string& parmName,
				int sourceNr, int station) = 0;

  // Put the polynomial coefficient for the given parameter and domain.
  virtual void putCoeff (const string& parmName,
			 int sourceNr, int station,
			 const MeqPolc& polc) = 0;

  // Get the names of all sources in the table.
  virtual vector<string> getSources() = 0;

  // Unlock the underlying table.
  virtual void unlock() = 0;
};



class ParmTable
{
public:
  // Create the ParmTable object.
  // The dbType argument gives the database type.
  // It can be postgres or aips. If aips is given, an AIPS++ table is used,
  // otherwise a database of the given type.
  // For an AIPS++ table, the extension .MEP is added to the table name.
  ParmTable (const string& dbType, const string& tableName,
	     const string& dbName, const string& pwd, const string& hostName = "dop50");

  ~ParmTable()
    { delete itsRep; }

  // Get the parameter values for the given parameter and domain.
  // The matchDomain argument is set telling if the found parameter
  // matches the domain exactly.
  // Note that the requested domain may contain multiple polcs.
  vector<MeqPolc> getPolcs (const string& parmName,
			    int sourceNr, int station,
			    const MeqDomain& domain)
    { return itsRep->getPolcs (parmName, sourceNr, station, domain); }

  // Get the initial polynomial coefficients for the given parameter.
  MeqPolc getInitCoeff (const string& parmName,
			int sourceNr, int station)
    { return itsRep->getInitCoeff (parmName, sourceNr, station); }

  // Put the polynomial coefficient for the given parameter and domain.
  void putCoeff (const string& parmName,
		 int sourceNr, int station,
		 const MeqPolc& polc)
    { itsRep->putCoeff (parmName, sourceNr, station, polc); }

  // Return point sources for the given source numbers.
  // An empty sourceNr vector means all sources.
  // In the 2nd version the pointers to the created MeqParm objects
  // are added to the vector of objects to be deleted.
  // <group>
  MeqSourceList getPointSources (const Vector<int>& sourceNrs);
  MeqSourceList getPointSources (const Vector<int>& sourceNrs,
				 vector<MeqExpr*>& exprDel);
  // </group>

  // Unlock the underlying table.
  void unlock()
    { itsRep->unlock(); }

private:
  // Forbid copy and assignment.
  ParmTable (const ParmTable&);
  ParmTable& operator= (const ParmTable&);

  ParmTableRep* itsRep;
};



#endif
