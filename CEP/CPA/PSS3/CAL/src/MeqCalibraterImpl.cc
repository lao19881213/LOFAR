//# MeqCalibraterImpl.cc: Implementation of the MeqCalibrater DO
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

#include <CAL/MeqCalibraterImpl.h>

#include <MNS/MeqJonesNode.h>
#include <MNS/MeqMatrixTmp.h>
#include <MNS/MeqParmSingle.h>
#include <MNS/MeqPointDFT.h>
#include <MNS/MeqPointSource.h>
#include <MNS/MeqWsrtInt.h>
#include <MNS/MeqWsrtPoint.h>

#include <Common/Debug.h>

#include <aips/Arrays/ArrayIO.h>
#include <aips/Arrays/ArrayMath.h>
#include <aips/Arrays/ArrayLogical.h>
#include <aips/Arrays/Matrix.h>
#include <aips/Arrays/Slice.h>
#include <aips/Arrays/Vector.h>
#include <aips/Exceptions/Error.h>
#include <aips/Functionals/Polynomial.h>
#include <aips/Glish/GlishArray.h>
#include <aips/Glish/GlishRecord.h>
#include <aips/Glish/GlishValue.h>
#include <aips/Mathematics/AutoDiff.h>
#include <aips/Mathematics/Constants.h>
#include <aips/MeasurementSets/MSAntenna.h>
#include <aips/MeasurementSets/MSAntennaColumns.h>
#include <aips/MeasurementSets/MSDataDescription.h>
#include <aips/MeasurementSets/MSDataDescColumns.h>
#include <aips/MeasurementSets/MSField.h>
#include <aips/MeasurementSets/MSFieldColumns.h>
#include <aips/MeasurementSets/MSSpectralWindow.h>
#include <aips/MeasurementSets/MSSpWindowColumns.h>
#include <aips/Measures/MDirection.h>
#include <aips/Measures/MeasConvert.h>
#include <aips/OS/Timer.h>
#include <aips/Quanta/MVBaseline.h>
#include <aips/Quanta/MVPosition.h>
#include <aips/Tables/ArrColDesc.h>
#include <aips/Tables/ArrayColumn.h>
#include <aips/Tables/ColumnDesc.h>
#include <aips/Tables/ExprNode.h>
#include <aips/Tables/SetupNewTab.h>
#include <aips/Utilities/Regex.h>
#include <trial/Fitting/LinearFit.h>
#include <trial/Tasking/MethodResult.h>
#include <trial/Tasking/Parameter.h>

#include <stdexcept>
#include <stdio.h>


//----------------------------------------------------------------------
//
// ~getPhaseRef
//
// Get the phase reference of the first field.
//
//----------------------------------------------------------------------
void MeqCalibrater::getPhaseRef()
{
  MSField mssub(itsMS.field());
  ROMSFieldColumns mssubc(mssub);
  // Get the phase reference of the first field.
  MDirection dir = mssubc.phaseDirMeasCol()(0)(IPosition(1,0));
  itsPhaseRef = MDirection::Convert (dir, MDirection::J2000)();
}

//----------------------------------------------------------------------
//
// ~getFreq
//
// Get the frequency window for the given data description id
// (giving the spectral window id).
//
//----------------------------------------------------------------------
void MeqCalibrater::getFreq (int ddid)
{
  // Get the frequency domain of the given data descriptor id
  // which gives as the spwid.
  MSDataDescription mssub1(itsMS.dataDescription());
  ROMSDataDescColumns mssub1c(mssub1);
  int spw = mssub1c.spectralWindowId()(ddid);
  MSSpectralWindow mssub(itsMS.spectralWindow());
  ROMSSpWindowColumns mssubc(mssub);
  Vector<double> chanFreq = mssubc.chanFreq()(spw);
  Vector<double> chanWidth = mssubc.chanWidth()(spw);
  // So far, only equal frequency spacings are possible.
  AssertMsg (allEQ (chanWidth, chanWidth(0)),
	     "Channels must have equal spacings");
  itsNrChan    = chanWidth.nelements();
  itsStartFreq = chanFreq(0) - chanWidth(0)/2;
  itsEndFreq   = itsStartFreq + itsNrChan*chanWidth(0);
}

//----------------------------------------------------------------------
//
// ~fillStations
//
// Fill the station positions and names.
//
//----------------------------------------------------------------------
void MeqCalibrater::fillStations()
{
  MSAntenna          mssub(itsMS.antenna());
  ROMSAntennaColumns mssubc(mssub);
  itsStations.reserve (mssub.nrow());
  // Get all stations from the antenna subtable.
  for (uInt i=0; i<mssub.nrow(); i++) {
    // Store each position as a constant parameter.
    // Use the antenna name as the parameter name.
    Vector<Double> antpos = mssubc.position()(i);
    String name = mssubc.name()(i);
    MeqParmSingle* px = new MeqParmSingle ("AntPosX." + name,
					   antpos(0));
    MeqParmSingle* py = new MeqParmSingle ("AntPosY." + name,
					   antpos(1));
    MeqParmSingle* pz = new MeqParmSingle ("AntPosZ." + name,
					   antpos(2));
    itsStations.push_back (MeqStation(px, py, pz, name));
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
void MeqCalibrater::fillBaselines (const Vector<int>& ant1,
				   const Vector<int>& ant2)
{
  MeqDomain  domain;
  MeqRequest req(domain, 1, 1);

  itsBaselines.reserve (ant1.nelements());
  itsBLIndex.resize (itsStations.size(), itsStations.size());
  itsBLIndex = -1;

  for (unsigned int i=0; i<ant1.nelements(); i++)
  {
    uInt a1 = ant1(i);
    uInt a2 = ant2(i);
    // Create an MVBaseline object for each antenna pair.
    Assert (a1 < itsStations.size()  &&  a2 < itsStations.size());
    itsBLIndex(a1,a2) = itsBaselines.size();
    MVPosition pos1
      (itsStations[a1].getPosX()->getResult(req).getValue().getDouble(),
       itsStations[a1].getPosY()->getResult(req).getValue().getDouble(),
       itsStations[a1].getPosZ()->getResult(req).getValue().getDouble());
    MVPosition pos2
      (itsStations[a2].getPosX()->getResult(req).getValue().getDouble(),
       itsStations[a2].getPosY()->getResult(req).getValue().getDouble(),
       itsStations[a2].getPosZ()->getResult(req).getValue().getDouble());

    itsBaselines.push_back (MVBaseline (pos1, pos2));
  }
}

//----------------------------------------------------------------------
//
// ~makeWSRTExpr
//
// Make the expression tree per baseline for the WSRT.
//
//----------------------------------------------------------------------
void MeqCalibrater::makeWSRTExpr()
{
  // Make an expression for each station.
  itsStatExpr.reserve (itsStations.size());
  for (unsigned int i=0; i<itsStations.size(); i++) {
    // Represent the leakage per station by constant parms.
    MeqExpr* stat11 = new MeqParmSingle ("Leakage." + 
					 itsStations[i].getName() + ".11",
					 double(1));
    MeqExpr* stat12 = new MeqParmSingle ("Leakage." + 
					 itsStations[i].getName() + ".12",
					 double(0));
    MeqExpr* stat21 = new MeqParmSingle ("Leakage." + 
					 itsStations[i].getName() + ".21",
					 double(0));
    MeqExpr* stat22 = new MeqParmSingle ("Leakage." + 
					 itsStations[i].getName() + ".22",
					 double(1));
    itsStatExpr.push_back (new MeqJonesNode (stat11, stat12, stat21, stat22));
  }    

  // Get the point sources from the GSM.
  vector<MeqPointSource> sources;
  itsGSM.getPointSources (sources);

  // Make an expression for each baseline.
  itsExpr.reserve (itsUVWPolc.size());
  int nrant = itsBLIndex.nrow();
  for (int ant2=0; ant2<nrant; ant2++) {
    for (int ant1=0; ant1<nrant; ant1++) {
      if (itsBLIndex(ant1,ant2) >= 0) {
	// Create the DFT kernel.
	MeqPointDFT* dft = new MeqPointDFT(sources, itsPhaseRef,
					   itsUVWPolc[itsBLIndex(ant1,ant2)]);
	MeqWsrtPoint* pnt = new MeqWsrtPoint (sources, dft);
	itsExpr.push_back (new MeqWsrtInt (pnt, itsStatExpr[ant1],
					   itsStatExpr[ant2]));
      }
    }
  }
}

//----------------------------------------------------------------------
//
// ~calcUVWPolc
//
// Calculate per baseline the polynomial coefficients for the UVW coordinates
// for the given MS which should be in order of baseline.
//
//----------------------------------------------------------------------
void MeqCalibrater::calcUVWPolc (const Table& ms)
{
  // Create all MeqUVWPolc objects.
  itsUVWPolc.resize (itsBaselines.size());
  for (vector<MeqUVWPolc*>::iterator iter = itsUVWPolc.begin();
       iter != itsUVWPolc.end();
       iter++) {
    *iter = new MeqUVWPolc();
  }
  // The MS is already sorted in order of baseline.
  Block<String> keys(2);
  keys[0] = "ANTENNA1";
  keys[1] = "ANTENNA2";
  // Iterate through the MS per baseline.
  TableIterator iter (ms, keys, TableIterator::Ascending,
		      TableIterator::NoSort);
  // Calculate the uvwpolc for the whole domain of the measurementset
  while (!iter.pastEnd())
  {
    Table tab = iter.table();
    ROScalarColumn<int> ant1col(tab, "ANTENNA1");
    ROScalarColumn<int> ant2col(tab, "ANTENNA2");
    ROArrayColumn<double> uvwcol(tab, "UVW");
    ROScalarColumn<double> timcol(tab, "TIME");
    Vector<double> dt = timcol.getColumn();
    Matrix<double> uvws = uvwcol.getColumn();
    int nrtim = dt.nelements();
    uInt ant1 = ant1col(0);
    uInt ant2 = ant2col(0);
    Assert (ant1 < itsBLIndex.nrow()  &&  ant2 < itsBLIndex.nrow()
	    &&  itsBLIndex(ant1,ant2) >= 0);
    int blindex = itsBLIndex(ant1,ant2);
    // Divide the data into chunks of 1 hour (assuming it is equally spaced).
    // Make all chunks about equally long.
    int nrChunk = max(1, int((dt(nrtim-1) - dt(0)) / 3600 + 0.5));
    int chunkLength = nrtim / nrChunk;
    if (nrChunk > 1) {
      Assert (chunkLength > 4);
    }
    // Loop through the data in periods of one hour.
    for (int nrdone=0; nrdone<nrtim; nrdone+=chunkLength) {
      int todo = min(nrdone+chunkLength, nrtim);
      itsUVWPolc[blindex]->calcCoeff (dt(Slice(nrdone,todo)),
				      uvws(Slice(0,3), Slice(nrdone, todo)));
    }
    iter++;
  }
}

//----------------------------------------------------------------------
//
// ~MeqCalibrater
//
// Constructor. Initialize a MeqCalibrater object.
//
// Calculate UVW polc.
// Create list of stations and list of baselines from MS.
// Create the MeqExpr tree for the WSRT.
//
//----------------------------------------------------------------------
MeqCalibrater::MeqCalibrater(const String& msName,
			     const String& meqModel,
			     const String& skyModel,
			     const uInt    ddid)
  :
  itsMS       (msName, Table::Update),
  itsMSCol    (itsMS),
  itsMEP      (meqModel + ".MEP"),
  itsGSMTable (skyModel + ".GSM"),
  itsGSM      (itsGSMTable),
  itsFitValue (10.0)
{
  cout << "MeqCalibrater constructor (";
  cout << "'" << msName   << "', ";
  cout << "'" << meqModel << "', ";
  cout << "'" << skyModel << "', ";
  cout << ddid << ")" << endl;

  // Get phase reference (for field 0).
  getPhaseRef();

  // We only handle field 0 and the given data desc id (for the time being).
  // Sort the MS in order of baseline.
  Table selMS = itsMS(itsMS.col("FIELD_ID")==0 &&
		      itsMS.col("DATA_DESC_ID")==int(ddid));
  Block<String> keys(2);
  keys[0] = "ANTENNA1";
  keys[1] = "ANTENNA2";
  Table sortMS = selMS.sort (keys);
  // Sort uniquely to get all baselines.
  Table blMS = sortMS.sort (keys, Sort::Ascending,
			    Sort::NoDuplicates + Sort::InsSort);

  // Find all stations (from the ANTENNA subtable of the MS).
  fillStations();
  // Now generate the baseline objects for the baselines.
  // If we ever want to solve for station positions, we cannot use the
  // fixed MVBaseline objects, but instead they should be recalculated
  // after each solve iteration.
  // It also forms an index giving for each (ordered) antenna pair the
  // index in the vector of baselines. -1 means that no baseline exists
  // for that antenna pair.
  ROScalarColumn<int> ant1(blMS, "ANTENNA1");
  ROScalarColumn<int> ant2(blMS, "ANTENNA2");
  Matrix<int> index;
  fillBaselines (ant1.getColumn(), ant2.getColumn());

  // Calculate the UVW polynomial coefficients for each baseline.
  calcUVWPolc (sortMS);

  // Setup the iterator to step through the MS in TIME order.
  itsIter = TableIterator(selMS, "TIME");

  // Set up the expression tree for a single baseline.
  makeWSRTExpr();

  cout << "MeqMat " << MeqMatrixRep::nctor << ' ' << MeqMatrixRep::ndtor
       << ' ' << MeqMatrixRep::nctor + MeqMatrixRep::ndtor << endl;
  cout << "MeqRes " << MeqResultRep::nctor << ' ' << MeqResultRep::ndtor
       << ' ' << MeqResultRep::nctor + MeqResultRep::ndtor << endl;

  // Calculate frequency domain.
  getFreq (ddid);
  cout << "Freq: " << itsStartFreq << ' ' << itsEndFreq << " (" <<
    itsEndFreq - itsStartFreq << " Hz) " << itsNrChan << endl;
}

//----------------------------------------------------------------------
//
// ~~MeqCalibrater
//
// Destructor for a MeqCalibrater object.
//
//----------------------------------------------------------------------
MeqCalibrater::~MeqCalibrater()
{
  itsMS.flush();

  cout << "MeqCalibrater destructor" << endl;
}

//----------------------------------------------------------------------
//
// ~initParms
//
// Initialize all parameters in the expression tree by calling
// its initDomain method.
//
//----------------------------------------------------------------------
void MeqCalibrater::initParms (const MeqDomain& domain)
{
  const vector<MeqParm*>& parmList = MeqParm::getParmList();

  itsNrScid = 0;
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if (*iter) {
      itsNrScid += (*iter)->initDomain (domain, itsNrScid);
    }
  }
}

//----------------------------------------------------------------------
//
// ~setTimeInterval
//
// Set the time domain (interval) for which the solver will solve.
// The predict could be on a smaller domain (but not larger) than
// this domain.
//
//----------------------------------------------------------------------
void MeqCalibrater::setTimeInterval (double secInterval)
{
  cout << "setTimeInterval = " << secInterval << endl;
  itsTimeInterval = secInterval;
}

//----------------------------------------------------------------------
//
// ~resetIterator
//
// Start iteration over time domains from the beginning.
//
//----------------------------------------------------------------------
void MeqCalibrater::resetIterator()
{
  cout << "resetTimeIterator" << endl;
  itsIter.reset();
}

//----------------------------------------------------------------------
//
// ~nextInterval
//
// Move to the next interval (domain).
// Set the request belonging to that.
//
//----------------------------------------------------------------------
bool MeqCalibrater::nextInterval()
{
  itsCurRows.resize(0);
  // Exit when no more chunks.
  if (itsIter.pastEnd()) {
    return false;
  }
  double timeSize = 0;
  double timeStart = 0;
  double timeStep;
  int nrtim = 0;
  // Get the next chunk until the time interval size is exceeded.
  while (timeSize < itsTimeInterval  &&  !itsIter.pastEnd()) {
    ROScalarColumn<double> timcol(itsIter.table(), "TIME");
    ROScalarColumn<double> intcol(itsIter.table(), "INTERVAL");
    // If first time, calculate interval and start time.
    if (timeStart == 0) {
      timeStep  = intcol(0);
      timeStart = timcol(0) - timeStep/2;
    }
    // Check all intervals are equal.
    ////    Assert (allNear (intcol.getColumn(), timeStep, 1.0e-5));
    timeSize += timeStep;
    int nrr = itsCurRows.nelements();
    int nrn = itsIter.table().nrow();
    itsCurRows.resize (nrr+nrn, True);
    itsCurRows(Slice(nrr,nrn)) = itsIter.table().rowNumbers();
    nrtim++;
    itsIter++;
  }
  itsSolveDomain = MeqDomain(timeStart, timeStart + nrtim*timeStep,
			     itsStartFreq, itsEndFreq);
  initParms (itsSolveDomain);
  return true;
}

//----------------------------------------------------------------------
//
// ~clearSolvableParms
//
// Clear the solvable flag on all parms (make them non-solvable).
//
//----------------------------------------------------------------------
void MeqCalibrater::clearSolvableParms()
{
  const vector<MeqParm*>& parmList = MeqParm::getParmList();

  cout << "clearSolvableParms" << endl;

  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if (*iter)
    {
      cout << "clearSolvable: " << (*iter)->getName() << endl;
      (*iter)->setSolvable(false);
    }
  }
}

//----------------------------------------------------------------------
//
// ~setSolvableParm
//
// Set the solvable flag (true or false) on all parameters whose
// name matches the parmPatterns pattern.
//
//----------------------------------------------------------------------
void MeqCalibrater::setSolvableParms (Vector<String>& parmPatterns,
				      Bool isSolvable)
{
  const vector<MeqParm*>& parmList = MeqParm::getParmList();

  cout << "setSolvableParms" << endl;

  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    for (int i=0; i < (int)parmPatterns.nelements(); i++)
    {
      Regex pattern(Regex::fromPattern(parmPatterns[i]));

      if (*iter)
      {
	if (String((*iter)->getName()).matches(pattern))
	{
	  cout << "setSolvable: " << (*iter)->getName() << endl;
	  (*iter)->setSolvable(isSolvable);
	}
      }
    }
  }

  cout << "isSolvable = " << isSolvable << endl;
}

//----------------------------------------------------------------------
//
// ~predict
//
// Predict visibilities for the current time domain.
//
//----------------------------------------------------------------------
void MeqCalibrater::predict()
{
  cout << "predict'" << endl;

  // Loop through all rows in the current solve domain.
  Timer timer;
  for (unsigned int i=0; i<itsCurRows.nelements(); i++) {
    int rownr = itsCurRows(i);
    uInt ant1 = itsMSCol.antenna1()(rownr);
    uInt ant2 = itsMSCol.antenna2()(rownr);
    Assert (ant1 < itsBLIndex.nrow()  &&  ant2 < itsBLIndex.nrow()
	    &&  itsBLIndex(ant1,ant2) >= 0);
    int blindex = itsBLIndex(ant1,ant2);
    double time = itsMSCol.time()(rownr);
    double step = itsMSCol.interval()(rownr);
    MeqDomain domain(time-step/2, time+step/2, itsStartFreq, itsEndFreq);
    MeqRequest request(domain, 1, itsNrChan, itsNrScid);
    itsExpr[blindex]->calcResult (request);
  }
  timer.show();
}

//----------------------------------------------------------------------
//
// ~solve
//
// Solve for the solvable parameters on the current time domain.
//
//----------------------------------------------------------------------
Double MeqCalibrater::solve()
{
  cout << "solve" << endl;

  if (!itsDataRead)
  {
    // read data from itsIter.table()

    itsDataRead = true;
  }
    

  // invoke solver for the current domain

  // update the paramters

#if 0
  // Dummy implementation
  itsFitValue /= 2.0;
#else
  itsFitValue = 0;
#endif

  return itsFitValue;
}

//----------------------------------------------------------------------
//
// ~saveParms
//
// Save parameters which have an updated warning.
//
//----------------------------------------------------------------------
void MeqCalibrater::saveParms()
{
  const vector<MeqParm*>& parmList = MeqParm::getParmList();

  cout << "saveParms" << endl;

  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if (*iter)
    {
      cout << "saveParm: " << (*iter)->getName() << endl;
      (*iter)->save();
    }
  }

  // Save the source parameters.
  // SkyModel writes all sources into the table. So when using the
  // existing table, all sources would be duplicated.
  // Therefore the table is recreated.
  // First get the table description and name.
  TableDesc td = itsGSMTable.tableDesc();
  String name  = itsGSMTable.tableName() + "_saved";
  // Close the table by assigining an empty object to it.
  itsGSMTable = Table();
  // Rrecreate the table and save the parameters.
  SetupNewTable newtab(name, td, Table::New);
  Table tab(newtab);
  itsGSM.store(tab);
  // Make the new table the current one.
  itsGSMTable = tab;
}

//----------------------------------------------------------------------
//
// ~savePredictedData
//
// Save the predicted data for the current time domain in column
// with name modelDataColName.
//
//----------------------------------------------------------------------
void MeqCalibrater::savePredictedData (const String& modelDataColName)
{
  cout << "savePredictedData('" << modelDataColName << "')" << endl;

  ////  itsMS.reopenRW();
  // Add the dataColName column if not existing yet.
  if (! itsMS.tableDesc().isColumn(modelDataColName)) {
    ArrayColumnDesc<Complex> mdcol(modelDataColName);
    itsMS.addColumn (mdcol);
    cout << "Added column " << modelDataColName << " to the MS" << endl;
  }

  ArrayColumn<Complex> mdcol(itsMS, modelDataColName);
  // Loop through all rows in the current solve domain.
  for (unsigned int i=0; i<itsCurRows.nelements(); i++) {
    int rownr = itsCurRows(i);
    uInt ant1 = itsMSCol.antenna1()(rownr);
    uInt ant2 = itsMSCol.antenna2()(rownr);
    Assert (ant1 < itsBLIndex.nrow()  &&  ant2 < itsBLIndex.nrow()
	    &&  itsBLIndex(ant1,ant2) >= 0);
    int blindex = itsBLIndex(ant1,ant2);
    double time = itsMSCol.time()(rownr);
    double step = itsMSCol.interval()(rownr);
    MeqDomain domain(time-step/2, time+step/2, itsStartFreq, itsEndFreq);
    MeqRequest request(domain, 1, itsNrChan, itsNrScid);
    itsExpr[blindex]->calcResult (request);
    // Write the requested data into the MS.
    // Use the same polarizations as for the original data.
    IPosition shp = itsMSCol.data().shape(rownr);
    Assert (shp(1) == itsNrChan);
    Matrix<Complex> data(shp);
    Slice sliceFreq(0, itsNrChan);
    // Store the DComplex results into the Complex data array.
    Array<Complex> tmp0 (data(Slice(0,1), sliceFreq));
    convertArray (tmp0,
      itsExpr[blindex]->getResult11().getValue().getDComplexMatrix());
    if (shp(0) > 2) {
      Array<Complex> tmp1 (data(Slice(1,1), sliceFreq));
      convertArray (tmp1,
	itsExpr[blindex]->getResult12().getValue().getDComplexMatrix());
      Array<Complex> tmp2 (data(Slice(2,1), sliceFreq));
      convertArray (tmp2,
	itsExpr[blindex]->getResult21().getValue().getDComplexMatrix());
      Array<Complex> tmp3 (data(Slice(3,1), sliceFreq));
      convertArray (tmp3,
	itsExpr[blindex]->getResult22().getValue().getDComplexMatrix());
    } else {
      Array<Complex> tmp1 (data(Slice(1,1), sliceFreq));
      convertArray (tmp1,
	itsExpr[blindex]->getResult22().getValue().getDComplexMatrix());
    }
    mdcol.put (rownr, data);
  }
}

//----------------------------------------------------------------------
//
// ~saveResidualData
//
// Save the colA - colB in residualCol.
//
//----------------------------------------------------------------------
void MeqCalibrater::saveResidualData(const String& colAName,
				     const String& colBName,
				     const String& residualColName)
{
  if (colAName == residualColName)
  {
    throw(AipsError("residualcolname can not be the same as colaname"));
  }
  if (colBName == residualColName)
  {
    throw(AipsError("residualcolname can not be the same as colbname"));
  }
  if (residualColName == "DATA")
  {
    throw(AipsError("residualcolname can not be the DATA column"));
  }

  // Make sure the MS is writable.
  itsMS.reopenRW();
  // Create the column if it does not exist yet.
  // Make it an indirect variable shaped column.
  if (! itsMS.tableDesc().isColumn (residualColName)) {
    ArrayColumnDesc<Complex> cdesc(residualColName);
    itsMS.addColumn (cdesc);
    itsMS.flush();
  }

  // Loop through all rows in the MS and store the result.
  ROArrayColumn<Complex> colA(itsMS, colAName);
  ROArrayColumn<Complex> colB(itsMS, colBName);
  ArrayColumn<Complex> colR(itsMS, residualColName);
  for (uInt i=0; i<itsMS.nrow(); i++) {
    colR.put (i, colA(i) - colB(i));
  }
  itsMS.flush();

  cout << "saveResidualData('" << colAName << "', '" << colBName << "', ";
  cout << "'" << residualColName << "')" << endl;
}

//----------------------------------------------------------------------
//
// ~addParm
//
// Add the result of a parameter for the current domain to a GlishRecord
// for the purpose of passing the information back to a glish script.
//
//----------------------------------------------------------------------
static void addParm(MeqParm* parm, GlishRecord* rec)
{
  GlishRecord parmRec;

#if 0

  //
  // In order to fill in the result for the current domain
  // we need some specification of the current domain.
  // The domain should be set in the nextTimeInterval method.
  //

  MeqRequest mr(MeqDomain(0,1,0,1), 1, 1);
  MeqMatrix mm = parm->getResult(mr).getValue();	    
  if (mm.isDouble()) rec->add("result", mm.getDoubleMatrix());
  else               rec->add("result", mm.getDComplexMatrix());
#endif

  parmRec.add("parmid", Int(parm->getParmId()));
  
  rec->add(parm->getName(), parmRec);
}

//----------------------------------------------------------------------
//
// ~getParms
//
// Get a description of the parameters whose name matches the
// parmPatterns pattern. The description shows the result of the
// evaluation of the parameter on the current time domain.
//
//----------------------------------------------------------------------
GlishRecord MeqCalibrater::getParms(Vector<String>& parmPatterns,
				    Vector<String>& excludePatterns)
{
  GlishRecord rec;
  bool parmDone = false;
  vector<MeqParm*> parmVector;

  const vector<MeqParm*>& parmList = MeqParm::getParmList();

  cout << "getParms: " << endl;


  //
  // Find all parms matching the parmPatterns
  //
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    parmDone = false;

    for (int i=0; i < (int)parmPatterns.nelements(); i++)
    {
      Regex pattern(Regex::fromPattern(parmPatterns[i]));

      if (*iter && !parmDone)
      {
	if (String((*iter)->getName()).matches(pattern))
	{
	  parmDone = true;
	  parmVector.push_back(*iter);
	}
      }
    }
  }

  //
  // Make record of parms to return, but exclude parms
  // matching the excludePatterns
  //
  for (vector<MeqParm*>::iterator iter = parmVector.begin();
       iter != parmVector.end();
       iter++)
  {
    if (0 == excludePatterns.nelements())
    {
      if (*iter) addParm((*iter), &rec);
    }
    else
    {
      for (int i=0; i < (int)excludePatterns.nelements(); i++)
      {
	Regex pattern(Regex::fromPattern(excludePatterns[i]));
	
	if (*iter)
	{
	  if (! String((*iter)->getName()).matches(pattern))
	  {
	    addParm((*iter), &rec);
	  }
	}
      }
    }
  }

  return rec;
}

//----------------------------------------------------------------------
//
// ~getSolveDomain
//
// Store the current solve domain in a GlishRecord for the purpose
// of passing it back to the calling glish script.
//
//----------------------------------------------------------------------
GlishRecord MeqCalibrater::getSolveDomain()
{
  GlishRecord rec;
  rec.add("startx", itsSolveDomain.startX());
  rec.add("endx",   itsSolveDomain.endX());
  rec.add("starty", itsSolveDomain.startY());
  rec.add("endy",   itsSolveDomain.endY());
  return rec;
}

//----------------------------------------------------------------------
//
// ~className
//
// Return the name of this DO class.
//
//----------------------------------------------------------------------
String MeqCalibrater::className() const
{
  return "meqcalibrater";
}

//----------------------------------------------------------------------
//
// ~methods
//
// Return a vector of method names for the DO.
//
//----------------------------------------------------------------------
Vector<String> MeqCalibrater::methods() const
{
  Vector<String> method(12);

  method(0)  = "settimeinterval";
  method(1)  = "resetiterator";
  method(2)  = "nextinterval";
  method(3)  = "clearsolvableparms";
  method(4)  = "setsolvableparms";
  method(5)  = "predict";
  method(6)  = "solve";
  method(7)  = "saveparms";
  method(8)  = "savepredicteddata";
  method(9)  = "saveresidualdata";
  method(10) = "getparms";
  method(11) = "getsolvedomain";

  return method;
}

//----------------------------------------------------------------------
//
// ~noTraceMethods
//
// ?
//
//----------------------------------------------------------------------
Vector<String> MeqCalibrater::noTraceMethods() const
{
  return methods();
}

//----------------------------------------------------------------------
//
// ~runMethod
//
// Call a method from glish.
//
//----------------------------------------------------------------------
MethodResult MeqCalibrater::runMethod(uInt which,
				      ParameterSet& inputRecord,
				      Bool runMethod)
{
  switch (which) 
  {
  case 0: // settimeinterval
    {
      Parameter<Int> secondsInterval(inputRecord, "secondsinterval",
				     ParameterSet::In);

      if (runMethod) setTimeInterval(secondsInterval());
    }
    break;

  case 1: // resetiterator
    {
      if (runMethod) resetIterator();
    }
    break;

  case 2: // nextinterval
    {
      Parameter<Bool> returnval(inputRecord, "returnval",
				ParameterSet::Out);
      if (runMethod) returnval() = nextInterval();
    }
    break;

  case 3: // clearsolvableparms
    {
      if (runMethod) clearSolvableParms();
    }
    break;

  case 4: // setsolvableparms
    {
      Parameter<Vector<String> > parmPatterns(inputRecord, "parmpatterns",
					     ParameterSet::In);
      Parameter<Bool> isSolvable(inputRecord, "issolvable",
				 ParameterSet::In);

      if (runMethod) setSolvableParms(parmPatterns(), isSolvable());
    }
    break;

  case 5: // predict
    {
      if (runMethod) predict();
    }
    break;

  case 6: // solve
    {
      Parameter<Double> returnval(inputRecord, "returnval",
				  ParameterSet::Out);

      if (runMethod) returnval() = solve();
    }
    break;

  case 7: // saveparms
    {
      if (runMethod) saveParms();
    }
    break;

  case 8: // savepredicteddata
    {
      Parameter<String> modelDataColName(inputRecord, "modeldatacolname",
					 ParameterSet::In);

      if (runMethod) savePredictedData(modelDataColName());
    }
    break;

  case 9: // saveresidualdata
    {
      Parameter<String> colAName(inputRecord, "colaname",
				 ParameterSet::In);
      Parameter<String> colBName(inputRecord, "colbname",
				 ParameterSet::In);
      Parameter<String> residualColName(inputRecord, "residualcolname",
					ParameterSet::In);

      if (runMethod) saveResidualData(colAName(),
				      colBName(),
				      residualColName());
    }
    break;

  case 10: // getparms
    {
      Parameter<Vector<String> > parmPatterns(inputRecord, "parmpatterns",
					      ParameterSet::In);
      Parameter<Vector<String> > excludePatterns(inputRecord, "excludepatterns",
						 ParameterSet::In);
      Parameter<GlishRecord> returnval(inputRecord, "returnval",
				       ParameterSet::Out);

      if (runMethod) returnval() = getParms(parmPatterns(), excludePatterns());
    }
    break;

  case 11: // getsolvedomain
    {
      Parameter<GlishRecord> returnval(inputRecord, "returnval",
				       ParameterSet::Out);

      if (runMethod) returnval() = getSolveDomain();
    }
    break;

  default:
    return error("No such method");
  }

  return ok();
}

//----------------------------------------------------------------------
//
// ~make
//
// Create an instance of the MeqCalibrater class.
//
//----------------------------------------------------------------------
MethodResult MeqCalibraterFactory::make(ApplicationObject*& newObject,
					const String& whichConstructor,
					ParameterSet& inputRecord,
					Bool runConstructor)
{
  MethodResult retval;
  newObject = 0;

  if (whichConstructor == "meqcalibrater")
    {
      Parameter<String>   msName(inputRecord, "msname",   ParameterSet::In);
      Parameter<String> meqModel(inputRecord, "meqmodel", ParameterSet::In);
      Parameter<String> skyModel(inputRecord, "skymodel", ParameterSet::In);
      Parameter<Int>        ddid(inputRecord, "ddid",     ParameterSet::In);

      if (runConstructor)
	{
	  newObject = new MeqCalibrater(msName(), meqModel(),
					skyModel(), ddid());
	}
    }
  else
    {
      retval = String("Unknown constructor") + whichConstructor;
    }

  if (retval.ok() && runConstructor && !newObject)
    {
      retval = "Memory allocation error";
    }

  return retval;
}
