//# parmdbclient.cc: Client handling a distributed ParmDB part
//#
//# Copyright (C) 2009
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
#include <ParmDB/ParmFacadeLocal.h>
#include <MWCommon/SocketConnection.h>
#include <MWCommon/MWBlobIO.h>
#include <MWCommon/VdsPartDesc.h>
#include <Blob/BlobArray.h>
#include <Blob/BlobAipsIO.h>
#include <Common/LofarLogger.h>
#include <casa/IO/AipsIO.h>
#include <iostream>
#include <unistd.h>     //# for basename
#include <libgen.h>

using namespace LOFAR::BBS;
using namespace LOFAR::CEP;
using namespace LOFAR;
using namespace std;

void getRange (ParmFacadeLocal& pdb, BlobIStream& bis, BlobOStream& bos)
{
  string pattern;
  bis >> pattern;
  bos << pdb.getRange (pattern);
}

void getNames (ParmFacadeLocal& pdb, BlobIStream& bis, BlobOStream& bos)
{
  string pattern;
  bis >> pattern;
  bos << pdb.getNames (pattern);
}

void getValues (ParmFacadeLocal& pdb, BlobIStream& bis, BlobOStream& bos)
{
  string pattern;
  double freqv1, freqv2, timev1, timev2;
  int nfreq, ntime;
  bool asStartEnd;
  bis >> pattern >> freqv1 >> freqv2 >> nfreq
      >> timev1 >> timev2 >> ntime >> asStartEnd;
  BlobAipsIO baio(bos);
  casa::AipsIO aio(&baio);
  aio << pdb.getValues (pattern, freqv1, freqv2, nfreq,
                        timev1, timev2, ntime, asStartEnd);
}

void getValuesVec (ParmFacadeLocal& pdb, BlobIStream& bis, BlobOStream& bos)
{
  string pattern;
  vector<double> freqv1, freqv2, timev1, timev2;
  bool asStartEnd;
  bis >> pattern >> freqv1 >> freqv2 >> timev1 >> timev2 >> asStartEnd;
  BlobAipsIO baio(bos);
  casa::AipsIO aio(&baio);
  aio << pdb.getValues (pattern, freqv1, freqv2, timev1, timev2, asStartEnd);
}

void getValuesGrid (ParmFacadeLocal& pdb, BlobIStream& bis, BlobOStream& bos)
{
  string pattern;
  double sfreq, efreq, stime, etime;
  bis >> pattern >> sfreq >> efreq >> stime >> etime;
  BlobAipsIO baio(bos);
  casa::AipsIO aio(&baio);
  aio << pdb.getValuesGrid (pattern, sfreq, efreq, stime, etime);
}

void doIt (SocketConnection& conn, ParmFacadeLocal& pdb)
{
  // Allocate buffers here, so they are kept alive and not resized all the time.
  BlobString bufin;
  BlobString bufout;
  while (true) {
    // Read and handle the message.
    // Stop if such a message is given.
    conn.read (bufin);
    MWBlobIn bbi(bufin);
    MWBlobOut bbo(bufout, 1, 0);
    switch (bbi.getOperation()) {
    case 0:
      // Exit
      bbi.finish();
      return;
    case 1:
      // Handle getRange;
      getRange (pdb, bbi.blobStream(), bbo.blobStream());
      break;
    case 2:
      // Handle getNames;
      getNames (pdb, bbi.blobStream(), bbo.blobStream());
      break;
    case 3:
      // Handle getValues with nfreq/ntime.
      getValues (pdb, bbi.blobStream(), bbo.blobStream());
      break;
    case 4:
      // Handle getValues with vector of freq/time.
      getValuesVec (pdb, bbi.blobStream(), bbo.blobStream());
      break;
    case 5:
      // Handle getValuesGrid
      getValuesGrid (pdb, bbi.blobStream(), bbo.blobStream());
      break;
    default:
      ASSERTSTR(false, "parmdbclient: unknown command-id "
                << bbi.getOperation());
    }
    // Finish the blobstreams and write the result message.
    bbi.finish();
    bbo.finish();
    conn.write (bufout);
  }
}

int main (int argc, const char* argv[])
{
  const char* progName = basename(argv[0]);
  INIT_LOGGER(progName);
  SocketConnection::ShPtr conn;
  try {
    ASSERTSTR (argc >= 7, "Use as: parmdbclient socket <host> "
               "<port> <#processes> <rank> <mspart>");
    string host (argv[2]);
    string port (argv[3]);
    istringstream iss(argv[4]);
    int nnode, rank;
    iss >> nnode;
    istringstream iss1(argv[5]);
    iss1 >> rank;
    string fname(argv[6]);
    // Setup the connection.
    conn = SocketConnection::ShPtr(new SocketConnection(host, port));
    // Open the ParmDB after getting its name from the VDS file.
    {
      VdsPartDesc vds((ParameterSet(fname)));
      fname = vds.getFileName();
    }
    ParmFacadeLocal parmdb(fname);
    {
      // Tell master init was successful.
      BlobString bufout;
      MWBlobOut bbo(bufout, 1, 0);
      bbo.finish();
      conn->write (bufout);
    }
    // Handle requests.
    doIt (*conn, parmdb);
  } catch (std::exception& x) {
    LOG_FATAL (string("Unexpected exception in parmdbclient: ") + x.what());
    // Tell master there is an error.
    BlobString bufout;
    MWBlobOut bbo(bufout, 0, 0);
    bbo.blobStream() << x.what();
    bbo.finish();
    conn->write (bufout);
    return 1;
  }
  return 0;
}
