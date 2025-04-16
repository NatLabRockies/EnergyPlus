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
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <AirflowNetwork/Elements.hpp>
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HeatBalanceInternalHeatGains.hh>
#include <EnergyPlus/HybridModel.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneContaminantPredictorCorrector.hh>
#include <EnergyPlus/ZonePlenum.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus::ZoneContaminantPredictorCorrector {

// MODULE INFORMATION:
//       AUTHOR         Lixing Gu
//       DATE WRITTEN   May, 2010

// PURPOSE OF THIS MODULE:
// This module contains routines to predict and correct zone contaminants.
//  also includes zone contaminant controlling

// METHODOLOGY EMPLOYED:
// Similar approach to ZoneTempPredictorCorrector

// Using/Aliasing
using namespace DataHeatBalance;
using namespace Psychrometrics;
using namespace HybridModel;

void ManageZoneContaminanUpdates(EnergyPlusData &state,
                                 DataHeatBalFanSys::PredictorCorrectorCtrl const UpdateType, // Can be iGetZoneSetPoints, iPredictStep, iCorrectStep
                                 bool const ShortenTimeStepSys,
                                 bool const UseZoneTimeStepHistory, // if true then use zone timestep history, if false use system time step
                                 Real64 const PriorTimeStep         // the old value for timestep length is passed for possible use in interpolating
)
{

    // SUBROUTINE INFORMATION
    //       AUTHOR         Lixing Gu
    //       DATE WRITTEN   July, 2010

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine predicts or corrects the zone air temperature
    // depending on the simulation status and determines the correct
    // temperature setpoint for each zone from the schedule manager.
    // This module is revised from subroutine ManageZoneAirUpdates in
    // ZoneTempPredictorCorrector module.

    if (state.dataZoneContaminantPredictorCorrector->GetZoneAirContamInputFlag) {
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) GetZoneContaminanInputs(state);
        GetZoneContaminanSetPoints(state);
        state.dataZoneContaminantPredictorCorrector->GetZoneAirContamInputFlag = false;
    }

    if (!state.dataContaminantBalance->Contaminant.SimulateContaminants) return;

    switch (UpdateType) {
    case DataHeatBalFanSys::PredictorCorrectorCtrl::GetZoneSetPoints: {
        InitZoneContSetPoints(state);
    } break;
    case DataHeatBalFanSys::PredictorCorrectorCtrl::PredictStep: {
        PredictZoneContaminants(state, ShortenTimeStepSys, UseZoneTimeStepHistory, PriorTimeStep);
    } break;
    case DataHeatBalFanSys::PredictorCorrectorCtrl::CorrectStep: {
        CorrectZoneContaminants(state, UseZoneTimeStepHistory);
    } break;
    case DataHeatBalFanSys::PredictorCorrectorCtrl::RevertZoneTimestepHistories: {
        RevertZoneTimestepHistories(state);
    } break;
    case DataHeatBalFanSys::PredictorCorrectorCtrl::PushZoneTimestepHistories: {
        PushZoneTimestepHistories(state);
    } break;
    case DataHeatBalFanSys::PredictorCorrectorCtrl::PushSystemTimestepHistories: {
        PushSystemTimestepHistories(state);
    } break;
    default:
        break;
    }
}

void GetZoneContaminanInputs(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Lixing Gu
    //       DATE WRITTEN   Dec. 2011

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine gets the inputs related to generic contaminant internal gain.

    // METHODOLOGY EMPLOYED:
    // Uses the status flags to trigger events.

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("GetSourcesAndSinks: ");
    static constexpr std::string_view routineName = "GetSourcesAndSinks";

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Array1D_string AlphaName;
    Array1D<Real64> IHGNumbers;
    int IOStat;
    int Loop;
    int ZonePtr;
    bool ErrorsFound(false);
    Array1D_bool RepVarSet;
    std::string CurrentModuleObject;

    RepVarSet.dimension(state.dataGlobal->NumOfZones, true);

    int NumAlpha = 0;
    int NumNumber = 0;
    int MaxAlpha = -100;
    int MaxNumber = -100;
    CurrentModuleObject = "ZoneContaminantSourceAndSink:Generic:Constant";
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, Loop, NumAlpha, NumNumber);
    MaxAlpha = max(MaxAlpha, NumAlpha);
    MaxNumber = max(MaxNumber, NumNumber);
    CurrentModuleObject = "SurfaceContaminantSourceAndSink:Generic:PressureDriven";
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, Loop, NumAlpha, NumNumber);
    MaxAlpha = max(MaxAlpha, NumAlpha);
    MaxNumber = max(MaxNumber, NumNumber);
    CurrentModuleObject = "ZoneContaminantSourceAndSink:Generic:CutoffModel";
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, Loop, NumAlpha, NumNumber);
    MaxAlpha = max(MaxAlpha, NumAlpha);
    MaxNumber = max(MaxNumber, NumNumber);
    CurrentModuleObject = "ZoneContaminantSourceAndSink:Generic:DecaySource";
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, Loop, NumAlpha, NumNumber);
    MaxAlpha = max(MaxAlpha, NumAlpha);
    MaxNumber = max(MaxNumber, NumNumber);
    CurrentModuleObject = "SurfaceContaminantSourceAndSink:Generic:BoundaryLayerDiffusion";
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, Loop, NumAlpha, NumNumber);
    MaxAlpha = max(MaxAlpha, NumAlpha);
    MaxNumber = max(MaxNumber, NumNumber);
    CurrentModuleObject = "SurfaceContaminantSourceAndSink:Generic:DepositionVelocitySink";
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, Loop, NumAlpha, NumNumber);
    MaxAlpha = max(MaxAlpha, NumAlpha);
    MaxNumber = max(MaxNumber, NumNumber);
    CurrentModuleObject = "ZoneContaminantSourceAndSink:Generic:DepositionRateSink";
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, Loop, NumAlpha, NumNumber);
    MaxAlpha = max(MaxAlpha, NumAlpha);
    MaxNumber = max(MaxNumber, NumNumber);
    IHGNumbers.allocate(MaxNumber);
    AlphaName.allocate(MaxAlpha);
    IHGNumbers = 0.0;
    AlphaName = "";

    CurrentModuleObject = "ZoneContaminantSourceAndSink:Generic:Constant";
    int TotGCGenConstant = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
    state.dataContaminantBalance->ZoneContamGenericConstant.allocate(TotGCGenConstant);

    for (Loop = 1; Loop <= TotGCGenConstant; ++Loop) {
        AlphaName = "";
        IHGNumbers = 0.0;
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 Loop,
                                                                 AlphaName,
                                                                 NumAlpha,
                                                                 IHGNumbers,
                                                                 NumNumber,
                                                                 IOStat,
                                                                 state.dataIPShortCut->lNumericFieldBlanks,
                                                                 state.dataIPShortCut->lAlphaFieldBlanks,
                                                                 state.dataIPShortCut->cAlphaFieldNames,
                                                                 state.dataIPShortCut->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, CurrentModuleObject, AlphaName(1)};

        Util::IsNameEmpty(state, AlphaName(1), CurrentModuleObject, ErrorsFound);

        auto &contam = state.dataContaminantBalance->ZoneContamGenericConstant(Loop);
        contam.Name = AlphaName(1);
        contam.ZoneName = AlphaName(2);
        contam.ActualZoneNum = Util::FindItemInList(AlphaName(2), state.dataHeatBal->Zone);
        if (contam.ActualZoneNum == 0) {
            ShowSevereError(state,
                            format("{}{}=\"{}\", invalid {} entered={}",
                                   RoutineName,
                                   CurrentModuleObject,
                                   AlphaName(1),
                                   state.dataIPShortCut->cAlphaFieldNames(2),
                                   AlphaName(2)));
            ErrorsFound = true;
        }

        if (state.dataIPShortCut->lAlphaFieldBlanks(3)) {
            ShowSevereEmptyField(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3));
            ErrorsFound = true;
        } else if ((contam.generateRateSched = Sched::GetSchedule(state, AlphaName(3))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3), AlphaName(3));
            ErrorsFound = true;
        } else if (!contam.generateRateSched->checkMinVal(state, Clusive::In, 0.0)) {
            Sched::ShowSevereBadMin(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3), AlphaName(3), Clusive::In, 0.0);
            ErrorsFound = true;
        }

        contam.GenerateRate = IHGNumbers(1);
        contam.RemovalCoef = IHGNumbers(2);

        if (state.dataIPShortCut->lAlphaFieldBlanks(4)) {
            ShowSevereEmptyField(state, eoh, state.dataIPShortCut->cAlphaFieldNames(4));
            ErrorsFound = true;
        } else if ((contam.removalCoefSched = Sched::GetSchedule(state, AlphaName(4))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(4), AlphaName(4));
            ErrorsFound = true;
        } else if (!contam.removalCoefSched->checkMinVal(state, Clusive::In, 0.0)) {
            Sched::ShowSevereBadMin(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3), AlphaName(3), Clusive::In, 0.0);
            ErrorsFound = true;
        }

        if (contam.ActualZoneNum <= 0) continue; // Error, will be caught and terminated later

        // Object report variables
        SetupOutputVariable(state,
                            "Generic Air Contaminant Constant Source Generation Volume Flow Rate",
                            Constant::Units::m3_s,
                            contam.GenRate,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            contam.Name);

        // Zone total report variables
        ZonePtr = contam.ActualZoneNum;
        if (RepVarSet(ZonePtr)) {
            RepVarSet(ZonePtr) = false;
            SetupOutputVariable(state,
                                "Zone Generic Air Contaminant Generation Volume Flow Rate",
                                Constant::Units::m3_s,
                                state.dataHeatBal->ZoneRpt(ZonePtr).GCRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(ZonePtr).Name);
        }
        SetupZoneInternalGain(state,
                              ZonePtr,
                              contam.Name,
                              DataHeatBalance::IntGainType::ZoneContaminantSourceAndSinkGenericContam,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              &contam.GenRate);
    }

    CurrentModuleObject = "SurfaceContaminantSourceAndSink:Generic:PressureDriven";
    int TotGCGenPDriven = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
    state.dataContaminantBalance->ZoneContamGenericPDriven.allocate(TotGCGenPDriven);

    for (Loop = 1; Loop <= TotGCGenPDriven; ++Loop) {
        AlphaName = "";
        IHGNumbers = 0.0;
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 Loop,
                                                                 AlphaName,
                                                                 NumAlpha,
                                                                 IHGNumbers,
                                                                 NumNumber,
                                                                 IOStat,
                                                                 state.dataIPShortCut->lNumericFieldBlanks,
                                                                 state.dataIPShortCut->lAlphaFieldBlanks,
                                                                 state.dataIPShortCut->cAlphaFieldNames,
                                                                 state.dataIPShortCut->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, CurrentModuleObject, AlphaName(1)};

        Util::IsNameEmpty(state, AlphaName(1), CurrentModuleObject, ErrorsFound);

        auto &contam = state.dataContaminantBalance->ZoneContamGenericPDriven(Loop);
        contam.Name = AlphaName(1);

        contam.SurfName = AlphaName(2);
        contam.SurfNum = Util::FindItemInList(AlphaName(2), state.afn->MultizoneSurfaceData, &AirflowNetwork::MultizoneSurfaceProp::surface_name);
        if (contam.SurfNum == 0) {
            ShowSevereError(state,
                            format("{}{}=\"{}\", invalid {} entered={}",
                                   RoutineName,
                                   CurrentModuleObject,
                                   AlphaName(1),
                                   state.dataIPShortCut->cAlphaFieldNames(2),
                                   AlphaName(2)));
            ShowContinueError(state, "which is not listed in AirflowNetwork:MultiZone:Surface.");
            ErrorsFound = true;
        }
        // Ensure external surface
        if (contam.SurfNum > 0 &&
            state.dataSurface->Surface(state.afn->MultizoneSurfaceData(contam.SurfNum).surface_number).ExtBoundCond != DataSurfaces::ExternalEnvironment) {
            ShowSevereError(
                state,
                format(
                    "{}{}=\"{}. The entered surface ({}) is not an exterior surface", RoutineName, CurrentModuleObject, AlphaName(1), AlphaName(2)));
            ErrorsFound = true;
        }

        if (state.dataIPShortCut->lAlphaFieldBlanks(3)) {
            ShowSevereEmptyField(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3));
            ErrorsFound = true;
        } else if ((contam.generateRateCoefSched = Sched::GetSchedule(state, AlphaName(3))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3), AlphaName(3));
            ErrorsFound = true;
        } else if (!contam.generateRateCoefSched->checkMinVal(state, Clusive::In, 0.0)) {
            Sched::ShowSevereBadMin(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3), AlphaName(3), Clusive::In, 0.0);
            ErrorsFound = true;
        }

        contam.GenRateCoef = IHGNumbers(1);
        if (IHGNumbers(1) < 0.0) {
            ShowSevereError(state,
                            format("{}Negative values are not allowed for {} in {} = {}",
                                   RoutineName,
                                   state.dataIPShortCut->cNumericFieldNames(1),
                                   CurrentModuleObject,
                                   AlphaName(1)));
            ShowContinueError(state, format("The input value is {:.2R}", IHGNumbers(1)));
            ErrorsFound = true;
        }

        contam.Expo = IHGNumbers(2);
        if (IHGNumbers(2) <= 0.0) {
            ShowSevereError(state,
                            format("{}Negative or zero value is not allowed for {} in {} = {}",
                                   RoutineName,
                                   state.dataIPShortCut->cNumericFieldNames(2),
                                   CurrentModuleObject,
                                   AlphaName(1)));
            ShowContinueError(state, format("The input value is {:.2R}", IHGNumbers(2)));
            ErrorsFound = true;
        }
        if (IHGNumbers(2) > 1.0) {
            ShowSevereError(state,
                            format("{}The value greater than 1.0 is not allowed for {} in {} = {}",
                                   RoutineName,
                                   state.dataIPShortCut->cNumericFieldNames(2),
                                   CurrentModuleObject,
                                   AlphaName(1)));
            ShowContinueError(state, format("The input value is {:.2R}", IHGNumbers(2)));
            ErrorsFound = true;
        }

        // Object report variables
        SetupOutputVariable(state,
                            "Generic Air Contaminant Pressure Driven Generation Volume Flow Rate",
                            Constant::Units::m3_s,
                            contam.GenRate,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            contam.Name);

        if (contam.SurfNum > 0) {
            ZonePtr = state.dataSurface->Surface(state.afn->MultizoneSurfaceData(contam.SurfNum).surface_number).Zone;
        } else {
            ZonePtr = 0;
        }
        // Zone total report variables
        if (ZonePtr > 0 && RepVarSet(ZonePtr)) {
            RepVarSet(ZonePtr) = false;
            SetupOutputVariable(state,
                                "Zone Generic Air Contaminant Generation Volume Flow Rate",
                                Constant::Units::m3_s,
                                state.dataHeatBal->ZoneRpt(ZonePtr).GCRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(ZonePtr).Name);
        }
        if (ZonePtr > 0)
            SetupZoneInternalGain(state,
                                  ZonePtr,
                                  contam.Name,
                                  DataHeatBalance::IntGainType::ZoneContaminantSourceAndSinkGenericContam,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  &contam.GenRate);
    }

    CurrentModuleObject = "ZoneContaminantSourceAndSink:Generic:CutoffModel";
    int TotGCGenCutoff = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
    state.dataContaminantBalance->ZoneContamGenericCutoff.allocate(TotGCGenCutoff);

    for (Loop = 1; Loop <= TotGCGenCutoff; ++Loop) {
        AlphaName = "";
        IHGNumbers = 0.0;
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 Loop,
                                                                 AlphaName,
                                                                 NumAlpha,
                                                                 IHGNumbers,
                                                                 NumNumber,
                                                                 IOStat,
                                                                 state.dataIPShortCut->lNumericFieldBlanks,
                                                                 state.dataIPShortCut->lAlphaFieldBlanks,
                                                                 state.dataIPShortCut->cAlphaFieldNames,
                                                                 state.dataIPShortCut->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, CurrentModuleObject, AlphaName(1)};

        Util::IsNameEmpty(state, AlphaName(1), CurrentModuleObject, ErrorsFound);

        auto &contam = state.dataContaminantBalance->ZoneContamGenericCutoff(Loop);
        contam.Name = AlphaName(1);

        contam.ZoneName = AlphaName(2);
        contam.ActualZoneNum = Util::FindItemInList(AlphaName(2), state.dataHeatBal->Zone);
        if (contam.ActualZoneNum == 0) {
            ShowSevereError(state,
                            format("{}{}=\"{}\", invalid {} entered={}",
                                   RoutineName,
                                   CurrentModuleObject,
                                   AlphaName(1),
                                   state.dataIPShortCut->cAlphaFieldNames(2),
                                   AlphaName(2)));
            ErrorsFound = true;
        }

        if (state.dataIPShortCut->lAlphaFieldBlanks(3)) {
            ShowSevereEmptyField(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3));
            ErrorsFound = true;
        } else if ((contam.generateRateSched = Sched::GetSchedule(state, AlphaName(3))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3), AlphaName(3));
            ErrorsFound = true;
        } else if (!contam.generateRateSched->checkMinVal(state, Clusive::In, 0.0)) {
            Sched::ShowSevereBadMin(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3), AlphaName(3), Clusive::In, 0.0);
            ErrorsFound = true;
        }

        contam.GenerateRate = IHGNumbers(1);
        contam.CutoffValue = IHGNumbers(2);

        if (IHGNumbers(1) < 0.0) {
            ShowSevereError(state,
                            format("{}Negative values are not allowed for {} in {} = {}",
                                   RoutineName,
                                   state.dataIPShortCut->cNumericFieldNames(1),
                                   CurrentModuleObject,
                                   AlphaName(1)));
            ShowContinueError(state, format("The input value is {:.2R}", IHGNumbers(1)));
            ErrorsFound = true;
        }
        if (IHGNumbers(2) <= 0.0) {
            ShowSevereError(state,
                            format("{}Negative values or zero are not allowed for {} in {} = {}",
                                   RoutineName,
                                   state.dataIPShortCut->cNumericFieldNames(2),
                                   CurrentModuleObject,
                                   AlphaName(1)));
            ShowContinueError(state, format("The input value is {:.2R}", IHGNumbers(2)));
            ErrorsFound = true;
        }

        // Object report variables
        SetupOutputVariable(state,
                            "Generic Air Contaminant Cutoff Model Generation Volume Flow Rate",
                            Constant::Units::m3_s,
                            contam.GenRate,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            contam.Name);

        // Zone total report variables
        ZonePtr = contam.ActualZoneNum;
        if (RepVarSet(ZonePtr)) {
            RepVarSet(ZonePtr) = false;
            SetupOutputVariable(state,
                                "Zone Generic Air Contaminant Generation Volume Flow Rate",
                                Constant::Units::m3_s,
                                state.dataHeatBal->ZoneRpt(ZonePtr).GCRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(ZonePtr).Name);
        }
        SetupZoneInternalGain(state,
                              ZonePtr,
                              contam.Name,
                              DataHeatBalance::IntGainType::ZoneContaminantSourceAndSinkGenericContam,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              &contam.GenRate);
    }

    CurrentModuleObject = "ZoneContaminantSourceAndSink:Generic:DecaySource";
    int TotGCGenDecay = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
    state.dataContaminantBalance->ZoneContamGenericDecay.allocate(TotGCGenDecay);

    for (Loop = 1; Loop <= TotGCGenDecay; ++Loop) {
        AlphaName = "";
        IHGNumbers = 0.0;
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 Loop,
                                                                 AlphaName,
                                                                 NumAlpha,
                                                                 IHGNumbers,
                                                                 NumNumber,
                                                                 IOStat,
                                                                 state.dataIPShortCut->lNumericFieldBlanks,
                                                                 state.dataIPShortCut->lAlphaFieldBlanks,
                                                                 state.dataIPShortCut->cAlphaFieldNames,
                                                                 state.dataIPShortCut->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, CurrentModuleObject, AlphaName(1)};

        auto &contam = state.dataContaminantBalance->ZoneContamGenericDecay(Loop);

        Util::IsNameEmpty(state, AlphaName(1), CurrentModuleObject, ErrorsFound);
        contam.Name = AlphaName(1);

        contam.ZoneName = AlphaName(2);
        contam.ActualZoneNum = Util::FindItemInList(AlphaName(2), state.dataHeatBal->Zone);
        if (contam.ActualZoneNum == 0) {
            ShowSevereError(state,
                            format("{}{}=\"{}\", invalid {} entered={}",
                                   RoutineName,
                                   CurrentModuleObject,
                                   AlphaName(1),
                                   state.dataIPShortCut->cAlphaFieldNames(2),
                                   AlphaName(2)));
            ErrorsFound = true;
        }

        if (state.dataIPShortCut->lAlphaFieldBlanks(3)) {
            ShowSevereEmptyField(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3));
            ErrorsFound = true;
        } else if ((contam.emitRateSched = Sched::GetSchedule(state, AlphaName(3))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3), AlphaName(3));
            ErrorsFound = true;
        } else if (!contam.emitRateSched->checkMinVal(state, Clusive::In, 0.0)) {
            Sched::ShowSevereBadMin(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3), AlphaName(3), Clusive::In, 0.0);
            ErrorsFound = true;
        }

        contam.InitEmitRate = IHGNumbers(1);
        contam.DelayTime = IHGNumbers(2);

        if (IHGNumbers(1) < 0.0) {
            ShowSevereError(state,
                            format("{}Negative values are not allowed for {} in {} = {}",
                                   RoutineName,
                                   state.dataIPShortCut->cNumericFieldNames(1),
                                   CurrentModuleObject,
                                   AlphaName(1)));
            ShowContinueError(state, format("The input value is {:.2R}", IHGNumbers(1)));
            ErrorsFound = true;
        }
        if (IHGNumbers(2) <= 0.0) {
            ShowSevereError(state,
                            format("{}Negative values or zero are not allowed for {} in {} = {}",
                                   RoutineName,
                                   state.dataIPShortCut->cNumericFieldNames(2),
                                   CurrentModuleObject,
                                   AlphaName(1)));
            ShowContinueError(state, format("The input value is {:.2R}", IHGNumbers(2)));
            ErrorsFound = true;
        }

        // Object report variables
        SetupOutputVariable(state,
                            "Generic Air Contaminant Decay Model Generation Volume Flow Rate",
                            Constant::Units::m3_s,
                            contam.GenRate,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            contam.Name);
        SetupOutputVariable(state,
                            "Generic Air Contaminant Decay Model Generation Emission Start Elapsed Time",
                            Constant::Units::s,
                            contam.Time,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            contam.Name);

        // Zone total report variables
        ZonePtr = contam.ActualZoneNum;
        if (RepVarSet(ZonePtr)) {
            RepVarSet(ZonePtr) = false;
            SetupOutputVariable(state,
                                "Zone Generic Air Contaminant Generation Volume Flow Rate",
                                Constant::Units::m3_s,
                                state.dataHeatBal->ZoneRpt(ZonePtr).GCRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(ZonePtr).Name);
        }
        SetupZoneInternalGain(state,
                              ZonePtr,
                              contam.Name,
                              DataHeatBalance::IntGainType::ZoneContaminantSourceAndSinkGenericContam,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              &contam.GenRate);
    }

    CurrentModuleObject = "SurfaceContaminantSourceAndSink:Generic:BoundaryLayerDiffusion";
    int TotGCBLDiff = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
    state.dataContaminantBalance->ZoneContamGenericBLDiff.allocate(TotGCBLDiff);

    for (Loop = 1; Loop <= TotGCBLDiff; ++Loop) {
        AlphaName = "";
        IHGNumbers = 0.0;
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 Loop,
                                                                 AlphaName,
                                                                 NumAlpha,
                                                                 IHGNumbers,
                                                                 NumNumber,
                                                                 IOStat,
                                                                 state.dataIPShortCut->lNumericFieldBlanks,
                                                                 state.dataIPShortCut->lAlphaFieldBlanks,
                                                                 state.dataIPShortCut->cAlphaFieldNames,
                                                                 state.dataIPShortCut->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, CurrentModuleObject, AlphaName(1)};

        Util::IsNameEmpty(state, AlphaName(1), CurrentModuleObject, ErrorsFound);

        auto &contam = state.dataContaminantBalance->ZoneContamGenericBLDiff(Loop);
        contam.Name = AlphaName(1);
        contam.SurfName = AlphaName(2);
        contam.SurfNum = Util::FindItemInList(AlphaName(2), state.dataSurface->Surface);
        if (contam.SurfNum == 0) {
            ShowSevereError(state,
                            format("{}{}=\"{}\", invalid {} entered={}",
                                   RoutineName,
                                   CurrentModuleObject,
                                   AlphaName(1),
                                   state.dataIPShortCut->cAlphaFieldNames(2),
                                   AlphaName(2)));
            ErrorsFound = true;
        }

        if (state.dataIPShortCut->lAlphaFieldBlanks(3)) {
            ShowSevereEmptyField(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3));
            ErrorsFound = true;
        } else if ((contam.transCoefSched = Sched::GetSchedule(state, AlphaName(3))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3), AlphaName(3));
            ErrorsFound = true;
        } else if (!contam.transCoefSched->checkMinVal(state, Clusive::In, 0.0)) {
            Sched::ShowSevereBadMin(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3), AlphaName(3), Clusive::In, 0.0);
            ErrorsFound = true;
        }

        contam.TransCoef = IHGNumbers(1);
        contam.HenryCoef = IHGNumbers(2);
        if (IHGNumbers(1) < 0.0) {
            ShowSevereError(state,
                            format("{}Negative values are not allowed for {} in {} = {}",
                                   RoutineName,
                                   state.dataIPShortCut->cNumericFieldNames(1),
                                   CurrentModuleObject,
                                   AlphaName(1)));
            ShowContinueError(state, format("The input value is {:.2R}", IHGNumbers(1)));
            ErrorsFound = true;
        }
        if (IHGNumbers(2) <= 0.0) {
            ShowSevereError(state,
                            format("{}Negative values or zero are not allowed for {} in {} = {}",
                                   RoutineName,
                                   state.dataIPShortCut->cNumericFieldNames(2),
                                   CurrentModuleObject,
                                   AlphaName(1)));
            ShowContinueError(state, format("The input value is {:.2R}", IHGNumbers(2)));
            ErrorsFound = true;
        }

        // Object report variables
        SetupOutputVariable(state,
                            "Generic Air Contaminant Boundary Layer Diffusion Generation Volume Flow Rate",
                            Constant::Units::m3_s,
                            contam.GenRate,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            contam.Name);
        if (contam.SurfNum > 0) {
            SetupOutputVariable(state,
                                "Generic Air Contaminant Boundary Layer Diffusion Inside Face Concentration",
                                Constant::Units::ppm,
                                state.dataSurface->SurfGenericContam(contam.SurfNum),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                contam.SurfName);
        }

        ZonePtr = state.dataSurface->Surface(contam.SurfNum).Zone;
        // Zone total report variables
        if (RepVarSet(ZonePtr)) {
            RepVarSet(ZonePtr) = false;
            SetupOutputVariable(state,
                                "Zone Generic Air Contaminant Generation Volume Flow Rate",
                                Constant::Units::m3_s,
                                state.dataHeatBal->ZoneRpt(ZonePtr).GCRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(ZonePtr).Name);
        }
        SetupZoneInternalGain(state,
                              ZonePtr,
                              contam.Name,
                              DataHeatBalance::IntGainType::ZoneContaminantSourceAndSinkGenericContam,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              &contam.GenRate);
    }

    CurrentModuleObject = "SurfaceContaminantSourceAndSink:Generic:DepositionVelocitySink";
    int TotGCDVS = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
    state.dataContaminantBalance->ZoneContamGenericDVS.allocate(TotGCDVS);

    for (Loop = 1; Loop <= TotGCDVS; ++Loop) {
        AlphaName = "";
        IHGNumbers = 0.0;
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 Loop,
                                                                 AlphaName,
                                                                 NumAlpha,
                                                                 IHGNumbers,
                                                                 NumNumber,
                                                                 IOStat,
                                                                 state.dataIPShortCut->lNumericFieldBlanks,
                                                                 state.dataIPShortCut->lAlphaFieldBlanks,
                                                                 state.dataIPShortCut->cAlphaFieldNames,
                                                                 state.dataIPShortCut->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, CurrentModuleObject, AlphaName(1)};

        Util::IsNameEmpty(state, AlphaName(1), CurrentModuleObject, ErrorsFound);
        auto &contam = state.dataContaminantBalance->ZoneContamGenericDVS(Loop);
        contam.Name = AlphaName(1);

        contam.SurfName = AlphaName(2);
        contam.SurfNum = Util::FindItemInList(AlphaName(2), state.dataSurface->Surface);
        if (contam.SurfNum == 0) {
            ShowSevereError(state,
                            format("{}{}=\"{}\", invalid {} entered={}",
                                   RoutineName,
                                   CurrentModuleObject,
                                   AlphaName(1),
                                   state.dataIPShortCut->cAlphaFieldNames(2),
                                   AlphaName(2)));
            ErrorsFound = true;
        }

        if (state.dataIPShortCut->lAlphaFieldBlanks(3)) {
            ShowSevereEmptyField(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3));
            ErrorsFound = true;
        } else if ((contam.depoVeloSched = Sched::GetSchedule(state, AlphaName(3))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3), AlphaName(3));
            ErrorsFound = true;
        } else if (!contam.depoVeloSched->checkMinVal(state, Clusive::In, 0.0)) {
            Sched::ShowSevereBadMin(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3), AlphaName(3), Clusive::In, 0.0);
            ErrorsFound = true;
        }

        contam.DepoVelo = IHGNumbers(1);
        if (IHGNumbers(1) < 0.0) {
            ShowSevereError(state,
                            format("{}Negative values are not allowed for {} in {} = {}",
                                   RoutineName,
                                   state.dataIPShortCut->cNumericFieldNames(1),
                                   CurrentModuleObject,
                                   AlphaName(1)));
            ShowContinueError(state, format("The input value is {:.2R}", IHGNumbers(1)));
            ErrorsFound = true;
        }

        // Object report variables
        SetupOutputVariable(state,
                            "Generic Air Contaminant Deposition Velocity Removal Volume Flow Rate",
                            Constant::Units::m3_s,
                            contam.GenRate,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            contam.Name);

        ZonePtr = state.dataSurface->Surface(contam.SurfNum).Zone;
        // Zone total report variables
        if (RepVarSet(ZonePtr)) {
            RepVarSet(ZonePtr) = false;
            SetupOutputVariable(state,
                                "Zone Generic Air Contaminant Generation Volume Flow Rate",
                                Constant::Units::m3_s,
                                state.dataHeatBal->ZoneRpt(ZonePtr).GCRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(ZonePtr).Name);
        }
        SetupZoneInternalGain(state,
                              ZonePtr,
                              contam.Name,
                              DataHeatBalance::IntGainType::ZoneContaminantSourceAndSinkGenericContam,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              &contam.GenRate);
    }

    CurrentModuleObject = "ZoneContaminantSourceAndSink:Generic:DepositionRateSink";
    int TotGCDRS = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
    state.dataContaminantBalance->ZoneContamGenericDRS.allocate(TotGCDRS);

    for (Loop = 1; Loop <= TotGCDRS; ++Loop) {
        AlphaName = "";
        IHGNumbers = 0.0;
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 Loop,
                                                                 AlphaName,
                                                                 NumAlpha,
                                                                 IHGNumbers,
                                                                 NumNumber,
                                                                 IOStat,
                                                                 state.dataIPShortCut->lNumericFieldBlanks,
                                                                 state.dataIPShortCut->lAlphaFieldBlanks,
                                                                 state.dataIPShortCut->cAlphaFieldNames,
                                                                 state.dataIPShortCut->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, CurrentModuleObject, AlphaName(1)};

        Util::IsNameEmpty(state, AlphaName(1), CurrentModuleObject, ErrorsFound);

        auto &contam = state.dataContaminantBalance->ZoneContamGenericDRS(Loop);
        contam.Name = AlphaName(1);

        contam.ZoneName = AlphaName(2);
        contam.ActualZoneNum = Util::FindItemInList(AlphaName(2), state.dataHeatBal->Zone);
        if (contam.ActualZoneNum == 0) {
            ShowSevereError(state,
                            format("{}{}=\"{}\", invalid {} entered={}",
                                   RoutineName,
                                   CurrentModuleObject,
                                   AlphaName(1),
                                   state.dataIPShortCut->cAlphaFieldNames(2),
                                   AlphaName(2)));
            ErrorsFound = true;
        }

        if (state.dataIPShortCut->lAlphaFieldBlanks(3)) {
            ShowSevereEmptyField(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3));
            ErrorsFound = true;
        } else if ((contam.depoRateSched = Sched::GetSchedule(state, AlphaName(3))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3), AlphaName(3));
            ErrorsFound = true;
        } else if (!contam.depoRateSched->checkMinVal(state, Clusive::In, 0.0)) {
            Sched::ShowSevereBadMin(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3), AlphaName(3), Clusive::In, 0.0);
            ErrorsFound = true;
        }

        contam.DepoRate = IHGNumbers(1);

        if (IHGNumbers(1) < 0.0) {
            ShowSevereError(state,
                            format("{}Negative values are not allowed for {} in {} = {}",
                                   RoutineName,
                                   state.dataIPShortCut->cNumericFieldNames(1),
                                   CurrentModuleObject,
                                   AlphaName(1)));
            ShowContinueError(state, format("The input value is {:.2R}", IHGNumbers(1)));
            ErrorsFound = true;
        }

        // Object report variables
        SetupOutputVariable(state,
                            "Generic Air Contaminant Deposition Rate Removal Volume Flow Rate",
                            Constant::Units::m3_s,
                            contam.GenRate,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            contam.Name);

        ZonePtr = contam.ActualZoneNum;
        // Zone total report variables
        if (RepVarSet(ZonePtr)) {
            RepVarSet(ZonePtr) = false;
            SetupOutputVariable(state,
                                "Zone Generic Air Contaminant Generation Volume Flow Rate",
                                Constant::Units::m3_s,
                                state.dataHeatBal->ZoneRpt(ZonePtr).GCRate,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(ZonePtr).Name);
        }
        SetupZoneInternalGain(state,
                              ZonePtr,
                              contam.Name,
                              DataHeatBalance::IntGainType::ZoneContaminantSourceAndSinkGenericContam,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              &contam.GenRate);
    }

    RepVarSet.deallocate();
    IHGNumbers.deallocate();
    AlphaName.deallocate();

    if (ErrorsFound) {
        ShowFatalError(state, "Errors getting Zone Contaminant Sources and Sinks input data.  Preceding condition(s) cause termination.");
    }
}

void GetZoneContaminanSetPoints(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Lixing Gu
    //       DATE WRITTEN   May 2010

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine gets the inputs related to contaminant control.

    // METHODOLOGY EMPLOYED:
    // Uses the status flags to trigger events.

    static constexpr std::string_view routineName = "GetZoneContaminantSetPoints";

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int ContControlledZoneNum; // The Splitter that you are currently loading input into
    int NumAlphas;
    int NumNums;
    int IOStat;
    bool ErrorsFound(false);

    struct NeededControlTypes
    {
        // Members
        Array1D_bool MustHave; // 4= the four control types
        Array1D_bool DidHave;

        // Default Constructor
        NeededControlTypes() : MustHave(4, false), DidHave(4, false)
        {
        }
    };

    struct NeededComfortControlTypes
    {
        // Members
        Array1D_bool MustHave; // 4= the four control types
        Array1D_bool DidHave;

        // Default Constructor
        NeededComfortControlTypes() : MustHave(12, false), DidHave(12, false)
        {
        }
    };
    auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;
    cCurrentModuleObject = "ZoneControl:ContaminantController";
    int NumContControlledZones = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

    if (NumContControlledZones > 0) {
        state.dataContaminantBalance->ContaminantControlledZone.allocate(NumContControlledZones);
    }

    for (ContControlledZoneNum = 1; ContControlledZoneNum <= NumContControlledZones; ++ContControlledZoneNum) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 ContControlledZoneNum,
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

        Util::IsNameEmpty(state, state.dataIPShortCut->cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);

        auto &controlledZone = state.dataContaminantBalance->ContaminantControlledZone(ContControlledZoneNum);
        controlledZone.Name = state.dataIPShortCut->cAlphaArgs(1);
        controlledZone.ZoneName = state.dataIPShortCut->cAlphaArgs(2);
        controlledZone.ActualZoneNum = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(2), state.dataHeatBal->Zone);
        if (controlledZone.ActualZoneNum == 0) {
            ShowSevereError(state,
                            format("{}=\"{}\" invalid {}=\"{}\" not found.",
                                   cCurrentModuleObject,
                                   state.dataIPShortCut->cAlphaArgs(1),
                                   state.dataIPShortCut->cAlphaFieldNames(2),
                                   state.dataIPShortCut->cAlphaArgs(2)));
            ErrorsFound = true;
        } else {
            //      Zone(ContaminantControlledZone(ContControlledZoneNum)%ActualZoneNum)%TempControlledZoneIndex = ContControlledZoneNum
        }

        if (state.dataIPShortCut->lAlphaFieldBlanks(3)) {
            controlledZone.availSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((controlledZone.availSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(3))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3), state.dataIPShortCut->cAlphaArgs(3));
            ErrorsFound = true;
        } else if (!controlledZone.availSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 1.0)) {
            Sched::ShowSevereBadMinMax(
                state, eoh, state.dataIPShortCut->cAlphaFieldNames(3), state.dataIPShortCut->cAlphaArgs(3), Clusive::In, 0.0, Clusive::In, 1.0);
            ErrorsFound = true;
        } else {
            state.dataHeatBal->Zone(controlledZone.ActualZoneNum).zoneContamControllerSched = controlledZone.availSched;
        }

        if (state.dataIPShortCut->lAlphaFieldBlanks(4)) {
            ShowSevereEmptyField(state, eoh, state.dataIPShortCut->cAlphaFieldNames(4));
            ErrorsFound = true;
        } else if ((controlledZone.setptSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(4))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(4), state.dataIPShortCut->cAlphaArgs(4));
            ErrorsFound = true;
        } else if (!controlledZone.setptSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 2000.0)) {
            Sched::ShowSevereBadMinMax(
                state, eoh, state.dataIPShortCut->cAlphaFieldNames(4), state.dataIPShortCut->cAlphaArgs(4), Clusive::In, 0.0, Clusive::In, 2000.0);
            ErrorsFound = true;
        }

        if (state.dataIPShortCut->lAlphaFieldBlanks(5)) {
            controlledZone.zoneMinCO2Sched = nullptr; // This needs to be nullptr because an empty schedule means outdoorCO2 not zero CO2
        } else if ((controlledZone.zoneMinCO2Sched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(5))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(5), state.dataIPShortCut->cAlphaArgs(5));
            ErrorsFound = true;
        } else if (!controlledZone.zoneMinCO2Sched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 2000.0)) {
            Sched::ShowSevereBadMinMax(
                state, eoh, state.dataIPShortCut->cAlphaFieldNames(5), state.dataIPShortCut->cAlphaArgs(5), Clusive::In, 0.0, Clusive::In, 2000.0);
            ErrorsFound = true;
        } else {
            state.dataHeatBal->Zone(controlledZone.ActualZoneNum).zoneMinCO2Sched = controlledZone.zoneMinCO2Sched;
        }

        if (state.dataIPShortCut->lAlphaFieldBlanks(6)) {
            controlledZone.zoneMaxCO2Sched = nullptr; // This needs to be nullptr because an empty schedule means outdoorCO2, not zero CO2
        } else if ((controlledZone.zoneMaxCO2Sched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(6))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(6), state.dataIPShortCut->cAlphaArgs(6));
            ErrorsFound = true;
        } else if (!controlledZone.zoneMaxCO2Sched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 2000.0)) {
            Sched::ShowSevereBadMinMax(
                state, eoh, state.dataIPShortCut->cAlphaFieldNames(6), state.dataIPShortCut->cAlphaArgs(6), Clusive::In, 0.0, Clusive::In, 2000.0);
            ErrorsFound = true;
        } else {
            state.dataHeatBal->Zone(controlledZone.ActualZoneNum).zoneMaxCO2Sched = controlledZone.zoneMaxCO2Sched;
        }

        if (NumAlphas <= 6) {
            controlledZone.genericContamAvailSched = Sched::GetScheduleAlwaysOn(state);
            continue;
        }

        if (state.dataIPShortCut->lAlphaFieldBlanks(7)) {
            controlledZone.genericContamAvailSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((controlledZone.genericContamAvailSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(7))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(7), state.dataIPShortCut->cAlphaArgs(7));
            ErrorsFound = true;
        } else if (!controlledZone.genericContamAvailSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 1.0)) {
            Sched::ShowSevereBadMinMax(
                state, eoh, state.dataIPShortCut->cAlphaFieldNames(7), state.dataIPShortCut->cAlphaArgs(7), Clusive::In, 0.0, Clusive::In, 1.0);
            ErrorsFound = true;
        }

        if (state.dataIPShortCut->lAlphaFieldBlanks(8)) {
            ShowSevereEmptyField(state, eoh, state.dataIPShortCut->cAlphaArgs(8));
            ErrorsFound = true;
        } else if ((controlledZone.genericContamSetptSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(8))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(8), state.dataIPShortCut->cAlphaArgs(8));
            ErrorsFound = true;
        }

    } // ContControlledZoneNum

    if (ErrorsFound) {
        ShowFatalError(state, "Errors getting Zone Contaminant Control input data.  Preceding condition(s) cause termination.");
    }
}

void InitZoneContSetPoints(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Lixing Gu
    //       DATE WRITTEN   May 2010

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine initializes the data for the zone air contaminant setpoints.

    // METHODOLOGY EMPLOYED:
    // Uses the status flags to trigger events.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 GCGain; // Zone generic contaminant gain
    Real64 Pi;     // Pressue at zone i
    Real64 Pj;     // Pressue at zone j
    Real64 Sch;    // Schedule value

    if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
        state.dataContaminantBalance->OutdoorCO2 = state.dataContaminantBalance->Contaminant.CO2OutdoorSched->getCurrentVal();
    }

    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
        state.dataContaminantBalance->OutdoorGC = state.dataContaminantBalance->Contaminant.genericOutdoorSched->getCurrentVal();
    }

    if (state.dataZoneContaminantPredictorCorrector->MyOneTimeFlag) {
        // CO2
        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            state.dataContaminantBalance->ZoneCO2SetPoint.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->CO2PredictedRate.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->CO2ZoneTimeMinus1.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->CO2ZoneTimeMinus2.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->CO2ZoneTimeMinus3.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->CO2ZoneTimeMinus4.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->DSCO2ZoneTimeMinus1.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->DSCO2ZoneTimeMinus2.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->DSCO2ZoneTimeMinus3.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->DSCO2ZoneTimeMinus4.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->CO2ZoneTimeMinus1Temp.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->CO2ZoneTimeMinus2Temp.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->CO2ZoneTimeMinus3Temp.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->ZoneCO2MX.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->ZoneCO2M2.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->ZoneCO21.dimension(state.dataGlobal->NumOfZones, 0.0);

            state.dataContaminantBalance->ZoneSysContDemand.allocate(state.dataGlobal->NumOfZones);
            state.dataContaminantBalance->ZoneCO2Gain.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->ZoneCO2GainFromPeople.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->ZoneCO2GainExceptPeople.dimension(state.dataGlobal->NumOfZones, 0.0); // Added for hybrid model
            state.dataContaminantBalance->MixingMassFlowCO2.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->ZoneAirDensityCO.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->AZ.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->BZ.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->CZ.dimension(state.dataGlobal->NumOfZones, 0.0);
        }

        state.dataContaminantBalance->CONTRAT.dimension(state.dataGlobal->NumOfZones, 0.0);

        // Allocate Derived Types

        for (int Loop = 1; Loop <= state.dataGlobal->NumOfZones; ++Loop) {
            // Zone CO2
            if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                SetupOutputVariable(state,
                                    "Zone Air CO2 Concentration",
                                    Constant::Units::ppm,
                                    state.dataContaminantBalance->ZoneAirCO2(Loop),
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(Loop).Name);
                SetupOutputVariable(state,
                                    "Zone Air CO2 Predicted Load to Setpoint Mass Flow Rate",
                                    Constant::Units::kg_s,
                                    state.dataContaminantBalance->CO2PredictedRate(Loop),
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(Loop).Name);
                SetupOutputVariable(state,
                                    "Zone Air CO2 Setpoint Concentration",
                                    Constant::Units::ppm,
                                    state.dataContaminantBalance->ZoneCO2SetPoint(Loop),
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(Loop).Name);
                SetupOutputVariable(state,
                                    "Zone Air CO2 Internal Gain Volume Flow Rate",
                                    Constant::Units::m3_s,
                                    state.dataContaminantBalance->ZoneCO2Gain(Loop),
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(Loop).Name);
            }

        } // Loop

        // Generic contaminant
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            state.dataContaminantBalance->ZoneGCSetPoint.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->GCPredictedRate.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->GCZoneTimeMinus1.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->GCZoneTimeMinus2.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->GCZoneTimeMinus3.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->GCZoneTimeMinus4.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->DSGCZoneTimeMinus1.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->DSGCZoneTimeMinus2.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->DSGCZoneTimeMinus3.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->DSGCZoneTimeMinus4.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->GCZoneTimeMinus1Temp.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->GCZoneTimeMinus2Temp.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->GCZoneTimeMinus3Temp.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->ZoneGCMX.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->ZoneGCM2.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->ZoneGC1.dimension(state.dataGlobal->NumOfZones, 0.0);

            if (!allocated(state.dataContaminantBalance->ZoneSysContDemand))
                state.dataContaminantBalance->ZoneSysContDemand.allocate(state.dataGlobal->NumOfZones);
            state.dataContaminantBalance->ZoneGCGain.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->MixingMassFlowGC.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->ZoneAirDensityGC.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->AZGC.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->BZGC.dimension(state.dataGlobal->NumOfZones, 0.0);
            state.dataContaminantBalance->CZGC.dimension(state.dataGlobal->NumOfZones, 0.0);
        }

        state.dataContaminantBalance->CONTRATGC.dimension(state.dataGlobal->NumOfZones, 0.0);

        // Allocate Derived Types

        for (int Loop = 1; Loop <= state.dataGlobal->NumOfZones; ++Loop) {
            // Zone CO2
            if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                SetupOutputVariable(state,
                                    "Zone Air Generic Air Contaminant Concentration",
                                    Constant::Units::ppm,
                                    state.dataContaminantBalance->ZoneAirGC(Loop),
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(Loop).Name);
                SetupOutputVariable(state,
                                    "Zone Generic Air Contaminant Predicted Load to Setpoint Mass Flow Rate",
                                    Constant::Units::kg_s,
                                    state.dataContaminantBalance->GCPredictedRate(Loop),
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(Loop).Name);
                SetupOutputVariable(state,
                                    "Zone Generic Air Contaminant Setpoint Concentration",
                                    Constant::Units::ppm,
                                    state.dataContaminantBalance->ZoneGCSetPoint(Loop),
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataHeatBal->Zone(Loop).Name);
            }
        } // Loop

        state.dataZoneContaminantPredictorCorrector->MyOneTimeFlag = false;
    }

    // Do the Begin Environment initializations
    if (state.dataZoneContaminantPredictorCorrector->MyEnvrnFlag && state.dataGlobal->BeginEnvrnFlag) {
        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            state.dataContaminantBalance->CONTRAT = 0.0;
            state.dataContaminantBalance->CO2ZoneTimeMinus1 = state.dataContaminantBalance->OutdoorCO2;
            state.dataContaminantBalance->CO2ZoneTimeMinus2 = state.dataContaminantBalance->OutdoorCO2;
            state.dataContaminantBalance->CO2ZoneTimeMinus3 = state.dataContaminantBalance->OutdoorCO2;
            state.dataContaminantBalance->CO2ZoneTimeMinus4 = state.dataContaminantBalance->OutdoorCO2;
            state.dataContaminantBalance->DSCO2ZoneTimeMinus1 = state.dataContaminantBalance->OutdoorCO2;
            state.dataContaminantBalance->DSCO2ZoneTimeMinus2 = state.dataContaminantBalance->OutdoorCO2;
            state.dataContaminantBalance->DSCO2ZoneTimeMinus3 = state.dataContaminantBalance->OutdoorCO2;
            state.dataContaminantBalance->DSCO2ZoneTimeMinus4 = state.dataContaminantBalance->OutdoorCO2;
            state.dataContaminantBalance->CO2ZoneTimeMinus1Temp = 0.0;
            state.dataContaminantBalance->CO2ZoneTimeMinus2Temp = 0.0;
            state.dataContaminantBalance->CO2ZoneTimeMinus3Temp = 0.0;
            state.dataContaminantBalance->ZoneAirCO2Temp = state.dataContaminantBalance->OutdoorCO2;
            state.dataContaminantBalance->ZoneCO2SetPoint = 0.0;
            state.dataContaminantBalance->CO2PredictedRate = 0.0;
            state.dataContaminantBalance->ZoneAirCO2 = state.dataContaminantBalance->OutdoorCO2;
            state.dataContaminantBalance->ZoneCO21 = state.dataContaminantBalance->OutdoorCO2;
            state.dataContaminantBalance->ZoneCO2MX = state.dataContaminantBalance->OutdoorCO2;
            state.dataContaminantBalance->ZoneCO2M2 = state.dataContaminantBalance->OutdoorCO2;
        }
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            state.dataContaminantBalance->CONTRAT = 0.0;
            state.dataContaminantBalance->GCZoneTimeMinus1 = state.dataContaminantBalance->OutdoorGC;
            state.dataContaminantBalance->GCZoneTimeMinus2 = state.dataContaminantBalance->OutdoorGC;
            state.dataContaminantBalance->GCZoneTimeMinus3 = state.dataContaminantBalance->OutdoorGC;
            state.dataContaminantBalance->GCZoneTimeMinus4 = state.dataContaminantBalance->OutdoorGC;
            state.dataContaminantBalance->DSGCZoneTimeMinus1 = state.dataContaminantBalance->OutdoorGC;
            state.dataContaminantBalance->DSGCZoneTimeMinus2 = state.dataContaminantBalance->OutdoorGC;
            state.dataContaminantBalance->DSGCZoneTimeMinus3 = state.dataContaminantBalance->OutdoorGC;
            state.dataContaminantBalance->DSGCZoneTimeMinus4 = state.dataContaminantBalance->OutdoorGC;
            state.dataContaminantBalance->GCZoneTimeMinus1Temp = 0.0;
            state.dataContaminantBalance->GCZoneTimeMinus2Temp = 0.0;
            state.dataContaminantBalance->GCZoneTimeMinus3Temp = 0.0;
            state.dataContaminantBalance->ZoneAirGCTemp = state.dataContaminantBalance->OutdoorGC;
            state.dataContaminantBalance->ZoneGCSetPoint = 0.0;
            state.dataContaminantBalance->GCPredictedRate = 0.0;
            state.dataContaminantBalance->ZoneAirGC = state.dataContaminantBalance->OutdoorGC;
            state.dataContaminantBalance->ZoneGC1 = state.dataContaminantBalance->OutdoorGC;
            state.dataContaminantBalance->ZoneGCMX = state.dataContaminantBalance->OutdoorGC;
            state.dataContaminantBalance->ZoneGCM2 = state.dataContaminantBalance->OutdoorGC;
            for (auto &con : state.dataContaminantBalance->ZoneContamGenericBLDiff) {
                state.dataSurface->SurfGenericContam(con.SurfNum) = state.dataContaminantBalance->OutdoorGC;
            }
            if (!state.dataContaminantBalance->ZoneContamGenericDecay.empty())
                for (auto &e : state.dataContaminantBalance->ZoneContamGenericDecay)
                    e.Time = 0.0;
        }
        state.dataZoneContaminantPredictorCorrector->MyEnvrnFlag = false;
    }

    if (!state.dataGlobal->BeginEnvrnFlag) {
        state.dataZoneContaminantPredictorCorrector->MyEnvrnFlag = true;
    }

    if (allocated(state.dataZoneEquip->ZoneEquipConfig) && state.dataZoneContaminantPredictorCorrector->MyConfigOneTimeFlag) {
        bool ErrorsFound = false;
        for (int ContZoneNum = 1; ContZoneNum <= (int)state.dataContaminantBalance->ContaminantControlledZone.size(); ++ContZoneNum) {
            int ZoneNum = state.dataContaminantBalance->ContaminantControlledZone(ContZoneNum).ActualZoneNum;
            for (int zoneInNode = 1; zoneInNode <= state.dataZoneEquip->ZoneEquipConfig(ZoneNum).NumInletNodes; ++zoneInNode) {
                int AirLoopNum = state.dataZoneEquip->ZoneEquipConfig(ZoneNum).InletNodeAirLoopNum(zoneInNode);
                state.dataContaminantBalance->ContaminantControlledZone(ContZoneNum).NumOfZones = 0;
                for (int Loop = 1; Loop <= state.dataGlobal->NumOfZones; ++Loop) {
                    if (!state.dataZoneEquip->ZoneEquipConfig(Loop).IsControlled) continue;
                    for (int zoneInNode2 = 1; zoneInNode2 <= state.dataZoneEquip->ZoneEquipConfig(Loop).NumInletNodes; ++zoneInNode2) {
                        if (AirLoopNum == state.dataZoneEquip->ZoneEquipConfig(Loop).InletNodeAirLoopNum(zoneInNode2)) {
                            ++state.dataContaminantBalance->ContaminantControlledZone(ContZoneNum).NumOfZones;
                            break; // only count a zone once
                        }
                    }
                }
                if (state.dataContaminantBalance->ContaminantControlledZone(ContZoneNum).NumOfZones > 0) {
                    state.dataContaminantBalance->ContaminantControlledZone(ContZoneNum)
                        .ControlZoneNum.allocate(state.dataContaminantBalance->ContaminantControlledZone(ContZoneNum).NumOfZones);
                    int I = 1;
                    for (int Loop = 1; Loop <= state.dataGlobal->NumOfZones; ++Loop) {
                        if (!state.dataZoneEquip->ZoneEquipConfig(Loop).IsControlled) continue;
                        for (int zoneInNode2 = 1; zoneInNode2 <= state.dataZoneEquip->ZoneEquipConfig(Loop).NumInletNodes; ++zoneInNode2) {
                            if (AirLoopNum == state.dataZoneEquip->ZoneEquipConfig(Loop).InletNodeAirLoopNum(zoneInNode2)) {
                                state.dataContaminantBalance->ContaminantControlledZone(ContZoneNum).ControlZoneNum(I) = Loop;
                                ++I;
                                break; // only count a zone once
                            }
                        }
                    }
                } else {
                    ShowSevereError(state,
                                    format("ZoneControl:ContaminantController: a corresponding AirLoopHVAC is not found for the controlled zone ={}",
                                           state.dataHeatBal->Zone(ZoneNum).Name));
                    ErrorsFound = true;
                }
            }
        }
        state.dataZoneContaminantPredictorCorrector->MyConfigOneTimeFlag = false;
        if (ErrorsFound) {
            ShowFatalError(state, "ZoneControl:ContaminantController: Program terminates for preceding reason(s).");
        }
    }

    for (int Loop = 1; Loop <= (int)state.dataContaminantBalance->ContaminantControlledZone.size(); ++Loop) {
        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            int ZoneNum = state.dataContaminantBalance->ContaminantControlledZone(Loop).ActualZoneNum;
            state.dataContaminantBalance->ZoneCO2SetPoint(ZoneNum) =
                state.dataContaminantBalance->ContaminantControlledZone(Loop).setptSched->getCurrentVal();
        }
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            int ZoneNum = state.dataContaminantBalance->ContaminantControlledZone(Loop).ActualZoneNum;
            state.dataContaminantBalance->ZoneGCSetPoint(ZoneNum) =
                state.dataContaminantBalance->ContaminantControlledZone(Loop).genericContamSetptSched->getCurrentVal();
        }
    }

    // CO2 gain
    if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
        for (int Loop = 1; Loop <= state.dataGlobal->NumOfZones; ++Loop) {
            state.dataContaminantBalance->ZoneCO2Gain(Loop) = InternalHeatGains::SumAllInternalCO2Gains(state, Loop);
            if (state.dataHybridModel->FlagHybridModel_PC) {
                state.dataContaminantBalance->ZoneCO2GainExceptPeople(Loop) = InternalHeatGains::SumAllInternalCO2GainsExceptPeople(state, Loop);
            }
            std::array<DataHeatBalance::IntGainType, 1> IntGainPeopleArray = {DataHeatBalance::IntGainType::People};
            state.dataContaminantBalance->ZoneCO2GainFromPeople(Loop) =
                InternalHeatGains::SumInternalCO2GainsByTypes(state, Loop, IntGainPeopleArray);
        }
    }

    // Generic contaminant gain
    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
        state.dataContaminantBalance->ZoneGCGain = 0.0;
        // from constant model
        for (auto &con : state.dataContaminantBalance->ZoneContamGenericConstant) {
            int ZoneNum = con.ActualZoneNum;
            GCGain = con.GenerateRate * con.generateRateSched->getCurrentVal() -
                     con.RemovalCoef * con.removalCoefSched->getCurrentVal() * state.dataContaminantBalance->ZoneAirGC(ZoneNum) * 1.0e-6;
            con.GenRate = GCGain;
        }

        // from pressure driven model
        if (state.afn->simulation_control.type != AirflowNetwork::ControlType::NoMultizoneOrDistribution) {
            for (auto &con : state.dataContaminantBalance->ZoneContamGenericPDriven) {
                int SurfNum = con.SurfNum;
                Pi = state.afn->AirflowNetworkNodeSimu(state.afn->MultizoneSurfaceData(SurfNum).NodeNums[0]).PZ;
                Pj = state.afn->AirflowNetworkNodeSimu(state.afn->MultizoneSurfaceData(SurfNum).NodeNums[1]).PZ;
                if (Pj >= Pi) {
                    GCGain = con.GenRateCoef * con.generateRateCoefSched->getCurrentVal() * std::pow(Pj - Pi, con.Expo);
                } else {
                    GCGain = 0.0;
                }
                con.GenRate = GCGain;
            }
        }

        // from cutoff model
        for (auto &con : state.dataContaminantBalance->ZoneContamGenericCutoff) {
            int ZoneNum = con.ActualZoneNum;
            if (state.dataContaminantBalance->ZoneAirGC(ZoneNum) < con.CutoffValue) {
                GCGain = con.GenerateRate * con.generateRateSched->getCurrentVal() *
                         (1.0 - state.dataContaminantBalance->ZoneAirGC(ZoneNum) / con.CutoffValue);
            } else {
                GCGain = 0.0;
            }
            con.GenRate = GCGain;
        }

        // From decay model
        for (auto &con : state.dataContaminantBalance->ZoneContamGenericDecay) {
            int schVal = con.emitRateSched->getCurrentVal();
            if (schVal == 0 || state.dataGlobal->BeginEnvrnFlag || state.dataGlobal->WarmupFlag) {
                con.Time = 0.0;
            } else {
                con.Time += state.dataGlobal->TimeStepZoneSec;
            }

            GCGain = con.InitEmitRate * schVal * std::exp(-con.Time / con.DelayTime);
            con.GenRate = GCGain;
        }

        // From boudary layer diffusion
        for (auto &con : state.dataContaminantBalance->ZoneContamGenericBLDiff) {
            int SurfNum = con.SurfNum;
            int ZoneNum = state.dataSurface->Surface(SurfNum).Zone;
            // Surface concentration level for the Boundary Layer Diffusion Controlled Model
            Real64 Cs = state.dataSurface->SurfGenericContam(SurfNum);
            Sch = con.transCoefSched->getCurrentVal();
            GCGain = con.TransCoef * Sch * state.dataSurface->Surface(SurfNum).Area * state.dataSurface->Surface(SurfNum).Multiplier *
                     (Cs / con.HenryCoef - state.dataContaminantBalance->ZoneAirGC(ZoneNum)) * 1.0e-6;
            con.GenRate = GCGain;
            // Surface concentration level based on steady-state assumption
            state.dataSurface->SurfGenericContam(SurfNum) =
                Cs - GCGain * 1.0e6 / state.dataSurface->Surface(SurfNum).Multiplier / state.dataSurface->Surface(SurfNum).Area;
        }

        // From deposition velocity sink model
        for (auto &con : state.dataContaminantBalance->ZoneContamGenericDVS) {
            int SurfNum = con.SurfNum;
            int ZoneNum = state.dataSurface->Surface(SurfNum).Zone;
            Sch = con.depoVeloSched->getCurrentVal();
            GCGain = -con.DepoVelo * state.dataSurface->Surface(SurfNum).Area * Sch * state.dataContaminantBalance->ZoneAirGC(ZoneNum) *
                     state.dataSurface->Surface(SurfNum).Multiplier * 1.0e-6;
            con.GenRate = GCGain;
        }

        // From deposition rate sink model
        for (auto &con : state.dataContaminantBalance->ZoneContamGenericDRS) {
            int ZoneNum = con.ActualZoneNum;
            Sch = con.depoRateSched->getCurrentVal();
            GCGain = -con.DepoRate * state.dataHeatBal->Zone(ZoneNum).Volume * Sch * state.dataContaminantBalance->ZoneAirGC(ZoneNum) * 1.0e-6;
            con.GenRate = GCGain;
        }
    }
}

void PredictZoneContaminants(EnergyPlusData &state,
                             bool const ShortenTimeStepSys,
                             bool const UseZoneTimeStepHistory, // if true then use zone timestep history, if false use system time step
                             Real64 const PriorTimeStep         // the old value for timestep length is passed for possible use in interpolating
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Lixing Gu
    //       DATE WRITTEN   May 2010

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine does the prediction step for contaminant control

    // METHODOLOGY EMPLOYED:
    // This solves for the required outdoor airflow to achieve the desired contaminant setpoint in the Zone

    static constexpr std::string_view RoutineName("PredictZoneContaminants");

    Real64 A;                  // Coefficient of storage term in a zone balance equation
    Real64 B;                  // Coefficient of variable term in a zone balance equation
    Real64 C;                  // Coefficient of constnat term in a zone balance equation
    Real64 LoadToCO2SetPoint;  // CO2 load at CO2 set point
    Real64 ZoneAirCO2SetPoint; // Zone CO2 setpoint
    Real64 LoadToGCSetPoint;   // Generic contaminant load at generic contaminant set point
    Real64 ZoneAirGCSetPoint;  // Zone generic contaminant setpoint
    Real64 GCGain;             // Zone generic contaminant internal load

    Real64 timeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;
    // Update zone CO2
    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
        auto &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum);

        if (ShortenTimeStepSys) {

            if (state.dataHeatBal->Zone(ZoneNum).SystemZoneNodeNumber > 0) { // roll back result for zone air node,
                if (state.dataContaminantBalance->Contaminant.CO2Simulation)
                    state.dataLoopNodes->Node(state.dataHeatBal->Zone(ZoneNum).SystemZoneNodeNumber).CO2 =
                        state.dataContaminantBalance->CO2ZoneTimeMinus1(ZoneNum);
                if (state.dataContaminantBalance->Contaminant.GenericContamSimulation)
                    state.dataLoopNodes->Node(state.dataHeatBal->Zone(ZoneNum).SystemZoneNodeNumber).GenContam =
                        state.dataContaminantBalance->GCZoneTimeMinus1(ZoneNum);
            }

            if (state.dataHVACGlobal->NumOfSysTimeSteps !=
                state.dataHVACGlobal->NumOfSysTimeStepsLastZoneTimeStep) { // cannot reuse existing DS data, interpolate from zone time

                if (state.dataContaminantBalance->Contaminant.CO2Simulation)
                    ZoneTempPredictorCorrector::DownInterpolate4HistoryValues(PriorTimeStep,
                                                                              state.dataHVACGlobal->TimeStepSys,
                                                                              state.dataContaminantBalance->CO2ZoneTimeMinus1(ZoneNum),
                                                                              state.dataContaminantBalance->CO2ZoneTimeMinus2(ZoneNum),
                                                                              state.dataContaminantBalance->CO2ZoneTimeMinus3(ZoneNum),
                                                                              state.dataContaminantBalance->ZoneAirCO2(ZoneNum),
                                                                              state.dataContaminantBalance->DSCO2ZoneTimeMinus1(ZoneNum),
                                                                              state.dataContaminantBalance->DSCO2ZoneTimeMinus2(ZoneNum),
                                                                              state.dataContaminantBalance->DSCO2ZoneTimeMinus3(ZoneNum),
                                                                              state.dataContaminantBalance->DSCO2ZoneTimeMinus4(ZoneNum));
                if (state.dataContaminantBalance->Contaminant.GenericContamSimulation)
                    ZoneTempPredictorCorrector::DownInterpolate4HistoryValues(PriorTimeStep,
                                                                              state.dataHVACGlobal->TimeStepSys,
                                                                              state.dataContaminantBalance->GCZoneTimeMinus1(ZoneNum),
                                                                              state.dataContaminantBalance->GCZoneTimeMinus2(ZoneNum),
                                                                              state.dataContaminantBalance->GCZoneTimeMinus3(ZoneNum),
                                                                              state.dataContaminantBalance->ZoneAirGC(ZoneNum),
                                                                              state.dataContaminantBalance->DSGCZoneTimeMinus1(ZoneNum),
                                                                              state.dataContaminantBalance->DSGCZoneTimeMinus2(ZoneNum),
                                                                              state.dataContaminantBalance->DSGCZoneTimeMinus3(ZoneNum),
                                                                              state.dataContaminantBalance->DSGCZoneTimeMinus4(ZoneNum));

            } else { // reuse history data in DS terms from last zone time step to preserve information that would be lost
                     // do nothing because DS history would have been pushed prior and should be ready
            }
        }
        // now update the variables actually used in the balance equations.
        if (UseZoneTimeStepHistory) {

            if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                state.dataContaminantBalance->CO2ZoneTimeMinus1Temp(ZoneNum) = state.dataContaminantBalance->CO2ZoneTimeMinus1(ZoneNum);
                state.dataContaminantBalance->CO2ZoneTimeMinus2Temp(ZoneNum) = state.dataContaminantBalance->CO2ZoneTimeMinus2(ZoneNum);
                state.dataContaminantBalance->CO2ZoneTimeMinus3Temp(ZoneNum) = state.dataContaminantBalance->CO2ZoneTimeMinus3(ZoneNum);
            }
            if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                state.dataContaminantBalance->GCZoneTimeMinus1Temp(ZoneNum) = state.dataContaminantBalance->GCZoneTimeMinus1(ZoneNum);
                state.dataContaminantBalance->GCZoneTimeMinus2Temp(ZoneNum) = state.dataContaminantBalance->GCZoneTimeMinus2(ZoneNum);
                state.dataContaminantBalance->GCZoneTimeMinus3Temp(ZoneNum) = state.dataContaminantBalance->GCZoneTimeMinus3(ZoneNum);
            }

        } else { // use down-stepped history

            if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                state.dataContaminantBalance->CO2ZoneTimeMinus1Temp(ZoneNum) = state.dataContaminantBalance->DSCO2ZoneTimeMinus1(ZoneNum);
                state.dataContaminantBalance->CO2ZoneTimeMinus2Temp(ZoneNum) = state.dataContaminantBalance->DSCO2ZoneTimeMinus2(ZoneNum);
                state.dataContaminantBalance->CO2ZoneTimeMinus3Temp(ZoneNum) = state.dataContaminantBalance->DSCO2ZoneTimeMinus3(ZoneNum);
            }
            if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                state.dataContaminantBalance->GCZoneTimeMinus1Temp(ZoneNum) = state.dataContaminantBalance->DSGCZoneTimeMinus1(ZoneNum);
                state.dataContaminantBalance->GCZoneTimeMinus2Temp(ZoneNum) = state.dataContaminantBalance->DSGCZoneTimeMinus2(ZoneNum);
                state.dataContaminantBalance->GCZoneTimeMinus3Temp(ZoneNum) = state.dataContaminantBalance->DSGCZoneTimeMinus3(ZoneNum);
            }
        }

        if (state.dataHeatBal->ZoneAirSolutionAlgo != DataHeatBalance::SolutionAlgo::ThirdOrder) {
            if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                if (ShortenTimeStepSys && state.dataHVACGlobal->TimeStepSys < state.dataGlobal->TimeStepZone) {
                    if (state.dataHVACGlobal->PreviousTimeStep < state.dataGlobal->TimeStepZone) {
                        state.dataContaminantBalance->ZoneCO21(ZoneNum) = state.dataContaminantBalance->ZoneCO2M2(ZoneNum);
                    } else {
                        state.dataContaminantBalance->ZoneCO21(ZoneNum) = state.dataContaminantBalance->ZoneCO2MX(ZoneNum);
                    }
                    state.dataHVACGlobal->ShortenTimeStepSysRoomAir = true;
                } else {
                    state.dataContaminantBalance->ZoneCO21(ZoneNum) = state.dataContaminantBalance->ZoneAirCO2(ZoneNum);
                }
            }
            if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                if (ShortenTimeStepSys && state.dataHVACGlobal->TimeStepSys < state.dataGlobal->TimeStepZone) {
                    if (state.dataHVACGlobal->PreviousTimeStep < state.dataGlobal->TimeStepZone) {
                        state.dataContaminantBalance->ZoneGC1(ZoneNum) = state.dataContaminantBalance->ZoneGCM2(ZoneNum);
                    } else {
                        state.dataContaminantBalance->ZoneGC1(ZoneNum) = state.dataContaminantBalance->ZoneGCMX(ZoneNum);
                    }
                    state.dataHVACGlobal->ShortenTimeStepSysRoomAir = true;
                } else {
                    state.dataContaminantBalance->ZoneGC1(ZoneNum) = state.dataContaminantBalance->ZoneAirGC(ZoneNum);
                }
            }
        }

        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {

            state.dataContaminantBalance->CO2PredictedRate(ZoneNum) = 0.0;
            LoadToCO2SetPoint = 0.0;
            state.dataContaminantBalance->ZoneSysContDemand(ZoneNum).OutputRequiredToCO2SP = 0.0;

            // Check to see if this is a "CO2 controlled zone"
            bool ControlledCO2ZoneFlag = false;
            // Check all the controlled zones to see if it matches the zone simulated
            for (auto const &contaminantControlledZone : state.dataContaminantBalance->ContaminantControlledZone) {
                if (contaminantControlledZone.ActualZoneNum == ZoneNum) {
                    if (contaminantControlledZone.availSched->getCurrentVal() > 0.0) {
                        ZoneAirCO2SetPoint = state.dataContaminantBalance->ZoneCO2SetPoint(contaminantControlledZone.ActualZoneNum);
                        if (contaminantControlledZone.EMSOverrideCO2SetPointOn) {
                            ZoneAirCO2SetPoint = contaminantControlledZone.EMSOverrideCO2SetPointValue;
                        }
                        ControlledCO2ZoneFlag = true;
                        break;
                    }
                }
            }
            if (!ControlledCO2ZoneFlag) {
                for (auto const &contaminantControlledZone : state.dataContaminantBalance->ContaminantControlledZone) {
                    if (contaminantControlledZone.availSched->getCurrentVal() > 0.0) {
                        ZoneAirCO2SetPoint = state.dataContaminantBalance->ZoneCO2SetPoint(contaminantControlledZone.ActualZoneNum);
                        if (contaminantControlledZone.EMSOverrideCO2SetPointOn) {
                            ZoneAirCO2SetPoint = contaminantControlledZone.EMSOverrideCO2SetPointValue;
                        }
                        if (contaminantControlledZone.NumOfZones >= 1) {
                            if (contaminantControlledZone.ActualZoneNum != ZoneNum) {
                                for (int I = 1; I <= contaminantControlledZone.NumOfZones; ++I) {
                                    if (contaminantControlledZone.ControlZoneNum(I) == ZoneNum) {
                                        ControlledCO2ZoneFlag = true;
                                        break;
                                    }
                                }
                                if (ControlledCO2ZoneFlag) break;
                            } else {
                                ControlledCO2ZoneFlag = true;
                                break;
                            }
                        }
                    }
                }
            } // CO2ControlledZoneNum

            if (ControlledCO2ZoneFlag) {
                Real64 RhoAir = PsyRhoAirFnPbTdbW(state,
                                                  state.dataEnvrn->OutBaroPress,
                                                  thisZoneHB.ZT,
                                                  thisZoneHB.airHumRat,
                                                  RoutineName); // The density of air

                // Calculate Co2 from infiltration + humidity added from latent load to determine system added/subtracted moisture.
                Real64 CO2Gain = state.dataContaminantBalance->ZoneCO2Gain(ZoneNum) * RhoAir * 1.0e6;

                // Calculate the coefficients for the 3rd Order derivative for final
                // zone CO2.  The A, B, C coefficients are analogous to the CO2 balance.
                // Assume that the system will have flow
                if (state.afn->multizone_always_simulated ||
                    (state.afn->simulation_control.type == AirflowNetwork::ControlType::MultizoneWithDistributionOnlyDuringFanOperation &&
                     state.afn->AirflowNetworkFanActivated)) {
                    // Multizone airflow calculated in AirflowNetwork
                    B = CO2Gain + state.afn->exchangeData(ZoneNum).SumMHrCO + state.afn->exchangeData(ZoneNum).SumMMHrCO;
                    A = state.afn->exchangeData(ZoneNum).SumMHr + state.afn->exchangeData(ZoneNum).SumMMHr;
                } else {
                    B = CO2Gain +
                        ((thisZoneHB.OAMFL + thisZoneHB.VAMFL + thisZoneHB.EAMFL + thisZoneHB.CTMFL) * state.dataContaminantBalance->OutdoorCO2) +
                        state.dataContaminantBalance->MixingMassFlowCO2(ZoneNum) + thisZoneHB.MDotOA * state.dataContaminantBalance->OutdoorCO2;
                    A = thisZoneHB.OAMFL + thisZoneHB.VAMFL + thisZoneHB.EAMFL + thisZoneHB.CTMFL + thisZoneHB.MixingMassFlowZone + thisZoneHB.MDotOA;
                }
                C = RhoAir * state.dataHeatBal->Zone(ZoneNum).Volume * state.dataHeatBal->Zone(ZoneNum).ZoneVolCapMultpCO2 / timeStepSysSec;

                // Use a 3rd Order derivative to predict zone moisture addition or removal and
                // smooth the changes using the zone air capacitance.  Positive values of CO2 Load means that
                // this amount of CO2 must be added to the zone to reach the setpoint.  Negative values represent
                // the amount of CO2 that must be removed by the system.
                switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
                case DataHeatBalance::SolutionAlgo::ThirdOrder: {
                    LoadToCO2SetPoint = ((11.0 / 6.0) * C + A) * ZoneAirCO2SetPoint -
                                        (B + C * (3.0 * state.dataContaminantBalance->CO2ZoneTimeMinus1Temp(ZoneNum) -
                                                  (3.0 / 2.0) * state.dataContaminantBalance->CO2ZoneTimeMinus2Temp(ZoneNum) +
                                                  (1.0 / 3.0) * state.dataContaminantBalance->CO2ZoneTimeMinus3Temp(ZoneNum)));
                    // Exact solution
                } break;
                case DataHeatBalance::SolutionAlgo::AnalyticalSolution: {
                    if (A == 0.0) { // B=0
                        LoadToCO2SetPoint = C * (ZoneAirCO2SetPoint - state.dataContaminantBalance->ZoneCO21(ZoneNum)) - B;
                    } else {
                        LoadToCO2SetPoint =
                            A * (ZoneAirCO2SetPoint - state.dataContaminantBalance->ZoneCO21(ZoneNum) * std::exp(min(700.0, -A / C))) /
                                (1.0 - std::exp(min(700.0, -A / C))) -
                            B;
                    }
                } break;
                case DataHeatBalance::SolutionAlgo::EulerMethod: {
                    LoadToCO2SetPoint = C * (ZoneAirCO2SetPoint - state.dataContaminantBalance->ZoneCO21(ZoneNum)) + A * ZoneAirCO2SetPoint - B;
                } break;
                default:
                    break;
                }
                if (ZoneAirCO2SetPoint > state.dataContaminantBalance->OutdoorCO2 && LoadToCO2SetPoint < 0.0) {
                    state.dataContaminantBalance->ZoneSysContDemand(ZoneNum).OutputRequiredToCO2SP =
                        LoadToCO2SetPoint / (state.dataContaminantBalance->OutdoorCO2 - ZoneAirCO2SetPoint);
                }
            }

            // Apply the Zone Multiplier to the total zone moisture load
            state.dataContaminantBalance->ZoneSysContDemand(ZoneNum).OutputRequiredToCO2SP *=
                state.dataHeatBal->Zone(ZoneNum).Multiplier * state.dataHeatBal->Zone(ZoneNum).ListMultiplier;
            state.dataContaminantBalance->CO2PredictedRate(ZoneNum) = state.dataContaminantBalance->ZoneSysContDemand(ZoneNum).OutputRequiredToCO2SP;
        }

        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {

            state.dataContaminantBalance->GCPredictedRate(ZoneNum) = 0.0;
            LoadToGCSetPoint = 0.0;
            state.dataContaminantBalance->ZoneSysContDemand(ZoneNum).OutputRequiredToGCSP = 0.0;

            // Check to see if this is a "GC controlled zone"
            bool ControlledGCZoneFlag = false;
            // Check all the controlled zones to see if it matches the zone simulated
            for (auto const &contaminantControlledZone : state.dataContaminantBalance->ContaminantControlledZone) {
                if (contaminantControlledZone.ActualZoneNum == ZoneNum) {
                    if (contaminantControlledZone.genericContamAvailSched->getCurrentVal() > 0.0) {
                        ZoneAirGCSetPoint = state.dataContaminantBalance->ZoneGCSetPoint(contaminantControlledZone.ActualZoneNum);
                        if (contaminantControlledZone.EMSOverrideCO2SetPointOn) {
                            ZoneAirGCSetPoint = contaminantControlledZone.EMSOverrideGCSetPointValue;
                        }
                        ControlledGCZoneFlag = true;
                        break;
                    }
                }
            }
            if (!ControlledGCZoneFlag) {
                for (auto const &contaminantControlledZone : state.dataContaminantBalance->ContaminantControlledZone) {
                    if (contaminantControlledZone.genericContamAvailSched->getCurrentVal() > 0.0) {
                        ZoneAirGCSetPoint = state.dataContaminantBalance->ZoneGCSetPoint(contaminantControlledZone.ActualZoneNum);
                        if (contaminantControlledZone.EMSOverrideCO2SetPointOn) {
                            ZoneAirGCSetPoint = contaminantControlledZone.EMSOverrideGCSetPointValue;
                        }
                        if (contaminantControlledZone.NumOfZones >= 1) {
                            if (contaminantControlledZone.ActualZoneNum != ZoneNum) {
                                for (int I = 1; I <= contaminantControlledZone.NumOfZones; ++I) {
                                    if (contaminantControlledZone.ControlZoneNum(I) == ZoneNum) {
                                        ControlledGCZoneFlag = true;
                                        break;
                                    }
                                }
                                if (ControlledGCZoneFlag) break;
                            } else {
                                ControlledGCZoneFlag = true;
                                break;
                            }
                        }
                    }
                }
            }

            if (ControlledGCZoneFlag) {
                // The density of air
                Real64 RhoAir = PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, thisZoneHB.ZT, thisZoneHB.airHumRat, RoutineName);

                // Calculate generic contaminant from infiltration + humidity added from latent load
                // to determine system added/subtracted moisture.
                GCGain = state.dataContaminantBalance->ZoneGCGain(ZoneNum) * RhoAir * 1.0e6;

                // Calculate the coefficients for the 3rd Order derivative for final
                // zone GC.  The A, B, C coefficients are analogous to the GC balance.
                // Assume that the system will have flow
                if (state.afn->multizone_always_simulated ||
                    (state.afn->simulation_control.type == AirflowNetwork::ControlType::MultizoneWithDistributionOnlyDuringFanOperation &&
                     state.afn->AirflowNetworkFanActivated)) {
                    // Multizone airflow calculated in AirflowNetwork
                    B = GCGain + state.afn->exchangeData(ZoneNum).SumMHrGC + state.afn->exchangeData(ZoneNum).SumMMHrGC;
                    A = state.afn->exchangeData(ZoneNum).SumMHr + state.afn->exchangeData(ZoneNum).SumMMHr;
                } else {
                    B = GCGain +
                        ((thisZoneHB.OAMFL + thisZoneHB.VAMFL + thisZoneHB.EAMFL + thisZoneHB.CTMFL) * state.dataContaminantBalance->OutdoorGC) +
                        state.dataContaminantBalance->MixingMassFlowGC(ZoneNum) + thisZoneHB.MDotOA * state.dataContaminantBalance->OutdoorGC;
                    A = thisZoneHB.OAMFL + thisZoneHB.VAMFL + thisZoneHB.EAMFL + thisZoneHB.CTMFL + thisZoneHB.MixingMassFlowZone + thisZoneHB.MDotOA;
                }
                C = RhoAir * state.dataHeatBal->Zone(ZoneNum).Volume * state.dataHeatBal->Zone(ZoneNum).ZoneVolCapMultpGenContam / timeStepSysSec;

                // Use a 3rd Order derivative to predict zone moisture addition or removal and
                // smooth the changes using the zone air capacitance.  Positive values of GC Load means that
                // this amount of GC must be added to the zone to reach the setpoint.  Negative values represent
                // the amount of GC that must be removed by the system.
                switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
                case DataHeatBalance::SolutionAlgo::ThirdOrder: {
                    LoadToGCSetPoint = ((11.0 / 6.0) * C + A) * ZoneAirGCSetPoint -
                                       (B + C * (3.0 * state.dataContaminantBalance->GCZoneTimeMinus1Temp(ZoneNum) -
                                                 (3.0 / 2.0) * state.dataContaminantBalance->GCZoneTimeMinus2Temp(ZoneNum) +
                                                 (1.0 / 3.0) * state.dataContaminantBalance->GCZoneTimeMinus3Temp(ZoneNum)));
                    // Exact solution
                } break;
                case DataHeatBalance::SolutionAlgo::AnalyticalSolution: {
                    if (A == 0.0) { // B=0
                        LoadToGCSetPoint = C * (ZoneAirGCSetPoint - state.dataContaminantBalance->ZoneGC1(ZoneNum)) - B;
                    } else {
                        LoadToGCSetPoint = A * (ZoneAirGCSetPoint - state.dataContaminantBalance->ZoneGC1(ZoneNum) * std::exp(min(700.0, -A / C))) /
                                               (1.0 - std::exp(min(700.0, -A / C))) -
                                           B;
                    }
                } break;
                case DataHeatBalance::SolutionAlgo::EulerMethod: {
                    LoadToGCSetPoint = C * (ZoneAirGCSetPoint - state.dataContaminantBalance->ZoneGC1(ZoneNum)) + A * ZoneAirGCSetPoint - B;
                } break;
                default:
                    break;
                }
                if (ZoneAirGCSetPoint > state.dataContaminantBalance->OutdoorGC && LoadToGCSetPoint < 0.0) {
                    state.dataContaminantBalance->ZoneSysContDemand(ZoneNum).OutputRequiredToGCSP =
                        LoadToGCSetPoint / (state.dataContaminantBalance->OutdoorGC - ZoneAirGCSetPoint);
                }
            }

            // Apply the Zone Multiplier to the total zone moisture load
            state.dataContaminantBalance->ZoneSysContDemand(ZoneNum).OutputRequiredToGCSP *=
                state.dataHeatBal->Zone(ZoneNum).Multiplier * state.dataHeatBal->Zone(ZoneNum).ListMultiplier;
            state.dataContaminantBalance->GCPredictedRate(ZoneNum) = state.dataContaminantBalance->ZoneSysContDemand(ZoneNum).OutputRequiredToGCSP;
        }
    }
}

void PushZoneTimestepHistories(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Lixing Gu
    //       DATE WRITTEN   July, 2010

    // PURPOSE OF THIS SUBROUTINE:
    // Push the temperature and humidity ratio histories
    // This subroutine is modified from PushZoneTimestepHistories in ZoneTempPredictorCorrector module

    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            state.dataContaminantBalance->CO2ZoneTimeMinus4(ZoneNum) = state.dataContaminantBalance->CO2ZoneTimeMinus3(ZoneNum);
            state.dataContaminantBalance->CO2ZoneTimeMinus3(ZoneNum) = state.dataContaminantBalance->CO2ZoneTimeMinus2(ZoneNum);
            state.dataContaminantBalance->CO2ZoneTimeMinus2(ZoneNum) = state.dataContaminantBalance->CO2ZoneTimeMinus1(ZoneNum);
            state.dataContaminantBalance->CO2ZoneTimeMinus1(ZoneNum) =
                state.dataContaminantBalance->ZoneAirCO2Avg(ZoneNum); // using average for whole zone time step.
            state.dataContaminantBalance->ZoneAirCO2(ZoneNum) = state.dataContaminantBalance->ZoneAirCO2Temp(ZoneNum);

            if (state.dataHeatBal->ZoneAirSolutionAlgo != DataHeatBalance::SolutionAlgo::ThirdOrder) {
                state.dataContaminantBalance->ZoneCO2M2(ZoneNum) = state.dataContaminantBalance->ZoneCO2MX(ZoneNum);
                state.dataContaminantBalance->ZoneCO2MX(ZoneNum) =
                    state.dataContaminantBalance->ZoneAirCO2Avg(ZoneNum); // using average for whole zone time step.
            }
        }

        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            state.dataContaminantBalance->GCZoneTimeMinus4(ZoneNum) = state.dataContaminantBalance->GCZoneTimeMinus3(ZoneNum);
            state.dataContaminantBalance->GCZoneTimeMinus3(ZoneNum) = state.dataContaminantBalance->GCZoneTimeMinus2(ZoneNum);
            state.dataContaminantBalance->GCZoneTimeMinus2(ZoneNum) = state.dataContaminantBalance->GCZoneTimeMinus1(ZoneNum);
            state.dataContaminantBalance->GCZoneTimeMinus1(ZoneNum) =
                state.dataContaminantBalance->ZoneAirGCAvg(ZoneNum); // using average for whole zone time step.
            state.dataContaminantBalance->ZoneAirGC(ZoneNum) = state.dataContaminantBalance->ZoneAirGCTemp(ZoneNum);

            if (state.dataHeatBal->ZoneAirSolutionAlgo != DataHeatBalance::SolutionAlgo::ThirdOrder) {
                state.dataContaminantBalance->ZoneGCM2(ZoneNum) = state.dataContaminantBalance->ZoneGCMX(ZoneNum);
                state.dataContaminantBalance->ZoneGCMX(ZoneNum) =
                    state.dataContaminantBalance->ZoneAirGCAvg(ZoneNum); // using average for whole zone time step.
            }
        }
    }
}

void PushSystemTimestepHistories(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Lixing Gu
    //       DATE WRITTEN   July, 2010

    // PURPOSE OF THIS SUBROUTINE:
    // push histories back in time
    // This subroutine is modified from PushSystemTimestepHistories in ZoneTempPredictorCorrector module

    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            state.dataContaminantBalance->DSCO2ZoneTimeMinus4(ZoneNum) = state.dataContaminantBalance->DSCO2ZoneTimeMinus3(ZoneNum);
            state.dataContaminantBalance->DSCO2ZoneTimeMinus3(ZoneNum) = state.dataContaminantBalance->DSCO2ZoneTimeMinus2(ZoneNum);
            state.dataContaminantBalance->DSCO2ZoneTimeMinus2(ZoneNum) = state.dataContaminantBalance->DSCO2ZoneTimeMinus1(ZoneNum);
            state.dataContaminantBalance->DSCO2ZoneTimeMinus1(ZoneNum) = state.dataContaminantBalance->ZoneAirCO2(ZoneNum);
        }
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            state.dataContaminantBalance->DSGCZoneTimeMinus4(ZoneNum) = state.dataContaminantBalance->DSGCZoneTimeMinus3(ZoneNum);
            state.dataContaminantBalance->DSGCZoneTimeMinus3(ZoneNum) = state.dataContaminantBalance->DSGCZoneTimeMinus2(ZoneNum);
            state.dataContaminantBalance->DSGCZoneTimeMinus2(ZoneNum) = state.dataContaminantBalance->DSGCZoneTimeMinus1(ZoneNum);
            state.dataContaminantBalance->DSGCZoneTimeMinus1(ZoneNum) = state.dataContaminantBalance->ZoneAirGC(ZoneNum);
        }
    }

    if (state.dataHeatBal->ZoneAirSolutionAlgo != DataHeatBalance::SolutionAlgo::ThirdOrder) {
        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                state.dataContaminantBalance->ZoneCO2M2(ZoneNum) = state.dataContaminantBalance->ZoneCO2MX(ZoneNum);
                state.dataContaminantBalance->ZoneCO2MX(ZoneNum) =
                    state.dataContaminantBalance->ZoneAirCO2Temp(ZoneNum); // using average for whole zone time step.
            }
            if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                state.dataContaminantBalance->ZoneGCM2(ZoneNum) = state.dataContaminantBalance->ZoneGCMX(ZoneNum);
                state.dataContaminantBalance->ZoneGCMX(ZoneNum) =
                    state.dataContaminantBalance->ZoneAirGCTemp(ZoneNum); // using average for whole zone time step.
            }
        }
    }
}

void RevertZoneTimestepHistories(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Lixing Gu
    //       DATE WRITTEN   July, 2010

    // PURPOSE OF THIS SUBROUTINE:
    // rewind histories to undo inadvertent pushing
    // This subroutine is modified from RevertZoneTimestepHistories in ZoneTempPredictorCorrector module

    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            state.dataContaminantBalance->CO2ZoneTimeMinus1(ZoneNum) = state.dataContaminantBalance->CO2ZoneTimeMinus2(ZoneNum);
            state.dataContaminantBalance->CO2ZoneTimeMinus2(ZoneNum) = state.dataContaminantBalance->CO2ZoneTimeMinus3(ZoneNum);
            state.dataContaminantBalance->CO2ZoneTimeMinus3(ZoneNum) = state.dataContaminantBalance->CO2ZoneTimeMinus4(ZoneNum);
        }
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            state.dataContaminantBalance->GCZoneTimeMinus1(ZoneNum) = state.dataContaminantBalance->GCZoneTimeMinus2(ZoneNum);
            state.dataContaminantBalance->GCZoneTimeMinus2(ZoneNum) = state.dataContaminantBalance->GCZoneTimeMinus3(ZoneNum);
            state.dataContaminantBalance->GCZoneTimeMinus3(ZoneNum) = state.dataContaminantBalance->GCZoneTimeMinus4(ZoneNum);
        }
    }
}

void InverseModelCO2(EnergyPlusData &state,
                     int const ZoneNum,                // Zone number
                     Real64 const CO2Gain,             // Zone total CO2 gain
                     Real64 const CO2GainExceptPeople, // ZOne total CO2 gain from sources except for people
                     Real64 const ZoneMassFlowRate,    // Zone air mass flow rate
                     Real64 const CO2MassFlowRate,     // Zone air CO2 mass flow rate
                     Real64 const RhoAir               // Air density
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Han Li
    //       DATE WRITTEN   February 2019

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine inversely solve infiltration airflow rate or people count with zone air CO2 concentration measurements.

    Real64 AA(0.0);
    Real64 BB(0.0);
    Real64 M_inf(0.0); // Reversely solved infiltration mass flow rate

    Real64 timeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

    auto &hmZone = state.dataHybridModel->hybridModelZones(ZoneNum);

    state.dataHeatBal->Zone(ZoneNum).ZoneMeasuredCO2Concentration = hmZone.measuredCO2ConcSched->getCurrentVal();

    if (state.dataEnvrn->DayOfYear >= hmZone.HybridStartDayOfYear && state.dataEnvrn->DayOfYear <= hmZone.HybridEndDayOfYear) {
        state.dataContaminantBalance->ZoneAirCO2(ZoneNum) = state.dataHeatBal->Zone(ZoneNum).ZoneMeasuredCO2Concentration;

        auto const &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum);
        if (hmZone.InfiltrationCalc_C && state.dataHVACGlobal->UseZoneTimeStepHistory) {
            static constexpr std::string_view RoutineNameInfiltration("CalcAirFlowSimple:Infiltration");
            // Conditionally calculate the CO2-dependent and CO2-independent terms.
            if (hmZone.IncludeSystemSupplyParameters) {
                state.dataHeatBal->Zone(ZoneNum).ZoneMeasuredSupplyAirFlowRate = hmZone.supplyAirMassFlowRateSched->getCurrentVal();
                state.dataHeatBal->Zone(ZoneNum).ZoneMeasuredSupplyAirCO2Concentration = hmZone.supplyAirCO2ConcSched->getCurrentVal();

                Real64 SumSysM_HM = state.dataHeatBal->Zone(ZoneNum).ZoneMeasuredSupplyAirFlowRate;
                Real64 SumSysMxCO2_HM = state.dataHeatBal->Zone(ZoneNum).ZoneMeasuredSupplyAirFlowRate *
                                        state.dataHeatBal->Zone(ZoneNum).ZoneMeasuredSupplyAirCO2Concentration;

                AA = SumSysM_HM + thisZoneHB.VAMFL + thisZoneHB.EAMFL + thisZoneHB.CTMFL + thisZoneHB.MixingMassFlowZone + thisZoneHB.MDotOA;
                BB = SumSysMxCO2_HM + CO2Gain +
                     ((thisZoneHB.VAMFL + thisZoneHB.EAMFL + thisZoneHB.CTMFL) * state.dataContaminantBalance->OutdoorCO2) +
                     state.dataContaminantBalance->MixingMassFlowCO2(ZoneNum) + thisZoneHB.MDotOA * state.dataContaminantBalance->OutdoorCO2;

            } else {
                AA = thisZoneHB.VAMFL + thisZoneHB.EAMFL + thisZoneHB.CTMFL + thisZoneHB.MixingMassFlowZone + thisZoneHB.MDotOA;
                BB = CO2Gain + ((thisZoneHB.VAMFL + thisZoneHB.EAMFL + thisZoneHB.CTMFL) * state.dataContaminantBalance->OutdoorCO2) +
                     state.dataContaminantBalance->MixingMassFlowCO2(ZoneNum) + thisZoneHB.MDotOA * state.dataContaminantBalance->OutdoorCO2;
            }

            Real64 CC = RhoAir * state.dataHeatBal->Zone(ZoneNum).Volume * state.dataHeatBal->Zone(ZoneNum).ZoneVolCapMultpCO2 / timeStepSysSec;
            Real64 DD = (3.0 * state.dataContaminantBalance->CO2ZoneTimeMinus1Temp(ZoneNum) -
                         (3.0 / 2.0) * state.dataContaminantBalance->CO2ZoneTimeMinus2Temp(ZoneNum) +
                         (1.0 / 3.0) * state.dataContaminantBalance->CO2ZoneTimeMinus3Temp(ZoneNum));

            Real64 delta_CO2 = (state.dataHeatBal->Zone(ZoneNum).ZoneMeasuredCO2Concentration - state.dataContaminantBalance->OutdoorCO2) / 1000;
            Real64 CpAir = PsyCpAirFnW(state.dataEnvrn->OutHumRat);
            Real64 AirDensity = PsyRhoAirFnPbTdbW(state,
                                                  state.dataEnvrn->OutBaroPress,
                                                  state.dataHeatBal->Zone(ZoneNum).OutDryBulbTemp,
                                                  state.dataEnvrn->OutHumRat,
                                                  RoutineNameInfiltration);

            if (state.dataHeatBal->Zone(ZoneNum).ZoneMeasuredCO2Concentration == state.dataContaminantBalance->OutdoorCO2) {
                M_inf = 0.0;
            } else {
                M_inf = (CC * DD + BB - ((11.0 / 6.0) * CC + AA) * state.dataHeatBal->Zone(ZoneNum).ZoneMeasuredCO2Concentration) / delta_CO2;
            }

            // Add threshold for air change rate
            Real64 ACH_inf = max(0.0, min(10.0, M_inf / (CpAir * AirDensity / Constant::rSecsInHour * state.dataHeatBal->Zone(ZoneNum).Volume)));
            M_inf = ACH_inf * state.dataHeatBal->Zone(ZoneNum).Volume * AirDensity / Constant::rSecsInHour;
            state.dataHeatBal->Zone(ZoneNum).MCPIHM = M_inf;
            state.dataHeatBal->Zone(ZoneNum).InfilOAAirChangeRateHM = ACH_inf;
        }

        // Hybrid Model calculate people count
        if (hmZone.PeopleCountCalc_C && state.dataHVACGlobal->UseZoneTimeStepHistory) {
            state.dataHeatBal->Zone(ZoneNum).ZonePeopleActivityLevel =
                hmZone.peopleActivityLevelSched ? hmZone.peopleActivityLevelSched->getCurrentVal() : 0.0;
            Real64 ActivityLevel = hmZone.peopleActivityLevelSched ? hmZone.peopleActivityLevelSched->getCurrentVal() : 0.0;
            Real64 CO2GenRate = hmZone.peopleCO2GenRateSched ? hmZone.peopleCO2GenRateSched->getCurrentVal() : 0.0;
            if (ActivityLevel <= 0.0) {
                ActivityLevel = 130.0; // 130.0 is the default people activity level [W]
            }
            if (CO2GenRate <= 0.0) {
                CO2GenRate = 0.0000000382; // 0.0000000382 is the default CO2, generation rate [m3/(s*W)]
            }

            // Conditionally calculate the CO2-dependent and CO2-independent terms.
            if (hmZone.IncludeSystemSupplyParameters) {
                state.dataHeatBal->Zone(ZoneNum).ZoneMeasuredSupplyAirFlowRate = hmZone.supplyAirMassFlowRateSched->getCurrentVal();
                state.dataHeatBal->Zone(ZoneNum).ZoneMeasuredSupplyAirCO2Concentration = hmZone.supplyAirCO2ConcSched->getCurrentVal();

                Real64 SumSysM_HM = state.dataHeatBal->Zone(ZoneNum).ZoneMeasuredSupplyAirFlowRate;
                Real64 SumSysMxCO2_HM = state.dataHeatBal->Zone(ZoneNum).ZoneMeasuredSupplyAirFlowRate *
                                        state.dataHeatBal->Zone(ZoneNum).ZoneMeasuredSupplyAirCO2Concentration;

                AA = SumSysM_HM + thisZoneHB.OAMFL + thisZoneHB.VAMFL + thisZoneHB.EAMFL + thisZoneHB.CTMFL + thisZoneHB.MixingMassFlowZone +
                     thisZoneHB.MDotOA;
                BB = CO2GainExceptPeople +
                     ((thisZoneHB.OAMFL + thisZoneHB.VAMFL + thisZoneHB.EAMFL + thisZoneHB.CTMFL) * state.dataContaminantBalance->OutdoorCO2) +
                     (SumSysMxCO2_HM) + state.dataContaminantBalance->MixingMassFlowCO2(ZoneNum) +
                     thisZoneHB.MDotOA * state.dataContaminantBalance->OutdoorCO2;

            } else {
                AA = ZoneMassFlowRate + thisZoneHB.OAMFL + thisZoneHB.VAMFL + thisZoneHB.EAMFL + thisZoneHB.CTMFL + thisZoneHB.MixingMassFlowZone +
                     thisZoneHB.MDotOA;
                BB = CO2GainExceptPeople +
                     ((thisZoneHB.OAMFL + thisZoneHB.VAMFL + thisZoneHB.EAMFL + thisZoneHB.CTMFL) * state.dataContaminantBalance->OutdoorCO2) +
                     (CO2MassFlowRate) + state.dataContaminantBalance->MixingMassFlowCO2(ZoneNum) +
                     thisZoneHB.MDotOA * state.dataContaminantBalance->OutdoorCO2;
            }

            Real64 CC = RhoAir * state.dataHeatBal->Zone(ZoneNum).Volume * state.dataHeatBal->Zone(ZoneNum).ZoneVolCapMultpCO2 / timeStepSysSec;
            Real64 DD = (3.0 * state.dataContaminantBalance->CO2ZoneTimeMinus1Temp(ZoneNum) -
                         (3.0 / 2.0) * state.dataContaminantBalance->CO2ZoneTimeMinus2Temp(ZoneNum) +
                         (1.0 / 3.0) * state.dataContaminantBalance->CO2ZoneTimeMinus3Temp(ZoneNum));

            Real64 CO2GainPeople =
                (((11.0 / 6.0) * CC + AA) * state.dataHeatBal->Zone(ZoneNum).ZoneMeasuredCO2Concentration - BB - CC * DD) / (1000000 * RhoAir);

            // Make sure the results are reasonable
            Real64 UpperBound = CO2Gain / (1000000 * RhoAir * CO2GenRate * ActivityLevel);
            Real64 NumPeople = min(UpperBound, CO2GainPeople / (CO2GenRate * ActivityLevel));

            NumPeople = floor(NumPeople * 100.00 + 0.5) / 100.00;
            if (NumPeople < 0.05) {
                NumPeople = 0;
            }
            state.dataHeatBal->Zone(ZoneNum).NumOccHM = NumPeople;
        }
    }

    // Update zone humidity ratio in the previous steps
    state.dataContaminantBalance->CO2ZoneTimeMinus3Temp(ZoneNum) = state.dataContaminantBalance->CO2ZoneTimeMinus2Temp(ZoneNum);
    state.dataContaminantBalance->CO2ZoneTimeMinus2Temp(ZoneNum) = state.dataContaminantBalance->CO2ZoneTimeMinus1Temp(ZoneNum);
    state.dataContaminantBalance->CO2ZoneTimeMinus1Temp(ZoneNum) = state.dataHeatBal->Zone(ZoneNum).ZoneMeasuredCO2Concentration;
}

void CorrectZoneContaminants(EnergyPlusData &state,
                             bool const UseZoneTimeStepHistory // if true then use zone timestep history, if false use system time step history
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Lixing Gu
    //       DATE WRITTEN   July, 2010

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine updates the zone contaminants.
    // This subroutine is modified from CorrectZoneHumRat in ZoneTempPredictorCorrector module

    // REFERENCES:
    // Routine FinalZnCalcs - FINAL ZONE CALCULATIONS, authored by Dale Herron for BLAST.

    static constexpr std::string_view RoutineName("CorrectZoneContaminants");

    Real64 CO2Gain;             // Zone CO2 internal gain
    Real64 CO2GainExceptPeople; // Added for hybrid model, Zone CO2 internal gain
    Real64 GCGain;              // Zone generic contaminant internal gain
    Real64 A;
    Real64 B;
    Real64 C;

    // Update zone CO2
    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {

        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            state.dataContaminantBalance->AZ(ZoneNum) = 0.0;
            state.dataContaminantBalance->BZ(ZoneNum) = 0.0;
            state.dataContaminantBalance->CZ(ZoneNum) = 0.0;
        }
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            state.dataContaminantBalance->AZGC(ZoneNum) = 0.0;
            state.dataContaminantBalance->BZGC(ZoneNum) = 0.0;
            state.dataContaminantBalance->CZGC(ZoneNum) = 0.0;
        }

        // update the variables actually used in the balance equations.
        if (!UseZoneTimeStepHistory) {
            if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                state.dataContaminantBalance->CO2ZoneTimeMinus1Temp(ZoneNum) = state.dataContaminantBalance->DSCO2ZoneTimeMinus1(ZoneNum);
                state.dataContaminantBalance->CO2ZoneTimeMinus2Temp(ZoneNum) = state.dataContaminantBalance->DSCO2ZoneTimeMinus2(ZoneNum);
                state.dataContaminantBalance->CO2ZoneTimeMinus3Temp(ZoneNum) = state.dataContaminantBalance->DSCO2ZoneTimeMinus3(ZoneNum);
            }
            if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                state.dataContaminantBalance->GCZoneTimeMinus1Temp(ZoneNum) = state.dataContaminantBalance->DSGCZoneTimeMinus1(ZoneNum);
                state.dataContaminantBalance->GCZoneTimeMinus2Temp(ZoneNum) = state.dataContaminantBalance->DSGCZoneTimeMinus2(ZoneNum);
                state.dataContaminantBalance->GCZoneTimeMinus3Temp(ZoneNum) = state.dataContaminantBalance->DSGCZoneTimeMinus3(ZoneNum);
            }
        } else {
            if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                state.dataContaminantBalance->CO2ZoneTimeMinus1Temp(ZoneNum) = state.dataContaminantBalance->CO2ZoneTimeMinus1(ZoneNum);
                state.dataContaminantBalance->CO2ZoneTimeMinus2Temp(ZoneNum) = state.dataContaminantBalance->CO2ZoneTimeMinus2(ZoneNum);
                state.dataContaminantBalance->CO2ZoneTimeMinus3Temp(ZoneNum) = state.dataContaminantBalance->CO2ZoneTimeMinus3(ZoneNum);
            }
            if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                state.dataContaminantBalance->GCZoneTimeMinus1Temp(ZoneNum) = state.dataContaminantBalance->GCZoneTimeMinus1(ZoneNum);
                state.dataContaminantBalance->GCZoneTimeMinus2Temp(ZoneNum) = state.dataContaminantBalance->GCZoneTimeMinus2(ZoneNum);
                state.dataContaminantBalance->GCZoneTimeMinus3Temp(ZoneNum) = state.dataContaminantBalance->GCZoneTimeMinus3(ZoneNum);
            }
        }

        // Start to calculate zone CO2 and genric contaminant levels
        Real64 CO2MassFlowRate = 0.0;
        Real64 GCMassFlowRate = 0.0;
        Real64 ZoneMassFlowRate = 0.0;
        int ZoneMult = state.dataHeatBal->Zone(ZoneNum).Multiplier * state.dataHeatBal->Zone(ZoneNum).ListMultiplier;

        // Check to see if this is a controlled zone
        const bool ControlledZoneAirFlag = state.dataHeatBal->Zone(ZoneNum).IsControlled;

        // Check to see if this is a plenum zone
        bool ZoneRetPlenumAirFlag = false;
        int ZoneRetPlenumNum = 0;
        for (ZoneRetPlenumNum = 1; ZoneRetPlenumNum <= state.dataZonePlenum->NumZoneReturnPlenums; ++ZoneRetPlenumNum) {
            if (state.dataZonePlenum->ZoneRetPlenCond(ZoneRetPlenumNum).ActualZoneNum != ZoneNum) continue;
            ZoneRetPlenumAirFlag = true;
            break;
        }
        bool ZoneSupPlenumAirFlag = false;
        int ZoneSupPlenumNum = 0;
        for (ZoneSupPlenumNum = 1; ZoneSupPlenumNum <= state.dataZonePlenum->NumZoneSupplyPlenums; ++ZoneSupPlenumNum) {
            if (state.dataZonePlenum->ZoneSupPlenCond(ZoneSupPlenumNum).ActualZoneNum != ZoneNum) continue;
            ZoneSupPlenumAirFlag = true;
            break;
        }

        if (ControlledZoneAirFlag) { // If there is system flow then calculate the flow rates

            // Calculate moisture flow rate into each zone
            for (int NodeNum = 1; NodeNum <= state.dataZoneEquip->ZoneEquipConfig(ZoneNum).NumInletNodes; ++NodeNum) {
                auto const &node = state.dataLoopNodes->Node(state.dataZoneEquip->ZoneEquipConfig(ZoneNum).InletNode(NodeNum));
                if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                    CO2MassFlowRate += (node.MassFlowRate * node.CO2) / ZoneMult;
                }
                if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                    GCMassFlowRate += (node.MassFlowRate * node.GenContam) / ZoneMult;
                }
                ZoneMassFlowRate += node.MassFlowRate / ZoneMult;
            }

            // Do the calculations for the plenum zone
        } else if (ZoneRetPlenumAirFlag) {
            for (int NodeNum = 1; NodeNum <= state.dataZonePlenum->ZoneRetPlenCond(ZoneRetPlenumNum).NumInletNodes; ++NodeNum) {
                auto const &node = state.dataLoopNodes->Node(state.dataZonePlenum->ZoneRetPlenCond(ZoneRetPlenumNum).InletNode(NodeNum));
                if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                    CO2MassFlowRate += (node.MassFlowRate * node.CO2) / ZoneMult;
                }
                if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                    GCMassFlowRate += (node.MassFlowRate * node.GenContam) / ZoneMult;
                }
                ZoneMassFlowRate += node.MassFlowRate / ZoneMult;
            }
            // add in the leak flow
            for (int ADUListIndex = 1; ADUListIndex <= state.dataZonePlenum->ZoneRetPlenCond(ZoneRetPlenumNum).NumADUs; ++ADUListIndex) {
                int ADUNum = state.dataZonePlenum->ZoneRetPlenCond(ZoneRetPlenumNum).ADUIndex(ADUListIndex);
                if (state.dataDefineEquipment->AirDistUnit(ADUNum).UpStreamLeak) {
                    auto &airDistUnit = state.dataDefineEquipment->AirDistUnit(ADUNum);
                    auto const &node = state.dataLoopNodes->Node(airDistUnit.InletNodeNum);
                    if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                        CO2MassFlowRate += (airDistUnit.MassFlowRateUpStrLk * node.CO2) / ZoneMult;
                    }
                    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                        GCMassFlowRate += (airDistUnit.MassFlowRateUpStrLk * node.GenContam) / ZoneMult;
                    }
                    ZoneMassFlowRate += airDistUnit.MassFlowRateUpStrLk / ZoneMult;
                }
                if (state.dataDefineEquipment->AirDistUnit(ADUNum).DownStreamLeak) {
                    auto &airDistUnit = state.dataDefineEquipment->AirDistUnit(ADUNum);
                    auto const &node = state.dataLoopNodes->Node(airDistUnit.OutletNodeNum);
                    if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                        CO2MassFlowRate += (airDistUnit.MassFlowRateDnStrLk * node.CO2) / ZoneMult;
                    }
                    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                        GCMassFlowRate += (airDistUnit.MassFlowRateDnStrLk * node.GenContam) / ZoneMult;
                    }
                    ZoneMassFlowRate += airDistUnit.MassFlowRateDnStrLk / ZoneMult;
                }
            }

        } else if (ZoneSupPlenumAirFlag) {
            auto const &node = state.dataLoopNodes->Node(state.dataZonePlenum->ZoneSupPlenCond(ZoneSupPlenumNum).InletNode);
            if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                CO2MassFlowRate += (node.MassFlowRate * node.CO2) / ZoneMult;
            }
            if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                GCMassFlowRate += (node.MassFlowRate * node.GenContam) / ZoneMult;
            }
            ZoneMassFlowRate += node.MassFlowRate / ZoneMult;
        }

        Real64 timeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;
        auto &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum);

        // Calculate the coefficients for the 3rd order derivative for final
        // zone humidity ratio.  The A, B, C coefficients are analogous to the
        // CO2 balance.  There are 2 cases that should be considered, system operating and system shutdown.

        Real64 RhoAir = PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, thisZoneHB.ZT, thisZoneHB.airHumRat, RoutineName);

        if (state.dataContaminantBalance->Contaminant.CO2Simulation) state.dataContaminantBalance->ZoneAirDensityCO(ZoneNum) = RhoAir;
        // Calculate Co2 internal gain
        if (state.dataContaminantBalance->Contaminant.CO2Simulation) CO2Gain = state.dataContaminantBalance->ZoneCO2Gain(ZoneNum) * RhoAir * 1.0e6;
        if (state.dataContaminantBalance->Contaminant.CO2Simulation)
            CO2GainExceptPeople = state.dataContaminantBalance->ZoneCO2GainExceptPeople(ZoneNum) * RhoAir * 1.0e6; // Addded for hybrid model
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation)
            GCGain = state.dataContaminantBalance->ZoneGCGain(ZoneNum) * RhoAir * 1.0e6;

        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            B = CO2Gain + ((thisZoneHB.OAMFL + thisZoneHB.VAMFL + thisZoneHB.EAMFL + thisZoneHB.CTMFL) * state.dataContaminantBalance->OutdoorCO2) +
                (CO2MassFlowRate) + state.dataContaminantBalance->MixingMassFlowCO2(ZoneNum) +
                thisZoneHB.MDotOA * state.dataContaminantBalance->OutdoorCO2;
            A = ZoneMassFlowRate + thisZoneHB.OAMFL + thisZoneHB.VAMFL + thisZoneHB.EAMFL + thisZoneHB.CTMFL + thisZoneHB.MixingMassFlowZone +
                thisZoneHB.MDotOA;
            if (state.afn->multizone_always_simulated ||
                (state.afn->simulation_control.type == AirflowNetwork::ControlType::MultizoneWithDistributionOnlyDuringFanOperation &&
                 state.afn->AirflowNetworkFanActivated)) {
                // Multizone airflow calculated in AirflowNetwork
                B = CO2Gain + (state.afn->exchangeData(ZoneNum).SumMHrCO + state.afn->exchangeData(ZoneNum).SumMMHrCO) + CO2MassFlowRate;
                A = ZoneMassFlowRate + state.afn->exchangeData(ZoneNum).SumMHr + state.afn->exchangeData(ZoneNum).SumMMHr;
            }
            C = RhoAir * state.dataHeatBal->Zone(ZoneNum).Volume * state.dataHeatBal->Zone(ZoneNum).ZoneVolCapMultpCO2 / timeStepSysSec;
        }

        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            auto &zoneAirCO2Temp = state.dataContaminantBalance->ZoneAirCO2Temp(ZoneNum);
            if (state.afn->distribution_simulated) {
                B += state.afn->exchangeData(ZoneNum).TotalCO2;
            }
            state.dataContaminantBalance->AZ(ZoneNum) = A;
            state.dataContaminantBalance->BZ(ZoneNum) = B;
            state.dataContaminantBalance->CZ(ZoneNum) = C;

            // Use a 3rd order derivative to predict final zone CO2 and
            // smooth the changes using the zone air capacitance.
            switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
            case DataHeatBalance::SolutionAlgo::ThirdOrder: {
                zoneAirCO2Temp = (B + C * (3.0 * state.dataContaminantBalance->CO2ZoneTimeMinus1Temp(ZoneNum) -
                                           (3.0 / 2.0) * state.dataContaminantBalance->CO2ZoneTimeMinus2Temp(ZoneNum) +
                                           (1.0 / 3.0) * state.dataContaminantBalance->CO2ZoneTimeMinus3Temp(ZoneNum))) /
                                 ((11.0 / 6.0) * C + A);
                // Exact solution
            } break;
            case DataHeatBalance::SolutionAlgo::AnalyticalSolution: {
                if (A == 0.0) { // B=0
                    zoneAirCO2Temp = state.dataContaminantBalance->ZoneCO21(ZoneNum) + B / C;
                } else {
                    zoneAirCO2Temp = (state.dataContaminantBalance->ZoneCO21(ZoneNum) - B / A) * std::exp(min(700.0, -A / C)) + B / A;
                }
            } break;
            case DataHeatBalance::SolutionAlgo::EulerMethod: {
                zoneAirCO2Temp = (C * state.dataContaminantBalance->ZoneCO21(ZoneNum) + B) / (C + A);
            } break;
            default:
                break;
            }

            // Set the CO2 to zero if the zone has been large sinks
            if (zoneAirCO2Temp < 0.0) zoneAirCO2Temp = 0.0;
            state.dataContaminantBalance->ZoneAirCO2(ZoneNum) = zoneAirCO2Temp;

            if (state.dataHybridModel->FlagHybridModel) {
                auto &hmZone = state.dataHybridModel->hybridModelZones(ZoneNum);
                if ((hmZone.InfiltrationCalc_C || hmZone.PeopleCountCalc_C) && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing)) {
                    InverseModelCO2(state, ZoneNum, CO2Gain, CO2GainExceptPeople, ZoneMassFlowRate, CO2MassFlowRate, RhoAir);
                }
            }
            // Now put the calculated info into the actual zone nodes; ONLY if there is zone air flow, i.e. controlled zone or plenum zone
            const int ZoneNodeNum = state.dataHeatBal->Zone(ZoneNum).SystemZoneNodeNumber;
            if (ZoneNodeNum > 0) {
                state.dataLoopNodes->Node(ZoneNodeNum).CO2 = zoneAirCO2Temp;
            }
        }

        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            B = GCGain + ((thisZoneHB.OAMFL + thisZoneHB.VAMFL + thisZoneHB.EAMFL + thisZoneHB.CTMFL) * state.dataContaminantBalance->OutdoorGC) +
                (GCMassFlowRate) + state.dataContaminantBalance->MixingMassFlowGC(ZoneNum) +
                thisZoneHB.MDotOA * state.dataContaminantBalance->OutdoorGC;
            A = ZoneMassFlowRate + thisZoneHB.OAMFL + thisZoneHB.VAMFL + thisZoneHB.EAMFL + thisZoneHB.CTMFL + thisZoneHB.MixingMassFlowZone +
                thisZoneHB.MDotOA;
            if (state.afn->multizone_always_simulated ||
                (state.afn->simulation_control.type == AirflowNetwork::ControlType::MultizoneWithDistributionOnlyDuringFanOperation &&
                 state.afn->AirflowNetworkFanActivated)) {
                // Multizone airflow calculated in AirflowNetwork
                B = GCGain + (state.afn->exchangeData(ZoneNum).SumMHrGC + state.afn->exchangeData(ZoneNum).SumMMHrGC) + GCMassFlowRate;
                A = ZoneMassFlowRate + state.afn->exchangeData(ZoneNum).SumMHr + state.afn->exchangeData(ZoneNum).SumMMHr;
            }
            C = RhoAir * state.dataHeatBal->Zone(ZoneNum).Volume * state.dataHeatBal->Zone(ZoneNum).ZoneVolCapMultpGenContam / timeStepSysSec;
        }

        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            auto &zoneAirGCTemp = state.dataContaminantBalance->ZoneAirGCTemp(ZoneNum);
            if (state.afn->distribution_simulated) {
                B += state.afn->exchangeData(ZoneNum).TotalGC;
            }

            state.dataContaminantBalance->AZGC(ZoneNum) = A;
            state.dataContaminantBalance->BZGC(ZoneNum) = B;
            state.dataContaminantBalance->CZGC(ZoneNum) = C;

            // Use a 3rd order derivative to predict final zone generic contaminant and
            // smooth the changes using the zone air capacitance.
            switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
            case DataHeatBalance::SolutionAlgo::ThirdOrder: {
                zoneAirGCTemp = (B + C * (3.0 * state.dataContaminantBalance->GCZoneTimeMinus1Temp(ZoneNum) -
                                          (3.0 / 2.0) * state.dataContaminantBalance->GCZoneTimeMinus2Temp(ZoneNum) +
                                          (1.0 / 3.0) * state.dataContaminantBalance->GCZoneTimeMinus3Temp(ZoneNum))) /
                                ((11.0 / 6.0) * C + A);
                // Exact solution
            } break;
            case DataHeatBalance::SolutionAlgo::AnalyticalSolution: {
                if (A == 0.0) { // B=0
                    zoneAirGCTemp = state.dataContaminantBalance->ZoneGC1(ZoneNum) + B / C;
                } else {
                    zoneAirGCTemp = (state.dataContaminantBalance->ZoneGC1(ZoneNum) - B / A) * std::exp(min(700.0, -A / C)) + B / A;
                }
            } break;
            case DataHeatBalance::SolutionAlgo::EulerMethod: {
                zoneAirGCTemp = (C * state.dataContaminantBalance->ZoneGC1(ZoneNum) + B) / (C + A);
            } break;
            default:
                break;
            }

            // Set the generic contaminant to zero if the zone has been large sinks
            if (zoneAirGCTemp < 0.0) zoneAirGCTemp = 0.0;
            state.dataContaminantBalance->ZoneAirGC(ZoneNum) = zoneAirGCTemp;

            // Now put the calculated info into the actual zone nodes; ONLY if there is zone air flow, i.e. controlled zone or plenum zone
            const int ZoneNodeNum = state.dataHeatBal->Zone(ZoneNum).SystemZoneNodeNumber;
            if (ZoneNodeNum > 0) {
                state.dataLoopNodes->Node(ZoneNodeNum).GenContam = zoneAirGCTemp;
            }
        }
    }
}

} // namespace EnergyPlus::ZoneContaminantPredictorCorrector
