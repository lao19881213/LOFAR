
// DH_Corr.cpp: implementation of the DH_Corr class.
//
//////////////////////////////////////////////////////////////////////


#include "DH_Corr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DH_Corr::DH_Corr (const string& name)
: DataHolder (name, "DH_Corr")
{
  setDataPacket(&itsDataPacket,sizeof(itsDataPacket));

}

DH_Corr::~DH_Corr () {

}
