// EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the University of Illinois,
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

// EnergyPlus Headers
#include <EnergyPlus/EPVector.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, EPVectorTest_Basic)
{
    EPVector<int> v;
    v.allocate(4);
    EXPECT_TRUE(v.allocated());
    v[0] = 1;
    v[1] = 2;
    v[2] = 3;
    v[3] = 4;
    EXPECT_EQ(1, v(1));
    EXPECT_EQ(2, v(2));
    EXPECT_EQ(3, v(3));
    EXPECT_EQ(4, v(4));
#ifndef NDEBUG
    ASSERT_THROW(v[4] = 5, std::out_of_range);
    ASSERT_THROW(v(5) = 5, std::out_of_range);
#endif // !NDEBUG
    v.resize(2);
    EXPECT_EQ(2, v.size());
    EXPECT_EQ(1, v(1));
    EXPECT_EQ(2, v(2));
    v.resize(4);
    EXPECT_EQ(1, v(1));
    EXPECT_EQ(2, v(2));
    EXPECT_EQ(0, v(3));
    EXPECT_EQ(0, v(4));
    v.clear();
    EXPECT_FALSE(v.allocated());
    EXPECT_TRUE(v.empty());
    v.resize(4);
    EXPECT_EQ(0, v(1));
    EXPECT_EQ(0, v(2));
    EXPECT_EQ(0, v(3));
    EXPECT_EQ(0, v(4));
    v.clear();
    v.resize(4, true);
    EXPECT_EQ(1, v(1));
    EXPECT_EQ(1, v(2));
    EXPECT_EQ(1, v(3));
    EXPECT_EQ(1, v(4));
    v.deallocate();
    EXPECT_FALSE(v.allocated());
    EXPECT_TRUE(v.empty());
    v.resize(3, 2);
    EXPECT_EQ(2, v(1));
    EXPECT_EQ(2, v(2));
    EXPECT_EQ(2, v(3));
    EXPECT_TRUE(v.allocated());
}

TEST_F(EnergyPlusFixture, EPVectorTest_Bools)
{
    EPVector<bool> v;
    v.allocate(4);
    EXPECT_TRUE(v.allocated());
    v[0] = true;
    v[1] = false;
    v[2] = false;
    v[3] = true;
    EXPECT_TRUE(v(1));
    EXPECT_FALSE(v(2));
    EXPECT_FALSE(v(3));
    EXPECT_TRUE(v(4));
#ifndef NDEBUG
    ASSERT_THROW(v[4] = true, std::out_of_range);
    ASSERT_THROW(v(5) = false, std::out_of_range);
#endif // !NDEBUG
    v.resize(2);
    EXPECT_EQ(2, v.size());
    EXPECT_TRUE(v(1));
    EXPECT_FALSE(v(2));
    v.resize(4);
    EXPECT_TRUE(v(1));
    EXPECT_FALSE(v(2));
    EXPECT_FALSE(v(3));
    EXPECT_FALSE(v(4));
    v.clear();
    EXPECT_FALSE(v.allocated());
    EXPECT_TRUE(v.empty());
    v.resize(4);
    EXPECT_FALSE(v(1));
    EXPECT_FALSE(v(2));
    EXPECT_FALSE(v(3));
    EXPECT_FALSE(v(4));
    v.clear();
    v.resize(4, true);
    EXPECT_TRUE(v(1));
    EXPECT_TRUE(v(2));
    EXPECT_TRUE(v(3));
    EXPECT_TRUE(v(4));
    v.deallocate();
    EXPECT_FALSE(v.allocated());
    EXPECT_TRUE(v.empty());
    v.resize(3, true);
    EXPECT_TRUE(v(1));
    EXPECT_TRUE(v(2));
    EXPECT_TRUE(v(3));
    EXPECT_TRUE(v.allocated());
}
