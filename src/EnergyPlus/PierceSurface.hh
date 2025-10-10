// EnergyPlus, Copyright (c) 1996-2025, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef EnergyPlus_PierceSurface_hh_INCLUDED
#define EnergyPlus_PierceSurface_hh_INCLUDED

// Purpose: Functions for checking if a ray hits a surface
//
// Author: Stuart Mentzer (Stuart_Mentzer@objexx.com)
//
// History:
//  Jun 2015: Last update of legacy version based on DOE-2 DPIERC
//  Jan 2016: Initial release
//
// Notes:
//  This is filling the role of the former PierceSurface function authored by Fred Winkelmann and based on
//   DOE-2.1E subroutine DPIERC and some aspects of this version are analogous
//  To match the former behavior rays with origin exactly on the surface are treated as not hitting
//  These functions are VERY performance critical for daylighting and solar reflection
//   This high-performance implementation was built to complement the octree system for scalability of those systems
//  This has been carefully designed for speed but is probably not be optimal yet
//   For EnergyPlus most surfaces are rectangular so that is the most important for performance
//   Inlining, storing preprocessed values in Surface, 2D projection, & short circuiting are used here for speed
//   Agressive inlining options may be needed to get peak performance
//   Don't make changes here without validating the performance impact

// EnergyPlus Headers
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Platform.hh>

// ObjexxFCL Headers
#include <ObjexxFCL/Vector2.hh>
#include <ObjexxFCL/Vector3.hh>
#include <ObjexxFCL/Vector4.hh>

// C++ Headers
#include <algorithm>
#include <cassert>
#include <limits>

namespace EnergyPlus {

inline bool PierceSurface_Triangular(DataSurfaces::Surface2D const &s2d, // 2D surface
                                     Vector2<Real64> const &h2d          // 2D hit point
)
{
    // Purpose: Check if a 2D hit point is in a triangular 2D surface
    //
    // Author: Stuart Mentzer (Stuart_Mentzer@objexx.com)
    //
    // History:
    //  Jan 2016: Initial release
    //
    // Notes:
    //  Pulled this case out into separate function to facilitate inlining

    using DataSurfaces::Surface2D;
    Surface2D::Vertices const &vs(s2d.vertices); // 2D surface vertices
    Surface2D::Vectors const &es(s2d.edges);     // 2D surface edge vectors
    if (es[0].cross(h2d - vs[0]) < 0.0) {
        return false;
    }
    if (es[1].cross(h2d - vs[1]) < 0.0) {
        return false;
    }
    if (es[2].cross(h2d - vs[2]) < 0.0) {
        return false;
    }
    return true;
} // PierceSurface_Triangular()

inline bool PierceSurface_Convex(DataSurfaces::Surface2D const &s2d, // 2D surface
                                 Vector2<Real64> const &h2d          // 2D hit point
)
{
    // Purpose: Check if a 2D hit point is in a convex 2D surface
    //
    // Author: Stuart Mentzer (Stuart_Mentzer@objexx.com)
    //
    // History:
    //  Jan 2016: Initial release
    //
    // Notes:
    //  Pulled this rare case out into separate function to facilitate inlining
    //  This is O( n ) complexity so it is isn't used for many-vertex surfaces

    using DataSurfaces::Surface2D;
    Surface2D::Vertices const &vs(s2d.vertices); // 2D surface vertices
    Surface2D::Vectors const &es(s2d.edges);     // 2D surface edge vectors
    Surface2D::Vertices::size_type const n(vs.size());
    assert(n >= 3u);
    switch (n) {
    case 8:
        if (es[7].cross(h2d - vs[7]) < 0.0) {
            return false;
        }
        // fallthrough
    case 7:
        if (es[6].cross(h2d - vs[6]) < 0.0) {
            return false;
        }
        // fallthrough
    case 6:
        if (es[5].cross(h2d - vs[5]) < 0.0) {
            return false;
        }
        // fallthrough
    case 5:
        if (es[4].cross(h2d - vs[4]) < 0.0) {
            return false;
        }
        // fallthrough
    case 4:
        if (es[3].cross(h2d - vs[3]) < 0.0) {
            return false;
        }
        // fallthrough
    case 3:
        if (es[2].cross(h2d - vs[2]) < 0.0) {
            return false;
        }
        if (es[1].cross(h2d - vs[1]) < 0.0) {
            return false;
        }
        if (es[0].cross(h2d - vs[0]) < 0.0) {
            return false;
        }
        return true;
    default:
        for (Surface2D::Vertices::size_type i = 0; i < n; ++i) {
            if (es[i].cross(h2d - vs[i]) < 0.0) {
                return false;
            }
        }
        return true;
    }
} // PierceSurface_Convex()

inline bool PierceSurface_Nonconvex(DataSurfaces::Surface2D const &s2d, // 2D surface
                                    Vector2<Real64> const &h2d          // 2D hit point
)
{
    // Purpose: Check if a 2D hit point is in a 2D possibly nonconvex surface
    //
    // Author: Stuart Mentzer (Stuart_Mentzer@objexx.com)
    //
    // History:
    //  Jan 2016: Initial release
    //
    // Notes:
    //  Pulled this rare case out into separate function to facilitate inlining
    //  This works for nonconvex "simple" (no edge crossings) polygons
    //  This is also a fast O( log n ) algorithm for many-vertex convex surfaces

    using DataSurfaces::Surface2D;
    using size_type = Surface2D::Vertices::size_type;
    using Slab = DataSurfaces::Surface2DSlab;
    using Vertex2D = Vector2<Real64>;
    assert(s2d.vertices.size() >= 3u);
    Surface2D::Slabs const &slabs(s2d.slabs);    // 2D surface y slice slabs
    Surface2D::SlabYs const &slabYs(s2d.slabYs); // 2D surface slab y coordinates
    assert(slabYs.size() > 0u);
    Real64 const yHit(h2d.y); // Hit point y coordinate

    // Find slab with y range containing hit point
    auto const iHit(std::lower_bound(slabYs.begin(), slabYs.end(), yHit));
    assert((yHit >= slabYs.front()) && (yHit <= slabYs.back())); // Passed bounding box check so hit point in slabs y range
    assert(iHit != slabYs.end());                                // Hit point can't be above all slabs: passed bounding box check
    size_type const iSlab(std::min(static_cast<size_type>(iHit - 1 - slabYs.begin()), slabs.size())); // Hit slab index
    Slab const &slab(slabs[iSlab]);

    // Check hit point within slab bounding box x range
    Real64 const xHit(h2d.x); // Hit point x coordinate
    if ((xHit < slab.xl) || (xHit > slab.xu)) {
        return false; // Hit point outside slab bounding box
    }

    // Find edge pair surrounding hit point
    Slab::Edges const &slabEdges(slab.edges);
    Slab::EdgesXY const &slabEdgesXY(slab.edgesXY);
    size_type const nEdges(slabEdges.size());
    assert(nEdges >= 2u);
    if (nEdges == 2) { // 2 edges
        Slab::Edge const se0(slabEdges[0]);
        Slab::EdgeXY const eXY0(slabEdgesXY[0]);
        Vertex2D v0(s2d.vertices[se0]);
        Surface2D::Edge e0(s2d.edges[se0]);
        Real64 const x0(v0.x + (yHit - v0.y) * eXY0);
        if (xHit < x0) {
            return false; // Hit point x is left of left edge
        }
        Slab::Edge const se1(slabEdges[1]);
        Slab::EdgeXY const eXY1(slabEdgesXY[1]);
        Vertex2D v1(s2d.vertices[se1]);
        Surface2D::Edge e1(s2d.edges[se1]);
        Real64 const x1(v1.x + (yHit - v1.y) * eXY1);
        if (x1 < xHit) {
            return false; // Hit point is right of right edge
        }
    } else { // 4+ edges: Binary search for edges surrounding hit point
        assert(nEdges >= 4u);
        assert(nEdges % 2 == 0u);
        size_type l(0u), u(nEdges - 1);
        Slab::Edge const il(slabEdges[l]);
        Slab::EdgeXY const eXYl(slabEdgesXY[l]);
        Vertex2D const &vl(s2d.vertices[il]);
        Surface2D::Edge const el(s2d.edges[il]);
        Real64 const xl(vl.x + (yHit - vl.y) * eXYl);
        if (xHit < xl) {
            return false; // Hit point x is left of leftmost edge
        }
        Slab::Edge const iu(slabEdges[u]);
        Slab::EdgeXY const eXYu(slabEdgesXY[u]);
        Vertex2D const &vu(s2d.vertices[iu]);
        Surface2D::Edge const eu(s2d.edges[iu]);
        Real64 const xu(vu.x + (yHit - vu.y) * eXYu);
        if (xu < xHit) {
            return false; // Hit point is right of rightmost edge
        }
        while (u - l > 1u) {
            size_type const m((l + u) / 2);
            Slab::Edge const im(slabEdges[m]);
            Slab::EdgeXY const eXYm(slabEdgesXY[m]);
            Vertex2D const &vm(s2d.vertices[im]);
            Surface2D::Edge const em(s2d.edges[im]);
            Real64 xm(vm.x + (yHit - vm.y) * eXYm);
            if (xHit <= xm) {
                u = m;
            } else {
                l = m;
            }
        }
        assert(u - l == 1u);
        if (u % 2 == 0u) {
            return false; // Outside of nonconvex surface polygon
        }
    }
    return true;
} // PierceSurface_nonconvex()

ALWAYS_INLINE
bool PierceSurface_polygon(DataSurfaces::SurfaceData const &surface, // Surface
                           Vector3<Real64> const &hitPt              // Ray-plane intersection point
)
{
    // Purpose: Check if hit point on surface plane is in surface polygon
    //
    // Author: Stuart Mentzer (Stuart_Mentzer@objexx.com)
    //
    // History:
    //  Jan 2016: Initial release

    using DataSurfaces::nVerticesBig;
    using DataSurfaces::Surface2D;
    using Vertex2D = Vector2<Real64>;
    Surface2D const &s2d(surface.surface2d);
    int const axis(s2d.axis);
    Vertex2D const h2d(axis == 0 ? hitPt.y : hitPt.x, axis == 2 ? hitPt.y : hitPt.z); // Hit point in 2D surface's plane
    if ((h2d.x < s2d.vl.x) || (s2d.vu.x < h2d.x) || (h2d.y < s2d.vl.y) || (s2d.vu.y < h2d.y)) {
        return false; // Misses 2D surface bounding box
    }
    ShapeCat const shapeCat(surface.shapeCat);
    if (shapeCat == ShapeCat::Rectangular) { // Rectangular is most common: Special case algorithm is faster but assumes these are really rectangular
        Vertex2D const v0h(h2d - s2d.vertices[0]);
        Real64 const he1(v0h.dot(s2d.edges[0]));
        if ((he1 < 0.0) || (he1 > s2d.s1)) {
            return false;
        }
        Real64 const he3(-v0h.dot(s2d.edges[3]));
        if ((he3 < 0.0) || (he3 > s2d.s3)) {
            return false;
        }
        return true;
    } else if (shapeCat == ShapeCat::Triangular) { // Cross products all nonnegative <=> Hit point in triangle
        return PierceSurface_Triangular(s2d, h2d);
    } else if ((shapeCat == ShapeCat::Nonconvex) ||
               (s2d.vertices.size() >= nVerticesBig)) { // O( log n ) algorithm for nonconvex and many-vertex convex surfaces
        return PierceSurface_Nonconvex(s2d, h2d);
    } else if (shapeCat == ShapeCat::Convex) { // O( n ) algorithm for convex surface without too many vertices
        return PierceSurface_Convex(s2d, h2d);
    } else {
        return false; // Should we assert here also?
    }
} // PierceSurface_Polygon()

ALWAYS_INLINE
bool PierceSurface(DataSurfaces::SurfaceData const &surface, // Surface
                   Vector3<Real64> const &rayOri,            // Ray origin point
                   Vector3<Real64> const &rayDir,            // Ray direction vector
                   Vector3<Real64> &hitPt                    // Ray-plane intersection point
)
{
    // Purpose: Check if a ray hits a surface and return the point of intersection
    //  with the surface's plane if they intersect.
    //  Convex and concave surfaces with 3 or more vertices are supported.
    //
    // Author: Stuart Mentzer (Stuart_Mentzer@objexx.com)
    //
    // History:
    //  Jan 2016: Initial release

    // Find ray intersection with surface plane
    DataSurfaces::SurfaceData::Plane const &plane(surface.plane);
    Real64 const den((plane.x * rayDir.x) + (plane.y * rayDir.y) + (plane.z * rayDir.z));
    if (den == 0.0) { // Ray is parallel to plane: This not treated as piercing even if ray lies in plane
        return false;
    } else { // Ray's line intersects plane
        Real64 const num(-((plane.x * rayOri.x) + (plane.y * rayOri.y) + (plane.z * rayOri.z) + plane.w));
        if (num * den <=
            0.0) { // Ray points away from surface or ray origin is on surface: This looks odd but is fast way to check for different signs
            return false;
        } else {                                 // Ray points toward surface: Compute hit point
            Real64 const t(num / den);           // Ray parameter at plane intersection: hitPt = rayOri + t * rayDir
            hitPt.x = rayOri.x + (t * rayDir.x); // Compute by coordinate to avoid Vertex temporaries
            hitPt.y = rayOri.y + (t * rayDir.y);
            hitPt.z = rayOri.z + (t * rayDir.z);
        }
    }

    // Check if hit point is in surface polygon
    return PierceSurface_polygon(surface, hitPt);
} // PierceSurface()

ALWAYS_INLINE
bool PierceSurface(EnergyPlusData &state,
                   int const iSurf,               // Surface index
                   Vector3<Real64> const &rayOri, // Ray origin point
                   Vector3<Real64> const &rayDir, // Ray direction vector
                   Vector3<Real64> &hitPt         // Ray-plane intersection point
)
{
    // Purpose: Overload taking surface index instead of surface
    //
    // Author: Stuart Mentzer (Stuart_Mentzer@objexx.com)
    //
    // History:
    //  Jan 2016: Initial release

    return PierceSurface(state.dataSurface->Surface(iSurf), rayOri, rayDir, hitPt);
} // PierceSurface()

ALWAYS_INLINE
bool PierceSurface(DataSurfaces::SurfaceData const &surface, // Surface
                   Vector3<Real64> const &rayOri,            // Ray origin point
                   Vector3<Real64> const &rayDir,            // Ray direction unit vector
                   Real64 const dMax,                        // Max distance from rayOri to hit point
                   Vector3<Real64> &hitPt                    // Ray-plane intersection point
)
{
    // Purpose: Check if a ray hits a surface and return the point of intersection
    //  with the surface's plane if they intersect.
    //  Convex and concave surfaces with 3 or more vertices are supported.
    //  This overload limits the ray-surface distance for a hit.
    //
    // Author: Stuart Mentzer (Stuart_Mentzer@objexx.com)
    //
    // History:
    //  Jan 2016: Initial release

    // Input checks
    assert(std::abs(rayDir.mag_squared() - 1.0) <
           6 * std::numeric_limits<Real64>::epsilon()); // Check unit vector (6x is rough estimate. Increase slightly as needed.)
    assert(dMax >= 0.0);                                // Distance must be nonnegative

    // Find ray intersection with surface plane
    DataSurfaces::SurfaceData::Plane const &plane(surface.plane);
    Real64 const den((plane.x * rayDir.x) + (plane.y * rayDir.y) + (plane.z * rayDir.z));
    if (den == 0.0) { // Ray is parallel to plane: This not treated as piercing even if ray lies in plane
        return false;
    } else { // Ray's line intersects plane
        Real64 const num(-((plane.x * rayOri.x) + (plane.y * rayOri.y) + (plane.z * rayOri.z) + plane.w));
        if (num * den <=
            0.0) { // Ray points away from surface or ray origin is on surface: This looks odd but is fast way to check for different signs
            return false;
        } else {                       // Ray points toward surface: Compute hit point
            Real64 const t(num / den); // Ray parameter at plane intersection: hitPt = rayOri + t * rayDir
            if (t > dMax) {
                return false; // Hit point exceeds distance from rayOri limit
            }
            hitPt.x = rayOri.x + (t * rayDir.x); // Compute by coordinate to avoid Vertex temporaries
            hitPt.y = rayOri.y + (t * rayDir.y);
            hitPt.z = rayOri.z + (t * rayDir.z);
        }
    }

    // Check if hit point is in surface polygon
    return PierceSurface_polygon(surface, hitPt);
} // PierceSurface()

ALWAYS_INLINE
bool PierceSurface(EnergyPlusData &state,
                   int const iSurf,               // Surface index
                   Vector3<Real64> const &rayOri, // Ray origin point
                   Vector3<Real64> const &rayDir, // Ray direction unit vector
                   Real64 const dMax,             // Max distance from rayOri to hit point
                   Vector3<Real64> &hitPt         // Ray-plane intersection point
)
{
    // Purpose: Overload taking surface index instead of surface
    //
    // Author: Stuart Mentzer (Stuart_Mentzer@objexx.com)
    //
    // History:
    //  Jan 2016: Initial release

    return PierceSurface(state.dataSurface->Surface(iSurf), rayOri, rayDir, dMax, hitPt);
}

} // namespace EnergyPlus

#endif
