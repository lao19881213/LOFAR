//#  GPM_Controller.h: singleton class; bridge between controller application 
//#                    and Property Agent
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

#ifndef GPM_CONTROLLER_H
#define GPM_CONTROLLER_H

#include <TM/GCF_Task.h>
#include <TM/PortInterface/GCF_Port.h>
#include "GPM_Defines.h"
#include "GPM_Service.h"
#include <Common/lofar_map.h>
#include <Common/lofar_list.h>

/**
   This singleton class forms the bridge between the PML API classes and the PA. 
   It is a hidden task with its own state machine in each Application, which 
   wants to be part of the MAC subsystem of LOFAR. It will be created at the 
   moment a service of PML will be requested by the Application (like load 
   property set or load APC).
*/

class GCFSupervisedTask;
class GPMPropertySet;
class GCFPValue;
class GCFEvent;
class GCFPortInterface;

class GPMController : public GCFTask
{
  public:
    GPMController (GCFSupervisedTask& supervisedTask);
    ~GPMController ();

  private: // member functions
    friend class GCFSupervisedTask;
    friend class GPMService;
    friend class GPMPropertySet;
    
    TPMResult loadAPC (const string& apcName, const string& scope);
    TPMResult unloadAPC (const string& apcName, const string& scope);
    TPMResult reloadAPC (const string& apcName, const string& scope);
    
    TPMResult loadMyProperties (const TPropertySet& newSet);
    TPMResult unloadMyProperties (const string& scope);
    
    TPMResult set (const string& propName, const GCFPValue& value);
    TPMResult get (const string& propName);
    
    TPMResult getMyOldValue (const string& propName, GCFPValue** value);
    void valueChanged (const string& propName, const GCFPValue& value);
    void valueGet (const string& propName, const GCFPValue& value);
    
    void propertiesLinked (const string& scope, list<string>& notLinkedProps);
    void propertiesUnlinked (const string& scope, list<string>& notUnlinkedProps);
  
  private: // state methods
    GCFEvent::TResult initial   (GCFEvent& e, GCFPortInterface& p);
    GCFEvent::TResult connected (GCFEvent& e, GCFPortInterface& p);
        
  private: // helper methods
    GPMPropertySet* findPropertySet (const string& propName);
    void registerScope (const string& scope);
    void unpackPropertyList (char* pListData, list<string>& propertyList);
    
  private: // data members
    GCFSupervisedTask&            _supervisedTask;
    
    GCFPort                       _propertyAgent;
    GPMService                    _scadaService;
    bool                          _isBusy;
    bool                          _preparing;
    unsigned int                  _counter;
    map<string, GPMPropertySet*>  _propertySets;
    typedef map<string, GPMPropertySet*>::iterator TPropertySetIter;
    
    typedef struct
    {
      GCFPValue* pValue;
      string propName;
    } TGetData;
};
#endif
