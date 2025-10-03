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
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/All_Simple_Sizing.hh>
#include <EnergyPlus/Autosizing/SystemAirFlowSizing.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Coils/CoilCoolingDX.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/DXCoils.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HeatRecovery.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/VariableSpeedCoils.hh>

namespace EnergyPlus {

namespace HeatRecovery {

    // Module containing the routines dealing with heat recovery from exhaust or relief air

    // MODULE INFORMATION:
    //       AUTHOR         Michael Wetter
    //       DATE WRITTEN   March 1999
    //       MODIFIED       F Buhl Nov 2000, D Shirey Feb 2003, R. Raustad April 2003
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // To encapsulate the data and routines required to model heat
    // recovery components in the EnergyPlus HVAC simulation

    // METHODOLOGY EMPLOYED:
    // Heat exchanger effectiveness - NTU models are used.

    // REFERENCES:
    // M. Wetter, Simulation Model Air-to-Air Plate Heat Exchanger,LBNL Report 42354, 1999.
    // ARI Standard 1060-2001,Rating Air-to-Air Heat Exchangers for Energy Recovery Ventilation Equipment, www.ari.org
    // ASHRAE Standard 84, Method of Testing Air-To-Air Heat Exchangers, www.ashrae.org
    // U.S. Environmental Protection Agency software "SAVES" -
    //  School Advanced Ventilation Engineering Software http://www.epa.gov/iaq/schooldesign/saves.html

    enum class HXOperation
    {
        Invalid = -1,
        WhenFansOn,
        Scheduled,
        WhenOutsideEconomizerLimits,
        WhenMinOA,
        Num
    };

    Real64 constexpr KELVZERO = 273.16;
    Real64 constexpr SMALL = 1.e-10;
    constexpr std::array<std::string_view, static_cast<int>(FrostControlOption::Num)> frostControlNamesUC = {
        "NONE", "EXHAUSTONLY", "EXHAUSTAIRRECIRCULATION", "MINIMUMEXHAUSTTEMPERATURE"};

    constexpr std::array<std::string_view, static_cast<int>(HXExchConfigType::Num)> hxExchConfigTypeNames = {"Plate", "Rotary"};

    constexpr std::array<std::string_view, static_cast<int>(HXExchConfigType::Num)> hxExchConfigTypeNamesUC = {"PLATE", "ROTARY"};

    constexpr std::array<std::string_view, static_cast<int>(HXOperation::Num)> hxOperationNames = {
        "WhenFansOn", "Scheduled", "WhenOutsideEconomizerLimits", "WhenMinimumOutdoorAir"};

    void SimHeatRecovery(EnergyPlusData &state,
                         std::string_view CompName,                          // name of the heat exchanger unit
                         bool const FirstHVACIteration,                      // TRUE if 1st HVAC simulation of system timestep
                         int &CompIndex,                                     // Pointer to Component
                         HVAC::FanOp const fanOp,                            // Supply air fan operating mode
                         ObjexxFCL::Optional<Real64 const> HXPartLoadRatio,  // Part load ratio requested of DX compressor
                         ObjexxFCL::Optional_bool_const HXUnitEnable,        // Flag to operate heat exchanger
                         ObjexxFCL::Optional_int_const CompanionCoilIndex,   // index of companion cooling coil
                         ObjexxFCL::Optional_bool_const RegenInletIsOANode,  // flag to determine if supply inlet is OA node, if so air flow cycles
                         ObjexxFCL::Optional_bool_const EconomizerFlag,      // economizer operation flag passed by airloop or OA sys
                         ObjexxFCL::Optional_bool_const HighHumCtrlFlag,     // high humidity control flag passed by airloop or OA sys
                         ObjexxFCL::Optional_int_const CompanionCoilType_Num // cooling coil type of coil
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Michael Wetter
        //       DATE WRITTEN   March 1999
        //       MODIFIED       Fred Buhl November 2000, R. Raustad FSEC - Feb 2009
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Manage the simulation of a heat recovery unit

        if (state.dataHeatRecovery->GetInputFlag) {
            GetHeatRecoveryInput(state);
            state.dataHeatRecovery->GetInputFlag = false;
        }

        // Find the correct unit index
        int HeatExchNum; // index of unit being simulated
        if (CompIndex == 0) {
            HeatExchNum = Util::FindItemInList(CompName, state.dataHeatRecovery->ExchCond);
            if (HeatExchNum == 0) {
                ShowFatalError(state, format("SimHeatRecovery: Unit not found={}", CompName));
            }
            CompIndex = HeatExchNum;
        } else {
            HeatExchNum = CompIndex;
            if (HeatExchNum > state.dataHeatRecovery->NumHeatExchangers || HeatExchNum < 1) {
                ShowFatalError(state,
                               format("SimHeatRecovery:  Invalid CompIndex passed={}, Number of Units={}, Entered Unit name={}",
                                      HeatExchNum,
                                      state.dataHeatRecovery->NumHeatExchangers,
                                      CompName));
            }
            if (state.dataHeatRecovery->CheckEquipName(HeatExchNum)) {
                if (CompName != state.dataHeatRecovery->ExchCond(HeatExchNum).Name) {
                    ShowFatalError(state,
                                   format("SimHeatRecovery: Invalid CompIndex passed={}, Unit name={}, stored Unit Name for that index={}",
                                          HeatExchNum,
                                          CompName,
                                          state.dataHeatRecovery->ExchCond(HeatExchNum).Name));
                }
                state.dataHeatRecovery->CheckEquipName(HeatExchNum) = false;
            }
        }

        int CompanionCoilNum = present(CompanionCoilIndex) ? int(CompanionCoilIndex) : -1; // Index to companion cooling coil
        int companionCoilType = present(CompanionCoilType_Num) ? int(CompanionCoilType_Num) : -1;

        bool HXUnitOn; // flag to enable heat exchanger
        if (present(HXUnitEnable)) {
            HXUnitOn = HXUnitEnable;
            //   When state.dataHeatRecovery->CalledFromParentObject is TRUE, this SIM routine was called by a parent object that passed in
            //   HXUnitEnable. HX will use the DX coil part-load ratio (optional CompanionCoilIndex must be present) or PLR passed in if not used with
            //   DX coil (optional CompanionCoilIndex must not be present).
            state.dataHeatRecovery->CalledFromParentObject = true;
        } else {
            //   HX is placed on a BRANCH, optional arguments are not passed in from SimAirServingZones.
            //   HX will calculate its own part-load ratio if optional HXUnitEnable flag is not present
            if (present(HXPartLoadRatio)) {
                HXUnitOn = (HXPartLoadRatio > 0.0);
            } else {
                HXUnitOn = true;
            }
            state.dataHeatRecovery->CalledFromParentObject = false;
        }

        auto &thisExch = state.dataHeatRecovery->ExchCond(HeatExchNum);

        thisExch.initialize(state, CompanionCoilNum, companionCoilType);

        // call the correct heat exchanger calculation routine
        switch (state.dataHeatRecovery->ExchCond(HeatExchNum).type) {
        case HVAC::HXType::AirToAir_FlatPlate: {
            thisExch.CalcAirToAirPlateHeatExch(state, HXUnitOn, EconomizerFlag, HighHumCtrlFlag);
        } break;

        case HVAC::HXType::AirToAir_SensAndLatent: {
            thisExch.CalcAirToAirGenericHeatExch(state, HXUnitOn, FirstHVACIteration, fanOp, EconomizerFlag, HighHumCtrlFlag, HXPartLoadRatio);
        } break;

        case HVAC::HXType::Desiccant_Balanced: {
            Real64 PartLoadRatio = present(HXPartLoadRatio) ? Real64(HXPartLoadRatio) : 1.0; // Part load ratio requested of DX compressor
            bool RegInIsOANode = present(RegenInletIsOANode) && bool(RegenInletIsOANode);
            thisExch.CalcDesiccantBalancedHeatExch(state,
                                                   HXUnitOn,
                                                   FirstHVACIteration,
                                                   fanOp,
                                                   PartLoadRatio,
                                                   CompanionCoilNum,
                                                   companionCoilType,
                                                   RegInIsOANode,
                                                   EconomizerFlag,
                                                   HighHumCtrlFlag);
        } break;

        default: {
            assert(false);
        } break;
        }

        thisExch.UpdateHeatRecovery(state);

        thisExch.ReportHeatRecovery(state);
    }

    void GetHeatRecoveryInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Michael Wetter
        //       DATE WRITTEN   March 1999
        //       MODIFIED       F Buhl Nov 2000, D Shirey Feb 2003, R. Raustad FSEC - Feb 2009 (EconoLockout inputs)
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains input data for heat recovery units and stores it in
        // appropriate data structures.

        // METHODOLOGY EMPLOYED:
        // Uses InputProcessor "Get" routines to obtain data.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlphas;                                                     // Number of Alphas for each GetObjectItem call
        int NumNumbers;                                                    // Number of Numbers for each GetObjectItem call
        int IOStatus;                                                      // Used in GetObjectItem
        bool ErrorsFound(false);                                           // Set to true if errors in input, fatal at end of routine
        constexpr std::string_view RoutineName = "GetHeatRecoveryInput: "; // include trailing blank space
        constexpr std::string_view routineName = "GetHeatRecoveryInput";
        auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;

        int NumAirToAirPlateExchs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "HeatExchanger:AirToAir:FlatPlate");
        int NumAirToAirGenericExchs =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "HeatExchanger:AirToAir:SensibleAndLatent");
        int NumDesiccantBalancedExchs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "HeatExchanger:Desiccant:BalancedFlow");
        int NumDesBalExchsPerfDataType1 =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "HeatExchanger:Desiccant:BalancedFlow:PerformanceDataType1");
        state.dataHeatRecovery->NumHeatExchangers = NumAirToAirPlateExchs + NumAirToAirGenericExchs + NumDesiccantBalancedExchs;

        // allocate the data array
        state.dataHeatRecovery->ExchCond.allocate(state.dataHeatRecovery->NumHeatExchangers);
        state.dataHeatRecovery->HeatExchangerUniqueNames.reserve(state.dataHeatRecovery->NumHeatExchangers);
        state.dataHeatRecovery->CheckEquipName.dimension(state.dataHeatRecovery->NumHeatExchangers, true);

        if (NumDesBalExchsPerfDataType1 > 0) {
            state.dataHeatRecovery->BalDesDehumPerfData.allocate(NumDesBalExchsPerfDataType1);
        }

        // loop over the air to air plate heat exchangers and load their input data
        for (int ExchIndex = 1; ExchIndex <= NumAirToAirPlateExchs; ++ExchIndex) {
            cCurrentModuleObject = "HeatExchanger:AirToAir:FlatPlate";
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     ExchIndex,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)};
            int const ExchNum = ExchIndex;
            auto &thisExchanger = state.dataHeatRecovery->ExchCond(ExchNum);
            thisExchanger.NumericFieldNames.allocate(NumNumbers);
            thisExchanger.NumericFieldNames = state.dataIPShortCut->cNumericFieldNames;

            GlobalNames::VerifyUniqueInterObjectName(state,
                                                     state.dataHeatRecovery->HeatExchangerUniqueNames,
                                                     state.dataIPShortCut->cAlphaArgs(1),
                                                     cCurrentModuleObject,
                                                     state.dataIPShortCut->cAlphaFieldNames(1),
                                                     ErrorsFound);

            thisExchanger.Name = state.dataIPShortCut->cAlphaArgs(1);
            thisExchanger.type = HVAC::HXType::AirToAir_FlatPlate;
            thisExchanger.ExchConfig = HXExchConfigType::Plate;
            if (state.dataIPShortCut->lAlphaFieldBlanks(2)) {
                thisExchanger.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((thisExchanger.availSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(2), state.dataIPShortCut->cAlphaArgs(2));
                ErrorsFound = true;
            }

            constexpr std::array<std::string_view, static_cast<int>(HXConfiguration::Num)> hxConfigurationNamesUC = {
                "COUNTERFLOW", "PARALLELFLOW", "CROSSFLOWBOTHUNMIXED", "CROSS_FLOW_OTHER_NOT_USED"};
            thisExchanger.FlowArr = static_cast<HXConfiguration>(getEnumValue(hxConfigurationNamesUC, state.dataIPShortCut->cAlphaArgs(3)));
            if (thisExchanger.FlowArr == HXConfiguration::Invalid) {
                ShowSevereError(state, format("{}: incorrect flow arrangement: {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(3)));
                ErrorsFound = true;
            }

            if (state.dataIPShortCut->lAlphaFieldBlanks(4)) {
                thisExchanger.EconoLockOut = true;
            } else {
                BooleanSwitch toggle = getYesNoValue(state.dataIPShortCut->cAlphaArgs(4));
                if (toggle == BooleanSwitch::Invalid) {
                    ShowSevereError(state, format("{}: incorrect econo lockout: {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(4)));
                }
                thisExchanger.EconoLockOut = static_cast<bool>(toggle);
            }

            thisExchanger.hARatio = state.dataIPShortCut->rNumericArgs(1);
            thisExchanger.NomSupAirVolFlow = state.dataIPShortCut->rNumericArgs(2);
            thisExchanger.NomSupAirInTemp = state.dataIPShortCut->rNumericArgs(3);
            thisExchanger.NomSupAirOutTemp = state.dataIPShortCut->rNumericArgs(4);
            thisExchanger.NomSecAirVolFlow = state.dataIPShortCut->rNumericArgs(5);
            thisExchanger.NomSecAirInTemp = state.dataIPShortCut->rNumericArgs(6);
            thisExchanger.NomElecPower = state.dataIPShortCut->rNumericArgs(7);
            thisExchanger.SupInletNode = GetOnlySingleNode(state,
                                                           state.dataIPShortCut->cAlphaArgs(5),
                                                           ErrorsFound,
                                                           DataLoopNode::ConnectionObjectType::HeatExchangerAirToAirFlatPlate,
                                                           thisExchanger.Name,
                                                           DataLoopNode::NodeFluidType::Air,
                                                           DataLoopNode::ConnectionType::Inlet,
                                                           NodeInputManager::CompFluidStream::Primary,
                                                           DataLoopNode::ObjectIsNotParent);
            thisExchanger.SupOutletNode = GetOnlySingleNode(state,
                                                            state.dataIPShortCut->cAlphaArgs(6),
                                                            ErrorsFound,
                                                            DataLoopNode::ConnectionObjectType::HeatExchangerAirToAirFlatPlate,
                                                            thisExchanger.Name,
                                                            DataLoopNode::NodeFluidType::Air,
                                                            DataLoopNode::ConnectionType::Outlet,
                                                            NodeInputManager::CompFluidStream::Primary,
                                                            DataLoopNode::ObjectIsNotParent);
            thisExchanger.SecInletNode = GetOnlySingleNode(state,
                                                           state.dataIPShortCut->cAlphaArgs(7),
                                                           ErrorsFound,
                                                           DataLoopNode::ConnectionObjectType::HeatExchangerAirToAirFlatPlate,
                                                           thisExchanger.Name,
                                                           DataLoopNode::NodeFluidType::Air,
                                                           DataLoopNode::ConnectionType::Inlet,
                                                           NodeInputManager::CompFluidStream::Secondary,
                                                           DataLoopNode::ObjectIsNotParent);
            thisExchanger.SecOutletNode = GetOnlySingleNode(state,
                                                            state.dataIPShortCut->cAlphaArgs(8),
                                                            ErrorsFound,
                                                            DataLoopNode::ConnectionObjectType::HeatExchangerAirToAirFlatPlate,
                                                            thisExchanger.Name,
                                                            DataLoopNode::NodeFluidType::Air,
                                                            DataLoopNode::ConnectionType::Outlet,
                                                            NodeInputManager::CompFluidStream::Secondary,
                                                            DataLoopNode::ObjectIsNotParent);

            BranchNodeConnections::TestCompSet(state,
                                               HVAC::hxTypeNames[(int)thisExchanger.type],
                                               thisExchanger.Name,
                                               state.dataIPShortCut->cAlphaArgs(5),
                                               state.dataIPShortCut->cAlphaArgs(6),
                                               "Process Air Nodes");

        } // end of input loop over air to air plate heat exchangers

        // loop over the air to air generic heat exchangers and load their input data
        for (int ExchIndex = 1; ExchIndex <= NumAirToAirGenericExchs; ++ExchIndex) {
            cCurrentModuleObject = "HeatExchanger:AirToAir:SensibleAndLatent";
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     ExchIndex,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)};
            int const ExchNum = ExchIndex + NumAirToAirPlateExchs;
            auto &thisExchanger = state.dataHeatRecovery->ExchCond(ExchNum);
            thisExchanger.NumericFieldNames.allocate(NumNumbers);
            thisExchanger.NumericFieldNames = state.dataIPShortCut->cNumericFieldNames;

            GlobalNames::VerifyUniqueInterObjectName(state,
                                                     state.dataHeatRecovery->HeatExchangerUniqueNames,
                                                     state.dataIPShortCut->cAlphaArgs(1),
                                                     cCurrentModuleObject,
                                                     state.dataIPShortCut->cAlphaFieldNames(1),
                                                     ErrorsFound);

            thisExchanger.Name = state.dataIPShortCut->cAlphaArgs(1);
            thisExchanger.type = HVAC::HXType::AirToAir_SensAndLatent;
            if (state.dataIPShortCut->lAlphaFieldBlanks(2)) {
                thisExchanger.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((thisExchanger.availSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(2), state.dataIPShortCut->cAlphaArgs(2));
                ErrorsFound = true;
            }
            thisExchanger.NomSupAirVolFlow = state.dataIPShortCut->rNumericArgs(1);
            thisExchanger.HeatEffectSensible100 = state.dataIPShortCut->rNumericArgs(2);
            thisExchanger.HeatEffectLatent100 = state.dataIPShortCut->rNumericArgs(3);
            thisExchanger.CoolEffectSensible100 = state.dataIPShortCut->rNumericArgs(4);
            thisExchanger.CoolEffectLatent100 = state.dataIPShortCut->rNumericArgs(5);
            thisExchanger.SupInletNode = GetOnlySingleNode(state,
                                                           state.dataIPShortCut->cAlphaArgs(3),
                                                           ErrorsFound,
                                                           DataLoopNode::ConnectionObjectType::HeatExchangerAirToAirSensibleAndLatent,
                                                           thisExchanger.Name,
                                                           DataLoopNode::NodeFluidType::Air,
                                                           DataLoopNode::ConnectionType::Inlet,
                                                           NodeInputManager::CompFluidStream::Primary,
                                                           DataLoopNode::ObjectIsNotParent);
            thisExchanger.SupOutletNode = GetOnlySingleNode(state,
                                                            state.dataIPShortCut->cAlphaArgs(4),
                                                            ErrorsFound,
                                                            DataLoopNode::ConnectionObjectType::HeatExchangerAirToAirSensibleAndLatent,
                                                            thisExchanger.Name,
                                                            DataLoopNode::NodeFluidType::Air,
                                                            DataLoopNode::ConnectionType::Outlet,
                                                            NodeInputManager::CompFluidStream::Primary,
                                                            DataLoopNode::ObjectIsNotParent);
            thisExchanger.SecInletNode = GetOnlySingleNode(state,
                                                           state.dataIPShortCut->cAlphaArgs(5),
                                                           ErrorsFound,
                                                           DataLoopNode::ConnectionObjectType::HeatExchangerAirToAirSensibleAndLatent,
                                                           thisExchanger.Name,
                                                           DataLoopNode::NodeFluidType::Air,
                                                           DataLoopNode::ConnectionType::Inlet,
                                                           NodeInputManager::CompFluidStream::Secondary,
                                                           DataLoopNode::ObjectIsNotParent);
            thisExchanger.SecOutletNode = GetOnlySingleNode(state,
                                                            state.dataIPShortCut->cAlphaArgs(6),
                                                            ErrorsFound,
                                                            DataLoopNode::ConnectionObjectType::HeatExchangerAirToAirSensibleAndLatent,
                                                            thisExchanger.Name,
                                                            DataLoopNode::NodeFluidType::Air,
                                                            DataLoopNode::ConnectionType::Outlet,
                                                            NodeInputManager::CompFluidStream::Secondary,
                                                            DataLoopNode::ObjectIsNotParent);

            thisExchanger.NomElecPower = state.dataIPShortCut->rNumericArgs(6);

            if (Util::SameString(state.dataIPShortCut->cAlphaArgs(7), "Yes")) {
                thisExchanger.ControlToTemperatureSetPoint = true;
            } else {
                if (!Util::SameString(state.dataIPShortCut->cAlphaArgs(7), "No")) {
                    ShowSevereError(state, "Rotary HX Speed Modulation or Plate Bypass for Temperature Control for ");
                    ShowContinueError(state, format("{} must be set to Yes or No", thisExchanger.Name));
                    ErrorsFound = true;
                }
            }

            thisExchanger.ExchConfig = static_cast<HXExchConfigType>(getEnumValue(hxExchConfigTypeNamesUC, state.dataIPShortCut->cAlphaArgs(8)));

            // Added additional inputs for frost control
            thisExchanger.FrostControlType = static_cast<FrostControlOption>(getEnumValue(frostControlNamesUC, state.dataIPShortCut->cAlphaArgs(9)));

            if (thisExchanger.FrostControlType != FrostControlOption::None) {
                thisExchanger.ThresholdTemperature = state.dataIPShortCut->rNumericArgs(7);
                thisExchanger.InitialDefrostTime = state.dataIPShortCut->rNumericArgs(8);
                thisExchanger.RateofDefrostTimeIncrease = state.dataIPShortCut->rNumericArgs(9);
            }

            if (state.dataIPShortCut->lAlphaFieldBlanks(10)) {
                thisExchanger.EconoLockOut = true;
            } else {
                BooleanSwitch toggle = getYesNoValue(state.dataIPShortCut->cAlphaArgs(10));
                if (toggle == BooleanSwitch::Invalid) {
                    ShowSevereError(state, format("{}: incorrect econo lockout: {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(10)));
                }
                thisExchanger.EconoLockOut = static_cast<bool>(toggle);
            }

            // read new curves here
            thisExchanger.HeatEffectSensibleCurveIndex =
                Curve::GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(11)); // convert curve name to number
            thisExchanger.HeatEffectLatentCurveIndex =
                Curve::GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(12)); // convert curve name to number
            thisExchanger.CoolEffectSensibleCurveIndex =
                Curve::GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(13)); // convert curve name to number
            thisExchanger.CoolEffectLatentCurveIndex =
                Curve::GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(14)); // convert curve name to number

            BranchNodeConnections::TestCompSet(state,
                                               HVAC::hxTypeNames[(int)thisExchanger.type],
                                               thisExchanger.Name,
                                               state.dataIPShortCut->cAlphaArgs(3),
                                               state.dataIPShortCut->cAlphaArgs(4),
                                               "Process Air Nodes");
        } // end of input loop over air to air generic heat exchangers

        // loop over the desiccant balanced heat exchangers and load their input data
        for (int ExchIndex = 1; ExchIndex <= NumDesiccantBalancedExchs; ++ExchIndex) {
            cCurrentModuleObject = "HeatExchanger:Desiccant:BalancedFlow";
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     ExchIndex,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)};

            int const ExchNum = ExchIndex + NumAirToAirPlateExchs + NumAirToAirGenericExchs;
            auto &thisExchanger = state.dataHeatRecovery->ExchCond(ExchNum);
            thisExchanger.NumericFieldNames.allocate(NumNumbers);
            thisExchanger.NumericFieldNames = state.dataIPShortCut->cNumericFieldNames;

            GlobalNames::VerifyUniqueInterObjectName(state,
                                                     state.dataHeatRecovery->HeatExchangerUniqueNames,
                                                     state.dataIPShortCut->cAlphaArgs(1),
                                                     cCurrentModuleObject,
                                                     state.dataIPShortCut->cAlphaFieldNames(1),
                                                     ErrorsFound);

            thisExchanger.Name = state.dataIPShortCut->cAlphaArgs(1);
            thisExchanger.type = HVAC::HXType::Desiccant_Balanced;
            thisExchanger.ExchConfig = HXExchConfigType::Rotary;
            if (state.dataIPShortCut->lAlphaFieldBlanks(2)) {
                thisExchanger.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((thisExchanger.availSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(2), state.dataIPShortCut->cAlphaArgs(2));
                ErrorsFound = true;
            }
            // desiccant HX's usually refer to process and regeneration air streams
            // In this module, Sup = Regeneration nodes and Sec = Process nodes
            // regeneration air inlet and outlet nodes
            thisExchanger.SupInletNode = GetOnlySingleNode(state,
                                                           state.dataIPShortCut->cAlphaArgs(3),
                                                           ErrorsFound,
                                                           DataLoopNode::ConnectionObjectType::HeatExchangerDesiccantBalancedFlow,
                                                           thisExchanger.Name,
                                                           DataLoopNode::NodeFluidType::Air,
                                                           DataLoopNode::ConnectionType::Inlet,
                                                           NodeInputManager::CompFluidStream::Primary,
                                                           DataLoopNode::ObjectIsNotParent);
            thisExchanger.SupOutletNode = GetOnlySingleNode(state,
                                                            state.dataIPShortCut->cAlphaArgs(4),
                                                            ErrorsFound,
                                                            DataLoopNode::ConnectionObjectType::HeatExchangerDesiccantBalancedFlow,
                                                            thisExchanger.Name,
                                                            DataLoopNode::NodeFluidType::Air,
                                                            DataLoopNode::ConnectionType::Outlet,
                                                            NodeInputManager::CompFluidStream::Primary,
                                                            DataLoopNode::ObjectIsNotParent);
            // process air inlet and outlet nodes
            thisExchanger.SecInletNode = GetOnlySingleNode(state,
                                                           state.dataIPShortCut->cAlphaArgs(5),
                                                           ErrorsFound,
                                                           DataLoopNode::ConnectionObjectType::HeatExchangerDesiccantBalancedFlow,
                                                           thisExchanger.Name,
                                                           DataLoopNode::NodeFluidType::Air,
                                                           DataLoopNode::ConnectionType::Inlet,
                                                           NodeInputManager::CompFluidStream::Secondary,
                                                           DataLoopNode::ObjectIsNotParent);
            thisExchanger.SecOutletNode = GetOnlySingleNode(state,
                                                            state.dataIPShortCut->cAlphaArgs(6),
                                                            ErrorsFound,
                                                            DataLoopNode::ConnectionObjectType::HeatExchangerDesiccantBalancedFlow,
                                                            thisExchanger.Name,
                                                            DataLoopNode::NodeFluidType::Air,
                                                            DataLoopNode::ConnectionType::Outlet,
                                                            NodeInputManager::CompFluidStream::Secondary,
                                                            DataLoopNode::ObjectIsNotParent);

            // Set up the component set for the process side of the HX (Sec = Process)
            BranchNodeConnections::TestCompSet(state,
                                               HVAC::hxTypeNames[(int)thisExchanger.type],
                                               thisExchanger.Name,
                                               state.dataLoopNodes->NodeID(thisExchanger.SecInletNode),
                                               state.dataLoopNodes->NodeID(thisExchanger.SecOutletNode),
                                               "Process Air Nodes");

            // A7 is the heat exchanger performance object type
            // It currently only has one choice key, with a default value, so currently no logic is needed
            // In the future if someone added another performance type, the logic could be added back here
            // HeatExchPerfType = state.dataIPShortCut->cAlphaArgs(7);

            thisExchanger.HeatExchPerfName = state.dataIPShortCut->cAlphaArgs(8);

            if (state.dataIPShortCut->lAlphaFieldBlanks(9)) {
                thisExchanger.EconoLockOut = true;
            } else {
                BooleanSwitch toggle = getYesNoValue(state.dataIPShortCut->cAlphaArgs(9));
                if (toggle == BooleanSwitch::Invalid) {
                    ShowSevereError(state, format("{}: incorrect econo lockout: {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(9)));
                }
                thisExchanger.EconoLockOut = static_cast<bool>(toggle);
            }

        } // end of input loop over desiccant balanced heat exchangers

        // get performance data set for balanced desiccant heat exchanger

        for (int PerfDataIndex = 1; PerfDataIndex <= NumDesBalExchsPerfDataType1; ++PerfDataIndex) {
            cCurrentModuleObject = "HeatExchanger:Desiccant:BalancedFlow:PerformanceDataType1";
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     PerfDataIndex,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            int const PerfDataNum = PerfDataIndex;
            auto &thisPerfData = state.dataHeatRecovery->BalDesDehumPerfData(PerfDataNum);
            thisPerfData.NumericFieldNames.allocate(NumNumbers);
            thisPerfData.NumericFieldNames = state.dataIPShortCut->cNumericFieldNames;

            thisPerfData.Name = state.dataIPShortCut->cAlphaArgs(1);
            thisPerfData.PerfType = cCurrentModuleObject;
            thisPerfData.NomSupAirVolFlow = state.dataIPShortCut->rNumericArgs(1);
            // check validity
            if (thisPerfData.NomSupAirVolFlow <= 0.0 && thisPerfData.NomSupAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Nominal air flow rate must be greater than zero.");
                ShowContinueError(state, format("... value entered = {:.6R}", thisPerfData.NomSupAirVolFlow));
                ErrorsFound = true;
            }

            thisPerfData.NomProcAirFaceVel = state.dataIPShortCut->rNumericArgs(2);
            // check validity
            if ((thisPerfData.NomProcAirFaceVel <= 0.0 && thisPerfData.NomProcAirFaceVel != DataSizing::AutoSize) ||
                thisPerfData.NomProcAirFaceVel > 6.0) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Nominal air face velocity cannot be less than or equal to zero or greater than 6 m/s.");
                ShowContinueError(state, format("... value entered = {:.6R}", thisPerfData.NomProcAirFaceVel));
                ErrorsFound = true;
            }
            thisPerfData.NomElecPower = state.dataIPShortCut->rNumericArgs(3);
            // check validity
            if (thisPerfData.NomElecPower < 0.0) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Nominal electric power cannot be less than zero.");
                ShowContinueError(state, format("... value entered = {:.6R}", thisPerfData.NomElecPower));
                ErrorsFound = true;
            }

            // regen outlet temp variables
            for (int i = 0; i < 8; ++i) {
                thisPerfData.B[i] = state.dataIPShortCut->rNumericArgs(i + 4);
            }

            //     Check that the minimum is not greater than or equal to the maximum for each of the following model boundaries
            thisPerfData.T_MinRegenAirInHumRat = state.dataIPShortCut->rNumericArgs(12);
            thisPerfData.T_MaxRegenAirInHumRat = state.dataIPShortCut->rNumericArgs(13);
            if (thisPerfData.T_MinRegenAirInHumRat >= thisPerfData.T_MaxRegenAirInHumRat) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in min/max boundary for the regen outlet air temperature equation.");
                ShowContinueError(state, "... the minimum value of regeneration inlet air humidity ratio must be less than the maximum.");
                ShowContinueError(state, format("... minimum value entered by user = {:.6R}", thisPerfData.T_MinRegenAirInHumRat));
                ShowContinueError(state, format("... maximum value entered by user = {:.6R}", thisPerfData.T_MaxRegenAirInHumRat));
                ErrorsFound = true;
            }
            if (thisPerfData.T_MinRegenAirInHumRat < 0.0) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in min boundary for the regen outlet air temperature equation.");
                ShowContinueError(state, "... the minimum value of regeneration inlet air humidity ratio must be greater than or equal to 0.");
                ShowContinueError(state, format("... minimum value entered by user = {:.6R}", thisPerfData.T_MinRegenAirInHumRat));
                ErrorsFound = true;
            }
            if (thisPerfData.T_MaxRegenAirInHumRat > 1.0) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in max boundary for the regen outlet air temperature equation.");
                ShowContinueError(state, "... the maximum value of regeneration inlet air humidity ratio must be less than or equal to 1.");
                ShowContinueError(state, format("... maximum value entered by user = {:.6R}", thisPerfData.T_MaxRegenAirInHumRat));
                ErrorsFound = true;
            }

            thisPerfData.T_MinRegenAirInTemp = state.dataIPShortCut->rNumericArgs(14);
            thisPerfData.T_MaxRegenAirInTemp = state.dataIPShortCut->rNumericArgs(15);
            if (thisPerfData.T_MinRegenAirInTemp >= thisPerfData.T_MaxRegenAirInTemp) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in min/max boundary for the regen outlet air temperature equation.");
                ShowContinueError(state, "... the minimum value of regeneration inlet air temperature must be less than the maximum.");
                ShowContinueError(state, format("... minimum value entered = {:.6R}", thisPerfData.T_MinRegenAirInTemp));
                ShowContinueError(state, format("... maximum value entered = {:.6R}", thisPerfData.T_MaxRegenAirInTemp));
                ErrorsFound = true;
            }

            thisPerfData.T_MinProcAirInHumRat = state.dataIPShortCut->rNumericArgs(16);
            thisPerfData.T_MaxProcAirInHumRat = state.dataIPShortCut->rNumericArgs(17);
            if (thisPerfData.T_MinProcAirInHumRat >= thisPerfData.T_MaxProcAirInHumRat) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in min/max boundary for the regen outlet air temperature equation.");
                ShowContinueError(state, "... the minimum value of process inlet air humidity ratio must be less than the maximum.");
                ShowContinueError(state, format("... minimum value entered by user = {:.6R}", thisPerfData.T_MinProcAirInHumRat));
                ShowContinueError(state, format("... maximum value entered by user = {:.6R}", thisPerfData.T_MaxProcAirInHumRat));
                ErrorsFound = true;
            }
            if (thisPerfData.T_MinProcAirInHumRat < 0.0) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in min boundary for the regen outlet air temperature equation.");
                ShowContinueError(state, "... the minimum value of process inlet air humidity ratio must be greater than or equal to 0.");
                ShowContinueError(state, format("... minimum value entered by user = {:.6R}", thisPerfData.T_MinProcAirInHumRat));
                ErrorsFound = true;
            }
            if (thisPerfData.T_MaxProcAirInHumRat > 1.0) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in max boundary for the regen outlet air temperature equation.");
                ShowContinueError(state, "... the maximum value of process inlet air humidity ratio must be less than or equal to 1.");
                ShowContinueError(state, format("... maximum value entered by user = {:.6R}", thisPerfData.T_MaxProcAirInHumRat));
                ErrorsFound = true;
            }

            thisPerfData.T_MinProcAirInTemp = state.dataIPShortCut->rNumericArgs(18);
            thisPerfData.T_MaxProcAirInTemp = state.dataIPShortCut->rNumericArgs(19);
            if (thisPerfData.T_MinProcAirInTemp >= thisPerfData.T_MaxProcAirInTemp) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in min/max boundary for the regen outlet air temperature equation.");
                ShowContinueError(state, "... the minimum value of process inlet air temperature must be less than the maximum.");
                ShowContinueError(state, format("... minimum value entered = {:.6R}", thisPerfData.T_MinProcAirInTemp));
                ShowContinueError(state, format("... maximum value entered = {:.6R}", thisPerfData.T_MaxProcAirInTemp));
                ErrorsFound = true;
            }

            thisPerfData.T_MinFaceVel = state.dataIPShortCut->rNumericArgs(20);
            thisPerfData.T_MaxFaceVel = state.dataIPShortCut->rNumericArgs(21);
            if (thisPerfData.T_MinFaceVel >= thisPerfData.T_MaxFaceVel) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in min/max boundary for the regen outlet air temperature equation.");
                ShowContinueError(state, "... the minimum value of regen air velocity must be less than the maximum.");
                ShowContinueError(state, format("... minimum value entered = {:.6R}", thisPerfData.T_MinFaceVel));
                ShowContinueError(state, format("... maximum value entered = {:.6R}", thisPerfData.T_MaxFaceVel));
                ErrorsFound = true;
            }

            thisPerfData.MinRegenAirOutTemp = state.dataIPShortCut->rNumericArgs(22);
            thisPerfData.MaxRegenAirOutTemp = state.dataIPShortCut->rNumericArgs(23);
            if (thisPerfData.MinRegenAirOutTemp >= thisPerfData.MaxRegenAirOutTemp) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in min/max boundary for the regen outlet air temperature equation.");
                ShowContinueError(state, "... the minimum value of regen outlet air temperature must be less than the maximum.");
                ShowContinueError(state, format("... minimum value entered = {:.6R}", thisPerfData.MinRegenAirOutTemp));
                ShowContinueError(state, format("... maximum value entered = {:.6R}", thisPerfData.MaxRegenAirOutTemp));
                ErrorsFound = true;
            }

            thisPerfData.T_MinRegenAirInRelHum = state.dataIPShortCut->rNumericArgs(24) / 100.0;
            thisPerfData.T_MaxRegenAirInRelHum = state.dataIPShortCut->rNumericArgs(25) / 100.0;
            if (thisPerfData.T_MinRegenAirInRelHum >= thisPerfData.T_MaxRegenAirInRelHum) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in min/max boundary for the regen outlet air temperature equation.");
                ShowContinueError(state, "... the minimum value of regen inlet air relative humidity must be less than the maximum.");
                ShowContinueError(state, format("... minimum value entered = {:.6R}", thisPerfData.T_MinRegenAirInRelHum * 100.0));
                ShowContinueError(state, format("... maximum value entered = {:.6R}", thisPerfData.T_MaxRegenAirInRelHum * 100.0));
                ErrorsFound = true;
            }
            if (thisPerfData.T_MinRegenAirInRelHum < 0.0) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in min boundary for the regen outlet air temperature equation.");
                ShowContinueError(state, "... the minimum value of regen inlet air relative humidity must be greater than or equal to 0.");
                ShowContinueError(state, format("... minimum value entered = {:.6R}", thisPerfData.T_MinRegenAirInRelHum * 100.0));
                ErrorsFound = true;
            }
            if (thisPerfData.T_MaxRegenAirInRelHum > 1.0) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in max boundary for the regen outlet air temperature equation.");
                ShowContinueError(state, "... the maximum value of regen inlet air relative humidity must be less than or equal to 100.");
                ShowContinueError(state, format("... maximum value entered = {:.6R}", thisPerfData.T_MaxRegenAirInRelHum * 100.0));
                ErrorsFound = true;
            }

            thisPerfData.T_MinProcAirInRelHum = state.dataIPShortCut->rNumericArgs(26) / 100.0;
            thisPerfData.T_MaxProcAirInRelHum = state.dataIPShortCut->rNumericArgs(27) / 100.0;
            if (thisPerfData.T_MinProcAirInRelHum >= thisPerfData.T_MaxProcAirInRelHum) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in min/max boundary for the regen outlet air temperature equation.");
                ShowContinueError(state, "... the minimum value of process inlet air relative humidity must be less than the maximum.");
                ShowContinueError(state, format("... minimum value entered = {:.6R}", thisPerfData.T_MinProcAirInRelHum * 100.0));
                ShowContinueError(state, format("... maximum value entered = {:.6R}", thisPerfData.T_MaxProcAirInRelHum * 100.0));
                ErrorsFound = true;
            }
            if (thisPerfData.T_MinProcAirInRelHum < 0.0) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in min boundary for the regen outlet air temperature equation.");
                ShowContinueError(state, "... the minimum value of process inlet air relative humidity must be greater than or equal to 0.");
                ShowContinueError(state, format("... minimum value entered = {:.6R}", thisPerfData.T_MinProcAirInRelHum * 100.0));
                ErrorsFound = true;
            }
            if (thisPerfData.T_MaxProcAirInRelHum > 1.0) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in max boundary for the regen outlet air temperature equation.");
                ShowContinueError(state, "... the maximum value of process inlet air relative humidity must be less than or equal to 100.");
                ShowContinueError(state, format("... maximum value entered = {:.6R}", thisPerfData.T_MaxProcAirInRelHum * 100.0));
                ErrorsFound = true;
            }

            // regen outlet humidity ratio variables
            for (int i = 0; i < 8; ++i) {
                thisPerfData.C[i] = state.dataIPShortCut->rNumericArgs(i + 28);
            }

            //     Check that the minimum is not greater than or equal to the maximum for each of the following model boundaries
            thisPerfData.H_MinRegenAirInHumRat = state.dataIPShortCut->rNumericArgs(36);
            thisPerfData.H_MaxRegenAirInHumRat = state.dataIPShortCut->rNumericArgs(37);
            if (thisPerfData.H_MinRegenAirInHumRat >= thisPerfData.H_MaxRegenAirInHumRat) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in min/max boundary for the regen outlet air humidity ratio equation.");
                ShowContinueError(state, "... the minimum value of regeneration inlet air humidity ratio must be less than the maximum.");
                ShowContinueError(state, format("... minimum value entered by user = {:.6R}", thisPerfData.H_MinRegenAirInHumRat));
                ShowContinueError(state, format("... maximum value entered by user = {:.6R}", thisPerfData.H_MaxRegenAirInHumRat));
                ErrorsFound = true;
            }
            if (thisPerfData.H_MinRegenAirInHumRat < 0.0) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in min boundary for the regen outlet air humidity ratio equation.");
                ShowContinueError(state, "... the minimum value of regeneration inlet air humidity ratio must be greater than or equal to 0.");
                ShowContinueError(state, format("... minimum value entered by user = {:.6R}", thisPerfData.H_MinRegenAirInHumRat));
                ErrorsFound = true;
            }
            if (thisPerfData.H_MaxRegenAirInHumRat > 1.0) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in max boundary for the regen outlet air humidity ratio equation.");
                ShowContinueError(state, "... the maximum value of regeneration inlet air humidity ratio must be less than or equal to 1.");
                ShowContinueError(state, format("... maximum value entered by user = {:.6R}", thisPerfData.H_MaxRegenAirInHumRat));
                ErrorsFound = true;
            }

            thisPerfData.H_MinRegenAirInTemp = state.dataIPShortCut->rNumericArgs(38);
            thisPerfData.H_MaxRegenAirInTemp = state.dataIPShortCut->rNumericArgs(39);
            if (thisPerfData.H_MinRegenAirInTemp >= thisPerfData.H_MaxRegenAirInTemp) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in min/max boundary for the regen outlet air humidity ratio equation.");
                ShowContinueError(state, "... the minimum value of regeneration inlet air temperature must be less than the maximum.");
                ShowContinueError(state, format("... minimum value entered = {:.6R}", thisPerfData.H_MinRegenAirInTemp));
                ShowContinueError(state, format("... maximum value entered = {:.6R}", thisPerfData.H_MaxRegenAirInTemp));
                ErrorsFound = true;
            }

            thisPerfData.H_MinProcAirInHumRat = state.dataIPShortCut->rNumericArgs(40);
            thisPerfData.H_MaxProcAirInHumRat = state.dataIPShortCut->rNumericArgs(41);
            if (thisPerfData.H_MinProcAirInHumRat >= thisPerfData.H_MaxProcAirInHumRat) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in min/max boundary for the regen outlet air humidity ratio equation.");
                ShowContinueError(state, "... the minimum value of process inlet air humidity ratio must be less than the maximum.");
                ShowContinueError(state, format("... minimum value entered by user = {:.6R}", thisPerfData.H_MinProcAirInHumRat));
                ShowContinueError(state, format("... maximum value entered by user = {:.6R}", thisPerfData.H_MaxProcAirInHumRat));
                ErrorsFound = true;
            }
            if (thisPerfData.H_MinProcAirInHumRat < 0.0) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in min boundary for the regen outlet air humidity ratio equation.");
                ShowContinueError(state, "... the minimum value of process inlet air humidity ratio must be greater than or equal to 0.");
                ShowContinueError(state, format("... minimum value entered by user = {:.6R}", thisPerfData.H_MinProcAirInHumRat));
                ErrorsFound = true;
            }
            if (thisPerfData.H_MaxProcAirInHumRat > 1.0) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in max boundary for the regen outlet air humidity ratio equation.");
                ShowContinueError(state, "... the maximum value of process inlet air humidity ratio must be less than or equal to 1.");
                ShowContinueError(state, format("... maximum value entered by user = {:.6R}", thisPerfData.H_MaxProcAirInHumRat));
                ErrorsFound = true;
            }

            thisPerfData.H_MinProcAirInTemp = state.dataIPShortCut->rNumericArgs(42);
            thisPerfData.H_MaxProcAirInTemp = state.dataIPShortCut->rNumericArgs(43);
            if (thisPerfData.H_MinProcAirInTemp >= thisPerfData.H_MaxProcAirInTemp) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in min/max boundary for the regen outlet air humidity ratio equation.");
                ShowContinueError(state, "... the minimum value of process inlet air temperature must be less than the maximum.");
                ShowContinueError(state, format("... minimum value entered = {:.6R}", thisPerfData.H_MinProcAirInTemp));
                ShowContinueError(state, format("... maximum value entered = {:.6R}", thisPerfData.H_MaxProcAirInTemp));
                ErrorsFound = true;
            }

            thisPerfData.H_MinFaceVel = state.dataIPShortCut->rNumericArgs(44);
            thisPerfData.H_MaxFaceVel = state.dataIPShortCut->rNumericArgs(45);
            if (thisPerfData.H_MinFaceVel >= thisPerfData.H_MaxFaceVel) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in min/max boundary for the regen outlet air humidity ratio equation.");
                ShowContinueError(state, "... the minimum value of regen air velocity must be less than the maximum.");
                ShowContinueError(state, format("... minimum value entered = {:.6R}", thisPerfData.H_MinFaceVel));
                ShowContinueError(state, format("... maximum value entered = {:.6R}", thisPerfData.H_MaxFaceVel));
                ErrorsFound = true;
            }

            thisPerfData.MinRegenAirOutHumRat = state.dataIPShortCut->rNumericArgs(46);
            thisPerfData.MaxRegenAirOutHumRat = state.dataIPShortCut->rNumericArgs(47);
            if (thisPerfData.MinRegenAirOutHumRat >= thisPerfData.MaxRegenAirOutHumRat) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in min/max boundary for the regen outlet air humidity ratio equation.");
                ShowContinueError(state, "... the minimum value of regen outlet air humidity ratio must be less than the maximum.");
                ShowContinueError(state, format("... minimum value entered = {:.6R}", thisPerfData.MinRegenAirOutHumRat));
                ShowContinueError(state, format("... maximum value entered = {:.6R}", thisPerfData.MaxRegenAirOutHumRat));
                ErrorsFound = true;
            }
            if (thisPerfData.MinRegenAirOutHumRat < 0.0) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in min boundary for the regen outlet air humidity ratio equation.");
                ShowContinueError(state, "... the minimum value of regen outlet air humidity ratio must be greater than or equal to 0.");
                ShowContinueError(state, format("... minimum value entered = {:.6R}", thisPerfData.MinRegenAirOutHumRat));
                ErrorsFound = true;
            }
            if (thisPerfData.MaxRegenAirOutHumRat > 1.0) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in max boundary for the regen outlet air humidity ratio equation.");
                ShowContinueError(state, "... the maximum value of regen outlet air humidity ratio must be less or equal to 1.");
                ShowContinueError(state, format("... maximum value entered = {:.6R}", thisPerfData.MaxRegenAirOutHumRat));
                ErrorsFound = true;
            }

            thisPerfData.H_MinRegenAirInRelHum = state.dataIPShortCut->rNumericArgs(48) / 100.0;
            thisPerfData.H_MaxRegenAirInRelHum = state.dataIPShortCut->rNumericArgs(49) / 100.0;
            if (thisPerfData.H_MinRegenAirInRelHum >= thisPerfData.H_MaxRegenAirInRelHum) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in min/max boundary for the regen outlet air humidity ratio equation.");
                ShowContinueError(state, "... the minimum value of regen inlet air relative humidity must be less than the maximum.");
                ShowContinueError(state, format("... minimum value entered = {:.6R}", thisPerfData.H_MinRegenAirInRelHum * 100.0));
                ShowContinueError(state, format("... maximum value entered = {:.6R}", thisPerfData.H_MaxRegenAirInRelHum * 100.0));
                ErrorsFound = true;
            }
            if (thisPerfData.H_MinRegenAirInRelHum < 0.0) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in min boundary for the regen outlet air humidity ratio equation.");
                ShowContinueError(state, "... the minimum value of regen inlet air relative humidity must be greater than or equal to 0.");
                ShowContinueError(state, format("... minimum value entered = {:.6R}", thisPerfData.H_MinRegenAirInRelHum * 100.0));
                ErrorsFound = true;
            }
            if (thisPerfData.H_MaxRegenAirInRelHum > 1.0) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in max boundary for the regen outlet air humidity ratio equation.");
                ShowContinueError(state, "... the maximum value of regen inlet air relative humidity must be less or equal to 100.");
                ShowContinueError(state, format("... maximum value entered = {:.6R}", thisPerfData.H_MaxRegenAirInRelHum * 100.0));
                ErrorsFound = true;
            }

            thisPerfData.H_MinProcAirInRelHum = state.dataIPShortCut->rNumericArgs(50) / 100.0;
            thisPerfData.H_MaxProcAirInRelHum = state.dataIPShortCut->rNumericArgs(51) / 100.0;
            if (thisPerfData.H_MinProcAirInRelHum >= thisPerfData.H_MaxProcAirInRelHum) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in min/max boundary for the regen outlet air humidity ratio equation.");
                ShowContinueError(state, "... the minimum value of process inlet air relative humidity must be less than the maximum.");
                ShowContinueError(state, format("... minimum value entered = {:.6R}", thisPerfData.H_MinProcAirInRelHum * 100.0));
                ShowContinueError(state, format("... maximum value entered = {:.6R}", thisPerfData.H_MaxProcAirInRelHum * 100.0));
                ErrorsFound = true;
            }
            if (thisPerfData.H_MinProcAirInRelHum < 0.0) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in min boundary for the regen outlet air humidity ratio equation.");
                ShowContinueError(state, "... the minimum value of process inlet air relative humidity must be greater than or equal to 0.");
                ShowContinueError(state, format("... minimum value entered = {:.6R}", thisPerfData.H_MinProcAirInRelHum * 100.0));
                ErrorsFound = true;
            }
            if (thisPerfData.H_MaxProcAirInRelHum > 1.0) {
                ShowSevereError(state, format("{} \"{}\"", cCurrentModuleObject, thisPerfData.Name));
                ShowContinueError(state, "Error found in max boundary for the regen outlet air humidity ratio equation.");
                ShowContinueError(state, "... the maximum value of process inlet air relative humidity must be less than or equal to 100.");
                ShowContinueError(state, format("... maximum value entered = {:.6R}", thisPerfData.H_MaxProcAirInRelHum * 100.0));
                ErrorsFound = true;
            }
        }
        // getting performance data set for balanced desiccant heat exchanger ends

        // match desiccant heat exchanger index to performance data index
        for (int ExchIndex = 1; ExchIndex <= NumDesiccantBalancedExchs; ++ExchIndex) {
            int const ExchNum = ExchIndex + NumAirToAirPlateExchs + NumAirToAirGenericExchs;
            auto &thisExchanger = state.dataHeatRecovery->ExchCond(ExchNum);
            for (int PerfDataNum = 1; PerfDataNum <= NumDesBalExchsPerfDataType1; ++PerfDataNum) {
                if (Util::SameString(thisExchanger.HeatExchPerfName, state.dataHeatRecovery->BalDesDehumPerfData(PerfDataNum).Name)) {
                    thisExchanger.PerfDataIndex = PerfDataNum;
                    break;
                }
            }
            if (thisExchanger.PerfDataIndex == 0) {
                ShowSevereError(state, format("{} \"{}\"", HVAC::hxTypeNames[(int)thisExchanger.type], thisExchanger.Name));
                ShowContinueError(state, format("... Performance data set not found = {}", thisExchanger.HeatExchPerfName));
                ErrorsFound = true;
            } else {
                if (!ErrorsFound) {
                    thisExchanger.FaceArea = state.dataHeatRecovery->BalDesDehumPerfData(thisExchanger.PerfDataIndex).NomSupAirVolFlow /
                                             (state.dataHeatRecovery->BalDesDehumPerfData(thisExchanger.PerfDataIndex).NomProcAirFaceVel);
                }
            }
        }
        // matching done

        // setup common report variables for heat exchangers
        for (int ExchIndex = 1; ExchIndex <= state.dataHeatRecovery->NumHeatExchangers; ++ExchIndex) {
            int const ExchNum = ExchIndex;
            auto &thisExchanger = state.dataHeatRecovery->ExchCond(ExchNum);
            // CurrentModuleObject='HeatExchanger:AirToAir:FlatPlate/AirToAir:SensibleAndLatent/Desiccant:BalancedFlow')
            SetupOutputVariable(state,
                                "Heat Exchanger Sensible Heating Rate",
                                Constant::Units::W,
                                thisExchanger.SensHeatingRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisExchanger.Name);
            SetupOutputVariable(state,
                                "Heat Exchanger Sensible Heating Energy",
                                Constant::Units::J,
                                thisExchanger.SensHeatingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisExchanger.Name);
            SetupOutputVariable(state,
                                "Heat Exchanger Latent Gain Rate",
                                Constant::Units::W,
                                thisExchanger.LatHeatingRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisExchanger.Name);
            SetupOutputVariable(state,
                                "Heat Exchanger Latent Gain Energy",
                                Constant::Units::J,
                                thisExchanger.LatHeatingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisExchanger.Name);
            SetupOutputVariable(state,
                                "Heat Exchanger Total Heating Rate",
                                Constant::Units::W,
                                thisExchanger.TotHeatingRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisExchanger.Name);
            SetupOutputVariable(state,
                                "Heat Exchanger Total Heating Energy",
                                Constant::Units::J,
                                thisExchanger.TotHeatingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisExchanger.Name,
                                Constant::eResource::EnergyTransfer,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::HeatRecoveryForHeating);
            SetupOutputVariable(state,
                                "Heat Exchanger Sensible Cooling Rate",
                                Constant::Units::W,
                                thisExchanger.SensCoolingRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisExchanger.Name);
            SetupOutputVariable(state,
                                "Heat Exchanger Sensible Cooling Energy",
                                Constant::Units::J,
                                thisExchanger.SensCoolingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisExchanger.Name);
            SetupOutputVariable(state,
                                "Heat Exchanger Latent Cooling Rate",
                                Constant::Units::W,
                                thisExchanger.LatCoolingRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisExchanger.Name);
            SetupOutputVariable(state,
                                "Heat Exchanger Latent Cooling Energy",
                                Constant::Units::J,
                                thisExchanger.LatCoolingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisExchanger.Name);
            SetupOutputVariable(state,
                                "Heat Exchanger Total Cooling Rate",
                                Constant::Units::W,
                                thisExchanger.TotCoolingRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisExchanger.Name);
            SetupOutputVariable(state,
                                "Heat Exchanger Total Cooling Energy",
                                Constant::Units::J,
                                thisExchanger.TotCoolingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisExchanger.Name,
                                Constant::eResource::EnergyTransfer,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::HeatRecoveryForCooling);

            SetupOutputVariable(state,
                                "Heat Exchanger Electricity Rate",
                                Constant::Units::W,
                                thisExchanger.ElecUseRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisExchanger.Name);
            SetupOutputVariable(state,
                                "Heat Exchanger Electricity Energy",
                                Constant::Units::J,
                                thisExchanger.ElecUseEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisExchanger.Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::HeatRecovery);
        }

        // setup additional report variables for generic heat exchangers
        for (int ExchIndex = 1; ExchIndex <= NumAirToAirGenericExchs; ++ExchIndex) {
            // generic heat exchangers are read in after flat plate heat exchanger objects (index needs to be set correctly)
            // CurrentModuleObject=HeatExchanger:AirToAir:SensibleAndLatent
            int const ExchNum = ExchIndex + NumAirToAirPlateExchs;
            auto &thisExchanger = state.dataHeatRecovery->ExchCond(ExchNum);
            SetupOutputVariable(state,
                                "Heat Exchanger Sensible Effectiveness",
                                Constant::Units::None,
                                thisExchanger.SensEffectiveness,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisExchanger.Name);
            SetupOutputVariable(state,
                                "Heat Exchanger Latent Effectiveness",
                                Constant::Units::None,
                                thisExchanger.LatEffectiveness,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisExchanger.Name);
            SetupOutputVariable(state,
                                "Heat Exchanger Supply Air Bypass Mass Flow Rate",
                                Constant::Units::kg_s,
                                thisExchanger.SupBypassMassFlow,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisExchanger.Name);
            SetupOutputVariable(state,
                                "Heat Exchanger Exhaust Air Bypass Mass Flow Rate",
                                Constant::Units::kg_s,
                                thisExchanger.SecBypassMassFlow,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisExchanger.Name);
            SetupOutputVariable(state,
                                "Heat Exchanger Defrost Time Fraction",
                                Constant::Units::None,
                                thisExchanger.DefrostFraction,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisExchanger.Name);
        }

        if (ErrorsFound) {
            ShowFatalError(state, format("{}Errors found in input.  Program terminates.", RoutineName));
        }
    }

    void HeatExchCond::initialize(EnergyPlusData &state, int const CompanionCoilIndex, int const CompanionCoilType_Num)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Michael Wetter
        //       DATE WRITTEN   March 1999
        //       MODIFIED       F Buhl Nov 2000, D Shirey Feb 2003
        //                      B Griffith May 2009, EMS setpoint check
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the Heat Recovery Components.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 CMin0;  // minimum capacity flow
        Real64 CMax0;  // maximum capacity flow
        Real64 Eps0;   // effectiveness at rated conditions
        Real64 NTU0;   // NTU at rated conditions
        Real64 RhoAir; // air density at outside pressure & standard temperature and humidity
        Real64 CpAir;  // heat capacity of air
        // of humidity ratio and temperature
        Real64 Z; // Min/max flow ratio

        if (!state.dataGlobal->SysSizingCalc && this->MySizeFlag) {
            this->size(state);
            this->MySizeFlag = false;
        }

        // Do the Begin Environment initializations
        if (state.dataGlobal->BeginEnvrnFlag && this->myEnvrnFlag) {
            bool FatalError = false;
            // I believe that all of these initializations should be taking place at the SCFM conditions
            RhoAir = state.dataEnvrn->StdRhoAir;
            //    RhoAir = PsyRhoAirFnPbTdbW(101325.0,20.0,0.0)  do we want standard air density at sea level for generic ERVs per ARI 1060?
            CpAir = Psychrometrics::PsyCpAirFnW(0.0);

            CalculateNTUBoundsErrors ErrStat = CalculateNTUBoundsErrors::NoError;
            switch (this->type) {
            case HVAC::HXType::AirToAir_FlatPlate:
                this->NomSupAirMassFlow = RhoAir * this->NomSupAirVolFlow;
                this->NomSecAirMassFlow = RhoAir * this->NomSecAirVolFlow;
                // Note: the capacity stream is here simply the mass flow
                //       since the thermal capacity can be assumed to be
                //       equal for both streams
                if (this->NomSupAirMassFlow > this->NomSecAirMassFlow) {
                    CMin0 = this->NomSecAirMassFlow;
                    CMax0 = this->NomSupAirMassFlow;
                } else {
                    CMin0 = this->NomSupAirMassFlow;
                    CMax0 = this->NomSecAirMassFlow;
                }

                Eps0 = this->NomSupAirMassFlow *
                       SafeDiv(this->NomSupAirOutTemp - this->NomSupAirInTemp, CMin0 * (this->NomSecAirInTemp - this->NomSupAirInTemp));
                Z = CMin0 / CMax0;

                CalculateNTUfromEpsAndZ(state, NTU0, ErrStat, Z, this->FlowArr, Eps0);

                switch (ErrStat) {
                case CalculateNTUBoundsErrors::NoError:
                    break; // great!
                case CalculateNTUBoundsErrors::MassFlowRatio:
                    FatalError = true;
                    ShowSevereError(state, format("In the HeatExchanger:AirToAir:FlatPlate component {}", this->Name));
                    ShowContinueError(state, "  the mass flow ratio is out of bounds");
                    ShowContinueError(state, format("The mass flow ratio is (Min_Mass_Flow_Rate / Max_Mass_Flow_Rate) = {:.2R}", Z));
                    ShowContinueError(state, "The mass flow ratio should be >= 0.0 and <= 1.0");
                    ShowContinueError(state,
                                      format("Min_Mass_Flow_Rate = {:.2R} [air density] * {:.1R} [Min_Vol_Flow_Rate]",
                                             RhoAir,
                                             min(this->NomSupAirVolFlow, this->NomSecAirVolFlow)));
                    ShowContinueError(state,
                                      format("Max_Mass_Flow_Rate = {:.2R} [air density] * {:.1R} [Max_Vol_Flow_Rate]",
                                             RhoAir,
                                             max(this->NomSupAirVolFlow, this->NomSecAirVolFlow)));
                    break;
                case CalculateNTUBoundsErrors::NominalEffectiveness1:
                    FatalError = true;
                    ShowSevereError(state, format("In the HeatExchanger:AirToAir:FlatPlate component {}", this->Name));
                    ShowContinueError(state, "  the calculated nominal effectiveness is out of bounds");
                    ShowContinueError(state, format("The effectiveness is {:.3R}", Eps0));
                    ShowContinueError(state, format("The effectiveness should be >= 0.0 and <= {:.3R}", 1.0 / (1.0 + Z)));
                    ShowContinueError(state,
                                      "Eff = (Nom_Sup_Mass_Flow_Rate/Min_Mass_Flow_Rate)*(T_nom_sup_out-T_nom_sup_in)/(T_nom_sec_in-T_nom_sup_in)");
                    ShowContinueError(state, "The temperatures are user inputs. The mass flow rates are user input volume flow rates");
                    ShowContinueError(state, format("  times the density of air [{:.2R} kg/m3]", RhoAir));
                    ShowContinueError(state, "Change these inputs to obtain a physically realizable heat exchanger effectiveness");
                    break;
                case CalculateNTUBoundsErrors::NominalEffectiveness2:
                    FatalError = true;
                    ShowSevereError(state, format("In the HeatExchanger:AirToAir:FlatPlate component {}", this->Name));
                    ShowContinueError(state, "  the calculated nominal effectiveness is out of bounds");
                    ShowContinueError(state, format("The effectiveness is {:.3R}", Eps0));
                    ShowContinueError(state, format("The effectiveness should be >= 0.0 and <= {:.3R}", (1.0 - std::exp(-Z)) / Z));
                    ShowContinueError(state,
                                      "Eff = (Nom_Sup_Mass_Flow_Rate/Min_Mass_Flow_Rate)*(T_nom_sup_out-T_nom_sup_in)/(T_nom_sec_in-T_nom_sup_in)");
                    ShowContinueError(state, "The temperatures are user inputs. The mass flow rates are user input volume flow rates");
                    ShowContinueError(state, format("  times the density of air [{:.2R} kg/m3]", RhoAir));
                    ShowContinueError(state, "Change these inputs to obtain a physically realizable heat exchanger effectiveness");
                    break;
                case CalculateNTUBoundsErrors::Quantity:
                    FatalError = true;
                    ShowSevereError(state, format("In the HeatExchanger:AirToAir:FlatPlate component {}", this->Name));
                    ShowContinueError(state, "  the quantity Eff_nom*(Min_Mass_Flow_Rate / Max_Mass_Flow_Rate) is out of bounds");
                    ShowContinueError(state, format("The value is {:.3R}", Eps0 * Z));
                    ShowContinueError(state, format("The value should be >= 0.0 and <= {:.3R}", 1.0 - std::exp(Z * (SMALL - 1.0))));
                    ShowContinueError(
                        state,
                        "Eff_nom = (Nom_Sup_Mass_Flow_Rate/Min_Mass_Flow_Rate) * (T_nom_sup_out - T_nom_sup_in)/(T_nom_sec_in - T_nom_sup_in)");
                    ShowContinueError(state, "The temperatures are user inputs. The mass flow rates are user input volume flow rates");
                    ShowContinueError(state, format("  times the density of air [{:.2R} kg/m3]", RhoAir));
                    ShowContinueError(state,
                                      "Change these inputs to obtain a physically realizable product of effectiveness times min/max mass ratio "
                                      "for this heat exchanger");
                    break;
                case CalculateNTUBoundsErrors::NominalEffectiveness3:
                    FatalError = true;
                    ShowSevereError(state, format("In the HeatExchanger:AirToAir:FlatPlate component {}", this->Name));
                    ShowContinueError(state, "  the calculated nominal effectiveness is out of bounds");
                    ShowContinueError(state, format("The effectiveness is {:.3R}", Eps0));
                    ShowContinueError(state, "The effectiveness should be >= 0.0 and <= 1.0");
                    ShowContinueError(state,
                                      "Eff = (Nom_Sup_Mass_Flow_Rate/Min_Mass_Flow_Rate)*(T_nom_sup_out-T_nom_sup_in)/(T_nom_sec_in-T_nom_sup_in)");
                    ShowContinueError(state, "The temperatures are user inputs. The mass flow rates are user input volume flow rates");
                    ShowContinueError(state, format("  times the density of air [{:.2R} kg/m3]", RhoAir));
                    ShowContinueError(state, "Change these inputs to obtain a physically realizable heat exchanger effectiveness");
                    break;
                case CalculateNTUBoundsErrors::Invalid:
                case CalculateNTUBoundsErrors::Num:
                    break; // function won't actually return ::Invalid or ::Num, this is just so the compiler doesn't complain about the missing cases
                }

                if (FatalError) {
                    ShowFatalError(state, "Heat exchanger design calculation caused fatal error: program terminated.");
                }

                this->UA0 = NTU0 * CMin0 * CpAir;
                this->mTSup0 = this->NomSupAirMassFlow * (this->NomSupAirInTemp + KELVZERO);
                this->mTSec0 = this->NomSecAirMassFlow * (this->NomSecAirInTemp + KELVZERO);

                // check validity
                if (this->NomSupAirMassFlow * this->NomSecAirMassFlow < HVAC::SmallMassFlow * HVAC::SmallMassFlow) {
                    ShowFatalError(state, "Mass flow in HeatExchanger:AirToAir:FlatPlate too small in initialization.");
                }

                if (this->mTSup0 < HVAC::SmallMassFlow) {
                    ShowFatalError(state, "(m*T)Sup,in in HeatExchanger:AirToAir:FlatPlate too small in initialization.");
                }

                if (this->mTSec0 < HVAC::SmallMassFlow) {
                    ShowFatalError(state, "(m*T)Sec,in in HeatExchanger:AirToAir:FlatPlate too small in initialization.");
                }

                if (CMin0 < HVAC::SmallMassFlow) {
                    ShowFatalError(state, "CMin0 in HeatExchanger:AirToAir:FlatPlate too small in initialization.");
                }
                break;

            case HVAC::HXType::AirToAir_SensAndLatent:
                if (this->SupOutletNode > 0 && this->ControlToTemperatureSetPoint) {
                    if (state.dataLoopNodes->Node(this->SupOutletNode).TempSetPoint == DataLoopNode::SensedNodeFlagValue) {
                        if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
                            ShowSevereError(state,
                                            format("Missing temperature setpoint for {} \"{}\" :", HVAC::hxTypeNames[(int)this->type], this->Name));
                            ShowContinueError(
                                state, "  use a Setpoint Manager to establish a setpoint at the supply air outlet node of the Heat Exchanger.");
                            ShowFatalError(state, " Previous condition causes program termination.");
                        } else {
                            // need call to EMS to check node
                            EMSManager::CheckIfNodeSetPointManagedByEMS(state, this->SupOutletNode, HVAC::CtrlVarType::Temp, FatalError);
                            if (FatalError) {
                                ShowSevereError(
                                    state, format("Missing temperature setpoint for {} \"{}\" :", HVAC::hxTypeNames[(int)this->type], this->Name));
                                ShowContinueError(
                                    state, "  use a Setpoint Manager to establish a setpoint at the supply air outlet node of the Heat Exchanger.");
                                ShowContinueError(
                                    state, "  or use an EMS actuator to establish a setpoint at the supply air outlet node of the Heat Exchanger.");
                                ShowFatalError(state, " Previous condition causes program termination.");
                            }
                        }
                    }
                }
                break;

            default:
                // nothing
                break;
            }

            this->myEnvrnFlag = false;
        }

        if (!state.dataGlobal->BeginEnvrnFlag) {
            this->myEnvrnFlag = true;
        }

        // Do these initializations every time step
        int const SupInNode = this->SupInletNode;
        int const SecInNode = this->SecInletNode;

        // Get information from inlet nodes

        this->SupInTemp = state.dataLoopNodes->Node(SupInNode).Temp;
        this->SupInHumRat = state.dataLoopNodes->Node(SupInNode).HumRat;
        this->SupInEnth = state.dataLoopNodes->Node(SupInNode).Enthalpy;
        this->SupInMassFlow = state.dataLoopNodes->Node(SupInNode).MassFlowRate;
        this->SecInTemp = state.dataLoopNodes->Node(SecInNode).Temp;
        this->SecInHumRat = state.dataLoopNodes->Node(SecInNode).HumRat;
        this->SecInEnth = state.dataLoopNodes->Node(SecInNode).Enthalpy;
        this->SecInMassFlow = state.dataLoopNodes->Node(SecInNode).MassFlowRate;

        // initialize the output variables
        this->SensHeatingRate = 0.0;
        this->SensHeatingEnergy = 0.0;
        this->LatHeatingRate = 0.0;
        this->LatHeatingEnergy = 0.0;
        this->TotHeatingRate = 0.0;
        this->TotHeatingEnergy = 0.0;
        this->SensCoolingRate = 0.0;
        this->SensCoolingEnergy = 0.0;
        this->LatCoolingRate = 0.0;
        this->LatCoolingEnergy = 0.0;
        this->TotCoolingRate = 0.0;
        this->TotCoolingEnergy = 0.0;
        this->ElecUseRate = 0.0;
        this->ElecUseEnergy = 0.0;
        this->SensEffectiveness = 0.0;
        this->LatEffectiveness = 0.0;
        this->SupBypassMassFlow = 0.0;
        this->SecBypassMassFlow = 0.0;

        //  Initialize inlet conditions

        switch (this->type) {
        case HVAC::HXType::AirToAir_FlatPlate:
        case HVAC::HXType::AirToAir_SensAndLatent:
            break;

        case HVAC::HXType::Desiccant_Balanced:
            if (this->MySetPointTest) {
                if (!state.dataGlobal->SysSizingCalc && state.dataHVACGlobal->DoSetPointTest) {
                    if (!state.dataHeatRecovery->CalledFromParentObject) {
                        if (state.dataLoopNodes->Node(this->SecOutletNode).HumRatMax == DataLoopNode::SensedNodeFlagValue) {
                            if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
                                ShowWarningError(
                                    state,
                                    format("Missing optional HumRatMax setpoint for {} \"{}\"", HVAC::hxTypeNames[(int)this->type], this->Name));
                                ShowContinueError(state,
                                                  "...the simulation will continue without control of the desiccant heat exchanger to a maximum "
                                                  "humidity ratio setpoint.");
                                ShowContinueError(state,
                                                  "...use a Setpoint Manager to establish a setpoint at the process air outlet node of the "
                                                  "desiccant Heat Exchanger if control is desired.");
                            } else {
                                bool LocalWarningError = false;
                                // need call to EMS to check node
                                EMSManager::CheckIfNodeSetPointManagedByEMS(
                                    state, this->SecOutletNode, HVAC::CtrlVarType::MaxHumRat, LocalWarningError);
                                state.dataLoopNodes->NodeSetpointCheck(this->SecOutletNode).needsSetpointChecking = false;
                                if (LocalWarningError) {
                                    ShowWarningError(
                                        state,
                                        format("Missing optional HumRatMax setpoint for {} \"{}\"", HVAC::hxTypeNames[(int)this->type], this->Name));
                                    ShowContinueError(state,
                                                      "...the simulation will continue without control of the desiccant heat exchanger to a "
                                                      "maximum humidity ratio setpoint.");
                                    ShowContinueError(state,
                                                      "...use a Setpoint Manager to establish a setpoint at the process air outlet node of the "
                                                      "desiccant Heat Exchanger if control is desired.");
                                    ShowContinueError(state,
                                                      "...or use an EMS Actuator to establish a maximum humidity ratio setpoint at the process "
                                                      "air outlet node of the desiccant Heat Exchanger if control is desired.");
                                }
                            }
                        }
                    }
                    this->MySetPointTest = false;
                }
            }

            if ((CompanionCoilIndex > -1) &&
                ((CompanionCoilType_Num == HVAC::CoilDX_CoolingSingleSpeed) || (CompanionCoilType_Num == HVAC::Coil_CoolingAirToAirVariableSpeed) ||
                 (CompanionCoilType_Num == HVAC::CoilDX_Cooling))) {

                if (CompanionCoilType_Num == HVAC::CoilDX_CoolingSingleSpeed || CompanionCoilType_Num == HVAC::CoilDX_CoolingTwoStageWHumControl) {
                    if (state.dataDXCoils->DXCoilFullLoadOutAirTemp(CompanionCoilIndex) == 0.0 ||
                        state.dataDXCoils->DXCoilFullLoadOutAirHumRat(CompanionCoilIndex) == 0.0) {
                        //       DX Coil is OFF, read actual inlet conditions
                        state.dataHeatRecovery->FullLoadOutAirTemp = this->SecInTemp;
                        state.dataHeatRecovery->FullLoadOutAirHumRat = this->SecInHumRat;
                    } else {
                        //       DX Coil is ON, read full load DX coil outlet conditions (conditions HX sees when ON)
                        state.dataHeatRecovery->FullLoadOutAirTemp = state.dataDXCoils->DXCoilFullLoadOutAirTemp(CompanionCoilIndex);
                        state.dataHeatRecovery->FullLoadOutAirHumRat = state.dataDXCoils->DXCoilFullLoadOutAirHumRat(CompanionCoilIndex);
                    }
                } else if (CompanionCoilType_Num == HVAC::Coil_CoolingAirToAirVariableSpeed) {
                    // how to support VS dx coil here?
                    state.dataHeatRecovery->FullLoadOutAirTemp = state.dataVariableSpeedCoils->VarSpeedCoil(CompanionCoilIndex).OutletAirDBTemp;
                    state.dataHeatRecovery->FullLoadOutAirHumRat = state.dataVariableSpeedCoils->VarSpeedCoil(CompanionCoilIndex).OutletAirHumRat;
                } else if (CompanionCoilType_Num == HVAC::CoilDX_Cooling) {
                    // Use the new coil option:
                    state.dataHeatRecovery->FullLoadOutAirTemp = state.dataCoilCoolingDX->coilCoolingDXs[CompanionCoilIndex].outletAirDryBulbTemp;
                    state.dataHeatRecovery->FullLoadOutAirHumRat = state.dataCoilCoolingDX->coilCoolingDXs[CompanionCoilIndex].outletAirHumRat;
                } else {
                    //
                }

            } else {

                //     HX only (not used in conjunction with DX coil), read inlet conditions
                state.dataHeatRecovery->FullLoadOutAirTemp = this->SecInTemp;
                state.dataHeatRecovery->FullLoadOutAirHumRat = this->SecInHumRat;
            }
            break;

        default:
            //   Will never get here
            break;
        }
    }

    void HeatExchCond::size(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   October 2007
        //       MODIFIED       February 2014 Daeho Kang, enable sizing multiple HX types and add additional sizing fields
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing Heat Exchanger components for which flow rates have not been
        // specified in the input. Currently, only nominal supply air flow rate for the generic HX can be autosized.

        // METHODOLOGY EMPLOYED:
        // Obtains flow rates from the system or OA system sizing arrays

        // SUBROUTINE PARAMETER DEFINITIONS:
        std::string_view RoutineName = "SizeHeatRecovery";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        std::string SizingString; // input field sizing description

        auto &ZoneEqSizing(state.dataSize->ZoneEqSizing);

        state.dataSize->HRFlowSizingFlag = true;
        bool PrintFlag = true; // true when sizing information is reported in the eio file
        int FieldNum = 0;      // IDD numeric field index where input field description is found
        switch (this->type) {
        case HVAC::HXType::Desiccant_Balanced:
            PrintFlag = false;
            break;
        case HVAC::HXType::AirToAir_SensAndLatent:
            FieldNum = 1;
            break;
        case HVAC::HXType::AirToAir_FlatPlate:
            FieldNum = 2;
            break;
        default:
            assert(0);
        }

        std::string CompName = this->Name;
        std::string CompType = std::string(HVAC::hxTypeNames[(int)this->type]);
        if (FieldNum > 0) {
            SizingString = this->NumericFieldNames(FieldNum) + " [m3/s]";
        } else {
            SizingString = "Nominal Supply Air Flow Rate [m3/s]"; // desiccant balanced flow does not have an input for air volume flow rate
        }
        if (state.dataSize->CurZoneEqNum > 0) {
            if (this->NomSupAirVolFlow == DataSizing::AutoSize) {
                if (ZoneEqSizing(state.dataSize->CurZoneEqNum).DesignSizeFromParent) {
                    // Heat recovery heat exchanger in zoneHVAC equipment should have been sized to OA flow in the parent equipment
                    state.dataSize->DataConstantUsedForSizing = ZoneEqSizing(state.dataSize->CurZoneEqNum).AirVolFlow;
                } else {
                    state.dataSize->DataConstantUsedForSizing =
                        std::max(state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolVolFlow,
                                 state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesHeatVolFlow);
                }
                state.dataSize->DataFractionUsedForSizing = 1.0;
            } else {
                if (state.dataSize->ZoneSizingRunDone) {
                    if (ZoneEqSizing(state.dataSize->CurZoneEqNum).DesignSizeFromParent) {
                        // Heat recovery heat exchanger in zoneHVAC equipment should have been sized to OA flow in the parent equipment
                        state.dataSize->DataConstantUsedForSizing = ZoneEqSizing(state.dataSize->CurZoneEqNum).AirVolFlow;
                    } else {
                        state.dataSize->DataConstantUsedForSizing =
                            std::max(state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolVolFlow,
                                     state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesHeatVolFlow);
                    }
                    state.dataSize->DataFractionUsedForSizing = 1.0;
                }
            }
        }
        Real64 TempSize = this->NomSupAirVolFlow;
        bool errorsFound = false;
        SystemAirFlowSizer sizerSystemAirFlow;
        sizerSystemAirFlow.overrideSizingString(SizingString);
        // sizerSystemAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
        sizerSystemAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
        this->NomSupAirVolFlow = sizerSystemAirFlow.size(state, TempSize, errorsFound);
        state.dataSize->DataConstantUsedForSizing = 0.0;
        state.dataSize->DataFractionUsedForSizing = 0.0;
        switch (this->type) {
        case HVAC::HXType::AirToAir_SensAndLatent: {
            this->NomSecAirVolFlow = this->NomSupAirVolFlow;
            state.dataSize->HRFlowSizingFlag = false;
            break;
        }
        case HVAC::HXType::AirToAir_FlatPlate: {
            PrintFlag = true;
            FieldNum = 5;
            CompName = this->Name;
            CompType = HVAC::hxTypeNames[(int)this->type];
            SizingString = this->NumericFieldNames(FieldNum) + " [m3/s]";
            if (this->NomSecAirVolFlow == DataSizing::AutoSize) {
                state.dataSize->DataConstantUsedForSizing = this->NomSupAirVolFlow;
                state.dataSize->DataFractionUsedForSizing = 1.0;
            } else {
                if (state.dataSize->ZoneSizingRunDone || state.dataSize->SysSizingRunDone) {
                    state.dataSize->DataConstantUsedForSizing = this->NomSupAirVolFlow;
                    state.dataSize->DataFractionUsedForSizing = 1.0;
                }
            }
            TempSize = this->NomSecAirVolFlow;
            bool errorsFound2 = false;
            SystemAirFlowSizer sizerSystemAirFlow2;
            sizerSystemAirFlow2.overrideSizingString(SizingString);
            // sizerSystemAirFlow2.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
            sizerSystemAirFlow2.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
            this->NomSecAirVolFlow = sizerSystemAirFlow2.size(state, TempSize, errorsFound2);
            state.dataSize->DataConstantUsedForSizing = 0.0;
            state.dataSize->DataFractionUsedForSizing = 0.0;
            state.dataSize->HRFlowSizingFlag = false;

            // Calculate nominal effectiveness

            break;
        }
        case HVAC::HXType::Desiccant_Balanced: {
            state.dataSize->HRFlowSizingFlag = false;
            int const BalDesDehumPerfIndex = this->PerfDataIndex; // index of dehum performance data1 object
            auto &thisBDDPerf = state.dataHeatRecovery->BalDesDehumPerfData(BalDesDehumPerfIndex);

            FieldNum = 1;
            PrintFlag = true;
            CompName = thisBDDPerf.Name;
            CompType = thisBDDPerf.PerfType;
            SizingString = thisBDDPerf.NumericFieldNames(FieldNum) + " [m3/s]";
            TempSize = thisBDDPerf.NomSupAirVolFlow;
            bool errorsFound2 = false;
            SystemAirFlowSizer sizerSystemAirFlow3;
            sizerSystemAirFlow3.overrideSizingString(SizingString);
            // sizerSystemAirFlow3.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
            sizerSystemAirFlow3.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
            thisBDDPerf.NomSupAirVolFlow = sizerSystemAirFlow3.size(state, TempSize, errorsFound2);
            this->NomSupAirVolFlow = thisBDDPerf.NomSupAirVolFlow;
            this->NomSecAirVolFlow = thisBDDPerf.NomSupAirVolFlow;

            state.dataSize->DataAirFlowUsedForSizing = thisBDDPerf.NomSupAirVolFlow;
            TempSize = thisBDDPerf.NomProcAirFaceVel;
            bool errorsFound3 = false;
            DesiccantDehumidifierBFPerfDataFaceVelocitySizer sizerDesDehumBFFaceVel;
            sizerDesDehumBFFaceVel.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
            thisBDDPerf.NomProcAirFaceVel = sizerDesDehumBFFaceVel.size(state, TempSize, errorsFound3);

            state.dataSize->DataAirFlowUsedForSizing = 0.0;
            break;
        }
        default:
            assert(0);
        }

        // std 229 new heat recovery table variables
        assert((this->type != HVAC::HXType::Invalid) && (this->ExchConfig != HXExchConfigType::Invalid));
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchAirHRInputObjType, this->Name, HVAC::hxTypeNames[(int)this->type]);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchAirHRPlateOrRotary, this->Name, hxExchConfigTypeNames[(int)this->ExchConfig]);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchAirHRSupplyAirflow, this->Name, this->NomSupAirVolFlow);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchAirHRExhaustAirflow, this->Name, this->NomSecAirVolFlow);

        if (this->type == HVAC::HXType::AirToAir_SensAndLatent) {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchAirHRSenEffAt100PerHeatAirFlow, this->Name, this->HeatEffectSensible100);
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchAirHRSenEffAt100PerCoolAirFlow, this->Name, this->CoolEffectSensible100);
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchAirHRLatEffAt100PerHeatAirFlow, this->Name, this->HeatEffectLatent100);
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchAirHRLatEffAt100PerCoolAirFlow, this->Name, this->CoolEffectLatent100);
        } else {
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchAirHRSenEffAt100PerHeatAirFlow, this->Name, "N/A");
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchAirHRSenEffAt100PerCoolAirFlow, this->Name, "N/A");
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchAirHRLatEffAt100PerHeatAirFlow, this->Name, "N/A");
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchAirHRLatEffAt100PerCoolAirFlow, this->Name, "N/A");
        }

        std::string_view loopName = "";

        if ((state.dataSize->CurSysNum > 0) && (state.dataSize->CurSysNum <= state.dataHVACGlobal->NumPrimaryAirSys)) {
            loopName = state.dataAirSystemsData->PrimaryAirSystems(state.dataSize->CurSysNum).Name;
        }

        int hxBypassControlType = -1;
        bool hasEconomizerControl = false;
        if ((state.dataSize->CurOASysNum > 0) && (state.dataSize->CurOASysNum <= state.dataAirLoop->NumOASystems)) {
            auto const &oaSys = state.dataAirLoop->OutsideAirSys(state.dataSize->CurOASysNum);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchAirHROASysName, this->Name, oaSys.Name);
            if (oaSys.AirLoopDOASNum > -1) {
                loopName = state.dataAirLoopHVACDOAS->airloopDOAS[oaSys.AirLoopDOASNum].Name;
                // no OAcontroller is directly applicable to HX in airLoopDOAS system
            } else {
                if (oaSys.OAControllerIndex > 0) {
                    auto &oaCntrlr = state.dataMixedAir->OAController(oaSys.OAControllerIndex);
                    if (oaCntrlr.Econo != MixedAir::EconoOp::NoEconomizer) {
                        hasEconomizerControl = true;
                        hxBypassControlType = oaCntrlr.HeatRecoveryBypassControlType;
                    }
                }
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchAirHROAControllerName, this->Name, oaSys.OAControllerName);
            }
        }
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchAirHRAirloopName, this->Name, loopName);

        // HX operation type
        HXOperation operation = HXOperation::WhenFansOn;
        if (!this->EconoLockOut || !hasEconomizerControl) {
            if ((this->availSched != nullptr) && (!this->availSched->checkMinVal(state, Clusive::Ex, 0.0))) {
                operation = HXOperation::Scheduled;
            }
        } else {
            if (hxBypassControlType == HVAC::BypassWhenOAFlowGreaterThanMinimum) {
                operation = HXOperation::WhenMinOA;
            } else if ((hxBypassControlType == HVAC::BypassWhenWithinEconomizerLimits) || this->hasZoneERVController) {
                operation = HXOperation::WhenOutsideEconomizerLimits;
            }
        }
        OutputReportPredefined::PreDefTableEntry(state,
                                                 state.dataOutRptPredefined->pdchAirHROperation,
                                                 this->Name,
                                                 (operation != HXOperation::Invalid) ? hxOperationNames[(int)operation] : "N/A");
    }

    void
    HeatExchCond::CalcAirToAirPlateHeatExch(EnergyPlusData &state,
                                            bool const HXUnitOn,                           // flag to simulate heat exchanger heat recovery
                                            ObjexxFCL::Optional_bool_const EconomizerFlag, // economizer flag pass by air loop or OA sys
                                            ObjexxFCL::Optional_bool_const HighHumCtrlFlag // high humidity control flag passed by airloop or OA sys
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Michael Wetter
        //       DATE WRITTEN   March 1999
        //       MODIFIED       F. Buhl Nov 2000, R. Raustad - FSEC, Feb 2009 - added economizer flags
        //                      Both the economizer and high humidity control flags can disable the HX
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculate the outlet conditions for an air to air plate heat
        // exchanger given the inlet conditions.

        // METHODOLOGY EMPLOYED:
        // This is a static heat exchanger model. No geometrical input data
        // is needed. No knowledge of h*A values is needed except the ratio
        // of the primary side to the secondary side convective heat transfer
        // coefficient times the exchanger surface area. Effectiveness - NTU
        // heat exchanger formulas are used.

        // The time varying load is calculated based on the variation of the
        // convective heat transfer coefficient.The variation is a function of
        // mass flow rate and inlet temperature. An iterative solution is only
        // required during initialization in one specific flow arrangement. During
        // the time steps the solution is explicit. The iteration is done with
        // the Regula Falsi algorithm. Convergence is always achieved.

        // REFERENCES:
        // M. Wetter, Simulation Model Air-to-Air Plate Heat Exchanger
        // LBNL Report 42354, 1999.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool UnitOn = true;    // unit on flag
        Real64 QTrans = 0.0;   // heat transferred in the heat exchanger [W]
        Real64 ElecCons = 0.0; // electricity consumption rate [W]

        bool EconomizerActiveFlag = present(EconomizerFlag) && bool(EconomizerFlag);    // local representing the economizer status when PRESENT
        bool HighHumCtrlActiveFlag = present(HighHumCtrlFlag) && bool(HighHumCtrlFlag); // local representing high humidity control when PRESENT

        Real64 UnitSupMassFlow; // supply air mass flow rate passing through the unit [kg/s]
        Real64 UnitSecMassFlow; // secondary air mass flow rate passing through the unit [kg/s]
        if ((EconomizerActiveFlag || HighHumCtrlActiveFlag) && this->EconoLockOut) {
            UnitSupMassFlow = 0.0; // set HX supply flow to 0, all supply air will go through supply bypass
            UnitSecMassFlow = 0.0; // set HX secondary flow to 0, all secondary air will got through secondary bypass
            UnitOn = false;        // turn off HX calculations when in economizer mode
        } else {
            // if economizer operation is not allowed, air always passes through HX
            // if CompanionCoilNum > 0, air always passes through HX (no economizer operation allowed)
            UnitSupMassFlow = min(this->NomSupAirMassFlow, this->SupInMassFlow);
            UnitSecMassFlow = min(this->NomSecAirMassFlow, this->SecInMassFlow);
        }

        if (this->availSched->getCurrentVal() <= 0.0) {
            UnitOn = false;
        }
        if (this->SupInMassFlow <= HVAC::SmallMassFlow) {
            UnitOn = false;
        }
        if (this->SecInMassFlow <= HVAC::SmallMassFlow) {
            UnitOn = false;
        }
        if (!HXUnitOn) {
            UnitOn = false;
        }

        if (UnitOn) {
            // unit is on
            // calculate the UA for this time step
            // ratio of supply nominal m*T to actual m*T
            Real64 const QuotSup = SafeDiv(this->mTSup0, UnitSupMassFlow * (this->SupInTemp + KELVZERO));
            // ratio of secondary nominal m*T to actual m*T
            Real64 const QuotExh = SafeDiv(this->mTSec0, UnitSecMassFlow * (this->SecInTemp + KELVZERO));
            // denominator of UA calculation
            Real64 const Deno = std::pow(QuotSup, 0.78) + this->hARatio * std::pow(QuotExh, 0.78);
            // present UA
            Real64 const UA = this->UA0 * (this->hARatio + 1.0) / Deno;
            // calculate the NTU
            Real64 const CSup = UnitSupMassFlow * Psychrometrics::PsyCpAirFnW(this->SupInHumRat);
            // secondary air capacitance rate [J/C/s]
            Real64 const CSec = UnitSecMassFlow * Psychrometrics::PsyCpAirFnW(this->SecInHumRat);
            // note: no C can be zero since otherwise we wouldn't be here
            Real64 Z;    // Ratio of minimum air capacitance rate to maximum air capacitance rate
            Real64 CMin; // minimum air capacitance rate [J/C/s]
            if (CSup < CSec) {
                CMin = CSup;
                Z = CMin / CSec;
            } else {
                CMin = CSec;
                Z = CMin / CSup;
            }
            // Number of heat transfer units
            Real64 const NTU = UA / CMin;
            // Get the effectiveness
            Real64 Eps = CalculateEpsFromNTUandZ(state, NTU, Z, this->FlowArr);
            // use the effectiveness to calculate the unit outlet conditions
            Real64 TempSupOut = this->SupInTemp + Eps * CMin / CSup * (this->SecInTemp - this->SupInTemp);
            QTrans = CSup * (TempSupOut - this->SupInTemp);
            // unit secondary outlet temperature [C]
            Real64 TempSecOut = this->SecInTemp - QTrans / CSec;
            // unit supply outlet humidity ratio [kg water / kg dry air]
            Real64 HumRatSupOut = this->SupInHumRat;
            // unit supply outlet enthalpy [J/kg]
            Real64 const EnthSupOut = Psychrometrics::PsyHFnTdbW(TempSupOut, HumRatSupOut);
            // check for saturation in supply outlet
            // unit supply outlet temperature at saturation (at EnthSupOut) [C]
            Real64 const TempSupOutSat = Psychrometrics::PsyTsatFnHPb(state, EnthSupOut, state.dataEnvrn->OutBaroPress);
            if (TempSupOutSat > TempSupOut) {
                TempSupOut = TempSupOutSat;
                HumRatSupOut = Psychrometrics::PsyWFnTdbH(state, TempSupOut, EnthSupOut);
            }
            // unit secondary outlet humidity ratio [kg water / kg dry air]
            Real64 HumRatSecOut = this->SecInHumRat;
            // unit secondary outlet enthalpy [J/kgC]
            Real64 const EnthSecOut = Psychrometrics::PsyHFnTdbW(TempSecOut, HumRatSecOut);
            // check for saturation in secondary outlet
            // unit secondary outlet temperature at saturation (at EnthsSecOut) [C]
            Real64 const TempSecOutSat = Psychrometrics::PsyTsatFnHPb(state, EnthSecOut, state.dataEnvrn->OutBaroPress);
            if (TempSecOutSat > TempSecOut) {
                TempSecOut = TempSecOutSat;
                HumRatSecOut = Psychrometrics::PsyWFnTdbH(state, TempSecOut, EnthSecOut);
            }
            // calculate outlet conditions by mixing bypass air stream with air that went through the
            // heat exchanger core.
            Real64 local_SupBypassMassFlow = max(0.0, this->SupInMassFlow - UnitSupMassFlow); // supply air mass flow rate bypassing unit [kg/s]
            Real64 local_SecBypassMassFlow = max(0.0, this->SecInMassFlow - UnitSecMassFlow); // secondary air mass flow rate bypassing unit [kg/s]

            this->SupOutEnth = (UnitSupMassFlow * EnthSupOut + local_SupBypassMassFlow * this->SupInEnth) / this->SupInMassFlow;
            this->SupOutHumRat = (UnitSupMassFlow * HumRatSupOut + local_SupBypassMassFlow * this->SupInHumRat) / this->SupInMassFlow;
            this->SupOutTemp = Psychrometrics::PsyTdbFnHW(this->SupOutEnth, this->SupOutHumRat);
            this->SupOutMassFlow = this->SupInMassFlow;
            this->SecOutEnth = (UnitSecMassFlow * EnthSecOut + local_SecBypassMassFlow * this->SecInEnth) / this->SecInMassFlow;
            this->SecOutHumRat = (UnitSecMassFlow * HumRatSecOut + local_SecBypassMassFlow * this->SecInHumRat) / this->SecInMassFlow;
            this->SecOutTemp = Psychrometrics::PsyTdbFnHW(this->SecOutEnth, this->SecOutHumRat);
            this->SecOutMassFlow = this->SecInMassFlow;
            ElecCons = this->NomElecPower;

        } else {
            // the unit is off. Pass through the air streams with no change
            this->SupOutEnth = this->SupInEnth;
            this->SupOutHumRat = this->SupInHumRat;
            this->SupOutTemp = this->SupInTemp;
            this->SupOutMassFlow = this->SupInMassFlow;
            this->SecOutEnth = this->SecInEnth;
            this->SecOutHumRat = this->SecInHumRat;
            this->SecOutTemp = this->SecInTemp;
            this->SecOutMassFlow = this->SecInMassFlow;
        }
        // supply air capacitance rate [J/C/s]
        Real64 const CSup = this->SupInMassFlow * Psychrometrics::PsyCpAirFnW(this->SupInHumRat);
        // sensible heat recovery rate to supply air (heating +, cooling -)
        Real64 const SensHeatRecRate = CSup * (this->SupOutTemp - this->SupInTemp);
        // total heat recovery rate to supply air (heating +, cooling -)
        Real64 const TotHeatRecRate = this->SupOutMassFlow * (this->SupOutEnth - this->SupInEnth);
        // latent heat recovery rate to supply air (heating [humidify] +, cooling [dehumidify] -)
        Real64 const LatHeatRecRate = TotHeatRecRate - SensHeatRecRate;

        if (SensHeatRecRate > 0.0) {
            this->SensHeatingRate = SensHeatRecRate;
            this->SensCoolingRate = 0.0;
        } else {
            this->SensHeatingRate = 0.0;
            this->SensCoolingRate = std::abs(SensHeatRecRate);
        }
        if (LatHeatRecRate > 0.0) {
            this->LatHeatingRate = LatHeatRecRate;
            this->LatCoolingRate = 0.0;
        } else {
            this->LatHeatingRate = 0.0;
            this->LatCoolingRate = std::abs(LatHeatRecRate);
        }
        if (TotHeatRecRate > 0.0) {
            this->TotHeatingRate = TotHeatRecRate;
            this->TotCoolingRate = 0.0;
        } else {
            this->TotHeatingRate = 0.0;
            this->TotCoolingRate = std::abs(TotHeatRecRate);
        }

        this->ElecUseRate = ElecCons;
    }

    void HeatExchCond::CalcAirToAirGenericHeatExch(
        EnergyPlusData &state,
        bool const HXUnitOn,                              // flag to simulate heat exchanger heat recovery
        bool const FirstHVACIteration,                    // first HVAC iteration flag
        HVAC::FanOp const fanOp,                          // Supply air fan operating mode (1=cycling, 2=constant)
        ObjexxFCL::Optional_bool_const EconomizerFlag,    // economizer flag pass by air loop or OA sys
        ObjexxFCL::Optional_bool_const HighHumCtrlFlag,   // high humidity control flag passed by airloop or OA sys
        ObjexxFCL::Optional<Real64 const> HXPartLoadRatio //
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Don Shirey
        //       DATE WRITTEN   February 2003
        //       MODIFIED       R. Raustad - FSEC, Feb 2009 - added economizer flags
        //                      Both the economizer and high humidity control flags can disable the HX
        //       RE-ENGINEERED  Richard Raustad, June 2003

        // PURPOSE OF THIS SUBROUTINE:
        //  Calculate the outlet conditions for an air to air generic heat
        //  exchanger given the inlet conditions.

        // METHODOLOGY EMPLOYED:
        //  This is a standard heat exchanger effectiveness model. No geometrical input data
        //  is needed. The model uses heat exchanger effectiveness performance data
        //  to calculate the air temperature and humidity ratio of the leaving
        //  supply and secondary air streams. Linear interpolation (or extrapolation)
        //  is assumed to obtain heat exchanger effectiveness at off-rated conditions.
        //  Economizer operation is allowed through the use of a Controller: Outside Air
        //  object.

        // REFERENCES:
        //  ARI Standard 1060-2001,Rating Air-to-Air Heat Exchangers for Energy Recovery Ventilation Equipment, www.ari.org
        //  ASHRAE Standard 84, Method of Testing Air-To-Air Heat Exchangers, www.ashrae.org
        //  U.S. Environmental Protection Agency software "SAVES" -
        //   School Advanced Ventilation Engineering Software http://www.epa.gov/iaq/schooldesign/saves.html

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr ErrorTol(0.001); // error tolerance

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int SupOutNode;
        Real64 Error;           // iteration loop error variable
        Real64 Iter;            // iteration counter
        Real64 ControlFraction; // fraction of effectiveness when rotary HX speed or plate bypass modulation is used for
        // temperature control
        Real64 RhoSup;              // supply air density at actual pressure, temperature and humidity conditions [kg/m3]
        Real64 RhoSec;              // secondary air density at actual pressure, temperature and humidity conditions [kg/m3]
        Real64 RhoStd;              // standard air density at actual pressure, 20C dry-bulb temp and 0.0 absolute humidity [kg/m3]
        Real64 CSup;                // supply air heat capacity rate [W/K]
        Real64 CSec;                // secondary air heat capacity rate [W/K]
        Real64 CMin;                // minimum air heat capacity rate [W/K]
        Real64 TempSecOutSat;       // secondary air outlet temperature at saturation (at EnthsSecOut) [C]
        Real64 HXSecAirVolFlowRate; // air volume flow rate of the secondary air stream through the heat exchanger [m3/sec]
        Real64 HXSupAirVolFlowRate; // air volume flow rate of the supply air stream through the heat exchanger [m3/sec]
        Real64 HXAvgAirVolFlowRate; // average air volume flow rate through the heat exchanger [m3/sec]
        Real64 HXAirVolFlowRatio;   // ratio of avg actual air volume flow through HX to nominal HX air volume flow [-]
        Real64 HXTempSetPoint;      // setpoint temperature at supply outlet node of HX when ControlToTemperatureSetPoint = Yes
        Real64 MassFlowSecIn;       // secondary air mass flow rate at HX inlet
        //  REAL(r64)    :: MassFlowSecOut      ! secondary air mass flow rate at HX outlet
        Real64 MassFlowSupIn;     // supply air mass flow rate at HX inlet
        Real64 MassFlowSupOut;    // supply air mass flow rate through HX core outlet
        Real64 MassFlowSupBypass; // supply air bypass mass flow rate around HX core
        Real64 TempSupIn;         // supply side temperature of air entering HX
        Real64 TempSupOut;        // supply side temperature of air leaving HX core
        Real64 HumRatSupIn;       // supply side humidity ratio of air entering HX
        Real64 TempSecIn;         // secondary side temperature of air entering HX
        Real64 SensHeatRecRate;   // sensible heat recovery rate to supply air (heating +, cooling -)
        Real64 LatHeatRecRate;    // latent heat recovery rate to supply air (heating [humidify] +, cooling [dehumidify] -)
        Real64 TotHeatRecRate;    // total heat recovery rate to supply air (heating +, cooling -)
        Real64 AirSidePLR;

        // Initialize local variables
        bool UnitOn = true;      // unit on flag
        Real64 QSensTrans = 0.0; // sensible heat transferred by the heat exchanger [W]
        Real64 QTotTrans = 0.0;  // total heat (sensible + latent) transferred by the heat exchanger [W]

        this->DefrostFraction = 0.0;
        this->SensEffectiveness = 0.0;
        this->LatEffectiveness = 0.0;
        this->ElecUseRate = 0.0;
        this->SupOutTemp = this->SupInTemp;
        this->SecOutTemp = this->SecInTemp;
        this->SupOutHumRat = this->SupInHumRat;
        this->SecOutHumRat = this->SecInHumRat;
        this->SupOutEnth = this->SupInEnth;
        this->SecOutEnth = this->SecInEnth;
        SupOutNode = this->SupOutletNode;
        HXTempSetPoint = state.dataLoopNodes->Node(SupOutNode).TempSetPoint;

        bool EconomizerActiveFlag = present(EconomizerFlag) && bool(EconomizerFlag);    // local representing the economizer status when PRESENT
        bool HighHumCtrlActiveFlag = present(HighHumCtrlFlag) && bool(HighHumCtrlFlag); // local representing high humidity control when PRESENT

        // Determine mass flow through heat exchanger and mass flow being bypassed (only flat plate bypasses flow)
        if (((EconomizerActiveFlag || HighHumCtrlActiveFlag) && this->EconoLockOut) && this->ExchConfig == HXExchConfigType::Plate) {
            this->SupBypassMassFlow = this->SupInMassFlow;
            this->SupOutMassFlow = this->SupInMassFlow;
            this->SecBypassMassFlow = this->SecInMassFlow;
            this->SecOutMassFlow = this->SecInMassFlow;
        } else { // No bypass mass flow
            this->SupOutMassFlow = this->SupInMassFlow;
            this->SecOutMassFlow = this->SecInMassFlow;
            this->SupBypassMassFlow = 0.0;
            this->SecBypassMassFlow = 0.0;
        }
        // Unit is scheduled OFF, so bypass heat exchange calcs
        if (this->availSched->getCurrentVal() <= 0.0) {
            UnitOn = false;
        }
        //! Economizer is active, so bypass heat exchange calcs. This applies to both flat plate and rotary HX's
        if ((EconomizerActiveFlag || HighHumCtrlActiveFlag) && this->EconoLockOut) {
            UnitOn = false;
        }
        // Determine if unit is ON or OFF based on air mass flow through the supply and secondary airstreams and operation flag
        if (this->SupInMassFlow <= HVAC::SmallMassFlow) {
            UnitOn = false;
        }
        if (this->SecInMassFlow <= HVAC::SmallMassFlow) {
            UnitOn = false;
        }
        if (!HXUnitOn) {
            UnitOn = false;
        }
        if (this->NomSupAirVolFlow == 0.0) {
            UnitOn = false;
        }

        if (UnitOn) {
            bool FrostControlFlag = false; // unit is in frost control mode when TRUE
            // Unit is on.
            if (present(HXPartLoadRatio) && fanOp == HVAC::FanOp::Cycling) {
                if (HXPartLoadRatio > 0) {
                    AirSidePLR = HXPartLoadRatio;
                } else {
                    AirSidePLR = 1.0;
                }
            } else {
                AirSidePLR = 1.0;
            }

            if (fanOp == HVAC::FanOp::Cycling) {
                this->SupInMassFlow /= AirSidePLR;
                this->SupOutMassFlow /= AirSidePLR;
                this->SecInMassFlow /= AirSidePLR;
                this->SecOutMassFlow /= AirSidePLR;
                this->SupBypassMassFlow /= AirSidePLR;
                this->SecBypassMassFlow /= AirSidePLR;
            }

            // In the future, use actual node pressures in the following air density calls
            RhoStd = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, 20.0, 0.0);
            HXSupAirVolFlowRate = this->SupOutMassFlow / RhoStd; // volume flow using standard density
            HXSecAirVolFlowRate = this->SecOutMassFlow / RhoStd;
            // Limit unbalanced volumetric flow ratio to 2:1
            if (!state.dataGlobal->WarmupFlag && !FirstHVACIteration) {
                if (HXSupAirVolFlowRate != 0.0 && HXSecAirVolFlowRate != 0.0) {
                    if (((HXSupAirVolFlowRate / HXSecAirVolFlowRate) > 2.0) || ((HXSecAirVolFlowRate / HXSupAirVolFlowRate) > 2.0)) {
                        ++this->UnBalancedErrCount;
                        if (this->UnBalancedErrCount <= 2) {
                            ShowSevereError(state,
                                            format("{}: \"{}\" unbalanced air volume flow ratio through the heat exchanger is greater than 2:1.",
                                                   HVAC::hxTypeNames[(int)this->type],
                                                   this->Name));
                            ShowContinueErrorTimeStamp(
                                state, format("...HX Supply air to Exhaust air flow ratio = {:.5R}.", HXSupAirVolFlowRate / HXSecAirVolFlowRate));
                        } else {
                            ShowRecurringWarningErrorAtEnd(
                                state,
                                format("{} \"{}\":  Unbalanced air volume flow ratio exceeds 2:1 warning continues. HX flow ratio statistics follow.",
                                       HVAC::hxTypeNames[(int)this->type],
                                       this->Name),
                                this->UnBalancedErrIndex,
                                HXSupAirVolFlowRate / HXSecAirVolFlowRate,
                                HXSupAirVolFlowRate / HXSecAirVolFlowRate);
                        }
                    }
                }
            }
            // Calculate average volumetric flow rate of the two air streams
            HXAvgAirVolFlowRate = (HXSecAirVolFlowRate + HXSupAirVolFlowRate) / 2.0;
            HXAirVolFlowRatio = HXAvgAirVolFlowRate / this->NomSupAirVolFlow;
            // Average air volume flow rate must be between 50% and 130% of nominal supply air volume flow
            if (HXAirVolFlowRatio > 1.3 || HXAirVolFlowRatio < 0.5) {
                if (!state.dataGlobal->WarmupFlag && !FirstHVACIteration) {
                    ++this->LowFlowErrCount;
                    if (this->LowFlowErrCount == 1) {
                        ShowWarningError(state, format("{} \"{}\"", HVAC::hxTypeNames[(int)this->type], this->Name));
                        ShowContinueError(state, "Average air volume flow rate is <50% or >130% of the nominal HX supply air volume flow rate.");
                        ShowContinueErrorTimeStamp(state, format("Air volume flow rate ratio = {:.3R}.", HXAirVolFlowRatio));
                    } else {
                        ShowRecurringWarningErrorAtEnd(
                            state,
                            format(
                                "{} \"{}\":  Average air volume flow rate is <50% or >130% warning continues. Air flow rate ratio statistics follow.",
                                HVAC::hxTypeNames[(int)this->type],
                                this->Name),
                            this->LowFlowErrIndex,
                            HXAirVolFlowRatio,
                            HXAirVolFlowRatio);
                    }
                }
            }

            // Determine heat exchanger effectiveness using avg air volume flow rate based on actual inlet air density
            // Linearly interpolate and extrapolate (within limits) from effectiveness input values
            RhoSup = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, this->SupInTemp, this->SupInHumRat);
            RhoSec = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, this->SecInTemp, this->SecInHumRat);
            HXSupAirVolFlowRate = this->SupOutMassFlow / RhoSup;
            HXSecAirVolFlowRate = this->SecOutMassFlow / RhoSec;
            HXAvgAirVolFlowRate = (HXSecAirVolFlowRate + HXSupAirVolFlowRate) / 2.0;
            HXAirVolFlowRatio = HXAvgAirVolFlowRate / this->NomSupAirVolFlow;

            if (this->SupInTemp < this->SecInTemp) {
                // Use heating effectiveness values
                this->SensEffectiveness = this->HeatEffectSensible100;
                if (this->HeatEffectSensibleCurveIndex > 0) {
                    this->SensEffectiveness *= Curve::CurveValue(state, this->HeatEffectSensibleCurveIndex, HXAirVolFlowRatio);
                }
                this->LatEffectiveness = this->HeatEffectLatent100;
                if (this->HeatEffectLatentCurveIndex > 0) {
                    this->LatEffectiveness *= Curve::CurveValue(state, this->HeatEffectLatentCurveIndex, HXAirVolFlowRatio);
                }
            } else {
                // Use cooling effectiveness values
                this->SensEffectiveness = this->CoolEffectSensible100;
                if (this->CoolEffectSensibleCurveIndex > 0) {
                    this->SensEffectiveness *= Curve::CurveValue(state, this->CoolEffectSensibleCurveIndex, HXAirVolFlowRatio);
                }
                this->LatEffectiveness = this->CoolEffectLatent100;
                if (this->CoolEffectLatentCurveIndex > 0) {
                    this->LatEffectiveness *= Curve::CurveValue(state, this->CoolEffectLatentCurveIndex, HXAirVolFlowRatio);
                }
            }

            //     Keep effectiveness between 0 and 1.0 ??
            //     HXOpSensEffect = MAX(MIN(HXOpSensEffect,1.0),0.0)
            //     HXOpLatEffect =  MAX(MIN(HXOpLatEffect,1.0),0.0)
            if (this->SensEffectiveness < 0.0) {
                //   The model should at least guard against negative numbers
                this->SensEffectiveness = 0.0;
                if (!this->SensEffectivenessFlag) {
                    ShowWarningError(
                        state,
                        format(
                            "HeatExchanger:AirToAir:SensibleAndLatent =\"{}\" sensible effectiveness is less than zero. Check the following inputs.",
                            this->Name));
                    if (this->SupInTemp < this->SecInTemp) {
                        ShowContinueError(state, format("...Sensible Effectiveness at 100% Heating Air Flow = {:.2R}", this->HeatEffectSensible100));
                        ShowContinueError(state, "...Sensible effectiveness reset to zero and the simulation continues.");
                    } else {
                        ShowContinueError(state, format("...Sensible Effectiveness at 100% Cooling Air Flow = {:.2R}", this->CoolEffectSensible100));
                        ShowContinueError(state, "...Sensible effectiveness reset to zero and the simulation continues.");
                    }
                    ShowContinueError(state, format("...Heat Exchanger Air Volume Flow Ratio = {:.2R}", HXAirVolFlowRatio));
                    this->SensEffectivenessFlag = true;
                }
            }
            if (this->LatEffectiveness < 0.0) {
                // The model should at least guard against negative numbers
                this->LatEffectiveness = 0.0;
                if (!this->LatEffectivenessFlag) {
                    ShowWarningError(
                        state,
                        format("HeatExchanger:AirToAir:SensibleAndLatent =\"{}\" latent effectiveness is less than zero. Check the following inputs.",
                               this->Name));
                    if (this->SupInTemp < this->SecInTemp) {
                        ShowContinueError(state, format("...Latent Effectiveness at 100% Heating Air Flow = {:.2R}", this->HeatEffectLatent100));
                        ShowContinueError(state, "...Latent effectiveness reset to zero and the simulation continues.");
                    } else {
                        ShowContinueError(state, format("...Latent Effectiveness at 100% Cooling Air Flow = {:.2R}", this->CoolEffectLatent100));
                        ShowContinueError(state, "...Latent effectiveness reset to zero and the simulation continues.");
                    }
                    ShowContinueError(state, format("...Heat Exchanger Air Volume Flow Ratio = {:.2R}", HXAirVolFlowRatio));
                    this->LatEffectivenessFlag = true;
                }
            }
            // Use the effectiveness to calculate the air conditions exiting the heat exchanger (all air flow through the HX)
            // Include EATR and OACF in the following calculations at some point

            CSup = this->SupOutMassFlow * Psychrometrics::PsyCpAirFnW(this->SupInHumRat);
            CSec = this->SecOutMassFlow * Psychrometrics::PsyCpAirFnW(this->SecInHumRat);
            CMin = min(CSup, CSec);

            this->SupOutTemp = this->SupInTemp + this->SensEffectiveness * CMin / CSup * (this->SecInTemp - this->SupInTemp);
            this->SupOutHumRat = this->SupInHumRat + this->LatEffectiveness * CMin / CSup * (this->SecInHumRat - this->SupInHumRat);
            this->SupOutEnth = Psychrometrics::PsyHFnTdbW(this->SupOutTemp, this->SupOutHumRat);

            //   Check for saturation in supply outlet and reset temp, then humidity ratio at constant enthalpy
            if (Psychrometrics::PsyTsatFnHPb(state, this->SupOutEnth, state.dataEnvrn->OutBaroPress) > this->SupOutTemp) {
                this->SupOutTemp = Psychrometrics::PsyTsatFnHPb(state, this->SupOutEnth, state.dataEnvrn->OutBaroPress);
                this->SupOutHumRat = Psychrometrics::PsyWFnTdbH(state, this->SupOutTemp, this->SupOutEnth);
            }
            QSensTrans = CSup * (this->SupInTemp - this->SupOutTemp);
            this->SecOutTemp = this->SecInTemp + QSensTrans / CSec;
            QTotTrans = this->SupOutMassFlow * (this->SupInEnth - this->SupOutEnth);
            this->SecOutEnth = this->SecInEnth + QTotTrans / this->SecOutMassFlow;
            this->SecOutHumRat = Psychrometrics::PsyWFnTdbH(state, this->SecOutTemp, this->SecOutEnth);
            //   Control the supply air outlet temperature to a setpoint for Heating Mode only
            //   (ControlFraction = 0 HX fully bypassed, ControlFraction = 1 air passed entirely through HX)
            //   (supply air stream bypass mass flow rate proportional to ControlFraction except when frost control is active)
            if (this->ControlToTemperatureSetPoint) {
                if ((this->SupInTemp - this->SupOutTemp) != 0.0) {
                    if ((this->SupOutTemp > HXTempSetPoint && this->SupInTemp < HXTempSetPoint) ||
                        (this->SupInTemp > HXTempSetPoint && this->SupOutTemp < HXTempSetPoint)) {
                        ControlFraction = max(0.0, min(1.0, std::abs((this->SupInTemp - HXTempSetPoint) / (this->SupInTemp - this->SupOutTemp))));
                    } else if ((this->SupInTemp < this->SupOutTemp && this->SupOutTemp < HXTempSetPoint) ||
                               (this->SupInTemp > this->SupOutTemp && this->SupOutTemp > HXTempSetPoint)) {
                        ControlFraction = 1.0;
                    } else {
                        ControlFraction = 0.0;
                    }
                } else {
                    //     ELSE fully bypass HX to maintain supply outlet temp as high as possible
                    ControlFraction = 0.0;
                }
                if (this->ExchConfig == HXExchConfigType::Rotary) {
                    //       Rotary HX's never get bypassed, rotational speed is modulated
                    this->SensEffectiveness *= ControlFraction;
                    this->LatEffectiveness *= ControlFraction;
                } else { // HX is a plate heat exchanger, bypass air to control SA temperature
                    Error = 1.0;
                    Iter = 0.0;
                    MassFlowSupIn = this->SupInMassFlow;
                    MassFlowSupOut = this->SupOutMassFlow;
                    MassFlowSupBypass = this->SupBypassMassFlow;
                    MassFlowSecIn = this->SecInMassFlow;
                    TempSupIn = this->SupInTemp;
                    TempSupOut = this->SupOutTemp;
                    HumRatSupIn = this->SupInHumRat;
                    TempSecIn = this->SecInTemp;
                    while ((std::abs(Error) > ErrorTol && Iter < 10 && ControlFraction < 1.0) || Iter == 1) {
                        MassFlowSupOut = MassFlowSupIn * ControlFraction;
                        MassFlowSupBypass = MassFlowSupIn * (1.0 - ControlFraction);
                        HXSupAirVolFlowRate = MassFlowSupOut / RhoSup;
                        HXAvgAirVolFlowRate = (HXSecAirVolFlowRate + HXSupAirVolFlowRate) / 2.0;
                        HXAirVolFlowRatio = HXAvgAirVolFlowRate / this->NomSupAirVolFlow;
                        CSup = MassFlowSupOut * Psychrometrics::PsyCpAirFnW(HumRatSupIn);
                        CMin = min(CSup, CSec);
                        if (TempSupIn < TempSecIn) {
                            //          Use heating effectiveness values
                            this->SensEffectiveness = this->HeatEffectSensible100;
                            if (this->HeatEffectSensibleCurveIndex > 0) {
                                this->SensEffectiveness *= Curve::CurveValue(state, this->HeatEffectSensibleCurveIndex, HXAirVolFlowRatio);
                            }
                            this->LatEffectiveness = this->HeatEffectLatent100;
                            if (this->HeatEffectLatentCurveIndex > 0) {
                                this->LatEffectiveness *= Curve::CurveValue(state, this->HeatEffectLatentCurveIndex, HXAirVolFlowRatio);
                            }
                        } else {
                            //          Use cooling effectiveness values
                            this->SensEffectiveness = this->CoolEffectSensible100;
                            if (this->CoolEffectSensibleCurveIndex > 0) {
                                this->SensEffectiveness *= Curve::CurveValue(state, this->CoolEffectSensibleCurveIndex, HXAirVolFlowRatio);
                            }
                            this->LatEffectiveness = this->CoolEffectLatent100;
                            if (this->CoolEffectLatentCurveIndex > 0) {
                                this->LatEffectiveness *= Curve::CurveValue(state, this->CoolEffectLatentCurveIndex, HXAirVolFlowRatio);
                            }
                        }

                        if (this->SensEffectiveness < 0.0) {
                            //   The model should at least guard against negative numbers
                            this->SensEffectiveness = 0.0;
                            if (!this->SensEffectivenessFlag) {
                                ShowWarningError(state,
                                                 format("HeatExchanger:AirToAir:SensibleAndLatent =\"{}\" sensible effectiveness is less than zero. "
                                                        "Check the following inputs.",
                                                        this->Name));
                                if (this->SupInTemp < this->SecInTemp) {
                                    ShowContinueError(
                                        state, format("...Sensible Effectiveness at 100% Heating Air Flow = {:.2R}", this->HeatEffectSensible100));
                                    ShowContinueError(state, "...Sensible effectiveness reset to zero and the simulation continues.");
                                } else {
                                    ShowContinueError(
                                        state, format("...Sensible Effectiveness at 100% Cooling Air Flow = {:.2R}", this->CoolEffectSensible100));
                                    ShowContinueError(state, "...Sensible effectiveness reset to zero and the simulation continues.");
                                }
                                ShowContinueError(state, format("...Heat Exchanger Air Volume Flow Ratio = {:.2R}", HXAirVolFlowRatio));
                                this->SensEffectivenessFlag = true;
                            }
                        }
                        if (this->LatEffectiveness < 0.0) {
                            // The model should at least guard against negative numbers
                            this->LatEffectiveness = 0.0;
                            if (!this->LatEffectivenessFlag) {
                                ShowWarningError(state,
                                                 format("HeatExchanger:AirToAir:SensibleAndLatent =\"{}\" latent effectiveness is less than zero. "
                                                        "Check the following inputs.",
                                                        this->Name));
                                if (this->SupInTemp < this->SecInTemp) {
                                    ShowContinueError(state,
                                                      format("...Latent Effectiveness at 100% Heating Air Flow = {:.2R}", this->HeatEffectLatent100));
                                    ShowContinueError(state, "...Latent effectiveness reset to zero and the simulation continues.");
                                } else {
                                    ShowContinueError(state,
                                                      format("...Latent Effectiveness at 100% Cooling Air Flow = {:.2R}", this->CoolEffectLatent100));
                                    ShowContinueError(state, "...Latent effectiveness reset to zero and the simulation continues.");
                                }
                                ShowContinueError(state, format("...Heat Exchanger Air Volume Flow Ratio = {:.2R}", HXAirVolFlowRatio));
                                this->LatEffectivenessFlag = true;
                            }
                        }

                        if (CSup == 0.0) {
                            //          IF CSup = 0, then supply air mass flow rate = 0 and HX is fully bypassed. Fix divide by 0 error below DO loop.
                            CSup = 1.0;
                            CMin = 0.0;
                            break;
                        }
                        TempSupOut = (MassFlowSupOut * (TempSupIn + this->SensEffectiveness * CMin / CSup * (TempSecIn - TempSupIn)) +
                                      MassFlowSupBypass * TempSupIn) /
                                     MassFlowSupIn;
                        Error = (TempSupOut - HXTempSetPoint);
                        //         IF supply inlet temp = supply outlet temp, fully bypass HX - ELSE control to SP
                        if (TempSupIn != TempSupOut) {
                            ControlFraction = max(0.0, min(1.0, std::abs(ControlFraction * (TempSupIn - HXTempSetPoint) / (TempSupIn - TempSupOut))));
                        } else if (std::abs(TempSupOut - HXTempSetPoint) < ErrorTol) {
                            //           IF TempSupIn = TempSupOut then TempSecIn = TempSupIn (ControlFraction = ?)
                            //           Do nothing, variables in ELSE below have already been calculated
                            break;
                        } else {
                            //           or HX is fully bypassed (ControlFraction = 0) which actually should be caught in IF(CSup .EQ. 0.0)THEN above.
                            ControlFraction = 0.0;
                            MassFlowSupOut = MassFlowSupIn * ControlFraction;
                            MassFlowSupBypass = MassFlowSupIn * (1.0 - ControlFraction);
                            CSup = 1.0;
                            CMin = 0.0;
                            break;
                        }
                        ++Iter;
                    }

                    this->SupInMassFlow = MassFlowSupIn;
                    this->SupOutMassFlow = MassFlowSupOut;
                    this->SupBypassMassFlow = MassFlowSupBypass;
                    this->SecInMassFlow = MassFlowSecIn;
                    this->SupInTemp = TempSupIn;
                    this->SupOutTemp = TempSupOut;
                    this->SupInHumRat = HumRatSupIn;
                    this->SecInTemp = TempSecIn;

                } // ENDIF for "IF (thisExch%ExchConfig == 'ROTARY') THEN"
                this->SupOutTemp = this->SupInTemp + this->SensEffectiveness * CMin / CSup * (this->SecInTemp - this->SupInTemp);
                this->SupOutHumRat = this->SupInHumRat + this->LatEffectiveness * CMin / CSup * (this->SecInHumRat - this->SupInHumRat);
                this->SupOutEnth = Psychrometrics::PsyHFnTdbW(this->SupOutTemp, this->SupOutHumRat);

                //     Check for saturation in supply outlet and reset temp, then humidity ratio at constant enthalpy
                if (Psychrometrics::PsyTsatFnHPb(state, this->SupOutEnth, state.dataEnvrn->OutBaroPress) > this->SupOutTemp) {
                    this->SupOutTemp = Psychrometrics::PsyTsatFnHPb(state, this->SupOutEnth, state.dataEnvrn->OutBaroPress);
                    this->SupOutHumRat = Psychrometrics::PsyWFnTdbH(state, this->SupOutTemp, this->SupOutEnth);
                }

                QSensTrans = CSup * (this->SupInTemp - this->SupOutTemp);
                this->SecOutTemp = this->SecInTemp + QSensTrans / CSec;
                QTotTrans = this->SupOutMassFlow * (this->SupInEnth - this->SupOutEnth);
                this->SecOutEnth = this->SecInEnth + QTotTrans / this->SecOutMassFlow;
                this->SecOutHumRat = Psychrometrics::PsyWFnTdbH(state, this->SecOutTemp, this->SecOutEnth);

            } // ENDIF for "IF(thisExch%ControlToTemperatureSetPoint .AND... THEN, ELSE"

            if (fanOp == HVAC::FanOp::Cycling) {
                this->SupInMassFlow *= AirSidePLR;
                this->SupOutMassFlow *= AirSidePLR;
                this->SecInMassFlow *= AirSidePLR;
                this->SecOutMassFlow *= AirSidePLR;
                this->SupBypassMassFlow *= AirSidePLR;
                this->SecBypassMassFlow *= AirSidePLR;
            } else if (fanOp == HVAC::FanOp::Continuous) {
                this->SupOutTemp = this->SupOutTemp * AirSidePLR + this->SupInTemp * (1.0 - AirSidePLR);
                this->SupOutHumRat = this->SupOutHumRat * AirSidePLR + this->SupInHumRat * (1.0 - AirSidePLR);
                this->SupOutEnth = this->SupOutEnth * AirSidePLR + this->SupOutEnth * (1.0 - AirSidePLR);
                this->SecOutTemp = this->SecOutTemp * AirSidePLR + this->SecInTemp * (1.0 - AirSidePLR);
                this->SecOutHumRat = this->SecOutHumRat * AirSidePLR + this->SecInHumRat * (1.0 - AirSidePLR);
                this->SecOutEnth = this->SecOutEnth * AirSidePLR + this->SecOutEnth * (1.0 - AirSidePLR);
            }

            if ((this->FrostControlType == FrostControlOption::MinimumExhaustTemperature && this->SecOutTemp < this->ThresholdTemperature) ||
                (this->FrostControlType == FrostControlOption::ExhaustAirRecirculation && this->SupInTemp <= this->ThresholdTemperature) ||
                (this->FrostControlType == FrostControlOption::ExhaustOnly && this->SupInTemp <= this->ThresholdTemperature)) {
                this->FrostControl(state);
                FrostControlFlag = true;
            }

            // check for saturation in secondary outlet
            TempSecOutSat = Psychrometrics::PsyTsatFnHPb(state, this->SecOutEnth, state.dataEnvrn->OutBaroPress);
            if (TempSecOutSat > this->SecOutTemp) {
                this->SecOutTemp = TempSecOutSat;
                this->SecOutHumRat = Psychrometrics::PsyWFnTdbH(state, this->SecOutTemp, this->SecOutEnth);
            }

            // calculate outlet conditions by mixing bypass air stream with air that went through the
            // heat exchanger core.  Perform this mixing only when no frost control is used or
            // heat exchanger is not in frost control mode.  Mixing similar to this is performed
            // in the frost control subroutine when in frost control mode.
            if (!FrostControlFlag) {
                this->SupOutEnth = (this->SupOutMassFlow * this->SupOutEnth + this->SupBypassMassFlow * this->SupInEnth) / this->SupInMassFlow;
                this->SupOutHumRat = (this->SupOutMassFlow * this->SupOutHumRat + this->SupBypassMassFlow * this->SupInHumRat) / this->SupInMassFlow;
                this->SupOutTemp = Psychrometrics::PsyTdbFnHW(this->SupOutEnth, this->SupOutHumRat);
                this->SupOutMassFlow = this->SupInMassFlow;
                this->SecOutEnth = (this->SecOutMassFlow * this->SecOutEnth + this->SecBypassMassFlow * this->SecInEnth) / this->SecInMassFlow;
                this->SecOutHumRat = (this->SecOutMassFlow * this->SecOutHumRat + this->SecBypassMassFlow * this->SecInHumRat) / this->SecInMassFlow;
                this->SecOutTemp = Psychrometrics::PsyTdbFnHW(this->SecOutEnth, this->SecOutHumRat);
                this->SecOutMassFlow = this->SecInMassFlow;
            }

            this->ElecUseRate = this->NomElecPower;

        } // ENDIF for "IF (UnitOn) THEN"

        // Calculate heat transfer from the unit using the final supply inlet and supply outlet air conditions
        CSup = this->SupOutMassFlow * Psychrometrics::PsyCpAirFnW(this->SupInHumRat);
        SensHeatRecRate = CSup * (this->SupOutTemp - this->SupInTemp);
        TotHeatRecRate = this->SupOutMassFlow * (this->SupOutEnth - this->SupInEnth);
        LatHeatRecRate = TotHeatRecRate - SensHeatRecRate;

        // Set report variables based on sign of recovery rate
        if (SensHeatRecRate > 0.0) {
            this->SensHeatingRate = SensHeatRecRate;
            this->SensCoolingRate = 0.0;
        } else {
            this->SensHeatingRate = 0.0;
            this->SensCoolingRate = std::abs(SensHeatRecRate);
        }
        if (LatHeatRecRate > 0.0) {
            this->LatHeatingRate = LatHeatRecRate;
            this->LatCoolingRate = 0.0;
        } else {
            this->LatHeatingRate = 0.0;
            this->LatCoolingRate = std::abs(LatHeatRecRate);
        }
        if (TotHeatRecRate > 0.0) {
            this->TotHeatingRate = TotHeatRecRate;
            this->TotCoolingRate = 0.0;
        } else {
            this->TotHeatingRate = 0.0;
            this->TotCoolingRate = std::abs(TotHeatRecRate);
        }
    }

    void HeatExchCond::CalcDesiccantBalancedHeatExch(
        EnergyPlusData &state,
        bool const HXUnitOn,                           // flag to simulate heat exchanger heat recovery
        bool const FirstHVACIteration,                 // First HVAC iteration flag
        HVAC::FanOp const fanOp,                       // Supply air fan operating mode (1=cycling, 2=constant)
        Real64 const PartLoadRatio,                    // Part load ratio requested of DX compressor
        int const CompanionCoilIndex,                  // index of companion cooling coil
        int const CompanionCoilType,                   // type of cooling coil
        bool const RegenInletIsOANode,                 // Flag to determine if regen side inlet is OANode, if so this air stream cycles
        ObjexxFCL::Optional_bool_const EconomizerFlag, // economizer flag pass by air loop or OA sys
        ObjexxFCL::Optional_bool_const HighHumCtrlFlag // high humidity control flag passed by airloop or OA sys
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Mangesh Basarkar, FSEC
        //       DATE WRITTEN   January 2007
        //       MODIFIED       R. Raustad - FSEC, Feb 2009 - added economizer flags
        //                      Both the economizer and high humidity control flags can disable the HX
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //  Calculate the outlet conditions for a balanced air-to-air desiccant heat exchanger
        //  given the inlet conditions and face velocity. Performance map is provided by user.

        // METHODOLOGY EMPLOYED:
        //  This is an empirical heat exchanger model. The model uses heat exchanger performance data to
        //  calculate the air temperature and humidity ratio of the leaving supply and secondary air streams.
        //  Humidity control can enable/disable heat recovery through the use of the HXUnitOn Subroutine argument.

        // Using/Aliasing
        using DataLoopNode::SensedNodeFlagValue;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool UnitOn;                   // unit on flag
        Real64 RhoStd;                 // standard air density at actual pressure, 20C dry-bulb temp and 0.0 absolute humidity [kg/m3]
        Real64 CSup;                   // supply air heat capacity rate [W/K]
        Real64 CSec;                   // secondary air heat capacity rate [W/K]
        Real64 TempSecOutSat;          // secondary air outlet temperature at saturation (at EnthsSecOut) [C]
        Real64 SensHeatRecRate;        // sensible heat recovery rate to supply air (heating +, cooling -)
        Real64 TotHeatRecRate;         // total heat recovery rate to supply air (heating +, cooling -)
        Real64 ProcessSensHeatRecRate; // process sensible heat recovery rate (heating +, cooling -)
        Real64 ProcessTotHeatRecRate;  // process total heat recovery rate (heating +, cooling -)
        Real64 ProcessLatHeatRecRate;  // process latent heat recovery rate (heating [humidify] +, cooling [dehumidify] -)

        Real64 BalFaceVelActual;        // operating face velocity [m/s]
        Real64 FullLoadSupOutTemp(0);   // empirical model supply outlet temperature [C]
        Real64 FullLoadSupOutHumRat(0); // empirical model supply outlet humidity ratio [kg/kg]
        Real64 FullLoadDeltaT;          // empirical model heat exchanger delta temperature [C]
        Real64 FullLoadDeltaW;          // empirical model heat exchanger delta humidity ratio [kg/kg]
        Real64 T_RegenInTemp;           // empirical model supply (regen) inlet temperature for temperature equation [C]
        Real64 T_RegenInHumRat;         // empirical model supply (regen) inlet humidity ratio for temperature equation [kg/kg]
        Real64 T_ProcInTemp;            // empirical model secondary (process) inlet temperature for temperature equation [C]
        Real64 T_ProcInHumRat;          // empirical model secondary (process) inlet humidity ratio for temperature equation [kg/kg]
        Real64 T_FaceVel;               // empirical model face velocity for temperature equation [m/s]
        Real64 H_RegenInTemp;           // empirical model supply (regen) inlet temperature for humidity ratio equation [C]
        Real64 H_RegenInHumRat;         // empirical model supply (regen) inlet humidity ratio for humidity ratio equation [kg/kg]
        Real64 H_ProcInTemp;            // empirical model secondary (process) inlet temperature for humidity ratio equation [C]
        Real64 H_ProcInHumRat;          // empirical model secondary (process) inlet humidity ratio for humidity ratio equation [kg/kg]
        Real64 H_FaceVel;               // empirical model face velocity for humidity ratio equation [m/s]
        Real64 MaxHumRatNeeded;         // maximum humidity ratio setpoint for balanced desiccant HX [kg/kg]
        Real64 MinHumRatNeeded;         // minimum humidity ratio setpoint for balanced desiccant HX [kg/kg]
        Real64 HXPartLoadRatio;         // local heat exchanger part-load ratio
        Real64 TestSaturationEnthalpy;  // enthalpy used to test for regeneration outlet condition over saturation curve (J/kg)
        Real64 AverageMassFlowRate;     // average of supply (regen) and secondary (process) mass flow rates [kg/s]
        bool EconomizerActiveFlag;      // local representing the economizer status when PRESENT
        bool HighHumCtrlActiveFlag;     // local representing high humidity control when PRESENT

        // Initialize local variables
        UnitOn = true;
        SensHeatRecRate = 0.0;
        TotHeatRecRate = 0.0;
        HXPartLoadRatio = PartLoadRatio;
        this->DefrostFraction = 0.0;
        this->ElecUseRate = 0.0;
        this->SupOutTemp = this->SupInTemp;
        this->SecOutTemp = this->SecInTemp;
        this->SupOutHumRat = this->SupInHumRat;
        this->SecOutHumRat = this->SecInHumRat;
        this->SupOutEnth = this->SupInEnth;
        this->SecOutEnth = this->SecInEnth;
        this->SupOutMassFlow = this->SupInMassFlow;
        this->SecOutMassFlow = this->SecInMassFlow;
        AverageMassFlowRate = (this->SupOutMassFlow + this->SecOutMassFlow) / 2.0;

        if (present(EconomizerFlag)) {
            EconomizerActiveFlag = EconomizerFlag;
        } else {
            EconomizerActiveFlag = false;
        }

        if (present(HighHumCtrlFlag)) {
            HighHumCtrlActiveFlag = HighHumCtrlFlag;
        } else {
            HighHumCtrlActiveFlag = false;
        }

        // Unit is scheduled OFF, so bypass heat exchange calcs
        if (this->availSched->getCurrentVal() <= 0.0) {
            UnitOn = false;
        }
        // Determine if unit is ON or OFF based on air mass flow through the supply and secondary airstreams and operation flag
        if (this->SupInMassFlow <= HVAC::SmallMassFlow) {
            UnitOn = false;
        }
        if (this->SecInMassFlow <= HVAC::SmallMassFlow) {
            UnitOn = false;
        }
        if (HXPartLoadRatio == 0.0) {
            UnitOn = false;
        }
        if (!HXUnitOn) {
            UnitOn = false;
        }
        if ((EconomizerActiveFlag || HighHumCtrlActiveFlag) && this->EconoLockOut) {
            UnitOn = false;
        }

        if (UnitOn) {
            constexpr std::string_view ThisSubTSat = "CalcDesiccantBalancedHeatExch:   TSat";
            constexpr std::string_view ThisSubSecOutHumRat = "CalcDesiccantBalancedHeatExch:   SecOutHumRat";
            constexpr std::string_view ThisSubTestSatSec = "CalcDesiccantBalancedHeatExch:   TestSatSec";
            Real64 local_SupInMassFlow; // Supply side HX mass flow rate
            Real64 local_SecInMassFlow; // Secondary side HX mass flow rate

            //   Use local variables to perform checks
            local_SecInMassFlow = this->SecInMassFlow;
            local_SupInMassFlow = this->SupInMassFlow;

            // In constant fan mode, process air mass flow rate is full flow and supply (regen) air cycles based on PLR.
            // If supply (regen) inlet is OA node, regen mass flow rate is proportional to PLR.
            // If both of the above is true then boost local variable up to full flow
            if ((fanOp == HVAC::FanOp::Continuous) && RegenInletIsOANode) {
                local_SupInMassFlow /= HXPartLoadRatio;
            }
            // for cycling fan case, boost both local variables up to full flow
            if (fanOp == HVAC::FanOp::Cycling) {
                local_SupInMassFlow /= HXPartLoadRatio; // supply = regen
                local_SecInMassFlow /= HXPartLoadRatio; // secondary = process
            }

            // Check for balanced flow condition
            this->CheckForBalancedFlow(state, local_SecInMassFlow, local_SupInMassFlow, FirstHVACIteration);

            auto const &perf = state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex);

            T_ProcInTemp = state.dataHeatRecovery->FullLoadOutAirTemp;
            T_ProcInHumRat = state.dataHeatRecovery->FullLoadOutAirHumRat;
            T_RegenInTemp = this->SupInTemp;
            T_RegenInHumRat = this->SupInHumRat;

            // Must use the same density used to convert volumetric flow rate to mass flow rate to get back to velocity
            RhoStd = state.dataEnvrn->StdRhoAir; // PsyRhoAirFnPbTdbW(StdBaroPress,20.0d0, 0.0d0)
            BalFaceVelActual = local_SupInMassFlow / (RhoStd * this->FaceArea);

            T_FaceVel = BalFaceVelActual;

            //     Call model check routines only when HX is active, if coil is off these checks do not apply (no potential for heat transfer)
            //     Check RH limits and warn user if out of bounds (T_* not modified in subroutine)

            this->CheckModelBoundsRH_TempEq(state, T_RegenInTemp, T_RegenInHumRat, T_ProcInTemp, T_ProcInHumRat, FirstHVACIteration);
            //     Check model boundaries and cap empirical model independent variables as needed (T_* may be modified on return from sub)
            this->CheckModelBoundsTempEq(state, T_RegenInTemp, T_RegenInHumRat, T_ProcInTemp, T_ProcInHumRat, T_FaceVel, FirstHVACIteration);

            if (T_ProcInTemp != 0.0 && T_RegenInTemp != 0.0) {
                FullLoadSupOutTemp = perf.B[0] + perf.B[1] * T_RegenInHumRat + perf.B[2] * T_RegenInTemp +
                                     perf.B[3] * (T_RegenInHumRat / T_RegenInTemp) + perf.B[4] * T_ProcInHumRat + perf.B[5] * T_ProcInTemp +
                                     perf.B[6] * (T_ProcInHumRat / T_ProcInTemp) + perf.B[7] * T_FaceVel;

                // Check model boundary for supply (regen) temp and do not cap value if out of bounds, check that supply in temp > out temp
                this->CheckModelBoundOutput_Temp(state, this->SupInTemp, FullLoadSupOutTemp, FirstHVACIteration);
                FullLoadDeltaT = FullLoadSupOutTemp - this->SupInTemp;
            } else {
                FullLoadDeltaT = 0.0;
            }

            H_ProcInTemp = state.dataHeatRecovery->FullLoadOutAirTemp;
            H_ProcInHumRat = state.dataHeatRecovery->FullLoadOutAirHumRat;
            H_RegenInTemp = this->SupInTemp;
            H_RegenInHumRat = this->SupInHumRat;
            H_FaceVel = BalFaceVelActual;

            //     Call model check routines only when HX is active, if coil is off these checks do not apply (no potential for heat transfer)
            //     Check RH limits and warn user if out of bounds (H_* not modified in subroutine)

            this->CheckModelBoundsRH_HumRatEq(state, H_RegenInTemp, H_RegenInHumRat, H_ProcInTemp, H_ProcInHumRat, FirstHVACIteration);
            //     Check model boundaries and cap empirical model independent variables as needed (H_* may be modified on return from sub)
            this->CheckModelBoundsHumRatEq(state, H_RegenInTemp, H_RegenInHumRat, H_ProcInTemp, H_ProcInHumRat, H_FaceVel, FirstHVACIteration);

            //     Calc curve
            if (H_ProcInTemp != 0.0 && H_RegenInTemp != 0.0) {
                FullLoadSupOutHumRat = perf.C[0] + perf.C[1] * H_RegenInHumRat + perf.C[2] * H_RegenInTemp +
                                       perf.C[3] * (H_RegenInHumRat / H_RegenInTemp) + perf.C[4] * H_ProcInHumRat + perf.C[5] * H_ProcInTemp +
                                       perf.C[6] * (H_ProcInHumRat / H_ProcInTemp) + perf.C[7] * H_FaceVel;

                // Check model boundary for supply (regen) hum rat and do not cap value if out of bounds, check that supply in HR < out HR
                this->CheckModelBoundOutput_HumRat(state, this->SupInHumRat, FullLoadSupOutHumRat, FirstHVACIteration);
                FullLoadDeltaW = FullLoadSupOutHumRat - this->SupInHumRat;
            } else {
                FullLoadDeltaW = 0.0;
            }

            //     Check for saturation in the model's calculated supply outlet and reset temp, then humidity ratio at constant enthalpy
            //     Reset delta T and delta W such that the model does not allow an outlet condition over saturation
            TestSaturationEnthalpy = Psychrometrics::PsyHFnTdbW(FullLoadSupOutTemp, FullLoadSupOutHumRat);
            if (Psychrometrics::PsyTsatFnHPb(state, TestSaturationEnthalpy, state.dataEnvrn->OutBaroPress, ThisSubTSat) > FullLoadSupOutTemp) {
                constexpr std::string_view ThisSubTSatFullLoadOutTemp = "CalcDesiccantBalancedHeatExch:   TSat-FullLoadOutTemp";
                constexpr std::string_view ThisSubTSatFullLoadOutHumRat = "CalcDesiccantBalancedHeatExch:   TSat-FullLoadOutHumRat";
                FullLoadSupOutTemp =
                    Psychrometrics::PsyTsatFnHPb(state, TestSaturationEnthalpy, state.dataEnvrn->OutBaroPress, ThisSubTSatFullLoadOutTemp);
                FullLoadSupOutHumRat = Psychrometrics::PsyWFnTdbH(state, FullLoadSupOutTemp, TestSaturationEnthalpy, ThisSubTSatFullLoadOutHumRat);
                FullLoadDeltaT = FullLoadSupOutTemp - this->SupInTemp;
                FullLoadDeltaW = FullLoadSupOutHumRat - this->SupInHumRat;
            }

            if (!state.dataHeatRecovery->CalledFromParentObject) {
                //       calculate part-load ratio for HX
                MaxHumRatNeeded = state.dataLoopNodes->Node(this->SecOutletNode).HumRatMax;
                MinHumRatNeeded = state.dataLoopNodes->Node(this->SecOutletNode).HumRatMin;
                // Calculate partload fraction of dehumidification capacity required to meet setpoint

                //       check the model output, if the regen delta W is positive, the process air stream is dehumidified
                if (FullLoadDeltaW > 0) {
                    //         check for a setpoint, if no setpoint then PLR remains at 1
                    if (MaxHumRatNeeded != SensedNodeFlagValue) {
                        if (this->SecInHumRat > MaxHumRatNeeded && MaxHumRatNeeded > 0.0) {
                            HXPartLoadRatio = (this->SecInHumRat - MaxHumRatNeeded) / FullLoadDeltaW;
                        } else {
                            HXPartLoadRatio = 0.0;
                        }
                    }
                    //       check the model output, if the regen delta W is negative, the process air stream is humidified
                } else if (FullLoadDeltaW < 0) {
                    //         check for a setpoint, if no setpoint then PLR remains at 1
                    if (MinHumRatNeeded != SensedNodeFlagValue) {
                        if (this->SecInHumRat < MinHumRatNeeded && MinHumRatNeeded > 0.0) {
                            HXPartLoadRatio = (this->SecInHumRat - MinHumRatNeeded) / FullLoadDeltaW;
                        } else {
                            HXPartLoadRatio = 0.0;
                        }
                    }
                }

                HXPartLoadRatio = max(0.0, HXPartLoadRatio);
                HXPartLoadRatio = min(1.0, HXPartLoadRatio);

            } else if (CompanionCoilType > 0 && CompanionCoilIndex > -1) {
                if (CompanionCoilType == HVAC::CoilDX_Cooling) {
                    HXPartLoadRatio = state.dataCoilCoolingDX->coilCoolingDXs[CompanionCoilIndex].partLoadRatioReport;
                } else if (CompanionCoilType == HVAC::Coil_CoolingAirToAirVariableSpeed) {
                    HXPartLoadRatio = state.dataVariableSpeedCoils->VarSpeedCoil(CompanionCoilIndex).PartLoadRatio;
                } else {
                    HXPartLoadRatio = state.dataDXCoils->DXCoilPartLoadRatio(CompanionCoilIndex);
                }
            }

            Real64 constexpr lowerLimit = 1.e-5;
            if (fanOp == HVAC::FanOp::Cycling || RegenInletIsOANode) {
                //       Supply (regen) air stream mass flow rate is cycling and proportional to PLR, outlet conditions are full load
                //       conditions
                this->SupOutTemp = this->SupInTemp + FullLoadDeltaT;
                this->SupOutHumRat = min(1.0, max(lowerLimit, this->SupInHumRat + FullLoadDeltaW));
            } else {
                //       Supply (regen) air stream mass flow rate is constant and outlet conditions are averaged
                this->SupOutTemp = this->SupInTemp + (FullLoadDeltaT * HXPartLoadRatio);
                this->SupOutHumRat = min(1.0, max(lowerLimit, this->SupInHumRat + (FullLoadDeltaW * HXPartLoadRatio)));
            }

            //     for a balanced flow HX, use average mass flow rate and actual node conditions to calculate CSup and CSec
            //     the mass flow rate on the process and secondary side of HX may be imbalanced when the HX is used in the OA branch
            //     use the average mass flow rate to avoid psych warnings, mass flow rates will converge at the end of the iteration
            //     if the air mass flow rates do not converge, this model should not be used
            CSup = AverageMassFlowRate * Psychrometrics::PsyCpAirFnW(this->SupInHumRat);
            CSec = AverageMassFlowRate * Psychrometrics::PsyCpAirFnW(this->SecInHumRat);

            this->SupOutEnth = Psychrometrics::PsyHFnTdbW(this->SupOutTemp, this->SupOutHumRat);

            SensHeatRecRate = CSup * (this->SupOutTemp - this->SupInTemp);

            TotHeatRecRate = AverageMassFlowRate * (this->SupOutEnth - this->SupInEnth);

            //     now calculate process side heat transfer

            this->SecOutEnth = this->SecInEnth - TotHeatRecRate / AverageMassFlowRate;

            this->SecOutTemp = this->SecInTemp - SensHeatRecRate / CSec;

            this->SecOutHumRat = Psychrometrics::PsyWFnTdbH(state, this->SecOutTemp, this->SecOutEnth, ThisSubSecOutHumRat);

            // check for saturation in process (secondary) outlet
            // The process outlet conditions should never be over the saturation curve for the balanced desiccant model
            // although this may occur during warmup. This check is included here for consistency.
            TempSecOutSat = Psychrometrics::PsyTsatFnHPb(state, this->SecOutEnth, state.dataEnvrn->OutBaroPress, ThisSubTestSatSec);
            if (TempSecOutSat > this->SecOutTemp) {
                constexpr std::string_view ThisSubTSatSecOutHumRat = "CalcDesiccantBalancedHeatExch:   TSat-SecOutHumRat";
                this->SecOutTemp = TempSecOutSat;
                this->SecOutHumRat = Psychrometrics::PsyWFnTdbH(state, this->SecOutTemp, this->SecOutEnth, ThisSubTSatSecOutHumRat);
            }

            this->ElecUseRate = perf.NomElecPower * HXPartLoadRatio;

        } // ENDIF for "IF (UnitOn) THEN"

        // Report the process side heat transfer
        CSec = AverageMassFlowRate * Psychrometrics::PsyCpAirFnW(this->SecInHumRat);
        ProcessSensHeatRecRate = CSec * (this->SecOutTemp - this->SecInTemp);

        ProcessTotHeatRecRate = this->SecOutMassFlow * (this->SecOutEnth - this->SecInEnth);

        ProcessLatHeatRecRate = ProcessTotHeatRecRate - ProcessSensHeatRecRate;

        // Set report variables based on sign of recovery rate
        if (ProcessSensHeatRecRate > 0.0) {
            this->SensHeatingRate = ProcessSensHeatRecRate;
            this->SensCoolingRate = 0.0;
        } else {
            this->SensHeatingRate = 0.0;
            this->SensCoolingRate = std::abs(ProcessSensHeatRecRate);
        }
        if (ProcessLatHeatRecRate > 0.0) {
            this->LatHeatingRate = ProcessLatHeatRecRate;
            this->LatCoolingRate = 0.0;
        } else {
            this->LatHeatingRate = 0.0;
            this->LatCoolingRate = std::abs(ProcessLatHeatRecRate);
        }
        if (ProcessTotHeatRecRate > 0.0) {
            this->TotHeatingRate = ProcessTotHeatRecRate;
            this->TotCoolingRate = 0.0;
        } else {
            this->TotHeatingRate = 0.0;
            this->TotCoolingRate = std::abs(ProcessTotHeatRecRate);
        }
    }

    void HeatExchCond::FrostControl(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   June 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates fraction of timestep necessary to eliminate frost on ERV surface
        // by comparing secondary outlet or outdoor temperature to a frost control threshold
        // temperature.  Supply air and secondary air outlet conditions are calculated
        // based on frost control method selected.

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr ErrorTol(0.001); // error tolerance for iteration loop

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 DFFraction = 0.0;    // fraction of timestep ERV is in frost control mode
        Real64 Error;               // iteration loop error variable
        Real64 Iter;                // iteration counter
        Real64 QTotTrans;           // total heat transfer by ERV [W]
        Real64 QSensTrans;          // sensible heat transfer by ERV [W]
        Real64 HXSecAirVolFlowRate; // air volume flow rate of the secondary air stream through the heat exchanger [m3/sec]
        Real64 HXSupAirVolFlowRate; // air volume flow rate of the supply air stream through the heat exchanger [m3/sec]
        Real64 HXAvgAirVolFlowRate; // average air volume flow rate through the heat exchanger [m3/sec]
        Real64 HXAirVolFlowRatio;   // nominal to actual air volume flow ratio
        Real64 MassFlowSupIn;       // supply air mass flow rate at HX inlet
        Real64 MassFlowSupOut;      // supply air mass flow rate through HX core outlet
        Real64 MassFlowSupBypass;   // supply air bypass mass flow rate around HX core
        Real64 TempSupIn;           // supply side temperature of air entering HX
        Real64 TempSupOut;          // supply side temperature of air leaving HX core
        Real64 HumRatSupIn;         // supply side humidity ratio of air entering HX
        Real64 TempSecIn;           // secondary side temperature of air entering HX
        Real64 TempSecOut;          // secondary side temperature of air leaving HX core

        this->SupOutMassFlow = this->SupInMassFlow;
        this->SecOutMassFlow = this->SecInMassFlow;
        this->SupBypassMassFlow = 0.0;
        this->SecBypassMassFlow = 0.0;
        // density of supply air [kg/m3]
        Real64 const RhoSup = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, this->SupInTemp, this->SupInHumRat);
        // density of secondary air [kg/m3]
        Real64 const RhoSec = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, this->SecInTemp, this->SecInHumRat);
        // mdot Cp of supply air [W/K]
        Real64 CSup = this->SupOutMassFlow * Psychrometrics::PsyCpAirFnW(this->SupInHumRat);
        // mdot Cp of secondary air [W/K]
        Real64 const CSec = this->SecOutMassFlow * Psychrometrics::PsyCpAirFnW(this->SecInHumRat);
        // minimum mdot Cp of supply or secondary air [W/K]
        Real64 CMin = min(CSup, CSec);
        // threshold temperature below which frost control is active
        Real64 const TempThreshold = this->ThresholdTemperature;

        if (this->ControlToTemperatureSetPoint) {
            // Recalculate HX outlet conditions as if control to temperature setpoint was not activated,
            // because defrost will override those results

            HXSupAirVolFlowRate = this->SupOutMassFlow / RhoSup;
            HXSecAirVolFlowRate = this->SecOutMassFlow / RhoSec;
            HXAvgAirVolFlowRate = (HXSecAirVolFlowRate + HXSupAirVolFlowRate) / 2.0;
            HXAirVolFlowRatio = HXAvgAirVolFlowRate / this->NomSupAirVolFlow;
            this->SensEffectiveness = this->HeatEffectSensible100;
            if (this->HeatEffectSensibleCurveIndex > 0) {
                this->SensEffectiveness *= Curve::CurveValue(state, this->HeatEffectSensibleCurveIndex, HXAirVolFlowRatio);
            }
            this->LatEffectiveness = this->HeatEffectLatent100;
            if (this->HeatEffectLatentCurveIndex > 0) {
                this->LatEffectiveness *= Curve::CurveValue(state, this->HeatEffectLatentCurveIndex, HXAirVolFlowRatio);
            }
            this->SupOutTemp = this->SupInTemp + this->SensEffectiveness * CMin / CSup * (this->SecInTemp - this->SupInTemp);
            this->SupOutHumRat = this->SupInHumRat + this->LatEffectiveness * CMin / CSup * (this->SecInHumRat - this->SupInHumRat);
            this->SupOutEnth = Psychrometrics::PsyHFnTdbW(this->SupOutTemp, this->SupOutHumRat);

            //   Check for saturation in supply outlet and reset temp, then humidity ratio at constant enthalpy
            if (Psychrometrics::PsyTsatFnHPb(state, this->SupOutEnth, state.dataEnvrn->OutBaroPress) > this->SupOutTemp) {
                this->SupOutTemp = Psychrometrics::PsyTsatFnHPb(state, this->SupOutEnth, state.dataEnvrn->OutBaroPress);
                this->SupOutHumRat = Psychrometrics::PsyWFnTdbH(state, this->SupOutTemp, this->SupOutEnth);
            }

            QSensTrans = CSup * (this->SupInTemp - this->SupOutTemp);
            this->SecOutTemp = this->SecInTemp + QSensTrans / CSec;
            QTotTrans = this->SupOutMassFlow * (this->SupInEnth - this->SupOutEnth);
            this->SecOutEnth = this->SecInEnth + QTotTrans / this->SecOutMassFlow;
            this->SecOutHumRat = Psychrometrics::PsyWFnTdbH(state, this->SecOutTemp, this->SecOutEnth);
        }

        // Check frost control by type

        switch (this->FrostControlType) {
        case FrostControlOption::MinimumExhaustTemperature:
            //   A plate HX will bypass air on the supply side to keep exhaust temp above a
            //   threshold temperature and requires recalculating effectiveness based on
            //   the reduced air flow rate. A rotary HX modulates rotational speed to try to keep the
            //   exhaust air temperature above the threshold temperature. Assume that
            //   sensible and latent effectiveness decrease proportionally with rotary HX speed.

            DFFraction = max(0.0, min(1.0, SafeDiv((TempThreshold - this->SecOutTemp), (this->SecInTemp - this->SecOutTemp))));
            if (this->ExchConfig == HXExchConfigType::Rotary) {
                this->SensEffectiveness *= (1.0 - DFFraction);
                this->LatEffectiveness *= (1.0 - DFFraction);
            } else { // HX is a plate heat exchanger, bypass air to eliminate frost
                Error = 1.0;
                Iter = 0.0;
                MassFlowSupIn = this->SupInMassFlow;
                MassFlowSupOut = this->SupOutMassFlow;
                MassFlowSupBypass = this->SupBypassMassFlow;
                TempSupIn = this->SupInTemp;
                HumRatSupIn = this->SupInHumRat;
                TempSecIn = this->SecInTemp;

                while (std::abs(Error) > ErrorTol && Iter < 10) {
                    MassFlowSupOut = MassFlowSupIn * (1.0 - DFFraction);
                    MassFlowSupBypass = MassFlowSupIn * DFFraction;
                    HXSupAirVolFlowRate = MassFlowSupOut / RhoSup;
                    HXSecAirVolFlowRate = this->SecOutMassFlow / RhoSec;
                    HXAvgAirVolFlowRate = (HXSecAirVolFlowRate + HXSupAirVolFlowRate) / 2.0;
                    HXAirVolFlowRatio = HXAvgAirVolFlowRate / this->NomSupAirVolFlow;
                    CSup = MassFlowSupOut * Psychrometrics::PsyCpAirFnW(HumRatSupIn);
                    CMin = min(CSup, CSec);
                    if (TempSupIn < TempSecIn) {
                        //         Use heating effectiveness values
                        this->SensEffectiveness = this->HeatEffectSensible100;
                        if (this->HeatEffectSensibleCurveIndex > 0) {
                            this->SensEffectiveness *= Curve::CurveValue(state, this->HeatEffectSensibleCurveIndex, HXAirVolFlowRatio);
                        }
                        this->LatEffectiveness = this->HeatEffectLatent100;
                        if (this->HeatEffectLatentCurveIndex > 0) {
                            this->LatEffectiveness *= Curve::CurveValue(state, this->HeatEffectLatentCurveIndex, HXAirVolFlowRatio);
                        }
                    } else {
                        //         Use cooling effectiveness values
                        this->SensEffectiveness = this->CoolEffectSensible100;
                        if (this->CoolEffectSensibleCurveIndex > 0) {
                            this->SensEffectiveness *= Curve::CurveValue(state, this->CoolEffectSensibleCurveIndex, HXAirVolFlowRatio);
                        }
                        this->LatEffectiveness = this->CoolEffectLatent100;
                        if (this->CoolEffectLatentCurveIndex > 0) {
                            this->LatEffectiveness *= Curve::CurveValue(state, this->CoolEffectLatentCurveIndex, HXAirVolFlowRatio);
                        }
                    }
                    //         calculation of local variable Csup can be 0, guard against divide by 0.
                    TempSupOut = TempSupIn + this->SensEffectiveness * SafeDiv(CMin, CSup) * (TempSecIn - TempSupIn);
                    QSensTrans = CSup * (TempSupIn - TempSupOut);
                    //         Csec cannot be 0 in this subroutine
                    TempSecOut = TempSecIn + QSensTrans / CSec;
                    Error = (TempSecOut - TempThreshold);
                    //         recalculate DFFraction until convergence, guard against divide by 0 (unlikely).
                    DFFraction = max(0.0, min(1.0, DFFraction * SafeDiv((TempSecIn - TempSecOut), (TempSecIn - TempThreshold))));
                    ++Iter;
                }
                this->SupInMassFlow = MassFlowSupIn;
                this->SupOutMassFlow = MassFlowSupOut;
                this->SupBypassMassFlow = MassFlowSupBypass;
            }
            this->SupOutTemp = this->SupInTemp + this->SensEffectiveness * SafeDiv(CMin, CSup) * (this->SecInTemp - this->SupInTemp);
            this->SupOutHumRat = this->SupInHumRat + this->LatEffectiveness * SafeDiv(CMin, CSup) * (this->SecInHumRat - this->SupInHumRat);
            this->SupOutEnth = Psychrometrics::PsyHFnTdbW(this->SupOutTemp, this->SupOutHumRat);

            //   Check for saturation in supply outlet and reset temp, then humidity ratio at constant enthalpy
            if (Psychrometrics::PsyTsatFnHPb(state, this->SupOutEnth, state.dataEnvrn->OutBaroPress) > this->SupOutTemp) {
                this->SupOutTemp = Psychrometrics::PsyTsatFnHPb(state, this->SupOutEnth, state.dataEnvrn->OutBaroPress);
                this->SupOutHumRat = Psychrometrics::PsyWFnTdbH(state, this->SupOutTemp, this->SupOutEnth);
            }

            QSensTrans = CSup * (this->SupInTemp - this->SupOutTemp);
            this->SecOutTemp = this->SecInTemp + QSensTrans / CSec;
            QTotTrans = this->SupOutMassFlow * (this->SupInEnth - this->SupOutEnth);
            this->SecOutEnth = this->SecInEnth + QTotTrans / this->SecOutMassFlow;
            this->SecOutHumRat = Psychrometrics::PsyWFnTdbH(state, this->SecOutTemp, this->SecOutEnth);

            //   Perform mixing of core air stream and bypass air stream and set mass flow rates at outlet nodes
            this->SupOutEnth = (this->SupOutMassFlow * this->SupOutEnth + this->SupBypassMassFlow * this->SupInEnth) / this->SupInMassFlow;
            this->SupOutHumRat = (this->SupOutMassFlow * this->SupOutHumRat + this->SupBypassMassFlow * this->SupInHumRat) / this->SupInMassFlow;
            this->SupOutTemp = Psychrometrics::PsyTdbFnHW(this->SupOutEnth, this->SupOutHumRat);
            this->SupOutMassFlow = this->SupInMassFlow;
            this->SecOutEnth = (this->SecOutMassFlow * this->SecOutEnth + this->SecBypassMassFlow * this->SecInEnth) / this->SecInMassFlow;
            this->SecOutHumRat = (this->SecOutMassFlow * this->SecOutHumRat + this->SecBypassMassFlow * this->SecInHumRat) / this->SecInMassFlow;
            this->SecOutTemp = Psychrometrics::PsyTdbFnHW(this->SecOutEnth, this->SecOutHumRat);
            this->SecOutMassFlow = this->SecInMassFlow;
            break;

        case FrostControlOption::ExhaustAirRecirculation:
            // Directing exhaust outlet air back across the HX core on the supply side
            // Assume no heat exchange when in frost control mode, full heat exchange otherwise
            DFFraction = max(0.0, min((this->InitialDefrostTime + this->RateofDefrostTimeIncrease * (TempThreshold - this->SupInTemp)), 1.0));

            //    Calculate derated heat transfer using outlet air conditions assuming no defrost (calculated earlier)
            //    and (1-DefrostFraction)
            QSensTrans = (1.0 - DFFraction) * CSup * (this->SupInTemp - this->SupOutTemp);
            QTotTrans = (1.0 - DFFraction) * this->SupOutMassFlow * (this->SupInEnth - this->SupOutEnth);

            this->SupOutMassFlow = (1.0 - DFFraction) * this->SupInMassFlow + DFFraction * this->SecInMassFlow;

            //    Blend supply outlet condition of HX core with exhaust air inlet to get final supply air outlet conditions
            this->SupOutTemp = ((1.0 - DFFraction) * this->SupInMassFlow * this->SupOutTemp + DFFraction * this->SecInMassFlow * this->SecInTemp) /
                               this->SupOutMassFlow;

            this->SupOutHumRat =
                ((1.0 - DFFraction) * this->SupInMassFlow * this->SupOutHumRat + DFFraction * this->SecInMassFlow * this->SecInHumRat) /
                this->SupOutMassFlow;

            this->SupOutEnth = Psychrometrics::PsyHFnTdbW(this->SupOutTemp, this->SupOutHumRat);
            //    No need to check for saturation after SA out and EA inlet are blended

            //    Derate effectiveness based on frost control time fraction for reporting purposes
            this->SensEffectiveness *= (1.0 - DFFraction);
            this->LatEffectiveness *= (1.0 - DFFraction);

            //    Secondary air outlet conditions are previously calculated as the conditions when not
            //    in defrost, and this is what we want to report so no changes here.
            //    Average SupInMassFlow and SecOutMassFlow rates have been reduced due to frost control
            //      Equipment attached to the supply inlet node may have problems with our setting the
            //      mass flow rate in the next statement. This is done only to simulate exhaust air recirc.
            state.dataLoopNodes->Node(this->SupInletNode).MassFlowRate = this->SupInMassFlow * (1.0 - DFFraction);
            this->SecOutMassFlow *= (1.0 - DFFraction);
            break;

        case FrostControlOption::ExhaustOnly:

            //   Perform frost control by bypassing the supply air around the HX core during the defrost
            //   time period. HX heat transfer is reduced proportionally to (1 - defrosttimefraction)

            DFFraction = max(0.0, min((this->InitialDefrostTime + this->RateofDefrostTimeIncrease * (TempThreshold - this->SupInTemp)), 1.0));

            //   Calculate derated heat transfer based on defrost time
            QSensTrans = (1.0 - DFFraction) * CSup * (this->SupInTemp - this->SupOutTemp);
            QTotTrans = (1.0 - DFFraction) * this->SupOutMassFlow * (this->SupInEnth - this->SupOutEnth);

            //   Calculate the air conditions leaving heat exchanger unit
            //   Heat exchanger effectiveness is not derated, HX is fully bypassed during frost control

            this->SupBypassMassFlow = this->SupInMassFlow * DFFraction;
            this->SupOutTemp = this->SupInTemp - QSensTrans / CSup;
            this->SupOutEnth = this->SupInEnth - QTotTrans / this->SupOutMassFlow;
            this->SupOutHumRat = Psychrometrics::PsyWFnTdbH(state, this->SupOutTemp, this->SupOutEnth);

            if (Psychrometrics::PsyTsatFnHPb(state, this->SupOutEnth, state.dataEnvrn->OutBaroPress) > this->SupOutTemp) {
                this->SupOutTemp = Psychrometrics::PsyTsatFnHPb(state, this->SupOutEnth, state.dataEnvrn->OutBaroPress);
                this->SupOutHumRat = Psychrometrics::PsyWFnTdbH(state, this->SupOutTemp, this->SupOutEnth);
                QSensTrans = CSup * (this->SupInTemp - this->SupOutTemp);
                // Should we be updating the sensible and latent effectiveness values also?
            }

            this->SecOutEnth = this->SecInEnth + QTotTrans / this->SecOutMassFlow;
            this->SecOutTemp = this->SecInTemp + QSensTrans / CSec;
            this->SecOutHumRat = Psychrometrics::PsyWFnTdbH(state, this->SecOutTemp, this->SecOutEnth);
            break;
        default:
            break; // :: None means don't do anything here, and ::Invalid is caught on GetInput
        }

        this->DefrostFraction = DFFraction;
    }

    void HeatExchCond::UpdateHeatRecovery(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Michael Wetter
        //       DATE WRITTEN   March 1999
        //       MODIFIED       Fred Buhl November 2000
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Moves heat exchanger output to the outlet nodes.

        int const SupInNode = this->SupInletNode;
        int const SupOutNode = this->SupOutletNode;
        int const SecInNode = this->SecInletNode;
        int const SecOutNode = this->SecOutletNode;

        auto &thisSupInNode = state.dataLoopNodes->Node(SupInNode);
        auto &thisSupOutNode = state.dataLoopNodes->Node(SupOutNode);
        auto &thisSecInNode = state.dataLoopNodes->Node(SecInNode);
        auto &thisSecOutNode = state.dataLoopNodes->Node(SecOutNode);

        // Set the outlet air nodes of the heat exchanger
        thisSupOutNode.Temp = this->SupOutTemp;
        thisSupOutNode.HumRat = this->SupOutHumRat;
        thisSupOutNode.Enthalpy = this->SupOutEnth;
        thisSupOutNode.MassFlowRate = this->SupOutMassFlow;
        thisSecOutNode.Temp = this->SecOutTemp;
        thisSecOutNode.HumRat = this->SecOutHumRat;
        thisSecOutNode.Enthalpy = this->SecOutEnth;
        thisSecOutNode.MassFlowRate = this->SecOutMassFlow;

        // Set the outlet nodes for properties that just pass through & not used
        thisSupOutNode.Quality = thisSupInNode.Quality;
        thisSupOutNode.Press = thisSupInNode.Press;
        thisSupOutNode.MassFlowRateMin = thisSupInNode.MassFlowRateMin;
        thisSupOutNode.MassFlowRateMax = thisSupInNode.MassFlowRateMax;
        thisSupOutNode.MassFlowRateMinAvail = thisSupInNode.MassFlowRateMinAvail;
        thisSupOutNode.MassFlowRateMaxAvail = thisSupInNode.MassFlowRateMaxAvail;
        thisSecOutNode.Quality = thisSecInNode.Quality;
        thisSecOutNode.Press = thisSecInNode.Press;
        thisSecOutNode.MassFlowRateMin = thisSecInNode.MassFlowRateMin;
        thisSecOutNode.MassFlowRateMax = thisSecInNode.MassFlowRateMax;
        thisSecOutNode.MassFlowRateMinAvail = thisSecInNode.MassFlowRateMinAvail;
        thisSecOutNode.MassFlowRateMaxAvail = thisSecInNode.MassFlowRateMaxAvail;

        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            thisSupOutNode.CO2 = thisSupInNode.CO2;
            thisSecOutNode.CO2 = thisSecInNode.CO2;
        }
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            thisSupOutNode.GenContam = thisSupInNode.GenContam;
            thisSecOutNode.GenContam = thisSecInNode.GenContam;
        }
    }

    void HeatExchCond::ReportHeatRecovery(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Michael Wetter
        //       DATE WRITTEN   March 1999
        //       MODIFIED       F Buhl Nov 2000, D Shirey Feb/June 2003
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Fill remaining report variables

        Real64 const TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;
        this->ElecUseEnergy = this->ElecUseRate * TimeStepSysSec;
        this->SensHeatingEnergy = this->SensHeatingRate * TimeStepSysSec;
        this->LatHeatingEnergy = this->LatHeatingRate * TimeStepSysSec;
        this->TotHeatingEnergy = this->TotHeatingRate * TimeStepSysSec;
        this->SensCoolingEnergy = this->SensCoolingRate * TimeStepSysSec;
        this->LatCoolingEnergy = this->LatCoolingRate * TimeStepSysSec;
        this->TotCoolingEnergy = this->TotCoolingRate * TimeStepSysSec;

        state.dataHVACGlobal->AirToAirHXElecPower = this->ElecUseRate;
    }

    Real64 SafeDiv(Real64 const a, Real64 const b)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Michael Wetter
        //       DATE WRITTEN   March 1999
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Returns a / b while preventing division by zero

        // METHODOLOGY EMPLOYED:
        // Check for small or zero values before performing division

        if (std::abs(b) < SMALL) {
            return a / sign(SMALL, b);
        } else {
            return a / b;
        }
    }

    Real64 CalculateEpsFromNTUandZ(EnergyPlusData &state,
                                   Real64 const NTU,             // number of transfer units
                                   Real64 const Z,               // capacity rate ratio
                                   HXConfiguration const FlowArr // flow arrangement
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Michael Wetter
        //       DATE WRITTEN   March 1999
        //       MODIFIED       Fred Buhl November 2000
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates eps, the exchanger effectiveness,
        // from NTU, the number of transfer units,
        // from Z, the capacity rate ratio, and
        // from the flow arrangement

        // METHODOLOGY EMPLOYED:
        // Uses the effectiveness - NTU heat exchanger formulas

        // REFERENCES:
        // M. Wetter, Simulation Model Air-to-Air Plate Heat Exchanger
        // LBNL Report 42354, 1999.
        // Also see:
        // ASHRAE HVAC 2 Toolkit, pages 4-3 through 4-5

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 Temp;      // temporary variable
        Real64 Eps = 0.0; // heat exchanger effectiveness, return value

        // check input validity
        if (Z < 0.0 || Z > 1.0) {
            ShowFatalError(state, format("Variable Z ({:.2R}) out of range [0.0,1.0] in CalculateEpsFromNTUandZ", Z));
        }

        // effectiveness
        if (NTU < SMALL) {
            Eps = 0.0;
        } else if (Z < SMALL) { // Eps independent of flow arrangement
            Eps = 1.0 - std::exp(-NTU);
        } else {
            switch (FlowArr) {
            case HXConfiguration::CounterFlow: { // COUNTER FLOW
                if (std::abs(Z - 1.0) < SMALL) {
                    Eps = NTU / (NTU + 1.0);
                } else {
                    Temp = std::exp(-NTU * (1.0 - Z));
                    Eps = (1.0 - Temp) / (1.0 - Z * Temp);
                }
            } break;
            case HXConfiguration::ParallelFlow: { // PARALLEL FLOW
                Temp = (1.0 + Z);
                Eps = (1.0 - std::exp(-NTU * Temp)) / Temp;
            } break;
            case HXConfiguration::CrossFlowBothUnmixed: { // CROSS FLOW BOTH UNMIXED
                Temp = Z * std::pow(NTU, -0.22);
                Eps = 1.0 - std::exp(std::expm1(-NTU * Temp) / Temp);
            } break;
            case HXConfiguration::CrossFlowOther: { // CROSS FLOW, Cmax MIXED, Cmin UNMIXED
                Eps = (1.0 - std::exp(-Z * (1.0 - std::exp(-NTU)))) / Z;
            } break;
            default: {
                ShowFatalError(state, format("HeatRecovery: Illegal flow arrangement in CalculateEpsFromNTUandZ, Value={}", FlowArr));
            } break;
            }
        }
        return Eps;
    }

    void CalculateNTUfromEpsAndZ(EnergyPlusData &state,
                                 Real64 &NTU,                   // number of transfer units
                                 CalculateNTUBoundsErrors &Err, // error indicator
                                 Real64 const Z,                // capacity rate ratio
                                 HXConfiguration const FlowArr, // flow arrangement
                                 Real64 const Eps               // heat exchanger effectiveness
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Michael Wetter
        //       DATE WRITTEN   March 1999
        //       MODIFIED       Fred Buhl November 2000
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates NTU, the number of transfer units,
        // based on eps, the exchanger effectiveness,
        // Z, the capacity rate ratio, and
        // from the flow arrangement

        // METHODOLOGY EMPLOYED:
        // Uses the effectiveness - NTU heat exchanger formulas

        // REFERENCES:
        // M. Wetter, Simulation Model Air-to-Air Plate Heat Exchanger
        // LBNL Report 42354, 1999.
        // Also see:
        // ASHRAE HVAC 2 Toolkit, pages 4-3 through 4-5

        NTU = 0.0;
        // check input validity
        if (Z < 0.0 || Z > 1.0) {
            Err = CalculateNTUBoundsErrors::MassFlowRatio;
            return;
        }

        if (FlowArr == HXConfiguration::ParallelFlow) {
            if (Eps < 0.0 || Eps > 1.0 / (1.0 + Z)) {
                Err = CalculateNTUBoundsErrors::NominalEffectiveness1;
                return;
            }
        } else if (FlowArr == HXConfiguration::CrossFlowOther) {
            if (Eps < 0.0 || Eps > (1.0 - std::exp(-Z)) / Z) {
                Err = CalculateNTUBoundsErrors::NominalEffectiveness2;
                return;
            }
            // check product (Eps*Z)
            if (Eps * Z < 0.0 || Eps * Z > 1.0 - std::exp(Z * (SMALL - 1.0))) {
                Err = CalculateNTUBoundsErrors::Quantity;
                return;
            }
            // check product (Eps*Z)
        } else {
            if (Eps < 0.0 || Eps > 1.0) {
                Err = CalculateNTUBoundsErrors::NominalEffectiveness3;
                return;
            }
        }

        if (Eps < SMALL) { // no effectiveness. Set NTU = 0
            NTU = 0.0;
        } else if (Z < SMALL) { // Eps independent of flow arrangement
            NTU = -std::log(1.0 - Eps);
        } else {
            // calculate based on configuration
            switch (FlowArr) {
            case HXConfiguration::CounterFlow: { // COUNTER FLOW
                if (std::abs(Z - 1.0) < SMALL) {
                    NTU = Eps / (1.0 - Eps);
                } else {
                    NTU = 1.0 / (Z - 1.0) * std::log((1.0 - Eps) / (1.0 - Eps * Z));
                }
            } break;
            case HXConfiguration::ParallelFlow: { // PARALLEL FLOW
                NTU = -std::log1p(-Eps - Eps * Z) / (Z + 1.0);
            } break;
            case HXConfiguration::CrossFlowBothUnmixed: { // CROSS FLOW BOTH UNMIXED
                NTU = GetNTUforCrossFlowBothUnmixed(state, Eps, Z);
            } break;
            case HXConfiguration::CrossFlowOther: { // CROSS FLOW, Cmax MIXED, Cmin UNMIXED
                NTU = -std::log1p(std::log(1.0 - Eps * Z) / Z);
            } break;
            default: {
                ShowFatalError(state, format("HeatRecovery: Illegal flow arrangement in CalculateNTUfromEpsAndZ, Value={}", FlowArr));
            } break;
            }
        }
    }

    Real64 GetNTUforCrossFlowBothUnmixed(EnergyPlusData &state,
                                         Real64 const Eps, // heat exchanger effectiveness
                                         Real64 const Z    // capacity rate ratio
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Michael Wetter
        //       DATE WRITTEN   March 1999
        //       MODIFIED       Fred Buhl November 2000
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Calculates the NTU value based on the exchanger effectiveness
        // and the capacity ratio for cross flow exchanger, both
        // streams unmixed

        // METHODOLOGY EMPLOYED:
        // Uses a Regula Falsi solver function to numerically invert the formula
        // giving effectiveness as a function of NTU and Z..

        // REFERENCES:
        // M. Wetter, Simulation Model Air-to-Air Plate Heat Exchanger
        // LBNL Report 42354, 1999.
        // Also see:
        // ASHRAE HVAC 2 Toolkit, pages 4-3 through 4-5

        // Return value
        Real64 NTU; // result variable; number of transfer units

        // FUNCTION PARAMETER DEFINITIONS:
        Real64 constexpr Acc(0.0001); // Accuracy of result
        int constexpr MaxIte(500);    // Maximum number of iterations

        int SolFla;                  // Flag of solver
        Real64 constexpr NTU0(0.0);  // lower bound for NTU
        Real64 constexpr NTU1(50.0); // upper bound for NTU
        auto f = [Eps, Z](Real64 const NTU) { return 1.0 - std::exp(std::expm1(-std::pow(NTU, 0.78) * Z) / Z * std::pow(NTU, 0.22)) - Eps; };
        General::SolveRoot(state, Acc, MaxIte, SolFla, NTU, f, NTU0, NTU1);

        if (SolFla == -2) {
            ShowFatalError(state, "HeatRecovery: Bad initial bounds for NTU in GetNTUforCrossFlowBothUnmixed");
        } else if (SolFla == -1) {
            ShowFatalError(state, "HeatRecovery: No convergence in solving for NTU in GetNTUforCrossFlowBothUnmixed");
        }

        return NTU;
    }

    void HeatExchCond::CheckModelBoundsTempEq(EnergyPlusData &state,
                                              Real64 &T_RegenInTemp,        // current regen inlet temperature (C) for regen outlet temp eqn
                                              Real64 &T_RegenInHumRat,      // current regen inlet hum rat for regen outlet temp eqn
                                              Real64 &T_ProcInTemp,         // current process inlet temperature (C) for regen outlet temp eqn
                                              Real64 &T_ProcInHumRat,       // current process inlet hum rat for regen outlet temp eqn
                                              Real64 &T_FaceVel,            // current process and regen face velocity (m/s)
                                              bool const FirstHVACIteration // First HVAC iteration flag
    ) const
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Mangesh Basarkar, FSEC
        //       DATE WRITTEN   January 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // To verify that the empirical model's independent variables are within the limits used during the
        // development of the empirical model.

        // METHODOLOGY EMPLOYED:
        // The empirical models used for simulating a desiccant enhanced cooling coil are based on a limited data set.
        // Extrapolation of empirical models can cause instability and the independent variables may need to be limited.
        // The range of each independent variable is provided by the user and are based on the limits of the
        // empirical model. These limits are tested in this subroutine each time step and returned for use by the calling
        // routine.

        // Using/Aliasing
        using General::CreateSysTimeIntervalString;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        auto &thisError = state.dataHeatRecovery->error1;
        Real64 SysTimeElapsed = state.dataHVACGlobal->SysTimeElapsed;
        Real64 TimeStepSys = state.dataHVACGlobal->TimeStepSys;

        // current end time is compared with last to see if time step changed

        //   calculate end time of current time step
        thisError.CurrentEndTime = state.dataGlobal->CurrentTime + SysTimeElapsed;

        //   Print warning messages only when valid and only for the first occurrence. Let summary provide statistics.
        //   Wait for next time step to print warnings. If simulation iterates, print out
        //   the warning for the last iteration only. Must wait for next time step to accomplish this.
        //   If a warning occurs and the simulation down shifts, the warning is not valid.
        if (thisError.CurrentEndTime > thisError.CurrentEndTimeLast && TimeStepSys >= thisError.TimeStepSysLast) {

            // print error for variables of regeneration outlet temperature equation
            // Regen inlet temp for temp eqn
            if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInTempError.print) {
                ++state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInTempError.count;
                if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInTempError.count < 2) {
                    ShowWarningError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInTempError.buffer1);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInTempError.buffer2);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInTempError.buffer3);
                    ShowContinueError(state,
                                      "...Using regeneration inlet air temperatures that are outside the regeneration outlet air temperature "
                                      "equation model boundaries may adversely affect desiccant model performance.");
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   format("{} \"{}\" - Regeneration inlet air temp used in regen outlet air temperature equation is "
                                                          "out of range error continues...",
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name),
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInTempError.index,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInTempError.last,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInTempError.last);
                }
            }
            // Regen inlet humidity ratio for temp eqn
            if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInHumRatError.print) {
                ++state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInHumRatError.count;
                if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInHumRatError.count < 2) {
                    ShowWarningError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInHumRatError.buffer1);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInHumRatError.buffer2);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInHumRatError.buffer3);
                    ShowContinueError(state,
                                      "...Using regeneration inlet air humidity ratios that are outside the regeneration outlet air temperature "
                                      "equation model boundaries may adversely affect desiccant model performance.");
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   format("{} \"{}\" - Regeneration inlet air humidity ratio used in regen outlet temperature "
                                                          "equation is out of range error continues...",
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name),
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInHumRatError.index,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInHumRatError.last,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInHumRatError.last);
                }
            }
            // Process inlet temp for temp eqn
            if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInTempError.print) {
                ++state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInTempError.count;
                if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInTempError.count < 2) {
                    ShowWarningError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInTempError.buffer1);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInTempError.buffer2);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInTempError.buffer3);
                    ShowContinueError(state,
                                      "...Using process inlet air temperatures that are outside the regeneration outlet air temperature equation "
                                      "model boundaries may adversely affect desiccant model performance.");
                } else {
                    ShowRecurringWarningErrorAtEnd(
                        state,
                        format(
                            "{} \"{}\" - Process inlet air temperature used in regen outlet temperature equation is out of range error continues...",
                            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name),
                        state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInTempError.index,
                        state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInTempError.last,
                        state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInTempError.last);
                }
            }
            // Process inlet humidity ratio for temp eqn
            if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInHumRatError.print) {
                ++state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInHumRatError.count;
                if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInHumRatError.count < 2) {
                    ShowWarningError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInHumRatError.buffer1);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInHumRatError.buffer2);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInHumRatError.buffer3);
                    ShowContinueError(state,
                                      "...Using process inlet air humidity ratios that are outside the regeneration outlet air temperature equation "
                                      "model boundaries may adversely affect desiccant model performance.");
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   format("{} \"{}\" - Process inlet air humidity ratio used in regen outlet temperature equation is "
                                                          "out of range error continues...",
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name),
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInHumRatError.index,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInHumRatError.last,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInHumRatError.last);
                }
            }
            // Process and regeneration face velocity for temp eqn
            if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_FaceVelError.print) {
                ++state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_FaceVelError.count;
                if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_FaceVelError.count < 2) {
                    ShowWarningError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_FaceVelError.buffer1);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_FaceVelError.buffer2);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_FaceVelError.buffer3);
                    ShowContinueError(state,
                                      "...Using process and regeneration face velocities that are outside the regeneration outlet air temperature "
                                      "equation model boundaries may adversely affect desiccant model performance.");
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   format("{} \"{}\" - Process and regen inlet air face velocity used in regen outlet temperature "
                                                          "equation is out of range error continues...",
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name),
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_FaceVelError.index,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_FaceVelError.last,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_FaceVelError.last);
                }
            }
        } // IF(CurrentEndTime .GT. CurrentEndTimeLast .AND. TimeStepSys .GE. TimeStepSysLast)THEN

        //   save last system time step and last end time of current time step (used to determine if warning is valid)
        thisError.TimeStepSysLast = TimeStepSys;
        thisError.CurrentEndTimeLast = thisError.CurrentEndTime;

        //   If regen and process inlet temperatures are the same the coil is off, do not print out of bounds warning for this case
        if (std::abs(T_RegenInTemp - T_ProcInTemp) < SMALL) {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInTempError.print = false;
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInHumRatError.print = false;
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInTempError.print = false;
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInHumRatError.print = false;
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_FaceVelError.print = false;
            return;
        }

        //   check boundaries of independent variables and post warnings to individual buffers to print at end of time step
        // checking model bounds for variables of regeneration outlet temperature equation
        // Regen inlet temp
        if (T_RegenInTemp < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MinRegenAirInTemp ||
            T_RegenInTemp > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MaxRegenAirInTemp) {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInTempError.last = T_RegenInTemp;
            thisError.OutputChar = format("{:.2R}", T_RegenInTemp);
            thisError.OutputCharLo = format("{:.2R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MinRegenAirInTemp);
            thisError.OutputCharHi = format("{:.2R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MaxRegenAirInTemp);
            if (T_RegenInTemp < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MinRegenAirInTemp) {
                T_RegenInTemp = state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MinRegenAirInTemp;
            }
            if (T_RegenInTemp > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MaxRegenAirInTemp) {
                T_RegenInTemp = state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MaxRegenAirInTemp;
            }
            if (!state.dataGlobal->WarmupFlag && !FirstHVACIteration) {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInTempError.print = true;
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInTempError.buffer1 = format(
                    "{} \"{}\" - Regeneration inlet air temperature used in regen outlet air temperature equation is outside model boundaries at {}.",
                    state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                    state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name,
                    thisError.OutputChar);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInTempError.buffer2 =
                    format("...Valid range = {} to {}. Occurrence info = {}, {} {}",
                           thisError.OutputCharLo,
                           thisError.OutputCharHi,
                           state.dataEnvrn->EnvironmentName,
                           state.dataEnvrn->CurMnDy,
                           CreateSysTimeIntervalString(state));
                thisError.CharValue = format("{:.6R}", T_RegenInTemp);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInTempError.buffer3 =
                    format("...Regeneration outlet air temperature equation: regeneration inlet air temperature passed to the model = {}",
                           thisError.CharValue);
            } else {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInTempError.print = false;
            }
        } else {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInTempError.print = false;
        }
        // regen inlet humidity ratio
        if (T_RegenInHumRat < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MinRegenAirInHumRat ||
            T_RegenInHumRat > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MaxRegenAirInHumRat) {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInHumRatError.last = T_RegenInHumRat;
            thisError.OutputChar = format("{:.6R}", T_RegenInHumRat);
            thisError.OutputCharLo = format("{:.6R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MinRegenAirInHumRat);
            thisError.OutputCharHi = format("{:.6R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MaxRegenAirInHumRat);
            if (T_RegenInHumRat < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MinRegenAirInHumRat) {
                T_RegenInHumRat = state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MinRegenAirInHumRat;
            }
            if (T_RegenInHumRat > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MaxRegenAirInHumRat) {
                T_RegenInHumRat = state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MaxRegenAirInHumRat;
            }
            if (!state.dataGlobal->WarmupFlag && !FirstHVACIteration) {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInHumRatError.print = true;
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInHumRatError.buffer1 =
                    format("{} \"{}\" - Regeneration inlet air humidity ratio used in regen outlet air temperature equation is outside model "
                           "boundaries at {}.",
                           state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                           state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name,
                           thisError.OutputChar);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInHumRatError.buffer2 =
                    format("...Valid range = {} to {}. Occurrence info = {}, {} {}",
                           thisError.OutputCharLo,
                           thisError.OutputCharHi,
                           state.dataEnvrn->EnvironmentName,
                           state.dataEnvrn->CurMnDy,
                           CreateSysTimeIntervalString(state));
                thisError.CharValue = format("{:.6R}", T_RegenInHumRat);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInHumRatError.buffer3 =
                    format("...Regeneration outlet air temperature equation: regeneration inlet air humidity ratio passed to the model = {}",
                           thisError.CharValue);
            } else {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInHumRatError.print = false;
            }
        } else {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_RegenInHumRatError.print = false;
        }
        // process inlet temp
        if (T_ProcInTemp < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MinProcAirInTemp ||
            T_ProcInTemp > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MaxProcAirInTemp) {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInTempError.last = T_ProcInTemp;
            thisError.OutputChar = format("{:.2R}", T_ProcInTemp);
            thisError.OutputCharLo = format("{:.2R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MinProcAirInTemp);
            thisError.OutputCharHi = format("{:.2R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MaxProcAirInTemp);
            if (T_ProcInTemp < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MinProcAirInTemp) {
                T_ProcInTemp = state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MinProcAirInTemp;
            }
            if (T_ProcInTemp > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MaxProcAirInTemp) {
                T_ProcInTemp = state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MaxProcAirInTemp;
            }
            if (!state.dataGlobal->WarmupFlag && !FirstHVACIteration) {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInTempError.print = true;
                //       Suppress warning message when process inlet temperature = 0 (DX coil is off)
                if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInTempError.last == 0.0) {
                    state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInTempError.print = false;
                }
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInTempError.buffer1 = format(
                    "{} \"{}\" - Process inlet air temperature used in regen outlet air temperature equation is outside model boundaries at {}.",
                    state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                    state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name,
                    thisError.OutputChar);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInTempError.buffer2 =
                    format("...Valid range = {} to {}. Occurrence info = {},{} {}",
                           thisError.OutputCharLo,
                           thisError.OutputCharHi,
                           state.dataEnvrn->EnvironmentName,
                           state.dataEnvrn->CurMnDy,
                           CreateSysTimeIntervalString(state));
                thisError.CharValue = format("{:.6R}", T_ProcInTemp);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInTempError.buffer3 = format(
                    "...Regeneration outlet air temperature equation: process inlet air temperature passed to the model = {}", thisError.CharValue);
            } else {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInTempError.print = false;
            }
        } else {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInTempError.print = false;
        }
        // process inlet humidity ratio
        if (T_ProcInHumRat < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MinProcAirInHumRat ||
            T_ProcInHumRat > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MaxProcAirInHumRat) {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInHumRatError.last = T_ProcInHumRat;
            thisError.OutputChar = format("{:.6R}", T_ProcInHumRat);
            thisError.OutputCharLo = format("{:.6R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MinProcAirInHumRat);
            thisError.OutputCharHi = format("{:.6R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MaxProcAirInHumRat);
            if (T_ProcInHumRat < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MinProcAirInHumRat) {
                T_ProcInHumRat = state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MinProcAirInHumRat;
            }
            if (T_ProcInHumRat > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MaxProcAirInHumRat) {
                T_ProcInHumRat = state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MaxProcAirInHumRat;
            }
            if (!state.dataGlobal->WarmupFlag && !FirstHVACIteration) {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInHumRatError.print = true;
                //       Suppress warning message when process inlet humrat = 0 (DX coil is off)
                if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInHumRatError.last == 0.0) {
                    state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInHumRatError.print = false;
                }
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInHumRatError.buffer1 = format(
                    "{} \"{}\" - Process inlet air humidity ratio used in regen outlet air temperature equation is outside model boundaries at {}.",
                    state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                    state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name,
                    thisError.OutputChar);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInHumRatError.buffer2 =
                    format("...Valid range = {} to {}. Occurrence info = {}, {} {}",
                           thisError.OutputCharLo,
                           thisError.OutputCharHi,
                           state.dataEnvrn->EnvironmentName,
                           state.dataEnvrn->CurMnDy,
                           CreateSysTimeIntervalString(state));
                thisError.CharValue = format("{:.6R}", T_ProcInHumRat);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInHumRatError.buffer3 =
                    format("...Regeneration outlet air temperature equation: process inlet air humidity ratio passed to the model = {}",
                           thisError.CharValue);
            } else {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInHumRatError.print = false;
            }
        } else {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInHumRatError.print = false;
        }
        // regeneration and process face velocity
        if (T_FaceVel < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MinFaceVel ||
            T_FaceVel > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MaxFaceVel) {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_FaceVelError.last = T_FaceVel;
            thisError.OutputChar = format("{:.6R}", T_FaceVel);
            thisError.OutputCharLo = format("{:.6R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MinFaceVel);
            thisError.OutputCharHi = format("{:.6R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MaxFaceVel);
            if (T_FaceVel < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MinFaceVel) {
                T_FaceVel = state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MinFaceVel;
            }
            if (T_FaceVel > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MaxFaceVel) {
                T_FaceVel = state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MaxFaceVel;
            }
            if (!state.dataGlobal->WarmupFlag && !FirstHVACIteration) {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_FaceVelError.print = true;
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_FaceVelError.buffer1 =
                    format("{} \"{}\" - Process and regen inlet air face velocity used in regen outlet air temperature equation is outside model "
                           "boundaries at {}.",
                           state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                           state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name,
                           thisError.OutputChar);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_FaceVelError.buffer2 =
                    format("...Valid range = {} to {}. Occurrence info = {}, {} {}",
                           thisError.OutputCharLo,
                           thisError.OutputCharHi,
                           state.dataEnvrn->EnvironmentName,
                           state.dataEnvrn->CurMnDy,
                           CreateSysTimeIntervalString(state));
                thisError.CharValue = format("{:.6R}", T_FaceVel);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_FaceVelError.buffer3 = format(
                    "...Regeneration outlet air temperature equation: process and regen face velocity passed to the model = {}", thisError.CharValue);
            } else {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_FaceVelError.print = false;
            }
        } else {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_FaceVelError.print = false;
        }
    }

    void HeatExchCond::CheckModelBoundsHumRatEq(EnergyPlusData &state,
                                                Real64 &H_RegenInTemp,        // current regen inlet temperature (C) for regen outlet hum rat eqn
                                                Real64 &H_RegenInHumRat,      // current regen inlet hum rat for regen outlet hum rat eqn
                                                Real64 &H_ProcInTemp,         // current process inlet temperature (C) for regen outlet hum rat eqn
                                                Real64 &H_ProcInHumRat,       // current process inlet hum rat for regen outlet hum rat eqn
                                                Real64 &H_FaceVel,            // current process and regen face velocity (m/s)
                                                bool const FirstHVACIteration // First HVAC iteration flag
    ) const
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Mangesh Basarkar, FSEC
        //       DATE WRITTEN   January 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // To verify that the empirical model's independent variables are within the limits used during the
        // development of the empirical model.

        // METHODOLOGY EMPLOYED:
        // The empirical models used for simulating a desiccant enhanced cooling coil are based on a limited data set.
        // Extrapolation of empirical models can cause instability and the independent variables may need to be limited.
        // The range of each independent variable is provided by the user and are based on the limits of the
        // empirical model. These limits are tested in this subroutine each time step and returned for use by the calling
        // routine.

        // Using/Aliasing
        using General::CreateSysTimeIntervalString;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        auto &thisError = state.dataHeatRecovery->error2;

        Real64 SysTimeElapsed = state.dataHVACGlobal->SysTimeElapsed;
        Real64 TimeStepSys = state.dataHVACGlobal->TimeStepSys;
        // current end time is compared with last to see if time step changed

        //   calculate end time of current time step
        thisError.CurrentEndTime = state.dataGlobal->CurrentTime + SysTimeElapsed;

        //   Print warning messages only when valid and only for the first occurrence. Let summary provide statistics.
        //   Wait for next time step to print warnings. If simulation iterates, print out
        //   the warning for the last iteration only. Must wait for next time step to accomplish this.
        //   If a warning occurs and the simulation down shifts, the warning is not valid.
        if (thisError.CurrentEndTime > thisError.CurrentEndTimeLast && TimeStepSys >= thisError.TimeStepSysLast) {

            // print error for variables of regeneration outlet humidity ratio equation
            // Regen inlet temp for humidity ratio eqn
            if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInTempError.print) {
                ++state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInTempError.count;
                if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInTempError.count < 2) {
                    ShowWarningError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInTempError.buffer1);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInTempError.buffer2);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInTempError.buffer3);
                    ShowContinueError(state,
                                      "...Using regeneration inlet air temperatures that are outside the regeneration inlet air temperature equation "
                                      "model boundaries may adversely affect desiccant model performance.");
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   format("{} \"{}\" - Regeneration inlet air temperature used in regen outlet air humidity ratio "
                                                          "equation is out of range error continues...",
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name),
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInTempError.index,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInTempError.last,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInTempError.last);
                }
            }
            // Regen inlet humidity ratio for humidity ratio eqn
            if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInHumRatError.print) {
                ++state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInHumRatError.count;
                if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInHumRatError.count < 2) {
                    ShowWarningError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInHumRatError.buffer1);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInHumRatError.buffer2);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInHumRatError.buffer3);
                    ShowContinueError(state,
                                      "...Using regeneration inlet air humidity ratios that are outside the regeneration outlet air humidity ratio "
                                      "equation model boundaries may adversely affect desiccant model performance.");
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   format("{} \"{}\" - Regeneration inlet air humidity ratio used in regen outlet air humidity ratio "
                                                          "equation is out of range error continues...",
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name),
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInHumRatError.index,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInHumRatError.last,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInHumRatError.last);
                }
            }
            // Process inlet temp for humidity ratio eqn
            if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInTempError.print) {
                ++state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInTempError.count;
                if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInTempError.count < 2) {
                    ShowWarningError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInTempError.buffer1);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInTempError.buffer2);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInTempError.buffer3);
                    ShowContinueError(state,
                                      "...Using process inlet air temperatures that are outside the regeneration outlet air humidity ratio equation "
                                      "model may adversely affect desiccant model performance.");
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   format("{} \"{}\" - Process inlet air temperature used in regen outlet air humidity ratio "
                                                          "equation is out of range error continues...",
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name),
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInTempError.index,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInTempError.last,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInTempError.last);
                }
            }
            // Process inlet humidity ratio for humidity ratio eqn
            if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInHumRatError.print) {
                ++state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInHumRatError.count;
                if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInHumRatError.count < 2) {
                    ShowWarningError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInHumRatError.buffer1);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInHumRatError.buffer2);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInHumRatError.buffer3);
                    ShowContinueError(state,
                                      "...Using process inlet air humidity ratios that are outside the regeneration outlet humidity ratio equation "
                                      "model boundaries may adversely affect desiccant model performance.");
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   format("{} \"{}\" - Process inlet air humidity ratio used in regen outlet air humidity ratio "
                                                          "equation is out of range error continues...",
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name),
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_ProcInHumRatError.index,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInHumRatError.last,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInHumRatError.last);
                }
            }
            // Process and regeneration face velocity for humidity ratio eqn
            if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_FaceVelError.print) {
                ++state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_FaceVelError.count;
                if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_FaceVelError.count < 2) {
                    ShowWarningError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_FaceVelError.buffer1);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_FaceVelError.buffer2);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_FaceVelError.buffer3);
                    ShowContinueError(state,
                                      "...Using process and regeneration face velocities that are outside the regeneration outlet air humidity ratio "
                                      "equation model boundaries may adversely affect desiccant model performance.");
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   format("{} \"{}\" - Process and regen face velocity used in regen outlet air humidity ratio "
                                                          "equation is out of range error continues...",
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name),
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_FaceVelError.index,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_FaceVelError.last,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_FaceVelError.last);
                }
            }
        } // IF(CurrentEndTime .GT. CurrentEndTimeLast .AND. TimeStepSys .GE. TimeStepSysLast)THEN

        //   save last system time step and last end time of current time step (used to determine if warning is valid)
        thisError.TimeStepSysLast = TimeStepSys;
        thisError.CurrentEndTimeLast = thisError.CurrentEndTime;

        //   If regen and process inlet temperatures are the same the coil is off, do not print out of bounds warning for this case
        if (std::abs(H_RegenInTemp - H_ProcInTemp) < SMALL) {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInTempError.print = false;
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInHumRatError.print = false;
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInTempError.print = false;
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInHumRatError.print = false;
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_FaceVelError.print = false;
            return;
        }

        //   check boundaries of independent variables and post warnings to individual buffers to print at end of time step
        // checking model bounds for variables of regeneration outlet humidity ratio equation
        // Regen inlet temp
        if (H_RegenInTemp < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MinRegenAirInTemp ||
            H_RegenInTemp > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MaxRegenAirInTemp) {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInTempError.last = H_RegenInTemp;
            thisError.OutputChar = format("{:.2R}", H_RegenInTemp);
            thisError.OutputCharLo = format("{:.2R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MinRegenAirInTemp);
            thisError.OutputCharHi = format("{:.2R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MaxRegenAirInTemp);
            if (H_RegenInTemp < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MinRegenAirInTemp) {
                H_RegenInTemp = state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MinRegenAirInTemp;
            }
            if (H_RegenInTemp > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MaxRegenAirInTemp) {
                H_RegenInTemp = state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MaxRegenAirInTemp;
            }
            if (!state.dataGlobal->WarmupFlag && !FirstHVACIteration) {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInTempError.print = true;
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInTempError.buffer1 =
                    format("{} \"{}\" - Regeneration inlet air temperature used in regen outlet air humidity ratio equation is outside model "
                           "boundaries at {}.",
                           state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                           state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name,
                           thisError.OutputChar);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInTempError.buffer2 =
                    format("...Valid range = {} to {}. Occurrence info = {}, {} , {}",
                           thisError.OutputCharLo,
                           thisError.OutputCharHi,
                           state.dataEnvrn->EnvironmentName,
                           state.dataEnvrn->CurMnDy,
                           CreateSysTimeIntervalString(state));
                thisError.CharValue = format("{:.2R}", H_RegenInTemp);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInTempError.buffer3 =
                    format("...Regeneration outlet air humidity ratio equation: regeneration inlet air temperature passed to the model = {}",
                           thisError.CharValue);
            } else {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInTempError.print = false;
            }
        } else {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInTempError.print = false;
        }
        // regen inlet humidity ratio
        if (H_RegenInHumRat < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MinRegenAirInHumRat ||
            H_RegenInHumRat > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MaxRegenAirInHumRat) {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInHumRatError.last = H_RegenInHumRat;
            thisError.OutputChar = format("{:.6R}", H_RegenInHumRat);
            thisError.OutputCharLo = format("{:.6R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MinRegenAirInHumRat);
            thisError.OutputCharHi = format("{:.6R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MaxRegenAirInHumRat);
            if (H_RegenInHumRat < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MinRegenAirInHumRat) {
                H_RegenInHumRat = state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MinRegenAirInHumRat;
            }
            if (H_RegenInHumRat > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MaxRegenAirInHumRat) {
                H_RegenInHumRat = state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MaxRegenAirInHumRat;
            }
            if (!state.dataGlobal->WarmupFlag && !FirstHVACIteration) {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInHumRatError.print = true;
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInHumRatError.buffer1 =
                    format("{} \"{}\" - Regeneration inlet air humidity ratio used in regen outlet air humidity ratio equation is outside model "
                           "boundaries at {}.",
                           state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                           state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name,
                           thisError.OutputChar);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInHumRatError.buffer2 =
                    format("...Valid range = {} to {}. Occurrence info = {}, {} {}",
                           thisError.OutputCharLo,
                           thisError.OutputCharHi,
                           state.dataEnvrn->EnvironmentName,
                           state.dataEnvrn->CurMnDy,
                           CreateSysTimeIntervalString(state));
                thisError.CharValue = format("{:.6R}", H_RegenInHumRat);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInHumRatError.buffer3 =
                    format("...Regeneration outlet air humidity ratio equation: regeneration inlet air humidity ratio passed to the model = {}",
                           thisError.CharValue);
            } else {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInHumRatError.print = false;
            }
        } else {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_RegenInHumRatError.print = false;
        }
        // process inlet temp
        if (H_ProcInTemp < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MinProcAirInTemp ||
            H_ProcInTemp > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MaxProcAirInTemp) {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInTempError.last = H_ProcInTemp;
            thisError.OutputChar = format("{:.2R}", H_ProcInTemp);
            thisError.OutputCharLo = format("{:.2R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MinProcAirInTemp);
            thisError.OutputCharHi = format("{:.2R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MaxProcAirInTemp);
            if (H_ProcInTemp < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MinProcAirInTemp) {
                H_ProcInTemp = state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MinProcAirInTemp;
            }
            if (H_ProcInTemp > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MaxProcAirInTemp) {
                H_ProcInTemp = state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MaxProcAirInTemp;
            }
            if (!state.dataGlobal->WarmupFlag && !FirstHVACIteration) {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInTempError.print = true;
                //       Suppress warning message when process inlet temperature = 0 (DX coil is off)
                if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInTempError.last == 0.0) {
                    state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInTempError.print = false;
                }
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInTempError.buffer1 = format(
                    "{} \"{}\" - Process inlet air temperature used in regen outlet air humidity ratio equation is outside model boundaries at {}.",
                    state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                    state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name,
                    thisError.OutputChar);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInTempError.buffer2 =
                    format("...Valid range = {} to {}. Occurrence info = {}, {} {}",
                           thisError.OutputCharLo,
                           thisError.OutputCharHi,
                           state.dataEnvrn->EnvironmentName,
                           state.dataEnvrn->CurMnDy,
                           CreateSysTimeIntervalString(state));
                thisError.CharValue = format("{:.6R}", H_ProcInTemp);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInTempError.buffer3 =
                    format("...Regeneration outlet air humidity ratio equation: process inlet air temperature passed to the model = {}",
                           thisError.CharValue);
            } else {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInTempError.print = false;
            }
        } else {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInTempError.print = false;
        }
        // process inlet humidity ratio
        if (H_ProcInHumRat < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MinProcAirInHumRat ||
            H_ProcInHumRat > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MaxProcAirInHumRat) {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInHumRatError.last = H_ProcInHumRat;
            thisError.OutputChar = format("{:.6R}", H_ProcInHumRat);
            thisError.OutputCharLo = format("{:.6R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MinProcAirInHumRat);
            thisError.OutputCharHi = format("{:.6R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MaxProcAirInHumRat);
            if (H_ProcInHumRat < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MinProcAirInHumRat) {
                H_ProcInHumRat = state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MinProcAirInHumRat;
            }
            if (H_ProcInHumRat > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MaxProcAirInHumRat) {
                H_ProcInHumRat = state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MaxProcAirInHumRat;
            }
            if (!state.dataGlobal->WarmupFlag && !FirstHVACIteration) {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInHumRatError.print = true;
                //       Suppress warning message when process inlet humrat = 0 (DX coil is off)
                if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInHumRatError.last == 0.0) {
                    state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInHumRatError.print = false;
                }
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInHumRatError.buffer1 =
                    format("{} \"{}\" - Process inlet air humidity ratio used in regen outlet air humidity ratio equation is outside model "
                           "boundaries at {}.",
                           state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                           state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name,
                           thisError.OutputChar);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInHumRatError.buffer2 =
                    format("...Valid range = {} to {}. Occurrence info = {}, {}, {}",
                           thisError.OutputCharLo,
                           thisError.OutputCharHi,
                           state.dataEnvrn->EnvironmentName,
                           state.dataEnvrn->CurMnDy,
                           CreateSysTimeIntervalString(state));
                thisError.CharValue = format("{:.6R}", H_ProcInHumRat);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInHumRatError.buffer3 =
                    format("...Regeneration outlet air humidity ratio equation: process inlet air humidity ratio passed to the model = {}",
                           thisError.CharValue);
            } else {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInHumRatError.print = false;
            }
        } else {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_ProcInHumRatError.print = false;
        }
        // regeneration and process face velocity
        if (H_FaceVel < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MinFaceVel ||
            H_FaceVel > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MaxFaceVel) {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_FaceVelError.last = H_FaceVel;
            thisError.OutputChar = format("{:.6R}", H_FaceVel);
            thisError.OutputCharLo = format("{:.6R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MinFaceVel);
            thisError.OutputCharHi = format("{:.6R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MaxFaceVel);
            if (H_FaceVel < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MinFaceVel) {
                H_FaceVel = state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MinFaceVel;
            }
            if (H_FaceVel > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MaxFaceVel) {
                H_FaceVel = state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MaxFaceVel;
            }
            if (!state.dataGlobal->WarmupFlag && !FirstHVACIteration) {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_FaceVelError.print = true;
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_FaceVelError.buffer1 =
                    format("{} \"{}\" - Process and regen inlet air face velocity used in regen outlet air humidity ratio equation is outside model "
                           "boundaries at {}.",
                           state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                           state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name,
                           thisError.OutputChar);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_FaceVelError.buffer2 =
                    format("...Valid range = {} to {}. Occurrence info = {}, {}, {}",
                           thisError.OutputCharLo,
                           thisError.OutputCharHi,
                           state.dataEnvrn->EnvironmentName,
                           state.dataEnvrn->CurMnDy,
                           CreateSysTimeIntervalString(state));
                thisError.CharValue = format("{:.6R}", H_FaceVel);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_FaceVelError.buffer3 =
                    format("...Regeneration outlet air humidity ratio equation: process and regeneration face velocity passed to the model = {}",
                           thisError.CharValue);
            } else {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_FaceVelError.print = false;
            }
        } else {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_FaceVelError.print = false;
        }
    }

    void HeatExchCond::CheckModelBoundOutput_Temp(EnergyPlusData &state,
                                                  Real64 const RegenInTemp,     // current regen inlet temp passed to eqn
                                                  Real64 &RegenOutTemp,         // current regen outlet temp from eqn
                                                  bool const FirstHVACIteration // First HVAC iteration flag
    ) const
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Mangesh Basarkar, FSEC
        //       DATE WRITTEN   January 2007
        //       MODIFIED       June 2007, R. Raustad, changed requirement that regen outlet temp be less than inlet temp
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // To verify that the empirical model's independent variables are within the limits used during the
        // development of the empirical model.

        // METHODOLOGY EMPLOYED:
        // The empirical models used for simulating a desiccant enhanced cooling coil are based on a limited data set.
        // Extrapolation of empirical models can cause instability and the independent variables may need to be limited.
        // The range of each independent variable is provided by the user and are based on the limits of the
        // empirical model. These limits are tested in this subroutine each time step and returned for use by the calling
        // routine.

        // Using/Aliasing
        using General::CreateSysTimeIntervalString;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        auto &thisError = state.dataHeatRecovery->error3;

        Real64 SysTimeElapsed = state.dataHVACGlobal->SysTimeElapsed;
        Real64 TimeStepSys = state.dataHVACGlobal->TimeStepSys;
        // current end time is compared with last to see if time step changed

        //   calculate end time of current time step
        thisError.CurrentEndTime = state.dataGlobal->CurrentTime + SysTimeElapsed;

        //   Print warning messages only when valid and only for the first occurrence. Let summary provide statistics.
        //   Wait for next time step to print warnings. If simulation iterates, print out
        //   the warning for the last iteration only. Must wait for next time step to accomplish this.
        //   If a warning occurs and the simulation down shifts, the warning is not valid.
        if (thisError.CurrentEndTime > thisError.CurrentEndTimeLast && TimeStepSys >= thisError.TimeStepSysLast) {

            // print error when regeneration outlet temperature is greater than regen inlet temperature
            if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempFailedError.print) {
                ++state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempFailedError.count;
                if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempFailedError.count < 2) {
                    ShowWarningError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempFailedError.buffer1);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempFailedError.buffer2);
                    ShowContinueError(state,
                                      "...Regeneration outlet air temperature should always be less than or equal to regen inlet air temperature. "
                                      "Verify correct model coefficients.");
                } else {
                    ShowRecurringWarningErrorAtEnd(
                        state,
                        format("{} \"{}\" - Regeneration outlet air temperature above regen inlet air temperature error continues...",
                               state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                               state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name),
                        state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempFailedError.index,
                        state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempFailedError.last,
                        state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempFailedError.last);
                }
            }

            // print error for variables of regeneration outlet temperature
            if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempError.print) {
                ++state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempError.count;
                if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempError.count < 2) {
                    ShowWarningError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempError.buffer1);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempError.buffer2);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempError.buffer3);
                    ShowContinueError(state,
                                      "...Regeneration outlet air temperature should always be less than or equal to regen inlet air temperature. "
                                      "Verify correct model coefficients.");
                } else {
                    ShowRecurringWarningErrorAtEnd(
                        state,
                        format("{} \"{}\" - Regeneration outlet air temperature should be less than regen inlet air temperature error continues...",
                               state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                               state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name),
                        state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempError.index,
                        state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempError.last,
                        state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempError.last);
                }
            }
        } // IF(CurrentEndTime .GT. CurrentEndTimeLast .AND. TimeStepSys .GE. TimeStepSysLast)THEN

        //   save last system time step and last end time of current time step (used to determine if warning is valid)
        thisError.TimeStepSysLast = TimeStepSys;
        thisError.CurrentEndTimeLast = thisError.CurrentEndTime;

        // checking model regeneration outlet temperature to always be less than or equal to regeneration inlet temperature
        if (RegenOutTemp > RegenInTemp) {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempFailedError.last = RegenOutTemp;
            thisError.OutputChar = format("{:.2R}", RegenOutTemp);
            thisError.OutputCharHi = format("{:.2R}", RegenInTemp);
            //      IF(RegenOutTemp .GT. RegenInTemp)THEN
            //        RegenOutTemp = RegenInTemp
            //      END IF
            if (!state.dataGlobal->WarmupFlag && !FirstHVACIteration) {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempFailedError.print = true;
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempFailedError.buffer1 =
                    format("{} \"{}\" - Regeneration outlet air temperature is greater than inlet temperature at {}.",
                           state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                           state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name,
                           thisError.OutputChar);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempFailedError.buffer2 =
                    format("...Regen inlet air temperature = {}. Occurrence info = {}, {}, {}",
                           thisError.OutputCharHi,
                           state.dataEnvrn->EnvironmentName,
                           state.dataEnvrn->CurMnDy,
                           CreateSysTimeIntervalString(state));
                thisError.CharValue = format("{:.6R}", RegenOutTemp);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempFailedError.buffer3 = format(
                    "...Regen outlet air temperature equation: regeneration outlet air temperature allowed from the model = {}", thisError.CharValue);
            } else {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempError.print = false;
            }
        } else {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempError.print = false;
        }

        //   check boundaries of regen outlet temperature and post warnings to individual buffers to print at end of time step
        // checking model bounds for regeneration outlet temperature
        if (RegenOutTemp < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).MinRegenAirOutTemp ||
            RegenOutTemp > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).MaxRegenAirOutTemp) {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempError.last = RegenOutTemp;
            thisError.OutputChar = format("{:.2R}", RegenOutTemp);
            thisError.OutputCharLo = format("{:.2R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).MinRegenAirOutTemp);
            thisError.OutputCharHi = format("{:.2R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).MaxRegenAirOutTemp);
            if (RegenOutTemp < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).MinRegenAirOutTemp) {
                RegenOutTemp = state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).MinRegenAirOutTemp;
            }
            if (RegenOutTemp > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).MaxRegenAirOutTemp) {
                RegenOutTemp = state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).MaxRegenAirOutTemp;
            }
            if (!state.dataGlobal->WarmupFlag && !FirstHVACIteration) {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempError.print = true;
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempError.buffer1 =
                    format("{} \"{}\" - Regeneration outlet air temperature equation is outside model boundaries at {}.",
                           state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                           state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name,
                           thisError.OutputChar);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempError.buffer2 =
                    format("...Valid range = {} to {}. Occurrence info = {}, {}, {}",
                           thisError.OutputCharLo,
                           thisError.OutputCharHi,
                           state.dataEnvrn->EnvironmentName,
                           state.dataEnvrn->CurMnDy,
                           CreateSysTimeIntervalString(state));
                thisError.CharValue = format("{:.6R}", RegenOutTemp);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempError.buffer3 = format(
                    "...Regen outlet air temperature equation: regeneration outlet air temperature allowed from the model = {}", thisError.CharValue);
            } else {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempError.print = false;
            }
        } else {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutTempError.print = false;
        }
    }

    void HeatExchCond::CheckModelBoundOutput_HumRat(EnergyPlusData &state,
                                                    Real64 const RegenInHumRat,   // current regen inlet hum rat passed to eqn
                                                    Real64 &RegenOutHumRat,       // current regen outlet hum rat from eqn
                                                    bool const FirstHVACIteration // First HVAC iteration flag
    ) const
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Mangesh Basarkar, FSEC
        //       DATE WRITTEN   January 2007
        //       MODIFIED       June 2007, R. Raustad, changed requirement that regen outlet temp be less than inlet temp
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // To verify that the empirical model's independent variables are within the limits used during the
        // development of the empirical model.

        // METHODOLOGY EMPLOYED:
        // The empirical models used for simulating a desiccant enhanced cooling coil are based on a limited data set.
        // Extrapolation of empirical models can cause instability and the independent variables may need to be limited.
        // The range of each independent variable is provided by the user and are based on the limits of the
        // empirical model. These limits are tested in this subroutine each time step and returned for use by the calling
        // routine.
        // REFERENCES:
        // na

        // Using/Aliasing
        using General::CreateSysTimeIntervalString;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        auto &thisError = state.dataHeatRecovery->error4; // (THIS_AUTO_OK)

        Real64 SysTimeElapsed = state.dataHVACGlobal->SysTimeElapsed;
        Real64 TimeStepSys = state.dataHVACGlobal->TimeStepSys;

        // current end time is compared with last to see if time step changed

        //   calculate end time of current time step
        thisError.CurrentEndTime = state.dataGlobal->CurrentTime + SysTimeElapsed;

        //   Print warning messages only when valid and only for the first occurrence. Let summary provide statistics.
        //   Wait for next time step to print warnings. If simulation iterates, print out
        //   the warning for the last iteration only. Must wait for next time step to accomplish this.
        //   If a warning occurs and the simulation down shifts, the warning is not valid.
        if (thisError.CurrentEndTime > thisError.CurrentEndTimeLast && TimeStepSys >= thisError.TimeStepSysLast) {

            // print error when regeneration outlet humidity ratio is less than regeneration inlet humidity ratio
            if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatFailedErr.print) {
                ++state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatFailedErr.count;
                if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatFailedErr.count < 2) {
                    ShowWarningError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatFailedErr.buffer1);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatFailedErr.buffer2);
                    ShowContinueError(state,
                                      "...Regeneration outlet air humidity ratio should always be greater than or equal to regen inlet air humidity "
                                      "ratio. Verify correct model coefficients.");
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   format("{} \"{}\" - Regeneration outlet air humidity ratio should be greater than regen inlet air "
                                                          "humidity ratio error continues...",
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name),
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatFailedErr.index,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatFailedErr.last,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatFailedErr.last);
                }
            }

            // print error for regeneration outlet humidity ratio
            if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatError.print) {
                ++state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatError.count;
                if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatError.count < 2) {
                    ShowWarningError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatError.buffer1);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatError.buffer2);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatError.buffer3);
                    ShowContinueError(
                        state,
                        "...Regeneration outlet air humidity ratio outside model boundaries may adversely affect desiccant model performance.");
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   format("{} \"{}\" - Regeneration outlet air humidity ratio is out of range error continues...",
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name),
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatError.index,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatError.last,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatError.last);
                }
            }
        } // IF(CurrentEndTime .GT. CurrentEndTimeLast .AND. TimeStepSys .GE. TimeStepSysLast)THEN

        //   save last system time step and last end time of current time step (used to determine if warning is valid)
        thisError.TimeStepSysLast = TimeStepSys;
        thisError.CurrentEndTimeLast = thisError.CurrentEndTime;

        // checking for regeneration outlet humidity ratio less than or equal to regeneration inlet humidity ratio
        if (RegenOutHumRat < RegenInHumRat) {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatFailedErr.last = RegenOutHumRat;
            thisError.OutputChar = format("{:.6R}", RegenOutHumRat);
            thisError.OutputCharHi = format("{:.6R}", RegenInHumRat);
            //      IF(RegenOutHumRat .LT. RegenInHumRat)THEN
            //        RegenOutHumRat = RegenInHumRat
            //      END IF
            if (!state.dataGlobal->WarmupFlag && !FirstHVACIteration) {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatFailedErr.print = true;
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatFailedErr.buffer1 =
                    format("{} \"{}\" - Regeneration outlet air humidity ratio is less than the inlet air humidity ratio at {}.",
                           state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                           state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name,
                           thisError.OutputChar);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatFailedErr.buffer2 =
                    format("...Regen inlet air humidity ratio = {}. Occurrence info = {}, {}, {}",
                           thisError.OutputCharHi,
                           state.dataEnvrn->EnvironmentName,
                           state.dataEnvrn->CurMnDy,
                           CreateSysTimeIntervalString(state));
                thisError.CharValue = format("{:.6R}", RegenOutHumRat);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatFailedErr.buffer3 =
                    format("...Regen outlet air humidity ratio equation: regeneration outlet air humidity ratio allowed from the model = {}",
                           thisError.CharValue);
            } else {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatFailedErr.print = false;
            }
        } else {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatFailedErr.print = false;
        }

        //   check boundaries of regen outlet humrat and post warnings to individual buffers to print at end of time step
        // checking model bounds for regeneration outlet humidity ratio
        if (RegenOutHumRat < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).MinRegenAirOutHumRat ||
            RegenOutHumRat > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).MaxRegenAirOutHumRat) {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatError.last = RegenOutHumRat;
            thisError.OutputChar = format("{:.6R}", RegenOutHumRat);
            thisError.OutputCharLo = format("{:.6R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).MinRegenAirOutHumRat);
            thisError.OutputCharHi = format("{:.6R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).MaxRegenAirOutHumRat);
            if (RegenOutHumRat < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).MinRegenAirOutHumRat) {
                RegenOutHumRat = state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).MinRegenAirOutHumRat;
            }
            if (RegenOutHumRat > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).MaxRegenAirOutHumRat) {
                RegenOutHumRat = state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).MaxRegenAirOutHumRat;
            }
            if (!state.dataGlobal->WarmupFlag && !FirstHVACIteration) {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatError.print = true;
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatError.buffer1 =
                    format("{} \"{}\" - Regeneration outlet air humidity ratio is outside model boundaries at {}.",
                           state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                           state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name,
                           thisError.OutputChar);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatError.buffer2 =
                    format("...Valid range = {} to {}. Occurrence info = {}, {}, {}",
                           thisError.OutputCharLo,
                           thisError.OutputCharHi,
                           state.dataEnvrn->EnvironmentName,
                           state.dataEnvrn->CurMnDy,
                           CreateSysTimeIntervalString(state));
                thisError.CharValue = format("{:.6R}", RegenOutHumRat);
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatError.buffer3 =
                    format("...Regen outlet air humidity ratio equation: regeneration outlet air humidity ratio allowed from the model = {}",
                           thisError.CharValue);
            } else {
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatError.print = false;
            }
        } else {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenOutHumRatError.print = false;
        }
    }

    void HeatExchCond::CheckModelBoundsRH_TempEq(EnergyPlusData &state,
                                                 Real64 const T_RegenInTemp,   // current regen inlet temperature passed to eqn
                                                 Real64 const T_RegenInHumRat, // current regen inlet hum rat passed to eqn
                                                 Real64 const T_ProcInTemp,    // current process inlet temperature passed to eqn
                                                 Real64 const T_ProcInHumRat,  // current regen outlet hum rat from eqn
                                                 bool const FirstHVACIteration // first HVAC iteration flag
    ) const
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   January 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // To verify that the empirical model's independent variables result in a relative humidity that is within the range
        // of relative humidities used when creating the empirical model. Both the regeneration and process inlet are tested.

        // METHODOLOGY EMPLOYED:
        // The empirical models used for simulating a desiccant enhanced cooling coil are based on a limited data set.
        // Extrapolation of empirical models can cause instability and the independent variables may need to be limited.
        // In addition, the range of relative humidities in the original data set may influence the output of the
        // empirical model. This subroutine tests the relative humidities passed to the empirical model and warns the
        // user if these relative humidities are out of bounds based on the limits set by the user.
        // REFERENCES:
        // na

        // Using/Aliasing
        using General::CreateSysTimeIntervalString;

        using Psychrometrics::PsyRhFnTdbWPb;

        Real64 SysTimeElapsed = state.dataHVACGlobal->SysTimeElapsed;
        Real64 TimeStepSys = state.dataHVACGlobal->TimeStepSys;
        auto &RegenInletRH = state.dataHeatRecovery->RegenInletRH2;
        auto &ProcInletRH = state.dataHeatRecovery->ProcInletRH2;
        auto &thisError = state.dataHeatRecovery->error6;
        // current end time is compared with last to see if time step changed

        if (state.dataGlobal->WarmupFlag || FirstHVACIteration) {
            return;
        }

        //   calculate end time of current time step
        thisError.CurrentEndTime = state.dataGlobal->CurrentTime + SysTimeElapsed;

        //   Print warning messages only when valid and only for the first occurrence. Let summary provide statistics.
        //   Wait for next time step to print warnings. If simulation iterates, print out
        //   the warning for the last iteration only. Must wait for next time step to accomplish this.
        //   If a warning occurs and the simulation down shifts, the warning is not valid.
        if (thisError.CurrentEndTime > thisError.CurrentEndTimeLast && TimeStepSys >= thisError.TimeStepSysLast) {

            // print error when regeneration inlet relative humidity is outside model boundaries
            if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumTempErr.print) {
                ++state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumTempErr.count;
                if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumTempErr.count < 2) {
                    ShowWarningError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumTempErr.buffer1);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumTempErr.buffer2);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumTempErr.buffer3);
                    ShowContinueError(state,
                                      "...Using regeneration inlet air relative humidities that are outside the regeneration outlet temperature "
                                      "equation model boundaries may adversely affect desiccant model performance. Verify correct model "
                                      "coefficients.");
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   format("{} \"{}\" - Regeneration inlet air relative humidity related to regen outlet air "
                                                          "temperature equation is outside model boundaries error continues...",
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name),
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumTempErr.index,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumTempErr.last,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumTempErr.last);
                }
            }

            // print error when process inlet relative humidity is outside model boundaries
            if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumTempErr.print) {
                ++state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumTempErr.count;
                if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumTempErr.count < 2) {
                    ShowWarningError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumTempErr.buffer1);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumTempErr.buffer2);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumTempErr.buffer3);
                    ShowContinueError(state,
                                      "...Using process inlet air relative humidities that are outside the regeneration outlet temperature equation "
                                      "model boundaries may adversely affect desiccant model performance. Verify correct model coefficients.");
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   format("{} \"{}\" - Process inlet air relative humidity related to regen outlet air temperature "
                                                          "equation is outside model boundaries error continues...",
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name),
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumTempErr.index,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumTempErr.last,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumTempErr.last);
                }
            }

        } // IF(CurrentEndTime .GT. CurrentEndTimeLast .AND. TimeStepSys .GE. TimeStepSysLast)THEN

        //     save last system time step and last end time of current time step (used to determine if warning is valid)
        thisError.TimeStepSysLast = TimeStepSys;
        thisError.CurrentEndTimeLast = thisError.CurrentEndTime;

        //     Check that condition is not above saturation curve prior to next calc (PsyRhFnTdbWPb) to avoid psyc routine errors
        //                           *
        //                          *
        //                  x------*---------- T_HumRat
        //                  |    *
        //                  |  *
        //                  *----------------- PsyWFnTdpPb(Tdp,Pb)
        //               *  |
        //                  |
        //                T_Temp
        if (T_RegenInHumRat > Psychrometrics::PsyWFnTdpPb(state, T_RegenInTemp, state.dataEnvrn->OutBaroPress) ||
            T_ProcInHumRat > Psychrometrics::PsyWFnTdpPb(state, T_ProcInTemp, state.dataEnvrn->OutBaroPress)) {
            //       reset RH print flags just in case
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumTempErr.print = false;
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumTempErr.print = false;
            return;
        }

        //     If regen and process inlet temperatures are the same the coil is off, do not print out of bounds warning for this case
        if (std::abs(T_RegenInTemp - T_ProcInTemp) < SMALL) {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumTempErr.print = false;
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumTempErr.print = false;
            return;
        }

        RegenInletRH = PsyRhFnTdbWPb(state, T_RegenInTemp, T_RegenInHumRat, state.dataEnvrn->OutBaroPress);
        ProcInletRH = min(1.0, PsyRhFnTdbWPb(state, T_ProcInTemp, T_ProcInHumRat, state.dataEnvrn->OutBaroPress));

        // checking if regeneration inlet relative humidity is within model boundaries
        if (RegenInletRH < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MinRegenAirInRelHum ||
            RegenInletRH > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MaxRegenAirInRelHum) {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumTempErr.last = RegenInletRH * 100.0;
            thisError.OutputChar = format("{:.1R}", RegenInletRH * 100.0);
            thisError.OutputCharLo = format("{:.1R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MinRegenAirInRelHum * 100.0);
            thisError.OutputCharHi = format("{:.1R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MaxRegenAirInRelHum * 100.0);
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumTempErr.print = true;

            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumTempErr.buffer1 =
                format("{} \"{}\" - Regeneration inlet air relative humidity related to regen outlet air temperature equation is outside model "
                       "boundaries at {}.",
                       state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                       state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name,
                       thisError.OutputChar);
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumTempErr.buffer2 =
                format("...Model limit on regeneration inlet air relative humidity is {} to {}.", thisError.OutputCharLo, thisError.OutputCharHi);
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumTempErr.buffer3 = format(
                "...Occurrence info = {}, {}, {}", state.dataEnvrn->EnvironmentName, state.dataEnvrn->CurMnDy, CreateSysTimeIntervalString(state));
        } else {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumTempErr.print = false;
        }

        // checking if process inlet relative humidity is within model boundaries
        if (ProcInletRH < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MinProcAirInRelHum ||
            ProcInletRH > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MaxProcAirInRelHum) {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumTempErr.last = ProcInletRH * 100.0;
            thisError.OutputChar = format("{:.1R}", ProcInletRH * 100.0);
            thisError.OutputCharLo = format("{:.1R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MinProcAirInRelHum * 100.0);
            thisError.OutputCharHi = format("{:.1R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).T_MaxProcAirInRelHum * 100.0);
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumTempErr.print = true;

            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumTempErr.buffer1 = format(
                "{} \"{}\" - Process inlet air relative humidity related to regen outlet air temperature equation is outside model boundaries at {}.",
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name,
                thisError.OutputChar);
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumTempErr.buffer2 =
                format("...Model limit on process inlet air relative humidity is {} to {}.", thisError.OutputCharLo, thisError.OutputCharHi);
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumTempErr.buffer3 = format(
                "...Occurrence info = {}, {}, {}", state.dataEnvrn->EnvironmentName, state.dataEnvrn->CurMnDy, CreateSysTimeIntervalString(state));
        } else {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumTempErr.print = false;
        }
    }

    void HeatExchCond::CheckModelBoundsRH_HumRatEq(EnergyPlusData &state,
                                                   Real64 const H_RegenInTemp,   // current regen inlet temperature passed to eqn
                                                   Real64 const H_RegenInHumRat, // current regen inlet hum rat passed to eqn
                                                   Real64 const H_ProcInTemp,    // current process inlet temperature passed to eqn
                                                   Real64 const H_ProcInHumRat,  // current process inlet hum rat passed to eqn
                                                   bool const FirstHVACIteration // first HVAC iteration flag
    ) const
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   January 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // To verify that the empirical model's independent variables result in a relative humidity that is within the range
        // of relative humidities used when creating the empirical model. Both the regeneration and process inlet are tested.

        // METHODOLOGY EMPLOYED:
        // The empirical models used for simulating a desiccant enhanced cooling coil are based on a limited data set.
        // Extrapolation of empirical models can cause instability and the independent variables may need to be limited.
        // In addition, the range of relative humidities in the original data set may influence the output of the
        // empirical model. This subroutine tests the relative humidities passed to the empirical model and warns the
        // user if these relative humidities are out of bounds based on the limits set by the user.

        // Using/Aliasing
        using General::CreateSysTimeIntervalString;

        using Psychrometrics::PsyRhFnTdbWPb;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 SysTimeElapsed = state.dataHVACGlobal->SysTimeElapsed;
        Real64 TimeStepSys = state.dataHVACGlobal->TimeStepSys;
        auto &RegenInletRH = state.dataHeatRecovery->RegenInletRH;
        auto &ProcInletRH = state.dataHeatRecovery->ProcInletRH;
        auto &thisError = state.dataHeatRecovery->error5;
        // current end time is compared with last to see if time step changed

        if (state.dataGlobal->WarmupFlag || FirstHVACIteration) {
            return;
        }

        //   calculate end time of current time step
        thisError.CurrentEndTime = state.dataGlobal->CurrentTime + SysTimeElapsed;

        //   Print warning messages only when valid and only for the first occurrence. Let summary provide statistics.
        //   Wait for next time step to print warnings. If simulation iterates, print out
        //   the warning for the last iteration only. Must wait for next time step to accomplish this.
        //   If a warning occurs and the simulation down shifts, the warning is not valid.
        if (thisError.CurrentEndTime > thisError.CurrentEndTimeLast && TimeStepSys >= thisError.TimeStepSysLast) {

            // print error when regeneration inlet relative humidity is outside model boundaries
            if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumHumRatErr.print) {
                ++state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumHumRatErr.count;
                if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumHumRatErr.count < 2) {
                    ShowWarningError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumHumRatErr.buffer1);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumHumRatErr.buffer2);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumHumRatErr.buffer3);
                    ShowContinueError(state,
                                      "...Using regeneration inlet air relative humidities that are outside the regeneration outlet humidity ratio "
                                      "equation model boundaries may adversely affect desiccant model performance. Verify correct model "
                                      "coefficients.");
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   format("{} \"{}\" - Regeneration inlet air relative humidity related to regen outlet air humidity "
                                                          "ratio equation is outside model boundaries error continues...",
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name),
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumHumRatErr.index,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumHumRatErr.last,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumHumRatErr.last);
                }
            }

            // print error when process inlet relative humidity is outside model boundaries
            if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumHumRatErr.print) {
                ++state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumHumRatErr.count;
                if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumHumRatErr.count < 2) {
                    ShowWarningError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumHumRatErr.buffer1);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumHumRatErr.buffer2);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumHumRatErr.buffer3);
                    ShowContinueError(state,
                                      "...Using process inlet air relative humidities that are outside the regeneration outlet humidity ratio "
                                      "equation model boundaries may adversely affect desiccant model performance. Verify correct model "
                                      "coefficients.");
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   format("{} \"{}\" - Process inlet air relative humidity related to regen outlet air humidity "
                                                          "ratio equation is outside model boundaries error continues...",
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                                                          state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name),
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumHumRatErr.index,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumHumRatErr.last,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumHumRatErr.last);
                }
            }

        } // IF(CurrentEndTime .GT. CurrentEndTimeLast .AND. TimeStepSys .GE. TimeStepSysLast)THEN

        //     save last system time step and last end time of current time step (used to determine if warning is valid)
        thisError.TimeStepSysLast = TimeStepSys;
        thisError.CurrentEndTimeLast = thisError.CurrentEndTime;

        //     Check that condition is not above saturation curve prior to next calc (PsyRhFnTdbWPb) to avoid psyc routine errors
        //                           *
        //                          *
        //                  x------*---------- H_HumRat
        //                  |    *
        //                  |  *
        //                  *----------------- PsyWFnTdpPb(Tdp,Pb)
        //               *  |
        //                  |
        //                H_Temp
        if (H_RegenInHumRat > Psychrometrics::PsyWFnTdpPb(state, H_RegenInTemp, state.dataEnvrn->OutBaroPress) ||
            H_ProcInHumRat > Psychrometrics::PsyWFnTdpPb(state, H_ProcInTemp, state.dataEnvrn->OutBaroPress)) {
            //       reset RH print flags just in case
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumHumRatErr.print = false;
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumHumRatErr.print = false;
            return;
        }

        //     If regen and process inlet temperatures are the same the coil is off, do not print out of bounds warning for this case
        if (std::abs(H_RegenInTemp - H_ProcInTemp) < SMALL) {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumHumRatErr.print = false;
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumHumRatErr.print = false;
            return;
        }

        RegenInletRH = PsyRhFnTdbWPb(state, H_RegenInTemp, H_RegenInHumRat, state.dataEnvrn->OutBaroPress);
        ProcInletRH = min(1.0, PsyRhFnTdbWPb(state, H_ProcInTemp, H_ProcInHumRat, state.dataEnvrn->OutBaroPress));

        // checking if regeneration inlet relative humidity is within model boundaries
        if (RegenInletRH < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MinRegenAirInRelHum ||
            RegenInletRH > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MaxRegenAirInRelHum) {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumHumRatErr.last = RegenInletRH * 100.0;
            thisError.OutputChar = format("{:.1R}", RegenInletRH * 100.0);
            thisError.OutputCharLo = format("{:.1R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MinRegenAirInRelHum * 100.0);
            thisError.OutputCharHi = format("{:.1R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MaxRegenAirInRelHum * 100.0);
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumHumRatErr.print = true;

            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumHumRatErr.buffer1 =
                format("{} \"{}\" - Regeneration inlet air relative humidity related to regen outlet air humidity ratio equation is outside model "
                       "boundaries at {}.",
                       state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                       state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name,
                       thisError.OutputChar);
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumHumRatErr.buffer2 =
                format("...Model limit on regeneration inlet air relative humidity is {} to {}.", thisError.OutputCharLo, thisError.OutputCharHi);
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumHumRatErr.buffer3 = format(
                "...Occurrence info = {}, {}, {}", state.dataEnvrn->EnvironmentName, state.dataEnvrn->CurMnDy, CreateSysTimeIntervalString(state));
        } else {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).regenInRelHumHumRatErr.print = false;
        }

        // checking if process inlet relative humidity is within model boundaries
        if (ProcInletRH < state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MinProcAirInRelHum ||
            ProcInletRH > state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MaxProcAirInRelHum) {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumHumRatErr.last = ProcInletRH * 100.0;
            thisError.OutputChar = format("{:.1R}", ProcInletRH * 100.0);
            thisError.OutputCharLo = format("{:.1R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MinProcAirInRelHum * 100.0);
            thisError.OutputCharHi = format("{:.1R}", state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).H_MaxProcAirInRelHum * 100.0);
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumHumRatErr.print = true;

            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumHumRatErr.buffer1 =
                format("{} \"{}\" - Process inlet air relative humidity related to regen outlet air humidity ratio equation is outside model "
                       "boundaries at {}.",
                       state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).PerfType,
                       state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).Name,
                       thisError.OutputChar);
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumHumRatErr.buffer2 =
                format("...Model limit on process inlet air relative humidity is {} to {}.", thisError.OutputCharLo, thisError.OutputCharHi);
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumHumRatErr.buffer3 = format(
                "...Occurrence info = {}, {}, {}", state.dataEnvrn->EnvironmentName, state.dataEnvrn->CurMnDy, CreateSysTimeIntervalString(state));
        } else {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).procInRelHumHumRatErr.print = false;
        }
    }

    void HeatExchCond::CheckForBalancedFlow(EnergyPlusData &state,
                                            Real64 const ProcessInMassFlow, // current process inlet air mass flow rate (m3/s)
                                            Real64 const RegenInMassFlow,   // current regeneration inlet air mass flow rate (m3/s)
                                            bool const FirstHVACIteration   // first HVAC iteration flag
    ) const
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   June 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // To verify that the balanced flow desiccant heat exchanger has the same regeneration and process air flow rates.

        // METHODOLOGY EMPLOYED:
        // Check that the regeneration and process air mass flow rates are within 2%.
        // REFERENCES:
        // na

        // Using/Aliasing
        using General::CreateSysTimeIntervalString;

        auto &thisError = state.dataHeatRecovery->error7;

        Real64 SysTimeElapsed = state.dataHVACGlobal->SysTimeElapsed;
        Real64 TimeStepSys = state.dataHVACGlobal->TimeStepSys;

        // current end time is compared with last to see if time step changed
        Real64 ABSImbalancedFlow; // absolute value of process and regeneration air flow imbalance fraction

        if (state.dataGlobal->WarmupFlag || FirstHVACIteration) {
            return;
        }

        //   calculate end time of current time step
        thisError.CurrentEndTime = state.dataGlobal->CurrentTime + SysTimeElapsed;

        //   Print warning messages only when valid and only for the first occurrence. Let summary provide statistics.
        //   Wait for next time step to print warnings. If simulation iterates, print out
        //   the warning for the last iteration only. Must wait for next time step to accomplish this.
        //   If a warning occurs and the simulation down shifts, the warning is not valid.
        if (thisError.CurrentEndTime > thisError.CurrentEndTimeLast && TimeStepSys >= thisError.TimeStepSysLast) {

            // print error when regeneration inlet relative humidity is outside model boundaries
            if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).imbalancedFlowErr.print) {
                ++state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).imbalancedFlowErr.count;
                if (state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).imbalancedFlowErr.count < 2) {
                    ShowWarningError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).imbalancedFlowErr.buffer1);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).imbalancedFlowErr.buffer2);
                    ShowContinueError(state, state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).imbalancedFlowErr.buffer3);
                    //           CALL ShowContinueError(state, '...Using regeneration inlet air relative humidities that are outside the regeneration
                    //           '&
                    //                 //'outlet humidity ratio equation model boundaries may adversely affect desiccant model performance. '&
                    //                 //'Verify correct model coefficients.')
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   format("{} \"{}\" - unbalanced air flow rate is limited to 2% error continues with the imbalanced "
                                                          "fraction statistics reported...",
                                                          HVAC::hxTypeNames[(int)this->type],
                                                          this->Name),
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).imbalancedFlowErr.index,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).imbalancedFlowErr.last,
                                                   state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).imbalancedFlowErr.last);
                }
            }

        } // IF(CurrentEndTime .GT. CurrentEndTimeLast .AND. TimeStepSys .GE. TimeStepSysLast)THEN

        //     save last system time step and last end time of current time step (used to determine if warning is valid)
        thisError.TimeStepSysLast = TimeStepSys;
        thisError.CurrentEndTimeLast = thisError.CurrentEndTime;

        // checking if regeneration inlet relative humidity is within model boundaries
        ABSImbalancedFlow = std::abs(RegenInMassFlow - ProcessInMassFlow) / RegenInMassFlow;
        if (ABSImbalancedFlow > 0.02) {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).imbalancedFlowErr.last = ABSImbalancedFlow;
            thisError.OutputCharLo = format("{:.6R}", RegenInMassFlow);
            thisError.OutputCharHi = format("{:.6R}", ProcessInMassFlow);
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).imbalancedFlowErr.print = true;

            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).imbalancedFlowErr.buffer1 =
                format("{} \"{}\" - unbalanced air flow rate is limited to 2%.", HVAC::hxTypeNames[(int)this->type], this->Name);
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).imbalancedFlowErr.buffer2 = format(
                "...Regeneration air mass flow rate is {} and process air mass flow rate is {}.", thisError.OutputCharLo, thisError.OutputCharHi);
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).imbalancedFlowErr.buffer3 = format(
                "...Occurrence info = {}, {}, {}", state.dataEnvrn->EnvironmentName, state.dataEnvrn->CurMnDy, CreateSysTimeIntervalString(state));
        } else {
            state.dataHeatRecovery->BalDesDehumPerfData(this->PerfDataIndex).imbalancedFlowErr.print = false;
        }
    }

    int GetSupplyInletNode(EnergyPlusData &state,
                           std::string const &HXName, // must match HX names for the state.dataHeatRecovery->ExchCond type
                           bool &ErrorsFound          // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   February 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given HX and returns the supply air inlet node number.
        // If incorrect HX name is given, ErrorsFound is returned as true and node number as zero.

        // Obtains and Allocates heat exchanger related parameters from input file
        if (state.dataHeatRecovery->GetInputFlag) { // First time subroutine has been entered
            GetHeatRecoveryInput(state);
            state.dataHeatRecovery->GetInputFlag = false;
        }

        int const WhichHX = Util::FindItemInList(HXName, state.dataHeatRecovery->ExchCond);
        if (WhichHX != 0) {
            return state.dataHeatRecovery->ExchCond(WhichHX).SupInletNode;
        } else {
            ShowSevereError(state, format("GetSupplyInletNode: Could not find heat exchanger = \"{}\"", HXName));
            ErrorsFound = true;
            return 0;
        }
    }

    int GetSupplyOutletNode(EnergyPlusData &state,
                            std::string const &HXName, // must match HX names for the state.dataHeatRecovery->ExchCond type
                            bool &ErrorsFound          // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   February 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given HX and returns the supply air outlet node number.
        // If incorrect HX name is given, ErrorsFound is returned as true and node number as zero.

        // Obtains and Allocates heat exchanger related parameters from input file
        if (state.dataHeatRecovery->GetInputFlag) { // First time subroutine has been entered
            GetHeatRecoveryInput(state);
            state.dataHeatRecovery->GetInputFlag = false;
        }

        int const WhichHX = Util::FindItemInList(HXName, state.dataHeatRecovery->ExchCond);
        if (WhichHX != 0) {
            return state.dataHeatRecovery->ExchCond(WhichHX).SupOutletNode;
        } else {
            ShowSevereError(state, format("GetSupplyOutletNode: Could not find heat exchanger = \"{}\"", HXName));
            ErrorsFound = true;
            return 0;
        }
    }

    int GetSecondaryInletNode(EnergyPlusData &state,
                              std::string const &HXName, // must match HX names for the state.dataHeatRecovery->ExchCond type
                              bool &ErrorsFound          // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   February 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given HX and returns the secondary air inlet node number.
        // If incorrect HX name is given, ErrorsFound is returned as true and node number as zero.

        // Obtains and Allocates heat exchanger related parameters from input file
        if (state.dataHeatRecovery->GetInputFlag) { // First time subroutine has been entered
            GetHeatRecoveryInput(state);
            state.dataHeatRecovery->GetInputFlag = false;
        }

        int const WhichHX = Util::FindItemInList(HXName, state.dataHeatRecovery->ExchCond);
        if (WhichHX != 0) {
            return state.dataHeatRecovery->ExchCond(WhichHX).SecInletNode;
        } else {
            ShowSevereError(state, format("GetSecondaryInletNode: Could not find heat exchanger = \"{}\"", HXName));
            ErrorsFound = true;
            return 0;
        }
    }

    int GetSecondaryOutletNode(EnergyPlusData &state,
                               std::string const &HXName, // must match HX names for the state.dataHeatRecovery->ExchCond type
                               bool &ErrorsFound          // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   February 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given HX assisted cooling coil and returns the secondary air outlet node number.
        // If incorrect HX name is given, ErrorsFound is returned as true and node number as zero.

        // Obtains and Allocates heat exchanger related parameters from input file
        if (state.dataHeatRecovery->GetInputFlag) { // First time subroutine has been entered
            GetHeatRecoveryInput(state);
            state.dataHeatRecovery->GetInputFlag = false;
        }

        int const WhichHX = Util::FindItemInList(HXName, state.dataHeatRecovery->ExchCond);
        if (WhichHX != 0) {
            return state.dataHeatRecovery->ExchCond(WhichHX).SecOutletNode;
        } else {
            ShowSevereError(state, format("GetSecondaryOutletNode: Could not find heat exchanger = \"{}\"", HXName));
            ErrorsFound = true;
            return 0;
        }
    }

    Real64 GetSupplyAirFlowRate(EnergyPlusData &state,
                                std::string const &HXName, // must match HX names for the state.dataHeatRecovery->ExchCond type
                                bool &ErrorsFound          // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   October 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given Generic HX and the volumetric air flow rate.
        // If incorrect HX name is given, ErrorsFound is returned as true and air flow rate as zero.

        // Obtains and Allocates heat exchanger related parameters from input file
        if (state.dataHeatRecovery->GetInputFlag) { // First time subroutine has been entered
            GetHeatRecoveryInput(state);
            state.dataHeatRecovery->GetInputFlag = false;
        }

        int const WhichHX = Util::FindItemInList(HXName, state.dataHeatRecovery->ExchCond);
        if (WhichHX != 0) {
            return state.dataHeatRecovery->ExchCond(WhichHX).NomSupAirVolFlow;
        } else {
            ShowSevereError(state, format("GetSupplyAirFlowRate: Could not find heat exchanger = \"{}\"", HXName));
            ShowContinueError(state, "... Supply Air Flow Rate returned as 0.");
            ErrorsFound = true;
            return 0.0;
        }
    }

    HVAC::HXType GetHeatExchangerObjectTypeNum(EnergyPlusData &state,
                                               std::string const &HXName, // must match HX names for the state.dataHeatRecovery->ExchCond type
                                               int &WhichHX,
                                               bool &ErrorsFound // set to true if problem
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   October 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given Generic HX and the volumetric air flow rate.
        // If incorrect HX name is given, ErrorsFound is returned as true and air flow rate as zero.

        // Obtains and Allocates heat exchanger related parameters from input file
        if (state.dataHeatRecovery->GetInputFlag) { // First time subroutine has been entered
            GetHeatRecoveryInput(state);
            state.dataHeatRecovery->GetInputFlag = false;
        }

        WhichHX = Util::FindItemInList(HXName, state.dataHeatRecovery->ExchCond);
        if (WhichHX != 0) {
            return state.dataHeatRecovery->ExchCond(WhichHX).type;
        } else {
            ShowSevereError(state, format("GetHeatExchangerObjectTypeNum: Could not find heat exchanger = \"{}\"", HXName));
            ErrorsFound = true;
            return HVAC::HXType::Invalid;
        }
    }

} // namespace HeatRecovery

} // namespace EnergyPlus
