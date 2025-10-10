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

// ObjexxFCL Headers
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataZoneControls.hh>
#include <EnergyPlus/DemandManager.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::DemandManager {

// MODULE INFORMATION:
//       AUTHOR         Peter Graham Ellis
//       DATE WRITTEN   July 2005
//       MODIFIED       Simon Vidanovic (March 2015) - Introduced DemandManager:Ventilation
//       RE-ENGINEERED  na

// PURPOSE OF THIS MODULE:
// This module provides controls for demand limiting various loads.

// METHODOLOGY EMPLOYED:
// ManageDemand is called from within the ManageHVAC routine after the first pass through SimHVAC, but
// _before_ any variables are reported or histories are updated.  If the metered demand is above the
// limit action is taken using the various demand managers to reduce loads.  Exterior energy use, zone
// heat balance, and HVAC system are then resimulated as necessary.  It is possible to iterate several
// times through ManageDemand before the final demand managers are established and the timestep can be
// completed.

constexpr std::array<std::string_view, static_cast<int>(ManagerType::Num)> ManagerNamesUC{"DEMANDMANAGER:EXTERIORLIGHTS",
                                                                                          "DEMANDMANAGER:LIGHTS",
                                                                                          "DEMANDMANAGER:ELECTRICEQUIPMENT",
                                                                                          "DEMANDMANAGER:THERMOSTATS",
                                                                                          "DEMANDMANAGER:VENTILATION"};
constexpr std::array<std::string_view, static_cast<int>(ManagePriorityType::Num)> ManagePriorityNamesUC{"SEQUENTIAL", "OPTIMAL", "ALL"};
constexpr std::array<std::string_view, static_cast<int>(ManagerLimit::Num)> ManagerLimitNamesUC{"OFF", "FIXED", "VARIABLE", "REDUCTIONRATIO"};
constexpr std::array<std::string_view, static_cast<int>(ManagerLimit::Num)> ManagerLimitVentNamesUC{"OFF", "FIXEDRATE", "VARIABLE", "REDUCTIONRATIO"};
constexpr std::array<std::string_view, static_cast<int>(ManagerSelection::Num)> ManagerSelectionNamesUC{"ALL", "ROTATEMANY", "ROTATEONE"};

void ManageDemand(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   July 2005

    // Locals
    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

    if (state.dataDemandManager->GetInput && !state.dataGlobal->DoingSizing) {
        GetDemandManagerInput(state);
        GetDemandManagerListInput(state);
        state.dataDemandManager->GetInput = false;
    }

    if (state.dataDemandManager->NumDemandManagerList > 0) {

        if (state.dataGlobal->WarmupFlag) {
            state.dataDemandManager->BeginDemandSim = true;
            if (state.dataDemandManager->ClearHistory) {
                // Clear historical variables
                for (int ListNum = 1; ListNum <= state.dataDemandManager->NumDemandManagerList; ++ListNum) {
                    state.dataDemandManager->DemandManagerList(ListNum).History = 0.0;
                    state.dataDemandManager->DemandManagerList(ListNum).MeterDemand = 0.0;
                    state.dataDemandManager->DemandManagerList(ListNum).AverageDemand = 0.0;
                    state.dataDemandManager->DemandManagerList(ListNum).PeakDemand = 0.0;
                    state.dataDemandManager->DemandManagerList(ListNum).ScheduledLimit = 0.0;
                    state.dataDemandManager->DemandManagerList(ListNum).DemandLimit = 0.0;
                    state.dataDemandManager->DemandManagerList(ListNum).AvoidedDemand = 0.0;
                    state.dataDemandManager->DemandManagerList(ListNum).OverLimit = 0.0;
                    state.dataDemandManager->DemandManagerList(ListNum).OverLimitDuration = 0.0;
                } // ListNum

                // Clear demand manager variables
                for (auto &e : state.dataDemandManager->DemandMgr) {
                    e.Active = false;
                    e.ElapsedTime = 0;
                    e.ElapsedRotationTime = 0;
                    e.RotatedLoadNum = 0;
                }
            }
            state.dataDemandManager->ClearHistory = false;
        }

        if (!state.dataGlobal->WarmupFlag && !state.dataGlobal->DoingSizing) {

            if (state.dataDemandManager->BeginDemandSim) {
                state.dataDemandManager->BeginDemandSim = false;
                state.dataDemandManager->ClearHistory = true;
            }

            state.dataDemandManager->DemandManagerExtIterations = 0;
            state.dataDemandManager->DemandManagerHBIterations = 0;
            state.dataDemandManager->DemandManagerHVACIterations = 0;

            state.dataDemandManager->firstTime = true;
            state.dataDemandManager->ResimExt = false;
            state.dataDemandManager->ResimHB = false;
            state.dataDemandManager->ResimHVAC = false;

            while (state.dataDemandManager->firstTime || state.dataDemandManager->ResimExt || state.dataDemandManager->ResimHB ||
                   state.dataDemandManager->ResimHVAC) {
                state.dataDemandManager->firstTime = false;

                Resimulate(state, state.dataDemandManager->ResimExt, state.dataDemandManager->ResimHB, state.dataDemandManager->ResimHVAC);
                state.dataDemandManager->ResimExt = false;
                state.dataDemandManager->ResimHB = false;
                state.dataDemandManager->ResimHVAC = false;

                SurveyDemandManagers(state); // Determines which Demand Managers can reduce demand

                for (int ListNum = 1; ListNum <= state.dataDemandManager->NumDemandManagerList; ++ListNum) {
                    SimulateDemandManagerList(
                        state, ListNum, state.dataDemandManager->ResimExt, state.dataDemandManager->ResimHB, state.dataDemandManager->ResimHVAC);
                } // ListNum

                ActivateDemandManagers(state); // Sets limits on loads

                if (state.dataDemandManager->DemandManagerExtIterations + state.dataDemandManager->DemandManagerHBIterations +
                        state.dataDemandManager->DemandManagerHVACIterations >
                    500) {
                    // This error can only happen if there is a bug in the code
                    ShowFatalError(state, "Too many DemandManager iterations. (>500)");
                    break;
                }
            }

            for (int ListNum = 1; ListNum <= state.dataDemandManager->NumDemandManagerList; ++ListNum) {
                ReportDemandManagerList(state, ListNum);
            } // ListNum
        }
    }
}

void SimulateDemandManagerList(EnergyPlusData &state,
                               int const ListNum,
                               bool &ResimExt, // Flag to resimulate the exterior energy use simulation
                               bool &ResimHB,  // Flag to resimulate the heat balance simulation (including HVAC)
                               bool &ResimHVAC // Flag to resimulate the HVAC simulation
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   July 2005
    //       MODIFIED       Simon Vidanovic (March 2015) - Introduced DemandManager:Ventilation

    // Using/Aliasing
    Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool OnPeak;

    auto &demandManagerList = state.dataDemandManager->DemandManagerList(ListNum);

    demandManagerList.ScheduledLimit = demandManagerList.limitSched->getCurrentVal();
    demandManagerList.DemandLimit = demandManagerList.ScheduledLimit * demandManagerList.SafetyFraction;

    demandManagerList.MeterDemand =
        GetInstantMeterValue(state, demandManagerList.Meter, OutputProcessor::TimeStepType::Zone) / state.dataGlobal->TimeStepZoneSec +
        GetInstantMeterValue(state, demandManagerList.Meter, OutputProcessor::TimeStepType::System) / TimeStepSysSec;

    // Calculate average demand over the averaging window including the current timestep meter demand
    Real64 AverageDemand =
        demandManagerList.AverageDemand + (demandManagerList.MeterDemand - demandManagerList.History(1)) / demandManagerList.AveragingWindow;

    OnPeak = (demandManagerList.peakSched == nullptr) || (demandManagerList.peakSched->getCurrentVal() == 1);

    if (OnPeak) {
        Real64 OverLimit = AverageDemand - demandManagerList.DemandLimit;

        if (OverLimit > 0.0) {

            switch (demandManagerList.ManagerPriority) {
            case ManagePriorityType::Sequential: { // Activate first Demand Manager that can reduce demand

                for (int MgrNum = 1; MgrNum <= demandManagerList.NumOfManager; ++MgrNum) {
                    auto &demandMgr = state.dataDemandManager->DemandMgr(demandManagerList.Manager(MgrNum));

                    if (demandMgr.CanReduceDemand) {
                        demandMgr.Activate = true;

                        switch (demandMgr.Type) {
                        case ManagerType::ExtLights: {
                            ResimExt = true;
                        } break;
                        case ManagerType::Lights:
                        case ManagerType::ElecEquip: {
                            ResimHB = true;
                            ResimHVAC = true;
                        } break;
                        case ManagerType::Thermostats:
                        case ManagerType::Ventilation: {
                            ResimHVAC = true;
                        } break;
                        default:
                            break;
                        }

                        break; // Leave the loop
                    }
                } // MgrNum

            } break;
            case ManagePriorityType::Optimal: {
                // Not yet implemented

            } break;
            case ManagePriorityType::All: { // Activate ALL Demand Managers that can reduce demand

                for (int MgrNum = 1; MgrNum <= demandManagerList.NumOfManager; ++MgrNum) {
                    auto &demandMgr = state.dataDemandManager->DemandMgr(demandManagerList.Manager(MgrNum));

                    if (demandMgr.CanReduceDemand) {
                        demandMgr.Activate = true;

                        switch (demandMgr.Type) {
                        case ManagerType::ExtLights: {
                            ResimExt = true;
                        } break;
                        case ManagerType::Lights:
                        case ManagerType::ElecEquip: {
                            ResimHB = true;
                            ResimHVAC = true;
                        } break;
                        case ManagerType::Thermostats:
                        case ManagerType::Ventilation: {
                            ResimHVAC = true;
                        } break;
                        default:
                            break;
                        }
                    }
                } // MgrNum
            } break;
            default:
                break;
            }
        }
    }
}

void GetDemandManagerListInput(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   July 2005
    //       MODIFIED       Simon Vidanovic (March 2015) - Introduced DemandManager:Ventilation

    // PURPOSE OF THIS SUBROUTINE:
    // Gets the DEMAND MANAGER LIST input from the input file.

    // METHODOLOGY EMPLOYED:
    // Standard EnergyPlus methodology.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

    static constexpr std::string_view routineName = "GetDemandManagerListInput";
    constexpr std::string_view cCurrentModuleObject = "DemandManagerAssignmentList";

    auto &s_ipsc = state.dataIPShortCut;
    auto const &s_ip = state.dataInputProcessing->inputProcessor;

    state.dataDemandManager->NumDemandManagerList = s_ip->getNumObjectsFound(state, cCurrentModuleObject);

    if (state.dataDemandManager->NumDemandManagerList > 0) {
        int NumAlphas; // Number of elements in the alpha array
        int NumNums;   // Number of elements in the numeric array
        int IOStat;    // IO Status when calling get input subroutine
        bool ErrorsFound = false;

        state.dataDemandManager->DemandManagerList.allocate(state.dataDemandManager->NumDemandManagerList);

        for (int ListNum = 1; ListNum <= state.dataDemandManager->NumDemandManagerList; ++ListNum) {

            auto &thisDemandMgrList = state.dataDemandManager->DemandManagerList(ListNum);

            s_ip->getObjectItem(state,
                                cCurrentModuleObject,
                                ListNum,
                                s_ipsc->cAlphaArgs,
                                NumAlphas,
                                s_ipsc->rNumericArgs,
                                NumNums,
                                IOStat,
                                _,
                                s_ipsc->lAlphaFieldBlanks,
                                s_ipsc->cAlphaFieldNames,
                                s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

            thisDemandMgrList.Name = s_ipsc->cAlphaArgs(1);

            thisDemandMgrList.Meter = GetMeterIndex(state, s_ipsc->cAlphaArgs(2));

            if (thisDemandMgrList.Meter == -1) {
                ShowSevereError(state, format("Invalid {} = {}", s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2)));
                ShowContinueError(state, format("Entered in {} = {}", cCurrentModuleObject, thisDemandMgrList.Name));
                ErrorsFound = true;

            } else if ((state.dataOutputProcessor->meters[thisDemandMgrList.Meter]->resource == Constant::eResource::Electricity) ||
                       (state.dataOutputProcessor->meters[thisDemandMgrList.Meter]->resource == Constant::eResource::ElectricityNet)) {
            } else {
                ShowSevereError(state,
                                format("{} = \"{}\" invalid value {} = \"{}\".",
                                       cCurrentModuleObject,
                                       thisDemandMgrList.Name,
                                       s_ipsc->cAlphaFieldNames(2),
                                       s_ipsc->cAlphaArgs(2)));
                ShowContinueError(state, "Only Electricity and ElectricityNet meters are currently allowed.");
                ErrorsFound = true;
            }

            // Further checking for conflicting DEMAND MANAGER LISTs

            if (s_ipsc->lAlphaFieldBlanks(3)) {
                ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(3));
                ErrorsFound = true;
            } else if ((thisDemandMgrList.limitSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(3))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
                ErrorsFound = true;
            }

            thisDemandMgrList.SafetyFraction = s_ipsc->rNumericArgs(1);

            if (s_ipsc->lAlphaFieldBlanks(4)) {
            } else if ((thisDemandMgrList.billingSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(4))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4));
                ErrorsFound = true;
            }

            if (s_ipsc->lAlphaFieldBlanks(5)) {
            } else if ((thisDemandMgrList.peakSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(5))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5));
                ErrorsFound = true;
            }

            thisDemandMgrList.AveragingWindow = max(int(s_ipsc->rNumericArgs(2) / state.dataGlobal->MinutesInTimeStep), 1);
            // Round to nearest timestep
            // Can make this fancier to include windows that do not fit the timesteps
            thisDemandMgrList.History.allocate(thisDemandMgrList.AveragingWindow);
            thisDemandMgrList.History = 0.0;

            // Validate Demand Manager Priority
            thisDemandMgrList.ManagerPriority =
                static_cast<ManagePriorityType>(getEnumValue(ManagePriorityNamesUC, Util::makeUPPER(s_ipsc->cAlphaArgs(6))));
            ErrorsFound = ErrorsFound || (thisDemandMgrList.ManagerPriority == ManagePriorityType::Invalid);

            // Get DEMAND MANAGER Type and Name pairs
            thisDemandMgrList.NumOfManager = int((NumAlphas - 6) / 2.0);

            if (thisDemandMgrList.NumOfManager > 0) {
                thisDemandMgrList.Manager.allocate(thisDemandMgrList.NumOfManager);
                for (int MgrNum = 1; MgrNum <= thisDemandMgrList.NumOfManager; ++MgrNum) {

                    auto &thisManager = thisDemandMgrList.Manager(MgrNum);
                    // Validate DEMAND MANAGER Type
                    ManagerType MgrType = static_cast<ManagerType>(getEnumValue(ManagerNamesUC, Util::makeUPPER(s_ipsc->cAlphaArgs(MgrNum * 2 + 5))));
                    if (MgrType != ManagerType::Invalid) {
                        thisManager = Util::FindItemInList(s_ipsc->cAlphaArgs(MgrNum * 2 + 6), state.dataDemandManager->DemandMgr);
                        if (thisManager == 0) {
                            ShowSevereError(state,
                                            format("{} = \"{}\" invalid {} = \"{}\" not found.",
                                                   cCurrentModuleObject,
                                                   thisDemandMgrList.Name,
                                                   s_ipsc->cAlphaFieldNames(MgrNum * 2 + 6),
                                                   s_ipsc->cAlphaArgs(MgrNum * 2 + 6)));
                            ErrorsFound = true;
                        }
                    } else {
                        ShowSevereError(state,
                                        format("{} = \"{}\" invalid value {} = \"{}\".",
                                               cCurrentModuleObject,
                                               thisDemandMgrList.Name,
                                               s_ipsc->cAlphaFieldNames(MgrNum * 2 + 5),
                                               s_ipsc->cAlphaArgs(MgrNum * 2 + 5)));
                        ErrorsFound = true;
                    }

                    // Check that each is not already referenced using %DemandManagerList field

                } // MgrNum
            }

            // Setup report variables
            SetupOutputVariable(state,
                                "Demand Manager Meter Demand Power",
                                Constant::Units::W,
                                thisDemandMgrList.MeterDemand,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                thisDemandMgrList.Name);

            SetupOutputVariable(state,
                                "Demand Manager Average Demand Power",
                                Constant::Units::W,
                                thisDemandMgrList.AverageDemand,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                thisDemandMgrList.Name);

            SetupOutputVariable(state,
                                "Demand Manager Peak Demand Power",
                                Constant::Units::W,
                                thisDemandMgrList.PeakDemand,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                thisDemandMgrList.Name);

            SetupOutputVariable(state,
                                "Demand Manager Scheduled Limit Power",
                                Constant::Units::W,
                                thisDemandMgrList.ScheduledLimit,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                thisDemandMgrList.Name);

            SetupOutputVariable(state,
                                "Demand Manager Demand Limit Power",
                                Constant::Units::W,
                                thisDemandMgrList.DemandLimit,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                thisDemandMgrList.Name);

            SetupOutputVariable(state,
                                "Demand Manager Over Limit Power",
                                Constant::Units::W,
                                thisDemandMgrList.OverLimit,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                thisDemandMgrList.Name);

            SetupOutputVariable(state,
                                "Demand Manager Over Limit Time",
                                Constant::Units::hr,
                                thisDemandMgrList.OverLimitDuration,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                thisDemandMgrList.Name);

            if (ErrorsFound) {
                ShowFatalError(state, format("Errors found in processing input for {}.", cCurrentModuleObject));
            }

        } // ListNum

        // Iteration diagnostic reporting for all DEMAND MANAGER LISTs
        SetupOutputVariable(state,
                            "Demand Manager Exterior Energy Iteration Count",
                            Constant::Units::None,
                            state.dataDemandManager->DemandManagerExtIterations,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            "ManageDemand");

        SetupOutputVariable(state,
                            "Demand Manager Heat Balance Iteration Count",
                            Constant::Units::None,
                            state.dataDemandManager->DemandManagerHBIterations,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            "ManageDemand");

        SetupOutputVariable(state,
                            "Demand Manager HVAC Iteration Count",
                            Constant::Units::None,
                            state.dataDemandManager->DemandManagerHVACIterations,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            "ManageDemand");
    }
}

void GetDemandManagerInput(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   July 2005
    //       MODIFIED       MODIFIED       Simon Vidanovic (March 2015) - Introduced DemandManager:Ventilation

    // PURPOSE OF THIS SUBROUTINE:
    // Gets the DEMAND MANAGER input from the input file.

    static constexpr std::string_view routineName = "GetDemandManagerInput";

    auto &s_ipsc = state.dataIPShortCut;
    auto const &s_ip = state.dataInputProcessing->inputProcessor;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int NumAlphas;            // Number of elements in the alpha array
    int NumNums;              // Number of elements in the numeric array
    int NumParams;            // Number of arguments total in an ObjectDef
    Array1D_string AlphArray; // Character string data
    Array1D<Real64> NumArray; // Numeric data
    bool ErrorsFound(false);

    int MaxAlphas = 0;
    int MaxNums = 0;
    std::string CurrentModuleObject = "DemandManager:ExteriorLights";
    int NumDemandMgrExtLights = s_ip->getNumObjectsFound(state, CurrentModuleObject);
    if (NumDemandMgrExtLights > 0) {
        s_ip->getObjectDefMaxArgs(state, CurrentModuleObject, NumParams, NumAlphas, NumNums);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        MaxNums = max(MaxNums, NumNums);
    }
    CurrentModuleObject = "DemandManager:Lights";
    int NumDemandMgrLights = s_ip->getNumObjectsFound(state, CurrentModuleObject);
    if (NumDemandMgrLights > 0) {
        s_ip->getObjectDefMaxArgs(state, CurrentModuleObject, NumParams, NumAlphas, NumNums);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        MaxNums = max(MaxNums, NumNums);
    }
    CurrentModuleObject = "DemandManager:ElectricEquipment";
    int NumDemandMgrElecEquip = s_ip->getNumObjectsFound(state, CurrentModuleObject);
    if (NumDemandMgrElecEquip > 0) {
        s_ip->getObjectDefMaxArgs(state, CurrentModuleObject, NumParams, NumAlphas, NumNums);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        MaxNums = max(MaxNums, NumNums);
    }
    CurrentModuleObject = "DemandManager:Thermostats";
    int NumDemandMgrThermostats = s_ip->getNumObjectsFound(state, CurrentModuleObject);
    if (NumDemandMgrThermostats > 0) {
        s_ip->getObjectDefMaxArgs(state, CurrentModuleObject, NumParams, NumAlphas, NumNums);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        MaxNums = max(MaxNums, NumNums);
    }
    CurrentModuleObject = "DemandManager:Ventilation";
    int NumDemandMgrVentilation = s_ip->getNumObjectsFound(state, CurrentModuleObject);
    if (NumDemandMgrVentilation > 0) {
        s_ip->getObjectDefMaxArgs(state, CurrentModuleObject, NumParams, NumAlphas, NumNums);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        MaxNums = max(MaxNums, NumNums);
    }

    state.dataDemandManager->NumDemandMgr =
        NumDemandMgrExtLights + NumDemandMgrLights + NumDemandMgrElecEquip + NumDemandMgrThermostats + NumDemandMgrVentilation;

    auto &DemandMgr(state.dataDemandManager->DemandMgr);

    if (state.dataDemandManager->NumDemandMgr > 0) {
        AlphArray.dimension(MaxAlphas, std::string());
        NumArray.dimension(MaxNums, 0.0);
        int IOStat; // IO Status when calling get input subroutine

        DemandMgr.allocate(state.dataDemandManager->NumDemandMgr);
        state.dataDemandManager->UniqueDemandMgrNames.reserve(state.dataDemandManager->NumDemandMgr);

        // Get input for DemandManager:ExteriorLights
        int StartIndex = 1;
        int EndIndex = NumDemandMgrExtLights;

        CurrentModuleObject = "DemandManager:ExteriorLights";

        for (int MgrNum = StartIndex; MgrNum <= EndIndex; ++MgrNum) {
            auto &demandMgr = DemandMgr(MgrNum);

            s_ip->getObjectItem(state,
                                CurrentModuleObject,
                                MgrNum - StartIndex + 1,
                                AlphArray,
                                NumAlphas,
                                NumArray,
                                NumNums,
                                IOStat,
                                _,
                                s_ipsc->lAlphaFieldBlanks,
                                s_ipsc->cAlphaFieldNames,
                                s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, AlphArray(1)};

            GlobalNames::VerifyUniqueInterObjectName(
                state, state.dataDemandManager->UniqueDemandMgrNames, AlphArray(1), CurrentModuleObject, s_ipsc->cAlphaFieldNames(1), ErrorsFound);
            demandMgr.Name = AlphArray(1);

            demandMgr.Type = ManagerType::ExtLights;

            if (s_ipsc->lAlphaFieldBlanks(2)) {
                demandMgr.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((demandMgr.availSched = Sched::GetSchedule(state, AlphArray(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), AlphArray(2));
                ErrorsFound = true;
            }

            // Validate Limiting Control
            demandMgr.LimitControl = static_cast<ManagerLimit>(getEnumValue(ManagerLimitNamesUC, Util::makeUPPER(AlphArray(3))));
            ErrorsFound = ErrorsFound || (demandMgr.LimitControl == ManagerLimit::Invalid);

            if (NumArray(1) == 0.0) {
                demandMgr.LimitDuration = state.dataGlobal->MinutesInTimeStep;
            } else {
                demandMgr.LimitDuration = NumArray(1);
            }

            demandMgr.LowerLimit = NumArray(2);

            // Validate Selection Control
            demandMgr.SelectionControl = static_cast<ManagerSelection>(getEnumValue(ManagerSelectionNamesUC, Util::makeUPPER(AlphArray(4))));
            ErrorsFound = ErrorsFound || (demandMgr.SelectionControl == ManagerSelection::Invalid);

            if (NumArray(4) == 0.0) {
                demandMgr.RotationDuration = state.dataGlobal->MinutesInTimeStep;
            } else {
                demandMgr.RotationDuration = NumArray(4);
            }

            demandMgr.NumOfLoads = NumAlphas - 4;

            if (demandMgr.NumOfLoads > 0) {
                demandMgr.Load.allocate(demandMgr.NumOfLoads);

                for (int LoadNum = 1; LoadNum <= demandMgr.NumOfLoads; ++LoadNum) {
                    int LoadPtr = Util::FindItemInList(AlphArray(LoadNum + 4), state.dataExteriorEnergyUse->ExteriorLights);

                    if (LoadPtr > 0) {
                        demandMgr.Load(LoadNum) = LoadPtr;

                    } else {
                        ShowSevereError(state,
                                        format("{}=\"{}\" invalid {}=\"{}\" not found.",
                                               CurrentModuleObject,
                                               s_ipsc->cAlphaArgs(1),
                                               s_ipsc->cAlphaFieldNames(LoadNum + 4),
                                               AlphArray(LoadNum + 4)));
                        ErrorsFound = true;
                    }
                } // LoadNum
            } else {
                ShowSevereError(state, format("{}=\"{}\" invalid value for number of loads.", CurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "Number of loads is calculated to be less than one. Demand manager must have at least one load assigned.");
                ErrorsFound = true;
            }

        } // MgrNum

        // Get input for DemandManager:Lights
        StartIndex = EndIndex + 1;
        EndIndex += NumDemandMgrLights;

        CurrentModuleObject = "DemandManager:Lights";

        for (int MgrNum = StartIndex; MgrNum <= EndIndex; ++MgrNum) {
            auto &demandMgr = DemandMgr(MgrNum);

            s_ip->getObjectItem(state,
                                CurrentModuleObject,
                                MgrNum - StartIndex + 1,
                                AlphArray,
                                NumAlphas,
                                NumArray,
                                NumNums,
                                IOStat,
                                _,
                                s_ipsc->lAlphaFieldBlanks,
                                s_ipsc->cAlphaFieldNames,
                                s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, AlphArray(1)};
            GlobalNames::VerifyUniqueInterObjectName(
                state, state.dataDemandManager->UniqueDemandMgrNames, AlphArray(1), CurrentModuleObject, s_ipsc->cAlphaFieldNames(1), ErrorsFound);
            demandMgr.Name = AlphArray(1);

            demandMgr.Type = ManagerType::Lights;

            if (s_ipsc->lAlphaFieldBlanks(2)) {
                demandMgr.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((demandMgr.availSched = Sched::GetSchedule(state, AlphArray(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), AlphArray(2));
                ErrorsFound = true;
            }

            // Validate Limiting Control
            demandMgr.LimitControl = static_cast<ManagerLimit>(getEnumValue(ManagerLimitNamesUC, Util::makeUPPER(AlphArray(3))));
            ErrorsFound = ErrorsFound || (demandMgr.LimitControl == ManagerLimit::Invalid);

            if (NumArray(1) == 0.0) {
                demandMgr.LimitDuration = state.dataGlobal->MinutesInTimeStep;
            } else {
                demandMgr.LimitDuration = NumArray(1);
            }

            demandMgr.LowerLimit = NumArray(2);

            // Validate Selection Control
            demandMgr.SelectionControl = static_cast<ManagerSelection>(getEnumValue(ManagerSelectionNamesUC, Util::makeUPPER(AlphArray(4))));
            ErrorsFound = ErrorsFound || (demandMgr.SelectionControl == ManagerSelection::Invalid);

            if (NumArray(4) == 0.0) {
                demandMgr.RotationDuration = state.dataGlobal->MinutesInTimeStep;
            } else {
                demandMgr.RotationDuration = NumArray(4);
            }

            // Count actual pointers to controlled zones
            demandMgr.NumOfLoads = 0;
            for (int LoadNum = 1; LoadNum <= NumAlphas - 4; ++LoadNum) {
                int LoadPtr = Util::FindItemInList(AlphArray(LoadNum + 4), state.dataInternalHeatGains->lightsObjects);
                if (LoadPtr > 0) {
                    demandMgr.NumOfLoads += state.dataInternalHeatGains->lightsObjects(LoadPtr).numOfSpaces;
                } else {
                    LoadPtr = Util::FindItemInList(AlphArray(LoadNum + 4), state.dataHeatBal->Lights);
                    if (LoadPtr > 0) {
                        ++demandMgr.NumOfLoads;
                    } else {
                        ShowSevereError(state,
                                        format("{}=\"{}\" invalid {}=\"{}\" not found.",
                                               CurrentModuleObject,
                                               s_ipsc->cAlphaArgs(1),
                                               s_ipsc->cAlphaFieldNames(LoadNum + 4),
                                               AlphArray(LoadNum + 4)));
                        ErrorsFound = true;
                    }
                }
            }

            //      demandMgr%NumOfLoads = NumAlphas - 4

            if (demandMgr.NumOfLoads > 0) {
                demandMgr.Load.allocate(demandMgr.NumOfLoads);
                int LoadNum = 0;
                for (int Item = 1; Item <= NumAlphas - 4; ++Item) {
                    int LoadPtr = Util::FindItemInList(AlphArray(Item + 4), state.dataInternalHeatGains->lightsObjects);
                    if (LoadPtr > 0) {
                        for (int Item1 = 1; Item1 <= state.dataInternalHeatGains->lightsObjects(LoadPtr).numOfSpaces; ++Item1) {
                            ++LoadNum;
                            demandMgr.Load(LoadNum) = state.dataInternalHeatGains->lightsObjects(LoadPtr).spaceStartPtr + Item1 - 1;
                        }
                    } else {
                        LoadPtr = Util::FindItemInList(AlphArray(Item + 4), state.dataHeatBal->Lights);
                        if (LoadPtr > 0) {
                            ++LoadNum;
                            demandMgr.Load(LoadNum) = LoadPtr;
                        }
                    }
                } // LoadNum
            } else {
                ShowSevereError(state, format("{}=\"{}\" invalid value for number of loads.", CurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "Number of loads is calculated to be less than one. Demand manager must have at least one load assigned.");
                ErrorsFound = true;
            }

        } // MgrNum

        // Get input for DemandManager:ElectricEquipment
        StartIndex = EndIndex + 1;
        EndIndex += NumDemandMgrElecEquip;

        CurrentModuleObject = "DemandManager:ElectricEquipment";

        for (int MgrNum = StartIndex; MgrNum <= EndIndex; ++MgrNum) {
            auto &demandMgr = DemandMgr(MgrNum);

            s_ip->getObjectItem(state,
                                CurrentModuleObject,
                                MgrNum - StartIndex + 1,
                                AlphArray,
                                NumAlphas,
                                NumArray,
                                NumNums,
                                IOStat,
                                _,
                                s_ipsc->lAlphaFieldBlanks,
                                s_ipsc->cAlphaFieldNames,
                                s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, AlphArray(1)};
            GlobalNames::VerifyUniqueInterObjectName(
                state, state.dataDemandManager->UniqueDemandMgrNames, AlphArray(1), CurrentModuleObject, s_ipsc->cAlphaFieldNames(1), ErrorsFound);

            demandMgr.Name = AlphArray(1);

            demandMgr.Type = ManagerType::ElecEquip;

            if (s_ipsc->lAlphaFieldBlanks(2)) {
                demandMgr.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((demandMgr.availSched = Sched::GetSchedule(state, AlphArray(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), AlphArray(2));
                ErrorsFound = true;
            }

            // Validate Limiting Control
            demandMgr.LimitControl = static_cast<ManagerLimit>(getEnumValue(ManagerLimitNamesUC, Util::makeUPPER(AlphArray(3))));
            ErrorsFound = ErrorsFound || (demandMgr.LimitControl == ManagerLimit::Invalid);

            if (NumArray(1) == 0.0) {
                demandMgr.LimitDuration = state.dataGlobal->MinutesInTimeStep;
            } else {
                demandMgr.LimitDuration = NumArray(1);
            }

            demandMgr.LowerLimit = NumArray(2);

            // Validate Selection Control
            demandMgr.SelectionControl = static_cast<ManagerSelection>(getEnumValue(ManagerSelectionNamesUC, Util::makeUPPER(AlphArray(4))));
            ErrorsFound = ErrorsFound || (demandMgr.SelectionControl == ManagerSelection::Invalid);

            if (NumArray(4) == 0.0) {
                demandMgr.RotationDuration = state.dataGlobal->MinutesInTimeStep;
            } else {
                demandMgr.RotationDuration = NumArray(4);
            }

            // Count actual pointers to controlled zones
            demandMgr.NumOfLoads = 0;
            for (int LoadNum = 1; LoadNum <= NumAlphas - 4; ++LoadNum) {
                int LoadPtr = Util::FindItemInList(AlphArray(LoadNum + 4), state.dataInternalHeatGains->zoneElectricObjects);
                if (LoadPtr > 0) {
                    demandMgr.NumOfLoads += state.dataInternalHeatGains->zoneElectricObjects(LoadPtr).numOfSpaces;
                } else {
                    LoadPtr = Util::FindItemInList(AlphArray(LoadNum + 4), state.dataHeatBal->ZoneElectric);
                    if (LoadPtr > 0) {
                        ++demandMgr.NumOfLoads;
                    } else {
                        ShowSevereError(state,
                                        format("{}=\"{}\" invalid {}=\"{}\" not found.",
                                               CurrentModuleObject,
                                               s_ipsc->cAlphaArgs(1),
                                               s_ipsc->cAlphaFieldNames(LoadNum + 4),
                                               AlphArray(LoadNum + 4)));
                        ErrorsFound = true;
                    }
                }
            }

            //      demandMgr%NumOfLoads = NumAlphas - 4

            if (demandMgr.NumOfLoads > 0) {
                demandMgr.Load.allocate(demandMgr.NumOfLoads);
                int LoadNum = 0;
                for (int Item = 1; Item <= NumAlphas - 4; ++Item) {
                    int LoadPtr = Util::FindItemInList(AlphArray(Item + 4), state.dataInternalHeatGains->zoneElectricObjects);
                    if (LoadPtr > 0) {
                        for (int Item1 = 1; Item1 <= state.dataInternalHeatGains->zoneElectricObjects(LoadPtr).numOfSpaces; ++Item1) {
                            ++LoadNum;
                            demandMgr.Load(LoadNum) = state.dataInternalHeatGains->zoneElectricObjects(LoadPtr).spaceStartPtr + Item1 - 1;
                        }
                    } else {
                        LoadPtr = Util::FindItemInList(AlphArray(Item + 4), state.dataHeatBal->ZoneElectric);
                        if (LoadPtr > 0) {
                            ++LoadNum;
                            demandMgr.Load(LoadNum) = LoadPtr;
                        }
                    }
                } // LoadNum
            } else {
                ShowSevereError(state, format("{}=\"{}\" invalid value for number of loads.", CurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "Number of loads is calculated to be less than one. Demand manager must have at least one load assigned.");
                ErrorsFound = true;
            }

        } // MgrNum

        // Get input for DemandManager:Thermostats
        StartIndex = EndIndex + 1;
        EndIndex += NumDemandMgrThermostats;

        CurrentModuleObject = "DemandManager:Thermostats";

        for (int MgrNum = StartIndex; MgrNum <= EndIndex; ++MgrNum) {
            auto &demandMgr = DemandMgr(MgrNum);

            s_ip->getObjectItem(state,
                                CurrentModuleObject,
                                MgrNum - StartIndex + 1,
                                AlphArray,
                                NumAlphas,
                                NumArray,
                                NumNums,
                                IOStat,
                                _,
                                s_ipsc->lAlphaFieldBlanks,
                                s_ipsc->cAlphaFieldNames,
                                s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, AlphArray(1)};

            GlobalNames::VerifyUniqueInterObjectName(
                state, state.dataDemandManager->UniqueDemandMgrNames, AlphArray(1), CurrentModuleObject, s_ipsc->cAlphaFieldNames(1), ErrorsFound);
            demandMgr.Name = AlphArray(1);

            demandMgr.Type = ManagerType::Thermostats;

            if (s_ipsc->lAlphaFieldBlanks(2)) {
                demandMgr.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((demandMgr.availSched = Sched::GetSchedule(state, AlphArray(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), AlphArray(2));
                ErrorsFound = true;
            }

            // Validate Limiting Control
            demandMgr.LimitControl = static_cast<ManagerLimit>(getEnumValue(ManagerLimitNamesUC, Util::makeUPPER(AlphArray(3))));
            ErrorsFound = ErrorsFound || (demandMgr.LimitControl == ManagerLimit::Invalid);

            if (NumArray(1) == 0.0) {
                demandMgr.LimitDuration = state.dataGlobal->MinutesInTimeStep;
            } else {
                demandMgr.LimitDuration = NumArray(1);
            }

            demandMgr.LowerLimit = NumArray(2);
            demandMgr.UpperLimit = NumArray(3);

            if (demandMgr.LowerLimit > demandMgr.UpperLimit) {
                ShowSevereError(state, format("Invalid input for {} = {}", CurrentModuleObject, AlphArray(1)));
                ShowContinueError(
                    state,
                    format("{} [{:.2R}] > {} [{:.2R}]", s_ipsc->cNumericFieldNames(2), NumArray(2), s_ipsc->cNumericFieldNames(3), NumArray(3)));
                ShowContinueError(state, format("{} cannot be greater than {}", s_ipsc->cNumericFieldNames(2), s_ipsc->cNumericFieldNames(3)));
                ErrorsFound = true;
            }

            // Validate Selection Control
            demandMgr.SelectionControl = static_cast<ManagerSelection>(getEnumValue(ManagerSelectionNamesUC, Util::makeUPPER(AlphArray(4))));
            ErrorsFound = ErrorsFound || (demandMgr.SelectionControl == ManagerSelection::Invalid);

            if (NumArray(5) == 0.0) {
                demandMgr.RotationDuration = state.dataGlobal->MinutesInTimeStep;
            } else {
                demandMgr.RotationDuration = NumArray(5);
            }

            // Count actual pointers to controlled zones
            demandMgr.NumOfLoads = 0;
            for (int LoadNum = 1; LoadNum <= NumAlphas - 4; ++LoadNum) {
                int LoadPtr = Util::FindItemInList(AlphArray(LoadNum + 4), state.dataZoneCtrls->TStatObjects);
                if (LoadPtr > 0) {
                    demandMgr.NumOfLoads += state.dataZoneCtrls->TStatObjects(LoadPtr).NumOfZones;
                } else {
                    LoadPtr = Util::FindItemInList(AlphArray(LoadNum + 4), state.dataZoneCtrls->TempControlledZone);
                    if (LoadPtr > 0) {
                        ++demandMgr.NumOfLoads;
                    } else {
                        ShowSevereError(state,
                                        format("{}=\"{}\" invalid {}=\"{}\" not found.",
                                               CurrentModuleObject,
                                               s_ipsc->cAlphaArgs(1),
                                               s_ipsc->cAlphaFieldNames(LoadNum + 4),
                                               AlphArray(LoadNum + 4)));
                        ErrorsFound = true;
                    }
                }
            }

            if (demandMgr.NumOfLoads > 0) {
                demandMgr.Load.allocate(demandMgr.NumOfLoads);
                int LoadNum = 0;
                for (int Item = 1; Item <= NumAlphas - 4; ++Item) {
                    int LoadPtr = Util::FindItemInList(AlphArray(Item + 4), state.dataZoneCtrls->TStatObjects);
                    if (LoadPtr > 0) {
                        for (int Item1 = 1; Item1 <= state.dataZoneCtrls->TStatObjects(LoadPtr).NumOfZones; ++Item1) {
                            ++LoadNum;
                            demandMgr.Load(LoadNum) = state.dataZoneCtrls->TStatObjects(LoadPtr).TempControlledZoneStartPtr + Item1 - 1;
                        }
                    } else {
                        LoadPtr = Util::FindItemInList(AlphArray(Item + 4), state.dataZoneCtrls->TempControlledZone);
                        if (LoadPtr > 0) {
                            ++LoadNum;
                            demandMgr.Load(LoadNum) = LoadPtr;
                        }
                    }
                } // LoadNum
            } else {
                ShowSevereError(state, format("{}=\"{}\" invalid value for number of loads.", CurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "Number of loads is calculated to be less than one. Demand manager must have at least one load assigned.");
                ErrorsFound = true;
            }
        } // MgrNum

        // Get input for DemandManager:Ventilation
        StartIndex = EndIndex + 1;
        EndIndex += NumDemandMgrVentilation;

        CurrentModuleObject = "DemandManager:Ventilation";

        for (int MgrNum = StartIndex; MgrNum <= EndIndex; ++MgrNum) {
            auto &demandMgr = DemandMgr(MgrNum);

            s_ip->getObjectItem(state,
                                CurrentModuleObject,
                                MgrNum - StartIndex + 1,
                                AlphArray,
                                NumAlphas,
                                NumArray,
                                NumNums,
                                IOStat,
                                _,
                                s_ipsc->lAlphaFieldBlanks,
                                s_ipsc->cAlphaFieldNames,
                                s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, AlphArray(1)};
            GlobalNames::VerifyUniqueInterObjectName(
                state, state.dataDemandManager->UniqueDemandMgrNames, AlphArray(1), CurrentModuleObject, s_ipsc->cAlphaFieldNames(1), ErrorsFound);
            demandMgr.Name = AlphArray(1);

            demandMgr.Type = ManagerType::Ventilation;

            if (s_ipsc->lAlphaFieldBlanks(2)) {
                demandMgr.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((demandMgr.availSched = Sched::GetSchedule(state, AlphArray(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), AlphArray(2));
                ErrorsFound = true;
            }

            // Validate Limiting Control
            demandMgr.LimitControl = static_cast<ManagerLimit>(getEnumValue(ManagerLimitVentNamesUC, Util::makeUPPER(AlphArray(3))));
            ErrorsFound = ErrorsFound || (demandMgr.LimitControl == ManagerLimit::Invalid);

            demandMgr.LimitDuration = (NumArray(1) == 0.0) ? state.dataGlobal->MinutesInTimeStep : NumArray(1);

            if (demandMgr.LimitControl == ManagerLimit::Fixed) {
                demandMgr.FixedRate = NumArray(2);
            }
            if (demandMgr.LimitControl == ManagerLimit::ReductionRatio) {
                demandMgr.ReductionRatio = NumArray(3);
            }

            demandMgr.LowerLimit = NumArray(4);

            // Validate Selection Control
            demandMgr.SelectionControl = static_cast<ManagerSelection>(getEnumValue(ManagerSelectionNamesUC, Util::makeUPPER(AlphArray(4))));
            ErrorsFound = ErrorsFound || (demandMgr.SelectionControl == ManagerSelection::Invalid);

            demandMgr.RotationDuration = (NumArray(5) == 0.0) ? state.dataGlobal->MinutesInTimeStep : NumArray(5);

            // Count number of string fields for loading Controller:OutdoorAir names. This number must be increased in case if
            // new string field is added or decreased if string fields are removed.
            int AlphaShift = 4;

            // Count actual pointers to air controllers
            demandMgr.NumOfLoads = 0;
            for (int LoadNum = 1; LoadNum <= NumAlphas - AlphaShift; ++LoadNum) {
                int LoadPtr = MixedAir::GetOAController(state, AlphArray(LoadNum + AlphaShift));
                if (LoadPtr > 0) {
                    ++demandMgr.NumOfLoads;
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {}=\"{}\" not found.",
                                           CurrentModuleObject,
                                           s_ipsc->cAlphaArgs(1),
                                           s_ipsc->cAlphaFieldNames(LoadNum + AlphaShift),
                                           AlphArray(LoadNum + AlphaShift)));
                    ErrorsFound = true;
                }
            }

            if (demandMgr.NumOfLoads > 0) {
                demandMgr.Load.allocate(demandMgr.NumOfLoads);
                for (int LoadNum = 1; LoadNum <= NumAlphas - AlphaShift; ++LoadNum) {
                    int LoadPtr = MixedAir::GetOAController(state, AlphArray(LoadNum + AlphaShift));
                    if (LoadPtr > 0) {
                        demandMgr.Load(LoadNum) = LoadPtr;
                    }
                }
            } else {
                ShowSevereError(state, format("{}=\"{}\" invalid value for number of loads.", CurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "Number of loads is calculated to be less than one. Demand manager must have at least one load assigned.");
                ErrorsFound = true;
            }
        } // MgrNum

        AlphArray.deallocate();
        NumArray.deallocate();
    }

    if (ErrorsFound) {
        ShowFatalError(state, "Errors found in processing input for demand managers. Preceding condition causes termination.");
    }
}

void SurveyDemandManagers(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   July 2005

    // PURPOSE OF THIS SUBROUTINE:
    // Checks to see if any demand managers can reduce the load

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool CanReduceDemand;

    for (int MgrNum = 1; MgrNum <= state.dataDemandManager->NumDemandMgr; ++MgrNum) {

        auto &demandMgr = state.dataDemandManager->DemandMgr(MgrNum);

        demandMgr.CanReduceDemand = false;

        if (!demandMgr.Available) {
            continue;
        }
        if (demandMgr.LimitControl == ManagerLimit::Off) {
            continue;
        }

        if (demandMgr.Active) {
            continue; // This works for FIXED control action, but not VARIABLE
        }
        // VARIABLE control could actually reduce demand farther, even if active already

        for (int LoadNum = 1; LoadNum <= demandMgr.NumOfLoads; ++LoadNum) {
            int LoadPtr = demandMgr.Load(LoadNum);

            // Check if this load can reduce demand
            // Assume FIXED control action for now, needs more sophisticated check for VARIABLE control
            LoadInterface(state, DemandAction::CheckCanReduce, MgrNum, LoadPtr, CanReduceDemand);

            if (CanReduceDemand) {
                demandMgr.CanReduceDemand = true;
                break; // If any one load can reduce demand, then the whole demand manager can reduce demand
            }

        } // LoadNum

    } // MgrNum
}

void ActivateDemandManagers(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   July 2005
    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int LoadPtr;

    for (int MgrNum = 1; MgrNum <= state.dataDemandManager->NumDemandMgr; ++MgrNum) {

        auto &demandMgr = state.dataDemandManager->DemandMgr(MgrNum);

        if (demandMgr.Activate) {
            bool CanReduceDemand;
            demandMgr.Activate = false;
            demandMgr.Active = true;

            switch (demandMgr.SelectionControl) {
            case ManagerSelection::All: {
                // Turn ON limiting on all loads
                for (int LoadNum = 1; LoadNum <= demandMgr.NumOfLoads; ++LoadNum) {
                    LoadPtr = demandMgr.Load(LoadNum);
                    LoadInterface(state, DemandAction::SetLimit, MgrNum, LoadPtr, CanReduceDemand);
                } // LoadNum

            } break;
            case ManagerSelection::Many: { // All loads are limited except for one
                if (demandMgr.NumOfLoads > 1) {

                    // Turn ON limiting on all loads
                    for (int LoadNum = 1; LoadNum <= demandMgr.NumOfLoads; ++LoadNum) {
                        LoadPtr = demandMgr.Load(LoadNum);
                        LoadInterface(state, DemandAction::SetLimit, MgrNum, LoadPtr, CanReduceDemand);
                    } // LoadNum

                    // Set next rotated load (from last time it was active)
                    int RotatedLoadNum = demandMgr.RotatedLoadNum;
                    ++RotatedLoadNum;
                    if (RotatedLoadNum > demandMgr.NumOfLoads) {
                        RotatedLoadNum = 1;
                    }
                    demandMgr.RotatedLoadNum = RotatedLoadNum;

                    // Turn OFF limiting for the new rotated load
                    LoadPtr = demandMgr.Load(RotatedLoadNum);
                    LoadInterface(state, DemandAction::ClearLimit, MgrNum, LoadPtr, CanReduceDemand);
                } else {
                    // Turn ON limiting for the one and only load
                    LoadPtr = demandMgr.Load(1);
                    LoadInterface(state, DemandAction::SetLimit, MgrNum, LoadPtr, CanReduceDemand);
                }

            } break;
            case ManagerSelection::One: { // Only one load is limited
                if (demandMgr.NumOfLoads > 1) {
                    // Turn OFF limiting on all loads
                    for (int LoadNum = 1; LoadNum <= demandMgr.NumOfLoads; ++LoadNum) {
                        LoadPtr = demandMgr.Load(LoadNum);
                        LoadInterface(state, DemandAction::ClearLimit, MgrNum, LoadPtr, CanReduceDemand);
                    } // LoadNum

                    // Set next rotated load (from last time it was active)
                    int RotatedLoadNum = demandMgr.RotatedLoadNum;
                    ++RotatedLoadNum;
                    if (RotatedLoadNum > demandMgr.NumOfLoads) {
                        RotatedLoadNum = 1;
                    }
                    demandMgr.RotatedLoadNum = RotatedLoadNum;

                    // Turn ON limiting for the new rotated load
                    LoadPtr = demandMgr.Load(RotatedLoadNum);
                    LoadInterface(state, DemandAction::SetLimit, MgrNum, LoadPtr, CanReduceDemand);
                } else {
                    // Turn ON limiting for the one and only load
                    LoadPtr = demandMgr.Load(1);
                    LoadInterface(state, DemandAction::SetLimit, MgrNum, LoadPtr, CanReduceDemand);
                }
            } break;
            default:
                break;
            }
        }

    } // MgrNum
}

void UpdateDemandManagers(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   July 2005

    // PURPOSE OF THIS SUBROUTINE:
    // Expires limits and rotates loads after specified time duration.
    // It updates availability flags, expires managers that ended in the last timestep, etc.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int LoadPtr;
    bool CanReduceDemand;
    int RotatedLoadNum;

    for (int MgrNum = 1; MgrNum <= state.dataDemandManager->NumDemandMgr; ++MgrNum) {

        auto &demandMgr = state.dataDemandManager->DemandMgr(MgrNum);

        // Check availability
        bool Available = demandMgr.availSched->getCurrentVal() > 0.0;

        demandMgr.Available = Available;

        // Update demand manager status
        if (Available) {

            if (demandMgr.Active) {

                demandMgr.ElapsedTime += state.dataGlobal->MinutesInTimeStep;

                // Check for expiring limit duration
                if (demandMgr.ElapsedTime >= demandMgr.LimitDuration) {
                    demandMgr.ElapsedTime = 0;
                    demandMgr.ElapsedRotationTime = 0;
                    demandMgr.Active = false;

                    // Demand Manager is not available, remove demand limits from all loads
                    for (int LoadNum = 1; LoadNum <= demandMgr.NumOfLoads; ++LoadNum) {
                        LoadPtr = demandMgr.Load(LoadNum);
                        LoadInterface(state, DemandAction::ClearLimit, MgrNum, LoadPtr, CanReduceDemand);
                    } // LoadNum

                } else {

                    switch (demandMgr.SelectionControl) {
                    case ManagerSelection::All: {
                        // Do nothing; limits remain on all loads

                    } break;
                    case ManagerSelection::Many: { // All loads are limited except for one
                        demandMgr.ElapsedRotationTime += state.dataGlobal->MinutesInTimeStep;

                        if (demandMgr.ElapsedRotationTime >= demandMgr.RotationDuration) {
                            demandMgr.ElapsedRotationTime = 0;

                            if (demandMgr.NumOfLoads > 1) {
                                // Turn ON limiting for the old rotated load
                                RotatedLoadNum = demandMgr.RotatedLoadNum;
                                LoadPtr = demandMgr.Load(RotatedLoadNum);
                                LoadInterface(state, DemandAction::SetLimit, MgrNum, LoadPtr, CanReduceDemand);

                                // Set next rotated load
                                ++RotatedLoadNum;
                                if (RotatedLoadNum > demandMgr.NumOfLoads) {
                                    RotatedLoadNum = 1;
                                }
                                demandMgr.RotatedLoadNum = RotatedLoadNum;

                                // Turn OFF limiting for the new rotated load
                                LoadPtr = demandMgr.Load(RotatedLoadNum);
                                LoadInterface(state, DemandAction::ClearLimit, MgrNum, LoadPtr, CanReduceDemand);
                            }
                        }

                    } break;
                    case ManagerSelection::One: { // Only one load is limited
                        demandMgr.ElapsedRotationTime += state.dataGlobal->MinutesInTimeStep;

                        if (demandMgr.ElapsedRotationTime >= demandMgr.RotationDuration) {
                            demandMgr.ElapsedRotationTime = 0;

                            if (demandMgr.NumOfLoads > 1) {
                                // Turn OFF limiting for the old rotated load
                                RotatedLoadNum = demandMgr.RotatedLoadNum;
                                LoadPtr = demandMgr.Load(RotatedLoadNum);
                                LoadInterface(state, DemandAction::ClearLimit, MgrNum, LoadPtr, CanReduceDemand);

                                // Set next rotated load
                                ++RotatedLoadNum;
                                if (RotatedLoadNum > demandMgr.NumOfLoads) {
                                    RotatedLoadNum = 1;
                                }
                                demandMgr.RotatedLoadNum = RotatedLoadNum;

                                // Turn ON limiting for the new rotated load
                                LoadPtr = demandMgr.Load(RotatedLoadNum);
                                LoadInterface(state, DemandAction::SetLimit, MgrNum, LoadPtr, CanReduceDemand);
                            }
                        }
                    } break;
                    default:
                        break;
                    }
                }
            }

        } else { // Demand Manager is not available
            demandMgr.Active = false;

            // Demand Manager is not available, remove demand limits from all loads
            for (int LoadNum = 1; LoadNum <= demandMgr.NumOfLoads; ++LoadNum) {
                LoadPtr = demandMgr.Load(LoadNum);
                LoadInterface(state, DemandAction::ClearLimit, MgrNum, LoadPtr, CanReduceDemand);
            } // LoadNum
        }

    } // MgrNum
}

void ReportDemandManagerList(EnergyPlusData &state, int const ListNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   July 2005

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates report variables.

    // METHODOLOGY EMPLOYED:
    // Standard EnergyPlus methodology.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int AveragingWindow;
    bool OnPeak;
    Real64 OverLimit;

    auto &demandManagerList = state.dataDemandManager->DemandManagerList(ListNum);

    Real64 BillingPeriod = (demandManagerList.billingSched == nullptr) ? state.dataEnvrn->Month : demandManagerList.billingSched->getCurrentVal();

    if (demandManagerList.BillingPeriod != BillingPeriod) {
        // Reset variables for new billing period
        // demandManagerList%History = 0.0        ! Don't reset--continue from previous billing period
        // demandManagerList%AverageDemand = 0.0  ! Don't reset--continue from previous billing period
        demandManagerList.PeakDemand = 0.0;
        demandManagerList.OverLimitDuration = 0.0;

        demandManagerList.BillingPeriod = BillingPeriod;
    }

    // Add new timestep to demand history and subtract oldest timestep
    AveragingWindow = demandManagerList.AveragingWindow;
    demandManagerList.AverageDemand += (demandManagerList.MeterDemand - demandManagerList.History(1)) / AveragingWindow;

    // Update demand history
    for (int Item = 1; Item <= AveragingWindow - 1; ++Item) {
        demandManagerList.History(Item) = demandManagerList.History(Item + 1);
    }
    demandManagerList.History(AveragingWindow) = demandManagerList.MeterDemand;

    OnPeak = (demandManagerList.peakSched == nullptr) || (demandManagerList.peakSched->getCurrentVal() == 1);

    if (OnPeak) {
        demandManagerList.PeakDemand = max(demandManagerList.AverageDemand, demandManagerList.PeakDemand);

        OverLimit = demandManagerList.AverageDemand - demandManagerList.ScheduledLimit;
        if (OverLimit > 0.0) {
            demandManagerList.OverLimit = OverLimit;
            demandManagerList.OverLimitDuration += (state.dataGlobal->MinutesInTimeStep / 60.0);
        } else {
            demandManagerList.OverLimit = 0.0;
        }

    } else {
        demandManagerList.OverLimit = 0.0;
    }
}

void LoadInterface(EnergyPlusData &state, DemandAction const Action, int const MgrNum, int const LoadPtr, bool &CanReduceDemand)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   August 2005

    // PURPOSE OF THIS SUBROUTINE:
    // Provides a universal interface to handle all communication with the various load objects.
    // Demand managers for new types of loads can be easily added with a new CASE statement in this subroutine
    // and new GetInput code.

    auto const &s_dhbf = state.dataHeatBalFanSys;
    auto const &demandMgr = state.dataDemandManager->DemandMgr(MgrNum);

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 LowestPower;

    CanReduceDemand = false;

    switch (demandMgr.Type) {
    case ManagerType::ExtLights: {
        LowestPower = state.dataExteriorEnergyUse->ExteriorLights(LoadPtr).DesignLevel * demandMgr.LowerLimit;
        if (Action == DemandAction::CheckCanReduce) {
            if (state.dataExteriorEnergyUse->ExteriorLights(LoadPtr).Power > LowestPower) {
                CanReduceDemand = true;
            }
        } else if (Action == DemandAction::SetLimit) {
            state.dataExteriorEnergyUse->ExteriorLights(LoadPtr).ManageDemand = true;
            state.dataExteriorEnergyUse->ExteriorLights(LoadPtr).DemandLimit = LowestPower;
        } else if (Action == DemandAction::ClearLimit) {
            state.dataExteriorEnergyUse->ExteriorLights(LoadPtr).ManageDemand = false;
        }
    } break;

    case ManagerType::Lights: {
        LowestPower = state.dataHeatBal->Lights(LoadPtr).DesignLevel * demandMgr.LowerLimit;
        if (Action == DemandAction::CheckCanReduce) {
            if (state.dataHeatBal->Lights(LoadPtr).Power > LowestPower) {
                CanReduceDemand = true;
            }
        } else if (Action == DemandAction::SetLimit) {
            state.dataHeatBal->Lights(LoadPtr).ManageDemand = true;
            state.dataHeatBal->Lights(LoadPtr).DemandLimit = LowestPower;
        } else if (Action == DemandAction::ClearLimit) {
            state.dataHeatBal->Lights(LoadPtr).ManageDemand = false;
        }
    } break;

    case ManagerType::ElecEquip: {
        LowestPower = state.dataHeatBal->ZoneElectric(LoadPtr).DesignLevel * demandMgr.LowerLimit;
        if (Action == DemandAction::CheckCanReduce) {
            if (state.dataHeatBal->ZoneElectric(LoadPtr).Power > LowestPower) {
                CanReduceDemand = true;
            }
        } else if (Action == DemandAction::SetLimit) {
            state.dataHeatBal->ZoneElectric(LoadPtr).ManageDemand = true;
            state.dataHeatBal->ZoneElectric(LoadPtr).DemandLimit = LowestPower;
        } else if (Action == DemandAction::ClearLimit) {
            state.dataHeatBal->ZoneElectric(LoadPtr).ManageDemand = false;
        }
    } break;

    case ManagerType::Thermostats: {
        auto &tempZone = state.dataZoneCtrls->TempControlledZone(LoadPtr);
        auto &zoneTstatSetpt = s_dhbf->zoneTstatSetpts(tempZone.ActualZoneNum);
        if (Action == DemandAction::CheckCanReduce) {
            if (zoneTstatSetpt.setptLo > demandMgr.LowerLimit || zoneTstatSetpt.setptHi < demandMgr.UpperLimit) {
                CanReduceDemand = true; // Heating | Cooling
            }
        } else if (Action == DemandAction::SetLimit) {
            tempZone.ManageDemand = true;
            tempZone.HeatingResetLimit = demandMgr.LowerLimit;
            tempZone.CoolingResetLimit = demandMgr.UpperLimit;
        } else if (Action == DemandAction::ClearLimit) {
            tempZone.ManageDemand = false;
        }
        if (state.dataZoneCtrls->NumComfortControlledZones > 0) {
            auto &comfortZone = state.dataZoneCtrls->ComfortControlledZone(LoadPtr);
            if (state.dataHeatBalFanSys->ComfortControlType(comfortZone.ActualZoneNum) != HVAC::SetptType::Uncontrolled) {
                auto &zoneTstatSetpt = s_dhbf->zoneTstatSetpts(comfortZone.ActualZoneNum);
                if (Action == DemandAction::CheckCanReduce) {
                    if (zoneTstatSetpt.setptLo > demandMgr.LowerLimit || zoneTstatSetpt.setptHi < demandMgr.UpperLimit) {
                        CanReduceDemand = true; // Heating
                    }
                } else if (Action == DemandAction::SetLimit) {
                    comfortZone.ManageDemand = true;
                    comfortZone.HeatingResetLimit = demandMgr.LowerLimit;
                    comfortZone.CoolingResetLimit = demandMgr.UpperLimit;
                } else if (Action == DemandAction::ClearLimit) {
                    comfortZone.ManageDemand = false;
                }
            }
        }
    } break;

    case ManagerType::Ventilation: {
        Real64 FlowRate(0);
        FlowRate = MixedAir::OAGetFlowRate(state, LoadPtr);
        if (Action == DemandAction::CheckCanReduce) {
            CanReduceDemand = true;
        } else if (Action == DemandAction::SetLimit) {
            MixedAir::OASetDemandManagerVentilationState(state, LoadPtr, true);
            if (demandMgr.LimitControl == ManagerLimit::Fixed) {
                MixedAir::OASetDemandManagerVentilationFlow(state, LoadPtr, demandMgr.FixedRate);
            } else if (demandMgr.LimitControl == ManagerLimit::ReductionRatio) {
                Real64 DemandRate(0);
                DemandRate = FlowRate * demandMgr.ReductionRatio;
                MixedAir::OASetDemandManagerVentilationFlow(state, LoadPtr, DemandRate);
            }
        } else if (Action == DemandAction::ClearLimit) {
            MixedAir::OASetDemandManagerVentilationState(state, LoadPtr, false);
        }
    } break;
    default:
        break;
    }
}

void InitDemandManagers(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   September 2010

    // PURPOSE OF THIS SUBROUTINE:
    // Provide external call to get Demand manager input after
    // appropriate initializations.

    if (state.dataDemandManager->GetInput) {
        GetDemandManagerInput(state);
        GetDemandManagerListInput(state);
        state.dataDemandManager->GetInput = false;
    }
}

} // namespace EnergyPlus::DemandManager
