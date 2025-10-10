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

#include "IdfParser.hh"

namespace EnergyPlus {

std::vector<std::vector<std::string>> IdfParser::decode(std::string const &idf)
{
    bool success = true;
    return decode(idf, success);
}

std::vector<std::vector<std::string>> IdfParser::decode(std::string const &idf, bool &success)
{
    success = true;
    if (idf.empty()) {
        return std::vector<std::vector<std::string>>();
    }

    size_t index = 0;
    return parse_idf(idf, index, success);
}

std::string IdfParser::encode(std::vector<std::vector<std::string>> const &idf_list)
{
    std::string idf;
    for (auto const &object : idf_list) {
        size_t const size = object.size();
        for (size_t i = 0; i < size - 1; ++i) {
            idf += object[i] + ',';
        }
        idf += object[size - 1] + ';' + '\n';
    }
    return idf;
}

std::vector<std::vector<std::string>> IdfParser::parse_idf(std::string const &idf, size_t &index, bool &success)
{
    std::vector<std::vector<std::string>> obj;
    Token token;

    bool done = false;
    while (!done) {
        token = look_ahead(idf, index);
        if (token == Token::END) {
            break;
        } else if (token == Token::NONE) {
            success = false;
            return std::vector<std::vector<std::string>>();
        } else if (token == Token::EXCLAMATION) {
            eat_comment(idf, index);
        } else {

            auto const array = parse_object(idf, index, success);
            if (!array.empty()) {
                obj.push_back(array);
            }
        }
    }

    return obj;
}

std::vector<std::string> IdfParser::parse_object(std::string const &idf, size_t &index, bool &success)
{
    std::vector<std::string> array;
    Token token;

    bool done = false;
    while (!done) {
        token = look_ahead(idf, index);
        if (token == Token::NONE) {
            success = false;
            return std::vector<std::string>();
        } else if (token == Token::COMMA) {
            next_token(idf, index);
            token = look_ahead(idf, index);
            if (Token::EXCLAMATION == token) {
                eat_comment(idf, index);
            }
            token = look_ahead(idf, index);
            if (Token::COMMA == token) {
                array.push_back("");
            } else if (Token::SEMICOLON == token) {
                array.push_back("");
                break;
            }
        } else if (token == Token::SEMICOLON) {
            next_token(idf, index);
            break;
        } else if (token == Token::EXCLAMATION) {
            eat_comment(idf, index);
        } else {
            std::string const value = parse_value(idf, index, success);
            if (!success) {
                return std::vector<std::string>();
            }
            array.push_back(value);
        }
    }

    return array;
}

std::string IdfParser::parse_value(std::string const &idf, size_t &index, bool &success)
{
    switch (look_ahead(idf, index)) {
    case Token::STRING:
        return parse_string(idf, index, success);
    case Token::NONE:
    case Token::END:
    case Token::EXCLAMATION:
    case Token::COMMA:
    case Token::SEMICOLON:
        break;
    default:
        // need to properly fail here at some point for the 'invalid' and 'num' cases
        // adding 'assert(false)' was causing build errors
        break;
    }

    success = false;
    return std::string();
}

std::string IdfParser::parse_string(std::string const &idf, size_t &index, bool &success)
{
    eat_whitespace(idf, index);

    std::string s;
    char c;

    auto const idf_size = idf.size();

    bool complete = false;
    while (!complete) {
        if (index == idf_size) {
            complete = true;
            break;
        }

        c = idf[index++];
        if (c == ',') {
            complete = true;
            index--;
            break;
        } else if (c == ';') {
            complete = true;
            index--;
            break;
        } else if (c == '!') {
            complete = true;
            index--;
            break;
        } else if (c == '\\') {
            if (index == idf_size) {
                break;
            }
            c = idf[index++];
            if (c == '"') {
                s += '"';
            } else if (c == '\\') {
                s += '\\';
            } else if (c == '/') {
                s += '/';
            } else if (c == 'b') {
                s += '\b';
            } else if (c == 't') {
                s += '\t';
            } else if (c == 'n') {
                complete = false;
                break;
            } else if (c == 'r') {
                complete = false;
                break;
            }
        } else {
            s += c;
        }
    }

    if (!complete) {
        success = false;
        return std::string();
    }

    return s;
}

void IdfParser::eat_whitespace(std::string const &idf, size_t &index)
{
    for (; index < idf.size(); index++) {
        switch (idf[index]) {
        case ' ':
        case '\n':
        case '\r':
        case '\t':
            continue;
        default:
            return;
        }
    }
}

void IdfParser::eat_comment(std::string const &idf, size_t &index)
{
    auto const idf_size = idf.size();
    while (true) {
        if (index == idf_size) {
            break;
        }
        if (idf[index++] == '\n') {
            break;
        }
    }
}

IdfParser::Token IdfParser::look_ahead(std::string const &idf, size_t index)
{
    size_t save_index = index;
    return next_token(idf, save_index);
}

IdfParser::Token IdfParser::next_token(std::string const &idf, size_t &index)
{
    eat_whitespace(idf, index);

    if (index == idf.size()) {
        return Token::END;
    }

    char const c = idf[index];
    index++;
    switch (c) {
    case '!':
        return Token::EXCLAMATION;
    case ',':
        return Token::COMMA;
    case ';':
        return Token::SEMICOLON;
    default:
        static constexpr std::string_view search_chars("-:.#/\\[]{}_@$%^&*()|+=<>?'\"~");
        if (isalnum(c) || (std::string::npos != search_chars.find_first_of(c))) {
            return Token::STRING;
        }
        break;
    }
    index--;
    return Token::NONE;
}

} // namespace EnergyPlus
