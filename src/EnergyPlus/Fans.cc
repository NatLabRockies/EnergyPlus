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
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/AirLoopHVACDOAS.hh>
#include <EnergyPlus/Autosizing/SystemAirFlowSizing.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataPrecisionGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/FaultsManager.hh>
#include <EnergyPlus/HeatBalanceInternalHeatGains.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::Fans {
// Module containing the fan simulation routines

// MODULE INFORMATION:
//       AUTHOR         Richard J. Liesen
//       DATE WRITTEN   April 1998
//       MODIFIED       Shirey, May 2001
//                      Griffith, May 2009, EMS changes
//                      Craig Wray 22Aug2010 Added Fan Component Model

// PURPOSE OF THIS MODULE:
// To encapsulate the data and algorithms required to
// manage the Fan System Component

constexpr std::array<std::string_view, (int)MinFlowFracMethod::Num> minFlowFracMethodNamesUC = {"FRACTION", "FIXEDFLOWRATE"};

void FanBase::simulate(EnergyPlusData &state,
                       [[maybe_unused]] bool const _FirstHVACIteration,
                       ObjexxFCL::Optional<Real64 const> _speedRatio, // SpeedRatio for Fan:SystemModel

                       // = current flow/ max design flow rate.  It is not exactly the same as
                       // the legacy speed ratio that was used with SimulateFanComponents.
                       ObjexxFCL::Optional<Real64 const> _pressureRise, // Pressure difference to use for DeltaPress, for rating DX coils at a
                       ObjexxFCL::Optional<Real64 const> _flowFraction, // when used, this directs the fan to set the flow at this flow fraction
                       // different pressure without entire duct system
                       ObjexxFCL::Optional<Real64 const> _massFlowRate1,    // Mass flow rate in operating mode 1 [kg/s]
                       ObjexxFCL::Optional<Real64 const> _runTimeFraction1, // Run time fraction in operating mode 1
                       ObjexxFCL::Optional<Real64 const> _massFlowRate2,    // Mass flow rate in operating mode 2 [kg/s]
                       ObjexxFCL::Optional<Real64 const> _runTimeFraction2, // Run time fraction in opearating mode 2
                       ObjexxFCL::Optional<Real64 const> _pressureRise2     // Pressure difference for operating mode 2
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   February 1998
    //       MODIFIED       Chandan Sharma, March 2011 - FSEC: Added logic for ZoneHVAC sys avail managers

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine manages Fan component simulation.

    // With the correct FanNum Initialize
    init(state);

    // It looks like the behavior is different for system and
    // non-system fans?  Non-system fans can simulate without being
    // sized first?

    // Calculate the Correct Fan Model with the current FanNum
    if (type != HVAC::FanType::SystemModel) {
        auto *_thisFan = dynamic_cast<FanComponent *>(this);
        assert(_thisFan != nullptr);

        switch (type) {

        case HVAC::FanType::Constant: {
            _thisFan->simulateConstant(state);
        } break;
        case HVAC::FanType::VAV: {
            _thisFan->simulateVAV(state, _pressureRise);
        } break;
        case HVAC::FanType::OnOff: {
            _thisFan->simulateOnOff(state, _speedRatio);
        } break;
        case HVAC::FanType::Exhaust: {
            _thisFan->simulateZoneExhaust(state);
        } break;
        case HVAC::FanType::ComponentModel: {
            _thisFan->simulateComponentModel(state);
        } break;
        default: {
        } break;
        } // switch (type)

    } else { // FanType::SystemModel

        if (sizingFlag) {
            return;
        }

        auto *_thisFan = dynamic_cast<FanSystem *>(this);
        assert(_thisFan != nullptr);

        if (present(_pressureRise) && present(_massFlowRate1) && present(_runTimeFraction1) && present(_massFlowRate2) &&
            present(_runTimeFraction2) && present(_pressureRise2)) {
            Real64 _flowRatio1 = _massFlowRate1 / maxAirMassFlowRate;
            Real64 _flowRatio2 = _massFlowRate2 / maxAirMassFlowRate;
            _thisFan->calcSimpleSystemFan(state, _, _pressureRise, _flowRatio1, _runTimeFraction1, _flowRatio2, _runTimeFraction2, _pressureRise2);
        } else if (!present(_pressureRise) && present(_massFlowRate1) && present(_runTimeFraction1) && present(_massFlowRate2) &&
                   present(_runTimeFraction2) && !present(_pressureRise2)) {
            Real64 _flowRatio1 = _massFlowRate1 / maxAirMassFlowRate;
            Real64 _flowRatio2 = _massFlowRate2 / maxAirMassFlowRate;
            _thisFan->calcSimpleSystemFan(state, _flowFraction, _, _flowRatio1, _runTimeFraction1, _flowRatio2, _runTimeFraction2, _);
        } else if (present(_pressureRise) && present(_flowFraction)) {
            _thisFan->calcSimpleSystemFan(state, _flowFraction, _pressureRise, _, _, _, _, _);
        } else if (present(_pressureRise) && !present(_flowFraction)) {
            _thisFan->calcSimpleSystemFan(state, _, _pressureRise, _, _, _, _, _);
        } else if (!present(_pressureRise) && present(_flowFraction)) {
            _thisFan->calcSimpleSystemFan(state, _flowFraction, _, _, _, _, _, _);
        } else {
            _thisFan->calcSimpleSystemFan(state, _, _, _, _, _, _, _);
        }
    }

    update(state);
    report(state);
}

// Get Input Section of the Module
//******************************************************************************

void GetFanInput(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   April 1998
    //       MODIFIED       Shirey, May 2001

    // PURPOSE OF THIS SUBROUTINE:
    // Obtains input data for fans and stores it in fan data structures

    static constexpr std::string_view routineName = "GetFanInput";
    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int NumAlphas;
    int NumNums;
    int IOStat;
    bool ErrorsFound(false); // If errors detected in input
    Array1D_string cAlphaFieldNames;
    Array1D_string cNumericFieldNames;
    Array1D_bool lNumericFieldBlanks;
    Array1D_bool lAlphaFieldBlanks;
    Array1D_string cAlphaArgs;
    Array1D<Real64> rNumericArgs;
    std::string cCurrentModuleObject;
    int NumParams;

    auto &ip = state.dataInputProcessing->inputProcessor;
    auto &df = state.dataFans;

    state.dataFans->GetFanInputFlag = false;

    int MaxAlphas = 0;
    int MaxNumbers = 0;
    int NumSimpFan = ip->getNumObjectsFound(state, "Fan:ConstantVolume");
    if (NumSimpFan > 0) {
        ip->getObjectDefMaxArgs(state, "Fan:ConstantVolume", NumParams, NumAlphas, NumNums);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        MaxNumbers = max(MaxNumbers, NumNums);
    }
    int NumVarVolFan = ip->getNumObjectsFound(state, "Fan:VariableVolume");
    if (NumVarVolFan > 0) {
        ip->getObjectDefMaxArgs(state, "Fan:VariableVolume", NumParams, NumAlphas, NumNums);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        MaxNumbers = max(MaxNumbers, NumNums);
    }
    int NumOnOff = ip->getNumObjectsFound(state, "Fan:OnOff");
    if (NumOnOff > 0) {
        ip->getObjectDefMaxArgs(state, "Fan:OnOff", NumParams, NumAlphas, NumNums);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        MaxNumbers = max(MaxNumbers, NumNums);
    }
    int NumZoneExhFan = ip->getNumObjectsFound(state, "Fan:ZoneExhaust");
    if (NumZoneExhFan > 0) {
        ip->getObjectDefMaxArgs(state, "Fan:ZoneExhaust", NumParams, NumAlphas, NumNums);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        MaxNumbers = max(MaxNumbers, NumNums);
    }
    state.dataFans->NumNightVentPerf = ip->getNumObjectsFound(state, "FanPerformance:NightVentilation");
    if (df->NumNightVentPerf > 0) {
        ip->getObjectDefMaxArgs(state, "FanPerformance:NightVentilation", NumParams, NumAlphas, NumNums);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        MaxNumbers = max(MaxNumbers, NumNums);
    }

    int NumCompModelFan = ip->getNumObjectsFound(state, "Fan:ComponentModel");
    if (NumCompModelFan > 0) {
        ip->getObjectDefMaxArgs(state, "Fan:ComponentModel", NumParams, NumAlphas, NumNums);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        MaxNumbers = max(MaxNumbers, NumNums);
    }

    int NumSystemModelFan = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Fan:SystemModel");
    if (NumSystemModelFan > 0) {
        ip->getObjectDefMaxArgs(state, "Fan:SystemModel", NumParams, NumAlphas, NumNums);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        MaxNumbers = max(MaxNumbers, NumNums);
    }

    cAlphaArgs.allocate(MaxAlphas);
    cAlphaFieldNames.allocate(MaxAlphas);
    lAlphaFieldBlanks.dimension(MaxAlphas, false);
    cNumericFieldNames.allocate(MaxNumbers);
    lNumericFieldBlanks.dimension(MaxNumbers, false);
    rNumericArgs.dimension(MaxNumbers, 0.0);

    for (int SimpFanNum = 1; SimpFanNum <= NumSimpFan; ++SimpFanNum) {
        cCurrentModuleObject = "Fan:ConstantVolume";
        ip->getObjectItem(state,
                          cCurrentModuleObject,
                          SimpFanNum,
                          cAlphaArgs,
                          NumAlphas,
                          rNumericArgs,
                          NumNums,
                          IOStat,
                          lNumericFieldBlanks,
                          lAlphaFieldBlanks,
                          cAlphaFieldNames,
                          cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};

        if (df->fanMap.find(cAlphaArgs(1)) != df->fanMap.end()) {
            ShowSevereDuplicateName(state, eoh);
            ErrorsFound = true;
        }

        auto *fan = new FanComponent;
        fan->Name = cAlphaArgs(1);

        df->fans.push_back(fan);
        df->fanMap.insert_or_assign(cAlphaArgs(1), df->fans.size());

        fan->type = HVAC::FanType::Constant;
        fan->sizingPrefix = cNumericFieldNames(3);

        if (lAlphaFieldBlanks(2)) {
            fan->availSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((fan->availSched = Sched::GetSchedule(state, cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(2), cAlphaArgs(2));
            ErrorsFound = true;
        }

        fan->totalEff = rNumericArgs(1);
        fan->deltaPress = rNumericArgs(2);
        fan->maxAirFlowRate = rNumericArgs(3);
        if (fan->maxAirFlowRate == 0.0) {
            ShowWarningError(
                state,
                format("{}=\"{}\" has specified 0.0 max air flow rate. It will not be used in the simulation.", cCurrentModuleObject, fan->Name));
        }
        fan->maxAirFlowRateIsAutosized = true;
        fan->motorEff = rNumericArgs(4);
        fan->motorInAirFrac = rNumericArgs(5);
        fan->minAirFlowRate = 0.0;

        fan->inletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                cAlphaArgs(3),
                                                                ErrorsFound,
                                                                DataLoopNode::ConnectionObjectType::FanConstantVolume,
                                                                cAlphaArgs(1),
                                                                DataLoopNode::NodeFluidType::Air,
                                                                DataLoopNode::ConnectionType::Inlet,
                                                                NodeInputManager::CompFluidStream::Primary,
                                                                DataLoopNode::ObjectIsNotParent);
        fan->outletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                 cAlphaArgs(4),
                                                                 ErrorsFound,
                                                                 DataLoopNode::ConnectionObjectType::FanConstantVolume,
                                                                 cAlphaArgs(1),
                                                                 DataLoopNode::NodeFluidType::Air,
                                                                 DataLoopNode::ConnectionType::Outlet,
                                                                 NodeInputManager::CompFluidStream::Primary,
                                                                 DataLoopNode::ObjectIsNotParent);

        fan->endUseSubcategoryName = (NumAlphas > 4) ? cAlphaArgs(5) : "General";

        BranchNodeConnections::TestCompSet(state, cCurrentModuleObject, cAlphaArgs(1), cAlphaArgs(3), cAlphaArgs(4), "Air Nodes");

        if (ErrorsFound) {
            ShowFatalError(state, format("{}: Errors found in input for fan name = {}.  Program terminates.", routineName, fan->Name));
        }
    } // for (iFanConstant)

    for (int VarVolFanNum = 1; VarVolFanNum <= NumVarVolFan; ++VarVolFanNum) {
        cCurrentModuleObject = "Fan:VariableVolume";
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 VarVolFanNum,
                                                                 cAlphaArgs,
                                                                 NumAlphas,
                                                                 rNumericArgs,
                                                                 NumNums,
                                                                 IOStat,
                                                                 lNumericFieldBlanks,
                                                                 lAlphaFieldBlanks,
                                                                 cAlphaFieldNames,
                                                                 cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};

        if (df->fanMap.find(cAlphaArgs(1)) != df->fanMap.end()) {
            ShowSevereDuplicateName(state, eoh);
            ErrorsFound = true;
        }

        auto *fan = new FanComponent;
        fan->Name = cAlphaArgs(1);

        df->fans.push_back(fan);
        df->fanMap.insert_or_assign(cAlphaArgs(1), df->fans.size());

        fan->type = HVAC::FanType::VAV;
        fan->sizingPrefix = cNumericFieldNames(3);

        if (lAlphaFieldBlanks(2)) {
            fan->availSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((fan->availSched = Sched::GetSchedule(state, cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(2), cAlphaArgs(2));
            ErrorsFound = true;
        }

        fan->totalEff = rNumericArgs(1);
        fan->deltaPress = rNumericArgs(2);
        fan->maxAirFlowRate = rNumericArgs(3);
        if (fan->maxAirFlowRate == 0.0) {
            ShowWarningError(
                state,
                format("{}=\"{}\" has specified 0.0 max air flow rate. It will not be used in the simulation.", cCurrentModuleObject, fan->Name));
        }
        fan->maxAirFlowRateIsAutosized = true;
        fan->minAirFracMethod = static_cast<MinFlowFracMethod>(getEnumValue(minFlowFracMethodNamesUC, cAlphaArgs(3)));

        fan->minFrac = rNumericArgs(4);
        fan->fixedMin = rNumericArgs(5);
        fan->motorEff = rNumericArgs(6);
        fan->motorInAirFrac = rNumericArgs(7);
        fan->coeffs[0] = rNumericArgs(8);
        fan->coeffs[1] = rNumericArgs(9);
        fan->coeffs[2] = rNumericArgs(10);
        fan->coeffs[3] = rNumericArgs(11);
        fan->coeffs[4] = rNumericArgs(12);
        if (fan->coeffs[0] == 0.0 && fan->coeffs[1] == 0.0 && fan->coeffs[2] == 0.0 && fan->coeffs[3] == 0.0 && fan->coeffs[4] == 0.0) {
            ShowWarningError(state, "Fan Coefficients are all zero.  No Fan power will be reported.");
            ShowContinueError(state, format("For {}, Fan={}", cCurrentModuleObject, cAlphaArgs(1)));
        }
        fan->inletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                cAlphaArgs(4),
                                                                ErrorsFound,
                                                                DataLoopNode::ConnectionObjectType::FanVariableVolume,
                                                                cAlphaArgs(1),
                                                                DataLoopNode::NodeFluidType::Air,
                                                                DataLoopNode::ConnectionType::Inlet,
                                                                NodeInputManager::CompFluidStream::Primary,
                                                                DataLoopNode::ObjectIsNotParent);
        fan->outletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                 cAlphaArgs(5),
                                                                 ErrorsFound,
                                                                 DataLoopNode::ConnectionObjectType::FanVariableVolume,
                                                                 cAlphaArgs(1),
                                                                 DataLoopNode::NodeFluidType::Air,
                                                                 DataLoopNode::ConnectionType::Outlet,
                                                                 NodeInputManager::CompFluidStream::Primary,
                                                                 DataLoopNode::ObjectIsNotParent);

        fan->endUseSubcategoryName = (NumAlphas > 5) ? cAlphaArgs(6) : "General";

        BranchNodeConnections::TestCompSet(state, cCurrentModuleObject, cAlphaArgs(1), cAlphaArgs(4), cAlphaArgs(5), "Air Nodes");

        if (ErrorsFound) {
            ShowFatalError(state, format("{}: Errors found in input for fan name = {}.  Program terminates.", routineName, fan->Name));
        }
    } // for (iFanVAV)

    for (int ExhFanNum = 1; ExhFanNum <= NumZoneExhFan; ++ExhFanNum) {
        cCurrentModuleObject = "Fan:ZoneExhaust";
        ip->getObjectItem(state,
                          cCurrentModuleObject,
                          ExhFanNum,
                          cAlphaArgs,
                          NumAlphas,
                          rNumericArgs,
                          NumNums,
                          IOStat,
                          lNumericFieldBlanks,
                          lAlphaFieldBlanks,
                          cAlphaFieldNames,
                          cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};

        if (df->fanMap.find(cAlphaArgs(1)) != df->fanMap.end()) {
            ShowSevereDuplicateName(state, eoh);
            ErrorsFound = true;
        }

        auto *fan = new FanComponent;
        fan->Name = cAlphaArgs(1);

        df->fans.push_back(fan);
        df->fanMap.insert_or_assign(fan->Name, df->fans.size());

        fan->type = HVAC::FanType::Exhaust;
        fan->sizingPrefix = cNumericFieldNames(3);

        if (lAlphaFieldBlanks(2)) {
            fan->availSched = Sched::GetScheduleAlwaysOn(state); // Not an availability schedule, but defaults to constant-1.0
        } else if ((fan->availSched = Sched::GetSchedule(state, cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(2), cAlphaArgs(2));
            ErrorsFound = true;
        } else if (fan->availSched->hasFractionalVal(state)) {
            ShowWarningCustom(
                state,
                eoh,
                format("{}={} has fracdtional values. Only 0.0 in the schedule value turns the fan off.", cAlphaFieldNames(2), cAlphaArgs(2)));
        }

        fan->totalEff = rNumericArgs(1);
        fan->deltaPress = rNumericArgs(2);
        fan->maxAirFlowRate = rNumericArgs(3);
        fan->maxAirFlowRateIsAutosized = false;
        fan->motorEff = 1.0;
        fan->motorInAirFrac = 1.0;
        fan->minAirFlowRate = 0.0;
        fan->rhoAirStdInit = state.dataEnvrn->StdRhoAir;
        fan->maxAirMassFlowRate = fan->maxAirFlowRate * fan->rhoAirStdInit;

        if (fan->maxAirFlowRate == 0.0) {
            ShowWarningError(
                state,
                format("{}=\"{}\" has specified 0.0 max air flow rate. It will not be used in the simulation.", cCurrentModuleObject, fan->Name));
        }

        fan->inletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                cAlphaArgs(3),
                                                                ErrorsFound,
                                                                DataLoopNode::ConnectionObjectType::FanZoneExhaust,
                                                                cAlphaArgs(1),
                                                                DataLoopNode::NodeFluidType::Air,
                                                                DataLoopNode::ConnectionType::Inlet,
                                                                NodeInputManager::CompFluidStream::Primary,
                                                                DataLoopNode::ObjectIsNotParent);
        fan->outletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                 cAlphaArgs(4),
                                                                 ErrorsFound,
                                                                 DataLoopNode::ConnectionObjectType::FanZoneExhaust,
                                                                 cAlphaArgs(1),
                                                                 DataLoopNode::NodeFluidType::Air,
                                                                 DataLoopNode::ConnectionType::Outlet,
                                                                 NodeInputManager::CompFluidStream::Primary,
                                                                 DataLoopNode::ObjectIsNotParent);

        fan->endUseSubcategoryName = (NumAlphas > 4 && !lAlphaFieldBlanks(5)) ? cAlphaArgs(5) : "General";

        if (NumAlphas <= 5 || lAlphaFieldBlanks(6)) {
            fan->flowFracSched = Sched::GetScheduleAlwaysOn(state); // Not an availability schedule, but defaults to constant-1.0
        } else if ((fan->flowFracSched = Sched::GetSchedule(state, cAlphaArgs(6))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(6), cAlphaArgs(6));
            ErrorsFound = true;
        } else if (!fan->flowFracSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 1.0)) {
            Sched::ShowSevereBadMinMax(state, eoh, cAlphaFieldNames(6), cAlphaArgs(6), Clusive::In, 0.0, Clusive::In, 1.0);
            ErrorsFound = true;
        }

        if (NumAlphas <= 6 || lAlphaFieldBlanks(7)) {
            fan->availManagerMode = AvailManagerMode::Coupled;
        } else if ((fan->availManagerMode = static_cast<AvailManagerMode>(getEnumValue(availManagerModeNamesUC, cAlphaArgs(7)))) ==
                   AvailManagerMode::Invalid) {
            ShowSevereInvalidKey(state, eoh, cAlphaFieldNames(7), cAlphaArgs(7));
            ErrorsFound = true;
        }

        if (NumAlphas <= 7 || lAlphaFieldBlanks(8)) {
            fan->minTempLimitSched = nullptr;
        } else if ((fan->minTempLimitSched = Sched::GetSchedule(state, cAlphaArgs(8))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(8), cAlphaArgs(8));
            ErrorsFound = true;
        }

        if (NumAlphas <= 8 || lAlphaFieldBlanks(9)) {
            fan->balancedFractSched = nullptr;
        } else if (state.dataHeatBal->ZoneAirMassFlow.ZoneFlowAdjustment != DataHeatBalance::AdjustmentType::NoAdjustReturnAndMixing) {
            // do not include adjusted for "balanced" exhaust flow in the zone total return calculation
            ShowWarningError(state,
                             format("{}: {}: invalid {} = {} for {}={}",
                                    routineName,
                                    cCurrentModuleObject,
                                    cAlphaFieldNames(9),
                                    cAlphaArgs(9),
                                    cAlphaFieldNames(1),
                                    cAlphaArgs(1)));
            ShowContinueError(state, "When zone air mass flow balance is enforced, this input field should be left blank.");
            ShowContinueError(state, "This schedule will be ignored in the simulation.");
            fan->balancedFractSched = nullptr;
        } else if ((fan->balancedFractSched = Sched::GetSchedule(state, cAlphaArgs(9))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(9), cAlphaArgs(9));
            ErrorsFound = true;
        } else if (!fan->balancedFractSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 1.0)) {
            Sched::ShowSevereBadMinMax(state, eoh, cAlphaFieldNames(9), cAlphaArgs(9), Clusive::In, 0.0, Clusive::In, 1.0);
            ErrorsFound = true;
        }
        if (ErrorsFound) {
            ShowFatalError(state, format("{}: Errors found in input for fan name = {}.  Program terminates.", routineName, fan->Name));
        }
    } // for (iFanExhaust)

    for (int OnOffFanNum = 1; OnOffFanNum <= NumOnOff; ++OnOffFanNum) {
        cCurrentModuleObject = "Fan:OnOff";
        ip->getObjectItem(state,
                          cCurrentModuleObject,
                          OnOffFanNum,
                          cAlphaArgs,
                          NumAlphas,
                          rNumericArgs,
                          NumNums,
                          IOStat,
                          lNumericFieldBlanks,
                          lAlphaFieldBlanks,
                          cAlphaFieldNames,
                          cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};

        if (df->fanMap.find(cAlphaArgs(1)) != df->fanMap.end()) {
            ShowSevereDuplicateName(state, eoh);
            ErrorsFound = true;
        }

        auto *fan = new FanComponent;
        fan->Name = cAlphaArgs(1);

        df->fans.push_back(fan);
        df->fanMap.insert_or_assign(fan->Name, df->fans.size());

        fan->type = HVAC::FanType::OnOff;
        fan->sizingPrefix = cNumericFieldNames(3);

        if (lAlphaFieldBlanks(2)) {
            fan->availSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((fan->availSched = Sched::GetSchedule(state, cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(2), cAlphaArgs(2));
            ErrorsFound = true;
        }

        fan->totalEff = rNumericArgs(1);
        fan->deltaPress = rNumericArgs(2);
        fan->maxAirFlowRate = rNumericArgs(3);
        if (fan->maxAirFlowRate == 0.0) {
            ShowWarningError(
                state,
                format("{}=\"{}\" has specified 0.0 max air flow rate. It will not be used in the simulation.", cCurrentModuleObject, fan->Name));
        }
        fan->maxAirFlowRateIsAutosized = true;
        //       the following two structure variables are set here, as well as in InitFan, for the Heat Pump:Water Heater object
        //       (Standard Rating procedure may be called before BeginEnvirFlag is set to TRUE, if so MaxAirMassFlowRate = 0)
        fan->rhoAirStdInit = state.dataEnvrn->StdRhoAir;
        fan->maxAirMassFlowRate = fan->maxAirFlowRate * fan->rhoAirStdInit;

        fan->motorEff = rNumericArgs(4);
        fan->motorInAirFrac = rNumericArgs(5);
        fan->minAirFlowRate = 0.0;

        fan->inletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                cAlphaArgs(3),
                                                                ErrorsFound,
                                                                DataLoopNode::ConnectionObjectType::FanOnOff,
                                                                cAlphaArgs(1),
                                                                DataLoopNode::NodeFluidType::Air,
                                                                DataLoopNode::ConnectionType::Inlet,
                                                                NodeInputManager::CompFluidStream::Primary,
                                                                DataLoopNode::ObjectIsNotParent);
        fan->outletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                 cAlphaArgs(4),
                                                                 ErrorsFound,
                                                                 DataLoopNode::ConnectionObjectType::FanOnOff,
                                                                 cAlphaArgs(1),
                                                                 DataLoopNode::NodeFluidType::Air,
                                                                 DataLoopNode::ConnectionType::Outlet,
                                                                 NodeInputManager::CompFluidStream::Primary,
                                                                 DataLoopNode::ObjectIsNotParent);

        if (NumAlphas > 4 && !lAlphaFieldBlanks(5)) {
            fan->powerRatioAtSpeedRatioCurveNum = Curve::GetCurveIndex(state, cAlphaArgs(5));
        }

        if (NumAlphas > 5 && !lAlphaFieldBlanks(6)) {
            fan->effRatioCurveNum = Curve::GetCurveIndex(state, cAlphaArgs(6));
        }

        fan->endUseSubcategoryName = (NumAlphas > 6 && !lAlphaFieldBlanks(7)) ? cAlphaArgs(7) : "General";

        BranchNodeConnections::TestCompSet(state, cCurrentModuleObject, cAlphaArgs(1), cAlphaArgs(3), cAlphaArgs(4), "Air Nodes");

        if (ErrorsFound) {
            ShowFatalError(state, format("{}: Errors found in input for fan name = {}.  Program terminates.", routineName, fan->Name));
        }
    } // for (iFanOnOff)

    if (state.dataFans->NumNightVentPerf > 0) {
        df->NightVentPerf.allocate(state.dataFans->NumNightVentPerf);
        for (auto &e : df->NightVentPerf) {
            e.FanName.clear();
            e.FanEff = 0.0;
            e.DeltaPress = 0.0;
            e.MaxAirFlowRate = 0.0;
            e.MotEff = 0.0;
            e.MotInAirFrac = 0.0;
            e.MaxAirMassFlowRate = 0.0;
        }

        // input the night ventilation performance objects
        for (int NVPerfNum = 1; NVPerfNum <= state.dataFans->NumNightVentPerf; ++NVPerfNum) {
            cCurrentModuleObject = "FanPerformance:NightVentilation";
            ip->getObjectItem(state,
                              cCurrentModuleObject,
                              NVPerfNum,
                              cAlphaArgs,
                              NumAlphas,
                              rNumericArgs,
                              NumNums,
                              IOStat,
                              lNumericFieldBlanks,
                              lAlphaFieldBlanks,
                              cAlphaFieldNames,
                              cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};

            auto &nvp = df->NightVentPerf(NVPerfNum);

            nvp.FanName = cAlphaArgs(1);
            nvp.FanEff = rNumericArgs(1);
            nvp.DeltaPress = rNumericArgs(2);
            nvp.MaxAirFlowRate = rNumericArgs(3);
            nvp.MotEff = rNumericArgs(4);
            nvp.MotInAirFrac = rNumericArgs(5);

            auto found = df->fanMap.find(nvp.FanName);
            if (found == df->fanMap.end()) {
                ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(1), cAlphaArgs(1));
                ErrorsFound = true;
            } else {
                auto *fan = dynamic_cast<FanComponent *>(df->fans(found->second));
                assert(fan != nullptr);
                fan->nightVentPerfNum = NVPerfNum;
            }
        } // for (iNightVent)
    }

    for (int CompModelFanNum = 1; CompModelFanNum <= NumCompModelFan; ++CompModelFanNum) {
        cCurrentModuleObject = "Fan:ComponentModel";
        ip->getObjectItem(state,
                          cCurrentModuleObject,
                          CompModelFanNum,
                          cAlphaArgs,
                          NumAlphas,
                          rNumericArgs,
                          NumNums,
                          IOStat,
                          lNumericFieldBlanks,
                          lAlphaFieldBlanks,
                          cAlphaFieldNames,
                          cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};

        if (df->fanMap.find(cAlphaArgs(1)) != df->fanMap.end()) {
            ShowSevereDuplicateName(state, eoh);
            ErrorsFound = true;
        }

        auto *fan = new FanComponent;
        fan->Name = cAlphaArgs(1); // Fan name

        df->fans.push_back(fan);
        df->fanMap.insert_or_assign(fan->Name, df->fans.size());

        fan->type = HVAC::FanType::ComponentModel;
        fan->sizingPrefix = cNumericFieldNames(1);

        fan->inletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                cAlphaArgs(2),
                                                                ErrorsFound,
                                                                DataLoopNode::ConnectionObjectType::FanComponentModel,
                                                                cAlphaArgs(1),
                                                                DataLoopNode::NodeFluidType::Air,
                                                                DataLoopNode::ConnectionType::Inlet,
                                                                NodeInputManager::CompFluidStream::Primary,
                                                                DataLoopNode::ObjectIsNotParent); // Air inlet node name
        fan->outletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                 cAlphaArgs(3),
                                                                 ErrorsFound,
                                                                 DataLoopNode::ConnectionObjectType::FanComponentModel,
                                                                 cAlphaArgs(1),
                                                                 DataLoopNode::NodeFluidType::Air,
                                                                 DataLoopNode::ConnectionType::Outlet,
                                                                 NodeInputManager::CompFluidStream::Primary,
                                                                 DataLoopNode::ObjectIsNotParent); // Air outlet node name

        BranchNodeConnections::TestCompSet(state, cCurrentModuleObject, cAlphaArgs(1), cAlphaArgs(2), cAlphaArgs(3), "Air Nodes");

        if (lAlphaFieldBlanks(4)) {
            fan->availSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((fan->availSched = Sched::GetSchedule(state, cAlphaArgs(4))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(4), cAlphaArgs(4));
            ErrorsFound = true;
        }

        fan->maxAirFlowRate = rNumericArgs(1);
        if (fan->maxAirFlowRate == 0.0) {
            ShowWarningError(
                state,
                format("{}=\"{}\" has specified 0.0 max air flow rate. It will not be used in the simulation.", cCurrentModuleObject, fan->Name));
        }
        fan->maxAirFlowRateIsAutosized = true;
        fan->minAirFlowRate = rNumericArgs(2);

        fan->sizingFactor = rNumericArgs(3);       // Fan max airflow sizing factor [-]
        fan->wheelDia = rNumericArgs(4);           // Fan wheel outer diameter [m]
        fan->outletArea = rNumericArgs(5);         // Fan outlet area [m2]
        fan->maxEff = rNumericArgs(6);             // Fan maximum static efficiency [-]
        fan->eulerMaxEff = rNumericArgs(7);        // Euler number at Fan maximum static efficiency [-]
        fan->maxDimFlow = rNumericArgs(8);         // Fan maximum dimensionless airflow [-]
        fan->pulleyDiaRatio = rNumericArgs(9);     // Motor/fan pulley diameter ratio [-]
        fan->beltMaxTorque = rNumericArgs(10);     // Belt maximum torque [N-m, autosizable]
        fan->beltSizingFactor = rNumericArgs(11);  // Belt sizing factor [-]
        fan->beltTorqueTrans = rNumericArgs(12);   // Belt fractional torque transition Region 1-2 [-]
        fan->motorMaxSpeed = rNumericArgs(13);     // Motor maximum speed [rpm]
        fan->motorMaxOutPower = rNumericArgs(14);  // Motor maximum output power [W, autosizable]
        fan->motorSizingFactor = rNumericArgs(15); // Motor sizing factor [-]
        fan->motorInAirFrac = rNumericArgs(16);    // Fraction of fan and motor losses to airstream [-]

        fan->vfdEffType = static_cast<VFDEffType>(getEnumValue(vfdEffTypeNamesUC, cAlphaArgs(5))); // VFD efficiency type [Speed or Power

        fan->vfdMaxOutPower = rNumericArgs(17);  // VFD maximum output power [W, autosizable]
        fan->vfdSizingFactor = rNumericArgs(18); // VFD sizing factor [-]

        // Do we need to do error checking on these curve names?
        if (lAlphaFieldBlanks(6)) {
            fan->pressRiseCurveNum = 0;
        } else if ((fan->pressRiseCurveNum = Curve::GetCurveIndex(state, cAlphaArgs(6))) == 0) { // Fan pressure rise curve
            ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(6), cAlphaArgs(6));
            ErrorsFound = true;
        }

        fan->pressResetCurveNum = Curve::GetCurveIndex(state, cAlphaArgs(7));      // Duct static pressure reset curve
        fan->plTotalEffNormCurveNum = Curve::GetCurveIndex(state, cAlphaArgs(8));  // Fan part-load eff (normal) curve
        fan->plTotalEffStallCurveNum = Curve::GetCurveIndex(state, cAlphaArgs(9)); // Fan part-load eff (stall) curve
        fan->dimFlowNormCurveNum = Curve::GetCurveIndex(state, cAlphaArgs(10));    // Fan dim airflow (normal) curve
        fan->dimFlowStallCurveNum = Curve::GetCurveIndex(state, cAlphaArgs(11));   // Fan dim airflow (stall) curve
        fan->beltMaxEffCurveNum = Curve::GetCurveIndex(state, cAlphaArgs(12));     // Belt max eff curve
        fan->plBeltEffReg1CurveNum = Curve::GetCurveIndex(state, cAlphaArgs(13));  // Belt part-load eff Region 1 curve
        fan->plBeltEffReg2CurveNum = Curve::GetCurveIndex(state, cAlphaArgs(14));  // Belt part-load eff Region 2 curve
        fan->plBeltEffReg3CurveNum = Curve::GetCurveIndex(state, cAlphaArgs(15));  // Belt part-load eff Region 3 curve
        fan->motorMaxEffCurveNum = Curve::GetCurveIndex(state, cAlphaArgs(16));    // Motor max eff curve
        fan->plMotorEffCurveNum = Curve::GetCurveIndex(state, cAlphaArgs(17));     // Motor part-load eff curve
        fan->vfdEffCurveNum = Curve::GetCurveIndex(state, cAlphaArgs(18));         // VFD eff curve

        fan->endUseSubcategoryName = (NumAlphas > 18) ? cAlphaArgs(19) : "General";

        if (ErrorsFound) {
            ShowFatalError(state, format("{}: Errors found in input for fan name = {}.  Program terminates.", routineName, fan->Name));
        }
    } // end Number of Component Model FAN Loop

    for (int SystemFanNum = 1; SystemFanNum <= NumSystemModelFan; ++SystemFanNum) {
        cCurrentModuleObject = "Fan:SystemModel";

        ip->getObjectItem(state,
                          cCurrentModuleObject,
                          SystemFanNum,
                          cAlphaArgs,
                          NumAlphas,
                          rNumericArgs,
                          NumNums,
                          IOStat,
                          lNumericFieldBlanks,
                          lAlphaFieldBlanks,
                          cAlphaFieldNames,
                          cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};

        if (df->fanMap.find(cAlphaArgs(1)) != df->fanMap.end()) {
            ShowSevereDuplicateName(state, eoh);
            ErrorsFound = true;
        }

        auto *fan = new FanSystem;
        fan->Name = cAlphaArgs(1);

        df->fans.push_back(fan);
        df->fanMap.insert_or_assign(fan->Name, df->fans.size());

        fan->type = HVAC::FanType::SystemModel;

        if (lAlphaFieldBlanks(2)) {
            fan->availSched = Sched::GetScheduleAlwaysOn(state); // nullptr
        } else if ((fan->availSched = Sched::GetSchedule(state, cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(2), cAlphaArgs(2));
            ErrorsFound = true;
        }

        fan->inletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                cAlphaArgs(3),
                                                                ErrorsFound,
                                                                DataLoopNode::ConnectionObjectType::FanSystemModel,
                                                                cAlphaArgs(1),
                                                                DataLoopNode::NodeFluidType::Air,
                                                                DataLoopNode::ConnectionType::Inlet,
                                                                NodeInputManager::CompFluidStream::Primary,
                                                                DataLoopNode::ObjectIsNotParent);
        fan->outletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                 cAlphaArgs(4),
                                                                 ErrorsFound,
                                                                 DataLoopNode::ConnectionObjectType::FanSystemModel,
                                                                 cAlphaArgs(1),
                                                                 DataLoopNode::NodeFluidType::Air,
                                                                 DataLoopNode::ConnectionType::Outlet,
                                                                 NodeInputManager::CompFluidStream::Primary,
                                                                 DataLoopNode::ObjectIsNotParent);

        BranchNodeConnections::TestCompSet(state, cCurrentModuleObject, cAlphaArgs(1), cAlphaArgs(3), cAlphaArgs(4), "Air Nodes");

        fan->maxAirFlowRate = rNumericArgs(1);
        if (fan->maxAirFlowRate == DataSizing::AutoSize) {
            fan->maxAirFlowRateIsAutosized = true;
        }

        if (lAlphaFieldBlanks(5)) {
            fan->speedControl = SpeedControl::Discrete;
        } else {
            fan->speedControl = static_cast<SpeedControl>(getEnumValue(speedControlNamesUC, cAlphaArgs(5)));
        }

        fan->minPowerFlowFrac = rNumericArgs(2);
        fan->deltaPress = rNumericArgs(3);
        if (fan->deltaPress <= 0.0) {
            ShowSevereError(state, format("{}: {} zero or negative, invalid entry in {}", routineName, cCurrentModuleObject, cNumericFieldNames(3)));
            ErrorsFound = true;
        }
        fan->motorEff = rNumericArgs(4);
        fan->motorInAirFrac = rNumericArgs(5);
        fan->designElecPower = rNumericArgs(6);
        if (fan->designElecPower == DataSizing::AutoSize) {
            fan->designElecPowerWasAutosized = true;
        }
        if (fan->designElecPowerWasAutosized) {
            if (lAlphaFieldBlanks(6)) {
                fan->powerSizingMethod = PowerSizing::PerFlowPerPressure;
            } else {
                fan->powerSizingMethod = static_cast<PowerSizing>(getEnumValue(powerSizingNamesUC, cAlphaArgs(6)));
            }
            fan->elecPowerPerFlowRate = rNumericArgs(7);
            fan->elecPowerPerFlowRatePerPressure = rNumericArgs(8);
            fan->totalEff = rNumericArgs(9);
        }

        if (lAlphaFieldBlanks(7)) {
            if (fan->speedControl == SpeedControl::Continuous) {
                ShowWarningError(state, format("{}{}=\"{}\", invalid entry.", routineName, cCurrentModuleObject, cAlphaArgs(1)));
                ShowContinueError(state,
                                  format("Continuous speed control requires a fan power curve in {} = {}", cAlphaFieldNames(7), cAlphaArgs(7)));
                ErrorsFound = true;
            }
        } else if ((fan->powerModFuncFlowFracCurveNum = Curve::GetCurveIndex(state, cAlphaArgs(7))) == 0) {
            ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(7), cAlphaArgs(7));
            if (fan->speedControl == SpeedControl::Continuous) {
                ErrorsFound = true;
            }
        }

        fan->nightVentPressureDelta = rNumericArgs(10);
        fan->nightVentFlowFraction = rNumericArgs(11); // not used

        if (lAlphaFieldBlanks(8)) {
            fan->heatLossDest = HeatLossDest::Outside;
        } else if ((fan->zoneNum = Util::FindItemInList(cAlphaArgs(8), state.dataHeatBal->Zone)) == 0) {
            fan->heatLossDest = HeatLossDest::Outside;
            ShowWarningItemNotFound(state, eoh, cAlphaFieldNames(8), cAlphaArgs(8), "Fan motor heat losses will not be added to a zone");
            // continue with simulation but motor losses not sent to a zone.
        } else {
            fan->heatLossDest = HeatLossDest::Zone;
        }

        fan->zoneRadFract = rNumericArgs(12);
        if (!lAlphaFieldBlanks(9)) {
            fan->endUseSubcategoryName = cAlphaArgs(9);
        } else {
            fan->endUseSubcategoryName = "General";
        }

        if (!lNumericFieldBlanks(13)) {
            fan->numSpeeds = rNumericArgs(13);
        } else {
            fan->numSpeeds = 1;
        }

        fan->runtimeFracAtSpeed.resize(fan->numSpeeds, 0.0);
        if (fan->speedControl == SpeedControl::Discrete && fan->numSpeeds > 1) {
            // should have field sets
            fan->flowFracAtSpeed.resize(fan->numSpeeds, 0.0);
            fan->powerFracAtSpeed.resize(fan->numSpeeds, 0.0);
            fan->powerFracInputAtSpeed.resize(fan->numSpeeds, false);
            if (fan->numSpeeds == ((NumNums - 13) / 2) || fan->numSpeeds == ((NumNums + 1 - 13) / 2)) {
                for (int loopSet = 0; loopSet < fan->numSpeeds; ++loopSet) {
                    fan->flowFracAtSpeed[loopSet] = rNumericArgs(13 + loopSet * 2 + 1);
                    if (!lNumericFieldBlanks(13 + loopSet * 2 + 2)) {
                        fan->powerFracAtSpeed[loopSet] = rNumericArgs(13 + loopSet * 2 + 2);
                        fan->powerFracInputAtSpeed[loopSet] = true;
                    } else {
                        fan->powerFracInputAtSpeed[loopSet] = false;
                    }
                }
            } else {
                // field set input does not match number of speeds, throw warning
                ShowSevereError(state, format("{}: {}=\"{}\", invalid entry.", routineName, cCurrentModuleObject, cAlphaArgs(1)));
                ShowContinueError(state, "Fan with Discrete speed control does not have input for speed data that matches the number of speeds.");
                ErrorsFound = true;
            }
            // check that flow fractions are increasing
            bool increasingOrderError = false;
            for (int loop = 0; loop < (fan->numSpeeds - 1); ++loop) {
                if (fan->flowFracAtSpeed[loop] > fan->flowFracAtSpeed[loop + 1]) {
                    increasingOrderError = true;
                }
            }
            if (increasingOrderError) {
                ShowSevereError(state, format("{}: {}=\"{}\", invalid entry.", routineName, cCurrentModuleObject, cAlphaArgs(1)));
                ShowContinueError(state,
                                  "Fan with Discrete speed control and multiple speed levels does not have input with flow fractions arranged in "
                                  "increasing order.");
                ErrorsFound = true;
            }
        }

        // check if power curve present when any speeds have no power fraction
        if (fan->speedControl == SpeedControl::Discrete && fan->numSpeeds > 1 && fan->powerModFuncFlowFracCurveNum == 0) {
            bool foundMissingPowerFraction = false;
            for (int loop = 0; loop < fan->numSpeeds; ++loop) {
                if (!fan->powerFracInputAtSpeed[loop]) {
                    foundMissingPowerFraction = true;
                }
            }
            if (foundMissingPowerFraction) {
                // field set input does not match number of speeds, throw warning
                ShowSevereError(state, format("{}: {}=\"{}\", invalid entry.", routineName, cCurrentModuleObject, cAlphaArgs(1)));
                ShowContinueError(
                    state,
                    "Fan with Discrete speed control does not have input for power fraction at all speed levels and does not have a power curve.");
                ErrorsFound = true;
            }
        }

        if (fan->heatLossDest == HeatLossDest::Zone) {
            SetupZoneInternalGain(
                state, fan->zoneNum, fan->Name, DataHeatBalance::IntGainType::FanSystemModel, &fan->qdotConvZone, nullptr, &fan->qdotRadZone);
        }
        if (ErrorsFound) {
            ShowFatalError(state, format("{}: Errors found in input for fan name = {}.  Program terminates.", routineName, fan->Name));
        }

    } // for (iFanSystemModel)

    cAlphaArgs.deallocate();
    cAlphaFieldNames.deallocate();
    lAlphaFieldBlanks.deallocate();
    cNumericFieldNames.deallocate();
    lNumericFieldBlanks.deallocate();
    rNumericArgs.deallocate();

    // Check Fans
    // There is a faster way to do this if this gets too slow (doubt it)
    for (auto const *fan1 : df->fans) {
        for (auto const *fan2 : df->fans) {
            if (fan1 == fan2) {
                continue;
            }

            if (fan1->inletNodeNum == fan2->inletNodeNum) {
                ErrorsFound = true;
                ShowSevereError(state, "GetFanInput, duplicate fan inlet node names, must be unique for fans.");
                ShowContinueError(state,
                                  format("Fan={}:{} and Fan={}:{}.",
                                         HVAC::fanTypeNames[(int)fan1->type],
                                         fan1->Name,
                                         HVAC::fanTypeNames[(int)fan2->type],
                                         fan2->Name));
                ShowContinueError(state, format("Inlet Node Name=\"{}\".", state.dataLoopNodes->NodeID(fan1->inletNodeNum)));
            }
            if (fan1->outletNodeNum == fan2->outletNodeNum) {
                ErrorsFound = true;
                ShowSevereError(state, "GetFanInput, duplicate fan outlet node names, must be unique for fans.");
                ShowContinueError(state,
                                  format("Fan={}:{} and Fan={}:{}.",
                                         HVAC::fanTypeNames[(int)fan1->type],
                                         fan1->Name,
                                         HVAC::fanTypeNames[(int)fan2->type],
                                         fan2->Name));
                ShowContinueError(state, format("Outlet Node Name=\"{}\".", state.dataLoopNodes->NodeID(fan1->outletNodeNum)));
            }
        }
    }

    for (auto *fan : df->fans) {
        // Setup Report variables for the Fans  CurrentModuleObject='Fans'
        SetupOutputVariable(state,
                            "Fan Electricity Rate",
                            Constant::Units::W,
                            fan->totalPower,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            fan->Name);
        SetupOutputVariable(state,
                            "Fan Rise in Air Temperature",
                            Constant::Units::deltaC,
                            fan->deltaTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            fan->Name);
        SetupOutputVariable(state,
                            "Fan Heat Gain to Air",
                            Constant::Units::W,
                            fan->powerLossToAir,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            fan->Name);
        SetupOutputVariable(state,
                            "Fan Electricity Energy",
                            Constant::Units::J,
                            fan->totalEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            fan->Name,
                            Constant::eResource::Electricity,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::Fans,
                            fan->endUseSubcategoryName);
        SetupOutputVariable(state,
                            "Fan Air Mass Flow Rate",
                            Constant::Units::kg_s,
                            fan->outletAirMassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            fan->Name);
        if (fan->type == HVAC::FanType::Exhaust) {
            auto *fanExhaust = dynamic_cast<FanComponent *>(fan);
            assert(fanExhaust != nullptr);
            if (fanExhaust->balancedFractSched != nullptr) {
                SetupOutputVariable(state,
                                    "Fan Unbalanced Air Mass Flow Rate",
                                    Constant::Units::kg_s,
                                    fanExhaust->unbalancedOutletMassFlowRate,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    fanExhaust->Name);
                SetupOutputVariable(state,
                                    "Fan Balanced Air Mass Flow Rate",
                                    Constant::Units::kg_s,
                                    fanExhaust->balancedOutletMassFlowRate,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    fanExhaust->Name);
            }
        }

        if (fan->type == HVAC::FanType::OnOff) {
            auto *fanOnOff = dynamic_cast<FanComponent *>(fan);
            assert(fanOnOff != nullptr);

            SetupOutputVariable(state,
                                "Fan Runtime Fraction",
                                Constant::Units::None,
                                fanOnOff->runtimeFrac,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                fanOnOff->Name);

        } else if (fan->type == HVAC::FanType::SystemModel) {
            auto *fanSystem = dynamic_cast<FanSystem *>(fan);
            assert(fanSystem != nullptr);

            if (fanSystem->speedControl != SpeedControl::Discrete) {
                continue;
            }

            if (fanSystem->numSpeeds == 1) {
                SetupOutputVariable(state,
                                    "Fan Runtime Fraction",
                                    Constant::Units::None,
                                    fanSystem->runtimeFracAtSpeed[0],
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    fanSystem->Name);
            } else {
                for (int speedLoop = 0; speedLoop < fanSystem->numSpeeds; ++speedLoop) {
                    SetupOutputVariable(state,
                                        format("Fan Runtime Fraction Speed {}", speedLoop + 1),
                                        Constant::Units::None,
                                        fanSystem->runtimeFracAtSpeed[speedLoop],
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        fan->Name);
                }
            }
        }

    } // for (fan)

    bool anyRan;
    EMSManager::ManageEMS(state, EMSManager::EMSCallFrom::ComponentGetInput, anyRan, ObjexxFCL::Optional_int_const());

    if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
        for (auto *fan : df->fans) {
            SetupEMSInternalVariable(state, "Fan Maximum Mass Flow Rate", fan->Name, "[kg/s]", fan->maxAirMassFlowRate);
            SetupEMSActuator(state, "Fan", fan->Name, "Fan Air Mass Flow Rate", "[kg/s]", fan->EMSMaxMassFlowOverrideOn, fan->EMSAirMassFlowValue);
            SetupEMSInternalVariable(state, "Fan Nominal Pressure Rise", fan->Name, "[Pa]", fan->deltaPress);
            SetupEMSActuator(state, "Fan", fan->Name, "Fan Pressure Rise", "[Pa]", fan->EMSPressureOverrideOn, fan->EMSPressureValue);
            SetupEMSInternalVariable(state, "Fan Nominal Total Efficiency", fan->Name, "[fraction]", fan->totalEff);
            SetupEMSActuator(state, "Fan", fan->Name, "Fan Total Efficiency", "[fraction]", fan->EMSTotalEffOverrideOn, fan->EMSTotalEffValue);
            SetupEMSActuator(
                state, "Fan", fan->Name, "Fan Autosized Air Flow Rate", "[m3/s]", fan->EMSMaxAirFlowRateOverrideOn, fan->EMSMaxAirFlowRateValue);
        }
    }
} // GetFanInput()

// End of Get Input subroutines for the HB Module
//******************************************************************************

// Beginning Initialization Section of the Module
//******************************************************************************

void FanComponent::init(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   February 1998

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is for initializations of the Fan Components.

    // METHODOLOGY EMPLOYED:
    // Uses the status flags to trigger initializations.
    auto &df = state.dataFans;

    // need to check all fans to see if they are on Zone Equipment List or issue warning
    if (!state.dataFans->ZoneEquipmentListChecked && state.dataZoneEquip->ZoneEquipInputsFilled) {
        state.dataFans->ZoneEquipmentListChecked = true;
        for (auto *fan : df->fans) {
            if (fan->type != HVAC::FanType::Exhaust) {
                continue;
            }
            if (DataZoneEquipment::CheckZoneEquipmentList(state, HVAC::fanTypeNames[(int)fan->type], fan->Name)) {
                continue;
            }
            ShowSevereError(state,
                            format("InitFans: Fan=[{},{}] is not on any ZoneHVAC:EquipmentList.  It will not be simulated.",
                                   HVAC::fanTypeNames[(int)fan->type],
                                   fan->Name));
        }
    }

    if (!state.dataGlobal->SysSizingCalc && sizingFlag) {

        set_size(state);
        // Set the loop cycling flag
        if (type == HVAC::FanType::OnOff) {
            if (state.dataSize->CurSysNum > 0) {
                state.dataAirLoop->AirLoopControlInfo(state.dataSize->CurSysNum).CyclingFan = true;
            }
        }

        sizingFlag = false;
    }

    // Do the Begin Environment initializations
    if (state.dataGlobal->BeginEnvrnFlag && envrnFlag) {

        // For all Fan inlet nodes convert the Volume flow to a mass flow
        rhoAirStdInit = state.dataEnvrn->StdRhoAir;

        // Change the Volume Flow Rates to Mass Flow Rates

        maxAirMassFlowRate = maxAirFlowRate * rhoAirStdInit;
        if (minAirFracMethod == MinFlowFracMethod::MinFrac) {
            minAirFlowRate = maxAirFlowRate * minFrac;
            minAirMassFlowRate = minAirFlowRate * rhoAirStdInit;
        } else if (minAirFracMethod == MinFlowFracMethod::FixedMin) {
            minAirFlowRate = fixedMin;
            minAirMassFlowRate = minAirFlowRate * rhoAirStdInit;
        }
        if (nightVentPerfNum > 0) {
            df->NightVentPerf(nightVentPerfNum).MaxAirMassFlowRate = df->NightVentPerf(nightVentPerfNum).MaxAirFlowRate * rhoAirStdInit;
        }

        // Init the Node Control variables
        state.dataLoopNodes->Node(outletNodeNum).MassFlowRateMax = maxAirMassFlowRate;
        // According to the IO Ref guide:
        // "Note that this field is only used to calculate the fan power.
        // This field does not enforce the system air flow rate during simulation"
        // Node(OutNode).MassFlowRateMin = fan.MinAirMassFlowRate;

        // Initialize all report variables to a known state at beginning of simulation
        totalPower = 0.0;
        deltaTemp = 0.0;
        powerLossToAir = 0.0;
        totalEnergy = 0.0;

        envrnFlag = false;
    }

    if (!state.dataGlobal->BeginEnvrnFlag) {
        envrnFlag = true;
    }

    // Do the following initializations (every time step): This should be the info from
    // the previous components outlets or the node data in this section.

    // Do a check and make sure that the max and min available(control) flow is
    // between the physical max and min for the Fan while operating.

    auto const &inletNode = state.dataLoopNodes->Node(inletNodeNum);
    auto const &outletNode = state.dataLoopNodes->Node(outletNodeNum);

    massFlowRateMaxAvail = min(outletNode.MassFlowRateMax, inletNode.MassFlowRateMaxAvail);
    massFlowRateMinAvail = min(max(outletNode.MassFlowRateMin, inletNode.MassFlowRateMinAvail), inletNode.MassFlowRateMaxAvail);

    // Load the node data in this section for the component simulation
    // First need to make sure that the MassFlowRate is between the max and min avail.
    if (type != HVAC::FanType::Exhaust) {
        inletAirMassFlowRate = min(inletNode.MassFlowRate, massFlowRateMaxAvail);
        inletAirMassFlowRate = max(inletAirMassFlowRate, massFlowRateMinAvail);
    } else { // zone exhaust fans
        massFlowRateMaxAvail = maxAirMassFlowRate;
        massFlowRateMinAvail = 0.0;
        if (flowFracSched != nullptr) { // modulate flow
            inletAirMassFlowRate = massFlowRateMaxAvail * flowFracSched->getCurrentVal();
            inletAirMassFlowRate = max(0.0, inletAirMassFlowRate);
        } else { // always run at max
            inletAirMassFlowRate = massFlowRateMaxAvail;
        }
        if (EMSMaxMassFlowOverrideOn) {
            inletAirMassFlowRate = min(EMSAirMassFlowValue, massFlowRateMaxAvail);
        }
    }

    // Then set the other conditions
    inletAirTemp = inletNode.Temp;
    inletAirHumRat = inletNode.HumRat;
    inletAirEnthalpy = inletNode.Enthalpy;
}

void FanComponent::set_size(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   September 2001
    //       MODIFIED       Craig Wray August 2010 - added fan, belt, motor, and VFD component sizing
    //                      August 2013 Daeho Kang, add component sizing table entries

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is for sizing fans for which flow rates have not been
    // specified in the input, or when fan component sizes have not been specified

    // METHODOLOGY EMPLOYED:
    // Obtains flow rates from the zone or system sizing arrays.

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view routineName = "FanComponent::set_size()"; // include trailing blank space

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool _bPRINT = true; // TRUE if sizing is reported to output (eio)

    std::string SizingString = sizingPrefix + " [m3/s]";

    Real64 _tempFlow = maxAirFlowRate; // autosized flow rate of fan [m3/s]
    state.dataSize->DataAutosizable = maxAirFlowRateIsAutosized;
    state.dataSize->DataEMSOverrideON = EMSMaxAirFlowRateOverrideOn;
    state.dataSize->DataEMSOverride = EMSMaxAirFlowRateValue;
    airLoopNum = state.dataSize->CurSysNum;

    bool errorsFound = false;
    SystemAirFlowSizer sizerSystemAirFlow;
    sizerSystemAirFlow.overrideSizingString(SizingString);
    sizerSystemAirFlow.initializeWithinEP(state, HVAC::fanTypeNames[(int)type], Name, _bPRINT, routineName);
    maxAirFlowRate = sizerSystemAirFlow.size(state, _tempFlow, errorsFound);

    state.dataSize->DataAutosizable = true;
    state.dataSize->DataEMSOverrideON = false;
    state.dataSize->DataEMSOverride = 0.0;

    Real64 _volFlow = maxAirFlowRate; // Maximum volumetric airflow through fan [m3/s at standard conditions]
    if (type == HVAC::FanType::ComponentModel) {
        // Get air density at standard conditions and get mass airflow through fan
        // From WeatherManager:
        //   StdBaroPress=(101.325d0*(1.0d0-2.25577d-05*WeatherFileElevation)**5.2559d0)*1000.d0
        //   StdRhoAir=PsyRhoAirFnPbTdbW(StdBaroPress,20,0)
        // From PsychRoutines:
        //   w=MAX(dw,1.0d-5)
        //   rhoair = pb/(287.d0*(tdb+Constant::Kelvin())*(1.0d0+1.6077687d0*w))
        Real64 _rhoAir = state.dataEnvrn->StdRhoAir;

        // Adjust max fan volumetric airflow using fan sizing factor
        _volFlow *= sizingFactor; //[m3/s at standard conditions]

        // Calculate max fan static pressure rise using max fan volumetric flow, std air density, air-handling system characteristics,
        //   and Sherman-Wray system curve model (assumes static pressure surrounding air distribution system is zero)
        Real64 _ductStaticPress = Curve::CurveValue(state, pressResetCurveNum, _volFlow);                // Duct static pressure setpoint [Pa]
        Real64 _deltaPressTot = Curve::CurveValue(state, pressRiseCurveNum, _volFlow, _ductStaticPress); // Max fan total pressure rise [Pa]
        Real64 _outletVelPress = 0.5 * _rhoAir * pow_2(_volFlow / outletArea);                           // Max fan outlet velocity pressure [Pa]
        // Outlet velocity pressure cannot exceed total pressure rise
        _outletVelPress = min(_outletVelPress, _deltaPressTot);
        deltaPress = _deltaPressTot - _outletVelPress; // Max fan static pressure rise [Pa]

        // Calculate max fan air power using volumetric flow abd corresponding fan static pressure rise
        airPower = _volFlow * deltaPress; //[W]

        // Calculate fan wheel efficiency at max fan volumetric flow and corresponding fan static pressure rise,
        //   using fan characteristics and Wray dimensionless fan static efficiency model
        Real64 _eulerNum = (deltaPress * pow_4(wheelDia)) / (_rhoAir * pow_2(_volFlow)); //[-]
        Real64 _normalizedEulerNum = std::log10(_eulerNum / eulerMaxEff);
        wheelEff = (_normalizedEulerNum <= 0.0) ? Curve::CurveValue(state, plTotalEffNormCurveNum, _normalizedEulerNum)
                                                : Curve::CurveValue(state, plTotalEffStallCurveNum, _normalizedEulerNum);

        wheelEff = max(wheelEff * maxEff, 0.01); // Minimum efficiency is 1% to avoid numerical errors

        // Calculate max fan shaft power using fan air power and fan efficiency
        // at max fan static pressure rise and max fan volumetric flow
        shaftPower = (airPower / wheelEff); //[W]
        shaftPowerMax = shaftPower;         //[W]

        // Calculate fan shaft speed, motor speed, and fan torque using Wray dimensionless fan airflow model
        Real64 _dimFlow = (_normalizedEulerNum <= 0.0) ? // Fan dimensionless airflow [-]
                              Curve::CurveValue(state, dimFlowNormCurveNum, _normalizedEulerNum)
                                                       : Curve::CurveValue(state, dimFlowStallCurveNum, _normalizedEulerNum); //[-]

        Real64 _speedRadS = _volFlow / (_dimFlow * maxDimFlow * pow_3(wheelDia)); //[rad/s]
        fanSpeed = _speedRadS * 9.549296586;                                      //[rpm, conversion factor is 30/PI]

        if (pulleyDiaRatio == DataSizing::AutoSize) {
            // WRITE(*,*) 'Autosizing pulley drive ratio'
            pulleyDiaRatio = fanSpeed / motorMaxSpeed; //[-]
        }

        // For direct-drive, should have PulleyDiaRatio = 1
        Real64 _motorSpeed = fanSpeed / pulleyDiaRatio; //[rpm]

        // Check for inconsistent drive ratio and motor speed, and report design fan speed with warning
        if (_motorSpeed > (motorMaxSpeed + 1.e-5)) {
            ShowWarningError(state,
                             format("Drive ratio for {}: {} is too low at design conditions -- check motor speed and drive ratio inputs",
                                    HVAC::fanTypeNames[(int)type],
                                    Name));
            ShowContinueError(state, format("...Design fan speed [rev/min]: {:.2R}", fanSpeed));
        }

        fanTorque = shaftPower / _speedRadS; //[N-m]

        if (beltMaxTorque == DataSizing::AutoSize) {
            // WRITE(*,*) 'Autosizing fan belt'
            beltMaxTorque = fanTorque; //[N-m]
        }
        // Adjust max belt torque using belt sizing factor
        beltMaxTorque *= beltSizingFactor; //[N-m]

        // Check for undersized belt and report design size with warning
        if (fanTorque > (beltMaxTorque + 1.e-5)) {
            ShowWarningError(state,
                             format("Belt for {}: {} is undersized at design conditions -- check belt inputs", HVAC::fanTypeNames[(int)type], Name));
            ShowContinueError(state, format("...Design belt output torque (without oversizing) [Nm]: {:.2R}", fanTorque));
        }

        // Calculate belt max efficiency using correlations and coefficients based on AMCA data
        // Direct-drive is represented using curve coefficients such that "belt" max eff and PL eff = 1.0
        Real64 _XbeltMax = std::log(shaftPowerMax / 746.0); // Natural log of belt output power in hp
        beltMaxEff = (beltMaxEffCurveNum != 0) ? std::exp(Curve::CurveValue(state, beltMaxEffCurveNum, _XbeltMax)) : 1.0;

        // Calculate belt part-load drive efficiency and input power using correlations and coefficients based on ACEEE data
        Real64 _torqueRatio = fanTorque / beltMaxTorque; //[-]
        Real64 _plBeltEff;                               // Belt normalized (part-load) efficiency [-]
        if ((_torqueRatio <= beltTorqueTrans) && (plBeltEffReg1CurveNum != 0)) {
            _plBeltEff = Curve::CurveValue(state, plBeltEffReg1CurveNum, _torqueRatio); //[-]
        } else if ((_torqueRatio > beltTorqueTrans) && (_torqueRatio <= 1.0) && (plBeltEffReg2CurveNum != 0)) {
            _plBeltEff = Curve::CurveValue(state, plBeltEffReg2CurveNum, _torqueRatio); //[-]
        } else if ((_torqueRatio > 1.0) && (plBeltEffReg3CurveNum != 0)) {
            _plBeltEff = Curve::CurveValue(state, plBeltEffReg3CurveNum, _torqueRatio); //[-]
        } else {
            _plBeltEff = 1.0; // Direct drive or no curve specified - use constant efficiency
        }
        beltEff = beltMaxEff * _plBeltEff;     //[-]
        beltEff = max(beltEff, 0.01);          // Minimum efficiency is 1% to avoid numerical errors
        beltInputPower = shaftPower / beltEff; //[W]

        if (motorMaxOutPower == DataSizing::AutoSize) {
            // WRITE(*,*) 'Autosizing fan motor'
            motorMaxOutPower = beltInputPower;
        }
        // Adjust max motor output power using motor sizing factor
        motorMaxOutPower *= motorSizingFactor; //[W]

        // Check for undersized motor and report design size with warning
        if (beltInputPower > (motorMaxOutPower + 1.e-5)) {
            ShowWarningError(
                state, format("Motor for {}: {} is undersized at design conditions -- check motor inputs", HVAC::fanTypeNames[(int)type], Name));
            ShowContinueError(state, format("...Design motor output power (without oversizing) [W]: {:.2R}", beltInputPower));
        }

        // Calculate motor max efficiency using correlations and coefficients based on MotorMaster+ data
        Real64 _XmotorMax = std::log(motorMaxOutPower / 746.0); // Natural log of motor output power in hp
        motorMaxEff = (motorMaxEffCurveNum != 0) ? Curve::CurveValue(state, motorMaxEffCurveNum, _XmotorMax) : 1.0;

        // Calculate motor part-load efficiency and input power using correlations and coefficients based on MotorMaster+ data
        Real64 _motorOutPowerRatio = beltInputPower / motorMaxOutPower; //[-]
        // Motor normalized (part-load) efficiency [-]
        Real64 _plMotorEff = (plMotorEffCurveNum != 0) ? Curve::CurveValue(state, plMotorEffCurveNum, _motorOutPowerRatio) : 1.0;

        motorEff = max(motorMaxEff * _plMotorEff, 0.01);

        // Calculate motor input power using belt input power and motor efficiency
        motorInputPower = beltInputPower / motorEff; //[W]

        // Calculate max VFD efficiency and input power using correlations and coefficients based on VFD type
        if ((vfdEffType == VFDEffType::Speed) && (vfdEffCurveNum != 0)) {
            Real64 _vfdSpeedRatio = _motorSpeed / motorMaxSpeed; // Ratio of motor speed to motor max speed [-]
            vfdEff = Curve::CurveValue(state, vfdEffCurveNum, _vfdSpeedRatio);
        } else if ((vfdEffType == VFDEffType::Power) && (vfdEffCurveNum != 0)) {
            if (vfdMaxOutPower == DataSizing::AutoSize) {
                // WRITE(*,*) 'Autosizing fan VFD'
                vfdMaxOutPower = motorInputPower;
            }
            // Adjust max VFD output power using VFD sizing factor
            vfdMaxOutPower *= vfdSizingFactor; //[W]

            // Check for undersized VFD and report design size with warning
            if (motorInputPower > (vfdMaxOutPower + 1.e-5)) {
                ShowWarningError(
                    state, format("VFD for {}: {} is undersized at design conditions -- check VFD inputs", HVAC::fanTypeNames[(int)type], Name));
                ShowContinueError(state, format("...Design VFD output power (without oversizing) [W]: {:.2R}", motorInputPower));
            }

            Real64 _vfdOutPowerRatio = motorInputPower / vfdMaxOutPower; // Ratio of VFD output power to max VFD output power [-]
            vfdEff = Curve::CurveValue(state, vfdEffCurveNum, _vfdOutPowerRatio);
        } else {
            // No curve specified - use constant efficiency
            vfdMaxOutPower = 0.0;
            vfdEff = 0.97;
        }

        vfdEff = max(vfdEff, 0.01); // Minimum efficiency is 1% to avoid numerical errors

        // Calculate VFD "rated" input power using motor input power and VFD efficiency
        Real64 _ratedPower = motorInputPower / vfdEff; //[W]

        // Calculate combined fan system efficiency: includes fan, belt, motor, and VFD
        // Equivalent to fan%FanAirPower / fan%FanPower
        totalEff = wheelEff * beltEff * motorEff * vfdEff;

        // Report fan, belt, motor, and VFD characteristics at design condition to .eio file
        std::string_view fanTypeName = HVAC::fanTypeNames[(int)type];

        BaseSizer::reportSizerOutput(state, fanTypeName, Name, "Design Fan Airflow [m3/s]", _volFlow);
        BaseSizer::reportSizerOutput(state, fanTypeName, Name, "Design Fan Static Pressure Rise [Pa]", deltaPress);
        BaseSizer::reportSizerOutput(state, fanTypeName, Name, "Design Fan Shaft Power [W]", shaftPower);
        BaseSizer::reportSizerOutput(state, fanTypeName, Name, "Design Motor Output Power [W]", motorMaxOutPower);
        BaseSizer::reportSizerOutput(state, fanTypeName, Name, "Design VFD Output Power [W]", vfdMaxOutPower);
        BaseSizer::reportSizerOutput(state, fanTypeName, Name, "Rated Power [W]", _ratedPower);
        BaseSizer::reportSizerOutput(state, fanTypeName, Name, "Drive Ratio []", pulleyDiaRatio);
        BaseSizer::reportSizerOutput(state, fanTypeName, Name, "Design Belt Output Torque [Nm]", beltMaxTorque);
        BaseSizer::reportSizerOutput(state, fanTypeName, Name, "Design Fan Efficiency  []", wheelEff);
        BaseSizer::reportSizerOutput(state, fanTypeName, Name, "Maximum Belt Efficiency []", beltMaxEff);
        BaseSizer::reportSizerOutput(state, fanTypeName, Name, "Design Belt Efficiency []", beltEff);
        BaseSizer::reportSizerOutput(state, fanTypeName, Name, "Maximum Motor Efficiency []", motorMaxEff);
        BaseSizer::reportSizerOutput(state, fanTypeName, Name, "Design Motor Efficiency []", motorEff);
        BaseSizer::reportSizerOutput(state, fanTypeName, Name, "Design VFD Efficiency []", vfdEff);
        BaseSizer::reportSizerOutput(state, fanTypeName, Name, "Design Combined Efficiency []", totalEff);
    } // End fan component sizing

    // Rearrange order to match table and use FanVolFlow to calculate RatedPower
    // ALSO generates values if Component Model fan, for which DeltaPress and FanEff vary with flow
    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanType, Name, HVAC::fanTypeNames[(int)type]);
    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanTotEff, Name, totalEff);
    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanDeltaP, Name, deltaPress);
    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanVolFlow, Name, _volFlow);
    Real64 _ratedPower = _volFlow * deltaPress / totalEff; // total fan power
    BaseSizer::reportSizerOutput(state, HVAC::fanTypeNames[(int)type], Name, "Design Electric Power Consumption [W]", _ratedPower);
    if (type != HVAC::FanType::ComponentModel) {
        designPointFEI = FanSystem::report_fei(state, _volFlow, _ratedPower, deltaPress);
    }
    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanPwr, Name, _ratedPower);
    if (_volFlow != 0.0) {
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanPwrPerFlow, Name, _ratedPower / _volFlow);
    }
    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanMotorIn, Name, motorInAirFrac);
    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanEndUse, Name, endUseSubcategoryName);
    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanEnergyIndex, Name, designPointFEI);

    // Std 229 Fans (Fans.cc)
    OutputReportPredefined::PreDefTableEntry(
        state, state.dataOutRptPredefined->pdchFanPurpose, Name, "N/A"); // fan.FanType); // purpose? not the same
    OutputReportPredefined::PreDefTableEntry(state,
                                             state.dataOutRptPredefined->pdchFanAutosized,
                                             Name,
                                             maxAirFlowRateIsAutosized ? "Yes" : "No"); // autosizable vs. autosized equivalent?
    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanMotorEff, Name, motorEff);
    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanMotorHeatToZoneFrac, Name, 0.0);
    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanMotorHeatZone, Name, "N/A");
    if (airLoopNum == 0) {
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanAirLoopName, Name, "N/A");
    } else if (airLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys) {
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchFanAirLoopName, Name, state.dataAirSystemsData->PrimaryAirSystems(airLoopNum).Name);
    } else {
        OutputReportPredefined::PreDefTableEntry(
            state,
            state.dataOutRptPredefined->pdchFanAirLoopName,
            Name,
            state.dataAirLoopHVACDOAS->airloopDOAS[airLoopNum - state.dataHVACGlobal->NumPrimaryAirSys - 1].Name);
    }

    if (nightVentPerfNum > 0) {
        if (state.dataFans->NightVentPerf(nightVentPerfNum).MaxAirFlowRate == DataSizing::AutoSize) {
            state.dataFans->NightVentPerf(nightVentPerfNum).MaxAirFlowRate = maxAirFlowRate;
        }
    }

    // Now that sizing is done, do check if the design point of fan is covered in the fault Fan Curve
    if (faultyFilterFlag) {
        auto &fault = state.dataFaultsMgr->FaultsFouledAirFilters(faultyFilterIndex);

        // Check fault availability schedules
        if (!fault.CheckFaultyAirFilterFanCurve(state)) {
            ShowSevereError(state, format("FaultModel:Fouling:AirFilter = \"{}\"", fault.Name));
            ShowContinueError(state,
                              format("Invalid Fan Curve Name = \"{}\" does not cover ", state.dataCurveManager->curves(fault.fanCurveNum)->Name));
            ShowContinueError(state, format("the operational point of Fan {}", Name));
            ShowFatalError(state, format("SizeFan: Invalid FaultModel:Fouling:AirFilter={}", fault.Name));
        }
    }
} // FanComponent::set_size()

void FanComponent::simulateConstant(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Unknown
    //       DATE WRITTEN   Unknown
    //       MODIFIED       Brent Griffith, May 2009, added EMS override
    //                      Rongpeng Zhang, April 2015, added faulty fan operations due to fouling air filters

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine simulates the simple constant volume fan.

    // METHODOLOGY EMPLOYED:
    // Converts design pressure rise and efficiency into fan power and temperature rise
    // Constant fan pressure rise is assumed.

    // REFERENCES:
    // ASHRAE HVAC 2 Toolkit, page 2-3 (FANSIM)

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 _deltaPress; // [N/m2]
    Real64 _totalEff;
    Real64 _motorInAirFrac;
    Real64 _motorEff;
    Real64 _shaftPower; // power delivered to fan shaft

    if (state.dataHVACGlobal->NightVentOn && nightVentPerfNum > 0) {
        auto const &nightVentPerf = state.dataFans->NightVentPerf(nightVentPerfNum);
        _deltaPress = nightVentPerf.DeltaPress;
        _totalEff = nightVentPerf.FanEff;
        _motorEff = nightVentPerf.MotEff;
        _motorInAirFrac = nightVentPerf.MotInAirFrac;
    } else {
        _deltaPress = deltaPress;
        _totalEff = totalEff;
        _motorEff = motorEff;
        _motorInAirFrac = motorInAirFrac;
    }

    // For a Constant Volume Simple Fan the Max Flow Rate is the Flow Rate for the fan
    Real64 _rhoAir = rhoAirStdInit;
    Real64 _massFlow = inletAirMassFlowRate;

    // Faulty fan operations
    // Update MassFlow & DeltaPress if there are fouling air filters corresponding to the fan
    if (faultyFilterFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) && (!state.dataGlobal->KickOffSimulation)) {
        auto &fault = state.dataFaultsMgr->FaultsFouledAirFilters(faultyFilterIndex);
        // Check fault availability schedules
        if (fault.availSched->getCurrentVal() > 0.0) {
            // Decrease of the Fan Design Volume Flow Rate [m3/sec]
            Real64 _fanDesignFlowRateDec = CalFaultyFanAirFlowReduction(
                state, Name, maxAirFlowRate, deltaPress, (fault.pressFracSched->getCurrentVal() - 1) * deltaPress, fault.fanCurveNum);

            // Update MassFlow & DeltaPress of the fan
            _massFlow = min(_massFlow, maxAirMassFlowRate - _fanDesignFlowRateDec * _rhoAir);
            _deltaPress = fault.pressFracSched->getCurrentVal() * deltaPress;
        }
    }

    // EMS overwrite MassFlow, DeltaPress, and FanEff
    if (EMSMaxMassFlowOverrideOn) {
        _massFlow = EMSAirMassFlowValue;
    }
    if (EMSPressureOverrideOn) {
        _deltaPress = EMSPressureValue;
    }
    if (EMSTotalEffOverrideOn) {
        _totalEff = EMSTotalEffValue;
    }

    _massFlow = min(_massFlow, maxAirMassFlowRate);
    _massFlow = max(_massFlow, minAirMassFlowRate);

    // Determine the Fan Schedule for the Time step
    if ((availSched->getCurrentVal() > 0.0 || state.dataHVACGlobal->TurnFansOn) && !state.dataHVACGlobal->TurnFansOff && _massFlow > 0.0) {
        // Fan is operating
        totalPower = max(0.0, _massFlow * _deltaPress / (_totalEff * _rhoAir)); // total fan power
        _shaftPower = _motorEff * totalPower;                                   // power delivered to shaft
        powerLossToAir = _shaftPower + (totalPower - _shaftPower) * _motorInAirFrac;
        outletAirEnthalpy = inletAirEnthalpy + powerLossToAir / _massFlow;
        // This fan does not change the moisture or Mass Flow across the component
        outletAirHumRat = inletAirHumRat;
        outletAirMassFlowRate = _massFlow;
        outletAirTemp = Psychrometrics::PsyTdbFnHW(outletAirEnthalpy, outletAirHumRat);

    } else {
        // Fan is off and not operating no power consumed and mass flow rate.
        totalPower = 0.0;
        powerLossToAir = 0.0;
        outletAirMassFlowRate = 0.0;
        outletAirHumRat = inletAirHumRat;
        outletAirEnthalpy = inletAirEnthalpy;
        outletAirTemp = inletAirTemp;
        // Set the Control Flow variables to 0.0 flow when OFF.
        massFlowRateMaxAvail = 0.0;
        massFlowRateMinAvail = 0.0;
    }
} // FanComponent::simulateConstant

void FanComponent::simulateVAV(EnergyPlusData &state, ObjexxFCL::Optional<Real64 const> _pressureRise)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Unknown
    //       DATE WRITTEN   Unknown
    //       MODIFIED       Phil Haves
    //                      Brent Griffith, May 2009 for EMS
    //                      Rongpeng Zhang, April 2015, added faulty fan operations due to fouling air filters

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine simulates the simple variable volume fan.

    // METHODOLOGY EMPLOYED:
    // Converts design pressure rise and efficiency into fan power and temperature rise
    // Constant fan pressure rise is assumed.
    // Uses curves of fan power fraction vs. fan part load to determine fan power at
    // off design conditions.

    // REFERENCES:
    // ASHRAE HVAC 2 Toolkit, page 2-3 (FANSIM)

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 _deltaPress; // [N/m2 = Pa]
    Real64 _totalEff;   // Total fan efficiency - combined efficiency of fan, drive train,
    Real64 _maxAirMassFlowRate;
    Real64 _motorInAirFrac;
    Real64 _motorEff;
    Real64 _partLoadFrac;

    // Simple Variable Volume Fan - default values from DOE-2
    // Type of Fan          Coeff1       Coeff2       Coeff3        Coeff4      Coeff5
    // INLET VANE DAMPERS   0.35071223   0.30850535   -0.54137364   0.87198823  0.000
    // DISCHARGE DAMPERS    0.37073425   0.97250253   -0.34240761   0.000       0.000
    // VARIABLE SPEED MOTOR 0.0015302446 0.0052080574  1.1086242   -0.11635563  0.000

    if (state.dataHVACGlobal->NightVentOn && nightVentPerfNum > 0) {
        auto const &nightVentPerf = state.dataFans->NightVentPerf(nightVentPerfNum);
        _deltaPress = nightVentPerf.DeltaPress;
        _totalEff = nightVentPerf.FanEff;
        _motorEff = nightVentPerf.MotEff;
        _motorInAirFrac = nightVentPerf.MotInAirFrac;
        _maxAirMassFlowRate = nightVentPerf.MaxAirMassFlowRate;
    } else {
        if (present(_pressureRise)) {
            _deltaPress = _pressureRise;
        } else {
            _deltaPress = deltaPress;
        }
        _totalEff = totalEff;
        _motorEff = motorEff;
        _motorInAirFrac = motorInAirFrac;
        _maxAirMassFlowRate = maxAirMassFlowRate;
    }

    Real64 _rhoAir = rhoAirStdInit;
    Real64 _massFlow = inletAirMassFlowRate;
    Real64 _maxAirFlowRate = maxAirFlowRate;

    // Faulty fan operations
    // Update MassFlow & DeltaPress if there are fouling air filters corresponding to the fan
    if (faultyFilterFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) && (!state.dataGlobal->KickOffSimulation) &&
        (!EMSMaxMassFlowOverrideOn)) {

        auto &fault = state.dataFaultsMgr->FaultsFouledAirFilters(faultyFilterIndex);
        // Check fault availability schedules
        if (fault.availSched->getCurrentVal() > 0.0) {
            Real64 _fanDesignFlowRateDec = // Decrease of the Fan Design Volume Flow Rate [m3/sec]
                CalFaultyFanAirFlowReduction(
                    state, Name, maxAirFlowRate, deltaPress, (fault.pressFracSched->getCurrentVal() - 1) * deltaPress, fault.fanCurveNum);

            // Update MassFlow & DeltaPress of the fan
            _maxAirFlowRate = maxAirFlowRate - _fanDesignFlowRateDec;
            _maxAirMassFlowRate = maxAirMassFlowRate - _fanDesignFlowRateDec * _rhoAir;
            _deltaPress = fault.pressFracSched->getCurrentVal() * deltaPress;
        }
    }

    // EMS overwrite MassFlow, DeltaPress, and FanEff
    if (EMSPressureOverrideOn) {
        _deltaPress = EMSPressureValue;
    }
    if (EMSTotalEffOverrideOn) {
        _totalEff = EMSTotalEffValue;
    }
    if (EMSMaxMassFlowOverrideOn) {
        _massFlow = EMSAirMassFlowValue;
    }

    _massFlow = min(_massFlow, _maxAirMassFlowRate);

    // Determine the Fan Schedule for the Time step
    if ((availSched->getCurrentVal() > 0.0 || state.dataHVACGlobal->TurnFansOn) && !state.dataHVACGlobal->TurnFansOff && _massFlow > 0.0) {
        // Fan is operating - calculate power loss and enthalpy rise
        //  fan%FanPower = PartLoadFrac*FullMassFlow*DeltaPress/(FanEff*RhoAir) ! total fan power
        // Calculate and check limits on fraction of system flow
        // unused0909    MaxFlowFrac = 1.0
        // MinFlowFrac is calculated from the ration of the volume flows and is non-dimensional
        Real64 _minFlowFrac = minAirFlowRate / _maxAirFlowRate;
        // The actual flow fraction is calculated from MassFlow and the MaxVolumeFlow * AirDensity
        Real64 _flowFracActual = _massFlow / _maxAirMassFlowRate;

        // Calculate the part Load Fraction             (PH 7/13/03)

        Real64 _flowFracForPower = max(_minFlowFrac, min(_flowFracActual, 1.0)); // limit flow fraction to allowed range
        if (state.dataHVACGlobal->NightVentOn && nightVentPerfNum > 0) {
            _partLoadFrac = 1.0;
        } else {
            _partLoadFrac = coeffs[0] + coeffs[1] * _flowFracForPower + coeffs[2] * pow_2(_flowFracForPower) + coeffs[3] * pow_3(_flowFracForPower) +
                            coeffs[4] * pow_4(_flowFracForPower);
        }

        totalPower = max(0.0, _partLoadFrac * _maxAirMassFlowRate * _deltaPress / (_totalEff * _rhoAir)); // total fan power (PH 7/13/03)

        Real64 _shaftPower = _motorEff * totalPower; // power delivered to shaft
        powerLossToAir = _shaftPower + (totalPower - _shaftPower) * _motorInAirFrac;
        outletAirEnthalpy = inletAirEnthalpy + powerLossToAir / _massFlow;
        // This fan does not change the moisture or Mass Flow across the component
        outletAirHumRat = inletAirHumRat;
        outletAirMassFlowRate = _massFlow;
        outletAirTemp = Psychrometrics::PsyTdbFnHW(outletAirEnthalpy, outletAirHumRat);

        // KHL/FB, 2/10/2011. NFP implemented as CR 8338.
        // When fan air flow is less than 10%, the fan power curve is linearized between the 10% to 0% to
        //  avoid the unrealistic high temperature rise across the fan.
        // TH, 2/15/2011
        // This change caused diffs for VAV systems when fan runs at less than 10% flow conditions.
        //  A potential way to improve is to check the temperature rise across the fan first,
        //  if it is too high (say > 20C) then applies the code.
        Real64 _deltaTAcrossFan = outletAirTemp - inletAirTemp;
        if (_deltaTAcrossFan > 20.0) {
            // added to address the fan heat issue during low air flow conditions
            Real64 _fanPoweratLowMinimum; // Fan Power at Low Minimum Airflow [W]
            Real64 _partLoadFracatLowMin;
            Real64 _minFlowFracLimitFanHeat = 0.10;
            if (_flowFracForPower < _minFlowFracLimitFanHeat) {
                _partLoadFracatLowMin = coeffs[0] + coeffs[1] * _minFlowFracLimitFanHeat + coeffs[2] * pow_2(_minFlowFracLimitFanHeat) +
                                        coeffs[3] * pow_3(_minFlowFracLimitFanHeat) + coeffs[4] * pow_4(_minFlowFracLimitFanHeat);
                _fanPoweratLowMinimum = _partLoadFracatLowMin * _maxAirMassFlowRate * _deltaPress / (_totalEff * _rhoAir);
                totalPower = max(0.0, _flowFracForPower * _fanPoweratLowMinimum / _minFlowFracLimitFanHeat);
            } else if (_flowFracActual < _minFlowFracLimitFanHeat) {
                _partLoadFracatLowMin = coeffs[0] + coeffs[1] * _minFlowFracLimitFanHeat + coeffs[2] * pow_2(_minFlowFracLimitFanHeat) +
                                        coeffs[3] * pow_3(_minFlowFracLimitFanHeat) + coeffs[4] * pow_4(_minFlowFracLimitFanHeat);
                _fanPoweratLowMinimum = _partLoadFracatLowMin * _maxAirMassFlowRate * _deltaPress / (_totalEff * _rhoAir);
                totalPower = max(0.0, _flowFracActual * _fanPoweratLowMinimum / _minFlowFracLimitFanHeat);
            }
            _shaftPower = _motorEff * totalPower; // power delivered to shaft
            powerLossToAir = _shaftPower + (totalPower - _shaftPower) * _motorInAirFrac;
            outletAirEnthalpy = inletAirEnthalpy + powerLossToAir / _massFlow;
            // This fan does not change the moisture or Mass Flow across the component
            outletAirHumRat = inletAirHumRat;
            outletAirMassFlowRate = _massFlow;
            outletAirTemp = Psychrometrics::PsyTdbFnHW(outletAirEnthalpy, outletAirHumRat);
        }

    } else {
        // Fan is off and not operating no power consumed and mass flow rate.
        totalPower = 0.0;
        powerLossToAir = 0.0;
        outletAirMassFlowRate = 0.0;
        outletAirHumRat = inletAirHumRat;
        outletAirEnthalpy = inletAirEnthalpy;
        outletAirTemp = inletAirTemp;
        // Set the Control Flow variables to 0.0 flow when OFF.
        massFlowRateMaxAvail = 0.0;
        massFlowRateMinAvail = 0.0;
    }
} // FanComponent::SimVAV()

void FanComponent::simulateOnOff(EnergyPlusData &state, ObjexxFCL::Optional<Real64 const> _speedRatio)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Unknown
    //       DATE WRITTEN   Unknown
    //       MODIFIED       Shirey, May 2001
    //                      R. Raustad - FSEC, Jan 2009 - added SpeedRatio for multi-speed fans
    //                      Brent Griffith, May 2009 for EMS
    //                      Rongpeng Zhang, April 2015, added faulty fan operations due to fouling air filters

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine simulates the simple on/off fan.

    // METHODOLOGY EMPLOYED:
    // Converts design pressure rise and efficiency into fan power and temperature rise
    // Constant fan pressure rise is assumed.
    // Uses curves of fan power fraction vs. fan part load to determine fan power at
    // off design conditions.
    // Same as simple (constant volume) fan, except added part-load curve input

    // REFERENCES:
    // ASHRAE HVAC 2 Toolkit, page 2-3 (FANSIM)

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 _effRatioAtSpeedRatio; // Efficiency ratio at current speed ratio (Curve object)

    Real64 _massFlow = inletAirMassFlowRate;
    Real64 _maxAirMassFlowRate = maxAirMassFlowRate;
    Real64 _deltaPress = deltaPress; // [N/m2]
    Real64 _totalEff = totalEff;
    Real64 _rhoAir = rhoAirStdInit;

    // Faulty fan operations
    // Update MassFlow & DeltaPress if there are fouling air filters corresponding to the fan
    if (faultyFilterFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) && (!state.dataGlobal->KickOffSimulation) &&
        (!EMSMaxMassFlowOverrideOn)) {

        auto &fault = state.dataFaultsMgr->FaultsFouledAirFilters(faultyFilterIndex);

        // Check fault availability schedules
        if (fault.availSched->getCurrentVal() > 0.0) {
            Real64 _fanDesignFlowRateDec = // Decrease of the Fan Design Volume Flow Rate [m3/sec]
                CalFaultyFanAirFlowReduction(
                    state, Name, maxAirFlowRate, deltaPress, (fault.pressFracSched->getCurrentVal() - 1) * deltaPress, fault.fanCurveNum);

            // Update MassFlow & DeltaPress of the fan
            _maxAirMassFlowRate = maxAirMassFlowRate - _fanDesignFlowRateDec * _rhoAir;
            _deltaPress = fault.pressFracSched->getCurrentVal() * deltaPress;
        }
    }

    // EMS overwrite MassFlow, DeltaPress, and FanEff
    if (EMSMaxMassFlowOverrideOn) {
        _massFlow = EMSAirMassFlowValue;
    }
    if (EMSPressureOverrideOn) {
        _deltaPress = EMSPressureValue;
    }
    if (EMSTotalEffOverrideOn) {
        _totalEff = EMSTotalEffValue;
    }

    _massFlow = min(_massFlow, _maxAirMassFlowRate);
    _massFlow = max(_massFlow, minAirMassFlowRate);
    runtimeFrac = 0.0;

    // Determine the Fan Schedule for the Time step
    if ((availSched->getCurrentVal() > 0.0 || state.dataHVACGlobal->TurnFansOn) && !state.dataHVACGlobal->TurnFansOff && _massFlow > 0.0 &&
        maxAirMassFlowRate > 0.0) {
        // The actual flow fraction is calculated from MassFlow and the MaxVolumeFlow * AirDensity
        Real64 _flowFrac = _massFlow / _maxAirMassFlowRate;

        // Calculate the part load ratio, can't be greater than 1
        Real64 _partLoadRatio = min(1.0, _flowFrac);
        // Fan is operating
        if (state.dataHVACGlobal->OnOffFanPartLoadFraction <= 0.0) {
            ShowRecurringWarningErrorAtEnd(state, "Fan:OnOff, OnOffFanPartLoadFraction <= 0.0, Reset to 1.0", state.dataFans->ErrCount);
            state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0; // avoid divide by zero or negative PLF
        }

        if (state.dataHVACGlobal->OnOffFanPartLoadFraction < 0.7) {
            state.dataHVACGlobal->OnOffFanPartLoadFraction = 0.7; // a warning message is already issued from the DX coils or gas heating coil
        }

        // Keep fan runtime fraction between 0.0 and 1.0, and RTF >= PLR
        if (state.dataHVACGlobal->OnOffFanPartLoadFraction >= 1.0) {
            runtimeFrac = _partLoadRatio;
        } else {
            runtimeFrac = max(0.0, min(1.0, _partLoadRatio / state.dataHVACGlobal->OnOffFanPartLoadFraction));
        }
        // The fan speed ratio (passed from parent) determines the fan power according to fan laws
        if (present(_speedRatio)) {
            //    fan%FanPower = MassFlow*DeltaPress/(FanEff*RhoAir*OnOffFanPartLoadFraction)! total fan power
            totalPower = max(0.0, _maxAirMassFlowRate * runtimeFrac * _deltaPress / (_totalEff * _rhoAir));

            //    Do not modify fan power calculation unless fan power vs speed ratio curve is used.
            if (powerRatioAtSpeedRatioCurveNum > 0) {

                //      adjust RTF to be in line with speed ratio (i.e., MaxAirMassFlowRate is not MAX when SpeedRatio /= 1)
                //      PLR = Mdot/MAXFlow => Mdot/(MAXFlow * SpeedRatio), RTF = PLR/PLF => PLR/SpeedRatio/PLF = RTF / SpeedRatio
                if (_speedRatio > 0.0) {
                    runtimeFrac = min(1.0, runtimeFrac / _speedRatio);
                }

                Real64 _speedRaisedToPower = Curve::CurveValue(state, powerRatioAtSpeedRatioCurveNum, _speedRatio);
                if (_speedRaisedToPower < 0.0) {
                    if (oneTimePowerRatioCheck && !state.dataGlobal->WarmupFlag) {
                        ShowSevereError(state, format("{} = {}\"", HVAC::fanTypeNames[(int)type], Name));
                        ShowContinueError(state, "Error in Fan Power Ratio curve. Curve output less than 0.0.");
                        ShowContinueError(state, format("Curve output = {:.5T}, fan speed ratio = {:.5T}", _speedRaisedToPower, _speedRatio));
                        ShowContinueError(state, "Check curve coefficients to ensure proper power ratio as a function of fan speed ratio.");
                        ShowContinueError(state, "Resetting Fan Power Ratio curve output to 0.0 and the simulation continues.");
                        ShowContinueErrorTimeStamp(state, "Occurrence info:");
                        oneTimePowerRatioCheck = false;
                    }
                    _speedRaisedToPower = 0.0;
                }
                if (effRatioCurveNum > 0 && !state.dataGlobal->WarmupFlag) {
                    _effRatioAtSpeedRatio = Curve::CurveValue(state, effRatioCurveNum, _speedRatio);
                    if (_effRatioAtSpeedRatio < 0.01) {
                        if (oneTimeEffRatioCheck && !state.dataGlobal->WarmupFlag) {
                            ShowSevereError(state, format("{} = {}\"", HVAC::fanTypeNames[(int)type], Name));
                            ShowContinueError(state, "Error in Fan Efficiency Ratio curve. Curve output less than 0.01.");
                            ShowContinueError(state, format("Curve output = {:.5T}, fan speed ratio = {:.5T}", _effRatioAtSpeedRatio, _speedRatio));
                            ShowContinueError(state, "Check curve coefficients to ensure proper efficiency ratio as a function of fan speed ratio.");
                            ShowContinueError(state, "Resetting Fan Efficiency Ratio curve output to 0.01 and the simulation continues.");
                            ShowContinueErrorTimeStamp(state, "Occurrence info:");
                            oneTimeEffRatioCheck = false;
                        }
                        _effRatioAtSpeedRatio = 0.01;
                    }
                } else {
                    _effRatioAtSpeedRatio = 1.0;
                }
                totalPower *= _speedRaisedToPower / _effRatioAtSpeedRatio;
            }
        } else {
            totalPower = max(0.0, _maxAirMassFlowRate * runtimeFrac * _deltaPress / (_totalEff * _rhoAir)); // total fan power
        }

        // OnOffFanPartLoadFraction is passed via DataHVACGlobals from the cooling or heating coil that is
        //   requesting the fan to operate in cycling fan/cycling coil mode
        state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0; // reset to 1 in case other on/off fan is called without a part load curve
        Real64 _shaftPower = motorEff * totalPower;           // power delivered to shaft
        powerLossToAir = _shaftPower + (totalPower - _shaftPower) * motorInAirFrac;
        outletAirEnthalpy = inletAirEnthalpy + powerLossToAir / _massFlow;
        // This fan does not change the moisture or Mass Flow across the component
        outletAirHumRat = inletAirHumRat;
        outletAirMassFlowRate = _massFlow;
        //   fan%OutletAirTemp = Tin + PowerLossToAir/(MassFlow*PsyCpAirFnW(Win,Tin))
        outletAirTemp = Psychrometrics::PsyTdbFnHW(outletAirEnthalpy, outletAirHumRat);
    } else {
        // Fan is off and not operating no power consumed and mass flow rate.
        totalPower = 0.0;
        powerLossToAir = 0.0;
        outletAirMassFlowRate = 0.0;
        outletAirHumRat = inletAirHumRat;
        outletAirEnthalpy = inletAirEnthalpy;
        outletAirTemp = inletAirTemp;
        // Set the Control Flow variables to 0.0 flow when OFF.
        massFlowRateMaxAvail = 0.0;
        massFlowRateMinAvail = 0.0;
    }
} // FanComponent::simulateOnOff()

void FanComponent::simulateZoneExhaust(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   Jan 2000
    //       MODIFIED       Brent Griffith, May 2009 for EMS
    //                      Brent Griffith, Feb 2013 controls upgrade

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine simulates the Zone Exhaust Fan

    // METHODOLOGY EMPLOYED:
    // Converts design pressure rise and efficiency into fan power and temperature rise
    // Constant fan pressure rise is assumed.

    // REFERENCES:
    // ASHRAE HVAC 2 Toolkit, page 2-3 (FANSIM)

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool _fanIsRunning = false; // There seems to be a missing else case below unless false is assumed

    Real64 _deltaPress = deltaPress; // [N/m2]
    if (EMSPressureOverrideOn) {
        _deltaPress = EMSPressureValue;
    }

    Real64 _totalEff = totalEff;
    if (EMSTotalEffOverrideOn) {
        _totalEff = EMSTotalEffValue;
    }

    // For a Constant Volume Simple Fan the Max Flow Rate is the Flow Rate for the fan
    Real64 _Tin = inletAirTemp;
    Real64 _rhoAir = rhoAirStdInit;
    Real64 _massFlow = inletAirMassFlowRate;

    //  When the AvailManagerMode == ExhaustFanCoupledToAvailManagers then the
    //  Exhaust Fan is  interlocked with air loop availability via global TurnFansOn and TurnFansOff variables.
    //  There is now the option to control if user wants to decouple air loop operation and exhaust fan operation
    //  (zone air mass balance issues).

    // apply controls to determine if operating
    if (availManagerMode == AvailManagerMode::Coupled) {
        if (((availSched->getCurrentVal() > 0.0) || state.dataHVACGlobal->TurnFansOn) && !state.dataHVACGlobal->TurnFansOff &&
            _massFlow > 0.0) { // available
            _fanIsRunning = (minTempLimitSched != nullptr) ? (_Tin >= minTempLimitSched->getCurrentVal()) : true;
        } else {
            _fanIsRunning = false;
        }

    } else if (availManagerMode == AvailManagerMode::Decoupled) {
        if (availSched->getCurrentVal() > 0.0 && _massFlow > 0.0) {
            _fanIsRunning = (minTempLimitSched != nullptr) ? (_Tin >= minTempLimitSched->getCurrentVal()) : true;
        } else {
            _fanIsRunning = false;
        }
    }

    if (_fanIsRunning) {
        // Fan is operating
        totalPower = max(0.0, _massFlow * _deltaPress / (_totalEff * _rhoAir)); // total fan power
        powerLossToAir = totalPower;
        outletAirEnthalpy = inletAirEnthalpy + powerLossToAir / _massFlow;
        // This fan does not change the moisture or Mass Flow across the component
        outletAirHumRat = inletAirHumRat;
        outletAirMassFlowRate = _massFlow;
        outletAirTemp = Psychrometrics::PsyTdbFnHW(outletAirEnthalpy, outletAirHumRat);

    } else {
        // Fan is off and not operating no power consumed and mass flow rate.
        totalPower = 0.0;
        powerLossToAir = 0.0;
        outletAirMassFlowRate = 0.0;
        outletAirHumRat = inletAirHumRat;
        outletAirEnthalpy = inletAirEnthalpy;
        outletAirTemp = inletAirTemp;
        // Set the Control Flow variables to 0.0 flow when OFF.
        massFlowRateMaxAvail = 0.0;
        massFlowRateMinAvail = 0.0;
        inletAirMassFlowRate = 0.0;
    }
} // FanComponent::SimulateZoneExhaust()

void FanComponent::simulateComponentModel(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Craig Wray, LBNL
    //       DATE WRITTEN   Feb 2010

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine simulates the component model fan.

    // METHODOLOGY EMPLOYED:
    // Calculate fan volumetric flow and corresponding fan static pressure rise,
    //    using air-handling system characteristics and Sherman-Wray system curve model
    // Calculate fan air power using volumetric flow and fan static pressure rise
    // Calculate fan wheel efficiency using fan volumetric flow, fan static pressure rise,
    //   fan characteristics, and Wray dimensionless fan static efficiency model
    // Calculate fan shaft power using fan air power and fan static efficiency
    // Calculate fan shaft speed and torque using Wray dimensionless fan airflow model
    // Calculate belt part-load efficiency using correlations and coefficients based on ACEEE data
    // Calculate belt input power using fan shaft power and belt efficiency
    // Calculate motor part-load efficiency using correlations and coefficients based on MotorMaster+ data
    // Calculate motor input power using belt input power and motor efficiency
    // Calculate VFD efficiency using correlations and coefficients based on DOE data
    // Calculate VFD input power using motor input power and VFD efficiency
    // Calculate combined efficiency of fan, belt, motor, and VFD
    // Calculate air temperature rise due to fan (and belt+motor if in airstream) power entering air-handler airflow
    // Calculate output node conditions

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 _maxAirMassFlowRate; // Fan Max mass airflow [kg/s]
    Real64 _motorInAirFrac;     // Fraction of fan power input to airstream

    // Local variables
    Real64 _dimFlow;               // Fan dimensionless airflow [-]
    Real64 _beltPLEff;             // Belt normalized (part-load) efficiency [-]
    Real64 _motorPLEff;            // Motor normalized (part-load) efficiency [-]
    Real64 _vfdSpeedRatio(0.0);    // Ratio of motor speed to motor max speed [-]
    Real64 _vfdOutPowerRatio(0.0); // Ratio of VFD output power to max VFD output power [-]

    if (state.dataHVACGlobal->NightVentOn && nightVentPerfNum > 0) {
        _motorInAirFrac = state.dataFans->NightVentPerf(nightVentPerfNum).MotInAirFrac;
        _maxAirMassFlowRate = state.dataFans->NightVentPerf(nightVentPerfNum).MaxAirMassFlowRate;
    } else {
        _motorInAirFrac = motorInAirFrac;
        _maxAirMassFlowRate = maxAirMassFlowRate;
    }

    // Get air density at standard conditions and get mass airflow through fan
    // From WeatherManager:
    //   StdBaroPress=(101.325d0*(1.0d0-2.25577d-05*WeatherFileElevation)**5.2559d0)*1000.d0
    //   StdRhoAir=PsyRhoAirFnPbTdbW(StdBaroPress,20,0)
    // From PsychRoutines:
    //   w=MAX(dw,1.0d-5)
    //   rhoair = pb/(287.d0*(tdb+Constant::Kelvin())*(1.0d0+1.6077687d0*w))
    Real64 _rhoAir = rhoAirStdInit;
    Real64 _massFlow = min(inletAirMassFlowRate, maxAirMassFlowRate);

    //  IF (fan%EMSMaxMassFlowOverrideOn) MassFlow   = fan%EMSAirMassFlowValue

    // Determine the Fan Schedule for the Time step
    if ((availSched->getCurrentVal() > 0.0 || state.dataHVACGlobal->TurnFansOn) && !state.dataHVACGlobal->TurnFansOff && _massFlow > 0.0) {
        // Fan is operating - calculate fan pressure rise, component efficiencies and power, and also air enthalpy rise

        // Calculate fan static pressure rise using fan volumetric flow, std air density, air-handling system characteristics,
        //   and Sherman-Wray system curve model (assumes static pressure surrounding air distribution system is zero)
        Real64 _volFlow = _massFlow / _rhoAir;                                                           //[m3/s at standard conditions]
        Real64 _ductStaticPress = Curve::CurveValue(state, pressResetCurveNum, _volFlow);                // Duct static pressure setpoint [Pa]
        Real64 _deltaPressTot = Curve::CurveValue(state, pressRiseCurveNum, _volFlow, _ductStaticPress); // Fan total pressure rise [Pa]
        Real64 _outletVelPress = 0.5 * _rhoAir * pow_2(_volFlow / outletArea);                           // Fan outlet velocity pressure [Pa]
        // Outlet velocity pressure cannot exceed total pressure rise
        _outletVelPress = min(_outletVelPress, _deltaPressTot);
        deltaPress = _deltaPressTot - _outletVelPress; // Fan static pressure rise [Pa]

        //    IF (fan%EMSFanPressureOverrideOn) DeltaPress = fan%EMSFanPressureValue

        // Calculate fan static air power using volumetric flow and fan static pressure rise
        airPower = _volFlow * deltaPress; //[W]

        // Calculate fan wheel efficiency using fan volumetric flow, fan static pressure rise,
        //   fan characteristics, and Wray dimensionless fan static efficiency model
        Real64 _eulerNum = (deltaPress * pow_4(wheelDia)) / (_rhoAir * pow_2(_volFlow)); //[-]
        Real64 _normalizedEulerNum = std::log10(_eulerNum / eulerMaxEff);
        if (_normalizedEulerNum <= 0.0) {
            wheelEff = Curve::CurveValue(state, plTotalEffNormCurveNum, _normalizedEulerNum);
        } else {
            wheelEff = Curve::CurveValue(state, plTotalEffStallCurveNum, _normalizedEulerNum);
        }
        wheelEff *= maxEff;             // [-]
        wheelEff = max(wheelEff, 0.01); // Minimum efficiency is 1% to avoid numerical errors

        // Calculate fan shaft power using fan static air power and fan static efficiency
        shaftPower = airPower / wheelEff; //[W]

        // Calculate fan shaft speed, fan torque, and motor speed using Wray dimensionless fan airflow model
        if (_normalizedEulerNum <= 0.0) {
            _dimFlow = Curve::CurveValue(state, dimFlowNormCurveNum, _normalizedEulerNum); //[-]
        } else {
            _dimFlow = Curve::CurveValue(state, dimFlowStallCurveNum, _normalizedEulerNum); //[-]
        }
        Real64 _speedRadS = _volFlow / (_dimFlow * maxDimFlow * pow_3(wheelDia)); //[rad/s]
        fanTorque = shaftPower / _speedRadS;                                      //[N-m]
        fanSpeed = _speedRadS * 9.549296586;                                      //[rpm, conversion factor is 30/PI]
        Real64 _motorSpeed = fanSpeed * pulleyDiaRatio;                           //[rpm]

        // Calculate belt part-load drive efficiency using correlations and coefficients based on ACEEE data
        // Direct-drive is represented using curve coefficients such that "belt" max eff and PL eff = 1.0
        Real64 _torqueRatio = fanTorque / beltMaxTorque; //[-]
        if ((_torqueRatio <= beltTorqueTrans) && (plBeltEffReg1CurveNum != 0)) {
            _beltPLEff = Curve::CurveValue(state, plBeltEffReg1CurveNum, _torqueRatio); //[-]
        } else if ((_torqueRatio > beltTorqueTrans) && (_torqueRatio <= 1.0) && (plBeltEffReg2CurveNum != 0)) {
            _beltPLEff = Curve::CurveValue(state, plBeltEffReg2CurveNum, _torqueRatio); //[-]
        } else if ((_torqueRatio > 1.0) && (plBeltEffReg3CurveNum != 0)) {
            _beltPLEff = Curve::CurveValue(state, plBeltEffReg3CurveNum, _torqueRatio); //[-]
        } else {
            _beltPLEff = 1.0; // Direct drive or no curve specified - use constant efficiency
        }
        beltEff = beltMaxEff * _beltPLEff; //[-]
        beltEff = max(beltEff, 0.01);      // Minimum efficiency is 1% to avoid numerical errors

        // Calculate belt input power using fan shaft power and belt efficiency
        beltInputPower = shaftPower / beltEff; //[W]

        // Calculate motor part-load efficiency using correlations and coefficients based on MotorMaster+ data
        Real64 _motorOutPowerRatio = beltInputPower / motorMaxOutPower; //[-]
        if (plMotorEffCurveNum != 0) {
            _motorPLEff = Curve::CurveValue(state, plMotorEffCurveNum, _motorOutPowerRatio); //[-]
        } else {
            _motorPLEff = 1.0; // No curve specified - use constant efficiency
        }
        motorEff = motorMaxEff * _motorPLEff; //[-]
        motorEff = max(motorEff, 0.01);       // Minimum efficiency is 1% to avoid numerical errors

        // Calculate motor input power using belt input power and motor efficiency
        motorInputPower = beltInputPower / motorEff; //[W]

        // Calculate VFD efficiency using correlations and coefficients based on VFD type
        if ((vfdEffType == VFDEffType::Speed) && (vfdEffCurveNum != 0)) {
            _vfdSpeedRatio = _motorSpeed / motorMaxSpeed;                      //[-]
            vfdEff = Curve::CurveValue(state, vfdEffCurveNum, _vfdSpeedRatio); //[-]
        } else if ((vfdEffType == VFDEffType::Power) && (vfdEffCurveNum != 0)) {
            _vfdOutPowerRatio = motorInputPower / vfdMaxOutPower;                 //[-]
            vfdEff = Curve::CurveValue(state, vfdEffCurveNum, _vfdOutPowerRatio); //[-]
        } else {
            // No curve specified - use constant efficiency
            vfdMaxOutPower = 0.0;
            vfdEff = 0.97;
        }
        vfdEff = max(vfdEff, 0.01); // Minimum efficiency is 1% to avoid numerical errors

        // Calculate VFD input power using motor input power and VFD efficiency
        vfdInputPower = motorInputPower / vfdEff; //[W]
        totalPower = vfdInputPower;               //[W]

        // Calculate combined fan system efficiency: includes fan, belt, motor, and VFD
        // Equivalent to fan%FanAirPower / fan%FanPower
        totalEff = wheelEff * beltEff * motorEff * vfdEff;

        //    IF (fan%EMSFanEffOverrideOn) FanEff = fan%EMSFanEffValue

        // Calculate air enthalpy and temperature rise from power entering air stream from fan wheel, belt, and motor
        // Assumes MotInAirFrac applies to belt and motor but NOT to VFD
        powerLossToAir = shaftPower + (motorInputPower - shaftPower) * motorInAirFrac; //[W]
        outletAirEnthalpy = inletAirEnthalpy + (powerLossToAir / _massFlow);           //[kJ/kg]

        // This fan does not change the moisture or mass flow across the component
        outletAirHumRat = inletAirHumRat;  //[-]
        outletAirMassFlowRate = _massFlow; //[kg/s]
        outletAirTemp = Psychrometrics::PsyTdbFnHW(outletAirEnthalpy, outletAirHumRat);
    } else {
        // Fan is OFF and not operating -- no power consumed and zero mass flow rate
        totalPower = 0.0;
        shaftPower = 0.0;
        powerLossToAir = 0.0;
        outletAirMassFlowRate = 0.0;
        outletAirHumRat = inletAirHumRat;
        outletAirEnthalpy = inletAirEnthalpy;
        outletAirTemp = inletAirTemp;
        // Set the Control Flow variables to 0.0 flow when OFF.
        massFlowRateMaxAvail = 0.0;
        massFlowRateMinAvail = 0.0;

        deltaPress = 0.0;
        airPower = 0.0;
        wheelEff = 0.0;
        fanSpeed = 0.0;
        fanTorque = 0.0;
        beltEff = 0.0;
        beltInputPower = 0.0;
        motorEff = 0.0;
        motorInputPower = 0.0;
        vfdEff = 0.0;
        vfdInputPower = 0.0;
        totalEff = 0.0;
    }
} // FanComponent::simulateComponentModel()

void FanComponent::update(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   April 1998
    //       MODIFIED       L. Gu, Feb. 1, 2007, No unbalance airflow when Zone Exhaust Fans are used in the AirflowNetwork

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine updates the fan outlet nodes.

    // METHODOLOGY EMPLOYED:
    // Data is moved from the fan data structure to the fan outlet nodes.

    auto &inletNode = state.dataLoopNodes->Node(inletNodeNum);
    auto &outletNode = state.dataLoopNodes->Node(outletNodeNum);

    // Set the outlet air nodes of the fan
    outletNode.MassFlowRate = outletAirMassFlowRate;
    outletNode.Temp = outletAirTemp;
    outletNode.HumRat = outletAirHumRat;
    outletNode.Enthalpy = outletAirEnthalpy;
    // Set the outlet nodes for properties that just pass through & not used
    outletNode.Quality = inletNode.Quality;
    outletNode.Press = inletNode.Press;

    // Set the Node Flow Control Variables from the Fan Control Variables
    outletNode.MassFlowRateMaxAvail = massFlowRateMaxAvail;
    outletNode.MassFlowRateMinAvail = massFlowRateMinAvail;

    if (type == HVAC::FanType::Exhaust) {
        inletNode.MassFlowRate = inletAirMassFlowRate;
        if (state.afn->AirflowNetworkNumOfExhFan == 0) {
            state.dataHVACGlobal->UnbalExhMassFlow = inletAirMassFlowRate;
            if (balancedFractSched != nullptr) {
                state.dataHVACGlobal->BalancedExhMassFlow = state.dataHVACGlobal->UnbalExhMassFlow * balancedFractSched->getCurrentVal();
                state.dataHVACGlobal->UnbalExhMassFlow = state.dataHVACGlobal->UnbalExhMassFlow - state.dataHVACGlobal->BalancedExhMassFlow;
            } else {
                state.dataHVACGlobal->BalancedExhMassFlow = 0.0;
            }
        } else {
            state.dataHVACGlobal->UnbalExhMassFlow = 0.0;
            state.dataHVACGlobal->BalancedExhMassFlow = 0.0;
        }
        unbalancedOutletMassFlowRate = state.dataHVACGlobal->UnbalExhMassFlow;
        balancedOutletMassFlowRate = state.dataHVACGlobal->BalancedExhMassFlow;
    }

    if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
        outletNode.CO2 = inletNode.CO2;
    }

    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
        outletNode.GenContam = inletNode.GenContam;
    }
}

void FanComponent::report(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   April 1998

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine updates the report variables for the fans.

    totalEnergy = totalPower * state.dataHVACGlobal->TimeStepSysSec;
    deltaTemp = outletAirTemp - inletAirTemp;

    if (isAFNFan && (airLoopNum > 0)) {
        if (type == HVAC::FanType::OnOff) {
            state.dataAirLoop->AirLoopAFNInfo(airLoopNum).AFNLoopOnOffFanRTF = runtimeFrac;
        }
    }
} // FanComponent::report()

int GetFanIndex(EnergyPlusData &state, std::string const &FanName)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   June 2004

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine sets an index for a given fan -- issues error message if that fan
    // is not legal fan.

    if (state.dataFans->GetFanInputFlag) { // First time subroutine has been entered
        GetFanInput(state);
        state.dataFans->GetFanInputFlag = false;
    }

    auto found = state.dataFans->fanMap.find(FanName);
    return (found == state.dataFans->fanMap.end()) ? 0 : found->second;
} // GetFanIndex()

Real64 CalFaultyFanAirFlowReduction(EnergyPlusData &state,
                                    std::string const &FanName,          // name of the fan
                                    Real64 const FanDesignAirFlowRate,   // Fan Design Volume Flow Rate [m3/sec]
                                    Real64 const FanDesignDeltaPress,    // Fan Design Delta Pressure [Pa]
                                    Real64 const FanFaultyDeltaPressInc, // Increase of Fan Delta Pressure in the Faulty Case [Pa]
                                    int const FanCurvePtr                // Fan Curve Index
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rongpeng Zhang
    //       DATE WRITTEN   Apr. 2015

    // PURPOSE OF THIS SUBROUTINE:
    // Calculate the decrease of the fan air flow rate, given the fan curve
    // and the increase of fan pressure rise due to fouling air filters

    // Check whether the fan curve covers the design operational point of the fan
    Real64 FanCalDeltaPress = Curve::CurveValue(state, FanCurvePtr, FanDesignAirFlowRate); // [Pa]
    if ((FanCalDeltaPress < 0.9 * FanDesignDeltaPress) || (FanCalDeltaPress > 1.1 * FanDesignDeltaPress)) {
        ShowWarningError(state, format("The design operational point of the fan {} does not fall ", FanName));
        ShowContinueError(state, "on the fan curve provided in the FaultModel:Fouling:AirFilter object. ");
        return 0.0;
    }

    // Calculate the Fan Volume Flow Rate in the Faulty Case
    Real64 FanFaultyAirFlowRate = FanDesignAirFlowRate;                                        // Fan Volume Flow Rate in the Faulty Case [m3/sec]
    Real64 FanCalDeltaPresstemp = Curve::CurveValue(state, FanCurvePtr, FanFaultyAirFlowRate); // Calculated Fan Delta Pressure for temp use [Pa]
    FanCalDeltaPress = FanCalDeltaPresstemp;

    while (FanCalDeltaPress < (FanDesignDeltaPress + FanFaultyDeltaPressInc)) {
        FanFaultyAirFlowRate = FanFaultyAirFlowRate - 0.005;
        FanCalDeltaPresstemp = Curve::CurveValue(state, FanCurvePtr, FanFaultyAirFlowRate);

        if ((FanCalDeltaPresstemp <= FanCalDeltaPress) || (FanFaultyAirFlowRate <= state.dataCurveManager->curves(FanCurvePtr)->inputLimits[0].min)) {
            // The new operational point of the fan go beyond the fan selection range
            ShowWarningError(state, format("The operational point of the fan {} may go beyond the fan selection ", FanName));
            ShowContinueError(state, "range in the faulty fouling air filter cases");
            break;
        }

        FanCalDeltaPress = FanCalDeltaPresstemp;
    }

    return FanDesignAirFlowRate - FanFaultyAirFlowRate;
}

Real64 FanComponent::getDesignHeatGain(EnergyPlusData &state,
                                       Real64 const _volFlow // fan volumetric flow rate [m3/s]
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   August 2014

    // PURPOSE OF THIS FUNCTION:
    // This function calculates and returns the design fan heat gain from the fan input data

    // METHODOLOGY EMPLOYED:
    // Simple fan:  Qdot,tot = (Vdot*deltaP)/Eff,tot
    //              Qdot,air = Eff,mot*Qdot,tot + (Qdot,tot - Eff,mot*Qdot,tot)*Frac,mot-in-airstream

    if (type != HVAC::FanType::ComponentModel) {
        Real64 _deltaP = deltaPress; // fan design pressure rise [N/m2]
        Real64 _totalEff = totalEff; // fan design total efficiency
        Real64 _motorEff = motorEff; // fan design motor efficiency
        Real64 _motorInAirFrac = motorInAirFrac;
        Real64 _powerTot = (_volFlow * _deltaP) / _totalEff;
        return _motorEff * _powerTot + (_powerTot - _motorEff * _powerTot) * _motorInAirFrac;
    } else {
        if (!state.dataGlobal->SysSizingCalc && sizingFlag) {
            set_size(state);
            sizingFlag = false;
        }
        return shaftPower + (motorInputPower - shaftPower) * motorInAirFrac;
    }
} // FanComponent::getDesignHeatGain()

void FanComponent::getInputsForDesignHeatGain(EnergyPlusData &state,
                                              Real64 &_deltaP,
                                              Real64 &_motEff,
                                              Real64 &_totEff,
                                              Real64 &_motInAirFrac,
                                              Real64 &_fanShaftPow,
                                              Real64 &_motInPower,
                                              bool &_fanCompModel)
{
    if (type != HVAC::FanType::ComponentModel) {
        _deltaP = deltaPress;
        _motEff = motorEff;
        _totEff = totalEff;
        _motInAirFrac = motorInAirFrac;
        _fanShaftPow = 0.0;
        _motInPower = 0.0;
        _fanCompModel = false;
    } else {
        _deltaP = 0.0;
        _motEff = 0.0;
        _totEff = 0.0;
        if (!state.dataGlobal->SysSizingCalc && sizingFlag) {
            set_size(state);
            sizingFlag = false;
        }
        _fanCompModel = true;
        _fanShaftPow = shaftPower;
        _motInPower = motorInputPower;
        _motInAirFrac = motorInAirFrac;
    }
} // FanComponent::getInputsForDesHeatGain()

void FanSystem::init(EnergyPlusData &state)
{
    if (!state.dataGlobal->SysSizingCalc && sizingFlag) {
        set_size(state);
        sizingFlag = false;
    }

    auto &outletNode = state.dataLoopNodes->Node(outletNodeNum);
    auto &inletNode = state.dataLoopNodes->Node(inletNodeNum);

    if (state.dataGlobal->BeginEnvrnFlag && envrnFlag) {

        // Currently, fan does not force minimum mass flow, only used for power calculation
        // m_minAirFlowRate = designAirVolFlowRate * m_minPowerFlowFrac;
        // m_minAirMassFlowRate = m_minAirFlowRate * m_rhoAirStdInit;

        // Init the Node Control variables
        outletNode.MassFlowRateMax = maxAirMassFlowRate;
        // Currently, fan does not force minimum mass flow, only used for power calculation
        // DataLoopNode::Node( outletNodeNum ).MassFlowRateMin = m_minAirMassFlowRate;

        // Initialize all report variables to a known state at beginning of simulation
        totalPower = 0.0;
        deltaTemp = 0.0;
        powerLossToAir = 0.0;
        totalEnergy = 0.0;
        for (int iSpeed = 0; iSpeed < numSpeeds; ++iSpeed) {
            runtimeFracAtSpeed[iSpeed] = 0.0;
        }
        envrnFlag = false;
    }

    if (!state.dataGlobal->BeginEnvrnFlag) {
        envrnFlag = true;
    }

    massFlowRateMaxAvail = min(outletNode.MassFlowRateMax, inletNode.MassFlowRateMaxAvail);
    massFlowRateMinAvail = min(max(outletNode.MassFlowRateMin, inletNode.MassFlowRateMinAvail), inletNode.MassFlowRateMaxAvail);

    // Load the node data in this section for the component simulation
    // First need to make sure that the MassFlowRate is between the max and min avail.
    inletAirMassFlowRate = min(inletNode.MassFlowRate, massFlowRateMaxAvail);
    inletAirMassFlowRate = max(inletAirMassFlowRate, massFlowRateMinAvail);

    // Then set the other conditions
    inletAirTemp = inletNode.Temp;
    inletAirHumRat = inletNode.HumRat;
    inletAirEnthalpy = inletNode.Enthalpy;
} // FanSystem::init()

void FanSystem::set_size(EnergyPlusData &state)
{
    static constexpr std::string_view routineName = "FanSystem::set_size";

    Real64 _tempFlow = maxAirFlowRate;
    bool _bPRINT = true;
    state.dataSize->DataAutosizable = true;
    state.dataSize->DataEMSOverrideON = EMSMaxAirFlowRateOverrideOn;
    state.dataSize->DataEMSOverride = EMSMaxAirFlowRateValue;
    airLoopNum = state.dataSize->CurSysNum;

    bool ErrorsFound = false;
    SystemAirFlowSizer sizerSystemAirFlow;
    sizerSystemAirFlow.initializeWithinEP(state, HVAC::fanTypeNames[(int)type], Name, _bPRINT, routineName);
    maxAirFlowRate = sizerSystemAirFlow.size(state, _tempFlow, ErrorsFound);

    state.dataSize->DataAutosizable = true; // should be false?
    state.dataSize->DataEMSOverrideON = false;
    state.dataSize->DataEMSOverride = 0.0;

    if (designElecPowerWasAutosized) {

        switch (powerSizingMethod) {
        case PowerSizing::PerFlow: {
            designElecPower = maxAirFlowRate * elecPowerPerFlowRate;
        } break;
        case PowerSizing::PerFlowPerPressure: {
            designElecPower = maxAirFlowRate * deltaPress * elecPowerPerFlowRatePerPressure;
        } break;
        case PowerSizing::TotalEfficiencyAndPressure: {
            designElecPower = maxAirFlowRate * deltaPress / totalEff;
        } break;
        case PowerSizing::Invalid: {
            // do nothing (no assert?)
            break;
        }
        default:
            assert(false);

        } // end switch

        // report design power
        BaseSizer::reportSizerOutput(state, HVAC::fanTypeNames[(int)type], Name, "Design Electric Power Consumption [W]", designElecPower);

    } // end if power was autosized

    rhoAirStdInit = state.dataEnvrn->StdRhoAir;
    maxAirMassFlowRate = maxAirFlowRate * rhoAirStdInit;

    // calculate total fan system efficiency at design, else set to 1 to avoid div by zero
    totalEff = (designElecPower > 0.0) ? maxAirFlowRate * deltaPress / designElecPower : 1.0;

    if (speedControl == SpeedControl::Discrete && numSpeeds > 1) { // set up values at speeds
        massFlowAtSpeed.resize(numSpeeds, 0.0);
        totalEffAtSpeed.resize(numSpeeds, 0.0);
        for (int loop = 0; loop < numSpeeds; ++loop) {
            massFlowAtSpeed[loop] = maxAirMassFlowRate * flowFracAtSpeed[loop];
            if (powerFracInputAtSpeed[loop]) { // use speed power fraction
                if (designElecPower > 0.0) {
                    totalEffAtSpeed[loop] = flowFracAtSpeed[loop] * maxAirFlowRate * deltaPress / (designElecPower * powerFracAtSpeed[loop]);
                } else {
                    totalEffAtSpeed[loop] = 1.0;
                }
            } else { // use power curve
                totalEffAtSpeed[loop] = flowFracAtSpeed[loop] * maxAirFlowRate * deltaPress /
                                        (designElecPower * Curve::CurveValue(state, powerModFuncFlowFracCurveNum, flowFracAtSpeed[loop]));
                powerFracAtSpeed[loop] = Curve::CurveValue(state, powerModFuncFlowFracCurveNum, flowFracAtSpeed[loop]);
            }
        }
    }
    designPointFEI = report_fei(state, maxAirFlowRate, designElecPower, deltaPress);

    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanType, Name, HVAC::fanTypeNames[(int)type]);
    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanTotEff, Name, totalEff);
    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanDeltaP, Name, deltaPress);
    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanVolFlow, Name, maxAirFlowRate);

    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanPwr, Name, designElecPower);
    if (maxAirFlowRate != 0.0) {
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanPwrPerFlow, Name, designElecPower / maxAirFlowRate);
    }
    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanMotorIn, Name, motorInAirFrac);
    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanEnergyIndex, Name, designPointFEI);

    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanEndUse, Name, endUseSubcategoryName);

    sizingFlag = false;

    // Std 229 Fans (HVACFan.cc)
    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanPurpose, Name, "N/A"); // m_fanType); // purpose? not the same
    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanAutosized, Name, maxAirFlowRateIsAutosized ? "Yes" : "No");
    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanMotorEff, Name, motorEff);
    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanMotorHeatToZoneFrac, Name, 1 - motorInAirFrac);
    OutputReportPredefined::PreDefTableEntry(state,
                                             state.dataOutRptPredefined->pdchFanMotorHeatZone,
                                             Name,
                                             heatLossDest == HeatLossDest::Zone ? state.dataHeatBal->Zone(zoneNum).Name : "N/A");
    if (airLoopNum == 0) {
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanAirLoopName, Name, "N/A");
    } else if (airLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys) {
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchFanAirLoopName, Name, state.dataAirSystemsData->PrimaryAirSystems(airLoopNum).Name);
    } else {
        OutputReportPredefined::PreDefTableEntry(
            state,
            state.dataOutRptPredefined->pdchFanAirLoopName,
            Name,
            state.dataAirLoopHVACDOAS->airloopDOAS[airLoopNum - state.dataHVACGlobal->NumPrimaryAirSys - 1].Name);
    }
}

Real64 FanSystem::report_fei(EnergyPlusData &state, Real64 const _designFlowRate, Real64 const _designElecPower, Real64 const _designDeltaPress)
{
    // PURPOSE OF THIS SUBROUTINE:
    // Calculate the Fan Energy Index

    // REFERENCES:
    // ANSI/AMCA Standard 207-17: Fan System Efficiency and Fan System Input Power Calculation, 2017.
    // AANSI / AMCA Standard 208 - 18: Calculation of the Fan Energy Index, 2018.

    Real64 constexpr rhoAirStd = 1.2;   // Value from the above referenced standard
    Real64 constexpr tempAirFan = 21.0; // Standard fan inlet temperature in Celsius
    Real64 constexpr hrAirFan = 0.5;    // Standard fan inlet humidity ratio (50%)
    Real64 _wAirFan = Psychrometrics::PsyWFnTdbRhPb(state, tempAirFan, hrAirFan, state.dataEnvrn->StdBaroPress);
    Real64 _rhoAirFan = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->StdBaroPress, tempAirFan, _wAirFan);

    // Calculate reference fan shaft power
    Real64 _refFanShaftPower = (_designFlowRate + 0.118) * (_designDeltaPress + 100 * _rhoAirFan / rhoAirStd) / (1000 * 0.66);

    // Calculate reference reference fan transmission efficiency
    Real64 _refFanTransEff = 0.96 * pow((_refFanShaftPower / (_refFanShaftPower + 1.64)), 0.05);

    // Calculate reference reference fan motor efficiency
    Real64 _refFanMotorOutput = _refFanShaftPower / _refFanTransEff;

    Real64 _refFanMotorEff;
    if (_refFanMotorOutput < 185.0) {
        _refFanMotorEff = -0.003812 * pow(std::log10(_refFanMotorOutput), 4) + 0.025834 * pow(std::log10(_refFanMotorOutput), 3) -
                          0.072577 * pow(std::log10(_refFanMotorOutput), 2) + 0.125559 * std::log10(_refFanMotorOutput) + 0.850274;
    } else {
        _refFanMotorEff = 0.962;
    }

    // Calculate reference reference fan motor controller  efficiency
    Real64 _refFanMotorCtrlEff = 1;

    Real64 _refFanElecPower = _refFanShaftPower / (_refFanTransEff * _refFanMotorEff * _refFanMotorCtrlEff);

    return (_designElecPower > 0.0) ? (_refFanElecPower * 1000 / _designElecPower) : 0.0;
} // FanSystem::report_fei()

void FanSystem::calcSimpleSystemFan(
    EnergyPlusData &state,
    ObjexxFCL::Optional<Real64 const> _flowFraction, // Flow fraction for entire timestep (not used if flow ratios are present)
    ObjexxFCL::Optional<Real64 const> _pressureRise, // Pressure difference to use for DeltaPress
    ObjexxFCL::Optional<Real64 const> _flowRatio1,   // Flow ratio in operating mode 1
    ObjexxFCL::Optional<Real64 const> _runTimeFrac1, // Run time fraction in operating mode 1
    ObjexxFCL::Optional<Real64 const> _flowRatio2,   // Flow ratio in operating mode 2
    ObjexxFCL::Optional<Real64 const> _runTimeFrac2, // Run time fraction in operating mode 2
    ObjexxFCL::Optional<Real64 const> _pressureRise2 // Pressure difference to use for operating mode 2
)
{
    Real64 _localFlowFrac;
    Real64 _localTotalEff;
    std::array<Real64, 2> _localPressureRise = {0.0, 0.0}; // [0] is operating mode 1, [1] is operating mode 2
    std::array<Real64, 2> _localAirMassFlow = {0.0, 0.0};
    std::array<Real64, 2> _localFlowRatio = {0.0, 0.0};
    std::array<Real64, 2> _localRuntimeFrac = {1.0, 1.0};
    bool _useFlowRatiosAndRunTimeFracs = false;

    // Number of operating modes, 1 or 2 ( e.g. heating, ventilating, cooling)
    int _numModes = (present(_flowRatio2) && present(_runTimeFrac2)) ? 2 : 1;

    if (state.dataHVACGlobal->NightVentOn) {
        // assume if non-zero inputs for night data then this fan is to be used with that data
        if (nightVentPressureDelta > 0.0) {
            _localPressureRise[0] = nightVentPressureDelta;
            _localPressureRise[1] = nightVentPressureDelta;
        }

        _localFlowFrac = (maxAirMassFlowRate > 0.0) ? inletAirMassFlowRate / maxAirMassFlowRate : 1.0;

        _localAirMassFlow[0] = inletAirMassFlowRate;

    } else { // not in night mode
        _localPressureRise[0] = present(_pressureRise) ? _pressureRise() : deltaPress;

        _localPressureRise[1] = present(_pressureRise2) ? _pressureRise2() : deltaPress;

        if (present(_flowFraction)) {
            _localFlowFrac = _flowFraction;
            _localAirMassFlow[0] = _localFlowFrac * maxAirMassFlowRate;
        } else {
            if (maxAirMassFlowRate > 0.0) { // protect div by 0
                _localFlowFrac = inletAirMassFlowRate / maxAirMassFlowRate;
            } else {
                _localFlowFrac = 1.0;
            }
            _localAirMassFlow[0] = inletAirMassFlowRate;
        }
        if (present(_flowRatio1) && present(_flowRatio2) && present(_runTimeFrac1) && present(_runTimeFrac2)) {
            _useFlowRatiosAndRunTimeFracs = true;
            _localRuntimeFrac[0] = _runTimeFrac1;
            _localRuntimeFrac[1] = _runTimeFrac2;
            _localFlowRatio[0] = _flowRatio1;
            _localAirMassFlow[0] = _localFlowRatio[0] * maxAirMassFlowRate * _localRuntimeFrac[0];
            _localFlowRatio[1] = _flowRatio2;
            _localAirMassFlow[1] = _localFlowRatio[1] * maxAirMassFlowRate * _localRuntimeFrac[1];
        } else {
            _localRuntimeFrac[0] = 1.0; // if runTimeFracs are not present, assume single-mode operation
            _localRuntimeFrac[1] = 0.0; // if runTimeFracs are not present, assume single-mode operation
        }
    }

    Real64 _localFaultMaxAirMassFlow = 0.0;
    bool _faultActive = false;
    Real64 _localFaultPressureRise = 0.0;
    if (faultyFilterFlag && (state.dataFaultsMgr->NumFaultyAirFilter > 0) && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) &&
        state.dataGlobal->DoWeathSim && (!EMSMaxMassFlowOverrideOn) && (!EMSPressureOverrideOn)) {
        auto &fault = state.dataFaultsMgr->FaultsFouledAirFilters(faultyFilterIndex);
        if (fault.availSched->getCurrentVal() > 0) {
            _faultActive = true;
            Real64 _pressFrac = fault.pressFracSched->getCurrentVal();
            Real64 _designFlowRateDec = // Decrease of the Fan Design Volume Flow Rate [m3/sec]
                Fans::CalFaultyFanAirFlowReduction(state, Name, maxAirFlowRate, deltaPress, (_pressFrac - 1) * deltaPress, fault.fanCurveNum);

            _localFaultMaxAirMassFlow = maxAirMassFlowRate - _designFlowRateDec * rhoAirStdInit;
            _localFaultPressureRise = _pressFrac * deltaPress;
        }
    }

    for (int mode = 0; mode < _numModes; ++mode) {
        // EMS override MassFlow, DeltaPress, and FanEff
        if (EMSPressureOverrideOn) {
            _localPressureRise[mode] = EMSPressureValue;
        }
        if (EMSTotalEffOverrideOn) {
            _localTotalEff = EMSTotalEffValue;
        }
        if (EMSMaxMassFlowOverrideOn) {
            _localAirMassFlow[mode] = EMSAirMassFlowValue;
        }

        _localAirMassFlow[mode] = min(_localAirMassFlow[mode], maxAirMassFlowRate);
        if (_faultActive) {
            _localAirMassFlow[mode] = min(_localAirMassFlow[mode], _localFaultMaxAirMassFlow);
            _localPressureRise[mode] = _localFaultPressureRise;
        }
        _localFlowFrac = _localAirMassFlow[0] / maxAirMassFlowRate;
        _localFlowFrac = min(1.0, _localFlowFrac);

        if (_localRuntimeFrac[mode] > 0.0) {
            _localFlowRatio[mode] = _localAirMassFlow[mode] / (maxAirMassFlowRate * _localRuntimeFrac[mode]);
        }
        _localFlowRatio[mode] = min(1.0, _localFlowRatio[mode]);
    }

    // zero these now, because the may accumulate across multiple operating modes
    powerLossToAir = 0.0;
    totalPower = 0.0;
    outletAirMassFlowRate = 0.0;
    if (speedControl == SpeedControl::Discrete) {
        for (int loop = 0; loop < numSpeeds; ++loop) {
            runtimeFracAtSpeed[loop] = 0.0;
        }
    }

    if ((availSched->getCurrentVal() > 0.0 || state.dataHVACGlobal->TurnFansOn) && !state.dataHVACGlobal->TurnFansOff &&
        ((_localAirMassFlow[0] + _localAirMassFlow[1]) > 0.0)) {
        // fan is running

        for (int mode = 0; mode < _numModes; ++mode) {

            // if no flow for this mode then continue to the next mode
            if (_localAirMassFlow[mode] == 0.0) {
                continue;
            }

            switch (speedControl) {

            case SpeedControl::Discrete: {
                //
                if (state.dataHVACGlobal->OnOffFanPartLoadFraction <= 0.0) {
                    state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
                }
                if (state.dataHVACGlobal->OnOffFanPartLoadFraction < 0.7) {
                    state.dataHVACGlobal->OnOffFanPartLoadFraction = 0.7; // a warning message is already issued from the DX coils or gas heating coil
                }
                if (_useFlowRatiosAndRunTimeFracs) {
                    // Use flow ratios and runtimefractions pass from parent (allows fan to cycle at a specified speed)
                    Real64 _locRuntimeFrac = (state.dataHVACGlobal->OnOffFanPartLoadFraction >= 1.0)
                                                 ? _localRuntimeFrac[mode]
                                                 : max(0.0, min(1.0, _localRuntimeFrac[mode] / state.dataHVACGlobal->OnOffFanPartLoadFraction));
                    Real64 _locFlowRatio = _localFlowRatio[mode]; // Current mode flow rate / max flow rate
                    Real64 _locLowSpeedRuntimeFrac = 0.0;
                    Real64 _locHiSpeedRuntimeFrac = 0.0;
                    if (numSpeeds == 1) { // CV or OnOff
                        _localTotalEff = totalEff;
                        _locHiSpeedRuntimeFrac = _locRuntimeFrac * _locFlowRatio;
                        runtimeFracAtSpeed[0] += _locHiSpeedRuntimeFrac;
                        totalPower +=
                            max(0.0, _locHiSpeedRuntimeFrac * maxAirMassFlowRate * _localPressureRise[mode] / (_localTotalEff * rhoAirStdInit));
                    } else if (numSpeeds > 1) { // multi speed

                        // find which two speed levels bracket flow ratios and calculate runtimefraction at each speed
                        // ideally the flow ratios passed in will match one of the fan m_flowFractionAtSpeed but it is not required
                        int _lowSideSpeed = -1;
                        int _hiSideSpeed = -1;

                        if (_locFlowRatio <= flowFracAtSpeed[0]) { // on/off at lowest speed
                            _hiSideSpeed = 0;
                            _locHiSpeedRuntimeFrac = _locFlowRatio * _locRuntimeFrac / flowFracAtSpeed[0];
                            runtimeFracAtSpeed[0] += _locHiSpeedRuntimeFrac;
                        } else {
                            _lowSideSpeed = 0; // hush up cppcheck
                            _hiSideSpeed = 0;  // hush up cppcheck
                            for (int loop = 0; loop < numSpeeds - 1; ++loop) {
                                if ((flowFracAtSpeed[loop] <= _locFlowRatio) && (_locFlowRatio <= flowFracAtSpeed[loop + 1])) {
                                    _lowSideSpeed = loop;
                                    _hiSideSpeed = loop + 1;
                                    break;
                                }
                            }
                            Real64 _locLowSpeedTimeFrac =
                                (flowFracAtSpeed[_hiSideSpeed] - _locFlowRatio) / (flowFracAtSpeed[_hiSideSpeed] - flowFracAtSpeed[_lowSideSpeed]);
                            _locLowSpeedRuntimeFrac = _locLowSpeedTimeFrac * _localRuntimeFrac[mode];
                            _locHiSpeedRuntimeFrac = (1 - _locLowSpeedTimeFrac) * _localRuntimeFrac[mode];
                            runtimeFracAtSpeed[_lowSideSpeed] += _locLowSpeedRuntimeFrac;
                            runtimeFracAtSpeed[_hiSideSpeed] += _locHiSpeedRuntimeFrac;
                        }
                        if (_lowSideSpeed != -1 && _hiSideSpeed != -1) {
                            totalPower += max(0.0,
                                              _locLowSpeedRuntimeFrac * massFlowAtSpeed[_lowSideSpeed] * _localPressureRise[mode] /
                                                      (totalEffAtSpeed[_lowSideSpeed] * rhoAirStdInit) +
                                                  _locHiSpeedRuntimeFrac * massFlowAtSpeed[_hiSideSpeed] * _localPressureRise[mode] /
                                                      (totalEffAtSpeed[_hiSideSpeed] * rhoAirStdInit));
                        } else if (_lowSideSpeed == -1 && _hiSideSpeed == 0) {
                            totalPower += max(0.0,
                                              _locHiSpeedRuntimeFrac * massFlowAtSpeed[_hiSideSpeed] * _localPressureRise[mode] /
                                                  (totalEffAtSpeed[_hiSideSpeed] * rhoAirStdInit));
                        }
                    }
                } else {
                    // Use localFlowFraction which is not locked at a particular flow ratio (legacy method for fan:onoff)
                    Real64 _locLowSpeedRuntimeFrac = 0.0;
                    Real64 _locHiSpeedRuntimeFrac = 0.0;
                    Real64 _locRuntimeFrac = (state.dataHVACGlobal->OnOffFanPartLoadFraction >= 1.0)
                                                 ? _localFlowFrac
                                                 : max(0.0, min(1.0, _localFlowFrac / state.dataHVACGlobal->OnOffFanPartLoadFraction));

                    if (numSpeeds == 1) { // CV or OnOff
                        _localTotalEff = totalEff;
                        _locHiSpeedRuntimeFrac = _locRuntimeFrac;
                        runtimeFracAtSpeed[0] += _locHiSpeedRuntimeFrac;
                        totalPower +=
                            max(0.0, _locHiSpeedRuntimeFrac * maxAirMassFlowRate * _localPressureRise[mode] / (_localTotalEff * rhoAirStdInit));
                    } else if (numSpeeds > 1) { // multi speed

                        // find which two speed levels bracket flow fraction and calculate runtimefraction
                        int _lowSideSpeed = -1;
                        int _hiSideSpeed = -1;

                        if (_locRuntimeFrac < flowFracAtSpeed[0]) { // on/off between zero and lowest speed
                            _hiSideSpeed = 0;
                            _locHiSpeedRuntimeFrac = _locRuntimeFrac / flowFracAtSpeed[0];
                            runtimeFracAtSpeed[0] += _locHiSpeedRuntimeFrac;
                        } else {
                            _lowSideSpeed = 0; // hush up cppcheck
                            _hiSideSpeed = 0;  // hush up cppcheck
                            for (int loop = 0; loop < numSpeeds - 1; ++loop) {
                                if ((flowFracAtSpeed[loop] <= _locRuntimeFrac) && (_locRuntimeFrac <= flowFracAtSpeed[loop + 1])) {
                                    _lowSideSpeed = loop;
                                    _hiSideSpeed = loop + 1;
                                    break;
                                }
                            }
                            _locLowSpeedRuntimeFrac =
                                (flowFracAtSpeed[_hiSideSpeed] - _locRuntimeFrac) / (flowFracAtSpeed[_hiSideSpeed] - flowFracAtSpeed[_lowSideSpeed]);
                            _locHiSpeedRuntimeFrac =
                                (_locRuntimeFrac - flowFracAtSpeed[_lowSideSpeed]) / (flowFracAtSpeed[_hiSideSpeed] - flowFracAtSpeed[_lowSideSpeed]);
                            runtimeFracAtSpeed[_lowSideSpeed] += _locLowSpeedRuntimeFrac;
                            runtimeFracAtSpeed[_hiSideSpeed] += _locHiSpeedRuntimeFrac;
                        }
                        if (_lowSideSpeed != -1 && _hiSideSpeed != -1) {
                            totalPower += max(0.0,
                                              _locLowSpeedRuntimeFrac * massFlowAtSpeed[_lowSideSpeed] * _localPressureRise[mode] /
                                                      (totalEffAtSpeed[_lowSideSpeed] * rhoAirStdInit) +
                                                  _locHiSpeedRuntimeFrac * massFlowAtSpeed[_hiSideSpeed] * _localPressureRise[mode] /
                                                      (totalEffAtSpeed[_hiSideSpeed] * rhoAirStdInit));
                        } else if (_lowSideSpeed == -1 && _hiSideSpeed == 0) {
                            totalPower += max(0.0,
                                              _locHiSpeedRuntimeFrac * massFlowAtSpeed[_hiSideSpeed] * _localPressureRise[mode] /
                                                  (totalEffAtSpeed[_hiSideSpeed] * rhoAirStdInit));
                        }
                    }
                }
                _localTotalEff = totalEff;
            } break;

            case SpeedControl::Continuous: {
                _localTotalEff = totalEff;
                Real64 _locFlowRatio(0.0);
                Real64 _locRuntimeFrac(0.0);
                if (_useFlowRatiosAndRunTimeFracs) {
                    _locFlowRatio = _localFlowRatio[mode];
                    _locRuntimeFrac = _localRuntimeFrac[mode];
                } else {
                    _locFlowRatio = _localFlowFrac;
                    _locRuntimeFrac = 1.0;
                }

                Real64 _localFlowFracForPower = max(minPowerFlowFrac, _locFlowRatio);
                Real64 _localPowerFrac = (state.dataHVACGlobal->NightVentOn) ? 1.0 : // not sure why, but legacy fan had this for night ventilation
                                             Curve::CurveValue(state, powerModFuncFlowFracCurveNum, _localFlowFracForPower);
                Real64 _localFanPower =
                    max(0.0, _locRuntimeFrac * _localPowerFrac * maxAirMassFlowRate * _localPressureRise[mode] / (_localTotalEff * rhoAirStdInit));
                Real64 _shaftPower = motorEff * _localFanPower;
                Real64 _localPowerLossToAir = _shaftPower + (_localFanPower - _shaftPower) * motorInAirFrac;
                outletAirEnthalpy = inletAirEnthalpy + _localPowerLossToAir / _localAirMassFlow[mode]; // this will get revised later
                outletAirHumRat = inletAirHumRat;                                                      // this will get revised later
                outletAirTemp = Psychrometrics::PsyTdbFnHW(outletAirEnthalpy, outletAirHumRat);        // this will get revised later
                // When fan air flow is less than 10%, the fan power curve is linearized between the 10% to 0% to
                //  avoid the unrealistic high temperature rise across the fan.
                Real64 _deltaTAcrossFan = outletAirTemp - inletAirTemp;
                if (_deltaTAcrossFan > 20.0) {
                    Real64 _minFlowFracLimitFanHeat = 0.10;
                    Real64 _powerFracAtLowMin = 0.0;
                    Real64 _fanPowerAtLowMinimum = 0.0;
                    if (_localFlowFracForPower < _minFlowFracLimitFanHeat) {
                        _powerFracAtLowMin = Curve::CurveValue(state, powerModFuncFlowFracCurveNum, _minFlowFracLimitFanHeat);
                        _fanPowerAtLowMinimum = _powerFracAtLowMin * maxAirMassFlowRate * _localPressureRise[mode] / (_localTotalEff * rhoAirStdInit);
                        _localFanPower = max(0.0, _localFlowFracForPower * _fanPowerAtLowMinimum / _minFlowFracLimitFanHeat);
                    } else if (_locFlowRatio < _minFlowFracLimitFanHeat) {
                        _powerFracAtLowMin = Curve::CurveValue(state, powerModFuncFlowFracCurveNum, _minFlowFracLimitFanHeat);
                        _fanPowerAtLowMinimum = _powerFracAtLowMin * maxAirMassFlowRate * _localPressureRise[mode] / (_localTotalEff * rhoAirStdInit);
                        _localFanPower = max(0.0, _locFlowRatio * _fanPowerAtLowMinimum / _minFlowFracLimitFanHeat);
                    }
                }
                totalPower += _localFanPower;
            } break;
            // continuous speed control case
            case SpeedControl::Invalid: {
                // do nothing
            } break;
            default:
                assert(false);
            } // end switch
            outletAirMassFlowRate += _localAirMassFlow[mode];

        } // end of operating mode loop

        if (outletAirMassFlowRate > 0.0) {
            Real64 _shaftPower = motorEff * totalPower; // power delivered to shaft
            powerLossToAir = _shaftPower + (totalPower - _shaftPower) * motorInAirFrac;
            outletAirEnthalpy = inletAirEnthalpy + powerLossToAir / outletAirMassFlowRate;
            // This fan does not change the moisture or Mass Flow across the component
            outletAirHumRat = inletAirHumRat;
            outletAirTemp = Psychrometrics::PsyTdbFnHW(outletAirEnthalpy, outletAirHumRat);
        } else {
            totalPower = 0.0;
            powerLossToAir = 0.0;
            outletAirHumRat = inletAirHumRat;
            outletAirEnthalpy = inletAirEnthalpy;
            outletAirTemp = inletAirTemp;
            massFlowRateMaxAvail = 0.0;
            massFlowRateMinAvail = 0.0;
        }
    } else { // fan is off
        // Fan is off and not operating no power consumed and mass flow rate.
        totalPower = 0.0;
        powerLossToAir = 0.0;
        outletAirHumRat = inletAirHumRat;
        outletAirEnthalpy = inletAirEnthalpy;
        outletAirTemp = inletAirTemp;
        // Set the Control Flow variables to 0.0 flow when OFF.
        if (isSecondaryDriver) {
            // sometimes the air is moving with the fan off, eg. AirTerminal:SingleDuct:VAV:Reheat:VariableSpeedFan
            outletAirMassFlowRate = _localAirMassFlow[0] + _localAirMassFlow[1];
            if (outletAirMassFlowRate == 0.0) {
                massFlowRateMaxAvail = 0.0;
                massFlowRateMinAvail = 0.0;
            }
        } else {
            outletAirMassFlowRate = 0.0;
            massFlowRateMaxAvail = 0.0;
            massFlowRateMinAvail = 0.0;
        }
    }

    if (heatLossDest == HeatLossDest::Zone) {
        Real64 _powerLossToZone = totalPower - powerLossToAir;
        qdotConvZone = _powerLossToZone * (1.0 - zoneRadFract);
        qdotRadZone = _powerLossToZone * zoneRadFract;
    }
    state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0; // reset to 1
} // FanSystem::report()

void FanSystem::update(EnergyPlusData &state) // does not change state of object, only update elsewhere
{
    // Set the outlet air node of the fan
    state.dataLoopNodes->Node(outletNodeNum).MassFlowRate = outletAirMassFlowRate;
    state.dataLoopNodes->Node(outletNodeNum).Temp = outletAirTemp;
    state.dataLoopNodes->Node(outletNodeNum).HumRat = outletAirHumRat;
    state.dataLoopNodes->Node(outletNodeNum).Enthalpy = outletAirEnthalpy;
    // Set the outlet nodes for properties that just pass through & not used
    state.dataLoopNodes->Node(outletNodeNum).Quality = state.dataLoopNodes->Node(inletNodeNum).Quality;
    state.dataLoopNodes->Node(outletNodeNum).Press = state.dataLoopNodes->Node(inletNodeNum).Press;

    // Set the Node Flow Control Variables from the Fan Control Variables
    state.dataLoopNodes->Node(outletNodeNum).MassFlowRateMaxAvail = massFlowRateMaxAvail;
    state.dataLoopNodes->Node(outletNodeNum).MassFlowRateMinAvail = massFlowRateMinAvail;

    // make sure inlet has the same mass flow
    state.dataLoopNodes->Node(inletNodeNum).MassFlowRate = outletAirMassFlowRate;

    if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
        state.dataLoopNodes->Node(outletNodeNum).CO2 = state.dataLoopNodes->Node(inletNodeNum).CO2;
    }
    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
        state.dataLoopNodes->Node(outletNodeNum).GenContam = state.dataLoopNodes->Node(inletNodeNum).GenContam;
    }

    if (isAFNFan && (airLoopNum > 0)) {
        if (speedControl == SpeedControl::Continuous) {
            state.dataAirLoop->AirLoopAFNInfo(airLoopNum).AFNLoopOnOffFanRTF = runtimeFracAtSpeed[0];
        } else {
            if (numSpeeds == 1) {
                state.dataAirLoop->AirLoopAFNInfo(airLoopNum).AFNLoopOnOffFanRTF = outletAirMassFlowRate / maxAirMassFlowRate;
            } else if (outletAirMassFlowRate <= massFlowAtSpeed[0]) {
                state.dataAirLoop->AirLoopAFNInfo(airLoopNum).AFNLoopOnOffFanRTF = outletAirMassFlowRate / massFlowAtSpeed[0];
            } else {
                state.dataAirLoop->AirLoopAFNInfo(airLoopNum).AFNLoopOnOffFanRTF = 1.0;
            }
        }
    }
} // FanSystem::update()

void FanSystem::report(EnergyPlusData &state)
{
    totalEnergy = totalPower * state.dataHVACGlobal->TimeStepSysSec;
    deltaTemp = outletAirTemp - inletAirTemp;
}

Real64 FanSystem::getDesignTemperatureRise(EnergyPlusData &state) const
{
    if (!sizingFlag) {
        Real64 _cpAir = Psychrometrics::PsyCpAirFnW(DataPrecisionGlobals::constant_zero);
        return (deltaPress / (rhoAirStdInit * _cpAir * totalEff)) * (motorEff + motorInAirFrac * (1.0 - motorEff));
    } else {
        // TODO throw warning, exception, call sizing?
        ShowWarningError(state, "FanSystem::getDesignTemperatureRise called before fan sizing completed ");
        return 0.0;
    }
}

Real64 FanSystem::getDesignHeatGain(EnergyPlusData &state, Real64 const _volFlow // fan volume flow rate [m3/s]
)
{
    if (sizingFlag) {
        set_size(state);
    }

    Real64 _fanPowerTot = (_volFlow * deltaPress) / totalEff;
    return motorEff * _fanPowerTot + (_fanPowerTot - motorEff * _fanPowerTot) * motorInAirFrac;
}

void FanSystem::getInputsForDesignHeatGain(EnergyPlusData &state,
                                           Real64 &_deltaP,
                                           Real64 &_motEff,
                                           Real64 &_totEff,
                                           Real64 &_motInAirFrac,
                                           Real64 &_shaftPower,
                                           Real64 &_motInPower,
                                           bool &_fanComponentModel)
{
    if (sizingFlag) {
        set_size(state);
    }

    _deltaP = deltaPress;
    _motEff = motorEff;
    _totEff = totalEff;
    _motInAirFrac = motorInAirFrac;

    _shaftPower = 0.0;
    _motInPower = 0.0;
    _fanComponentModel = false;
}
} // namespace EnergyPlus::Fans
