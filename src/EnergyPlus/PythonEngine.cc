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

#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/PluginManager.hh>
#include <EnergyPlus/PythonEngine.hh>
#include <EnergyPlus/UtilityRoutines.hh>

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

#    include <fmt/format.h>
namespace fmt {
template <> struct formatter<PyStatus>
{
    // parse is inherited from formatter<string_view>.
    constexpr auto parse(format_parse_context &ctx) -> format_parse_context::iterator
    {
        return ctx.begin();
    }

    auto format(const PyStatus &status, format_context &ctx) const -> format_context::iterator
    {
        if (PyStatus_Exception(status) == 0) {
            return ctx.out();
        }
        if (PyStatus_IsExit(status) != 0) {
            return fmt::format_to(ctx.out(), "Exited with code {}", status.exitcode);
        }
        if (PyStatus_IsError(status) != 0) {
            auto it = ctx.out();
            it = fmt::format_to(it, "Fatal Python error: ");
            if (status.func) {
                it = fmt::format_to(it, "{}: ", status.func);
            }
            it = fmt::format_to(it, "{}", status.err_msg);
            return it;
        }
        return ctx.out();
    }
};
} // namespace fmt

#endif

namespace EnergyPlus {

namespace Python {

#if LINK_WITH_PYTHON

    void reportPythonError([[maybe_unused]] EnergyPlusData &state)
    {
        PyObject *exc_type = nullptr;
        PyObject *exc_value = nullptr;
        PyObject *exc_tb = nullptr;
        PyErr_Fetch(&exc_type, &exc_value, &exc_tb);
        // Normalizing the exception is needed. Without it, our custom EnergyPlusException go through just fine
        // but any ctypes built-in exception for eg will have wrong types
        PyErr_NormalizeException(&exc_type, &exc_value, &exc_tb);
        PyObject *str_exc_value = PyObject_Repr(exc_value); // Now a unicode object
        PyObject *pyStr2 = PyUnicode_AsEncodedString(str_exc_value, "utf-8", "Error ~");
        Py_DECREF(str_exc_value);
        char *strExcValue = PyBytes_AsString(pyStr2); // NOLINT(hicpp-signed-bitwise)
        Py_DECREF(pyStr2);
        EnergyPlus::ShowContinueError(state, "Python error description follows: ");
        EnergyPlus::ShowContinueError(state, strExcValue);

        // See if we can get a full traceback.
        // Calls into python, and does the same as capturing the exception in `e`
        // then `print(traceback.format_exception(e.type, e.value, e.tb))`
        PyObject *pModuleName = PyUnicode_DecodeFSDefault("traceback");
        PyObject *pyth_module = PyImport_Import(pModuleName);
        Py_DECREF(pModuleName);

        if (pyth_module == nullptr) {
            EnergyPlus::ShowContinueError(state, "Cannot find 'traceback' module in reportPythonError(), this is weird");
            return;
        }

        PyObject *pyth_func = PyObject_GetAttrString(pyth_module, "format_exception");
        Py_DECREF(pyth_module); // PyImport_Import returns a new reference, decrement it

        if (pyth_func || PyCallable_Check(pyth_func)) {

            PyObject *pyth_val = PyObject_CallFunction(pyth_func, "OOO", exc_type, exc_value, exc_tb);

            // traceback.format_exception returns a list, so iterate on that
            if (!pyth_val || !PyList_Check(pyth_val)) { // NOLINT(hicpp-signed-bitwise)
                EnergyPlus::ShowContinueError(state, "In reportPythonError(), traceback.format_exception did not return a list.");
                return;
            }

            Py_ssize_t numVals = PyList_Size(pyth_val);
            if (numVals == 0) {
                EnergyPlus::ShowContinueError(state, "No traceback available");
                return;
            }

            EnergyPlus::ShowContinueError(state, "Python traceback follows: ");

            EnergyPlus::ShowContinueError(state, "```");

            for (Py_ssize_t itemNum = 0; itemNum < numVals; itemNum++) {
                PyObject *item = PyList_GetItem(pyth_val, itemNum);
                if (PyUnicode_Check(item)) { // NOLINT(hicpp-signed-bitwise) -- something inside Python code causes warning
                    std::string traceback_line = PyUnicode_AsUTF8(item);
                    if (!traceback_line.empty() && traceback_line[traceback_line.length() - 1] == '\n') {
                        traceback_line.erase(traceback_line.length() - 1);
                    }
                    EnergyPlus::ShowContinueError(state, format(" >>> {}", traceback_line));
                }
                // PyList_GetItem returns a borrowed reference, do not decrement
            }

            EnergyPlus::ShowContinueError(state, "```");

            // PyList_Size returns a borrowed reference, do not decrement
            Py_DECREF(pyth_val); // PyObject_CallFunction returns new reference, decrement
        }
        Py_DECREF(pyth_func); // PyObject_GetAttrString returns a new reference, decrement it
    }

    void addToPythonPath(EnergyPlusData &state, const fs::path &includePath, bool userDefinedPath)
    {
        if (includePath.empty()) {
            return;
        }

        // We use generic_string / generic_wstring here, which will always use a forward slash as directory separator even on windows
        // This doesn't handle the (very strange, IMHO) case were on unix you have backlashes (which are VALID filenames on Unix!)
        // Could use FileSystem::makeNativePath first to convert the backslashes to forward slashes on Unix
        PyObject *unicodeIncludePath = nullptr;
        if constexpr (std::is_same_v<typename fs::path::value_type, wchar_t>) {
            const std::wstring ws = includePath.generic_wstring();
            unicodeIncludePath = PyUnicode_FromWideChar(ws.c_str(), static_cast<Py_ssize_t>(ws.size())); // New reference
        } else {
            const std::string s = includePath.generic_string();
            unicodeIncludePath = PyUnicode_FromString(s.c_str()); // New reference
        }
        if (unicodeIncludePath == nullptr) {
            EnergyPlus::ShowFatalError(
                state, format("ERROR converting the path \"{}\" for addition to the sys.path in Python", includePath.generic_string()));
        }

        PyObject *sysPath = PySys_GetObject("path"); // Borrowed reference
        int const ret = PyList_Insert(sysPath, 0, unicodeIncludePath);
        Py_DECREF(unicodeIncludePath);

        if (ret != 0) {
            if (PyErr_Occurred()) {
                reportPythonError(state);
            }
            EnergyPlus::ShowFatalError(state, format("ERROR adding \"{}\" to the sys.path in Python", includePath.generic_string()));
        }

        if (userDefinedPath) {
            EnergyPlus::ShowMessage(state, format("Successfully added path \"{}\" to the sys.path in Python", includePath.generic_string()));
        }

        // PyRun_SimpleString)("print(' EPS : ' + str(sys.path))");
    }

    void initPython(EnergyPlusData &state, fs::path const &pathToPythonPackages)
    {
        PyStatus status;

        // first pre-config Python so that it can speak UTF-8
        PyPreConfig preConfig;
        // This is the other related line that caused Decent CI to start having trouble.  I'm putting it back to
        // PyPreConfig_InitPythonConfig, even though I think it should be isolated.  Will deal with this after IO freeze.
        PyPreConfig_InitPythonConfig(&preConfig);
        // PyPreConfig_InitIsolatedConfig(&preConfig);
        preConfig.utf8_mode = 1;
        status = Py_PreInitialize(&preConfig);
        if (PyStatus_Exception(status) != 0) {
            ShowFatalError(state, fmt::format("Could not pre-initialize Python to speak UTF-8... {}", status));
        }

        PyConfig config;
        PyConfig_InitIsolatedConfig(&config);
        config.isolated = 1;

        status = PyConfig_SetBytesString(&config, &config.program_name, PluginManagement::programName);
        if (PyStatus_Exception(status) != 0) {
            ShowFatalError(state, fmt::format("Could not initialize program_name on PyConfig... {}", status));
        }

        status = PyConfig_Read(&config);
        if (PyStatus_Exception(status) != 0) {
            ShowFatalError(state, fmt::format("Could not read back the PyConfig... {}", status));
        }

        if constexpr (std::is_same_v<typename fs::path::value_type, wchar_t>) {
            // PyConfig_SetString copies the wide character string str into *config_str.
            std::wstring const ws = pathToPythonPackages.generic_wstring();
            const wchar_t *wcharPath = ws.c_str();

            status = PyConfig_SetString(&config, &config.home, wcharPath);
            if (PyStatus_Exception(status) != 0) {
                ShowFatalError(state, fmt::format("Could not set home to {} on PyConfig... {}", pathToPythonPackages.generic_string(), status));
            }
            status = PyConfig_SetString(&config, &config.base_prefix, wcharPath);
            if (PyStatus_Exception(status) != 0) {
                ShowFatalError(state,
                               fmt::format("Could not set base_prefix to {} on PyConfig... {}", pathToPythonPackages.generic_string(), status));
            }
            config.module_search_paths_set = 1;
            status = PyWideStringList_Append(&config.module_search_paths, wcharPath);
            if (PyStatus_Exception(status) != 0) {
                ShowFatalError(
                    state, fmt::format("Could not add {} to module_search_paths on PyConfig... {}", pathToPythonPackages.generic_string(), status));
            }

        } else {
            // PyConfig_SetBytesString takes a `const char * str` and decodes str using Py_DecodeLocale() and set the result into *config_str
            // But we want to avoid doing it three times, so we PyDecodeLocale manually
            // Py_DecodeLocale can be called because Python has been PreInitialized.
            wchar_t *wcharPath = Py_DecodeLocale(pathToPythonPackages.generic_string().c_str(), nullptr); // This allocates!

            status = PyConfig_SetString(&config, &config.home, wcharPath);
            if (PyStatus_Exception(status) != 0) {
                ShowFatalError(state, fmt::format("Could not set home to {} on PyConfig... {}", pathToPythonPackages.generic_string(), status));
            }
            status = PyConfig_SetString(&config, &config.base_prefix, wcharPath);
            if (PyStatus_Exception(status) != 0) {
                ShowFatalError(state,
                               fmt::format("Could not set base_prefix to {} on PyConfig... {}", pathToPythonPackages.generic_string(), status));
            }
            config.module_search_paths_set = 1;
            status = PyWideStringList_Append(&config.module_search_paths, wcharPath);
            if (PyStatus_Exception(status) != 0) {
                ShowFatalError(
                    state, fmt::format("Could not add {} to module_search_paths on PyConfig... {}", pathToPythonPackages.generic_string(), status));
            }

            PyMem_RawFree(wcharPath);
        }

        // This was Py_InitializeFromConfig(&config), but was giving a seg fault when running inside
        // another Python instance, for example as part of an API run.  Per the example here:
        // https://docs.python.org/3/c-api/init_config.html#preinitialize-python-with-pypreconfig
        // It looks like we don't need to initialize from config again, it should be all set up with
        // the init calls above, so just initialize and move on.
        // UPDATE: This worked happily for me on Linux, and also when I build locally on Windows, but not on Decent CI
        // I suspect a difference in behavior for Python versions.  I'm going to temporarily revert this back to initialize
        // with config and get IO freeze going, then get back to solving it.
        // Py_Initialize();
        Py_InitializeFromConfig(&config);
    }

    PythonEngine::PythonEngine(EnergyPlusData &state) : eplusRunningViaPythonAPI(state.dataPluginManager->eplusRunningViaPythonAPI)
    {
        // we'll need the program directory for a few things so get it once here at the top and sanitize it
        fs::path programDir;
        if (state.dataGlobal->installRootOverride) {
            programDir = state.dataStrGlobals->exeDirectoryPath;
        } else {
            programDir = FileSystem::getParentDirectoryPath(FileSystem::getAbsolutePath(FileSystem::getProgramPath()));
        }
        fs::path const pathToPythonPackages = programDir / "python_lib";

        initPython(state, pathToPythonPackages);

        // we also need to set an extra import path to find some dynamic library loading stuff, again make it relative to the binary
        addToPythonPath(state, programDir / "python_lib/lib-dynload", false);

        // now for additional paths:
        // we'll always want to add the program executable directory to PATH so that Python can find the installed pyenergyplus package
        // we will then optionally add the current working directory to allow Python to find scripts in the current directory
        // we will then optionally add the directory of the running IDF to allow Python to find scripts kept next to the IDF
        // we will then optionally add any additional paths the user specifies on the search paths object

        // so add the executable directory here
        addToPythonPath(state, programDir, false);

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
        cmd += fmt::format("sys.path.insert(0, \"{}\")\n", sPathToPythonPackages);
        return cmd;
    }

    std::string PythonEngine::getTclPreppedPreamble(std::vector<std::string> const &python_fwd_args)
    {
        std::string cmd = R"python(import sys
sys.argv.clear()
sys.argv.append("energyplus")
)python";
        for (const auto &arg : python_fwd_args) {
            cmd += fmt::format("sys.argv.append(\"{}\")\n", arg);
        }
        fs::path programDir = FileSystem::getParentDirectoryPath(FileSystem::getAbsolutePath(FileSystem::getProgramPath()));
        fs::path const pathToPythonPackages = programDir / "python_lib";
        std::string sPathToPythonPackages = std::string(pathToPythonPackages.string());
        std::replace(sPathToPythonPackages.begin(), sPathToPythonPackages.end(), '\\', '/');
        cmd += fmt::format("sys.path.insert(0, \"{}\")\n", sPathToPythonPackages);
        std::string tclConfigDir;
        std::string tkConfigDir;
        for (auto &p : std::filesystem::directory_iterator(pathToPythonPackages)) {
            if (p.is_directory()) {
                std::string dirName = p.path().filename().string();
                if (dirName.find("tcl", 0) == 0 && dirName.find('.', 0) > 0) {
                    tclConfigDir = dirName;
                }
                if (dirName.find("tk", 0) == 0 && dirName.find('.', 0) > 0) {
                    tkConfigDir = dirName;
                }
                if (!tclConfigDir.empty() && !tkConfigDir.empty()) {
                    break;
                }
            }
        }
        cmd += "from os import environ\n";
        cmd += fmt::format("environ[\'TCL_LIBRARY\'] = \"{}/{}\"\n", sPathToPythonPackages, tclConfigDir);
        cmd += fmt::format("environ[\'TK_LIBRARY\'] = \"{}/{}\"\n", sPathToPythonPackages, tkConfigDir);
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
