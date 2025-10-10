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

#include <EnergyPlus/IOFiles.hh>

#include "Data/EnergyPlusData.hh"
#include "DataStringGlobals.hh"
#include "FileSystem.hh"
#include "InputProcessing/InputProcessor.hh"
#include "ResultsFramework.hh"
#include "UtilityRoutines.hh"
#include <embedded/EmbeddedEpJSONSchema.hh>

#include <algorithm>
#include <fmt/format.h>
#include <memory>
#include <stdexcept>

namespace EnergyPlus {

InputFile &InputFile::ensure_open(EnergyPlusData &state, const std::string &caller, bool output_to_file)
{
    if (!good()) {
        open(false, output_to_file);
    }
    if (!good()) {
        ShowFatalError(state, fmt::format("{}: Could not open file {} for input (read).", caller, filePath));
    }
    return *this;
}

bool InputFile::good() const noexcept
{
    if (is) {
        return is->good();
    } else {
        return false;
    }
}

void InputFile::close()
{
    is.reset();
}

InputFile::ReadResult<std::string> InputFile::readLine() noexcept
{
    if (!is) {
        return {"", true, false};
    }

    std::string line;
    if (std::getline(*is, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        // Use operator bool, see ReadResult::good() docstring
        return {std::move(line), is->eof(), bool(is)};
    } else {
        return {"", is->eof(), false};
    }
}

std::string InputFile::readFile()
{
    std::string result(file_size, '\0');
    is->read(result.data(), file_size);
    return result;
}

nlohmann::json InputFile::readJSON()
{
    FileSystem::FileTypes const ext = FileSystem::getFileType(filePath);
    switch (ext) {
    case FileSystem::FileTypes::EpJSON:
    case FileSystem::FileTypes::JSON:
    case FileSystem::FileTypes::GLHE:
        return nlohmann::json::parse(*is, nullptr, true, true);
    case FileSystem::FileTypes::CBOR:
        return nlohmann::json::from_cbor(*is);
    case FileSystem::FileTypes::MsgPack:
        return nlohmann::json::from_msgpack(*is);
    case FileSystem::FileTypes::UBJSON:
        return nlohmann::json::from_ubjson(*is);
    case FileSystem::FileTypes::BSON:
        return nlohmann::json::from_bson(*is);
    default:
        throw FatalError("Invalid file extension. Must be epJSON, JSON, or other experimental extensions");
    }
}

InputFile::InputFile(fs::path FilePath) : filePath(std::move(FilePath))
{
}

std::ostream::pos_type InputFile::position() const noexcept
{
    return is->tellg();
}

void InputFile::open(bool, bool)
{
    file_size = fs::file_size(filePath);
    // basic_fstream is a template, it has no problem with wchar_t (which filePath.c_str() returns on Windows)
    is = std::make_unique<std::fstream>(filePath.c_str(), std::ios_base::in | std::ios_base::binary);
    // is->imbue(std::locale("C"));
}

std::string InputFile::error_state_to_string() const
{
    const std::istream::iostate state = rdstate();

    if (!is_open()) {
        return "file not opened'";
    }

    if (state == std::ios_base::failbit) {
        return "io operation failed";
    } else if (state == std::ios_base::badbit) {
        return "irrecoverable stream error";
    } else if (state == std::ios_base::eofbit) {
        return "end of file reached";
    } else {
        return "no error";
    }
}

std::istream::iostate InputFile::rdstate() const noexcept
{
    if (is) {
        return is->rdstate();
    } else {
        return std::ios_base::badbit;
    }
}

bool InputFile::is_open() const noexcept
{
    if (is) {
        auto *ss = dynamic_cast<std::ifstream *>(is.get());
        if (ss) {
            return ss->is_open();
        } else {
            return true;
        }
    } else {
        return false;
    }
}

void InputFile::backspace() noexcept
{
    if (is) {
        is->clear();
        std::streamoff g1(is->tellg()); // Current position
        is->seekg(0, std::ios::beg);    // Beginning of file
        std::streampos const g0(is->tellg());
        is->seekg(g1, std::ios::beg); // Restore position
        if (g1 > g0) {
            --g1;
        }
        while (g1 > g0) {
            is->seekg(--g1, std::ios::beg); // Backup by 1
            if (is->peek() == '\n') {       // Found end of previous record
                is->seekg(++g1, std::ios::beg);
                break;
            }
        }
    }
}

InputOutputFile &InputOutputFile::ensure_open(EnergyPlusData &state, const std::string &caller, bool output_to_file)
{
    if (!good()) {
        open(false, output_to_file);
    }
    if (!good()) {
        ShowFatalError(state, fmt::format("{}: Could not open file {} for output (write).", caller, filePath));
    }
    return *this;
}

bool InputOutputFile::good() const
{
    if (os && print_to_dev_null && os->bad()) { // badbit is set
        return true;
    } else if (os) {
        return os->good();
    } else {
        return false;
    }
}

void InputOutputFile::close()
{
    os.reset();
}

void InputOutputFile::del()
{
    if (os) {
        os.reset();
        FileSystem::removeFile(filePath);
    }
}

void InputOutputFile::open_as_stringstream()
{
    os = std::make_unique<std::stringstream>();
}

void InputOutputFile::flush()
{
    if (os) {
        os->flush();
    }
}

std::string InputOutputFile::get_output()
{
    auto *ss = dynamic_cast<std::stringstream *>(os.get());
    if (ss) {
        return ss->str();
    } else {
        return "";
    }
}

InputOutputFile::InputOutputFile(fs::path FilePath, const bool DefaultToStdout) : filePath{std::move(FilePath)}, defaultToStdOut{DefaultToStdout}
{
}

std::ostream::pos_type InputOutputFile::position() const noexcept
{
    return os->tellg();
}

void InputOutputFile::open(const bool forAppend, bool output_to_file)
{
    auto appendMode = [=]() {
        if (forAppend) {
            return std::ios_base::app;
        } else {
            return std::ios_base::trunc;
        }
    }();
    if (!output_to_file) {
        os = std::make_unique<std::iostream>(nullptr);
        // os->imbue(std::locale("C"));
        print_to_dev_null = true;
    } else {
        os = std::make_unique<std::fstream>(filePath.c_str(), std::ios_base::in | std::ios_base::out | appendMode);
        // os->imbue(std::locale("C"));
        print_to_dev_null = false;
    }
}

std::vector<std::string> InputOutputFile::getLines()
{
    if (os) {
        // avoid saving and reloading the file by simply reading the current input stream
        os->flush();
        const size_t last_pos = os->tellg();
        std::string line;
        std::vector<std::string> lines;
        os->seekg(0);

        while (std::getline(*os, line)) {
            lines.push_back(line);
        }

        // after getline is done, we're at eof/fail bit
        os->clear();
        os->seekg(last_pos);
        return lines;
    }
    return std::vector<std::string>();
}

bool IOFiles::OutputControl::writeTabular(EnergyPlusData &state)
{
    bool const htmlTabular = state.files.outputControl.tabular;
    bool const jsonTabular = state.files.outputControl.json && state.dataResultsFramework->resultsFramework->timeSeriesAndTabularEnabled();
    bool const sqliteTabular = state.files.outputControl.sqlite; // && @JasonGlazer thinks something else maybe?
    return (htmlTabular || jsonTabular || sqliteTabular);
}

void IOFiles::OutputControl::getInput(EnergyPlusData &state)
{
    auto &ip = state.dataInputProcessing->inputProcessor;
    auto const instances = ip->epJSON.find("OutputControl:Files");
    if (instances != ip->epJSON.end()) {

        auto find_input = [=, &state](nlohmann::json const &fields, std::string const &field_name) -> std::string {
            std::string input;
            auto found = fields.find(field_name);
            if (found != fields.end()) {
                input = found.value().get<std::string>();
                input = Util::makeUPPER(input);
            } else {
                state.dataInputProcessing->inputProcessor->getDefaultValue(state, "OutputControl:Files", field_name, input);
            }
            return input;
        };

        auto boolean_choice = [=, &state](std::string const &input) -> bool {
            if (input == "YES") {
                return true;
            } else if (input == "NO") {
                return false;
            }
            ShowFatalError(state, "Invalid boolean Yes/No choice input");
            return true;
        };

        auto &instancesValue = instances.value();
        for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
            auto const &fields = instance.value();

            ip->markObjectAsUsed("OutputControl:Files", instance.key());

            { // "output_csv"
                csv = boolean_choice(find_input(fields, "output_csv"));
            }
            { // "output_mtr"
                mtr = boolean_choice(find_input(fields, "output_mtr"));
            }
            { // "output_eso"
                eso = boolean_choice(find_input(fields, "output_eso"));
            }
            { // "output_eio"
                eio = boolean_choice(find_input(fields, "output_eio"));
            }
            { // "output_audit"
                audit = boolean_choice(find_input(fields, "output_audit"));
            }
            { // "output_space_sizing"
                spsz = boolean_choice(find_input(fields, "output_space_sizing"));
            }
            { // "output_zone_sizing"
                zsz = boolean_choice(find_input(fields, "output_zone_sizing"));
            }
            { // "output_system_sizing"
                ssz = boolean_choice(find_input(fields, "output_system_sizing"));
            }
            { // "output_dxf"
                dxf = boolean_choice(find_input(fields, "output_dxf"));
            }
            { // "output_bnd"
                bnd = boolean_choice(find_input(fields, "output_bnd"));
            }
            { // "output_rdd"
                rdd = boolean_choice(find_input(fields, "output_rdd"));
            }
            { // "output_mdd"
                mdd = boolean_choice(find_input(fields, "output_mdd"));
            }
            { // "output_mtd"
                mtd = boolean_choice(find_input(fields, "output_mtd"));
            }
            { // "output_end"
                end = boolean_choice(find_input(fields, "output_end"));
            }
            { // "output_shd"
                shd = boolean_choice(find_input(fields, "output_shd"));
            }
            { // "output_dfs"
                dfs = boolean_choice(find_input(fields, "output_dfs"));
            }
            { // "output_glhe"
                glhe = boolean_choice(find_input(fields, "output_glhe"));
            }
            { // "output_delightin"
                delightin = boolean_choice(find_input(fields, "output_delightin"));
            }
            { // "output_delighteldmp"
                delighteldmp = boolean_choice(find_input(fields, "output_delighteldmp"));
            }
            { // "output_delightdfdmp"
                delightdfdmp = boolean_choice(find_input(fields, "output_delightdfdmp"));
            }
            { // "output_edd"
                edd = boolean_choice(find_input(fields, "output_edd"));
            }
            { // "output_dbg"
                dbg = boolean_choice(find_input(fields, "output_dbg"));
            }
            { // "output_perflog"
                perflog = boolean_choice(find_input(fields, "output_perflog"));
            }
            { // "output_sln"
                sln = boolean_choice(find_input(fields, "output_sln"));
            }
            { // "output_sci"
                sci = boolean_choice(find_input(fields, "output_sci"));
            }
            { // "output_wrl"
                wrl = boolean_choice(find_input(fields, "output_wrl"));
            }
            { // "output_screen"
                screen = boolean_choice(find_input(fields, "output_screen"));
            }
            { // "output_tarcog"
                tarcog = boolean_choice(find_input(fields, "output_tarcog"));
            }
            { // "output_extshd"
                extshd = boolean_choice(find_input(fields, "output_extshd"));
            }
            { // "json"
                json = boolean_choice(find_input(fields, "output_json"));
            }
            { // "tabular"
                tabular = boolean_choice(find_input(fields, "output_tabular"));
            }
            { // "sqlite"
                sqlite = boolean_choice(find_input(fields, "output_sqlite"));
            }
        }
    }

    auto const timestamp_instances = ip->epJSON.find("OutputControl:Timestamp");
    if (timestamp_instances != ip->epJSON.end()) {
        auto const &instancesValue = timestamp_instances.value();
        for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
            auto const &fields = instance.value();
            ip->markObjectAsUsed("OutputControl:Timestamp", instance.key());

            auto item = fields.find("iso_8601_format");
            if (item != fields.end()) {
                state.dataResultsFramework->resultsFramework->setISO8601(item->get<std::string>() == "Yes");
            }

            item = fields.find("timestamp_at_beginning_of_interval");
            if (item != fields.end()) {
                state.dataResultsFramework->resultsFramework->setBeginningOfInterval(item->get<std::string>() == "Yes");
            }
        }
    }
}

void IOFiles::flushAll()
{

    audit.flush();
    eio.flush();
    eso.flush();
    zsz.flush();
    spsz.flush();
    ssz.flush();
    map.flush();
    mtr.flush();
    bnd.flush();
    rdd.flush();
    mdd.flush();
    debug.flush();
    dfs.flush();
    mtd.flush();
    edd.flush();
    shade.flush();
    csv.flush();

    if (err_stream) {
        err_stream->flush();
    }
}

} // namespace EnergyPlus

template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, int>(std::string_view, int &&);
template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, const char *const &>(std::string_view, const char *const &);
template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, int &, std::string &>(std::string_view, int &, std::string &);
template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, std::string &, std::string &, std::string &, double &>(
    std::string_view, std::string &, std::string &, std::string &, double &);
template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, const std::string_view &>(std::string_view, const std::string_view &);
template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, const std::string_view &, std::string &>(std::string_view,
                                                                                                                    const std::string_view &,
                                                                                                                    std::string &);
template std::string
EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, std::string &, double &, double &>(std::string_view, std::string &, double &, double &);
template std::string
EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, std::string &, std::string &, int &>(std::string_view, std::string &, std::string &, int &);
template std::string
EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, double &, double &, double &>(std::string_view, double &, double &, double &);
template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, double &, std::string &>(std::string_view, double &, std::string &);
template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, std::string &>(std::string_view, std::string &);
template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, const int &, int &>(std::string_view, const int &, int &);
template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, double>(std::string_view, double &&);
template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, int &, int &>(std::string_view, int &, int &);
template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, const double &>(std::string_view, const double &);
template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, std::string &, int &>(std::string_view, std::string &, int &);
template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, std::string &, std::string &, double &>(std::string_view,
                                                                                                                   std::string &,
                                                                                                                   std::string &,
                                                                                                                   double &);
template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, std::string &, double &, std::string &, double &>(
    std::string_view, std::string &, double &, std::string &, double &);
template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, const int &>(std::string_view, const int &);
template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, int &, const std::string &, std::string &>(std::string_view,
                                                                                                                      int &,
                                                                                                                      const std::string &,
                                                                                                                      std::string &);
template std::string
EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, int &, int &, const std::string &>(std::string_view, int &, int &, const std::string &);
template std::string
EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, int &, int &, std::string_view &>(std::string_view, int &, int &, std::string_view &);
template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, int &, std::string_view &, std::string &>(std::string_view,
                                                                                                                     int &,
                                                                                                                     std::string_view &,
                                                                                                                     std::string &);
template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, double &, double &>(std::string_view, double &, double &);
template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, int &>(std::string_view, int &);
template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, std::string &, double &>(std::string_view, std::string &, double &);
template std::string EnergyPlus::format<EnergyPlus::FormatSyntax::Fortran, double &>(std::string_view, double &);
