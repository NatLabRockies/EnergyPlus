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
#include <EnergyPlus/Autosizing/SystemAirFlowSizing.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneControls.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HVACStandAloneERV.hh>
#include <EnergyPlus/HeatRecovery.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::HVACStandAloneERV {

// Module containing the routines dealing with stand alone energy recovery ventilators (ERVs)

// MODULE INFORMATION:
//       AUTHOR         Richard Raustad, FSEC
//       DATE WRITTEN   June 2003

// PURPOSE OF THIS MODULE:
// To encapsulate the data and algorithms needed to simulate stand alone
// energy recovery ventilators that condition outdoor ventilation air and
// supply that air directly to a zone.

// METHODOLOGY EMPLOYED:
// These units are modeled as a collection of components: air-to-air generic heat exchanger,
// supply air fan, exhaust air fan and an optional controller to avoid overheating
// of the supply air (economizer or free cooling operation).

void SimStandAloneERV(EnergyPlusData &state,
                      std::string_view CompName,     // name of the Stand Alone ERV unit
                      int const ZoneNum,             // number of zone being served unused1208
                      bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                      Real64 &SensLoadMet,           // net sensible load supplied by the ERV unit to the zone (W)
                      Real64 &LatLoadMet,            // net latent load supplied by ERV unit to the zone (kg/s),
                      int &CompIndex                 // pointer to correct component
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad, FSEC
    //       DATE WRITTEN   June 2003
    //       MODIFIED       Don Shirey, Aug 2009 (LatLoadMet)

    // PURPOSE OF THIS SUBROUTINE:
    // Manages the simulation of a Stand Alone ERV unit. Called from SimZoneEquipment

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int StandAloneERVNum; // index of Stand Alone ERV unit being simulated

    // First time SimStandAloneERV is called, get the input for all Stand Alone ERV units
    if (state.dataHVACStandAloneERV->GetERVInputFlag) {
        GetStandAloneERV(state);
        state.dataHVACStandAloneERV->GetERVInputFlag = false;
    }

    // Find the correct Stand Alone ERV unit index
    if (CompIndex == 0) {
        StandAloneERVNum = Util::FindItem(CompName, state.dataHVACStandAloneERV->StandAloneERV);
        if (StandAloneERVNum == 0) {
            ShowFatalError(state, format("SimStandAloneERV: Unit not found={}", CompName));
        }
        CompIndex = StandAloneERVNum;
    } else {
        StandAloneERVNum = CompIndex;
        if (StandAloneERVNum > state.dataHVACStandAloneERV->NumStandAloneERVs || StandAloneERVNum < 1) {
            ShowFatalError(state,
                           format("SimStandAloneERV:  Invalid CompIndex passed={}, Number of Units={}, Entered Unit name={}",
                                  StandAloneERVNum,
                                  state.dataHVACStandAloneERV->NumStandAloneERVs,
                                  CompName));
        }
        if (state.dataHVACStandAloneERV->CheckEquipName(StandAloneERVNum)) {
            if (CompName != state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).Name) {
                ShowFatalError(state,
                               format("SimStandAloneERV: Invalid CompIndex passed={}, Unit name={}, stored Unit Name for that index={}",
                                      StandAloneERVNum,
                                      CompName,
                                      state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).Name));
            }
            state.dataHVACStandAloneERV->CheckEquipName(StandAloneERVNum) = false;
        }
    }

    // Initialize the Stand Alone ERV unit
    InitStandAloneERV(state, StandAloneERVNum, ZoneNum, FirstHVACIteration);

    CalcStandAloneERV(state, StandAloneERVNum, FirstHVACIteration, SensLoadMet, LatLoadMet);

    ReportStandAloneERV(state, StandAloneERVNum);
}

void GetStandAloneERV(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad
    //       DATE WRITTEN   June 2003
    //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone sys avail managers

    // PURPOSE OF THIS SUBROUTINE:
    // Obtains input data for Stand Alone ERV units and stores it in the Stand Alone ERV data structure

    // METHODOLOGY EMPLOYED:
    // Uses "Get" routines to read in data.

    static constexpr std::string_view routineName = "GetStandAloneERV";

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Array1D_string Alphas;   // Alpha items for object
    Array1D<Real64> Numbers; // Numeric items for object
    Array1D_string cAlphaFields;
    Array1D_string cNumericFields;
    Array1D_bool lAlphaBlanks;
    Array1D_bool lNumericBlanks;
    int NumArg;
    int NumAlphas;           // Number of Alphas for each GetObjectItem call
    int NumNumbers;          // Number of Numbers for each GetObjectItem call
    int IOStatus;            // Used in GetObjectItem
    bool ErrorsFound(false); // Set to true if errors in input, fatal at end of routine
    int NumERVCtrlrs;        // total number of CONTROLLER:STAND ALONE ERV objects
    int ERVControllerNum;    // index to ERV controller
    Real64 AirFlowRate;      // used to find zone with humidistat
    int NodeNumber;          // used to find zone with humidistat
    int HStatZoneNum;        // used to find zone with humidistat
    int NumHstatZone;        // index to humidity controlled zones
    Real64 HXSupAirFlowRate; // HX supply air flow rate [m3/s]
    int ZoneInletCZN;        // used for warning when zone node not listed in equipment connections
    int ZoneExhaustCZN;      // used for warning when zone node not listed in equipment connections

    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "ZoneHVAC:EnergyRecoveryVentilator", NumArg, NumAlphas, NumNumbers);
    int MaxAlphas = NumAlphas;
    int MaxNumbers = NumNumbers;
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
        state, "ZoneHVAC:EnergyRecoveryVentilator:Controller", NumArg, NumAlphas, NumNumbers);
    MaxAlphas = max(MaxAlphas, NumAlphas);
    MaxNumbers = max(MaxNumbers, NumNumbers);

    Alphas.allocate(MaxAlphas);
    Numbers.dimension(MaxNumbers, 0.0);
    cAlphaFields.allocate(MaxAlphas);
    cNumericFields.allocate(MaxNumbers);
    lNumericBlanks.dimension(MaxNumbers, false);
    lAlphaBlanks.dimension(MaxAlphas, false);

    state.dataHVACStandAloneERV->GetERVInputFlag = false;

    // find the number of each type of Stand Alone ERV unit
    std::string CurrentModuleObject = "ZoneHVAC:EnergyRecoveryVentilator";

    state.dataHVACStandAloneERV->NumStandAloneERVs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);

    // allocate the data structures
    state.dataHVACStandAloneERV->StandAloneERV.allocate(state.dataHVACStandAloneERV->NumStandAloneERVs);
    state.dataHVACStandAloneERV->HeatExchangerUniqueNames.reserve(static_cast<unsigned>(state.dataHVACStandAloneERV->NumStandAloneERVs));
    state.dataHVACStandAloneERV->SupplyAirFanUniqueNames.reserve(static_cast<unsigned>(state.dataHVACStandAloneERV->NumStandAloneERVs));
    state.dataHVACStandAloneERV->ExhaustAirFanUniqueNames.reserve(static_cast<unsigned>(state.dataHVACStandAloneERV->NumStandAloneERVs));
    state.dataHVACStandAloneERV->ControllerUniqueNames.reserve(static_cast<unsigned>(state.dataHVACStandAloneERV->NumStandAloneERVs));
    state.dataHVACStandAloneERV->CheckEquipName.dimension(state.dataHVACStandAloneERV->NumStandAloneERVs, true);

    // loop over Stand Alone ERV units; get and load the input data
    for (int StandAloneERVIndex = 1; StandAloneERVIndex <= state.dataHVACStandAloneERV->NumStandAloneERVs; ++StandAloneERVIndex) {
        auto &standAloneERV = state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVIndex);

        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 StandAloneERVIndex,
                                                                 Alphas,
                                                                 NumAlphas,
                                                                 Numbers,
                                                                 NumNumbers,
                                                                 IOStatus,
                                                                 lNumericBlanks,
                                                                 lAlphaBlanks,
                                                                 cAlphaFields,
                                                                 cNumericFields);

        standAloneERV.Name = Alphas(1);
        standAloneERV.UnitType = CurrentModuleObject;

        ErrorObjectHeader eoh{routineName, CurrentModuleObject, standAloneERV.Name};

        if (lAlphaBlanks(2)) {
            standAloneERV.availSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((standAloneERV.availSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
            ErrorsFound = true;
        }

        GlobalNames::IntraObjUniquenessCheck(
            state, Alphas(3), CurrentModuleObject, cAlphaFields(3), state.dataHVACStandAloneERV->HeatExchangerUniqueNames, ErrorsFound);
        standAloneERV.HeatExchangerName = Alphas(3);
        bool errFlag = false;
        standAloneERV.hxType =
            HeatRecovery::GetHeatExchangerObjectTypeNum(state, standAloneERV.HeatExchangerName, standAloneERV.HeatExchangerIndex, errFlag);
        if (errFlag) {
            ShowContinueError(state, format("... occurs in {} \"{}\"", CurrentModuleObject, standAloneERV.Name));
            ErrorsFound = true;
        }

        errFlag = false;
        HXSupAirFlowRate = HeatRecovery::GetSupplyAirFlowRate(state, standAloneERV.HeatExchangerName, errFlag);
        if (errFlag) {
            ShowContinueError(state, format("... occurs in {} \"{}\"", CurrentModuleObject, standAloneERV.Name));
            ErrorsFound = true;
        }
        standAloneERV.DesignHXVolFlowRate = HXSupAirFlowRate;

        standAloneERV.SupplyAirFanName = Alphas(4);
        GlobalNames::IntraObjUniquenessCheck(
            state, Alphas(4), CurrentModuleObject, cAlphaFields(4), state.dataHVACStandAloneERV->SupplyAirFanUniqueNames, ErrorsFound);

        if ((standAloneERV.SupplyAirFanIndex = Fans::GetFanIndex(state, standAloneERV.SupplyAirFanName)) == 0) {
            ShowSevereItemNotFound(state, eoh, cAlphaFields(4), standAloneERV.SupplyAirFanName);
            ErrorsFound = true;
        } else {
            auto *fan = state.dataFans->fans(standAloneERV.SupplyAirFanIndex);
            standAloneERV.supplyAirFanType = fan->type;
            standAloneERV.supplyAirFanSched = fan->availSched;
            standAloneERV.DesignSAFanVolFlowRate = fan->maxAirFlowRate;
            standAloneERV.SupplyAirOutletNode = fan->outletNodeNum;
        }

        standAloneERV.ExhaustAirFanName = Alphas(5);
        GlobalNames::IntraObjUniquenessCheck(
            state, Alphas(5), CurrentModuleObject, cAlphaFields(5), state.dataHVACStandAloneERV->ExhaustAirFanUniqueNames, ErrorsFound);

        if ((standAloneERV.ExhaustAirFanIndex = Fans::GetFanIndex(state, standAloneERV.ExhaustAirFanName)) == 0) {
            ShowSevereItemNotFound(state, eoh, cAlphaFields(5), standAloneERV.ExhaustAirFanName);
            ErrorsFound = true;

        } else {
            auto *fan = state.dataFans->fans(standAloneERV.ExhaustAirFanIndex);
            standAloneERV.exhaustAirFanType = fan->type;

            standAloneERV.exhaustAirFanSched = fan->availSched;
            standAloneERV.DesignEAFanVolFlowRate = fan->maxAirFlowRate;
            standAloneERV.ExhaustAirOutletNode = fan->outletNodeNum;
        }

        errFlag = false;
        standAloneERV.SupplyAirInletNode = HeatRecovery::GetSupplyInletNode(state, standAloneERV.HeatExchangerName, errFlag);
        standAloneERV.ExhaustAirInletNode = HeatRecovery::GetSecondaryInletNode(state, standAloneERV.HeatExchangerName, errFlag);
        if (errFlag) {
            ShowContinueError(state, format("... occurs in {} ={}", CurrentModuleObject, standAloneERV.Name));
            ErrorsFound = true;
        }
        standAloneERV.SupplyAirInletNode = GetOnlySingleNode(state,
                                                             state.dataLoopNodes->NodeID(standAloneERV.SupplyAirInletNode),
                                                             ErrorsFound,
                                                             DataLoopNode::ConnectionObjectType::ZoneHVACEnergyRecoveryVentilator,
                                                             Alphas(1),
                                                             DataLoopNode::NodeFluidType::Air,
                                                             DataLoopNode::ConnectionType::Inlet,
                                                             NodeInputManager::CompFluidStream::Primary,
                                                             DataLoopNode::ObjectIsParent);
        standAloneERV.SupplyAirOutletNode = GetOnlySingleNode(state,
                                                              state.dataLoopNodes->NodeID(standAloneERV.SupplyAirOutletNode),
                                                              ErrorsFound,
                                                              DataLoopNode::ConnectionObjectType::ZoneHVACEnergyRecoveryVentilator,
                                                              Alphas(1),
                                                              DataLoopNode::NodeFluidType::Air,
                                                              DataLoopNode::ConnectionType::Outlet,
                                                              NodeInputManager::CompFluidStream::Primary,
                                                              DataLoopNode::ObjectIsParent);
        standAloneERV.ExhaustAirInletNode = GetOnlySingleNode(state,
                                                              state.dataLoopNodes->NodeID(standAloneERV.ExhaustAirInletNode),
                                                              ErrorsFound,
                                                              DataLoopNode::ConnectionObjectType::ZoneHVACEnergyRecoveryVentilator,
                                                              Alphas(1),
                                                              DataLoopNode::NodeFluidType::Air,
                                                              DataLoopNode::ConnectionType::Inlet,
                                                              NodeInputManager::CompFluidStream::Secondary,
                                                              DataLoopNode::ObjectIsParent);
        standAloneERV.ExhaustAirOutletNode = GetOnlySingleNode(state,
                                                               state.dataLoopNodes->NodeID(standAloneERV.ExhaustAirOutletNode),
                                                               ErrorsFound,
                                                               DataLoopNode::ConnectionObjectType::ZoneHVACEnergyRecoveryVentilator,
                                                               Alphas(1),
                                                               DataLoopNode::NodeFluidType::Air,
                                                               DataLoopNode::ConnectionType::ReliefAir,
                                                               NodeInputManager::CompFluidStream::Secondary,
                                                               DataLoopNode::ObjectIsParent);

        //   Check that supply air inlet node is an OA node
        if (!OutAirNodeManager::CheckOutAirNodeNumber(state, standAloneERV.SupplyAirInletNode)) {
            ShowSevereError(state, format("For {} \"{}\"", CurrentModuleObject, standAloneERV.Name));
            ShowContinueError(state,
                              format(" Node name of supply air inlet node not valid Outdoor Air Node = {}",
                                     state.dataLoopNodes->NodeID(standAloneERV.SupplyAirInletNode)));
            ShowContinueError(state, "...does not appear in an OutdoorAir:NodeList or as an OutdoorAir:Node.");
            ErrorsFound = true;
        }

        //   Check to make sure inlet and exhaust nodes are listed in a ZoneHVAC:EquipmentConnections object
        bool ZoneInletNodeFound = false;
        bool ZoneExhaustNodeFound = false;
        for (int ControlledZoneNum = 1; ControlledZoneNum <= state.dataGlobal->NumOfZones; ++ControlledZoneNum) {
            if (!ZoneInletNodeFound) {
                for (NodeNumber = 1; NodeNumber <= state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).NumInletNodes; ++NodeNumber) {
                    if (state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).InletNode(NodeNumber) == standAloneERV.SupplyAirOutletNode) {
                        ZoneInletNodeFound = true;
                        ZoneInletCZN = ControlledZoneNum;
                        break; // found zone inlet node
                    }
                }
            }
            if (!ZoneExhaustNodeFound) {
                for (NodeNumber = 1; NodeNumber <= state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).NumExhaustNodes; ++NodeNumber) {
                    if (state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).ExhaustNode(NodeNumber) == standAloneERV.ExhaustAirInletNode) {
                        ZoneExhaustNodeFound = true;
                        ZoneExhaustCZN = ControlledZoneNum;
                        break; // found zone exhaust node
                    }
                }
            }
        }
        if (!ZoneInletNodeFound) {
            ShowSevereError(state, format("For {} \"{}\"", CurrentModuleObject, standAloneERV.Name));
            ShowContinueError(state, "... Node name of supply air outlet node does not appear in a ZoneHVAC:EquipmentConnections object.");
            ShowContinueError(state, format("... Supply air outlet node = {}", state.dataLoopNodes->NodeID(standAloneERV.SupplyAirOutletNode)));
            ErrorsFound = true;
        }
        if (!ZoneExhaustNodeFound) {
            ShowSevereError(state, format("For {} \"{}\"", CurrentModuleObject, standAloneERV.Name));
            ShowContinueError(state, "... Node name of exhaust air inlet node does not appear in a ZoneHVAC:EquipmentConnections object.");
            ShowContinueError(state, format("... Exhaust air inlet node = {}", state.dataLoopNodes->NodeID(standAloneERV.ExhaustAirInletNode)));
            ErrorsFound = true;
        }
        //   If nodes are found, make sure they are in the same zone
        if (ZoneInletNodeFound && ZoneExhaustNodeFound) {
            if (ZoneInletCZN != ZoneExhaustCZN) {
                ShowSevereError(state, format("For {} \"{}\"", CurrentModuleObject, standAloneERV.Name));
                ShowContinueError(state,
                                  "... Node name of supply air outlet node and exhasut air inlet node must appear in the same "
                                  "ZoneHVAC:EquipmentConnections object.");
                ShowContinueError(state, format("... Supply air outlet node = {}", state.dataLoopNodes->NodeID(standAloneERV.SupplyAirOutletNode)));
                ShowContinueError(
                    state, format("... ZoneHVAC:EquipmentConnections Zone Name = {}", state.dataZoneEquip->ZoneEquipConfig(ZoneInletCZN).ZoneName));
                ShowContinueError(state, format("... Exhaust air inlet node = {}", state.dataLoopNodes->NodeID(standAloneERV.ExhaustAirInletNode)));
                ShowContinueError(
                    state, format("... ZoneHVAC:EquipmentConnections Zone Name = {}", state.dataZoneEquip->ZoneEquipConfig(ZoneExhaustCZN).ZoneName));
                ErrorsFound = true;
            }
        }

        standAloneERV.ControllerName = Alphas(6);
        // If controller name is blank the ERV unit will operate with no controller
        if (lAlphaBlanks(6)) {
            standAloneERV.ControllerName = "xxxxx";
            standAloneERV.ControllerNameDefined = false;
        } else {
            // Verify controller name in Stand Alone ERV object matches name of valid controller object
            GlobalNames::IntraObjUniquenessCheck(
                state, Alphas(6), CurrentModuleObject, cAlphaFields(6), state.dataHVACStandAloneERV->ControllerUniqueNames, ErrorsFound);
            standAloneERV.ControllerNameDefined = true;
            if (ErrorsFound) {
                standAloneERV.ControllerNameDefined = false;
            }

            if (state.dataInputProcessing->inputProcessor->getObjectItemNum(
                    state, "ZoneHVAC:EnergyRecoveryVentilator:Controller", standAloneERV.ControllerName) <= 0) {
                ShowSevereError(
                    state, format("{} controller type ZoneHVAC:EnergyRecoveryVentilator:Controller not found = {}", CurrentModuleObject, Alphas(6)));
                ErrorsFound = true;
                standAloneERV.ControllerNameDefined = false;
            } else {
                state.dataHeatRecovery->ExchCond(standAloneERV.HeatExchangerIndex).hasZoneERVController = true;
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchAirHROAControllerName, standAloneERV.HeatExchangerName, standAloneERV.ControllerName);
            }
        }

        if (!lAlphaBlanks(7)) {
            standAloneERV.AvailManagerListName = Alphas(7);
        }

        // Read supply and exhaust air flow rates
        standAloneERV.SupplyAirVolFlow = Numbers(1);
        standAloneERV.ExhaustAirVolFlow = Numbers(2);

        // Read ventilation rate per floor area for autosizing HX and fans
        standAloneERV.AirVolFlowPerFloorArea = Numbers(3);
        standAloneERV.AirVolFlowPerOccupant = Numbers(4);

        if (standAloneERV.SupplyAirVolFlow == DataSizing::AutoSize && standAloneERV.DesignSAFanVolFlowRate != DataSizing::AutoSize) {
            ShowSevereError(state, format("{} \"{}\"", CurrentModuleObject, standAloneERV.Name));
            ShowContinueError(state,
                              format("... When autosizing ERV, supply air fan = {} \"{}\" must also be autosized.",
                                     HVAC::fanTypeNames[(int)standAloneERV.supplyAirFanType],
                                     standAloneERV.SupplyAirFanName));
        }

        if (standAloneERV.ExhaustAirVolFlow == DataSizing::AutoSize && standAloneERV.DesignEAFanVolFlowRate != DataSizing::AutoSize) {
            ShowSevereError(state, format("{} \"{}\"", CurrentModuleObject, standAloneERV.Name));
            ShowContinueError(state,
                              format("... When autosizing ERV, exhaust air fan = {} \"{}\" must also be autosized.",
                                     HVAC::fanTypeNames[(int)standAloneERV.exhaustAirFanType],
                                     standAloneERV.ExhaustAirFanName));
        }

        if (standAloneERV.SupplyAirVolFlow == DataSizing::AutoSize && HXSupAirFlowRate != DataSizing::AutoSize) {
            ShowSevereError(state, format("{} \"{}\"", CurrentModuleObject, standAloneERV.Name));
            ShowContinueError(
                state,
                format("... When autosizing ERV {}, nominal supply air flow rate for heat exchanger with name = {} must also be autosized.",
                       cNumericFields(1),
                       standAloneERV.HeatExchangerName));
        }

        if (standAloneERV.ExhaustAirVolFlow == DataSizing::AutoSize && HXSupAirFlowRate != DataSizing::AutoSize) {
            ShowSevereError(state, format("{} \"{}\"", CurrentModuleObject, standAloneERV.Name));
            ShowContinueError(
                state,
                format("... When autosizing ERV {}, nominal supply air flow rate for heat exchanger with name = {} must also be autosized.",
                       cNumericFields(2),
                       standAloneERV.HeatExchangerName));
        }

        // Compare the ERV SA flow rates to SA fan object.
        if (standAloneERV.DesignSAFanVolFlowRate != DataSizing::AutoSize && standAloneERV.SupplyAirVolFlow != DataSizing::AutoSize) {
            if (standAloneERV.SupplyAirVolFlow > standAloneERV.DesignSAFanVolFlowRate) {
                ShowWarningError(state,
                                 format("{} = {} has a {} > Max Volume Flow Rate defined in the associated fan object, should be <=",
                                        CurrentModuleObject,
                                        standAloneERV.Name,
                                        cNumericFields(1)));
                ShowContinueError(state,
                                  format("... Entered value={:.2R}... Fan [{} \"{}\"] Max Value = {:.2R}",
                                         standAloneERV.SupplyAirVolFlow,
                                         HVAC::fanTypeNames[(int)standAloneERV.supplyAirFanType],
                                         standAloneERV.SupplyAirFanName,
                                         standAloneERV.DesignSAFanVolFlowRate));
                ShowContinueError(state,
                                  format(" The ERV {} is reset to the supply air fan flow rate and the simulation continues.", cNumericFields(1)));
                standAloneERV.SupplyAirVolFlow = standAloneERV.DesignSAFanVolFlowRate;
            }
        }
        if (standAloneERV.SupplyAirVolFlow != DataSizing::AutoSize) {
            if (standAloneERV.SupplyAirVolFlow <= 0.0) {
                ShowSevereError(state,
                                format("{} = {} has a {} <= 0.0, it must be >0.0", CurrentModuleObject, standAloneERV.Name, cNumericFields(1)));
                ShowContinueError(state, format("... Entered value={:.2R}", standAloneERV.SupplyAirVolFlow));
                ErrorsFound = true;
            }
        } else {
            if (standAloneERV.AirVolFlowPerFloorArea == 0.0 && standAloneERV.AirVolFlowPerOccupant == 0.0) {
                ShowSevereError(state, format("{} \"{}\"", CurrentModuleObject, standAloneERV.Name));
                ShowContinueError(
                    state,
                    format("... Autosizing {} requires at least one input for {} or {}.", cNumericFields(1), cNumericFields(3), cNumericFields(4)));
                ErrorsFound = true;
            }
            // both inputs must be autosized
            if (standAloneERV.ExhaustAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} \"{}\"", CurrentModuleObject, standAloneERV.Name));
                ShowContinueError(state, format("... When autosizing, {} and {} must both be autosized.", cNumericFields(1), cNumericFields(2)));
                ErrorsFound = true;
            }
        }

        // Compare the ERV EA flow rates to EA fan object.
        if (standAloneERV.DesignEAFanVolFlowRate != DataSizing::AutoSize && standAloneERV.ExhaustAirVolFlow != DataSizing::AutoSize) {
            if (standAloneERV.ExhaustAirVolFlow > standAloneERV.DesignEAFanVolFlowRate) {
                ShowWarningError(state,
                                 format("{} = {} has an {} > Max Volume Flow Rate defined in the associated fan object, should be <=",
                                        CurrentModuleObject,
                                        standAloneERV.Name,
                                        cNumericFields(2)));
                ShowContinueError(state,
                                  format("... Entered value={:.2R}... Fan [{}:{}] Max Value = {:.2R}",
                                         standAloneERV.ExhaustAirVolFlow,
                                         HVAC::fanTypeNames[(int)standAloneERV.exhaustAirFanType],
                                         standAloneERV.ExhaustAirFanName,
                                         standAloneERV.DesignEAFanVolFlowRate));
                ShowContinueError(state,
                                  format(" The ERV {} is reset to the exhaust air fan flow rate and the simulation continues.", cNumericFields(2)));
                standAloneERV.ExhaustAirVolFlow = standAloneERV.DesignEAFanVolFlowRate;
            }
        }
        if (standAloneERV.ExhaustAirVolFlow != DataSizing::AutoSize) {
            if (standAloneERV.ExhaustAirVolFlow <= 0.0) {
                ShowSevereError(state,
                                format("{} = {} has an {} <= 0.0, it must be >0.0", CurrentModuleObject, standAloneERV.Name, cNumericFields(2)));
                ShowContinueError(state, format("... Entered value={:.2R}", standAloneERV.ExhaustAirVolFlow));
                ErrorsFound = true;
            }
        } else {
            if (standAloneERV.AirVolFlowPerFloorArea == 0.0 && standAloneERV.AirVolFlowPerOccupant == 0.0) {
                ShowSevereError(state, format("{} \"{}\"", CurrentModuleObject, standAloneERV.Name));
                ShowContinueError(
                    state,
                    format("... Autosizing {} requires at least one input for {} or {}.", cNumericFields(2), cNumericFields(3), cNumericFields(4)));
                ErrorsFound = true;
            }
            if (standAloneERV.SupplyAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} \"{}\"", CurrentModuleObject, standAloneERV.Name));
                ShowContinueError(state, format("... When autosizing, {} and {} must both be autosized.", cNumericFields(1), cNumericFields(2)));
                ErrorsFound = true;
            }
        }

        // Add supply fan to component sets array
        std::string CompSetSupplyFanInlet = "UNDEFINED";
        std::string CompSetSupplyFanOutlet = state.dataLoopNodes->NodeID(standAloneERV.SupplyAirOutletNode);

        // Add exhaust fan to component sets array
        std::string CompSetExhaustFanInlet = "UNDEFINED";
        std::string CompSetExhaustFanOutlet = state.dataLoopNodes->NodeID(standAloneERV.ExhaustAirOutletNode);

        // Add HX to component sets array
        BranchNodeConnections::SetUpCompSets(
            state, standAloneERV.UnitType, standAloneERV.Name, "UNDEFINED", standAloneERV.HeatExchangerName, "UNDEFINED", "UNDEFINED");
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchAirHRZoneHVACName, standAloneERV.HeatExchangerName, standAloneERV.Name);

        // Add supply fan to component sets array
        BranchNodeConnections::SetUpCompSets(state,
                                             standAloneERV.UnitType,
                                             standAloneERV.Name,
                                             "UNDEFINED",
                                             standAloneERV.SupplyAirFanName,
                                             CompSetSupplyFanInlet,
                                             CompSetSupplyFanOutlet);

        // Add exhaust fan to component sets array
        BranchNodeConnections::SetUpCompSets(state,
                                             standAloneERV.UnitType,
                                             standAloneERV.Name,
                                             "UNDEFINED",
                                             standAloneERV.ExhaustAirFanName,
                                             CompSetExhaustFanInlet,
                                             CompSetExhaustFanOutlet);

        // Verify HX name in Stand Alone ERV object matches name of valid HX object
        if (state.dataInputProcessing->inputProcessor->getObjectItemNum(
                state, "HeatExchanger:AirToAir:SensibleAndLatent", standAloneERV.HeatExchangerName) <= 0) {
            ShowSevereError(state,
                            format("{} heat exchanger type HeatExchanger:AirToAir:SensibleAndLatent not found = {}",
                                   CurrentModuleObject,
                                   standAloneERV.HeatExchangerName));
            ErrorsFound = true;
        }
        // Verify supply air fan name in Stand Alone ERV object matches name of valid fan object
        if (standAloneERV.supplyAirFanType != HVAC::FanType::SystemModel) {
            if (state.dataInputProcessing->inputProcessor->getObjectItemNum(state, "Fan:OnOff", standAloneERV.SupplyAirFanName) <= 0) {
                ShowSevereError(state, format("{} supply fan type Fan:OnOff not found = {}", CurrentModuleObject, standAloneERV.SupplyAirFanName));
                ErrorsFound = true;
            }
        } else {
            if (state.dataInputProcessing->inputProcessor->getObjectItemNum(state, "Fan:SystemModel", standAloneERV.SupplyAirFanName) <= 0) {
                ShowSevereError(state,
                                format("{} supply fan type Fan:SystemModel not found = {}", CurrentModuleObject, standAloneERV.SupplyAirFanName));
                ErrorsFound = true;
            }
        }

        // Verify exhaust air fan name in Stand Alone ERV object matches name of valid fan object
        if (standAloneERV.exhaustAirFanType != HVAC::FanType::SystemModel) {
            if (state.dataInputProcessing->inputProcessor->getObjectItemNum(state, "Fan:OnOff", standAloneERV.ExhaustAirFanName) <= 0) {
                ShowSevereError(state, format("{} exhaust fan type Fan:OnOff not found = {}", CurrentModuleObject, standAloneERV.ExhaustAirFanName));
                ErrorsFound = true;
            }
        } else {
            if (state.dataInputProcessing->inputProcessor->getObjectItemNum(state, "Fan:SystemModel", standAloneERV.ExhaustAirFanName) <= 0) {
                ShowSevereError(state,
                                format("{} exhaust fan type Fan:SystemModel not found = {}", CurrentModuleObject, standAloneERV.ExhaustAirFanName));
                ErrorsFound = true;
            }
        }
    }

    int OutAirNum = 0;
    CurrentModuleObject = "ZoneHVAC:EnergyRecoveryVentilator:Controller";
    NumERVCtrlrs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);

    for (ERVControllerNum = 1; ERVControllerNum <= NumERVCtrlrs; ++ERVControllerNum) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 ERVControllerNum,
                                                                 Alphas,
                                                                 NumAlphas,
                                                                 Numbers,
                                                                 NumNumbers,
                                                                 IOStatus,
                                                                 lNumericBlanks,
                                                                 lAlphaBlanks,
                                                                 cAlphaFields,
                                                                 cNumericFields);
        MixedAir::CheckOAControllerName(state, Alphas(1), CurrentModuleObject, cAlphaFields(1), ErrorsFound);
        ++OutAirNum;
        auto &thisOAController = state.dataMixedAir->OAController(OutAirNum);

        thisOAController.Name = Alphas(1);
        thisOAController.ControllerType = MixedAir::MixedAirControllerType::ControllerStandAloneERV;
        int WhichERV = Util::FindItemInList(Alphas(1), state.dataHVACStandAloneERV->StandAloneERV, &StandAloneERVData::ControllerName);
        if (WhichERV != 0) {
            AirFlowRate = state.dataHVACStandAloneERV->StandAloneERV(WhichERV).SupplyAirVolFlow;
            state.dataHVACStandAloneERV->StandAloneERV(WhichERV).ControllerIndex = OutAirNum;
        } else {
            ShowSevereError(
                state, format("GetERVController: Could not find ZoneHVAC:EnergyRecoveryVentilator with {} = \"{}\"", cAlphaFields(1), Alphas(1)));
            ErrorsFound = true;
            AirFlowRate = -1000.0;
        }
        thisOAController.MaxOA = AirFlowRate;
        thisOAController.MinOA = AirFlowRate;
        //    OAController(OutAirNum)%TempLim = Numbers(1)
        if (lNumericBlanks(1)) {
            thisOAController.TempLim = HVAC::BlankNumeric;
        } else {
            thisOAController.TempLim = Numbers(1);
        }
        //    OAController(OutAirNum)%TempLowLim = Numbers(2)
        if (lNumericBlanks(2)) {
            thisOAController.TempLowLim = HVAC::BlankNumeric;
        } else {
            thisOAController.TempLowLim = Numbers(2);
        }
        //    OAController(OutAirNum)%EnthLim = Numbers(3)
        if (lNumericBlanks(3)) {
            thisOAController.EnthLim = HVAC::BlankNumeric;
        } else {
            thisOAController.EnthLim = Numbers(3);
        }
        //    OAController(OutAirNum)%DPTempLim = Numbers(4)
        if (lNumericBlanks(4)) {
            thisOAController.DPTempLim = HVAC::BlankNumeric;
        } else {
            thisOAController.DPTempLim = Numbers(4);
        }

        if (WhichERV != 0) {
            NodeNumber = state.dataHVACStandAloneERV->StandAloneERV(WhichERV).SupplyAirInletNode;
        } else {
            NodeNumber = 0;
        }
        thisOAController.OANode = NodeNumber;
        // set the inlet node to also equal the OA node because this is a special controller for economizing stand alone ERV
        // with the assumption that equipment is bypassed....(moved from module MixedAir)
        thisOAController.InletNode = NodeNumber;

        if (WhichERV != 0) {
            NodeNumber = state.dataHVACStandAloneERV->StandAloneERV(WhichERV).ExhaustAirInletNode;
        } else {
            NodeNumber = 0;
        }
        thisOAController.RetNode = NodeNumber;

        if (!lAlphaBlanks(2)) {
            thisOAController.EnthalpyCurvePtr = Curve::GetCurveIndex(state, Alphas(2));
            if (Curve::GetCurveIndex(state, Alphas(2)) == 0) {
                ShowSevereError(state, format("{} \"{}\"", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("...{} not found:{}", cAlphaFields(2), Alphas(2)));
                ErrorsFound = true;
            } else {
                // Verify Curve Object, only legal types are Quadratic and Cubic
                ErrorsFound |= Curve::CheckCurveDims(state,
                                                     thisOAController.EnthalpyCurvePtr, // Curve index
                                                     {1},                               // Valid dimensions
                                                     "GetStandAloneERV: ",              // Routine name
                                                     CurrentModuleObject,               // Object Type
                                                     thisOAController.Name,             // Object Name
                                                     cAlphaFields(2));                  // Field Name
            }
        }

        // Changed by AMIT for new implementation of the controller:outside air
        if (Alphas(3) == "EXHAUSTAIRTEMPERATURELIMIT" && Alphas(4) == "EXHAUSTAIRENTHALPYLIMIT") {
            thisOAController.Econo = MixedAir::EconoOp::DifferentialDryBulbAndEnthalpy;
        } else if (Alphas(3) == "EXHAUSTAIRTEMPERATURELIMIT" && Alphas(4) == "NOEXHAUSTAIRENTHALPYLIMIT") {
            thisOAController.Econo = MixedAir::EconoOp::DifferentialDryBulb;
        } else if (Alphas(3) == "NOEXHAUSTAIRTEMPERATURELIMIT" && Alphas(4) == "EXHAUSTAIRENTHALPYLIMIT") {
            thisOAController.Econo = MixedAir::EconoOp::DifferentialEnthalpy;
        } else if (Alphas(3) == "NOEXHAUSTAIRTEMPERATURELIMIT" && Alphas(4) == "NOEXHAUSTAIRENTHALPYLIMIT") {
            if ((!lNumericBlanks(1)) || (!lNumericBlanks(3)) || (!lNumericBlanks(4)) || (!lAlphaBlanks(2))) {
                // This means that any of the FIXED DRY BULB, FIXED ENTHALPY, FIXED DEW POINT AND DRY BULB OR
                // ELECTRONIC ENTHALPY ECONOMIZER STRATEGY is present
                thisOAController.Econo = MixedAir::EconoOp::FixedDryBulb;
            }
        } else if ((!lAlphaBlanks(3)) && (!lAlphaBlanks(4))) {
            if ((lNumericBlanks(1)) && (lNumericBlanks(3)) && (lNumericBlanks(4)) && lAlphaBlanks(2)) {
                ShowWarningError(state, format("{} \"{}\"", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("... Invalid {}{} = {}{}", cAlphaFields(3), cAlphaFields(4), Alphas(3), Alphas(4)));
                ShowContinueError(state, "... Assumed NO EXHAUST AIR TEMP LIMIT and NO EXHAUST AIR ENTHALPY LIMIT.");
                thisOAController.Econo = MixedAir::EconoOp::NoEconomizer;
            } else {
                // This means that any of the FIXED DRY BULB, FIXED ENTHALPY, FIXED DEW POINT AND DRY BULB OR
                // ELECTRONIC ENTHALPY ECONOMIZER STRATEGY is present
                thisOAController.Econo = MixedAir::EconoOp::FixedDryBulb;
            }
        } else if ((lAlphaBlanks(3)) && (!lAlphaBlanks(4))) {
            if ((lNumericBlanks(1)) && (lNumericBlanks(3)) && (lNumericBlanks(4)) && lAlphaBlanks(2)) {
                ShowWarningError(state, format("{} \"{}\"", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("... Invalid {} = {}", cAlphaFields(4), Alphas(4)));
                ShowContinueError(state, "... Assumed  NO EXHAUST AIR ENTHALPY LIMIT.");
                thisOAController.Econo = MixedAir::EconoOp::NoEconomizer;
            } else {
                // This means that any of the FIXED DRY BULB, FIXED ENTHALPY, FIXED DEW POINT AND DRY BULB OR
                // ELECTRONIC ENTHALPY ECONOMIZER STRATEGY is present
                thisOAController.Econo = MixedAir::EconoOp::FixedDryBulb;
            }
        } else if ((!lAlphaBlanks(3)) && (lAlphaBlanks(4))) {
            if ((lNumericBlanks(1)) && (lNumericBlanks(3)) && (lNumericBlanks(4)) && lAlphaBlanks(2)) {
                ShowWarningError(state, format("{} \"{}\"", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("... Invalid {} = {}", cAlphaFields(3), Alphas(3)));
                ShowContinueError(state, "... Assumed NO EXHAUST AIR TEMP LIMIT ");
                thisOAController.Econo = MixedAir::EconoOp::NoEconomizer;
            } else {
                // This means that any of the FIXED DRY BULB, FIXED ENTHALPY, FIXED DEW POINT AND DRY BULB OR
                // ELECTRONIC ENTHALPY ECONOMIZER STRATEGY is present
                thisOAController.Econo = MixedAir::EconoOp::FixedDryBulb;
            }
        } else { // NO Economizer
            thisOAController.Econo = MixedAir::EconoOp::NoEconomizer;
        }

        thisOAController.FixedMin = false;
        thisOAController.EconBypass = true;

        //   Initialize to one in case high humidity control is NOT used
        Real64 HighRHOARatio = 1.0;
        //   READ Modify Air Flow Data
        //   High humidity control option is YES, read in additional data
        if (Util::SameString(Alphas(6), "Yes")) {

            HStatZoneNum = Util::FindItemInList(Alphas(7), state.dataHeatBal->Zone);
            thisOAController.HumidistatZoneNum = HStatZoneNum;

            // Get the node number for the zone with the humidistat
            if (HStatZoneNum > 0) {
                bool ZoneNodeFound = false;
                if (state.dataZoneEquip->ZoneEquipConfig(HStatZoneNum).IsControlled) {
                    //         Find the controlled zone number for the specified humidistat location
                    thisOAController.NodeNumofHumidistatZone = state.dataZoneEquip->ZoneEquipConfig(HStatZoneNum).ZoneNode;
                    ZoneNodeFound = true;
                }
                if (!ZoneNodeFound) {
                    ShowSevereError(state, format("{} \"{}\"", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, "... Did not find Air Node (Zone with Humidistat)");
                    ShowContinueError(state, format("... Specified {} = {}", cAlphaFields(7), Alphas(7)));
                    ShowContinueError(state, "... A ZoneHVAC:EquipmentConnections object must be specified for this zone.");
                    ErrorsFound = true;
                } else {
                    bool HStatFound = false;
                    for (NumHstatZone = 1; NumHstatZone <= state.dataZoneCtrls->NumHumidityControlZones; ++NumHstatZone) {
                        if (state.dataZoneCtrls->HumidityControlZone(NumHstatZone).ActualZoneNum != HStatZoneNum) {
                            continue;
                        }
                        HStatFound = true;
                        break;
                    }
                    if (!HStatFound) {
                        ShowSevereError(state, format("{} \"{}\"", CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state, "... Did not find zone humidistat");
                        ShowContinueError(state, "... A ZoneControl:Humidistat object must be specified for this zone.");
                        ErrorsFound = true;
                    }
                }
            } else {
                ShowSevereError(state, format("{} \"{}\"", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, "... Did not find Air Node (Zone with Humidistat)");
                ShowContinueError(state, "... A ZoneHVAC:EquipmentConnections object must be specified for this zone.");
                ErrorsFound = true;
            }

            if (Numbers(5) <= 0.0 && NumNumbers > 4) {

                ShowWarningError(state, format("{} \"{}\"", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("... {} must be greater than 0.", cNumericFields(5)));
                ShowContinueError(state, format("... {} is reset to 1 and the simulation continues.", cNumericFields(5)));

                HighRHOARatio = 1.0;

            } else if (NumNumbers > 4) {

                HighRHOARatio = Numbers(5);

            } else {

                HighRHOARatio = 1.0;
            }

            if (Util::SameString(Alphas(8), "Yes")) {
                thisOAController.ModifyDuringHighOAMoisture = false;
            } else {
                thisOAController.ModifyDuringHighOAMoisture = true;
            }

        } else if (!Util::SameString(Alphas(6), "No") && NumAlphas > 4 && (!lAlphaBlanks(5))) {
            ShowWarningError(state, format("{} \"{}\"", CurrentModuleObject, Alphas(1)));
            ShowContinueError(state, format("... Invalid {} = {}", cAlphaFields(6), Alphas(6)));
            ShowContinueError(state, format("... {} is assumed to be \"No\" and the simulation continues.", cAlphaFields(6)));
        } // IF(Util::SameString(Alphas(6),'Yes'))THEN

        thisOAController.HighRHOAFlowRatio = HighRHOARatio;
        if (WhichERV != 0) {
            state.dataHVACStandAloneERV->StandAloneERV(WhichERV).HighRHOAFlowRatio = HighRHOARatio;
        }

        //   Check for a time of day outside air schedule
        thisOAController.economizerOASched = Sched::GetSchedule(state, Alphas(5));

        if (WhichERV != 0) {
            state.dataHVACStandAloneERV->StandAloneERV(WhichERV).economizerOASched = Sched::GetSchedule(state, Alphas(5));

            // Compare the ERV SA fan flow rates to modified air flow rate.
            if (HighRHOARatio > 1.0 && state.dataHVACStandAloneERV->StandAloneERV(WhichERV).SupplyAirVolFlow != DataSizing::AutoSize &&
                state.dataHVACStandAloneERV->StandAloneERV(WhichERV).DesignSAFanVolFlowRate != DataSizing::AutoSize) {
                if (state.dataHVACStandAloneERV->StandAloneERV(WhichERV).SupplyAirVolFlow * HighRHOARatio >
                    state.dataHVACStandAloneERV->StandAloneERV(WhichERV).DesignSAFanVolFlowRate) {
                    ShowWarningError(state, format("{} \"{}\"", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format("... A {} was entered as {:.4R}", cNumericFields(5), HighRHOARatio));
                    ShowContinueError(state,
                                      "... This flow ratio results in a Supply Air Volume Flow Rate through the ERV which is greater than the "
                                      "Max Volume specified in the supply air fan object.");
                    ShowContinueError(state,
                                      format("... Associated fan object = {} \"{}\"",
                                             HVAC::fanTypeNames[(int)state.dataHVACStandAloneERV->StandAloneERV(WhichERV).supplyAirFanType],
                                             state.dataHVACStandAloneERV->StandAloneERV(WhichERV).SupplyAirFanName));
                    ShowContinueError(state,
                                      format("... Modified value                   = {:.2R}",
                                             state.dataHVACStandAloneERV->StandAloneERV(WhichERV).SupplyAirVolFlow * HighRHOARatio));
                    ShowContinueError(state,
                                      format(" ... Supply Fan Max Volume Flow Rate = {:.2R}",
                                             state.dataHVACStandAloneERV->StandAloneERV(WhichERV).DesignSAFanVolFlowRate));
                    ShowContinueError(state, "... The ERV supply air fan will limit the air flow through the ERV and the simulation continues.");
                }
            }

            // Compare the ERV EA fan flow rates to modified air flow rate.
            if (HighRHOARatio > 1.0 && state.dataHVACStandAloneERV->StandAloneERV(WhichERV).ExhaustAirVolFlow != DataSizing::AutoSize &&
                state.dataHVACStandAloneERV->StandAloneERV(WhichERV).DesignEAFanVolFlowRate != DataSizing::AutoSize) {
                if (state.dataHVACStandAloneERV->StandAloneERV(WhichERV).ExhaustAirVolFlow * HighRHOARatio >
                    state.dataHVACStandAloneERV->StandAloneERV(WhichERV).DesignEAFanVolFlowRate) {
                    ShowWarningError(state, format("ZoneHVAC:EnergyRecoveryVentilator:Controller \"{}\"", Alphas(1)));
                    ShowContinueError(state, format("... A {} was entered as {:.4R}", cNumericFields(5), HighRHOARatio));
                    ShowContinueError(state,
                                      "... This flow ratio results in an Exhaust Air Volume Flow Rate through the ERV which is greater than the "
                                      "Max Volume specified in the exhaust air fan object.");
                    ShowContinueError(state,
                                      format("... Associated fan object = {} \"{}\"",
                                             HVAC::fanTypeNames[(int)state.dataHVACStandAloneERV->StandAloneERV(WhichERV).exhaustAirFanType],
                                             state.dataHVACStandAloneERV->StandAloneERV(WhichERV).ExhaustAirFanName));
                    ShowContinueError(state,
                                      format("... Modified value                    = {:.2R}",
                                             state.dataHVACStandAloneERV->StandAloneERV(WhichERV).ExhaustAirVolFlow * HighRHOARatio));
                    ShowContinueError(state,
                                      format(" ... Exhaust Fan Max Volume Flow Rate = {:.2R}",
                                             state.dataHVACStandAloneERV->StandAloneERV(WhichERV).DesignEAFanVolFlowRate));
                    ShowContinueError(state, "... The ERV exhaust air fan will limit the air flow through the ERV and the simulation continues.");
                }
            }
        } // IF(WhichERV /= 0)THEN
    }

    if (ErrorsFound) {
        ShowFatalError(state, "Errors found in getting ZoneHVAC:EnergyRecoveryVentilator input.");
    }

    // Setup report variables for the stand alone ERVs
    for (int StandAloneERVIndex = 1; StandAloneERVIndex <= state.dataHVACStandAloneERV->NumStandAloneERVs; ++StandAloneERVIndex) {
        auto &standAloneERV = state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVIndex);
        SetupOutputVariable(state,
                            "Zone Ventilator Sensible Cooling Rate",
                            Constant::Units::W,
                            standAloneERV.SensCoolingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            standAloneERV.Name);
        SetupOutputVariable(state,
                            "Zone Ventilator Sensible Cooling Energy",
                            Constant::Units::J,
                            standAloneERV.SensCoolingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            standAloneERV.Name);
        SetupOutputVariable(state,
                            "Zone Ventilator Latent Cooling Rate",
                            Constant::Units::W,
                            standAloneERV.LatCoolingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            standAloneERV.Name);
        SetupOutputVariable(state,
                            "Zone Ventilator Latent Cooling Energy",
                            Constant::Units::J,
                            standAloneERV.LatCoolingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            standAloneERV.Name);
        SetupOutputVariable(state,
                            "Zone Ventilator Total Cooling Rate",
                            Constant::Units::W,
                            standAloneERV.TotCoolingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            standAloneERV.Name);
        SetupOutputVariable(state,
                            "Zone Ventilator Total Cooling Energy",
                            Constant::Units::J,
                            standAloneERV.TotCoolingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            standAloneERV.Name);

        SetupOutputVariable(state,
                            "Zone Ventilator Sensible Heating Rate",
                            Constant::Units::W,
                            standAloneERV.SensHeatingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            standAloneERV.Name);
        SetupOutputVariable(state,
                            "Zone Ventilator Sensible Heating Energy",
                            Constant::Units::J,
                            standAloneERV.SensHeatingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            standAloneERV.Name);
        SetupOutputVariable(state,
                            "Zone Ventilator Latent Heating Rate",
                            Constant::Units::W,
                            standAloneERV.LatHeatingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            standAloneERV.Name);
        SetupOutputVariable(state,
                            "Zone Ventilator Latent Heating Energy",
                            Constant::Units::J,
                            standAloneERV.LatHeatingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            standAloneERV.Name);
        SetupOutputVariable(state,
                            "Zone Ventilator Total Heating Rate",
                            Constant::Units::W,
                            standAloneERV.TotHeatingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            standAloneERV.Name);
        SetupOutputVariable(state,
                            "Zone Ventilator Total Heating Energy",
                            Constant::Units::J,
                            standAloneERV.TotHeatingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            standAloneERV.Name);

        SetupOutputVariable(state,
                            "Zone Ventilator Electricity Rate",
                            Constant::Units::W,
                            standAloneERV.ElecUseRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            standAloneERV.Name);
        SetupOutputVariable(state,
                            "Zone Ventilator Electricity Energy",
                            Constant::Units::J,
                            standAloneERV.ElecUseEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            standAloneERV.Name);
        SetupOutputVariable(state,
                            "Zone Ventilator Supply Fan Availability Status",
                            Constant::Units::None,
                            standAloneERV.availStatus,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            standAloneERV.Name);
    }

    Alphas.deallocate();
    Numbers.deallocate();
    cAlphaFields.deallocate();
    cNumericFields.deallocate();
    lNumericBlanks.deallocate();
    lAlphaBlanks.deallocate();
}

void InitStandAloneERV(EnergyPlusData &state,
                       int const StandAloneERVNum,   // number of the current Stand Alone ERV unit being simulated
                       int const ZoneNum,            // number of zone being served unused1208
                       bool const FirstHVACIteration // TRUE if first HVAC iteration
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad, FSEC
    //       DATE WRITTEN   June 2003
    //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone sys avail managers

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is for initializations of the Stand Alone ERV unit information.

    // METHODOLOGY EMPLOYED:
    // Uses the status flags to trigger initializations.

    // Do the one time initializations
    if (state.dataHVACStandAloneERV->MyOneTimeFlag) {

        state.dataHVACStandAloneERV->MyEnvrnFlag.allocate(state.dataHVACStandAloneERV->NumStandAloneERVs);
        state.dataHVACStandAloneERV->MySizeFlag_InitStandAloneERV.allocate(state.dataHVACStandAloneERV->NumStandAloneERVs);
        state.dataHVACStandAloneERV->MyZoneEqFlag.allocate(state.dataHVACStandAloneERV->NumStandAloneERVs);
        state.dataHVACStandAloneERV->MyEnvrnFlag = true;
        state.dataHVACStandAloneERV->MySizeFlag_InitStandAloneERV = true;
        state.dataHVACStandAloneERV->MyZoneEqFlag = true;
        state.dataHVACStandAloneERV->MyOneTimeFlag = false;
    }

    if (allocated(state.dataAvail->ZoneComp)) {
        auto &availMgr = state.dataAvail->ZoneComp(DataZoneEquipment::ZoneEquipType::EnergyRecoveryVentilator).ZoneCompAvailMgrs(StandAloneERVNum);
        if (state.dataHVACStandAloneERV->MyZoneEqFlag(StandAloneERVNum)) { // initialize the name of each availability manager list and zone number
            availMgr.AvailManagerListName = state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).AvailManagerListName;
            availMgr.ZoneNum = ZoneNum;
            state.dataHVACStandAloneERV->MyZoneEqFlag(StandAloneERVNum) = false;
        }
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).availStatus = availMgr.availStatus;
    }

    // need to check all units to see if they are on Zone Equipment List or issue warning
    if (!state.dataHVACStandAloneERV->ZoneEquipmentListChecked && state.dataZoneEquip->ZoneEquipInputsFilled) {
        state.dataHVACStandAloneERV->ZoneEquipmentListChecked = true;
        for (int Loop = 1; Loop <= state.dataHVACStandAloneERV->NumStandAloneERVs; ++Loop) {
            if (DataZoneEquipment::CheckZoneEquipmentList(
                    state, state.dataHVACStandAloneERV->StandAloneERV(Loop).UnitType, state.dataHVACStandAloneERV->StandAloneERV(Loop).Name)) {
                continue;
            }
            ShowSevereError(state,
                            format("InitStandAloneERV: Unit=[{},{}] is not on any ZoneHVAC:EquipmentList.  It will not be simulated.",
                                   state.dataHVACStandAloneERV->StandAloneERV(Loop).UnitType,
                                   state.dataHVACStandAloneERV->StandAloneERV(Loop).Name));
        }
    }

    if (!state.dataGlobal->SysSizingCalc && state.dataHVACStandAloneERV->MySizeFlag_InitStandAloneERV(StandAloneERVNum)) {
        SizeStandAloneERV(state, StandAloneERVNum);
        state.dataHVACStandAloneERV->MySizeFlag_InitStandAloneERV(StandAloneERVNum) = false;
    }

    // Do the Begin Environment initializations
    if (state.dataGlobal->BeginEnvrnFlag && state.dataHVACStandAloneERV->MyEnvrnFlag(StandAloneERVNum)) {
        int SupInNode = state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SupplyAirInletNode;
        int ExhInNode = state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ExhaustAirInletNode;
        // set the mass flow rates from the input volume flow rates
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).MaxSupAirMassFlow =
            state.dataEnvrn->StdRhoAir * state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SupplyAirVolFlow;
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).MaxExhAirMassFlow =
            state.dataEnvrn->StdRhoAir * state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ExhaustAirVolFlow;
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).DesignSAFanMassFlowRate =
            state.dataEnvrn->StdRhoAir * state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).DesignSAFanVolFlowRate;
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).DesignEAFanMassFlowRate =
            state.dataEnvrn->StdRhoAir * state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).DesignEAFanVolFlowRate;
        // set the node max and min mass flow rates
        auto &supInNode = state.dataLoopNodes->Node(SupInNode);
        auto &exhInNode = state.dataLoopNodes->Node(ExhInNode);

        supInNode.MassFlowRateMax = state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).MaxSupAirMassFlow;
        supInNode.MassFlowRateMin = 0.0;
        exhInNode.MassFlowRateMax = state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).MaxExhAirMassFlow;
        exhInNode.MassFlowRateMin = 0.0;
        state.dataHVACStandAloneERV->MyEnvrnFlag(StandAloneERVNum) = false;
        //   Initialize OA Controller on BeginEnvrnFlag
        if (state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ControllerNameDefined) {
            MixedAir::SimOAController(state,
                                      state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ControllerName,
                                      state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ControllerIndex,
                                      FirstHVACIteration,
                                      0);
        }
    } // end one time inits

    if (!state.dataGlobal->BeginEnvrnFlag) {
        state.dataHVACStandAloneERV->MyEnvrnFlag(StandAloneERVNum) = true;
    }

    // These initializations are done every iteration
    state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ElecUseRate = 0.0;
    state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SensCoolingRate = 0.0;
    state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).LatCoolingRate = 0.0;
    state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).TotCoolingRate = 0.0;
    state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SensHeatingRate = 0.0;
    state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).LatHeatingRate = 0.0;
    state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).TotHeatingRate = 0.0;
    int SupInNode = state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SupplyAirInletNode;
    int ExhInNode = state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ExhaustAirInletNode;
    auto &supInNode = state.dataLoopNodes->Node(SupInNode);
    auto &exhInNode = state.dataLoopNodes->Node(ExhInNode);

    // Set the inlet node mass flow rate
    if (state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).availSched->getCurrentVal() > 0.0) {

        //   IF optional ControllerName is defined SimOAController ONLY to set economizer and Modifyairflow flags
        if (state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ControllerNameDefined) {
            //     Initialize a flow rate for controller
            supInNode.MassFlowRate = state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).MaxSupAirMassFlow;
            MixedAir::SimOAController(state,
                                      state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ControllerName,
                                      state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ControllerIndex,
                                      FirstHVACIteration,
                                      0);
        }

        if (state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).supplyAirFanSched->getCurrentVal() > 0 ||
            (state.dataHVACGlobal->TurnFansOn && !state.dataHVACGlobal->TurnFansOff)) {
            if (state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ControllerNameDefined) {
                if (state.dataMixedAir->OAController(state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ControllerIndex)
                        .HighHumCtrlActive) {
                    supInNode.MassFlowRate = min(state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).DesignSAFanMassFlowRate,
                                                 state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).MaxSupAirMassFlow *
                                                     state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).HighRHOAFlowRatio);
                } else {
                    supInNode.MassFlowRate = min(state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).DesignSAFanMassFlowRate,
                                                 state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).MaxSupAirMassFlow);
                }
            } else {
                supInNode.MassFlowRate = min(state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).DesignSAFanMassFlowRate,
                                             state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).MaxSupAirMassFlow);
            }
        } else {
            supInNode.MassFlowRate = 0.0;
        }
        supInNode.MassFlowRateMaxAvail = supInNode.MassFlowRate;
        supInNode.MassFlowRateMinAvail = supInNode.MassFlowRate;

        if (state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).exhaustAirFanSched->getCurrentVal() > 0) {
            if (state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ControllerNameDefined) {
                if (state.dataMixedAir->OAController(state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ControllerIndex)
                        .HighHumCtrlActive) {
                    exhInNode.MassFlowRate = min(state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).DesignEAFanMassFlowRate,
                                                 state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).MaxExhAirMassFlow *
                                                     state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).HighRHOAFlowRatio);
                } else {
                    exhInNode.MassFlowRate = min(state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).DesignEAFanMassFlowRate,
                                                 state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).MaxExhAirMassFlow);
                }
            } else {
                exhInNode.MassFlowRate = min(state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).DesignEAFanMassFlowRate,
                                             state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).MaxExhAirMassFlow);
            }
        } else {
            exhInNode.MassFlowRate = 0.0;
        }
        exhInNode.MassFlowRateMaxAvail = exhInNode.MassFlowRate;
        exhInNode.MassFlowRateMinAvail = exhInNode.MassFlowRate;
    } else {
        supInNode.MassFlowRate = 0.0;
        supInNode.MassFlowRateMaxAvail = 0.0;
        supInNode.MassFlowRateMinAvail = 0.0;
        exhInNode.MassFlowRate = 0.0;
        exhInNode.MassFlowRateMaxAvail = 0.0;
        exhInNode.MassFlowRateMinAvail = 0.0;
    }
}

void SizeStandAloneERV(EnergyPlusData &state, int const StandAloneERVNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad
    //       DATE WRITTEN   October 2007
    //       MODIFIED       August 2013 Daeho Kang, add component sizing table entries

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is for sizing Stand Alone ERV Components for which flow rates have not been
    // specified in the input.

    // METHODOLOGY EMPLOYED:
    // Obtains flow rates from the zone or system sizing arrays.

    static constexpr std::string_view RoutineName("SizeStandAloneERV: ");

    bool IsAutoSize = false;
    Real64 SupplyAirVolFlowDes = 0.0;
    Real64 DesignSAFanVolFlowRateDes = 0.0;
    Real64 DesignSAFanVolFlowRateUser = 0.0;
    Real64 ExhaustAirVolFlowDes = 0.0;
    std::string CompType = "ZoneHVAC:EnergyRecoveryVentilator";
    std::string CompName = state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).Name;
    bool PrintFlag = true;
    bool ErrorsFound = false;

    auto &zoneEqSizing = state.dataSize->ZoneEqSizing(state.dataSize->CurZoneEqNum);

    if (state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SupplyAirVolFlow == DataSizing::AutoSize) {
        IsAutoSize = true;
    }

    if (state.dataSize->CurZoneEqNum > 0) {

        //      Sizing objects are not required for stand alone ERV
        //      CALL CheckZoneSizing('ZoneHVAC:EnergyRecoveryVentilator',StandAloneERV(StandAloneERVNum)%Name)
        int ZoneNum = state.dataSize->CurZoneEqNum;
        Real64 ZoneMult = state.dataHeatBal->Zone(ZoneNum).Multiplier * state.dataHeatBal->Zone(ZoneNum).ListMultiplier;
        Real64 FloorArea = state.dataHeatBal->Zone(ZoneNum).FloorArea;
        Real64 NumberOfPeople = 0.0;
        Real64 MaxPeopleSch = 0.0;
        for (int PeopleNum = 1; PeopleNum <= state.dataHeatBal->TotPeople; ++PeopleNum) {
            if (ZoneNum != state.dataHeatBal->People(PeopleNum).ZonePtr) {
                continue;
            }
            MaxPeopleSch = state.dataHeatBal->People(PeopleNum).sched->getMaxVal(state);
            NumberOfPeople = NumberOfPeople + (state.dataHeatBal->People(PeopleNum).NumberOfPeople * MaxPeopleSch);
        }
        SupplyAirVolFlowDes = FloorArea * state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).AirVolFlowPerFloorArea +
                              NumberOfPeople * state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).AirVolFlowPerOccupant;
        SupplyAirVolFlowDes = ZoneMult * SupplyAirVolFlowDes;

        if (SupplyAirVolFlowDes < HVAC::SmallAirVolFlow) {
            SupplyAirVolFlowDes = 0.0;
        }

        // Size ERV supply flow rate
        Real64 TempSize = state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SupplyAirVolFlow;
        if (IsAutoSize) {
            state.dataSize->DataConstantUsedForSizing = SupplyAirVolFlowDes;
            state.dataSize->DataFractionUsedForSizing = 1.0;
            TempSize = SupplyAirVolFlowDes;
            if (state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ControllerNameDefined) {
                state.dataMixedAir->OAController(state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ControllerIndex).MaxOA =
                    SupplyAirVolFlowDes * state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).HighRHOAFlowRatio;
                state.dataMixedAir->OAController(state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ControllerIndex).MinOA =
                    SupplyAirVolFlowDes;
            }
        } else {
            state.dataSize->DataConstantUsedForSizing = state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SupplyAirVolFlow;
            state.dataSize->DataFractionUsedForSizing = 1.0;
        }
        if (TempSize > 0.0) {
            std::string SizingString = "Supply Air Flow Rate [m3/s]";
            SystemAirFlowSizer sizerSystemAirFlow;
            sizerSystemAirFlow.overrideSizingString(SizingString);
            sizerSystemAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
            TempSize = sizerSystemAirFlow.size(state, TempSize, ErrorsFound);
        }
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SupplyAirVolFlow = TempSize;
    }

    // Size ERV exhaust flow rate
    state.dataSize->DataFractionUsedForSizing = 1.0;
    IsAutoSize = false;
    if (state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ExhaustAirVolFlow == DataSizing::AutoSize) {
        IsAutoSize = true;
    }

    if (state.dataSize->CurZoneEqNum > 0) {

        ExhaustAirVolFlowDes = SupplyAirVolFlowDes;

        if (ExhaustAirVolFlowDes < HVAC::SmallAirVolFlow) {
            ExhaustAirVolFlowDes = 0.0;
        }

        if (ExhaustAirVolFlowDes > state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SupplyAirVolFlow) {
            ExhaustAirVolFlowDes = state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SupplyAirVolFlow;
        }

        Real64 TempSize = state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ExhaustAirVolFlow;
        if (IsAutoSize) {
            TempSize = ExhaustAirVolFlowDes;
            state.dataSize->DataConstantUsedForSizing = ExhaustAirVolFlowDes;
        } else {
            state.dataSize->DataConstantUsedForSizing = state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ExhaustAirVolFlow;
        }
        state.dataSize->DataFractionUsedForSizing = 1.0;
        if (TempSize > 0.0) {
            std::string SizingString = "Exhaust Air Flow Rate [m3/s]";
            SystemAirFlowSizer sizerSystemAirFlow;
            sizerSystemAirFlow.overrideSizingString(SizingString);
            sizerSystemAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
            TempSize = sizerSystemAirFlow.size(state, TempSize, ErrorsFound);
        }
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ExhaustAirVolFlow = TempSize;
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).DesignEAFanVolFlowRate =
            TempSize * state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).HighRHOAFlowRatio;
    }

    // Set Zone equipment sizing data for autosizing the fans and heat exchanger
    zoneEqSizing.AirVolFlow = state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SupplyAirVolFlow *
                              state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).HighRHOAFlowRatio;
    zoneEqSizing.OAVolFlow = state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SupplyAirVolFlow;
    zoneEqSizing.SystemAirFlow = true;
    zoneEqSizing.DesignSizeFromParent = true;

    // Check supply fan flow rate or set flow rate if autosized in fan object
    IsAutoSize = false;
    if (state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).DesignSAFanVolFlowRate == DataSizing::AutoSize) {
        IsAutoSize = true;
    }
    DesignSAFanVolFlowRateDes = state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SupplyAirVolFlow *
                                state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).HighRHOAFlowRatio;
    if (IsAutoSize) {
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).DesignSAFanVolFlowRate = DesignSAFanVolFlowRateDes;
    } else {
        if (state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).DesignSAFanVolFlowRate > 0.0 && DesignSAFanVolFlowRateDes > 0.0) {
            DesignSAFanVolFlowRateUser = state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).DesignSAFanVolFlowRate;
            if (state.dataGlobal->DisplayExtraWarnings) {
                if ((std::abs(DesignSAFanVolFlowRateDes - DesignSAFanVolFlowRateUser) / DesignSAFanVolFlowRateUser) >
                    state.dataSize->AutoVsHardSizingThreshold) {
                    ShowMessage(state,
                                format("SizeStandAloneERV: Potential issue with equipment sizing for ZoneHVAC:EnergyRecoveryVentilator {} {}",
                                       HVAC::fanTypeNames[(int)state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).supplyAirFanType],
                                       state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SupplyAirFanName));
                    ShowContinueError(state, format("User-Specified Supply Fan Maximum Flow Rate of {:.5R} [m3/s]", DesignSAFanVolFlowRateUser));
                    ShowContinueError(state, format("differs from the ERV Supply Air Flow Rate of {:.5R} [m3/s]", DesignSAFanVolFlowRateDes));
                    ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                    ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                }
            }
        }
    }

    // simulate the fan to size using the flow rate specified above
    // (i.e., ZoneEqSizing( CurZoneEqNum ).AirVolFlow = StandAloneERV( StandAloneERVNum ).SupplyAirVolFlow * StandAloneERV( StandAloneERVNum
    // ).HighRHOAFlowRatio;)
    state.dataFans->fans(state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SupplyAirFanIndex)->simulate(state, true, _, _);

    state.dataFans->fans(state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ExhaustAirFanIndex)->simulate(state, true, _, _);

    // now reset the ZoneEqSizing variable to NOT use the multiplier for HighRHOAFlowRatio for sizing HXs
    zoneEqSizing.AirVolFlow = state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SupplyAirVolFlow;
}

void CalcStandAloneERV(EnergyPlusData &state,
                       int const StandAloneERVNum,    // Unit index in ERV data structure
                       bool const FirstHVACIteration, // flag for 1st HVAC iteration in the time step
                       Real64 &SensLoadMet,           // sensible zone load met by unit (W)
                       Real64 &LatentMassLoadMet      // latent zone load met by unit (kg/s), dehumid = negative
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad, FSEC
    //       DATE WRITTEN   June 2003
    //       MODIFIED       Don Shirey, Aug 2009 (LatentMassLoadMet)
    //                      July 2012, Chandan Sharma - FSEC: Added zone sys avail managers

    // PURPOSE OF THIS SUBROUTINE:
    // Simulate the components making up the Stand Alone ERV unit.

    // METHODOLOGY EMPLOYED:
    // Simulates the unit components sequentially in the air flow direction.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 TotLoadMet;    // total zone load met by unit (W)
    Real64 LatLoadMet;    // latent zone load met by unit (W)
    bool EconomizerFlag;  // economizer signal from OA controller
    bool HighHumCtrlFlag; // high humditiy control signal from OA controller

    int SupInletNode = state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SupplyAirInletNode;
    int SupOutletNode = state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SupplyAirOutletNode;
    int ExhaustInletNode = state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ExhaustAirInletNode;

    // Stand alone ERV's HX is ON by default
    bool HXUnitOn = true;

    // Get stand alone ERV's controller economizer and high humidity control status
    if (state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ControllerNameDefined) {
        EconomizerFlag = state.dataMixedAir->OAController(state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ControllerIndex).EconoActive;
        HighHumCtrlFlag =
            state.dataMixedAir->OAController(state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ControllerIndex).HighHumCtrlActive;
    } else {
        EconomizerFlag = false;
        HighHumCtrlFlag = false;
    }

    HeatRecovery::SimHeatRecovery(state,
                                  state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).HeatExchangerName,
                                  FirstHVACIteration,
                                  state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).HeatExchangerIndex,
                                  HVAC::FanOp::Continuous,
                                  _,
                                  HXUnitOn,
                                  _,
                                  _,
                                  EconomizerFlag,
                                  HighHumCtrlFlag);
    state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ElecUseRate = state.dataHVACGlobal->AirToAirHXElecPower;

    state.dataFans->fans(state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SupplyAirFanIndex)->simulate(state, FirstHVACIteration, _, _);
    state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ElecUseRate +=
        state.dataFans->fans(state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SupplyAirFanIndex)->totalPower;

    state.dataFans->fans(state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ExhaustAirFanIndex)->simulate(state, FirstHVACIteration, _, _);
    state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ElecUseRate +=
        state.dataFans->fans(state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ExhaustAirFanIndex)->totalPower;

    // total mass flow through supply side of the ERV (supply air outlet node)
    Real64 AirMassFlow = state.dataLoopNodes->Node(SupOutletNode).MassFlowRate;
    CalcZoneSensibleLatentOutput(AirMassFlow,
                                 state.dataLoopNodes->Node(SupOutletNode).Temp,
                                 state.dataLoopNodes->Node(SupOutletNode).HumRat,
                                 state.dataLoopNodes->Node(ExhaustInletNode).Temp,
                                 state.dataLoopNodes->Node(ExhaustInletNode).HumRat,
                                 SensLoadMet,
                                 LatLoadMet,
                                 TotLoadMet);
    LatentMassLoadMet = AirMassFlow * (state.dataLoopNodes->Node(SupOutletNode).HumRat -
                                       state.dataLoopNodes->Node(ExhaustInletNode).HumRat); // kg/s, dehumidification = negative

    if (SensLoadMet < 0.0) {
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SensCoolingRate = std::abs(SensLoadMet);
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SensHeatingRate = 0.0;
    } else {
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SensCoolingRate = 0.0;
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SensHeatingRate = SensLoadMet;
    }
    if (TotLoadMet < 0.0) {
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).TotCoolingRate = std::abs(TotLoadMet);
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).TotHeatingRate = 0.0;
    } else {
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).TotCoolingRate = 0.0;
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).TotHeatingRate = TotLoadMet;
    }
    if (LatLoadMet < 0.0) {
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).LatCoolingRate = std::abs(LatLoadMet);
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).LatHeatingRate = 0.0;
    } else {
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).LatCoolingRate = 0.0;
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).LatHeatingRate = LatLoadMet;
    }

    // Provide a one time message when exhaust flow rate is greater than supply flow rate
    if (state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).FlowError && !state.dataGlobal->WarmupFlag) {
        Real64 TotalExhaustMassFlow = state.dataLoopNodes->Node(ExhaustInletNode).MassFlowRate;
        Real64 TotalSupplyMassFlow = state.dataLoopNodes->Node(SupInletNode).MassFlowRate;
        if (TotalExhaustMassFlow > TotalSupplyMassFlow && !state.dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance) {
            ShowWarningError(state,
                             format("For {} \"{}\" there is unbalanced exhaust air flow.",
                                    state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).UnitType,
                                    state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).Name));
            ShowContinueError(state, format("... The exhaust air mass flow rate = {:.6R}", state.dataLoopNodes->Node(ExhaustInletNode).MassFlowRate));
            ShowContinueError(state, format("... The  supply air mass flow rate = {:.6R}", state.dataLoopNodes->Node(SupInletNode).MassFlowRate));
            ShowContinueErrorTimeStamp(state, "");
            ShowContinueError(state, "... Unless there is balancing infiltration / ventilation air flow, this will result in");
            ShowContinueError(state, "... load due to induced outside air being neglected in the simulation.");
            state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).FlowError = false;
        }
    }
}

void ReportStandAloneERV(EnergyPlusData &state, int const StandAloneERVNum) // number of the current Stand Alone ERV being simulated
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Raustad, FSEC
    //       DATE WRITTEN   June 2003

    Real64 ReportingConstant = state.dataHVACGlobal->TimeStepSysSec;
    state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ElecUseEnergy =
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ElecUseRate * ReportingConstant;
    state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SensCoolingEnergy =
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SensCoolingRate * ReportingConstant;
    state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).LatCoolingEnergy =
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).LatCoolingRate * ReportingConstant;
    state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).TotCoolingEnergy =
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).TotCoolingRate * ReportingConstant;
    state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SensHeatingEnergy =
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SensHeatingRate * ReportingConstant;
    state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).LatHeatingEnergy =
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).LatHeatingRate * ReportingConstant;
    state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).TotHeatingEnergy =
        state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).TotHeatingRate * ReportingConstant;

    if (state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).FirstPass) { // reset sizing flags so other zone equipment can size normally
        if (!state.dataGlobal->SysSizingCalc) {
            DataSizing::resetHVACSizingGlobals(
                state, state.dataSize->CurZoneEqNum, 0, state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).FirstPass);
        }
    }
}

//        Utility subroutines/functions for the HeatingCoil Module

Real64 GetSupplyAirFlowRate(EnergyPlusData &state,
                            std::string const &ERVType,     // must be "ZoneHVAC:EnergyRecoveryVentilator"
                            std::string const &ERVCtrlName, // must match a controller name in the ERV data structure
                            bool &ErrorsFound               // set to true if problem
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   October 2006

    // PURPOSE OF THIS FUNCTION:
    // This function looks up the ERVCtrlName in the ERV Stand Alone list and returns the
    // Supply Air Flow rate, if found.  If incorrect name is given, ErrorsFound is returned as true
    // and supply air flow rate as negative.

    if (state.dataHVACStandAloneERV->GetERVInputFlag) {
        GetStandAloneERV(state);
        state.dataHVACStandAloneERV->GetERVInputFlag = false;
    }

    if (Util::SameString(ERVType, "ZoneHVAC:EnergyRecoveryVentilator")) {
        int WhichERV = Util::FindItem(ERVCtrlName, state.dataHVACStandAloneERV->StandAloneERV, &StandAloneERVData::ControllerName);
        if (WhichERV != 0) {
            return state.dataHVACStandAloneERV->StandAloneERV(WhichERV).SupplyAirVolFlow;
        }
    }

    ShowSevereError(state, format("Could not find ZoneHVAC:EnergyRecoveryVentilator with Controller Name=\"{}\"", ERVCtrlName));
    ErrorsFound = true;
    return -1000.0;
}

int GetStandAloneERVOutAirNode(EnergyPlusData &state, int const StandAloneERVNum)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         B Griffith
    //       DATE WRITTEN   Dec  2006

    // PURPOSE OF THIS FUNCTION:
    // lookup function for OA inlet node for ventilation rate reporting

    if (state.dataHVACStandAloneERV->GetERVInputFlag) {
        GetStandAloneERV(state);
        state.dataHVACStandAloneERV->GetERVInputFlag = false;
    }

    if (StandAloneERVNum > 0 && StandAloneERVNum <= state.dataHVACStandAloneERV->NumStandAloneERVs) {
        return state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SupplyAirInletNode;
    }

    return 0;
}

int GetStandAloneERVZoneInletAirNode(EnergyPlusData &state, int const StandAloneERVNum)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         B Griffith
    //       DATE WRITTEN   Dec  2006

    // PURPOSE OF THIS FUNCTION:
    // lookup function for OA inlet node for ventilation rate reporting

    if (state.dataHVACStandAloneERV->GetERVInputFlag) {
        GetStandAloneERV(state);
        state.dataHVACStandAloneERV->GetERVInputFlag = false;
    }

    if (StandAloneERVNum > 0 && StandAloneERVNum <= state.dataHVACStandAloneERV->NumStandAloneERVs) {
        return state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).SupplyAirOutletNode;
    }

    return 0;
}

int GetStandAloneERVReturnAirNode(EnergyPlusData &state, int const StandAloneERVNum)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         B Griffith
    //       DATE WRITTEN   Dec  2006

    // PURPOSE OF THIS FUNCTION:
    // lookup function for OA inlet node for ventilation rate reporting

    if (state.dataHVACStandAloneERV->GetERVInputFlag) {
        GetStandAloneERV(state);
        state.dataHVACStandAloneERV->GetERVInputFlag = false;
    }

    if (StandAloneERVNum > 0 && StandAloneERVNum <= state.dataHVACStandAloneERV->NumStandAloneERVs) {
        return state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).ExhaustAirInletNode;
    }

    return 0;
}

bool GetStandAloneERVNodeNumber(EnergyPlusData &state, int const NodeNumber)
{
    // PURPOSE OF THIS FUNCTION:
    // Check if a node is used by a stand alone ERV
    // and can be excluded from an airflow network.

    if (state.dataHVACStandAloneERV->GetERVInputFlag) {
        GetStandAloneERV(state);
        state.dataHVACStandAloneERV->GetERVInputFlag = false;
    }

    for (int StandAloneERVIndex = 1; StandAloneERVIndex <= state.dataHVACStandAloneERV->NumStandAloneERVs; ++StandAloneERVIndex) {

        auto &StandAloneERV = state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVIndex);
        int SupplyFanInletNodeIndex = 0;
        int SupplyFanOutletNodeIndex = 0;
        int ExhaustFanInletNodeIndex = 0;
        int ExhaustFanOutletNodeIndex = 0;
        Real64 SupplyFanAirFlow;
        Real64 ExhaustFanAirFlow;

        // Get supply air fan inlet and outlet node index and air flow
        // ZoneHVAC:EnergyRecoveryVentilator only accepts Fan:SystemModel or Fan:OnOff
        SupplyFanInletNodeIndex = state.dataFans->fans(StandAloneERV.SupplyAirFanIndex)->inletNodeNum;
        SupplyFanOutletNodeIndex = state.dataFans->fans(StandAloneERV.SupplyAirFanIndex)->outletNodeNum;
        SupplyFanAirFlow = state.dataFans->fans(StandAloneERV.SupplyAirFanIndex)->maxAirFlowRate;

        // Get exhaust air fan inlet and outlet node index and air flow
        ExhaustFanInletNodeIndex = state.dataFans->fans(StandAloneERV.ExhaustAirFanIndex)->inletNodeNum;
        ExhaustFanOutletNodeIndex = state.dataFans->fans(StandAloneERV.ExhaustAirFanIndex)->outletNodeNum;
        ExhaustFanAirFlow = state.dataFans->fans(StandAloneERV.ExhaustAirFanIndex)->maxAirFlowRate;

        // If a standalone ERV's airflow is unbalanced it shouldn't be model along with an AFN
        if (std::abs(SupplyFanAirFlow - ExhaustFanAirFlow) >= 1E-20 ||
            std::abs(StandAloneERV.DesignSAFanVolFlowRate - StandAloneERV.DesignEAFanVolFlowRate) >= 1E-20) {
            break;
        }

        // Supply air fan nodes
        if (NodeNumber == SupplyFanInletNodeIndex || NodeNumber == SupplyFanOutletNodeIndex || NodeNumber == ExhaustFanInletNodeIndex ||
            NodeNumber == ExhaustFanOutletNodeIndex) {
            return true;
        }

        // Supply air inlet node
        if (NodeNumber == StandAloneERV.SupplyAirInletNode) {
            return true;
        }
    }

    return false;
}

int getEqIndex(EnergyPlusData &state, std::string_view CompName)
{
    if (state.dataHVACStandAloneERV->GetERVInputFlag) {
        GetStandAloneERV(state);
        state.dataHVACStandAloneERV->GetERVInputFlag = false;
    }

    for (int StandAloneERVNum = 1; StandAloneERVNum <= state.dataHVACStandAloneERV->NumStandAloneERVs; StandAloneERVNum++) {
        if (Util::SameString(CompName, state.dataHVACStandAloneERV->StandAloneERV(StandAloneERVNum).Name)) {
            return StandAloneERVNum;
        }
    }
    return 0;
}

} // namespace EnergyPlus::HVACStandAloneERV
