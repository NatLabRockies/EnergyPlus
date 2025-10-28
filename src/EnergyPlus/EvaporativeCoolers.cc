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
#include <cassert>
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/CoolingAirFlowSizing.hh>
#include <EnergyPlus/Autosizing/CoolingCapacitySizing.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataWater.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/EvaporativeCoolers.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/FaultsManager.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterManager.hh>

namespace EnergyPlus::EvaporativeCoolers {
// Module containing the EvaporativeCoolers simulation routines

// MODULE INFORMATION:
//       AUTHOR         Richard J. Liesen
//       DATE WRITTEN   Oct 2000
//       MODIFIED       BG July 2003 ResearchSpecial Indirect
//                      BG February 2007 outside air nodes
//                      BG March 2009 ResearchSpecial Direct
//       RE-ENGINEERED  na

// PURPOSE OF THIS MODULE:
// To encapsulate the data and algorithms required for
// Evaporative Coolers Components for use in mechanical air systems

// provide models for evaporative coolers as zone forced air units.

// METHODOLOGY EMPLOYED:
// various evaporative component models in this module
//   different models share common module level data structure.

constexpr std::array<std::string_view, static_cast<int>(EvapCoolerType::Num)> evapCoolerTypeNamesUC = {"EVAPORATIVECOOLER:DIRECT:CELDEKPAD",
                                                                                                       "EVAPORATIVECOOLER:INDIRECT:CELDEKPAD",
                                                                                                       "EVAPORATIVECOOLER:INDIRECT:WETCOIL",
                                                                                                       "EVAPORATIVECOOLER:INDIRECT:RESEARCHSPECIAL",
                                                                                                       "EVAPORATIVECOOLER:DIRECT:RESEARCHSPECIAL"};
constexpr std::array<std::string_view, static_cast<int>(EvapCoolerType::Num)> evapCoolerTypeNames = {"EvaporativeCooler:Direct:CelDekPad",
                                                                                                     "EvaporativeCooler:Indirect:CelDekPad",
                                                                                                     "EvaporativeCooler:Indirect:WetCoil",
                                                                                                     "EvaporativeCooler:Indirect:ResearchSpecial",
                                                                                                     "EvaporativeCooler:Direct:ResearchSpecial"};

void SimEvapCooler(EnergyPlusData &state, std::string_view CompName, int &CompIndex, Real64 const ZoneEvapCoolerPLR)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   October 2000

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine manages EvapCooler component simulation.
    // It is called from the SimAirLoopComponent
    // at the system time step.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int EvapCoolNum; // The EvapCooler that you are currently loading input into

    auto &EvapCond(state.dataEvapCoolers->EvapCond);

    // Obtains and Allocates EvapCooler related parameters from input file
    if (state.dataEvapCoolers->GetInputEvapComponentsFlag) { // First time subroutine has been entered
        GetEvapInput(state);
        state.dataEvapCoolers->GetInputEvapComponentsFlag = false;
    }

    // Find the correct EvapCoolNumber
    if (CompIndex == 0) {
        EvapCoolNum = Util::FindItemInList(CompName, EvapCond, &EvapConditions::Name);
        if (EvapCoolNum == 0) {
            ShowFatalError(state, format("SimEvapCooler: Unit not found={}", CompName));
        }
        CompIndex = EvapCoolNum;
    } else {
        EvapCoolNum = CompIndex;
        if (EvapCoolNum > state.dataEvapCoolers->NumEvapCool || EvapCoolNum < 1) {
            ShowFatalError(state,
                           format("SimEvapCooler:  Invalid CompIndex passed={}, Number of Units={}, Entered Unit name={}",
                                  EvapCoolNum,
                                  state.dataEvapCoolers->NumEvapCool,
                                  CompName));
        }
        if (state.dataEvapCoolers->CheckEquipName(EvapCoolNum)) {
            if (CompName != EvapCond(EvapCoolNum).Name) {
                ShowFatalError(state,
                               format("SimEvapCooler: Invalid CompIndex passed={}, Unit name={}, stored Unit Name for that index={}",
                                      EvapCoolNum,
                                      CompName,
                                      EvapCond(EvapCoolNum).Name));
            }
            state.dataEvapCoolers->CheckEquipName(EvapCoolNum) = false;
        }
    }

    // With the correct EvapCoolNum Initialize
    InitEvapCooler(state, EvapCoolNum); // Initialize all related parameters

    switch (EvapCond(EvapCoolNum).evapCoolerType) {
    case EvapCoolerType::DirectCELDEKPAD: {
        CalcDirectEvapCooler(state, EvapCoolNum, ZoneEvapCoolerPLR);
    } break;
    case EvapCoolerType::IndirectCELDEKPAD: {
        CalcDryIndirectEvapCooler(state, EvapCoolNum, ZoneEvapCoolerPLR);
    } break;
    case EvapCoolerType::IndirectWETCOIL: {
        CalcWetIndirectEvapCooler(state, EvapCoolNum, ZoneEvapCoolerPLR);
    } break;
    case EvapCoolerType::IndirectRDDSpecial: {
        CalcResearchSpecialPartLoad(state, EvapCoolNum);
        CalcIndirectResearchSpecialEvapCooler(state, EvapCoolNum, ZoneEvapCoolerPLR);
    } break;
    case EvapCoolerType::DirectResearchSpecial: {
        CalcResearchSpecialPartLoad(state, EvapCoolNum);
        CalcDirectResearchSpecialEvapCooler(state, EvapCoolNum, ZoneEvapCoolerPLR);
    } break;
    default:
        break;
    }
    // Update the current Evap Cooler to the outlet nodes
    UpdateEvapCooler(state, EvapCoolNum);

    // Report the current Evap Cooler
    ReportEvapCooler(state, EvapCoolNum);
}

void GetEvapInput(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   Oct 2000
    //       MODIFIED       BTG,  adding in EVAPCOOLER:INDIRECT:RDDSPECIAL

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is the main routine to call other input routines and Get routines

    // METHODOLOGY EMPLOYED:
    // Uses the status flags to trigger events.

    static constexpr std::string_view routineName = "GetEvapInput";

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int NumDirectEvapCool;                // The number of Direct CelDek EvapCooler in this simulation
    int NumDryInDirectEvapCool;           // The number of dry indirect evap coolers
    int NumWetInDirectEvapCool;           // The number of wet indirect evap coolers
    int NumRDDEvapCool;                   // the number of special research indirect evap coolers
    int NumDirectResearchSpecialEvapCool; // the number of special research direct evap coolers

    int NumAlphas;
    int NumNums;
    int IOStat;
    bool ErrorsFound(false);

    auto &EvapCond(state.dataEvapCoolers->EvapCond);
    auto &UniqueEvapCondNames(state.dataEvapCoolers->UniqueEvapCondNames);

    state.dataEvapCoolers->GetInputEvapComponentsFlag = false;
    // Start getting the input data
    NumDirectEvapCool = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "EvaporativeCooler:Direct:CelDekPad");
    NumDryInDirectEvapCool = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "EvaporativeCooler:Indirect:CelDekPad");
    NumWetInDirectEvapCool = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "EvaporativeCooler:Indirect:WetCoil");
    NumRDDEvapCool = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "EvaporativeCooler:Indirect:ResearchSpecial");
    NumDirectResearchSpecialEvapCool =
        state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "EvaporativeCooler:Direct:ResearchSpecial");

    // Sum up all of the Evap Cooler Types
    state.dataEvapCoolers->NumEvapCool =
        NumDirectEvapCool + NumDryInDirectEvapCool + NumWetInDirectEvapCool + NumRDDEvapCool + NumDirectResearchSpecialEvapCool;

    if (state.dataEvapCoolers->NumEvapCool > 0) {
        EvapCond.allocate(state.dataEvapCoolers->NumEvapCool);
        UniqueEvapCondNames.reserve(state.dataEvapCoolers->NumEvapCool);
    }
    state.dataEvapCoolers->CheckEquipName.dimension(state.dataEvapCoolers->NumEvapCool, true);
    auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;
    cCurrentModuleObject = "EvaporativeCooler:Direct:CelDekPad";

    for (int EvapCoolNum = 1; EvapCoolNum <= NumDirectEvapCool; ++EvapCoolNum) {
        auto &thisEvapCooler = EvapCond(EvapCoolNum);
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 EvapCoolNum,
                                                                 state.dataIPShortCut->cAlphaArgs,
                                                                 NumAlphas,
                                                                 state.dataIPShortCut->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 _,
                                                                 state.dataIPShortCut->lAlphaFieldBlanks,
                                                                 state.dataIPShortCut->cAlphaFieldNames,
                                                                 state.dataIPShortCut->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)};

        GlobalNames::VerifyUniqueInterObjectName(state,
                                                 UniqueEvapCondNames,
                                                 state.dataIPShortCut->cAlphaArgs(1),
                                                 cCurrentModuleObject,
                                                 state.dataIPShortCut->cAlphaFieldNames(1),
                                                 ErrorsFound);

        thisEvapCooler.Name = state.dataIPShortCut->cAlphaArgs(1);
        thisEvapCooler.evapCoolerType = EvapCoolerType::DirectCELDEKPAD;

        thisEvapCooler.Schedule = state.dataIPShortCut->cAlphaArgs(2);
        if (state.dataIPShortCut->lAlphaFieldBlanks(2)) {
            thisEvapCooler.availSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((thisEvapCooler.availSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(2), state.dataIPShortCut->cAlphaArgs(2));
            ErrorsFound = true;
        }

        thisEvapCooler.InletNode = GetOnlySingleNode(state,
                                                     state.dataIPShortCut->cAlphaArgs(3),
                                                     ErrorsFound,
                                                     DataLoopNode::ConnectionObjectType::EvaporativeCoolerDirectCelDekPad,
                                                     state.dataIPShortCut->cAlphaArgs(1),
                                                     DataLoopNode::NodeFluidType::Air,
                                                     DataLoopNode::ConnectionType::Inlet,
                                                     NodeInputManager::CompFluidStream::Primary,
                                                     DataLoopNode::ObjectIsNotParent);

        thisEvapCooler.OutletNode = GetOnlySingleNode(state,
                                                      state.dataIPShortCut->cAlphaArgs(4),
                                                      ErrorsFound,
                                                      DataLoopNode::ConnectionObjectType::EvaporativeCoolerDirectCelDekPad,
                                                      state.dataIPShortCut->cAlphaArgs(1),
                                                      DataLoopNode::NodeFluidType::Air,
                                                      DataLoopNode::ConnectionType::Outlet,
                                                      NodeInputManager::CompFluidStream::Primary,
                                                      DataLoopNode::ObjectIsNotParent);

        BranchNodeConnections::TestCompSet(state,
                                           cCurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           state.dataIPShortCut->cAlphaArgs(3),
                                           state.dataIPShortCut->cAlphaArgs(4),
                                           "Evap Air Nodes");

        thisEvapCooler.EvapControlType = state.dataIPShortCut->cAlphaArgs(5);

        // input the numerical data
        thisEvapCooler.PadArea = state.dataIPShortCut->rNumericArgs(1);
        thisEvapCooler.PadDepth = state.dataIPShortCut->rNumericArgs(2);
        thisEvapCooler.RecircPumpPower = state.dataIPShortCut->rNumericArgs(3);

        SetupOutputVariable(state,
                            "Evaporative Cooler Wet Bulb Effectiveness",
                            Constant::Units::None,
                            thisEvapCooler.SatEff,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisEvapCooler.Name);

        // A6 ; \Field Name of Water Supply Storage Tank
        thisEvapCooler.EvapWaterSupplyName = state.dataIPShortCut->cAlphaArgs(6);
        if (state.dataIPShortCut->lAlphaFieldBlanks(6)) {
            thisEvapCooler.EvapWaterSupplyMode = WaterSupply::FromMains;
        } else {
            thisEvapCooler.EvapWaterSupplyMode = WaterSupply::FromTank;
            WaterManager::SetupTankDemandComponent(state,
                                                   thisEvapCooler.Name,
                                                   cCurrentModuleObject,
                                                   thisEvapCooler.EvapWaterSupplyName,
                                                   ErrorsFound,
                                                   thisEvapCooler.EvapWaterSupTankID,
                                                   thisEvapCooler.EvapWaterTankDemandARRID);
        }

    } // end Number of EvapCooler Loop

    //**************************************************************
    // This is the start of the Dry Indirect Evap Cooler Loop
    cCurrentModuleObject = "EvaporativeCooler:Indirect:CelDekPad";

    for (int IndEvapCoolNum = 1; IndEvapCoolNum <= NumDryInDirectEvapCool; ++IndEvapCoolNum) {
        int EvapCoolNum = NumDirectEvapCool + IndEvapCoolNum;
        auto &thisEvapCooler = EvapCond(EvapCoolNum);
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 IndEvapCoolNum,
                                                                 state.dataIPShortCut->cAlphaArgs,
                                                                 NumAlphas,
                                                                 state.dataIPShortCut->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 _,
                                                                 state.dataIPShortCut->lAlphaFieldBlanks,
                                                                 state.dataIPShortCut->cAlphaFieldNames,
                                                                 state.dataIPShortCut->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)};

        GlobalNames::VerifyUniqueInterObjectName(state,
                                                 UniqueEvapCondNames,
                                                 state.dataIPShortCut->cAlphaArgs(1),
                                                 cCurrentModuleObject,
                                                 state.dataIPShortCut->cAlphaFieldNames(1),
                                                 ErrorsFound);
        thisEvapCooler.Name = state.dataIPShortCut->cAlphaArgs(1);
        thisEvapCooler.evapCoolerType = EvapCoolerType::IndirectCELDEKPAD; //'EvaporativeCooler:Indirect:CelDekPad'

        thisEvapCooler.Schedule = state.dataIPShortCut->cAlphaArgs(2);
        if (state.dataIPShortCut->lAlphaFieldBlanks(2)) {
            thisEvapCooler.availSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((thisEvapCooler.availSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(2), state.dataIPShortCut->cAlphaArgs(2));
            ErrorsFound = true;
        }

        thisEvapCooler.InletNode = GetOnlySingleNode(state,
                                                     state.dataIPShortCut->cAlphaArgs(3),
                                                     ErrorsFound,
                                                     DataLoopNode::ConnectionObjectType::EvaporativeCoolerIndirectCelDekPad,
                                                     state.dataIPShortCut->cAlphaArgs(1),
                                                     DataLoopNode::NodeFluidType::Air,
                                                     DataLoopNode::ConnectionType::Inlet,
                                                     NodeInputManager::CompFluidStream::Primary,
                                                     DataLoopNode::ObjectIsNotParent);

        thisEvapCooler.OutletNode = GetOnlySingleNode(state,
                                                      state.dataIPShortCut->cAlphaArgs(4),
                                                      ErrorsFound,
                                                      DataLoopNode::ConnectionObjectType::EvaporativeCoolerIndirectCelDekPad,
                                                      state.dataIPShortCut->cAlphaArgs(1),
                                                      DataLoopNode::NodeFluidType::Air,
                                                      DataLoopNode::ConnectionType::Outlet,
                                                      NodeInputManager::CompFluidStream::Primary,
                                                      DataLoopNode::ObjectIsNotParent);

        BranchNodeConnections::TestCompSet(state,
                                           cCurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           state.dataIPShortCut->cAlphaArgs(3),
                                           state.dataIPShortCut->cAlphaArgs(4),
                                           "Evap Air Nodes");

        thisEvapCooler.EvapControlType = state.dataIPShortCut->cAlphaArgs(5);

        // input the numerical data
        thisEvapCooler.IndirectPadArea = state.dataIPShortCut->rNumericArgs(1);
        thisEvapCooler.IndirectPadDepth = state.dataIPShortCut->rNumericArgs(2);
        thisEvapCooler.IndirectRecircPumpPower = state.dataIPShortCut->rNumericArgs(3);
        thisEvapCooler.IndirectVolFlowRate = state.dataIPShortCut->rNumericArgs(4);
        thisEvapCooler.IndirectFanEff = state.dataIPShortCut->rNumericArgs(5);
        thisEvapCooler.IndirectFanDeltaPress = state.dataIPShortCut->rNumericArgs(6);
        thisEvapCooler.IndirectHXEffectiveness = state.dataIPShortCut->rNumericArgs(7);

        SetupOutputVariable(state,
                            "Evaporative Cooler Wetbulb Effectiveness",
                            Constant::Units::None,
                            thisEvapCooler.SatEff,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisEvapCooler.Name);
        SetupOutputVariable(state,
                            "Evaporative Cooler Total Stage Effectiveness",
                            Constant::Units::None,
                            thisEvapCooler.StageEff,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisEvapCooler.Name);

        // A6 ; \Field Name of Water Supply Storage Tank
        thisEvapCooler.EvapWaterSupplyName = state.dataIPShortCut->cAlphaArgs(6);
        if (state.dataIPShortCut->lAlphaFieldBlanks(6)) {
            thisEvapCooler.EvapWaterSupplyMode = WaterSupply::FromMains;
        } else {
            thisEvapCooler.EvapWaterSupplyMode = WaterSupply::FromTank;
            WaterManager::SetupTankDemandComponent(state,
                                                   thisEvapCooler.Name,
                                                   cCurrentModuleObject,
                                                   thisEvapCooler.EvapWaterSupplyName,
                                                   ErrorsFound,
                                                   thisEvapCooler.EvapWaterSupTankID,
                                                   thisEvapCooler.EvapWaterTankDemandARRID);
        }

        // A7 ; \field Secondary Outside Air Inlet node.
        if (state.dataIPShortCut->lAlphaFieldBlanks(7)) {
            thisEvapCooler.SecondaryInletNode = 0;
        } else {
            thisEvapCooler.SecondaryInletNode = GetOnlySingleNode(state,
                                                                  state.dataIPShortCut->cAlphaArgs(7),
                                                                  ErrorsFound,
                                                                  DataLoopNode::ConnectionObjectType::EvaporativeCoolerIndirectCelDekPad,
                                                                  state.dataIPShortCut->cAlphaArgs(1),
                                                                  DataLoopNode::NodeFluidType::Air,
                                                                  DataLoopNode::ConnectionType::OutsideAirReference,
                                                                  NodeInputManager::CompFluidStream::Primary,
                                                                  DataLoopNode::ObjectIsNotParent);
            if (!OutAirNodeManager::CheckOutAirNodeNumber(state, thisEvapCooler.SecondaryInletNode)) {
                ShowSevereError(state, format("Invalid {}={}", state.dataIPShortCut->cAlphaFieldNames(7), state.dataIPShortCut->cAlphaArgs(7)));
                ShowContinueError(state, format("Entered in {}={}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                // TODO rename point
                ShowContinueError(state, "Node does not appear in an OutdoorAir:NodeList or as an OutdoorAir:Node.");
                ErrorsFound = true;
            }
        }

    } // end Number of Dry Indirect EvapCooler Loop

    //**************************************************************
    // This is the start of the WetIndirect Evap Cooler Loop
    cCurrentModuleObject = "EvaporativeCooler:Indirect:WetCoil";
    for (int IndEvapCoolNum = 1; IndEvapCoolNum <= NumWetInDirectEvapCool; ++IndEvapCoolNum) {
        int EvapCoolNum = NumDirectEvapCool + NumDryInDirectEvapCool + IndEvapCoolNum;
        auto &thisEvapCooler = EvapCond(EvapCoolNum);
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 IndEvapCoolNum,
                                                                 state.dataIPShortCut->cAlphaArgs,
                                                                 NumAlphas,
                                                                 state.dataIPShortCut->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 _,
                                                                 state.dataIPShortCut->lAlphaFieldBlanks,
                                                                 state.dataIPShortCut->cAlphaFieldNames,
                                                                 state.dataIPShortCut->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)};

        GlobalNames::VerifyUniqueInterObjectName(state,
                                                 UniqueEvapCondNames,
                                                 state.dataIPShortCut->cAlphaArgs(1),
                                                 cCurrentModuleObject,
                                                 state.dataIPShortCut->cAlphaFieldNames(1),
                                                 ErrorsFound);
        thisEvapCooler.Name = state.dataIPShortCut->cAlphaArgs(1);
        thisEvapCooler.evapCoolerType = EvapCoolerType::IndirectWETCOIL; //'EvaporativeCooler:Indirect:WetCoil'

        thisEvapCooler.Schedule = state.dataIPShortCut->cAlphaArgs(2);
        if (state.dataIPShortCut->lAlphaFieldBlanks(2)) {
            thisEvapCooler.availSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((thisEvapCooler.availSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(2), state.dataIPShortCut->cAlphaArgs(2));
            ErrorsFound = true;
        }

        thisEvapCooler.InletNode = GetOnlySingleNode(state,
                                                     state.dataIPShortCut->cAlphaArgs(3),
                                                     ErrorsFound,
                                                     DataLoopNode::ConnectionObjectType::EvaporativeCoolerIndirectWetCoil,
                                                     state.dataIPShortCut->cAlphaArgs(1),
                                                     DataLoopNode::NodeFluidType::Air,
                                                     DataLoopNode::ConnectionType::Inlet,
                                                     NodeInputManager::CompFluidStream::Primary,
                                                     DataLoopNode::ObjectIsNotParent);

        thisEvapCooler.OutletNode = GetOnlySingleNode(state,
                                                      state.dataIPShortCut->cAlphaArgs(4),
                                                      ErrorsFound,
                                                      DataLoopNode::ConnectionObjectType::EvaporativeCoolerIndirectWetCoil,
                                                      state.dataIPShortCut->cAlphaArgs(1),
                                                      DataLoopNode::NodeFluidType::Air,
                                                      DataLoopNode::ConnectionType::Outlet,
                                                      NodeInputManager::CompFluidStream::Primary,
                                                      DataLoopNode::ObjectIsNotParent);

        BranchNodeConnections::TestCompSet(state,
                                           cCurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           state.dataIPShortCut->cAlphaArgs(3),
                                           state.dataIPShortCut->cAlphaArgs(4),
                                           "Evap Air Nodes");

        thisEvapCooler.EvapControlType = state.dataIPShortCut->cAlphaArgs(5);

        // input the numerical data
        thisEvapCooler.WetCoilMaxEfficiency = state.dataIPShortCut->rNumericArgs(1);
        thisEvapCooler.WetCoilFlowRatio = state.dataIPShortCut->rNumericArgs(2);
        thisEvapCooler.IndirectRecircPumpPower = state.dataIPShortCut->rNumericArgs(3);
        thisEvapCooler.IndirectVolFlowRate = state.dataIPShortCut->rNumericArgs(4);
        thisEvapCooler.IndirectFanEff = state.dataIPShortCut->rNumericArgs(5);
        thisEvapCooler.IndirectFanDeltaPress = state.dataIPShortCut->rNumericArgs(6);

        SetupOutputVariable(state,
                            "Evaporative Cooler Total Stage Effectiveness",
                            Constant::Units::None,
                            thisEvapCooler.StageEff,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisEvapCooler.Name);

        //  A6 ; \Field Name of Water Supply Storage Tank
        thisEvapCooler.EvapWaterSupplyName = state.dataIPShortCut->cAlphaArgs(6);
        if (state.dataIPShortCut->lAlphaFieldBlanks(6)) {
            thisEvapCooler.EvapWaterSupplyMode = WaterSupply::FromMains;
        } else {
            thisEvapCooler.EvapWaterSupplyMode = WaterSupply::FromTank;
            WaterManager::SetupTankDemandComponent(state,
                                                   thisEvapCooler.Name,
                                                   cCurrentModuleObject,
                                                   thisEvapCooler.EvapWaterSupplyName,
                                                   ErrorsFound,
                                                   thisEvapCooler.EvapWaterSupTankID,
                                                   thisEvapCooler.EvapWaterTankDemandARRID);
        }

        // A7 ; \field Secondary Outside Air Inlet node.
        if (state.dataIPShortCut->lAlphaFieldBlanks(7)) {
            thisEvapCooler.SecondaryInletNode = 0;
        } else {
            thisEvapCooler.SecondaryInletNode = GetOnlySingleNode(state,
                                                                  state.dataIPShortCut->cAlphaArgs(7),
                                                                  ErrorsFound,
                                                                  DataLoopNode::ConnectionObjectType::EvaporativeCoolerIndirectWetCoil,
                                                                  state.dataIPShortCut->cAlphaArgs(1),
                                                                  DataLoopNode::NodeFluidType::Air,
                                                                  DataLoopNode::ConnectionType::OutsideAirReference,
                                                                  NodeInputManager::CompFluidStream::Primary,
                                                                  DataLoopNode::ObjectIsNotParent);
            if (!OutAirNodeManager::CheckOutAirNodeNumber(state, thisEvapCooler.SecondaryInletNode)) {
                ShowSevereError(state, format("Invalid {}={}", state.dataIPShortCut->cAlphaFieldNames(7), state.dataIPShortCut->cAlphaArgs(7)));
                ShowContinueError(state, format("Entered in {}={}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                // TODO rename point
                ShowContinueError(state, "Node does not appear in an OutdoorAir:NodeList or as an OutdoorAir:Node.");
                ErrorsFound = true;
            }
        }

    } // end Number of Wet Coil Indirect EvapCooler Loop
    //**************************************************************
    // This is the start of the Indirect Research Special Evap Cooler
    cCurrentModuleObject = "EvaporativeCooler:Indirect:ResearchSpecial";
    for (int IndEvapCoolNum = 1; IndEvapCoolNum <= NumRDDEvapCool; ++IndEvapCoolNum) {
        int EvapCoolNum = NumDirectEvapCool + NumDryInDirectEvapCool + NumWetInDirectEvapCool + IndEvapCoolNum;
        auto &thisEvapCooler = EvapCond(EvapCoolNum);
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 IndEvapCoolNum,
                                                                 state.dataIPShortCut->cAlphaArgs,
                                                                 NumAlphas,
                                                                 state.dataIPShortCut->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 state.dataIPShortCut->lNumericFieldBlanks,
                                                                 state.dataIPShortCut->lAlphaFieldBlanks,
                                                                 state.dataIPShortCut->cAlphaFieldNames,
                                                                 state.dataIPShortCut->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)};
        GlobalNames::VerifyUniqueInterObjectName(state,
                                                 UniqueEvapCondNames,
                                                 state.dataIPShortCut->cAlphaArgs(1),
                                                 cCurrentModuleObject,
                                                 state.dataIPShortCut->cAlphaFieldNames(1),
                                                 ErrorsFound);
        thisEvapCooler.Name = state.dataIPShortCut->cAlphaArgs(1);
        thisEvapCooler.evapCoolerType = EvapCoolerType::IndirectRDDSpecial; //'EvaporativeCooler:Indirect:ResearchSpecial'

        thisEvapCooler.Schedule = state.dataIPShortCut->cAlphaArgs(2);
        if (state.dataIPShortCut->lAlphaFieldBlanks(2)) {
            thisEvapCooler.availSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((thisEvapCooler.availSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(2), state.dataIPShortCut->cAlphaArgs(2));
            ErrorsFound = true;
        }

        thisEvapCooler.InletNode = GetOnlySingleNode(state,
                                                     state.dataIPShortCut->cAlphaArgs(7),
                                                     ErrorsFound,
                                                     DataLoopNode::ConnectionObjectType::EvaporativeCoolerIndirectResearchSpecial,
                                                     state.dataIPShortCut->cAlphaArgs(1),
                                                     DataLoopNode::NodeFluidType::Air,
                                                     DataLoopNode::ConnectionType::Inlet,
                                                     NodeInputManager::CompFluidStream::Primary,
                                                     DataLoopNode::ObjectIsNotParent);

        thisEvapCooler.OutletNode = GetOnlySingleNode(state,
                                                      state.dataIPShortCut->cAlphaArgs(8),
                                                      ErrorsFound,
                                                      DataLoopNode::ConnectionObjectType::EvaporativeCoolerIndirectResearchSpecial,
                                                      state.dataIPShortCut->cAlphaArgs(1),
                                                      DataLoopNode::NodeFluidType::Air,
                                                      DataLoopNode::ConnectionType::Outlet,
                                                      NodeInputManager::CompFluidStream::Primary,
                                                      DataLoopNode::ObjectIsNotParent);

        BranchNodeConnections::TestCompSet(state,
                                           cCurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           state.dataIPShortCut->cAlphaArgs(7),
                                           state.dataIPShortCut->cAlphaArgs(8),
                                           "Evap Air Nodes");

        if (state.dataIPShortCut->lAlphaFieldBlanks(9)) {
            thisEvapCooler.SecondaryInletNode = 0;
        } else {
            thisEvapCooler.SecondaryInletNode = GetOnlySingleNode(state,
                                                                  state.dataIPShortCut->cAlphaArgs(9),
                                                                  ErrorsFound,
                                                                  DataLoopNode::ConnectionObjectType::EvaporativeCoolerIndirectResearchSpecial,
                                                                  state.dataIPShortCut->cAlphaArgs(1),
                                                                  DataLoopNode::NodeFluidType::Air,
                                                                  DataLoopNode::ConnectionType::Inlet,
                                                                  NodeInputManager::CompFluidStream::Secondary,
                                                                  DataLoopNode::ObjectIsNotParent);
        }

        if (state.dataIPShortCut->lAlphaFieldBlanks(10)) {
            thisEvapCooler.SecondaryOutletNode = 0;
        } else {
            thisEvapCooler.SecondaryOutletNode = GetOnlySingleNode(state,
                                                                   state.dataIPShortCut->cAlphaArgs(10),
                                                                   ErrorsFound,
                                                                   DataLoopNode::ConnectionObjectType::EvaporativeCoolerIndirectResearchSpecial,
                                                                   state.dataIPShortCut->cAlphaArgs(1),
                                                                   DataLoopNode::NodeFluidType::Air,
                                                                   DataLoopNode::ConnectionType::Outlet,
                                                                   NodeInputManager::CompFluidStream::Secondary,
                                                                   DataLoopNode::ObjectIsNotParent);
        }

        thisEvapCooler.EvapControlNodeNum = GetOnlySingleNode(state,
                                                              state.dataIPShortCut->cAlphaArgs(11),
                                                              ErrorsFound,
                                                              DataLoopNode::ConnectionObjectType::EvaporativeCoolerIndirectResearchSpecial,
                                                              state.dataIPShortCut->cAlphaArgs(1),
                                                              DataLoopNode::NodeFluidType::Air,
                                                              DataLoopNode::ConnectionType::Sensor,
                                                              NodeInputManager::CompFluidStream::Primary,
                                                              DataLoopNode::ObjectIsNotParent);

        thisEvapCooler.TertiaryInletNode = GetOnlySingleNode(state,
                                                             state.dataIPShortCut->cAlphaArgs(12),
                                                             ErrorsFound,
                                                             DataLoopNode::ConnectionObjectType::EvaporativeCoolerIndirectResearchSpecial,
                                                             state.dataIPShortCut->cAlphaArgs(1),
                                                             DataLoopNode::NodeFluidType::Air,
                                                             DataLoopNode::ConnectionType::Inlet,
                                                             NodeInputManager::CompFluidStream::Tertiary,
                                                             DataLoopNode::ObjectIsNotParent);

        thisEvapCooler.EvapWaterSupplyName = state.dataIPShortCut->cAlphaArgs(13);
        if (state.dataIPShortCut->lAlphaFieldBlanks(13)) {
            thisEvapCooler.EvapWaterSupplyMode = WaterSupply::FromMains;
        } else {
            thisEvapCooler.EvapWaterSupplyMode = WaterSupply::FromTank;
            WaterManager::SetupTankDemandComponent(state,
                                                   thisEvapCooler.Name,
                                                   cCurrentModuleObject,
                                                   thisEvapCooler.EvapWaterSupplyName,
                                                   ErrorsFound,
                                                   thisEvapCooler.EvapWaterSupTankID,
                                                   thisEvapCooler.EvapWaterTankDemandARRID);
        }

        // input the numerical data
        thisEvapCooler.WetCoilMaxEfficiency = state.dataIPShortCut->rNumericArgs(1);
        if (state.dataIPShortCut->lNumericFieldBlanks(2)) {
            thisEvapCooler.DryCoilMaxEfficiency = 0.0;
        } else {
            thisEvapCooler.DryCoilMaxEfficiency = state.dataIPShortCut->rNumericArgs(2);
        }
        thisEvapCooler.IndirectRecircPumpPower = state.dataIPShortCut->rNumericArgs(3);
        thisEvapCooler.RecircPumpSizingFactor = state.dataIPShortCut->rNumericArgs(4);
        thisEvapCooler.IndirectVolFlowRate = state.dataIPShortCut->rNumericArgs(5);
        thisEvapCooler.IndirectVolFlowScalingFactor = state.dataIPShortCut->rNumericArgs(6);
        thisEvapCooler.IndirectFanPower = state.dataIPShortCut->rNumericArgs(7);
        thisEvapCooler.FanSizingSpecificPower = state.dataIPShortCut->rNumericArgs(8);
        thisEvapCooler.DesVolFlowRate = state.dataIPShortCut->rNumericArgs(9);
        thisEvapCooler.DPBoundFactor = state.dataIPShortCut->rNumericArgs(10);
        if (state.dataIPShortCut->lNumericFieldBlanks(11)) {
            thisEvapCooler.DriftFraction = 0.0;
        } else {
            thisEvapCooler.DriftFraction = state.dataIPShortCut->rNumericArgs(11);
        }
        if (state.dataIPShortCut->lNumericFieldBlanks(12)) {
            thisEvapCooler.BlowDownRatio = 0.0;
        } else {
            thisEvapCooler.BlowDownRatio = state.dataIPShortCut->rNumericArgs(12);
        }
        if (state.dataIPShortCut->lNumericFieldBlanks(2) || state.dataIPShortCut->lNumericFieldBlanks(13) ||
            state.dataIPShortCut->lNumericFieldBlanks(14) || state.dataIPShortCut->lNumericFieldBlanks(15)) {
            thisEvapCooler.EvapCoolerOperationControlFlag = false;
        } else {
            if (!state.dataIPShortCut->lNumericFieldBlanks(2) && !state.dataIPShortCut->lNumericFieldBlanks(13) &&
                !state.dataIPShortCut->lNumericFieldBlanks(14) && !state.dataIPShortCut->lNumericFieldBlanks(15)) {
                thisEvapCooler.EvapCoolerOperationControlFlag = true;
                thisEvapCooler.MinOATDBEvapCooler = state.dataIPShortCut->rNumericArgs(13);
                thisEvapCooler.MaxOATWBEvapCooler = state.dataIPShortCut->rNumericArgs(14);
                thisEvapCooler.MaxOATDBEvapCooler = state.dataIPShortCut->rNumericArgs(15);
            } else {
                thisEvapCooler.EvapCoolerOperationControlFlag = false;
            }
        }
        thisEvapCooler.WetbulbEffecCurve = Curve::GetCurve(state, state.dataIPShortCut->cAlphaArgs(3));
        thisEvapCooler.DrybulbEffecCurve = Curve::GetCurve(state, state.dataIPShortCut->cAlphaArgs(4));
        thisEvapCooler.PumpPowerModifierCurve = Curve::GetCurve(state, state.dataIPShortCut->cAlphaArgs(5));
        thisEvapCooler.FanPowerModifierCurve = Curve::GetCurve(state, state.dataIPShortCut->cAlphaArgs(6));

        SetupOutputVariable(state,
                            "Evaporative Cooler Total Stage Effectiveness",
                            Constant::Units::None,
                            thisEvapCooler.StageEff,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisEvapCooler.Name);
        SetupOutputVariable(state,
                            "Evaporative Cooler Part Load Ratio",
                            Constant::Units::None,
                            thisEvapCooler.PartLoadFract,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisEvapCooler.Name);

        SetupOutputVariable(state,
                            "Evaporative Cooler Dewpoint Bound Status",
                            Constant::Units::None,
                            thisEvapCooler.DewPointBoundFlag,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisEvapCooler.Name);
        SetupOutputVariable(state,
                            "Evaporative Cooler Operating Mode Status",
                            Constant::Units::None,
                            thisEvapCooler.IECOperatingStatus,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisEvapCooler.Name);

    } // end of Indirect Research Special cooler input loop

    cCurrentModuleObject = "EvaporativeCooler:Direct:ResearchSpecial";
    for (int DirectEvapCoolNum = 1; DirectEvapCoolNum <= NumDirectResearchSpecialEvapCool; ++DirectEvapCoolNum) {
        int EvapCoolNum = NumDirectEvapCool + NumDryInDirectEvapCool + NumWetInDirectEvapCool + NumRDDEvapCool + DirectEvapCoolNum;
        auto &thisEvapCooler = EvapCond(EvapCoolNum);
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 DirectEvapCoolNum,
                                                                 state.dataIPShortCut->cAlphaArgs,
                                                                 NumAlphas,
                                                                 state.dataIPShortCut->rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 state.dataIPShortCut->lNumericFieldBlanks,
                                                                 state.dataIPShortCut->lAlphaFieldBlanks,
                                                                 state.dataIPShortCut->cAlphaFieldNames,
                                                                 state.dataIPShortCut->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)};
        GlobalNames::VerifyUniqueInterObjectName(state,
                                                 UniqueEvapCondNames,
                                                 state.dataIPShortCut->cAlphaArgs(1),
                                                 cCurrentModuleObject,
                                                 state.dataIPShortCut->cAlphaFieldNames(1),
                                                 ErrorsFound);
        thisEvapCooler.Name = state.dataIPShortCut->cAlphaArgs(1);
        thisEvapCooler.evapCoolerType = EvapCoolerType::DirectResearchSpecial;

        thisEvapCooler.Schedule = state.dataIPShortCut->cAlphaArgs(2);
        if (state.dataIPShortCut->lAlphaFieldBlanks(2)) {
            thisEvapCooler.availSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((thisEvapCooler.availSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(2), state.dataIPShortCut->cAlphaArgs(2));
            ErrorsFound = true;
        }

        thisEvapCooler.InletNode = GetOnlySingleNode(state,
                                                     state.dataIPShortCut->cAlphaArgs(5),
                                                     ErrorsFound,
                                                     DataLoopNode::ConnectionObjectType::EvaporativeCoolerDirectResearchSpecial,
                                                     state.dataIPShortCut->cAlphaArgs(1),
                                                     DataLoopNode::NodeFluidType::Air,
                                                     DataLoopNode::ConnectionType::Inlet,
                                                     NodeInputManager::CompFluidStream::Primary,
                                                     DataLoopNode::ObjectIsNotParent);

        thisEvapCooler.OutletNode = GetOnlySingleNode(state,
                                                      state.dataIPShortCut->cAlphaArgs(6),
                                                      ErrorsFound,
                                                      DataLoopNode::ConnectionObjectType::EvaporativeCoolerDirectResearchSpecial,
                                                      state.dataIPShortCut->cAlphaArgs(1),
                                                      DataLoopNode::NodeFluidType::Air,
                                                      DataLoopNode::ConnectionType::Outlet,
                                                      NodeInputManager::CompFluidStream::Primary,
                                                      DataLoopNode::ObjectIsNotParent);

        BranchNodeConnections::TestCompSet(state,
                                           cCurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           state.dataIPShortCut->cAlphaArgs(5),
                                           state.dataIPShortCut->cAlphaArgs(6),
                                           "Evap Air Nodes");

        thisEvapCooler.EvapControlNodeNum = GetOnlySingleNode(state,
                                                              state.dataIPShortCut->cAlphaArgs(7),
                                                              ErrorsFound,
                                                              DataLoopNode::ConnectionObjectType::EvaporativeCoolerDirectResearchSpecial,
                                                              state.dataIPShortCut->cAlphaArgs(1),
                                                              DataLoopNode::NodeFluidType::Air,
                                                              DataLoopNode::ConnectionType::Sensor,
                                                              NodeInputManager::CompFluidStream::Primary,
                                                              DataLoopNode::ObjectIsNotParent);

        thisEvapCooler.EvapWaterSupplyName = state.dataIPShortCut->cAlphaArgs(8);

        if (state.dataIPShortCut->lAlphaFieldBlanks(8)) {
            thisEvapCooler.EvapWaterSupplyMode = WaterSupply::FromMains;
        } else {
            thisEvapCooler.EvapWaterSupplyMode = WaterSupply::FromTank;
            WaterManager::SetupTankDemandComponent(state,
                                                   thisEvapCooler.Name,
                                                   cCurrentModuleObject,
                                                   thisEvapCooler.EvapWaterSupplyName,
                                                   ErrorsFound,
                                                   thisEvapCooler.EvapWaterSupTankID,
                                                   thisEvapCooler.EvapWaterTankDemandARRID);
        }
        thisEvapCooler.DirectEffectiveness = state.dataIPShortCut->rNumericArgs(1);

        thisEvapCooler.DesVolFlowRate = state.dataIPShortCut->rNumericArgs(2);
        thisEvapCooler.RecircPumpPower = state.dataIPShortCut->rNumericArgs(3);
        thisEvapCooler.RecircPumpSizingFactor = state.dataIPShortCut->rNumericArgs(4);
        if (state.dataIPShortCut->lNumericFieldBlanks(5)) {
            thisEvapCooler.DriftFraction = 0.0;
        } else {
            thisEvapCooler.DriftFraction = state.dataIPShortCut->rNumericArgs(5);
        }
        if (state.dataIPShortCut->lNumericFieldBlanks(6)) {
            thisEvapCooler.BlowDownRatio = 0.0;
        } else {
            thisEvapCooler.BlowDownRatio = state.dataIPShortCut->rNumericArgs(6);
        }
        if (state.dataIPShortCut->lNumericFieldBlanks(7) || state.dataIPShortCut->lNumericFieldBlanks(8) ||
            state.dataIPShortCut->lNumericFieldBlanks(9)) {
            thisEvapCooler.EvapCoolerOperationControlFlag = false;
        } else {
            if (!state.dataIPShortCut->lNumericFieldBlanks(7) && !state.dataIPShortCut->lNumericFieldBlanks(8) &&
                !state.dataIPShortCut->lNumericFieldBlanks(9)) {
                thisEvapCooler.EvapCoolerOperationControlFlag = true;
                thisEvapCooler.MinOATDBEvapCooler = state.dataIPShortCut->rNumericArgs(7);
                thisEvapCooler.MaxOATWBEvapCooler = state.dataIPShortCut->rNumericArgs(8);
                thisEvapCooler.MaxOATDBEvapCooler = state.dataIPShortCut->rNumericArgs(9);
            } else {
                thisEvapCooler.EvapCoolerOperationControlFlag = false;
            }
        }
        thisEvapCooler.WetbulbEffecCurve = Curve::GetCurve(state, state.dataIPShortCut->cAlphaArgs(3));
        thisEvapCooler.PumpPowerModifierCurve = Curve::GetCurve(state, state.dataIPShortCut->cAlphaArgs(4));

        SetupOutputVariable(state,
                            "Evaporative Cooler Stage Effectiveness",
                            Constant::Units::None,
                            thisEvapCooler.StageEff,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisEvapCooler.Name);
    }

    if (ErrorsFound) {
        ShowFatalError(state, "Errors found in processing input for evaporative coolers");
    }

    for (int EvapCoolNum = 1; EvapCoolNum <= state.dataEvapCoolers->NumEvapCool; ++EvapCoolNum) {
        auto &thisEvapCooler = EvapCond(EvapCoolNum);
        // Setup Report variables for the Evap Coolers
        SetupOutputVariable(state,
                            "Evaporative Cooler Electricity Energy",
                            Constant::Units::J,
                            thisEvapCooler.EvapCoolerEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisEvapCooler.Name,
                            Constant::eResource::Electricity,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::Cooling);
        SetupOutputVariable(state,
                            "Evaporative Cooler Electricity Rate",
                            Constant::Units::W,
                            thisEvapCooler.EvapCoolerPower,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisEvapCooler.Name);
        // this next report variable is setup differently depending on how the water should be metered here.
        if (thisEvapCooler.EvapWaterSupplyMode == WaterSupply::FromMains) {
            SetupOutputVariable(state,
                                "Evaporative Cooler Water Volume",
                                Constant::Units::m3,
                                thisEvapCooler.EvapWaterConsump,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisEvapCooler.Name,
                                Constant::eResource::Water,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Cooling);
            SetupOutputVariable(state,
                                "Evaporative Cooler Mains Water Volume",
                                Constant::Units::m3,
                                thisEvapCooler.EvapWaterConsump,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisEvapCooler.Name,
                                Constant::eResource::MainsWater,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Cooling);

        } else if (thisEvapCooler.EvapWaterSupplyMode == WaterSupply::FromTank) {
            SetupOutputVariable(state,
                                "Evaporative Cooler Storage Tank Water Volume",
                                Constant::Units::m3,
                                thisEvapCooler.EvapWaterConsump,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisEvapCooler.Name,
                                Constant::eResource::Water,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Cooling);
            SetupOutputVariable(state,
                                "Evaporative Cooler Starved Water Volume",
                                Constant::Units::m3,
                                thisEvapCooler.EvapWaterStarvMakup,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisEvapCooler.Name,
                                Constant::eResource::Water,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Cooling);
            SetupOutputVariable(state,
                                "Evaporative Cooler Starved Mains Water Volume",
                                Constant::Units::m3,
                                thisEvapCooler.EvapWaterStarvMakup,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisEvapCooler.Name,
                                Constant::eResource::MainsWater,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Cooling);
        }
    }
}

void InitEvapCooler(EnergyPlusData &state, int const EvapCoolNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   October 2000
    //       MODIFIED       B. Griffith, May 2009, added EMS setpoint check

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is for  initializations of the EvapCooler Components.

    // METHODOLOGY EMPLOYED:
    // Uses the status flags to trigger events.

    // Using/Aliasing
    auto &evapCond = state.dataEvapCoolers->EvapCond(EvapCoolNum);

    // Check that setpoint is active
    if (!state.dataGlobal->SysSizingCalc && state.dataEvapCoolers->MySetPointCheckFlag && state.dataHVACGlobal->DoSetPointTest) {
        for (int EvapUnitNum = 1; EvapUnitNum <= state.dataEvapCoolers->NumEvapCool; ++EvapUnitNum) {

            // only check evap coolers that are supposed to have a control node
            if ((evapCond.evapCoolerType != EvapCoolerType::IndirectRDDSpecial) &&
                (evapCond.evapCoolerType != EvapCoolerType::DirectResearchSpecial)) {
                continue;
            }

            int ControlNode = state.dataEvapCoolers->EvapCond(EvapUnitNum).EvapControlNodeNum;
            if (ControlNode > 0) {
                if (state.dataLoopNodes->Node(ControlNode).TempSetPoint == DataLoopNode::SensedNodeFlagValue) {
                    if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
                        ShowSevereError(state, format("Missing temperature setpoint for Evap Cooler unit {}", evapCond.Name));
                        ShowContinueError(state, " use a Setpoint Manager to establish a setpoint at the unit control node.");
                    } else {
                        bool localSetPointCheck = false;
                        EMSManager::CheckIfNodeSetPointManagedByEMS(state, ControlNode, HVAC::CtrlVarType::Temp, localSetPointCheck);
                        state.dataLoopNodes->NodeSetpointCheck(ControlNode).needsSetpointChecking = false;
                        // Let it slide apparently
                        if (localSetPointCheck) {
                            ShowSevereError(state, format("Missing temperature setpoint for Evap Cooler unit {}", evapCond.Name));
                            ShowContinueError(state, " use a Setpoint Manager to establish a setpoint at the unit control node.");
                            ShowContinueError(state, " or use an EMS actuator to establish a setpoint at the unit control node.");
                        }
                    }
                }
            }
        }
        state.dataEvapCoolers->MySetPointCheckFlag = false;
    }

    if (!state.dataGlobal->SysSizingCalc && evapCond.MySizeFlag) {
        // for each cooler, do the sizing once.
        SizeEvapCooler(state, EvapCoolNum);
        evapCond.MySizeFlag = false;
    }

    // Do the following initializations (every time step): This should be the info from
    // the previous components outlets or the node data in this section.

    // Transfer the node data to EvapCond data structure
    auto &thisInletNode = state.dataLoopNodes->Node(evapCond.InletNode);

    Real64 const RhoAir = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, thisInletNode.Temp, thisInletNode.HumRat);

    // set the volume flow rates from the input mass flow rates
    evapCond.VolFlowRate = thisInletNode.MassFlowRate / RhoAir;

    // Calculate the entering wet bulb temperature for inlet conditions
    evapCond.InletWetBulbTemp = Psychrometrics::PsyTwbFnTdbWPb(state, thisInletNode.Temp, thisInletNode.HumRat, state.dataEnvrn->OutBaroPress);

    // Set all of the inlet mass flow variables from the nodes
    evapCond.InletMassFlowRate = thisInletNode.MassFlowRate;
    evapCond.InletMassFlowRateMaxAvail = thisInletNode.MassFlowRateMaxAvail;
    evapCond.InletMassFlowRateMinAvail = thisInletNode.MassFlowRateMinAvail;
    // Set all of the inlet state variables from the inlet nodes
    evapCond.InletTemp = thisInletNode.Temp;
    evapCond.InletHumRat = thisInletNode.HumRat;
    evapCond.InletEnthalpy = thisInletNode.Enthalpy;
    evapCond.InletPressure = thisInletNode.Press;
    // Set default outlet state to inlet states(?)
    evapCond.OutletTemp = evapCond.InletTemp;
    evapCond.OutletHumRat = evapCond.InletHumRat;
    evapCond.OutletEnthalpy = evapCond.InletEnthalpy;
    evapCond.OutletPressure = evapCond.InletPressure;

    evapCond.OutletMassFlowRate = evapCond.InletMassFlowRate;
    evapCond.OutletMassFlowRateMaxAvail = evapCond.InletMassFlowRateMaxAvail;
    evapCond.OutletMassFlowRateMinAvail = evapCond.InletMassFlowRateMinAvail;

    // Set all of the secondary inlet mass flow variables from the nodes
    if (evapCond.SecondaryInletNode != 0) {
        auto const &thisSecInletNode = state.dataLoopNodes->Node(evapCond.SecondaryInletNode);
        evapCond.SecInletMassFlowRate = thisSecInletNode.MassFlowRate;
        evapCond.SecInletMassFlowRateMaxAvail = thisSecInletNode.MassFlowRateMaxAvail;
        evapCond.SecInletMassFlowRateMinAvail = thisSecInletNode.MassFlowRateMinAvail;
        evapCond.SecInletTemp = thisSecInletNode.Temp;
        evapCond.SecInletHumRat = thisSecInletNode.HumRat;
        evapCond.SecInletEnthalpy = thisSecInletNode.Enthalpy;
        evapCond.SecInletPressure = thisSecInletNode.Press;
    } else {
        evapCond.SecInletMassFlowRate = evapCond.IndirectVolFlowRate * state.dataEnvrn->OutAirDensity;
        evapCond.SecInletMassFlowRateMaxAvail = evapCond.IndirectVolFlowRate * state.dataEnvrn->OutAirDensity;
        evapCond.SecInletMassFlowRateMinAvail = 0.0;
        evapCond.SecInletTemp = state.dataEnvrn->OutDryBulbTemp;
        evapCond.SecInletHumRat =
            Psychrometrics::PsyWFnTdbTwbPb(state, state.dataEnvrn->OutDryBulbTemp, state.dataEnvrn->OutWetBulbTemp, state.dataEnvrn->OutBaroPress);
        evapCond.SecInletEnthalpy = state.dataEnvrn->OutEnthalpy;
        evapCond.SecInletPressure = state.dataEnvrn->OutBaroPress;
    }
    // Set the energy consumption to zero each time through for reporting
    evapCond.EvapCoolerEnergy = 0.0;
    evapCond.EvapCoolerPower = 0.0;
    evapCond.DewPointBoundFlag = 0;
    // Set the water consumption to zero each time through for reporting
    evapCond.EvapWaterConsumpRate = 0.0;
    evapCond.EvapWaterConsump = 0.0;
    evapCond.EvapWaterStarvMakup = 0.0;

    // Set the Saturation and Stage Efficiency to zero each time through for reporting
    evapCond.StageEff = 0.0;
    evapCond.SatEff = 0.0;

    // These initializations are done every iteration
    int OutNode = evapCond.OutletNode;
    int ControlNode = evapCond.EvapControlNodeNum;
    evapCond.IECOperatingStatus = 0;

    if (ControlNode == 0) {
        evapCond.DesiredOutletTemp = 0.0;
    } else if (ControlNode == OutNode) {
        evapCond.DesiredOutletTemp = state.dataLoopNodes->Node(ControlNode).TempSetPoint;
    } else {
        evapCond.DesiredOutletTemp = state.dataLoopNodes->Node(ControlNode).TempSetPoint -
                                     (state.dataLoopNodes->Node(ControlNode).Temp - state.dataLoopNodes->Node(OutNode).Temp);
    }
}

void SizeEvapCooler(EnergyPlusData &state, int const EvapCoolNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   March 2009
    //       MODIFIED       March 2014 Daeho Kang, Add sizing additional fields

    // PURPOSE OF THIS SUBROUTINE:
    // Size calculations for Evap coolers
    //  currently just for secondary side of Research Special Indirect evap cooler

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool IsAutoSize; // Indicator to autosize

    Real64 volFlowRateDes; // Autosized volume flow rate for reporting
    std::string CompType;  // for ease in getting objects

    // inits
    bool CoolerOnOApath = false;
    bool CoolerOnMainAirLoop = false;
    Real64 IndirectVolFlowRateDes = 0.0;  // Autosized volume flow rate for reporting
    Real64 IndirectVolFlowRateUser = 0.0; // Hardsized volume flow rate for reporting
    Real64 PadAreaDes = 0.0;              // Autosized celdek pad area for reporting
    Real64 PadAreaUser = 0.0;             // Hardsized celdek pad area for reporting
    Real64 PadDepthDes = 0.0;             // Autosized celdek pad depth for reporting
    Real64 PadDepthUser = 0.0;            // Hardsized celdek pad depth for reporting

    auto &CurSysNum(state.dataSize->CurSysNum);
    auto &CurZoneEqNum(state.dataSize->CurZoneEqNum);
    auto &FinalSysSizing(state.dataSize->FinalSysSizing);
    auto &EvapCond(state.dataEvapCoolers->EvapCond);
    auto &thisEvapCond(EvapCond(EvapCoolNum));

    bool HardSizeNoDesRun = !((state.dataSize->SysSizingRunDone || state.dataSize->ZoneSizingRunDone));
    bool SizingDesRunThisAirSys = false; // true if a particular air system had a Sizing:System object and system sizing done
    bool SizingDesRunThisZone = false;   // true if a particular zone had a Sizing:Zone object and zone sizing was done

    if (CurSysNum > 0) {
        CheckThisAirSystemForSizing(state, CurSysNum, SizingDesRunThisAirSys);
        if (SizingDesRunThisAirSys) {
            HardSizeNoDesRun = false; // Check if design information is available
        }
    }
    if (CurZoneEqNum > 0) {
        CheckThisZoneForSizing(state, CurZoneEqNum, SizingDesRunThisZone);
        // This next check was added during CppCheck corrections. This does not cause diffs
        // because SizingDesRunThisZone is not used below this point.
        // This check was commented to get back to original code and an issue is needed to correct.
        // Why no check for zone equipment?
        // if (SizingDesRunThisZone) {
        //    HardSizeNoDesRun = false; // Check if design information is available
        //}
    }
    // I don't think the sizing logic is correct when it comes to autosized vs hard-sized inputs
    // or input files that use Sizing:Zone or Sizing:System with autosized and/or hard-sized inputs

    CompType = evapCoolerTypeNames[static_cast<int>(thisEvapCond.evapCoolerType)];

    // Search once for the object on an air system
    if (CurSysNum > 0) { // central system
        // where is this cooler located, is it on OA system or main loop?
        // search for this component in Air loop branches.
        for (int AirSysBranchLoop = 1; AirSysBranchLoop <= state.dataAirSystemsData->PrimaryAirSystems(CurSysNum).NumBranches; ++AirSysBranchLoop) {
            for (int BranchComp = 1; BranchComp <= state.dataAirSystemsData->PrimaryAirSystems(CurSysNum).Branch(AirSysBranchLoop).TotalComponents;
                 ++BranchComp) {

                if (Util::SameString(state.dataAirSystemsData->PrimaryAirSystems(CurSysNum).Branch(AirSysBranchLoop).Comp(BranchComp).Name,
                                     thisEvapCond.Name)) {
                    CoolerOnMainAirLoop = true;
                }
            }
        }
        if (!CoolerOnMainAirLoop) {
            CoolerOnOApath = true;
        }
    }

    // Start with the indirect volume flow rate
    IsAutoSize = false;
    if (thisEvapCond.IndirectVolFlowRate == DataSizing::AutoSize) {
        IsAutoSize = true;
    }
    if (CurSysNum > 0 && !IsAutoSize && !SizingDesRunThisAirSys) {
        HardSizeNoDesRun = true;
    }
    if (CurSysNum > 0) { // central system
        if (!IsAutoSize && !SizingDesRunThisAirSys) {
            if (thisEvapCond.IndirectVolFlowRate > 0.0) {
                if (thisEvapCond.evapCoolerType == EvapCoolerType::IndirectCELDEKPAD ||
                    thisEvapCond.evapCoolerType == EvapCoolerType::IndirectWETCOIL ||
                    thisEvapCond.evapCoolerType == EvapCoolerType::IndirectRDDSpecial) {
                    BaseSizer::reportSizerOutput(
                        state, CompType, thisEvapCond.Name, "User-Specified Secondary Fan Flow Rate [m3/s]", thisEvapCond.IndirectVolFlowRate);
                }
            }
        } else { // Autosize or hardsize with design data
            CheckSysSizing(state, CompType, thisEvapCond.Name);
            if (CoolerOnMainAirLoop) {
                IndirectVolFlowRateDes = FinalSysSizing(CurSysNum).DesMainVolFlow;
            } else if (CoolerOnOApath) {
                IndirectVolFlowRateDes = max(FinalSysSizing(CurSysNum).DesOutAirVolFlow, 0.5 * FinalSysSizing(CurSysNum).DesMainVolFlow);
            }
            // apply scaling factor the secondary air fan flow rate
            if (thisEvapCond.evapCoolerType == EvapCoolerType::IndirectRDDSpecial) {
                IndirectVolFlowRateDes = IndirectVolFlowRateDes * thisEvapCond.IndirectVolFlowScalingFactor;
            }
        }
    } else if (CurZoneEqNum > 0) {                    // zone equipment
        if (!IsAutoSize && !SizingDesRunThisAirSys) { // this should be SizingDesRunThisZone
            if (thisEvapCond.IndirectVolFlowRate > 0.0) {
                // report for the indirect evap cooler types only
                if (thisEvapCond.evapCoolerType == EvapCoolerType::IndirectCELDEKPAD ||
                    thisEvapCond.evapCoolerType == EvapCoolerType::IndirectWETCOIL ||
                    thisEvapCond.evapCoolerType == EvapCoolerType::IndirectRDDSpecial) {
                    BaseSizer::reportSizerOutput(
                        state, CompType, thisEvapCond.Name, "User-Specified Secondary Fan Flow Rate [m3/s]", thisEvapCond.IndirectVolFlowRate);
                }
            }
        } else { // Autosize or hardsize with design data
            // zone equip evap coolers
            IndirectVolFlowRateDes = state.dataSize->FinalZoneSizing(CurZoneEqNum).DesCoolVolFlow;
            // apply scaling factor the secondary air fan flow rate
            if (thisEvapCond.evapCoolerType == EvapCoolerType::IndirectRDDSpecial) {
                IndirectVolFlowRateDes = IndirectVolFlowRateDes * thisEvapCond.IndirectVolFlowScalingFactor;
            }
        }
    }
    if (!HardSizeNoDesRun) {
        if (IsAutoSize) {
            thisEvapCond.IndirectVolFlowRate = IndirectVolFlowRateDes;
            if (thisEvapCond.evapCoolerType == EvapCoolerType::IndirectCELDEKPAD || thisEvapCond.evapCoolerType == EvapCoolerType::IndirectWETCOIL ||
                thisEvapCond.evapCoolerType == EvapCoolerType::IndirectRDDSpecial) {
                BaseSizer::reportSizerOutput(
                    state, CompType, thisEvapCond.Name, "Design Size Secondary Fan Flow Rate [m3/s]", thisEvapCond.IndirectVolFlowRate);
            }
        } else {
            if (thisEvapCond.IndirectVolFlowRate > 0.0 && IndirectVolFlowRateDes > 0.0) {
                IndirectVolFlowRateUser = thisEvapCond.IndirectVolFlowRate;
                BaseSizer::reportSizerOutput(state,
                                             "EvaporativeCooler:Indirect:ResearchSpecial",
                                             thisEvapCond.Name,
                                             "Design Size Secondary Fan Flow Rate [m3/s]",
                                             IndirectVolFlowRateDes,
                                             "User-Specified Secondary Fan Flow Rate [m3/s]",
                                             IndirectVolFlowRateUser);
                if (state.dataGlobal->DisplayExtraWarnings) {
                    if ((std::abs(IndirectVolFlowRateDes - IndirectVolFlowRateUser) / IndirectVolFlowRateUser) >
                        state.dataSize->AutoVsHardSizingThreshold) {
                        ShowMessage(state,
                                    format("SizeEvaporativeCooler:Indirect:ResearchSpecial: Potential issue with equipment sizing for {}",
                                           thisEvapCond.Name));
                        ShowContinueError(state, format("User-Specified Secondary Fan Flow Rate of {:.5R} [m3/s]", IndirectVolFlowRateUser));
                        ShowContinueError(state, format("differs from Design Size Secondary Fan Flow Rate of {:.5R} [m3/s]", IndirectVolFlowRateDes));
                        ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                        ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                    }
                }
            }
        }
    }

    // Next up the other volume flow rate
    IsAutoSize = false;
    if (thisEvapCond.DesVolFlowRate == DataSizing::AutoSize) {
        IsAutoSize = true;
    }
    if (CurSysNum > 0 && !IsAutoSize && !SizingDesRunThisAirSys) {
        HardSizeNoDesRun = true;
    }
    if (CurSysNum > 0) { // central system
        if (!IsAutoSize && !SizingDesRunThisAirSys) {
            // the .VolFlowRate variable wasn't reported to the eio in develop, so not doing it here
            // if ( EvapCond( EvapCoolNum ).VolFlowRate > 0.0 ) {
            // BaseSizer::reportSizerOutput( CompType, EvapCond( EvapCoolNum ).Name,
            //"User-Specified Secondary Fan Flow Rate [m3/s]", EvapCond( EvapCoolNum ).VolFlowRate );
            //}
        } else { // Autosize or hardsize with design data
            CheckSysSizing(state, CompType, thisEvapCond.Name);
            if (CoolerOnMainAirLoop) {
                volFlowRateDes = FinalSysSizing(CurSysNum).DesMainVolFlow;
            } else if (CoolerOnOApath) {
                volFlowRateDes = max(FinalSysSizing(CurSysNum).DesOutAirVolFlow, 0.5 * FinalSysSizing(CurSysNum).DesMainVolFlow);
            }
            // no scaling factor on the volFlowRate in develop, so not doing it here
        }
    } else if (CurZoneEqNum > 0) { // zone equipment
        // zone equip evap coolers

        // this should be SizingDesRunThisZone
        if (!IsAutoSize && !SizingDesRunThisAirSys) {
            // the .VolFlowRate variable wasn't reported to the eio in develop, so not doing it here
            // if ( EvapCond( EvapCoolNum ).VolFlowRate > 0.0 ) {
            // BaseSizer::reportSizerOutput( "EvaporativeCooler:Indirect:ResearchSpecial", EvapCond( EvapCoolNum ).Name,
            //"User-Specified Secondary Fan Flow Rate [m3/s]", EvapCond( EvapCoolNum ).VolFlowRate );
            //}
        } else { // Autosize or hardsize with design data
            volFlowRateDes = state.dataSize->FinalZoneSizing(CurZoneEqNum).DesCoolVolFlow;
        }

    } else { // zone equipment
        // can't do zone equip evap coolers yet
    }
    if (!HardSizeNoDesRun) {
        if (IsAutoSize) {
            thisEvapCond.DesVolFlowRate = volFlowRateDes;
            // only these two evap coolers has primary air design flow rate
            if (thisEvapCond.evapCoolerType == EvapCoolerType::IndirectRDDSpecial) {
                BaseSizer::reportSizerOutput(state,
                                             "EvaporativeCooler:Indirect:ResearchSpecial",
                                             thisEvapCond.Name,
                                             "Primary Air Design Flow Rate [m3/s]",
                                             thisEvapCond.DesVolFlowRate);
                BaseSizer::reportSizerOutput(state,
                                             "EvaporativeCooler:Indirect:ResearchSpecial",
                                             thisEvapCond.Name,
                                             "Secondary Air Design Flow Rate [m3/s]",
                                             thisEvapCond.IndirectVolFlowRate);
            } else if (thisEvapCond.evapCoolerType == EvapCoolerType::DirectResearchSpecial) {
                BaseSizer::reportSizerOutput(state,
                                             "EvaporativeCooler:Direct:ResearchSpecial",
                                             thisEvapCond.Name,
                                             "Primary Air Design Flow Rate [m3/s]",
                                             thisEvapCond.DesVolFlowRate);
            }
        } else {
            // the .VolFlowRate variable wasn't reported to the eio in develop, so not doing it here
            // if ( EvapCond( EvapCoolNum ).IndirectVolFlowRate > 0.0 && IndirectVolFlowRateDes > 0.0 ) {
            // IndirectVolFlowRateUser = EvapCond( EvapCoolNum ).IndirectVolFlowRate;
            // BaseSizer::reportSizerOutput( "EvaporativeCooler:Indirect:ResearchSpecial", EvapCond( EvapCoolNum ).Name,
            //"Design Size Secondary Fan Flow Rate [m3/s]", IndirectVolFlowRateDes,
            //"User-Specified Secondary Fan Flow Rate [m3/s]", IndirectVolFlowRateUser );
            // if ( DisplayExtraWarnings ) {
            // if ( ( std::abs( IndirectVolFlowRateDes - IndirectVolFlowRateUser ) / IndirectVolFlowRateUser ) > AutoVsHardSizingThreshold ) {
            // ShowMessage(state, format("SizeEvaporativeCooler:Indirect:ResearchSpecial: \nPotential issue with equipment sizing for {}", EvapCond(
            // EvapCoolNum
            // ).Name));  ShowContinueError(state, format("User-Specified Secondary Fan Flow Rate of {} [m3/s]", RoundSigDigits(
            // IndirectVolFlowRateUser, 5 ))); ShowContinueError(state,  format("differs from Design Size Secondary Fan Flow Rate of
            // {:.5R}", IndirectVolFlowRateDes) + " [m3/s]" ); ShowContinueError(state,  "This may, or may not, indicate mismatched component
            // sizes." ); ShowContinueError(state,  "Verify that the value entered is intended and is consistent with other components." );
            //}
            //}
            //}
        }
    }

    if (thisEvapCond.evapCoolerType == EvapCoolerType::DirectCELDEKPAD) {
        IsAutoSize = false;
        if (thisEvapCond.PadArea == DataSizing::AutoSize) {
            IsAutoSize = true;
        }
        if (CurSysNum > 0 && !IsAutoSize && !SizingDesRunThisAirSys) {
            HardSizeNoDesRun = true;
        }
        if (SizingDesRunThisAirSys) {
            HardSizeNoDesRun = false; // Check if design information is available
        }
        // Design air flow rate
        if (CurSysNum > 0) { // central system
            if (!IsAutoSize && !SizingDesRunThisAirSys) {
                HardSizeNoDesRun = true;
                if (thisEvapCond.PadArea > 0.0) {
                    BaseSizer::reportSizerOutput(
                        state, "EvaporativeCooler:Direct:CelDekPad", thisEvapCond.Name, "User-Specified Celdek Pad Area [m2]", thisEvapCond.PadArea);
                }
            } else { // Autosize or hardsize with design data
                CheckSysSizing(state, CompType, thisEvapCond.Name);
                if (CoolerOnMainAirLoop) {
                    IndirectVolFlowRateDes = FinalSysSizing(CurSysNum).DesMainVolFlow;
                } else if (CoolerOnOApath) {
                    IndirectVolFlowRateDes = std::max(FinalSysSizing(CurSysNum).DesOutAirVolFlow, 0.50 * FinalSysSizing(CurSysNum).DesMainVolFlow);
                }
                // Face air velocity of 3m/s is assumed
                PadAreaDes = IndirectVolFlowRateDes / 3.0;
            }
        } else if (CurZoneEqNum > 0) { // zone equipment
            // zone equip evap coolers

            // this should be SizingDesRunThisZone
            if (!IsAutoSize && !SizingDesRunThisAirSys) {
                HardSizeNoDesRun = true;
                if (thisEvapCond.PadArea > 0.0) {
                    // report for the indirect evap cooler types only
                    BaseSizer::reportSizerOutput(
                        state, "EvaporativeCooler:Direct:CelDekPad", thisEvapCond.Name, "User-Specified Celdek Pad Area [m2]", thisEvapCond.PadArea);
                }
            } else { // Autosize or hardsize with design data
                // zone equip evap coolers
                IndirectVolFlowRateDes = state.dataSize->FinalZoneSizing(CurZoneEqNum).DesCoolVolFlow;
                // Face air velocity of 3m/s is assumed
                PadAreaDes = IndirectVolFlowRateDes / 3.0;
            }
        } else {
        }

        if (!HardSizeNoDesRun) {
            if (IsAutoSize) {
                thisEvapCond.PadArea = PadAreaDes;
                BaseSizer::reportSizerOutput(
                    state, "EvaporativeCooler:Direct:CelDekPad", thisEvapCond.Name, "Design Size Celdek Pad Area [m2]", PadAreaDes);
            } else {
                if (thisEvapCond.PadArea > 0.0 && PadAreaDes > 0.0) {
                    PadAreaUser = thisEvapCond.PadArea;
                    BaseSizer::reportSizerOutput(state,
                                                 "EvaporativeCooler:Direct:CelDekPad",
                                                 thisEvapCond.Name,
                                                 "Design Size Celdek Pad Area [m2]",
                                                 PadAreaDes,
                                                 "User-Specified Celdek Pad Area [m2]",
                                                 PadAreaUser);
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        if ((std::abs(PadAreaDes - PadAreaUser) / PadAreaUser) > state.dataSize->AutoVsHardSizingThreshold) {
                            ShowMessage(
                                state,
                                format("SizeEvaporativeCooler:Direct:CelDekPad: Potential issue with equipment sizing for {}", thisEvapCond.Name));
                            ShowContinueError(state, format("User-Specified Celdek Pad Area of{:.2R} [m2]", PadAreaUser));
                            ShowContinueError(state, format("differs from Design Size Celdek Pad Area of {:.2R} [m2]", PadAreaDes));
                            ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                        }
                    }
                }
            }
        }

        IsAutoSize = false;
        if (thisEvapCond.PadDepth == DataSizing::AutoSize) {
            IsAutoSize = true;
        }
        if (CurSysNum > 0 && !IsAutoSize && !SizingDesRunThisAirSys) {
            HardSizeNoDesRun = true;
        }
        // The following regression equation is used to determine pad depth,
        // assuming saturation effectiveness of 70% and face air velocity of 3m/s:
        // Effectiveness = 0.792714 + 0.958569D - 0.25193V - 1.03215D^2 + 0.0262659V^2 + 0.914869DV -
        // 1.48241VD^2 - 0.018992V^3D + 1.13137D^3V + 0.0327622V^3D^2 - 0.145384D^3V^2
        PadDepthDes = 0.17382;
        if (IsAutoSize) {
            thisEvapCond.PadDepth = PadDepthDes;
            BaseSizer::reportSizerOutput(
                state, "EvaporativeCooler:Direct:CelDekPad", thisEvapCond.Name, "Design Size Celdek Pad Depth [m]", PadDepthDes);
        } else {
            if (thisEvapCond.PadDepth > 0.0 && PadDepthDes > 0.0) {
                PadDepthUser = thisEvapCond.PadDepth;
                BaseSizer::reportSizerOutput(state,
                                             "EvaporativeCooler:Direct:CelDekPad",
                                             thisEvapCond.Name,
                                             "Design Size Celdek Pad Depth [m]",
                                             PadDepthDes,
                                             "User-Specified Celdek Pad Depth [m]",
                                             PadDepthUser);
                if (state.dataGlobal->DisplayExtraWarnings) {
                    if ((std::abs(PadDepthDes - PadDepthUser) / PadDepthUser) > state.dataSize->AutoVsHardSizingThreshold) {
                        ShowMessage(
                            state, format("SizeEvaporativeCooler:Direct:CelDekPad: Potential issue with equipment sizing for {}", thisEvapCond.Name));
                        ShowContinueError(state, format("User-Specified Celdek Pad Depth of {:.2R} [m]", PadDepthUser));
                        ShowContinueError(state, format("differs from Design Size Celdek Pad Depth of {:.2R} [m]", PadDepthDes));
                        ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                        ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                    }
                }
            }
        }
    }

    if (thisEvapCond.evapCoolerType == EvapCoolerType::IndirectCELDEKPAD) {
        IsAutoSize = false;

        if (thisEvapCond.IndirectPadArea == DataSizing::AutoSize) {
            IsAutoSize = true;
        }
        if (SizingDesRunThisAirSys) {
            HardSizeNoDesRun = false; // Check if design information is available
        }
        // Design air flow rate
        if (CurSysNum > 0) { // central system
            // where is this cooler located, is it on OA system or main loop?
            // search for this component in Air loop branches.
            for (int AirSysBranchLoop = 1; AirSysBranchLoop <= state.dataAirSystemsData->PrimaryAirSystems(CurSysNum).NumBranches;
                 ++AirSysBranchLoop) {
                for (int BranchComp = 1;
                     BranchComp <= state.dataAirSystemsData->PrimaryAirSystems(CurSysNum).Branch(AirSysBranchLoop).TotalComponents;
                     ++BranchComp) {
                    if (Util::SameString(state.dataAirSystemsData->PrimaryAirSystems(CurSysNum).Branch(AirSysBranchLoop).Comp(BranchComp).Name,
                                         thisEvapCond.Name)) {
                        CoolerOnMainAirLoop = true;
                    }
                }
            }
            if (!IsAutoSize && !SizingDesRunThisAirSys) {
                HardSizeNoDesRun = true;
                if (thisEvapCond.IndirectPadArea > 0.0) {
                    BaseSizer::reportSizerOutput(state,
                                                 "EvaporativeCooler:Indirect:CelDekPad",
                                                 thisEvapCond.Name,
                                                 "User-Specified Celdek Pad Area [m2]",
                                                 thisEvapCond.IndirectPadArea);
                }
            } else { // Autosize or hardsize with design data
                CheckSysSizing(state, CompType, thisEvapCond.Name);
                if (!CoolerOnMainAirLoop) {
                    CoolerOnOApath = true;
                }
                if (CoolerOnMainAirLoop) {
                    IndirectVolFlowRateDes = FinalSysSizing(CurSysNum).DesMainVolFlow;
                } else if (CoolerOnOApath) {
                    IndirectVolFlowRateDes = std::max(FinalSysSizing(CurSysNum).DesOutAirVolFlow, 0.5 * FinalSysSizing(CurSysNum).DesMainVolFlow);
                }
                // Face air velocity of 3m/s is assumed
                PadAreaDes = IndirectVolFlowRateDes / 3.0;
            }
        } else if (CurZoneEqNum > 0) { // zone equipment
            // zone equip evap coolers
            if (!IsAutoSize && !SizingDesRunThisAirSys) { // this should be SizingDesRunThisZone
                HardSizeNoDesRun = true;
                if (thisEvapCond.IndirectPadArea > 0.0) {
                    // report for the indirect evap cooler types only
                    if (thisEvapCond.PadArea > 0.0) {
                        BaseSizer::reportSizerOutput(state,
                                                     "EvaporativeCooler:Indirect:CelDekPad",
                                                     thisEvapCond.Name,
                                                     "User-Specified Celdek Pad Area [m2]",
                                                     thisEvapCond.IndirectPadArea);
                    }
                }
            } else { // Autosize or hardsize with design data
                // zone equip evap coolers
                IndirectVolFlowRateDes = state.dataSize->FinalZoneSizing(CurZoneEqNum).DesCoolVolFlow;
                // Face air velocity of 3m/s is assumed
                PadAreaDes = IndirectVolFlowRateDes / 3.0;
            }
        } else {
        }

        if (!HardSizeNoDesRun) {
            if (IsAutoSize) {
                thisEvapCond.IndirectPadArea = PadAreaDes;
                BaseSizer::reportSizerOutput(
                    state, "EvaporativeCooler:Indirect:CelDekPad", thisEvapCond.Name, "Design Size Celdek Pad Area [m2]", PadAreaDes);
            } else {
                if (thisEvapCond.IndirectPadArea > 0.0 && PadAreaDes > 0.0) {
                    PadAreaUser = thisEvapCond.IndirectPadArea;
                    BaseSizer::reportSizerOutput(state,
                                                 "EvaporativeCooler:Indirect:CelDekPad",
                                                 thisEvapCond.Name,
                                                 "Design Size Celdek Pad Area [m2]",
                                                 PadAreaDes,
                                                 "User-Specified Celdek Pad Area [m2]",
                                                 PadAreaUser);
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        if ((std::abs(PadAreaDes - PadAreaUser) / PadAreaUser) > state.dataSize->AutoVsHardSizingThreshold) {
                            ShowMessage(
                                state,
                                format("SizeEvaporativeCooler:Indirect:CelDekPad: Potential issue with equipment sizing for {}", thisEvapCond.Name));
                            ShowContinueError(state, format("User-Specified Celdek Pad Area {:.2R} [m2]", PadAreaUser));
                            ShowContinueError(state, format("differs from Design Size Celdek Pad Area of {:.2R} [m2]", PadAreaDes));
                            ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                            ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                        }
                    }
                }
            }
        }

        IsAutoSize = thisEvapCond.IndirectPadDepth == DataSizing::AutoSize;
        // The following regression equation is used to determine pad depth,
        // assuming saturation effectiveness of 70% and face air velocity of 3m/s:
        // Effectiveness = 0.792714 + 0.958569D - 0.25193V - 1.03215D^2 + 0.0262659V^2 + 0.914869DV -
        // 1.48241VD^2 - 0.018992V^3D + 1.13137D^3V + 0.0327622V^3D^2 - 0.145384D^3V^2

        PadDepthDes = 0.17382;
        if (IsAutoSize) {
            thisEvapCond.IndirectPadDepth = PadDepthDes;
            BaseSizer::reportSizerOutput(
                state, "EvaporativeCooler:Indirect:CelDekPad", thisEvapCond.Name, "Design Size Celdek Pad Depth [m]", PadDepthDes);
        } else {
            if (thisEvapCond.IndirectPadDepth > 0.0 && PadDepthDes > 0.0) {
                PadDepthUser = thisEvapCond.IndirectPadDepth;
                BaseSizer::reportSizerOutput(state,
                                             "EvaporativeCooler:Indirect:CelDekPad",
                                             thisEvapCond.Name,
                                             "Design Size Celdek Pad Depth [m]",
                                             PadDepthDes,
                                             "User-Specified Celdek Pad Depth [m]",
                                             PadDepthUser);
                if (state.dataGlobal->DisplayExtraWarnings) {
                    if ((std::abs(PadDepthDes - PadDepthUser) / PadDepthUser) > state.dataSize->AutoVsHardSizingThreshold) {
                        ShowMessage(
                            state,
                            format("SizeEvaporativeCooler:Indirect:CelDekPad: Potential issue with equipment sizing for {}", thisEvapCond.Name));
                        ShowContinueError(state, format("User-Specified Celdek Pad Depth of {:.2R} [m]", PadDepthUser));
                        ShowContinueError(state, format("differs from Design Size Celdek Pad Depth of {:.2R} [m]", PadDepthDes));
                        ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                        ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                    }
                }
            }
        }
    }

    if (thisEvapCond.evapCoolerType == EvapCoolerType::IndirectRDDSpecial) {
        // secondary air fan sizing: Secondary flow Rate (m3/s) * Fan Flow Sizing Factor (W/(m3/s)
        if (thisEvapCond.IndirectFanPower == DataSizing::AutoSize) {
            thisEvapCond.IndirectFanPower = thisEvapCond.IndirectVolFlowRate * thisEvapCond.FanSizingSpecificPower;
            BaseSizer::reportSizerOutput(
                state, "EvaporativeCooler:Indirect:ResearchSpecial", thisEvapCond.Name, "Secondary Fan Power [W]", thisEvapCond.IndirectFanPower);
        }
        // recirculating water pump sizing: Secondary flow Rate (m3/s) * Pump Sizing Factor (W/(m3/s)
        if (thisEvapCond.IndirectRecircPumpPower == DataSizing::AutoSize) {
            thisEvapCond.IndirectRecircPumpPower = thisEvapCond.IndirectVolFlowRate * thisEvapCond.RecircPumpSizingFactor;
            BaseSizer::reportSizerOutput(state,
                                         "EvaporativeCooler:Indirect:ResearchSpecial",
                                         thisEvapCond.Name,
                                         "Recirculating Pump Power [W]",
                                         thisEvapCond.IndirectRecircPumpPower);
        }
    }

    if (thisEvapCond.evapCoolerType == EvapCoolerType::DirectResearchSpecial) {
        // recirculating water pump sizing: Primary Air Design flow Rate (m3/s) * Pump Sizing Factor (W/(m3/s)
        if (thisEvapCond.RecircPumpPower == DataSizing::AutoSize) {
            thisEvapCond.RecircPumpPower = thisEvapCond.DesVolFlowRate * thisEvapCond.RecircPumpSizingFactor;
            BaseSizer::reportSizerOutput(
                state, "EvaporativeCooler:Direct:ResearchSpecial", thisEvapCond.Name, "Recirculating Pump Power [W]", thisEvapCond.RecircPumpPower);
        }
    }
}

void CalcDirectEvapCooler(EnergyPlusData &state, int EvapCoolNum, Real64 const PartLoadRatio)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   October 2000

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 PadDepth; // EvapCooler Pad Depth in Meters as input by the User
    Real64 SatEff;   // Saturation Efficiency of the CelDek Pad
    Real64 AirVel;   // The Calculated Air Velocity through the Pad
    Real64 TEDB;     // Entering Dry Bulb Temperature
    Real64 TEWB;     // Entering Wet Bulb Temperature
    Real64 RhoWater;

    auto &thisEvapCond(state.dataEvapCoolers->EvapCond(EvapCoolNum));

    // If the Evaporative Cooler  is operating there should be some mass flow rate
    //  Also the evap cooler has to be scheduled to be available
    if ((thisEvapCond.InletMassFlowRate > 0.0) && (thisEvapCond.availSched->getCurrentVal() > 0.0)) {

        PadDepth = thisEvapCond.PadDepth;
        //******************************************************************************
        //   THIS SUBROUTINE WILL CACULATE THE TEMPERATURE OF THE LEAVING AIR DRY BULB
        //   FOR A DIRECT EVAPORATIVE AIR COOLER SUPPLIED WITH CFMAir,DIRPAD,TEWB,TEDB,
        //   AND PB (ATM. PRESS.) FOR AIR DENSITY CALCULATIONS.
        //******************************************************************************

        AirVel = thisEvapCond.VolFlowRate / thisEvapCond.PadArea;

        //******************************************************************************
        //   SAT EFF IS FOR DIFFERENT THICKNESS CELDEK PAD (CURVE FIT FROM DATA)
        //******************************************************************************
        SatEff = 0.792714 + 0.958569 * PadDepth - 0.25193 * AirVel - 1.03215 * pow_2(PadDepth) + 2.62659e-2 * pow_2(AirVel) +
                 0.914869 * PadDepth * AirVel - 1.48241 * AirVel * pow_2(PadDepth) - 1.89919e-2 * pow_3(AirVel) * PadDepth +
                 1.13137 * pow_3(PadDepth) * AirVel + 3.27622e-2 * pow_3(AirVel) * pow_2(PadDepth) - 0.145384 * pow_3(PadDepth) * pow_2(AirVel);

        if (SatEff >= 1.0) {
            SatEff = 1.0;
        }
        if (SatEff < 0.0) { // we have a serious problem.  Pad Area and/or depth not suitable for system air flow rates
            ShowSevereError(state, format("EVAPCOOLER:DIRECT:CELDEKPAD: {} has a problem", thisEvapCond.Name));
            ShowContinueError(state, "Check size of Pad Area and/or Pad Depth in input");
            ShowContinueError(state, format("Cooler Effectiveness calculated as: {:.2R}", SatEff));
            ShowContinueError(state, format("Air velocity (m/s) through pads calculated as: {:.2R}", AirVel));
            ShowFatalError(state, "Program Terminates due to previous error condition");
        }
        thisEvapCond.SatEff = SatEff;
        //***************************************************************************
        //   TEMP LEAVING DRY BULB IS CALCULATED FROM SATURATION EFFICIENCY AS THE
        //   DRY BULB TEMP APPROACHES THE WET BULB TEMP. WET BULB TEMP IS CONSTANT
        //   ACROSS A DIRECT EVAPORATION COOLER.
        TEWB = thisEvapCond.InletWetBulbTemp;
        TEDB = thisEvapCond.InletTemp;

        thisEvapCond.OutletTemp = TEDB - ((TEDB - TEWB) * SatEff);

        thisEvapCond.OuletWetBulbTemp = thisEvapCond.InletWetBulbTemp;

        thisEvapCond.OutletHumRat = Psychrometrics::PsyWFnTdbTwbPb(state, thisEvapCond.OutletTemp, TEWB, state.dataEnvrn->OutBaroPress);

        thisEvapCond.OutletEnthalpy = Psychrometrics::PsyHFnTdbW(thisEvapCond.OutletTemp, thisEvapCond.OutletHumRat);

        //***************************************************************************
        //                  ENERGY CONSUMED BY THE RECIRCULATING PUMP
        // Add the pump energy to the total Evap Cooler energy consumption
        thisEvapCond.EvapCoolerPower += PartLoadRatio * thisEvapCond.RecircPumpPower;
        //******************
        //             WATER CONSUMPTION IN m3 OF WATER FOR DIRECT
        //             H2O [m3/s] = Delta W[kgWater/kDryAir]*Mass Flow Air[kgDryAir/s]
        //                                /RhoWater [kgWater/m3]
        //******************
        RhoWater = Psychrometrics::RhoH2O(thisEvapCond.OutletTemp);
        thisEvapCond.EvapWaterConsumpRate = (thisEvapCond.OutletHumRat - thisEvapCond.InletHumRat) * thisEvapCond.InletMassFlowRate / RhoWater;
        // A numerical check to keep from having very tiny negative water consumption values being reported
        if (thisEvapCond.EvapWaterConsumpRate < 0.0) {
            thisEvapCond.EvapWaterConsumpRate = 0.0;
        }

    } else {
        // The evap cooler is not running and does not change conditions from inlet to outlet
        thisEvapCond.OutletTemp = thisEvapCond.InletTemp;

        thisEvapCond.OuletWetBulbTemp = thisEvapCond.InletWetBulbTemp;

        thisEvapCond.OutletHumRat = thisEvapCond.InletHumRat;

        thisEvapCond.OutletEnthalpy = thisEvapCond.InletEnthalpy;

        thisEvapCond.EvapCoolerEnergy = 0.0;

        thisEvapCond.EvapWaterConsumpRate = 0.0;
    }
    // all of the mass flowrates are not changed across the evap cooler
    thisEvapCond.OutletMassFlowRate = thisEvapCond.InletMassFlowRate;
    thisEvapCond.OutletMassFlowRateMaxAvail = thisEvapCond.InletMassFlowRateMaxAvail;
    thisEvapCond.OutletMassFlowRateMinAvail = thisEvapCond.InletMassFlowRateMinAvail;

    // the pressure is not changed across the evap cooler
    thisEvapCond.OutletPressure = thisEvapCond.InletPressure;
}

void CalcDryIndirectEvapCooler(EnergyPlusData &state, int EvapCoolNum, Real64 const PartLoadRatio)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   October 2000
    //       MODIFIED       BG Feb. 2007 secondary air inlet node

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 PadDepth;  // EvapCooler Pad Depth in Meters as input by the User
    Real64 SatEff;    // Saturation Efficiency of the CelDek Pad
    Real64 AirVel;    // The Calculated Air Velocity through the Pad
    Real64 TDBSec;    // Secondary leaving dry bulb
    Real64 TWBSec;    // Secondary Leaving Wet Bulb
    Real64 HumRatSec; // Secondary leaving Humidity Ratio
    Real64 EffHX;     // Effectiveness of Secondary Heat Exchanger
    Real64 QHX;       // Q Across Sec HX
    Real64 RhoWater;
    Real64 RhoAir; // Density of the primary side air
    Real64 CpAir;  // Cp of the primary side air
    Real64 CFMAir;
    Real64 CFMSec;

    auto &thisEvapCond(state.dataEvapCoolers->EvapCond(EvapCoolNum));

    // If the Evaporative Cooler  is operating there should be some mass flow rate
    //  Also the evap cooler has to be scheduled to be available
    if ((thisEvapCond.InletMassFlowRate > 0.0) && (thisEvapCond.availSched->getCurrentVal() > 0.0)) {

        PadDepth = thisEvapCond.IndirectPadDepth;
        //******************************************************************************
        //   THIS SUBROUTINE WILL CACULATE THE TEMPERATURE OF THE LEAVING AIR DRY BULB
        //   FOR A DIRECT EVAPORATIVE AIR COOLER SUPPLIED WITH CFMAir,DIRPAD,TEWB,TEDB,
        //   AND PB (ATM. PRESS.) FOR AIR DENSITY CALCULATIONS.
        //******************************************************************************

        AirVel = thisEvapCond.IndirectVolFlowRate / thisEvapCond.IndirectPadArea;

        //******************************************************************************
        //   SAT EFF IS FOR DIFFERENT THICKNESS CELDEK PAD (CURVE FIT FROM DATA)
        //******************************************************************************
        SatEff = 0.792714 + 0.958569 * PadDepth - 0.25193 * AirVel - 1.03215 * pow_2(PadDepth) + 2.62659e-2 * pow_2(AirVel) +
                 0.914869 * PadDepth * AirVel - 1.48241 * AirVel * pow_2(PadDepth) - 1.89919e-2 * pow_3(AirVel) * PadDepth +
                 1.13137 * pow_3(PadDepth) * AirVel + 3.27622e-2 * pow_3(AirVel) * pow_2(PadDepth) - 0.145384 * pow_3(PadDepth) * pow_2(AirVel);

        if (SatEff >= 1.0) {
            SatEff = 1.0;
        }
        thisEvapCond.SatEff = SatEff;
        //***************************************************************************
        //   TEMP LEAVING DRY BULB IS CALCULATED FROM SATURATION EFFICIENCY AS THE
        //   DRY BULB TEMP APPROACHES THE WET BULB TEMP ACROSS THE PAD BEFORE THE HX.
        //***************************************************************************
        //***** FIRST CHECK IF THIS TEWB IS A FEASIBLE POINT ON PSYCH CHART**********

        // BG Feb 2007 mods for oa node (eg. height-dependent outside air model)
        TWBSec = Psychrometrics::PsyTwbFnTdbWPb(state,
                                                thisEvapCond.SecInletTemp,
                                                thisEvapCond.SecInletHumRat,
                                                thisEvapCond.SecInletPressure); //  OutWetBulbTemp
        TDBSec = thisEvapCond.SecInletTemp - ((thisEvapCond.SecInletTemp - TWBSec) * SatEff);

        HumRatSec = Psychrometrics::PsyWFnTdbTwbPb(state, TDBSec, TWBSec, thisEvapCond.SecInletPressure);

        //***************************************************************************
        //                  CALCULATE THE TLDB FROM HX EQUATIONS GIVEN AN EFFICIENCY
        //***************************************************************************
        EffHX = thisEvapCond.IndirectHXEffectiveness;
        CpAir = Psychrometrics::PsyCpAirFnW(thisEvapCond.InletHumRat);
        RhoAir = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, thisEvapCond.InletTemp, thisEvapCond.InletHumRat);
        CFMAir = thisEvapCond.VolFlowRate;         // Volume Flow Rate Primary Side
        CFMSec = thisEvapCond.IndirectVolFlowRate; // Volume Flow Rate Secondary Side

        QHX = EffHX * min(CFMSec, CFMAir) * RhoAir * CpAir * (thisEvapCond.InletTemp - TDBSec);
        thisEvapCond.OutletTemp = thisEvapCond.InletTemp - QHX / (RhoAir * CFMAir * CpAir);
        // This is a rough approximation of the Total Indirect Stage Efficiency for the Dry stage which
        //   is a 2 step process the first being the pad efficiency and then the HX Effectiveness.  I think that
        //   this would mainly be used for evap sizing purposes.
        thisEvapCond.StageEff = SatEff * EffHX;
        //***************************************************************************
        //                  CALCULATE THE WET BULB TEMP in the primary system air USING PSYCH ROUTINES
        // There is a constant humidity ratio across the primary side but a reduction in the dry bulb temp
        thisEvapCond.OuletWetBulbTemp =
            Psychrometrics::PsyTwbFnTdbWPb(state, thisEvapCond.OutletTemp, thisEvapCond.InletHumRat, state.dataEnvrn->OutBaroPress);
        //***************************************************************************
        //   TEMP LEAVING DRY BULB IS CALCULATED FROM SATURATION EFFICIENCY AS THE
        //   DRY BULB TEMP APPROACHES THE WET BULB TEMP. WET BULB TEMP IS CONSTANT
        //   ACROSS A DIRECT EVAPORATION COOLER.

        thisEvapCond.OutletHumRat = thisEvapCond.InletHumRat;

        thisEvapCond.OutletEnthalpy = Psychrometrics::PsyHFnTdbW(thisEvapCond.OutletTemp, thisEvapCond.OutletHumRat);

        //***************************************************************************
        //                  POWER OF THE SECONDARY AIR FAN
        if (thisEvapCond.IndirectFanEff > 0.0) {
            thisEvapCond.EvapCoolerPower +=
                PartLoadRatio * thisEvapCond.IndirectFanDeltaPress * thisEvapCond.IndirectVolFlowRate / thisEvapCond.IndirectFanEff;
        }

        //                  ENERGY CONSUMED BY THE RECIRCULATING PUMP
        //                  ENERGY CONSUMED BY THE RECIRCULATING PUMP
        // Add the pump energy to the total Evap Cooler energy consumption
        thisEvapCond.EvapCoolerPower += PartLoadRatio * thisEvapCond.IndirectRecircPumpPower;

        //******************
        //             WATER CONSUMPTION IN m3 OF WATER FOR DIRECT
        //             H2O [m3/s] = Delta W[kgWater/kgDryAir]*Mass Flow Air[kgDryAir/s]
        //                                /RhoWater [kgWater/m3]
        //******************
        RhoWater = Psychrometrics::RhoH2O(TDBSec);
        RhoAir = (Psychrometrics::PsyRhoAirFnPbTdbW(state, thisEvapCond.SecInletPressure, thisEvapCond.SecInletTemp, thisEvapCond.SecInletHumRat) +
                  Psychrometrics::PsyRhoAirFnPbTdbW(state, thisEvapCond.SecInletPressure, TDBSec, HumRatSec)) /
                 2.0;
        thisEvapCond.EvapWaterConsumpRate =
            PartLoadRatio * (HumRatSec - thisEvapCond.SecInletHumRat) * thisEvapCond.IndirectVolFlowRate * RhoAir / RhoWater;
        // A numerical check to keep from having very tiny negative water consumption values being reported
        if (thisEvapCond.EvapWaterConsumpRate < 0.0) {
            thisEvapCond.EvapWaterConsumpRate = 0.0;
        }

    } else {
        // The evap cooler is not running and does not change conditions from inlet to outlet
        thisEvapCond.OutletTemp = thisEvapCond.InletTemp;

        thisEvapCond.OuletWetBulbTemp = thisEvapCond.InletWetBulbTemp;

        thisEvapCond.OutletHumRat = thisEvapCond.InletHumRat;

        thisEvapCond.OutletEnthalpy = thisEvapCond.InletEnthalpy;

        thisEvapCond.EvapCoolerEnergy = 0.0;

        thisEvapCond.EvapWaterConsumpRate = 0.0;
    }
    // all of the mass flowrates are not changed across the evap cooler
    thisEvapCond.OutletMassFlowRate = thisEvapCond.InletMassFlowRate;
    thisEvapCond.OutletMassFlowRateMaxAvail = thisEvapCond.InletMassFlowRateMaxAvail;
    thisEvapCond.OutletMassFlowRateMinAvail = thisEvapCond.InletMassFlowRateMinAvail;

    // the pressure is not changed across the evap cooler
    thisEvapCond.OutletPressure = thisEvapCond.InletPressure;
}

void CalcWetIndirectEvapCooler(EnergyPlusData &state, int EvapCoolNum, Real64 const PartLoadRatio)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   October 2000
    //       RE-ENGINEERED  Jan. 2017, Rongpeng Zhang, added fouling fault for evaporative coolers

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 StageEff; // Stage Efficiency of the Heat Exchanger
    Real64 TEDB;     // Entering Dry Bulb Temperature
    Real64 TEWB;     // Entering Wet Bulb Temperature
    Real64 QHX;      // Q Across Sec HX in Watts or J/sec
    Real64 RhoWater;
    Real64 RhoAir; // Density of the primary side air
    Real64 CFMAir;
    Real64 CFMSec;
    Real64 TWBSec; // wet bulb of secondary air

    auto &thisEvapCond(state.dataEvapCoolers->EvapCond(EvapCoolNum));

    // If the Evaporative Cooler  is operating there should be some mass flow rate
    //  Also the evap cooler has to be scheduled to be available
    if ((thisEvapCond.InletMassFlowRate > 0.0) && (thisEvapCond.availSched->getCurrentVal() > 0.0)) {

        //******************************************************************************
        //   THIS SUBROUTINE WILL CACULATE THE TEMPERATURE OF THE LEAVING AIR DRY BULB
        //   FOR A WET COIL EVAPORATIVE COOLER
        //******************************************************************************
        //  INDIRECT STAGE EFFICIENCY FOR WET COIL INDIRECT EVAP COOLERS
        CFMAir = thisEvapCond.VolFlowRate;         // Volume Flow Rate Primary Side
        CFMSec = thisEvapCond.IndirectVolFlowRate; // Volume Flow Rate Secondary Side

        StageEff = thisEvapCond.WetCoilMaxEfficiency - min(thisEvapCond.WetCoilFlowRatio * CFMAir / CFMSec, thisEvapCond.WetCoilMaxEfficiency);

        if (StageEff >= 1.0) {
            StageEff = 1.0;
        }
        // This is a rough approximation of the Total Indirect Stage Efficiency.  I think that
        //   this would mainly be used for evap sizing purposes.

        // If there is a fault of fouling
        if (thisEvapCond.FaultyEvapCoolerFoulingFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) &&
            (!state.dataGlobal->KickOffSimulation)) {
            int FaultIndex = thisEvapCond.FaultyEvapCoolerFoulingIndex;
            Real64 StageEff_ff = StageEff;

            // calculate the Faulty Evaporative Cooler Fouling Factor using fault information
            thisEvapCond.FaultyEvapCoolerFoulingFactor = state.dataFaultsMgr->FaultsEvapCoolerFouling(FaultIndex).CalFoulingFactor(state);

            // update the StageEff at faulty cases
            StageEff = StageEff_ff * thisEvapCond.FaultyEvapCoolerFoulingFactor;
        }

        thisEvapCond.StageEff = StageEff;
        //***************************************************************************
        //   TEMP LEAVING DRY BULB IS CALCULATED FROM A SIMPLE WET BULB APPROACH
        //   MODEL GIVEN THE INDIRECT STAGE EFFICIENCY.
        //   DRY BULB TEMP APPROACHES THE WET BULB TEMP ACROSS THE INDIRECT STAGE.
        //***************************************************************************
        //                  CALCULATE THE TLDB
        TEWB = thisEvapCond.InletWetBulbTemp;
        TEDB = thisEvapCond.InletTemp;
        TWBSec = Psychrometrics::PsyTwbFnTdbWPb(state, thisEvapCond.SecInletTemp, thisEvapCond.SecInletHumRat, thisEvapCond.SecInletPressure);
        thisEvapCond.OutletTemp = TEDB - StageEff * (TEDB - TWBSec);

        //***************************************************************************
        //                  CALCULATE THE WET BULB TEMP in the primary system air using PSYCH ROUTINES
        // There is a constant humidity ratio across the primary side but a reduction in the dry bulb temp
        thisEvapCond.OuletWetBulbTemp =
            Psychrometrics::PsyTwbFnTdbWPb(state, thisEvapCond.OutletTemp, thisEvapCond.InletHumRat, state.dataEnvrn->OutBaroPress);
        //***************************************************************************
        //                  CALCULATE other outlet properties using PSYCH ROUTINES
        thisEvapCond.OutletHumRat = thisEvapCond.InletHumRat;

        thisEvapCond.OutletEnthalpy = Psychrometrics::PsyHFnTdbW(thisEvapCond.OutletTemp, thisEvapCond.OutletHumRat);

        //***************************************************************************
        // Real64 FlowFraction = 1.0;
        // Real64 MassFlowRateMax = Node(thisEvapCond.InletNode).MassFlowRateMax;
        // if (MassFlowRateMax > 0) {
        //    FlowFraction = thisEvapCond.InletMassFlowRate / MassFlowRateMax;
        //}
        //                  POWER OF THE SECONDARY AIR FAN
        if (thisEvapCond.IndirectFanEff > 0.0) {
            thisEvapCond.EvapCoolerPower +=
                PartLoadRatio * thisEvapCond.IndirectFanDeltaPress * thisEvapCond.IndirectVolFlowRate / thisEvapCond.IndirectFanEff;
        }

        //                  ENERGY CONSUMED BY THE RECIRCULATING PUMP
        //                  ENERGY CONSUMED BY THE RECIRCULATING PUMP
        // Add the pump energy to the total Evap Cooler energy consumption
        thisEvapCond.EvapCoolerPower += PartLoadRatio * thisEvapCond.IndirectRecircPumpPower;

        //******************
        //             WATER CONSUMPTION IN m3 OF WATER FOR Wet InDIRECT
        //             H2O [m3/s] = (QHX [J/s])/(2,500,000 [J/kgWater] * RhoWater [kgWater/m3])
        //******************
        //***** FIRST calculate the heat exchange on the primary air side**********
        RhoAir = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, thisEvapCond.InletTemp, thisEvapCond.InletHumRat);
        QHX = PartLoadRatio * CFMAir * RhoAir * (thisEvapCond.InletEnthalpy - thisEvapCond.OutletEnthalpy);

        RhoWater = Psychrometrics::RhoH2O(thisEvapCond.SecInletTemp);
        thisEvapCond.EvapWaterConsumpRate = (QHX / StageEff) / (2500000.0 * RhoWater);
        // A numerical check to keep from having very tiny negative water consumption values being reported
        if (thisEvapCond.EvapWaterConsumpRate < 0.0) {
            thisEvapCond.EvapWaterConsumpRate = 0.0;
        }

    } else {
        // The evap cooler is not running and does not change conditions from inlet to outlet
        thisEvapCond.OutletTemp = thisEvapCond.InletTemp;

        thisEvapCond.OuletWetBulbTemp = thisEvapCond.InletWetBulbTemp;

        thisEvapCond.OutletHumRat = thisEvapCond.InletHumRat;

        thisEvapCond.OutletEnthalpy = thisEvapCond.InletEnthalpy;

        thisEvapCond.EvapCoolerEnergy = 0.0;

        thisEvapCond.EvapWaterConsumpRate = 0.0;
    }
    // all of the mass flowrates are not changed across the evap cooler
    thisEvapCond.OutletMassFlowRate = thisEvapCond.InletMassFlowRate;
    thisEvapCond.OutletMassFlowRateMaxAvail = thisEvapCond.InletMassFlowRateMaxAvail;
    thisEvapCond.OutletMassFlowRateMinAvail = thisEvapCond.InletMassFlowRateMinAvail;

    // the pressure is not changed across the evap cooler
    thisEvapCond.OutletPressure = thisEvapCond.InletPressure;
}

void CalcResearchSpecialPartLoad(EnergyPlusData &state, int EvapCoolNum)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   July 2003

    // REFERENCES:
    // copied CalcWetIndirectEvapCooler as template for new cooler

    Real64 constexpr MinAirMassFlow(0.001);

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 FullOutput = 0.0;
    Real64 ReqOutput = 0.0;
    // Set local variables

    auto &thisEvapCond = state.dataEvapCoolers->EvapCond(EvapCoolNum);

    // Retrieve the load on the controlled zone
    int OutletNode = thisEvapCond.OutletNode;
    int InletNode = thisEvapCond.InletNode;
    int ControlNode = thisEvapCond.EvapControlNodeNum;
    Real64 DesOutTemp = thisEvapCond.DesiredOutletTemp;
    Real64 PartLoadFrac = 0.0;

    // If Evap Cooler runs with a cooling load then set PartLoadFrac on Cooling System and the Mass Flow
    if ((thisEvapCond.availSched->getCurrentVal() > 0.0) && (state.dataLoopNodes->Node(InletNode).MassFlowRate > MinAirMassFlow) &&
        (state.dataLoopNodes->Node(InletNode).Temp > state.dataLoopNodes->Node(ControlNode).TempSetPoint) &&
        (std::abs(state.dataLoopNodes->Node(InletNode).Temp - DesOutTemp) > HVAC::TempControlTol)) {

        // Get full load result, depending on model
        thisEvapCond.PartLoadFract = 1.0;
        switch (thisEvapCond.evapCoolerType) {
        case EvapCoolerType::IndirectRDDSpecial: {
            CalcIndirectResearchSpecialEvapCooler(state, EvapCoolNum);
            UpdateEvapCooler(state, EvapCoolNum);
            FullOutput = state.dataLoopNodes->Node(InletNode).MassFlowRate *
                         (Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(OutletNode).Temp, state.dataLoopNodes->Node(InletNode).HumRat) -
                          Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(InletNode).Temp, state.dataLoopNodes->Node(InletNode).HumRat));

            ReqOutput = state.dataLoopNodes->Node(InletNode).MassFlowRate *
                        (Psychrometrics::PsyHFnTdbW(thisEvapCond.DesiredOutletTemp, state.dataLoopNodes->Node(InletNode).HumRat) -
                         Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(InletNode).Temp, state.dataLoopNodes->Node(InletNode).HumRat));

            // now reinit after test call
            InitEvapCooler(state, EvapCoolNum);

        } break;
        case EvapCoolerType::DirectResearchSpecial: {
            CalcDirectResearchSpecialEvapCooler(state, EvapCoolNum);
            UpdateEvapCooler(state, EvapCoolNum);
            FullOutput = state.dataLoopNodes->Node(OutletNode).Temp - state.dataLoopNodes->Node(InletNode).Temp;
            ReqOutput = thisEvapCond.DesiredOutletTemp - state.dataLoopNodes->Node(InletNode).Temp;

            // now reinit after test call
            InitEvapCooler(state, EvapCoolNum);

        } break;
        default: {
            assert(false);
        } break;
        }

        // Since we are cooling, we expect FullOutput to be < 0 and FullOutput < NoCoolOutput
        // Check that this is the case; if not set PartLoadFrac = 0.0 (off) and return
        // Calculate the part load fraction
        if (FullOutput == 0.0) {
            FullOutput = 0.00001;
        }
        PartLoadFrac = ReqOutput / FullOutput;
        if (PartLoadFrac > 1.0) {
            PartLoadFrac = 1.0;
        } else if (PartLoadFrac < 0.0) {
            PartLoadFrac = 0.0;
        }

    } else { // No cooling
        PartLoadFrac = 0.0;

    } // End of the cooler running If block
    // Set the final results
    thisEvapCond.PartLoadFract = PartLoadFrac;
}

void CalcIndirectResearchSpecialEvapCooler(EnergyPlusData &state, int const EvapCoolNum, Real64 const FanPLR)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   July 2003
    //       RE-ENGINEERED  October 2014, B Nigusse, added dry and wet operating modes
    //                      and secondary air flow control

    // PURPOSE OF THIS SUBROUTINE:
    // Subroutine models a "special" cooler that allows high effectiveness and controls

    // REFERENCES:
    // copied CalcWetIndirectEvapCooler as template for new cooler

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 SecondaryInletDryBulbTemp;  // entering drybulb for secondary/purge side
    Real64 SecondaryInletWetBulbTemp;  // entering wet bulb for secondary/purge side
    Real64 SecondaryInletDewPointTemp; // entering dewpoint for secondary/purge side
    Real64 SecondaryInletHumRatio;     // entering humidity ratio for secondary/purge side
    Real64 StageEff;                   // Stage Efficiency of the Heat Exchanger
    Real64 TEDB;                       // Entering Dry Bulb Temperature
    Real64 TEWB;                       // Entering Wet Bulb Temperature
    Real64 QHX;                        // Q Across Sec HX in Watts or J/sec
    Real64 RhoWater;
    Real64 RhoAir; // Density of the primary side air
    Real64 CFMAir;
    Real64 BoundTemp; // temperature limit for outlet
    Real64 PartLoad;
    Real64 TotalVolFlow;
    Real64 TertMdot;
    Real64 TertHumRate;
    Real64 TertTemp;
    Real64 TertRho;
    Real64 TertVdot;
    Real64 SecVdot;
    Real64 SecRho;
    Real64 SecMdot;
    Real64 PurgeMdot;
    Real64 PurgeHumRat;
    Real64 PurgeEnthalpy;
    Real64 PurgeTemp;
    Real64 BlowDownVdot(0.0);
    Real64 DriftVdot(0.0);
    Real64 EvapVdot(0.0);

    auto &thisEvapCond(state.dataEvapCoolers->EvapCond(EvapCoolNum));

    // If the Evaporative Cooler  is operating there should be some mass flow rate
    //  Also the evap cooler has to be scheduled to be available
    if ((thisEvapCond.InletMassFlowRate > 0.0) && (thisEvapCond.availSched->getCurrentVal() > 0.0)) {

        //******************************************************************************
        //   THIS SUBROUTINE WILL CACULATE THE TEMPERATURE OF THE LEAVING AIR DRY BULB
        //   FOR A WET COIL EVAPORATIVE COOLER
        //******************************************************************************
        //  INDIRECT STAGE EFFICIENCY FOR WET COIL INDIRECT EVAP COOLERS
        CFMAir = thisEvapCond.VolFlowRate; // Volume Flow Rate Primary Side
        StageEff = thisEvapCond.WetCoilMaxEfficiency;

        // This is model is for special indirect cooler with efficiency greater than 1.0
        if (StageEff >= 1.5) {
            StageEff = 1.5;
        }

        thisEvapCond.StageEff = StageEff;

        //***********************************************
        //  Unit is allowed to mix relief air that would otherwise be exhausted outdoors for ventilation
        //  If tertiary node is set >0 then it assumed that this node is the exhaust out of the building
        //  and the remainder will be made up with outside air from the secondary node
        //*********************************************

        int TertNode = thisEvapCond.TertiaryInletNode;
        if (TertNode == 0) {
            SecondaryInletDryBulbTemp = thisEvapCond.SecInletTemp;
            SecondaryInletWetBulbTemp =
                Psychrometrics::PsyTwbFnTdbWPb(state, thisEvapCond.SecInletTemp, thisEvapCond.SecInletHumRat, state.dataEnvrn->OutBaroPress);
            SecondaryInletDewPointTemp =
                Psychrometrics::PsyTdpFnTdbTwbPb(state, thisEvapCond.SecInletTemp, SecondaryInletWetBulbTemp, state.dataEnvrn->OutBaroPress);
            SecondaryInletHumRatio = thisEvapCond.SecInletHumRat;

        } else {

            TotalVolFlow = thisEvapCond.IndirectVolFlowRate;
            TertMdot = state.dataLoopNodes->Node(TertNode).MassFlowRate;
            TertHumRate = state.dataLoopNodes->Node(TertNode).HumRat;
            TertTemp = state.dataLoopNodes->Node(TertNode).Temp;
            // is Node pressure available or better? using outdoor pressure for now
            TertRho = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, TertTemp, TertHumRate);
            TertVdot = TertMdot / TertRho;

            SecVdot = TotalVolFlow - TertVdot;

            if (SecVdot < 0.0) { // all tertiary/relief air e.g. economizer wide open
                SecVdot = 0.0;
                SecondaryInletDryBulbTemp = TertTemp;
                SecondaryInletWetBulbTemp = Psychrometrics::PsyTwbFnTdbWPb(state, TertTemp, TertHumRate, state.dataEnvrn->OutBaroPress);
                SecondaryInletDewPointTemp =
                    Psychrometrics::PsyTdpFnTdbTwbPb(state, TertTemp, SecondaryInletWetBulbTemp, state.dataEnvrn->OutBaroPress);
                SecondaryInletHumRatio = TertHumRate;
            } else {

                // First determine mass flow of OA,  in secondary
                SecRho =
                    Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, thisEvapCond.SecInletTemp, thisEvapCond.SecInletHumRat);
                SecMdot = SecRho * SecVdot;
                // Mass balance on moisture to get outlet air humidity ratio
                // this mixing takes place before wet media.
                PurgeMdot = SecMdot + TertMdot;
                PurgeHumRat = (SecMdot * thisEvapCond.SecInletHumRat + TertMdot * TertHumRate) / PurgeMdot;

                // Energy balance to get outlet air enthalpy

                PurgeEnthalpy = (SecMdot * Psychrometrics::PsyHFnTdbW(thisEvapCond.SecInletTemp, thisEvapCond.SecInletHumRat) +
                                 TertMdot * Psychrometrics::PsyHFnTdbW(TertTemp, TertHumRate)) /
                                PurgeMdot;

                // Use Enthalpy and humidity ratio to get outlet temperature from psych chart

                PurgeTemp = Psychrometrics::PsyTdbFnHW(PurgeEnthalpy, PurgeHumRat);
                SecondaryInletDryBulbTemp = PurgeTemp;
                SecondaryInletWetBulbTemp = Psychrometrics::PsyTwbFnTdbWPb(state, PurgeTemp, PurgeHumRat, state.dataEnvrn->OutBaroPress);
                SecondaryInletDewPointTemp =
                    Psychrometrics::PsyTdpFnTdbTwbPb(state, PurgeTemp, SecondaryInletWetBulbTemp, state.dataEnvrn->OutBaroPress);
                SecondaryInletHumRatio = PurgeHumRat;
            }
        }
        if (thisEvapCond.EvapCoolerOperationControlFlag) {
            // advanced mode: runs either in dry or wet depending on the entering conditions
            CalcIndirectResearchSpecialEvapCoolerAdvanced(
                state, EvapCoolNum, SecondaryInletDryBulbTemp, SecondaryInletWetBulbTemp, SecondaryInletDewPointTemp, SecondaryInletHumRatio);

        } else {

            TEWB = thisEvapCond.InletWetBulbTemp;
            TEDB = thisEvapCond.InletTemp;
            PartLoad = thisEvapCond.PartLoadFract;

            //***************************************************************************
            //   TEMP LEAVING DRY BULB IS CALCULATED FROM A SIMPLE WET BULB APPROACH
            //   MODEL GIVEN THE INDIRECT STAGE EFFICIENCY.
            //   DRY BULB TEMP APPROACHES THE WET BULB TEMP ACROSS THE INDIRECT STAGE.
            //***************************************************************************
            if (PartLoad == 1.0) {
                //                                 Tout = Tin -  (   0.7    (Tin  - Tpurge,wb,in)
                thisEvapCond.OutletTemp = TEDB - StageEff * (TEDB - SecondaryInletWetBulbTemp);
                //  now bound with secondary dewpoint.
                // unless the resulting Tout<=Tpurge,dp,in ; in which case Tout = Tin - 0.9(Tin-Tpurge,dp,in)

                BoundTemp = TEDB - thisEvapCond.DPBoundFactor * (TEDB - SecondaryInletDewPointTemp);
                if (thisEvapCond.OutletTemp < BoundTemp) {
                    thisEvapCond.OutletTemp = BoundTemp;
                    thisEvapCond.DewPointBoundFlag = 1;
                }
            } else if ((PartLoad < 1.0) && (PartLoad > 0.0)) {
                // assume perfect control Use PLF for energy consumption
                if (thisEvapCond.DesiredOutletTemp < TEDB) {
                    thisEvapCond.OutletTemp = thisEvapCond.DesiredOutletTemp;
                }
            } else {
                // part load set to zero so no cooling
                thisEvapCond.OutletTemp = thisEvapCond.InletTemp;
            }

            //***************************************************************************
            //                  POWER OF THE SECONDARY AIR FAN with part load factor applied (assumes const efficiency)
            thisEvapCond.EvapCoolerPower += thisEvapCond.IndirectVolFlowRate * thisEvapCond.FanSizingSpecificPower * PartLoad * FanPLR;

            //                  ENERGY CONSUMED BY THE RECIRCULATING PUMP
            //                  ENERGY CONSUMED BY THE RECIRCULATING PUMP
            // Add the pump energy to the total Evap Cooler energy consumption
            thisEvapCond.EvapCoolerPower += thisEvapCond.IndirectRecircPumpPower * PartLoad * FanPLR;

            //***************************************************************************
            //                  CALCULATE THE WET BULB TEMP in the primary system air using PSYCH ROUTINES
            // There is a constant humidity ratio across the primary side but a reduction in the dry bulb temp
            thisEvapCond.OuletWetBulbTemp =
                Psychrometrics::PsyTwbFnTdbWPb(state, thisEvapCond.OutletTemp, thisEvapCond.InletHumRat, state.dataEnvrn->OutBaroPress);
            //***************************************************************************
            //                  CALCULATE other outlet properties using PSYCH ROUTINES
            thisEvapCond.OutletHumRat = thisEvapCond.InletHumRat;

            thisEvapCond.OutletEnthalpy = Psychrometrics::PsyHFnTdbW(thisEvapCond.OutletTemp, thisEvapCond.OutletHumRat);
            //******************
            //             WATER CONSUMPTION IN m3 OF WATER FOR Wet InDIRECT
            //             H2O [m3/s] = (QHX [J/s])/(2,500,000 [J/kgWater] * RhoWater [kgWater/m3])
            //******************
            //***** FIRST calculate the heat exchange on the primary air side**********
            RhoAir = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, thisEvapCond.InletTemp, thisEvapCond.InletHumRat);
            QHX = CFMAir * RhoAir * (thisEvapCond.InletEnthalpy - thisEvapCond.OutletEnthalpy);

            RhoWater = Psychrometrics::RhoH2O(state.dataEnvrn->OutDryBulbTemp);
            EvapVdot = (QHX) / (2500000.0 * RhoWater);
            DriftVdot = EvapVdot * thisEvapCond.DriftFraction;
            if (thisEvapCond.BlowDownRatio > 0.0) {
                BlowDownVdot = EvapVdot / (thisEvapCond.BlowDownRatio - 1) - DriftVdot;
                if (BlowDownVdot < 0.0) {
                    BlowDownVdot = 0.0;
                }
            } else {
                BlowDownVdot = 0.0;
            }
            thisEvapCond.EvapWaterConsumpRate = EvapVdot + DriftVdot + BlowDownVdot;
            // A numerical check to keep from having very tiny negative water consumption values being reported
            if (thisEvapCond.EvapWaterConsumpRate < 0.0) {
                thisEvapCond.EvapWaterConsumpRate = 0.0;
            }
        }

    } else {
        // The evap cooler is not running and does not change conditions from inlet to outlet
        thisEvapCond.OutletTemp = thisEvapCond.InletTemp;
        thisEvapCond.OuletWetBulbTemp = thisEvapCond.InletWetBulbTemp;
        thisEvapCond.OutletHumRat = thisEvapCond.InletHumRat;
        thisEvapCond.OutletEnthalpy = thisEvapCond.InletEnthalpy;
        thisEvapCond.EvapCoolerEnergy = 0.0;
        thisEvapCond.EvapCoolerPower = 0.0;
        thisEvapCond.EvapWaterConsumpRate = 0.0;
        thisEvapCond.SecInletMassFlowRate = 0.0;
    }

    // all of the mass flowrates are not changed across the evap cooler
    thisEvapCond.OutletMassFlowRate = thisEvapCond.InletMassFlowRate;
    thisEvapCond.OutletMassFlowRateMaxAvail = thisEvapCond.InletMassFlowRateMaxAvail;
    thisEvapCond.OutletMassFlowRateMinAvail = thisEvapCond.InletMassFlowRateMinAvail;
    // set secondary air side inlet mass flow rate to the outlet node
    thisEvapCond.SecOutletMassFlowRate = thisEvapCond.SecInletMassFlowRate;
    state.dataLoopNodes->Node(thisEvapCond.SecondaryInletNode).MassFlowRate = thisEvapCond.SecInletMassFlowRate;

    // the pressure is not changed across the evap cooler
    thisEvapCond.OutletPressure = thisEvapCond.InletPressure;
}

void CalcIndirectResearchSpecialEvapCoolerAdvanced(EnergyPlusData &state,
                                                   int const EvapCoolNum,
                                                   Real64 const InletDryBulbTempSec,
                                                   Real64 const InletWetBulbTempSec,
                                                   Real64 const InletDewPointTempSec,
                                                   Real64 const InletHumRatioSec)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Nigusse
    //       DATE WRITTEN   October 2014

    // PURPOSE OF THIS SUBROUTINE:
    // Subroutine models indirect evaporative cooler with variable effectiveness for wet and dry
    // operating modes depending on entering conditions

    // SUBROUTINE PARAMETER DEFINITIONS:
    int constexpr MaxIte(500);      // Maximum number of iterations for solver
    Real64 constexpr TempTol(0.01); // convergence tolerance

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 BoundTemp; // temperature limit for outlet
    Real64 PartLoad;
    Real64 TdbOutSysWetMin;                 // system( primary ) air drybulb outlet temperature minimum based on wet coil
    Real64 TdbOutSysDryMin;                 // system (primary) air drybulb outlet temperature minimum based on dry coil
    Real64 AirMassFlowSec;                  // current secondary air mass flow rate
    Real64 AirMassFlowSecDry;               // current secondary air mass flow rate in dry mode
    Real64 AirMassFlowSecWet;               // current secondary air mass flow rate in wet mode
    Real64 FlowRatioSec;                    // secondary air flow ratio in dry and wet mode
    Real64 EvapCoolerTotalElectricPowerDry; // evaporative cooler current total electric power drawn
    Real64 EvapCoolerTotalElectricPowerWet; // evaporative cooler current total electric power drawn
    Real64 QHXLatent;                       // evaporative cooler latent heat transfer rate
    Real64 hfg;                             // latent heat of vaporization of water at the secondary air inlet condition

    Real64 QHX; // Q Across Sec HX in Watts or J/sec
    Real64 RhoWater;
    Real64 RhoAir; // Density of the primary side air
    Real64 MassFlowRateSecMin;
    Real64 BlowDownVdot(0.0);
    Real64 DriftVdot(0.0);
    Real64 EvapVdot(0.0);

    auto &thisEvapCond(state.dataEvapCoolers->EvapCond(EvapCoolNum));

    Real64 FlowRatioSecDry = 0.0; // current secondary air mass flow ratio in dry mode
    Real64 FlowRatioSecWet = 0.0; // current secondary air mass flow ratio in wet mode
    thisEvapCond.EvapCoolerRDDOperatingMode = OperatingMode::None;
    Real64 TEDB = thisEvapCond.InletTemp;                    // Entering Dry Bulb Temperature
    Real64 SysTempSetPoint = thisEvapCond.DesiredOutletTemp; // evaporative cooler outlet setpoint temperature, drybulb
    Real64 SecRho = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, InletDryBulbTempSec, InletHumRatioSec);
    Real64 MassFlowRateSecMax = SecRho * thisEvapCond.IndirectVolFlowRate; // Design secondary air mass flow rate
    CalcIndirectRDDEvapCoolerOutletTemp(
        state, EvapCoolNum, OperatingMode::WetFull, MassFlowRateSecMax, InletDryBulbTempSec, InletWetBulbTempSec, InletHumRatioSec);
    TdbOutSysWetMin = thisEvapCond.OutletTemp;
    CalcIndirectRDDEvapCoolerOutletTemp(
        state, EvapCoolNum, OperatingMode::DryFull, MassFlowRateSecMax, InletDryBulbTempSec, InletWetBulbTempSec, InletHumRatioSec);
    TdbOutSysDryMin = thisEvapCond.OutletTemp;

    // get current operating modes of indirect evaporative cooler research special
    thisEvapCond.EvapCoolerRDDOperatingMode = IndirectResearchSpecialEvapCoolerOperatingMode(
        state, EvapCoolNum, InletDryBulbTempSec, InletWetBulbTempSec, TdbOutSysWetMin, TdbOutSysDryMin);

    MassFlowRateSecMin = 0.0;
    AirMassFlowSec = MassFlowRateSecMax;
    PartLoad = thisEvapCond.PartLoadFract;
    {
        if (thisEvapCond.EvapCoolerRDDOperatingMode == OperatingMode::DryModulated) {
            auto f = [&state, EvapCoolNum, SysTempSetPoint, InletDryBulbTempSec, InletWetBulbTempSec, InletHumRatioSec](Real64 AirMassFlowSec) {
                auto &EvapCond(state.dataEvapCoolers->EvapCond(EvapCoolNum));
                EvapCond.SecInletMassFlowRate = AirMassFlowSec;
                CalcIndirectRDDEvapCoolerOutletTemp(
                    state, EvapCoolNum, OperatingMode::DryModulated, AirMassFlowSec, InletDryBulbTempSec, InletWetBulbTempSec, InletHumRatioSec);
                Real64 const OutletAirTemp = EvapCond.OutletTemp; // evap Cooler outlet air temperature
                return SysTempSetPoint - OutletAirTemp;
            };
            int SolFla = 0; // Flag of solver
            General::SolveRoot(state, TempTol, MaxIte, SolFla, AirMassFlowSec, f, MassFlowRateSecMin, MassFlowRateSecMax);
            // if the numerical inversion failed, issue error messages.
            if (SolFla == -1) {
                if (!state.dataGlobal->WarmupFlag) {
                    if (thisEvapCond.IterationLimit == 0) {
                        ShowSevereError(state,
                                        format("CalcIndirectResearchSpecialEvapCooler: calculate secondary air mass flow failed for Indirect "
                                               "Evaporative Cooler Research Special = {}",
                                               thisEvapCond.Name));
                        ShowContinueErrorTimeStamp(state, "");
                        ShowContinueError(state, format("  Iteration limit [{}] exceeded in calculating secondary air mass flow rate", MaxIte));
                        ShowContinueError(state, "  Simulation continues");
                    }
                    ShowRecurringWarningErrorAtEnd(
                        state,
                        "Secondary air mass flow Iteration limit exceeded in Indirect Evaporative Cooler Research Special = " + thisEvapCond.Name,
                        thisEvapCond.IterationLimit);
                }
            } else if (SolFla == -2) {
                if (!state.dataGlobal->WarmupFlag) {
                    if (thisEvapCond.IterationFailed == 0) {
                        ShowSevereError(state,
                                        format("CalcIndirectResearchSpecialEvapCooler: calculate secondary air mass flow failed for Indirect "
                                               "Evaporative Cooler Research Special = {}",
                                               thisEvapCond.Name));
                        ShowContinueErrorTimeStamp(state, "");
                        ShowContinueError(state, "...Bad secondary air mass flow rate limits");
                        ShowContinueError(state, format("...Given minimum secondary air mass flow rate={:.3R} kg/s", MassFlowRateSecMin));
                        ShowContinueError(state, format("...Given maximum secondary air mass flow rate={:.3R} kg/s", MassFlowRateSecMax));
                        ShowContinueError(state, " Simulation continues");
                    }
                    ShowRecurringWarningErrorAtEnd(state,
                                                   "Secondary air mass flow control failed in Indirect Evaporative Cooler Research Special = " +
                                                       thisEvapCond.Name,
                                                   thisEvapCond.IterationFailed);
                }
            }
            thisEvapCond.SecInletMassFlowRate = AirMassFlowSec;
            if (AirMassFlowSec > 0.0) {
                if (MassFlowRateSecMax > 0.0) {
                    FlowRatioSec = AirMassFlowSec / MassFlowRateSecMax;
                } else {
                    FlowRatioSec = 0.0;
                }
            } else {
                FlowRatioSec = 0.0;
            }
            thisEvapCond.EvapCoolerPower = IndEvapCoolerPower(state, EvapCoolNum, OperatingMode::DryModulated, FlowRatioSec);
            thisEvapCond.IECOperatingStatus = 1;
        } else if (thisEvapCond.EvapCoolerRDDOperatingMode == OperatingMode::DryFull) {
            CalcIndirectRDDEvapCoolerOutletTemp(
                state, EvapCoolNum, OperatingMode::DryFull, MassFlowRateSecMax, InletDryBulbTempSec, InletWetBulbTempSec, InletHumRatioSec);
            thisEvapCond.SecInletMassFlowRate = MassFlowRateSecMax;
            FlowRatioSec = 1.0;
            thisEvapCond.EvapCoolerPower = IndEvapCoolerPower(state, EvapCoolNum, OperatingMode::DryFull, FlowRatioSec);
            thisEvapCond.IECOperatingStatus = 1;
        } else if (thisEvapCond.EvapCoolerRDDOperatingMode == OperatingMode::DryWetModulated) {
            auto f = [&state, EvapCoolNum, SysTempSetPoint, InletDryBulbTempSec, InletWetBulbTempSec, InletHumRatioSec](Real64 AirMassFlowSec) {
                auto &EvapCond(state.dataEvapCoolers->EvapCond(EvapCoolNum));
                EvapCond.SecInletMassFlowRate = AirMassFlowSec;
                CalcIndirectRDDEvapCoolerOutletTemp(
                    state, EvapCoolNum, OperatingMode::DryModulated, AirMassFlowSec, InletDryBulbTempSec, InletWetBulbTempSec, InletHumRatioSec);
                Real64 const OutletAirTemp = EvapCond.OutletTemp; // evap Cooler outlet air temperature
                return SysTempSetPoint - OutletAirTemp;
            };
            int SolFla = 0; // Flag of solver
            General::SolveRoot(state, TempTol, MaxIte, SolFla, AirMassFlowSec, f, MassFlowRateSecMin, MassFlowRateSecMax);
            // if the numerical inversion failed, issue error messages.
            if (SolFla == -1) {
                if (!state.dataGlobal->WarmupFlag) {
                    if (thisEvapCond.IterationLimit == 0) {
                        ShowSevereError(state,
                                        format("CalcIndirectResearchSpecialEvapCooler: calculate secondary air mass flow failed for Indirect "
                                               "Evaporative Cooler Research Special = {}",
                                               thisEvapCond.Name));
                        ShowContinueErrorTimeStamp(state, "");
                        ShowContinueError(state, format("  Iteration limit [{}] exceeded in calculating secondary air mass flow rate", MaxIte));
                        ShowContinueError(state, "  Simulation continues");
                    }
                    ShowRecurringWarningErrorAtEnd(
                        state,
                        "Secondary air mass flow Iteration limit exceeded in Indirect Evaporative Cooler Research Special = " + thisEvapCond.Name,
                        thisEvapCond.IterationLimit);
                }
            } else if (SolFla == -2) {
                if (!state.dataGlobal->WarmupFlag) {
                    if (thisEvapCond.IterationFailed == 0) {
                        ShowSevereError(state,
                                        format("CalcIndirectResearchSpecialEvapCooler: calculate secondary air mass flow failed for Indirect "
                                               "Evaporative Cooler Research Special = {}",
                                               thisEvapCond.Name));
                        ShowContinueErrorTimeStamp(state, "");
                        ShowContinueError(state, "...Bad secondary air mass flow rate limits");
                        ShowContinueError(state, format("...Given minimum secondary air mass flow rate={:.3R} kg/s", MassFlowRateSecMin));
                        ShowContinueError(state, format("...Given maximum secondary air mass flow rate={:.3R} kg/s", MassFlowRateSecMax));
                        ShowContinueError(state, " Simulation continues");
                    }
                    ShowRecurringWarningErrorAtEnd(state,
                                                   "Secondary air mass flow control failed in Indirect Evaporative Cooler Research Special = " +
                                                       thisEvapCond.Name,
                                                   thisEvapCond.IterationFailed);
                }
            }
            if (AirMassFlowSec > 0.0) {
                if (MassFlowRateSecMax > 0.0) {
                    FlowRatioSec = AirMassFlowSec / MassFlowRateSecMax;
                } else {
                    FlowRatioSec = 0.0;
                }
            } else {
                FlowRatioSec = 0.0;
            }
            FlowRatioSecDry = FlowRatioSec;
            AirMassFlowSecDry = AirMassFlowSec;
            EvapCoolerTotalElectricPowerDry = IndEvapCoolerPower(state, EvapCoolNum, OperatingMode::DryModulated, FlowRatioSecDry);
            // get wet operation performance
            auto f2 = [&state, EvapCoolNum, SysTempSetPoint, InletDryBulbTempSec, InletWetBulbTempSec, InletHumRatioSec](Real64 AirMassFlowSec) {
                auto &EvapCond(state.dataEvapCoolers->EvapCond(EvapCoolNum));
                EvapCond.SecInletMassFlowRate = AirMassFlowSec;
                CalcIndirectRDDEvapCoolerOutletTemp(
                    state, EvapCoolNum, OperatingMode::WetModulated, AirMassFlowSec, InletDryBulbTempSec, InletWetBulbTempSec, InletHumRatioSec);
                Real64 const OutletAirTemp = EvapCond.OutletTemp; // evap Cooler outlet air temperature
                return SysTempSetPoint - OutletAirTemp;
            };
            General::SolveRoot(state, TempTol, MaxIte, SolFla, AirMassFlowSec, f2, MassFlowRateSecMin, MassFlowRateSecMax);
            // if the numerical inversion failed, issue error messages.
            if (SolFla == -1) {
                if (!state.dataGlobal->WarmupFlag) {
                    if (thisEvapCond.IterationLimit == 0) {
                        ShowSevereError(state,
                                        format("CalcIndirectResearchSpecialEvapCooler: calculate secondary air mass flow failed for Indirect "
                                               "Evaporative Cooler Research Special = {}",
                                               thisEvapCond.Name));
                        ShowContinueErrorTimeStamp(state, "");
                        ShowContinueError(state, format("  Iteration limit [{}] exceeded in calculating secondary air mass flow rate", MaxIte));
                        ShowContinueError(state, "  Simulation continues");
                    }
                    ShowRecurringWarningErrorAtEnd(
                        state,
                        "Secondary air mass flow Iteration limit exceeded in Indirect Evaporative Cooler Research Special = " + thisEvapCond.Name,
                        thisEvapCond.IterationLimit);
                }
            } else if (SolFla == -2) {
                if (!state.dataGlobal->WarmupFlag) {
                    if (thisEvapCond.IterationFailed == 0) {
                        ShowSevereError(state,
                                        format("CalcIndirectResearchSpecialEvapCooler: calculate secondary air mass flow failed for Indirect "
                                               "Evaporative Cooler Research Special = {}",
                                               thisEvapCond.Name));
                        ShowContinueErrorTimeStamp(state, "");
                        ShowContinueError(state, "...Bad secondary air mass flow rate limits");
                        ShowContinueError(state, format("...Given minimum secondary air mass flow rate={:.3R} kg/s", MassFlowRateSecMin));
                        ShowContinueError(state, format("...Given maximum secondary air mass flow rate={:.3R} kg/s", MassFlowRateSecMax));
                        ShowContinueError(state, " Simulation continues");
                    }
                    ShowRecurringWarningErrorAtEnd(state,
                                                   "Secondary air mass flow control failed in Indirect Evaporative Cooler Research Special = " +
                                                       thisEvapCond.Name,
                                                   thisEvapCond.IterationFailed);
                }
            }
            if (AirMassFlowSec > 0.0) {
                if (MassFlowRateSecMax > 0.0) {
                    FlowRatioSec = AirMassFlowSec / MassFlowRateSecMax;
                } else {
                    FlowRatioSec = 0.0;
                }
            } else {
                FlowRatioSec = 0.0;
            }
            FlowRatioSecWet = FlowRatioSec;
            AirMassFlowSecWet = AirMassFlowSec;
            EvapCoolerTotalElectricPowerWet = IndEvapCoolerPower(state, EvapCoolNum, OperatingMode::WetModulated, FlowRatioSecWet);
            // compare the dry and wet operation total electric power
            if (EvapCoolerTotalElectricPowerDry < EvapCoolerTotalElectricPowerWet) {
                thisEvapCond.EvapCoolerRDDOperatingMode = OperatingMode::DryModulated;
                FlowRatioSec = FlowRatioSecDry;
                thisEvapCond.SecInletMassFlowRate = AirMassFlowSecDry;
                CalcIndirectRDDEvapCoolerOutletTemp(
                    state, EvapCoolNum, OperatingMode::DryModulated, AirMassFlowSecDry, InletDryBulbTempSec, InletWetBulbTempSec, InletHumRatioSec);
                thisEvapCond.EvapCoolerPower = IndEvapCoolerPower(state, EvapCoolNum, OperatingMode::DryModulated, FlowRatioSec);
                thisEvapCond.IECOperatingStatus = 1;
            } else {
                thisEvapCond.EvapCoolerRDDOperatingMode = OperatingMode::WetModulated;
                FlowRatioSec = FlowRatioSecWet;
                thisEvapCond.SecInletMassFlowRate = AirMassFlowSecWet;
                CalcIndirectRDDEvapCoolerOutletTemp(
                    state, EvapCoolNum, OperatingMode::WetModulated, AirMassFlowSecWet, InletDryBulbTempSec, InletWetBulbTempSec, InletHumRatioSec);
                thisEvapCond.EvapCoolerPower = IndEvapCoolerPower(state, EvapCoolNum, OperatingMode::WetModulated, FlowRatioSec);
                thisEvapCond.IECOperatingStatus = 2;
            }
        } else if (thisEvapCond.EvapCoolerRDDOperatingMode == OperatingMode::WetModulated) {
            auto f = [&state, EvapCoolNum, SysTempSetPoint, InletDryBulbTempSec, InletWetBulbTempSec, InletHumRatioSec](Real64 AirMassFlowSec) {
                auto &EvapCond(state.dataEvapCoolers->EvapCond(EvapCoolNum));
                EvapCond.SecInletMassFlowRate = AirMassFlowSec;
                CalcIndirectRDDEvapCoolerOutletTemp(
                    state, EvapCoolNum, OperatingMode::WetModulated, AirMassFlowSec, InletDryBulbTempSec, InletWetBulbTempSec, InletHumRatioSec);
                Real64 const OutletAirTemp = EvapCond.OutletTemp; // evap Cooler outlet air temperature
                return SysTempSetPoint - OutletAirTemp;
            };
            int SolFla = 0; // Flag of solver
            General::SolveRoot(state, TempTol, MaxIte, SolFla, AirMassFlowSec, f, MassFlowRateSecMin, MassFlowRateSecMax);
            // if the numerical inversion failed, issue error messages.
            if (SolFla == -1) {
                if (!state.dataGlobal->WarmupFlag) {
                    if (thisEvapCond.IterationLimit == 0) {
                        ShowSevereError(state,
                                        format("CalcIndirectResearchSpecialEvapCooler: calculate secondary air mass flow failed for Indirect "
                                               "Evaporative Cooler Research Special = {}",
                                               thisEvapCond.Name));
                        ShowContinueErrorTimeStamp(state, "");
                        ShowContinueError(state, format("  Iteration limit [{}] exceeded in calculating secondary air mass flow rate", MaxIte));
                        ShowContinueError(state, "  Simulation continues");
                    }
                    ShowRecurringWarningErrorAtEnd(
                        state,
                        "Secondary air mass flow Iteration limit exceeded in Indirect Evaporative Cooler Research Special = " + thisEvapCond.Name,
                        thisEvapCond.IterationLimit);
                }
            } else if (SolFla == -2) {
                if (!state.dataGlobal->WarmupFlag) {
                    if (thisEvapCond.IterationFailed == 0) {
                        ShowSevereError(state,
                                        format("CalcIndirectResearchSpecialEvapCooler: calculate secondary air mass flow failed for Indirect "
                                               "Evaporative Cooler Research Special = {}",
                                               thisEvapCond.Name));
                        ShowContinueErrorTimeStamp(state, "");
                        ShowContinueError(state, "...Bad secondary air mass flow rate limits");
                        ShowContinueError(state, format("...Given minimum secondary air mass flow rate={:.3R} kg/s", MassFlowRateSecMin));
                        ShowContinueError(state, format("...Given maximum secondary air mass flow rate={:.3R} kg/s", MassFlowRateSecMax));
                        ShowContinueError(state, " Simulation continues");
                    }
                    ShowRecurringWarningErrorAtEnd(state,
                                                   "Secondary air mass flow control failed in Indirect Evaporative Cooler Research Special = " +
                                                       thisEvapCond.Name,
                                                   thisEvapCond.IterationFailed);
                }
            }
            thisEvapCond.SecInletMassFlowRate = AirMassFlowSec;
            if (AirMassFlowSec > 0.0) {
                if (MassFlowRateSecMax > 0.0) {
                    FlowRatioSec = AirMassFlowSec / MassFlowRateSecMax;
                } else {
                    FlowRatioSec = 0.0;
                }
            } else {
                FlowRatioSec = 0.0;
            }
            thisEvapCond.EvapCoolerPower = IndEvapCoolerPower(state, EvapCoolNum, OperatingMode::WetModulated, FlowRatioSec);
            thisEvapCond.IECOperatingStatus = 2;
        } else if (thisEvapCond.EvapCoolerRDDOperatingMode == OperatingMode::WetFull) {
            CalcIndirectRDDEvapCoolerOutletTemp(
                state, EvapCoolNum, OperatingMode::WetFull, MassFlowRateSecMax, InletDryBulbTempSec, InletWetBulbTempSec, InletHumRatioSec);
            thisEvapCond.SecInletMassFlowRate = MassFlowRateSecMax;
            FlowRatioSec = 1.0;
            thisEvapCond.EvapCoolerPower = IndEvapCoolerPower(state, EvapCoolNum, OperatingMode::WetFull, FlowRatioSec);
            thisEvapCond.IECOperatingStatus = 2;
        }
    }
    if (PartLoad == 1.0) {
        if (thisEvapCond.EvapCoolerRDDOperatingMode == OperatingMode::WetModulated ||
            thisEvapCond.EvapCoolerRDDOperatingMode == OperatingMode::WetFull) {
            BoundTemp = TEDB - thisEvapCond.DPBoundFactor * (TEDB - InletDewPointTempSec);
            if (thisEvapCond.OutletTemp < BoundTemp) {
                thisEvapCond.OutletTemp = BoundTemp;
                thisEvapCond.DewPointBoundFlag = 1;
            }
        }
    } else if ((PartLoad < 1.0) && (PartLoad > 0.0)) {
        // assume perfect control Use PLF for energy consumption
        if (thisEvapCond.DesiredOutletTemp < TEDB) {
            thisEvapCond.OutletTemp = thisEvapCond.DesiredOutletTemp;
        }
    } else {
        // part load set to zero so no cooling
        thisEvapCond.OutletTemp = thisEvapCond.InletTemp;
    }
    if (thisEvapCond.EvapCoolerRDDOperatingMode != OperatingMode::None) {
        // There is a constant humidity ratio across the primary side but a reduction in the dry bulb temp
        thisEvapCond.OuletWetBulbTemp =
            Psychrometrics::PsyTwbFnTdbWPb(state, thisEvapCond.OutletTemp, thisEvapCond.InletHumRat, state.dataEnvrn->OutBaroPress);
        thisEvapCond.OutletHumRat = thisEvapCond.InletHumRat;
        thisEvapCond.OutletEnthalpy = Psychrometrics::PsyHFnTdbW(thisEvapCond.OutletTemp, thisEvapCond.OutletHumRat);
        RhoAir = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, thisEvapCond.InletTemp, thisEvapCond.InletHumRat);
        QHX = thisEvapCond.VolFlowRate * RhoAir * (thisEvapCond.InletEnthalpy - thisEvapCond.OutletEnthalpy);
        if (QHX > HVAC::SmallLoad) {
            // get secondary air outlet condition
            CalcSecondaryAirOutletCondition(state,
                                            EvapCoolNum,
                                            thisEvapCond.EvapCoolerRDDOperatingMode,
                                            thisEvapCond.SecInletMassFlowRate,
                                            InletDryBulbTempSec,
                                            InletWetBulbTempSec,
                                            InletHumRatioSec,
                                            QHX,
                                            QHXLatent);
            RhoWater = Psychrometrics::RhoH2O(state.dataEnvrn->OutDryBulbTemp); // this if it is at the outside air inlet node condition
            hfg = Psychrometrics::PsyHfgAirFnWTdb(InletHumRatioSec, InletDryBulbTempSec);
            EvapVdot = (QHXLatent) / (hfg * RhoWater);
            DriftVdot = EvapVdot * thisEvapCond.DriftFraction;
            if (thisEvapCond.BlowDownRatio > 0.0) {
                BlowDownVdot = EvapVdot / (thisEvapCond.BlowDownRatio - 1) - DriftVdot;
                if (BlowDownVdot < 0.0) {
                    BlowDownVdot = 0.0;
                }
            } else {
                BlowDownVdot = 0.0;
            }
            thisEvapCond.EvapWaterConsumpRate = EvapVdot + DriftVdot + BlowDownVdot;
            // A numerical check to keep from having very tiny negative water consumption values being reported
            if (thisEvapCond.EvapWaterConsumpRate < 0.0) {
                thisEvapCond.EvapWaterConsumpRate = 0.0;
            }
        } else {
            thisEvapCond.OutletTemp = thisEvapCond.InletTemp;
            thisEvapCond.OuletWetBulbTemp = thisEvapCond.InletWetBulbTemp;
            thisEvapCond.OutletEnthalpy = thisEvapCond.InletEnthalpy;
            thisEvapCond.EvapCoolerEnergy = 0.0;
            thisEvapCond.EvapCoolerPower = 0.0;
            thisEvapCond.EvapWaterConsumpRate = 0.0;
            thisEvapCond.SecInletMassFlowRate = 0.0;
            thisEvapCond.IECOperatingStatus = 0;
            thisEvapCond.StageEff = 0.0;
            CalcSecondaryAirOutletCondition(state,
                                            EvapCoolNum,
                                            thisEvapCond.EvapCoolerRDDOperatingMode,
                                            0.0,
                                            InletDryBulbTempSec,
                                            InletWetBulbTempSec,
                                            InletHumRatioSec,
                                            QHX,
                                            QHXLatent);
        }

    } else {
        // The evap cooler is not running and does not change conditions from inlet to outlet
        thisEvapCond.OutletTemp = thisEvapCond.InletTemp;
        thisEvapCond.OuletWetBulbTemp = thisEvapCond.InletWetBulbTemp;
        thisEvapCond.OutletHumRat = thisEvapCond.InletHumRat;
        thisEvapCond.OutletEnthalpy = thisEvapCond.InletEnthalpy;
        thisEvapCond.SecOutletTemp = thisEvapCond.SecInletTemp;
        thisEvapCond.SecOutletHumRat = thisEvapCond.SecInletHumRat;
        thisEvapCond.SecOutletEnthalpy = thisEvapCond.SecInletEnthalpy;
        thisEvapCond.SecOutletMassFlowRate = thisEvapCond.SecInletMassFlowRate;
        thisEvapCond.EvapCoolerEnergy = 0.0;
        thisEvapCond.EvapCoolerPower = 0.0;
        thisEvapCond.EvapWaterConsumpRate = 0.0;
        thisEvapCond.SecInletMassFlowRate = 0.0;
        thisEvapCond.IECOperatingStatus = 0;
        thisEvapCond.StageEff = 0.0;
    }
}

OperatingMode IndirectResearchSpecialEvapCoolerOperatingMode(EnergyPlusData &state,
                                                             int const EvapCoolNum,
                                                             Real64 const InletDryBulbTempSec,
                                                             Real64 const InletWetBulbTempSec,
                                                             Real64 const TdbOutSysWetMin,
                                                             Real64 const TdbOutSysDryMin)
{

    // PURPOSE OF THIS SUBROUTINE:
    // Determines current operating mode of indirect research special evaporative cooler
    // from the five valid operating modes depending the primary and secondary air
    // temperatures, setpoint temperature, and full capacity air outlet temperature.

    // METHODOLOGY EMPLOYED:
    // compares various temperatures to determine the operating mode

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 InletDryBulbTempPri;  // entering air dry bulb temperature of primary air
    Real64 SysTempSetPoint;      // evaporative cooler outlet setpoint temperature, drybulb
    OperatingMode OperatingMode; // current operating mode of indirect evaporative cooler

    auto const &thisEvapCond = state.dataEvapCoolers->EvapCond(EvapCoolNum);

    InletDryBulbTempPri = thisEvapCond.InletTemp;
    SysTempSetPoint = thisEvapCond.DesiredOutletTemp;

    // Now determine the operating modes of indirect evaporative cooler research special. There are five allowed operating modes
    if ((InletDryBulbTempPri <= SysTempSetPoint) ||
        (InletDryBulbTempPri > thisEvapCond.MaxOATDBEvapCooler && InletWetBulbTempSec > thisEvapCond.MaxOATWBEvapCooler) ||
        (InletDryBulbTempPri < InletDryBulbTempSec)) {
        OperatingMode = OperatingMode::None;
    } else if ((InletDryBulbTempSec < thisEvapCond.MinOATDBEvapCooler && TdbOutSysDryMin < SysTempSetPoint)) {
        OperatingMode = OperatingMode::DryModulated; // dry mode capacity modulated
    } else if ((InletDryBulbTempSec < thisEvapCond.MinOATDBEvapCooler && SysTempSetPoint <= TdbOutSysDryMin)) {
        OperatingMode = OperatingMode::DryFull; // dry mode in full capacity
    } else if ((InletDryBulbTempSec >= thisEvapCond.MinOATDBEvapCooler && InletWetBulbTempSec < thisEvapCond.MaxOATWBEvapCooler &&
                SysTempSetPoint <= TdbOutSysWetMin)) {
        OperatingMode = OperatingMode::WetFull; // wet mode in full capacity
    } else if ((InletDryBulbTempSec >= thisEvapCond.MinOATDBEvapCooler && InletWetBulbTempSec < thisEvapCond.MaxOATWBEvapCooler &&
                TdbOutSysWetMin < SysTempSetPoint)) { // && SysTempSetPoint < TdbOutSysDryMin
        OperatingMode = OperatingMode::WetModulated;  // wet mode capacity modulated
    } else if ((InletDryBulbTempSec >= thisEvapCond.MinOATDBEvapCooler && InletDryBulbTempSec < thisEvapCond.MaxOATDBEvapCooler &&
                InletWetBulbTempSec < thisEvapCond.MaxOATWBEvapCooler && SysTempSetPoint < TdbOutSysDryMin && TdbOutSysWetMin < SysTempSetPoint)) {
        OperatingMode = OperatingMode::DryWetModulated; // modulated in dry and wet mode, and the lower total power will be used
    } else {
        OperatingMode = OperatingMode::None; // this condition should not happen unless the bounds do not cover all combinations possible
    }
    return OperatingMode;
}

void CalcIndirectRDDEvapCoolerOutletTemp(EnergyPlusData &state,
                                         int const EvapCoolNum,
                                         OperatingMode DryOrWetOperatingMode,
                                         Real64 const AirMassFlowSec,
                                         Real64 const EDBTSec,
                                         Real64 const EWBTSec,
                                         Real64 const EHumRatSec)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Nigusse
    //       DATE WRITTEN   Sep 2014

    // PURPOSE OF THIS SUBROUTINE:
    // Indirect research special evaporative cooler performance:
    // determines the IEC primary air outlet temperature

    // METHODOLOGY EMPLOYED:
    // Uses effectiveness and energy balance equations to determine
    // primary air outlet temperature.  The dry and wet effectiveness
    // values are used depending on operating modes.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 OutletTemp;       // evaporative cooler current outlet air drybulb temperature
    Real64 RhoAirSec;        // density of secondary air at inlet condition
    Real64 RhoAirSys;        // density of primary air at inlet condition
    Real64 EffectivenessDry; // dry coil effectiveness
    Real64 EffectivenessWet; // wet coil effectiveness
    Real64 FlowRatio;        // flow ratio based on current to the design of secondary air flow rate
    Real64 EffModDryMode;    // dry mode effectiveness modifier for flow ratio
    Real64 EffModWetMode;    // wet mode effectiveness modifier for flow ratio
    Real64 CapFlowSys;       // capacity flow (massFlowRate * Specific Heat) of primary air system
    Real64 CapFlowSec;       // capacity flow (massFlowRate * Specific Heat) of secondary system
    Real64 CpAirSec;         // specific heat of secondary air at inlet condition
    Real64 CpAirSys;         // specific heat of primary air at inlet condition

    Real64 QHXRate;            // total heat transfer rate
    Real64 OutletTempSec;      // secondary air outlet temperature
    Real64 SecOutletAirHumRat; // secondary air humidity ratio at constant temperature (Pure mass transfer)
    Real64 SecOutletEnthalpy;  // secondary air outlet enthalpy

    auto &thisEvapCond(state.dataEvapCoolers->EvapCond(EvapCoolNum));

    if (thisEvapCond.InletMassFlowRate > 0.0) {
        FlowRatio = AirMassFlowSec / thisEvapCond.InletMassFlowRate; // ratio of current secondary air flow to current primary air flow
    } else {
        FlowRatio = 1.0;
    }
    if (AirMassFlowSec > 0.0) {
        RhoAirSec = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, EDBTSec, EHumRatSec);
        RhoAirSys = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, thisEvapCond.InletTemp, thisEvapCond.InletHumRat);
        if (DryOrWetOperatingMode == OperatingMode::DryModulated || DryOrWetOperatingMode == OperatingMode::DryFull) {
            if (thisEvapCond.DrybulbEffecCurve != nullptr) {
                EffModDryMode = thisEvapCond.DrybulbEffecCurve->value(state, FlowRatio);
            } else {
                EffModDryMode = 1.0;
            }
            EffectivenessDry = thisEvapCond.DryCoilMaxEfficiency * EffModDryMode;
            thisEvapCond.StageEff = EffectivenessDry;
            OutletTemp = thisEvapCond.InletTemp - EffectivenessDry * (thisEvapCond.InletTemp - EDBTSec);
            if (OutletTemp > thisEvapCond.InletTemp) {
                OutletTemp = thisEvapCond.InletTemp;
            }
            CpAirSys = Psychrometrics::PsyCpAirFnW(thisEvapCond.InletHumRat);
            CapFlowSys = thisEvapCond.InletMassFlowRate * CpAirSys;
            QHXRate = CapFlowSys * (thisEvapCond.InletTemp - OutletTemp);
            CpAirSec = Psychrometrics::PsyCpAirFnW(EHumRatSec);
            CapFlowSec = AirMassFlowSec * CpAirSec;
            OutletTempSec = EDBTSec + QHXRate / CapFlowSec;
            if (OutletTempSec >= thisEvapCond.InletTemp) {
                OutletTempSec = thisEvapCond.InletTemp - 0.2;
                QHXRate = CapFlowSec * (OutletTempSec - EDBTSec);
                OutletTemp = thisEvapCond.InletTemp - QHXRate / CapFlowSys;
            }
            thisEvapCond.SecOutletTemp = OutletTempSec;
        } else if (DryOrWetOperatingMode == OperatingMode::WetModulated || DryOrWetOperatingMode == OperatingMode::WetFull) {
            if (thisEvapCond.WetbulbEffecCurve != nullptr) {
                EffModWetMode = thisEvapCond.WetbulbEffecCurve->value(state, FlowRatio);
            } else {
                EffModWetMode = 1.0;
            }
            EffectivenessWet = thisEvapCond.WetCoilMaxEfficiency * EffModWetMode;
            thisEvapCond.StageEff = EffectivenessWet;
            OutletTemp = thisEvapCond.InletTemp - EffectivenessWet * (thisEvapCond.InletTemp - EWBTSec);
            if (OutletTemp > thisEvapCond.InletTemp) {
                OutletTemp = thisEvapCond.InletTemp;
            }
            CpAirSys = Psychrometrics::PsyCpAirFnW(thisEvapCond.InletHumRat);
            CapFlowSys = thisEvapCond.InletMassFlowRate * CpAirSys;
            QHXRate = CapFlowSys * (thisEvapCond.InletTemp - OutletTemp);
            SecOutletEnthalpy = thisEvapCond.SecInletEnthalpy + QHXRate / AirMassFlowSec;
            SecOutletAirHumRat = Psychrometrics::PsyWFnTdbH(state, EDBTSec,
                                                            SecOutletEnthalpy); // assumes constant temperature moisture addition
            // we may need check based on maximum allowed humidity ratio
            thisEvapCond.SecOutletTemp = EDBTSec;
            thisEvapCond.SecOutletHumRat = SecOutletAirHumRat;
            thisEvapCond.SecOutletEnthalpy = SecOutletEnthalpy;
        } else {
            OutletTemp = thisEvapCond.InletTemp;
            thisEvapCond.StageEff = 0.0;
        }
    } else {
        OutletTemp = thisEvapCond.InletTemp;
        thisEvapCond.StageEff = 0.0;
    }
    // set results to into output variables
    thisEvapCond.OutletTemp = OutletTemp;
}

void CalcSecondaryAirOutletCondition(EnergyPlusData &state,
                                     int const EvapCoolNum,
                                     OperatingMode OperatingMode,
                                     Real64 const AirMassFlowSec,
                                     Real64 const EDBTSec,
                                     Real64 const EWBTSec,
                                     Real64 const EHumRatSec,
                                     Real64 const QHXTotal,
                                     Real64 &QHXLatent)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Nigusse
    //       DATE WRITTEN   Oct 2014

    // PURPOSE OF THIS SUBROUTINE:
    // Indirect research special evaporative cooler: determines the secondary air outlet conditions

    // METHODOLOGY EMPLOYED:
    // applies energy balance equations to determine the secondary air outlet condition
    // For wt operations assumes the secondary air leaves at at inlet temperature, i.e.,
    // latent heat transfer only.  For dry operation the humidity ratio remains constant.

    // REFERENCES:
    // CalculateWaterUsage routine of cooling towers for wet operation mode

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 SecOutletAirHumRat; // secondary air humidity ratio at the outlet node
    Real64 SecOutletEnthalpy;  // secondary air outlet enthalpy
    Real64 CpAirSec;           // specific heat of secondary air at inlet condition
    Real64 hfg;                // secondary air side enthalpy of evaporation

    auto &thisEvapCond(state.dataEvapCoolers->EvapCond(EvapCoolNum));

    QHXLatent = 0.0;
    if (AirMassFlowSec > 0.0) {
        if ((OperatingMode == OperatingMode::DryModulated || OperatingMode == OperatingMode::DryFull)) {
            thisEvapCond.SecOutletHumRat = EHumRatSec;
            CpAirSec = Psychrometrics::PsyCpAirFnW(EHumRatSec);
            thisEvapCond.SecOutletTemp = EDBTSec + QHXTotal / AirMassFlowSec / CpAirSec;
            thisEvapCond.SecOutletEnthalpy = Psychrometrics::PsyHFnTdbW(thisEvapCond.SecOutletTemp, EHumRatSec);
            thisEvapCond.SecOuletWetBulbTemp =
                Psychrometrics::PsyTwbFnTdbWPb(state, thisEvapCond.SecOutletTemp, EHumRatSec, state.dataEnvrn->OutBaroPress);
        } else if ((OperatingMode == OperatingMode::WetModulated || OperatingMode == OperatingMode::WetFull)) {
            SecOutletEnthalpy = thisEvapCond.SecInletEnthalpy + QHXTotal / AirMassFlowSec;
            SecOutletAirHumRat = Psychrometrics::PsyWFnTdbH(state, EDBTSec,
                                                            SecOutletEnthalpy); // assumes a constant temperature moisture addition
            thisEvapCond.SecOutletTemp = EDBTSec;
            thisEvapCond.SecOutletHumRat = SecOutletAirHumRat;
            thisEvapCond.SecOutletEnthalpy = SecOutletEnthalpy;
            thisEvapCond.SecOuletWetBulbTemp =
                Psychrometrics::PsyTwbFnTdbWPb(state, thisEvapCond.SecOutletTemp, SecOutletAirHumRat, state.dataEnvrn->OutBaroPress);
            hfg = Psychrometrics::PsyHfgAirFnWTdb(EHumRatSec, EDBTSec);
            QHXLatent = min(QHXTotal, AirMassFlowSec * (SecOutletAirHumRat - EHumRatSec) * hfg);
        } else {
            // set results to into output variables
            thisEvapCond.SecOutletTemp = EDBTSec;
            thisEvapCond.SecOuletWetBulbTemp = EWBTSec;
            thisEvapCond.SecOutletHumRat = EHumRatSec;
            thisEvapCond.SecOutletEnthalpy = thisEvapCond.SecInletEnthalpy;
        }
    } else {
        thisEvapCond.SecOutletTemp = EDBTSec;
        thisEvapCond.SecOuletWetBulbTemp = EWBTSec;
        thisEvapCond.SecOutletHumRat = EHumRatSec;
        thisEvapCond.SecOutletEnthalpy = thisEvapCond.SecInletEnthalpy;
    }
}

Real64 IndEvapCoolerPower(EnergyPlusData &state,
                          int const EvapCoolIndex,  // Unit index
                          OperatingMode DryWetMode, // dry or wet operating mode of evaporator cooler
                          Real64 const FlowRatio    // secondary air flow fraction
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Nigusse
    //       DATE WRITTEN   Sep 2014

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates the Indirect Evaporative Cooler Total Electric Power

    // METHODOLOGY EMPLOYED:
    // Scales the design fan and pump power depending on secondary air flow fraction
    // and sums the two to determine the evaporative cooler total electric power.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 FanPowerModCurveValue;  // fan power modifier curve value
    Real64 PumpPowerModCurveValue; // fan power modifier curve value
    Real64 EvapCoolertotalPower;   // current evaporative cooler total electric power

    auto &thisEvapCond(state.dataEvapCoolers->EvapCond(EvapCoolIndex));

    EvapCoolertotalPower = 0.0;
    if (FlowRatio > 0.0) {
        if (thisEvapCond.FanPowerModifierCurve != nullptr) {
            FanPowerModCurveValue = thisEvapCond.FanPowerModifierCurve->value(state, FlowRatio);
        } else {
            FanPowerModCurveValue = thisEvapCond.PartLoadFract * FlowRatio;
        }
        EvapCoolertotalPower += thisEvapCond.IndirectFanPower * FanPowerModCurveValue;
        if (DryWetMode == OperatingMode::WetModulated || DryWetMode == OperatingMode::WetFull) {
            // Add the pump power to the total Evap Cooler power for wet operating mode
            if (thisEvapCond.PumpPowerModifierCurve != nullptr) {
                PumpPowerModCurveValue = thisEvapCond.PumpPowerModifierCurve->value(state, FlowRatio);
            } else {
                // linearly scale pump power using part-load-fraction when pump power modifier curve is not specified
                PumpPowerModCurveValue = thisEvapCond.PartLoadFract * FlowRatio;
            }
            EvapCoolertotalPower += thisEvapCond.IndirectRecircPumpPower * PumpPowerModCurveValue;
        }
    } else {
        EvapCoolertotalPower = 0.0;
    }
    return EvapCoolertotalPower;
}

void CalcDirectResearchSpecialEvapCooler(EnergyPlusData &state, int const EvapCoolNum, Real64 const FanPLR)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   March 2009

    // PURPOSE OF THIS SUBROUTINE:
    // calculate model for direct evaporative cooler that is simple and controllable

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 SatEff; // Saturation Efficiency of the CelDek Pad
    Real64 TEDB;   // Entering Dry Bulb Temperature
    Real64 TEWB;   // Entering Wet Bulb Temperature
    Real64 RhoWater;
    Real64 PartLoad;
    Real64 EffModCurveValue;       // effectiveness modifier curve value
    Real64 PumpPowerModCurveValue; // recirculation pump power modifier curve value
    Real64 FlowRatio(0);           // primary air flow frcation (current flow divided by the design flow rate)
    Real64 MassFlowRateSysDesign;  // primary air design mass flow rate
    Real64 MassFlowRateSys;        // primary air current mass flow rate
    Real64 BlowDownVdot(0.0);
    Real64 DriftVdot(0.0);
    Real64 EvapVdot(0.0);
    bool EvapCoolerOperatingLimitFlag(false);

    auto &thisEvapCond(state.dataEvapCoolers->EvapCond(EvapCoolNum));

    EvapCoolerOperatingLimitFlag = false;
    TEDB = thisEvapCond.InletTemp;
    TEWB = thisEvapCond.InletWetBulbTemp;
    if (thisEvapCond.EvapCoolerOperationControlFlag) {
        if (TEDB >= thisEvapCond.MinOATDBEvapCooler && (TEWB <= thisEvapCond.MaxOATWBEvapCooler || TEDB <= thisEvapCond.MaxOATDBEvapCooler)) {
            EvapCoolerOperatingLimitFlag = true;
        }
    } else {
        EvapCoolerOperatingLimitFlag = true;
    }

    // If the Evaporative Cooler  is operating there should be some mass flow rate
    //  Also the evap cooler has to be scheduled to be available
    if ((thisEvapCond.InletMassFlowRate > 0.0) && (thisEvapCond.availSched->getCurrentVal() > 0.0) && EvapCoolerOperatingLimitFlag) {

        //***************************************************************************
        //   TEMP LEAVING DRY BULB IS CALCULATED FROM SATURATION EFFICIENCY AS THE
        //   DRY BULB TEMP APPROACHES THE WET BULB TEMP. WET BULB TEMP IS CONSTANT
        //   ACROSS A DIRECT EVAPORATION COOLER.
        TEWB = thisEvapCond.InletWetBulbTemp;
        TEDB = thisEvapCond.InletTemp;

        MassFlowRateSys = thisEvapCond.InletMassFlowRate;
        MassFlowRateSysDesign = state.dataLoopNodes->Node(thisEvapCond.InletNode).MassFlowRateMax;
        if (MassFlowRateSysDesign > 0.0) {
            if (MassFlowRateSys > 0.0) {
                FlowRatio = MassFlowRateSys / MassFlowRateSysDesign;
            } else {
                FlowRatio = 1.0;
            }
        }
        if (thisEvapCond.WetbulbEffecCurve != nullptr) {
            EffModCurveValue = thisEvapCond.WetbulbEffecCurve->value(state, FlowRatio);
        } else {
            // if no curve specified assume constant effectiveness
            EffModCurveValue = 1.0;
        }
        SatEff = thisEvapCond.DirectEffectiveness * EffModCurveValue;
        thisEvapCond.StageEff = SatEff;
        PartLoad = thisEvapCond.PartLoadFract;
        if (PartLoad == 1.0) {
            thisEvapCond.OutletTemp = TEDB - ((TEDB - TEWB) * SatEff);
            thisEvapCond.OuletWetBulbTemp = TEWB;
            thisEvapCond.OutletHumRat = Psychrometrics::PsyWFnTdbTwbPb(state, thisEvapCond.OutletTemp, TEWB, state.dataEnvrn->OutBaroPress);
            thisEvapCond.OutletEnthalpy = Psychrometrics::PsyHFnTdbW(thisEvapCond.OutletTemp, thisEvapCond.OutletHumRat);
        } else if ((PartLoad < 1.0) && (PartLoad > 0.0)) {
            // assume perfect control Use PLF for energy consumption
            if (thisEvapCond.DesiredOutletTemp < TEDB) {
                thisEvapCond.OutletTemp = thisEvapCond.DesiredOutletTemp;
                thisEvapCond.OuletWetBulbTemp = TEWB;
                thisEvapCond.OutletHumRat = Psychrometrics::PsyWFnTdbTwbPb(state, thisEvapCond.OutletTemp, TEWB, state.dataEnvrn->OutBaroPress);

                thisEvapCond.OutletEnthalpy = Psychrometrics::PsyHFnTdbW(thisEvapCond.OutletTemp, thisEvapCond.OutletHumRat);
            } else { // do no cooling
                thisEvapCond.OutletTemp = TEDB;
                thisEvapCond.OuletWetBulbTemp = TEWB;
                thisEvapCond.OutletHumRat = Psychrometrics::PsyWFnTdbTwbPb(state, thisEvapCond.OutletTemp, TEWB, state.dataEnvrn->OutBaroPress);
                thisEvapCond.OutletEnthalpy = Psychrometrics::PsyHFnTdbW(thisEvapCond.OutletTemp, thisEvapCond.OutletHumRat);
            }
        } else {
            // part load set to zero so no cooling
            thisEvapCond.OutletTemp = TEDB;
            thisEvapCond.OuletWetBulbTemp = TEWB;
            thisEvapCond.OutletHumRat = Psychrometrics::PsyWFnTdbTwbPb(state, thisEvapCond.OutletTemp, TEWB, state.dataEnvrn->OutBaroPress);
            thisEvapCond.OutletEnthalpy = Psychrometrics::PsyHFnTdbW(thisEvapCond.OutletTemp, thisEvapCond.OutletHumRat);
        }

        //***************************************************************************
        //                  ENERGY CONSUMED BY THE RECIRCULATING PUMP
        // Add the pump energy to the total Evap Cooler energy comsumption
        if (thisEvapCond.PumpPowerModifierCurve != nullptr) {
            PumpPowerModCurveValue = thisEvapCond.PumpPowerModifierCurve->value(state, FlowRatio);
        } else {
            // if no pump power modifier curve specified, then assume linear variation with part-load and primary fan PLR
            PumpPowerModCurveValue = PartLoad * FanPLR;
        }
        thisEvapCond.EvapCoolerPower = thisEvapCond.RecircPumpPower * PumpPowerModCurveValue;
        //******************
        //             WATER CONSUMPTION IN m3 OF WATER FOR DIRECT
        //             H2O [m3/s] = Delta W[kgWater/kgDryAir]*Mass Flow Air[kgDryAir]
        //                                /RhoWater [kgWater/m3]
        //******************
        RhoWater = Psychrometrics::RhoH2O(thisEvapCond.OutletTemp);
        EvapVdot = (thisEvapCond.OutletHumRat - thisEvapCond.InletHumRat) * thisEvapCond.InletMassFlowRate / RhoWater;
        DriftVdot = EvapVdot * thisEvapCond.DriftFraction;

        if (thisEvapCond.BlowDownRatio > 0.0) {
            BlowDownVdot = EvapVdot / (thisEvapCond.BlowDownRatio - 1.0) - DriftVdot;
            if (BlowDownVdot < 0.0) {
                BlowDownVdot = 0.0;
            }
        } else {
            BlowDownVdot = 0.0;
        }

        thisEvapCond.EvapWaterConsumpRate = EvapVdot + DriftVdot + BlowDownVdot;

        // A numerical check to keep from having very tiny negative water consumption values being reported
        if (thisEvapCond.EvapWaterConsumpRate < 0.0) {
            thisEvapCond.EvapWaterConsumpRate = 0.0;
        }

    } else {
        // The evap cooler is not running and does not change conditions from inlet to outlet
        thisEvapCond.OutletTemp = thisEvapCond.InletTemp;

        thisEvapCond.OuletWetBulbTemp = thisEvapCond.InletWetBulbTemp;

        thisEvapCond.OutletHumRat = thisEvapCond.InletHumRat;

        thisEvapCond.OutletEnthalpy = thisEvapCond.InletEnthalpy;
        thisEvapCond.EvapCoolerPower = 0.0;
        thisEvapCond.EvapCoolerEnergy = 0.0;

        thisEvapCond.EvapWaterConsumpRate = 0.0;
    }
    // all of the mass flowrates are not changed across the evap cooler
    thisEvapCond.OutletMassFlowRate = thisEvapCond.InletMassFlowRate;
    thisEvapCond.OutletMassFlowRateMaxAvail = thisEvapCond.InletMassFlowRateMaxAvail;
    thisEvapCond.OutletMassFlowRateMinAvail = thisEvapCond.InletMassFlowRateMinAvail;

    // the pressure is not changed across the evap cooler
    thisEvapCond.OutletPressure = thisEvapCond.InletPressure;
}

void UpdateEvapCooler(EnergyPlusData &state, int const EvapCoolNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   October 2000

    auto &thisEvapCond = state.dataEvapCoolers->EvapCond(EvapCoolNum);
    auto &thisOutletNode = state.dataLoopNodes->Node(thisEvapCond.OutletNode);
    auto const &thisInletNode = state.dataLoopNodes->Node(thisEvapCond.InletNode);

    // Set the outlet air nodes of the EvapCooler
    thisOutletNode.MassFlowRate = thisEvapCond.OutletMassFlowRate;
    thisOutletNode.MassFlowRateMaxAvail = thisEvapCond.OutletMassFlowRateMaxAvail;
    thisOutletNode.MassFlowRateMinAvail = thisEvapCond.OutletMassFlowRateMinAvail;
    thisOutletNode.Temp = thisEvapCond.OutletTemp;
    thisOutletNode.HumRat = thisEvapCond.OutletHumRat;
    thisOutletNode.Enthalpy = thisEvapCond.OutletEnthalpy;
    thisOutletNode.Press = thisEvapCond.OutletPressure;

    if (thisEvapCond.SecondaryOutletNode > 0) {
        auto &thisOutletNodeSec = state.dataLoopNodes->Node(thisEvapCond.SecondaryOutletNode);
        // set outlet nodes of the secondary air side of the EvapCooler (mass Flow Rate Only)
        if (thisEvapCond.evapCoolerType == EvapCoolerType::IndirectRDDSpecial && thisEvapCond.EvapCoolerOperationControlFlag) {
            thisOutletNodeSec.Temp = thisEvapCond.SecOutletTemp;
            thisOutletNodeSec.HumRat = thisEvapCond.SecOutletHumRat;
            thisOutletNodeSec.Enthalpy = thisEvapCond.SecOutletEnthalpy;
            thisOutletNodeSec.MassFlowRate = thisEvapCond.SecOutletMassFlowRate;
        }
    }

    // Set the outlet nodes for properties that just pass through & not used
    thisOutletNode.Quality = thisInletNode.Quality;

    // Set the demand request for supply water from water storage tank (if needed)
    if (thisEvapCond.EvapWaterSupplyMode == WaterSupply::FromTank) {
        state.dataWaterData->WaterStorage(thisEvapCond.EvapWaterSupTankID).VdotRequestDemand(thisEvapCond.EvapWaterTankDemandARRID) =
            thisEvapCond.EvapWaterConsumpRate;
    }

    // check if should be starved by restricted flow from tank
    if (thisEvapCond.EvapWaterSupplyMode == WaterSupply::FromTank) {
        Real64 AvailWaterRate =
            state.dataWaterData->WaterStorage(thisEvapCond.EvapWaterSupTankID).VdotAvailDemand(thisEvapCond.EvapWaterTankDemandARRID);
        if (AvailWaterRate < thisEvapCond.EvapWaterConsumpRate) {
            thisEvapCond.EvapWaterStarvMakupRate = thisEvapCond.EvapWaterConsumpRate - AvailWaterRate;
            thisEvapCond.EvapWaterConsumpRate = AvailWaterRate;
        } else {
            thisEvapCond.EvapWaterStarvMakupRate = 0.0;
        }
    }

    if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
        thisOutletNode.CO2 = thisInletNode.CO2;
    }

    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
        thisOutletNode.GenContam = thisInletNode.GenContam;
    }
}

void ReportEvapCooler(EnergyPlusData &state, int const EvapCoolNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   Oct 2000

    Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;
    auto &thisEvapCond(state.dataEvapCoolers->EvapCond(EvapCoolNum));

    // report the Evap Cooler energy from this component
    thisEvapCond.EvapCoolerPower = thisEvapCond.EvapCoolerPower;
    thisEvapCond.EvapCoolerEnergy = thisEvapCond.EvapCoolerPower * TimeStepSysSec;

    // Report Water comsumption in cubic meters per timestep
    thisEvapCond.EvapWaterConsump = thisEvapCond.EvapWaterConsumpRate * TimeStepSysSec;
    thisEvapCond.EvapWaterStarvMakup = thisEvapCond.EvapWaterStarvMakupRate * TimeStepSysSec;
}

void SimZoneEvaporativeCoolerUnit(EnergyPlusData &state,
                                  std::string_view CompName,      // name of the packaged terminal heat pump
                                  int const ZoneNum,              // number of zone being served
                                  Real64 &SensibleOutputProvided, // sensible capacity delivered to zone
                                  Real64 &LatentOutputProvided,   // Latent add/removal  (kg/s), dehumid = negative
                                  int &CompIndex                  // index to zone hvac unit
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   July 2013
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // public simulation routine for managing zone hvac evaporative cooler unit

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int CompNum;

    auto &ZoneEvapUnit(state.dataEvapCoolers->ZoneEvapUnit);

    if (state.dataEvapCoolers->GetInputZoneEvapUnit) {
        GetInputZoneEvaporativeCoolerUnit(state);
        state.dataEvapCoolers->GetInputZoneEvapUnit = false;
    }

    // Find the correct Equipment
    if (CompIndex == 0) {
        CompNum = Util::FindItemInList(CompName, ZoneEvapUnit);
        if (CompNum == 0) {
            ShowFatalError(state, "SimZoneEvaporativeCoolerUnit: Zone evaporative cooler unit not found.");
        }
        CompIndex = CompNum;
    } else {
        CompNum = CompIndex;
        if (CompNum < 1 || CompNum > state.dataEvapCoolers->NumZoneEvapUnits) {
            ShowFatalError(state,
                           format("SimZoneEvaporativeCoolerUnit: Invalid CompIndex passed={}, Number of units ={}, Entered Unit name = {}",
                                  CompNum,
                                  state.dataEvapCoolers->NumZoneEvapUnits,
                                  CompName));
        }
        if (state.dataEvapCoolers->CheckZoneEvapUnitName(CompNum)) {
            if (CompName != ZoneEvapUnit(CompNum).Name) {
                ShowFatalError(state,
                               format("SimZoneEvaporativeCoolerUnit: Invalid CompIndex passed={}, Unit name={}, stored unit name for that index={}",
                                      CompNum,
                                      CompName,
                                      ZoneEvapUnit(CompNum).Name));
            }
            state.dataEvapCoolers->CheckZoneEvapUnitName(CompNum) = false;
        }
    }

    InitZoneEvaporativeCoolerUnit(state, CompNum, ZoneNum);

    CalcZoneEvaporativeCoolerUnit(state, CompNum, ZoneNum, SensibleOutputProvided, LatentOutputProvided);

    ReportZoneEvaporativeCoolerUnit(state, CompNum);
}

void GetInputZoneEvaporativeCoolerUnit(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   July 2013

    // PURPOSE OF THIS SUBROUTINE:
    // get input for zone evap cooler unit

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view routineName = "GetInputZoneEvaporativeCoolerUnit";
    static constexpr std::string_view RoutineName("GetInputZoneEvaporativeCoolerUnit: ");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    std::string CurrentModuleObject; // Object type for getting and error messages
    Array1D_string Alphas;           // Alpha items for object
    Array1D<Real64> Numbers;         // Numeric items for object
    Array1D_string cAlphaFields;     // Alpha field names
    Array1D_string cNumericFields;   // Numeric field names
    Array1D_bool lAlphaBlanks;       // Logical array, alpha field input BLANK = .TRUE.
    Array1D_bool lNumericBlanks;     // Logical array, numeric field input BLANK = .TRUE.
    int NumAlphas;                   // Number of Alphas for each GetObjectItem call
    int NumNumbers;                  // Number of Numbers for each GetObjectItem call
    int NumFields;                   // Total number of fields in object
    bool ErrorsFound(false);         // Set to true if errors in input, fatal at end of routine

    auto &EvapCond(state.dataEvapCoolers->EvapCond);
    auto &ZoneEvapUnit(state.dataEvapCoolers->ZoneEvapUnit);

    if (state.dataEvapCoolers->GetInputEvapComponentsFlag) {
        GetEvapInput(state);
        state.dataEvapCoolers->GetInputEvapComponentsFlag = false;
    }

    state.dataEvapCoolers->GetInputZoneEvapUnit = false;
    int MaxNumbers = 0;
    int MaxAlphas = 0;

    CurrentModuleObject = "ZoneHVAC:EvaporativeCoolerUnit";
    state.dataEvapCoolers->NumZoneEvapUnits = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumFields, NumAlphas, NumNumbers);
    MaxNumbers = max(MaxNumbers, NumNumbers);
    MaxAlphas = max(MaxAlphas, NumAlphas);
    Alphas.allocate(MaxAlphas);
    Numbers.dimension(MaxNumbers, 0.0);
    cAlphaFields.allocate(MaxAlphas);
    cNumericFields.allocate(MaxNumbers);
    lAlphaBlanks.dimension(MaxAlphas, true);
    lNumericBlanks.dimension(MaxNumbers, true);

    if (state.dataEvapCoolers->NumZoneEvapUnits > 0) {
        int IOStatus; // Used in GetObjectItem
        state.dataEvapCoolers->CheckZoneEvapUnitName.dimension(state.dataEvapCoolers->NumZoneEvapUnits, true);
        ZoneEvapUnit.allocate(state.dataEvapCoolers->NumZoneEvapUnits);

        for (int UnitLoop = 1; UnitLoop <= state.dataEvapCoolers->NumZoneEvapUnits; ++UnitLoop) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     UnitLoop,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            auto &thisZoneEvapUnit = ZoneEvapUnit(UnitLoop);
            thisZoneEvapUnit.Name = Alphas(1);
            if (lAlphaBlanks(2)) {
                thisZoneEvapUnit.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((thisZoneEvapUnit.availSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ErrorsFound = true;
            }

            if (!lAlphaBlanks(3)) {
                thisZoneEvapUnit.AvailManagerListName = Alphas(3);
            }

            thisZoneEvapUnit.OAInletNodeNum = GetOnlySingleNode(state,
                                                                Alphas(4),
                                                                ErrorsFound,
                                                                DataLoopNode::ConnectionObjectType::ZoneHVACEvaporativeCoolerUnit,
                                                                Alphas(1),
                                                                DataLoopNode::NodeFluidType::Air,
                                                                DataLoopNode::ConnectionType::OutsideAir,
                                                                NodeInputManager::CompFluidStream::Primary,
                                                                DataLoopNode::ObjectIsParent);

            thisZoneEvapUnit.UnitOutletNodeNum = GetOnlySingleNode(state,
                                                                   Alphas(5),
                                                                   ErrorsFound,
                                                                   DataLoopNode::ConnectionObjectType::ZoneHVACEvaporativeCoolerUnit,
                                                                   Alphas(1),
                                                                   DataLoopNode::NodeFluidType::Air,
                                                                   DataLoopNode::ConnectionType::Outlet,
                                                                   NodeInputManager::CompFluidStream::Primary,
                                                                   DataLoopNode::ObjectIsParent);

            if (!lAlphaBlanks(6)) {
                thisZoneEvapUnit.UnitReliefNodeNum = GetOnlySingleNode(state,
                                                                       Alphas(6),
                                                                       ErrorsFound,
                                                                       DataLoopNode::ConnectionObjectType::ZoneHVACEvaporativeCoolerUnit,
                                                                       Alphas(1),
                                                                       DataLoopNode::NodeFluidType::Air,
                                                                       DataLoopNode::ConnectionType::Inlet,
                                                                       NodeInputManager::CompFluidStream::Primary,
                                                                       DataLoopNode::ObjectIsParent);
            }

            thisZoneEvapUnit.FanName = Alphas(8);

            thisZoneEvapUnit.fanType = static_cast<HVAC::FanType>(getEnumValue(HVAC::fanTypeNamesUC, Alphas(7)));
            assert(thisZoneEvapUnit.fanType != HVAC::FanType::Invalid);

            thisZoneEvapUnit.FanIndex = Fans::GetFanIndex(state, thisZoneEvapUnit.FanName);
            if (thisZoneEvapUnit.FanIndex == 0) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(8), thisZoneEvapUnit.FanName);
                ErrorsFound = true;
            } else {
                auto *fan = state.dataFans->fans(thisZoneEvapUnit.FanIndex);
                assert(thisZoneEvapUnit.fanType == fan->type);
                thisZoneEvapUnit.FanInletNodeNum = fan->inletNodeNum;
                thisZoneEvapUnit.FanOutletNodeNum = fan->outletNodeNum;
                thisZoneEvapUnit.ActualFanVolFlowRate = fan->maxAirFlowRate;
                thisZoneEvapUnit.fanAvailSched = fan->availSched;
            }

            // set evap unit to cycling mode for all fan types. Note OpMode var is not used
            // with used for ZONECOOLINGLOADVARIABLESPEEDFAN Cooler Unit Control Method
            thisZoneEvapUnit.fanOp = HVAC::FanOp::Cycling;

            thisZoneEvapUnit.DesignAirVolumeFlowRate = Numbers(1);

            thisZoneEvapUnit.fanPlace = static_cast<HVAC::FanPlace>(getEnumValue(HVAC::fanPlaceNamesUC, Alphas(9)));
            assert(thisZoneEvapUnit.fanPlace != HVAC::FanPlace::Invalid);

            // get the zone numer served by the zoneHVAC evaporative cooler
            for (int CtrlZone = 1; CtrlZone <= state.dataGlobal->NumOfZones; ++CtrlZone) {
                if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZone).IsControlled) {
                    continue;
                }
                for (int NodeNum = 1; NodeNum <= state.dataZoneEquip->ZoneEquipConfig(CtrlZone).NumInletNodes; ++NodeNum) {
                    if (thisZoneEvapUnit.UnitOutletNodeNum == state.dataZoneEquip->ZoneEquipConfig(CtrlZone).InletNode(NodeNum)) {
                        thisZoneEvapUnit.ZonePtr = CtrlZone;
                        break;
                    }
                }
            }

            constexpr std::array<std::string_view, static_cast<int>(ControlType::Num)> controlTypeNamesUC = {
                "ZONETEMPERATUREDEADBANDONOFFCYCLING", "ZONECOOLINGLOADONOFFCYCLING", "ZONECOOLINGLOADVARIABLESPEEDFAN"};
            thisZoneEvapUnit.ControlSchemeType = static_cast<ControlType>(getEnumValue(controlTypeNamesUC, Alphas(10)));
            if (thisZoneEvapUnit.ControlSchemeType == ControlType::Invalid) {
                ShowSevereError(state, format("{}=\"{}\" invalid data.", CurrentModuleObject, thisZoneEvapUnit.Name));
                ShowContinueError(state, format("invalid choice found {}=\"{}\".", cAlphaFields(10), Alphas(10)));
                ErrorsFound = true;
            }

            thisZoneEvapUnit.ThrottlingRange = Numbers(2);
            thisZoneEvapUnit.ThresholdCoolingLoad = Numbers(3);

            thisZoneEvapUnit.EvapCooler_1_Type_Num = static_cast<EvapCoolerType>(getEnumValue(evapCoolerTypeNamesUC, Alphas(11)));
            if (thisZoneEvapUnit.EvapCooler_1_Type_Num != EvapCoolerType::Invalid) {
                thisZoneEvapUnit.EvapCooler_1_ObjectClassName = evapCoolerTypeNames[static_cast<int>(thisZoneEvapUnit.EvapCooler_1_Type_Num)];
            } else {
                ShowSevereError(state, format("{}=\"{}\" invalid data.", CurrentModuleObject, thisZoneEvapUnit.Name));
                ShowContinueError(state, format("invalid choice found {}=\"{}\".", cAlphaFields(11), Alphas(11)));
                ErrorsFound = true;
            }

            thisZoneEvapUnit.EvapCooler_1_Name = Alphas(12);
            thisZoneEvapUnit.EvapCooler_1_Index = Util::FindItemInList(Alphas(12), state.dataEvapCoolers->EvapCond, &EvapConditions::Name);
            if (thisZoneEvapUnit.EvapCooler_1_Index == 0) {
                ShowSevereError(state, format("{}=\"{}\" invalid data.", CurrentModuleObject, thisZoneEvapUnit.Name));
                ShowContinueError(state, format("invalid, not found {}=\"{}\".", cAlphaFields(12), Alphas(12)));
                ErrorsFound = true;
            }

            if (!lAlphaBlanks(13)) {
                thisZoneEvapUnit.EvapCooler_2_Type_Num = static_cast<EvapCoolerType>(getEnumValue(evapCoolerTypeNamesUC, Alphas(13)));
                if (thisZoneEvapUnit.EvapCooler_2_Type_Num != EvapCoolerType::Invalid) {
                    thisZoneEvapUnit.EvapCooler_2_ObjectClassName = evapCoolerTypeNames[static_cast<int>(thisZoneEvapUnit.EvapCooler_2_Type_Num)];
                } else {
                    ShowSevereError(state, format("{}=\"{}\" invalid data.", CurrentModuleObject, thisZoneEvapUnit.Name));
                    ShowContinueError(state, format("invalid choice found {}=\"{}\".", cAlphaFields(13), Alphas(13)));
                    ErrorsFound = true;
                }

                if (!lAlphaBlanks(14)) {
                    thisZoneEvapUnit.EvapCooler_2_Name = Alphas(14);
                    thisZoneEvapUnit.EvapCooler_2_Index = Util::FindItemInList(Alphas(14), state.dataEvapCoolers->EvapCond, &EvapConditions::Name);
                    if (thisZoneEvapUnit.EvapCooler_2_Index == 0) {
                        ShowSevereError(state, format("{}=\"{}\" invalid data.", CurrentModuleObject, thisZoneEvapUnit.Name));
                        ShowContinueError(state, format("invalid, not found {}=\"{}\".", cAlphaFields(14), Alphas(14)));
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(state, format("{}=\"{}\" invalid data.", CurrentModuleObject, thisZoneEvapUnit.Name));
                    ShowContinueError(state, format("missing input for {}", cAlphaFields(14)));
                    ErrorsFound = true;
                }
            }

            thisZoneEvapUnit.HVACSizingIndex = 0;
            if (!lAlphaBlanks(15)) {
                thisZoneEvapUnit.HVACSizingIndex = Util::FindItemInList(Alphas(15), state.dataSize->ZoneHVACSizing);
                if (thisZoneEvapUnit.HVACSizingIndex == 0) {
                    ShowSevereError(state, format("{} = {} not found.", cAlphaFields(15), Alphas(15)));
                    ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, thisZoneEvapUnit.Name));
                    ErrorsFound = true;
                }
            }

            if (!lNumericBlanks(4)) {
                // Shut Off Relative Humidity
                thisZoneEvapUnit.ShutOffRelativeHumidity = Numbers(4);
            }

            // Add fan to component sets array
            BranchNodeConnections::SetUpCompSets(state,
                                                 CurrentModuleObject,
                                                 thisZoneEvapUnit.Name,
                                                 HVAC::fanTypeNamesUC[(int)thisZoneEvapUnit.fanType],
                                                 thisZoneEvapUnit.FanName,
                                                 state.dataLoopNodes->NodeID(thisZoneEvapUnit.FanInletNodeNum),
                                                 state.dataLoopNodes->NodeID(thisZoneEvapUnit.FanOutletNodeNum));

            // Add first evap cooler to component sets array
            BranchNodeConnections::SetUpCompSets(state,
                                                 CurrentModuleObject,
                                                 thisZoneEvapUnit.Name,
                                                 thisZoneEvapUnit.EvapCooler_1_ObjectClassName,
                                                 thisZoneEvapUnit.EvapCooler_1_Name,
                                                 state.dataLoopNodes->NodeID(EvapCond(thisZoneEvapUnit.EvapCooler_1_Index).InletNode),
                                                 state.dataLoopNodes->NodeID(EvapCond(thisZoneEvapUnit.EvapCooler_1_Index).OutletNode));

            if (thisZoneEvapUnit.EvapCooler_2_Index > 0) {
                // Add second evap cooler to component sets array
                BranchNodeConnections::SetUpCompSets(state,
                                                     CurrentModuleObject,
                                                     thisZoneEvapUnit.Name,
                                                     thisZoneEvapUnit.EvapCooler_2_ObjectClassName,
                                                     thisZoneEvapUnit.EvapCooler_2_Name,
                                                     state.dataLoopNodes->NodeID(EvapCond(thisZoneEvapUnit.EvapCooler_2_Index).InletNode),
                                                     state.dataLoopNodes->NodeID(EvapCond(thisZoneEvapUnit.EvapCooler_2_Index).OutletNode));
            }

            // check that fan type is consistent with control method
            if (thisZoneEvapUnit.ControlSchemeType == ControlType::ZoneCoolingLoadVariableSpeedFan) { // must have a VS fan type
                if (thisZoneEvapUnit.fanType == HVAC::FanType::Constant) {
                    ShowSevereError(state, format("{}=\"{}\" invalid data.", CurrentModuleObject, thisZoneEvapUnit.Name));
                    ShowContinueError(state, "Fan:ConstantVolume is not consistent with control method ZoneCoolingLoadVariableSpeedFan.");
                    ShowContinueError(state, "Change to a variable speed fan object type");
                    ErrorsFound = true;
                } else if (thisZoneEvapUnit.fanType == HVAC::FanType::OnOff) {
                    ShowSevereError(state, format("{}=\"{}\" invalid data.", CurrentModuleObject, thisZoneEvapUnit.Name));
                    ShowContinueError(state, "Fan:OnOff is not consistent with control method ZoneCoolingLoadVariableSpeedFan.");
                    ShowContinueError(state, "Change to a variable speed fan object type");
                    ErrorsFound = true;
                }
            }

        } // unit loop
    }

    //***********************************************************************************

    Alphas.deallocate();
    Numbers.deallocate();
    cAlphaFields.deallocate();
    cNumericFields.deallocate();
    lAlphaBlanks.deallocate();
    lNumericBlanks.deallocate();

    if (ErrorsFound) {
        ShowFatalError(state, format("{}Errors found in getting input.", RoutineName));
        ShowContinueError(state, "... Preceding condition causes termination.");
    }

    // setup output variables
    for (int UnitLoop = 1; UnitLoop <= state.dataEvapCoolers->NumZoneEvapUnits; ++UnitLoop) {
        auto &thisZoneEvapUnit = ZoneEvapUnit(UnitLoop);
        SetupOutputVariable(state,
                            "Zone Evaporative Cooler Unit Total Cooling Rate",
                            Constant::Units::W,
                            thisZoneEvapUnit.UnitTotalCoolingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisZoneEvapUnit.Name);
        SetupOutputVariable(state,
                            "Zone Evaporative Cooler Unit Total Cooling Energy",
                            Constant::Units::J,
                            thisZoneEvapUnit.UnitTotalCoolingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisZoneEvapUnit.Name,
                            Constant::eResource::EnergyTransfer,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::CoolingCoils);
        SetupOutputVariable(state,
                            "Zone Evaporative Cooler Unit Sensible Cooling Rate",
                            Constant::Units::W,
                            thisZoneEvapUnit.UnitSensibleCoolingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisZoneEvapUnit.Name);
        SetupOutputVariable(state,
                            "Zone Evaporative Cooler Unit Sensible Cooling Energy",
                            Constant::Units::J,
                            thisZoneEvapUnit.UnitSensibleCoolingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisZoneEvapUnit.Name);
        SetupOutputVariable(state,
                            "Zone Evaporative Cooler Unit Latent Heating Rate",
                            Constant::Units::W,
                            thisZoneEvapUnit.UnitLatentHeatingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisZoneEvapUnit.Name);
        SetupOutputVariable(state,
                            "Zone Evaporative Cooler Unit Latent Heating Energy",
                            Constant::Units::J,
                            thisZoneEvapUnit.UnitLatentHeatingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisZoneEvapUnit.Name);
        SetupOutputVariable(state,
                            "Zone Evaporative Cooler Unit Latent Cooling Rate",
                            Constant::Units::W,
                            thisZoneEvapUnit.UnitLatentCoolingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisZoneEvapUnit.Name);
        SetupOutputVariable(state,
                            "Zone Evaporative Cooler Unit Latent Cooling Energy",
                            Constant::Units::J,
                            thisZoneEvapUnit.UnitLatentCoolingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            thisZoneEvapUnit.Name);
        SetupOutputVariable(state,
                            "Zone Evaporative Cooler Unit Fan Speed Ratio",
                            Constant::Units::None,
                            thisZoneEvapUnit.UnitFanSpeedRatio,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisZoneEvapUnit.Name);
        SetupOutputVariable(state,
                            "Zone Evaporative Cooler Unit Fan Availability Status",
                            Constant::Units::None,
                            thisZoneEvapUnit.FanAvailStatus,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            thisZoneEvapUnit.Name);
        if (thisZoneEvapUnit.ControlSchemeType != ControlType::ZoneCoolingLoadVariableSpeedFan) {
            SetupOutputVariable(state,
                                "Zone Evaporative Cooler Unit Part Load Ratio",
                                Constant::Units::None,
                                thisZoneEvapUnit.UnitPartLoadRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisZoneEvapUnit.Name);
        }
    }
}

void InitZoneEvaporativeCoolerUnit(EnergyPlusData &state,
                                   int const UnitNum, // unit number
                                   int const ZoneNum  // number of zone being served
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   July 2013

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    auto &zoneEvapUnit = state.dataEvapCoolers->ZoneEvapUnit(UnitNum);

    if (allocated(state.dataAvail->ZoneComp)) {
        if (zoneEvapUnit.MyZoneEq) { // initialize the name of each availability manager list and zone number
            state.dataAvail->ZoneComp(DataZoneEquipment::ZoneEquipType::EvaporativeCooler).ZoneCompAvailMgrs(UnitNum).AvailManagerListName =
                zoneEvapUnit.AvailManagerListName;
            state.dataAvail->ZoneComp(DataZoneEquipment::ZoneEquipType::EvaporativeCooler).ZoneCompAvailMgrs(UnitNum).ZoneNum = ZoneNum;
            zoneEvapUnit.MyZoneEq = false;
        }
        zoneEvapUnit.FanAvailStatus =
            state.dataAvail->ZoneComp(DataZoneEquipment::ZoneEquipType::EvaporativeCooler).ZoneCompAvailMgrs(UnitNum).availStatus;
    }

    if (!state.dataEvapCoolers->ZoneEquipmentListChecked && state.dataZoneEquip->ZoneEquipInputsFilled) {
        state.dataEvapCoolers->ZoneEquipmentListChecked = true;
        for (int Loop = 1; Loop <= state.dataEvapCoolers->NumZoneEvapUnits; ++Loop) {
            if (DataZoneEquipment::CheckZoneEquipmentList(state, "ZoneHVAC:EvaporativeCoolerUnit", state.dataEvapCoolers->ZoneEvapUnit(Loop).Name)) {
                state.dataEvapCoolers->ZoneEvapUnit(Loop).ZoneNodeNum = state.dataZoneEquip->ZoneEquipConfig(ZoneNum).ZoneNode;
            } else {
                ShowSevereError(state,
                                format("InitZoneEvaporativeCoolerUnit: ZoneHVAC:EvaporativeCoolerUnit = {}, is not on any ZoneHVAC:EquipmentList.  "
                                       "It will not be simulated.",
                                       state.dataEvapCoolers->ZoneEvapUnit(Loop).Name));
            }
        }
    }

    if (!state.dataGlobal->SysSizingCalc && zoneEvapUnit.MySize) {
        SizeZoneEvaporativeCoolerUnit(state, UnitNum);
        zoneEvapUnit.MySize = false;
    }

    if (zoneEvapUnit.MyFan) {
        if (zoneEvapUnit.ActualFanVolFlowRate != DataSizing::AutoSize) {

            if (zoneEvapUnit.ActualFanVolFlowRate < zoneEvapUnit.DesignAirVolumeFlowRate) {
                ShowSevereError(state, format("InitZoneEvaporativeCoolerUnit: ZoneHVAC:EvaporativeCoolerUnit = {}", zoneEvapUnit.Name));
                ShowContinueError(state, "...unit fan volumetric flow rate less than evaporative cooler unit design supply air flow rate.");
                ShowContinueError(state, format("...fan volumetric flow rate = {:.5T} m3/s.", zoneEvapUnit.ActualFanVolFlowRate));
                ShowContinueError(state, format("...evap cooler unit volumetric flow rate = {:.5T} m3/s.", zoneEvapUnit.DesignAirVolumeFlowRate));
                zoneEvapUnit.DesignAirVolumeFlowRate = zoneEvapUnit.ActualFanVolFlowRate;
                ShowContinueError(state, "...evaporative cooler unit design supply air flow rate will match fan flow rate and simulation continues.");
                zoneEvapUnit.MyEnvrn = true; // re-initialize to set mass flow rate and max mass flow rate
            }

            if (zoneEvapUnit.ActualFanVolFlowRate > 0.0) {
                zoneEvapUnit.DesignFanSpeedRatio = zoneEvapUnit.DesignAirVolumeFlowRate / zoneEvapUnit.ActualFanVolFlowRate;
            }

            zoneEvapUnit.MyFan = false;
        } else {
            zoneEvapUnit.ActualFanVolFlowRate = state.dataFans->fans(zoneEvapUnit.FanIndex)->maxAirFlowRate;
        }
    }

    if (zoneEvapUnit.fanAvailSched != nullptr) {
        // include fan is not available, then unit is not available
        zoneEvapUnit.UnitIsAvailable = ((zoneEvapUnit.fanAvailSched->getCurrentVal() > 0.0) && (zoneEvapUnit.availSched->getCurrentVal() > 0.0));
    } else {
        zoneEvapUnit.UnitIsAvailable = (zoneEvapUnit.availSched->getCurrentVal() > 0.0);
    }

    zoneEvapUnit.EvapCooler_1_AvailStatus = (state.dataEvapCoolers->EvapCond(zoneEvapUnit.EvapCooler_1_Index).availSched->getCurrentVal() > 0.0);

    if (zoneEvapUnit.EvapCooler_2_Index > 0) {
        zoneEvapUnit.EvapCooler_2_AvailStatus = (state.dataEvapCoolers->EvapCond(zoneEvapUnit.EvapCooler_2_Index).availSched->getCurrentVal() > 0.0);
    }
    // Do the Begin Environment initializations
    if (state.dataGlobal->BeginEnvrnFlag && zoneEvapUnit.MyEnvrn) {
        zoneEvapUnit.DesignAirMassFlowRate = state.dataEnvrn->StdRhoAir * zoneEvapUnit.DesignAirVolumeFlowRate;
        state.dataLoopNodes->Node(zoneEvapUnit.OAInletNodeNum).MassFlowRateMax = zoneEvapUnit.DesignAirMassFlowRate;
        state.dataLoopNodes->Node(zoneEvapUnit.OAInletNodeNum).MassFlowRateMin = 0.0;
        state.dataLoopNodes->Node(zoneEvapUnit.OAInletNodeNum).MassFlowRateMinAvail = 0.0;

        state.dataLoopNodes->Node(zoneEvapUnit.UnitOutletNodeNum).MassFlowRateMax = zoneEvapUnit.DesignAirMassFlowRate;
        state.dataLoopNodes->Node(zoneEvapUnit.UnitOutletNodeNum).MassFlowRateMin = 0.0;
        state.dataLoopNodes->Node(zoneEvapUnit.UnitOutletNodeNum).MassFlowRateMinAvail = 0.0;

        if (zoneEvapUnit.UnitReliefNodeNum > 0) {
            state.dataLoopNodes->Node(zoneEvapUnit.UnitReliefNodeNum).MassFlowRateMax = zoneEvapUnit.DesignAirMassFlowRate;
            state.dataLoopNodes->Node(zoneEvapUnit.UnitReliefNodeNum).MassFlowRateMin = 0.0;
            state.dataLoopNodes->Node(zoneEvapUnit.UnitReliefNodeNum).MassFlowRateMinAvail = 0.0;
        }
        zoneEvapUnit.WasOnLastTimestep = false;
        zoneEvapUnit.IsOnThisTimestep = false;
        zoneEvapUnit.FanSpeedRatio = 0.0;
        zoneEvapUnit.UnitFanSpeedRatio = 0.0;
        zoneEvapUnit.UnitTotalCoolingRate = 0.0;
        zoneEvapUnit.UnitTotalCoolingEnergy = 0.0;
        zoneEvapUnit.UnitSensibleCoolingRate = 0.0;
        zoneEvapUnit.UnitSensibleCoolingEnergy = 0.0;
        zoneEvapUnit.UnitLatentHeatingRate = 0.0;
        zoneEvapUnit.UnitLatentHeatingEnergy = 0.0;
        zoneEvapUnit.UnitLatentCoolingRate = 0.0;
        zoneEvapUnit.UnitLatentCoolingEnergy = 0.0;
        zoneEvapUnit.FanAvailStatus = Avail::Status::NoAction;

        // place default cold setpoints on control nodes of select evap coolers
        if ((zoneEvapUnit.EvapCooler_1_Type_Num == EvapCoolerType::DirectResearchSpecial) ||
            (zoneEvapUnit.EvapCooler_1_Type_Num == EvapCoolerType::IndirectRDDSpecial)) {
            if (state.dataEvapCoolers->EvapCond(zoneEvapUnit.EvapCooler_1_Index).EvapControlNodeNum > 0) {
                state.dataLoopNodes->Node(state.dataEvapCoolers->EvapCond(zoneEvapUnit.EvapCooler_1_Index).EvapControlNodeNum).TempSetPoint = -20.0;
            }
        }
        if ((zoneEvapUnit.EvapCooler_2_Type_Num == EvapCoolerType::DirectResearchSpecial) ||
            (zoneEvapUnit.EvapCooler_2_Type_Num == EvapCoolerType::IndirectRDDSpecial)) {
            if (state.dataEvapCoolers->EvapCond(zoneEvapUnit.EvapCooler_2_Index).EvapControlNodeNum > 0) {
                state.dataLoopNodes->Node(state.dataEvapCoolers->EvapCond(zoneEvapUnit.EvapCooler_2_Index).EvapControlNodeNum).TempSetPoint = -20.0;
            }
        }

        zoneEvapUnit.MyEnvrn = false;
    }
    if (!state.dataGlobal->BeginEnvrnFlag) {
        zoneEvapUnit.MyEnvrn = true;
    }

    Real64 TimeElapsed =
        state.dataGlobal->HourOfDay + state.dataGlobal->TimeStep * state.dataGlobal->TimeStepZone + state.dataHVACGlobal->SysTimeElapsed;
    if (zoneEvapUnit.TimeElapsed != TimeElapsed) {
        zoneEvapUnit.WasOnLastTimestep = zoneEvapUnit.IsOnThisTimestep;
        zoneEvapUnit.TimeElapsed = TimeElapsed;
    }
}

void SizeZoneEvaporativeCoolerUnit(EnergyPlusData &state, int const UnitNum) // unit number
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   July 2013
    //       MODIFIED       August 2014 Bereket Nigusse, added scalable sizing
    //       MODIFIED       January 2013 Daeho Kang, add component sizing table entries

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("SizeZoneEvaporativeCoolerUnit: "); // include trailing blank space

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 TempSize; // autosized value of coil input field

    auto &zoneEvapUnit = state.dataEvapCoolers->ZoneEvapUnit(UnitNum);

    state.dataSize->DataScalableSizingON = false;
    state.dataSize->ZoneHeatingOnlyFan = false;
    state.dataSize->ZoneCoolingOnlyFan = false;

    state.dataSize->DataZoneNumber = zoneEvapUnit.ZonePtr;

    if (state.dataSize->CurZoneEqNum > 0) {
        std::string CompName = zoneEvapUnit.Name;
        std::string CompType = "ZoneHVAC:EvaporativeCoolerUnit";
        bool errorsFound = false;
        auto &zoneEqSizing = state.dataSize->ZoneEqSizing(state.dataSize->CurZoneEqNum);
        bool PrintFlag = true; // TRUE when sizing information is reported in the eio file

        if (zoneEvapUnit.HVACSizingIndex > 0) {
            state.dataSize->ZoneCoolingOnlyFan = true;
            // index of zoneHVAC equipment sizing specification
            int zoneHVACIndex = zoneEvapUnit.HVACSizingIndex;
            // Integer representation of sizing method name (e.g., CoolingAirflowSizing, HeatingAirflowSizing,
            // CoolingCapacitySizing, HeatingCapacitySizing, etc.)
            int SizingMethod = HVAC::CoolingAirflowSizing;
            // supply air flow rate sizing method (SupplyAirFlowRate, FlowPerFloorArea, FractionOfAutosizedCoolingAirflow,
            // FractionOfAutosizedHeatingAirflow ...)
            int SAFMethod = state.dataSize->ZoneHVACSizing(zoneHVACIndex).CoolingSAFMethod;
            zoneEqSizing.SizingMethod(SizingMethod) = SAFMethod;
            if (SAFMethod == DataSizing::None || SAFMethod == DataSizing::SupplyAirFlowRate || SAFMethod == DataSizing::FlowPerFloorArea ||
                SAFMethod == DataSizing::FractionOfAutosizedCoolingAirflow) {
                switch (SAFMethod) {
                case DataSizing::SupplyAirFlowRate:
                    if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow > 0.0) {
                        zoneEqSizing.AirVolFlow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
                        zoneEqSizing.SystemAirFlow = true;
                    }
                    TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
                    if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow > 0.0) {
                        PrintFlag = false;
                    }
                    break;
                case DataSizing::FlowPerFloorArea:
                    zoneEqSizing.SystemAirFlow = true;
                    zoneEqSizing.AirVolFlow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow *
                                              state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                    TempSize = zoneEqSizing.AirVolFlow;
                    state.dataSize->DataScalableSizingON = true;
                    break;
                case DataSizing::FractionOfAutosizedCoolingAirflow:
                    state.dataSize->DataFracOfAutosizedCoolingAirflow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
                    TempSize = DataSizing::AutoSize;
                    state.dataSize->DataScalableSizingON = true;
                    break;
                default:
                    TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
                }

                CoolingAirFlowSizer sizingCoolingAirFlow;
                std::string stringOverride = "Design Supply Air Flow Rate [m3/s]";
                if (state.dataGlobal->isEpJSON) {
                    stringOverride = "design_supply_air_flow_rate [m3/s]";
                }
                sizingCoolingAirFlow.overrideSizingString(stringOverride);
                sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                zoneEvapUnit.DesignAirVolumeFlowRate = sizingCoolingAirFlow.size(state, TempSize, errorsFound);

            } else if (SAFMethod == DataSizing::FlowPerCoolingCapacity) {
                SizingMethod = HVAC::CoolingCapacitySizing;
                TempSize = DataSizing::AutoSize;
                PrintFlag = false;
                state.dataSize->DataScalableSizingON = true;
                state.dataSize->DataFlowUsedForSizing = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolVolFlow;
                if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).CoolingCapMethod == DataSizing::FractionOfAutosizedCoolingCapacity) {
                    state.dataSize->DataFracOfAutosizedCoolingCapacity = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledCoolingCapacity;
                }
                CoolingCapacitySizer sizerCoolingCapacity;
                sizerCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                state.dataSize->DataCapacityUsedForSizing = sizerCoolingCapacity.size(state, TempSize, errorsFound);
                state.dataSize->DataFlowPerCoolingCapacity = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
                PrintFlag = true;
                TempSize = DataSizing::AutoSize;

                CoolingAirFlowSizer sizingCoolingAirFlow;
                std::string stringOverride = "Design Supply Air Flow Rate [m3/s]";
                if (state.dataGlobal->isEpJSON) {
                    stringOverride = "design_supply_air_flow_rate [m3/s]";
                }
                sizingCoolingAirFlow.overrideSizingString(stringOverride);
                sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                zoneEvapUnit.DesignAirVolumeFlowRate = sizingCoolingAirFlow.size(state, TempSize, errorsFound);
            }
            state.dataSize->DataScalableSizingON = false;
            state.dataSize->ZoneCoolingOnlyFan = false;
        } else {
            // no scalble sizing method has been specified. Sizing proceeds using the method
            // specified in the zoneHVAC object
            // N1 , \field Maximum Supply Air Flow Rate
            state.dataSize->ZoneCoolingOnlyFan = true;
            if (zoneEvapUnit.DesignAirVolumeFlowRate > 0.0) {
                PrintFlag = false;
            }
            TempSize = zoneEvapUnit.DesignAirVolumeFlowRate;
            CoolingAirFlowSizer sizingCoolingAirFlow;
            std::string stringOverride = "Design Supply Air Flow Rate [m3/s]";
            if (state.dataGlobal->isEpJSON) {
                stringOverride = "design_supply_air_flow_rate [m3/s]";
            }
            sizingCoolingAirFlow.overrideSizingString(stringOverride);
            sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
            zoneEvapUnit.DesignAirVolumeFlowRate = sizingCoolingAirFlow.size(state, TempSize, errorsFound);
            state.dataSize->ZoneCoolingOnlyFan = false;
        }
    }
}

void CalcZoneEvaporativeCoolerUnit(EnergyPlusData &state,
                                   int const UnitNum,              // unit number
                                   int const ZoneNum,              // number of zone being served
                                   Real64 &SensibleOutputProvided, // sensible capacity delivered to zone
                                   Real64 &LatentOutputProvided    // Latent add/removal  (kg/s), dehumid = negative
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   July 2013

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 ZoneCoolingLoad;
    Real64 CoolingLoadThreashold;
    Real64 ZoneTemp;
    Real64 CoolSetLowThrottle;
    Real64 CoolSetHiThrottle;
    Real64 PartLoadRatio;

    {
        auto &zoneEvapUnit = state.dataEvapCoolers->ZoneEvapUnit(UnitNum);

        Real64 relativeHumidity = 100.0 * Psychrometrics::PsyRhFnTdbWPb(state,
                                                                        state.dataLoopNodes->Node(zoneEvapUnit.ZoneNodeNum).Temp,
                                                                        state.dataLoopNodes->Node(zoneEvapUnit.ZoneNodeNum).HumRat,
                                                                        state.dataEnvrn->OutBaroPress,
                                                                        "CalcZoneEvaporativeCoolerUnit");
        if (relativeHumidity > zoneEvapUnit.ShutOffRelativeHumidity) {
            // unit is off when humidity is too high
            PartLoadRatio = 0.0;
            zoneEvapUnit.UnitPartLoadRatio = PartLoadRatio;
            CalcZoneEvapUnitOutput(state, UnitNum, PartLoadRatio, SensibleOutputProvided, LatentOutputProvided);
            zoneEvapUnit.IsOnThisTimestep = false;
            return;
        }

        if (zoneEvapUnit.ControlSchemeType == ControlType::ZoneTemperatureDeadBandOnOffCycling) {
            ZoneTemp = state.dataLoopNodes->Node(zoneEvapUnit.ZoneNodeNum).Temp;

            auto const &zoneTstatSetpt = state.dataHeatBalFanSys->zoneTstatSetpts(ZoneNum);
            CoolSetLowThrottle = zoneTstatSetpt.setptHi - (0.5 * zoneEvapUnit.ThrottlingRange);
            CoolSetHiThrottle = zoneTstatSetpt.setptHi + (0.5 * zoneEvapUnit.ThrottlingRange);

            if ((ZoneTemp < CoolSetLowThrottle) || !zoneEvapUnit.UnitIsAvailable) {
                zoneEvapUnit.IsOnThisTimestep = false;
            } else if (ZoneTemp > CoolSetHiThrottle) {
                zoneEvapUnit.IsOnThisTimestep = true;
            } else {
                if (zoneEvapUnit.WasOnLastTimestep) {
                    zoneEvapUnit.IsOnThisTimestep = true;
                } else {
                    zoneEvapUnit.IsOnThisTimestep = false;
                }
            }

            if (zoneEvapUnit.IsOnThisTimestep) {

                if (zoneEvapUnit.fanOp == HVAC::FanOp::Continuous) {
                    PartLoadRatio = 1.0;
                    zoneEvapUnit.UnitPartLoadRatio = PartLoadRatio;
                    CalcZoneEvapUnitOutput(state, UnitNum, PartLoadRatio, SensibleOutputProvided, LatentOutputProvided);
                } else {
                    ZoneCoolingLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToCoolSP;
                    // calculate part load ratio for cycling fan/unit first
                    ControlZoneEvapUnitOutput(state, UnitNum, ZoneCoolingLoad);
                    PartLoadRatio = zoneEvapUnit.UnitPartLoadRatio;
                    CalcZoneEvapUnitOutput(state, UnitNum, PartLoadRatio, SensibleOutputProvided, LatentOutputProvided);
                }

            } else { // not running

                PartLoadRatio = 0.0;
                zoneEvapUnit.UnitPartLoadRatio = PartLoadRatio;
                CalcZoneEvapUnitOutput(state, UnitNum, PartLoadRatio, SensibleOutputProvided, LatentOutputProvided);
            }

        } else if (zoneEvapUnit.ControlSchemeType == ControlType::ZoneCoolingLoadOnOffCycling) {

            // get zone loads
            ZoneCoolingLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToCoolSP;
            CoolingLoadThreashold = -1.0 * zoneEvapUnit.ThresholdCoolingLoad;

            if ((ZoneCoolingLoad < CoolingLoadThreashold) && zoneEvapUnit.UnitIsAvailable) {

                if (zoneEvapUnit.fanOp == HVAC::FanOp::Continuous) {
                    PartLoadRatio = 1.0;
                    zoneEvapUnit.UnitPartLoadRatio = PartLoadRatio;
                    CalcZoneEvapUnitOutput(state, UnitNum, PartLoadRatio, SensibleOutputProvided, LatentOutputProvided);
                } else {
                    // calculate part load ratio for cycling fan/unit first
                    ControlZoneEvapUnitOutput(state, UnitNum, ZoneCoolingLoad);
                    PartLoadRatio = zoneEvapUnit.UnitPartLoadRatio;
                    CalcZoneEvapUnitOutput(state, UnitNum, PartLoadRatio, SensibleOutputProvided, LatentOutputProvided);
                }

            } else {
                // unit is off
                PartLoadRatio = 0.0;
                zoneEvapUnit.UnitPartLoadRatio = PartLoadRatio;
                CalcZoneEvapUnitOutput(state, UnitNum, PartLoadRatio, SensibleOutputProvided, LatentOutputProvided);
            }

        } else if (zoneEvapUnit.ControlSchemeType == ControlType::ZoneCoolingLoadVariableSpeedFan) {
            // get zone loads
            ZoneCoolingLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToCoolSP;
            CoolingLoadThreashold = -1.0 * zoneEvapUnit.ThresholdCoolingLoad;
            if ((ZoneCoolingLoad < CoolingLoadThreashold) && zoneEvapUnit.UnitIsAvailable) {

                // determine fan speed to meet load
                ControlVSEvapUnitToMeetLoad(state, UnitNum, ZoneCoolingLoad);
                // variable speed fan used fan speed ratio instead of partload ratio
                CalcZoneEvapUnitOutput(state, UnitNum, zoneEvapUnit.FanSpeedRatio, SensibleOutputProvided, LatentOutputProvided);

            } else {
                // unit is off
                PartLoadRatio = 0.0;
                CalcZoneEvapUnitOutput(state, UnitNum, PartLoadRatio, SensibleOutputProvided, LatentOutputProvided);
            }
        }
    }
}

void CalcZoneEvapUnitOutput(EnergyPlusData &state,
                            int const UnitNum,              // unit number
                            Real64 const PartLoadRatio,     // zone evap unit part load ratiod
                            Real64 &SensibleOutputProvided, // target cooling load
                            Real64 &LatentOutputProvided    // target cooling load
)
{
    // caculates zone evaporative cooler sensible and latent outputs

    Real64 MinHumRat; // minimum humidity ratio

    auto &zoneEvapUnit = state.dataEvapCoolers->ZoneEvapUnit(UnitNum);

    int const ZoneNodeNum = zoneEvapUnit.ZoneNodeNum;
    int const OAInletNodeNum = zoneEvapUnit.OAInletNodeNum;
    int const OutletNodeNum = zoneEvapUnit.UnitOutletNodeNum;
    int const ReliefNodeNum = zoneEvapUnit.UnitReliefNodeNum;
    int const FanInletNodeNum = zoneEvapUnit.FanInletNodeNum;
    int const FanOutletNodeNum = zoneEvapUnit.FanOutletNodeNum;
    int const EvapCooler_1_Index = zoneEvapUnit.EvapCooler_1_Index;
    int const EvapCooler_2_Index = zoneEvapUnit.EvapCooler_2_Index;

    // calculate unit sensible cooling output
    if (PartLoadRatio > 0) {
        state.dataLoopNodes->Node(OAInletNodeNum).MassFlowRate = zoneEvapUnit.DesignAirMassFlowRate * PartLoadRatio;
        state.dataLoopNodes->Node(OAInletNodeNum).MassFlowRateMaxAvail = state.dataLoopNodes->Node(OAInletNodeNum).MassFlowRate;
        state.dataLoopNodes->Node(OutletNodeNum).MassFlowRate = state.dataLoopNodes->Node(OAInletNodeNum).MassFlowRate;
        state.dataLoopNodes->Node(OutletNodeNum).MassFlowRateMaxAvail = state.dataLoopNodes->Node(OutletNodeNum).MassFlowRate;
    } else { // not running
        state.dataLoopNodes->Node(OAInletNodeNum).MassFlowRate = 0.0;
        state.dataLoopNodes->Node(OAInletNodeNum).MassFlowRateMaxAvail = 0.0;
        state.dataLoopNodes->Node(FanInletNodeNum).MassFlowRate = 0.0;
        state.dataLoopNodes->Node(FanInletNodeNum).MassFlowRateMaxAvail = 0.0;
        state.dataLoopNodes->Node(FanOutletNodeNum).MassFlowRate = 0.0;
        state.dataLoopNodes->Node(FanOutletNodeNum).MassFlowRateMaxAvail = 0.0;
        state.dataLoopNodes->Node(OutletNodeNum).MassFlowRate = 0.0;
        state.dataLoopNodes->Node(OutletNodeNum).MassFlowRateMaxAvail = 0.0;

        state.dataLoopNodes->Node(state.dataEvapCoolers->EvapCond(EvapCooler_1_Index).InletNode).MassFlowRate = 0.0;
        state.dataLoopNodes->Node(state.dataEvapCoolers->EvapCond(EvapCooler_1_Index).InletNode).MassFlowRateMaxAvail = 0.0;
        state.dataLoopNodes->Node(state.dataEvapCoolers->EvapCond(EvapCooler_1_Index).OutletNode).MassFlowRate = 0.0;
        state.dataLoopNodes->Node(state.dataEvapCoolers->EvapCond(EvapCooler_1_Index).OutletNode).MassFlowRateMaxAvail = 0.0;

        if (EvapCooler_2_Index > 0) {
            state.dataLoopNodes->Node(state.dataEvapCoolers->EvapCond(EvapCooler_2_Index).InletNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(state.dataEvapCoolers->EvapCond(EvapCooler_2_Index).InletNode).MassFlowRateMaxAvail = 0.0;
            state.dataLoopNodes->Node(state.dataEvapCoolers->EvapCond(EvapCooler_2_Index).OutletNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(state.dataEvapCoolers->EvapCond(EvapCooler_2_Index).OutletNode).MassFlowRateMaxAvail = 0.0;
        }
    }
    if (ReliefNodeNum > 0) {
        state.dataLoopNodes->Node(ReliefNodeNum).MassFlowRate = state.dataLoopNodes->Node(OAInletNodeNum).MassFlowRate;
        state.dataLoopNodes->Node(ReliefNodeNum).MassFlowRateMaxAvail = state.dataLoopNodes->Node(OAInletNodeNum).MassFlowRate;
    }
    if (zoneEvapUnit.fanPlace == HVAC::FanPlace::BlowThru) {
        state.dataLoopNodes->Node(FanOutletNodeNum).MassFlowRate = state.dataLoopNodes->Node(OAInletNodeNum).MassFlowRate;
        state.dataLoopNodes->Node(FanOutletNodeNum).MassFlowRateMaxAvail = state.dataLoopNodes->Node(OAInletNodeNum).MassFlowRate;
        state.dataFans->fans(zoneEvapUnit.FanIndex)->simulate(state, false, _, _);
    }

    if (zoneEvapUnit.EvapCooler_1_AvailStatus) {
        SimEvapCooler(state, zoneEvapUnit.EvapCooler_1_Name, zoneEvapUnit.EvapCooler_1_Index, PartLoadRatio);
    }

    if ((zoneEvapUnit.EvapCooler_2_Index > 0) && zoneEvapUnit.EvapCooler_2_AvailStatus) {
        SimEvapCooler(state, zoneEvapUnit.EvapCooler_2_Name, zoneEvapUnit.EvapCooler_2_Index, PartLoadRatio);
    }
    if (zoneEvapUnit.fanPlace == HVAC::FanPlace::DrawThru) {
        state.dataFans->fans(zoneEvapUnit.FanIndex)->simulate(state, false, _, _);
    }

    // calculate sensible and latent outputs delivered
    MinHumRat = min(state.dataLoopNodes->Node(ZoneNodeNum).HumRat, state.dataLoopNodes->Node(OutletNodeNum).HumRat);
    SensibleOutputProvided = state.dataLoopNodes->Node(OutletNodeNum).MassFlowRate *
                             (Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(OutletNodeNum).Temp, MinHumRat) -
                              Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(ZoneNodeNum).Temp, MinHumRat));
    LatentOutputProvided = state.dataLoopNodes->Node(OutletNodeNum).MassFlowRate *
                           (state.dataLoopNodes->Node(OutletNodeNum).HumRat - state.dataLoopNodes->Node(ZoneNodeNum).HumRat);
}

void ControlZoneEvapUnitOutput(EnergyPlusData &state,
                               int const UnitNum,           // unit number
                               Real64 const ZoneCoolingLoad // target cooling load
)
{

    // calculates unit cooling part load ratio using root solver numerical method

    // local variables
    int constexpr MaxIte(50);      // maximum number of iterations
    Real64 constexpr Tol(0.01);    // error tolerance
    Real64 PartLoadRatio;          // cooling part load ratio
    Real64 FullFlowSensibleOutput; // full flow sensible cooling output
    Real64 FullFlowLatentOutput;   // full flow sensible cooling output

    auto &zoneEvapUnit = state.dataEvapCoolers->ZoneEvapUnit(UnitNum);

    // get full flow sensible cooling output
    PartLoadRatio = 1.0;
    CalcZoneEvapUnitOutput(state, UnitNum, PartLoadRatio, FullFlowSensibleOutput, FullFlowLatentOutput);

    // calculate part load ratio
    if (FullFlowSensibleOutput < ZoneCoolingLoad) {
        auto f = [&state, UnitNum, ZoneCoolingLoad](Real64 PartLoadRatio) {
            // calculates cooling load residual by varying part load ratio
            Real64 QSensOutputProvided; // sensible output at a given PLR
            Real64 QLatOutputProvided;  // latent output at a given PLR
            CalcZoneEvapUnitOutput(state, UnitNum, PartLoadRatio, QSensOutputProvided, QLatOutputProvided);
            return QSensOutputProvided - ZoneCoolingLoad;
        };
        int SolFla; // Flag of root solver
        General::SolveRoot(state, Tol, MaxIte, SolFla, PartLoadRatio, f, 0.0, 1.0);
        if (SolFla == -1) {
            if (zoneEvapUnit.UnitLoadControlMaxIterErrorIndex == 0) {
                ShowWarningError(state, format("Iteration limit exceeded calculating evap unit part load ratio, for unit={}", zoneEvapUnit.Name));
                ShowContinueErrorTimeStamp(state, "");
                ShowContinueError(state, format("Unit part load ratio returned={:.2R}", PartLoadRatio));
                ShowContinueError(state, "Check input for Fan Placement.");
            }
            ShowRecurringWarningErrorAtEnd(
                state,
                format("Zone Evaporative Cooler unit part load ratio control failed (iteration limit [{}]) for ZoneHVAC:EvaporativeCoolerUnit =\"{}",
                       MaxIte,
                       zoneEvapUnit.Name),
                zoneEvapUnit.UnitLoadControlMaxIterErrorIndex);

        } else if (SolFla == -2) {
            if (zoneEvapUnit.UnitLoadControlLimitsErrorIndex == 0) {
                ShowWarningError(state,
                                 format("Zone Evaporative Cooler unit calculation failed: unit part load ratio limits exceeded, for unit = {}",
                                        zoneEvapUnit.Name));
                ShowContinueError(state, "Check input for Fan Placement.");
                ShowContinueErrorTimeStamp(state, "");
                if (state.dataGlobal->WarmupFlag) {
                    ShowContinueError(state, "Error occurred during warmup days.");
                }
            }
            ShowRecurringWarningErrorAtEnd(
                state,
                "Zone Evaporative Cooler unit part load ratio control failed (limits exceeded) for ZoneHVAC:EvaporativeCoolerUnit =\"" +
                    zoneEvapUnit.Name,
                zoneEvapUnit.UnitLoadControlLimitsErrorIndex);
        }

    } else {
        PartLoadRatio = 1.0;
    }
    zoneEvapUnit.UnitPartLoadRatio = PartLoadRatio;
}

void ControlVSEvapUnitToMeetLoad(EnergyPlusData &state,
                                 int const UnitNum,           // unit number
                                 Real64 const ZoneCoolingLoad // target cooling load
)
{

    // SUBROUTINE PARAMETER DEFINITIONS:
    int constexpr MaxIte(500); // maximum number of iterations
    auto &zoneEvapUnit = state.dataEvapCoolers->ZoneEvapUnit(UnitNum);

    // first get full load result
    zoneEvapUnit.FanSpeedRatio = 1.0;
    state.dataLoopNodes->Node(zoneEvapUnit.OAInletNodeNum).MassFlowRate = zoneEvapUnit.DesignAirMassFlowRate;
    state.dataLoopNodes->Node(zoneEvapUnit.OAInletNodeNum).MassFlowRateMaxAvail = state.dataLoopNodes->Node(zoneEvapUnit.OAInletNodeNum).MassFlowRate;
    state.dataLoopNodes->Node(zoneEvapUnit.UnitOutletNodeNum).MassFlowRate = state.dataLoopNodes->Node(zoneEvapUnit.OAInletNodeNum).MassFlowRate;
    state.dataLoopNodes->Node(zoneEvapUnit.UnitOutletNodeNum).MassFlowRateMaxAvail =
        state.dataLoopNodes->Node(zoneEvapUnit.UnitOutletNodeNum).MassFlowRate;

    if (zoneEvapUnit.UnitReliefNodeNum > 0) {
        state.dataLoopNodes->Node(zoneEvapUnit.UnitReliefNodeNum).MassFlowRate = state.dataLoopNodes->Node(zoneEvapUnit.OAInletNodeNum).MassFlowRate;
        state.dataLoopNodes->Node(zoneEvapUnit.UnitReliefNodeNum).MassFlowRateMaxAvail =
            state.dataLoopNodes->Node(zoneEvapUnit.OAInletNodeNum).MassFlowRate;
    }
    if (zoneEvapUnit.fanPlace == HVAC::FanPlace::BlowThru) {
        state.dataLoopNodes->Node(zoneEvapUnit.FanOutletNodeNum).MassFlowRate = state.dataLoopNodes->Node(zoneEvapUnit.OAInletNodeNum).MassFlowRate;
        state.dataLoopNodes->Node(zoneEvapUnit.FanOutletNodeNum).MassFlowRateMaxAvail =
            state.dataLoopNodes->Node(zoneEvapUnit.OAInletNodeNum).MassFlowRate;
        state.dataFans->fans(zoneEvapUnit.FanIndex)->simulate(state, false, _, _);
    }

    if (zoneEvapUnit.EvapCooler_1_AvailStatus) {
        SimEvapCooler(state, zoneEvapUnit.EvapCooler_1_Name, zoneEvapUnit.EvapCooler_1_Index);
    }

    if ((zoneEvapUnit.EvapCooler_2_Index > 0) && zoneEvapUnit.EvapCooler_2_AvailStatus) {
        SimEvapCooler(state, zoneEvapUnit.EvapCooler_2_Name, zoneEvapUnit.EvapCooler_2_Index);
    }
    if (zoneEvapUnit.fanPlace == HVAC::FanPlace::DrawThru) {
        state.dataFans->fans(zoneEvapUnit.FanIndex)->simulate(state, false, _, _);
    }

    // calculate sensible load met using delta enthalpy at a constant (minimum) humidity ratio)
    Real64 MinHumRat =
        min(state.dataLoopNodes->Node(zoneEvapUnit.ZoneNodeNum).HumRat, state.dataLoopNodes->Node(zoneEvapUnit.UnitOutletNodeNum).HumRat);
    Real64 FullFlowSensibleOutputProvided = state.dataLoopNodes->Node(zoneEvapUnit.UnitOutletNodeNum).MassFlowRate *
                                            (Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(zoneEvapUnit.UnitOutletNodeNum).Temp, MinHumRat) -
                                             Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(zoneEvapUnit.ZoneNodeNum).Temp, MinHumRat));

    if (FullFlowSensibleOutputProvided < ZoneCoolingLoad) { // find speed ratio by regula falsi numerical method
        Real64 FanSpeedRatio = 1.0;
        auto f = [&state, UnitNum, ZoneCoolingLoad](Real64 FanSpeedRatio) {
            auto &unit = state.dataEvapCoolers->ZoneEvapUnit(UnitNum);
            state.dataLoopNodes->Node(unit.OAInletNodeNum).MassFlowRate = unit.DesignAirMassFlowRate * FanSpeedRatio;
            state.dataLoopNodes->Node(unit.OAInletNodeNum).MassFlowRateMaxAvail = state.dataLoopNodes->Node(unit.OAInletNodeNum).MassFlowRate;
            state.dataLoopNodes->Node(unit.UnitOutletNodeNum).MassFlowRate = state.dataLoopNodes->Node(unit.OAInletNodeNum).MassFlowRate;
            state.dataLoopNodes->Node(unit.UnitOutletNodeNum).MassFlowRateMaxAvail = state.dataLoopNodes->Node(unit.UnitOutletNodeNum).MassFlowRate;

            if (unit.UnitReliefNodeNum > 0) {
                state.dataLoopNodes->Node(unit.UnitReliefNodeNum).MassFlowRate = state.dataLoopNodes->Node(unit.OAInletNodeNum).MassFlowRate;
                state.dataLoopNodes->Node(unit.UnitReliefNodeNum).MassFlowRateMaxAvail = state.dataLoopNodes->Node(unit.OAInletNodeNum).MassFlowRate;
            }
            if (unit.fanPlace == HVAC::FanPlace::BlowThru) {
                state.dataLoopNodes->Node(unit.FanOutletNodeNum).MassFlowRate = state.dataLoopNodes->Node(unit.OAInletNodeNum).MassFlowRate;
                state.dataLoopNodes->Node(unit.FanOutletNodeNum).MassFlowRateMaxAvail = state.dataLoopNodes->Node(unit.OAInletNodeNum).MassFlowRate;
                state.dataFans->fans(unit.FanIndex)->simulate(state, false, _, _);
            }

            if (unit.EvapCooler_1_AvailStatus) {
                SimEvapCooler(state, unit.EvapCooler_1_Name, unit.EvapCooler_1_Index, FanSpeedRatio);
            }

            if ((unit.EvapCooler_2_Index > 0) && unit.EvapCooler_2_AvailStatus) {
                SimEvapCooler(state, unit.EvapCooler_2_Name, unit.EvapCooler_2_Index, FanSpeedRatio);
            }
            if (unit.fanPlace == HVAC::FanPlace::DrawThru) {
                state.dataFans->fans(unit.FanIndex)->simulate(state, false, _, _);
            }

            Real64 const MinHumRat =
                min(state.dataLoopNodes->Node(unit.ZoneNodeNum).HumRat, state.dataLoopNodes->Node(unit.UnitOutletNodeNum).HumRat);
            Real64 const SensibleOutputProvided = state.dataLoopNodes->Node(unit.UnitOutletNodeNum).MassFlowRate *
                                                  (Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(unit.UnitOutletNodeNum).Temp, MinHumRat) -
                                                   Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(unit.ZoneNodeNum).Temp, MinHumRat));

            return SensibleOutputProvided - ZoneCoolingLoad;
        };
        int SolFla = 0; // Flag of RegulaFalsi solver
        Real64 ErrorToler = 0.01;
        General::SolveRoot(state, ErrorToler, MaxIte, SolFla, FanSpeedRatio, f, 0.0, 1.0);
        if (SolFla == -1) {
            if (zoneEvapUnit.UnitVSControlMaxIterErrorIndex == 0) {
                ShowWarningError(
                    state, format("Iteration limit exceeded calculating variable speed evap unit fan speed ratio, for unit={}", zoneEvapUnit.Name));
                ShowContinueErrorTimeStamp(state, "");
                ShowContinueError(state, format("Fan speed ratio returned={:.2R}", FanSpeedRatio));
                ShowContinueError(state, "Check input for Fan Placement.");
            }
            ShowRecurringWarningErrorAtEnd(
                state,
                format("Zone Evaporative Cooler unit control failed (iteration limit [{}]) for ZoneHVAC:EvaporativeCoolerUnit =\"{}",
                       MaxIte,
                       zoneEvapUnit.Name),
                zoneEvapUnit.UnitVSControlMaxIterErrorIndex);

        } else if (SolFla == -2) {
            if (zoneEvapUnit.UnitVSControlLimitsErrorIndex == 0) {
                ShowWarningError(state,
                                 format("Variable speed evaporative cooler unit calculation failed: fan speed ratio limits exceeded, for unit = {}",
                                        zoneEvapUnit.Name));
                ShowContinueError(state, "Check input for Fan Placement.");
                ShowContinueErrorTimeStamp(state, "");
                if (state.dataGlobal->WarmupFlag) {
                    ShowContinueError(state, "Error occurred during warmup days.");
                }
            }
            ShowRecurringWarningErrorAtEnd(state,
                                           "Zone Evaporative Cooler unit control failed (limits exceeded) for ZoneHVAC:EvaporativeCoolerUnit =\"" +
                                               zoneEvapUnit.Name,
                                           zoneEvapUnit.UnitVSControlLimitsErrorIndex);
        }
        zoneEvapUnit.FanSpeedRatio = FanSpeedRatio;
    }
}

void ReportZoneEvaporativeCoolerUnit(EnergyPlusData &state, int const UnitNum) // unit number
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   July 2013

    // PURPOSE OF THIS SUBROUTINE:
    // update output variables for the zone evap unit

    // Using/Aliasing
    Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

    auto &zoneEvapUnit = state.dataEvapCoolers->ZoneEvapUnit(UnitNum);

    int ZoneNodeNum = zoneEvapUnit.ZoneNodeNum;
    int UnitOutletNodeNum = zoneEvapUnit.UnitOutletNodeNum;
    Real64 AirMassFlow = state.dataLoopNodes->Node(UnitOutletNodeNum).MassFlowRate;
    Real64 QTotUnitOut = AirMassFlow * (state.dataLoopNodes->Node(UnitOutletNodeNum).Enthalpy - state.dataLoopNodes->Node(ZoneNodeNum).Enthalpy);
    Real64 MinHumRat = min(state.dataLoopNodes->Node(ZoneNodeNum).HumRat, state.dataLoopNodes->Node(UnitOutletNodeNum).HumRat);
    Real64 QSensUnitOut = AirMassFlow * (Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(UnitOutletNodeNum).Temp, MinHumRat) -
                                         Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(ZoneNodeNum).Temp, MinHumRat));

    zoneEvapUnit.UnitTotalCoolingRate = std::abs(min(0.0, QTotUnitOut));
    zoneEvapUnit.UnitTotalCoolingEnergy = zoneEvapUnit.UnitTotalCoolingRate * TimeStepSysSec;
    zoneEvapUnit.UnitSensibleCoolingRate = std::abs(min(0.0, QSensUnitOut));
    zoneEvapUnit.UnitSensibleCoolingEnergy = zoneEvapUnit.UnitSensibleCoolingRate * TimeStepSysSec;
    zoneEvapUnit.UnitLatentHeatingRate = std::abs(max(0.0, (QTotUnitOut - QSensUnitOut)));
    zoneEvapUnit.UnitLatentHeatingEnergy = zoneEvapUnit.UnitLatentHeatingRate * TimeStepSysSec;
    zoneEvapUnit.UnitLatentCoolingRate = std::abs(min(0.0, (QTotUnitOut - QSensUnitOut)));
    zoneEvapUnit.UnitLatentCoolingEnergy = zoneEvapUnit.UnitLatentCoolingRate * TimeStepSysSec;
    zoneEvapUnit.UnitFanSpeedRatio = zoneEvapUnit.FanSpeedRatio;
}

int GetInletNodeNum(EnergyPlusData &state, std::string const &EvapCondName, bool &ErrorsFound)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Lixing Gu
    //       DATE WRITTEN   May 2019

    // PURPOSE OF THIS FUNCTION:
    // This function looks up the given EvapCond and returns the air inlet node number.
    // If incorrect EvapCond name is given, ErrorsFound is returned as true and node number as zero.

    if (state.dataEvapCoolers->GetInputEvapComponentsFlag) { // First time subroutine has been entered
        GetEvapInput(state);
        state.dataEvapCoolers->GetInputEvapComponentsFlag = false;
    }

    int WhichEvapCond =
        Util::FindItemInList(EvapCondName, state.dataEvapCoolers->EvapCond, &EvapConditions::Name, state.dataEvapCoolers->NumEvapCool);
    if (WhichEvapCond != 0) {
        return state.dataEvapCoolers->EvapCond(WhichEvapCond).InletNode;
    } else {
        ShowSevereError(state, format("GetInletNodeNum: Could not find EvaporativeCooler = \"{}\"", EvapCondName));
        ErrorsFound = true;
        return 0;
    }
}

int GetOutletNodeNum(EnergyPlusData &state, std::string const &EvapCondName, bool &ErrorsFound)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Lixing Gu
    //       DATE WRITTEN   May 2019

    // PURPOSE OF THIS FUNCTION:
    // This function looks up the given EvapCond and returns the air outlet node number.
    // If incorrect EvapCond name is given, ErrorsFound is returned as true and node number as zero.

    if (state.dataEvapCoolers->GetInputEvapComponentsFlag) { // First time subroutine has been entered
        GetEvapInput(state);
        state.dataEvapCoolers->GetInputEvapComponentsFlag = false;
    }
    int WhichEvapCond =
        Util::FindItemInList(EvapCondName, state.dataEvapCoolers->EvapCond, &EvapConditions::Name, state.dataEvapCoolers->NumEvapCool);
    if (WhichEvapCond != 0) {
        return state.dataEvapCoolers->EvapCond(WhichEvapCond).OutletNode;
    } else {
        ShowSevereError(state, format("GetOutletNodeNum: Could not find EvaporativeCooler = \"{}\"", EvapCondName));
        ErrorsFound = true;
        return 0;
    }
}

} // namespace EnergyPlus::EvaporativeCoolers
