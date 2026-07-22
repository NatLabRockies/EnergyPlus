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

#ifndef IOFiles_hh_INCLUDED
#define IOFiles_hh_INCLUDED

// C++ Headers
#include <cassert>
#include <format>
#include <fstream>
#include <iostream>
#include <limits>
#include <ostream>
#include <vector>

// Third Party Headers
#include <nlohmann/json.hpp>

// EnergyPlus Headers
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/FileSystem.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

class InputOutputFile;
template <typename... Args> void print(InputOutputFile &outputFile, std::format_string<Args...> format_str, Args &&...args);

class InputFile
{
public:
    template <typename Type> struct ReadResult
    {
        ReadResult(Type data_, bool eof_, bool good_) : data{std::move(data_)}, eof{eof_}, good{good_}
        {
        }

        // Update the current eof/good state from the incoming value
        // but only update the `data` member if the state is good
        // The idea is to keep consistency with the operator>> that was used
        // from gio
        void update(ReadResult &&other)
        {
            eof = other.eof;
            good = other.good;
            if (good) {
                data = std::move(other.data);
            }
        }

        Type data;
        bool eof;
        bool good;
    };

    void close();

    // This is different from istream::good(), which is false if EOF is true while there were no errors (happens when no EOL at end of file)
    // this operate like `operator bool(istream& is)` <=> `!is.bad() && !is.fail()`
    bool good() const noexcept;

    bool is_open() const noexcept;

    void backspace() noexcept;

    std::string error_state_to_string() const;

    // opens the file if it is not currently open and returns
    // a reference back to itself
    InputFile &ensure_open(EnergyPlusData &state, const std::string &caller, bool output_to_file = true);
    std::istream::iostate rdstate() const noexcept;

    fs::path filePath;
    void open(bool = false, bool = true);
    std::fstream::pos_type position() const noexcept;

    void rewind() noexcept
    {
        if (is) {
            is->clear(); // clear potentially failbit and badbit (seekg would only clear eofbit)
            is->seekg(0, std::ios::beg);
        }
    }

    ReadResult<std::string> readLine() noexcept;

    template <typename T> ReadResult<T> read() noexcept
    {
        if (is) {
            T result;
            *is >> result;
            // Use operator bool, see ReadResult::good() docstring
            return ReadResult<T>{result, is->eof(), bool(is)};
        }
        return ReadResult<T>{T{}, true, false};
    }

    std::string readFile();

    nlohmann::json readJSON();

    explicit InputFile(fs::path FilePath);

private:
    std::uintmax_t file_size{};
    std::unique_ptr<std::istream> is;
    friend class IOFiles;
};

class InputOutputFile
{
public:
    fs::path filePath;
    bool defaultToStdOut = false;

    void close();
    void del();
    bool good() const;

    // opens the file if it is not currently open and returns
    // a reference back to itself
    InputOutputFile &ensure_open(EnergyPlusData &state, const std::string &caller, bool output_to_file = true);

    void open(const bool forAppend = false, bool output_to_file = true);
    std::fstream::pos_type position() const noexcept;
    std::vector<std::string> getLines();
    void open_as_stringstream();
    std::string get_output();
    void flush();
    explicit InputOutputFile(fs::path FilePath, const bool DefaultToStdOut = false);

private:
    std::unique_ptr<std::iostream> os;
    bool print_to_dev_null = false;
    template <typename... Args> friend void print(InputOutputFile &outputFile, std::format_string<Args...>, Args &&...args);
    friend class IOFiles;
};

template <typename FileType> struct IOFilePath
{
    fs::path filePath;
    FileType open(EnergyPlusData &state, const std::string &caller, bool output_to_file = true)
    {
        FileType file{filePath};
        file.ensure_open(state, caller, output_to_file);
        return file;
    }
    FileType try_open(bool output_to_file = true)
    {
        FileType file{filePath};
        file.open(false, output_to_file);
        return file;
    }
};

using InputOutputFilePath = IOFilePath<InputOutputFile>;
using InputFilePath = IOFilePath<InputFile>;

struct JsonOutputFilePaths
{
    fs::path outputJsonFilePath;
    fs::path outputTSHvacJsonFilePath;
    fs::path outputTSZoneJsonFilePath;
    fs::path outputTSJsonFilePath;
    fs::path outputYRJsonFilePath;
    fs::path outputMNJsonFilePath;
    fs::path outputDYJsonFilePath;
    fs::path outputHRJsonFilePath;
    fs::path outputSMJsonFilePath;
    fs::path outputCborFilePath;
    fs::path outputTSHvacCborFilePath;
    fs::path outputTSZoneCborFilePath;
    fs::path outputTSCborFilePath;
    fs::path outputYRCborFilePath;
    fs::path outputMNCborFilePath;
    fs::path outputDYCborFilePath;
    fs::path outputHRCborFilePath;
    fs::path outputSMCborFilePath;
    fs::path outputMsgPackFilePath;
    fs::path outputTSHvacMsgPackFilePath;
    fs::path outputTSZoneMsgPackFilePath;
    fs::path outputTSMsgPackFilePath;
    fs::path outputYRMsgPackFilePath;
    fs::path outputMNMsgPackFilePath;
    fs::path outputDYMsgPackFilePath;
    fs::path outputHRMsgPackFilePath;
    fs::path outputSMMsgPackFilePath;
};

class IOFiles
{
public:
    struct OutputControl
    {
        OutputControl() = default;

        void getInput(EnergyPlusData &state);
        bool writeTabular(EnergyPlusData &state);

        bool csv = false;
        bool mtr = true;
        bool eso = true;
        bool eio = true;
        bool audit = true;
        bool spsz = true;
        bool zsz = true;
        bool ssz = true;
        bool psz = true;
        bool dxf = true;
        bool bnd = true;
        bool rdd = true;
        bool mdd = true;
        bool mtd = true;
        bool end = true;
        bool shd = true;
        bool dfs = true;
        bool delightin = true;
        bool delighteldmp = true;
        bool delightdfdmp = true;
        bool edd = true;
        bool dbg = true;
        bool perflog = true;
        bool sln = true;
        bool sci = true;
        bool wrl = true;
        bool screen = true;
        bool tarcog = true;
        bool extshd = true;
        bool json = true;
        bool tabular = true;
        bool sqlite = true;
    };

    OutputControl outputControl;

    InputOutputFile audit{"eplusout.audit"};
    InputOutputFile eio{"eplusout.eio"};
    InputOutputFile eso{"eplusout.eso"}; // (hourly data only)

    InputOutputFile zsz{""};
    fs::path outputZszCsvFilePath{"epluszsz.csv"};
    fs::path outputZszTabFilePath{"epluszsz.tab"};
    fs::path outputZszTxtFilePath{"epluszsz.txt"};

    InputOutputFile spsz{""};
    fs::path outputSpszCsvFilePath{"eplusspsz.csv"};
    fs::path outputSpszTabFilePath{"eplusspsz.tab"};
    fs::path outputSpszTxtFilePath{"eplusspsz.txt"};

    InputOutputFile ssz{""};
    fs::path outputSszCsvFilePath{"eplusssz.csv"};
    fs::path outputSszTabFilePath{"eplusssz.tab"};
    fs::path outputSszTxtFilePath{"eplusssz.txt"};

    InputOutputFile psz{""};
    fs::path outputPszCsvFilePath{"epluspsz.csv"};
    fs::path outputPszTabFilePath{"epluspsz.tab"};
    fs::path outputPszTxtFilePath{"epluspsz.txt"};

    InputOutputFile map{""};
    fs::path outputMapCsvFilePath{"eplusmap.csv"};
    fs::path outputMapTabFilePath{"eplusmap.tab"};
    fs::path outputMapTxtFilePath{"eplusmap.txt"};

    InputOutputFile mtr{"eplusout.mtr"};
    InputOutputFile bnd{"eplusout.bnd"};
    InputOutputFile rdd{"eplusout.rdd"};
    InputOutputFile mdd{"eplusout.mdd"};

    InputOutputFile debug{"eplusout.dbg"};

    InputOutputFile dfs{"eplusout.dfs"};

    InputOutputFilePath sln{"eplusout.sln"};
    InputOutputFilePath dxf{"eplusout.dxf"};
    InputOutputFilePath sci{"eplusout.sci"};
    InputOutputFilePath wrl{"eplusout.wrl"};

    InputOutputFilePath delightIn{"eplusout.delightin"};

    InputOutputFile mtd{"eplusout.mtd"};
    InputOutputFile edd{"eplusout.edd", true}; // write to stdout if no file never opened
    InputOutputFile shade{"eplusshading.csv"};

    InputOutputFile csv{"eplusout.csv"};
    InputOutputFile mtr_csv{"eplusmtr.csv"};

    InputOutputFilePath screenCsv{"eplusscreen.csv"};
    InputOutputFilePath endFile{"eplusout.end"};

    InputFilePath iniFile{"EnergyPlus.ini"};

    InputFilePath outputDelightEldmpFilePath{"eplusout.delighteldmp"};
    InputFilePath outputDelightDfdmpFilePath{"eplusout.delightdfdmp"};

    // for transient uses of weather files
    // also, keeper of the currently set input weather file name
    InputFilePath inputWeatherFilePath{""};

    // for the persistent weather simulation, using the EPW
    // uses the file name set in `inputWeatherFilePath`
    InputFile inputWeatherFile{""};

    InputFilePath TempFullFilePath{""};
    InputFilePath inStatFilePath{""};

    fs::path outputErrFilePath{"eplusout.err"};
    std::unique_ptr<std::ostream> err_stream;

    JsonOutputFilePaths json; // Internal streams used for json outputs

    void flushAll(); // For RunningEnergyPlusViaAPI only
};

class SharedFileHandle
{
    std::shared_ptr<InputOutputFile> file;
    InputOutputFile *ptr()
    {
        if (!file) {
            file = std::make_shared<InputOutputFile>("");
        }

        return file.get();
    }

public:
    InputOutputFile &operator*()
    {
        return *ptr();
    }

    InputOutputFile *operator->()
    {
        return ptr();
    }
};

template <typename... Args> void print(InputOutputFile &outputFile, std::format_string<Args...> format_str, Args &&...args)
{
    auto *outputStream = [&]() -> std::ostream * {
        if (outputFile.os) {
            return outputFile.os.get();
        }
        if (outputFile.defaultToStdOut) {
            return &std::cout;
        }
        assert(outputFile.os);
        return nullptr;
    }();
    *outputStream << std::format(format_str, std::forward<Args>(args)...);
}

} // namespace EnergyPlus

#endif
