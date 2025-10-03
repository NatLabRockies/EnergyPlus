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

#include <EnergyPlus/Autosizing/All_Simple_Sizing.hh>
#include <EnergyPlus/Autosizing/CoolingAirFlowSizing.hh>
#include <EnergyPlus/Autosizing/CoolingCapacitySizing.hh>
#include <EnergyPlus/Coils/CoilCoolingDXCurveFitOperatingMode.hh>
#include <EnergyPlus/Coils/CoilCoolingDXCurveFitSpeed.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>

using namespace EnergyPlus;

void CoilCoolingDXCurveFitOperatingMode::instantiateFromInputSpec(EnergyPlus::EnergyPlusData &state,
                                                                  CoilCoolingDXCurveFitOperatingModeInputSpecification input_data)
{
    static constexpr std::string_view routineName("CoilCoolingDXCurveFitOperatingMode::instantiateFromInputSpec: ");
    bool errorsFound(false);
    this->original_input_specs = input_data;
    this->name = input_data.name;
    this->ratedGrossTotalCap = input_data.gross_rated_total_cooling_capacity;
    if (this->ratedGrossTotalCap == DataSizing::AutoSize) {
        this->ratedGrossTotalCapIsAutosized = true;
    }
    this->ratedEvapAirFlowRate = input_data.rated_evaporator_air_flow_rate;
    if (this->ratedEvapAirFlowRate == DataSizing::AutoSize) {
        this->ratedEvapAirFlowRateIsAutosized = true;
    }
    this->ratedCondAirFlowRate = input_data.rated_condenser_air_flow_rate;
    this->timeForCondensateRemoval = input_data.nominal_time_for_condensate_removal_to_begin;
    this->evapRateRatio = input_data.ratio_of_initial_moisture_evaporation_rate_and_steady_state_latent_capacity;
    this->maxCyclingRate = input_data.maximum_cycling_rate;
    this->latentTimeConst = input_data.latent_capacity_time_constant;
    if (Util::SameString(input_data.apply_latent_degradation_to_speeds_greater_than_1, "Yes")) {
        this->applyLatentDegradationAllSpeeds = true;
    } else {
        this->applyLatentDegradationAllSpeeds = false;
    }
    // TODO: UNUSED apply_latent_degradation_to_speeds_greater_than_1
    this->nominalEvaporativePumpPower = input_data.nominal_evap_condenser_pump_power;

    // Must all be greater than zero to use the latent capacity degradation model
    if ((this->maxCyclingRate > 0.0 || this->evapRateRatio > 0.0 || this->latentTimeConst > 0.0 || this->timeForCondensateRemoval > 0.0) &&
        (this->maxCyclingRate <= 0.0 || this->evapRateRatio <= 0.0 || this->latentTimeConst <= 0.0 || this->timeForCondensateRemoval <= 0.0)) {
        ShowWarningError(state, std::string{routineName} + this->object_name + "=\"" + this->name + "\":");
        ShowContinueError(state, "...At least one of the four input parameters for the latent capacity degradation model");
        ShowContinueError(state, "...is set to zero. Therefore, the latent degradation model will not be used for this simulation.");
        this->latentDegradationActive = false;
    } else if (this->maxCyclingRate > 0.0 && this->evapRateRatio > 0.0 && this->latentTimeConst > 0.0 && this->timeForCondensateRemoval > 0.0) {
        this->latentDegradationActive = true;
    }

    if (Util::SameString(input_data.condenser_type, "AirCooled")) {
        this->condenserType = CondenserType::AIRCOOLED;
    } else if (Util::SameString(input_data.condenser_type, "EvaporativelyCooled")) {
        this->condenserType = CondenserType::EVAPCOOLED;
    } else {
        ShowSevereError(state, std::string{routineName} + this->object_name + "=\"" + this->name + "\", invalid");
        ShowContinueError(state, "...Condenser Type=\"" + input_data.condenser_type + "\":");
        ShowContinueError(state, "...must be AirCooled or EvaporativelyCooled.");
        errorsFound = true;
    }
    for (auto &speed_name : input_data.speed_data_names) {
        this->speeds.emplace_back(state, speed_name);
    }

    // convert speed num in IDF to vector index
    this->nominalSpeedIndex = input_data.nominal_speed_number - 1;

    if (errorsFound) {
        ShowFatalError(
            state, std::string{routineName} + "Errors found in getting " + this->object_name + " input. Preceding condition(s) causes termination.");
    }
}

CoilCoolingDXCurveFitOperatingMode::CoilCoolingDXCurveFitOperatingMode(EnergyPlus::EnergyPlusData &state, const std::string &name_to_find)
{
    int numModes = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CoilCoolingDXCurveFitOperatingMode::object_name);
    if (numModes <= 0) {
        // error
    }
    bool found_it = false;
    for (int modeNum = 1; modeNum <= numModes; ++modeNum) {
        int NumAlphas;  // Number of Alphas for each GetObjectItem call
        int NumNumbers; // Number of Numbers for each GetObjectItem call
        int IOStatus;
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CoilCoolingDXCurveFitOperatingMode::object_name,
                                                                 modeNum,
                                                                 state.dataIPShortCut->cAlphaArgs,
                                                                 NumAlphas,
                                                                 state.dataIPShortCut->rNumericArgs,
                                                                 NumNumbers,
                                                                 IOStatus);
        if (!Util::SameString(name_to_find, state.dataIPShortCut->cAlphaArgs(1))) {
            continue;
        }
        found_it = true;

        CoilCoolingDXCurveFitOperatingModeInputSpecification input_specs;

        input_specs.name = state.dataIPShortCut->cAlphaArgs(1);
        input_specs.gross_rated_total_cooling_capacity = state.dataIPShortCut->rNumericArgs(1);
        input_specs.rated_evaporator_air_flow_rate = state.dataIPShortCut->rNumericArgs(2);
        input_specs.rated_condenser_air_flow_rate = state.dataIPShortCut->rNumericArgs(3);
        input_specs.maximum_cycling_rate = state.dataIPShortCut->rNumericArgs(4);
        input_specs.ratio_of_initial_moisture_evaporation_rate_and_steady_state_latent_capacity = state.dataIPShortCut->rNumericArgs(5);
        input_specs.latent_capacity_time_constant = state.dataIPShortCut->rNumericArgs(6);
        input_specs.nominal_time_for_condensate_removal_to_begin = state.dataIPShortCut->rNumericArgs(7);
        input_specs.apply_latent_degradation_to_speeds_greater_than_1 = state.dataIPShortCut->cAlphaArgs(2);
        input_specs.condenser_type = state.dataIPShortCut->cAlphaArgs(3);
        input_specs.nominal_evap_condenser_pump_power = state.dataIPShortCut->rNumericArgs(8);
        input_specs.nominal_speed_number = state.dataIPShortCut->rNumericArgs(9);
        for (int fieldNum = 4; fieldNum <= NumAlphas; fieldNum++) {
            if (state.dataIPShortCut->cAlphaArgs(fieldNum).empty()) {
                break;
            }
            input_specs.speed_data_names.push_back(state.dataIPShortCut->cAlphaArgs(fieldNum));
        }

        this->instantiateFromInputSpec(state, input_specs);
        break;
    }

    if (!found_it) {
        ShowFatalError(state, "Could not find Coil:Cooling:DX:CurveFit:OperatingMode object with name: " + name_to_find);
    }
}

void CoilCoolingDXCurveFitOperatingMode::oneTimeInit(EnergyPlus::EnergyPlusData &state)
{
    if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
        SetupEMSActuator(state,
                         this->object_name,
                         this->name,
                         "Autosized Rated Air Flow Rate",
                         "[m3/s]",
                         this->ratedAirVolFlowEMSOverrideON,
                         this->ratedAirVolFlowEMSOverrideValue);
        SetupEMSActuator(state,
                         this->object_name,
                         this->name,
                         "Autosized Rated Total Cooling Capacity",
                         "[W]",
                         this->ratedTotCapFlowEMSOverrideON,
                         this->ratedTotCapFlowEMSOverrideValue);
    }
}

void CoilCoolingDXCurveFitOperatingMode::size(EnergyPlus::EnergyPlusData &state)
{

    static constexpr std::string_view RoutineName = "sizeOperatingMode";
    std::string CompType = this->object_name;
    std::string CompName = this->name;
    bool PrintFlag = true;
    bool errorsFound = false;

    Real64 TempSize = this->original_input_specs.rated_evaporator_air_flow_rate;
    CoolingAirFlowSizer sizingCoolingAirFlow;
    std::string stringOverride = "Rated Evaporator Air Flow Rate [m3/s]";
    if (state.dataGlobal->isEpJSON) {
        stringOverride = "rated_evaporator_air_flow_rate";
    }
    sizingCoolingAirFlow.overrideSizingString(stringOverride);
    sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
    this->ratedEvapAirFlowRate = sizingCoolingAirFlow.size(state, TempSize, errorsFound);

    Real64 constexpr ratedInletAirTemp(26.6667);     // 26.6667C or 80F
    Real64 constexpr ratedInletAirHumRat(0.0111847); // Humidity ratio corresponding to 80F dry bulb/67F wet bulb
    this->ratedEvapAirMassFlowRate =
        this->ratedEvapAirFlowRate *
        Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->StdBaroPress, ratedInletAirTemp, ratedInletAirHumRat, RoutineName);

    std::string SizingString = "Rated Gross Total Cooling Capacity [W]";
    state.dataSize->DataFlowUsedForSizing = this->ratedEvapAirFlowRate;
    state.dataSize->DataTotCapCurveIndex = this->speeds[this->nominalSpeedIndex].indexCapFT;
    TempSize = this->original_input_specs.gross_rated_total_cooling_capacity;
    CoolingCapacitySizer sizerCoolingCapacity;
    sizerCoolingCapacity.overrideSizingString(SizingString);
    sizerCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
    this->ratedGrossTotalCap = sizerCoolingCapacity.size(state, TempSize, errorsFound);

    // Auto size condenser air flow to Total Capacity * 0.000114 m3/s/w (850 cfm/ton)
    state.dataSize->DataConstantUsedForSizing = this->ratedGrossTotalCap;
    state.dataSize->DataFractionUsedForSizing = 0.000114;
    TempSize = this->original_input_specs.rated_condenser_air_flow_rate;

    AutoCalculateSizer sizerCondAirFlow;
    stringOverride = "Rated Condenser Air Flow Rate [m3/s]";
    if (state.dataGlobal->isEpJSON) {
        stringOverride = "rated_condenser_air_flow_rate";
    }
    sizerCondAirFlow.overrideSizingString(stringOverride);
    sizerCondAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
    this->ratedCondAirFlowRate = sizerCondAirFlow.size(state, TempSize, errorsFound);

    if (this->condenserType != CondenserType::AIRCOOLED) {
        // Auto size Nominal Evaporative Condenser Pump Power to Total Capacity * 0.004266 w/w (15 W/ton)
        AutoCalculateSizer sizerCondEvapPumpPower;
        state.dataSize->DataConstantUsedForSizing = this->ratedGrossTotalCap;
        state.dataSize->DataFractionUsedForSizing = 0.004266;
        stringOverride = "Nominal Evaporative Condenser Pump Power [W]";
        sizerCondEvapPumpPower.overrideSizingString(stringOverride);
        TempSize = this->original_input_specs.nominal_evap_condenser_pump_power;
        sizerCondEvapPumpPower.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
        this->nominalEvaporativePumpPower = sizerCondEvapPumpPower.size(state, TempSize, errorsFound);
    }

    int thisSpeedNum = 0;
    for (auto &curSpeed : this->speeds) {
        curSpeed.parentName = this->parentName;
        curSpeed.parentModeRatedGrossTotalCap = this->ratedGrossTotalCap;
        curSpeed.ratedGrossTotalCapIsAutosized = this->ratedGrossTotalCapIsAutosized;
        curSpeed.parentModeRatedEvapAirFlowRate = this->ratedEvapAirFlowRate;
        curSpeed.ratedEvapAirFlowRateIsAutosized = this->ratedEvapAirFlowRateIsAutosized;
        curSpeed.parentModeRatedCondAirFlowRate = this->ratedCondAirFlowRate;

        // Set latent degradation parameters if applicable
        curSpeed.doLatentDegradation = false;
        if (this->latentDegradationActive) {
            if ((thisSpeedNum == 0) || ((thisSpeedNum > 0) && this->applyLatentDegradationAllSpeeds)) {
                curSpeed.parentModeTimeForCondensateRemoval = this->timeForCondensateRemoval;
                curSpeed.parentModeEvapRateRatio = this->evapRateRatio;
                curSpeed.parentModeMaxCyclingRate = this->maxCyclingRate;
                curSpeed.parentModeLatentTimeConst = this->latentTimeConst;
                curSpeed.doLatentDegradation = true;
            }
        }

        curSpeed.size(state);
        thisSpeedNum++;
    }
}

void CoilCoolingDXCurveFitOperatingMode::CalcOperatingMode(EnergyPlus::EnergyPlusData &state,
                                                           const DataLoopNode::NodeData &inletNode,
                                                           DataLoopNode::NodeData &outletNode,
                                                           int const speedNum,
                                                           Real64 const speedRatio,
                                                           HVAC::FanOp const fanOp,
                                                           DataLoopNode::NodeData &condInletNode,
                                                           [[maybe_unused]] DataLoopNode::NodeData &condOutletNode,
                                                           [[maybe_unused]] bool const singleMode)
{

    static constexpr std::string_view RoutineName = "CoilCoolingDXCurveFitOperatingMode::calcOperatingMode";
    // Currently speedNum is 1-based, while this->speeds are zero-based
    auto &thisspeed(this->speeds[max(speedNum - 1, 0)]);
    if ((speedNum == 0) || ((speedNum == 1) && (speedRatio == 0.0)) || (inletNode.MassFlowRate == 0.0) ||
        (state.dataEnvrn->OutDryBulbTemp < this->minOutdoorDrybulb)) {
        outletNode.Temp = inletNode.Temp;
        outletNode.HumRat = inletNode.HumRat;
        outletNode.Enthalpy = inletNode.Enthalpy;
        outletNode.Press = inletNode.Press;
        OpModeRTF = 0.0;
        OpModePower = 0.0;
        OpModeWasteHeat = 0.0;
        return;
    }

    if (condInletNode.Press <= 0.0) {
        condInletNode.Press = state.dataEnvrn->OutBaroPress;
    }
    if (this->condenserType == CondenserType::AIRCOOLED) {
        this->condInletTemp = condInletNode.Temp;
    } else if (this->condenserType == CondenserType::EVAPCOOLED) {
        this->condInletTemp = Psychrometrics::PsyTwbFnTdbWPb(
            state, condInletNode.Temp, condInletNode.HumRat, condInletNode.Press, "CoilCoolingDXCurveFitOperatingMode::CalcOperatingMode");
    }
    thisspeed.ambPressure = condInletNode.Press;
    thisspeed.AirMassFlow = inletNode.MassFlowRate;
    if (fanOp == HVAC::FanOp::Cycling && speedNum == 1) {
        if (speedRatio > 0.0) {
            thisspeed.AirMassFlow = thisspeed.AirMassFlow / speedRatio;
        } else {
            thisspeed.AirMassFlow = 0.0;
        }
    } else if (speedNum > 1) {
        thisspeed.AirMassFlow = state.dataHVACGlobal->MSHPMassFlowRateHigh;
    }
    // rated flow rate is adjusted by coil face area fraction so adjustment is before next IF
    thisspeed.AirMassFlow *= thisspeed.active_fraction_of_face_coil_area;
    if (thisspeed.RatedAirMassFlowRate > 0.0) {
        thisspeed.AirFF = thisspeed.AirMassFlow / thisspeed.RatedAirMassFlowRate;
    } else {
        thisspeed.AirFF = 0.0;
    }

    // If multispeed, evaluate high speed first using speedRatio as PLR

    thisspeed.CalcSpeedOutput(state, inletNode, outletNode, speedRatio, fanOp, this->condInletTemp);

    // the outlet node conditions are based on it running at the truncated flow, we need to merge the bypassed air back in and ramp up flow rate
    if (thisspeed.adjustForFaceArea) {
        thisspeed.AirMassFlow /= thisspeed.active_fraction_of_face_coil_area;
        Real64 correctedEnthalpy = (1.0 - thisspeed.active_fraction_of_face_coil_area) * inletNode.Enthalpy +
                                   thisspeed.active_fraction_of_face_coil_area * outletNode.Enthalpy;
        Real64 correctedHumRat =
            (1.0 - thisspeed.active_fraction_of_face_coil_area) * inletNode.HumRat + thisspeed.active_fraction_of_face_coil_area * outletNode.HumRat;
        Real64 correctedTemp = Psychrometrics::PsyTdbFnHW(correctedEnthalpy, correctedHumRat);
        // Check for saturation error and modify temperature at constant enthalpy
        if (correctedTemp < Psychrometrics::PsyTsatFnHPb(state, correctedEnthalpy, inletNode.Press, RoutineName)) {
            correctedTemp = Psychrometrics::PsyTsatFnHPb(state, correctedEnthalpy, inletNode.Press, RoutineName);
            correctedHumRat = Psychrometrics::PsyWFnTdbH(state, correctedTemp, correctedEnthalpy, RoutineName);
        }
        outletNode.Temp = correctedTemp;
        outletNode.HumRat = correctedHumRat;
        outletNode.Enthalpy = correctedEnthalpy;
    }

    Real64 outSpeed1HumRat = outletNode.HumRat;
    Real64 outSpeed1Enthalpy = outletNode.Enthalpy;

    if (fanOp == HVAC::FanOp::Continuous) {
        outletNode.HumRat = outletNode.HumRat * speedRatio + (1.0 - speedRatio) * inletNode.HumRat;
        outletNode.Enthalpy = outletNode.Enthalpy * speedRatio + (1.0 - speedRatio) * inletNode.Enthalpy;
        outletNode.Temp = Psychrometrics::PsyTdbFnHW(outletNode.Enthalpy, outletNode.HumRat);

        // Check for saturation error and modify temperature at constant enthalpy
        Real64 tsat = Psychrometrics::PsyTsatFnHPb(state, outletNode.Enthalpy, inletNode.Press, RoutineName);
        if (outletNode.Temp < tsat) {
            outletNode.Temp = tsat;
            outletNode.HumRat = Psychrometrics::PsyWFnTdbH(state, tsat, outletNode.Enthalpy);
        }
    }

    OpModeRTF = thisspeed.RTF;
    OpModePower = thisspeed.fullLoadPower * thisspeed.RTF;
    OpModeWasteHeat = thisspeed.fullLoadWasteHeat * thisspeed.RTF;

    if ((speedNum > 1) && (speedRatio < 1.0) && !singleMode) {

        // If multispeed, evaluate next lower speed using PLR, then combine with high speed for final outlet conditions
        auto &lowerspeed(this->speeds[max(speedNum - 2, 0)]);
        lowerspeed.AirMassFlow = state.dataHVACGlobal->MSHPMassFlowRateLow * lowerspeed.active_fraction_of_face_coil_area;

        lowerspeed.CalcSpeedOutput(state, inletNode, outletNode, 1.0, fanOp, condInletTemp); // out

        if (lowerspeed.adjustForFaceArea) {
            lowerspeed.AirMassFlow /= lowerspeed.active_fraction_of_face_coil_area;
            Real64 correctedEnthalpy = (1.0 - lowerspeed.active_fraction_of_face_coil_area) * inletNode.Enthalpy +
                                       lowerspeed.active_fraction_of_face_coil_area * outletNode.Enthalpy;
            Real64 correctedHumRat = (1.0 - lowerspeed.active_fraction_of_face_coil_area) * inletNode.HumRat +
                                     lowerspeed.active_fraction_of_face_coil_area * outletNode.HumRat;
            Real64 correctedTemp = Psychrometrics::PsyTdbFnHW(correctedEnthalpy, correctedHumRat);
            // Check for saturation error and modify temperature at constant enthalpy
            if (correctedTemp < Psychrometrics::PsyTsatFnHPb(state, correctedEnthalpy, inletNode.Press, RoutineName)) {
                correctedTemp = Psychrometrics::PsyTsatFnHPb(state, correctedEnthalpy, inletNode.Press, RoutineName);
                correctedHumRat = Psychrometrics::PsyWFnTdbH(state, correctedTemp, correctedEnthalpy, RoutineName);
            }
            outletNode.Temp = correctedTemp;
            outletNode.HumRat = correctedHumRat;
            outletNode.Enthalpy = correctedEnthalpy;
        }

        outletNode.HumRat = (outSpeed1HumRat * speedRatio * thisspeed.AirMassFlow + (1.0 - speedRatio) * outletNode.HumRat * lowerspeed.AirMassFlow) /
                            inletNode.MassFlowRate;
        outletNode.Enthalpy =
            (outSpeed1Enthalpy * speedRatio * thisspeed.AirMassFlow + (1.0 - speedRatio) * outletNode.Enthalpy * lowerspeed.AirMassFlow) /
            inletNode.MassFlowRate;
        outletNode.Temp = Psychrometrics::PsyTdbFnHW(outletNode.Enthalpy, outletNode.HumRat);

        this->OpModePower += (1.0 - thisspeed.RTF) * lowerspeed.fullLoadPower;
        this->OpModeWasteHeat += (1.0 - thisspeed.RTF) * lowerspeed.fullLoadWasteHeat;
        this->OpModeRTF = 1.0; // if we are on greater than 1 speed, RTF *must* be 1
    }
}

Real64 CoilCoolingDXCurveFitOperatingMode::getCurrentEvapCondPumpPower(int speedNum)
{
    // Currently speedNum is 1-based, while this->speeds are zero-based
    auto const &thisspeed(this->speeds[max(speedNum - 1, 0)]);
    auto const &powerFraction(thisspeed.evap_condenser_pump_power_fraction);
    return this->nominalEvaporativePumpPower * powerFraction;
}
