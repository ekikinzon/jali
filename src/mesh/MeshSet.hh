//
// Copyright Los Alamos National Security, LLC 2009-2015
// All rights reserved. See Copyright notice in main directory
//

#ifndef _JALI_MESHSET_H_
#define _JALI_MESHSET_H_

#include <vector>
#include <algorithm>
#include <memory>
#include <string>
#include <cassert>

#include "mpi.h"

#include "MeshDefs.hh"

namespace Jali {

/*! 
  @class MeshSet "MeshSet.hh"
  @brief Encapsulates notion of a meshset (a list of mesh entities)
  
  Mesh sets are groupings of mesh entities (typically cells, faces or
  nodes but can be anything) on a compute node and can be used to
  represent materials or boundary conditions. They are lightweight
  structures that are merely lists of entities.

  
  **** IMPORTANT NOTE ABOUT CONSTANTNESS OF THIS CLASS ****
  Instantiating a const version of this class only guarantees that
  the underlying mesh topology and geometry does not change (the
  public interface conforms strictly to this definition). However,
  for purposes of memory savings we use lazy initialization and
  caching of face data, edge data, geometry quantities, columns etc.,
  which means that these data may still change. We also cannot
  initialize the cached quantities in the constructor since they
  depend on initialization of data structures in the derived class -
  however, the base class gets constructed before the derived class
  gets constructed so it is not possible without more obscure
  acrobatics. This is why some of the caching data declarations are
  declared with the keyword 'mutable' and routines that modify the
  mutable data are declared with a constant qualifier.
  
*/

// forward declaration of the mesh class

class Mesh;

class MeshSet {

 public:

  /// @brief Constructor

  MeshSet(std::string const& name,
          Mesh& parent_mesh,
          Entity_kind const& kind,
          Entity_ID_List const& owned_entities,
          Entity_ID_List const& ghost_entities,
          bool build_reverse_map = true);


  /// @brief Copy Constructor

  MeshSet(MeshSet const &meshset_in) :
      name_(meshset_in.name_),
      mesh_(meshset_in.mesh_),
      kind_(meshset_in.kind_),
      entityids_owned_(meshset_in.entityids_owned_),
      entityids_ghost_(meshset_in.entityids_ghost_),
      entityids_all_(meshset_in.entityids_all_),
      mesh2subset_(meshset_in.mesh2subset_) {}

  /// @brief Assignment operator - deleted because we cannot reassign the 
  /// reference to the Mesh

  MeshSet & operator=(MeshSet const &meshset_in) = delete;

  /// @brief Destructor

  ~MeshSet() {}

  /// @brief The mesh that this set belongs to

  Mesh& mesh() {
    return mesh_;
  }

  /// @brief Name of set

  std::string const& name() const {
    return name_;
  }

  /// @brief Rename set

  void set_name(std::string name) {
    name_ = name;
  }

  /// @brief Kind of entities in set

  Entity_kind kind() const {
    return kind_;
  }

  //
  // General mesh set information
  // -------------------------
  //

  /*!
    @brief Number of entities of a particular type
  */

  unsigned int num_entities(Entity_type type = Entity_type::ALL) const {
    if (type == Entity_type::PARALLEL_OWNED)
      return entityids_owned_.size();
    else if (type == Entity_type::PARALLEL_GHOST)
      return entityids_ghost_.size();
    else if (type == Entity_type::ALL)
      return entityids_all_.size();
    else
      return 0;
  }

  /*! 
    @brief Number of entities of a particular kind and type
    @param kind Entity_kind of the entities (CELL, NODE, WEDGE etc)
    @param parallel_type Entity_type of entities (PARALLEL_OWNED,
    PARALLEL_GHOST, ALL)

    Number of entities of a particular kind and type - this form is
    needed for defining statevectors on meshes - non-zero only if
    asking about kind of entities set is templated on.
  */

  unsigned int num_entities(Entity_kind kind,
                            Entity_type type = Entity_type::ALL)
      const {
    assert(kind_ == kind);
    return num_entities(type);
  }


  /// @brief mesh entities of the set
  
  template<Entity_type ptype = Entity_type::ALL>
  std::vector<Entity_ID> const & entities() const {
    if (ptype == Entity_type::PARALLEL_OWNED)
      return entityids_owned_;
    else if (ptype == Entity_type::PARALLEL_GHOST)
      return entityids_ghost_;
    else if (ptype == Entity_type::ALL)
      return entityids_all_;
    else
      return dummylist_;
  }

  
  /// @brief check if mesh entity index is in meshset

  Entity_ID index_in_set(Entity_ID const& mesh_entity) const {
    return (mesh2subset_.size() ? mesh2subset_[mesh_entity] : -1);
  }
  
  /// @brief Union of arbitrary number of mesh sets
  ///
  /// @param inpsets     Sets to be unioned
  /// @param temporary   Whether the new set is a temporary set (meaning it does
  /// not need to be added to the mesh)
  
  friend
  std::shared_ptr<MeshSet>
  merge(std::vector<std::shared_ptr<MeshSet>> const& inpsets,
        bool temporary = false);

  /// @brief Subtraction of arbitrary number mesh sets from a first mesh set
  ///
  /// @param set1   First set
  /// @param subtractsets   Sets to be subtracted from first set
  /// @param temporary   Whether the new set is a temporary set (meaning it does
  /// not need to be added to the mesh)

  friend
  std::shared_ptr<MeshSet>
  subtract(std::shared_ptr<MeshSet> const& set1,
           std::vector<std::shared_ptr<MeshSet>> const& subtractsets,
           bool temporary = false);

  /// @brief intersection of arbitrary number of mesh sets
  ///
  /// @param inpsets  Sets whose intersection we want
  /// @param temporary   Whether the new set is a temporary set (meaning it does
  /// not need to be added to the mesh)

  friend
  std::shared_ptr<MeshSet>
  intersect(std::vector<std::shared_ptr<MeshSet>> const& inpsets,
            bool temporary = false);

  /// @brief complement of sets (all mesh entities not in union of sets)
  ///
  /// @param inpsets     Sets whose complement we want
  /// @param temporary   Whether the new set is a temporary set (meaning it does
  /// not need to be added to the mesh)

  friend
  std::shared_ptr<MeshSet>
  complement(std::vector<std::shared_ptr<MeshSet>> inpsets,
             bool temporary = false);
 private:

  // Data

  std::string name_;
  Mesh& mesh_;
  Entity_kind kind_;

  Entity_ID_List entityids_owned_, entityids_ghost_, entityids_all_;
  Entity_ID_List dummylist_;

  Entity_ID_List mesh2subset_;

  // Make the State class a friend so that it can access protected
  // methods for retrieving and storing mesh fields

  friend class State;


};  // End class MeshSet



// @brief MeshSet factory
//
// Standalone function to make a set and return a pointer to it so
// that Mesh.hh can use a forward declaration of MeshSet and this
// function to create new sets

std::shared_ptr<MeshSet> make_meshset(std::string const& name,
                                      Mesh& parent_mesh,
                                      Entity_kind const& kind,
                                      Entity_ID_List const& owned_entities,
                                      Entity_ID_List const& ghost_entities,
                                      bool with_reverse_map = true);


}  // end namespace Jali





#endif /* _JALI_MESHSET_H_ */