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
#include <algorithm>
#include <fstream>
#include <iostream>
#include <istream>
#include <memory>
#include <unordered_set>

#include <ObjexxFCL/Array1S.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataOutputs.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/DisplayRoutines.hh>
#include <EnergyPlus/FileSystem.hh>
#include <EnergyPlus/InputProcessing/DataStorage.hh>
#include <EnergyPlus/InputProcessing/IdfParser.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/InputProcessing/InputValidation.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <embedded/EmbeddedEpJSONSchema.hh>

#include <fmt/os.h>
#include <milo/dtoa.h>
#include <milo/itoa.h>

namespace EnergyPlus {
// Module containing the input processor routines

// MODULE INFORMATION:
//       AUTHOR         Linda K. Lawrie
//       DATE WRITTEN   August 1997
//       MODIFIED       na
//       RE-ENGINEERED  Mark Adams 2017

// PURPOSE OF THIS MODULE:
// To provide the capabilities of reading the input data dictionary
// and input file and supplying the simulation routines with the data
// contained therein.

// METHODOLOGY EMPLOYED:

// REFERENCES:
// The input syntax is designed to allow for future flexibility without
// necessitating massive (or any) changes to this code.  Two files are
// used as key elements: (1) the input data dictionary will specify the
// sections and objects that will be allowed in the actual simulation
// input file and (2) the simulation input data file will be processed
// with the data therein being supplied to the actual simulation routines.

static std::string const BlankString;

using json = nlohmann::json;

const json &InputProcessor::schema()
{
    // avoid re-parsing embedded JSON schema by making this into a static const singleton
    // because it is const, we don't have to worry about threading issues for creation or access
    static const auto json_schema = json::from_cbor(EmbeddedEpJSONSchema::embeddedEpJSONSchema()); // (AUTO_OK_OBJ)
    return json_schema;
}

InputProcessor::InputProcessor() : idf_parser(std::make_unique<IdfParser>()), data(std::make_unique<DataStorage>())
{
    const json &loc = schema()["properties"];
    caseInsensitiveObjectMap.reserve(loc.size());
    for (auto it = loc.begin(); it != loc.end(); ++it) {
        caseInsensitiveObjectMap.emplace(convertToUpper(it.key()), it.key());
    }
    idf_parser = std::make_unique<IdfParser>();
    data = std::make_unique<DataStorage>();
    epJSON = json::object();
    //    objectCacheMap.clear();
    //    unusedInputs.clear();
    validation = std::make_unique<Validation>(&schema());
}

std::unique_ptr<InputProcessor> InputProcessor::factory()
{
    return std::make_unique<InputProcessor>();
}

json const &InputProcessor::getFields(EnergyPlusData &state, std::string const &objectType, std::string const &objectName)
{
    auto const it = epJSON.find(objectType);
    if (it == epJSON.end()) {
        ShowFatalError(state, "ObjectType (" + objectType + ") requested was not found in input");
    }
    auto const &objs = it.value();
    auto const it2 = objs.find(objectName);
    if (it2 == objs.end()) {
        // HACK: this is not ideal and should be removed once everything is case sensitive internally
        for (auto it3 = objs.begin(); it3 != objs.end(); ++it3) {
            if (Util::makeUPPER(it3.key()) == objectName) {
                return it3.value();
            }
        }
        ShowFatalError(state, "Name \"" + objectName + "\" requested was not found in input for ObjectType (" + objectType + ")");
    }
    return it2.value();
}

json const &InputProcessor::getFields(EnergyPlusData &state, std::string const &objectType)
{
    static const std::string blankString;
    auto const it = epJSON.find(objectType);
    if (it == epJSON.end()) {
        ShowFatalError(state, "ObjectType (" + objectType + ") requested was not found in input");
    }
    auto const &objs = it.value();
    auto const it2 = objs.find(blankString);
    if (it2 == objs.end()) {
        ShowFatalError(state, "Name \"\" requested was not found in input for ObjectType (" + objectType + ")");
    }
    return it2.value();
}

json const &InputProcessor::getPatternProperties(EnergyPlusData &state, json const &schema_obj)
{
    std::string pattern_property;
    auto const &pattern_properties = schema_obj["patternProperties"];
    int dot_star_present = pattern_properties.count(".*");
    int no_whitespace_present = pattern_properties.count(R"(^.*\S.*$)");
    if (dot_star_present) {
        pattern_property = ".*";
    } else if (no_whitespace_present) {
        pattern_property = R"(^.*\S.*$)";
    } else {
        ShowFatalError(state, R"(The patternProperties value is not a valid choice (".*", "^.*\S.*$"))");
    }
    auto const &schema_obj_props = pattern_properties[pattern_property]["properties"];
    return schema_obj_props;
}

// Functions

// void InputProcessor::clear_state() {
//    idf_parser = std::make_unique<IdfParser>();
//    data = std::make_unique<DataStorage>();
//    epJSON = json::object();
//    objectCacheMap.clear();
//    unusedInputs.clear();
//    validation = std::make_unique<Validation>(&schema);
//}

std::vector<std::string> const &InputProcessor::validationErrors()
{
    return validation->errors();
}

std::vector<std::string> const &InputProcessor::validationWarnings()
{
    return validation->warnings();
}

std::pair<bool, std::string> InputProcessor::convertInsensitiveObjectType(std::string const &objectType)
{
    auto tmp_umit = caseInsensitiveObjectMap.find(convertToUpper(objectType));
    if (tmp_umit != caseInsensitiveObjectMap.end()) {
        return std::make_pair(true, tmp_umit->second);
    }
    return std::make_pair(false, "");
}

void InputProcessor::initializeMaps()
{
    unusedInputs.clear();
    objectCacheMap.clear();
    objectCacheMap.reserve(epJSON.size());
    auto const &schema_properties = schema().at("properties");

    for (auto epJSON_iter = epJSON.begin(); epJSON_iter != epJSON.end(); ++epJSON_iter) {
        auto const &objects = epJSON_iter.value();
        auto const &objectType = epJSON_iter.key();
        ObjectCache objectCache;
        objectCache.inputObjectIterators.reserve(objects.size());
        for (auto epJSON_obj_iter = objects.begin(); epJSON_obj_iter != objects.end(); ++epJSON_obj_iter) {
            objectCache.inputObjectIterators.emplace_back(epJSON_obj_iter);
            unusedInputs.emplace(objectType, epJSON_obj_iter.key());
        }
        auto const schema_iter = schema_properties.find(objectType);
        objectCache.schemaIterator = schema_iter;
        objectCacheMap.emplace(schema_iter.key(), objectCache);
    }
}

void InputProcessor::markObjectAsUsed(const std::string &objectType, const std::string &objectName)
{
    auto const find_unused = unusedInputs.find({objectType, objectName});
    if (find_unused != unusedInputs.end()) {
        unusedInputs.erase(find_unused);
    }
}

void cleanEPJSON(json &epjson)
{
    if (epjson.type() == json::value_t::object) {
        epjson.erase("idf_order");
        epjson.erase("idf_max_fields");
        epjson.erase("idf_max_extensible_fields");
        for (auto it = epjson.begin(); it != epjson.end(); ++it) {
            cleanEPJSON(epjson[it.key()]);
        }
    }
}

void InputProcessor::processInput(EnergyPlusData &state)
{
    if (!FileSystem::fileExists(state.dataStrGlobals->inputFilePath)) {
        ShowFatalError(state, fmt::format("Input file path {} not found", state.dataStrGlobals->inputFilePath));
        return;
    }

    try {
        if (!state.dataGlobal->isEpJSON) {
            auto input_file = FileSystem::readFile(state.dataStrGlobals->inputFilePath); // (AUTO_OK_OBJ)

            bool success = true;
            epJSON = idf_parser->decode(input_file, schema(), success);

            if (state.dataGlobal->outputEpJSONConversion || state.dataGlobal->outputEpJSONConversionOnly) {
                json epJSONClean = epJSON;
                cleanEPJSON(epJSONClean);
                fs::path convertedIDF = FileSystem::makeNativePath(
                    FileSystem::replaceFileExtension(state.dataStrGlobals->outDirPath / state.dataStrGlobals->inputFilePathNameOnly, ".epJSON"));
                FileSystem::writeFile<FileSystem::FileTypes::EpJSON>(convertedIDF, epJSONClean);
            }
        } else {
            epJSON = FileSystem::readJSON(state.dataStrGlobals->inputFilePath);
        }
    } catch (const std::exception &e) {
        ShowSevereError(state, e.what());
        ShowFatalError(state, "Errors occurred on processing input file. Preceding condition(s) cause termination.");
    }

    bool is_valid = validation->validate(epJSON);
    bool hasErrors = processErrors(state);
    bool versionMatch = checkVersionMatch(state);
    bool unsupportedFound = checkForUnsupportedObjects(state);

    if (!is_valid || hasErrors || unsupportedFound) {
        ShowFatalError(state, "Errors occurred on processing input file. Preceding condition(s) cause termination.");
    }

    if (state.dataGlobal->isEpJSON && (state.dataGlobal->outputEpJSONConversion || state.dataGlobal->outputEpJSONConversionOnly)) {
        if (versionMatch) {
            std::string const encoded = idf_parser->encode(epJSON, schema());
            fs::path convertedEpJSON = FileSystem::makeNativePath(
                FileSystem::replaceFileExtension(state.dataStrGlobals->outDirPath / state.dataStrGlobals->inputFilePathNameOnly, ".idf"));
            FileSystem::writeFile<FileSystem::FileTypes::IDF>(convertedEpJSON, encoded);
        } else {
            ShowWarningError(state, "Skipping conversion of epJSON to IDF due to mismatched Version.");
        }
    }

    initializeMaps();

    int MaxArgs = 0;
    int MaxAlpha = 0;
    int MaxNumeric = 0;
    getMaxSchemaArgs(MaxArgs, MaxAlpha, MaxNumeric);

    state.dataIPShortCut->cAlphaFieldNames.allocate(MaxAlpha);
    state.dataIPShortCut->cAlphaArgs.allocate(MaxAlpha);
    state.dataIPShortCut->lAlphaFieldBlanks.dimension(MaxAlpha, false);
    state.dataIPShortCut->cNumericFieldNames.allocate(MaxNumeric);
    state.dataIPShortCut->rNumericArgs.dimension(MaxNumeric, 0.0);
    state.dataIPShortCut->lNumericFieldBlanks.dimension(MaxNumeric, false);

    reportIDFRecordsStats(state);
}

bool InputProcessor::checkVersionMatch(EnergyPlusData &state)
{
    using DataStringGlobals::MatchVersion;
    auto it = epJSON.find("Version");
    if (it != epJSON.end()) {
        for (auto const &version : it.value()) {
            std::string v = version["version_identifier"].get<std::string>();
            if (v.empty()) {
                ShowWarningError(state, "Input errors occurred and version ID was left blank, verify file version");
            } else {
                std::string::size_type const lenVer(len(MatchVersion));
                int Which;
                if ((lenVer > 0) && (MatchVersion[lenVer - 1] == '0')) {
                    Which = static_cast<int>(index(v.substr(0, lenVer - 2), MatchVersion.substr(0, lenVer - 2)));
                } else {
                    Which = static_cast<int>(index(v, MatchVersion));
                }
                if (Which != 0) {
                    ShowWarningError(state, "Version: in IDF=\"" + v + "\" not the same as expected=\"" + MatchVersion + "\"");
                    return false;
                }
            }
        }
    }
    return true;
}

bool InputProcessor::checkForUnsupportedObjects(EnergyPlusData &state)
{
    bool errorsFound = false;
    static constexpr std::array<std::string_view, 32> hvacTemplateObjects = {"HVACTemplate:Thermostat",
                                                                             "HVACTemplate:Zone:IdealLoadsAirSystem",
                                                                             "HVACTemplate:Zone:BaseboardHeat",
                                                                             "HVACTemplate:Zone:FanCoil",
                                                                             "HVACTemplate:Zone:PTAC",
                                                                             "HVACTemplate:Zone:PTHP",
                                                                             "HVACTemplate:Zone:WaterToAirHeatPump",
                                                                             "HVACTemplate:Zone:VRF",
                                                                             "HVACTemplate:Zone:Unitary",
                                                                             "HVACTemplate:Zone:VAV",
                                                                             "HVACTemplate:Zone:VAV:FanPowered",
                                                                             "HVACTemplate:Zone:VAV:HeatAndCool",
                                                                             "HVACTemplate:Zone:ConstantVolume",
                                                                             "HVACTemplate:Zone:DualDuct",
                                                                             "HVACTemplate:System:VRF",
                                                                             "HVACTemplate:System:Unitary",
                                                                             "HVACTemplate:System:UnitaryHeatPump:AirToAir",
                                                                             "HVACTemplate:System:UnitarySystem",
                                                                             "HVACTemplate:System:VAV",
                                                                             "HVACTemplate:System:PackagedVAV",
                                                                             "HVACTemplate:System:ConstantVolume",
                                                                             "HVACTemplate:System:DualDuct",
                                                                             "HVACTemplate:System:DedicatedOutdoorAir",
                                                                             "HVACTemplate:Plant:ChilledWaterLoop",
                                                                             "HVACTemplate:Plant:Chiller",
                                                                             "HVACTemplate:Plant:Chiller:ObjectReference",
                                                                             "HVACTemplate:Plant:Tower",
                                                                             "HVACTemplate:Plant:Tower:ObjectReference",
                                                                             "HVACTemplate:Plant:HotWaterLoop",
                                                                             "HVACTemplate:Plant:Boiler",
                                                                             "HVACTemplate:Plant:Boiler:ObjectReference",
                                                                             "HVACTemplate:Plant:MixedWaterLoop"};

    // For EnergyPlus, there is no option to convert or allow these objects
    bool objectFound = false;
    std::string objectType;
    for (size_t count = 0; count < hvacTemplateObjects.size(); ++count) {
        objectType = hvacTemplateObjects[count];
        auto it = epJSON.find(objectType);
        if (it != epJSON.end()) {
            objectFound = true;
            break;
        }
    }
    if (objectFound) {
        ShowSevereError(state, "HVACTemplate:* objects found. These objects are not supported directly by EnergyPlus.");
        ShowContinueError(state, "You must run the ExpandObjects program on this input.");
        errorsFound = true;
    }

    static constexpr std::array<std::string_view, 26> groundHTObjects = {"GroundHeatTransfer:Control",
                                                                         "GroundHeatTransfer:Slab:Materials",
                                                                         "GroundHeatTransfer:Slab:MatlProps",
                                                                         "GroundHeatTransfer:Slab:BoundConds",
                                                                         "GroundHeatTransfer:Slab:BldgProps",
                                                                         "GroundHeatTransfer:Slab:Insulation",
                                                                         "GroundHeatTransfer:Slab:EquivalentSlab",
                                                                         "GroundHeatTransfer:Slab:AutoGrid",
                                                                         "GroundHeatTransfer:Slab:ManualGrid",
                                                                         "GroundHeatTransfer:Slab:XFACE",
                                                                         "GroundHeatTransfer:Slab:YFACE",
                                                                         "GroundHeatTransfer:Slab:ZFACE",
                                                                         "GroundHeatTransfer:Basement:SimParameters",
                                                                         "GroundHeatTransfer:Basement:MatlProps",
                                                                         "GroundHeatTransfer:Basement:Insulation",
                                                                         "GroundHeatTransfer:Basement:SurfaceProps",
                                                                         "GroundHeatTransfer:Basement:BldgData",
                                                                         "GroundHeatTransfer:Basement:Interior",
                                                                         "GroundHeatTransfer:Basement:ComBldg",
                                                                         "GroundHeatTransfer:Basement:EquivSlab",
                                                                         "GroundHeatTransfer:Basement:EquivAutoGrid",
                                                                         "GroundHeatTransfer:Basement:AutoGrid",
                                                                         "GroundHeatTransfer:Basement:ManualGrid",
                                                                         "GroundHeatTransfer:Basement:XFACE",
                                                                         "GroundHeatTransfer:Basement:YFACE",
                                                                         "GroundHeatTransfer:Basement:ZFACE"};

    objectFound = false;
    for (size_t count = 0; count < groundHTObjects.size(); ++count) {
        objectType = groundHTObjects[count];
        auto it = epJSON.find(objectType);
        if (it != epJSON.end()) {
            objectFound = true;
            break;
        }
    }
    if (objectFound) {
        ShowSevereError(state, "GroundHeatTransfer:* objects found. These objects are not supported directly by EnergyPlus.");
        ShowContinueError(state, "You must run the ExpandObjects program on this input.");
        errorsFound = true;
    }

    static constexpr std::array<std::string_view, 4> parametricObjects = {
        "Parametric:SetValueForRun", "Parametric:Logic", "Parametric:RunControl", "Parametric:FileNameSuffix"};

    objectFound = false;
    for (size_t count = 0; count < parametricObjects.size(); ++count) {
        objectType = parametricObjects[count];
        auto it = epJSON.find(objectType);
        if (it != epJSON.end()) {
            objectFound = true;
            break;
        }
    }
    if (objectFound) {
        ShowSevereError(state, "Parametric:* objects found. These objects are not supported directly by EnergyPlus.");
        ShowContinueError(state, "You must run the ParametricPreprocesor program on this input.");
        errorsFound = true;
    }
    return errorsFound;
}

bool InputProcessor::processErrors(EnergyPlusData &state)
{
    for (auto const &error : idf_parser->errors()) {
        ShowSevereError(state, error);
    }
    for (auto const &warning : idf_parser->warnings()) {
        ShowWarningError(state, warning);
    }
    for (auto const &error : validation->errors()) {
        ShowSevereError(state, error);
    }
    for (auto const &warning : validation->warnings()) {
        ShowWarningError(state, warning);
    }

    bool has_errors = validation->hasErrors() || idf_parser->hasErrors();

    return has_errors;
}

int InputProcessor::getNumSectionsFound(std::string const &SectionWord)
{
    // PURPOSE OF THIS SUBROUTINE:
    // This function returns the number of a particular section (in input data file)
    // found in the current run.  If it can't find the section in list
    // of sections, a -1 will be returned.

    // METHODOLOGY EMPLOYED:
    // Look up section in list of sections.  If there, return the
    // number of sections of that kind found in the current input.  If not, return -1.

    auto const SectionWord_iter = epJSON.find(SectionWord);
    if (SectionWord_iter == epJSON.end()) {
        return -1;
    }
    return static_cast<int>(SectionWord_iter.value().size());
}

int InputProcessor::getNumObjectsFound(EnergyPlusData &state, std::string_view const ObjectWord)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   September 1997
    //       MODIFIED       Mark Adams
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This function returns the number of objects (in input data file)
    // found in the current run.  If it can't find the object in list
    // of objects, a 0 will be returned.

    // METHODOLOGY EMPLOYED:
    // Look up object in list of objects.  If there, return the
    // number of objects found in the current input.  If not, return 0.

    auto const find_obj = epJSON.find(std::string(ObjectWord));

    if (find_obj == epJSON.end()) {
        auto tmp_umit = caseInsensitiveObjectMap.find(convertToUpper(ObjectWord));
        if (tmp_umit == caseInsensitiveObjectMap.end() || epJSON.find(tmp_umit->second) == epJSON.end()) {
            return 0;
        }
        return static_cast<int>(epJSON[tmp_umit->second].size());
    } else {
        return static_cast<int>(find_obj.value().size());
    }

    if (schema()["properties"].find(std::string(ObjectWord)) == schema()["properties"].end()) {
        auto tmp_umit = caseInsensitiveObjectMap.find(convertToUpper(ObjectWord));
        if (tmp_umit == caseInsensitiveObjectMap.end()) {
            ShowWarningError(state, fmt::format("Requested Object not found in Definitions: {}", ObjectWord));
        }
    }
    return 0;
}

bool InputProcessor::findDefault(std::string &default_value, json const &schema_field_obj)
{
    auto const find_default = schema_field_obj.find("default");
    if (find_default != schema_field_obj.end()) {
        auto const &default_val = find_default.value();
        if (default_val.is_string()) {
            default_value = default_val.get<std::string>();
        } else {
            if (default_val.is_number_integer()) {
                i64toa(default_val.get<std::int64_t>(), s);
            } else {
                dtoa(default_val.get<double>(), s);
            }
            default_value = s;
        }
        if (schema_field_obj.find("retaincase") == schema_field_obj.end()) {
            default_value = Util::makeUPPER(default_value);
        }
        return true;
    }
    return false;
}

bool InputProcessor::findDefault(Real64 &default_value, json const &schema_field_obj)
{
    auto const find_default = schema_field_obj.find("default");
    default_value = 0;
    if (find_default != schema_field_obj.end()) {
        auto const &default_val = find_default.value();
        if (default_val.is_string() && !default_val.get<std::string>().empty()) {
            // autosize and autocalculate
            default_value = Constant::AutoCalculate;
        } else if (default_val.is_number_integer()) {
            default_value = default_val.get<std::int64_t>();
        } else {
            default_value = default_val.get<double>();
        }
        return true;
    }
    return false;
}

bool InputProcessor::getDefaultValue(EnergyPlusData &state, std::string const &objectWord, std::string const &fieldName, Real64 &value)
{
    auto find_iterators = objectCacheMap.find(objectWord);
    if (find_iterators == objectCacheMap.end()) {
        auto const tmp_umit = caseInsensitiveObjectMap.find(convertToUpper(objectWord));
        if (tmp_umit == caseInsensitiveObjectMap.end() || epJSON.find(tmp_umit->second) == epJSON.end()) {
            return false;
        }
        find_iterators = objectCacheMap.find(tmp_umit->second);
    }
    auto const &epJSON_schema_it = find_iterators->second.schemaIterator;
    auto const &epJSON_schema_it_val = epJSON_schema_it.value();
    auto const &schema_obj_props = getPatternProperties(state, epJSON_schema_it_val);
    auto const &sizing_factor_schema_field_obj = schema_obj_props.at(fieldName);
    bool defaultFound = findDefault(value, sizing_factor_schema_field_obj);
    return defaultFound;
}

bool InputProcessor::getDefaultValue(EnergyPlusData &state, std::string const &objectWord, std::string const &fieldName, std::string &value)
{
    auto find_iterators = objectCacheMap.find(objectWord);
    if (find_iterators == objectCacheMap.end()) {
        auto const tmp_umit = caseInsensitiveObjectMap.find(convertToUpper(objectWord));
        if (tmp_umit == caseInsensitiveObjectMap.end() || epJSON.find(tmp_umit->second) == epJSON.end()) {
            return false;
        }
        find_iterators = objectCacheMap.find(tmp_umit->second);
    }
    auto const &epJSON_schema_it = find_iterators->second.schemaIterator;
    auto const &epJSON_schema_it_val = epJSON_schema_it.value();
    auto const &schema_obj_props = getPatternProperties(state, epJSON_schema_it_val);
    auto const &sizing_factor_schema_field_obj = schema_obj_props.at(fieldName);
    bool defaultFound = findDefault(value, sizing_factor_schema_field_obj);
    return defaultFound;
}

std::string InputProcessor::getAlphaFieldValue(json const &ep_object, json const &schema_obj_props, std::string const &fieldName, bool uc)
{
    // Return the value of fieldName in ep_object as a string.
    // If the field is not present in ep_object then return its default if there is one, or return an empty string
    auto const &fprops = schema_obj_props[fieldName];
    assert(!fprops.empty()); // Check that field name exists in the schema for this object type

    uc = (fprops.find("retaincase") == fprops.end());

    auto it = ep_object.find(fieldName);
    if (it != ep_object.end()) {
        auto const &val = it.value();
        assert(val.is_string());
        if (!val.empty()) {
            return uc ? Util::makeUPPER(val.get<std::string>()) : val.get<std::string>();
        }
    }

    auto const it2 = fprops.find("default");
    return (it2 != fprops.end()) ? (uc ? Util::makeUPPER(it2.value().get<std::string>()) : it2.value().get<std::string>()) : std::string();

#ifdef GET_OUT
    if (default_val.is_number_integer()) {
        i64toa(default_val.get<std::int64_t>(), s);
    } else if (default_val.is_number()) {
        dtoa(default_val.get<double>(), s);
    }
#endif // GET_OUT
}

Real64 InputProcessor::getRealFieldValue(json const &ep_object, json const &schema_obj_props, std::string const &fieldName)
{
    // Return the value of fieldName in ep_object as a Real64.
    // If the field value is a string, then assume autosize and return Constant::AutoCalculate(-99999).
    // If the field is not present in ep_object then return its default if there is one, or return 0.0
    auto it = ep_object.find(fieldName);
    if (it != ep_object.end()) {
        auto const &field_value = it.value();
        if (field_value.is_number()) {
            return (field_value.is_number_integer()) ? field_value.get<std::int64_t>() : field_value.get<double>();
        } else if (!field_value.get<std::string>().empty()) {
            return Constant::AutoCalculate; // autosize and autocalculate
        }
    }

    auto const &schema_field_obj = schema_obj_props[fieldName];
    assert(!schema_field_obj.empty()); // Check that field name exists in the schema for this object type

    auto const find_default = schema_field_obj.find("default");
    if (find_default != schema_field_obj.end()) {
        auto const &default_val = find_default.value();
        if (default_val.is_string()) {
            return (!default_val.get<std::string>().empty()) ? Constant::AutoCalculate : 0.0;
        } else if (default_val.is_number_integer()) {
            return default_val.get<std::int64_t>();
        } else {
            return default_val.get<double>();
        }
    } else {
        return 0.0;
    }
}

int InputProcessor::getIntFieldValue(json const &ep_object, json const &schema_obj_props, std::string const &fieldName)
{
    // Return the value of fieldName in ep_object as an integer.
    // If the field value is a string, then assume autosize or autocalculate and return Constant::AutoCalculate(-99999).
    // If the field is not present in ep_object then return its default if there is one, or return 0

    auto const &schema_field_obj = schema_obj_props[fieldName];
    assert(!schema_field_obj.empty()); // Check that field name exists in the schema for this object type
    bool isDefaulted = false;
    int value = 0;
    Real64 defaultValue = 0.0;
    auto it = ep_object.find(fieldName);
    if (it != ep_object.end()) {
        auto const &field_value = it.value();
        if (field_value.is_number_integer()) {
            value = field_value.get<std::int64_t>();
        } else if (field_value.is_number()) {
            // This is a developer error, floating point numbers should not be retrieved this way. If this field
            // really is an int then the input processor will have forced it to be an integer.
            assert(!field_value.is_number());
        } else if (field_value.get<std::string>().empty()) {
            isDefaulted = findDefault(defaultValue, schema_field_obj);
            if (isDefaulted) {
                value = static_cast<int>(defaultValue);
            }
        }
    } else {
        isDefaulted = findDefault(defaultValue, schema_field_obj);
        if (isDefaulted) {
            value = static_cast<int>(defaultValue);
        }
    }
    return value;
}

const json &InputProcessor::getObjectSchemaProps(EnergyPlusData &state, std::string const &objectWord)
{
    auto const &schema_properties = schema().at("properties");
    const json &object_schema = schema_properties.at(objectWord);
    assert(!object_schema.empty()); // If this fails, the object type does not exist in the schema

    auto const &schema_obj_props = getPatternProperties(state, object_schema);
    return schema_obj_props;
}

std::pair<std::string, bool> InputProcessor::getObjectItemValue(std::string const &field_value, json const &schema_field_obj)
{
    std::pair<std::string, bool> output;
    if (field_value.empty()) {
        findDefault(output.first, schema_field_obj);
        output.second = true;
    } else {
        output.first = field_value;
        output.second = false;
    }
    if (schema_field_obj.find("retaincase") == schema_field_obj.end()) {
        output.first = Util::makeUPPER(output.first);
    }
    return output;
}

const json &InputProcessor::getObjectInstances(std::string const &ObjType)
{
    return epJSON.find(ObjType).value();
}

InputProcessor::MaxFields InputProcessor::findMaxFields(
    EnergyPlusData &state, json const &ep_object, std::string const &extension_key, json const &legacy_idd, std::size_t const min_fields)
{
    InputProcessor::MaxFields maxFields;
    if (!state.dataGlobal->isEpJSON) {
        auto found_idf_max_fields = ep_object.find("idf_max_fields");
        if (found_idf_max_fields != ep_object.end()) {
            maxFields.max_fields = found_idf_max_fields->get<size_t>();
        }
        auto found_idf_max_extensible_fields = ep_object.find("idf_max_extensible_fields");
        if (found_idf_max_extensible_fields != ep_object.end()) {
            maxFields.max_extensible_fields = found_idf_max_extensible_fields->get<size_t>();
        }
    } else {
        auto const &legacy_idd_fields = legacy_idd["fields"];
        // start with at least min_fields as the number of fields
        maxFields.max_fields = min_fields;
        for (auto const &field : ep_object.items()) {
            auto const &field_key = field.key();
            if (field_key == extension_key) {
                continue;
            }
            for (std::size_t i = maxFields.max_fields; i < legacy_idd_fields.size(); ++i) {
                if (field_key == legacy_idd_fields[i]) {
                    maxFields.max_fields = (i + 1);
                }
            }
        }

        auto const legacy_idd_extensibles_iter = legacy_idd.find("extensibles");
        if (legacy_idd_extensibles_iter != legacy_idd.end()) {
            auto const epJSON_extensions_array_itr = ep_object.find(extension_key);
            if (epJSON_extensions_array_itr != ep_object.end()) {
                auto const &legacy_idd_extensibles = legacy_idd_extensibles_iter.value();
                auto const &epJSON_extensions_array = epJSON_extensions_array_itr.value();

                for (auto const &exts : epJSON_extensions_array.items()) {
                    std::size_t max_extensible_field = 0;
                    for (auto const &ext : exts.value().items()) {
                        auto const &ext_key = ext.key();
                        for (std::size_t i = max_extensible_field; i < legacy_idd_extensibles.size(); ++i) {
                            if (ext_key == legacy_idd_extensibles[i]) {
                                max_extensible_field = (i + 1);
                            }
                        }
                    }
                    maxFields.max_extensible_fields += max_extensible_field;
                }
            }
        }
    }
    return maxFields;
}

void InputProcessor::setObjectItemValue(EnergyPlusData &state,
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
                                        ObjexxFCL::Optional<Array1D_bool> NumBlank,
                                        ObjexxFCL::Optional<Array1D_bool> AlphaBlank,
                                        ObjexxFCL::Optional<Array1D_string> AlphaFieldNames,
                                        ObjexxFCL::Optional<Array1D_string> NumericFieldNames)
{
    bool const is_AlphaBlank = present(AlphaBlank);
    bool const is_AlphaFieldNames = present(AlphaFieldNames);
    bool const is_NumBlank = present(NumBlank);
    bool const is_NumericFieldNames = present(NumericFieldNames);

    std::string field_type = legacy_field_info.at("field_type").get<std::string>();
    auto const &schema_field_obj = ep_schema_object[field];
    auto it = ep_object.find(field);
    if (it != ep_object.end()) {
        auto const &field_value = it.value();
        if (field_type == "a") {
            // process alpha value
            if (field_value.is_string()) {
                auto const value = getObjectItemValue(field_value.get<std::string>(), schema_field_obj); // (AUTO_OK_OBJ)

                Alphas(alpha_index) = value.first;
                if (is_AlphaBlank) {
                    AlphaBlank()(alpha_index) = value.second;
                }

            } else {
                if (field_value.is_number_integer()) {
                    i64toa(field_value.get<std::int64_t>(), s);
                } else {
                    dtoa(field_value.get<double>(), s);
                }
                Alphas(alpha_index) = s;
                if (is_AlphaBlank) {
                    AlphaBlank()(alpha_index) = false;
                }
            }
        } else if (field_type == "n") {
            // process numeric value
            if (field_value.is_number()) {
                if (field_value.is_number_integer()) {
                    Numbers(numeric_index) = field_value.get<std::int64_t>();
                } else {
                    Numbers(numeric_index) = field_value.get<double>();
                }
                if (is_NumBlank) {
                    NumBlank()(numeric_index) = false;
                }
            } else {
                bool is_empty = field_value.get<std::string>().empty();
                if (is_empty) {
                    findDefault(Numbers(numeric_index), schema_field_obj);
                } else {
                    Numbers(numeric_index) = Constant::AutoCalculate; // autosize and autocalculate
                }
                if (is_NumBlank) {
                    NumBlank()(numeric_index) = is_empty;
                }
            }
        }
    } else {
        if (field_type == "a") {
            if (!(findDefault(Alphas(alpha_index), schema_field_obj))) {
                Alphas(alpha_index) = "";
            }
            if (is_AlphaBlank) {
                AlphaBlank()(alpha_index) = true;
            }
        } else if (field_type == "n") {
            findDefault(Numbers(numeric_index), schema_field_obj);
            if (is_NumBlank) {
                NumBlank()(numeric_index) = true;
            }
        }
    }
    if (field_type == "a") {
        if (within_max_fields) {
            NumAlphas = alpha_index;
        }
        if (is_AlphaFieldNames) {
            AlphaFieldNames()(alpha_index) = (state.dataGlobal->isEpJSON) ? field : legacy_field_info.at("field_name").get<std::string>();
        }
        alpha_index++;
    } else if (field_type == "n") {
        if (within_max_fields) {
            NumNumbers = numeric_index;
        }
        if (is_NumericFieldNames) {
            NumericFieldNames()(numeric_index) = (state.dataGlobal->isEpJSON) ? field : legacy_field_info.at("field_name").get<std::string>();
        }
        numeric_index++;
    }
}

const json &InputProcessor::getJSONObjectItem(EnergyPlusData &state, std::string_view ObjType, std::string_view ObjName)
{
    std::string objTypeStr(ObjType);
    std::string objNameStr(ObjName);

    auto objectInfo = ObjectInfo(objTypeStr, objNameStr); // (AUTO_OK_OBJ)

    auto obj_iter = epJSON.find(objTypeStr);
    if (obj_iter == epJSON.end() || obj_iter.value().find(objectInfo.objectName) == obj_iter.value().end()) {
        auto tmp_umit = caseInsensitiveObjectMap.find(convertToUpper(objectInfo.objectType));
        if (tmp_umit == caseInsensitiveObjectMap.end()) {
            // indicates object type not found, see function GeneralRoutines::ValidateComponent
            ShowFatalError(state, format(R"(ObjectType of type "{}" requested was not found in input)", objectInfo.objectType));
        }
        objectInfo.objectType = tmp_umit->second;
        obj_iter = epJSON.find(objectInfo.objectType);
    }

    std::string const upperObjName = convertToUpper(objectInfo.objectName);

    for (const auto &[key, val] : obj_iter->items()) {
        if (convertToUpper(key) == upperObjName) {
            objectInfo.objectName = key;
            // markObjectAsUsed(objectInfo.objectType, objectInfo.objectName);
            auto const find_unused = unusedInputs.find(objectInfo);
            if (find_unused != unusedInputs.end()) {
                unusedInputs.erase(find_unused);
            }
            return val;
        }
    }

    ShowFatalError(state, format(R"(Name "{}" requested was not found in input for ObjectType "{}")", objectInfo.objectType, objectInfo.objectName));
    throw;
}

void InputProcessor::getObjectItem(EnergyPlusData &state,
                                   std::string_view Object,
                                   int const Number,
                                   Array1S_string Alphas,
                                   int &NumAlphas,
                                   Array1D<Real64> &Numbers,
                                   int &NumNumbers,
                                   int &Status,
                                   ObjexxFCL::Optional<Array1D_bool> NumBlank,
                                   ObjexxFCL::Optional<Array1D_bool> AlphaBlank,
                                   ObjexxFCL::Optional<Array1D_string> AlphaFieldNames,
                                   ObjexxFCL::Optional<Array1D_string> NumericFieldNames)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   September 1997
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine gets the 'number' 'object' from the IDFRecord data structure.

    int adjustedNumber = getJSONObjNum(state, std::string(Object), Number); // if incoming input is idf, then use idf object order

    auto objectInfo = ObjectInfo(); // (AUTO_OK_OBJ)
    objectInfo.objectType = Object;
    // auto sorted_iterators = find_iterators;

    auto find_iterators = objectCacheMap.find(std::string(Object));
    if (find_iterators == objectCacheMap.end()) {
        auto const tmp_umit = caseInsensitiveObjectMap.find(convertToUpper(Object));
        if (tmp_umit == caseInsensitiveObjectMap.end() || epJSON.find(tmp_umit->second) == epJSON.end()) {
            return;
        }
        objectInfo.objectType = tmp_umit->second;
        find_iterators = objectCacheMap.find(objectInfo.objectType);
    }

    NumAlphas = 0;
    NumNumbers = 0;
    Status = -1;
    bool const is_AlphaBlank = present(AlphaBlank);
    bool const is_AlphaFieldNames = present(AlphaFieldNames);
    bool const is_NumBlank = present(NumBlank);
    bool const is_NumericFieldNames = present(NumericFieldNames);

    auto const &epJSON_it = find_iterators->second.inputObjectIterators.at(adjustedNumber - 1);
    auto const &epJSON_schema_it = find_iterators->second.schemaIterator;
    auto const &epJSON_schema_it_val = epJSON_schema_it.value();

    // Locations in JSON schema relating to normal fields
    auto const &schema_obj_props = getPatternProperties(state, epJSON_schema_it_val);

    // Locations in JSON schema storing the positional aspects from the IDD format, legacy prefixed
    auto const &legacy_idd = epJSON_schema_it_val["legacy_idd"];
    auto const &legacy_idd_field_info = legacy_idd["field_info"];
    auto const &legacy_idd_fields = legacy_idd["fields"];
    auto const schema_name_field = epJSON_schema_it_val.find("name");
    auto const has_idd_name_field = schema_name_field != epJSON_schema_it_val.end();
    auto const found_min_fields = epJSON_schema_it_val.find("min_fields");
    size_t min_fields = 0;
    if (found_min_fields != epJSON_schema_it_val.end()) {
        min_fields = found_min_fields.value().get<size_t>();
    }

    auto key = legacy_idd.find("extension");
    std::string extension_key;
    if (key != legacy_idd.end()) {
        extension_key = key.value().get<std::string>();
    }

    auto const &obj = epJSON_it;
    auto const &obj_val = obj.value();
    objectInfo.objectName = obj.key();

    int alpha_index = 1;
    int numeric_index = 1;
    InputProcessor::MaxFields maxFields = findMaxFields(state, obj_val, extension_key, legacy_idd, min_fields);

    Alphas = "";
    Numbers = 0;
    if (is_NumBlank) {
        NumBlank() = true;
    }
    if (is_AlphaBlank) {
        AlphaBlank() = true;
    }
    if (is_AlphaFieldNames) {
        AlphaFieldNames() = "";
    }
    if (is_NumericFieldNames) {
        NumericFieldNames() = "";
    }

    auto const find_unused = unusedInputs.find(objectInfo);
    if (find_unused != unusedInputs.end()) {
        unusedInputs.erase(find_unused);
    }

    for (size_t i = 0; i < legacy_idd_fields.size(); ++i) {
        std::string const field = legacy_idd_fields[i].get<std::string>();
        auto const field_info = legacy_idd_field_info.find(field);
        auto const &field_info_val = field_info.value();
        if (field_info == legacy_idd_field_info.end()) {
            ShowFatalError(state, fmt::format(R"(Could not find field = "{}" in "{}" in epJSON Schema.)", field, Object));
        }

        bool within_idf_fields = (i < maxFields.max_fields);

        if (has_idd_name_field && field == "name") {
            auto const &name_iter = schema_name_field.value();
            if (name_iter.find("retaincase") != name_iter.end()) {
                Alphas(alpha_index) = objectInfo.objectName;
            } else {
                Alphas(alpha_index) = Util::makeUPPER(objectInfo.objectName);
            }
            if (is_AlphaBlank) {
                AlphaBlank()(alpha_index) = objectInfo.objectName.empty();
            }
            if (is_AlphaFieldNames) {
                AlphaFieldNames()(alpha_index) = (state.dataGlobal->isEpJSON) ? field : field_info_val.at("field_name").get<std::string>();
            }
            NumAlphas++;
            alpha_index++;
            continue;
        }

        setObjectItemValue(state,
                           obj_val,
                           schema_obj_props,
                           field,
                           field_info_val,
                           alpha_index,
                           numeric_index,
                           within_idf_fields,
                           Alphas,
                           NumAlphas,
                           Numbers,
                           NumNumbers,
                           NumBlank,
                           AlphaBlank,
                           AlphaFieldNames,
                           NumericFieldNames);
    }

    auto const legacy_idd_extensibles_iter = legacy_idd.find("extensibles");
    if (legacy_idd_extensibles_iter != legacy_idd.end()) {
        size_t extensible_count = 0;
        auto const epJSON_extensions_array_itr = obj_val.find(extension_key);
        if (epJSON_extensions_array_itr != obj_val.end()) {
            auto const &legacy_idd_extensibles = legacy_idd_extensibles_iter.value();
            auto const &epJSON_extensions_array = epJSON_extensions_array_itr.value();
            auto const &schema_extension_fields = schema_obj_props[extension_key]["items"]["properties"];

            for (auto it = epJSON_extensions_array.begin(); it != epJSON_extensions_array.end(); ++it) {
                auto const &epJSON_extension_obj = it.value();
                for (size_t i = 0; i < legacy_idd_extensibles.size(); i++, extensible_count++) {
                    std::string const field_name = legacy_idd_extensibles[i].get<std::string>();
                    auto const field_info = legacy_idd_field_info.find(field_name);
                    auto const &field_info_val = field_info.value();

                    if (field_info == legacy_idd_field_info.end()) {
                        ShowFatalError(state, fmt::format(R"(Could not find field = "{}" in "{}" in epJSON Schema.)", field_name, Object));
                    }

                    bool within_idf_extensible_fields = (extensible_count < maxFields.max_extensible_fields);

                    setObjectItemValue(state,
                                       epJSON_extension_obj,
                                       schema_extension_fields,
                                       field_name,
                                       field_info_val,
                                       alpha_index,
                                       numeric_index,
                                       within_idf_extensible_fields,
                                       Alphas,
                                       NumAlphas,
                                       Numbers,
                                       NumNumbers,
                                       NumBlank,
                                       AlphaBlank,
                                       AlphaFieldNames,
                                       NumericFieldNames);
                }
            }
        }
    }

    Status = 1;
}

int InputProcessor::getIDFObjNum(EnergyPlusData &state, std::string_view Object, int const Number)
{
    // Given the number (index) of an object in JSON order, return it's number in original idf order

    // Only applicable if the incoming file was idf
    int idfOrderNumber = Number;
    if (state.dataGlobal->isEpJSON || !state.dataGlobal->preserveIDFOrder) {
        return idfOrderNumber;
    }

    json *obj;
    auto obj_iter = epJSON.find(std::string(Object));
    if (obj_iter == epJSON.end()) {
        auto tmp_umit = caseInsensitiveObjectMap.find(convertToUpper(Object));
        if (tmp_umit == caseInsensitiveObjectMap.end()) {
            return idfOrderNumber;
        }
        obj = &epJSON[tmp_umit->second];
    } else {
        obj = &(obj_iter.value());
    }

    std::vector<int> idfObjNums;
    std::vector<int> idfObjNumsSorted;

    // get list of saved object numbers from idf processing
    for (auto it = obj->begin(); it != obj->end(); ++it) {
        int objNum = it.value()["idf_order"].get<int>();
        idfObjNums.emplace_back(objNum);
    }

    idfObjNumsSorted = idfObjNums;
    std::sort(idfObjNumsSorted.begin(), idfObjNumsSorted.end());

    // find matching object number in unsorted list
    int targetIdfObjNum = idfObjNums[Number - 1];
    for (size_t i = 1; i <= idfObjNums.size(); ++i) {
        if (idfObjNumsSorted[i - 1] == targetIdfObjNum) {
            idfOrderNumber = i;
            break;
        }
    }
    return idfOrderNumber;
}

std::vector<std::string> InputProcessor::getIDFOrderedKeys(EnergyPlusData &state, std::string_view const Object)
{
    // Given the number (index) of an object in JSON order, return it's number in original idf order
    std::vector<std::string> keys;
    std::vector<int> nums;

    json *obj;
    auto obj_iter = epJSON.find(std::string(Object));
    if (obj_iter == epJSON.end()) {
        auto tmp_umit = caseInsensitiveObjectMap.find(convertToUpper(Object));
        if (tmp_umit == caseInsensitiveObjectMap.end()) {
            return keys;
        }
        obj = &epJSON[tmp_umit->second];
    } else {
        obj = &(obj_iter.value());
    }

    // Return names in JSON order
    if (state.dataGlobal->isEpJSON || !state.dataGlobal->preserveIDFOrder) {
        for (auto it = obj->begin(); it != obj->end(); ++it) {
            keys.emplace_back(it.key());
        }

        return keys;
    }

    // Now, the real work begins

    for (auto it = obj->begin(); it != obj->end(); ++it) {
        nums.push_back(it.value()["idf_order"].get<int>());
    }
    std::sort(nums.begin(), nums.end());

    // Reserve doesn't seem to work :(
    for (int i = 0; i < (int)nums.size(); ++i) {
        keys.push_back("");
    }

    // get list of saved object numbers from idf processing
    for (auto it = obj->begin(); it != obj->end(); ++it) {
        int objNum = it.value()["idf_order"].get<int>();
        int objIdx = std::find(nums.begin(), nums.end(), objNum) - nums.begin();
        keys[objIdx] = it.key();
    }

    return keys;
}

int InputProcessor::getJSONObjNum(EnergyPlusData &state, std::string const &Object, int const Number)
{
    // Given the number (index) of an object in original idf order, return it's number in JSON order

    // Only applicable if the incoming file was idf
    int jSONOrderNumber = Number;
    if (state.dataGlobal->isEpJSON || !state.dataGlobal->preserveIDFOrder) {
        return jSONOrderNumber;
    }

    json *obj;
    auto obj_iter = epJSON.find(Object);
    if (obj_iter == epJSON.end()) {
        auto tmp_umit = caseInsensitiveObjectMap.find(convertToUpper(Object));
        if (tmp_umit == caseInsensitiveObjectMap.end()) {
            return jSONOrderNumber;
        }
        obj = &epJSON[tmp_umit->second];
    } else {
        obj = &(obj_iter.value());
    }

    std::vector<int> idfObjNums;
    std::vector<int> idfObjNumsSorted;

    // get list of saved object numbers from idf processing
    for (auto it = obj->begin(); it != obj->end(); ++it) {
        int objNum = it.value()["idf_order"].get<int>();
        idfObjNums.emplace_back(objNum);
    }

    idfObjNumsSorted = idfObjNums;
    std::sort(idfObjNumsSorted.begin(), idfObjNumsSorted.end());

    // find matching object number in unsorted list
    int targetIdfObjNum = idfObjNumsSorted[Number - 1];
    for (size_t i = 1; i <= idfObjNums.size(); ++i) {
        if (idfObjNums[i - 1] == targetIdfObjNum) {
            jSONOrderNumber = i;
            break;
        }
    }
    return jSONOrderNumber;
}

int InputProcessor::getObjectItemNum(EnergyPlusData &state,
                                     std::string_view ObjType, // Object Type (ref: IDD Objects)
                                     std::string_view ObjName  // Name of the object type
)
{
    // PURPOSE OF THIS SUBROUTINE:
    // Get the occurrence number of an object of type ObjType and name ObjName

    json *obj;
    auto obj_iter = epJSON.find(std::string(ObjType));
    if (obj_iter == epJSON.end() || obj_iter.value().find(std::string(ObjName)) == obj_iter.value().end()) {
        auto tmp_umit = caseInsensitiveObjectMap.find(convertToUpper(ObjType));
        if (tmp_umit == caseInsensitiveObjectMap.end()) {
            return -1; // indicates object type not found, see function GeneralRoutines::ValidateComponent
        }
        obj = &epJSON[tmp_umit->second];
    } else {
        obj = &(obj_iter.value());
    }

    int object_item_num = 1;
    bool found = false;
    std::string const upperObjName = Util::makeUPPER(ObjName);
    for (auto it = obj->begin(); it != obj->end(); ++it) {
        if (Util::makeUPPER(it.key()) == upperObjName) {
            found = true;
            break;
        }
        object_item_num++;
    }

    if (!found) {
        return 0; // indicates object name not found, see function GeneralRoutines::ValidateComponent
    }
    return getIDFObjNum(state, std::string(ObjType), object_item_num); // if incoming input is idf, then return idf object order
}

int InputProcessor::getObjectItemNum(EnergyPlusData &state,
                                     std::string_view ObjType,       // Object Type (ref: IDD Objects)
                                     std::string const &NameTypeVal, // Object "name" field type ( used as search key )
                                     std::string const &ObjName      // Name of the object type
)
{
    // PURPOSE OF THIS SUBROUTINE:
    // Get the occurrence number of an object of type ObjType and name ObjName

    json *obj;
    auto obj_iter = epJSON.find(std::string(ObjType));
    if (obj_iter == epJSON.end() || obj_iter.value().find(ObjName) == obj_iter.value().end()) {
        auto tmp_umit = caseInsensitiveObjectMap.find(convertToUpper(ObjType));
        if (tmp_umit == caseInsensitiveObjectMap.end()) {
            return -1; // indicates object type not found, see function GeneralRoutines::ValidateComponent
        }
        obj = &epJSON[tmp_umit->second];
    } else {
        obj = &(obj_iter.value());
    }

    int object_item_num = 1;
    bool found = false;
    std::string const upperObjName = Util::makeUPPER(ObjName);
    for (auto it = obj->begin(); it != obj->end(); ++it) {
        auto it2 = it.value().find(NameTypeVal);

        if ((it2 != it.value().end()) && (Util::makeUPPER(it2.value().get<std::string>()) == upperObjName)) {
            found = true;
            break;
        }
        object_item_num++;
    }

    if (!found) {
        return 0; // indicates object field name or value not found
    }
    return getIDFObjNum(state, ObjType, object_item_num); // if incoming input is idf, then return idf object order
}

void InputProcessor::getMaxSchemaArgs(int &NumArgs, int &NumAlpha, int &NumNumeric)
{
    NumArgs = 0;
    NumAlpha = 0;
    NumNumeric = 0;
    std::string extension_key;
    auto const &schema_properties = schema().at("properties");

    for (json::iterator object = epJSON.begin(); object != epJSON.end(); ++object) {
        int num_alpha = 0;
        int num_numeric = 0;

        const json &legacy_idd = schema_properties.at(object.key()).at("legacy_idd");
        auto key = legacy_idd.find("extension");
        if (key != legacy_idd.end()) {
            extension_key = key.value().get<std::string>();
        }

        size_t max_size = 0;
        for (auto const &obj : object.value()) {
            auto const find_extensions = obj.find(extension_key);
            if (find_extensions != obj.end()) {
                size_t const size = find_extensions.value().size();
                if (size > max_size) {
                    max_size = size;
                }
            }
        }

        auto const find_alphas = legacy_idd.find("alphas");
        if (find_alphas != legacy_idd.end()) {
            json const &alphas = find_alphas.value();
            auto const find_fields = alphas.find("fields");
            if (find_fields != alphas.end()) {
                num_alpha += find_fields.value().size();
            }
            if (alphas.find("extensions") != alphas.end()) {
                num_alpha += alphas["extensions"].size() * max_size;
            }
        }
        if (legacy_idd.find("numerics") != legacy_idd.end()) {
            json const &numerics = legacy_idd["numerics"];
            if (numerics.find("fields") != numerics.end()) {
                num_numeric += numerics["fields"].size();
            }
            if (numerics.find("extensions") != numerics.end()) {
                num_numeric += numerics["extensions"].size() * max_size;
            }
        }
        if (num_alpha > NumAlpha) {
            NumAlpha = num_alpha;
        }
        if (num_numeric > NumNumeric) {
            NumNumeric = num_numeric;
        }
    }

    NumArgs = NumAlpha + NumNumeric;
}

void InputProcessor::getObjectDefMaxArgs(EnergyPlusData &state,
                                         std::string_view const ObjectWord, // Object for definition
                                         int &NumArgs,                      // How many arguments (max) this Object can have
                                         int &NumAlpha,                     // How many Alpha arguments (max) this Object can have
                                         int &NumNumeric                    // How many Numeric arguments (max) this Object can have
)
{
    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine returns maximum argument limits (total, alphas, numerics) of an Object from the IDD.
    // These dimensions (not sure what one can use the total for) can be used to dynamically dimension the
    // arrays in the GetInput routines.

    NumArgs = 0;
    NumAlpha = 0;
    NumNumeric = 0;
    const json *object;

    auto const &props = schema()["properties"];

    if (auto found = props.find(std::string(ObjectWord)); found == props.end()) {
        auto tmp_umit = caseInsensitiveObjectMap.find(convertToUpper(ObjectWord));
        if (tmp_umit == caseInsensitiveObjectMap.end()) {
            ShowSevereError(state, fmt::format(R"(getObjectDefMaxArgs: Did not find object="{}" in list of objects.)", ObjectWord));
            return;
        }
        object = &props[tmp_umit->second];
    } else {
        object = &found.value();
    }

    const json &legacy_idd = object->at("legacy_idd");
    json *objects;
    if (auto found = epJSON.find(std::string(ObjectWord)); found == epJSON.end()) {
        auto tmp_umit = caseInsensitiveObjectMap.find(convertToUpper(ObjectWord));
        if (tmp_umit == caseInsensitiveObjectMap.end()) {
            ShowSevereError(state, fmt::format(R"(getObjectDefMaxArgs: Did not find object="{}" in list of objects.)", ObjectWord));
            return;
        }
        objects = &epJSON[tmp_umit->second];
    } else {
        objects = &found.value();
    }

    size_t max_size = 0;

    std::string extension_key;
    auto key = legacy_idd.find("extension");
    if (key != legacy_idd.end()) {
        extension_key = key.value().get<std::string>();
    }

    for (auto const &obj : *objects) {
        if (auto found = obj.find(extension_key); found != obj.end()) {
            size_t const size = found.value().size();
            if (size > max_size) {
                max_size = size;
            }
        }
    }

    if (auto found = legacy_idd.find("alphas"); found != legacy_idd.end()) {
        json const &alphas = found.value();
        if (auto found2 = alphas.find("fields"); found2 != alphas.end()) {
            NumAlpha += found2.value().size();
        }
        if (auto found2 = alphas.find("extensions"); found2 != alphas.end()) {
            NumAlpha += found2.value().size() * max_size;
        }
    }
    if (auto found = legacy_idd.find("numerics"); found != legacy_idd.end()) {
        json const &numerics = found.value();
        if (auto found2 = numerics.find("fields"); found2 != numerics.end()) {
            NumNumeric += found2.value().size();
        }
        if (auto found2 = numerics.find("extensions"); found2 != numerics.end()) {
            NumNumeric += found2.value().size() * max_size;
        }
    }
    NumArgs = NumAlpha + NumNumeric;
}

void InputProcessor::reportIDFRecordsStats(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION: (previously called GetIDFRecordsStats)
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   February 2009
    //       MODIFIED       na
    //       RE-ENGINEERED  Julien Marrec of EffiBEM, 2020 (ported to the new InputProcessor/epJSON)

    // PURPOSE OF THIS SUBROUTINE:
    // This routine provides some statistics on the current IDF, such as number of records, total fields with defaults,
    // number of fields that overrode the default (even if it was default value), and similarly for AutoSize.

    // METHODOLOGY EMPLOYED:
    // Traverses the IDF Records looking at each field vs object definition for defaults and autosize.

    // Reset the globals
    state.dataOutput->iNumberOfRecords = 0;             // Number of IDF Records
    state.dataOutput->iNumberOfDefaultedFields = 0;     // Number of defaulted fields in IDF
    state.dataOutput->iTotalFieldsWithDefaults = 0;     // Total number of fields that could be defaulted
    state.dataOutput->iNumberOfAutoSizedFields = 0;     // Number of autosized fields in IDF
    state.dataOutput->iTotalAutoSizableFields = 0;      // Total number of autosizable fields
    state.dataOutput->iNumberOfAutoCalcedFields = 0;    // Number of autocalculated fields
    state.dataOutput->iTotalAutoCalculatableFields = 0; // Total number of autocalculatable fields

    auto const &schema_properties = schema().at("properties");

    // Lambda to avoid repeating code twice (when processing regular fields, and extensible fields)
    auto processField = [&state](const std::string &field, const json &epJSONObj, const json &schema_field_obj) {
        bool hasDefault = false;
        bool canBeAutosized = false;
        bool canBeAutocalculated = false;

        // If we wanted to count number of fields, would do it here

        std::string defaultValue;

        auto const default_it = schema_field_obj.find("default");
        if (default_it != schema_field_obj.end()) {
            ++state.dataOutput->iTotalFieldsWithDefaults;
            hasDefault = true;
            auto const &default_val = default_it.value();
            if (default_val.is_string()) {
                defaultValue = default_val.get<std::string>();
            }
        }

        auto const anyOf_it = schema_field_obj.find("anyOf");
        if (anyOf_it != schema_field_obj.end()) {
            for (auto const &anyOf : anyOf_it.value()) {
                auto const enum_it = anyOf.find("enum");
                if (enum_it != anyOf.end()) {
                    for (auto const &e : enum_it.value()) {
                        if (e.is_string()) {
                            std::string const enumVal = e.get<std::string>();
                            if (enumVal == "Autosize") {
                                ++state.dataOutput->iTotalAutoSizableFields;
                                canBeAutosized = true;
                            } else if (enumVal == "Autocalculate") {
                                ++state.dataOutput->iTotalAutoCalculatableFields;
                                canBeAutocalculated = true;
                            }
                        }
                    }
                }
            }
        }

        // Locate the field in the ep_object
        auto it = epJSONObj.find(field);
        if (it != epJSONObj.end()) { // && !it.value().empty()) {
            // Found it: check if Autosized or Autocalculated
            auto const &field_value = it.value();
            if (field_value.is_string()) {
                std::string const val = field_value.get<std::string>();
                // In the IDF, casing is an issue and Autosize/Autocalculate are accepted as synonyms
                // but once converted to epJSON everything should is resolved, eg:
                // * if "AutoSize" is entered for an autosizable field, the result is "Autosize"
                // * if "AutoSize" is entered for an autocalculatable field, the result is "Autocalculate"
                if (canBeAutosized && (val == "Autosize")) {
                    ++state.dataOutput->iNumberOfAutoSizedFields;
                } else if (canBeAutocalculated && (val == "Autocalculate")) {
                    ++state.dataOutput->iNumberOfAutoCalcedFields;
                }
            }
        } else if (hasDefault) {
            // Not found: was defaulted
            ++state.dataOutput->iNumberOfDefaultedFields;
            if (canBeAutosized && (defaultValue == "Autosize")) {
                ++state.dataOutput->iNumberOfAutoSizedFields;
            } else if (canBeAutocalculated && (defaultValue == "Autocalculate")) {
                ++state.dataOutput->iNumberOfAutoCalcedFields;
            }
        }
    };

    // Loop on all objectTypes
    for (auto epJSON_iter = epJSON.begin(); epJSON_iter != epJSON.end(); ++epJSON_iter) {
        auto const &objectType = epJSON_iter.key();
        auto const &objects = epJSON_iter.value();

        const json &object_schema = schema_properties.at(objectType);

        // Locations in JSON schema relating to normal fields
        auto const &schema_obj_props = getPatternProperties(state, object_schema);
        auto const schema_name_field = object_schema.find("name");
        bool const has_idd_name_field = schema_name_field != object_schema.end();

        // Locations in JSON schema storing the positional aspects from the IDD format, legacy prefixed
        auto const &legacy_idd = object_schema["legacy_idd"];
        auto const &legacy_idd_fields = legacy_idd["fields"];

        // Look for extensible
        auto key = legacy_idd.find("extension");
        std::string extension_key;
        if (key != legacy_idd.end()) {
            extension_key = key.value().get<std::string>();
        }

        for (auto const &ep_object : objects) {

            // Count number of objects
            ++state.dataOutput->iNumberOfRecords;

            // Loop on all regular fields
            for (size_t i = 0; i < legacy_idd_fields.size(); ++i) {

                std::string const field = legacy_idd_fields[i].get<std::string>();

                // This is weird, but some objects like Building have a Name default... and it's not in the patternProperties
                if (has_idd_name_field && field == "name") {
                    auto const &name_iter = schema_name_field.value();
                    if (name_iter.find("default") != name_iter.end()) {
                        ++state.dataOutput->iTotalFieldsWithDefaults;
                        auto it = ep_object.find(field);
                        if (it == ep_object.end()) {
                            ++state.dataOutput->iNumberOfDefaultedFields;
                        }
                    }
                    continue;
                }

                auto const &schema_field_obj = schema_obj_props[field];

                processField(field, ep_object, schema_field_obj);

            } // End regular fields

            auto const legacy_idd_extensibles_iter = legacy_idd.find("extensibles");
            if (legacy_idd_extensibles_iter != legacy_idd.end()) {
                auto const epJSON_extensions_array_itr = ep_object.find(extension_key);
                if (epJSON_extensions_array_itr != ep_object.end()) {
                    auto const &legacy_idd_extensibles = legacy_idd_extensibles_iter.value();
                    auto const &epJSON_extensions_array = epJSON_extensions_array_itr.value();
                    auto const &schema_extension_fields = schema_obj_props[extension_key]["items"]["properties"];

                    for (auto it = epJSON_extensions_array.begin(); it != epJSON_extensions_array.end(); ++it) {
                        auto const &epJSON_extension_obj = it.value();
                        for (size_t i = 0; i < legacy_idd_extensibles.size(); ++i) {
                            std::string const &field = legacy_idd_extensibles[i].get<std::string>();
                            auto const &schema_extension_field_obj = schema_extension_fields[field];

                            processField(field, epJSON_extension_obj, schema_extension_field_obj);
                        }
                    }
                }
            } // End extensible fields

        } // End loop on each object of a given objectType
    } // End loop on all objectTypes
}

void InputProcessor::reportOrphanRecordObjects(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   August 2002
    //       MODIFIED       na
    //       RE-ENGINEERED  Mark Adams, Oct 2016

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine reports "orphan" objects that are in the input but were
    // not "gotten" during the simulation.

    std::unordered_set<std::string> unused_object_types;
    unused_object_types.reserve(unusedInputs.size());

    if (unusedInputs.size() && state.dataGlobal->DisplayUnusedObjects) {
        ShowWarningError(state, "The following lines are \"Unused Objects\".  These objects are in the input");
        ShowContinueError(state, " file but are never obtained by the simulation and therefore are NOT used.");
        if (!state.dataGlobal->DisplayAllWarnings) {
            ShowContinueError(
                state, " Only the first unused named object of an object class is shown.  Use Output:Diagnostics,DisplayAllWarnings; to see all.");
        } else {
            ShowContinueError(state, " Each unused object is shown.");
        }
        ShowContinueError(state, " See InputOutputReference document for more details.");
    }

    bool first_iteration = true;
    for (auto it = unusedInputs.begin(); it != unusedInputs.end(); ++it) {
        std::string const &object_type = it->objectType;
        std::string const &name = it->objectName;

        // there are some orphans that we are deeming as special, in that they should be warned in detail even if !DisplayUnusedObjects and
        // !DisplayAllWarnings
        if (has_prefix(object_type, "ZoneHVAC:")) {
            ShowSevereError(state, "Orphaned ZoneHVAC object found.  This was object never referenced in the input, and was not used.");
            ShowContinueError(state, " -- Object type: " + object_type);
            ShowContinueError(state, " -- Object name: " + name);
        }

        if (!state.dataGlobal->DisplayUnusedObjects) {
            continue;
        }

        if (!state.dataGlobal->DisplayAllWarnings) {
            auto found_type = unused_object_types.find(object_type);
            if (found_type != unused_object_types.end()) {
                // only show first unused named object of an object class
                continue;
            } else {
                unused_object_types.emplace(object_type);
            }
        }

        if (first_iteration) {
            if (!name.empty()) {
                ShowMessage(state, "Object=" + object_type + '=' + name);
            } else {
                ShowMessage(state, "Object=" + object_type);
            }
            first_iteration = false;
        } else {
            if (!name.empty()) {
                ShowContinueError(state, "Object=" + object_type + '=' + name);
            } else {
                ShowContinueError(state, "Object=" + object_type);
            }
        }
    }

    if (unusedInputs.size() && !state.dataGlobal->DisplayUnusedObjects) {
        u64toa(unusedInputs.size(), s);
        ShowMessage(state, "There are " + std::string(s) + " unused objects in input.");
        ShowMessage(state, "Use Output:Diagnostics,DisplayUnusedObjects; to see them.");
    }
}

void InputProcessor::preProcessorCheck(EnergyPlusData &state, bool &PreP_Fatal) // True if a preprocessor flags a fatal error
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   August 2005
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This routine checks for existence of "Preprocessor Message" object and
    // performs appropriate action.

    // METHODOLOGY EMPLOYED:
    // na

    // REFERENCES:
    // Preprocessor Message,
    //    \memo This object does not come from a user input.  This is generated by a pre-processor
    //    \memo so that various conditions can be gracefully passed on by the InputProcessor.
    //    A1,        \field preprocessor name
    //    A2,        \field error severity
    //               \note Depending on type, InputProcessor may terminate the program.
    //               \type choice
    //               \key warning
    //               \key severe
    //               \key fatal
    //    A3,        \field message line 1
    //    A4,        \field message line 2
    //    A5,        \field message line 3
    //    A6,        \field message line 4
    //    A7,        \field message line 5
    //    A8,        \field message line 6
    //    A9,        \field message line 7
    //    A10,       \field message line 8
    //    A11,       \field message line 9
    //    A12;       \field message line 10

    state.dataIPShortCut->cCurrentModuleObject = "Output:PreprocessorMessage";
    int NumPrePM = getNumObjectsFound(state, state.dataIPShortCut->cCurrentModuleObject);
    if (NumPrePM > 0) {
        int NumAlphas;  // Used to retrieve names from IDF
        int NumNumbers; // Used to retrieve rNumericArgs from IDF
        int IOStat;     // Could be used in the Get Routines, not currently checked
        int NumParams;  // Total Number of Parameters in 'Output:PreprocessorMessage' Object
        std::string Multiples;

        getObjectDefMaxArgs(state, state.dataIPShortCut->cCurrentModuleObject, NumParams, NumAlphas, NumNumbers);
        state.dataIPShortCut->cAlphaArgs({1, NumAlphas}) = BlankString;
        for (int CountP = 1; CountP <= NumPrePM; ++CountP) {
            getObjectItem(state,
                          state.dataIPShortCut->cCurrentModuleObject,
                          CountP,
                          state.dataIPShortCut->cAlphaArgs,
                          NumAlphas,
                          state.dataIPShortCut->rNumericArgs,
                          NumNumbers,
                          IOStat,
                          state.dataIPShortCut->lNumericFieldBlanks,
                          state.dataIPShortCut->lAlphaFieldBlanks,
                          state.dataIPShortCut->cAlphaFieldNames,
                          state.dataIPShortCut->cNumericFieldNames);
            if (state.dataIPShortCut->cAlphaArgs(1).empty()) {
                state.dataIPShortCut->cAlphaArgs(1) = "Unknown";
            }
            if (NumAlphas > 3) {
                Multiples = "s";
            } else {
                Multiples = BlankString;
            }
            if (state.dataIPShortCut->cAlphaArgs(2).empty()) {
                state.dataIPShortCut->cAlphaArgs(2) = "Unknown";
            }
            {
                std::string const errorType = uppercased(state.dataIPShortCut->cAlphaArgs(2));
                if (errorType == "INFORMATION") {
                    ShowMessage(state,
                                state.dataIPShortCut->cCurrentModuleObject + "=\"" + state.dataIPShortCut->cAlphaArgs(1) +
                                    "\" has the following Information message" + Multiples + ':');
                } else if (errorType == "WARNING") {
                    ShowWarningError(state,
                                     state.dataIPShortCut->cCurrentModuleObject + "=\"" + state.dataIPShortCut->cAlphaArgs(1) +
                                         "\" has the following Warning condition" + Multiples + ':');
                } else if (errorType == "SEVERE") {
                    ShowSevereError(state,
                                    state.dataIPShortCut->cCurrentModuleObject + "=\"" + state.dataIPShortCut->cAlphaArgs(1) +
                                        "\" has the following Severe condition" + Multiples + ':');
                } else if (errorType == "FATAL") {
                    ShowSevereError(state,
                                    state.dataIPShortCut->cCurrentModuleObject + "=\"" + state.dataIPShortCut->cAlphaArgs(1) +
                                        "\" has the following Fatal condition" + Multiples + ':');
                    PreP_Fatal = true;
                } else {
                    ShowSevereError(state,
                                    state.dataIPShortCut->cCurrentModuleObject + "=\"" + state.dataIPShortCut->cAlphaArgs(1) +
                                        "\" has the following " + state.dataIPShortCut->cAlphaArgs(2) + " condition" + Multiples + ':');
                }
            }
            int CountM = 3;
            if (CountM > NumAlphas) {
                ShowContinueError(state,
                                  state.dataIPShortCut->cCurrentModuleObject + " was blank.  Check " + state.dataIPShortCut->cAlphaArgs(1) +
                                      " audit trail or error file for possible reasons.");
            }
            while (CountM <= NumAlphas) {
                if (len(state.dataIPShortCut->cAlphaArgs(CountM)) == Constant::MaxNameLength) {
                    ShowContinueError(state, state.dataIPShortCut->cAlphaArgs(CountM) + state.dataIPShortCut->cAlphaArgs(CountM + 1));
                    CountM += 2;
                } else {
                    ShowContinueError(state, state.dataIPShortCut->cAlphaArgs(CountM));
                    ++CountM;
                }
            }
        }
    }
}

void InputProcessor::preScanReportingVariables(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   July 2010

    // PURPOSE OF THIS SUBROUTINE:
    // This routine scans the input records and determines which output variables
    // are actually being requested for the run so that the OutputProcessor will only
    // consider those variables for output.  (At this time, all metered variables are
    // allowed to pass through).

    // This routine also scans any variables requested by API call for library usage.
    // These variables are stored in a vector in output processor, and the values are added before E+ begins.

    // METHODOLOGY EMPLOYED:
    // Uses internal records and structures.
    // Looks at:
    // Output:Variable
    // Meter:Custom
    // Meter:CustomDecrement
    // Output:Table:Monthly
    // Output:Table:TimeBins
    // Output:Table:SummaryReports
    // EnergyManagementSystem:Sensor
    // EnergyManagementSystem:OutputVariable

    // SUBROUTINE PARAMETER DEFINITIONS:
    static std::string const OutputVariable("Output:Variable");
    static std::string const MeterCustom("Meter:Custom");
    static std::string const MeterCustomDecrement("Meter:CustomDecrement");
    static std::string const OutputTableMonthly("Output:Table:Monthly");
    static std::string const OutputTableAnnual("Output:Table:Annual");
    static std::string const OutputTableTimeBins("Output:Table:TimeBins");
    static std::string const OutputTableSummaries("Output:Table:SummaryReports");
    static std::string const EMSSensor("EnergyManagementSystem:Sensor");
    static std::string const EMSOutputVariable("EnergyManagementSystem:OutputVariable");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    std::string extension_key;
    // state.dataOutput->OutputVariablesForSimulation.reserve(1024);
    state.dataOutput->MaxConsideredOutputVariables = 10000;

    // Output Variable
    auto epJSON_objects = epJSON.find(OutputVariable);
    if (epJSON_objects != epJSON.end()) {
        auto const &epJSON_object = epJSON_objects.value();
        for (auto obj = epJSON_object.begin(); obj != epJSON_object.end(); ++obj) {
            json const &fields = obj.value();
            auto it = fields.find("key_value");
            if (it != fields.end() && !it.value().empty()) {
                addRecordToOutputVariableStructure(state, it.value().get<std::string>(), fields.at("variable_name").get<std::string>());
            } else {
                addRecordToOutputVariableStructure(state, "*", fields.at("variable_name").get<std::string>());
            }
        }
    }

    epJSON_objects = epJSON.find(MeterCustom);
    if (epJSON_objects != epJSON.end()) {
        auto const &epJSON_object = epJSON_objects.value();
        auto const &legacy_idd = schema()["properties"][MeterCustom]["legacy_idd"];
        auto key = legacy_idd.find("extension");
        if (key != legacy_idd.end()) {
            extension_key = key.value().get<std::string>();
        }
        for (auto obj = epJSON_object.begin(); obj != epJSON_object.end(); ++obj) {
            json const &fields = obj.value();
            for (auto const &extensions : fields[extension_key]) {
                auto it = extensions.find("key_name");
                if (it != extensions.end() && !obj.key().empty()) {
                    addRecordToOutputVariableStructure(
                        state, it.value().get<std::string>(), extensions.at("output_variable_or_meter_name").get<std::string>());
                } else {
                    addRecordToOutputVariableStructure(state, "*", extensions.at("output_variable_or_meter_name").get<std::string>());
                }
            }
        }
    }

    epJSON_objects = epJSON.find(MeterCustomDecrement);
    if (epJSON_objects != epJSON.end()) {
        auto const &epJSON_object = epJSON_objects.value();
        auto const &legacy_idd = schema()["properties"][MeterCustomDecrement]["legacy_idd"];
        auto key = legacy_idd.find("extension");
        if (key != legacy_idd.end()) {
            extension_key = key.value().get<std::string>();
        }
        for (auto obj = epJSON_object.begin(); obj != epJSON_object.end(); ++obj) {
            json const &fields = obj.value();
            for (auto const &extensions : fields[extension_key]) {
                auto it = extensions.find("key_name");
                if (it != extensions.end() && !obj.key().empty()) {
                    addRecordToOutputVariableStructure(
                        state, it.value().get<std::string>(), extensions.at("output_variable_or_meter_name").get<std::string>());
                } else {
                    addRecordToOutputVariableStructure(state, "*", extensions.at("output_variable_or_meter_name").get<std::string>());
                }
            }
        }
    }

    epJSON_objects = epJSON.find(EMSSensor);
    if (epJSON_objects != epJSON.end()) {
        auto const &epJSON_object = epJSON_objects.value();
        for (auto obj = epJSON_object.begin(); obj != epJSON_object.end(); ++obj) {
            json const &fields = obj.value();
            auto it = fields.find("output_variable_or_output_meter_index_key_name");
            if (it != fields.end() && !it.value().empty()) {
                addRecordToOutputVariableStructure(
                    state, it.value().get<std::string>(), fields.at("output_variable_or_output_meter_name").get<std::string>());
            } else {
                addRecordToOutputVariableStructure(state, "*", fields.at("output_variable_or_output_meter_name").get<std::string>());
            }
        }
    }

    epJSON_objects = epJSON.find(EMSOutputVariable);
    if (epJSON_objects != epJSON.end()) {
        auto const &epJSON_object = epJSON_objects.value();
        for (auto obj = epJSON_object.begin(); obj != epJSON_object.end(); ++obj) {
            addRecordToOutputVariableStructure(state, "*", obj.key());
        }
    }

    for (auto const &requestedVar : state.dataOutputProcessor->apiVarRequests) {
        addRecordToOutputVariableStructure(state, requestedVar.varKey, requestedVar.varName);
    }

    epJSON_objects = epJSON.find(OutputTableTimeBins);
    if (epJSON_objects != epJSON.end()) {
        auto const &epJSON_object = epJSON_objects.value();
        for (auto obj = epJSON_object.begin(); obj != epJSON_object.end(); ++obj) {
            json const &fields = obj.value();
            if (!obj.key().empty()) {
                addRecordToOutputVariableStructure(state, obj.key(), fields.at("key_value").get<std::string>());
            } else {
                addRecordToOutputVariableStructure(state, "*", fields.at("key_value").get<std::string>());
            }
        }
    }

    epJSON_objects = epJSON.find(OutputTableMonthly);
    if (epJSON_objects != epJSON.end()) {
        auto const &epJSON_object = epJSON_objects.value();
        auto const &legacy_idd = schema()["properties"][OutputTableMonthly]["legacy_idd"];
        auto key = legacy_idd.find("extension");
        if (key != legacy_idd.end()) {
            extension_key = key.value().get<std::string>();
        }
        for (auto obj = epJSON_object.begin(); obj != epJSON_object.end(); ++obj) {
            json const &fields = obj.value();
            for (auto const &extensions : fields[extension_key]) {
                try {
                    addRecordToOutputVariableStructure(state, "*", extensions.at("variable_or_meter_name").get<std::string>());
                } catch (...) {
                    continue; // blank or erroneous fields are handled at the get input function for the object
                }
            }
        }
    }

    epJSON_objects = epJSON.find(OutputTableAnnual);
    if (epJSON_objects != epJSON.end()) {
        auto const &epJSON_object = epJSON_objects.value();
        auto const &legacy_idd = schema()["properties"][OutputTableAnnual]["legacy_idd"];
        auto key = legacy_idd.find("extension");
        if (key != legacy_idd.end()) {
            extension_key = key.value().get<std::string>();
        }
        for (auto obj = epJSON_object.begin(); obj != epJSON_object.end(); ++obj) {
            json const &fields = obj.value();
            for (auto const &extensions : fields[extension_key]) {
                try {
                    addRecordToOutputVariableStructure(
                        state, "*", extensions.at("variable_or_meter_or_ems_variable_or_field_name").get<std::string>());
                } catch (...) {
                    continue; // blank or erroneous fields are handled at the get input function for the object
                }
            }
        }
    }

    epJSON_objects = epJSON.find(OutputTableSummaries);
    if (epJSON_objects != epJSON.end()) {
        auto const &epJSON_object = epJSON_objects.value();
        auto const &legacy_idd = schema()["properties"][OutputTableSummaries]["legacy_idd"];
        auto key = legacy_idd.find("extension");
        if (key != legacy_idd.end()) {
            extension_key = key.value().get<std::string>();
        }
        for (auto obj = epJSON_object.begin(); obj != epJSON_object.end(); ++obj) {
            json const &fields = obj.value();
            for (auto const &extensions : fields[extension_key]) {
                try {
                    std::string const report_name = Util::makeUPPER(extensions.at("report_name").get<std::string>());
                    if (report_name == "ALLMONTHLY" || report_name == "ALLSUMMARYANDMONTHLY") {
                        for (int i = 1; i <= DataOutputs::NumMonthlyReports; ++i) {
                            addVariablesForMonthlyReport(state, DataOutputs::MonthlyNamedReports(i));
                        }
                    } else {
                        addVariablesForMonthlyReport(state, report_name);
                    }
                } catch (...) {
                    continue; // blank or erroneous fields should be warned about during actual get input routines
                }
            }
        }
    }
}

void InputProcessor::addVariablesForMonthlyReport(EnergyPlusData &state, std::string const &reportName)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   July 2010
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This routine adds specific variables to the Output Variables for Simulation
    // Structure. Note that only non-metered variables need to be added here.  Metered
    // variables are automatically included in the minimized output variable structure.

    if (reportName == "ZONECOOLINGSUMMARYMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "ZONE AIR SYSTEM SENSIBLE COOLING RATE");
        addRecordToOutputVariableStructure(state, "*", "SITE OUTDOOR AIR DRYBULB TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "SITE OUTDOOR AIR WETBULB TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "ZONE TOTAL INTERNAL LATENT GAIN ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE TOTAL INTERNAL LATENT GAIN RATE");

    } else if (reportName == "ZONEHEATINGSUMMARYMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "ZONE AIR SYSTEM SENSIBLE HEATING ENERGY"); // on meter
        addRecordToOutputVariableStructure(state, "*", "ZONE AIR SYSTEM SENSIBLE HEATING RATE");
        addRecordToOutputVariableStructure(state, "*", "SITE OUTDOOR AIR DRYBULB TEMPERATURE");

    } else if (reportName == "ZONEELECTRICSUMMARYMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "ZONE LIGHTS ELECTRICITY ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE ELECTRIC EQUIPMENT ELECTRICITY ENERGY");

    } else if (reportName == "SPACEGAINSMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "ZONE PEOPLE TOTAL HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE LIGHTS TOTAL HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE ELECTRIC EQUIPMENT TOTAL HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE GAS EQUIPMENT TOTAL HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE HOT WATER EQUIPMENT TOTAL HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE STEAM EQUIPMENT TOTAL HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE OTHER EQUIPMENT TOTAL HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE INFILTRATION SENSIBLE HEAT GAIN ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE INFILTRATION SENSIBLE HEAT LOSS ENERGY");

    } else if (reportName == "PEAKSPACEGAINSMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "ZONE PEOPLE TOTAL HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE LIGHTS TOTAL HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE ELECTRIC EQUIPMENT TOTAL HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE GAS EQUIPMENT TOTAL HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE HOT WATER EQUIPMENT TOTAL HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE STEAM EQUIPMENT TOTAL HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE OTHER EQUIPMENT TOTAL HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE INFILTRATION SENSIBLE HEAT GAIN ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE INFILTRATION SENSIBLE HEAT LOSS ENERGY");

    } else if (reportName == "SPACEGAINCOMPONENTSATCOOLINGPEAKMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "ZONE AIR SYSTEM SENSIBLE COOLING RATE");
        addRecordToOutputVariableStructure(state, "*", "ZONE PEOPLE TOTAL HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE LIGHTS TOTAL HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE ELECTRIC EQUIPMENT TOTAL HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE GAS EQUIPMENT TOTAL HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE HOT WATER EQUIPMENT TOTAL HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE STEAM EQUIPMENT TOTAL HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE OTHER EQUIPMENT TOTAL HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE INFILTRATION SENSIBLE HEAT GAIN ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE INFILTRATION SENSIBLE HEAT LOSS ENERGY");

    } else if (reportName == "SETPOINTSNOTMETWITHTEMPERATURESMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "ZONE HEATING SETPOINT NOT MET TIME");
        addRecordToOutputVariableStructure(state, "*", "ZONE MEAN AIR TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "ZONE HEATING SETPOINT NOT MET WHILE OCCUPIED TIME");
        addRecordToOutputVariableStructure(state, "*", "ZONE COOLING SETPOINT NOT MET TIME");
        addRecordToOutputVariableStructure(state, "*", "ZONE COOLING SETPOINT NOT MET WHILE OCCUPIED TIME");

    } else if (reportName == "COMFORTREPORTSIMPLE55MONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "ZONE THERMAL COMFORT ASHRAE 55 SIMPLE MODEL SUMMER CLOTHES NOT COMFORTABLE TIME");
        addRecordToOutputVariableStructure(state, "*", "ZONE MEAN AIR TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "ZONE THERMAL COMFORT ASHRAE 55 SIMPLE MODEL WINTER CLOTHES NOT COMFORTABLE TIME");
        addRecordToOutputVariableStructure(state, "*", "ZONE THERMAL COMFORT ASHRAE 55 SIMPLE MODEL SUMMER OR WINTER CLOTHES NOT COMFORTABLE TIME");

    } else if (reportName == "UNGLAZEDTRANSPIREDSOLARCOLLECTORSUMMARYMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "SOLAR COLLECTOR SYSTEM EFFICIENCY");
        addRecordToOutputVariableStructure(state, "*", "SOLAR COLLECTOR OUTSIDE FACE SUCTION VELOCITY");
        addRecordToOutputVariableStructure(state, "*", "SOLAR COLLECTOR SENSIBLE HEATING RATE");

    } else if (reportName == "OCCUPANTCOMFORTDATASUMMARYMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "PEOPLE OCCUPANT COUNT");
        addRecordToOutputVariableStructure(state, "*", "PEOPLE AIR TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "PEOPLE AIR RELATIVE HUMIDITY");
        addRecordToOutputVariableStructure(state, "*", "ZONE THERMAL COMFORT FANGER MODEL PMV");
        addRecordToOutputVariableStructure(state, "*", "ZONE THERMAL COMFORT FANGER MODEL PPD");

    } else if (reportName == "CHILLERREPORTMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "CHILLER ELECTRICITY ENERGY"); // on meter
        addRecordToOutputVariableStructure(state, "*", "CHILLER ELECTRICITY RATE");
        addRecordToOutputVariableStructure(state, "*", "CHILLER EVAPORATOR COOLING ENERGY");      // on meter
        addRecordToOutputVariableStructure(state, "*", "CHILLER CONDENSER HEAT TRANSFER ENERGY"); // on meter
        addRecordToOutputVariableStructure(state, "*", "CHILLER COP");

    } else if (reportName == "TOWERREPORTMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "COOLING TOWER FAN ELECTRICITY ENERGY"); // on meter
        addRecordToOutputVariableStructure(state, "*", "COOLING TOWER FAN ELECTRICITY RATE");
        addRecordToOutputVariableStructure(state, "*", "COOLING TOWER HEAT TRANSFER RATE");
        addRecordToOutputVariableStructure(state, "*", "COOLING TOWER INLET TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "COOLING TOWER OUTLET TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "COOLING TOWER MASS FLOW RATE");

    } else if (reportName == "BOILERREPORTMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "BOILER HEATING ENERGY");         // on meter
        addRecordToOutputVariableStructure(state, "*", "BOILER NATURALGAS CONSUMPTION"); // on meter
        addRecordToOutputVariableStructure(state, "*", "BOILER HEATING ENERGY");         // on meter
        addRecordToOutputVariableStructure(state, "*", "BOILER HEATING RATE");
        addRecordToOutputVariableStructure(state, "*", "BOILER NATURALGAS CONSUMPTION RATE");
        addRecordToOutputVariableStructure(state, "*", "BOILER INLET TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "BOILER OUTLET TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "BOILER MASS FLOW RATE");
        addRecordToOutputVariableStructure(state, "*", "BOILER ANCILLARY ELECTRICITY RATE");

    } else if (reportName == "DXREPORTMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "COOLING COIL TOTAL COOLING ENERGY"); // on meter
        addRecordToOutputVariableStructure(state, "*", "COOLING COIL ELECTRICITY ENERGY");   // on meter
        addRecordToOutputVariableStructure(state, "*", "COOLING COIL SENSIBLE COOLING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "COOLING COIL LATENT COOLING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "COOLING COIL CRANKCASE HEATER ELECTRICITY ENERGY");
        addRecordToOutputVariableStructure(state, "*", "COOLING COIL RUNTIME FRACTION");
        addRecordToOutputVariableStructure(state, "*", "COOLING COIL TOTAL COOLING RATE");
        addRecordToOutputVariableStructure(state, "*", "COOLING COIL SENSIBLE COOLING RATE");
        addRecordToOutputVariableStructure(state, "*", "COOLING COIL LATENT COOLING RATE");
        addRecordToOutputVariableStructure(state, "*", "COOLING COIL ELECTRICITY RATE");
        addRecordToOutputVariableStructure(state, "*", "COOLING COIL CRANKCASE HEATER ELECTRICITY RATE");

    } else if (reportName == "WINDOWREPORTMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "SURFACE WINDOW TRANSMITTED SOLAR RADIATION RATE");
        addRecordToOutputVariableStructure(state, "*", "SURFACE WINDOW TRANSMITTED BEAM SOLAR RADIATION RATE");
        addRecordToOutputVariableStructure(state, "*", "SURFACE WINDOW TRANSMITTED DIFFUSE SOLAR RADIATION RATE");
        addRecordToOutputVariableStructure(state, "*", "SURFACE WINDOW HEAT GAIN RATE");
        addRecordToOutputVariableStructure(state, "*", "SURFACE WINDOW HEAT LOSS RATE");
        addRecordToOutputVariableStructure(state, "*", "SURFACE WINDOW INSIDE FACE GLAZING CONDENSATION STATUS");
        addRecordToOutputVariableStructure(state, "*", "SURFACE SHADING DEVICE IS ON TIME FRACTION");
        addRecordToOutputVariableStructure(state, "*", "SURFACE STORM WINDOW ON OFF STATUS");

    } else if (reportName == "WINDOWENERGYREPORTMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "SURFACE WINDOW TRANSMITTED SOLAR RADIATION ENERGY");
        addRecordToOutputVariableStructure(state, "*", "SURFACE WINDOW TRANSMITTED BEAM SOLAR RADIATION ENERGY");
        addRecordToOutputVariableStructure(state, "*", "SURFACE WINDOW TRANSMITTED DIFFUSE SOLAR RADIATION ENERGY");
        addRecordToOutputVariableStructure(state, "*", "SURFACE WINDOW HEAT GAIN ENERGY");
        addRecordToOutputVariableStructure(state, "*", "SURFACE WINDOW HEAT LOSS ENERGY");

    } else if (reportName == "WINDOWZONESUMMARYMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "ZONE WINDOWS TOTAL HEAT GAIN RATE");
        addRecordToOutputVariableStructure(state, "*", "ZONE WINDOWS TOTAL HEAT LOSS RATE");
        addRecordToOutputVariableStructure(state, "*", "ENCLOSURE WINDOWS TOTAL TRANSMITTED SOLAR RADIATION RATE");
        addRecordToOutputVariableStructure(state, "*", "ENCLOSURE EXTERIOR WINDOWS TOTAL TRANSMITTED BEAM SOLAR RADIATION RATE");
        addRecordToOutputVariableStructure(state, "*", "ENCLOSURE EXTERIOR WINDOWS TOTAL TRANSMITTED DIFFUSE SOLAR RADIATION RATE");
        addRecordToOutputVariableStructure(state, "*", "ENCLOSURE INTERIOR WINDOWS TOTAL TRANSMITTED DIFFUSE SOLAR RADIATION RATE");
        addRecordToOutputVariableStructure(state, "*", "ENCLOSURE INTERIOR WINDOWS TOTAL TRANSMITTED BEAM SOLAR RADIATION RATE");

    } else if (reportName == "WINDOWENERGYZONESUMMARYMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "ZONE WINDOWS TOTAL HEAT GAIN ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE WINDOWS TOTAL HEAT LOSS ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ENCLOSURE WINDOWS TOTAL TRANSMITTED SOLAR RADIATION ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ENCLOSURE EXTERIOR WINDOWS TOTAL TRANSMITTED BEAM SOLAR RADIATION ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ENCLOSURE EXTERIOR WINDOWS TOTAL TRANSMITTED DIFFUSE SOLAR RADIATION ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ENCLOSURE INTERIOR WINDOWS TOTAL TRANSMITTED DIFFUSE SOLAR RADIATION ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ENCLOSURE INTERIOR WINDOWS TOTAL TRANSMITTED BEAM SOLAR RADIATION ENERGY");

    } else if (reportName == "AVERAGEOUTDOORCONDITIONSMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "SITE OUTDOOR AIR DRYBULB TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "SITE OUTDOOR AIR WETBULB TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "SITE OUTDOOR AIR DEWPOINT TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "SITE WIND SPEED");
        addRecordToOutputVariableStructure(state, "*", "SITE SKY TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "SITE DIFFUSE SOLAR RADIATION RATE PER AREA");
        addRecordToOutputVariableStructure(state, "*", "SITE DIRECT SOLAR RADIATION RATE PER AREA");
        addRecordToOutputVariableStructure(state, "*", "SITE RAIN STATUS");

    } else if (reportName == "OUTDOORCONDITIONSMAXIMUMDRYBULBMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "SITE OUTDOOR AIR DRYBULB TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "SITE OUTDOOR AIR WETBULB TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "SITE OUTDOOR AIR DEWPOINT TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "SITE WIND SPEED");
        addRecordToOutputVariableStructure(state, "*", "SITE SKY TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "SITE DIFFUSE SOLAR RADIATION RATE PER AREA");
        addRecordToOutputVariableStructure(state, "*", "SITE DIRECT SOLAR RADIATION RATE PER AREA");

    } else if (reportName == "OUTDOORCONDITIONSMINIMUMDRYBULBMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "SITE OUTDOOR AIR DRYBULB TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "SITE OUTDOOR AIR WETBULB TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "SITE OUTDOOR AIR DEWPOINT TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "SITE WIND SPEED");
        addRecordToOutputVariableStructure(state, "*", "SITE SKY TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "SITE DIFFUSE SOLAR RADIATION RATE PER AREA");
        addRecordToOutputVariableStructure(state, "*", "SITE DIRECT SOLAR RADIATION RATE PER AREA");

    } else if (reportName == "OUTDOORCONDITIONSMAXIMUMWETBULBMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "SITE OUTDOOR AIR WETBULB TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "SITE OUTDOOR AIR DRYBULB TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "SITE OUTDOOR AIR DEWPOINT TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "SITE WIND SPEED");
        addRecordToOutputVariableStructure(state, "*", "SITE SKY TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "SITE DIFFUSE SOLAR RADIATION RATE PER AREA");
        addRecordToOutputVariableStructure(state, "*", "SITE DIRECT SOLAR RADIATION RATE PER AREA");

    } else if (reportName == "OUTDOORCONDITIONSMAXIMUMDEWPOINTMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "SITE OUTDOOR AIR DEWPOINT TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "SITE OUTDOOR AIR DRYBULB TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "SITE OUTDOOR AIR WETBULB TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "SITE WIND SPEED");
        addRecordToOutputVariableStructure(state, "*", "SITE SKY TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "SITE DIFFUSE SOLAR RADIATION RATE PER AREA");
        addRecordToOutputVariableStructure(state, "*", "SITE DIRECT SOLAR RADIATION RATE PER AREA");

    } else if (reportName == "OUTDOORGROUNDCONDITIONSMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "SITE GROUND TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "SITE SURFACE GROUND TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "SITE DEEP GROUND TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "SITE MAINS WATER TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "SITE GROUND REFLECTED SOLAR RADIATION RATE PER AREA");
        addRecordToOutputVariableStructure(state, "*", "SITE SNOW ON GROUND STATUS");

    } else if (reportName == "WINDOWACREPORTMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "ZONE WINDOW AIR CONDITIONER TOTAL COOLING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE WINDOW AIR CONDITIONER ELECTRICITY ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE WINDOW AIR CONDITIONER TOTAL COOLING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE WINDOW AIR CONDITIONER SENSIBLE COOLING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE WINDOW AIR CONDITIONER LATENT COOLING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE WINDOW AIR CONDITIONER TOTAL COOLING RATE");
        addRecordToOutputVariableStructure(state, "*", "ZONE WINDOW AIR CONDITIONER SENSIBLE COOLING RATE");
        addRecordToOutputVariableStructure(state, "*", "ZONE WINDOW AIR CONDITIONER LATENT COOLING RATE");
        addRecordToOutputVariableStructure(state, "*", "ZONE WINDOW AIR CONDITIONER ELECTRICITY RATE");

    } else if (reportName == "WATERHEATERREPORTMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "WATER HEATER TOTAL DEMAND HEAT TRANSFER ENERGY");
        addRecordToOutputVariableStructure(state, "*", "WATER HEATER USE SIDE HEAT TRANSFER ENERGY");
        addRecordToOutputVariableStructure(state, "*", "WATER HEATER BURNER HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "WATER HEATER NATURALGAS CONSUMPTION");
        addRecordToOutputVariableStructure(state, "*", "WATER HEATER TOTAL DEMAND HEAT TRANSFER ENERGY");
        addRecordToOutputVariableStructure(state, "*", "WATER HEATER LOSS DEMAND ENERGY");
        addRecordToOutputVariableStructure(state, "*", "WATER HEATER HEAT LOSS ENERGY");
        addRecordToOutputVariableStructure(state, "*", "WATER HEATER TANK TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "WATER HEATER HEAT RECOVERY SUPPLY ENERGY");
        addRecordToOutputVariableStructure(state, "*", "WATER HEATER SOURCE SIDE HEAT TRANSFER ENERGY");

    } else if (reportName == "GENERATORREPORTMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "GENERATOR PRODUCED AC ELECTRICITY ENERGY");
        addRecordToOutputVariableStructure(state, "*", "GENERATOR DIESEL CONSUMPTION");
        addRecordToOutputVariableStructure(state, "*", "GENERATOR NATURALGAS CONSUMPTION");
        addRecordToOutputVariableStructure(state, "*", "GENERATOR PRODUCED AC ELECTRICITY ENERGY");
        addRecordToOutputVariableStructure(state, "*", "GENERATOR TOTAL HEAT RECOVERY");
        addRecordToOutputVariableStructure(state, "*", "GENERATOR JACKET HEAT RECOVERY ENERGY");
        addRecordToOutputVariableStructure(state, "*", "GENERATOR LUBE HEAT RECOVERY");
        addRecordToOutputVariableStructure(state, "*", "GENERATOR EXHAUST HEAT RECOVERY ENERGY");
        addRecordToOutputVariableStructure(state, "*", "GENERATOR EXHAUST AIR TEMPERATURE");

    } else if (reportName == "DAYLIGHTINGREPORTMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "SITE EXTERIOR BEAM NORMAL ILLUMINANCE");
        addRecordToOutputVariableStructure(state, "*", "DAYLIGHTING LIGHTING POWER MULTIPLIER");
        addRecordToOutputVariableStructure(state, "*", "DAYLIGHTING LIGHTING POWER MULTIPLIER");
        addRecordToOutputVariableStructure(state, "*", "DAYLIGHTING REFERENCE POINT 1 ILLUMINANCE");
        addRecordToOutputVariableStructure(state, "*", "DAYLIGHTING REFERENCE POINT 1 GLARE INDEX");
        addRecordToOutputVariableStructure(state, "*", "DAYLIGHTING REFERENCE POINT 1 GLARE INDEX SETPOINT EXCEEDED TIME");
        addRecordToOutputVariableStructure(state, "*", "DAYLIGHTING REFERENCE POINT 1 DAYLIGHT ILLUMINANCE SETPOINT EXCEEDED TIME");
        addRecordToOutputVariableStructure(state, "*", "DAYLIGHTING REFERENCE POINT 2 ILLUMINANCE");
        addRecordToOutputVariableStructure(state, "*", "DAYLIGHTING REFERENCE POINT 2 GLARE INDEX");
        addRecordToOutputVariableStructure(state, "*", "DAYLIGHTING REFERENCE POINT 2 GLARE INDEX SETPOINT EXCEEDED TIME");
        addRecordToOutputVariableStructure(state, "*", "DAYLIGHTING REFERENCE POINT 2 DAYLIGHT ILLUMINANCE SETPOINT EXCEEDED TIME");

    } else if (reportName == "COILREPORTMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "HEATING COIL HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "HEATING COIL HEATING RATE");
        addRecordToOutputVariableStructure(state, "*", "COOLING COIL SENSIBLE COOLING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "COOLING COIL TOTAL COOLING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "COOLING COIL TOTAL COOLING RATE");
        addRecordToOutputVariableStructure(state, "*", "COOLING COIL SENSIBLE COOLING RATE");
        addRecordToOutputVariableStructure(state, "*", "COOLING COIL WETTED AREA FRACTION");

    } else if (reportName == "PLANTLOOPDEMANDREPORTMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "PLANT SUPPLY SIDE COOLING DEMAND RATE");
        addRecordToOutputVariableStructure(state, "*", "PLANT SUPPLY SIDE HEATING DEMAND RATE");

    } else if (reportName == "FANREPORTMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "FAN ELECTRICITY ENERGY");
        addRecordToOutputVariableStructure(state, "*", "FAN RISE IN AIR TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "FAN ELECTRICITY RATE");

    } else if (reportName == "PUMPREPORTMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "PUMP ELECTRICITY ENERGY");
        addRecordToOutputVariableStructure(state, "*", "PUMP FLUID HEAT GAIN ENERGY");
        addRecordToOutputVariableStructure(state, "*", "PUMP ELECTRICITY RATE");
        addRecordToOutputVariableStructure(state, "*", "PUMP SHAFT POWER");
        addRecordToOutputVariableStructure(state, "*", "PUMP FLUID HEAT GAIN RATE");
        addRecordToOutputVariableStructure(state, "*", "PUMP OUTLET TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "PUMP MASS FLOW RATE");

    } else if (reportName == "CONDLOOPDEMANDREPORTMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "PLANT SUPPLY SIDE COOLING DEMAND RATE");
        addRecordToOutputVariableStructure(state, "*", "PLANT SUPPLY SIDE HEATING DEMAND RATE");
        addRecordToOutputVariableStructure(state, "*", "PLANT SUPPLY SIDE INLET TEMPERATURE");
        addRecordToOutputVariableStructure(state, "*", "PLANT SUPPLY SIDE OUTLET TEMPERATURE");

    } else if (reportName == "ZONETEMPERATUREOSCILLATIONREPORTMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "ZONE OSCILLATING TEMPERATURES TIME");
        addRecordToOutputVariableStructure(state, "*", "ZONE PEOPLE OCCUPANT COUNT");

    } else if (reportName == "AIRLOOPSYSTEMENERGYANDWATERUSEMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "AIR SYSTEM HOT WATER ENERGY");
        addRecordToOutputVariableStructure(state, "*", "AIR SYSTEM STEAM ENERGY");
        addRecordToOutputVariableStructure(state, "*", "AIR SYSTEM CHILLED WATER ENERGY");
        addRecordToOutputVariableStructure(state, "*", "AIR SYSTEM ELECTRICITY ENERGY");
        addRecordToOutputVariableStructure(state, "*", "AIR SYSTEM NATURALGAS ENERGY");
        addRecordToOutputVariableStructure(state, "*", "AIR SYSTEM WATER VOLUME");

    } else if (reportName == "AIRLOOPSYSTEMCOMPONENTLOADSMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "AIR SYSTEM FAN AIR HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "AIR SYSTEM COOLING COIL TOTAL COOLING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "AIR SYSTEM HEATING COIL TOTAL HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "AIR SYSTEM HEAT EXCHANGER TOTAL HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "AIR SYSTEM HEAT EXCHANGER TOTAL COOLING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "AIR SYSTEM HUMIDIFIER TOTAL HEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "AIR SYSTEM EVAPORATIVE COOLER TOTAL COOLING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "AIR SYSTEM DESICCANT DEHUMIDIFIER TOTAL COOLING ENERGY");

    } else if (reportName == "AIRLOOPSYSTEMCOMPONENTENERGYUSEMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "AIR SYSTEM FAN ELECTRICITY ENERGY");
        addRecordToOutputVariableStructure(state, "*", "AIR SYSTEM HEATING COIL HOT WATER ENERGY");
        addRecordToOutputVariableStructure(state, "*", "AIR SYSTEM COOLING COIL CHILLED WATER ENERGY");
        addRecordToOutputVariableStructure(state, "*", "AIR SYSTEM DX HEATING COIL ELECTRICITY ENERGY");
        addRecordToOutputVariableStructure(state, "*", "AIR SYSTEM DX COOLING COIL ELECTRICITY ENERGY");
        addRecordToOutputVariableStructure(state, "*", "AIR SYSTEM HEATING COIL ELECTRICITY ENERGY");
        addRecordToOutputVariableStructure(state, "*", "AIR SYSTEM HEATING COIL NATURALGAS ENERGY");
        addRecordToOutputVariableStructure(state, "*", "AIR SYSTEM HEATING COIL STEAM ENERGY");
        addRecordToOutputVariableStructure(state, "*", "AIR SYSTEM HUMIDIFIER ELECTRICITY ENERGY");
        addRecordToOutputVariableStructure(state, "*", "AIR SYSTEM EVAPORATIVE COOLER ELECTRICITY ENERGY");
        addRecordToOutputVariableStructure(state, "*", "AIR SYSTEM DESICCANT DEHUMIDIFIER ELECTRICITY ENERGY");

    } else if (reportName == "MECHANICALVENTILATIONLOADSMONTHLY") {
        addRecordToOutputVariableStructure(state, "*", "ZONE MECHANICAL VENTILATION NO LOAD HEAT REMOVAL ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE MECHANICAL VENTILATION COOLING LOAD INCREASE ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE MECHANICAL VENTILATION COOLING LOAD INCREASE DUE TO OVERHEATING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE MECHANICAL VENTILATION COOLING LOAD DECREASE ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE MECHANICAL VENTILATION NO LOAD HEAT ADDITION ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE MECHANICAL VENTILATION HEATING LOAD INCREASE ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE MECHANICAL VENTILATION HEATING LOAD INCREASE DUE TO OVERCOOLING ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE MECHANICAL VENTILATION HEATING LOAD DECREASE ENERGY");
        addRecordToOutputVariableStructure(state, "*", "ZONE MECHANICAL VENTILATION AIR CHANGES PER HOUR");

    } else if (reportName == "HEATEMISSIONSREPORTMONTHLY") {
        // Place holder
        addRecordToOutputVariableStructure(state, "*", "Site Total Surface Heat Emission to Air");
        addRecordToOutputVariableStructure(state, "*", "Site Total Zone Exfiltration Heat Loss");
        addRecordToOutputVariableStructure(state, "*", "Site Total Zone Exhaust Air Heat Loss");
        addRecordToOutputVariableStructure(state, "*", "Air System Relief Air Total Heat Loss Energy");
        addRecordToOutputVariableStructure(state, "*", "HVAC System Total Heat Rejection Energy");
    } else {
    }
}

void InputProcessor::addRecordToOutputVariableStructure(EnergyPlusData &state, std::string const &KeyValue, std::string const &VariableName)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   July 2010
    //       MODIFIED       March 2017
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This routine adds a new record (if necessary) to the Output Variable
    // reporting structure.  DataOutputs, OutputVariablesForSimulation

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    std::string::size_type vnameLen; // if < length, there were units on the line/name

    std::string::size_type const rbpos = index(VariableName, '[');
    if (rbpos == std::string::npos) {
        vnameLen = len_trim(VariableName);
    } else {
        vnameLen = len_trim(VariableName.substr(0, rbpos));
    }

    std::string const VarName(VariableName.substr(0, vnameLen));

    auto const found = state.dataOutput->OutputVariablesForSimulation.find(VarName);
    if (found == state.dataOutput->OutputVariablesForSimulation.end()) {
        std::map<std::string,
                 DataOutputs::OutputReportingVariables,
                 // Util::case_insensitive_hasher,
                 Util::case_insensitive_comparator>
            data;
        // data.reserve(32);
        data.emplace(KeyValue, DataOutputs::OutputReportingVariables(state, KeyValue, VarName));
        state.dataOutput->OutputVariablesForSimulation.emplace(VarName, std::move(data));
    } else {
        found->second.emplace(KeyValue, DataOutputs::OutputReportingVariables(state, KeyValue, VarName));
    }
    state.dataOutput->NumConsideredOutputVariables++;
}

} // namespace EnergyPlus
