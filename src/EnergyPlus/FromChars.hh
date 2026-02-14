// EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the University of Illinois,
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

#ifndef FromChars_hh_INCLUDED
#define FromChars_hh_INCLUDED

#if __has_include(<charconv>)
#    include <charconv>
#endif
#include <cstdlib>
#include <limits>
#include <system_error>

namespace FromChars {

struct from_chars_result
{
    const char *ptr;
    std::errc ec;
};

// GCC doesn't have <charconv> until 8.1 so we need to implement an equivalent shim until we move to >=8.1
template <typename T> from_chars_result from_chars(const char *first, const char *last, T &value) noexcept
{
    static_assert(std::is_same_v<T, int> || std::is_same_v<T, long int>, "only int and long int are currently supported");

    from_chars_result answer{};

    while (first != last) {
        if (*first == ' ') {
            ++first;
        } else {
            break;
        }
    }
    if (first == last) {
        answer.ec = std::errc::invalid_argument;
        answer.ptr = first;
        return answer;
    }

#if __has_include(<charconv>)
    auto const result = std::from_chars(first, last, value);
    answer.ptr = result.ptr;
    answer.ec = result.ec;
    return answer;
#else
    errno = 0;
    char *pEnd;
    auto const ret_val = std::strtol(first, &pEnd, 10);

    if (errno == ERANGE) {
        errno = 0;
        answer.ec = std::errc::result_out_of_range;
        answer.ptr = pEnd;
        return answer;
    }
    if (pEnd == first) {
        answer.ec = std::errc::invalid_argument;
        answer.ptr = pEnd;
        return answer;
    }
    if constexpr (std::is_same_v<T, int>) {
        if (ret_val > std::numeric_limits<int>::max()) {
            answer.ec = std::errc::result_out_of_range;
            answer.ptr = pEnd;
            value = 0;
            return answer;
        }
    }

    answer.ptr = pEnd;
    answer.ec = std::errc();
    value = ret_val;
    return answer;
#endif
}

} // namespace FromChars

#endif // FromChars_hh_INCLUDED
