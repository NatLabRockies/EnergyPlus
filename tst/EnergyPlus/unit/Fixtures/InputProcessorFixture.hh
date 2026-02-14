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

#ifndef InputProcessorFixture_hh_INCLUDED
#define InputProcessorFixture_hh_INCLUDED

// Google test headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/InputProcessing/IdfParser.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>

namespace EnergyPlus {

class InputProcessorFixture : public EnergyPlusFixture
{
protected:
    using json = nlohmann::json;

    //    static void SetUpTestCase()
    //    {
    //        EnergyPlusFixture::SetUpTestCase(); // Sets up the base fixture
    //    }
    static void TearDownTestCase()
    {
    }

    void SetUp() override
    {
        EnergyPlusFixture::SetUp(); // Sets up individual test cases.
    }

    void TearDown() override
    {
        EnergyPlusFixture::TearDown(); // Remember to tear down the base fixture after cleaning up derived fixture!
    }

    //    bool process_idd(std::string const &idd, bool &errors_found)
    //    {
    //        return EnergyPlusFixture::process_idd(idd, errors_found);
    //    }

    bool processErrors(EnergyPlusData &state)
    {
        return state.dataInputProcessing->inputProcessor->processErrors(state);
    }

    std::vector<std::string> const &validationErrors()
    {
        return state->dataInputProcessing->inputProcessor->validationErrors();
    }

    std::vector<std::string> const &validationWarnings()
    {
        return state->dataInputProcessing->inputProcessor->validationWarnings();
    }

    std::string encodeIDF()
    {
        return state->dataInputProcessing->inputProcessor->idf_parser->encode(state->dataInputProcessing->inputProcessor->epJSON,
                                                                              state->dataInputProcessing->inputProcessor->schema());
    }

    json &getEpJSON()
    {
        return state->dataInputProcessing->inputProcessor->epJSON;
    }

    void eat_whitespace(std::string const &idf, size_t &index)
    {
        IdfParser idfParser;
        idfParser.idf_size = idf.size();
        idfParser.eat_whitespace(idf, index);
    }

    void eat_comment(std::string const &idf, size_t &index)
    {
        IdfParser idfParser;
        idfParser.idf_size = idf.size();
        idfParser.eat_comment(idf, index);
    }

    std::string parse_string(std::string const &idf, size_t &index)
    {
        IdfParser idfParser;
        idfParser.idf_size = idf.size();
        return idfParser.parse_string(idf, index);
    }

    json parse_value(std::string const &idf, size_t &index, bool &success)
    {
        IdfParser idfParser;
        idfParser.idf_size = idf.size();
        return idfParser.parse_value(idf, index, success, state->dataInputProcessing->inputProcessor->schema()["properties"]);
    }

    json parse_value(std::string const &idf, size_t &index, bool &success, json const &field_loc)
    {
        IdfParser idfParser;
        idfParser.idf_size = idf.size();
        return idfParser.parse_value(idf, index, success, field_loc);
    }

    json parse_number(std::string const &idf, size_t &index)
    {
        IdfParser idfParser;
        idfParser.idf_size = idf.size();
        return idfParser.parse_number(idf, index);
    }

    IdfParser::Token look_ahead(std::string const &idf, size_t index)
    {
        IdfParser idfParser;
        idfParser.idf_size = idf.size();
        return idfParser.look_ahead(idf, index);
    }

    IdfParser::Token next_token(std::string const &idf, size_t &index)
    {
        IdfParser idfParser;
        idfParser.idf_size = idf.size();
        return idfParser.next_token(idf, index);
    }

    json parse_idf(std::string const &idf, size_t &index, bool &success, json const &schema)
    {
        IdfParser idfParser;
        idfParser.idf_size = idf.size();
        return idfParser.parse_idf(idf, index, success, schema);
    }

    json parse_object(std::string const &idf, size_t &index, bool &success, json const &schema_loc, json const &obj_loc, int idfObjectCount)
    {
        IdfParser idfParser;
        idfParser.idf_size = idf.size();
        return idfParser.parse_object(idf, index, success, schema_loc, obj_loc, idfObjectCount);
    }
};

} // namespace EnergyPlus

#endif
