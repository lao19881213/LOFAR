//#  tCSConnection.cc: Test program for CSConnection using one DataHolder.
//#
//#  Copyright (C) 2002-2004
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
//#  $Id$

// \file 
// Test program for CSConnection using one DataHolder for send and
// receive. This program can only be run interactively. You need to start two
// instances in two different terminals. First start the server (by suppyling
// "s" as argument), next start the client (by supplying "c" as argument). The
// server should send the message "Server message" to the client. In response,
// the client should send the message "Client message" to the server.

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Transport/DataHolder.h>
#include <Transport/CSConnection.h>
#include <Transport/TH_Socket.h>
#include <Common/lofar_iostream.h>
#include <libgen.h>

namespace LOFAR
{
  class MyDH : public DataHolder
  {
  public:
    MyDH* clone() const { return new MyDH(*this); }
    void init() { addField("MyText", BlobField<char>(1, 64)); createDataBlock();}
    char* text() { return myText; }
    void text(char* txt) { strcpy(myText, txt); }
  private:
    virtual void fillDataPointers() { myText = getData<char>("MyText"); }
    char* myText;
  };
}

using namespace LOFAR;

int main(int argc, char* argv[])
{
  const char* progName = basename(argv[0]);

  INIT_LOGGER(progName);

  if (argc < 2)
  {
    LOG_ERROR_STR("Usage: " << progName << " [c|s]");
    return 1;
  }
  bool isServer = (!strcmp(argv[1], "s"));
  LOG_INFO_STR("I'm a " << (isServer ? "server." : "client."));

  try
  {
    MyDH*         myDH = new MyDH();
    TH_Socket*    myTH;
    CSConnection* myConn;
    myDH->init();

    if (isServer)
    {
      myTH = new TH_Socket("3333");
      LOG_INFO("Constructed TH_Socket(server)");

      ASSERT(myTH->init());
      LOG_INFO("TH_Socket is initialized");

      myConn = new CSConnection("r/w", myDH, myDH, myTH);
      ASSERT(myConn);
      LOG_INFO("Created CSConnection");

      myDH->text("Server message");
      LOG_INFO_STR("Sending: " << myDH->text());
      myConn->write();
      myConn->read();
      LOG_INFO_STR("Received: " << myDH->text());
    }
    else
    {
      myTH = new TH_Socket("localhost", string("3333"));
      LOG_INFO("Constructed TH_Socket(client)");

      ASSERT(myTH->init());
      LOG_INFO("TH_Socket is initialized");

      myConn = new CSConnection("r/w", myDH, myDH, myTH);
      ASSERT(myConn);
      LOG_INFO("Created CSConnection");

      myConn->read();
      LOG_INFO_STR("Received: " << myDH->text());
      sleep(1);
      myDH->text("Client message");
      LOG_INFO_STR("Sending: " << myDH->text());
      myConn->write();
    }

  }
  catch (Exception& e)
  {
    cerr << e << endl;
    return 1;
  }
  
  return 0;
}
