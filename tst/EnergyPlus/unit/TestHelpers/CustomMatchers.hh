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

#ifndef EnergyPlusFixture_CustomMatchers_hh_INCLUDED
#define EnergyPlusFixture_CustomMatchers_hh_INCLUDED

// Google Test Headers
#include <gtest/gtest.h>

#include <type_traits>

class EnumHelper
{
public:
    template <typename T1, typename T2, typename = typename std::enable_if_t<std::is_enum_v<T1>, T1>>
    static ::testing::AssertionResult Compare(const char *lhs_expression, const char *rhs_expression, const T1 &lhs, const T2 &rhs)
    {
        // Clarify compiler error when types differ
        ::testing::StaticAssertTypeEq<T1, T2>();

        auto underlying_lhs = static_cast<typename std::underlying_type_t<T1>>(lhs);
        auto underlying_rhs = static_cast<typename std::underlying_type_t<T1>>(rhs);

        if (underlying_lhs == underlying_rhs) {
            return ::testing::AssertionSuccess();
        }

        return ::testing::AssertionFailure() << "In comparing enums of type '" << ::testing::internal::GetTypeName<T1>()
                                             << "', Expected equality of these values:" << "\n  " << lhs_expression << "\n    Which is: "
                                             << ::testing::internal::FormatForComparisonFailureMessage(underlying_lhs, underlying_rhs) << "\n  "
                                             << rhs_expression << "\n    Which is: "
                                             << ::testing::internal::FormatForComparisonFailureMessage(underlying_rhs, underlying_lhs);
    }

    template <typename T1, typename T2, typename = typename std::enable_if_t<std::is_enum_v<T1>, T1>>
    static ::testing::AssertionResult CompareNE(const char *lhs_expression, const char *rhs_expression, const T1 &lhs, const T2 &rhs)
    {
        // Clarify compiler error when types differ
        ::testing::StaticAssertTypeEq<T1, T2>();

        auto underlying_lhs = static_cast<typename std::underlying_type_t<T1>>(lhs);
        auto underlying_rhs = static_cast<typename std::underlying_type_t<T1>>(rhs);

        if (underlying_lhs != underlying_rhs) {
            return ::testing::AssertionSuccess();
        }

        return ::testing::AssertionFailure() << "Expected: (" << lhs_expression << ") != (" << rhs_expression << "), actual: both are "
                                             << ::testing::internal::GetTypeName<T1>() << "("
                                             << ::testing::internal::FormatForComparisonFailureMessage(underlying_lhs, underlying_rhs) << ")";
    }
};

#define EXPECT_ENUM_EQ(val1, val2) EXPECT_PRED_FORMAT2(EnumHelper::Compare, val1, val2)

#define EXPECT_ENUM_NE(val1, val2) EXPECT_PRED_FORMAT2(EnumHelper::CompareNE, val1, val2)

#define ASSERT_ENUM_EQ(val1, val2) ASSERT_PRED_FORMAT2(EnumHelper::Compare, val1, val2)

#define ASSERT_ENUM_NE(val1, val2) ASSERT_PRED_FORMAT2(EnumHelper::CompareNE, val1, val2)

#endif // EnergyPlusFixture_CustomMatchers_hh_INCLUDED
