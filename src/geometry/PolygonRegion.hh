/*
Copyright (c) 2017, Los Alamos National Security, LLC
All rights reserved.

Copyright 2017. Los Alamos National Security, LLC. This software was
produced under U.S. Government contract DE-AC52-06NA25396 for Los
Alamos National Laboratory (LANL), which is operated by Los Alamos
National Security, LLC for the U.S. Department of Energy. The
U.S. Government has rights to use, reproduce, and distribute this
software.  NEITHER THE GOVERNMENT NOR LOS ALAMOS NATIONAL SECURITY,
LLC MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LIABILITY
FOR THE USE OF THIS SOFTWARE.  If software is modified to produce
derivative works, such modified software should be clearly marked, so
as not to confuse it with the version available from LANL.
 
Additionally, redistribution and use in source and binary forms, with
or without modification, are permitted provided that the following
conditions are met:

1.  Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2.  Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3.  Neither the name of Los Alamos National Security, LLC, Los Alamos
National Laboratory, LANL, the U.S. Government, nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY LOS ALAMOS NATIONAL SECURITY, LLC AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LOS
ALAMOS NATIONAL SECURITY, LLC OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


/**
 * @file   PolygonRegion.hh
 * @author Rao Garimella
 * @date Wed Sep 28 08:54:19 2011
 *
 * @brief  Declaration of PolygonRegion class
 *
 *
 */

#ifndef _PolygonRegion_hh_
#define _PolygonRegion_hh_

#include "Region.hh"

namespace JaliGeometry {

// -------------------------------------------------------------
//  class PolygonRegion
// -------------------------------------------------------------
/// A closed polygonal segment of a plane

class PolygonRegion : public Region {
public:

  /// Default constructor uses two corner points (order not important).

  PolygonRegion(const std::string name, const unsigned int id,
                const unsigned int npolypoints,
                const std::vector<Point>& polypoints,
                const LifeCycle_type lifecycle = LifeCycle_type::PERMANENT);

  /// Protected copy constructor to avoid unwanted copies.
  PolygonRegion(const PolygonRegion& old);

  /// Destructor
  ~PolygonRegion(void);


  // Type of the region
  inline
  Region_type type() const { return Region_type::POLYGON; }

  inline
  unsigned int num_points() const { return num_points_; }

  inline
  const std::vector<Point>& points() const { return points_; }

  inline
  const Point& normal() const { return normal_; }

  /// Is the the specified point inside this region
  bool inside(const Point& p) const;

protected:

  const unsigned int num_points_;    /* Number of points defining polygon */
  const std::vector<Point> points_;  /* Points of the polygon */
  Point normal_;                     /* Normal to the polygon */
  unsigned int elim_dir_;            /* Coord dir to eliminate while projecting
                                        polygon for in/out tests
                                        0 - yz, eliminate x coord
                                        1 - xz, eliminate y coord
                                        2 - xy, eliminate z coord        */

private:
  void init();
};

/// A smart pointer to PolygonRegion instances
//
// typedef Teuchos::RCP<PolygonRegion> PolygonRegionPtr;

// RVG: I am not able to correctly code a region factory using smart
// pointers so I will revert to a simpler definition

typedef PolygonRegion *PolygonRegionPtr;

} // namespace JaliGeometry


#endif
