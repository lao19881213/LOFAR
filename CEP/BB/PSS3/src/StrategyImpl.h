//#  StrategyImpl.h: A base class for all calibration strategies
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

#ifndef PSS3_STRATEGYIMPL_H
#define PSS3_STRATEGYIMPL_H

#include <lofar_config.h>

//# Includes
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>
#include <PSS3/Solution.h>

//# Forward Declarations
class Calibrator;

// This is a base class for all calibration strategies.

class StrategyImpl
{
public:
  StrategyImpl();

  virtual ~StrategyImpl();

  /// Execute the strategy
  virtual bool execute(vector<string>& parmNames,     // Parameters for which 
		                                       // to solve  
		       vector<float>& parmValues,    // Parameter values
		       Solution& solutionQuality,    // Solution quality
		       int& source) = 0;             // Source number
    
  /// Get strategy implementation type
  virtual string getType() const = 0;

 private:

};


#endif
