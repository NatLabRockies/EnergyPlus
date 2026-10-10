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
#include <EnergyPlus/Geometry/Vector3D.hh>

using namespace EnergyPlus;

TEST(Vector3Test, Basic)
{
    Vector3D d; // Default construction zero-initializes
    EXPECT_EQ(0.0, d.x);
    EXPECT_EQ(0.0, d.y);
    EXPECT_EQ(0.0, d.z);
    Vector3D v(15.0); // Uniform value construction
    EXPECT_EQ(15.0, v.x);
    EXPECT_EQ(15.0, v.y);
    EXPECT_EQ(15.0, v.z);
    v = {1.0, 2.0, 3.0};
    EXPECT_EQ(1.0, v[0]);
    EXPECT_EQ(2.0, v[1]);
    EXPECT_EQ(3.0, v[2]);
    v[1] = 5.0;
    EXPECT_EQ(5.0, v.y);
    EXPECT_DOUBLE_EQ(5.0, Vector3D(3.0, 4.0, 0.0).length());
    EXPECT_DOUBLE_EQ(25.0, Vector3D(3.0, 4.0, 0.0).length_squared());
}

TEST(Vector3Test, Comparisons)
{
    Vector3D v(1.0, 2.0, 3.0);
    Vector3D w(1.0, 2.0, 3.0);
    EXPECT_EQ(v, w);
    EXPECT_FALSE(v != w);
    v.z = 0.0;
    EXPECT_TRUE(v != w);
    EXPECT_FALSE(v == w);
}

TEST(Vector3Test, Operators)
{
    Vector3D v(1.0, 12.0, 4.0);
    Vector3D w(2.0, 6.0, 1.0);
    EXPECT_EQ(Vector3D(3.0, 18.0, 5.0), v + w);
    EXPECT_EQ(Vector3D(-1.0, 6.0, 3.0), v - w);
    EXPECT_EQ(Vector3D(-1.0, -12.0, -4.0), -v);
    EXPECT_EQ(Vector3D(2.0, 24.0, 8.0), v * 2.0);
    EXPECT_EQ(Vector3D(2.0, 24.0, 8.0), 2.0 * v);
    EXPECT_EQ(Vector3D(0.5, 6.0, 2.0), v / 2.0);
    Vector3D u(v);
    u += w;
    EXPECT_EQ(Vector3D(3.0, 18.0, 5.0), u);
    u -= w;
    EXPECT_EQ(v, u);
    u *= 2.0;
    EXPECT_EQ(Vector3D(2.0, 24.0, 8.0), u);
    u /= 2.0;
    EXPECT_EQ(v, u);
    EXPECT_EQ(Vector3D(1.5, 9.0, 2.5), 0.5 * (v + w)); // midpoint
}

TEST(Vector3Test, DotCrossDistance)
{
    Vector3D x(3.0, 0.0, 0.0);
    Vector3D y(0.0, 2.0, 0.0);
    EXPECT_EQ(0.0, x.dot(y));
    EXPECT_EQ(Vector3D(0.0, 0.0, 6.0), x.cross(y));
    EXPECT_EQ(Vector3D(0.0, 0.0, -6.0), y.cross(x));
    Vector3D v(3.0, 3.0, 3.0);
    Vector3D w(3.0, 2.0, 3.0);
    EXPECT_DOUBLE_EQ(1.0, v.distance(w));
    EXPECT_DOUBLE_EQ(1.0, v.distance_squared(w));
}

TEST(Vector3Test, Normalize)
{
    Vector3D v(0.0, 3.0, 4.0);
    EXPECT_DOUBLE_EQ(1.0, v.normalized().length());
    EXPECT_DOUBLE_EQ(10.0, v.normalized(10.0).length());
    Vector3D n(v);
    n.normalize();
    EXPECT_DOUBLE_EQ(0.6, n.y);
    EXPECT_DOUBLE_EQ(0.8, n.z);
}

TEST(Vector3Test, Constexpr)
{
    constexpr Vector3D a(1.0, 2.0, 3.0);
    constexpr Vector3D b(4.0, 5.0, 6.0);
    static_assert(Vector3D() == Vector3D(0.0, 0.0, 0.0));
    static_assert(a + b == Vector3D(5.0, 7.0, 9.0));
    static_assert(b - a == Vector3D(3.0, 3.0, 3.0));
    static_assert(2.0 * a == Vector3D(2.0, 4.0, 6.0));
    static_assert(-a == Vector3D(-1.0, -2.0, -3.0));
    static_assert(a.dot(b) == 32.0);
    static_assert(a.cross(b) == Vector3D(-3.0, 6.0, -3.0));
    static_assert(a.length_squared() == 14.0);
    static_assert(a.distance_squared(b) == 27.0);
    static_assert(a[2] == 3.0);
    static_assert(a != b);
    SUCCEED();
}
