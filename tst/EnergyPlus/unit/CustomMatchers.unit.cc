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

// EnergyPlus::BoilerSteam Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/DataGlobalConstants.hh>

#include "TestHelpers/CustomMatchers.hh"

using namespace EnergyPlus;

// TEST(CustomMatchers, enums_enforce_type)
// {
//     // Compile time error
//     // third_party/gtest/googletest/include/gtest/gtest.h:2317:3: error: static assertion failed due to requirement
//     // 'std::is_same<EnergyPlus::Constant::eResource, EnergyPlus::Constant::eFuel>::value': T1 and T2 are not the same type
//     EXPECT_ENUM_EQ(Constant::eResource::NaturalGas, Constant::eFuel::NaturalGas);
// }

TEST(CustomMatchers, enums_ok)
{
    EXPECT_ENUM_EQ(Constant::eResource::NaturalGas, Constant::eResource::NaturalGas);
    ASSERT_ENUM_EQ(Constant::eResource::NaturalGas, Constant::eResource::NaturalGas);

    EXPECT_ENUM_NE(Constant::eResource::Electricity, Constant::eResource::NaturalGas);
    ASSERT_ENUM_NE(Constant::eResource::Electricity, Constant::eResource::NaturalGas);
}

// TEST(CustomMatchers, enums_not_ok)
// {
//     EXPECT_ENUM_EQ(Constant::eResource::Electricity, Constant::eResource::NaturalGas);
//     EXPECT_ENUM_NE(Constant::eResource::NaturalGas, Constant::eResource::NaturalGas);
// }
//
// TEST(CustomMatchers, enums_not_ok_assert_eq)
// {
//     ASSERT_ENUM_EQ(Constant::eResource::Electricity, Constant::eResource::NaturalGas);
// }
//
// TEST(CustomMatchers, enums_not_ok_assert_ne)
// {
//     ASSERT_ENUM_NE(Constant::eResource::NaturalGas, Constant::eResource::NaturalGas);
// }
