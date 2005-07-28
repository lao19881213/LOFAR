//# MeqMatrixRealArr.cc: Temporary matrix for Mns
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

#undef TIMER

#include <lofar_config.h>
#include <BBS3/MNS/MeqMatrixRealSca.h>
#include <BBS3/MNS/MeqMatrixRealArr.h>
#include <BBS3/MNS/MeqMatrixComplexSca.h>
#include <BBS3/MNS/MeqMatrixComplexArr.h>
#include <Common/LofarLogger.h>
#include <casa/BasicSL/Constants.h>
#include <cmath>

#if defined TIMER
#include <Common/Timer.h>
#endif

using namespace casa;

namespace LOFAR {

MeqMatrixRealArr::MeqMatrixRealArr (int nx, int ny)
: MeqMatrixRep (nx, ny, RealArray)
{
  itsValue = new double[nelements()];
}

MeqMatrixRealArr::~MeqMatrixRealArr()
{
  delete [] itsValue;
}

MeqMatrixRep* MeqMatrixRealArr::clone() const
{
#if defined TIMER
  static NSTimer timer("clone RA", true);
  timer.start();
#endif

  MeqMatrixRealArr* v = new MeqMatrixRealArr (nx(), ny());
  memcpy (v->itsValue, itsValue, sizeof(double) * nelements());

#if defined TIMER
  timer.stop();
#endif

  return v;
}

void MeqMatrixRealArr::set (double value)
{
#if defined TIMER
  static NSTimer timer("set RA", true);
  timer.start();
#endif

  for (int i=0; i<nelements(); i++) {
    itsValue[i] = value;
  }

#if defined TIMER
  timer.stop();
#endif
}

void MeqMatrixRealArr::show (ostream& os) const
{
  os << '[';
  for (int i=0; i<nelements(); i++) {
    if (i > 0) {
      os << ", ";
    }
    os << itsValue[i];
  }
  os << ']';
}

MeqMatrixRep* MeqMatrixRealArr::add (MeqMatrixRep& right, bool rightTmp)
{
  return right.addRep (*this, rightTmp);
}
MeqMatrixRep* MeqMatrixRealArr::subtract (MeqMatrixRep& right, bool rightTmp)
{
  return right.subRep (*this, rightTmp);
}
MeqMatrixRep* MeqMatrixRealArr::multiply (MeqMatrixRep& right, bool rightTmp)
{
  return right.mulRep (*this, rightTmp);
}
MeqMatrixRep* MeqMatrixRealArr::divide (MeqMatrixRep& right, bool rightTmp)
{
  return right.divRep (*this, rightTmp);
}
MeqMatrixRep* MeqMatrixRealArr::posdiff (MeqMatrixRep& right)
{
  return right.posdiffRep (*this);
}
MeqMatrixRep* MeqMatrixRealArr::tocomplex (MeqMatrixRep& right)
{
  return right.tocomplexRep (*this);
}

const double* MeqMatrixRealArr::doubleStorage() const
{
  return itsValue;
}
double MeqMatrixRealArr::getDouble (int x, int y) const
{
  return itsValue[offset(x,y)];
}
dcomplex MeqMatrixRealArr::getDComplex (int x, int y) const
{
  return makedcomplex(itsValue[offset(x,y)], 0);
}

#define MNSMATRIXREALARR_OP(NAME, OP, OP2) \
MeqMatrixRep* MeqMatrixRealArr::NAME (MeqMatrixRealSca& left, \
				      bool rightTmp) \
{ \
  MeqMatrixRealArr* v = this; \
  if (!rightTmp) { \
    v = (MeqMatrixRealArr*)clone(); \
  } \
  double* value = v->itsValue; \
  double lvalue = left.itsValue; \
  double* end = value + nelements(); \
  while (value < end) { \
    *value = lvalue OP2 *value; \
    value++; \
  } \
  return v; \
} \
MeqMatrixRep* MeqMatrixRealArr::NAME (MeqMatrixRealArr& left,  \
				      bool) \
{ \
  ASSERT (nelements() == left.nelements()); \
  double* value = left.itsValue; \
  double* value2 = itsValue; \
  double* end = value + nelements(); \
  while (value < end) { \
    *value++ OP *value2++; \
  } \
  return &left; \
}

MNSMATRIXREALARR_OP(addRep,+=,+);
MNSMATRIXREALARR_OP(subRep,-=,-);
MNSMATRIXREALARR_OP(mulRep,*=,*);
MNSMATRIXREALARR_OP(divRep,/=,/);

MeqMatrixRep* MeqMatrixRealArr::addRep(MeqMatrixComplexSca& left, bool)
{
#if defined TIMER
  static NSTimer timer("add RA CS", true);
  timer.start();
#endif

  MeqMatrixComplexArr* v = MeqMatrixComplexArr::allocate (nx(), ny());
  double left_r = real(left.itsValue), left_i = imag(left.itsValue);
  int n = nelements();
  for (int i=0; i<n; i++) {
    v->itsReal[i] = left_r + itsValue[i];
    v->itsImag[i] = left_i;
  }

#if defined TIMER
  timer.stop();
#endif
  
  return v;
}

MeqMatrixRep* MeqMatrixRealArr::addRep(MeqMatrixComplexArr& left, bool)
{
#if defined TIMER
  static NSTimer timer("add RA CA", true);
  timer.start();
#endif

  int n = left.nelements();
  for (int i=0; i<n; i++) {
    left.itsReal[i] += itsValue[i];
  }

#if defined TIMER
  timer.stop();
#endif
  
  return &left;
}

MeqMatrixRep* MeqMatrixRealArr::subRep(MeqMatrixComplexSca& left, bool)
{
#if defined TIMER
  static NSTimer timer("sub RA CS", true);
  timer.start();
#endif

  MeqMatrixComplexArr* v = MeqMatrixComplexArr::allocate (nx(), ny());
  double left_r = real(left.itsValue), left_i = imag(left.itsValue);
  int n = nelements();
  for (int i=0; i<n; i++) {
    v->itsReal[i] = left_r - itsValue[i];
    v->itsImag[i] = left_i;
  }

#if defined TIMER
  timer.stop();
#endif
  
  return v;
}

MeqMatrixRep* MeqMatrixRealArr::subRep(MeqMatrixComplexArr& left, bool)
{
#if defined TIMER
  static NSTimer timer("sub RA CA", true);
  timer.start();
#endif

  int n = left.nelements();
  for (int i=0; i<n; i++) {
    left.itsReal[i] -= itsValue[i];
  }

#if defined TIMER
  timer.stop();
#endif
  
  return &left;
}

MeqMatrixRep* MeqMatrixRealArr::mulRep(MeqMatrixComplexSca& left, bool)
{
#if defined TIMER
  static NSTimer timer("mul RA CS", true);
  timer.start();
#endif

  MeqMatrixComplexArr* v = MeqMatrixComplexArr::allocate (nx(), ny());
  double left_r = real(left.itsValue), left_i = imag(left.itsValue);
  int n = nelements();
  for (int i=0; i<n; i++) {
    v->itsReal[i] = left_r * itsValue[i];
    v->itsImag[i] = left_i * itsValue[i];
  }

#if defined TIMER
  timer.stop();
#endif
  
  return v;
}

MeqMatrixRep* MeqMatrixRealArr::mulRep(MeqMatrixComplexArr& left, bool)
{
#if defined TIMER
  static NSTimer timer("mul RA CA", true);
  timer.start();
#endif

  int n = left.nelements();
  for (int i=0; i<n; i++) {
    left.itsReal[i] *= itsValue[i];
    left.itsImag[i] *= itsValue[i];
  }

#if defined TIMER
  timer.stop();
#endif
  
  return &left;
}

MeqMatrixRep* MeqMatrixRealArr::divRep(MeqMatrixComplexSca& left, bool)
{
#if defined TIMER
  static NSTimer timer("div RA CS", true);
  timer.start();
#endif

  MeqMatrixComplexArr* v = MeqMatrixComplexArr::allocate (nx(), ny());
  double left_r = real(left.itsValue), left_i = imag(left.itsValue);
  int n = nelements();
  for (int i=0; i<n; i++) {
    double tmp = 1.0 / itsValue[i];
    v->itsReal[i] = left_r * tmp;
    v->itsImag[i] = left_i * tmp;
  }

#if defined TIMER
  timer.stop();
#endif
  
  return v;
}

MeqMatrixRep* MeqMatrixRealArr::divRep(MeqMatrixComplexArr& left, bool)
{
#if defined TIMER
  static NSTimer timer("div RA CA", true);
  timer.start();
#endif

  int n = left.nelements();
  for (int i=0; i<n; i++) {
    double tmp = 1.0 / itsValue[i];
    left.itsReal[i] *= tmp;
    left.itsImag[i] *= tmp;
  }

#if defined TIMER
  timer.stop();
#endif
  
  return &left;
}

MeqMatrixRep* MeqMatrixRealArr::posdiffRep (MeqMatrixRealSca& left)
{
#if defined TIMER
  static NSTimer timer("posdiffRep RA RS", true);
  timer.start();
#endif

  MeqMatrixRealArr* v = new MeqMatrixRealArr (nx(), ny());
  double* value = v->itsValue;
  double* rvalue = itsValue;
  double  lvalue = left.itsValue;
  int n = left.nelements();
  for (int i=0; i<n; i++) {
    double diff = lvalue - rvalue[i];
    if (diff < -1 * C::pi) {
      diff += C::_2pi;
    }
    if (diff > C::pi) {
      diff -= C::_2pi;
    }
    value[i] = diff;
  }

#if defined TIMER
  timer.stop();
#endif
  
  return v;
}
MeqMatrixRep* MeqMatrixRealArr::posdiffRep (MeqMatrixRealArr& left)
{
#if defined TIMER
  static NSTimer timer("posdiffRep RA RA", true);
  timer.start();
#endif

  ASSERT (nelements() == left.nelements());
  MeqMatrixRealArr* v = new MeqMatrixRealArr (nx(), ny());
  double* value = v->itsValue;
  double* rvalue = itsValue;
  double* lvalue = left.itsValue;
  int n = left.nelements();
  for (int i=0; i<n; i++) {
    double diff = lvalue[i] - rvalue[i];
    if (diff < -1 * C::pi) {
      diff += C::_2pi;
    }
    if (diff > C::pi) {
      diff -= C::_2pi;
    }
    value[i] = diff;
  }

#if defined TIMER
  timer.stop();
#endif
  
  return v;
}

MeqMatrixRep* MeqMatrixRealArr::tocomplexRep (MeqMatrixRealSca& left)
{
#if defined TIMER
  static NSTimer timer("tocomplex RA RS", true);
  timer.start();
#endif

  MeqMatrixComplexArr* v = MeqMatrixComplexArr::allocate (nx(), ny());
  double* rvalue = itsValue;
  double  lvalue = left.itsValue;
  int n = nelements();
  for (int i=0; i<n; i++) {
    v->itsReal[i] = lvalue;
    v->itsImag[i] = rvalue[i];
  }

#if defined TIMER
  timer.stop();
#endif
  
  return v;
}
MeqMatrixRep* MeqMatrixRealArr::tocomplexRep (MeqMatrixRealArr& left)
{
#if defined TIMER
  static NSTimer timer("tocomplex RA RA", true);
  timer.start();
#endif

  ASSERT (nelements() == left.nelements());
  MeqMatrixComplexArr* v = MeqMatrixComplexArr::allocate (nx(), ny());
  double* rvalue = itsValue;
  double* lvalue = left.itsValue;
  int n = nelements();
  for (int i=0; i<n; i++) {
    v->itsReal[i] = lvalue[i];
    v->itsImag[i] = rvalue[i];
  }

#if defined TIMER
  timer.stop();
#endif
  
  return v;
}


MeqMatrixRep* MeqMatrixRealArr::negate()
{
#if defined TIMER
  static NSTimer timer("negate RA", true);
  timer.start();
#endif

  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = -(itsValue[i]);
  }

#if defined TIMER
  timer.stop();
#endif
  
  return this;
}

MeqMatrixRep* MeqMatrixRealArr::sin()
{
#if defined TIMER
  static NSTimer timer("sin RA", true);
  timer.start();
#endif

  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = std::sin(itsValue[i]);
  }

#if defined TIMER
  timer.stop();
#endif
  
  return this;
}

MeqMatrixRep* MeqMatrixRealArr::cos()
{
#if defined TIMER
  static NSTimer timer("cos RA", true);
  timer.start();
#endif

  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = std::cos(itsValue[i]);
  }

#if defined TIMER
  timer.stop();
#endif
  
  return this;
}

MeqMatrixRep* MeqMatrixRealArr::exp()
{
#if defined TIMER
  static NSTimer timer("exp RA", true);
  timer.start();
#endif

  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = std::exp(itsValue[i]);
  }

#if defined TIMER
  timer.stop();
#endif
  
  return this;
}

MeqMatrixRep* MeqMatrixRealArr::sqr()
{
#if defined TIMER
  static NSTimer timer("sqr RA", true);
  timer.start();
#endif

  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] *= itsValue[i];
  }

#if defined TIMER
  timer.stop();
#endif
  
  return this;
}

MeqMatrixRep* MeqMatrixRealArr::sqrt()
{
#if defined TIMER
  static NSTimer timer("sqrt RA", true);
  timer.start();
#endif

  int n = nelements();
  for (int i=0; i<n; i++) {
    itsValue[i] = std::sqrt(itsValue[i]);
  }

#if defined TIMER
  timer.stop();
#endif
  
  return this;
}

MeqMatrixRep* MeqMatrixRealArr::conj()
{
  return this;
}

MeqMatrixRep* MeqMatrixRealArr::min()
{
#if defined TIMER
  static NSTimer timer("min RA", true);
  timer.start();
#endif

  double val = 0;
  int n = nelements();
  if (n > 0) {
    val = itsValue[0];
    for (int i=1; i<n; i++) {
      if (itsValue[i] < val) {
	val = itsValue[i];
      }
    }
  }
  MeqMatrixRep *result = new MeqMatrixRealSca (val);

#if defined TIMER
  timer.stop();
#endif
  
  return result;
}


MeqMatrixRep* MeqMatrixRealArr::max()
{
#if defined TIMER
  static NSTimer timer("max RA", true);
  timer.start();
#endif

  double val = 0;
  int n = nelements();
  if (n > 0) {
    val = itsValue[0];
    for (int i=1; i<n; i++) {
      if (itsValue[i] > val) {
	val = itsValue[i];
      }
    }
  }
  MeqMatrixRep *result = new MeqMatrixRealSca (val);

#if defined TIMER
  timer.stop();
#endif
  
  return result;
}


MeqMatrixRep* MeqMatrixRealArr::mean()
{
#if defined TIMER
  static NSTimer timer("mean RA", true);
  timer.start();
#endif

  double sum = 0;
  int n = nelements();
  for (int i=0; i<n; i++) {
    sum += itsValue[i];
  }
  MeqMatrixRep *result = new MeqMatrixRealSca (sum/n);

#if defined TIMER
  timer.stop();
#endif
  
  return result;
}


MeqMatrixRep* MeqMatrixRealArr::sum()
{
#if defined TIMER
  static NSTimer timer("sum RA", true);
  timer.start();
#endif

  double sum = 0;
  int n = nelements();
  for (int i=0; i<n; i++) {
    sum += itsValue[i];
  }
  MeqMatrixRep *result = new MeqMatrixRealSca (sum);

#if defined TIMER
  timer.stop();
#endif
  
  return result;
}

}
