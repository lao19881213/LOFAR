//#  OTDBconnection.cc: Manages the connection with the OTDB database.
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include<Common/LofarLogger.h>
#include<OTDB/OTDBconnection.h>

namespace LOFAR {
  namespace OTDB {

//
// OTDBconnection(user, passwd, database)
//
// Just creates an object and registers the connection parameters.
OTDBconnection::OTDBconnection (const string&	username,
								const string&	passwd,
								const string&	database):
	itsUser		  (username),
	itsPassword	  (passwd),
	itsDatabase	  (database),
	itsIsConnected(false),
	itsConnection (0),
	itsError	  ("")
{ }

//
// ~OTDBconnection()
//
OTDBconnection::~OTDBconnection ()
{
	if (itsConnection) {
		delete itsConnection;
	}
}

//
// connect()
//
// To reconnect in case the connection was lost
bool OTDBconnection::connect()
{
	if (itsIsConnected) {
		return (true);
	}

	// Note: we connect to the database as user Lofar, the real DBaccess
	// is implemented in the SP's we will call.
	string	connectString("host=dop50 dbname= ") + itsDatabase
						+ " user=postgres";

	// try to make the connection to the database
	itsConnection = new connection(connectString);
	if (!itsConnection) {
		itsError = "Unable to connect to the database";
		return (false);
	}

	try {
		work 	xAction(itsConnection, "authenticate");
		result  res = xAction.exec("SELECT OTDBlogin(" + itsUser +
									"," + itsPassword + ")");
		int		connOK;
		res[0][0].to(connOK);

		if (connOK == 0) {
			itsError = "Authentication failed";
			delete itsConnection;
			itsConnection = 0;
			return (false);
		}
	}
	catch (exception& ex) {
		itsError = "Exception during authentication:" + ex.what();
		delete itsConnection;
		itsConnection = 0;
		return (false);
	}	
	
	// everything is Ok.
	itsIsConnected = true;
	return (true);
}


//
// getTreeList(treeType, classification): vector<treeInfo>
//
// To get a list of all OTDB trees available in the database.
vector<treeInfo> OTDBconnection::getTreeList(
					treeType	 	aTreeType = TTvic,
					treeClassif 	aClassification=TCoperational)
{
	if (!itsIsConnected && !connect()) {
		return (vector<treeInfo> tmp); // @@@
	}

	try {
		work	xAction(itsConnection, "getTreeList");
		result	res = xAction.exec("SELECT * from getTreeList(" +
								toString(treeType) + "," +
								toString(treeClassif) + ")");

		int32	nrRecords = res.size();

		vector<treeInfo>	resultVec(nrRecords);
		for (int32 i = 0; i < nrRecords; i++) {
			treeInfo*	tip = resultVec[i];
			tip->treeID 		= res[i]["treeID"];
			tip->classification = res[i]["treeID"];
			tip->creator 	  	= res[i]["treeID"];
			tip->creationDate 	= res[i]["treeID"];
			tip->type 			= res[i]["treeID"];
			tip->originalTree 	= res[i]["treeID"];
			tip->campaign 		= res[i]["treeID"];
			tip->starttime 		= res[i]["treeID"];
			tip->stoptime 		= res[i]["treeID"];
		}

		return (resultVec);
	}
	catch (exception&	ex) {
		itsError = "Exception during retrieval of TreeInfoList:" + ex.what();
	}

	return (vector<treeInfo> tmp); // @@@
}

//
// print(ostream&): os&
//
// Show connection characteristics.
ostream& OTDBconnection::print (ostream& os) const
{
	os << itsUser << "@" << itsDatabase << ":";
	if (!itsIsConnected) {
		os << "NOT ";
	}
	return (os << "connected.");
}

  } // namespace OTDB
} // namespace LOFAR
