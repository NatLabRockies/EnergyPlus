// EnergyPlus, Copyright (c) 1996-present, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Energy Innovation, LLC, and other
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

// Google Test Headers
#include <gtest/gtest.h>

#include "GeometryFixture.hh"

// EnergyPlus Headers
#include <EnergyPlus/Geometry/Plane.hh>
#include <EnergyPlus/Geometry/Vector3D.hh>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// C++ Headers
#include <cmath>
#include <optional>
#include <sstream>

using namespace EnergyPlus;

TEST_F(GeometryFixture, Basic)
{
    Plane v; // Default construction zero-initializes
    EXPECT_EQ(0.0, v.x);
    EXPECT_EQ(0.0, v.y);
    EXPECT_EQ(0.0, v.z);
    EXPECT_EQ(0.0, v.w);
    v = Plane(1.0, 2.0, 3.0, 4.0); // Value construction
    EXPECT_EQ(1.0, v.x);
    EXPECT_EQ(2.0, v.y);
    EXPECT_EQ(3.0, v.z);
    EXPECT_EQ(4.0, v.w);
}

TEST_F(GeometryFixture, Normalize)
{
    // Normalizing a plane scales the normal (x, y, z) to unit length; w (the offset) scales with it, not into the length
    Plane p(3.0, 0.0, 4.0, 10.0); // |normal| = 5
    Plane const q(p.normalized());
    EXPECT_DOUBLE_EQ(0.6, q.x);
    EXPECT_DOUBLE_EQ(0.0, q.y);
    EXPECT_DOUBLE_EQ(0.8, q.z);
    EXPECT_DOUBLE_EQ(2.0, q.w);
    EXPECT_DOUBLE_EQ(1.0, std::sqrt((q.x * q.x) + (q.y * q.y) + (q.z * q.z)));
    EXPECT_DOUBLE_EQ(3.0, p.x); // normalized() leaves the original alone

    p.normalize();
    EXPECT_EQ(q, p);

    // The plane x = 3 stays x = 3 however it is scaled: (2, 0, 0, -6) -> (1, 0, 0, -3)
    Plane s(2.0, 0.0, 0.0, -6.0);
    s.normalize();
    EXPECT_DOUBLE_EQ(1.0, s.x);
    EXPECT_DOUBLE_EQ(0.0, s.y);
    EXPECT_DOUBLE_EQ(0.0, s.z);
    EXPECT_DOUBLE_EQ(-3.0, s.w);
}

TEST_F(GeometryFixture, Comparisons)
{
    Plane v(1.0, 2.0, 3.0, 4.0);
    Plane w(1.0, 2.0, 3.0, 4.0);

    EXPECT_TRUE(v == w);
    EXPECT_FALSE(v != w);

    // Each coefficient takes part in the comparison
    v.x = 0.0;
    EXPECT_TRUE(v != w);
    EXPECT_FALSE(v == w);
    v.x = w.x;
    v.w = 4.5;
    EXPECT_TRUE(v != w);
    EXPECT_FALSE(v == w);

    // The comparison is exact on the representation: a scaled copy is the same plane but compares unequal
    EXPECT_NE(Plane(1.0, 0.0, 0.0, -1.0), Plane(2.0, 0.0, 0.0, -2.0));
}

TEST_F(GeometryFixture, Generators)
{
    Plane v(1.0, 12.0, 21.0, 18.0);
    EXPECT_EQ(Plane(-1.0, -12.0, -21.0, -18.0), -v); // Negation flips the plane's orientation
}

TEST_F(GeometryFixture, StreamOutput)
{
    {
        std::ostringstream os;
        os << Plane(1.0, -2.5, 3.0, 0.25); // Values that print exactly on every platform: no exponent, no rounding
        EXPECT_EQ("[1, -2.5, 3, 0.25]", os.str());
    }
    {
        std::ostringstream os;
        os << Plane();
        EXPECT_EQ("[0, 0, 0, 0]", os.str());
    }
}

// The planes below are written as a*x + b*y + c*z + d = 0 with d = -(normal . point_on_plane).
// Where a test says "Newell", the coefficients are what Plane::fromVertices() produces (and SurfaceData::plane holds) for that polygon:
// the normal's length is twice the polygon area, so these planes are not normalized.

TEST_F(GeometryFixture, Normal)
{
    Plane const p(3.0, 0.0, 4.0, 10.0);
    Vector3D const n(p.normal());
    EXPECT_EQ(3.0, n.x);
    EXPECT_EQ(0.0, n.y);
    EXPECT_EQ(4.0, n.z);

    // Not unit length unless the plane is normalized
    Vector3D const u(p.normalized().normal());
    EXPECT_DOUBLE_EQ(0.6, u.x);
    EXPECT_DOUBLE_EQ(0.0, u.y);
    EXPECT_DOUBLE_EQ(0.8, u.z);
    EXPECT_DOUBLE_EQ(1.0, u.length());
}

TEST_F(GeometryFixture, ReversedPlane)
{
    Plane const p(1.0, -2.0, 3.0, -4.0);
    Plane const r(-p);
    EXPECT_EQ(Plane(-1.0, 2.0, -3.0, 4.0), r);
    EXPECT_EQ(p, -r);                   // Reversing twice gives the original back
    EXPECT_EQ(-p.normal(), r.normal()); // The normal flips
}

TEST_F(GeometryFixture, RayIntersection)
{
    using Point = Vector3D;
    Plane const z2(0.0, 0.0, 1.0, -2.0); // Plane z = 2, normal +z

    // Ray from below, going up: hits at z = 2
    std::optional<Point> hit = z2.rayIntersection(Point(1.0, 3.0, 0.0), Vector3D::UnitZ());
    ASSERT_TRUE(hit);
    EXPECT_EQ(Point(1.0, 3.0, 2.0), *hit);

    // Either side can be hit: ray from above, going down
    hit = z2.rayIntersection(Point(1.0, 3.0, 5.0), -Vector3D::UnitZ());
    ASSERT_TRUE(hit);
    EXPECT_EQ(Point(1.0, 3.0, 2.0), *hit);

    // Oblique ray: (0, 0, 0) + t * (1, 0, 1) reaches z = 2 at t = 2
    hit = z2.rayIntersection(Point(0.0, 0.0, 0.0), Vector3D(1.0, 0.0, 1.0));
    ASSERT_TRUE(hit);
    EXPECT_EQ(Point(2.0, 0.0, 2.0), *hit);

    // No hit: parallel to the plane, pointing away from it, or starting on it
    EXPECT_FALSE(z2.rayIntersection(Point(0.0, 0.0, 0.0), Vector3D::UnitX()));
    EXPECT_FALSE(z2.rayIntersection(Point(0.0, 0.0, 2.0), Vector3D::UnitX())); // Even when the ray lies in the plane
    EXPECT_FALSE(z2.rayIntersection(Point(0.0, 0.0, 0.0), -Vector3D::UnitZ()));
    EXPECT_FALSE(z2.rayIntersection(Point(0.0, 0.0, 2.0), Vector3D::UnitZ()));

    // tMax: the plane is 2 away along a unit direction
    EXPECT_TRUE(z2.rayIntersection(Point(0.0, 0.0, 0.0), Vector3D::UnitZ(), 3.0));
    EXPECT_TRUE(z2.rayIntersection(Point(0.0, 0.0, 0.0), Vector3D::UnitZ(), 2.0)); // t == tMax is a hit
    EXPECT_FALSE(z2.rayIntersection(Point(0.0, 0.0, 0.0), Vector3D::UnitZ(), 1.0));

    // A non-normalized plane (same plane, scaled by 7) gives the same hit point
    hit = Plane(0.0, 0.0, 7.0, -14.0).rayIntersection(Point(1.0, 3.0, 0.0), Vector3D::UnitZ());
    ASSERT_TRUE(hit);
    EXPECT_EQ(Point(1.0, 3.0, 2.0), *hit);
}

TEST_F(GeometryFixture, SignedDistance)
{
    using Point = Vector3D;

    // x = 10 facing +x: positive in front (x > 10), negative behind, zero on the plane
    Plane const x10(1.0, 0.0, 0.0, -10.0);
    EXPECT_DOUBLE_EQ(2.0, x10.signedDistance(Point(12.0, 5.0, 7.0)));
    EXPECT_DOUBLE_EQ(-3.0, x10.signedDistance(Point(7.0, -1.0, 4.0)));
    EXPECT_DOUBLE_EQ(0.0, x10.signedDistance(Point(10.0, 3.0, -8.0)));

    // Reversing the plane flips the sign
    EXPECT_DOUBLE_EQ(-2.0, (-x10).signedDistance(Point(12.0, 5.0, 7.0)));

    // A true distance even when the plane is not normalized: the same plane scaled by 2 (Newell of a 1 m^2 surface)
    // and by 1e-4 (a tiny surface) gives the same distances
    Plane const scaled(2.0, 0.0, 0.0, -20.0);
    Plane const tiny(1.0e-4, 0.0, 0.0, -1.0e-3);
    EXPECT_DOUBLE_EQ(2.0, scaled.signedDistance(Point(12.0, 5.0, 7.0)));
    EXPECT_DOUBLE_EQ(2.0, tiny.signedDistance(Point(12.0, 5.0, 7.0)));

    // Tilted plane 3x + 4z + 10 = 0: |normal| = 5, so the origin is 10 / 5 = 2 in front
    Plane const tilted(3.0, 0.0, 4.0, 10.0);
    EXPECT_DOUBLE_EQ(2.0, tilted.signedDistance(Point(0.0, 0.0, 0.0)));
    EXPECT_DOUBLE_EQ(0.0, tilted.signedDistance(Point(-2.0, 0.0, -1.0))); // 3*(-2) + 4*(-1) + 10 = 0
    EXPECT_DOUBLE_EQ(2.0, tilted.normalized().signedDistance(Point(0.0, 0.0, 0.0)));

    // Moving along the unit normal by s changes the signed distance by s
    Point const onPlane(-2.0, 7.0, -1.0);
    Point const unitNormal(tilted.normalized().normal());
    EXPECT_DOUBLE_EQ(1.5, tilted.signedDistance(onPlane + 1.5 * unitNormal));
    EXPECT_DOUBLE_EQ(-0.25, tilted.signedDistance(onPlane - 0.25 * unitNormal));
}

TEST_F(GeometryFixture, FromVertices)
{
    using Point = Vector3D;
    using Vertices = ObjexxFCL::Array1D<Point>;

    {
        // 1 m x 1 m square at z = 0, counterclockwise seen from above: normal +z, length 2 * area = 2
        Vertices const v({Point(0.0, 0.0, 0.0), Point(1.0, 0.0, 0.0), Point(1.0, 1.0, 0.0), Point(0.0, 1.0, 0.0)});
        Plane const p(Plane::fromVertices(v));
        EXPECT_EQ(Plane(0.0, 0.0, 2.0, 0.0), p);
        EXPECT_EQ(Plane(0.0, 0.0, 1.0, 0.0), p.normalized());
    }
    {
        // Same square, clockwise: the normal flips
        Vertices const v({Point(0.0, 1.0, 0.0), Point(1.0, 1.0, 0.0), Point(1.0, 0.0, 0.0), Point(0.0, 0.0, 0.0)});
        Plane const p(Plane::fromVertices(v));
        EXPECT_EQ(Plane(0.0, 0.0, -2.0, 0.0), p);
        EXPECT_EQ(Plane(0.0, 0.0, 1.0, 0.0), (-p).normalized());
    }
    {
        // 2 m x 3 m rectangle at z = 2: |normal| = 12, plane z = 2
        Vertices const v({Point(0.0, 0.0, 2.0), Point(2.0, 0.0, 2.0), Point(2.0, 3.0, 2.0), Point(0.0, 3.0, 2.0)});
        Plane const p(Plane::fromVertices(v));
        EXPECT_EQ(Plane(0.0, 0.0, 12.0, -24.0), p);
        EXPECT_DOUBLE_EQ(5.0, p.signedDistance(Point(1.0, 1.0, 7.0)));
    }
    {
        // Wall at x = 10 facing +x (counterclockwise seen from +x)
        Vertices const v({Point(10.0, 0.0, 1.0), Point(10.0, 0.0, 0.0), Point(10.0, 1.0, 0.0), Point(10.0, 1.0, 1.0)});
        Plane const p(Plane::fromVertices(v));
        EXPECT_EQ(Plane(2.0, 0.0, 0.0, -20.0), p);
        EXPECT_EQ(Plane(1.0, 0.0, 0.0, -10.0), p.normalized());
        for (Point const &vertex : v) {
            EXPECT_DOUBLE_EQ(0.0, p.signedDistance(vertex));
        }
    }
    {
        // Same triangles and expected (raw Newell) coefficients as SurfaceTest_Plane in DataSurfaces.unit.cc
        EXPECT_EQ(Plane(-1.0, 3.0, 2.0, -4.0), Plane::fromVertices(Vertices({Point(1, 1, 1), Point(-1, 1, 0), Point(2, 0, 3)})));
        EXPECT_EQ(Plane(-7.0, 5.0, 1.0, 10.0), Plane::fromVertices(Vertices({Point(2, 1, -1), Point(0, -2, 0), Point(1, -1, 2)})));
    }
    {
        // Small surface: 1 cm x 1 cm square at z = 0
        Vertices const v({Point(0.0, 0.0, 0.0), Point(0.01, 0.0, 0.0), Point(0.01, 0.01, 0.0), Point(0.0, 0.01, 0.0)});
        Plane const p(Plane::fromVertices(v));
        EXPECT_DOUBLE_EQ(2.0e-4, p.z);
        EXPECT_DOUBLE_EQ(1.0, p.normalized().z);                         // Still a unit +z normal once normalized
        EXPECT_DOUBLE_EQ(0.0, p.signedDistance(Point(10.0, 10.0, 0.0))); // Far point on the same plane
    }
    {
        // Degenerate: collinear vertices give a zero normal
        Vertices const v({Point(0.0, 0.0, 0.0), Point(1.0, 0.0, 0.0), Point(2.0, 0.0, 0.0)});
        Plane const p(Plane::fromVertices(v));
        EXPECT_EQ(0.0, p.normal().length());
        EXPECT_TRUE(p.isDegenerate());
        EXPECT_FALSE(Plane::fromVertices(Vertices({Point(0.0, 0.0, 0.0), Point(1.0, 0.0, 0.0), Point(1.0, 1.0, 0.0)})).isDegenerate());
        EXPECT_TRUE(Plane().isDegenerate()); // Default (zero) plane
    }
}
