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

#if LINK_WITH_PYTHON

// C++ Headers
#    include <format>

// EnergyPlus Headers
#    include <EnergyPlus/Data/EnergyPlusData.hh>
#    include <EnergyPlus/FileSystem.hh>
#    include <EnergyPlus/PythonHelpers.hh>
#    include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace PythonHelpers {

    namespace {
        // RAII helper to convert a std::filesystem::path to a wchar_t* that can be passed to Python C API functions, and ensure proper cleanup of
        // the wchar_t* if it was allocated
        struct PyWcharPath
        {
            PyWcharPath(const PyWcharPath &) = delete;
            PyWcharPath &operator=(const PyWcharPath &) = delete;
            PyWcharPath(PyWcharPath &&) = delete;
            PyWcharPath &operator=(PyWcharPath &&) = delete;

            explicit PyWcharPath(const fs::path &p)
            {
                if constexpr (std::is_same_v<fs::path::value_type, wchar_t>) {
                    wstr_ = p.generic_wstring();
                    ptr_ = wstr_.data();
                } else {
                    ptr_ = Py_DecodeLocale(p.generic_string().c_str(), nullptr);
                }
            }

            ~PyWcharPath()
            {
                if constexpr (!std::is_same_v<fs::path::value_type, wchar_t>) {
                    PyMem_RawFree(ptr_);
                }
            }

            const wchar_t *get() const
            {
                return ptr_;
            }
            operator const wchar_t *() const
            {
                return ptr_;
            }

        private:
            wchar_t *ptr_ = nullptr;
            std::wstring wstr_; // only populated on Windows
        };
    } // namespace

    void initPython(EnergyPlus::EnergyPlusData &state, fs::path const &programDir)
    {
        PyStatus status;

        // first pre-config Python so that it can speak UTF-8
        PyPreConfig preConfig;
        // This is the other related line that caused Decent CI to start having trouble.  I'm putting it back to
        // PyPreConfig_InitPythonConfig, even though I think it should be isolated.  Will deal with this after IO freeze.
        // PyPreConfig_InitIsolatedConfig(&preConfig);
        PyPreConfig_InitPythonConfig(&preConfig);
        // PyPreConfig_InitIsolatedConfig sets configure_locale=0 which likely caused Decent CI failures
        // https://github.com/python/cpython/blob/v3.12.2/Python/preconfig.c#L310-L345
#    if DEBUG_PYTHON_CONFIG
        EnergyPlus::print("PyPreConfig initialized:\n{}\n", preConfig);
#    endif
        preConfig.utf8_mode = 1;
        // disable use_environment so VIRTUAL_ENV/PYTHONPATH don't leak the user's venv into EnergyPlus's embedded Python
        // preConfig.use_environment = 0;
#    if DEBUG_PYTHON_CONFIG
        EnergyPlus::print("Final PyPreConfig:\n{}\n", preConfig);
#    endif
        status = Py_PreInitialize(&preConfig);
        if (PyStatus_Exception(status) != 0) {
            ShowFatalError(state, std::format("Could not pre-initialize Python to speak UTF-8... {}", status));
        }

        PyConfig config;
        PyConfig_InitIsolatedConfig(&config);

#    if DEBUG_PYTHON_CONFIG
        EnergyPlus::print("Isolated config initialized:\n{}\n", config);
#    endif
        config.isolated = 1;

        PyWcharPath wcharProgramPath(FileSystem::getAbsolutePath(FileSystem::getProgramPath()));
        status = PyConfig_SetString(&config, &config.executable, wcharProgramPath);
        if (PyStatus_Exception(status) != 0) {
            ShowFatalError(state, std::format("Could not initialize executable on PyConfig... {}", status));
        }

        status = PyConfig_SetString(&config, &config.program_name, L"energyplus");
        if (PyStatus_Exception(status) != 0) {
            ShowFatalError(state, std::format("Could not initialize program_name on PyConfig... {}", status));
        }

        // Ensure site.py doesn't run, this picks up your virtualenv, even with isolated config
        // config.site_import = 0;

        status = PyConfig_Read(&config);
        if (PyStatus_Exception(status) != 0) {
            ShowFatalError(state, std::format("Could not read back the PyConfig... {}", status));
        }

        fs::path const pathToPythonPackages = programDir / "python_lib";
        {
            PyWcharPath wcharPath(pathToPythonPackages);

            status = PyConfig_SetString(&config, &config.home, wcharPath);
            if (PyStatus_Exception(status) != 0) {
                ShowFatalError(state, std::format("Could not set home to {:g} on PyConfig... {}", pathToPythonPackages, status));
            }

            status = PyConfig_SetString(&config, &config.base_prefix, wcharPath);
            if (PyStatus_Exception(status) != 0) {
                ShowFatalError(state, std::format("Could not set base_prefix to {:g} on PyConfig... {}", pathToPythonPackages, status));
            }

            config.module_search_paths_set = 1;
            status = PyWideStringList_Append(&config.module_search_paths, wcharPath);
            if (PyStatus_Exception(status) != 0) {
                ShowFatalError(state, std::format("Could not add {:g} to module_search_paths on PyConfig... {}", pathToPythonPackages, status));
            }
        }

        {
            // we also need to set an extra import path to find some dynamic library loading stuff, again make it relative to the binary
            fs::path pathToPythonLibDynload = pathToPythonPackages / "lib-dynload";
            PyWcharPath wcharPath(pathToPythonLibDynload);
            status = PyWideStringList_Append(&config.module_search_paths, wcharPath);
            if (PyStatus_Exception(status) != 0) {
                ShowFatalError(state, std::format("Could not add {:g} to module_search_paths on PyConfig... {}", pathToPythonLibDynload, status));
            }
        }

        {
            // we'll always want to add the program executable directory to PATH so that Python can find the installed pyenergyplus package
            PyWcharPath wcharPath(programDir);
            status = PyWideStringList_Append(&config.module_search_paths, wcharPath);
            if (PyStatus_Exception(status) != 0) {
                ShowFatalError(state, std::format("Could not add {:g} to module_search_paths on PyConfig... {}", programDir, status));
            }
        }

#    if DEBUG_PYTHON_CONFIG
        EnergyPlus::print("Final PyConfig:\n{}\n", config);
#    endif
        Py_InitializeFromConfig(&config);
        PyConfig_Clear(&config);
    }

    void reportPythonError([[maybe_unused]] EnergyPlus::EnergyPlusData &state)
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

        if ((pyth_func != nullptr) || (PyCallable_Check(pyth_func) != 0)) {

            PyObject *pyth_val = PyObject_CallFunction(pyth_func, "OOO", exc_type, exc_value, exc_tb); // NOLINT(cppcoreguidelines-pro-type-vararg)

            // traceback.format_exception returns a list, so iterate on that
            if ((pyth_val == nullptr) || !PyList_Check(pyth_val)) { // NOLINT(hicpp-signed-bitwise)
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
                    EnergyPlus::ShowContinueError(state, std::format(" >>> {}", traceback_line));
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
            EnergyPlus::ShowFatalError(state, std::format("ERROR converting the path \"{:g}\" for addition to the sys.path in Python", includePath));
        }

        PyObject *sysPath = PySys_GetObject("path"); // Borrowed reference
        int const ret = PyList_Insert(sysPath, 0, unicodeIncludePath);
        Py_DECREF(unicodeIncludePath);

        if (ret != 0) {
            if (PyErr_Occurred() != nullptr) {
                reportPythonError(state);
            }
            EnergyPlus::ShowFatalError(state, std::format("ERROR adding \"{:g}\" to the sys.path in Python", includePath));
        }

        if (userDefinedPath) {
            EnergyPlus::ShowMessage(state, std::format("Successfully added path \"{:g}\" to the sys.path in Python", includePath));
        }

        // PyRun_SimpleString)("print(' EPS : ' + str(sys.path))");
    }

} // namespace PythonHelpers
} // namespace EnergyPlus

#endif // LINK_WITH_PYTHON
