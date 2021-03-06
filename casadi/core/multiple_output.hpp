/*
 *    This file is part of CasADi.
 *
 *    CasADi -- A symbolic framework for dynamic optimization.
 *    Copyright (C) 2010-2014 Joel Andersson, Joris Gillis, Moritz Diehl,
 *                            K.U. Leuven. All rights reserved.
 *    Copyright (C) 2011-2014 Greg Horn
 *
 *    CasADi is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 3 of the License, or (at your option) any later version.
 *
 *    CasADi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with CasADi; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#ifndef CASADI_MULTIPLE_OUTPUT_HPP
#define CASADI_MULTIPLE_OUTPUT_HPP

#include "mx_node.hpp"
#include "function.hpp"
#include <set>

/// \cond INTERNAL

namespace casadi {

  /// Forward declaration
  class OutputNode;

  /**
      \author Joel Andersson
      \date 2010
  */
  class CASADI_EXPORT MultipleOutput : public MXNode {
    friend class OutputNode;
  public:

    /** \brief  Constructor */
    MultipleOutput();

    /** \brief  Destructor */
    ~MultipleOutput() override;

    /** \brief  Number of outputs */
    int nout() const override=0;

    /** \brief  Get an output */
    MX getOutput(int oind) const override;

    /** \brief  Get the sparsity of output oind */
    const Sparsity& sparsity(int oind) const override=0;

    /** \brief  Check if a multiple output node */
    bool isMultipleOutput() const override {return true;}

  };

  class CASADI_EXPORT OutputNode : public MXNode {
  public:

    /** \brief  Constructor */
    OutputNode(const MX& parent, int oind);

    /** \brief  Destructor */
    ~OutputNode() override;

    /** \brief  Print expression */
    std::string print(const std::vector<std::string>& arg) const override;

    /** \brief Is the node nonlinear */
    bool isNonLinear() override {return true;}

    /** \brief  Check if evaluation output */
    bool isOutputNode() const override {return true;}

    /** \brief  Get function input */
    int getFunction_input() const override { return -1;}

    /** \brief  Get function output */
    int getFunctionOutput() const override { return oind_;}

    /** \brief Get the operation */
    int op() const override { return -1;}

    /// Create a horizontal concatenation node
    MX getHorzcat(const std::vector<MX>& x) const override { return dep()->getHorzcat(x);}

    /// Create a vertical concatenation node (vectors only)
    MX getVertcat(const std::vector<MX>& x) const override { return dep()->getVertcat(x);}

    /** \brief  Output index */
    int oind_;
  };

} // namespace casadi
/// \endcond

#endif // CASADI_MULTIPLE_OUTPUT_HPP
