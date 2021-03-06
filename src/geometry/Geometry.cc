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

#include "Geometry.hh"

#include <math.h>

  namespace JaliGeometry
  {

    // Return the volume and centroid of a general polyhedron
    //
    // ccoords  - vertices of the polyhedron (in no particular order)
    // nf       - number of faces of polyhedron
    // nfnodes  - number of nodes for each face
    // fcoords  - linear array of face coordinates in in ccw manner
    //            assuming normal of face is pointing out (
    //
    // So if the polyhedron has 5 faces with 5,3,3,3 and 3 nodes each
    // then entries 1-5 of fcoords describes face 1, entries 6-9
    // describes face 2 and so on
    //
    // So much common work has to be done for computing the centroid
    // and volume calculations that they have been combined into one
    //
    // The volume of all polyhedra except tets is computed as a sum of
    // volumes of tets created by connecting the polyhedron center to
    // a face center and an edge of the face


    void polyhed_get_vol_centroid(const std::vector<Point> ccoords,
                                  const unsigned int nf,
                                  const std::vector<unsigned int> nfnodes,
                                  const std::vector<Point> fcoords,
                                  double *volume,
                                  Point *centroid)
    {
      Point v1(3), v2(3), v3(3);
      bool negvol = false;


      // Initialize to sane values

      centroid->set(0.0);
      (*volume) = 0.0;

      // Compute the geometric center of all face nodes

      int np = ccoords.size();
      if (np < 4) {
        std::cout << "Not a polyhedron" << std::endl;
        return;
      }

      if (np == 4) {  // is a tetrahedron

        *centroid = (ccoords[0]+ccoords[1]+ccoords[2]+ccoords[3])/4.0;
        v1 = ccoords[1]-ccoords[0];
        v2 = ccoords[2]-ccoords[0];
        v3 = ccoords[3]-ccoords[0];
        *volume = (v1^v2)*v3;

      } else {  // if (np > 4), polyhedron with possibly curved faces

        Point center(0.0, 0.0, 0.0);
        for (int i = 0; i < np; ++i)
          center += ccoords[i];
        center /= np;

        int offset = 0;
        for (int i = 0; i < nf; ++i) {
          Point tcentroid(3);
          double tvolume;

          if (nfnodes[i] == 3) {

              tcentroid = (center+fcoords[offset]+fcoords[offset+1]+
                           fcoords[offset+2])/4.0;
              v1 = fcoords[offset]-center;
              v2 = fcoords[offset+1]-center;
              v3 = fcoords[offset+2]-center;
              tvolume = (v1^v2)*v3;

              if (tvolume <= 0.0) negvol = true;

              (*centroid) += tvolume*tcentroid;      // sum up 1st moment
              (*volume) += tvolume;                  // sum up 0th moment

          } else {
            // geometric center of all face nodes

            Point fcenter(0.0, 0.0, 0.0);
            for (int j = 0; j < nfnodes[i]; ++j)
              fcenter += fcoords[offset+j];
            fcenter /= nfnodes[i];

            for (int j = 0; j < nfnodes[i]; ++j) {  // for each edge of face

              // form tet from edge of face, face center and cell center

              int k, kp1;

              k = offset+j;
              kp1 = offset+(j+1)%nfnodes[i];

              tcentroid = (center+fcenter+fcoords[k]+fcoords[kp1])/4.0;
              v1 = fcoords[k]-center;
              v2 = fcoords[kp1]-center;
              v3 = fcenter-center;
              tvolume = (v1^v2)*v3;

              if (tvolume <= 0.0) negvol = true;

              (*centroid) += tvolume*tcentroid;      // sum up 1st moment
              (*volume) += tvolume;                  // sum up 0th moment

            }  // for each edge of face
          }

          offset += nfnodes[i];

        }  // for each face


        (*centroid) /= (*volume);  // centroid = 1st moment / 0th moment
      }  // end if (np > 4)

      (*volume) /= 6;  // Account for multiplier here rather than in
                       // computation of each tet

      if (negvol) {  // one of the subtets was inverted. Label the volume
                     // total as negative so that calling applications
                     // understand that this is an invalid element
        if (*volume > 0.0)
          (*volume) = -(*volume);
      }

    }  // polyhed_get_vol_centroid



    // Checks if point is inside polyhedron
    //
    // ccoords  - vertices of the polyhedron (in no particular order)
    // nf       - number of faces of polyhedron
    // nfnodes  - number of nodes for each face
    // fcoords  - linear array of face coordinates in in ccw manner
    //            assuming normal of face is pointing out (
    //
    // So if the polyhedron has 5 faces with 5,3,3,3 and 3 nodes each
    // then entries 1-5 of fcoords describes face 1, entries 6-9
    // describes face 2 and so on
    //
    // Assuming that the polyhedron's faces can be broken into
    // triangular subfaces, this routine checks that the test point
    // forms a positive volume with each triangular subface


    bool point_in_polyhed(const Point testpnt,
                          const std::vector<Point> ccoords,
                          const unsigned int nf,
                          const std::vector<unsigned int> nfnodes,
                          const std::vector<Point> fcoords) {

      int np = ccoords.size();
      if (np < 4) {
        std::cout << "Not a polyhedron" << std::endl;
        return false;
      }

      int offset = 0;
      for (int i = 0; i < nf; i++) {

        Point v1(3), v2(3), v3(3);
        double tvolume;

        if (nfnodes[i] == 3) {

          v1 = fcoords[offset]-testpnt;
          v2 = fcoords[offset+1]-testpnt;
          v3 = fcoords[offset+2]-testpnt;
          tvolume = (v1^v2)*v3;

          if (tvolume < 0.0)
            return false;
        } else {

          // geometric center of all face nodes

          Point fcenter(0.0, 0.0, 0.0);
          for (int j = 0; j < nfnodes[i]; j++)
            fcenter += fcoords[offset+j];
          fcenter /= nfnodes[i];

          for (int j = 0; j < nfnodes[i]; ++j) {  // for each edge of face

            // form tet from edge of face, face center and test point

            Point tcentroid(3);
            int k, kp1;

            k = offset+j;
            kp1 = offset+(j+1)%nfnodes[i];

            v1 = fcoords[k]-testpnt;
            v2 = fcoords[kp1]-testpnt;
            v3 = fcenter-testpnt;
            tvolume = (v1^v2)*v3;

            if (tvolume < 0.0)
              return false;

          }  // for each edge of face

          offset += nfnodes[i];
        }

      }  // for each face

      return true;


    }  // point_in_polyhed



    // Compute area and centroid of polygon by connecting a center
    // point to the edges of the polygon and summing the moments of
    // the resulting triangles
    //
    // Also, compute the "normal" of the polygon as the sum of the
    // area weighted normals of the triangular facets
    //
    // Cannot use the contour integral method as it might indicate that a
    // self-intersecting polygon has positive volume. This situation
    // might occur in dynamic meshes

    void polygon_get_area_centroid_normal(const std::vector<Point> coords,
                                          double *area, Point *centroid,
                                          Point *normal) {

      bool negvol = false;

      (*area) = 0;
      centroid->set(0.0);
      normal->set(0.0);

      unsigned int np = coords.size();

      if (np < 3) {
        std::cout << "Degenerate polygon - area is zero" << std::endl;
        return;
      }

      int dim = coords[0].dim();

      Point center(dim);

      // Compute a center point

      for (int i = 0; i < np; i++)
        center += coords[i];
      center /= np;

      if (coords.size() == 3) {  // triangle - straightforward
        Point v1 = coords[2]-coords[1];
        Point v2 = coords[0]-coords[1];

        (*normal) = 0.5*v1^v2;

        (*area) = norm(*normal);
        (*centroid) = center;
      } else {
        // Compute the area of each triangle formed by
        // the center point and each polygon edge

        *area = 0.0;
        for (int i = 0; i < np; i++) {
          Point v1 = coords[i]-center;
          Point v2 = coords[(i+1)%np]-center;

          Point v3 = 0.5*v1^v2;

          double area_temp = norm(v3);

          // In 2D, if the cross-product is negative, the element is inverted
          // In 3D, validity is a lot more subtle - a polygon in 3D is
          // "inverted" if its normal deviates "substantially" from the
          // average normal of the "surface" in that neighborhood - we won't
          // deal with that judgement here

          if (dim == 2 && v3[0] <= 0.0)
            negvol = true;

          (*normal) += v3;
          (*area) += area_temp;
          (*centroid) += area_temp*(coords[i]+coords[(i+1)%np]+center)/3.0;
        }

        (*centroid) /= (*area);
      }

      if (negvol) {  // one of the subtris was inverted or degenerate.
                     // Label the volume total as negative so that
                     // calling applications understand that this is an
                     // invalid element
        if (*area > 0.0)
          (*area) = -(*area);
      }

    } // polygon_get_area_centroid



    // Get area weighted normal of polygon
    // In 2D, the normal is unambiguous - the normal is evaluated at one corner
    // In 3D, the procedure evaluates the normal of each triangular facet and
    // returns the area weighted sum of the normals

    // Point polygon_get_normal(const std::vector<Point> coords)
    // {

    //   if (coords.size() < 3) {
    //     std::cout << "Degenerate polygon - Cannot compute normal"
    //               << std::endl;
    //     Point dummynormal(0,0,0);
    //     return dummynormal;
    //   }

    //   int dim = coords[0].dim();

    //   Point normal(dim);

    //   switch (dim) {
    //   case 2: {
    //     // sufficient to evaluate normal at one corner

    //     Point v1 = coords[2]-coords[1];
    //     Point v2 = coords[0]-coords[1];

    //     normal.set(0.0,0.0);
    //     normal = v1^v2;
    //     break;
    //   }
    //   case 3: {
    //     normal.set(0.0,0.0,0.0);
    //     unsigned int np = coords.size();

    //     if (np == 3) {
    //       Point v1 = coords[1]-coords[0];
    //       Point v2 = coords[2]-coords[0];
    //       normal += v1^v2;
    //     }
    //     else {
    //       // compute sum of area weighted normals of each triangular facet

    //       Point center(dim);

    //       // Compute a center point

    //       for (int i = 0; i < np; i++)
    //         center += coords[i];
    //       center /= np;

    //       for (int i = 0; i < np; i++) {
    //         Point v1 = coords[i]-center;
    //         Point v2 = coords[(i+1)%np]-center;

    //         Point v3 = v1^v2;

    //         normal += 0.5*v3;
    //       }
    //     }
    //     break;
    //   }
    //   default:
    //     std::cout << "Invalid coordinate dimensions" << std::endl;
    //   }

    //   return normal;

    // } // polygon_get_normal


    // Check if point is in polygon by Jordan's crossing algorithm

    bool point_in_polygon(const Point testpnt,
                          const std::vector<Point> coords) {
      int i, ip1, c;

      /* Basic test - will work for strictly interior and exterior points */

      int np = coords.size();

      double x = testpnt.x();
      double y = testpnt.y();

      for (i = 0, c = 0; i < np; i++) {
        ip1 = (i+1)%np;
        if (((coords[i].y() > y && coords[ip1].y() <= y) ||
             (coords[ip1].y() > y && coords[i].y() <= y)) &&
            (x <= (coords[i].x() + (y-coords[i].y())
                   *(coords[ip1].x()-coords[i].x())
                   /(coords[ip1].y()-coords[i].y()))))
          c = !c;
      }

      /* If we don't need consistent classification of points on the
         boundary, we can quit here.  If the point is classified as
         inside, it is definitely inside or on the boundary - no way it
         can be outside and be classified as inside */

      return (c == 1);
    }


  void segment_get_vol_centroid(const std::vector<Point> ccoords,
                                Geom_type my_geom_type,
                                double *volume, Point* centroid) {
    if (my_geom_type == Geom_type::CARTESIAN) {
      *volume = norm(ccoords[1]-ccoords[0]);
      *centroid = 0.5*(ccoords[1]+ccoords[0]);
    } else if (my_geom_type == Geom_type::SPHERICAL) {
      *volume = 4.0*M_PI*(pow(ccoords[1].x(), 3) -
                          pow(ccoords[0].x(), 3)) / 3.0;
      *centroid = 0.5*(ccoords[1]+ccoords[0]);
    }
  }

  void face1d_get_area(const std::vector<Point> fcoords,
                       Geom_type my_geom_type,
                       double *area) {
    if (my_geom_type == Geom_type::CARTESIAN) {
      *area = 1.0;
    } else if (my_geom_type == Geom_type::SPHERICAL) {
      *area = 4.0*M_PI*pow(fcoords[0].x(), 2);
    }

  }

  }  // namespace JaliGeometry



