//#  ACDaemon.cc: launches Application Controllers on demand.
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
#include <arpa/inet.h>			// inet_ntoa
#include <Common/LofarLogger.h>
#include <ACC/ACDaemon.h>
#include <ACC/ACRequest.h>

namespace LOFAR {
  namespace ACC {

ACDaemon::ACDaemon(string	aParamFile) :
	itsListener   (0),
	itsPingSocket (0),
	itsParamSet   (new ParameterSet),
	itsACPool     (new ACRequestPool)
{
	// Read in the parameterfile with network parameters.
	itsParamSet->adoptFile(aParamFile);		// May throw

	// Open listener for AC requests.
	itsListener = new Socket("ACdaemon",
							 itsParamSet->getString("ACDaemon.requestportnr"),
							 Socket::TCP);

	// Open listener for ping from AC's.
	itsPingSocket = new Socket("ACdaemon",
							 itsParamSet->getString("ACDaemon.pingportnr"),
							 Socket::UDP);

	// Try to read in an old administration if it is available.
	itsACPool->load(itsParamSet->getString("ACDaemon.adminfile"));

}

ACDaemon::~ACDaemon()
{
	if (itsListener)   { delete itsListener; }
	if (itsPingSocket) { delete itsPingSocket; }
	if (itsParamSet)   { delete itsParamSet; }
	if (itsACPool)     { delete itsACPool; }
}

void ACDaemon::doWork() throw (Exception)
{
	ACRequest 	aRequest;
	uint16		reqSize    = sizeof (ACRequest);
	Socket*		dataSocket = 0;
	string		sysCommand = itsParamSet->getString("ACDaemon.command");

	while (true) {
		// clean up previous mess
		if (dataSocket) {
			sleep (1);
			delete dataSocket;
		}

		// wait for new connection.
		dataSocket = itsListener->accept(-1);
		ASSERTSTR(dataSocket,
				  "Serious problems on listener socket, exiting! : " <<
				  itsListener->errstr());

		// read request
		dataSocket->setBlocking(true);
		uint32 btsRead = dataSocket->read(static_cast<void*>(&aRequest), reqSize);
		if (btsRead != reqSize) {
			LOG_WARN_STR ("ILLEGAL REQUEST SIZE (" << btsRead << 
									" iso " << reqSize << "), IGNORING REQUEST");
			continue;
		}

		// TODO: do some clever assignment of node and portnr

		aRequest.itsRequester[ACREQUESTNAMESIZE-1] = '\0'; // be save
#if defined (__APPLE__)
		aRequest.itsAddr = htonl(0xA9FE3264);
#else
		aRequest.itsAddr = htonl(0xC0A80175);
#endif
		aRequest.itsPort = htons(itsParamSet->getInt("ACDaemon.poolport"));

		uint32 btsWritten = dataSocket->write(static_cast<void*>(&aRequest), 
																	reqSize);
		if (btsWritten != reqSize) {
			LOG_WARN_STR ("REQUEST FOR " << aRequest.itsRequester << 
						  "COULD NOT BE WRITTEN (" << btsWritten << " iso "  <<
						  reqSize << ")");
			continue;
		}
	
		in_addr		IPaddr;
		IPaddr.s_addr = aRequest.itsAddr;

		LOG_DEBUG_STR (aRequest.itsRequester << "gets " << 
				     inet_ntoa(IPaddr) << ", " << aRequest.itsPort);

		constructACFile(&aRequest, "AC.param");

		int32 result = system(sysCommand.c_str());
		LOG_DEBUG_STR ("result=" << result << ", errno=" << errno <<
															strerror(errno));
	}
}

//
// constructACFile(filename)
//
void ACDaemon::constructACFile(const ACRequest*		anACR,
							   const string&		aFilename) {
	ParameterSet		ACPS;

	uint16	backlog = anACR->itsNrProcs / 10;
	backlog = MAX (5, MIN (backlog, 100));
	
	// TODO: Calculate userportnr and processportnr

	ACPS.add(KVpair("AC.backlog",       backlog));
	ACPS.add(KVpair("AC.userportnr",    3820));
	ACPS.add(KVpair("AC.processportnr", 3900));
	ACPS.add("AC.pingportnr", itsParamSet->getString("ACDaemon.pingportnr"));
	
	ACPS.writeFile (aFilename);
}

  } // namespace ACC
} // namespace LOFAR
