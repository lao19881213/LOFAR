//#  GCF_PVString.cc: 
//#
//#  Copyright (C) 2002-2003
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


#include <GCF/GCF_PVString.h>

unsigned int GCFPVString::unpackConcrete(const char* valBuf)
{
  unsigned int unpackedBytes(0);
  unsigned int stringLength(0);
  memcpy((void *) &stringLength, valBuf, sizeof(unsigned int));
  _value.assign(valBuf + sizeof(unsigned int), stringLength); 
  unpackedBytes += sizeof(unsigned int) + stringLength;
  return unpackedBytes;
}

unsigned int GCFPVString::packConcrete(char* valBuf) const
{
  unsigned int packedBytes(0);
  unsigned int stringLength(_value.length());
  memcpy(valBuf, (void *) &stringLength, sizeof(unsigned int));
  memcpy(valBuf + sizeof(unsigned int), (void *) _value.c_str(), stringLength);
  packedBytes += sizeof(unsigned int) + stringLength;
  return packedBytes;
}

TGCFResult GCFPVString::setValue(const string value)
{
  TGCFResult result(GCF_NO_ERROR);

  _value = value;
  
  return result;
}

GCFPValue* GCFPVString::clone() const
{
  GCFPValue* pNewValue = new GCFPVString(_value);
  return pNewValue;
}

TGCFResult GCFPVString::copy(const GCFPValue& newVal)
{
  TGCFResult result(GCF_NO_ERROR);

  if (newVal.getType() == getType())
    _value = ((GCFPVString *)&newVal)->getValue();
  else
    result = GCF_DIFFERENT_TYPES;
  
  return result;
}
