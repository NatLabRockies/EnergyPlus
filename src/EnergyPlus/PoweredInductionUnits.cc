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
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataPrecisionGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HeatingCoils.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/MixerComponent.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/PoweredInductionUnits.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SteamCoils.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterCoils.hh>
#include <EnergyPlus/ZoneAirLoopEquipmentManager.hh>

namespace EnergyPlus::PoweredInductionUnits {

// Module containing routines dealing with Series and Parallel fan powered terminal boxes

// MODULE INFORMATION:
//       AUTHOR         Fred Buhl
//       DATE WRITTEN   August 2000
//       MODIFIED       Brent Griffith, Sept 2010, plant upgrades, fluid properties
//       RE-ENGINEERED  na

// PURPOSE OF THIS MODULE:
// To encapsulate the data and algorithms needed to simulate Series and Parallel
// fan powered induction terminal boxes.

// METHODOLOGY EMPLOYED:
// The terminal boxes are modeled as a collection of components: air mixer,
// fan, and heating coil plus an integrated control
// algorithm that adjusts the primary air flow and the heating coil output
// to meet the zone load.

// Using/Aliasing
using namespace DataLoopNode;
using HVAC::SmallAirVolFlow;
using HVAC::SmallLoad;
using HVAC::SmallMassFlow;
using HVAC::SmallTempDiff;
using Psychrometrics::PsyCpAirFnW;
using Psychrometrics::PsyHFnTdbW;
using SteamCoils::SimulateSteamCoilComponents;

constexpr std::array<std::string_view, static_cast<int>(FanCntrlType::Num)> fanCntrlTypeNames = {"ConstantSpeed", "VariableSpeed"};

constexpr std::array<std::string_view, static_cast<int>(FanCntrlType::Num)> fanCntrlTypeNamesUC = {"CONSTANTSPEED", "VARIABLESPEED"};

constexpr std::array<std::string_view, static_cast<int>(HeatCntrlBehaviorType::Num)> heatCntrlTypeNames = {"Staged", "Modulated"};

constexpr std::array<std::string_view, static_cast<int>(HeatCntrlBehaviorType::Num)> heatCntrlTypeNamesUC = {"STAGED", "MODULATED"};

void SimPIU(EnergyPlusData &state,
            std::string_view CompName,     // name of the PIU
            bool const FirstHVACIteration, // TRUE if first HVAC iteration in time step
            int const ZoneNum,             // index of zone served by PIU
            int const ZoneNodeNum,         // zone node number of zone served by PIU
            int &CompIndex                 // PIU Index in PIU names
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   March 2000
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Manages the simulation of a fan powered induction terminal unit.
    // Called from SimZoneAirLoopEquipmentin module ZoneAirLoopEquipmentManager.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int PIUNum = 0; // index of powered induction unit being simulated

    // First time SimPIU is called, get the input for all the fan coil units
    if (state.dataPowerInductionUnits->GetPIUInputFlag) {
        GetPIUs(state);
        state.dataPowerInductionUnits->GetPIUInputFlag = false;
    }

    // Get the powered induction unit index
    if (CompIndex == 0) {
        PIUNum = Util::FindItemInList(CompName, state.dataPowerInductionUnits->PIU);
        if (PIUNum == 0) {
            ShowFatalError(state, format("SimPIU: PIU Unit not found={}", CompName));
        }
        CompIndex = PIUNum;
    } else {
        PIUNum = CompIndex;
        if (PIUNum > state.dataPowerInductionUnits->NumPIUs || PIUNum < 1) {
            ShowFatalError(state,
                           format("SimPIU: Invalid CompIndex passed={}, Number of PIU Units={}, PIU Unit name={}",
                                  CompIndex,
                                  state.dataPowerInductionUnits->NumPIUs,
                                  CompName));
        }
        if (state.dataPowerInductionUnits->CheckEquipName(PIUNum)) {
            if (CompName != state.dataPowerInductionUnits->PIU(PIUNum).Name) {
                ShowFatalError(state,
                               format("SimPIU: Invalid CompIndex passed={}, PIU Unit name={}, stored PIU Unit Name for that index={}",
                                      CompIndex,
                                      CompName,
                                      state.dataPowerInductionUnits->PIU(PIUNum).Name));
            }
            state.dataPowerInductionUnits->CheckEquipName(PIUNum) = false;
        }
    }

    state.dataSize->CurTermUnitSizingNum =
        state.dataDefineEquipment->AirDistUnit(state.dataPowerInductionUnits->PIU(PIUNum).ADUNum).TermUnitSizingNum;
    // initialize the unit
    InitPIU(state, PIUNum, FirstHVACIteration);

    state.dataSize->TermUnitPIU = true;

    // Select the correct unit type
    switch (state.dataPowerInductionUnits->PIU(PIUNum).UnitType_Num) {

    case DataDefineEquip::ZnAirLoopEquipType::SingleDuct_SeriesPIU_Reheat: { //  'AirTerminal:SingleDuct:SeriesPIU:Reheat'

        CalcSeriesPIU(state, PIUNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
        break;
    }
    case DataDefineEquip::ZnAirLoopEquipType::SingleDuct_ParallelPIU_Reheat: { // 'AirTerminal:SingleDuct:ParallelPIU:Reheat'

        CalcParallelPIU(state, PIUNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
        break;
    }
    default:
        ShowSevereError(state, format("Illegal PI Unit Type used={}", state.dataPowerInductionUnits->PIU(PIUNum).UnitType));
        ShowContinueError(state, format("Occurs in PI Unit={}", state.dataPowerInductionUnits->PIU(PIUNum).Name));
        ShowFatalError(state, "Preceding condition causes termination.");
        break;
    }

    state.dataSize->TermUnitPIU = false;

    // Update the current unit's outlet nodes
    // no update needed: reheat coil updates outlet node; inlet nodes' mass flow rate set by Calc routine

    // Fill the report variables
    ReportPIU(state, PIUNum);
}

void GetPIUs(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   August 2000
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Obtains input data for powered induction unit terminal boxes and stores it
    // in PIU data structures

    // METHODOLOGY EMPLOYED:
    // Uses "Get" routines to read in data.

    // Using/Aliasing
    using BranchNodeConnections::SetUpCompSets;
    using BranchNodeConnections::TestCompSet;

    using NodeInputManager::GetOnlySingleNode;
    using SteamCoils::GetCoilSteamInletNode;
    using WaterCoils::GetCoilWaterInletNode;

    static constexpr std::string_view routineName = "GetPIUs";

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool ErrorsFound(false);                                    // Set to true if errors in input, fatal at end of routine
    static constexpr std::string_view RoutineName("GetPIUs: "); // include trailing blank space
    bool SteamMessageNeeded = true;

    // find the number of each type of fan coil unit
    state.dataPowerInductionUnits->NumSeriesPIUs =
        state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirTerminal:SingleDuct:SeriesPIU:Reheat");
    state.dataPowerInductionUnits->NumParallelPIUs =
        state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirTerminal:SingleDuct:ParallelPIU:Reheat");
    state.dataPowerInductionUnits->NumPIUs = state.dataPowerInductionUnits->NumSeriesPIUs + state.dataPowerInductionUnits->NumParallelPIUs;

    if (state.dataPowerInductionUnits->NumPIUs > 0) {
        // GetZonePlenumInput might call this routine before the AirDistUnit has been populated
        if (state.dataZoneAirLoopEquipmentManager->GetAirDistUnitsFlag) {
            ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment(state);
            state.dataZoneAirLoopEquipmentManager->GetAirDistUnitsFlag = false;
        }
    }

    // allocate the data structures
    state.dataPowerInductionUnits->PIU.allocate(state.dataPowerInductionUnits->NumPIUs);
    state.dataPowerInductionUnits->PiuUniqueNames.reserve(static_cast<unsigned>(state.dataPowerInductionUnits->NumPIUs));
    state.dataPowerInductionUnits->CheckEquipName.dimension(state.dataPowerInductionUnits->NumPIUs, true);

    int PIUNum = 0;
    auto &ip = state.dataInputProcessing->inputProcessor;
    // loop over Series PIUs; get and load the input data
    for (const std::string cCurrentModuleObject : {"AirTerminal:SingleDuct:SeriesPIU:Reheat", "AirTerminal:SingleDuct:ParallelPIU:Reheat"}) {
        auto const &objectSchemaProps = ip->getObjectSchemaProps(state, cCurrentModuleObject);
        auto const &PIUsInstances = ip->epJSON.find(cCurrentModuleObject);
        if (PIUsInstances != ip->epJSON.end()) {
            auto &PIUInstances = PIUsInstances.value();
            for (auto instance = PIUInstances.begin(); instance != PIUInstances.end(); ++instance) {
                ++PIUNum;
                auto const &fields = instance.value();

                ErrorObjectHeader eoh{routineName, cCurrentModuleObject, instance.key()};

                GlobalNames::VerifyUniqueInterObjectName(
                    state, state.dataPowerInductionUnits->PiuUniqueNames, Util::makeUPPER(instance.key()), cCurrentModuleObject, "Name", ErrorsFound);
                auto &thisPIU = state.dataPowerInductionUnits->PIU(PIUNum);
                thisPIU.Name = Util::makeUPPER(instance.key());
                thisPIU.UnitType = cCurrentModuleObject;
                ip->markObjectAsUsed(cCurrentModuleObject, instance.key());
                if (cCurrentModuleObject == "AirTerminal:SingleDuct:SeriesPIU:Reheat") {
                    thisPIU.UnitType_Num = DataDefineEquip::ZnAirLoopEquipType::SingleDuct_SeriesPIU_Reheat;
                } else if (cCurrentModuleObject == "AirTerminal:SingleDuct:ParallelPIU:Reheat") {
                    thisPIU.UnitType_Num = DataDefineEquip::ZnAirLoopEquipType::SingleDuct_ParallelPIU_Reheat;
                }

                std::string schedName = ip->getAlphaFieldValue(fields, objectSchemaProps, "availability_schedule_name");
                if (schedName.empty()) {
                    thisPIU.availSched = Sched::GetScheduleAlwaysOn(state);
                } else if ((thisPIU.availSched = Sched::GetSchedule(state, Util::makeUPPER(schedName))) == nullptr) {
                    ShowWarningItemNotFound(
                        state, eoh, "Availability Schedule Name", schedName, "Set the default as Always On. Simulation continues.");
                    thisPIU.availSched = Sched::GetScheduleAlwaysOn(state);
                }

                if (cCurrentModuleObject == "AirTerminal:SingleDuct:SeriesPIU:Reheat") {
                    thisPIU.MaxTotAirVolFlow = ip->getRealFieldValue(fields, objectSchemaProps, "maximum_air_flow_rate");
                }
                if (cCurrentModuleObject == "AirTerminal:SingleDuct:ParallelPIU:Reheat") {
                    thisPIU.MaxSecAirVolFlow = ip->getRealFieldValue(fields, objectSchemaProps, "maximum_secondary_air_flow_rate");
                }
                thisPIU.MaxPriAirVolFlow = ip->getRealFieldValue(fields, objectSchemaProps, "maximum_primary_air_flow_rate");
                thisPIU.MinPriAirFlowFrac = ip->getRealFieldValue(fields, objectSchemaProps, "minimum_primary_air_flow_fraction");
                if (cCurrentModuleObject == "AirTerminal:SingleDuct:ParallelPIU:Reheat") {
                    thisPIU.FanOnFlowFrac = ip->getRealFieldValue(fields, objectSchemaProps, "fan_on_flow_fraction");
                }
                thisPIU.HCoilType = static_cast<HtgCoilType>(
                    getEnumValue(HCoilNamesUC, Util::makeUPPER(ip->getAlphaFieldValue(fields, objectSchemaProps, "reheat_coil_object_type"))));
                switch (thisPIU.HCoilType) {
                case HtgCoilType::SimpleHeating: {
                    thisPIU.HCoil_PlantType = DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
                    break;
                }
                case HtgCoilType::Electric:
                case HtgCoilType::Gas: {
                    break;
                }
                case HtgCoilType::SteamAirHeating: {
                    thisPIU.HCoil_PlantType = DataPlant::PlantEquipmentType::CoilSteamAirHeating;
                    thisPIU.HCoil_fluid = Fluid::GetSteam(state);
                    if (thisPIU.HCoil_fluid == nullptr) {
                        ShowSevereError(state, format("{} Steam Properties for {} not found.", RoutineName, thisPIU.Name));
                        if (SteamMessageNeeded) {
                            ShowContinueError(state, "Steam Fluid Properties should have been included in the input file.");
                        }
                        ErrorsFound = true;
                        SteamMessageNeeded = false;
                    }
                    break;
                }
                default: {
                    ShowSevereError(state, format("Illegal Reheat Coil Type = {}", thisPIU.HCoilType));
                    ShowContinueError(state, format("Occurs in {} = {}", cCurrentModuleObject, thisPIU.Name));
                    ErrorsFound = true;
                }
                }

                auto connectionType = DataLoopNode::ConnectionObjectType::AirTerminalSingleDuctSeriesPIUReheat;
                if (cCurrentModuleObject == "AirTerminal:SingleDuct:ParallelPIU:Reheat") {
                    connectionType = DataLoopNode::ConnectionObjectType::AirTerminalSingleDuctParallelPIUReheat;
                }
                thisPIU.PriAirInNode = GetOnlySingleNode(state,
                                                         ip->getAlphaFieldValue(fields, objectSchemaProps, "supply_air_inlet_node_name"),
                                                         ErrorsFound,
                                                         connectionType,
                                                         thisPIU.Name,
                                                         DataLoopNode::NodeFluidType::Air,
                                                         DataLoopNode::ConnectionType::Inlet,
                                                         NodeInputManager::CompFluidStream::Primary,
                                                         ObjectIsParent,
                                                         "Supply Air Inlet Node Name");

                thisPIU.SecAirInNode = GetOnlySingleNode(state,
                                                         ip->getAlphaFieldValue(fields, objectSchemaProps, "secondary_air_inlet_node_name"),
                                                         ErrorsFound,
                                                         connectionType,
                                                         thisPIU.Name,
                                                         DataLoopNode::NodeFluidType::Air,
                                                         DataLoopNode::ConnectionType::Inlet,
                                                         NodeInputManager::CompFluidStream::Primary,
                                                         ObjectIsParent,
                                                         "Secondary Air Inlet Node Name");

                thisPIU.OutAirNode = GetOnlySingleNode(state,
                                                       ip->getAlphaFieldValue(fields, objectSchemaProps, "outlet_node_name"),
                                                       ErrorsFound,
                                                       connectionType,
                                                       thisPIU.Name,
                                                       DataLoopNode::NodeFluidType::Air,
                                                       DataLoopNode::ConnectionType::Outlet,
                                                       NodeInputManager::CompFluidStream::Primary,
                                                       ObjectIsParent,
                                                       "Outlet Node Name");

                thisPIU.HCoilInAirNode = GetOnlySingleNode(state,
                                                           ip->getAlphaFieldValue(fields, objectSchemaProps, "reheat_coil_air_inlet_node_name"),
                                                           ErrorsFound,
                                                           connectionType,
                                                           thisPIU.Name,
                                                           DataLoopNode::NodeFluidType::Air,
                                                           DataLoopNode::ConnectionType::Internal,
                                                           NodeInputManager::CompFluidStream::Primary,
                                                           ObjectIsParent,
                                                           "Reheat Coil Air Inlet Node Name");
                // The reheat coil control node is necessary for hot water reheat, but not necessary for
                // electric or gas reheat.
                if (thisPIU.HCoilType == HtgCoilType::SimpleHeating) {
                    thisPIU.HotControlNode = GetCoilWaterInletNode(state,
                                                                   ip->getAlphaFieldValue(fields, objectSchemaProps, "reheat_coil_object_type"),
                                                                   ip->getAlphaFieldValue(fields, objectSchemaProps, "reheat_coil_name"),
                                                                   ErrorsFound);
                }
                if (thisPIU.HCoilType == HtgCoilType::SteamAirHeating) {
                    thisPIU.HotControlNode = GetCoilSteamInletNode(state,
                                                                   ip->getAlphaFieldValue(fields, objectSchemaProps, "reheat_coil_object_type"),
                                                                   ip->getAlphaFieldValue(fields, objectSchemaProps, "reheat_coil_name"),
                                                                   ErrorsFound);
                }
                thisPIU.MixerName = ip->getAlphaFieldValue(fields, objectSchemaProps, "zone_mixer_name");
                thisPIU.FanName = ip->getAlphaFieldValue(fields, objectSchemaProps, "fan_name");

                // find fan type
                // test if Fan:SystemModel fan of this name exists
                if ((thisPIU.Fan_Index = Fans::GetFanIndex(state, thisPIU.FanName)) == 0) {
                    ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(8), thisPIU.FanName);
                    ErrorsFound = true;
                } else {
                    // Assert that this is a constant volume fan?
                    auto *fan = state.dataFans->fans(thisPIU.Fan_Index);
                    thisPIU.fanType = fan->type;
                    thisPIU.fanAvailSched = fan->availSched;
                }

                thisPIU.HCoil = ip->getAlphaFieldValue(fields, objectSchemaProps, "reheat_coil_name");
                bool IsNotOK = false;
                ValidateComponent(
                    state, HCoilNamesUC[static_cast<int>(thisPIU.HCoilType)], thisPIU.HCoil, IsNotOK, cCurrentModuleObject + " - Heating Coil");
                if (IsNotOK) {
                    ShowContinueError(state, format("In {} = {}", cCurrentModuleObject, thisPIU.Name));
                    ErrorsFound = true;
                }
                thisPIU.MaxVolHotWaterFlow = ip->getRealFieldValue(fields, objectSchemaProps, "maximum_hot_water_or_steam_flow_rate");
                thisPIU.MinVolHotWaterFlow = ip->getRealFieldValue(fields, objectSchemaProps, "minimum_hot_water_or_steam_flow_rate");
                thisPIU.HotControlOffset = ip->getRealFieldValue(fields, objectSchemaProps, "convergence_tolerance");
                // Set default convergence tolerance
                if (thisPIU.HotControlOffset <= 0.0) {
                    thisPIU.HotControlOffset = 0.001;
                }

                // Variable speed fan inputs
                std::string const fan_control_type = ip->getAlphaFieldValue(fields, objectSchemaProps, "fan_control_type");
                thisPIU.fanControlType = static_cast<FanCntrlType>(getEnumValue(fanCntrlTypeNamesUC, Util::makeUPPER(fan_control_type)));

                if (thisPIU.fanControlType == FanCntrlType::Invalid) {
                    ShowSevereError(state, format("Illegal Fan Control Type = {}", fan_control_type));
                    ShowContinueError(state, format("Occurs in {} = {}", cCurrentModuleObject, thisPIU.Name));
                    ErrorsFound = true;
                }
                if (thisPIU.fanControlType == FanCntrlType::VariableSpeedFan) {
                    if (thisPIU.fanType != HVAC::FanType::SystemModel) {
                        ErrorsFound = true;
                        ShowSevereError(state, format("Fan type must be Fan:SystemModel when Fan Control Type = {}", fan_control_type));
                        ShowContinueError(state, format("Occurs in {} = {}", cCurrentModuleObject, thisPIU.Name));
                    }
                    // Heating Control Type is only applicable for variable speed fans
                    thisPIU.heatingControlType = static_cast<HeatCntrlBehaviorType>(getEnumValue(
                        heatCntrlTypeNamesUC, Util::makeUPPER(ip->getAlphaFieldValue(fields, objectSchemaProps, "heating_control_type"))));

                    if (thisPIU.heatingControlType == HeatCntrlBehaviorType::Invalid) {
                        ShowSevereError(state, "Heating Control Type should either be Staged or Modulated");
                        ShowContinueError(state, format("Occurs in {} = {}", cCurrentModuleObject, thisPIU.Name));
                        ErrorsFound = true;
                    }
                }

                thisPIU.MinFanTurnDownRatio = ip->getRealFieldValue(fields, objectSchemaProps, "minimum_fan_turn_down_ratio");
                thisPIU.designHeatingDAT = ip->getRealFieldValue(fields, objectSchemaProps, "design_heating_discharge_air_temperature");
                thisPIU.highLimitDAT = ip->getRealFieldValue(fields, objectSchemaProps, "high_limit_heating_discharge_air_temperature");

                // Add fan to component sets array
                if (cCurrentModuleObject == "AirTerminal:SingleDuct:SeriesPIU:Reheat") {
                    SetUpCompSets(state,
                                  thisPIU.UnitType,
                                  thisPIU.Name,
                                  "UNDEFINED",
                                  thisPIU.FanName,
                                  "UNDEFINED",
                                  ip->getAlphaFieldValue(fields, objectSchemaProps, "reheat_coil_air_inlet_node_name"));
                } else if (cCurrentModuleObject == "AirTerminal:SingleDuct:ParallelPIU:Reheat") {
                    SetUpCompSets(state,
                                  thisPIU.UnitType,
                                  thisPIU.Name,
                                  "UNDEFINED",
                                  thisPIU.FanName,
                                  ip->getAlphaFieldValue(fields, objectSchemaProps, "secondary_air_inlet_node_name"),
                                  "UNDEFINED");
                }

                // Add reheat coil to component sets array
                SetUpCompSets(state,
                              thisPIU.UnitType,
                              thisPIU.Name,
                              ip->getAlphaFieldValue(fields, objectSchemaProps, "reheat_coil_object_type"),
                              ip->getAlphaFieldValue(fields, objectSchemaProps, "reheat_coil_name"),
                              ip->getAlphaFieldValue(fields, objectSchemaProps, "reheat_coil_air_inlet_node_name"),
                              ip->getAlphaFieldValue(fields, objectSchemaProps, "outlet_node_name"));

                // Register component set data
                TestCompSet(state,
                            thisPIU.UnitType,
                            thisPIU.Name,
                            state.dataLoopNodes->NodeID(thisPIU.PriAirInNode),
                            state.dataLoopNodes->NodeID(thisPIU.OutAirNode),
                            "Air Nodes");

                for (int ADUNum = 1; ADUNum <= (int)state.dataDefineEquipment->AirDistUnit.size(); ++ADUNum) {
                    if (thisPIU.OutAirNode == state.dataDefineEquipment->AirDistUnit(ADUNum).OutletNodeNum) {
                        thisPIU.ADUNum = ADUNum;
                        state.dataDefineEquipment->AirDistUnit(ADUNum).InletNodeNum = thisPIU.PriAirInNode;
                    }
                }
                // one assumes if there isn't one assigned, it's an error?
                if (thisPIU.ADUNum == 0) {
                    ShowSevereError(state,
                                    format("{}No matching Air Distribution Unit, for PIU = [{},{}].", RoutineName, thisPIU.UnitType, thisPIU.Name));
                    ShowContinueError(state, format("...should have outlet node = {}", state.dataLoopNodes->NodeID(thisPIU.OutAirNode)));
                    ErrorsFound = true;
                } else {

                    bool AirNodeFound = false;
                    // Fill the Zone Equipment data with the supply air inlet node number of this unit.
                    for (int CtrlZone = 1; CtrlZone <= state.dataGlobal->NumOfZones; ++CtrlZone) {
                        if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZone).IsControlled) {
                            continue;
                        }
                        for (int SupAirIn = 1; SupAirIn <= state.dataZoneEquip->ZoneEquipConfig(CtrlZone).NumInletNodes; ++SupAirIn) {
                            if (thisPIU.OutAirNode == state.dataZoneEquip->ZoneEquipConfig(CtrlZone).InletNode(SupAirIn)) {
                                state.dataZoneEquip->ZoneEquipConfig(CtrlZone).AirDistUnitCool(SupAirIn).InNode = thisPIU.PriAirInNode;
                                state.dataZoneEquip->ZoneEquipConfig(CtrlZone).AirDistUnitCool(SupAirIn).OutNode = thisPIU.OutAirNode;
                                state.dataDefineEquipment->AirDistUnit(thisPIU.ADUNum).TermUnitSizingNum =
                                    state.dataZoneEquip->ZoneEquipConfig(CtrlZone).AirDistUnitCool(SupAirIn).TermUnitSizingIndex;
                                state.dataDefineEquipment->AirDistUnit(thisPIU.ADUNum).ZoneEqNum = CtrlZone;
                                AirNodeFound = true;
                                thisPIU.CtrlZoneNum = CtrlZone; // fill index for later use in finding air loop index
                                thisPIU.ctrlZoneInNodeIndex = SupAirIn;
                                break;
                            }
                        }
                    }
                    if (!AirNodeFound) {
                        ShowSevereError(state, format("The outlet air node from the {} Unit = {}", cCurrentModuleObject, thisPIU.Name));
                        ShowContinueError(
                            state, format("did not have a matching Zone Equipment Inlet Node, Node = {}", state.dataIPShortCut->cAlphaArgs(5)));
                        ErrorsFound = true;
                    }
                }
            }
        }
    }

    if (ErrorsFound) {
        ShowFatalError(state, format("{} Errors found in getting input.  Preceding conditions cause termination.", RoutineName));
    }

    for (int PIURpt = 1; PIURpt <= state.dataPowerInductionUnits->NumPIUs; ++PIURpt) {
        auto &thisPIU = state.dataPowerInductionUnits->PIU(PIURpt);

        // Setup Report variables for the PIUs
        SetupOutputVariable(state,
                            "Zone Air Terminal Primary Damper Position",
                            Constant::Units::None,
                            thisPIU.PriDamperPosition,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisPIU.Name);
        SetupOutputVariable(state,
                            "Zone Air Terminal Heating Rate",
                            Constant::Units::W,
                            thisPIU.HeatingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisPIU.Name);
        SetupOutputVariable(state,
                            "Zone Air Terminal Heating Energy",
                            Constant::Units::J,
                            thisPIU.HeatingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisPIU.Name);
        SetupOutputVariable(state,
                            "Zone Air Terminal Sensible Cooling Rate",
                            Constant::Units::W,
                            thisPIU.SensCoolRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisPIU.Name);
        SetupOutputVariable(state,
                            "Zone Air Terminal Sensible Cooling Energy",
                            Constant::Units::J,
                            thisPIU.SensCoolEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisPIU.Name);
        SetupOutputVariable(state,
                            "Zone Air Terminal Outdoor Air Volume Flow Rate",
                            Constant::Units::m3_s,
                            thisPIU.OutdoorAirFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisPIU.Name);
        SetupOutputVariable(state,
                            "Zone Air Terminal Total Air Mass Flow Rate",
                            Constant::Units::kg_s,
                            thisPIU.TotMassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            state.dataPowerInductionUnits->PIU(PIURpt).Name);
        SetupOutputVariable(state,
                            "Zone Air Terminal Primary Air Mass Flow Rate",
                            Constant::Units::kg_s,
                            thisPIU.PriMassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            state.dataPowerInductionUnits->PIU(PIURpt).Name);
        SetupOutputVariable(state,
                            "Zone Air Terminal Secondary Air Mass Flow Rate",
                            Constant::Units::kg_s,
                            thisPIU.SecMassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            state.dataPowerInductionUnits->PIU(PIURpt).Name);
        SetupOutputVariable(state,
                            "Zone Air Terminal Outlet Discharge Air Temperature",
                            Constant::Units::C,
                            thisPIU.DischargeAirTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            state.dataPowerInductionUnits->PIU(PIURpt).Name);
        SetupOutputVariable(state,
                            "Zone Air Terminal Current Operation Control Stage",
                            Constant::Units::unknown,
                            thisPIU.CurOperationControlStage,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            state.dataPowerInductionUnits->PIU(PIURpt).Name);
    }
}

void InitPIU(EnergyPlusData &state,
             int const PIUNum,             // number of the current fan coil unit being simulated
             bool const FirstHVACIteration // TRUE if first zone equip this HVAC step
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   August 2000
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is for initializations of the powered induction unit
    // terminal boxe.

    // METHODOLOGY EMPLOYED:
    // Uses the status flags to trigger initializations.

    // Using/Aliasing

    using DataZoneEquipment::CheckZoneEquipmentList;
    using PlantUtilities::InitComponentNodes;
    using PlantUtilities::ScanPlantLoopsForObject;

    auto &thisPIU = state.dataPowerInductionUnits->PIU(PIUNum);
    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("InitPIU");

    // Do the one time initializations
    if (state.dataPowerInductionUnits->MyOneTimeFlag) {
        state.dataPowerInductionUnits->MyEnvrnFlag.dimension(state.dataPowerInductionUnits->NumPIUs, true);
        state.dataPowerInductionUnits->MySizeFlag.dimension(state.dataPowerInductionUnits->NumPIUs, true);
        state.dataPowerInductionUnits->MyPlantScanFlag.dimension(state.dataPowerInductionUnits->NumPIUs, true);
        state.dataPowerInductionUnits->MyOneTimeFlag = false;
    }

    if (state.dataPowerInductionUnits->MyPlantScanFlag(PIUNum) && allocated(state.dataPlnt->PlantLoop)) {
        if ((thisPIU.HCoil_PlantType == DataPlant::PlantEquipmentType::CoilWaterSimpleHeating) ||
            (thisPIU.HCoil_PlantType == DataPlant::PlantEquipmentType::CoilSteamAirHeating)) {
            bool errFlag = false;
            ScanPlantLoopsForObject(state, thisPIU.HCoil, thisPIU.HCoil_PlantType, thisPIU.HWplantLoc, errFlag, _, _, _, _, _);
            if (errFlag) {
                ShowFatalError(state, "InitPIU: Program terminated due to previous condition(s).");
            }
            thisPIU.HotCoilOutNodeNum = DataPlant::CompData::getPlantComponent(state, thisPIU.HWplantLoc).NodeNumOut;
        }
        state.dataPowerInductionUnits->MyPlantScanFlag(PIUNum) = false;
    } else if (state.dataPowerInductionUnits->MyPlantScanFlag(PIUNum) && !state.dataGlobal->AnyPlantInModel) {
        state.dataPowerInductionUnits->MyPlantScanFlag(PIUNum) = false;
    }

    if (!state.dataPowerInductionUnits->ZoneEquipmentListChecked && state.dataZoneEquip->ZoneEquipInputsFilled) {
        state.dataPowerInductionUnits->ZoneEquipmentListChecked = true;
        // Check to see if there is a Air Distribution Unit on the Zone Equipment List
        for (int Loop = 1; Loop <= state.dataPowerInductionUnits->NumPIUs; ++Loop) {
            if (state.dataPowerInductionUnits->PIU(Loop).ADUNum == 0) {
                continue;
            }
            if (CheckZoneEquipmentList(state,
                                       "ZoneHVAC:AirDistributionUnit",
                                       state.dataDefineEquipment->AirDistUnit(state.dataPowerInductionUnits->PIU(Loop).ADUNum).Name)) {
                continue;
            }
            ShowSevereError(state,
                            format("InitPIU: ADU=[Air Distribution Unit,{}] is not on any ZoneHVAC:EquipmentList.",
                                   state.dataDefineEquipment->AirDistUnit(state.dataPowerInductionUnits->PIU(Loop).ADUNum).Name));
            ShowContinueError(state,
                              format("...PIU=[{},{}] will not be simulated.",
                                     state.dataPowerInductionUnits->PIU(Loop).UnitType,
                                     state.dataPowerInductionUnits->PIU(Loop).Name));
        }
    }

    if (!state.dataGlobal->SysSizingCalc && state.dataPowerInductionUnits->MySizeFlag(PIUNum) &&
        !state.dataPowerInductionUnits->MyPlantScanFlag(PIUNum)) {

        SizePIU(state, PIUNum);

        // If there's a hot water control node number defined in PIU
        if (thisPIU.HotControlNode > 0) {
            // plant upgrade note? why no separate handling of steam coil? add it ?
            // local plant fluid density
            Real64 const rho = state.dataPlnt->PlantLoop(thisPIU.HWplantLoc.loopNum).glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);

            thisPIU.MaxHotWaterFlow = rho * thisPIU.MaxVolHotWaterFlow;
            thisPIU.MinHotWaterFlow = rho * thisPIU.MinVolHotWaterFlow;
            InitComponentNodes(state, thisPIU.MinHotWaterFlow, thisPIU.MaxHotWaterFlow, thisPIU.HotControlNode, thisPIU.HotCoilOutNodeNum);
        }

        state.dataPowerInductionUnits->MySizeFlag(PIUNum) = false;
    }

    int const PriNode = thisPIU.PriAirInNode;
    int const SecNode = thisPIU.SecAirInNode;

    // Do the Begin Environment initializations
    if (state.dataGlobal->BeginEnvrnFlag && state.dataPowerInductionUnits->MyEnvrnFlag(PIUNum)) {
        Real64 const RhoAir = state.dataEnvrn->StdRhoAir;
        int const OutletNode = thisPIU.OutAirNode;
        // set the mass flow rates from the input volume flow rates
        if (thisPIU.UnitType == "AirTerminal:SingleDuct:SeriesPIU:Reheat") {
            thisPIU.MaxTotAirMassFlow = RhoAir * thisPIU.MaxTotAirVolFlow;
            thisPIU.MaxPriAirMassFlow = RhoAir * thisPIU.MaxPriAirVolFlow;
            thisPIU.MinPriAirMassFlow = RhoAir * thisPIU.MinPriAirFlowFrac * thisPIU.MaxPriAirVolFlow;
            state.dataLoopNodes->Node(PriNode).MassFlowRateMax = thisPIU.MaxPriAirMassFlow;
            state.dataLoopNodes->Node(PriNode).MassFlowRateMin = thisPIU.MinPriAirMassFlow;
            state.dataLoopNodes->Node(OutletNode).MassFlowRateMax = thisPIU.MaxTotAirMassFlow;
        } else {
            // parallel
            thisPIU.MaxPriAirMassFlow = RhoAir * thisPIU.MaxPriAirVolFlow;
            thisPIU.MinPriAirMassFlow = RhoAir * thisPIU.MinPriAirFlowFrac * thisPIU.MaxPriAirVolFlow;
            thisPIU.MaxSecAirMassFlow = RhoAir * thisPIU.MaxSecAirVolFlow;
            thisPIU.FanOnAirMassFlow = RhoAir * thisPIU.FanOnFlowFrac * thisPIU.MaxPriAirVolFlow;
            state.dataLoopNodes->Node(PriNode).MassFlowRateMax = thisPIU.MaxPriAirMassFlow;
            state.dataLoopNodes->Node(PriNode).MassFlowRateMin = thisPIU.MinPriAirMassFlow;
            state.dataLoopNodes->Node(OutletNode).MassFlowRateMax = thisPIU.MaxPriAirMassFlow;
        }
        if (thisPIU.fanControlType == FanCntrlType::VariableSpeedFan) {
            if (thisPIU.UnitType == "AirTerminal:SingleDuct:SeriesPIU:Reheat") {
                thisPIU.MinTotAirMassFlow = thisPIU.MaxTotAirMassFlow * thisPIU.MinFanTurnDownRatio;
                thisPIU.MaxSecAirVolFlow = thisPIU.MaxTotAirMassFlow - thisPIU.MinPriAirMassFlow;
                thisPIU.MaxSecAirMassFlow = RhoAir * thisPIU.MaxSecAirVolFlow;
                thisPIU.MinSecAirMassFlow = max(0.0, thisPIU.MinTotAirMassFlow - thisPIU.MinPriAirMassFlow);
            } else {
                thisPIU.MaxSecAirMassFlow = RhoAir * thisPIU.MaxSecAirVolFlow;
                thisPIU.MinSecAirMassFlow = max(0.0, thisPIU.MaxSecAirMassFlow * thisPIU.MinFanTurnDownRatio);
                thisPIU.MinTotAirMassFlow = thisPIU.MinSecAirMassFlow + thisPIU.MinPriAirMassFlow;
            }
        }

        if (((thisPIU.HCoilType == HtgCoilType::SimpleHeating) || (thisPIU.HCoilType == HtgCoilType::SteamAirHeating)) &&
            !state.dataPowerInductionUnits->MyPlantScanFlag(PIUNum)) {
            InitComponentNodes(state, thisPIU.MinHotWaterFlow, thisPIU.MaxHotWaterFlow, thisPIU.HotControlNode, thisPIU.HotCoilOutNodeNum);
        }

        if (thisPIU.AirLoopNum == 0) { // fill air loop index
            if (thisPIU.CtrlZoneNum > 0 && thisPIU.ctrlZoneInNodeIndex > 0) {
                thisPIU.AirLoopNum = state.dataZoneEquip->ZoneEquipConfig(thisPIU.CtrlZoneNum).InletNodeAirLoopNum(thisPIU.ctrlZoneInNodeIndex);
                state.dataDefineEquipment->AirDistUnit(thisPIU.ADUNum).AirLoopNum = thisPIU.AirLoopNum;
                // Set the airloopnum for the PIU fan
                state.dataFans->fans(thisPIU.Fan_Index)->airLoopNum = thisPIU.AirLoopNum;
            }
        }

        state.dataPowerInductionUnits->MyEnvrnFlag(PIUNum) = false;
    } // end one time inits

    if (!state.dataGlobal->BeginEnvrnFlag) {
        state.dataPowerInductionUnits->MyEnvrnFlag(PIUNum) = true;
    }

    // Do the start of HVAC time step initializations
    if (FirstHVACIteration) {
        // check for upstream zero flow. If nonzero and schedule ON, set primary flow to max
        if (thisPIU.availSched->getCurrentVal() > 0.0 && state.dataLoopNodes->Node(PriNode).MassFlowRate > 0.0) {
            if (thisPIU.UnitType == "AirTerminal:SingleDuct:SeriesPIU:Reheat") {
                state.dataLoopNodes->Node(PriNode).MassFlowRate = thisPIU.MaxPriAirMassFlow;
                state.dataLoopNodes->Node(SecNode).MassFlowRate = max(0.0, thisPIU.MaxTotAirMassFlow - thisPIU.MaxPriAirMassFlow);
            } else {
                state.dataLoopNodes->Node(PriNode).MassFlowRate = thisPIU.MaxPriAirMassFlow;
                state.dataLoopNodes->Node(SecNode).MassFlowRate = thisPIU.MaxSecAirMassFlow;
            }
        } else {
            state.dataLoopNodes->Node(PriNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(SecNode).MassFlowRate = 0.0;
        }
        // reset the max and min avail flows
        if (thisPIU.availSched->getCurrentVal() > 0.0 && state.dataLoopNodes->Node(PriNode).MassFlowRateMaxAvail > 0.0) {
            if (thisPIU.UnitType == "AirTerminal:SingleDuct:SeriesPIU:Reheat") {
                state.dataLoopNodes->Node(PriNode).MassFlowRateMaxAvail = thisPIU.MaxPriAirMassFlow;
                state.dataLoopNodes->Node(PriNode).MassFlowRateMinAvail = thisPIU.MinPriAirMassFlow;
                state.dataLoopNodes->Node(SecNode).MassFlowRateMaxAvail = max(0.0, thisPIU.MaxTotAirMassFlow - thisPIU.MinPriAirMassFlow);
                state.dataLoopNodes->Node(SecNode).MassFlowRateMinAvail = max(0.0, thisPIU.MaxTotAirMassFlow - thisPIU.MaxPriAirMassFlow);
            } else {
                state.dataLoopNodes->Node(PriNode).MassFlowRateMaxAvail = thisPIU.MaxPriAirMassFlow;
                state.dataLoopNodes->Node(PriNode).MassFlowRateMinAvail = thisPIU.MinPriAirMassFlow;
                state.dataLoopNodes->Node(SecNode).MassFlowRateMaxAvail = thisPIU.MaxSecAirMassFlow;
                state.dataLoopNodes->Node(SecNode).MassFlowRateMinAvail = 0.0;
            }
        } else {
            state.dataLoopNodes->Node(PriNode).MassFlowRateMaxAvail = 0.0;
            state.dataLoopNodes->Node(PriNode).MassFlowRateMinAvail = 0.0;
            state.dataLoopNodes->Node(SecNode).MassFlowRateMaxAvail = 0.0;
            state.dataLoopNodes->Node(SecNode).MassFlowRateMinAvail = 0.0;
        }
    }

    // Do the following initializations every time step

    // None needed
}

void SizePIU(EnergyPlusData &state, int const PIUNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   January 2002
    //       MODIFIED       August 2013 Daeho Kang, add component sizing table entries
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is for sizing PIU terminal units for which flow rates have not been
    // specified in the input.

    // METHODOLOGY EMPLOYED:
    // Obtains flow rates from the zone or system sizing arrays.

    // Using/Aliasing
    using namespace DataSizing;
    using SteamCoils::GetCoilSteamInletNode;
    using SteamCoils::GetCoilSteamOutletNode;
    using WaterCoils::GetCoilWaterInletNode;
    using WaterCoils::GetCoilWaterOutletNode;
    using WaterCoils::SetCoilDesFlow;

    using PlantUtilities::MyPlantSizingIndex;

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("SizePIU");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool IsAutoSize = false;           // Indicator to autosize
    bool IsMaxPriFlowAutoSize = false; // Indicate if the maximum terminal flow is autosize
    int SysSizNum = 0;                 // System sizing number
    Real64 DesCoilLoad = 0.0;
    bool ErrorsFound = false;

    auto &TermUnitSizing(state.dataSize->TermUnitSizing);
    auto &CurTermUnitSizingNum(state.dataSize->CurTermUnitSizingNum);
    auto &thisPIU = state.dataPowerInductionUnits->PIU(PIUNum);

    if (thisPIU.MaxPriAirVolFlow == AutoSize) {
        IsAutoSize = true;
    }
    if ((state.dataSize->CurZoneEqNum > 0) && (CurTermUnitSizingNum > 0)) {
        if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // Simulation continue
            if (thisPIU.MaxPriAirVolFlow > 0.0) {
                BaseSizer::reportSizerOutput(
                    state, thisPIU.UnitType, thisPIU.Name, "User-Specified Maximum Primary Air Flow Rate [m3/s]", thisPIU.MaxPriAirVolFlow);
            }
        } else {
            CheckZoneSizing(state, thisPIU.UnitType, thisPIU.Name);
            // Autosized maximum primary air flow for reporting
            Real64 MaxPriAirVolFlowDes = max(state.dataSize->TermUnitFinalZoneSizing(CurTermUnitSizingNum).DesCoolVolFlow,
                                             state.dataSize->TermUnitFinalZoneSizing(CurTermUnitSizingNum).DesHeatVolFlow);
            if (MaxPriAirVolFlowDes < SmallAirVolFlow) {
                MaxPriAirVolFlowDes = 0.0;
            }

            if (IsAutoSize) {
                thisPIU.MaxPriAirVolFlow = MaxPriAirVolFlowDes;
                IsMaxPriFlowAutoSize = true;
                BaseSizer::reportSizerOutput(
                    state, thisPIU.UnitType, thisPIU.Name, "Design Size Maximum Primary Air Flow Rate [m3/s]", MaxPriAirVolFlowDes);
            } else {
                if (thisPIU.MaxPriAirVolFlow > 0.0 && MaxPriAirVolFlowDes > 0.0) {
                    // Hardsized maximum primary air flow for reporting
                    Real64 const MaxPriAirVolFlowUser = thisPIU.MaxPriAirVolFlow;
                    BaseSizer::reportSizerOutput(state,
                                                 thisPIU.UnitType,
                                                 thisPIU.Name,
                                                 "Design Size Maximum Primary Air Flow Rate [m3/s]",
                                                 MaxPriAirVolFlowDes,
                                                 "User-Specified Maximum Primary Air Flow Rate [m3/s]",
                                                 MaxPriAirVolFlowUser);
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        if ((std::abs(MaxPriAirVolFlowDes - MaxPriAirVolFlowUser) / MaxPriAirVolFlowUser) >
                            state.dataSize->AutoVsHardSizingThreshold) {
                            ShowMessage(state, format("SizePIU: Potential issue with equipment sizing for {} {}", thisPIU.UnitType, thisPIU.Name));
                            ShowContinueError(state, format("User-Specified Primary Air Flow Rate of {:.5R} [m3/s]", MaxPriAirVolFlowUser));
                            ShowContinueError(state, format("differs from Design Size Primary Air Flow Rate of {:.5R} [m3/s]", MaxPriAirVolFlowDes));
                            ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                        }
                    }
                }
            }
        }
    }

    IsAutoSize = false;
    if (thisPIU.MaxTotAirVolFlow == AutoSize) {
        IsAutoSize = true;
    }
    if ((state.dataSize->CurZoneEqNum > 0) && (CurTermUnitSizingNum > 0)) {
        if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // Simulation continue
            if (thisPIU.MaxTotAirVolFlow > 0.0) {
                BaseSizer::reportSizerOutput(
                    state, thisPIU.UnitType, thisPIU.Name, "User-Specified Maximum Air Flow Rate [m3/s]", thisPIU.MaxTotAirVolFlow);
            }
        } else {
            CheckZoneSizing(state, thisPIU.UnitType, thisPIU.Name);
            // Autosized maximum air flow for reporting
            Real64 MaxTotAirVolFlowDes = max(state.dataSize->TermUnitFinalZoneSizing(CurTermUnitSizingNum).DesCoolVolFlow,
                                             state.dataSize->TermUnitFinalZoneSizing(CurTermUnitSizingNum).DesHeatVolFlow);
            if (MaxTotAirVolFlowDes < SmallAirVolFlow) {
                MaxTotAirVolFlowDes = 0.0;
            }
            if (IsAutoSize) {
                thisPIU.MaxTotAirVolFlow = MaxTotAirVolFlowDes;
                BaseSizer::reportSizerOutput(state, thisPIU.UnitType, thisPIU.Name, "Design Size Maximum Air Flow Rate [m3/s]", MaxTotAirVolFlowDes);
            } else {
                if (thisPIU.MaxTotAirVolFlow > 0.0 && MaxTotAirVolFlowDes > 0.0) {
                    // Hardsized maximum air flow for reporting
                    Real64 const MaxTotAirVolFlowUser = thisPIU.MaxTotAirVolFlow;
                    BaseSizer::reportSizerOutput(state,
                                                 thisPIU.UnitType,
                                                 thisPIU.Name,
                                                 "Design Size Maximum Air Flow Rate [m3/s]",
                                                 MaxTotAirVolFlowDes,
                                                 "User-Specified Maximum Air Flow Rate [m3/s]",
                                                 MaxTotAirVolFlowUser);
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        if ((std::abs(MaxTotAirVolFlowDes - MaxTotAirVolFlowUser) / MaxTotAirVolFlowUser) >
                            state.dataSize->AutoVsHardSizingThreshold) {
                            ShowMessage(state, format("SizePIU: Potential issue with equipment sizing for {} {}", thisPIU.UnitType, thisPIU.Name));
                            ShowContinueError(state, format("User-Specified Maximum Air Flow Rate of {:.5R} [m3/s]", MaxTotAirVolFlowUser));
                            ShowContinueError(state, format("differs from Design Size Maximum Air Flow Rate of {:.5R} [m3/s]", MaxTotAirVolFlowDes));
                            ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                        }
                    }
                }
            }
        }
    }

    // if a sizing run has been done, check if system sizing has been done for this system
    bool SizingDesRunThisAirSys = false;
    if (state.dataSize->SysSizingRunDone) {
        int const AirLoopNum = state.dataZoneEquip->ZoneEquipConfig(thisPIU.CtrlZoneNum).InletNodeAirLoopNum(thisPIU.ctrlZoneInNodeIndex);
        if (AirLoopNum > 0) {
            CheckThisAirSystemForSizing(state, AirLoopNum, SizingDesRunThisAirSys);
        }

        // get system sizing id if a sizing run has been done for this system
        if (SizingDesRunThisAirSys) {
            SysSizNum = Util::FindItemInList(
                state.dataSize->FinalSysSizing(AirLoopNum).AirPriLoopName, state.dataSize->SysSizInput, &SystemSizingInputData::AirPriLoopName);
            if (SysSizNum == 0) {
                SysSizNum = 1; // use first when none applicable
            }
        }
    }

    IsAutoSize = false;
    if (thisPIU.MaxSecAirVolFlow == AutoSize) {
        IsAutoSize = true;
    }
    if ((state.dataSize->CurZoneEqNum > 0) && (CurTermUnitSizingNum > 0)) {
        if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // Simulation continue
            if (thisPIU.MaxSecAirVolFlow > 0.0) {
                BaseSizer::reportSizerOutput(
                    state, thisPIU.UnitType, thisPIU.Name, "User-Specified Maximum Secondary Air Flow Rate [m3/s]", thisPIU.MaxSecAirVolFlow);
            }
        } else {
            CheckZoneSizing(state, thisPIU.UnitType, thisPIU.Name);
            // Autosized maximum secondary air flow for reporting
            Real64 MaxSecAirVolFlowDes = max(state.dataSize->TermUnitFinalZoneSizing(CurTermUnitSizingNum).DesCoolVolFlow,
                                             state.dataSize->TermUnitFinalZoneSizing(CurTermUnitSizingNum).DesHeatVolFlow);
            if (MaxSecAirVolFlowDes < SmallAirVolFlow) {
                MaxSecAirVolFlowDes = 0.0;
            }
            if (IsAutoSize) {
                thisPIU.MaxSecAirVolFlow = MaxSecAirVolFlowDes;
                BaseSizer::reportSizerOutput(
                    state, thisPIU.UnitType, thisPIU.Name, "Design Size Maximum Secondary Air Flow Rate [m3/s]", MaxSecAirVolFlowDes);
            } else {
                if (thisPIU.MaxSecAirVolFlow > 0.0 && MaxSecAirVolFlowDes > 0.0) {
                    // Harsized maximum secondary air flow for reporting
                    Real64 const MaxSecAirVolFlowUser = thisPIU.MaxSecAirVolFlow;
                    BaseSizer::reportSizerOutput(state,
                                                 thisPIU.UnitType,
                                                 thisPIU.Name,
                                                 "Design Size Maximum Secondary Air Flow Rate [m3/s]",
                                                 MaxSecAirVolFlowDes,
                                                 "User-Specified Maximum Secondary Air Flow Rate [m3/s]",
                                                 MaxSecAirVolFlowUser);
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        if ((std::abs(MaxSecAirVolFlowDes - MaxSecAirVolFlowUser) / MaxSecAirVolFlowUser) >
                            state.dataSize->AutoVsHardSizingThreshold) {
                            ShowMessage(state, format("SizePIU: Potential issue with equipment sizing for {} {}", thisPIU.UnitType, thisPIU.Name));
                            ShowContinueError(state, format("User-Specified Maximum Secondary Air Flow Rate of {:.5R} [m3/s]", MaxSecAirVolFlowUser));
                            ShowContinueError(
                                state, format("differs from Design Size Maximum Secondary Air Flow Rate of {:.5R} [m3/s]", MaxSecAirVolFlowDes));
                            ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                        }
                    }
                }
            }
        }
    }

    IsAutoSize = false;
    if (thisPIU.MinPriAirFlowFrac == AutoSize) {
        IsAutoSize = true;
    }
    if ((state.dataSize->CurZoneEqNum > 0) && (CurTermUnitSizingNum > 0)) {
        if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // Simulation continue
            if (thisPIU.MinPriAirFlowFrac > 0.0) {
                BaseSizer::reportSizerOutput(
                    state, thisPIU.UnitType, thisPIU.Name, "User-Specified Minimum Primary Air Flow Fraction", thisPIU.MinPriAirFlowFrac);
            }
        } else {
            CheckZoneSizing(state, thisPIU.UnitType, thisPIU.Name);
            // Autosized minimum primary air flow fraction for reporting
            Real64 MinPriAirFlowFracDes = 0.0;
            if (thisPIU.MaxPriAirVolFlow >= SmallAirVolFlow &&
                state.dataSize->TermUnitFinalZoneSizing(CurTermUnitSizingNum).MinOA >= SmallAirVolFlow) {
                MinPriAirFlowFracDes = state.dataSize->TermUnitFinalZoneSizing(CurTermUnitSizingNum).MinOA / thisPIU.MaxPriAirVolFlow;
            }
            if (SizingDesRunThisAirSys) {
                if (state.dataSize->SysSizInput(SysSizNum).SystemOAMethod == SysOAMethod::SP) { // 62.1 simplified procedure
                    if (thisPIU.MaxPriAirVolFlow > 0.0) {
                        MinPriAirFlowFracDes = 1.5 *
                                               max(state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).VozClgByZone,
                                                   state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).VozHtgByZone) /
                                               thisPIU.MaxPriAirVolFlow;

                        // adjust maximum flow rate
                        if (MinPriAirFlowFracDes > 1.0 && IsMaxPriFlowAutoSize) {
                            thisPIU.MaxPriAirVolFlow *= MinPriAirFlowFracDes;
                            MinPriAirFlowFracDes = 1.0;
                            ShowWarningError(state,
                                             format("SingleDuctSystem:SizeSys: Autosized maximum air flow rate for {} was increased to meet the zone "
                                                    "primary air flow determined according to the ASHRAE Standard 62.1 Simplified Procedure.",
                                                    thisPIU.Name));
                        } else if (MinPriAirFlowFracDes > 1.0) {
                            ShowWarningError(
                                state,
                                format("SingleDuctSystem:SizeSys: Maximum primary air flow rate for {} is potentially too low.", thisPIU.Name));
                            ShowContinueError(state,
                                              "The flow is lower than the minimum primary air flow rate calculated following the ASHRAE Standard "
                                              "62.1 Simplified Procedure:");
                            ShowContinueError(state, format(" User-specified maximum primary air flow rate: {:.3R} m3/s.", thisPIU.MaxPriAirVolFlow));
                            ShowContinueError(
                                state,
                                format(" Calculated minimum primary air flow rate: {:.3R} m3/s.", thisPIU.MaxPriAirVolFlow * MinPriAirFlowFracDes));
                            MinPriAirFlowFracDes = 1.0;
                        }
                    }
                }
            }
            if (IsAutoSize) {
                if (SizingDesRunThisAirSys) {
                    if (state.dataSize->SysSizInput(SysSizNum).SystemOAMethod == SysOAMethod::SP) {
                        state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).VpzMinByZoneSPSized = true;
                    }
                }
                thisPIU.MinPriAirFlowFrac = MinPriAirFlowFracDes;
                BaseSizer::reportSizerOutput(
                    state, thisPIU.UnitType, thisPIU.Name, "Design Size Minimum Primary Air Flow Fraction", MinPriAirFlowFracDes);
            } else {
                if (thisPIU.MinPriAirFlowFrac > 0.0 && MinPriAirFlowFracDes > 0.0) {
                    // Hardsized minimum primary air flow fraction for reporting
                    Real64 const MinPriAirFlowFracUser = thisPIU.MinPriAirFlowFrac;
                    BaseSizer::reportSizerOutput(state,
                                                 thisPIU.UnitType,
                                                 thisPIU.Name,
                                                 "Design Size Minimum Primary Air Flow Fraction",
                                                 MinPriAirFlowFracDes,
                                                 "User-Specified Minimum Primary Air Flow Fraction",
                                                 MinPriAirFlowFracUser);
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        if ((std::abs(MinPriAirFlowFracDes - MinPriAirFlowFracUser) / MinPriAirFlowFracUser) >
                            state.dataSize->AutoVsHardSizingThreshold) {
                            ShowMessage(state, format("SizePIU: Potential issue with equipment sizing for {} {}", thisPIU.UnitType, thisPIU.Name));
                            ShowContinueError(state, format("User-Specified Minimum Primary Air Flow Fraction of {:.1R}", MinPriAirFlowFracUser));
                            ShowContinueError(state,
                                              format("differs from Design Size Minimum Primary Air Flow Fraction of {:.1R}", MinPriAirFlowFracDes));
                            ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                        }
                    }
                }
            }
        }
    }

    if (CurTermUnitSizingNum > 0) {

        if (thisPIU.UnitType_Num == DataDefineEquip::ZnAirLoopEquipType::SingleDuct_SeriesPIU_Reheat) {
            TermUnitSizing(CurTermUnitSizingNum).AirVolFlow = thisPIU.MaxTotAirVolFlow;
        } else if (thisPIU.UnitType_Num == DataDefineEquip::ZnAirLoopEquipType::SingleDuct_ParallelPIU_Reheat) {
            TermUnitSizing(CurTermUnitSizingNum).AirVolFlow = thisPIU.MaxSecAirVolFlow + thisPIU.MinPriAirFlowFrac * thisPIU.MaxPriAirVolFlow;
        }
    }

    IsAutoSize = false;
    if (thisPIU.FanOnFlowFrac == AutoSize) {
        IsAutoSize = true;
    }
    if (state.dataSize->CurZoneEqNum > 0) {
        if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // Simulation continue
            if (thisPIU.FanOnFlowFrac > 0.0) {
                BaseSizer::reportSizerOutput(state, thisPIU.UnitType, thisPIU.Name, "User-Specified Fan On Flow Fraction", thisPIU.FanOnFlowFrac);
            }
        } else {
            CheckZoneSizing(state, thisPIU.UnitType, thisPIU.Name);
            // Autosized fan on flow fraction for reporting
            Real64 FanOnFlowFracDes = thisPIU.MinPriAirFlowFrac;
            if (IsAutoSize) {
                thisPIU.FanOnFlowFrac = FanOnFlowFracDes;
                BaseSizer::reportSizerOutput(state, thisPIU.UnitType, thisPIU.Name, "Design Size Fan On Flow Fraction", FanOnFlowFracDes);
            } else {
                if (thisPIU.FanOnFlowFrac > 0.0 && FanOnFlowFracDes > 0.0) {
                    // Hardsized fan on flow fraction for reporting
                    Real64 const FanOnFlowFracUser = thisPIU.FanOnFlowFrac;
                    BaseSizer::reportSizerOutput(state,
                                                 thisPIU.UnitType,
                                                 thisPIU.Name,
                                                 "Design Size Fan On Flow Fraction",
                                                 FanOnFlowFracDes,
                                                 "User-Specified Fan On Flow Fraction",
                                                 FanOnFlowFracUser);
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        if ((std::abs(FanOnFlowFracDes - FanOnFlowFracUser) / FanOnFlowFracUser) > state.dataSize->AutoVsHardSizingThreshold) {
                            ShowMessage(state, format("SizePIU: Potential issue with equipment sizing for {} {}", thisPIU.UnitType, thisPIU.Name));
                            ShowContinueError(state, format("User-Specified Fan On Flow Fraction of {:.1R}", FanOnFlowFracUser));
                            ShowContinueError(state, format("differs from Design Size Fan On Flow Fraction of {:.1R}", FanOnFlowFracDes));
                            ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                        }
                    }
                }
            }
        }
    }

    IsAutoSize = false;
    if (thisPIU.MaxVolHotWaterFlow == AutoSize) { //.or.()) THEN
        IsAutoSize = true;
    }
    if ((state.dataSize->CurZoneEqNum > 0) && (CurTermUnitSizingNum > 0)) {
        if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // Simulation continue
            if (thisPIU.MaxVolHotWaterFlow > 0.0) {
                BaseSizer::reportSizerOutput(
                    state, thisPIU.UnitType, thisPIU.Name, "User-Specified Maximum Reheat Water Flow Rate [m3/s]", thisPIU.MaxVolHotWaterFlow);
            }
        } else {
            CheckZoneSizing(state, thisPIU.UnitType, thisPIU.Name);
            if (Util::SameString(HCoilNamesUC[static_cast<int>(thisPIU.HCoilType)], "Coil:Heating:Water")) {

                int const CoilWaterInletNode = GetCoilWaterInletNode(state, "Coil:Heating:Water", thisPIU.HCoil, ErrorsFound);
                int const CoilWaterOutletNode = GetCoilWaterOutletNode(state, "Coil:Heating:Water", thisPIU.HCoil, ErrorsFound);

                // Autosized maximum hot water flow for reporting
                Real64 MaxVolHotWaterFlowDes = 0.0;

                if (IsAutoSize) {
                    int const PltSizHeatNum =
                        MyPlantSizingIndex(state, "Coil:Heating:Water", thisPIU.HCoil, CoilWaterInletNode, CoilWaterOutletNode, ErrorsFound);
                    if (PltSizHeatNum > 0) {

                        if (state.dataSize->TermUnitFinalZoneSizing(CurTermUnitSizingNum).DesHeatMassFlow >= SmallAirVolFlow) {
                            Real64 const CoilInTemp =
                                state.dataSize->TermUnitFinalZoneSizing(CurTermUnitSizingNum).DesHeatCoilInTempTU * thisPIU.MinPriAirFlowFrac +
                                state.dataSize->TermUnitFinalZoneSizing(CurTermUnitSizingNum).ZoneTempAtHeatPeak * (1.0 - thisPIU.MinPriAirFlowFrac);
                            Real64 const CoilOutTemp = state.dataSize->TermUnitFinalZoneSizing(CurTermUnitSizingNum).HeatDesTemp;
                            Real64 const CoilOutHumRat = state.dataSize->TermUnitFinalZoneSizing(CurTermUnitSizingNum).HeatDesHumRat;
                            Real64 const DesMassFlow = state.dataEnvrn->StdRhoAir * TermUnitSizing(CurTermUnitSizingNum).AirVolFlow;
                            DesCoilLoad = PsyCpAirFnW(CoilOutHumRat) * DesMassFlow * (CoilOutTemp - CoilInTemp);

                            Real64 const rho = state.dataPlnt->PlantLoop(thisPIU.HWplantLoc.loopNum)
                                                   .glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                            Real64 const Cp = state.dataPlnt->PlantLoop(thisPIU.HWplantLoc.loopNum)
                                                  .glycol->getSpecificHeat(state, Constant::HWInitConvTemp, RoutineName);

                            MaxVolHotWaterFlowDes = DesCoilLoad / (state.dataSize->PlantSizData(PltSizHeatNum).DeltaT * Cp * rho);
                        } else {
                            MaxVolHotWaterFlowDes = 0.0;
                        }
                    } else {
                        ShowSevereError(state, "Autosizing of water flow requires a heating loop Sizing:Plant object");
                        ShowContinueError(state, format("Occurs in{} Object={}", thisPIU.UnitType, thisPIU.Name));
                        ErrorsFound = true;
                    }
                    thisPIU.MaxVolHotWaterFlow = MaxVolHotWaterFlowDes;
                    BaseSizer::reportSizerOutput(
                        state, thisPIU.UnitType, thisPIU.Name, "Design Size Maximum Reheat Water Flow Rate [m3/s]", MaxVolHotWaterFlowDes);
                    BaseSizer::reportSizerOutput(state,
                                                 thisPIU.UnitType,
                                                 thisPIU.Name,
                                                 "Design Size Reheat Coil Inlet Air Temperature [C]",
                                                 state.dataSize->TermUnitFinalZoneSizing(CurTermUnitSizingNum).DesHeatCoilInTempTU);
                    BaseSizer::reportSizerOutput(state,
                                                 thisPIU.UnitType,
                                                 thisPIU.Name,
                                                 "Design Size Reheat Coil Inlet Air Humidity Ratio [kgWater/kgDryAir]",
                                                 state.dataSize->TermUnitFinalZoneSizing(CurTermUnitSizingNum).DesHeatCoilInHumRatTU);
                } else { // Hardsize with sizing data
                    if (thisPIU.MaxVolHotWaterFlow > 0.0 && MaxVolHotWaterFlowDes > 0.0) {
                        // Hardsized maximum hot water flow for reporting
                        Real64 const MaxVolHotWaterFlowUser = thisPIU.MaxVolHotWaterFlow;
                        BaseSizer::reportSizerOutput(state,
                                                     thisPIU.UnitType,
                                                     thisPIU.Name,
                                                     "Design Size Maximum Reheat Water Flow Rate [m3/s]",
                                                     MaxVolHotWaterFlowDes,
                                                     "User-Specified Maximum Reheat Water Flow Rate [m3/s]",
                                                     MaxVolHotWaterFlowUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(MaxVolHotWaterFlowDes - MaxVolHotWaterFlowUser) / MaxVolHotWaterFlowUser) >
                                state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state,
                                            format("SizePIU: Potential issue with equipment sizing for {} {}", thisPIU.UnitType, thisPIU.Name));
                                ShowContinueError(state,
                                                  format("User-Specified Maximum Reheat Water Flow Rate of {:.5R} [m3/s]", MaxVolHotWaterFlowUser));
                                ShowContinueError(
                                    state, format("differs from Design Size Maximum Reheat Water Flow Rate of {:.5R} [m3/s]", MaxVolHotWaterFlowDes));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                }
            } else {
                thisPIU.MaxVolHotWaterFlow = 0.0;
            }
        }
    }

    IsAutoSize = false;
    if (thisPIU.MaxVolHotSteamFlow == AutoSize) {
        IsAutoSize = true;
    }
    if ((state.dataSize->CurZoneEqNum > 0) && (CurTermUnitSizingNum > 0)) {
        if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // Simulation continue
            if (thisPIU.MaxVolHotWaterFlow > 0.0) {
                BaseSizer::reportSizerOutput(
                    state, thisPIU.UnitType, thisPIU.Name, "User-Specified Maximum Reheat Steam Flow Rate [m3/s]", thisPIU.MaxVolHotWaterFlow);
            }
        } else {
            if (Util::SameString(HCoilNames[static_cast<int>(thisPIU.HCoilType)], "Coil:Heating:Steam")) {

                int const CoilSteamInletNode = GetCoilSteamInletNode(state, "Coil:Heating:Steam", thisPIU.HCoil, ErrorsFound);
                int const CoilSteamOutletNode = GetCoilSteamOutletNode(state, "Coil:Heating:Steam", thisPIU.HCoil, ErrorsFound);
                Real64 MaxVolHotSteamFlowDes = 0.0; // Autosized maximum hot steam flow for reporting

                if (IsAutoSize) {
                    int const PltSizHeatNum =
                        MyPlantSizingIndex(state, "Coil:Heating:Steam", thisPIU.HCoil, CoilSteamInletNode, CoilSteamOutletNode, ErrorsFound);
                    if (PltSizHeatNum > 0) {
                        if (state.dataSize->TermUnitFinalZoneSizing(CurTermUnitSizingNum).DesHeatMassFlow >= SmallAirVolFlow) {
                            Real64 const CoilInTemp =
                                state.dataSize->TermUnitFinalZoneSizing(CurTermUnitSizingNum).DesHeatCoilInTempTU * thisPIU.MinPriAirFlowFrac +
                                state.dataSize->TermUnitFinalZoneSizing(CurTermUnitSizingNum).ZoneTempAtHeatPeak * (1.0 - thisPIU.MinPriAirFlowFrac);
                            Real64 const CoilOutTemp = state.dataSize->TermUnitFinalZoneSizing(CurTermUnitSizingNum).HeatDesTemp;
                            Real64 const CoilOutHumRat = state.dataSize->TermUnitFinalZoneSizing(CurTermUnitSizingNum).HeatDesHumRat;
                            Real64 const DesMassFlow = state.dataEnvrn->StdRhoAir * TermUnitSizing(CurTermUnitSizingNum).AirVolFlow;
                            DesCoilLoad = PsyCpAirFnW(CoilOutHumRat) * DesMassFlow * (CoilOutTemp - CoilInTemp);
                            Real64 constexpr TempSteamIn = 100.00;
                            Real64 const EnthSteamInDry = thisPIU.HCoil_fluid->getSatEnthalpy(state, TempSteamIn, 1.0, RoutineName);
                            Real64 const EnthSteamOutWet = thisPIU.HCoil_fluid->getSatEnthalpy(state, TempSteamIn, 0.0, RoutineName);
                            Real64 const LatentHeatSteam = EnthSteamInDry - EnthSteamOutWet;
                            Real64 const SteamDensity = thisPIU.HCoil_fluid->getSatDensity(state, TempSteamIn, 1.0, RoutineName);
                            Real64 const Cp =
                                Fluid::GetWater(state)->getSpecificHeat(state, state.dataSize->PlantSizData(PltSizHeatNum).ExitTemp, RoutineName);
                            MaxVolHotSteamFlowDes =
                                DesCoilLoad / (SteamDensity * (LatentHeatSteam + state.dataSize->PlantSizData(PltSizHeatNum).DeltaT * Cp));
                        } else {
                            MaxVolHotSteamFlowDes = 0.0;
                        }
                    } else {
                        ShowSevereError(state, "Autosizing of Steam flow requires a heating loop Sizing:Plant object");
                        ShowContinueError(state, format("Occurs in{} Object={}", thisPIU.UnitType, thisPIU.Name));
                        ErrorsFound = true;
                    }
                    thisPIU.MaxVolHotSteamFlow = MaxVolHotSteamFlowDes;
                    BaseSizer::reportSizerOutput(
                        state, thisPIU.UnitType, thisPIU.Name, "Design Size Maximum Reheat Steam Flow [m3/s]", MaxVolHotSteamFlowDes);
                } else {
                    if (thisPIU.MaxVolHotSteamFlow > 0.0 && MaxVolHotSteamFlowDes > 0.0) {
                        Real64 const MaxVolHotSteamFlowUser = thisPIU.MaxVolHotSteamFlow;
                        BaseSizer::reportSizerOutput(state,
                                                     thisPIU.UnitType,
                                                     thisPIU.Name,
                                                     "Design Size Maximum Reheat Steam Flow [m3/s]",
                                                     MaxVolHotSteamFlowDes,
                                                     "User-Specified Maximum Reheat Steam Flow [m3/s]",
                                                     MaxVolHotSteamFlowUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(MaxVolHotSteamFlowDes - MaxVolHotSteamFlowUser) / MaxVolHotSteamFlowUser) >
                                state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state,
                                            format("SizePIU: Potential issue with equipment sizing for {} {}", thisPIU.UnitType, thisPIU.Name));
                                ShowContinueError(state, format("User-Specified Maximum Reheat Steam Flow of {:.5R} [m3/s]", MaxVolHotSteamFlowUser));
                                ShowContinueError(
                                    state, format("differs from Design Size Maximum Reheat Steam Flow of {:.5R} [m3/s]", MaxVolHotSteamFlowDes));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                }
            } else {
                thisPIU.MaxVolHotSteamFlow = 0.0;
            }
        }
    }

    if (CurTermUnitSizingNum > 0) {
        TermUnitSizing(CurTermUnitSizingNum).MinPriFlowFrac = thisPIU.MinPriAirFlowFrac;
        TermUnitSizing(CurTermUnitSizingNum).plenumIndex = thisPIU.plenumIndex;
        TermUnitSizing(CurTermUnitSizingNum).MaxHWVolFlow = thisPIU.MaxVolHotWaterFlow;
        TermUnitSizing(CurTermUnitSizingNum).MaxSTVolFlow = thisPIU.MaxVolHotSteamFlow;
        TermUnitSizing(CurTermUnitSizingNum).DesHeatingLoad = DesCoilLoad; // coil report
        TermUnitSizing(CurTermUnitSizingNum).InducesPlenumAir = thisPIU.InducesPlenumAir;
        if (thisPIU.HCoilType == HtgCoilType::SimpleHeating) {
            SetCoilDesFlow(state,
                           HCoilNamesUC[static_cast<int>(thisPIU.HCoilType)],
                           thisPIU.HCoil,
                           TermUnitSizing(CurTermUnitSizingNum).AirVolFlow,
                           ErrorsFound);
        }
    }

    if (ErrorsFound) {
        ShowFatalError(state, "Preceding sizing errors cause program termination");
    }
}

void CalcSeriesPIU(EnergyPlusData &state,
                   int const PIUNum,             // number of the current PIU being simulated
                   int const ZoneNum,            // number of zone being served
                   int const ZoneNode,           // zone node number
                   bool const FirstHVACIteration // TRUE if 1st HVAC simulation of system timestep
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   August 2000
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Simulate a series powered induction unit; adjust its primary air flow
    // and reheat coil output to match the zone load.

    // METHODOLOGY EMPLOYED:
    // If unit is on and there is a cooling load:
    // (1) simulates mixer and fan at max secondary air flow and heating coil
    //     off. Obtains fan temperature increase.
    // (2) Calculates primary and secomdary air flow to meet zone load and
    //     resimulates mixer, fan, and (off) coil.
    // If unit is on and there is a heating load
    // (1) sets primary air flow to a minimum.
    // (2) simulates mixer and fan
    // (3) if reheat is hot water, calls ControlCompOutput to simulate hot
    //     water coil and adjust water flow to match coil output to the zone load.
    // (4) if reheat is electric or gas calls SimulateHeatingCoilComponents to
    //     simulate coil at coil output that matches the zone load

    // REFERENCES:
    // na

    // Using/Aliasing
    using namespace DataZoneEnergyDemands;
    using HeatingCoils::SimulateHeatingCoilComponents;
    using MixerComponent::SimAirMixer;
    using PlantUtilities::SetComponentFlowRate;
    using SteamCoils::SimulateSteamCoilComponents;
    using WaterCoils::SimulateWaterCoilComponents;

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:

    // SUBROUTINE PARAMETER DEFINITIONS:

    // INTERFACE BLOCK SPECIFICATIONS

    // DERIVED TYPE DEFINITIONS
    // na

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool UnitOn(true); // TRUE if unit is on
    bool PriOn(true);  // TRUE if primary air available

    Real64 QCoilReq = 0.0;     // required heating coil outlet to meet zone load
    Real64 MaxWaterFlow = 0.0; // maximum water flow for heating or cooling [kg/s]
    Real64 MinWaterFlow = 0.0; // minimum water flow for heating or cooling [kg/s]

    // initialize local variables
    auto &thisPIU = state.dataPowerInductionUnits->PIU(PIUNum);

    Real64 const PriAirMassFlowMax = state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRateMaxAvail; // max primary air mass flow rate [kg/s]
    Real64 const PriAirMassFlowMin = state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRateMinAvail; // min primary air mass flow rate [kg/s]
    Real64 const QZnReq =
        state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputRequired; // heating or cooling needed by zone [Watts]
    Real64 const QToHeatSetPt =
        state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToHeatSP; // [W]  remaining load to heating setpoint
    Real64 const CpAirZn = PsyCpAirFnW(state.dataLoopNodes->Node(ZoneNode).HumRat);          // zone air specific heat [J/kg-C]
    thisPIU.PriAirMassFlow = state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate;   // primary air mass flow rate [kg/s]
    thisPIU.SecAirMassFlow = state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate;   // secondary air mass flow rate [kg/s]
    if (thisPIU.fanControlType == FanCntrlType::VariableSpeedFan) {
        thisPIU.heatingOperatingMode = HeatOpModeType::HeaterOff;
    } else {
        thisPIU.heatingOperatingMode = HeatOpModeType::ConstantVolumeHeat;
    }
    thisPIU.coolingOperatingMode = CoolOpModeType::CoolerOff;

    // On the first HVAC iteration the system values are given to the controller, but after that
    // the demand limits are in place and there needs to be feedback to the Zone Equipment
    if (thisPIU.HotControlNode > 0) {
        if (FirstHVACIteration) {
            MaxWaterFlow = thisPIU.MaxHotWaterFlow;
            MinWaterFlow = thisPIU.MinHotWaterFlow;
        } else {
            MaxWaterFlow = state.dataLoopNodes->Node(thisPIU.HotControlNode).MassFlowRateMaxAvail;
            MinWaterFlow = state.dataLoopNodes->Node(thisPIU.HotControlNode).MassFlowRateMinAvail;
        }
    }
    if (thisPIU.availSched->getCurrentVal() <= 0.0) {
        UnitOn = false;
    }
    if ((thisPIU.fanAvailSched->getCurrentVal() <= 0.0 || state.dataHVACGlobal->TurnFansOff) && !state.dataHVACGlobal->TurnFansOn) {
        UnitOn = false;
    }
    if (thisPIU.PriAirMassFlow <= SmallMassFlow || PriAirMassFlowMax <= SmallMassFlow) {
        PriOn = false;
    }
    // Set the mass flow rates
    if (UnitOn) {
        // unit is on
        if (!PriOn) {
            // no primary air flow
            thisPIU.PriAirMassFlow = 0.0;
            // PIU fan off if there is no heating load, also reset fan flag if fan should be off
            if (QZnReq <= SmallLoad) {
                thisPIU.SecAirMassFlow = 0.0;
                state.dataHVACGlobal->TurnFansOn = false;
            } else {
                if (thisPIU.fanControlType == FanCntrlType::VariableSpeedFan &&
                    thisPIU.heatingControlType == HeatCntrlBehaviorType::StagedHeaterBehavior) {
                    CalcVariableSpeedPIUStagedHeatingBehavior(state, PIUNum, ZoneNode, QZnReq, PriOn, thisPIU.PriAirMassFlow);
                } else if (thisPIU.fanControlType == FanCntrlType::VariableSpeedFan &&
                           thisPIU.heatingControlType == HeatCntrlBehaviorType::ModulatedHeaterBehavior) {
                    CalcVariableSpeedPIUModulatedHeatingBehavior(state, PIUNum, ZoneNode, QZnReq, PriOn, thisPIU.PriAirMassFlow);
                } else if (thisPIU.fanControlType == FanCntrlType::ConstantSpeedFan) {
                    thisPIU.heatingOperatingMode = HeatOpModeType::ConstantVolumeHeat;
                    thisPIU.SecAirMassFlow = thisPIU.MaxTotAirMassFlow;
                }
            }
        } else if (state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum) || std::abs(QZnReq) < SmallLoad) {
            // in deadband or very small load: set primary air flow to the minimum
            thisPIU.PriAirMassFlow = PriAirMassFlowMin;
            if (thisPIU.fanControlType == FanCntrlType::ConstantSpeedFan) {
                thisPIU.heatingOperatingMode = HeatOpModeType::ConstantVolumeHeat;
                thisPIU.SecAirMassFlow = max(0.0, thisPIU.MaxTotAirMassFlow - thisPIU.PriAirMassFlow);
            } else if (thisPIU.fanControlType == FanCntrlType::VariableSpeedFan) {
                thisPIU.SecAirMassFlow = max(0.0, thisPIU.MinTotAirMassFlow - thisPIU.PriAirMassFlow);
            }
        } else if (QZnReq > SmallLoad) {
            // heating: set primary air flow to the minimum
            thisPIU.PriAirMassFlow = PriAirMassFlowMin;
            // determine secondary flow rate
            if (thisPIU.fanControlType == FanCntrlType::VariableSpeedFan &&
                thisPIU.heatingControlType == HeatCntrlBehaviorType::StagedHeaterBehavior) {
                CalcVariableSpeedPIUStagedHeatingBehavior(state, PIUNum, ZoneNode, QZnReq, PriOn, thisPIU.PriAirMassFlow);
            } else if (thisPIU.fanControlType == FanCntrlType::VariableSpeedFan &&
                       thisPIU.heatingControlType == HeatCntrlBehaviorType::ModulatedHeaterBehavior) {
                CalcVariableSpeedPIUModulatedHeatingBehavior(state, PIUNum, ZoneNode, QZnReq, PriOn, thisPIU.PriAirMassFlow);
            } else if (thisPIU.fanControlType == FanCntrlType::ConstantSpeedFan) {
                thisPIU.heatingOperatingMode = HeatOpModeType::ConstantVolumeHeat;
                thisPIU.SecAirMassFlow = max(0.0, thisPIU.MaxTotAirMassFlow - thisPIU.PriAirMassFlow);
            }
        } else {
            if (thisPIU.fanControlType == FanCntrlType::ConstantSpeedFan) {
                // cooling: set the primary air flow rate to meet the load.
                // First calculate the fan temperature rise
                // use only secondary air for this calculation
                state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate = 0.0;
                state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = thisPIU.MaxTotAirMassFlow;
                SimAirMixer(state, thisPIU.MixerName, thisPIU.Mixer_Num); // fire the mixer
                state.dataFans->fans(thisPIU.Fan_Index)->simulate(state, FirstHVACIteration, _, _);

                // fan temperature rise [C]
                Real64 const FanDeltaTemp =
                    state.dataLoopNodes->Node(thisPIU.HCoilInAirNode).Temp - state.dataLoopNodes->Node(thisPIU.SecAirInNode).Temp;

                // using the required zone load, calculate the air temperature needed to meet the load
                Real64 const OutletTempNeeded = state.dataLoopNodes->Node(ZoneNode).Temp + QZnReq / (thisPIU.MaxTotAirMassFlow * CpAirZn);

                // mixer outlet temperature needed to meet cooling load
                Real64 const MixTempNeeded = OutletTempNeeded - FanDeltaTemp;

                if (MixTempNeeded <= state.dataLoopNodes->Node(thisPIU.PriAirInNode).Temp) { //
                    thisPIU.PriAirMassFlow = PriAirMassFlowMax;
                } else if (MixTempNeeded >= state.dataLoopNodes->Node(thisPIU.PriAirInNode).Temp &&
                           MixTempNeeded >= state.dataLoopNodes->Node(thisPIU.SecAirInNode).Temp) {
                    thisPIU.PriAirMassFlow = PriAirMassFlowMin;
                } else {
                    thisPIU.PriAirMassFlow =
                        thisPIU.MaxTotAirMassFlow * (state.dataLoopNodes->Node(thisPIU.SecAirInNode).Temp - MixTempNeeded) /
                        max(SmallTempDiff,
                            state.dataLoopNodes->Node(thisPIU.SecAirInNode).Temp - state.dataLoopNodes->Node(thisPIU.PriAirInNode).Temp);
                    thisPIU.PriAirMassFlow = min(max(thisPIU.PriAirMassFlow, PriAirMassFlowMin), PriAirMassFlowMax);
                }
                thisPIU.SecAirMassFlow = max(0.0, thisPIU.MaxTotAirMassFlow - thisPIU.PriAirMassFlow);
                if (QZnReq < 0) {
                    thisPIU.coolingOperatingMode = CoolOpModeType::ConstantVolumeCool;
                }
            } else if (thisPIU.fanControlType == FanCntrlType::VariableSpeedFan) {
                CalcVariableSpeedPIUCoolingBehavior(state, PIUNum, ZoneNode, QZnReq, QToHeatSetPt, PriAirMassFlowMin, PriAirMassFlowMax);
            }
        }
    } else {
        // unit is off ; no flow
        thisPIU.PriAirMassFlow = 0.0;
        thisPIU.SecAirMassFlow = 0.0;
    }
    // set inlet node flowrates
    state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate = thisPIU.PriAirMassFlow;
    state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = thisPIU.SecAirMassFlow;
    if (PriAirMassFlowMax == 0) {
        thisPIU.PriDamperPosition = 0;
    } else {
        thisPIU.PriDamperPosition = thisPIU.PriAirMassFlow / PriAirMassFlowMax;
    }

    // now that inlet airflows have been set, the terminal components can be simulated.
    // fire the mixer
    SimAirMixer(state, thisPIU.MixerName, thisPIU.Mixer_Num);

    // fire the fan
    if (thisPIU.fanType == HVAC::FanType::SystemModel) {
        if (thisPIU.fanControlType == FanCntrlType::VariableSpeedFan) {
            // calculate fan speed ratio
            Real64 fanFlowRatio(1.0);
            if (thisPIU.MaxTotAirMassFlow > 0.0) {
                fanFlowRatio = (thisPIU.PriAirMassFlow + thisPIU.SecAirMassFlow) / thisPIU.MaxTotAirMassFlow;
            }
            state.dataFans->fans(thisPIU.Fan_Index)->simulate(state, FirstHVACIteration, fanFlowRatio, _);
        } else {
            state.dataFans->fans(thisPIU.Fan_Index)->simulate(state, FirstHVACIteration, _, _);
        }
    } else if (thisPIU.fanType == HVAC::FanType::Constant) {
        state.dataFans->fans(thisPIU.Fan_Index)->simulate(state, FirstHVACIteration, _, _);
    }

    // the heating load seen by the reheat coil [W]
    Real64 QActualHeating = QToHeatSetPt - state.dataLoopNodes->Node(thisPIU.HCoilInAirNode).MassFlowRate * CpAirZn *
                                               (state.dataLoopNodes->Node(thisPIU.HCoilInAirNode).Temp - state.dataLoopNodes->Node(ZoneNode).Temp);

    // check if heating coil is off
    if (((!UnitOn) || (QActualHeating < SmallLoad) || (state.dataHeatBalFanSys->TempControlType(ZoneNum) == HVAC::SetptType::SingleCool) ||
         (thisPIU.PriAirMassFlow > PriAirMassFlowMin)) &&
        (thisPIU.heatingOperatingMode != HeatOpModeType::StagedHeatFirstStage)) { // reheat is off during the first stage of heating
        thisPIU.heatingOperatingMode = HeatOpModeType::HeaterOff;
    }

    // determine what is required of heater for current operating stage
    if (thisPIU.heatingOperatingMode == HeatOpModeType::HeaterOff) {
        QCoilReq = 0.0;
    } else if (thisPIU.heatingOperatingMode == HeatOpModeType::StagedHeatFirstStage) {
        QCoilReq = 0.0;
    } else if (thisPIU.heatingOperatingMode == HeatOpModeType::ConstantVolumeHeat) {
        QCoilReq = QActualHeating;
    } else if (thisPIU.heatingOperatingMode == HeatOpModeType::StagedHeatSecondStage) {
        QCoilReq = QActualHeating;
    } else if (thisPIU.heatingOperatingMode == HeatOpModeType::ModulatedHeatFirstStage) {
        QCoilReq = QActualHeating;
    } else if (thisPIU.heatingOperatingMode == HeatOpModeType::ModulatedHeatSecondStage) {
        // find heater power to deliver design discharge air temperature
        Real64 targetDATEnthalpy = Psychrometrics::PsyHFnTdbW(thisPIU.designHeatingDAT, state.dataLoopNodes->Node(ZoneNode).HumRat);
        Real64 mixAirEnthalpy =
            Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(thisPIU.HCoilInAirNode).Temp, state.dataLoopNodes->Node(ZoneNode).HumRat);
        QCoilReq = state.dataLoopNodes->Node(thisPIU.HCoilInAirNode).MassFlowRate * (targetDATEnthalpy - mixAirEnthalpy);
    } else if (thisPIU.heatingOperatingMode == HeatOpModeType::ModulatedHeatThirdStage) {
        // find heater power to deliver maximum discharge air temperature
        Real64 HiLimitDATEnthalpy = Psychrometrics::PsyHFnTdbW(thisPIU.highLimitDAT, state.dataLoopNodes->Node(ZoneNode).HumRat);
        Real64 mixAirEnthalpy =
            Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(thisPIU.HCoilInAirNode).Temp, state.dataLoopNodes->Node(ZoneNode).HumRat);
        Real64 QcoilLimit = state.dataLoopNodes->Node(thisPIU.HCoilInAirNode).MassFlowRate * (HiLimitDATEnthalpy - mixAirEnthalpy);
        if (QcoilLimit < QActualHeating) { // if requried power is too high use limit of coil discharge
            QCoilReq = QcoilLimit;
        } else {
            QCoilReq = QActualHeating;
        }
    } else {
        ShowSevereError(state, "Incorrect series PIU heating operation.");
        ShowFatalError(state, format("Series PIU control failed for {}:{}", thisPIU.UnitType, thisPIU.Name));
    }
    if ((QCoilReq < SmallLoad) &&
        (thisPIU.heatingOperatingMode != HeatOpModeType::StagedHeatFirstStage)) { // reheat is off during the first stage of heating
        thisPIU.heatingOperatingMode = HeatOpModeType::HeaterOff;
        QCoilReq = 0.0;
    }

    // fire the heating coil
    switch (thisPIU.HCoilType) {
    case HtgCoilType::SimpleHeating: { // COIL:WATER:SIMPLEHEATING
        if ((thisPIU.heatingOperatingMode == HeatOpModeType::HeaterOff) || (thisPIU.heatingOperatingMode == HeatOpModeType::StagedHeatFirstStage)) {
            // call the reheat coil with the NO FLOW condition
            Real64 mdot = 0.0;
            SetComponentFlowRate(state, mdot, thisPIU.HotControlNode, thisPIU.HotCoilOutNodeNum, thisPIU.HWplantLoc);

            SimulateWaterCoilComponents(state, thisPIU.HCoil, FirstHVACIteration, thisPIU.HCoil_Index);
        } else {
            // control water flow to obtain output matching QZnReq
            ControlCompOutput(state,
                              thisPIU.HCoil,
                              thisPIU.UnitType,
                              thisPIU.HCoil_Index,
                              FirstHVACIteration,
                              QCoilReq,
                              thisPIU.HotControlNode,
                              MaxWaterFlow,
                              MinWaterFlow,
                              thisPIU.HotControlOffset,
                              thisPIU.ControlCompTypeNum,
                              thisPIU.CompErrIndex,
                              thisPIU.HCoilInAirNode,
                              thisPIU.OutAirNode,
                              _,
                              _,
                              _,
                              thisPIU.HWplantLoc);
        }
        break;
    }
    case HtgCoilType::SteamAirHeating: { // COIL:STEAM:AIRHEATING
        SimulateSteamCoilComponents(state, thisPIU.HCoil, FirstHVACIteration, thisPIU.HCoil_Index, QCoilReq);
        break;
    }
    case HtgCoilType::Electric: { // COIL:ELECTRIC:HEATING
        SimulateHeatingCoilComponents(state, thisPIU.HCoil, FirstHVACIteration, QCoilReq, thisPIU.HCoil_Index);
        break;
    }
    case HtgCoilType::Gas: { // COIL:GAS:HEATING
        SimulateHeatingCoilComponents(state, thisPIU.HCoil, FirstHVACIteration, QCoilReq, thisPIU.HCoil_Index);
        break;
    }
    default:
        break;
    }

    // Power supplied
    Real64 PowerMet = state.dataLoopNodes->Node(thisPIU.OutAirNode).MassFlowRate *
                      (PsyHFnTdbW(state.dataLoopNodes->Node(thisPIU.OutAirNode).Temp, state.dataLoopNodes->Node(ZoneNode).HumRat) -
                       PsyHFnTdbW(state.dataLoopNodes->Node(ZoneNode).Temp, state.dataLoopNodes->Node(ZoneNode).HumRat));
    thisPIU.HeatingRate = max(0.0, PowerMet);
    thisPIU.SensCoolRate = std::abs(min(DataPrecisionGlobals::constant_zero, PowerMet));
    thisPIU.TotMassFlowRate = state.dataLoopNodes->Node(thisPIU.OutAirNode).MassFlowRate;
    thisPIU.SecMassFlowRate = state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate;
    thisPIU.PriMassFlowRate = state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate;
    thisPIU.DischargeAirTemp = state.dataLoopNodes->Node(thisPIU.OutAirNode).Temp;
    if (state.dataLoopNodes->Node(thisPIU.OutAirNode).MassFlowRate == 0.0) {
        state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate = 0.0;
        state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = 0.0;
    }
    if (thisPIU.InducesPlenumAir) {
        state.dataHVACGlobal->PlenumInducedMassFlow = state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate;
    } else {
        state.dataHVACGlobal->PlenumInducedMassFlow = 0.0;
    }
    state.dataDefineEquipment->AirDistUnit(thisPIU.ADUNum).MassFlowRatePlenInd = state.dataHVACGlobal->PlenumInducedMassFlow;
    state.dataLoopNodes->Node(thisPIU.OutAirNode).MassFlowRateMax = thisPIU.MaxTotAirMassFlow;

    ReportCurOperatingControlStage(state, PIUNum, UnitOn, thisPIU.heatingOperatingMode, thisPIU.coolingOperatingMode);
}

void CalcParallelPIU(EnergyPlusData &state,
                     int const PIUNum,             // number of the current PIU being simulated
                     int const ZoneNum,            // number of zone being served
                     int const ZoneNode,           // zone node number
                     bool const FirstHVACIteration // TRUE if 1st HVAC simulation of system timestep
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   August 2000
    //       MODIFIED       September 2016, March 2017

    // PURPOSE OF THIS SUBROUTINE:
    // Simulate a parallel powered induction unit; adjust its primary air flow
    // and reheat coil output to match the zone load.

    // METHODOLOGY EMPLOYED:
    // If unit is on and there is a cooling load:
    // (1) simulate fan at max secondary air flow and heating coil
    //     off. Obtains fan temperature increase.
    // (2) Calculates primary and secomdary air flow to meet zone load.
    //     (a) Assume fan is off and calculate primary air flow to meet cooling load.
    //     (b1) If calculated primary air flow is above the fan turn on ratio, fan is off.
    //         Otherwise fan is on; calculate mixed secondary and primary air flow that
    //         will meet the zone load
    //     (b2) If the fan turn on ratio is zero, then the fan is on only if reheat is needed.
    //  (3) Simulate fan, mixer, and (off) heating coil to obtain zone inlet conditions.
    // If unit is on and there is a heating load
    // (1) sets primary air flow to a minimum.
    // (2) simulates fan and mixer
    // (3) if reheat is hot water, calls ControlCompOutput to simulate hot
    //     water coil and adjust water flow to match coil output to the zone load.
    // (4) if reheat is electric or gas calls SimulateHeatingCoilComponents to
    //     simulate coil at coil output that matches the zone load

    using namespace DataZoneEnergyDemands;
    using HeatingCoils::SimulateHeatingCoilComponents;
    using MixerComponent::SimAirMixer;
    using PlantUtilities::SetComponentFlowRate;
    using SteamCoils::SimulateSteamCoilComponents;
    using WaterCoils::SimulateWaterCoilComponents;

    bool UnitOn(true); // TRUE if unit is on
    bool PriOn(true);  // TRUE if primary air available

    Real64 QCoilReq = 0.0;     // required heating coil outlet to meet zone load
    Real64 MaxWaterFlow = 0.0; // maximum water flow for heating or cooling [kg/s]
    Real64 MinWaterFlow = 0.0; // minimum water flow for heating or cooling [kg/s]

    // initialize local variables
    auto &thisPIU = state.dataPowerInductionUnits->PIU(PIUNum);

    Real64 const PriAirMassFlowMax = state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRateMaxAvail; // max primary air mass flow rate [kg/s]
    Real64 const PriAirMassFlowMin = state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRateMinAvail; // min primary air mass flow rate [kg/s]
    thisPIU.PriAirMassFlow = state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate;                 // primary air mass flow rate [kg/s]
    thisPIU.SecAirMassFlow = state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate;                 // secondary air mass flow rate [kg/s]
    Real64 const QZnReq =
        state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputRequired; // heating or cooling needed by zone [Watts]
    Real64 const QToHeatSetPt =
        state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToHeatSP; // [W]  remaining load to heating setpoint
    Real64 const CpAirZn = PsyCpAirFnW(state.dataLoopNodes->Node(ZoneNode).HumRat);          // zone air specific heat [J/kg-C]
    thisPIU.heatingOperatingMode = HeatOpModeType::HeaterOff;
    thisPIU.coolingOperatingMode = CoolOpModeType::CoolerOff;

    // On the first HVAC iteration the system values are given to the controller, but after that
    // the demand limits are in place and there needs to be feedback to the Zone Equipment
    if (thisPIU.HotControlNode > 0) {
        if (FirstHVACIteration) {
            MaxWaterFlow = thisPIU.MaxHotWaterFlow;
            MinWaterFlow = thisPIU.MinHotWaterFlow;
        } else {
            MaxWaterFlow = state.dataLoopNodes->Node(thisPIU.HotControlNode).MassFlowRateMaxAvail;
            MinWaterFlow = state.dataLoopNodes->Node(thisPIU.HotControlNode).MassFlowRateMinAvail;
        }
    }
    if (thisPIU.availSched->getCurrentVal() <= 0.0) {
        UnitOn = false;
    }
    if (thisPIU.PriAirMassFlow <= SmallMassFlow || PriAirMassFlowMax <= SmallMassFlow) {
        PriOn = false;
    }
    // Set the mass flow rates
    if (UnitOn) {
        // unit is on
        // Calculate if reheat is needed
        bool ReheatRequired = false;
        Real64 const qMinPrimary =
            PriAirMassFlowMin *
            (CpAirZn * min(-SmallTempDiff, (state.dataLoopNodes->Node(thisPIU.PriAirInNode).Temp - state.dataLoopNodes->Node(ZoneNode).Temp)));
        if (qMinPrimary < QToHeatSetPt) {
            ReheatRequired = true;
        }

        if (!PriOn) {
            // no primary air flow
            thisPIU.PriAirMassFlow = 0.0;
            // PIU fan off if there is no heating load, also reset fan flag if fan should be off
            if (QZnReq <= SmallLoad) {
                thisPIU.SecAirMassFlow = 0.0;
                state.dataHVACGlobal->TurnFansOn = false;
            } else {
                if (thisPIU.fanControlType == FanCntrlType::VariableSpeedFan &&
                    thisPIU.heatingControlType == HeatCntrlBehaviorType::StagedHeaterBehavior) {
                    CalcVariableSpeedPIUStagedHeatingBehavior(state, PIUNum, ZoneNode, QZnReq, PriOn, thisPIU.PriAirMassFlow);
                } else if (thisPIU.fanControlType == FanCntrlType::VariableSpeedFan &&
                           thisPIU.heatingControlType == HeatCntrlBehaviorType::ModulatedHeaterBehavior) {
                    CalcVariableSpeedPIUModulatedHeatingBehavior(state, PIUNum, ZoneNode, QZnReq, PriOn, thisPIU.PriAirMassFlow);
                } else if (thisPIU.fanControlType == FanCntrlType::ConstantSpeedFan) {
                    thisPIU.heatingOperatingMode = HeatOpModeType::ConstantVolumeHeat;
                    thisPIU.SecAirMassFlow = thisPIU.MaxSecAirMassFlow;
                }
            }
        } else if (state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum) || std::abs(QZnReq) < SmallLoad) {
            // in deadband or very small load: set primary air flow to the minimum
            thisPIU.PriAirMassFlow = PriAirMassFlowMin;
            // PIU fan off if reheat is not needed, also reset fan flag if fan should be off
            if (ReheatRequired) {
                state.dataHVACGlobal->TurnFansOn = true;
                if (thisPIU.fanControlType == FanCntrlType::ConstantSpeedFan) {
                    thisPIU.heatingOperatingMode = HeatOpModeType::ConstantVolumeHeat;
                    thisPIU.SecAirMassFlow = thisPIU.MaxSecAirMassFlow;
                } else if (thisPIU.fanControlType == FanCntrlType::VariableSpeedFan) {
                    if (thisPIU.heatingControlType == HeatCntrlBehaviorType::StagedHeaterBehavior) {
                        thisPIU.heatingOperatingMode = HeatOpModeType::StagedHeatFirstStage;
                    } else {
                        thisPIU.heatingOperatingMode = HeatOpModeType::ModulatedHeatFirstStage;
                    }
                    thisPIU.SecAirMassFlow = thisPIU.MinSecAirMassFlow;
                }
            } else {
                thisPIU.SecAirMassFlow = 0.0;
                state.dataHVACGlobal->TurnFansOn = false;
                thisPIU.heatingOperatingMode = HeatOpModeType::HeaterOff;
            }
        } else if (QZnReq > SmallLoad) {
            // heating
            // set primary air flow to the minimum
            thisPIU.PriAirMassFlow = PriAirMassFlowMin;
            // determine secondary flow rate
            if (thisPIU.fanControlType == FanCntrlType::VariableSpeedFan &&
                thisPIU.heatingControlType == HeatCntrlBehaviorType::StagedHeaterBehavior) {
                CalcVariableSpeedPIUStagedHeatingBehavior(state, PIUNum, ZoneNode, QZnReq, PriOn, thisPIU.PriAirMassFlow);
            } else if (thisPIU.fanControlType == FanCntrlType::VariableSpeedFan &&
                       thisPIU.heatingControlType == HeatCntrlBehaviorType::ModulatedHeaterBehavior) {
                CalcVariableSpeedPIUModulatedHeatingBehavior(state, PIUNum, ZoneNode, QZnReq, PriOn, thisPIU.PriAirMassFlow);
            } else if (thisPIU.fanControlType == FanCntrlType::ConstantSpeedFan) {
                thisPIU.heatingOperatingMode = HeatOpModeType::ConstantVolumeHeat;
                thisPIU.SecAirMassFlow = thisPIU.MaxSecAirMassFlow;
            }
        } else {
            // cooling: set the primary air flow rate to meet the load.
            // First calculate the fan temperature rise
            state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = thisPIU.MaxSecAirMassFlow;
            state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRateMaxAvail = thisPIU.MaxSecAirMassFlow;
            state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate = 0.0;

            state.dataFans->fans(thisPIU.Fan_Index)->simulate(state, FirstHVACIteration, _, _);

            SimAirMixer(state, thisPIU.MixerName, thisPIU.Mixer_Num); // fire the mixer
            // fan temperature rise [C]
            Real64 const FanDeltaTemp = state.dataLoopNodes->Node(thisPIU.HCoilInAirNode).Temp - state.dataLoopNodes->Node(thisPIU.SecAirInNode).Temp;
            // Assuming the fan is off, calculate the primary air flow needed to meet the zone cooling demand.
            thisPIU.PriAirMassFlow =
                QZnReq /
                (CpAirZn * min(-SmallTempDiff, (state.dataLoopNodes->Node(thisPIU.PriAirInNode).Temp - state.dataLoopNodes->Node(ZoneNode).Temp)));
            thisPIU.PriAirMassFlow = min(max(thisPIU.PriAirMassFlow, PriAirMassFlowMin), PriAirMassFlowMax);
            // check for fan on or off
            if ((thisPIU.PriAirMassFlow > thisPIU.FanOnAirMassFlow) && !ReheatRequired) {
                thisPIU.SecAirMassFlow = 0.0; // Fan is off unless reheat is required; no secondary air; also reset fan flag
                state.dataHVACGlobal->TurnFansOn = false;
            } else {
                // fan is on; recalc primary air flow
                thisPIU.PriAirMassFlow =
                    (QZnReq - CpAirZn * thisPIU.SecAirMassFlow *
                                  (state.dataLoopNodes->Node(thisPIU.SecAirInNode).Temp + FanDeltaTemp - state.dataLoopNodes->Node(ZoneNode).Temp)) /
                    (CpAirZn *
                     min(-SmallTempDiff, (state.dataLoopNodes->Node(thisPIU.PriAirInNode).Temp - state.dataLoopNodes->Node(ZoneNode).Temp)));
                thisPIU.PriAirMassFlow = min(max(thisPIU.PriAirMassFlow, PriAirMassFlowMin), PriAirMassFlowMax);
                thisPIU.SecAirMassFlow = thisPIU.MaxSecAirMassFlow;
            }
            if (QZnReq < 0) {
                if (thisPIU.fanControlType == FanCntrlType::VariableSpeedFan) {
                    if (thisPIU.PriAirMassFlow == PriAirMassFlowMax) {
                        thisPIU.coolingOperatingMode = CoolOpModeType::CoolSecondStage;
                    } else {
                        thisPIU.coolingOperatingMode = CoolOpModeType::CoolFirstStage;
                    }
                } else {
                    thisPIU.coolingOperatingMode = CoolOpModeType::ConstantVolumeCool;
                }
            }
        }
    } else {
        // unit is off; no flow
        thisPIU.PriAirMassFlow = 0.0;
        thisPIU.SecAirMassFlow = 0.0;
    }
    // set inlet node flowrates
    state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate = thisPIU.PriAirMassFlow;
    state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = thisPIU.SecAirMassFlow;
    state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRateMaxAvail = thisPIU.SecAirMassFlow;
    if (PriAirMassFlowMax == 0) {
        thisPIU.PriDamperPosition = 0;
    } else {
        thisPIU.PriDamperPosition = thisPIU.PriAirMassFlow / PriAirMassFlowMax;
    }

    // now that inlet airflows have been set, the terminal box components can be simulated.
    // fire the fan
    if (thisPIU.fanType == HVAC::FanType::SystemModel) {
        if (thisPIU.fanControlType == FanCntrlType::VariableSpeedFan) {
            // calculate fan speed ratio
            Real64 fanFlowRatio(1.0);
            if (thisPIU.MaxSecAirMassFlow > 0.0) {
                fanFlowRatio = thisPIU.SecAirMassFlow / thisPIU.MaxSecAirMassFlow;
            }
            state.dataFans->fans(thisPIU.Fan_Index)->simulate(state, FirstHVACIteration, fanFlowRatio, _);
        } else {
            state.dataFans->fans(thisPIU.Fan_Index)->simulate(state, FirstHVACIteration, _, _);
        }
    } else if (thisPIU.fanType == HVAC::FanType::Constant) {
        state.dataFans->fans(thisPIU.Fan_Index)->simulate(state, FirstHVACIteration, _, _);
    }

    // fire the mixer
    SimAirMixer(state, thisPIU.MixerName, thisPIU.Mixer_Num);

    // the heating load seen by the reheat coil [W]
    Real64 QActualHeating = QToHeatSetPt - state.dataLoopNodes->Node(thisPIU.HCoilInAirNode).MassFlowRate * CpAirZn *
                                               (state.dataLoopNodes->Node(thisPIU.HCoilInAirNode).Temp - state.dataLoopNodes->Node(ZoneNode).Temp);

    // check if heating coil is off
    if (((!UnitOn) || (QActualHeating < SmallLoad) || (state.dataHeatBalFanSys->TempControlType(ZoneNum) == HVAC::SetptType::SingleCool) ||
         (thisPIU.PriAirMassFlow > PriAirMassFlowMin)) &&
        (thisPIU.heatingOperatingMode != HeatOpModeType::StagedHeatFirstStage)) { // reheat is off during the first stage of heating
        thisPIU.heatingOperatingMode = HeatOpModeType::HeaterOff;
    }

    // determine what is required of heater for current operating stage
    if (thisPIU.heatingOperatingMode == HeatOpModeType::HeaterOff) {
        QCoilReq = 0.0;
    } else if (thisPIU.heatingOperatingMode == HeatOpModeType::StagedHeatFirstStage) {
        QCoilReq = 0.0;
    } else if (thisPIU.heatingOperatingMode == HeatOpModeType::ConstantVolumeHeat) {
        QCoilReq = QActualHeating;
    } else if (thisPIU.heatingOperatingMode == HeatOpModeType::StagedHeatSecondStage) {
        QCoilReq = QActualHeating;
    } else if (thisPIU.heatingOperatingMode == HeatOpModeType::ModulatedHeatFirstStage) {
        QCoilReq = QActualHeating;
    } else if (thisPIU.heatingOperatingMode == HeatOpModeType::ModulatedHeatSecondStage) {
        // find heater power to deliver design discharge air temperature
        Real64 targetDATEnthalpy = Psychrometrics::PsyHFnTdbW(thisPIU.designHeatingDAT, state.dataLoopNodes->Node(ZoneNode).HumRat);
        Real64 mixAirEnthalpy =
            Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(thisPIU.HCoilInAirNode).Temp, state.dataLoopNodes->Node(ZoneNode).HumRat);
        QCoilReq = state.dataLoopNodes->Node(thisPIU.HCoilInAirNode).MassFlowRate * (targetDATEnthalpy - mixAirEnthalpy);
    } else if (thisPIU.heatingOperatingMode == HeatOpModeType::ModulatedHeatThirdStage) {
        // find heater power to deliver maximum discharge air temperature
        Real64 HiLimitDATEnthalpy = Psychrometrics::PsyHFnTdbW(thisPIU.highLimitDAT, state.dataLoopNodes->Node(ZoneNode).HumRat);
        Real64 mixAirEnthalpy =
            Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(thisPIU.HCoilInAirNode).Temp, state.dataLoopNodes->Node(ZoneNode).HumRat);
        Real64 QcoilLimit = state.dataLoopNodes->Node(thisPIU.HCoilInAirNode).MassFlowRate * (HiLimitDATEnthalpy - mixAirEnthalpy);
        if (QcoilLimit < QActualHeating) { // if requried power is too high use limit of coil discharge
            QCoilReq = QcoilLimit;
        } else {
            QCoilReq = QActualHeating;
        }
    } else {
        ShowSevereError(state, "Incorrect parallel PIU heating operation.");
        ShowFatalError(state, format("Parallel PIU control failed for {}:{}", thisPIU.UnitType, thisPIU.Name));
    }
    if ((QCoilReq < SmallLoad) &&
        (thisPIU.heatingOperatingMode != HeatOpModeType::StagedHeatFirstStage)) { // reheat is off during the first stage of heating
        thisPIU.heatingOperatingMode = HeatOpModeType::HeaterOff;
        QCoilReq = 0.0;
    }

    // fire the heating coil
    switch (thisPIU.HCoilType) {
    case HtgCoilType::SimpleHeating: { // COIL:WATER:SIMPLEHEATING
        if ((thisPIU.heatingOperatingMode == HeatOpModeType::HeaterOff) || (thisPIU.heatingOperatingMode == HeatOpModeType::StagedHeatFirstStage)) {
            // call the reheat coil with the NO FLOW condition
            Real64 mdot = 0.0;
            SetComponentFlowRate(state, mdot, thisPIU.HotControlNode, thisPIU.HotCoilOutNodeNum, thisPIU.HWplantLoc);
            SimulateWaterCoilComponents(state, thisPIU.HCoil, FirstHVACIteration, thisPIU.HCoil_Index);
        } else {
            // control water flow to obtain output matching QZnReq
            ControlCompOutput(state,
                              thisPIU.HCoil,
                              thisPIU.UnitType,
                              thisPIU.HCoil_Index,
                              FirstHVACIteration,
                              QCoilReq,
                              thisPIU.HotControlNode,
                              MaxWaterFlow,
                              MinWaterFlow,
                              thisPIU.HotControlOffset,
                              thisPIU.ControlCompTypeNum,
                              thisPIU.CompErrIndex,
                              thisPIU.HCoilInAirNode,
                              thisPIU.OutAirNode,
                              _,
                              _,
                              _,
                              thisPIU.HWplantLoc);
        }
        break;
    }
    case HtgCoilType::SteamAirHeating: { // COIL:STEAM:AIRHEATING
        SimulateSteamCoilComponents(state, thisPIU.HCoil, FirstHVACIteration, thisPIU.HCoil_Index, QCoilReq);
        break;
    }
    case HtgCoilType::Electric: { // COIL:ELECTRIC:HEATING
        SimulateHeatingCoilComponents(state, thisPIU.HCoil, FirstHVACIteration, QCoilReq, thisPIU.HCoil_Index);
        break;
    }
    case HtgCoilType::Gas: { // COIL:GAS:HEATING
        SimulateHeatingCoilComponents(state, thisPIU.HCoil, FirstHVACIteration, QCoilReq, thisPIU.HCoil_Index);
        break;
    }
    default:
        break;
    }
    // Power supplied
    Real64 PowerMet = state.dataLoopNodes->Node(thisPIU.OutAirNode).MassFlowRate *
                      (PsyHFnTdbW(state.dataLoopNodes->Node(thisPIU.OutAirNode).Temp, state.dataLoopNodes->Node(ZoneNode).HumRat) -
                       PsyHFnTdbW(state.dataLoopNodes->Node(ZoneNode).Temp, state.dataLoopNodes->Node(ZoneNode).HumRat));
    thisPIU.HeatingRate = max(0.0, PowerMet);
    thisPIU.SensCoolRate = std::abs(min(DataPrecisionGlobals::constant_zero, PowerMet));
    thisPIU.TotMassFlowRate = state.dataLoopNodes->Node(thisPIU.OutAirNode).MassFlowRate;
    thisPIU.SecMassFlowRate = state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate;
    thisPIU.PriMassFlowRate = state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate;
    thisPIU.DischargeAirTemp = state.dataLoopNodes->Node(thisPIU.OutAirNode).Temp;
    if (state.dataLoopNodes->Node(thisPIU.OutAirNode).MassFlowRate == 0.0) {
        state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate = 0.0;
        state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = 0.0;
    }
    if (thisPIU.InducesPlenumAir) {
        state.dataHVACGlobal->PlenumInducedMassFlow = state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate;
    } else {
        state.dataHVACGlobal->PlenumInducedMassFlow = 0.0;
    }
    state.dataDefineEquipment->AirDistUnit(thisPIU.ADUNum).MassFlowRatePlenInd = state.dataHVACGlobal->PlenumInducedMassFlow;
    state.dataLoopNodes->Node(thisPIU.OutAirNode).MassFlowRateMax = thisPIU.MaxPriAirMassFlow;

    ReportCurOperatingControlStage(state, PIUNum, UnitOn, thisPIU.heatingOperatingMode, thisPIU.coolingOperatingMode);
}

void ReportCurOperatingControlStage(EnergyPlusData &state, int const piuNum, bool const unitOn, HeatOpModeType heaterMode, CoolOpModeType coolingMode)
{
    int constexpr undetermined = -1;
    int constexpr off = 0;
    int constexpr constantVolumeCooling = 1;
    int constexpr constantVolumeHeating = 2;
    int constexpr deadband = 3;
    int constexpr variableSpeedFirstStageCooling = 4;
    int constexpr variableSpeedSecondStageCooling = 5;
    int constexpr variableSpeedStagedHeatFirstStageHeating = 6;
    int constexpr variableSpeedStagedHeatSecondStageHeating = 7;
    int constexpr variableSpeedModulatedHeatFirstStageHeating = 8;
    int constexpr variableSpeedModulatedHeatSecondStageHeating = 9;
    int constexpr variableSpeedModulatedHeatThirdStageHeating = 10;

    state.dataPowerInductionUnits->PIU(piuNum).CurOperationControlStage = undetermined;

    if (!unitOn) {
        state.dataPowerInductionUnits->PIU(piuNum).CurOperationControlStage = off;
    } else {
        if (state.dataPowerInductionUnits->PIU(piuNum).fanControlType == FanCntrlType::ConstantSpeedFan) {
            if (heaterMode != HeatOpModeType::HeaterOff && coolingMode == CoolOpModeType::CoolerOff) {
                state.dataPowerInductionUnits->PIU(piuNum).CurOperationControlStage = constantVolumeHeating;
            } else if (coolingMode != CoolOpModeType::CoolerOff && heaterMode == HeatOpModeType::HeaterOff) {
                state.dataPowerInductionUnits->PIU(piuNum).CurOperationControlStage = constantVolumeCooling;
            } else {
                state.dataPowerInductionUnits->PIU(piuNum).CurOperationControlStage = deadband;
            }
        }
        if (state.dataPowerInductionUnits->PIU(piuNum).fanControlType == FanCntrlType::VariableSpeedFan) {
            if (heaterMode != HeatOpModeType::HeaterOff) {
                if (state.dataPowerInductionUnits->PIU(piuNum).heatingControlType == HeatCntrlBehaviorType::StagedHeaterBehavior) {
                    if (heaterMode == HeatOpModeType::StagedHeatFirstStage) {
                        state.dataPowerInductionUnits->PIU(piuNum).CurOperationControlStage = variableSpeedStagedHeatFirstStageHeating;
                    } else if (heaterMode == HeatOpModeType::StagedHeatSecondStage) {
                        state.dataPowerInductionUnits->PIU(piuNum).CurOperationControlStage = variableSpeedStagedHeatSecondStageHeating;
                    }
                } else if (state.dataPowerInductionUnits->PIU(piuNum).heatingControlType == HeatCntrlBehaviorType::ModulatedHeaterBehavior) {
                    if (heaterMode == HeatOpModeType::ModulatedHeatFirstStage) {
                        state.dataPowerInductionUnits->PIU(piuNum).CurOperationControlStage = variableSpeedModulatedHeatFirstStageHeating;
                    } else if (heaterMode == HeatOpModeType::ModulatedHeatSecondStage) {
                        state.dataPowerInductionUnits->PIU(piuNum).CurOperationControlStage = variableSpeedModulatedHeatSecondStageHeating;
                    } else if (heaterMode == HeatOpModeType::ModulatedHeatThirdStage) {
                        state.dataPowerInductionUnits->PIU(piuNum).CurOperationControlStage = variableSpeedModulatedHeatThirdStageHeating;
                    }
                }
            } else if (coolingMode == CoolOpModeType::CoolFirstStage) {
                state.dataPowerInductionUnits->PIU(piuNum).CurOperationControlStage = variableSpeedFirstStageCooling;
            } else if (coolingMode == CoolOpModeType::CoolSecondStage) {
                state.dataPowerInductionUnits->PIU(piuNum).CurOperationControlStage = variableSpeedSecondStageCooling;
            } else if (heaterMode == HeatOpModeType::HeaterOff && coolingMode == CoolOpModeType::CoolerOff) {
                state.dataPowerInductionUnits->PIU(piuNum).CurOperationControlStage = deadband;
            }
        }
    }
}

void CalcVariableSpeedPIUCoolingBehavior(EnergyPlusData &state,
                                         int const piuNum,   // number of the current PIU being simulated
                                         int const zoneNode, // zone node number
                                         Real64 const zoneLoad,
                                         Real64 const loadToHeatSetPt,
                                         Real64 const priAirMassFlowMin,
                                         [[maybe_unused]] Real64 const priAirMassFlowMax)
{
    auto &thisPIU = state.dataPowerInductionUnits->PIU(piuNum);
    thisPIU.coolingOperatingMode = CoolOpModeType::CoolerOff;

    // set min primary flow and low secondary
    state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate = priAirMassFlowMin;
    Real64 TotAirMassFlow = thisPIU.MinTotAirMassFlow;
    state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = max(0.0, TotAirMassFlow - priAirMassFlowMin);

    // calculate cooling provided to zone at minimum fan speed and minimum primary air mass flow
    Real64 qdotDelivMinPrim = CalcVariableSpeedPIUQdotDelivered(state, piuNum, zoneNode, false, TotAirMassFlow, thisPIU.MinFanTurnDownRatio);

    if (qdotDelivMinPrim <= zoneLoad) { // will provide more cooling than required at minimum primary flow
        thisPIU.PriAirMassFlow = priAirMassFlowMin;
        if (qdotDelivMinPrim >=
            loadToHeatSetPt) { // will provide more cooling than required but not enough to drop below the heating thermostat setpoint
            thisPIU.SecAirMassFlow = max(0.0, thisPIU.MinTotAirMassFlow - thisPIU.PriAirMassFlow);
            thisPIU.heatingOperatingMode = HeatOpModeType::HeaterOff;
        } else {
            if (thisPIU.heatingControlType == HeatCntrlBehaviorType::StagedHeaterBehavior) {
                CalcVariableSpeedPIUStagedHeatingBehavior(state, piuNum, zoneNode, loadToHeatSetPt, true, thisPIU.PriAirMassFlow);
            } else if (thisPIU.heatingControlType == HeatCntrlBehaviorType::ModulatedHeaterBehavior) {
                CalcVariableSpeedPIUModulatedHeatingBehavior(state, piuNum, zoneNode, loadToHeatSetPt, true, thisPIU.PriAirMassFlow);
            }
        }
    } else {
        // check how much cooling provided at max fan and primary air
        state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate = thisPIU.MaxPriAirMassFlow;
        TotAirMassFlow = thisPIU.MaxTotAirMassFlow;
        state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = max(0.0, TotAirMassFlow - thisPIU.MaxPriAirMassFlow);
        Real64 qdotDelivMaxFan = CalcVariableSpeedPIUQdotDelivered(state, piuNum, zoneNode, false, TotAirMassFlow, 1.0);

        if (zoneLoad <= qdotDelivMaxFan) { // not going to make it just run at max
            thisPIU.PriAirMassFlow = thisPIU.PriAirMassFlow;
            TotAirMassFlow = thisPIU.MaxTotAirMassFlow;
            thisPIU.SecAirMassFlow = max(0.0, TotAirMassFlow - thisPIU.PriAirMassFlow);
            thisPIU.heatingOperatingMode = HeatOpModeType::HeaterOff;
            thisPIU.coolingOperatingMode = CoolOpModeType::CoolSecondStage;
        } else {
            // call regula falsi solver, vary a coooling control signal for fan speed and primary air flow together from min to max.
            int constexpr MaxIte(500);    // Maximum number of iterations
            Real64 constexpr Acc(0.0001); // Accuracy of result
            int SolFla(0);                // Flag of solver
            Real64 coolSignal = 0.5;      // starting value
            auto f = [&state, piuNum, zoneLoad, zoneNode](Real64 const coolSignal) {
                return CalcVariableSpeedPIUCoolingResidual(state, coolSignal, piuNum, zoneLoad, zoneNode);
            };

            General::SolveRoot(state, Acc, MaxIte, SolFla, coolSignal, f, 0.0, 1.0);

            if (SolFla == -1) {
                ShowSevereError(state, "Iteration limit exceeded in calculating variable speed fan powered box cooling signal");
                ShowContinueErrorTimeStamp(state, "");
                ShowFatalError(state, format("Series PIU control failed for {}:{} ", thisPIU.UnitType, thisPIU.Name));
            } else if (SolFla == -2) {
                ShowSevereError(state, "Bad starting values for in calculating variable speed fan powered box cooling signal");
                ShowContinueError(state, format("Zone Load to Cooling Setpoint = {:.2R} [W]", zoneLoad));
                ShowContinueError(state, format("Load Delivered to Zone at Minimum Fan Speed  = {:.2R} [W]", qdotDelivMinPrim));
                ShowContinueErrorTimeStamp(state, "");
                ShowFatalError(state, format("Series PIU control failed for {}:{}", thisPIU.UnitType, thisPIU.Name));
            } else {
                thisPIU.PriAirMassFlow = coolSignal * (thisPIU.MaxPriAirMassFlow - thisPIU.MinPriAirMassFlow) + thisPIU.MinPriAirMassFlow;
                TotAirMassFlow = coolSignal * (thisPIU.MaxTotAirMassFlow - thisPIU.MinTotAirMassFlow) + thisPIU.MinTotAirMassFlow;
                thisPIU.SecAirMassFlow = max(0.0, TotAirMassFlow - thisPIU.PriAirMassFlow);
                thisPIU.heatingOperatingMode = HeatOpModeType::HeaterOff;
                thisPIU.coolingOperatingMode = CoolOpModeType::CoolFirstStage;
            }
        }
    }
}

void CalcVariableSpeedPIUStagedHeatingBehavior(EnergyPlusData &state,
                                               int const piuNum,   // number of the current PIU being simulated
                                               int const zoneNode, // zone node number
                                               Real64 const zoneLoad,
                                               bool const pri,
                                               Real64 const primaryAirMassFlow)
{
    auto &thisPIU = state.dataPowerInductionUnits->PIU(piuNum);

    // Calculate heating provided to zone with no coil at the maximum secondary flow rate: "1st stage, max fan"
    if (pri) {
        if (thisPIU.UnitType == "AirTerminal:SingleDuct:SeriesPIU:Reheat") {
            state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate = thisPIU.MinPriAirMassFlow;
            state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = max(0.0, thisPIU.MaxTotAirMassFlow - thisPIU.MinPriAirMassFlow);
        } else if (thisPIU.UnitType == "AirTerminal:SingleDuct:ParallelPIU:Reheat") {
            state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate = thisPIU.MinPriAirMassFlow;
            state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = thisPIU.MaxSecAirMassFlow;
        }
    } else {
        if (thisPIU.UnitType == "AirTerminal:SingleDuct:SeriesPIU:Reheat") {
            state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = thisPIU.MaxTotAirMassFlow;
        } else if (thisPIU.UnitType == "AirTerminal:SingleDuct:ParallelPIU:Reheat") {
            state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = thisPIU.MaxSecAirMassFlow;
        }
    }
    Real64 TotAirMassFlow =
        state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate + state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate;
    Real64 qdotDelivered1stStageMaxFan = CalcVariableSpeedPIUQdotDelivered(state, piuNum, zoneNode, false, TotAirMassFlow, 1.0);

    // Calculate heating provided to zone with no coil at the minimum secondary flow rate: "1st stage, min fan"
    if (pri) {
        if (thisPIU.UnitType == "AirTerminal:SingleDuct:SeriesPIU:Reheat") {
            state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate = thisPIU.MinPriAirMassFlow;
            state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = max(0.0, thisPIU.MinTotAirMassFlow - thisPIU.MinPriAirMassFlow);
        } else if (thisPIU.UnitType == "AirTerminal:SingleDuct:ParallelPIU:Reheat") {
            state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate = thisPIU.MinPriAirMassFlow;
            state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = thisPIU.MinSecAirMassFlow;
        }
    } else {
        if (thisPIU.UnitType == "AirTerminal:SingleDuct:SeriesPIU:Reheat") {
            state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = thisPIU.MinTotAirMassFlow;
        } else if (thisPIU.UnitType == "AirTerminal:SingleDuct:ParallelPIU:Reheat") {
            state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = thisPIU.MinSecAirMassFlow;
        }
    }
    TotAirMassFlow = state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate + state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate;
    Real64 qdotDelivered1stStageMinFan =
        CalcVariableSpeedPIUQdotDelivered(state, piuNum, zoneNode, false, TotAirMassFlow, thisPIU.MinFanTurnDownRatio);

    if (qdotDelivered1stStageMinFan <= zoneLoad && qdotDelivered1stStageMaxFan >= zoneLoad) { // 1st of heating (no coil) can meet the load
        // Find fan speed/flow that meets the load through iteration
        thisPIU.heatingOperatingMode = HeatOpModeType::StagedHeatFirstStage;
        int constexpr MaxIte(500);   // Maximum number of iterations
        Real64 constexpr Acc(0.001); // Accuracy of result
        int SolFla(0);               // Flag of solver
        Real64 fanSignal = 0.0;
        fanSignal = (1.0 - thisPIU.MinFanTurnDownRatio) * 0.5 + thisPIU.MinFanTurnDownRatio; // average speed as the initial value
        auto f = [&state, piuNum, zoneLoad, zoneNode, primaryAirMassFlow](Real64 const fanSignal) {
            return CalcVariableSpeedPIUHeatingResidual(state, fanSignal, piuNum, zoneLoad, zoneNode, primaryAirMassFlow, false, fanSignal);
        };

        General::SolveRoot(state, Acc, MaxIte, SolFla, fanSignal, f, thisPIU.MinFanTurnDownRatio, 1.0);

        if (SolFla == -1) {
            ShowSevereError(state, "Iteration limit exceeded in calculating variable speed fan powered box 1st stage heating fan speed");
            ShowContinueErrorTimeStamp(state, "");
            ShowFatalError(state, format("PIU control failed for {}:{} ", thisPIU.UnitType, thisPIU.Name));
        } else if (SolFla == -2) {
            ShowSevereError(state, "Bad starting values in calculating variable speed fan powered box 1st stage heating fan speed");
            ShowContinueErrorTimeStamp(state, "");
            ShowFatalError(state, format("PIU control failed for {}:{}", thisPIU.UnitType, thisPIU.Name));
        } else {
            if (thisPIU.UnitType == "AirTerminal:SingleDuct:SeriesPIU:Reheat") {
                thisPIU.SecAirMassFlow = max(0.0, fanSignal * thisPIU.MaxTotAirMassFlow - primaryAirMassFlow);
            } else if (thisPIU.UnitType == "AirTerminal:SingleDuct:ParallelPIU:Reheat") {
                thisPIU.SecAirMassFlow = max(0.0, fanSignal * thisPIU.MaxSecAirMassFlow);
            }
        }
    } else if (qdotDelivered1stStageMaxFan < zoneLoad) {
        thisPIU.heatingOperatingMode = HeatOpModeType::StagedHeatSecondStage;
        if (thisPIU.UnitType == "AirTerminal:SingleDuct:SeriesPIU:Reheat") {
            thisPIU.SecAirMassFlow = max(0.0, thisPIU.MaxTotAirMassFlow - primaryAirMassFlow);
        } else if (thisPIU.UnitType == "AirTerminal:SingleDuct:ParallelPIU:Reheat") {
            thisPIU.SecAirMassFlow = thisPIU.MaxSecAirMassFlow;
        }
    } else if (qdotDelivered1stStageMinFan > zoneLoad) {
        thisPIU.heatingOperatingMode = HeatOpModeType::HeaterOff;
        if (thisPIU.UnitType == "AirTerminal:SingleDuct:SeriesPIU:Reheat") {
            thisPIU.SecAirMassFlow = max(0.0, thisPIU.MinTotAirMassFlow - thisPIU.MinPriAirMassFlow);
        } else if (thisPIU.UnitType == "AirTerminal:SingleDuct:ParallelPIU:Reheat") {
            thisPIU.SecAirMassFlow = 0.0;
        }
    }
}

Real64 CalcVariableSpeedPIUQdotDelivered(EnergyPlusData &state,
                                         int const piuNum,   // number of the current PIU being simulated
                                         int const zoneNode, // zone node number
                                         bool const useDAT,
                                         Real64 const totAirMassFlow,
                                         Real64 const fanTurnDown)
{
    Real64 qdotDelivered = 0.0;
    auto &thisPIU = state.dataPowerInductionUnits->PIU(piuNum);
    if (thisPIU.UnitType == "AirTerminal:SingleDuct:SeriesPIU:Reheat") {
        MixerComponent::SimAirMixer(state, thisPIU.MixerName, thisPIU.Mixer_Num);
        state.dataFans->fans(thisPIU.Fan_Index)->simulate(state, false, fanTurnDown, _);
    } else {
        state.dataFans->fans(thisPIU.Fan_Index)->simulate(state, false, fanTurnDown, _);
        MixerComponent::SimAirMixer(state, thisPIU.MixerName, thisPIU.Mixer_Num);
    }
    Real64 zoneEnthalpy = Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(zoneNode).Temp, state.dataLoopNodes->Node(zoneNode).HumRat);
    Real64 piuTemp = 0.0;
    if (useDAT) {
        piuTemp = thisPIU.designHeatingDAT;
    } else {
        piuTemp = state.dataLoopNodes->Node(thisPIU.HCoilInAirNode).Temp;
    }
    Real64 piuEnthalpy = Psychrometrics::PsyHFnTdbW(piuTemp, state.dataLoopNodes->Node(zoneNode).HumRat);
    qdotDelivered = totAirMassFlow * (piuEnthalpy - zoneEnthalpy);
    return qdotDelivered;
}

void CalcVariableSpeedPIUModulatedHeatingBehavior(EnergyPlusData &state,
                                                  int const piuNum,   // number of the current PIU being simulated
                                                  int const zoneNode, // zone node number
                                                  Real64 const zoneLoad,
                                                  bool const pri,
                                                  Real64 const primaryAirMassFlow)
{
    auto &thisPIU = state.dataPowerInductionUnits->PIU(piuNum);

    // Calculate heating provided to zone with no coil at the minimum secondary flow rate: "1st stage, min fan"
    if (pri) {
        if (thisPIU.UnitType == "AirTerminal:SingleDuct:SeriesPIU:Reheat") {
            state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate = thisPIU.MinPriAirMassFlow;
            state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = max(0.0, thisPIU.MinTotAirMassFlow - thisPIU.MinPriAirMassFlow);
        } else if (thisPIU.UnitType == "AirTerminal:SingleDuct:ParallelPIU:Reheat") {
            state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate = thisPIU.MinPriAirMassFlow;
            state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = thisPIU.MinSecAirMassFlow;
        }
    } else {
        if (thisPIU.UnitType == "AirTerminal:SingleDuct:SeriesPIU:Reheat") {
            state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = thisPIU.MinTotAirMassFlow;
        } else if (thisPIU.UnitType == "AirTerminal:SingleDuct:ParallelPIU:Reheat") {
            state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = thisPIU.MinSecAirMassFlow;
        }
    }
    Real64 TotAirMassFlow =
        state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate + state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate;
    Real64 qdotDeliveredEnd1stStage = CalcVariableSpeedPIUQdotDelivered(state, piuNum, zoneNode, true, TotAirMassFlow, thisPIU.MinFanTurnDownRatio);
    if (qdotDeliveredEnd1stStage >= zoneLoad) { // 1st stage, find heating power at minimum fan speed
        thisPIU.heatingOperatingMode = HeatOpModeType::ModulatedHeatFirstStage;
        if (thisPIU.UnitType == "AirTerminal:SingleDuct:SeriesPIU:Reheat") {
            thisPIU.SecAirMassFlow = thisPIU.MinSecAirMassFlow;
        } else if (thisPIU.UnitType == "AirTerminal:SingleDuct:ParallelPIU:Reheat") {
            thisPIU.SecAirMassFlow = thisPIU.MinSecAirMassFlow;
        }
    } else {
        if (pri) {
            if (thisPIU.UnitType == "AirTerminal:SingleDuct:SeriesPIU:Reheat") {
                state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate = thisPIU.MinPriAirMassFlow;
                state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = max(0.0, thisPIU.MaxTotAirMassFlow - thisPIU.MinPriAirMassFlow);
            } else if (thisPIU.UnitType == "AirTerminal:SingleDuct:ParallelPIU:Reheat") {
                state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate = thisPIU.MinPriAirMassFlow;
                state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = thisPIU.MaxSecAirMassFlow;
            }
        } else {
            if (thisPIU.UnitType == "AirTerminal:SingleDuct:SeriesPIU:Reheat") {
                state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate = 0.0;
                state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = thisPIU.MaxTotAirMassFlow;
            } else if (thisPIU.UnitType == "AirTerminal:SingleDuct:ParallelPIU:Reheat") {
                state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate = 0.0;
                state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = thisPIU.MaxSecAirMassFlow;
            }
        }
        TotAirMassFlow = state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate + state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate;
        Real64 qdotDeliveredEnd2ndStage =
            CalcVariableSpeedPIUQdotDelivered(state, piuNum, zoneNode, true, TotAirMassFlow, thisPIU.MinFanTurnDownRatio);
        if (qdotDeliveredEnd2ndStage > zoneLoad) { // 2nd stage
            thisPIU.heatingOperatingMode = HeatOpModeType::ModulatedHeatSecondStage;
            // Find fan speed that meets zone heating load
            int constexpr MaxIte(500);                                                                  // Maximum number of iterations
            Real64 constexpr Acc(0.0001);                                                               // Accuracy of result
            int SolFla(0);                                                                              // Flag of solver
            Real64 fanSignal = (1.0 - thisPIU.MinFanTurnDownRatio) * 0.5 + thisPIU.MinFanTurnDownRatio; // starting value in middle
            auto f = [&state, piuNum, zoneLoad, zoneNode, primaryAirMassFlow](Real64 const fanSignal) {
                return CalcVariableSpeedPIUHeatingResidual(state, fanSignal, piuNum, zoneLoad, zoneNode, primaryAirMassFlow, true, fanSignal);
            };

            General::SolveRoot(state, Acc, MaxIte, SolFla, fanSignal, f, thisPIU.MinFanTurnDownRatio, 1.0);

            if (SolFla == -1) {
                ShowSevereError(state, "Iteration limit exceeded in calculating variable speed fan powered box 2nd stage heating fan speed");
                ShowContinueErrorTimeStamp(state, "");
                ShowFatalError(state, format("PIU control failed for {}:{}", thisPIU.UnitType, thisPIU.Name));
            } else if (SolFla == -2) {
                ShowSevereError(state, "Bad starting values for in calculating variable speed fan powered box 2nd stage heating fan speed");
                ShowContinueErrorTimeStamp(state, "");
                ShowFatalError(state, format("PIU control failed for {}:{}", thisPIU.UnitType, thisPIU.Name));
            } else {
                if (thisPIU.UnitType == "AirTerminal:SingleDuct:SeriesPIU:Reheat") {
                    thisPIU.SecAirMassFlow = max(0.0, fanSignal * thisPIU.MaxTotAirMassFlow - primaryAirMassFlow);
                } else if (thisPIU.UnitType == "AirTerminal:SingleDuct:ParallelPIU:Reheat") {
                    thisPIU.SecAirMassFlow = max(0.0, fanSignal * thisPIU.MaxSecAirMassFlow);
                }
            }
        } else { // 3rd stage, full fan speed
            thisPIU.heatingOperatingMode = HeatOpModeType::ModulatedHeatThirdStage;
            if (thisPIU.UnitType == "AirTerminal:SingleDuct:SeriesPIU:Reheat") {
                thisPIU.SecAirMassFlow = thisPIU.MaxTotAirMassFlow - thisPIU.MinPriAirMassFlow;
            } else if (thisPIU.UnitType == "AirTerminal:SingleDuct:ParallelPIU:Reheat") {
                thisPIU.SecAirMassFlow = thisPIU.MaxSecAirMassFlow;
            }
        }
    }
}

Real64 CalcVariableSpeedPIUHeatingResidual(EnergyPlusData &state,
                                           Real64 const fanSignal,
                                           int const piuNum,
                                           Real64 const targetQznReq,
                                           int const zoneNodeNum,
                                           Real64 const primaryMassFlow,
                                           bool useDAT,
                                           Real64 const fanTurnDown)

{
    // used to find a fan speed to meet load to heating setpoint with no heater power
    // 1st stage heating for staged heat, also used for undershoot case where cooling at min primary flow would push below heating
    // setpoint.
    auto &thisPIU = state.dataPowerInductionUnits->PIU(piuNum);
    state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate = primaryMassFlow;
    Real64 TotAirMassFlow = 0.0;
    if (thisPIU.UnitType == "AirTerminal:SingleDuct:SeriesPIU:Reheat") {
        TotAirMassFlow = fanSignal * thisPIU.MaxTotAirMassFlow;
        state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = max(0.0, TotAirMassFlow - primaryMassFlow);
    } else {
        // parallel
        TotAirMassFlow = fanSignal * thisPIU.MaxSecAirMassFlow + primaryMassFlow;
        state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = fanSignal * thisPIU.MaxSecAirMassFlow;
    }

    // calculate heating provided to zone
    Real64 qdotDelivered = CalcVariableSpeedPIUQdotDelivered(state, piuNum, zoneNodeNum, useDAT, TotAirMassFlow, fanTurnDown);
    // formulate residual and return
    Real64 Residuum = (targetQznReq - qdotDelivered);
    return Residuum;
}

Real64 CalcVariableSpeedPIUCoolingResidual(EnergyPlusData &state, Real64 const coolSignal, int piuNum, Real64 targetQznReq, int zoneNodeNum)
{
    // used for cooling control with VS fan.  Simultaneous control of fan speed and primary air damper
    // given trial cooling signal, calculate the cooling provided and a residual that compares what is delivered vs what the zone
    // needs. set the flows, controller acts on fan and damper simultaneously
    auto &thisPIU = state.dataPowerInductionUnits->PIU(piuNum);
    Real64 PriAirMassFlow = coolSignal * (thisPIU.MaxPriAirMassFlow - thisPIU.MinPriAirMassFlow) + thisPIU.MinPriAirMassFlow;
    Real64 TotAirMassFlow = coolSignal * (thisPIU.MaxTotAirMassFlow - thisPIU.MinTotAirMassFlow) + thisPIU.MinTotAirMassFlow;
    Real64 SecAirMassFlow = max(0.0, TotAirMassFlow - PriAirMassFlow);
    state.dataLoopNodes->Node(thisPIU.PriAirInNode).MassFlowRate = PriAirMassFlow;
    state.dataLoopNodes->Node(thisPIU.SecAirInNode).MassFlowRate = SecAirMassFlow;

    Real64 fanTurnDown = coolSignal * (1.0 - thisPIU.MinFanTurnDownRatio) + thisPIU.MinFanTurnDownRatio;
    Real64 qdotDelivered = CalcVariableSpeedPIUQdotDelivered(state, piuNum, zoneNodeNum, false, TotAirMassFlow, fanTurnDown);
    // formulate residual and return
    Real64 Residuum = (targetQznReq - qdotDelivered);
    return Residuum;
}

void ReportPIU(EnergyPlusData &state, int const PIUNum) // number of the current fan coil unit being simulated
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   August 2000
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Fills some report variables for the PIU terminal boxes

    // Using/Aliasing
    Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

    auto &thisPIU = state.dataPowerInductionUnits->PIU(PIUNum);
    thisPIU.HeatingEnergy = thisPIU.HeatingRate * TimeStepSysSec;
    thisPIU.SensCoolEnergy = thisPIU.SensCoolRate * TimeStepSysSec;

    // set zone OA Volume flow rate
    thisPIU.CalcOutdoorAirVolumeFlowRate(state);
}

// ===================== Utilities =====================================

bool PIUnitHasMixer(EnergyPlusData &state, std::string_view CompName) // component (mixer) name
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   September 2011
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Given a mixer name, this routine determines if that mixer is found on
    // PIUnits.

    // Return value
    bool YesNo = false; // True if found

    if (state.dataPowerInductionUnits->GetPIUInputFlag) {
        GetPIUs(state);
        state.dataPowerInductionUnits->GetPIUInputFlag = false;
    }

    if (state.dataPowerInductionUnits->NumPIUs > 0) {
        int const ItemNum = Util::FindItemInList(CompName, state.dataPowerInductionUnits->PIU, &PowIndUnitData::MixerName);
        if (ItemNum > 0) {
            YesNo = true;
        }
    }

    return YesNo;
}

void PIUInducesPlenumAir(EnergyPlusData &state, int const NodeNum, int const plenumNum) // induced air node number
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   January 2012
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Marks a PIU air terminal unit as obtaining its induced air from
    // a plenum.

    if (state.dataPowerInductionUnits->GetPIUInputFlag) {
        GetPIUs(state);
        state.dataPowerInductionUnits->GetPIUInputFlag = false;
    }

    for (int PIUIndex = 1; PIUIndex <= state.dataPowerInductionUnits->NumPIUs; ++PIUIndex) {
        if (NodeNum == state.dataPowerInductionUnits->PIU(PIUIndex).SecAirInNode) {
            state.dataPowerInductionUnits->PIU(PIUIndex).InducesPlenumAir = true;
            state.dataPowerInductionUnits->PIU(PIUIndex).plenumIndex = plenumNum;
            break;
        }
    }
}

void PowIndUnitData::CalcOutdoorAirVolumeFlowRate(EnergyPlusData &state)
{
    // calculates zone outdoor air volume flow rate using the supply air flow rate and OA fraction
    if (this->AirLoopNum > 0) {
        this->OutdoorAirFlowRate = (state.dataLoopNodes->Node(this->PriAirInNode).MassFlowRate / state.dataEnvrn->StdRhoAir) *
                                   state.dataAirLoop->AirLoopFlow(this->AirLoopNum).OAFrac;
    } else {
        this->OutdoorAirFlowRate = 0.0;
    }
}

void PowIndUnitData::reportTerminalUnit(EnergyPlusData &state)
{
    // populate the predefined equipment summary report related to air terminals
    auto &orp = state.dataOutRptPredefined;
    auto &adu = state.dataDefineEquipment->AirDistUnit(this->ADUNum);
    if (!state.dataSize->TermUnitFinalZoneSizing.empty()) {
        auto &sizing = state.dataSize->TermUnitFinalZoneSizing(adu.TermUnitSizingNum);
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermMinFlow, adu.Name, sizing.DesCoolVolFlowMin);
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermMinOutdoorFlow, adu.Name, sizing.MinOA);
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermSupCoolingSP, adu.Name, sizing.CoolDesTemp);
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermSupHeatingSP, adu.Name, sizing.HeatDesTemp);
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermHeatingCap, adu.Name, sizing.DesHeatLoad);
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermCoolingCap, adu.Name, sizing.DesCoolLoad);
    }
    OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermTypeInp, adu.Name, this->UnitType);
    OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermPrimFlow, adu.Name, this->MaxPriAirVolFlow);
    OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermSecdFlow, adu.Name, this->MaxSecAirVolFlow);
    OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermMinFlowSch, adu.Name, "n/a");
    OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermMaxFlowReh, adu.Name, "n/a");
    OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermMinOAflowSch, adu.Name, "n/a");
    OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermHeatCoilType, adu.Name, HCoilNamesUC[(int)this->HCoilType]);
    OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermCoolCoilType, adu.Name, "n/a");
    OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermFanType, adu.Name, HVAC::fanTypeNames[(int)this->fanType]);
    OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermFanName, adu.Name, this->FanName);
    OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermFanCtrlType, adu.Name, fanCntrlTypeNames[static_cast<int>(this->fanControlType)]);
    if (this->fanControlType == FanCntrlType::VariableSpeedFan) {
        OutputReportPredefined::PreDefTableEntry(
            state, orp->pdchAirTermPIUHeatCtrlType, adu.Name, heatCntrlTypeNames[static_cast<int>(this->heatingControlType)]);
    } else {
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermPIUHeatCtrlType, adu.Name, "n/a");
    }
}

} // namespace EnergyPlus::PoweredInductionUnits
