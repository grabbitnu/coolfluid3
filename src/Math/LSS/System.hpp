// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Math_LSS_System_hpp
#define CF_Math_LSS_System_hpp

////////////////////////////////////////////////////////////////////////////////////////////

//#include <boost/utility.hpp>

//#include "Math/LibMath.hpp"
//#include "Common/MPI/PE.hpp"
//#include "Common/Component.hpp"
//#include "Common/MPI/CommPattern.hpp"
#include "Math/LSS/BlockAccumulator.hpp"
#include "Math/LSS/Matrix.hpp"
#include "Math/LSS/Vector.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
  @file System.hpp main header of the linear system solver interface.
  @brief This header collects all the headers needed for the linear system solver, also including configure-time present dependency specializations.
  @author Tamas Banyai

  The structure is organized into four main unit:
  * Vector: designed to wrap the right hand side and the solution
  * Matrix: obviously, the matrix
  * System: encapsulation of a matrix and two vectors
  * BlockAccumulator: a little class to collect "element-wise" data, in order to reduce cache misses

  Notation:
  * eq: size of the equation system (for example 2D incompressible navier stokes has neq=3, for solving p,u,v)
  * block: an nequation*nequation matrix
  * blockrow/blockcol: ith block in the row/column
  * row/col: ith actual row/column in the matrix (translates as irow=iblockrow*neq+ieq or the inverse iblockrow=irow/neq ieq=irow%neq)

  Include System.h in order to include everything related to linear system solver.
  In case the underlying dependency works with closed data, therefore the create function of the vector takes an LSS::Matrix as an argument.
  Also, the actual implementation of solve is in LSS::Matrix taking two LSS::Vector arguments as rhs and solution.
  For performance reasons, dynamic buld of a matrix is not allowed, If you change your matrix, collect your changes into commpattern + connectiviy and call create again.
**/

////////////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Math {
namespace LSS {

////////////////////////////////////////////////////////////////////////////////////////////

class Math_API System : public Common::Component {
public:

  /// @name CREATION, DESTRUCTION AND COMPONENT SYSTEM
  //@{

  /// pointer to this type
  typedef boost::shared_ptr<System> Ptr;

  /// const pointer to this type
  typedef boost::shared_ptr<System const> ConstPtr;

  /// name of the type
  static std::string type_name () { return "System"; }

  /// Default constructor
  System(const std::string& name);

  /// Setup sparsity structure
  /// @todo action for it
  inline void create(Comm::CommPattern& cp, Uint neq, std::vector<Uint>& node_connectivity, std::vector<Uint>& starting_indices);

  /// Exchange to existing matrix and vectors
  /// @todo action for it
  inline void swap(LSS::Matrix::Ptr matrix, LSS::Vector::Ptr solution, LSS::Vector::Ptr rhs);

  /// Deallocate underlying data
  inline void destroy();

  //@} END CREATION, DESTRUCTION AND COMPONENT SYSTEM

  /// @name SOLVE THE SYSTEM
  //@{

  /// solving the system
  /// @todo action for it
  inline void solve();

  //@} END SOLVE THE SYSTEM

  /// @name EFFICCIENT ACCESS
  //@{

  /// Set a list of values
  inline void set_values(const BlockAccumulator& values);

  /// Add a list of values
  inline void add_values(const BlockAccumulator& values);

  /// Add a list of values
  inline void get_values(BlockAccumulator& values);

  /// Apply dirichlet-type boundary conditions.
  /// When preserve_symmetry is true than blockrow*numequations+eq column is is zeroed by moving it to the right hand side (however this usually results in performance penalties).
  inline void dirichlet(const Uint iblockrow, const Uint ieq, bool preserve_symmetry=false);

  /// Applying periodicity by adding one line to another and dirichlet-style fixing it to
  /// Note that prerequisite for this is to work that the matrix sparsity should be compatible (same nonzero pattern for the two blocks).
  /// Note that only structural symmetry can be preserved (again, if sparsity input was symmetric).
  inline void periodicity (const Uint iblockrow_to, const Uint iblockrow_from);

  /// Set the diagonal
  inline void set_diagonal(const std::vector<Real>& diag);

  /// Add to the diagonal
  inline void add_diagonal(const std::vector<Real>& diag);

  /// Get the diagonal
  inline void get_diagonal(std::vector<Real>& diag);

  /// Reset Matrix
  inline void reset(Real reset_to=0.);

  //@} END EFFICCIENT ACCESS

  /// @name MISCELLANEOUS
  //@{

  /// Print to wherever
  inline void print(std::iostream& stream);

  /// Print to file given by filename
  inline void print(const std::string& filename);

  /// Accessor to matrix
  inline LSS::Matrix::ConstPtr matrix() { return m_mat; };

  /// Accessor to right hand side
  inline LSS::Vector::ConstPtr rhs() { return m_rhs; };

  /// Accessor to solution
  inline LSS::Vector::ConstPtr sol() { return m_sol; };

  /// Accessor to the state of create
  inline const bool is_created() { return m_is_created; };

  //@} END MISCELLANEOUS

private:

  /// flag if the system is created or not
  bool m_is_created;

  /// shared_ptr to system matrix
  LSS::Matrix::Ptr m_mat;

  /// shared_ptr to solution vectoe
  LSS::Vector::Ptr m_sol;

  /// shared_ptr to right hand side vector
  LSS::Vector::Ptr m_rhs;

}; // end of class System

////////////////////////////////////////////////////////////////////////////////////////////

} // namespace LSS
} // namespace Math
} // namespace CF

#endif // CF_Math_LSS_System_hpp
