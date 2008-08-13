//# BlobArray.cc: Blob handling for arrays
//#
//# Copyright (C) 2003
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

#ifndef COMMON_BLOBARRAY_TCC
#define COMMON_BLOBARRAY_TCC

#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <Common/TypeNames.h>
#include <Common/LofarLogger.h>

#if defined(HAVE_AIPSPP) 
# include <casa/Arrays/Array.h>
#endif


namespace LOFAR
{

template<typename T>
BlobOStream& putBlobArray (BlobOStream& bs, const T* data,
	                   const uint32* shape, uint16 ndim,
			   bool fortranOrder)
{
  uint32 n = putBlobArrayHeader (bs, true,
				 LOFAR::typeName((const T**)0),
				 shape, ndim, fortranOrder, 1);
  putBlobArrayData (bs, data, n);
  bs.putEnd();
  return bs;
}


template<typename T>
uint setSpaceBlobArray2 (BlobOStream& bs, bool useBlobHeader,
			 uint32 size0, uint32 size1,
			 bool fortranOrder, uint alignment)
{
  uint32 shp[2];
  shp[0] = size0;
  shp[1] = size1;
  return setSpaceBlobArray<T> (bs, useBlobHeader, shp, 2, fortranOrder,
			       alignment);
}
template<typename T>
uint setSpaceBlobArray3 (BlobOStream& bs,  bool useBlobHeader,
			 uint32 size0, uint32 size1, uint32 size2,
			 bool fortranOrder, uint alignment)
{
  uint32 shp[3];
  shp[0] = size0;
  shp[1] = size1;
  shp[2] = size2;
  return setSpaceBlobArray<T> (bs, useBlobHeader, shp, 3, fortranOrder,
			       alignment);
}
template<typename T>
uint setSpaceBlobArray4 (BlobOStream& bs, bool useBlobHeader,
			 uint32 size0, uint32 size1,
			 uint32 size2, uint32 size3,
			 bool fortranOrder, uint alignment)
{
  uint32 shp[4];
  shp[0] = size0;
  shp[1] = size1;
  shp[2] = size2;
  shp[3] = size3;
  return setSpaceBlobArray<T> (bs, useBlobHeader, shp, 4, fortranOrder,
			       alignment);
}
template<typename T>
uint setSpaceBlobArray (BlobOStream& bs, bool useBlobHeader,
			const std::vector<uint32>& shape,
			bool fortranOrder, uint alignment)
{
  return setSpaceBlobArray<T> (bs, useBlobHeader, &shape[0], shape.size(),
			       fortranOrder, alignment);
}

template<typename T>
uint setSpaceBlobArray (BlobOStream& bs, bool useBlobHeader,
			const uint32* shape, uint16 ndim,
			bool fortranOrder, uint alignment)
{
  // Default alignment is the size of an array element with a maximum of 8.
  if (alignment == 0) {
    alignment = std::min((size_t) 8, sizeof(T));
  }
  uint32 n = putBlobArrayHeader (bs, useBlobHeader,
				 LOFAR::typeName((const T**)0),
				 shape, ndim, fortranOrder, alignment);
  uint pos = bs.setSpace (n*sizeof(T));
  if (useBlobHeader) {
    bs.putEnd();
  }
  return pos;
}


#if defined(HAVE_BLITZ) 
template<typename T, uint NDIM>
BlobOStream& operator<< (BlobOStream& bs, const blitz::Array<T,NDIM>& arr)
{
  if (arr.isStorageContiguous()) {
    putBlobArray (bs, arr.dataFirst(), arr.shape().data(), NDIM,
                  arr.isMinorRank(0));
  } else {
    blitz::Array<T,NDIM> arrc(arr.copy());
    putBlobArray (bs, arrc.dataFirst(), arrc.shape().data(), NDIM,
                  arrc.isMinorRank(0));
  }
  return bs;
}

template<typename T, uint NDIM>
BlobIStream& operator>> (BlobIStream& bs, blitz::Array<T,NDIM>& arr)
{
  bs.getStart (LOFAR::typeName((const T**)0));
  bool fortranOrder;
  uint16 ndim;
  uint nalign = getBlobArrayStart (bs, fortranOrder, ndim);
  ASSERT (ndim == NDIM);
  blitz::TinyVector<uint32,NDIM> shape;
  getBlobArrayShape (bs, shape.data(), NDIM, fortranOrder!=arr.isMinorRank(),
		     nalign);
  arr.resize (shape);
  if (arr.isStorageContiguous()) {
    getBlobArrayData (bs, arr.dataFirst(), arr.numElements());
  } else {
    blitz::Array<T,NDIM>& arrc(shape);
    getBlobArrayData (bs, arrc.dataFirst(), arrc.numElements());
    arr = arrc;
  }
  bs.getEnd();
  return bs;
}
#endif


#if defined(HAVE_AIPSPP) 
template<typename T>
BlobOStream& operator<< (BlobOStream& bs, const casa::Array<T>& arr)
{
  bool deleteIt;
  const T* data = arr.getStorage(deleteIt);
  vector<uint32> shp(arr.ndim());
  for (uint i=0; i<arr.ndim(); i++) {
    shp[i] = arr.shape()[i];
  }
  putBlobArray (bs, data, &shp[0], arr.ndim(), true);
  arr.freeStorage (data, deleteIt);
  return bs;
}

template<typename T>
BlobIStream& operator>> (BlobIStream& bs, casa::Array<T>& arr)
{
  bs.getStart (LOFAR::typeName((const T**)0));
  bool fortranOrder;
  uint16 ndim;
  uint nalign = getBlobArrayStart (bs, fortranOrder, ndim);
  vector<uint32> shp(ndim);
  getBlobArrayShape (bs, &shp[0], ndim, !fortranOrder, nalign);
  casa::IPosition shape(ndim);
  for (uint i=0; i<ndim; i++) {
    shape[i] = shp[i];
  }
  arr.resize (shape);
  bool deleteIt;
  T* data = arr.getStorage(deleteIt);
  getBlobArrayData (bs, data, arr.nelements());
  arr.putStorage (data, deleteIt);
  bs.getEnd();
  return bs;
}
#endif


template<typename T>
BlobIStream& getBlobVector (BlobIStream& bs, T*& arr, uint32& size)
{
  bs.getStart (LOFAR::typeName((const T**)0));
  bool fortranOrder;
  uint16 ndim;
  uint nalign = getBlobArrayStart (bs, fortranOrder, ndim);
  ASSERT (ndim == 1);
  getBlobArrayShape (bs, &size, 1, false, nalign);
  arr = new T[size];
  getBlobArrayData (bs, arr, size);
  bs.getEnd();
  return bs;
}

template<typename T>
BlobIStream& getBlobArray (BlobIStream& bs, T*& arr,
	                   std::vector<uint32>& shape, bool fortranOrder)
{
  bs.getStart (LOFAR::typeName((const T**)0));
  bool fortranOrder1;
  uint16 ndim;
  uint nalign = getBlobArrayStart (bs, fortranOrder1, ndim);
  shape.resize (ndim);
  uint n = getBlobArrayShape (bs, &shape[0], ndim,
			      fortranOrder!=fortranOrder1, nalign);
  arr = new T[n];
  getBlobArrayData (bs, arr, n);
  bs.getEnd();
  return bs;
}

template<typename T>
uint getSpaceBlobArray (BlobIStream& bs, bool useBlobHeader,
			std::vector<uint32>& shape,
			bool fortranOrder)
{
  if (useBlobHeader) {
    bs.getStart (LOFAR::typeName((const T**)0));
  }
  bool fortranOrder1;
  uint16 ndim;
  uint nalign = getBlobArrayStart (bs, fortranOrder1, ndim);
  shape.resize (ndim);
  uint n = getBlobArrayShape (bs, &shape[0], ndim,
			      fortranOrder!=fortranOrder1, nalign);
  uint pos = bs.getSpace (n*sizeof(T));
  if (useBlobHeader) {
    bs.getEnd();
  }
  return pos;
}

template<typename T>
BlobIStream& operator>> (BlobIStream& bs, std::vector<T>& arr)
{
  bs.getStart (LOFAR::typeName((const T**)0));
  bool fortranOrder;
  uint16 ndim;
  uint nalign = getBlobArrayStart (bs, fortranOrder, ndim);
  ASSERT (ndim == 1);
  uint32 size;
  getBlobArrayShape (bs, &size, 1, false, nalign);
  arr.resize (size);
  getBlobArrayData (bs, &(arr[0]), size);
  bs.getEnd();
  return bs;
}


template<typename T>
void putBlobArrayData (BlobOStream& bs, const T* data, uint nr)
{
  for (uint i=0; i<nr; i++) {
    bs << data[i];
  }
}

template<typename T>
void getBlobArrayData (BlobIStream& bs, T* data, uint nr)
{
  for (uint i=0; i<nr; i++) {
    bs >> data[i];
  }
}

} // end namespace LOFAR

#endif
