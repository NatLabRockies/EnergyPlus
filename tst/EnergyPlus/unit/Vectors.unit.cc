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

// EnergyPlus::Vectors Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/Vectors.hh>

#include "Fixtures/EnergyPlusFixture.hh"

// C++ Headers
#include <cmath>

using namespace EnergyPlus;
using namespace EnergyPlus::Vectors;

TEST_F(EnergyPlusFixture, VectorsTest_AreaPolygon)
{
    Array1D<Vector> a(4); // 3 x 7 rectangle
    a(1).x = a(1).y = a(1).z = 0.0;
    a(2).x = 3.0;
    a(2).y = a(2).z = 0.0;
    a(3).x = 3.0;
    a(3).y = 7.0;
    a(3).z = 0.0;
    a(4).x = 0.0;
    a(4).y = 7.0;
    a(4).z = 0.0;
    EXPECT_EQ(21.0, AreaPolygon(4, a));
}

TEST_F(EnergyPlusFixture, VectorsTest_VecNormalize)
{
    {
        Vector const v(3.0, 3.0, 3.0);
        Vector const n(VecNormalize(v));
        Real64 const h(3.0 / std::sqrt(27.0));
        EXPECT_DOUBLE_EQ(h, n.x);
        EXPECT_DOUBLE_EQ(h, n.y);
        EXPECT_DOUBLE_EQ(h, n.z);
    }
    {
        Vector const v(1.0, 3.0, 5.0);
        Vector const n(VecNormalize(v));
        Real64 const f(1.0 / std::sqrt(35.0));
        EXPECT_DOUBLE_EQ(f * v.x, n.x);
        EXPECT_DOUBLE_EQ(f * v.y, n.y);
        EXPECT_DOUBLE_EQ(f * v.z, n.z);
    }
}

TEST_F(EnergyPlusFixture, VectorsTest_VecRound)
{
    Vector v(11.567, -33.602, 55.981);
    VecRound(v, 2.0);
    EXPECT_DOUBLE_EQ(11.5, v.x);
    EXPECT_DOUBLE_EQ(-33.5, v.y);
    EXPECT_DOUBLE_EQ(56.0, v.z);
}

TEST_F(EnergyPlusFixture, VectorsTest_CoplnarPoints)
{
    {
        Array1D<Vector> base = {Vector(0, 0, 0), Vector(1, 0, 0), Vector(1, 1, 0), Vector(0, 1, 0)};
        std::vector<int> coplanarPoints;
        bool ErrorsFound;

        Array1D<Vector> query = {Vector(0, 0, 0), Vector(1, 1, 1), Vector(2, 0, 0), Vector(0, 0, -1)};
        coplanarPoints = PointsInPlane(base, base.size(), query, query.size(), ErrorsFound);

        EXPECT_EQ(coplanarPoints[0], 1);      // 1st point in query is coplanar with base
        EXPECT_EQ(coplanarPoints[1], 3);      // 3rd point in query is coplanar with base
        EXPECT_EQ(coplanarPoints.size(), 2u); // Only 2 points in query are coplanar with base
    }
}
