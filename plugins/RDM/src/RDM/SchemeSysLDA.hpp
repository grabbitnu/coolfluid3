// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_SchemeLDA_hpp
#define CF_Solver_SchemeLDA_hpp

#include <boost/assign.hpp>

#include <Eigen/Dense>

#include "Common/Core.hpp"
#include "Common/OptionT.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/ElementData.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/ElementType.hpp"

#include "Solver/Actions/CLoopOperation.hpp"

#include "RDM/LibRDM.hpp"
#include "RDM/FluxOp2D.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

template < typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS >
class RDM_API SchemeLDA : public Solver::Actions::CLoopOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr< SchemeLDA > Ptr;
  typedef boost::shared_ptr< SchemeLDA const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  SchemeLDA ( const std::string& name );

  /// Virtual destructor
  virtual ~SchemeLDA() {};

  /// Get the class name
  static std::string type_name () { return "SchemeLDA<" + SHAPEFUNC::type_name() + ">"; }
	
  /// execute the action
  virtual void execute ();

private: // helper functions

  void change_elements()
  { 
    /// @todo improve this (ugly)

    connectivity_table = elements().as_ptr<Mesh::CElements>()->connectivity_table().as_ptr< Mesh::CTable<Uint> >();
    coordinates = elements().nodes().coordinates().as_ptr< Mesh::CTable<Real> >();

    cf_assert( is_not_null(connectivity_table) );

    /// @todo modify these to option components configured from

    Mesh::CField2::Ptr csolution = Common::find_component_ptr_recursively_with_tag<Mesh::CField2>( *Common::Core::instance().root(), "solution" );
    cf_assert( is_not_null( csolution ) );
    solution = csolution->data_ptr();

    Mesh::CField2::Ptr cresidual = Common::find_component_ptr_recursively_with_tag<Mesh::CField2>( *Common::Core::instance().root(), "residual" );
    cf_assert( is_not_null( cresidual ) );
    residual = cresidual->data_ptr();

    Mesh::CField2::Ptr cwave_speed = Common::find_component_ptr_recursively_with_tag<Mesh::CField2>( *Common::Core::instance().root(), "wave_speed" );
    cf_assert( is_not_null( cwave_speed ) );
    wave_speed = cwave_speed->data_ptr();
  }


private: // data

  Mesh::CTable<Uint>::Ptr connectivity_table;

  Mesh::CTable<Real>::Ptr coordinates;

  Mesh::CTable<Real>::Ptr solution;
  Mesh::CTable<Real>::Ptr residual;
  Mesh::CTable<Real>::Ptr wave_speed;

  typedef FluxOp2D<SHAPEFUNC,QUADRATURE,PHYSICS> DiscreteOpType;

  DiscreteOpType m_oper;

  // Values of the solution located in the dof of the element
  // RealVector m_solution_values;
  typename DiscreteOpType::SolutionMatrixT m_solution_values;

  // node values
  typename SHAPEFUNC::NodeMatrixT m_nodes;

  // The operator L in the advection equation Lu = f
  // Matrix m_sf_oper_values stores the value L(N_i) at each quadrature point for each shape function N_i
  typename DiscreteOpType::SFMatrixT m_sf_oper_values;

  // Values of the operator L(u) computed in quadrature points. These operator L returns these values
  // multiplied by Jacobian and quadrature weight
  RealVector m_flux_oper_values;

  // Nodal residuals
  RealVector m_phi;

  //Integration factor (jacobian multiplied by quadrature weight)
  Eigen::Matrix<Real, QUADRATURE::nb_points, 1u> m_wj;

};

///////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC, typename QUADRATURE, typename PHYSICS>
SchemeLDA<SHAPEFUNC,QUADRATURE,PHYSICS>::SchemeLDA ( const std::string& name ) :
  CLoopOperation(name)
{
  regist_typeinfo(this);

  m_properties["Elements"].as_option().attach_trigger ( boost::bind ( &SchemeLDA<SHAPEFUNC,QUADRATURE,PHYSICS>::change_elements, this ) );

  m_flux_oper_values.resize(QUADRATURE::nb_points);
  m_phi.resize(SHAPEFUNC::nb_nodes);
}

/////////////////////////////////////////////////////////////////////////////////////

template<typename SHAPEFUNC,typename QUADRATURE, typename PHYSICS>
void SchemeLDA<SHAPEFUNC, QUADRATURE,PHYSICS>::execute()
{

//  CFinfo << "LDA ELEM [" << idx() << "]" << CFendl;

  const Mesh::CTable<Uint>::ConstRow nodes_idx = connectivity_table->array()[idx()];

//  std::cout << "nodes_idx";
//  for ( Uint i = 0; i < nodes_idx.size(); ++i)
//     std::cout << " " << nodes_idx[i];

 // copy the coordinates from the large array to a small
 Mesh::fill(m_nodes, *coordinates, nodes_idx );

//  elements().as_ptr<Mesh::CElements>()->put_coordinates( nodes, idx() );

//  std::cout << "field put_coordinates function" <<  std::endl;
//  std::cout << "nodes: " << nodes << std::endl;


//  std::cout << "mesh::fill function" <<  std::endl;
//  std::cout << "nodes: " << nodes << std::endl;


  // copy the solution from the large array to a small
 for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
   for(Uint v = 0; v < PHYSICS::nbeqs(); ++v)
      m_solution_values(n,v) = (*solution)[nodes_idx[n]][v];


 m_phi.setZero();

 m_oper.compute(m_nodes,m_solution_values, m_sf_oper_values, m_flux_oper_values,m_wj);

// std::cout << "solution_values  [" << m_solution_values << "]" << std::endl;
// std::cout << "sf_oper_values   [" << m_sf_oper_values << "]" << std::endl;
// std::cout << std::endl;
// std::cout << "flux_oper_values [" << m_flux_oper_values << "]" << std::endl;
//// if (m_flux_oper_values.norm() > 0.0)
////     std::cin.get();
// std::cout << std::endl;

for(Uint q = 0; q < QUADRATURE::nb_points; ++q)

  PHYSICS::compute_param(pparams, xq, uq);

  PHYSICS::LuPlus( pparams, xq, uq, grad_phi, Lup, evalues );
// x - x[dim] coordinates @ q
// u - u[eqs] solution    @ q
// grad_phi - gphi[eqs,dim] gradient  @ q
// Lup - Lu[i][eqs,eqs] projected jacobian over grad sf @ q (per i sf )
// evalues - ev[eqs] eigen values of the jacobian

PHYSICS::Lu( pparams, xq, uq, grad_u, Lu   );
// x - x[dim] coordinates @ q
// u - u[eqs] solution    @ q
// grad_u - gradu[eqs,dim] gradient  @ q
// Lu - Lu[eqs] residum @ q (summed over i sf)

// for(Uint q = 0; q < QUADRATURE::nb_points; ++q)
// {
//   Real sumLplus = 0.0;
//   for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
//   {
//     sumLplus += std::max(0.0,m_sf_oper_values(q,n));
//   }

//   for(Uint n = 0; n < SHAPEFUNC::nb_nodes; ++n)
//   {
//     m_phi[n] += std::max(0.0,m_sf_oper_values(q,n))/sumLplus * m_flux_oper_values[q] * m_wj[q];
//   }
// }
  
//   std::cout << "phi [";
//   for (Uint n=0; n < SHAPEFUNC::nb_nodes; ++n)
//      std::cout << m_phi[n] << " ";
//   std::cout << "]" << std::endl;

  for (Uint n=0; n<SHAPEFUNC::nb_nodes; ++n)
    (*residual)[nodes_idx[n]][0] += m_phi[n];

//  std::cout << "residual [";
//  for (Uint n=0; n < SHAPEFUNC::nb_nodes; ++n)
//     std::cout << (*residual)[nodes_idx[n]][0] << " ";
//  std::cout << "]" << std::endl;


  // computing average advection speed on element

	typename SHAPEFUNC::CoordsT centroid;
	
	centroid.setZero();

  for (Uint n=0; n<SHAPEFUNC::nb_nodes; ++n)
  {
    centroid[XX] += m_nodes(n, XX);
    centroid[YY] += m_nodes(n, YY);
  }
  centroid /= SHAPEFUNC::nb_nodes;


  // compute a bounding box of the element:

  Real xmin = m_nodes(0, XX);
  Real xmax = m_nodes(0, XX);
  Real ymin = m_nodes(0, YY);
  Real ymax = m_nodes(0, YY);

  for(Uint inode = 1; inode < SHAPEFUNC::nb_nodes; ++inode)
  {
    xmin = std::min(xmin,m_nodes(inode, XX));
    xmax = std::max(xmax,m_nodes(inode, XX));

    ymin = std::min(ymin,m_nodes(inode, YY));
    ymax = std::max(ymax,m_nodes(inode, YY));

  }

  const Real dx = xmax - xmin;
  const Real dy = ymax - ymin;

  // The update coeff is updated by a product of bb radius and norm of advection velocity
  for (Uint n=0; n<SHAPEFUNC::nb_nodes; ++n)
  {
    (*wave_speed)[nodes_idx[n]][0] += std::sqrt( dx*dx+dy*dy);
//       * std::sqrt( centroid[XX]*centroid[XX] + centroid[YY]*centroid[YY] );
  }


//  std::cout << "wave_speed [";
//  for (Uint n=0; n < SHAPEFUNC::nb_nodes; ++n)
//     std::cout << (*wave_speed)[nodes_idx[n]][0] << " ";
//  std::cout << "]" << std::endl;

//  std::cout << " --------------------------------------------------------------- " << std::endl;

}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_SchemeLDA_hpp