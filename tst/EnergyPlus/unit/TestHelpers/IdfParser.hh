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

#ifndef IdfParser_hh_INCLUDED
#define IdfParser_hh_INCLUDED

#include <string>
#include <vector>

namespace EnergyPlus {

// Recursive descent parser for IDF format. This using a vector< vector< string > > approach to store objects. Thus each object is stored
// in the outer vector and a vector of each field is stored in the inner vector. This is a naive data structure for now since it works for
// the original purpose. Could use a map, unordered_map, a new struct, or the IDFRecords data structure in the future.
// This should not be used outside of the EnergyPlusFixture.
// This parser was created to facilitate searching an IDF snippet before it is run through the E+ InputProcessor.
class IdfParser
{
public:
    static std::vector<std::vector<std::string>> decode(std::string const &idf);

    static std::vector<std::vector<std::string>> decode(std::string const &idf, bool &success);

    static std::string encode(std::vector<std::vector<std::string>> const &idf_list);

    enum class Token : size_t
    {
        NONE,
        END,
        EXCLAMATION,
        COMMA,
        SEMICOLON,
        STRING,
        Num
    };

private:
    friend class IdfParserFixture;

    static std::vector<std::vector<std::string>> parse_idf(std::string const &idf, size_t &index, bool &success);

    static std::vector<std::string> parse_object(std::string const &idf, size_t &index, bool &success);

    static std::string parse_value(std::string const &idf, size_t &index, bool &success);

    static std::string parse_string(std::string const &idf, size_t &index, bool &success);

    static void eat_whitespace(std::string const &idf, size_t &index);

    static void eat_comment(std::string const &idf, size_t &index);

    static Token look_ahead(std::string const &idf, size_t index);

    static Token next_token(std::string const &idf, size_t &index);
};

} // namespace EnergyPlus

#endif
