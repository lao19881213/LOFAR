//# Parm.h: Parameter with polynomial coefficients
//#
//# Copyright (C) 2002
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

#ifndef MEQ_PARM_H
#define MEQ_PARM_H

//# Includes
#include <MEQ/Function.h>
#include <MEQ/ParmTable.h>
#include <MEQ/Polc.h>
#include <MEQ/Vells.h>
#include <Common/lofar_vector.h>

#pragma aidgroup Meq
#pragma types #Meq::Parm

// The comments below are used to automatically generate a default
// init-record for the class 

//defrec begin MeqParm
//  Represents a parameter, either created on-the-fly (a default
//  value must then be supplied), or read from a MEP database.
//  A MeqParm cannot have any children.
//field: polcs [=]
//  active polcs. One or more meqpolc() objects may be provided. These
//  will be reused for subsequent requests if the domains match, or
//  if the inf_domain or grow_domain attributes are specified.
//field: default [=]
//  default polc. A meqpolc() object. This is used when an applicable
//  polc is not found in the table, or a table is not provided.
//field: table_name '' 
//  MEP table name. If empty, then the default parameter value is used
//field: parm_name '' 
//  MEP parm name used to look inside the table. If empty, then the node 
//  name is used instead.
//field: auto_save F 
//  if T, then any updates to a polc are saved into the MEP table 
//  automatically (for example, after each solve iteration). Default 
//  behaviour is to only save when specified via a request rider (e.g.,
//  at the end of a solve).
//defrec end

namespace Meq {

// This class contains the coefficients of a 2-dim polynomial.
// The order in time and frequency must be given.
// The nr of coefficients is (1+order(time)) * (1+order(freq)).
// The coefficients are numbered 0..N with the time as the most rapidly
// varying axis.

//##ModelId=3F86886E01BD
class Parm: public Function
{
public:
  // The default constructor.
  // The object should be filled by the init method.
    //##ModelId=3F86886F021B
  Parm();

  // Create a parameter with the given name and default value.
  // The default value is used if no suitable value can be found.
  // The ParmTable can be null meaning that the parameter is temporary.
    //##ModelId=3F86886F0242
  Parm (const string& name, ParmTable* table,
	      const Polc::Ref::Xfer & defaultValue = Polc::Ref() );

    //##ModelId=3F86886F021E
  virtual ~Parm();

    //##ModelId=400E53510330
    virtual TypeId objectType() const
    { return TpMeqParm; }

  // Get the parameter id.
    //##ModelId=3F86886F0224
  unsigned int getParmId() const
    { return parmid_; }

    //##ModelId=3F86886F022C
  bool isSolvable() const
    { return solvable_; }

  // Get the requested result of the parameter.
    //##ModelId=3F86886F022E
  virtual int getResult (Result::Ref &resref, 
                         const std::vector<Result::Ref> &childres,
                         const Request &req,bool newreq);

  // process parm-specific rider commands
  virtual void processCommands (const DataRecord &rec,const Request &req);

  // Initialize the parameter for the given domain.
    //##ModelId=3F86886F0226
  virtual int initDomain (const Domain&);

  // Make the new value persistent (for the given domain).
    //##ModelId=3F86886F023C
  virtual void save();

    //##ModelId=400E5352023D
  virtual void init (DataRecord::Ref::Xfer& initrec, Forest* frst);

    //##ModelId=400E53520391
  //## Standard debug info method
  virtual string sdebug (int detail = 1, const string& prefix = "",
			 const char* name = 0) const;
  
  LocalDebugContext;

protected:
    //##ModelId=400E5353019E
  // finds polcs in table or uses the default
  void findRelevantPolcs (vector<Polc::Ref> &polcs,const Domain &domain);
  // initializes polcs based on value of solvable flag
  int  initSolvable  (const Domain &domain);
  // assigns spids to solvable polcs. Called from initSolvable()
  int  initSpids     ();
    

  // Set the polynomials.
    //##ModelId=400E535301F7
  void setPolc (Polc *polc,int flags=DMI::ANONWR)
  { 
    polcs_.resize(1);
    polcs_[0].attach(polc,flags); 
  }
  
  void setPolc (Polc &polc)
  { setPolc(&polc,DMI::EXTERNAL|DMI::WRITE); }

   // Get the polynomials.
    //##ModelId=400E535302DB
  const Polc & getPolc(int i=0) const
  { return *(polcs_[i]); }
  
//  virtual void checkInitState (DataRecord &rec);
    //##ModelId=400E5353033A
  virtual void setStateImpl (DataRecord &rec,bool initializing);
  
private:
    
    //##ModelId=3F86886F0215
  uint parmid_;
    //##ModelId=3F86886F0216
  bool solvable_;
  bool auto_save_;
  
    //##ModelId=3F86886F0213
  string name_;
  
    //##ModelId=400E535000A3
  ParmTable * parmtable_;
  
  //##ModelId=400E535000B2
  //##Documentation
  //## default polc (used if no table or no matching polcs in the table)
  Polc::Ref    default_polc_;
  
  //##Documentation
  //## ID of current domain
  HIID         domain_id_;
  
  //##Documentation
  //## active polcs for current domain. If Parm solvable, this is always a 
  //## single polc. 
  vector<Polc::Ref> polcs_;
  
//   //##ModelId=400E535000C1
//   //##Documentation
//   //## vector of all polcs relevant to current domain. This may be a superset
//   //## of polcs_.
//   vector<Polc::Ref> all_polcs_;
  
};


} // namespace Meq

#endif
