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

#ifndef EPLUS_PYTHON_HELPERS_HH
#define EPLUS_PYTHON_HELPERS_HH

#if !LINK_WITH_PYTHON
#    error "This file should only be included when LINK_WITH_PYTHON is defined"
#endif

// Third Party Headers
#ifdef _DEBUG
// We don't want to try to import a debug build of Python here
// so if we are building a Debug build of the C++ code, we need
// to undefine _DEBUG during the #include command for Python.h.
// Otherwise it will fail
#    undef _DEBUG
#    include <Python.h>
#    define _DEBUG
#else
#    include <Python.h>
#endif

// C++ Headers
#if DEBUG_PYTHON_CONFIG
#    include <cwchar>
#endif
#include <format>

#include <EnergyPlus/Formatters.hh>

template <> struct std::formatter<PyStatus>
{
    // parse is inherited from formatter<string_view>.
    constexpr auto parse(std::format_parse_context &ctx) -> std::format_parse_context::iterator
    {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(const PyStatus &status, FormatContext &ctx) const
    {
        if (PyStatus_Exception(status) == 0) {
            return ctx.out();
        }
        if (PyStatus_IsExit(status) != 0) {
            return std::format_to(ctx.out(), "Exited with code {}", status.exitcode);
        }
        if (PyStatus_IsError(status) != 0) {
            auto it = ctx.out();
            it = std::format_to(it, "Fatal Python error: ");
            if (status.func != nullptr) {
                it = std::format_to(it, "{}: ", status.func);
            }
            it = std::format_to(it, "{}", status.err_msg);
            return it;
        }
        return ctx.out();
    }
};

#if DEBUG_PYTHON_CONFIG
static std::string narrowWide(const wchar_t *ws)
{
    if (ws == nullptr) {
        return "(null)";
    }
    std::mbstate_t state{};
    const wchar_t *tmp = ws;
#    ifndef NDEBUG
    // LC_TYPE is set to C.UTF-8 in PyPreConfig_InitPythonConfig already
    const char *ctype = std::setlocale(LC_CTYPE, nullptr);
    assert(ctype != nullptr && (std::strstr(ctype, "UTF-8") != nullptr || std::strstr(ctype, "utf8") != nullptr));
#    endif

    std::size_t n = std::wcsrtombs(nullptr, &tmp, 0, &state);
    if (n == static_cast<std::size_t>(-1)) {
        return "(conversion error)";
    }
    std::string result(n, '\0');
    n = std::wcsrtombs(result.data(), &ws, n, &state);
    if (n == static_cast<std::size_t>(-1)) {
        return "(conversion error)";
    }
    return result;
}

template <> struct std::formatter<PyPreConfig>
{
    constexpr auto parse(std::format_parse_context &ctx) -> std::format_parse_context::iterator
    {
        return ctx.begin();
    }

    auto format(const PyPreConfig &c, std::format_context &ctx) const -> std::format_context::iterator
    {
        auto it = ctx.out();
        it = std::format_to(it, "PyPreConfig(\n");
        it = std::format_to(it, "  parse_argv={}\n", c.parse_argv);
        it = std::format_to(it, "  isolated={}\n", c.isolated);
        it = std::format_to(it, "  use_environment={}\n", c.use_environment);
        it = std::format_to(it, "  configure_locale={}\n", c.configure_locale);
        it = std::format_to(it, "  coerce_c_locale={}\n", c.coerce_c_locale);
        it = std::format_to(it, "  coerce_c_locale_warn={}\n", c.coerce_c_locale_warn);
        it = std::format_to(it, "  utf8_mode={}\n", c.utf8_mode);
        it = std::format_to(it, "  dev_mode={}\n", c.dev_mode);
        it = std::format_to(it, "  allocator={}\n", c.allocator);
#    ifdef MS_WINDOWS
        it = std::format_to(it, "  legacy_windows_fs_encoding={}\n", c.legacy_windows_fs_encoding);
#    endif
        it = std::format_to(it, ")");
        return it;
    }
};

template <> struct std::formatter<PyConfig>
{
    constexpr auto parse(std::format_parse_context &ctx)
    {
        return ctx.begin();
    }

    std::format_context::iterator format(const PyConfig &c, std::format_context &ctx) const
    {
        auto it = ctx.out();

        it = std::format_to(it, "PyConfig(\n");
        it = std::format_to(it, "  isolated={}\n", c.isolated);
        it = std::format_to(it, "  use_environment={}\n", c.use_environment);
        it = std::format_to(it, "  site_import={}\n", c.site_import);
        it = std::format_to(it, "  program_name={}\n", narrowWide(c.program_name));
        it = std::format_to(it, "  home={}\n", narrowWide(c.home));
        it = std::format_to(it, "  base_prefix={}\n", narrowWide(c.base_prefix));
        it = std::format_to(it, "  prefix={}\n", narrowWide(c.prefix));
        it = std::format_to(it, "  exec_prefix={}\n", narrowWide(c.exec_prefix));
        it = std::format_to(it, "  base_exec_prefix={}\n", narrowWide(c.base_exec_prefix));
        it = std::format_to(it, "  pythonpath_env={}\n", narrowWide(c.pythonpath_env));
        it = std::format_to(it, "  executable={}\n", narrowWide(c.executable));

        it = std::format_to(
            it, "  module_search_paths_set={}, module_search_paths.length={}\n", c.module_search_paths_set, c.module_search_paths.length);
        if (c.module_search_paths.items != nullptr) {
            for (Py_ssize_t i = 0; i < c.module_search_paths.length; ++i) {
                it = std::format_to(it, "    module_search_paths[{}]={}\n", i, narrowWide(c.module_search_paths.items[i]));
            }
        }
        it = std::format_to(it, ")");
        return it;
    }
};
#endif // DEBUG_PYTHON_CONFIG

namespace EnergyPlus {

namespace PythonHelpers {

    void initPython(EnergyPlus::EnergyPlusData &state, fs::path const &programDir);

    void reportPythonError(EnergyPlus::EnergyPlusData &state);

    void addToPythonPath(EnergyPlusData &state, const fs::path &includePath, bool userDefinedPath);

} // namespace PythonHelpers
} // namespace EnergyPlus

#endif // EPLUS_PYTHON_HELPERS_HH
