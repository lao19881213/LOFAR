//#  WH_AWE.cc:
//#
//#  Copyright (C) 2002
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
//#  Chris Broekema, november 2002.
//#
//#  $Id$
//#

#include <stdio.h>             // for sprintf
#include <blitz/blitz.h>

#include <StationSim/WH_AWE.h>
#include <BaseSim/ParamBlock.h>
#include <Common/Debug.h>
#include <Common/lofar_mdl.h>
#include <Common/lofar_pastd.h>
#include <Common/Lorrays.h>
#include <Math/LCSMath.h>

WH_AWE::WH_AWE (const string& name, unsigned int nin, unsigned int nout,
		unsigned int nant, unsigned int buflength)
: WorkHolder    (nin, nout, name, "WH_AWE"),
  itsInHolder   ("in", nant, buflength),
  itsOutHolder  ("out"),
  itsNrcu       (nant),
  itsBufLength  (buflength)
{

  // Read the dipole positions from file
  cerr << "Reading array configuration\n";
  std::ifstream arrfile(itsDipoleName.c_str());
  FailWhen1( !arrfile,"can't open dipole file" );
  int n;
  arrfile >> n;
  dprintf1(1)("Expecting %d antennas\n",n);
  px.resize(n);
  py.resize(n);
  arrfile >> px;
  arrfile >> py;
  dprintf1(1)("Read %d antenna coordinates\n",px.size());
  arrfile.close();
  cout << px << endl;
  cout << py << endl;
}

WH_AWE::~WH_AWE()
{
}

WorkHolder* WH_AWE::construct (const string& name,
			       int ninput, int noutput,
			       const ParamBlock& params)
{
  Assert (ninput == 1);
  return new WH_AWE (name, ninput, noutput, 
		     params.getInt ("nant", 10), params.getInt ("buffer_length", 100));
}

WH_AWE* WH_AWE::make (const string& name) const
{
  return new WH_AWE (name, getInputs(), getOutputs(), itsNrcu, itsBufLength);
}

void WH_AWE::preprocess()
{
  // Find the snapshot array for the current AWE snapshot
  itsBuffer.resize(itsNrcu, itsBufLength);
  itsBuffer = -1;

  DH_SampleC::BufferType* bufin = itsInHolder.getBuffer();

  itsBuffer = *bufin;

}


void WH_AWE::process()
{
  /// REMOVE ME!!
  double phi = 0.33;
  double theta = -0.67;

  // Select the appropriate algorithm
  // See if the PASTd algorithm need updating
  // Use either EVD or SVD for updating

  // EVD - first calculate the ACM
  LoMat_dcomplex itsAcm (itsNrcu, itsNrcu) ;
  itsAcm = LCSMath::acm(itsBuffer);
  
  // EVD - using the ACM, calculate eigen vectors and values.
  LoMat_dcomplex itsEvectors;
  LoVec_double   itsEvalues;
  LCSMath::eig(itsAcm, itsEvectors, itsEvalues);

  // EVD - Determine the number of sources in the signal
  // TODO: fit MDL into Common library.
  unsigned int RFI = mdl(itsEvalues, itsNrcu, itsBufLength);
  
  // EVD - Find the appropriate Eigen vectors
  // Assume the most powerfull sources are in front. This may not be true!!
  // TODO check if this is a good assumption -- possible overflow
  LoMat_dcomplex B(itsNrcu, RFI);
  B = itsEvectors(Range::all(), Range(itsEvectors.lbound(firstDim), 
				     itsEvectors.lbound(firstDim) + RFI - 1));

  LoVec_dcomplex w (itsNrcu) ; // the weight vector
  dcomplex *w_ptr = w.data() ;

  LoVec_dcomplex d(itsNrcu);
  d = steerv(phi, theta, px, py); 
  w = getWeights(B, d);
  
  // Now assign the calculated weight vector to the output
  memcpy(itsOutHolder.getBuffer(), w_ptr, itsNrcu * sizeof(LoVec_dcomplex));
  
  
  cout << itsBuffer << endl;
  cout << itsAcm << endl;

  

  // SVD 

  // PASTd
 
}

void WH_AWE::dump() const
{

}


DH_SampleC* WH_AWE::getInHolder (int channel)
{
  return &itsInHolder;
}

DH_Weight* WH_AWE::getOutHolder (int channel)
{
  AssertStr (channel < getOutputs(),
	     "output channel too high");
  return &itsOutHolder;
}

LoVec_dcomplex WH_AWE::steerv (double phi, double theta, LoVec_double px, LoVec_double py) {
  
  FailWhen1( px.size() != py.size(),"vector size mismatch" );
  LoVec_dcomplex res( px.size() );
  dcomplex i = dcomplex (0,1);

  res = i * -2*M_PI*( px*sin(theta)*cos(phi) + py*sin(theta)*sin(phi) );
  
  return res;
}

LoVec_dcomplex WH_AWE::getWeights (LoVec_dcomplex B, LoVec_dcomplex d) {
  LoVec_dcomplex w (B.size());
  
  if (B.size() != d.size()) {
    cout << "Error. WH_AWE::getWeights() encountered non equal size arrays" << endl;
  } else {
    LoVec_dcomplex temp(B.size());
    
	//    temp = LCSMath::matMult(B, 1/LCSMath::matMultReduce(B, B)) ;
	//    w = d - LCSMath::matMult( LCSMath::matMult( B, temp ), d );
  }
  return w;
}

LoVec_dcomplex WH_AWE::getWeights (LoMat_dcomplex B, LoVec_dcomplex d) {
  LoVec_dcomplex w (B.rows());
  
  if (B.rows() != d.size()) {
    cout << "Error. WH_AWE::getWeights() encountered non equal size arrays" << endl;
  } else {
    LoMat_dcomplex temp(B.cols(),B.rows());

    temp = LCSMath::matMult(pow(LCSMath::matMult(B.transpose(firstDim, secondDim), B), -1), 
				    B.transpose(firstDim,secondDim)) ;

    w = d - LCSMath::matMult( LCSMath::matMult( B, temp ), d );
  }
  return w;
}
