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

#ifndef IdfParserFixture_hh_INCLUDED
#define IdfParserFixture_hh_INCLUDED

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/DataStringGlobals.hh>

#include "../TestHelpers/CustomMatchers.hh"
#include "../TestHelpers/IdfParser.hh"

#include <ostream>

namespace EnergyPlus {

class IdfParserFixture : public testing::Test
{
protected:
    static void SetUpTestCase()
    {
    }
    static void TearDownTestCase()
    {
    }

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    // This function creates a string based on a vector of string inputs that is delimited by DataStringGlobals::NL by default, but any
    // delimiter can be passed in to this function. This allows for cross platform output string comparisons.
    std::string delimited_string(std::vector<std::string> const &strings, std::string const &delimiter = "\n")
    {
        std::ostringstream compare_text;
        for (std::string const &str : strings) {
            compare_text << str << delimiter;
        }
        return compare_text.str();
    }

    void eat_whitespace(std::string const &idf, size_t &index)
    {
        IdfParser::eat_whitespace(idf, index);
    }

    void eat_comment(std::string const &idf, size_t &index)
    {
        IdfParser::eat_comment(idf, index);
    }

    std::string parse_string(std::string const &idf, size_t &index, bool &success)
    {
        return IdfParser::parse_string(idf, index, success);
    }

    std::string parse_value(std::string const &idf, size_t &index, bool &success)
    {
        return IdfParser::parse_value(idf, index, success);
    }

    IdfParser::Token look_ahead(std::string const &idf, size_t index)
    {
        return IdfParser::look_ahead(idf, index);
    }

    IdfParser::Token next_token(std::string const &idf, size_t &index)
    {
        return IdfParser::next_token(idf, index);
    }

    std::vector<std::vector<std::string>> parse_idf(std::string const &idf, size_t &index, bool &success)
    {
        return IdfParser::parse_idf(idf, index, success);
    }

    std::vector<std::string> parse_object(std::string const &idf, size_t &index, bool &success)
    {
        return IdfParser::parse_object(idf, index, success);
    }
};

} // namespace EnergyPlus

#endif
