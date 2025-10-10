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

// C++ Headers

// ObjexxFCL Headers

// EnergyPlus Headers
#include <EnergyPlus/InputProcessing/InputValidation.hh>
#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/utils/nlohmann_json_utils.hpp>
#include <valijson/validator.hpp>

using json = nlohmann::json;

Validation::Validation(json const *parsed_schema)
{
    schema = parsed_schema;
}

bool Validation::hasErrors()
{
    return !errors_.empty();
}

std::vector<std::string> const &Validation::errors()
{
    return errors_;
}

std::vector<std::string> const &Validation::warnings()
{
    return warnings_;
}

// this is a little bit risky, since it's theoretically
// possible that the schema could change?
// But we're trying it to see the impact on the code
const valijson::Schema &validation_schema(const json *schema)
{
    [[maybe_unused]] static const json *last_schema = schema;

    assert(last_schema == schema);

    static const std::unique_ptr<valijson::Schema> retval = [&]() {
        auto vs = std::make_unique<valijson::Schema>(); // (THIS_AUTO_OK)
        valijson::SchemaParser parser;
        valijson::adapters::NlohmannJsonAdapter schema_doc(*schema);
        parser.populateSchema(schema_doc, *vs);
        return vs;
    }();

    return *retval;
}

bool Validation::validate(json const &parsed_input)
{

    static constexpr std::string_view nameError =
        "Object contains a property that could not be validated using 'properties' or 'additionalProperties' constraints: ''.";
    static constexpr std::string_view otherError =
        "Object contains a property that could not be validated using 'properties' or 'additionalProperties' constraints";

    // valijson::Validator = valijson::ValidatorT<DefaultRegexEngine>, which uses std::regex, and we want RE2 because std::regex is horribly slow
    valijson::ValidatorT<RE2RegexpEngine> validator;
    valijson::adapters::NlohmannJsonAdapter doc(parsed_input);
    valijson::ValidationResults results;
    if (!validator.validate(validation_schema(schema), doc, &results)) {
        valijson::ValidationResults::Error error;
        size_t max_context = 0;
        while (results.popError(error)) {
            if (error.context.size() >= max_context) {
                max_context = error.context.size();
                std::string context;
                for (auto it = error.context.begin(); it != error.context.end(); it++) {
                    context += *it;
                }

                errors_.emplace_back(context + " - " + error.description);
                if (max_context == 2) {
                    if (error.description == nameError) { // C++20: should mark this [[likely]]
                        errors_.emplace_back(context + " - Object name is required and cannot be blank or whitespace");
                    } else if (error.description.find(otherError) != std::string::npos) {
                        errors_.emplace_back(context + " - Object name is required and cannot be blank or whitespace, and must be UTF-8 encoded");
                    }
                }
            }
        }
        return false;
    }
    return true;
}
