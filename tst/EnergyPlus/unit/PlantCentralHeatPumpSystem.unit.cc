// EnergyPlus, Copyright (c) 1996-present, The Board of Trustees of the University of Illinois,
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

// EnergyPlus::PlantCentralHeatPumpSystem Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchNodeConnections.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantCentralHeatPumpSystem.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/ScheduleManager.hh>

#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>
#include <string_view>
#include <vector>

using namespace EnergyPlus;

namespace {

using EnergyPlus::PlantCentralHeatPumpSystem::CurrentMode;

struct LoopRouting
{
    Real64 cooling = 0.0; // Heat removed from the chilled-water loop [W]
    Real64 heating = 0.0; // Heat added to the hot-water loop [W]
    Real64 source = 0.0;  // Positive rejects heat to source; negative extracts heat from source [W]
};

struct ModeEnergyContract
{
    CurrentMode mode = CurrentMode::Invalid;
    Real64 qEvaporator = 0.0;
    Real64 qCondenser = 0.0;
    Real64 compressorPower = 0.0;
    Real64 falseLoad = 0.0;
    Real64 openMotorEfficiency = 1.0;
    Real64 partLoadRatio = 0.0;
    Real64 cyclingRatio = 0.0;
    LoopRouting routing;
};

struct PartLoadContract
{
    Real64 availableCapacity = 0.0;
    Real64 requestedLoad = 0.0;
    Real64 deliveredLoad = 0.0;
    Real64 minimumPartLoadRatio = 0.0;
    Real64 maximumPartLoadRatio = 1.0;
    Real64 reportedPartLoadRatio = 0.0;
    Real64 cyclingRatio = 0.0;
    Real64 curveEvaluationPartLoadRatio = 0.0;
};

struct FlowAllocationContract
{
    Real64 systemAvailableFlow = 0.0;
    std::vector<Real64> requestedFlows;
    std::vector<Real64> moduleMaximumFlows;
    std::vector<Real64> allocatedFlows;
};

constexpr Real64 contractTolerance = 1.0e-9;

bool contractNear(Real64 const actual, Real64 const expected, Real64 const tolerance = contractTolerance)
{
    Real64 const scale = std::max({1.0, std::abs(actual), std::abs(expected)});
    return std::abs(actual - expected) <= tolerance * scale;
}

std::string_view modeName(CurrentMode const mode)
{
    switch (mode) {
    case CurrentMode::Off:
        return "Off";
    case CurrentMode::CoolingOnly:
        return "CoolingOnly";
    case CurrentMode::HeatingOnly:
        return "HeatingOnly";
    case CurrentMode::HeatRecovery:
        return "HeatRecovery";
    case CurrentMode::CoolingDominant:
        return "CoolingDominant";
    case CurrentMode::HeatingDominant:
        return "HeatingDominant";
    default:
        return "Invalid";
    }
}

::testing::AssertionResult checkModeEnergyContract(ModeEnergyContract const &point)
{
    auto fail = [&point](std::string_view const message) { return ::testing::AssertionFailure() << modeName(point.mode) << ": " << message; };

    if (point.mode == CurrentMode::Invalid || point.mode == CurrentMode::Num) {
        return fail("invalid operating mode");
    }

    std::array<Real64, 10> const values = {point.qEvaporator,
                                           point.qCondenser,
                                           point.compressorPower,
                                           point.falseLoad,
                                           point.openMotorEfficiency,
                                           point.partLoadRatio,
                                           point.cyclingRatio,
                                           point.routing.cooling,
                                           point.routing.heating,
                                           point.routing.source};
    if (!std::all_of(values.begin(), values.end(), [](Real64 const value) { return std::isfinite(value); })) {
        return fail("contract contains a non-finite value");
    }
    if (point.qEvaporator < 0.0 || point.qCondenser < 0.0 || point.compressorPower < 0.0 || point.falseLoad < 0.0) {
        return fail("heat-transfer rates, power, and false load must be nonnegative");
    }
    if (point.openMotorEfficiency < 0.0 || point.openMotorEfficiency > 1.0) {
        return fail("open motor efficiency is outside [0, 1]");
    }

    if (point.mode == CurrentMode::Off) {
        Real64 const magnitude = point.qEvaporator + point.qCondenser + point.compressorPower + point.falseLoad + std::abs(point.routing.cooling) +
                                 std::abs(point.routing.heating) + std::abs(point.routing.source) + std::abs(point.partLoadRatio) +
                                 std::abs(point.cyclingRatio);
        if (!contractNear(magnitude, 0.0)) {
            return fail("off mode has nonzero heat transfer, power, routing, PLR, or cycling");
        }
        return ::testing::AssertionSuccess();
    }

    if (point.compressorPower <= 0.0) {
        return fail("an active Electric:EIR operating point must have positive compressor power");
    }
    if (point.partLoadRatio <= 0.0 || point.partLoadRatio > 1.0) {
        return fail("active-mode PLR is outside (0, 1]");
    }
    if (point.cyclingRatio <= 0.0 || point.cyclingRatio > 1.0) {
        return fail("active-mode cycling ratio is outside (0, 1]");
    }

    Real64 const refrigerantPower = point.compressorPower * point.openMotorEfficiency;
    Real64 const moduleResidual = point.qCondenser - point.qEvaporator - refrigerantPower - point.falseLoad;
    if (!contractNear(moduleResidual, 0.0)) {
        return fail("module energy residual is " + std::to_string(moduleResidual) + " W");
    }

    if (point.routing.cooling < 0.0 || point.routing.heating < 0.0) {
        return fail("useful cooling and heating routing must be nonnegative");
    }

    switch (point.mode) {
    case CurrentMode::CoolingOnly:
        if (!contractNear(point.routing.cooling, point.qEvaporator) || !contractNear(point.routing.heating, 0.0) ||
            !contractNear(point.routing.source, point.qCondenser)) {
            return fail("cooling-only heat is not fully routed to chilled water and the Source loop");
        }
        break;
    case CurrentMode::HeatingOnly:
        if (!contractNear(point.routing.cooling, 0.0) || !contractNear(point.routing.heating, point.qCondenser) ||
            !contractNear(point.routing.source, -point.qEvaporator)) {
            return fail("heating-only heat is not fully routed from the Source loop to hot water");
        }
        break;
    case CurrentMode::HeatRecovery:
        if (!contractNear(point.routing.cooling, point.qEvaporator) || !contractNear(point.routing.heating, point.qCondenser) ||
            !contractNear(point.routing.source, 0.0)) {
            return fail("balanced heat recovery must route the complete evaporator and condenser loads with zero source transfer");
        }
        break;
    case CurrentMode::CoolingDominant:
        if (point.routing.heating <= 0.0 || point.routing.heating >= point.qCondenser || !contractNear(point.routing.cooling, point.qEvaporator) ||
            !contractNear(point.routing.source, point.qCondenser - point.routing.heating)) {
            return fail("cooling-dominant operation must recover part of condenser heat and reject the residual to source");
        }
        break;
    case CurrentMode::HeatingDominant:
        if (point.routing.cooling <= 0.0 || point.routing.cooling >= point.qEvaporator || !contractNear(point.routing.heating, point.qCondenser) ||
            !contractNear(point.routing.source, -(point.qEvaporator - point.routing.cooling))) {
            return fail("heating-dominant operation must preserve useful cooling and extract only the residual from source");
        }
        break;
    default:
        return fail("unsupported operating mode");
    }

    Real64 const systemResidual = point.routing.heating + point.routing.source - point.routing.cooling - refrigerantPower - point.falseLoad;
    if (!contractNear(systemResidual, 0.0)) {
        return fail("three-loop routing energy residual is " + std::to_string(systemResidual) + " W");
    }

    return ::testing::AssertionSuccess();
}

::testing::AssertionResult checkPartLoadContract(PartLoadContract const &point)
{
    if (point.availableCapacity <= 0.0) {
        return ::testing::AssertionFailure() << "available capacity must be positive";
    }
    if (point.minimumPartLoadRatio <= 0.0 || point.maximumPartLoadRatio < point.minimumPartLoadRatio) {
        return ::testing::AssertionFailure() << "invalid PLR bounds";
    }

    Real64 const requestedPLR = std::max(0.0, point.requestedLoad / point.availableCapacity);
    Real64 const expectedPLR = std::clamp(requestedPLR, point.minimumPartLoadRatio, point.maximumPartLoadRatio);
    Real64 const expectedCycling = requestedPLR < point.minimumPartLoadRatio ? requestedPLR / point.minimumPartLoadRatio : 1.0;
    Real64 const expectedDeliveredLoad = std::min(point.requestedLoad, point.availableCapacity * point.maximumPartLoadRatio);

    if (!contractNear(point.reportedPartLoadRatio, expectedPLR)) {
        return ::testing::AssertionFailure() << "reported PLR " << point.reportedPartLoadRatio << " does not equal final operating PLR "
                                             << expectedPLR;
    }
    if (!contractNear(point.cyclingRatio, expectedCycling)) {
        return ::testing::AssertionFailure() << "cycling ratio " << point.cyclingRatio << " does not equal " << expectedCycling;
    }
    if (!contractNear(point.deliveredLoad, expectedDeliveredLoad)) {
        return ::testing::AssertionFailure() << "delivered load " << point.deliveredLoad << " does not equal " << expectedDeliveredLoad;
    }
    if (!contractNear(point.curveEvaluationPartLoadRatio, point.reportedPartLoadRatio)) {
        return ::testing::AssertionFailure() << "EIRFPLR was evaluated at " << point.curveEvaluationPartLoadRatio << " instead of final reported PLR "
                                             << point.reportedPartLoadRatio;
    }
    return ::testing::AssertionSuccess();
}

::testing::AssertionResult checkSequentialFlowAllocation(FlowAllocationContract const &flow)
{
    if (flow.requestedFlows.size() != flow.moduleMaximumFlows.size() || flow.requestedFlows.size() != flow.allocatedFlows.size()) {
        return ::testing::AssertionFailure() << "flow vectors have different sizes";
    }
    if (flow.systemAvailableFlow < 0.0) {
        return ::testing::AssertionFailure() << "system available flow is negative";
    }

    Real64 remainingFlow = flow.systemAvailableFlow;
    for (std::size_t module = 0; module < flow.allocatedFlows.size(); ++module) {
        Real64 const expected = std::min({flow.requestedFlows[module], flow.moduleMaximumFlows[module], remainingFlow});
        if (!contractNear(flow.allocatedFlows[module], expected)) {
            return ::testing::AssertionFailure() << "module " << module + 1 << " allocation " << flow.allocatedFlows[module]
                                                 << " does not equal sequentially available flow " << expected;
        }
        remainingFlow -= expected;
    }

    Real64 const totalAllocated = std::accumulate(flow.allocatedFlows.begin(), flow.allocatedFlows.end(), 0.0);
    if (totalAllocated > flow.systemAvailableFlow && !contractNear(totalAllocated, flow.systemAvailableFlow)) {
        return ::testing::AssertionFailure() << "module flow sum " << totalAllocated << " exceeds system flow " << flow.systemAvailableFlow;
    }
    return ::testing::AssertionSuccess();
}

::testing::AssertionResult
checkLoopHeatTransfer(Real64 const reportedHeat, Real64 const massFlow, Real64 const specificHeat, Real64 const inletTemp, Real64 const outletTemp)
{
    if (reportedHeat < 0.0 || massFlow < 0.0 || specificHeat <= 0.0) {
        return ::testing::AssertionFailure() << "invalid loop heat-transfer input";
    }
    Real64 const nodeHeat = massFlow * specificHeat * std::abs(outletTemp - inletTemp);
    if (!contractNear(reportedHeat, nodeHeat)) {
        return ::testing::AssertionFailure() << "reported heat " << reportedHeat << " W does not match node heat " << nodeHeat << " W";
    }
    return ::testing::AssertionSuccess();
}

std::string makeChillerHeaterValidationInput(Real64 const capacityRatio = 0.75,
                                             Real64 const coolingOptimumPLR = 0.5,
                                             Real64 const heatingOptimumPLR = 0.5,
                                             Real64 const coolingMinimumPLR = 0.2)
{
    std::vector<std::string> const lines{
        "ChillerHeaterPerformance:Electric:EIR,",
        "  Validation Module,",
        "  10000,",
        "  5.0,",
        "  7.0,",
        "  30.0,",
        "  35.0,",
        "  " + std::to_string(capacityRatio) + ",",
        "  1.0,",
        "  7.0,",
        "  50.0,",
        "  30.0,",
        "  3.0,",
        "  variableFlow,",
        "  0.001,",
        "  0.001,",
        "  0.001,",
        "  0.8,",
        "  LeavingCondenser,",
        "  Cooling Reference Curve,",
        "  Cooling Reference Curve,",
        "  Cooling PLR Curve,",
        "  " + std::to_string(coolingOptimumPLR) + ",",
        "  LeavingCondenser,",
        "  Heating Reference Curve,",
        "  Heating Reference Curve,",
        "  Heating PLR Curve,",
        "  " + std::to_string(heatingOptimumPLR) + ",",
        "  1.0;",

        "Curve:Biquadratic,",
        "  Cooling Reference Curve,",
        "  0.0, 0.0, 0.0, 0.0285714285714286, 0.0, 0.0,",
        "  -100.0, 100.0, -100.0, 100.0;",

        "Curve:Biquadratic,",
        "  Heating Reference Curve,",
        "  0.0, 0.0, 0.0, 0.02, 0.0, 0.0,",
        "  -100.0, 100.0, -100.0, 100.0;",

        "Curve:Bicubic,",
        "  Cooling PLR Curve,",
        "  0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,",
        "  -100.0, 100.0,",
        "  " + std::to_string(coolingMinimumPLR) + ", 1.0;",

        "Curve:Quadratic,",
        "  Heating PLR Curve,",
        "  1.0, 0.0, 0.0,",
        "  0.3, 1.0;",
    };
    return std::accumulate(lines.begin(), lines.end(), std::string(), [](std::string result, std::string const &line) {
        result += line;
        result += '\n';
        return result;
    });
}

nlohmann::json makeChillerHeaterNativeJSON(bool const includeOptionalFields = true, bool const autosize = false)
{
    nlohmann::json performance = {
        {"reference_cooling_mode_evaporator_capacity", autosize ? nlohmann::json("Autosize") : nlohmann::json(10000.0)},
        {"reference_cooling_mode_cop", 5.0},
        {"cooling_mode_cooling_capacity_function_of_temperature_curve_name", "Reference Temperature Curve"},
        {"cooling_mode_electric_input_to_cooling_output_ratio_function_of_temperature_curve_name", "Reference Temperature Curve"},
        {"cooling_mode_electric_input_to_cooling_output_ratio_function_of_part_load_ratio_curve_name", "Reference PLR Curve"},
        {"heating_mode_cooling_capacity_function_of_temperature_curve_name", "Reference Temperature Curve"},
        {"heating_mode_electric_input_to_cooling_output_ratio_function_of_temperature_curve_name", "Reference Temperature Curve"},
        {"heating_mode_electric_input_to_cooling_output_ratio_function_of_part_load_ratio_curve_name", "Reference PLR Curve"},
    };

    if (includeOptionalFields) {
        performance.update({
            {"reference_cooling_mode_leaving_chilled_water_temperature", 7.0},
            {"reference_cooling_mode_entering_condenser_fluid_temperature", 30.0},
            {"reference_cooling_mode_leaving_condenser_water_temperature", 35.0},
            {"reference_heating_mode_cooling_capacity_ratio", 0.75},
            {"reference_heating_mode_cooling_power_input_ratio", 1.0},
            {"reference_heating_mode_leaving_chilled_water_temperature", 7.0},
            {"reference_heating_mode_leaving_condenser_water_temperature", 50.0},
            {"reference_heating_mode_entering_condenser_fluid_temperature", 30.0},
            {"heating_mode_entering_chilled_water_temperature_low_limit", 3.0},
            {"chilled_water_flow_mode_type", "VariableFlow"},
            {"design_chilled_water_flow_rate", autosize ? nlohmann::json("Autosize") : nlohmann::json(0.001)},
            {"design_condenser_water_flow_rate", autosize ? nlohmann::json("Autosize") : nlohmann::json(0.001)},
            {"design_hot_water_flow_rate", 0.001},
            {"compressor_motor_efficiency", 0.8},
            {"cooling_mode_temperature_curve_condenser_water_independent_variable", "LeavingCondenser"},
            {"cooling_mode_cooling_capacity_optimum_part_load_ratio", 0.5},
            {"heating_mode_temperature_curve_condenser_water_independent_variable", "EnteringCondenser"},
            {"heating_mode_cooling_capacity_optimum_part_load_ratio", 0.5},
            {"sizing_factor", 1.2},
        });
    }

    nlohmann::json const temperatureCurve = {
        {"coefficient1_constant", 1.0},
        {"coefficient2_x", 0.0},
        {"coefficient3_x_2", 0.0},
        {"coefficient4_y", 0.0},
        {"coefficient5_y_2", 0.0},
        {"coefficient6_x_y", 0.0},
        {"minimum_value_of_x", -100.0},
        {"maximum_value_of_x", 100.0},
        {"minimum_value_of_y", -100.0},
        {"maximum_value_of_y", 100.0},
    };
    nlohmann::json const partLoadCurve = {
        {"coefficient1_constant", 1.0},
        {"coefficient2_x", 0.0},
        {"coefficient3_x_2", 0.0},
        {"minimum_value_of_x", 0.0},
        {"maximum_value_of_x", 1.0},
    };

    return {
        {"ChillerHeaterPerformance:Electric:EIR", {{"Native Mixed Case Module", performance}}},
        {"Curve:Biquadratic", {{"Reference Temperature Curve", temperatureCurve}}},
        {"Curve:Quadratic", {{"Reference PLR Curve", partLoadCurve}}},
    };
}

struct FixedModuleGroupFieldKeys
{
    std::string performanceObjectType;
    std::string performanceName;
    std::string controlScheduleName;
    std::string numberOfModules;
};

FixedModuleGroupFieldKeys fixedModuleGroupFieldKeys(int const groupNumber)
{
    std::string const suffix = std::to_string(groupNumber);
    FixedModuleGroupFieldKeys keys{
        "chiller_heater_modules_performance_component_object_type_" + suffix,
        "chiller_heater_modules_performance_component_name_" + suffix,
        "chiller_heater_modules_control_schedule_name_" + suffix,
        "number_of_chiller_heater_modules_" + suffix,
    };
    if (groupNumber == 3) {
        keys.performanceObjectType = "chiller_heater_performance_component_object_type_3";
        keys.performanceName = "chiller_heater_performance_component_name_3";
    } else if (groupNumber == 5) {
        keys.performanceName = "chiller_heater_models_performance_component_name_5";
    } else if (groupNumber == 11) {
        keys.controlScheduleName = "chiller_heater_module_control_schedule_name_11";
    } else if (groupNumber == 18) {
        keys.controlScheduleName = "chiller_heater_modules_control_control_schedule_name_18";
    }
    return keys;
}

nlohmann::json makeNativeSystemJSON(bool const useAllGroups)
{
    auto epJSON = makeChillerHeaterNativeJSON();
    epJSON["ScheduleTypeLimits"]["Fraction"] = {
        {"lower_limit_value", 0.0},
        {"upper_limit_value", 1.0},
        {"numeric_type", "Continuous"},
        {"unit_type", "Dimensionless"},
    };
    epJSON["Schedule:Constant"] = {
        {"Ancillary Schedule", {{"schedule_type_limits_name", "Fraction"}, {"hourly_value", 0.5}}},
        {"Module Schedule", {{"schedule_type_limits_name", "Fraction"}, {"hourly_value", 1.0}}},
        {"Module Schedule 1", {{"schedule_type_limits_name", "Fraction"}, {"hourly_value", 1.0}}},
        {"Module Schedule 3", {{"schedule_type_limits_name", "Fraction"}, {"hourly_value", 1.0}}},
        {"Module Schedule 4", {{"schedule_type_limits_name", "Fraction"}, {"hourly_value", 1.0}}},
        {"Module Schedule 6", {{"schedule_type_limits_name", "Fraction"}, {"hourly_value", 1.0}}},
    };

    nlohmann::json system = {
        {"cooling_loop_inlet_node_name", useAllGroups ? "Cooling Inlet" : "Sparse Cooling Inlet"},
        {"cooling_loop_outlet_node_name", useAllGroups ? "Cooling Outlet" : "Sparse Cooling Outlet"},
        {"source_loop_inlet_node_name", useAllGroups ? "Source Inlet" : "Sparse Source Inlet"},
        {"source_loop_outlet_node_name", useAllGroups ? "Source Outlet" : "Sparse Source Outlet"},
        {"heating_loop_inlet_node_name", useAllGroups ? "Heating Inlet" : "Sparse Heating Inlet"},
        {"heating_loop_outlet_node_name", useAllGroups ? "Heating Outlet" : "Sparse Heating Outlet"},
        {"ancillary_power", 25.0},
        {"ancillary_operation_schedule_name", "Ancillary Schedule"},
    };

    auto addGroup = [&system](int const groupNumber, int const count, std::string_view const scheduleName) {
        auto const keys = fixedModuleGroupFieldKeys(groupNumber);
        system[keys.performanceObjectType] = "ChillerHeaterPerformance:Electric:EIR";
        system[keys.performanceName] = "Native Mixed Case Module";
        system[keys.numberOfModules] = count;
        if (!scheduleName.empty()) {
            system[keys.controlScheduleName] = scheduleName;
        }
    };

    if (useAllGroups) {
        for (int group = 1; group <= 20; ++group) {
            addGroup(group, 1, "Module Schedule");
        }
        epJSON["CentralHeatPumpSystem"]["Native Many Groups Wrapper"] = std::move(system);
    } else {
        addGroup(1, 1, "Module Schedule 1");
        addGroup(2, 2, "");
        addGroup(3, 1, "Module Schedule 3");
        addGroup(4, 1, "Module Schedule 4");
        addGroup(5, 1, "Missing Module Schedule");
        addGroup(6, 2, "Module Schedule 6");
        epJSON["CentralHeatPumpSystem"]["Native Sparse Wrapper"] = std::move(system);
    }
    return epJSON;
}

std::string makeAllModuleGroupsIDF()
{
    std::string idf = R"IDF(
CentralHeatPumpSystem,
  Native Many Groups Wrapper,
  Cooling Inlet,
  Cooling Outlet,
  Source Inlet,
  Source Outlet,
  Heating Inlet,
  Heating Outlet,
  25.0,
  Ancillary Schedule,
)IDF";
    for (int group = 1; group <= 20; ++group) {
        idf += "  ChillerHeaterPerformance:Electric:EIR,\n";
        idf += "  Native Mixed Case Module,\n";
        idf += "  Module Schedule,\n";
        idf += group == 20 ? "  1;\n" : "  1,\n";
    }
    idf += R"IDF(
ScheduleTypeLimits,
  Fraction,
  0.0,
  1.0,
  Continuous,
  Dimensionless;
Schedule:Constant,
  Ancillary Schedule,
  Fraction,
  0.5;
Schedule:Constant,
  Module Schedule,
  Fraction,
  1.0;
ChillerHeaterPerformance:Electric:EIR,
  Native Mixed Case Module,
  10000,
  5.0,
  7.0,
  30.0,
  35.0,
  0.75,
  1.0,
  7.0,
  50.0,
  30.0,
  3.0,
  variableFlow,
  0.001,
  0.001,
  0.001,
  0.8,
  LeavingCondenser,
  Reference Temperature Curve,
  Reference Temperature Curve,
  Reference PLR Curve,
  0.5,
  EnteringCondenser,
  Reference Temperature Curve,
  Reference Temperature Curve,
  Reference PLR Curve,
  0.5,
  1.2;
Curve:Biquadratic,
  Reference Temperature Curve,
  1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
  -100.0, 100.0, -100.0, 100.0;
Curve:Quadratic,
  Reference PLR Curve,
  1.0, 0.0, 0.0,
  0.0, 1.0;
)IDF";
    return idf;
}
} // namespace

TEST_F(EnergyPlusFixture, ChillerHeater_Autosize)
{
    state->init_state(*state);
    // Allocate one system with One module (=distinct ChillerHeaterPerformance:Electric:EIR)
    // but with a number of identical number module of 2 in CentralHeatPumpSystem
    state->dataPlantCentralHeatPumpSystem->systems.resize(1);

    state->dataPlantCentralHeatPumpSystem->systems[0].modules.resize(2);

    state->dataPlantCentralHeatPumpSystem->performanceDefinitions.resize(1);
    state->dataPlantCentralHeatPumpSystem->performanceDefinitions[0].constantFlow = false;
    state->dataPlantCentralHeatPumpSystem->performanceDefinitions[0].variableFlow = true;

    state->dataPlantCentralHeatPumpSystem->performanceDefinitions[0].sizingFactor = 1.2;

    state->dataPlantCentralHeatPumpSystem->performanceDefinitions[0].referenceCoolingCapacity = DataSizing::AutoSize;
    state->dataPlantCentralHeatPumpSystem->performanceDefinitions[0].referenceCoolingCapacityWasAutoSized = true;

    state->dataPlantCentralHeatPumpSystem->performanceDefinitions[0].designEvaporatorVolFlowRate = DataSizing::AutoSize;
    state->dataPlantCentralHeatPumpSystem->performanceDefinitions[0].designEvaporatorVolFlowRateWasAutoSized = true;

    state->dataPlantCentralHeatPumpSystem->performanceDefinitions[0].designCondenserVolFlowRate = DataSizing::AutoSize;
    state->dataPlantCentralHeatPumpSystem->performanceDefinitions[0].designCondenserVolFlowRateWasAutoSized = true;

    // Needed for calcs
    state->dataPlantCentralHeatPumpSystem->performanceDefinitions[0].referenceCoolingCOP = 1.5;
    state->dataPlantCentralHeatPumpSystem->performanceDefinitions[0].compressorMotorEfficiency = 0.98;
    state->dataPlantCentralHeatPumpSystem->performanceDefinitions[0].coolingReferenceCondenserInletTemp = 29.4;
    state->dataPlantCentralHeatPumpSystem->performanceDefinitions[0].heatingToCoolingCapacityRatio = 0.74;
    state->dataPlantCentralHeatPumpSystem->performanceDefinitions[0].heatingToCoolingPowerRatio = 1.38;

    // Both modules share one retained immutable performance definition.
    auto const &performance = state->dataPlantCentralHeatPumpSystem->performanceDefinitions[0];
    auto *availabilitySchedule = Sched::GetScheduleAlwaysOn(*state);
    state->dataPlantCentralHeatPumpSystem->systems[0].modules[0].initialize(performance, availabilitySchedule);
    state->dataPlantCentralHeatPumpSystem->systems[0].modules[1].initialize(performance, availabilitySchedule);
    auto &module1 = state->dataPlantCentralHeatPumpSystem->systems[0].modules[0];
    auto &module2 = state->dataPlantCentralHeatPumpSystem->systems[0].modules[1];
    EXPECT_EQ(&performance, module1.performance);
    EXPECT_EQ(&performance, module2.performance);
    EXPECT_EQ(availabilitySchedule, module1.availabilitySchedule);
    EXPECT_EQ(availabilitySchedule, module2.availabilitySchedule);
    module1.result.qEvaporator = 100.0;
    EXPECT_DOUBLE_EQ(0.0, module2.result.qEvaporator);

    state->dataPlnt->PlantLoop.allocate(2);
    state->dataSize->PlantSizData.allocate(2);

    // Chilled Water Loop
    int PltSizNum = 1;
    state->dataPlnt->PlantLoop(PltSizNum).PlantSizNum = 1;
    state->dataPlnt->PlantLoop(PltSizNum).FluidName = "WATER";
    state->dataPlnt->PlantLoop(PltSizNum).glycol = Fluid::GetWater(*state);
    state->dataSize->PlantSizData(PltSizNum).DesVolFlowRate = 1.0;
    state->dataSize->PlantSizData(PltSizNum).DeltaT = 10.0;
    state->dataSize->PlantSizData(PltSizNum).LoopType = DataSizing::TypeOfPlantLoop::Cooling;
    // Assign to the system
    state->dataPlantCentralHeatPumpSystem->systems[0].coolingPlantLoc.loopNum = PltSizNum;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataPlantCentralHeatPumpSystem->systems[0].coolingPlantLoc);

    // Condenser Loop
    int PltSizCondNum = 2;
    state->dataPlnt->PlantLoop(PltSizCondNum).PlantSizNum = PltSizCondNum;
    state->dataPlnt->PlantLoop(PltSizCondNum).FluidName = "WATER";
    state->dataPlnt->PlantLoop(PltSizCondNum).glycol = Fluid::GetWater(*state);
    state->dataSize->PlantSizData(PltSizCondNum).DeltaT = 5.6;
    state->dataSize->PlantSizData(PltSizCondNum).DesVolFlowRate = 1.0;
    state->dataSize->PlantSizData(PltSizCondNum).LoopType = DataSizing::TypeOfPlantLoop::Condenser;
    // Assign to the system
    state->dataPlantCentralHeatPumpSystem->systems[0].sourcePlantLoc.loopNum = PltSizCondNum;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataPlantCentralHeatPumpSystem->systems[0].sourcePlantLoc);
    state->dataPlantCentralHeatPumpSystem->systems[0].coolingInletNodeNum = 1;
    state->dataPlantCentralHeatPumpSystem->systems[0].heatingInletNodeNum = 2;
    state->dataPlantCentralHeatPumpSystem->systems[0].sourceInletNodeNum = 3;

    // Calculate expected values
    Real64 rho_evap = state->dataPlnt->PlantLoop(PltSizNum).glycol->getDensity(*state, Constant::CWInitConvTemp, "ChillerHeater_Autosize_TEST");

    Real64 Cp_evap = state->dataPlnt->PlantLoop(PltSizNum).glycol->getSpecificHeat(*state, Constant::CWInitConvTemp, "ChillerHeater_Autosize_TEST");

    Real64 rho_cond =
        state->dataPlnt->PlantLoop(PltSizCondNum)
            .glycol->getDensity(*state,
                                state->dataPlantCentralHeatPumpSystem->systems[0].modules[0].performanceData().coolingReferenceCondenserInletTemp,
                                "ChillerHeater_Autosize_TEST");

    Real64 Cp_cond = state->dataPlnt->PlantLoop(PltSizCondNum)
                         .glycol->getSpecificHeat(
                             *state,
                             state->dataPlantCentralHeatPumpSystem->systems[0].modules[0].performanceData().coolingReferenceCondenserInletTemp,
                             "ChillerHeater_Autosize_TEST");

    // Note: Each individual chiller heater module is sized to be capable of supporting the total load on the system

    // Flow is multiplied by the sizingFactor
    Real64 expectedEvaporatorVolFlowRate = state->dataSize->PlantSizData(PltSizNum).DesVolFlowRate *
                                           state->dataPlantCentralHeatPumpSystem->systems[0].modules[0].performanceData().sizingFactor;

    Real64 expectedReferenceCoolingCapacity = rho_evap * Cp_evap * expectedEvaporatorVolFlowRate * state->dataSize->PlantSizData(PltSizNum).DeltaT;

    Real64 expectedCondenserVolFlowRate =
        expectedReferenceCoolingCapacity *
        (1.0 + (1.0 / state->dataPlantCentralHeatPumpSystem->systems[0].modules[0].performanceData().referenceCoolingCOP) *
                   state->dataPlantCentralHeatPumpSystem->systems[0].modules[0].performanceData().compressorMotorEfficiency) /
        (rho_cond * Cp_cond * state->dataSize->PlantSizData(PltSizCondNum).DeltaT);

    // now call sizing routine
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    state->dataPlantCentralHeatPumpSystem->systems[0].size(*state);

    // Autosized values are system-specific module sizing state; the shared definition remains unchanged.
    EXPECT_DOUBLE_EQ(expectedEvaporatorVolFlowRate, state->dataPlantCentralHeatPumpSystem->systems[0].modules[0].sizing.designEvaporatorVolFlowRate);
    EXPECT_DOUBLE_EQ(expectedReferenceCoolingCapacity, state->dataPlantCentralHeatPumpSystem->systems[0].modules[0].sizing.referenceCoolingCapacity);

    EXPECT_DOUBLE_EQ(expectedCondenserVolFlowRate, state->dataPlantCentralHeatPumpSystem->systems[0].modules[0].sizing.designCondenserVolFlowRate);
    EXPECT_DOUBLE_EQ(expectedCondenserVolFlowRate, state->dataPlantCentralHeatPumpSystem->systems[0].modules[1].sizing.designCondenserVolFlowRate);

    // Ensure that stuff that other quantities that depends on referenceCoolingCapacity are also initialized properly
    // Heating Cap
    Real64 expectedReferenceHeatingCapacity =
        expectedReferenceCoolingCapacity *
        state->dataPlantCentralHeatPumpSystem->systems[0].modules[0].performanceData().heatingToCoolingCapacityRatio;
    EXPECT_DOUBLE_EQ(expectedReferenceHeatingCapacity, state->dataPlantCentralHeatPumpSystem->systems[0].modules[0].sizing.referenceHeatingCapacity);

    // Heating Power: Calc cooling Power = Cap / COP, and multiply by ratio
    Real64 expectedReferenceHeatingPower =
        (expectedReferenceCoolingCapacity / state->dataPlantCentralHeatPumpSystem->systems[0].modules[0].performanceData().referenceCoolingCOP) *
        state->dataPlantCentralHeatPumpSystem->systems[0].modules[0].performanceData().heatingToCoolingPowerRatio;
    EXPECT_DOUBLE_EQ(expectedReferenceHeatingPower, state->dataPlantCentralHeatPumpSystem->systems[0].modules[0].sizing.referenceHeatingPower);

    // Heating COP = Heating Cap / Heating Power
    Real64 expectedReferenceHeatingCOP = expectedReferenceHeatingCapacity / expectedReferenceHeatingPower;
    EXPECT_DOUBLE_EQ(expectedReferenceHeatingCOP, state->dataPlantCentralHeatPumpSystem->systems[0].modules[0].sizing.referenceHeatingCOP);

    ASSERT_EQ(3u, state->dataSize->CompDesWaterFlow.size());
    EXPECT_DOUBLE_EQ(2.0 * expectedEvaporatorVolFlowRate, state->dataSize->CompDesWaterFlow(1).DesVolFlowRate);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->CompDesWaterFlow(2).DesVolFlowRate);
    EXPECT_DOUBLE_EQ(2.0 * std::max(expectedEvaporatorVolFlowRate, expectedCondenserVolFlowRate),
                     state->dataSize->CompDesWaterFlow(3).DesVolFlowRate);
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_DesignCapacityReportingUsesConnectionsAndStageability)
{
    state->init_state(*state);

    PlantCentralHeatPumpSystem::CentralHeatPumpSystem system;
    system.coolingPlantLoc.loopNum = 1;
    system.heatingPlantLoc.loopNum = 2;
    system.sourcePlantLoc.loopNum = 3;
    system.modules.resize(2);

    PlantCentralHeatPumpSystem::PerformanceData performance1;
    PlantCentralHeatPumpSystem::PerformanceData performance2;
    system.modules[0].initialize(performance1, nullptr);
    system.modules[1].initialize(performance2, nullptr);

    auto &module1 = system.modules[0];
    module1.sizing.referenceCoolingCapacity = 10000.0;
    performance1.referenceCoolingCOP = 5.0;
    performance1.coolingMinimumPartLoadRatio = 0.20;
    performance1.coolingMaximumPartLoadRatio = 1.00;
    performance1.coolingOptimumPartLoadRatio = 0.80;
    module1.sizing.referenceHeatingCapacity = 8000.0;
    module1.sizing.referenceHeatingPower = 2000.0;
    performance1.heatingMinimumPartLoadRatio = 0.25;
    performance1.heatingMaximumPartLoadRatio = 1.10;
    performance1.heatingOptimumPartLoadRatio = 0.75;
    performance1.compressorMotorEfficiency = 0.80;

    auto &module2 = system.modules[1];
    module2.sizing.referenceCoolingCapacity = 6000.0;
    performance2.referenceCoolingCOP = 3.0;
    performance2.coolingMinimumPartLoadRatio = 0.10;
    performance2.coolingMaximumPartLoadRatio = 1.20;
    performance2.coolingOptimumPartLoadRatio = 0.70;
    module2.sizing.referenceHeatingCapacity = 12000.0;
    module2.sizing.referenceHeatingPower = 3000.0;
    performance2.heatingMinimumPartLoadRatio = 0.30;
    performance2.heatingMaximumPartLoadRatio = 0.90;
    performance2.heatingOptimumPartLoadRatio = 0.60;
    performance2.compressorMotorEfficiency = 0.50;

    Real64 maximumLoad = 0.0;
    Real64 minimumLoad = 0.0;
    Real64 optimumLoad = 0.0;
    PlantLocation calledFromLocation;

    calledFromLocation.loopNum = system.coolingPlantLoc.loopNum;
    system.getDesignCapacities(*state, calledFromLocation, maximumLoad, minimumLoad, optimumLoad);
    EXPECT_DOUBLE_EQ(17200.0, maximumLoad);
    EXPECT_DOUBLE_EQ(600.0, minimumLoad);
    EXPECT_DOUBLE_EQ(12200.0, optimumLoad);

    calledFromLocation.loopNum = system.heatingPlantLoc.loopNum;
    system.getDesignCapacities(*state, calledFromLocation, maximumLoad, minimumLoad, optimumLoad);
    EXPECT_DOUBLE_EQ(22710.0, maximumLoad);
    EXPECT_DOUBLE_EQ(2400.0, minimumLoad);
    EXPECT_DOUBLE_EQ(15300.0, optimumLoad);

    calledFromLocation.loopNum = system.sourcePlantLoc.loopNum;
    system.getDesignCapacities(*state, calledFromLocation, maximumLoad, minimumLoad, optimumLoad);
    EXPECT_DOUBLE_EQ(20000.0, maximumLoad);
    EXPECT_DOUBLE_EQ(700.0, minimumLoad);
    EXPECT_DOUBLE_EQ(14180.0, optimumLoad);

    performance2.heatingMaximumPartLoadRatio = 1.20;
    performance2.heatingOptimumPartLoadRatio = 0.80;
    system.getDesignCapacities(*state, calledFromLocation, maximumLoad, minimumLoad, optimumLoad);
    EXPECT_DOUBLE_EQ(23200.0, maximumLoad);
    EXPECT_DOUBLE_EQ(700.0, minimumLoad);
    EXPECT_DOUBLE_EQ(15600.0, optimumLoad);

    calledFromLocation.loopNum = 4;
    system.getDesignCapacities(*state, calledFromLocation, maximumLoad, minimumLoad, optimumLoad);
    EXPECT_DOUBLE_EQ(0.0, maximumLoad);
    EXPECT_DOUBLE_EQ(0.0, minimumLoad);
    EXPECT_DOUBLE_EQ(0.0, optimumLoad);
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_SourceOnlySizingUsesSourcePlantSizingData)
{
    state->init_state(*state);

    state->dataPlnt->PlantLoop.allocate(3);
    state->dataSize->PlantSizData.allocate(1);
    auto *water = Fluid::GetWater(*state);
    ASSERT_NE(nullptr, water);

    state->dataPlnt->PlantLoop(1).PlantSizNum = 0;
    state->dataPlnt->PlantLoop(1).glycol = water;
    state->dataPlnt->PlantLoop(2).PlantSizNum = 1;
    state->dataPlnt->PlantLoop(2).glycol = water;
    state->dataPlnt->PlantLoop(3).PlantSizNum = 0;
    state->dataPlnt->PlantLoop(3).glycol = water;
    state->dataSize->PlantSizData(1).LoopType = DataSizing::TypeOfPlantLoop::Condenser;
    state->dataSize->PlantSizData(1).DesVolFlowRate = 0.002;
    state->dataSize->PlantSizData(1).DeltaT = 5.0;

    PlantCentralHeatPumpSystem::CentralHeatPumpSystem system;
    system.coolingPlantLoc.loopNum = 1;
    system.sourcePlantLoc.loopNum = 2;
    system.heatingPlantLoc.loopNum = 3;
    PlantUtilities::SetPlantLocationLinks(*state, system.coolingPlantLoc);
    PlantUtilities::SetPlantLocationLinks(*state, system.sourcePlantLoc);
    PlantUtilities::SetPlantLocationLinks(*state, system.heatingPlantLoc);
    system.coolingInletNodeNum = 11;
    system.heatingInletNodeNum = 12;
    system.sourceInletNodeNum = 13;
    system.modules.resize(1);

    PlantCentralHeatPumpSystem::PerformanceData performance;
    system.modules[0].initialize(performance, nullptr);
    auto &module = system.modules[0];
    performance.Name = "SOURCE SIZING ONLY";
    performance.sizingFactor = 1.0;
    module.sizing.designEvaporatorVolFlowRate = 0.001;
    module.sizing.referenceCoolingCapacity = 10000.0;
    performance.referenceCoolingCOP = 4.0;
    performance.compressorMotorEfficiency = 0.80;
    performance.coolingReferenceCondenserInletTemp = 30.0;
    module.sizing.designCondenserVolFlowRate = DataSizing::AutoSize;
    performance.designCondenserVolFlowRateWasAutoSized = true;
    performance.designHeatingVolFlowRate = 0.0007;

    Real64 const sourceDensity =
        water->getDensity(*state, performance.coolingReferenceCondenserInletTemp, "PlantCentralHeatPumpSystem source sizing test");
    Real64 const sourceSpecificHeat =
        water->getSpecificHeat(*state, performance.coolingReferenceCondenserInletTemp, "PlantCentralHeatPumpSystem source sizing test");
    Real64 const expectedSourceCondenserFlow = module.sizing.referenceCoolingCapacity *
                                               (1.0 + performance.compressorMotorEfficiency / performance.referenceCoolingCOP) /
                                               (state->dataSize->PlantSizData(1).DeltaT * sourceSpecificHeat * sourceDensity);

    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    system.size(*state);

    EXPECT_NEAR(expectedSourceCondenserFlow, module.sizing.designCondenserVolFlowRate, 1.0e-12);
    EXPECT_DOUBLE_EQ(module.sizing.designEvaporatorVolFlowRate, module.sizing.temporaryEvaporatorVolFlowRate);
    EXPECT_NEAR(expectedSourceCondenserFlow, module.sizing.temporaryCondenserVolFlowRate, 1.0e-12);
    ASSERT_EQ(3u, state->dataSize->CompDesWaterFlow.size());
    EXPECT_DOUBLE_EQ(module.sizing.designEvaporatorVolFlowRate, state->dataSize->CompDesWaterFlow(1).DesVolFlowRate);
    EXPECT_DOUBLE_EQ(performance.designHeatingVolFlowRate, state->dataSize->CompDesWaterFlow(2).DesVolFlowRate);
    EXPECT_NEAR(std::max(module.sizing.designEvaporatorVolFlowRate, expectedSourceCondenserFlow),
                state->dataSize->CompDesWaterFlow(3).DesVolFlowRate,
                1.0e-12);
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_HardSizedWarningsRetainCalculatedDesignValues)
{
    state->init_state(*state);
    OutputReportPredefined::SetPredefinedTables(*state);

    state->dataPlnt->PlantLoop.allocate(3);
    state->dataSize->PlantSizData.allocate(2);
    auto *water = Fluid::GetWater(*state);
    ASSERT_NE(nullptr, water);
    for (int loopNum = 1; loopNum <= 3; ++loopNum) {
        state->dataPlnt->PlantLoop(loopNum).glycol = water;
    }
    state->dataPlnt->PlantLoop(1).PlantSizNum = 1;
    state->dataPlnt->PlantLoop(2).PlantSizNum = 2;
    state->dataSize->PlantSizData(1).LoopType = DataSizing::TypeOfPlantLoop::Cooling;
    state->dataSize->PlantSizData(1).DesVolFlowRate = 0.010;
    state->dataSize->PlantSizData(1).DeltaT = 6.0;
    state->dataSize->PlantSizData(2).LoopType = DataSizing::TypeOfPlantLoop::Condenser;
    state->dataSize->PlantSizData(2).DesVolFlowRate = 0.020;
    state->dataSize->PlantSizData(2).DeltaT = 5.0;

    PlantCentralHeatPumpSystem::CentralHeatPumpSystem system;
    system.coolingPlantLoc.loopNum = 1;
    system.sourcePlantLoc.loopNum = 2;
    system.heatingPlantLoc.loopNum = 3;
    PlantUtilities::SetPlantLocationLinks(*state, system.coolingPlantLoc);
    PlantUtilities::SetPlantLocationLinks(*state, system.sourcePlantLoc);
    PlantUtilities::SetPlantLocationLinks(*state, system.heatingPlantLoc);
    system.coolingInletNodeNum = 21;
    system.heatingInletNodeNum = 22;
    system.sourceInletNodeNum = 23;
    system.modules.resize(1);

    PlantCentralHeatPumpSystem::PerformanceData performance;
    system.modules[0].initialize(performance, nullptr);
    auto &module = system.modules[0];
    performance.Name = "HARD SIZED MODULE";
    performance.sizingFactor = 1.0;
    module.sizing.designEvaporatorVolFlowRate = 0.001;
    module.sizing.referenceCoolingCapacity = 1000.0;
    performance.referenceCoolingCOP = 5.0;
    performance.compressorMotorEfficiency = 1.0;
    performance.coolingReferenceCondenserInletTemp = 30.0;
    module.sizing.designCondenserVolFlowRate = 0.001;
    performance.designHeatingVolFlowRate = 0.002;

    state->dataGlobal->DisplayExtraWarnings = true;
    state->dataSize->AutoVsHardSizingThreshold = 0.01;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    state->dataPlnt->PlantFinalSizesOkayToReport = true;
    system.size(*state);

    EXPECT_DOUBLE_EQ(0.001, module.sizing.designEvaporatorVolFlowRate);
    EXPECT_DOUBLE_EQ(1000.0, module.sizing.referenceCoolingCapacity);
    EXPECT_DOUBLE_EQ(0.001, module.sizing.designCondenserVolFlowRate);
    EXPECT_DOUBLE_EQ(0.001, module.sizing.temporaryEvaporatorVolFlowRate);
    EXPECT_DOUBLE_EQ(0.001, module.sizing.temporaryCondenserVolFlowRate);
    EXPECT_TRUE(compare_err_stream_substring("User-Specified Reference Chilled Water Flow Rate", false));
    EXPECT_TRUE(compare_err_stream_substring("User-Specified Reference Capacity", false));
    EXPECT_TRUE(compare_err_stream_substring("User-Specified Reference Condenser Water Flow Rate", true));
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_Control_Schedule_fix)
{
    std::string const idf_objects = delimited_string({

        "Schedule:Compact,",
        "Always1, !-Name",
        "On/Off, !-Schedule Type Limits Name",
        "Through: 12/31, !-Field 1",
        "For: AllDays, !-Field 2",
        "Until: 24:00, 1; !-Field 3 ",

        "CentralHeatPumpSystem,",
        "ChW_Loop HeatPump1, !-Name",
        "ChW_Loop HeatPump1 ChW Inlet, !-Cooling Loop Inlet Node Name",
        "ChW_Loop HeatPump1 ChW Outlet, !-Cooling Loop Outlet Node Name",
        "ChW_Loop HeatPump1 Cnd Inlet, !-Source Loop Inlet Node Name",
        "ChW_Loop HeatPump1 Cnd Outlet, !-Source Loop Outlet Node Name",
        "ChW_Loop HeatPump1 HHW Inlet, !-Heating Loop Inlet Node Name",
        "ChW_Loop HeatPump1 HHW Outlet, !-Heating Loop Outlet Node Name",
        "460,  !-Ancillary Power{W}",
        ",  !-Ancillary Operation Schedule Name",
        "ChillerHeaterPerformance:Electric:EIR, !-Chiller Heater Modules Performance Component Object Type 1",
        "ChW_Loop HeatPump1 Module, !-Chiller Heater Modules Performance Component Name 1",
        "Always_1_typo, !-Chiller Heater Modules Control Schedule Name 1",
        "2; !-Number of Chiller Heater Modules 1",

        "ChillerHeaterPerformance:Electric:EIR,",
        "    ChW_Loop HeatPump1 Module,  !- Name",
        "    autosize,                !- Reference Cooling Mode Evaporator Capacity {W}",
        "    1.5,                     !- Reference Cooling Mode COP {W/W}",
        "    6.67,                    !- Reference Cooling Mode Leaving Chilled Water Temperature {C}",
        "    29.4,                    !- Reference Cooling Mode Entering Condenser Fluid Temperature {C}",
        "    35.0,                    !- Reference Cooling Mode Leaving Condenser Water Temperature {C}",
        "    0.74,                    !- Reference Heating Mode Cooling Capacity Ratio",
        "    0.925,                   !- Reference Heating Mode Cooling Power Input Ratio",
        "    6.67,                    !- Reference Heating Mode Leaving Chilled Water Temperature {C}",
        "    60,                      !- Reference Heating Mode Leaving Condenser Water Temperature {C}",
        "    29.4,                    !- Reference Heating Mode Entering Condenser Fluid Temperature {C}",
        "    5,                       !- Heating Mode Entering Chilled Water Temperature Low Limit {C}",
        "    variableFlow,            !- Chilled Water Flow Mode Type",
        "    autosize,                !- Design Chilled Water Flow Rate {m3/s}",
        "    autosize,                !- Design Condenser Water Flow Rate {m3/s}",
        "    0.01684,                 !- Design Hot water Flow Rate {m3/s}",
        "    1,                       !- Compressor Motor Efficiency",
        "    EnteringCondenser,       !- Cooling Mode Temperature Curve Condenser Water Independent Variable",
        "    ChillerHeaterClgCapFT,   !- Cooling Mode Cooling Capacity Function of Temperature Curve Name",
        "    ChillerHeaterClgEIRFT,   !- Cooling Mode Electric Input to Cooling Output Ratio Function of Temperature Curve Name",
        "    ChillerHeaterClgEIRFPLR, !- Cooling Mode Electric Input to Cooling Output Ratio Function of Part Load Ratio Curve Name",
        "    1,                       !- Cooling Mode Cooling Capacity Optimum Part Load Ratio",
        "    LeavingCondenser,        !- Heating Mode Temperature Curve Condenser Water Independent Variable",
        "    ChillerHeaterHtgCapFT,   !- Heating Mode Cooling Capacity Function of Temperature Curve Name",
        "    ChillerHeaterHtgEIRFT,   !- Heating Mode Electric Input to Cooling Output Ratio Function of Temperature Curve Name",
        "    ChillerHeaterHtgEIRFPLR, !- Heating Mode Electric Input to Cooling Output Ratio Function of Part Load Ratio Curve Name",
        "    1,                       !- Heating Mode Cooling Capacity Optimum Part Load Ratio",
        "    1;                       !- Sizing Factor",

        "Curve:Biquadratic,",
        "    ChillerHeaterClgCapFT,   !- Name",
        "    0.950829,                !- Coefficient1 Constant",
        "    3.419327E-02,            !- Coefficient2 x",
        "    2.66642E-04,             !- Coefficient3 x**2",
        "    -1.733397E-03,           !- Coefficient4 y",
        "    -1.762417E-04,           !- Coefficient5 y**2",
        "    -3.69198E-05,            !- Coefficient6 x*y",
        "    4.44,                    !- Minimum Value of x",
        "    12.78,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    29.44,                   !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Curve:Biquadratic,",
        "    ChillerHeaterHtgCapFT,   !- Name",
        "    0.9415266,               !- Coefficient1 Constant",
        "    5.527431E-02,            !- Coefficient2 x",
        "    3.573558E-04,            !- Coefficient3 x**2",
        "    1.258391E-03,            !- Coefficient4 y",
        "    -6.420546E-05,           !- Coefficient5 y**2",
        "    -5.350989E-04,           !- Coefficient6 x*y",
        "    4.44,                    !- Minimum Value of x",
        "    15.56,                   !- Maximum Value of x",
        "    35,                      !- Minimum Value of y",
        "    57.22,                   !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Curve:Biquadratic,",
        "    ChillerHeaterClgEIRFT,   !- Name",
        "    0.7362431,               !- Coefficient1 Constant",
        "    2.136491E-02,            !- Coefficient2 x",
        "    3.638909E-04,            !- Coefficient3 x**2",
        "    -4.284947E-03,           !- Coefficient4 y",
        "    3.389817E-04,            !- Coefficient5 y**2",
        "    -3.632396E-04,           !- Coefficient6 x*y",
        "    4.44,                    !- Minimum Value of x",
        "    12.78,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    29.44,                   !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Curve:Biquadratic,",
        "    ChillerHeaterHtgEIRFT,   !- Name",
        "    0.2286246,               !- Coefficient1 Constant",
        "    2.498714E-02,            !- Coefficient2 x",
        "    -1.267106E-05,           !- Coefficient3 x**2",
        "    9.327184E-03,            !- Coefficient4 y",
        "    5.892037E-05,            !- Coefficient5 y**2",
        "    -3.268512E-04,           !- Coefficient6 x*y",
        "    4.44,                    !- Minimum Value of x",
        "    15.56,                   !- Maximum Value of x",
        "    35.0,                    !- Minimum Value of y",
        "    57.22,                   !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        " Curve:Cubic,",
        "     ChillerHeaterClgEIRFPLR, !- Name",
        "     0.0,                     !- Coefficient1 Constant",
        "     1.22895,                 !- Coefficient2 x",
        "     -0.751383,               !- Coefficient3 x**2",
        "     0.517396,                !- Coefficient4 x**3",
        "     0.2,                     !- Minimum Value of x",
        "     1;                       !- Maximum Value of x",

        "Curve:Cubic,",
        "    ChillerHeaterHtgEIRFPLR, !- Name",
        "    0.0,                     !- Coefficient1 Constant",
        "    1.12853,                 !- Coefficient2 x",
        "    -0.0264962,              !- Coefficient3 x**2",
        "    -0.103811,               !- Coefficient4 x**3",
        "    0.3,                     !- Minimum Value of x",
        "    1;                       !- Maximum Value of x"

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    // May not need for direct system input processing call (need when caling factory)
    state->dataPlantCentralHeatPumpSystem->getSystemInputFlag = true;

    // call the central heat pump system input processing function
    PlantCentralHeatPumpSystem::getCentralHeatPumpSystemInput(*state);

    // verify that under this scenario of not finding a schedule match, ScheduleAlwaysOn is the treated default
    EXPECT_EQ(state->dataPlantCentralHeatPumpSystem->systems[0].ancillaryPowerSched, Sched::GetScheduleAlwaysOn(*state));
    EXPECT_TRUE(state->dataPlantCentralHeatPumpSystem->systems[0].allModulesVariableFlow);
    auto const &module = state->dataPlantCentralHeatPumpSystem->systems[0].modules[0];
    EXPECT_TRUE(module.variableFlow);
    EXPECT_EQ(module.availabilitySchedule, Sched::GetScheduleAlwaysOn(*state));
    EXPECT_FALSE(module.performanceData().constantFlow);
    ASSERT_FALSE(state->dataPlantCentralHeatPumpSystem->performanceDefinitions.empty());
    EXPECT_EQ(1u, state->dataPlantCentralHeatPumpSystem->performanceDefinitions.size());
    EXPECT_EQ(1, state->dataPlantCentralHeatPumpSystem->numPerformanceReferences);
    EXPECT_EQ(&state->dataPlantCentralHeatPumpSystem->performanceDefinitions[0], module.performance);

    // verify that node names were processed correctly
    EXPECT_EQ(state->dataBranchNodeConnections->NumOfNodeConnections, 6);

    EXPECT_EQ(state->dataBranchNodeConnections->NodeConnections(1).NodeName, "CHW_LOOP HEATPUMP1 CHW INLET");
    EXPECT_ENUM_EQ(state->dataBranchNodeConnections->NodeConnections(1).ConnectionType, Node::ConnectionType::Inlet);
    EXPECT_ENUM_EQ(state->dataBranchNodeConnections->NodeConnections(1).FluidStream, Node::CompFluidStream::Primary);

    EXPECT_EQ(state->dataBranchNodeConnections->NodeConnections(2).NodeName, "CHW_LOOP HEATPUMP1 CHW OUTLET");
    EXPECT_ENUM_EQ(state->dataBranchNodeConnections->NodeConnections(2).ConnectionType, Node::ConnectionType::Outlet);
    EXPECT_ENUM_EQ(state->dataBranchNodeConnections->NodeConnections(2).FluidStream, Node::CompFluidStream::Primary);

    EXPECT_EQ(state->dataBranchNodeConnections->NodeConnections(3).NodeName, "CHW_LOOP HEATPUMP1 CND INLET");
    EXPECT_ENUM_EQ(state->dataBranchNodeConnections->NodeConnections(3).ConnectionType, Node::ConnectionType::Inlet);
    EXPECT_ENUM_EQ(state->dataBranchNodeConnections->NodeConnections(3).FluidStream, Node::CompFluidStream::Secondary);

    EXPECT_EQ(state->dataBranchNodeConnections->NodeConnections(4).NodeName, "CHW_LOOP HEATPUMP1 CND OUTLET");
    EXPECT_ENUM_EQ(state->dataBranchNodeConnections->NodeConnections(4).ConnectionType, Node::ConnectionType::Outlet);
    EXPECT_ENUM_EQ(state->dataBranchNodeConnections->NodeConnections(4).FluidStream, Node::CompFluidStream::Secondary);

    EXPECT_EQ(state->dataBranchNodeConnections->NodeConnections(5).NodeName, "CHW_LOOP HEATPUMP1 HHW INLET");
    EXPECT_ENUM_EQ(state->dataBranchNodeConnections->NodeConnections(5).ConnectionType, Node::ConnectionType::Inlet);
    EXPECT_ENUM_EQ(state->dataBranchNodeConnections->NodeConnections(5).FluidStream, Node::CompFluidStream::Tertiary);

    EXPECT_EQ(state->dataBranchNodeConnections->NodeConnections(6).NodeName, "CHW_LOOP HEATPUMP1 HHW OUTLET");
    EXPECT_ENUM_EQ(state->dataBranchNodeConnections->NodeConnections(6).ConnectionType, Node::ConnectionType::Outlet);
    EXPECT_ENUM_EQ(state->dataBranchNodeConnections->NodeConnections(6).FluidStream, Node::CompFluidStream::Tertiary);
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_FlowModeResolutionIsSystemScopedAndMutuallyExclusive)
{
    state->init_state(*state);

    PlantCentralHeatPumpSystem::CentralHeatPumpSystem mixedSystem;
    mixedSystem.Name = "MIXED WRAPPER";
    mixedSystem.modules.resize(2);
    PlantCentralHeatPumpSystem::PerformanceData constantPerformance;
    PlantCentralHeatPumpSystem::PerformanceData variablePerformance;
    constantPerformance.constantFlow = true;
    variablePerformance.variableFlow = true;
    mixedSystem.modules[0].initialize(constantPerformance, nullptr);
    mixedSystem.modules[1].initialize(variablePerformance, nullptr);

    mixedSystem.resolveFlowMode(*state);

    EXPECT_FALSE(mixedSystem.allModulesVariableFlow);
    for (auto const &module : mixedSystem.modules) {
        EXPECT_FALSE(module.variableFlow);
    }
    EXPECT_TRUE(constantPerformance.constantFlow);
    EXPECT_TRUE(variablePerformance.variableFlow);
    EXPECT_TRUE(compare_err_stream_substring("MIXED WRAPPER contains both constant-flow and variable-flow", true));

    PlantCentralHeatPumpSystem::CentralHeatPumpSystem variableSystem;
    variableSystem.Name = "VARIABLE WRAPPER";
    variableSystem.modules.resize(2);
    PlantCentralHeatPumpSystem::PerformanceData variablePerformance1;
    PlantCentralHeatPumpSystem::PerformanceData variablePerformance2;
    variablePerformance1.variableFlow = true;
    variablePerformance2.variableFlow = true;
    variableSystem.modules[0].initialize(variablePerformance1, nullptr);
    variableSystem.modules[1].initialize(variablePerformance2, nullptr);

    variableSystem.resolveFlowMode(*state);

    EXPECT_TRUE(variableSystem.allModulesVariableFlow);
    for (auto const &module : variableSystem.modules) {
        EXPECT_TRUE(module.variableFlow);
    }
    EXPECT_FALSE(has_err_output());
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_StandardContainersResetToEmpty)
{
    auto &data = *state->dataPlantCentralHeatPumpSystem;

    EXPECT_TRUE(data.systems.empty());
    EXPECT_TRUE(data.performanceDefinitions.empty());

    data.systems.resize(1);
    data.systems.front().modules.resize(2);
    data.performanceDefinitions.resize(2);
    data.numPerformanceReferences = 2;

    data.clear_state();

    EXPECT_TRUE(data.systems.empty());
    EXPECT_TRUE(data.performanceDefinitions.empty());
    EXPECT_EQ(0, data.numPerformanceReferences);
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_NativePerformanceInputReadsCompleteObjectAndMarksItUsed)
{
    ASSERT_TRUE(process_json(makeChillerHeaterNativeJSON(true, true)));
    state->init_state(*state);

    EXPECT_NO_THROW(PlantCentralHeatPumpSystem::getPerformanceInput(*state));
    EXPECT_FALSE(has_err_output());

    ASSERT_FALSE(state->dataPlantCentralHeatPumpSystem->performanceDefinitions.empty());
    auto const &performance = state->dataPlantCentralHeatPumpSystem->performanceDefinitions[0];
    EXPECT_EQ("NATIVE MIXED CASE MODULE", performance.Name);
    EXPECT_EQ(PlantCentralHeatPumpSystem::CondenserTemperatureMode::LeavingCondenser, performance.coolingCondenserTemperatureMode);
    EXPECT_EQ(PlantCentralHeatPumpSystem::CondenserTemperatureMode::EnteringCondenser, performance.heatingCondenserTemperatureMode);
    EXPECT_FALSE(performance.constantFlow);
    EXPECT_TRUE(performance.variableFlow);
    EXPECT_EQ(DataSizing::AutoSize, performance.referenceCoolingCapacity);
    EXPECT_TRUE(performance.referenceCoolingCapacityWasAutoSized);
    EXPECT_EQ(DataSizing::AutoSize, performance.designEvaporatorVolFlowRate);
    EXPECT_TRUE(performance.designEvaporatorVolFlowRateWasAutoSized);
    EXPECT_EQ(DataSizing::AutoSize, performance.designCondenserVolFlowRate);
    EXPECT_TRUE(performance.designCondenserVolFlowRateWasAutoSized);
    EXPECT_DOUBLE_EQ(0.001, performance.designHeatingVolFlowRate);
    EXPECT_DOUBLE_EQ(0.8, performance.compressorMotorEfficiency);
    EXPECT_DOUBLE_EQ(0.5, performance.coolingOptimumPartLoadRatio);
    EXPECT_DOUBLE_EQ(0.5, performance.heatingOptimumPartLoadRatio);
    EXPECT_DOUBLE_EQ(1.2, performance.sizingFactor);
    EXPECT_GT(performance.coolingCapacityTemperatureCurveIndex, 0);
    EXPECT_GT(performance.heatingEIRPartLoadCurveIndex, 0);

    state->dataGlobal->DisplayUnusedObjects = true;
    state->dataGlobal->DisplayAllWarnings = true;
    state->dataInputProcessing->inputProcessor->reportOrphanRecordObjects(*state);
    EXPECT_FALSE(compare_err_stream_substring("Native Mixed Case Module", true, false));
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_NativePerformanceInputAppliesSchemaDefaults)
{
    ASSERT_TRUE(process_json(makeChillerHeaterNativeJSON(false)));
    state->init_state(*state);

    EXPECT_NO_THROW(PlantCentralHeatPumpSystem::getPerformanceInput(*state));
    EXPECT_FALSE(has_err_output());

    auto const &performance = state->dataPlantCentralHeatPumpSystem->performanceDefinitions[0];
    EXPECT_TRUE(performance.constantFlow);
    EXPECT_FALSE(performance.variableFlow);
    EXPECT_EQ(PlantCentralHeatPumpSystem::CondenserTemperatureMode::EnteringCondenser, performance.coolingCondenserTemperatureMode);
    EXPECT_EQ(PlantCentralHeatPumpSystem::CondenserTemperatureMode::LeavingCondenser, performance.heatingCondenserTemperatureMode);
    EXPECT_DOUBLE_EQ(6.67, performance.coolingReferenceEvaporatorOutletTemp);
    EXPECT_DOUBLE_EQ(29.44, performance.coolingReferenceCondenserInletTemp);
    EXPECT_DOUBLE_EQ(35.0, performance.coolingReferenceCondenserOutletTemp);
    EXPECT_DOUBLE_EQ(0.75, performance.heatingToCoolingCapacityRatio);
    EXPECT_DOUBLE_EQ(1.38, performance.heatingToCoolingPowerRatio);
    EXPECT_DOUBLE_EQ(6.67, performance.heatingReferenceEvaporatorOutletTemp);
    EXPECT_DOUBLE_EQ(49.0, performance.heatingReferenceCondenserOutletTemp);
    EXPECT_DOUBLE_EQ(29.44, performance.heatingReferenceCondenserInletTemp);
    EXPECT_DOUBLE_EQ(12.22, performance.minimumEvaporatorOutletTemp);
    EXPECT_DOUBLE_EQ(0.0, performance.designEvaporatorVolFlowRate);
    EXPECT_DOUBLE_EQ(0.0, performance.designCondenserVolFlowRate);
    EXPECT_DOUBLE_EQ(0.0, performance.designHeatingVolFlowRate);
    EXPECT_DOUBLE_EQ(1.0, performance.compressorMotorEfficiency);
    EXPECT_DOUBLE_EQ(1.0, performance.coolingOptimumPartLoadRatio);
    EXPECT_DOUBLE_EQ(1.0, performance.heatingOptimumPartLoadRatio);
    EXPECT_DOUBLE_EQ(1.0, performance.sizingFactor);
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_NativePerformanceInputReportsReferencedField)
{
    auto epJSON = makeChillerHeaterNativeJSON();
    epJSON["ChillerHeaterPerformance:Electric:EIR"]["Native Mixed Case Module"]["cooling_mode_cooling_capacity_function_of_temperature_curve_name"] =
        "Missing Temperature Curve";
    ASSERT_TRUE(process_json(epJSON));
    state->init_state(*state);

    EXPECT_THROW(PlantCentralHeatPumpSystem::getPerformanceInput(*state), std::runtime_error);
    EXPECT_TRUE(compare_err_stream_substring("Cooling Mode Cooling Capacity Function of Temperature Curve Name=MISSING TEMPERATURE CURVE", true));
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_NativePerformanceInputRejectsCaseInsensitiveDuplicateNames)
{
    auto epJSON = makeChillerHeaterNativeJSON();
    auto &objects = epJSON["ChillerHeaterPerformance:Electric:EIR"];
    objects["native mixed case module"] = objects["Native Mixed Case Module"];
    ASSERT_TRUE(process_json(epJSON));
    state->init_state(*state);

    EXPECT_THROW(PlantCentralHeatPumpSystem::getPerformanceInput(*state), std::runtime_error);
    EXPECT_TRUE(compare_err_stream_substring("duplicate name.", true));
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_IDFAndNativePerformanceInputsProduceEquivalentState)
{
    std::string const idf = delimited_string({
        "ChillerHeaterPerformance:Electric:EIR,",
        "  Native Mixed Case Module,",
        "  10000,",
        "  5.0,",
        "  7.0,",
        "  30.0,",
        "  35.0,",
        "  0.75,",
        "  1.0,",
        "  7.0,",
        "  50.0,",
        "  30.0,",
        "  3.0,",
        "  variableFlow,",
        "  0.001,",
        "  0.001,",
        "  0.001,",
        "  0.8,",
        "  LeavingCondenser,",
        "  Reference Temperature Curve,",
        "  Reference Temperature Curve,",
        "  Reference PLR Curve,",
        "  0.5,",
        "  EnteringCondenser,",
        "  Reference Temperature Curve,",
        "  Reference Temperature Curve,",
        "  Reference PLR Curve,",
        "  0.5,",
        "  1.2;",

        "Curve:Biquadratic,",
        "  Reference Temperature Curve,",
        "  1.0, 0.0, 0.0, 0.0, 0.0, 0.0,",
        "  -100.0, 100.0, -100.0, 100.0;",

        "Curve:Quadratic,",
        "  Reference PLR Curve,",
        "  1.0, 0.0, 0.0,",
        "  0.0, 1.0;",
    });

    ASSERT_TRUE(process_idf(idf));
    state->init_state(*state);
    EXPECT_NO_THROW(PlantCentralHeatPumpSystem::getPerformanceInput(*state));
    EXPECT_FALSE(has_err_output());
    PlantCentralHeatPumpSystem::PerformanceData const idfPerformance = state->dataPlantCentralHeatPumpSystem->performanceDefinitions[0];

    state->dataPlantCentralHeatPumpSystem->clear_state();
    state->dataCurveManager->clear_state();
    state->dataInputProcessing->clear_state();
    state->dataIPShortCut->clear_state();
    EXPECT_TRUE(compare_err_stream("", true));

    ASSERT_TRUE(process_json(makeChillerHeaterNativeJSON()));
    Curve::GetCurveInput(*state);
    EXPECT_NO_THROW(PlantCentralHeatPumpSystem::getPerformanceInput(*state));
    EXPECT_TRUE(compare_err_stream("", true));
    auto const &nativePerformance = state->dataPlantCentralHeatPumpSystem->performanceDefinitions[0];

    EXPECT_EQ(idfPerformance.Name, nativePerformance.Name);
    EXPECT_EQ(idfPerformance.coolingCondenserTemperatureMode, nativePerformance.coolingCondenserTemperatureMode);
    EXPECT_EQ(idfPerformance.heatingCondenserTemperatureMode, nativePerformance.heatingCondenserTemperatureMode);
    EXPECT_EQ(idfPerformance.constantFlow, nativePerformance.constantFlow);
    EXPECT_EQ(idfPerformance.variableFlow, nativePerformance.variableFlow);
    EXPECT_EQ(idfPerformance.referenceCoolingCapacityWasAutoSized, nativePerformance.referenceCoolingCapacityWasAutoSized);
    EXPECT_EQ(idfPerformance.designEvaporatorVolFlowRateWasAutoSized, nativePerformance.designEvaporatorVolFlowRateWasAutoSized);
    EXPECT_EQ(idfPerformance.designCondenserVolFlowRateWasAutoSized, nativePerformance.designCondenserVolFlowRateWasAutoSized);
    EXPECT_DOUBLE_EQ(idfPerformance.referenceCoolingCapacity, nativePerformance.referenceCoolingCapacity);
    EXPECT_DOUBLE_EQ(idfPerformance.referenceCoolingCOP, nativePerformance.referenceCoolingCOP);
    EXPECT_DOUBLE_EQ(idfPerformance.coolingReferenceEvaporatorOutletTemp, nativePerformance.coolingReferenceEvaporatorOutletTemp);
    EXPECT_DOUBLE_EQ(idfPerformance.coolingReferenceCondenserInletTemp, nativePerformance.coolingReferenceCondenserInletTemp);
    EXPECT_DOUBLE_EQ(idfPerformance.coolingReferenceCondenserOutletTemp, nativePerformance.coolingReferenceCondenserOutletTemp);
    EXPECT_DOUBLE_EQ(idfPerformance.heatingToCoolingCapacityRatio, nativePerformance.heatingToCoolingCapacityRatio);
    EXPECT_DOUBLE_EQ(idfPerformance.heatingToCoolingPowerRatio, nativePerformance.heatingToCoolingPowerRatio);
    EXPECT_DOUBLE_EQ(idfPerformance.heatingReferenceEvaporatorOutletTemp, nativePerformance.heatingReferenceEvaporatorOutletTemp);
    EXPECT_DOUBLE_EQ(idfPerformance.heatingReferenceCondenserOutletTemp, nativePerformance.heatingReferenceCondenserOutletTemp);
    EXPECT_DOUBLE_EQ(idfPerformance.heatingReferenceCondenserInletTemp, nativePerformance.heatingReferenceCondenserInletTemp);
    EXPECT_DOUBLE_EQ(idfPerformance.minimumEvaporatorOutletTemp, nativePerformance.minimumEvaporatorOutletTemp);
    EXPECT_DOUBLE_EQ(idfPerformance.designEvaporatorVolFlowRate, nativePerformance.designEvaporatorVolFlowRate);
    EXPECT_DOUBLE_EQ(idfPerformance.designCondenserVolFlowRate, nativePerformance.designCondenserVolFlowRate);
    EXPECT_DOUBLE_EQ(idfPerformance.designHeatingVolFlowRate, nativePerformance.designHeatingVolFlowRate);
    EXPECT_DOUBLE_EQ(idfPerformance.compressorMotorEfficiency, nativePerformance.compressorMotorEfficiency);
    EXPECT_DOUBLE_EQ(idfPerformance.coolingOptimumPartLoadRatio, nativePerformance.coolingOptimumPartLoadRatio);
    EXPECT_DOUBLE_EQ(idfPerformance.heatingOptimumPartLoadRatio, nativePerformance.heatingOptimumPartLoadRatio);
    EXPECT_DOUBLE_EQ(idfPerformance.sizingFactor, nativePerformance.sizingFactor);
    EXPECT_DOUBLE_EQ(idfPerformance.coolingMinimumPartLoadRatio, nativePerformance.coolingMinimumPartLoadRatio);
    EXPECT_DOUBLE_EQ(idfPerformance.coolingMaximumPartLoadRatio, nativePerformance.coolingMaximumPartLoadRatio);
    EXPECT_DOUBLE_EQ(idfPerformance.heatingMinimumPartLoadRatio, nativePerformance.heatingMinimumPartLoadRatio);
    EXPECT_DOUBLE_EQ(idfPerformance.heatingMaximumPartLoadRatio, nativePerformance.heatingMaximumPartLoadRatio);
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_IDFAndNativeSystemInputsProduceEquivalentStateForAllFixedGroups)
{
    ASSERT_TRUE(process_idf(makeAllModuleGroupsIDF()));
    state->init_state(*state);
    EXPECT_NO_THROW(PlantCentralHeatPumpSystem::getCentralHeatPumpSystemInput(*state));
    EXPECT_TRUE(compare_err_stream("", true));

    ASSERT_EQ(1u, state->dataPlantCentralHeatPumpSystem->systems.size());
    ASSERT_EQ(1u, state->dataPlantCentralHeatPumpSystem->performanceDefinitions.size());
    EXPECT_EQ(20, state->dataPlantCentralHeatPumpSystem->numPerformanceReferences);
    auto const &idfSystem = state->dataPlantCentralHeatPumpSystem->systems[0];
    ASSERT_EQ(20u, idfSystem.modules.size());
    std::array<std::string, 6> const idfNodeNames = {
        state->dataLoopNodes->NodeID(idfSystem.coolingInletNodeNum),
        state->dataLoopNodes->NodeID(idfSystem.coolingOutletNodeNum),
        state->dataLoopNodes->NodeID(idfSystem.sourceInletNodeNum),
        state->dataLoopNodes->NodeID(idfSystem.sourceOutletNodeNum),
        state->dataLoopNodes->NodeID(idfSystem.heatingInletNodeNum),
        state->dataLoopNodes->NodeID(idfSystem.heatingOutletNodeNum),
    };
    std::vector<std::string> idfPerformanceNames;
    std::vector<std::string> idfScheduleNames;
    for (auto const &module : idfSystem.modules) {
        idfPerformanceNames.push_back(module.name());
        ASSERT_NE(nullptr, module.availabilitySchedule);
        idfScheduleNames.push_back(module.availabilitySchedule->Name);
    }
    std::string const idfName = idfSystem.Name;
    std::string const idfAncillaryScheduleName = idfSystem.ancillaryPowerSched->Name;
    Real64 const idfancillaryPower = idfSystem.ancillaryPower;
    bool const idfVariableFlow = idfSystem.allModulesVariableFlow;

    state->dataPlantCentralHeatPumpSystem->clear_state();
    state->dataCurveManager->clear_state();
    state->dataInputProcessing->clear_state();
    state->dataIPShortCut->clear_state();
    EXPECT_TRUE(compare_err_stream("", true));

    ASSERT_TRUE(process_json(makeNativeSystemJSON(true)));
    Curve::GetCurveInput(*state);
    EXPECT_NO_THROW(PlantCentralHeatPumpSystem::getCentralHeatPumpSystemInput(*state));
    EXPECT_TRUE(compare_err_stream("", true));

    ASSERT_EQ(1u, state->dataPlantCentralHeatPumpSystem->systems.size());
    ASSERT_EQ(1u, state->dataPlantCentralHeatPumpSystem->performanceDefinitions.size());
    EXPECT_EQ(20, state->dataPlantCentralHeatPumpSystem->numPerformanceReferences);
    auto const &nativeSystem = state->dataPlantCentralHeatPumpSystem->systems[0];
    ASSERT_EQ(20u, nativeSystem.modules.size());
    std::array<std::string, 6> const nativeNodeNames = {
        state->dataLoopNodes->NodeID(nativeSystem.coolingInletNodeNum),
        state->dataLoopNodes->NodeID(nativeSystem.coolingOutletNodeNum),
        state->dataLoopNodes->NodeID(nativeSystem.sourceInletNodeNum),
        state->dataLoopNodes->NodeID(nativeSystem.sourceOutletNodeNum),
        state->dataLoopNodes->NodeID(nativeSystem.heatingInletNodeNum),
        state->dataLoopNodes->NodeID(nativeSystem.heatingOutletNodeNum),
    };
    std::vector<std::string> nativePerformanceNames;
    std::vector<std::string> nativeScheduleNames;
    for (auto const &module : nativeSystem.modules) {
        EXPECT_EQ(&state->dataPlantCentralHeatPumpSystem->performanceDefinitions[0], module.performance);
        nativePerformanceNames.push_back(module.name());
        ASSERT_NE(nullptr, module.availabilitySchedule);
        nativeScheduleNames.push_back(module.availabilitySchedule->Name);
    }

    EXPECT_EQ(idfName, nativeSystem.Name);
    EXPECT_EQ(idfNodeNames, nativeNodeNames);
    EXPECT_EQ(idfPerformanceNames, nativePerformanceNames);
    EXPECT_EQ(idfScheduleNames, nativeScheduleNames);
    EXPECT_EQ(idfAncillaryScheduleName, nativeSystem.ancillaryPowerSched->Name);
    EXPECT_DOUBLE_EQ(idfancillaryPower, nativeSystem.ancillaryPower);
    EXPECT_EQ(idfVariableFlow, nativeSystem.allModulesVariableFlow);
    EXPECT_EQ(6, state->dataBranchNodeConnections->NumOfNodeConnections);

    state->dataGlobal->DisplayUnusedObjects = true;
    state->dataGlobal->DisplayAllWarnings = true;
    state->dataInputProcessing->inputProcessor->reportOrphanRecordObjects(*state);
    EXPECT_FALSE(compare_err_stream_substring("Object=CentralHeatPumpSystem=Native Many Groups Wrapper", true, false));
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_NativeSystemInputReadsFixedGroupsAndSchedules)
{
    ASSERT_TRUE(process_json(makeNativeSystemJSON(false)));
    state->init_state(*state);
    EXPECT_NO_THROW(PlantCentralHeatPumpSystem::getCentralHeatPumpSystemInput(*state));

    ASSERT_EQ(1u, state->dataPlantCentralHeatPumpSystem->systems.size());
    ASSERT_EQ(1u, state->dataPlantCentralHeatPumpSystem->performanceDefinitions.size());
    EXPECT_EQ(6, state->dataPlantCentralHeatPumpSystem->numPerformanceReferences);
    auto const &system = state->dataPlantCentralHeatPumpSystem->systems[0];
    EXPECT_EQ("NATIVE SPARSE WRAPPER", system.Name);
    EXPECT_DOUBLE_EQ(25.0, system.ancillaryPower);
    ASSERT_NE(nullptr, system.ancillaryPowerSched);
    EXPECT_EQ("ANCILLARY SCHEDULE", system.ancillaryPowerSched->Name);
    ASSERT_EQ(8u, system.modules.size());

    auto *alwaysOn = Sched::GetScheduleAlwaysOn(*state);
    std::array<std::string, 8> const expectedScheduleNames = {
        "MODULE SCHEDULE 1",
        alwaysOn->Name,
        alwaysOn->Name,
        "MODULE SCHEDULE 3",
        "MODULE SCHEDULE 4",
        alwaysOn->Name,
        "MODULE SCHEDULE 6",
        "MODULE SCHEDULE 6",
    };
    for (std::size_t moduleIndex = 0; moduleIndex < system.modules.size(); ++moduleIndex) {
        auto const &module = system.modules[moduleIndex];
        EXPECT_EQ(&state->dataPlantCentralHeatPumpSystem->performanceDefinitions[0], module.performance);
        ASSERT_NE(nullptr, module.availabilitySchedule);
        EXPECT_EQ(expectedScheduleNames[moduleIndex], module.availabilitySchedule->Name);
    }
    EXPECT_EQ(6, state->dataBranchNodeConnections->NumOfNodeConnections);
    EXPECT_TRUE(compare_err_stream_substring("MISSING MODULE SCHEDULE", false));
    EXPECT_TRUE(compare_err_stream_substring("the AlwaysOn schedule will be used", true));
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_PerformanceReferencesRemainStableAcrossVectorBinding)
{
    auto epJSON = makeNativeSystemJSON(false);
    auto &performanceObjects = epJSON["ChillerHeaterPerformance:Electric:EIR"];
    performanceObjects["Second Native Module"] = performanceObjects.at("Native Mixed Case Module");
    auto &systemObject = epJSON["CentralHeatPumpSystem"]["Native Sparse Wrapper"];
    systemObject[fixedModuleGroupFieldKeys(6).performanceName] = "Second Native Module";

    ASSERT_TRUE(process_json(epJSON));
    state->init_state(*state);
    EXPECT_NO_THROW(PlantCentralHeatPumpSystem::getCentralHeatPumpSystemInput(*state));

    auto const &data = *state->dataPlantCentralHeatPumpSystem;
    ASSERT_EQ(2u, data.performanceDefinitions.size());
    auto const firstPerformance = std::find_if(data.performanceDefinitions.begin(), data.performanceDefinitions.end(), [](auto const &performance) {
        return performance.Name == "NATIVE MIXED CASE MODULE";
    });
    auto const secondPerformance = std::find_if(data.performanceDefinitions.begin(), data.performanceDefinitions.end(), [](auto const &performance) {
        return performance.Name == "SECOND NATIVE MODULE";
    });
    ASSERT_NE(data.performanceDefinitions.end(), firstPerformance);
    ASSERT_NE(data.performanceDefinitions.end(), secondPerformance);
    ASSERT_NE(&*firstPerformance, &*secondPerformance);

    ASSERT_EQ(1u, data.systems.size());
    auto const &modules = data.systems.front().modules;
    ASSERT_EQ(8u, modules.size());
    EXPECT_EQ(&*firstPerformance, modules.front().performance);
    EXPECT_EQ(&*firstPerformance, modules[5].performance);
    EXPECT_EQ(&*secondPerformance, modules[6].performance);
    EXPECT_EQ(&*secondPerformance, modules.back().performance);
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_NativeSystemSchemaRequiresFirstModuleGroup)
{
    auto epJSON = makeNativeSystemJSON(true);
    epJSON["CentralHeatPumpSystem"]["Native Many Groups Wrapper"].erase(fixedModuleGroupFieldKeys(1).performanceObjectType);

    EXPECT_FALSE(process_json(epJSON, false));
    EXPECT_TRUE(compare_err_stream_substring("Missing required property", true));
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_NativeSystemInputRejectsInvalidPerformanceReference)
{
    auto epJSON = makeNativeSystemJSON(false);
    auto const group2Keys = fixedModuleGroupFieldKeys(2);
    epJSON["CentralHeatPumpSystem"]["Native Sparse Wrapper"][group2Keys.performanceName] = "Missing Performance";
    ASSERT_TRUE(process_json(epJSON));
    state->init_state(*state);

    EXPECT_THROW(PlantCentralHeatPumpSystem::getCentralHeatPumpSystemInput(*state), std::runtime_error);
    EXPECT_TRUE(compare_err_stream_substring(group2Keys.performanceName + " = MISSING PERFORMANCE, item not found.", true));
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_NativeSystemInputRejectsCaseInsensitiveDuplicateNames)
{
    auto epJSON = makeNativeSystemJSON(true);
    auto &systems = epJSON["CentralHeatPumpSystem"];
    systems["native many groups wrapper"] = systems["Native Many Groups Wrapper"];
    ASSERT_TRUE(process_json(epJSON));
    state->init_state(*state);

    EXPECT_THROW(PlantCentralHeatPumpSystem::getCentralHeatPumpSystemInput(*state), std::runtime_error);
    EXPECT_TRUE(compare_err_stream_substring("duplicate name.", true));
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_InputValidationUsesConfiguredReferenceTemperaturesAndBicubicPLRDomain)
{
    ASSERT_TRUE(process_idf(makeChillerHeaterValidationInput()));
    state->init_state(*state);

    EXPECT_NO_THROW(PlantCentralHeatPumpSystem::getPerformanceInput(*state));
    EXPECT_FALSE(has_err_output());

    ASSERT_FALSE(state->dataPlantCentralHeatPumpSystem->performanceDefinitions.empty());
    auto const &chillerHeater = state->dataPlantCentralHeatPumpSystem->performanceDefinitions[0];
    EXPECT_DOUBLE_EQ(0.2, chillerHeater.coolingMinimumPartLoadRatio);
    EXPECT_DOUBLE_EQ(1.0, chillerHeater.coolingMaximumPartLoadRatio);
    EXPECT_DOUBLE_EQ(0.3, chillerHeater.heatingMinimumPartLoadRatio);
    EXPECT_DOUBLE_EQ(1.0, chillerHeater.heatingMaximumPartLoadRatio);
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_InputValidationRejectsOptimumPLROutsideCurveDomain)
{
    ASSERT_TRUE(process_idf(makeChillerHeaterValidationInput(0.75, 0.1)));
    state->init_state(*state);

    EXPECT_THROW(PlantCentralHeatPumpSystem::getPerformanceInput(*state), std::runtime_error);
    EXPECT_TRUE(compare_err_stream_substring("Cooling Mode Cooling Capacity Optimum Part Load Ratio must be within", true));
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_InputValidationRejectsInvalidPartLoadCurveDomain)
{
    ASSERT_TRUE(process_idf(makeChillerHeaterValidationInput(0.75, 0.5, 0.5, -0.1)));
    state->init_state(*state);

    EXPECT_THROW(PlantCentralHeatPumpSystem::getPerformanceInput(*state), std::runtime_error);
    EXPECT_TRUE(compare_err_stream_substring("Part-load ratio limits [-0.100, 1.000] must include 1.0", true));
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_RuntimeRejectsNonpositiveHeatingCapacityRatio)
{
    ASSERT_TRUE(process_idf(makeChillerHeaterValidationInput(-0.75)));
    state->init_state(*state);

    EXPECT_THROW(PlantCentralHeatPumpSystem::getPerformanceInput(*state), std::runtime_error);
    EXPECT_TRUE(compare_err_stream_substring("Reference Heating Mode Cooling Capacity Ratio=-0.75", true));
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_RuntimeValidationRejectsInvalidStaticPerformanceInputs)
{
    ASSERT_TRUE(process_idf(makeChillerHeaterValidationInput()));
    auto &performanceObject = state->dataInputProcessing->inputProcessor->epJSON["ChillerHeaterPerformance:Electric:EIR"]["Validation Module"];
    performanceObject["reference_cooling_mode_evaporator_capacity"] = -10000.0;
    performanceObject["reference_cooling_mode_cop"] = -5.0;
    performanceObject["reference_heating_mode_cooling_capacity_ratio"] = -0.75;
    performanceObject["reference_heating_mode_cooling_power_input_ratio"] = -1.0;
    performanceObject["compressor_motor_efficiency"] = 1.1;
    state->init_state(*state);

    EXPECT_THROW(PlantCentralHeatPumpSystem::getPerformanceInput(*state), std::runtime_error);
    EXPECT_TRUE(compare_err_stream_substring("Reference Cooling Mode Evaporator Capacity=-10000.00", false));
    EXPECT_TRUE(compare_err_stream_substring("Reference Cooling Mode COP=-5.00", false));
    EXPECT_TRUE(compare_err_stream_substring("Reference Heating Mode Cooling Capacity Ratio=-0.75", false));
    EXPECT_TRUE(compare_err_stream_substring("Reference Heating Mode Cooling Power Input Ratio=-1.00", false));
    EXPECT_TRUE(compare_err_stream_substring("Compressor Motor Efficiency = 1.100", true));
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_AncillaryScheduleDefaultsAndScalesPowerAndEnergy)
{
    state->init_state(*state);
    state->dataLoopNodes->Node.allocate(6);
    state->dataHVACGlobal->TimeStepSysSec = 600.0;

    PlantCentralHeatPumpSystem::CentralHeatPumpSystem system;
    system.coolingInletNodeNum = 1;
    system.coolingOutletNodeNum = 2;
    system.heatingInletNodeNum = 3;
    system.heatingOutletNodeNum = 4;
    system.sourceInletNodeNum = 5;
    system.sourceOutletNodeNum = 6;
    system.ancillaryPower = 100.0;
    system.modules.resize(1);

    auto setCoolingResult = [&]() {
        auto &result = system.modules[0].result;
        result = PlantCentralHeatPumpSystem::ModuleResult();
        result.currentMode = CurrentMode::CoolingOnly;
        result.coolingPower = 50.0;
        result.compressorPower = 50.0;
        result.coolingDelivered = 1000.0;
        result.sourceHeatTransfer = 1050.0;
        result.coolingMassFlowRate = 1.0;
        result.coolingOutletTemp = 7.0;
        result.sourceMassFlowRate = 1.0;
        result.sourceOutletTemp = 30.25;
    };

    setCoolingResult();
    system.ancillaryPowerSched = nullptr;
    system.updateReportingAndNodes(*state, 1.0, 0.0, 1.0, 12.0, 40.0, 30.0);
    EXPECT_DOUBLE_EQ(150.0, system.report.coolingElectricPower);
    EXPECT_DOUBLE_EQ(90000.0, system.report.coolingElectricEnergy);

    setCoolingResult();
    system.ancillaryPowerSched = Sched::GetScheduleAlwaysOn(*state);
    system.updateReportingAndNodes(*state, 1.0, 0.0, 1.0, 12.0, 40.0, 30.0);
    EXPECT_DOUBLE_EQ(150.0, system.report.coolingElectricPower);
    EXPECT_DOUBLE_EQ(90000.0, system.report.coolingElectricEnergy);

    Sched::ScheduleConstant fractionalSchedule;
    fractionalSchedule.currentVal = 0.25;
    setCoolingResult();
    system.ancillaryPowerSched = &fractionalSchedule;
    system.updateReportingAndNodes(*state, 1.0, 0.0, 1.0, 12.0, 40.0, 30.0);
    EXPECT_DOUBLE_EQ(75.0, system.report.coolingElectricPower);
    EXPECT_DOUBLE_EQ(45000.0, system.report.coolingElectricEnergy);
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_OffStateClearsAuthoritativeState)
{
    state->init_state(*state);
    state->dataLoopNodes->Node.allocate(6);

    PlantCentralHeatPumpSystem::CentralHeatPumpSystem system;
    system.coolingInletNodeNum = 1;
    system.coolingOutletNodeNum = 2;
    system.heatingInletNodeNum = 3;
    system.heatingOutletNodeNum = 4;
    system.sourceInletNodeNum = 5;
    system.sourceOutletNodeNum = 6;
    state->dataLoopNodes->Node(1).Temp = 12.0;
    state->dataLoopNodes->Node(3).Temp = 40.0;
    state->dataLoopNodes->Node(5).Temp = 15.0;
    state->dataLoopNodes->Node(1).MassFlowRateRequest = 1.0;
    state->dataLoopNodes->Node(3).MassFlowRateRequest = 2.0;
    state->dataLoopNodes->Node(5).MassFlowRateRequest = 3.0;

    system.requestedCoolingLoad = 1000.0;
    system.requestedHeatingLoad = 1200.0;
    system.isCoolingDominant = true;
    system.report.coolingHeatTransferRate = 1000.0;
    system.report.heatingHeatTransferRate = 1200.0;
    system.report.sourceHeatTransferRate = 200.0;
    system.modules.resize(1);
    PlantCentralHeatPumpSystem::PerformanceData performance;
    system.modules[0].initialize(performance, nullptr);
    auto &chillerHeater = system.modules[0];
    chillerHeater.result.currentMode = CurrentMode::CoolingDominant;
    chillerHeater.result.coolingPower = 500.0;
    chillerHeater.result.coolingDelivered = 1000.0;

    system.resetOffState(*state, false);

    EXPECT_DOUBLE_EQ(0.0, system.requestedCoolingLoad);
    EXPECT_DOUBLE_EQ(0.0, system.requestedHeatingLoad);
    EXPECT_FALSE(system.isCoolingDominant);
    EXPECT_FALSE(system.isHeatingDominant);
    EXPECT_EQ(CurrentMode::Off, chillerHeater.result.currentMode);
    EXPECT_DOUBLE_EQ(0.0, system.report.coolingHeatTransferRate);
    EXPECT_DOUBLE_EQ(0.0, system.report.heatingHeatTransferRate);
    EXPECT_DOUBLE_EQ(0.0, system.report.sourceHeatTransferRate);
    EXPECT_DOUBLE_EQ(12.0, state->dataLoopNodes->Node(2).Temp);
    EXPECT_DOUBLE_EQ(40.0, state->dataLoopNodes->Node(4).Temp);
    EXPECT_DOUBLE_EQ(15.0, state->dataLoopNodes->Node(6).Temp);
    EXPECT_DOUBLE_EQ(0.0, state->dataLoopNodes->Node(1).MassFlowRateRequest);
    EXPECT_DOUBLE_EQ(0.0, state->dataLoopNodes->Node(3).MassFlowRateRequest);
    EXPECT_DOUBLE_EQ(0.0, state->dataLoopNodes->Node(5).MassFlowRateRequest);
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_InactiveConnectionPreservesActiveLoad)
{
    state->init_state(*state);
    state->dataLoopNodes->Node.allocate(6);
    state->dataPlnt->PlantLoop.allocate(3);

    auto *water = Fluid::GetWater(*state);
    ASSERT_NE(nullptr, water);
    for (int loopNum = 1; loopNum <= 3; ++loopNum) {
        auto &plantLoop = state->dataPlnt->PlantLoop(loopNum);
        plantLoop.glycol = water;
        auto &loopSide = plantLoop.LoopSide(DataPlant::LoopSideLocation::Supply);
        loopSide.Branch.allocate(1);
        loopSide.Branch(1).Comp.allocate(1);
    }

    PlantCentralHeatPumpSystem::CentralHeatPumpSystem system;
    system.coolingPlantLoc = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);
    system.heatingPlantLoc = PlantLocation(2, DataPlant::LoopSideLocation::Supply, 1, 1);
    system.sourcePlantLoc = PlantLocation(3, DataPlant::LoopSideLocation::Supply, 1, 1);
    PlantUtilities::SetPlantLocationLinks(*state, system.coolingPlantLoc);
    PlantUtilities::SetPlantLocationLinks(*state, system.heatingPlantLoc);
    PlantUtilities::SetPlantLocationLinks(*state, system.sourcePlantLoc);

    system.coolingInletNodeNum = 1;
    system.coolingOutletNodeNum = 2;
    system.heatingInletNodeNum = 3;
    system.heatingOutletNodeNum = 4;
    system.sourceInletNodeNum = 5;
    system.sourceOutletNodeNum = 6;
    state->dataLoopNodes->Node(1).Temp = 12.0;
    state->dataLoopNodes->Node(3).Temp = 40.0;
    state->dataLoopNodes->Node(5).Temp = 15.0;
    state->dataLoopNodes->Node(5).MassFlowRate = 1.0;
    state->dataLoopNodes->Node(1).MassFlowRateRequest = 1.0;
    state->dataLoopNodes->Node(5).MassFlowRateRequest = 2.0;

    system.setupOutputVarsFlag = false;
    system.plantScanPending = false;
    system.environmentInitPending = false;
    system.requestedCoolingLoad = 1000.0;
    system.report.coolingHeatTransferRate = 1000.0;
    system.report.sourceHeatTransferRate = 1200.0;
    system.report.sourceInletTemp = 15.0;
    system.report.sourceOutletTemp = 15.3;
    system.report.sourceMassFlowRate = 1.0;
    system.modules.resize(1);
    PlantCentralHeatPumpSystem::PerformanceData performance;
    performance.compressorMotorEfficiency = 0.80;
    system.modules[0].initialize(performance, nullptr);
    system.modules[0].result.currentMode = CurrentMode::CoolingOnly;
    system.modules[0].result.coolingDelivered = 1000.0;

    Real64 sourceLoad = 0.0;
    system.simulate(*state, system.sourcePlantLoc, false, sourceLoad, false);
    EXPECT_DOUBLE_EQ(1000.0, system.requestedCoolingLoad);
    EXPECT_DOUBLE_EQ(1000.0, system.report.coolingHeatTransferRate);
    EXPECT_EQ(CurrentMode::CoolingOnly, system.modules[0].result.currentMode);

    Real64 heatingLoad = 0.0;
    system.simulate(*state, system.heatingPlantLoc, false, heatingLoad, false);
    EXPECT_DOUBLE_EQ(0.0, heatingLoad);
    EXPECT_DOUBLE_EQ(1000.0, system.requestedCoolingLoad);
    EXPECT_DOUBLE_EQ(0.0, system.requestedHeatingLoad);
    EXPECT_DOUBLE_EQ(1000.0, system.report.coolingHeatTransferRate);
    EXPECT_EQ(CurrentMode::CoolingOnly, system.modules[0].result.currentMode);
    EXPECT_DOUBLE_EQ(1.0, state->dataLoopNodes->Node(1).MassFlowRateRequest);
    EXPECT_DOUBLE_EQ(2.0, state->dataLoopNodes->Node(5).MassFlowRateRequest);

    Real64 coolingLoad = 0.0;
    system.simulate(*state, system.coolingPlantLoc, false, coolingLoad, false);
    EXPECT_DOUBLE_EQ(0.0, coolingLoad);
    EXPECT_DOUBLE_EQ(0.0, system.requestedCoolingLoad);
    EXPECT_DOUBLE_EQ(0.0, system.report.coolingHeatTransferRate);
    EXPECT_EQ(CurrentMode::Off, system.modules[0].result.currentMode);
    EXPECT_DOUBLE_EQ(0.0, state->dataLoopNodes->Node(1).MassFlowRateRequest);
    EXPECT_DOUBLE_EQ(0.0, state->dataLoopNodes->Node(5).MassFlowRateRequest);
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_FailedPlantScanTerminatesInitialization)
{
    state->init_state(*state);
    state->dataLoopNodes->Node.allocate(6);

    PlantCentralHeatPumpSystem::CentralHeatPumpSystem system;
    system.Name = "UNCONNECTED WRAPPER";
    system.setupOutputVarsFlag = false;
    system.environmentInitPending = false;
    system.coolingInletNodeNum = 1;
    system.coolingOutletNodeNum = 2;
    system.heatingInletNodeNum = 3;
    system.heatingOutletNodeNum = 4;
    system.sourceInletNodeNum = 5;
    system.sourceOutletNodeNum = 6;

    EXPECT_THROW(system.initialize(*state, 0.0, 1, false), std::runtime_error);
    EXPECT_TRUE(compare_err_stream_substring("could not be located on all three connected plant loops", true));
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_selectCondenserCurveTemperature)
{
    PlantCentralHeatPumpSystem::ModePerformanceData modePerformance;
    Real64 constexpr allowedTolerance = 0.001;
    Real64 constexpr condEnterTemp = 55.5;
    Real64 constexpr condLeaveTemp = 44.4;

    modePerformance.condenserMode = PlantCentralHeatPumpSystem::CondenserTemperatureMode::EnteringCondenser;
    EXPECT_NEAR(55.5,
                PlantCentralHeatPumpSystem::CentralHeatPumpSystem::selectCondenserCurveTemperature(modePerformance, condEnterTemp, condLeaveTemp),
                allowedTolerance);

    modePerformance.condenserMode = PlantCentralHeatPumpSystem::CondenserTemperatureMode::LeavingCondenser;
    EXPECT_NEAR(44.4,
                PlantCentralHeatPumpSystem::CentralHeatPumpSystem::selectCondenserCurveTemperature(modePerformance, condEnterTemp, condLeaveTemp),
                allowedTolerance);
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_ModeEnergyAndRoutingContracts)
{
    std::array<ModeEnergyContract, 6> const operatingPoints = {
        ModeEnergyContract{CurrentMode::Off, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, {0.0, 0.0, 0.0}},
        ModeEnergyContract{CurrentMode::CoolingOnly, 9000.0, 10900.0, 2000.0, 0.0, 0.95, 0.75, 1.0, {9000.0, 0.0, 10900.0}},
        ModeEnergyContract{CurrentMode::HeatingOnly, 7500.0, 9750.0, 2500.0, 0.0, 0.90, 0.60, 1.0, {0.0, 9750.0, -7500.0}},
        ModeEnergyContract{CurrentMode::HeatRecovery, 8000.0, 10000.0, 2000.0, 0.0, 1.0, 0.80, 1.0, {8000.0, 10000.0, 0.0}},
        ModeEnergyContract{CurrentMode::CoolingDominant, 8000.0, 10000.0, 2000.0, 0.0, 1.0, 0.80, 1.0, {8000.0, 3500.0, 6500.0}},
        ModeEnergyContract{CurrentMode::HeatingDominant, 8000.0, 10000.0, 2000.0, 0.0, 1.0, 0.80, 1.0, {3000.0, 10000.0, -5000.0}},
    };

    for (auto const &point : operatingPoints) {
        SCOPED_TRACE(modeName(point.mode));
        EXPECT_TRUE(checkModeEnergyContract(point));
    }
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_ModeContractsRejectKnownDefectSignatures)
{
    // Issue #7838 / current mode-3 example signature: most condenser energy disappears and there is no source transfer.
    ModeEnergyContract const issue7838Point{CurrentMode::HeatRecovery, 10790.844, 48.8987, 9602.5995, 0.0, 1.0, 0.20, 1.0, {10790.844, 48.8987, 0.0}};
    EXPECT_FALSE(checkModeEnergyContract(issue7838Point));

    // Issue #10065 signature: an active Electric:EIR module meets simultaneous loads with zero compressor power.
    ModeEnergyContract const issue10065Point{CurrentMode::HeatRecovery, 8000.0, 8000.0, 0.0, 0.0, 1.0, 0.80, 1.0, {8000.0, 8000.0, 0.0}};
    EXPECT_FALSE(checkModeEnergyContract(issue10065Point));

    // Heating-dominant useful cooling cannot be silently reclassified as source extraction.
    ModeEnergyContract const lostCoolingRoute{CurrentMode::HeatingDominant, 8000.0, 10000.0, 2000.0, 0.0, 1.0, 0.80, 1.0, {0.0, 10000.0, -8000.0}};
    EXPECT_FALSE(checkModeEnergyContract(lostCoolingRoute));
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_FinalPartLoadContracts)
{
    std::array<PartLoadContract, 3> const points = {
        PartLoadContract{10000.0, 1000.0, 1000.0, 0.30, 1.0, 0.30, 1.0 / 3.0, 0.30},
        PartLoadContract{10000.0, 6000.0, 6000.0, 0.30, 1.0, 0.60, 1.0, 0.60},
        PartLoadContract{10000.0, 12000.0, 10000.0, 0.30, 1.0, 1.00, 1.0, 1.00},
    };

    for (auto const &point : points) {
        EXPECT_TRUE(checkPartLoadContract(point));
    }

    // Issue #8191 / current mode-2 signature: post-scaled PLR, full cycling, and a full-load EIRFPLR evaluation.
    PartLoadContract const issue8191Point{10000.0, 1000.0, 1000.0, 0.30, 1.0, 0.10, 1.0, 1.0};
    EXPECT_FALSE(checkPartLoadContract(issue8191Point));
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_SingleModeSolversUseFinalStateAndCloseEnergyBalances)
{
    std::string const idf_objects = delimited_string({
        "FluidProperties:GlycolConcentration,",
        "  Source Fluid,",
        "  PropyleneGlycol,",
        "  ,",
        "  0.30;",

        "Curve:Biquadratic,",
        "  Constant Temperature Modifier,",
        "  1.0,",
        "  0.0,",
        "  0.0,",
        "  0.0,",
        "  0.0,",
        "  0.0,",
        "  -100.0,",
        "  100.0,",
        "  -100.0,",
        "  100.0;",

        "Curve:Biquadratic,",
        "  Evaporator Temperature Capacity Modifier,",
        "  0.56,",
        "  0.05,",
        "  0.0,",
        "  0.0,",
        "  0.0,",
        "  0.0,",
        "  -100.0,",
        "  100.0,",
        "  -100.0,",
        "  100.0;",

        "Curve:Quadratic,",
        "  Linear Part Load EIR,",
        "  0.0,",
        "  1.0,",
        "  0.0,",
        "  0.3,",
        "  1.0;",

        "Curve:Bicubic,",
        "  Bivariate Part Load EIR,",
        "  0.0,",
        "  0.0,",
        "  0.0,",
        "  1.0,",
        "  0.0,",
        "  0.0,",
        "  0.0,",
        "  0.0,",
        "  0.0,",
        "  0.0,",
        "  -100.0,",
        "  100.0,",
        "  0.3,",
        "  1.0;",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    EXPECT_FALSE(has_err_output());
    state->init_state(*state);

    state->dataPlnt->PlantLoop.allocate(3);
    auto *water = Fluid::GetWater(*state);
    auto *sourceGlycol = Fluid::GetGlycol(*state, "SOURCE FLUID");
    ASSERT_NE(nullptr, water);
    ASSERT_NE(nullptr, sourceGlycol);
    state->dataPlnt->PlantLoop(1).glycol = water;
    state->dataPlnt->PlantLoop(2).glycol = sourceGlycol;
    state->dataPlnt->PlantLoop(3).glycol = water;

    PlantCentralHeatPumpSystem::CentralHeatPumpSystem system;
    system.coolingPlantLoc.loopNum = 1;
    system.sourcePlantLoc.loopNum = 2;
    system.heatingPlantLoc.loopNum = 3;
    PlantUtilities::SetPlantLocationLinks(*state, system.coolingPlantLoc);
    PlantUtilities::SetPlantLocationLinks(*state, system.sourcePlantLoc);
    PlantUtilities::SetPlantLocationLinks(*state, system.heatingPlantLoc);

    state->dataLoopNodes->Node.allocate(8);
    system.coolingSetpointNodeNum = 1;
    system.heatingSetpointNodeNum = 2;
    system.coolingInletNodeNum = 3;
    system.coolingOutletNodeNum = 4;
    system.heatingInletNodeNum = 5;
    system.heatingOutletNodeNum = 6;
    system.sourceInletNodeNum = 7;
    system.sourceOutletNodeNum = 8;
    state->dataLoopNodes->Node(system.coolingInletNodeNum).Temp = 12.0;
    state->dataLoopNodes->Node(system.heatingInletNodeNum).Temp = 40.0;
    state->dataLoopNodes->Node(system.sourceInletNodeNum).Temp = 15.0;
    state->dataLoopNodes->Node(system.coolingSetpointNodeNum).TempSetPoint = 7.0;
    state->dataLoopNodes->Node(system.heatingSetpointNodeNum).TempSetPoint = 45.0;

    system.modules.resize(2);
    PlantCentralHeatPumpSystem::PerformanceData performance1;
    performance1.compressorMotorEfficiency = 0.80;
    performance1.minimumEvaporatorOutletTemp = 5.0;
    performance1.referenceCoolingCOP = 5.0;
    performance1.coolingCondenserTemperatureMode = PlantCentralHeatPumpSystem::CondenserTemperatureMode::EnteringCondenser;
    performance1.coolingCapacityTemperatureCurveIndex = Curve::GetCurveIndex(*state, "CONSTANT TEMPERATURE MODIFIER");
    performance1.coolingEIRTemperatureCurveIndex = performance1.coolingCapacityTemperatureCurveIndex;
    performance1.coolingEIRPartLoadCurveIndex = Curve::GetCurveIndex(*state, "LINEAR PART LOAD EIR");
    performance1.coolingMinimumPartLoadRatio = 0.3;
    performance1.coolingMaximumPartLoadRatio = 1.0;
    performance1.heatingCondenserTemperatureMode = PlantCentralHeatPumpSystem::CondenserTemperatureMode::EnteringCondenser;
    performance1.heatingCapacityTemperatureCurveIndex = performance1.coolingCapacityTemperatureCurveIndex;
    performance1.heatingEIRTemperatureCurveIndex = performance1.coolingEIRTemperatureCurveIndex;
    performance1.heatingEIRPartLoadCurveIndex = Curve::GetCurveIndex(*state, "BIVARIATE PART LOAD EIR");
    performance1.heatingMinimumPartLoadRatio = 0.3;
    performance1.heatingMaximumPartLoadRatio = 1.0;

    ASSERT_GT(performance1.coolingCapacityTemperatureCurveIndex, 0);
    ASSERT_GT(performance1.coolingEIRPartLoadCurveIndex, 0);
    ASSERT_GT(performance1.heatingEIRPartLoadCurveIndex, 0);

    system.modules[0].initialize(performance1, nullptr);
    auto &chillerHeater = system.modules[0];
    chillerHeater.sizing.referenceCoolingCapacity = 10000.0;
    chillerHeater.sizing.referenceHeatingCapacity = 10000.0;
    chillerHeater.sizing.referenceHeatingCOP = 4.0;
    chillerHeater.minimumEvaporatorOutletTemp = 5.0;

    auto coolingResult = system.solveCoolingOnly(*state, 0, 1000.0, 1.0, 1.0, 12.0, 30.0);

    EXPECT_EQ(CurrentMode::CoolingOnly, coolingResult.currentMode);
    EXPECT_NEAR(1000.0, coolingResult.qEvaporator, 1.0e-6);
    EXPECT_NEAR(200.0, coolingResult.coolingPower, 1.0e-6);
    EXPECT_NEAR(1160.0, coolingResult.qCondenser, 1.0e-6);
    EXPECT_NEAR(0.30, coolingResult.partLoadRatio, 1.0e-9);
    EXPECT_NEAR(1.0 / 3.0, coolingResult.cyclingRatio, 1.0e-9);
    EXPECT_NEAR(coolingResult.partLoadRatio, coolingResult.eirPartLoadCurvePLR, 1.0e-12);
    EXPECT_NEAR(coolingResult.evaporatorOutletTemp, coolingResult.capacityCurveEvaporatorTemp, 1.0e-12);
    EXPECT_NEAR(30.0, coolingResult.capacityCurveCondenserTemp, 1.0e-12);
    EXPECT_NE(7.0, coolingResult.evaporatorOutletTemp);
    EXPECT_NEAR(0.0, coolingResult.moduleEnergyBalanceResidual(), 1.0e-9);
    EXPECT_EQ(PlantCentralHeatPumpSystem::SolverConvergenceStatus::Converged, coolingResult.solver.outerStatus);
    EXPECT_EQ(PlantCentralHeatPumpSystem::SolverConvergenceStatus::NotRequired, coolingResult.solver.partLoadStatus);
    EXPECT_GT(coolingResult.solver.outerIterations, 0);
    EXPECT_EQ(0, coolingResult.solver.partLoadIterations);
    EXPECT_EQ(3 * coolingResult.solver.outerIterations + 4, coolingResult.solver.curveEvaluations);
    chillerHeater.result = coolingResult;
    chillerHeater.mapResultToPlantConnections();
    EXPECT_NEAR(0.0, chillerHeater.result.routingEnergyBalanceResidual(), 1.0e-9);

    system.allModulesVariableFlow = true;
    performance1.coolingCondenserTemperatureMode = PlantCentralHeatPumpSystem::CondenserTemperatureMode::LeavingCondenser;
    coolingResult = system.solveCoolingOnly(*state, 0, 1000.0, 1.0, 1.0, 12.0, 30.0);
    Real64 const coolingCp = water->getSpecificHeat(*state, 12.0, "PlantCentralHeatPumpSystem solver test");
    EXPECT_NEAR(7.0, coolingResult.evaporatorOutletTemp, 1.0e-9);
    EXPECT_NEAR(1000.0 / (coolingCp * 5.0), coolingResult.evaporatorMassFlowRate, 1.0e-9);
    EXPECT_NEAR(coolingResult.condenserOutletTemp, coolingResult.capacityCurveCondenserTemp, 1.0e-12);
    EXPECT_NEAR(0.0, coolingResult.moduleEnergyBalanceResidual(), 1.0e-9);

    auto const tinyHeatingResult = system.solveHeatingOnly(*state, 0, 0.5 * HVAC::SmallLoad, 1.0, 1.0, 15.0, 40.0);
    EXPECT_EQ(PlantCentralHeatPumpSystem::SolverConvergenceStatus::NotRequired, tinyHeatingResult.solver.outerStatus);
    EXPECT_EQ(PlantCentralHeatPumpSystem::SolverConvergenceStatus::NotRequired, tinyHeatingResult.solver.partLoadStatus);
    EXPECT_EQ(0, tinyHeatingResult.solver.outerIterations);
    EXPECT_EQ(0, tinyHeatingResult.solver.partLoadIterations);
    EXPECT_EQ(0, tinyHeatingResult.solver.curveEvaluations);

    auto heatingResult = system.solveHeatingOnly(*state, 0, 1200.0, 1.0, 1.0, 15.0, 40.0);
    EXPECT_EQ(CurrentMode::HeatingOnly, heatingResult.currentMode);
    EXPECT_NEAR(1000.0, heatingResult.qEvaporator, 1.0e-6);
    EXPECT_NEAR(250.0, heatingResult.heatingPower, 1.0e-6);
    EXPECT_NEAR(1200.0, heatingResult.qCondenser, 1.0e-6);
    EXPECT_NEAR(0.30, heatingResult.partLoadRatio, 1.0e-9);
    EXPECT_NEAR(1.0 / 3.0, heatingResult.cyclingRatio, 1.0e-9);
    EXPECT_NEAR(heatingResult.partLoadRatio, heatingResult.eirPartLoadCurvePLR, 1.0e-12);
    EXPECT_NEAR(40.0, heatingResult.eirPartLoadCurveCondenserTemp, 1.0e-12);
    EXPECT_NEAR(heatingResult.evaporatorOutletTemp, heatingResult.capacityCurveEvaporatorTemp, 1.0e-12);
    EXPECT_NEAR(40.0, heatingResult.capacityCurveCondenserTemp, 1.0e-12);
    EXPECT_NEAR(45.0, heatingResult.condenserOutletTemp, 1.0e-9);
    EXPECT_NEAR(0.0, heatingResult.moduleEnergyBalanceResidual(), 1.0e-9);
    EXPECT_EQ(PlantCentralHeatPumpSystem::SolverConvergenceStatus::NotRequired, heatingResult.solver.partLoadStatus);
    chillerHeater.result = heatingResult;
    chillerHeater.mapResultToPlantConnections();
    EXPECT_NEAR(0.0, chillerHeater.result.routingEnergyBalanceResidual(), 1.0e-9);

    auto simultaneousResult = system.solveSimultaneous(*state, 0, 1000.0, 1200.0, 1.0, 1.0, 1.0, 12.0, 40.0, 15.0);
    EXPECT_EQ(CurrentMode::HeatRecovery, simultaneousResult.currentMode);
    EXPECT_NEAR(1000.0, simultaneousResult.qEvaporator, 1.0e-6);
    EXPECT_NEAR(250.0, simultaneousResult.compressorPower, 1.0e-6);
    EXPECT_NEAR(1200.0, simultaneousResult.qCondenser, 1.0e-6);
    EXPECT_NEAR(1000.0, simultaneousResult.coolingDelivered, 1.0e-6);
    EXPECT_NEAR(1200.0, simultaneousResult.heatingDelivered, 1.0e-6);
    EXPECT_NEAR(0.0, simultaneousResult.sourceHeatTransfer, 1.0e-6);
    EXPECT_NEAR(0.0, simultaneousResult.moduleEnergyBalanceResidual(), 1.0e-9);
    EXPECT_NEAR(0.0, simultaneousResult.routingEnergyBalanceResidual(), 1.0e-9);

    simultaneousResult = system.solveSimultaneous(*state, 0, 1000.0, 600.0, 1.0, 1.0, 1.0, 12.0, 40.0, 15.0);
    EXPECT_EQ(CurrentMode::CoolingDominant, simultaneousResult.currentMode);
    EXPECT_NEAR(1000.0, simultaneousResult.qEvaporator, 1.0e-6);
    EXPECT_NEAR(1200.0, simultaneousResult.qCondenser, 1.0e-6);
    EXPECT_NEAR(1000.0, simultaneousResult.coolingDelivered, 1.0e-6);
    EXPECT_NEAR(600.0, simultaneousResult.heatingDelivered, 1.0e-6);
    EXPECT_NEAR(600.0, simultaneousResult.sourceHeatTransfer, 1.0e-6);
    EXPECT_NEAR(0.5, simultaneousResult.sourceMassFlowRate, 1.0e-9);
    EXPECT_GT(simultaneousResult.capacityCurveCondenserTemp, 15.0);
    EXPECT_LT(simultaneousResult.capacityCurveCondenserTemp, 40.0);
    EXPECT_NEAR(0.0, simultaneousResult.moduleEnergyBalanceResidual(), 1.0e-9);
    EXPECT_NEAR(0.0, simultaneousResult.routingEnergyBalanceResidual(), 1.0e-9);

    simultaneousResult = system.solveSimultaneous(*state, 0, 1000.0, 600.0, 1.0, 1.0, 0.0, 12.0, 40.0, 15.0);
    EXPECT_EQ(CurrentMode::HeatRecovery, simultaneousResult.currentMode);
    EXPECT_NEAR(500.0, simultaneousResult.qEvaporator, 1.0e-6);
    EXPECT_NEAR(600.0, simultaneousResult.qCondenser, 1.0e-6);
    EXPECT_NEAR(500.0, simultaneousResult.coolingDelivered, 1.0e-6);
    EXPECT_NEAR(600.0, simultaneousResult.heatingDelivered, 1.0e-6);
    EXPECT_NEAR(500.0, simultaneousResult.unmetCoolingLoad, 1.0e-6);
    EXPECT_NEAR(0.0, simultaneousResult.sourceHeatTransfer, 1.0e-6);
    EXPECT_NEAR(0.0, simultaneousResult.moduleEnergyBalanceResidual(), 1.0e-9);
    EXPECT_NEAR(0.0, simultaneousResult.routingEnergyBalanceResidual(), 1.0e-9);

    simultaneousResult = system.solveSimultaneous(*state, 0, 500.0, 1200.0, 1.0, 1.0, 1.0, 12.0, 40.0, 15.0);
    EXPECT_EQ(CurrentMode::HeatingDominant, simultaneousResult.currentMode);
    EXPECT_NEAR(1000.0, simultaneousResult.qEvaporator, 1.0e-6);
    EXPECT_NEAR(1200.0, simultaneousResult.qCondenser, 1.0e-6);
    EXPECT_NEAR(500.0, simultaneousResult.coolingDelivered, 1.0e-6);
    EXPECT_NEAR(1200.0, simultaneousResult.heatingDelivered, 1.0e-6);
    EXPECT_NEAR(-500.0, simultaneousResult.sourceHeatTransfer, 1.0e-6);
    EXPECT_GT(simultaneousResult.capacityCurveEvaporatorTemp, performance1.minimumEvaporatorOutletTemp);
    EXPECT_LT(simultaneousResult.capacityCurveEvaporatorTemp, 7.0);
    EXPECT_NEAR(0.0, simultaneousResult.moduleEnergyBalanceResidual(), 1.0e-9);
    EXPECT_NEAR(0.0, simultaneousResult.routingEnergyBalanceResidual(), 1.0e-9);
    EXPECT_EQ(PlantCentralHeatPumpSystem::SolverConvergenceStatus::Converged, simultaneousResult.solver.outerStatus);
    EXPECT_EQ(PlantCentralHeatPumpSystem::SolverConvergenceStatus::Converged, simultaneousResult.solver.partLoadStatus);
    EXPECT_GT(simultaneousResult.solver.partLoadIterations, 0);
    EXPECT_LE(simultaneousResult.solver.partLoadIterations, 50 * simultaneousResult.solver.outerIterations);
    EXPECT_LT(simultaneousResult.solver.partLoadIterations, 80 * simultaneousResult.solver.outerIterations);
    EXPECT_LE(simultaneousResult.solver.partLoadBracketWidth, 1.0e-12);
    EXPECT_LE(simultaneousResult.solver.loadResidual, 1.0e-7);
    EXPECT_GT(simultaneousResult.solver.curveEvaluations, 0);

    int const constantHeatingCapacityCurveIndex = performance1.heatingCapacityTemperatureCurveIndex;
    performance1.heatingCapacityTemperatureCurveIndex = Curve::GetCurveIndex(*state, "EVAPORATOR TEMPERATURE CAPACITY MODIFIER");
    ASSERT_GT(performance1.heatingCapacityTemperatureCurveIndex, 0);
    simultaneousResult = system.solveSimultaneous(*state, 0, 9000.0, 20000.0, 1.0, 1.0, 1.0, 12.0, 40.0, 5.2);
    Real64 const boundarySourceCp = sourceGlycol->getSpecificHeat(*state, 5.2, "PlantCentralHeatPumpSystem solver test");
    EXPECT_EQ(CurrentMode::HeatingDominant, simultaneousResult.currentMode);
    EXPECT_EQ(PlantCentralHeatPumpSystem::SolverConvergenceStatus::Converged, simultaneousResult.solver.outerStatus);
    EXPECT_LT(simultaneousResult.sourceHeatTransfer, -HVAC::SmallLoad);
    EXPECT_GT(simultaneousResult.sourceMassFlowRate, 0.0);
    EXPECT_LT(simultaneousResult.sourceMassFlowRate, 1.0);
    EXPECT_NEAR(-simultaneousResult.sourceHeatTransfer, simultaneousResult.sourceMassFlowRate * boundarySourceCp * (5.2 - 5.0), 1.0e-6);
    EXPECT_NEAR(5.0, simultaneousResult.sourceOutletTemp, 1.0e-9);
    EXPECT_NEAR(0.0, simultaneousResult.moduleEnergyBalanceResidual(), 1.0e-9);
    EXPECT_NEAR(0.0, simultaneousResult.routingEnergyBalanceResidual(), 1.0e-9);
    performance1.heatingCapacityTemperatureCurveIndex = constantHeatingCapacityCurveIndex;

    system.allModulesVariableFlow = false;
    performance1.heatingCondenserTemperatureMode = PlantCentralHeatPumpSystem::CondenserTemperatureMode::LeavingCondenser;
    performance1.heatingReferenceCondenserOutletTemp = 40.5;
    heatingResult = system.solveHeatingOnly(*state, 0, 5000.0, 1.0, 1.0, 15.0, 40.0);
    EXPECT_GT(heatingResult.condenserOutletTemp, performance1.heatingReferenceCondenserOutletTemp);
    EXPECT_NEAR(heatingResult.condenserOutletTemp, heatingResult.capacityCurveCondenserTemp, 1.0e-12);
    EXPECT_NEAR(heatingResult.condenserOutletTemp, heatingResult.eirPartLoadCurveCondenserTemp, 1.0e-12);
    EXPECT_NEAR(0.0, heatingResult.unmetHeatingLoad, 1.0e-6);
    EXPECT_NEAR(0.0, heatingResult.moduleEnergyBalanceResidual(), 1.0e-9);
    EXPECT_EQ(PlantCentralHeatPumpSystem::SolverConvergenceStatus::Converged, heatingResult.solver.outerStatus);
    EXPECT_EQ(PlantCentralHeatPumpSystem::SolverConvergenceStatus::Converged, heatingResult.solver.partLoadStatus);
    EXPECT_GT(heatingResult.solver.partLoadIterations, 0);
    EXPECT_LE(heatingResult.solver.partLoadIterations, 50 * heatingResult.solver.outerIterations);
    EXPECT_LT(heatingResult.solver.partLoadIterations, 80 * heatingResult.solver.outerIterations);
    EXPECT_LE(heatingResult.solver.partLoadBracketWidth, 1.0e-12);
    EXPECT_LE(heatingResult.solver.loadResidual, 1.0e-7);
    EXPECT_GT(heatingResult.solver.curveEvaluations, 0);

    performance1.coolingCondenserTemperatureMode = PlantCentralHeatPumpSystem::CondenserTemperatureMode::EnteringCondenser;
    performance1.heatingCondenserTemperatureMode = PlantCentralHeatPumpSystem::CondenserTemperatureMode::EnteringCondenser;
    PlantCentralHeatPumpSystem::PerformanceData performance2 = performance1;
    system.modules[1].initialize(performance2, nullptr);
    chillerHeater.sizing.maximumEvaporatorMassFlowRate = 1.0;
    chillerHeater.sizing.maximumCondenserMassFlowRate = 1.0;
    system.modules[1].sizing.referenceCoolingCapacity = 5000.0;
    system.modules[1].sizing.referenceHeatingCapacity = 5000.0;
    system.modules[1].sizing.referenceHeatingCOP = 4.0;
    system.modules[1].sizing.maximumEvaporatorMassFlowRate = 1.0;
    system.modules[1].sizing.maximumCondenserMassFlowRate = 1.0;
    system.requestedCoolingLoad = 18000.0;
    system.requestedHeatingLoad = 21600.0;

    system.calculateSimultaneous(*state, 2.0, 2.0, 2.0, 12.0, 40.0, 15.0);
    EXPECT_FALSE(system.isCoolingDominant);
    EXPECT_FALSE(system.isHeatingDominant);
    EXPECT_EQ(CurrentMode::HeatRecovery, system.modules[0].result.currentMode);
    EXPECT_EQ(CurrentMode::HeatRecovery, system.modules[1].result.currentMode);
    EXPECT_NEAR(10000.0, system.modules[0].result.coolingDelivered, 1.0e-6);
    EXPECT_NEAR(12000.0, system.modules[0].result.heatingDelivered, 1.0e-6);
    EXPECT_NEAR(8000.0, system.modules[1].result.requestedCoolingLoad, 1.0e-6);
    EXPECT_NEAR(9600.0, system.modules[1].result.requestedHeatingLoad, 1.0e-6);
    EXPECT_NEAR(5000.0, system.modules[1].result.coolingDelivered, 1.0e-6);
    EXPECT_NEAR(6000.0, system.modules[1].result.heatingDelivered, 1.0e-6);
    EXPECT_NEAR(3000.0, system.modules[1].result.unmetCoolingLoad, 1.0e-6);
    EXPECT_NEAR(3600.0, system.modules[1].result.unmetHeatingLoad, 1.0e-6);
    EXPECT_NEAR(15000.0, system.report.coolingHeatTransferRate, 1.0e-6);
    EXPECT_NEAR(18000.0, system.report.heatingHeatTransferRate, 1.0e-6);
    EXPECT_NEAR(0.0, system.report.sourceHeatTransferRate, 1.0e-6);
    EXPECT_NEAR(3750.0, system.report.coolingElectricPower + system.report.heatingElectricPower, 1.0e-6);

    auto const firstDispatchResult = system.modules[1].result;
    system.calculateSimultaneous(*state, 2.0, 2.0, 2.0, 12.0, 40.0, 15.0);
    EXPECT_EQ(firstDispatchResult.currentMode, system.modules[1].result.currentMode);
    EXPECT_NEAR(firstDispatchResult.qEvaporator, system.modules[1].result.qEvaporator, 1.0e-9);
    EXPECT_NEAR(firstDispatchResult.qCondenser, system.modules[1].result.qCondenser, 1.0e-9);
    EXPECT_NEAR(firstDispatchResult.unmetCoolingLoad, system.modules[1].result.unmetCoolingLoad, 1.0e-9);
    EXPECT_NEAR(firstDispatchResult.unmetHeatingLoad, system.modules[1].result.unmetHeatingLoad, 1.0e-9);

    auto configureConnectionPerformance = [&system](std::size_t const moduleIndex, Real64 const referenceCapacity) {
        auto &module = system.modules[moduleIndex];
        module.sizing.referenceCoolingCapacity = referenceCapacity;
        module.sizing.referenceHeatingCapacity = referenceCapacity;
        module.sizing.referenceHeatingCOP = 4.0;
        module.sizing.maximumCoolingMassFlowRate = 0.20;
        module.sizing.maximumHeatingMassFlowRate = 0.10;
        module.sizing.maximumSourceEvaporatorMassFlowRate = 0.12;
        module.sizing.maximumSourceCondenserMassFlowRate = 0.15;
    };
    configureConnectionPerformance(0, 10000.0);
    configureConnectionPerformance(1, 5000.0);

    system.requestedCoolingLoad = 20000.0;
    system.requestedHeatingLoad = 0.0;
    system.calculateCoolingOnly(*state, 0.30, 0.25, 12.0, 15.0);
    auto const &coolingModule1 = system.modules[0].result;
    auto const &coolingModule2 = system.modules[1].result;
    EXPECT_EQ(CurrentMode::CoolingOnly, coolingModule1.currentMode);
    EXPECT_EQ(CurrentMode::CoolingOnly, coolingModule2.currentMode);
    EXPECT_NEAR(0.20, coolingModule1.coolingMassFlowRate, 1.0e-12);
    EXPECT_NEAR(0.10, coolingModule2.coolingMassFlowRate, 1.0e-12);
    EXPECT_NEAR(0.15, coolingModule1.sourceMassFlowRate, 1.0e-12);
    EXPECT_NEAR(0.10, coolingModule2.sourceMassFlowRate, 1.0e-12);
    EXPECT_LE(coolingModule1.coolingMassFlowRate + coolingModule2.coolingMassFlowRate, 0.30);
    EXPECT_LE(coolingModule1.sourceMassFlowRate + coolingModule2.sourceMassFlowRate, 0.25);
    EXPECT_NEAR(7.0, system.report.coolingOutletTemp, 1.0e-9);
    Real64 const sourceCp = sourceGlycol->getSpecificHeat(*state, 15.0, "PlantCentralHeatPumpSystem connection test");
    EXPECT_NEAR(system.report.sourceHeatTransferRate, 0.25 * sourceCp * (system.report.sourceOutletTemp - system.report.sourceInletTemp), 1.0e-6);
    EXPECT_NEAR(0.0, coolingModule1.routingEnergyBalanceResidual(), 1.0e-9);
    EXPECT_NEAR(0.0, coolingModule2.routingEnergyBalanceResidual(), 1.0e-9);

    system.allModulesVariableFlow = true;
    system.requestedCoolingLoad = 1000.0;
    system.calculateCoolingOnly(*state, 0.30, 0.25, 12.0, 15.0);
    Real64 const expectedVariableCoolingFlow = 1000.0 / (coolingCp * 5.0);
    EXPECT_NEAR(expectedVariableCoolingFlow, system.modules[0].result.coolingMassFlowRate, 1.0e-9);
    EXPECT_NEAR(0.0, system.modules[1].result.coolingMassFlowRate, 1.0e-12);
    EXPECT_NEAR(12.0 - 1000.0 / (0.30 * coolingCp), system.report.coolingOutletTemp, 1.0e-9);
    EXPECT_NEAR(system.report.sourceHeatTransferRate, 0.25 * sourceCp * (system.report.sourceOutletTemp - system.report.sourceInletTemp), 1.0e-6);

    system.allModulesVariableFlow = false;
    system.requestedCoolingLoad = 0.0;
    system.requestedHeatingLoad = 20000.0;
    system.calculateHeatingOnly(*state, 0.15, 0.18, 40.0, 15.0);
    auto const &heatingModule1 = system.modules[0].result;
    auto const &heatingModule2 = system.modules[1].result;
    EXPECT_EQ(CurrentMode::HeatingOnly, heatingModule1.currentMode);
    EXPECT_EQ(CurrentMode::HeatingOnly, heatingModule2.currentMode);
    EXPECT_NEAR(0.10, heatingModule1.heatingMassFlowRate, 1.0e-12);
    EXPECT_NEAR(0.05, heatingModule2.heatingMassFlowRate, 1.0e-12);
    EXPECT_NEAR(0.12, heatingModule1.sourceMassFlowRate, 1.0e-12);
    EXPECT_NEAR(0.06, heatingModule2.sourceMassFlowRate, 1.0e-12);
    EXPECT_LE(heatingModule1.heatingMassFlowRate + heatingModule2.heatingMassFlowRate, 0.15);
    EXPECT_LE(heatingModule1.sourceMassFlowRate + heatingModule2.sourceMassFlowRate, 0.18);
    EXPECT_NEAR(45.0, system.report.heatingOutletTemp, 1.0e-9);
    EXPECT_NEAR(-system.report.sourceHeatTransferRate, 0.18 * sourceCp * (system.report.sourceInletTemp - system.report.sourceOutletTemp), 1.0e-6);
    EXPECT_NEAR(0.0, heatingModule1.routingEnergyBalanceResidual(), 1.0e-9);
    EXPECT_NEAR(0.0, heatingModule2.routingEnergyBalanceResidual(), 1.0e-9);

    system.requestedCoolingLoad = 1000.0;
    system.requestedHeatingLoad = 500.0;
    system.calculateSimultaneous(*state, 0.20, 0.10, 0.30, 12.0, 40.0, 15.0);
    EXPECT_EQ(CurrentMode::CoolingDominant, system.modules[0].result.currentMode);
    EXPECT_NEAR(0.15, system.modules[0].result.sourceMassFlowRate, 1.0e-12);

    system.requestedCoolingLoad = 500.0;
    system.requestedHeatingLoad = 1200.0;
    system.calculateSimultaneous(*state, 0.20, 0.10, 0.30, 12.0, 40.0, 15.0);
    EXPECT_EQ(CurrentMode::HeatingDominant, system.modules[0].result.currentMode);
    EXPECT_NEAR(0.12, system.modules[0].result.sourceMassFlowRate, 1.0e-12);

    auto &overAllocatedResult1 = system.modules[0].result;
    auto &overAllocatedResult2 = system.modules[1].result;
    overAllocatedResult1 = PlantCentralHeatPumpSystem::ModuleResult();
    overAllocatedResult2 = PlantCentralHeatPumpSystem::ModuleResult();
    overAllocatedResult1.heatingMassFlowRate = 0.20;
    overAllocatedResult1.heatingOutletTemp = 46.0;
    overAllocatedResult2.heatingMassFlowRate = 0.10;
    overAllocatedResult2.heatingOutletTemp = 44.0;
    system.updateReportingAndNodes(*state, 0.0, 0.15, 0.0, 12.0, 40.0, 15.0);
    EXPECT_NEAR((0.20 * 46.0 + 0.10 * 44.0) / 0.30, system.report.heatingOutletTemp, 1.0e-12);
    EXPECT_NE(40.0, system.report.heatingOutletTemp);

    system.modules[0].sizing.designEvaporatorVolFlowRate = 0.0010;
    system.modules[0].sizing.designCondenserVolFlowRate = 0.0005;
    performance1.designHeatingVolFlowRate = 0.0003;
    system.modules[1].sizing.designEvaporatorVolFlowRate = 0.0020;
    system.modules[1].sizing.designCondenserVolFlowRate = 0.0025;
    performance2.designHeatingVolFlowRate = 0.0004;
    system.initializeDesignFlowLimits(*state);

    Real64 const coolingDensity = water->getDensity(*state, Constant::CWInitConvTemp, "PlantCentralHeatPumpSystem design flow test");
    Real64 const heatingDensity = water->getDensity(*state, Constant::HWInitConvTemp, "PlantCentralHeatPumpSystem design flow test");
    Real64 const sourceDensity = sourceGlycol->getDensity(*state, Constant::CWInitConvTemp, "PlantCentralHeatPumpSystem design flow test");
    EXPECT_NEAR(0.0030, system.coolingVolFlowRate, 1.0e-12);
    EXPECT_NEAR(0.0007, system.heatingVolFlowRate, 1.0e-12);
    EXPECT_NEAR(0.0035, system.sourceVolFlowRate, 1.0e-12);
    EXPECT_NEAR(0.0030 * coolingDensity, system.coolingMassFlowRateMax, 1.0e-9);
    EXPECT_NEAR(0.0007 * heatingDensity, system.heatingMassFlowRateMax, 1.0e-9);
    EXPECT_NEAR(0.0035 * sourceDensity, system.sourceMassFlowRateMax, 1.0e-9);
    EXPECT_NEAR(0.0010 * coolingDensity, system.modules[0].sizing.maximumCoolingMassFlowRate, 1.0e-9);
    EXPECT_NEAR(0.0003 * heatingDensity, system.modules[0].sizing.maximumHeatingMassFlowRate, 1.0e-9);
    EXPECT_NEAR(0.0010 * sourceDensity, system.modules[0].sizing.maximumSourceEvaporatorMassFlowRate, 1.0e-9);
    EXPECT_NEAR(0.0005 * sourceDensity, system.modules[0].sizing.maximumSourceCondenserMassFlowRate, 1.0e-9);
    EXPECT_NEAR(system.coolingMassFlowRateMax, state->dataLoopNodes->Node(system.coolingInletNodeNum).MassFlowRateMax, 1.0e-9);
    EXPECT_NEAR(system.heatingMassFlowRateMax, state->dataLoopNodes->Node(system.heatingInletNodeNum).MassFlowRateMax, 1.0e-9);
    EXPECT_NEAR(system.sourceMassFlowRateMax, state->dataLoopNodes->Node(system.sourceInletNodeNum).MassFlowRateMax, 1.0e-9);

    auto *adversarialPartLoadCurve = state->dataCurveManager->curves(performance1.heatingEIRPartLoadCurveIndex);
    adversarialPartLoadCurve->coeff.fill(0.0);
    constexpr Real64 steepPartLoadSlope = 1.0e10;
    adversarialPartLoadCurve->coeff[0] = -performance1.heatingMinimumPartLoadRatio * steepPartLoadSlope;
    adversarialPartLoadCurve->coeff[3] = steepPartLoadSlope;
    auto const exhaustedResult = system.solveHeatingOnly(*state, 0, 5000.0, 1.0, 1.0, 15.0, 40.0);
    EXPECT_EQ(PlantCentralHeatPumpSystem::SolverConvergenceStatus::IterationLimit, exhaustedResult.solver.partLoadStatus);
    EXPECT_EQ(PlantCentralHeatPumpSystem::SolverConvergenceStatus::Converged, exhaustedResult.solver.outerStatus);
    EXPECT_EQ(50 * exhaustedResult.solver.outerIterations, exhaustedResult.solver.partLoadIterations);
    EXPECT_GT(exhaustedResult.solver.loadResidual, 1.0e-7);
    auto const repeatedExhaustedResult = system.solveHeatingOnly(*state, 0, 5000.0, 1.0, 1.0, 15.0, 40.0);
    EXPECT_EQ(PlantCentralHeatPumpSystem::SolverConvergenceStatus::IterationLimit, repeatedExhaustedResult.solver.partLoadStatus);
    EXPECT_EQ(2, system.modules[0].heatingPartLoadSolverWarning.count);
    EXPECT_GT(system.modules[0].heatingPartLoadSolverWarning.recurringIndex, 0);
    EXPECT_EQ(0, system.modules[0].heatingSolverWarning.count);
    EXPECT_NEAR(0.0, exhaustedResult.moduleEnergyBalanceResidual(), 1.0e-9);
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_SequentialFlowAllocationContracts)
{
    FlowAllocationContract const variableFlow{1.20, {0.40, 0.80}, {0.75, 1.00}, {0.40, 0.80}};
    EXPECT_TRUE(checkSequentialFlowAllocation(variableFlow));

    FlowAllocationContract const heterogeneousConstantFlow{1.50, {0.80, 0.80, 0.80}, {1.00, 0.50, 1.00}, {0.80, 0.50, 0.20}};
    EXPECT_TRUE(checkSequentialFlowAllocation(heterogeneousConstantFlow));

    FlowAllocationContract const overAllocated{1.50, {0.80, 0.80, 0.80}, {1.00, 0.50, 1.00}, {0.80, 0.50, 0.80}};
    EXPECT_FALSE(checkSequentialFlowAllocation(overAllocated));
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_WaterAndGlycolNodeHeatTransferContracts)
{
    std::string const idf_objects = delimited_string({"FluidProperties:GlycolConcentration,",
                                                      "  GLHXFluid,        !- Name",
                                                      "  PropyleneGlycol, !- Glycol Type",
                                                      "  ,                 !- User Defined Glycol Name",
                                                      "  0.3;              !- Glycol Concentration"});

    ASSERT_TRUE(process_idf(idf_objects));
    EXPECT_FALSE(has_err_output());
    state->init_state(*state);

    auto *water = Fluid::GetWater(*state);
    auto *sourceGlycol = Fluid::GetGlycol(*state, "GLHXFLUID");
    ASSERT_NE(nullptr, water);
    ASSERT_NE(nullptr, sourceGlycol);

    Real64 constexpr coolingInletTemp = 12.0;
    Real64 constexpr coolingOutletTemp = 7.0;
    Real64 constexpr coolingMassFlow = 1.0;
    Real64 const coolingCp = water->getSpecificHeat(*state, coolingInletTemp, "PlantCentralHeatPumpSystem contract test");
    Real64 const cooling = coolingMassFlow * coolingCp * (coolingInletTemp - coolingOutletTemp);
    EXPECT_TRUE(checkLoopHeatTransfer(cooling, coolingMassFlow, coolingCp, coolingInletTemp, coolingOutletTemp));

    Real64 constexpr heatingInletTemp = 40.0;
    Real64 constexpr heatingOutletTemp = 45.0;
    Real64 constexpr heatingMassFlow = 0.8;
    Real64 const heatingCp = water->getSpecificHeat(*state, heatingInletTemp, "PlantCentralHeatPumpSystem contract test");
    Real64 const heating = heatingMassFlow * heatingCp * (heatingOutletTemp - heatingInletTemp);
    EXPECT_TRUE(checkLoopHeatTransfer(heating, heatingMassFlow, heatingCp, heatingInletTemp, heatingOutletTemp));

    Real64 constexpr sourceInletTemp = 15.0;
    Real64 constexpr sourceOutletTemp = 17.0;
    Real64 constexpr sourceMassFlow = 1.2;
    Real64 const sourceCp = sourceGlycol->getSpecificHeat(*state, sourceInletTemp, "PlantCentralHeatPumpSystem contract test");
    Real64 const sourceHeat = sourceMassFlow * sourceCp * (sourceOutletTemp - sourceInletTemp);
    EXPECT_TRUE(checkLoopHeatTransfer(sourceHeat, sourceMassFlow, sourceCp, sourceInletTemp, sourceOutletTemp));

    EXPECT_FALSE(contractNear(sourceCp, coolingCp));
    EXPECT_FALSE(checkLoopHeatTransfer(sourceHeat, sourceMassFlow, coolingCp, sourceInletTemp, sourceOutletTemp));
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_AuthoritativeResultMapsPlantConnections)
{
    PlantCentralHeatPumpSystem::PerformanceData performance;
    performance.compressorMotorEfficiency = 0.80;
    PlantCentralHeatPumpSystem::Module chillerHeater;
    chillerHeater.initialize(performance, nullptr);

    auto configureResult = [&chillerHeater](CurrentMode const mode) {
        auto &result = chillerHeater.result;
        result = PlantCentralHeatPumpSystem::ModuleResult();
        result.currentMode = mode;
        result.isAvailable = true;
        result.qEvaporator = 8000.0;
        result.qCondenser = 9600.0;
        result.coolingPower = 2000.0;
        result.evaporatorInletTemp = 12.0;
        result.evaporatorOutletTemp = 7.0;
        result.evaporatorMassFlowRate = 0.40;
        result.condenserInletTemp = 30.0;
        result.condenserOutletTemp = 35.0;
        result.condenserMassFlowRate = 0.50;
        chillerHeater.mapResultToPlantConnections();
    };

    std::array<CurrentMode, 2> const coolingModes = {CurrentMode::CoolingOnly, CurrentMode::CoolingDominant};
    for (CurrentMode const mode : coolingModes) {
        SCOPED_TRACE(modeName(mode));
        configureResult(mode);
        auto const &result = chillerHeater.result;
        EXPECT_TRUE(result.isAvailable);
        EXPECT_TRUE(result.isRunning);
        EXPECT_DOUBLE_EQ(8000.0, result.coolingDelivered);
        EXPECT_DOUBLE_EQ(9600.0, result.sourceHeatTransfer);
        EXPECT_DOUBLE_EQ(0.40, result.coolingMassFlowRate);
        EXPECT_DOUBLE_EQ(0.50, result.sourceMassFlowRate);
        EXPECT_DOUBLE_EQ(0.0, result.heatingMassFlowRate);
        EXPECT_NEAR(0.0, result.moduleEnergyBalanceResidual(), contractTolerance);
        EXPECT_NEAR(0.0, result.routingEnergyBalanceResidual(), contractTolerance);
    }

    std::array<CurrentMode, 2> const heatingModes = {CurrentMode::HeatingOnly, CurrentMode::HeatingDominant};
    for (CurrentMode const mode : heatingModes) {
        SCOPED_TRACE(modeName(mode));
        configureResult(mode);
        auto const &result = chillerHeater.result;
        EXPECT_DOUBLE_EQ(9600.0, result.heatingDelivered);
        EXPECT_DOUBLE_EQ(-8000.0, result.sourceHeatTransfer);
        EXPECT_DOUBLE_EQ(0.50, result.heatingMassFlowRate);
        EXPECT_DOUBLE_EQ(0.40, result.sourceMassFlowRate);
        EXPECT_DOUBLE_EQ(0.0, result.coolingMassFlowRate);
        EXPECT_NEAR(0.0, result.routingEnergyBalanceResidual(), contractTolerance);
    }

    configureResult(CurrentMode::HeatRecovery);
    auto const &result = chillerHeater.result;
    EXPECT_DOUBLE_EQ(8000.0, result.coolingDelivered);
    EXPECT_DOUBLE_EQ(9600.0, result.heatingDelivered);
    EXPECT_DOUBLE_EQ(9600.0, result.heatRecovered);
    EXPECT_DOUBLE_EQ(0.0, result.sourceHeatTransfer);
    EXPECT_DOUBLE_EQ(0.40, result.coolingMassFlowRate);
    EXPECT_DOUBLE_EQ(0.50, result.heatingMassFlowRate);
    EXPECT_DOUBLE_EQ(0.0, result.sourceMassFlowRate);
    EXPECT_DOUBLE_EQ(2000.0, result.compressorPower);
    EXPECT_DOUBLE_EQ(1600.0, result.motorHeatToRefrigerant);
    EXPECT_DOUBLE_EQ(400.0, result.motorHeatLoss);
    EXPECT_NEAR(0.0, result.moduleEnergyBalanceResidual(), contractTolerance);
    EXPECT_NEAR(0.0, result.routingEnergyBalanceResidual(), contractTolerance);
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_AuthoritativeResultDrivesEnergyIntegration)
{
    PlantCentralHeatPumpSystem::PerformanceData performance;
    performance.compressorMotorEfficiency = 0.80;
    PlantCentralHeatPumpSystem::Module chillerHeater;
    chillerHeater.initialize(performance, nullptr);
    auto &result = chillerHeater.result;
    result.currentMode = CurrentMode::HeatRecovery;
    result.isAvailable = true;
    result.qEvaporator = 7000.0;
    result.qCondenser = 7800.0;
    result.coolingPower = 1000.0;
    result.partLoadRatio = 0.75;
    result.cyclingRatio = 0.50;
    result.capacityTemperatureModifier = 0.95;
    result.eirTemperatureModifier = 1.05;
    result.eirPartLoadModifier = 0.90;
    result.actualCOP = 4.0;
    result.evaporatorInletTemp = 12.0;
    result.evaporatorOutletTemp = 8.0;
    result.evaporatorMassFlowRate = 0.40;
    result.condenserInletTemp = 30.0;
    result.condenserOutletTemp = 35.0;
    result.condenserMassFlowRate = 0.50;
    chillerHeater.mapResultToPlantConnections();
    chillerHeater.updateResultEnergies(60.0);

    EXPECT_DOUBLE_EQ(7000.0, result.qEvaporator);
    EXPECT_DOUBLE_EQ(7800.0, result.qCondenser);
    EXPECT_DOUBLE_EQ(1000.0, result.coolingPower);
    EXPECT_DOUBLE_EQ(60000.0, result.coolingEnergy);
    EXPECT_DOUBLE_EQ(420000.0, result.evaporatorEnergy);
    EXPECT_DOUBLE_EQ(468000.0, result.condenserEnergy);
    EXPECT_DOUBLE_EQ(8.0, result.coolingOutletTemp);
    EXPECT_DOUBLE_EQ(35.0, result.heatingOutletTemp);
    EXPECT_DOUBLE_EQ(7000.0, result.coolingDelivered);
    EXPECT_DOUBLE_EQ(7800.0, result.heatingDelivered);
    EXPECT_DOUBLE_EQ(1000.0, result.compressorPower);
    EXPECT_DOUBLE_EQ(800.0, result.motorHeatToRefrigerant);
    EXPECT_DOUBLE_EQ(200.0, result.motorHeatLoss);
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_AuthoritativeResultResetIsComplete)
{
    PlantCentralHeatPumpSystem::PerformanceData performance;
    performance.compressorMotorEfficiency = 0.80;
    PlantCentralHeatPumpSystem::Module chillerHeater;
    chillerHeater.initialize(performance, nullptr);
    chillerHeater.result.currentMode = CurrentMode::CoolingOnly;
    chillerHeater.result.isAvailable = true;
    chillerHeater.result.qEvaporator = 8000.0;
    chillerHeater.result.qCondenser = 9600.0;
    chillerHeater.result.coolingPower = 2000.0;
    chillerHeater.result.evaporatorMassFlowRate = 0.40;
    chillerHeater.result.condenserMassFlowRate = 0.50;
    chillerHeater.mapResultToPlantConnections();

    chillerHeater.resetResult(12.0, 30.0);

    EXPECT_EQ(CurrentMode::Off, chillerHeater.result.currentMode);
    EXPECT_FALSE(chillerHeater.result.isRunning);
    EXPECT_FALSE(chillerHeater.result.isAvailable);
    EXPECT_DOUBLE_EQ(0.0, chillerHeater.result.compressorPower);
    EXPECT_DOUBLE_EQ(0.0, chillerHeater.result.qEvaporator);
    EXPECT_DOUBLE_EQ(0.0, chillerHeater.result.qCondenser);
    EXPECT_DOUBLE_EQ(0.0, chillerHeater.result.evaporatorMassFlowRate);
    EXPECT_DOUBLE_EQ(0.0, chillerHeater.result.condenserMassFlowRate);
    EXPECT_DOUBLE_EQ(12.0, chillerHeater.result.evaporatorInletTemp);
    EXPECT_DOUBLE_EQ(12.0, chillerHeater.result.evaporatorOutletTemp);
    EXPECT_DOUBLE_EQ(30.0, chillerHeater.result.condenserInletTemp);
    EXPECT_DOUBLE_EQ(30.0, chillerHeater.result.condenserOutletTemp);
    EXPECT_DOUBLE_EQ(0.0, chillerHeater.result.coolingDelivered);
    EXPECT_DOUBLE_EQ(0.0, chillerHeater.result.heatingDelivered);
    EXPECT_DOUBLE_EQ(0.0, chillerHeater.result.sourceHeatTransfer);
}
