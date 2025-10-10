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

// ObjexxFCL Headers
#include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataOutputs.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::DataOutputs {

// Module containing the data and routines dealing with prescanning for
// requested output variables to limit the number being processed in OutputProcessor
// Also any input counts (such as autosize counts/records that are used
// by later program modules.

// MODULE INFORMATION:
//       AUTHOR         Linda Lawrie
//       DATE WRITTEN   July 2010
//       MODIFIED       April 2011; to include autosize counts
//       RE-ENGINEERED  na

// PURPOSE OF THIS MODULE:
// The module contains structure for output variables that are used in a small number of modules.

// MODULE PARAMETER DEFINITIONS:
Array1D_string const MonthlyNamedReports(NumMonthlyReports,
                                         {"ZONECOOLINGSUMMARYMONTHLY",
                                          "ZONEHEATINGSUMMARYMONTHLY",
                                          "ZONEELECTRICSUMMARYMONTHLY",
                                          "SPACEGAINSMONTHLY",
                                          "PEAKSPACEGAINSMONTHLY",
                                          "SPACEGAINCOMPONENTSATCOOLINGPEAKMONTHLY",
                                          "ENERGYCONSUMPTIONELECTRICITYNATURALGASMONTHLY",
                                          "ENERGYCONSUMPTIONELECTRICITYGENERATEDPROPANEMONTHLY",
                                          "ENERGYCONSUMPTIONDIESELFUELOILMONTHLY",
                                          "ENERGYCONSUMPTIONDISTRICTHEATINGCOOLINGMONTHLY",
                                          "ENERGYCONSUMPTIONCOALGASOLINEMONTHLY",
                                          "ENERGYCONSUMPTIONOTHERFUELSMONTHLY",
                                          "ENDUSEENERGYCONSUMPTIONELECTRICITYMONTHLY",
                                          "ENDUSEENERGYCONSUMPTIONNATURALGASMONTHLY",
                                          "ENDUSEENERGYCONSUMPTIONDIESELMONTHLY",
                                          "ENDUSEENERGYCONSUMPTIONFUELOILMONTHLY",
                                          "ENDUSEENERGYCONSUMPTIONCOALMONTHLY",
                                          "ENDUSEENERGYCONSUMPTIONPROPANEMONTHLY",
                                          "ENDUSEENERGYCONSUMPTIONGASOLINEMONTHLY",
                                          "ENDUSEENERGYCONSUMPTIONOTHERFUELSMONTHLY",
                                          "PEAKENERGYENDUSEELECTRICITYPART1MONTHLY",
                                          "PEAKENERGYENDUSEELECTRICITYPART2MONTHLY",
                                          "ELECTRICCOMPONENTSOFPEAKDEMANDMONTHLY",
                                          "PEAKENERGYENDUSENATURALGASMONTHLY",
                                          "PEAKENERGYENDUSEDIESELMONTHLY",
                                          "PEAKENERGYENDUSEFUELOILMONTHLY",
                                          "PEAKENERGYENDUSECOALMONTHLY",
                                          "PEAKENERGYENDUSEPROPANEMONTHLY",
                                          "PEAKENERGYENDUSEGASOLINEMONTHLY",
                                          "PEAKENERGYENDUSEOTHERFUELSMONTHLY",
                                          "SETPOINTSNOTMETWITHTEMPERATURESMONTHLY",
                                          "COMFORTREPORTSIMPLE55MONTHLY",
                                          "UNGLAZEDTRANSPIREDSOLARCOLLECTORSUMMARYMONTHLY",
                                          "OCCUPANTCOMFORTDATASUMMARYMONTHLY",
                                          "CHILLERREPORTMONTHLY",
                                          "TOWERREPORTMONTHLY",
                                          "BOILERREPORTMONTHLY",
                                          "DXREPORTMONTHLY",
                                          "WINDOWREPORTMONTHLY",
                                          "WINDOWENERGYREPORTMONTHLY",
                                          "WINDOWZONESUMMARYMONTHLY",
                                          "WINDOWENERGYZONESUMMARYMONTHLY",
                                          "AVERAGEOUTDOORCONDITIONSMONTHLY",
                                          "OUTDOORCONDITIONSMAXIMUMDRYBULBMONTHLY",
                                          "OUTDOORCONDITIONSMINIMUMDRYBULBMONTHLY",
                                          "OUTDOORCONDITIONSMAXIMUMWETBULBMONTHLY",
                                          "OUTDOORCONDITIONSMAXIMUMDEWPOINTMONTHLY",
                                          "OUTDOORGROUNDCONDITIONSMONTHLY",
                                          "WINDOWACREPORTMONTHLY",
                                          "WATERHEATERREPORTMONTHLY",
                                          "GENERATORREPORTMONTHLY",
                                          "DAYLIGHTINGREPORTMONTHLY",
                                          "COILREPORTMONTHLY",
                                          "PLANTLOOPDEMANDREPORTMONTHLY",
                                          "FANREPORTMONTHLY",
                                          "PUMPREPORTMONTHLY",
                                          "CONDLOOPDEMANDREPORTMONTHLY",
                                          "ZONETEMPERATUREOSCILLATIONREPORTMONTHLY",
                                          "AIRLOOPSYSTEMENERGYANDWATERUSEMONTHLY",
                                          "AIRLOOPSYSTEMCOMPONENTLOADSMONTHLY",
                                          "AIRLOOPSYSTEMCOMPONENTENERGYUSEMONTHLY",
                                          "MECHANICALVENTILATIONLOADSMONTHLY",
                                          "HEATEMISSIONSREPORTMONTHLY"});

bool isKeyRegexLike(std::string_view key)
{
    if (key == "*") {
        return false;
    }

    return key.find_first_of("*+?()|[]\\.") != std::string_view::npos;
}

OutputReportingVariables::OutputReportingVariables(EnergyPlusData &state, std::string const &KeyValue, std::string const &VariableName)
    : key(KeyValue), variableName(VariableName), is_simple_string(!DataOutputs::isKeyRegexLike(KeyValue))
{

    if (is_simple_string) {
        return;
    }
    pattern = std::make_shared<RE2>(KeyValue);
    case_insensitive_pattern = std::make_shared<RE2>("(?i)" + KeyValue);
    if (!pattern->ok()) {
        ShowSevereError(state, format("Regular expression \"{}\" for variable name \"{}\" in input file is incorrect", KeyValue, VariableName));
        ShowContinueError(state, pattern->error());
        ShowFatalError(state, "Error found in regular expression. Previous error(s) cause program termination.");
    }
}

bool FindItemInVariableList(const EnergyPlusData &state, std::string_view const KeyedValue, std::string_view const VariableName)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   July 2010
    //       MODIFIED       December 2016

    // PURPOSE OF THIS FUNCTION:
    // This function looks up a key and variable name value and determines if they are
    // in the list of required variables for a simulation.

    auto const found_variable = state.dataOutput->OutputVariablesForSimulation.find(VariableName);
    if (found_variable == state.dataOutput->OutputVariablesForSimulation.end()) {
        return false;
    }

    auto found_key = found_variable->second.find(KeyedValue);
    if (found_key != found_variable->second.end()) {
        return true;
    }

    found_key = found_variable->second.find("*");
    if (found_key != found_variable->second.end()) {
        return true;
    }

    for (auto it = found_variable->second.begin(); it != found_variable->second.end(); ++it) {
        if (equali(KeyedValue, it->second.key)) {
            return true;
        }
        if (it->second.is_simple_string) {
            continue;
        }
        if ((it->second.pattern != nullptr && RE2::FullMatch(std::string{KeyedValue}, *it->second.pattern)) || // match against regex as written
            (it->second.case_insensitive_pattern != nullptr &&
             RE2::FullMatch(std::string{KeyedValue}, *it->second.case_insensitive_pattern)) // attempt case-insensitive regex comparison
        ) {
            return true;
        }
    }
    return false;
}

} // namespace EnergyPlus::DataOutputs
