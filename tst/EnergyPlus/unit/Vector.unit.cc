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

// EnergyPlus::DataVectorTypes::Vector Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataVectorTypes.hh>
#include <EnergyPlus/UtilityRoutines.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;
using namespace EnergyPlus::DataVectorTypes;

TEST_F(EnergyPlusFixture, VectorTest_Basic)
{
    ShowMessage(*state, "Begin Test: VectorTest, Basic");
    {
        Vector v(0.0, 0.0, 0.0);
        EXPECT_EQ(0.0, v.x);
        EXPECT_EQ(0.0, v.y);
        EXPECT_EQ(0.0, v.z);
        EXPECT_EQ(0.0, v.length());
        EXPECT_EQ(0.0, v.length_squared());
        EXPECT_EQ(0.0, magnitude(v));
        EXPECT_EQ(0.0, magnitude_squared(v));
        v *= 22.0;
        EXPECT_EQ(0.0, v.x);
        EXPECT_EQ(0.0, v.y);
        EXPECT_EQ(0.0, v.z);
        EXPECT_EQ(0.0, v.length());
        EXPECT_EQ(0.0, v.length_squared());
        EXPECT_EQ(0.0, magnitude(v));
        EXPECT_EQ(0.0, magnitude_squared(v));
        v += 2.0;
        EXPECT_EQ(2.0, v.x);
        EXPECT_EQ(2.0, v.y);
        EXPECT_EQ(2.0, v.z);
        EXPECT_EQ(12.0, v.length_squared());
        EXPECT_EQ(12.0, magnitude_squared(v));
        v /= 2.0;
        EXPECT_EQ(1.0, v.x);
        EXPECT_EQ(1.0, v.y);
        EXPECT_EQ(1.0, v.z);
        EXPECT_EQ(3.0, v.length_squared());
        EXPECT_EQ(3.0, magnitude_squared(v));
        v -= 1.0;
        EXPECT_EQ(0.0, v.x);
        EXPECT_EQ(0.0, v.y);
        EXPECT_EQ(0.0, v.z);
        EXPECT_EQ(0.0, v.length());
        EXPECT_EQ(0.0, v.length_squared());
        EXPECT_EQ(0.0, magnitude(v));
        EXPECT_EQ(0.0, magnitude_squared(v));
        Vector u(v);
        EXPECT_EQ(u.x, v.x);
        EXPECT_EQ(u.y, v.y);
        EXPECT_EQ(u.z, v.z);
    }
    {
        Vector v(3.0);
        EXPECT_EQ(3.0, v.x);
        EXPECT_EQ(3.0, v.y);
        EXPECT_EQ(3.0, v.z);
        EXPECT_EQ(27.0, v.length_squared());
        EXPECT_EQ(27.0, magnitude_squared(v));
        Vector u(v * 2.0);
        EXPECT_EQ(6.0, u.x);
        EXPECT_EQ(6.0, u.y);
        EXPECT_EQ(6.0, u.z);
        Vector w(-u);
        EXPECT_EQ(-6.0, w.x);
        EXPECT_EQ(-6.0, w.y);
        EXPECT_EQ(-6.0, w.z);
    }
    {
        Vector v(1.0, 2.0, 3.0);
        EXPECT_EQ(1.0, v.x);
        EXPECT_EQ(2.0, v.y);
        EXPECT_EQ(3.0, v.z);
        EXPECT_EQ(14.0, v.length_squared());
        EXPECT_EQ(14.0, magnitude_squared(v));
        Vector u(1.0, 2.0, 3.0);
        EXPECT_EQ(0.0, distance(u, v));
        EXPECT_EQ(0.0, distance_squared(u, v));
        EXPECT_EQ(14.0, dot(u, v));
        Vector x(cross(u, v));
        EXPECT_EQ(0.0, x.x);
        EXPECT_EQ(0.0, x.y);
        EXPECT_EQ(0.0, x.z);
    }
    {
        Vector v(1.0, 2.0, 3.0);
        EXPECT_EQ(1.0, v.x);
        EXPECT_EQ(2.0, v.y);
        EXPECT_EQ(3.0, v.z);
        EXPECT_EQ(14.0, v.length_squared());
        EXPECT_EQ(14.0, magnitude_squared(v));
        Vector u(2.0);
        EXPECT_EQ(2.0, distance_squared(u, v));
        EXPECT_EQ(12.0, dot(u, v));
        Vector x(cross(u, v));
        EXPECT_EQ(2.0, x.x);
        EXPECT_EQ(-4.0, x.y);
        EXPECT_EQ(2.0, x.z);
    }
}
