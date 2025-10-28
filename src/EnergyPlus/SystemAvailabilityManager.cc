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
#include <algorithm>
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Array2D.hh>

// EnergyPlus Headers
#include <AirflowNetwork/Elements.hpp>
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataZoneControls.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SystemAvailabilityManager.hh>
#include <EnergyPlus/ThermalComfort.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace Avail {

    // Module containing the System Availability Manager routines

    // MODULE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   August 2001
    //       MODIFIED       February 2004, PGE: Added plant managers.
    //       MODIFIED       March 2007, LG: Added hybrid ventilation control.
    //                      August 2008, R. Raustad - FSEC: added 2 new scheduled sys avail managers
    //                      March 2011, Chandan Sharma - FSEC: Added zone sys avail managers
    //                      August 2013, Xiufeng Pang (XP) - added algorithms for optimal start
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE
    // To encapsulate the data and algorithms required to
    // determine system (loop) availability and "cycle on" status.

    // METHODOLOGY EMPLOYED:
    // Previous time step node data and current zone thermostat setpoints are used
    // in a set of fixed, precoded algorithms to determine the current time step
    // on/off status of systems and loops.

    // USE STATEMENTS:
    // Use statements for data only modules
    static constexpr std::array<std::string_view, (int)ManagerType::Num> managerTypeNamesUC = {"AVAILABILITYMANAGER:SCHEDULED",
                                                                                               "AVAILABILITYMANAGER:SCHEDULEDON",
                                                                                               "AVAILABILITYMANAGER:SCHEDULEDOFF",
                                                                                               "AVAILABILITYMANAGER:NIGHTCYCLE",
                                                                                               "AVAILABILITYMANAGER:DIFFERENTIALTHERMOSTAT",
                                                                                               "AVAILABILITYMANAGER:HIGHTEMPERATURETURNOFF",
                                                                                               "AVAILABILITYMANAGER:HIGHTEMPERATURETURNON",
                                                                                               "AVAILABILITYMANAGER:LOWTEMPERATURETURNOFF",
                                                                                               "AVAILABILITYMANAGER:LOWTEMPERATURETURNON",
                                                                                               "AVAILABILITYMANAGER:NIGHTVENTILATION",
                                                                                               "AVAILABILITYMANAGER:HYBRIDVENTILATION",
                                                                                               "AVAILABILITYMANAGER:OPTIMUMSTART"};

    static constexpr std::array<std::string_view, (int)ManagerType::Num> managerTypeNames = {"AvailabilityManager:Scheduled",
                                                                                             "AvailabilityManager:ScheduledOn",
                                                                                             "AvailabilityManager:ScheduledOff",
                                                                                             "AvailabilityManager:NightCycle",
                                                                                             "AvailabilityManager:DifferentialThermostat",
                                                                                             "AvailabilityManager:HighTemperatureTurnOff",
                                                                                             "AvailabilityManager:HighTemperatureTurnOn",
                                                                                             "AvailabilityManager:LowTemperatureTurnOff",
                                                                                             "AvailabilityManager:LowTemperatureTurnOn",
                                                                                             "AvailabilityManager:NightVentilation",
                                                                                             "AvailabilityManager:HybridVentilation",
                                                                                             "AvailabilityManager:OptimumStart"};

    void ManageSystemAvailability(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   August 2001
        //       MODIFIED       L. Gu, April, 2007. Added hybrid ventilation control
        //                      Chandan Sharma, March 2011/July 2012 - FSEC: Added zone sys avail managers
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Manage the simulation of the System Availability Managers

        using DataZoneEquipment::NumValidSysAvailZoneComponents;
        using namespace DataLoopNode;
        using namespace DataAirLoop;
        using namespace DataPlant;

        int PriAirSysNum;         // Primary Air System index
        int PriAirSysAvailMgrNum; // Index of Sys Avail Manager in a Primary Air System
        int PlantNum;             // Plant Loop index
        int PlantAvailMgrNum;     // Index of Plant Avail Manager in a Plant Loop
        Status availStatus;
        Status previousAvailStatus;
        int ZoneInSysNum;
        int CtrldZoneNum;
        int HybridVentNum;              // Hybrid ventilation control number
        int ZoneEquipType;              // Type of ZoneHVAC:* component
        int CompNum;                    // Index of ZoneHVAC:* component
        int ZoneCompAvailMgrNum;        // Index of availability manager associated with the ZoneHVAC:* component
        int constexpr DummyArgument(1); // This variable is used when SimSysAvailManager is called for a ZoneHVAC:* component

        if (state.dataAvail->GetAvailMgrInputFlag) {
            GetSysAvailManagerInputs(state);
            state.dataAvail->GetAvailMgrInputFlag = false;
            return;
        }

        InitSysAvailManagers(state);

        for (PriAirSysNum = 1; PriAirSysNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++PriAirSysNum) { // loop over the primary air systems
            auto &availMgr = state.dataAirLoop->PriAirSysAvailMgr(PriAirSysNum);
            previousAvailStatus = availMgr.availStatus; // Save the previous status for differential thermostat
            availMgr.availStatus = Status::NoAction;    // initialize the availability to "take no action"

            for (PriAirSysAvailMgrNum = 1; PriAirSysAvailMgrNum <= availMgr.NumAvailManagers; ++PriAirSysAvailMgrNum) {

                availStatus = SimSysAvailManager(state,
                                                 availMgr.availManagers(PriAirSysAvailMgrNum).type,
                                                 availMgr.availManagers(PriAirSysAvailMgrNum).Name,
                                                 availMgr.availManagers(PriAirSysAvailMgrNum).Num,
                                                 PriAirSysNum,
                                                 previousAvailStatus);

                if (availStatus == Status::ForceOff) {
                    availMgr.availStatus = Status::ForceOff;
                    break; // Fans forced off takes precedence
                } else if (availStatus == Status::CycleOnZoneFansOnly) {
                    availMgr.availStatus = Status::CycleOnZoneFansOnly; // zone fans only takes next precedence
                } else if ((availStatus == Status::CycleOn) && (availMgr.availStatus == Status::NoAction)) {
                    availMgr.availStatus = Status::CycleOn; // cycle on is lowest precedence
                }

            } // end of availability manager loop

            // Add hybrid ventilation control
            if (state.dataAvail->NumHybridVentSysAvailMgrs > 0) {
                for (HybridVentNum = 1; HybridVentNum <= state.dataAvail->NumHybridVentSysAvailMgrs; ++HybridVentNum) {
                    if (state.dataAvail->HybridVentData(HybridVentNum).AirLoopNum == PriAirSysNum &&
                        state.dataAvail->HybridVentData(HybridVentNum).ctrlStatus == VentCtrlStatus::Open) {
                        availMgr.availStatus = Status::ForceOff; // Force the system off
                    }
                }
            }

            // loop over the zones served by the system and set the zone equipment availability
            for (ZoneInSysNum = 1; ZoneInSysNum <= state.dataAirLoop->AirToZoneNodeInfo(PriAirSysNum).NumZonesCooled; ++ZoneInSysNum) {

                CtrldZoneNum = state.dataAirLoop->AirToZoneNodeInfo(PriAirSysNum).CoolCtrlZoneNums(ZoneInSysNum);
                state.dataZoneEquip->ZoneEquipAvail(CtrldZoneNum) = availMgr.availStatus;
            }

        } // end of primary air system loop

        for (PlantNum = 1; PlantNum <= state.dataHVACGlobal->NumPlantLoops; ++PlantNum) {
            auto &availMgr = state.dataAvail->PlantAvailMgr(PlantNum);
            previousAvailStatus = availMgr.availStatus; // Save the previous status for differential thermostat
            availMgr.availStatus = Status::NoAction;    // Initialize the availability to "take no action"

            for (PlantAvailMgrNum = 1; PlantAvailMgrNum <= availMgr.NumAvailManagers; ++PlantAvailMgrNum) { // loop over the avail managers in plant

                availStatus = SimSysAvailManager(state,
                                                 availMgr.availManagers(PlantAvailMgrNum).type,
                                                 availMgr.availManagers(PlantAvailMgrNum).Name,
                                                 availMgr.availManagers(PlantAvailMgrNum).Num,
                                                 PlantNum,
                                                 previousAvailStatus);

                if (availStatus != Status::NoAction) {
                    availMgr.availStatus = availStatus;
                    break; // First manager to do anything other than "NoAction" gets to set the availability
                }

            } // end of availability manager loop

        } // end of plant loop

        if (!allocated(state.dataAvail->ZoneComp)) {
            return;
        }

        // loop over the zone equipment types which allow system avail managers
        for (ZoneEquipType = 1; ZoneEquipType <= NumValidSysAvailZoneComponents; ++ZoneEquipType) {
            auto &zoneComp = state.dataAvail->ZoneComp(ZoneEquipType);
            if (zoneComp.TotalNumComp == 0) {
                continue;
            }
            if (!allocated(zoneComp.ZoneCompAvailMgrs)) {
                continue;
            }

            for (CompNum = 1; CompNum <= zoneComp.TotalNumComp; ++CompNum) {

                auto &zcam = zoneComp.ZoneCompAvailMgrs(CompNum);
                if (zcam.NumAvailManagers > 0) {

                    // Save the previous status for differential thermostat
                    previousAvailStatus = zcam.availStatus;
                    // initialize the availability to "take no action"
                    zcam.availStatus = Status::NoAction;
                    for (ZoneCompAvailMgrNum = 1; ZoneCompAvailMgrNum <= zcam.NumAvailManagers; ++ZoneCompAvailMgrNum) {
                        // loop over the avail managers in ZoneHVAC:* components
                        availStatus = SimSysAvailManager(state,
                                                         zcam.availManagers(ZoneCompAvailMgrNum).type,
                                                         zcam.availManagers(ZoneCompAvailMgrNum).Name,
                                                         zcam.availManagers(ZoneCompAvailMgrNum).Num,
                                                         DummyArgument,
                                                         previousAvailStatus,
                                                         ZoneEquipType,
                                                         CompNum);
                        if (availStatus == Status::ForceOff) {
                            zcam.availStatus = Status::ForceOff;
                            break; // Fans forced off takes precedence
                        } else if ((availStatus == Status::CycleOn) && (zcam.availStatus == Status::NoAction)) {
                            // cycle on is next precedence
                            zcam.availStatus = Status::CycleOn;
                        }
                    }
                } else {
                    zcam.availStatus = Status::NoAction;
                }

                if (zcam.ZoneNum == 0) {
                    continue;
                }
                if (state.dataAvail->NumHybridVentSysAvailMgrs == 0) {
                    continue;
                }

                for (HybridVentNum = 1; HybridVentNum <= state.dataAvail->NumHybridVentSysAvailMgrs; ++HybridVentNum) {
                    if (!state.dataAvail->HybridVentData(HybridVentNum).HybridVentMgrConnectedToAirLoop) {
                        if (state.dataAvail->HybridVentData(HybridVentNum).ControlledZoneNum == zcam.ZoneNum) {
                            if (state.dataAvail->HybridVentData(HybridVentNum).ctrlStatus == VentCtrlStatus::Open) {
                                zcam.availStatus = Status::ForceOff;
                            }
                        }
                    }
                }
            }
        } // for (ZoneEquipType)
    } // ManageSystemAvailability()

    void GetSysAvailManagerInputs(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   August 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains input data for System Availability Managers and stores it in
        // appropriate data structures.

        // METHODOLOGY EMPLOYED:
        // Uses InputProcessor "Get" routines to obtain data.

        // Using/Aliasing
        using NodeInputManager::GetOnlySingleNode;
        using NodeInputManager::MarkNode;
        using namespace DataLoopNode;
        using DataZoneEquipment::cValidSysAvailManagerCompTypes;
        using DataZoneEquipment::NumValidSysAvailZoneComponents;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetSysAvailManagerInputs: "); // include trailing blank
        static constexpr std::string_view routineName = "GetSysAvailManagerInputs";

        constexpr std::array<std::string_view, (int)ControlAlgorithm::Num> ControlAlgorithmNamesUC = {
            "CONSTANTTEMPERATUREGRADIENT", "ADAPTIVETEMPERATUREGRADIENT", "ADAPTIVEASHRAE", "CONSTANTSTARTTIME"};

        constexpr std::array<std::string_view, (int)CyclingRunTimeControl::Num> CyclingRunTimeControlNamesUC{
            "FIXEDRUNTIME",
            "THERMOSTAT",
            "THERMOSTATWITHMINIMUMRUNTIME",
        };

        constexpr std::array<std::string_view, (int)NightCycleControlType::Num> NightCycleControlTypeNamesUC{
            "STAYOFF",
            "CYCLEONANY",
            "CYCLEONCONTROLZONE",
            "CYCLEONANYZONEFANSONLY",
            "CYCLEONANYCOOLINGORHEATINGZONE",
            "CYCLEONANYCOOLINGZONE",
            "CYCLEONANYHEATINGZONE",
            "CYCLEONANYHEATINGZONEFANSONLY",
        };

        constexpr std::array<std::string_view, (int)OptimumStartControlType::Num> OptimumStartControlTypeNamesUC{
            "STAYOFF",
            "CONTROLZONE",
            "MAXIMUMOFZONELIST",
        };

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Array1D_string cAlphaFieldNames;
        Array1D_string cNumericFieldNames;
        Array1D_bool lNumericFieldBlanks;
        Array1D_bool lAlphaFieldBlanks;
        Array1D_string cAlphaArgs;
        Array1D<Real64> rNumericArgs;
        int NumAlphas;           // Number of Alphas for each GetObjectItem call
        int NumNumbers;          // Number of Numbers for each GetObjectItem call
        int maxAlphas = 0;       // maximum number of alphas for this set of objects
        int maxNumbers = 0;      // maximum number of numbers for this set of objects
        int numArgs;             // maximum number of arguments for this set of objects
        int IOStatus;            // Used in GetObjectItem
        bool ErrorsFound(false); // Set to true if errors in input, fatal at end of routine
        int CyclingTimeSteps;
        int ZoneEquipType;
        int TotalNumComp;

        // Get the number of occurrences of each type of manager and read in data
        for (int currentModuleObjectCount = 0; currentModuleObjectCount < (int)ManagerType::Num; ++currentModuleObjectCount) {
            std::string_view cCurrentModuleObject = managerTypeNames[currentModuleObjectCount];
            state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, numArgs, NumAlphas, NumNumbers);
            maxNumbers = max(maxNumbers, NumNumbers);
            maxAlphas = max(maxAlphas, NumAlphas);
        }

        cAlphaFieldNames.allocate(maxAlphas);
        cAlphaArgs.allocate(maxAlphas);
        lAlphaFieldBlanks.dimension(maxAlphas, false);
        cNumericFieldNames.allocate(maxNumbers);
        rNumericArgs.dimension(maxNumbers, 0.0);
        lNumericFieldBlanks.dimension(maxNumbers, false);

        if (!allocated(state.dataAvail->ZoneComp)) {
            state.dataAvail->ZoneComp.allocate(NumValidSysAvailZoneComponents);
        }

        for (ZoneEquipType = 1; ZoneEquipType <= NumValidSysAvailZoneComponents; ++ZoneEquipType) {
            auto &zoneComp = state.dataAvail->ZoneComp(ZoneEquipType);
            if (!allocated(zoneComp.ZoneCompAvailMgrs)) {
                TotalNumComp = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cValidSysAvailManagerCompTypes(ZoneEquipType));
                zoneComp.TotalNumComp = TotalNumComp;
                if (TotalNumComp > 0) {
                    zoneComp.ZoneCompAvailMgrs.allocate(TotalNumComp);
                }
            }
        }

        std::string_view cCurrentModuleObject = managerTypeNames[(int)ManagerType::Scheduled];
        state.dataAvail->NumSchedSysAvailMgrs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        if (state.dataAvail->NumSchedSysAvailMgrs > 0) {

            state.dataAvail->SchedData.allocate(state.dataAvail->NumSchedSysAvailMgrs);

            for (int SysAvailNum = 1; SysAvailNum <= state.dataAvail->NumSchedSysAvailMgrs; ++SysAvailNum) {

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         SysAvailNum,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         lNumericFieldBlanks,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         cNumericFieldNames);

                ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};
                auto &schedMgr = state.dataAvail->SchedData(SysAvailNum);
                schedMgr.Name = cAlphaArgs(1);
                schedMgr.type = ManagerType::Scheduled;

                if (lAlphaFieldBlanks(2)) {
                    ShowSevereEmptyField(state, eoh, cAlphaFieldNames(2));
                } else if ((schedMgr.availSched = Sched::GetSchedule(state, cAlphaArgs(2))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(2), cAlphaArgs(2));
                    ErrorsFound = true;
                }

                SetupOutputVariable(state,
                                    "Availability Manager Scheduled Control Status",
                                    Constant::Units::None,
                                    schedMgr.availStatus,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    schedMgr.Name);

            } // SysAvailNum
        }

        cCurrentModuleObject = managerTypeNames[(int)ManagerType::ScheduledOn];
        state.dataAvail->NumSchedOnSysAvailMgrs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        if (state.dataAvail->NumSchedOnSysAvailMgrs > 0) {

            state.dataAvail->SchedOnData.allocate(state.dataAvail->NumSchedOnSysAvailMgrs);

            for (int SysAvailNum = 1; SysAvailNum <= state.dataAvail->NumSchedOnSysAvailMgrs; ++SysAvailNum) {

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         SysAvailNum,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         lNumericFieldBlanks,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         cNumericFieldNames);

                ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};
                auto &schedOnMgr = state.dataAvail->SchedOnData(SysAvailNum);
                schedOnMgr.Name = cAlphaArgs(1);
                schedOnMgr.type = ManagerType::ScheduledOn;

                if (lAlphaFieldBlanks(2)) {
                    ShowSevereEmptyField(state, eoh, cAlphaFieldNames(2));
                    ErrorsFound = true;
                } else if ((schedOnMgr.availSched = Sched::GetSchedule(state, cAlphaArgs(2))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(2), cAlphaArgs(2));
                    ErrorsFound = true;
                }

                SetupOutputVariable(state,
                                    "Availability Manager Scheduled On Control Status",
                                    Constant::Units::None,
                                    schedOnMgr.availStatus,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    schedOnMgr.Name);

            } // SysAvailNum
        }

        cCurrentModuleObject = managerTypeNames[(int)ManagerType::ScheduledOff];
        state.dataAvail->NumSchedOffSysAvailMgrs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        if (state.dataAvail->NumSchedOffSysAvailMgrs > 0) {

            state.dataAvail->SchedOffData.allocate(state.dataAvail->NumSchedOffSysAvailMgrs);

            for (int SysAvailNum = 1; SysAvailNum <= state.dataAvail->NumSchedOffSysAvailMgrs; ++SysAvailNum) {

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         SysAvailNum,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         lNumericFieldBlanks,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         cNumericFieldNames);

                ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};
                auto &schedOffMgr = state.dataAvail->SchedOffData(SysAvailNum);
                schedOffMgr.Name = cAlphaArgs(1);
                schedOffMgr.type = ManagerType::ScheduledOff;

                if (lAlphaFieldBlanks(2)) {
                    ShowSevereEmptyField(state, eoh, cAlphaFieldNames(2));
                    ErrorsFound = true;
                } else if ((schedOffMgr.availSched = Sched::GetSchedule(state, cAlphaArgs(2))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(2), cAlphaArgs(2));
                    ErrorsFound = true;
                }

                SetupOutputVariable(state,
                                    "Availability Manager Scheduled Off Control Status",
                                    Constant::Units::None,
                                    schedOffMgr.availStatus,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    schedOffMgr.Name);

            } // SysAvailNum
        }

        cCurrentModuleObject = managerTypeNames[(int)ManagerType::NightCycle];
        state.dataAvail->NumNCycSysAvailMgrs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
        CyclingTimeSteps = 0;

        if (state.dataAvail->NumNCycSysAvailMgrs > 0) {

            state.dataAvail->NightCycleData.allocate(state.dataAvail->NumNCycSysAvailMgrs);

            for (int SysAvailNum = 1; SysAvailNum <= state.dataAvail->NumNCycSysAvailMgrs; ++SysAvailNum) {

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         SysAvailNum,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         lNumericFieldBlanks,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         cNumericFieldNames);

                ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};
                auto &nightCycleMgr = state.dataAvail->NightCycleData(SysAvailNum);
                nightCycleMgr.Name = cAlphaArgs(1);
                nightCycleMgr.type = ManagerType::NightCycle;
                nightCycleMgr.TempTolRange = rNumericArgs(1);
                CyclingTimeSteps = nint((rNumericArgs(2) / Constant::rSecsInHour) * double(state.dataGlobal->TimeStepsInHour));
                CyclingTimeSteps = max(1, CyclingTimeSteps);
                nightCycleMgr.CyclingTimeSteps = CyclingTimeSteps;

                if (lAlphaFieldBlanks(2)) {
                    ShowSevereEmptyField(state, eoh, cAlphaFieldNames(2));
                    ErrorsFound = true;
                } else if ((nightCycleMgr.availSched = Sched::GetSchedule(state, cAlphaArgs(2))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(2), cAlphaArgs(2));
                    ErrorsFound = true;
                }

                if (lAlphaFieldBlanks(3)) {
                    ShowSevereEmptyField(state, eoh, cAlphaFieldNames(3));
                    ErrorsFound = true;
                } else if ((nightCycleMgr.fanSched = Sched::GetSchedule(state, cAlphaArgs(3))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(3), cAlphaArgs(3));
                    ErrorsFound = true;
                }

                nightCycleMgr.nightCycleControlType = static_cast<NightCycleControlType>(getEnumValue(NightCycleControlTypeNamesUC, cAlphaArgs(4)));

                // Cycling Run Time Control Type
                nightCycleMgr.cyclingRunTimeControl = static_cast<CyclingRunTimeControl>(getEnumValue(CyclingRunTimeControlNamesUC, cAlphaArgs(5)));

                // Control zone or zonelist
                if (!lAlphaFieldBlanks(6)) {
                    nightCycleMgr.CtrlZoneListName = cAlphaArgs(6);
                    int ZoneNum = Util::FindItemInList(cAlphaArgs(6), state.dataHeatBal->Zone);
                    if (ZoneNum > 0) {
                        nightCycleMgr.NumOfCtrlZones = 1;
                        nightCycleMgr.CtrlZonePtrs.allocate(1);
                        nightCycleMgr.CtrlZonePtrs(1) = ZoneNum;
                    } else {
                        int zoneListNum = 0;
                        if (state.dataHeatBal->NumOfZoneLists > 0) {
                            zoneListNum = Util::FindItemInList(cAlphaArgs(6), state.dataHeatBal->ZoneList);
                        }
                        if (zoneListNum > 0) {
                            int NumZones = state.dataHeatBal->ZoneList(zoneListNum).NumOfZones;
                            nightCycleMgr.NumOfCtrlZones = NumZones;
                            nightCycleMgr.CtrlZonePtrs.allocate(NumZones);
                            for (int zoneNumInList = 1; zoneNumInList <= NumZones; ++zoneNumInList) {
                                nightCycleMgr.CtrlZonePtrs(zoneNumInList) = state.dataHeatBal->ZoneList(zoneListNum).Zone(zoneNumInList);
                            }
                        } else {
                            ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(6), cAlphaArgs(6));
                            ErrorsFound = true;
                        }
                    }
                } else if (nightCycleMgr.nightCycleControlType == NightCycleControlType::OnControlZone) {
                    ShowSevereEmptyField(state, eoh, cAlphaFieldNames(6), cAlphaFieldNames(4), cAlphaArgs(4));
                    ErrorsFound = true;
                }

                // Cooling zone or zonelist
                if (!lAlphaFieldBlanks(7)) {
                    nightCycleMgr.CoolingZoneListName = cAlphaArgs(7);
                    int ZoneNum = Util::FindItemInList(cAlphaArgs(7), state.dataHeatBal->Zone);
                    if (ZoneNum > 0) {
                        nightCycleMgr.NumOfCoolingZones = 1;
                        nightCycleMgr.CoolingZonePtrs.allocate(1);
                        nightCycleMgr.CoolingZonePtrs(1) = ZoneNum;
                    } else {
                        int zoneListNum = 0;
                        if (state.dataHeatBal->NumOfZoneLists > 0) {
                            zoneListNum = Util::FindItemInList(cAlphaArgs(7), state.dataHeatBal->ZoneList);
                        }
                        if (zoneListNum > 0) {
                            int NumZones = state.dataHeatBal->ZoneList(zoneListNum).NumOfZones;
                            nightCycleMgr.NumOfCoolingZones = NumZones;
                            nightCycleMgr.CoolingZonePtrs.allocate(NumZones);
                            for (int zoneNumInList = 1; zoneNumInList <= NumZones; ++zoneNumInList) {
                                nightCycleMgr.CoolingZonePtrs(zoneNumInList) = state.dataHeatBal->ZoneList(zoneListNum).Zone(zoneNumInList);
                            }
                        } else {
                            ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(7), cAlphaArgs(7));
                            ErrorsFound = true;
                        }
                    }
                }

                // Heating zone or zonelist
                if (!lAlphaFieldBlanks(8)) {
                    nightCycleMgr.HeatingZoneListName = cAlphaArgs(8);
                    int ZoneNum = Util::FindItemInList(cAlphaArgs(8), state.dataHeatBal->Zone);
                    if (ZoneNum > 0) {
                        nightCycleMgr.NumOfHeatingZones = 1;
                        nightCycleMgr.HeatingZonePtrs.allocate(1);
                        nightCycleMgr.HeatingZonePtrs(1) = ZoneNum;
                    } else {
                        int zoneListNum = 0;
                        if (state.dataHeatBal->NumOfZoneLists > 0) {
                            zoneListNum = Util::FindItemInList(cAlphaArgs(8), state.dataHeatBal->ZoneList);
                        }
                        if (zoneListNum > 0) {
                            int NumZones = state.dataHeatBal->ZoneList(zoneListNum).NumOfZones;
                            nightCycleMgr.NumOfHeatingZones = NumZones;
                            nightCycleMgr.HeatingZonePtrs.allocate(NumZones);
                            for (int zoneNumInList = 1; zoneNumInList <= NumZones; ++zoneNumInList) {
                                nightCycleMgr.HeatingZonePtrs(zoneNumInList) = state.dataHeatBal->ZoneList(zoneListNum).Zone(zoneNumInList);
                            }
                        } else {
                            ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(8), cAlphaArgs(8));
                            ErrorsFound = true;
                        }
                    }
                }

                // HeatZnFan zone or zonelist
                if (!lAlphaFieldBlanks(9)) {
                    nightCycleMgr.HeatZnFanZoneListName = cAlphaArgs(9);
                    int ZoneNum = Util::FindItemInList(cAlphaArgs(9), state.dataHeatBal->Zone);
                    if (ZoneNum > 0) {
                        nightCycleMgr.NumOfHeatZnFanZones = 1;
                        nightCycleMgr.HeatZnFanZonePtrs.allocate(1);
                        nightCycleMgr.HeatZnFanZonePtrs(1) = ZoneNum;
                    } else {
                        int zoneListNum = 0;
                        if (state.dataHeatBal->NumOfZoneLists > 0) {
                            zoneListNum = Util::FindItemInList(cAlphaArgs(9), state.dataHeatBal->ZoneList);
                        }
                        if (zoneListNum > 0) {
                            int NumZones = state.dataHeatBal->ZoneList(zoneListNum).NumOfZones;
                            nightCycleMgr.NumOfHeatZnFanZones = NumZones;
                            nightCycleMgr.HeatZnFanZonePtrs.allocate(NumZones);
                            for (int zoneNumInList = 1; zoneNumInList <= NumZones; ++zoneNumInList) {
                                nightCycleMgr.HeatZnFanZonePtrs(zoneNumInList) = state.dataHeatBal->ZoneList(zoneListNum).Zone(zoneNumInList);
                            }
                        } else {
                            ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(9), cAlphaArgs(9));
                            ErrorsFound = true;
                        }
                    }
                }

                SetupOutputVariable(state,
                                    "Availability Manager Night Cycle Control Status",
                                    Constant::Units::None,
                                    nightCycleMgr.availStatus,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    nightCycleMgr.Name);

            } // SysAvailNum
        }

        cCurrentModuleObject = managerTypeNames[(int)ManagerType::OptimumStart];
        state.dataAvail->NumOptStartSysAvailMgrs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
        CyclingTimeSteps = 0;

        if (state.dataAvail->NumOptStartSysAvailMgrs > 0) {
            // Array size of variable type OptStartSysAvailMgrData is updated
            state.dataAvail->OptimumStartData.allocate(state.dataAvail->NumOptStartSysAvailMgrs);

            for (int SysAvailNum = 1; SysAvailNum <= state.dataAvail->NumOptStartSysAvailMgrs; ++SysAvailNum) {

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         SysAvailNum,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         lNumericFieldBlanks,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         cNumericFieldNames);

                ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};
                auto &optimumStartMgr = state.dataAvail->OptimumStartData(SysAvailNum);
                optimumStartMgr.Name = cAlphaArgs(1);
                optimumStartMgr.type = ManagerType::OptimumStart;

                if (lAlphaFieldBlanks(2)) {
                    ShowSevereEmptyField(state, eoh, cAlphaFieldNames(2));
                    ErrorsFound = true;
                } else if ((optimumStartMgr.availSched = Sched::GetSchedule(state, cAlphaArgs(2))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(2), cAlphaArgs(2));
                    ErrorsFound = true;
                }

                if (lAlphaFieldBlanks(3)) {
                    ShowSevereEmptyField(state, eoh, cAlphaFieldNames(3));
                    ErrorsFound = true;
                } else if ((optimumStartMgr.fanSched = Sched::GetSchedule(state, cAlphaArgs(3))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(3), cAlphaArgs(3));
                    ErrorsFound = true;
                }

                optimumStartMgr.MaxOptStartTime = rNumericArgs(1);
                optimumStartMgr.optimumStartControlType =
                    static_cast<OptimumStartControlType>(getEnumValue(OptimumStartControlTypeNamesUC, cAlphaArgs(4)));

                if (optimumStartMgr.optimumStartControlType == OptimumStartControlType::Invalid) {
                    optimumStartMgr.optimumStartControlType = OptimumStartControlType::ControlZone;
                    ShowSevereInvalidKey(state, eoh, cAlphaFieldNames(4), cAlphaArgs(4));
                    ErrorsFound = true;
                }

                if (optimumStartMgr.optimumStartControlType == OptimumStartControlType::ControlZone) {
                    optimumStartMgr.CtrlZoneName = cAlphaArgs(5);
                    optimumStartMgr.ZoneNum = Util::FindItemInList(cAlphaArgs(5), state.dataHeatBal->Zone);
                    if (optimumStartMgr.ZoneNum == 0) {
                        ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(5), cAlphaArgs(5));
                        ErrorsFound = true;
                    }
                }

                if (optimumStartMgr.optimumStartControlType == OptimumStartControlType::MaximumOfZoneList) {
                    optimumStartMgr.ZoneListName = cAlphaArgs(6);
                    for (int zoneListNum = 1; zoneListNum <= state.dataHeatBal->NumOfZoneLists; ++zoneListNum) {
                        if (state.dataHeatBal->ZoneList(zoneListNum).Name == cAlphaArgs(6)) {
                            optimumStartMgr.NumOfZones = state.dataHeatBal->ZoneList(zoneListNum).NumOfZones;
                            optimumStartMgr.ZonePtrs.allocate(state.dataHeatBal->ZoneList(zoneListNum).NumOfZones);
                            for (int zoneNumInList = 1; zoneNumInList <= state.dataHeatBal->ZoneList(zoneListNum).NumOfZones; ++zoneNumInList) {
                                optimumStartMgr.ZonePtrs(zoneNumInList) = state.dataHeatBal->ZoneList(zoneListNum).Zone(zoneNumInList);
                            }
                        }
                    }
                    optimumStartMgr.NumOfZones = Util::FindItemInList(cAlphaArgs(6), state.dataHeatBal->ZoneList);
                    if (optimumStartMgr.NumOfZones == 0) {
                        ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(6), cAlphaArgs(6));
                        ErrorsFound = true;
                    }
                }

                optimumStartMgr.controlAlgorithm = static_cast<ControlAlgorithm>(getEnumValue(ControlAlgorithmNamesUC, cAlphaArgs(7)));

                switch (optimumStartMgr.controlAlgorithm) {
                case ControlAlgorithm::ConstantTemperatureGradient: {
                    optimumStartMgr.ConstTGradCool = rNumericArgs(2);
                    optimumStartMgr.ConstTGradHeat = rNumericArgs(3);
                } break;

                case ControlAlgorithm::AdaptiveTemperatureGradient: {
                    optimumStartMgr.InitTGradCool = rNumericArgs(4);
                    optimumStartMgr.InitTGradHeat = rNumericArgs(5);
                    optimumStartMgr.NumPreDays = rNumericArgs(7);
                } break;

                case ControlAlgorithm::ConstantStartTime: {
                    optimumStartMgr.ConstStartTime = rNumericArgs(6);
                } break;

                default:
                    break;
                }

                SetupOutputVariable(state,
                                    "Availability Manager Optimum Start Control Status",
                                    Constant::Units::None,
                                    optimumStartMgr.availStatus,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    optimumStartMgr.Name);

                // add
                SetupOutputVariable(state,
                                    "Availability Manager Optimum Start Time Before Occupancy",
                                    Constant::Units::hr,
                                    optimumStartMgr.NumHoursBeforeOccupancy,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    optimumStartMgr.Name,
                                    Constant::eResource::Invalid,
                                    OutputProcessor::Group::Invalid,
                                    OutputProcessor::EndUseCat::Invalid,
                                    "",   // End-use SubCat
                                    "",   // Zone
                                    1,    // ZoneMult
                                    1,    // ZoneListMult
                                    "",   // space type
                                    -999, // indexGroupKey
                                    "",   // custom units
                                    OutputProcessor::ReportFreq::Day);
            }
        }

        cCurrentModuleObject = managerTypeNames[(int)ManagerType::DiffThermo];
        state.dataAvail->NumDiffTSysAvailMgrs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        if (state.dataAvail->NumDiffTSysAvailMgrs > 0) {

            state.dataAvail->DiffThermoData.allocate(state.dataAvail->NumDiffTSysAvailMgrs);

            for (int SysAvailNum = 1; SysAvailNum <= state.dataAvail->NumDiffTSysAvailMgrs; ++SysAvailNum) {

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         SysAvailNum,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         lNumericFieldBlanks,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         cNumericFieldNames);

                auto &diffThermoMgr = state.dataAvail->DiffThermoData(SysAvailNum);
                diffThermoMgr.Name = cAlphaArgs(1);
                diffThermoMgr.type = ManagerType::DiffThermo;

                diffThermoMgr.HotNode = GetOnlySingleNode(state,
                                                          cAlphaArgs(2),
                                                          ErrorsFound,
                                                          DataLoopNode::ConnectionObjectType::AvailabilityManagerDifferentialThermostat,
                                                          cAlphaArgs(1),
                                                          DataLoopNode::NodeFluidType::Blank,
                                                          DataLoopNode::ConnectionType::Sensor,
                                                          NodeInputManager::CompFluidStream::Primary,
                                                          ObjectIsNotParent);
                MarkNode(state,
                         diffThermoMgr.HotNode,
                         DataLoopNode::ConnectionObjectType::AvailabilityManagerDifferentialThermostat,
                         cAlphaArgs(1),
                         "Hot Node");
                diffThermoMgr.ColdNode = GetOnlySingleNode(state,
                                                           cAlphaArgs(3),
                                                           ErrorsFound,
                                                           DataLoopNode::ConnectionObjectType::AvailabilityManagerDifferentialThermostat,
                                                           cAlphaArgs(1),
                                                           DataLoopNode::NodeFluidType::Blank,
                                                           DataLoopNode::ConnectionType::Sensor,
                                                           NodeInputManager::CompFluidStream::Primary,
                                                           ObjectIsNotParent);
                MarkNode(state,
                         diffThermoMgr.ColdNode,
                         DataLoopNode::ConnectionObjectType::AvailabilityManagerDifferentialThermostat,
                         cAlphaArgs(1),
                         "Cold Node");

                diffThermoMgr.TempDiffOn = rNumericArgs(1);

                if (NumNumbers > 1) {
                    diffThermoMgr.TempDiffOff = rNumericArgs(2);
                } else {
                    diffThermoMgr.TempDiffOff = diffThermoMgr.TempDiffOn;
                }

                if (diffThermoMgr.TempDiffOff > diffThermoMgr.TempDiffOn) {
                    ShowSevereError(state, format("{}{}=\"{}\", invalid", RoutineName, cCurrentModuleObject, cAlphaArgs(1)));
                    ShowContinueError(state, format("The {} is greater than the {}.", cNumericFieldNames(2), cNumericFieldNames(1)));
                    ErrorsFound = true;
                }

                SetupOutputVariable(state,
                                    "Availability Manager Differential Thermostat Control Status",
                                    Constant::Units::None,
                                    diffThermoMgr.availStatus,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    diffThermoMgr.Name);

            } // SysAvailNum
        }

        cCurrentModuleObject = managerTypeNames[(int)ManagerType::HiTempTOff];
        state.dataAvail->NumHiTurnOffSysAvailMgrs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        if (state.dataAvail->NumHiTurnOffSysAvailMgrs > 0) {
            state.dataAvail->HiTurnOffData.allocate(state.dataAvail->NumHiTurnOffSysAvailMgrs);

            for (int SysAvailNum = 1; SysAvailNum <= state.dataAvail->NumHiTurnOffSysAvailMgrs; ++SysAvailNum) {

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         SysAvailNum,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         lNumericFieldBlanks,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         cNumericFieldNames);

                auto &hiTurnOffMgr = state.dataAvail->HiTurnOffData(SysAvailNum);
                hiTurnOffMgr.Name = cAlphaArgs(1);
                hiTurnOffMgr.type = ManagerType::HiTempTOff;

                hiTurnOffMgr.Node = GetOnlySingleNode(state,
                                                      cAlphaArgs(2),
                                                      ErrorsFound,
                                                      DataLoopNode::ConnectionObjectType::AvailabilityManagerHighTemperatureTurnOff,
                                                      cAlphaArgs(1),
                                                      DataLoopNode::NodeFluidType::Blank,
                                                      DataLoopNode::ConnectionType::Sensor,
                                                      NodeInputManager::CompFluidStream::Primary,
                                                      ObjectIsNotParent);
                MarkNode(state,
                         hiTurnOffMgr.Node,
                         DataLoopNode::ConnectionObjectType::AvailabilityManagerHighTemperatureTurnOff,
                         cAlphaArgs(1),
                         "Sensor Node");

                hiTurnOffMgr.Temp = rNumericArgs(1);

                SetupOutputVariable(state,
                                    "Availability Manager High Temperature Turn Off Control Status",
                                    Constant::Units::None,
                                    hiTurnOffMgr.availStatus,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    hiTurnOffMgr.Name);

            } // SysAvailNum
        }

        cCurrentModuleObject = managerTypeNames[(int)ManagerType::HiTempTOn];
        state.dataAvail->NumHiTurnOnSysAvailMgrs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        if (state.dataAvail->NumHiTurnOnSysAvailMgrs > 0) {

            state.dataAvail->HiTurnOnData.allocate(state.dataAvail->NumHiTurnOnSysAvailMgrs);

            for (int SysAvailNum = 1; SysAvailNum <= state.dataAvail->NumHiTurnOnSysAvailMgrs; ++SysAvailNum) {

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         SysAvailNum,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         lNumericFieldBlanks,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         cNumericFieldNames);
                auto &hiTurnOnMgr = state.dataAvail->HiTurnOnData(SysAvailNum);
                hiTurnOnMgr.Name = cAlphaArgs(1);
                hiTurnOnMgr.type = ManagerType::HiTempTOn;

                hiTurnOnMgr.Node = GetOnlySingleNode(state,
                                                     cAlphaArgs(2),
                                                     ErrorsFound,
                                                     DataLoopNode::ConnectionObjectType::AvailabilityManagerHighTemperatureTurnOn,
                                                     cAlphaArgs(1),
                                                     DataLoopNode::NodeFluidType::Blank,
                                                     DataLoopNode::ConnectionType::Sensor,
                                                     NodeInputManager::CompFluidStream::Primary,
                                                     ObjectIsNotParent);
                MarkNode(state,
                         hiTurnOnMgr.Node,
                         DataLoopNode::ConnectionObjectType::AvailabilityManagerHighTemperatureTurnOn,
                         cAlphaArgs(1),
                         "Sensor Node");

                hiTurnOnMgr.Temp = rNumericArgs(1);

                SetupOutputVariable(state,
                                    "Availability Manager High Temperature Turn On Control Status",
                                    Constant::Units::None,
                                    hiTurnOnMgr.availStatus,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    hiTurnOnMgr.Name);

            } // SysAvailNum
        }

        cCurrentModuleObject = managerTypeNames[(int)ManagerType::LoTempTOff];
        state.dataAvail->NumLoTurnOffSysAvailMgrs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        if (state.dataAvail->NumLoTurnOffSysAvailMgrs > 0) {

            state.dataAvail->LoTurnOffData.allocate(state.dataAvail->NumLoTurnOffSysAvailMgrs);

            for (int SysAvailNum = 1; SysAvailNum <= state.dataAvail->NumLoTurnOffSysAvailMgrs; ++SysAvailNum) {

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         SysAvailNum,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         lNumericFieldBlanks,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         cNumericFieldNames);
                ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};
                auto &loTurnOffMgr = state.dataAvail->LoTurnOffData(SysAvailNum);
                loTurnOffMgr.Name = cAlphaArgs(1);
                loTurnOffMgr.type = ManagerType::LoTempTOff;

                loTurnOffMgr.Node = GetOnlySingleNode(state,
                                                      cAlphaArgs(2),
                                                      ErrorsFound,
                                                      DataLoopNode::ConnectionObjectType::AvailabilityManagerLowTemperatureTurnOff,
                                                      cAlphaArgs(1),
                                                      DataLoopNode::NodeFluidType::Blank,
                                                      DataLoopNode::ConnectionType::Sensor,
                                                      NodeInputManager::CompFluidStream::Primary,
                                                      ObjectIsNotParent);
                MarkNode(state,
                         loTurnOffMgr.Node,
                         DataLoopNode::ConnectionObjectType::AvailabilityManagerLowTemperatureTurnOff,
                         cAlphaArgs(1),
                         "Sensor Node");

                loTurnOffMgr.Temp = rNumericArgs(1);

                if (lAlphaFieldBlanks(3)) {
                } else if ((loTurnOffMgr.availSched = Sched::GetSchedule(state, cAlphaArgs(3))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(3), cAlphaArgs(3));
                    ErrorsFound = true;
                }

                SetupOutputVariable(state,
                                    "Availability Manager Low Temperature Turn Off Control Status",
                                    Constant::Units::None,
                                    loTurnOffMgr.availStatus,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    loTurnOffMgr.Name);

            } // SysAvailNum
        }

        cCurrentModuleObject = managerTypeNames[(int)ManagerType::LoTempTOn];
        state.dataAvail->NumLoTurnOnSysAvailMgrs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        if (state.dataAvail->NumLoTurnOnSysAvailMgrs > 0) {

            state.dataAvail->LoTurnOnData.allocate(state.dataAvail->NumLoTurnOnSysAvailMgrs);

            for (int SysAvailNum = 1; SysAvailNum <= state.dataAvail->NumLoTurnOnSysAvailMgrs; ++SysAvailNum) {

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         SysAvailNum,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         lNumericFieldBlanks,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         cNumericFieldNames);

                auto &loTurnOnMgr = state.dataAvail->LoTurnOnData(SysAvailNum);
                loTurnOnMgr.Name = cAlphaArgs(1);
                loTurnOnMgr.type = ManagerType::LoTempTOn;

                loTurnOnMgr.Node = GetOnlySingleNode(state,
                                                     cAlphaArgs(2),
                                                     ErrorsFound,
                                                     DataLoopNode::ConnectionObjectType::AvailabilityManagerLowTemperatureTurnOn,
                                                     cAlphaArgs(1),
                                                     DataLoopNode::NodeFluidType::Blank,
                                                     DataLoopNode::ConnectionType::Sensor,
                                                     NodeInputManager::CompFluidStream::Primary,
                                                     ObjectIsNotParent);
                MarkNode(state,
                         loTurnOnMgr.Node,
                         DataLoopNode::ConnectionObjectType::AvailabilityManagerLowTemperatureTurnOn,
                         cAlphaArgs(1),
                         "Sensor Node");

                loTurnOnMgr.Temp = rNumericArgs(1);

                SetupOutputVariable(state,
                                    "Availability Manager Low Temperature Turn On Control Status",
                                    Constant::Units::None,
                                    loTurnOnMgr.availStatus,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    loTurnOnMgr.Name);

            } // SysAvailNum
        }

        cCurrentModuleObject = managerTypeNames[(int)ManagerType::NightVent];
        state.dataAvail->NumNVentSysAvailMgrs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        if (state.dataAvail->NumNVentSysAvailMgrs > 0) {

            state.dataAvail->NightVentData.allocate(state.dataAvail->NumNVentSysAvailMgrs);

            for (int SysAvailNum = 1; SysAvailNum <= state.dataAvail->NumNVentSysAvailMgrs; ++SysAvailNum) {

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         SysAvailNum,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         lNumericFieldBlanks,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         cNumericFieldNames);

                ErrorObjectHeader eoh{routineName, cCurrentModuleObject, cAlphaArgs(1)};
                auto &nightVentMgr = state.dataAvail->NightVentData(SysAvailNum);
                nightVentMgr.Name = cAlphaArgs(1);
                nightVentMgr.type = ManagerType::NightVent;

                if (lAlphaFieldBlanks(2)) {
                    ShowSevereEmptyField(state, eoh, cAlphaFieldNames(2));
                    ErrorsFound = true;
                } else if ((nightVentMgr.availSched = Sched::GetSchedule(state, cAlphaArgs(2))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(2), cAlphaArgs(2));
                    ErrorsFound = true;
                }

                if (lAlphaFieldBlanks(3)) {
                    ShowSevereEmptyField(state, eoh, cAlphaFieldNames(3));
                    ErrorsFound = true;
                } else if ((nightVentMgr.fanSched = Sched::GetSchedule(state, cAlphaArgs(3))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(3), cAlphaArgs(3));
                    ErrorsFound = true;
                }

                if (lAlphaFieldBlanks(4)) {
                    ShowSevereEmptyField(state, eoh, cAlphaFieldNames(4));
                    ErrorsFound = true;
                } else if ((nightVentMgr.ventTempSched = Sched::GetSchedule(state, cAlphaArgs(4))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(4), cAlphaArgs(4));
                    ErrorsFound = true;
                }

                nightVentMgr.VentDelT = rNumericArgs(1);
                nightVentMgr.VentTempLowLim = rNumericArgs(2);
                nightVentMgr.VentFlowFrac = rNumericArgs(3);
                nightVentMgr.CtrlZoneName = cAlphaArgs(5);
                nightVentMgr.ZoneNum = Util::FindItemInList(cAlphaArgs(5), state.dataHeatBal->Zone);
                if (nightVentMgr.ZoneNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFieldNames(5), cAlphaArgs(5));
                    ErrorsFound = true;
                }

                SetupOutputVariable(state,
                                    "Availability Manager Night Ventilation Control Status",
                                    Constant::Units::None,
                                    nightVentMgr.availStatus,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    nightVentMgr.Name);

            } // SysAvailNum
        }

        cAlphaFieldNames.deallocate();
        cAlphaArgs.deallocate();
        lAlphaFieldBlanks.deallocate();
        cNumericFieldNames.deallocate();
        rNumericArgs.deallocate();
        lNumericFieldBlanks.deallocate();

        if (ErrorsFound) {
            ShowFatalError(state, format("{}Errors found in input.  Preceding condition(s) cause termination.", RoutineName));
        }
    } // GetSysAvailManagerInputs()

    void GetSysAvailManagerListInputs(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   August 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This routine gets the System Availability Manager List object input and stores
        // it for later retrieval of items from the Plant and Air Loops.

        if (state.dataAvail->GetAvailMgrInputFlag) {
            GetSysAvailManagerInputs(state);
            state.dataAvail->GetAvailMgrInputFlag = false;
        }

        bool ErrorsFound = false;
        std::string const cCurrentModuleObject = "AvailabilityManagerAssignmentList";
        auto &ip = state.dataInputProcessing->inputProcessor;

        state.dataAvail->NumAvailManagerLists = ip->getNumObjectsFound(state, cCurrentModuleObject);

        if (state.dataAvail->NumAvailManagerLists > 0) {

            state.dataAvail->ListData.allocate(state.dataAvail->NumAvailManagerLists);
            auto const instances = ip->epJSON.find(cCurrentModuleObject);
            auto const &objectSchemaProps = ip->getObjectSchemaProps(state, cCurrentModuleObject);

            auto &instancesValue = instances.value();
            int Item = 0;
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                ++Item;
                auto const &objectFields = instance.value();
                std::string const thisObjectName = Util::makeUPPER(instance.key());
                ip->markObjectAsUsed(cCurrentModuleObject, instance.key());
                auto &mgrList = state.dataAvail->ListData(Item);
                mgrList.Name = thisObjectName;

                auto extensibles = objectFields.find("managers");
                auto const &extensionSchemaProps = objectSchemaProps["managers"]["items"]["properties"];
                if (extensibles != objectFields.end()) {
                    auto &extensiblesArray = extensibles.value();
                    int numExtensibles = extensiblesArray.size();
                    mgrList.NumItems = numExtensibles;
                    mgrList.availManagers.allocate(numExtensibles);
                    for (int extItem = 1; extItem <= numExtensibles; ++extItem) {
                        mgrList.availManagers(extItem).Name = "";
                        mgrList.availManagers(extItem).type = ManagerType::Invalid;
                    }

                    int listItem = 0;
                    for (nlohmann::json const &extensibleInstance : extensiblesArray) {
                        ++listItem;
                        mgrList.availManagers(listItem).Name =
                            ip->getAlphaFieldValue(extensibleInstance, extensionSchemaProps, "availability_manager_name");
                        std::string availManagerObjType =
                            ip->getAlphaFieldValue(extensibleInstance, extensionSchemaProps, "availability_manager_object_type");
                        mgrList.availManagers(listItem).type =
                            static_cast<ManagerType>(getEnumValue(managerTypeNamesUC, Util::makeUPPER(availManagerObjType)));
                        if (mgrList.availManagers(listItem).type == ManagerType::HybridVent) {
                            mgrList.availManagers(listItem).type = ManagerType::Invalid;
                        }
                        // these are validated individually in the GetPlant, GetSystem and GetZoneEq lists
                    }
                }
            }

            if (ErrorsFound) {
                ShowFatalError(state, "GetSysAvailManagerListInputs: Program terminates due to preceding conditions.");
            }
        }
    } // GetSysAvailManagerListInputs()

    void GetPlantAvailabilityManager(EnergyPlusData &state,
                                     std::string const &AvailabilityListName, // name that should be an Availability Manager List Name
                                     int const Loop,                          // which loop this is
                                     int const NumPlantLoops,                 // Total number of plant loops
                                     bool &ErrorsFound                        // true if certain errors are detected here
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   August 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the plant availability manager data for the indicated
        // loop.  If the PlantAvailMgr structure has not been allocated, it will be allocated
        // to "number of plant loops".
        auto &availMgr = state.dataAvail->PlantAvailMgr(Loop);

        if (state.dataAvail->GetAvailListsInput) {
            GetSysAvailManagerListInputs(state);
            state.dataAvail->GetAvailListsInput = false;
        }

        if (!allocated(state.dataAvail->PlantAvailMgr)) {
            state.dataAvail->PlantAvailMgr.allocate(NumPlantLoops);
        }

        int Found = 0;
        if (state.dataAvail->NumAvailManagerLists > 0) {
            Found = Util::FindItemInList(AvailabilityListName, state.dataAvail->ListData);
        }

        if (Found != 0) {
            availMgr.NumAvailManagers = state.dataAvail->ListData(Found).NumItems;
            availMgr.availStatus = Status::NoAction;
            availMgr.StartTime = 0;
            availMgr.StopTime = 0;
            availMgr.availManagers.allocate(availMgr.NumAvailManagers);
            for (int Num = 1; Num <= availMgr.NumAvailManagers; ++Num) {
                auto &am = availMgr.availManagers(Num);
                am.Name = state.dataAvail->ListData(Found).availManagers(Num).Name;
                am.Num = 0;
                am.type = state.dataAvail->ListData(Found).availManagers(Num).type;
                assert(am.type != ManagerType::Invalid);

                if (am.type == ManagerType::DiffThermo && Num != availMgr.NumAvailManagers) {
                    ShowWarningError(
                        state, format("GetPlantLoopData/GetPlantAvailabilityManager: AvailabilityManager:DifferentialThermostat=\"{}\".", am.Name));
                    ShowContinueError(
                        state, "...is not the last manager on the AvailabilityManagerAssignmentList.  Any remaining managers will not be used.");
                    ShowContinueError(state, format("Occurs in AvailabilityManagerAssignmentList =\"{}\".", AvailabilityListName));
                }
                if (am.type == ManagerType::NightVent || am.type == ManagerType::NightCycle) {
                    ShowSevereError(state,
                                    format("GetPlantLoopData/GetPlantAvailabilityManager: Invalid System Availability Manager Type entered=\"{}\".",
                                           managerTypeNames[(int)am.type]));
                    ShowContinueError(state, "...this manager is not used in a Plant Loop.");
                    ShowContinueError(state, format("Occurs in AvailabilityManagerAssignmentList=\"{}\".", AvailabilityListName));
                    ErrorsFound = true;
                }
            } // End of Num Loop

        } else {
            if (AvailabilityListName != "") {
                ShowWarningError(state,
                                 format("GetPlantLoopData/GetPlantAvailabilityManager: AvailabilityManagerAssignmentList={} not found in lists.  No "
                                        "availability will be used.",
                                        AvailabilityListName));
            }
            availMgr.NumAvailManagers = 0;
            availMgr.availStatus = Status::NoAction;
            availMgr.availManagers.allocate(availMgr.NumAvailManagers);
        }
    }

    void GetAirLoopAvailabilityManager(EnergyPlusData &state,
                                       std::string const &AvailabilityListName, // name that should be an Availability Manager List Name
                                       int const Loop,                          // which loop this is
                                       int const NumAirLoops,                   // Total number of air loops
                                       [[maybe_unused]] bool &ErrorsFound       // true if certain errors are detected here
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   August 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the availability manager data for the indicated air
        // loop or for the indicated type of zone equipment component.
        // If the PriAirSysAvailMgr structure has not been allocated, it will be allocated
        // to "number of air loops".
        if (state.dataAvail->GetAvailListsInput) {
            GetSysAvailManagerListInputs(state);
            state.dataAvail->GetAvailListsInput = false;
        }

        if (!allocated(state.dataAirLoop->PriAirSysAvailMgr)) {
            state.dataAirLoop->PriAirSysAvailMgr.allocate(NumAirLoops);
        }

        auto &availMgr = state.dataAirLoop->PriAirSysAvailMgr(Loop);

        int Found = 0;
        if (state.dataAvail->NumAvailManagerLists > 0) {
            Found = Util::FindItemInList(AvailabilityListName, state.dataAvail->ListData);
        }

        if (Found != 0) {
            availMgr.NumAvailManagers = state.dataAvail->ListData(Found).NumItems;
            availMgr.availStatus = Status::NoAction;
            availMgr.StartTime = 0;
            availMgr.StopTime = 0;
            availMgr.ReqSupplyFrac = 1.0;
            availMgr.availManagers.allocate(availMgr.NumAvailManagers);
            for (int Num = 1; Num <= availMgr.NumAvailManagers; ++Num) {
                auto &am = availMgr.availManagers(Num);
                am.Name = state.dataAvail->ListData(Found).availManagers(Num).Name;
                am.Num = 0;
                am.type = state.dataAvail->ListData(Found).availManagers(Num).type;
                assert(am.type != ManagerType::Invalid);

                if (am.type == ManagerType::DiffThermo && Num != availMgr.NumAvailManagers) {
                    ShowWarningError(
                        state, format("GetAirPathData/GetAirLoopAvailabilityManager: AvailabilityManager:DifferentialThermostat=\"{}\".", am.Name));
                    ShowContinueError(
                        state, "...is not the last manager on the AvailabilityManagerAssignmentList.  Any remaining managers will not be used.");
                    ShowContinueError(state, format("Occurs in AvailabilityManagerAssignmentList=\"{}\".", am.Name));
                }
            } // End of Num Loop

        } else {
            if (AvailabilityListName != "") {
                ShowWarningError(state,
                                 format("GetAirPathData/GetAirLoopAvailabilityManager: AvailabilityManagerAssignmentList={} not found in lists.  No "
                                        "availability will be used.",
                                        AvailabilityListName));
            }
            availMgr.NumAvailManagers = 0;
            availMgr.availStatus = Status::NoAction;
            availMgr.availManagers.allocate(availMgr.NumAvailManagers);
        }
    }

    void GetZoneEqAvailabilityManager(EnergyPlusData &state,
                                      int const ZoneEquipType,           // Type of ZoneHVAC:* component
                                      int const CompNum,                 // Index of a particular ZoneHVAC:* component
                                      [[maybe_unused]] bool &ErrorsFound // true if certain errors are detected here
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   April 2011
        //       MODIFIED       Chandan Sharma, March 2011/July 2012 - FSEC: Added zone sys avail managers
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets the availability manager data for the indicated type of zone
        // equipment component.
        // If not allocated, ZoneComp structure will be allocated to "Total num of zone equip types" and
        // ZoneCompAvailMgrs structure will be allocated to "Total number of components of the indicated type".

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        std::string AvailabilityListName; // name that should be an Availability Manager List Name
        int CompNumAvailManagers;         // Number of availability managers associated with a ZoneHVAC:* component

        if (state.dataAvail->GetAvailListsInput) {
            GetSysAvailManagerListInputs(state);
            state.dataAvail->GetAvailListsInput = false;
        }

        auto &zoneComp = state.dataAvail->ZoneComp(ZoneEquipType);
        auto &availMgr = zoneComp.ZoneCompAvailMgrs(CompNum);
        if (availMgr.Input) { // when both air loop and zone eq avail managers are present, zone
                              // avail mngrs list name has not been read in first time through here
                              // (see end of if block)
            AvailabilityListName = availMgr.AvailManagerListName;
            int Found = 0;
            if (state.dataAvail->NumAvailManagerLists > 0) {
                Found = Util::FindItemInList(AvailabilityListName, state.dataAvail->ListData);
            }
            if (Found != 0) {
                availMgr.NumAvailManagers = state.dataAvail->ListData(Found).NumItems;
                CompNumAvailManagers = availMgr.NumAvailManagers;
                availMgr.availStatus = Status::NoAction;
                availMgr.StartTime = 0;
                availMgr.StopTime = 0;
                if (!allocated(availMgr.availManagers)) {
                    availMgr.availManagers.allocate(CompNumAvailManagers);
                }
                for (int Num = 1; Num <= availMgr.NumAvailManagers; ++Num) {
                    auto &am = availMgr.availManagers(Num);
                    am.Name = state.dataAvail->ListData(Found).availManagers(Num).Name;
                    am.Num = 0;
                    am.type = state.dataAvail->ListData(Found).availManagers(Num).type;
                    assert(am.type != ManagerType::Invalid);

                    if (am.type == ManagerType::DiffThermo && Num != availMgr.NumAvailManagers) {
                        ShowWarningError(state, format("GetZoneEqAvailabilityManager: AvailabilityManager:DifferentialThermostat=\"{}\".", am.Name));
                        ShowContinueError(
                            state, "...is not the last manager on the AvailabilityManagerAssignmentList.  Any remaining managers will not be used.");
                        ShowContinueError(state, format("Occurs in AvailabilityManagerAssignmentList=\"{}\".", am.Name));
                    }
                } // End of Num Loop
            }
            if (availMgr.Count > 0 || Found > 0) {
                availMgr.Input = false;
            }
            availMgr.Count += 1;
        }
    }

    void FillPredefinedTablesForAvailManager(EnergyPlusData &state)
    {
        // J. Glazer August 2025
        auto &orp = state.dataOutRptPredefined;
        auto &asd = state.dataAvail;
        for (int PriAirSysNum = 1; PriAirSysNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++PriAirSysNum) { // loop over the primary air systems
            auto &availMgr = state.dataAirLoop->PriAirSysAvailMgr(PriAirSysNum);
            for (int PriAirSysAvailMgrNum = 1; PriAirSysAvailMgrNum <= availMgr.NumAvailManagers; ++PriAirSysAvailMgrNum) {
                std::string availMgrName = availMgr.availManagers(PriAirSysAvailMgrNum).Name;
                std::string loopName = state.dataAirSystemsData->PrimaryAirSystems(PriAirSysNum).Name;
                int num = availMgr.availManagers(PriAirSysAvailMgrNum).Num;
                ManagerType availMgrType = availMgr.availManagers(PriAirSysAvailMgrNum).type;
                switch (availMgrType) {
                case ManagerType::Scheduled:
                case ManagerType::ScheduledOn:
                case ManagerType::ScheduledOff: {
                    OutputReportPredefined::PreDefTableEntry(state, orp->pdchAvlMgrSchType, loopName, managerTypeNames[(int)availMgrType]);
                    OutputReportPredefined::PreDefTableEntry(state, orp->pdchAvlMgrSchAvailNm, loopName, availMgrName);
                } break;
                default:
                    break;
                }
                switch (availMgrType) {
                case ManagerType::Scheduled: {
                    if (asd->SchedData[num - 1].availSched != nullptr) {
                        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAvlMgrSchSchNm, loopName, asd->SchedData[num - 1].availSched->Name);
                    }
                } break;
                case ManagerType::ScheduledOn: {
                    if (asd->SchedOnData[num - 1].availSched != nullptr) {
                        OutputReportPredefined::PreDefTableEntry(
                            state, orp->pdchAvlMgrSchSchNm, loopName, asd->SchedOnData[num - 1].availSched->Name);
                    }
                } break;
                case ManagerType::ScheduledOff: {
                    if (asd->SchedOffData[num - 1].availSched != nullptr) {
                        OutputReportPredefined::PreDefTableEntry(
                            state, orp->pdchAvlMgrSchSchNm, loopName, asd->SchedOffData[num - 1].availSched->Name);
                    }
                } break;
                default:
                    break;
                }
            }
        }
    }

    void InitSysAvailManagers(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   August 2001
        //       MODIFIED       Brent Griffith, CR8376 initialize to Status::NoAction every timestep

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the System Availability Manager objects.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations.

        using DataZoneEquipment::NumValidSysAvailZoneComponents;

        int ZoneListNum;
        int ScanZoneListNum;
        int ZoneNum;
        // One time initializations

        if (state.dataAvail->InitSysAvailManagers_MyOneTimeFlag) {

            for (int SysAvailNum = 1; SysAvailNum <= state.dataAvail->NumOptStartSysAvailMgrs; ++SysAvailNum) {
                auto &optimumStartMgr = state.dataAvail->OptimumStartData(SysAvailNum);
                if (optimumStartMgr.optimumStartControlType == OptimumStartControlType::MaximumOfZoneList) {
                    // a zone list
                    ZoneListNum = Util::FindItemInList(optimumStartMgr.ZoneListName, state.dataHeatBal->ZoneList);
                    if (ZoneListNum > 0) {
                        optimumStartMgr.NumOfZones = state.dataHeatBal->ZoneList(ZoneListNum).NumOfZones;
                        if (!allocated(optimumStartMgr.ZonePtrs)) {
                            optimumStartMgr.ZonePtrs.allocate({1, state.dataHeatBal->ZoneList(ZoneListNum).NumOfZones});
                        }
                        for (ScanZoneListNum = 1; ScanZoneListNum <= state.dataHeatBal->ZoneList(ZoneListNum).NumOfZones; ++ScanZoneListNum) {
                            ZoneNum = state.dataHeatBal->ZoneList(ZoneListNum).Zone(ScanZoneListNum);
                            optimumStartMgr.ZonePtrs(ScanZoneListNum) = ZoneNum;
                        }
                    }
                }
            }

            state.dataAvail->InitSysAvailManagers_MyOneTimeFlag = false;

        } // end 1 time initializations

        // initialize individual availability managers to no action (CR 8376 reporting issue)
        if (allocated(state.dataAvail->SchedData)) {
            for (auto &e : state.dataAvail->SchedData) {
                e.availStatus = Status::NoAction;
            }
        }
        if (allocated(state.dataAvail->SchedOnData)) {
            for (auto &e : state.dataAvail->SchedOnData) {
                e.availStatus = Status::NoAction;
            }
        }
        if (allocated(state.dataAvail->SchedOffData)) {
            for (auto &e : state.dataAvail->SchedOffData) {
                e.availStatus = Status::NoAction;
            }
        }
        if (allocated(state.dataAvail->NightCycleData)) {
            for (auto &e : state.dataAvail->NightCycleData) {
                e.availStatus = Status::NoAction;
            }
        }
        if (allocated(state.dataAvail->NightVentData)) {
            for (auto &e : state.dataAvail->NightVentData) {
                e.availStatus = Status::NoAction;
            }
        }
        if (allocated(state.dataAvail->DiffThermoData)) {
            for (auto &e : state.dataAvail->DiffThermoData) {
                e.availStatus = Status::NoAction;
            }
        }
        if (allocated(state.dataAvail->HiTurnOffData)) {
            for (auto &e : state.dataAvail->HiTurnOffData) {
                e.availStatus = Status::NoAction;
            }
        }
        if (allocated(state.dataAvail->HiTurnOnData)) {
            for (auto &e : state.dataAvail->HiTurnOnData) {
                e.availStatus = Status::NoAction;
            }
        }
        if (allocated(state.dataAvail->LoTurnOffData)) {
            for (auto &e : state.dataAvail->LoTurnOffData) {
                e.availStatus = Status::NoAction;
            }
        }
        if (allocated(state.dataAvail->LoTurnOnData)) {
            for (auto &e : state.dataAvail->LoTurnOnData) {
                e.availStatus = Status::NoAction;
            }
        }
        if (allocated(state.dataAvail->OptimumStartData)) {
            for (auto &e : state.dataAvail->OptimumStartData) {
                e.availStatus = Status::NoAction;
                e.isSimulated = false;
            }
        }
        //  HybridVentSysAvailMgrData%AvailStatus= Status::NoAction
        if (allocated(state.dataAvail->ZoneComp)) {
            for (int ZoneEquipType = 1; ZoneEquipType <= NumValidSysAvailZoneComponents; ++ZoneEquipType) { // loop over the zone equipment types
                if (state.dataAvail->ZoneComp(ZoneEquipType).TotalNumComp > 0) {
                    for (auto &e : state.dataAvail->ZoneComp(ZoneEquipType).ZoneCompAvailMgrs) {
                        e.availStatus = Status::NoAction;
                    }
                }
            }
        }
    }

    Status SimSysAvailManager(EnergyPlusData &state,
                              const ManagerType type,
                              std::string const &SysAvailName,
                              int &SysAvailNum,
                              int const PriAirSysNum, // Primary Air System index. If being called for a ZoneHVAC:* component
                              Status const previousStatus,
                              ObjexxFCL::Optional_int_const ZoneEquipType, // Type of ZoneHVAC:* equipment component
                              ObjexxFCL::Optional_int_const CompNum        // Index of ZoneHVAC:* equipment component
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   August 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE
        // Loop over all the System Availability Managers and invoke the correct
        // System Availability Manager algorithm.

        Status availStatus = Status::Invalid;

        switch (type) {
        case ManagerType::Scheduled: { // 'AvailabilityManager:Scheduled'
            if (SysAvailNum == 0) {
                SysAvailNum = Util::FindItemInList(SysAvailName, state.dataAvail->SchedData);
            }
            if (SysAvailNum > 0) {
                availStatus = CalcSchedSysAvailMgr(state, SysAvailNum);
            } else {
                ShowFatalError(state, format("SimSysAvailManager: AvailabilityManager:Scheduled not found: {}", SysAvailName));
            }

        } break;
        case ManagerType::ScheduledOn: { // 'AvailabilityManager:ScheduledOn'
            if (SysAvailNum == 0) {
                SysAvailNum = Util::FindItemInList(SysAvailName, state.dataAvail->SchedOnData);
            }
            if (SysAvailNum > 0) {
                availStatus = CalcSchedOnSysAvailMgr(state, SysAvailNum);
            } else {
                ShowFatalError(state, format("SimSysAvailManager: AvailabilityManager:ScheduledOn not found: {}", SysAvailName));
            }

        } break;
        case ManagerType::ScheduledOff: { // 'AvailabilityManager:ScheduledOff'
            if (SysAvailNum == 0) {
                SysAvailNum = Util::FindItemInList(SysAvailName, state.dataAvail->SchedOffData);
            }
            if (SysAvailNum > 0) {
                availStatus = CalcSchedOffSysAvailMgr(state, SysAvailNum);
            } else {
                ShowFatalError(state, format("SimSysAvailManager: AvailabilityManager:ScheduledOff not found: {}", SysAvailName));
            }

        } break;
        case ManagerType::NightCycle: { // 'AvailabilityManager:NightCycle'
            if (SysAvailNum == 0) {
                SysAvailNum = Util::FindItemInList(SysAvailName, state.dataAvail->NightCycleData);
            }
            if (SysAvailNum > 0) {
                availStatus = CalcNCycSysAvailMgr(state, SysAvailNum, PriAirSysNum, ZoneEquipType, CompNum);
            } else {
                ShowFatalError(state, format("SimSysAvailManager: AvailabilityManager:NightCycle not found: {}", SysAvailName));
            }

        } break;
        case ManagerType::OptimumStart: { // 'AvailabilityManager:OptimumStart'
            if (SysAvailNum == 0) {
                SysAvailNum = Util::FindItemInList(SysAvailName, state.dataAvail->OptimumStartData);
            }
            if (SysAvailNum > 0) {
                availStatus = CalcOptStartSysAvailMgr(state, SysAvailNum, PriAirSysNum, ZoneEquipType, CompNum);
            } else {
                ShowFatalError(state, format("SimSysAvailManager: AvailabilityManager:OptimumStart not found: {}", SysAvailName));
            }

        } break;
        case ManagerType::NightVent: { // 'AvailabilityManager:NightVentilation'
            if (SysAvailNum == 0) {
                SysAvailNum = Util::FindItemInList(SysAvailName, state.dataAvail->NightVentData);
            }
            if (SysAvailNum > 0) {
                availStatus = CalcNVentSysAvailMgr(state, SysAvailNum, PriAirSysNum, present(ZoneEquipType));
            } else {
                ShowFatalError(state, format("SimSysAvailManager: AvailabilityManager:NightVentilation not found: {}", SysAvailName));
            }

        } break;
        case ManagerType::DiffThermo: { // 'AvailabilityManager:DifferentialThermostat'
            if (SysAvailNum == 0) {
                SysAvailNum = Util::FindItemInList(SysAvailName, state.dataAvail->DiffThermoData);
            }
            if (SysAvailNum > 0) {
                availStatus = CalcDiffTSysAvailMgr(state, SysAvailNum, previousStatus);
            } else {
                ShowFatalError(state, format("SimSysAvailManager: AvailabilityManager:DifferentialThermostat not found: {}", SysAvailName));
            }
        } break;
        case ManagerType::HiTempTOff: { // 'AvailabilityManager:HighTemperatureTurnOff'
            if (SysAvailNum == 0) {
                SysAvailNum = Util::FindItemInList(SysAvailName, state.dataAvail->HiTurnOffData);
            }
            if (SysAvailNum > 0) {
                availStatus = CalcHiTurnOffSysAvailMgr(state, SysAvailNum);
            } else {
                ShowFatalError(state, format("SimSysAvailManager: AvailabilityManager:HighTemperatureTurnOff not found: {}", SysAvailName));
            }
        } break;
        case ManagerType::HiTempTOn: { // 'AvailabilityManager:HighTemperatureTurnOn'
            if (SysAvailNum == 0) {
                SysAvailNum = Util::FindItemInList(SysAvailName, state.dataAvail->HiTurnOnData);
            }
            if (SysAvailNum > 0) {
                availStatus = CalcHiTurnOnSysAvailMgr(state, SysAvailNum);
            } else {
                ShowFatalError(state, format("SimSysAvailManager: AvailabilityManager:HighTemperatureTurnOn not found: {}", SysAvailName));
            }
        } break;
        case ManagerType::LoTempTOff: { // 'AvailabilityManager:LowTemperatureTurnOff'
            if (SysAvailNum == 0) {
                SysAvailNum = Util::FindItemInList(SysAvailName, state.dataAvail->LoTurnOffData);
            }
            if (SysAvailNum > 0) {
                availStatus = CalcLoTurnOffSysAvailMgr(state, SysAvailNum);
            } else {
                ShowFatalError(state, format("SimSysAvailManager: AvailabilityManager:LowTemperatureTurnOff not found: {}", SysAvailName));
            }

        } break;
        case ManagerType::LoTempTOn: { // 'AvailabilityManager:LowTemperatureTurnOn'
            if (SysAvailNum == 0) {
                SysAvailNum = Util::FindItemInList(SysAvailName, state.dataAvail->LoTurnOnData);
            }
            if (SysAvailNum > 0) {
                availStatus = CalcLoTurnOnSysAvailMgr(state, SysAvailNum);
            } else {
                ShowFatalError(state, format("SimSysAvailManager: AvailabilityManager:LowTemperatureTurnOn not found: {}", SysAvailName));
            }

        } break;

        default: {
            ShowSevereError(state, format("AvailabilityManager Type not found: {}", type));
            ShowContinueError(state, format("Occurs in Manager={}", SysAvailName));
            ShowFatalError(state, "Preceding condition causes termination.");
        }
        }
        return availStatus;
    }

    Status CalcSchedSysAvailMgr(EnergyPlusData &state,
                                int const SysAvailNum // number of the current scheduled system availability manager
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   August 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set AvailStatus indicator for a primary air loop, plant loop or ZoneHVAC component.

        // METHODOLOGY EMPLOYED:
        // Looks at the System Availability Manager schedule and sets the
        // AvailStatus indicator accordingly. Mostly a useless algorithm
        // since the fan schedules can do the same thing.
        auto &availMgr = state.dataAvail->SchedData(SysAvailNum);
        availMgr.availStatus = (availMgr.availSched->getCurrentVal() > 0.0) ? Status::CycleOn : Status::ForceOff;
        return availMgr.availStatus;
    }

    Status CalcSchedOnSysAvailMgr(EnergyPlusData &state,
                                  int const SysAvailNum // number of the current scheduled on system availability manager
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         R. Raustad - FSEC
        //       DATE WRITTEN   August 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set AvailStatus indicator for a primary air loop, plant loop or ZoneHVAC component.

        // METHODOLOGY EMPLOYED:
        // Looks at the System Availability Manager schedule and sets the
        // AvailStatus indicator accordingly. If the schedule value is > 0
        // the availability status is Status::CycleOn, ELSE the status is Status::NoAction.
        auto &availMgr = state.dataAvail->SchedOnData(SysAvailNum);
        availMgr.availStatus = (availMgr.availSched->getCurrentVal() > 0.0) ? Status::CycleOn : Status::NoAction;
        return availMgr.availStatus;
    }

    Status CalcSchedOffSysAvailMgr(EnergyPlusData &state,
                                   int const SysAvailNum // number of the current scheduled off system availability manager
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         R. Raustad - FSEC
        //       DATE WRITTEN   August 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set AvailStatus indicator for a primary air loop, plant loop or ZoneHVAC component.

        // METHODOLOGY EMPLOYED:
        // Looks at the System Availability Manager schedule and sets the
        // AvailStatus indicator accordingly.  If the schedule value is = 0
        // the availability status is Status::ForceOff, ELSE the status is Status::NoAction.
        auto &availMgr = state.dataAvail->SchedOffData(SysAvailNum);
        availMgr.availStatus = (availMgr.availSched->getCurrentVal() == 0.0) ? Status::ForceOff : Status::NoAction;
        return availMgr.availStatus;
    }

    Status CalcNCycSysAvailMgr(EnergyPlusData &state,
                               int const SysAvailNum,                       // number of the current scheduled system availability manager
                               int const PriAirSysNum,                      // number of the primary air system affected by this Avail. Manager
                               ObjexxFCL::Optional_int_const ZoneEquipType, // Type of ZoneHVAC equipment component
                               ObjexxFCL::Optional_int_const CompNum        // Index of ZoneHVAC equipment component
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   August 2001
        //       MODIFIED       March 2011, Chandan Sharma - FSEC: Allowed night cycle
        //                             availability manager to work for ZoneHVAC component
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set AvailStatus indicator for a primary air loop or ZoneHVAC component.

        // METHODOLOGY EMPLOYED:
        // For air loop, depending on the type of control, looks at 1 named zone or all the zones
        // attached to a primary air system, compares zone temperature to the setup
        // or setback thermostat setpoint, and sets the AvailStaus indicator according
        // to whether the system needs to be cycled on or not.
        // For ZoneHVAC component, uses the exact same method as above but only looks at the
        // zone where component is located.
        int StartTime;
        int StopTime;
        int ZoneInSysNum;
        Real64 TempTol;
        auto &ZoneCompNCControlType = state.dataAvail->ZoneCompNCControlType;

        if (present(ZoneEquipType)) {
            auto &zoneComp = state.dataAvail->ZoneComp(ZoneEquipType);
            if (state.dataGlobal->WarmupFlag && state.dataGlobal->BeginDayFlag) {
                // reset start/stop times at beginning of each day during warmup to prevent non-convergence due to rotating start times
                zoneComp.ZoneCompAvailMgrs(CompNum).StartTime = state.dataGlobal->SimTimeSteps;
                zoneComp.ZoneCompAvailMgrs(CompNum).StopTime = state.dataGlobal->SimTimeSteps;
            }

            StartTime = zoneComp.ZoneCompAvailMgrs(CompNum).StartTime;
            StopTime = zoneComp.ZoneCompAvailMgrs(CompNum).StopTime;
            if (state.dataAvail->CalcNCycSysAvailMgr_OneTimeFlag) {
                ZoneCompNCControlType.dimension(state.dataAvail->NumNCycSysAvailMgrs, true);
                state.dataAvail->CalcNCycSysAvailMgr_OneTimeFlag = false;
            }
        } else {
            auto &availMgr = state.dataAirLoop->PriAirSysAvailMgr(PriAirSysNum);
            if (state.dataGlobal->WarmupFlag && state.dataGlobal->BeginDayFlag) {
                // reset start/stop times at beginning of each day during warmup to prevent non-convergence due to rotating start times
                availMgr.StartTime = state.dataGlobal->SimTimeSteps;
                availMgr.StopTime = state.dataGlobal->SimTimeSteps;
            }

            StartTime = availMgr.StartTime;
            StopTime = availMgr.StopTime;
        }

        // CR 7913 changed to allow during warmup
        auto &nightCycleMgr = state.dataAvail->NightCycleData(SysAvailNum);
        if ((nightCycleMgr.availSched->getCurrentVal() <= 0.0) || (nightCycleMgr.fanSched->getCurrentVal() > 0.0)) {
            return nightCycleMgr.availStatus = Status::NoAction; // CR 8358
        }

        TempTol = (nightCycleMgr.cyclingRunTimeControl == CyclingRunTimeControl::FixedRunTime) ? (0.5 * nightCycleMgr.TempTolRange) : 0.05;

        Status availStatus;

        if (present(ZoneEquipType)) {
            if (state.dataGlobal->SimTimeSteps >= StartTime && state.dataGlobal->SimTimeSteps < StopTime &&
                (nightCycleMgr.cyclingRunTimeControl == CyclingRunTimeControl::FixedRunTime ||
                 nightCycleMgr.cyclingRunTimeControl == CyclingRunTimeControl::ThermostatWithMinimumRunTime)) { // if cycled on
                availStatus = Status::CycleOn;
            } else if (state.dataGlobal->SimTimeSteps == StopTime &&
                       nightCycleMgr.cyclingRunTimeControl == CyclingRunTimeControl::FixedRunTime) { // if end of cycle run time, shut down if fan off
                availStatus = Status::NoAction;
            } else {

                switch (nightCycleMgr.nightCycleControlType) { // select type of night cycle control

                case NightCycleControlType::Off: {
                    availStatus = Status::NoAction;
                } break;
                case NightCycleControlType::OnControlZone: {

                    int ZoneNum = nightCycleMgr.CtrlZonePtrs(1);

                    auto const &zoneTstatSetpt = state.dataHeatBalFanSys->zoneTstatSetpts(ZoneNum);

                    switch (state.dataHeatBalFanSys->TempControlType(ZoneNum)) { // select on thermostat control

                    case HVAC::SetptType::SingleHeat: {
                        if (state.dataHeatBalFanSys->TempTstatAir(ZoneNum) < zoneTstatSetpt.setpt - TempTol) {
                            availStatus = Status::CycleOn;
                        } else {
                            availStatus = Status::NoAction;
                        }

                    } break;
                    case HVAC::SetptType::SingleCool: {
                        if (state.dataHeatBalFanSys->TempTstatAir(ZoneNum) > zoneTstatSetpt.setpt + TempTol) {
                            availStatus = Status::CycleOn;
                        } else {
                            availStatus = Status::NoAction;
                        }

                    } break;
                    case HVAC::SetptType::SingleHeatCool: {
                        if ((state.dataHeatBalFanSys->TempTstatAir(ZoneNum) < zoneTstatSetpt.setpt - TempTol) ||
                            (state.dataHeatBalFanSys->TempTstatAir(ZoneNum) > zoneTstatSetpt.setpt + TempTol)) {
                            availStatus = Status::CycleOn;
                        } else {
                            availStatus = Status::NoAction;
                        }

                    } break;
                    case HVAC::SetptType::DualHeatCool: {
                        if ((state.dataHeatBalFanSys->TempTstatAir(ZoneNum) < zoneTstatSetpt.setptLo - TempTol) ||
                            (state.dataHeatBalFanSys->TempTstatAir(ZoneNum) > zoneTstatSetpt.setptHi + TempTol)) {
                            availStatus = Status::CycleOn;
                        } else {
                            availStatus = Status::NoAction;
                        }

                    } break;
                    default: {
                        availStatus = Status::NoAction;
                    }
                    } // end select on thermostat control
                } break;
                case NightCycleControlType::OnAny:
                case NightCycleControlType::OnZoneFansOnly: {
                    if (ZoneCompNCControlType(SysAvailNum)) {
                        ShowWarningError(state,
                                         format("AvailabilityManager:NightCycle = {}, is specified for a ZoneHVAC component.", nightCycleMgr.Name));
                        ShowContinueError(state, "The only valid Control Types for ZoneHVAC components are Status::CycleOnControlZone and StayOff.");
                        ShowContinueError(state, "Night Cycle operation will not be modeled for ZoneHVAC components that reference this manager.");
                        ZoneCompNCControlType(SysAvailNum) = false;
                    }
                    availStatus = Status::NoAction;
                } break;
                default: {
                    availStatus = Status::NoAction;
                    break;
                }
                } // end select type of night cycle control

                if (availStatus == Status::CycleOn) { // reset the start and stop times
                    auto &zoneComp = state.dataAvail->ZoneComp(ZoneEquipType);
                    if (nightCycleMgr.cyclingRunTimeControl == CyclingRunTimeControl::Thermostat) { // Cycling Run Time is ignored
                        zoneComp.ZoneCompAvailMgrs(CompNum).StartTime = state.dataGlobal->SimTimeSteps;
                        zoneComp.ZoneCompAvailMgrs(CompNum).StopTime = state.dataGlobal->SimTimeSteps;
                    } else {
                        zoneComp.ZoneCompAvailMgrs(CompNum).StartTime = state.dataGlobal->SimTimeSteps;
                        zoneComp.ZoneCompAvailMgrs(CompNum).StopTime = state.dataGlobal->SimTimeSteps + nightCycleMgr.CyclingTimeSteps;
                    }
                }
            }
        } else {
            if (state.dataGlobal->SimTimeSteps >= StartTime && state.dataGlobal->SimTimeSteps < StopTime &&
                (nightCycleMgr.cyclingRunTimeControl == CyclingRunTimeControl::FixedRunTime ||
                 nightCycleMgr.cyclingRunTimeControl == CyclingRunTimeControl::ThermostatWithMinimumRunTime)) { // if cycled on
                availStatus = nightCycleMgr.priorAvailStatus;
                if (nightCycleMgr.nightCycleControlType == NightCycleControlType::OnZoneFansOnly) {
                    availStatus = Status::CycleOnZoneFansOnly;
                }
            } else if (state.dataGlobal->SimTimeSteps == StopTime &&
                       nightCycleMgr.cyclingRunTimeControl == CyclingRunTimeControl::FixedRunTime) { // if end of cycle run time, shut down if fan off
                availStatus = Status::NoAction;
            } else {

                switch (nightCycleMgr.nightCycleControlType) { // select type of night cycle control

                case NightCycleControlType::Off: {
                    availStatus = Status::NoAction;
                } break;
                case NightCycleControlType::OnAny:
                case NightCycleControlType::OnZoneFansOnly: {

                    // If no zones cooled, Availstatus could be "unknown"
                    availStatus = Status::NoAction;

                    for (ZoneInSysNum = 1; ZoneInSysNum <= state.dataAirLoop->AirToZoneNodeInfo(PriAirSysNum).NumZonesCooled;
                         ++ZoneInSysNum) { // loop over zones in system

                        int ZoneNum = state.dataAirLoop->AirToZoneNodeInfo(PriAirSysNum).CoolCtrlZoneNums(ZoneInSysNum);
                        auto const &zoneTstatSetpt = state.dataHeatBalFanSys->zoneTstatSetpts(ZoneNum);

                        switch (state.dataHeatBalFanSys->TempControlType(ZoneNum)) {
                        case HVAC::SetptType::SingleHeat: {
                            if (state.dataHeatBalFanSys->TempTstatAir(ZoneNum) < zoneTstatSetpt.setpt - TempTol) {
                                availStatus = Status::CycleOn;
                            } else {
                                availStatus = Status::NoAction;
                            }
                        } break;
                        case HVAC::SetptType::SingleCool: {
                            if (state.dataHeatBalFanSys->TempTstatAir(ZoneNum) > zoneTstatSetpt.setpt + TempTol) {
                                availStatus = Status::CycleOn;
                            } else {
                                availStatus = Status::NoAction;
                            }
                        } break;
                        case HVAC::SetptType::SingleHeatCool: {
                            if ((state.dataHeatBalFanSys->TempTstatAir(ZoneNum) < zoneTstatSetpt.setpt - TempTol) ||
                                (state.dataHeatBalFanSys->TempTstatAir(ZoneNum) > zoneTstatSetpt.setpt + TempTol)) {
                                availStatus = Status::CycleOn;
                            } else {
                                availStatus = Status::NoAction;
                            }
                        } break;
                        case HVAC::SetptType::DualHeatCool: {
                            if ((state.dataHeatBalFanSys->TempTstatAir(ZoneNum) < zoneTstatSetpt.setptLo - TempTol) ||
                                (state.dataHeatBalFanSys->TempTstatAir(ZoneNum) > zoneTstatSetpt.setptHi + TempTol)) {
                                availStatus = Status::CycleOn;
                            } else {
                                availStatus = Status::NoAction;
                            }
                        } break;
                        default: {
                            availStatus = Status::NoAction;
                        }
                        } // end select on thermostat control
                        if (availStatus == Status::CycleOn) {
                            break; // loop break
                        }
                    } // end loop over zones in system
                } break;

                case NightCycleControlType::OnControlZone: {
                    availStatus = Status::NoAction;
                    if (CoolingZoneOutOfTolerance(state, nightCycleMgr.CtrlZonePtrs, nightCycleMgr.NumOfCtrlZones, TempTol)) {
                        availStatus = Status::CycleOn;
                    }
                    if (HeatingZoneOutOfTolerance(state, nightCycleMgr.CtrlZonePtrs, nightCycleMgr.NumOfCtrlZones, TempTol)) {
                        availStatus = Status::CycleOn;
                    }
                } break;

                case NightCycleControlType::OnAnyCoolingOrHeatingZone: {
                    if (CoolingZoneOutOfTolerance(state, nightCycleMgr.CoolingZonePtrs, nightCycleMgr.NumOfCoolingZones, TempTol)) {
                        availStatus = Status::CycleOn;
                    } else if (HeatingZoneOutOfTolerance(state, nightCycleMgr.HeatingZonePtrs, nightCycleMgr.NumOfHeatingZones, TempTol)) {
                        availStatus = Status::CycleOn;
                    } else if (HeatingZoneOutOfTolerance(state, nightCycleMgr.HeatZnFanZonePtrs, nightCycleMgr.NumOfHeatZnFanZones, TempTol)) {
                        availStatus = Status::CycleOnZoneFansOnly;
                    } else {
                        availStatus = Status::NoAction;
                    }
                } break;

                case NightCycleControlType::OnAnyCoolingZone: {
                    if (CoolingZoneOutOfTolerance(state, nightCycleMgr.CoolingZonePtrs, nightCycleMgr.NumOfCoolingZones, TempTol)) {
                        availStatus = Status::CycleOn;
                    } else {
                        availStatus = Status::NoAction;
                    }
                } break;
                case NightCycleControlType::OnAnyHeatingZone: {
                    if (HeatingZoneOutOfTolerance(state, nightCycleMgr.HeatingZonePtrs, nightCycleMgr.NumOfHeatingZones, TempTol)) {
                        availStatus = Status::CycleOn;
                    } else if (HeatingZoneOutOfTolerance(state, nightCycleMgr.HeatZnFanZonePtrs, nightCycleMgr.NumOfHeatZnFanZones, TempTol)) {
                        availStatus = Status::CycleOnZoneFansOnly;
                    } else {
                        availStatus = Status::NoAction;
                    }
                } break;

                case NightCycleControlType::OnAnyHeatingZoneFansOnly: {
                    if (HeatingZoneOutOfTolerance(state, nightCycleMgr.HeatZnFanZonePtrs, nightCycleMgr.NumOfHeatZnFanZones, TempTol)) {
                        availStatus = Status::CycleOnZoneFansOnly;
                    } else {
                        availStatus = Status::NoAction;
                    }
                } break;

                default:
                    availStatus = Status::NoAction;
                } // end select type of night cycle control

                if ((availStatus == Status::CycleOn) || (availStatus == Status::CycleOnZoneFansOnly)) { // reset the start and stop times
                    if (nightCycleMgr.nightCycleControlType == NightCycleControlType::OnZoneFansOnly) {
                        availStatus = Status::CycleOnZoneFansOnly;
                    }
                    // issue #6151
                    auto &availMgr = state.dataAirLoop->PriAirSysAvailMgr(PriAirSysNum);
                    if (nightCycleMgr.cyclingRunTimeControl == CyclingRunTimeControl::Thermostat) { // Cycling Run Time is ignored
                        availMgr.StartTime = state.dataGlobal->SimTimeSteps;
                        availMgr.StopTime = state.dataGlobal->SimTimeSteps;
                    } else {
                        availMgr.StartTime = state.dataGlobal->SimTimeSteps;
                        availMgr.StopTime = state.dataGlobal->SimTimeSteps + nightCycleMgr.CyclingTimeSteps;
                    }
                }
            }
        }
        nightCycleMgr.availStatus = availStatus;
        nightCycleMgr.priorAvailStatus = availStatus;
        return availStatus;
    }

    bool CoolingZoneOutOfTolerance(EnergyPlusData &state,
                                   Array1D_int const ZonePtrList, // list of controlled zone pointers
                                   int const NumZones,            // number of zones in list
                                   Real64 const TempTolerance     // temperature tolerance
    )
    {

        // Check if any zone temperature is above the cooling setpoint plus tolerance
        for (int Index = 1; Index <= NumZones; ++Index) { // loop over zones in list
            int ZoneNum = ZonePtrList(Index);
            auto const &zoneTstatSetpt = state.dataHeatBalFanSys->zoneTstatSetpts(ZoneNum);

            switch (state.dataHeatBalFanSys->TempControlType(ZoneNum)) {
            case HVAC::SetptType::SingleCool:
            case HVAC::SetptType::SingleHeatCool:
                if (state.dataHeatBalFanSys->TempTstatAir(ZoneNum) > zoneTstatSetpt.setpt + TempTolerance) {
                    return true; // return on the first zone found
                }
                break;
            case HVAC::SetptType::DualHeatCool:
                if (state.dataHeatBalFanSys->TempTstatAir(ZoneNum) > zoneTstatSetpt.setptHi + TempTolerance) {
                    return true; // return on the first zone found
                }
                break;
            default:
                break;
            }
        }
        return false;
    }

    bool HeatingZoneOutOfTolerance(EnergyPlusData &state,
                                   Array1D_int const ZonePtrList, // list of controlled zone pointers
                                   int const NumZones,            // number of zones in list
                                   Real64 const TempTolerance     // temperature tolerance
    )
    {
        // Check if any zone temperature is below the heating setpoint less tolerance
        for (int Index = 1; Index <= NumZones; ++Index) { // loop over zones in list
            int ZoneNum = ZonePtrList(Index);
            { // Why is this a new scope?
                auto const &zoneTstatSetpt = state.dataHeatBalFanSys->zoneTstatSetpts(ZoneNum);

                HVAC::SetptType const tstatType(state.dataHeatBalFanSys->TempControlType(ZoneNum));

                if ((tstatType == HVAC::SetptType::SingleHeat) || (tstatType == HVAC::SetptType::SingleHeatCool)) {
                    if (state.dataHeatBalFanSys->TempTstatAir(ZoneNum) < zoneTstatSetpt.setpt - TempTolerance) {
                        return true; // return on the first zone found
                    }
                } else if (tstatType == HVAC::SetptType::DualHeatCool) {
                    if (state.dataHeatBalFanSys->TempTstatAir(ZoneNum) < zoneTstatSetpt.setptLo - TempTolerance) {
                        return true; // return on the first zone found
                    }
                }
            }
        }
        return false;
    }

    Status CalcOptStartSysAvailMgr(EnergyPlusData &state,
                                   int const SysAvailNum,  // number of the current scheduled system availability manager
                                   int const PriAirSysNum, // number of the primary air system affected by this Avail. Manager
                                   [[maybe_unused]] ObjexxFCL::Optional_int_const ZoneEquipType, // Type of ZoneHVAC equipment component
                                   [[maybe_unused]] ObjexxFCL::Optional_int_const CompNum        // Index of ZoneHVAC equipment component
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR            Xiufeng Pang (XP)
        //       DATE WRITTEN      August 2013
        //       MODIFIED
        //       RE-ENGINEERED

        // PURPOSE OF THIS SUBROUTINE:
        // Set AvailStatus indicator for a primary air loop, plant loop or ZoneHVAC component

        // METHODOLOGY EMPLOYED:
        // Sets the AvailStatus indicator according to the
        // optimum start algorithm

        // Using/Aliasing
        using namespace DataAirLoop;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        Array2D<Real64> DayValues;
        Array2D<Real64> DayValuesTmr;
        int JDay;
        int TmrJDay;
        int TmrDayOfWeek;
        int ZoneNum;
        Real64 FanStartTime;
        Real64 FanStartTimeTmr;
        Real64 PreStartTime;
        Real64 PreStartTimeTmr;
        Real64 DeltaTime;
        Real64 TempDiff;
        Real64 TempDiffHi;
        Real64 TempDiffLo;
        bool FirstTimeATGFlag(true);
        bool OverNightStartFlag(false); // Flag to indicate the optimum start starts before mid night.
        bool CycleOnFlag(false);
        bool OSReportVarFlag(true);
        int NumPreDays;
        int NumOfZonesInList;
        Real64 AdaTempGradHeat;
        Real64 AdaTempGradCool;
        Real64 ATGUpdateTime1(0.0);
        Real64 ATGUpdateTime2(0.0);
        Real64 ATGUpdateTemp1(0.0);
        Real64 ATGUpdateTemp2(0.0);
        bool ATGUpdateFlag1(false);
        bool ATGUpdateFlag2(false);
        int ATGCounter;
        int ATGWCZoneNumHi;
        int ATGWCZoneNumLo;
        Real64 NumHoursBeforeOccupancy; // Variable to store the number of hours before occupancy in optimum start period
        bool exitLoop;                  // exit loop on found data

        Status availStatus = Status::Invalid;

        auto &OptStartMgr = state.dataAvail->OptimumStartData(SysAvailNum);

        // some avail managers may be used in air loop and plant availability manager lists, if so they only need be simulated once
        if (OptStartMgr.isSimulated) {
            return OptStartMgr.availStatus;
        }
        OptStartMgr.isSimulated = true;

        // update air loop specific data
        TempDiffLo = OptStartMgr.TempDiffLo;
        TempDiffHi = OptStartMgr.TempDiffHi;
        ATGWCZoneNumLo = OptStartMgr.ATGWCZoneNumLo;
        ATGWCZoneNumHi = OptStartMgr.ATGWCZoneNumHi;
        CycleOnFlag = OptStartMgr.CycleOnFlag;
        ATGUpdateFlag1 = OptStartMgr.ATGUpdateFlag1;
        ATGUpdateFlag2 = OptStartMgr.ATGUpdateFlag2;
        NumHoursBeforeOccupancy = OptStartMgr.NumHoursBeforeOccupancy;
        FirstTimeATGFlag = OptStartMgr.FirstTimeATGFlag;
        OverNightStartFlag = OptStartMgr.OverNightStartFlag;
        OSReportVarFlag = OptStartMgr.OSReportVarFlag;

        if (OptStartMgr.controlAlgorithm == ControlAlgorithm::AdaptiveTemperatureGradient) {
            NumPreDays = OptStartMgr.NumPreDays;
            if (!allocated(state.dataAvail->OptStart_AdaTempGradTrdHeat)) {
                state.dataAvail->OptStart_AdaTempGradTrdHeat.allocate(NumPreDays);
                state.dataAvail->OptStart_AdaTempGradTrdCool.allocate(NumPreDays);
            }
            if (!allocated(OptStartMgr.AdaTempGradTrdHeat)) {
                OptStartMgr.AdaTempGradTrdHeat.allocate(NumPreDays);
                OptStartMgr.AdaTempGradTrdHeat = 0.0;
                OptStartMgr.AdaTempGradTrdCool.allocate(NumPreDays);
                OptStartMgr.AdaTempGradTrdCool = 0.0;
            }
            state.dataAvail->OptStart_AdaTempGradTrdHeat = OptStartMgr.AdaTempGradTrdHeat;
            state.dataAvail->OptStart_AdaTempGradTrdCool = OptStartMgr.AdaTempGradTrdCool;
            AdaTempGradHeat = OptStartMgr.AdaTempGradHeat;
            AdaTempGradCool = OptStartMgr.AdaTempGradCool;
            ATGUpdateTime1 = OptStartMgr.ATGUpdateTime1;
            ATGUpdateTime2 = OptStartMgr.ATGUpdateTime2;
            ATGUpdateTemp1 = OptStartMgr.ATGUpdateTemp1;
            ATGUpdateTemp2 = OptStartMgr.ATGUpdateTemp2;
        }

        // add or use a new variable OptStartSysAvailMgrData(SysAvailNum)%FanSchIndex
        if (state.dataGlobal->KickOffSimulation) {
            availStatus = Status::NoAction;
        } else {
            JDay = state.dataEnvrn->DayOfYear;
            TmrJDay = JDay + 1;
            TmrDayOfWeek = state.dataEnvrn->DayOfWeekTomorrow;

            DayValues.allocate(state.dataGlobal->TimeStepsInHour, Constant::iHoursInDay);
            DayValuesTmr.allocate(state.dataGlobal->TimeStepsInHour, Constant::iHoursInDay);
            if (!allocated(state.dataAvail->OptStart)) {
                state.dataAvail->OptStart.allocate(state.dataGlobal->NumOfZones);
            }

            // OptStartFlag needs to be reset each timestep to not stay set to true post-occupancy
            for (auto &optStart : state.dataAvail->OptStart) {
                optStart.OptStartFlag = false;
            }

            // reset OptStartData once per beginning of day
            if (state.dataGlobal->BeginDayFlag) {
                NumHoursBeforeOccupancy = 0.0; // Initialize the hours of optimum start period. This variable is for reporting purpose.
                if (state.dataAvail->BeginOfDayResetFlag) {
                    for (auto &optStart : state.dataAvail->OptStart) {
                        optStart.OccStartTime = 22.99; // initialize the zone occupancy start time
                    }
                    state.dataAvail->BeginOfDayResetFlag = false;
                }
            }
            if (!state.dataGlobal->BeginDayFlag) {
                state.dataAvail->BeginOfDayResetFlag = true;
            }

            std::vector<Real64> const &dayVals = OptStartMgr.fanSched->getDayVals(state);
            std::vector<Real64> const &tmwDayVals = OptStartMgr.fanSched->getDayVals(state, TmrJDay, TmrDayOfWeek);

            FanStartTime = 0.0;
            FanStartTimeTmr = 0.0;
            exitLoop = false;
            for (int hr = 0; hr < Constant::iHoursInDay; ++hr) {
                for (int ts = 0; ts <= state.dataGlobal->TimeStepsInHour; ++ts) {
                    if (dayVals[hr * state.dataGlobal->TimeStepsInHour + ts] <= 0.0) {
                        continue;
                    }
                    FanStartTime = hr + (1.0 / state.dataGlobal->TimeStepsInHour) * (ts + 1) - 0.01;
                    exitLoop = true;
                    break;
                }
                if (exitLoop) {
                    break;
                }
            }

            exitLoop = false;
            for (int hr = 0; hr < Constant::iHoursInDay; ++hr) {
                for (int ts = 0; ts < state.dataGlobal->TimeStepsInHour; ++ts) {
                    if (tmwDayVals[hr * state.dataGlobal->TimeStepsInHour + ts] <= 0.0) {
                        continue;
                    }
                    FanStartTimeTmr = hr + (1.0 / state.dataGlobal->TimeStepsInHour) * (ts + 1) - 0.01;
                    exitLoop = true;
                    break;
                }
                if (exitLoop) {
                    break;
                }
            }

            if (FanStartTimeTmr == 0.0) {
                FanStartTimeTmr = 24.0;
            }

            // Pass the start time to ZoneTempPredictorCorrector
            for (int counter = 1; counter <= state.dataAirLoop->AirToZoneNodeInfo(PriAirSysNum).NumZonesCooled; ++counter) {
                int actZoneNum = state.dataAirLoop->AirToZoneNodeInfo(PriAirSysNum).CoolCtrlZoneNums(counter);
                auto &optStart = state.dataAvail->OptStart(actZoneNum);
                optStart.OccStartTime = FanStartTime;
                optStart.ActualZoneNum = actZoneNum;
            }
            for (int counter = 1; counter <= state.dataAirLoop->AirToZoneNodeInfo(PriAirSysNum).NumZonesHeated; ++counter) {
                int actZoneNum = state.dataAirLoop->AirToZoneNodeInfo(PriAirSysNum).HeatCtrlZoneNums(counter);
                auto &optStart = state.dataAvail->OptStart(actZoneNum);
                optStart.OccStartTime = FanStartTime;
                optStart.ActualZoneNum = actZoneNum;
            }

            if (state.dataEnvrn->DSTIndicator > 0) {
                --FanStartTime;
                --FanStartTimeTmr;
            }

            switch (OptStartMgr.controlAlgorithm) {
            case ControlAlgorithm::ConstantStartTime: {
                if (OptStartMgr.optimumStartControlType == OptimumStartControlType::Off) {
                    availStatus = Status::NoAction;
                } else {
                    DeltaTime = OptStartMgr.ConstStartTime;
                    if (DeltaTime > OptStartMgr.MaxOptStartTime) {
                        DeltaTime = OptStartMgr.MaxOptStartTime;
                    }
                    PreStartTime = FanStartTime - DeltaTime;
                    if (PreStartTime < 0.0) {
                        PreStartTime = -0.1;
                    }
                    PreStartTimeTmr = FanStartTimeTmr - DeltaTime;
                    if (PreStartTimeTmr < 0.0) {
                        PreStartTimeTmr += 24.0;
                        OverNightStartFlag = true;
                    } else {
                        OverNightStartFlag = false;
                    }
                    if (!OverNightStartFlag) {
                        if (FanStartTime == 0.0 || state.dataGlobal->PreviousHour > FanStartTime) {
                            availStatus = Status::NoAction;
                            OSReportVarFlag = true;
                        } else if (PreStartTime < state.dataGlobal->CurrentTime) {
                            if (OSReportVarFlag) {
                                NumHoursBeforeOccupancy = DeltaTime;
                                OSReportVarFlag = false;
                            }
                            availStatus = Status::CycleOn;
                            OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                        } else {
                            availStatus = Status::NoAction;
                            OSReportVarFlag = true;
                        }
                    } else {
                        if (FanStartTime == 0.0 || (state.dataGlobal->HourOfDay > FanStartTime && state.dataGlobal->CurrentTime <= PreStartTimeTmr)) {
                            availStatus = Status::NoAction;
                            OSReportVarFlag = true;
                        } else if (PreStartTime < state.dataGlobal->CurrentTime || PreStartTimeTmr < state.dataGlobal->CurrentTime) {
                            if (OSReportVarFlag) {
                                NumHoursBeforeOccupancy = DeltaTime;
                                OSReportVarFlag = false;
                            }
                            availStatus = Status::CycleOn;
                            OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                        } else {
                            availStatus = Status::NoAction;
                            OSReportVarFlag = true;
                        }
                    }
                }
            } break;

            case ControlAlgorithm::ConstantTemperatureGradient: {
                if (OptStartMgr.optimumStartControlType == OptimumStartControlType::ControlZone) {
                    ZoneNum = OptStartMgr.ZoneNum;
                    if (!allocated(state.dataHeatBalFanSys->TempTstatAir) || !allocated(state.dataHeatBalFanSys->zoneTstatSetpts)) {
                        TempDiff = 0.0;
                    } else {
                        if (!CycleOnFlag) {
                            if (allocated(state.dataZoneCtrls->OccRoomTSetPointHeat) && allocated(state.dataZoneCtrls->OccRoomTSetPointCool)) {
                                TempDiffHi = state.dataHeatBalFanSys->TempTstatAir(ZoneNum) - state.dataZoneCtrls->OccRoomTSetPointCool(ZoneNum);
                                TempDiffLo = state.dataHeatBalFanSys->TempTstatAir(ZoneNum) - state.dataZoneCtrls->OccRoomTSetPointHeat(ZoneNum);
                            } else {
                                TempDiffHi = 0.0;
                                TempDiffLo = 0.0;
                            }
                        }
                    }

                    if (TempDiffHi < 0.0) {
                        TempDiff = TempDiffLo;
                        if (TempDiff < 0.0) { // Heating Mode
                            TempDiff = std::abs(TempDiff);
                            DeltaTime = TempDiff / OptStartMgr.ConstTGradHeat;
                            if (DeltaTime > OptStartMgr.MaxOptStartTime) {
                                DeltaTime = OptStartMgr.MaxOptStartTime;
                            }
                            PreStartTime = FanStartTime - DeltaTime;
                            if (PreStartTime < 0) {
                                PreStartTime = -0.1;
                            }
                            PreStartTimeTmr = FanStartTimeTmr - DeltaTime;
                            if (PreStartTimeTmr < 0) {
                                PreStartTimeTmr += 24.0;
                                OverNightStartFlag = true;
                            } else {
                                OverNightStartFlag = false;
                            }
                            if (!OverNightStartFlag) {
                                if (FanStartTime == 0.0 || state.dataGlobal->CurrentTime > FanStartTime) {
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                } else if (CycleOnFlag) {
                                    availStatus = Status::CycleOn;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                    if (state.dataGlobal->CurrentTime > FanStartTime) {
                                        CycleOnFlag = false;
                                    }
                                } else if (PreStartTime < state.dataGlobal->CurrentTime) {
                                    availStatus = Status::CycleOn;
                                    CycleOnFlag = true;
                                    if (OSReportVarFlag) {
                                        NumHoursBeforeOccupancy = DeltaTime;
                                        OSReportVarFlag = false;
                                    }
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                } else {
                                    availStatus = Status::NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                }
                            } else {
                                if (FanStartTime == 0.0 ||
                                    (state.dataGlobal->CurrentTime > FanStartTime && state.dataGlobal->CurrentTime <= PreStartTimeTmr)) {
                                    availStatus = Status::NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                } else if (CycleOnFlag) {
                                    availStatus = Status::CycleOn;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                    if (state.dataGlobal->CurrentTime > FanStartTime && state.dataGlobal->CurrentTime < PreStartTimeTmr) {
                                        CycleOnFlag = false;
                                    }
                                } else if (PreStartTime < state.dataGlobal->CurrentTime || PreStartTimeTmr < state.dataGlobal->CurrentTime) {
                                    if (OSReportVarFlag) {
                                        NumHoursBeforeOccupancy = DeltaTime;
                                        OSReportVarFlag = false;
                                    }
                                    availStatus = Status::CycleOn;
                                    CycleOnFlag = true;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                } else {
                                    availStatus = Status::NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                }
                            }
                        } else {
                            availStatus = Status::NoAction;
                            CycleOnFlag = false;
                        }
                    } else if (state.dataZoneCtrls->OccRoomTSetPointCool(ZoneNum) < 50.0) { // Cooling Mode
                        TempDiff = TempDiffHi;
                        DeltaTime = TempDiff / OptStartMgr.ConstTGradCool;
                        if (DeltaTime > OptStartMgr.MaxOptStartTime) {
                            DeltaTime = OptStartMgr.MaxOptStartTime;
                        }
                        PreStartTime = FanStartTime - DeltaTime;
                        if (PreStartTime < 0) {
                            PreStartTime = -0.1;
                        }
                        PreStartTimeTmr = FanStartTimeTmr - DeltaTime;
                        if (PreStartTimeTmr < 0) {
                            PreStartTimeTmr += 24.0;
                            OverNightStartFlag = true;
                        } else {
                            OverNightStartFlag = false;
                        }
                        if (!OverNightStartFlag) {
                            if (FanStartTime == 0.0 || state.dataGlobal->CurrentTime > FanStartTime) {
                                availStatus = Status::NoAction;
                                CycleOnFlag = false;
                                OSReportVarFlag = true;
                            } else if (CycleOnFlag) {
                                availStatus = Status::CycleOn;
                                OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                            } else if (PreStartTime < state.dataGlobal->CurrentTime) {
                                if (OSReportVarFlag) {
                                    NumHoursBeforeOccupancy = DeltaTime;
                                    OSReportVarFlag = false;
                                }
                                availStatus = Status::CycleOn;
                                CycleOnFlag = true;
                                OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                            } else {
                                availStatus = Status::NoAction;
                                CycleOnFlag = false;
                                OSReportVarFlag = true;
                            }
                        } else {
                            if (FanStartTime == 0.0 ||
                                (state.dataGlobal->CurrentTime > FanStartTime && state.dataGlobal->CurrentTime <= PreStartTimeTmr)) {
                                availStatus = Status::NoAction;
                                CycleOnFlag = false;
                                OSReportVarFlag = true;
                            } else if (CycleOnFlag) {
                                availStatus = Status::CycleOn;
                                OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                            } else if (PreStartTime < state.dataGlobal->CurrentTime || PreStartTimeTmr < state.dataGlobal->CurrentTime) {
                                if (OSReportVarFlag) {
                                    NumHoursBeforeOccupancy = DeltaTime;
                                    OSReportVarFlag = false;
                                }
                                availStatus = Status::CycleOn;
                                CycleOnFlag = true;
                                OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                            } else {
                                availStatus = Status::NoAction;
                                CycleOnFlag = false;
                                OSReportVarFlag = true;
                            }
                        }
                    } else {
                        availStatus = Status::NoAction;
                        CycleOnFlag = false;
                    }
                } else if (OptStartMgr.optimumStartControlType == OptimumStartControlType::MaximumOfZoneList) {

                    NumOfZonesInList = OptStartMgr.NumOfZones;
                    if (!allocated(state.dataHeatBalFanSys->TempTstatAir) || !allocated(state.dataHeatBalFanSys->zoneTstatSetpts)) {
                        TempDiff = 0.0;
                    } else {
                        if (!CycleOnFlag) {
                            if (allocated(state.dataZoneCtrls->OccRoomTSetPointHeat) && allocated(state.dataZoneCtrls->OccRoomTSetPointCool)) {
                                TempDiffHi = 0.0;
                                TempDiffLo = 0.0;
                                for (ZoneNum = 1; ZoneNum <= NumOfZonesInList; ++ZoneNum) {
                                    TempDiff = state.dataHeatBalFanSys->TempTstatAir(OptStartMgr.ZonePtrs(ZoneNum)) -
                                               state.dataZoneCtrls->OccRoomTSetPointCool(OptStartMgr.ZonePtrs(ZoneNum));
                                    TempDiffHi = max(TempDiffHi, TempDiff);
                                    TempDiff = state.dataHeatBalFanSys->TempTstatAir(OptStartMgr.ZonePtrs(ZoneNum)) -
                                               state.dataZoneCtrls->OccRoomTSetPointHeat(OptStartMgr.ZonePtrs(ZoneNum));
                                    TempDiffLo = min(TempDiffLo, TempDiff);
                                }
                            } else {
                                TempDiffHi = 0.0;
                                TempDiffLo = 0.0;
                            }
                        }
                    }
                    if ((TempDiffHi < 0.0 && TempDiffLo < 0.0) || (std::abs(TempDiffLo) > std::abs(TempDiffHi) && TempDiffLo < 0)) { // Heating Mode
                        TempDiff = TempDiffLo;
                        TempDiff = std::abs(TempDiff);
                        DeltaTime = TempDiff / OptStartMgr.ConstTGradHeat;
                        if (DeltaTime > OptStartMgr.MaxOptStartTime) {
                            DeltaTime = OptStartMgr.MaxOptStartTime;
                        }
                        PreStartTime = FanStartTime - DeltaTime;
                        if (PreStartTime < 0) {
                            PreStartTime = -0.1;
                        }
                        PreStartTimeTmr = FanStartTimeTmr - DeltaTime;
                        if (PreStartTimeTmr < 0) {
                            PreStartTimeTmr += 24.0;
                            OverNightStartFlag = true;
                        } else {
                            OverNightStartFlag = false;
                        }
                        if (!OverNightStartFlag) {
                            if (FanStartTime == 0.0 || state.dataGlobal->CurrentTime > FanStartTime) {
                                availStatus = Status::NoAction;
                                CycleOnFlag = false;
                                OSReportVarFlag = true;
                            } else if (CycleOnFlag) {
                                availStatus = Status::CycleOn;
                                OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                if (state.dataGlobal->CurrentTime > FanStartTime) {
                                    CycleOnFlag = false;
                                }
                            } else if (PreStartTime < state.dataGlobal->CurrentTime) {
                                if (OSReportVarFlag) {
                                    NumHoursBeforeOccupancy = DeltaTime;
                                    OSReportVarFlag = false;
                                }
                                availStatus = Status::CycleOn;
                                CycleOnFlag = true;
                                OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                            } else {
                                availStatus = Status::NoAction;
                                CycleOnFlag = false;
                                OSReportVarFlag = true;
                            }
                        } else {
                            if (FanStartTime == 0.0 ||
                                (state.dataGlobal->CurrentTime > FanStartTime && state.dataGlobal->CurrentTime <= PreStartTimeTmr)) {
                                availStatus = Status::NoAction;
                                CycleOnFlag = false;
                                OSReportVarFlag = true;
                            } else if (CycleOnFlag) {
                                availStatus = Status::CycleOn;
                                OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                if (state.dataGlobal->CurrentTime > FanStartTime && state.dataGlobal->CurrentTime < PreStartTimeTmr) {
                                    CycleOnFlag = false;
                                }
                            } else if (PreStartTime < state.dataGlobal->CurrentTime || PreStartTimeTmr < state.dataGlobal->CurrentTime) {
                                if (OSReportVarFlag) {
                                    NumHoursBeforeOccupancy = DeltaTime;
                                    OSReportVarFlag = false;
                                }
                                availStatus = Status::CycleOn;
                                CycleOnFlag = true;
                                OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                            } else {
                                availStatus = Status::NoAction;
                                CycleOnFlag = false;
                                OSReportVarFlag = true;
                            }
                        }
                    } else if (TempDiffHi <= 0.0 && TempDiffLo >= 0.0) { // not heating and not cooling
                        availStatus = Status::NoAction;
                        CycleOnFlag = false;
                        TempDiffHi = 0.0;
                        TempDiffLo = 0.0;
                    } else if (TempDiffHi < 30.0) { // Cooling Mode
                        TempDiff = TempDiffHi;
                        DeltaTime = TempDiff / OptStartMgr.ConstTGradCool;
                        if (DeltaTime > OptStartMgr.MaxOptStartTime) {
                            DeltaTime = OptStartMgr.MaxOptStartTime;
                        }
                        PreStartTime = FanStartTime - DeltaTime;
                        if (PreStartTime < 0) {
                            PreStartTime = -0.1;
                        }
                        PreStartTimeTmr = FanStartTimeTmr - DeltaTime;
                        if (PreStartTimeTmr < 0) {
                            PreStartTimeTmr += 24.0;
                            OverNightStartFlag = true;
                        } else {
                            OverNightStartFlag = false;
                        }
                        if (!OverNightStartFlag) {
                            if (FanStartTime == 0.0 || state.dataGlobal->CurrentTime > FanStartTime) {
                                availStatus = Status::NoAction;
                                CycleOnFlag = false;
                                OSReportVarFlag = true;
                            } else if (CycleOnFlag) {
                                availStatus = Status::CycleOn;
                                OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                            } else if (PreStartTime < state.dataGlobal->CurrentTime) {
                                if (OSReportVarFlag) {
                                    NumHoursBeforeOccupancy = DeltaTime;
                                    OSReportVarFlag = false;
                                }
                                availStatus = Status::CycleOn;
                                CycleOnFlag = true;
                                OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                            } else {
                                availStatus = Status::NoAction;
                                CycleOnFlag = false;
                                OSReportVarFlag = true;
                            }
                        } else {
                            if (FanStartTime == 0.0 ||
                                (state.dataGlobal->CurrentTime > FanStartTime && state.dataGlobal->CurrentTime <= PreStartTimeTmr)) {
                                availStatus = Status::NoAction;
                                CycleOnFlag = false;
                                OSReportVarFlag = true;
                            } else if (CycleOnFlag) {
                                availStatus = Status::CycleOn;
                                OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                            } else if (PreStartTime < state.dataGlobal->CurrentTime || PreStartTimeTmr < state.dataGlobal->CurrentTime) {
                                if (OSReportVarFlag) {
                                    NumHoursBeforeOccupancy = DeltaTime;
                                    OSReportVarFlag = false;
                                }
                                availStatus = Status::CycleOn;
                                CycleOnFlag = true;
                                OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                            } else {
                                availStatus = Status::NoAction;
                                CycleOnFlag = false;
                                OSReportVarFlag = true;
                            }
                        }
                    } else {
                        availStatus = Status::NoAction;
                        CycleOnFlag = false;
                    }
                } else {
                    availStatus = Status::NoAction;
                }
            } break;

            case ControlAlgorithm::AdaptiveTemperatureGradient: {

                if (OptStartMgr.optimumStartControlType == OptimumStartControlType::ControlZone) {
                    ZoneNum = OptStartMgr.ZoneNum;
                    if (!allocated(state.dataHeatBalFanSys->TempTstatAir) || !allocated(state.dataHeatBalFanSys->zoneTstatSetpts)) {
                        TempDiff = 0.0;
                    } else {
                        if (!CycleOnFlag) {
                            if (allocated(state.dataZoneCtrls->OccRoomTSetPointHeat) && allocated(state.dataZoneCtrls->OccRoomTSetPointCool)) {
                                TempDiffHi = state.dataHeatBalFanSys->TempTstatAir(ZoneNum) - state.dataZoneCtrls->OccRoomTSetPointCool(ZoneNum);
                                TempDiffLo = state.dataHeatBalFanSys->TempTstatAir(ZoneNum) - state.dataZoneCtrls->OccRoomTSetPointHeat(ZoneNum);
                            } else {
                                TempDiffHi = 0.0;
                                TempDiffLo = 0.0;
                            }
                        }
                    }
                    // Store adaptive temperature gradients for previous days and calculate the adaptive temp gradients
                    //-----------------------------------------------------------------------------
                    if (state.dataGlobal->WarmupFlag) {
                        AdaTempGradHeat = OptStartMgr.InitTGradHeat;
                        AdaTempGradCool = OptStartMgr.InitTGradCool;
                    } else if (state.dataGlobal->DayOfSim == 1 && state.dataGlobal->BeginDayFlag) {
                        state.dataAvail->OptStart_AdaTempGradTrdHeat = OptStartMgr.InitTGradHeat;
                        AdaTempGradHeat = OptStartMgr.InitTGradHeat;
                        state.dataAvail->OptStart_AdaTempGradTrdCool = OptStartMgr.InitTGradCool;
                        AdaTempGradCool = OptStartMgr.InitTGradCool;
                    } else {
                        if (state.dataGlobal->BeginDayFlag && FirstTimeATGFlag) {
                            FirstTimeATGFlag = false;
                            AdaTempGradHeat += state.dataAvail->OptStart_AdaTempGradTrdHeat(NumPreDays) / NumPreDays -
                                               state.dataAvail->OptStart_AdaTempGradTrdHeat(1) / NumPreDays;
                            AdaTempGradCool += state.dataAvail->OptStart_AdaTempGradTrdCool(NumPreDays) / NumPreDays -
                                               state.dataAvail->OptStart_AdaTempGradTrdCool(1) / NumPreDays;
                            if (FanStartTime > 0) {
                                for (ATGCounter = 1; ATGCounter <= NumPreDays - 1; ++ATGCounter) {
                                    state.dataAvail->OptStart_AdaTempGradTrdHeat(ATGCounter) =
                                        state.dataAvail->OptStart_AdaTempGradTrdHeat(ATGCounter + 1);
                                    state.dataAvail->OptStart_AdaTempGradTrdCool(ATGCounter) =
                                        state.dataAvail->OptStart_AdaTempGradTrdCool(ATGCounter + 1);
                                }
                            }
                        }
                    }

                    if (state.dataGlobal->CurrentTime >= 1.0) {
                        FirstTimeATGFlag = true;
                    }
                    //------------------------------------------------------------------------------

                    if (TempDiffHi < 0.0) {
                        TempDiff = TempDiffLo;
                        if (TempDiff < 0.0) { // Heating Mode
                            TempDiff = std::abs(TempDiff);
                            DeltaTime = TempDiff / AdaTempGradHeat;
                            if (DeltaTime > OptStartMgr.MaxOptStartTime) {
                                DeltaTime = OptStartMgr.MaxOptStartTime;
                            }
                            PreStartTime = FanStartTime - DeltaTime;
                            if (PreStartTime < 0.0) {
                                PreStartTime = -0.1;
                            }
                            PreStartTimeTmr = FanStartTimeTmr - DeltaTime;
                            if (PreStartTimeTmr < 0.0) {
                                PreStartTimeTmr += 24.0;
                                OverNightStartFlag = true;
                            } else {
                                OverNightStartFlag = false;
                            }
                            if (!OverNightStartFlag) {
                                if (FanStartTime == 0.0 || state.dataGlobal->CurrentTime > FanStartTime) {
                                    availStatus = Status::NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                } else if (CycleOnFlag) {
                                    availStatus = Status::CycleOn;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                    if (state.dataGlobal->CurrentTime > FanStartTime) {
                                        CycleOnFlag = false;
                                    }
                                    // Calculate the current day actual temperature gradient --------------------------
                                    if (!state.dataGlobal->WarmupFlag) {
                                        if (ATGUpdateFlag1) {
                                            ATGUpdateTime1 = state.dataGlobal->CurrentTime;
                                            ATGUpdateTemp1 = state.dataHeatBalFanSys->TempTstatAir(ZoneNum);
                                            ATGUpdateFlag1 = false;
                                        }
                                        if (state.dataHeatBalFanSys->TempTstatAir(ZoneNum) >= state.dataZoneCtrls->OccRoomTSetPointHeat(ZoneNum) &&
                                            ATGUpdateFlag2) {
                                            ATGUpdateTime2 = state.dataGlobal->CurrentTime;
                                            ATGUpdateTemp2 = state.dataHeatBalFanSys->TempTstatAir(ZoneNum);
                                            ATGUpdateFlag2 = false;
                                            if (std::abs(ATGUpdateTime2 - ATGUpdateTime1) > 1.e-10) {
                                                state.dataAvail->OptStart_AdaTempGradTrdHeat(NumPreDays) =
                                                    (ATGUpdateTemp2 - ATGUpdateTemp1) / (ATGUpdateTime2 - ATGUpdateTime1);
                                            } else {
                                                state.dataAvail->OptStart_AdaTempGradTrdHeat(NumPreDays) =
                                                    (ATGUpdateTemp2 - ATGUpdateTemp1) * state.dataGlobal->TimeStepsInHour;
                                            }
                                        }
                                    }
                                    //---------------------------------------------------------------------------------
                                } else if (PreStartTime < state.dataGlobal->CurrentTime) {
                                    if (OSReportVarFlag) {
                                        NumHoursBeforeOccupancy = DeltaTime;
                                        OSReportVarFlag = false;
                                    }
                                    availStatus = Status::CycleOn;
                                    CycleOnFlag = true;
                                    ATGUpdateFlag1 = true;
                                    ATGUpdateFlag2 = true;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                } else {
                                    availStatus = Status::NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                }
                            } else {
                                if (FanStartTime == 0.0 ||
                                    (state.dataGlobal->CurrentTime > FanStartTime && state.dataGlobal->CurrentTime <= PreStartTimeTmr)) {
                                    availStatus = Status::NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                } else if (CycleOnFlag) {
                                    availStatus = Status::CycleOn;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                    if (state.dataGlobal->CurrentTime > FanStartTime && state.dataGlobal->CurrentTime < PreStartTimeTmr) {
                                        CycleOnFlag = false;
                                    }
                                    // Calculate the current day actual temperature gradient --------------------------
                                    if (!state.dataGlobal->WarmupFlag) {
                                        if (ATGUpdateFlag1) {
                                            ATGUpdateTime1 = state.dataGlobal->CurrentTime;
                                            ATGUpdateTemp1 = state.dataHeatBalFanSys->TempTstatAir(ZoneNum);
                                            ATGUpdateFlag1 = false;
                                        }
                                        if (state.dataHeatBalFanSys->TempTstatAir(ZoneNum) >= state.dataZoneCtrls->OccRoomTSetPointHeat(ZoneNum) &&
                                            ATGUpdateFlag2) {
                                            ATGUpdateTime2 = state.dataGlobal->CurrentTime;
                                            ATGUpdateTemp2 = state.dataHeatBalFanSys->TempTstatAir(ZoneNum);
                                            ATGUpdateFlag2 = false;
                                            if (std::abs(ATGUpdateTime2 - ATGUpdateTime1 + 24.0) > 1.e-10) {
                                                state.dataAvail->OptStart_AdaTempGradTrdHeat(NumPreDays) =
                                                    (ATGUpdateTemp2 - ATGUpdateTemp1) / (ATGUpdateTime2 - ATGUpdateTime1 + 24.0);
                                            } else {
                                                state.dataAvail->OptStart_AdaTempGradTrdHeat(NumPreDays) =
                                                    (ATGUpdateTemp2 - ATGUpdateTemp1) * state.dataGlobal->TimeStepsInHour;
                                            }
                                        }
                                    }
                                    //---------------------------------------------------------------------------------
                                } else if (PreStartTime < state.dataGlobal->CurrentTime || PreStartTimeTmr < state.dataGlobal->CurrentTime) {
                                    if (OSReportVarFlag) {
                                        NumHoursBeforeOccupancy = DeltaTime;
                                        OSReportVarFlag = false;
                                    }
                                    availStatus = Status::CycleOn;
                                    CycleOnFlag = true;
                                    ATGUpdateFlag1 = true;
                                    ATGUpdateFlag2 = true;
                                    OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                } else {
                                    availStatus = Status::NoAction;
                                    CycleOnFlag = false;
                                    OSReportVarFlag = true;
                                }
                            }
                        } else {
                            availStatus = Status::NoAction;
                            CycleOnFlag = false;
                        }
                    } else if (state.dataZoneCtrls->OccRoomTSetPointCool(ZoneNum) < 50.0) { // Cooling Mode
                        TempDiff = TempDiffHi;
                        DeltaTime = TempDiff / AdaTempGradCool;
                        if (DeltaTime > OptStartMgr.MaxOptStartTime) {
                            DeltaTime = OptStartMgr.MaxOptStartTime;
                        }
                        PreStartTime = FanStartTime - DeltaTime;
                        if (PreStartTime < 0.0) {
                            PreStartTime = -0.1;
                        }
                        PreStartTimeTmr = FanStartTimeTmr - DeltaTime;
                        if (PreStartTimeTmr < 0.0) {
                            PreStartTimeTmr += 24.0;
                            OverNightStartFlag = true;
                        } else {
                            OverNightStartFlag = false;
                        }
                        if (!OverNightStartFlag) {
                            if (FanStartTime == 0.0 || state.dataGlobal->CurrentTime > FanStartTime) {
                                availStatus = Status::NoAction;
                                CycleOnFlag = false;
                                OSReportVarFlag = true;
                            } else if (CycleOnFlag) {
                                if (OSReportVarFlag) {
                                    NumHoursBeforeOccupancy = DeltaTime;
                                    OSReportVarFlag = false;
                                }
                                availStatus = Status::CycleOn;
                                OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                if (!state.dataGlobal->WarmupFlag) {
                                    if (ATGUpdateFlag1) {
                                        ATGUpdateTime1 = state.dataGlobal->CurrentTime;
                                        ATGUpdateTemp1 = state.dataHeatBalFanSys->TempTstatAir(ZoneNum);
                                        ATGUpdateFlag1 = false;
                                    }
                                    if (state.dataHeatBalFanSys->TempTstatAir(ZoneNum) <= state.dataZoneCtrls->OccRoomTSetPointCool(ZoneNum) &&
                                        ATGUpdateFlag2) {
                                        ATGUpdateTime2 = state.dataGlobal->CurrentTime;
                                        ATGUpdateTemp2 = state.dataHeatBalFanSys->TempTstatAir(ZoneNum);
                                        ATGUpdateFlag2 = false;
                                        if (std::abs(ATGUpdateTime2 - ATGUpdateTime1) > 1.e-10) {
                                            state.dataAvail->OptStart_AdaTempGradTrdCool(NumPreDays) =
                                                (ATGUpdateTemp1 - ATGUpdateTemp2) / (ATGUpdateTime2 - ATGUpdateTime1);
                                        } else {
                                            state.dataAvail->OptStart_AdaTempGradTrdCool(NumPreDays) =
                                                (ATGUpdateTemp1 - ATGUpdateTemp2) * state.dataGlobal->TimeStepsInHour;
                                        }
                                    }
                                }
                            } else if (PreStartTime < state.dataGlobal->CurrentTime) {
                                availStatus = Status::CycleOn;
                                CycleOnFlag = true;
                                ATGUpdateFlag1 = true;
                                ATGUpdateFlag2 = true;
                                OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                            } else {
                                availStatus = Status::NoAction;
                                CycleOnFlag = false;
                                OSReportVarFlag = true;
                            }
                        } else {
                            if (FanStartTime == 0.0 ||
                                (state.dataGlobal->CurrentTime > FanStartTime && state.dataGlobal->CurrentTime <= PreStartTimeTmr)) {
                                availStatus = Status::NoAction;
                                CycleOnFlag = false;
                                OSReportVarFlag = true;
                            } else if (CycleOnFlag) {
                                availStatus = Status::CycleOn;
                                if (!state.dataGlobal->WarmupFlag) {
                                    if (ATGUpdateFlag1) {
                                        ATGUpdateTime1 = state.dataGlobal->CurrentTime;
                                        ATGUpdateTemp1 = state.dataHeatBalFanSys->TempTstatAir(ZoneNum);
                                        ATGUpdateFlag1 = false;
                                    }
                                    if (state.dataHeatBalFanSys->TempTstatAir(ZoneNum) <= state.dataZoneCtrls->OccRoomTSetPointCool(ZoneNum) &&
                                        ATGUpdateFlag2) {
                                        ATGUpdateTime2 = state.dataGlobal->CurrentTime;
                                        ATGUpdateTemp2 = state.dataHeatBalFanSys->TempTstatAir(ZoneNum);
                                        ATGUpdateFlag2 = false;
                                        if (std::abs(ATGUpdateTime2 - ATGUpdateTime1 + 24.0) > 1.e-10) {
                                            state.dataAvail->OptStart_AdaTempGradTrdCool(NumPreDays) =
                                                (ATGUpdateTemp1 - ATGUpdateTemp2) / (ATGUpdateTime2 - ATGUpdateTime1 + 24.0);
                                        } else {
                                            state.dataAvail->OptStart_AdaTempGradTrdCool(NumPreDays) =
                                                (ATGUpdateTemp1 - ATGUpdateTemp2) * state.dataGlobal->TimeStepsInHour;
                                        }
                                    }
                                }
                                OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                            } else if (PreStartTime < state.dataGlobal->CurrentTime || PreStartTimeTmr < state.dataGlobal->CurrentTime) {
                                if (OSReportVarFlag) {
                                    NumHoursBeforeOccupancy = DeltaTime;
                                    OSReportVarFlag = false;
                                }
                                availStatus = Status::CycleOn;
                                CycleOnFlag = true;
                                ATGUpdateFlag1 = true;
                                ATGUpdateFlag2 = true;
                                OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                            } else {
                                availStatus = Status::NoAction;
                                CycleOnFlag = false;
                                OSReportVarFlag = true;
                            }
                        }
                    } else { // Not heating nor cooling mode
                        availStatus = Status::NoAction;
                        CycleOnFlag = false;
                    }
                } else if (OptStartMgr.optimumStartControlType == OptimumStartControlType::MaximumOfZoneList) {

                    NumOfZonesInList = OptStartMgr.NumOfZones;
                    ATGWCZoneNumHi = OptStartMgr.ZonePtrs(1);
                    ATGWCZoneNumLo = OptStartMgr.ZonePtrs(1);
                    if (!allocated(state.dataHeatBalFanSys->TempTstatAir) || !allocated(state.dataHeatBalFanSys->zoneTstatSetpts)) {
                        TempDiff = 0.0;
                    } else {
                        if (!CycleOnFlag) {
                            if (allocated(state.dataZoneCtrls->OccRoomTSetPointHeat) && allocated(state.dataZoneCtrls->OccRoomTSetPointCool)) {
                                TempDiffHi = 0.0;
                                TempDiffLo = 0.0;
                                ATGWCZoneNumHi = OptStartMgr.ZonePtrs(1);
                                ATGWCZoneNumLo = OptStartMgr.ZonePtrs(1);
                                for (ZoneNum = 1; ZoneNum <= NumOfZonesInList; ++ZoneNum) {
                                    TempDiff = state.dataHeatBalFanSys->TempTstatAir(OptStartMgr.ZonePtrs(ZoneNum)) -
                                               state.dataZoneCtrls->OccRoomTSetPointCool(OptStartMgr.ZonePtrs(ZoneNum));
                                    TempDiffHi = max(TempDiffHi, TempDiff);
                                    // Store the worse case zone number for actual temperature gradient calculation
                                    if (TempDiff == TempDiffHi) {
                                        ATGWCZoneNumHi = OptStartMgr.ZonePtrs(ZoneNum);
                                    }
                                    TempDiff = state.dataHeatBalFanSys->TempTstatAir(OptStartMgr.ZonePtrs(ZoneNum)) -
                                               state.dataZoneCtrls->OccRoomTSetPointHeat(OptStartMgr.ZonePtrs(ZoneNum));
                                    TempDiffLo = min(TempDiffLo, TempDiff);
                                    if (TempDiff == TempDiffLo) {
                                        ATGWCZoneNumLo = OptStartMgr.ZonePtrs(ZoneNum);
                                    }
                                }
                            } else {
                                TempDiffHi = 0.0;
                                TempDiffLo = 0.0;
                            }
                        }
                    }
                    // Store adaptive temperature gradients for previous days and calculate the adaptive temp gradients
                    //-----------------------------------------------------------------------------
                    if (state.dataGlobal->WarmupFlag) {
                        AdaTempGradHeat = OptStartMgr.InitTGradHeat;
                        AdaTempGradCool = OptStartMgr.InitTGradCool;
                    } else if (state.dataGlobal->DayOfSim == 1 && state.dataGlobal->BeginDayFlag) {
                        state.dataAvail->OptStart_AdaTempGradTrdHeat = OptStartMgr.InitTGradHeat;
                        AdaTempGradHeat = OptStartMgr.InitTGradHeat;
                        state.dataAvail->OptStart_AdaTempGradTrdCool = OptStartMgr.InitTGradCool;
                        AdaTempGradCool = OptStartMgr.InitTGradCool;
                    } else {
                        if (state.dataGlobal->BeginDayFlag && FirstTimeATGFlag) {
                            FirstTimeATGFlag = false;
                            AdaTempGradHeat += state.dataAvail->OptStart_AdaTempGradTrdHeat(NumPreDays) / NumPreDays -
                                               state.dataAvail->OptStart_AdaTempGradTrdHeat(1) / NumPreDays;
                            AdaTempGradCool += state.dataAvail->OptStart_AdaTempGradTrdCool(NumPreDays) / NumPreDays -
                                               state.dataAvail->OptStart_AdaTempGradTrdCool(1) / NumPreDays;
                            if (FanStartTime > 0) {
                                for (ATGCounter = 1; ATGCounter <= NumPreDays - 1; ++ATGCounter) {
                                    state.dataAvail->OptStart_AdaTempGradTrdHeat(ATGCounter) =
                                        state.dataAvail->OptStart_AdaTempGradTrdHeat(ATGCounter + 1);
                                    state.dataAvail->OptStart_AdaTempGradTrdCool(ATGCounter) =
                                        state.dataAvail->OptStart_AdaTempGradTrdCool(ATGCounter + 1);
                                }
                            }
                        }
                    }

                    if (state.dataGlobal->CurrentTime >= 1.0) {
                        FirstTimeATGFlag = true;
                    }
                    //------------------------------------------------------------------------------

                    if ((TempDiffHi < 0.0 && TempDiffLo < 0.0) || (std::abs(TempDiffLo) > std::abs(TempDiffHi) && TempDiffLo < 0.0)) { // Heating Mode
                        TempDiff = TempDiffLo;
                        TempDiff = std::abs(TempDiff);
                        DeltaTime = TempDiff / AdaTempGradHeat;
                        if (DeltaTime > OptStartMgr.MaxOptStartTime) {
                            DeltaTime = OptStartMgr.MaxOptStartTime;
                        }
                        PreStartTime = FanStartTime - DeltaTime;
                        if (PreStartTime < 0.0) {
                            PreStartTime = -0.1;
                        }
                        PreStartTimeTmr = FanStartTimeTmr - DeltaTime;
                        if (PreStartTimeTmr < 0.0) {
                            PreStartTimeTmr += 24.0;
                            OverNightStartFlag = true;
                        } else {
                            OverNightStartFlag = false;
                        }
                        if (!OverNightStartFlag) {
                            if (FanStartTime == 0.0 || state.dataGlobal->CurrentTime > FanStartTime) {
                                OSReportVarFlag = true;
                                availStatus = Status::NoAction;
                                CycleOnFlag = false;
                            } else if (CycleOnFlag) {
                                availStatus = Status::CycleOn;
                                OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                if (state.dataGlobal->CurrentTime > FanStartTime) {
                                    CycleOnFlag = false;
                                }
                                // Calculate the current day actual temperature gradient --------------------------
                                if (!state.dataGlobal->WarmupFlag) {
                                    if (ATGUpdateFlag1) {
                                        ATGUpdateTime1 = state.dataGlobal->CurrentTime;
                                        ATGUpdateTemp1 = state.dataHeatBalFanSys->TempTstatAir(ATGWCZoneNumLo);
                                        ATGUpdateFlag1 = false;
                                    }
                                    if (state.dataHeatBalFanSys->TempTstatAir(ATGWCZoneNumLo) >=
                                            state.dataZoneCtrls->OccRoomTSetPointHeat(ATGWCZoneNumLo) &&
                                        ATGUpdateFlag2) {
                                        ATGUpdateTime2 = state.dataGlobal->CurrentTime;
                                        ATGUpdateTemp2 = state.dataHeatBalFanSys->TempTstatAir(ATGWCZoneNumLo);
                                        ATGUpdateFlag2 = false;
                                        if (std::abs(ATGUpdateTime2 - ATGUpdateTime1) > 1.e-10) {
                                            state.dataAvail->OptStart_AdaTempGradTrdHeat(NumPreDays) =
                                                (ATGUpdateTemp2 - ATGUpdateTemp1) / (ATGUpdateTime2 - ATGUpdateTime1);
                                        } else {
                                            state.dataAvail->OptStart_AdaTempGradTrdHeat(NumPreDays) =
                                                (ATGUpdateTemp2 - ATGUpdateTemp1) * state.dataGlobal->TimeStepsInHour;
                                        }
                                    }
                                }
                                //---------------------------------------------------------------------------------
                            } else if (PreStartTime < state.dataGlobal->CurrentTime) {
                                if (OSReportVarFlag) {
                                    NumHoursBeforeOccupancy = DeltaTime;
                                    OSReportVarFlag = false;
                                }
                                availStatus = Status::CycleOn;
                                CycleOnFlag = true;
                                ATGUpdateFlag1 = true;
                                ATGUpdateFlag2 = true;
                                OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                            } else {
                                availStatus = Status::NoAction;
                                CycleOnFlag = false;
                                OSReportVarFlag = true;
                            }
                        } else {
                            if (FanStartTime == 0.0 ||
                                (state.dataGlobal->CurrentTime > FanStartTime && state.dataGlobal->CurrentTime <= PreStartTimeTmr)) {
                                availStatus = Status::NoAction;
                                CycleOnFlag = false;
                                OSReportVarFlag = true;
                            } else if (CycleOnFlag) {
                                availStatus = Status::CycleOn;
                                // Calculate the current day actual temperature gradient --------------------------
                                if (!state.dataGlobal->WarmupFlag) {
                                    if (ATGUpdateFlag1) {
                                        ATGUpdateTime1 = state.dataGlobal->CurrentTime;
                                        ATGUpdateTemp1 = state.dataHeatBalFanSys->TempTstatAir(ATGWCZoneNumLo);
                                        ATGUpdateFlag1 = false;
                                    }
                                    if (state.dataHeatBalFanSys->TempTstatAir(ATGWCZoneNumLo) >=
                                            state.dataZoneCtrls->OccRoomTSetPointHeat(ATGWCZoneNumLo) &&
                                        ATGUpdateFlag2) {
                                        ATGUpdateTime2 = state.dataGlobal->CurrentTime;
                                        ATGUpdateTemp2 = state.dataHeatBalFanSys->TempTstatAir(ATGWCZoneNumLo);
                                        ATGUpdateFlag2 = false;
                                        if (std::abs(ATGUpdateTime2 - ATGUpdateTime1 + 24.0) > 1.e-10) {
                                            state.dataAvail->OptStart_AdaTempGradTrdHeat(NumPreDays) =
                                                (ATGUpdateTemp2 - ATGUpdateTemp1) / (ATGUpdateTime2 - ATGUpdateTime1 + 24.0);
                                        } else {
                                            state.dataAvail->OptStart_AdaTempGradTrdHeat(NumPreDays) =
                                                (ATGUpdateTemp2 - ATGUpdateTemp1) * state.dataGlobal->TimeStepsInHour;
                                        }
                                    }
                                }
                                //---------------------------------------------------------------------------------
                                OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                                if (state.dataGlobal->CurrentTime > FanStartTime && state.dataGlobal->CurrentTime < PreStartTimeTmr) {
                                    CycleOnFlag = false;
                                }
                            } else if (PreStartTime < state.dataGlobal->CurrentTime || PreStartTimeTmr < state.dataGlobal->CurrentTime) {
                                if (OSReportVarFlag) {
                                    NumHoursBeforeOccupancy = DeltaTime;
                                    OSReportVarFlag = false;
                                }
                                availStatus = Status::CycleOn;
                                CycleOnFlag = true;
                                ATGUpdateFlag1 = true;
                                ATGUpdateFlag2 = true;
                                OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                            } else {
                                availStatus = Status::NoAction;
                                CycleOnFlag = false;
                                OSReportVarFlag = true;
                            }
                        }
                    } else if (TempDiffHi <= 0.0 && TempDiffLo >= 0.0) { // not heating and not cooling
                        availStatus = Status::NoAction;
                        CycleOnFlag = false;
                        TempDiffHi = 0.0;
                        TempDiffLo = 0.0;
                    } else if (TempDiffHi < 30.0) { // Cooling Mode
                        TempDiff = TempDiffHi;
                        DeltaTime = TempDiff / AdaTempGradCool;
                        if (DeltaTime > OptStartMgr.MaxOptStartTime) {
                            DeltaTime = OptStartMgr.MaxOptStartTime;
                        }
                        PreStartTime = FanStartTime - DeltaTime;
                        if (PreStartTime < 0) {
                            PreStartTime = -0.1;
                        }
                        PreStartTimeTmr = FanStartTimeTmr - DeltaTime;
                        if (PreStartTimeTmr < 0) {
                            PreStartTimeTmr += 24.0;
                            OverNightStartFlag = true;
                        } else {
                            OverNightStartFlag = false;
                        }
                        if (!OverNightStartFlag) {
                            if (FanStartTime == 0.0 || state.dataGlobal->CurrentTime > FanStartTime) {
                                availStatus = Status::NoAction;
                                CycleOnFlag = false;
                                OSReportVarFlag = true;
                            } else if (CycleOnFlag) {
                                availStatus = Status::CycleOn;
                                // Calculate the current day actual temperature gradient --------------------------
                                if (!state.dataGlobal->WarmupFlag) {
                                    if (ATGUpdateFlag1) {
                                        ATGUpdateTime1 = state.dataGlobal->CurrentTime;
                                        ATGUpdateTemp1 = state.dataHeatBalFanSys->TempTstatAir(ATGWCZoneNumHi);
                                        ATGUpdateFlag1 = false;
                                    }
                                    if (state.dataHeatBalFanSys->TempTstatAir(ATGWCZoneNumHi) <=
                                            state.dataZoneCtrls->OccRoomTSetPointCool(ATGWCZoneNumHi) &&
                                        ATGUpdateFlag2) {
                                        ATGUpdateTime2 = state.dataGlobal->CurrentTime;
                                        ATGUpdateTemp2 = state.dataHeatBalFanSys->TempTstatAir(ATGWCZoneNumHi);
                                        ATGUpdateFlag2 = false;
                                        if (std::abs(ATGUpdateTime2 - ATGUpdateTime1) > 1.e-10) {
                                            state.dataAvail->OptStart_AdaTempGradTrdCool(NumPreDays) =
                                                (ATGUpdateTemp1 - ATGUpdateTemp2) / (ATGUpdateTime2 - ATGUpdateTime1);
                                        } else {
                                            state.dataAvail->OptStart_AdaTempGradTrdCool(NumPreDays) =
                                                (ATGUpdateTemp1 - ATGUpdateTemp2) * state.dataGlobal->TimeStepsInHour;
                                        }
                                    }
                                }
                                //---------------------------------------------------------------------------------
                                OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                            } else if (PreStartTime < state.dataGlobal->CurrentTime) {
                                if (OSReportVarFlag) {
                                    NumHoursBeforeOccupancy = DeltaTime;
                                    OSReportVarFlag = false;
                                }
                                availStatus = Status::CycleOn;
                                CycleOnFlag = true;
                                ATGUpdateFlag1 = true;
                                ATGUpdateFlag2 = true;
                                OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                            } else {
                                availStatus = Status::NoAction;
                                CycleOnFlag = false;
                                OSReportVarFlag = true;
                            }
                        } else {
                            if (FanStartTime == 0.0 ||
                                (state.dataGlobal->CurrentTime > FanStartTime && state.dataGlobal->CurrentTime <= PreStartTimeTmr)) {
                                availStatus = Status::NoAction;
                                CycleOnFlag = false;
                                OSReportVarFlag = true;
                            } else if (CycleOnFlag) {
                                availStatus = Status::CycleOn;
                                // Calculate the current day actual temperature gradient --------------------------
                                if (!state.dataGlobal->WarmupFlag) {
                                    if (ATGUpdateFlag1) {
                                        ATGUpdateTime1 = state.dataGlobal->CurrentTime;
                                        ATGUpdateTemp1 = state.dataHeatBalFanSys->TempTstatAir(ATGWCZoneNumHi);
                                        ATGUpdateFlag1 = false;
                                    }
                                    if (state.dataHeatBalFanSys->TempTstatAir(ATGWCZoneNumHi) <=
                                            state.dataZoneCtrls->OccRoomTSetPointCool(ATGWCZoneNumHi) &&
                                        ATGUpdateFlag2) {
                                        ATGUpdateTime2 = state.dataGlobal->CurrentTime;
                                        ATGUpdateTemp2 = state.dataHeatBalFanSys->TempTstatAir(ATGWCZoneNumHi);
                                        ATGUpdateFlag2 = false;
                                        if (std::abs(ATGUpdateTime2 - ATGUpdateTime1 + 24.0) > 1.e-10) {
                                            state.dataAvail->OptStart_AdaTempGradTrdCool(NumPreDays) =
                                                (ATGUpdateTemp1 - ATGUpdateTemp2) / (ATGUpdateTime2 - ATGUpdateTime1 + 24.0);
                                        } else {
                                            state.dataAvail->OptStart_AdaTempGradTrdCool(NumPreDays) =
                                                (ATGUpdateTemp1 - ATGUpdateTemp2) * state.dataGlobal->TimeStepsInHour;
                                        }
                                    }
                                }
                                //---------------------------------------------------------------------------------
                                OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                            } else if (PreStartTime < state.dataGlobal->CurrentTime || PreStartTimeTmr < state.dataGlobal->CurrentTime) {
                                if (OSReportVarFlag) {
                                    NumHoursBeforeOccupancy = DeltaTime;
                                    OSReportVarFlag = false;
                                }
                                availStatus = Status::CycleOn;
                                CycleOnFlag = true;
                                ATGUpdateFlag2 = true;
                                ATGUpdateFlag1 = true;
                                OptStartMgr.SetOptStartFlag(state, PriAirSysNum);
                            } else {
                                availStatus = Status::NoAction;
                                CycleOnFlag = false;
                                OSReportVarFlag = true;
                            }
                        }
                    } else {
                        availStatus = Status::NoAction;
                        CycleOnFlag = false;
                    }
                } else {
                    availStatus = Status::NoAction;
                }
            } break;
            case ControlAlgorithm::AdaptiveASHRAE: {
                availStatus = Status::NoAction;
            } break;
            default:
                break;
            }
        }

        OptStartMgr.availStatus = availStatus;
        OptStartMgr.NumHoursBeforeOccupancy = NumHoursBeforeOccupancy;
        OptStartMgr.TempDiffLo = TempDiffLo;
        OptStartMgr.TempDiffHi = TempDiffHi;
        OptStartMgr.ATGWCZoneNumLo = ATGWCZoneNumLo;
        OptStartMgr.ATGWCZoneNumHi = ATGWCZoneNumHi;
        OptStartMgr.CycleOnFlag = CycleOnFlag;
        OptStartMgr.ATGUpdateFlag1 = ATGUpdateFlag1;
        OptStartMgr.ATGUpdateFlag2 = ATGUpdateFlag2;
        OptStartMgr.FirstTimeATGFlag = FirstTimeATGFlag;
        OptStartMgr.OverNightStartFlag = OverNightStartFlag;
        OptStartMgr.OSReportVarFlag = OSReportVarFlag;
        if (OptStartMgr.controlAlgorithm == ControlAlgorithm::AdaptiveTemperatureGradient) {
            OptStartMgr.AdaTempGradTrdHeat = state.dataAvail->OptStart_AdaTempGradTrdHeat;
            OptStartMgr.AdaTempGradTrdCool = state.dataAvail->OptStart_AdaTempGradTrdCool;
            OptStartMgr.AdaTempGradHeat = AdaTempGradHeat;
            OptStartMgr.AdaTempGradCool = AdaTempGradCool;
            OptStartMgr.ATGUpdateTime1 = ATGUpdateTime1;
            OptStartMgr.ATGUpdateTime2 = ATGUpdateTime2;
            OptStartMgr.ATGUpdateTemp1 = ATGUpdateTemp1;
            OptStartMgr.ATGUpdateTemp2 = ATGUpdateTemp2;
        }

        return availStatus;
    }

    void SysAvailManagerOptimumStart::SetOptStartFlag(EnergyPlusData &state, int const AirLoopNum)
    {
        // Set the OptStartFlag true for all zones on the air loop
        auto const &thisAirToZoneNodeInfo = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum);
        for (int counter = 1; counter <= thisAirToZoneNodeInfo.NumZonesCooled; ++counter) {
            state.dataAvail->OptStart(thisAirToZoneNodeInfo.CoolCtrlZoneNums(counter)).OptStartFlag = true;
        }
        for (int counter = 1; counter <= thisAirToZoneNodeInfo.NumZonesHeated; ++counter) {
            state.dataAvail->OptStart(thisAirToZoneNodeInfo.HeatCtrlZoneNums(counter)).OptStartFlag = true;
        }
    }

    Status CalcNVentSysAvailMgr(EnergyPlusData &state,
                                int const SysAvailNum,     // number of the current scheduled system availability manager
                                int const PriAirSysNum,    // number of the primary air system affected by this Avail. Manager
                                bool const isZoneEquipType // Type of zone equipment component
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   December 2004
        //       MODIFIED       March 2011, Chandan Sharma - FSEC: Allowed night ventilation
        //                             availability manager to work for zone component
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set AvailStatus indicator for a primary air loop and ZoneHVAC component and sets a specified flow
        // rate fraction for the air loop for use during night ventilation.

        // METHODOLOGY EMPLOYED:
        // Looks at outside and indoor conditions to determine if night ventilation
        // is beneficial. If it is and it is scheduled on the AvailStatus is set to cycle
        // on and the loop flow rate fractionis set to the specified night ventilation
        // value.

        using namespace DataAirLoop;

        int ZoneInSysNum;
        bool TempCheck;   // TRUE if one zone's temperature is above the value of the vent temp sched
        bool DelTCheck;   // TRUE if the control zone temperature - outside temperature > VentDelT
        bool LowLimCheck; // TRUE if one zones's air temperature is below this value
        Real64 VentTemp;  // value of the ventilation temperature schedule

        Status availStatus;

        TempCheck = false;
        DelTCheck = false;
        LowLimCheck = false;
        // check if night venting allowed: not allowed if avail sched is off or fan sched is on
        // CR 7913 changed to allow during warmup
        auto &nightVentMgr = state.dataAvail->NightVentData(SysAvailNum);
        if ((nightVentMgr.availSched->getCurrentVal() <= 0.0) || (nightVentMgr.fanSched->getCurrentVal() > 0.0)) {
            availStatus = Status::NoAction;
        } else {

            VentTemp = nightVentMgr.ventTempSched->getCurrentVal();
            int ControlZoneNum = nightVentMgr.ZoneNum;

            if (isZoneEquipType) {
                // if the room temperature is greater than the vent temp sched value, set the vent temp check to TRUE
                if (state.dataHeatBalFanSys->TempTstatAir(ControlZoneNum) > VentTemp) {
                    TempCheck = true;
                }
                // if the room temperature is less than the low limit set the low limit check to TRUE
                if (state.dataHeatBalFanSys->TempTstatAir(ControlZoneNum) < nightVentMgr.VentTempLowLim) {
                    LowLimCheck = true;
                }
            } else {
                for (ZoneInSysNum = 1; ZoneInSysNum <= state.dataAirLoop->AirToZoneNodeInfo(PriAirSysNum).NumZonesCooled;
                     ++ZoneInSysNum) { // loop over zones in system

                    int ZoneNum = state.dataAirLoop->AirToZoneNodeInfo(PriAirSysNum).CoolCtrlZoneNums(ZoneInSysNum);
                    // if the room temperature is greater than the vent temp sched value, set the vent temp check to TRUE
                    if (state.dataHeatBalFanSys->TempTstatAir(ZoneNum) > VentTemp) {
                        TempCheck = true;
                    }
                    // if the room temperature is less than the low limit set the low limit check to TRUE
                    if (state.dataHeatBalFanSys->TempTstatAir(ZoneNum) < nightVentMgr.VentTempLowLim) {
                        LowLimCheck = true;
                    }
                }
            }
            // If the difference between the control zone temperature and the outside temperature is greater than
            // the specified night venting delta T then set the delta T check to TRUE
            if ((state.dataHeatBalFanSys->TempTstatAir(ControlZoneNum) - state.dataEnvrn->OutDryBulbTemp) > nightVentMgr.VentDelT) {
                DelTCheck = true;
            }
            // If the limit requirements are met turn on night ventilation
            if (TempCheck && DelTCheck && !LowLimCheck) {
                availStatus = Status::CycleOn;
            } else {
                availStatus = Status::NoAction;
            }
        }

        if (!isZoneEquipType) {
            if (availStatus == Status::CycleOn) {
                state.dataAirLoop->AirLoopControlInfo(PriAirSysNum).LoopFlowRateSet = true;
                state.dataAirLoop->AirLoopControlInfo(PriAirSysNum).NightVent = true;
                state.dataAirLoop->AirLoopFlow(PriAirSysNum).ReqSupplyFrac = nightVentMgr.VentFlowFrac;
            }
        }

        nightVentMgr.availStatus = availStatus;
        return availStatus;
    }

    Status CalcDiffTSysAvailMgr(EnergyPlusData &state,
                                int const SysAvailNum,      // Number of the current scheduled system availability manager
                                Status const previousStatus // System status for the previous timestep
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   February 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set AvailStatus indicator for a plant loop, primary air loop or ZoneHVAC component.

        // METHODOLOGY EMPLOYED:

        Status availStatus;

        auto &diffThermoMgr = state.dataAvail->DiffThermoData(SysAvailNum);
        Real64 DeltaTemp = state.dataLoopNodes->Node(diffThermoMgr.HotNode).Temp - state.dataLoopNodes->Node(diffThermoMgr.ColdNode).Temp;

        if (DeltaTemp >= diffThermoMgr.TempDiffOn) {
            availStatus = Status::CycleOn;
        } else if (DeltaTemp <= diffThermoMgr.TempDiffOff) {
            availStatus = Status::ForceOff;
        } else if (previousStatus == Status::NoAction) {
            availStatus = Status::ForceOff;
        } else {
            availStatus = previousStatus; // No change, but not "NoAction"; it should always be on or off.
        }

        diffThermoMgr.availStatus = availStatus;
        return availStatus;
    }

    Status CalcHiTurnOffSysAvailMgr(EnergyPlusData &state,
                                    int const SysAvailNum // Number of the current scheduled system availability manager
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   February 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set AvailStatus indicator for a plant loop, primary air loop or ZoneHVAC component.
        Status availStatus;
        if (state.dataLoopNodes->Node(state.dataAvail->HiTurnOffData(SysAvailNum).Node).Temp >= state.dataAvail->HiTurnOffData(SysAvailNum).Temp) {
            availStatus = Status::ForceOff;
        } else {
            availStatus = Status::NoAction;
        }

        state.dataAvail->HiTurnOffData(SysAvailNum).availStatus = availStatus;
        return availStatus;
    }

    Status CalcHiTurnOnSysAvailMgr(EnergyPlusData &state,
                                   int const SysAvailNum // Number of the current scheduled system availability manager
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   February 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set AvailStatus indicator for a plant loop, primary air loop or ZoneHVAC component.

        Status availStatus;
        if (state.dataLoopNodes->Node(state.dataAvail->HiTurnOnData(SysAvailNum).Node).Temp >= state.dataAvail->HiTurnOnData(SysAvailNum).Temp) {
            availStatus = Status::CycleOn;
        } else {
            availStatus = Status::NoAction;
        }

        state.dataAvail->HiTurnOnData(SysAvailNum).availStatus = availStatus;
        return availStatus;
    }

    Status CalcLoTurnOffSysAvailMgr(EnergyPlusData &state,
                                    int const SysAvailNum // Number of the current scheduled system availability manager
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   February 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set AvailStatus indicator for a plant loop, primary air loop or ZoneHVAC component.

        Status availStatus;
        // If applicability schedule is off, then availability manager is inactive, return no action
        auto &loTurnOffMgr = state.dataAvail->LoTurnOffData(SysAvailNum);
        if (loTurnOffMgr.availSched != nullptr) {
            if (loTurnOffMgr.availSched->getCurrentVal() <= 0.0) {
                availStatus = Status::NoAction;
                loTurnOffMgr.availStatus = availStatus;
                return availStatus;
            }
        }

        // Availability manager is active, check temperature limit
        if (state.dataLoopNodes->Node(loTurnOffMgr.Node).Temp <= loTurnOffMgr.Temp) {
            availStatus = Status::ForceOff;
        } else {
            availStatus = Status::NoAction;
        }

        loTurnOffMgr.availStatus = availStatus;
        return availStatus;
    }

    Status CalcLoTurnOnSysAvailMgr(EnergyPlusData &state,
                                   int const SysAvailNum // Number of the current scheduled system availability manager
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   February 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set AvailStatus indicator for a plant loop, primary air loop or ZoneHVAC component.
        Status availStatus;
        if (state.dataLoopNodes->Node(state.dataAvail->LoTurnOnData(SysAvailNum).Node).Temp <= state.dataAvail->LoTurnOnData(SysAvailNum).Temp) {
            availStatus = Status::CycleOn;
        } else {
            availStatus = Status::NoAction;
        }

        state.dataAvail->LoTurnOnData(SysAvailNum).availStatus = availStatus;
        return availStatus;
    }

    void ManageHybridVentilation(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   March 2007
        //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone sys avail managers
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Manage the simulation of the Hybrid Ventilation Control System Availability Managers

        using namespace DataLoopNode;
        using namespace DataAirLoop;

        int PriAirSysNum; // Primary Air System index

        if (state.dataAvail->GetHybridInputFlag) {
            GetHybridVentilationInputs(state);
            state.dataAvail->GetHybridInputFlag = false;
        }

        if (state.dataAvail->NumHybridVentSysAvailMgrs == 0) {
            return;
        }

        InitHybridVentSysAvailMgr(state);

        for (int SysAvailNum = 1; SysAvailNum <= state.dataAvail->NumHybridVentSysAvailMgrs; ++SysAvailNum) {
            if (state.dataAvail->HybridVentData(SysAvailNum).HybridVentMgrConnectedToAirLoop) {
                for (PriAirSysNum = 1; PriAirSysNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++PriAirSysNum) {
                    if (state.dataAvail->HybridVentData(SysAvailNum).AirLoopNum == PriAirSysNum) {
                        CalcHybridVentSysAvailMgr(state, SysAvailNum, PriAirSysNum);
                    }
                }
            } else {
                // Hybrid ventilation manager is applied to zone component
                if (state.dataAvail->HybridVentData(SysAvailNum).SimHybridVentSysAvailMgr) {
                    CalcHybridVentSysAvailMgr(state, SysAvailNum);
                }
            }
        }
    }

    void GetHybridVentilationInputs(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   March 2007
        //       MODIFIED       L. GU, 6/23/08, Added more controls, including simple airflow objects
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains input data for Hybrid Ventilation Control System Availability Managers and stores it in
        // appropriate data structures.

        // METHODOLOGY EMPLOYED:
        // Uses InputProcessor "Get" routines to obtain data.

        // Using/Aliasing
        using NodeInputManager::GetOnlySingleNode;
        using NodeInputManager::MarkNode;
        using namespace DataLoopNode;

        using Curve::CurveValue;
        using Curve::GetCurveIndex;
        using Curve::GetCurveMinMaxValues;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetHybridVentilationInputs: "); // include trailing blank
        static constexpr std::string_view routineName = "GetHybridVentilationInputs";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlphas;           // Number of Alphas for each GetObjectItem call
        int NumNumbers;          // Number of Numbers for each GetObjectItem call
        int IOStatus;            // Used in GetObjectItem
        bool ErrorsFound(false); // Set to true if errors in input, fatal at end of routine
        Real64 SchedMin;         // Minimum value specified in a schedule
        Real64 SchedMax;         // Maximum value specified in a schedule
        Real64 CurveMin;         // Minimum value specified in a curve
        Real64 CurveMax;         // Maximum value specified in a curve
        Real64 CurveVal;         // Curve value

        auto &ipsc = state.dataIPShortCut;

        // Get the number of occurrences of each type of System Availability Manager
        std::string_view cCurrentModuleObject = managerTypeNames[(int)ManagerType::HybridVent];
        state.dataAvail->NumHybridVentSysAvailMgrs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        if (state.dataAvail->NumHybridVentSysAvailMgrs == 0) {
            return;
        }

        // Allocate the data arrays
        state.dataAvail->HybridVentData.allocate(state.dataAvail->NumHybridVentSysAvailMgrs);

        for (int SysAvailNum = 1; SysAvailNum <= state.dataAvail->NumHybridVentSysAvailMgrs; ++SysAvailNum) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     SysAvailNum,
                                                                     ipsc->cAlphaArgs,
                                                                     NumAlphas,
                                                                     ipsc->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     ipsc->lNumericFieldBlanks,
                                                                     ipsc->lAlphaFieldBlanks,
                                                                     ipsc->cAlphaFieldNames,
                                                                     ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, cCurrentModuleObject, ipsc->cAlphaArgs(1)};
            auto &hybridVentMgr = state.dataAvail->HybridVentData(SysAvailNum);
            hybridVentMgr.Name = ipsc->cAlphaArgs(1);
            hybridVentMgr.type = ManagerType::HybridVent;

            hybridVentMgr.AirLoopName = ipsc->cAlphaArgs(2);

            if (ipsc->lAlphaFieldBlanks(2)) { // Hybrid ventilation manager applied to zone
                hybridVentMgr.HybridVentMgrConnectedToAirLoop = false;
            }
            hybridVentMgr.ControlZoneName = ipsc->cAlphaArgs(3);
            // Check zone number
            hybridVentMgr.ControlledZoneNum = Util::FindItemInList(ipsc->cAlphaArgs(3), state.dataHeatBal->Zone);
            if (hybridVentMgr.ControlledZoneNum == 0) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(3), ipsc->cAlphaArgs(3));
                ErrorsFound = true;
            }

            if (ipsc->lAlphaFieldBlanks(4)) {
                ShowSevereEmptyField(state, eoh, ipsc->cAlphaFieldNames(4));
                ErrorsFound = true;
            } else if ((hybridVentMgr.controlModeSched = Sched::GetSchedule(state, ipsc->cAlphaArgs(4))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(4), ipsc->cAlphaArgs(4));
                ErrorsFound = true;
            }

            // Check schedule values
            SchedMin = hybridVentMgr.controlModeSched->getMinVal(state);
            SchedMax = hybridVentMgr.controlModeSched->getMaxVal(state);
            if (SchedMin == 0 && SchedMax == 0) {
                ShowWarningCustomField(state,
                                       eoh,
                                       ipsc->cAlphaFieldNames(4),
                                       ipsc->cAlphaArgs(4),
                                       "Schedule specifies control mode 0 for all entries, "
                                       "All zones using this schedule have no hybrid ventilation control.");
            }
            if (SchedMax > 7.0) {
                ShowSevereCustomField(state,
                                      eoh,
                                      ipsc->cAlphaFieldNames(4),
                                      ipsc->cAlphaArgs(4),
                                      format("Maximum value should be 7. However, the maximum value in the schedule is {:.1T}", SchedMax));
                ErrorsFound = true;
            }

            if (SchedMin < 0.0) {
                ShowSevereCustomField(state,
                                      eoh,
                                      ipsc->cAlphaFieldNames(4),
                                      ipsc->cAlphaArgs(4),
                                      format("Minimum value should be 0. However, the minimum value in the schedule is {:.1T}", SchedMin));
                ErrorsFound = true;
            }

            if (SchedMax == 7.0 && !state.dataContaminantBalance->Contaminant.CO2Simulation) {
                ShowSevereCustomField(state,
                                      eoh,
                                      ipsc->cAlphaFieldNames(4),
                                      ipsc->cAlphaArgs(4),
                                      "When the schedule value is 7, carbon dioxide (CO2) control is requested."
                                      "However, CO2 simulation is not enabled. Please use ZoneAirContaminantBalance object to simulate CO2.");
                ErrorsFound = true;
            }

            // Read use weather rain indicator
            BooleanSwitch b = static_cast<BooleanSwitch>(getYesNoValue(ipsc->cAlphaArgs(5)));
            if (b == BooleanSwitch::Invalid) {
                ShowSevereInvalidKey(state, eoh, ipsc->cAlphaFieldNames(5), ipsc->cAlphaArgs(5));
                ErrorsFound = true;
            } else {
                hybridVentMgr.UseRainIndicator = static_cast<bool>(b);
            }

            // Check max wind speed
            if (NumNumbers > 0) {
                hybridVentMgr.MaxWindSpeed = ipsc->rNumericArgs(1);
                if (ipsc->rNumericArgs(1) > 40.0 || ipsc->rNumericArgs(1) < 0.0) {
                    ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                    ShowContinueError(state, format("{} is beyond the range.", ipsc->cNumericFieldNames(1)));
                    ShowContinueError(state,
                                      format("The input value is {:.0T}. The allowed value must be >= 0 and <= 40 m/s", ipsc->rNumericArgs(1)));
                    ErrorsFound = true;
                }
            }

            // Read Max and Min outdoor temperature
            if (NumNumbers > 1) {
                hybridVentMgr.MinOutdoorTemp = ipsc->rNumericArgs(2);
                if (ipsc->rNumericArgs(2) > 100.0 || ipsc->rNumericArgs(2) < -100.0) {
                    ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                    ShowContinueError(state, format("{} is beyond the range.", ipsc->cNumericFieldNames(2)));
                    ShowContinueError(
                        state, format("The input value is {:.0T}. The allowed value must be between -100 C and +100 C", ipsc->rNumericArgs(2)));
                    ErrorsFound = true;
                }
            }
            if (NumNumbers > 2) {
                hybridVentMgr.MaxOutdoorTemp = ipsc->rNumericArgs(3);
                if (ipsc->rNumericArgs(3) > 100.0 || ipsc->rNumericArgs(3) < -100.0) {
                    ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                    ShowContinueError(state, format("{} is beyond the range.", ipsc->cNumericFieldNames(3)));
                    ShowContinueError(
                        state, format("The input value is {:.0T}. The allowed value must be between -100 C and +100 C", ipsc->rNumericArgs(3)));
                    ErrorsFound = true;
                }
            }
            // Ensure MaxTemp >= MinTemp
            if (ipsc->rNumericArgs(2) >= ipsc->rNumericArgs(3)) {
                ShowSevereError(state,
                                format("{}{}=\"{}\" The {} must be less than the {}",
                                       RoutineName,
                                       cCurrentModuleObject,
                                       ipsc->cAlphaArgs(1),
                                       ipsc->cNumericFieldNames(2),
                                       ipsc->cNumericFieldNames(3)));
                ShowContinueError(state,
                                  format("The {} is {:.0T}. The {} is {:.0T}.",
                                         ipsc->cNumericFieldNames(2),
                                         ipsc->rNumericArgs(2),
                                         ipsc->cNumericFieldNames(3),
                                         ipsc->rNumericArgs(3)));
                ErrorsFound = true;
            }

            // Read Max and Min outdoor enthalpy
            if (NumNumbers > 3) {
                hybridVentMgr.MinOutdoorEnth = ipsc->rNumericArgs(4);
                if (ipsc->rNumericArgs(4) > 300000.0 || ipsc->rNumericArgs(4) < 0.0) {
                    ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                    ShowContinueError(state, format("{} is beyond the range.", ipsc->cNumericFieldNames(4)));
                    ShowContinueError(
                        state, format("The input value is {:.0T}. The allowed value must be between 0 and 300000 J/kg", ipsc->rNumericArgs(4)));
                    ErrorsFound = true;
                }
            }
            if (NumNumbers > 4) {
                hybridVentMgr.MaxOutdoorEnth = ipsc->rNumericArgs(5);
                if (ipsc->rNumericArgs(5) > 300000.0 || ipsc->rNumericArgs(5) < 0.0) {
                    ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                    ShowContinueError(state, format("{} is beyond the range.", ipsc->cNumericFieldNames(5)));
                    ShowContinueError(
                        state, format("The input value is {:.0T}. The allowed value must be between 0 and 300000 J/kg", ipsc->rNumericArgs(5)));
                    ErrorsFound = true;
                }
            }
            // Ensure MaxEnth >= MiniEnth
            if (ipsc->rNumericArgs(4) >= ipsc->rNumericArgs(5)) {
                ShowSevereError(state,
                                format("{}{}=\"{}\" The {} must be less than the {}",
                                       RoutineName,
                                       cCurrentModuleObject,
                                       ipsc->cAlphaArgs(1),
                                       ipsc->cNumericFieldNames(4),
                                       ipsc->cNumericFieldNames(5)));
                ShowContinueError(state,
                                  format("The {} is {:.0T}. The {} is {:.0T}.",
                                         ipsc->cNumericFieldNames(4),
                                         ipsc->rNumericArgs(4),
                                         ipsc->cNumericFieldNames(5),
                                         ipsc->rNumericArgs(5)));
                ErrorsFound = true;
            }

            // Read Max and Min outdoor dew point
            if (NumNumbers > 5) {
                hybridVentMgr.MinOutdoorDewPoint = ipsc->rNumericArgs(6);
                if (ipsc->rNumericArgs(6) > 100.0 || ipsc->rNumericArgs(6) < -100.0) {
                    ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                    ShowContinueError(state, format("{} is beyond the range.", ipsc->cNumericFieldNames(6)));
                    ShowContinueError(
                        state, format("The input value is {:.0T}. The allowed value must be between -100 C and +100 C", ipsc->rNumericArgs(6)));
                    ErrorsFound = true;
                }
            }
            if (NumNumbers > 6) {
                hybridVentMgr.MaxOutdoorDewPoint = ipsc->rNumericArgs(7);
                if (ipsc->rNumericArgs(7) > 100.0 || ipsc->rNumericArgs(7) < -100.0) {
                    ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                    ShowContinueError(state, format("{} is beyond the range.", ipsc->cNumericFieldNames(7)));
                    ShowContinueError(
                        state, format("The input value is {:.0T}. The allowed value must be between -100 C and +100 C", ipsc->rNumericArgs(7)));
                    ErrorsFound = true;
                }
            }
            // Ensure MaxTemp >= MinTemp
            if (ipsc->rNumericArgs(6) >= ipsc->rNumericArgs(7)) {
                ShowSevereError(state,
                                format("{}{}=\"{}\" The {} must be less than the {}",
                                       RoutineName,
                                       cCurrentModuleObject,
                                       ipsc->cAlphaArgs(1),
                                       ipsc->cNumericFieldNames(6),
                                       ipsc->cNumericFieldNames(7)));
                ShowContinueError(state,
                                  format("The {} is {:.0T}. The {} is {:.0T}.",
                                         ipsc->cNumericFieldNames(6),
                                         ipsc->rNumericArgs(6),
                                         ipsc->cNumericFieldNames(7),
                                         ipsc->rNumericArgs(7)));
                ErrorsFound = true;
            }

            if (ipsc->lAlphaFieldBlanks(6)) {
                ShowSevereEmptyField(state, eoh, ipsc->cAlphaFieldNames(6));
                ErrorsFound = true;
            } else if ((hybridVentMgr.minOASched = Sched::GetSchedule(state, ipsc->cAlphaArgs(6))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(6), ipsc->cAlphaArgs(6));
                ErrorsFound = true;
            } else if (!hybridVentMgr.minOASched->checkMinVal(state, Clusive::In, 0.0)) {
                Sched::ShowSevereBadMin(state, eoh, ipsc->cAlphaFieldNames(6), ipsc->cAlphaArgs(6), Clusive::In, 0.0);
                ErrorsFound = true;
            }

            if (!ipsc->lAlphaFieldBlanks(7)) {
                hybridVentMgr.OpeningFactorFWS = GetCurveIndex(state, ipsc->cAlphaArgs(7));
                if (hybridVentMgr.OpeningFactorFWS <= 0) {
                    ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(7), ipsc->cAlphaArgs(7));
                    ErrorsFound = true;
                } else {
                    GetCurveMinMaxValues(state, hybridVentMgr.OpeningFactorFWS, CurveMin, CurveMax);
                    if (CurveMin < 0.0) {
                        ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                        ShowContinueError(state,
                                          format("The minimum wind speed used in {}=\"{}should be greater than or equal to 0.0 (m/s)",
                                                 ipsc->cAlphaFieldNames(7),
                                                 ipsc->cAlphaArgs(7)));
                        ShowContinueError(state, "Curve minimum value appears to be less than 0.");
                        ErrorsFound = true;
                    }
                    CurveVal = CurveValue(state, hybridVentMgr.OpeningFactorFWS, CurveMin);
                    if (CurveVal < 0.0) {
                        ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                        ShowContinueError(state,
                                          format("The minimum value of {} must be greater than or equal to 0.0 at the minimum value of wind speed.",
                                                 ipsc->cAlphaFieldNames(7)));
                        ShowContinueError(state, format("{}=\"{}\".", ipsc->cAlphaFieldNames(7), ipsc->cAlphaArgs(7)));
                        ShowContinueError(state, format("Curve output at the minimum wind speed = {:.3T}", CurveVal));
                        ErrorsFound = true;
                    }
                    CurveVal = CurveValue(state, hybridVentMgr.OpeningFactorFWS, CurveMax);
                    if (CurveVal > 1.0) {
                        ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                        ShowContinueError(state,
                                          format("The maximum value of {} must be less than or equal to 1.0 at the maximum value of wind speed.",
                                                 ipsc->cAlphaFieldNames(7)));
                        ShowContinueError(state, format("{}=\"{}\".", ipsc->cAlphaFieldNames(7), ipsc->cAlphaArgs(7)));
                        ShowContinueError(state, format("Curve output at the maximum wind speed = {:.3T}", CurveVal));
                        ErrorsFound = true;
                    }
                    // Check curve type
                    ErrorsFound |= Curve::CheckCurveDims(state,
                                                         hybridVentMgr.OpeningFactorFWS, // Curve index
                                                         {1},                            // Valid dimensions
                                                         RoutineName,                    // Routine name
                                                         cCurrentModuleObject,           // Object Type
                                                         hybridVentMgr.Name,             // Object Name
                                                         ipsc->cAlphaFieldNames(7));     // Field Name
                }
            }

            if (ipsc->lAlphaFieldBlanks(8)) {
            } else if ((hybridVentMgr.afnControlTypeSched = Sched::GetSchedule(state, ipsc->cAlphaArgs(8))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(8), ipsc->cAlphaArgs(8));
                ErrorsFound = true;
            } else if (!hybridVentMgr.afnControlTypeSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 1.0)) {
                Sched::ShowSevereBadMinMax(state, eoh, ipsc->cAlphaFieldNames(8), ipsc->cAlphaArgs(8), Clusive::In, 0.0, Clusive::In, 1.0);
                ErrorsFound = true;
            } else {
                hybridVentMgr.Master = hybridVentMgr.ControlledZoneNum;
                hybridVentMgr.afnControlStatus = hybridVentMgr.afnControlTypeSched->getCurrentVal(); // this was ANControlTypeSchedPtr!!
            }

            if (ipsc->lAlphaFieldBlanks(9)) {
            } else if ((hybridVentMgr.simpleControlTypeSched = Sched::GetSchedule(state, ipsc->cAlphaArgs(9))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(9), ipsc->cAlphaArgs(9));
                ErrorsFound = true;
            } else if (hybridVentMgr.afnControlTypeSched != nullptr) {
                ShowWarningCustom(state,
                                  eoh,
                                  format("{} and {} cannot be used at the same time, {} is disabled.",
                                         ipsc->cAlphaFieldNames(8),
                                         ipsc->cAlphaFieldNames(9),
                                         ipsc->cAlphaFieldNames(9)));
                hybridVentMgr.simpleControlTypeSched = nullptr;
            } else if (!hybridVentMgr.simpleControlTypeSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 1.0)) {
                Sched::ShowSevereBadMinMax(state, eoh, ipsc->cAlphaFieldNames(9), ipsc->cAlphaArgs(9), Clusive::In, 0.0, Clusive::In, 1.0);
                ErrorsFound = true;
            }

            if (hybridVentMgr.simpleControlTypeSched != nullptr) {

                hybridVentMgr.VentilationName = ipsc->cAlphaArgs(10);
                if (state.dataHeatBal->TotVentilation > 0) {

                    hybridVentMgr.VentilationPtr = Util::FindItemInList(ipsc->cAlphaArgs(10), state.dataHeatBal->Ventilation);
                    hybridVentMgr.Master = hybridVentMgr.VentilationPtr;

                    if (hybridVentMgr.VentilationPtr <= 0) {
                        if (int(hybridVentMgr.simpleControlTypeSched->getMaxVal(state)) == 1) {
                            ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(10), ipsc->cAlphaArgs(10));
                            ErrorsFound = true;
                        }

                        if (state.afn->simulation_control.type == AirflowNetwork::ControlType::NoMultizoneOrDistribution) {
                            ShowWarningError(state, format("{}{}=\"{}\"", RoutineName, cCurrentModuleObject, hybridVentMgr.Name));
                            ShowContinueError(state, "The Airflow Network model is not available for Hybrid Ventilation Control.");
                        } else if (state.afn->simulation_control.type ==
                                   AirflowNetwork::ControlType::MultizoneWithDistributionOnlyDuringFanOperation) {
                            ShowWarningError(state, format("{}{}=\"{}\"", RoutineName, cCurrentModuleObject, hybridVentMgr.Name));
                            ShowContinueError(state, "Please check the AirflowNetwork Control field in the AirflowNetwork:SimulationControl object.");
                            ShowContinueError(state, "The suggested choices are MultizoneWithDistribution or MultizoneWithoutDistribution.");
                        }

                    } else { // hybridVentMgr.VentilationPtr > 0
                        if (hybridVentMgr.ControlledZoneNum != state.dataHeatBal->Ventilation(hybridVentMgr.VentilationPtr).ZonePtr) {
                            ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, cCurrentModuleObject, ipsc->cAlphaArgs(1)));
                            ShowContinueError(
                                state,
                                format("The Zone name specified in the Ventilation object {}",
                                       state.dataHeatBal->Zone(state.dataHeatBal->Ventilation(hybridVentMgr.VentilationPtr).ZonePtr).Name));
                            ShowContinueError(state, format("is not equal to the {}=\"{}\".", ipsc->cAlphaFieldNames(3), ipsc->cAlphaArgs(3)));
                            ErrorsFound = true;
                        }

                        if (state.afn->simulation_control.type != AirflowNetwork::ControlType::NoMultizoneOrDistribution) {
                            ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, cCurrentModuleObject, hybridVentMgr.Name));
                            ShowContinueError(state, "The simple airflow objects are used for natural ventilation calculation.");
                            ShowContinueError(
                                state,
                                "The Airflow Network model is not allowed to perform. Please set the control type = NoMultizoneOrDistribution");
                            ErrorsFound = true;
                        }
                    }
                }
            }

            // Disallow combination of simple control and OA control mode
            if (hybridVentMgr.simpleControlTypeSched != nullptr && hybridVentMgr.controlModeSched->getMaxVal(state) == 4.0) {
                ShowSevereCustom(state,
                                 eoh,
                                 format("The outdoor ventilation air control type defined in {} cannot work together with {}",
                                        ipsc->cAlphaArgs(4),
                                        ipsc->cAlphaFieldNames(9)));
                ErrorsFound = true;
            }

            if (!ipsc->lNumericFieldBlanks(8)) {
                hybridVentMgr.MinOperTime = ipsc->rNumericArgs(8);
            }
            if (!ipsc->lNumericFieldBlanks(9)) {
                hybridVentMgr.MinVentTime = ipsc->rNumericArgs(9);
            }

        } // SysAvailNum

        if (state.dataAvail->NumHybridVentSysAvailMgrs > 1) {
            for (int SysAvailNum = 2; SysAvailNum <= state.dataAvail->NumHybridVentSysAvailMgrs; ++SysAvailNum) {
                if (state.dataAvail->HybridVentData(SysAvailNum - 1).afnControlTypeSched != nullptr) {
                    if (state.dataAvail->HybridVentData(SysAvailNum).simpleControlTypeSched != nullptr) {
                        ShowSevereError(state,
                                        format("The AirflowNetwork model is used for natural ventilation calculation in {}=\"{}\"",
                                               cCurrentModuleObject,
                                               state.dataAvail->HybridVentData(SysAvailNum - 1).Name));
                        ShowContinueError(state,
                                          format("The simple airflow objects are used for natural ventilation calculation in {}=\"{}\"",
                                                 cCurrentModuleObject,
                                                 state.dataAvail->HybridVentData(SysAvailNum).Name));
                        ShowContinueError(state, "The hybrid ventilation control requires the same models to calculate natural ventilation");
                        ErrorsFound = true;
                    }
                }
                if (state.dataAvail->HybridVentData(SysAvailNum - 1).simpleControlTypeSched != nullptr) {
                    if (state.dataAvail->HybridVentData(SysAvailNum).afnControlTypeSched != nullptr) {
                        ShowSevereError(state,
                                        format("The Airflow Network model is used for natural ventilation calculation in {}=\"{}\"",
                                               cCurrentModuleObject,
                                               state.dataAvail->HybridVentData(SysAvailNum).Name));
                        ShowContinueError(state,
                                          format("The simple airflow objects are used for natural ventilation calculation in {}=\"{}\"",
                                                 cCurrentModuleObject,
                                                 state.dataAvail->HybridVentData(SysAvailNum - 1).Name));
                        ShowContinueError(state, "The hybrid ventilation control requires the same models to calculate natural ventilation");
                        ErrorsFound = true;
                    }
                }
            } // SysAvailNum
        }

        if (ErrorsFound) {
            ShowFatalError(state, format("{} Errors found in input.  Preceding condition(s) cause termination.", RoutineName));
        }

        // Set up output variables
        for (int SysAvailNum = 1; SysAvailNum <= state.dataAvail->NumHybridVentSysAvailMgrs; ++SysAvailNum) {
            if (state.dataAvail->HybridVentData(SysAvailNum).HybridVentMgrConnectedToAirLoop) {
                SetupOutputVariable(state,
                                    "Availability Manager Hybrid Ventilation Control Status",
                                    Constant::Units::None,
                                    state.dataAvail->HybridVentData(SysAvailNum).ctrlStatus,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataAvail->HybridVentData(SysAvailNum).AirLoopName);
                SetupOutputVariable(state,
                                    "Availability Manager Hybrid Ventilation Control Mode",
                                    Constant::Units::None,
                                    state.dataAvail->HybridVentData(SysAvailNum).ctrlType,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataAvail->HybridVentData(SysAvailNum).AirLoopName);
            } else {
                SetupOutputVariable(state,
                                    "Availability Manager Hybrid Ventilation Control Status",
                                    Constant::Units::None,
                                    state.dataAvail->HybridVentData(SysAvailNum).ctrlStatus,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataAvail->HybridVentData(SysAvailNum).ControlZoneName);
                SetupOutputVariable(state,
                                    "Availability Manager Hybrid Ventilation Control Mode",
                                    Constant::Units::None,
                                    state.dataAvail->HybridVentData(SysAvailNum).ctrlType,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataAvail->HybridVentData(SysAvailNum).ControlZoneName);
            }

            if (state.dataAvail->HybridVentData(SysAvailNum).MinOperTime > 0) {
                SetupOutputVariable(state,
                                    "Hybrid Ventilation Control HVAC System Operation Elapsed Time",
                                    Constant::Units::min,
                                    state.dataAvail->HybridVentData(SysAvailNum).TimeOperDuration,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataAvail->HybridVentData(SysAvailNum).Name);
            }

            if (state.dataAvail->HybridVentData(SysAvailNum).MinVentTime > 0) {
                SetupOutputVariable(state,
                                    "Hybrid Ventilation Control Natural Ventilation Elapsed Time",
                                    Constant::Units::min,
                                    state.dataAvail->HybridVentData(SysAvailNum).TimeVentDuration,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataAvail->HybridVentData(SysAvailNum).Name);
            }

            if (state.dataAvail->HybridVentData(SysAvailNum).controlModeSched->hasVal(state, (int)VentCtrlType::OperT80) ||
                state.dataAvail->HybridVentData(SysAvailNum).controlModeSched->hasVal(state, (int)VentCtrlType::OperT90)) {
                SetupOutputVariable(state,
                                    "Hybrid Ventilation Operative Temperature",
                                    Constant::Units::C,
                                    state.dataAvail->HybridVentData(SysAvailNum).OperativeTemp,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataAvail->HybridVentData(SysAvailNum).Name);
                SetupOutputVariable(state,
                                    "Hybrid Ventilation Lower Limit Operative Temperature",
                                    Constant::Units::C,
                                    state.dataAvail->HybridVentData(SysAvailNum).minAdaTem,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataAvail->HybridVentData(SysAvailNum).Name);
                SetupOutputVariable(state,
                                    "Hybrid Ventilation Upper Limit Operative Temperature",
                                    Constant::Units::C,
                                    state.dataAvail->HybridVentData(SysAvailNum).maxAdaTem,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataAvail->HybridVentData(SysAvailNum).Name);
            }

            if (state.dataAvail->HybridVentData(SysAvailNum).controlModeSched->hasVal(state, (int)VentCtrlType::CO2)) {
                SetupOutputVariable(state,
                                    "Hybrid Ventilation CO2 Concentration",
                                    Constant::Units::ppm,
                                    state.dataAvail->HybridVentData(SysAvailNum).CO2,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    state.dataAvail->HybridVentData(SysAvailNum).Name);
            }
        }
    }

    void InitHybridVentSysAvailMgr(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   March 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the Hybrid Ventilation Control System Availability Manager

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations.

        // Using/Aliasing
        using DataZoneEquipment::NumValidSysAvailZoneComponents;

        static constexpr std::string_view routineName = "InitHybridVentSysAvailMgr";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool ErrorsFound(false); // Set to true if errors in input, fatal at end of routine
        int AirLoopNum;          // Air loop number
        int AirLoopCount;        // Air loop name count
        int SysAvailIndex;       // Hybrid Ventilation Sys Avail Manager index
        int ZoneEquipType;
        int HybridVentNum;

        // One time initializations
        if (state.dataAvail->MyOneTimeFlag && allocated(state.dataZoneEquip->ZoneEquipConfig) &&
            allocated(state.dataAirSystemsData->PrimaryAirSystems)) {

            // Ensure the controlled zone is listed and defined in an HVAC Air Loop
            for (int SysAvailNum = 1; SysAvailNum <= state.dataAvail->NumHybridVentSysAvailMgrs; ++SysAvailNum) {
                auto &hybridVentMgr = state.dataAvail->HybridVentData(SysAvailNum);
                ErrorObjectHeader eoh{routineName, managerTypeNames[(int)ManagerType::HybridVent], hybridVentMgr.Name};
                if (hybridVentMgr.simpleControlTypeSched != nullptr && state.dataHeatBal->TotVentilation > 0 && hybridVentMgr.VentilationPtr == 0) {
                    hybridVentMgr.VentilationPtr = Util::FindItemInList(hybridVentMgr.VentilationName, state.dataHeatBal->Ventilation);
                    hybridVentMgr.Master = hybridVentMgr.VentilationPtr;
                    if (hybridVentMgr.VentilationPtr <= 0 && int(hybridVentMgr.simpleControlTypeSched->getMaxVal(state)) == 1) {
                        ShowSevereItemNotFound(state, eoh, "ZoneVentilation Object Name", hybridVentMgr.VentilationName);
                        ErrorsFound = true;
                    }
                }
                // Check air loop number
                for (AirLoopNum = 1; AirLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++AirLoopNum) { // loop over the primary air systems
                    if (Util::SameString(state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Name, hybridVentMgr.AirLoopName)) {
                        hybridVentMgr.AirLoopNum = AirLoopNum;
                    }
                }

                bool zoneFound = false;
                int ControlledZoneNum = hybridVentMgr.ControlledZoneNum;
                if (hybridVentMgr.HybridVentMgrConnectedToAirLoop) {
                    if (hybridVentMgr.ControlledZoneNum > 0) {
                        for (int zoneInNode = 1; zoneInNode <= state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).NumInletNodes; ++zoneInNode) {
                            if (state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).InletNodeAirLoopNum(zoneInNode) == hybridVentMgr.AirLoopNum) {
                                zoneFound = true;
                            }
                        }
                        if (!zoneFound) {
                            ShowSevereError(state,
                                            format("{}, The controlled zone ={} is not served by this Air Loop={}",
                                                   managerTypeNames[(int)hybridVentMgr.type],
                                                   hybridVentMgr.ControlZoneName,
                                                   hybridVentMgr.AirLoopName));
                            ErrorsFound = true;
                        }
                    }
                }
                if (std::any_of(state.dataAvail->HybridVentData.begin(),
                                state.dataAvail->HybridVentData.end(),
                                [](SysAvailManagerHybridVent const &e) { return e.HybridVentMgrConnectedToAirLoop; })) {
                    for (int zoneInNode = 1; zoneInNode <= state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).NumInletNodes; ++zoneInNode) {
                        if (state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).InletNodeAirLoopNum(zoneInNode) == hybridVentMgr.AirLoopNum &&
                            hybridVentMgr.AirLoopNum > 0) {
                            for (HybridVentNum = 1; HybridVentNum <= state.dataAvail->NumHybridVentSysAvailMgrs; ++HybridVentNum) {
                                if (!state.dataAvail->HybridVentData(HybridVentNum).HybridVentMgrConnectedToAirLoop &&
                                    (HybridVentNum != SysAvailNum)) {
                                    if (ControlledZoneNum == state.dataAvail->HybridVentData(HybridVentNum).ControlledZoneNum &&
                                        ControlledZoneNum > 0) {
                                        ShowWarningError(
                                            state,
                                            format("AvailabilityManager:HybridVentilation = \"{}\" has the controlled zone name = \"{}\".",
                                                   state.dataAvail->HybridVentData(HybridVentNum).Name,
                                                   state.dataAvail->HybridVentData(HybridVentNum).ControlZoneName));
                                        ShowContinueError(
                                            state,
                                            format("This controlled zone already has hybrid ventilation control through this air loop = \"{}\".",
                                                   hybridVentMgr.AirLoopName));
                                        ShowContinueError(
                                            state,
                                            format("Only AvailabilityManager:HybridVentilation = \"{}\" will be simulated. Simulation continues...",
                                                   hybridVentMgr.Name));
                                    } else {
                                        state.dataAvail->HybridVentData(HybridVentNum).SimHybridVentSysAvailMgr = true;
                                    }
                                }
                            }
                        }
                    }
                } else {
                    for (auto &e : state.dataAvail->HybridVentData) {
                        e.SimHybridVentSysAvailMgr = true;
                    }
                }

                if (hybridVentMgr.ControlledZoneNum == 0) {
                    ShowSevereError(state,
                                    format("{}, The controlled zone is not defined correctly ={}",
                                           managerTypeNames[(int)hybridVentMgr.type],
                                           hybridVentMgr.ControlZoneName));
                    ErrorsFound = true;
                }
                // check schedule value for adaptive temperature control
                if (hybridVentMgr.controlModeSched->hasVal(state, 5.0) || hybridVentMgr.controlModeSched->hasVal(state, 6.0)) {
                    if (!state.dataHeatBal->AdaptiveComfortRequested_ASH55) {
                        ShowSevereError(state,
                                        format("GetHybridVentilationInputs: AvailabilityManager:HybridVentilation =\"{}\"", hybridVentMgr.Name));
                        ShowContinueError(state,
                                          format("Ventilation Control Mode Schedule Name =\"{}\", When the schedule value is 5 or 6, operative "
                                                 "temperature control is requested. ",
                                                 hybridVentMgr.controlModeSched->Name));
                        ShowContinueError(state,
                                          "However, AdaptiveASH55 is not entered in the Thermal Comfort Model Type fields in the People object.");
                        ErrorsFound = true;
                    }
                }
            }

            // Ensure an airloop name is not used more than once in the hybrid ventilation control objects
            for (AirLoopNum = 1; AirLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++AirLoopNum) { // loop over the primary air systems
                AirLoopCount = 0;
                for (int SysAvailNum = 1; SysAvailNum <= state.dataAvail->NumHybridVentSysAvailMgrs; ++SysAvailNum) {
                    if (Util::SameString(state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Name,
                                         state.dataAvail->HybridVentData(SysAvailNum).AirLoopName)) {
                        ++AirLoopCount;
                        if (AirLoopCount > 1) {
                            SysAvailIndex = SysAvailNum;
                        }
                    }
                }
                if (AirLoopCount > 1) {
                    ShowSevereError(state,
                                    format("{}, The AirLoopHVAC name found more than once={}",
                                           managerTypeNames[(int)state.dataAvail->HybridVentData(SysAvailIndex).type],
                                           state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Name));
                    ShowContinueError(state, "Each AirLoopHVAC allows one hybrid ventilation control object.");
                    ErrorsFound = true;
                }
            }

            if (ErrorsFound) {
                ShowFatalError(state, "Errors found in getting AvailabilityManager:* inputs");
            }

            state.dataAvail->MyOneTimeFlag = false;

        } // end 1 time initializations

        for (int SysAvailNum = 1; SysAvailNum <= state.dataAvail->NumHybridVentSysAvailMgrs; ++SysAvailNum) {
            auto &hybridVentMgr = state.dataAvail->HybridVentData(SysAvailNum);
            hybridVentMgr.ctrlType = static_cast<VentCtrlType>(hybridVentMgr.controlModeSched->getCurrentVal());
            // -1 means that the value will be determined inside CalcHybridVentSysAvailMgr.
            // IF the value is still -1, the program will stop.
            // hybridVentMgr.ctrlStatus = VentCtrlStatus::Invalid; // Not sure what this is for
            hybridVentMgr.WindModifier = -1.0;
        }

        if (allocated(state.dataAvail->HybridVentData)) {
            for (auto &e : state.dataAvail->HybridVentData) {
                e.availStatus = Status::NoAction;
            }
        }

        if (allocated(state.dataAvail->ZoneComp)) {
            for (ZoneEquipType = 1; ZoneEquipType <= NumValidSysAvailZoneComponents; ++ZoneEquipType) { // loop over the zone equipment types
                if (state.dataAvail->ZoneComp(ZoneEquipType).TotalNumComp > 0) {
                    for (auto &e : state.dataAvail->ZoneComp(ZoneEquipType).ZoneCompAvailMgrs) {
                        e.availStatus = Status::NoAction;
                    }
                }
            }
        }

        if (state.dataGlobal->BeginEnvrnFlag && state.dataAvail->MyEnvrnFlag) {
            for (int SysAvailNum = 1; SysAvailNum <= state.dataAvail->NumHybridVentSysAvailMgrs; ++SysAvailNum) {
                state.dataAvail->HybridVentData(SysAvailNum).TimeVentDuration = 0.0;
                state.dataAvail->HybridVentData(SysAvailNum).TimeOperDuration = 0.0;
            }
            state.dataAvail->MyEnvrnFlag = false;
        }
        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataAvail->MyEnvrnFlag = true;
        }
        // check minimum operation time
        state.dataAvail->CurrentEndTime = state.dataGlobal->CurrentTime + state.dataHVACGlobal->SysTimeElapsed;
        if (state.dataAvail->CurrentEndTime > state.dataAvail->CurrentEndTimeLast &&
            state.dataHVACGlobal->TimeStepSys >= state.dataAvail->TimeStepSysLast) {
            for (int SysAvailNum = 1; SysAvailNum <= state.dataAvail->NumHybridVentSysAvailMgrs; ++SysAvailNum) {
                auto &hybridVentMgr = state.dataAvail->HybridVentData(SysAvailNum);
                if (hybridVentMgr.ctrlStatus == VentCtrlStatus::NoAction) {
                    hybridVentMgr.TimeOperDuration = 0.0;
                    hybridVentMgr.TimeVentDuration = 0.0;
                }
                if (hybridVentMgr.MinVentTime > 0.0) {
                    if (hybridVentMgr.ctrlStatus == VentCtrlStatus::Open) {
                        hybridVentMgr.TimeVentDuration += (state.dataAvail->CurrentEndTime - state.dataAvail->CurrentEndTimeLast) * 60.0;
                        hybridVentMgr.TimeOperDuration = 0.0;
                    }
                }
                if (hybridVentMgr.MinOperTime > 0.0) {
                    if (hybridVentMgr.ctrlStatus == VentCtrlStatus::Close) {
                        hybridVentMgr.TimeOperDuration += (state.dataAvail->CurrentEndTime - state.dataAvail->CurrentEndTimeLast) * 60.0;
                        hybridVentMgr.TimeVentDuration = 0.0;
                    }
                }
            }
        }
        state.dataAvail->TimeStepSysLast = state.dataHVACGlobal->TimeStepSys;
        state.dataAvail->CurrentEndTimeLast = state.dataAvail->CurrentEndTime;
    }

    void CalcHybridVentSysAvailMgr(EnergyPlusData &state,
                                   int const SysAvailNum,                     // number of the current scheduled system availability manager
                                   ObjexxFCL::Optional_int_const PriAirSysNum // number of the primary air system affected by this Avail. Manager
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   March 2007
        //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone sys avail managers
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set AvailStatus indicator for a primary air loop and AirflowNetwork model to prevent
        // windows or doors open during HVAC system operation

        // METHODOLOGY EMPLOYED:
        // Looks at outside and indoor conditions to determine if hybrid ventilation
        // is beneficial. If it is and it is scheduled on the AvailStatus is set to cycle
        // on and open windows or doors.

        using namespace DataAirLoop;
        using Curve::CurveValue;
        using DataZoneEquipment::NumValidSysAvailZoneComponents;
        using Psychrometrics::PsyHFnTdbW;
        using Psychrometrics::PsyRhFnTdbWPb;
        using Psychrometrics::PsyTdpFnWPb;
        using Psychrometrics::PsyWFnTdbRhPb;

        int HStatZoneNum;                   // Humidity control zone number
        Real64 ZoneAirEnthalpy;             // Zone air enthalpy
        Real64 ZoneAirDewPoint;             // Zone air dew point temperature
        Real64 ZoneAirRH;                   // Zone air relative humidity
        Real64 TempExt;                     // Outdoor dry bulb temperature at zone height
        Real64 WindExt;                     // Outdoor wind speed at zone height
        Real64 WSetPoint;                   // Humidity ratio setpoint from a given RH setpoint schedule
        Real64 OASetPoint;                  // Outdoor air setpoint from a given OA setpoint schedule
        Real64 ACH;                         // Zone air change per hour
        bool found;                         // Used for humidistat object
        bool HybridVentModeOA;              // USed to check whether HybridVentModeOA is allowed
        Real64 ZoneRHHumidifyingSetPoint;   // Zone humidifying setpoint (%)
        Real64 ZoneRHDehumidifyingSetPoint; // Zone dehumidifying setpoint (%)
        int SimpleControlType;              // Simple control type from a schedule: 0 individual, 1 global
        int i;                              // Array index
        Real64 minAdaTem;                   // minimum adaptive temperature for adaptive temperature control
        Real64 maxAdaTem;                   // maximum adaptive temperature for adaptive temperature control
        bool KeepStatus;                    // true, if minimum time operation is needed
        int ZoneEquipType;
        int ZoneCompNum;
        int AirLoopNum;
        int Num;
        Status availStatus;

        KeepStatus = false;
        auto &hybridVentMgr = state.dataAvail->HybridVentData(SysAvailNum);
        if (hybridVentMgr.TimeVentDuration > 0.0 && hybridVentMgr.TimeVentDuration <= hybridVentMgr.MinVentTime) {
            KeepStatus = true;
        }
        if (hybridVentMgr.TimeOperDuration > 0.0 && hybridVentMgr.TimeOperDuration <= hybridVentMgr.MinOperTime) {
            KeepStatus = true;
        }

        int ZoneNum = hybridVentMgr.ControlledZoneNum;
        auto &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum);
        if (!KeepStatus) {
            hybridVentMgr.ctrlStatus = VentCtrlStatus::NoAction;
        }
        TempExt = state.dataHeatBal->Zone(ZoneNum).OutDryBulbTemp;
        WindExt = state.dataHeatBal->Zone(ZoneNum).WindSpeed;
        hybridVentMgr.OperativeTemp = 0.0;
        hybridVentMgr.minAdaTem = 0.0;
        hybridVentMgr.maxAdaTem = 0.0;

        if (!KeepStatus) {
            switch (hybridVentMgr.ctrlType) {

            case VentCtrlType::No: {
                hybridVentMgr.ctrlStatus = VentCtrlStatus::NoAction;

                // Temperature control
            } break;

            case VentCtrlType::Temp: {
                if (TempExt >= hybridVentMgr.MinOutdoorTemp && TempExt <= hybridVentMgr.MaxOutdoorTemp) {
                    hybridVentMgr.ctrlStatus = VentCtrlStatus::Open;
                } else {
                    hybridVentMgr.ctrlStatus = VentCtrlStatus::Close;
                }

                // Enthalpy control
            } break;

            case VentCtrlType::Enth: {
                ZoneAirEnthalpy = PsyHFnTdbW(thisZoneHB.MAT, thisZoneHB.airHumRat);
                if (state.dataEnvrn->OutEnthalpy >= hybridVentMgr.MinOutdoorEnth && state.dataEnvrn->OutEnthalpy <= hybridVentMgr.MaxOutdoorEnth) {
                    hybridVentMgr.ctrlStatus = VentCtrlStatus::Open;
                } else {
                    hybridVentMgr.ctrlStatus = VentCtrlStatus::Close;
                }

                // Dew point control
            } break;

            case VentCtrlType::DewPoint: {
                if (state.dataEnvrn->OutDewPointTemp >= hybridVentMgr.MinOutdoorDewPoint &&
                    state.dataEnvrn->OutDewPointTemp <= hybridVentMgr.MaxOutdoorDewPoint) {
                    hybridVentMgr.ctrlStatus = VentCtrlStatus::Open;
                } else {
                    hybridVentMgr.ctrlStatus = VentCtrlStatus::Close;
                }

            } break;

            case VentCtrlType::OA: {
                OASetPoint = hybridVentMgr.minOASched->getCurrentVal();
                ACH = 0.0;
                HybridVentModeOA = true;
                if (!hybridVentMgr.HybridVentMgrConnectedToAirLoop) {
                    if (state.afn->simulation_control.type == AirflowNetwork::ControlType::NoMultizoneOrDistribution) {
                        HybridVentModeOA = false;
                    }
                }

                if (hybridVentMgr.afnControlTypeSched != nullptr && HybridVentModeOA) {
                    state.afn->manage_balance(true);
                    ACH = state.afn->zone_OA_change_rate(ZoneNum);
                }
                if (ACH > OASetPoint) {
                    hybridVentMgr.ctrlStatus = VentCtrlStatus::Open;
                } else {
                    hybridVentMgr.ctrlStatus = VentCtrlStatus::Close;
                }

            } break;

            case VentCtrlType::OperT80: {
                if (state.dataThermalComforts->runningAverageASH >= 10.0 && state.dataThermalComforts->runningAverageASH <= 33.5) {
                    hybridVentMgr.OperativeTemp = 0.5 * (thisZoneHB.MAT + thisZoneHB.MRT);
                    minAdaTem = 0.31 * state.dataThermalComforts->runningAverageASH + 14.3;
                    maxAdaTem = 0.31 * state.dataThermalComforts->runningAverageASH + 21.3;
                    hybridVentMgr.minAdaTem = minAdaTem;
                    hybridVentMgr.maxAdaTem = maxAdaTem;
                    if (hybridVentMgr.OperativeTemp <= maxAdaTem && hybridVentMgr.OperativeTemp >= minAdaTem) {
                        hybridVentMgr.ctrlStatus = VentCtrlStatus::Open;
                    } else {
                        hybridVentMgr.ctrlStatus = VentCtrlStatus::Close;
                    }
                } else {
                    hybridVentMgr.ctrlStatus = VentCtrlStatus::Close;
                }

            } break;

            case VentCtrlType::OperT90: {
                if (state.dataThermalComforts->runningAverageASH >= 10.0 && state.dataThermalComforts->runningAverageASH <= 33.5) {
                    hybridVentMgr.OperativeTemp = 0.5 * (thisZoneHB.MAT + thisZoneHB.MRT);
                    minAdaTem = 0.31 * state.dataThermalComforts->runningAverageASH + 15.3;
                    maxAdaTem = 0.31 * state.dataThermalComforts->runningAverageASH + 20.3;
                    hybridVentMgr.minAdaTem = minAdaTem;
                    hybridVentMgr.maxAdaTem = maxAdaTem;
                    if (hybridVentMgr.OperativeTemp <= maxAdaTem && hybridVentMgr.OperativeTemp >= minAdaTem) {
                        hybridVentMgr.ctrlStatus = VentCtrlStatus::Open;
                    } else {
                        hybridVentMgr.ctrlStatus = VentCtrlStatus::Close;
                    }
                } else {
                    hybridVentMgr.ctrlStatus = VentCtrlStatus::Close;
                }

            } break;
            case VentCtrlType::CO2: {
                hybridVentMgr.CO2 = state.dataContaminantBalance->ZoneAirCO2(ZoneNum);
                if (state.dataContaminantBalance->ZoneAirCO2(ZoneNum) > state.dataContaminantBalance->ZoneCO2SetPoint(ZoneNum)) {
                    if (hybridVentMgr.HybridVentMgrConnectedToAirLoop) {
                        AirLoopNum = hybridVentMgr.AirLoopNum;
                        for (Num = 1; Num <= state.dataAirLoop->PriAirSysAvailMgr(hybridVentMgr.AirLoopNum).NumAvailManagers; ++Num) {
                            availStatus = SimSysAvailManager(state,
                                                             state.dataAirLoop->PriAirSysAvailMgr(AirLoopNum).availManagers(Num).type,
                                                             state.dataAirLoop->PriAirSysAvailMgr(AirLoopNum).availManagers(Num).Name,
                                                             state.dataAirLoop->PriAirSysAvailMgr(AirLoopNum).availManagers(Num).Num,
                                                             AirLoopNum,
                                                             state.dataAirLoop->PriAirSysAvailMgr(AirLoopNum).availStatus);
                        }
                        if (availStatus == Status::CycleOn) {
                            hybridVentMgr.ctrlStatus = VentCtrlStatus::Close;
                        } else {
                            hybridVentMgr.ctrlStatus = VentCtrlStatus::Open;
                        }
                    } else if (hybridVentMgr.SimHybridVentSysAvailMgr) {
                        hybridVentMgr.ctrlStatus = VentCtrlStatus::Open;
                        for (ZoneEquipType = 1; ZoneEquipType <= NumValidSysAvailZoneComponents; ++ZoneEquipType) {
                            for (ZoneCompNum = 1; ZoneCompNum <= state.dataAvail->ZoneComp(ZoneEquipType).TotalNumComp; ++ZoneCompNum) {
                                if (state.dataAvail->ZoneComp(ZoneEquipType).ZoneCompAvailMgrs(ZoneCompNum).availStatus == Status::CycleOn) {
                                    hybridVentMgr.ctrlStatus = VentCtrlStatus::Close;
                                    break;
                                }
                            }
                        }
                    } else {
                        hybridVentMgr.ctrlStatus = VentCtrlStatus::Open;
                    }
                }
            } break;
            default: {
                ShowSevereError(state,
                                format("{}: incorrect Control Type: {}", managerTypeNames[(int)hybridVentMgr.type], hybridVentMgr.AirLoopName));
                ShowFatalError(state, format("Errors found in getting {} Control mode value", managerTypeNames[(int)hybridVentMgr.type]));
            }
            }

            if (hybridVentMgr.ctrlStatus == VentCtrlStatus::Open) {

                auto const &zoneTstatSetpt = state.dataHeatBalFanSys->zoneTstatSetpts(ZoneNum);
                // Temperature and enthalpy control
                if (hybridVentMgr.ctrlType == VentCtrlType::Temp || hybridVentMgr.ctrlType == VentCtrlType::Enth) {

                    switch (state.dataHeatBalFanSys->TempControlType(ZoneNum)) {

                    case HVAC::SetptType::SingleHeat: {
                        if (thisZoneHB.MAT < zoneTstatSetpt.setpt) {
                            hybridVentMgr.ctrlStatus = VentCtrlStatus::Close;
                        }

                    } break;
                    case HVAC::SetptType::SingleCool: {
                        if (thisZoneHB.MAT > zoneTstatSetpt.setpt) {
                            hybridVentMgr.ctrlStatus = VentCtrlStatus::Close;
                        }

                    } break;
                    case HVAC::SetptType::SingleHeatCool: {
                        hybridVentMgr.ctrlStatus = VentCtrlStatus::Close;
                        ++hybridVentMgr.SingleHCErrCount;
                        if (hybridVentMgr.SingleHCErrCount < 2) {
                            ShowWarningError(state,
                                             format("Hybrid ventilation control: {}: The zone temperature control type is "
                                                    "ThermostatSetpoint:SingleHeatingOrCooling. Natural ventilation is not allowed.",
                                                    hybridVentMgr.AirLoopName));
                            ShowContinueErrorTimeStamp(state, "");
                        } else {
                            ShowRecurringWarningErrorAtEnd(
                                state,
                                "Hybrid ventilation control: " + hybridVentMgr.AirLoopName +
                                    ": No natural ventilation continues with a ThermostatSetpoint:SingleHeatingOrCooling type...",
                                hybridVentMgr.SingleHCErrIndex,
                                double(hybridVentMgr.ctrlType),
                                double(hybridVentMgr.ctrlType));
                        }

                    } break;

                    case HVAC::SetptType::DualHeatCool: {
                        if (thisZoneHB.MAT < zoneTstatSetpt.setptLo || thisZoneHB.MAT > zoneTstatSetpt.setptHi) {
                            hybridVentMgr.ctrlStatus = VentCtrlStatus::Close;
                        }

                    } break;
                    default:
                        break;
                    } // end select on thermostat control
                }

                // Dew point control mode
                if (hybridVentMgr.ctrlType == VentCtrlType::DewPoint) {
                    ZoneAirRH = PsyRhFnTdbWPb(state, thisZoneHB.MAT, thisZoneHB.airHumRat, state.dataEnvrn->OutBaroPress) * 100.0;
                    ZoneAirDewPoint = PsyTdpFnWPb(state, thisZoneHB.airHumRat, state.dataEnvrn->OutBaroPress);
                    if (state.dataZoneCtrls->NumHumidityControlZones == 0) {
                        ++hybridVentMgr.DewPointNoRHErrCount;
                        if (hybridVentMgr.DewPointNoRHErrCount < 2) {
                            ShowWarningError(
                                state,
                                format("Hybrid ventilation control: Dew point control mode is selected, but no ZoneControl:Humidistat object={}",
                                       hybridVentMgr.AirLoopName));
                            ShowContinueError(state, "The hybrid ventilation control is triggered by outdoor min and max dewpoint only.");
                            ShowContinueError(state, "HVAC system may turn off when outdoor dewpoint is between min and max dewpoint.");
                            ShowContinueErrorTimeStamp(state, "");
                        } else {
                            ShowRecurringWarningErrorAtEnd(state,
                                                           "Hybrid ventilation control: " + hybridVentMgr.AirLoopName +
                                                               ": no ZoneControl:Humidistat object continues...",
                                                           hybridVentMgr.DewPointNoRHErrIndex,
                                                           double(hybridVentMgr.ctrlType),
                                                           double(hybridVentMgr.ctrlType));
                        }
                    }
                    found = false;
                    for (HStatZoneNum = 1; HStatZoneNum <= state.dataZoneCtrls->NumHumidityControlZones; ++HStatZoneNum) {
                        if (state.dataZoneCtrls->HumidityControlZone(HStatZoneNum).ActualZoneNum == ZoneNum) {
                            found = true;
                            ZoneRHHumidifyingSetPoint = state.dataZoneCtrls->HumidityControlZone(HStatZoneNum).humidifyingSched->getCurrentVal();
                            ZoneRHDehumidifyingSetPoint = state.dataZoneCtrls->HumidityControlZone(HStatZoneNum).dehumidifyingSched->getCurrentVal();
                            if (ZoneAirRH > ZoneRHDehumidifyingSetPoint) { // Need dehumidification
                                WSetPoint =
                                    PsyWFnTdbRhPb(state, thisZoneHB.MAT, (ZoneRHDehumidifyingSetPoint / 100.0), state.dataEnvrn->OutBaroPress);
                                if (WSetPoint < state.dataEnvrn->OutHumRat) {
                                    hybridVentMgr.ctrlStatus = VentCtrlStatus::Close;
                                }
                            } else if (ZoneAirRH < ZoneRHHumidifyingSetPoint) { // Need humidification
                                WSetPoint = PsyWFnTdbRhPb(state, thisZoneHB.MAT, (ZoneRHHumidifyingSetPoint / 100.0), state.dataEnvrn->OutBaroPress);
                                if (WSetPoint > state.dataEnvrn->OutHumRat) {
                                    hybridVentMgr.ctrlStatus = VentCtrlStatus::Close;
                                }
                            } else {
                                hybridVentMgr.ctrlStatus = VentCtrlStatus::Close;
                            }
                        }
                    }
                    if (!found && state.dataZoneCtrls->NumHumidityControlZones > 0) {
                        ++hybridVentMgr.DewPointErrCount;
                        if (hybridVentMgr.DewPointErrCount < 2) {
                            ShowWarningError(state,
                                             format("Hybrid ventilation control: The zone for dew point control mode is different from the zone for "
                                                    "ZoneControl:Humidistat={}",
                                                    hybridVentMgr.AirLoopName));
                            ShowContinueError(
                                state,
                                format("The Zone name for hybrid control is {}. Humidistat has no impact", state.dataHeatBal->Zone(ZoneNum).Name));
                            ShowContinueError(state, "HVAC system may turn off when outdoor dewpoint is between min and max dewpoint.");
                            ShowContinueErrorTimeStamp(state, "");
                        } else {
                            ShowRecurringWarningErrorAtEnd(state,
                                                           "Hybrid ventilation control: " + hybridVentMgr.AirLoopName +
                                                               " No humidistat control impact continues...",
                                                           hybridVentMgr.DewPointErrIndex,
                                                           double(hybridVentMgr.ctrlType),
                                                           double(hybridVentMgr.ctrlType));
                        }
                    }
                }

                // Outdoor ventilation air control mode
                if (hybridVentMgr.ctrlType == VentCtrlType::OA) {
                }
            }
        }

        if (WindExt > hybridVentMgr.MaxWindSpeed) {
            hybridVentMgr.ctrlStatus = VentCtrlStatus::Close;
        }

        if (state.dataEnvrn->IsRain && hybridVentMgr.UseRainIndicator) {
            hybridVentMgr.ctrlStatus = VentCtrlStatus::Close;
        }
        // Sent a signal to the AirflowNetwork to ensure large onpenings are close or open based on this logic
        if (hybridVentMgr.ctrlStatus == VentCtrlStatus::Invalid) {
            // Fatal error
            ShowFatalError(state,
                           "Hybrid ventilation control: the ventilation control status is beyond the range. Please check input of control "
                           "mode schedule");
        }

        if (hybridVentMgr.HybridVentMgrConnectedToAirLoop) {
            if (hybridVentMgr.ctrlStatus == VentCtrlStatus::Close) {
                state.dataAirLoop->PriAirSysAvailMgr(PriAirSysNum).availStatus = Status::CycleOn;
            }
        }

        if (hybridVentMgr.ctrlStatus == VentCtrlStatus::Open && hybridVentMgr.afnControlTypeSched != nullptr && hybridVentMgr.OpeningFactorFWS > 0) {
            hybridVentMgr.WindModifier = CurveValue(state, hybridVentMgr.OpeningFactorFWS, WindExt);
        }

        // Set up flags to control simple airflow objects
        if (hybridVentMgr.AirLoopNum > 0 && hybridVentMgr.simpleControlTypeSched != nullptr) {
            SimpleControlType = hybridVentMgr.simpleControlTypeSched->getCurrentVal();
            for (int ControlledZoneNum = 1; ControlledZoneNum <= state.dataGlobal->NumOfZones; ++ControlledZoneNum) {
                for (int zoneInNode = 1; zoneInNode <= state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).NumInletNodes; ++zoneInNode) {
                    if (hybridVentMgr.AirLoopNum == state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).InletNodeAirLoopNum(zoneInNode)) {
                        // Setup flag for ventilation objects
                        for (i = 1; i <= state.dataHeatBal->TotVentilation; ++i) {
                            if (state.dataHeatBal->Ventilation(i).ZonePtr == ControlledZoneNum) {
                                state.dataHeatBal->Ventilation(i).HybridControlType = DataHeatBalance::HybridCtrlType::Indiv;
                                if (hybridVentMgr.ctrlStatus == VentCtrlStatus::Close) {
                                    state.dataHeatBal->Ventilation(i).HybridControlType = DataHeatBalance::HybridCtrlType::Close;
                                } else {
                                    if (SimpleControlType == 1) {
                                        state.dataHeatBal->Ventilation(i).HybridControlType = DataHeatBalance::HybridCtrlType::Global;
                                        state.dataHeatBal->Ventilation(i).HybridControlMasterNum = hybridVentMgr.VentilationPtr;
                                    }
                                }
                            }
                        }
                        // Setup flag for Mixing objects
                        for (i = 1; i <= state.dataHeatBal->TotMixing; ++i) {
                            if (state.dataHeatBal->Mixing(i).ZonePtr == ControlledZoneNum) {
                                state.dataHeatBal->Mixing(i).HybridControlType = DataHeatBalance::HybridCtrlType::Indiv;
                                if (hybridVentMgr.ctrlStatus == VentCtrlStatus::Close) {
                                    state.dataHeatBal->Mixing(i).HybridControlType = DataHeatBalance::HybridCtrlType::Close;
                                } else {
                                    if (SimpleControlType == 1) {
                                        state.dataHeatBal->Mixing(i).HybridControlType = DataHeatBalance::HybridCtrlType::Global;
                                        state.dataHeatBal->Mixing(i).HybridControlMasterNum = hybridVentMgr.VentilationPtr;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } else if (hybridVentMgr.simpleControlTypeSched != nullptr) {
            SimpleControlType = hybridVentMgr.simpleControlTypeSched->getCurrentVal();
            // Hybrid ventilation manager is applied to zone component
            // setup flag for ventilation objects
            for (i = 1; i <= state.dataHeatBal->TotVentilation; ++i) {
                if (state.dataHeatBal->Ventilation(i).ZonePtr == hybridVentMgr.ControlledZoneNum) {
                    state.dataHeatBal->Ventilation(i).HybridControlType = DataHeatBalance::HybridCtrlType::Indiv;
                    if (hybridVentMgr.ctrlStatus == VentCtrlStatus::Close) {
                        state.dataHeatBal->Ventilation(i).HybridControlType = DataHeatBalance::HybridCtrlType::Close;
                    } else {
                        if (SimpleControlType == 1) {
                            state.dataHeatBal->Ventilation(i).HybridControlType = DataHeatBalance::HybridCtrlType::Global;
                            state.dataHeatBal->Ventilation(i).HybridControlMasterNum = hybridVentMgr.VentilationPtr;
                        }
                    }
                }
            }
            // Setup flag for Mixing objects
            for (i = 1; i <= state.dataHeatBal->TotMixing; ++i) {
                if (state.dataHeatBal->Mixing(i).ZonePtr == hybridVentMgr.ControlledZoneNum) {
                    state.dataHeatBal->Mixing(i).HybridControlType = DataHeatBalance::HybridCtrlType::Indiv;
                    if (hybridVentMgr.ctrlStatus == VentCtrlStatus::Close) {
                        state.dataHeatBal->Mixing(i).HybridControlType = DataHeatBalance::HybridCtrlType::Close;
                    } else {
                        if (SimpleControlType == 1) {
                            state.dataHeatBal->Mixing(i).HybridControlType = DataHeatBalance::HybridCtrlType::Global;
                            state.dataHeatBal->Mixing(i).HybridControlMasterNum = hybridVentMgr.VentilationPtr;
                        }
                    }
                }
            }
        }
    }

    bool GetHybridVentilationControlStatus(EnergyPlusData &state, int const ZoneNum) // Index of zone
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   July 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This routine was designed to find whether this zone is controlled by hybrid ventilation
        // ventilation control option.

        // Return value
        bool VentControl; // Set to true if ventilation control in the same zone

        if (state.dataAvail->GetHybridInputFlag) { // First time subroutine has been entered
            GetHybridVentilationInputs(state);
            state.dataAvail->GetHybridInputFlag = false;
        }

        VentControl = false;

        for (int SysAvailNum = 1; SysAvailNum <= state.dataAvail->NumHybridVentSysAvailMgrs; ++SysAvailNum) {
            if (state.dataAvail->HybridVentData(SysAvailNum).ControlledZoneNum == ZoneNum) {
                if (state.dataAvail->HybridVentData(SysAvailNum).simpleControlTypeSched != nullptr) {
                    VentControl = true;
                }
            }
        }

        return VentControl;
    }

} // namespace Avail

} // namespace EnergyPlus
