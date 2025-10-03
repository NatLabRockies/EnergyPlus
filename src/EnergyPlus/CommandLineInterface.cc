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
#include <thread>

// CLI Headers
#include <CLI/CLI11.hpp>

// Project headers
#include <EnergyPlus/CommandLineInterface.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/DisplayRoutines.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/FileSystem.hh>
#include <EnergyPlus/PluginManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

#if LINK_WITH_PYTHON
#    include <EnergyPlus/PythonEngine.hh>
#endif

namespace EnergyPlus {

namespace CommandLineInterface {

    int ProcessArgs(EnergyPlusData &state, const std::vector<std::string> &args)
    {
        using size_type = std::string::size_type;

        // Expand long-name options using "=" sign in to two arguments
        // and expand multiple short options into separate arguments
        std::vector<std::string> arguments;

        std::string const dash("-");

        for (const auto &inputArg : args) {

            size_type const doubleDashPosition = inputArg.find("--");
            size_type const equalsPosition = inputArg.find("=");

            if (doubleDashPosition == 0 && equalsPosition != std::string::npos) { // --option=value
                arguments.push_back(inputArg.substr(0, equalsPosition));
                arguments.push_back(inputArg.substr(equalsPosition + 1, inputArg.size() - 1));
            } else if (doubleDashPosition == 0 && inputArg.size() == 2) {
                // Filter it out, it's a bash-like separator (end of the command options, before positionals arguments are accepted)
            } else if ((inputArg.size() > 2) && (inputArg[0] == '-') && (inputArg[1] != '-')) { // -abc style
                for (size_type c = 1; c < inputArg.size(); ++c) {
                    arguments.push_back(dash + inputArg[c]);
                }
            } else { // ?
                arguments.push_back(inputArg);
            }
        }

        // erase the first element, which is the name of the program
        const std::string programName = std::move(arguments.front());
        arguments.erase(arguments.begin());

        size_type const argCount = arguments.size();
        bool const legacyMode = (argCount == 0);

        // Set path of EnergyPlus program path (if we aren't overriding it)
        if (!state.dataGlobal->installRootOverride) {
            state.dataStrGlobals->exeDirectoryPath = FileSystem::getParentDirectoryPath(FileSystem::getAbsolutePath(FileSystem::getProgramPath()));
        }

        CLI::App app{"energyplus", programName};
        // opt.add("", false, 0, 0, "Display version information", "-v", "--version");
        app.set_version_flag("-v,--version", EnergyPlus::DataStringGlobals::VerString);

        std::string const description = fmt::format(R"({}
PythonLinkage: {}
Built on Platform: {}
)",
                                                    state.dataStrGlobals->VerStringVar,
                                                    PluginManagement::pythonStringForUsage(state),
                                                    DataStringGlobals::BuildPlatformString);
        app.description(description);

        auto *annualOpt = app.add_flag("-a,--annual", state.dataGlobal->AnnualSimulation, "Force annual simulation");

        app.add_flag("-D,--design-day", state.dataGlobal->DDOnlySimulation, "Force design-day-only simulation")->excludes(annualOpt);

        app.add_option("-d,--output-directory", state.dataStrGlobals->outDirPath, "Output directory path (default: current directory)")
            ->option_text("DIR")
            ->required(false);
        // ->check(CLI::ExistingDirectory) // We don't require it to exist, we make it if needed

        if (legacyMode) {
            state.dataStrGlobals->inputIddFilePath = "Energy+.idd";
        } else {
            state.dataStrGlobals->inputIddFilePath = state.dataStrGlobals->exeDirectoryPath / "Energy+.idd";
        }
        app.add_option(
               "-i,--idd", state.dataStrGlobals->inputIddFilePath, "Input data dictionary path (default: Energy+.idd in executable directory)")
            ->required(false)
            ->option_text("IDD")
            ->check(CLI::ExistingFile);

        bool runEPMacro = false;
        app.add_flag("-m,--epmacro", runEPMacro, "Run EPMacro prior to simulation");

        std::string prefixOutName = "eplus";
        app.add_option("-p,--output-prefix", prefixOutName, "Prefix for output file names (default: eplus)")->required(false)->option_text("PRE");

        app.add_flag("-r,--readvars", state.dataGlobal->runReadVars, "Run ReadVarsESO after simulation");

        app.add_flag("-c,--convert", state.dataGlobal->outputEpJSONConversion, "Output IDF->epJSON or epJSON->IDF, dependent on input file type");

        app.add_flag("--convert-only",
                     state.dataGlobal->outputEpJSONConversionOnly,
                     "Only convert IDF->epJSON or epJSON->IDF, dependent on input file type. No simulation");

        std::string suffixType = "L";
        std::string const suffixHelp = R"help(Suffix style for output file names (default: L)
   L: Legacy (e.g., eplustbl.csv)
   C: Capital (e.g., eplusTable.csv)
   D: Dash (e.g., eplus-table.csv))help";
        app.add_option("-s,--output-suffix", suffixType, suffixHelp)
            ->option_text("SUFFIX")
            ->required(false)
            ->check(CLI::IsMember({"L", "C", "D"}, CLI::ignore_case));

        // TODO: maybe delay validation to output a better error message?
        const int MAX_N = static_cast<int>(std::thread::hardware_concurrency());
        app.add_option("-j,--jobs",
                       state.dataGlobal->numThread,
                       "Multi-thread with N threads; 1 thread with no arg. (Currently only for G-Function generation)")
            ->option_text("N")
            // ->check(CLI::Range(1, MAX_N)  // Tempted to just do that... much simpler
            // ->check(CLI::Number)
            ->transform([MAX_N, &state](std::string input) -> std::string {
                int number_of_threads = -1;
                bool const converted = CLI::detail::lexical_cast(input, number_of_threads);
                if (!converted) {
                    // CLI::ValidationError
                    return fmt::format("Argument should be an integer, not '{}'", input);
                }
                if (number_of_threads <= 0) {
                    DisplayString(state, "Invalid value for -j arg. Defaulting to 1.");
                    return "1";
                }
                if (number_of_threads > MAX_N) {
                    DisplayString(state,
                                  fmt::format("Invalid value for -j arg. Value exceeds num available. Defaulting to num available. -j {}", MAX_N));
                    return std::to_string(MAX_N);
                }
                return input;
            });

        state.files.inputWeatherFilePath.filePath = "in.epw";
        // Note: we can't do check(CLI::ExistingFile) here since passing a non-existing /path/to/myfile.epw file
        // when there exists a /path/to/myfile.stat would mean the Weather File Statistics are still parsed and reported to the tabular output
        // We still report a message + fail later if DDOnlySimulation is false
        auto *weatherPathOpt =
            app.add_option("-w,--weather", state.files.inputWeatherFilePath.filePath, "Weather file path (default: in.epw in current directory)")
                ->required(false)
                ->option_text("EPW");

        bool runExpandObjects = false;
        app.add_flag("-x,--expandobjects", runExpandObjects, "Run ExpandObjects prior to simulation");

        // Positional
        state.dataStrGlobals->inputFilePath = "in.idf";
        app.add_option("input_file", state.dataStrGlobals->inputFilePath, "Input file (default: in.idf in current directory)")
            ->required(false)
            ->check(CLI::ExistingFile);

        // Catching it myself, so I can print the arguments vector before it's mutated
        bool debugCLI = std::any_of(args.begin(), args.end(), [](const auto &arg) { return arg == "--debug-cli"; });
        if (debugCLI) {
            {
                fmt::print("ProcessArgs: received args\n");
                int na = 0;
                for (const auto &a : args) {
                    fmt::print("* {}: '{}'\n", na++, a);
                }
            }
            {
                fmt::print("\nAfter massaging/expanding of args\n");
                int na = 0;
                for (const auto &a : arguments) {
                    fmt::print("* {}: '{}'\n", na++, a);
                }
            }
            fmt::print("\n");
        }
        // bool debugCLI = false;
        app.add_flag("--debug-cli", debugCLI, "Print the result of the CLI assignments to the console and exit")->group(""); // Empty group to hide it

#if LINK_WITH_PYTHON
#    ifdef PYTHON_CLI
        auto *auxiliaryToolsSubcommand = app.add_subcommand("auxiliary", "Run Auxiliary Python Tools");
        auxiliaryToolsSubcommand->require_subcommand(); // should default to requiring 1 or more additional args?

        std::vector<std::string> python_fwd_args;
        auto *epLaunchSubCommand = auxiliaryToolsSubcommand->add_subcommand("eplaunch", "EnergyPlus Launch");
        epLaunchSubCommand->add_option("args", python_fwd_args, "Extra Arguments forwarded to EnergyPlus Launch")->option_text("ARG ...");
        epLaunchSubCommand->positionals_at_end(true);
        epLaunchSubCommand->footer("You can pass extra arguments after the eplaunch keyword, they will be forwarded to EnergyPlus Launch.");

        epLaunchSubCommand->callback([&state, &python_fwd_args] {
            EnergyPlus::Python::PythonEngine engine(state);
            // There's probably better to be done, like instantiating the pythonEngine with the argc/argv then calling PyRun_SimpleFile but whatever
            std::string cmd = Python::PythonEngine::getTclPreppedPreamble(python_fwd_args);
            cmd += R"python(
from eplaunch.tk_runner import main_gui
main_gui(True)
)python";
            engine.exec(cmd);
            exit(0);
        });

        auto *updaterSubCommand = auxiliaryToolsSubcommand->add_subcommand("updater", "IDF Version Updater");
        updaterSubCommand->add_option("args", python_fwd_args, "Extra Arguments forwarded to IDF Version Updater")->option_text("ARG ...");
        updaterSubCommand->positionals_at_end(true);
        updaterSubCommand->footer("You can pass extra arguments after the updater keyword, they will be forwarded to IDF Version Updater.");

        updaterSubCommand->callback([&state, &python_fwd_args] {
            EnergyPlus::Python::PythonEngine engine(state);
            // There's probably better to be done, like instantiating the pythonEngine with the argc/argv then calling PyRun_SimpleFile but whatever
            std::string cmd = Python::PythonEngine::getTclPreppedPreamble(python_fwd_args);
            cmd += R"python(
from energyplus_transition.runner import main_gui
main_gui(True)
)python";
            engine.exec(cmd);
            exit(0);
        });

        auto *gheDesignerSubCommand = auxiliaryToolsSubcommand->add_subcommand("ghedesigner", "GHEDesigner Operation");
        gheDesignerSubCommand->add_option("args", python_fwd_args, "Extra Arguments forwarded to GHEDesigner")->option_text("ARG ...");
        gheDesignerSubCommand->positionals_at_end(true);
        gheDesignerSubCommand->footer(
            "You can pass extra arguments after the ghedesigner keyword, they should be the input file and output directory.");

        gheDesignerSubCommand->callback([&state, &python_fwd_args] {
            EnergyPlus::Python::PythonEngine engine(state);
            // There's probably better to be done, like instantiating the pythonEngine with the argc/argv then calling PyRun_SimpleFile but whatever
            std::string cmd = Python::PythonEngine::getTclPreppedPreamble(python_fwd_args);
            cmd += R"python(
from ghedesigner.main import run_manager_from_cli
run_manager_from_cli()
)python";
            try {
                engine.exec(cmd);
                exit(0);
            } catch (std::runtime_error &) {
                exit(1);
            }
        });
#    endif
#endif

        app.footer("Example: energyplus -w weather.epw -r input.idf");

        const bool eplusRunningViaAPI = state.dataGlobal->eplusRunningViaAPI;

        try {
            // app.parse(argc, argv);
            // CLI11 when passing argc, argv creates a vector<string> but **in reverse** order:
            // https://github.com/CLIUtils/CLI11/blob/291c58789c031208f08f4f261a858b5b7083e8e2/include/CLI/impl/App_inl.hpp#L476-L488
            std::reverse(arguments.begin(), arguments.end());
            app.parse(arguments);
        } catch (const CLI::Success &e) {
            int const return_code = app.exit(e);
            if (eplusRunningViaAPI) {
                return static_cast<int>(ReturnCodes::SuccessButHelper);
            } else {
                exit(return_code);
            }
        } catch (const CLI::ParseError &e) {
            int const return_code = app.exit(e);
            if (eplusRunningViaAPI) {
                return static_cast<int>(ReturnCodes::Failure);
            } else {
                exit(return_code);
            }
        }

        if (debugCLI) {
            fmt::print(stderr,
                       R"debug(
state.dataGlobal->AnnualSimulation = {},
state.dataGlobal->DDOnlySimulation = {},
state.dataStrGlobals->outDirPath = '{:g}',
state.dataStrGlobals->inputIddFilePath= '{:g}',

runEPMacro = {},
prefixOutName = {},

state.dataGlobal->runReadVars={},
state.dataGlobal->outputEpJSONConversion={},
state.dataGlobal->outputEpJSONConversionOnly={},

suffixType={},

state.dataGlobal->numThread={},
state.files.inputWeatherFilePath.filePath='{:g}',
state.dataStrGlobals->inputFilePath='{:g}',
)debug",
                       state.dataGlobal->AnnualSimulation,
                       state.dataGlobal->DDOnlySimulation,
                       state.dataStrGlobals->outDirPath,
                       state.dataStrGlobals->inputIddFilePath,

                       runEPMacro,
                       prefixOutName,
                       state.dataGlobal->runReadVars,
                       state.dataGlobal->outputEpJSONConversion,
                       state.dataGlobal->outputEpJSONConversionOnly,
                       suffixType,
                       state.dataGlobal->numThread,
                       state.files.inputWeatherFilePath.filePath,
                       state.dataStrGlobals->inputFilePath);

            fmt::print(stderr, "--debug-cli passed: exiting early\n");

            exit(0);
        }

        // Convert all paths to native paths
        state.dataStrGlobals->inputFilePath = FileSystem::makeNativePath(state.dataStrGlobals->inputFilePath);
        state.files.inputWeatherFilePath.filePath = FileSystem::makeNativePath(state.files.inputWeatherFilePath.filePath);
        state.dataStrGlobals->inputIddFilePath = FileSystem::makeNativePath(state.dataStrGlobals->inputIddFilePath);
        state.dataStrGlobals->outDirPath = FileSystem::makeNativePath(state.dataStrGlobals->outDirPath);

        state.dataStrGlobals->inputFilePathNameOnly = FileSystem::getFileName(state.dataStrGlobals->inputFilePath);
        state.dataStrGlobals->inputDirPath = FileSystem::getParentDirectoryPath(state.dataStrGlobals->inputFilePath);

        {
            FileSystem::FileTypes const fileType = FileSystem::getFileType(state.dataStrGlobals->inputFilePath);
            state.dataGlobal->isEpJSON = FileSystem::is_all_json_type(fileType);
            switch (fileType) {
            case FileSystem::FileTypes::IDF:
            case FileSystem::FileTypes::IMF:
            case FileSystem::FileTypes::EpJSON:
            case FileSystem::FileTypes::JSON:
                break;
            case FileSystem::FileTypes::CBOR:
                DisplayString(state, "CBOR input format is experimental and unsupported.");
                break;
            case FileSystem::FileTypes::MsgPack:
                DisplayString(state, "MsgPack input format is experimental and unsupported.");
                break;
            case FileSystem::FileTypes::UBJSON:
                DisplayString(state, "UBJSON input format is experimental and unsupported.");
                break;
            case FileSystem::FileTypes::BSON:
                DisplayString(state, "BSON input format is experimental and unsupported.");
                break;
            default:
                DisplayString(state,
                              fmt::format("ERROR: Input file must have IDF, IMF, or epJSON extension: {:g}", state.dataStrGlobals->inputFilePath));
                if (eplusRunningViaAPI) {
                    return static_cast<int>(ReturnCodes::Failure);
                } else {
                    exit(EXIT_FAILURE);
                }
            }
        }

        if (!state.dataStrGlobals->outDirPath.empty()) {
            // Create directory if it doesn't already exist
            FileSystem::makeDirectory(state.dataStrGlobals->outDirPath);
        }

        // File naming scheme
        fs::path outputFilePrefixFullPath = state.dataStrGlobals->outDirPath / prefixOutName;

        fs::path outputEpmdetFilePath;
        fs::path outputEpmidfFilePath;

        fs::path outputExpidfFilePath;
        fs::path outputExperrFilePath;

        std::string normalSuffix;
        std::string tableSuffix;
        std::string mapSuffix;
        std::string zszSuffix;
        std::string spszSuffix;
        std::string sszSuffix;
        std::string meterSuffix;
        std::string sqliteSuffix;
        std::string adsSuffix;
        std::string screenSuffix;
        std::string shdSuffix;

        std::string const errorFollowUp = "Type 'energyplus --help' for usage.";
        {
            std::transform(suffixType.begin(), suffixType.end(), suffixType.begin(), ::toupper);

            if (suffixType == "L") {

                normalSuffix = "out";
                tableSuffix = "tbl";
                mapSuffix = "map";
                zszSuffix = "zsz";
                spszSuffix = "spsz";
                sszSuffix = "ssz";
                meterSuffix = "mtr";
                sqliteSuffix = "sqlite";
                adsSuffix = "ADS";
                screenSuffix = "screen";
                shdSuffix = "shading";

            } else if (suffixType == "D") {

                normalSuffix = "";
                tableSuffix = "-table";
                mapSuffix = "-map";
                zszSuffix = "-zsz";
                spszSuffix = "-spsz";
                sszSuffix = "-ssz";
                meterSuffix = "-meter";
                sqliteSuffix = "-sqlite";
                adsSuffix = "-ads";
                screenSuffix = "-screen";
                shdSuffix = "-shading";

            } else if (suffixType == "C") {

                normalSuffix = "";
                tableSuffix = "Table";
                mapSuffix = "Map";
                zszSuffix = "Zsz";
                spszSuffix = "Spsz";
                sszSuffix = "Ssz";
                meterSuffix = "Meter";
                sqliteSuffix = "Sqlite";
                adsSuffix = "Ads";
                screenSuffix = "Screen";
                shdSuffix = "Shading";
            }
            // No else needed, this is validated at CLI level above
        }

        // Helper to construct output file path
        auto composePath = [&outputFilePrefixFullPath](const std::string &suffix) -> fs::path {
            return FileSystem::appendSuffixToPath(outputFilePrefixFullPath, suffix);
        };

        // EnergyPlus files
        state.files.audit.filePath = composePath(normalSuffix + ".audit");
        state.files.bnd.filePath = composePath(normalSuffix + ".bnd");
        state.files.dxf.filePath = composePath(normalSuffix + ".dxf");
        state.files.eio.filePath = composePath(normalSuffix + ".eio");
        state.files.endFile.filePath = composePath(normalSuffix + ".end");
        state.files.outputErrFilePath = composePath(normalSuffix + ".err");
        state.files.eso.filePath = composePath(normalSuffix + ".eso");

        state.files.json.outputJsonFilePath = composePath(normalSuffix + ".json");
        state.files.json.outputTSZoneJsonFilePath = composePath(normalSuffix + "_detailed_zone.json");
        state.files.json.outputTSHvacJsonFilePath = composePath(normalSuffix + "_detailed_HVAC.json");
        state.files.json.outputTSJsonFilePath = composePath(normalSuffix + "_timestep.json");
        state.files.json.outputYRJsonFilePath = composePath(normalSuffix + "_yearly.json");
        state.files.json.outputMNJsonFilePath = composePath(normalSuffix + "_monthly.json");
        state.files.json.outputDYJsonFilePath = composePath(normalSuffix + "_daily.json");
        state.files.json.outputHRJsonFilePath = composePath(normalSuffix + "_hourly.json");
        state.files.json.outputSMJsonFilePath = composePath(normalSuffix + "_runperiod.json");
        state.files.json.outputCborFilePath = composePath(normalSuffix + ".cbor");
        state.files.json.outputTSZoneCborFilePath = composePath(normalSuffix + "_detailed_zone.cbor");
        state.files.json.outputTSHvacCborFilePath = composePath(normalSuffix + "_detailed_HVAC.cbor");
        state.files.json.outputTSCborFilePath = composePath(normalSuffix + "_timestep.cbor");
        state.files.json.outputYRCborFilePath = composePath(normalSuffix + "_yearly.cbor");
        state.files.json.outputMNCborFilePath = composePath(normalSuffix + "_monthly.cbor");
        state.files.json.outputDYCborFilePath = composePath(normalSuffix + "_daily.cbor");
        state.files.json.outputHRCborFilePath = composePath(normalSuffix + "_hourly.cbor");
        state.files.json.outputSMCborFilePath = composePath(normalSuffix + "_runperiod.cbor");
        state.files.json.outputMsgPackFilePath = composePath(normalSuffix + ".msgpack");
        state.files.json.outputTSZoneMsgPackFilePath = composePath(normalSuffix + "_detailed_zone.msgpack");
        state.files.json.outputTSHvacMsgPackFilePath = composePath(normalSuffix + "_detailed_HVAC.msgpack");
        state.files.json.outputTSMsgPackFilePath = composePath(normalSuffix + "_timestep.msgpack");
        state.files.json.outputYRMsgPackFilePath = composePath(normalSuffix + "_yearly.msgpack");
        state.files.json.outputMNMsgPackFilePath = composePath(normalSuffix + "_monthly.msgpack");
        state.files.json.outputDYMsgPackFilePath = composePath(normalSuffix + "_daily.msgpack");
        state.files.json.outputHRMsgPackFilePath = composePath(normalSuffix + "_hourly.msgpack");
        state.files.json.outputSMMsgPackFilePath = composePath(normalSuffix + "_runperiod.msgpack");

        state.files.mtd.filePath = composePath(normalSuffix + ".mtd");
        state.files.mdd.filePath = composePath(normalSuffix + ".mdd");
        state.files.mtr.filePath = composePath(normalSuffix + ".mtr");
        state.files.rdd.filePath = composePath(normalSuffix + ".rdd");
        state.dataStrGlobals->outputShdFilePath = composePath(normalSuffix + ".shd");
        state.files.dfs.filePath = composePath(normalSuffix + ".dfs");
        state.dataStrGlobals->outputGLHEFilePath = composePath(normalSuffix + ".glhe");
        state.files.edd.filePath = composePath(normalSuffix + ".edd");
        state.dataStrGlobals->outputIperrFilePath = composePath(normalSuffix + ".iperr");
        state.files.sln.filePath = composePath(normalSuffix + ".sln");
        state.files.sci.filePath = composePath(normalSuffix + ".sci");
        state.files.wrl.filePath = composePath(normalSuffix + ".wrl");
        state.dataStrGlobals->outputSqlFilePath = composePath(normalSuffix + ".sql");
        state.files.debug.filePath = composePath(normalSuffix + ".dbg");
        state.dataStrGlobals->outputPerfLogFilePath = composePath(normalSuffix + "_perflog.csv");
        state.dataStrGlobals->outputTblCsvFilePath = composePath(tableSuffix + ".csv");
        state.dataStrGlobals->outputTblHtmFilePath = composePath(tableSuffix + ".htm");
        state.dataStrGlobals->outputTblTabFilePath = composePath(tableSuffix + ".tab");
        state.dataStrGlobals->outputTblTxtFilePath = composePath(tableSuffix + ".txt");
        state.dataStrGlobals->outputTblXmlFilePath = composePath(tableSuffix + ".xml");
        state.files.outputMapTabFilePath = composePath(mapSuffix + ".tab");
        state.files.outputMapCsvFilePath = composePath(mapSuffix + ".csv");
        state.files.outputMapTxtFilePath = composePath(mapSuffix + ".txt");
        state.files.outputZszCsvFilePath = composePath(zszSuffix + ".csv");
        state.files.outputZszTabFilePath = composePath(zszSuffix + ".tab");
        state.files.outputZszTxtFilePath = composePath(zszSuffix + ".txt");
        state.files.outputSpszCsvFilePath = composePath(spszSuffix + ".csv");
        state.files.outputSpszTabFilePath = composePath(spszSuffix + ".tab");
        state.files.outputSpszTxtFilePath = composePath(spszSuffix + ".txt");
        state.files.outputSszCsvFilePath = composePath(sszSuffix + ".csv");
        state.files.outputSszTabFilePath = composePath(sszSuffix + ".tab");
        state.files.outputSszTxtFilePath = composePath(sszSuffix + ".txt");
        state.dataStrGlobals->outputAdsFilePath = composePath(adsSuffix + ".out");
        state.files.shade.filePath = composePath(shdSuffix + ".csv");
        if (suffixType == "L") {
            // out/sqlite.err
            state.dataStrGlobals->outputSqliteErrFilePath = state.dataStrGlobals->outDirPath / fs::path{sqliteSuffix + ".err"};
        } else {
            // if 'D':  out/eplus-sqlite.err
            state.dataStrGlobals->outputSqliteErrFilePath = composePath(sqliteSuffix + ".err");
        }

        state.files.screenCsv.filePath = composePath(screenSuffix + ".csv");
        // TODO, why are these relative paths?
        state.files.delightIn.filePath = "eplusout.delightin";
        state.dataStrGlobals->outputDelightOutFilePath = "eplusout.delightout";

        // TODO: why is this relative?
        state.files.iniFile.filePath = "Energy+.ini";

        // Stat next to epw
        // Careful! fs::path::replace_extension will **mutate the original object**
        state.files.inStatFilePath.filePath = state.files.inputWeatherFilePath.filePath;
        state.files.inStatFilePath.filePath.replace_extension(".stat");
        // Or is it better to provide a helper that does not mutate like this?
        // state.files.inStatFilePath.filePath = FileSystem::replaceFileExtension(state.inputWeatherFilePath.filePath, ".stat");

        state.dataStrGlobals->eplusADSFilePath = state.dataStrGlobals->inputDirPath / "eplusADS.inp";

        // Readvars files
        state.files.csv.filePath = composePath(normalSuffix + ".csv");
        state.files.mtr_csv.filePath = composePath(meterSuffix + ".csv");
        state.dataStrGlobals->outputRvauditFilePath = composePath(normalSuffix + ".rvaudit");

        // EPMacro files
        outputEpmdetFilePath = composePath(normalSuffix + ".epmdet");
        outputEpmidfFilePath = composePath(normalSuffix + ".epmidf");

        // ExpandObjects files
        outputExpidfFilePath = composePath(normalSuffix + ".expidf");
        outputExperrFilePath = composePath(normalSuffix + ".experr");

        // Read path from INI file if it exists

        // Check for IDD and IDF files
        if (FileSystem::fileExists(state.files.iniFile.filePath)) {
            EnergyPlus::InputFile iniFile = state.files.iniFile.try_open();
            if (!iniFile.good()) {
                DisplayString(state, fmt::format("ERROR: Could not open file {} for input (read).", iniFile.filePath));
                if (eplusRunningViaAPI) {
                    return static_cast<int>(ReturnCodes::Failure);
                } else {
                    exit(EXIT_FAILURE);
                }
            }
            state.dataStrGlobals->CurrentWorkingFolder = iniFile.filePath;
            // Relying on compiler to supply full path name here
            // TODO: not sure I understand this block
            // const int TempIndx = index(state.dataStrGlobals->CurrentWorkingFolder, state.dataStrGlobals->pathChar, true);
            // if (TempIndx == std::string::npos) {
            // state.dataStrGlobals->CurrentWorkingFolder = "";
            //} else {
            // state.dataStrGlobals->CurrentWorkingFolder.erase(TempIndx + 1);
            //}
            //       Get directories from ini file
            std::string programPathStr;
            ReadINIFile(iniFile, "program", "dir", programPathStr);
            state.dataStrGlobals->ProgramPath = fs::path(programPathStr);

            state.dataStrGlobals->inputIddFilePath = state.dataStrGlobals->ProgramPath / "Energy+.idd";
        }

        // Check if specified files exist
        if (!FileSystem::fileExists(state.dataStrGlobals->inputFilePath)) {
            DisplayString(
                state, fmt::format("ERROR: Could not find input data file: {}.", FileSystem::getAbsolutePath(state.dataStrGlobals->inputFilePath)));
            DisplayString(state, errorFollowUp);
            if (eplusRunningViaAPI) {
                return static_cast<int>(ReturnCodes::Failure);
            } else {
                exit(EXIT_FAILURE);
            }
        }

        if ((weatherPathOpt->count() > 0) && !state.dataGlobal->DDOnlySimulation) {
            if (!FileSystem::fileExists(state.files.inputWeatherFilePath.filePath)) {
                DisplayString(
                    state,
                    fmt::format("ERROR: Could not find weather file: {}.", FileSystem::getAbsolutePath(state.files.inputWeatherFilePath.filePath)));
                DisplayString(state, errorFollowUp);
                if (eplusRunningViaAPI) {
                    return static_cast<int>(ReturnCodes::Failure);
                } else {
                    exit(EXIT_FAILURE);
                }
            }
        }

        // TODO: might be able to convert epJSON->IDF, run preprocessors, then go back IDF->epJSON

        // Preprocessors (These will likely move to a new file)
        if (runEPMacro) {
            fs::path epMacroPath = (state.dataStrGlobals->exeDirectoryPath / "EPMacro").replace_extension(FileSystem::exeExtension);
            if (!FileSystem::fileExists(epMacroPath)) {
                DisplayString(state, fmt::format("ERROR: Could not find EPMacro executable: {}.", FileSystem::getAbsolutePath(epMacroPath)));
                if (eplusRunningViaAPI) {
                    return static_cast<int>(ReturnCodes::Failure);
                } else {
                    exit(EXIT_FAILURE);
                }
            }
            std::string epMacroCommand = "\"" + FileSystem::toString(epMacroPath) + "\"";
            bool inputFilePathdIn = (FileSystem::getAbsolutePath(state.dataStrGlobals->inputFilePath) == FileSystem::getAbsolutePath("in.imf"));

            if (!inputFilePathdIn) {
                FileSystem::linkFile(state.dataStrGlobals->inputFilePath, "in.imf");
            }
            DisplayString(state, "Running EPMacro...");
            FileSystem::systemCall(epMacroCommand);
            if (!inputFilePathdIn) {
                FileSystem::removeFile("in.imf");
            }
            FileSystem::moveFile("audit.out", outputEpmdetFilePath);
            FileSystem::moveFile("out.idf", outputEpmidfFilePath);
            state.dataStrGlobals->inputFilePath = outputEpmidfFilePath;
        }

        if (runExpandObjects) {
            fs::path expandObjectsPath =
                (state.dataStrGlobals->exeDirectoryPath / fs::path("ExpandObjects")).replace_extension(FileSystem::exeExtension);
            if (!FileSystem::fileExists(expandObjectsPath)) {
                DisplayString(state,
                              fmt::format("ERROR: Could not find ExpandObjects executable: {}.", FileSystem::getAbsolutePath(expandObjectsPath)));
                if (eplusRunningViaAPI) {
                    return static_cast<int>(ReturnCodes::Failure);
                } else {
                    exit(EXIT_FAILURE);
                }
            }
            std::string expandObjectsCommand = "\"" + FileSystem::toString(expandObjectsPath) + "\"";
            bool inputFilePathdIn = (FileSystem::getAbsolutePath(state.dataStrGlobals->inputFilePath) == FileSystem::getAbsolutePath("in.idf"));

            // check if IDD actually exists since ExpandObjects still requires it
            if (!FileSystem::fileExists(state.dataStrGlobals->inputIddFilePath)) {
                DisplayString(state,
                              fmt::format("ERROR: Could not find input data dictionary: {}.",
                                          FileSystem::getAbsolutePath(state.dataStrGlobals->inputIddFilePath)));
                DisplayString(state, errorFollowUp);
                if (eplusRunningViaAPI) {
                    return static_cast<int>(ReturnCodes::Failure);
                } else {
                    exit(EXIT_FAILURE);
                }
            }

            bool iddFilePathdEnergy =
                (FileSystem::getAbsolutePath(state.dataStrGlobals->inputIddFilePath) == FileSystem::getAbsolutePath("Energy+.idd"));

            if (!inputFilePathdIn) {
                FileSystem::linkFile(state.dataStrGlobals->inputFilePath, "in.idf");
            }
            if (!iddFilePathdEnergy) {
                FileSystem::linkFile(state.dataStrGlobals->inputIddFilePath, "Energy+.idd");
            }

            FileSystem::systemCall(expandObjectsCommand);
            if (!inputFilePathdIn) {
                FileSystem::removeFile("in.idf");
            }
            if (!iddFilePathdEnergy) {
                FileSystem::removeFile("Energy+.idd");
            }

            FileSystem::moveFile("expandedidf.err", outputExperrFilePath);
            if (FileSystem::fileExists("expanded.idf")) {
                FileSystem::moveFile("expanded.idf", outputExpidfFilePath);
                state.dataStrGlobals->inputFilePath = outputExpidfFilePath;
            }
        }

        return static_cast<int>(ReturnCodes::Success);
    }

    // Fix This is Fortranic code that needs to be brought up to C++ style
    //     All the index and len and strip should be eliminated and replaced by string calls only where needed
    //     I/o with std::string should not be pulling in trailing blanks so stripping should not be needed, etc.
    //     Rewinding is a big performance hit and should be avoided if possible
    //     Case-insensitive comparison is much faster than converting strings to upper or lower case
    //     Each strip and case conversion is a heap hit and should be avoided if possible
    void ReadINIFile(InputFile &inputFile,               // Unit number of the opened INI file
                     std::string const &Heading,         // Heading for the parameters ('[heading]')
                     std::string const &KindofParameter, // Kind of parameter to be found (String)
                     std::string &DataOut                // Output from the retrieval
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   September 1997
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This routine reads the .ini file and retrieves
        // the path names for the files from it.

        // METHODOLOGY EMPLOYED:
        // Duplicate the kind of reading the Windows "GetINISetting" would
        // do.

        // Using/Aliasing
        using namespace EnergyPlus;
        using namespace DataStringGlobals;

        std::string Param;
        std::string::size_type ILB;
        std::string::size_type IRB;
        std::string::size_type IEQ;
        std::string::size_type IPAR;
        std::string::size_type IPOS;

        // Formats

        DataOut.clear();

        // I tried ADJUSTL(TRIM(KindofParameter)) and got an internal compiler error

        Param = KindofParameter;
        strip(Param);
        inputFile.rewind();
        bool Found = false;
        bool NewHeading = false;

        while (inputFile.good() && !Found) {
            EnergyPlus::InputFile::ReadResult const readResult = inputFile.readLine();

            if (readResult.eof) {
                break;
            }

            if (readResult.data.empty()) {
                continue;
            } // Ignore Blank Lines

            std::string LINEOut;
            ConvertCaseToLower(readResult.data, LINEOut); // Turn line into lower case
            //        LINE=LINEOut

            if (!has(LINEOut, Heading)) {
                continue;
            }

            //                                  See if [ and ] are on line
            ILB = index(LINEOut, '[');
            IRB = index(LINEOut, ']');
            if (ILB == std::string::npos && IRB == std::string::npos) {
                continue;
            }
            if (!has(LINEOut, '[' + Heading + ']')) {
                continue; // Must be really correct heading line
            }

            //                                  Heading line found, now looking for Kind
            while (inputFile.good() && !NewHeading) {
                const auto innerReadResult = inputFile.readLine(); // readLine returns a ReadResult<std::string>, hence no & (THIS_AUTO_OK)
                if (innerReadResult.eof) {
                    break;
                }

                std::string line = innerReadResult.data;
                strip(line);

                if (line.empty()) {
                    continue; // Ignore Blank Lines
                }

                ConvertCaseToLower(line, LINEOut); // Turn line into lower case
                //         LINE=LINEOut

                ILB = index(LINEOut, '[');
                IRB = index(LINEOut, ']');
                NewHeading = (ILB != std::string::npos && IRB != std::string::npos);

                //                                  Should be a parameter line
                //                                  KindofParameter = string
                IEQ = index(LINEOut, '=');
                IPAR = index(LINEOut, Param);
                if (IEQ == std::string::npos) {
                    continue;
                }
                if (IPAR == std::string::npos) {
                    continue;
                }
                if (IPAR != 0) {
                    continue;
                }
                if (!has(LINEOut, Param + '=')) {
                    continue; // needs to be param=
                }

                //                                  = found and parameter found.
                if (IPAR > IEQ) {
                    continue;
                }

                //                                  parameter = found
                //                                  Set output string to start with non-blank character

                DataOut = stripped(line.substr(IEQ + 1));
                Found = true;
                break;
            }
        }

        if (Param == "dir") {
            IPOS = len(DataOut);
            if (IPOS != 0) {
                // Non-blank make sure last position is valid path character
                //  (Set in DataStringGlobals)

                if (DataOut[IPOS - 1] != pathChar) {
                    DataOut += pathChar;
                }
            }
        }
    }

    int runReadVarsESO(EnergyPlusData &state)
    {
        fs::path readVarsPath = (state.dataStrGlobals->exeDirectoryPath / "ReadVarsESO").replace_extension(FileSystem::exeExtension);

        if (!FileSystem::fileExists(readVarsPath)) {
            readVarsPath = (state.dataStrGlobals->exeDirectoryPath / "PostProcess" / "ReadVarsESO").replace_extension(FileSystem::exeExtension);
            if (!FileSystem::fileExists(readVarsPath)) {
                // should report the error differently if the user is calling into E+ through EXE or DLL
                if (state.dataGlobal->eplusRunningViaAPI) {
                    DisplayString(
                        state,
                        "ERROR: Could not find ReadVarsESO executable.  When calling through C API, make sure to call setEnergyPlusRootDirectory");
                } else {
                    DisplayString(state, fmt::format("ERROR: Could not find ReadVarsESO executable: {}.", FileSystem::getAbsolutePath(readVarsPath)));
                }
                return static_cast<int>(ReturnCodes::Failure);
            }
        }

        fs::path const RVIfile = (state.dataStrGlobals->inputDirPath / state.dataStrGlobals->inputFilePathNameOnly).replace_extension(".rvi");
        fs::path const MVIfile = (state.dataStrGlobals->inputDirPath / state.dataStrGlobals->inputFilePathNameOnly).replace_extension(".mvi");

        const bool rviFileExists = FileSystem::fileExists(RVIfile);
        if (!rviFileExists) {
            std::ofstream ofs{RVIfile};
            if (!ofs.good()) {
                ShowFatalError(state, format("EnergyPlus: Could not open file \"{}\" for output (write).", RVIfile));
            } else {
                ofs << FileSystem::toString(state.files.eso.filePath) << '\n';
                ofs << FileSystem::toString(state.files.csv.filePath) << '\n';
            }
        }

        const bool mviFileExists = FileSystem::fileExists(MVIfile);
        if (!mviFileExists) {
            std::ofstream ofs{MVIfile};
            if (!ofs.good()) {
                ShowFatalError(state, format("EnergyPlus: Could not open file \"{}\" for output (write).", RVIfile));
            } else {
                ofs << FileSystem::toString(state.files.mtr.filePath) << '\n';
                ofs << FileSystem::toString(state.files.mtr_csv.filePath) << '\n';
            }
        }

        // We quote the paths in case we have spaces
        // "/Path/to/ReadVarEso" "/Path/to/folder with spaces/file.rvi" unlimited
        std::string const readVarsRviCommand = "\"" + FileSystem::toString(readVarsPath) + "\" \"" + FileSystem::toString(RVIfile) + "\" unlimited";
        std::string const readVarsMviCommand = "\"" + FileSystem::toString(readVarsPath) + "\" \"" + FileSystem::toString(MVIfile) + "\" unlimited";

        // systemCall will be responsible to handle to above command on Windows versus Unix
        FileSystem::systemCall(readVarsRviCommand);
        FileSystem::systemCall(readVarsMviCommand);

        if (!rviFileExists) {
            FileSystem::removeFile(RVIfile);
        }

        if (!mviFileExists) {
            FileSystem::removeFile(MVIfile);
        }

        FileSystem::moveFile("readvars.audit", state.dataStrGlobals->outputRvauditFilePath);
        return static_cast<int>(ReturnCodes::Success);
    }

} // namespace CommandLineInterface
} // namespace EnergyPlus
