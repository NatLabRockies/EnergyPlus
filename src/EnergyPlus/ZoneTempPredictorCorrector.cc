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
#include <numeric>
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>

// EnergyPlus Headers
#include <AirflowNetwork/Elements.hpp>
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataPrecisionGlobals.hh>
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneControls.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/FaultsManager.hh>
#include <EnergyPlus/FileSystem.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HeatBalFiniteDiffManager.hh>
#include <EnergyPlus/HeatBalanceSurfaceManager.hh>
#include <EnergyPlus/HybridModel.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/OutputReportTabular.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/RoomAirModelAirflowNetwork.hh>
#include <EnergyPlus/RoomAirModelManager.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/ThermalComfort.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WeatherManager.hh>
#include <EnergyPlus/ZonePlenum.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus::ZoneTempPredictorCorrector {

// MODULE INFORMATION:
//       AUTHOR         Russell D. Taylor
//       DATE WRITTEN   1997
//       MODIFIED       Aug 2001(FW): make SNLoadHeatRate public
//                      Nov 2010  BN(FSEC) added TemperatureAndHumidity Control
//       RE-ENGINEERED  July 2003 (Peter Graham Ellis)
//                      July 2006 (BG) added operative temp control
//                      February 2008 (BG) reworked zone air temp histories

// PURPOSE OF THIS MODULE:
// This module contains routines to predict and correct zone temperatures.
//  also includes zone thermostatic controlling
//  Model the "Air Heat Balance" part of the the "Zone Heat Balance Method."

// METHODOLOGY EMPLOYED:
// apply model equations for air heat balance solved for zone air temp.
//    sum up values for the terms (e.g SUMHAT, SUMHA etc. )
//    "Predict" step is used to get zone loads for HVAC equipment
//    "correct" step determines zone air temp with available HVAC

enum class ZoneControlTypes
{
    Invalid = -1,
    TStat = 1,
    TCTStat = 2,
    OTTStat = 3,
    HStat = 4,
    TandHStat = 5,
    StagedDual = 6,
    Num
};

enum class AdaptiveComfortModel
{
    Invalid = -1,
    ADAP_NONE = 1,
    ASH55_CENTRAL = 2,
    ASH55_UPPER_90 = 3,
    ASH55_UPPER_80 = 4,
    CEN15251_CENTRAL = 5,
    CEN15251_UPPER_I = 6,
    CEN15251_UPPER_II = 7,
    CEN15251_UPPER_III = 8,
    Num
};

static constexpr std::array<std::string_view, (int)HVAC::SetptType::Num> setptTypeNames = {"Uncontrolled",
                                                                                           "ThermostatSetpoint:SingleHeating",
                                                                                           "ThermostatSetpoint:SingleCooling",
                                                                                           "ThermostatSetpoint:SingleHeatingOrCooling",
                                                                                           "ThermostatSetpoint:DualSetpoint"};

static constexpr std::array<std::string_view, (int)HVAC::SetptType::Num> setptTypeNamesUC = {"UNCONTROLLED",
                                                                                             "THERMOSTATSETPOINT:SINGLEHEATING",
                                                                                             "THERMOSTATSETPOINT:SINGLECOOLING",
                                                                                             "THERMOSTATSETPOINT:SINGLEHEATINGORCOOLING",
                                                                                             "THERMOSTATSETPOINT:DUALSETPOINT"};

static constexpr std::array<std::string_view, (int)HVAC::SetptType::Num> comfortSetptTypeNames = {
    "Uncontrolled",
    "ThermostatSetpoint:ThermalComfort:Fanger:SingleHeating",
    "ThermostatSetpoint:ThermalComfort:Fanger:SingleCooling",
    "ThermostatSetpoint:ThermalComfort:Fanger:SingleHeatingOrCooling",
    "ThermostatSetpoint:ThermalComfort:Fanger:DualSetpoint"};

static constexpr std::array<std::string_view, (int)HVAC::SetptType::Num> comfortSetptTypeNamesUC = {
    "UNCONTROLLED",
    "THERMOSTATSETPOINT:THERMALCOMFORT:FANGER:SINGLEHEATING",
    "THERMOSTATSETPOINT:THERMALCOMFORT:FANGER:SINGLECOOLING",
    "THERMOSTATSETPOINT:THERMALCOMFORT:FANGER:SINGLEHEATINGORCOOLING",
    "THERMOSTATSETPOINT:THERMALCOMFORT:FANGER:DUALSETPOINT"};

Array1D_string const cZControlTypes(6,
                                    {"ZoneControl:Thermostat",
                                     "ZoneControl:Thermostat:ThermalComfort",
                                     "ZoneControl:Thermostat:OperativeTemperature",
                                     "ZoneControl:Humidistat",
                                     "ZoneControl:Thermostat:TemperatureAndHumidity",
                                     "ZoneControl:Thermostat:StagedDualSetpoint"});

Array1D_string const AdaptiveComfortModelTypes(8,
                                               {"None",
                                                "AdaptiveASH55CentralLine",
                                                "AdaptiveASH5590PercentUpperLine",
                                                "AdaptiveASH5580PercentUpperLine",
                                                "AdaptiveCEN15251CentralLine",
                                                "AdaptiveCEN15251CategoryIUpperLine",
                                                "AdaptiveCEN15251CategoryIIUpperLine",
                                                "AdaptiveCEN15251CategoryIIIUpperLine"});

// Functions
void ManageZoneAirUpdates(EnergyPlusData &state,
                          DataHeatBalFanSys::PredictorCorrectorCtrl const UpdateType, // Can be iGetZoneSetPoints, iPredictStep, iCorrectStep
                          Real64 &ZoneTempChange,                                     // Temp change in zone air btw previous and current timestep
                          bool const ShortenTimeStepSys,
                          bool const UseZoneTimeStepHistory, // if true then use zone timestep history, if false use system time step
                          Real64 const PriorTimeStep         // the old value for timestep length is passed for possible use in interpolating
)
{

    // SUBROUTINE INFORMATION
    //       AUTHOR         Russ Taylor
    //       DATE WRITTEN   September 1998
    //       MODIFIED       na
    //       RE-ENGINEERED  Brent Griffith Feb. 2008,  added arguments

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine predicts or corrects the zone air temperature
    // depending on the simulation status and determines the correct
    // temperature setpoint for each zone from the schedule manager.

    if (state.dataZoneCtrls->GetZoneAirStatsInputFlag) {
        GetZoneAirSetPoints(state);
        state.dataZoneCtrls->GetZoneAirStatsInputFlag = false;
    }

    InitZoneAirSetPoints(state);

    switch (UpdateType) {
    case DataHeatBalFanSys::PredictorCorrectorCtrl::GetZoneSetPoints: {
        CalcZoneAirTempSetPoints(state);
    } break;
    case DataHeatBalFanSys::PredictorCorrectorCtrl::PredictStep: {
        PredictSystemLoads(state, ShortenTimeStepSys, UseZoneTimeStepHistory, PriorTimeStep);
    } break;
    case DataHeatBalFanSys::PredictorCorrectorCtrl::CorrectStep: {
        ZoneTempChange = correctZoneAirTemps(state, UseZoneTimeStepHistory);
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

void GetZoneAirSetPoints(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Russell Taylor
    //       DATE WRITTEN   September 1998
    //       MODIFIED       L.Gu, May 2006, B. Griffith June 2006
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine gets the inputs related to thermostatic control.

    // METHODOLOGY EMPLOYED:
    // Uses the status flags to trigger events.

    // Using/Aliasing
    using General::CheckCreatedZoneItemName;
    using General::FindNumberInList;

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("GetZoneAirSetpoints: ");
    static constexpr std::string_view routineName = "GetZoneAirSetpoints";

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int TempControlledZoneNum; // The Splitter that you are currently loading input into
    int NumAlphas;
    int NumNums;
    int IOStat;
    bool ErrorsFound(false);
    bool errFlag;
    int HumidControlledZoneNum; // The Humidity Controller that information is being loaded into
    int ActualZoneNum;

    int ComfortControlledZoneNum; // The Splitter that you are currently loading input into
    int i;
    int found;
    int NumStageControlledZones; // Number of staged controlled objects

    Array1D_int CTSchedMapToControlledZone;
    Array1D_int CCmSchedMapToControlledZone;
    int Item;
    int Item1;
    int ZLItem;

    struct NeededControlTypes
    {
        // Members 4= the four control types + uncontrolled
        std::array<bool, static_cast<int>(HVAC::SetptType::Num)> MustHave = {false, false, false, false, false};
        std::array<bool, static_cast<int>(HVAC::SetptType::Num)> DidHave = {false, false, false, false, false};
    };

    struct NeededComfortControlTypes
    {
        // Members 4= the four control types + uncontrolled
        std::array<bool, static_cast<int>(HVAC::SetptType::Num)> MustHave = {false, false, false, false, false};
        std::array<bool, static_cast<int>(HVAC::SetptType::Num)> DidHave = {false, false, false, false, false};
    };

    // Object Data
    Array1D<NeededControlTypes> TStatControlTypes;
    Array1D<NeededComfortControlTypes> TComfortControlTypes;

    // Formats
    static constexpr std::string_view Header(
        "! <Zone Volume Capacitance Multiplier>, Sensible Heat Capacity Multiplier, Moisture Capacity Multiplier, Carbon "
        "Dioxide Capacity Multiplier, Generic Contaminant Capacity Multiplier\n");
    static constexpr std::string_view Format_701("Zone Volume Capacitance Multiplier,{:8.3F} ,{:8.3F},{:8.3F},{:8.3F}\n");

    auto &s_ipsc = state.dataIPShortCut;
    auto &s_ztpc = state.dataZoneTempPredictorCorrector;

    auto &TStatObjects = state.dataZoneCtrls->TStatObjects;
    auto &Zone = state.dataHeatBal->Zone;
    auto &ZoneList = state.dataHeatBal->ZoneList;
    auto &ComfortTStatObjects = state.dataZoneCtrls->ComfortTStatObjects;
    int NumOfZones = state.dataGlobal->NumOfZones;
    auto &s_ip = state.dataInputProcessing->inputProcessor;

    s_ipsc->cCurrentModuleObject = cZControlTypes(static_cast<int>(ZoneControlTypes::TStat));
    // Update Num in state and make local convenience copy
    int NumTStatStatements = state.dataZoneCtrls->NumTStatStatements = s_ip->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
    TStatObjects.allocate(NumTStatStatements);

    // Pre-scan for use of Zone lists in TStat statements (i.e. Global application of TStat)
    state.dataZoneCtrls->NumTempControlledZones = 0;
    for (Item = 1; Item <= NumTStatStatements; ++Item) {
        s_ip->getObjectItem(state,
                            s_ipsc->cCurrentModuleObject,
                            Item,
                            s_ipsc->cAlphaArgs,
                            NumAlphas,
                            s_ipsc->rNumericArgs,
                            NumNums,
                            IOStat,
                            s_ipsc->lNumericFieldBlanks,
                            s_ipsc->lAlphaFieldBlanks,
                            s_ipsc->cAlphaFieldNames,
                            s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};
        Util::IsNameEmpty(state, s_ipsc->cAlphaArgs(1), s_ipsc->cCurrentModuleObject, ErrorsFound);

        TStatObjects(Item).Name = s_ipsc->cAlphaArgs(1);
        Item1 = Util::FindItemInList(s_ipsc->cAlphaArgs(2), Zone);
        ZLItem = 0;
        if (Item1 == 0 && state.dataHeatBal->NumOfZoneLists > 0) {
            ZLItem = Util::FindItemInList(s_ipsc->cAlphaArgs(2), ZoneList);
        }
        if (Item1 > 0) {
            TStatObjects(Item).TempControlledZoneStartPtr = state.dataZoneCtrls->NumTempControlledZones + 1;
            ++state.dataZoneCtrls->NumTempControlledZones;
            TStatObjects(Item).NumOfZones = 1;
            TStatObjects(Item).ZoneListActive = false;
            TStatObjects(Item).ZoneOrZoneListPtr = Item1;
        } else if (ZLItem > 0) {
            auto const &ZoneList = state.dataHeatBal->ZoneList(ZLItem);
            TStatObjects(Item).TempControlledZoneStartPtr = state.dataZoneCtrls->NumTempControlledZones + 1;
            state.dataZoneCtrls->NumTempControlledZones += ZoneList.NumOfZones;
            TStatObjects(Item).NumOfZones = ZoneList.NumOfZones;
            TStatObjects(Item).ZoneListActive = true;
            TStatObjects(Item).ZoneOrZoneListPtr = ZLItem;
        } else {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
            ErrorsFound = true;
        }
    }

    if (ErrorsFound) {
        ShowSevereError(state, format("GetZoneAirSetpoints: Errors with invalid names in {} objects.", s_ipsc->cCurrentModuleObject));
        ShowContinueError(state, "...These will not be read in.  Other errors may occur.");
        state.dataZoneCtrls->NumTempControlledZones = 0;
    }

    if (state.dataZoneCtrls->NumTempControlledZones > 0) {
        state.dataZoneCtrls->TempControlledZone.allocate(state.dataZoneCtrls->NumTempControlledZones);
        TStatControlTypes.allocate(state.dataZoneCtrls->NumTempControlledZones); // Number of set point types
        CTSchedMapToControlledZone.dimension(state.dataZoneCtrls->NumTempControlledZones, 0);

        TempControlledZoneNum = 0;
        s_ztpc->NumOnOffCtrZone = 0;
        for (Item = 1; Item <= NumTStatStatements; ++Item) {
            s_ip->getObjectItem(state,
                                s_ipsc->cCurrentModuleObject,
                                Item,
                                s_ipsc->cAlphaArgs,
                                NumAlphas,
                                s_ipsc->rNumericArgs,
                                NumNums,
                                IOStat,
                                s_ipsc->lNumericFieldBlanks,
                                s_ipsc->lAlphaFieldBlanks,
                                s_ipsc->cAlphaFieldNames,
                                s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

            for (Item1 = 1; Item1 <= TStatObjects(Item).NumOfZones; ++Item1) {
                ++TempControlledZoneNum;
                auto &tempZone = state.dataZoneCtrls->TempControlledZone(TempControlledZoneNum);

                if (TStatObjects(Item).ZoneListActive) {
                    s_ipsc->cAlphaArgs(2) = Zone(ZoneList(TStatObjects(Item).ZoneOrZoneListPtr).Zone(Item1)).Name;
                }
                int ZoneAssigned = Util::FindItemInList(s_ipsc->cAlphaArgs(2),
                                                        state.dataZoneCtrls->TempControlledZone,
                                                        &DataZoneControls::ZoneTempControls::ZoneName,
                                                        TempControlledZoneNum - 1);
                if (ZoneAssigned == 0) {
                    tempZone.ZoneName = s_ipsc->cAlphaArgs(2);
                    tempZone.ActualZoneNum = Util::FindItemInList(s_ipsc->cAlphaArgs(2), Zone);
                    if (tempZone.ActualZoneNum == 0) {
                        ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
                        ErrorsFound = true;
                    } else {
                        Zone(tempZone.ActualZoneNum).TempControlledZoneIndex = TempControlledZoneNum;
                    }
                } else {
                    tempZone.ZoneName = s_ipsc->cAlphaArgs(2); // for continuity
                    ShowSevereDuplicateAssignment(
                        state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2), state.dataZoneCtrls->TempControlledZone(ZoneAssigned).Name);
                    ErrorsFound = true;
                    continue;
                }

                if (!TStatObjects(Item).ZoneListActive) {
                    tempZone.Name = s_ipsc->cAlphaArgs(1);
                } else {
                    auto &ZoneList = state.dataHeatBal->ZoneList(TStatObjects(Item).ZoneOrZoneListPtr);
                    CheckCreatedZoneItemName(state,
                                             RoutineName,
                                             s_ipsc->cCurrentModuleObject,
                                             Zone(ZoneList.Zone(Item1)).Name,
                                             ZoneList.MaxZoneNameLength,
                                             TStatObjects(Item).Name,
                                             state.dataZoneCtrls->TempControlledZone,
                                             TempControlledZoneNum - 1,
                                             tempZone.Name,
                                             errFlag);
                    if (errFlag) {
                        ErrorsFound = true;
                    }
                }

                tempZone.setptTypeSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(3));
                if (Item1 == 1) { // only show error on first of several if zone list
                    if (tempZone.setptTypeSched == nullptr) {
                        ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
                        ErrorsFound = true;
                    } else if (!tempZone.setptTypeSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 4.0)) {
                        Sched::ShowSevereBadMinMax(
                            state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3), Clusive::In, 0.0, Clusive::In, 4.0);
                        ErrorsFound = true;
                    }
                }

                if (s_ipsc->lAlphaFieldBlanks(7)) {
                    NumAlphas = 5;
                } else if (s_ipsc->lAlphaFieldBlanks(9)) {
                    NumAlphas = 7;
                } else if (s_ipsc->lAlphaFieldBlanks(11)) {
                    NumAlphas = 9;
                }

                int NumSetptTypes = nint((NumAlphas - 3.0) / 2.0);

                for (int iSetpt = 1; iSetpt <= NumSetptTypes; ++iSetpt) {

                    HVAC::SetptType setptType = HVAC::SetptType::Invalid;
                    int spIdx = 2 * iSetpt - 1 + 3;

                    if (s_ipsc->lAlphaFieldBlanks(spIdx)) {
                        ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(spIdx));
                        ErrorsFound = true;
                        continue;
                    } else if ((setptType = static_cast<HVAC::SetptType>(getEnumValue(setptTypeNamesUC, s_ipsc->cAlphaArgs(spIdx)))) ==
                               HVAC::SetptType::Invalid) {
                        ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(spIdx), s_ipsc->cAlphaArgs(spIdx));
                        ErrorsFound = true;
                        continue;
                    }

                    tempZone.setpts[(int)setptType].Name = s_ipsc->cAlphaArgs(2 * iSetpt + 3);
                    tempZone.setpts[(int)setptType].isUsed = true;
                }

                if (NumNums > 0) {
                    if (s_ipsc->rNumericArgs(1) >= 0.0) {
                        tempZone.DeltaTCutSet = s_ipsc->rNumericArgs(1);
                        if (s_ipsc->rNumericArgs(1) > 0.0) {
                            s_ztpc->NumOnOffCtrZone++;
                        }
                    } else {
                        ShowSevereError(state,
                                        format("{}=\"{} invalid {}=[{:.0T}].",
                                               s_ipsc->cCurrentModuleObject,
                                               s_ipsc->cAlphaArgs(1),
                                               s_ipsc->cNumericFieldNames(1),
                                               s_ipsc->rNumericArgs(1)));
                        ShowContinueError(state, "..Allowable values must be greater or equal to 0");
                        ErrorsFound = true;
                    }
                }

                if (tempZone.DeltaTCutSet > 0.0 && tempZone.setpts[(int)HVAC::SetptType::SingleHeatCool].Name != "") {
                    ShowWarningError(state,
                                     format("{}=\"{}: The choice of Temperature Difference Between Cutout And Setpoint will not be applied "
                                            "to ThermostatSetpoint:SingleHeatingOrCooling.",
                                            s_ipsc->cCurrentModuleObject,
                                            s_ipsc->cAlphaArgs(1)));
                }
            }
        } // NumTStatStatements
    } // Check on number of TempControlledZones

    s_ipsc->cCurrentModuleObject = setptTypeNamesUC[(int)HVAC::SetptType::SingleHeat];
    s_ztpc->NumTempControls[(int)HVAC::SetptType::SingleHeat] = s_ip->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

    if (s_ztpc->NumTempControls[(int)HVAC::SetptType::SingleHeat] > 0) {
        s_ztpc->tempSetptScheds[(int)HVAC::SetptType::SingleHeat].allocate(s_ztpc->NumTempControls[(int)HVAC::SetptType::SingleHeat]);
    }

    for (int idx = 1; idx <= s_ztpc->NumTempControls[(int)HVAC::SetptType::SingleHeat]; ++idx) {
        s_ip->getObjectItem(state,
                            s_ipsc->cCurrentModuleObject,
                            idx,
                            s_ipsc->cAlphaArgs,
                            NumAlphas,
                            s_ipsc->rNumericArgs,
                            NumNums,
                            IOStat,
                            s_ipsc->lNumericFieldBlanks,
                            s_ipsc->lAlphaFieldBlanks,
                            s_ipsc->cAlphaFieldNames,
                            s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

        Util::IsNameEmpty(state, s_ipsc->cAlphaArgs(1), s_ipsc->cCurrentModuleObject, ErrorsFound);
        auto &setpt = s_ztpc->tempSetptScheds[(int)HVAC::SetptType::SingleHeat](idx);
        setpt.Name = s_ipsc->cAlphaArgs(1);

        if (s_ipsc->lAlphaFieldBlanks(2)) {
            ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(2));
            ErrorsFound = true;
        } else if ((setpt.heatSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
            ErrorsFound = true;
        }

    } // SingleTempHeatingControlNum

    s_ipsc->cCurrentModuleObject = setptTypeNamesUC[(int)HVAC::SetptType::SingleCool];
    s_ztpc->NumTempControls[(int)HVAC::SetptType::SingleCool] = s_ip->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

    if (s_ztpc->NumTempControls[(int)HVAC::SetptType::SingleCool] > 0) {
        s_ztpc->tempSetptScheds[(int)HVAC::SetptType::SingleCool].allocate(s_ztpc->NumTempControls[(int)HVAC::SetptType::SingleCool]);
    }

    for (int idx = 1; idx <= s_ztpc->NumTempControls[(int)HVAC::SetptType::SingleCool]; ++idx) {
        s_ip->getObjectItem(state,
                            s_ipsc->cCurrentModuleObject,
                            idx,
                            s_ipsc->cAlphaArgs,
                            NumAlphas,
                            s_ipsc->rNumericArgs,
                            NumNums,
                            IOStat,
                            s_ipsc->lNumericFieldBlanks,
                            s_ipsc->lAlphaFieldBlanks,
                            s_ipsc->cAlphaFieldNames,
                            s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

        Util::IsNameEmpty(state, s_ipsc->cAlphaArgs(1), s_ipsc->cCurrentModuleObject, ErrorsFound);
        auto &setpt = s_ztpc->tempSetptScheds[(int)HVAC::SetptType::SingleCool](idx);
        setpt.Name = s_ipsc->cAlphaArgs(1);

        if (s_ipsc->lAlphaFieldBlanks(2)) {
            ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(2));
            ErrorsFound = true;
        } else if ((setpt.coolSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
            ErrorsFound = true;
        }

    } // SingleTempCoolingControlNum

    s_ipsc->cCurrentModuleObject = setptTypeNames[(int)HVAC::SetptType::SingleHeatCool];
    s_ztpc->NumTempControls[(int)HVAC::SetptType::SingleHeatCool] = s_ip->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

    if (s_ztpc->NumTempControls[(int)HVAC::SetptType::SingleHeatCool] > 0) {
        s_ztpc->tempSetptScheds[(int)HVAC::SetptType::SingleHeatCool].allocate(s_ztpc->NumTempControls[(int)HVAC::SetptType::SingleHeatCool]);
    }

    for (int idx = 1; idx <= s_ztpc->NumTempControls[(int)HVAC::SetptType::SingleHeatCool]; ++idx) {
        s_ip->getObjectItem(state,
                            s_ipsc->cCurrentModuleObject,
                            idx,
                            s_ipsc->cAlphaArgs,
                            NumAlphas,
                            s_ipsc->rNumericArgs,
                            NumNums,
                            IOStat,
                            s_ipsc->lNumericFieldBlanks,
                            s_ipsc->lAlphaFieldBlanks,
                            s_ipsc->cAlphaFieldNames,
                            s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

        auto &setpt = s_ztpc->tempSetptScheds[(int)HVAC::SetptType::SingleHeatCool](idx);
        setpt.Name = s_ipsc->cAlphaArgs(1);

        if (s_ipsc->lAlphaFieldBlanks(2)) {
            ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(2));
            ErrorsFound = true;
        } else if ((setpt.heatSched = setpt.coolSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
            ErrorsFound = true;
        }

    } // SingleTempHeatCoolControlNum

    s_ipsc->cCurrentModuleObject = setptTypeNames[(int)HVAC::SetptType::DualHeatCool];
    s_ztpc->NumTempControls[(int)HVAC::SetptType::DualHeatCool] = s_ip->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

    if (s_ztpc->NumTempControls[(int)HVAC::SetptType::DualHeatCool] > 0) {
        s_ztpc->tempSetptScheds[(int)HVAC::SetptType::DualHeatCool].allocate(s_ztpc->NumTempControls[(int)HVAC::SetptType::DualHeatCool]);
    }

    for (int idx = 1; idx <= s_ztpc->NumTempControls[(int)HVAC::SetptType::DualHeatCool]; ++idx) {
        s_ip->getObjectItem(state,
                            s_ipsc->cCurrentModuleObject,
                            idx,
                            s_ipsc->cAlphaArgs,
                            NumAlphas,
                            s_ipsc->rNumericArgs,
                            NumNums,
                            IOStat,
                            s_ipsc->lNumericFieldBlanks,
                            s_ipsc->lAlphaFieldBlanks,
                            s_ipsc->cAlphaFieldNames,
                            s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};
        Util::IsNameEmpty(state, s_ipsc->cAlphaArgs(1), s_ipsc->cCurrentModuleObject, ErrorsFound);
        auto &setpt = s_ztpc->tempSetptScheds[(int)HVAC::SetptType::DualHeatCool](idx);
        setpt.Name = s_ipsc->cAlphaArgs(1);

        if (s_ipsc->lAlphaFieldBlanks(2)) {
            ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(2));
            ErrorsFound = true;
        } else if ((setpt.heatSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
            ErrorsFound = true;
        }

        if (s_ipsc->lAlphaFieldBlanks(3)) {
            ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(3));
            ErrorsFound = true;
        } else if ((setpt.coolSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(3))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
            ErrorsFound = true;
        }

    } // DualTempHeatCoolControlNum

    // Finish filling in Schedule pointing indexes
    for (TempControlledZoneNum = 1; TempControlledZoneNum <= state.dataZoneCtrls->NumTempControlledZones; ++TempControlledZoneNum) {
        auto &tempZone = state.dataZoneCtrls->TempControlledZone(TempControlledZoneNum);

        for (HVAC::SetptType setptType : HVAC::controlledSetptTypes) {
            auto &setpt = tempZone.setpts[(int)setptType];
            if (!setpt.isUsed) {
                continue;
            }

            int setptIdx = Util::FindItem(setpt.Name, s_ztpc->tempSetptScheds[(int)setptType]);

            if (setptType == HVAC::SetptType::SingleHeat || setptType == HVAC::SetptType::SingleHeatCool ||
                setptType == HVAC::SetptType::DualHeatCool) {
                setpt.heatSetptSched = s_ztpc->tempSetptScheds[(int)setptType](setptIdx).heatSched;
            }

            if (setptType == HVAC::SetptType::SingleCool || setptType == HVAC::SetptType::SingleHeatCool ||
                setptType == HVAC::SetptType::DualHeatCool) {
                setpt.coolSetptSched = s_ztpc->tempSetptScheds[(int)setptType](setptIdx).coolSched;
            }
        }
    }

    // Now, Check the schedule values/indices for validity

    for (int TempControlledZoneNum = 1; TempControlledZoneNum <= state.dataZoneCtrls->NumTempControlledZones; ++TempControlledZoneNum) {
        auto &tempZone = state.dataZoneCtrls->TempControlledZone(TempControlledZoneNum);

        if (tempZone.setptTypeSched == nullptr) {
            continue; // error will be caught elsewhere
        }

        int SchedMin = tempZone.setptTypeSched->getMinVal(state);
        int SchedMax = tempZone.setptTypeSched->getMaxVal(state);

        if (SchedMin == (int)HVAC::SetptType::Uncontrolled && SchedMax == (int)HVAC::SetptType::Uncontrolled) {
            if (FindNumberInList(tempZone.setptTypeSched->Num, CTSchedMapToControlledZone, state.dataZoneCtrls->NumTempControlledZones) == 0) {
                ShowSevereError(state, format("Control Type Schedule={}", tempZone.setptTypeSched->Name));
                ShowContinueError(state, "..specifies control type 0 for all entries.");
                ShowContinueError(state, "All zones using this Control Type Schedule have no heating or cooling available.");
            }
            CTSchedMapToControlledZone(TempControlledZoneNum) = tempZone.setptTypeSched->Num;
        }

        for (HVAC::SetptType setptType : HVAC::controlledSetptTypes) {
            auto const &setpt = tempZone.setpts[(int)setptType];

            if (!setpt.isUsed) {
                // Catch early issues
                if (tempZone.setptTypeSched->hasVal(state, (int)setptType)) {
                    ShowSevereError(state, format("Control Type Schedule={}", tempZone.setptTypeSched->Name));
                    ShowContinueError(
                        state,
                        format("..specifies {} ({}) as the control type. Not valid for this zone.", (int)setptType, setptTypeNames[(int)setptType]));
                    ShowContinueError(state, format("..reference {}={}", cZControlTypes((int)ZoneControlTypes::TStat), tempZone.Name));
                    ShowContinueError(state, format("..reference ZONE={}", tempZone.ZoneName));
                    ErrorsFound = true;
                }
                continue;
            }

            if (setpt.heatSetptSched == nullptr &&
                (setptType == HVAC::SetptType::SingleHeat || setptType == HVAC::SetptType::SingleHeatCool ||
                 setptType == HVAC::SetptType::DualHeatCool) &&
                tempZone.setptTypeSched->hasVal(state, (int)setptType)) {
                ShowSevereError(state, format("Control Type Schedule={}", tempZone.setptTypeSched->Name));
                ShowContinueError(
                    state,
                    format("..specifies {} ({}) as the control type. Not valid for this zone.", (int)setptType, setptTypeNames[(int)setptType]));
                ShowContinueError(state, format("..reference {}={}", cZControlTypes((int)ZoneControlTypes::TStat), tempZone.Name));
                ShowContinueError(state, format("..reference ZONE={}", tempZone.ZoneName));
                ErrorsFound = true;
            }

            if (setpt.coolSetptSched == nullptr &&
                (setptType == HVAC::SetptType::SingleCool || setptType == HVAC::SetptType::SingleHeatCool ||
                 setptType == HVAC::SetptType::DualHeatCool) &&
                tempZone.setptTypeSched->hasVal(state, (int)setptType)) {
                ShowSevereError(state, format("Control Type Schedule={}", tempZone.setptTypeSched->Name));
                ShowContinueError(
                    state,
                    format("..specifies {} ({}) as the control type. Not valid for this zone.", (int)setptType, setptTypeNames[(int)setptType]));
                ShowContinueError(state, format("..reference {}={}", cZControlTypes((int)ZoneControlTypes::TStat), tempZone.Name));
                ShowContinueError(state, format("..reference ZONE={}", tempZone.ZoneName));
                ErrorsFound = true;
            }
        } // for (setptType)
    }

    for (TempControlledZoneNum = 1; TempControlledZoneNum <= state.dataZoneCtrls->NumTempControlledZones; ++TempControlledZoneNum) {
        auto &tempZone = state.dataZoneCtrls->TempControlledZone(TempControlledZoneNum);
        ActualZoneNum = tempZone.ActualZoneNum;

        if (tempZone.setptTypeSched == nullptr) {
            continue; // error caught elsewhere -- would just be confusing here
        }

        for (HVAC::SetptType setptType : HVAC::controlledSetptTypes) {
            if (TStatControlTypes(TempControlledZoneNum).MustHave[(int)setptType] &&
                TStatControlTypes(TempControlledZoneNum).DidHave[(int)setptType]) {
                continue;
            }

            if (!TStatControlTypes(TempControlledZoneNum).MustHave[(int)setptType]) {
                continue;
            }
            ShowWarningError(state, format("Schedule={}", tempZone.setptTypeSched->Name));
            ShowContinueError(state, format("...should include control type {} ({}) but does not.", (int)setptType, setptTypeNames[(int)setptType]));
            ShowContinueError(state, format("..reference {}={}", cZControlTypes((int)ZoneControlTypes::TStat), tempZone.Name));
            ShowContinueError(state, format("..reference ZONE={}", tempZone.ZoneName));
        }
    }

    if (allocated(TStatControlTypes)) {
        TStatControlTypes.deallocate();
    }
    // This starts the Humidity Control Get Input section
    s_ipsc->cCurrentModuleObject = cZControlTypes(static_cast<int>(ZoneControlTypes::HStat));
    state.dataZoneCtrls->NumHumidityControlZones = s_ip->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

    if (state.dataZoneCtrls->NumHumidityControlZones > 0) {
        state.dataZoneCtrls->HumidityControlZone.allocate(state.dataZoneCtrls->NumHumidityControlZones);
        s_ztpc->HumidityControlZoneUniqueNames.reserve(static_cast<unsigned>(state.dataZoneCtrls->NumHumidityControlZones));
    }

    for (HumidControlledZoneNum = 1; HumidControlledZoneNum <= state.dataZoneCtrls->NumHumidityControlZones; ++HumidControlledZoneNum) {
        s_ip->getObjectItem(state,
                            s_ipsc->cCurrentModuleObject,
                            HumidControlledZoneNum,
                            s_ipsc->cAlphaArgs,
                            NumAlphas,
                            s_ipsc->rNumericArgs,
                            NumNums,
                            IOStat,
                            s_ipsc->lNumericFieldBlanks,
                            s_ipsc->lAlphaFieldBlanks,
                            s_ipsc->cAlphaFieldNames,
                            s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

        Util::IsNameEmpty(state, s_ipsc->cAlphaArgs(1), s_ipsc->cCurrentModuleObject, ErrorsFound);

        auto &humidControlledZone = state.dataZoneCtrls->HumidityControlZone(HumidControlledZoneNum);
        humidControlledZone.ControlName = s_ipsc->cAlphaArgs(1);
        GlobalNames::IntraObjUniquenessCheck(state,
                                             s_ipsc->cAlphaArgs(2),
                                             s_ipsc->cCurrentModuleObject,
                                             s_ipsc->cAlphaFieldNames(2),
                                             s_ztpc->HumidityControlZoneUniqueNames,
                                             ErrorsFound);

        humidControlledZone.ZoneName = s_ipsc->cAlphaArgs(2);
        humidControlledZone.ActualZoneNum = Util::FindItem(s_ipsc->cAlphaArgs(2), Zone);
        if (humidControlledZone.ActualZoneNum == 0) {
            ShowSevereError(state,
                            format("{}=\"{} invalid {}=\"{}\" not found.",
                                   s_ipsc->cCurrentModuleObject,
                                   s_ipsc->cAlphaArgs(1),
                                   s_ipsc->cAlphaFieldNames(2),
                                   s_ipsc->cAlphaArgs(2)));
            ErrorsFound = true;
        } else {
            state.dataHeatBal->Zone(humidControlledZone.ActualZoneNum).humidityControlZoneIndex = HumidControlledZoneNum;
        }

        if (s_ipsc->lAlphaFieldBlanks(3)) {
            ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(3));
            ErrorsFound = true;
        } else if ((humidControlledZone.humidifyingSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(3))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
            ErrorsFound = true;
        }

        if (NumAlphas < 4 || s_ipsc->lAlphaFieldBlanks(4)) {
            humidControlledZone.dehumidifyingSched = humidControlledZone.humidifyingSched;
        } else if ((humidControlledZone.dehumidifyingSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(4))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4));
            ErrorsFound = true;
        }

    } // HumidControlledZoneNum

    // Start to read Thermal comfort control objects
    s_ipsc->cCurrentModuleObject = cZControlTypes(static_cast<int>(ZoneControlTypes::TCTStat));
    state.dataZoneCtrls->NumComfortTStatStatements = s_ip->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
    ComfortTStatObjects.allocate(state.dataZoneCtrls->NumComfortTStatStatements);

    // Pre-scan for use of Zone lists in TStat statements (i.e. Global application of TStat)
    state.dataZoneCtrls->NumComfortControlledZones = 0;
    errFlag = false;
    for (Item = 1; Item <= state.dataZoneCtrls->NumComfortTStatStatements; ++Item) {
        s_ip->getObjectItem(state,
                            s_ipsc->cCurrentModuleObject,
                            Item,
                            s_ipsc->cAlphaArgs,
                            NumAlphas,
                            s_ipsc->rNumericArgs,
                            NumNums,
                            IOStat,
                            s_ipsc->lNumericFieldBlanks,
                            s_ipsc->lAlphaFieldBlanks,
                            s_ipsc->cAlphaFieldNames,
                            s_ipsc->cNumericFieldNames);
        Util::IsNameEmpty(state, s_ipsc->cAlphaArgs(1), s_ipsc->cCurrentModuleObject, ErrorsFound);

        Item1 = Util::FindItemInList(s_ipsc->cAlphaArgs(2), Zone);
        ZLItem = 0;
        if (Item1 == 0 && state.dataHeatBal->NumOfZoneLists > 0) {
            ZLItem = Util::FindItemInList(s_ipsc->cAlphaArgs(2), ZoneList);
        }
        ComfortTStatObjects(Item).Name = s_ipsc->cAlphaArgs(1);
        if (Item1 > 0) {
            ComfortTStatObjects(Item).ComfortControlledZoneStartPtr = state.dataZoneCtrls->NumComfortControlledZones + 1;
            ++state.dataZoneCtrls->NumComfortControlledZones;
            ComfortTStatObjects(Item).NumOfZones = 1;
            ComfortTStatObjects(Item).ZoneListActive = false;
            ComfortTStatObjects(Item).ZoneOrZoneListPtr = Item1;
        } else if (ZLItem > 0) {
            auto const &ZoneList = state.dataHeatBal->ZoneList(ZLItem);
            ComfortTStatObjects(Item).ComfortControlledZoneStartPtr = state.dataZoneCtrls->NumComfortControlledZones + 1;
            state.dataZoneCtrls->NumComfortControlledZones += ZoneList.NumOfZones;
            ComfortTStatObjects(Item).NumOfZones = ZoneList.NumOfZones;
            ComfortTStatObjects(Item).ZoneListActive = true;
            ComfortTStatObjects(Item).ZoneOrZoneListPtr = ZLItem;
        } else {
            ShowSevereError(state,
                            format("{}=\"{}\" invalid {}=\"{}\" not found.",
                                   s_ipsc->cCurrentModuleObject,
                                   s_ipsc->cAlphaArgs(1),
                                   s_ipsc->cAlphaFieldNames(2),
                                   s_ipsc->cAlphaArgs(2)));
            errFlag = true;
            ErrorsFound = true;
        }
    }

    if (errFlag) {
        ShowSevereError(state, format("GetZoneAirSetpoints: Errors with invalid names in {} objects.", s_ipsc->cCurrentModuleObject));
        ShowContinueError(state, "...These will not be read in.  Other errors may occur.");
        state.dataZoneCtrls->NumComfortControlledZones = 0;
    }

    if (state.dataZoneCtrls->NumComfortControlledZones > 0) {
        state.dataZoneCtrls->ComfortControlledZone.allocate(state.dataZoneCtrls->NumComfortControlledZones);
        TComfortControlTypes.allocate(state.dataZoneCtrls->NumComfortControlledZones); // Number of set point types
        CCmSchedMapToControlledZone.dimension(state.dataZoneCtrls->NumComfortControlledZones, 0);

        ComfortControlledZoneNum = 0;
        for (Item = 1; Item <= state.dataZoneCtrls->NumComfortTStatStatements; ++Item) {
            s_ip->getObjectItem(state,
                                s_ipsc->cCurrentModuleObject,
                                Item,
                                s_ipsc->cAlphaArgs,
                                NumAlphas,
                                s_ipsc->rNumericArgs,
                                NumNums,
                                IOStat,
                                s_ipsc->lNumericFieldBlanks,
                                s_ipsc->lAlphaFieldBlanks,
                                s_ipsc->cAlphaFieldNames,
                                s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

            for (Item1 = 1; Item1 <= ComfortTStatObjects(Item).NumOfZones; ++Item1) {
                ++ComfortControlledZoneNum;

                auto &comfortZone = state.dataZoneCtrls->ComfortControlledZone(ComfortControlledZoneNum);

                if (ComfortTStatObjects(Item).ZoneListActive) {
                    s_ipsc->cAlphaArgs(2) = state.dataHeatBal->Zone(ZoneList(ComfortTStatObjects(Item).ZoneOrZoneListPtr).Zone(Item1)).Name;
                }
                int ZoneAssigned = Util::FindItemInList(s_ipsc->cAlphaArgs(2),
                                                        state.dataZoneCtrls->ComfortControlledZone,
                                                        &DataZoneControls::ZoneComfortControls::ZoneName,
                                                        ComfortControlledZoneNum - 1);
                if (ZoneAssigned == 0) {
                    comfortZone.ZoneName = s_ipsc->cAlphaArgs(2);
                    comfortZone.ActualZoneNum = Util::FindItemInList(s_ipsc->cAlphaArgs(2), Zone);
                    if (comfortZone.ActualZoneNum == 0) {
                        ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
                        ErrorsFound = true;
                    }
                } else {
                    comfortZone.ZoneName = s_ipsc->cAlphaArgs(2); // for continuity
                    ShowSevereDuplicateAssignment(state,
                                                  eoh,
                                                  s_ipsc->cAlphaFieldNames(2),
                                                  s_ipsc->cAlphaArgs(2),
                                                  state.dataZoneCtrls->ComfortControlledZone(ZoneAssigned).Name);
                    ErrorsFound = true;
                    continue;
                }

                if (!ComfortTStatObjects(Item).ZoneListActive) {
                    comfortZone.Name = s_ipsc->cAlphaArgs(1);
                } else {
                    comfortZone.Name = state.dataHeatBal->Zone(ZoneList(ComfortTStatObjects(Item).ZoneOrZoneListPtr).Zone(Item1)).Name + ' ' +
                                       ComfortTStatObjects(Item).Name;
                }

                // Read Fields A3 and A4 for averaging method
                int IZoneCount = 0;
                for (i = 1; i <= state.dataHeatBal->TotPeople; ++i) {
                    if (comfortZone.ActualZoneNum == state.dataHeatBal->People(i).ZonePtr) {
                        ++IZoneCount;
                    }
                }
                // Could not find a people object for this particular zone
                if (IZoneCount == 0 && comfortZone.ActualZoneNum > 0) {
                    ShowSevereError(state,
                                    format("{}=\"{} no PEOPLE in {}=\"{}\" - cannot use Comfort Control.",
                                           s_ipsc->cCurrentModuleObject,
                                           s_ipsc->cAlphaArgs(1),
                                           s_ipsc->cAlphaFieldNames(2),
                                           s_ipsc->cAlphaArgs(2)));
                    ErrorsFound = true;
                }

                comfortZone.averageMethod = DataZoneControls::AverageMethod::NO;
                if (IZoneCount > 1) {
                    comfortZone.averageMethod =
                        static_cast<DataZoneControls::AverageMethod>(getEnumValue(DataZoneControls::averageMethodNamesUC, s_ipsc->cAlphaArgs(3)));
                    if (comfortZone.averageMethod == DataZoneControls::AverageMethod::Invalid) {
                        ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
                        ErrorsFound = true;
                    } else if (comfortZone.averageMethod == DataZoneControls::AverageMethod::SPE) {
                        comfortZone.AverageObjectName = s_ipsc->cAlphaArgs(4);
                        if (Util::FindItem(s_ipsc->cAlphaArgs(4), state.dataHeatBal->People) == 0) {
                            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4));
                            ErrorsFound = true;
                        } else {
                            comfortZone.SpecificObjectNum = Util::FindItem(s_ipsc->cAlphaArgs(4), state.dataHeatBal->People);
                        }
                    }
                } else {
                    for (i = 1; i <= state.dataHeatBal->TotPeople; ++i) {
                        if (comfortZone.ActualZoneNum == state.dataHeatBal->People(i).ZonePtr) {
                            break;
                        }
                    }
                    comfortZone.SpecificObjectNum = i;
                }
                // Check values used for thermal comfort calculation
                for (i = 1; i <= state.dataHeatBal->TotPeople; ++i) {
                    auto &people = state.dataHeatBal->People(i);

                    if (comfortZone.ActualZoneNum != people.ZonePtr) {
                        continue;
                    }

                    // Check activity level
                    if (people.activityLevelSched == nullptr) {
                        ShowSevereError(state, format("GetPeople Activity Level: Activity level schedule is not found={}", people.Name));
                        ShowContinueError(state, "Required when the zone has Thermal Comfort Controls.");
                        ErrorsFound = true;
                    } else if (!people.activityLevelSched->checkMinMaxVals(state, Clusive::In, 72.0, Clusive::In, 909.0)) {
                        ShowSevereError(state, "GetPeople Activity Level: Invalid activity level values entered for thermal comfort calculation");
                        ShowContinueError(state, format("Outside of range values [72,909], Reference object={}", people.Name));
                    }

                    // Check Work Efficiency
                    if (people.workEffSched == nullptr) {
                        ShowSevereError(state, format("GetPeople work efficiency: Work efficiency schedule is not found={}", people.Name));
                        ShowContinueError(state, "Required when the zone has Thermal Comfort Controls.");
                        ErrorsFound = true;
                    } else if (!people.workEffSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 1.0)) {
                        ShowSevereError(state, "GetPeople work efficiency: Invalid work efficiency values entered for thermal comfort calculation");
                        ShowContinueError(state, format("Outside of range values [0,1], Reference object={}", people.Name));
                        ErrorsFound = true;
                    }

                    // Check Clothing Insulation
                    if (people.clothingSched == nullptr) {
                        ShowSevereError(state, format("GetPeople Clothing Insulation: Clothing Insulation schedule is not found={}", people.Name));
                        ShowContinueError(state, "Required when the zone has Thermal Comfort Controls.");
                        ErrorsFound = true;
                    } else if (!people.clothingSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 2.0)) {
                        ShowSevereError(state,
                                        "GetPeople Clothing Insulation: Invalid Clothing Insulation values entered for thermal comfort calculation");
                        ShowContinueError(state, format("Outside of range values [0.0,2.0], Reference object={}", people.Name));
                        ErrorsFound = true;
                    }

                    // Check Air velocity
                    if (people.airVelocitySched == nullptr) {
                        ShowSevereError(state, format("GetPeople Air Velocity: Air velocity schedule is not found={}", people.Name));
                        ShowContinueError(state, "Required when the zone has Thermal Comfort Controls.");
                        ErrorsFound = true;
                    }
                }

                // Read Max and Min temperature setpoint
                if (NumNums > 0) {
                    comfortZone.TdbMinSetPoint = s_ipsc->rNumericArgs(1);
                    if (s_ipsc->rNumericArgs(1) > 50 || s_ipsc->rNumericArgs(1) < 0) {
                        ShowSevereError(state,
                                        format("{}=\"{} invalid {}=[{:.0T}].",
                                               s_ipsc->cCurrentModuleObject,
                                               s_ipsc->cAlphaArgs(1),
                                               s_ipsc->cNumericFieldNames(1),
                                               s_ipsc->rNumericArgs(1)));
                        ShowContinueError(state, "..Allowable values must be between 0 C and 50 C");
                        ErrorsFound = true;
                    }
                }
                if (NumNums > 1) {
                    comfortZone.TdbMaxSetPoint = s_ipsc->rNumericArgs(2);
                    if (s_ipsc->rNumericArgs(2) > 50 || s_ipsc->rNumericArgs(2) < 0) {
                        ShowSevereError(state,
                                        format("{}=\"{} invalid {}=[{:.0T}].",
                                               s_ipsc->cCurrentModuleObject,
                                               s_ipsc->cAlphaArgs(1),
                                               s_ipsc->cNumericFieldNames(2),
                                               s_ipsc->rNumericArgs(2)));
                        ShowContinueError(state, "..Allowable values must be between 0 C and 50 C");
                        ErrorsFound = true;
                    }
                }
                // Ensure MaxTemp >= MinTemp
                if (comfortZone.TdbMinSetPoint > comfortZone.TdbMaxSetPoint) {
                    ShowSevereError(state, format("{}=\"{}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                    ShowContinueError(state, format("..{} > {}", s_ipsc->cNumericFieldNames(1), s_ipsc->cNumericFieldNames(2)));
                    ShowContinueError(state, format("..[{:.0T}] > [{:.0T}].", s_ipsc->rNumericArgs(1), s_ipsc->rNumericArgs(2)));
                    ErrorsFound = true;
                }
                // If MaxTemp = MinTemp, no thermal comfort control
                if (comfortZone.TdbMinSetPoint == comfortZone.TdbMaxSetPoint) {
                    ShowSevereError(state, format("{}=\"{}", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                    ShowContinueError(state, format("..{} = {}", s_ipsc->cNumericFieldNames(1), s_ipsc->cNumericFieldNames(2)));
                    ShowContinueError(state, "The zone will be controlled using this dry-bulb temperature setpoint.");
                }

                // read Thermal comfort type schedule name
                if (s_ipsc->lAlphaFieldBlanks(5)) {
                    ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(5));
                    ErrorsFound = true;
                } else if ((comfortZone.setptTypeSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(5))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5));
                    ErrorsFound = true;
                } else if (!comfortZone.setptTypeSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 4.0)) {
                    Sched::ShowSevereBadMinMax(state, eoh, s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5), Clusive::In, 0.0, Clusive::In, 4.0);
                    ErrorsFound = true;
                }

                int NumSetptTypes = nint((NumAlphas - 5.0) / 2.0);

                for (int iSetptType = 1; iSetptType <= NumSetptTypes; ++iSetptType) {

                    int ctIdx = 2 * iSetptType - 1 + 5;

                    HVAC::SetptType setptType = HVAC::SetptType::Invalid;
                    if (s_ipsc->lAlphaFieldBlanks(ctIdx)) {
                        ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(ctIdx));
                        ErrorsFound = true;
                        continue;
                    } else if ((setptType = static_cast<HVAC::SetptType>(getEnumValue(comfortSetptTypeNamesUC, s_ipsc->cAlphaArgs(ctIdx)))) ==
                               HVAC::SetptType::Invalid) {
                        ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(ctIdx), s_ipsc->cAlphaFieldNames(ctIdx));
                        ErrorsFound = true;
                        continue;
                    }

                    auto &setpt = comfortZone.setpts[(int)setptType];
                    setpt.Name = s_ipsc->cAlphaArgs(nint(2.0 * iSetptType + 5));
                    setpt.isUsed = true;
                }
            }
        } // NumComfortTStatStatements
    }
    // End of Thermal comfort control reading and checking

    s_ipsc->cCurrentModuleObject = comfortSetptTypeNames[(int)HVAC::SetptType::SingleHeat];
    s_ztpc->NumComfortControls[(int)HVAC::SetptType::SingleHeat] = s_ip->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

    if (s_ztpc->NumComfortControls[(int)HVAC::SetptType::SingleHeat] > 0) {
        s_ztpc->comfortSetptScheds[(int)HVAC::SetptType::SingleHeat].allocate(s_ztpc->NumComfortControls[(int)HVAC::SetptType::SingleHeat]);
    }

    for (int idx = 1; idx <= s_ztpc->NumComfortControls[(int)HVAC::SetptType::SingleHeat]; ++idx) {
        s_ip->getObjectItem(state,
                            s_ipsc->cCurrentModuleObject,
                            idx,
                            s_ipsc->cAlphaArgs,
                            NumAlphas,
                            s_ipsc->rNumericArgs,
                            NumNums,
                            IOStat,
                            s_ipsc->lNumericFieldBlanks,
                            s_ipsc->lAlphaFieldBlanks,
                            s_ipsc->cAlphaFieldNames,
                            s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

        Util::IsNameEmpty(state, s_ipsc->cAlphaArgs(1), s_ipsc->cCurrentModuleObject, ErrorsFound);
        auto &setpt = s_ztpc->comfortSetptScheds[(int)HVAC::SetptType::SingleHeat](idx);
        setpt.Name = s_ipsc->cAlphaArgs(1);

        if (s_ipsc->lAlphaFieldBlanks(2)) {
            ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(2));
            ErrorsFound = true;
        } else if ((setpt.heatSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
            ErrorsFound = true;
        } else if (!setpt.heatSched->checkMinMaxVals(state, Clusive::In, -3.0, Clusive::In, 3.0)) {
            Sched::ShowSevereBadMinMax(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2), Clusive::In, -3.0, Clusive::In, 3.0);
            ErrorsFound = true;
        }
    } // SingleFangerHeatingControlNum

    s_ipsc->cCurrentModuleObject = comfortSetptTypeNames[(int)HVAC::SetptType::SingleCool];
    s_ztpc->NumComfortControls[(int)HVAC::SetptType::SingleCool] = s_ip->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

    if (s_ztpc->NumComfortControls[(int)HVAC::SetptType::SingleCool] > 0) {
        s_ztpc->comfortSetptScheds[(int)HVAC::SetptType::SingleCool].allocate(s_ztpc->NumComfortControls[(int)HVAC::SetptType::SingleCool]);
    }

    for (int idx = 1; idx <= s_ztpc->NumComfortControls[(int)HVAC::SetptType::SingleCool]; ++idx) {
        s_ip->getObjectItem(state,
                            s_ipsc->cCurrentModuleObject,
                            idx,
                            s_ipsc->cAlphaArgs,
                            NumAlphas,
                            s_ipsc->rNumericArgs,
                            NumNums,
                            IOStat,
                            s_ipsc->lNumericFieldBlanks,
                            s_ipsc->lAlphaFieldBlanks,
                            s_ipsc->cAlphaFieldNames,
                            s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

        Util::IsNameEmpty(state, s_ipsc->cAlphaArgs(1), s_ipsc->cCurrentModuleObject, ErrorsFound);
        auto &setpt = s_ztpc->comfortSetptScheds[(int)HVAC::SetptType::SingleCool](idx);
        setpt.Name = s_ipsc->cAlphaArgs(1);

        if (s_ipsc->lAlphaFieldBlanks(2)) {
            ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(2));
            ErrorsFound = true;
        } else if ((setpt.coolSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
            ErrorsFound = true;
        } else if (!setpt.coolSched->checkMinMaxVals(state, Clusive::In, -3.0, Clusive::In, 3.0)) {
            Sched::ShowSevereBadMinMax(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2), Clusive::In, -3.0, Clusive::In, 3.0);
            ErrorsFound = true;
        }

    } // SingleFangerCoolingControlNum

    s_ipsc->cCurrentModuleObject = comfortSetptTypeNames[(int)HVAC::SetptType::SingleHeatCool];
    s_ztpc->NumComfortControls[(int)HVAC::SetptType::SingleHeatCool] = s_ip->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

    if (s_ztpc->NumComfortControls[(int)HVAC::SetptType::SingleHeatCool] > 0) {
        s_ztpc->comfortSetptScheds[(int)HVAC::SetptType::SingleHeatCool].allocate(s_ztpc->NumComfortControls[(int)HVAC::SetptType::SingleHeatCool]);
    }

    for (int idx = 1; idx <= s_ztpc->NumComfortControls[(int)HVAC::SetptType::SingleHeatCool]; ++idx) {
        s_ip->getObjectItem(state,
                            s_ipsc->cCurrentModuleObject,
                            idx,
                            s_ipsc->cAlphaArgs,
                            NumAlphas,
                            s_ipsc->rNumericArgs,
                            NumNums,
                            IOStat,
                            s_ipsc->lNumericFieldBlanks,
                            s_ipsc->lAlphaFieldBlanks,
                            s_ipsc->cAlphaFieldNames,
                            s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

        Util::IsNameEmpty(state, s_ipsc->cAlphaArgs(1), s_ipsc->cCurrentModuleObject, ErrorsFound);
        auto &setpt = s_ztpc->comfortSetptScheds[(int)HVAC::SetptType::SingleHeatCool](idx);
        setpt.Name = s_ipsc->cAlphaArgs(1);

        if (s_ipsc->lAlphaFieldBlanks(2)) {
            ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(2));
            ErrorsFound = true;
        } else if ((setpt.heatSched = setpt.coolSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
            ErrorsFound = true;
        } else if (!setpt.heatSched->checkMinMaxVals(state, Clusive::In, -3.0, Clusive::In, 3.0)) {
            Sched::ShowSevereBadMinMax(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2), Clusive::In, -3.0, Clusive::In, 3.0);
            ErrorsFound = true;
        }

    } // SingleFangerHeatCoolControlNum

    s_ipsc->cCurrentModuleObject = comfortSetptTypeNames[(int)HVAC::SetptType::DualHeatCool];
    s_ztpc->NumComfortControls[(int)HVAC::SetptType::DualHeatCool] = s_ip->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

    if (s_ztpc->NumComfortControls[(int)HVAC::SetptType::DualHeatCool] > 0) {
        s_ztpc->comfortSetptScheds[(int)HVAC::SetptType::DualHeatCool].allocate(s_ztpc->NumComfortControls[(int)HVAC::SetptType::DualHeatCool]);
    }

    for (int idx = 1; idx <= s_ztpc->NumComfortControls[(int)HVAC::SetptType::DualHeatCool]; ++idx) {
        s_ip->getObjectItem(state,
                            s_ipsc->cCurrentModuleObject,
                            idx,
                            s_ipsc->cAlphaArgs,
                            NumAlphas,
                            s_ipsc->rNumericArgs,
                            NumNums,
                            IOStat,
                            s_ipsc->lNumericFieldBlanks,
                            s_ipsc->lAlphaFieldBlanks,
                            s_ipsc->cAlphaFieldNames,
                            s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

        Util::IsNameEmpty(state, s_ipsc->cAlphaArgs(1), s_ipsc->cCurrentModuleObject, ErrorsFound);
        auto &setpt = s_ztpc->comfortSetptScheds[(int)HVAC::SetptType::DualHeatCool](idx);
        setpt.Name = s_ipsc->cAlphaArgs(1);

        if (s_ipsc->lAlphaFieldBlanks(2)) {
            ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(2));
            ErrorsFound = true;
        } else if ((setpt.heatSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
            ErrorsFound = true;
        } else if (!setpt.heatSched->checkMinMaxVals(state, Clusive::In, -3.0, Clusive::In, 3.0)) {
            Sched::ShowSevereBadMinMax(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2), Clusive::In, -3.0, Clusive::In, 3.0);
            ErrorsFound = true;
        }

        if (s_ipsc->lAlphaFieldBlanks(3)) {
            ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(3));
            ErrorsFound = true;
        } else if ((setpt.coolSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(3))) == nullptr) {
            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
            ErrorsFound = true;
        } else if (!setpt.coolSched->checkMinMaxVals(state, Clusive::In, -3.0, Clusive::In, 3.0)) {
            Sched::ShowSevereBadMinMax(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3), Clusive::In, -3.0, Clusive::In, 3.0);
            ErrorsFound = true;
        }

    } // DualFangerHeatCoolControlNum

    // Finish filling in Schedule pointing indexes for Thermal Comfort Control
    for (ComfortControlledZoneNum = 1; ComfortControlledZoneNum <= state.dataZoneCtrls->NumComfortControlledZones; ++ComfortControlledZoneNum) {

        auto &comfortZone = state.dataZoneCtrls->ComfortControlledZone(ComfortControlledZoneNum);

        for (HVAC::SetptType setptType : HVAC::controlledSetptTypes) {
            auto &setpt = comfortZone.setpts[(int)setptType];
            if (!setpt.isUsed) {
                continue;
            }

            int setptIdx = Util::FindItem(setpt.Name, s_ztpc->comfortSetptScheds[(int)setptType]);

            if (setptType == HVAC::SetptType::SingleHeat || setptType == HVAC::SetptType::SingleHeatCool ||
                setptType == HVAC::SetptType::DualHeatCool) {
                setpt.heatSetptSched = s_ztpc->comfortSetptScheds[(int)setptType](setptIdx).heatSched;
            }

            if (setptType == HVAC::SetptType::SingleCool || setptType == HVAC::SetptType::SingleHeatCool ||
                setptType == HVAC::SetptType::DualHeatCool) {
                setpt.coolSetptSched = s_ztpc->comfortSetptScheds[(int)setptType](setptIdx).coolSched;
            }

            TComfortControlTypes(ComfortControlledZoneNum).MustHave[(int)setptType] = true;
        }
    }

    // Now, Check the schedule values/indices for validity for Thermal Comfort Control

    for (ComfortControlledZoneNum = 1; ComfortControlledZoneNum <= state.dataZoneCtrls->NumComfortControlledZones; ++ComfortControlledZoneNum) {
        auto &comfortZone = state.dataZoneCtrls->ComfortControlledZone(ComfortControlledZoneNum);
        ActualZoneNum = comfortZone.ActualZoneNum;
        if (comfortZone.setptTypeSched == nullptr) {
            continue;
        }

        int SchedMin = comfortZone.setptTypeSched->getMinVal(state);
        int SchedMax = comfortZone.setptTypeSched->getMaxVal(state);

        if (SchedMin == (int)HVAC::SetptType::Uncontrolled && SchedMax == (int)HVAC::SetptType::Uncontrolled) {
            if (FindNumberInList(comfortZone.setptTypeSched->Num, CCmSchedMapToControlledZone, state.dataZoneCtrls->NumComfortControlledZones) == 0) {
                ShowWarningError(state, format("Control Type Schedule={}", comfortZone.setptTypeSched->Name));
                ShowContinueError(state, "..specifies control type 0 for all entries.");
                ShowContinueError(state, "All zones using this Control Type Schedule have no thermal comfort control.");
            }
            CCmSchedMapToControlledZone(ComfortControlledZoneNum) = comfortZone.setptTypeSched->Num;
        }

        for (HVAC::SetptType setptType : HVAC::controlledSetptTypes) {
            auto const &setpt = comfortZone.setpts[(int)setptType];
            if (!setpt.isUsed) {
                continue;
            }

            TComfortControlTypes(ComfortControlledZoneNum).DidHave[(int)setptType] = true;

            if (setpt.heatSetptSched == nullptr &&
                (setptType == HVAC::SetptType::SingleHeat || setptType == HVAC::SetptType::SingleHeatCool ||
                 setptType == HVAC::SetptType::DualHeatCool) &&
                comfortZone.setptTypeSched->hasVal(state, (int)setptType)) {
                ShowSevereError(state, format("Control Type Schedule={}", comfortZone.setptTypeSched->Name));
                ShowContinueError(
                    state,
                    format("..specifies {} ({}) as the control type. Not valid for this zone.", (int)setptType, setptTypeNames[(int)setptType]));
                ShowContinueError(state, format("..reference {}={}", cZControlTypes((int)ZoneControlTypes::TStat), comfortZone.Name));
                ShowContinueError(state, format("..reference ZONE={}", comfortZone.ZoneName));
                ErrorsFound = true;
            }

            if (setpt.coolSetptSched == nullptr &&
                (setptType == HVAC::SetptType::SingleCool || setptType == HVAC::SetptType::SingleHeatCool ||
                 setptType == HVAC::SetptType::DualHeatCool) &&
                comfortZone.setptTypeSched->hasVal(state, (int)setptType)) {
                ShowSevereError(state, format("Control Type Schedule={}", comfortZone.setptTypeSched->Name));
                ShowContinueError(
                    state,
                    format("..specifies {} ({}) as the control type. Not valid for this zone.", (int)setptType, setptTypeNames[(int)setptType]));
                ShowContinueError(state, format("..reference {}={}", cZControlTypes((int)ZoneControlTypes::TStat), comfortZone.Name));
                ShowContinueError(state, format("..reference ZONE={}", comfortZone.ZoneName));
                ErrorsFound = true;
            }
        } // for (setptType)
    } // for (ComfortControlledZoneNum)

    for (int ComfortControlledZoneNum = 1; ComfortControlledZoneNum <= state.dataZoneCtrls->NumComfortControlledZones; ++ComfortControlledZoneNum) {

        auto &comfortZone = state.dataZoneCtrls->ComfortControlledZone(ComfortControlledZoneNum);
        ActualZoneNum = comfortZone.ActualZoneNum;
        if (comfortZone.setptTypeSched == nullptr) {
            continue;
        }

        for (HVAC::SetptType setptType : HVAC::controlledSetptTypes) {
            if (TComfortControlTypes(ComfortControlledZoneNum).MustHave[(int)setptType] &&
                TComfortControlTypes(ComfortControlledZoneNum).DidHave[(int)setptType]) {
                continue;
            }

            if (!TComfortControlTypes(ComfortControlledZoneNum).MustHave[(int)setptType]) {
                continue;
            }

            ShowWarningError(state, format("Schedule={}", comfortZone.setptTypeSched->Name));
            ShowContinueError(state,
                              format("...should include control type {} ({}) but does not.", (int)setptType, comfortSetptTypeNames[(int)setptType]));
            ShowContinueError(state, format("..reference {}={}", cZControlTypes((int)ZoneControlTypes::TCTStat), comfortZone.Name));
            ShowContinueError(state, format("...reference ZONE={}", comfortZone.ZoneName));
        }
    }

    if (allocated(TComfortControlTypes)) {
        TComfortControlTypes.deallocate();
    }

    // Get the Hybrid Model setting inputs
    HybridModel::GetHybridModelZone(state);

    // Default multiplier values
    Real64 ZoneVolCapMultpSens = 1.0;
    Real64 ZoneVolCapMultpMoist = 1.0;
    Real64 ZoneVolCapMultpCO2 = 1.0;
    Real64 ZoneVolCapMultpGenContam = 1.0;

    // Get the Zone Air Capacitance Multiplier for use in the Predictor-Corrector Procedure
    s_ipsc->cCurrentModuleObject = "ZoneCapacitanceMultiplier:ResearchSpecial";
    int NumZoneCapaMultiplier = s_ip->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject); // Number of ZonesCapacityMultiplier object
    if (NumZoneCapaMultiplier == 0) {
        // Assign default multiplier values to all zones
        for (int ZoneNum = 1; ZoneNum <= NumOfZones; ZoneNum++) {
            auto &Zone = state.dataHeatBal->Zone(ZoneNum);
            Zone.ZoneVolCapMultpSens = ZoneVolCapMultpSens;
            Zone.ZoneVolCapMultpMoist = ZoneVolCapMultpMoist;
            Zone.ZoneVolCapMultpCO2 = ZoneVolCapMultpCO2;
            Zone.ZoneVolCapMultpGenContam = ZoneVolCapMultpGenContam;
        }

    } else {

        // Allow user to specify ZoneCapacitanceMultiplier:ResearchSpecial at zone level
        // Added by S. Lee and R. Zhang in Oct. 2016.
        // Assign the user inputted multipliers to specified zones
        for (int ZoneCapNum = 1; ZoneCapNum <= NumZoneCapaMultiplier; ZoneCapNum++) {
            s_ip->getObjectItem(state,
                                s_ipsc->cCurrentModuleObject,
                                ZoneCapNum,
                                s_ipsc->cAlphaArgs,
                                NumAlphas,
                                s_ipsc->rNumericArgs,
                                NumNums,
                                IOStat,
                                s_ipsc->lNumericFieldBlanks,
                                s_ipsc->lAlphaFieldBlanks,
                                s_ipsc->cAlphaFieldNames,
                                s_ipsc->cNumericFieldNames);

            if (s_ipsc->lAlphaFieldBlanks(2)) {
                // default multiplier values for all the zones not specified (zone or zonelist name field is empty)
                ZoneVolCapMultpSens = s_ipsc->rNumericArgs(1);
                ZoneVolCapMultpMoist = s_ipsc->rNumericArgs(2);
                ZoneVolCapMultpCO2 = s_ipsc->rNumericArgs(3);
                ZoneVolCapMultpGenContam = s_ipsc->rNumericArgs(4);
            } else {
                // multiplier values for the specified zone(s)
                ZLItem = 0;

                Item1 = Util::FindItemInList(s_ipsc->cAlphaArgs(2), Zone);
                if (Item1 == 0 && state.dataHeatBal->NumOfZoneLists > 0) {
                    ZLItem = Util::FindItemInList(s_ipsc->cAlphaArgs(2), ZoneList);
                }
                if (Item1 > 0) {
                    int ZoneNum = Item1;
                    Zone(ZoneNum).FlagCustomizedZoneCap = true;
                    Zone(ZoneNum).ZoneVolCapMultpSens = s_ipsc->rNumericArgs(1);
                    Zone(ZoneNum).ZoneVolCapMultpMoist = s_ipsc->rNumericArgs(2);
                    Zone(ZoneNum).ZoneVolCapMultpCO2 = s_ipsc->rNumericArgs(3);
                    Zone(ZoneNum).ZoneVolCapMultpGenContam = s_ipsc->rNumericArgs(4);
                } else if (ZLItem > 0) {
                    for (int ZonePtrNum = 1; ZonePtrNum < ZoneList(ZLItem).NumOfZones; ZonePtrNum++) {
                        int ZoneNum = ZoneList(ZLItem).Zone(ZonePtrNum);
                        Zone(ZoneNum).FlagCustomizedZoneCap = true;
                        Zone(ZoneNum).ZoneVolCapMultpSens = s_ipsc->rNumericArgs(1);
                        Zone(ZoneNum).ZoneVolCapMultpMoist = s_ipsc->rNumericArgs(2);
                        Zone(ZoneNum).ZoneVolCapMultpCO2 = s_ipsc->rNumericArgs(3);
                        Zone(ZoneNum).ZoneVolCapMultpGenContam = s_ipsc->rNumericArgs(4);
                    }

                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {}=\"{}\" not found.",
                                           s_ipsc->cCurrentModuleObject,
                                           s_ipsc->cAlphaArgs(1),
                                           s_ipsc->cAlphaFieldNames(2),
                                           s_ipsc->cAlphaArgs(2)));
                    ErrorsFound = true;
                }
            }
        }

        // Assign default multiplier values to all the other zones
        for (int ZoneNum = 1; ZoneNum <= NumOfZones; ZoneNum++) {
            auto &Zone = state.dataHeatBal->Zone(ZoneNum);
            if (!Zone.FlagCustomizedZoneCap) {
                Zone.ZoneVolCapMultpSens = ZoneVolCapMultpSens;
                Zone.ZoneVolCapMultpMoist = ZoneVolCapMultpMoist;
                Zone.ZoneVolCapMultpCO2 = ZoneVolCapMultpCO2;
                Zone.ZoneVolCapMultpGenContam = ZoneVolCapMultpGenContam;
            }
        }

        // Calculate the average multiplier value from all zones
        {
            Real64 ZoneVolCapMultpSens_temp = 0.0;
            Real64 ZoneVolCapMultpMoist_temp = 0.0;
            Real64 ZoneVolCapMultpCO2_temp = 0.0;
            Real64 ZoneVolCapMultpGenContam_temp = 0.0;

            for (int ZoneNum = 1; ZoneNum <= NumOfZones; ZoneNum++) {
                auto const &Zone = state.dataHeatBal->Zone(ZoneNum);
                ZoneVolCapMultpSens_temp += Zone.ZoneVolCapMultpSens;
                ZoneVolCapMultpMoist_temp += Zone.ZoneVolCapMultpMoist;
                ZoneVolCapMultpCO2_temp += Zone.ZoneVolCapMultpCO2;
                ZoneVolCapMultpGenContam_temp += Zone.ZoneVolCapMultpGenContam;
            }

            if (NumOfZones > 0) {
                ZoneVolCapMultpSens = ZoneVolCapMultpSens_temp / NumOfZones;
                ZoneVolCapMultpMoist = ZoneVolCapMultpMoist_temp / NumOfZones;
                ZoneVolCapMultpCO2 = ZoneVolCapMultpCO2_temp / NumOfZones;
                ZoneVolCapMultpGenContam = ZoneVolCapMultpGenContam_temp / NumOfZones;
            }
        }
    }

    print(state.files.eio, Header);
    print(state.files.eio, Format_701, ZoneVolCapMultpSens, ZoneVolCapMultpMoist, ZoneVolCapMultpCO2, ZoneVolCapMultpGenContam);

    s_ipsc->cCurrentModuleObject = cZControlTypes((int)ZoneControlTypes::OTTStat);
    state.dataZoneCtrls->NumOpTempControlledZones = s_ip->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

    if (state.dataZoneCtrls->NumOpTempControlledZones > 0) {
        state.dataZoneCtrls->AnyOpTempControl = true;

        for (int idx = 1; idx <= state.dataZoneCtrls->NumOpTempControlledZones; ++idx) {
            s_ip->getObjectItem(state,
                                s_ipsc->cCurrentModuleObject,
                                idx,
                                s_ipsc->cAlphaArgs,
                                NumAlphas,
                                s_ipsc->rNumericArgs,
                                NumNums,
                                IOStat,
                                s_ipsc->lNumericFieldBlanks,
                                s_ipsc->lAlphaFieldBlanks,
                                s_ipsc->cAlphaFieldNames,
                                s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

            // find matching name of  ZONECONTROL:THERMOSTAT object
            if ((found = Util::FindItem(s_ipsc->cAlphaArgs(1), TStatObjects)) != 0) {
                auto const &TStatObjects = state.dataZoneCtrls->TStatObjects(found);
                for (Item = 1; Item <= TStatObjects.NumOfZones; ++Item) {
                    TempControlledZoneNum = TStatObjects.TempControlledZoneStartPtr + Item - 1;
                    auto &TempControlledZone = state.dataZoneCtrls->TempControlledZone(TempControlledZoneNum);
                    if (state.dataZoneCtrls->NumTempControlledZones == 0) {
                        continue;
                    }
                    auto &tempZone = state.dataZoneCtrls->TempControlledZone(TempControlledZoneNum);
                    tempZone.OpTempCtrl =
                        static_cast<DataZoneControls::TempCtrl>(getEnumValue(DataZoneControls::tempCtrlNamesUC, s_ipsc->cAlphaArgs(2)));

                    if (Item == 1) {
                        if (tempZone.OpTempCtrl != DataZoneControls::TempCtrl::Constant &&
                            tempZone.OpTempCtrl != DataZoneControls::TempCtrl::Scheduled) {
                            ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
                            ErrorsFound = true;
                        }
                    }

                    tempZone.FixedRadiativeFraction = s_ipsc->rNumericArgs(1);

                    if (tempZone.OpTempCtrl == DataZoneControls::TempCtrl::Scheduled) {
                        if (s_ipsc->lAlphaFieldBlanks(3)) {
                            if (Item == 1) {
                                ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(3));
                                ErrorsFound = true;
                            }
                        } else if ((tempZone.opTempRadiativeFractionSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(3))) == nullptr) {
                            if (Item == 1) {
                                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
                                ErrorsFound = true;
                            }
                        } else if (!tempZone.opTempRadiativeFractionSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::Ex, 0.9)) {
                            if (Item == 1) {
                                Sched::ShowSevereBadMinMax(
                                    state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3), Clusive::In, 0.0, Clusive::Ex, 0.9);
                                ErrorsFound = true;
                            }
                        }

                    } else { // !tempZone.OpTempCntrlModeScheduled

                        // check validity of fixed radiative fraction
                        if (Item == 1) {
                            if (tempZone.FixedRadiativeFraction < 0.0) {
                                ShowSevereBadMin(state, eoh, s_ipsc->cNumericFieldNames(1), s_ipsc->rNumericArgs(1), Clusive::In, 0.0);
                                ErrorsFound = true;
                            } else if (tempZone.FixedRadiativeFraction >= 0.9) {
                                ShowSevereBadMax(state, eoh, s_ipsc->cNumericFieldNames(1), s_ipsc->rNumericArgs(1), Clusive::Ex, 0.9);
                                ErrorsFound = true;
                            }
                        }
                    }

                    // added Jan, 2017 - Xuan Luo
                    // read adaptive comfort model and calculate adaptive thermal comfort setpoint
                    if (tempZone.OpTempCtrl == DataZoneControls::TempCtrl::Constant || tempZone.OpTempCtrl == DataZoneControls::TempCtrl::Scheduled) {
                        if (NumAlphas >= 4 && !s_ipsc->lAlphaFieldBlanks(4)) {
                            int adaptiveComfortModelTypeIndex =
                                Util::FindItem(s_ipsc->cAlphaArgs(4), AdaptiveComfortModelTypes, AdaptiveComfortModelTypes.isize());
                            if (!adaptiveComfortModelTypeIndex) {
                                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4));
                                ErrorsFound = true;
                            } else if (adaptiveComfortModelTypeIndex != static_cast<int>(AdaptiveComfortModel::ADAP_NONE)) {
                                tempZone.AdaptiveComfortTempControl = true;
                                tempZone.AdaptiveComfortModelTypeIndex =
                                    Util::FindItem(s_ipsc->cAlphaArgs(4), AdaptiveComfortModelTypes, AdaptiveComfortModelTypes.isize());
                                if (!s_ztpc->AdapComfortDailySetPointSchedule.initialized) {
                                    Array1D<Real64> runningAverageASH(state.dataWeather->NumDaysInYear, 0.0);
                                    Array1D<Real64> runningAverageCEN(state.dataWeather->NumDaysInYear, 0.0);
                                    CalculateMonthlyRunningAverageDryBulb(state, runningAverageASH, runningAverageCEN);
                                    CalculateAdaptiveComfortSetPointSchl(state, runningAverageASH, runningAverageCEN);
                                }
                            }
                        }
                    }

                    // CurrentModuleObject='ZoneControl:Thermostat:OperativeTemperature'
                    SetupOutputVariable(state,
                                        "Zone Thermostat Operative Temperature",
                                        Constant::Units::C,
                                        state.dataHeatBal->ZnAirRpt(tempZone.ActualZoneNum).ThermOperativeTemp,
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        Zone(tempZone.ActualZoneNum).Name);
                } // TStat Objects Loop

                // It might be in the TempControlledZones
            } else if ((found = Util::FindItem(s_ipsc->cAlphaArgs(1), state.dataZoneCtrls->TempControlledZone)) != 0) {

                TempControlledZoneNum = found;
                auto &tempZone = state.dataZoneCtrls->TempControlledZone(TempControlledZoneNum);
                tempZone.OpTempCtrl = static_cast<DataZoneControls::TempCtrl>(getEnumValue(DataZoneControls::tempCtrlNamesUC, s_ipsc->cAlphaArgs(2)));
                if (tempZone.OpTempCtrl == DataZoneControls::TempCtrl::Invalid || tempZone.OpTempCtrl == DataZoneControls::TempCtrl::None) {
                    ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
                    ErrorsFound = true;
                }

                tempZone.FixedRadiativeFraction = s_ipsc->rNumericArgs(1);

                if (tempZone.OpTempCtrl == DataZoneControls::TempCtrl::Scheduled) {
                    if (s_ipsc->lAlphaFieldBlanks(3)) {
                        ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(3));
                        ErrorsFound = true;
                    } else if ((tempZone.opTempRadiativeFractionSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(3))) == nullptr) {
                        ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
                        ErrorsFound = true;
                    } else if (!tempZone.opTempRadiativeFractionSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::Ex, 0.9)) {
                        Sched::ShowSevereBadMinMax(
                            state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3), Clusive::In, 0.0, Clusive::Ex, 0.9);
                        ErrorsFound = true;
                    }

                } else { // !tempZone.OpTempCntrlModeScheduled

                    if (tempZone.FixedRadiativeFraction < 0.0) {
                        ShowSevereBadMin(state, eoh, s_ipsc->cNumericFieldNames(1), s_ipsc->rNumericArgs(1), Clusive::In, 0.0);
                        ErrorsFound = true;
                    } else if (tempZone.FixedRadiativeFraction >= 0.9) {
                        ShowSevereBadMax(state, eoh, s_ipsc->cNumericFieldNames(1), s_ipsc->rNumericArgs(1), Clusive::Ex, 0.9);
                        ErrorsFound = true;
                    }
                }

                // added Jan, 2017 - Xuan Luo
                // read adaptive comfort model and calculate adaptive thermal comfort setpoint
                if (tempZone.OpTempCtrl == DataZoneControls::TempCtrl::Constant || tempZone.OpTempCtrl == DataZoneControls::TempCtrl::Scheduled) {
                    if (NumAlphas >= 4 && !s_ipsc->lAlphaFieldBlanks(4)) {
                        int adaptiveComfortModelTypeIndex =
                            Util::FindItem(s_ipsc->cAlphaArgs(4), AdaptiveComfortModelTypes, AdaptiveComfortModelTypes.isize());
                        if (!adaptiveComfortModelTypeIndex) {
                            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4));
                            ErrorsFound = true;
                        } else if (adaptiveComfortModelTypeIndex != static_cast<int>(AdaptiveComfortModel::ADAP_NONE)) {
                            tempZone.AdaptiveComfortTempControl = true;
                            tempZone.AdaptiveComfortModelTypeIndex = adaptiveComfortModelTypeIndex;
                            if (!s_ztpc->AdapComfortDailySetPointSchedule.initialized) {
                                Array1D<Real64> runningAverageASH(state.dataWeather->NumDaysInYear, 0.0);
                                Array1D<Real64> runningAverageCEN(state.dataWeather->NumDaysInYear, 0.0);
                                // What does this accomplish?
                                CalculateMonthlyRunningAverageDryBulb(state, runningAverageASH, runningAverageCEN);
                                CalculateAdaptiveComfortSetPointSchl(state, runningAverageASH, runningAverageCEN);
                            }
                        }
                    }

                    // CurrentModuleObject='ZoneControl:Thermostat:OperativeTemperature'
                    SetupOutputVariable(state,
                                        "Zone Thermostat Operative Temperature",
                                        Constant::Units::C,
                                        state.dataHeatBal->ZnAirRpt(tempZone.ActualZoneNum).ThermOperativeTemp,
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        Zone(tempZone.ActualZoneNum).Name);
                }
                // throw error
            } else {
                ShowSevereError(state,
                                format("{}={} invalid {} reference not found.",
                                       s_ipsc->cCurrentModuleObject,
                                       s_ipsc->cAlphaArgs(1),
                                       cZControlTypes(static_cast<int>(ZoneControlTypes::TStat))));
                ErrorsFound = true;
            }
        } // loop over NumOpTempControlledZones
    } // NumOpTempControlledZones > 0

    // Overcool dehumidificaton GetInput starts here
    s_ipsc->cCurrentModuleObject = cZControlTypes((int)ZoneControlTypes::TandHStat);
    state.dataZoneCtrls->NumTempAndHumidityControlledZones = s_ip->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
    if (state.dataZoneCtrls->NumTempAndHumidityControlledZones > 0) {
        state.dataZoneCtrls->AnyZoneTempAndHumidityControl = true;

        for (int idx = 1; idx <= state.dataZoneCtrls->NumTempAndHumidityControlledZones; ++idx) {
            s_ip->getObjectItem(state,
                                s_ipsc->cCurrentModuleObject,
                                idx,
                                s_ipsc->cAlphaArgs,
                                NumAlphas,
                                s_ipsc->rNumericArgs,
                                NumNums,
                                IOStat,
                                s_ipsc->lNumericFieldBlanks,
                                s_ipsc->lAlphaFieldBlanks,
                                s_ipsc->cAlphaFieldNames,
                                s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

            // find matching name of  ZONECONTROL:THERMOSTAT object
            found = Util::FindItem(s_ipsc->cAlphaArgs(1), TStatObjects);
            if (found == 0) {
                // It might be in the TempControlledZones
                found = Util::FindItem(s_ipsc->cAlphaArgs(1), state.dataZoneCtrls->TempControlledZone);
                if (found == 0) { // throw error
                    ShowSevereError(state,
                                    format("{}={} invalid {} reference not found.",
                                           s_ipsc->cCurrentModuleObject,
                                           s_ipsc->cAlphaArgs(1),
                                           cZControlTypes(static_cast<int>(ZoneControlTypes::TStat))));
                    ErrorsFound = true;
                } else {
                    TempControlledZoneNum = found;
                    auto &tempZone = state.dataZoneCtrls->TempControlledZone(TempControlledZoneNum);

                    if (s_ipsc->lAlphaFieldBlanks(2)) {
                        ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(2));
                        ErrorsFound = true;
                    } else if ((tempZone.dehumidifyingSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
                        ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
                        ErrorsFound = true;
                    }

                    tempZone.OvercoolCtrl =
                        static_cast<DataZoneControls::TempCtrl>(getEnumValue(DataZoneControls::tempCtrlNamesUC, s_ipsc->cAlphaArgs(3)));
                    if (tempZone.OvercoolCtrl == DataZoneControls::TempCtrl::Invalid) {
                        ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4));
                        ErrorsFound = true;
                    }

                    tempZone.ZoneOvercoolConstRange = s_ipsc->rNumericArgs(1);

                    if (tempZone.OvercoolCtrl == DataZoneControls::TempCtrl::Scheduled) {
                        if (s_ipsc->lAlphaFieldBlanks(5)) {
                            ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(5));
                            ErrorsFound = true;
                        } else if ((tempZone.zoneOvercoolRangeSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(5))) == nullptr) {
                            ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5));
                            ErrorsFound = true;
                        } else if (!tempZone.zoneOvercoolRangeSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 3.0)) {
                            Sched::ShowSevereBadMinMax(
                                state, eoh, s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5), Clusive::In, 0.0, Clusive::In, 3.0);
                            ErrorsFound = true;
                        }
                    } else { // !tempZone.OvercoolCntrlModeScheduled

                        if (tempZone.ZoneOvercoolConstRange < 0.0) {
                            ShowSevereBadMin(state, eoh, s_ipsc->cNumericFieldNames(1), s_ipsc->rNumericArgs(1), Clusive::In, 0.0);
                            ErrorsFound = true;
                        } else if (tempZone.ZoneOvercoolConstRange > 3.0) {
                            ShowSevereBadMax(state, eoh, s_ipsc->cNumericFieldNames(1), s_ipsc->rNumericArgs(1), Clusive::In, 3.0);
                            ErrorsFound = true;
                        }

                        // check Overcool Control Ratio limits
                        tempZone.ZoneOvercoolControlRatio = s_ipsc->rNumericArgs(2);
                        if (tempZone.ZoneOvercoolControlRatio < 0.0) {
                            ShowSevereBadMin(state, eoh, s_ipsc->cNumericFieldNames(2), s_ipsc->rNumericArgs(2), Clusive::In, 0.0);
                            ErrorsFound = true;
                        }
                    }
                }

            } else {
                for (int Item = 1; Item <= TStatObjects(found).NumOfZones; ++Item) {
                    TempControlledZoneNum = TStatObjects(found).TempControlledZoneStartPtr + Item - 1;

                    auto &tempZone = state.dataZoneCtrls->TempControlledZone(TempControlledZoneNum);

                    if (s_ipsc->lAlphaFieldBlanks(2)) {
                        ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(2));
                        ErrorsFound = true;
                    } else if ((tempZone.dehumidifyingSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(2))) == nullptr) {
                        ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
                        ErrorsFound = true;
                    }

                    // This (i.e., using two booleans to represent three options) is a bad idiom
                    if (s_ipsc->cAlphaArgs(3) == "NONE") {
                        tempZone.OvercoolCtrl = DataZoneControls::TempCtrl::None;
                    } else if (s_ipsc->cAlphaArgs(3) != "OVERCOOL") {
                        if (Item == 1) {
                            ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
                            ErrorsFound = true;
                        }
                    } else if ((tempZone.OvercoolCtrl = static_cast<DataZoneControls::TempCtrl>(
                                    getEnumValue(DataZoneControls::tempCtrlNamesUC, s_ipsc->cAlphaArgs(4)))) == DataZoneControls::TempCtrl::Invalid) {
                        if (Item == 1) {
                            ShowSevereInvalidKey(state, eoh, s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4));
                            ErrorsFound = true;
                        }
                    }

                    tempZone.ZoneOvercoolConstRange = s_ipsc->rNumericArgs(1);

                    if (tempZone.OvercoolCtrl == DataZoneControls::TempCtrl::Scheduled) {
                        if (s_ipsc->lAlphaFieldBlanks(6)) {
                            if (Item == 1) {
                                ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(6));
                                ErrorsFound = true;
                            }
                        } else if ((tempZone.zoneOvercoolRangeSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(6))) == nullptr) {
                            if (Item == 1) {
                                ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(6), s_ipsc->cAlphaArgs(6));
                                ErrorsFound = true;
                            }
                        } else if (!tempZone.zoneOvercoolRangeSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 3.0)) {
                            if (Item == 1) {
                                Sched::ShowSevereBadMinMax(
                                    state, eoh, s_ipsc->cAlphaFieldNames(6), s_ipsc->cAlphaArgs(6), Clusive::In, 0.0, Clusive::In, 3.0);
                                ErrorsFound = true;
                            }
                        }
                    } else { // tempZone.OvercoolCntrlModeScheduled
                        if (Item == 1) {
                            if (tempZone.ZoneOvercoolConstRange < 0.0) {
                                ShowSevereBadMin(state, eoh, s_ipsc->cNumericFieldNames(1), s_ipsc->rNumericArgs(1), Clusive::In, 0.0);
                                ErrorsFound = true;
                            } else if (tempZone.ZoneOvercoolConstRange > 3.0) {
                                ShowSevereBadMax(state, eoh, s_ipsc->cNumericFieldNames(1), s_ipsc->rNumericArgs(1), Clusive::In, 3.0);
                                ErrorsFound = true;
                            }
                        }
                    }

                    tempZone.ZoneOvercoolControlRatio = s_ipsc->rNumericArgs(2);
                    // check Overcool Control Ratio limits
                    if (tempZone.ZoneOvercoolControlRatio < 0.0) {
                        if (Item == 1) {
                            ShowSevereBadMin(state, eoh, s_ipsc->cNumericFieldNames(2), s_ipsc->rNumericArgs(2), Clusive::In, 0.0);
                            ErrorsFound = true;
                        }
                    }
                } // TStat Objects Loop
            } // found thermostat reference
        } // loop over NumTempAndHumidityControlledZones
    } // NumTempAndHumidityControlledZones > 0

    // Staged thermostat control inputs start
    s_ipsc->cCurrentModuleObject = cZControlTypes((int)ZoneControlTypes::StagedDual);
    NumStageControlledZones = s_ip->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
    if (NumStageControlledZones > 0) {
        state.dataZoneCtrls->StagedTStatObjects.allocate(NumStageControlledZones);
    }

    // Pre-scan for use of Zone lists in TStat statements (i.e. Global application of TStat)
    s_ztpc->NumStageCtrZone = 0;
    for (Item = 1; Item <= NumStageControlledZones; ++Item) {
        s_ip->getObjectItem(state,
                            s_ipsc->cCurrentModuleObject,
                            Item,
                            s_ipsc->cAlphaArgs,
                            NumAlphas,
                            s_ipsc->rNumericArgs,
                            NumNums,
                            IOStat,
                            s_ipsc->lNumericFieldBlanks,
                            s_ipsc->lAlphaFieldBlanks,
                            s_ipsc->cAlphaFieldNames,
                            s_ipsc->cNumericFieldNames);
        Util::IsNameEmpty(state, s_ipsc->cAlphaArgs(1), s_ipsc->cCurrentModuleObject, ErrorsFound);

        state.dataZoneCtrls->StagedTStatObjects(Item).Name = s_ipsc->cAlphaArgs(1);
        Item1 = Util::FindItemInList(s_ipsc->cAlphaArgs(2), Zone);
        ZLItem = 0;
        if (Item1 == 0 && state.dataHeatBal->NumOfZoneLists > 0) {
            ZLItem = Util::FindItemInList(s_ipsc->cAlphaArgs(2), ZoneList);
        }
        if (Item1 > 0) {
            state.dataZoneCtrls->StagedTStatObjects(Item).StageControlledZoneStartPtr = s_ztpc->NumStageCtrZone + 1;
            ++s_ztpc->NumStageCtrZone;
            state.dataZoneCtrls->StagedTStatObjects(Item).NumOfZones = 1;
            state.dataZoneCtrls->StagedTStatObjects(Item).ZoneListActive = false;
            state.dataZoneCtrls->StagedTStatObjects(Item).ZoneOrZoneListPtr = Item1;
        } else if (ZLItem > 0) {
            state.dataZoneCtrls->StagedTStatObjects(Item).TempControlledZoneStartPtr = s_ztpc->NumStageCtrZone + 1;
            s_ztpc->NumStageCtrZone += ZoneList(ZLItem).NumOfZones;
            state.dataZoneCtrls->StagedTStatObjects(Item).NumOfZones = ZoneList(ZLItem).NumOfZones;
            state.dataZoneCtrls->StagedTStatObjects(Item).ZoneListActive = true;
            state.dataZoneCtrls->StagedTStatObjects(Item).ZoneOrZoneListPtr = ZLItem;
        } else {
            ShowSevereError(state,
                            format("{}=\"{}\" invalid {}=\"{}\" not found.",
                                   s_ipsc->cCurrentModuleObject,
                                   s_ipsc->cAlphaArgs(1),
                                   s_ipsc->cAlphaFieldNames(2),
                                   s_ipsc->cAlphaArgs(2)));
            ErrorsFound = true;
        }
    }

    if (ErrorsFound) {
        ShowSevereError(state, format("GetStagedDualSetpoint: Errors with invalid names in {} objects.", s_ipsc->cCurrentModuleObject));
        ShowContinueError(state, "...These will not be read in.  Other errors may occur.");
        s_ztpc->NumStageCtrZone = 0;
    }

    if (s_ztpc->NumStageCtrZone > 0) {
        state.dataZoneCtrls->StageControlledZone.allocate(s_ztpc->NumStageCtrZone);
        state.dataZoneCtrls->StageZoneLogic.dimension(NumOfZones, false);

        int StageControlledZoneNum = 0;
        for (Item = 1; Item <= NumStageControlledZones; ++Item) {
            s_ip->getObjectItem(state,
                                s_ipsc->cCurrentModuleObject,
                                Item,
                                s_ipsc->cAlphaArgs,
                                NumAlphas,
                                s_ipsc->rNumericArgs,
                                NumNums,
                                IOStat,
                                s_ipsc->lNumericFieldBlanks,
                                s_ipsc->lAlphaFieldBlanks,
                                s_ipsc->cAlphaFieldNames,
                                s_ipsc->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

            for (Item1 = 1; Item1 <= state.dataZoneCtrls->StagedTStatObjects(Item).NumOfZones; ++Item1) {
                ++StageControlledZoneNum;

                auto &stageZone = state.dataZoneCtrls->StageControlledZone(StageControlledZoneNum);

                if (state.dataZoneCtrls->StagedTStatObjects(Item).ZoneListActive) {
                    s_ipsc->cAlphaArgs(2) =
                        state.dataHeatBal->Zone(ZoneList(state.dataZoneCtrls->StagedTStatObjects(Item).ZoneOrZoneListPtr).Zone(Item1)).Name;
                }
                int ZoneAssigned = Util::FindItemInList(s_ipsc->cAlphaArgs(2),
                                                        state.dataZoneCtrls->StageControlledZone,
                                                        &DataZoneControls::ZoneStagedControls::ZoneName,
                                                        StageControlledZoneNum - 1);
                if (ZoneAssigned == 0) {
                    stageZone.ZoneName = s_ipsc->cAlphaArgs(2);
                    if ((stageZone.ActualZoneNum = Util::FindItemInList(s_ipsc->cAlphaArgs(2), Zone)) == 0) {
                        ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2));
                        ErrorsFound = true;
                    } else {
                        //           Zone(stageControlledZone%ActualZoneNum)%StageControlledZoneIndex =
                        //           StageControlledZoneNum
                    }
                    state.dataZoneCtrls->StageZoneLogic(stageZone.ActualZoneNum) = true;
                } else {
                    stageZone.ZoneName = s_ipsc->cAlphaArgs(2); // for continuity
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {}=\"{}\" zone previously assigned.",
                                           s_ipsc->cCurrentModuleObject,
                                           s_ipsc->cAlphaArgs(1),
                                           s_ipsc->cAlphaFieldNames(2),
                                           s_ipsc->cAlphaArgs(2)));
                    ShowContinueError(
                        state,
                        format("...Zone was previously assigned to Setpt=\"{}\".", state.dataZoneCtrls->StageControlledZone(ZoneAssigned).Name));
                    ErrorsFound = true;
                    continue;
                }

                if (!state.dataZoneCtrls->StagedTStatObjects(Item).ZoneListActive) {
                    stageZone.Name = s_ipsc->cAlphaArgs(1);
                } else {
                    CheckCreatedZoneItemName(
                        state,
                        RoutineName,
                        s_ipsc->cCurrentModuleObject,
                        state.dataHeatBal->Zone(ZoneList(state.dataZoneCtrls->StagedTStatObjects(Item).ZoneOrZoneListPtr).Zone(Item1)).Name,
                        ZoneList(state.dataZoneCtrls->StagedTStatObjects(Item).ZoneOrZoneListPtr).MaxZoneNameLength,
                        state.dataZoneCtrls->StagedTStatObjects(Item).Name,
                        state.dataZoneCtrls->StageControlledZone,
                        StageControlledZoneNum - 1,
                        stageZone.Name,
                        errFlag);
                    if (errFlag) {
                        ErrorsFound = true;
                    }
                }

                stageZone.NumOfHeatStages = s_ipsc->rNumericArgs(1);
                if (s_ipsc->rNumericArgs(1) < 1 || s_ipsc->rNumericArgs(1) > 4) {
                    ShowSevereBadMinMax(state, eoh, s_ipsc->cNumericFieldNames(1), s_ipsc->rNumericArgs(1), Clusive::In, 1, Clusive::In, 4);
                    ErrorsFound = true;
                }

                if (s_ipsc->lAlphaFieldBlanks(3)) {
                    if (Item1 == 1) {
                        ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(3));
                        ErrorsFound = true;
                    }
                } else if ((stageZone.heatSetptBaseSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(3))) == nullptr) {
                    if (Item1 == 1) {
                        ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(3), s_ipsc->cAlphaArgs(3));
                        ErrorsFound = true;
                    }
                }

                stageZone.HeatThroRange = s_ipsc->rNumericArgs(2);
                if (s_ipsc->rNumericArgs(1) < 0.0) {
                    ShowSevereBadMin(state, eoh, s_ipsc->cNumericFieldNames(2), s_ipsc->rNumericArgs(2), Clusive::In, 0.0);
                    ErrorsFound = true;
                }

                if (stageZone.NumOfHeatStages > 0) {
                    stageZone.HeatTOffset.allocate(stageZone.NumOfHeatStages);
                    for (i = 1; i <= stageZone.NumOfHeatStages; ++i) {
                        stageZone.HeatTOffset(i) = s_ipsc->rNumericArgs(2 + i);
                        if (s_ipsc->lNumericFieldBlanks(2 + i)) {
                            ShowSevereEmptyField(state, eoh, s_ipsc->cNumericFieldNames(2 + i));
                            ErrorsFound = true;
                        } else if (s_ipsc->rNumericArgs(2 + i) > 0.0) {
                            ShowSevereBadMax(state, eoh, s_ipsc->cNumericFieldNames(2 + i), s_ipsc->rNumericArgs(2 + i), Clusive::In, 0.0);
                            ErrorsFound = true;
                        }

                        if (i > 1) {
                            if (s_ipsc->rNumericArgs(2 + i) >= s_ipsc->rNumericArgs(1 + i)) {
                                ShowSevereCustom(state,
                                                 eoh,
                                                 format("{} = {:.1R} must be less than than  {}={:.1R}",
                                                        s_ipsc->cNumericFieldNames(2 + i),
                                                        s_ipsc->rNumericArgs(2 + i),
                                                        s_ipsc->cNumericFieldNames(1 + i),
                                                        s_ipsc->rNumericArgs(1 + i)));
                                ErrorsFound = true;
                            }
                        }
                    }
                }

                stageZone.NumOfCoolStages = s_ipsc->rNumericArgs(7);
                if (s_ipsc->rNumericArgs(7) < 1 || s_ipsc->rNumericArgs(7) > 4) {
                    ShowSevereBadMinMax(state, eoh, s_ipsc->cNumericFieldNames(7), s_ipsc->rNumericArgs(7), Clusive::In, 1, Clusive::In, 4);
                    ErrorsFound = true;
                }

                if (s_ipsc->lAlphaFieldBlanks(4)) {
                    if (Item1 == 1) {
                        ShowSevereEmptyField(state, eoh, s_ipsc->cAlphaFieldNames(4));
                        ErrorsFound = true;
                    }
                } else if ((stageZone.coolSetptBaseSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(4))) == nullptr) {
                    if (Item1 == 1) { // only show error on first of several if zone list
                        ShowSevereItemNotFound(state, eoh, s_ipsc->cAlphaFieldNames(4), s_ipsc->cAlphaArgs(4));
                        ErrorsFound = true;
                    }
                }

                stageZone.CoolThroRange = s_ipsc->rNumericArgs(8);
                if (s_ipsc->rNumericArgs(8) < 0.0) {
                    ShowSevereBadMin(state, eoh, s_ipsc->cNumericFieldNames(8), s_ipsc->rNumericArgs(8), Clusive::In, 0.0);
                    ErrorsFound = true;
                }

                if (stageZone.NumOfCoolStages > 0) {
                    stageZone.CoolTOffset.allocate(stageZone.NumOfCoolStages);
                    for (i = 1; i <= stageZone.NumOfCoolStages; ++i) {
                        stageZone.CoolTOffset(i) = s_ipsc->rNumericArgs(8 + i);
                        if (s_ipsc->lNumericFieldBlanks(8 + i)) {
                            ShowSevereEmptyField(state, eoh, s_ipsc->cNumericFieldNames(8 + i));
                            ErrorsFound = true;
                        } else if (s_ipsc->rNumericArgs(8 + i) < 0.0) {
                            ShowSevereBadMin(state, eoh, s_ipsc->cNumericFieldNames(8 + i), s_ipsc->rNumericArgs(8 + i), Clusive::In, 0.0);
                            ErrorsFound = true;
                        }

                        if (i > 1 && s_ipsc->rNumericArgs(8 + i) <= s_ipsc->rNumericArgs(7 + i)) {
                            ShowSevereCustom(state,
                                             eoh,
                                             format("{} = {:.1R} must be greater than {} = {:.1R}",
                                                    s_ipsc->cNumericFieldNames(8 + i),
                                                    s_ipsc->rNumericArgs(8 + i),
                                                    s_ipsc->cNumericFieldNames(7 + i),
                                                    s_ipsc->rNumericArgs(7 + i)));
                            ErrorsFound = true;
                        }
                    }
                }
            }
        } // loop over NumStageControlledZones

        if ((s_ip->getNumObjectsFound(state, "AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed") == 0) &&
            (s_ip->getNumObjectsFound(state, "AirLoopHVAC:UnitarySystem") == 0) &&
            (s_ip->getNumObjectsFound(state, "SetpointManager:SingleZone:OneStageCooling") == 0) &&
            (s_ip->getNumObjectsFound(state, "SetpointManager:SingleZone:OneStageHeating") == 0)) {
            ShowWarningError(state,
                             format("{} is applicable to only selected HVAC objects which are missing from input.", s_ipsc->cCurrentModuleObject));
            ShowContinueError(state, "Model should include one or more of the following objects:  ");
            ShowContinueError(state, "AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed, AirLoopHVAC:UnitarySystem, ");
            ShowContinueError(
                state, "SetpointManager:SingleZone:OneStageCooling, and/or SetpointManager:SingleZone:OneStageHeating. The simulation continues...");
        }
    } // NumStageControlledZones > 0

    if (ErrorsFound) {
        ShowFatalError(state, "Errors getting Zone Control input data.  Preceding condition(s) cause termination.");
    }
}

void CalculateMonthlyRunningAverageDryBulb(EnergyPlusData &state, Array1D<Real64> &runningAverageASH, Array1D<Real64> &runningAverageCEN)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Xuan Luo
    //       DATE WRITTEN   January 2017
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine calculate the monthly running average dry bulb temperature;

    // Using/Aliasing

    using OutputReportTabular::GetColumnUsingTabs;
    using OutputReportTabular::StrToReal;

    // SUBROUTINE PARAMETER DEFINITIONS:

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 dryBulb;
    Real64 avgDryBulb;

    std::string::size_type pos;

    Array1D<Real64> adaptiveTemp(state.dataWeather->NumDaysInYear, 0.0);
    Array1D<Real64> dailyDryTemp(state.dataWeather->NumDaysInYear, 0.0);

    if (FileSystem::fileExists(state.files.inputWeatherFilePath.filePath)) {
        // Read hourly dry bulb temperature first
        auto epwFile = state.files.inputWeatherFilePath.open(state, "CalcThermalComfortAdaptive");
        for (int i = 1; i <= 9; ++i) { // Headers
            epwFile.readLine();
        }
        for (int i = 1; i <= state.dataWeather->NumDaysInYear; ++i) {
            avgDryBulb = 0.0;
            for (int j = 1; j <= 24; ++j) {
                std::string epwLine = epwFile.readLine().data;
                for (int ind = 1; ind <= 6; ++ind) {
                    pos = index(epwLine, ',');
                    epwLine.erase(0, pos + 1);
                }
                pos = index(epwLine, ',');
                dryBulb = StrToReal(epwLine.substr(0, pos));
                avgDryBulb += (dryBulb / 24.0);
            }
            dailyDryTemp(i) = avgDryBulb;
        }
        epwFile.close();

        // Calculate monthly running average dry bulb temperature.
        int dayOfYear = 0;
        while (dayOfYear < state.dataWeather->NumDaysInYear) {
            dayOfYear++;
            int calcEndDay = dayOfYear - 1;
            int calcStartDayASH = calcEndDay - 30;
            int calcStartDayCEN = calcEndDay - 7;

            if (calcStartDayASH > 0) {
                for (int i = calcStartDayASH; i <= calcStartDayASH + 30; i++) {
                    avgDryBulb = dailyDryTemp(i);
                    runningAverageASH(dayOfYear) = runningAverageASH(dayOfYear) + avgDryBulb;
                }
                runningAverageASH(dayOfYear) /= 30;
            } else { // Do special things for wrapping the epw
                calcStartDayASH += state.dataWeather->NumDaysInYear;
                for (int i = 1; i <= calcEndDay; i++) {
                    avgDryBulb = dailyDryTemp(i);
                    runningAverageASH(dayOfYear) = runningAverageASH(dayOfYear) + avgDryBulb;
                }
                for (int i = calcStartDayASH; i < state.dataWeather->NumDaysInYear; i++) {
                    avgDryBulb = dailyDryTemp(i);
                    runningAverageASH(dayOfYear) = runningAverageASH(dayOfYear) + avgDryBulb;
                }
                runningAverageASH(dayOfYear) /= 30;
            }

            if (calcStartDayCEN > 0) {
                for (int i = calcStartDayCEN; i <= calcStartDayCEN + 7; i++) {
                    avgDryBulb = dailyDryTemp(i);
                    runningAverageCEN(dayOfYear) = runningAverageCEN(dayOfYear) + avgDryBulb;
                }
                runningAverageCEN(dayOfYear) /= 7;
            } else { // Do special things for wrapping the epw
                calcStartDayCEN += state.dataWeather->NumDaysInYear;
                for (int i = 1; i <= calcEndDay; i++) {
                    avgDryBulb = dailyDryTemp(i);
                    runningAverageCEN(dayOfYear) = runningAverageCEN(dayOfYear) + avgDryBulb;
                }
                for (int i = calcStartDayCEN; i < state.dataWeather->NumDaysInYear; i++) {
                    avgDryBulb = dailyDryTemp(i);
                    runningAverageCEN(dayOfYear) = runningAverageCEN(dayOfYear) + avgDryBulb;
                }
                runningAverageCEN(dayOfYear) /= 7;
            }
        }
    } else {
        ShowFatalError(state,
                       format("CalcThermalComfortAdaptive: Could not open file {} for input (read). (File does not exist)",
                              state.files.inputWeatherFilePath.filePath));
    }
}

void CalculateAdaptiveComfortSetPointSchl(EnergyPlusData &state, Array1D<Real64> const &runningAverageASH, Array1D<Real64> const &runningAverageCEN)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Xuan Luo
    //       DATE WRITTEN   January 2017
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine calculates the zone operative temperature setpoint using adaptive comfort model.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int constexpr summerDesignDayTypeIndex(9);
    Real64 GrossApproxAvgDryBulbDesignDay(0.0);

    auto &s_ztpc = state.dataZoneTempPredictorCorrector;

    for (size_t i = 1; i <= state.dataWeather->DesDayInput.size(); i++) {
        // Summer design day
        if (state.dataWeather->DesDayInput(i).DayType == summerDesignDayTypeIndex) {
            GrossApproxAvgDryBulbDesignDay = (state.dataWeather->DesDayInput(i).MaxDryBulb +
                                              (state.dataWeather->DesDayInput(i).MaxDryBulb - state.dataWeather->DesDayInput(i).DailyDBRange)) /
                                             2.0;
            if (GrossApproxAvgDryBulbDesignDay > 10 && GrossApproxAvgDryBulbDesignDay < 33.5) {
                s_ztpc->AdapComfortSetPointSummerDesDay[0] = 0.31 * GrossApproxAvgDryBulbDesignDay + 17.8;
                s_ztpc->AdapComfortSetPointSummerDesDay[1] = 0.31 * GrossApproxAvgDryBulbDesignDay + 20.3;
                s_ztpc->AdapComfortSetPointSummerDesDay[2] = 0.31 * GrossApproxAvgDryBulbDesignDay + 21.3;
            }
            if (GrossApproxAvgDryBulbDesignDay > 10 && GrossApproxAvgDryBulbDesignDay < 30) {
                s_ztpc->AdapComfortSetPointSummerDesDay[3] = 0.33 * GrossApproxAvgDryBulbDesignDay + 18.8;
                s_ztpc->AdapComfortSetPointSummerDesDay[4] = 0.33 * GrossApproxAvgDryBulbDesignDay + 20.8;
                ; // What is this?
                s_ztpc->AdapComfortSetPointSummerDesDay[5] = 0.33 * GrossApproxAvgDryBulbDesignDay + 21.8;
                ;
                s_ztpc->AdapComfortSetPointSummerDesDay[6] = 0.33 * GrossApproxAvgDryBulbDesignDay + 22.8;
                ;
            }
        }
    }

    s_ztpc->AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveASH55_Central.allocate(state.dataWeather->NumDaysInYear);
    s_ztpc->AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveASH55_Upper_90.allocate(state.dataWeather->NumDaysInYear);
    s_ztpc->AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveASH55_Upper_80.allocate(state.dataWeather->NumDaysInYear);
    s_ztpc->AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveCEN15251_Central.allocate(state.dataWeather->NumDaysInYear);
    s_ztpc->AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveCEN15251_Upper_I.allocate(state.dataWeather->NumDaysInYear);
    s_ztpc->AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveCEN15251_Upper_II.allocate(state.dataWeather->NumDaysInYear);
    s_ztpc->AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveCEN15251_Upper_III.allocate(state.dataWeather->NumDaysInYear);

    // Calculate the set points based on different models, set flag as -1 when running average temperature is not in the range.
    for (int day = 1; day <= state.dataWeather->NumDaysInYear; day++) {
        if (runningAverageASH(day) > 10 && runningAverageASH(day) < 33.5) {
            s_ztpc->AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveASH55_Central(day) = 0.31 * runningAverageASH(day) + 17.8;
            s_ztpc->AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveASH55_Upper_90(day) = 0.31 * runningAverageASH(day) + 20.3;
            s_ztpc->AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveASH55_Upper_80(day) = 0.31 * runningAverageASH(day) + 21.3;
        } else {
            s_ztpc->AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveASH55_Central(day) = -1;
            s_ztpc->AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveASH55_Upper_90(day) = -1;
            s_ztpc->AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveASH55_Upper_80(day) = -1;
        }
        if (runningAverageCEN(day) > 10 && runningAverageCEN(day) < 30) {
            s_ztpc->AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveCEN15251_Central(day) = 0.33 * runningAverageCEN(day) + 18.8;
            s_ztpc->AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveCEN15251_Upper_I(day) = 0.33 * runningAverageCEN(day) + 20.8;
            s_ztpc->AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveCEN15251_Upper_II(day) = 0.33 * runningAverageCEN(day) + 21.8;
            s_ztpc->AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveCEN15251_Upper_III(day) = 0.33 * runningAverageCEN(day) + 22.8;
        } else {
            s_ztpc->AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveCEN15251_Central(day) = -1;
            s_ztpc->AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveCEN15251_Upper_I(day) = -1;
            s_ztpc->AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveCEN15251_Upper_II(day) = -1;
            s_ztpc->AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveCEN15251_Upper_III(day) = -1;
        }
    }
    s_ztpc->AdapComfortDailySetPointSchedule.initialized = true;
}

void InitZoneAirSetPoints(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Russell Taylor
    //       DATE WRITTEN   September 1998
    //       MODIFIED       November 2004, M. J. Witte additional report variables
    //       MODIFIED       L.Gu, May 2006
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine initializes the data for the zone air setpoints.

    // METHODOLOGY EMPLOYED:
    // Uses the status flags to trigger events.

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("InitZoneAirSetpoints: ");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool FirstSurfFlag;
    int TRefFlag; // Flag for Reference Temperature process in Zones

    auto &s_ztpc = state.dataZoneTempPredictorCorrector;
    auto &s_hbfs = state.dataHeatBalFanSys;

    auto &ZoneList = state.dataHeatBal->ZoneList;
    auto &TempControlType = state.dataHeatBalFanSys->TempControlType;
    auto &TempControlTypeRpt = state.dataHeatBalFanSys->TempControlTypeRpt;
    int NumOfZones = state.dataGlobal->NumOfZones;

    if (s_ztpc->InitZoneAirSetPointsOneTimeFlag) {
        s_hbfs->zoneTstatSetpts.allocate(NumOfZones);

        state.dataHeatBalFanSys->LoadCorrectionFactor.dimension(NumOfZones, 0.0);
        TempControlType.dimension(NumOfZones, HVAC::SetptType::Uncontrolled);
        TempControlTypeRpt.dimension(NumOfZones, 0);
        if (state.dataZoneCtrls->NumComfortControlledZones > 0) {
            state.dataHeatBalFanSys->ComfortControlType.dimension(NumOfZones, HVAC::SetptType::Uncontrolled);
            state.dataHeatBalFanSys->ComfortControlTypeRpt.dimension(NumOfZones, 0);
            state.dataHeatBalFanSys->ZoneComfortControlsFanger.allocate(NumOfZones);
        }
        state.dataZoneEnergyDemand->Setback.dimension(NumOfZones, false);
        state.dataZoneEnergyDemand->DeadBandOrSetback.dimension(NumOfZones, false);
        state.dataZoneEnergyDemand->CurDeadBandOrSetback.dimension(NumOfZones, false);

        state.dataHeatBal->ZoneListSNLoadHeatEnergy.dimension(state.dataHeatBal->NumOfZoneLists, 0.0);
        state.dataHeatBal->ZoneListSNLoadCoolEnergy.dimension(state.dataHeatBal->NumOfZoneLists, 0.0);
        state.dataHeatBal->ZoneListSNLoadHeatRate.dimension(state.dataHeatBal->NumOfZoneLists, 0.0);
        state.dataHeatBal->ZoneListSNLoadCoolRate.dimension(state.dataHeatBal->NumOfZoneLists, 0.0);

        state.dataHeatBal->ZoneGroupSNLoadHeatEnergy.dimension(state.dataHeatBal->NumOfZoneGroups, 0.0);
        state.dataHeatBal->ZoneGroupSNLoadCoolEnergy.dimension(state.dataHeatBal->NumOfZoneGroups, 0.0);
        state.dataHeatBal->ZoneGroupSNLoadHeatRate.dimension(state.dataHeatBal->NumOfZoneGroups, 0.0);
        state.dataHeatBal->ZoneGroupSNLoadCoolRate.dimension(state.dataHeatBal->NumOfZoneGroups, 0.0);

        // Hybrid modeling
        state.dataHeatBalFanSys->PreviousMeasuredZT1.dimension(NumOfZones, 0.0);
        state.dataHeatBalFanSys->PreviousMeasuredZT2.dimension(NumOfZones, 0.0);
        state.dataHeatBalFanSys->PreviousMeasuredZT3.dimension(NumOfZones, 0.0);
        state.dataHeatBalFanSys->PreviousMeasuredHumRat1.dimension(NumOfZones, 0.0);
        state.dataHeatBalFanSys->PreviousMeasuredHumRat2.dimension(NumOfZones, 0.0);
        state.dataHeatBalFanSys->PreviousMeasuredHumRat3.dimension(NumOfZones, 0.0);

        // Allocate Derived Types
        state.dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(NumOfZones);
        state.dataZoneEnergyDemand->ZoneSysMoistureDemand.allocate(NumOfZones);
        if (state.dataHeatBal->doSpaceHeatBalanceSimulation || state.dataHeatBal->doSpaceHeatBalanceSizing) {
            state.dataZoneEnergyDemand->spaceSysEnergyDemand.allocate(state.dataGlobal->numSpaces);
            state.dataZoneEnergyDemand->spaceSysMoistureDemand.allocate(state.dataGlobal->numSpaces);
        }

        for (int zoneNum = 1; zoneNum <= NumOfZones; ++zoneNum) {
            bool FirstSurfFlag = true;
            for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                for (int SurfNum = thisSpace.HTSurfaceFirst; SurfNum <= thisSpace.HTSurfaceLast; ++SurfNum) {
                    if (FirstSurfFlag) {
                        TRefFlag = state.dataSurface->SurfTAirRef(SurfNum);
                        FirstSurfFlag = false;
                    }
                    // for each particular zone, the reference air temperature(s) should be the same
                    // (either mean air, bulk air, or supply air temp).
                    if (state.dataSurface->SurfTAirRef(SurfNum) != TRefFlag) {
                        ShowWarningError(state,
                                         format("Different reference air temperatures for difference surfaces encountered in zone {}",
                                                state.dataHeatBal->Zone(zoneNum).Name));
                    }
                }
            }
        }

        // CurrentModuleObject='Zone'
        for (int zoneNum = 1; zoneNum <= NumOfZones; ++zoneNum) {
            auto &thisZone = state.dataHeatBal->Zone(zoneNum);
            s_ztpc->zoneHeatBalance(zoneNum).setUpOutputVars(state, DataStringGlobals::zonePrefix, thisZone.Name);
            if (state.dataHeatBal->doSpaceHeatBalanceSimulation) {
                for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                    s_ztpc->spaceHeatBalance(spaceNum).setUpOutputVars(
                        state, DataStringGlobals::spacePrefix, state.dataHeatBal->space(spaceNum).Name);
                }
            }
            bool staged = false;
            if (allocated(state.dataZoneCtrls->StageZoneLogic)) {
                staged = state.dataZoneCtrls->StageZoneLogic(zoneNum);
            }
            // If not doSpaceHeatBalanceSimulation then meter zones, not spaces
            bool attachMeters = !state.dataHeatBal->doSpaceHeatBalanceSimulation;
            state.dataZoneEnergyDemand->ZoneSysEnergyDemand(zoneNum).setUpOutputVars(
                state, DataStringGlobals::zonePrefix, thisZone.Name, staged, attachMeters, thisZone.Multiplier, thisZone.ListMultiplier);
            if (state.dataHeatBal->doSpaceHeatBalanceSimulation) {
                // If doSpaceHeatBalanceSimulation then meter spaces, not zones
                attachMeters = state.dataHeatBal->doSpaceHeatBalanceSimulation;
                for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                    state.dataZoneEnergyDemand->spaceSysEnergyDemand(spaceNum).setUpOutputVars(state,
                                                                                               DataStringGlobals::spacePrefix,
                                                                                               state.dataHeatBal->space(spaceNum).Name,
                                                                                               staged,
                                                                                               attachMeters,
                                                                                               thisZone.Multiplier,
                                                                                               thisZone.ListMultiplier);
                }
            }
            state.dataZoneEnergyDemand->ZoneSysMoistureDemand(zoneNum).setUpOutputVars(state, DataStringGlobals::zonePrefix, thisZone.Name);
            if (state.dataHeatBal->doSpaceHeatBalanceSimulation) {
                for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                    state.dataZoneEnergyDemand->spaceSysMoistureDemand(spaceNum).setUpOutputVars(
                        state, DataStringGlobals::spacePrefix, state.dataHeatBal->space(spaceNum).Name);
                }
            }
            SetupOutputVariable(state,
                                "Zone Thermostat Air Temperature",
                                Constant::Units::C,
                                state.dataHeatBalFanSys->TempTstatAir(zoneNum),
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisZone.Name);
            SetupOutputVariable(state,
                                "Zone Thermostat Control Type",
                                Constant::Units::None,
                                state.dataHeatBalFanSys->TempControlTypeRpt(zoneNum),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                thisZone.Name);
            SetupOutputVariable(state,
                                "Zone Thermostat Heating Setpoint Temperature",
                                Constant::Units::C,
                                s_hbfs->zoneTstatSetpts(zoneNum).setptLo,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisZone.Name);
            SetupOutputVariable(state,
                                "Zone Thermostat Cooling Setpoint Temperature",
                                Constant::Units::C,
                                s_hbfs->zoneTstatSetpts(zoneNum).setptHi,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisZone.Name);
            SetupOutputVariable(state,
                                "Zone Adaptive Comfort Operative Temperature Set Point",
                                Constant::Units::C,
                                s_hbfs->zoneTstatSetpts(zoneNum).setptAdapComfortCool,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                thisZone.Name);
            SetupOutputVariable(state,
                                "Zone Predicted Sensible Load Room Air Correction Factor",
                                Constant::Units::None,
                                state.dataHeatBalFanSys->LoadCorrectionFactor(zoneNum),
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisZone.Name);
        } // zoneNum

        // Thermal comfort control output
        if (state.dataZoneCtrls->NumComfortControlledZones > 0) {
            // CurrentModuleObject='ZoneControl:Thermostat:ThermalComfort'
            for (int Loop = 1; Loop <= state.dataZoneCtrls->NumComfortControlledZones; ++Loop) {
                int zoneNum = state.dataZoneCtrls->ComfortControlledZone(Loop).ActualZoneNum;
                auto &thisZone = state.dataHeatBal->Zone(zoneNum);
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort Control Type",
                                    Constant::Units::None,
                                    state.dataHeatBalFanSys->ComfortControlTypeRpt(zoneNum),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    thisZone.Name);
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort Control Fanger Low Setpoint PMV",
                                    Constant::Units::None,
                                    state.dataHeatBalFanSys->ZoneComfortControlsFanger(zoneNum).LowPMV,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    thisZone.Name);
                SetupOutputVariable(state,
                                    "Zone Thermal Comfort Control Fanger High Setpoint PMV",
                                    Constant::Units::None,
                                    state.dataHeatBalFanSys->ZoneComfortControlsFanger(zoneNum).HighPMV,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    thisZone.Name);
            }
        }

        // CurrentModuleObject='ZoneList'
        for (int Loop = 1; Loop <= state.dataHeatBal->NumOfZoneLists; ++Loop) {
            auto &zoneList = state.dataHeatBal->ZoneList(Loop);
            SetupOutputVariable(state,
                                "Zone List Sensible Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneListSNLoadHeatEnergy(Loop),
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                zoneList.Name);
            SetupOutputVariable(state,
                                "Zone List Sensible Cooling Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneListSNLoadCoolEnergy(Loop),
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                zoneList.Name);
            SetupOutputVariable(state,
                                "Zone List Sensible Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneListSNLoadHeatRate(Loop),
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                zoneList.Name);
            SetupOutputVariable(state,
                                "Zone List Sensible Cooling Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneListSNLoadCoolRate(Loop),
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                zoneList.Name);
        } // Loop

        // CurrentModuleObject='ZoneGroup'
        for (int Loop = 1; Loop <= state.dataHeatBal->NumOfZoneGroups; ++Loop) {
            auto &zoneGroup = state.dataHeatBal->ZoneGroup(Loop);
            SetupOutputVariable(state,
                                "Zone Group Sensible Heating Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneGroupSNLoadHeatEnergy(Loop),
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                zoneGroup.Name);
            SetupOutputVariable(state,
                                "Zone Group Sensible Cooling Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZoneGroupSNLoadCoolEnergy(Loop),
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                zoneGroup.Name);
            SetupOutputVariable(state,
                                "Zone Group Sensible Heating Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneGroupSNLoadHeatRate(Loop),
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                zoneGroup.Name);
            SetupOutputVariable(state,
                                "Zone Group Sensible Cooling Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneGroupSNLoadCoolRate(Loop),
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                zoneGroup.Name);
        } // Loop

        s_ztpc->InitZoneAirSetPointsOneTimeFlag = false;
    }

    // Do the Begin Environment initializations
    if (s_ztpc->MyEnvrnFlag && state.dataGlobal->BeginEnvrnFlag) {
        for (auto &thisZoneHB : s_ztpc->zoneHeatBalance) {
            thisZoneHB.beginEnvironmentInit(state);
        }
        if (state.dataHeatBal->doSpaceHeatBalance) {
            for (auto &thisSpaceHB : s_ztpc->spaceHeatBalance) {
                thisSpaceHB.beginEnvironmentInit(state);
            }
        }

        for (auto &zoneTstatSetpt : s_hbfs->zoneTstatSetpts) {
            zoneTstatSetpt.setpt = zoneTstatSetpt.setptAdapComfortCool = zoneTstatSetpt.setptLo = zoneTstatSetpt.setptHi = 0.0;
        }

        state.dataHeatBalFanSys->LoadCorrectionFactor = 1.0;
        TempControlType = HVAC::SetptType::Uncontrolled;
        for (auto &e : state.dataZoneEnergyDemand->ZoneSysEnergyDemand) {
            e.beginEnvironmentInit();
        }
        for (auto &e : state.dataZoneEnergyDemand->ZoneSysMoistureDemand) {
            e.beginEnvironmentInit();
        }
        if (state.dataHeatBal->doSpaceHeatBalance) {
            for (auto &e : state.dataZoneEnergyDemand->spaceSysEnergyDemand) {
                e.beginEnvironmentInit();
            }
            for (auto &e : state.dataZoneEnergyDemand->spaceSysMoistureDemand) {
                e.beginEnvironmentInit();
            }
        }

        state.dataZoneEnergyDemand->DeadBandOrSetback = false;

        for (auto &e : state.dataHeatBal->Zone) {
            e.NoHeatToReturnAir = false;
        }
        state.dataHeatBalFanSys->PreviousMeasuredZT1 = 0.0;     // Hybrid modeling
        state.dataHeatBalFanSys->PreviousMeasuredZT2 = 0.0;     // Hybrid modeling
        state.dataHeatBalFanSys->PreviousMeasuredZT3 = 0.0;     // Hybrid modeling
        state.dataHeatBalFanSys->PreviousMeasuredHumRat1 = 0.0; // Hybrid modeling
        state.dataHeatBalFanSys->PreviousMeasuredHumRat2 = 0.0; // Hybrid modeling
        state.dataHeatBalFanSys->PreviousMeasuredHumRat3 = 0.0; // Hybrid modeling

        s_ztpc->MyEnvrnFlag = false;
    }

    if (!state.dataGlobal->BeginEnvrnFlag) {
        s_ztpc->MyEnvrnFlag = true;
    }

    // Do the Begin Day initializations
    if (s_ztpc->MyDayFlag && state.dataGlobal->BeginDayFlag) {
        s_ztpc->MyDayFlag = false;
    }

    if (!state.dataGlobal->BeginDayFlag) {
        s_ztpc->MyDayFlag = true;
    }

    for (int Loop = 1; Loop <= state.dataZoneCtrls->NumTempControlledZones; ++Loop) {
        auto &tempZone = state.dataZoneCtrls->TempControlledZone(Loop);
        if (state.dataZoneEquip->ZoneEquipInputsFilled && !s_ztpc->ControlledZonesChecked) {
            if (!VerifyControlledZoneForThermostat(state, tempZone.ZoneName)) {
                ShowSevereError(
                    state,
                    format("{}Zone=\"{}\" has specified a Thermostatic control but is not a controlled zone.", RoutineName, tempZone.ZoneName));
                ShowContinueError(state, "...must have a ZoneHVAC:EquipmentConnections specification for this zone.");
                s_ztpc->ErrorsFound = true;
            }
        }

        if (tempZone.ManageDemand) {
            int ZoneNum = tempZone.ActualZoneNum;

            auto &zoneTstatSetpt = s_hbfs->zoneTstatSetpts(ZoneNum);

            switch (TempControlType(ZoneNum)) {
            case HVAC::SetptType::SingleHeat: {
                if (zoneTstatSetpt.setpt > tempZone.HeatingResetLimit) {
                    zoneTstatSetpt.setptLo = zoneTstatSetpt.setpt = tempZone.HeatingResetLimit;
                }
            } break;

            case HVAC::SetptType::SingleCool: {
                if (zoneTstatSetpt.setpt < tempZone.CoolingResetLimit) {
                    zoneTstatSetpt.setptHi = zoneTstatSetpt.setpt = tempZone.CoolingResetLimit;
                }
            } break;

            case HVAC::SetptType::SingleHeatCool: {
                if ((zoneTstatSetpt.setpt > tempZone.HeatingResetLimit) || (zoneTstatSetpt.setpt < tempZone.CoolingResetLimit)) {

                    TempControlType(ZoneNum) = HVAC::SetptType::DualHeatCool;
                    TempControlTypeRpt(ZoneNum) = static_cast<int>(TempControlType(ZoneNum));
                    zoneTstatSetpt.setptHi = zoneTstatSetpt.setptLo = zoneTstatSetpt.setpt;

                    if (zoneTstatSetpt.setptLo > tempZone.HeatingResetLimit) {
                        zoneTstatSetpt.setptLo = tempZone.HeatingResetLimit;
                    }
                    if (zoneTstatSetpt.setptHi < tempZone.CoolingResetLimit) {
                        zoneTstatSetpt.setptHi = tempZone.CoolingResetLimit;
                    }
                }
            } break;

            case HVAC::SetptType::DualHeatCool: {
                if (zoneTstatSetpt.setptLo > tempZone.HeatingResetLimit) {
                    zoneTstatSetpt.setptLo = tempZone.HeatingResetLimit;
                }
                if (zoneTstatSetpt.setptHi < tempZone.CoolingResetLimit) {
                    zoneTstatSetpt.setptHi = tempZone.CoolingResetLimit;
                }
            } break;

            default: {
            } break;
            } // switch (setptType)
        }
    }

    for (int Loop = 1; Loop <= state.dataZoneCtrls->NumComfortControlledZones; ++Loop) {
        auto const &comfortZone = state.dataZoneCtrls->ComfortControlledZone(Loop);
        if (state.dataZoneEquip->ZoneEquipInputsFilled && !s_ztpc->ControlledZonesChecked) {
            if (!VerifyControlledZoneForThermostat(state, comfortZone.ZoneName)) {
                ShowSevereError(
                    state, format("{}Zone=\"{}\" has specified a Comfort control but is not a controlled zone.", RoutineName, comfortZone.ZoneName));
                ShowContinueError(state, "...must have a ZoneHVAC:EquipmentConnections specification for this zone.");
                s_ztpc->ErrorsFound = true;
            }
        }

        if (comfortZone.ManageDemand) {
            int ZoneNum = comfortZone.ActualZoneNum;
            auto &zoneTstatSetpt = s_hbfs->zoneTstatSetpts(ZoneNum);
            switch (s_hbfs->ComfortControlType(ZoneNum)) {
            case HVAC::SetptType::SingleHeat: {
                if (zoneTstatSetpt.setpt >= comfortZone.HeatingResetLimit) {
                    zoneTstatSetpt.setptLo = zoneTstatSetpt.setpt = comfortZone.HeatingResetLimit;
                    TempControlType(ZoneNum) = HVAC::SetptType::SingleHeat;
                    TempControlTypeRpt(ZoneNum) = static_cast<int>(TempControlType(ZoneNum));
                }
            } break;

            case HVAC::SetptType::SingleCool: {
                if (zoneTstatSetpt.setpt <= comfortZone.CoolingResetLimit) {
                    zoneTstatSetpt.setptHi = zoneTstatSetpt.setpt = comfortZone.CoolingResetLimit;
                    TempControlType(ZoneNum) = HVAC::SetptType::SingleCool;
                    TempControlTypeRpt(ZoneNum) = static_cast<int>(TempControlType(ZoneNum));
                }
            } break;

            case HVAC::SetptType::SingleHeatCool: {
                if ((zoneTstatSetpt.setpt >= comfortZone.HeatingResetLimit) || (zoneTstatSetpt.setpt <= comfortZone.CoolingResetLimit)) {

                    TempControlType(ZoneNum) = HVAC::SetptType::DualHeatCool;
                    TempControlTypeRpt(ZoneNum) = static_cast<int>(TempControlType(ZoneNum));
                    zoneTstatSetpt.setptLo = zoneTstatSetpt.setptHi = zoneTstatSetpt.setpt;

                    if (zoneTstatSetpt.setptLo >= comfortZone.HeatingResetLimit) {
                        zoneTstatSetpt.setptLo = comfortZone.HeatingResetLimit;
                    }
                    if (zoneTstatSetpt.setptHi <= comfortZone.CoolingResetLimit) {
                        zoneTstatSetpt.setptHi = comfortZone.CoolingResetLimit;
                    }
                }
            } break;

            case HVAC::SetptType::DualHeatCool: {
                TempControlType(ZoneNum) = HVAC::SetptType::DualHeatCool;
                TempControlTypeRpt(ZoneNum) = static_cast<int>(TempControlType(ZoneNum));
                if (zoneTstatSetpt.setptLo >= comfortZone.HeatingResetLimit) {
                    zoneTstatSetpt.setptLo = comfortZone.HeatingResetLimit;
                }
                if (zoneTstatSetpt.setptHi <= comfortZone.CoolingResetLimit) {
                    zoneTstatSetpt.setptHi = comfortZone.CoolingResetLimit;
                }
            } break;

            default: {
            } break;
            } // switch
        } // Demand manager
    }

    if (s_ztpc->ErrorsFound) {
        ShowFatalError(state, "InitZoneAirSetpoints - program terminates due to previous condition.");
    }

    if (state.dataZoneEquip->ZoneEquipInputsFilled) {
        s_ztpc->ControlledZonesChecked = true;
    }
}

void ZoneSpaceHeatBalanceData::beginEnvironmentInit(EnergyPlusData &state)
{
    for (int i = 0; i <= 3; ++i) {
        this->ZTM[i] = 0.0;
        this->WPrevZoneTS[i] = state.dataEnvrn->OutHumRat;
        this->DSWPrevZoneTS[i] = state.dataEnvrn->OutHumRat;
        this->WPrevZoneTSTemp[i] = 0.0;
    }
    this->WTimeMinusP = state.dataEnvrn->OutHumRat;
    this->W1 = state.dataEnvrn->OutHumRat;
    this->WMX = state.dataEnvrn->OutHumRat;
    this->WM2 = state.dataEnvrn->OutHumRat;
    this->airHumRatTemp = 0.0;
    this->tempIndLoad = 0.0;
    this->tempDepLoad = 0.0;
    this->airRelHum = 0.0;
    this->AirPowerCap = 0.0;
    this->T1 = 0.0;
}

void ZoneSpaceHeatBalanceData::setUpOutputVars(EnergyPlusData &state, std::string_view prefix, std::string const &name)
{
    SetupOutputVariable(state,
                        format("{} Air Temperature", prefix),
                        Constant::Units::C,
                        this->ZT,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        name);
    SetupOutputVariable(state,
                        format("{} Air Humidity Ratio", prefix),
                        Constant::Units::None,
                        this->airHumRat,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        name);
    SetupOutputVariable(state,
                        format("{} Air Relative Humidity", prefix),
                        Constant::Units::Perc,
                        this->airRelHum,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        name);
    SetupOutputVariable(state,
                        format("{} Mean Radiant Temperature", prefix),
                        Constant::Units::C,
                        this->MRT,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Average,
                        name);
}

void PredictSystemLoads(EnergyPlusData &state,
                        bool const ShortenTimeStepSys,
                        bool const UseZoneTimeStepHistory, // if true then use zone timestep history, if false use system time step
                        Real64 const PriorTimeStep         // the old value for timestep length is passed for possible use in interpolating
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Russ Taylor
    //       DATE WRITTEN   May 1997
    //       MODIFIED       na
    //       RE-ENGINEERED  July 2003 (Peter Graham Ellis)

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is responsible for determining
    // how much of each type of energy every zone requires.
    // In effect, this subroutine defines and simulates all
    // the system types and in the case of hybrid systems
    // which use more than one type of energy must determine
    // how to apportion the load. An example of a hybrid system
    // is a water loop heat pump with supplemental air.  In
    // this case, a zone will require water from the loop and
    // cooled or heated air from the air system. A simpler
    // example would be a VAV system with baseboard heaters.

    //  Basic Air System Types
    //  1) Constant Volume Single Duct
    //  2) Variable Volume Single Duct
    //  3) Constant Volume Dual Duct
    //  4) Variable Volume Dual Duct

    // METHODOLOGY EMPLOYED:
    // 0.  Determine if simulation has downstepped and readjust history and revert node results
    // 1.  Determine zone load - this is zone temperature dependent
    // 2.  Determine balance point - the temperature at which the
    //     zone load is balanced by the system output. The way the
    //     balance point is determined will be different depending on
    //     the type of system being simulated.
    // 3.  Calculate zone energy requirements

    auto &s_ztpc = state.dataZoneTempPredictorCorrector;
    auto const &s_hbfs = state.dataHeatBalFanSys;

    // Staged thermostat setpoint
    if (s_ztpc->NumStageCtrZone > 0) {
        for (int RelativeZoneNum = 1; RelativeZoneNum <= s_ztpc->NumStageCtrZone; ++RelativeZoneNum) {
            auto &thisStageControlZone = state.dataZoneCtrls->StageControlledZone(RelativeZoneNum);
            int ActualZoneNum = thisStageControlZone.ActualZoneNum;

            auto &thisZoneHB = s_ztpc->zoneHeatBalance(ActualZoneNum);
            auto &zoneTstatSetpt = s_hbfs->zoneTstatSetpts(ActualZoneNum);
            Real64 ZoneT = thisZoneHB.MAT; // Zone temperature at previous time step
            if (ShortenTimeStepSys) {
                ZoneT = thisZoneHB.XMPT;
            }
            thisStageControlZone.HeatSetPoint = thisStageControlZone.heatSetptBaseSched->getCurrentVal();
            thisStageControlZone.CoolSetPoint = thisStageControlZone.coolSetptBaseSched->getCurrentVal();

            if (thisStageControlZone.HeatSetPoint >= thisStageControlZone.CoolSetPoint) {
                ++thisStageControlZone.StageErrCount;
                if (thisStageControlZone.StageErrCount < 2) {
                    ShowWarningError(
                        state,
                        format("ZoneControl:Thermostat:StagedDualSetpoint: The heating setpoint is equal to or above the cooling setpoint in {}",
                               thisStageControlZone.Name));
                    ShowContinueError(state, "The zone heating setpoint is set to the cooling setpoint - 0.1C.");
                    ShowContinueErrorTimeStamp(state, "Occurrence info:");
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   "The heating setpoint is still above the cooling setpoint",
                                                   thisStageControlZone.StageErrIndex,
                                                   thisStageControlZone.HeatSetPoint,
                                                   thisStageControlZone.HeatSetPoint);
                }
                thisStageControlZone.HeatSetPoint = thisStageControlZone.CoolSetPoint - 0.1; //???????????
            }
            // Determine either cooling or heating
            if (thisStageControlZone.CoolSetPoint < ZoneT) { // Cooling
                Real64 SetpointOffset = ZoneT - thisStageControlZone.CoolSetPoint;
                int Itemp = 0;
                for (int I = 1; I <= thisStageControlZone.NumOfCoolStages; ++I) {
                    if (SetpointOffset >= thisStageControlZone.CoolTOffset(I)) {
                        Itemp = -I;
                    }
                }
                state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ActualZoneNum).StageNum = Itemp;
                if (SetpointOffset >= 0.5 * thisStageControlZone.CoolThroRange) {
                    zoneTstatSetpt.setptHi = thisStageControlZone.CoolSetPoint - 0.5 * thisStageControlZone.CoolThroRange;
                } else {
                    zoneTstatSetpt.setptHi = thisStageControlZone.CoolSetPoint + 0.5 * thisStageControlZone.CoolThroRange;
                }
                zoneTstatSetpt.setptLo = zoneTstatSetpt.setptHi;

            } else if (thisStageControlZone.HeatSetPoint > ZoneT) { // heating
                Real64 SetpointOffset = ZoneT - thisStageControlZone.HeatSetPoint;
                int Itemp = 0;
                for (int I = 1; I <= thisStageControlZone.NumOfHeatStages; ++I) {
                    if (std::abs(SetpointOffset) >= std::abs(thisStageControlZone.HeatTOffset(I))) {
                        Itemp = I;
                    }
                }
                state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ActualZoneNum).StageNum = Itemp;
                if (std::abs(SetpointOffset) >= 0.5 * thisStageControlZone.CoolThroRange) {
                    zoneTstatSetpt.setptLo = thisStageControlZone.HeatSetPoint + 0.5 * thisStageControlZone.HeatThroRange;
                } else {
                    zoneTstatSetpt.setptLo = thisStageControlZone.HeatSetPoint - 0.5 * thisStageControlZone.HeatThroRange;
                }
                zoneTstatSetpt.setptHi = zoneTstatSetpt.setptLo;

            } else {
                zoneTstatSetpt.setptHi = thisStageControlZone.CoolSetPoint + 0.5 * thisStageControlZone.CoolThroRange;
                zoneTstatSetpt.setptLo = thisStageControlZone.HeatSetPoint - 0.5 * thisStageControlZone.HeatThroRange;
                state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ActualZoneNum).StageNum = 0;
            }
            // SpaceHB TODO: For now, set space stagenum to zone stagenum - later need to see what space the thermostat is in
            if (state.dataHeatBal->doSpaceHeatBalance) {
                for (int spaceNum : state.dataHeatBal->Zone(ActualZoneNum).spaceIndexes) {
                    state.dataZoneEnergyDemand->spaceSysEnergyDemand(spaceNum).StageNum =
                        state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ActualZoneNum).StageNum;
                }
            }
        }
    }

    // Setpoint revision for onoff thermostat
    if (s_ztpc->NumOnOffCtrZone > 0) {
        Real64 TempTole = 0.02;
        Real64 Tprev;
        for (int RelativeZoneNum = 1; RelativeZoneNum <= state.dataZoneCtrls->NumTempControlledZones; ++RelativeZoneNum) {
            auto &thisTempControlledZone = state.dataZoneCtrls->TempControlledZone(RelativeZoneNum);
            if (thisTempControlledZone.DeltaTCutSet > 0.0) {
                if (ShortenTimeStepSys) {
                    thisTempControlledZone.HeatModeLast = thisTempControlledZone.HeatModeLastSave;
                    thisTempControlledZone.CoolModeLast = thisTempControlledZone.CoolModeLastSave;
                } else {
                    thisTempControlledZone.HeatModeLastSave = thisTempControlledZone.HeatModeLast;
                    thisTempControlledZone.CoolModeLastSave = thisTempControlledZone.CoolModeLast;
                }

                auto &zoneTstatSetpt = s_hbfs->zoneTstatSetpts(thisTempControlledZone.ActualZoneNum);
                auto &thisZoneHB = s_ztpc->zoneHeatBalance(thisTempControlledZone.ActualZoneNum);
                thisTempControlledZone.CoolOffFlag = false;
                thisTempControlledZone.HeatOffFlag = false;
                if (state.dataHeatBal->ZoneAirSolutionAlgo == DataHeatBalance::SolutionAlgo::ThirdOrder) {
                    Tprev = thisZoneHB.MAT;
                    if (ShortenTimeStepSys) {
                        Tprev = thisZoneHB.XMPT;
                    }
                } else {
                    Tprev = thisZoneHB.T1;
                }

                switch (state.dataHeatBalFanSys->TempControlType(thisTempControlledZone.ActualZoneNum)) {

                case HVAC::SetptType::SingleHeat: {
                    zoneTstatSetpt.setpt = thisTempControlledZone.ZoneThermostatSetPointLo;
                    zoneTstatSetpt.setptLo = thisTempControlledZone.ZoneThermostatSetPointLo;
                    if (Tprev < thisTempControlledZone.ZoneThermostatSetPointLo + TempTole) {
                        zoneTstatSetpt.setpt = thisTempControlledZone.ZoneThermostatSetPointLo + thisTempControlledZone.DeltaTCutSet;
                        zoneTstatSetpt.setptLo = zoneTstatSetpt.setpt;
                    } else if (Tprev > thisTempControlledZone.ZoneThermostatSetPointLo &&
                               (Tprev < thisTempControlledZone.ZoneThermostatSetPointLo + thisTempControlledZone.DeltaTCutSet - TempTole)) {
                        zoneTstatSetpt.setpt = thisTempControlledZone.ZoneThermostatSetPointLo + thisTempControlledZone.DeltaTCutSet;
                        zoneTstatSetpt.setptLo = zoneTstatSetpt.setpt;
                    } else {
                        thisTempControlledZone.HeatOffFlag = true;
                    }
                    if (thisTempControlledZone.HeatModeLast && Tprev > thisTempControlledZone.ZoneThermostatSetPointLo) {
                        zoneTstatSetpt.setpt = thisTempControlledZone.ZoneThermostatSetPointLo;
                        zoneTstatSetpt.setptLo = thisTempControlledZone.ZoneThermostatSetPointLo;
                        thisTempControlledZone.HeatOffFlag = true;
                    }
                } break;

                case HVAC::SetptType::SingleCool: {
                    zoneTstatSetpt.setpt = thisTempControlledZone.ZoneThermostatSetPointHi;
                    zoneTstatSetpt.setptHi = thisTempControlledZone.ZoneThermostatSetPointHi;
                    if (Tprev > thisTempControlledZone.ZoneThermostatSetPointHi - TempTole) {
                        zoneTstatSetpt.setpt = thisTempControlledZone.ZoneThermostatSetPointHi - thisTempControlledZone.DeltaTCutSet;
                        zoneTstatSetpt.setptHi = zoneTstatSetpt.setpt;
                    } else if (Tprev < thisTempControlledZone.ZoneThermostatSetPointHi &&
                               Tprev > thisTempControlledZone.ZoneThermostatSetPointHi - thisTempControlledZone.DeltaTCutSet + TempTole) {
                        zoneTstatSetpt.setpt = thisTempControlledZone.ZoneThermostatSetPointHi - thisTempControlledZone.DeltaTCutSet;
                        zoneTstatSetpt.setptHi = zoneTstatSetpt.setpt;
                    } else {
                        thisTempControlledZone.CoolOffFlag = true;
                    }
                    if (thisTempControlledZone.CoolModeLast && Tprev < thisTempControlledZone.ZoneThermostatSetPointHi) {
                        zoneTstatSetpt.setpt = thisTempControlledZone.ZoneThermostatSetPointHi;
                        zoneTstatSetpt.setptHi = thisTempControlledZone.ZoneThermostatSetPointHi;
                        thisTempControlledZone.CoolOffFlag = true;
                    }
                } break;

                case HVAC::SetptType::DualHeatCool: {
                    zoneTstatSetpt.setptHi = thisTempControlledZone.ZoneThermostatSetPointHi;
                    zoneTstatSetpt.setptLo = thisTempControlledZone.ZoneThermostatSetPointLo;
                    if (Tprev > thisTempControlledZone.ZoneThermostatSetPointHi - TempTole) {
                        zoneTstatSetpt.setptHi = thisTempControlledZone.ZoneThermostatSetPointHi - thisTempControlledZone.DeltaTCutSet;
                    } else if (Tprev < thisTempControlledZone.ZoneThermostatSetPointHi &&
                               Tprev > thisTempControlledZone.ZoneThermostatSetPointHi - thisTempControlledZone.DeltaTCutSet + TempTole) {
                        zoneTstatSetpt.setptHi = thisTempControlledZone.ZoneThermostatSetPointHi - thisTempControlledZone.DeltaTCutSet;
                    } else {
                        thisTempControlledZone.CoolOffFlag = true;
                    }
                    if (thisTempControlledZone.CoolModeLast && Tprev < thisTempControlledZone.ZoneThermostatSetPointHi) {
                        zoneTstatSetpt.setptHi = thisTempControlledZone.ZoneThermostatSetPointHi;
                        thisTempControlledZone.CoolOffFlag = true;
                    }

                    if (Tprev < thisTempControlledZone.ZoneThermostatSetPointLo + TempTole) {
                        zoneTstatSetpt.setptLo = thisTempControlledZone.ZoneThermostatSetPointLo + thisTempControlledZone.DeltaTCutSet;
                    } else if (Tprev > thisTempControlledZone.ZoneThermostatSetPointLo &&
                               (Tprev < thisTempControlledZone.ZoneThermostatSetPointLo + thisTempControlledZone.DeltaTCutSet - TempTole)) {
                        zoneTstatSetpt.setptLo = thisTempControlledZone.ZoneThermostatSetPointLo + thisTempControlledZone.DeltaTCutSet;
                    } else {
                        thisTempControlledZone.HeatOffFlag = true;
                    }
                    if (thisTempControlledZone.HeatModeLast && Tprev > thisTempControlledZone.ZoneThermostatSetPointLo) {
                        zoneTstatSetpt.setptLo = thisTempControlledZone.ZoneThermostatSetPointLo;
                        thisTempControlledZone.HeatOffFlag = true;
                    }

                    // check setpoint for both and provde an error message
                    if (zoneTstatSetpt.setptLo >= zoneTstatSetpt.setptHi) {
                        ShowSevereError(state,
                                        "DualSetPointWithDeadBand: When Temperature Difference Between Cutout And Setpoint is applied, the heating "
                                        "setpoint is greater than the cooling setpoint. ");
                        ShowContinueErrorTimeStamp(state,
                                                   format("occurs in Zone={}", state.dataHeatBal->Zone(thisTempControlledZone.ActualZoneNum).Name));
                        ShowContinueError(state, format("Zone Heating ThermostatSetPoint={:.2R}", zoneTstatSetpt.setptLo));
                        ShowContinueError(state, format("Zone Cooling ThermostatSetPoint={:.2R}", zoneTstatSetpt.setptHi));
                        ShowFatalError(state, "Program terminates due to above conditions.");
                    }
                } break;

                default: {
                } break;
                } // switch (setptType)
            }
        }
    }

    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        auto &thisZoneHB = s_ztpc->zoneHeatBalance(zoneNum);
        thisZoneHB.predictSystemLoad(state, ShortenTimeStepSys, UseZoneTimeStepHistory, PriorTimeStep, zoneNum);
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            if (state.dataHeatBal->doSpaceHeatBalance) {
                s_ztpc->spaceHeatBalance(spaceNum).predictSystemLoad(
                    state, ShortenTimeStepSys, UseZoneTimeStepHistory, PriorTimeStep, zoneNum, spaceNum);
            } else if (ShortenTimeStepSys) {
                s_ztpc->spaceHeatBalance(spaceNum).MAT = thisZoneHB.MAT;
                s_ztpc->spaceHeatBalance(spaceNum).airHumRat = thisZoneHB.airHumRat;
            }
        }
    }
    if (s_ztpc->NumOnOffCtrZone > 0) {
        for (int RelativeZoneNum = 1; RelativeZoneNum <= state.dataZoneCtrls->NumTempControlledZones; ++RelativeZoneNum) {
            auto &thisTempControlledZone = state.dataZoneCtrls->TempControlledZone(RelativeZoneNum);
            if (thisTempControlledZone.DeltaTCutSet > 0.0) {
                int ZoneNum = thisTempControlledZone.ActualZoneNum;
                if (thisTempControlledZone.CoolOffFlag && state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).TotalOutputRequired >= 0.0) {
                    thisTempControlledZone.CoolModeLast = true;
                } else {
                    thisTempControlledZone.CoolModeLast = false;
                }
                if (thisTempControlledZone.HeatOffFlag && state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).TotalOutputRequired <= 0.0) {
                    thisTempControlledZone.HeatModeLast = true;
                } else {
                    thisTempControlledZone.HeatModeLast = false;
                }
            }
        }
    }
}
void ZoneSpaceHeatBalanceData::predictSystemLoad(
    EnergyPlusData &state,
    bool const shortenTimeStepSys,
    bool const useZoneTimeStepHistory, // if true then use zone timestep history, if false use system time step
    Real64 const priorTimeStep,        // the old value for timestep length is passed for possible use in interpolating
    int zoneNum,
    int spaceNum)
{
    assert(zoneNum > 0);
    this->updateTemperatures(state, shortenTimeStepSys, useZoneTimeStepHistory, priorTimeStep, zoneNum, spaceNum);

    Real64 TimeStepSys = state.dataHVACGlobal->TimeStepSys;
    Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

    Real64 volume = 0.0;
    if (spaceNum > 0) {
        volume = state.dataHeatBal->space(spaceNum).Volume;
    } else {
        volume = state.dataHeatBal->Zone(zoneNum).Volume;
    }

    this->AirPowerCap = volume * state.dataHeatBal->Zone(zoneNum).ZoneVolCapMultpSens *
                        Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, this->MAT, this->airHumRat) *
                        Psychrometrics::PsyCpAirFnW(this->airHumRat) / TimeStepSysSec;
    Real64 RAFNFrac = 0.0;

    // Calculate the various heat balance sums

    // NOTE: SumSysMCp and SumSysMCpT are not used in the predict step
    this->calcZoneOrSpaceSums(state, false, zoneNum, spaceNum);

    // Sum all convective internal gains except for people: SumIntGainExceptPeople
    if (spaceNum == 0 && state.dataHybridModel->FlagHybridModel_PC) {
        this->SumIntGainExceptPeople = 0.0;
        this->SumIntGainExceptPeople = InternalHeatGains::SumAllInternalConvectionGainsExceptPeople(state, zoneNum);
    }

    this->TempDepCoef = this->SumHA + this->SumMCp;
    this->TempIndCoef = this->SumIntGain + this->SumHATsurf - this->SumHATref + this->SumMCpT + this->SysDepZoneLoadsLagged;
    this->TempHistoryTerm = this->AirPowerCap * (3.0 * this->ZTM[0] - (3.0 / 2.0) * this->ZTM[1] + (1.0 / 3.0) * this->ZTM[2]);
    this->tempDepLoad = (11.0 / 6.0) * this->AirPowerCap + this->TempDepCoef;
    this->tempIndLoad = this->TempHistoryTerm + this->TempIndCoef;
    if (state.dataRoomAir->anyNonMixingRoomAirModel) {
        if (state.dataRoomAir->AirModel(zoneNum).AirModel == RoomAir::RoomAirModel::AirflowNetwork) {
            // RoomAirflowNetworkModel - make dynamic term independent of TimeStepSys
            auto &afnZoneInfo = state.dataRoomAir->AFNZoneInfo(zoneNum);
            if (afnZoneInfo.IsUsed) {
                int RoomAirNode = afnZoneInfo.ControlAirNodeID;
                RoomAir::LoadPredictionRoomAirModelAFN(state, zoneNum, RoomAirNode);
                this->TempDepCoef = afnZoneInfo.Node(RoomAirNode).SumHA + afnZoneInfo.Node(RoomAirNode).SumLinkMCp;
                this->TempIndCoef = afnZoneInfo.Node(RoomAirNode).SumIntSensibleGain + afnZoneInfo.Node(RoomAirNode).SumHATsurf -
                                    afnZoneInfo.Node(RoomAirNode).SumHATref + afnZoneInfo.Node(RoomAirNode).SumLinkMCpT +
                                    afnZoneInfo.Node(RoomAirNode).SysDepZoneLoadsLagged;
                this->AirPowerCap = afnZoneInfo.Node(RoomAirNode).AirVolume * state.dataHeatBal->Zone(zoneNum).ZoneVolCapMultpSens *
                                    afnZoneInfo.Node(RoomAirNode).RhoAir * afnZoneInfo.Node(RoomAirNode).CpAir / TimeStepSysSec;
                this->TempHistoryTerm = this->AirPowerCap * (3.0 * this->ZTM[0] - (3.0 / 2.0) * this->ZTM[1] + (1.0 / 3.0) * this->ZTM[2]);
                this->tempDepLoad = (11.0 / 6.0) * this->AirPowerCap + this->TempDepCoef;
                this->tempIndLoad = this->TempHistoryTerm + this->TempIndCoef;
                if (afnZoneInfo.Node(RoomAirNode).HasHVACAssigned) {
                    RAFNFrac = afnZoneInfo.Node(RoomAirNode).HVAC(1).SupplyFraction;
                }
            }
        }
    }

    // Exact solution or Euler method
    state.dataHVACGlobal->ShortenTimeStepSysRoomAir = false;
    if (state.dataHeatBal->ZoneAirSolutionAlgo != DataHeatBalance::SolutionAlgo::ThirdOrder) {
        if (shortenTimeStepSys && TimeStepSys < state.dataGlobal->TimeStepZone) {
            if (state.dataHVACGlobal->PreviousTimeStep < state.dataGlobal->TimeStepZone) {
                this->T1 = this->TM2;
                this->W1 = this->WM2;
                if (state.dataRoomAir->AirModel(zoneNum).AirModel == RoomAir::RoomAirModel::AirflowNetwork) {
                    auto &afnZoneInfo = state.dataRoomAir->AFNZoneInfo(zoneNum);
                    for (auto &afnNode : afnZoneInfo.Node) {
                        afnNode.AirTempT1 = afnNode.AirTempT2;
                        afnNode.HumRatT1 = afnNode.HumRatT2;
                    }
                }
            } else {
                this->T1 = this->TMX;
                this->W1 = this->WMX;
                if (state.dataRoomAir->AirModel(zoneNum).AirModel == RoomAir::RoomAirModel::AirflowNetwork) {
                    auto &afnZoneInfo = state.dataRoomAir->AFNZoneInfo(zoneNum);
                    for (auto &afnNode : afnZoneInfo.Node) {
                        afnNode.AirTempT1 = afnNode.AirTempTX;
                        afnNode.HumRatT1 = afnNode.HumRatTX;
                    }
                }
            }
            state.dataHVACGlobal->ShortenTimeStepSysRoomAir = true;
        } else {
            this->T1 = this->ZT;
            this->W1 = this->airHumRat;
            if (state.dataRoomAir->AirModel(zoneNum).AirModel == RoomAir::RoomAirModel::AirflowNetwork) {
                auto &afnZoneInfo = state.dataRoomAir->AFNZoneInfo(zoneNum);
                for (auto &afnNode : afnZoneInfo.Node) {
                    afnNode.AirTempT1 = afnNode.AirTemp;
                    afnNode.HumRatT1 = afnNode.HumRat;
                }
            }
        }
        this->tempDepLoad = this->TempDepCoef;
        this->tempIndLoad = this->TempIndCoef;
    }

    // Calculate the predicted zone load to be provided by the system with the given desired zone air temperature
    this->calcPredictedSystemLoad(state, RAFNFrac, zoneNum, spaceNum);

    // Calculate the predicted zone load to be provided by the system with the given desired humidity ratio
    this->calcPredictedHumidityRatio(state, RAFNFrac, zoneNum, spaceNum);
}

void CalcZoneAirTempSetPoints(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Russ Taylor
    //       DATE WRITTEN   Nov 1997
    //       MODIFIED       Aug 2013, Xiufeng Pang (XP) - Added code for updating set points during
    //                      optimum start period
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This routine sets what the setpoints for each controlled zone should be based on schedules.
    // This is called each time step.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int RelativeZoneNum;
    int ActualZoneNum;
    int OccStartTime; // Occupancy start time - for optimum start
    Real64 DeltaT;    // Temperature difference between cutout and setpoint

    auto &s_hbfs = state.dataHeatBalFanSys;

    auto &Zone = state.dataHeatBal->Zone;
    auto &TempControlledZone = state.dataZoneCtrls->TempControlledZone;
    auto &TempControlType = state.dataHeatBalFanSys->TempControlType;
    auto &TempControlTypeRpt = state.dataHeatBalFanSys->TempControlTypeRpt;
    int NumOfZones = state.dataGlobal->NumOfZones;

    TempControlType = HVAC::SetptType::Uncontrolled; // Default

    // Place holder for occupied heating and cooling set points - for optimum start
    if (!allocated(state.dataZoneCtrls->OccRoomTSetPointHeat)) {
        state.dataZoneCtrls->OccRoomTSetPointHeat.allocate(NumOfZones);
    }
    if (!allocated(state.dataZoneCtrls->OccRoomTSetPointCool)) {
        state.dataZoneCtrls->OccRoomTSetPointCool.allocate(NumOfZones);
    }
    state.dataZoneCtrls->OccRoomTSetPointHeat = 0.0;
    state.dataZoneCtrls->OccRoomTSetPointCool = 100.0;
    DeltaT = 0.0;

    for (RelativeZoneNum = 1; RelativeZoneNum <= state.dataZoneCtrls->NumTempControlledZones; ++RelativeZoneNum) {
        auto &tempZone = state.dataZoneCtrls->TempControlledZone(RelativeZoneNum);
        // What if this zone not controlled???

        int ActualZoneNum = tempZone.ActualZoneNum;
        TempControlType(ActualZoneNum) = static_cast<HVAC::SetptType>(tempZone.setptTypeSched->getCurrentVal());
        TempControlTypeRpt(ActualZoneNum) = static_cast<int>(TempControlType(ActualZoneNum));
        // Error detection for these values is done in the Get routine

        auto &zoneTstatSetpt = s_hbfs->zoneTstatSetpts(tempZone.ActualZoneNum);

        switch (TempControlType(ActualZoneNum)) {
        case HVAC::SetptType::Uncontrolled: {
        } break;

        case HVAC::SetptType::SingleHeat: {
            if (tempZone.setpts[(int)HVAC::SetptType::SingleHeat].isUsed) {
                zoneTstatSetpt.setpt = tempZone.setpts[(int)HVAC::SetptType::SingleHeat].heatSetptSched->getCurrentVal();
                tempZone.ZoneThermostatSetPointLo = zoneTstatSetpt.setpt;

                AdjustAirSetPointsforOpTempCntrl(state, RelativeZoneNum, ActualZoneNum, zoneTstatSetpt.setpt);
                zoneTstatSetpt.setptLo = zoneTstatSetpt.setpt;
            }
        } break;

        case HVAC::SetptType::SingleCool: {
            zoneTstatSetpt.setpt = tempZone.setpts[(int)HVAC::SetptType::SingleCool].coolSetptSched->getCurrentVal();
            tempZone.ZoneThermostatSetPointHi = zoneTstatSetpt.setpt;

            // Added Jan 17 (X. Luo)
            // Adjust operative temperature based on adaptive comfort model
            if (tempZone.AdaptiveComfortTempControl) {
                AdjustOperativeSetPointsforAdapComfort(state, RelativeZoneNum, zoneTstatSetpt.setpt);
                zoneTstatSetpt.setptAdapComfortCool = zoneTstatSetpt.setpt;
            }

            AdjustAirSetPointsforOpTempCntrl(state, RelativeZoneNum, ActualZoneNum, zoneTstatSetpt.setpt);
            zoneTstatSetpt.setptHi = zoneTstatSetpt.setpt;

            AdjustCoolingSetPointforTempAndHumidityControl(state, RelativeZoneNum, ActualZoneNum);
        } break;

        case HVAC::SetptType::SingleHeatCool: {
            zoneTstatSetpt.setpt = tempZone.setpts[(int)HVAC::SetptType::SingleHeatCool].heatSetptSched->getCurrentVal();

            // Added Jan 17 (X. Luo)
            // Adjust operative temperature based on adaptive comfort model
            if (tempZone.AdaptiveComfortTempControl) {
                AdjustOperativeSetPointsforAdapComfort(state, RelativeZoneNum, zoneTstatSetpt.setpt);
                zoneTstatSetpt.setptAdapComfortCool = zoneTstatSetpt.setpt;
            }

            AdjustAirSetPointsforOpTempCntrl(state, RelativeZoneNum, ActualZoneNum, zoneTstatSetpt.setpt);

            zoneTstatSetpt.setptLo = zoneTstatSetpt.setptHi = zoneTstatSetpt.setpt;

            // Change the room set point to occupied set point during optimum start period--------------

            if (allocated(state.dataAvail->OptStart)) {
                if (state.dataAvail->OptStart(ActualZoneNum).ActualZoneNum == ActualZoneNum) {

                    OccStartTime = CEILING(state.dataAvail->OptStart(ActualZoneNum).OccStartTime) + 1;
                    zoneTstatSetpt.setpt = tempZone.setpts[(int)HVAC::SetptType::SingleHeat].heatSetptSched->getDayVals(
                        state)[OccStartTime * state.dataGlobal->TimeStepsInHour];
                }

                if (state.dataAvail->OptStart(ActualZoneNum).OptStartFlag) {
                    zoneTstatSetpt.setptLo = zoneTstatSetpt.setptHi = zoneTstatSetpt.setpt;
                }
            }
            //--------------------------------------------------------------------------------------------
        } break;

        case HVAC::SetptType::DualHeatCool: {
            zoneTstatSetpt.setptHi = tempZone.setpts[(int)HVAC::SetptType::DualHeatCool].coolSetptSched->getCurrentVal();
            TempControlledZone(RelativeZoneNum).ZoneThermostatSetPointHi = zoneTstatSetpt.setptHi;

            // Added Jan 17 (X. Luo)
            // Adjust operative temperature based on adaptive comfort model
            if ((TempControlledZone(RelativeZoneNum).AdaptiveComfortTempControl)) {
                AdjustOperativeSetPointsforAdapComfort(state, RelativeZoneNum, zoneTstatSetpt.setptHi);
                zoneTstatSetpt.setptAdapComfortCool = zoneTstatSetpt.setptHi;
            }

            AdjustAirSetPointsforOpTempCntrl(state, RelativeZoneNum, ActualZoneNum, zoneTstatSetpt.setptHi);

            zoneTstatSetpt.setptLo = tempZone.setpts[(int)HVAC::SetptType::DualHeatCool].heatSetptSched->getCurrentVal();
            TempControlledZone(RelativeZoneNum).ZoneThermostatSetPointLo = zoneTstatSetpt.setptLo;
            AdjustAirSetPointsforOpTempCntrl(state, RelativeZoneNum, ActualZoneNum, zoneTstatSetpt.setptLo);

            // Change the room set point to occupied set point during optimum start period--------------

            if (allocated(state.dataAvail->OptStart)) {
                if (state.dataAvail->OptStart(ActualZoneNum).ActualZoneNum == ActualZoneNum) {
                    // TODO: Why are we getting all day values if all we want is the value at (1, OccStartTime);
                    OccStartTime = CEILING(state.dataAvail->OptStart(ActualZoneNum).OccStartTime) + 1;
                    state.dataZoneCtrls->OccRoomTSetPointCool(ActualZoneNum) =
                        tempZone.setpts[(int)HVAC::SetptType::DualHeatCool].coolSetptSched->getDayVals(
                            state)[OccStartTime * state.dataGlobal->TimeStepsInHour];
                    state.dataZoneCtrls->OccRoomTSetPointHeat(ActualZoneNum) =
                        tempZone.setpts[(int)HVAC::SetptType::DualHeatCool].heatSetptSched->getDayVals(
                            state)[OccStartTime * state.dataGlobal->TimeStepsInHour];
                }

                if (state.dataAvail->OptStart(ActualZoneNum).OptStartFlag) {
                    zoneTstatSetpt.setptHi = state.dataZoneCtrls->OccRoomTSetPointCool(ActualZoneNum);
                    zoneTstatSetpt.setptLo = state.dataZoneCtrls->OccRoomTSetPointHeat(ActualZoneNum);
                }
            }
            //--------------------------------------------------------------------------------------------

            AdjustCoolingSetPointforTempAndHumidityControl(state, RelativeZoneNum, ActualZoneNum);
        } break;

        default: {
            ShowSevereError(state,
                            format("CalcZoneAirTempSetpoints: Illegal control type for Zone={}, Found value={}, in Schedule={}",
                                   Zone(ActualZoneNum).Name,
                                   TempControlType(ActualZoneNum),
                                   tempZone.setptTypeSched->Name));

        } break;
        } // switch

        // Apply offset for faulty thermostats
        if ((state.dataFaultsMgr->NumFaultyThermostat > 0) && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) &&
            (!state.dataGlobal->KickOffSimulation)) {
            //  loop through the FaultsThermostatOffset objects to find the one for the zone
            for (int iFault = 1; iFault <= state.dataFaultsMgr->NumFaultyThermostat; ++iFault) {
                // Why are we doing this here?
                if (Util::SameString(TempControlledZone(RelativeZoneNum).Name,
                                     state.dataFaultsMgr->FaultsThermostatOffset(iFault).FaultyThermostatName)) {

                    // Check fault availability schedules
                    if (state.dataFaultsMgr->FaultsThermostatOffset(iFault).availSched->getCurrentVal() > 0.0) {

                        // Check fault severity schedules to update the reference thermostat offset
                        Real64 rSchVal = 1.0;
                        Real64 offsetUpdated;
                        if (state.dataFaultsMgr->FaultsThermostatOffset(iFault).severitySched != nullptr) {
                            rSchVal = state.dataFaultsMgr->FaultsThermostatOffset(iFault).severitySched->getCurrentVal();
                        }
                        offsetUpdated = rSchVal * state.dataFaultsMgr->FaultsThermostatOffset(iFault).Offset;

                        // Positive offset means the sensor reading is higher than the actual value
                        zoneTstatSetpt.setpt -= offsetUpdated;
                        zoneTstatSetpt.setptLo -= offsetUpdated;
                        zoneTstatSetpt.setptHi -= offsetUpdated;
                    }

                    // Stop searching the FaultsThermostatOffset object for the zone
                    break;
                }
            }
        }
    }

    if (state.dataZoneCtrls->NumComfortControlledZones > 0) {
        CalcZoneAirComfortSetPoints(state);
    }
    OverrideAirSetPointsforEMSCntrl(state);
}

void ZoneSpaceHeatBalanceData::calcPredictedHumidityRatio(EnergyPlusData &state, Real64 const RAFNFrac, int const zoneNum, int const spaceNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   May 2001

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine does the prediction step for humidity control

    // METHODOLOGY EMPLOYED:
    // This solves for the required system moisture required to try and achieve the desired
    // Humidity Ratio in the Zone

    // REFERENCES:
    // Routine FinalZnCalcs - FINAL ZONE CALCULATIONS, authored by Dale Herron
    // for BLAST.

    static constexpr std::string_view RoutineName("calcPredictedHumidityRatio");

    Real64 ZoneRHHumidifyingSetPoint = 0.0;   // Zone humidifying set point (%)
    Real64 ZoneRHDehumidifyingSetPoint = 0.0; // Zone dehumidifying set point (%)

    auto &thisZone = state.dataHeatBal->Zone(zoneNum);
    bool SingleSetPoint = false; // This determines whether both setpoint are equal or not

    // Check to see if this is a "humidity controlled zone"
    bool ControlledHumidZoneFlag = false;
    // Check all the controlled zones to see if it matches the zone simulated
    if (thisZone.humidityControlZoneIndex > 0) {
        auto &humidityControlZone = state.dataZoneCtrls->HumidityControlZone(thisZone.humidityControlZoneIndex);
        assert(humidityControlZone.ActualZoneNum == zoneNum);
        ZoneRHHumidifyingSetPoint = humidityControlZone.humidifyingSched->getCurrentVal();
        ZoneRHDehumidifyingSetPoint = humidityControlZone.dehumidifyingSched->getCurrentVal();

        // Apply EMS values to overwrite the humidistat values
        if (humidityControlZone.EMSOverrideHumidifySetPointOn) {
            ZoneRHHumidifyingSetPoint = humidityControlZone.EMSOverrideHumidifySetPointValue;
        }
        if (humidityControlZone.EMSOverrideDehumidifySetPointOn) {
            ZoneRHDehumidifyingSetPoint = humidityControlZone.EMSOverrideDehumidifySetPointValue;
        }

        // Apply offsets for faulty humidistats
        if ((state.dataFaultsMgr->NumFaultyHumidistat > 0) && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) &&
            (!state.dataGlobal->KickOffSimulation)) {

            //  loop through the FaultsHumidistatOffset objects to find the one for the zone
            for (int iFault = 1; iFault <= state.dataFaultsMgr->NumFaultyHumidistat; ++iFault) {

                if (Util::SameString(humidityControlZone.ControlName, state.dataFaultsMgr->FaultsHumidistatOffset(iFault).FaultyHumidistatName)) {

                    if (Util::SameString(state.dataFaultsMgr->FaultsHumidistatOffset(iFault).FaultyHumidistatType, "ThermostatOffsetDependent")) {
                        // For Humidistat Offset Type I: ThermostatOffsetDependent

                        bool IsThermostatFound = false;
                        Real64 offsetThermostat = 0.0;

                        // Get the offset value of the corresponding thermostat fault object
                        if (state.dataFaultsMgr->NumFaultyThermostat > 0) {

                            //  loop through the FaultsThermostatOffset objects to find the one causes the Humidistat Offset
                            for (int iFaultThermo = 1; iFaultThermo <= state.dataFaultsMgr->NumFaultyThermostat; ++iFaultThermo) {

                                if (Util::SameString(state.dataFaultsMgr->FaultsHumidistatOffset(iFault).FaultyThermostatName,
                                                     state.dataFaultsMgr->FaultsThermostatOffset(iFaultThermo).Name)) {
                                    IsThermostatFound = true;

                                    // Check fault availability schedules
                                    if (state.dataFaultsMgr->FaultsThermostatOffset(iFaultThermo).availSched->getCurrentVal() > 0.0) {

                                        // Check fault severity schedules to update the reference thermostat offset
                                        Real64 rSchVal = 1.0;
                                        if (state.dataFaultsMgr->FaultsThermostatOffset(iFaultThermo).severitySched != nullptr) {
                                            rSchVal = state.dataFaultsMgr->FaultsThermostatOffset(iFaultThermo).severitySched->getCurrentVal();
                                        }
                                        offsetThermostat = rSchVal * state.dataFaultsMgr->FaultsThermostatOffset(iFaultThermo).Offset;
                                    }

                                    // Stop searching the FaultsThermostatOffset object for the Humidistat Offset
                                    break;
                                }
                            }
                        }

                        // The FaultsThermostatOffset specified in the FaultHumidistatOffset is not found
                        if (!IsThermostatFound) {
                            ShowSevereError(
                                state,
                                format("FaultModel:HumidistatOffset = \"{}\" invalid Reference Humidistat Offset Name = \"{}\" not found.",
                                       state.dataFaultsMgr->FaultsHumidistatOffset(iFault).Name,
                                       state.dataFaultsMgr->FaultsHumidistatOffset(iFault).FaultyThermostatName));
                            ShowFatalError(state, "Errors getting FaultModel input data.  Preceding condition(s) cause termination.");
                        }

                        if (offsetThermostat != 0.0) {
                            // Calculate the humidistat offset value from the thermostat offset value
                            Real64 faultZoneWHumidifyingSetPoint = Psychrometrics::PsyWFnTdbRhPb(
                                state, (this->MAT + offsetThermostat), (ZoneRHHumidifyingSetPoint / 100.0), state.dataEnvrn->OutBaroPress);
                            Real64 faultZoneWDehumidifyingSetPoint = Psychrometrics::PsyWFnTdbRhPb(
                                state, (this->MAT + offsetThermostat), (ZoneRHDehumidifyingSetPoint / 100.0), state.dataEnvrn->OutBaroPress);
                            Real64 offsetZoneRHHumidifyingSetPoint =
                                ZoneRHHumidifyingSetPoint -
                                Psychrometrics::PsyRhFnTdbWPb(state, this->MAT, faultZoneWHumidifyingSetPoint, state.dataEnvrn->OutBaroPress) * 100.0;
                            Real64 offsetZoneRHDehumidifyingSetPoint =
                                ZoneRHDehumidifyingSetPoint -
                                Psychrometrics::PsyRhFnTdbWPb(state, this->MAT, faultZoneWDehumidifyingSetPoint, state.dataEnvrn->OutBaroPress) *
                                    100.0;

                            // Apply the calculated humidistat offset value
                            // Positive offset means the sensor reading is higher than the actual value
                            ZoneRHHumidifyingSetPoint -= offsetZoneRHHumidifyingSetPoint;
                            ZoneRHDehumidifyingSetPoint -= offsetZoneRHDehumidifyingSetPoint;

                            // constrain value to something reasonable
                            ZoneRHHumidifyingSetPoint = min(100.0, max(0.0, ZoneRHHumidifyingSetPoint));
                            ZoneRHDehumidifyingSetPoint = min(100.0, max(0.0, ZoneRHDehumidifyingSetPoint));
                        }

                    } else {
                        // For Humidistat Offset Type II: ThermostatOffsetIndependent

                        // Check fault availability schedules
                        if (state.dataFaultsMgr->FaultsHumidistatOffset(iFault).availSched->getCurrentVal() > 0.0) {

                            // Check fault severity schedules to update the reference humidistat offset
                            Real64 rSchVal = 1.0;
                            Real64 offsetUpdated;
                            if (state.dataFaultsMgr->FaultsHumidistatOffset(iFault).severitySched != nullptr) {
                                rSchVal = state.dataFaultsMgr->FaultsHumidistatOffset(iFault).severitySched->getCurrentVal();
                            }
                            offsetUpdated = rSchVal * state.dataFaultsMgr->FaultsHumidistatOffset(iFault).Offset;

                            // Positive offset means the sensor reading is higher than the actual value
                            ZoneRHHumidifyingSetPoint -= offsetUpdated;
                            ZoneRHDehumidifyingSetPoint -= offsetUpdated;

                            // constrain value to something reasonable
                            ZoneRHHumidifyingSetPoint = min(100.0, max(0.0, ZoneRHHumidifyingSetPoint));
                            ZoneRHDehumidifyingSetPoint = min(100.0, max(0.0, ZoneRHDehumidifyingSetPoint));
                        }
                    }
                    break;
                }
            }
        }

        // Run-time error check
        if (ZoneRHHumidifyingSetPoint > ZoneRHDehumidifyingSetPoint) {
            if (humidityControlZone.ErrorIndex == 0) {
                ShowWarningMessage(
                    state, format("HUMIDISTAT: The humidifying setpoint is above the dehumidifying setpoint in {}", humidityControlZone.ControlName));
                ShowContinueError(state, "The zone humidifying setpoint is set to the dehumidifying setpoint.");
                ShowContinueErrorTimeStamp(state, "Occurrence info:");
            }
            ShowRecurringWarningErrorAtEnd(state,
                                           "The humidifying setpoint is still above the dehumidifying setpoint",
                                           humidityControlZone.ErrorIndex,
                                           ZoneRHHumidifyingSetPoint,
                                           ZoneRHHumidifyingSetPoint);
            ZoneRHHumidifyingSetPoint = ZoneRHDehumidifyingSetPoint;
        }
        if (ZoneRHHumidifyingSetPoint == ZoneRHDehumidifyingSetPoint) {
            SingleSetPoint = true;
        }
        ControlledHumidZoneFlag = true;

    } // HumidControlledZoneNum

    // if zone latent sizing is requested but no humidistat exists
    if (state.dataGlobal->DoingSizing && !ControlledHumidZoneFlag && state.dataHeatBal->DoLatentSizing) {
        for (size_t zoneEqConfigNum = 1; zoneEqConfigNum <= state.dataZoneEquip->ZoneEquipConfig.size(); ++zoneEqConfigNum) {
            auto &zoneEqConfig = state.dataZoneEquip->ZoneEquipConfig(zoneEqConfigNum);
            if (!zoneEqConfig.IsControlled) {
                continue;
            }
            int ZoneSizNum = Util::FindItemInList(zoneEqConfig.ZoneName, state.dataSize->ZoneSizingInput, &DataSizing::ZoneSizingInputData::ZoneName);
            // should use the first Sizing:Zone object if not found
            if (ZoneSizNum == 0 && !state.dataSize->ZoneSizingInput.empty()) {
                ZoneSizNum = 1;
            }
            if (ZoneSizNum > 0) {
                auto &zoneSizingInput = state.dataSize->ZoneSizingInput(ZoneSizNum);
                if (zoneSizingInput.zoneLatentSizing) {
                    ZoneRHDehumidifyingSetPoint = (zoneSizingInput.zoneRHDehumidifySched != nullptr)
                                                      ? zoneSizingInput.zoneRHDehumidifySched->getCurrentVal()
                                                      : zoneSizingInput.zoneRHDehumidifySetPoint;
                    ZoneRHHumidifyingSetPoint = (zoneSizingInput.zoneRHHumidifySched != nullptr)
                                                    ? zoneSizingInput.zoneRHHumidifySched->getCurrentVal()
                                                    : zoneSizingInput.zoneRHHumidifySetPoint;
                    if (ZoneRHHumidifyingSetPoint > ZoneRHDehumidifyingSetPoint) {
                        ZoneRHHumidifyingSetPoint = ZoneRHDehumidifyingSetPoint;
                    }
                    if (ZoneRHHumidifyingSetPoint == ZoneRHDehumidifyingSetPoint) {
                        SingleSetPoint = true;
                    }
                    ControlledHumidZoneFlag = true;
                }
            }
            break;
        }
    }

    Real64 LoadToHumidifySetPoint = 0.0;   // Moisture load at humidifying set point
    Real64 LoadToDehumidifySetPoint = 0.0; // Moisture load at dehumidifying set point
    Real64 totalOutputRequired = 0.0;
    if (ControlledHumidZoneFlag) {

        // Calculate hourly humidity ratio from infiltration + humidity added from latent load
        // to determine system added/subtracted moisture.
        Real64 LatentGain = this->latentGain + state.dataHeatBalFanSys->SumLatentHTRadSys(zoneNum) + state.dataHeatBalFanSys->SumLatentPool(zoneNum);

        Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

        // Calculate the coefficients for the 3rd Order derivative for final
        // zone humidity ratio.  The A, B, C coefficients are analogous to the heat balance.
        // SumHmARaW and SumHmARa will be used with the Moisture Balance on the building elements and
        // are currently set to zero when the CTF only version is used.

        // The density of air and latent heat of vaporization are calculated as functions.
        Real64 RhoAir = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, this->ZT, this->airHumRat, RoutineName);
        Real64 H2OHtOfVap = Psychrometrics::PsyHgAirFnWTdb(this->airHumRat, this->ZT);

        // Assume that the system will have flow
        Real64 A = 0.0;
        Real64 B = 0.0;
        Real64 C = 0.0;
        if (state.afn->multizone_always_simulated ||
            (state.afn->simulation_control.type == AirflowNetwork::ControlType::MultizoneWithDistributionOnlyDuringFanOperation &&
             state.afn->AirflowNetworkFanActivated)) {
            // Multizone airflow calculated in AirflowNetwork
            B = (LatentGain / H2OHtOfVap) + state.afn->exchangeData(zoneNum).SumMHrW + state.afn->exchangeData(zoneNum).SumMMHrW + this->SumHmARaW;
            A = state.afn->exchangeData(zoneNum).SumMHr + state.afn->exchangeData(zoneNum).SumMMHr + this->SumHmARa;
        } else {
            B = (LatentGain / H2OHtOfVap) + ((this->OAMFL + this->VAMFL + this->CTMFL) * state.dataEnvrn->OutHumRat) + this->EAMFLxHumRat +
                this->SumHmARaW + this->MixingMassFlowXHumRat + this->MDotOA * state.dataEnvrn->OutHumRat;
            A = this->OAMFL + this->VAMFL + this->EAMFL + this->CTMFL + this->SumHmARa + this->MixingMassFlowZone + this->MDotOA;
        }
        Real64 volume = 0.0;
        if (spaceNum > 0) {
            volume = state.dataHeatBal->space(spaceNum).Volume;
        } else {
            volume = thisZone.Volume;
        }
        C = RhoAir * volume * thisZone.ZoneVolCapMultpMoist / TimeStepSysSec;

        if (state.dataRoomAir->AirModel(zoneNum).AirModel == RoomAir::RoomAirModel::AirflowNetwork) {
            auto &roomAFNInfo = state.dataRoomAir->AFNZoneInfo(zoneNum);
            int RoomAirNode = roomAFNInfo.ControlAirNodeID;
            H2OHtOfVap = Psychrometrics::PsyHgAirFnWTdb(roomAFNInfo.Node(RoomAirNode).HumRat, roomAFNInfo.Node(RoomAirNode).AirTemp);
            A = roomAFNInfo.Node(RoomAirNode).SumLinkM + roomAFNInfo.Node(RoomAirNode).SumHmARa;
            B = (roomAFNInfo.Node(RoomAirNode).SumIntLatentGain / H2OHtOfVap) + roomAFNInfo.Node(RoomAirNode).SumLinkMW +
                roomAFNInfo.Node(RoomAirNode).SumHmARaW;
            C = roomAFNInfo.Node(RoomAirNode).RhoAir * roomAFNInfo.Node(RoomAirNode).AirVolume * thisZone.ZoneVolCapMultpMoist / TimeStepSysSec;
        }

        // Use a 3rd Order derivative to predict zone moisture addition or removal and
        // smooth the changes using the zone air capacitance.  Positive values of Moist Load means that
        // this amount of moisture must be added to the zone to reach the setpoint.  Negative values represent
        // the amount of moisture that must be removed by the system.
        // MoistLoadHumidSetPoint = massflow * HumRat = kgDryAir/s * kgWater/kgDryAir = kgWater/s
        Real64 WZoneSetPoint =
            Psychrometrics::PsyWFnTdbRhPb(state, this->ZT, (ZoneRHHumidifyingSetPoint / 100.0), state.dataEnvrn->OutBaroPress, RoutineName);
        Real64 exp_700_A_C(0.0);
        if (state.dataHeatBal->ZoneAirSolutionAlgo == DataHeatBalance::SolutionAlgo::ThirdOrder) {
            LoadToHumidifySetPoint =
                ((11.0 / 6.0) * C + A) * WZoneSetPoint -
                (B + C * (3.0 * this->WPrevZoneTSTemp[0] - (3.0 / 2.0) * this->WPrevZoneTSTemp[1] + (1.0 / 3.0) * this->WPrevZoneTSTemp[2]));
            // Exact solution
        } else if (state.dataHeatBal->ZoneAirSolutionAlgo == DataHeatBalance::SolutionAlgo::AnalyticalSolution) {
            if (A == 0.0) { // B=0
                LoadToHumidifySetPoint = C * (WZoneSetPoint - this->W1) - B;
            } else {
                exp_700_A_C = std::exp(min(700.0, -A / C)); // Tuned Save expensive value
                LoadToHumidifySetPoint = A * (WZoneSetPoint - this->W1 * exp_700_A_C) / (1.0 - exp_700_A_C) - B;
            }
        } else if (state.dataHeatBal->ZoneAirSolutionAlgo == DataHeatBalance::SolutionAlgo::EulerMethod) {
            LoadToHumidifySetPoint = C * (WZoneSetPoint - this->W1) + A * WZoneSetPoint - B;
        }
        if (RAFNFrac > 0.0) {
            LoadToHumidifySetPoint = LoadToHumidifySetPoint / RAFNFrac;
        }
        WZoneSetPoint =
            Psychrometrics::PsyWFnTdbRhPb(state, this->ZT, (ZoneRHDehumidifyingSetPoint / 100.0), state.dataEnvrn->OutBaroPress, RoutineName);
        if (state.dataHeatBal->ZoneAirSolutionAlgo == DataHeatBalance::SolutionAlgo::ThirdOrder) {
            LoadToDehumidifySetPoint =
                ((11.0 / 6.0) * C + A) * WZoneSetPoint -
                (B + C * (3.0 * this->WPrevZoneTSTemp[0] - (3.0 / 2.0) * this->WPrevZoneTSTemp[1] + (1.0 / 3.0) * this->WPrevZoneTSTemp[2]));
            // Exact solution
        } else if (state.dataHeatBal->ZoneAirSolutionAlgo == DataHeatBalance::SolutionAlgo::AnalyticalSolution) {
            if (A == 0.0) { // B=0
                LoadToDehumidifySetPoint = C * (WZoneSetPoint - this->W1) - B;
            } else {
                LoadToDehumidifySetPoint = A * (WZoneSetPoint - this->W1 * exp_700_A_C) / (1.0 - exp_700_A_C) - B; // exp_700_A_C set above
            }
        } else if (state.dataHeatBal->ZoneAirSolutionAlgo == DataHeatBalance::SolutionAlgo::EulerMethod) {
            LoadToDehumidifySetPoint = C * (WZoneSetPoint - this->W1) + A * WZoneSetPoint - B;
        }
        if (RAFNFrac > 0.0) {
            LoadToDehumidifySetPoint = LoadToDehumidifySetPoint / RAFNFrac;
        }

        // The load is added to the TotalOutputRequired as in the Temperature Predictor.  There is also the remaining
        // output variable for those who will use this for humidity control and stored in DataZoneEnergyDemands with the
        // analogous temperature terms.

        if (SingleSetPoint) {
            totalOutputRequired = LoadToHumidifySetPoint;
        } else {
            if (LoadToHumidifySetPoint > 0.0 && LoadToDehumidifySetPoint > 0.0) {
                totalOutputRequired = LoadToHumidifySetPoint;
            } else if (LoadToHumidifySetPoint < 0.0 && LoadToDehumidifySetPoint < 0.0) {
                totalOutputRequired = LoadToDehumidifySetPoint;
            } else if (LoadToHumidifySetPoint <= 0.0 && LoadToDehumidifySetPoint >= 0.0) { // deadband includes zero loads
                totalOutputRequired = 0.0;
            } else { // this should never occur!
                ShowSevereError(
                    state, "Humidistat: Unanticipated combination of humidifying and dehumidifying loads - report to EnergyPlus Development Team");
                ShowContinueErrorTimeStamp(state, format("occurs in Zone = {}", thisZone.Name));
                ShowContinueError(
                    state,
                    format("LoadToHumidifySetPoint={:.5R}, LoadToDehumidifySetPoint={:.5R}", LoadToHumidifySetPoint, LoadToDehumidifySetPoint));
                ShowContinueError(state, format("Zone RH Humidifying Set-point={:.1R}", ZoneRHHumidifyingSetPoint));
                ShowContinueError(state, format("Zone RH Dehumidifying Set-point={:.2R}", ZoneRHDehumidifyingSetPoint));
                ShowFatalError(state, "Program terminates due to above conditions.");
            }
        }
    }

    // Apply zone multipliers as needed or set to zero
    if (spaceNum > 0) {
        auto &thisspaceSysMoistureDemand = state.dataZoneEnergyDemand->spaceSysMoistureDemand(spaceNum);
        if (ControlledHumidZoneFlag) {
            thisspaceSysMoistureDemand.reportMoistLoadsZoneMultiplier(
                state, zoneNum, totalOutputRequired, LoadToHumidifySetPoint, LoadToDehumidifySetPoint);
        } else {
            thisspaceSysMoistureDemand.TotalOutputRequired = 0.0;
            thisspaceSysMoistureDemand.OutputRequiredToDehumidifyingSP = 0.0;
            thisspaceSysMoistureDemand.OutputRequiredToHumidifyingSP = 0.0;
        }
    } else {
        auto &thisZoneSysMoistureDemand = state.dataZoneEnergyDemand->ZoneSysMoistureDemand(zoneNum);
        if (ControlledHumidZoneFlag) {
            thisZoneSysMoistureDemand.reportMoistLoadsZoneMultiplier(
                state, zoneNum, totalOutputRequired, LoadToHumidifySetPoint, LoadToDehumidifySetPoint);
        } else {
            thisZoneSysMoistureDemand.TotalOutputRequired = 0.0;
            thisZoneSysMoistureDemand.OutputRequiredToDehumidifyingSP = 0.0;
            thisZoneSysMoistureDemand.OutputRequiredToHumidifyingSP = 0.0;
        }
    }
}

Real64 correctZoneAirTemps(EnergyPlusData &state,
                           bool useZoneTimeStepHistory // if true then use zone timestep history, if false use system time step history
)
{
    auto &s_ztpc = state.dataZoneTempPredictorCorrector;
    Real64 maxTempChange = DataPrecisionGlobals::constant_zero; // Max absolute air temperature change between previous and current timestep
    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        auto &thisZoneHB = s_ztpc->zoneHeatBalance(zoneNum);
        Real64 zoneTempChange = thisZoneHB.correctAirTemp(state, useZoneTimeStepHistory, zoneNum);
        auto &thisZone = state.dataHeatBal->Zone(zoneNum);
        for (int spaceNum : thisZone.spaceIndexes) {
            auto &thisSpaceHB = s_ztpc->spaceHeatBalance(spaceNum);
            if (state.dataHeatBal->doSpaceHeatBalanceSimulation &&
                !state.dataGlobal->DoingSizing) { // Need space air temps to match zone temps for sizing
                Real64 spaceTempChange = thisSpaceHB.correctAirTemp(state, useZoneTimeStepHistory, zoneNum, spaceNum);
                maxTempChange = max(maxTempChange, spaceTempChange);
            } else {
                // If doing sizing and zone is controlled, then set space node to match zone node
                if (state.dataHeatBal->doSpaceHeatBalanceSizing && thisZone.IsControlled) {
                    auto const &thisZoneNode = state.dataLoopNodes->Node(thisZone.SystemZoneNodeNumber);
                    auto &thisSpaceNode = state.dataLoopNodes->Node(state.dataHeatBal->space(spaceNum).SystemZoneNodeNumber);
                    thisSpaceNode.Temp = thisZoneNode.Temp;
                    thisSpaceNode.HumRat = thisZoneNode.HumRat;
                    thisSpaceNode.Enthalpy = thisZoneNode.Enthalpy;
                }
                // If no SpaceHB or doing sizing, then set space temps and humrat to match zone
                thisSpaceHB.ZT = thisZoneHB.ZT;
                thisSpaceHB.ZTM = thisZoneHB.ZTM;
                thisSpaceHB.MAT = thisZoneHB.MAT;
                thisSpaceHB.airHumRat = thisZoneHB.airHumRat;
                thisSpaceHB.airRelHum = thisZoneHB.airRelHum;
                // thisSpaceHB.ZTAVComf = thisZoneHB.ZTAVComf;
            }
        }
        maxTempChange = max(maxTempChange, zoneTempChange);

        CalcZoneComponentLoadSums(state, zoneNum, &s_ztpc->zoneHeatBalance(zoneNum), state.dataHeatBal->ZnAirRpt(zoneNum));
        if (state.dataHeatBal->doSpaceHeatBalanceSimulation) {
            for (int spaceNum : thisZone.spaceIndexes) {
                CalcZoneComponentLoadSums(state, zoneNum, &s_ztpc->spaceHeatBalance(spaceNum), state.dataHeatBal->spaceAirRpt(spaceNum));
            }
        }
    }
    return maxTempChange;
}

Real64 ZoneSpaceHeatBalanceData::correctAirTemp(
    EnergyPlusData &state,
    bool const useZoneTimeStepHistory, // if true then use zone timestep history, if false use system time step history
    int const zoneNum,
    int const spaceNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Russell Taylor
    //       MODIFIED       November 1999, LKL; November 2016 Sang Hoon Lee, Tianzhen Hong, Rongpeng Zhang;
    //       RE-ENGINEERED  July 2003 (Peter Graham Ellis)
    //                      February 2008 (Brent Griffith reworked history )

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine updates the zone air temperature and modifies the system
    // time step.

    static constexpr std::string_view RoutineName("correctAirTemp");

    Real64 tempChange = DataPrecisionGlobals::constant_zero; // Zone or space air temperature change between previous and current timestep

    assert(zoneNum > 0);
    auto &thisZone = state.dataHeatBal->Zone(zoneNum);

    // Update zone temperatures

    Real64 ZoneMult = thisZone.Multiplier * thisZone.ListMultiplier;

    Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

    // update the variables actually used in the balance equations.
    if (!useZoneTimeStepHistory) {
        this->ZTM = this->DSXMAT;
        this->WPrevZoneTSTemp = this->DSWPrevZoneTS;
    } else {
        this->ZTM = this->XMAT;
        this->WPrevZoneTSTemp = this->WPrevZoneTS;
    }

    Real64 volume = 0.0;
    if (spaceNum > 0) {
        volume = state.dataHeatBal->space(spaceNum).Volume;
    } else {
        volume = thisZone.Volume;
    }
    this->AirPowerCap = volume * thisZone.ZoneVolCapMultpSens *
                        Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, this->MAT, this->airHumRat, RoutineName) *
                        Psychrometrics::PsyCpAirFnW(this->airHumRat) / TimeStepSysSec;

    // SpaceHB TODO: For now, room air model is only for zones
    if (spaceNum == 0) {
        RoomAir::ManageAirModel(state, zoneNum);
    }

    // Calculate the various heat balance sums
    this->calcZoneOrSpaceSums(state, true, zoneNum, spaceNum);

    // Sum all convective internal gains except for people: SumIntGainExceptPeople
    if (state.dataHybridModel->FlagHybridModel_PC) {
        // TODO: For now, don't do space heat balance with hybrid model
        this->SumIntGainExceptPeople = InternalHeatGains::SumAllInternalConvectionGainsExceptPeople(state, zoneNum);
    }

    //    ZoneTempHistoryTerm = (3.0D0 * ZTM1(zoneNum) - (3.0D0/2.0D0) * ZTM2(zoneNum) + (1.0D0/3.0D0) * ZTM3(zoneNum))
    int ZoneNodeNum = thisZone.SystemZoneNodeNumber;
    if (spaceNum > 0) {
        ZoneNodeNum = state.dataHeatBal->space(spaceNum).SystemZoneNodeNumber;
    }

    Real64 SNLoad = 0.0;

    if (ZoneNodeNum > 0) { // This zone is controlled by a zone equipment configuration or zone plenum
        auto &thisSystemNode = state.dataLoopNodes->Node(ZoneNodeNum);

        // Heat balance coefficients for controlled zone, i.e. with system air flow
        this->TempDepCoef = this->SumHA + this->SumMCp + this->SumSysMCp;
        this->TempIndCoef = this->SumIntGain + this->SumHATsurf - this->SumHATref + this->SumMCpT + this->SumSysMCpT +
                            (this->NonAirSystemResponse / ZoneMult + this->SysDepZoneLoadsLagged);

        if (state.afn->distribution_simulated) {
            this->TempIndCoef += state.afn->exchangeData(zoneNum).TotalSen;
        }

        // Solve for zone air temperature
        switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
        case DataHeatBalance::SolutionAlgo::ThirdOrder: {
            this->ZT = (this->TempIndCoef + this->AirPowerCap * (3.0 * this->ZTM[0] - (3.0 / 2.0) * this->ZTM[1] + (1.0 / 3.0) * this->ZTM[2])) /
                       ((11.0 / 6.0) * this->AirPowerCap + this->TempDepCoef);
        } break;
        case DataHeatBalance::SolutionAlgo::AnalyticalSolution: {
            if (this->TempDepCoef == 0.0) { // B=0
                this->ZT = this->T1 + this->TempIndCoef / this->AirPowerCap;
            } else {
                this->ZT = (this->T1 - this->TempIndCoef / this->TempDepCoef) * std::exp(min(700.0, -this->TempDepCoef / this->AirPowerCap)) +
                           this->TempIndCoef / this->TempDepCoef;
            }
        } break;
        case DataHeatBalance::SolutionAlgo::EulerMethod: {
            this->ZT = (this->AirPowerCap * this->T1 + this->TempIndCoef) / (this->AirPowerCap + this->TempDepCoef);
        } break;
        default:
            break;
        }
        // Update zone node temperature and thermostat temperature unless already updated in Room Air Model,
        // calculate load correction factor
        if (!state.dataRoomAir->anyNonMixingRoomAirModel) {
            // Fully mixed
            thisSystemNode.Temp = this->ZT;
            // SpaceHB TODO: What to do here if this is for space
            if (spaceNum == 0) {
                state.dataHeatBalFanSys->TempTstatAir(zoneNum) = this->ZT;
            }
            state.dataHeatBalFanSys->LoadCorrectionFactor(zoneNum) = 1.0;
        } else {
            auto const &thisAirModel = state.dataRoomAir->AirModel(zoneNum);
            if ((thisAirModel.AirModel == RoomAir::RoomAirModel::Mixing) || (!thisAirModel.SimAirModel)) {
                // Fully mixed
                thisSystemNode.Temp = this->ZT;
                // SpaceHB TODO: What to do here if this is for space
                if (spaceNum == 0) {
                    state.dataHeatBalFanSys->TempTstatAir(zoneNum) = this->ZT;
                }
                state.dataHeatBalFanSys->LoadCorrectionFactor(zoneNum) = 1.0;
            } else if (state.dataRoomAir->IsZoneDispVent3Node(zoneNum) || state.dataRoomAir->IsZoneUFAD(zoneNum)) {
                // UCSDDV: Not fully mixed - calculate factor to correct load for fully mixed assumption
                // Space HB TODO: Space HB doesn't mix with DV etc.
                if (this->SumSysMCp > HVAC::SmallMassFlow) {
                    Real64 TempSupplyAir = this->SumSysMCpT / this->SumSysMCp; // Non-negligible flow, calculate supply air temperature
                    if (std::abs(TempSupplyAir - this->ZT) > state.dataHeatBal->TempConvergTol) {
                        state.dataHeatBalFanSys->LoadCorrectionFactor(zoneNum) = (TempSupplyAir - thisSystemNode.Temp) / (TempSupplyAir - this->ZT);
                        // constrain value to something reasonable
                        state.dataHeatBalFanSys->LoadCorrectionFactor(zoneNum) = max(-3.0, state.dataHeatBalFanSys->LoadCorrectionFactor(zoneNum));
                        state.dataHeatBalFanSys->LoadCorrectionFactor(zoneNum) = min(3.0, state.dataHeatBalFanSys->LoadCorrectionFactor(zoneNum));

                    } else {
                        state.dataHeatBalFanSys->LoadCorrectionFactor(zoneNum) = 1.0; // Indeterminate
                    }
                } else {
                    // Negligible flow, assume mixed - reasonable lagged starting value for first step time with significant flow
                    state.dataHeatBalFanSys->LoadCorrectionFactor(zoneNum) = 1.0;
                }
            } else if (thisAirModel.SimAirModel && ((thisAirModel.AirModel == RoomAir::RoomAirModel::UserDefined) ||
                                                    (thisAirModel.AirModel == RoomAir::RoomAirModel::DispVent1Node))) {
                if (this->SumSysMCp > HVAC::SmallMassFlow) {
                    Real64 TempSupplyAir = this->SumSysMCpT / this->SumSysMCp; // Non-negligible flow, calculate supply air temperature
                    if (std::abs(TempSupplyAir - this->ZT) > state.dataHeatBal->TempConvergTol) {
                        state.dataHeatBalFanSys->LoadCorrectionFactor(zoneNum) = (TempSupplyAir - thisSystemNode.Temp) / (TempSupplyAir - this->ZT);
                        // constrain value
                        state.dataHeatBalFanSys->LoadCorrectionFactor(zoneNum) = max(-3.0, state.dataHeatBalFanSys->LoadCorrectionFactor(zoneNum));
                        state.dataHeatBalFanSys->LoadCorrectionFactor(zoneNum) = min(3.0, state.dataHeatBalFanSys->LoadCorrectionFactor(zoneNum));

                    } else {
                        state.dataHeatBalFanSys->LoadCorrectionFactor(zoneNum) = 1.0; // Indeterminate
                    }
                } else {
                    // Negligible flow, assume mixed - reasonable lagged starting value for first step time with significant flow
                    state.dataHeatBalFanSys->LoadCorrectionFactor(zoneNum) = 1.0;
                }
            } else if (thisAirModel.AirModel == RoomAir::RoomAirModel::AirflowNetwork) {
                // Zone node used in the RoomAirflowNetwork model
                this->ZT = state.dataRoomAir->AFNZoneInfo(zoneNum).Node(state.dataRoomAir->AFNZoneInfo(zoneNum).ControlAirNodeID).AirTemp;
                thisSystemNode.Temp = this->ZT;
                // SpaceHB TODO: What to do here if this is for space
                if (spaceNum == 0) {
                    state.dataHeatBalFanSys->TempTstatAir(zoneNum) = this->ZT;
                }
                state.dataHeatBalFanSys->LoadCorrectionFactor(zoneNum) = 1.0;
            } else {
                thisSystemNode.Temp = this->ZT;
                // SpaceHB TODO: What to do here if this is for space
                if (spaceNum == 0) {
                    state.dataHeatBalFanSys->TempTstatAir(zoneNum) = this->ZT;
                }
                state.dataHeatBalFanSys->LoadCorrectionFactor(zoneNum) = 1.0;
            }
        }

        // Sensible load is the enthalpy into the zone minus the enthalpy that leaves the zone.
        Real64 CpAir = Psychrometrics::PsyCpAirFnW(this->airHumRat);
        Real64 ZoneEnthalpyIn = this->SumSysMCpT;

        // SNLOAD is the single zone load, without Zone Multiplier or Zone List Multiplier
        SNLoad = ZoneEnthalpyIn - (thisSystemNode.MassFlowRate / ZoneMult) * CpAir * thisSystemNode.Temp + this->NonAirSystemResponse / ZoneMult +
                 this->SysDepZoneLoadsLagged;

    } else {

        // Heat balance coefficients for uncontrolled zone, i.e. without system air flow
        this->TempDepCoef = this->SumHA + this->SumMCp;
        this->TempIndCoef = this->SumIntGain + this->SumHATsurf - this->SumHATref + this->SumMCpT;

        if (state.afn->distribution_simulated) {
            this->TempIndCoef += state.afn->exchangeData(zoneNum).TotalSen;
        }

        // Solve for zone air temperature
        switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
        case DataHeatBalance::SolutionAlgo::ThirdOrder: {
            this->ZT = (this->TempIndCoef + this->AirPowerCap * (3.0 * this->ZTM[0] - (3.0 / 2.0) * this->ZTM[1] + (1.0 / 3.0) * this->ZTM[2])) /
                       ((11.0 / 6.0) * this->AirPowerCap + this->TempDepCoef);
            // Exact solution
        } break;
        case DataHeatBalance::SolutionAlgo::AnalyticalSolution: {
            if (this->TempDepCoef == 0.0) { // B=0
                this->ZT = this->T1 + this->TempIndCoef / this->AirPowerCap;
            } else {
                this->ZT = (this->T1 - this->TempIndCoef / this->TempDepCoef) * std::exp(min(700.0, -this->TempDepCoef / this->AirPowerCap)) +
                           this->TempIndCoef / this->TempDepCoef;
            }
        } break;
        case DataHeatBalance::SolutionAlgo::EulerMethod: {
            this->ZT = (this->AirPowerCap * this->T1 + this->TempIndCoef) / (this->AirPowerCap + this->TempDepCoef);
        } break;
        default:
            break;
        }

        // SpaceHB TODO: For now, room air model is only for zones
        if (spaceNum == 0 && state.dataRoomAir->anyNonMixingRoomAirModel) {
            if (state.dataRoomAir->AirModel(zoneNum).AirModel == RoomAir::RoomAirModel::AirflowNetwork) {
                this->ZT = state.dataRoomAir->AFNZoneInfo(zoneNum).Node(state.dataRoomAir->AFNZoneInfo(zoneNum).ControlAirNodeID).AirTemp;
            }
        }

        // No sensible load
        SNLoad = 0.0;
    }

    // Hybrid modeling start
    // SpaceHB TODO: For now, hybrid model is only for zones
    if (spaceNum == 0 && state.dataHybridModel->FlagHybridModel) {
        auto &hmZone = state.dataHybridModel->hybridModelZones(zoneNum);
        if ((hmZone.InfiltrationCalc_T || hmZone.InternalThermalMassCalc_T || hmZone.PeopleCountCalc_T) && (!state.dataGlobal->WarmupFlag) &&
            (!state.dataGlobal->DoingSizing)) {
            InverseModelTemperature(state,
                                    zoneNum,
                                    this->SumIntGain,
                                    this->SumIntGainExceptPeople,
                                    this->SumHA,
                                    this->SumHATsurf,
                                    this->SumHATref,
                                    this->SumMCp,
                                    this->SumMCpT,
                                    this->SumSysMCp,
                                    this->SumSysMCpT,
                                    this->AirPowerCap);
        }
    }

    this->MAT = this->ZT;

    // Determine sensible load heating/cooling rate and energy
    if (spaceNum > 0) {
        state.dataZoneEnergyDemand->spaceSysEnergyDemand(spaceNum).reportZoneAirSystemSensibleLoads(state, SNLoad);
    } else {
        state.dataZoneEnergyDemand->ZoneSysEnergyDemand(zoneNum).reportZoneAirSystemSensibleLoads(state, SNLoad);
    }

    // Final humidity calcs
    this->correctHumRat(state, zoneNum, spaceNum);

    this->airHumRat = this->airHumRatTemp;
    this->airRelHum = 100.0 * Psychrometrics::PsyRhFnTdbWPb(state, this->ZT, this->airHumRat, state.dataEnvrn->OutBaroPress, RoutineName);

    // tempChange is used by HVACManager to determine if the timestep needs to be shortened.
    bool isMixed = true;
    // SpaceHB TODO: For now, room air model is only for zones
    if (spaceNum == 0 && state.dataRoomAir->anyNonMixingRoomAirModel) {
        isMixed = !((state.dataRoomAir->IsZoneDispVent3Node(zoneNum) && !state.dataRoomAir->ZoneDispVent3NodeMixedFlag(zoneNum)) ||
                    (state.dataRoomAir->IsZoneUFAD(zoneNum) && !state.dataRoomAir->ZoneUFADMixedFlag(zoneNum)));
    }
    switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
    case DataHeatBalance::SolutionAlgo::ThirdOrder: {
        if (isMixed) {
            tempChange = max(tempChange, std::abs(this->ZT - this->ZTM[0]));
        } else {
            tempChange = max(tempChange,
                             max(std::abs(state.dataRoomAir->ZTOC(zoneNum) - state.dataRoomAir->ZTMOC(zoneNum)[0]),
                                 std::abs(state.dataRoomAir->ZTMX(zoneNum) - state.dataRoomAir->ZTMMX(zoneNum)[0])));
        }
    } break;
    case DataHeatBalance::SolutionAlgo::AnalyticalSolution:
    case DataHeatBalance::SolutionAlgo::EulerMethod: {
        if (isMixed) {
            tempChange = max(tempChange, std::abs(this->ZT - this->T1));
        } else {
            tempChange = max(tempChange,
                             max(std::abs(state.dataRoomAir->ZTOC(zoneNum) - state.dataRoomAir->Zone1OC(zoneNum)),
                                 std::abs(state.dataRoomAir->ZTMX(zoneNum) - state.dataRoomAir->Zone1MX(zoneNum))));
        }
    } break;
    default:
        break;
    }

    return tempChange;
}

void PushZoneTimestepHistories(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   February 2008

    // PURPOSE OF THIS SUBROUTINE:
    // push histories for timestep advancing
    auto &s_ztpc = state.dataZoneTempPredictorCorrector;
    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        s_ztpc->zoneHeatBalance(zoneNum).pushZoneTimestepHistory(state, zoneNum);
        if (state.dataHeatBal->doSpaceHeatBalance) {
            for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                s_ztpc->spaceHeatBalance(spaceNum).pushZoneTimestepHistory(state, zoneNum, spaceNum);
            }
        }
    }
}

void ZoneSpaceHeatBalanceData::pushZoneTimestepHistory(EnergyPlusData &state, int const zoneNum, int const spaceNum)
{

    constexpr std::string_view routineName("pushTimestepHistories");
    assert(zoneNum > 0);

    auto &thisAirModel = state.dataRoomAir->AirModel(zoneNum);

    // Push the temperature and humidity ratio histories

    for (int iHistory = 3; iHistory >= 1; --iHistory) {
        this->XMAT[iHistory] = this->XMAT[iHistory - 1];
        this->WPrevZoneTS[iHistory] = this->WPrevZoneTS[iHistory - 1];
    }
    this->XMAT[0] = this->ZTAV; // using average for whole zone time step.
    this->XMPT = this->ZT;
    this->WPrevZoneTS[0] = this->airHumRatAvg; // using average for whole zone time step.
    this->airHumRat = this->airHumRatTemp;
    this->WTimeMinusP = this->airHumRatTemp;
    this->airRelHum = 100.0 * Psychrometrics::PsyRhFnTdbWPb(state, this->ZT, this->airHumRat, state.dataEnvrn->OutBaroPress, routineName);

    // SpaceHB TODO: For now, room air model is only for zones
    if (spaceNum == 0) {
        if (thisAirModel.AirModel == RoomAir::RoomAirModel::DispVent3Node || thisAirModel.AirModel == RoomAir::RoomAirModel::UFADInt ||
            thisAirModel.AirModel == RoomAir::RoomAirModel::UFADExt) {
            state.dataRoomAir->XMATFloor(zoneNum)[3] = state.dataRoomAir->XMATFloor(zoneNum)[2];
            state.dataRoomAir->XMATFloor(zoneNum)[2] = state.dataRoomAir->XMATFloor(zoneNum)[1];
            state.dataRoomAir->XMATFloor(zoneNum)[1] = state.dataRoomAir->XMATFloor(zoneNum)[0];
            state.dataRoomAir->XMATFloor(zoneNum)[0] = state.dataRoomAir->ZTFloor(zoneNum);
            state.dataRoomAir->MATFloor(zoneNum) = state.dataRoomAir->ZTFloor(zoneNum);

            state.dataRoomAir->XMATOC(zoneNum)[3] = state.dataRoomAir->XMATOC(zoneNum)[2];
            state.dataRoomAir->XMATOC(zoneNum)[2] = state.dataRoomAir->XMATOC(zoneNum)[1];
            state.dataRoomAir->XMATOC(zoneNum)[1] = state.dataRoomAir->XMATOC(zoneNum)[0];
            state.dataRoomAir->XMATOC(zoneNum)[0] = state.dataRoomAir->ZTOC(zoneNum);
            state.dataRoomAir->MATOC(zoneNum) = state.dataRoomAir->ZTOC(zoneNum);

            state.dataRoomAir->XMATMX(zoneNum)[3] = state.dataRoomAir->XMATMX(zoneNum)[2];
            state.dataRoomAir->XMATMX(zoneNum)[2] = state.dataRoomAir->XMATMX(zoneNum)[1];
            state.dataRoomAir->XMATMX(zoneNum)[1] = state.dataRoomAir->XMATMX(zoneNum)[0];
            state.dataRoomAir->XMATMX(zoneNum)[0] = state.dataRoomAir->ZTMX(zoneNum);
            state.dataRoomAir->MATMX(zoneNum) = state.dataRoomAir->ZTMX(zoneNum);
        }

        // for RoomAirflowNetwork model
        if (thisAirModel.AirModel == RoomAir::RoomAirModel::AirflowNetwork) {
            for (auto &afnNode : state.dataRoomAir->AFNZoneInfo(zoneNum).Node) {
                afnNode.AirTempX[3] = afnNode.AirTempX[2];
                afnNode.AirTempX[2] = afnNode.AirTempX[1];
                afnNode.AirTempX[1] = afnNode.AirTempX[0];
                afnNode.AirTempX[0] = afnNode.AirTemp;

                afnNode.HumRatX[3] = afnNode.HumRatX[2];
                afnNode.HumRatX[2] = afnNode.HumRatX[1];
                afnNode.HumRatX[1] = afnNode.HumRatX[0];
                afnNode.HumRatX[0] = afnNode.HumRat;
            }
        }
    }

    if (state.dataHeatBal->ZoneAirSolutionAlgo != DataHeatBalance::SolutionAlgo::ThirdOrder) {
        this->TM2 = this->TMX;
        this->TMX = this->ZTAV; // using average for whole zone time step.
        this->WM2 = this->WMX;
        this->WMX = this->airHumRatAvg; // using average for whole zone time step.
        // SpaceHB TODO: For now, room air model is only for zones
        if (spaceNum == 0) {
            if (thisAirModel.AirModel == RoomAir::RoomAirModel::DispVent3Node || thisAirModel.AirModel == RoomAir::RoomAirModel::UFADInt ||
                thisAirModel.AirModel == RoomAir::RoomAirModel::UFADExt) {
                state.dataRoomAir->ZoneM2Floor(zoneNum) = state.dataRoomAir->ZoneMXFloor(zoneNum);
                state.dataRoomAir->ZoneMXFloor(zoneNum) = state.dataRoomAir->ZTFloor(zoneNum); // using average for whole zone time step.
                state.dataRoomAir->ZoneM2OC(zoneNum) = state.dataRoomAir->ZoneMXOC(zoneNum);
                state.dataRoomAir->ZoneMXOC(zoneNum) = state.dataRoomAir->ZTOC(zoneNum); // using average for whole zone time step.
                state.dataRoomAir->ZoneM2MX(zoneNum) = state.dataRoomAir->ZoneMXMX(zoneNum);
                state.dataRoomAir->ZoneMXMX(zoneNum) = state.dataRoomAir->ZTMX(zoneNum); // using average for whole zone time step.
            }

            if (thisAirModel.AirModel == RoomAir::RoomAirModel::AirflowNetwork) {
                for (auto &afnNode : state.dataRoomAir->AFNZoneInfo(zoneNum).Node) {
                    afnNode.AirTempT2 = afnNode.AirTempTX;
                    afnNode.AirTempTX = afnNode.AirTemp;

                    afnNode.HumRatT2 = afnNode.HumRatTX;
                    afnNode.HumRatTX = afnNode.HumRat;
                }
            }
        }
    }
}

void PushSystemTimestepHistories(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   April 2008

    // PURPOSE OF THIS SUBROUTINE:
    // Push the temperature and humidity ratio histories back in time
    auto &s_ztpc = state.dataZoneTempPredictorCorrector;
    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        s_ztpc->zoneHeatBalance(zoneNum).pushSystemTimestepHistory(state, zoneNum);
        if (state.dataHeatBal->doSpaceHeatBalance) {
            for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                s_ztpc->spaceHeatBalance(spaceNum).pushSystemTimestepHistory(state, zoneNum, spaceNum);
            }
        }
    }
}

void ZoneSpaceHeatBalanceData::pushSystemTimestepHistory(EnergyPlusData &state, int const zoneNum, int const spaceNum)
{
    assert(zoneNum > 0);
    for (int iHistory = 3; iHistory >= 1; --iHistory) {
        this->DSXMAT[iHistory] = this->DSXMAT[iHistory - 1];
        this->DSWPrevZoneTS[iHistory] = this->DSWPrevZoneTS[iHistory - 1];
    }
    this->DSXMAT[0] = this->MAT;
    this->DSWPrevZoneTS[0] = this->airHumRat;

    // SpaceHB TODO: For now, room air model is only for zones
    if (spaceNum == 0 && state.dataRoomAir->anyNonMixingRoomAirModel) {
        if (state.dataRoomAir->IsZoneDispVent3Node(zoneNum) || state.dataRoomAir->IsZoneUFAD(zoneNum)) {
            state.dataRoomAir->DSXMATFloor(zoneNum)[3] = state.dataRoomAir->DSXMATFloor(zoneNum)[2];
            state.dataRoomAir->DSXMATFloor(zoneNum)[2] = state.dataRoomAir->DSXMATFloor(zoneNum)[1];
            state.dataRoomAir->DSXMATFloor(zoneNum)[1] = state.dataRoomAir->DSXMATFloor(zoneNum)[0];
            state.dataRoomAir->DSXMATFloor(zoneNum)[0] = state.dataRoomAir->MATFloor(zoneNum);

            state.dataRoomAir->DSXMATOC(zoneNum)[3] = state.dataRoomAir->DSXMATOC(zoneNum)[2];
            state.dataRoomAir->DSXMATOC(zoneNum)[2] = state.dataRoomAir->DSXMATOC(zoneNum)[1];
            state.dataRoomAir->DSXMATOC(zoneNum)[1] = state.dataRoomAir->DSXMATOC(zoneNum)[0];
            state.dataRoomAir->DSXMATOC(zoneNum)[0] = state.dataRoomAir->MATOC(zoneNum);

            state.dataRoomAir->DSXMATMX(zoneNum)[3] = state.dataRoomAir->DSXMATMX(zoneNum)[2];
            state.dataRoomAir->DSXMATMX(zoneNum)[2] = state.dataRoomAir->DSXMATMX(zoneNum)[1];
            state.dataRoomAir->DSXMATMX(zoneNum)[1] = state.dataRoomAir->DSXMATMX(zoneNum)[0];
            state.dataRoomAir->DSXMATMX(zoneNum)[0] = state.dataRoomAir->MATMX(zoneNum);
        }
        if (state.dataRoomAir->AirModel(zoneNum).AirModel == RoomAir::RoomAirModel::AirflowNetwork) {
            for (auto &afnNode : state.dataRoomAir->AFNZoneInfo(zoneNum).Node) {
                afnNode.AirTempDSX[3] = afnNode.AirTempDSX[2];
                afnNode.AirTempDSX[2] = afnNode.AirTempDSX[1];
                afnNode.AirTempDSX[1] = afnNode.AirTempDSX[0];
                afnNode.AirTempDSX[0] = afnNode.AirTemp;

                afnNode.HumRatDSX[3] = afnNode.HumRatDSX[2];
                afnNode.HumRatDSX[2] = afnNode.HumRatDSX[1];
                afnNode.HumRatDSX[1] = afnNode.HumRatDSX[0];
                afnNode.HumRatDSX[0] = afnNode.HumRat;
            }
        }
    }

    if (state.dataHeatBal->ZoneAirSolutionAlgo != DataHeatBalance::SolutionAlgo::ThirdOrder) {
        this->TM2 = this->TMX;
        this->TMX = this->MAT; // using average for whole zone time step.
        this->WM2 = this->WMX;
        this->WMX = this->airHumRatTemp; // using average for whole zone time step.

        // SpaceHB TODO: For now, room air model is only for zones
        if (spaceNum == 0) {
            if (state.dataRoomAir->AirModel(zoneNum).AirModel == RoomAir::RoomAirModel::DispVent3Node ||
                state.dataRoomAir->AirModel(zoneNum).AirModel == RoomAir::RoomAirModel::UFADInt ||
                state.dataRoomAir->AirModel(zoneNum).AirModel == RoomAir::RoomAirModel::UFADExt) {
                state.dataRoomAir->ZoneM2Floor(zoneNum) = state.dataRoomAir->ZoneMXFloor(zoneNum);
                state.dataRoomAir->ZoneMXFloor(zoneNum) = state.dataRoomAir->ZTFloor(zoneNum); // using average for whole zone time step.
                state.dataRoomAir->ZoneM2OC(zoneNum) = state.dataRoomAir->ZoneMXOC(zoneNum);
                state.dataRoomAir->ZoneMXOC(zoneNum) = state.dataRoomAir->ZTOC(zoneNum); // using average for whole zone time step.
                state.dataRoomAir->ZoneM2MX(zoneNum) = state.dataRoomAir->ZoneMXMX(zoneNum);
                state.dataRoomAir->ZoneMXMX(zoneNum) = state.dataRoomAir->ZTMX(zoneNum); // using average for whole zone time step.
            }
            if (state.dataRoomAir->AirModel(zoneNum).AirModel == RoomAir::RoomAirModel::AirflowNetwork) {
                for (int LoopNode = 1; LoopNode <= state.dataRoomAir->AFNZoneInfo(zoneNum).NumOfAirNodes; ++LoopNode) {
                    auto &afnNode = state.dataRoomAir->AFNZoneInfo(zoneNum).Node(LoopNode);
                    afnNode.AirTempT2 = afnNode.AirTempTX;
                    afnNode.AirTempTX = afnNode.AirTemp;

                    afnNode.HumRatT2 = afnNode.HumRatTX;
                    afnNode.HumRatTX = afnNode.HumRat;
                }
            }
        }
    }
}

void RevertZoneTimestepHistories(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   February 2008

    // PURPOSE OF THIS SUBROUTINE:
    // Revert the temperature and humidity ratio histories
    auto &s_ztpc = state.dataZoneTempPredictorCorrector;
    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        s_ztpc->zoneHeatBalance(zoneNum).revertZoneTimestepHistory(state, zoneNum);
        if (state.dataHeatBal->doSpaceHeatBalance) {
            for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                s_ztpc->spaceHeatBalance(spaceNum).revertZoneTimestepHistory(state, zoneNum, spaceNum);
            }
        }
    }
}

void ZoneSpaceHeatBalanceData::revertZoneTimestepHistory(EnergyPlusData &state, int const zoneNum, int const spaceNum)
{
    assert(zoneNum > 0);

    for (int iHistory = 0; iHistory <= 2; ++iHistory) {
        this->XMAT[iHistory] = this->XMAT[iHistory + 1];
        this->WPrevZoneTS[iHistory] = this->WPrevZoneTS[iHistory + 1];
    }

    // SpaceHB TODO: For now, room air model is only for zones
    if (spaceNum == 0) {
        if (state.dataRoomAir->AirModel(zoneNum).AirModel == RoomAir::RoomAirModel::DispVent3Node ||
            state.dataRoomAir->AirModel(zoneNum).AirModel == RoomAir::RoomAirModel::UFADInt ||
            state.dataRoomAir->AirModel(zoneNum).AirModel == RoomAir::RoomAirModel::UFADExt) {

            state.dataRoomAir->XMATFloor(zoneNum)[0] = state.dataRoomAir->XMATFloor(zoneNum)[1];
            state.dataRoomAir->XMATFloor(zoneNum)[1] = state.dataRoomAir->XMATFloor(zoneNum)[2];
            state.dataRoomAir->XMATFloor(zoneNum)[2] = state.dataRoomAir->XMATFloor(zoneNum)[3];

            state.dataRoomAir->XMATOC(zoneNum)[0] = state.dataRoomAir->XMATOC(zoneNum)[1];
            state.dataRoomAir->XMATOC(zoneNum)[1] = state.dataRoomAir->XMATOC(zoneNum)[2];
            state.dataRoomAir->XMATOC(zoneNum)[2] = state.dataRoomAir->XMATOC(zoneNum)[3];

            state.dataRoomAir->XMATMX(zoneNum)[0] = state.dataRoomAir->XMATMX(zoneNum)[1];
            state.dataRoomAir->XMATMX(zoneNum)[1] = state.dataRoomAir->XMATMX(zoneNum)[2];
            state.dataRoomAir->XMATMX(zoneNum)[3] = state.dataRoomAir->XMATMX(zoneNum)[3];
        }

        if (state.dataRoomAir->AirModel(zoneNum).AirModel == RoomAir::RoomAirModel::AirflowNetwork) {
            for (auto &afnNode : state.dataRoomAir->AFNZoneInfo(zoneNum).Node) {
                afnNode.AirTempX[0] = afnNode.AirTempX[1];
                afnNode.AirTempX[1] = afnNode.AirTempX[2];
                afnNode.AirTempX[2] = afnNode.AirTempX[3];

                afnNode.HumRatX[0] = afnNode.HumRatX[1];
                afnNode.HumRatX[1] = afnNode.HumRatX[2];
                afnNode.HumRatX[2] = afnNode.HumRatX[3];
            }
        }
    }
}

void ZoneSpaceHeatBalanceData::correctHumRat(EnergyPlusData &state, int const zoneNum, int const spaceNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   2000
    // REFERENCES: Routine FinalZnCalcs - FINAL ZONE CALCULATIONS, authored by Dale Herron for BLAST.

    assert(zoneNum > 0);
    static constexpr std::string_view RoutineName("correctHumRat");

    Real64 MoistureMassFlowRate = 0.0;
    Real64 ZoneMassFlowRate = 0.0;
    auto &zone = state.dataHeatBal->Zone(zoneNum);
    int ZoneMult = zone.Multiplier * zone.ListMultiplier;
    bool ControlledZoneAirFlag = zone.IsControlled;
    bool ZoneRetPlenumAirFlag = zone.IsReturnPlenum;
    bool ZoneSupPlenumAirFlag = zone.IsSupplyPlenum;

    if (ControlledZoneAirFlag) { // If there is system flow then calculate the flow rates
        auto &zoneEquipConfig = state.dataZoneEquip->ZoneEquipConfig(zoneNum);
        // Calculate moisture flow rate into each zone
        for (int NodeNum = 1; NodeNum <= zoneEquipConfig.NumInletNodes; ++NodeNum) {
            auto const &inletNode = state.dataLoopNodes->Node(zoneEquipConfig.InletNode(NodeNum));
            MoistureMassFlowRate += (inletNode.MassFlowRate * inletNode.HumRat) / ZoneMult;
            ZoneMassFlowRate += inletNode.MassFlowRate / ZoneMult;
        }

        // Do the calculations for the plenum zone
    } else if (ZoneRetPlenumAirFlag) {
        int ZoneRetPlenumNum = zone.PlenumCondNum;
        auto &zoneRetPlenCond = state.dataZonePlenum->ZoneRetPlenCond(ZoneRetPlenumNum);
        for (int NodeNum = 1; NodeNum <= zoneRetPlenCond.NumInletNodes; ++NodeNum) {
            auto const &inletNode = state.dataLoopNodes->Node(zoneRetPlenCond.InletNode(NodeNum));
            MoistureMassFlowRate += (inletNode.MassFlowRate * inletNode.HumRat) / ZoneMult;
            ZoneMassFlowRate += inletNode.MassFlowRate / ZoneMult;
        }
        // add in the leak flow
        for (int ADUListIndex = 1; ADUListIndex <= zoneRetPlenCond.NumADUs; ++ADUListIndex) {
            int ADUNum = zoneRetPlenCond.ADUIndex(ADUListIndex);
            auto const &airDistUnit = state.dataDefineEquipment->AirDistUnit(ADUNum);
            if (airDistUnit.UpStreamLeak) {
                int ADUInNode = airDistUnit.InletNodeNum;
                MoistureMassFlowRate += (airDistUnit.MassFlowRateUpStrLk * state.dataLoopNodes->Node(ADUInNode).HumRat) / ZoneMult;
                ZoneMassFlowRate += airDistUnit.MassFlowRateUpStrLk / ZoneMult;
            }
            if (airDistUnit.DownStreamLeak) {
                int ADUOutNode = airDistUnit.OutletNodeNum;
                MoistureMassFlowRate += (airDistUnit.MassFlowRateDnStrLk * state.dataLoopNodes->Node(ADUOutNode).HumRat) / ZoneMult;
                ZoneMassFlowRate += airDistUnit.MassFlowRateDnStrLk / ZoneMult;
            }
        }

    } else if (ZoneSupPlenumAirFlag) {
        int ZoneSupPlenumNum = zone.PlenumCondNum;
        auto const &inletNode = state.dataLoopNodes->Node(state.dataZonePlenum->ZoneSupPlenCond(ZoneSupPlenumNum).InletNode);
        MoistureMassFlowRate += (inletNode.MassFlowRate * inletNode.HumRat) / ZoneMult;
        ZoneMassFlowRate += inletNode.MassFlowRate / ZoneMult;
    }

    // Calculate hourly humidity ratio from infiltration + humidity added from latent load + system added moisture
    Real64 LatentGain = this->latentGain + state.dataHeatBalFanSys->SumLatentHTRadSys(zoneNum) + state.dataHeatBalFanSys->SumLatentPool(zoneNum);

    Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

    // Calculate the coefficients for the 3rd order derivative for final
    // zone humidity ratio.  The A, B, C coefficients are analogous to the
    // heat balance.  There are 2 cases that should be considered, system
    // operating and system shutdown.

    Real64 const RhoAir = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, this->ZT, this->airHumRat, RoutineName);
    Real64 const H2OHtOfVap = Psychrometrics::PsyHgAirFnWTdb(this->airHumRat, this->ZT);

    Real64 B = (LatentGain / H2OHtOfVap) + ((this->OAMFL + this->VAMFL + this->CTMFL) * state.dataEnvrn->OutHumRat) + this->EAMFLxHumRat +
               (MoistureMassFlowRate) + this->SumHmARaW + this->MixingMassFlowXHumRat + this->MDotOA * state.dataEnvrn->OutHumRat;
    Real64 A = ZoneMassFlowRate + this->OAMFL + this->VAMFL + this->EAMFL + this->CTMFL + this->SumHmARa + this->MixingMassFlowZone + this->MDotOA;

    if (state.afn->multizone_always_simulated ||
        (state.afn->simulation_control.type == AirflowNetwork::ControlType::MultizoneWithDistributionOnlyDuringFanOperation &&
         state.afn->AirflowNetworkFanActivated)) {
        auto const &exchangeData = state.afn->exchangeData(zoneNum);
        // Multizone airflow calculated in AirflowNetwork
        B = (LatentGain / H2OHtOfVap) + (exchangeData.SumMHrW + exchangeData.SumMMHrW) + (MoistureMassFlowRate) + this->SumHmARaW;
        A = ZoneMassFlowRate + exchangeData.SumMHr + exchangeData.SumMMHr + this->SumHmARa;
    }
    Real64 C = RhoAir * zone.Volume * zone.ZoneVolCapMultpMoist / TimeStepSysSec;

    if (state.afn->distribution_simulated) {
        B += state.afn->exchangeData(zoneNum).TotalLat;
    }

    // Use a 3rd order derivative to predict final zone humidity ratio and
    // smooth the changes using the zone air capacitance.
    // auto &zoneAirHumRatTemp = this->ZoneAirHumRatTemp;
    // auto &zoneW1 = s_ztpc->zoneHeatBalance(ZoneNum).ZoneW1;
    switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
    case DataHeatBalance::SolutionAlgo::ThirdOrder: {
        this->airHumRatTemp =
            (B + C * (3.0 * this->WPrevZoneTSTemp[0] - (3.0 / 2.0) * this->WPrevZoneTSTemp[1] + (1.0 / 3.0) * this->WPrevZoneTSTemp[2])) /
            ((11.0 / 6.0) * C + A);
        // Exact solution
    } break;
    case DataHeatBalance::SolutionAlgo::AnalyticalSolution: {
        if (A == 0.0) { // B=0
            this->airHumRatTemp = this->W1 + B / C;
        } else {
            this->airHumRatTemp = (this->W1 - B / A) * std::exp(min(700.0, -A / C)) + B / A;
        }
    } break;
    case DataHeatBalance::SolutionAlgo::EulerMethod: {
        this->airHumRatTemp = (C * this->W1 + B) / (C + A);
    } break;
    default:
        break;
    }

    // Set the humidity ratio to zero if the zone has been dried out
    if (this->airHumRatTemp < 0.0) {
        this->airHumRatTemp = 0.0;
    }

    // Check to make sure that is saturated there is condensation in the zone
    // by resetting to saturation conditions.
    Real64 const WZSat = Psychrometrics::PsyWFnTdbRhPb(state, this->ZT, 1.0, state.dataEnvrn->OutBaroPress, RoutineName);

    if (this->airHumRatTemp > WZSat) {
        this->airHumRatTemp = WZSat;
    }

    if (state.dataRoomAir->AirModel(zoneNum).AirModel == RoomAir::RoomAirModel::AirflowNetwork) {
        this->airHumRatTemp = state.dataRoomAir->AFNZoneInfo(zoneNum).Node(state.dataRoomAir->AFNZoneInfo(zoneNum).ControlAirNodeID).HumRat;
    }

    // HybridModel with measured humidity ratio begins
    // SpaceHB TODO: For now, hybrid model is only for zones
    if (spaceNum == 0 && state.dataHybridModel->FlagHybridModel) {
        auto &hmZone = state.dataHybridModel->hybridModelZones(zoneNum);
        if ((hmZone.InfiltrationCalc_H || hmZone.PeopleCountCalc_H) && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing)) {
            Real64 LatentGainExceptPeople = 0.0;
            if (hmZone.PeopleCountCalc_H) {
                LatentGainExceptPeople = this->latentGainExceptPeople + state.dataHeatBalFanSys->SumLatentHTRadSys(zoneNum) +
                                         state.dataHeatBalFanSys->SumLatentPool(zoneNum);
            }

            InverseModelHumidity(state, zoneNum, LatentGain, LatentGainExceptPeople, ZoneMassFlowRate, MoistureMassFlowRate, H2OHtOfVap, RhoAir);
        }
    }

    // Now put the calculated info into the actual zone nodes; ONLY if there is zone air flow, i.e. controlled zone or plenum zone
    int ZoneNodeNum = zone.SystemZoneNodeNumber;
    if (spaceNum > 0) {
        ZoneNodeNum = state.dataHeatBal->space(spaceNum).SystemZoneNodeNumber;
    }
    if (ZoneNodeNum > 0) {
        state.dataLoopNodes->Node(ZoneNodeNum).HumRat = this->airHumRatTemp;
        state.dataLoopNodes->Node(ZoneNodeNum).Enthalpy = Psychrometrics::PsyHFnTdbW(this->ZT, this->airHumRatTemp);
    }
    if (state.dataHeatBal->DoLatentSizing) {
        Real64 sensibleLoad = 0.0;
        Real64 pSat = Psychrometrics::PsyPsatFnTemp(state, this->ZT, RoutineName);
        Real64 Tdp = Psychrometrics::PsyTdpFnWPb(state, this->airHumRatTemp, state.dataEnvrn->StdBaroPress);
        Real64 vaporPressureDiff = pSat - Psychrometrics::PsyPsatFnTemp(state, Tdp, RoutineName);
        if (spaceNum > 0) {
            sensibleLoad = state.dataZoneEnergyDemand->spaceSysEnergyDemand(spaceNum).airSysHeatRate +
                           state.dataZoneEnergyDemand->spaceSysEnergyDemand(spaceNum).airSysCoolRate;
            state.dataZoneEnergyDemand->spaceSysMoistureDemand(spaceNum).reportZoneAirSystemMoistureLoads(
                state, LatentGain, sensibleLoad, vaporPressureDiff);
        } else {
            sensibleLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(zoneNum).airSysHeatRate +
                           state.dataZoneEnergyDemand->ZoneSysEnergyDemand(zoneNum).airSysCoolRate;
            state.dataZoneEnergyDemand->ZoneSysMoistureDemand(zoneNum).reportZoneAirSystemMoistureLoads(
                state, LatentGain, sensibleLoad, vaporPressureDiff);
        }
    }
}

void DownInterpolate4HistoryValues(Real64 const OldTimeStep,
                                   Real64 const NewTimeStep,
                                   Real64 const oldVal0,
                                   Real64 const oldVal1,
                                   Real64 const oldVal2,
                                   Real64 &newVal0,
                                   Real64 &newVal1,
                                   Real64 &newVal2,
                                   Real64 &newVal3,
                                   Real64 &newVal4)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Feb 2008

    // PURPOSE OF THIS SUBROUTINE:
    // provide a reusable routine for the various places that need to
    // interpolate a new set of history values on a different time scale
    // Once the systemtimestep has shortened, the new history terms need to be interpolated

    // METHODOLOGY EMPLOYED:
    // This routine assumes that the direction is to a shorter timestep.
    // The down step ratio, DSRatio = OldTimeStep/ NewTimeStep
    //  is expected to be roughly integer-valued and near 2.0 or 3.0 or 4.0 or more.

    // old math variables
    // Real64 const oldTime0 = 0.0;
    // Real64 const oldTime1 = oldTime0 - OldTimeStep;
    // Real64 const newTime0 = 0.0;
    // Real64 const newTime1 = newTime0 - NewTimeStep;
    // Real64 const newTime2 = newTime1 - NewTimeStep;
    // Real64 const newTime3 = newTime2 - NewTimeStep;
    // Real64 const newTime4 = newTime3 - NewTimeStep;

    Real64 constexpr realTWO = 2.0;
    Real64 constexpr realTHREE = 3.0;
    // first determine the ratio of system time step to zone time step
    Real64 const DSRatio = OldTimeStep / NewTimeStep; // should pretty much be an integer value 2, 3, 4, etc.

    newVal0 = oldVal0;

    if (std::abs(DSRatio - realTWO) < 0.01) { // DSRatio = 2
        // when DSRatio = 2 the 1st point lies exactly between old points, and 2nd point is old 1st point
        // first two points lie between oldVal0 and oldVal1
        // old math example
        // newVal1 = oldVal0 + (oldVal1 - oldVal0) * ((oldTime0 - newTime1) / (OldTimeStep));
        // newVal2 = oldVal0 + (oldVal1 - oldVal0) * ((oldTime0 - newTime2) / (OldTimeStep));
        newVal1 = (oldVal0 + oldVal1) / realTWO;
        newVal2 = oldVal1;
        // when DSRatio = 2 the 3rd point lies exactly between old points, and 4th point is old 2nd point
        // last two points lie between oldVal1 and oldVal2
        // newVal3 = oldVal1 + (oldVal2 - oldVal1) * ((oldTime1 - newTime3) / (OldTimeStep));
        // newVal4 = oldVal1 + (oldVal2 - oldVal1) * ((oldTime1 - newTime4) / (OldTimeStep));
        newVal3 = (oldVal1 + oldVal2) / realTWO;
        newVal4 = oldVal2;
    } else if (std::abs(DSRatio - realTHREE) < 0.01) { // DSRatio = 3
        // when DSRatio = 3 the 1st point lies 1/3 way between old points, and 2nd and 3rd points are 2/3 and 3/3 the way
        // first three points lie between oldVal0 and oldVal1
        // newVal1 = oldVal0 + (oldVal1 - oldVal0) * ((oldTime0 - newTime1) / (OldTimeStep));
        // newVal2 = oldVal0 + (oldVal1 - oldVal0) * ((oldTime0 - newTime2) / (OldTimeStep));
        // newVal3 = oldVal0 + (oldVal1 - oldVal0) * ((oldTime0 - newTime3) / (OldTimeStep));
        Real64 delta10 = (oldVal1 - oldVal0) / realTHREE;
        newVal1 = oldVal0 + delta10;
        newVal2 = newVal1 + delta10;
        newVal3 = oldVal1;
        // last point lies 1/3 way between oldVal1 and oldVal2
        // newVal4 = oldVal1 + (oldVal2 - oldVal1) * ((oldTime1 - newTime4) / (OldTimeStep));
        newVal4 = oldVal1 + (oldVal2 - oldVal1) / realTHREE;

    } else { // DSRatio = 4 or more
        // all new points lie between oldVal0 and oldVal1 (if DSRatio = 4, newVal4 = oldVal1)
        // newVal1 = oldVal0 + (oldVal1 - oldVal0) * ((oldTime0 - newTime1) / (OldTimeStep));
        // newVal2 = oldVal0 + (oldVal1 - oldVal0) * ((oldTime0 - newTime2) / (OldTimeStep));
        // newVal3 = oldVal0 + (oldVal1 - oldVal0) * ((oldTime0 - newTime3) / (OldTimeStep));
        // newVal4 = oldVal0 + (oldVal1 - oldVal0) * ((oldTime0 - newTime4) / (OldTimeStep));
        Real64 delta10 = (oldVal1 - oldVal0) / DSRatio;
        newVal1 = oldVal0 + delta10;
        newVal2 = newVal1 + delta10;
        newVal3 = newVal2 + delta10;
        newVal4 = newVal3 + delta10;
    }
}

Real64 DownInterpolate4HistoryValues(Real64 OldTimeStep, Real64 NewTimeStep, std::array<Real64, 4> const &oldVals, std::array<Real64, 4> &newVals)
{
    Real64 constexpr realTWO = 2.0;
    Real64 constexpr realTHREE = 3.0;
    // first determine the ratio of system time step to zone time step
    Real64 const DSRatio = OldTimeStep / NewTimeStep; // should pretty much be an integer value 2, 3, 4, etc.

    newVals[0] = oldVals[0];

    if (std::abs(DSRatio - realTWO) < 0.01) { // DSRatio = 2
        // first point lies exactly between (oldVals[0] and oldVals[1])
        newVals[1] = (oldVals[0] + oldVals[1]) / realTWO;
        // 2nd point is oldVal[1] and last point lies exactly between (oldVals[1] and oldVals[2])
        newVals[2] = oldVals[1];
        newVals[3] = (oldVals[1] + oldVals[2]) / realTWO;

    } else if (std::abs(DSRatio - realTHREE) < 0.01) { // DSRatio = 3
        // first two points lie between (oldVals[0] and oldVals[1])
        Real64 delta10 = (oldVals[1] - oldVals[0]) / realTHREE;
        newVals[1] = oldVals[0] + delta10;
        newVals[2] = newVals[1] + delta10;
        // last point is oldVals[1]
        newVals[3] = oldVals[1];

    } else { // DSRatio = 4 or more
        // all new points lie between (oldVals[0] and oldVals[1])
        Real64 delta10 = (oldVals[1] - oldVals[0]) / DSRatio;
        newVals[1] = oldVals[0] + delta10;
        newVals[2] = newVals[1] + delta10;
        newVals[3] = newVals[2] + delta10;
    }
    return oldVals[0];
}
void InverseModelTemperature(EnergyPlusData &state,
                             int const ZoneNum,                   // Zone number
                             Real64 const SumIntGain,             // Zone sum of convective internal gains
                             Real64 const SumIntGainExceptPeople, // Zone sum of convective internal gains except for people
                             Real64 const SumHA,                  // Zone sum of Hc*Area
                             Real64 const SumHATsurf,             // Zone sum of Hc*Area*Tsurf
                             Real64 const SumHATref,              // Zone sum of Hc*Area*Tref, for ceiling diffuser convection correlation
                             Real64 const SumMCp,                 // Zone sum of MassFlowRate*Cp
                             Real64 const SumMCpT,                // Zone sum of MassFlowRate*Cp*T
                             Real64 const SumSysMCp,              // Zone sum of air system MassFlowRate*Cp
                             Real64 const SumSysMCpT,             // Zone sum of air system MassFlowRate*Cp*T
                             Real64 const AirCap                  // Formerly CoefAirrat, coef in zone temp eqn with dim of "air power capacity"rd
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Han Li
    //       DATE WRITTEN   February 2019

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine inversely solve infiltration airflow rate or people count with zone air temperatures measurements.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 AirCapHM(0.0); // Air power capacity for hybrid modeling
    Real64 AA(0.0);
    Real64 BB(0.0);
    Real64 FractionConvection(0.0); // Default convection portion of the sensible heat from people

    auto &s_ztpc = state.dataZoneTempPredictorCorrector;
    auto &zone = state.dataHeatBal->Zone(ZoneNum);
    auto &hmZone = state.dataHybridModel->hybridModelZones(ZoneNum);
    auto &thisZoneHB = s_ztpc->zoneHeatBalance(ZoneNum);

    int ZoneMult = zone.Multiplier * zone.ListMultiplier;
    zone.ZoneMeasuredTemperature = (hmZone.measuredTempSched != nullptr) ? hmZone.measuredTempSched->getCurrentVal() : 0.0;
    zone.ZoneVolCapMultpSensHM = 1.0; // Initialize to 1.0 in case hybrid not active

    // HM calculation only HM calculation period start
    if (state.dataEnvrn->DayOfYear >= hmZone.HybridStartDayOfYear && state.dataEnvrn->DayOfYear <= hmZone.HybridEndDayOfYear) {
        Real64 MultpHM(1.0);

        thisZoneHB.ZT = zone.ZoneMeasuredTemperature; // Array1D<Real64> ZT -- Zone
                                                      // Air Temperature Averaged over
                                                      // the System Time Increment
        if (hmZone.InfiltrationCalc_T && state.dataHVACGlobal->UseZoneTimeStepHistory) {
            static constexpr std::string_view RoutineNameInfiltration("CalcAirFlowSimple:Infiltration");

            if (hmZone.IncludeSystemSupplyParameters) {
                zone.ZoneMeasuredSupplyAirTemperature = hmZone.supplyAirTempSched->getCurrentVal();
                zone.ZoneMeasuredSupplyAirFlowRate = hmZone.supplyAirMassFlowRateSched ? hmZone.supplyAirMassFlowRateSched->getCurrentVal() : 0.0;
                zone.ZoneMeasuredSupplyAirHumidityRatio = hmZone.supplyAirHumRatSched ? hmZone.supplyAirHumRatSched->getCurrentVal() : 0.0;
                // Calculate the air humidity ratio at supply air inlet.
                Real64 CpAirInlet(0.0);
                CpAirInlet = Psychrometrics::PsyCpAirFnW(zone.ZoneMeasuredSupplyAirHumidityRatio);

                Real64 SumSysMCp_HM = zone.ZoneMeasuredSupplyAirFlowRate * CpAirInlet;
                Real64 SumSysMCpT_HM = zone.ZoneMeasuredSupplyAirFlowRate * CpAirInlet * zone.ZoneMeasuredSupplyAirTemperature;

                AA = SumSysMCp_HM + SumHA + thisZoneHB.MCPV + thisZoneHB.MCPM + thisZoneHB.MCPE + thisZoneHB.MCPC + thisZoneHB.MDotCPOA;
                BB = SumSysMCpT_HM + SumIntGain + SumHATsurf - SumHATref + thisZoneHB.MCPTV + thisZoneHB.MCPTM + thisZoneHB.MCPTE + thisZoneHB.MCPTC +
                     thisZoneHB.MDotCPOA * zone.OutDryBulbTemp + (thisZoneHB.NonAirSystemResponse / ZoneMult + thisZoneHB.SysDepZoneLoadsLagged);
            } else {
                AA = SumHA + thisZoneHB.MCPV + thisZoneHB.MCPM + thisZoneHB.MCPE + thisZoneHB.MCPC + thisZoneHB.MDotCPOA;
                BB = SumIntGain + SumHATsurf - SumHATref + thisZoneHB.MCPTV + thisZoneHB.MCPTM + thisZoneHB.MCPTE + thisZoneHB.MCPTC +
                     thisZoneHB.MDotCPOA * zone.OutDryBulbTemp;
            }
            Real64 CC = AirCap;
            Real64 DD =
                (3.0 * state.dataHeatBalFanSys->PreviousMeasuredZT1(ZoneNum) - (3.0 / 2.0) * state.dataHeatBalFanSys->PreviousMeasuredZT2(ZoneNum) +
                 (1.0 / 3.0) * state.dataHeatBalFanSys->PreviousMeasuredZT3(ZoneNum));

            Real64 delta_T = (zone.ZoneMeasuredTemperature - zone.OutDryBulbTemp);
            Real64 CpAir = Psychrometrics::PsyCpAirFnW(state.dataEnvrn->OutHumRat);
            Real64 AirDensity = Psychrometrics::PsyRhoAirFnPbTdbW(
                state, state.dataEnvrn->OutBaroPress, zone.OutDryBulbTemp, state.dataEnvrn->OutHumRat, RoutineNameInfiltration);
            zone.delta_T = delta_T;

            // s4 - Set ACH to 0 when delta_T <= 0.5, add max and min limits to ach
            Real64 M_inf = 0.0;
            if (std::abs(delta_T) > 0.5) {
                M_inf = (BB + CC * DD - ((11.0 / 6.0) * CC + AA) * zone.ZoneMeasuredTemperature) / (CpAir * delta_T);
            }
            Real64 ACH_inf = max(0.0, min(10.0, (M_inf / AirDensity) / zone.Volume * Constant::rSecsInHour));
            M_inf = (ACH_inf / Constant::rSecsInHour) * zone.Volume * AirDensity;

            // Overwrite variable with inverse solution
            zone.MCPIHM = M_inf;
            zone.InfilOAAirChangeRateHM = ACH_inf;

        } // Hybrid model infiltration calculation end

        // Hybrid modeling internal thermal mass calculation start
        if (hmZone.InternalThermalMassCalc_T && SumSysMCpT == 0 && thisZoneHB.ZT != state.dataHeatBalFanSys->PreviousMeasuredZT1(ZoneNum) &&
            state.dataHVACGlobal->UseZoneTimeStepHistory) { // HM calculation only when SumSysMCpT =0,
                                                            // TimeStepZone (not @ TimeStepSys)
            Real64 TempDepCoef = SumHA + SumMCp + SumSysMCp;
            Real64 TempIndCoef = SumIntGain + SumHATsurf - SumHATref + SumMCpT + SumSysMCpT +
                                 (thisZoneHB.NonAirSystemResponse / ZoneMult + thisZoneHB.SysDepZoneLoadsLagged);
            //    TempHistoryTerm = AirCap * (3.0 * ZTM1(ZoneNum) - (3.0/2.0) * ZTM2(ZoneNum) + (1.0/3.0) * ZTM3(ZoneNum)) !debug only

            if (state.afn->distribution_simulated) {
                TempIndCoef += state.afn->exchangeData(ZoneNum).TotalSen;
            }
            // Calculate air capacity using DataHeatBalance::SolutionAlgo::AnalyticalSolution
            if (TempDepCoef == 0.0) {
                // Is this correct? Shouldn't we use log?? What if thisZT ==
                // PreviousMeasuredZT1(ZoneNum)??
                AirCapHM = TempIndCoef / (thisZoneHB.ZT - state.dataHeatBalFanSys->PreviousMeasuredZT1(ZoneNum)); // Inverse equation
            } else {
                Real64 AirCapHM_temp = 0.0;
                if (TempIndCoef == TempDepCoef * thisZoneHB.ZT) {
                    AirCapHM_temp = 0.0; //  This is the denominator.
                } else {
                    AirCapHM_temp = (TempIndCoef - TempDepCoef * state.dataHeatBalFanSys->PreviousMeasuredZT1(ZoneNum)) /
                                    (TempIndCoef - TempDepCoef * thisZoneHB.ZT);
                }

                if ((AirCapHM_temp > 0) && (AirCapHM_temp != 1)) {    // Avoid IND
                    AirCapHM = TempDepCoef / std::log(AirCapHM_temp); // Inverse equation
                } else {
                    AirCapHM = TempIndCoef / (thisZoneHB.ZT - state.dataHeatBalFanSys->PreviousMeasuredZT1(ZoneNum));
                }
            }

            // Calculate multiplier
            if (std::abs(thisZoneHB.ZT - state.dataHeatBalFanSys->PreviousMeasuredZT1(ZoneNum)) > 0.05) { // Filter
                MultpHM = AirCapHM /
                          (zone.Volume *
                           Psychrometrics::PsyRhoAirFnPbTdbW(state,
                                                             state.dataEnvrn->OutBaroPress,
                                                             thisZoneHB.ZT,
                                                             thisZoneHB.airHumRat) *
                           Psychrometrics::PsyCpAirFnW(thisZoneHB.airHumRat)) *
                          (state.dataGlobal->TimeStepZone * Constant::rSecsInHour); // Inverse equation
            } else {
                MultpHM = 1.0; // Default value 1.0
            }

            processInverseModelMultpHM(
                state, MultpHM, zone.ZoneVolCapMultpSensHMSum, zone.ZoneVolCapMultpSensHMCountSum, zone.ZoneVolCapMultpSensHMAverage, ZoneNum);
            zone.ZoneVolCapMultpSensHM = MultpHM;

        } // Hybrid model internal thermal mass calculation end

        // Hybrid model people count calculation
        if (hmZone.PeopleCountCalc_T && state.dataHVACGlobal->UseZoneTimeStepHistory) {
            zone.ZoneMeasuredTemperature = hmZone.measuredTempSched->getCurrentVal();
            zone.ZonePeopleActivityLevel = hmZone.peopleActivityLevelSched ? hmZone.peopleActivityLevelSched->getCurrentVal() : 0.0;
            zone.ZonePeopleSensibleHeatFraction = hmZone.peopleSensibleFracSched ? hmZone.peopleSensibleFracSched->getCurrentVal() : 0.0;
            zone.ZonePeopleRadiantHeatFraction = hmZone.peopleRadiantFracSched ? hmZone.peopleRadiantFracSched->getCurrentVal() : 0.0;

            Real64 FractionSensible = zone.ZonePeopleSensibleHeatFraction;
            Real64 FractionRadiation = zone.ZonePeopleRadiantHeatFraction;
            Real64 ActivityLevel = hmZone.peopleActivityLevelSched ? hmZone.peopleActivityLevelSched->getCurrentVal() : 0.0;

            if (FractionSensible <= 0.0) {
                FractionSensible = 0.6;
            }

            if (FractionRadiation <= 0.0) {
                FractionConvection = 0.7;
            } else {
                FractionConvection = 1.0 - FractionRadiation;
            }

            if (ActivityLevel <= 0.0) {
                ActivityLevel = 130.0;
            }

            if (hmZone.IncludeSystemSupplyParameters) {
                zone.ZoneMeasuredSupplyAirTemperature = hmZone.supplyAirTempSched->getCurrentVal();
                zone.ZoneMeasuredSupplyAirFlowRate = hmZone.supplyAirMassFlowRateSched ? hmZone.supplyAirMassFlowRateSched->getCurrentVal() : 0.0;
                zone.ZoneMeasuredSupplyAirHumidityRatio = hmZone.supplyAirHumRatSched ? hmZone.supplyAirHumRatSched->getCurrentVal() : 0.0;

                // Calculate the air humidity ratio at supply air inlet.
                Real64 CpAirInlet = Psychrometrics::PsyCpAirFnW(zone.ZoneMeasuredSupplyAirHumidityRatio);

                Real64 SumSysMCp_HM = zone.ZoneMeasuredSupplyAirFlowRate * CpAirInlet;
                Real64 SumSysMCpT_HM = zone.ZoneMeasuredSupplyAirFlowRate * CpAirInlet * zone.ZoneMeasuredSupplyAirTemperature;

                AA = SumSysMCp_HM + SumHA + SumMCp;
                BB = SumSysMCpT_HM + SumIntGainExceptPeople + SumHATsurf - SumHATref + SumMCpT +
                     (thisZoneHB.NonAirSystemResponse / ZoneMult + thisZoneHB.SysDepZoneLoadsLagged);
            } else {
                AA = SumHA + SumMCp;
                BB = SumIntGainExceptPeople + SumHATsurf - SumHATref + SumMCpT;
            }

            Real64 CC = AirCap;
            Real64 DD =
                (3.0 * state.dataHeatBalFanSys->PreviousMeasuredZT1(ZoneNum) - (3.0 / 2.0) * state.dataHeatBalFanSys->PreviousMeasuredZT2(ZoneNum) +
                 (1.0 / 3.0) * state.dataHeatBalFanSys->PreviousMeasuredZT3(ZoneNum));

            Real64 SumIntGainPeople = ((11.0 / 6.0) * CC + AA) * zone.ZoneMeasuredTemperature - BB - CC * DD;
            Real64 UpperBound = max(0.0, SumIntGain / (ActivityLevel * FractionSensible * FractionConvection));
            Real64 NumPeople = min(UpperBound, max(0.0, SumIntGainPeople / (ActivityLevel * FractionSensible * FractionConvection)));

            if (NumPeople < 0.05) {
                NumPeople = 0;
            }
            zone.NumOccHM = NumPeople;
        }
    }

    // Update zone temperatures in the previous steps
    state.dataHeatBalFanSys->PreviousMeasuredZT3(ZoneNum) = state.dataHeatBalFanSys->PreviousMeasuredZT2(ZoneNum);
    state.dataHeatBalFanSys->PreviousMeasuredZT2(ZoneNum) = state.dataHeatBalFanSys->PreviousMeasuredZT1(ZoneNum);
    state.dataHeatBalFanSys->PreviousMeasuredZT1(ZoneNum) = thisZoneHB.ZT;
}

void processInverseModelMultpHM(EnergyPlusData &state,
                                Real64 &multiplierHM, // Hybrid model thermal mass multiplier
                                Real64 &multSumHM,    // Sum of Hybrid model thermal mass multipliers
                                Real64 &countSumHM,   // Count of number of points in sum
                                Real64 &multAvgHM,    // Average of hybrid model mass multiplier
                                int zoneNum           // Zone number for the hybrid model
)
{
    Real64 constexpr minHMMultValue = 1.0;
    Real64 constexpr maxHMMultValue = 30.0;

    auto &s_ztpc = state.dataZoneTempPredictorCorrector;
    auto &zone = state.dataHeatBal->Zone(zoneNum);
    auto &thisZoneHB = s_ztpc->zoneHeatBalance(zoneNum);

    // Apply limits and generate warnings as needed
    if (multiplierHM < minHMMultValue) { // don't allow this to be less than minimum (potential for instability)
        multiplierHM = minHMMultValue;
    } else if (multiplierHM > maxHMMultValue) { // as per suggestions in Defect #10508, only warn if greater than the max
        if (thisZoneHB.hmThermalMassMultErrIndex == 0) {
            ShowWarningMessage(state, format("Hybrid model thermal mass multiplier higher than the limit for {}", zone.Name));
            ShowContinueError(state, "This means that the ratio of the zone air heat capacity for the current time step to the");
            ShowContinueError(state, format("zone air heat storage is higher than the maximum limit of {:.1R}.", maxHMMultValue));
        }
        ShowRecurringWarningErrorAtEnd(
            state, "Hybrid model thermal mass multiplier limit exceeded in zone " + zone.Name, thisZoneHB.hmThermalMassMultErrIndex);
    }

    // Update running totals (but only when there is a valid multiplier, i.e. multiplier is greater than min but not higher than the max)
    if (multiplierHM > minHMMultValue) {
        multSumHM += multiplierHM;
        countSumHM++;
    }

    // Calculate average (always so that it does get calculated)
    if (countSumHM >= 1) {
        multAvgHM = multSumHM / countSumHM;
    }
}

void InverseModelHumidity(EnergyPlusData &state,
                          int const ZoneNum,                   // Zone number
                          Real64 const LatentGain,             // Zone sum of latent gain
                          Real64 const LatentGainExceptPeople, // Zone sum of latent gain except for people
                          Real64 const ZoneMassFlowRate,       // Zone air mass flow rate
                          Real64 const MoistureMassFlowRate,   // Zone moisture mass flow rate
                          Real64 const H2OHtOfVap,             // Heat of vaporization of air
                          Real64 const RhoAir                  // Air density
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Han Li
    //       DATE WRITTEN   February 2019

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine inversely solve infiltration airflow rate or people count with zone air humidity measurements.

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("InverseModelHumidity");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 AA(0.0);
    Real64 BB(0.0);
    Real64 ActivityLevel(0.0);
    Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

    auto &s_ztpc = state.dataZoneTempPredictorCorrector;
    auto &zone = state.dataHeatBal->Zone(ZoneNum);
    auto &hmZone = state.dataHybridModel->hybridModelZones(ZoneNum);
    auto &thisZoneHB = s_ztpc->zoneHeatBalance(ZoneNum);

    // Get measured zone humidity ratio
    zone.ZoneMeasuredHumidityRatio = hmZone.measuredHumRatSched->getCurrentVal();

    if (state.dataEnvrn->DayOfYear >= hmZone.HybridStartDayOfYear && state.dataEnvrn->DayOfYear <= hmZone.HybridEndDayOfYear) {
        thisZoneHB.airHumRat = zone.ZoneMeasuredHumidityRatio;

        // Hybrid Model calculate air infiltration rate
        if (hmZone.InfiltrationCalc_H && state.dataHVACGlobal->UseZoneTimeStepHistory) {
            // Conditionally calculate the time dependent and time independent terms
            if (hmZone.IncludeSystemSupplyParameters) {
                zone.ZoneMeasuredSupplyAirFlowRate = hmZone.supplyAirMassFlowRateSched->getCurrentVal();
                zone.ZoneMeasuredSupplyAirHumidityRatio = hmZone.supplyAirHumRatSched->getCurrentVal();

                Real64 SumSysM_HM = zone.ZoneMeasuredSupplyAirFlowRate;
                Real64 SumSysMHumRat_HM = zone.ZoneMeasuredSupplyAirFlowRate * zone.ZoneMeasuredSupplyAirHumidityRatio;

                AA = SumSysM_HM + thisZoneHB.VAMFL + thisZoneHB.EAMFL + thisZoneHB.CTMFL + thisZoneHB.SumHmARa + thisZoneHB.MixingMassFlowZone +
                     thisZoneHB.MDotOA;
                BB = SumSysMHumRat_HM + (LatentGain / H2OHtOfVap) + ((thisZoneHB.VAMFL + thisZoneHB.CTMFL) * state.dataEnvrn->OutHumRat) +
                     thisZoneHB.EAMFLxHumRat + thisZoneHB.SumHmARaW + thisZoneHB.MixingMassFlowXHumRat +
                     thisZoneHB.MDotOA * state.dataEnvrn->OutHumRat;
            } else {
                AA = thisZoneHB.VAMFL + thisZoneHB.EAMFL + thisZoneHB.CTMFL + thisZoneHB.SumHmARa + thisZoneHB.MixingMassFlowZone + thisZoneHB.MDotOA;
                BB = (LatentGain / H2OHtOfVap) + ((thisZoneHB.VAMFL + thisZoneHB.CTMFL) * state.dataEnvrn->OutHumRat) + thisZoneHB.EAMFLxHumRat +
                     thisZoneHB.SumHmARaW + thisZoneHB.MixingMassFlowXHumRat + thisZoneHB.MDotOA * state.dataEnvrn->OutHumRat;
            }

            Real64 CC = RhoAir * zone.Volume * zone.ZoneVolCapMultpMoist / TimeStepSysSec;
            Real64 DD = (3.0 * state.dataHeatBalFanSys->PreviousMeasuredHumRat1(ZoneNum) -
                         (3.0 / 2.0) * state.dataHeatBalFanSys->PreviousMeasuredHumRat2(ZoneNum) +
                         (1.0 / 3.0) * state.dataHeatBalFanSys->PreviousMeasuredHumRat3(ZoneNum));

            Real64 delta_HR = (zone.ZoneMeasuredHumidityRatio - state.dataEnvrn->OutHumRat);

            Real64 AirDensity =
                Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, zone.OutDryBulbTemp, state.dataEnvrn->OutHumRat, RoutineName);

            Real64 M_inf = 0.0;
            if (std::abs(zone.ZoneMeasuredHumidityRatio - state.dataEnvrn->OutHumRat) > 0.0000001) {
                M_inf = (CC * DD + BB - ((11.0 / 6.0) * CC + AA) * zone.ZoneMeasuredHumidityRatio) / delta_HR;
            }

            // Add threshold for air change rate
            Real64 ACH_inf = max(0.0, min(10.0, (M_inf / AirDensity) / zone.Volume * Constant::rSecsInHour));
            M_inf = (ACH_inf / Constant::rSecsInHour) * zone.Volume * AirDensity;
            zone.MCPIHM = M_inf;
            zone.InfilOAAirChangeRateHM = ACH_inf;
        }

        // Hybrid Model calculate people count
        if (hmZone.PeopleCountCalc_H && state.dataHVACGlobal->UseZoneTimeStepHistory) {
            zone.ZonePeopleActivityLevel = hmZone.peopleActivityLevelSched ? hmZone.peopleActivityLevelSched->getCurrentVal() : 0.0;
            zone.ZonePeopleSensibleHeatFraction = hmZone.peopleSensibleFracSched ? hmZone.peopleSensibleFracSched->getCurrentVal() : 0.0;
            zone.ZonePeopleRadiantHeatFraction = hmZone.peopleRadiantFracSched ? hmZone.peopleRadiantFracSched->getCurrentVal() : 0.0;

            Real64 FractionSensible = zone.ZonePeopleSensibleHeatFraction;

            if (FractionSensible <= 0.0) {
                FractionSensible = 0.6;
            }

            if (ActivityLevel <= 0.0) {
                ActivityLevel = 130.0;
            }

            // Conditionally calculate the humidity-dependent and humidity-independent
            // terms.
            if (hmZone.IncludeSystemSupplyParameters) {
                zone.ZoneMeasuredSupplyAirFlowRate = hmZone.supplyAirMassFlowRateSched->getCurrentVal();
                zone.ZoneMeasuredSupplyAirHumidityRatio = hmZone.supplyAirHumRatSched->getCurrentVal();

                Real64 SumSysM_HM = zone.ZoneMeasuredSupplyAirFlowRate;
                Real64 SumSysMHumRat_HM = zone.ZoneMeasuredSupplyAirFlowRate * zone.ZoneMeasuredSupplyAirHumidityRatio;

                AA = SumSysM_HM + thisZoneHB.OAMFL + thisZoneHB.VAMFL + thisZoneHB.EAMFL + thisZoneHB.CTMFL + thisZoneHB.SumHmARa +
                     thisZoneHB.MixingMassFlowZone + thisZoneHB.MDotOA;
                BB = SumSysMHumRat_HM + (LatentGainExceptPeople / H2OHtOfVap) +
                     ((thisZoneHB.OAMFL + thisZoneHB.VAMFL + thisZoneHB.CTMFL) * state.dataEnvrn->OutHumRat) + thisZoneHB.EAMFLxHumRat +
                     thisZoneHB.SumHmARaW + thisZoneHB.MixingMassFlowXHumRat + thisZoneHB.MDotOA * state.dataEnvrn->OutHumRat;
            } else {
                AA = ZoneMassFlowRate + thisZoneHB.OAMFL + thisZoneHB.VAMFL + thisZoneHB.EAMFL + thisZoneHB.CTMFL + thisZoneHB.SumHmARa +
                     thisZoneHB.MixingMassFlowZone + thisZoneHB.MDotOA;
                BB = (LatentGainExceptPeople / H2OHtOfVap) + ((thisZoneHB.OAMFL + thisZoneHB.VAMFL + thisZoneHB.CTMFL) * state.dataEnvrn->OutHumRat) +
                     thisZoneHB.EAMFLxHumRat + (MoistureMassFlowRate) + thisZoneHB.SumHmARaW + thisZoneHB.MixingMassFlowXHumRat +
                     thisZoneHB.MDotOA * state.dataEnvrn->OutHumRat;
            }

            Real64 CC = RhoAir * zone.Volume * zone.ZoneVolCapMultpMoist / TimeStepSysSec;
            Real64 DD = (3.0 * state.dataHeatBalFanSys->PreviousMeasuredHumRat1(ZoneNum) -
                         (3.0 / 2.0) * state.dataHeatBalFanSys->PreviousMeasuredHumRat2(ZoneNum) +
                         (1.0 / 3.0) * state.dataHeatBalFanSys->PreviousMeasuredHumRat3(ZoneNum));

            Real64 LatentGainPeople = (((11.0 / 6.0) * CC + AA) * zone.ZoneMeasuredHumidityRatio - BB - CC * DD) * H2OHtOfVap;
            Real64 UpperBound = max(0.0, LatentGain / (ActivityLevel * (1.0 - FractionSensible)));
            Real64 NumPeople = min(UpperBound, max(0.0, LatentGainPeople / (ActivityLevel * (1.0 - FractionSensible))));
            NumPeople = floor(NumPeople * 100.00 + 0.5) / 100.00;
            if (NumPeople < 0.05) {
                NumPeople = 0;
            }
            zone.NumOccHM = NumPeople;
        }
    }

    // Update zone humidity ratio in the previous steps
    state.dataHeatBalFanSys->PreviousMeasuredHumRat3(ZoneNum) = state.dataHeatBalFanSys->PreviousMeasuredHumRat2(ZoneNum);
    state.dataHeatBalFanSys->PreviousMeasuredHumRat2(ZoneNum) = state.dataHeatBalFanSys->PreviousMeasuredHumRat1(ZoneNum);
    state.dataHeatBalFanSys->PreviousMeasuredHumRat1(ZoneNum) = zone.ZoneMeasuredHumidityRatio;
}

void ZoneSpaceHeatBalanceData::calcZoneOrSpaceSums(EnergyPlusData &state,
                                                   bool const CorrectorFlag, // Corrector call flag
                                                   int const zoneNum,
                                                   int const spaceNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   July 2003
    //       MODIFIED       Aug 2003, FCW: add this->SumHA contributions from window frame and divider
    //                      Aug 2003, CC: change how the reference temperatures are used

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine calculates the various sums that go into the zone heat balance
    // equation.  This replaces the SUMC, SumHA, and SumHAT calculations that were
    // previously done in various places throughout the program.
    // The SumHAT portion of the code is reproduced in RadiantSystemHighTemp and
    // RadiantSystemLowTemp and should be updated accordingly.
    // A reference temperature (Tref) is specified for use with the ceiling diffuser
    // convection correlation.  A bogus value of Tref = -999.9 defaults to using
    // the zone air (i.e. outlet) temperature for the reference temperature.
    // If Tref is applied to all surfaces, SumHA = 0, and SumHATref /= 0.
    // If Tref is not used at all, SumHATref = 0, and SumHA /= 0.
    // For future implementations, Tref can be easily converted into an array to
    // allow a different reference temperature to be specified for each surface.
    assert(zoneNum > 0);

    this->SumHA = 0.0;
    this->SumHATsurf = 0.0;
    this->SumHATref = 0.0;
    this->SumSysMCp = 0.0;
    this->SumSysMCpT = 0.0;
    // Sum all convective internal gains: this->SumIntGain
    if (spaceNum == 0) {
        this->SumIntGain = InternalHeatGains::zoneSumAllInternalConvectionGains(state, zoneNum);
    } else {
        this->SumIntGain = InternalHeatGains::spaceSumAllInternalConvectionGains(state, spaceNum);
    }
    this->SumIntGain += state.dataHeatBalFanSys->SumConvHTRadSys(zoneNum) + state.dataHeatBalFanSys->SumConvPool(zoneNum);

    // Add heat to return air if zonal system (no return air) or cycling system (return air frequently very low or zero)
    assert(zoneNum > 0);
    auto &thisZone = state.dataHeatBal->Zone(zoneNum);
    if (thisZone.NoHeatToReturnAir) {
        if (spaceNum == 0) {
            this->SumIntGain += InternalHeatGains::zoneSumAllReturnAirConvectionGains(state, zoneNum, 0);
        } else {
            this->SumIntGain += InternalHeatGains::spaceSumAllReturnAirConvectionGains(state, spaceNum, 0);
        }
    }

    // Sum all non-system air flow, i.e. infiltration, simple ventilation, mixing, earth tube: this->SumMCp, this->SumMCpT
    this->SumMCp = this->MCPI + this->MCPV + this->MCPM + this->MCPE + this->MCPC + this->MDotCPOA;
    this->SumMCpT = this->MCPTI + this->MCPTV + this->MCPTM + this->MCPTE + this->MCPTC + this->MDotCPOA * thisZone.OutDryBulbTemp;

    // Sum all multizone air flow calculated from AirflowNetwork by assuming no simple air infiltration model
    if (state.afn->multizone_always_simulated ||
        (state.afn->simulation_control.type == AirflowNetwork::ControlType::MultizoneWithDistributionOnlyDuringFanOperation &&
         state.afn->AirflowNetworkFanActivated)) {
        auto const &exchangeData = state.afn->exchangeData(zoneNum);
        this->SumMCp = exchangeData.SumMCp + exchangeData.SumMVCp + exchangeData.SumMMCp;
        this->SumMCpT = exchangeData.SumMCpT + exchangeData.SumMVCpT + exchangeData.SumMMCpT;
    }

    // Sum all system air flow: this->SumSysMCp, this->SumSysMCpT and check to see if this is a controlled zone
    // If the space is controlled, use space supply nodes, otherwise use zone supply nodes and allocate later
    bool isSpaceControlled = (spaceNum > 0 && state.dataZoneEquip->spaceEquipConfig(spaceNum).IsControlled);
    if (CorrectorFlag) {
        // Plenum and controlled zones have a different set of inlet nodes which must be calculated.
        if (thisZone.IsControlled) {
            auto const &zsec = (isSpaceControlled ? state.dataZoneEquip->spaceEquipConfig(spaceNum) : state.dataZoneEquip->ZoneEquipConfig(zoneNum));
            for (int NodeNum = 1, NodeNum_end = zsec.NumInletNodes; NodeNum <= NodeNum_end; ++NodeNum) {
                // Get node conditions, this next block is of interest to irratic system loads... maybe nodes are not accurate at time of call?
                //  how can we tell?  predict step must be lagged ?  correct step, systems have run.
                auto const &node(state.dataLoopNodes->Node(zsec.InletNode(NodeNum)));
                Real64 CpAir = Psychrometrics::PsyCpAirFnW(this->airHumRat);
                Real64 const MassFlowRate_CpAir(node.MassFlowRate * CpAir);
                this->SumSysMCp += MassFlowRate_CpAir;
                this->SumSysMCpT += MassFlowRate_CpAir * node.Temp;
            }

        } else if (thisZone.IsReturnPlenum) {
            auto const &zrpc(state.dataZonePlenum->ZoneRetPlenCond(thisZone.PlenumCondNum));
            Real64 const air_hum_rat(this->airHumRat);
            for (int NodeNum = 1, NodeNum_end = zrpc.NumInletNodes; NodeNum <= NodeNum_end; ++NodeNum) {
                auto const &node(state.dataLoopNodes->Node(zrpc.InletNode(NodeNum)));
                Real64 const MassFlowRate_CpAir(node.MassFlowRate * Psychrometrics::PsyCpAirFnW(air_hum_rat));
                this->SumSysMCp += MassFlowRate_CpAir;
                this->SumSysMCpT += MassFlowRate_CpAir * node.Temp;
            }
            // add in the leaks
            for (int ADUListIndex = 1, ADUListIndex_end = zrpc.NumADUs; ADUListIndex <= ADUListIndex_end; ++ADUListIndex) {
                auto &airDistUnit = state.dataDefineEquipment->AirDistUnit(zrpc.ADUIndex(ADUListIndex));
                if (airDistUnit.UpStreamLeak) {
                    Real64 const MassFlowRate_CpAir(airDistUnit.MassFlowRateUpStrLk * Psychrometrics::PsyCpAirFnW(air_hum_rat));
                    this->SumSysMCp += MassFlowRate_CpAir;
                    this->SumSysMCpT += MassFlowRate_CpAir * state.dataLoopNodes->Node(airDistUnit.InletNodeNum).Temp;
                }
                if (airDistUnit.DownStreamLeak) {
                    Real64 const MassFlowRate_CpAir(airDistUnit.MassFlowRateDnStrLk * Psychrometrics::PsyCpAirFnW(air_hum_rat));
                    this->SumSysMCp += MassFlowRate_CpAir;
                    this->SumSysMCpT += MassFlowRate_CpAir * state.dataLoopNodes->Node(airDistUnit.OutletNodeNum).Temp;
                }
            }

        } else if (thisZone.IsSupplyPlenum) {
            Real64 MassFlowRate = state.dataLoopNodes->Node(state.dataZonePlenum->ZoneSupPlenCond(thisZone.PlenumCondNum).InletNode).MassFlowRate;
            Real64 CpAir = Psychrometrics::PsyCpAirFnW(this->airHumRat);
            this->SumSysMCp += MassFlowRate * CpAir;
            this->SumSysMCpT +=
                MassFlowRate * CpAir * state.dataLoopNodes->Node(state.dataZonePlenum->ZoneSupPlenCond(thisZone.PlenumCondNum).InletNode).Temp;
        }

        int ZoneMult = thisZone.Multiplier * thisZone.ListMultiplier;

        this->SumSysMCp /= ZoneMult;
        this->SumSysMCpT /= ZoneMult;
    }

    if (spaceNum > 0 && !isSpaceControlled) {
        // If space is not controlled, allocate zone-level airflow by volume
        Real64 spaceFrac = state.dataHeatBal->space(spaceNum).fracZoneVolume;
        this->SumSysMCp *= spaceFrac;
        this->SumSysMCpT *= spaceFrac;
    }

    // Sum all surface convection: this->SumHA, this->SumHATsurf, this->SumHATref (and additional contributions to this->SumIntGain)
    SumHATOutput sumHATResults; // space or zone return values
    sumHATResults = this->calcSumHAT(state, zoneNum, spaceNum);
    this->SumIntGain += sumHATResults.sumIntGain;
    this->SumHA = sumHATResults.sumHA;
    this->SumHATsurf = sumHATResults.sumHATsurf;
    this->SumHATref = sumHATResults.sumHATref;
}

SumHATOutput ZoneHeatBalanceData::calcSumHAT(EnergyPlusData &state, int const zoneNum, [[maybe_unused]] int const spaceNum)
{
    auto &s_ztpc = state.dataZoneTempPredictorCorrector;
    assert(zoneNum > 0);
    assert(spaceNum == 0);
    SumHATOutput zoneResults; // zone-level return values
    for (int zoneSpaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
        SumHATOutput spaceResults; // temporary return value from space-level calcSumHAT
        spaceResults = s_ztpc->spaceHeatBalance(zoneSpaceNum).calcSumHAT(state, zoneNum, zoneSpaceNum);
        zoneResults.sumIntGain += spaceResults.sumIntGain;
        zoneResults.sumHA += spaceResults.sumHA;
        zoneResults.sumHATsurf += spaceResults.sumHATsurf;
        zoneResults.sumHATref += spaceResults.sumHATref;
    }
    return zoneResults;
}

SumHATOutput SpaceHeatBalanceData::calcSumHAT(EnergyPlusData &state, int const zoneNum, int const spaceNum)
{
    assert(zoneNum > 0);
    assert(spaceNum > 0);
    auto &s_ztpc = state.dataZoneTempPredictorCorrector;
    auto &thisZone = state.dataHeatBal->Zone(zoneNum);
    auto &thisSpace = state.dataHeatBal->space(spaceNum);
    SumHATOutput results; // space-level return values

    for (int SurfNum = thisSpace.HTSurfaceFirst; SurfNum <= thisSpace.HTSurfaceLast; ++SurfNum) {
        Real64 HA = 0.0;
        Real64 Area = state.dataSurface->Surface(SurfNum).Area; // For windows, this is the glazing area

        if (state.dataSurface->Surface(SurfNum).Class == DataSurfaces::SurfaceClass::Window) {
            DataSurfaces::WinShadingType const shading_flag = state.dataSurface->SurfWinShadingFlag(SurfNum);

            // Add to the convective internal gains
            if (ANY_INTERIOR_SHADE_BLIND(shading_flag)) {
                // The shade area covers the area of the glazing plus the area of the dividers.
                Area += state.dataSurface->SurfWinDividerArea(SurfNum);
                // If interior shade or blind is present it is assumed that both the convective and IR radiative gain
                // from the inside surface of the divider goes directly into the zone air -- i.e., the IR radiative
                // interaction between divider and shade or blind is ignored due to the difficulty of calculating this interaction
                // at the same time that the interaction between glass and shade is calculated.
                results.sumIntGain += state.dataSurface->SurfWinDividerHeatGain(SurfNum);
            }

            // Other convection term is applicable to equivalent layer window (ASHWAT) model
            if (state.dataConstruction->Construct(state.dataSurface->Surface(SurfNum).Construction).WindowTypeEQL) {
                results.sumIntGain += state.dataSurface->SurfWinOtherConvHeatGain(SurfNum);
            }

            // Convective heat gain from natural convection in gap between glass and interior shade or blind
            if (ANY_INTERIOR_SHADE_BLIND(shading_flag)) {
                results.sumIntGain += state.dataSurface->SurfWinConvHeatFlowNatural(SurfNum);
            }

            // Convective heat gain from airflow window
            if (state.dataSurface->SurfWinAirflowThisTS(SurfNum) > 0.0) {
                results.sumIntGain += state.dataSurface->SurfWinConvHeatGainToZoneAir(SurfNum);
                if (thisZone.NoHeatToReturnAir) {
                    results.sumIntGain += state.dataSurface->SurfWinRetHeatGainToZoneAir(SurfNum);
                    state.dataSurface->SurfWinHeatGain(SurfNum) += state.dataSurface->SurfWinRetHeatGainToZoneAir(SurfNum);
                    if (state.dataSurface->SurfWinHeatGain(SurfNum) >= 0.0) {
                        state.dataSurface->SurfWinHeatGainRep(SurfNum) = state.dataSurface->SurfWinHeatGain(SurfNum);
                        state.dataSurface->SurfWinHeatGainRepEnergy(SurfNum) =
                            state.dataSurface->SurfWinHeatGainRep(SurfNum) * state.dataGlobal->TimeStepZoneSec;
                    } else {
                        state.dataSurface->SurfWinHeatLossRep(SurfNum) = -state.dataSurface->SurfWinHeatGain(SurfNum);
                        state.dataSurface->SurfWinHeatLossRepEnergy(SurfNum) =
                            state.dataSurface->SurfWinHeatLossRep(SurfNum) * state.dataGlobal->TimeStepZoneSec;
                    }
                    state.dataSurface->SurfWinHeatTransferRepEnergy(SurfNum) =
                        state.dataSurface->SurfWinHeatGain(SurfNum) * state.dataGlobal->TimeStepZoneSec;
                }
            }

            // Add to the surface convection sums
            if (state.dataSurface->SurfWinFrameArea(SurfNum) > 0.0) {
                // Window frame contribution
                Real64 const HA_surf(state.dataHeatBalSurf->SurfHConvInt(SurfNum) * state.dataSurface->SurfWinFrameArea(SurfNum) *
                                     (1.0 + state.dataSurface->SurfWinProjCorrFrIn(SurfNum)));
                results.sumHATsurf += HA_surf * state.dataSurface->SurfWinFrameTempIn(SurfNum);
                HA += HA_surf;
            }

            if (state.dataSurface->SurfWinDividerArea(SurfNum) > 0.0 && !ANY_INTERIOR_SHADE_BLIND(shading_flag)) {
                // Window divider contribution (only from shade or blind for window with divider and interior shade or blind)
                Real64 const HA_surf(state.dataHeatBalSurf->SurfHConvInt(SurfNum) * state.dataSurface->SurfWinDividerArea(SurfNum) *
                                     (1.0 + 2.0 * state.dataSurface->SurfWinProjCorrDivIn(SurfNum)));
                results.sumHATsurf += HA_surf * state.dataSurface->SurfWinDividerTempIn(SurfNum);
                HA += HA_surf;
            }

        } // End of check if window

        HA += state.dataHeatBalSurf->SurfHConvInt(SurfNum) * Area;
        results.sumHATsurf += state.dataHeatBalSurf->SurfHConvInt(SurfNum) * Area * state.dataHeatBalSurf->SurfTempInTmp(SurfNum);

        // determine reference air temperature for this surface
        switch (state.dataSurface->SurfTAirRef(SurfNum)) {
        case DataSurfaces::RefAirTemp::ZoneMeanAirTemp:
            // The zone air is the reference temperature (which is to be solved for in CorrectZoneAirTemp).
            results.sumHA += HA;
            break;
        case DataSurfaces::RefAirTemp::AdjacentAirTemp:
            results.sumHATref += HA * state.dataHeatBal->SurfTempEffBulkAir(SurfNum);
            break;
        case DataSurfaces::RefAirTemp::ZoneSupplyAirTemp:
            // check whether this zone is a controlled zone or not
            if (!thisZone.IsControlled) {
                ShowFatalError(state,
                               format("Zones must be controlled for Ceiling-Diffuser Convection model. No system serves zone {}", thisZone.Name));
                return results;
            }
            // determine supply air temperature as a weighted average of the inlet temperatures.
            // TODO: For now, use zone-level values for system flow
            if (s_ztpc->zoneHeatBalance(zoneNum).SumSysMCp > 0.0) {
                results.sumHATref += HA * s_ztpc->zoneHeatBalance(zoneNum).SumSysMCpT / s_ztpc->zoneHeatBalance(zoneNum).SumSysMCp;
            } else {
                // no system flow (yet) so just use zone air temperature #5906
                results.sumHA += HA;
            }
            break;
        default:
            // currently set to mean air temp but should add error warning here
            results.sumHA += HA;
            break;
        }

    } // SurfNum
    return results;
}
void CalcZoneComponentLoadSums(EnergyPlusData &state,
                               int ZoneNum, // Zone number
                               ZoneTempPredictorCorrector::ZoneSpaceHeatBalanceData *thisHB,
                               DataHeatBalance::AirReportVars &thisAirRpt)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Feb 2008

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine calculates the various sums that go into the zone heat balance
    // equation for reporting (and diagnostic) purposes only.
    // It was derived from CalcZonethisAirRpt.Sums but differs in that that routine
    // breaks up the component's dependence on zone air temp in order to *solve* for zone air temp,
    // but here we *use* the result for zone air temp and calculate the terms of the heat balance
    // Go back and calculate each of the 6 terms in Equation 5 and fill report variables.
    // notes on these raw terms for zone air heat balance model :
    //  these are state variables at the end of the last system timestep.
    //  they are not necessarily proper averages for what happened over entire zone time step
    //  these are not multiplied by zone multipliers.
    //  The values are all Watts.

    // REFERENCES:
    // Equation 5 in Engineering Reference.

    thisAirRpt.SumIntGains = 0.0;    // Zone sum of convective internal gains
    thisAirRpt.SumHADTsurfs = 0.0;   // Zone sum of Hc*Area*(Tsurf - Tz)
    thisAirRpt.SumMCpDTzones = 0.0;  // zone sum of MassFlowRate*cp*(TremotZone - Tz) transfer air from other zone, Mixing
    thisAirRpt.SumMCpDtInfil = 0.0;  // Zone sum of MassFlowRate*Cp*(Tout - Tz)
    thisAirRpt.SumMCpDTsystem = 0.0; // Zone sum of air system MassFlowRate*Cp*(Tsup - Tz)
    thisAirRpt.SumNonAirSystem = 0.0;
    thisAirRpt.CzdTdt = 0.0;
    thisAirRpt.imBalance = 0.0;
    thisAirRpt.SumEnthalpyM = 0.0;
    thisAirRpt.SumEnthalpyH = 0.0;

    auto &thisZone = state.dataHeatBal->Zone(ZoneNum);

    Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

    // Sum all convective internal gains: SumIntGain
    thisAirRpt.SumIntGains = InternalHeatGains::zoneSumAllInternalConvectionGains(state, ZoneNum);

    // Add heat to return air if zonal system (no return air) or cycling system (return air frequently very
    // low or zero)
    if (thisZone.NoHeatToReturnAir) {
        thisAirRpt.SumIntGains += InternalHeatGains::zoneSumAllReturnAirConvectionGains(state, ZoneNum, 0);
    }

    // sum non-system air flow transfers between zones
    thisAirRpt.SumMCpDTzones = thisHB->MCPTM - thisHB->MCPM * thisHB->MAT; // but maybe it should be ZTAV(ZoneNum)

    // Sum non-system air flow, i.e. infiltration, simple ventilation, earth tube
    //  reuse SumMCp, SumMCpT from CalcZoneSum but use MAT (or maybe ZTAV?) to complete
    thisAirRpt.SumMCpDtInfil = (thisHB->MCPTI - thisHB->MCPI * thisHB->MAT) + (thisHB->MCPTV - thisHB->MCPV * thisHB->MAT) +
                               (thisHB->MCPTE - thisHB->MCPE * thisHB->MAT) + (thisHB->MCPTC - thisHB->MCPC * thisHB->MAT) +
                               (thisHB->MDotCPOA * thisZone.OutDryBulbTemp -
                                thisHB->MDotCPOA * thisHB->MAT); // infiltration | Ventilation (simple) | Earth tube. | Cooltower | combined OA flow

    // Sum all multizone air flow calculated from AirflowNetwork by assuming no simple air infiltration model (if used)
    if (state.afn->multizone_always_simulated ||
        (state.afn->simulation_control.type == AirflowNetwork::ControlType::MultizoneWithDistributionOnlyDuringFanOperation &&
         state.afn->AirflowNetworkFanActivated)) {
        // Multizone airflow calculated in AirflowNetwork
        thisAirRpt.SumMCpDtInfil = state.afn->exchangeData(ZoneNum).SumMCpT + state.afn->exchangeData(ZoneNum).SumMVCpT -
                                   (state.afn->exchangeData(ZoneNum).SumMCp + state.afn->exchangeData(ZoneNum).SumMVCp) * thisHB->MAT;
        thisAirRpt.SumMCpDTzones = state.afn->exchangeData(ZoneNum).SumMMCpT - state.afn->exchangeData(ZoneNum).SumMMCp * thisHB->MAT;
    }

    // Sum all system air flow: reusing how SumSysMCp, SumSysMCpT are calculated in CalcZoneSums
    // Plenum and controlled zones have a different set of inlet nodes which must be calculated.
    Real64 QSensRate = 0.0;
    if (thisZone.IsControlled) {
        auto &zoneEquipConfig = state.dataZoneEquip->ZoneEquipConfig(ZoneNum);
        for (int NodeNum = 1; NodeNum <= zoneEquipConfig.NumInletNodes; ++NodeNum) {
            // Get node conditions
            Real64 const NodeTemp = state.dataLoopNodes->Node(zoneEquipConfig.InletNode(NodeNum)).Temp;
            Real64 const MassFlowRate = state.dataLoopNodes->Node(zoneEquipConfig.InletNode(NodeNum)).MassFlowRate;
            QSensRate = calcZoneSensibleOutput(MassFlowRate, NodeTemp, thisHB->MAT, thisHB->airHumRat);
            thisAirRpt.SumMCpDTsystem += QSensRate;

            if (zoneEquipConfig.InletNodeADUNum(NodeNum) > 0) {
                auto &airDistUnit = state.dataDefineEquipment->AirDistUnit(zoneEquipConfig.InletNodeADUNum(NodeNum));
                Real64 ADUHeatAddRate = calcZoneSensibleOutput(state.dataLoopNodes->Node(airDistUnit.OutletNodeNum).MassFlowRate,
                                                               state.dataLoopNodes->Node(airDistUnit.OutletNodeNum).Temp,
                                                               thisHB->MAT,
                                                               thisHB->airHumRat);
                airDistUnit.HeatRate = max(0.0, ADUHeatAddRate);
                airDistUnit.CoolRate = std::abs(min(0.0, ADUHeatAddRate));
                airDistUnit.HeatGain = airDistUnit.HeatRate * TimeStepSysSec;
                airDistUnit.CoolGain = airDistUnit.CoolRate * TimeStepSysSec;
            }
        }

    } else if (thisZone.IsReturnPlenum) {
        auto &zoneRetPlenCond = state.dataZonePlenum->ZoneRetPlenCond(thisZone.PlenumCondNum);
        for (int NodeNum = 1; NodeNum <= zoneRetPlenCond.NumInletNodes; ++NodeNum) {
            QSensRate = calcZoneSensibleOutput(state.dataLoopNodes->Node(zoneRetPlenCond.InletNode(NodeNum)).MassFlowRate,
                                               state.dataLoopNodes->Node(zoneRetPlenCond.InletNode(NodeNum)).Temp,
                                               thisHB->MAT,
                                               thisHB->airHumRat);
            thisAirRpt.SumMCpDTsystem += QSensRate;
        }
        // add in the leaks
        for (int ADUListIndex = 1; ADUListIndex <= zoneRetPlenCond.NumADUs; ++ADUListIndex) {
            auto &airDistUnit = state.dataDefineEquipment->AirDistUnit(zoneRetPlenCond.ADUIndex(ADUListIndex));
            if (airDistUnit.UpStreamLeak) {
                QSensRate = calcZoneSensibleOutput(
                    airDistUnit.MassFlowRateUpStrLk, state.dataLoopNodes->Node(airDistUnit.InletNodeNum).Temp, thisHB->MAT, thisHB->airHumRat);
                thisAirRpt.SumMCpDTsystem += QSensRate;
            }
            if (airDistUnit.DownStreamLeak) {
                QSensRate = calcZoneSensibleOutput(
                    airDistUnit.MassFlowRateDnStrLk, state.dataLoopNodes->Node(airDistUnit.OutletNodeNum).Temp, thisHB->MAT, thisHB->airHumRat);
                thisAirRpt.SumMCpDTsystem += QSensRate;
            }
        }

    } else if (thisZone.IsSupplyPlenum) {
        auto &zoneSupPlenCond = state.dataZonePlenum->ZoneSupPlenCond(thisZone.PlenumCondNum);
        QSensRate = calcZoneSensibleOutput(state.dataLoopNodes->Node(zoneSupPlenCond.InletNode).MassFlowRate,
                                           state.dataLoopNodes->Node(zoneSupPlenCond.InletNode).Temp,
                                           thisHB->MAT,
                                           thisHB->airHumRat);
        thisAirRpt.SumMCpDTsystem += QSensRate;
    }

    // non air system response.
    thisAirRpt.SumNonAirSystem =
        thisHB->NonAirSystemResponse + state.dataHeatBalFanSys->SumConvHTRadSys(ZoneNum) + state.dataHeatBalFanSys->SumConvPool(ZoneNum);

    // Sum all surface convection: SumHA, SumHATsurf, SumHATref (and additional contributions to SumIntGain)
    for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
        auto const &thisSpace = state.dataHeatBal->space(spaceNum);
        for (int SurfNum = thisSpace.HTSurfaceFirst; SurfNum <= thisSpace.HTSurfaceLast; ++SurfNum) {

            Real64 Area = state.dataSurface->Surface(SurfNum).Area; // For windows, this is the glazing area
            Real64 RefAirTemp = state.dataSurface->Surface(SurfNum).getInsideAirTemperature(state, SurfNum);

            if (state.dataSurface->Surface(SurfNum).Class == DataSurfaces::SurfaceClass::Window) {

                // Add to the convective internal gains
                if (ANY_INTERIOR_SHADE_BLIND(state.dataSurface->SurfWinShadingFlag(SurfNum))) {
                    // The shade area covers the area of the glazing plus the area of the dividers.
                    Area += state.dataSurface->SurfWinDividerArea(SurfNum);
                    // If interior shade or blind is present it is assumed that both the convective and IR radiative gain
                    // from the inside surface of the divider goes directly into the zone air -- i.e., the IR radiative
                    // interaction between divider and shade or blind is ignored due to the difficulty of calculating this interaction
                    // at the same time that the interaction between glass and shade is calculated.
                    thisAirRpt.SumIntGains += state.dataSurface->SurfWinDividerHeatGain(SurfNum);
                }

                // Other convection term is applicable to equivalent layer window (ASHWAT) model
                if (state.dataConstruction->Construct(state.dataSurface->Surface(SurfNum).Construction).WindowTypeEQL) {
                    thisAirRpt.SumIntGains += state.dataSurface->SurfWinOtherConvHeatGain(SurfNum);
                }

                // Convective heat gain from natural convection in gap between glass and interior shade or blind
                if (ANY_INTERIOR_SHADE_BLIND(state.dataSurface->SurfWinShadingFlag(SurfNum))) {
                    thisAirRpt.SumIntGains += state.dataSurface->SurfWinConvHeatFlowNatural(SurfNum);
                }

                // Convective heat gain from airflow window
                if (state.dataSurface->SurfWinAirflowThisTS(SurfNum) > 0.0) {
                    thisAirRpt.SumIntGains += state.dataSurface->SurfWinConvHeatGainToZoneAir(SurfNum);
                    if (thisZone.NoHeatToReturnAir) {
                        thisAirRpt.SumIntGains += state.dataSurface->SurfWinRetHeatGainToZoneAir(SurfNum);
                    }
                }

                // Add to the surface convection sums
                if (state.dataSurface->SurfWinFrameArea(SurfNum) > 0.0) {
                    // Window frame contribution
                    thisAirRpt.SumHADTsurfs += state.dataHeatBalSurf->SurfHConvInt(SurfNum) * state.dataSurface->SurfWinFrameArea(SurfNum) *
                                               (1.0 + state.dataSurface->SurfWinProjCorrFrIn(SurfNum)) *
                                               (state.dataSurface->SurfWinFrameTempIn(SurfNum) - RefAirTemp);
                }

                if (state.dataSurface->SurfWinDividerArea(SurfNum) > 0.0 &&
                    !ANY_INTERIOR_SHADE_BLIND(state.dataSurface->SurfWinShadingFlag(SurfNum))) {
                    // Window divider contribution (only from shade or blind for window with divider and interior shade or blind)
                    thisAirRpt.SumHADTsurfs += state.dataHeatBalSurf->SurfHConvInt(SurfNum) * state.dataSurface->SurfWinDividerArea(SurfNum) *
                                               (1.0 + 2.0 * state.dataSurface->SurfWinProjCorrDivIn(SurfNum)) *
                                               (state.dataSurface->SurfWinDividerTempIn(SurfNum) - RefAirTemp);
                }

            } // End of check if window

            thisAirRpt.SumHADTsurfs +=
                state.dataHeatBalSurf->SurfHConvInt(SurfNum) * Area * (state.dataHeatBalSurf->SurfTempInTmp(SurfNum) - RefAirTemp);

            // Accumulate Zone Phase Change Material Melting/Freezing Enthalpy output variables
            if (state.dataSurface->Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CondFD) {
                thisAirRpt.SumEnthalpyM += state.dataHeatBalFiniteDiffMgr->SurfaceFD(SurfNum).EnthalpyM;
                thisAirRpt.SumEnthalpyH += state.dataHeatBalFiniteDiffMgr->SurfaceFD(SurfNum).EnthalpyF;
            }
        }
    }
    // now calculate air energy storage source term.
    // capacitance is volume * density * heat capacity
    Real64 CpAir = Psychrometrics::PsyCpAirFnW(thisHB->airHumRat);
    Real64 RhoAir = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, thisHB->MAT, thisHB->airHumRat);

    switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
    case DataHeatBalance::SolutionAlgo::ThirdOrder: {
        thisAirRpt.CzdTdt = RhoAir * CpAir * thisZone.Volume * thisZone.ZoneVolCapMultpSens * (thisHB->MAT - thisHB->ZTM[0]) / TimeStepSysSec;
        // Exact solution
    } break;
    case DataHeatBalance::SolutionAlgo::AnalyticalSolution: {
        thisAirRpt.CzdTdt = thisHB->TempIndCoef - thisHB->TempDepCoef * thisHB->MAT;
    } break;
    case DataHeatBalance::SolutionAlgo::EulerMethod: {
        thisAirRpt.CzdTdt = thisHB->AirPowerCap * (thisHB->MAT - thisHB->T1);
    } break;
    default:
        break;
    }

    if (state.dataGlobal->DisplayZoneAirHeatBalanceOffBalance) {
        thisAirRpt.imBalance = thisAirRpt.SumIntGains + thisAirRpt.SumHADTsurfs + thisAirRpt.SumMCpDTzones + thisAirRpt.SumMCpDtInfil +
                               thisAirRpt.SumMCpDTsystem + thisAirRpt.SumNonAirSystem - thisAirRpt.CzdTdt;

        // throw warning if seriously out of balance (this may need to be removed if too noisy... )
        // formulate dynamic threshold value based on 20% of quadrature sum of components
        Real64 Threshold = 0.2 * std::sqrt(pow_2(thisAirRpt.SumIntGains) + pow_2(thisAirRpt.SumHADTsurfs) + pow_2(thisAirRpt.SumMCpDTzones) +
                                           pow_2(thisAirRpt.SumMCpDtInfil) + pow_2(thisAirRpt.SumMCpDTsystem) + pow_2(thisAirRpt.SumNonAirSystem) +
                                           pow_2(thisAirRpt.CzdTdt));
        if ((std::abs(thisAirRpt.imBalance) > Threshold) && (!state.dataGlobal->WarmupFlag) &&
            (!state.dataGlobal->DoingSizing)) { // air balance is out by more than threshold
            if (thisZone.AirHBimBalanceErrIndex == 0) {
                ShowWarningMessage(state, format("Zone Air Heat Balance is out of balance for zone named {}", thisZone.Name));
                ShowContinueError(state, format("Zone Air Heat Balance Deviation Rate is more than {:.1R} {{W}}", Threshold));
                if (state.dataHVACGlobal->TurnFansOn) {
                    ShowContinueError(state, "Night cycle fan operation may be causing above error");
                }

                ShowContinueErrorTimeStamp(state, " Occurrence info:");
            }
            ShowRecurringWarningErrorAtEnd(state,
                                           format("Zone Air Heat Balance is out of balance ... zone named {}", thisZone.Name),
                                           thisZone.AirHBimBalanceErrIndex,
                                           std::abs(thisAirRpt.imBalance) - Threshold,
                                           std::abs(thisAirRpt.imBalance) - Threshold,
                                           _,
                                           "{W}",
                                           "{W}");
        }
    }
}

bool VerifyThermostatInZone(EnergyPlusData &state, std::string const &ZoneName) // Zone to verify
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   Feb 2005

    // PURPOSE OF THIS FUNCTION:
    // This function verifies that a zone (by name) has a Zone Control:Thermostatic object entered.

    if (state.dataZoneCtrls->GetZoneAirStatsInputFlag) {
        GetZoneAirSetPoints(state);
        state.dataZoneCtrls->GetZoneAirStatsInputFlag = false;
    }
    if (state.dataZoneCtrls->NumTempControlledZones > 0) {
        if (Util::FindItemInList(ZoneName, state.dataZoneCtrls->TempControlledZone, &DataZoneControls::ZoneTempControls::ZoneName) > 0) {
            return true;
        } else {
            return false;
        }
    }
    return false;
}

bool VerifyControlledZoneForThermostat(EnergyPlusData &state, std::string const &ZoneName) // Zone to verify
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   Mar 2007

    // PURPOSE OF THIS FUNCTION:
    // This function verifies that a zone (by name) has a ZoneHVAC:EquipmentConnections object entered.

    return (Util::FindItemInList(ZoneName, state.dataZoneEquip->ZoneEquipConfig, &DataZoneEquipment::EquipConfiguration::ZoneName) > 0);
}

void DetectOscillatingZoneTemp(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Jason Glazer
    //       DATE WRITTEN   August 2005

    // PURPOSE OF THIS SUBROUTINE:
    // Oscillating temperatures between HVAC timesteps indicate that the
    // simulation may be poor. Code is trying to be fast since the purpose
    // is to see the impact on oscillating by trying longer time steps in
    // an attempt to speed up the simulation.
    // Note that the OscillateMagnitude threshold must be less than
    // MaxZoneTempDiff since ManageHVAC keeps shortening the timestep
    // until that is reached unless it goes to less than the
    // MinTimeStepSys.
    auto &s_ztpc = state.dataZoneTempPredictorCorrector;

    // first time run allocate arrays and setup output variable
    if (s_ztpc->SetupOscillationOutputFlag) {
        s_ztpc->ZoneTempHist.allocate(4, state.dataGlobal->NumOfZones);
        s_ztpc->ZoneTempHist = 0.0;
        s_ztpc->ZoneTempOscillate.dimension(state.dataGlobal->NumOfZones, 0.0);
        s_ztpc->ZoneTempOscillateDuringOccupancy.dimension(state.dataGlobal->NumOfZones, 0.0);
        s_ztpc->ZoneTempOscillateInDeadband.dimension(state.dataGlobal->NumOfZones, 0.0);
        // set up zone by zone variables, CurrentModuleObject='Zone'
        for (int iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
            auto &zone = state.dataHeatBal->Zone(iZone);
            SetupOutputVariable(state,
                                "Zone Oscillating Temperatures Time",
                                Constant::Units::hr,
                                s_ztpc->ZoneTempOscillate(iZone),
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                zone.Name);
            SetupOutputVariable(state,
                                "Zone Oscillating Temperatures During Occupancy Time",
                                Constant::Units::hr,
                                s_ztpc->ZoneTempOscillateDuringOccupancy(iZone),
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                zone.Name);
            SetupOutputVariable(state,
                                "Zone Oscillating Temperatures in Deadband Time",
                                Constant::Units::hr,
                                s_ztpc->ZoneTempOscillateInDeadband(iZone),
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                zone.Name);
        }
        // set up a variable covering all zones
        SetupOutputVariable(state,
                            "Facility Any Zone Oscillating Temperatures Time",
                            Constant::Units::hr,
                            s_ztpc->AnyZoneTempOscillate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            "Facility");
        SetupOutputVariable(state,
                            "Facility Any Zone Oscillating Temperatures During Occupancy Time",
                            Constant::Units::hr,
                            s_ztpc->AnyZoneTempOscillateDuringOccupancy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            "Facility");
        SetupOutputVariable(state,
                            "Facility Any Zone Oscillating Temperatures in Deadband Time",
                            Constant::Units::hr,
                            s_ztpc->AnyZoneTempOscillateInDeadband,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            "Facility");
        // test if the oscillation variables are even used
        if (ReportingThisVariable(state, "Zone Oscillating Temperatures Time") ||
            ReportingThisVariable(state, "Zone Oscillating Temperatures During Occupancy Time") ||
            ReportingThisVariable(state, "Zone Oscillating Temperatures in Deadband Time") ||
            ReportingThisVariable(state, "Facility Any Zone Oscillating Temperatures Time") ||
            ReportingThisVariable(state, "Facility Any Zone Oscillating Temperatures During Occupancy Time") ||
            ReportingThisVariable(state, "Facility Any Zone Oscillating Temperatures in Deadband Time")) {
            s_ztpc->OscillationVariablesNeeded = true;
        }
        s_ztpc->SetupOscillationOutputFlag = false;
    }

    Real64 TimeStepSys = state.dataHVACGlobal->TimeStepSys;
    if (s_ztpc->OscillationVariablesNeeded) {
        // precalc the negative value for performance
        Real64 NegOscillateMagnitude = -HVAC::OscillateMagnitude;
        // assume no zone is oscillating
        bool isAnyZoneOscillating = false;
        bool isAnyZoneOscillatingDuringOccupancy = false;
        bool isAnyZoneOscillatingInDeadband = false;

        for (int iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
            bool isOscillate = false;
            s_ztpc->ZoneTempHist(4, iZone) = s_ztpc->ZoneTempHist(3, iZone);
            s_ztpc->ZoneTempHist(3, iZone) = s_ztpc->ZoneTempHist(2, iZone);
            s_ztpc->ZoneTempHist(2, iZone) = s_ztpc->ZoneTempHist(1, iZone);
            s_ztpc->ZoneTempHist(1, iZone) = s_ztpc->zoneHeatBalance(iZone).ZT;
            Real64 Diff34 = s_ztpc->ZoneTempHist(3, iZone) - s_ztpc->ZoneTempHist(4, iZone);
            Real64 Diff23 = s_ztpc->ZoneTempHist(2, iZone) - s_ztpc->ZoneTempHist(3, iZone);
            Real64 Diff12 = s_ztpc->ZoneTempHist(1, iZone) - s_ztpc->ZoneTempHist(2, iZone);
            // roll out the conditionals for increased performance
            if (Diff12 > HVAC::OscillateMagnitude) {
                if (Diff23 < NegOscillateMagnitude) {
                    if (Diff34 > HVAC::OscillateMagnitude) {
                        isOscillate = true;
                    }
                }
            }
            // now try the opposite sequence of swings
            if (Diff12 < NegOscillateMagnitude) {
                if (Diff23 > HVAC::OscillateMagnitude) {
                    if (Diff34 < NegOscillateMagnitude) {
                        isOscillate = true;
                    }
                }
            }
            s_ztpc->ZoneTempOscillateDuringOccupancy(iZone) = 0.0;
            s_ztpc->ZoneTempOscillateInDeadband(iZone) = 0.0;
            if (isOscillate) {
                s_ztpc->ZoneTempOscillate(iZone) = TimeStepSys;
                isAnyZoneOscillating = true;
                if (allocated(state.dataThermalComforts->ThermalComfortInASH55)) {
                    if (state.dataThermalComforts->ThermalComfortInASH55(iZone).ZoneIsOccupied) {
                        s_ztpc->ZoneTempOscillateDuringOccupancy(iZone) = TimeStepSys;
                        isAnyZoneOscillatingDuringOccupancy = true;
                    }
                }
                if (state.dataZoneEnergyDemand->CurDeadBandOrSetback(iZone)) {
                    s_ztpc->ZoneTempOscillateInDeadband(iZone) = TimeStepSys;
                    isAnyZoneOscillatingInDeadband = true;
                }
            } else {
                s_ztpc->ZoneTempOscillate(iZone) = 0.0;
            }
        }
        // any zone variable
        s_ztpc->AnyZoneTempOscillate = (isAnyZoneOscillating) ? TimeStepSys : 0.0;
        s_ztpc->AnyZoneTempOscillateDuringOccupancy = (isAnyZoneOscillatingDuringOccupancy) ? TimeStepSys : 0.0;
        s_ztpc->AnyZoneTempOscillateInDeadband = (isAnyZoneOscillatingInDeadband) ? TimeStepSys : 0.0;

        // annual/runperiod sum for _perflog.csv file
        s_ztpc->AnnualAnyZoneTempOscillate += s_ztpc->AnyZoneTempOscillate;
        s_ztpc->AnnualAnyZoneTempOscillateDuringOccupancy += s_ztpc->AnyZoneTempOscillateDuringOccupancy;
        s_ztpc->AnnualAnyZoneTempOscillateInDeadband += s_ztpc->AnyZoneTempOscillateInDeadband;
    }
}

void AdjustAirSetPointsforOpTempCntrl(EnergyPlusData &state, int const TempControlledZoneID, int const ActualZoneNum, Real64 &ZoneAirSetPoint)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   June 2006

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine modifies the air temperature setpoint to effect operative temperature control

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    auto &s_ztpc = state.dataZoneTempPredictorCorrector;
    Real64 thisMRTFraction; // local variable for fraction that MRT is in Op Temp definition

    if (!(state.dataZoneCtrls->AnyOpTempControl)) {
        return; // do nothing to setpoint
    }

    auto &tempControlledZone = state.dataZoneCtrls->TempControlledZone(TempControlledZoneID);
    if (tempControlledZone.OpTempCtrl == DataZoneControls::TempCtrl::None) {
        return; // do nothing to setpoint
    }

    // is operative temp radiative fraction scheduled or fixed?
    thisMRTFraction = (tempControlledZone.OpTempCtrl == DataZoneControls::TempCtrl::Scheduled)
                          ? tempControlledZone.opTempRadiativeFractionSched->getCurrentVal()
                          : tempControlledZone.FixedRadiativeFraction;

    // get mean radiant temperature for zone
    Real64 thisMRT = s_ztpc->zoneHeatBalance(ActualZoneNum).MRT;

    // modify setpoint for operative temperature control
    //  trapping for MRT fractions between 0.0 and 0.9 during get input, so shouldn't be able to divide by zero here.
    ZoneAirSetPoint = (ZoneAirSetPoint - thisMRTFraction * thisMRT) / (1.0 - thisMRTFraction);
}

void AdjustOperativeSetPointsforAdapComfort(EnergyPlusData &state, int const TempControlledZoneID, Real64 &ZoneAirSetPoint)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Xuan Luo
    //       DATE WRITTEN   Jan 2017

    // PURPOSE OF THIS SUBROUTINE:
    // This routine adjust the operative setpoints for each controlled adaptive thermal comfort models.

    auto &s_ztpc = state.dataZoneTempPredictorCorrector;
    auto const &tempControlledZone = state.dataZoneCtrls->TempControlledZone(TempControlledZoneID);
    auto const &AdapComfortDailySetPointSchedule = s_ztpc->AdapComfortDailySetPointSchedule;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int originZoneAirSetPoint = ZoneAirSetPoint;
    int AdaptiveComfortModelTypeIndex = tempControlledZone.AdaptiveComfortModelTypeIndex;

    // adjust zone operative setpoint
    if (!(tempControlledZone.AdaptiveComfortTempControl)) {
        return; // do nothing to setpoint
    }
    if ((state.dataWeather->Environment(state.dataWeather->Envrn).KindOfEnvrn != Constant::KindOfSim::DesignDay) &&
        (state.dataWeather->Environment(state.dataWeather->Envrn).KindOfEnvrn != Constant::KindOfSim::HVACSizeDesignDay)) {
        // Adjust run period cooling set point
        switch (AdaptiveComfortModelTypeIndex) {
        case static_cast<int>(AdaptiveComfortModel::ASH55_CENTRAL):
            ZoneAirSetPoint = AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveASH55_Central(state.dataEnvrn->DayOfYear);
            break;
        case static_cast<int>(AdaptiveComfortModel::ASH55_UPPER_90):
            ZoneAirSetPoint = AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveASH55_Upper_90(state.dataEnvrn->DayOfYear);
            break;
        case static_cast<int>(AdaptiveComfortModel::ASH55_UPPER_80):
            ZoneAirSetPoint = AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveASH55_Upper_80(state.dataEnvrn->DayOfYear);
            break;
        case static_cast<int>(AdaptiveComfortModel::CEN15251_CENTRAL):
            ZoneAirSetPoint = AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveCEN15251_Central(state.dataEnvrn->DayOfYear);
            break;
        case static_cast<int>(AdaptiveComfortModel::CEN15251_UPPER_I):
            ZoneAirSetPoint = AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveCEN15251_Upper_I(state.dataEnvrn->DayOfYear);
            break;
        case static_cast<int>(AdaptiveComfortModel::CEN15251_UPPER_II):
            ZoneAirSetPoint = AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveCEN15251_Upper_II(state.dataEnvrn->DayOfYear);
            break;
        case static_cast<int>(AdaptiveComfortModel::CEN15251_UPPER_III):
            ZoneAirSetPoint = AdapComfortDailySetPointSchedule.ThermalComfortAdaptiveCEN15251_Upper_III(state.dataEnvrn->DayOfYear);
            break;
        default:
            break;
        }
    } else {
        int const envrnDayNum(state.dataWeather->Environment(state.dataWeather->Envrn).DesignDayNum);
        int constexpr summerDesignDayTypeIndex(9);
        // Adjust summer design day set point
        if (state.dataWeather->DesDayInput(envrnDayNum).DayType == summerDesignDayTypeIndex) {
            ZoneAirSetPoint = s_ztpc->AdapComfortSetPointSummerDesDay[AdaptiveComfortModelTypeIndex - 2];
        }
    }
    // If adaptive operative temperature not applicable, set back
    if (ZoneAirSetPoint < originZoneAirSetPoint) {
        ZoneAirSetPoint = originZoneAirSetPoint;
    }
    // If meet fault flag, set back
    if (ZoneAirSetPoint == -1) {
        ZoneAirSetPoint = originZoneAirSetPoint;
    }
}

void CalcZoneAirComfortSetPoints(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Lixing Gu
    //       DATE WRITTEN   May 2006

    // PURPOSE OF THIS SUBROUTINE:
    // This routine sets the thermal comfort setpoints for each controlled zone based on air temperature obtained from thermal comfort models.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 SetPointLo = 0.0;
    Real64 SetPointHi = 0.0;
    Real64 Tset = 0.0;
    int ObjectCount = 0;
    Real64 PeopleCount = 0.0;

    auto &s_ztpc = state.dataZoneTempPredictorCorrector;
    auto &s_hbfs = state.dataHeatBalFanSys;
    // Call thermal comfort module to read zone control comfort object
    if (s_ztpc->CalcZoneAirComfortSetPointsFirstTimeFlag) {
        ThermalComfort::ManageThermalComfort(state, true);
        s_ztpc->CalcZoneAirComfortSetPointsFirstTimeFlag = false;
    }

    s_hbfs->ComfortControlType = HVAC::SetptType::Uncontrolled; // Default

    for (int RelativeZoneNum = 1; RelativeZoneNum <= state.dataZoneCtrls->NumComfortControlledZones; ++RelativeZoneNum) {

        auto &comfortZone = state.dataZoneCtrls->ComfortControlledZone(RelativeZoneNum);
        int ActualZoneNum = comfortZone.ActualZoneNum;
        auto &zone = state.dataHeatBal->Zone(ActualZoneNum);
        auto &zoneTstatSetpt = state.dataHeatBalFanSys->zoneTstatSetpts(ActualZoneNum);
        auto &zoneComfortControlsFanger = state.dataHeatBalFanSys->ZoneComfortControlsFanger(ActualZoneNum);
        s_hbfs->ComfortControlType(ActualZoneNum) = static_cast<HVAC::SetptType>(comfortZone.setptTypeSched->getCurrentVal());
        s_hbfs->ComfortControlTypeRpt(ActualZoneNum) = (int)s_hbfs->ComfortControlType(ActualZoneNum);

        // Get PMV values
        switch (s_hbfs->ComfortControlType(ActualZoneNum)) {
        case HVAC::SetptType::Uncontrolled: {
            zoneComfortControlsFanger.LowPMV = -999.0;
            zoneComfortControlsFanger.HighPMV = -999.0;
        } break;

        case HVAC::SetptType::SingleHeat: {
            zoneComfortControlsFanger.FangerType = (int)HVAC::SetptType::SingleHeat;
            zoneComfortControlsFanger.LowPMV = comfortZone.setpts[(int)HVAC::SetptType::SingleHeat].heatSetptSched->getCurrentVal();
            zoneComfortControlsFanger.HighPMV = -999.0;
        } break;

        case HVAC::SetptType::SingleCool: {
            zoneComfortControlsFanger.FangerType = (int)HVAC::SetptType::SingleCool;
            zoneComfortControlsFanger.LowPMV = -999.0;
            zoneComfortControlsFanger.HighPMV = comfortZone.setpts[(int)HVAC::SetptType::SingleCool].coolSetptSched->getCurrentVal();
        } break;

        case HVAC::SetptType::SingleHeatCool: {
            zoneComfortControlsFanger.FangerType = (int)HVAC::SetptType::SingleHeatCool;
            zoneComfortControlsFanger.LowPMV = zoneComfortControlsFanger.HighPMV =
                comfortZone.setpts[(int)HVAC::SetptType::SingleHeatCool].coolSetptSched->getCurrentVal();
        } break;

        case HVAC::SetptType::DualHeatCool: {
            zoneComfortControlsFanger.FangerType = (int)HVAC::SetptType::DualHeatCool;
            zoneComfortControlsFanger.LowPMV = comfortZone.setpts[(int)HVAC::SetptType::DualHeatCool].heatSetptSched->getCurrentVal();
            zoneComfortControlsFanger.HighPMV = comfortZone.setpts[(int)HVAC::SetptType::DualHeatCool].coolSetptSched->getCurrentVal();
            if (zoneComfortControlsFanger.LowPMV > zoneComfortControlsFanger.HighPMV) {
                ++zoneComfortControlsFanger.DualPMVErrCount;
                if (zoneComfortControlsFanger.DualPMVErrCount < 2) {
                    ShowWarningError(state,
                                     format("ThermostatSetpoint:ThermalComfort:Fanger:DualSetpoint: The heating PMV setpoint is above the "
                                            "cooling PMV setpoint in {}",
                                            comfortZone.setpts[(int)HVAC::SetptType::DualHeatCool].Name));
                    ShowContinueError(state, "The zone dual heating PMV setpoint is set to the dual cooling PMV setpoint.");
                    ShowContinueErrorTimeStamp(state, "Occurrence info:");
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   "The heating PMV setpoint is still above the cooling PMV setpoint",
                                                   zoneComfortControlsFanger.DualPMVErrIndex,
                                                   zoneComfortControlsFanger.LowPMV,
                                                   zoneComfortControlsFanger.LowPMV);
                }
                zoneComfortControlsFanger.LowPMV = zoneComfortControlsFanger.HighPMV;
            }
        } break;

        default: {
            ShowSevereError(state,
                            format("CalcZoneAirTempSetpoints: Illegal thermal control control type for Zone={}, Found value={}, in Schedule={}",
                                   zone.Name,
                                   s_hbfs->ComfortControlTypeRpt(ActualZoneNum),
                                   comfortZone.setptTypeSched->Name));
        } break;
        } // switch

        // Check Average method
        switch (comfortZone.averageMethod) {
        case DataZoneControls::AverageMethod::NO: {
            int PeopleNum = comfortZone.SpecificObjectNum;
            if (s_hbfs->ComfortControlType(ActualZoneNum) == HVAC::SetptType::SingleCool) {
                GetComfortSetPoints(state, PeopleNum, RelativeZoneNum, zoneComfortControlsFanger.HighPMV, SetPointLo);
            } else {
                GetComfortSetPoints(state, PeopleNum, RelativeZoneNum, zoneComfortControlsFanger.LowPMV, SetPointLo);
            }
            if (s_hbfs->ComfortControlType(ActualZoneNum) == HVAC::SetptType::DualHeatCool) {
                GetComfortSetPoints(state, PeopleNum, RelativeZoneNum, zoneComfortControlsFanger.HighPMV, SetPointHi);
            }
        } break;

        case DataZoneControls::AverageMethod::SPE: {
            int PeopleNum = comfortZone.SpecificObjectNum;
            if (s_hbfs->ComfortControlType(ActualZoneNum) == HVAC::SetptType::SingleCool) {
                GetComfortSetPoints(state, PeopleNum, RelativeZoneNum, zoneComfortControlsFanger.HighPMV, SetPointLo);
            } else {
                GetComfortSetPoints(state, PeopleNum, RelativeZoneNum, zoneComfortControlsFanger.LowPMV, SetPointLo);
            }
            if (s_hbfs->ComfortControlType(ActualZoneNum) == HVAC::SetptType::DualHeatCool) {
                GetComfortSetPoints(state, PeopleNum, RelativeZoneNum, zoneComfortControlsFanger.HighPMV, SetPointHi);
            }
        } break;

        case DataZoneControls::AverageMethod::OBJ: {
            SetPointLo = 0.0;
            SetPointHi = 0.0;
            for (int peopleNum = 1; peopleNum <= state.dataHeatBal->TotPeople; ++peopleNum) {
                if (ActualZoneNum == state.dataHeatBal->People(peopleNum).ZonePtr) {
                    ++ObjectCount;
                    GetComfortSetPoints(state, peopleNum, RelativeZoneNum, zoneComfortControlsFanger.LowPMV, Tset);
                    SetPointLo += Tset;

                    if (s_hbfs->ComfortControlType(ActualZoneNum) == HVAC::SetptType::DualHeatCool) {
                        GetComfortSetPoints(state, peopleNum, RelativeZoneNum, zoneComfortControlsFanger.HighPMV, Tset);
                        SetPointHi += Tset;
                    }
                }
            }
            SetPointLo /= ObjectCount;
            if (s_hbfs->ComfortControlType(ActualZoneNum) == HVAC::SetptType::DualHeatCool) {
                SetPointHi /= ObjectCount;
            }
        } break;

        case DataZoneControls::AverageMethod::PEO: {
            SetPointLo = 0.0;
            SetPointHi = 0.0;

            for (int PeopleNum = 1; PeopleNum <= state.dataHeatBal->TotPeople; ++PeopleNum) {
                auto &people = state.dataHeatBal->People(PeopleNum);
                if (ActualZoneNum == people.ZonePtr) {
                    int NumberOccupants = people.NumberOfPeople * people.sched->getCurrentVal();
                    PeopleCount += NumberOccupants;
                    GetComfortSetPoints(state, PeopleNum, RelativeZoneNum, zoneComfortControlsFanger.LowPMV, Tset);
                    SetPointLo += Tset * NumberOccupants;
                    if (s_hbfs->ComfortControlType(ActualZoneNum) == HVAC::SetptType::DualHeatCool) {
                        GetComfortSetPoints(state, PeopleNum, RelativeZoneNum, zoneComfortControlsFanger.HighPMV, Tset);
                        SetPointHi += Tset * NumberOccupants;
                    }
                }
            }
            if (PeopleCount > 0) {
                SetPointLo /= PeopleCount;
                if (s_hbfs->ComfortControlType(ActualZoneNum) == HVAC::SetptType::DualHeatCool) {
                    SetPointHi /= PeopleCount;
                }
            } else {
                if (comfortZone.PeopleAverageErrIndex == 0) {
                    ShowWarningMessage(state,
                                       format("ZoneControl:Thermostat:ThermalComfort: The total number of people in Zone = {} is zero. The People "
                                              "Average option is not used.",
                                              zone.Name));
                    ShowContinueError(state, "The Object Average option is used instead. Simulation continues .....");
                    ShowContinueErrorTimeStamp(state, "Occurrence info:");
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "ZoneControl:Thermostat:ThermalComfort: The total number of people in Zone = " + zone.Name +
                                                   " is still zero. The People Average option is not used",
                                               comfortZone.PeopleAverageErrIndex,
                                               PeopleCount,
                                               PeopleCount);
                SetPointLo = 0.0;
                SetPointHi = 0.0;
                for (int peopleNum = 1; peopleNum <= state.dataHeatBal->TotPeople; ++peopleNum) {
                    if (ActualZoneNum == state.dataHeatBal->People(peopleNum).ZonePtr) {
                        ++ObjectCount;
                        GetComfortSetPoints(state, peopleNum, RelativeZoneNum, zoneComfortControlsFanger.LowPMV, Tset);
                        SetPointLo += Tset;
                        if (s_hbfs->ComfortControlType(ActualZoneNum) == HVAC::SetptType::DualHeatCool) {
                            GetComfortSetPoints(state, peopleNum, RelativeZoneNum, zoneComfortControlsFanger.HighPMV, Tset);
                            SetPointHi += Tset;
                        }
                    }
                }
                SetPointLo /= ObjectCount;
                if (s_hbfs->ComfortControlType(ActualZoneNum) == HVAC::SetptType::DualHeatCool) {
                    SetPointHi /= ObjectCount;
                }
            }
        } break;

        default: {
        } break;
        } // switch

        // Assign setpoint
        switch (s_hbfs->ComfortControlType(ActualZoneNum)) {

        case HVAC::SetptType::Uncontrolled: {
            switch (state.dataHeatBalFanSys->TempControlType(ActualZoneNum)) {
            case HVAC::SetptType::SingleHeat:
                zoneTstatSetpt.setptHi = 0.0;
                break;
            case HVAC::SetptType::SingleCool:
                zoneTstatSetpt.setptLo = 0.0;
                break;
            default:
                break;
            }
        } break;

        case HVAC::SetptType::SingleHeat: {
            if (SetPointLo < comfortZone.TdbMinSetPoint) {
                SetPointLo = comfortZone.TdbMinSetPoint;
                if (comfortZone.TdbMinErrIndex < 2) {
                    ShowWarningMessage(state,
                                       format("ThermostatSetpoint:ThermalComfort:Fanger:SingleHeating temperature is below the Minimum dry-bulb "
                                              "temperature setpoint {}",
                                              comfortZone.Name));
                    ShowContinueError(state, "The zone heating setpoint is set to the Minimum dry-bulb temperature setpoint");
                    ShowContinueErrorTimeStamp(state, "Occurrence info:");
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "ThermostatSetpoint:ThermalComfort:Fanger:SingleHeating temperature is still below the "
                                               "Minimum dry-bulb temperature setpoint ...",
                                               comfortZone.TdbMinErrIndex,
                                               SetPointLo,
                                               SetPointLo);
            }
            zoneTstatSetpt.setpt = SetPointLo;
            zoneTstatSetpt.setptLo = zoneTstatSetpt.setpt;
            state.dataHeatBalFanSys->TempControlType(ActualZoneNum) = HVAC::SetptType::SingleHeat;
            state.dataHeatBalFanSys->TempControlTypeRpt(ActualZoneNum) = static_cast<int>(state.dataHeatBalFanSys->TempControlType(ActualZoneNum));
        } break;

        case HVAC::SetptType::SingleCool: {
            if (SetPointLo > comfortZone.TdbMaxSetPoint) {
                SetPointLo = comfortZone.TdbMaxSetPoint;
                if (comfortZone.TdbMaxErrIndex == 0) {
                    ShowWarningMessage(state,
                                       format("ThermostatSetpoint:ThermalComfort:Fanger:SingleCooling temperature is above the Maximum dry-bulb "
                                              "temperature setpoint {}",
                                              comfortZone.Name));
                    ShowContinueError(state, "The zone cooling setpoint is set to the Maximum dry-bulb temperature setpoint");
                    ShowContinueErrorTimeStamp(state, "Occurrence info:");
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "ThermostatSetpoint:ThermalComfort:Fanger:SingleCooling temperature is still above the "
                                               "Maximum dry-bulb temperature setpoint ...",
                                               comfortZone.TdbMaxErrIndex,
                                               SetPointLo,
                                               SetPointLo);
            }
            zoneTstatSetpt.setpt = SetPointLo;
            zoneTstatSetpt.setptHi = zoneTstatSetpt.setpt;
            state.dataHeatBalFanSys->TempControlType(ActualZoneNum) = HVAC::SetptType::SingleCool;
            state.dataHeatBalFanSys->TempControlTypeRpt(ActualZoneNum) = static_cast<int>(state.dataHeatBalFanSys->TempControlType(ActualZoneNum));
        } break;

        case HVAC::SetptType::SingleHeatCool: {
            if (comfortZone.TdbMaxSetPoint == comfortZone.TdbMinSetPoint) {
                SetPointLo = comfortZone.TdbMaxSetPoint;
            }
            if (SetPointLo > comfortZone.TdbMaxSetPoint) {
                SetPointLo = comfortZone.TdbMaxSetPoint;
            }
            if (SetPointLo < comfortZone.TdbMinSetPoint) {
                SetPointLo = comfortZone.TdbMinSetPoint;
            }
            if (SetPointLo < comfortZone.TdbMinSetPoint || SetPointLo > comfortZone.TdbMaxSetPoint) {
                if (comfortZone.TdbHCErrIndex == 0) {
                    ShowWarningMessage(state,
                                       format("ThermostatSetpoint:ThermalComfort:Fanger:SingleHeatingOrCooling temperature is above the Maximum or "
                                              "below the Minimum dry-bulb temperature setpoint {}",
                                              comfortZone.Name));
                    ShowContinueError(state,
                                      "The zone setpoint is set to the Maximum dry-bulb temperature setpoint if above or the Minimum "
                                      "dry-bulb temperature setpoint if below");
                    ShowContinueErrorTimeStamp(state, "Occurrence info:");
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "ThermostatSetpoint:ThermalComfort:Fanger:SingleHeatingOrCooling temperature is still beyond "
                                               "the range between Maximum and Minimum dry-bulb temperature setpoint ...",
                                               comfortZone.TdbHCErrIndex,
                                               SetPointLo,
                                               SetPointLo);
            }

            zoneTstatSetpt.setpt = SetPointLo;
            zoneTstatSetpt.setptHi = zoneTstatSetpt.setptLo = zoneTstatSetpt.setpt;
            state.dataHeatBalFanSys->TempControlType(ActualZoneNum) = HVAC::SetptType::SingleHeatCool;
            state.dataHeatBalFanSys->TempControlTypeRpt(ActualZoneNum) = static_cast<int>(state.dataHeatBalFanSys->TempControlType(ActualZoneNum));
        } break;

        case HVAC::SetptType::DualHeatCool: {
            if (SetPointLo < comfortZone.TdbMinSetPoint) {
                SetPointLo = comfortZone.TdbMinSetPoint;

                if (comfortZone.TdbDualMinErrIndex == 0) {
                    ShowWarningMessage(state,
                                       format("ThermostatSetpoint:ThermalComfort:Fanger:DualSetpoint temperature is below the Minimum dry-bulb "
                                              "temperature setpoint {}",
                                              comfortZone.Name));
                    ShowContinueError(state, "The zone dual heating setpoint is set to the Minimum dry-bulb temperature setpoint");
                    ShowContinueErrorTimeStamp(state, "Occurrence info:");
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "ThermostatSetpoint:ThermalComfort:Fanger:DualSetpoint temperature is still below the Minimum "
                                               "dry-bulb temperature setpoint ...",
                                               comfortZone.TdbDualMinErrIndex,
                                               SetPointLo,
                                               SetPointLo);
            }
            if (SetPointHi > comfortZone.TdbMaxSetPoint) {
                SetPointHi = comfortZone.TdbMaxSetPoint;
                if (comfortZone.TdbDualMaxErrIndex == 0) {
                    ShowWarningMessage(state,
                                       format("ThermostatSetpoint:ThermalComfort:Fanger:DualSetpoint temperature is above the Maximum dry-bulb "
                                              "temperature setpoint in zone = {}",
                                              comfortZone.Name));
                    ShowContinueError(state, "The zone dual cooling setpoint is set to the Maximum dry-bulb temperature setpoint");
                    ShowContinueErrorTimeStamp(state, "Occurrence info:");
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "ThermostatSetpoint:ThermalComfort:Fanger:DualSetpoint temperature is still above the Maximum "
                                               "dry-bulb temperature setpoint ...",
                                               comfortZone.TdbDualMaxErrIndex,
                                               SetPointLo,
                                               SetPointLo);
            }

            zoneTstatSetpt.setptLo = SetPointLo;
            zoneTstatSetpt.setptHi = SetPointHi;
            state.dataHeatBalFanSys->TempControlType(ActualZoneNum) = HVAC::SetptType::DualHeatCool;
            state.dataHeatBalFanSys->TempControlTypeRpt(ActualZoneNum) = static_cast<int>(state.dataHeatBalFanSys->TempControlType(ActualZoneNum));
        } break;

        default: {
            ShowSevereError(state,
                            format("CalcZoneAirComfortSetpoints: Illegal thermal control control type for Zone={}, Found value={}, in Schedule={}",
                                   zone.Name,
                                   s_hbfs->ComfortControlTypeRpt(ActualZoneNum),
                                   comfortZone.setptTypeSched->Name));
        } break;
        } // switch ()
    }
} // CalcZoneAirComfortSetpoints()

void GetComfortSetPoints(EnergyPlusData &state,
                         int const PeopleNum,
                         int const ComfortControlNum,
                         Real64 const PMVSet,
                         Real64 &Tset // drybulb setpoint temperature for a given PMV value
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Lixing Gu
    //       DATE WRITTEN   May, 2006
    // PURPOSE OF THIS SUBROUTINE:
    // This routine sets what the thermal comfort setpoints for each controlled zone should be based on air temperature
    // obtained from thermal comfort models. This is called each time step.

    // SUBROUTINE ARGUMENT DEFINITIONS:
    // 0 = Solution; 1 = Set to Min; 2 Set to Max

    // SUBROUTINE PARAMETER DEFINITIONS:
    Real64 constexpr Acc(0.001); // accuracy control for SolveRoot
    int constexpr MaxIter(500);  // iteration control for SolveRoot

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 PMVResult = 0.0; // Calculated PMV value

    auto &s_ztpc = state.dataZoneTempPredictorCorrector;

    auto &comfortControlledZone = state.dataZoneCtrls->ComfortControlledZone(ComfortControlNum);
    Real64 Tmin = comfortControlledZone.TdbMinSetPoint;
    Real64 Tmax = comfortControlledZone.TdbMaxSetPoint;

    ThermalComfort::CalcThermalComfortFanger(state, PeopleNum, Tmin, PMVResult);
    Real64 PMVMin = PMVResult;
    ThermalComfort::CalcThermalComfortFanger(state, PeopleNum, Tmax, PMVResult);
    Real64 PMVMax = PMVResult;
    if (PMVSet > PMVMin && PMVSet < PMVMax) {

        auto f = [&state, PMVSet, PeopleNum](Real64 Tset) {
            Real64 PMVresult = 0.0; // resulting PMV values
            ThermalComfort::CalcThermalComfortFanger(state, PeopleNum, Tset, PMVresult);
            return (PMVSet - PMVresult);
        };

        int SolFla = 0; // feed back flag from SolveRoot
        General::SolveRoot(state, Acc, MaxIter, SolFla, Tset, f, Tmin, Tmax);
        if (SolFla == -1) {
            if (!state.dataGlobal->WarmupFlag) {
                ++s_ztpc->IterLimitExceededNum1;
                if (s_ztpc->IterLimitExceededNum1 == 1) {
                    ShowWarningError(
                        state,
                        format("{}: Iteration limit exceeded calculating thermal comfort Fanger setpoint and non-converged setpoint is used",
                               comfortControlledZone.Name));
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   comfortControlledZone.Name + ":  Iteration limit exceeded calculating thermal comfort setpoint.",
                                                   s_ztpc->IterLimitErrIndex1,
                                                   Tset,
                                                   Tset);
                }
            }
        } else if (SolFla == -2) {
            if (!state.dataGlobal->WarmupFlag) {
                ++s_ztpc->IterLimitExceededNum2;
                if (s_ztpc->IterLimitExceededNum2 == 1) {
                    ShowWarningError(
                        state,
                        format("{}: Solution is not found in calculating thermal comfort Fanger setpoint and the minimum setpoint is used",
                               comfortControlledZone.Name));
                } else {
                    ShowRecurringWarningErrorAtEnd(
                        state,
                        format("{}:  Solution is not found in  calculating thermal comfort Fanger setpoint.", comfortControlledZone.Name),
                        s_ztpc->IterLimitErrIndex2,
                        Tset,
                        Tset);
                }
            }
        }
    } else if (PMVSet < PMVMin) {
        Tset = Tmin;
    } else if (PMVSet > PMVMax) {
        Tset = Tmax;
    }
}

void AdjustCoolingSetPointforTempAndHumidityControl(EnergyPlusData &state,
                                                    int const TempControlledZoneID,
                                                    int const ActualZoneNum // controlled zone actual zone number
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Bereket A Nigusse, FSEC/UCF
    //       DATE WRITTEN   Nov 2010

    // PURPOSE OF THIS SUBROUTINE:
    //  This subroutine modifies the air cooling setpoint temperature to effect zone air Temperature and humidity control
    //  Alter the zone air cooling setpoint if the zone air relative humidity value exceeds the the zone dehumidifying relative humidity setpoint.

    auto const &s_ztpc = state.dataZoneTempPredictorCorrector;
    auto &tempZone = state.dataZoneCtrls->TempControlledZone(TempControlledZoneID);
    auto &zoneTstatSetpt = state.dataHeatBalFanSys->zoneTstatSetpts(ActualZoneNum);

    if (!state.dataZoneCtrls->AnyZoneTempAndHumidityControl) {
        return; // do nothing to setpoint
    }
    if (tempZone.OvercoolCtrl == DataZoneControls::TempCtrl::None) {
        return; // do nothing to setpoint
    }

    Real64 ZoneOvercoolRange = (tempZone.OvercoolCtrl == DataZoneControls::TempCtrl::Scheduled) ? tempZone.zoneOvercoolRangeSched->getCurrentVal()
                                                                                                : tempZone.ZoneOvercoolConstRange;

    Real64 ZoneOvercoolControlRatio = tempZone.ZoneOvercoolControlRatio;

    // For Dual Setpoint thermostat the overcool range is limited by the temperature difference between cooling and heating setpoints
    Real64 MaxAllowedOvercoolRange = zoneTstatSetpt.setptHi - zoneTstatSetpt.setptLo;
    if (MaxAllowedOvercoolRange > 0.0) {
        ZoneOvercoolRange = min(ZoneOvercoolRange, MaxAllowedOvercoolRange);
    }
    // Calculate difference between zone air relative humidity and the dehumidifying setpoint
    Real64 RelativeHumidityDiff = s_ztpc->zoneHeatBalance(ActualZoneNum).airRelHum - tempZone.dehumidifyingSched->getCurrentVal();
    if (RelativeHumidityDiff > 0.0 && ZoneOvercoolControlRatio > 0.0) {
        // proportionally reset the cooling setpoint temperature downward (zone Overcool)
        ZoneOvercoolRange = min(ZoneOvercoolRange, RelativeHumidityDiff / ZoneOvercoolControlRatio);
        zoneTstatSetpt.setptHi -= ZoneOvercoolRange;
    }
}

void OverrideAirSetPointsforEMSCntrl(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         L. Gu
    //       DATE WRITTEN   June 2017

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine overrides the air temperature setpoint based on EMS
    auto const &s_hbfs = state.dataHeatBalFanSys;

    for (int Loop = 1; Loop <= state.dataZoneCtrls->NumTempControlledZones; ++Loop) {
        auto const &tempZone = state.dataZoneCtrls->TempControlledZone(Loop);
        if (tempZone.EMSOverrideHeatingSetPointOn) {
            int ZoneNum = tempZone.ActualZoneNum;
            auto &zoneTstatSetpt = s_hbfs->zoneTstatSetpts(ZoneNum);

            switch (state.dataHeatBalFanSys->TempControlType(ZoneNum)) {
            case HVAC::SetptType::SingleHeat:
            case HVAC::SetptType::SingleHeatCool: {
                zoneTstatSetpt.setptLo = zoneTstatSetpt.setpt = tempZone.EMSOverrideHeatingSetPointValue;
            } break;

            case HVAC::SetptType::DualHeatCool: {
                zoneTstatSetpt.setptLo = tempZone.EMSOverrideHeatingSetPointValue;
            } break;

            default: {
            } break;
            } // switch ()
        }

        if (tempZone.EMSOverrideCoolingSetPointOn) {
            int ZoneNum = tempZone.ActualZoneNum;
            auto &zoneTstatSetpt = s_hbfs->zoneTstatSetpts(ZoneNum);

            switch (s_hbfs->TempControlType(ZoneNum)) {
            case HVAC::SetptType::SingleCool:
            case HVAC::SetptType::SingleHeatCool: {
                zoneTstatSetpt.setptHi = zoneTstatSetpt.setpt = tempZone.EMSOverrideCoolingSetPointValue;
            } break;

            case HVAC::SetptType::DualHeatCool: {
                zoneTstatSetpt.setptHi = tempZone.EMSOverrideCoolingSetPointValue;
            } break;

            default: {
            } break;
            } // switch ()
        }
    }

    for (int Loop = 1; Loop <= state.dataZoneCtrls->NumComfortControlledZones; ++Loop) {
        auto &comfortZone = state.dataZoneCtrls->ComfortControlledZone(Loop);
        if (comfortZone.EMSOverrideHeatingSetPointOn) {
            int ZoneNum = comfortZone.ActualZoneNum;
            auto &zoneTstatSetpt = s_hbfs->zoneTstatSetpts(ZoneNum);

            switch (s_hbfs->ComfortControlType(ZoneNum)) {
            case HVAC::SetptType::SingleHeat:
            case HVAC::SetptType::SingleHeatCool: {
                zoneTstatSetpt.setptLo = zoneTstatSetpt.setpt = comfortZone.EMSOverrideHeatingSetPointValue;
            } break;

            case HVAC::SetptType::DualHeatCool: {
                zoneTstatSetpt.setptLo = comfortZone.EMSOverrideHeatingSetPointValue;
            } break;

            default: {
            } break;

            } // switch ()
        }

        if (comfortZone.EMSOverrideCoolingSetPointOn) {

            int ZoneNum = comfortZone.ActualZoneNum;
            auto &zoneTstatSetpt = s_hbfs->zoneTstatSetpts(ZoneNum);

            switch (static_cast<HVAC::SetptType>(s_hbfs->ComfortControlType(ZoneNum))) {
            case HVAC::SetptType::SingleCool:
            case HVAC::SetptType::SingleHeatCool: {
                zoneTstatSetpt.setptHi = zoneTstatSetpt.setpt = comfortZone.EMSOverrideCoolingSetPointValue;
            } break;

            case HVAC::SetptType::DualHeatCool: {
                zoneTstatSetpt.setptHi = comfortZone.EMSOverrideCoolingSetPointValue;
            } break;

            default: {
            } break;

            } // switch ()
        }
    }
}

// add values to the LEED tabular report related to schedules used by the thermostat objects
void FillPredefinedTableOnThermostatSetpoints(EnergyPlusData &state)
{
    // J.Glazer - Aug 2017
    using namespace OutputReportPredefined;
    auto &s_ztpc = state.dataZoneTempPredictorCorrector;

    std::vector<int> uniqSch;
    uniqSch.reserve(s_ztpc->NumTempControls[(int)HVAC::SetptType::SingleHeat] + s_ztpc->NumTempControls[(int)HVAC::SetptType::SingleCool] +
                    s_ztpc->NumTempControls[(int)HVAC::SetptType::SingleHeatCool] + s_ztpc->NumTempControls[(int)HVAC::SetptType::DualHeatCool] * 2);

    Real64 setPointAt11;
    Real64 setPointAt23;
    int numDays;
    std::string monthAssumed;
    std::string monthAssumed2;

    for (auto &setpt : s_ztpc->tempSetptScheds[(int)HVAC::SetptType::SingleHeat]) {
        if (std::find(uniqSch.begin(), uniqSch.end(), setpt.heatSched->Num) != uniqSch.end()) {
            continue;
        }

        uniqSch.emplace_back(setpt.heatSched->Num);
        PreDefTableEntry(state, state.dataOutRptPredefined->pdChLeedSchStPtFirstObjUsed, setpt.heatSched->Name, setpt.Name);

        std::tie(setPointAt11, numDays, monthAssumed) = setpt.heatSched->getValAndCountOnDay(state, false, Sched::DayType::Wednesday, 11);
        PreDefTableEntry(state, state.dataOutRptPredefined->pdchLeedSchStPt11amWednesday, setpt.heatSched->Name, setPointAt11);
        PreDefTableEntry(state, state.dataOutRptPredefined->pdchLeedSchStPt11amWedCnt, setpt.heatSched->Name, numDays);

        std::tie(setPointAt23, numDays, monthAssumed) = setpt.heatSched->getValAndCountOnDay(state, false, Sched::DayType::Wednesday, 23);
        PreDefTableEntry(state, state.dataOutRptPredefined->pdchLeedSchStPt11pmWednesday, setpt.heatSched->Name, setPointAt23);
        PreDefTableEntry(state, state.dataOutRptPredefined->pdchLeedSchStPt11pmWedCnt, setpt.heatSched->Name, numDays);

        PreDefTableEntry(state, state.dataOutRptPredefined->pdChLeedSchStPtMonthUsed, setpt.heatSched->Name, monthAssumed);
    }

    for (auto &setpt : s_ztpc->tempSetptScheds[(int)HVAC::SetptType::SingleCool]) {
        if (std::find(uniqSch.begin(), uniqSch.end(), setpt.coolSched->Num) != uniqSch.end()) {
            continue;
        }

        uniqSch.emplace_back(setpt.coolSched->Num);
        PreDefTableEntry(state, state.dataOutRptPredefined->pdChLeedSchStPtFirstObjUsed, setpt.coolSched->Name, setpt.Name);

        std::tie(setPointAt11, numDays, monthAssumed) = setpt.coolSched->getValAndCountOnDay(state, true, Sched::DayType::Wednesday, 11);
        PreDefTableEntry(state, state.dataOutRptPredefined->pdchLeedSchStPt11amWednesday, setpt.coolSched->Name, setPointAt11);
        PreDefTableEntry(state, state.dataOutRptPredefined->pdchLeedSchStPt11amWedCnt, setpt.coolSched->Name, numDays);

        std::tie(setPointAt23, numDays, monthAssumed) = setpt.coolSched->getValAndCountOnDay(state, true, Sched::DayType::Wednesday, 23);
        PreDefTableEntry(state, state.dataOutRptPredefined->pdchLeedSchStPt11pmWednesday, setpt.coolSched->Name, setPointAt23);
        PreDefTableEntry(state, state.dataOutRptPredefined->pdchLeedSchStPt11pmWedCnt, setpt.coolSched->Name, numDays);

        PreDefTableEntry(state, state.dataOutRptPredefined->pdChLeedSchStPtMonthUsed, setpt.coolSched->Name, monthAssumed);
    }

    for (auto &setpt : s_ztpc->tempSetptScheds[(int)HVAC::SetptType::SingleHeatCool]) {
        if (std::find(uniqSch.begin(), uniqSch.end(), setpt.heatSched->Num) != uniqSch.end()) {
            continue;
        }

        uniqSch.emplace_back(setpt.heatSched->Num);
        PreDefTableEntry(state, state.dataOutRptPredefined->pdChLeedSchStPtFirstObjUsed, setpt.heatSched->Name, setpt.Name);

        std::string schNm = setpt.heatSched->Name + " (summer)";
        std::tie(setPointAt11, numDays, monthAssumed) = setpt.heatSched->getValAndCountOnDay(state, true, Sched::DayType::Wednesday, 11);
        PreDefTableEntry(state, state.dataOutRptPredefined->pdchLeedSchStPt11amWednesday, schNm, setPointAt11);
        PreDefTableEntry(state, state.dataOutRptPredefined->pdchLeedSchStPt11amWedCnt, schNm, numDays);

        std::tie(setPointAt23, numDays, monthAssumed) = setpt.heatSched->getValAndCountOnDay(state, true, Sched::DayType::Wednesday, 23);
        PreDefTableEntry(state, state.dataOutRptPredefined->pdchLeedSchStPt11pmWednesday, schNm, setPointAt23);
        PreDefTableEntry(state, state.dataOutRptPredefined->pdchLeedSchStPt11pmWedCnt, schNm, numDays);

        schNm = setpt.heatSched->Name + " (winter)";
        std::tie(setPointAt11, numDays, monthAssumed2) = setpt.heatSched->getValAndCountOnDay(state, false, Sched::DayType::Wednesday, 11);
        PreDefTableEntry(state, state.dataOutRptPredefined->pdchLeedSchStPt11amWednesday, schNm, setPointAt11);
        PreDefTableEntry(state, state.dataOutRptPredefined->pdchLeedSchStPt11amWedCnt, schNm, numDays);

        std::tie(setPointAt23, numDays, monthAssumed2) = setpt.heatSched->getValAndCountOnDay(state, false, Sched::DayType::Wednesday, 23);
        PreDefTableEntry(state, state.dataOutRptPredefined->pdchLeedSchStPt11pmWednesday, schNm, setPointAt23);
        PreDefTableEntry(state, state.dataOutRptPredefined->pdchLeedSchStPt11pmWedCnt, schNm, numDays);

        PreDefTableEntry(state, state.dataOutRptPredefined->pdChLeedSchStPtMonthUsed, setpt.heatSched->Name, monthAssumed + " and " + monthAssumed2);
    }

    for (auto &setpt : s_ztpc->tempSetptScheds[(int)HVAC::SetptType::DualHeatCool]) {
        if (std::find(uniqSch.begin(), uniqSch.end(), setpt.heatSched->Num) == uniqSch.end()) {
            uniqSch.emplace_back(setpt.heatSched->Num);
            PreDefTableEntry(state, state.dataOutRptPredefined->pdChLeedSchStPtFirstObjUsed, setpt.heatSched->Name, setpt.Name);

            std::tie(setPointAt11, numDays, monthAssumed) = setpt.heatSched->getValAndCountOnDay(state, false, Sched::DayType::Wednesday, 11);
            PreDefTableEntry(state, state.dataOutRptPredefined->pdchLeedSchStPt11amWednesday, setpt.heatSched->Name, setPointAt11);
            PreDefTableEntry(state, state.dataOutRptPredefined->pdchLeedSchStPt11amWedCnt, setpt.heatSched->Name, numDays);

            std::tie(setPointAt23, numDays, monthAssumed) = setpt.heatSched->getValAndCountOnDay(state, false, Sched::DayType::Wednesday, 23);
            PreDefTableEntry(state, state.dataOutRptPredefined->pdchLeedSchStPt11pmWednesday, setpt.heatSched->Name, setPointAt23);
            PreDefTableEntry(state, state.dataOutRptPredefined->pdchLeedSchStPt11pmWedCnt, setpt.heatSched->Name, numDays);

            PreDefTableEntry(state, state.dataOutRptPredefined->pdChLeedSchStPtMonthUsed, setpt.heatSched->Name, monthAssumed);
        }

        if (std::find(uniqSch.begin(), uniqSch.end(), setpt.coolSched->Num) == uniqSch.end()) {
            uniqSch.emplace_back(setpt.coolSched->Num);
            PreDefTableEntry(state, state.dataOutRptPredefined->pdChLeedSchStPtFirstObjUsed, setpt.coolSched->Name, setpt.Name);

            std::tie(setPointAt11, numDays, monthAssumed) = setpt.coolSched->getValAndCountOnDay(state, true, Sched::DayType::Wednesday, 11);
            PreDefTableEntry(state, state.dataOutRptPredefined->pdchLeedSchStPt11amWednesday, setpt.coolSched->Name, setPointAt11);
            PreDefTableEntry(state, state.dataOutRptPredefined->pdchLeedSchStPt11amWedCnt, setpt.coolSched->Name, numDays);

            std::tie(setPointAt23, numDays, monthAssumed) = setpt.coolSched->getValAndCountOnDay(state, true, Sched::DayType::Wednesday, 23);
            PreDefTableEntry(state, state.dataOutRptPredefined->pdchLeedSchStPt11pmWednesday, setpt.coolSched->Name, setPointAt23);
            PreDefTableEntry(state, state.dataOutRptPredefined->pdchLeedSchStPt11pmWedCnt, setpt.coolSched->Name, numDays);

            PreDefTableEntry(state, state.dataOutRptPredefined->pdChLeedSchStPtMonthUsed, setpt.coolSched->Name, monthAssumed);
        }
    }
}

void FillPredefinedTableOnThermostatSchedules(EnergyPlusData &state)
{
    // add values to the System Summary tabular report related to schedules used by the thermostat objects
    // J.Glazer - March 2024
    using OutputReportPredefined::PreDefTableEntry;
    auto &orp = state.dataOutRptPredefined;

    // Helper struct so we can sort to ensure a consistent order.
    // No matter the order in which the multiple Field Sets (Control Object Type, Control Name), the same thing is reported to the tabular outputs
    // You don't actually need this anymore
    struct ControlTypeInfo
    {
        // HVAC::ThermostatType tType = HVAC::ThermostatType::Invalid;
        std::string thermostatType;
        std::string controlTypeName;
        std::string heatSchName;
        std::string coolSchName;

        // Only need the operator<, and we use C++17 so I can't use a defaulted 3-way operator<=>
        bool operator<(const ControlTypeInfo &other) const
        {
            return std::tie(this->thermostatType, this->controlTypeName, this->heatSchName, this->coolSchName) <
                   std::tie(other.thermostatType, other.controlTypeName, other.heatSchName, other.coolSchName);
        }
    };
    using ControlTypeInfoMemPtr = std::string ControlTypeInfo::*;

    // How many people on the EnergyPlus team understand this code?
    auto joinStrings = [](const std::vector<ControlTypeInfo> &infos, ControlTypeInfoMemPtr memPtr) -> std::string {
        std::vector<std::string> result;
        result.reserve(infos.size());
        for (const auto &info : infos) {
            std::string val = info.*memPtr;
            if (val.empty()) {
                continue;
            }
            result.emplace_back(std::move(val));
        }
        return fmt::format("{}", fmt::join(result, ", "));
    };

    for (int idx = 1; idx <= state.dataZoneCtrls->NumTempControlledZones; ++idx) {
        auto &tcz = state.dataZoneCtrls->TempControlledZone(idx);
        PreDefTableEntry(state, orp->pdchStatName, tcz.ZoneName, tcz.Name);
        PreDefTableEntry(state, orp->pdchStatCtrlTypeSchd, tcz.ZoneName, tcz.setptTypeSched->Name);

        std::vector<ControlTypeInfo> infos;
        infos.resize((int)HVAC::SetptType::Num);
        for (HVAC::SetptType setptType : HVAC::controlledSetptTypes) {
            auto &setpt = tcz.setpts[(int)setptType];

            auto &info = infos[(int)setptType];

            if (setpt.Name.empty()) {
                continue;
            }

            info.thermostatType = HVAC::setptTypeNames[(int)setptType];
            info.controlTypeName = setpt.Name;
            switch (setptType) {
            case HVAC::SetptType::DualHeatCool:
            case HVAC::SetptType::SingleHeatCool: {
                info.coolSchName = setpt.coolSetptSched->Name;
                info.heatSchName = setpt.heatSetptSched->Name;
            } break;

            case HVAC::SetptType::SingleCool: {
                info.coolSchName = setpt.coolSetptSched->Name;
            } break;

            case HVAC::SetptType::SingleHeat: {
                info.heatSchName = setpt.heatSetptSched->Name;
            } break;
            }
            infos.emplace_back(std::move(info));
        }
        std::sort(infos.begin(), infos.end());

        PreDefTableEntry(state, orp->pdchStatSchdType1, tcz.ZoneName, joinStrings(infos, &ControlTypeInfo::thermostatType));
        PreDefTableEntry(state, orp->pdchStatSchdTypeName1, tcz.ZoneName, joinStrings(infos, &ControlTypeInfo::controlTypeName));
        if (auto heatSchNames = joinStrings(infos, &ControlTypeInfo::heatSchName); !heatSchNames.empty()) {
            PreDefTableEntry(state, orp->pdchStatSchdHeatName, tcz.ZoneName, heatSchNames);
        }
        if (auto coolSchNames = joinStrings(infos, &ControlTypeInfo::coolSchName); !coolSchNames.empty()) {
            PreDefTableEntry(state, orp->pdchStatSchdCoolName, tcz.ZoneName, coolSchNames);
        }
    }
}

void ZoneSpaceHeatBalanceData::updateTemperatures(EnergyPlusData &state,
                                                  bool const ShortenTimeStepSys,
                                                  bool const UseZoneTimeStepHistory,
                                                  Real64 const PriorTimeStep,
                                                  int const zoneNum,
                                                  int const spaceNum)
{
    assert(zoneNum > 0);
    if (ShortenTimeStepSys) {
        // timestep has just shifted from full zone timestep to a new shorter system timestep
        // throw away last updates in corrector and rewind for resimulating smaller timestep
        if (spaceNum == 0) {
            if (state.dataHeatBal->Zone(zoneNum).SystemZoneNodeNumber > 0) { // roll back result for zone air node,
                auto &zoneNode = state.dataLoopNodes->Node(state.dataHeatBal->Zone(zoneNum).SystemZoneNodeNumber);
                zoneNode.Temp = this->XMAT[0];
                state.dataHeatBalFanSys->TempTstatAir(zoneNum) = this->XMAT[0];
                zoneNode.HumRat = this->WPrevZoneTS[0];
                zoneNode.Enthalpy = Psychrometrics::PsyHFnTdbW(this->XMAT[0], this->WPrevZoneTS[0]);
            }
        } else {
            if (state.dataHeatBal->space(spaceNum).SystemZoneNodeNumber > 0) { // roll back result for space air node,
                auto &spaceNode = state.dataLoopNodes->Node(state.dataHeatBal->space(spaceNum).SystemZoneNodeNumber);
                spaceNode.Temp = this->XMAT[0];
                state.dataHeatBalFanSys->TempTstatAir(zoneNum) = this->XMAT[0];
                spaceNode.HumRat = this->WPrevZoneTS[0];
                spaceNode.Enthalpy = Psychrometrics::PsyHFnTdbW(this->XMAT[0], this->WPrevZoneTS[0]);
            }
        }

        if (state.dataHVACGlobal->NumOfSysTimeSteps !=
            state.dataHVACGlobal->NumOfSysTimeStepsLastZoneTimeStep) { // cannot reuse existing DS data, interpolate from zone time
            Real64 TimeStepSys = state.dataHVACGlobal->TimeStepSys;
            this->MAT = DownInterpolate4HistoryValues(PriorTimeStep, TimeStepSys, this->XMAT, this->DSXMAT);
            this->airHumRat = DownInterpolate4HistoryValues(PriorTimeStep, TimeStepSys, this->WPrevZoneTS, this->DSWPrevZoneTS);

            if (spaceNum == 0 && state.dataRoomAir->anyNonMixingRoomAirModel) {
                if (state.dataRoomAir->IsZoneDispVent3Node(zoneNum) || state.dataRoomAir->IsZoneUFAD(zoneNum)) {

                    state.dataRoomAir->MATFloor(zoneNum) = DownInterpolate4HistoryValues(
                        PriorTimeStep, TimeStepSys, state.dataRoomAir->XMATFloor(zoneNum), state.dataRoomAir->DSXMATFloor(zoneNum));
                    state.dataRoomAir->MATOC(zoneNum) = DownInterpolate4HistoryValues(
                        PriorTimeStep, TimeStepSys, state.dataRoomAir->XMATOC(zoneNum), state.dataRoomAir->DSXMATOC(zoneNum));
                    state.dataRoomAir->MATMX(zoneNum) = DownInterpolate4HistoryValues(
                        PriorTimeStep, TimeStepSys, state.dataRoomAir->XMATMX(zoneNum), state.dataRoomAir->DSXMATMX(zoneNum));
                }
                if (state.dataRoomAir->AirModel(zoneNum).AirModel == RoomAir::RoomAirModel::AirflowNetwork) {
                    for (auto &afnNode : state.dataRoomAir->AFNZoneInfo(zoneNum).Node) {
                        afnNode.AirTemp = DownInterpolate4HistoryValues(PriorTimeStep, TimeStepSys, afnNode.AirTempX, afnNode.AirTempDSX);

                        afnNode.HumRat = DownInterpolate4HistoryValues(PriorTimeStep, TimeStepSys, afnNode.HumRatX, afnNode.HumRatDSX);
                    }
                }
            }
        } else { // reuse history data in DS terms from last zone time step to preserve information that would be lost
                 // do nothing because DS history would have been pushed prior and should be ready
        }
    }
    // now update the variables actually used in the balance equations.
    if (UseZoneTimeStepHistory) {
        this->ZTM = this->XMAT;
        this->WPrevZoneTSTemp = this->WPrevZoneTS;
    } else { // use down-stepped history
        this->ZTM = this->DSXMAT;
        this->WPrevZoneTSTemp = this->DSWPrevZoneTS;
    }
}

void ZoneSpaceHeatBalanceData::calcPredictedSystemLoad(EnergyPlusData &state, Real64 const RAFNFrac, int const zoneNum, int const spaceNum)
{
    // Calculate the predicted system load for a time step.

    assert(zoneNum > 0);
    auto const &thisZone = state.dataHeatBal->Zone(zoneNum);
    auto const &zoneTstatSetpt = state.dataHeatBalFanSys->zoneTstatSetpts(zoneNum);

    bool thisDeadBandOrSetBack = false;
    Real64 ZoneSetPoint = 0.0;
    Real64 totalLoad = 0.0;
    Real64 LoadToHeatingSetPoint = 0.0;
    Real64 LoadToCoolingSetPoint = 0.0;

    auto &s_ztpc = state.dataZoneTempPredictorCorrector;

    int zoneNodeNum = thisZone.SystemZoneNodeNumber;
    if (spaceNum > 0) {
        zoneNodeNum = state.dataHeatBal->space(spaceNum).SystemZoneNodeNumber;
    }

    switch (state.dataHeatBalFanSys->TempControlType(zoneNum)) {
    case HVAC::SetptType::Uncontrolled: {
        // Uncontrolled Zone
        LoadToHeatingSetPoint = 0.0;
        LoadToCoolingSetPoint = 0.0;
        totalLoad = 0.0;
    } break;

    case HVAC::SetptType::SingleHeat: {
        switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
        case DataHeatBalance::SolutionAlgo::ThirdOrder: {
            LoadToHeatingSetPoint = (this->tempDepLoad * zoneTstatSetpt.setpt - this->tempIndLoad);
            break;
        }
        case DataHeatBalance::SolutionAlgo::AnalyticalSolution: {
            if (this->tempDepLoad == 0.0) { // B=0
                LoadToHeatingSetPoint = this->AirPowerCap * (zoneTstatSetpt.setpt - this->T1) - this->tempIndLoad;
            } else {
                Real64 const exp_700_TA(std::exp(min(700.0, -this->tempDepLoad / this->AirPowerCap)));
                LoadToHeatingSetPoint = this->tempDepLoad * (zoneTstatSetpt.setpt - this->T1 * exp_700_TA) / (1.0 - exp_700_TA) - this->tempIndLoad;
            }
        } break;

        case DataHeatBalance::SolutionAlgo::EulerMethod: {
            LoadToHeatingSetPoint =
                this->AirPowerCap * (zoneTstatSetpt.setpt - this->T1) + this->tempDepLoad * (zoneTstatSetpt.setpt) - this->tempIndLoad;
        } break;

        default: {
            assert(false);
        } break;
        } // switch (Algo)

        if (RAFNFrac > 0.0) {
            LoadToHeatingSetPoint = LoadToHeatingSetPoint / RAFNFrac;
        }
        totalLoad = LoadToHeatingSetPoint;
        ZoneSetPoint = zoneTstatSetpt.setpt;
        LoadToCoolingSetPoint = LoadToHeatingSetPoint;
        // for consistency with the other cases, use LE instead of LT and don't subtract 1.0 Watt as a way of pushing the zero load
        // case over the threshold
        if ((totalLoad) <= 0.0) {
            thisDeadBandOrSetBack = true;
        }
    } break;

    case HVAC::SetptType::SingleCool: {
        switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
        case DataHeatBalance::SolutionAlgo::ThirdOrder: {
            LoadToCoolingSetPoint = this->tempDepLoad * zoneTstatSetpt.setpt - this->tempIndLoad;
        } break;

        case DataHeatBalance::SolutionAlgo::AnalyticalSolution: {
            if (this->tempDepLoad == 0.0) { // B=0
                LoadToCoolingSetPoint = this->AirPowerCap * (zoneTstatSetpt.setpt - this->T1) - this->tempIndLoad;
            } else {
                Real64 const exp_700_TA(std::exp(min(700.0, -this->tempDepLoad / this->AirPowerCap)));
                LoadToCoolingSetPoint = this->tempDepLoad * (zoneTstatSetpt.setpt - this->T1 * exp_700_TA) / (1.0 - exp_700_TA) - this->tempIndLoad;
            }
        } break;

        case DataHeatBalance::SolutionAlgo::EulerMethod: {
            LoadToCoolingSetPoint =
                this->AirPowerCap * (zoneTstatSetpt.setpt - this->T1) + this->tempDepLoad * zoneTstatSetpt.setpt - this->tempIndLoad;
        } break;
        default: {
            assert(false);
        } break;
        } // swtich (Algo)

        if (RAFNFrac > 0.0) {
            LoadToHeatingSetPoint = LoadToHeatingSetPoint / RAFNFrac;
        }
        if (thisZone.HasAdjustedReturnTempByITE && !(state.dataGlobal->BeginSimFlag)) {
            LoadToCoolingSetPoint = this->tempDepLoad * thisZone.AdjustedReturnTempByITE - this->tempIndLoad;
        }
        totalLoad = LoadToCoolingSetPoint;
        ZoneSetPoint = zoneTstatSetpt.setpt;
        LoadToHeatingSetPoint = LoadToCoolingSetPoint;
        // for consistency with the other cases, use GE instead of GT and don't add 1.0 Watt as a way of pushing the zero load
        // case over the threshold
        if ((totalLoad) >= 0.0) {
            thisDeadBandOrSetBack = true;
        }
    } break;

    case HVAC::SetptType::SingleHeatCool: {
        switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
        case DataHeatBalance::SolutionAlgo::ThirdOrder: {
            LoadToHeatingSetPoint = (this->tempDepLoad * (zoneTstatSetpt.setpt) - this->tempIndLoad);
            LoadToCoolingSetPoint = (this->tempDepLoad * (zoneTstatSetpt.setpt) - this->tempIndLoad);
        } break;

        case DataHeatBalance::SolutionAlgo::AnalyticalSolution: {
            if (this->tempDepLoad == 0.0) { // B=0
                LoadToHeatingSetPoint = this->AirPowerCap * (zoneTstatSetpt.setpt - this->T1) - this->tempIndLoad;
                LoadToCoolingSetPoint = this->AirPowerCap * (zoneTstatSetpt.setpt - this->T1) - this->tempIndLoad;
            } else {
                Real64 const exp_700_TA(std::exp(min(700.0, -this->tempDepLoad / this->AirPowerCap)));
                LoadToHeatingSetPoint = this->tempDepLoad * (zoneTstatSetpt.setpt - this->T1 * exp_700_TA) / (1.0 - exp_700_TA) - this->tempIndLoad;
                LoadToCoolingSetPoint = this->tempDepLoad * (zoneTstatSetpt.setpt - this->T1 * exp_700_TA) / (1.0 - exp_700_TA) - this->tempIndLoad;
            }
        } break;

        case DataHeatBalance::SolutionAlgo::EulerMethod: {
            LoadToHeatingSetPoint =
                this->AirPowerCap * (zoneTstatSetpt.setpt - this->T1) + this->tempDepLoad * zoneTstatSetpt.setpt - this->tempIndLoad;
            LoadToCoolingSetPoint =
                this->AirPowerCap * (zoneTstatSetpt.setpt - this->T1) + this->tempDepLoad * zoneTstatSetpt.setpt - this->tempIndLoad;
        } break;
        default: {
            assert(false);
        } break;
        } // swtich (Algo)

        ZoneSetPoint = zoneTstatSetpt.setpt;
        if (RAFNFrac > 0.0) {
            LoadToHeatingSetPoint = LoadToHeatingSetPoint / RAFNFrac;
        }
        if (RAFNFrac > 0.0) {
            LoadToCoolingSetPoint = LoadToCoolingSetPoint / RAFNFrac;
        }

        if (thisZone.HasAdjustedReturnTempByITE && !(state.dataGlobal->BeginSimFlag)) {
            LoadToCoolingSetPoint = this->tempDepLoad * thisZone.AdjustedReturnTempByITE - this->tempIndLoad;
        }

        // Note that LoadToHeatingSetPoint is generally not equal to LoadToCoolingSetPoint
        // when the heating and cooling set-points are equal if the zone is unmixed,
        // e.g. displacement ventilation or UFAD, since the stratification is generally not the same in heating and cooling modes

        // Possible combinations:
        // 1/  LoadToHeatingSetPoint > 0 & LoadToCoolingSetPoint > 0 -->  Heating required
        // 2/  LoadToHeatingSetPoint  >  LoadToCoolingSetPoint       -->  Possible in the unmixed case but should be trapped
        //                                                                 as a poor choice of set-points
        // 3/  LoadToHeatingSetPoint < 0 & LoadToCoolingSetPoint < 0 -->  Cooling Required
        // 4/  LoadToHeatingSetPoint <=0 & LoadToCoolingSetPoint >=0 -->  Dead Band Operation ! includes zero load cases
        // First trap bad set-points
        if (LoadToHeatingSetPoint > LoadToCoolingSetPoint) {
            ShowSevereError(state,
                            "HVAC::SetptType::SingleHeatCool: Effective heating set-point higher than effective cooling set-point - use "
                            "DualSetPointWithDeadBand if using unmixed air model");
            ShowContinueErrorTimeStamp(state, format("occurs in Zone={}", thisZone.Name));
            ShowContinueError(state,
                              format("LoadToHeatingSetPoint={:.3R}, LoadToCoolingSetPoint={:.3R}", LoadToHeatingSetPoint, LoadToCoolingSetPoint));
            ShowContinueError(state, format("Zone TempDepZnLd={:.2R}", this->tempDepLoad));
            ShowContinueError(state, format("Zone TempIndZnLd={:.2R}", this->tempIndLoad));
            ShowContinueError(state, format("Zone ThermostatSetPoint={:.2R}", zoneTstatSetpt.setpt));
            ShowFatalError(state, "Program terminates due to above conditions.");
        }

        if (LoadToHeatingSetPoint > 0.0 && LoadToCoolingSetPoint > 0.0) {
            totalLoad = LoadToHeatingSetPoint;
        } else if (LoadToHeatingSetPoint < 0.0 && LoadToCoolingSetPoint < 0.0) {
            totalLoad = LoadToCoolingSetPoint;
        } else if (LoadToHeatingSetPoint <= 0.0 && LoadToCoolingSetPoint >= 0.0) { // deadband includes zero loads
            totalLoad = 0.0;
            if (zoneNodeNum > 0) {
                ZoneSetPoint = state.dataLoopNodes->Node(zoneNodeNum).Temp;
                ZoneSetPoint = max(ZoneSetPoint, zoneTstatSetpt.setptLo); // trap out of deadband
                ZoneSetPoint = min(ZoneSetPoint, zoneTstatSetpt.setptHi); // trap out of deadband
            }
            thisDeadBandOrSetBack = true;
        } else { // this should never occur!
            ShowSevereError(state,
                            "SingleHeatCoolSetPoint: Unanticipated combination of heating and cooling loads - report to EnergyPlus Development Team");
            ShowContinueErrorTimeStamp(state, format("occurs in Zone={}", thisZone.Name));
            ShowContinueError(state,
                              format("LoadToHeatingSetPoint={:.3R}, LoadToCoolingSetPoint={:.3R}", LoadToHeatingSetPoint, LoadToCoolingSetPoint));
            ShowContinueError(state, format("Zone TempDepZnLd={:.2R}", this->tempDepLoad));
            ShowContinueError(state, format("Zone TempIndZnLd={:.2R}", this->tempIndLoad));
            ShowContinueError(state, format("Zone ThermostatSetPoint={:.2R}", zoneTstatSetpt.setpt));
            ShowFatalError(state, "Program terminates due to above conditions.");
        }
    } break;

    case HVAC::SetptType::DualHeatCool: {
        switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
        case DataHeatBalance::SolutionAlgo::ThirdOrder: {
            LoadToHeatingSetPoint = (this->tempDepLoad * (zoneTstatSetpt.setptLo) - this->tempIndLoad);
            LoadToCoolingSetPoint = (this->tempDepLoad * (zoneTstatSetpt.setptHi) - this->tempIndLoad);
        } break;

        case DataHeatBalance::SolutionAlgo::AnalyticalSolution: {
            if (this->tempDepLoad == 0.0) { // B=0
                LoadToHeatingSetPoint = this->AirPowerCap * (zoneTstatSetpt.setptLo - this->T1) - this->tempIndLoad;
                LoadToCoolingSetPoint = this->AirPowerCap * (zoneTstatSetpt.setptHi - this->T1) - this->tempIndLoad;
            } else {
                Real64 const exp_700_TA(std::exp(min(700.0, -this->tempDepLoad / this->AirPowerCap)));
                LoadToHeatingSetPoint = this->tempDepLoad * (zoneTstatSetpt.setptLo - this->T1 * exp_700_TA) / (1.0 - exp_700_TA) - this->tempIndLoad;
                LoadToCoolingSetPoint = this->tempDepLoad * (zoneTstatSetpt.setptHi - this->T1 * exp_700_TA) / (1.0 - exp_700_TA) - this->tempIndLoad;
            }
        } break;

        case DataHeatBalance::SolutionAlgo::EulerMethod: {
            LoadToHeatingSetPoint =
                this->AirPowerCap * (zoneTstatSetpt.setptLo - this->T1) + this->tempDepLoad * zoneTstatSetpt.setptLo - this->tempIndLoad;
            LoadToCoolingSetPoint =
                this->AirPowerCap * (zoneTstatSetpt.setptHi - this->T1) + this->tempDepLoad * zoneTstatSetpt.setptHi - this->tempIndLoad;
        } break;
        default: {
            assert(false);

        } break;
        } // switch (Algo)

        if (RAFNFrac > 0.0) {
            LoadToHeatingSetPoint = LoadToHeatingSetPoint / RAFNFrac;
        }
        if (RAFNFrac > 0.0) {
            LoadToCoolingSetPoint = LoadToCoolingSetPoint / RAFNFrac;
        }

        if (thisZone.HasAdjustedReturnTempByITE && !(state.dataGlobal->BeginSimFlag)) {
            LoadToCoolingSetPoint = this->tempDepLoad * thisZone.AdjustedReturnTempByITE - this->tempIndLoad;
        }

        // Possible combinations:
        // 1/  LoadToHeatingSetPoint > 0 & LoadToCoolingSetPoint > 0 -->  Heating required
        // 2/  LoadToHeatingSetPoint  >  LoadToCoolingSetPoint       -->  Possible in the unmixed case but should be trapped
        //                                                                  as a poor choice of set-points
        // 3/  LoadToHeatingSetPoint < 0 & LoadToCoolingSetPoint < 0 -->  Cooling Required
        // 4/  LoadToHeatingSetPoint <=0 & LoadToCoolingSetPoint >=0 -->  Dead Band Operation - includes zero load cases
        // First trap bad set-points
        if (LoadToHeatingSetPoint > LoadToCoolingSetPoint) {
            ShowSevereError(state,
                            "DualSetPointWithDeadBand: Effective heating set-point higher than effective cooling set-point - increase "
                            "deadband if using unmixed air model");
            ShowContinueErrorTimeStamp(state, format("occurs in Zone={}", thisZone.Name));
            ShowContinueError(state,
                              format("LoadToHeatingSetPoint={:.3R}, LoadToCoolingSetPoint={:.3R}", LoadToHeatingSetPoint, LoadToCoolingSetPoint));
            ShowContinueError(state, format("Zone TempDepZnLd={:.2R}", this->tempDepLoad));
            ShowContinueError(state, format("Zone TempIndZnLd={:.2R}", this->tempIndLoad));
            ShowContinueError(state, format("Zone Heating ThermostatSetPoint={:.2R}", zoneTstatSetpt.setptLo));
            ShowContinueError(state, format("Zone Cooling ThermostatSetPoint={:.2R}", zoneTstatSetpt.setptHi));
            ShowFatalError(state, "Program terminates due to above conditions.");
        }

        if (LoadToHeatingSetPoint > 0.0 && LoadToCoolingSetPoint > 0.0) {
            totalLoad = LoadToHeatingSetPoint;
            ZoneSetPoint = zoneTstatSetpt.setptLo;
        } else if (LoadToHeatingSetPoint < 0.0 && LoadToCoolingSetPoint < 0.0) {
            totalLoad = LoadToCoolingSetPoint;
            ZoneSetPoint = zoneTstatSetpt.setptHi;
        } else if (LoadToHeatingSetPoint <= 0.0 && LoadToCoolingSetPoint >= 0.0) { // deadband includes zero loads
            // this turns out to cause instabilities sometimes? that lead to setpoint errors if predictor is off.
            totalLoad = 0.0;
            if (zoneNodeNum > 0) {
                ZoneSetPoint = state.dataLoopNodes->Node(zoneNodeNum).Temp;
                ZoneSetPoint = max(ZoneSetPoint, zoneTstatSetpt.setptLo); // trap out of deadband
                ZoneSetPoint = min(ZoneSetPoint, zoneTstatSetpt.setptHi); // trap out of deadband
            }
            thisDeadBandOrSetBack = true;
        } else { // this should never occur!
            ShowSevereError(
                state, "DualSetPointWithDeadBand: Unanticipated combination of heating and cooling loads - report to EnergyPlus Development Team");
            ShowContinueErrorTimeStamp(state, format("occurs in Zone={}", thisZone.Name));
            ShowContinueError(state,
                              format("LoadToHeatingSetPoint={:.3R}, LoadToCoolingSetPoint={:.3R}", LoadToHeatingSetPoint, LoadToCoolingSetPoint));
            ShowContinueError(state, format("Zone Heating Set-point={:.2R}", zoneTstatSetpt.setptLo));
            ShowContinueError(state, format("Zone Cooling Set-point={:.2R}", zoneTstatSetpt.setptHi));
            ShowContinueError(state, format("Zone TempDepZnLd={:.2R}", this->tempDepLoad));
            ShowContinueError(state, format("Zone TempIndZnLd={:.2R}", this->tempIndLoad));
            ShowContinueError(state, format("Zone ThermostatSetPoint={:.2R}", zoneTstatSetpt.setpt));

            ShowFatalError(state, "Program terminates due to above conditions.");
        }
    } break;

    default: {
    } break;
    } // swtich (setptType)

    int systemNodeNumber = 0;
    int stageNum = 0;
    if (spaceNum > 0) {
        systemNodeNumber = state.dataHeatBal->space(spaceNum).SystemZoneNodeNumber;
        stageNum = state.dataZoneEnergyDemand->spaceSysEnergyDemand(spaceNum).StageNum;
        assert(stageNum == state.dataZoneEnergyDemand->ZoneSysEnergyDemand(zoneNum).StageNum);
    } else {
        systemNodeNumber = thisZone.SystemZoneNodeNumber;
        stageNum = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(zoneNum).StageNum;
    }
    // Staged control zone
    if (s_ztpc->NumStageCtrZone > 0) {
        if (state.dataZoneCtrls->StageZoneLogic(zoneNum)) {
            if (stageNum == 0) { // No load
                LoadToHeatingSetPoint = 0.0;
                LoadToCoolingSetPoint = 0.0;
                totalLoad = 0.0;
                if (systemNodeNumber > 0) {
                    ZoneSetPoint = state.dataLoopNodes->Node(systemNodeNumber).Temp;
                    ZoneSetPoint = max(ZoneSetPoint, zoneTstatSetpt.setptLo); // trap out of deadband
                    ZoneSetPoint = min(ZoneSetPoint, zoneTstatSetpt.setptHi); // trap out of deadband
                }
                thisDeadBandOrSetBack = true;

            } else if (stageNum < 0) { // Cooling load
                switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
                case DataHeatBalance::SolutionAlgo::ThirdOrder: {
                    LoadToCoolingSetPoint = (this->tempDepLoad * (zoneTstatSetpt.setptHi) - this->tempIndLoad);
                } break;

                case DataHeatBalance::SolutionAlgo::AnalyticalSolution: {
                    if (this->tempDepLoad == 0.0) { // B=0
                        LoadToCoolingSetPoint = this->AirPowerCap * (zoneTstatSetpt.setptHi - this->T1) - this->tempIndLoad;
                    } else {
                        Real64 const exp_700_TA(std::exp(min(700.0, -this->tempDepLoad / this->AirPowerCap)));
                        LoadToCoolingSetPoint =
                            this->tempDepLoad * (zoneTstatSetpt.setptHi - this->T1 * exp_700_TA) / (1.0 - exp_700_TA) - this->tempIndLoad;
                    }
                } break;

                case DataHeatBalance::SolutionAlgo::EulerMethod: {
                    LoadToCoolingSetPoint =
                        this->AirPowerCap * (zoneTstatSetpt.setptHi - this->T1) + this->tempDepLoad * zoneTstatSetpt.setptHi - this->tempIndLoad;
                } break;
                default: {
                    assert(false);
                } break;
                } // switch (Algo)

                totalLoad = LoadToCoolingSetPoint;
                ZoneSetPoint = zoneTstatSetpt.setptHi;
                LoadToHeatingSetPoint = LoadToCoolingSetPoint;
                if ((totalLoad) >= 0.0) {
                    thisDeadBandOrSetBack = true;
                }
            } else { // Heating load
                switch (state.dataHeatBal->ZoneAirSolutionAlgo) {
                case DataHeatBalance::SolutionAlgo::ThirdOrder: {
                    LoadToHeatingSetPoint = (this->tempDepLoad * zoneTstatSetpt.setptLo - this->tempIndLoad);
                } break;

                case DataHeatBalance::SolutionAlgo::AnalyticalSolution: {
                    if (this->tempDepLoad == 0.0) { // B=0
                        LoadToHeatingSetPoint = this->AirPowerCap * (zoneTstatSetpt.setptLo - this->T1) - this->tempIndLoad;
                    } else {
                        Real64 const exp_700_TA(std::exp(min(700.0, -this->tempDepLoad / this->AirPowerCap)));
                        LoadToHeatingSetPoint =
                            this->tempDepLoad * (zoneTstatSetpt.setptLo - this->T1 * exp_700_TA) / (1.0 - exp_700_TA) - this->tempIndLoad;
                    }
                } break;

                case DataHeatBalance::SolutionAlgo::EulerMethod: {
                    LoadToHeatingSetPoint =
                        this->AirPowerCap * (zoneTstatSetpt.setptLo - this->T1) + this->tempDepLoad * (zoneTstatSetpt.setptLo) - this->tempIndLoad;
                } break;
                default: {
                    assert(false);
                } break;
                } // switch (Algo)

                totalLoad = LoadToHeatingSetPoint;
                ZoneSetPoint = zoneTstatSetpt.setptLo;
                LoadToCoolingSetPoint = LoadToHeatingSetPoint;
                if ((totalLoad) <= 0.0) {
                    thisDeadBandOrSetBack = true;
                }
            }
        }
    }

    // If the ZoneNodeNum has been set for a Controlled Zone, then the zone setpoint is placed on the node.
    if (zoneNodeNum > 0) {
        state.dataLoopNodes->Node(zoneNodeNum).TempSetPoint = ZoneSetPoint;
    }

    state.dataZoneEnergyDemand->Setback(zoneNum) = (ZoneSetPoint > this->setPointLast);

    this->setPointLast = ZoneSetPoint;
    state.dataHeatBalFanSys->zoneTstatSetpts(zoneNum).setpt = ZoneSetPoint; // needed to fix Issue # 5048
    state.dataZoneEnergyDemand->DeadBandOrSetback(zoneNum) = thisDeadBandOrSetBack;
    state.dataZoneEnergyDemand->CurDeadBandOrSetback(zoneNum) = thisDeadBandOrSetBack;

    // Apply the Zone Multiplier and Load Correction factor as needed
    if (spaceNum > 0) {
        state.dataZoneEnergyDemand->spaceSysEnergyDemand(spaceNum).reportSensibleLoadsZoneMultiplier(
            state, zoneNum, totalLoad, LoadToHeatingSetPoint, LoadToCoolingSetPoint);
    } else {
        state.dataZoneEnergyDemand->ZoneSysEnergyDemand(zoneNum).reportSensibleLoadsZoneMultiplier(
            state, zoneNum, totalLoad, LoadToHeatingSetPoint, LoadToCoolingSetPoint);
    }
}
} // namespace EnergyPlus::ZoneTempPredictorCorrector
