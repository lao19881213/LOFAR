//#  GCF_PVBool.h: MAC boolean property type
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

#ifndef GCF_PVBOOL_H
#define GCF_PVBOOL_H

#include <lofar_config.h>
#ifdef HAVE_LOFAR_SAL
#include <SAL/GCF_PValue.h>
#else
#include <GCF_PValue.h>
#endif

/**
   By means of this property type a boolean (TRUE/FALSE or YES/NO) value can be 
   used.
*/

class GCFPVBool : public GCFPValue
{
  public: 
  	GCFPVBool (bool val = false) : GCFPValue(LPT_BOOL), _value(val) {;}
  	virtual ~GCFPVBool () {;}
    
    /** Changes the value of this object */
    inline void setValue (const bool newVal) {_value = newVal;};

    /** 
     * Changes the value of this object by means of a stringbuffer, 
     * which will be translated.
     * @see GCFPValue::setValue(const string value)
     */
    virtual TGCFResult setValue (const string value);

    /** Returns the value of this object*/
    inline bool getValue () const {return _value;};

    /** @see GCFPValue::clone() */
    virtual GCFPValue* clone () const;

    /** @see GCFPValue::copy() */
    virtual TGCFResult copy (const GCFPValue& value);
    
  private: // Private attributes
    /** The value */
    volatile bool _value;
};
#endif
