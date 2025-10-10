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
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataConvergParams.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/DualDuct.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace DualDuct {
    // Module containing the DualDuct simulation routines

    // MODULE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   February 2000
    //       MODIFIED       Clayton Miller, Brent Griffith Aug. 2010 - Added DualDuctOA Terminal Unit to Simulate Decoupled OA/RA
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // To encapsulate the data and algorithms required to
    // manage the DualDuct Systems Simulation

    constexpr std::string_view cCMO_DDConstantVolume = "AirTerminal:DualDuct:ConstantVolume";
    constexpr std::string_view cCMO_DDVariableVolume = "AirTerminal:DualDuct:VAV";
    constexpr std::string_view cCMO_DDVarVolOA = "AirTerminal:DualDuct:VAV:OutdoorAir";
    constexpr Real64 DualDuctMassFlowSetToler = DataConvergParams::HVACFlowRateToler * 0.00001;
    constexpr std::array<std::string_view, static_cast<int>(PerPersonMode::Num)> modeStrings = {"NOTSET", "CURRENTOCCUPANCY", "DESIGNOCCUPANCY"};
    constexpr std::array<std::string_view, static_cast<int>(DualDuctDamper::Num)> damperTypeStrings = {"ConstantVolume", "VAV", "VAV:OutdoorAir"};
    constexpr std::array<std::string_view, static_cast<int>(DualDuctDamper::Num)> cmoNameArray = {
        cCMO_DDConstantVolume, cCMO_DDVariableVolume, cCMO_DDVarVolOA};

    void SimulateDualDuct(
        EnergyPlusData &state, std::string_view CompName, bool const FirstHVACIteration, int const ZoneNum, int const ZoneNodeNum, int &CompIndex)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   February 2000
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine manages Damper component simulation.
        // It is called from the SimAirLoopComponent
        // at the system time step.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int DDNum; // The Damper that you are currently loading input into

        // Obtains and Allocates Damper related parameters from input file
        if (state.dataDualDuct->GetDualDuctInputFlag) { // First time subroutine has been entered
            GetDualDuctInput(state);
            state.dataDualDuct->GetDualDuctInputFlag = false;
        }

        // Find the correct DDNumber with the AirLoop & CompNum from AirLoop Derived Type
        if (CompIndex == 0) {
            DDNum = Util::FindItemInList(CompName, state.dataDualDuct->dd_airterminal, &DualDuctAirTerminal::Name);
            if (DDNum == 0) {
                ShowFatalError(state, format("SimulateDualDuct: Damper not found={}", CompName));
            }
            CompIndex = DDNum;
        } else {
            DDNum = CompIndex;
            if (DDNum > state.dataDualDuct->NumDDAirTerminal || DDNum < 1) {
                ShowFatalError(state,
                               format("SimulateDualDuct: Invalid CompIndex passed={}, Number of Dampers={}, Damper name={}",
                                      CompIndex,
                                      state.dataDualDuct->NumDDAirTerminal,
                                      CompName));
            }
            if (state.dataDualDuct->dd_airterminal(DDNum).CheckEquipName) {
                if (CompName != state.dataDualDuct->dd_airterminal(DDNum).Name) {
                    ShowFatalError(state,
                                   format("SimulateDualDuct: Invalid CompIndex passed={}, Damper name={}, stored Damper Name for that index={}",
                                          CompIndex,
                                          CompName,
                                          state.dataDualDuct->dd_airterminal(DDNum).Name));
                }
                state.dataDualDuct->dd_airterminal(DDNum).CheckEquipName = false;
            }
        }

        auto &thisDualDuct(state.dataDualDuct->dd_airterminal(DDNum));

        if (CompIndex > 0) {
            state.dataSize->CurTermUnitSizingNum = state.dataDefineEquipment->AirDistUnit(thisDualDuct.ADUNum).TermUnitSizingNum;
            // With the correct DDNum Initialize
            thisDualDuct.InitDualDuct(state, FirstHVACIteration); // Initialize all Damper related parameters

            // Calculate the Correct Damper Model with the current DDNum
            switch (thisDualDuct.DamperType) {
            case DualDuctDamper::ConstantVolume: { // 'AirTerminal:DualDuct:ConstantVolume'
                thisDualDuct.SimDualDuctConstVol(state, ZoneNum, ZoneNodeNum);
            } break;
            case DualDuctDamper::VariableVolume: { // 'AirTerminal:DualDuct:VAV'
                thisDualDuct.SimDualDuctVarVol(state, ZoneNum, ZoneNodeNum);
            } break;
            case DualDuctDamper::OutdoorAir: {
                thisDualDuct.SimDualDuctVAVOutdoorAir(state, ZoneNum, ZoneNodeNum); // 'AirTerminal:DualDuct:VAV:OutdoorAir'
            } break;
            default:
                break;
            }

            // Update the current Damper to the outlet nodes
            thisDualDuct.UpdateDualDuct(state);
        } else {
            ShowFatalError(state, format("SimulateDualDuct: Damper not found={}", CompName));
        }
    }

    void GetDualDuctInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   April 1998
        //       MODIFIED       Julien Marrec of EffiBEM, 2017-12-18
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is the main routine to call other input routines and Get routines

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger events.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetDualDuctInput: "); // include trailing bla
        static constexpr std::string_view routineName = "GetDualDuctInput";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlphas;
        int NumNums;
        int IOStat;
        Array1D<Real64> NumArray(2, 0.0);
        Array1D_string AlphArray(7);
        Array1D_string cAlphaFields(7);       // Alpha field names
        Array1D_string cNumericFields(2);     // Numeric field names
        Array1D_bool lAlphaBlanks(7, true);   // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks(2, true); // Logical array, numeric field input BLANK = .TRUE.
        std::string CurrentModuleObject;      // for ease in getting objects
        bool ErrorsFound(false);              // If errors detected in input
        int SupAirIn;                         // controlled zone supply air inlet index
        int ADUNum;                           // loop control to search Air Distribution Units
        Real64 DummyOAFlow(0.0);

        int NumDualDuctConstVolDampers = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCMO_DDConstantVolume);
        int NumDualDuctVarVolDampers = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCMO_DDVariableVolume);
        state.dataDualDuct->NumDualDuctVarVolOA = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCMO_DDVarVolOA);
        state.dataDualDuct->NumDDAirTerminal = NumDualDuctConstVolDampers + NumDualDuctVarVolDampers + state.dataDualDuct->NumDualDuctVarVolOA;
        state.dataDualDuct->dd_airterminal.allocate(state.dataDualDuct->NumDDAirTerminal);
        state.dataDualDuct->UniqueDualDuctAirTerminalNames.reserve(state.dataDualDuct->NumDDAirTerminal);

        if (NumDualDuctConstVolDampers > 0) {
            CurrentModuleObject = cCMO_DDConstantVolume;
            for (int DamperIndex = 1; DamperIndex <= NumDualDuctConstVolDampers; ++DamperIndex) {

                // Load the info from the damper

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         CurrentModuleObject,
                                                                         DamperIndex,
                                                                         AlphArray,
                                                                         NumAlphas,
                                                                         NumArray,
                                                                         NumNums,
                                                                         IOStat,
                                                                         lNumericBlanks,
                                                                         lAlphaBlanks,
                                                                         cAlphaFields,
                                                                         cNumericFields);

                ErrorObjectHeader eoh{routineName, CurrentModuleObject, AlphArray(1)};

                // Anything below this line in this control block should use DDNum
                int DDNum = DamperIndex;
                auto &thisDD = state.dataDualDuct->dd_airterminal(DDNum);
                GlobalNames::VerifyUniqueInterObjectName(
                    state, state.dataDualDuct->UniqueDualDuctAirTerminalNames, AlphArray(1), CurrentModuleObject, cAlphaFields(1), ErrorsFound);
                thisDD.Name = AlphArray(1);
                thisDD.DamperType = DualDuctDamper::ConstantVolume;
                if (lAlphaBlanks(2)) {
                    thisDD.availSched = Sched::GetScheduleAlwaysOn(state);
                } else if ((thisDD.availSched = Sched::GetSchedule(state, AlphArray(2))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(2), AlphArray(2));
                    ErrorsFound = true;
                }
                thisDD.OutletNodeNum = GetOnlySingleNode(state,
                                                         AlphArray(3),
                                                         ErrorsFound,
                                                         DataLoopNode::ConnectionObjectType::AirTerminalDualDuctConstantVolume,
                                                         thisDD.Name,
                                                         DataLoopNode::NodeFluidType::Air,
                                                         DataLoopNode::ConnectionType::Outlet,
                                                         NodeInputManager::CompFluidStream::Primary,
                                                         DataLoopNode::ObjectIsNotParent,
                                                         cAlphaFields(3));
                thisDD.HotAirInletNodeNum = GetOnlySingleNode(state,
                                                              AlphArray(4),
                                                              ErrorsFound,
                                                              DataLoopNode::ConnectionObjectType::AirTerminalDualDuctConstantVolume,
                                                              thisDD.Name,
                                                              DataLoopNode::NodeFluidType::Air,
                                                              DataLoopNode::ConnectionType::Inlet,
                                                              NodeInputManager::CompFluidStream::Primary,
                                                              DataLoopNode::ObjectIsNotParent,
                                                              cAlphaFields(4));
                thisDD.ColdAirInletNodeNum = GetOnlySingleNode(state,
                                                               AlphArray(5),
                                                               ErrorsFound,
                                                               DataLoopNode::ConnectionObjectType::AirTerminalDualDuctConstantVolume,
                                                               thisDD.Name,
                                                               DataLoopNode::NodeFluidType::Air,
                                                               DataLoopNode::ConnectionType::Inlet,
                                                               NodeInputManager::CompFluidStream::Primary,
                                                               DataLoopNode::ObjectIsNotParent,
                                                               cAlphaFields(5));

                thisDD.MaxAirVolFlowRate = NumArray(1);
                thisDD.ZoneMinAirFracDes = 0.0;

                // Register component set data - one for heat and one for cool
                BranchNodeConnections::TestCompSet(state, CurrentModuleObject + ":HEAT", thisDD.Name, AlphArray(4), AlphArray(3), "Air Nodes");
                BranchNodeConnections::TestCompSet(state, CurrentModuleObject + ":COOL", thisDD.Name, AlphArray(5), AlphArray(3), "Air Nodes");

                for (ADUNum = 1; ADUNum <= (int)state.dataDefineEquipment->AirDistUnit.size(); ++ADUNum) {
                    if (thisDD.OutletNodeNum == state.dataDefineEquipment->AirDistUnit(ADUNum).OutletNodeNum) {
                        state.dataDefineEquipment->AirDistUnit(ADUNum).InletNodeNum = thisDD.ColdAirInletNodeNum;
                        state.dataDefineEquipment->AirDistUnit(ADUNum).InletNodeNum2 = thisDD.HotAirInletNodeNum;
                        thisDD.ADUNum = ADUNum;
                    }
                }
                // one assumes if there isn't one assigned, it's an error?
                if (thisDD.ADUNum == 0) {
                    auto &thisObjType = damperTypeStrings[static_cast<int>(thisDD.DamperType)];
                    ShowSevereError(
                        state,
                        format("{}No matching List:Zone:AirTerminal for AirTerminal:DualDuct = [{},{}].", RoutineName, thisObjType, thisDD.Name));
                    ShowContinueError(state, format("...should have outlet node={}", state.dataLoopNodes->NodeID(thisDD.OutletNodeNum)));
                    ErrorsFound = true;
                } else {

                    // Fill the Zone Equipment data with the inlet node numbers of this unit.
                    for (int CtrlZone = 1; CtrlZone <= state.dataGlobal->NumOfZones; ++CtrlZone) {
                        auto &thisZoneEquipConfig = state.dataZoneEquip->ZoneEquipConfig(CtrlZone);
                        if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZone).IsControlled) {
                            continue;
                        }
                        for (SupAirIn = 1; SupAirIn <= thisZoneEquipConfig.NumInletNodes; ++SupAirIn) {
                            if (thisDD.OutletNodeNum == thisZoneEquipConfig.InletNode(SupAirIn)) {
                                if (state.dataZoneEquip->ZoneEquipConfig(CtrlZone).AirDistUnitCool(SupAirIn).OutNode > 0) {
                                    ShowSevereError(state, "Error in connecting a terminal unit to a zone");
                                    ShowContinueError(
                                        state, format("{} already connects to another zone", state.dataLoopNodes->NodeID(thisDD.OutletNodeNum)));
                                    ShowContinueError(state, format("Occurs for terminal unit {} = {}", CurrentModuleObject, thisDD.Name));
                                    ShowContinueError(state, "Check terminal unit node names for errors");
                                    ErrorsFound = true;
                                } else {
                                    thisZoneEquipConfig.AirDistUnitCool(SupAirIn).InNode = thisDD.ColdAirInletNodeNum;
                                    thisZoneEquipConfig.AirDistUnitHeat(SupAirIn).InNode = thisDD.HotAirInletNodeNum;
                                    thisZoneEquipConfig.AirDistUnitCool(SupAirIn).OutNode = thisDD.OutletNodeNum;
                                    thisZoneEquipConfig.AirDistUnitHeat(SupAirIn).OutNode = thisDD.OutletNodeNum;
                                    state.dataDefineEquipment->AirDistUnit(thisDD.ADUNum).TermUnitSizingNum =
                                        thisZoneEquipConfig.AirDistUnitCool(SupAirIn).TermUnitSizingIndex;
                                    state.dataDefineEquipment->AirDistUnit(thisDD.ADUNum).ZoneEqNum = CtrlZone;
                                }
                                thisDD.CtrlZoneNum = CtrlZone;
                                thisDD.CtrlZoneInNodeIndex = SupAirIn;
                            }
                        }
                    }
                }
                // Setup the Average damper Position output variable
                // CurrentModuleObject='AirTerminal:DualDuct:ConstantVolume'
                SetupOutputVariable(state,
                                    "Zone Air Terminal Cold Supply Duct Damper Position",
                                    Constant::Units::None,
                                    thisDD.ColdAirDamperPosition,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    thisDD.Name);
                SetupOutputVariable(state,
                                    "Zone Air Terminal Hot Supply Duct Damper Position",
                                    Constant::Units::None,
                                    thisDD.HotAirDamperPosition,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    thisDD.Name);

            } // end Number of Damper Loop
        }

        if (NumDualDuctVarVolDampers > 0) {
            CurrentModuleObject = cCMO_DDVariableVolume;
            for (int DamperIndex = 1; DamperIndex <= NumDualDuctVarVolDampers; ++DamperIndex) {

                // Load the info from the damper

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         CurrentModuleObject,
                                                                         DamperIndex,
                                                                         AlphArray,
                                                                         NumAlphas,
                                                                         NumArray,
                                                                         NumNums,
                                                                         IOStat,
                                                                         lNumericBlanks,
                                                                         lAlphaBlanks,
                                                                         cAlphaFields,
                                                                         cNumericFields);

                ErrorObjectHeader eoh{routineName, CurrentModuleObject, AlphArray(1)};

                // Anything below this line in this control block should use DDNum
                int DDNum = DamperIndex + NumDualDuctConstVolDampers;
                auto &thisDD = state.dataDualDuct->dd_airterminal(DDNum);
                GlobalNames::VerifyUniqueInterObjectName(
                    state, state.dataDualDuct->UniqueDualDuctAirTerminalNames, AlphArray(1), CurrentModuleObject, cAlphaFields(1), ErrorsFound);
                thisDD.Name = AlphArray(1);
                thisDD.DamperType = DualDuctDamper::VariableVolume;
                if (lAlphaBlanks(2)) {
                    thisDD.availSched = Sched::GetScheduleAlwaysOn(state);
                } else if ((thisDD.availSched = Sched::GetSchedule(state, AlphArray(2))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(2), AlphArray(2));
                    ErrorsFound = true;
                }
                thisDD.OutletNodeNum = GetOnlySingleNode(state,
                                                         AlphArray(3),
                                                         ErrorsFound,
                                                         DataLoopNode::ConnectionObjectType::AirTerminalDualDuctVAV,
                                                         thisDD.Name,
                                                         DataLoopNode::NodeFluidType::Air,
                                                         DataLoopNode::ConnectionType::Outlet,
                                                         NodeInputManager::CompFluidStream::Primary,
                                                         DataLoopNode::ObjectIsNotParent,
                                                         cAlphaFields(3));
                thisDD.HotAirInletNodeNum = GetOnlySingleNode(state,
                                                              AlphArray(4),
                                                              ErrorsFound,
                                                              DataLoopNode::ConnectionObjectType::AirTerminalDualDuctVAV,
                                                              thisDD.Name,
                                                              DataLoopNode::NodeFluidType::Air,
                                                              DataLoopNode::ConnectionType::Inlet,
                                                              NodeInputManager::CompFluidStream::Primary,
                                                              DataLoopNode::ObjectIsNotParent,
                                                              cAlphaFields(4));
                thisDD.ColdAirInletNodeNum = GetOnlySingleNode(state,
                                                               AlphArray(5),
                                                               ErrorsFound,
                                                               DataLoopNode::ConnectionObjectType::AirTerminalDualDuctVAV,
                                                               thisDD.Name,
                                                               DataLoopNode::NodeFluidType::Air,
                                                               DataLoopNode::ConnectionType::Inlet,
                                                               NodeInputManager::CompFluidStream::Primary,
                                                               DataLoopNode::ObjectIsNotParent,
                                                               cAlphaFields(5));

                thisDD.MaxAirVolFlowRate = NumArray(1);
                thisDD.ZoneMinAirFracDes = NumArray(2);

                // Register component set data - one for heat and one for cool
                BranchNodeConnections::TestCompSet(state, CurrentModuleObject + ":HEAT", thisDD.Name, AlphArray(4), AlphArray(3), "Air Nodes");
                BranchNodeConnections::TestCompSet(state, CurrentModuleObject + ":COOL", thisDD.Name, AlphArray(5), AlphArray(3), "Air Nodes");

                for (ADUNum = 1; ADUNum <= (int)state.dataDefineEquipment->AirDistUnit.size(); ++ADUNum) {
                    if (thisDD.OutletNodeNum == state.dataDefineEquipment->AirDistUnit(ADUNum).OutletNodeNum) {
                        state.dataDefineEquipment->AirDistUnit(ADUNum).InletNodeNum = thisDD.ColdAirInletNodeNum;
                        state.dataDefineEquipment->AirDistUnit(ADUNum).InletNodeNum2 = thisDD.HotAirInletNodeNum;
                        thisDD.ADUNum = ADUNum;
                    }
                }
                // one assumes if there isn't one assigned, it's an error?
                if (thisDD.ADUNum == 0) {
                    auto &thisObjType = damperTypeStrings[static_cast<int>(thisDD.DamperType)];
                    ShowSevereError(
                        state,
                        format("{}No matching List:Zone:AirTerminal for AirTerminal:DualDuct = [{},{}].", RoutineName, thisObjType, thisDD.Name));
                    ShowContinueError(state, format("...should have outlet node={}", state.dataLoopNodes->NodeID(thisDD.OutletNodeNum)));
                    ErrorsFound = true;
                } else {

                    // Fill the Zone Equipment data with the inlet node numbers of this unit.
                    for (int CtrlZone = 1; CtrlZone <= state.dataGlobal->NumOfZones; ++CtrlZone) {
                        auto &thisZoneEquipConfig = state.dataZoneEquip->ZoneEquipConfig(CtrlZone);
                        if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZone).IsControlled) {
                            continue;
                        }
                        for (SupAirIn = 1; SupAirIn <= thisZoneEquipConfig.NumInletNodes; ++SupAirIn) {
                            if (thisDD.OutletNodeNum == thisZoneEquipConfig.InletNode(SupAirIn)) {
                                thisZoneEquipConfig.AirDistUnitCool(SupAirIn).InNode = thisDD.ColdAirInletNodeNum;
                                thisZoneEquipConfig.AirDistUnitHeat(SupAirIn).InNode = thisDD.HotAirInletNodeNum;
                                thisZoneEquipConfig.AirDistUnitCool(SupAirIn).OutNode = thisDD.OutletNodeNum;
                                thisZoneEquipConfig.AirDistUnitHeat(SupAirIn).OutNode = thisDD.OutletNodeNum;
                                state.dataDefineEquipment->AirDistUnit(thisDD.ADUNum).TermUnitSizingNum =
                                    thisZoneEquipConfig.AirDistUnitCool(SupAirIn).TermUnitSizingIndex;
                                state.dataDefineEquipment->AirDistUnit(thisDD.ADUNum).ZoneEqNum = CtrlZone;

                                thisDD.CtrlZoneNum = CtrlZone;
                                thisDD.CtrlZoneInNodeIndex = SupAirIn;
                            }
                        }
                    }
                }
                if (!lAlphaBlanks(6)) {
                    thisDD.OARequirementsPtr = Util::FindItemInList(AlphArray(6), state.dataSize->OARequirements);
                    if (thisDD.OARequirementsPtr == 0) {
                        ShowSevereError(state, format("{} = {} not found.", cAlphaFields(6), AlphArray(6)));
                        ShowContinueError(state, format("Occurs in {} = {}", cCMO_DDVariableVolume, thisDD.Name));
                        ErrorsFound = true;
                    } else {
                        thisDD.NoOAFlowInputFromUser = false;
                    }
                }

                if (lAlphaBlanks(7)) {
                    thisDD.ZoneTurndownMinAirFrac = 1.0;
                } else if ((thisDD.zoneTurndownMinAirFracSched = Sched::GetSchedule(state, AlphArray(7))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(7), AlphArray(7));
                    ErrorsFound = true;
                }

                // Setup the Average damper Position output variable
                // CurrentModuleObject='AirTerminal:DualDuct:VAV'
                SetupOutputVariable(state,
                                    "Zone Air Terminal Cold Supply Duct Damper Position",
                                    Constant::Units::None,
                                    thisDD.ColdAirDamperPosition,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    thisDD.Name);
                SetupOutputVariable(state,
                                    "Zone Air Terminal Hot Supply Duct Damper Position",
                                    Constant::Units::None,
                                    thisDD.HotAirDamperPosition,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    thisDD.Name);
                SetupOutputVariable(state,
                                    "Zone Air Terminal Outdoor Air Volume Flow Rate",
                                    Constant::Units::m3_s,
                                    thisDD.OutdoorAirFlowRate,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    thisDD.Name);
            } // end Number of Damper Loop
        }

        if (state.dataDualDuct->NumDualDuctVarVolOA > 0) {
            CurrentModuleObject = cCMO_DDVarVolOA;
            for (int DamperIndex = 1; DamperIndex <= state.dataDualDuct->NumDualDuctVarVolOA; ++DamperIndex) {

                // Load the info from the damper
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         CurrentModuleObject,
                                                                         DamperIndex,
                                                                         AlphArray,
                                                                         NumAlphas,
                                                                         NumArray,
                                                                         NumNums,
                                                                         IOStat,
                                                                         lNumericBlanks,
                                                                         lAlphaBlanks,
                                                                         cAlphaFields,
                                                                         cNumericFields);

                ErrorObjectHeader eoh{routineName, CurrentModuleObject, AlphArray(1)};

                // Anything below this line in this control block should use DDNum
                int DDNum = DamperIndex + NumDualDuctConstVolDampers + NumDualDuctVarVolDampers;
                auto &thisDD = state.dataDualDuct->dd_airterminal(DDNum);
                GlobalNames::VerifyUniqueInterObjectName(
                    state, state.dataDualDuct->UniqueDualDuctAirTerminalNames, AlphArray(1), CurrentModuleObject, cAlphaFields(1), ErrorsFound);
                thisDD.Name = AlphArray(1);
                thisDD.DamperType = DualDuctDamper::OutdoorAir;
                if (lAlphaBlanks(2)) {
                    thisDD.availSched = Sched::GetScheduleAlwaysOn(state);
                } else if ((thisDD.availSched = Sched::GetSchedule(state, AlphArray(2))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(2), AlphArray(2));
                    ErrorsFound = true;
                }
                thisDD.OutletNodeNum = GetOnlySingleNode(state,
                                                         AlphArray(3),
                                                         ErrorsFound,
                                                         DataLoopNode::ConnectionObjectType::AirTerminalDualDuctVAVOutdoorAir,
                                                         thisDD.Name,
                                                         DataLoopNode::NodeFluidType::Air,
                                                         DataLoopNode::ConnectionType::Outlet,
                                                         NodeInputManager::CompFluidStream::Primary,
                                                         DataLoopNode::ObjectIsNotParent,
                                                         cAlphaFields(3));
                thisDD.OAInletNodeNum = GetOnlySingleNode(state,
                                                          AlphArray(4),
                                                          ErrorsFound,
                                                          DataLoopNode::ConnectionObjectType::AirTerminalDualDuctVAVOutdoorAir,
                                                          thisDD.Name,
                                                          DataLoopNode::NodeFluidType::Air,
                                                          DataLoopNode::ConnectionType::Inlet,
                                                          NodeInputManager::CompFluidStream::Primary,
                                                          DataLoopNode::ObjectIsNotParent,
                                                          cAlphaFields(4));

                if (!lAlphaBlanks(5)) {
                    thisDD.RecircAirInletNodeNum = GetOnlySingleNode(state,
                                                                     AlphArray(5),
                                                                     ErrorsFound,
                                                                     DataLoopNode::ConnectionObjectType::AirTerminalDualDuctVAVOutdoorAir,
                                                                     thisDD.Name,
                                                                     DataLoopNode::NodeFluidType::Air,
                                                                     DataLoopNode::ConnectionType::Inlet,
                                                                     NodeInputManager::CompFluidStream::Primary,
                                                                     DataLoopNode::ObjectIsNotParent,
                                                                     cAlphaFields(5));
                } else {
                    // for this model, we intentionally allow not using the recirc side
                    thisDD.RecircIsUsed = false;
                }

                thisDD.MaxAirVolFlowRate = NumArray(1);
                thisDD.MaxAirMassFlowRate = thisDD.MaxAirVolFlowRate * state.dataEnvrn->StdRhoAir;

                // Register component set data - one for OA and one for RA
                BranchNodeConnections::TestCompSet(state, CurrentModuleObject + ":OutdoorAir", thisDD.Name, AlphArray(4), AlphArray(3), "Air Nodes");
                if (thisDD.RecircIsUsed) {
                    BranchNodeConnections::TestCompSet(
                        state, CurrentModuleObject + ":RecirculatedAir", thisDD.Name, AlphArray(5), AlphArray(3), "Air Nodes");
                }

                thisDD.OAPerPersonMode = static_cast<PerPersonMode>(getEnumValue(modeStrings, AlphArray(7)));
                if (thisDD.OAPerPersonMode == PerPersonMode::Invalid) {
                    thisDD.OAPerPersonMode = PerPersonMode::ModeNotSet;
                }
                // checks on this are done later

                for (ADUNum = 1; ADUNum <= (int)state.dataDefineEquipment->AirDistUnit.size(); ++ADUNum) {
                    if (thisDD.OutletNodeNum == state.dataDefineEquipment->AirDistUnit(ADUNum).OutletNodeNum) {
                        state.dataDefineEquipment->AirDistUnit(ADUNum).InletNodeNum = thisDD.OAInletNodeNum;
                        state.dataDefineEquipment->AirDistUnit(ADUNum).InletNodeNum2 = thisDD.RecircAirInletNodeNum;
                        thisDD.ADUNum = ADUNum;
                    }
                }
                // one assumes if there isn't one assigned, it's an error?
                if (thisDD.ADUNum == 0) {
                    auto &thisObjType = damperTypeStrings[static_cast<int>(thisDD.DamperType)];
                    ShowSevereError(
                        state,
                        format("{}No matching List:Zone:AirTerminal for AirTerminal:DualDuct = [{},{}].", RoutineName, thisObjType, thisDD.Name));
                    ShowContinueError(state, format("...should have outlet node={}", state.dataLoopNodes->NodeID(thisDD.OutletNodeNum)));
                    ErrorsFound = true;
                } else {

                    // Fill the Zone Equipment data with the inlet node numbers of this unit.
                    for (int CtrlZone = 1; CtrlZone <= state.dataGlobal->NumOfZones; ++CtrlZone) {
                        auto &thisZoneEquipConfig = state.dataZoneEquip->ZoneEquipConfig(CtrlZone);
                        if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZone).IsControlled) {
                            continue;
                        }
                        for (SupAirIn = 1; SupAirIn <= thisZoneEquipConfig.NumInletNodes; ++SupAirIn) {
                            if (thisDD.OutletNodeNum == thisZoneEquipConfig.InletNode(SupAirIn)) {
                                if (thisDD.RecircIsUsed) {
                                    thisZoneEquipConfig.AirDistUnitCool(SupAirIn).InNode = thisDD.RecircAirInletNodeNum;
                                } else {
                                    thisZoneEquipConfig.AirDistUnitCool(SupAirIn).InNode = thisDD.OAInletNodeNum;
                                }
                                thisZoneEquipConfig.AirDistUnitHeat(SupAirIn).InNode = thisDD.OAInletNodeNum;
                                thisZoneEquipConfig.AirDistUnitCool(SupAirIn).OutNode = thisDD.OutletNodeNum;
                                thisZoneEquipConfig.AirDistUnitHeat(SupAirIn).OutNode = thisDD.OutletNodeNum;
                                state.dataDefineEquipment->AirDistUnit(thisDD.ADUNum).TermUnitSizingNum =
                                    thisZoneEquipConfig.AirDistUnitCool(SupAirIn).TermUnitSizingIndex;
                                state.dataDefineEquipment->AirDistUnit(thisDD.ADUNum).ZoneEqNum = CtrlZone;

                                thisDD.CtrlZoneNum = CtrlZone;
                                thisDD.CtrlZoneInNodeIndex = SupAirIn;
                            }
                        }
                    }
                }
                thisDD.OARequirementsPtr = Util::FindItemInList(AlphArray(6), state.dataSize->OARequirements);
                if (thisDD.OARequirementsPtr == 0) {
                    ShowSevereError(state, format("{} = {} not found.", cAlphaFields(6), AlphArray(6)));
                    ShowContinueError(state, format("Occurs in {} = {}", cCMO_DDVarVolOA, thisDD.Name));
                    ErrorsFound = true;
                } else {
                    thisDD.NoOAFlowInputFromUser = false;

                    // now fill design OA rate
                    thisDD.CalcOAOnlyMassFlow(state, DummyOAFlow, thisDD.DesignOAFlowRate);

                    if (thisDD.MaxAirVolFlowRate != DataSizing::AutoSize) {
                        BaseSizer::reportSizerOutput(
                            state, CurrentModuleObject, thisDD.Name, "Maximum Outdoor Air Flow Rate [m3/s]", thisDD.DesignOAFlowRate);

                        if (thisDD.RecircIsUsed) {
                            thisDD.DesignRecircFlowRate = thisDD.MaxAirVolFlowRate - thisDD.DesignOAFlowRate;
                            thisDD.DesignRecircFlowRate = max(0.0, thisDD.DesignRecircFlowRate);
                            BaseSizer::reportSizerOutput(
                                state, CurrentModuleObject, thisDD.Name, "Maximum Recirculated Air Flow Rate [m3/s]", thisDD.DesignRecircFlowRate);
                        } else {
                            if (thisDD.MaxAirVolFlowRate < thisDD.DesignOAFlowRate) {
                                ShowSevereError(state,
                                                format("The value {:.5R} in {}is lower than the outdoor air requirement.",
                                                       thisDD.MaxAirVolFlowRate,
                                                       cNumericFields(1)));
                                ShowContinueError(state, format("Occurs in {} = {}", cCMO_DDVarVolOA, thisDD.Name));
                                ShowContinueError(state, format("The design outdoor air requirement is {:.5R}", thisDD.DesignOAFlowRate));
                                ErrorsFound = true;
                            }
                        }
                    }
                }

                if (thisDD.OAPerPersonMode == PerPersonMode::ModeNotSet) {
                    DummyOAFlow = state.dataSize->OARequirements(thisDD.OARequirementsPtr).OAFlowPerPerson;
                    if ((DummyOAFlow == 0.0) && (lAlphaBlanks(7))) {       // no worries
                                                                           // do nothing, okay since no per person requirement involved
                    } else if ((DummyOAFlow > 0.0) && (lAlphaBlanks(7))) { // missing input
                        ShowSevereError(state, format("{} was blank.", cAlphaFields(7)));
                        ShowContinueError(state, format("Occurs in {} = {}", cCMO_DDVarVolOA, thisDD.Name));
                        ShowContinueError(state, R"(Valid choices are "CurrentOccupancy" or "DesignOccupancy")");
                        ErrorsFound = true;
                    } else if ((DummyOAFlow > 0.0) && !(lAlphaBlanks(7))) { // incorrect input
                        ShowSevereError(state, format("{} = {} not a valid key choice.", cAlphaFields(7), AlphArray(7)));
                        ShowContinueError(state, format("Occurs in {} = {}", cCMO_DDVarVolOA, thisDD.Name));
                        ShowContinueError(state, R"(Valid choices are "CurrentOccupancy" or "DesignOccupancy")");
                        ErrorsFound = true;
                    }
                }

                // Setup the Average damper Position output variable
                SetupOutputVariable(state,
                                    "Zone Air Terminal Outdoor Air Duct Damper Position",
                                    Constant::Units::None,
                                    thisDD.OADamperPosition,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    thisDD.Name);
                SetupOutputVariable(state,
                                    "Zone Air Terminal Recirculated Air Duct Damper Position",
                                    Constant::Units::None,
                                    thisDD.RecircAirDamperPosition,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    thisDD.Name);
                SetupOutputVariable(state,
                                    "Zone Air Terminal Outdoor Air Fraction",
                                    Constant::Units::None,
                                    thisDD.OAFraction,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    thisDD.Name);

            } // end Number of Damper Loop
        }

        if (ErrorsFound) {
            ShowFatalError(state, format("{}Errors found in input.  Preceding condition(s) cause termination.", RoutineName));
        }
    }

    void DualDuctAirTerminal::InitDualDuct(EnergyPlusData &state, bool const FirstHVACIteration)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard J. Liesen
        //       DATE WRITTEN   February 1998
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for  initializations of the Damper Components.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger events.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int HotInNode;
        int ColdInNode;
        int OAInNode; // Outdoor Air Inlet Node for VAV:OutdoorAir units
        int RAInNode; // Reciruclated Air Inlet Node for VAV:OutdoorAir units
        int OutNode;
        int Loop;          // Loop checking control variable
        Real64 PeopleFlow; // local sum variable, m3/s

        if (!state.dataDualDuct->ZoneEquipmentListChecked && state.dataZoneEquip->ZoneEquipInputsFilled) {
            state.dataDualDuct->ZoneEquipmentListChecked = true;
            // Check to see if there is a Air Distribution Unit on the Zone Equipment List
            for (Loop = 1; Loop <= state.dataDualDuct->NumDDAirTerminal; ++Loop) {
                if (this->ADUNum == 0) {
                    continue;
                }
                if (DataZoneEquipment::CheckZoneEquipmentList(
                        state, "ZONEHVAC:AIRDISTRIBUTIONUNIT", state.dataDefineEquipment->AirDistUnit(this->ADUNum).Name)) {
                    continue;
                }
                ShowSevereError(state,
                                format("InitDualDuct: ADU=[Air Distribution Unit,{}] is not on any ZoneHVAC:EquipmentList.",
                                       state.dataDefineEquipment->AirDistUnit(this->ADUNum).Name));
                if (this->DamperType == DualDuctDamper::ConstantVolume) {
                    ShowContinueError(state, format("...Dual Duct Damper=[{},{}] will not be simulated.", cCMO_DDConstantVolume, this->Name));
                } else if (this->DamperType == DualDuctDamper::VariableVolume) {
                    ShowContinueError(state, format("...Dual Duct Damper=[{},{}] will not be simulated.", cCMO_DDVariableVolume, this->Name));
                } else if (this->DamperType == DualDuctDamper::OutdoorAir) {
                    ShowContinueError(state, format("...Dual Duct Damper=[{},{}] will not be simulated.", cCMO_DDVarVolOA, this->Name));
                } else {
                    ShowContinueError(state, format("...Dual Duct Damper=[unknown/invalid,{}] will not be simulated.", this->Name));
                }
            }
        }

        if (!state.dataGlobal->SysSizingCalc && this->MySizeFlag) {
            this->SizeDualDuct(state);
            this->MySizeFlag = false;
        }

        // Do the Begin Environment initializations
        if (state.dataGlobal->BeginEnvrnFlag && this->MyEnvrnFlag) {

            if (this->DamperType == DualDuctDamper::ConstantVolume || this->DamperType == DualDuctDamper::VariableVolume) {
                OutNode = this->OutletNodeNum;
                HotInNode = this->HotAirInletNodeNum;
                ColdInNode = this->ColdAirInletNodeNum;
                state.dataLoopNodes->Node(OutNode).MassFlowRateMax = this->MaxAirVolFlowRate * state.dataEnvrn->StdRhoAir;
                if (this->DamperType == DualDuctDamper::ConstantVolume) {
                    state.dataLoopNodes->Node(OutNode).MassFlowRateMin = 0.0;
                } else if (this->DamperType == DualDuctDamper::VariableVolume) {
                    // get dual duct air terminal box minimum flow fraction value
                    if (this->zoneTurndownMinAirFracSched != nullptr) {
                        this->ZoneTurndownMinAirFrac = this->zoneTurndownMinAirFracSched->getMinVal(state);
                    } else {
                        this->ZoneTurndownMinAirFrac = 1.0;
                    }
                    state.dataLoopNodes->Node(OutNode).MassFlowRateMin =
                        state.dataLoopNodes->Node(OutNode).MassFlowRateMax * this->ZoneMinAirFracDes * this->ZoneTurndownMinAirFrac;
                } else {
                    state.dataLoopNodes->Node(OutNode).MassFlowRateMin = 0.0;
                }
                this->dd_airterminalHotAirInlet.AirMassFlowRateMax = state.dataLoopNodes->Node(OutNode).MassFlowRateMax;
                this->dd_airterminalColdAirInlet.AirMassFlowRateMax = state.dataLoopNodes->Node(OutNode).MassFlowRateMax;
                state.dataLoopNodes->Node(HotInNode).MassFlowRateMax = state.dataLoopNodes->Node(OutNode).MassFlowRateMax;
                state.dataLoopNodes->Node(ColdInNode).MassFlowRateMax = state.dataLoopNodes->Node(OutNode).MassFlowRateMax;
                state.dataLoopNodes->Node(HotInNode).MassFlowRateMin = 0.0;
                state.dataLoopNodes->Node(ColdInNode).MassFlowRateMin = 0.0;
                this->MyEnvrnFlag = false;

            } else if (this->DamperType == DualDuctDamper::OutdoorAir) {
                // Initialize for DualDuct:VAV:OutdoorAir
                OutNode = this->OutletNodeNum;
                OAInNode = this->OAInletNodeNum;
                if (this->RecircIsUsed) {
                    RAInNode = this->RecircAirInletNodeNum;
                }
                state.dataLoopNodes->Node(OutNode).MassFlowRateMax = this->MaxAirMassFlowRate;
                state.dataLoopNodes->Node(OutNode).MassFlowRateMin = 0.0;
                this->dd_airterminalOAInlet.AirMassFlowRateMax = this->DesignOAFlowRate * state.dataEnvrn->StdRhoAir;
                if (this->RecircIsUsed) {
                    this->dd_airterminalRecircAirInlet.AirMassFlowRateMax = this->MaxAirMassFlowRate - this->dd_airterminalOAInlet.AirMassFlowRateMax;
                    state.dataLoopNodes->Node(RAInNode).MassFlowRateMax = this->dd_airterminalRecircAirInlet.AirMassFlowRateMax;
                    state.dataLoopNodes->Node(RAInNode).MassFlowRateMin = 0.0;
                    this->dd_airterminalRecircAirInlet.AirMassFlowDiffMag = 1.0e-10 * this->dd_airterminalRecircAirInlet.AirMassFlowRateMax;
                }
                state.dataLoopNodes->Node(OAInNode).MassFlowRateMax = this->dd_airterminalOAInlet.AirMassFlowRateMax;
                state.dataLoopNodes->Node(OAInNode).MassFlowRateMin = 0.0;
                // figure per person by design level for the OA duct.
                PeopleFlow = 0.0;
                for (Loop = 1; Loop <= state.dataHeatBal->TotPeople; ++Loop) {
                    if (state.dataHeatBal->People(Loop).ZonePtr != this->CtrlZoneNum) {
                        continue;
                    }
                    DataSizing::OAFlowCalcMethod damperOAFlowMethod = state.dataSize->OARequirements(this->OARequirementsPtr).OAFlowMethod;
                    if (damperOAFlowMethod == DataSizing::OAFlowCalcMethod::PerPerson || damperOAFlowMethod == DataSizing::OAFlowCalcMethod::Sum ||
                        damperOAFlowMethod == DataSizing::OAFlowCalcMethod::Max) {
                        PeopleFlow +=
                            state.dataHeatBal->People(Loop).NumberOfPeople * state.dataSize->OARequirements(this->OARequirementsPtr).OAFlowPerPerson;
                    }
                }
                this->MyEnvrnFlag = false;
            }
        }

        if (!state.dataGlobal->BeginEnvrnFlag) {
            this->MyEnvrnFlag = true;
        }

        // Find air loop associated with this terminal unit
        if (this->MyAirLoopFlag) {
            if (this->AirLoopNum == 0) {
                if ((this->CtrlZoneNum > 0) && (this->CtrlZoneInNodeIndex > 0)) {
                    this->AirLoopNum = state.dataZoneEquip->ZoneEquipConfig(this->CtrlZoneNum).InletNodeAirLoopNum(this->CtrlZoneInNodeIndex);
                    state.dataDefineEquipment->AirDistUnit(this->ADUNum).AirLoopNum = this->AirLoopNum;
                    // Don't set MyAirLoopFlag to false yet because airloopnums might not be populated yet
                }
            } else {
                this->MyAirLoopFlag = false;
            }
        }

        // Initialize the Inlet Nodes of the Sys
        if (this->DamperType == DualDuctDamper::ConstantVolume || this->DamperType == DualDuctDamper::VariableVolume) {
            HotInNode = this->HotAirInletNodeNum;
            ColdInNode = this->ColdAirInletNodeNum;
            OutNode = this->OutletNodeNum;
        } else if (this->DamperType == DualDuctDamper::OutdoorAir) {
            OAInNode = this->OAInletNodeNum;
            if (this->RecircIsUsed) {
                RAInNode = this->RecircAirInletNodeNum;
            }
            OutNode = this->OutletNodeNum;
        }

        if (FirstHVACIteration) {
            //     CALL DisplayString('Init First HVAC Iteration {'//TRIM(  dd_airterminal(DDNum)%DamperName)//'}') !-For debugging - REMOVE
            // The first time through set the mass flow rate to the Max
            // Take care of the flow rates first. For Const Vol and VAV.
            if (this->DamperType == DualDuctDamper::ConstantVolume || this->DamperType == DualDuctDamper::VariableVolume) {
                auto &thisHotInNode = state.dataLoopNodes->Node(HotInNode);
                auto &thisColdInNode = state.dataLoopNodes->Node(ColdInNode);
                Real64 schedValue = this->availSched->getCurrentVal();
                if ((thisHotInNode.MassFlowRate > 0.0) && (schedValue > 0.0)) {
                    thisHotInNode.MassFlowRate = this->dd_airterminalHotAirInlet.AirMassFlowRateMax;
                } else {
                    thisHotInNode.MassFlowRate = 0.0;
                }
                if ((thisColdInNode.MassFlowRate > 0.0) && (schedValue > 0.0)) {
                    thisColdInNode.MassFlowRate = this->dd_airterminalColdAirInlet.AirMassFlowRateMax;
                } else {
                    thisColdInNode.MassFlowRate = 0.0;
                }
                // Next take care of the Max Avail Flow Rates
                if ((thisHotInNode.MassFlowRateMaxAvail > 0.0) && (schedValue > 0.0)) {
                    thisHotInNode.MassFlowRateMaxAvail = this->dd_airterminalHotAirInlet.AirMassFlowRateMax;
                } else {
                    thisHotInNode.MassFlowRateMaxAvail = 0.0;
                }
                if ((thisColdInNode.MassFlowRateMaxAvail > 0.0) && (schedValue > 0.0)) {
                    thisColdInNode.MassFlowRateMaxAvail = this->dd_airterminalColdAirInlet.AirMassFlowRateMax;
                } else {
                    thisColdInNode.MassFlowRateMaxAvail = 0.0;
                }
                // get current time step air terminal box turndown minimum flow fraction
                if (this->zoneTurndownMinAirFracSched != nullptr) {
                    this->ZoneTurndownMinAirFrac = this->zoneTurndownMinAirFracSched->getCurrentVal();
                } else {
                    this->ZoneTurndownMinAirFrac = 1.0;
                }
                // update to the current dual duct minimum air flow fraction
                this->ZoneMinAirFrac = this->ZoneMinAirFracDes * this->ZoneTurndownMinAirFrac;
                // The last item is to take care of the Min Avail Flow Rates
                if ((thisHotInNode.MassFlowRate > 0.0) && (schedValue > 0.0)) {
                    thisHotInNode.MassFlowRateMinAvail = this->dd_airterminalHotAirInlet.AirMassFlowRateMax * this->ZoneMinAirFrac;
                } else {
                    thisHotInNode.MassFlowRateMinAvail = 0.0;
                }
                if ((thisColdInNode.MassFlowRate > 0.0) && (schedValue > 0.0)) {
                    thisColdInNode.MassFlowRateMinAvail = this->dd_airterminalColdAirInlet.AirMassFlowRateMax * this->ZoneMinAirFrac;
                } else {
                    thisColdInNode.MassFlowRateMinAvail = 0.0;
                }

            } else if (this->DamperType == DualDuctDamper::OutdoorAir) {
                auto &thisOAInNode = state.dataLoopNodes->Node(OAInNode);
                Real64 schedValue = this->availSched->getCurrentVal();
                // The first time through set the mass flow rate to the Max for VAV:OutdoorAir
                if ((thisOAInNode.MassFlowRate > 0.0) && (schedValue > 0.0)) {
                    thisOAInNode.MassFlowRate = this->dd_airterminalOAInlet.AirMassFlowRateMax;
                } else {
                    thisOAInNode.MassFlowRate = 0.0;
                }
                if (this->RecircIsUsed) {
                    auto &thisRAInNode = state.dataLoopNodes->Node(RAInNode);
                    if ((thisRAInNode.MassFlowRate > 0.0) && (schedValue > 0.0)) {
                        thisRAInNode.MassFlowRate = this->dd_airterminalRecircAirInlet.AirMassFlowRateMax;
                    } else {
                        thisRAInNode.MassFlowRate = 0.0;
                    }
                    // clear flow history
                    this->dd_airterminalRecircAirInlet.AirMassFlowRateHist1 = 0.0;
                    this->dd_airterminalRecircAirInlet.AirMassFlowRateHist2 = 0.0;
                    this->dd_airterminalRecircAirInlet.AirMassFlowRateHist3 = 0.0;
                }
                // Next take care of the Max Avail Flow Rates
                if ((thisOAInNode.MassFlowRateMaxAvail > 0.0) && (schedValue > 0.0)) {
                    thisOAInNode.MassFlowRateMaxAvail = this->dd_airterminalOAInlet.AirMassFlowRateMax;
                } else {
                    thisOAInNode.MassFlowRateMaxAvail = 0.0;
                }
                if (this->RecircIsUsed) {
                    auto &thisRAInNode = state.dataLoopNodes->Node(RAInNode);
                    if ((thisRAInNode.MassFlowRateMaxAvail > 0.0) && (schedValue > 0.0)) {
                        thisRAInNode.MassFlowRateMaxAvail = this->dd_airterminalRecircAirInlet.AirMassFlowRateMax;
                    } else {
                        thisRAInNode.MassFlowRateMaxAvail = 0.0;
                    }
                }
                // The last item is to take care of the Min Avail Flow Rates. VAV:OutdoorAir
                thisOAInNode.MassFlowRateMinAvail = 0.0;
                if (this->RecircIsUsed) {
                    auto &thisRAInNode = state.dataLoopNodes->Node(RAInNode);
                    thisRAInNode.MassFlowRateMinAvail = 0.0;
                }
            }
        }

        // Initialize the Inlet Nodes of the Dampers for Const. Vol and VAV
        if (this->DamperType == DualDuctDamper::ConstantVolume || this->DamperType == DualDuctDamper::VariableVolume) {

            this->dd_airterminalHotAirInlet.AirMassFlowRateMaxAvail =
                min(state.dataLoopNodes->Node(OutNode).MassFlowRateMax, state.dataLoopNodes->Node(HotInNode).MassFlowRateMaxAvail);
            this->dd_airterminalHotAirInlet.AirMassFlowRateMinAvail =
                min(max(state.dataLoopNodes->Node(OutNode).MassFlowRateMin, state.dataLoopNodes->Node(HotInNode).MassFlowRateMinAvail),
                    state.dataLoopNodes->Node(HotInNode).MassFlowRateMaxAvail);

            this->dd_airterminalColdAirInlet.AirMassFlowRateMaxAvail =
                min(state.dataLoopNodes->Node(OutNode).MassFlowRateMax, state.dataLoopNodes->Node(ColdInNode).MassFlowRateMaxAvail);
            this->dd_airterminalColdAirInlet.AirMassFlowRateMinAvail =
                min(max(state.dataLoopNodes->Node(OutNode).MassFlowRateMin, state.dataLoopNodes->Node(ColdInNode).MassFlowRateMinAvail),
                    state.dataLoopNodes->Node(ColdInNode).MassFlowRateMaxAvail);

            // Do the following initializations (every time step): This should be the info from
            // the previous components outlets or the node data in this section.
            // Load the node data in this section for the component simulation
            this->dd_airterminalHotAirInlet.AirMassFlowRate = state.dataLoopNodes->Node(HotInNode).MassFlowRate;
            this->dd_airterminalHotAirInlet.AirTemp = state.dataLoopNodes->Node(HotInNode).Temp;
            this->dd_airterminalHotAirInlet.AirHumRat = state.dataLoopNodes->Node(HotInNode).HumRat;
            this->dd_airterminalHotAirInlet.AirEnthalpy = state.dataLoopNodes->Node(HotInNode).Enthalpy;
            this->dd_airterminalColdAirInlet.AirMassFlowRate = state.dataLoopNodes->Node(ColdInNode).MassFlowRate;
            this->dd_airterminalColdAirInlet.AirTemp = state.dataLoopNodes->Node(ColdInNode).Temp;
            this->dd_airterminalColdAirInlet.AirHumRat = state.dataLoopNodes->Node(ColdInNode).HumRat;
            this->dd_airterminalColdAirInlet.AirEnthalpy = state.dataLoopNodes->Node(ColdInNode).Enthalpy;

            // Initialize the Inlet Nodes of the Dampers for VAV:OutdoorAir
        } else if (this->DamperType == DualDuctDamper::OutdoorAir) {
            this->dd_airterminalOAInlet.AirMassFlowRateMaxAvail = state.dataLoopNodes->Node(OAInNode).MassFlowRateMaxAvail;
            this->dd_airterminalOAInlet.AirMassFlowRateMinAvail = state.dataLoopNodes->Node(OAInNode).MassFlowRateMinAvail;

            // Do the following initializations (every time step): This should be the info from
            // the previous components outlets or the node data in this section.
            // Load the node data in this section for the component simulation
            this->dd_airterminalOAInlet.AirMassFlowRate = state.dataLoopNodes->Node(OAInNode).MassFlowRate;
            this->dd_airterminalOAInlet.AirTemp = state.dataLoopNodes->Node(OAInNode).Temp;
            this->dd_airterminalOAInlet.AirHumRat = state.dataLoopNodes->Node(OAInNode).HumRat;
            this->dd_airterminalOAInlet.AirEnthalpy = state.dataLoopNodes->Node(OAInNode).Enthalpy;
            if (this->RecircIsUsed) {
                this->dd_airterminalRecircAirInlet.AirMassFlowRateMaxAvail = state.dataLoopNodes->Node(RAInNode).MassFlowRateMaxAvail;
                this->dd_airterminalRecircAirInlet.AirMassFlowRateMinAvail = state.dataLoopNodes->Node(RAInNode).MassFlowRateMinAvail;
                this->dd_airterminalRecircAirInlet.AirMassFlowRate = state.dataLoopNodes->Node(RAInNode).MassFlowRate;
                this->dd_airterminalRecircAirInlet.AirTemp = state.dataLoopNodes->Node(RAInNode).Temp;
                this->dd_airterminalRecircAirInlet.AirHumRat = state.dataLoopNodes->Node(RAInNode).HumRat;
                this->dd_airterminalRecircAirInlet.AirEnthalpy = state.dataLoopNodes->Node(RAInNode).Enthalpy;
            }
        }
    }

    void DualDuctAirTerminal::SizeDualDuct(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   January 2002
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing Dual Duct air terminal units for which flow rates have not been
        // specified in the input.

        // METHODOLOGY EMPLOYED:
        // Obtains flow rates from the zone or system sizing arrays.

        if (this->MaxAirVolFlowRate == DataSizing::AutoSize) {

            if ((state.dataSize->CurZoneEqNum > 0) && (state.dataSize->CurTermUnitSizingNum > 0)) {
                std::string_view damperType = cmoNameArray[static_cast<int>(this->DamperType)];
                // ideally we'd just use a string_view, but there are multiple calls that are not yet set up for string_view, and they pass a
                //  reference, so we just create a string version for now.  When we do more string_view cleanup, we'll end up searching on
                //  std::string() to find usages of it, so this should show up and get cleaned up then.  Regardless, this is only called at
                //  program initialization, so it is not a runtime issue.
                std::string damperTypeAsString = std::string(damperType);
                CheckZoneSizing(state, damperTypeAsString, this->Name);
                this->MaxAirVolFlowRate = max(state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesCoolVolFlow,
                                              state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesHeatVolFlow);
                if (this->DamperType == DualDuctDamper::OutdoorAir) {
                    if (this->RecircIsUsed) {
                        this->DesignRecircFlowRate =
                            max(state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesCoolVolFlow,
                                state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesHeatVolFlow);
                        this->MaxAirVolFlowRate = this->DesignRecircFlowRate + this->DesignOAFlowRate;
                    } else {
                        this->MaxAirVolFlowRate = this->DesignOAFlowRate;
                        this->DesignRecircFlowRate = 0.0;
                    }
                    this->MaxAirMassFlowRate = this->MaxAirVolFlowRate * state.dataEnvrn->StdRhoAir;
                }

                if (this->MaxAirVolFlowRate < HVAC::SmallAirVolFlow) {
                    this->MaxAirVolFlowRate = 0.0;
                    this->MaxAirMassFlowRate = 0.0;
                    this->DesignOAFlowRate = 0.0;
                    this->DesignRecircFlowRate = 0.0;
                }
                BaseSizer::reportSizerOutput(state, damperTypeAsString, this->Name, "Maximum Air Flow Rate [m3/s]", this->MaxAirVolFlowRate);
                if (this->DamperType == DualDuctDamper::OutdoorAir) {
                    BaseSizer::reportSizerOutput(
                        state, damperTypeAsString, this->Name, "Maximum Outdoor Air Flow Rate [m3/s]", this->DesignOAFlowRate);
                    if (this->RecircIsUsed) {
                        BaseSizer::reportSizerOutput(
                            state, damperTypeAsString, this->Name, "Maximum Recirculated Air Flow Rate [m3/s]", this->DesignRecircFlowRate);
                    }
                }
            }
        }
    }

    void DualDuctAirTerminal::SimDualDuctConstVol(EnergyPlusData &state, int const ZoneNum, int const ZoneNodeNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard J. Liesen
        //       DATE WRITTEN   Jan 2000
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine simulates the simple mixing damper.

        // METHODOLOGY EMPLOYED:
        // There is method to this madness.

        // Using/Aliasing
        using namespace DataZoneEnergyDemands;
        using HVAC::SmallTempDiff;
        using Psychrometrics::PsyCpAirFnW;
        using Psychrometrics::PsyTdbFnHW;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 MassFlow;    // [kg/sec]   Total Mass Flow Rate from Hot & Cold Inlets
        Real64 HumRat;      // [Kg Moisture / Kg dry air]
        Real64 Enthalpy;    // [Watts]
        Real64 Temperature; // [C]
        Real64 QTotLoad;    // [W]
        Real64 QZnReq;      // [W]
        Real64 CpAirZn;
        Real64 CpAirSysHot;
        Real64 CpAirSysCold;

        // Get the calculated load from the Heat Balance from ZoneSysEnergyDemand
        QTotLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputRequired;
        // Need the design MassFlowRate for calculations
        if (this->availSched->getCurrentVal() > 0.0) {
            MassFlow = this->dd_airterminalHotAirInlet.AirMassFlowRateMaxAvail / 2.0 + this->dd_airterminalColdAirInlet.AirMassFlowRateMaxAvail / 2.0;
        } else {
            MassFlow = 0.0;
        }
        // If there is massflow then need to provide the correct amount of total
        //  required zone energy
        if (MassFlow > HVAC::SmallMassFlow) {
            CpAirZn = PsyCpAirFnW(state.dataLoopNodes->Node(ZoneNodeNum).HumRat);
            QZnReq = QTotLoad + MassFlow * CpAirZn * state.dataLoopNodes->Node(ZoneNodeNum).Temp;
            // If the enthalpy is the same for the hot and cold duct then there would be a
            //  divide by zero so for heating or cooling set the damper to one max flow
            //  or the other.
            if (std::abs(this->dd_airterminalColdAirInlet.AirTemp - this->dd_airterminalHotAirInlet.AirTemp) > SmallTempDiff) {
                // CpAirSysHot = PsyCpAirFnWTdb(dd_airterminalHotAirInlet(DDNum)%AirHumRat,dd_airterminalHotAirInlet(DDNum)%AirTemp)
                // CpAirSysCold= PsyCpAirFnWTdb(dd_airterminalColdAirInlet(DDNum)%AirHumRat,dd_airterminalColdAirInlet(DDNum)%AirTemp)
                CpAirSysHot = CpAirZn;
                CpAirSysCold = CpAirZn;
                // Determine the Cold Air Mass Flow Rate
                this->dd_airterminalColdAirInlet.AirMassFlowRate =
                    (QZnReq - MassFlow * CpAirSysHot * this->dd_airterminalHotAirInlet.AirTemp) /
                    (CpAirSysCold * this->dd_airterminalColdAirInlet.AirTemp - CpAirSysHot * this->dd_airterminalHotAirInlet.AirTemp);
            } else if ((QTotLoad > 0.0) && (this->dd_airterminalHotAirInlet.AirMassFlowRate > 0.0)) {
                this->dd_airterminalColdAirInlet.AirMassFlowRate = 0.0;
            } else {
                this->dd_airterminalColdAirInlet.AirMassFlowRate = MassFlow;
            }
            // Check to make sure that the calculated flow is not greater than the available flows
            if (this->dd_airterminalColdAirInlet.AirMassFlowRate > this->dd_airterminalColdAirInlet.AirMassFlowRateMaxAvail) {
                this->dd_airterminalColdAirInlet.AirMassFlowRate = this->dd_airterminalColdAirInlet.AirMassFlowRateMaxAvail;
            } else if (this->dd_airterminalColdAirInlet.AirMassFlowRate < this->dd_airterminalColdAirInlet.AirMassFlowRateMinAvail) {
                this->dd_airterminalColdAirInlet.AirMassFlowRate = this->dd_airterminalColdAirInlet.AirMassFlowRateMinAvail;
            }
            // Using Mass Continuity to determine the other duct flow quantity
            this->dd_airterminalHotAirInlet.AirMassFlowRate = MassFlow - this->dd_airterminalColdAirInlet.AirMassFlowRate;
            if (this->dd_airterminalHotAirInlet.AirMassFlowRate > this->dd_airterminalHotAirInlet.AirMassFlowRateMaxAvail) {
                this->dd_airterminalHotAirInlet.AirMassFlowRate = this->dd_airterminalHotAirInlet.AirMassFlowRateMaxAvail;
            } else if (this->dd_airterminalHotAirInlet.AirMassFlowRate < this->dd_airterminalHotAirInlet.AirMassFlowRateMinAvail) {
                this->dd_airterminalHotAirInlet.AirMassFlowRate = this->dd_airterminalHotAirInlet.AirMassFlowRateMinAvail;
            }
            MassFlow = this->dd_airterminalColdAirInlet.AirMassFlowRate + this->dd_airterminalHotAirInlet.AirMassFlowRate;
        } else {
            // System is Off set massflow to 0.0
            MassFlow = 0.0;
        }
        if (MassFlow > HVAC::SmallMassFlow) {
            // After flows are calculated then calculate the mixed air flow properties.
            HumRat = (this->dd_airterminalHotAirInlet.AirHumRat * this->dd_airterminalHotAirInlet.AirMassFlowRate +
                      this->dd_airterminalColdAirInlet.AirHumRat * this->dd_airterminalColdAirInlet.AirMassFlowRate) /
                     MassFlow;
            Enthalpy = (this->dd_airterminalHotAirInlet.AirEnthalpy * this->dd_airterminalHotAirInlet.AirMassFlowRate +
                        this->dd_airterminalColdAirInlet.AirEnthalpy * this->dd_airterminalColdAirInlet.AirMassFlowRate) /
                       MassFlow;

            // If there is no air flow than calculate the No Flow conditions
        } else {
            this->dd_airterminalColdAirInlet.AirMassFlowRate = 0.0;
            this->dd_airterminalHotAirInlet.AirMassFlowRate = 0.0;
            HumRat = (this->dd_airterminalHotAirInlet.AirHumRat + this->dd_airterminalColdAirInlet.AirHumRat) / 2.0;
            Enthalpy = (this->dd_airterminalHotAirInlet.AirEnthalpy + this->dd_airterminalColdAirInlet.AirEnthalpy) / 2.0;
        }
        Temperature = PsyTdbFnHW(Enthalpy, HumRat);

        // Load all properties in the damper outlet
        this->dd_airterminalOutlet.AirTemp = Temperature;
        this->dd_airterminalOutlet.AirHumRat = HumRat;
        this->dd_airterminalOutlet.AirMassFlowRate = MassFlow;
        this->dd_airterminalOutlet.AirMassFlowRateMaxAvail = MassFlow;
        this->dd_airterminalOutlet.AirMassFlowRateMinAvail =
            min(this->dd_airterminalHotAirInlet.AirMassFlowRateMinAvail, this->dd_airterminalColdAirInlet.AirMassFlowRateMinAvail);
        this->dd_airterminalOutlet.AirEnthalpy = Enthalpy;

        // Calculate the hot and cold damper position in %
        if ((this->dd_airterminalHotAirInlet.AirMassFlowRateMax == 0.0) || (this->dd_airterminalColdAirInlet.AirMassFlowRateMax == 0.0)) {
            this->ColdAirDamperPosition = 0.0;
            this->HotAirDamperPosition = 0.0;
        } else {
            this->ColdAirDamperPosition = this->dd_airterminalColdAirInlet.AirMassFlowRate / this->dd_airterminalColdAirInlet.AirMassFlowRateMax;
            this->HotAirDamperPosition = this->dd_airterminalHotAirInlet.AirMassFlowRate / this->dd_airterminalHotAirInlet.AirMassFlowRateMax;
        }
    }

    void DualDuctAirTerminal::SimDualDuctVarVol(EnergyPlusData &state, int const ZoneNum, int const ZoneNodeNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard J. Liesen
        //       DATE WRITTEN   Jan 2000
        //       MODIFIED       na
        //                      TH 3/2012: added supply air flow adjustment based on zone maximum outdoor
        //                                 air fraction - a TRACE feature
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine simulates the simple mixing damper.

        // METHODOLOGY EMPLOYED:
        // There is method to this madness.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 MassFlow;    // [kg/sec]   Total Mass Flow Rate from Hot & Cold Inlets
        Real64 HumRat;      // [Kg Moisture / Kg dry air]
        Real64 Enthalpy;    // [Watts]
        Real64 Temperature; // [C]
        Real64 QTotLoad;    // [W]
        Real64 QZnReq;      // [W]
        Real64 CpAirZn;     // specific heat of zone air
        Real64 CpAirSysHot;
        Real64 CpAirSysCold;
        Real64 MassFlowBasedOnOA; // Supply air flow rate based on minimum OA requirement
        Real64 AirLoopOAFrac;     // fraction of outdoor air entering air loop outside air system

        // The calculated load from the Heat Balance
        QTotLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputRequired;
        // Calculate all of the required Cp's
        CpAirZn = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(ZoneNodeNum).HumRat);
        // CpAirSysHot = PsyCpAirFnW(DamperHotAirInlet(DDNum)%AirHumRat,DamperHotAirInlet(DDNum)%AirTemp)
        // CpAirSysCold= PsyCpAirFnW(DamperColdAirInlet(DDNum)%AirHumRat,DamperColdAirInlet(DDNum)%AirTemp)
        CpAirSysHot = CpAirZn;
        CpAirSysCold = CpAirZn;

        // calculate supply air flow rate based on user specified OA requirement
        this->CalcOAMassFlow(state, MassFlowBasedOnOA, AirLoopOAFrac);

        // Then depending on if the Load is for heating or cooling it is handled differently.  First
        // the massflow rate of either heating or cooling is determined to meet the entire load.  Then
        // if the massflow is below the minimum or greater than the Max it is set to either the Min
        // or the Max as specified for the VAV model.
        if (this->availSched->getCurrentVal() == 0.0) {
            // System is Off set massflow to 0.0
            MassFlow = 0.0;

        } else if ((QTotLoad > 0.0) && (this->dd_airterminalHotAirInlet.AirMassFlowRateMaxAvail > 0.0)) {
            // Then heating is needed
            // Next check for the denominator equal to zero
            if (std::abs((CpAirSysHot * this->dd_airterminalHotAirInlet.AirTemp) - (CpAirZn * state.dataLoopNodes->Node(ZoneNodeNum).Temp)) /
                    CpAirZn >
                HVAC::SmallTempDiff) {
                MassFlow = QTotLoad / (CpAirSysHot * this->dd_airterminalHotAirInlet.AirTemp - CpAirZn * state.dataLoopNodes->Node(ZoneNodeNum).Temp);
            } else {
                // If denominator tends to zero then mass flow would go to infinity thus set to the max for this iteration
                MassFlow = this->dd_airterminalHotAirInlet.AirMassFlowRateMaxAvail;
            }
            // Check to see if the flow is < the Min or > the Max air Fraction to the zone; then set to min or max
            if (MassFlow <= (this->dd_airterminalHotAirInlet.AirMassFlowRateMax * this->ZoneMinAirFrac)) {
                MassFlow = this->dd_airterminalHotAirInlet.AirMassFlowRateMax * this->ZoneMinAirFrac;
                MassFlow = max(MassFlow, this->dd_airterminalHotAirInlet.AirMassFlowRateMinAvail);
            } else if (MassFlow >= this->dd_airterminalHotAirInlet.AirMassFlowRateMaxAvail) {
                MassFlow = this->dd_airterminalHotAirInlet.AirMassFlowRateMaxAvail;
            }

            // Apply the zone maximum outdoor air fraction for VAV boxes - a TRACE feature
            if (state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).SupplyAirAdjustFactor > 1.0) {
                MassFlow *= state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).SupplyAirAdjustFactor;
            }

            MassFlow = max(MassFlow, MassFlowBasedOnOA);
            MassFlow = min(MassFlow, this->dd_airterminalHotAirInlet.AirMassFlowRateMaxAvail);

        } else if ((QTotLoad < 0.0) && (this->dd_airterminalColdAirInlet.AirMassFlowRateMaxAvail > 0.0)) {
            // Then cooling is required
            // Next check for the denominator equal to zero
            if (std::abs((CpAirSysCold * this->dd_airterminalColdAirInlet.AirTemp) - (CpAirZn * state.dataLoopNodes->Node(ZoneNodeNum).Temp)) /
                    CpAirZn >
                HVAC::SmallTempDiff) {
                MassFlow =
                    QTotLoad / (CpAirSysCold * this->dd_airterminalColdAirInlet.AirTemp - CpAirZn * state.dataLoopNodes->Node(ZoneNodeNum).Temp);
            } else {
                // If denominator tends to zero then mass flow would go to infinity thus set to the max for this iteration
                MassFlow = this->dd_airterminalColdAirInlet.AirMassFlowRateMaxAvail;
            }

            // Check to see if the flow is < the Min or > the Max air Fraction to the zone; then set to min or max
            if ((MassFlow <= (this->dd_airterminalColdAirInlet.AirMassFlowRateMax * this->ZoneMinAirFrac)) && (MassFlow >= 0.0)) {
                MassFlow = this->dd_airterminalColdAirInlet.AirMassFlowRateMax * this->ZoneMinAirFrac;
                MassFlow = max(MassFlow, this->dd_airterminalColdAirInlet.AirMassFlowRateMinAvail);
            } else if (MassFlow < 0.0) {
                MassFlow = this->dd_airterminalColdAirInlet.AirMassFlowRateMaxAvail;
            } else if (MassFlow >= this->dd_airterminalColdAirInlet.AirMassFlowRateMaxAvail) {
                MassFlow = this->dd_airterminalColdAirInlet.AirMassFlowRateMaxAvail;
            }

            // Apply the zone maximum outdoor air fraction for VAV boxes - a TRACE feature
            if (state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).SupplyAirAdjustFactor > 1.0) {
                MassFlow *= state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).SupplyAirAdjustFactor;
            }

            MassFlow = max(MassFlow, MassFlowBasedOnOA);
            MassFlow = min(MassFlow, this->dd_airterminalColdAirInlet.AirMassFlowRateMaxAvail);

        } else if ((this->dd_airterminalHotAirInlet.AirMassFlowRateMaxAvail > 0.0) ||
                   (this->dd_airterminalColdAirInlet.AirMassFlowRateMaxAvail > 0.0)) {
            // No Load on Zone set to mixed condition
            MassFlow = (this->dd_airterminalHotAirInlet.AirMassFlowRateMax / 2.0) * this->ZoneMinAirFrac +
                       this->dd_airterminalColdAirInlet.AirMassFlowRateMax / 2.0 * this->ZoneMinAirFrac;

            // Apply the zone maximum outdoor air fraction for VAV boxes - a TRACE feature
            if (state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).SupplyAirAdjustFactor > 1.0) {
                MassFlow *= state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).SupplyAirAdjustFactor;
            }

            MassFlow = max(MassFlow, MassFlowBasedOnOA);
            MassFlow =
                min(MassFlow, (this->dd_airterminalHotAirInlet.AirMassFlowRateMaxAvail + this->dd_airterminalColdAirInlet.AirMassFlowRateMaxAvail));

        } else {
            // System is Off set massflow to 0.0
            MassFlow = 0.0;
        }

        // Now the massflow for heating or cooling has been determined and if the massflow was reset to the
        // Min or Max we will need to mix the hot and cold deck to meet the zone load.  Knowing the enthalpy
        // of the zone and the hot and cold air flows we can determine exactly by using the Energy and Continuity
        // Eqns.  Of course we have to make sure that we are within the Min and Max flow conditions.
        if (MassFlow > HVAC::SmallMassFlow) {
            // Determine the enthalpy required from Zone enthalpy and the zone load.
            QZnReq = QTotLoad + MassFlow * CpAirZn * state.dataLoopNodes->Node(ZoneNodeNum).Temp;
            // Using the known enthalpies the cold air inlet mass flow is determined.  If the enthalpy of the hot and cold
            // air streams are equal the IF-Then block handles that condition.
            if (std::abs(this->dd_airterminalColdAirInlet.AirTemp - this->dd_airterminalHotAirInlet.AirTemp) > HVAC::SmallTempDiff) {
                // Calculate the Cold air mass flow rate
                this->dd_airterminalColdAirInlet.AirMassFlowRate =
                    (QZnReq - MassFlow * CpAirSysHot * this->dd_airterminalHotAirInlet.AirTemp) /
                    (CpAirSysCold * this->dd_airterminalColdAirInlet.AirTemp - CpAirSysHot * this->dd_airterminalHotAirInlet.AirTemp);
            } else if ((QTotLoad > 0.0) && (this->dd_airterminalHotAirInlet.AirMassFlowRate > 0.0)) {
                this->dd_airterminalColdAirInlet.AirMassFlowRate = 0.0;
            } else {
                this->dd_airterminalColdAirInlet.AirMassFlowRate = MassFlow;
            }

            // Need to make sure that the flows are within limits
            if (this->dd_airterminalColdAirInlet.AirMassFlowRate > this->dd_airterminalColdAirInlet.AirMassFlowRateMaxAvail) {
                this->dd_airterminalColdAirInlet.AirMassFlowRate = this->dd_airterminalColdAirInlet.AirMassFlowRateMaxAvail;

                // These are shutoff boxes for either the hot or the cold, therfore one side or other can = 0.0
            } else if (this->dd_airterminalColdAirInlet.AirMassFlowRate < 0.0) {
                this->dd_airterminalColdAirInlet.AirMassFlowRate = 0.0;
            } else if (this->dd_airterminalColdAirInlet.AirMassFlowRate > MassFlow) {
                this->dd_airterminalColdAirInlet.AirMassFlowRate = MassFlow;
            }
            // Using Mass Continuity to determine the other duct flow quantity
            this->dd_airterminalHotAirInlet.AirMassFlowRate = MassFlow - this->dd_airterminalColdAirInlet.AirMassFlowRate;

            if (this->dd_airterminalHotAirInlet.AirMassFlowRate < DualDuctMassFlowSetToler) {
                this->dd_airterminalHotAirInlet.AirMassFlowRate = 0.0;
                this->dd_airterminalColdAirInlet.AirMassFlowRate = MassFlow;
            } else if (this->dd_airterminalColdAirInlet.AirMassFlowRate < DualDuctMassFlowSetToler) {
                this->dd_airterminalColdAirInlet.AirMassFlowRate = 0.0;
                this->dd_airterminalHotAirInlet.AirMassFlowRate = MassFlow;
            }

            // After the flow rates are determined the properties are calculated.
            HumRat = (this->dd_airterminalHotAirInlet.AirHumRat * this->dd_airterminalHotAirInlet.AirMassFlowRate +
                      this->dd_airterminalColdAirInlet.AirHumRat * this->dd_airterminalColdAirInlet.AirMassFlowRate) /
                     MassFlow;
            Enthalpy = (this->dd_airterminalHotAirInlet.AirEnthalpy * this->dd_airterminalHotAirInlet.AirMassFlowRate +
                        this->dd_airterminalColdAirInlet.AirEnthalpy * this->dd_airterminalColdAirInlet.AirMassFlowRate) /
                       MassFlow;

            // IF the system is OFF the properties are calculated for this special case.
        } else {
            this->dd_airterminalColdAirInlet.AirMassFlowRate = 0.0;
            this->dd_airterminalHotAirInlet.AirMassFlowRate = 0.0;
            HumRat = (this->dd_airterminalHotAirInlet.AirHumRat + this->dd_airterminalColdAirInlet.AirHumRat) / 2.0;
            Enthalpy = (this->dd_airterminalHotAirInlet.AirEnthalpy + this->dd_airterminalColdAirInlet.AirEnthalpy) / 2.0;
        }
        Temperature = Psychrometrics::PsyTdbFnHW(Enthalpy, HumRat);

        this->dd_airterminalOutlet.AirTemp = Temperature;
        this->dd_airterminalOutlet.AirHumRat = HumRat;
        this->dd_airterminalOutlet.AirMassFlowRate = MassFlow;
        this->dd_airterminalOutlet.AirMassFlowRateMaxAvail = MassFlow;
        this->dd_airterminalOutlet.AirMassFlowRateMinAvail = this->ZoneMinAirFrac * this->dd_airterminalHotAirInlet.AirMassFlowRateMax;
        this->dd_airterminalOutlet.AirEnthalpy = Enthalpy;

        // Calculate the hot and cold damper position in %
        if ((this->dd_airterminalHotAirInlet.AirMassFlowRateMax == 0.0) || (this->dd_airterminalColdAirInlet.AirMassFlowRateMax == 0.0)) {
            this->ColdAirDamperPosition = 0.0;
            this->HotAirDamperPosition = 0.0;
        } else {
            this->ColdAirDamperPosition = this->dd_airterminalColdAirInlet.AirMassFlowRate / this->dd_airterminalColdAirInlet.AirMassFlowRateMax;
            this->HotAirDamperPosition = this->dd_airterminalHotAirInlet.AirMassFlowRate / this->dd_airterminalHotAirInlet.AirMassFlowRateMax;
        }
    }

    void DualDuctAirTerminal::SimDualDuctVAVOutdoorAir(EnergyPlusData &state, int const ZoneNum, int const ZoneNodeNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Clayton Miller
        //       DATE WRITTEN   Aug 2010
        //       MODIFIED       B. Griffith, Dec 2010, major rework
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Designed to accommodate for systems with outdoor air (OA) and recirculated air (RA)
        // as two separate air streams to controlled at the zone level in a dual duct system.

        // METHODOLOGY EMPLOYED:
        // The terminal unit is be designed to set the airflow of the of the OA stream at the zone
        // level based on the zonal ventilation requirements and the RA stream flowrate of recirculated
        // cooling air stream in order to meet the remaining thermal load.
        // If the zone calls for cooling but the inlet air temperature is too warm, recirc side set to zero
        // if the zone calls for heating and the inlet air is warm enough, modulate damper to meet load
        // if the zone calls for heating and the inlet air is too cold, zero flow (will not control sans reheat)

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 MassFlowMax;     // [kg/sec]   Maximum Mass Flow Rate from OA and Recirc Inlets
        Real64 HumRat;          // [Kg Moisture / Kg dry air]
        Real64 Enthalpy;        // [Watts]
        Real64 Temperature;     // [C]
        Real64 QTotLoadRemain;  // [W]
        Real64 QtoHeatSPRemain; // [W]
        Real64 QtoCoolSPRemain; // [W]
        //  REAL(r64) :: QTotRemainAdjust  ! [W]
        Real64 QtoHeatSPRemainAdjust; // [W]
        Real64 QtoCoolSPRemainAdjust; // [W]
        Real64 QOALoadToHeatSP;       // [W]
        Real64 QOALoadToCoolSP;       // [W]
        Real64 QOALoad;               // Amount of cooling load accounted for by OA Stream [W]
        Real64 QRALoad;               // Amount of cooling load accounted for by Recirc Stream [W]
        Real64 CpAirZn;               // specific heat of zone air
        Real64 CpAirSysOA;            // specific heat of outdoor air
        Real64 CpAirSysRA;            // specific heat of recirculated air
        Real64 OAMassFlow;            // Supply air flow rate based on minimum OA requirement - for printing
        Real64 TotMassFlow;           // [kg/sec]   Total Mass Flow Rate from OA and Recirc Inlets
        int OAInletNodeNum;
        int RecircInletNodeNum;

        OAInletNodeNum = this->OAInletNodeNum;
        if (this->RecircIsUsed) {
            RecircInletNodeNum = this->RecircAirInletNodeNum;
        }
        // Calculate required ventilation air flow rate based on user specified OA requirement
        this->CalcOAOnlyMassFlow(state, OAMassFlow);

        // The calculated load from the Heat Balance, adjusted for any equipment sequenced before terminal
        QTotLoadRemain = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputRequired;
        QtoHeatSPRemain = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToHeatSP;
        QtoCoolSPRemain = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToCoolSP;

        // Calculate all of the required Cp's
        CpAirZn = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(ZoneNodeNum).HumRat);
        CpAirSysOA = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(OAInletNodeNum).HumRat);
        if (this->RecircIsUsed) {
            CpAirSysRA = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(RecircInletNodeNum).HumRat);
        }

        // Set the OA Damper to the calculated ventilation flow rate
        this->dd_airterminalOAInlet.AirMassFlowRate = OAMassFlow;
        // Need to make sure that the OA flows are within limits
        if (this->dd_airterminalOAInlet.AirMassFlowRate > this->dd_airterminalOAInlet.AirMassFlowRateMaxAvail) {
            this->dd_airterminalOAInlet.AirMassFlowRate = this->dd_airterminalOAInlet.AirMassFlowRateMaxAvail;
        } else if (this->dd_airterminalOAInlet.AirMassFlowRate < 0.0) {
            this->dd_airterminalOAInlet.AirMassFlowRate = 0.0;
        }

        //..Find the amount of load that the OAMassFlow accounted for
        if (std::abs((CpAirSysOA * this->dd_airterminalOAInlet.AirTemp) - (CpAirZn * state.dataLoopNodes->Node(ZoneNodeNum).Temp)) / CpAirZn >
            HVAC::SmallTempDiff) {
            QOALoad = this->dd_airterminalOAInlet.AirMassFlowRate *
                      (CpAirSysOA * this->dd_airterminalOAInlet.AirTemp - CpAirZn * state.dataLoopNodes->Node(ZoneNodeNum).Temp);

            auto const &zoneTstatSetpt = state.dataHeatBalFanSys->zoneTstatSetpts(ZoneNum);
            QOALoadToHeatSP =
                this->dd_airterminalOAInlet.AirMassFlowRate * (CpAirSysOA * this->dd_airterminalOAInlet.AirTemp - CpAirZn * zoneTstatSetpt.setptLo);
            QOALoadToCoolSP =
                this->dd_airterminalOAInlet.AirMassFlowRate * (CpAirSysOA * this->dd_airterminalOAInlet.AirTemp - CpAirZn * zoneTstatSetpt.setptHi);

        } else {
            QOALoad = 0.0;
            QOALoadToHeatSP = 0.0;
            QOALoadToCoolSP = 0.0;
        }

        if (this->RecircIsUsed) {

            // correct load for recirc side to account for impact of OA side
            // QTotRemainAdjust      = QTotLoadRemain  - QOALoad
            QtoHeatSPRemainAdjust = QtoHeatSPRemain - QOALoadToHeatSP;
            QtoCoolSPRemainAdjust = QtoCoolSPRemain - QOALoadToCoolSP;

            if (QtoCoolSPRemainAdjust < 0.0) {
                QRALoad = QtoCoolSPRemainAdjust;
            } else if (QtoHeatSPRemainAdjust > 0.0) {
                QRALoad = QtoHeatSPRemainAdjust;
            } else {
                QRALoad = 0.0;
            }

            if (QRALoad < 0.0) {                                                                                         // cooling
                if ((this->dd_airterminalRecircAirInlet.AirTemp - state.dataLoopNodes->Node(ZoneNodeNum).Temp) < -0.5) { // can cool
                    //  Find the Mass Flow Rate of the RA Stream needed to meet the zone cooling load
                    if (std::abs((CpAirSysRA * this->dd_airterminalRecircAirInlet.AirTemp) -
                                 (CpAirZn * state.dataLoopNodes->Node(ZoneNodeNum).Temp)) /
                            CpAirZn >
                        HVAC::SmallTempDiff) {
                        this->dd_airterminalRecircAirInlet.AirMassFlowRate = QRALoad / (CpAirSysRA * this->dd_airterminalRecircAirInlet.AirTemp -
                                                                                        CpAirZn * state.dataLoopNodes->Node(ZoneNodeNum).Temp);
                    }
                } else {
                    this->dd_airterminalRecircAirInlet.AirMassFlowRate = 0.0;
                }

            } else { // heating or none needed.
                this->dd_airterminalRecircAirInlet.AirMassFlowRate = 0.0;
            }

            // Need to make sure that the RA flows are within limits
            if (this->dd_airterminalRecircAirInlet.AirMassFlowRate > this->dd_airterminalRecircAirInlet.AirMassFlowRateMaxAvail) {
                this->dd_airterminalRecircAirInlet.AirMassFlowRate = this->dd_airterminalRecircAirInlet.AirMassFlowRateMaxAvail;
                // These are shutoff boxes for either the hot or the cold, therfore one side or other can = 0.0
            } else if (this->dd_airterminalRecircAirInlet.AirMassFlowRate < 0.0) {
                this->dd_airterminalRecircAirInlet.AirMassFlowRate = 0.0;
            }

        } else {
            this->dd_airterminalRecircAirInlet.AirMassFlowRate = 0.0;
            this->dd_airterminalRecircAirInlet.AirMassFlowRateMaxAvail = 0.0;
        } // recirc used

        // look for bang-bang condition: flow rate oscillating between 2 values during the air loop / zone
        // equipment iteration. If detected, set flow rate to previous value.
        if (((std::abs(this->dd_airterminalRecircAirInlet.AirMassFlowRate - this->dd_airterminalRecircAirInlet.AirMassFlowRateHist2) <
              this->dd_airterminalRecircAirInlet.AirMassFlowDiffMag) ||
             (std::abs(this->dd_airterminalRecircAirInlet.AirMassFlowRate - this->dd_airterminalRecircAirInlet.AirMassFlowRateHist3) <
              this->dd_airterminalRecircAirInlet.AirMassFlowDiffMag)) &&
            (std::abs(this->dd_airterminalRecircAirInlet.AirMassFlowRate - this->dd_airterminalRecircAirInlet.AirMassFlowRateHist1) >=
             this->dd_airterminalRecircAirInlet.AirMassFlowDiffMag)) {
            if (this->dd_airterminalRecircAirInlet.AirMassFlowRate > 0.0) {
                this->dd_airterminalRecircAirInlet.AirMassFlowRate = this->dd_airterminalRecircAirInlet.AirMassFlowRateHist1;
            }
        }

        // Find the Max Box Flow Rate.
        MassFlowMax = this->dd_airterminalOAInlet.AirMassFlowRateMaxAvail + this->dd_airterminalRecircAirInlet.AirMassFlowRateMaxAvail;
        if (this->availSched->getCurrentVal() > 0.0) {
            TotMassFlow = this->dd_airterminalOAInlet.AirMassFlowRate + this->dd_airterminalRecircAirInlet.AirMassFlowRate;
        } else {
            TotMassFlow = 0.0;
        }

        if (TotMassFlow > HVAC::SmallMassFlow) {

            // If the sum of the two air streams' flow is greater than the Max Box Flow Rate then reset the RA Stream
            if (TotMassFlow > MassFlowMax) {
                this->dd_airterminalRecircAirInlet.AirMassFlowRate = MassFlowMax - this->dd_airterminalOAInlet.AirMassFlowRate;
            }
            // After the flow rates are determined the properties are calculated.
            TotMassFlow = this->dd_airterminalOAInlet.AirMassFlowRate + this->dd_airterminalRecircAirInlet.AirMassFlowRate;
            if (TotMassFlow > HVAC::SmallMassFlow) {
                HumRat = (this->dd_airterminalOAInlet.AirHumRat * this->dd_airterminalOAInlet.AirMassFlowRate +
                          this->dd_airterminalRecircAirInlet.AirHumRat * this->dd_airterminalRecircAirInlet.AirMassFlowRate) /
                         TotMassFlow;
                Enthalpy = (this->dd_airterminalOAInlet.AirEnthalpy * this->dd_airterminalOAInlet.AirMassFlowRate +
                            this->dd_airterminalRecircAirInlet.AirEnthalpy * this->dd_airterminalRecircAirInlet.AirMassFlowRate) /
                           TotMassFlow;
            } else {
                HumRat = (this->dd_airterminalRecircAirInlet.AirHumRat + this->dd_airterminalOAInlet.AirHumRat) / 2.0;
                Enthalpy = (this->dd_airterminalRecircAirInlet.AirEnthalpy + this->dd_airterminalOAInlet.AirEnthalpy) / 2.0;
            }
        } else {

            // The Max Box Flow Rate is zero and the box is off.
            this->dd_airterminalRecircAirInlet.AirMassFlowRate = 0.0;
            this->dd_airterminalOAInlet.AirMassFlowRate = 0.0;
            HumRat = (this->dd_airterminalRecircAirInlet.AirHumRat + this->dd_airterminalOAInlet.AirHumRat) / 2.0;
            Enthalpy = (this->dd_airterminalRecircAirInlet.AirEnthalpy + this->dd_airterminalOAInlet.AirEnthalpy) / 2.0;
        }

        Temperature = Psychrometrics::PsyTdbFnHW(Enthalpy, HumRat);

        this->dd_airterminalOutlet.AirTemp = Temperature;
        this->dd_airterminalOutlet.AirHumRat = HumRat;
        this->dd_airterminalOutlet.AirMassFlowRate = TotMassFlow;
        this->dd_airterminalOutlet.AirMassFlowRateMaxAvail = MassFlowMax;
        this->dd_airterminalOutlet.AirEnthalpy = Enthalpy;

        // Calculate the OA and RA damper position in %
        if (this->RecircIsUsed) {
            if (this->dd_airterminalRecircAirInlet.AirMassFlowRateMax == 0.0) { // protect div by zero
                this->RecircAirDamperPosition = 0.0;
            } else {
                this->RecircAirDamperPosition =
                    this->dd_airterminalRecircAirInlet.AirMassFlowRate / this->dd_airterminalRecircAirInlet.AirMassFlowRateMax;
            }
        }

        if (this->dd_airterminalOAInlet.AirMassFlowRateMax == 0.0) { // protect div by zero
            this->OADamperPosition = 0.0;
        } else {
            this->OADamperPosition = this->dd_airterminalOAInlet.AirMassFlowRate / this->dd_airterminalOAInlet.AirMassFlowRateMax;
        }

        // Calculate OAFraction of mixed air after the box
        if (TotMassFlow > 0) {
            if (this->RecircIsUsed) {
                if (this->dd_airterminalOAInlet.AirMassFlowRate == 0.0) {
                    this->OAFraction = 0.0;
                } else if (this->dd_airterminalRecircAirInlet.AirMassFlowRate == 0.0) {
                    this->OAFraction = 1.0;
                } else {
                    this->OAFraction = this->dd_airterminalOAInlet.AirMassFlowRate / TotMassFlow;
                }
            } else {
                this->OAFraction = 1.0;
            }
        } else {
            this->OAFraction = 0.0;
        }

        this->dd_airterminalRecircAirInlet.AirMassFlowRateHist3 = this->dd_airterminalRecircAirInlet.AirMassFlowRateHist2;
        this->dd_airterminalRecircAirInlet.AirMassFlowRateHist2 = this->dd_airterminalRecircAirInlet.AirMassFlowRateHist1;
        this->dd_airterminalRecircAirInlet.AirMassFlowRateHist1 = this->dd_airterminalRecircAirInlet.AirMassFlowRate;
    }

    void DualDuctAirTerminal::CalcOAMassFlow(EnergyPlusData &state, // NOLINT(readability-make-member-function-const)
                                             Real64 &SAMassFlow,    // outside air based on optional user input
                                             Real64 &AirLoopOAFrac  // outside air based on optional user input
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         R. Raustad (FSEC)
        //       DATE WRITTEN   Mar 2010
        //       MODIFIED       Mangesh Basarkar, 06/2011: Modifying outside air based on airloop DCV flag
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Calculates the amount of outside air required based on optional user input.
        // Zone multipliers are included and are applied in GetInput.

        // METHODOLOGY EMPLOYED:
        // User input defines method used to calculate OA.

        // initialize OA flow rate and OA report variable
        SAMassFlow = 0.0;
        AirLoopOAFrac = 0.0;

        // Calculate the amount of OA based on optional user inputs
        if (AirLoopNum > 0) {
            AirLoopOAFrac = state.dataAirLoop->AirLoopFlow(AirLoopNum).OAFrac;
            // If no additional input from user, RETURN from subroutine
            if (this->NoOAFlowInputFromUser) {
                return;
            }
            // Calculate outdoor air flow rate, zone multipliers are applied in GetInput
            if (AirLoopOAFrac > 0.0) {
                bool constexpr UseMinOASchFlag(true); // Always use min OA schedule in calculations.
                Real64 const OAVolumeFlowRate =
                    DataSizing::calcDesignSpecificationOutdoorAir(state,
                                                                  this->OARequirementsPtr,
                                                                  this->CtrlZoneNum,
                                                                  state.dataAirLoop->AirLoopControlInfo(AirLoopNum).AirLoopDCVFlag,
                                                                  UseMinOASchFlag);
                Real64 const OAMassFlow = OAVolumeFlowRate * state.dataEnvrn->StdRhoAir;

                // convert OA mass flow rate to supply air flow rate based on air loop OA fraction
                SAMassFlow = OAMassFlow / AirLoopOAFrac;
            }
        }
    }

    void DualDuctAirTerminal::CalcOAOnlyMassFlow(EnergyPlusData &state,                   // NOLINT(readability-make-member-function-const)
                                                 Real64 &OAMassFlow,                      // outside air flow from user input kg/s
                                                 ObjexxFCL::Optional<Real64> MaxOAVolFlow // design level for outside air m3/s
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         C. Miller (Mod of CaclOAMassFlow by R. Raustad (FSEC))
        //       DATE WRITTEN   Aug 2010
        //       MODIFIED       B. Griffith, Dec 2010 clean up, sizing optional, scheduled OA
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Calculates the amount of outside air required based on optional user input. Returns
        // ONLY calculated OAMassFlow without consideration of AirLoopOAFrac. Used for
        // the DualDuct:VAV:OutdoorAir object which does not mix OA with RA

        // METHODOLOGY EMPLOYED:
        // User input defines method used to calculate OA.

        // Calculate the amount of OA based on optional user inputs
        OAMassFlow = 0.0;

        // If no additional input from user, RETURN from subroutine
        if (this->NoOAFlowInputFromUser) {
            ShowSevereError(
                state,
                format("CalcOAOnlyMassFlow: Problem in AirTerminal:DualDuct:VAV:OutdoorAir = {}, check outdoor air specification", this->Name));
            if (present(MaxOAVolFlow)) {
                MaxOAVolFlow = 0.0;
            }
            return;
        }

        bool UseOccSchFlag = this->OAPerPersonMode == PerPersonMode::DCVByCurrentLevel; // TRUE = use actual occupancy, FALSE = use total zone people
        bool PerPersonNotSet = this->OAPerPersonMode != PerPersonMode::DCVByCurrentLevel && this->OAPerPersonMode != PerPersonMode::ByDesignLevel;

        bool constexpr UseMinOASchFlag(true); // Always use min OA schedule in calculations.
        Real64 OAVolumeFlowRate = DataSizing::calcDesignSpecificationOutdoorAir(state,
                                                                                this->OARequirementsPtr,
                                                                                this->CtrlZoneNum,
                                                                                UseOccSchFlag,
                                                                                UseMinOASchFlag,
                                                                                PerPersonNotSet); // outside air volume flow rate (m3/s)

        OAMassFlow = OAVolumeFlowRate * state.dataEnvrn->StdRhoAir;

        if (present(MaxOAVolFlow)) {
            OAVolumeFlowRate = DataSizing::calcDesignSpecificationOutdoorAir(
                state, this->OARequirementsPtr, this->CtrlZoneNum, UseOccSchFlag, UseMinOASchFlag, false, true);
            MaxOAVolFlow = OAVolumeFlowRate;
        }
    }

    void DualDuctAirTerminal::UpdateDualDuct(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard J. Liesen
        //       DATE WRITTEN   February 2000
        //       MODIFIED       Aug 2010 Clayton Miller - Added DualDuctVAVOutdoorAir
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine updates the dampers.

        if (this->DamperType == DualDuctDamper::ConstantVolume || this->DamperType == DualDuctDamper::VariableVolume) {

            int OutletNode = this->OutletNodeNum;
            int HotInletNode = this->HotAirInletNodeNum;
            int ColdInletNode = this->ColdAirInletNodeNum;

            // Set the outlet air nodes of the Damper
            state.dataLoopNodes->Node(HotInletNode).MassFlowRate = this->dd_airterminalHotAirInlet.AirMassFlowRate;
            state.dataLoopNodes->Node(ColdInletNode).MassFlowRate = this->dd_airterminalColdAirInlet.AirMassFlowRate;
            state.dataLoopNodes->Node(OutletNode).MassFlowRate = this->dd_airterminalOutlet.AirMassFlowRate;
            state.dataLoopNodes->Node(OutletNode).MassFlowRateMaxAvail = this->dd_airterminalOutlet.AirMassFlowRate;
            state.dataLoopNodes->Node(OutletNode).MassFlowRateMinAvail = this->dd_airterminalOutlet.AirMassFlowRateMinAvail;
            state.dataLoopNodes->Node(OutletNode).Temp = this->dd_airterminalOutlet.AirTemp;
            state.dataLoopNodes->Node(OutletNode).HumRat = this->dd_airterminalOutlet.AirHumRat;
            state.dataLoopNodes->Node(OutletNode).Enthalpy = this->dd_airterminalOutlet.AirEnthalpy;
            // Set the outlet nodes for properties that just pass through & not used
            // FIX THIS LATER!!!!
            state.dataLoopNodes->Node(OutletNode).Quality = state.dataLoopNodes->Node(HotInletNode).Quality;
            state.dataLoopNodes->Node(OutletNode).Press = state.dataLoopNodes->Node(HotInletNode).Press;

            if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                if (state.dataLoopNodes->Node(OutletNode).MassFlowRate > 0.0) {
                    state.dataLoopNodes->Node(OutletNode).CO2 =
                        (state.dataLoopNodes->Node(HotInletNode).CO2 * state.dataLoopNodes->Node(HotInletNode).MassFlowRate +
                         state.dataLoopNodes->Node(ColdInletNode).CO2 * state.dataLoopNodes->Node(ColdInletNode).MassFlowRate) /
                        state.dataLoopNodes->Node(OutletNode).MassFlowRate;
                } else {
                    state.dataLoopNodes->Node(OutletNode).CO2 =
                        max(state.dataLoopNodes->Node(HotInletNode).CO2, state.dataLoopNodes->Node(ColdInletNode).CO2);
                }
            }
            if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                if (state.dataLoopNodes->Node(OutletNode).MassFlowRate > 0.0) {
                    state.dataLoopNodes->Node(OutletNode).GenContam =
                        (state.dataLoopNodes->Node(HotInletNode).GenContam * state.dataLoopNodes->Node(HotInletNode).MassFlowRate +
                         state.dataLoopNodes->Node(ColdInletNode).GenContam * state.dataLoopNodes->Node(ColdInletNode).MassFlowRate) /
                        state.dataLoopNodes->Node(OutletNode).MassFlowRate;
                } else {
                    state.dataLoopNodes->Node(OutletNode).GenContam =
                        max(state.dataLoopNodes->Node(HotInletNode).GenContam, state.dataLoopNodes->Node(ColdInletNode).GenContam);
                }
            }

            this->CalcOutdoorAirVolumeFlowRate(state);

        } else if (this->DamperType == DualDuctDamper::OutdoorAir) {

            int OutletNode = this->OutletNodeNum;
            int OAInletNode = this->OAInletNodeNum;
            // Set the outlet air nodes of the Damper
            state.dataLoopNodes->Node(OAInletNode).MassFlowRate = this->dd_airterminalOAInlet.AirMassFlowRate;
            state.dataLoopNodes->Node(OutletNode).MassFlowRate = this->dd_airterminalOutlet.AirMassFlowRate;
            state.dataLoopNodes->Node(OutletNode).MassFlowRateMaxAvail = this->dd_airterminalOutlet.AirMassFlowRate;
            state.dataLoopNodes->Node(OutletNode).MassFlowRateMinAvail = this->dd_airterminalOutlet.AirMassFlowRateMinAvail;
            state.dataLoopNodes->Node(OutletNode).Temp = this->dd_airterminalOutlet.AirTemp;
            state.dataLoopNodes->Node(OutletNode).HumRat = this->dd_airterminalOutlet.AirHumRat;
            state.dataLoopNodes->Node(OutletNode).Enthalpy = this->dd_airterminalOutlet.AirEnthalpy;
            // Set the outlet nodes for properties that just pass through & not used
            // FIX THIS LATER!!!!
            state.dataLoopNodes->Node(OutletNode).Quality = state.dataLoopNodes->Node(OAInletNode).Quality;
            state.dataLoopNodes->Node(OutletNode).Press = state.dataLoopNodes->Node(OAInletNode).Press;

            if (this->RecircIsUsed) {
                int RAInletNode = this->RecircAirInletNodeNum;
                state.dataLoopNodes->Node(RAInletNode).MassFlowRate = this->dd_airterminalRecircAirInlet.AirMassFlowRate;
                if (state.dataLoopNodes->Node(OutletNode).MassFlowRate > 0.0) {
                    if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                        state.dataLoopNodes->Node(OutletNode).CO2 =
                            (state.dataLoopNodes->Node(OAInletNode).CO2 * state.dataLoopNodes->Node(OAInletNode).MassFlowRate +
                             state.dataLoopNodes->Node(RAInletNode).CO2 * state.dataLoopNodes->Node(RAInletNode).MassFlowRate) /
                            state.dataLoopNodes->Node(OutletNode).MassFlowRate;
                    }
                    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                        state.dataLoopNodes->Node(OutletNode).GenContam =
                            (state.dataLoopNodes->Node(OAInletNode).GenContam * state.dataLoopNodes->Node(OAInletNode).MassFlowRate +
                             state.dataLoopNodes->Node(RAInletNode).GenContam * state.dataLoopNodes->Node(RAInletNode).MassFlowRate) /
                            state.dataLoopNodes->Node(OutletNode).MassFlowRate;
                    }
                } else {
                    if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                        state.dataLoopNodes->Node(OutletNode).CO2 =
                            max(state.dataLoopNodes->Node(OAInletNode).CO2, state.dataLoopNodes->Node(RAInletNode).CO2);
                    }
                    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                        state.dataLoopNodes->Node(OutletNode).GenContam =
                            max(state.dataLoopNodes->Node(OAInletNode).GenContam, state.dataLoopNodes->Node(RAInletNode).GenContam);
                    }
                }

            } else {
                if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                    state.dataLoopNodes->Node(OutletNode).CO2 = state.dataLoopNodes->Node(OAInletNode).CO2;
                }
                if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                    state.dataLoopNodes->Node(OutletNode).GenContam = state.dataLoopNodes->Node(OAInletNode).GenContam;
                }
            }
        }
    }

    void ReportDualDuctConnections(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Michael J. Witte
        //       DATE WRITTEN   February 2004
        //       MODIFIED       B. Griffith, DOAS VAV dual duct
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Report dual duct damper connections to the BND file.

        // Using/Aliasing
        int NumPrimaryAirSys = state.dataHVACGlobal->NumPrimaryAirSys;

        // Formats
        static constexpr std::string_view Format_100("! <#Dual Duct Damper Connections>,<Number of Dual Duct Damper Connections>");
        static constexpr std::string_view Format_102(
            "! <Dual Duct Damper>,<Dual Duct Damper Count>,<Dual Duct Damper Name>,<Inlet Node>,<Outlet Node>,<Inlet "
            "Node Type>,<AirLoopHVAC Name>");

        if (!allocated(state.dataDualDuct->dd_airterminal)) {
            return; // Autodesk Bug: Can arrive here with Damper unallocated (SimulateDualDuct not yet called) with NumDDAirTerminal either set >0 or
        }
        // uninitialized

        // Report Dual Duct Dampers to BND File
        print(state.files.bnd, "{}\n", "! ===============================================================");
        print(state.files.bnd, "{}\n", Format_100);
        print(state.files.bnd, " #Dual Duct Damper Connections,{}\n", state.dataDualDuct->NumDDAirTerminal * 2);
        print(state.files.bnd, "{}\n", Format_102);

        for (int Count1 = 1; Count1 <= state.dataDualDuct->NumDDAirTerminal; ++Count1) {

            // Determine if this damper is connected to a supply air path
            int Found = 0;
            int SupplyAirPathNum = 0;
            for (int Count2 = 1; Count2 <= state.dataZoneEquip->NumSupplyAirPaths; ++Count2) {
                SupplyAirPathNum = Count2;
                Found = 0;
                for (int Count3 = 1; Count3 <= state.dataZoneEquip->SupplyAirPath(Count2).NumOutletNodes; ++Count3) {
                    if (state.dataDualDuct->dd_airterminal(Count1).HotAirInletNodeNum ==
                        state.dataZoneEquip->SupplyAirPath(Count2).OutletNode(Count3)) {
                        Found = Count3;
                    }
                    if (state.dataDualDuct->dd_airterminal(Count1).ColdAirInletNodeNum ==
                        state.dataZoneEquip->SupplyAirPath(Count2).OutletNode(Count3)) {
                        Found = Count3;
                    }
                    if (state.dataDualDuct->dd_airterminal(Count1).OAInletNodeNum == state.dataZoneEquip->SupplyAirPath(Count2).OutletNode(Count3)) {
                        Found = Count3;
                    }
                    if (state.dataDualDuct->dd_airterminal(Count1).RecircAirInletNodeNum ==
                        state.dataZoneEquip->SupplyAirPath(Count2).OutletNode(Count3)) {
                        Found = Count3;
                    }
                }
                if (Found != 0) {
                    break;
                }
            }
            if (Found == 0) {
                SupplyAirPathNum = 0;
            }

            // Determine which air loop this dual duct damper is connected to
            Found = 0;
            std::string ChrName;
            for (int Count2 = 1; Count2 <= NumPrimaryAirSys; ++Count2) {
                ChrName = state.dataAirLoop->AirToZoneNodeInfo(Count2).AirLoopName;
                Found = 0;
                for (int Count3 = 1; Count3 <= state.dataAirLoop->AirToZoneNodeInfo(Count2).NumSupplyNodes; ++Count3) {
                    if (SupplyAirPathNum != 0) {
                        if (state.dataZoneEquip->SupplyAirPath(SupplyAirPathNum).InletNodeNum ==
                            state.dataAirLoop->AirToZoneNodeInfo(Count2).ZoneEquipSupplyNodeNum(Count3)) {
                            Found = Count3;
                        }
                    } else {
                        if (state.dataDualDuct->dd_airterminal(Count1).HotAirInletNodeNum ==
                            state.dataAirLoop->AirToZoneNodeInfo(Count2).ZoneEquipSupplyNodeNum(Count3)) {
                            Found = Count3;
                        }
                        if (state.dataDualDuct->dd_airterminal(Count1).ColdAirInletNodeNum ==
                            state.dataAirLoop->AirToZoneNodeInfo(Count2).ZoneEquipSupplyNodeNum(Count3)) {
                            Found = Count3;
                        }
                        if (state.dataDualDuct->dd_airterminal(Count1).OAInletNodeNum ==
                            state.dataAirLoop->AirToZoneNodeInfo(Count2).ZoneEquipSupplyNodeNum(Count3)) {
                            Found = Count3;
                        }
                        if (state.dataDualDuct->dd_airterminal(Count1).RecircAirInletNodeNum ==
                            state.dataAirLoop->AirToZoneNodeInfo(Count2).ZoneEquipSupplyNodeNum(Count3)) {
                            Found = Count3;
                        }
                    }
                }
                if (Found != 0) {
                    break;
                }
            }
            if (Found == 0) {
                ChrName = "**Unknown**";
            }

            std::string_view damperType = cmoNameArray[static_cast<int>(state.dataDualDuct->dd_airterminal(Count1).DamperType)];
            if ((state.dataDualDuct->dd_airterminal(Count1).DamperType == DualDuctDamper::ConstantVolume) ||
                (state.dataDualDuct->dd_airterminal(Count1).DamperType == DualDuctDamper::VariableVolume)) {
                print(state.files.bnd,
                      " Dual Duct Damper,{},{},{},{},{},Hot Air,{}\n",
                      Count1,
                      damperType,
                      state.dataDualDuct->dd_airterminal(Count1).Name,
                      state.dataLoopNodes->NodeID(state.dataDualDuct->dd_airterminal(Count1).HotAirInletNodeNum),
                      state.dataLoopNodes->NodeID(state.dataDualDuct->dd_airterminal(Count1).OutletNodeNum),
                      ChrName);

                print(state.files.bnd,
                      " Dual Duct Damper,{},{},{},{},{},Cold Air,{}\n",
                      Count1,
                      damperType,
                      state.dataDualDuct->dd_airterminal(Count1).Name,
                      state.dataLoopNodes->NodeID(state.dataDualDuct->dd_airterminal(Count1).ColdAirInletNodeNum),
                      state.dataLoopNodes->NodeID(state.dataDualDuct->dd_airterminal(Count1).OutletNodeNum),
                      ChrName);

            } else if (state.dataDualDuct->dd_airterminal(Count1).DamperType == DualDuctDamper::OutdoorAir) {
                print(state.files.bnd,
                      "Dual Duct Damper, {},{},{},{},{},Outdoor Air,{}\n",
                      Count1,
                      damperType,
                      state.dataDualDuct->dd_airterminal(Count1).Name,
                      state.dataLoopNodes->NodeID(state.dataDualDuct->dd_airterminal(Count1).OAInletNodeNum),
                      state.dataLoopNodes->NodeID(state.dataDualDuct->dd_airterminal(Count1).OutletNodeNum),
                      ChrName);
                print(state.files.bnd,
                      "Dual Duct Damper, {},{},{},{},{},Recirculated Air,{}\n",
                      Count1,
                      damperType,
                      state.dataDualDuct->dd_airterminal(Count1).Name,
                      state.dataLoopNodes->NodeID(state.dataDualDuct->dd_airterminal(Count1).RecircAirInletNodeNum),
                      state.dataLoopNodes->NodeID(state.dataDualDuct->dd_airterminal(Count1).OutletNodeNum),
                      ChrName);
            }
        }
    }

    void GetDualDuctOutdoorAirRecircUse(EnergyPlusData &state,
                                        [[maybe_unused]] std::string const &CompTypeName,
                                        std::string_view CompName,
                                        bool &RecircIsUsed)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Aug 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // get routine to learn if a dual duct outdoor air unit is using its recirc deck

        RecircIsUsed = true;
        if (state.dataDualDuct->GetDualDuctOutdoorAirRecircUseFirstTimeOnly) {
            state.dataDualDuct->NumDualDuctVarVolOA = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCMO_DDVarVolOA);
            state.dataDualDuct->RecircIsUsedARR.allocate(state.dataDualDuct->NumDualDuctVarVolOA);
            state.dataDualDuct->DamperNamesARR.allocate(state.dataDualDuct->NumDualDuctVarVolOA);
            if (state.dataDualDuct->NumDualDuctVarVolOA > 0) {
                Array1D<Real64> NumArray(2, 0.0);
                Array1D_string AlphArray(7);
                Array1D_string cAlphaFields(7);       // Alpha field names
                Array1D_string cNumericFields(2);     // Numeric field names
                Array1D_bool lAlphaBlanks(7, true);   // Logical array, alpha field input BLANK = .TRUE.
                Array1D_bool lNumericBlanks(2, true); // Logical array, numeric field input BLANK = .TRUE.
                for (int DamperIndex = 1; DamperIndex <= state.dataDualDuct->NumDualDuctVarVolOA; ++DamperIndex) {

                    int NumAlphas;
                    int NumNums;
                    int IOStat;
                    state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                             cCMO_DDVarVolOA,
                                                                             DamperIndex,
                                                                             AlphArray,
                                                                             NumAlphas,
                                                                             NumArray,
                                                                             NumNums,
                                                                             IOStat,
                                                                             lNumericBlanks,
                                                                             lAlphaBlanks,
                                                                             cAlphaFields,
                                                                             cNumericFields);
                    state.dataDualDuct->DamperNamesARR(DamperIndex) = AlphArray(1);
                    if (!lAlphaBlanks(5)) {
                        state.dataDualDuct->RecircIsUsedARR(DamperIndex) = true;
                    } else {
                        state.dataDualDuct->RecircIsUsedARR(DamperIndex) = false;
                    }
                }
            }
            state.dataDualDuct->GetDualDuctOutdoorAirRecircUseFirstTimeOnly = false;
        }

        int DamperIndex = Util::FindItemInList(CompName, state.dataDualDuct->DamperNamesARR, state.dataDualDuct->NumDualDuctVarVolOA);
        if (DamperIndex > 0) {
            RecircIsUsed = state.dataDualDuct->RecircIsUsedARR(DamperIndex);
        }
    }

    void DualDuctAirTerminal::CalcOutdoorAirVolumeFlowRate(EnergyPlusData &state)
    {
        // calculates zone outdoor air volume flow rate using the supply air flow rate and OA fraction, for AirLoopNum > 0 only for now
        if (this->AirLoopNum > 0) {
            this->OutdoorAirFlowRate =
                (this->dd_airterminalOutlet.AirMassFlowRate / state.dataEnvrn->StdRhoAir) * state.dataAirLoop->AirLoopFlow(this->AirLoopNum).OAFrac;
        }
    }

    void DualDuctAirTerminal::reportTerminalUnit(EnergyPlusData &state)
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

        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermTypeInp, adu.Name, dualDuctDamperNames[(int)this->DamperType]);
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermPrimFlow, adu.Name, this->MaxAirVolFlowRate);
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermSecdFlow, adu.Name, "n/a");
        if (this->zoneTurndownMinAirFracSched != nullptr) {
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermMinFlowSch, adu.Name, this->zoneTurndownMinAirFracSched->Name);
        } else {
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermMinFlowSch, adu.Name, "n/a");
        }
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermMaxFlowReh, adu.Name, "n/a");
        std::string schName = "n/a";
        if (this->OARequirementsPtr > 0) {
            schName = state.dataSize->OARequirements(this->OARequirementsPtr).oaFlowFracSched->Name;
        }
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermMinOAflowSch, adu.Name, schName);
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermHeatCoilType, adu.Name, "n/a");
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermCoolCoilType, adu.Name, "n/a");
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermFanType, adu.Name, "n/a");
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermFanName, adu.Name, "n/a");
    }

} // namespace DualDuct

} // namespace EnergyPlus
