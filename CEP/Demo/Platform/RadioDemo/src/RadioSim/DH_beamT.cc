
// DH_beamT.cpp: implementation of the DH_beamT class.
//
//////////////////////////////////////////////////////////////////////


#include "DH_beamT.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DH_beamT::DH_beamT (const string& name)
: DataHolder (name, "DH_beamT")
{
  setDataPacket(&itsDataPacket,sizeof(itsDataPacket));
}

DH_beamT::~DH_beamT (){

}
