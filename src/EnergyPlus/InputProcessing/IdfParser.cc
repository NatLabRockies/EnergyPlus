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

#include <EnergyPlus/FromChars.hh>
#include <EnergyPlus/InputProcessing/IdfParser.hh>
#include <cmath>
#include <fast_float/fast_float.h>
#include <fmt/format.h>
#include <milo/dtoa.h>
#include <milo/itoa.h>

using json = nlohmann::json;

auto const icompare = [](std::string_view a, std::string_view b) { // (AUTO_OK)
    return (a.length() == b.length()
                ? std::equal(a.begin(), a.end(), b.begin(), [](char const c, char const d) { return (::tolower(c) == ::tolower(d)); })
                : false);
};

json IdfParser::decode(std::string_view idf, json const &schema)
{
    bool success = true;
    return decode(idf, idf.size(), schema, success);
}

json IdfParser::decode(std::string_view idf, json const &schema, bool &success)
{
    return decode(idf, idf.size(), schema, success);
}

json IdfParser::decode(std::string_view idf, size_t _idf_size, json const &schema)
{
    bool success = true;
    return decode(idf, _idf_size, schema, success);
}

json IdfParser::decode(std::string_view idf, size_t _idf_size, json const &schema, bool &success)
{
    success = true;
    cur_line_num = 1;
    index_into_cur_line = 0;
    beginning_of_line_index = 0;
    idf_size = _idf_size;

    if (idf.empty()) {
        success = false;
        return nullptr;
    }

    size_t index = 0;
    return parse_idf(idf, index, success, schema);
}

std::string IdfParser::encode(json const &root, json const &schema)
{
    static constexpr std::string_view end_of_field(",\n  ", 4);
    static constexpr std::string_view end_of_object(";\n\n", 3);

    std::string encoded, extension_key;
    if (idf_size > 0) {
        encoded.reserve(idf_size);
    } else {
        encoded.reserve(root.size() * 1024);
    }

    for (auto obj = root.begin(); obj != root.end(); ++obj) {
        const auto &legacy_idd = schema["properties"][obj.key()]["legacy_idd"];
        const auto &legacy_idd_field = legacy_idd["fields"];
        auto key = legacy_idd.find("extension");
        if (key != legacy_idd.end()) {
            extension_key = key.value().get<std::string>();
        }
        for (auto obj_in = obj.value().begin(); obj_in != obj.value().end(); ++obj_in) {
            encoded += obj.key();
            size_t skipped_fields = 0;
            for (size_t i = 0; i < legacy_idd_field.size(); i++) {
                std::string const &entry = legacy_idd_field[i].get<std::string>();
                if (obj_in.value().find(entry) == obj_in.value().end()) {
                    if (entry == "name") {
                        encoded += std::string{end_of_field} + obj_in.key();
                    } else {
                        skipped_fields++;
                    }
                    continue;
                }
                for (size_t j = 0; j < skipped_fields; j++) {
                    encoded += end_of_field;
                }
                skipped_fields = 0;
                encoded += end_of_field;
                auto const &val = obj_in.value()[entry];
                if (val.is_string()) {
                    encoded += val.get<std::string>();
                } else if (val.is_number_integer()) {
                    encoded += std::to_string(val.get<int>());
                } else {
                    dtoa(val.get<double>(), s);
                    encoded += s;
                }
            }

            if (obj_in.value().find(extension_key) == obj_in.value().end()) {
                encoded += end_of_object;
                continue;
            }

            auto &extensions = obj_in.value()[extension_key];
            for (size_t extension_i = 0; extension_i < extensions.size(); extension_i++) {
                auto const &cur_extension_obj = extensions[extension_i];
                auto const &extensible = schema["properties"][obj.key()]["legacy_idd"]["extensibles"];
                for (size_t i = 0; i < extensible.size(); i++) {
                    std::string const &tmp = extensible[i].get<std::string>();
                    if (cur_extension_obj.find(tmp) == cur_extension_obj.end()) {
                        skipped_fields++;
                        continue;
                    }
                    for (size_t j = 0; j < skipped_fields; j++) {
                        encoded += end_of_field;
                    }
                    skipped_fields = 0;
                    encoded += end_of_field;
                    if (cur_extension_obj[tmp].is_string()) {
                        encoded += cur_extension_obj[tmp].get<std::string>();
                    } else {
                        dtoa(cur_extension_obj[tmp].get<double>(), s);
                        encoded += s;
                    }
                }
            }
            encoded += end_of_object;
        }
    }
    return encoded;
}

std::string IdfParser::normalizeObjectType(std::string const &objectType)
{
    if (objectType.empty()) {
        return std::string{};
    }
    std::string key = convertToUpper(objectType);
    auto tmp_umit = objectTypeMap.find(key);
    if (tmp_umit != objectTypeMap.end()) {
        return tmp_umit->second;
    }
    return std::string{};
}

std::vector<std::string> const &IdfParser::errors()
{
    return errors_;
}

std::vector<std::string> const &IdfParser::warnings()
{
    return warnings_;
}

bool IdfParser::hasErrors()
{
    return !errors_.empty();
}

json IdfParser::parse_idf(std::string_view idf, size_t &index, bool &success, json const &schema)
{
    json root;
    Token token;
    auto const &schema_properties = schema["properties"];

    objectTypeMap.reserve(schema_properties.size());
    for (auto it = schema_properties.begin(); it != schema_properties.end(); ++it) {
        std::string key = convertToUpper(it.key());
        objectTypeMap.emplace(std::move(key), it.key());
    }

    if (idf_size > 3) {
        // UTF-8 Byte Order Mark
        if (idf[0] == '\xEF' && idf[1] == '\xBB' && idf[2] == '\xBF') {
            index += 3;
            index_into_cur_line += 3;
        }
    }

    int idfObjectCount = 0;
    while (true) {
        token = look_ahead(idf, index);
        if (token == Token::END) {
            break;
        } else if (token == Token::NONE) {
            success = false;
            return root;
        } else if (token == Token::SEMICOLON) {
            next_token(idf, index);
            continue;
        } else if (token == Token::COMMA) {
            errors_.emplace_back(fmt::format("Line: {} Index: {} - Extraneous comma found.", cur_line_num, index_into_cur_line));
            success = false;
            return root;
        } else if (token == Token::EXCLAMATION) {
            eat_comment(idf, index);
        } else {
            ++idfObjectCount;
            std::string const parsed_obj_name = parse_string(idf, index);
            std::string const obj_name = normalizeObjectType(parsed_obj_name);
            if (obj_name.empty()) {
                errors_.emplace_back(
                    fmt::format("Line: {} Index: {} - \"{}\" is not a valid Object Type.", cur_line_num, index_into_cur_line, parsed_obj_name));
                while (token != Token::SEMICOLON && token != Token::END) {
                    token = next_token(idf, index);
                }
                continue;
            }

            bool object_success = true;
            json const &obj_loc = schema_properties[obj_name];
            json const &legacy_idd = obj_loc["legacy_idd"];
            json obj = parse_object(idf, index, object_success, legacy_idd, obj_loc, idfObjectCount);
            if (!object_success) {
                auto found_index = idf.find_first_of('\n', beginning_of_line_index);
                std::string line;
                if (found_index != std::string::npos) {
                    line = idf.substr(beginning_of_line_index, found_index - beginning_of_line_index - 1);
                }
                errors_.emplace_back(
                    fmt::format("Line: {} Index: {} - Error parsing \"{}\". Error in following line.", cur_line_num, index_into_cur_line, obj_name));
                errors_.emplace_back(fmt::format("~~~ {}", line));
                success = false;
                continue;
            }
            u64toa(root[obj_name].size() + 1, s);
            std::string name = fmt::format("{} {}", obj_name, s);

            if (!obj.is_null()) {
                auto const name_iter = obj.find("name");
                // If you find a name field, use that
                if (name_iter != obj.end()) {
                    name = name_iter.value().get<std::string>();
                    obj.erase(name_iter);
                } else {
                    // Otherwise, see if it should have a name field
                    auto const it = obj_loc.find("name");
                    if (it != obj_loc.end()) {
                        // Let it slide, as a blank string, to be handled in the appropriate GetInput routine
                        name = "";
                    }
                }
            }

            if (root[obj_name].find(name) != root[obj_name].end()) {
                errors_.emplace_back(
                    fmt::format(R"(Duplicate name found for object of type "{}" named "{}". Overwriting existing object.)", obj_name, name));
            }

            root[obj_name][name] = std::move(obj);
        }
    }

    return root;
}

json IdfParser::parse_object(
    std::string_view idf, size_t &index, bool &success, json const &legacy_idd, json const &schema_obj_loc, int idfObjectCount)
{
    json root = json::object();
    json extensible = json::object();
    json array_of_extensions = json::array();
    Token token;
    std::string extension_key;
    size_t legacy_idd_index = 0;
    size_t extensible_index = 0;
    success = true;
    bool was_value_parsed = false;
    auto const &legacy_idd_fields_array = legacy_idd["fields"];
    auto const legacy_idd_extensibles_iter = legacy_idd.find("extensibles");

    auto const &schema_patternProperties = schema_obj_loc["patternProperties"];
    std::string patternProperty;
    int dot_star_present = schema_patternProperties.count(".*");
    int no_whitespace_present = schema_patternProperties.count(R"(^.*\S.*$)");
    if (dot_star_present) {
        patternProperty = ".*";
    } else if (no_whitespace_present) {
        patternProperty = R"(^.*\S.*$)";
    } else {
        throw std::runtime_error(R"(The patternProperties value is not a valid choice (".*", "^.*\S.*$"))");
    }
    auto const &schema_obj_props = schema_patternProperties[patternProperty]["properties"];
    auto key = legacy_idd.find("extension");

    json const *schema_obj_extensions = nullptr;
    if (legacy_idd_extensibles_iter != legacy_idd.end()) {
        if (key == legacy_idd.end()) {
            errors_.emplace_back("\"extension\" key not found in schema. Need to add to list in modify_schema.py.");
            success = false;
            return root;
        }
        extension_key = key.value().get<std::string>();
        schema_obj_extensions = &schema_obj_props[extension_key]["items"]["properties"];
    }

    root["idf_order"] = idfObjectCount;

    auto const found_min_fields = schema_obj_loc.find("min_fields");

    index += 1;

    while (true) {
        token = look_ahead(idf, index);
        root["idf_max_fields"] = legacy_idd_index;
        root["idf_max_extensible_fields"] = extensible_index;
        if (token == Token::NONE) {
            success = false;
            return root;
        } else if (token == Token::END) {
            return root;
        } else if (token == Token::COMMA || token == Token::SEMICOLON) {
            if (!was_value_parsed) {
                int ext_size = 0;
                if (legacy_idd_index < legacy_idd_fields_array.size()) {
                    //                    std::string_view  field_name = legacy_idd_fields_array[ legacy_idd_index ];
                    //                    root[ field_name ] = "";
                } else {
                    auto const &legacy_idd_extensibles_array = legacy_idd_extensibles_iter.value();
                    ext_size = static_cast<int>(legacy_idd_extensibles_array.size());
                    //                    std::string_view  field_name = legacy_idd_extensibles_array[ extensible_index % ext_size ];
                    extensible_index++;
                    //                    extensible[ field_name ] = "";
                }
                if (ext_size && extensible_index % ext_size == 0) {
                    array_of_extensions.push_back(extensible);
                    extensible.clear();
                }
            }
            legacy_idd_index++;
            was_value_parsed = false;
            next_token(idf, index);
            if (token == Token::SEMICOLON) {
                size_t min_fields = 0;
                if (found_min_fields != schema_obj_loc.end()) {
                    min_fields = found_min_fields.value().get<size_t>();
                }
                for (; legacy_idd_index < min_fields; legacy_idd_index++) {
                    //                    std::string_view  field_name = legacy_idd_fields_array[ legacy_idd_index ];
                    //                    root[ field_name ] = "";
                }
                if (!extensible.empty()) {
                    array_of_extensions.push_back(extensible);
                    extensible.clear();
                }
                root["idf_max_fields"] = legacy_idd_index;
                root["idf_max_extensible_fields"] = extensible_index;
                break;
            }
        } else if (token == Token::EXCLAMATION) {
            eat_comment(idf, index);
        } else if (legacy_idd_index >= legacy_idd_fields_array.size()) {
            if (legacy_idd_extensibles_iter == legacy_idd.end()) {
                errors_.emplace_back(
                    fmt::format("Line: {} Index: {} - Object contains more field values than maximum number of IDD fields and is not extensible.",
                                cur_line_num,
                                index_into_cur_line));
                success = false;
                return root;
            }
            if (schema_obj_extensions == nullptr) {
                errors_.emplace_back(fmt::format("Line: {} Index: {} - Object does not have extensible fields but should. Likely a parsing error.",
                                                 cur_line_num,
                                                 index_into_cur_line));
                success = false;
                return root;
            }
            auto const &legacy_idd_extensibles_array = legacy_idd_extensibles_iter.value();
            size_t const size = legacy_idd_extensibles_array.size();
            std::string const &field_name = legacy_idd_extensibles_array[extensible_index % size].get<std::string>();
            json val = parse_value(idf, index, success, schema_obj_extensions->at(field_name));
            if (!success) {
                return root;
            }
            extensible[field_name] = std::move(val);
            was_value_parsed = true;
            extensible_index++;
            if (extensible_index && extensible_index % size == 0) {
                array_of_extensions.push_back(extensible);
                extensible.clear();
            }
        } else {
            was_value_parsed = true;
            std::string const &field = legacy_idd_fields_array[legacy_idd_index].get<std::string>();
            auto const find_field_iter = schema_obj_props.find(field);
            if (find_field_iter == schema_obj_props.end()) {
                if (field == "name") {
                    root[field] = parse_string(idf, index);
                } else {
                    u64toa(cur_line_num, s);
                    errors_.emplace_back(fmt::format("Line: {} - Field \"{}\" was not found.", s, field));
                }
            } else {
                json val = parse_value(idf, index, success, find_field_iter.value());
                if (!success) {
                    return root;
                }
                root[field] = std::move(val);
            }
            if (!success) {
                return root;
            }
        }
    }
    if (!array_of_extensions.empty()) {
        root[extension_key] = std::move(array_of_extensions);
        array_of_extensions = nullptr;
    }
    return root;
}

json IdfParser::parse_number(std::string_view idf, size_t &index)
{
    eat_whitespace(idf, index);

    size_t save_i = index;

    bool running = true;
    while (running) {
        if (save_i == idf_size) {
            break;
        }

        char const c = idf[save_i];
        switch (c) {
        case '!':
        case ',':
        case ';':
        case '\r':
        case '\n':
            running = false;
            break;
        default:
            ++save_i;
        }
    }

    size_t diff = save_i - index;
    std::string_view value = idf.substr(index, diff);
    index_into_cur_line += diff;
    index = save_i;

    auto const convert_double = [&index, this](std::string_view str) -> json { // (AUTO_OK)
        size_t plus_sign = 0;
        if (str.front() == '+') {
            plus_sign = 1;
        }
        auto const str_end = str.data() + str.size(); // have to do this for MSVC // (AUTO_OK)
        double val;
        auto result = fast_float::from_chars(str.data() + plus_sign, str.data() + str.size(), val); // (AUTO_OK)
        if (result.ec == std::errc::invalid_argument || result.ec == std::errc::result_out_of_range) {
            return rtrim(str);
        } else if (result.ptr != str_end) {
            auto const initial_ptr = result.ptr; // (AUTO_OK)
            while (result.ptr != str_end) {
                if (*result.ptr != ' ') {
                    break;
                }
                ++result.ptr;
            }
            if (result.ptr == str_end) {
                index -= (str_end - initial_ptr);
                this->index_into_cur_line -= (str_end - initial_ptr);
                return val;
            }
            return rtrim(str);
        }
        return val;
    };

    auto const convert_int = [&convert_double, &index, this](std::string_view str) -> json { // (AUTO_OK)
        auto const str_end = str.data() + str.size();                                        // have to do this for MSVC // (AUTO_OK)
        int val;
        auto result = FromChars::from_chars(str.data(), str.data() + str.size(), val); // (AUTO_OK)
        if (result.ec == std::errc::result_out_of_range || result.ec == std::errc::invalid_argument) {
            return convert_double(str);
        } else if (result.ptr != str_end) {
            if (*result.ptr == '.' || *result.ptr == 'e' || *result.ptr == 'E') {
                return convert_double(str);
            } else {
                auto const initial_ptr = result.ptr; // (AUTO_OK)
                while (result.ptr != str_end) {
                    if (*result.ptr != ' ') {
                        break;
                    }
                    ++result.ptr;
                }
                if (result.ptr == str_end) {
                    index -= (str_end - initial_ptr);
                    this->index_into_cur_line -= (str_end - initial_ptr);
                    return val;
                }
                return rtrim(str);
            }
        }
        return val;
    };

    return convert_int(value);
}

json IdfParser::parse_integer(std::string_view idf, size_t &index)
{
    eat_whitespace(idf, index);

    size_t save_i = index;

    bool running = true;
    while (running) {
        if (save_i == idf_size) {
            break;
        }

        char const c = idf[save_i];
        switch (c) {
        case '!':
        case ',':
        case ';':
        case '\r':
        case '\n':
            running = false;
            break;
        default:
            ++save_i;
        }
    }

    size_t diff = save_i - index;
    std::string_view string_value = idf.substr(index, diff);
    index_into_cur_line += diff;
    index = save_i;

    auto const string_end = string_value.data() + string_value.size(); // have to do this for MSVC // (AUTO_OK)
    int int_value;
    // Try using from_chars
    auto result = FromChars::from_chars(string_value.data(), string_value.data() + string_value.size(), int_value); // (AUTO_OK)
    if (result.ec == std::errc::result_out_of_range || result.ec == std::errc::invalid_argument) {
        // Failure, return the string
        return rtrim(string_value);
    } else if (result.ptr != string_end) {
        // Didn't use the entire string, try again via double conversion + rounding
        size_t plus_sign = 0;
        if (string_value.front() == '+') {
            plus_sign = 1;
        }
        double double_value;
        auto fresult = fast_float::from_chars(string_value.data() + plus_sign, string_value.data() + string_value.size(), double_value); // (AUTO_OK)
        if (fresult.ec == std::errc::invalid_argument || fresult.ec == std::errc::result_out_of_range) {
            // Failure, return the string
            return rtrim(string_value);
        }
        int_value = static_cast<int>(std::round(double_value));
    }
    return int_value;
}

json IdfParser::parse_value(std::string_view idf, size_t &index, bool &success, json const &field_loc)
{
    Token token;
    auto const &field_type = field_loc.find("type");
    if (field_type != field_loc.end()) {
        if (field_type.value() == "number") {
            token = Token::NUMBER;
        } else if (field_type.value() == "integer") {
            token = Token::INTEGER;
        } else {
            token = Token::STRING;
        }
    } else {
        token = look_ahead(idf, index);
    }

    switch (token) {
    case Token::STRING: {
        std::string const parsed_string = parse_string(idf, index);
        auto const enum_it = field_loc.find("enum");
        if (enum_it != field_loc.end()) {
            for (auto const &enum_str : enum_it.value()) {
                std::string const str = enum_str.get<std::string>();
                if (icompare(str, parsed_string)) {
                    return str;
                }
            }
        } else if (icompare(parsed_string, "Autosize") || icompare(parsed_string, "Autocalculate")) {
            auto const default_it = field_loc.find("default");
            auto const anyOf_it = field_loc.find("anyOf");

            if (anyOf_it == field_loc.end()) {
                errors_.emplace_back(
                    fmt::format("Line: {} Index: {} - Field cannot be Autosize or Autocalculate", cur_line_num, index_into_cur_line));
                return parsed_string;
            }
            // The following is hacky because it abuses knowing the consistent generated structure
            // in the future this might not hold true for the array indexes.
            if (default_it != field_loc.end()) {
                return field_loc.at("anyOf")[1]["enum"][1];
            } else {
                return field_loc.at("anyOf")[1]["enum"][0];
            }
        }
        return parsed_string;
    }
    case Token::NUMBER: {
        return parse_number(idf, index);
    }
    case Token::INTEGER: {
        return parse_integer(idf, index);
    }
    case Token::NONE:
    case Token::END:
    case Token::EXCLAMATION:
    case Token::COMMA:
    case Token::SEMICOLON:
    default:
        break;
    }
    success = false;
    return nullptr;
}

std::string IdfParser::parse_string(std::string_view idf, size_t &index)
{
    eat_whitespace(idf, index);

    std::string str;

    while (true) {
        if (index == idf_size) {
            break;
        }

        char c = idf[index];
        increment_both_index(index, index_into_cur_line);
        if (c == ',' || c == ';' || c == '!') {
            decrement_both_index(index, index_into_cur_line);
            break;
        } else {
            str += c;
        }
    }

    return rtrim(str);
}

void IdfParser::increment_both_index(size_t &index, size_t &line_index)
{
    index++;
    line_index++;
}

void IdfParser::decrement_both_index(size_t &index, size_t &line_index)
{
    index--;
    line_index--;
}

void IdfParser::eat_whitespace(std::string_view idf, size_t &index)
{
    while (index < idf_size) {
        switch (idf[index]) {
        case ' ':
        case '\r':
        case '\t':
            increment_both_index(index, index_into_cur_line);
            continue;
        case '\n':
            increment_both_index(index, cur_line_num);
            beginning_of_line_index = index;
            index_into_cur_line = 0;
            continue;
        default:
            return;
        }
    }
}

void IdfParser::eat_comment(std::string_view idf, size_t &index)
{
    while (true) {
        if (index == idf_size) {
            break;
        }
        if (idf[index] == '\n') {
            increment_both_index(index, cur_line_num);
            index_into_cur_line = 0;
            beginning_of_line_index = index;
            break;
        }
        increment_both_index(index, index_into_cur_line);
    }
}

IdfParser::Token IdfParser::look_ahead(std::string_view idf, size_t index)
{
    size_t save_index = index;
    size_t save_line_num = cur_line_num;
    size_t save_line_index = index_into_cur_line;
    Token token = next_token(idf, save_index);
    cur_line_num = save_line_num;
    index_into_cur_line = save_line_index;
    return token;
}

IdfParser::Token IdfParser::next_token(std::string_view idf, size_t &index)
{
    eat_whitespace(idf, index);

    if (index == idf_size) {
        return Token::END;
    }

    char const c = idf[index];
    increment_both_index(index, index_into_cur_line);
    switch (c) {
    case '!':
        return Token::EXCLAMATION;
    case ',':
        return Token::COMMA;
    case ';':
        return Token::SEMICOLON;
    default:
        static constexpr std::string_view numeric(".-+0123456789");
        if (numeric.find_first_of(c) != std::string::npos) {
            return Token::NUMBER;
        }
        return Token::STRING;
    }
    decrement_both_index(index, index_into_cur_line);
    return Token::NONE;
}

IdfParser::Token IdfParser::next_limited_token(std::string_view idf, size_t &index)
{
    if (index == idf_size) {
        return Token::END;
    }

    char const c = idf[index];
    increment_both_index(index, index_into_cur_line);
    switch (c) {
    case '!':
        return Token::EXCLAMATION;
    case ',':
        return Token::COMMA;
    case ';':
        return Token::SEMICOLON;
    default:
        return Token::NONE;
    }
}

std::string IdfParser::rtrim(std::string_view str)
{
    static constexpr std::string_view whitespace(" \t\0", 3);
    if (str.empty()) {
        return std::string{};
    }
    size_t const index = str.find_last_not_of(whitespace);
    if (index == std::string::npos) {
        return std::string{};
    } else if (index + 1 < str.length()) {
        return std::string{str.substr(0, index + 1)};
    }
    return std::string{str};
}
