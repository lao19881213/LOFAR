//#  AH_FrontEnd.cc:
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//#  $Id$

#include <sys/time.h>

// TransportHolders
#include <Common/lofar_iostream.h>
#include <Common/LofarLogger.h>
#include <Transport/TH_Socket.h>
#include <tinyCEP/SimulatorParseClass.h>

#include <DH_CorrCube.h>
#include <AH_FrontEnd.h>

#define LOCALHOST_IP "127.0.0.1"


using namespace LOFAR;

AH_FrontEnd::AH_FrontEnd (int port, int elements, 
			  int samples, int channels, int polarisations, 
			  int runs, int targets):
  itsPort     (port),
  itsNelements(elements),
  itsNsamples (samples),
  itsNchannels(channels),
  itsNpolarisations(polarisations),
  itsNruns    (runs),
  itsNtargets (targets)
{

  starttime.tv_sec = 0;
  starttime.tv_usec = 0;
  stoptime.tv_sec = 0;
  stoptime.tv_usec = 0;

  bandwidth = 0.0;
}


AH_FrontEnd::~AH_FrontEnd() {

  this->undefine();
}

void AH_FrontEnd::define(const KeyValueMap& /*params*/) {

  undefine();

  WH_Correlator myWHCorrelator("noname",
			       itsNelements, 
			       itsNsamples,
			       itsNchannels, 
			       itsNpolarisations,
			       itsNtargets
			       );

  for (int cn = 0; cn < itsNtargets; cn++) {
    itsWHs.push_back((WorkHolder*) 
		     new WH_Random("noname",
				   itsNelements,
				   itsNsamples,
				   itsNchannels, 
				   itsNpolarisations));
      
    itsWHs.back()->getDataManager().getOutHolder(0)->connectTo
      ( *myWHCorrelator.getDataManager().getInHolder(0),
	TH_Socket(LOCALHOST_IP, LOCALHOST_IP, itsPort+cn, false, true) );
    
  }
}

void AH_FrontEnd::undefine() {
  vector<WorkHolder*>::iterator it = itsWHs.begin();
  for (; it!=itsWHs.end(); it++) {
    delete *it;
  }
  itsWHs.clear();
}

void AH_FrontEnd::init() {
  vector<WorkHolder*>::iterator it = itsWHs.begin();
  for (; it != itsWHs.end(); it++) {
    (*it)->basePreprocess();
  }
}

void AH_FrontEnd::run(int nsteps) {
  vector<WorkHolder*>::iterator it;
  double aggregate_bandwidth=0.0;

  for (int s = 0; s < nsteps; s++) {

//     if (starttime.tv_sec != 0 && starttime.tv_usec !=0) {
//       gettimeofday(&stoptime, NULL);
      
//       bandwidth = (8.0*itsNtargets*itsNchannels*itsNelements*itsNsamples*itsNpolarisations*sizeof(DH_CorrCube::BufferType))/
// 	(stoptime.tv_sec + 1.0e-6*stoptime.tv_usec - starttime.tv_sec + 1.0e-6*starttime.tv_usec);
      
//       cout << bandwidth/(1024.0*1024.0) << " Mbit/sec    " ;
//       cout << (100.0 * bandwidth)/(1024.0*1024.0*1024.0) << "% of theoretical peak (Gbit/sec)" << endl;
//     }

    for (it = itsWHs.begin(); it != itsWHs.end(); it++) {
      (*it)->baseProcess();
      aggregate_bandwidth += reinterpret_cast<WH_Random*>(*it)->getBandwidth();
    }

    if (aggregate_bandwidth != 0.0) {
      cout << (8.0*aggregate_bandwidth)/(1024.0*1024.0) << " Mbit/sec       ";
      cout << (800.0*aggregate_bandwidth)/(1024.0*1024.0*1024.0) << "% of theoretical peak (Gbit/sec)" << endl;
    }
//     gettimeofday(&starttime, NULL);
  }
}


void AH_FrontEnd::dump() const {
//   vector<WorkHolder*>::iterator it = itsWHs.begin();
//   for (; it != itsWHs.end(); it++) {
//     (*it)->dump();
//   }
}

void AH_FrontEnd::quit() {
  this->undefine();
}
