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

#ifndef InputProcessing_CsvParser_hh_INCLUDED
#define InputProcessing_CsvParser_hh_INCLUDED

#include <string>
#include <string_view>

#include <nlohmann/json.hpp>
#include <unordered_map>

namespace EnergyPlus {
class InputProcessorFixture;
}

class CsvParser
{
public:
    friend class EnergyPlus::InputProcessorFixture;
    using json = nlohmann::json;

    CsvParser() = default;

    // Skipping 1 row is assumed to be the header row and will be recorded as such.
    // Otherwise, if rows_to_skip == 0 or >1 then there will be no header
    json decode(std::string_view csv, char t_delimiter = ',', int t_rows_to_skip = 0);

    std::vector<std::pair<std::string, bool>> const &errors();
    std::vector<std::pair<std::string, bool>> const &warnings();

    bool hasErrors();
    bool hasWarnings();

    enum class Token : size_t
    {
        NONE,
        FILE_END,
        DELIMITER,
        LINE_END,
        VALUE,
        Num
    };

private:
    bool success = false;
    size_t cur_line_num = 1;
    size_t index_into_cur_line = 0;
    size_t beginning_of_line_index = 0;
    size_t csv_size = 0;
    char delimiter = ',';
    int rows_to_skip = 0;
    char s[129] = {};
    std::vector<std::pair<std::string, bool>> errors_;   // the boolean is for continuing lines
    std::vector<std::pair<std::string, bool>> warnings_; // the boolean is for continuing lines

    static void increment_both_index(size_t &index, size_t &line_index);

    static void decrement_both_index(size_t &index, size_t &line_index);

    void skip_rows(std::string_view csv, size_t &index);

    int find_number_columns(std::string_view csv, size_t &index);

    json parse_csv(std::string_view csv, size_t &index);

    void parse_header(std::string_view csv, size_t &index, json &header);

    void parse_line(std::string_view csv, size_t &index, json &columns);

    json parse_value(std::string_view csv, size_t &index);

    //            json parse_number(std::string_view csv, size_t &index);

    //            std::string parse_string(std::string_view csv, size_t &index);

    Token look_ahead(std::string_view csv, size_t index);

    Token next_token(std::string_view csv, size_t &index);

    static std::string_view rtrim(std::string_view str);

    void eat_whitespace(std::string_view csv, size_t &index);

    static inline std::string convertToUpper(std::string str)
    {
        size_t len = str.size();
        for (size_t i = 0; i < len; ++i) {
            char c = str[i];
            str[i] = ('a' <= c && c <= 'z') ? c ^ 0x20 : c; // ASCII only, which is fine
        }
        return str;
    }
};

#endif // InputProcessing_CsvParser_hh_INCLUDED
