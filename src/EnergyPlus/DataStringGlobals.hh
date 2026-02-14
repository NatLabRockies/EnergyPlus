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

#ifndef DataStringGlobals_hh_INCLUDED
#define DataStringGlobals_hh_INCLUDED

// C++ Headers
#include <string>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/FileSystem.hh>

namespace EnergyPlus {

namespace DataStringGlobals {

#ifdef _WIN32
    char constexpr pathChar('\\');
    char constexpr altpathChar('/');
#elif __linux__
    char constexpr pathChar('/');
    char constexpr altpathChar('\\');
#elif __unix__
    char constexpr pathChar('/');
    char constexpr altpathChar('\\');
#elif __posix__
    char constexpr pathChar('/');
    char constexpr altpathChar('\\');
#elif __APPLE__
    char constexpr pathChar('/');
    char constexpr altpathChar('\\');
#else
#    error "Invalid platform detection in DataStringGlobals."
#endif
    char constexpr CharComma(',');     // comma
    char constexpr CharSemicolon(';'); // semicolon
    char constexpr CharTab('\t');      // tab
    char constexpr CharSpace(' ');     // space
    constexpr std::string_view zonePrefix = "Zone";
    constexpr std::string_view spacePrefix = "Space";

    extern std::string const VerString;        // String that represents version information
    extern std::string const MatchVersion;     // String to be matched by Version object
    extern std::string const PythonAPIVersion; // API version string to be matched when using the Python API
    extern std::string const BuildPlatformString;
} // namespace DataStringGlobals

struct DataStringGlobalsData : BaseGlobalStruct
{

    fs::path outputMddFilePath = "eplusout.mdd";
    fs::path outputRddFilePath = "eplusout.rdd";
    fs::path outputShdFilePath = "eplusout.shd";
    fs::path outputTblCsvFilePath = "eplustbl.csv";
    fs::path outputTblHtmFilePath = "eplustbl.htm";
    fs::path outputTblTabFilePath = "eplustbl.tab";
    fs::path outputTblTxtFilePath = "eplustbl.txt";
    fs::path outputTblXmlFilePath = "eplustbl.xml";
    fs::path outputAdsFilePath = "eplusADS.out";
    fs::path outputGLHEFilePath = "eplusout.glhe";
    fs::path outputDelightOutFilePath = "eplusout.delightout";
    fs::path outputIperrFilePath = "eplusout.iperr";
    fs::path outputPerfLogFilePath = "eplusout_perflog.csv";
    fs::path outputSqlFilePath = "eplusout.sql";
    fs::path outputSqliteErrFilePath = "eplussqlite.err";
    fs::path outputCsvFilePath = "eplusout.csv";
    fs::path outputMtrCsvFilePath = "eplusmtr.csv";
    fs::path outputRvauditFilePath = "eplusout.rvaudit";

    fs::path outputErrFilePath;
    fs::path eplusADSFilePath;
    fs::path inputFilePath; // Full name of file to open, including path `/path/to/myfile.idf`
    fs::path inputIddFilePath;
    fs::path inputEpJSONSchemaFilePath;
    fs::path outDirPath;
    fs::path inputDirPath;          // `/path/to/`
    fs::path inputFilePathNameOnly; // `myfile.idf`
    fs::path exeDirectoryPath;
    fs::path ProgramPath;          // Path for Program from INI file
    fs::path CurrentWorkingFolder; // Current working directory for run

    std::string IDDVerString;    // Version information from the IDD (line 1)
    std::string CurrentDateTime; // For printing current date and time at start of run
    std::string VerStringVar;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        outputErrFilePath.clear();
        eplusADSFilePath.clear();
        outDirPath.clear();
        inputFilePathNameOnly.clear();
        inputDirPath.clear();
        exeDirectoryPath.clear();
        inputFilePath.clear();
        inputIddFilePath.clear();
        inputEpJSONSchemaFilePath.clear();
        ProgramPath.clear();
        CurrentWorkingFolder.clear();
        CurrentDateTime.clear();
        IDDVerString.clear();
        VerStringVar.clear();
    }
};

} // namespace EnergyPlus

#endif
