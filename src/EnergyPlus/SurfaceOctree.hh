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

#ifndef EnergyPlus_SurfaceOctree_hh_INCLUDED
#define EnergyPlus_SurfaceOctree_hh_INCLUDED

// EnergyPlus Headers
#include <EnergyPlus/EPVector.hh>
#include <EnergyPlus/EnergyPlus.hh>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1.fwd.hh>
#include <ObjexxFCL/Vector3.hh>

// C++ Headers
#include <cassert>
#include <cstdint>
#include <vector>

namespace EnergyPlus {

// Forward
struct EnergyPlusData;

namespace DataSurfaces {
    struct SurfaceData;
}

// Package: Surface Octree System
//
// Purpose: Spatial sort of surfaces for fast, scalable identification of active surfaces for algorithms
//  making spatial queries such as solar shading, solar reflection, and daylighting obstruction
//
// Author: Stuart Mentzer (Stuart_Mentzer@objexx.com)
//
// History:
//  Sep 2015: Experimental code
//  Jan 2016: Initial release
//
// Notes: See the .cc file

class SurfaceOctreeCube
{

public: // Types
    using Real = Real64;
    using Surface = DataSurfaces::SurfaceData;
    using Vertex = ObjexxFCL::Vector3<Real>;
    using Surfaces = std::vector<Surface *>;
    using size_type = Surfaces::size_type;

public: // Creation
    // Default Constructor
    SurfaceOctreeCube() : d_(0u), n_(0u), l_(Vertex(0.0)), u_(Vertex(0.0)), c_(Vertex(0.0)), w_(0.0), r_(0.0)
    {
        for (std::uint8_t i = 0; i < 8; ++i) {
            cubes_[i] = nullptr; // VC++ 2013 compatible initialization
        }
    }

    // Surfaces Outer Cube Constructor
    SurfaceOctreeCube(EPVector<Surface> &surfaces) : d_(0u), n_(0u), l_(Vertex(0.0)), u_(Vertex(0.0)), c_(Vertex(0.0)), w_(0.0), r_(0.0)
    {
        for (std::uint8_t i = 0; i < 8; ++i) {
            cubes_[i] = nullptr; // VC++ 2013 compatible initialization
        }
        init(surfaces);
    }

    // Box Constructor
    SurfaceOctreeCube(std::uint8_t const d, Vertex const &l, Vertex const &u, Real const w)
        : d_(d), n_(0u), l_(l), u_(u), c_(cen(l, u)), w_(w), r_(0.75 * (w * w))
    {
        for (std::uint8_t i = 0; i < 8; ++i) {
            cubes_[i] = nullptr; // VC++ 2013 compatible initialization
        }
        assert(valid());
    }

    // Destructor
    ~SurfaceOctreeCube()
    {
        for (std::uint8_t i = 0; i < n_; ++i) {
            delete cubes_[i];
        }
    }

public: // Properties
    // Depth
    std::uint8_t d() const
    {
        return d_;
    }

    // Depth
    std::uint8_t depth() const
    {
        return d_;
    }

    // Number of Children
    std::uint8_t nChildren() const
    {
        return n_;
    }

    // Number of Sub-Cubes
    std::uint8_t nSubCube() const
    {
        return n_;
    }

    // Lower Corner
    Vertex const &l() const
    {
        return l_;
    }

    // Upper Corner
    Vertex const &u() const
    {
        return u_;
    }

    // Center Point
    Vertex const &c() const
    {
        return c_;
    }

    // Center Point
    Vertex const &center() const
    {
        return c_;
    }

    // Width
    Real w() const
    {
        return w_;
    }

    // Width
    Real width() const
    {
        return w_;
    }

    // Surfaces
    Surfaces const &surfaces() const
    {
        return surfaces_;
    }

    // Surfaces
    Surfaces::size_type surfaces_size() const
    {
        return surfaces_.size();
    }

    // Surfaces Begin Iterator
    Surfaces::const_iterator surfaces_begin() const
    {
        return surfaces_.begin();
    }

    // Surfaces Begin Iterator
    Surfaces::iterator surfaces_begin()
    {
        return surfaces_.begin();
    }

    // Surfaces End Iterator
    Surfaces::const_iterator surfaces_end() const
    {
        return surfaces_.end();
    }

    // Surfaces End Iterator
    Surfaces::iterator surfaces_end()
    {
        return surfaces_.end();
    }

public: // Methods
    // Vertex in Cube?
    bool contains(Vertex const &v) const
    {
        return (l_.x <= v.x) && (v.x <= u_.x) && (l_.y <= v.y) && (v.y <= u_.y) && (l_.z <= v.z) && (v.z <= u_.z);
    }

    // Surface in Cube?
    bool contains(Surface const &surface) const;

    // Line Segment Intersects Enclosing Sphere?
    bool segmentIntersectsSphere(Vertex const &a, Vertex const &b) const
    {
        Vertex const ab(b - a);
        Real const ab_mag_squared(ab.mag_squared());
        if (ab_mag_squared == 0.0) { // Segment is a point
            return ObjexxFCL::distance_squared(a, c_) <= r_;
        } else { // Might pay to check if a or b in sphere first in some applications
            Vertex const ac(c_ - a);
            Real const projection_fac(((ac.x * ab.x) + (ac.y * ab.y) + (ac.z * ab.z)) / ab_mag_squared);
            if ((0.0 <= projection_fac) && (projection_fac <= 1.0)) { // Projected (closest) point is on ab segment
                return ObjexxFCL::distance_squared(ac, projection_fac * ab) <= r_;
            } else { // Projection (closest) point is outside of ab segment: Intersects iff a or b are in sphere
                return (ObjexxFCL::distance_squared(a, c_) <= r_) || (ObjexxFCL::distance_squared(b, c_) <= r_);
            }
        }
    }

    // Ray Intersects Enclosing Sphere?
    bool rayIntersectsSphere(Vertex const &a, Vertex const &dir) const
    {
        assert(std::abs(dir.mag_squared() - 1.0) < 4 * std::numeric_limits<Real>::epsilon()); // Check unit vector
        // Might pay to check if a in sphere first in some applications
        Vertex const ac(c_ - a);
        Real const projection_fac((ac.x * dir.x) + (ac.y * dir.y) + (ac.z * dir.z));
        if (0.0 <= projection_fac) { // Projected (closest) point is on ray
            return ObjexxFCL::distance_squared(ac, projection_fac * dir) <= r_;
        } else { // Projection (closest) point is outside of ray: Intersects iff a is in sphere
            return ObjexxFCL::distance_squared(a, c_) <= r_;
        }
    }

    // Line Intersects Enclosing Sphere?
    bool lineIntersectsSphere(Vertex const &a, Vertex const &dir) const
    {
        assert(std::abs(dir.mag_squared() - 1.0) < 4 * std::numeric_limits<Real>::epsilon()); // Check unit vector
        Vertex const ac(c_ - a);
        return ac.mag_squared() - ObjexxFCL::square((ac.x * dir.x) + (ac.y * dir.y) + (ac.z * dir.z)) <= r_;
    }

    // Line Segment Intersects Cube?
    bool segmentIntersectsCube(Vertex const &a, Vertex const &b) const
    {
        // Check if either segment endpoint is in cube
        if (contains(a) || contains(b)) {
            return true;
        }

        // Use separating axis theorem (faster variants exist)
        Vertex const m(mid(a, b) - c_);                                 // Mid-point relative to cube center
        Vertex const mb(b - c_ - m);                                    // ab mid-point to b half segment vector
        Vertex const e(std::abs(mb.x), std::abs(mb.y), std::abs(mb.z)); // Extent of half ab segment
        Real const h(0.5 * w_);                                         // Half-width
        // Check if x,y,z axes are separating
        if (std::abs(m.x) > h + e.x) {
            return false;
        }
        if (std::abs(m.y) > h + e.y) {
            return false;
        }
        if (std::abs(m.z) > h + e.z) {
            return false;
        }
        // Check if cross product axes are separating
        if (std::abs(m.y * mb.z - m.z * mb.y) > h * (e.z + e.y)) {
            return false;
        }
        if (std::abs(m.x * mb.z - m.z * mb.x) > h * (e.z + e.x)) {
            return false;
        }
        if (std::abs(m.x * mb.y - m.y * mb.x) > h * (e.y + e.x)) {
            return false;
        }
        return true;
    }

    // Ray Intersects Cube?
    bool rayIntersectsCube(Vertex const &a, Vertex const &dir, Vertex const &dir_inv) const
    {
        // Note: dir_inv coordinates corresponding to a zero dir coordinate are not used and can be set to zero

        assert(std::abs(dir.mag_squared() - 1.0) < 4 * std::numeric_limits<Real>::epsilon()); // Check unit vector
        assert((dir.x == 0.0) || (std::abs(dir_inv.x - (1.0 / dir.x)) < 2 * std::numeric_limits<Real>::epsilon() * std::abs(dir_inv.x)));
        assert((dir.y == 0.0) || (std::abs(dir_inv.y - (1.0 / dir.y)) < 2 * std::numeric_limits<Real>::epsilon() * std::abs(dir_inv.y)));
        assert((dir.z == 0.0) || (std::abs(dir_inv.z - (1.0 / dir.z)) < 2 * std::numeric_limits<Real>::epsilon() * std::abs(dir_inv.z)));

        // Check if ray origin is in cube
        if (contains(a)) {
            return true;
        }

        // Refined Smits' method: Largest distance to 3 visible cube face planes along ray is the candidate entry point
        Real tx, ty, tz; // Ray position parameter for intersections with box face planes
        if (dir.x > 0.0) {
            tx = (l_.x - a.x) * dir_inv.x;
        } else if (dir.x < 0.0) {
            tx = (u_.x - a.x) * dir_inv.x;
        } else { // dir.x == 0
            tx = std::numeric_limits<Real>::lowest();
        }
        if (dir.y > 0.0) {
            ty = (l_.y - a.y) * dir_inv.y;
        } else if (dir.y < 0.0) {
            ty = (u_.y - a.y) * dir_inv.y;
        } else { // dir.y == 0
            ty = std::numeric_limits<Real>::lowest();
        }
        if (dir.z > 0.0) {
            tz = (l_.z - a.z) * dir_inv.z;
        } else if (dir.z < 0.0) {
            tz = (u_.z - a.z) * dir_inv.z;
        } else { // dir.z == 0
            tz = std::numeric_limits<Real>::lowest();
        }

        Real const tmax(ObjexxFCL::max(tx, ty, tz));
        if (tmax >= 0.0) { // Entry point is on ray: See if it is within cube extent
            if (tx == tmax) {
                Real const y(a.y + (tmax * dir.y));
                if ((y < l_.y) || (y > u_.y)) {
                    return false;
                }
                Real const z(a.z + (tmax * dir.z));
                if ((z < l_.z) || (z > u_.z)) {
                    return false;
                }
            } else if (ty == tmax) {
                Real const x(a.x + (tmax * dir.x));
                if ((x < l_.x) || (x > u_.x)) {
                    return false;
                }
                Real const z(a.z + (tmax * dir.z));
                if ((z < l_.z) || (z > u_.z)) {
                    return false;
                }
            } else { // tz == tmax
                Real const x(a.x + (tmax * dir.x));
                if ((x < l_.x) || (x > u_.x)) {
                    return false;
                }
                Real const y(a.y + (tmax * dir.y));
                if ((y < l_.y) || (y > u_.y)) {
                    return false;
                }
            }
            return true;
        } else { // Entry point is on backwards projection of ray
            return false;
        }
    }

    // Ray Intersects Cube?
    bool rayIntersectsCube(Vertex const &a, Vertex const &dir) const
    {
        return rayIntersectsCube(a, dir, safe_inverse(dir)); // Inefficient if called in loop with same dir
    }

    // Line Intersects Cube?
    bool lineIntersectsCube(Vertex const &a, Vertex const &dir, Vertex const &dir_inv) const
    {
        // Note: dir_inv coordinates corresponding to a zero dir coordinate are not used and can be set to zero

        assert(std::abs(dir.mag_squared() - 1.0) < 4 * std::numeric_limits<Real>::epsilon()); // Check unit vector
        assert((dir.x == 0.0) || (std::abs(dir_inv.x - (1.0 / dir.x)) < 2 * std::numeric_limits<Real>::epsilon() * std::abs(dir_inv.x)));
        assert((dir.y == 0.0) || (std::abs(dir_inv.y - (1.0 / dir.y)) < 2 * std::numeric_limits<Real>::epsilon() * std::abs(dir_inv.y)));
        assert((dir.z == 0.0) || (std::abs(dir_inv.z - (1.0 / dir.z)) < 2 * std::numeric_limits<Real>::epsilon() * std::abs(dir_inv.z)));

        // Smits' method: Largest distance to 3 visible cube faces along ray is the candidate entry point
        Real txl, txu, tyl, tyu, tzl, tzu; // Ray position parameter for intersections with box face planes
        if (dir.x > 0.0) {
            txl = (l_.x - a.x) * dir_inv.x;
            txu = (u_.x - a.x) * dir_inv.x;
        } else if (dir.x < 0.0) {
            txl = (u_.x - a.x) * dir_inv.x;
            txu = (l_.x - a.x) * dir_inv.x;
        } else { // dir.x == 0
            if ((l_.x - a.x <= 0.0) && (u_.x - a.x >= 0.0)) {
                txl = std::numeric_limits<Real>::lowest();
                txu = std::numeric_limits<Real>::max();
            } else {
                return false;
            }
        }
        if (dir.y > 0.0) {
            tyl = (l_.y - a.y) * dir_inv.y;
            tyu = (u_.y - a.y) * dir_inv.y;
        } else if (dir.y < 0.0) {
            tyl = (u_.y - a.y) * dir_inv.y;
            tyu = (l_.y - a.y) * dir_inv.y;
        } else { // dir.y == 0
            if ((l_.y - a.y <= 0.0) && (u_.y - a.y >= 0.0)) {
                tyl = std::numeric_limits<Real>::lowest();
                tyu = std::numeric_limits<Real>::max();
            } else {
                return false;
            }
        }
        if ((txl > tyu) || (tyl > txu)) {
            return false;
        }
        if (dir.z > 0.0) {
            tzl = (l_.z - a.z) * dir_inv.z;
            tzu = (u_.z - a.z) * dir_inv.z;
        } else if (dir.z < 0.0) {
            tzl = (u_.z - a.z) * dir_inv.z;
            tzu = (l_.z - a.z) * dir_inv.z;
        } else { // dir.z == 0
            if ((l_.z - a.z <= 0.0) && (u_.z - a.z >= 0.0)) {
                tzl = std::numeric_limits<Real>::lowest();
                tzu = std::numeric_limits<Real>::max();
            } else {
                return false;
            }
        }
        return ((txl <= tzu) && (tzl <= txu) && (tyl <= tzu) && (tzl <= tyu));
    }

    // Line Intersects Cube?
    bool lineIntersectsCube(Vertex const &a, Vertex const &dir) const
    {
        return lineIntersectsCube(a, dir, safe_inverse(dir)); // Inefficient if called in loop with same dir
    }

    // Surfaces Outer Cube Initilization
    void init(EPVector<Surface> &surfaces);

    // Surfaces that Line Segment Intersects Cube's Enclosing Sphere
    void surfacesSegmentIntersectsSphere(Vertex const &a, Vertex const &b, Surfaces &surfaces) const
    {
        if (segmentIntersectsSphere(a, b)) {
            if (!surfaces_.empty()) {
                surfaces.insert(surfaces.end(), surfaces_.begin(), surfaces_.end()); // Add this cube's surfaces
            }
            for (std::uint8_t i = 0; i < n_; ++i) { // Recurse
                cubes_[i]->surfacesSegmentIntersectsSphere(a, b, surfaces);
            }
        }
    }

    // Surfaces that Ray Intersects Cube's Enclosing Sphere
    void surfacesRayIntersectsSphere(Vertex const &a, Vertex const &dir, Surfaces &surfaces) const
    {
        if (rayIntersectsSphere(a, dir)) {
            if (!surfaces_.empty()) {
                surfaces.insert(surfaces.end(), surfaces_.begin(), surfaces_.end()); // Add this cube's surfaces
            }
            for (std::uint8_t i = 0; i < n_; ++i) { // Recurse
                cubes_[i]->surfacesRayIntersectsSphere(a, dir, surfaces);
            }
        }
    }

    // Surfaces that Line Intersects Cube's Enclosing Sphere
    void surfacesLineIntersectsSphere(Vertex const &a, Vertex const &dir, Surfaces &surfaces) const
    {
        if (lineIntersectsSphere(a, dir)) {
            if (!surfaces_.empty()) {
                surfaces.insert(surfaces.end(), surfaces_.begin(), surfaces_.end()); // Add this cube's surfaces
            }
            for (std::uint8_t i = 0; i < n_; ++i) { // Recurse
                cubes_[i]->surfacesLineIntersectsSphere(a, dir, surfaces);
            }
        }
    }

    // Surfaces that Line Segment Intersects Cube
    void surfacesSegmentIntersectsCube(Vertex const &a, Vertex const &b, Surfaces &surfaces) const
    {
        if (segmentIntersectsCube(a, b)) {
            if (!surfaces_.empty()) {
                surfaces.insert(surfaces.end(), surfaces_.begin(), surfaces_.end()); // Add this cube's surfaces
            }
            for (std::uint8_t i = 0; i < n_; ++i) { // Recurse
                cubes_[i]->surfacesSegmentIntersectsCube(a, b, surfaces);
            }
        }
    }

    // Surfaces that Ray Intersects Cube
    void surfacesRayIntersectsCube(Vertex const &a, Vertex const &dir, Vertex const &dir_inv, Surfaces &surfaces) const
    {
        if (rayIntersectsCube(a, dir, dir_inv)) {
            if (!surfaces_.empty()) {
                surfaces.insert(surfaces.end(), surfaces_.begin(), surfaces_.end()); // Add this cube's surfaces
            }
            for (std::uint8_t i = 0; i < n_; ++i) { // Recurse
                cubes_[i]->surfacesRayIntersectsCube(a, dir, dir_inv, surfaces);
            }
        }
    }

    // Surfaces that Ray Intersects Cube
    void surfacesRayIntersectsCube(Vertex const &a, Vertex const &dir, Surfaces &surfaces) const
    {
        surfacesRayIntersectsCube(a, dir, safe_inverse(dir), surfaces); // Inefficient if called in loop with same dir
    }

    // Surfaces that Line Intersects Cube
    void surfacesLineIntersectsCube(Vertex const &a, Vertex const &dir, Vertex const &dir_inv, Surfaces &surfaces) const
    {
        if (lineIntersectsCube(a, dir, dir_inv)) {
            if (!surfaces_.empty()) {
                surfaces.insert(surfaces.end(), surfaces_.begin(), surfaces_.end()); // Add this cube's surfaces
            }
            for (std::uint8_t i = 0; i < n_; ++i) { // Recurse
                cubes_[i]->surfacesLineIntersectsCube(a, dir, dir_inv, surfaces);
            }
        }
    }

    // Surfaces that Line Intersects Cube
    void surfacesLineIntersectsCube(Vertex const &a, Vertex const &dir, Surfaces &surfaces) const
    {
        surfacesLineIntersectsCube(a, dir, safe_inverse(dir), surfaces); // Inefficient if called in loop with same dir
    }

    // Seek a Surface in Cube that Line Segment Intersects and Satisfies Predicate
    template <typename Predicate> bool hasSurfaceSegmentIntersectsCube(Vertex const &a, Vertex const &b, Predicate const &predicate) const
    {
        if (segmentIntersectsCube(a, b)) {
            for (auto const *surface_p : surfaces_) { // Try this cube's surfaces
                if (predicate(*surface_p)) {
                    return true;
                }
            }
            for (std::uint8_t i = 0; i < n_; ++i) { // Recurse
                if (cubes_[i]->hasSurfaceSegmentIntersectsCube(a, b, predicate)) {
                    return true;
                }
            }
        }
        return false;
    }

    // Seek a Surface in Cube that Ray Intersects and Satisfies Predicate
    template <typename Predicate>
    bool hasSurfaceRayIntersectsCube(Vertex const &a, Vertex const &dir, Vertex const &dir_inv, Predicate const &predicate) const
    {
        if (rayIntersectsCube(a, dir, dir_inv)) {
            for (auto const *surface_p : surfaces_) { // Try this cube's surfaces
                if (predicate(*surface_p)) {
                    return true;
                }
            }
            for (std::uint8_t i = 0; i < n_; ++i) { // Recurse
                if (cubes_[i]->hasSurfaceRayIntersectsCube(a, dir, dir_inv, predicate)) {
                    return true;
                }
            }
        }
        return false;
    }

    // Seek a Surface in Cube that Ray Intersects and Satisfies Predicate
    template <typename Predicate> bool hasSurfaceRayIntersectsCube(Vertex const &a, Vertex const &dir, Predicate const &predicate) const
    {
        return hasSurfaceRayIntersectsCube(a, dir, safe_inverse(dir), predicate); // Inefficient if called in loop with same dir
    }

    // Process Surfaces in Cube that Ray Intersects with Function
    template <typename Function>
    void processSurfaceRayIntersectsCube(Vertex const &a, Vertex const &dir, Vertex const &dir_inv, Function const &function) const
    {
        if (rayIntersectsCube(a, dir, dir_inv)) {
            for (auto const *surface_p : surfaces_) { // Process this cube's surfaces
                function(*surface_p);
            }
            for (std::uint8_t i = 0; i < n_; ++i) { // Recurse
                cubes_[i]->processSurfaceRayIntersectsCube(a, dir, dir_inv, function);
            }
        }
    }

    // Process Surfaces in Cube that Ray Intersects with Function
    template <typename Function> void processSurfaceRayIntersectsCube(Vertex const &a, Vertex const &dir, Function const &function) const
    {
        processSurfaceRayIntersectsCube(a, dir, safe_inverse(dir), function); // Inefficient if called in loop with same dir
    }

    // Process Surfaces in Cube that Ray Intersects Stopping if Predicate Satisfied
    template <typename Predicate>
    bool processSomeSurfaceRayIntersectsCube(
        EnergyPlusData &state, Vertex const &a, Vertex const &dir, Vertex const &dir_inv, Predicate const &predicate) const
    {
        if (rayIntersectsCube(a, dir, dir_inv)) {
            for (auto const *surface_p : surfaces_) { // Process this cube's surfaces
                if (predicate(*surface_p)) {
                    return true; // Don't need to process more surfaces
                }
            }
            for (std::uint8_t i = 0; i < n_; ++i) { // Recurse
                if (cubes_[i]->processSomeSurfaceRayIntersectsCube(state, a, dir, dir_inv, predicate)) {
                    return true; // Don't need to process more surfaces
                }
            }
        }
        return false;
    }

    // Process Surfaces in Cube that Ray Intersects Stopping if Predicate Satisfied
    template <typename Predicate>
    bool processSomeSurfaceRayIntersectsCube(EnergyPlusData &state, Vertex const &a, Vertex const &dir, Predicate const &predicate) const
    {
        return processSomeSurfaceRayIntersectsCube(state, a, dir, safe_inverse(dir), predicate); // Inefficient if called in loop with same dir
    }

public: // Static Methods
    // Octree-Safe Vector Inverse
    static Vertex safe_inverse(Vertex const &v)
    {
        return Vertex((v.x != 0.0 ? 1.0 / v.x : 0.0), (v.y != 0.0 ? 1.0 / v.y : 0.0), (v.z != 0.0 ? 1.0 / v.z : 0.0));
    }

private: // Methods
    // Valid?
    bool valid() const;

    // Add a Surface
    void add(Surface &surface)
    {
        surfaces_.push_back(&surface);
    }

    // Branch to Sub-Tree
    void branch();

    // Surface Branch Processing
    void surfaceBranch(Surface &surface);

private: // Static Methods
    // Vertex in Cube Defined by Lower and Upper Corners?
    static bool contains(Vertex const &l, Vertex const &u, Vertex const &v)
    {
        return (l.x <= v.x) && (v.x <= u.x) && (l.y <= v.y) && (v.y <= u.y) && (l.z <= v.z) && (v.z <= u.z);
    }

    // Surface in Cube Defined by Lower and Upper Corners?
    static bool contains(Vertex const &l, Vertex const &u, Surface const &surface);

private:                                 // Static Data
    static std::uint8_t const maxDepth_; // Max tree depth
    static size_type const maxSurfaces_; // Max surfaces in a cube before subdividing

private:                          // Data
    std::uint8_t d_;              // Depth in tree
    std::uint8_t n_;              // Number of active sub-cubes ([0,8])
    Vertex l_;                    // Lower corner
    Vertex u_;                    // Upper corner
    Vertex c_;                    // Center point
    Real w_;                      // Side width
    Real r_;                      // Enclosing sphere radius
    SurfaceOctreeCube *cubes_[8]; // Sub-cubes
    Surfaces surfaces_;           // Surfaces in this cube

}; // SurfaceOctreeCube

} // namespace EnergyPlus

#endif
