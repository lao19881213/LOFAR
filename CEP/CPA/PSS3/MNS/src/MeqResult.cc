//# MeqResult.cc: The result of an expression for a domain.
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

//# Includes
#include <MNS/MeqResult.h>
#include <MNS/MeqMatrixTmp.h>

int MeqResultRep::nctor = 0;
int MeqResultRep::ndtor = 0;

MeqResultRep::MeqResultRep (int nspid)
: itsCount           (0),
  itsDefPert         (0.),
  itsPerturbedValues (nspid),
  itsPerturbation    (nspid)
{
  nctor++;
}

MeqResultRep::~MeqResultRep()
{
  for (unsigned int i=0; i<itsPerturbedValues.size(); i++) {
    delete itsPerturbedValues[i];
    delete itsPerturbation[i];
  }
  ndtor--;
}

void MeqResultRep::unlink (MeqResultRep* rep)
{
  if (rep != 0  &&  --rep->itsCount == 0) {
    delete rep;
  }
}

void MeqResultRep::setValue (const MeqMatrix& value)
{
  itsValue = value;
}
  
void MeqResultRep::setPerturbedValue (int i, const MeqMatrix& value)
{
  if (itsPerturbedValues[i] == 0) {
    itsPerturbedValues[i] = new MeqMatrix();
  }
  *(itsPerturbedValues[i]) = value;
}

void MeqResultRep::show (ostream& os) const
{
  os << "Value: " << itsValue << endl;
  for (unsigned int i=0; i<itsPerturbedValues.size(); i++) {
    if (isDefined(i)) {
      os << "deriv parm " << i << " with " << *(itsPerturbation[i]) << endl;
      os << "   " << (*(itsPerturbedValues[i]) - itsValue) << endl;
      os << "   " << (*(itsPerturbedValues[i]) - itsValue) /
	 *(itsPerturbation[i]) << endl;
    }
  }
}




MeqResult::MeqResult (int nspid)
{
  itsRep = new MeqResultRep(nspid);
  itsRep->link();
}

MeqResult::MeqResult (const MeqResult& that)
: itsRep (that.itsRep)
{
  if (itsRep != 0) {
    itsRep->link();
  }
}
MeqResult& MeqResult::operator= (const MeqResult& that)
{
  MeqResultRep::unlink (itsRep);
  itsRep = that.itsRep;
  if (itsRep != 0) {
    itsRep->link();
  }
  return *this;
}
