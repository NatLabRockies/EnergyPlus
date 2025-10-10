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

#ifndef InputProcessor_hh_INCLUDED
#define InputProcessor_hh_INCLUDED

// C++ Headers
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array1S.fwd.hh>
#include <ObjexxFCL/Optional.hh>

#include <nlohmann/json.hpp>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/InputProcessing/DataStorage.hh>
#include <EnergyPlus/InputProcessing/IdfParser.hh>
#include <EnergyPlus/InputProcessing/InputValidation.hh>

class IdfParser;
class Validation;
struct EnergyPlusData;

namespace EnergyPlus {

void cleanEPJSON(nlohmann::json &epjson);

class InputProcessor
{
public:
    using json = nlohmann::json;

    json::parser_callback_t callback;

    InputProcessor();

    static std::unique_ptr<InputProcessor> factory();

    template <typename T> T *objectFactory(EnergyPlusData &state, std::string const &objectName)
    {
        T *p = data->objectFactory<T>(objectName);
        if (p != nullptr) {
            return p;
        }
        auto const &fields = getFields(state, T::canonicalObjectType(), objectName);
        p = data->addObject<T>(objectName, fields);
        return p;
    }

    template <typename T> T *objectFactory(EnergyPlusData &state)
    {
        T *p = data->objectFactory<T>();
        if (p != nullptr) {
            return p;
        }
        auto const &fields = getFields(state, T::canonicalObjectType());
        p = data->addObject<T>(fields);
        return p;
    }

    std::pair<bool, std::string> convertInsensitiveObjectType(std::string const &objectType);

    void initializeMaps();

    void markObjectAsUsed(const std::string &objectType, const std::string &objectName);

    void processInput(EnergyPlusData &state);

    int getNumSectionsFound(std::string const &SectionWord);

    int getNumObjectsFound(EnergyPlusData &state, std::string_view const ObjectWord);

    bool findDefault(std::string &default_value, json const &schema_field_obj);

    bool findDefault(Real64 &default_value, json const &schema_field_obj);

    bool getDefaultValue(EnergyPlusData &state, std::string const &objectWord, std::string const &fieldName, Real64 &value);

    bool getDefaultValue(EnergyPlusData &state, std::string const &objectWord, std::string const &fieldName, std::string &value);

    std::string getAlphaFieldValue(json const &ep_object, json const &schema_obj_props, std::string const &fieldName, bool uc = true);

    Real64 getRealFieldValue(json const &ep_object, json const &schema_obj_props, std::string const &fieldName);

    int getIntFieldValue(json const &ep_object, json const &schema_obj_props, std::string const &fieldName);

    const json &getObjectSchemaProps(EnergyPlusData &state, std::string const &objectWord);

    std::pair<std::string, bool> getObjectItemValue(std::string const &field_value, json const &schema_field_obj);

    const json &getJSONObjectItem(EnergyPlusData &state, std::string_view ObjType, std::string_view ObjName);

    void getObjectItem(EnergyPlusData &state,
                       std::string_view Object,
                       int const Number,
                       Array1S_string Alphas,
                       int &NumAlphas,
                       Array1D<Real64> &Numbers,
                       int &NumNumbers,
                       int &Status,
                       ObjexxFCL::Optional<Array1D_bool> NumBlank = _,
                       ObjexxFCL::Optional<Array1D_bool> AlphaBlank = _,
                       ObjexxFCL::Optional<Array1D_string> AlphaFieldNames = _,
                       ObjexxFCL::Optional<Array1D_string> NumericFieldNames = _);

    int getIDFObjNum(EnergyPlusData &state, std::string_view Object, int const Number);

    std::vector<std::string> getIDFOrderedKeys(EnergyPlusData &state, std::string_view Object);

    int getJSONObjNum(EnergyPlusData &state, std::string const &Object, int const Number);

    int getObjectItemNum(EnergyPlusData &state,
                         std::string_view ObjType, // Object Type (ref: IDD Objects)
                         std::string_view ObjName  // Name of the object type
    );

    int getObjectItemNum(EnergyPlusData &state,
                         std::string_view ObjType,       // Object Type (ref: IDD Objects)
                         std::string const &NameTypeVal, // Object "name" field type ( used as search key )
                         std::string const &ObjName      // Name of the object type
    );

    void getMaxSchemaArgs(int &NumArgs, int &NumAlpha, int &NumNumeric);

    void getObjectDefMaxArgs(EnergyPlusData &state,
                             std::string_view const ObjectWord, // Object for definition
                             int &NumArgs,                      // How many arguments (max) this Object can have
                             int &NumAlpha,                     // How many Alpha arguments (max) this Object can have
                             int &NumNumeric                    // How many Numeric arguments (max) this Object can have
    );

    void preProcessorCheck(EnergyPlusData &state, bool &PreP_Fatal); // True if a preprocessor flags a fatal error

    void preScanReportingVariables(EnergyPlusData &state);

    void reportIDFRecordsStats(EnergyPlusData &state);

    void reportOrphanRecordObjects(EnergyPlusData &state);

    const json &getObjectInstances(std::string const &ObjType);

    bool checkForUnsupportedObjects(EnergyPlusData &state);

    //    void clear_state();
private:
    friend class EnergyPlusFixture;
    friend class InputProcessorFixture;

    struct ObjectInfo
    {
        ObjectInfo() = default;

        ObjectInfo(std::string const &objectType, std::string const &objectName) : objectType(objectType), objectName(objectName)
        {
        }

        ObjectInfo(std::string &&objectType, std::string &&objectName) : objectType(objectType), objectName(objectName)
        {
        }

        bool operator<(const ObjectInfo &rhs) const
        {
            int cmp = this->objectType.compare(rhs.objectType);
            if (cmp == 0) {
                return this->objectName < rhs.objectName;
            }
            return cmp < 0;
        }

        std::string objectType = "";
        std::string objectName = "";
    };

    struct ObjectCache
    {
        ObjectCache() = default;

        ObjectCache(json::const_iterator const &schemaIterator, std::vector<json::const_iterator> const &inputObjectIterators)
            : schemaIterator(schemaIterator), inputObjectIterators(inputObjectIterators)
        {
        }

        ObjectCache(json::const_iterator &&schemaIterator, std::vector<json::const_iterator> &&inputObjectIterators)
            : schemaIterator(schemaIterator), inputObjectIterators(inputObjectIterators)
        {
        }

        json::const_iterator schemaIterator;
        std::vector<json::const_iterator> inputObjectIterators;
    };

    struct MaxFields
    {
        MaxFields() = default;
        std::size_t max_fields = 0;
        std::size_t max_extensible_fields = 0;
    };

    MaxFields findMaxFields(
        EnergyPlusData &state, json const &ep_object, std::string const &extension_key, json const &legacy_idd, std::size_t const min_fields);

    void setObjectItemValue(EnergyPlusData &state,
                            json const &ep_object,
                            json const &ep_schema_object,
                            std::string const &field,
                            json const &legacy_field_info,
                            int &alpha_index,
                            int &numeric_index,
                            bool within_max_fields,
                            Array1S_string Alphas,
                            int &NumAlphas,
                            Array1D<Real64> &Numbers,
                            int &NumNumbers,
                            ObjexxFCL::Optional<Array1D_bool> NumBlank = _,
                            ObjexxFCL::Optional<Array1D_bool> AlphaBlank = _,
                            ObjexxFCL::Optional<Array1D_string> AlphaFieldNames = _,
                            ObjexxFCL::Optional<Array1D_string> NumericFieldNames = _);

    void addVariablesForMonthlyReport(EnergyPlusData &state, std::string const &reportName);

    void addRecordToOutputVariableStructure(EnergyPlusData &state, std::string const &KeyValue, std::string const &VariableName);

    std::vector<std::string> const &validationErrors();

    std::vector<std::string> const &validationWarnings();

    bool checkVersionMatch(EnergyPlusData &state);

    bool processErrors(EnergyPlusData &state);

    json const &getFields(EnergyPlusData &state, std::string const &objectType, std::string const &objectName);

    json const &getFields(EnergyPlusData &state, std::string const &objectType);

    json const &getPatternProperties(EnergyPlusData &state, json const &schema_obj);

    inline std::string convertToUpper(std::string_view s)
    {
        std::string s2;
        size_t len = s.size();
        s2.resize(len);
        for (size_t i = 0; i < len; ++i) {
            char c = s[i];
            s2[i] = ('a' <= c && c <= 'z') ? c ^ 0x20 : c; // ASCII only
        }
        s2[len] = '\0';
        return s2;
    }

    using UnorderedObjectTypeMap = std::unordered_map<std::string, std::string>;
    using UnorderedObjectCacheMap = std::unordered_map<std::string, ObjectCache>;
    using UnusedObjectSet = std::set<ObjectInfo>;

    std::unique_ptr<IdfParser> idf_parser;
    std::unique_ptr<Validation> validation;
    std::unique_ptr<DataStorage> data;
    static const json &schema();

public:
    json epJSON;

private:
    // Maps OBJECTTYPE to ObjectType (example entry: {"ZONEHVAC:EQUIPMENTLIST", "ZoneHVAC:EquipmentList"})
    UnorderedObjectTypeMap caseInsensitiveObjectMap;
    // Maps ObjectType to ObjectCache (json::const_iterator const &schemaIterator, std::vector<json::const_iterator> const &inputObjectIterators)
    UnorderedObjectCacheMap objectCacheMap;

    // ObjectType to vector of ObjectName
    UnusedObjectSet unusedInputs;
    char s[129] = {0};

}; // InputProcessor

struct DataInputProcessing : BaseGlobalStruct
{
    std::unique_ptr<InputProcessor> inputProcessor = InputProcessor::factory();

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        inputProcessor.reset();
        inputProcessor = EnergyPlus::InputProcessor::factory();
    }
};

} // namespace EnergyPlus

#endif
