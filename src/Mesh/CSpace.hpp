// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CSpace_hpp
#define CF_Mesh_CSpace_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Mesh/LibMesh.hpp"
#include "Mesh/CTable.hpp"

namespace CF {
namespace Mesh {

  class ElementType;
  class CElements;

////////////////////////////////////////////////////////////////////////////////

/// CSpace component class
/// This class stores information about a set of elements of the same type
/// @author Willem Deconinck, Tiago Quintino, Bart Janssens
class Mesh_API CSpace : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<CSpace> Ptr;
  typedef boost::shared_ptr<CSpace const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CSpace ( const std::string& name );
  
  /// Virtual destructor
  virtual ~CSpace();

  /// Get the class name
  static std::string type_name () { return "CSpace"; }

  /// initialize
  void initialize(const std::string& shape_function_builder_name);

  void initialize(const CElements& elements);

  /// return the elementType
  const ElementType& shape_function() const { return *m_shape_function; }

  /// return the number of elements
  Uint size() const { return m_connectivity_table->size(); }
  
  CTable<Uint>::ConstRow get_nodes(const Uint elem_idx) ;

  /// Mutable access to the connectivity table
  CTable<Uint>& connectivity_table() { return *m_connectivity_table; }
  
  /// Const access to the connectivity table
  const CTable<Uint>& connectivity_table() const { return *m_connectivity_table; }

  Uint nb_states() const { return 1u; }
  
protected: // data

  boost::shared_ptr<ElementType> m_shape_function;
  
  CTable<Uint>::Ptr m_connectivity_table;

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CSpace_hpp