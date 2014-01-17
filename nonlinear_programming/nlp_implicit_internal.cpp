/*
 *    This file is part of CasADi.
 *
 *    CasADi -- A symbolic framework for dynamic optimization.
 *    Copyright (C) 2010 by Joel Andersson, Moritz Diehl, K.U.Leuven. All rights reserved.
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

#include "nlp_implicit_internal.hpp"

#include "symbolic/mx/mx_tools.hpp"
#include "symbolic/fx/mx_function.hpp"

using namespace std;
namespace CasADi {
  
  NLPImplicitInternal::NLPImplicitInternal(const FX& f, const FX& jac, const LinearSolver& linsol) : ImplicitFunctionInternal(f,jac,linsol) {
    addOption("nlp_solver",               OT_NLPSOLVER,  GenericType(), "The NLPSolver used to solve the implicit system.");
    addOption("nlp_solver_options",       OT_DICTIONARY, GenericType(), "Options to be passed to the NLPSolver");
  }

  NLPImplicitInternal::~NLPImplicitInternal(){ 
  }

  void NLPImplicitInternal::deepCopyMembers(std::map<SharedObjectNode*,SharedObject>& already_copied){
    ImplicitFunctionInternal::deepCopyMembers(already_copied);
    nlp_solver_ = deepcopy(nlp_solver_,already_copied);
  }

  void NLPImplicitInternal::solveNonLinear() {
    // Obtain initial guess
    nlp_solver_.input(NLP_SOLVER_X0).set(output(iout_));
  
    // Add other arguments
    for(int i=0, k=0; i<getNumInputs(); ++i){
      if(i!=iin_){
        std::copy(input(i).begin(),input(i).end(),nlp_solver_.input(NLP_SOLVER_P).begin()+k);
        k += input(i).size();
      }
    }
  
    // Solve NLP
    nlp_solver_.evaluate();
    
    stats_["nlp_solver_stats"] = nlp_solver_.getStats();

    // Copy the outputs
    output(iout_).set(nlp_solver_.output(NLP_SOLVER_X));
  }

  void NLPImplicitInternal::init(){

    // Call the base class initializer
    ImplicitFunctionInternal::init();

  
    MX V = msym("V",input(iin_).sparsity());
  
    std::vector<CRSSparsity> sps;
    for(int i=0; i<getNumInputs(); ++i)
      if(i!=iin_) sps.push_back(input(i).sparsity());
  
    // So that we can pass it on to createParent
    std::pair<MX,std::vector<MX> > mypair = createParent(sps);

    // V groups all parameters in an MX
    MX P(mypair.first);
    std::vector< MX > inputs(mypair.second);
    
    // Dummy objective
    MX nlp_F = 0;

    // Constraints
    std::vector< MX > args_call;
    args_call.push_back(V);
    args_call.insert(args_call.end(),mypair.second.begin(),mypair.second.end());
    MX nlp_G = f_.call(args_call).front();

    // We're going to use two-argument objective and constraints to allow the use of parameters
    MXFunction nlp(nlpIn("x",V,"p",P),nlpOut("f",nlp_F,"g",nlp_G));
  
    // Create an nlpsolver instance
    NLPSolverCreator nlp_solvercreator = getOption("nlp_solver");
    nlp_solver_ = nlp_solvercreator(nlp);
    if(hasSetOption("nlp_solver_options")){
      nlp_solver_.setOption(getOption("nlp_solver_options"));
    }
  
    // Initialize the NLP solver
    nlp_solver_.init();
  
  
    nlp_solver_.input(NLP_SOLVER_LBG).setAll(0);
    nlp_solver_.input(NLP_SOLVER_UBG).setAll(0);
    
    for (int k=0;k<u_c_.size();++k) {
       if (u_c_[k] < 0) {
         nlp_solver_.input(NLP_SOLVER_LBX)[k] = -std::numeric_limits<double>::infinity();
         nlp_solver_.input(NLP_SOLVER_UBX)[k] = 0;
       } else if (u_c_[k] > 0) {
         nlp_solver_.input(NLP_SOLVER_LBX)[k] = 0;
         nlp_solver_.input(NLP_SOLVER_UBX)[k] = std::numeric_limits<double>::infinity();
       }
    }
  
  }

} // namespace CasADi

