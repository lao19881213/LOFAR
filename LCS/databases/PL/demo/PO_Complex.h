//# This file was generated by genDBcode v2.8 on Wed Mar 10 10:58:37 CET 2004
//# with the command: genDBcode P Complex.plmap  
//# from the directory: /home/loose/LOFAR/LCS/databases/PL/demo
//#
//# EDITING THIS FILE MANUALLY IS AT YOUR OWN RISK
//# IT MIGHT BE OVERWRITTEN BY THE NEXT MAKE OF YOUR PROJECT
//#
//# only include this file once
#if !defined(PO_040310105837_COMPLEX_H)
#define PO_040310105837_COMPLEX_H

#include "Complex.h"
#include <PL/DBRep.h>

namespace LOFAR {
	namespace PL {


// The DBRep< Complex > structure is a contigious representation of
// all fields that should be stored to the database
template <>
struct DBRep< Complex > {
	void bindCols(dtl::BoundIOs& cols);
	double			value_re;
	double			value_im;
};

	} // close namespace PL
}	// close namespace LOFAR

#include "PO_Complex.tcc"	// Include template code

#endif
