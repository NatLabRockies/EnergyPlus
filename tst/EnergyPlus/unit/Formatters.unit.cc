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
#include <array>
#include <format>
#include <set>
#include <string>
#include <string_view>
#include <vector>

// Third Party Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/EPVector.hh>
#include <EnergyPlus/FileSystem.hh>
#include <EnergyPlus/Formatters.hh>

using namespace EnergyPlus;

TEST(Formatters, Formatters_Path)
{
    EXPECT_EQ("foo/bar/baz", std::format("{}", fs::path("foo/bar/baz")));
    EXPECT_EQ("foo\\bar\\baz", std::format("{}", fs::path("foo\\bar\\baz")));
    EXPECT_EQ("foo/bar/baz", std::format("{:g}", fs::path("foo/bar/baz")));
#ifdef _WIN32
    EXPECT_EQ("foo/bar/baz", std::format("{:g}", fs::path("foo\\bar\\baz")));
#endif
}

TEST(Formatters, Formatters_Ranges_Numeric)
{

    {
        std::vector<int> vec{1, 2, 3};
        EXPECT_EQ("[1, 2, 3]", std::format("{}", vec));
    }

    {
        std::vector<double> vec{1.1, 2.2, 3.3};
        EXPECT_EQ("[1.1, 2.2, 3.3]", std::format("{}", vec));
        EXPECT_EQ("[1.10, 2.20, 3.30]", std::format("{::.2f}", vec));
        EXPECT_EQ("[1.100, 2.200, 3.300]", std::format("{::.3f}", vec));
    }

    {
        std::array<double, 3> vec{1.1, 2.2, 3.3};
        EXPECT_EQ("[1.1, 2.2, 3.3]", std::format("{}", vec));
        EXPECT_EQ("[1.10, 2.20, 3.30]", std::format("{::.2f}", vec));
        EXPECT_EQ("[1.100, 2.200, 3.300]", std::format("{::.3f}", vec));
    }

    {
        EnergyPlus::EPVector<double> vec;
        vec.allocate(3);
        vec(1) = 1.1;
        vec(2) = 2.2;
        vec(3) = 3.3;
        EXPECT_EQ("[1.1, 2.2, 3.3]", std::format("{}", vec));
        EXPECT_EQ("[1.10, 2.20, 3.30]", std::format("{::.2f}", vec));
        EXPECT_EQ("[1.100, 2.200, 3.300]", std::format("{::.3f}", vec));
    }

    {
        ObjexxFCL::Array1D<double> vec(3);
        vec(1) = 1.1;
        vec(2) = 2.2;
        vec(3) = 3.3;
        EXPECT_EQ("[1.1, 2.2, 3.3]", std::format("{}", vec));
        EXPECT_EQ("[1.10, 2.20, 3.30]", std::format("{::.2f}", vec));
        EXPECT_EQ("[1.100, 2.200, 3.300]", std::format("{::.3f}", vec));
    }

    {
        std::set<int> s{1, 2, 3};
        EXPECT_EQ("{1, 2, 3}", std::format("{}", s));
    }
}

TEST(Formatters, Formatters_Ranges_Chars)
{

    {
        std::string str = "element";
        EXPECT_EQ("element", std::format("{}", str));
    }

    {
        std::string_view str = "element";
        EXPECT_EQ("element", std::format("{}", str));
    }

    {
        const char *str = "element";
        EXPECT_EQ("element", std::format("{}", str));
    }

    {
        std::vector<std::string> vec{"a", "b", "c"};
        EXPECT_EQ(R"(["a", "b", "c"])", std::format("{}", vec));
    }

    {
        std::vector<std::string_view> vec{"a", "b", "c"};
        EXPECT_EQ(R"(["a", "b", "c"])", std::format("{}", vec));
    }

    {
        std::vector<const char *> vec{"a", "b", "c"};
        EXPECT_EQ("[a, b, c]", std::format("{}", vec));
    }
}

TEST(Formatters, Formatters_Ranges_Nested)
{
    {
        std::vector<std::vector<int>> vec{{1, 2}, {3, 4}, {5, 6, 7}};
        EXPECT_EQ(R"([[1, 2], [3, 4], [5, 6, 7]])", std::format("{}", vec));
    }
}

TEST(Formatters, Formatters_join)
{
    {
        std::vector<double> CondTempArray{0.0000, 0.1000, 0.2000, 0.3000, 0.4000, 0.5000, 0.6000, 0.7000, 0.8000, 0.9000, 1.0000};
        EXPECT_EQ("PLR           =    0.00   0.10   0.20   0.30   0.40   0.50   0.60   0.70   0.80   0.90   1.00",
                  std::format("PLR           = {:7.2F}", EnergyPlus::join(CondTempArray, "")));
    }
}
