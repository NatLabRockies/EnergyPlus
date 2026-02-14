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

// EnergyPlus::InputProcessor Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/InputProcessorFixture.hh"
#include <EnergyPlus/InputProcessing/CsvParser.hh>

#include <string>
#include <vector>

namespace EnergyPlus {

std::string format_errors_or_warnings(std::vector<std::pair<std::string, bool>> const &errors, bool is_error = true)
{
    std::string base = is_error ? "** Severe  **" : "** Warning **";

    std::string errs;
    for (auto const &[error, isContinued] : errors) {
        errs += fmt::format("{}{}\n", isContinued ? "**   ~~~   **" : base, error);
    }
    return errs;
}

TEST_F(InputProcessorFixture, CsvParser_ProperlyFormed)
{

    constexpr std::string_view csv = R"(Hour,Value1,Value2
0,0.1,0.01
1,0.2,0.02
)";

    CsvParser csvParser;
    nlohmann::json result = csvParser.decode(csv, ',', 1);

    EXPECT_FALSE(csvParser.hasErrors()) << format_errors_or_warnings(csvParser.errors());
    EXPECT_TRUE(csvParser.errors().empty());

    EXPECT_FALSE(csvParser.hasWarnings()) << format_errors_or_warnings(csvParser.warnings(), false);
    EXPECT_TRUE(csvParser.warnings().empty());

    auto const &header = result["header"];
    EXPECT_EQ("Hour", header[0]);
    EXPECT_EQ("Value1", header[1]);
    EXPECT_EQ("Value2", header[2]);

    auto const &values = result["values"];
    // Three columns found while parsing the first line
    EXPECT_EQ(3, values.size());

    {
        auto const &col = values[0];
        ASSERT_EQ(2, col.size());
        EXPECT_EQ(0.0, col[0]);
        EXPECT_EQ(1.0, col[1]);
    }
    {
        auto const &col = values[1];
        ASSERT_EQ(2, col.size());
        EXPECT_EQ(0.1, col[0]);
        EXPECT_EQ(0.2, col[1]);
    }
    {
        auto const &col = values[2];
        ASSERT_EQ(2, col.size());
        EXPECT_EQ(0.01, col[0]);
        EXPECT_EQ(0.02, col[1]);
    }

    EXPECT_NO_THROW(values.at(0).get<std::vector<Real64>>());
    EXPECT_NO_THROW(values.at(1).get<std::vector<Real64>>());
    EXPECT_NO_THROW(values.at(2).get<std::vector<Real64>>());
}

TEST_F(InputProcessorFixture, CsvParser_WrongNumberOfValues)
{

    constexpr std::string_view csv = R"(Hour,Value1,Value2
0,0.1,0.01
1,0.02
)";

    CsvParser csvParser;
    nlohmann::json result = csvParser.decode(csv, ',', 1);

    EXPECT_TRUE(csvParser.hasErrors());
    EXPECT_EQ(2, csvParser.errors().size());
    {
        auto const &[error, isContinued] = csvParser.errors().front();
        EXPECT_EQ("CsvParser - Line 3 - Expected 3 columns, got 2. Error in following line.", error);
        EXPECT_FALSE(isContinued);
    }
    {
        auto const &[error, isContinued] = csvParser.errors().back();
        EXPECT_EQ("1,0.02", error);
        EXPECT_TRUE(isContinued);
    }

    auto const &header = result["header"];
    EXPECT_EQ("Hour", header[0]);
    EXPECT_EQ("Value1", header[1]);
    EXPECT_EQ("Value2", header[2]);

    auto const &values = result["values"];
    // Three columns found while parsing the first line
    EXPECT_EQ(3, values.size());
}

TEST_F(InputProcessorFixture, CsvParser_NullValue)
{

    constexpr std::string_view csv = R"(Hour,Value1,Value2
0,0.1,0.01
1,,0.02
)";

    CsvParser csvParser;
    nlohmann::json result = csvParser.decode(csv, ',', 1);

    EXPECT_FALSE(csvParser.hasErrors()) << format_errors_or_warnings(csvParser.errors());
    EXPECT_TRUE(csvParser.errors().empty());

    EXPECT_TRUE(csvParser.hasWarnings());
    EXPECT_EQ(2, csvParser.warnings().size());
    {
        auto const &[warning, isContinued] = csvParser.warnings().front();
        EXPECT_EQ("CsvParser - Line 3 Column 2 - Blank value found, setting to null. Error in following line.", warning);
        EXPECT_FALSE(isContinued);
    }
    {
        auto const &[warning, isContinued] = csvParser.warnings().back();
        EXPECT_EQ("1,,0.02", warning);
        EXPECT_TRUE(isContinued);
    }

    auto const &header = result["header"];
    EXPECT_EQ("Hour", header[0]);
    EXPECT_EQ("Value1", header[1]);
    EXPECT_EQ("Value2", header[2]);

    auto const &values = result["values"];
    // Three columns found while parsing the first line
    EXPECT_EQ(3, values.size());

    {
        auto const &col = values[0];
        ASSERT_EQ(2, col.size());
        EXPECT_EQ(0.0, col[0]);
        EXPECT_EQ(1.0, col[1]);
    }
    {
        auto const &col = values[1];
        ASSERT_EQ(2, col.size());
        EXPECT_EQ(0.1, col[0]);
        EXPECT_EQ(json::value_t::null, col[1]);
    }
    {
        auto const &col = values[2];
        ASSERT_EQ(2, col.size());
        EXPECT_EQ(0.01, col[0]);
        EXPECT_EQ(0.02, col[1]);
    }

    EXPECT_NO_THROW(values.at(0).get<std::vector<Real64>>());
    EXPECT_THROW(values.at(1).get<std::vector<Real64>>(), nlohmann::json::type_error);
    EXPECT_NO_THROW(values.at(2).get<std::vector<Real64>>());
}

TEST_F(InputProcessorFixture, CsvParser_ExtraColumns)
{

    constexpr std::string_view csv = R"(Hour,Value1,Value2
0,0.1,0.01
1,0.2,0.02,0.33
)";

    CsvParser csvParser;
    nlohmann::json result = csvParser.decode(csv, ',', 1);

    EXPECT_FALSE(csvParser.hasErrors()) << format_errors_or_warnings(csvParser.errors());
    EXPECT_TRUE(csvParser.errors().empty());

    EXPECT_TRUE(csvParser.hasWarnings());
    EXPECT_EQ(2, csvParser.warnings().size());
    {
        auto const &[warning, isContinued] = csvParser.warnings().front();
        EXPECT_EQ("CsvParser - Line 3 - Expected 3 columns, got 4. Ignored extra columns. Error in following line.", warning);
        EXPECT_FALSE(isContinued);
    }
    {
        auto const &[warning, isContinued] = csvParser.warnings().back();
        EXPECT_EQ("1,0.2,0.02,0.33", warning);
        EXPECT_TRUE(isContinued);
    }

    auto const &header = result["header"];
    EXPECT_EQ("Hour", header[0]);
    EXPECT_EQ("Value1", header[1]);
    EXPECT_EQ("Value2", header[2]);

    auto const &values = result["values"];
    // Three columns found while parsing the first line
    EXPECT_EQ(3, values.size());

    {
        auto const &col = values[0];
        EXPECT_EQ(2, col.size());
        EXPECT_EQ(0.0, col[0]);
        EXPECT_EQ(1.0, col[1]);
    }
    {
        auto const &col = values[1];
        EXPECT_EQ(2, col.size());
        EXPECT_EQ(0.1, col[0]);
        EXPECT_EQ(0.2, col[1]);
    }
    {
        auto const &col = values[2];
        EXPECT_EQ(2, col.size());
        EXPECT_EQ(0.01, col[0]);
        EXPECT_EQ(0.02, col[1]);
    }

    EXPECT_NO_THROW(values.at(0).get<std::vector<Real64>>());
    EXPECT_NO_THROW(values.at(1).get<std::vector<Real64>>());
    EXPECT_NO_THROW(values.at(2).get<std::vector<Real64>>());
}

} // namespace EnergyPlus
