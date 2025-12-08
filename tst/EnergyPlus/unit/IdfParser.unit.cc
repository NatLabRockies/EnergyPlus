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

// IdfParser Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/IdfParserFixture.hh"

namespace EnergyPlus {

TEST_F(IdfParserFixture, decode)
{
    std::string const test_object(delimited_string({
        "Version," + DataStringGlobals::MatchVersion + ";",
        "  Building,",
        "    Ref Bldg Medium Office New2004_v1.3_5.0,  !- Name",
        "    0.0000,                  !- North Axis {deg}",
        "    City,                    !- Terrain",
        "    0.0400,                  !- Loads Convergence Tolerance Value",
        "    0.2000,                  !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullInteriorAndExterior, !- Solar Distribution",
        "    25,                      !- Maximum Number of Warmup Days",
        "    6;  ",
    }));

    auto const output = IdfParser::decode(test_object);

    EXPECT_EQ(
        std::vector<std::vector<std::string>>(
            {{"Version", DataStringGlobals::MatchVersion},
             {"Building", "Ref Bldg Medium Office New2004_v1.3_5.0", "0.0000", "City", "0.0400", "0.2000", "FullInteriorAndExterior", "25", "6"}}),
        output);
}

TEST_F(IdfParserFixture, decode_success)
{
    bool success = true;
    std::string const test_object(delimited_string({
        "Version," + DataStringGlobals::MatchVersion + ";",
        "  Building,",
        "    Ref Bldg Medium Office New2004_v1.3_5.0,  !- Name",
        "    0.0000,                  !- North Axis {deg}",
        "    City,                    !- Terrain",
        "    0.0400,                  !- Loads Convergence Tolerance Value",
        "    0.2000,                  !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullInteriorAndExterior, !- Solar Distribution",
        "    25,                      !- Maximum Number of Warmup Days",
        "    6;  ",
    }));

    auto const output = IdfParser::decode(test_object, success);

    EXPECT_EQ(
        std::vector<std::vector<std::string>>(
            {{"Version", DataStringGlobals::MatchVersion},
             {"Building", "Ref Bldg Medium Office New2004_v1.3_5.0", "0.0000", "City", "0.0400", "0.2000", "FullInteriorAndExterior", "25", "6"}}),
        output);
    EXPECT_TRUE(success);
}

TEST_F(IdfParserFixture, decode_success_2)
{
    bool success = true;
    std::string const test_object(delimited_string({
        "Version," + DataStringGlobals::MatchVersion + ";",
        "  Building,",
        "    Ref Bldg Medium Office New2004_v1.3_5.0,  !- Name",
        "    0.0000,                  !- North Axis {deg}",
        "    ,                        !- Terrain",
        "    0.0400,                  !- Loads Convergence Tolerance Value",
        "    0.2000,                  !- Temperature Convergence Tolerance Value {deltaC}",
        "    ,                        !- Solar Distribution",
        "    25,                      !- Maximum Number of Warmup Days",
        "    6;  ",
    }));

    auto const output = IdfParser::decode(test_object, success);

    EXPECT_EQ(std::vector<std::vector<std::string>>(
                  {{"Version", DataStringGlobals::MatchVersion},
                   {"Building", "Ref Bldg Medium Office New2004_v1.3_5.0", "0.0000", "", "0.0400", "0.2000", "", "25", "6"}}),
              output);
    EXPECT_TRUE(success);
}

TEST_F(IdfParserFixture, decode_success_3)
{
    bool success = true;
    std::string const test_object(delimited_string({
        "Version," + DataStringGlobals::MatchVersion + ";",
        "  Building,",
        "    Ref Bldg Medium Office New2004_v1.3_5.0,  !- Name",
        "    0.0000,                  !- North Axis {deg}",
        "    ,                        !- Terrain",
        "    0.0400,                  !- Loads Convergence Tolerance Value",
        "    0.2000,                  !- Temperature Convergence Tolerance Value {deltaC}",
        "    ,                        !- Solar Distribution",
        "    25,                      !- Maximum Number of Warmup Days",
        "    ;  ",
    }));

    auto const output = IdfParser::decode(test_object, success);

    EXPECT_EQ(std::vector<std::vector<std::string>>(
                  {{"Version", DataStringGlobals::MatchVersion},
                   {"Building", "Ref Bldg Medium Office New2004_v1.3_5.0", "0.0000", "", "0.0400", "0.2000", "", "25", ""}}),
              output);
    EXPECT_TRUE(success);
}

TEST_F(IdfParserFixture, decode_success_4)
{
    bool success = true;
    std::string const test_object(delimited_string({
        "Version," + DataStringGlobals::MatchVersion + ";",
        "Schedule:Constant,OnSch,,1.0;",
        "Schedule:Constant,Aula people sched,,0.0;",
    }));

    auto const output = IdfParser::decode(test_object, success);

    EXPECT_EQ(std::vector<std::vector<std::string>>({{"Version", DataStringGlobals::MatchVersion},
                                                     {"Schedule:Constant", "OnSch", "", "1.0"},
                                                     {"Schedule:Constant", "Aula people sched", "", "0.0"}}),
              output);
    EXPECT_TRUE(success);
}

TEST_F(IdfParserFixture, decode_encode)
{
    bool success = true;
    std::string const test_object(delimited_string({
        "Version," + DataStringGlobals::MatchVersion + ";",
        "Schedule:Constant,OnSch,,1.0;",
        "Schedule:Constant,Aula people sched,,0.0;",
    }));

    auto const output = IdfParser::decode(test_object, success);

    EXPECT_EQ(std::vector<std::vector<std::string>>({{"Version", DataStringGlobals::MatchVersion},
                                                     {"Schedule:Constant", "OnSch", "", "1.0"},
                                                     {"Schedule:Constant", "Aula people sched", "", "0.0"}}),
              output);
    EXPECT_TRUE(success);

    auto const encoded_string = IdfParser::encode(output);

    EXPECT_EQ(test_object, encoded_string);
}

TEST_F(IdfParserFixture, decode_encode_2)
{
    bool success = true;
    std::string const test_object(delimited_string({
        "Version," + DataStringGlobals::MatchVersion + ";",
        "Schedule:Constant,OnSch,,;",
        "Schedule:Constant,Aula people sched,,;",
    }));

    auto const output = IdfParser::decode(test_object, success);

    EXPECT_EQ(std::vector<std::vector<std::string>>({{"Version", DataStringGlobals::MatchVersion},
                                                     {"Schedule:Constant", "OnSch", "", ""},
                                                     {"Schedule:Constant", "Aula people sched", "", ""}}),
              output);
    EXPECT_TRUE(success);

    auto const encoded_string = IdfParser::encode(output);

    EXPECT_EQ(test_object, encoded_string);
}

TEST_F(IdfParserFixture, parse_idf)
{
    size_t index = 0;
    bool success = true;
    std::string const test_object(delimited_string({
        "Version," + DataStringGlobals::MatchVersion + ";",
        "  Building,",
        "    Ref Bldg Medium Office New2004_v1.3_5.0,  !- Name",
        "    0.0000,                  !- North Axis {deg}",
        "    City,                    !- Terrain",
        "    0.0400,                  !- Loads Convergence Tolerance Value",
        "    0.2000,                  !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullInteriorAndExterior, !- Solar Distribution",
        "    25,                      !- Maximum Number of Warmup Days",
        "    6;",
    }));

    auto const output = parse_idf(test_object, index, success);

    EXPECT_EQ(
        std::vector<std::vector<std::string>>(
            {{"Version", DataStringGlobals::MatchVersion},
             {"Building", "Ref Bldg Medium Office New2004_v1.3_5.0", "0.0000", "City", "0.0400", "0.2000", "FullInteriorAndExterior", "25", "6"}}),
        output);

    EXPECT_EQ(test_object.size() - 1, index);
    EXPECT_TRUE(success);
}

TEST_F(IdfParserFixture, parse_object)
{
    size_t index = 0;
    bool success = true;
    std::string const test_object(delimited_string({
        "  Building,",
        "    Ref Bldg Medium Office New2004_v1.3_5.0,  !- Name",
        "    0.0000,                  !- North Axis {deg}",
        "    City,                    !- Terrain",
        "    0.0400,                  !- Loads Convergence Tolerance Value",
        "    0.2000,                  !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullInteriorAndExterior, !- Solar Distribution",
        "    25,                      !- Maximum Number of Warmup Days",
        "    6;",
    }));

    auto const output_vector = parse_object(test_object, index, success);
    EXPECT_EQ(
        std::vector<std::string>(
            {"Building", "Ref Bldg Medium Office New2004_v1.3_5.0", "0.0000", "City", "0.0400", "0.2000", "FullInteriorAndExterior", "25", "6"}),
        output_vector);

    EXPECT_EQ(test_object.size() - 1, index);
    EXPECT_TRUE(success);
}

TEST_F(IdfParserFixture, eat_whitespace)
{
    size_t index = 0;
    eat_whitespace("    test", index);
    EXPECT_EQ(4ul, index);

    index = 0;
    eat_whitespace("t   test", index);
    EXPECT_EQ(0ul, index);
}

TEST_F(IdfParserFixture, eat_comment)
{
    size_t index = 0;
    eat_comment("!- North Axis {deg}\n", index);
    EXPECT_EQ(20ul, index);

    index = 0;
    eat_comment("                    !- Terrain\n", index);
    EXPECT_EQ(31ul, index);

    index = 0;
    eat_comment("  !- Name\n    0.0000", index);
    EXPECT_EQ(10ul, index);

    index = 0;
    eat_comment("  !- Name\n\r    0.0000", index);
    EXPECT_EQ(10ul, index);
}

TEST_F(IdfParserFixture, parse_string)
{
    size_t index = 0;
    bool success = true;
    std::string output_string;

    output_string = parse_string("test_string", index, success);
    EXPECT_EQ("test_string", output_string);
    EXPECT_EQ(11ul, index);
    EXPECT_TRUE(success);

    index = 0;
    success = true;
    output_string = parse_string("test string", index, success);
    EXPECT_EQ("test string", output_string);
    EXPECT_EQ(11ul, index);
    EXPECT_TRUE(success);

    index = 0;
    success = true;
    output_string = parse_string("-1234.1234", index, success);
    EXPECT_EQ("-1234.1234", output_string);
    EXPECT_EQ(10ul, index);
    EXPECT_TRUE(success);

    index = 0;
    success = true;
    output_string = parse_string(R"(\b\t/\\\";)", index, success);
    EXPECT_EQ("\b\t/\\\"", output_string);
    EXPECT_EQ(9ul, index);
    EXPECT_TRUE(success);

    index = 0;
    success = true;
    output_string = parse_string(R"(test \n string)", index, success);
    EXPECT_EQ("", output_string);
    EXPECT_EQ(7ul, index);
    EXPECT_FALSE(success);

    index = 0;
    success = true;
    output_string = parse_string(R"(! this is a comment \n)", index, success);
    EXPECT_EQ("", output_string);
    EXPECT_EQ(0ul, index);
    EXPECT_TRUE(success);
}

TEST_F(IdfParserFixture, parse_value)
{
    size_t index = 0;
    bool success = true;
    std::string output_string;

    output_string = parse_value("test_string", index, success);
    EXPECT_EQ("test_string", output_string);
    EXPECT_EQ(11ul, index);
    EXPECT_TRUE(success);

    index = 0;
    success = true;
    output_string = parse_value(", test_string", index, success);
    EXPECT_EQ("", output_string);
    EXPECT_EQ(0ul, index);
    EXPECT_FALSE(success);

    index = 0;
    success = true;
    output_string = parse_value(R"(test \n string)", index, success);
    EXPECT_EQ("", output_string);
    EXPECT_EQ(7ul, index);
    EXPECT_FALSE(success);

    index = 0;
    success = true;
    output_string = parse_value("; test_string", index, success);
    EXPECT_EQ("", output_string);
    EXPECT_EQ(0ul, index);
    EXPECT_FALSE(success);

    index = 0;
    success = true;
    output_string = parse_value("! test_string", index, success);
    EXPECT_EQ("", output_string);
    EXPECT_EQ(0ul, index);
    EXPECT_FALSE(success);
}

TEST_F(IdfParserFixture, look_ahead)
{
    std::string const test_input("B , ! t ; `");
    size_t index = 0;
    IdfParser::Token token = look_ahead(test_input, index);
    EXPECT_EQ(0ul, index);
    EXPECT_ENUM_EQ(IdfParser::Token::STRING, token);
    index = 2;
    token = look_ahead(test_input, index);
    EXPECT_EQ(2ul, index);
    EXPECT_ENUM_EQ(IdfParser::Token::COMMA, token);
    index = 3;
    token = look_ahead(test_input, index);
    EXPECT_EQ(3ul, index);
    EXPECT_ENUM_EQ(IdfParser::Token::EXCLAMATION, token);
    index = 5;
    token = look_ahead(test_input, index);
    EXPECT_EQ(5ul, index);
    EXPECT_ENUM_EQ(IdfParser::Token::STRING, token);
    index = 7;
    token = look_ahead(test_input, index);
    EXPECT_EQ(7ul, index);
    EXPECT_ENUM_EQ(IdfParser::Token::SEMICOLON, token);
    index = 9;
    token = look_ahead(test_input, index);
    EXPECT_EQ(9ul, index);
    EXPECT_ENUM_EQ(IdfParser::Token::NONE, token);
    index = test_input.size();
    token = look_ahead(test_input, index);
    EXPECT_EQ(test_input.size(), index);
    EXPECT_ENUM_EQ(IdfParser::Token::END, token);
}

TEST_F(IdfParserFixture, next_token)
{
    size_t index = 0;

    std::string const test_input("B , ! t ; `");
    IdfParser::Token token = next_token(test_input, index);
    EXPECT_EQ(1ul, index);
    EXPECT_ENUM_EQ(IdfParser::Token::STRING, token);
    token = next_token(test_input, index);
    EXPECT_EQ(3ul, index);
    EXPECT_ENUM_EQ(IdfParser::Token::COMMA, token);
    token = next_token(test_input, index);
    EXPECT_EQ(5ul, index);
    EXPECT_ENUM_EQ(IdfParser::Token::EXCLAMATION, token);
    token = next_token(test_input, index);
    EXPECT_EQ(7ul, index);
    EXPECT_ENUM_EQ(IdfParser::Token::STRING, token);
    token = next_token(test_input, index);
    EXPECT_EQ(9ul, index);
    EXPECT_ENUM_EQ(IdfParser::Token::SEMICOLON, token);
    token = next_token(test_input, index);
    EXPECT_EQ(10ul, index);
    EXPECT_ENUM_EQ(IdfParser::Token::NONE, token);
    index = test_input.size();
    token = next_token(test_input, index);
    EXPECT_EQ(test_input.size(), index);
    EXPECT_ENUM_EQ(IdfParser::Token::END, token);
}

} // namespace EnergyPlus
