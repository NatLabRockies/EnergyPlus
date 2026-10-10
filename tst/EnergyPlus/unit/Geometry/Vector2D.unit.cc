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
#include <EnergyPlus/Geometry/Vector2D.hh>

using namespace EnergyPlus;

TEST_F(GeometryFixture, Vector2D_Basic)
{
    Vector2D v(15.0); // Uniform value construction
    EXPECT_EQ(15.0f, v.x);
    EXPECT_EQ(15.0f, v.y);
    v.normalize();
    EXPECT_FLOAT_EQ(1.0f, v.length());
    v.normalize(5.0f);
    EXPECT_FLOAT_EQ(5.0f, v.length());
    v = {0.0, 0.0};
    EXPECT_EQ(0.0f, v.x);
    EXPECT_EQ(0.0f, v.y);
    EXPECT_EQ(0.0f, v.length());
    Vector2D const n(Vector2D(3.0, 4.0).normalized());
    EXPECT_DOUBLE_EQ(0.6, n.x);
    EXPECT_DOUBLE_EQ(0.8, n.y);
}

TEST_F(GeometryFixture, Vector2D_BraceInit)
{
    Vector2D v{33, 52};
    EXPECT_EQ(33, v.x);
    EXPECT_EQ(52, v.y);
    EXPECT_EQ(33, v[0]);
    EXPECT_EQ(52, v[1]);
    v = {44, 55};
    EXPECT_EQ(44, v.x);
    EXPECT_EQ(55, v.y);
    v *= 2;
    EXPECT_EQ(88, v.x);
    EXPECT_EQ(110, v.y);
    v /= 2.0;
    EXPECT_EQ(44, v.x);
    EXPECT_EQ(55, v.y);
    v = -v;
    EXPECT_EQ(-44, v.x);
    EXPECT_EQ(-55, v.y);
}

TEST_F(GeometryFixture, Vector2D_Comparisons)
{
    Vector2D v(1.0, 2.0);
    Vector2D w(1.0, 2.0);

    EXPECT_EQ(v, w);

    // Reduce v and test inequality
    v -= Vector2D(0.5);
    EXPECT_TRUE(v != w);
    EXPECT_TRUE(!(v == w));

    // Increase v and test inequality
    v += Vector2D(1.0);
    EXPECT_TRUE(v != w);
    EXPECT_TRUE(!(v == w));

    // Set v.x to 0: v and w now differ
    v.x = 0.0;
    EXPECT_TRUE(v != w);
    EXPECT_TRUE(!(v == w));
}

TEST_F(GeometryFixture, Vector2D_Generators)
{
    Vector2D v(1.0, 12.0);
    Vector2D w(2.0, 6.0);
    EXPECT_EQ(Vector2D(3.0, 18.0), v + w);
    EXPECT_EQ(Vector2D(-1.0, 6.0), v - w);
    EXPECT_EQ(Vector2D(2.0, 24.0), v * 2.0);
    EXPECT_EQ(Vector2D(2.0, 24.0), 2.0 * v);
    EXPECT_EQ(Vector2D(0.5, 6.0), v / 2.0);
}

TEST_F(GeometryFixture, Vector2D_Distance)
{
    Vector2D v(3.0, 3.0);
    Vector2D w(3.0, 2.0);
    EXPECT_DOUBLE_EQ(1.0, v.distance(w));
    EXPECT_DOUBLE_EQ(1.0, v.distance_squared(w));
    EXPECT_DOUBLE_EQ(5.0, Vector2D(0.0, 0.0).distance(Vector2D(3.0, 4.0)));
}

TEST_F(GeometryFixture, Vector2D_Dot)
{
    Vector2D x(3.0, 0.0);
    Vector2D y(0.0, 2.0);
    EXPECT_EQ(0.0, x.dot(y));
}

TEST_F(GeometryFixture, Vector2D_Cross)
{
    Vector2D x(3.0, 0.0);
    Vector2D y(0.0, 2.0);
    EXPECT_EQ(6.0, x.cross(y));
}

TEST_F(GeometryFixture, Vector2D_Center)
{
    Vector2D x(4.0, 0.0);
    Vector2D y(0.0, 4.0);
    EXPECT_EQ(Vector2D(2.0, 2.0), 0.5 * (x + y));
}

TEST_F(GeometryFixture, Vector2D_BinaryOperations)
{
    Vector2D v(1.0, 2.0);
    Vector2D w(1.0, 2.0);
    Vector2D const original(v);

    // Check dot product of equal vectors
    EXPECT_DOUBLE_EQ(v.length_squared(), v.dot(w)); // v == w here

    // Check midpoint (should match original vector)
    v += Vector2D(1.0);
    w -= Vector2D(1.0);
    Vector2D const midpoint(0.5 * (v + w));
    EXPECT_DOUBLE_EQ(original.x, midpoint.x);
    EXPECT_DOUBLE_EQ(original.y, midpoint.y);
}

TEST_F(GeometryFixture, Vector2D_Constexpr)
{
    constexpr Vector2D a(1.0, 2.0);
    constexpr Vector2D b(3.0, 4.0);
    static_assert(Vector2D() == Vector2D(0.0, 0.0));
    static_assert(a + b == Vector2D(4.0, 6.0));
    static_assert(b - a == Vector2D(2.0, 2.0));
    static_assert(2.0 * a == Vector2D(2.0, 4.0));
    static_assert(b / 2.0 == Vector2D(1.5, 2.0));
    static_assert(-a == Vector2D(-1.0, -2.0));
    static_assert(a.dot(b) == 11.0);
    static_assert(a.cross(b) == -2.0);
    static_assert(a.length_squared() == 5.0);
    static_assert(a.distance_squared(b) == 8.0);
    static_assert(a[1] == 2.0);
    static_assert(0.5 * (a + b) == Vector2D(2.0, 3.0));
    static_assert(a != b);
    SUCCEED();
}
