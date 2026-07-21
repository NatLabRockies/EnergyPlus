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

// EnergyPlus Headers
#include "EnergyPlusFixture.hh"

// A to Z order
#include <EnergyPlus/Data/CommonIncludes.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/FileSystem.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/InputProcessing/IdfParser.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/InputProcessing/InputValidation.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ReportCoilSelection.hh>
#include <EnergyPlus/SQLiteProcedures.hh>
#include <EnergyPlus/SimulationManager.hh>

#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include <regex>

using json = nlohmann::json;

namespace EnergyPlus {

// void EnergyPlusFixture::SetUpTestCase()
//{
//    state->dataInputProcessing->inputProcessor = InputProcessor::factory();
//}

void EnergyPlusFixture::openOutputFiles(EnergyPlusData &t_state)
{
    t_state.files.eio.open_as_stringstream();
    t_state.files.mtr.open_as_stringstream();
    t_state.files.eso.open_as_stringstream();
    t_state.files.audit.open_as_stringstream();
    t_state.files.bnd.open_as_stringstream();
    t_state.files.debug.open_as_stringstream();
    t_state.files.mtd.open_as_stringstream();
    t_state.files.edd.open_as_stringstream();
    t_state.files.zsz.open_as_stringstream();
    t_state.files.spsz.open_as_stringstream();
    t_state.files.ssz.open_as_stringstream();
}

void EnergyPlusFixture::SetUp()
{
    //    if (this->state) {
    //        this->state->clear_state();
    //    } else {
    this->state = new EnergyPlusData;
    //    }

    show_message();

    openOutputFiles(*state);

    this->err_stream = new std::ostringstream;

    state->files.err_stream = std::unique_ptr<std::ostream>(this->err_stream);

    m_cout_buffer = std::unique_ptr<std::ostringstream>(new std::ostringstream);
    m_redirect_cout = std::make_unique<RedirectCout>(m_cout_buffer);

    m_cerr_buffer = std::unique_ptr<std::ostringstream>(new std::ostringstream);
    m_redirect_cerr = std::make_unique<RedirectCerr>(m_cerr_buffer);

    state->dataUtilityRoutines->outputErrorHeader = false;

    state->init_constant_state(*state);
    state->dataEnvrn->StdRhoAir = 1.2;
}

void EnergyPlusFixture::TearDown()
{
    state->files.mtd.del();
    state->files.eso.del();
    state->files.err_stream.reset();
    state->files.eio.del();
    state->files.debug.del();
    state->files.zsz.del();
    state->files.spsz.del();
    state->files.ssz.del();
    state->files.mtr.del();
    state->files.bnd.del();
    state->files.shade.del();

    state->clear_state();
    delete this->state;
}

void EnergyPlusFixture::show_message()
{
    // Gets information about the currently running test.
    // Do NOT delete the returned object - it's managed by the UnitTest class.
    const ::testing::TestInfo *const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
    ShowMessage(*state, "Begin Test: " + std::string(test_info->test_case_name()) + ", " + std::string(test_info->name()));
}

std::string EnergyPlusFixture::delimited_string(std::vector<std::string> const &strings, std::string const &delimiter)
{
    std::ostringstream compare_text;
    for (auto const &str : strings) {
        compare_text << str << delimiter;
    }
    return compare_text.str();
}

std::vector<std::string> EnergyPlusFixture::read_lines_in_file(fs::path const &filePath)
{
    std::ifstream infile(filePath);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(infile, line)) {
        lines.push_back(line);
    }
    return lines;
}

bool EnergyPlusFixture::compare_eso_stream(std::string const &expected_string, bool reset_stream)
{
    auto const stream_str = state->files.eso.get_output();
    EXPECT_EQ(expected_string, stream_str);
    bool are_equal = (expected_string == stream_str);
    if (reset_stream) {
        state->files.eso.open_as_stringstream();
    }
    return are_equal;
}

bool EnergyPlusFixture::compare_eio_stream(std::string const &expected_string, bool reset_stream)
{
    auto const stream_str = state->files.eio.get_output();
    EXPECT_EQ(expected_string, stream_str);
    bool are_equal = (expected_string == stream_str);
    if (reset_stream) {
        state->files.eio.open_as_stringstream();
    }
    return are_equal;
}

bool EnergyPlusFixture::compare_eio_stream_substring(std::string const &search_string, bool reset_stream)
{
    if (search_string.empty()) {
        ADD_FAILURE() << "compare_eio_stream_substring cannot search for an empty string; use compare_eso_stream or has_eso_output instead.";
        return false;
    }

    auto const stream_str = state->files.eio.get_output();
    bool const found = stream_str.find(search_string) != std::string::npos;
    EXPECT_TRUE(found);
    if (reset_stream) {
        state->files.eio.open_as_stringstream();
    }
    return found;
}

bool EnergyPlusFixture::compare_mtr_stream(std::string const &expected_string, bool reset_stream)
{
    auto const stream_str = state->files.mtr.get_output();
    EXPECT_EQ(expected_string, stream_str);
    bool are_equal = (expected_string == stream_str);
    if (reset_stream) {
        state->files.mtr.open_as_stringstream();
    }
    return are_equal;
}

bool EnergyPlusFixture::compare_err_stream(std::string const &expected_string, bool reset_stream)
{
    auto const stream_str = this->err_stream->str();
    EXPECT_EQ(expected_string, stream_str);
    bool are_equal = (expected_string == stream_str);
    if (reset_stream) {
        this->err_stream->str(std::string());
    }
    return are_equal;
}

bool EnergyPlusFixture::compare_err_stream_substring(std::string const &search_string, bool reset_stream, bool call_expect)
{
    if (search_string.empty()) {
        ADD_FAILURE() << "compare_err_stream_substring cannot search for an empty string; use compare_err_stream or has_err_output instead.";
        return false;
    }

    auto const stream_str = this->err_stream->str();
    bool const found = stream_str.find(search_string) != std::string::npos;
    if (call_expect) {
        EXPECT_TRUE(found) << "Not found in:" << "\n" << stream_str;
    }
    if (reset_stream) {
        this->err_stream->str(std::string());
    }
    return found;
}

bool EnergyPlusFixture::compare_cout_stream(std::string const &expected_string, bool reset_stream)
{
    auto const stream_str = this->m_cout_buffer->str();
    EXPECT_EQ(expected_string, stream_str);
    bool are_equal = (expected_string == stream_str);
    if (reset_stream) {
        this->m_cout_buffer->str(std::string());
    }
    return are_equal;
}
bool EnergyPlusFixture::compare_cout_stream_substring(std::string const &search_string, bool reset_stream)
{
    if (search_string.empty()) {
        ADD_FAILURE() << "compare_cout_stream_substring cannot search for an empty string; use compare_cout_stream or has_cout_output instead.";
        return false;
    }

    auto const stream_str = this->m_cout_buffer->str();
    bool const found = stream_str.find(search_string) != std::string::npos;
    if (reset_stream) {
        this->m_cout_buffer->str(std::string());
    }
    return found;
}

bool EnergyPlusFixture::compare_cerr_stream(std::string const &expected_string, bool reset_stream)
{
    auto const stream_str = this->m_cerr_buffer->str();
    EXPECT_EQ(expected_string, stream_str);
    bool are_equal = (expected_string == stream_str);
    if (reset_stream) {
        this->m_cerr_buffer->str(std::string());
    }
    return are_equal;
}

bool EnergyPlusFixture::compare_dfs_stream(std::string const &expected_string, bool reset_stream)
{
    auto const stream_str = state->files.dfs.get_output();
    EXPECT_EQ(expected_string, stream_str);
    bool are_equal = (expected_string == stream_str);
    if (reset_stream) {
        state->files.dfs.open_as_stringstream();
    }
    return are_equal;
}

bool EnergyPlusFixture::has_eso_output(bool reset_stream)
{
    bool const has_output = !state->files.eso.get_output().empty();
    if (reset_stream) {
        state->files.eso.open_as_stringstream();
    }
    return has_output;
}

bool EnergyPlusFixture::has_eio_output(bool reset_stream)
{
    bool const has_output = !state->files.eio.get_output().empty();
    if (reset_stream) {
        state->files.eio.open_as_stringstream();
    }
    return has_output;
}

bool EnergyPlusFixture::has_mtr_output(bool reset_stream)
{
    bool const has_output = !state->files.mtr.get_output().empty();
    if (reset_stream) {
        state->files.mtr.open_as_stringstream();
    }
    return has_output;
}

bool EnergyPlusFixture::has_err_output(bool reset_stream)
{
    bool const has_output = !this->err_stream->str().empty();
    if (reset_stream) {
        this->err_stream->str(std::string());
    }
    return has_output;
}

bool EnergyPlusFixture::has_cout_output(bool reset_stream)
{
    bool const has_output = !this->m_cout_buffer->str().empty();
    if (reset_stream) {
        this->m_cout_buffer->str(std::string());
    }
    return has_output;
}

bool EnergyPlusFixture::has_cerr_output(bool reset_stream)
{
    bool const has_output = !this->m_cerr_buffer->str().empty();
    if (reset_stream) {
        this->m_cerr_buffer->str(std::string());
    }
    return has_output;
}

bool EnergyPlusFixture::has_dfs_output(bool reset_stream)
{
    bool const has_output = !state->files.dfs.get_output().empty();
    if (reset_stream) {
        state->files.dfs.open_as_stringstream();
    }
    return has_output;
}

bool EnergyPlusFixture::match_err_stream(std::string const &expected_match, bool use_regex, bool reset_stream)
{
    auto const stream_str = this->err_stream->str();
    bool match_found;
    if (use_regex) {
        match_found = std::regex_match(stream_str, std::regex(expected_match));
    } else {
        match_found = stream_str.find(expected_match) != std::string::npos;
    }
    if (reset_stream) {
        this->err_stream->str(std::string());
    }
    return match_found;
}

bool EnergyPlusFixture::process_idf(std::string_view const idf_snippet, bool use_assertions)
{
    auto &inputProcessor = state->dataInputProcessing->inputProcessor;

    state->dataGlobal->isEpJSON = false;
    bool success = true;
    inputProcessor->epJSON = inputProcessor->idf_parser->decode(idf_snippet, inputProcessor->schema(), success);
    bool const successful_processing = success && common_process_json(false);
    if (!successful_processing && use_assertions) {
        EXPECT_TRUE(compare_err_stream(""));
    }
    return successful_processing;
}

bool EnergyPlusFixture::process_json(nlohmann::json const &epJSON, bool use_assertions)
{
    auto &inputProcessor = state->dataInputProcessing->inputProcessor;

    state->dataGlobal->isEpJSON = true;
    inputProcessor->epJSON = epJSON;
    return common_process_json(use_assertions);
}

bool EnergyPlusFixture::common_process_json(bool use_assertions)
{
    auto &inputProcessor = state->dataInputProcessing->inputProcessor;

    // Add common objects that will trigger a warning if not present
    if (inputProcessor->epJSON.find("Timestep") == inputProcessor->epJSON.end()) {
        inputProcessor->epJSON["Timestep"] = {{"", {{"idf_order", 0}, {"number_of_timesteps_per_hour", 4}}}};
    }
    if (inputProcessor->epJSON.find("Version") == inputProcessor->epJSON.end()) {
        inputProcessor->epJSON["Version"] = {{"", {{"idf_order", 0}, {"version_identifier", DataStringGlobals::MatchVersion}}}};
    }
    if (inputProcessor->epJSON.find("Building") == inputProcessor->epJSON.end()) {
        inputProcessor->epJSON["Building"] = {{"Bldg",
                                               {{"idf_order", 0},
                                                {"north_axis", 0.0},
                                                {"terrain", "Suburbs"},
                                                {"loads_convergence_tolerance_value", 0.04},
                                                {"temperature_convergence_tolerance_value", 0.4000},
                                                {"solar_distribution", "FullExterior"},
                                                {"maximum_number_of_warmup_days", 25},
                                                {"minimum_number_of_warmup_days", 6}}}};
    }
    if (inputProcessor->epJSON.find("GlobalGeometryRules") == inputProcessor->epJSON.end()) {
        inputProcessor->epJSON["GlobalGeometryRules"] = {{"",
                                                          {{"idf_order", 0},
                                                           {"starting_vertex_position", "UpperLeftCorner"},
                                                           {"vertex_entry_direction", "Counterclockwise"},
                                                           {"coordinate_system", "Relative"},
                                                           {"daylighting_reference_point_coordinate_system", "Relative"},
                                                           {"rectangular_surface_coordinate_system", "Relative"}}}};
    }

    int MaxArgs = 0;
    int MaxAlpha = 0;
    int MaxNumeric = 0;
    inputProcessor->getMaxSchemaArgs(MaxArgs, MaxAlpha, MaxNumeric);

    state->dataIPShortCut->cAlphaFieldNames.allocate(MaxAlpha);
    state->dataIPShortCut->cAlphaArgs.allocate(MaxAlpha);
    state->dataIPShortCut->lAlphaFieldBlanks.dimension(MaxAlpha, false);
    state->dataIPShortCut->cNumericFieldNames.allocate(MaxNumeric);
    state->dataIPShortCut->rNumericArgs.dimension(MaxNumeric, 0.0);
    state->dataIPShortCut->lNumericFieldBlanks.dimension(MaxNumeric, false);

    bool is_valid = inputProcessor->validation->validate(inputProcessor->epJSON);
    bool hasErrors = inputProcessor->processErrors(*state);

    inputProcessor->initializeMaps();
    SimulationManager::PostIPProcessing(*state);

    // Can't do this here because many tests set TimeStepsInHour and
    // other global settings manually, and init_state() has to be
    // called after those.

    // state->init_state(*state);

    if (state->dataSQLiteProcedures->sqlite) {
        bool writeOutputToSQLite = false;
        bool writeTabularDataToSQLite = false;
        ParseSQLiteInput(*state, writeOutputToSQLite, writeTabularDataToSQLite);
    }

    // inputProcessor->state->printErrors();

    bool successful_processing = is_valid && !hasErrors;

    if (!successful_processing && use_assertions) {
        EXPECT_TRUE(compare_err_stream(""));
    }

    return successful_processing;
}

} // namespace EnergyPlus
