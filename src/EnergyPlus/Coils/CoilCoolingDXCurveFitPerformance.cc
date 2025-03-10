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

#include <EnergyPlus/Coils/CoilCoolingDXCurveFitPerformance.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

using namespace EnergyPlus;

void CoilCoolingDXCurveFitPerformance::instantiateFromInputSpec(EnergyPlus::EnergyPlusData &state,
                                                                const CoilCoolingDXCurveFitPerformanceInputSpecification &input_data)
{
    static constexpr std::string_view routineName = "CoilCoolingDXCurveFitOperatingMode::instantiateFromInputSpec";

    ErrorObjectHeader eoh{routineName, this->object_name, input_data.name};

    bool errorsFound(false);
    this->original_input_specs = input_data;
    this->name = input_data.name;
    this->minOutdoorDrybulb = input_data.minimum_outdoor_dry_bulb_temperature_for_compressor_operation;
    this->maxOutdoorDrybulbForBasin = input_data.maximum_outdoor_dry_bulb_temperature_for_crankcase_heater_operation;
    this->crankcaseHeaterCap = input_data.crankcase_heater_capacity;
    this->normalMode = CoilCoolingDXCurveFitOperatingMode(state, input_data.base_operating_mode_name);
    this->normalMode.oneTimeInit(state); // oneTimeInit does not need to be delayed in this use case

    if (Util::SameString(input_data.capacity_control, "CONTINUOUS")) {
        this->capControlMethod = CapControlMethod::CONTINUOUS;
    } else if (Util::SameString(input_data.capacity_control, "DISCRETE")) {
        this->capControlMethod = CapControlMethod::DISCRETE;
    } else {
        ShowSevereError(state, std::string{routineName} + this->object_name + "=\"" + this->name + "\", invalid");
        ShowContinueError(state, "...Capacity Control Method=\"" + input_data.capacity_control + "\":");
        ShowContinueError(state, "...must be Discrete or Continuous.");
        errorsFound = true;
    }
    this->evapCondBasinHeatCap = input_data.basin_heater_capacity;
    this->evapCondBasinHeatSetpoint = input_data.basin_heater_setpoint_temperature;
    if (input_data.basin_heater_operating_schedule_name.empty()) {
        this->evapCondBasinHeatSched = Sched::GetScheduleAlwaysOn(state);
    } else if ((this->evapCondBasinHeatSched = Sched::GetSchedule(state, input_data.basin_heater_operating_schedule_name)) == nullptr) {
        ShowSevereItemNotFound(
            state, eoh, "Evaporative Condenser Basin Heater Operating Schedule Name", input_data.basin_heater_operating_schedule_name);
        errorsFound = true;
    }

    if (!input_data.alternate_operating_mode_name.empty() && input_data.alternate_operating_mode2_name.empty()) {
        this->maxAvailCoilMode = HVAC::CoilMode::Enhanced;
        this->alternateMode = CoilCoolingDXCurveFitOperatingMode(state, input_data.alternate_operating_mode_name);
        this->alternateMode.oneTimeInit(state); // oneTimeInit does not need to be delayed in this use case
    }
    // Validate fuel type input
    this->compressorFuelType = static_cast<Constant::eFuel>(getEnumValue(Constant::eFuelNamesUC, Util::makeUPPER(input_data.compressor_fuel_type)));
    if (this->compressorFuelType == Constant::eFuel::Invalid) {
        ShowSevereError(state, std::string{routineName} + this->object_name + "=\"" + this->name + "\", invalid");
        ShowContinueError(state, "...Compressor Fuel Type=\"" + input_data.compressor_fuel_type + "\".");
        errorsFound = true;
    }

    if (!input_data.alternate_operating_mode2_name.empty() && !input_data.alternate_operating_mode_name.empty()) {
        this->maxAvailCoilMode = HVAC::CoilMode::SubcoolReheat;
        this->alternateMode = CoilCoolingDXCurveFitOperatingMode(state, input_data.alternate_operating_mode_name);
        this->alternateMode2 = CoilCoolingDXCurveFitOperatingMode(state, input_data.alternate_operating_mode2_name);
        setOperMode(state, this->normalMode, 1);
        setOperMode(state, this->alternateMode, 2);
        setOperMode(state, this->alternateMode2, 3);
    }

    if (!input_data.outdoor_temperature_dependent_crankcase_heater_capacity_curve_name.empty()) {
        this->crankcaseHeaterCapacityCurveIndex =
            Curve::GetCurveIndex(state, input_data.outdoor_temperature_dependent_crankcase_heater_capacity_curve_name);
        if (this->crankcaseHeaterCapacityCurveIndex == 0) { // can't find the curve
            ShowSevereError(state,
                            format("{} = {}:  {} not found = {}",
                                   this->object_name,
                                   this->name,
                                   "Crankcase Heater Capacity Function of Temperature Curve Name",
                                   input_data.outdoor_temperature_dependent_crankcase_heater_capacity_curve_name));

            errorsFound = true;
        } else {
            // Verify Curve Object, only legal type is Quadratic and Cubic
            errorsFound |= Curve::CheckCurveDims(state,
                                                 this->crankcaseHeaterCapacityCurveIndex,                                        // Curve index
                                                 {1},                                                                            // Valid dimensions
                                                 routineName,                                                                    // Routine name
                                                 this->object_name,                                                              // Object Type
                                                 this->name,                                                                     // Object Name
                                                 input_data.outdoor_temperature_dependent_crankcase_heater_capacity_curve_name); // Field Name
        }
    }
    if (errorsFound) {
        ShowFatalError(
            state, std::string{routineName} + "Errors found in getting " + this->object_name + " input. Preceding condition(s) causes termination.");
    }
}

CoilCoolingDXCurveFitPerformance::CoilCoolingDXCurveFitPerformance(EnergyPlus::EnergyPlusData &state, const std::string &name_to_find)
{
    int numPerformances = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CoilCoolingDXCurveFitPerformance::object_name);
    if (numPerformances <= 0) {
        // error
    }
    bool found_it = false;
    for (int perfNum = 1; perfNum <= numPerformances; ++perfNum) {
        int NumAlphas;  // Number of Alphas for each GetObjectItem call
        int NumNumbers; // Number of Numbers for each GetObjectItem call
        int IOStatus;
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CoilCoolingDXCurveFitPerformance::object_name,
                                                                 perfNum,
                                                                 state.dataIPShortCut->cAlphaArgs,
                                                                 NumAlphas,
                                                                 state.dataIPShortCut->rNumericArgs,
                                                                 NumNumbers,
                                                                 IOStatus,
                                                                 _,
                                                                 state.dataIPShortCut->lAlphaFieldBlanks);
        if (!Util::SameString(name_to_find, state.dataIPShortCut->cAlphaArgs(1))) {
            continue;
        }
        found_it = true;

        CoilCoolingDXCurveFitPerformanceInputSpecification input_specs;

        input_specs.name = state.dataIPShortCut->cAlphaArgs(1);
        input_specs.crankcase_heater_capacity = state.dataIPShortCut->rNumericArgs(1);
        input_specs.minimum_outdoor_dry_bulb_temperature_for_compressor_operation = state.dataIPShortCut->rNumericArgs(2);
        input_specs.maximum_outdoor_dry_bulb_temperature_for_crankcase_heater_operation = state.dataIPShortCut->rNumericArgs(3);
        if (state.dataIPShortCut->lNumericFieldBlanks(4)) {
            input_specs.unit_internal_static_air_pressure = 0.0;
        } else {
            input_specs.unit_internal_static_air_pressure = state.dataIPShortCut->rNumericArgs(4);
        }
        if (!state.dataIPShortCut->lAlphaFieldBlanks(2)) {
            input_specs.outdoor_temperature_dependent_crankcase_heater_capacity_curve_name = state.dataIPShortCut->cAlphaArgs(2);
        }
        input_specs.capacity_control = state.dataIPShortCut->cAlphaArgs(3);
        input_specs.basin_heater_capacity = state.dataIPShortCut->rNumericArgs(5);
        input_specs.basin_heater_setpoint_temperature = state.dataIPShortCut->rNumericArgs(6);
        input_specs.basin_heater_operating_schedule_name = state.dataIPShortCut->cAlphaArgs(4);
        input_specs.compressor_fuel_type = state.dataIPShortCut->cAlphaArgs(5);
        input_specs.base_operating_mode_name = state.dataIPShortCut->cAlphaArgs(6);
        if (!state.dataIPShortCut->lAlphaFieldBlanks(6)) {
            input_specs.alternate_operating_mode_name = state.dataIPShortCut->cAlphaArgs(7);
        }
        if (!state.dataIPShortCut->lAlphaFieldBlanks(8)) {
            input_specs.alternate_operating_mode2_name = state.dataIPShortCut->cAlphaArgs(8);
        }

        this->instantiateFromInputSpec(state, input_specs);
        break;
    }

    if (!found_it) {
        ShowFatalError(state, "Could not find Coil:Cooling:DX:Performance object with name: " + name_to_find);
    }
}

void CoilCoolingDXCurveFitPerformance::simulate(EnergyPlus::EnergyPlusData &state,
                                                const DataLoopNode::NodeData &inletNode,
                                                DataLoopNode::NodeData &outletNode,
                                                HVAC::CoilMode currentCoilMode,
                                                int const speedNum,
                                                Real64 const speedRatio,
                                                HVAC::FanOp const fanOp,
                                                DataLoopNode::NodeData &condInletNode,
                                                DataLoopNode::NodeData &condOutletNode,
                                                bool const singleMode,
                                                Real64 LoadSHR)
{
    static constexpr std::string_view RoutineName = "CoilCoolingDXCurveFitPerformance::simulate";
    Real64 reportingConstant = state.dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;
    this->recoveredEnergyRate = 0.0;
    this->NormalSHR = 0.0;

    if (currentCoilMode == HVAC::CoilMode::SubcoolReheat) {
        Real64 totalCoolingRate;
        Real64 sensNorRate;
        Real64 sensSubRate;
        Real64 sensRehRate;
        Real64 latRate;
        Real64 SysNorSHR;
        Real64 SysSubSHR;
        Real64 SysRehSHR;
        Real64 HumRatNorOut;
        Real64 TempNorOut;
        Real64 EnthalpyNorOut;
        Real64 modeRatio;

        this->calculate(state, this->normalMode, inletNode, outletNode, speedNum, speedRatio, fanOp, condInletNode, condOutletNode, singleMode);

        // this->OperatingMode = 1;
        CalcComponentSensibleLatentOutput(
            outletNode.MassFlowRate, inletNode.Temp, inletNode.HumRat, outletNode.Temp, outletNode.HumRat, sensNorRate, latRate, totalCoolingRate);
        if (totalCoolingRate > 1.0E-10) {
            this->OperatingMode = 1;
            this->NormalSHR = sensNorRate / totalCoolingRate;
            this->powerUse = this->normalMode.OpModePower;
            this->RTF = this->normalMode.OpModeRTF;
            this->wasteHeatRate = this->normalMode.OpModeWasteHeat;
        }

        if ((speedRatio != 0.0) && (LoadSHR != 0.0)) {
            if (totalCoolingRate == 0.0) {
                SysNorSHR = 1.0;
            } else {
                SysNorSHR = sensNorRate / totalCoolingRate;
            }
            HumRatNorOut = outletNode.HumRat;
            TempNorOut = outletNode.Temp;
            EnthalpyNorOut = outletNode.Enthalpy;
            this->recoveredEnergyRate = sensNorRate;

            if (LoadSHR < SysNorSHR) {
                outletNode.MassFlowRate = inletNode.MassFlowRate;
                this->calculate(
                    state, this->alternateMode, inletNode, outletNode, speedNum, speedRatio, fanOp, condInletNode, condOutletNode, singleMode);
                CalcComponentSensibleLatentOutput(outletNode.MassFlowRate,
                                                  inletNode.Temp,
                                                  inletNode.HumRat,
                                                  outletNode.Temp,
                                                  outletNode.HumRat,
                                                  sensSubRate,
                                                  latRate,
                                                  totalCoolingRate);
                SysSubSHR = sensSubRate / totalCoolingRate;
                if (LoadSHR < SysSubSHR) {
                    outletNode.MassFlowRate = inletNode.MassFlowRate;
                    this->calculate(
                        state, this->alternateMode2, inletNode, outletNode, speedNum, speedRatio, fanOp, condInletNode, condOutletNode, singleMode);
                    CalcComponentSensibleLatentOutput(outletNode.MassFlowRate,
                                                      inletNode.Temp,
                                                      inletNode.HumRat,
                                                      outletNode.Temp,
                                                      outletNode.HumRat,
                                                      sensRehRate,
                                                      latRate,
                                                      totalCoolingRate);
                    SysRehSHR = sensRehRate / totalCoolingRate;
                    if (LoadSHR > SysRehSHR) {
                        modeRatio = (LoadSHR - SysNorSHR) / (SysRehSHR - SysNorSHR);
                        this->OperatingMode = 3;
                        outletNode.HumRat = HumRatNorOut * (1.0 - modeRatio) + modeRatio * outletNode.HumRat;
                        outletNode.Enthalpy = EnthalpyNorOut * (1.0 - modeRatio) + modeRatio * outletNode.Enthalpy;
                        outletNode.Temp = Psychrometrics::PsyTdbFnHW(outletNode.Enthalpy, outletNode.HumRat);
                        this->ModeRatio = modeRatio;
                        // update other reporting terms
                        this->powerUse = this->normalMode.OpModePower * (1.0 - modeRatio) + modeRatio * this->alternateMode2.OpModePower;
                        this->RTF = this->normalMode.OpModeRTF * (1.0 - modeRatio) + modeRatio * this->alternateMode2.OpModeRTF;
                        this->wasteHeatRate = this->normalMode.OpModeWasteHeat * (1.0 - modeRatio) + modeRatio * this->alternateMode2.OpModeWasteHeat;
                        this->recoveredEnergyRate = (this->recoveredEnergyRate - sensRehRate) * this->ModeRatio;
                    } else {
                        this->ModeRatio = 1.0;
                        this->OperatingMode = 3;
                        this->recoveredEnergyRate = (this->recoveredEnergyRate - sensRehRate) * this->ModeRatio;
                    }
                } else {
                    modeRatio = (LoadSHR - SysNorSHR) / (SysSubSHR - SysNorSHR);
                    this->OperatingMode = 2;
                    // process outlet conditions and total output
                    outletNode.HumRat = HumRatNorOut * (1.0 - modeRatio) + modeRatio * outletNode.HumRat;
                    outletNode.Enthalpy = EnthalpyNorOut * (1.0 - modeRatio) + modeRatio * outletNode.Enthalpy;
                    outletNode.Temp = Psychrometrics::PsyTdbFnHW(outletNode.Enthalpy, outletNode.HumRat);
                    this->ModeRatio = modeRatio;
                    // update other reporting terms
                    this->powerUse = this->normalMode.OpModePower * (1.0 - modeRatio) + modeRatio * this->alternateMode.OpModePower;
                    this->RTF = this->normalMode.OpModeRTF * (1.0 - modeRatio) + modeRatio * this->alternateMode.OpModeRTF;
                    this->wasteHeatRate = this->normalMode.OpModeWasteHeat * (1.0 - modeRatio) + modeRatio * this->alternateMode.OpModeWasteHeat;
                    this->recoveredEnergyRate = (this->recoveredEnergyRate - sensSubRate) * this->ModeRatio;
                }
            } else {
                this->ModeRatio = 0.0;
                this->OperatingMode = 1;
                this->recoveredEnergyRate = 0.0;
            }
            // Check for saturation error and modify temperature at constant enthalpy
            Real64 tsat = Psychrometrics::PsyTsatFnHPb(state, outletNode.Enthalpy, inletNode.Press, RoutineName);
            if (outletNode.Temp < tsat) {
                outletNode.Temp = tsat;
                outletNode.HumRat = Psychrometrics::PsyWFnTdbH(state, tsat, outletNode.Enthalpy);
            }
        }
    } else if (currentCoilMode == HVAC::CoilMode::Enhanced) {
        this->calculate(state, this->alternateMode, inletNode, outletNode, speedNum, speedRatio, fanOp, condInletNode, condOutletNode, singleMode);
        this->OperatingMode = 2;
        this->powerUse = this->alternateMode.OpModePower;
        this->RTF = this->alternateMode.OpModeRTF;
        this->wasteHeatRate = this->alternateMode.OpModeWasteHeat;
    } else {
        this->calculate(state, this->normalMode, inletNode, outletNode, speedNum, speedRatio, fanOp, condInletNode, condOutletNode, singleMode);
        this->OperatingMode = 1;
        this->powerUse = this->normalMode.OpModePower;
        this->RTF = this->normalMode.OpModeRTF;
        this->wasteHeatRate = this->normalMode.OpModeWasteHeat;
    }

    // calculate crankcase heater operation
    if (state.dataEnvrn->OutDryBulbTemp < this->maxOutdoorDrybulbForBasin) {
        this->crankcaseHeaterPower = this->crankcaseHeaterCap;
        if (this->crankcaseHeaterCapacityCurveIndex > 0) {
            this->crankcaseHeaterPower *= Curve::CurveValue(state, this->crankcaseHeaterCapacityCurveIndex, state.dataEnvrn->OutDryBulbTemp);
        }
    } else {
        this->crankcaseHeaterPower = 0.0;
    }
    this->crankcaseHeaterPower = this->crankcaseHeaterPower * (1.0 - this->RTF);
    this->crankcaseHeaterElectricityConsumption = this->crankcaseHeaterPower * reportingConstant;

    // basin heater
    if (this->evapCondBasinHeatSched != nullptr) {
        Real64 currentBasinHeaterAvail = this->evapCondBasinHeatSched->getCurrentVal();
        if (this->evapCondBasinHeatCap > 0.0 && currentBasinHeaterAvail > 0.0) {
            this->basinHeaterPower = max(0.0, this->evapCondBasinHeatCap * (this->evapCondBasinHeatSetpoint - state.dataEnvrn->OutDryBulbTemp));
        }
    } else {
        // If schedule does not exist, basin heater operates anytime outdoor dry-bulb temp is below setpoint
        if (this->evapCondBasinHeatCap > 0.0) {
            this->basinHeaterPower = max(0.0, this->evapCondBasinHeatCap * (this->evapCondBasinHeatSetpoint - state.dataEnvrn->OutDryBulbTemp));
        }
    }
    this->basinHeaterPower *= (1.0 - this->RTF);
    this->electricityConsumption = this->powerUse * reportingConstant;

    if (this->compressorFuelType != Constant::eFuel::Electricity) {
        this->compressorFuelRate = this->powerUse;
        this->compressorFuelConsumption = this->electricityConsumption;

        // check this after adding parasitic loads
        this->powerUse = 0.0;
        this->electricityConsumption = 0.0;
    }
}

void CoilCoolingDXCurveFitPerformance::size(EnergyPlus::EnergyPlusData &state)
{
    if (!state.dataGlobal->SysSizingCalc && this->mySizeFlag) {
        this->normalMode.parentName = this->parentName;
        this->normalMode.size(state);
        if (this->maxAvailCoilMode == HVAC::CoilMode::Enhanced) {
            this->alternateMode.size(state);
        }
        if (this->maxAvailCoilMode == HVAC::CoilMode::SubcoolReheat) {
            this->alternateMode.size(state);
            this->alternateMode2.size(state);
        }
        this->mySizeFlag = false;
    }
}

void CoilCoolingDXCurveFitPerformance::calculate(EnergyPlus::EnergyPlusData &state,
                                                 CoilCoolingDXCurveFitOperatingMode &currentMode,
                                                 const DataLoopNode::NodeData &inletNode,
                                                 DataLoopNode::NodeData &outletNode,
                                                 int const speedNum,
                                                 Real64 const speedRatio,
                                                 HVAC::FanOp const fanOp,
                                                 DataLoopNode::NodeData &condInletNode,
                                                 DataLoopNode::NodeData &condOutletNode,
                                                 bool const singleMode)
{

    // calculate the performance at this mode/speed
    currentMode.CalcOperatingMode(state, inletNode, outletNode, speedNum, speedRatio, fanOp, condInletNode, condOutletNode, singleMode);
}

void CoilCoolingDXCurveFitPerformance::calcStandardRatings210240(EnergyPlus::EnergyPlusData &state)
{

    // for now this will provide standard ratings for the coil at the normal mode at speed N
    // future iterations will extend the inputs to give the user the flexibility to select different standards to
    // apply and such

    int constexpr NumOfReducedCap(4); // Number of reduced capacity test conditions (100%,75%,50%,and 25%)

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 TotCapFlowModFac(0.0);                 // Total capacity modifier f(actual flow vs rated flow) for each speed [-]
    Real64 EIRFlowModFac(0.0);                    // EIR modifier f(actual supply air flow vs rated flow) for each speed [-]
    Real64 TotCapTempModFac(0.0);                 // Total capacity modifier (function of entering wetbulb, outside drybulb) [-]
    Real64 EIRTempModFac(0.0);                    // EIR modifier (function of entering wetbulb, outside drybulb) [-]
    Real64 TotCoolingCapAHRI(0.0);                // Total Cooling Coil capacity (gross) at AHRI test conditions [W]
    Real64 NetCoolingCapAHRI(0.0);                // Net Cooling Coil capacity at AHRI TestB conditions, accounting for fan heat [W]
    Real64 NetCoolingCapAHRI2023(0.0);            // Net Cooling Coil capacity at AHRI TestB conditions, accounting for fan heat [W]
    Real64 TotalElecPower(0.0);                   // Net power consumption (Cond Fan+Compressor+Indoor Fan) at AHRI test conditions [W]
    Real64 TotalElecPower2023(0.0);               // Net power consumption (Cond Fan+Compressor+Indoor Fan) at AHRI test conditions [W]
    Real64 TotalElecPowerRated(0.0);              // Net power consumption (Cond Fan+Compressor+Indoor Fan) at Rated test conditions [W]
    Real64 TotalElecPowerRated2023(0.0);          // Net power consumption (Cond Fan+Compressor+Indoor Fan) at Rated test conditions [W]
    Real64 EIR(0.0);                              // Energy Efficiency Ratio at AHRI test conditions for SEER [-]
    Real64 PartLoadFactor(0.0);                   // Part load factor, accounts for thermal lag at compressor startup [-]
    Real64 EERReduced(0.0);                       // EER at reduced capacity test conditions (100%, 75%, 50%, and 25%)
    Real64 ElecPowerReducedCap(0.0);              // Net power consumption (Cond Fan+Compressor) at reduced test condition [W]
    Real64 NetCoolingCapReduced(0.0);             // Net Cooling Coil capacity at reduced conditions, accounting for supply fan heat [W]
    Real64 LoadFactor(0.0);                       // Fractional "on" time for last stage at the desired reduced capacity, (dimensionless)
    Real64 DegradationCoeff(0.0);                 // Degradation coefficient, (dimensionless)
    Real64 OutdoorUnitInletAirDryBulbTempReduced; // Outdoor unit entering air dry-bulb temperature at reduced capacity [C]

    // *** SOME CONSTANTS FROM THE STANDARD
    // The AHRI standard specifies a nominal/default fan electric power consumption per rated air
    // volume flow rate to account for indoor fan electric power consumption
    // when the standard tests are conducted on units that do not have an
    // indoor air circulating fan. Used if user doesn't enter a specific value.
    Real64 constexpr DefaultFanPowerPerEvapAirFlowRate(773.3);     // 365 W/1000 scfm or 773.3 W/(m3/s).
    Real64 constexpr DefaultFanPowerPerEvapAirFlowRate2023(934.4); // 441 W/1000 scfm or 934.4 W/(m3/s).
    // AHRI Standard 210/240-2008 Performance Test Conditions for Unitary Air-to-Air Air-Conditioning and Heat Pump Equipment
    Real64 constexpr CoolingCoilInletAirWetBulbTempRated(19.44); // 19.44C (67F)  Tests A and B
    Real64 constexpr OutdoorUnitInletAirDryBulbTemp(27.78);      // 27.78C (82F)  Test B (for SEER)
    Real64 constexpr OutdoorUnitInletAirDryBulbTempRated(35.0);  // 35.00C (95F)  Test A (rated capacity)
    Real64 constexpr AirMassFlowRatioRated(1.0);                 // AHRI test is at the design flow rate so AirMassFlowRatio is 1.0
    Real64 constexpr PLRforSEER(0.5);                            // Part-load ratio for SEER calculation (single speed DX cooling coils)
    static constexpr std::array<Real64, 4> ReducedPLR = {1.0, 0.75, 0.50, 0.25};               // Reduced Capacity part-load conditions
    static constexpr std::array<Real64, 4> IEERWeightingFactor = {0.020, 0.617, 0.238, 0.125}; // EER Weighting factors (IEER)
    Real64 constexpr OADBTempLowReducedCapacityTest(18.3);                                     // Outdoor air dry-bulb temp in degrees C (65F)

    // For Single Stage Systems, if the optional CFull and DFull tests are not performed, a
    // default value of 0.20 shall be used for the cooling Degradation Coefficient
    Real64 constexpr CyclicDegradationCoefficient(0.20); // ANSI/AHRI 210/240 2023 Section 6.1.3.1

    // some conveniences
    auto &mode = this->normalMode;
    auto &speed = mode.speeds.back();

    Real64 FanPowerPerEvapAirFlowRate = DefaultFanPowerPerEvapAirFlowRate;
    if (speed.rated_evap_fan_power_per_volume_flow_rate > 0.0) {
        FanPowerPerEvapAirFlowRate = speed.rated_evap_fan_power_per_volume_flow_rate;
    }

    Real64 FanPowerPerEvapAirFlowRate2023 = DefaultFanPowerPerEvapAirFlowRate2023;
    if (speed.rated_evap_fan_power_per_volume_flow_rate_2023 > 0.0) {
        FanPowerPerEvapAirFlowRate2023 = speed.rated_evap_fan_power_per_volume_flow_rate_2023;
    }

    if (mode.ratedGrossTotalCap > 0.0) {
        TotCapFlowModFac = Curve::CurveValue(state, speed.indexCapFFF, AirMassFlowRatioRated);

        TotCapTempModFac = Curve::CurveValue(state, speed.indexCapFT, CoolingCoilInletAirWetBulbTempRated, OutdoorUnitInletAirDryBulbTemp);
        TotCoolingCapAHRI = mode.ratedGrossTotalCap * TotCapTempModFac * TotCapFlowModFac;
        // Calculate net cooling capacity | 2017
        this->standardRatingCoolingCapacity = TotCoolingCapAHRI - FanPowerPerEvapAirFlowRate * mode.ratedEvapAirFlowRate;
        // Calculate net cooling capacity | 2023
        this->standardRatingCoolingCapacity2023 = TotCoolingCapAHRI - FanPowerPerEvapAirFlowRate2023 * mode.ratedEvapAirFlowRate;
        // TODO: Commercial and industrial unitary air-conditioning condensing units with a capacity greater than 135,000 Btu/h (39564.59445 Watts)
        // as defined in ANSI/AHRI Standard 365(I-P). | Scope 2.2.6 (ANSI/AHRI 340-360 2022)

        // SEER2 standard applies to factory-made Unitary Air-conditioners and Unitary Air-source Heat Pumps with
        // capacities less than 65,000 Btu/h (19049.61955 Watts) | Section 2.1 (ANSI/AHRI 210-240 2023)
        // Removal of water-cooled and evaporatively-cooled products from the scope | Foreword (ANSI/AHRI 210-240 2023)

        // SEER calculations:
        TotCapTempModFac = Curve::CurveValue(state, speed.indexCapFT, CoolingCoilInletAirWetBulbTempRated, OutdoorUnitInletAirDryBulbTemp);
        TotCoolingCapAHRI = mode.ratedGrossTotalCap * TotCapTempModFac * TotCapFlowModFac;
        EIRTempModFac = Curve::CurveValue(state, speed.indexEIRFT, CoolingCoilInletAirWetBulbTempRated, OutdoorUnitInletAirDryBulbTemp);
        EIRFlowModFac = Curve::CurveValue(state, speed.indexEIRFFF, AirMassFlowRatioRated);
        if (speed.ratedCOP > 0.0) { // RatedCOP <= 0.0 is trapped in GetInput, but keep this as "safety"
            EIR = EIRTempModFac * EIRFlowModFac / speed.ratedCOP;
        } else {
            EIR = 0.0;
        }

        // Calculate net cooling capacity
        NetCoolingCapAHRI = TotCoolingCapAHRI - FanPowerPerEvapAirFlowRate * mode.ratedEvapAirFlowRate;
        TotalElecPower = EIR * TotCoolingCapAHRI + FanPowerPerEvapAirFlowRate * mode.ratedEvapAirFlowRate;

        NetCoolingCapAHRI2023 = TotCoolingCapAHRI - FanPowerPerEvapAirFlowRate2023 * mode.ratedEvapAirFlowRate;
        TotalElecPower2023 = EIR * TotCoolingCapAHRI + FanPowerPerEvapAirFlowRate2023 * mode.ratedEvapAirFlowRate;
        // Calculate SEER value from the Energy Efficiency Ratio (EER) at the AHRI test conditions and the part load factor.
        // First evaluate the Part Load Factor curve at PLR = 0.5 (AHRI Standard 210/240)
        PartLoadFactor = Curve::CurveValue(state, speed.indexPLRFPLF, PLRforSEER);
        Real64 PartLoadFactorStandard = 1.0 - (1 - PLRforSEER) * CyclicDegradationCoefficient;
        if (TotalElecPower > 0.0) {
            this->standardRatingSEER = (NetCoolingCapAHRI / TotalElecPower) * PartLoadFactor;
            this->standardRatingSEER_Standard = (NetCoolingCapAHRI / TotalElecPower) * PartLoadFactorStandard;
        } else {
            this->standardRatingSEER = 0.0;
            this->standardRatingSEER2_Standard = 0.0;
        }

        if (TotalElecPower2023 > 0.0) {
            this->standardRatingSEER2_User = (NetCoolingCapAHRI2023 / TotalElecPower2023) * PartLoadFactor;
            this->standardRatingSEER2_Standard = (NetCoolingCapAHRI2023 / TotalElecPower2023) * PartLoadFactorStandard;
        } else {
            this->standardRatingSEER2_User = 0.0;
            this->standardRatingSEER2_Standard = 0.0;
        }

        // EER calculations:
        // Calculate the net cooling capacity at the rated conditions (19.44C WB and 35.0C DB )
        TotCapTempModFac = Curve::CurveValue(state, speed.indexCapFT, CoolingCoilInletAirWetBulbTempRated, OutdoorUnitInletAirDryBulbTempRated);
        this->standardRatingCoolingCapacity =
            mode.ratedGrossTotalCap * TotCapTempModFac * TotCapFlowModFac - FanPowerPerEvapAirFlowRate * mode.ratedEvapAirFlowRate;
        this->standardRatingCoolingCapacity2023 =
            mode.ratedGrossTotalCap * TotCapTempModFac * TotCapFlowModFac - FanPowerPerEvapAirFlowRate2023 * mode.ratedEvapAirFlowRate;
        // Calculate Energy Efficiency Ratio (EER) at (19.44C WB and 35.0C DB ), ANSI/AHRI Std. 340/360
        EIRTempModFac = Curve::CurveValue(state, speed.indexEIRFT, CoolingCoilInletAirWetBulbTempRated, OutdoorUnitInletAirDryBulbTempRated);
        if (speed.ratedCOP > 0.0) {
            // RatedCOP <= 0.0 is trapped in GetInput, but keep this as "safety"
            EIR = EIRTempModFac * EIRFlowModFac / speed.ratedCOP;
        } else {
            EIR = 0.0;
        }
        TotalElecPowerRated =
            EIR * (mode.ratedGrossTotalCap * TotCapTempModFac * TotCapFlowModFac) + FanPowerPerEvapAirFlowRate * mode.ratedEvapAirFlowRate;
        if (TotalElecPowerRated > 0.0) {
            this->standardRatingEER = this->standardRatingCoolingCapacity / TotalElecPowerRated;
        } else {
            this->standardRatingEER = 0.0;
        }
        TotalElecPowerRated2023 =
            EIR * (mode.ratedGrossTotalCap * TotCapTempModFac * TotCapFlowModFac) + FanPowerPerEvapAirFlowRate2023 * mode.ratedEvapAirFlowRate;
        if (TotalElecPowerRated2023 > 0.0) {
            this->standardRatingEER2 = this->standardRatingCoolingCapacity2023 / TotalElecPowerRated2023;
        } else {
            this->standardRatingEER2 = 0.0;
        }

        if (mode.condenserType == CoilCoolingDXCurveFitOperatingMode::CondenserType::AIRCOOLED) {
            std::tie(this->standardRatingCoolingCapacity2023,
                     this->standardRatingSEER2_User,
                     this->standardRatingSEER2_Standard,
                     this->standardRatingEER2) = StandardRatings::SEER2CalulcationCurveFit(state, "Coil:Cooling:DX:CurveFit", this->normalMode);
        }

        // IEER calculations: Capacity of 65K Btu/h (19050 W) to less than 135K Btu/h (39565 W) - calculated as per AHRI Standard 340/360-2022.
        this->standardRatingIEER = 0.0;
        // Calculate the net cooling capacity at the rated conditions (19.44C WB and 35.0C DB )
        TotCapTempModFac = Curve::CurveValue(state, speed.indexCapFT, CoolingCoilInletAirWetBulbTempRated, OutdoorUnitInletAirDryBulbTempRated);
        this->standardRatingCoolingCapacity =
            mode.ratedGrossTotalCap * TotCapTempModFac * TotCapFlowModFac - FanPowerPerEvapAirFlowRate * mode.ratedEvapAirFlowRate;
        for (int RedCapNum = 0; RedCapNum < NumOfReducedCap; ++RedCapNum) {
            // get the outdoor air dry bulb temperature for the reduced capacity test conditions
            if (ReducedPLR[RedCapNum] > 0.444) {
                OutdoorUnitInletAirDryBulbTempReduced = 5.0 + 30.0 * ReducedPLR[RedCapNum];
            } else {
                OutdoorUnitInletAirDryBulbTempReduced = OADBTempLowReducedCapacityTest;
            }
            TotCapTempModFac = Curve::CurveValue(state, speed.indexCapFT, CoolingCoilInletAirWetBulbTempRated, OutdoorUnitInletAirDryBulbTempReduced);
            NetCoolingCapReduced =
                mode.ratedGrossTotalCap * TotCapTempModFac * TotCapFlowModFac - FanPowerPerEvapAirFlowRate * mode.ratedEvapAirFlowRate;
            EIRTempModFac = Curve::CurveValue(state, speed.indexEIRFT, CoolingCoilInletAirWetBulbTempRated, OutdoorUnitInletAirDryBulbTempReduced);
            EIRFlowModFac = Curve::CurveValue(state, speed.indexEIRFFF, AirMassFlowRatioRated);
            if (speed.ratedCOP > 0.0) {
                EIR = EIRTempModFac * EIRFlowModFac / speed.ratedCOP;
            } else {
                EIR = 0.0;
            }
            if (NetCoolingCapReduced > 0.0) {
                LoadFactor = ReducedPLR[RedCapNum] * this->standardRatingCoolingCapacity / NetCoolingCapReduced;
            } else {
                LoadFactor = 1.0;
            }
            DegradationCoeff = 1.130 - 0.130 * LoadFactor;
            ElecPowerReducedCap = DegradationCoeff * EIR * (mode.ratedGrossTotalCap * TotCapTempModFac * TotCapFlowModFac);
            EERReduced =
                (LoadFactor * NetCoolingCapReduced) / (LoadFactor * ElecPowerReducedCap + FanPowerPerEvapAirFlowRate * mode.ratedEvapAirFlowRate);
            this->standardRatingIEER += IEERWeightingFactor[RedCapNum] * EERReduced;
        }

        // IEER 2022 -->
        // TODO: we can always decide and give precedence to Alternate Mode 1 or Alternate Mode 2 if present | Needs Discussion about the
        // applicability.
        std::tie(this->standardRatingIEER2, this->standardRatingCoolingCapacity2023, this->standardRatingEER2) =
            StandardRatings::IEERCalulcationCurveFit(state, "Coil:Cooling:DX:CurveFit", this->normalMode);

    } else {
        ShowSevereError(state,
                        "Standard Ratings: Coil:Cooling:DX " + this->name +
                            " has zero rated total cooling capacity. Standard ratings cannot be calculated.");
    }
}

void CoilCoolingDXCurveFitPerformance::setOperMode(EnergyPlus::EnergyPlusData &state, CoilCoolingDXCurveFitOperatingMode &currentMode, int const mode)
{
    // set parent mode for each speed
    int numSpeeds;
    bool errorsFound = false;

    numSpeeds = (int)currentMode.speeds.size();
    for (int speedNum = 0; speedNum < numSpeeds; speedNum++) {
        currentMode.speeds[speedNum].parentOperatingMode = mode;
        if (mode == 2) {
            if (currentMode.speeds[speedNum].indexSHRFT == 0) {
                ShowSevereError(state, currentMode.speeds[speedNum].object_name + "=\"" + currentMode.speeds[speedNum].name + "\", Curve check:");
                ShowContinueError(state,
                                  "The input of Sensible Heat Ratio Modifier Function of Temperature Curve Name is required, but not available for "
                                  "SubcoolReheat mode. Please input");
                errorsFound = true;
            }
            if (currentMode.speeds[speedNum].indexSHRFFF == 0) {
                ShowSevereError(state, currentMode.speeds[speedNum].object_name + "=\"" + currentMode.speeds[speedNum].name + "\", Curve check:");
                ShowContinueError(state,
                                  "The input of Sensible Heat Ratio Modifier Function of Flow Fraction Curve Name is required, but not available for "
                                  "SubcoolReheat mode. Please input");
                errorsFound = true;
            }
        }
        if (mode == 3) {
            if (currentMode.speeds[speedNum].indexSHRFT == 0) {
                ShowSevereError(state, currentMode.speeds[speedNum].object_name + "=\"" + currentMode.speeds[speedNum].name + "\", Curve check:");
                ShowContinueError(state,
                                  "The input of Sensible Heat Ratio Modifier Function of Temperature Curve Name is required, but not available for "
                                  "SubcoolReheat mode. Please input");
                errorsFound = true;
            }
            if (currentMode.speeds[speedNum].indexSHRFFF == 0) {
                ShowSevereError(state, currentMode.speeds[speedNum].object_name + "=\"" + currentMode.speeds[speedNum].name + "\", Curve check:");
                ShowContinueError(state,
                                  "The input of Sensible Heat Ratio Modifier Function of Flow Fraction Curve Name is required, but not available for "
                                  "SubcoolReheat mode. Please input");
                errorsFound = true;
            }
        }
    }
    if (errorsFound) {
        ShowFatalError(state,
                       "CoilCoolingDXCurveFitPerformance: Errors found in getting " + this->object_name +
                           " input. Preceding condition(s) causes termination.");
    }
}
