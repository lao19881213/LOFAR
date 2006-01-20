//#  UnaryExprNode.h: Unary expression node.
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

#ifndef LOFAR_PL_QUERY_UNARYEXPRNODE_H
#define LOFAR_PL_QUERY_UNARYEXPRNODE_H

// \file
// Unary expression node.

//# Includes
//# Never #include <config.h> or #include <lofar_config.h> in a header file!
#include <PL/Query/Expr.h>
#include <PL/Query/ExprNode.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
  namespace PL
  {
    namespace Query
    {
      // \ingroup QueryExpr
      // \defgroup UnaryExprNode Unary Expression Nodes
      // A unary expression is an expression that takes one operator and one
      // operand.
      // @{

      // This class represents a unary expression node. A unary expression is
      // an expression that takes one operator and one operand.
      class UnaryExprNode : public ExprNode
      {
      public:
        // Construct a unary expression node.
        UnaryExprNode(const string& oper, 
                      const Expr& value);

        virtual ~UnaryExprNode();

        virtual void print(ostream& os) const;

        virtual Expr getConstraint() const;

      private:

        // The operation
        const string itsOperation;

        // The operand
        const Expr        itsOperand;

      };

      // @}

    } // namespace Query

  } // namespace PL

} // namespace LOFAR

#endif
