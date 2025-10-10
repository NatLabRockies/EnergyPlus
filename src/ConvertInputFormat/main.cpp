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

#include "EnergyPlus/DataStringGlobals.hh"
#include "EnergyPlus/FileSystem.hh"
#include "EnergyPlus/InputProcessing/IdfParser.hh"
#include "EnergyPlus/InputProcessing/InputValidation.hh"
#include <embedded/EmbeddedEpJSONSchema.hh>

#include <CLI/CLI11.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <nlohmann/json.hpp>

#ifdef _OPENMP
#    include <omp.h>
#endif

#include <array>
#include <filesystem>
#include <iterator> // for make_move_iterator
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using json = nlohmann::json;

enum class OutputTypes
{
    Default = 0,
    IDF,
    epJSON,
    // Experimental starting here
    CBOR,
    MsgPack,
    UBJSON,
    BSON,
    Num
};
static constexpr std::array<std::string_view, static_cast<int>(OutputTypes::Num)> outputTypeStrs = {
    "default", "IDF", "epJSON", "CBOR", "MsgPack", "UBJSON", "BSON"};

static constexpr auto outputTypeExperimentalStart = OutputTypes::CBOR;

template <typename... Args> void displayMessage(std::string_view str_format, Args &&...args)
{
    fmt::print(std::cout, str_format, args...);
    std::cout.write("\n", 1);
}

bool checkVersionMatch(json const &epJSON)
{
    auto it = epJSON.find("Version");
    if (it != epJSON.end()) {
        for (auto const &version : it.value()) {
            std::string v = version["version_identifier"].get<std::string>();
            if (v.empty()) {
                displayMessage("Input errors occurred and version ID was left blank, verify file version");
            } else {
                std::string::size_type const lenVer(len(EnergyPlus::DataStringGlobals::MatchVersion));
                int Which;
                if ((lenVer > 0) && (EnergyPlus::DataStringGlobals::MatchVersion[lenVer - 1] == '0')) {
                    Which = static_cast<int>(index(v.substr(0, lenVer - 2), EnergyPlus::DataStringGlobals::MatchVersion.substr(0, lenVer - 2)));
                } else {
                    Which = static_cast<int>(index(v, EnergyPlus::DataStringGlobals::MatchVersion));
                }
                if (Which != 0) {
                    displayMessage(R"(Version: in IDF="{}" not the same as expected="{}")", v, EnergyPlus::DataStringGlobals::MatchVersion);
                    return false;
                }
            }
        }
    }
    return true;
}

bool checkForUnsupportedObjects(json const &epJSON, bool convertHVACTemplate)
{
    bool errorsFound = false;
    constexpr std::array<std::string_view, 32> hvacTemplateObjects = {"HVACTemplate:Thermostat",
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

    bool objectFound = false;
    std::string objectType;

    // For ConvertInputFormat, skip this unless -n option is present to not allow conversion of HVACTemplate objects
    if (!convertHVACTemplate) {
        for (size_t count = 0; count < hvacTemplateObjects.size(); ++count) {
            objectType = hvacTemplateObjects[count];
            auto it = epJSON.find(objectType);
            if (it != epJSON.end()) {
                objectFound = true;
                break;
            }
        }
        if (objectFound) {
            displayMessage("HVACTemplate:* objects found. These objects are not supported directly by EnergyPlus.");
            displayMessage("You must run the ExpandObjects program on this input.");
            errorsFound = true;
        }
    }

    constexpr std::array<std::string_view, 26> groundHTObjects = {"GroundHeatTransfer:Control",
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
        displayMessage("GroundHeatTransfer:* objects found. These objects are not supported directly by EnergyPlus.");
        displayMessage("You must run the ExpandObjects program on this input.");
        errorsFound = true;
    }

    constexpr std::array<std::string_view, 4> parametricObjects = {
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
        displayMessage("Parametric:* objects found. These objects are not supported directly by EnergyPlus.");
        displayMessage("You must run the ParametricPreprocesor program on this input.");
        errorsFound = true;
    }
    return errorsFound;
}

bool processErrors(std::unique_ptr<IdfParser> const &idf_parser, std::unique_ptr<Validation> const &validation, bool isDDY)
{
    auto const idf_parser_errors = idf_parser->errors();
    auto const idf_parser_warnings = idf_parser->warnings();

    auto const validation_errors = validation->errors();
    bool hasValidationErrors = false;

    auto const validation_warnings = validation->warnings();

    for (auto const &error : idf_parser_errors) {
        displayMessage(error);
    }
    for (auto const &warning : idf_parser_warnings) {
        displayMessage(warning);
    }
    for (auto const &error : validation_errors) {
        bool const missing_building = error.find("Missing required property 'Building'") != std::string::npos;
        bool const missing_geometry = error.find("Missing required property 'GlobalGeometryRules'") != std::string::npos;
        // if we encountered one of the expected missing building/geometry errors, we should handle them special
        if (missing_building || missing_geometry) {
            if (isDDY) { // for DDY files just ignore it completely
                continue;
            }
            // for other IDFs, go ahead and emit a message, but don't trigger a failure
            displayMessage(error);
            continue;
        }
        // and if it wasn't a missing building or missing geometry error, we will emit that as a proper error and fail
        hasValidationErrors = true;
        displayMessage(error);
    }
    for (auto const &warning : validation_warnings) {
        displayMessage(warning);
    }

    return hasValidationErrors || idf_parser->hasErrors();
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

bool processInput(fs::path const &inputFilePath,
                  json const &schema,
                  OutputTypes outputType,
                  fs::path outputDirPath,
                  std::string &outputTypeStr,
                  bool convertHVACTemplate)
{
    auto validation(std::make_unique<Validation>(&schema));
    auto idf_parser(std::make_unique<IdfParser>());
    json epJSON;

    auto const inputDirPath = EnergyPlus::FileSystem::getParentDirectoryPath(inputFilePath);

    if (outputDirPath.empty()) {
        outputDirPath = inputDirPath;
    }

    auto const inputFileType = EnergyPlus::FileSystem::getFileType(inputFilePath);

    const bool isEpJSON = EnergyPlus::FileSystem::is_all_json_type(inputFileType);
    const bool isCBOR = (inputFileType == EnergyPlus::FileSystem::FileTypes::CBOR);
    const bool isMsgPack = (inputFileType == EnergyPlus::FileSystem::FileTypes::MsgPack);
    const bool isUBJSON = (inputFileType == EnergyPlus::FileSystem::FileTypes::UBJSON);
    const bool isBSON = (inputFileType == EnergyPlus::FileSystem::FileTypes::BSON);
    const bool isIDForIMF = EnergyPlus::FileSystem::is_idf_type(inputFileType); // IDF or IMF
    const bool isDDY = (inputFileType == EnergyPlus::FileSystem::FileTypes::DDY);

    if (!(isEpJSON || isIDForIMF || isDDY)) {
        displayMessage("ERROR: Input file must have IDF, IMF, DDY, or epJSON extension.");
        return false;
    }

    if (outputType == OutputTypes::epJSON &&
        (inputFileType == EnergyPlus::FileSystem::FileTypes::EpJSON || inputFileType == EnergyPlus::FileSystem::FileTypes::JSON)) {
        displayMessage("Same output format as input format requested (epJSON). Skipping conversion and moving to next file.");
        return false;
    } else if (outputType == OutputTypes::IDF && (isIDForIMF || isDDY)) {
        displayMessage("Same output format as input format requested (IDF). Skipping conversion and moving to next file.");
        return false;
    } else if (outputType == OutputTypes::CBOR && isCBOR) {
        displayMessage("Same output format as input format requested (CBOR). Skipping conversion and moving to next file.");
        return false;
    } else if (outputType == OutputTypes::MsgPack && isMsgPack) {
        displayMessage("Same output format as input format requested (MsgPack). Skipping conversion and moving to next file.");
        return false;
    } else if (outputType == OutputTypes::UBJSON && isUBJSON) {
        displayMessage("Same output format as input format requested (UBJSON). Skipping conversion and moving to next file.");
        return false;
    } else if (outputType == OutputTypes::BSON && isBSON) {
        displayMessage("Same output format as input format requested (BSON). Skipping conversion and moving to next file.");
        return false;
    }

    if (!EnergyPlus::FileSystem::fileExists(inputFilePath)) {
        displayMessage("Input file path {} not found", inputFilePath.generic_string());
        return false;
    }

    try {
        if (!isEpJSON) {
            auto input_file = EnergyPlus::FileSystem::readFile(inputFilePath);

            bool success = true;
            epJSON = idf_parser->decode(input_file, schema, success);
            cleanEPJSON(epJSON);
        } else {
            epJSON = EnergyPlus::FileSystem::readJSON(inputFilePath);
        }
    } catch (const std::exception &e) {
        displayMessage(e.what());
        displayMessage("Errors occurred when processing input file. Preceding condition(s) cause termination.");
        return false;
    }

    bool is_valid = validation->validate(epJSON);
    bool const hasErrors = processErrors(idf_parser, validation, isDDY);
    if (isDDY && !hasErrors) {
        is_valid = true;
    }
    bool const versionMatch = checkVersionMatch(epJSON);
    bool const unsupportedFound = checkForUnsupportedObjects(epJSON, convertHVACTemplate);

    if (!is_valid || hasErrors || unsupportedFound) {
        displayMessage("Errors occurred when validating input file. Preceding condition(s) cause termination.");
        return false;
    }

    if (isEpJSON && !versionMatch) {
        displayMessage("Skipping conversion of input file to IDF due to mismatched Version.");
        return false;
    }

    auto const outputFilePathWithOriExtension = outputDirPath / EnergyPlus::FileSystem::getFileName(inputFilePath);
    if ((outputType == OutputTypes::Default || outputType == OutputTypes::IDF) && isEpJSON) {
        auto const input_file = idf_parser->encode(epJSON, schema);
        fs::path convertedEpJSON =
            EnergyPlus::FileSystem::makeNativePath(EnergyPlus::FileSystem::replaceFileExtension(outputFilePathWithOriExtension, ".idf"));
        EnergyPlus::FileSystem::writeFile<EnergyPlus::FileSystem::FileTypes::IDF>(convertedEpJSON, input_file);
        outputTypeStr = "IDF";
    } else if ((outputType == OutputTypes::Default || outputType == OutputTypes::epJSON) && !isEpJSON) {
        fs::path convertedIDF =
            EnergyPlus::FileSystem::makeNativePath(EnergyPlus::FileSystem::replaceFileExtension(outputFilePathWithOriExtension, ".epJSON"));
        EnergyPlus::FileSystem::writeFile<EnergyPlus::FileSystem::FileTypes::EpJSON>(convertedIDF, epJSON);
        outputTypeStr = "EPJSON";
    } else if (outputType == OutputTypes::CBOR) {
        fs::path convertedCBOR =
            EnergyPlus::FileSystem::makeNativePath(EnergyPlus::FileSystem::replaceFileExtension(outputFilePathWithOriExtension, ".cbor"));
        EnergyPlus::FileSystem::writeFile<EnergyPlus::FileSystem::FileTypes::CBOR>(convertedCBOR, epJSON);
    } else if (outputType == OutputTypes::MsgPack) {
        fs::path convertedMsgPack =
            EnergyPlus::FileSystem::makeNativePath(EnergyPlus::FileSystem::replaceFileExtension(outputFilePathWithOriExtension, ".msgpack"));
        EnergyPlus::FileSystem::writeFile<EnergyPlus::FileSystem::FileTypes::MsgPack>(convertedMsgPack, epJSON);
    } else if (outputType == OutputTypes::UBJSON) {
        fs::path convertedUBJSON =
            EnergyPlus::FileSystem::makeNativePath(EnergyPlus::FileSystem::replaceFileExtension(outputFilePathWithOriExtension, ".ubjson"));
        EnergyPlus::FileSystem::writeFile<EnergyPlus::FileSystem::FileTypes::UBJSON>(convertedUBJSON, epJSON);
    } else if (outputType == OutputTypes::BSON) {
        fs::path convertedBSON =
            EnergyPlus::FileSystem::makeNativePath(EnergyPlus::FileSystem::replaceFileExtension(outputFilePathWithOriExtension, ".bson"));
        EnergyPlus::FileSystem::writeFile<EnergyPlus::FileSystem::FileTypes::BSON>(convertedBSON, epJSON);
    } else {
        return false;
    }
    return true;
}

std::vector<fs::path> parse_input_paths(fs::path const &inputFilePath)
{
    std::ifstream input_paths_stream(inputFilePath);
    if (!input_paths_stream.is_open()) {
        displayMessage("Could not open file: {}", inputFilePath.generic_string());
        return {};
    }
    std::vector<fs::path> input_paths;
    std::string line;
    while (std::getline(input_paths_stream, line)) {
        if (line.empty()) {
            continue;
        }
        fs::path input_file{line};
        if (!fs::is_regular_file(input_file)) {
            // Fall back on searching in the same directory as the lstfile
            input_file = inputFilePath.parent_path() / input_file;
            if (!fs::is_regular_file(input_file)) {
                displayMessage("Input file does not exist: {}", line);
                continue;
            }
        }
        input_paths.emplace_back(std::move(input_file));
    }
    return input_paths;
}

int main(int argc, char **argv)
{

    CLI::App app{"ConvertInputFormat"};
    argv = app.ensure_utf8(argv);
    app.description("Run input file conversion tool");
    app.set_version_flag("-v,--version", EnergyPlus::DataStringGlobals::VerString);

#ifdef _OPENMP
    int number_of_threads = std::thread::hardware_concurrency();
#else
    int number_of_threads = 1;
#endif

    [[maybe_unused]] CLI::Option *nproc_opt =
        app.add_option("-j", number_of_threads, fmt::format("Number of threads [Default: {}]", number_of_threads))->option_text("N");

#ifndef _OPENMP
    // I don't want to throw if the user passes -j while OpenMP is unavailable, so we hide it by setting an empty group instead
    nproc_opt->group("");
#endif

    fs::path inputFilePath;
    app.add_option("-i,--input", inputFilePath, "Text file with list of input files to convert (newline delimited)")
        ->required(false)
        ->option_text("LSTFILE")
        ->check(CLI::ExistingFile);

    fs::path outputDirectoryPath;
    app.add_option("-o,--output", outputDirectoryPath, "Output directory. Will use input file location by default")
        ->option_text("DIR")
        ->required(false);
    // ->check(CLI::ExistingDirectory) // We don't require it to exist, we make it if needed

    const std::map<std::string, OutputTypes> outputTypeMap{
        {"default", OutputTypes::Default},
        {"idf", OutputTypes::IDF},
        {"epjson", OutputTypes::epJSON},
        {"cbor", OutputTypes::CBOR},
        {"msgpack", OutputTypes::MsgPack},
        {"ubjson", OutputTypes::UBJSON},
        {"bson", OutputTypes::BSON},
    };

    OutputTypes outputType{OutputTypes::Default};

    const std::string help_message = fmt::format(R"help(Output format.
Default means IDF->epJSON or epJSON->IDF
Select one (case insensitive):
[{}])help",
                                                 fmt::join(outputTypeStrs, ","));

    app.add_option("-f,--format", outputType, help_message)
        ->option_text("FORMAT")
        ->required(false)
        ->transform(CLI::CheckedTransformer(outputTypeMap, CLI::ignore_case));

    bool noConvertHVACTemplate = false;
    app.add_flag("-n,--noHVACTemplate", noConvertHVACTemplate, "Do not convert HVACTemplate objects");

    std::vector<fs::path> files;
    app.add_option("input_file", files, "Multiple input files to be translated")->required(false)->check(CLI::ExistingFile);

    app.footer("Example: ConvertInputFormat in.idf");

    // We are not modifying argc/argv, so we defer to CLI11 to find the argc/argv instead. It'll use GetCommandLineW & CommandLineToArgvW on windows
    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        return app.exit(e);
    }

    const bool convertHVACTemplate = !noConvertHVACTemplate;

    if (!outputDirectoryPath.empty()) {
        EnergyPlus::FileSystem::makeDirectory(outputDirectoryPath);
    }

    if (!inputFilePath.empty()) {
        std::vector<fs::path> list_files = parse_input_paths(inputFilePath);
        files.insert(files.end(), std::make_move_iterator(list_files.begin()), std::make_move_iterator(list_files.end()));
    }

    std::string outputTypeStr{outputTypeStrs[static_cast<size_t>(outputType)]};
    if (outputType >= outputTypeExperimentalStart) {
        displayMessage("{} input format is experimental.", outputTypeStr);
    }

    if (files.empty()) {
        displayMessage("No valid files found. Either specify --input or pass files as extra arguments");
        return 1;
    }

    // Must sort before unique
    std::sort(files.begin(), files.end());
    auto it = std::unique(files.begin(), files.end());
    files.resize(std::distance(files.begin(), it));

    auto const embeddedEpJSONSchema = EnergyPlus::EmbeddedEpJSONSchema::embeddedEpJSONSchema();
    auto schema = json::from_cbor(embeddedEpJSONSchema);

    int number_files = static_cast<int>(files.size());
    std::size_t fileCount = 0;

#ifdef _OPENMP
    omp_set_num_threads(number_of_threads);

#    pragma omp parallel default(none)                                                                                                               \
        shared(files, number_files, fileCount, schema, outputType, outputTypeStr, outputDirectoryPath, convertHVACTemplate)
    {
#    pragma omp for
        for (int i = 0; i < number_files; ++i) {
            const bool successful = processInput(files[i], schema, outputType, outputDirectoryPath, outputTypeStr, convertHVACTemplate);
#    pragma omp atomic
            ++fileCount;
            if (successful) {
                displayMessage(
                    "Input file converted to {} successfully | {}/{} | {}", outputTypeStr, fileCount, number_files, files[i].generic_string());
            } else {
                displayMessage("Input file conversion failed: | {}/{} | {}", fileCount, number_files, files[i].generic_string());
            }
        }
    }

#else
    if (number_of_threads > 1) {
        displayMessage("ConvertInputFormat is not compiled with OpenMP. Only running on 1 thread, not requested {} threads.", number_of_threads);
        number_of_threads = 1;
    }

    for (auto const &file : files) {
        bool successful = processInput(file, schema, outputType, outputDirectoryPath, outputTypeStr, convertHVACTemplate);
        ++fileCount;
        if (successful) {
            displayMessage("Input file converted to {} successfully | {}/{} | {}", outputTypeStr, fileCount, number_files, file.generic_string());
        } else {
            displayMessage("Input file conversion failed: | {}/{} | {}", fileCount, number_files, file.generic_string());
        }
    }
#endif

    return 0;
}
