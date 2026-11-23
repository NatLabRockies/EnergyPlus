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

#ifndef Formatters_hh_INCLUDED
#define Formatters_hh_INCLUDED

#include <concepts>
#include <cstdio>
#include <format>
#include <ranges>
#include <string>
#ifdef __cpp_lib_print
#    include <print>
#endif

#if 0
#    define DO_PRAGMA_FORMATTERS(x) _Pragma(#x)
#    define DEBUG_PRAGMA(msg) DO_PRAGMA_FORMATTERS(message(#msg))
#else
#    define DEBUG_PRAGMA(msg)
#endif

// ===============================  EnergyPlus::formattable  =================================
namespace EnergyPlus {
#if __cplusplus >= 202302L
// #error "std::formattable is defined, remove C++ 20 implementation"
DEBUG_PRAGMA("Using std::formattable")
using std::formattable;
#else
template <typename T, typename CharT = char>
concept formattable =
    requires(std::remove_reference_t<T> &v, std::basic_format_context<std::back_insert_iterator<std::basic_string<CharT>>, CharT> ctx) {
        std::formatter<std::remove_reference_t<T>, CharT>{}.format(v, ctx);
        std::formatter<std::remove_reference_t<T>, CharT>{}.parse(std::declval<std::basic_format_parse_context<CharT> &>());
    };
#endif
} // namespace EnergyPlus

// ===============================  formatting ranges  =================================
#if __cpp_lib_format_ranges >= 202207L
DEBUG_PRAGMA("Formatting ranges is built-in")
#else
template <typename T>
concept set_like = std::ranges::range<T> && requires { typename T::key_type; };

template <typename T>
concept formattable_range =
    // A range
    std::ranges::range<T>
    // A range of formattable elements. It'd be a nicer error message to include
    // it, but that prevents using it for a range of range eg :
    // std::vector<std::vector<int>>.
    // && EnergyPlus::formattable<std::ranges::range_value_t<T>, char>
    // But we exclude the ones that are already formattable as a whole, like
    // std::string and std::string_view, char*.
    && !std::is_same_v<T, std::string> && !std::is_same_v<T, std::string_view> && !std::is_same_v<std::ranges::range_value_t<T>, char>;

template <formattable_range Container> struct std::formatter<Container>
{
    using T = std::ranges::range_value_t<Container>;
    std::formatter<T> element_formatter;
    bool no_brackets = false;

    constexpr auto parse(std::format_parse_context &ctx) -> std::format_parse_context::iterator
    {
        auto it = ctx.begin();
        // Accept the standard range-format-spec's 'n' range-type, which suppresses the
        // surrounding brackets, eg "{:n:.2f}" for a range of double.
        if (it != ctx.end() && *it == 'n') {
            no_brackets = true;
            ctx.advance_to(++it);
        }
        // Accept the optional ':' that separates the range-spec from the underlying
        // element-spec, eg "{::.2f}" or "{:n:.2f}" for a range of double.
        if (it != ctx.end() && *it == ':') {
            ctx.advance_to(++it);
        }
        return element_formatter.parse(ctx);
    }

    template <typename FormatContext> auto format(const Container &v, FormatContext &ctx) const
    {
        constexpr char open = set_like<Container> ? '{' : '[';
        constexpr char close = set_like<Container> ? '}' : ']';
        constexpr bool quoted = std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>;

        auto it = ctx.out();
        if (!no_brackets) {
            *it++ = open;
        }
        for (bool first = true; const auto &elem : v) {
            if (!first) {
                it = std::format_to(it, ", ");
            }
            first = false;
            if constexpr (quoted) {
                it = std::format_to(it, "\"");
                it = element_formatter.format(elem, ctx);
                it = std::format_to(it, "\"");
            } else {
                it = element_formatter.format(elem, ctx);
            }
        }
        if (!no_brackets) {
            *it++ = close;
        }
        return it;
    }
};
#endif

// ===============================  EnergyPlus::print  =================================
namespace EnergyPlus {
#ifdef __cpp_lib_print

DEBUG_PRAGMA("__cpp_lib_print is defined, using std::print")
using std::print;

#else

template <class... Args> void print(FILE *stream, std::format_string<Args...> fmt, Args &&...args)
{
    std::string formatted = std::format(fmt, std::forward<Args>(args)...);
    if (std::fwrite(formatted.data(), 1, formatted.size(), stream) != formatted.size()) {
        throw std::system_error(EIO, std::generic_category());
    }
}
template <class... Args> void print(std::format_string<Args...> fmt, Args &&...args)
{
    print(stdout, fmt, std::forward<Args>(args)...);
}
#endif
} // namespace EnergyPlus

// ===============================  EnergyPlus::join  =================================
namespace detail {
template <typename R> struct format_join_view
{
    const R &range;
    std::string_view sep;
};
} // namespace detail

template <typename R> struct std::formatter<::detail::format_join_view<R>>
{
    std::formatter<std::ranges::range_value_t<R>> element_formatter;

    constexpr auto parse(std::format_parse_context &ctx) -> std::format_parse_context::iterator
    {
        return element_formatter.parse(ctx);
    }

    template <typename FormatContext> auto format(const ::detail::format_join_view<R> &jv, FormatContext &ctx) const
    {
        auto it = ctx.out();
        bool first = true;
        for (const auto &elem : jv.range) {
            if (!first) {
                it = std::ranges::copy(jv.sep, it).out;
            }
            first = false;
            it = element_formatter.format(elem, ctx);
        }
        return it;
    }
};

namespace EnergyPlus {

// Usage: std::format("{:.4f}", EnergyPlus::join(range, "|"))
template <typename R> auto join(const R &r, std::string_view sep)
{
    return ::detail::format_join_view<R>{r, sep};
}

} // namespace EnergyPlus

#endif
