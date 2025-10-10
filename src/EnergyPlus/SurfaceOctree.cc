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

// EnergyPlus Headers
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/SurfaceOctree.hh>

// C++ Headers
#include <algorithm>
#include <cmath>
#include <limits>

namespace EnergyPlus {

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
// Notes:
//  Initial octree is for use in daylighting but should be adaptable to other use cases:
//   Surfaces without vertices are omitted
//   Transparent surfaces are omitted (can't obstruct light)
//  The octree holds live references to surfaces so it must be updated if surfaces change after its construction (this doesn't occur in EnergyPlus
//  currently) Copy and move ctors/assignment omitted for now since not needed The use of multiple octrees for faster lookups of surface type subsets
//  may be worthwhile for performance in some uses Octree variations and parameter tuning can give better performance for specific applications This
//  design uses "tight" cubes (no overlap) and surfaces filtering down to deepest cube they fit in completely Alternative designs that might offer
//  somewhat better or worse performance:
//   Loose cubes oversied by x2 or some other factor to allow surfaces to filter down further: This requires more cubes to be processed for a given
//   operation Filter all surfaces down to leaf cubes placing a surface in any cube it intersects: More specificity but redundant surfaces in each
//   operation so must collect them in a set

// Surface in Cube?
bool SurfaceOctreeCube::contains(Surface const &surface) const
{
    for (Vertex const &v : surface.Vertex) { // All surface vertices must be in cube
        if (!contains(v)) {
            return false;
        }
    }
    return true;
}

// Surfaces Outer Cube Initilization
void SurfaceOctreeCube::init(EPVector<Surface> &surfaces)
{
    assert(d_ == 0u);
    assert(n_ == 0u);
    surfaces_.clear();
    surfaces_.reserve(surfaces.size());
    for (Surface &surface : surfaces) {
        if ((surface.Vertex.size() >= 3) && // Skip no-vertex "surfaces"
            (!surface.IsTransparent)        // Skip transparent surfaces
        ) {
            surfaces_.push_back(&surface);
        }
    }

    // No surfaces handler
    if (surfaces_.empty()) {
        l_ = u_ = c_ = Vertex(0.0);
        w_ = r_ = 0.0;
        return;
    }

    // Bounding box corners and center
    Surface const &surface_0(*surfaces_[0]);
    assert(!surface_0.Vertex.empty());
    l_ = u_ = surface_0.Vertex[0];               // Initialize corners to first vertex of first surface
    for (Surface const *surface_p : surfaces_) { // Surfaces
        auto const &vertices(surface_p->Vertex);
        for (auto const &vertex : vertices) { // Expand cube to hold surface vertices
            l_.min(vertex);
            u_.max(vertex);
        }
    }
    c_ = cen(l_, u_); // Center vertex

    // Expand bounding box to cube with uniform side width
    Vertex const diagonal(u_ - l_); // Diagonal
    w_ = ObjexxFCL::max(diagonal.x, diagonal.y, diagonal.z);
    r_ = 0.75 * (w_ * w_);
    Real const h(0.5 * w_); // Half-width
    l_ = c_ - h;
    u_ = c_ + h;

    assert(valid());

    // Branch sub-tree
    branch();
}

// Valid?
bool SurfaceOctreeCube::valid() const
{
    if (((l_.x <= c_.x) && (l_.y <= c_.y) && (l_.z <= c_.z)) && ((c_.x <= u_.x) && (c_.y <= u_.y) && (c_.z <= u_.z))) {
        Real const tol2(
            std::max(std::max(ObjexxFCL::magnitude_squared(l_), ObjexxFCL::magnitude_squared(u_)) * (4 * std::numeric_limits<Real>::epsilon()),
                     2 * std::numeric_limits<Real>::min()));
        if (ObjexxFCL::distance_squared(c_, cen(l_, u_)) <= tol2) {
            Real const tol(std::max(std::sqrt(std::max(ObjexxFCL::magnitude_squared(l_), ObjexxFCL::magnitude_squared(u_))) *
                                        (4 * std::numeric_limits<Real>::epsilon()),
                                    2 * std::numeric_limits<Real>::min()));
            Vertex const d(u_ - l_);                                                                            // Diagonal
            return (std::abs(d.x - w_) <= tol) && (std::abs(d.x - d.y) <= tol) && (std::abs(d.x - d.z) <= tol); // Uniform side widths?
        }
    }
    return false;
}

// Branch to Sub-Tree
void SurfaceOctreeCube::branch()
{
    if ((surfaces_.size() > maxSurfaces_) && (d_ < maxDepth_)) {
        // Assign Surfaces to cubes containing them
        Surfaces surfaces_all;
        surfaces_all.swap(surfaces_);
        for (auto *surface_p : surfaces_all) { // Surfaces
            surfaceBranch(*surface_p);
        }

        // Compact occupied cube array
        n_ = 0u;
        for (std::uint8_t i = 0; i < 8; ++i) {
            SurfaceOctreeCube *&cube = cubes_[i];
            if (cube != nullptr) {
                if (n_ < i) {
                    cubes_[n_] = cube;
                    cube = nullptr;
                }
                ++n_;
            }
        }

        // Branch sub-tree recursively
        for (std::uint8_t i = 0; i < n_; ++i) {
            cubes_[i]->branch();
        }
    }
}

// Surface Branch Processing
void SurfaceOctreeCube::surfaceBranch(Surface &surface)
{
    Real const h(0.5 * w_);                              // Half-width
    Vertex sl(surface.Vertex[0]), su(surface.Vertex[0]); // Surface bounding box corners
    auto const &vertices(surface.Vertex);                // Surface vertices
    for (auto const &vertex : vertices) {                // Expand bounding box to hold surface vertices
        sl.min(vertex);
        su.max(vertex);
    }
    Vertex const ctr(cen(sl, su));
    std::uint8_t const i(ctr.x <= c_.x ? 0 : 1);
    std::uint8_t const j(ctr.y <= c_.y ? 0 : 1);
    std::uint8_t const k(ctr.z <= c_.z ? 0 : 1);
    SurfaceOctreeCube *&cube = cubes_[(i << 2) + (j << 1) + k];
    if (cube != nullptr) { // Candidate cube exists
        if (((cube->l_.x <= sl.x) && (cube->l_.y <= sl.y) && (cube->l_.z <= sl.z)) &&
            ((su.x <= cube->u_.x) && (su.y <= cube->u_.y) && (su.z <= cube->u_.z))) { // Surface is contained in sub-cube
            cube->add(surface);
        } else { // Surface stays in this cube
            surfaces_.push_back(&surface);
        }
    } else { // Create cube if surface contained
        Real const x(i * h);
        Real const y(j * h);
        Real const z(k * h);
        Vertex const l(l_.x + x, l_.y + y, l_.z + z);
        Vertex const u(c_.x + x, c_.y + y, c_.z + z);
        if (((l.x <= sl.x) && (l.y <= sl.y) && (l.z <= sl.z)) &&
            ((su.x <= u.x) && (su.y <= u.y) && (su.z <= u.z))) { // Surface is contained in sub-cube
            cube = new SurfaceOctreeCube(d_ + 1, l, u, h);
            cube->add(surface);
        } else { // Surface stays in this cube
            surfaces_.push_back(&surface);
        }
    }
}

// Surface in Cube?
bool SurfaceOctreeCube::contains(Vertex const &l, Vertex const &u, Surface const &surface)
{
    for (Vertex const &v : surface.Vertex) { // All surface vertices must be in cube
        if (!contains(l, u, v)) {
            return false;
        }
    }
    return true;
}

// Static Data Member Definitions
std::uint8_t const SurfaceOctreeCube::maxDepth_ = 255u;                   // Max tree depth
SurfaceOctreeCube::size_type const SurfaceOctreeCube::maxSurfaces_ = 10u; // Max surfaces in a cube before subdividing

} // namespace EnergyPlus
