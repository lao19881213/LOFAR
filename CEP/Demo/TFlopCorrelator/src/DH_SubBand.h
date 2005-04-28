//# DH_SubBand.h: SubBand DataHolder
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//# $Id$

#ifndef TFLOPCORRELATOR_DH_SUBBAND_H
#define TFLOPCORRELATOR_DH_SUBBAND_H


#include <lofar_config.h>

#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>

namespace LOFAR
{


/**
   TBW
*/
class DH_SubBand: public DataHolder
{
public:

  explicit DH_SubBand (const string& name,
		       const short   subband); 


  DH_SubBand(const DH_SubBand&);

  virtual ~DH_SubBand();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// Get write access to the Buffer in the DataPacket.
  fcomplex* getBuffer();
  /// Get access to the Buffer in the DataPacket.
  const fcomplex* getBuffer() const;

  /// return pointer to array containing time/pol series for specified freqchannel and station 
  /// to be used in correlator inner loop

  //todo: define usefull accessors 
  //fcomplex* getBufferTimePolSeries(int channel, int station);

  /// get/set completely specified element in the buffer
  //fcomplex* getBufferElement(int channel, int station, int sample, int polarisation);
  //void setBufferElement(int channel, int station, int sample, int polarisation, fcomplex* value); 

   const unsigned int getBufSize() const;

private:
  /// Forbid assignment.
  DH_SubBand& operator= (const DH_SubBand&);

  fcomplex*  itsBuffer;    // 
  unsigned int itsBufSize;
  
  short itsSubBand;
  short itsNFChannels;
  short itsNStations;      // #stations per buffer
  short itsNTimes;         // #time samples per buffer
  short itsNPol;           // #polarisations per sample

  void fillDataPointers();
};

//exact addressing to be defined 
#define SBADDRESS_STATION (freq, station)       itsNPol*itsNTimes*itsNStations*(station) 
#define SBADDRESS_TIME    (freq, station, time) SBADDRESS_STATION((freq),(station)) +  itsNPol*itsNTimes*(time) 
#define SBADDRESS_POL     (freq, station, time, pol) SBADDRESS_TIME((freq),(station),(time))  +  itsNPol*(pol)
 
 inline fcomplex* DH_SubBand::getBuffer()
   { return itsBuffer; }
 
 inline const fcomplex* DH_SubBand::getBuffer() const
   { return itsBuffer; }
 
/*  inline fcomplex* DH_SubBand::getBufferElement(int channel,  */
/* 					       int station, */
/* 					       int sample, */
/* 					       int pol)      */
/*    { return itsBuffer + SBADDRESS_POL(channel, station, sample, pol); } */
 
/*  fcomplex* getBufferTimePolSeries(int channel, int station)  */
/*    { return itsBuffer + SBADDRESS_STATION(channel, station); }  */
 
/*  inline void DH_SubBand::setBufferElement(int channel,  */
/* 					  int sample,  */
/* 					  int station,  */
/* 					  int polarisation, */
/* 					  fcomplex* valueptr) { */
/*    *(itsBuffer + SBADDRESS_POL(channel, station, sample, pol)) = *valueptr; */
/*  } */
 
 inline const unsigned int DH_SubBand::getBufSize() const {
   return itsBufSize;
 }
 
}

#endif 
