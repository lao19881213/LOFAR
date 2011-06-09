//# MSWriterHDF5: an implementation of MSWriter using the DAL to weite HDF5
//#
//#  Copyright (C) 2001
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
//#  $Id: $

#include <lofar_config.h>

#include <AMCBase/Epoch.h>
#include <Common/LofarLogger.h>
#include <Common/SystemUtil.h>

#include <Storage/MSWriter.h>
#include <Storage/MSWriterHDF5.h>

#ifdef HAVE_HDF5
#include <Common/Thread/Mutex.h>
#include <Interface/StreamableData.h>
#include <iostream>
#include <ctime>

#include <boost/format.hpp>
using boost::format;

// C->HDF5 translations of native types (Storage endianness)
template<typename T> hid_t h5nativeType();

template<> hid_t h5nativeType<float>()    { return H5T_NATIVE_FLOAT;  }
template<> hid_t h5nativeType<double>()   { return H5T_NATIVE_DOUBLE; }
template<> hid_t h5nativeType<unsigned>() { return H5T_NATIVE_UINT;   }
template<> hid_t h5nativeType<int>()      { return H5T_NATIVE_INT;    }
template<> hid_t h5nativeType<bool>()     { return H5T_NATIVE_CHAR;   } // assuming sizeof(bool) == 1

// C->HDF5 translations of types to use in header (ICD 003)
template<typename T> hid_t h5writeType();

template<> hid_t h5writeType<float>()    { return H5T_IEEE_F32LE; }
template<> hid_t h5writeType<double>()   { return H5T_IEEE_F64LE; }
template<> hid_t h5writeType<unsigned>() { return H5T_STD_U32LE;  }
template<> hid_t h5writeType<int>()      { return H5T_STD_I32LE;  }
template<> hid_t h5writeType<bool>()     { return H5T_STD_I32LE;  } // emulate bool with a 32-bit int

// C->HDF5 translations of types to use for data (CNProc endianness)
template<typename T> hid_t h5dataType( bool bigEndian );

template<> hid_t h5dataType<float>( bool bigEndian ) {
  return bigEndian ? H5T_IEEE_F32BE : H5T_IEEE_F32LE;
}

template<> hid_t h5dataType<LOFAR::fcomplex>( bool bigEndian ) {
  // emulate fcomplex with a 64-bit bitfield
  return bigEndian ? H5T_STD_B64BE : H5T_STD_B64LE;
}

template<typename T> void writeAttribute( hid_t loc, const char *name, T value )
{
  hid_t ret;

  hid_t dataspace;
  dataspace = H5Screate( H5S_SCALAR );
  ASSERT( dataspace > 0 );

  hid_t attr;
  attr = H5Acreate2( loc, name, h5writeType<T>(), dataspace,  H5P_DEFAULT,  H5P_DEFAULT );
  ASSERT( attr > 0 );

  ret = H5Awrite( attr, h5nativeType<T>(), &value );
  ASSERT( ret >= 0 );

  H5Aclose( attr );

  H5Sclose( dataspace );
}

template<typename U> void writeAttributeV( hid_t loc, const char *name, std::vector<U> value )
{
  hid_t ret;

  hsize_t dims[1] = { value.size() };

  hid_t dataspace;
  dataspace = H5Screate_simple( 1, dims, NULL );
  ASSERT( dataspace > 0 );

  hid_t attr;
  attr = H5Acreate2( loc, name, h5writeType<U>(), dataspace,  H5P_DEFAULT,  H5P_DEFAULT );
  ASSERT( attr > 0 );

  ret = H5Awrite( attr, h5nativeType<U>(), &value );
  ASSERT( ret >= 0 );

  H5Aclose( attr );

  H5Sclose( dataspace );
}


template<> void writeAttribute( hid_t loc, const char *name, char const *value )
{
  hid_t ret;

  hid_t datatype;
  datatype = H5Tcopy( H5T_C_S1 );
  ASSERT( datatype > 0 );
  ret = H5Tset_size( datatype, H5T_VARIABLE );
  ASSERT( ret >= 0 );

  hid_t dataspace;
  dataspace = H5Screate( H5S_SCALAR );
  ASSERT( dataspace > 0 );

  hid_t attr;
  attr = H5Acreate2( loc, name, datatype, dataspace,  H5P_DEFAULT,  H5P_DEFAULT );
  ASSERT( attr > 0 );

  ret = H5Awrite( attr, datatype, &value );
  ASSERT( ret >= 0 );

  H5Aclose( attr );

  H5Tclose( datatype );
  H5Sclose( dataspace );
}


template<> void writeAttribute( hid_t loc, const char *name, const std::string value )
{
  writeAttribute(loc, name, value.c_str());
}

template<> void writeAttributeV( hid_t loc, const char *name, std::vector<const char *> value )
{
  hid_t ret;

  hid_t datatype;
  datatype = H5Tcopy( H5T_C_S1 );
  ASSERT( datatype > 0 );
  ret = H5Tset_size( datatype, H5T_VARIABLE );
  ASSERT( ret >= 0 );

  hsize_t dims[1] = { value.size() };

  hid_t dataspace;
  dataspace = H5Screate_simple( 1, dims, NULL );
  ASSERT( dataspace > 0 );

  hid_t attr;
  attr = H5Acreate2( loc, name, datatype, dataspace,  H5P_DEFAULT,  H5P_DEFAULT );
  ASSERT( attr > 0 );

  ret = H5Awrite( attr, datatype, &value[0] );
  ASSERT( ret >= 0 );

  H5Aclose( attr );

  H5Tclose( datatype );
  H5Sclose( dataspace );
}

template<> void writeAttributeV( hid_t loc, const char *name, std::vector<std::string> value )
{
  // convert to C-style strings
  std::vector<const char *> cstrs(value.size());
  for (unsigned i = 0; i < value.size(); i++)
    cstrs[i] = value[i].c_str();

  writeAttributeV(loc, name, cstrs);
}

namespace LOFAR 
{

  namespace RTCP
  {
    // Prevent concurrent access to HDF5, which may not be compiled thread-safe. The Thread-safe version
    // uses global locks too anyway.
    static Mutex HDF5Mutex;

    template <typename T,unsigned DIM> MSWriterHDF5<T,DIM>::MSWriterHDF5 (const char *filename, const Parset &parset, OutputType outputType, unsigned fileno, bool isBigEndian)
    :
      MSWriterFile(str(format("%s.dat") % filename).c_str(),false),
      itsNrChannels(parset.nrChannelsPerSubband() * parset.nrSubbands()),
      itsNextSeqNr(0),
      itsDatatype(h5dataType<T>(isBigEndian))
    {
      ScopedLock sl(HDF5Mutex);

      unsigned sapNr = 0;
      unsigned beamNr;
      unsigned stokesNr;
      const char *stokes;

      unsigned nrBlocks = ceil((parset.stopTime() - parset.startTime()) / parset.CNintegrationTime());

      switch (outputType) {
        case COHERENT_STOKES: {
          // assume stokes are either I or IQUV
          const char *stokesVars[] = { "I", "Q", "U", "V" };
          stokesNr = fileno % parset.nrCoherentStokes();
          beamNr = fileno / parset.nrCoherentStokes() / parset.nrPartsPerStokes();

          stokes = stokesVars[stokesNr];

          itsNrSamples = parset.CNintegrationSteps() / parset.coherentStokesTimeIntegrationFactor();
          break;
        }

        case BEAM_FORMED_DATA: {
          const char *stokesVars[] = { "X", "Y" };
          stokesNr = fileno % NR_POLARIZATIONS;
          beamNr = fileno / NR_POLARIZATIONS / parset.nrPartsPerStokes();

          stokes = stokesVars[stokesNr];

          itsNrSamples = parset.CNintegrationSteps();
          break;
        } 

        default:
          THROW(StorageException, "MSWriterHDF5 can only handle Coherent Stokes and Beam-formed Data");
      }

      itsZeroBlock.resize( itsNrSamples * itsNrChannels );

      LOG_DEBUG_STR("MSWriterHDF5: opening " << filename);

      hid_t ret;

      // create the top structure
      hid_t file = H5Fcreate( filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT );
      ASSERT( file > 0 );

      char now_str[50];
      time_t now = time(NULL);
      if (strftime( now_str, sizeof now_str, "%Y-%m-%dT%H:%M:%S.0", gmtime(&now) ) == 0 )
        now_str[0] = 0;

      writeAttribute( file, "GROUPTYPE", "Root" );
      writeAttribute( file, "FILENAME",  LOFAR::basename(filename).c_str() );


      writeAttribute<const char*>( file, "FILEDATE",  now_str );
      writeAttribute( file, "FILETYPE",  "bf" );
      writeAttribute( file, "TELESCOPE", "LOFAR" );
      writeAttribute( file, "OBSERVER",  parset.observerName() );

      writeAttribute( file, "PROJECT_ID",      parset.getString("Observation.Campaign.name") );
      writeAttribute( file, "PROJECT_TITLE",   parset.getString("Observation.Campaign.title") );
      writeAttribute( file, "PROJECT_PI",      parset.getString("Observation.Campaign.PI") );
      writeAttribute( file, "PROJECT_CO_I",    parset.getString("Observation.Campaign.CO_I") ); // TODO: actually a vector, so pretty print a bit more
      writeAttribute( file, "PROJECT_CONTACT", parset.getString("Observation.Campaign.contact") );

      writeAttribute( file, "OBSERVATION_ID",  str(format("%s") % parset.observationID()).c_str() );

      //writeAttribute( file, "OBSERVATION_START_MJD",  "" );
      //writeAttribute( file, "OBSERVATION_START_TAI",  "" );
      //writeAttribute( file, "OBSERVATION_START_UTC",  "" );
      //writeAttribute( file, "OBSERVATION_END_MJD",    "" );
      //writeAttribute( file, "OBSERVATION_END_TAI",    "" );
      //writeAttribute( file, "OBSERVATION_END_UTC",    "" );

      writeAttribute<int>( file, "OBSERVATION_NOF_STATIONS",  parset.nrStations() ); // TODO: SS beamformer?
      writeAttributeV( file, "OBSERVATION_STATIONS_LIST", parset.allStationNames() ); // TODO: SS beamformer?
#if 0
      // TODO: are subbands represented by their beginning, end, or middle frequency?

      std::vector<unsigned> subbands = parset.subbandList();
      unsigned max_subband = *std::max_element( subbands.begin(), subbands.end() );
      unsigned min_subband = *std::min_element( subbands.begin(), subbands.end() );
#endif
      //writeAttribute<double>( file, "OBSERVATION_FREQUENCY_MAX",    0.0 );
      //writeAttribute<double>( file, "OBSERVATION_FREQUENCY_MIN",    0.0 );
      //writeAttribute<double>( file, "OBSERVATION_FREQUENCY_CENTER", 0.0 );
      //writeAttribute( file, "OBSERVATION_FREQUENCY_UNIT", "MHz" );
      writeAttribute<int>( file, "OBSERVATION_NOF_BITS_PER_SAMPLE", parset.nrBitsPerSample() );

      writeAttribute<double>( file, "CLOCK_FREQUENCY", parset.clockSpeed() / 1e6 );
      writeAttribute( file, "CLOCK_FREQUENCY_UNIT", "MHz" );

      writeAttribute( file, "ANTENNA_SET",      parset.antennaSet() );
      writeAttribute( file, "FILTER_SELECTION", parset.getString("Observation.bandFilter") );
      //writeAttribute( file, "TARGET",           "" );

      //writeAttribute( file, "SYSTEM_VERSION",   "" );
      //writeAttribute( file, "PIPELINE_NAME",    "" );
      //writeAttribute( file, "PIPELINE_VERSION", "" );
      writeAttribute( file, "NOTES",            "" );

      // SysLog group -- empty for now

      hid_t syslog = H5Gcreate2( file, "SysLog", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
      ASSERT( syslog > 0 );

      ret = H5Gclose(syslog);
      ASSERT( ret >= 0 );

      // Information about the station beam (SAP)

      hid_t sap = H5Gcreate2( file, str(format("SubArrayPointing%03u") % sapNr).c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
      ASSERT( sap > 0 );

      writeAttribute( sap, "GROUPTYPE",     "SubArrayPointing" );
      writeAttribute( sap, "NOF_STATIONS",  parset.nrStations() );
      writeAttributeV( sap, "STATIONS_LIST", parset.allStationNames() ); // TODO: SS beamformer?

      // TODO: non-J2000 pointings
      ASSERT( parset.getBeamDirectionType(sapNr) == "J2000" );
      std::vector<double> beamDir = parset.getBeamDirection(sapNr);
      writeAttribute<double>( sap, "POINT_RA",             beamDir[0] * 180.0 / M_PI );
      writeAttribute<double>( sap, "POINT_DEC",            beamDir[1] * 180.0 / M_PI );

      //writeAttribute(         sap, "POINT_ALTITUDE",       "" );
      //writeAttribute(         sap, "POINT_AZIMUTH",        "" );

      writeAttribute<double>( sap, "CLOCK_RATE",           parset.clockSpeed() / 1e6 );
      writeAttribute(         sap, "CLOCK_RATE_UNIT",      "MHz" );

      writeAttribute<int>(    sap, "NOF_SAMPLES",          itsNrSamples * nrBlocks );
      writeAttribute<double>( sap, "SAMPLING_RATE",        itsNrSamples / parset.CNintegrationTime() / 1e6 );
      writeAttribute(         sap, "SAMPLING_RATE_UNIT",   "MHz" );

      writeAttribute<int>(    sap, "CHANNELS_PER_SUBBAND", parset.nrChannelsPerSubband() );
      writeAttribute<double>( sap, "SUBBAND_WIDTH",        parset.clockSpeed() / 1e6 / 1024 );
      writeAttribute(         sap, "SUBBAND_WIDTH_UNIT",   "MHz" );
      writeAttribute<double>( sap, "CHANNEL_WIDTH",        parset.clockSpeed() / 1e6 / 1024 / parset.nrChannelsPerSubband() );
      writeAttribute(         sap, "CHANNEL_WIDTH_UNIT",   "MHz" );

      writeAttribute<int>(    sap, "NOF_BEAMS",            parset.nrPencilBeams() );

      // Process History group -- empty for now
      {
        hid_t prochist = H5Gcreate2( sap, "ProcessHistory", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
        ASSERT( prochist > 0 );

        ret = H5Gclose(prochist);
        ASSERT( ret >= 0 );
      }

      // Information about the pencil beam

      hid_t beam = H5Gcreate2( sap, str(format("Beam%03u") % sapNr).c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
      ASSERT( beam > 0 );

      writeAttribute(         beam, "GROUPTYPE",     "Beam" );
      //writeAttribute(       beam, "TARGET",        "" ); // TODO: vector of targets
      writeAttribute(         beam, "NOF_STATIONS",  parset.nrStations() );
      writeAttributeV(        beam, "STATIONS_LIST", parset.allStationNames() ); // TODO: SS beamformer?

      //const char *trackingTypes[] = { "J2000", "LMN", "TBD" };
      //writeAttribute(         beam, "TRACKING",      "J2000" ); // TODO: support non-tracking

      // TODO: non-J2000 pointings
      //ASSERT( parset.getBeamDirectionType() == "J2000" );
      BeamCoordinates pbeamDirs = parset.pencilBeams();
      BeamCoord3D pbeamDir = pbeamDirs[beamNr];
      writeAttribute<double>( beam, "POINT_RA",      (beamDir[0] + pbeamDir[0]) * 180.0 / M_PI );
      writeAttribute<double>( beam, "POINT_DEC",     (beamDir[1] + pbeamDir[1]) * 180.0 / M_PI );
      writeAttribute<double>( beam, "POINT_OFFSET_RA",  pbeamDir[0] * 180.0 / M_PI );
      writeAttribute<double>( beam, "POINT_OFFSET_DEC", pbeamDir[1] * 180.0 / M_PI );

      //writeAttribute<double>( beam, "BEAM_DIAMETER_RA",      0.0 );
      //writeAttribute<double>( beam, "BEAM_DIAMETER_DEC",     0.0 );
      //writeAttribute<double>( beam, "BEAM_FREQUENCY_CENTER", 0.0 );
      //writeAttribute(         beam, "BEAM_FREQUENCY_CENTER_UNIT", "MHz" );

      writeAttribute<bool>(   beam, "FOLDED_DATA",      false );
      //writeAttribute<float>(  beam, "FOLD_PERIOD",      0.0 );
      //writeAttribute<float>(  beam, "FOLD_PERIOD_UNIT", "s" );

      //writeAttribute<bool>(   beam, "DEDISPERSION",              "NONE" );
      //writeAttribute<float>(  beam, "DEDISPERSION_MEASURE",      0.0 );
      //writeAttribute(         beam, "DEDISPERSION_MEASURE_UNIT", "pc/cm^3" );

      //writeAttribute<bool>(   beam, "BARYCENTER",       false );

      writeAttribute<int>(   beam, "NOF_STOKES",       1 ); // we always write 1 stokes per file for now
      writeAttributeV(       beam, "STOKES_COMPONENTS", std::vector<std::string>( 1, stokes ) );
      writeAttribute<bool>(  beam, "COMPLEX_VOLTAGES", outputType == BEAM_FORMED_DATA );
      writeAttribute(        beam, "SIGNAL_SUM",       "COHERENT" );

      // Process History group -- empty for now
      {
        hid_t prochist = H5Gcreate2( beam, "ProcessHistory", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
        ASSERT( prochist > 0 );

        ret = H5Gclose(prochist);
        ASSERT( ret >= 0 );
      }

      // Coordinates group -- empty for now
      {
        hid_t coordinates = H5Gcreate2( beam, "Coordinates", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
        ASSERT( coordinates > 0 );

        ret = H5Gclose(coordinates);
        ASSERT( ret >= 0 );
      }

      // define the dimensions
      const int rank = 2;
      hsize_t dims[rank] = { itsNrSamples * nrBlocks, itsNrChannels };
      hsize_t maxdims[rank] = { H5S_UNLIMITED, itsNrChannels };

      hid_t filespace = H5Screate_simple( rank, dims, maxdims );
      ASSERT( filespace > 0 );

      // define the file writing strategy
      hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
      ASSERT( dcpl > 0 );
      ret = H5Pset_layout(dcpl, H5D_CONTIGUOUS);
      ASSERT( ret >= 0 );
      ret = H5Pset_external(dcpl, LOFAR::basename(str(format("%s.dat") % filename)).c_str(), 0, H5F_UNLIMITED);
      ASSERT( ret >= 0 );

      // create the dataset
      itsDataset = H5Dcreate2( beam, str(format("Stokes%u") % stokesNr).c_str(), itsDatatype, filespace, H5P_DEFAULT, dcpl,  H5P_DEFAULT );
      ASSERT( itsDataset > 0 );

      ret = H5Pclose(dcpl);
      ASSERT( ret >= 0 );

      ret = H5Sclose(filespace);
      ASSERT( ret >= 0 );

      writeAttribute( itsDataset, "STOKES_COMPONENT", stokes );
      std::vector<int> nofChannels( parset.nrSubbands(), parset.nrChannelsPerSubband() );
      writeAttributeV(     itsDataset, "NOF_CHANNELS", nofChannels );
      writeAttribute<int>( itsDataset, "NOF_SUBBANDS", parset.nrSubbands() );
      writeAttribute<int>( itsDataset, "NOF_SAMPLES",  itsNrSamples * nrBlocks );

      ret = H5Dclose(itsDataset);
      ASSERT( ret >= 0 );

      ret = H5Gclose(beam);
      ASSERT( ret >= 0 );

      ret = H5Gclose(sap);
      ASSERT( ret >= 0 );

      ret = H5Fclose(file);
      ASSERT( ret >= 0 );
    }

    template <typename T,unsigned DIM> MSWriterHDF5<T,DIM>::~MSWriterHDF5()
    {
    }

    template <typename T,unsigned DIM> void MSWriterHDF5<T,DIM>::write(StreamableData *data)
    {
      SampleData<T,DIM> *sdata = static_cast<SampleData<T,DIM> *>(data);

      unsigned seqNr = data->byteSwappedSequenceNumber();

      // fill in zeroes for lost blocks
      for (unsigned i = itsNextSeqNr; i < seqNr; i++)
        itsFile.write( &itsZeroBlock[0], itsZeroBlock.size() * sizeof(T) );

      // make sure we skip |2 in the highest dimension
      itsFile.write( sdata->samples.origin(), itsZeroBlock.size() * sizeof(T) );

      itsNextSeqNr = seqNr + 1;
    }

    // specialisation for StokesData
    template class MSWriterHDF5<float,3>;

    // specialisation for BeamFormedData
    template class MSWriterHDF5<fcomplex,3>;

  } // namespace RTCP
} // namespace LOFAR

#else // no HAVE_HDF5

namespace LOFAR 
{

  namespace RTCP
  {

    template <typename T,unsigned DIM> MSWriterHDF5<T,DIM>::MSWriterHDF5 (const char *filename, const Parset&, OutputType, unsigned, bool)
    :
      MSWriterFile(filename,false)
    {
      LOG_ERROR_STR( "Using the HDF5 writer is not supported (file: " << filename << ")" );
    }

    template <typename T,unsigned DIM> MSWriterHDF5<T,DIM>::~MSWriterHDF5()
    {
    }

  } // namespace RTCP
} // namespace LOFAR

#endif
