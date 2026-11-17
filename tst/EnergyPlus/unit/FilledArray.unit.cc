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

// C++ Headers
#include <algorithm>
#include <array>
#include <string>
#include <type_traits>

// EnergyPlus Headers
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/FilledArray.hh>

using namespace EnergyPlus;

namespace {

enum class Color
{
    Invalid = -1,
    Red,
    Green,
    Blue,
    Num
};

struct Holder
{
    std::array<Real64, 11> member = filled_array{296.15};
    std::array<int, (int)Color::Num> enumSized = filled_array{-1};
    std::array<Color, 4> enums = filled_array{Color::Blue};
    std::array<Real64, 12> explicitType = filled_array<Real64>{13};
    std::array<std::string, 2> strings = filled_array{std::string("abc")};
    static constexpr std::array<Real64, 5> staticMember = filled_array{45.0};
    static constexpr std::array<int, 3> staticIntMember = filled_array{-1};
};

template <typename T, std::size_t N> constexpr bool allEqual(std::array<T, N> const &a, T const &v)
{
    return std::ranges::all_of(a, [&v](T const &x) { return x == v; });
}

// Compile-time checks
static_assert(allEqual(Holder::staticMember, 45.0));
static_assert(allEqual(Holder::staticIntMember, -1));
// The value type must match the element type exactly: no implicit conversion
static_assert(std::is_convertible_v<filled_array<Real64>, std::array<Real64, 3>>);
static_assert(!std::is_convertible_v<filled_array<int>, std::array<Real64, 3>>);
static_assert(!std::is_convertible_v<filled_array<Real64>, std::array<int, 3>>);

} // namespace

TEST(FilledArray, AggregateInitPitfall)
{
    // This is the reason filled_array exists: aggregate init only sets the first element
    std::array<Real64, 3> a = {1.0};
    EXPECT_EQ(1.0, a[0]);
    EXPECT_EQ(0.0, a[1]);
    EXPECT_EQ(0.0, a[2]);

    std::array<Real64, 3> b = filled_array{1.0};
    EXPECT_EQ(1.0, b[0]);
    EXPECT_EQ(1.0, b[1]);
    EXPECT_EQ(1.0, b[2]);
}

TEST(FilledArray, NonStaticMembers)
{
    Holder const h; // default-initialized: the default member initializers still run

    for (std::size_t i = 0; i < h.member.size(); ++i) {
        EXPECT_EQ(296.15, h.member[i]) << "i=" << i;
    }
    EXPECT_EQ((std::size_t)Color::Num, h.enumSized.size());
    for (std::size_t i = 0; i < h.enumSized.size(); ++i) {
        EXPECT_EQ(-1, h.enumSized[i]) << "i=" << i;
    }
    for (std::size_t i = 0; i < h.enums.size(); ++i) {
        EXPECT_EQ(Color::Blue, h.enums[i]) << "i=" << i;
    }
    for (std::size_t i = 0; i < h.explicitType.size(); ++i) {
        EXPECT_EQ(13.0, h.explicitType[i]) << "i=" << i;
    }
    for (std::size_t i = 0; i < h.strings.size(); ++i) {
        EXPECT_EQ("abc", h.strings[i]) << "i=" << i;
    }

    auto const *heap = new Holder; // default-initialized on the heap
    EXPECT_TRUE(allEqual(heap->member, 296.15));
    EXPECT_TRUE(allEqual(heap->enumSized, -1));
    delete heap;
}

TEST(FilledArray, Locals)
{
    std::array<Real64, 4> a = filled_array{1.5};
    EXPECT_TRUE(allEqual(a, 1.5));

    constexpr std::array<int, 3> c = filled_array{7};
    static_assert(allEqual(c, 7));

    static constexpr std::array<int, 3> s = filled_array{-2};
    static_assert(allEqual(s, -2));

    Real64 const runtimeValue = a[0] * 2.0;
    std::array<Real64, 6> r = filled_array{runtimeValue};
    EXPECT_TRUE(allEqual(r, 3.0));

    std::array<Real64, 0> empty = filled_array{1.0};
    EXPECT_TRUE(empty.empty());
}
