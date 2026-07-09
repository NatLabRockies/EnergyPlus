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

// C++ Headers
#include <cmath>
#include <format>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterToAirHeatPump.hh>

namespace EnergyPlus {

namespace WaterToAirHeatPump {
    // Module containing the Water to Air Heat Pump simulation routines

    // MODULE INFORMATION:
    //       AUTHOR         Hui Jin
    //       DATE WRITTEN   Oct 2000
    //       MODIFIED       Dan Fisher, Kenneth Tang (Jan 2004)
    //                      Brent Griffith, plant upgrades, fluid props

    // PURPOSE OF THIS MODULE:
    // To encapsulate the data and algorithms required to
    // manage the Water to Air Heat Pump Component

    // METHODOLOGY EMPLOYED:

    // REFERENCES:
    // Jin, H. 2002. Parameter Estimation Based Models of Water Source Heat Pumps. Phd Thesis.
    // Oklahoma State University.

    void SimWatertoAirHP(EnergyPlusData &state,
                         std::string_view CompName,     // component name
                         int &CompIndex,                // Index for Component name
                         Real64 const DesignAirflow,    // design air flow rate
                         HVAC::FanOp const fanOp,       // cycling scheme--either continuous fan/cycling compressor or
                         bool const FirstHVACIteration, // first iteration flag
                         bool const InitFlag,           // initialization flag used to suppress property routine errors
                         Real64 const SensLoad,         // sensible load
                         Real64 const LatentLoad,       // latent load
                         HVAC::CompressorOp const compressorOp,
                         Real64 const PartLoadRatio)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Hui Jin
        //       DATE WRITTEN   Oct 2000
        //       MODIFIED       Dan Fisher, Kenneth Tang (Jan 2004)

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine manages Water to Air Heat Pump component simulation.

        // shut off after compressor cycle off  [s]
        // cycling fan/cycling compressor

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int HPNum; // The WatertoAirHP that you are currently loading input into

        // Obtains and Allocates WatertoAirHP related parameters from input file
        if (state.dataWaterToAirHeatPump->GetCoilsInputFlag) { // First time subroutine has been entered
            GetWatertoAirHPInput(state);
            state.dataWaterToAirHeatPump->GetCoilsInputFlag = false;
        }

        if (CompIndex == 0) {
            HPNum = Util::FindItemInList(CompName, state.dataWaterToAirHeatPump->WatertoAirHP);
            if (HPNum == 0) {
                ShowFatalError(state, std::format("WaterToAir HP not found={}", CompName));
            }
            CompIndex = HPNum;
        } else {
            HPNum = CompIndex;
            if (HPNum > state.dataWaterToAirHeatPump->NumWatertoAirHPs || HPNum < 1) {
                ShowFatalError(state,
                               std::format("SimWatertoAirHP: Invalid CompIndex passed={}, Number of Water to Air HPs={}, WaterToAir HP name={}",
                                           HPNum,
                                           state.dataWaterToAirHeatPump->NumWatertoAirHPs,
                                           CompName));
            }
            if (state.dataWaterToAirHeatPump->CheckEquipName(HPNum)) {
                if (!CompName.empty() && CompName != state.dataWaterToAirHeatPump->WatertoAirHP(HPNum).Name) {
                    ShowFatalError(
                        state,
                        std::format(
                            "SimWatertoAirHP: Invalid CompIndex passed={}, WaterToAir HP name={}, stored WaterToAir HP Name for that index={}",
                            HPNum,
                            CompName,
                            state.dataWaterToAirHeatPump->WatertoAirHP(HPNum).Name));
                }
                state.dataWaterToAirHeatPump->CheckEquipName(HPNum) = false;
            }
        }
        // Calculate the Correct Water to Air HP Model with the current HPNum

        if (state.dataWaterToAirHeatPump->WatertoAirHP(HPNum).WAHPType == DataPlant::PlantEquipmentType::CoilWAHPCoolingParamEst) {
            InitWatertoAirHP(state, HPNum, InitFlag, SensLoad, LatentLoad, DesignAirflow, PartLoadRatio);
            CalcWatertoAirHPCooling(state, HPNum, fanOp, FirstHVACIteration, InitFlag, SensLoad, compressorOp, PartLoadRatio);

            UpdateWatertoAirHP(state, HPNum);

        } else if (state.dataWaterToAirHeatPump->WatertoAirHP(HPNum).WAHPType == DataPlant::PlantEquipmentType::CoilWAHPHeatingParamEst) {
            InitWatertoAirHP(state, HPNum, InitFlag, SensLoad, LatentLoad, DesignAirflow, PartLoadRatio);
            CalcWatertoAirHPHeating(state, HPNum, fanOp, FirstHVACIteration, InitFlag, SensLoad, compressorOp, PartLoadRatio);

            UpdateWatertoAirHP(state, HPNum);

        } else {
            ShowFatalError(state, "SimWatertoAirHP: AirtoAir heatpump not in either HEATING or COOLING");
        }
    }

    // Get Input Section of the Module
    //******************************************************************************

    void GetWatertoAirHPInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Hui Jin
        //       DATE WRITTEN   Oct 2000
        //       MODIFIED       Dan Fisher, Kenneth Tang (Jan 2004)

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains input data for HPs and stores it in HP data structures

        // METHODOLOGY EMPLOYED:
        // Uses "Get" routines to read in data.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetWatertoAirHPInput: "); // include trailing blank space
        static constexpr std::string_view routineName = "GetWatertoAirHPInput";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int HPNum; // The Water to Air HP that you are currently loading input into
        int NumCool;
        int NumHeat;
        bool ErrorsFound(false);         // If errors detected in input
        std::string CurrentModuleObject; // for ease in getting objects

        constexpr std::array<std::string_view, static_cast<int>(CompressorType::Num)> CompressTypeNamesUC{"RECIPROCATING", "ROTARY", "SCROLL"};

        auto &s_ip = state.dataInputProcessing->inputProcessor;

        NumCool = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Coil:Cooling:WaterToAirHeatPump:ParameterEstimation");
        NumHeat = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Coil:Heating:WaterToAirHeatPump:ParameterEstimation");
        state.dataWaterToAirHeatPump->NumWatertoAirHPs = NumCool + NumHeat;
        HPNum = 0;

        if (state.dataWaterToAirHeatPump->NumWatertoAirHPs <= 0) {
            ShowSevereError(state, "No Equipment found in SimWatertoAirHP");
            ErrorsFound = true;
        }

        // Allocate Arrays
        if (state.dataWaterToAirHeatPump->NumWatertoAirHPs > 0) {
            state.dataWaterToAirHeatPump->WatertoAirHP.allocate(state.dataWaterToAirHeatPump->NumWatertoAirHPs);
            state.dataWaterToAirHeatPump->CheckEquipName.dimension(state.dataWaterToAirHeatPump->NumWatertoAirHPs, true);
        }

        // Get the data for detailed cooling Heat Pump
        CurrentModuleObject = "Coil:Cooling:WaterToAirHeatPump:ParameterEstimation";
        auto const instances = s_ip->epJSON.find(CurrentModuleObject);

        HPNum = 0;
        if (instances != s_ip->epJSON.end()) {
            auto const &schemaProps = s_ip->getObjectSchemaProps(state, CurrentModuleObject);
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                std::string cFieldName;
                auto const &fields = instance.value();
                std::string const &thisObjectName = instance.key();
                s_ip->markObjectAsUsed(CurrentModuleObject, thisObjectName);
                ++HPNum;

                auto &heatPump = state.dataWaterToAirHeatPump->WatertoAirHP(HPNum);
                heatPump.Name = Util::makeUPPER(thisObjectName);
                heatPump.WatertoAirHPType = "COOLING";
                heatPump.WAHPType = DataPlant::PlantEquipmentType::CoilWAHPCoolingParamEst;
                ErrorObjectHeader eoh{routineName, CurrentModuleObject, heatPump.Name};
                GlobalNames::VerifyUniqueCoilName(
                    state, CurrentModuleObject, heatPump.Name, ErrorsFound, std::format("{} Name", CurrentModuleObject));
                std::string const availSchedName = s_ip->getAlphaFieldValue(fields, schemaProps, "availability_schedule_name");
                if (availSchedName.empty()) {
                    heatPump.availSched = Sched::GetScheduleAlwaysOn(state);
                } else if ((heatPump.availSched = Sched::GetSchedule(state, availSchedName)) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, "Availability Schedule Name", availSchedName);
                    ErrorsFound = true;
                }
                cFieldName = "Refrigerant Type";
                heatPump.Refrigerant = s_ip->getAlphaFieldValue(fields, schemaProps, "refrigerant_type"); // AlphArray(3);
                if (heatPump.Refrigerant.empty()) {
                    ShowSevereEmptyField(state, eoh, cFieldName);
                    ErrorsFound = true;
                } else if ((heatPump.refrig = Fluid::GetRefrig(state, heatPump.Refrigerant)) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cFieldName, heatPump.Refrigerant);
                    ErrorsFound = true;
                }
                heatPump.DesignWaterVolFlowRate = s_ip->getRealFieldValue(fields, schemaProps, "design_source_side_flow_rate");
                heatPump.CoolingCapacity = s_ip->getRealFieldValue(fields, schemaProps, "nominal_cooling_coil_capacity");
                heatPump.Twet_Rated = s_ip->getRealFieldValue(fields, schemaProps, "nominal_time_for_condensate_removal_to_begin");
                heatPump.Gamma_Rated =
                    s_ip->getRealFieldValue(fields, schemaProps, "ratio_of_initial_moisture_evaporation_rate_and_steady_state_latent_capacity");
                heatPump.HighPressCutoff = s_ip->getRealFieldValue(fields, schemaProps, "high_pressure_cutoff");
                heatPump.LowPressCutoff = s_ip->getRealFieldValue(fields, schemaProps, "low_pressure_cutoff");

                std::string waterInletNodeName = s_ip->getAlphaFieldValue(fields, schemaProps, "water_inlet_node_name");
                std::string waterOutletNodeName = s_ip->getAlphaFieldValue(fields, schemaProps, "water_outlet_node_name");
                std::string airInletNodeName = s_ip->getAlphaFieldValue(fields, schemaProps, "air_inlet_node_name");
                std::string airOutletNodeName = s_ip->getAlphaFieldValue(fields, schemaProps, "air_outlet_node_name");

                heatPump.WaterInletNodeNum = GetOnlySingleNode(state,
                                                               waterInletNodeName,
                                                               ErrorsFound,
                                                               Node::ConnectionObjectType::CoilCoolingWaterToAirHeatPumpParameterEstimation,
                                                               heatPump.Name,
                                                               Node::FluidType::Water,
                                                               Node::ConnectionType::Inlet,
                                                               Node::CompFluidStream::Secondary,
                                                               Node::ObjectIsNotParent);
                heatPump.WaterOutletNodeNum = GetOnlySingleNode(state,
                                                                waterOutletNodeName,
                                                                ErrorsFound,
                                                                Node::ConnectionObjectType::CoilCoolingWaterToAirHeatPumpParameterEstimation,
                                                                heatPump.Name,
                                                                Node::FluidType::Water,
                                                                Node::ConnectionType::Outlet,
                                                                Node::CompFluidStream::Secondary,
                                                                Node::ObjectIsNotParent);
                heatPump.AirInletNodeNum = GetOnlySingleNode(state,
                                                             airInletNodeName,
                                                             ErrorsFound,
                                                             Node::ConnectionObjectType::CoilCoolingWaterToAirHeatPumpParameterEstimation,
                                                             heatPump.Name,
                                                             Node::FluidType::Air,
                                                             Node::ConnectionType::Inlet,
                                                             Node::CompFluidStream::Primary,
                                                             Node::ObjectIsNotParent);
                heatPump.AirOutletNodeNum = GetOnlySingleNode(state,
                                                              airOutletNodeName,
                                                              ErrorsFound,
                                                              Node::ConnectionObjectType::CoilCoolingWaterToAirHeatPumpParameterEstimation,
                                                              heatPump.Name,
                                                              Node::FluidType::Air,
                                                              Node::ConnectionType::Outlet,
                                                              Node::CompFluidStream::Primary,
                                                              Node::ObjectIsNotParent);

                heatPump.LoadSideTotalUACoeff = s_ip->getRealFieldValue(fields, schemaProps, "load_side_total_heat_transfer_coefficient");
                heatPump.LoadSideOutsideUACoeff = s_ip->getRealFieldValue(fields, schemaProps, "load_side_outside_surface_heat_transfer_coefficient");
                if ((heatPump.LoadSideOutsideUACoeff < Constant::rTinyValue) || (heatPump.LoadSideTotalUACoeff < Constant::rTinyValue)) {
                    ShowSevereError(state, std::format("Input problem for {}={}", CurrentModuleObject, heatPump.Name));
                    ShowContinueError(state, " One or both load side UA values entered are below tolerance, likely zero or blank.");
                    ShowContinueError(state, " Verify inputs, as the parameter syntax for this object went through a change with");
                    ShowContinueError(state, "  the release of EnergyPlus version 5.");
                    ErrorsFound = true;
                }

                heatPump.SuperheatTemp = s_ip->getRealFieldValue(fields, schemaProps, "superheat_temperature_at_the_evaporator_outlet");
                heatPump.PowerLosses = s_ip->getRealFieldValue(fields, schemaProps, "compressor_power_losses");
                heatPump.LossFactor = s_ip->getRealFieldValue(fields, schemaProps, "compressor_efficiency");

                std::string const compType = s_ip->getAlphaFieldValue(fields, schemaProps, "compressor_type");
                heatPump.compressorType = static_cast<CompressorType>(getEnumValue(CompressTypeNamesUC, Util::makeUPPER(compType)));
                switch (heatPump.compressorType) {
                case CompressorType::Reciprocating: {
                    heatPump.CompPistonDisp = s_ip->getRealFieldValue(fields, schemaProps, "compressor_piston_displacement");
                    heatPump.CompSucPressDrop = s_ip->getRealFieldValue(fields, schemaProps, "compressor_suction_discharge_pressure_drop");
                    heatPump.CompClearanceFactor = s_ip->getRealFieldValue(fields, schemaProps, "compressor_clearance_factor");
                    break;
                }
                case CompressorType::Rotary: {
                    heatPump.CompPistonDisp = s_ip->getRealFieldValue(fields, schemaProps, "compressor_piston_displacement");
                    heatPump.CompSucPressDrop = s_ip->getRealFieldValue(fields, schemaProps, "compressor_suction_discharge_pressure_drop");
                    break;
                }
                case CompressorType::Scroll: {
                    heatPump.RefVolFlowRate = s_ip->getRealFieldValue(fields, schemaProps, "refrigerant_volume_flow_rate");
                    heatPump.VolumeRatio = s_ip->getRealFieldValue(fields, schemaProps, "volume_ratio");
                    heatPump.LeakRateCoeff = s_ip->getRealFieldValue(fields, schemaProps, "leak_rate_coefficient");
                    break;
                }
                default: {
                    ShowSevereInvalidKey(state, eoh, "Compressor Type", compType);
                    ErrorsFound = true;
                    break;
                }
                }
                heatPump.SourceSideUACoeff = s_ip->getRealFieldValue(fields, schemaProps, "source_side_heat_transfer_coefficient");
                heatPump.SourceSideHTR1 = s_ip->getRealFieldValue(fields, schemaProps, "source_side_heat_transfer_resistance1");
                heatPump.SourceSideHTR2 = s_ip->getRealFieldValue(fields, schemaProps, "source_side_heat_transfer_resistance2");
                cFieldName = "Part Load Fraction Correlation Curve Name";
                std::string const coolPLFCurveName = s_ip->getAlphaFieldValue(fields, schemaProps, "part_load_fraction_correlation_curve_name");
                if (coolPLFCurveName.empty()) {
                    ShowWarningEmptyField(state, eoh, cFieldName, "Required field is blank.");
                    ErrorsFound = true;
                } else if ((heatPump.PLFCurveIndex = Curve::GetCurveIndex(state, coolPLFCurveName)) == 0) {
                    ShowSevereItemNotFound(state, eoh, cFieldName, coolPLFCurveName);
                    ErrorsFound = true;
                } else if (Curve::CheckCurveDims(state, heatPump.PLFCurveIndex, {1}, RoutineName, CurrentModuleObject, heatPump.Name, cFieldName)) {
                    ShowSevereCustomField(state, eoh, cFieldName, coolPLFCurveName, "Illegal curve dimension.");
                    ErrorsFound = true;
                } else {
                    // Process curve data
                    // Test PLF curve minimum and maximum. Cap if less than 0.7 or greater than 1.0.
                    Real64 MinCurveVal = 999.0;
                    Real64 MaxCurveVal = -999.0;
                    Real64 CurveInput = 0.0;
                    Real64 MinCurvePLR{0.0};
                    Real64 MaxCurvePLR{0.0};

                    while (CurveInput <= 1.0) {
                        Real64 CurveVal = Curve::CurveValue(state, heatPump.PLFCurveIndex, CurveInput);
                        if (CurveVal < MinCurveVal) {
                            MinCurveVal = CurveVal;
                            MinCurvePLR = CurveInput;
                        }
                        if (CurveVal > MaxCurveVal) {
                            MaxCurveVal = CurveVal;
                            MaxCurvePLR = CurveInput;
                        }
                        CurveInput += 0.01;
                    }
                    if (MinCurveVal < 0.7) {
                        ShowSevereBadMin(
                            state, eoh, cFieldName, MinCurveVal, Clusive::In, 0.7, "Setting curve minimum to 0.7 and simulation continues.");
                        Curve::SetCurveOutputMinValue(state, heatPump.PLFCurveIndex, ErrorsFound, 0.7);
                    }
                    if (MaxCurveVal > 1.0) {
                        ShowSevereBadMax(
                            state, eoh, cFieldName, MaxCurveVal, Clusive::In, 1.0, "Setting curve maximum to 1.0 and simulation continues.");
                        Curve::SetCurveOutputMaxValue(state, heatPump.PLFCurveIndex, ErrorsFound, 1.0);
                    }
                }

                Node::TestCompSet(state, CurrentModuleObject, heatPump.Name, waterInletNodeName, waterOutletNodeName, "Water Nodes");
                Node::TestCompSet(state, CurrentModuleObject, heatPump.Name, airInletNodeName, airOutletNodeName, "Air Nodes");

                heatPump.MaxONOFFCyclesperHour = s_ip->getRealFieldValue(fields, schemaProps, "maximum_cycling_rate");
                heatPump.LatentCapacityTimeConstant = s_ip->getRealFieldValue(fields, schemaProps, "latent_capacity_time_constant");
                heatPump.FanDelayTime = s_ip->getRealFieldValue(fields, schemaProps, "fan_delay_time");

                // Setup Report variables for the detailed cooling Heat Pump
                // CurrentModuleObject = "Coil:Cooling:WaterToAirHeatPump:ParameterEstimation"
                SetupOutputVariable(state,
                                    "Cooling Coil Electricity Energy",
                                    Constant::Units::J,
                                    heatPump.Energy,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    heatPump.Name,
                                    Constant::eResource::Electricity,
                                    OutputProcessor::Group::HVAC,
                                    OutputProcessor::EndUseCat::Cooling);
                SetupOutputVariable(state,
                                    "Cooling Coil Total Cooling Energy",
                                    Constant::Units::J,
                                    heatPump.EnergyLoadTotal,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    heatPump.Name,
                                    Constant::eResource::EnergyTransfer,
                                    OutputProcessor::Group::HVAC,
                                    OutputProcessor::EndUseCat::CoolingCoils);
                SetupOutputVariable(state,
                                    "Cooling Coil Sensible Cooling Energy",
                                    Constant::Units::J,
                                    heatPump.EnergySensible,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    heatPump.Name);
                SetupOutputVariable(state,
                                    "Cooling Coil Latent Cooling Energy",
                                    Constant::Units::J,
                                    heatPump.EnergyLatent,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    heatPump.Name);
                SetupOutputVariable(state,
                                    "Cooling Coil Source Side Heat Transfer Energy",
                                    Constant::Units::J,
                                    heatPump.EnergySource,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    heatPump.Name,
                                    Constant::eResource::PlantLoopCoolingDemand,
                                    OutputProcessor::Group::HVAC,
                                    OutputProcessor::EndUseCat::CoolingCoils);

                // save the design source side flow rate for use by plant loop sizing algorithms
                PlantUtilities::RegisterPlantCompDesignFlow(state, heatPump.WaterInletNodeNum, 0.5 * heatPump.DesignWaterVolFlowRate);

                // create predefined report entries
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoolCoilType, heatPump.Name, CurrentModuleObject);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchCoolCoilTotCap, heatPump.Name, heatPump.CoolingCapacity);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoolCoilSensCap, heatPump.Name, "-");
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoolCoilLatCap, heatPump.Name, "-");
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoolCoilSHR, heatPump.Name, "-");
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoolCoilNomEff, heatPump.Name, "-");
            }
        }

        CurrentModuleObject = "Coil:Heating:WaterToAirHeatPump:ParameterEstimation";
        auto const instances_h = s_ip->epJSON.find(CurrentModuleObject);

        if (instances != s_ip->epJSON.end()) {
            auto const &schemaProps = s_ip->getObjectSchemaProps(state, CurrentModuleObject);
            auto &instancesValue = instances_h.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                std::string cFieldName;
                auto const &fields = instance.value();
                std::string const &thisObjectName = instance.key();
                s_ip->markObjectAsUsed(CurrentModuleObject, thisObjectName);
                ++HPNum;

                auto &heatPump = state.dataWaterToAirHeatPump->WatertoAirHP(HPNum);
                heatPump.Name = Util::makeUPPER(thisObjectName);
                heatPump.WatertoAirHPType = "HEATING";
                heatPump.WAHPType = DataPlant::PlantEquipmentType::CoilWAHPHeatingParamEst;
                ErrorObjectHeader eoh{routineName, CurrentModuleObject, heatPump.Name};
                GlobalNames::VerifyUniqueCoilName(
                    state, CurrentModuleObject, heatPump.Name, ErrorsFound, std::format("{} Name", CurrentModuleObject));
                std::string const availSchedName = s_ip->getAlphaFieldValue(fields, schemaProps, "availability_schedule_name");
                if (availSchedName.empty()) {
                    heatPump.availSched = Sched::GetScheduleAlwaysOn(state);
                } else if ((heatPump.availSched = Sched::GetSchedule(state, availSchedName)) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, "Availability Schedule Name", availSchedName);
                    ErrorsFound = true;
                }
                cFieldName = "Refrigerant Type";
                heatPump.Refrigerant = s_ip->getAlphaFieldValue(fields, schemaProps, "refrigerant_type"); // AlphArray(3);
                if (heatPump.Refrigerant.empty()) {
                    ShowSevereEmptyField(state, eoh, cFieldName);
                    ErrorsFound = true;
                } else if ((heatPump.refrig = Fluid::GetRefrig(state, heatPump.Refrigerant)) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cFieldName, heatPump.Refrigerant);
                    ErrorsFound = true;
                }

                heatPump.DesignWaterVolFlowRate = s_ip->getRealFieldValue(fields, schemaProps, "design_source_side_flow_rate");
                heatPump.HeatingCapacity = s_ip->getRealFieldValue(fields, schemaProps, "gross_rated_heating_capacity");
                heatPump.HighPressCutoff = s_ip->getRealFieldValue(fields, schemaProps, "high_pressure_cutoff");
                heatPump.LowPressCutoff = s_ip->getRealFieldValue(fields, schemaProps, "low_pressure_cutoff");

                std::string waterInletNodeName = s_ip->getAlphaFieldValue(fields, schemaProps, "water_inlet_node_name");
                std::string waterOutletNodeName = s_ip->getAlphaFieldValue(fields, schemaProps, "water_outlet_node_name");
                std::string airInletNodeName = s_ip->getAlphaFieldValue(fields, schemaProps, "air_inlet_node_name");
                std::string airOutletNodeName = s_ip->getAlphaFieldValue(fields, schemaProps, "air_outlet_node_name");

                heatPump.WaterInletNodeNum = GetOnlySingleNode(state,
                                                               waterInletNodeName,
                                                               ErrorsFound,
                                                               Node::ConnectionObjectType::CoilCoolingWaterToAirHeatPumpParameterEstimation,
                                                               heatPump.Name,
                                                               Node::FluidType::Water,
                                                               Node::ConnectionType::Inlet,
                                                               Node::CompFluidStream::Secondary,
                                                               Node::ObjectIsNotParent);
                heatPump.WaterOutletNodeNum = GetOnlySingleNode(state,
                                                                waterOutletNodeName,
                                                                ErrorsFound,
                                                                Node::ConnectionObjectType::CoilCoolingWaterToAirHeatPumpParameterEstimation,
                                                                heatPump.Name,
                                                                Node::FluidType::Water,
                                                                Node::ConnectionType::Outlet,
                                                                Node::CompFluidStream::Secondary,
                                                                Node::ObjectIsNotParent);
                heatPump.AirInletNodeNum = GetOnlySingleNode(state,
                                                             airInletNodeName,
                                                             ErrorsFound,
                                                             Node::ConnectionObjectType::CoilCoolingWaterToAirHeatPumpParameterEstimation,
                                                             heatPump.Name,
                                                             Node::FluidType::Air,
                                                             Node::ConnectionType::Inlet,
                                                             Node::CompFluidStream::Primary,
                                                             Node::ObjectIsNotParent);
                heatPump.AirOutletNodeNum = GetOnlySingleNode(state,
                                                              airOutletNodeName,
                                                              ErrorsFound,
                                                              Node::ConnectionObjectType::CoilCoolingWaterToAirHeatPumpParameterEstimation,
                                                              heatPump.Name,
                                                              Node::FluidType::Air,
                                                              Node::ConnectionType::Outlet,
                                                              Node::CompFluidStream::Primary,
                                                              Node::ObjectIsNotParent);

                heatPump.LoadSideTotalUACoeff = s_ip->getRealFieldValue(fields, schemaProps, "load_side_total_heat_transfer_coefficient");
                if (heatPump.LoadSideTotalUACoeff < Constant::rTinyValue) {
                    ShowSevereError(state, std::format("Input problem for {}={}", CurrentModuleObject, heatPump.Name));
                    ShowContinueError(state, " Load side UA value is less than tolerance, likely zero or blank.");
                    ShowContinueError(state, " Verify inputs, as the parameter syntax for this object went through a change with");
                    ShowContinueError(state, "  the release of EnergyPlus version 5.");
                    ErrorsFound = true;
                }

                heatPump.SuperheatTemp = s_ip->getRealFieldValue(fields, schemaProps, "superheat_temperature_at_the_evaporator_outlet");
                heatPump.PowerLosses = s_ip->getRealFieldValue(fields, schemaProps, "compressor_power_losses");
                heatPump.LossFactor = s_ip->getRealFieldValue(fields, schemaProps, "compressor_efficiency");

                std::string const compType = s_ip->getAlphaFieldValue(fields, schemaProps, "compressor_type");
                heatPump.compressorType = static_cast<CompressorType>(getEnumValue(CompressTypeNamesUC, Util::makeUPPER(compType)));
                switch (heatPump.compressorType) {
                case CompressorType::Reciprocating: {
                    heatPump.CompPistonDisp = s_ip->getRealFieldValue(fields, schemaProps, "compressor_piston_displacement");
                    heatPump.CompSucPressDrop = s_ip->getRealFieldValue(fields, schemaProps, "compressor_suction_discharge_pressure_drop");
                    heatPump.CompClearanceFactor = s_ip->getRealFieldValue(fields, schemaProps, "compressor_clearance_factor");
                    break;
                }
                case CompressorType::Rotary: {
                    heatPump.CompPistonDisp = s_ip->getRealFieldValue(fields, schemaProps, "compressor_piston_displacement");
                    heatPump.CompSucPressDrop = s_ip->getRealFieldValue(fields, schemaProps, "compressor_suction_discharge_pressure_drop");
                    break;
                }
                case CompressorType::Scroll: {
                    heatPump.RefVolFlowRate = s_ip->getRealFieldValue(fields, schemaProps, "refrigerant_volume_flow_rate");
                    heatPump.VolumeRatio = s_ip->getRealFieldValue(fields, schemaProps, "volume_ratio");
                    heatPump.LeakRateCoeff = s_ip->getRealFieldValue(fields, schemaProps, "leak_rate_coefficient");
                    break;
                }
                default: {
                    ShowSevereInvalidKey(state, eoh, "Compressor Type", compType);
                    ErrorsFound = true;
                    break;
                }
                }
                heatPump.SourceSideUACoeff = s_ip->getRealFieldValue(fields, schemaProps, "source_side_heat_transfer_coefficient");
                heatPump.SourceSideHTR1 = s_ip->getRealFieldValue(fields, schemaProps, "source_side_heat_transfer_resistance1");
                heatPump.SourceSideHTR2 = s_ip->getRealFieldValue(fields, schemaProps, "source_side_heat_transfer_resistance2");
                cFieldName = "Part Load Fraction Correlation Curve Name";
                std::string const coolPLFCurveName = s_ip->getAlphaFieldValue(fields, schemaProps, "part_load_fraction_correlation_curve_name");
                if (coolPLFCurveName.empty()) {
                    ShowWarningEmptyField(state, eoh, cFieldName, "Required field is blank.");
                    ErrorsFound = true;
                } else if ((heatPump.PLFCurveIndex = Curve::GetCurveIndex(state, coolPLFCurveName)) == 0) {
                    ShowSevereItemNotFound(state, eoh, cFieldName, coolPLFCurveName);
                    ErrorsFound = true;
                } else if (Curve::CheckCurveDims(state, heatPump.PLFCurveIndex, {1}, RoutineName, CurrentModuleObject, heatPump.Name, cFieldName)) {
                    ShowSevereCustomField(state, eoh, cFieldName, coolPLFCurveName, "Illegal curve dimension.");
                    ErrorsFound = true;
                } else {
                    // Process curve data
                    // Test PLF curve minimum and maximum. Cap if less than 0.7 or greater than 1.0.
                    Real64 MinCurveVal = 999.0;
                    Real64 MaxCurveVal = -999.0;
                    Real64 CurveInput = 0.0;
                    Real64 MinCurvePLR{0.0};
                    Real64 MaxCurvePLR{0.0};

                    while (CurveInput <= 1.0) {
                        Real64 CurveVal = Curve::CurveValue(state, heatPump.PLFCurveIndex, CurveInput);
                        if (CurveVal < MinCurveVal) {
                            MinCurveVal = CurveVal;
                            MinCurvePLR = CurveInput;
                        }
                        if (CurveVal > MaxCurveVal) {
                            MaxCurveVal = CurveVal;
                            MaxCurvePLR = CurveInput;
                        }
                        CurveInput += 0.01;
                    }
                    if (MinCurveVal < 0.7) {
                        ShowSevereBadMin(
                            state, eoh, cFieldName, MinCurveVal, Clusive::In, 0.7, "Setting curve minimum to 0.7 and simulation continues.");
                        Curve::SetCurveOutputMinValue(state, heatPump.PLFCurveIndex, ErrorsFound, 0.7);
                    }
                    if (MaxCurveVal > 1.0) {
                        ShowSevereBadMax(
                            state, eoh, cFieldName, MaxCurveVal, Clusive::In, 1.0, "Setting curve maximum to 1.0 and simulation continues.");
                        Curve::SetCurveOutputMaxValue(state, heatPump.PLFCurveIndex, ErrorsFound, 1.0);
                    }
                }

                Node::TestCompSet(state, CurrentModuleObject, heatPump.Name, waterInletNodeName, waterOutletNodeName, "Water Nodes");
                Node::TestCompSet(state, CurrentModuleObject, heatPump.Name, airInletNodeName, airOutletNodeName, "Air Nodes");

                // CurrentModuleObject = "Coil:Heating:WaterToAirHeatPump:ParameterEstimation"
                SetupOutputVariable(state,
                                    "Heating Coil Electricity Energy",
                                    Constant::Units::J,
                                    heatPump.Energy,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    heatPump.Name,
                                    Constant::eResource::Electricity,
                                    OutputProcessor::Group::HVAC,
                                    OutputProcessor::EndUseCat::Heating);
                SetupOutputVariable(state,
                                    "Heating Coil Heating Energy",
                                    Constant::Units::J,
                                    heatPump.EnergyLoadTotal,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    heatPump.Name,
                                    Constant::eResource::EnergyTransfer,
                                    OutputProcessor::Group::HVAC,
                                    OutputProcessor::EndUseCat::HeatingCoils);
                SetupOutputVariable(state,
                                    "Heating Coil Source Side Heat Transfer Energy",
                                    Constant::Units::J,
                                    heatPump.EnergySource,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    heatPump.Name,
                                    Constant::eResource::PlantLoopHeatingDemand,
                                    OutputProcessor::Group::HVAC,
                                    OutputProcessor::EndUseCat::HeatingCoils);

                // save the design source side flow rate for use by plant loop sizing algorithms
                PlantUtilities::RegisterPlantCompDesignFlow(state, heatPump.WaterInletNodeNum, 0.5 * heatPump.DesignWaterVolFlowRate);

                // create predefined report entries
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchHeatCoilType, heatPump.Name, CurrentModuleObject);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchHeatCoilNomCap, heatPump.Name, heatPump.HeatingCapacity);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchHeatCoilNomEff, heatPump.Name, "-");
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, std::format("{}Errors found getting input. Program terminates.", RoutineName));
        }

        for (HPNum = 1; HPNum <= state.dataWaterToAirHeatPump->NumWatertoAirHPs; ++HPNum) {

            auto &heatPump = state.dataWaterToAirHeatPump->WatertoAirHP(HPNum);
            if (heatPump.WAHPType == DataPlant::PlantEquipmentType::CoilWAHPCoolingParamEst) {
                // COOLING COIL: Setup Report variables for the Heat Pump
                SetupOutputVariable(state,
                                    "Cooling Coil Electricity Rate",
                                    Constant::Units::W,
                                    heatPump.Power,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);

                SetupOutputVariable(state,
                                    "Cooling Coil Total Cooling Rate",
                                    Constant::Units::W,
                                    heatPump.QLoadTotal,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);

                SetupOutputVariable(state,
                                    "Cooling Coil Sensible Cooling Rate",
                                    Constant::Units::W,
                                    heatPump.QSensible,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);

                SetupOutputVariable(state,
                                    "Cooling Coil Latent Cooling Rate",
                                    Constant::Units::W,
                                    heatPump.QLatent,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);

                SetupOutputVariable(state,
                                    "Cooling Coil Source Side Heat Transfer Rate",
                                    Constant::Units::W,
                                    heatPump.QSource,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);

                SetupOutputVariable(state,
                                    "Cooling Coil Part Load Ratio",
                                    Constant::Units::None,
                                    heatPump.PartLoadRatio,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);
                SetupOutputVariable(state,
                                    "Cooling Coil Runtime Fraction",
                                    Constant::Units::None,
                                    heatPump.RunFrac,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);

                SetupOutputVariable(state,
                                    "Cooling Coil Air Mass Flow Rate",
                                    Constant::Units::kg_s,
                                    heatPump.OutletAirMassFlowRate,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);
                SetupOutputVariable(state,
                                    "Cooling Coil Air Inlet Temperature",
                                    Constant::Units::C,
                                    heatPump.InletAirDBTemp,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);
                SetupOutputVariable(state,
                                    "Cooling Coil Air Inlet Humidity Ratio",
                                    Constant::Units::kgWater_kgDryAir,
                                    heatPump.InletAirHumRat,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);
                SetupOutputVariable(state,
                                    "Cooling Coil Air Outlet Temperature",
                                    Constant::Units::C,
                                    heatPump.OutletAirDBTemp,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);
                SetupOutputVariable(state,
                                    "Cooling Coil Air Outlet Humidity Ratio",
                                    Constant::Units::kgWater_kgDryAir,
                                    heatPump.OutletAirHumRat,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);

                SetupOutputVariable(state,
                                    "Cooling Coil Source Side Mass Flow Rate",
                                    Constant::Units::kg_s,
                                    heatPump.OutletWaterMassFlowRate,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);
                SetupOutputVariable(state,
                                    "Cooling Coil Source Side Inlet Temperature",
                                    Constant::Units::C,
                                    heatPump.InletWaterTemp,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);
                SetupOutputVariable(state,
                                    "Cooling Coil Source Side Outlet Temperature",
                                    Constant::Units::C,
                                    heatPump.OutletWaterTemp,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);
            } else if (heatPump.WAHPType == DataPlant::PlantEquipmentType::CoilWAHPHeatingParamEst) {
                // HEATING COIL Setup Report variables for the Heat Pump
                SetupOutputVariable(state,
                                    "Heating Coil Electricity Rate",
                                    Constant::Units::W,
                                    heatPump.Power,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);

                SetupOutputVariable(state,
                                    "Heating Coil Heating Rate",
                                    Constant::Units::W,
                                    heatPump.QLoadTotal,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);

                SetupOutputVariable(state,
                                    "Heating Coil Sensible Heating Rate",
                                    Constant::Units::W,
                                    heatPump.QSensible,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);

                SetupOutputVariable(state,
                                    "Heating Coil Source Side Heat Transfer Rate",
                                    Constant::Units::W,
                                    heatPump.QSource,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);

                SetupOutputVariable(state,
                                    "Heating Coil Part Load Ratio",
                                    Constant::Units::None,
                                    heatPump.PartLoadRatio,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);
                SetupOutputVariable(state,
                                    "Heating Coil Runtime Fraction",
                                    Constant::Units::None,
                                    heatPump.RunFrac,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);

                SetupOutputVariable(state,
                                    "Heating Coil Air Mass Flow Rate",
                                    Constant::Units::kg_s,
                                    heatPump.OutletAirMassFlowRate,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);
                SetupOutputVariable(state,
                                    "Heating Coil Air Inlet Temperature",
                                    Constant::Units::C,
                                    heatPump.InletAirDBTemp,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);
                SetupOutputVariable(state,
                                    "Heating Coil Air Inlet Humidity Ratio",
                                    Constant::Units::kgWater_kgDryAir,
                                    heatPump.InletAirHumRat,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);
                SetupOutputVariable(state,
                                    "Heating Coil Air Outlet Temperature",
                                    Constant::Units::C,
                                    heatPump.OutletAirDBTemp,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);
                SetupOutputVariable(state,
                                    "Heating Coil Air Outlet Humidity Ratio",
                                    Constant::Units::kgWater_kgDryAir,
                                    heatPump.OutletAirHumRat,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);

                SetupOutputVariable(state,
                                    "Heating Coil Source Side Mass Flow Rate",
                                    Constant::Units::kg_s,
                                    heatPump.OutletWaterMassFlowRate,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);
                SetupOutputVariable(state,
                                    "Heating Coil Source Side Inlet Temperature",
                                    Constant::Units::C,
                                    heatPump.InletWaterTemp,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);
                SetupOutputVariable(state,
                                    "Heating Coil Source Side Outlet Temperature",
                                    Constant::Units::C,
                                    heatPump.OutletWaterTemp,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    heatPump.Name);
            }
        }
    }

    void InitWatertoAirHP(EnergyPlusData &state,
                          int const HPNum, // index to main heat pump data structure
                          bool const InitFlag,
                          Real64 const SensLoad,
                          Real64 const LatentLoad,
                          Real64 const DesignAirFlow,
                          Real64 const PartLoadRatio)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Hui Jin
        //       DATE WRITTEN   Oct 2000
        //       MODIFIED       Dan Fisher, Kenneth Tang (Jan 2004)
        //                      Brent Griffith, Sept 2010, plant upgrades, general fluid properties

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the Water to Air HP Components.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations.

        // Using/Aliasing
        auto &heatPump = state.dataWaterToAirHeatPump->WatertoAirHP(HPNum);

        static constexpr std::string_view RoutineName("InitWatertoAirHP");
        int WaterInletNode = heatPump.WaterInletNodeNum;

        if (state.dataWaterToAirHeatPump->MyOneTimeFlag) {
            state.dataWaterToAirHeatPump->MyEnvrnFlag.allocate(state.dataWaterToAirHeatPump->NumWatertoAirHPs);
            state.dataWaterToAirHeatPump->MyPlantScanFlag.allocate(state.dataWaterToAirHeatPump->NumWatertoAirHPs);
            state.dataWaterToAirHeatPump->MyEnvrnFlag = true;
            state.dataWaterToAirHeatPump->MyPlantScanFlag = true;
            state.dataWaterToAirHeatPump->MyOneTimeFlag = false;
        }

        if (state.dataWaterToAirHeatPump->MyPlantScanFlag(HPNum) && allocated(state.dataPlnt->PlantLoop)) {
            bool errFlag = false;
            PlantUtilities::ScanPlantLoopsForObject(state, heatPump.Name, heatPump.WAHPType, heatPump.plantLoc, errFlag, _, _, _, _, _);

            if (heatPump.plantLoc.loop->FluidName == "WATER") {
                if (heatPump.SourceSideUACoeff < Constant::rTinyValue) {
                    ShowSevereError(state, std::format("Input problem for water to air heat pump, \"{}\".", heatPump.Name));
                    ShowContinueError(state, " Source side UA value is less than tolerance, likely zero or blank.");
                    ShowContinueError(state, " Verify inputs, as the parameter syntax for this object went through a change with");
                    ShowContinueError(state, "  the release of EnergyPlus version 5.");
                    errFlag = true;
                }
            } else {
                if ((heatPump.SourceSideHTR1 < Constant::rTinyValue) || (heatPump.SourceSideHTR2 < Constant::rTinyValue)) {
                    ShowSevereError(state, std::format("Input problem for water to air heat pump, \"{}\".", heatPump.Name));
                    ShowContinueError(state, " A source side heat transfer resistance value is less than tolerance, likely zero or blank.");
                    ShowContinueError(state, " Verify inputs, as the parameter syntax for this object went through a change with");
                    ShowContinueError(state, "  the release of EnergyPlus version 5.");
                    errFlag = true;
                }
            }

            if (errFlag) {
                ShowFatalError(state, "InitWatertoAirHP: Program terminated for previous conditions.");
            }

            state.dataWaterToAirHeatPump->MyPlantScanFlag(HPNum) = false;
        }

        // Do the Begin Environment initializations
        if (state.dataGlobal->BeginEnvrnFlag && state.dataWaterToAirHeatPump->MyEnvrnFlag(HPNum) &&
            !state.dataWaterToAirHeatPump->MyPlantScanFlag(HPNum)) {

            // Initialize all report variables to a known state at beginning of simulation
            heatPump.Power = 0.0;
            heatPump.Energy = 0.0;
            heatPump.QLoadTotal = 0.0;
            heatPump.QSensible = 0.0;
            heatPump.QLatent = 0.0;
            heatPump.QSource = 0.0;
            heatPump.EnergyLoadTotal = 0.0;
            heatPump.EnergySensible = 0.0;
            heatPump.EnergyLatent = 0.0;
            heatPump.EnergySource = 0.0;
            heatPump.RunFrac = 0.0;
            heatPump.PartLoadRatio = 0.0;
            heatPump.OutletAirDBTemp = 0.0;
            heatPump.OutletAirHumRat = 0.0;
            heatPump.InletAirDBTemp = 0.0;
            heatPump.InletAirHumRat = 0.0;
            heatPump.OutletWaterTemp = 0.0;
            heatPump.InletWaterTemp = 0.0;
            heatPump.InletAirMassFlowRate = 0.0;
            heatPump.InletWaterMassFlowRate = 0.0;
            heatPump.OutletAirEnthalpy = 0.0;
            heatPump.OutletWaterEnthalpy = 0.0;

            // The rest of the one time initializations
            Real64 rho = heatPump.plantLoc.loop->glycol->getDensity(state, Constant::InitConvTemp, RoutineName);
            Real64 Cp = heatPump.plantLoc.loop->glycol->getSpecificHeat(state, Constant::InitConvTemp, RoutineName);

            heatPump.DesignWaterMassFlowRate = rho * heatPump.DesignWaterVolFlowRate;

            int PlantOutletNode = DataPlant::CompData::getPlantComponent(state, heatPump.plantLoc).NodeNumOut;
            PlantUtilities::InitComponentNodes(state, 0.0, heatPump.DesignWaterMassFlowRate, WaterInletNode, PlantOutletNode);

            state.dataLoopNodes->Node(WaterInletNode).Temp = 5.0;
            state.dataLoopNodes->Node(WaterInletNode).Enthalpy = Cp * state.dataLoopNodes->Node(WaterInletNode).Temp;
            state.dataLoopNodes->Node(WaterInletNode).Quality = 0.0;
            state.dataLoopNodes->Node(WaterInletNode).Press = 0.0;
            state.dataLoopNodes->Node(WaterInletNode).HumRat = 0.0;

            state.dataLoopNodes->Node(PlantOutletNode).Temp = 5.0;
            state.dataLoopNodes->Node(PlantOutletNode).Enthalpy = Cp * state.dataLoopNodes->Node(WaterInletNode).Temp;
            state.dataLoopNodes->Node(PlantOutletNode).Quality = 0.0;
            state.dataLoopNodes->Node(PlantOutletNode).Press = 0.0;
            state.dataLoopNodes->Node(PlantOutletNode).HumRat = 0.0;

            heatPump.SimFlag = true;

            state.dataWaterToAirHeatPump->MyEnvrnFlag(HPNum) = false;
        } // End If for the Begin Environment initializations

        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataWaterToAirHeatPump->MyEnvrnFlag(HPNum) = true;
        }

        // Do the following initializations (every time step): This should be the info from
        // the previous components outlets or the node data in this section.
        // First set the conditions for the air into the heat pump model

        // Set water and air inlet nodes
        int AirInletNode = heatPump.AirInletNodeNum;

        if (((SensLoad != 0.0 || LatentLoad != 0.0) || (SensLoad == 0.0 && InitFlag)) && state.dataLoopNodes->Node(AirInletNode).MassFlowRate > 0.0 &&
            PartLoadRatio > 0.0 && (heatPump.availSched->getCurrentVal() > 0.0)) {
            // set the water side flow rate to the design flow rate unless constrained by
            // the demand side manager (MIN/MAX available). now done by call to setcomponentFlowRate
            heatPump.InletWaterMassFlowRate = heatPump.DesignWaterMassFlowRate;
            heatPump.InletAirMassFlowRate = DesignAirFlow; // This is required instead of the node temperature
            // because the air loop operates handles part load for
            // cycling equipment by modulating the air flow rate
            // the heat pump model requires an accurate (i.e. full load
            // flow rate for accurate simulation.
        } else { // heat pump is off
            heatPump.InletWaterMassFlowRate = 0.0;

            heatPump.InletAirMassFlowRate = 0.0;
        }
        // constrain water flow provided by plant
        PlantUtilities::SetComponentFlowRate(
            state, heatPump.InletWaterMassFlowRate, heatPump.WaterInletNodeNum, heatPump.WaterOutletNodeNum, heatPump.plantLoc);

        heatPump.InletWaterTemp = state.dataLoopNodes->Node(WaterInletNode).Temp;
        //  IF (WatertoAirHP(HPNum)%InletWaterTemp < 0.0) THEN  ! Debug trap
        //    Temptemp         = Node(WaterInletNode)%Temp
        //  ENDIF
        heatPump.InletWaterEnthalpy = state.dataLoopNodes->Node(WaterInletNode).Enthalpy;

        heatPump.InletAirDBTemp = state.dataLoopNodes->Node(AirInletNode).Temp;
        heatPump.InletAirHumRat = state.dataLoopNodes->Node(AirInletNode).HumRat;
        heatPump.InletAirEnthalpy = state.dataLoopNodes->Node(AirInletNode).Enthalpy;

        heatPump.Power = 0.0;
        heatPump.Energy = 0.0;
        heatPump.QLoadTotal = 0.0;
        heatPump.QSensible = 0.0;
        heatPump.QLatent = 0.0;
        heatPump.QSource = 0.0;
        heatPump.EnergyLoadTotal = 0.0;
        heatPump.EnergySensible = 0.0;
        heatPump.EnergyLatent = 0.0;
        heatPump.EnergySource = 0.0;
        heatPump.RunFrac = 0.0;
        heatPump.OutletAirDBTemp = 0.0;
        heatPump.OutletAirHumRat = 0.0;
        heatPump.OutletWaterTemp = 0.0;
        heatPump.OutletAirEnthalpy = 0.0;
        heatPump.OutletWaterEnthalpy = 0.0;
    }

    void CalcWatertoAirHPCooling(EnergyPlusData &state,
                                 int const HPNum,                      // heat pump number
                                 HVAC::FanOp const fanOp,              // fan/compressor cycling scheme indicator
                                 bool const FirstHVACIteration,        // first iteration flag
                                 [[maybe_unused]] bool const InitFlag, // suppress property errors if true
                                 Real64 const SensDemand,
                                 HVAC::CompressorOp const compressorOp,
                                 Real64 const PartLoadRatio)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Hui Jin
        //       DATE WRITTEN   Oct 2000
        //       MODIFIED       Dan Fisher, Kenneth Tang (Jan 2004), R. Raustad (Oct 2006) Revised iteration technique

        // PURPOSE OF THIS SUBROUTINE:
        // Simulates a parameter estimation based water to air heat pump model

        // Using/Aliasing
        auto &heatPump = state.dataWaterToAirHeatPump->WatertoAirHP(HPNum);

        // SUBROUTINE PARAMETER DEFINITIONS:
        constexpr Real64 CpWater(4210.0);         // Specific heat of water J/kg_C
        constexpr Real64 DegreeofSuperheat(80.0); // Initial guess of degree of superheat
        constexpr Real64 gamma(1.114);            // Expansion Coefficient
        constexpr Real64 ERR(0.01);               // Error Value
        constexpr Real64 PB(1.013e5);             // Barometric Pressure (Pa)

        constexpr int STOP1(1000); // Iteration stopper1
        constexpr int STOP2(1000); // Iteration stopper2
        constexpr int STOP3(1000); // Iteration stopper3

        static constexpr std::string_view RoutineNameSourceSideInletTemp("CalcWatertoAirHPCooling:SourceSideInletTemp");
        static constexpr std::string_view RoutineNameSourceSideTemp("CalcWatertoAirHPCooling:SourceSideTemp");
        static constexpr std::string_view RoutineNameLoadSideTemp("CalcWatertoAirHPCooling:LoadSideTemp");
        static constexpr std::string_view RoutineNameLoadSideSurfaceTemp("CalcWatertoAirHPCooling:LoadSideSurfaceTemp");
        static constexpr std::string_view RoutineNameLoadSideEvapTemp("CalcWatertoAirHPCooling:LoadSideEvapTemp");
        static constexpr std::string_view RoutineNameLoadSideOutletEnthalpy("CalcWatertoAirHPCooling:LoadSideOutletEnthalpy");
        static constexpr std::string_view RoutineNameCompressInletTemp("CalcWatertoAirHPCooling:CompressInletTemp");
        static constexpr std::string_view RoutineNameSuctionPr("CalcWatertoAirHPCooling:SuctionPr");
        static constexpr std::string_view RoutineNameCompSuctionTemp("CalcWatertoAirHPCooling:CompSuctionTemp");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumIteration3;                // Number of Iteration3
        int NumIteration4;                // Number of Iteration4 (use of latent degradation model ONLY)
        Real64 Quality;                   // Quality of Refrigerant
        Real64 SourceSideOutletTemp;      // Source Side Outlet Temperature [C]
        Real64 SourceSideVolFlowRate;     // Source Side Volumetric Flow Rate [m3/s]
        Real64 DegradFactor;              // Degradation Factor [~]
        Real64 CpFluid;                   // Specific heat of source side fluid(J/kg)
        Real64 LoadSideInletWBTemp;       // Wet-bulb temperature of indoor inlet air [C]
        Real64 LoadSideInletDBTemp;       // Load Side Inlet Dry Bulb Temp [C]
        Real64 LoadSideInletHumRat;       // Load Side Inlet Humidity Ratio [kg/kg]
        Real64 LoadSideOutletDBTemp;      // Load Side Outlet Dry Bulb Temperature [C]
        Real64 LoadSideOutletHumRat;      // Load Side Outlet Humidity Ratio [kg/kg]
        Real64 LoadSideAirInletEnth;      // Load Side Inlet Enthalpy [J/kg]
        Real64 LoadSideAirOutletEnth;     // Load Side Outlet Enthalpy [J/kg]
        Real64 EffectiveSurfaceTemp;      // Effective Surface Temperature [C]
        Real64 EffectiveSatEnth;          // Saturated Enthalpy of Air Corresponding to the Effective Surface Temperature [J/kg]
        Real64 QSource = 0.0;             // Source Side Heat Transfer Rate [W]
        Real64 QLoadTotal = 0.0;          // Load Side Total Heat Transfer Rate [W]
        Real64 QSensible = 0.0;           // Load Side Sensible Heat Transfer Rate [W]
        Real64 Power = 0.0;               // Power Consumption [W]
        Real64 EvapTemp;                  // Evaporating Temperature [C]
        Real64 ANTUWET;                   // Number of Transfer Unit for Wet Condition
        Real64 EffectWET;                 // Load Side Heat Exchanger Effectiveness
        Real64 EvapSatEnth;               // Saturated Enthalpy of Air Corresponding to the Evaporating Temperature [J/kg]
        Real64 SourceSideEffect;          // Source Side Heat Exchanger Effectiveness
        Real64 SourceSideTemp;            // Source Side Saturated Refrigerant Temperature [C]
        Real64 LoadSideTemp;              // Load Side Saturated Refrigerant Temperature [C]
        Real64 SourceSidePressure;        // Source Side Saturated Refrigerant Pressure [Pa]
        Real64 LoadSidePressure;          // Load Side Saturated Refrigerant Pressure [Pa]
        Real64 SuctionPr = 0.0;           // Compressor Suction Pressure [Pa]
        Real64 DischargePr = 0.0;         // Compressor Discharge Pressure [Pa]
        Real64 CompressInletTemp;         // Temperature of the Refrigerant Entering the Compressor [C]
        Real64 MassRef = 0.0;             // Mass Flow Rate of Refrigerant [kg/s]
        Real64 SourceSideOutletEnth;      // Enthalpy of Refrigerant leaving the Source Side Heat Exchanger [J/kg]
        Real64 LoadSideOutletEnth;        // Enthalpy of Refrigerant leaving the Load Side Heat Exchanger [J/kg]
        Real64 CpAir;                     // Specific Heat of Air [J/kg_C]
        Real64 SuperHeatEnth;             // Enthalpy of the Superheated Refrigerant [J/kg]
        Real64 CompSuctionTemp1 = 0.0;    // Guess of the Temperature of the Refrigerant Entering the Compressor #1 [C]
        Real64 CompSuctionTemp2 = 0.0;    // Guess of the Temperature of the Refrigerant Entering the Compressor #2 [C]
        Real64 CompSuctionEnth;           // Enthalpy of the Refrigerant Entering the Compressor [J/kg]
        Real64 CompSuctionDensity = 0.0;  // Density of the Refrigerant Entering the Compressor [kg/m3]
        Real64 CompSuctionSatTemp;        // Temperature of Saturated Refrigerant at Compressor Suction Pressure [C]
        bool LatDegradModelSimFlag;       // Latent degradation model simulation flag
        bool StillSimulatingFlag;         // Final Simulation Flag
        bool Converged;                   // overall convergence Flag
        Real64 QLatRated;                 // Qlatent at rated conditions of indoor(TDB,TWB)=(26.7C,19.4C)
        Real64 QLatActual;                // Qlatent at actual operating conditions
        Real64 SHRss;                     // Sensible heat ratio at steady state
        Real64 SHReff;                    // Effective sensible heat ratio at part-load condition
        int SolFlag;                      // Solution flag returned from RegulaFalsi function
        Real64 LoadSideAirInletEnth_Unit; // calc conditions for unit
        Real64 LoadResidual;              // loop convergence criteria
        Real64 SourceResidual;            // loop convergence criteria
        Real64 RelaxParam(0.5);           // Relaxation Parameter

        if (state.dataWaterToAirHeatPump->firstTime) {
            // Set indoor air conditions to the rated condition
            state.dataWaterToAirHeatPump->LoadSideInletDBTemp_Init = 26.7;
            state.dataWaterToAirHeatPump->LoadSideInletHumRat_Init = 0.0111;
            state.dataWaterToAirHeatPump->LoadSideAirInletEnth_Init = Psychrometrics::PsyHFnTdbW(
                state.dataWaterToAirHeatPump->LoadSideInletDBTemp_Init, state.dataWaterToAirHeatPump->LoadSideInletHumRat_Init);
            state.dataWaterToAirHeatPump->firstTime = false;
        }

        //  SET LOCAL VARIABLES FROM DATA STRUCTURE (for code readability)
        // Set indoor air conditions to the actual condition
        CpAir = Psychrometrics::PsyCpAirFnW(heatPump.InletAirHumRat);
        LoadSideAirInletEnth_Unit = Psychrometrics::PsyHFnTdbW(heatPump.InletAirDBTemp, heatPump.InletAirHumRat);
        SourceSideVolFlowRate = heatPump.InletWaterMassFlowRate /
                                heatPump.plantLoc.loop->glycol->getDensity(state, heatPump.InletWaterTemp, RoutineNameSourceSideInletTemp);

        StillSimulatingFlag = true;

        // If heat pump is not operating, return
        if (SensDemand == 0.0 || heatPump.InletAirMassFlowRate <= 0.0 || heatPump.InletWaterMassFlowRate <= 0.0 ||
            (heatPump.availSched->getCurrentVal() <= 0.0)) {
            heatPump.SimFlag = false;
            return;
        }
        heatPump.SimFlag = true;

        if (compressorOp == HVAC::CompressorOp::Off) {
            heatPump.SimFlag = false;
            return;
        }

        if (FirstHVACIteration) {
            state.dataWaterToAirHeatPump->initialQSource_calc = heatPump.CoolingCapacity;
            state.dataWaterToAirHeatPump->initialQLoadTotal_calc = heatPump.CoolingCapacity;
        }

        if (state.dataWaterToAirHeatPump->initialQLoadTotal_calc == 0.0) {
            state.dataWaterToAirHeatPump->initialQLoadTotal_calc = heatPump.CoolingCapacity;
        }
        if (state.dataWaterToAirHeatPump->initialQSource_calc == 0.0) {
            state.dataWaterToAirHeatPump->initialQSource_calc = heatPump.CoolingCapacity;
        }

        // Loop the calculation at least twice depending whether the latent degradation model
        // is enabled. 1st iteration to calculate the QLatent(rated) at (TDB,TWB)indoorair=(26.7C,19.4C)
        // and 2nd iteration to calculate the  QLatent(actual)

        // Calculate Part Load Factor and Runtime Fraction
        Real64 PLF = 1.0; // part load factor as a function of PLR, RTF = PLR / PLF
        if (heatPump.PLFCurveIndex > 0) {
            PLF = Curve::CurveValue(state, heatPump.PLFCurveIndex, PartLoadRatio); // Calculate part-load factor
        }
        if (fanOp == HVAC::FanOp::Cycling) {
            state.dataHVACGlobal->OnOffFanPartLoadFraction = PLF;
        }
        heatPump.RunFrac = PartLoadRatio / PLF;

        QLatRated = 0.0;
        QLatActual = 0.0;
        // IF((RuntimeFrac .GE. 1.0) .OR. (Twet_rated .LE. 0.0) .OR. (Gamma_rated .LE. 0.0)) THEN
        // Cycling fan does not required latent degradation model, only the constant fan case
        if ((heatPump.RunFrac >= 1.0) || (heatPump.Twet_Rated <= 0.0) || (heatPump.Gamma_Rated <= 0.0) || (fanOp == HVAC::FanOp::Cycling)) {
            LatDegradModelSimFlag = false;
            // Set NumIteration4=1 so that latent model would quit after 1 simulation with the actual condition
            NumIteration4 = 1;
        } else {
            LatDegradModelSimFlag = true;
            // Set NumIteration4=0 so that latent model would simulate twice with rated and actual condition
            NumIteration4 = 0;
        }

        // Tuned Hoisted quantities out of nested loop that don't change
        Real64 const LoadSideMassFlowRate_CpAir_inv(1.0 / (heatPump.InletAirMassFlowRate * CpAir));
        Real64 const LoadSideEffec(1.0 -
                                   std::exp(-heatPump.LoadSideOutsideUACoeff *
                                            LoadSideMassFlowRate_CpAir_inv)); // Load Side Effectiveness based on Outside Heat Transfer Coefficient
        Real64 const LoadSideEffec_MassFlowRate_inv(1.0 / (LoadSideEffec * heatPump.InletAirMassFlowRate));
        ANTUWET = heatPump.LoadSideTotalUACoeff * LoadSideMassFlowRate_CpAir_inv;
        EffectWET = 1.0 - std::exp(-ANTUWET);

        while (true) {
            ++NumIteration4;
            if (NumIteration4 == 1) {
                // Set indoor air conditions to the rated condition
                LoadSideInletDBTemp = state.dataWaterToAirHeatPump->LoadSideInletDBTemp_Init;
                LoadSideInletHumRat = state.dataWaterToAirHeatPump->LoadSideInletHumRat_Init;
                LoadSideAirInletEnth = state.dataWaterToAirHeatPump->LoadSideAirInletEnth_Init;
            } else {
                // Set indoor air conditions to the actual condition
                LoadSideInletDBTemp = heatPump.InletAirDBTemp;
                LoadSideInletHumRat = heatPump.InletAirHumRat;
                LoadSideAirInletEnth = LoadSideAirInletEnth_Unit;
            }

            // Outerloop: Calculate source side heat transfer
            int NumIteration2 = 0;
            Converged = false;
            StillSimulatingFlag = true;
            SourceResidual = 1.0;
            while (StillSimulatingFlag) {
                if (Converged) {
                    StillSimulatingFlag = false;
                }

                ++NumIteration2;
                if (NumIteration2 == 1) {
                    RelaxParam = 0.5;
                }

                if (NumIteration2 > STOP2) {
                    heatPump.SimFlag = false;
                    return;
                }

                // Innerloop: Calculate load side heat transfer
                NumIteration3 = 0;
                LoadResidual = 1.0;
                while (LoadResidual > ERR) {

                    ++NumIteration3;

                    if (NumIteration3 > STOP3) {
                        heatPump.SimFlag = false;
                        return;
                    }

                    // Determine Effectiveness of Source Side
                    CpFluid = heatPump.plantLoc.loop->glycol->getSpecificHeat(state, heatPump.InletWaterTemp, RoutineNameSourceSideInletTemp);

                    if (heatPump.plantLoc.loop->glycol->Num == Fluid::GlycolNum_Water) {
                        SourceSideEffect = 1.0 - std::exp(-heatPump.SourceSideUACoeff / (CpFluid * heatPump.InletWaterMassFlowRate));
                    } else {
                        DegradFactor = DegradF(state, heatPump.plantLoc.loop->glycol, heatPump.InletWaterTemp);
                        SourceSideEffect =
                            1.0 / ((heatPump.SourceSideHTR1 * std::pow(SourceSideVolFlowRate, -0.8)) / DegradFactor + heatPump.SourceSideHTR2);
                    }

                    // Determine Source Side Temperature (Condensing Temp in this case)
                    SourceSideTemp = heatPump.InletWaterTemp + state.dataWaterToAirHeatPump->initialQSource_calc /
                                                                   (SourceSideEffect * CpFluid * heatPump.InletWaterMassFlowRate);

                    // Compute the Effective Surface Temperature
                    EffectiveSatEnth = LoadSideAirInletEnth - state.dataWaterToAirHeatPump->initialQLoadTotal_calc * LoadSideEffec_MassFlowRate_inv;

                    EffectiveSurfaceTemp = Psychrometrics::PsyTsatFnHPb(state, EffectiveSatEnth, PB, RoutineNameLoadSideSurfaceTemp);

                    QSensible = heatPump.InletAirMassFlowRate * CpAir * (LoadSideInletDBTemp - EffectiveSurfaceTemp) * LoadSideEffec;
                    EvapSatEnth =
                        LoadSideAirInletEnth - state.dataWaterToAirHeatPump->initialQLoadTotal_calc / (EffectWET * heatPump.InletAirMassFlowRate);

                    EvapTemp = Psychrometrics::PsyTsatFnHPb(state, EvapSatEnth, PB, RoutineNameLoadSideEvapTemp);

                    // Load Side Saturated Temperature (Evaporating Temp in this case)
                    LoadSideTemp = EvapTemp;

                    // Determine the Load Side and Source Side Saturated Temp (evaporating and condensing pressures)
                    SourceSidePressure = heatPump.refrig->getSatPressure(state, SourceSideTemp, RoutineNameSourceSideTemp);
                    LoadSidePressure = heatPump.refrig->getSatPressure(state, LoadSideTemp, RoutineNameLoadSideTemp);

                    if (LoadSidePressure < heatPump.LowPressCutoff && !FirstHVACIteration) {
                        if (!state.dataGlobal->WarmupFlag) {
                            ShowRecurringWarningErrorAtEnd(state,
                                                           std::format("WaterToAir Heat pump:cooling [{}] shut off on low pressure < {:.0f}",
                                                                       heatPump.Name,
                                                                       heatPump.LowPressCutoff),
                                                           heatPump.LowPressClgError,
                                                           LoadSidePressure,
                                                           LoadSidePressure,
                                                           _,
                                                           "[Pa]",
                                                           "[Pa]");
                        }
                        heatPump.SimFlag = false;
                        return;
                    }

                    if (SourceSidePressure > heatPump.HighPressCutoff && !FirstHVACIteration) {
                        if (!state.dataGlobal->WarmupFlag) {
                            ShowRecurringWarningErrorAtEnd(state,
                                                           std::format("WaterToAir Heat pump:cooling [{}] shut off on high pressure > {:.0f}",
                                                                       heatPump.Name,
                                                                       heatPump.HighPressCutoff),
                                                           heatPump.HighPressClgError,
                                                           heatPump.InletWaterTemp,
                                                           heatPump.InletWaterTemp,
                                                           _,
                                                           "SourceSideInletTemp[C]",
                                                           "SourceSideInletTemp[C]");
                        }
                        heatPump.SimFlag = false;
                        return;
                    }

                    // Determine Suction Pressure & Discharge Pressure at Compressor Exit
                    if (heatPump.compressorType == CompressorType::Reciprocating) { // RECIPROCATING
                        SuctionPr = LoadSidePressure - heatPump.CompSucPressDrop;
                        DischargePr = SourceSidePressure + heatPump.CompSucPressDrop;
                    } else if (heatPump.compressorType == CompressorType::Rotary) { // ROTARY
                        SuctionPr = LoadSidePressure;
                        DischargePr = SourceSidePressure + heatPump.CompSucPressDrop;
                    } else if (heatPump.compressorType == CompressorType::Scroll) { // SCROLL
                        SuctionPr = LoadSidePressure;
                        DischargePr = SourceSidePressure;
                    }

                    // Determine the Load Side Outlet Enthalpy (Saturated Gas)
                    Quality = 1.0;
                    LoadSideOutletEnth = heatPump.refrig->getSatEnthalpy(state, LoadSideTemp, Quality, RoutineNameLoadSideTemp);

                    // Determine Source Side Outlet Enthalpy (Saturated Liquid)
                    Quality = 0.0;
                    SourceSideOutletEnth = heatPump.refrig->getSatEnthalpy(state, SourceSideTemp, Quality, RoutineNameSourceSideTemp);
                    // Determine Superheated Temperature of the Load Side outlet/compressor Inlet
                    CompressInletTemp = LoadSideTemp + heatPump.SuperheatTemp;

                    // Determine the Enthalpy of the Superheated Fluid at Load Side Outlet/Compressor Inlet
                    SuperHeatEnth = heatPump.refrig->getSupHeatEnthalpy(state, CompressInletTemp, LoadSidePressure, RoutineNameCompressInletTemp);

                    // Determining the suction state of the fluid from inlet state involves interation
                    // Method employed...
                    // Determine the saturated temp at suction pressure, shoot out into the superheated region find the enthalpy
                    // check that with the inlet enthalpy ( as suction loss is isenthalpic). Iterate till desired accuracy is reached
                    if (!Converged) {
                        CompSuctionSatTemp = heatPump.refrig->getSatTemperature(state, SuctionPr, RoutineNameSuctionPr);
                        CompSuctionTemp1 = CompSuctionSatTemp;

                        // Shoot into the Superheated Region
                        CompSuctionTemp2 = CompSuctionSatTemp + DegreeofSuperheat;
                    }

                    auto f = [&state, &heatPump, SuctionPr, SuperHeatEnth](Real64 const CompSuctionTemp) {
                        static constexpr std::string_view RoutineName("CalcWaterToAirHPHeating:CalcCompSuctionTemp");
                        Real64 compSuctionEnth = heatPump.refrig->getSupHeatEnthalpy(state, CompSuctionTemp, SuctionPr, RoutineName);
                        return (compSuctionEnth - SuperHeatEnth) / SuperHeatEnth;
                    };

                    state.dataWaterToAirHeatPump->CompSuctionTemp =
                        General::SolveRoot2(state, ERR, STOP1, SolFlag, f, CompSuctionTemp1, CompSuctionTemp2, heatPump.solveRootStats);
                    if (SolFlag == General::SOLVEROOT_ERROR_ITER) {
                        heatPump.SimFlag = false;
                        return;
                    }
                    CompSuctionEnth = heatPump.refrig->getSupHeatEnthalpy(
                        state, state.dataWaterToAirHeatPump->CompSuctionTemp, SuctionPr, RoutineNameCompSuctionTemp);
                    CompSuctionDensity = heatPump.refrig->getSupHeatDensity(
                        state, state.dataWaterToAirHeatPump->CompSuctionTemp, SuctionPr, RoutineNameCompSuctionTemp);

                    // Find Refrigerant Flow Rate
                    switch (heatPump.compressorType) {
                    case CompressorType::Reciprocating: {
                        MassRef =
                            heatPump.CompPistonDisp * CompSuctionDensity *
                            (1.0 + heatPump.CompClearanceFactor - heatPump.CompClearanceFactor * std::pow(DischargePr / SuctionPr, 1.0 / gamma));
                        break;
                    }
                    case CompressorType::Rotary: {
                        MassRef = heatPump.CompPistonDisp * CompSuctionDensity;
                        break;
                    }
                    case CompressorType::Scroll: {
                        MassRef = heatPump.RefVolFlowRate * CompSuctionDensity - heatPump.LeakRateCoeff * (DischargePr / SuctionPr);
                        break;
                    }
                    default:
                        break;
                    }
                    MassRef = max(0.0, MassRef);

                    // Find the Load Side Heat Transfer
                    QLoadTotal = MassRef * (LoadSideOutletEnth - SourceSideOutletEnth);
                    LoadResidual = std::abs(QLoadTotal - state.dataWaterToAirHeatPump->initialQLoadTotal_calc) /
                                   state.dataWaterToAirHeatPump->initialQLoadTotal_calc;
                    state.dataWaterToAirHeatPump->initialQLoadTotal_calc +=
                        RelaxParam * (QLoadTotal - state.dataWaterToAirHeatPump->initialQLoadTotal_calc);
                    if (NumIteration3 > 8) {
                        RelaxParam = 0.3;
                    }
                }

                // Determine the Power Consumption
                switch (heatPump.compressorType) {
                case CompressorType::Reciprocating:
                case CompressorType::Rotary: {
                    Power = heatPump.PowerLosses + (1.0 / heatPump.LossFactor) * (MassRef * gamma / (gamma - 1.0) * SuctionPr / CompSuctionDensity *
                                                                                  (std::pow(DischargePr / SuctionPr, (gamma - 1.0) / gamma) - 1.0));
                    break;
                }
                case CompressorType::Scroll: {
                    Power = heatPump.PowerLosses + (1.0 / heatPump.LossFactor) * (gamma / (gamma - 1.0)) * SuctionPr * heatPump.RefVolFlowRate *
                                                       (((gamma - 1.0) / gamma) * ((DischargePr / SuctionPr) / heatPump.VolumeRatio) +
                                                        ((1.0 / gamma) * std::pow(heatPump.VolumeRatio, gamma - 1.0)) - 1.0);
                    break;
                }
                default:
                    break;
                }

                // Determine the Sourceside Heat Rate
                QSource = Power + QLoadTotal;
                SourceResidual =
                    std::abs(QSource - state.dataWaterToAirHeatPump->initialQSource_calc) / state.dataWaterToAirHeatPump->initialQSource_calc;
                if (SourceResidual < ERR) {
                    Converged = true;
                }
                state.dataWaterToAirHeatPump->initialQSource_calc += RelaxParam * (QSource - state.dataWaterToAirHeatPump->initialQSource_calc);
                if (NumIteration2 > 8) {
                    RelaxParam = 0.2;
                }
            }

            if (SuctionPr < heatPump.LowPressCutoff) {
                ShowWarningError(state, "Heat pump:cooling shut down on low pressure");
                heatPump.SimFlag = false;
            }

            if (DischargePr > heatPump.HighPressCutoff && !FirstHVACIteration) {
                ShowWarningError(state, "Heat pump:cooling shut down on high pressure");
                heatPump.SimFlag = false;
            }

            if (QSensible > QLoadTotal) {
                QSensible = QLoadTotal;
            }

            if (LatDegradModelSimFlag) {
                if (NumIteration4 == 1) {
                    QLatRated = QLoadTotal - QSensible;

                } else if (NumIteration4 == 2) {
                    QLatActual = QLoadTotal - QSensible;
                    SHRss = QSensible / QLoadTotal;
                    LoadSideInletWBTemp = Psychrometrics::PsyTwbFnTdbWPb(state, LoadSideInletDBTemp, LoadSideInletHumRat, PB);
                    SHReff = CalcEffectiveSHR(
                        state, HPNum, SHRss, fanOp, heatPump.RunFrac, QLatRated, QLatActual, LoadSideInletDBTemp, LoadSideInletWBTemp);
                    //   Update sensible capacity based on effective SHR
                    QSensible = QLoadTotal * SHReff;
                    goto LOOPLatentDegradationModel_exit;
                }
            } else {

                SHReff = QSensible / QLoadTotal;
                goto LOOPLatentDegradationModel_exit;
            }
        }
    LOOPLatentDegradationModel_exit:;

        // calculate coil outlet state variables
        LoadSideAirOutletEnth = LoadSideAirInletEnth - QLoadTotal / heatPump.InletAirMassFlowRate;
        LoadSideOutletDBTemp = LoadSideInletDBTemp - QSensible * LoadSideMassFlowRate_CpAir_inv;
        LoadSideOutletHumRat = Psychrometrics::PsyWFnTdbH(state, LoadSideOutletDBTemp, LoadSideAirOutletEnth, RoutineNameLoadSideOutletEnthalpy);
        SourceSideOutletTemp = heatPump.InletWaterTemp + QSource / (heatPump.InletWaterMassFlowRate * CpWater);

        // Actual outlet conditions are "average" for time step
        if (fanOp == HVAC::FanOp::Continuous) {
            // continuous fan, cycling compressor
            heatPump.OutletAirEnthalpy = PartLoadRatio * LoadSideAirOutletEnth + (1.0 - PartLoadRatio) * LoadSideAirInletEnth;
            heatPump.OutletAirHumRat = PartLoadRatio * LoadSideOutletHumRat + (1.0 - PartLoadRatio) * LoadSideInletHumRat;
            heatPump.OutletAirDBTemp = Psychrometrics::PsyTdbFnHW(heatPump.OutletAirEnthalpy, heatPump.OutletAirHumRat);
        } else {
            // default to cycling fan, cycling compressor
            heatPump.OutletAirEnthalpy = LoadSideAirOutletEnth;
            heatPump.OutletAirHumRat = LoadSideOutletHumRat;
            heatPump.OutletAirDBTemp = LoadSideOutletDBTemp;
        }

        // scale heat transfer rates and power to run time
        QLoadTotal *= PartLoadRatio;
        QSensible *= PartLoadRatio;
        Power *= heatPump.RunFrac;
        QSource *= PartLoadRatio;

        // Update heat pump data structure
        state.dataHVACGlobal->DXElecCoolingPower = Power;
        heatPump.Power = Power;
        heatPump.QLoadTotal = QLoadTotal;
        heatPump.QSensible = QSensible;
        heatPump.QLatent = QLoadTotal - QSensible;
        heatPump.QSource = QSource;
        heatPump.PartLoadRatio = PartLoadRatio;

        //  Air-side outlet conditions are already calculated above
        heatPump.OutletAirMassFlowRate = heatPump.InletAirMassFlowRate;
        heatPump.OutletWaterTemp = SourceSideOutletTemp;
        heatPump.OutletWaterMassFlowRate = heatPump.InletWaterMassFlowRate;
        heatPump.OutletWaterEnthalpy = heatPump.InletWaterEnthalpy + QSource / heatPump.InletWaterMassFlowRate;
    }

    void CalcWatertoAirHPHeating(EnergyPlusData &state,
                                 int const HPNum,                      // heat pump number
                                 HVAC::FanOp const fanOp,              // fan/compressor cycling scheme indicator
                                 bool const FirstHVACIteration,        // first iteration flag
                                 [[maybe_unused]] bool const InitFlag, // first iteration flag
                                 Real64 const SensDemand,
                                 HVAC::CompressorOp const compressorOp,
                                 Real64 const PartLoadRatio)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Hui Jin
        //       DATE WRITTEN   Oct 2000
        //       MODIFIED       R. Raustad (Oct 2006) Revised iteration technique

        // PURPOSE OF THIS SUBROUTINE:
        // Simulates a parameter estimation based water to air heat pump model

        // Using/Aliasing
        auto &heatPump = state.dataWaterToAirHeatPump->WatertoAirHP(HPNum);

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr CpWater(4210.0);         // Specific heat of water J/kg_C
        Real64 constexpr DegreeofSuperheat(80.0); // Initial guess of degree of superheat
        Real64 constexpr gamma(1.114);            // Expnasion Coefficient
        Real64 RelaxParam(0.5);                   // Relaxation Parameter
        Real64 constexpr ERR(0.01);               // Error Value
        int constexpr STOP1(1000);                // Iteration stopper1
        int constexpr STOP2(1000);                // Iteration stopper2
        int constexpr STOP3(1000);                // Iteration stopper3

        static constexpr std::string_view RoutineNameSourceSideInletTemp("CalcWatertoAirHPHeating:SourceSideInletTemp");
        static constexpr std::string_view RoutineNameSourceSideTemp("CalcWatertoAirHPHeating:SourceSideTemp");
        static constexpr std::string_view RoutineNameLoadSideTemp("CalcWatertoAirHPHeating:LoadSideTemp");
        static constexpr std::string_view RoutineNameLoadSideOutletEnthalpy("CalcWatertoAirHPHeating:LoadSideOutletEnthalpy");
        static constexpr std::string_view RoutineNameCompressInletTemp("CalcWatertoAirHPHeating:CompressInletTemp");
        static constexpr std::string_view RoutineNameSuctionPr("CalcWatertoAirHPHeating:SuctionPr");
        static constexpr std::string_view RoutineNameCompSuctionTemp("CalcWatertoAirHPHeating:CompSuctionTemp");

        int NumIteration3; // Number of Iteration3
        Real64 Quality;
        Real64 SourceSideOutletTemp;  // Source Side Outlet Temperature [C]
        Real64 SourceSideVolFlowRate; // Source Side Volumetric Flow Rate [m3/s]
        Real64 CpFluid;               // Specific heat of source side fluid(J/kg)
        Real64 LoadSideOutletDBTemp;  // Load Side Outlet Dry Bulb Temperature [C]
        Real64 LoadSideOutletHumRat;  // Load Side Outlet Humidity Ratio [kg/kg]
        Real64 LoadSideAirOutletEnth; // Load Side Outlet Enthalpy [J/kg]
        Real64 CpAir;                 // Specific Heat of Air [J/kg_C]
        Real64 DegradFactor;          // Degradation Factor [~]
        Real64 QSource = 0.0;         // Source Side Heat Transfer Rate [W]
        Real64 QLoadTotal = 0.0;      // Load Side Heat Transfer Rate [W]
        Real64 Power = 0.0;           // Power Consumption [W]

        Real64 SourceSideEffect;       // Source Side Heat Exchanger Effectiveness
        Real64 SourceSideTemp;         // Source Side Saturated Refrigerant Temperature [C]
        Real64 LoadSideTemp;           // Load Side Saturated Refrigerant Temperature [C]
        Real64 SourceSidePressure;     // Source Side Saturated Refrigerant Pressure [Pa]
        Real64 LoadSidePressure;       // Load Side Saturated Refrigerant Pressure [Pa]
        Real64 SuctionPr = 0.0;        // Compressor Suction Pressure [Pa]
        Real64 DischargePr = 0.0;      // Compressor Discharge Pressure [Pa]
        Real64 CompressInletTemp;      // Temperature of the Refrigerant Entering the Compressor [C]
        Real64 MassRef = 0.0;          // Mass Flow Rate of Refrigerant [kg/s]
        Real64 SourceSideOutletEnth;   // Enthalpy of Refrigerant leaving the Source Side Heat Exchanger [J/kg]
        Real64 LoadSideOutletEnth;     // Enthalpy of Refrigerant leaving the Load Side Heat Exchanger [J/kg]
        Real64 SuperHeatEnth;          // Enthalpy of the Superheated Refrigerant [J/kg]
        Real64 CompSuctionTemp1 = 0.0; // Guess of the Temperature of the Refrigerant Entering the
        // Compressor #1 [C]
        Real64 CompSuctionTemp2 = 0.0; // Guess of the Temperature of the Refrigerant Entering the
        // Compressor #2 [C]
        Real64 CompSuctionTemp;          // Temperature of the Refrigerant Entering the Compressor [C]
        Real64 CompSuctionEnth;          // Enthalpy of the Refrigerant Entering the Compressor [J/kg]
        Real64 CompSuctionDensity = 0.0; // Density of the Refrigerant Entering the Compressorkg/m3
        Real64 CompSuctionSatTemp;       // Temperature of Saturated Refrigerant at Compressor Suction Pressure [C]
        bool StillSimulatingFlag;        // Final Simulation Flag
        bool Converged;                  // Overall convergence Flag
        int SolFlag;                     // Solution flag returned from RegulaFalsi function
        Real64 LoadResidual;             // loop convergence criteria
        Real64 SourceResidual;           // loop convergence criteria

        //  LOAD LOCAL VARIABLES FROM DATA STRUCTURE (for code readability)

        CpAir = Psychrometrics::PsyCpAirFnW(heatPump.InletAirHumRat);
        SourceSideVolFlowRate = heatPump.InletWaterMassFlowRate /
                                heatPump.plantLoc.loop->glycol->getDensity(state, heatPump.InletWaterTemp, RoutineNameSourceSideInletTemp);

        // If heat pump is not operating, return
        if (SensDemand == 0.0 || heatPump.InletAirMassFlowRate <= 0.0 || heatPump.InletWaterMassFlowRate <= 0.0 ||
            (heatPump.availSched->getCurrentVal() <= 0.0)) {
            heatPump.SimFlag = false;
            return;
        }
        heatPump.SimFlag = true;

        if (compressorOp == HVAC::CompressorOp::Off) {
            heatPump.SimFlag = false;
            return;
        }

        if (FirstHVACIteration) {
            state.dataWaterToAirHeatPump->initialQLoad = heatPump.HeatingCapacity;
            state.dataWaterToAirHeatPump->initialQSource = heatPump.HeatingCapacity;
        }

        if (state.dataWaterToAirHeatPump->initialQLoad == 0.0) {
            state.dataWaterToAirHeatPump->initialQLoad = heatPump.HeatingCapacity;
        }
        if (state.dataWaterToAirHeatPump->initialQSource == 0.0) {
            state.dataWaterToAirHeatPump->initialQSource = heatPump.HeatingCapacity;
        }

        // Tuned Hoisted quantities out of nested loop that don't change
        Real64 const LoadSideMassFlowRate_CpAir_inv(1.0 / (heatPump.InletAirMassFlowRate * CpAir));
        Real64 const LoadSideEffect(1.0 -
                                    std::exp(-heatPump.LoadSideTotalUACoeff *
                                             LoadSideMassFlowRate_CpAir_inv)); // Load Side Effectiveness based on Outside Heat Transfer Coefficient
        Real64 const LoadSideEffect_CpAir_MassFlowRate_inv(1.0 / (LoadSideEffect * CpAir * heatPump.InletAirMassFlowRate));

        // Outerloop: calculate load side heat transfer
        NumIteration3 = 0;
        Converged = false;
        StillSimulatingFlag = true;
        LoadResidual = 1.0;
        while (StillSimulatingFlag) {
            if (Converged) {
                StillSimulatingFlag = false;
            }

            ++NumIteration3;
            if (NumIteration3 == 1) {
                RelaxParam = 0.5;
            }

            if (NumIteration3 > STOP3) {
                heatPump.SimFlag = false;
                return;
            }

            // Innerloop: calculate load side heat transfer
            int NumIteration2 = 0;
            SourceResidual = 1.0;
            while (SourceResidual > ERR) {

                ++NumIteration2;

                if (NumIteration2 > STOP2) {
                    heatPump.SimFlag = false;
                    return;
                }

                // Determine Effectiveness of Source Side
                CpFluid = heatPump.plantLoc.loop->glycol->getSpecificHeat(state, heatPump.InletWaterTemp, RoutineNameSourceSideInletTemp);

                if (heatPump.plantLoc.loop->glycol->Num == Fluid::GlycolNum_Water) {
                    SourceSideEffect = 1.0 - std::exp(-heatPump.SourceSideUACoeff / (CpFluid * heatPump.InletWaterMassFlowRate));
                } else {
                    DegradFactor = DegradF(state, heatPump.plantLoc.loop->glycol, heatPump.InletWaterTemp);
                    SourceSideEffect =
                        1.0 / ((heatPump.SourceSideHTR1 * std::pow(SourceSideVolFlowRate, -0.8)) / DegradFactor + heatPump.SourceSideHTR2);
                }

                // Determine Source Side Temperature (Evap. Temp for this mode)
                SourceSideTemp = heatPump.InletWaterTemp -
                                 state.dataWaterToAirHeatPump->initialQSource / (SourceSideEffect * CpFluid * heatPump.InletWaterMassFlowRate);

                // Determine Load Side Temperature (Condensing Temp for this mode)
                LoadSideTemp = heatPump.InletAirDBTemp + state.dataWaterToAirHeatPump->initialQLoad * LoadSideEffect_CpAir_MassFlowRate_inv;

                // Determine the Load Side and Source Side Saturated Temp (evaporating and condensing pressures)
                SourceSidePressure = heatPump.refrig->getSatPressure(state, SourceSideTemp, RoutineNameSourceSideTemp);
                LoadSidePressure = heatPump.refrig->getSatPressure(state, LoadSideTemp, RoutineNameLoadSideTemp);
                if (SourceSidePressure < heatPump.LowPressCutoff && !FirstHVACIteration) {
                    if (!state.dataGlobal->WarmupFlag) {
                        ShowRecurringWarningErrorAtEnd(state,
                                                       std::format("WaterToAir Heat pump:heating [{}] shut off on low pressure < {:.0f}",
                                                                   heatPump.Name,
                                                                   heatPump.LowPressCutoff),
                                                       heatPump.LowPressHtgError,
                                                       SourceSidePressure,
                                                       SourceSidePressure,
                                                       _,
                                                       "[Pa]",
                                                       "[Pa]");
                    }
                    heatPump.SimFlag = false;
                    return;
                }

                if (LoadSidePressure > heatPump.HighPressCutoff && !FirstHVACIteration) {
                    if (!state.dataGlobal->WarmupFlag) {
                        ShowRecurringWarningErrorAtEnd(state,
                                                       std::format("WaterToAir Heat pump:heating [{}] shut off on high pressure > {:.0f}",
                                                                   heatPump.Name,
                                                                   heatPump.HighPressCutoff),
                                                       heatPump.HighPressHtgError,
                                                       heatPump.InletWaterTemp,
                                                       heatPump.InletWaterTemp,
                                                       _,
                                                       "SourceSideInletTemp[C]",
                                                       "SourceSideInletTemp[C]");
                    }
                    //         CALL ShowWarningError(state, 'Heat pump:heating shut off on high pressure')
                    //         WRITE(CErrCount,*) SourceSideInletTemp
                    //         CErrCount=ADJUSTL(CErrCount)
                    //         CALL ShowContinueError(state, 'Source side inlet temperature too low, T='//TRIM(CErrCount))
                    //         CALL ShowContinueError(state, 'Heat pump heating demand not met by plant side')
                    heatPump.SimFlag = false;
                    return;
                }

                // Determine Suction Pressure at Compressor Entrance & Discharge Pressure at Compressor Exit
                switch (heatPump.compressorType) {
                case CompressorType::Reciprocating: {
                    SuctionPr = SourceSidePressure - heatPump.CompSucPressDrop;
                    DischargePr = LoadSidePressure + heatPump.CompSucPressDrop;
                    break;
                }
                case CompressorType::Rotary: {
                    SuctionPr = SourceSidePressure;
                    DischargePr = LoadSidePressure + heatPump.CompSucPressDrop;
                    break;
                }
                case CompressorType::Scroll: {
                    SuctionPr = SourceSidePressure;
                    DischargePr = LoadSidePressure;
                    break;
                }
                default:
                    break;
                }

                // Determine the Source Side Outlet Enthalpy
                // Quality of the refrigerant leaving the evaporator is saturated gas
                Quality = 1.0;
                SourceSideOutletEnth = heatPump.refrig->getSatEnthalpy(state, SourceSideTemp, Quality, RoutineNameSourceSideTemp);

                // Determine Load Side Outlet Enthalpy
                // Quality of the refrigerant leaving the condenser is saturated liguid
                Quality = 0.0;
                LoadSideOutletEnth = heatPump.refrig->getSatEnthalpy(state, LoadSideTemp, Quality, RoutineNameLoadSideTemp);

                // Determine Superheated Temperature of the Source Side outlet/compressor Inlet
                CompressInletTemp = SourceSideTemp + heatPump.SuperheatTemp;

                // Determine the Enathalpy of the Superheated Fluid at Source Side Outlet/Compressor Inlet
                SuperHeatEnth = heatPump.refrig->getSupHeatEnthalpy(state, CompressInletTemp, SourceSidePressure, RoutineNameCompressInletTemp);

                // Determining the suction state of the fluid from inlet state involves interation
                // Method employed...
                // Determine the saturated temp at suction pressure, shoot out into the superheated region find the enthalpy
                // check that with the inlet enthalpy ( as suction loss is isenthalpic). Iterate till desired accuracy is reached

                if (!Converged) {
                    CompSuctionSatTemp = heatPump.refrig->getSatTemperature(state, SuctionPr, RoutineNameSuctionPr);
                    CompSuctionTemp1 = CompSuctionSatTemp;

                    // Shoot into the Superheated Region
                    CompSuctionTemp2 = CompSuctionSatTemp + DegreeofSuperheat;
                }

                auto f = [&state, &heatPump, SuctionPr, SuperHeatEnth](Real64 const CompSuctionTemp) {
                    static constexpr std::string_view RoutineName("CalcWaterToAirHPHeating:CalcCompSuctionTemp");
                    Real64 compSuctionEnth = heatPump.refrig->getSupHeatEnthalpy(state, CompSuctionTemp, SuctionPr, RoutineName);
                    return (compSuctionEnth - SuperHeatEnth) / SuperHeatEnth;
                };

                CompSuctionTemp = General::SolveRoot2(state, ERR, STOP1, SolFlag, f, CompSuctionTemp1, CompSuctionTemp2, heatPump.solveRootStats);
                if (SolFlag == General::SOLVEROOT_ERROR_ITER) {
                    heatPump.SimFlag = false;
                    return;
                }
                CompSuctionEnth = heatPump.refrig->getSupHeatEnthalpy(state, CompSuctionTemp, SuctionPr, RoutineNameCompSuctionTemp);
                CompSuctionDensity = heatPump.refrig->getSupHeatDensity(state, CompSuctionTemp, SuctionPr, RoutineNameCompSuctionTemp);

                // Find Refrigerant Flow Rate
                switch (heatPump.compressorType) {
                case CompressorType::Reciprocating: {
                    MassRef = heatPump.CompPistonDisp * CompSuctionDensity *
                              (1 + heatPump.CompClearanceFactor - heatPump.CompClearanceFactor * std::pow(DischargePr / SuctionPr, 1 / gamma));
                    break;
                }
                case CompressorType::Rotary: {
                    MassRef = heatPump.CompPistonDisp * CompSuctionDensity;
                    break;
                }
                case CompressorType::Scroll: {
                    MassRef = heatPump.RefVolFlowRate * CompSuctionDensity - heatPump.LeakRateCoeff * (DischargePr / SuctionPr);
                    break;
                }
                default:
                    break;
                }
                MassRef = max(0.0, MassRef);

                // Find the Source Side Heat Transfer
                QSource = MassRef * (SourceSideOutletEnth - LoadSideOutletEnth);
                SourceResidual = std::abs(QSource - state.dataWaterToAirHeatPump->initialQSource) / state.dataWaterToAirHeatPump->initialQSource;
                state.dataWaterToAirHeatPump->initialQSource += RelaxParam * (QSource - state.dataWaterToAirHeatPump->initialQSource);
                if (NumIteration2 > 8) {
                    RelaxParam = 0.3;
                }
            }

            // Determine the Power Consumption
            switch (heatPump.compressorType) {
            case CompressorType::Reciprocating:
            case CompressorType::Rotary: {
                Power = heatPump.PowerLosses + (1 / heatPump.LossFactor) * (MassRef * gamma / (gamma - 1) * SuctionPr / CompSuctionDensity *
                                                                            (std::pow(DischargePr / SuctionPr, (gamma - 1) / gamma) - 1));
                break;
            }
            case CompressorType::Scroll: {
                Power = heatPump.PowerLosses + (1 / heatPump.LossFactor) * (gamma / (gamma - 1)) * SuctionPr * heatPump.RefVolFlowRate *
                                                   (((gamma - 1) / gamma) * ((DischargePr / SuctionPr) / heatPump.VolumeRatio) +
                                                    ((1 / gamma) * std::pow(heatPump.VolumeRatio, gamma - 1)) - 1);
                break;
            }
            default:
                break;
            }

            // Determine the Load Side Heat Rate
            QLoadTotal = Power + QSource;
            LoadResidual = std::abs(QLoadTotal - state.dataWaterToAirHeatPump->initialQLoad) / state.dataWaterToAirHeatPump->initialQLoad;
            if (LoadResidual < ERR) {
                Converged = true;
            }
            state.dataWaterToAirHeatPump->initialQLoad += RelaxParam * (QLoadTotal - state.dataWaterToAirHeatPump->initialQLoad);
            if (NumIteration3 > 8) {
                RelaxParam = 0.2;
            }
        }

        if (SuctionPr < heatPump.LowPressCutoff && !FirstHVACIteration) {
            ShowWarningError(state, "Heat pump:heating shut down on low pressure");
            heatPump.SimFlag = false;
            return;
        }

        if (DischargePr > heatPump.HighPressCutoff && !FirstHVACIteration) {
            ShowWarningError(state, "Heat pump:heating shut down on high pressure");
            heatPump.SimFlag = false;
            return;
        }

        // calculate coil outlet state variables
        LoadSideAirOutletEnth = heatPump.InletAirEnthalpy + QLoadTotal / heatPump.InletAirMassFlowRate;
        LoadSideOutletDBTemp = heatPump.InletAirDBTemp + QLoadTotal / (heatPump.InletAirMassFlowRate * CpAir);
        LoadSideOutletHumRat = Psychrometrics::PsyWFnTdbH(state, LoadSideOutletDBTemp, LoadSideAirOutletEnth, RoutineNameLoadSideOutletEnthalpy);
        SourceSideOutletTemp = heatPump.InletWaterTemp - QSource / (heatPump.InletWaterMassFlowRate * CpWater);

        // Calculate actual outlet conditions for the run time fraction
        // Actual outlet conditions are "average" for time step
        if (fanOp == HVAC::FanOp::Continuous) {
            // continuous fan, cycling compressor
            heatPump.OutletAirEnthalpy = PartLoadRatio * LoadSideAirOutletEnth + (1.0 - PartLoadRatio) * heatPump.InletAirEnthalpy;
            heatPump.OutletAirHumRat = PartLoadRatio * LoadSideOutletHumRat + (1.0 - PartLoadRatio) * heatPump.InletAirHumRat;
            heatPump.OutletAirDBTemp = Psychrometrics::PsyTdbFnHW(heatPump.OutletAirEnthalpy, heatPump.OutletAirHumRat);
        } else {
            // default to cycling fan, cycling compressor
            heatPump.OutletAirEnthalpy = LoadSideAirOutletEnth;
            heatPump.OutletAirHumRat = LoadSideOutletHumRat;
            heatPump.OutletAirDBTemp = LoadSideOutletDBTemp;
        }

        // Calculate Part Load Factor and Runtime Fraction
        Real64 PLF = 1.0; // part load factor as a function of PLR, RTF = PLR / PLF
        if (heatPump.PLFCurveIndex > 0) {
            PLF = Curve::CurveValue(state, heatPump.PLFCurveIndex, PartLoadRatio); // Calculate part-load factor
        }
        if (fanOp == HVAC::FanOp::Cycling) {
            state.dataHVACGlobal->OnOffFanPartLoadFraction = PLF;
        }
        heatPump.RunFrac = PartLoadRatio / PLF;

        // scale heat transfer rates and power to run time
        QLoadTotal *= PartLoadRatio;
        Power *= heatPump.RunFrac;
        QSource *= PartLoadRatio;

        // Update heat pump data structure
        state.dataHVACGlobal->DXElecHeatingPower = Power;
        heatPump.Power = Power;
        heatPump.QLoadTotal = QLoadTotal;
        heatPump.QSensible = QLoadTotal;

        heatPump.QSource = QSource;
        heatPump.PartLoadRatio = PartLoadRatio;
        heatPump.OutletAirMassFlowRate = heatPump.InletAirMassFlowRate;
        heatPump.OutletWaterTemp = SourceSideOutletTemp;
        heatPump.OutletWaterMassFlowRate = heatPump.InletWaterMassFlowRate;
        heatPump.OutletWaterEnthalpy = heatPump.InletWaterEnthalpy - QSource / heatPump.InletWaterMassFlowRate;
    }

    void UpdateWatertoAirHP(EnergyPlusData &state, int const HPNum)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Hui Jin
        //       DATE WRITTEN   Oct 2000

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine updates the Water to Air Heat Pump outlet nodes.

        // METHODOLOGY EMPLOYED:
        // Data is moved from the HP data structure to the HP outlet nodes.

        // Using/Aliasing
        Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;
        auto &heatPump = state.dataWaterToAirHeatPump->WatertoAirHP(HPNum);

        // WatertoAirHP(HPNum)%SimFlag=.FALSE.
        if (!heatPump.SimFlag) {
            // Heatpump is off; just pass through conditions
            heatPump.Power = 0.0;
            heatPump.Energy = 0.0;
            heatPump.QLoadTotal = 0.0;
            heatPump.QSensible = 0.0;
            heatPump.QLatent = 0.0;
            heatPump.QSource = 0.0;
            heatPump.RunFrac = 0.0;
            heatPump.PartLoadRatio = 0.0;
            heatPump.OutletAirDBTemp = heatPump.InletAirDBTemp;
            heatPump.OutletAirHumRat = heatPump.InletAirHumRat;
            heatPump.OutletWaterTemp = heatPump.InletWaterTemp;
            heatPump.OutletAirMassFlowRate = heatPump.InletAirMassFlowRate;
            heatPump.OutletWaterMassFlowRate = heatPump.InletWaterMassFlowRate;
            heatPump.OutletAirEnthalpy = heatPump.InletAirEnthalpy;
            heatPump.OutletWaterEnthalpy = heatPump.InletWaterEnthalpy;
        }

        // Set the outlet air nodes of the WatertoAirHP
        state.dataLoopNodes->Node(heatPump.AirOutletNodeNum).MassFlowRate = state.dataLoopNodes->Node(heatPump.AirInletNodeNum).MassFlowRate;
        state.dataLoopNodes->Node(heatPump.AirOutletNodeNum).Temp = heatPump.OutletAirDBTemp;
        state.dataLoopNodes->Node(heatPump.AirOutletNodeNum).HumRat = heatPump.OutletAirHumRat;
        state.dataLoopNodes->Node(heatPump.AirOutletNodeNum).Enthalpy = heatPump.OutletAirEnthalpy;

        // Set the outlet nodes for properties that just pass through & not used
        PlantUtilities::SafeCopyPlantNode(state, heatPump.WaterInletNodeNum, heatPump.WaterOutletNodeNum);
        // Set the outlet water nodes for the heat pump
        state.dataLoopNodes->Node(heatPump.WaterOutletNodeNum).Temp = heatPump.OutletWaterTemp;
        state.dataLoopNodes->Node(heatPump.WaterOutletNodeNum).Enthalpy = heatPump.OutletWaterEnthalpy;

        // Set the outlet nodes for properties that just pass through & not used
        state.dataLoopNodes->Node(heatPump.AirOutletNodeNum).Quality = state.dataLoopNodes->Node(heatPump.AirInletNodeNum).Quality;
        state.dataLoopNodes->Node(heatPump.AirOutletNodeNum).Press = state.dataLoopNodes->Node(heatPump.AirInletNodeNum).Press;
        state.dataLoopNodes->Node(heatPump.AirOutletNodeNum).MassFlowRateMin = state.dataLoopNodes->Node(heatPump.AirInletNodeNum).MassFlowRateMin;
        state.dataLoopNodes->Node(heatPump.AirOutletNodeNum).MassFlowRateMax = state.dataLoopNodes->Node(heatPump.AirInletNodeNum).MassFlowRateMax;
        state.dataLoopNodes->Node(heatPump.AirOutletNodeNum).MassFlowRateMinAvail =
            state.dataLoopNodes->Node(heatPump.AirInletNodeNum).MassFlowRateMinAvail;
        state.dataLoopNodes->Node(heatPump.AirOutletNodeNum).MassFlowRateMaxAvail =
            state.dataLoopNodes->Node(heatPump.AirInletNodeNum).MassFlowRateMaxAvail;

        // Pass through the load side mass flow rates
        heatPump.InletAirMassFlowRate = state.dataLoopNodes->Node(heatPump.AirInletNodeNum).MassFlowRate;
        heatPump.OutletAirMassFlowRate = heatPump.InletAirMassFlowRate;

        heatPump.Energy = heatPump.Power * TimeStepSysSec;
        heatPump.EnergyLoadTotal = heatPump.QLoadTotal * TimeStepSysSec;
        heatPump.EnergySensible = heatPump.QSensible * TimeStepSysSec;
        heatPump.EnergyLatent = heatPump.QLatent * TimeStepSysSec;
        heatPump.EnergySource = heatPump.QSource * TimeStepSysSec;

        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            state.dataLoopNodes->Node(heatPump.AirOutletNodeNum).CO2 = state.dataLoopNodes->Node(heatPump.AirInletNodeNum).CO2;
        }
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            state.dataLoopNodes->Node(heatPump.AirOutletNodeNum).GenContam = state.dataLoopNodes->Node(heatPump.AirInletNodeNum).GenContam;
        }
    }

    //        End of Update subroutines for the WatertoAirHP Module
    // *****************************************************************************

    Real64 CalcEffectiveSHR(EnergyPlusData &state,
                            int const HPNum,         // Index number for cooling coil
                            Real64 const SHRss,      // Steady-state sensible heat ratio
                            HVAC::FanOp const fanOp, // fan/compressor cycling scheme indicator
                            Real64 const RTF,        // Compressor run-time fraction
                            Real64 const QLatRated,  // Rated latent capacity
                            Real64 const QLatActual, // Actual latent capacity
                            Real64 const EnteringDB, // Entering air dry-bulb temperature
                            Real64 const EnteringWB  // Entering air wet-bulb temperature
    )
    {

        // FUNCTION INFORMATION:
        //    AUTHOR         Richard Raustad, FSEC
        //    DATE WRITTEN   September 2003
        //    MODIFIED       Kenneth Tang (Aug 2004) Added capability for simulating FanOp::Cycling

        // PURPOSE OF THIS FUNCTION:
        //    Adjust sensible heat ratio to account for degradation of DX coil latent
        //    capacity at part-load (cycling) conditions.

        // METHODOLOGY EMPLOYED:
        //    With model parameters entered by the user, the part-load latent performance
        //    of a DX cooling coil is determined for a constant air flow system with
        //    a cooling coil that cycles on/off. The model calculates the time
        //    required for condensate to begin falling from the cooling coil.
        //    Runtimes greater than this are integrated to a "part-load" latent
        //    capacity which is used to determine the "part-load" sensible heat ratio.
        //    See reference below for additional details (linear decay model, Eq. 8b).
        // REFERENCES:
        //   "A Model to Predict the Latent Capacity of Air Conditioners and
        //    Heat Pumps at Part-Load Conditions with Constant Fan Operation"
        //    1996 ASHRAE Transactions, Volume 102, Part 1, Pp. 266 - 274,
        //    Hugh I. Henderson, Jr., P.E., Kannan Rengarajan, P.E.

        // Using/Aliasing
        auto const &heatPump = state.dataWaterToAirHeatPump->WatertoAirHP(HPNum);

        // Return value
        Real64 SHReff; // Effective sensible heat ratio, includes degradation due to cycling effects

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        Real64 Twet; // Nominal time for condensate to begin leaving the coil's condensate drain line
        //   at the current operating conditions (sec)
        Real64 Gamma; // Initial moisture evaporation rate divided by steady-state AC latent capacity
        //   at the current operating conditions
        Real64 Twet_max; // Maximum allowed value for Twet
        // shut off after compressor cycle off  [s]

        Real64 Ton;       // Coil on time (sec)
        Real64 Toff;      // Coil off time (sec)
        Real64 Toffa;     // Actual coil off time (sec). Equations valid for Toff <= (2.0 * Twet/Gamma)
        Real64 aa;        // Intermediate variable
        Real64 To1;       // Intermediate variable (first guess at To). To = time to the start of moisture removal
        Real64 To2 = 0.0; // Intermediate variable (second guess at To). To = time to the start of moisture removal
        Real64 Error;     // Error for iteration (DO) loop
        Real64 LHRmult;   // Latent Heat Ratio (LHR) multiplier. The effective latent heat ratio LHR = (1-SHRss)*LHRmult

        //  No moisture evaporation (latent degradation) occurs for runtime fraction of 1.0
        //  All latent degradation model parameters cause divide by 0.0 if not greater than 0.0
        //  Latent degradation model parameters initialize to 0.0 meaning no evaporation model used.
        if ((RTF >= 1.0) || (QLatRated == 0.0) || (QLatActual == 0.0) || (heatPump.Twet_Rated <= 0.0) || (heatPump.Gamma_Rated <= 0.0) ||
            (heatPump.MaxONOFFCyclesperHour <= 0.0) || (heatPump.LatentCapacityTimeConstant <= 0.0) || (RTF <= 0.0)) {
            SHReff = SHRss;
            return SHReff;
        }

        Twet_max = 9999.0; // high limit for Twet

        //  Calculate the model parameters at the actual operating conditions
        Twet = min(heatPump.Twet_Rated * QLatRated / (QLatActual + 1.e-10), Twet_max);
        Gamma = heatPump.Gamma_Rated * QLatRated * (EnteringDB - EnteringWB) / ((26.7 - 19.4) * QLatActual + 1.e-10);

        //  Calculate the compressor on and off times using a conventional thermostat curve
        Ton = 3600.0 / (4.0 * heatPump.MaxONOFFCyclesperHour * (1.0 - RTF)); // duration of cooling coil on-cycle (sec)

        if ((fanOp == HVAC::FanOp::Cycling) && (heatPump.FanDelayTime != 0.0)) {
            //  For FanOp::Cycling, moisture is evaporated from the cooling coil back to the air stream
            //  until the fan cycle off. Assume no evaporation from the coil after the fan shuts off.
            Toff = heatPump.FanDelayTime;
        } else {
            //  For FanOp::Continuous, moisture is evaporated from the cooling coil back to the air stream
            //  for the entire heat pump off-cycle.
            Toff = 3600.0 / (4.0 * heatPump.MaxONOFFCyclesperHour * RTF); // duration of cooling coil off-cycle (sec)
        }

        //  Cap Toff to meet the equation restriction
        if (Gamma > 0.0) {
            Toffa = min(Toff, 2.0 * Twet / Gamma);
        } else {
            Toffa = Toff;
        }

        //  Use successive substitution to solve for To
        aa = (Gamma * Toffa) - (0.25 / Twet) * pow_2(Gamma) * pow_2(Toffa);
        To1 = aa + heatPump.LatentCapacityTimeConstant;
        Error = 1.0;
        while (Error > 0.001) {
            //  Floating overflow errors occur when -To1/LatentCapacityTimeConstant is a large positive number.
            //  Cap upper limit at 700 to avoid the overflow errors.
            To2 = aa - heatPump.LatentCapacityTimeConstant * std::expm1(min(700.0, -To1 / heatPump.LatentCapacityTimeConstant));
            Error = std::abs((To2 - To1) / To1);
            To1 = To2;
        }

        //  Adjust Sensible Heat Ratio (SHR) using Latent Heat Ratio (LHR) multiplier
        //  Floating underflow errors occur when -Ton/LatentCapacityTimeConstant is a large negative number.
        //  Cap lower limit at -700 to avoid the underflow errors.
        aa = std::exp(max(-700.0, -Ton / heatPump.LatentCapacityTimeConstant));
        //  Calculate latent heat ratio multiplier
        LHRmult = max(((Ton - To2) / (Ton + heatPump.LatentCapacityTimeConstant * (aa - 1.0))), 0.0);

        //  Calculate part-load or "effective" sensible heat ratio
        SHReff = 1.0 - (1.0 - SHRss) * LHRmult;

        if (SHReff < SHRss) {
            SHReff = SHRss; // Effective SHR can be less than the steady-state SHR
        }
        if (SHReff > 1.0) {
            SHReff = 1.0; // Effective sensible heat ratio can't be greater than 1.0
        }

        return SHReff;
    }

    Real64 DegradF(EnergyPlusData &state,
                   Fluid::GlycolProps *glycol,
                   Real64 &Temp // Temperature of the fluid
    )
    {
        // FUNCTION INFORMATION:
        //    AUTHOR         Kenneth Tang
        //    DATE WRITTEN   October 2004

        // PURPOSE OF THIS FUNCTION:
        //    Calculate the degradation factor to predict the heat pump performance
        //    when antifreeze is used.
        // METHODOLOGY EMPLOYED:
        //    Use FluidProperties to calculate the properties of water and glycol
        //    at the given temperature. Then substitute the properties into the equation.
        // REFERENCES:
        //    Jin, H. 2002. Parameter Estimation Based Models of Water Source Heat Pumps. Phd Thesis.
        //    Oklahoma State University.

        // Return value
        Real64 DegradF;

        // FUNCTION PARAMETER DEFINITIONS:
        static constexpr std::string_view CalledFrom("HVACWaterToAir:DegradF");

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        Real64 VisWater;       // Viscosity of water [mPa-s]
        Real64 DensityWater;   // Density of water [kg/m3]
        Real64 CpWater;        // Specific heat of water [J/kg-K]
        Real64 CondWater;      // Conductivity of water [W/m-K]
        Real64 VisCoolant;     // Viscosity of water [mPa-s]
        Real64 DensityCoolant; // Density of water [kg/m3]
        Real64 CpCoolant;      // Specific heat of water [J/kg-K]
        Real64 CondCoolant;    // Conductivity of water [W/m-K]

        auto *water = Fluid::GetWater(state);

        VisWater = water->getViscosity(state, Temp, CalledFrom);
        DensityWater = water->getDensity(state, Temp, CalledFrom);
        CpWater = water->getSpecificHeat(state, Temp, CalledFrom);
        CondWater = water->getConductivity(state, Temp, CalledFrom);
        VisCoolant = glycol->getViscosity(state, Temp, CalledFrom);
        DensityCoolant = glycol->getDensity(state, Temp, CalledFrom);
        CpCoolant = glycol->getSpecificHeat(state, Temp, CalledFrom);
        CondCoolant = glycol->getConductivity(state, Temp, CalledFrom);

        DegradF = std::pow(VisCoolant / VisWater, -0.47) * std::pow(DensityCoolant / DensityWater, 0.8) * std::pow(CpCoolant / CpWater, 0.33) *
                  std::pow(CondCoolant / CondWater, 0.67);

        return DegradF;
    }

    int GetCoilIndex(EnergyPlusData &state,
                     std::string const &CoilType, // must match coil types in this module
                     std::string const &CoilName, // must match coil names for the coil type
                     bool &ErrorsFound            // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         R. Raustad
        //       DATE WRITTEN   August 2007

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given coil and returns the index.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and value is returned
        // as zero.

        // Obtains and Allocates WatertoAirHP related parameters from input file
        if (state.dataWaterToAirHeatPump->GetCoilsInputFlag) { // First time subroutine has been entered
            GetWatertoAirHPInput(state);
            state.dataWaterToAirHeatPump->GetCoilsInputFlag = false;
        }

        int IndexNum = Util::FindItemInList(CoilName, state.dataWaterToAirHeatPump->WatertoAirHP);

        if (IndexNum == 0) {
            ShowSevereError(state, std::format("Could not find CoilType=\"{}\" with Name=\"{}\"", CoilType, CoilName));
            ErrorsFound = true;
        }

        return IndexNum;
    }

    Real64 GetCoilCapacity(EnergyPlusData &state,
                           std::string_view const CoilType, // must match coil types in this module
                           std::string const &CoilName,     // must match coil names for the coil type
                           bool &ErrorsFound                // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   February 2006

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the coil capacity for the given coil and returns it.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and capacity is returned
        // as negative.

        // Return value
        Real64 CoilCapacity = 0.0; // returned capacity of matched coil

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        int WhichCoil;

        // Obtains and Allocates WatertoAirHP related parameters from input file
        if (state.dataWaterToAirHeatPump->GetCoilsInputFlag) { // First time subroutine has been entered
            GetWatertoAirHPInput(state);
            state.dataWaterToAirHeatPump->GetCoilsInputFlag = false;
        }

        if (Util::SameString(CoilType, "COIL:HEATING:WATERTOAIRHEATPUMP:PARAMETERESTIMATION") ||
            Util::SameString(CoilType, "COIL:COOLING:WATERTOAIRHEATPUMP:PARAMETERESTIMATION")) {
            WhichCoil = Util::FindItemInList(CoilName, state.dataWaterToAirHeatPump->WatertoAirHP);
            if (WhichCoil != 0) {
                if (Util::SameString(CoilType, "COIL:HEATING:WATERTOAIRHEATPUMP:PARAMETERESTIMATION")) {
                    CoilCapacity = state.dataWaterToAirHeatPump->WatertoAirHP(WhichCoil).HeatingCapacity;
                } else {
                    CoilCapacity = state.dataWaterToAirHeatPump->WatertoAirHP(WhichCoil).CoolingCapacity;
                }
            }
        } else {
            WhichCoil = 0;
        }

        if (WhichCoil == 0) {
            ShowSevereError(state, std::format("Could not find CoilType=\"{}\" with Name=\"{}\"", CoilType, CoilName));
            ErrorsFound = true;
            CoilCapacity = -1000.0;
        }

        return CoilCapacity;
    }

    int GetCoilInletNode(EnergyPlusData &state,
                         std::string const &CoilType, // must match coil types in this module
                         std::string const &CoilName, // must match coil names for the coil type
                         bool &ErrorsFound            // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   February 2006

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given coil and returns the inlet node.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and value is returned
        // as zero.

        // Return value
        int NodeNumber = 0; // returned outlet node of matched coil

        // Obtains and Allocates WatertoAirHP related parameters from input file
        if (state.dataWaterToAirHeatPump->GetCoilsInputFlag) { // First time subroutine has been entered
            GetWatertoAirHPInput(state);
            state.dataWaterToAirHeatPump->GetCoilsInputFlag = false;
        }

        int WhichCoil = Util::FindItemInList(CoilName, state.dataWaterToAirHeatPump->WatertoAirHP);
        if (WhichCoil != 0) {
            NodeNumber = state.dataWaterToAirHeatPump->WatertoAirHP(WhichCoil).AirInletNodeNum;
        }

        if (WhichCoil == 0) {
            ShowSevereError(state, std::format("Could not find CoilType=\"{}\" with Name=\"{}\"", CoilType, CoilName));
            ErrorsFound = true;
            NodeNumber = 0;
        }

        return NodeNumber;
    }

    int GetCoilOutletNode(EnergyPlusData &state,
                          std::string const &CoilType, // must match coil types in this module
                          std::string const &CoilName, // must match coil names for the coil type
                          bool &ErrorsFound            // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         R. Raustad
        //       DATE WRITTEN   July 2007

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given coil and returns the outlet node.  If
        // incorrect coil type or name is given, ErrorsFound is returned as true and value is returned
        // as zero.

        // Return value
        int NodeNumber = 0; // returned outlet node of matched coil

        // Obtains and Allocates WatertoAirHP related parameters from input file
        if (state.dataWaterToAirHeatPump->GetCoilsInputFlag) { // First time subroutine has been entered
            GetWatertoAirHPInput(state);
            state.dataWaterToAirHeatPump->GetCoilsInputFlag = false;
        }

        int WhichCoil = Util::FindItemInList(CoilName, state.dataWaterToAirHeatPump->WatertoAirHP);
        if (WhichCoil != 0) {
            NodeNumber = state.dataWaterToAirHeatPump->WatertoAirHP(WhichCoil).AirOutletNodeNum;
        }

        if (WhichCoil == 0) {
            ShowSevereError(state, std::format("Could not find CoilType=\"{}\" with Name=\"{}\"", CoilType, CoilName));
            ErrorsFound = true;
            NodeNumber = 0;
        }

        return NodeNumber;
    }

} // namespace WaterToAirHeatPump

} // namespace EnergyPlus
