//# This file was generated by genDBcode v2.9 on Mon Mar 15 11:52:59 CET 2004
//# with the command: genDBcode P ./tAttrib.plmap.proto  
//# from the directory: /home/loose/LOFAR/LCS/databases/PL/test
//#
//# EDITING THIS FILE MANUALLY IS AT YOUR OWN RISK
//# IT MIGHT BE OVERWRITTEN BY THE NEXT MAKE OF YOUR PROJECT
//#
//# only include this file once
#if !defined(PO_040315115259_TATTRIB_H)
#define PO_040315115259_TATTRIB_H

#include "tAttrib.h"
#include <PL/DBRep.h>

namespace LOFAR {
	namespace PL {


// The DBRep< A > structure is a contigious representation of
// all fields that should be stored to the database
template <>
struct DBRep< A > {
	void bindCols(dtl::BoundIOs& cols);
	string			s;
};


// The DBRep< B > structure is a contigious representation of
// all fields that should be stored to the database
template <>
struct DBRep< B > {
	void bindCols(dtl::BoundIOs& cols);
	string			s;
};


// The DBRep< C > structure is a contigious representation of
// all fields that should be stored to the database
template <>
struct DBRep< C > {
	void bindCols(dtl::BoundIOs& cols);
	string			s;
};


// The DBRep< D > structure is a contigious representation of
// all fields that should be stored to the database
template <>
struct DBRep< D > {
	void bindCols(dtl::BoundIOs& cols);
	string			s;
};


// The DBRep< E > structure is a contigious representation of
// all fields that should be stored to the database
template <>
struct DBRep< E > {
	void bindCols(dtl::BoundIOs& cols);
	string			s;
};


// The DBRep< F > structure is a contigious representation of
// all fields that should be stored to the database
template <>
struct DBRep< F > {
	void bindCols(dtl::BoundIOs& cols);
	string			s;
};


// The DBRep< G > structure is a contigious representation of
// all fields that should be stored to the database
template <>
struct DBRep< G > {
	void bindCols(dtl::BoundIOs& cols);
	string			s;
};


	} // close namespace PL
}	// close namespace LOFAR

#include "PO_tAttrib.tcc"	// Include template code

#endif
