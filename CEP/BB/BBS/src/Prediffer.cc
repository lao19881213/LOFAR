//# Prediffer.cc: Read and predict read visibilities
//#
//# Copyright (C) 2004
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

#include <lofar_config.h>

#include <BBS/Prediffer.h>
#include <BBS/MMap.h>
#include <BBS/FlagsMap.h>
#include <BBS/MNS/MeqLMN.h>
#include <BBS/MNS/MeqDFTPS.h>
#include <BBS/MNS/MeqDiag.h>
#include <BBS/MNS/MeqBaseDFTPS.h>
#include <BBS/MNS/MeqBaseLinPS.h>
#include <BBS/MNS/MeqStatExpr.h>
#include <BBS/MNS/MeqJonesSum.h>
#include <BBS/MNS/MeqMatrixTmp.h>
#include <BBS/MNS/MeqMatrixComplexArr.h>
#include <BBS/MNS/MeqMatrixRealArr.h>
#include <BBS/MNS/MeqParmFunklet.h>
#include <BBS/MNS/MeqParmSingle.h>
#include <BBS/MNS/MeqPointSource.h>
#include <BBS/MNS/MeqJonesCMul3.h>
#include <BBS/MNS/MeqJonesInvert.h>
#include <BBS/MNS/MeqJonesMMap.h>

#include <MS/MSDesc.h>
#include <Common/Timer.h>
#include <Common/LofarLogger.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobIBufStream.h>
#include <Blob/BlobArray.h>
#include <Common/VectorUtil.h>
#include <Common/DataConvert.h>
#include <Common/LofarLogger.h>

#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Slice.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/Vector.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/Quanta/MVBaseline.h>
#include <casa/Quanta/MVPosition.h>
#include <casa/Utilities/Regex.h>
#include <casa/OS/Timer.h>
#include <casa/OS/RegularFile.h>
#include <casa/OS/SymLink.h>
#include <casa/Containers/Record.h>
#include <casa/IO/AipsIO.h>
#include <casa/IO/MemoryIO.h>
#include <casa/Exceptions/Error.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/TiledColumnStMan.h>

#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <malloc.h>
#include <unistd.h>

#include <BBS/BBSTestLogger.h>

#if defined _OPENMP
#include <omp.h>
#endif

using namespace casa;

#if 0
void *operator new(size_t size)
{
    long *ptr = (long *) malloc(size + 24);

    size += 23, size >>= 3;
    ptr[0] = size;
    ptr[1] = 0x6161F898097771BC;
    ptr[size] = 0x6161F898097771BD;
    return (void *) (ptr + 2);
}

void operator delete(void *addr)
{
    if (addr != 0) {
	long *ptr = (long *) addr - 2;
	if (ptr[1] != 0x6161F898097771BC || ptr[ptr[0]] != 0x6161F898097771BD) {
	    std::cerr << ptr << ": " << ptr[0] << ' ' << ptr[1] << ' ' << ptr[ptr[0]] << '\n';
	    for (;;);
	}
	long value = 0x7777000000000000 + (long) __builtin_return_address(0);
	for (size_t size = ptr[0], i = 0; i <= size; i ++)
	    ptr[i] = value;
	free(ptr);
    }
}
#endif

namespace LOFAR
{

//----------------------------------------------------------------------
//
// Constructor. Initialize a Prediffer object.
//
// Create list of stations and list of baselines from MS.
// Create the MeqExpr tree for the given model type.
//
//----------------------------------------------------------------------
Prediffer::Prediffer(const string& msName,
		     const ParmDB::ParmDBMeta& meqPdm,
		     const ParmDB::ParmDBMeta& skyPdm,
		     const vector<int>& ant,
		     const string& modelType,
		     const vector<vector<int> >& sourceGroups,
		     bool      calcUVW)
  :
  itsMSName       (msName),
  itsMEPName      (meqPdm.getTableName()),
  itsMEP          (meqPdm),
  itsGSMMEPName   (skyPdm.getTableName()),
  itsGSMMEP       (skyPdm),
  itsCalcUVW      (calcUVW),
  itsSources      (0),
  itsSrcGrp       (sourceGroups),
  itsNrPert       (0),
  itsNCorr        (0),
  itsNrBl         (0),
  itsTimeIndex    (0),
  itsNrTimes      (0),
  itsNrTimesDone  (0),
  itsInDataMap    (0),
  itsOutDataMap   (0),
  itsFlagsMap     (0),
  itsWeightMap    (0),
  itsIsWeightSpec (false),
  itsPredTimer    ("P:predict", false),
  itsEqTimer      ("P:eq|save", false)
{
  LOG_INFO_STR( "Prediffer constructor ("
		<< "'" << msName   << "', "
		<< "'" << meqPdm.getTableName() << "', "
		<< "'" << skyPdm.getTableName() << "', "
		<< itsCalcUVW << ")" );

  // Initially use all correlations.
  for (int i=0; i<4; ++i) {
    itsCorr[i] = true;
  }
  // Read the meta data and map files.
  readDescriptiveData (msName);
  fillStations (ant);                  // Selected antennas
  fillBaselines (ant);
  itsFlagsMap = new FlagsMap(msName + "/vis.flg", MMap::Read);

  // Set the MS info.
  itsMSMapInfo = MMapMSInfo (itsNCorr, itsNrChan, itsNrBl,
			     itsReverseChan);
  // Get all sources from the GSM and check the source groups.
  getSources();

  // Set up the expression tree for all baselines.
  bool asAP = true;
  bool useStatParm = false;
  bool usePEJ = false;
  bool useTEJ = false;
  bool useBandpass = false;
  vector<string> types = StringUtil::split(modelType, '.');
  for (uint i=0; i<types.size(); ++i) {
    if (types[i] != "") {
      if (types[i] == "TOTALEJ") {
	useTEJ = true;
      } else if (types[i] == "PATCHEJ") {
	usePEJ = true;
      } else if (types[i] == "REALIMAG") {
	asAP = false;
      } else if (types[i] == "DIPOLE") {
	useStatParm = true;
      } else if (types[i] == "BANDPASS") {
	useBandpass = true;
      } else {
	ASSERTSTR (false, "Modeltype part " << types[i] << " is invalid; "
		   "valid are TOTALEJ,PATCHEJ,REALIMAG,DIPOLE,BANDPASS");
      }
    }
  }
  makeLOFARExpr(useTEJ, usePEJ, asAP, useStatParm, useBandpass);

  // Show frequency domain.
  LOG_INFO_STR( "Freq: " << itsStartFreq << ' ' << itsEndFreq << " (" <<
    itsEndFreq - itsStartFreq << " Hz) " << itsNrChan << " channels of "
	    << itsStepFreq << " Hz" );

  if (!itsCalcUVW) {
    // Fill the UVW coordinates from the MS instead of calculating them.
    fillUVW();
 }

  // Allocate thread private buffers.
#if defined _OPENMP
  itsNthread = omp_get_max_threads();
#else
  itsNthread = 1;
#endif
  itsFlagVecs.resize (itsNthread);
  itsResultVecs.resize (itsNthread);
  itsDiffVecs.resize (itsNthread);
  itsIndexVecs.resize (itsNthread);
  ///  itsOrdFlagVecs.resize (itsNthread);
}

//----------------------------------------------------------------------
//
// ~~Prediffer
//
// Destructor for a Prediffer object.
//
//----------------------------------------------------------------------
Prediffer::~Prediffer()
{
  LOG_TRACE_FLOW( "Prediffer destructor" );

  delete itsSources;
  for (vector<MeqStatUVW*>::iterator iter = itsStatUVW.begin();
       iter != itsStatUVW.end();
       iter++) {
    delete *iter;
  }
  for (vector<MeqStation*>::iterator iter = itsStations.begin();
       iter != itsStations.end();
       iter++) {
    delete *iter;
  }

  delete itsInDataMap;
  delete itsOutDataMap;
  delete itsFlagsMap;
  delete itsWeightMap;

  // clear up the matrix pool
  MeqMatrixComplexArr::poolDeactivate();
  MeqMatrixRealArr::poolDeactivate();
}

//-------------------------------------------------------------------
//
// ~readDescriptiveData
//
// Get measurement set description from file
//
//-------------------------------------------------------------------
void Prediffer::readDescriptiveData(const string& fileName)
{
  // Get meta data from description file.
  string name(fileName+"/vis.des");
  std::ifstream istr(name.c_str());
  ASSERTSTR (istr, "File " << fileName << "/vis.des could not be opened");
  BlobIBufStream bbs(istr);
  BlobIStream bis(bbs);
  MSDesc msd;
  bis >> msd;
  ASSERTSTR (msd.nchan.size() == 1, "Multiple bands in MS cannot be handled");
  itsNCorr     = msd.corrTypes.size();
  itsNrChan    = msd.nchan[0];
  itsStartFreq = msd.startFreq[0];
  itsEndFreq   = msd.endFreq[0];
  itsStepFreq  = (itsEndFreq - itsStartFreq)/itsNrChan;
  itsAnt1      = msd.ant1;
  itsAnt2      = msd.ant2;
  itsTimes     = msd.times;
  itsIntervals = msd.exposures;
  itsAntPos    = msd.antPos;
  ASSERT (itsAnt1.size() == itsAnt2.size());
  itsNrBl = itsAnt1.size();
  itsReverseChan = itsStartFreq > itsEndFreq;
  if (itsReverseChan) {
    double  tmp  = itsEndFreq;
    itsEndFreq   = itsStartFreq;
    itsStartFreq = tmp;
    itsStepFreq  = std::abs(itsStepFreq);
  }
  getPhaseRef(msd.ra, msd.dec, msd.startTime);
}

//----------------------------------------------------------------------
//                                                                       
// ~getPhaseRef
//
// Get the phase reference of the first field.
//
//----------------------------------------------------------------------
void Prediffer::getPhaseRef(double ra, double dec, double startTime)
{
  // Use the phase reference of the given J2000 ra/dec.
  MVDirection mvdir(ra, dec);
  MDirection phaseRef(mvdir, MDirection::J2000);
  itsPhaseRef = MeqPhaseRef (phaseRef, startTime);
}


//----------------------------------------------------------------------
//
// ~fillStations
//
// Fill the station positions and names.
//
//----------------------------------------------------------------------
void Prediffer::fillStations (const vector<int>& antnrs)
{
  uint nrant = itsAntPos.ncolumn();
  itsStations = vector<MeqStation*>(nrant, (MeqStation*)0);
  // Get all stations actually used.
  char str[8];
  for (uint i=0; i<antnrs.size(); i++) {
    uint ant = antnrs[i];
    ASSERT (ant < nrant);
    if (itsStations[ant] == 0) {
      // Store each position as a constant parameter.
      // Use the antenna name as the parameter name.
      Vector<double> antpos = itsAntPos.column(ant);
      sprintf (str, "%d", ant+1);
      String name = string("SR") + str;
      MeqParmSingle* px = new MeqParmSingle("AntPosX." + name,
					    &itsParmGroup, antpos(0));
      MeqParmSingle* py = new MeqParmSingle("AntPosY." + name,
					    &itsParmGroup, antpos(1));
      MeqParmSingle* pz = new MeqParmSingle("AntPosZ." + name,
					    &itsParmGroup, antpos(2));
      itsStations[ant] = new MeqStation(px, py, pz, name);
    }
  }
}

//----------------------------------------------------------------------
//
// ~fillBaselines
//
// Create objects representing the baseline directions.
// Create an index giving the baseline index of an ordered antenna pair.
//
//----------------------------------------------------------------------
void Prediffer::fillBaselines (const vector<int>& antnrs)
{
  // Convert antnrs to bools.
  uint maxAnt = itsStations.size();
  Block<bool> useAnt(maxAnt, false);
  for (uint i=0; i<antnrs.size(); i++) {
    uint ant = antnrs[i];
    ASSERT (ant < maxAnt);
    useAnt[ant] = true;
  }
  itsNrUsedBl = 0;
  itsBLIndex.resize (maxAnt, maxAnt);
  itsBLIndex = -1;
  itsBLSelection.resize (maxAnt, maxAnt);
  itsBLSelection = false;
  itsBLUsedInx.reserve (itsNrBl);
  
  for (uint i=0; i<itsNrBl; i++) {
    uint a1 = itsAnt1[i];
    uint a2 = itsAnt2[i];
    ASSERT (a1 < maxAnt  &&  a2 < maxAnt);
    if (useAnt[a1] && useAnt[a2]) {
      // Assign a baseline index and set to selected.
      itsBLSelection(a1,a2) = true;
      itsBLIndex(a1,a2) = itsNrUsedBl++;
      itsBLUsedInx.push_back (i);
    }
  }
  countBaseCorr();
}

//----------------------------------------------------------------------
//
// ~countBaseCorr
//
// Count the selected nr of baselines and correlations
//
//----------------------------------------------------------------------
void Prediffer::countBaseCorr()
{
  itsBLSelInx.resize  (0);
  itsBLSelInx.reserve (itsNrBl);
  const bool* sel = itsBLSelection.data();
  const int*  inx = itsBLIndex.data();
  for (uint i=0; i<itsBLSelection.nelements(); ++i) {
    if (sel[i]  &&  inx[i] >= 0) {
      itsBLSelInx.push_back(inx[i]);
    }
  }
  ASSERTSTR (itsBLSelInx.size() > 0, "No valid baselines selected");
  int nSelCorr = 0;
  for (int i=0; i<itsNCorr; ++i) {
    if (itsCorr[i]) {
      nSelCorr++;
    }
  }
  ASSERTSTR (nSelCorr > 0, "No valid correlations selected");
}

//----------------------------------------------------------------------
//
// ~getSources
//
// Get all sources from the parmtable.
//
//----------------------------------------------------------------------
void Prediffer::getSources()
{
  // Get the sources from the ParmTable
  itsSources = new MeqSourceList(itsGSMMEP, &itsParmGroup);
  int nrsrc = itsSources->size();
  for (int i=0; i<nrsrc; ++i) {
    (*itsSources)[i].setSourceNr (i);
  }
  // Make a map for the sources actually used.
  itsSrcNrMap.reserve (nrsrc);
  if (itsSrcGrp.size() == 0) {
    // Set groups if nothing given.
    itsSrcGrp.resize (nrsrc);
    vector<int> vec(1);
    for (int i=0; i<nrsrc; ++i) {
      vec[0] = i+1;                 // source nrs are 1-relative
      itsSrcGrp[i] = vec;
      itsSrcNrMap.push_back (i);
    }
  } else {
    for (uint j=0; j<itsSrcGrp.size(); ++j) {
      vector<int> srcs = itsSrcGrp[j];
      ASSERTSTR (srcs.size() > 0, "Sourcegroup " << j << " is empty");
      for (uint i=0; i<srcs.size(); ++i) {
	ASSERTSTR (srcs[i] > 0  &&  srcs[i] <= nrsrc,
		   "Sourcenr " << srcs[i]
		   << " must be > 0 and <= #sources (=" << nrsrc << ')');
	(*itsSources)[srcs[i]-1].setSourceNr (itsSrcNrMap.size());
	itsSrcNrMap.push_back (srcs[i]-1);
      }
    }
  }
  // Make an LMN node for each source used.
  int nrused = itsSrcNrMap.size();
  itsLMN.reserve (nrused);
  for (int i=0; i<nrused; ++i) {
    int src = itsSrcNrMap[i];
    MeqLMN* lmn = new MeqLMN(&((*itsSources)[src]));
    lmn->setPhaseRef (&itsPhaseRef);
    itsLMN.push_back (lmn);
  }
  // Set the peel source nrs initially to the sources.
  vector<int> peelNrs;
  for (uint i=0; i<itsSrcGrp.size(); ++i) {
    const vector<int>& grp = itsSrcGrp[i];
    for (uint j=0; j<grp.size(); ++j) {
      peelNrs.push_back (grp[j] - 1);
    }
  }
  itsPeelSourceNrs = peelNrs;
}

//----------------------------------------------------------------------
//
// ~makeLOFARExpr
//
// Make the expression tree per baseline for LOFAR.
//
//----------------------------------------------------------------------
void Prediffer::makeLOFARExpr (bool useTEJ, bool usePEJ, bool asAP,
			       bool useStatParm, bool useBandpass)
{
  // Allocate the vectors holding the expressions.
  int nrstat = itsStations.size();
  int nrsrc  = itsSrcNrMap.size();
  int nrgrp  = itsSrcGrp.size();
  itsStatUVW.reserve (nrstat);
  // EJ is real/imag or ampl/phase
  string ejname1 = "real:";
  string ejname2 = "imag:";
  if (asAP) {
    ejname1 = "ampl:";
    ejname2 = "phase:";
  }
  // Vector containing StatExpr-s.
  vector<MeqJonesExpr> statExpr(nrstat);
  // Vector containing DFTPS-s.
  vector<MeqExpr> pdfts(nrsrc*nrstat);
  // Vector containing all EJ-s per station per patch.
  vector<MeqJonesExpr> patchEJ(nrgrp*nrstat);
  // Vector containing all EJ-s per station.
  vector<MeqJonesExpr> totalEJ(nrstat);
  // Correction per station.
  vector<MeqJonesExpr> corrStat(nrstat);
  // Bandpass per station.
  vector<MeqJonesExpr> bandpass(nrstat);

  // Fill the vectors for each station.
  for (int i=0; i<nrstat; ++i) {
    MeqStatUVW* uvw = 0;
    // Do it only if the station is actually used.
    if (itsStations[i] != 0) {
      // Expression to calculate UVW per station
      uvw = new MeqStatUVW (itsStations[i], &itsPhaseRef);
      // Do pure station parameters only if told so.
      if (useStatParm) {
	MeqExpr frot (MeqParmFunklet::create ("frot:" +
					      itsStations[i]->getName(),
					      &itsParmGroup, &itsMEP));
	MeqExpr drot (MeqParmFunklet::create ("drot:" +
					      itsStations[i]->getName(),
					      &itsParmGroup, &itsMEP));
	MeqExpr dell (MeqParmFunklet::create ("dell:" +
					      itsStations[i]->getName(),
					      &itsParmGroup, &itsMEP));
	MeqExpr gain11 (MeqParmFunklet::create ("gain:11:" +
						itsStations[i]->getName(),
						&itsParmGroup, &itsMEP));
	MeqExpr gain22 (MeqParmFunklet::create ("gain:22:" +
						itsStations[i]->getName(),
						&itsParmGroup, &itsMEP));
	statExpr[i] = MeqJonesExpr(new MeqStatExpr (frot, drot, dell,
						    gain11, gain22));
      }
      // Make a bandpass per station
      if (useBandpass) {
        string stationName = itsStations[i]->getName();
        
        MeqExpr bandpassXX(MeqParmFunklet::create("Bandpass:XX:" + stationName,
						  &itsParmGroup, &itsMEP));
        MeqExpr bandpassYY(MeqParmFunklet::create("Bandpass:YY:" + stationName,
						  &itsParmGroup, &itsMEP));
        
        bandpass[i] = new MeqDiag(MeqExpr(bandpassXX), MeqExpr(bandpassYY));
      }
      // Make a DFT per station per source.
      for (int src=0; src<nrsrc; ++src) {
	pdfts[i*nrsrc + src] = MeqExpr(new MeqDFTPS (itsLMN[src], uvw));
      }
      // Make optionally EJones expressions.
      MeqExprRep* ej11;
      MeqExprRep* ej12;
      MeqExprRep* ej21;
      MeqExprRep* ej22;
      if (useTEJ) {
	// Make a gain/phase expression per station.
	string nm = itsStations[i]->getName();
	MeqExpr ej11r (MeqParmFunklet::create ("EJ11:" + ejname1 + nm,
					       &itsParmGroup, &itsMEP));
	MeqExpr ej11i (MeqParmFunklet::create ("EJ11:" + ejname2 + nm,
					       &itsParmGroup, &itsMEP));
	MeqExpr ej12r (MeqParmFunklet::create ("EJ12:" + ejname1 + nm,
					       &itsParmGroup, &itsMEP));
	MeqExpr ej12i (MeqParmFunklet::create ("EJ12:" + ejname2 + nm,
					       &itsParmGroup, &itsMEP));
	MeqExpr ej21r (MeqParmFunklet::create ("EJ21:" + ejname1 + nm,
					       &itsParmGroup, &itsMEP));
	MeqExpr ej21i (MeqParmFunklet::create ("EJ21:" + ejname2 + nm,
					       &itsParmGroup, &itsMEP));
	MeqExpr ej22r (MeqParmFunklet::create ("EJ22:" + ejname1 + nm,
					       &itsParmGroup, &itsMEP));
	MeqExpr ej22i (MeqParmFunklet::create ("EJ22:" + ejname2 + nm,
					       &itsParmGroup, &itsMEP));
	if (asAP) {
	  ej11 = new MeqExprAPToComplex (ej11r, ej11i);
	  ej12 = new MeqExprAPToComplex (ej12r, ej12i);
	  ej21 = new MeqExprAPToComplex (ej21r, ej21i);
	  ej22 = new MeqExprAPToComplex (ej22r, ej22i);
	} else {
	  ej11 = new MeqExprToComplex (ej11r, ej11i);
	  ej12 = new MeqExprToComplex (ej12r, ej12i);
	  ej21 = new MeqExprToComplex (ej21r, ej21i);
	  ej22 = new MeqExprToComplex (ej22r, ej22i);
	}
	totalEJ[i] = new MeqJonesNode (MeqExpr(ej11), MeqExpr(ej12),
				       MeqExpr(ej21), MeqExpr(ej22));
	corrStat[i] = new MeqJonesInvert (totalEJ[i]);
      }
      if (usePEJ) {
	// Make a complex gain expression per station per patch.
	for (int j=0; j<nrgrp; j++) {
	  ostringstream ostr;
	  ostr << j+1;
	  string nm = itsStations[i]->getName() + ":SG" + ostr.str();
	  MeqExpr ej11r (MeqParmFunklet::create ("EJ11:" + ejname1 + nm,
						 &itsParmGroup, &itsMEP));
	  MeqExpr ej11i (MeqParmFunklet::create ("EJ11:" + ejname2 + nm,
						 &itsParmGroup, &itsMEP));
	  MeqExpr ej12r (MeqParmFunklet::create ("EJ12:" + ejname1 + nm,
						 &itsParmGroup, &itsMEP));
	  MeqExpr ej12i (MeqParmFunklet::create ("EJ12:" + ejname2 + nm,
						 &itsParmGroup, &itsMEP));
	  MeqExpr ej21r (MeqParmFunklet::create ("EJ21:" + ejname1 + nm,
						 &itsParmGroup, &itsMEP));
	  MeqExpr ej21i (MeqParmFunklet::create ("EJ21:" + ejname2 + nm,
						 &itsParmGroup, &itsMEP));
	  MeqExpr ej22r (MeqParmFunklet::create ("EJ22:" + ejname1 + nm,
						 &itsParmGroup, &itsMEP));
	  MeqExpr ej22i (MeqParmFunklet::create ("EJ22:" + ejname2 + nm,
						 &itsParmGroup, &itsMEP));
	  if (asAP) {
	    ej11 = new MeqExprAPToComplex (ej11r, ej11i);
	    ej12 = new MeqExprAPToComplex (ej12r, ej12i);
	    ej21 = new MeqExprAPToComplex (ej21r, ej21i);
	    ej22 = new MeqExprAPToComplex (ej22r, ej22i);
	  } else {
	    ej11 = new MeqExprToComplex (ej11r, ej11i);
	    ej12 = new MeqExprToComplex (ej12r, ej12i);
	    ej21 = new MeqExprToComplex (ej21r, ej21i);
	    ej22 = new MeqExprToComplex (ej22r, ej22i);
	  }
	  patchEJ[i*nrgrp + j] = new MeqJonesNode (MeqExpr(ej11),
						   MeqExpr(ej12),
						   MeqExpr(ej21),
						   MeqExpr(ej22));
	  // Only AP of first group is used for correction.
	  if (j == 0) {
	    corrStat[i] = new MeqJonesInvert (patchEJ[i*nrgrp + j]);
	  }
	}
      }
    }
    itsStatUVW.push_back (uvw);
  }    

  // Make an expression for each baseline.
  itsExpr.resize (itsNrUsedBl);
  itsCorrExpr.resize (itsNrUsedBl);
  itsCorrMMap.resize (itsNrUsedBl);
  int nrant = itsBLIndex.nrow();
  for (int ant2=0; ant2<nrant; ant2++) {
    for (int ant1=0; ant1<nrant; ant1++) {
      int blindex = itsBLIndex(ant1,ant2);
      if (blindex >= 0) {
	if (usePEJ || useTEJ) {
	  // Make correction expressions.
	  itsCorrMMap[blindex] = new MeqJonesMMap (itsMSMapInfo,
						   itsBLUsedInx[blindex]);
	  itsCorrExpr[blindex] = new MeqJonesCMul3 (corrStat[ant1],
						    itsCorrMMap[blindex],
						    corrStat[ant2]);
	}
	// Predict expressions.
	vector<MeqJonesExpr> vecPatch;
	// Loop through all source groups.
	for (int grp=0; grp<nrgrp; ++grp) {
	  const vector<int>& srcgrp = itsSrcGrp[grp];
	  vector<MeqJonesExpr> vecSrc;
	  vecSrc.reserve (srcgrp.size());
	  for (uint j=0; j<srcgrp.size(); ++j) {
	    // Create the total DFT per source.
	    int src = srcgrp[j] - 1;
	    MeqExpr expr1 (new MeqBaseDFTPS (pdfts[ant1*nrsrc + src],
					     pdfts[ant2*nrsrc + src],
					     itsLMN[src]));
	    // For the time being only point sources are supported.
	    MeqPointSource& mps = dynamic_cast<MeqPointSource&>
					    ((*itsSources)[itsSrcNrMap[src]]);
	    vecSrc.push_back (MeqJonesExpr (new MeqBaseLinPS(expr1, &mps)));
	  }
	  MeqJonesExpr sum;
	  // Sum all sources in the group.
	  if (vecSrc.size() == 1) {
	    sum = vecSrc[0];
	  } else {
	    sum = MeqJonesExpr (new MeqJonesSum(vecSrc));
	  }
	  // Multiply by ionospheric gain/phase per station per patch.
	  if (usePEJ) {
	    vecPatch.push_back (new MeqJonesCMul3(patchEJ[ant1*nrgrp + grp],
						  sum,
						  patchEJ[ant2*nrgrp + grp]));
	  } else {
	    vecPatch.push_back (sum);
	  }
	}
	// Sum all patches.
	MeqJonesExpr sumAll;
	if (vecPatch.size() == 1) {
	  sumAll = vecPatch[0];
	} else {
	  sumAll = MeqJonesExpr (new MeqJonesSum(vecPatch));
	}
	// Multiply by total gain/phase per station.
	if (useTEJ) {
	  sumAll = new MeqJonesCMul3(totalEJ[ant1],
				     sumAll,
				     totalEJ[ant2]);
	}
        if (useBandpass) {
          sumAll = new MeqJonesCMul3(bandpass[ant1],
                                     sumAll,
                                     bandpass[ant2]);
        }
	if (useStatParm) {
	  sumAll = new MeqJonesCMul3(statExpr[ant1],
				     sumAll,
				     statExpr[ant2]);
	}
	itsExpr[blindex] = sumAll;
      }
    }
  }
}

void Prediffer::setPrecalcNodes (vector<MeqJonesExpr>& nodes)
{
  // First clear the levels of all nodes in the tree.
  for (uint i=0; i<nodes.size(); ++i) {
    if (! nodes[i].isNull()) {
      nodes[i].clearDone();
    }
  }
  // Now set the levels of all nodes in the tree.
  // The top nodes have level 0; lower nodes have 1, 2, etc..
  int nrLev = -1;
  for (uint i=0; i<nodes.size(); ++i) {
    if (! nodes[i].isNull()) {
      nrLev = std::max (nrLev, nodes[i].setLevel(0));
    }
  }
  nrLev++;
  ASSERT (nrLev > 0);
  itsPrecalcNodes.resize (nrLev);
  // Find the nodes to be precalculated at each level.
  // That is not needed for the root nodes (the baselines).
  // The nodes used by the baselines are always precalculated (even if
  // having one parent).
  // It may happen that a station is used by only one baseline. Calculating
  // such a baseline is much more work if the station was not precalculated.
  for (int level=1; level<nrLev; ++level) {
    std::vector<MeqExprRep*>& pcnodes = itsPrecalcNodes[level];
    pcnodes.resize (0);
    for (std::vector<MeqJonesExpr>::iterator iter=nodes.begin();
	 iter != nodes.end();
	 ++iter) {
      if (! iter->isNull()) {
	iter->getCachingNodes (pcnodes, level, false);
      }
    }
  }
  LOG_TRACE_FLOW_STR("#levels=" << nrLev);
  for (int i=0; i<nrLev; ++i) {
    LOG_TRACE_FLOW_STR("#expr on level " << i << " is " << itsPrecalcNodes[i].size());
  }
}


void Prediffer::readParms()
{
  // Read all parms for this domain into a single map.
  map<string,ParmDB::ParmValueSet> parmValues;
  vector<string> emptyvec;
  ParmDB::ParmDomain pdomain(itsWorkDomain.startX(), itsWorkDomain.endX(),
			     itsWorkDomain.startY(), itsWorkDomain.endY());
  itsMEP.getValues (parmValues, emptyvec, pdomain);
  itsGSMMEP.getValues (parmValues, emptyvec, pdomain);

  const vector<MeqParm*>& parmList = itsParmGroup.getParms();
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       ++iter)
  {
    if (*iter) {
      (*iter)->fillFunklets (parmValues, itsWorkDomain);
    }
  }
}

void Prediffer::initSolvableParms (const vector<MeqDomain>& solveDomains)
{
  itsParmData.set (solveDomains, itsWorkDomain);
  const vector<MeqDomain>& localSolveDomains = itsParmData.getDomains();
  itsNrPert = 0;
  itsNrScids.resize (localSolveDomains.size());
  std::fill (itsNrScids.begin(), itsNrScids.end(), 0);
  const vector<MeqParm*>& parmList = itsParmGroup.getParms();
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       ++iter)
  {
    if (*iter) {
      int nr = (*iter)->initDomain (localSolveDomains, itsNrPert, itsNrScids);
      if (nr > 0) {
	itsParmData.parms().push_back (ParmData((*iter)->getName(),
						(*iter)->getParmDBSeqNr(),
						(*iter)->getFunklets()));
      }
    }
  }
  // Determine the fitter indices for the frequency and time axis.
  // First check if the (local) solve domains are ordered and regular.
  // Start with finding the number of frequency intervals.
  for (itsFreqNrFit=1; itsFreqNrFit<localSolveDomains.size(); itsFreqNrFit++) {
    if (localSolveDomains[itsFreqNrFit].startX() < 
	localSolveDomains[itsFreqNrFit-1].endX()) {
      break;
    }
  }
  // Now check if regular in frequency and time.
  uint timeNrFit = localSolveDomains.size() / itsFreqNrFit;
  ASSERT (timeNrFit*itsFreqNrFit == localSolveDomains.size());
  for (uint inxt=0; inxt<timeNrFit; inxt++) {
    const MeqDomain& first = localSolveDomains[inxt*itsFreqNrFit];
    for (uint inxf=1; inxf<itsFreqNrFit; inxf++) {
      const MeqDomain& cur = localSolveDomains[inxt*itsFreqNrFit + inxf];
      ASSERT (cur.startY() == first.startY()  &&  cur.endY() == first.endY());
    }
  }
  for (uint inxf=0; inxf<itsFreqNrFit; inxf++) {
    const MeqDomain& first = localSolveDomains[inxf];
    for (uint inxt=1; inxt<timeNrFit; inxt++) {
      const MeqDomain& cur = localSolveDomains[inxt*itsFreqNrFit + inxf];
      ASSERT (cur.startX() == first.startX()  &&  cur.endX() == first.endX());
    }
  }
  // Determine for each frequency point to which fitter it belongs.
  // For the time axis it is determined by nextDataChunk.
  int nrchan = itsLastChan-itsFirstChan+1;
  itsFreqFitInx.resize (nrchan);
  double step = itsWorkDomain.sizeX() / nrchan;
  double freq = itsWorkDomain.startX() + step / 2;
  int interv = 0;
  for (int i=0; i<nrchan; i++) {
    if (freq > localSolveDomains[interv].endX()) {
      interv++;
    }
    itsFreqFitInx[i] = interv;
    freq += step;
  }
}

void Prediffer::setWorkDomain (int startChan, int endChan,
			       double tstart, double tlength)
{
  // Determine the first and last channel to process.
  if (startChan < 0) {
    itsFirstChan = 0;
  } else {
    itsFirstChan = startChan;
  }
  if (endChan < 0  ||  endChan >= itsNrChan) {
    itsLastChan = itsNrChan-1;
  } else {
    itsLastChan = endChan;
  }
  ASSERT (itsFirstChan <= itsLastChan);

  // Find the times matching the given time interval.
  // Normally the times are in sequential order, so we can continue searching.
  // Otherwise start the search at the start.
  itsTimeIndex += itsNrTimes;
  if (itsTimeIndex >= itsTimes.nelements()
  ||  tstart < itsTimes[itsTimeIndex]) {
    itsTimeIndex = 0;
  }
  // Find the time matching the start time.
  while (itsTimeIndex < itsTimes.nelements()
	 &&  tstart > itsTimes[itsTimeIndex]) {
    ++itsTimeIndex;
  }
  // Exit when no more chunks.
  if (itsTimeIndex >= itsTimes.nelements()) {
    return;
  }
  
  BBSTest::Logger::log("BeginOfInterval");

  // Find the end of the interval.
  uint endIndex = itsTimeIndex;
  double startTime = itsTimes[itsTimeIndex] - itsIntervals[itsTimeIndex]/2;
  double endTime = tstart + tlength;
  itsNrTimes     = 0;
  while (endIndex < itsTimes.nelements()
	 && endTime >= itsTimes[endIndex]) {
    ///cout << "time find " << itsTimes[endIndex]-itsTimes[0] << ' ' << endIndex<<endl;
    ++endIndex;
    ++itsNrTimes;
  }
  ASSERT (itsNrTimes > 0);
  endTime = itsTimes[endIndex-1] + itsIntervals[endIndex-1]/2;
  ///cout << "time-indices " << itsTimeIndex << ' ' << endIndex << ' ' << itsNrTimes << ' ' << itsTimes[endIndex]-itsTimes[0]<<' '<<endTime-tstart<<' '<<startTime-itsTimes[0]<< ' '<<endTime-itsTimes[0]<<endl;
  
  BBSTest::ScopedTimer parmTimer("P:readparms");
  //  parmTimer.start();
  itsWorkDomain = MeqDomain(itsStartFreq + itsFirstChan*itsStepFreq,
			    itsStartFreq + (itsLastChan+1)*itsStepFreq,
			    startTime,
			    endTime);
  // Read all parameter values which are part of the work domain.
  readParms();
  //  parmTimer.stop();
}


//----------------------------------------------------------------------
//
// ~clearSolvableParms
//
// Clear the solvable flag on all parms (make them non-solvable).
//
//----------------------------------------------------------------------
void Prediffer::clearSolvableParms()
{
  LOG_TRACE_FLOW( "clearSolvableParms" );
  const vector<MeqParm*>& parmList = itsParmGroup.getParms();
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if (*iter) {
      (*iter)->setSolvable(false);
    }
  }
}

//----------------------------------------------------------------------
//
// ~setSolvableParms
//
// Set the solvable flag (true or false) on all parameters whose
// name matches the parmPatterns pattern.
//
//----------------------------------------------------------------------
void Prediffer::setSolvableParms (const vector<string>& parms,
				  const vector<string>& excludePatterns)
{
  LOG_INFO_STR( "setSolvableParms");
  const vector<MeqParm*>& parmList = itsParmGroup.getParms();
  // Convert patterns to regexes.
  vector<Regex> parmRegex;
  for (unsigned int i=0; i<parms.size(); i++) {
    parmRegex.push_back (Regex::fromPattern(parms[i]));
  }
  vector<Regex> excludeRegex;
  for (unsigned int i=0; i<excludePatterns.size(); i++) {
    excludeRegex.push_back (Regex::fromPattern(excludePatterns[i]));
  }
  // Find all parms matching the parms.
  // Exclude them if matching an excludePattern
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if (*iter) {
      String parmName ((*iter)->getName());
      // Loop through all regex-es until a match is found.
      for (vector<Regex>::const_iterator incIter = parmRegex.begin();
	   incIter != parmRegex.end();
	   incIter++)
      {
	if (parmName.matches(*incIter)) {
	  bool parmExc = false;
	  // Test if not excluded.
	  for (vector<Regex>::const_iterator excIter = excludeRegex.begin();
	       excIter != excludeRegex.end();
	       excIter++)
	  {
	    if (parmName.matches(*excIter)) {
	      parmExc = true;
	      break;
	    }
	  }
	  if (!parmExc) {
	    LOG_TRACE_OBJ_STR( "setSolvable: " << (*iter)->getName());
	    (*iter)->setSolvable (true);
	  }
	  break;
	}
      }
    }
  }
}

void Prediffer::mapDataFiles (const string& inColumnName,
			      const string& outColumnName)
{
  Table tab;
  if (! inColumnName.empty()) {
    string inFile = itsMSName + "/vis." + inColumnName;
    if (itsInDataMap == 0  ||  itsInDataMap->getFileName() != inFile) {
      tab = Table(itsMSName);
      ASSERTSTR (tab.tableDesc().isColumn(inColumnName),
		 "Column " << inColumnName << " does not exist");
      delete itsInDataMap;
      itsInDataMap = 0;
      // See if the input file is the previous output file.
      if (itsOutDataMap  &&  itsOutDataMap->getFileName() == inFile) {
	itsInDataMap = itsOutDataMap;
	itsOutDataMap = 0;
      } else {
	itsInDataMap = new MMap (inFile, MMap::Read);
      }
    }
  }
  if (! outColumnName.empty()) {
    string outFile = itsMSName + "/vis." + outColumnName;
    if (tab.isNull()) {
      tab = Table(itsMSName);
    }
    if (itsOutDataMap == 0  ||  itsOutDataMap->getFileName() != outFile) {
      if (! tab.tableDesc().isColumn(outColumnName)) {
	addDataColumn (tab, outColumnName, outFile);
      }
      delete itsOutDataMap;
      itsOutDataMap = 0;
      itsOutDataMap = new MMap (outFile, MMap::ReWr);
    }
  }
}

// Currently this function always returns all data in the work domain.
// In the future it can be made smarter and process less if the work domain
// is very large.
bool Prediffer::nextDataChunk (bool useFitters)
{
  if (itsNrTimesDone >= itsNrTimes) {
    return false;
  }
  /// For the time being all times of the work domain are mapped in.
  int nt = itsNrTimes;
  int st = itsNrTimesDone;
  // Map the part of the file matching the given times.
  BBSTest::ScopedTimer mapTimer("P:file-mapping");

  int64 nrValues = nt * itsMSMapInfo.timeSize();
  int64 startOffset = itsTimeIndex * itsMSMapInfo.timeSize();
  if (itsInDataMap) {
    itsInDataMap->mapFile(startOffset*sizeof(fcomplex),
			  nrValues*sizeof(fcomplex)); 
    itsMSMapInfo.setInData(static_cast<fcomplex*>(itsInDataMap->getStart()));
  }
  if (itsOutDataMap) {
    itsOutDataMap->mapFile(startOffset*sizeof(fcomplex),
			   nrValues*sizeof(fcomplex)); 
    itsMSMapInfo.setOutData(static_cast<fcomplex*>(itsOutDataMap->getStart()));
  }
  // Map the correct flags subset (this time interval).
  itsFlagsMap->mapFile(startOffset, nrValues); 
  // Map the weights.
  ///  itsWeightMap->mapFile(..,..);
  // Fill the time vector.
  itsChunkTimes.resize (nt+1);
  for (int i=0; i<nt; ++i) {
    itsChunkTimes[i] = itsTimes[itsTimeIndex + st + i] -
                       itsIntervals[itsTimeIndex + st + i] / 2;
  }
  itsChunkTimes[nt] = itsTimes[itsTimeIndex + st + nt-1] +
                      itsIntervals[itsTimeIndex + st + nt-1] / 2;
  itsMSMapInfo.setTimes (st, nt);
  itsNrTimesDone += nt;
  if (useFitters) {
    // Determine for each time point to which fitter it belongs.
    const vector<MeqDomain>& localSolveDomains = itsParmData.getDomains();
    itsTimeFitInx.resize (nt);
    uint interv = 0;
    for (int i=0; i<nt; i++) {
      double time = itsTimes[itsTimeIndex + st + i];
      while (time > localSolveDomains[interv*itsFreqNrFit].endY()) {
	interv++;
	DBGASSERT (interv < localSolveDomains.size()/itsFreqNrFit);
      }
      itsTimeFitInx[i] = interv;
    }
  }
  return true;
}

void Prediffer::addDataColumn (Table& tab, const string& columnName,
			       const string& symlinkName)
{
  tab.reopenRW();
  File fil(symlinkName);
  ASSERT (!fil.exists());
  ArrayColumnDesc<Complex> resCol(columnName,
				  IPosition(2,itsNCorr, itsNrChan),
				  ColumnDesc::FixedShape);
  String stManName = "Tiled_"+columnName;
  TiledColumnStMan tiledRes(stManName, IPosition(3,itsNCorr,itsNrChan,1));
  tab.addColumn (resCol, tiledRes);
  tab.flush();
  // Find out which datamanager the new column is in.
  // Create symlink for it.
  Record dminfo = tab.dataManagerInfo();
  ostringstream filNam;
  for (uint i=0; i<dminfo.nfields(); ++i) {
    const Record& dm = dminfo.subRecord(i);
    if (dm.asString("NAME") == stManName) {
      ostringstream ostr;
      filNam << "table.f" << i << "_TSM0";
      SymLink sl(fil);
      sl.create (filNam.str());
      break;
    }
  }
}

void Prediffer::processData (const string& inColumnName,
			     const string& outColumnName,
			     bool useFlags, bool preCalc, bool calcDeriv,
			     ProcessFuncBL pfunc,
			     void* arg)
{
  // Map the correct input and output file (if needed).
  mapDataFiles (inColumnName, outColumnName);
  // Calculate frequency axis info.
  int nrchan = itsLastChan-itsFirstChan+1;
  double startFreq = itsStartFreq + itsFirstChan*itsStepFreq;
  double endFreq   = itsStartFreq + (itsLastChan+1)*itsStepFreq;
  unsigned int freqOffset = itsFirstChan*itsNCorr;
  // Determine if perturbed values have to be calculated.
  int nrpert = calcDeriv ? itsNrPert:0;
  // Loop through the domain of the data to be processed.
  // Process as much data as possible (hopefully everything).
  itsNrTimesDone = 0;
  while (nextDataChunk(nrpert>0)) {
    int nrtimes = itsMSMapInfo.nrTimes();
    // Initialize the ComplexArr pool with the most frequently used size
    // itsNrChan is the number of frequency channels
    MeqMatrixComplexArr::poolDeactivate();
    MeqMatrixRealArr::poolDeactivate();
    MeqMatrixComplexArr::poolActivate(itsNrChan * nrtimes);
    MeqMatrixRealArr::poolActivate(itsNrChan * nrtimes);

    // Size the thread private flag buffers.
    if (useFlags) {
      for (int i=0; i<itsNthread; ++i) {
	itsFlagVecs[i].resize (nrtimes*nrchan*itsNCorr);
      }
    }
    fcomplex* inDataStart  = itsMSMapInfo.inData();
    fcomplex* outDataStart = itsMSMapInfo.outData();
    void* flagStart = itsFlagsMap->getStart();
    int flagStartBit = itsFlagsMap->getStartBit();

    // Create a request.
    MeqDomain domain(startFreq, endFreq, itsChunkTimes[0],
		     itsChunkTimes[itsChunkTimes.size()-1]);
    MeqRequest request(domain, nrchan, itsChunkTimes, nrpert);
    request.setFirstX (itsFirstChan);
    // Loop through all baselines.
    //static NSTimer timer("Prediffer::fillFitters", true);
    //timer.start();
    if (preCalc) {
      precalcNodes (request);
    }

    // Loop through all baselines and fill its equations if selected.
#pragma omp parallel
    {
#pragma omp for schedule(dynamic)
      for (uint bls=0; bls<itsBLSelInx.size(); ++bls) {
	int blindex = itsBLSelInx[bls];
	int bl = itsBLUsedInx[blindex];
	// Get pointer to correct data part.
	unsigned int offset = freqOffset + bl*itsNrChan*itsNCorr;
	fcomplex* idata = inDataStart + offset;
	fcomplex* odata = outDataStart + offset;
	// Convert the flag bits to bools.
	///	  if (ant1==8&&ant2==11&&tStep<5) {
	  ///cout << "flagmap: start="<<flagStart<<" sbit="<<flagStartBit
	  /// << " offs="<<offset<<endl;
	  ///	  }
#if defined _OPENMP
	int threadNr = omp_get_thread_num();
#else
	int threadNr = 0;
#endif
	// If needed, convert flag bits to bools.
	bool* flags = 0;
	if (useFlags) {
	  flags = &itsFlagVecs[threadNr][0];
	  for (int i=0; i<itsMSMapInfo.nrTimes(); ++i) {
	    bitToBool (flags+i*nrchan*itsNCorr, flagStart, nrchan*itsNCorr,
		       offset + flagStartBit);
	    offset += itsMSMapInfo.timeSize();
	  }
	}
	// Call the given member function.
	(this->*pfunc) (threadNr, arg, idata, odata, flags,
			request, blindex, false);
			///request, blindex, itsAnt1[bl]==4&&itsAnt2[bl]==8);
      }
    } // end omp parallel
      //timer.stop();
  }
}

void Prediffer::precalcNodes (const MeqRequest& request)
{
#pragma omp parallel
  {
    // Loop through expressions to be precalculated.
    // At each level the expressions can be executed in parallel.
    // Level 0 is formed by itsExpr which are not calculated here.
    for (int level = itsPrecalcNodes.size(); --level > 0;) {
      vector<MeqExprRep*>& exprs = itsPrecalcNodes[level];
      int nrExprs = exprs.size();
      if (nrExprs > 0) {
#pragma omp for schedule(dynamic)
	for (int i=0; i<nrExprs; ++i) {
	  exprs[i]->precalculate (request);
	}
      }
    }
  } // end omp parallel
}

//----------------------------------------------------------------------
//
// ~fillFitters
//
// Fill the fitter with the condition equations for the selected baselines
// and domain.
//
//----------------------------------------------------------------------
void Prediffer::fillFitters (vector<casa::LSQFit>& fitters,
			     const string& dataColumnName)
{
  // Find all nodes to be precalculated.
  setPrecalcNodes (itsExpr);
  // Initialize the fitters.
  int nfitter = itsNrScids.size();
  fitters.resize (nfitter);
  // Create thread private fitters for parallel execution.
  vector<vector<LSQFit*> > threadPrivateFitters(itsNthread);
  for (int i=0; i<itsNthread; ++i) {
    threadPrivateFitters[i].resize (nfitter);
  }
  for (int i=0; i<nfitter; ++i) {
    fitters[i].set (itsNrScids[i]);
    threadPrivateFitters[0][i] = &fitters[i];
    for (int j=1; j<itsNthread; j++) {
      threadPrivateFitters[j][i] = new LSQFit(itsNrScids[i]);
    }
  }
  // Size the thread private buffers.
  ///int nrchan = itsLastChan-itsFirstChan+1;
  for (int i=0; i<itsNthread; ++i) {
    itsResultVecs[i].resize (2*itsNrPert);
    itsDiffVecs[i].resize (3*itsNrPert);
    itsIndexVecs[i].resize (itsNrPert);
    ///    itsOrdFlagVecs[i].resize (2*nrchan);
  }
  // Process the data and use the flags.
  processData (dataColumnName, "", true, true, true,
	       &Prediffer::fillEquation, &threadPrivateFitters);
  // Merge the thread-specific fitters into the main ones.
  for (int j=1; j<itsNthread; ++j) {
    for (int i=0; i<nfitter; ++i) {
      fitters[i].merge (*threadPrivateFitters[j][i]);
      delete threadPrivateFitters[j][i];
    }
  }
  BBSTest::Logger::log(itsPredTimer);
  BBSTest::Logger::log(itsEqTimer);
  itsPredTimer.reset();
  itsEqTimer.reset();
}

void Prediffer::correctData (const string& inColumnName,
			     const string& outColumnName, bool flush)
{
  // Find all nodes to be precalculated.
  setPrecalcNodes (itsCorrExpr);
  processData (inColumnName, outColumnName, false, true, false,
	       &Prediffer::correctBL, 0);
  if (flush) {
    itsOutDataMap->flush();
  }
}

void Prediffer::subtractData (const string& inColumnName,
			      const string& outColumnName, bool flush)
{
  // Find all nodes to be precalculated.
  setPrecalcNodes (itsExpr);
  processData (inColumnName, outColumnName, false, true, false,
	       &Prediffer::subtractBL, 0);
  if (flush) {
    itsOutDataMap->flush();
  }
}

void Prediffer::writePredictedData (const string& outColumnName)
{
  // Find all nodes to be precalculated.
  setPrecalcNodes (itsExpr);
  processData ("", outColumnName, false, true, false,
	       &Prediffer::predictBL, 0);
}

void Prediffer::getData (const string& columnName, bool useTree,
			 Array<Complex>& dataArr, Array<Bool>& flagArr)
{
  int nrchan = itsLastChan-itsFirstChan+1;
  dataArr.resize (IPosition(4, itsNCorr, nrchan, itsNrUsedBl, itsNrTimes));
  flagArr.resize (IPosition(4, itsNCorr, nrchan, itsNrUsedBl, itsNrTimes));
  pair<Complex*,bool*> p;
  p.first = dataArr.data();
  p.second = flagArr.data();
  if (!useTree) {
    processData (columnName, "", true, false, false, &Prediffer::getBL, &p);
    return;
  }
  vector<MeqJonesExpr> expr(itsNrUsedBl);
  for (int i=0; i<itsNrUsedBl; ++i) {
    expr[i] = new MeqJonesMMap (itsMSMapInfo, itsBLUsedInx[i]);
  }
  pair<pair<Complex*,bool*>*, vector<MeqJonesExpr>*> p1;
  p1.first = &p;
  p1.second = &expr;
  processData (columnName, "", true, false, false, &Prediffer::getMapBL, &p1);
}

//----------------------------------------------------------------------
//
// ~fillEquation
//
// Fill the fitter with the equations for the given baseline.
//
//----------------------------------------------------------------------
void Prediffer::fillEquation (int threadnr, void* arg,
			      const fcomplex* data, fcomplex*,
			      const bool* flags,
			      const MeqRequest& request, int blindex,
			      bool showd)
{
  ///int bl = itsBLUsedInx[blindex];
  ///int ant1 = itsAnt1[bl];
  ///int ant2 = itsAnt2[bl];
  //static NSTimer fillEquationTimer("fillEquation", true);
  //fillEquationTimer.start();
  // Get the fitter objects.
  vector<LSQFit*>& fitters =
    (*static_cast<vector<vector<LSQFit*> >*>(arg))[threadnr];
  // Allocate temporary vectors.
  // If needed, they can be pre-allocated per thread, so there is no
  // malloc needed for each invocation. Currently it is believed that
  // the work domains are so large, that the malloc overhead is negligible.
  // ON the other hand, there are many baselines and the malloc is done
  // for each of them.
  uint* indices = &(itsIndexVecs[threadnr][0]);
  const double** pertReal = &(itsResultVecs[threadnr][0]);
  const double** pertImag = pertReal + itsNrPert;
  double* invPert = &(itsDiffVecs[threadnr][0]);
  double* resultr = invPert + itsNrPert;
  double* resulti = resultr + itsNrPert;
  // Get all equations.
  itsPredTimer.start();
  MeqJonesExpr& expr = itsExpr[blindex];
  // Do the actual predict for the entire work domain.
  MeqJonesResult jresult = expr.getResult (request); 
  itsPredTimer.stop();

  itsEqTimer.start();
  int nrchan = request.nx();
  int nrtime = request.ny();
  int timeSize = itsMSMapInfo.timeSize();
  // Put the results in a single array for easier handling.
  const MeqResult* predResults[4];
  predResults[0] = &(jresult.getResult11());
  if (itsNCorr == 2) {
    predResults[1] = &(jresult.getResult22());
  } else if (itsNCorr == 4) {
    predResults[1] = &(jresult.getResult12());
    predResults[2] = &(jresult.getResult21());
    predResults[3] = &(jresult.getResult22());
  }

  // Determine start and incr reflecting the order of the observed data.
  int sdch = itsReverseChan ? itsNCorr * (nrchan - 1) : 0;
  int inc  = itsReverseChan ? -itsNCorr : itsNCorr;
  // To avoid having to use large temporary arrays, step through the
  // data by timestep and correlation.
  uint nreq=0;
  for (int corr=0; corr<itsNCorr; corr++, data++, flags++) {
    if (itsCorr[corr]) {
      // Get the results for this correlation.
      const MeqResult& tcres = *predResults[corr];
      // Get pointers to the main data.
      const MeqMatrix& val = tcres.getValue();
      const double* realVals;
      const double* imagVals;
      val.dcomplexStorage(realVals, imagVals);

      // Determine which parameters have derivatives and keep that info.
      // E.g. when solving for station parameters, only a few parameters
      // per baseline have derivatives.
      // Note that this is the same for the entire work domain.
      // Also get pointers to the perturbed values.
      int nrParamsFound = 0;
      for (int scinx=0; scinx<itsNrPert; ++scinx) {
	if (tcres.isDefined(scinx)) {
	  indices[nrParamsFound] = scinx;
	  const MeqMatrix& valp = tcres.getPerturbedValue(scinx);
	  valp.dcomplexStorage (pertReal[nrParamsFound],
				pertImag[nrParamsFound]);
	  invPert[nrParamsFound] = 1. / tcres.getPerturbation(scinx, 0);
	  nrParamsFound++;
	}
      }

      if (nrParamsFound > 0) {
	const fcomplex* cdata  = data;
	const bool*     cflags = flags;
	if (fitters.size() == 1) {
	  // No fitter indexing needed if only one fitter.
	  for (int tim=0; tim<nrtime; tim++) {
	    // Loop through all channels.
	    if (showd) {
	      showData (corr, sdch, inc, nrchan, cflags, cdata,
			realVals, imagVals);
	    }
	    // Form two equations for each unflagged data point.
	    int dch = sdch;
	    for (int ch=0; ch<nrchan; ++ch, dch+=inc) {
	      if (! cflags[dch]) {
		double diffr = real(cdata[dch]) - *realVals;
		double diffi = imag(cdata[dch]) - *imagVals;
		for (int scinx=0; scinx<nrParamsFound; ++scinx) {
		  // Calculate the derivative for real and imag part.
		  double invp = invPert[scinx];
		  resultr[scinx] = (*pertReal[scinx]++ - *realVals) * invp;
		  resulti[scinx] = (*pertImag[scinx]++ - *imagVals) * invp;
		}
		// Now add the equations to the correct fitter object.
		if (nrParamsFound != itsNrPert) {
		  fitters[0]->makeNorm (nrParamsFound, indices,
					resultr, 1., diffr);
		  fitters[0]->makeNorm (nrParamsFound, indices,
					resulti, 1., diffi);
		} else {
		  fitters[0]->makeNorm (resultr, 1., diffr);
		  fitters[0]->makeNorm (resulti, 1., diffi);
		}
		if (showd) {
		  cout << "eq"<<corr<<" ch " << 2*ch << " diff " << diffr;
		  for (int ii=0; ii<nrParamsFound; ii++) {
		    cout << ' '<<resultr[ii];
		  }
		  cout << endl;
		  cout << "eq"<<corr<<" ch " << 2*ch+1 << " diff " << diffi;
		  for (int ii=0; ii<nrParamsFound; ii++) {
		    cout << ' '<<resulti[ii];
		  }
		  cout << endl;
		}
		nreq += 2;
	      } else {
		for (int scinx=0; scinx<nrParamsFound; ++scinx) {
		  pertReal[scinx]++;
		  pertImag[scinx]++;
		}
	      }
	      realVals++;
	      imagVals++;
	    }
	    cdata  += timeSize;       // next observed data time step
	    cflags += nrchan*itsNCorr;
	  }
	} else {
	  // Multiple fitters, so for each point the correct fitter
	  // has to be determined.
	  for (int tim=0; tim<nrtime; tim++) {
	    // Loop through all channels.
	    if (showd) {
	      showData (corr, sdch, inc, nrchan, cflags, cdata,
			realVals, imagVals);
	    }
	    // Get first fitter index for this time line.
	    int fitInxT = itsTimeFitInx[tim] * itsFreqNrFit;
	    // Form two equations for each unflagged data point.
	    int dch = sdch;
	    for (int ch=0; ch<nrchan; ++ch, dch+=inc) {
	      if (! cflags[dch]) {
		int fitInx = fitInxT + itsFreqFitInx[ch];
		double diffr = real(cdata[dch]) - *realVals;
		double diffi = imag(cdata[dch]) - *imagVals;
		for (int scinx=0; scinx<nrParamsFound; ++scinx) {
		  // Calculate the derivative for real and imag part.
		  double invp = 1. / tcres.getPerturbation(indices[scinx],
							   fitInx);
		  resultr[scinx] = (*pertReal[scinx]++ - *realVals) * invp;
		  resulti[scinx] = (*pertImag[scinx]++ - *imagVals) * invp;
		}
		// Now add the equations to the correct fitter object.
		if (nrParamsFound != itsNrPert) {
		  fitters[fitInx]->makeNorm (nrParamsFound, indices,
					     resultr, 1., diffr);
		  fitters[fitInx]->makeNorm (nrParamsFound, indices,
					     resulti, 1., diffi);
		} else {
		  fitters[fitInx]->makeNorm (resultr, 1., diffr);
		  fitters[fitInx]->makeNorm (resulti, 1., diffi);
		}
		if (showd) {
		  cout << "eq"<<corr<<" ch " << 2*ch << " diff " << diffr;
		  for (int ii=0; ii<nrParamsFound; ii++) {
		    cout << ' '<<resultr[ii];
		  }
		  cout << endl;
		  cout << "eq"<<corr<<" ch " << 2*ch+1 << " diff " << diffi;
		  for (int ii=0; ii<nrParamsFound; ii++) {
		    cout << ' '<<resulti[ii];
		  }
		  cout << endl;
		}
		nreq += 2;
	      } else {
		for (int scinx=0; scinx<nrParamsFound; ++scinx) {
		  pertReal[scinx]++;
		  pertImag[scinx]++;
		}
	      }
	      realVals++;
	      imagVals++;
	    }
	    cdata  += timeSize;       // next observed data time step
	    cflags += nrchan*itsNCorr;
	  }
	}
      }
    }
  }
  //fillEquationTimer.stop();
  itsEqTimer.stop();
  ///  cout << "nreq="<<nreq<<endl;
}

void Prediffer::showData (int corr, int sdch, int inc, int nrchan,
			  const bool* flags, const fcomplex* data,
			  const double* realVals, const double* imagVals)
{
  cout << "flag=" << corr << "x ";
  int dch = sdch;
  for (int ch=0; ch<nrchan; ch++, dch+=inc) {
    cout << flags[dch]<< ' ';
  }
  cout << endl;
  cout << "cor=" << corr << "x ";
  dch = sdch;
  for (int ch=0; ch<nrchan; ch++, dch+=inc) {
    cout << '(' << std::setprecision(12)
	 << real(data[dch])
	 << ',' << std::setprecision(12)
	 << imag(data[dch])<< ')';
  }
  cout << endl;
  cout << "corr=" << corr << "x ";
  for (int ch=0; ch<nrchan; ch++) {
    cout << '(' << std::setprecision(12)
	 << realVals[ch]
	 << ',' << std::setprecision(12)
	 << imagVals[ch]<< ')';
  }
  cout << endl;
}

//----------------------------------------------------------------------
//
// ~getResults
//
// Get the results for the selected baselines and domain.
//
//----------------------------------------------------------------------
vector<MeqResult> Prediffer::getResults (bool calcDeriv)
{
  // Find all nodes to be precalculated.
  setPrecalcNodes (itsExpr);
  // Allocate result vector.
  vector<MeqResult> results;
  int nrchan = itsLastChan-itsFirstChan+1;
  double startFreq = itsStartFreq + itsFirstChan*itsStepFreq;
  double endFreq   = itsStartFreq + (itsLastChan+1)*itsStepFreq;
  for (unsigned int tStep=0; tStep<itsNrTimes; tStep++)
  {
    double time = itsTimes[itsTimeIndex+tStep];
    double interv = itsIntervals[itsTimeIndex+tStep];
    MeqDomain domain(startFreq, endFreq, time-interv/2, time+interv/2);
    MeqRequest request(domain, nrchan, 1, 0);
    if (calcDeriv) {
      request = MeqRequest(domain, nrchan, 1, itsNrPert);
    }
    // Loop through expressions to be precalculated.
    // We can parallellize them at each level.
    // Level 0 is formed by itsExpr which are not calculated here.
    for (int level = itsPrecalcNodes.size(); -- level > 0;) {
      vector<MeqExprRep*> exprs = itsPrecalcNodes[level];
      int nrExprs = exprs.size();
      if (nrExprs > 0) {
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < nrExprs; i ++) {
	  exprs[i]->precalculate (request);
	}
      }
    }
    // Evaluate for all selected baselines.
    for (uint bls=0; bls<itsBLSelInx.size(); ++bls) {
      int blindex = itsBLSelInx[bls];
      // Get the result for this baseline.
      MeqJonesExpr& expr = itsExpr[blindex];
      // This is the actual predict.
      MeqJonesResult result = expr.getResult (request);
      results.push_back (result.getResult11());
      results.push_back (result.getResult12());
      results.push_back (result.getResult21());
      results.push_back (result.getResult22());
    }
  }
  return results;
}

void Prediffer::getBL (int, void* arg,
		       const fcomplex* data, fcomplex*,
		       const bool* flags,
		       const MeqRequest&, int blindex,
		       bool)
{
  Complex* datap = static_cast<pair<Complex*,bool*>*>(arg)->first;
  bool*    flagp = static_cast<pair<Complex*,bool*>*>(arg)->second;
  int nrchan = itsLastChan-itsFirstChan+1;
  memcpy (datap+blindex*nrchan*itsNCorr, data,
	  nrchan*itsNCorr*sizeof(Complex));
  memcpy (flagp+blindex*nrchan*itsNCorr, flags,
	  nrchan*itsNCorr*sizeof(bool));
}

void Prediffer::getMapBL (int, void* arg,
			  const fcomplex*, fcomplex*,
			  const bool* flags,
			  const MeqRequest& request, int blindex,
			  bool)
{
  pair<pair<Complex*,bool*>*, vector<MeqJonesExpr>*>* p1 =
    static_cast<pair<pair<Complex*,bool*>*, vector<MeqJonesExpr>*>*>(arg);
  Complex* datap = p1->first->first;
  bool*    flagp = p1->first->second;
  vector<MeqJonesExpr>& expr = *p1->second;
  int nrchan = itsLastChan-itsFirstChan+1;
  datap += blindex*nrchan*itsNCorr;
  memcpy (flagp+blindex*nrchan*itsNCorr, flags,
	  nrchan*itsNCorr*sizeof(bool));
  MeqJonesResult jresult = expr[blindex].getResult (request);
  const double *r1, *i1, *r2, *i2, *r3, *i3, *r4, *i4;
  jresult.result11().getValue().dcomplexStorage (r1, i1);
  jresult.result12().getValue().dcomplexStorage (r2, i2);
  jresult.result21().getValue().dcomplexStorage (r3, i3);
  jresult.result22().getValue().dcomplexStorage (r4, i4);
  int nx = jresult.result11().getValue().nx();
  for (int iy=0; iy<jresult.result11().getValue().ny(); ++iy) {
    if (itsReverseChan) {
      for (int ix=nx; ix>0;) {
	ix--;
	int i = iy*nx + ix;
	*datap++ = Complex(r1[i], i1[i]);
	if (itsNCorr > 2) {
	  *datap++ = Complex(r2[i], i2[i]);
	  *datap++ = Complex(r3[i], i3[i]);
	}
	if (itsNCorr > 1) {
	  *datap++ = Complex(r4[i], i4[i]);
	}
      }
    } else {
      for (int ix=0; ix<nx; ++ix) {
	int i = iy*nx + ix;
	*datap++ = Complex(r1[i], i1[i]);
	if (itsNCorr > 2) {
	  *datap++ = Complex(r2[i], i2[i]);
	  *datap++ = Complex(r3[i], i3[i]);
	}
	if (itsNCorr > 1) {
	  *datap++ = Complex(r4[i], i4[i]);
	}
      }
    }
  }
}

void Prediffer::subtractBL (int, void*,
			    const fcomplex* dataIn, fcomplex* dataOut,
			    const bool*,
			    const MeqRequest& request, int blindex,
			    bool)
{
  itsPredTimer.start();
  MeqJonesExpr& expr = itsExpr[blindex];
  // Do the actual predict.
  MeqJonesResult jresult = expr.getResult (request); 
  itsPredTimer.stop();

  itsEqTimer.start();
  int nrchan = request.nx();
  int nrtime = request.ny();
  int timeSize = itsMSMapInfo.timeSize();
  const double* predReal[4];
  const double* predImag[4];
  jresult.getResult11().getValue().dcomplexStorage(predReal[0], predImag[0]);
  if (itsNCorr == 2) {
    jresult.getResult22().getValue().dcomplexStorage(predReal[1], predImag[1]);
  } else if (itsNCorr == 4) {
    jresult.getResult12().getValue().dcomplexStorage(predReal[1], predImag[1]);
    jresult.getResult21().getValue().dcomplexStorage(predReal[2], predImag[2]);
    jresult.getResult22().getValue().dcomplexStorage(predReal[3], predImag[3]);
  }
  // Loop through the times and correlations.
  for (int tim=0; tim<nrtime; tim++) {
    const fcomplex* idata = dataIn;
    fcomplex* odata = dataOut;
    for (int corr=0; corr<itsNCorr; corr++, idata++, odata++) {
      if (itsCorr[corr]) {
	const double* realVals = predReal[corr];
	const double* imagVals = predImag[corr];
	// Subtract predicted from the data.
	int dch = itsReverseChan ? itsNCorr * (nrchan - 1) : 0;
	int inc = itsReverseChan ? -itsNCorr : itsNCorr;
	if (idata == odata) {
	  for (int ch=0; ch<nrchan; ch++, dch+=inc) {
	    odata[dch] -= makefcomplex(realVals[ch], imagVals[ch]);
	  }
	} else {
	  for (int ch=0; ch<nrchan; ch++, dch+=inc) {
	    odata[dch] = idata[dch] - makefcomplex(realVals[ch],
						   imagVals[ch]);
	  }
	}
	predReal[corr] += nrchan;
	predImag[corr] += nrchan;
      }
    }
    dataIn  += timeSize;
    dataOut += timeSize;
  }
  itsEqTimer.stop();
}

void Prediffer::correctBL (int, void*,
			   const fcomplex*, fcomplex*,
			   const bool*,
			   const MeqRequest& request, int blindex,
			   bool)
{
  itsPredTimer.start();
  MeqJonesResult res = itsCorrExpr[blindex].getResult (request);
  itsPredTimer.stop();
  itsEqTimer.start();
  itsCorrMMap[blindex]->putJResult (res, request);
  itsEqTimer.stop();
}

void Prediffer::predictBL (int, void*,
			   const fcomplex*, fcomplex* dataOut,
			   const bool*,
			   const MeqRequest& request, int blindex,
			   bool)
{
  itsPredTimer.start();
  MeqJonesExpr& expr = itsExpr[blindex];
  // Do the actual predict.
  MeqJonesResult jresult = expr.getResult (request); 
  itsPredTimer.stop();

  itsEqTimer.start();
  int nrchan = request.nx();
  int nrtime = request.ny();
  int timeSize = itsMSMapInfo.timeSize();
  // Put the results in a single array for easier handling.
  const double* predReal[4];
  const double* predImag[4];
  jresult.getResult11().getValue().dcomplexStorage(predReal[0], predImag[0]);
  if (itsNCorr == 2) {
    jresult.getResult22().getValue().dcomplexStorage(predReal[1], predImag[1]);
  } else if (itsNCorr == 4) {
    jresult.getResult12().getValue().dcomplexStorage(predReal[1], predImag[1]);
    jresult.getResult21().getValue().dcomplexStorage(predReal[2], predImag[2]);
    jresult.getResult22().getValue().dcomplexStorage(predReal[3], predImag[3]);
  }
  // Loop through the times and correlations.
  for (int tim=0; tim<nrtime; tim++) {
    fcomplex* data = dataOut;
    for (int corr=0; corr<itsNCorr; corr++, data++) {
      if (itsCorr[corr]) {
	const double* realVals = predReal[corr];
	const double* imagVals = predImag[corr];
	// Store the predict in data.
	int dch = itsReverseChan ? itsNCorr * (nrchan - 1) : 0;
	int inc = itsReverseChan ? -itsNCorr : itsNCorr;
	for (int ch=0; ch<nrchan; ch++, dch+=inc) {
	  data[dch] = makefcomplex(realVals[ch], imagVals[ch]);
	}
	predReal[corr] += nrchan;
	predImag[corr] += nrchan;
      }
    }
    dataOut += timeSize;
  }
  itsEqTimer.stop();
}


//----------------------------------------------------------------------
//
// ~select
//
// Select a subset of the MS data.
// This selection has to be done before the loop over domains.
//
//----------------------------------------------------------------------
void Prediffer::select (const vector<int>& ant1, 
			const vector<int>& ant2,
			bool useAutoCorrelations,
			const vector<int>& corr)
{
  ASSERT (ant1.size() == ant2.size());
  if (ant1.size() == 0) {
    // No baselines specified, select all baselines
    itsBLSelection = true;
  } else {
    itsBLSelection = false;
    for (unsigned int j=0; j<ant2.size(); j++) {
      ASSERT(ant2[j] < int(itsBLSelection.nrow()));
      for (unsigned int i=0; i<ant1.size(); i++) {
	ASSERT(ant1[i] < int(itsBLSelection.nrow()));
	itsBLSelection(ant1[i], ant2[j]) = true;     // select this baseline
      }
    }
  }
  // Unset auto-correlations if needed.
  if (!useAutoCorrelations) {
    for (unsigned int i=0; i<itsBLSelection.nrow(); i++) {
      itsBLSelection(i,i) = false;
    }
  } 
  // Fill the correlations to use. Use all if vector is empty.
  // If only 2 corr, YY is the 2nd one.
  for (int i=0; i<4; ++i) {
    itsCorr[i] = corr.empty();
  }
  for (uint i=0; i<corr.size(); ++i) {
    if (corr[i] < 4) {
      if (itsNCorr == 2  &&  corr[i] == 3) {
	itsCorr[1] = true;
      } else {
	itsCorr[corr[i]] = true;
      }
    }
  }
  countBaseCorr();
}

//----------------------------------------------------------------------
//
// ~fillUVW
//
// Calculate the station UVW coordinates from the MS.
//
//----------------------------------------------------------------------
void Prediffer::fillUVW()
{
  LOG_TRACE_RTTI( "get UVW coordinates from MS" );
  int nant = itsStatUVW.size();
  vector<bool> statFnd (nant);
  vector<bool> statDone (nant);
  vector<double> statuvw(3*nant);

  // Determine the number of stations (found)
  statFnd.assign (statFnd.size(), false);
  int nStatFnd = 0;
  for (unsigned int bl=0; bl<itsNrBl; bl++) {
    int a1 = itsAnt1[bl];
    int a2 = itsAnt2[bl];
    if (itsBLSelection(a1,a2) == true) {
      if (!statFnd[a1]) {
	nStatFnd++;
	statFnd[a1] = true;
      }
      if (!statFnd[a2]) {
	nStatFnd++;
	statFnd[a2] = true;
      }
    }
  }
  // Map uvw data into memory
  size_t nrBytes = itsTimes.nelements() * itsNrBl * 3 * sizeof(double);
  double* uvwDataPtr = 0;
  MMap* mapPtr = new MMap(itsMSName+"/vis.uvw", MMap::Read);
  mapPtr->mapFile(0, nrBytes);
  uvwDataPtr = (double*)mapPtr->getStart();

  // Step time by time through the MS.
  for (unsigned int tStep=0; tStep < itsTimes.nelements(); tStep++) {
    // Set uvw pointer to beginning of this time
    unsigned int tOffset = tStep * itsNrBl * 3;
    double* uvw = uvwDataPtr + tOffset;
    double time = itsTimes[tStep];
    
    // Set UVW of first station used to 0 (UVW coordinates are relative!).
    statDone.assign (statDone.size(), false);
    int ant0 = itsAnt1[itsBLUsedInx[0]];
    statuvw[3*ant0]   = 0;
    statuvw[3*ant0+1] = 0;
    statuvw[3*ant0+2] = 0;
    statDone[ant0] = true;
    itsStatUVW[ant0]->set (time, 0, 0, 0);

//     cout << "itsStatUVW[" << itsAnt1Data[0] << "] time: " << time << " 0, 0, 0" << endl;

    int ndone = 1;
    // Loop until all found stations are handled. This is necessary when not all 
    // stations can be calculated in one loop (depends on the order)
    while (ndone < nStatFnd) {
      int nd = 0;
      // Loop over baselines
      for (unsigned int bl=0; bl<itsNrBl; bl++) {
	int a1 = itsAnt1[bl];
	int a2 = itsAnt2[bl];
	if (itsBLSelection(a1,a2)  &&  itsBLIndex(a1,a2) >= 0) {
	  if (!statDone[a2]) {
	    if (statDone[a1]) {
	      statuvw[3*a2]   = uvw[3*bl]   - statuvw[3*a1];
	      statuvw[3*a2+1] = uvw[3*bl+1] - statuvw[3*a1+1];
	      statuvw[3*a2+2] = uvw[3*bl+2] - statuvw[3*a1+2];
	      statDone[a2] = true;
	      itsStatUVW[a2]->set (time, statuvw[3*a2], statuvw[3*a2+1],
				   statuvw[3*a2+2]);

// 	      cout << "itsStatUVW[" << a2 << "] time: " << time << statuvw[3*a2] << " ," << statuvw[3*a2+1] << " ," << statuvw[3*a2+2] << endl;

	      ndone++;
	      nd++;
	    }
	  } else if (!statDone[a1]) {
	    if (statDone[a2]) {
	      statuvw[3*a1]   = statuvw[3*a2]   - uvw[3*bl];
	      statuvw[3*a1+1] = statuvw[3*a2+1] - uvw[3*bl+1];
	      statuvw[3*a1+2] = statuvw[3*a2+2] - uvw[3*bl+2];
	      statDone[a1] = true;
	      itsStatUVW[a1]->set (time, statuvw[3*a1], statuvw[3*a1+1],
				   statuvw[3*a1+2]);
// 	      cout << "itsStatUVW[" << a1 << "] time: " << time << statuvw[3*a1] << " ," << statuvw[3*a1+1] << " ," << statuvw[3*a1+2] << endl;

	      ndone++;
	      nd++;
	    }
	  }
	  if (ndone == nStatFnd) {
	    break;
	  }
	} // End if (itsBLSelection(ant1,ant2) ==...
      } // End loop baselines
      //	  ASSERT (nd > 0);
    } // End loop stations found
  } // End loop time

  // Finished with map
  delete mapPtr;
}


//----------------------------------------------------------------------
//
// ~peel
//
// Define the source numbers to use in a peel step.
//
//----------------------------------------------------------------------
bool Prediffer::setPeelGroups (const vector<int>& peelGroups,
			       const vector<int>& extraGroups)
{
//   vector<int> allNrs;
//   for (uint i=0; i<extraGroups.size(); ++i) {
//     ASSERT (extraGroups[i] >= 0  &&  extraGroups[i] < int(itsSrcGrp.size()));
//     const vector<int>& grp = itsSrcGrp[i];
//     for (uint j=0; j<grp.size(); ++j) {
//       allNrs.push_back (grp[j] - 1);
//     }
//   }
//   vector<int> peelNrs;
//   for (uint i=0; i<peelGroups.size(); ++i) {
//     ASSERT (peelGroups[i] >= 0  &&  peelGroups[i] < int(itsSrcGrp.size()));
//     const vector<int>& grp = itsSrcGrp[peelGroups[i]];
//     for (uint j=0; j<grp.size(); ++j) {
//       peelNrs.push_back (grp[j] - 1);
//       allNrs.push_back (grp[j] - 1);
//     }
//   }
//   LOG_TRACE_OBJ_STR( "peel sources " << peelNrs << "; predict sources "
// 		     << allNrs );
//   ASSERT (peelNrs.size() > 0);
//   itsSources.setSelected (allNrs);
//   itsPeelSourceNrs = peelNrs;
  return true;
}

void Prediffer::updateSolvableParms (const vector<double>& values)
{
  // Iterate through all parms.
  const vector<MeqParm*>& parmList = itsParmGroup.getParms();
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if (*iter) {
      if ((*iter)->isSolvable()) {
	MeqParmFunklet* ppc = dynamic_cast<MeqParmFunklet*>(*iter);
	ASSERT (ppc);
	ppc->update (values);
      }
    }
  }
  resetEqLoop();
}

void Prediffer::updateSolvableParms (const ParmDataInfo& parmDataInfo)
{
  const vector<ParmData>& parmData = parmDataInfo.parms();
  // Iterate through all parms.
  const vector<MeqParm*>& parmList = itsParmGroup.getParms();
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if (*iter) {
      if ((*iter)->isSolvable()) {
	const string& pname = (*iter)->getName();
	// Update the parameter matching the name.
	for (vector<ParmData>::const_iterator iterpd = parmData.begin();
	     iterpd != parmData.end();
	     iterpd++) {
	  if (iterpd->getName() == pname) {
	    MeqParmFunklet* ppc = dynamic_cast<MeqParmFunklet*>(*iter);
	    ASSERT (ppc);
	    ppc->update (*iterpd);
	    break;
	  }
	  // A non-matching name is ignored.
	}
      }
    }
  }
  resetEqLoop();
}

void Prediffer::updateSolvableParms()
{
  // Iterate through all parms.
  const vector<MeqParm*>& parmList = itsParmGroup.getParms();
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if (*iter) {
      if ((*iter)->isSolvable()) {
	MeqParmFunklet* ppc = dynamic_cast<MeqParmFunklet*>(*iter);
	ASSERT (ppc);
	ppc->updateFromTable();
	
	//       streamsize prec = cout.precision();
	//       cout.precision(10);
	//       cout << "****Read: " << (*iter)->getCoeffValues().getDouble()
	// 	   << " for parameter " << (*iter)->getName() << endl;
	//       cout.precision (prec);
      }
    }
  }
  resetEqLoop();
}

void Prediffer::resetEqLoop()
{
}

void Prediffer::writeParms()
{
  BBSTest::ScopedTimer saveTimer("P:write-parm");
  //  saveTimer.start();
  const vector<MeqParm*>& parmList = itsParmGroup.getParms();

  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if (*iter) {
      if ((*iter)->isSolvable()) {
	(*iter)->save();
      }
    }
  }
  //  saveTimer.stop();
  //  BBSTest::Logger::log("write-parm", saveTimer);
  cout << "wrote timeIndex=" << itsTimeIndex
       << " nrTimes=" << itsNrTimes << endl;
}

void Prediffer::showSettings() const
{
  cout << "Prediffer settings:" << endl;
  cout << "  msname:    " << itsMSName << endl;
  cout << "  mepname:   " << itsMEPName << endl;
  cout << "  gsmname:   " << itsGSMMEPName << endl;
  cout << "  solvparms: " << itsParmData << endl;
  if (itsReverseChan) {
    cout << "  stchan:    " << itsNrChan - 1 - itsFirstChan << endl;
    cout << "  endchan:   " << itsNrChan - 1 - itsLastChan << endl;
    cout << "    Note: channels are in reversed order" << endl;
  } else {
    cout << "  stchan:    " << itsFirstChan << endl;
    cout << "  endchan:   " << itsLastChan << endl;
  }
  cout << "  corr     : " << itsNCorr << "  " << itsCorr[0];
  for (int i=1; i<itsNCorr; ++i) {
    cout << ',' << itsCorr[i];
  }
  cout << endl;
  cout << "  calcuvw  : " << itsCalcUVW << endl;
  cout << endl;
}

} // namespace LOFAR
