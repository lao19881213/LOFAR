//# tSaxpy.cc: Program to test performance of std::complex
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <iostream>
#include <sstream>
#include <complex>
#include <Common/Timer.h>

using namespace LOFAR;
using namespace std;

// Generic Saxpy function
template<class T, class U>
void saxpy1(int n, const T* x, const U a, T* y)
{
   NSTimer tim;
   tim.start();
   for (int i=0; i<n; i++)
   {
     (*(y++))+=a*(*(x++));
   }
   tim.stop();
   cout<< tim;
}
template<class T, class U>
void saxpy2(int n, const T* x, const U a, T* y)
{
   NSTimer tim;
   tim.start();
   for (int i=0; i<n; i++)
   {
     y[i]+=a*x[i];
   }
   tim.stop();
   cout<< tim;
}

template<class T, class U>
void saxpy(int n, T val)
{
  T* xp = new T[n];
  T* yp = new T[n];
  fill (xp, xp+n, val);
  fill (yp, yp+n, val);
  U u = U();
  cout << xp << ' ' << yp << endl;
  saxpy1 (n, xp, u, yp);
  saxpy2 (n, xp, u, yp);
  delete xp;
  delete yp;
}

int main (int argc, const char* argv[])
{
  int n = 2048;
  if (argc > 1) {
    istringstream istr(argv[1]);
    istr >> n;
  }
  cout<< "float,float " << n << endl;
  saxpy<float,float> (n, 0);
  cout<< "__complex__ float,float " << n << endl;
  saxpy<__complex__ float,float> (n, 0.+0.i);
  cout<< "std::complex<float>,float " << n << endl;
  saxpy<std::complex<float>,float> (n, std::complex<float>());
  cout<< "double,double " << n << endl;
  saxpy<double,double> (n, 0);
  cout<< "__complex__ double,double " << n << endl;
  saxpy<__complex__ double,double> (n, 0.+0.i);
  cout<< "std::complex<double>,double " << n << endl;
  saxpy<std::complex<double>,double> (n, std::complex<double>());
  cout<< "std::complex<double>,std::complex<double> " << n << endl;
  saxpy<std::complex<double>,std::complex<double> > (n, std::complex<double>());
  return 0;
}
