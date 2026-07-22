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

// Third Party Headers
#if LINK_WITH_PYTHON
#    ifdef _DEBUG
// We don't want to try to import a debug build of Python here
// so if we are building a Debug build of the C++ code, we need
// to undefine _DEBUG during the #include command for Python.h.
// Otherwise it will fail
#        undef _DEBUG
#        include <Python.h>
#        define _DEBUG
#    else
#        include <Python.h>
#    endif
#endif

// C++ Headers
#include <filesystem>
#include <format>

// EnergyPlus Headers
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/FileSystem.hh>
#include <EnergyPlus/Formatters.hh>
#include <EnergyPlus/PluginManager.hh>
#include <EnergyPlus/PythonEngine.hh>
#include <EnergyPlus/UtilityRoutines.hh>

#if LINK_WITH_PYTHON
#    include <EnergyPlus/PythonHelpers.hh>
#endif

namespace EnergyPlus {

namespace Python {

#if LINK_WITH_PYTHON

    PythonEngine::PythonEngine(EnergyPlusData &state) : eplusRunningViaPythonAPI(state.dataPluginManager->eplusRunningViaPythonAPI)
    {
        // we'll need the program directory for a few things so get it once here at the top and sanitize it
        fs::path programDir;
        if (state.dataGlobal->installRootOverride) {
            programDir = state.dataStrGlobals->exeDirectoryPath;
        } else {
            programDir = FileSystem::getParentDirectoryPath(FileSystem::getAbsolutePath(FileSystem::getProgramPath()));
        }

        EnergyPlus::PythonHelpers::initPython(state, programDir);

        PyObject *m = PyImport_AddModule("__main__");
        if (m == nullptr) {
            throw std::runtime_error("Unable to add module __main__ for python script execution");
        }
        m_globalDict = PyModule_GetDict(m);
    }

    void PythonEngine::exec(std::string_view sv)
    {
        std::string command{sv};

        PyObject *v = PyRun_String(command.c_str(), Py_file_input, m_globalDict, m_globalDict);
        // PyObject* v = PyRun_SimpleString(command.c_str());
        if (v == nullptr) {
            PyErr_Print();
            throw std::runtime_error("Error executing Python code");
        }

        Py_DECREF(v);
    }

    PythonEngine::~PythonEngine()
    {
        if (!this->eplusRunningViaPythonAPI) {
            bool alreadyInitialized = (Py_IsInitialized() != 0);
            if (alreadyInitialized) {
                if (Py_FinalizeEx() < 0) {
                    exit(120);
                }
            }
        }
    }

    std::string PythonEngine::getBasicPreamble()
    {
        std::string cmd = R"python(import sys
sys.argv.clear()
sys.argv.append("energyplus")
)python";
        fs::path programDir = FileSystem::getParentDirectoryPath(FileSystem::getAbsolutePath(FileSystem::getProgramPath()));
        fs::path const pathToPythonPackages = programDir / "python_lib";
        std::string sPathToPythonPackages = std::string(pathToPythonPackages.string());
        std::replace(sPathToPythonPackages.begin(), sPathToPythonPackages.end(), '\\', '/');
        cmd += std::format("sys.path.insert(0, \"{}\")\n", sPathToPythonPackages);
        return cmd;
    }

    std::string PythonEngine::getTclPreppedPreamble(std::vector<std::string> const &python_fwd_args)
    {
        std::string cmd = R"python(import sys
sys.argv.clear()
sys.argv.append("energyplus")
)python";
        for (const auto &arg : python_fwd_args) {
            cmd += std::format("sys.argv.append(\"{}\")\n", arg);
        }
        fs::path programDir = FileSystem::getParentDirectoryPath(FileSystem::getAbsolutePath(FileSystem::getProgramPath()));
        fs::path const pathToPythonPackages = programDir / "python_lib";
        std::string sPathToPythonPackages = std::string(pathToPythonPackages.string());
        std::replace(sPathToPythonPackages.begin(), sPathToPythonPackages.end(), '\\', '/');
        cmd += std::format("sys.path.insert(0, \"{}\")\n", sPathToPythonPackages);
        std::string tclConfigDir;
        std::string tkConfigDir;
        for (auto &p : std::filesystem::directory_iterator(pathToPythonPackages)) {
            if (p.is_directory()) {
                std::string dirName = p.path().filename().string();
                if (dirName.starts_with("tcl") && dirName.find('.') != std::string::npos) {
                    tclConfigDir = dirName;
                }
                if (dirName.starts_with("tk") && dirName.find('.') != std::string::npos) {
                    tkConfigDir = dirName;
                }
                if (!tclConfigDir.empty() && !tkConfigDir.empty()) {
                    break;
                }
            }
        }
        cmd += "from os import environ\n";
        cmd += std::format("environ[\'TCL_LIBRARY\'] = \"{}/{}\"\n", sPathToPythonPackages, tclConfigDir);
        cmd += std::format("environ[\'TK_LIBRARY\'] = \"{}/{}\"\n", sPathToPythonPackages, tkConfigDir);
        return cmd;
    }

#else // NOT LINK_WITH_PYTHON
    PythonEngine::PythonEngine(EnergyPlus::EnergyPlusData &state)
    {
        ShowFatalError(state, "EnergyPlus is not linked with python");
    }

    PythonEngine::~PythonEngine()
    {
    }

    void PythonEngine::exec(std::string_view)
    {
    }

#endif // LINK_WITH_PYTHON

} // namespace Python
} // namespace EnergyPlus
