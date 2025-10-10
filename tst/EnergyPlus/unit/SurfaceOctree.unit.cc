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

// EnergyPlus::SurfaceOctree Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/SurfaceOctree.hh>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// C++ Headers
#include <algorithm>

using namespace EnergyPlus;
using namespace EnergyPlus::DataSurfaces;
using Vertex = SurfaceOctreeCube::Vertex;
using Surfaces = SurfaceOctreeCube::Surfaces;

TEST(SurfaceOctreeTest, Basic)
{

    // Surfaces: Simple Unit Cube
    int constexpr TotSurfaces = 6;
    SurfaceData surface;
    surface.Area = 1.0;
    surface.Sides = 4;
    surface.Vertex.dimension(4);
    EPVector<DataSurfaces::SurfaceData> Surface;
    Surface.dimension(TotSurfaces, surface);
    Surface(1).Vertex = {Vertex(0, 0, 0), Vertex(1, 0, 0), Vertex(1, 0, 1), Vertex(0, 0, 1)};
    Surface(2).Vertex = {Vertex(0, 1, 0), Vertex(1, 1, 0), Vertex(1, 1, 1), Vertex(0, 1, 1)};
    Surface(3).Vertex = {Vertex(0, 0, 0), Vertex(0, 1, 0), Vertex(0, 1, 1), Vertex(0, 0, 1)};
    Surface(4).Vertex = {Vertex(1, 0, 0), Vertex(1, 1, 0), Vertex(1, 1, 1), Vertex(1, 0, 1)};
    Surface(5).Vertex = {Vertex(0, 0, 0), Vertex(1, 0, 0), Vertex(1, 1, 0), Vertex(0, 1, 0)};
    Surface(6).Vertex = {Vertex(0, 0, 1), Vertex(1, 0, 1), Vertex(1, 1, 1), Vertex(0, 1, 1)};

    // Surface octree
    SurfaceOctreeCube const cube(Surface);

    EXPECT_EQ(Vertex(0.0, 0.0, 0.0), cube.l());
    EXPECT_EQ(Vertex(0.5, 0.5, 0.5), cube.c());
    EXPECT_EQ(Vertex(1.0, 1.0, 1.0), cube.u());
    EXPECT_EQ(1.0, cube.w());

    { // Contains
        EXPECT_TRUE(cube.contains(Vertex(0, 1, 0)));
        EXPECT_FALSE(cube.contains(Vertex(0, 2, 0)));
        EXPECT_TRUE(cube.contains(Surface(1)));
        EXPECT_TRUE(cube.contains(Surface(2)));
        EXPECT_TRUE(cube.contains(Surface(3)));
        EXPECT_TRUE(cube.contains(Surface(4)));
        EXPECT_TRUE(cube.contains(Surface(5)));
        EXPECT_TRUE(cube.contains(Surface(6)));
        SurfaceData other_surface(surface);
        other_surface.Vertex = {Vertex(0, 0, 0), Vertex(1, 0, 0), Vertex(1, 0, 1), 2 * Vertex(0, 0, 1)};
        EXPECT_FALSE(cube.contains(other_surface));
    }

    // Line/segment/ray intersections with cube enclosing sphere
    { // X line through center
        Vertex const a(-1.0, 0.5, 0.5), b(2.0, 0.5, 0.5), dir((b - a).normalize_zero());
        EXPECT_TRUE(cube.segmentIntersectsSphere(a, b));
        EXPECT_TRUE(cube.rayIntersectsSphere(a, dir));
        EXPECT_TRUE(cube.lineIntersectsSphere(a, dir));
        Surfaces surfaces;
        cube.surfacesSegmentIntersectsSphere(a, b, surfaces);
        EXPECT_EQ(6u, surfaces.size());
        surfaces.clear();
        cube.surfacesRayIntersectsSphere(a, dir, surfaces);
        EXPECT_EQ(6u, surfaces.size());
        surfaces.clear();
        cube.surfacesLineIntersectsSphere(a, dir, surfaces);
        EXPECT_EQ(6u, surfaces.size());
        surfaces.clear();
    }
    { // Diagonal
        Vertex const a(0.0), b(1.0), dir((b - a).normalize_zero());
        EXPECT_TRUE(cube.segmentIntersectsSphere(a, b));
        EXPECT_TRUE(cube.rayIntersectsSphere(a, dir));
        EXPECT_TRUE(cube.lineIntersectsSphere(a, dir));
    }
    { // Center up half-diagonal
        Vertex const a(0.5), b(2.0), dir((b - a).normalize_zero());
        EXPECT_TRUE(cube.segmentIntersectsSphere(a, b));
        EXPECT_TRUE(cube.rayIntersectsSphere(a, dir));
        EXPECT_TRUE(cube.lineIntersectsSphere(a, dir));
    }
    { // Outward at corner
        Vertex const a(1.0), b(2.0), dir((b - a).normalize_zero());
        EXPECT_TRUE(cube.segmentIntersectsSphere(a, b));
        EXPECT_TRUE(cube.rayIntersectsSphere(a, dir));
        EXPECT_TRUE(cube.lineIntersectsSphere(a, dir));
    }
    { // Tangent at origin
        Vertex const a(-1.0 + 1.0e-12, 1.0 + 1.0e-12, 0.0), b(1.0 + 1.0e-12, -1.0 + 1.0e-12, 0.0), dir((b - a).normalize_zero());
        EXPECT_TRUE(cube.segmentIntersectsSphere(a, b));
        EXPECT_TRUE(cube.rayIntersectsSphere(a, dir));
        EXPECT_TRUE(cube.lineIntersectsSphere(a, dir));
    }
    { // Inward outside but pointing to corner
        Vertex const a(3.0), b(2.0), dir((b - a).normalize_zero());
        EXPECT_FALSE(cube.segmentIntersectsSphere(a, b));
        EXPECT_TRUE(cube.rayIntersectsSphere(a, dir));
        EXPECT_TRUE(cube.lineIntersectsSphere(a, dir));
    }
    { // Just beyond corner
        Vertex const a(1.0000001), b(2.0000002), dir((b - a).normalize_zero());
        EXPECT_FALSE(cube.segmentIntersectsSphere(a, b));
        EXPECT_FALSE(cube.rayIntersectsSphere(a, dir));
        EXPECT_TRUE(cube.lineIntersectsSphere(a, dir));
    }
    { // Way outside cube but along diagonal
        Vertex const a(5.0), b(9.0), dir((b - a).normalize_zero());
        EXPECT_FALSE(cube.segmentIntersectsSphere(a, b));
        EXPECT_FALSE(cube.rayIntersectsSphere(a, dir));
        EXPECT_TRUE(cube.lineIntersectsSphere(a, dir));
    }
    { // Way outside cube with no contact
        Vertex const a(9.0), b(12.0, 8.0, 4.0), dir((b - a).normalize_zero());
        EXPECT_FALSE(cube.segmentIntersectsSphere(a, b));
        EXPECT_FALSE(cube.rayIntersectsSphere(a, dir));
        EXPECT_FALSE(cube.lineIntersectsSphere(a, dir));
    }

    // Line/segment/ray intersections with cube
    { // X line through center
        Vertex const a(-1.0, 0.5, 0.5), b(2.0, 0.5, 0.5), dir((b - a).normalize_zero()), dir_inv(SurfaceOctreeCube::safe_inverse(dir));
        EXPECT_TRUE(cube.segmentIntersectsCube(a, b));
        EXPECT_TRUE(cube.rayIntersectsCube(a, dir, dir_inv));
        EXPECT_TRUE(cube.lineIntersectsCube(a, dir, dir_inv));
        Surfaces surfaces;
        cube.surfacesSegmentIntersectsCube(a, b, surfaces);
        EXPECT_EQ(6u, surfaces.size());
        surfaces.clear();
        cube.surfacesRayIntersectsCube(a, dir, dir_inv, surfaces);
        EXPECT_EQ(6u, surfaces.size());
        surfaces.clear();
        cube.surfacesLineIntersectsCube(a, dir, dir_inv, surfaces);
        EXPECT_EQ(6u, surfaces.size());
    }
    { // Diagonal
        Vertex const a(0.0), b(1.0), dir((b - a).normalize_zero()), dir_inv(1.0 / dir);
        EXPECT_TRUE(cube.segmentIntersectsCube(a, b));
        EXPECT_TRUE(cube.rayIntersectsCube(a, dir, dir_inv));
        EXPECT_TRUE(cube.lineIntersectsCube(a, dir, dir_inv));
    }
    { // Center up half-diagonal
        Vertex const a(0.5), b(2.0), dir((b - a).normalize_zero()), dir_inv(1.0 / dir);
        EXPECT_TRUE(cube.segmentIntersectsCube(a, b));
        EXPECT_TRUE(cube.rayIntersectsCube(a, dir, dir_inv));
        EXPECT_TRUE(cube.lineIntersectsCube(a, dir, dir_inv));
    }
    { // Outward at corner
        Vertex const a(1.0), b(2.0), dir((b - a).normalize_zero()), dir_inv(1.0 / dir);
        EXPECT_TRUE(cube.segmentIntersectsCube(a, b));
        EXPECT_TRUE(cube.rayIntersectsCube(a, dir, dir_inv));
        EXPECT_TRUE(cube.lineIntersectsCube(a, dir, dir_inv));
    }
    { // Tangent at origin
        Vertex const a(-1.0 + 1.0e-12, 1.0 + 1.0e-12, 0.0), b(1.0 + 1.0e-12, -1.0 + 1.0e-12, 0.0), dir((b - a).normalize_zero()),
            dir_inv(1.0 / dir.x, 1.0 / dir.y, 0.0);
        EXPECT_TRUE(cube.segmentIntersectsCube(a, b));
        EXPECT_TRUE(cube.rayIntersectsCube(a, dir, dir_inv));
        EXPECT_TRUE(cube.lineIntersectsCube(a, dir, dir_inv));
    }
    { // Inward outside but pointing to corner
        Vertex const a(3.0), b(2.0), dir((b - a).normalize_zero()), dir_inv(1.0 / dir);
        EXPECT_FALSE(cube.segmentIntersectsCube(a, b));
        EXPECT_TRUE(cube.rayIntersectsCube(a, dir, dir_inv));
        EXPECT_TRUE(cube.lineIntersectsCube(a, dir, dir_inv));
    }
    { // Just beyond corner
        Vertex const a(1.0000001), b(2.0000002), dir((b - a).normalize_zero()), dir_inv(1.0 / dir);
        EXPECT_FALSE(cube.segmentIntersectsCube(a, b));
        EXPECT_FALSE(cube.rayIntersectsCube(a, dir, dir_inv));
        EXPECT_TRUE(cube.lineIntersectsCube(a, dir, dir_inv));
    }
    { // Way outside cube but along diagonal
        Vertex const a(5.0), b(9.0), dir((b - a).normalize_zero()), dir_inv(1.0 / dir);
        EXPECT_FALSE(cube.segmentIntersectsCube(a, b));
        EXPECT_FALSE(cube.rayIntersectsCube(a, dir, dir_inv));
        EXPECT_TRUE(cube.lineIntersectsCube(a, dir, dir_inv));
    }
    { // Way outside cube with no contact
        Vertex const a(9.0), b(12.0, 8.0, 4.0), dir((b - a).normalize_zero()), dir_inv(1.0 / dir);
        EXPECT_FALSE(cube.segmentIntersectsCube(a, b));
        EXPECT_FALSE(cube.rayIntersectsCube(a, dir, dir_inv));
        EXPECT_FALSE(cube.lineIntersectsCube(a, dir, dir_inv));
    }

    { // X-axis aligned line through enclosing sphere but not cube
        Vertex const a(-1.0, 1.1, 1.1), b(2.0, 1.1, 1.1), dir((b - a).normalize_zero());

        EXPECT_TRUE(cube.segmentIntersectsSphere(a, b));
        EXPECT_TRUE(cube.rayIntersectsSphere(a, dir));
        EXPECT_TRUE(cube.lineIntersectsSphere(a, dir));
        EXPECT_FALSE(cube.segmentIntersectsCube(a, b));
        EXPECT_FALSE(cube.rayIntersectsCube(a, dir));
        EXPECT_FALSE(cube.lineIntersectsCube(a, dir));

        Surfaces surfaces;
        cube.surfacesSegmentIntersectsSphere(a, b, surfaces);
        EXPECT_EQ(6u, surfaces.size());
        surfaces.clear();
        cube.surfacesRayIntersectsSphere(a, dir, surfaces);
        EXPECT_EQ(6u, surfaces.size());
        surfaces.clear();
        cube.surfacesLineIntersectsSphere(a, dir, surfaces);
        EXPECT_EQ(6u, surfaces.size());

        surfaces.clear();
        cube.surfacesSegmentIntersectsCube(a, b, surfaces);
        EXPECT_EQ(0u, surfaces.size());
        surfaces.clear();
        cube.surfacesRayIntersectsCube(a, dir, surfaces);
        EXPECT_EQ(0u, surfaces.size());
        surfaces.clear();
        cube.surfacesLineIntersectsCube(a, dir, surfaces);
        EXPECT_EQ(0u, surfaces.size());
    }
}

TEST_F(EnergyPlusFixture, Composite)
{
    // Surfaces: Unit Cube in 2-Unit Cube
    state->dataSurface->TotSurfaces = 12;
    SurfaceData surface;
    surface.Area = 1.0;
    surface.Sides = 4;
    surface.Vertex.dimension(4);
    state->dataSurface->Surface.dimension(state->dataSurface->TotSurfaces, surface);
    // Outer [0,2] cube
    state->dataSurface->Surface(1).Vertex = {2 * Vertex(0, 0, 0), 2 * Vertex(1, 0, 0), 2 * Vertex(1, 0, 1), 2 * Vertex(0, 0, 1)};
    state->dataSurface->Surface(2).Vertex = {2 * Vertex(0, 1, 0), 2 * Vertex(1, 1, 0), 2 * Vertex(1, 1, 1), 2 * Vertex(0, 1, 1)};
    state->dataSurface->Surface(3).Vertex = {2 * Vertex(0, 0, 0), 2 * Vertex(0, 1, 0), 2 * Vertex(0, 1, 1), 2 * Vertex(0, 0, 1)};
    state->dataSurface->Surface(4).Vertex = {2 * Vertex(1, 0, 0), 2 * Vertex(1, 1, 0), 2 * Vertex(1, 1, 1), 2 * Vertex(1, 0, 1)};
    state->dataSurface->Surface(5).Vertex = {2 * Vertex(0, 0, 0), 2 * Vertex(1, 0, 0), 2 * Vertex(1, 1, 0), 2 * Vertex(0, 1, 0)};
    state->dataSurface->Surface(6).Vertex = {2 * Vertex(0, 0, 1), 2 * Vertex(1, 0, 1), 2 * Vertex(1, 1, 1), 2 * Vertex(0, 1, 1)};
    // Inner [0,1] cube
    state->dataSurface->Surface(7).Vertex = {Vertex(0, 0, 0), Vertex(1, 0, 0), Vertex(1, 0, 1), Vertex(0, 0, 1)};
    state->dataSurface->Surface(8).Vertex = {Vertex(0, 1, 0), Vertex(1, 1, 0), Vertex(1, 1, 1), Vertex(0, 1, 1)};
    state->dataSurface->Surface(9).Vertex = {Vertex(0, 0, 0), Vertex(0, 1, 0), Vertex(0, 1, 1), Vertex(0, 0, 1)};
    state->dataSurface->Surface(10).Vertex = {Vertex(1, 0, 0), Vertex(1, 1, 0), Vertex(1, 1, 1), Vertex(1, 0, 1)};
    state->dataSurface->Surface(11).Vertex = {Vertex(0, 0, 0), Vertex(1, 0, 0), Vertex(1, 1, 0), Vertex(0, 1, 0)};
    state->dataSurface->Surface(12).Vertex = {Vertex(0, 0, 1), Vertex(1, 0, 1), Vertex(1, 1, 1), Vertex(0, 1, 1)};
    for (int i = 1; i <= state->dataSurface->TotSurfaces; ++i) {
        state->dataSurface->Surface(i).Shape = SurfaceShape::Rectangle;
    }

    // SurfaceOctreeCube
    SurfaceOctreeCube const cube(state->dataSurface->Surface);

    EXPECT_EQ(Vertex(0.0, 0.0, 0.0), cube.l());
    EXPECT_EQ(Vertex(1.0, 1.0, 1.0), cube.c());
    EXPECT_EQ(Vertex(2.0, 2.0, 2.0), cube.u());
    EXPECT_EQ(2.0, cube.w());

    // Cube and enclosing sphere intersections
    { // X line through outer and inner cube
        Vertex const a(-1.0, 0.5, 0.5), b(3.0, 0.5, 0.5), dir((b - a).normalize_zero());
        EXPECT_TRUE(cube.segmentIntersectsSphere(a, b));
        EXPECT_TRUE(cube.rayIntersectsSphere(a, dir));
        EXPECT_TRUE(cube.lineIntersectsSphere(a, dir));
        EXPECT_TRUE(cube.segmentIntersectsCube(a, b));
        EXPECT_TRUE(cube.rayIntersectsCube(a, dir));
        EXPECT_TRUE(cube.lineIntersectsCube(a, dir));

        Surfaces surfaces;
        cube.surfacesSegmentIntersectsSphere(a, b, surfaces);
        EXPECT_EQ(12u, surfaces.size());
        surfaces.clear();
        cube.surfacesRayIntersectsSphere(a, dir, surfaces);
        EXPECT_EQ(12u, surfaces.size());
        surfaces.clear();
        cube.surfacesLineIntersectsSphere(a, dir, surfaces);
        EXPECT_EQ(12u, surfaces.size());

        surfaces.clear();
        cube.surfacesSegmentIntersectsCube(a, b, surfaces);
        EXPECT_EQ(12u, surfaces.size());
        surfaces.clear();
        cube.surfacesRayIntersectsCube(a, dir, surfaces);
        EXPECT_EQ(12u, surfaces.size());
        surfaces.clear();
        cube.surfacesLineIntersectsCube(a, dir, surfaces);
        EXPECT_EQ(12u, surfaces.size());
    }
    { // X line through outer and inner enclosing spheres and outer cube but not inner cube
        Vertex const a(-1.0, 1.1, 1.1), b(3.0, 1.1, 1.1), dir((b - a).normalize_zero());
        EXPECT_TRUE(cube.segmentIntersectsSphere(a, b));
        EXPECT_TRUE(cube.rayIntersectsSphere(a, dir));
        EXPECT_TRUE(cube.lineIntersectsSphere(a, dir));
        EXPECT_TRUE(cube.segmentIntersectsCube(a, b));
        EXPECT_TRUE(cube.rayIntersectsCube(a, dir));
        EXPECT_TRUE(cube.lineIntersectsCube(a, dir));

        Surfaces surfaces;
        cube.surfacesSegmentIntersectsSphere(a, b, surfaces);
        EXPECT_EQ(12u, surfaces.size());
        surfaces.clear();
        cube.surfacesRayIntersectsSphere(a, dir, surfaces);
        EXPECT_EQ(12u, surfaces.size());
        surfaces.clear();
        cube.surfacesLineIntersectsSphere(a, dir, surfaces);
        EXPECT_EQ(12u, surfaces.size());

        surfaces.clear();
        cube.surfacesSegmentIntersectsCube(a, b, surfaces);
        EXPECT_EQ(6u, surfaces.size());
        surfaces.clear();
        cube.surfacesRayIntersectsCube(a, dir, surfaces);
        EXPECT_EQ(6u, surfaces.size());
        surfaces.clear();
        cube.surfacesLineIntersectsCube(a, dir, surfaces);
        EXPECT_EQ(6u, surfaces.size());
    }
    { // X line through outer but not inner cube
        Vertex const a(-1.0, 1.5, 1.5), b(3.0, 1.5, 1.5), dir((b - a).normalize_zero());
        EXPECT_TRUE(cube.segmentIntersectsSphere(a, b));
        EXPECT_TRUE(cube.rayIntersectsSphere(a, dir));
        EXPECT_TRUE(cube.lineIntersectsSphere(a, dir));
        EXPECT_TRUE(cube.segmentIntersectsCube(a, b));
        EXPECT_TRUE(cube.rayIntersectsCube(a, dir));
        EXPECT_TRUE(cube.lineIntersectsCube(a, dir));

        Surfaces surfaces;
        cube.surfacesSegmentIntersectsSphere(a, b, surfaces);
        EXPECT_EQ(6u, surfaces.size());
        surfaces.clear();
        cube.surfacesRayIntersectsSphere(a, dir, surfaces);
        EXPECT_EQ(6u, surfaces.size());
        surfaces.clear();
        cube.surfacesLineIntersectsSphere(a, dir, surfaces);
        EXPECT_EQ(6u, surfaces.size());
        surfaces.clear();

        surfaces.clear();
        cube.surfacesSegmentIntersectsCube(a, b, surfaces);
        EXPECT_EQ(6u, surfaces.size());
        surfaces.clear();
        cube.surfacesRayIntersectsCube(a, dir, surfaces);
        EXPECT_EQ(6u, surfaces.size());
        surfaces.clear();
        cube.surfacesLineIntersectsCube(a, dir, surfaces);
        EXPECT_EQ(6u, surfaces.size());
    }

    // Function processing
    { // Hits cube but predicate never satisfied
        Vertex const a(-1.0, 0.5, 0.5), b(3.0, 0.5, 0.5), dir((b - a).normalize_zero());
        auto predicate = [](SurfaceData const &surface) -> bool { return surface.Shape == SurfaceShape::Triangle; };
        EXPECT_FALSE(cube.hasSurfaceSegmentIntersectsCube(a, b, predicate));
        EXPECT_FALSE(cube.hasSurfaceRayIntersectsCube(a, dir, predicate));
    }
    { // Hits cube and predicate satisfied
        Vertex const a(-1.0, 0.5, 0.5), b(3.0, 0.5, 0.5), dir((b - a).normalize_zero());
        auto predicate = [](SurfaceData const &surface) -> bool { return surface.Shape == SurfaceShape::Rectangle; };
        EXPECT_TRUE(cube.hasSurfaceSegmentIntersectsCube(a, b, predicate));
        EXPECT_TRUE(cube.hasSurfaceRayIntersectsCube(a, dir, predicate));
    }
    { // Misses cube so predicate never tested
        Vertex const a(-1.0, 3.0, 3.0), b(3.0, 3.0, 3.0), dir((b - a).normalize_zero());
        auto predicate = [](SurfaceData const &surface) -> bool { return surface.Shape == SurfaceShape::Rectangle; };
        EXPECT_FALSE(cube.hasSurfaceSegmentIntersectsCube(a, b, predicate));
        EXPECT_FALSE(cube.hasSurfaceRayIntersectsCube(a, dir, predicate));
    }
    { // Hits cube: find max vertices in a surface
        Vertex const a(-1.0, 0.5, 0.5), b(3.0, 0.5, 0.5), dir((b - a).normalize_zero());
        std::size_t n(0);
        auto function = [&n](SurfaceData const &surface) { n = std::max(n, surface.Vertex.size()); };
        cube.processSurfaceRayIntersectsCube(a, dir, function);
        EXPECT_EQ(4u, n);
    }
    { // Hits cube: find number of surfaces but no higher than 8
        Vertex const a(-1.0, 0.5, 0.5), b(3.0, 0.5, 0.5), dir((b - a).normalize_zero());
        std::size_t n(0);
        auto predicate = [&n](SurfaceData const &) -> bool {
            ++n;
            return n >= 8u;
        };
        cube.processSomeSurfaceRayIntersectsCube(*state, a, dir, predicate);
        EXPECT_EQ(8u, n);
    }
}
