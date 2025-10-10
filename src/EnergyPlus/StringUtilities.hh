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

#ifndef StringUtilities_hh_INCLUDED
#define StringUtilities_hh_INCLUDED

#include <EnergyPlus/FromChars.hh>
#include <ObjexxFCL/src/ObjexxFCL/Array1S.hh>
#include <fast_float/fast_float.h>
#include <sstream>
#include <type_traits>

namespace EnergyPlus {
inline std::stringstream stringReader(std::string_view str)
{
    std::stringstream result{str.data()};
    // result.imbue(std::locale("C"));
    return result;
}

template <class T, typename = std::enable_if_t<std::is_same_v<T, double> || std::is_same_v<T, int>>>
bool readListItem(std::string_view input, size_t &index, T &param)
{
    if (index >= input.size()) {
        return false;
    }

    input.remove_prefix(index);

    auto first_comma = input.find_first_not_of(',');
    if (first_comma != std::string_view::npos) {
        input.remove_prefix(first_comma);
    }

    auto first_char = input.find_first_not_of(' ');
    if (first_char != std::string_view::npos) {
        input.remove_prefix(first_char);
    }

    FromChars::from_chars_result result{};
    if constexpr (std::is_same_v<T, int>) {
        result = FromChars::from_chars(input.data(), input.data() + input.size(), param);
    } else if constexpr (std::is_same_v<T, double>) {
        auto fast_float_result = fast_float::from_chars(input.data(), input.data() + input.size(), param);
        result.ptr = fast_float_result.ptr;
        result.ec = fast_float_result.ec;
    }
    if (result.ec == std::errc::result_out_of_range || result.ec == std::errc::invalid_argument) {
        return false;
    }
    size_t pos = result.ptr - input.data();
    index += (pos + first_comma + first_char);
    return true;
}

template <typename Param> bool readListItem(std::istream &stream, Param &&param)
{
    if (stream.good()) {
        stream >> param;
        if (stream.good() && stream.peek() == ',') {
            stream.get(); // eat comma
        }
    } else {
        // the stream was not good, it was, perhaps EOF. So we need to
        // set the fail bit here to say that we ran out of string
        // and were unable to read the next bit of data
        stream.setstate(std::ios_base::failbit);
    }

    return !stream.fail();
}

template <typename Param> bool readItem(std::string_view input, Param &&param)
{
    auto stream = stringReader(input);
    stream >> param;
    return !stream.fail() && stream.eof();
}

inline auto nth_occurrence(std::string_view input_str, std::string_view search_str, std::size_t Nth)
{
    if (Nth == 0) {
        return std::string_view::npos;
    }
    std::string_view::size_type pos = 0;
    size_t cnt{};

    do {
        pos = input_str.find(search_str, pos);
        if (pos == std::string_view::npos) {
            return std::string_view::npos;
        }
        ++cnt;
        ++pos;
    } while (cnt != Nth);

    return pos;
}

// Returns the position **after** the nth occurrence of the search_char, eg:
// eg: Given a string test="aa,bb,cc", nth_occurrence(test, ',', x), for x = 1 it gives 3, and for x = 2 it gives 6
inline auto nth_occurrence(std::string_view input_str, char const search_char, std::size_t Nth)
{
    if (Nth == 0) {
        return std::string_view::npos;
    }
    std::string_view::size_type pos = 0;
    size_t cnt{};

    do {
        pos = input_str.find(search_char, pos);
        if (pos == std::string_view::npos) {
            return std::string_view::npos;
        }
        ++cnt;
        ++pos;
    } while (cnt != Nth);

    return pos;
}

template <typename... Param> bool readList(std::string_view input, Param &&...param)
{
    if constexpr (std::conjunction_v<std::is_same<double &, Param>...> || std::conjunction_v<std::is_same<int &, Param>...>) {
        size_t index = 0;
        return ((readListItem(input, index, param)), ...);
    } else {
        auto reader = stringReader(input);
        return ((readListItem(reader, param)), ...);
    }
}

} // namespace EnergyPlus

namespace ObjexxFCL {
// since this is a slice (reference) we want to bind to temporaries
// so we're going to allow that where the one provided by Array1S.hh does not
template <typename T> std::istream &operator>>(std::istream &stream, Array1S<T> &&a)
{
    // just pass on to the `&` version of this operator>>
    return stream >> a;
}
} // namespace ObjexxFCL

#endif
