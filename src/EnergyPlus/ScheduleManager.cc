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
#include <limits>
#include <map>

// ObjexxFCL Headers
#include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/CommandLineInterface.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/FileSystem.hh>
#include <EnergyPlus/General.hh>
// #include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/CsvParser.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/StringUtilities.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WeatherManager.hh>

namespace EnergyPlus {

namespace Sched {
    // Module containing the Schedule Manager routines

    // MODULE INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   September 1997
    //       MODIFIED       January 2003 -- added sub-hourly schedule possibility (and interval scheduling)
    //                      J. Glazer January 2005 -- added Schedule:File
    //                      Michael Wetter February 2010 -- added Schedule for external Interface
    //                      L Lawrie - October 2012 - added sub-hourly option for Schedule:File

    // PURPOSE OF THIS MODULE:
    // To provide the capabilities of getting the schedule data from the input,
    // validating it, and storing it in such a manner that the schedule manager
    // can provide the scheduling value needs for the simulation.

    // REFERENCES:
    // Proposal for Schedule Manager in EnergyPlus (Rick Strand)

    // MODULE PARAMETER DEFINITIONS
    int GetScheduleTypeNum(EnergyPlusData &state, std::string const &name)
    {
        auto const &s_sched = state.dataSched;
        for (int i = 0; i < (int)s_sched->scheduleTypes.size(); ++i) {
            if (s_sched->scheduleTypes[i]->Name == name) {
                return i;
            }
        }
        return SchedNum_Invalid;
    }

    Real64 ScheduleBase::getMinVal(EnergyPlusData &state)
    {
        if (!isMinMaxSet) {
            setMinMaxVals(state);
        }
        return minVal;
    }

    Real64 ScheduleBase::getMaxVal(EnergyPlusData &state)
    {
        if (!isMinMaxSet) {
            setMinMaxVals(state);
        }
        return maxVal;
    }

    // Day types are 1-based for EMS and output and other uses, so add a dummy
    constexpr std::array<std::string_view, (int)DayType::Num> dayTypeNames = {"Unused",
                                                                              "Sunday",
                                                                              "Monday",
                                                                              "Tuesday",
                                                                              "Wednesday",
                                                                              "Thursday",
                                                                              "Friday",
                                                                              "Saturday",
                                                                              "Holiday",
                                                                              "SummerDesignDay",
                                                                              "WinterDesignDay",
                                                                              "CustomDay1",
                                                                              "CustomDay2"};

    constexpr std::array<std::string_view, (int)DayType::Num> dayTypeNamesUC = {"UNUSED",
                                                                                "SUNDAY",
                                                                                "MONDAY",
                                                                                "TUESDAY",
                                                                                "WEDNESDAY",
                                                                                "THURSDAY",
                                                                                "FRIDAY",
                                                                                "SATURDAY",
                                                                                "HOLIDAY",
                                                                                "SUMMERDESIGNDAY",
                                                                                "WINTERDESIGNDAY",
                                                                                "CUSTOMDAY1",
                                                                                "CUSTOMDAY2"};

    static constexpr std::array<std::string_view, (int)LimitUnits::Num> limitUnitNamesUC = {"DIMENSIONLESS",
                                                                                            "TEMPERATURE",
                                                                                            "DELTATEMPERATURE",
                                                                                            "PRECIPITATIONRATE",
                                                                                            "ANGLE",
                                                                                            "CONVECTIONCOEFFICIENT",
                                                                                            "ACTIVITYLEVEL",
                                                                                            "VELOCITY",
                                                                                            "CAPACITY",
                                                                                            "POWER",
                                                                                            "AVAILABILITY",
                                                                                            "PERCENT",
                                                                                            "CONTROL",
                                                                                            "MODE"};

    constexpr std::array<std::string_view, (int)ReportLevel::Num> reportLevelNames = {"Hourly", "Timestep"};
    constexpr std::array<std::string_view, (int)ReportLevel::Num> reportLevelNamesUC = {"HOURLY", "TIMESTEP"};
    constexpr std::array<std::string_view, (int)Interpolation::Num> interpolationNames = {"No", "Average", "Linear"};
    constexpr std::array<std::string_view, (int)Interpolation::Num> interpolationNamesUC = {"NO", "AVERAGE", "LINEAR"};

    bool DaySchedule::checkValsForLimitViolations(EnergyPlusData &state) const
    {
        auto &s_sched = state.dataSched;

        if (this->schedTypeNum == SchedNum_Invalid) {
            return false;
        }
        auto *schedType = s_sched->scheduleTypes[this->schedTypeNum];
        if (!schedType->isLimited) {
            return false;
        }

        for (int i = 0; i < Constant::iHoursInDay * state.dataGlobal->TimeStepsInHour; ++i) {
            if (this->tsVals[i] < schedType->minVal || this->tsVals[i] > schedType->maxVal) {
                return true;
            }
        }

        return false;
    } // ScheduleDay::checkValsForLimitViolations()

    bool DaySchedule::checkValsForBadIntegers(EnergyPlusData &state) const
    {
        auto &s_sched = state.dataSched;
        if (this->schedTypeNum == SchedNum_Invalid) {
            return false;
        }
        auto *schedType = s_sched->scheduleTypes[this->schedTypeNum];
        if (schedType->isReal) {
            return false;
        }
        // Make sure each is integer
        for (int i = 0; i < Constant::iHoursInDay * state.dataGlobal->TimeStepsInHour; ++i) {
            if (this->tsVals[i] != int(this->tsVals[i])) {
                return true;
            }
        }
        return false;
    } // ScheduleDay::checkValsForBadIntegers()

    void DaySchedule::populateFromMinuteVals(EnergyPlusData &state, std::array<Real64, Constant::iMinutesInDay> const &minuteVals)
    {
        auto &s_glob = state.dataGlobal;
        if (this->interpolation == Interpolation::Average) {
            for (int hr = 0; hr < Constant::iHoursInDay; ++hr) {
                int begMin = 0;
                int endMin = s_glob->MinutesInTimeStep - 1;
                for (int ts = 0; ts < s_glob->TimeStepsInHour; ++ts) {
                    Real64 accum = 0.0;
                    for (int iMin = begMin; iMin <= endMin; ++iMin) {
                        accum += minuteVals[hr * Constant::iMinutesInHour + iMin];
                    }
                    this->tsVals[hr * s_glob->TimeStepsInHour + ts] = accum / double(s_glob->MinutesInTimeStep);
                    this->sumTsVals += this->tsVals[hr * s_glob->TimeStepsInHour + ts];
                    begMin = endMin + 1;
                    endMin += s_glob->MinutesInTimeStep;
                }
            }
        } else {
            for (int hr = 0; hr < Constant::iHoursInDay; ++hr) {
                int endMinute = s_glob->MinutesInTimeStep - 1;
                for (int ts = 0; ts < s_glob->TimeStepsInHour; ++ts) {
                    this->tsVals[hr * s_glob->TimeStepsInHour + ts] = minuteVals[hr * Constant::iMinutesInHour + endMinute];
                    this->sumTsVals += this->tsVals[hr * s_glob->TimeStepsInHour + ts];
                    endMinute += s_glob->MinutesInTimeStep;
                }
            }
        }
    } // ScheduleDay::populateFromHrMinVals()

    ScheduleConstant *AddScheduleConstant(EnergyPlusData &state, std::string const &name, Real64 value)
    {
        auto const &s_sched = state.dataSched;
        auto const &s_glob = state.dataGlobal;

        auto *sched = new ScheduleConstant;
        sched->Name = name;
        sched->type = SchedType::Constant;
        sched->Num = (int)s_sched->schedules.size();
        sched->currentVal = value;
        // When InitConstantScheduleData is called, TimeStepsInHour is 0, so we ensure 24
        sched->tsVals.assign(Constant::iHoursInDay * max(1, s_glob->TimeStepsInHour), value);

        s_sched->schedules.push_back(sched);
        s_sched->scheduleMap.insert_or_assign(Util::makeUPPER(sched->Name), sched->Num);

        return sched;
    } // AddScheduleConstant()

    ScheduleDetailed *AddScheduleDetailed(EnergyPlusData &state, std::string const &name)
    {
        auto const &s_sched = state.dataSched;

        auto *sched = new ScheduleDetailed;
        sched->Name = name;

        sched->Num = (int)s_sched->schedules.size();
        s_sched->schedules.push_back(sched);
        s_sched->scheduleMap.insert_or_assign(Util::makeUPPER(sched->Name), sched->Num);

        sched->type = SchedType::Year;
        return sched;
    } // AddScheduleDetailed()

    DaySchedule *AddDaySchedule(EnergyPlusData &state, std::string const &name)
    {
        auto &s_glob = state.dataGlobal;
        auto &s_sched = state.dataSched;

        auto *daySched = new DaySchedule;
        daySched->Name = name;

        daySched->Num = (int)s_sched->daySchedules.size();
        s_sched->daySchedules.push_back(daySched);
        s_sched->dayScheduleMap.insert_or_assign(Util::makeUPPER(daySched->Name), daySched->Num);

        // When InitConstantScheduleData is called, TimeStepsInHour is 0, so we ensure 24
        daySched->tsVals.assign(Constant::iHoursInDay * max(1, s_glob->TimeStepsInHour), 0.0);

        return daySched;
    } // AddDaySchedule()

    WeekSchedule *AddWeekSchedule(EnergyPlusData &state, std::string const &name)
    {
        auto const &s_sched = state.dataSched;

        auto *weekSched = new WeekSchedule;
        weekSched->Name = name;

        // Fill the dayScheds with the Missing Day Schedule (Always Off)
        for (int iDayType = 1; iDayType < (int)DayType::Num; ++iDayType) {
            weekSched->dayScheds[iDayType] = s_sched->daySchedules[SchedNum_AlwaysOff];
        }

        weekSched->Num = (int)s_sched->weekSchedules.size();
        s_sched->weekSchedules.push_back(weekSched);
        s_sched->weekScheduleMap.insert_or_assign(Util::makeUPPER(weekSched->Name), weekSched->Num);

        return weekSched;
    } // AddWeekSchedule()

    void InitConstantScheduleData(EnergyPlusData &state)
    {
        // Create ScheduleAlwaysOn and ScheduleAlwaysOff
        // Create constant schedules
        auto *schedOff = AddScheduleConstant(state, "Constant-0.0", 0.0);
        assert(schedOff->Num == SchedNum_AlwaysOff);
        schedOff->isUsed = true; // Suppress unused warnings

        auto *schedOn = AddScheduleConstant(state, "Constant-1.0", 1.0);
        assert(schedOn->Num == SchedNum_AlwaysOn);
        schedOn->isUsed = true; // Suppress unused warnings

        auto *missingDaySchedule = AddDaySchedule(state, "MissingDaySchedule-0.0");
        assert(missingDaySchedule->Num == SchedNum_AlwaysOff);
        missingDaySchedule->isUsed = true;
    }

    void ProcessScheduleInput(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   September 1997
        //       MODIFIED       Rui Zhang February 2010

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine processes the schedules input for EnergyPlus.

        // METHODOLOGY EMPLOYED:
        // Uses the standard get routines in the InputProcessor.

        // Using/Aliasing
        using DataStringGlobals::CharComma;
        using DataStringGlobals::CharSemicolon;
        using DataStringGlobals::CharSpace;
        using DataStringGlobals::CharTab;
        using DataSystemVariables::CheckForActualFilePath;
        using General::ProcessDateString;

        // Locals
        // SUBROUTINE PARAMETER DEFINITIONS:
        constexpr std::string_view routineName = "ProcessScheduleInput";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        Array1D_string Alphas;
        Array1D_string cAlphaFields;
        Array1D_string cNumericFields;
        Array1D<Real64> Numbers;
        Array1D_bool lAlphaBlanks;
        Array1D_bool lNumericBlanks;
        int NumAlphas;
        int NumNumbers;
        int Status;

        int EndMonth;
        int EndDay;
        int StartPointer;
        int EndPointer;
        int NumPointer;
        bool ErrorsFound(false);
        bool NumErrorFlag;

        std::string CFld; // Character field for error message
        //  CHARACTER(len=20) CFld1        ! Character field for error message

        std::array<Real64, Constant::iMinutesInDay> minuteVals;
        std::array<bool, Constant::iMinutesInDay> setMinuteVals;

        int NumFields;
        //  LOGICAL RptSchedule

        int MinutesPerItem;
        int NumExpectedItems;
        std::array<bool, (int)DayType::Num> allDays;
        std::array<bool, (int)DayType::Num> theseDays;
        bool ErrorHere;
        int SchNum;
        int WkCount;
        int DyCount;
        int NumField;
        int Count;
        Weather::DateType PDateType;
        int PWeekDay;
        int ThruField;
        int UntilFld;
        int xxcount;
        //  REAL(r64) tempval
        std::string CurrentThrough;
        std::string LastFor;
        std::string errmsg;
        // for SCHEDULE:FILE
        int rowCnt;

        std::string subString;
        int MaxNums1;
        char ColumnSep;
        int rowLimitCount;
        int skiprowCount;
        int curcolCount;
        int numerrors = 0;

        auto const &s_glob = state.dataGlobal;
        auto const &s_ip = state.dataInputProcessing->inputProcessor;
        auto const &s_sched = state.dataSched;

        if (s_sched->ScheduleInputProcessed) {
            return;
        }

        s_sched->ScheduleInputProcessed = true;

        int MaxNums = 1; // Need at least 1 number because it's used as a local variable in the Schedule Types loop
        int MaxAlps = 0;

        std::string CurrentModuleObject = "ScheduleTypeLimits";
        int NumScheduleTypes = s_ip->getNumObjectsFound(state, CurrentModuleObject);
        if (NumScheduleTypes > 0) {
            s_ip->getObjectDefMaxArgs(state, CurrentModuleObject, Count, NumAlphas, NumNumbers);
            MaxNums = max(MaxNums, NumNumbers);
            MaxAlps = max(MaxAlps, NumAlphas);
        }
        CurrentModuleObject = "Schedule:Day:Hourly";
        int NumHrDaySchedules = s_ip->getNumObjectsFound(state, CurrentModuleObject);
        if (NumHrDaySchedules > 0) {
            s_ip->getObjectDefMaxArgs(state, CurrentModuleObject, Count, NumAlphas, NumNumbers);
            MaxNums = max(MaxNums, NumNumbers);
            MaxAlps = max(MaxAlps, NumAlphas);
        }
        CurrentModuleObject = "Schedule:Day:Interval";
        int NumIntDaySchedules = s_ip->getNumObjectsFound(state, CurrentModuleObject);
        if (NumIntDaySchedules > 0) {
            s_ip->getObjectDefMaxArgs(state, CurrentModuleObject, Count, NumAlphas, NumNumbers);
            MaxNums = max(MaxNums, NumNumbers);
            MaxAlps = max(MaxAlps, NumAlphas);
        }
        CurrentModuleObject = "Schedule:Day:List";
        int NumLstDaySchedules = s_ip->getNumObjectsFound(state, CurrentModuleObject);
        if (NumLstDaySchedules > 0) {
            s_ip->getObjectDefMaxArgs(state, CurrentModuleObject, Count, NumAlphas, NumNumbers);
            MaxNums = max(MaxNums, NumNumbers);
            MaxAlps = max(MaxAlps, NumAlphas);
        }
        CurrentModuleObject = "Schedule:Week:Daily";
        int NumRegWeekSchedules = s_ip->getNumObjectsFound(state, CurrentModuleObject);
        if (NumRegWeekSchedules > 0) {
            s_ip->getObjectDefMaxArgs(state, CurrentModuleObject, Count, NumAlphas, NumNumbers);
            MaxNums = max(MaxNums, NumNumbers);
            MaxAlps = max(MaxAlps, NumAlphas);
        }
        CurrentModuleObject = "Schedule:Week:Compact";
        int NumCptWeekSchedules = s_ip->getNumObjectsFound(state, CurrentModuleObject);
        if (NumCptWeekSchedules > 0) {
            s_ip->getObjectDefMaxArgs(state, CurrentModuleObject, Count, NumAlphas, NumNumbers);
            MaxNums = max(MaxNums, NumNumbers);
            MaxAlps = max(MaxAlps, NumAlphas);
        }
        CurrentModuleObject = "Schedule:Year";
        int NumRegSchedules = s_ip->getNumObjectsFound(state, CurrentModuleObject);
        if (NumRegSchedules > 0) {
            s_ip->getObjectDefMaxArgs(state, CurrentModuleObject, Count, NumAlphas, NumNumbers);
            MaxNums = max(MaxNums, NumNumbers);
            MaxAlps = max(MaxAlps, NumAlphas);
        }
        CurrentModuleObject = "Schedule:Compact";
        int NumCptSchedules = s_ip->getNumObjectsFound(state, CurrentModuleObject);
        if (NumCptSchedules > 0) {
            s_ip->getObjectDefMaxArgs(state, CurrentModuleObject, Count, NumAlphas, NumNumbers);
            MaxNums = max(MaxNums, NumNumbers);
            MaxAlps = max(MaxAlps, NumAlphas + 1);
        }
        CurrentModuleObject = "Schedule:File";
        int NumCommaFileSchedules = s_ip->getNumObjectsFound(state, CurrentModuleObject);
        if (NumCommaFileSchedules > 0) {
            s_ip->getObjectDefMaxArgs(state, CurrentModuleObject, Count, NumAlphas, NumNumbers);
            MaxNums = max(MaxNums, NumNumbers);
            MaxAlps = max(MaxAlps, NumAlphas);
        }

        CurrentModuleObject = "Schedule:Constant";
        int NumConstantSchedules = s_ip->getNumObjectsFound(state, CurrentModuleObject);
        if (NumConstantSchedules > 0) {
            s_ip->getObjectDefMaxArgs(state, CurrentModuleObject, Count, NumAlphas, NumNumbers);
            MaxNums = max(MaxNums, NumNumbers);
            MaxAlps = max(MaxAlps, NumAlphas);
        }
        CurrentModuleObject = "ExternalInterface:Schedule";
        int NumExternalInterfaceSchedules = s_ip->getNumObjectsFound(state, CurrentModuleObject);
        // added for FMI
        if (NumExternalInterfaceSchedules > 0) {
            s_ip->getObjectDefMaxArgs(state, CurrentModuleObject, Count, NumAlphas, NumNumbers);
            MaxNums = max(MaxNums, NumNumbers);
            MaxAlps = max(MaxAlps, NumAlphas + 1);
        }
        // added for FMU Import
        CurrentModuleObject = "ExternalInterface:FunctionalMockupUnitImport:To:Schedule";
        int NumExternalInterfaceFunctionalMockupUnitImportSchedules = s_ip->getNumObjectsFound(state, CurrentModuleObject);
        if (NumExternalInterfaceFunctionalMockupUnitImportSchedules > 0) {
            s_ip->getObjectDefMaxArgs(state, CurrentModuleObject, Count, NumAlphas, NumNumbers);
            MaxNums = max(MaxNums, NumNumbers);
            MaxAlps = max(MaxAlps, NumAlphas + 1);
        }
        // added for FMU Export
        CurrentModuleObject = "ExternalInterface:FunctionalMockupUnitExport:To:Schedule";
        int NumExternalInterfaceFunctionalMockupUnitExportSchedules = s_ip->getNumObjectsFound(state, CurrentModuleObject);
        if (NumExternalInterfaceFunctionalMockupUnitExportSchedules > 0) {
            s_ip->getObjectDefMaxArgs(state, CurrentModuleObject, Count, NumAlphas, NumNumbers);
            MaxNums = max(MaxNums, NumNumbers);
            MaxAlps = max(MaxAlps, NumAlphas + 1);
        }
        CurrentModuleObject = "Output:Schedules";
        s_ip->getObjectDefMaxArgs(state, CurrentModuleObject, Count, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlps = max(MaxAlps, NumAlphas);

        Alphas.allocate(MaxAlps); // Maximum Alphas possible
        cAlphaFields.allocate(MaxAlps);
        cNumericFields.allocate(MaxNums);
        Numbers.dimension(MaxNums, 0.0); // Maximum Numbers possible
        lAlphaBlanks.dimension(MaxAlps, true);
        lNumericBlanks.dimension(MaxNums, true);

        // Prescan to determine extra day and week schedules due to compact schedule input
        CurrentModuleObject = "Schedule:Compact";
        MaxNums1 = 0;

        for (int LoopIndex = 1; LoopIndex <= NumCptSchedules; ++LoopIndex) {
            s_ip->getObjectItem(state, CurrentModuleObject, LoopIndex, Alphas, NumAlphas, Numbers, NumNumbers, Status);
            // # 'THROUGH" => Number of additional week schedules
            // # 'FOR' => Number of additional day schedules
            for (Count = 3; Count <= NumAlphas; ++Count) {
                if (has_prefix(Alphas(Count), "UNTIL")) {
                    ++MaxNums1;
                }
            }
        }
        if (MaxNums1 > MaxNums) {
            MaxNums = MaxNums1;
            cNumericFields.deallocate();
            Numbers.deallocate();
            lNumericBlanks.deallocate();
            cNumericFields.allocate(MaxNums);
            Numbers.dimension(MaxNums, 0.0); // Maximum Numbers possible
            lNumericBlanks.dimension(MaxNums, true);
        }

        // add week and day schedules for each FILE:COMMA schedule

        CurrentModuleObject = "Schedule:File:Shading";
        int NumCommaFileShading = s_ip->getNumObjectsFound(state, CurrentModuleObject);
        NumAlphas = 0;
        NumNumbers = 0;
        if (NumCommaFileShading > 1) {
            ShowWarningError(state, format("{}: More than 1 occurrence of this object found, only first will be used.", CurrentModuleObject));
        }

        std::map<fs::path, nlohmann::json>::iterator schedule_file_shading_result;
        if (NumCommaFileShading != 0) {
            s_ip->getObjectItem(state,
                                CurrentModuleObject,
                                1,
                                Alphas,
                                NumAlphas,
                                Numbers,
                                NumNumbers,
                                Status,
                                lNumericBlanks,
                                lAlphaBlanks,
                                cAlphaFields,
                                cNumericFields);
            std::string ShadingSunlitFracFileName = Alphas(1);

            std::string contextString = CurrentModuleObject + ", " + cAlphaFields(1) + ": ";
            state.files.TempFullFilePath.filePath = CheckForActualFilePath(state, ShadingSunlitFracFileName, contextString);

            if (state.files.TempFullFilePath.filePath.empty()) {
                ShowFatalError(state, "Program terminates due to previous condition.");
            }

            if (state.dataEnvrn->CurrentYearIsLeapYear) {
                rowLimitCount = 366 * Constant::iHoursInDay * s_glob->TimeStepsInHour;
            } else {
                rowLimitCount = 365 * Constant::iHoursInDay * s_glob->TimeStepsInHour;
            }
            ColumnSep = CharComma;

            schedule_file_shading_result = s_sched->UniqueProcessedExternalFiles.find(state.files.TempFullFilePath.filePath);
            if (schedule_file_shading_result == s_sched->UniqueProcessedExternalFiles.end()) {

                FileSystem::FileTypes const ext = FileSystem::getFileType(state.files.TempFullFilePath.filePath);
                if (FileSystem::is_flat_file_type(ext)) {
                    auto const schedule_data = FileSystem::readFile(state.files.TempFullFilePath.filePath);
                    CsvParser csvParser;
                    skiprowCount = 1; // make sure to parse header row only for Schedule:File:Shading
                    auto it = s_sched->UniqueProcessedExternalFiles.emplace(state.files.TempFullFilePath.filePath,
                                                                            csvParser.decode(schedule_data, ColumnSep, skiprowCount));
                    if (csvParser.hasErrors()) {
                        for (const auto &[error, isContinued] : csvParser.errors()) {
                            if (isContinued) {
                                ShowContinueError(state, error);
                            } else {
                                ShowSevereError(state, error);
                            }
                        }
                        ShowContinueError(state, fmt::format("Error Occurred in {}", state.files.TempFullFilePath.filePath));
                        ShowFatalError(state, "Program terminates due to previous condition.");
                    }
                    schedule_file_shading_result = it.first;
                } else if (FileSystem::is_all_json_type(ext)) {
                    auto schedule_data = FileSystem::readJSON(state.files.TempFullFilePath.filePath);
                    auto it = // (AUTO_OK_ITER)
                        s_sched->UniqueProcessedExternalFiles.emplace(state.files.TempFullFilePath.filePath, std::move(schedule_data));
                    schedule_file_shading_result = it.first;
                } else {
                    ShowSevereError(state,
                                    fmt::format(R"({}: {}="{}", {}="{}" has an unknown file extension and cannot be read by this program.)",
                                                routineName,
                                                CurrentModuleObject,
                                                Alphas(1),
                                                cAlphaFields(3),
                                                Alphas(3)));
                    ShowFatalError(state, "Program terminates due to previous condition.");
                }
            }

            auto const &column_json = schedule_file_shading_result->second["values"].at(0); // assume there is at least 1 column
            rowCnt = column_json.size();
            int NumCSVAllColumnsSchedules =
                schedule_file_shading_result->second["header"].get<std::set<std::string>>().size() - 1; // -1 to account for timestamp column

            if (schedule_file_shading_result->second["header"].back().get<std::string>() == "()") {
                ShowWarningError(state,
                                 format("{}: {}=\"{}\" Removing last column of the CSV since it has '()' for the surface name.",
                                        routineName,
                                        CurrentModuleObject,
                                        Alphas(1)));
                ShowContinueError(state, "This was a problem in E+ 22.2.0 and below, consider removing it from the file to suppress this warning.");
                schedule_file_shading_result->second["header"].erase(NumCSVAllColumnsSchedules);
                assert(schedule_file_shading_result->second["header"].size() == schedule_file_shading_result->second["values"].size());
                --NumCSVAllColumnsSchedules;
            }

            if (rowCnt != rowLimitCount) {
                if (rowCnt < rowLimitCount) {
                    ShowSevereError(state, format("{}: {}=\"{}\" {} data values read.", routineName, CurrentModuleObject, Alphas(1), rowCnt));
                } else if (rowCnt > rowLimitCount) {
                    ShowSevereError(state, format("{}: {}=\"{}\" too many data values read.", routineName, CurrentModuleObject, Alphas(1)));
                }
                ShowContinueError(
                    state,
                    format("Number of rows in the shading file must be a full year multiplied by the simulation TimeStep: {}.", rowLimitCount));
                ShowFatalError(state, "Program terminates due to previous condition.");
            }

            // schedule values have been filled into the CSVAllColumnNameAndValues map.
            s_sched->ScheduleFileShadingProcessed = true;

            if (numerrors > 0) {
                ShowWarningError(
                    state,
                    format(
                        "{}:{}=\"{}\" {} records had errors - these values are set to 0.", routineName, CurrentModuleObject, Alphas(1), numerrors));
            }
        }

        //!  Most initializations in the schedule data structures are taken care of in
        //!  the definitions (see above)

        print(state.files.audit.ensure_open(state, "ProcessScheduleInput", state.files.outputControl.audit),
              "{}\n",
              "  Processing Schedule Input -- Start");

        //!! Get Schedule Types

        CurrentModuleObject = "ScheduleTypeLimits";
        for (int Loop = 1; Loop <= NumScheduleTypes; ++Loop) {
            s_ip->getObjectItem(state,
                                CurrentModuleObject,
                                Loop,
                                Alphas,
                                NumAlphas,
                                Numbers,
                                NumNumbers,
                                Status,
                                lNumericBlanks,
                                lAlphaBlanks,
                                cAlphaFields,
                                cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (s_sched->scheduleTypeMap.find(Alphas(1)) != s_sched->scheduleTypeMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
                continue;
            }

            auto *schedType = new ScheduleType;
            schedType->Name = Alphas(1);

            schedType->Num = (int)s_sched->scheduleTypes.size();
            s_sched->scheduleTypes.push_back(schedType);
            s_sched->scheduleTypeMap.insert_or_assign(schedType->Name, schedType->Num);

            schedType->isLimited = !lNumericBlanks(1) && !lNumericBlanks(2);

            if (!lNumericBlanks(1)) {
                schedType->minVal = Numbers(1);
            }
            if (!lNumericBlanks(2)) {
                schedType->maxVal = Numbers(2);
            }

            if (schedType->isLimited) {
                if (Alphas(2) == "DISCRETE" || Alphas(2) == "INTEGER") {
                    schedType->isReal = false;
                } else if (Alphas(2) == "CONTINUOUS" || Alphas(2) == "REAL") {
                    schedType->isReal = true;
                } else {
                    ShowSevereInvalidKey(state, eoh, cAlphaFields(2), Alphas(2));
                    ErrorsFound = true;
                }
            }

            if (NumAlphas >= 3 && !lAlphaBlanks(3)) {
                schedType->limitUnits = static_cast<LimitUnits>(getEnumValue(limitUnitNamesUC, Alphas(3)));
                if (schedType->limitUnits == LimitUnits::Invalid) {
                    ShowSevereInvalidKey(state, eoh, cAlphaFields(3), Alphas(3));
                    ErrorsFound = true;
                }
            }

            if (schedType->isLimited && schedType->minVal > schedType->maxVal) {
                if (schedType->isReal) {
                    ShowSevereCustom(
                        state, eoh, format("{} [{:.2R}] > {} [{:.2R}].", cNumericFields(1), schedType->minVal, cNumericFields(2), schedType->maxVal));
                } else {
                    ShowSevereCustom(
                        state, eoh, format("{} [{:.0R}] > {} [{:.0R}].", cNumericFields(1), schedType->minVal, cNumericFields(2), schedType->maxVal));
                }
                ShowContinueError(state, "  Other warning/severes about schedule values may appear.");
            }
        } // for (Loop)

        //!! Get Day Schedules (all types)

        //!!=> Get "DAYSCHEDULE" (Hourly)

        Count = 0;
        CurrentModuleObject = "Schedule:Day:Hourly";
        for (int Loop = 1; Loop <= NumHrDaySchedules; ++Loop) {
            s_ip->getObjectItem(state,
                                CurrentModuleObject,
                                Loop,
                                Alphas,
                                NumAlphas,
                                Numbers,
                                NumNumbers,
                                Status,
                                lNumericBlanks,
                                lAlphaBlanks,
                                cAlphaFields,
                                cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (s_sched->dayScheduleMap.find(Alphas(1)) != s_sched->dayScheduleMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
                continue;
            }

            auto *daySched = AddDaySchedule(state, Alphas(1));

            // Validate ScheduleType
            if (lAlphaBlanks(2)) {
                ShowWarningEmptyField(state, eoh, cAlphaFields(2));
                ShowContinueError(state, "Schedule will not be validated.");
            } else if ((daySched->schedTypeNum = GetScheduleTypeNum(state, Alphas(2))) == SchedNum_Invalid) {
                ShowWarningItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ShowContinueError(state, "Schedule will not be validated.");
            }

            daySched->interpolation = Interpolation::No;

            for (int hr = 0; hr < Constant::iHoursInDay; ++hr) {
                for (int ts = 0; ts < s_glob->TimeStepsInHour; ++ts) {
                    daySched->tsVals[hr * s_glob->TimeStepsInHour + ts] = Numbers(hr + 1);
                    daySched->sumTsVals += daySched->tsVals[hr * s_glob->TimeStepsInHour + ts];
                }
            }

            if (daySched->checkValsForLimitViolations(state)) {
                ShowWarningCustom(state, eoh, format("Values are outside of range for {}={}", cAlphaFields(2), Alphas(2)));
            }

            if (daySched->checkValsForBadIntegers(state)) {
                ShowWarningCustom(state, eoh, format("One or more values are not integer in {}={}", cAlphaFields(2), Alphas(2)));
            }

        } // for (Loop)

        //!! Get "DaySchedule:Interval"

        CurrentModuleObject = "Schedule:Day:Interval";
        for (int Loop = 1; Loop <= NumIntDaySchedules; ++Loop) {
            s_ip->getObjectItem(state,
                                CurrentModuleObject,
                                Loop,
                                Alphas,
                                NumAlphas,
                                Numbers,
                                NumNumbers,
                                Status,
                                lNumericBlanks,
                                lAlphaBlanks,
                                cAlphaFields,
                                cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (s_sched->dayScheduleMap.find(Alphas(1)) != s_sched->dayScheduleMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
                continue;
            }

            auto *daySched = AddDaySchedule(state, Alphas(1));

            // Validate ScheduleType
            if (lAlphaBlanks(2)) {
                ShowWarningEmptyField(state, eoh, cAlphaFields(2));
                ShowContinueError(state, "Schedule will not be validated.");
            } else if ((daySched->schedTypeNum = GetScheduleTypeNum(state, Alphas(2))) == SchedNum_Invalid) {
                ShowWarningItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ShowContinueError(state, "Schedule will not be validated.");
            }

            NumFields = NumAlphas - 3;
            // check to see if numfield=0
            if (NumFields == 0) {
                ShowSevereCustom(state,
                                 eoh,
                                 format("Insufficient data entered for a full schedule day."
                                        "Number of interval fields == [{}].",
                                        NumFields));
                ErrorsFound = true;
            }

            // Depending on value of "Interpolate" field, the value for each time step in each hour gets processed:
            daySched->interpolation = static_cast<Interpolation>(getEnumValue(interpolationNamesUC, Alphas(3)));
            if (daySched->interpolation == Interpolation::Invalid) {
                ShowSevereInvalidKey(state, eoh, cAlphaFields(3), Alphas(3));
                ErrorsFound = true;
            }

            ProcessIntervalFields(state,
                                  Alphas({4, _}),
                                  Numbers,
                                  NumFields,
                                  NumNumbers,
                                  minuteVals,
                                  setMinuteVals,
                                  ErrorsFound,
                                  Alphas(1),
                                  CurrentModuleObject,
                                  daySched->interpolation);

            // Now parcel into TS Value.... tsVals.resize() was called in AddDaySchedule()
            daySched->populateFromMinuteVals(state, minuteVals);

            if (daySched->checkValsForLimitViolations(state)) {
                ShowWarningCustom(state, eoh, format("Values are outside of range for {}={}", cAlphaFields(2), Alphas(2)));
            }

            if (daySched->checkValsForBadIntegers(state)) {
                ShowWarningCustom(state, eoh, format("One or more values are not integer in {}={}", cAlphaFields(2), Alphas(2)));
            }
        }

        //!! Get "DaySchedule:List"

        CurrentModuleObject = "Schedule:Day:List";
        for (int Loop = 1; Loop <= NumLstDaySchedules; ++Loop) {
            s_ip->getObjectItem(state,
                                CurrentModuleObject,
                                Loop,
                                Alphas,
                                NumAlphas,
                                Numbers,
                                NumNumbers,
                                Status,
                                lNumericBlanks,
                                lAlphaBlanks,
                                cAlphaFields,
                                cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (s_sched->dayScheduleMap.find(Alphas(1)) != s_sched->dayScheduleMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
                continue;
            }

            auto *daySched = AddDaySchedule(state, Alphas(1));

            // Validate ScheduleType
            if (lAlphaBlanks(2)) {
                ShowWarningEmptyField(state, eoh, cAlphaFields(2));
                ShowContinueError(state, "Schedule will not be validated.");
            } else if ((daySched->schedTypeNum = GetScheduleTypeNum(state, Alphas(2))) == SchedNum_Invalid) {
                ShowWarningItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ShowContinueError(state, "Schedule will not be validated.");
            }

            // Depending on value of "Interpolate" field, the value for each time step in each hour gets processed:
            daySched->interpolation = static_cast<Interpolation>(getEnumValue(interpolationNamesUC, Alphas(3)));

            // check to see if there are any fields
            if (Numbers(1) <= 0.0) {
                ShowSevereCustom(state,
                                 eoh,
                                 format("Insufficient data entered for a full schedule day."
                                        "...Minutes per Item field = [{}].",
                                        Numbers(1)));
                ErrorsFound = true;
                continue;
            }
            if (NumNumbers < 25) {
                ShowSevereCustom(state,
                                 eoh,
                                 format("Insufficient data entered for a full schedule day."
                                        "...Minutes per Item field = [{}] and only [{}] to apply to list fields.",
                                        Numbers(1),
                                        NumNumbers - 1));
                ErrorsFound = true;
                continue;
            }

            MinutesPerItem = int(Numbers(1));
            NumExpectedItems = 1440 / MinutesPerItem;
            if ((NumNumbers - 1) != NumExpectedItems) {
                ShowSevereCustom(state,
                                 eoh,
                                 format("Number of Entered Items={} not equal number of expected items={}"
                                        "based on {}={}",
                                        NumNumbers - 1,
                                        NumExpectedItems,
                                        cNumericFields(1),
                                        MinutesPerItem));
                ErrorsFound = true;
                continue;
            }

            if (mod(Constant::iMinutesInHour, MinutesPerItem) != 0) {
                ShowSevereCustom(state, eoh, format("{}={} not evenly divisible into 60", cNumericFields(1), MinutesPerItem));
                ErrorsFound = true;
                continue;
            }

            // Number of numbers in the Numbers list okay to process
            int hr = 0;
            int begMin = 0;
            int endMin = MinutesPerItem - 1;
            for (int NumFields = 2; NumFields <= NumNumbers; ++NumFields) {
                for (int iMin = begMin; iMin <= endMin; ++iMin) {
                    minuteVals[hr * Constant::iMinutesInHour + iMin] = Numbers(NumFields);
                }
                begMin = endMin + 1;
                endMin += MinutesPerItem;
                if (endMin >= Constant::iMinutesInHour) {
                    endMin = MinutesPerItem - 1;
                    begMin = 0;
                    ++hr;
                }
            }

            // Now parcel into TS Value.... tsVals.resize() was called in AddDaySchedule()
            daySched->populateFromMinuteVals(state, minuteVals);

            if (daySched->checkValsForLimitViolations(state)) {
                ShowWarningCustom(state, eoh, format("Values are outside of range for {}={}", cAlphaFields(2), Alphas(2)));
            }

            if (daySched->checkValsForBadIntegers(state)) {
                ShowWarningCustom(state, eoh, format("One or more values are not integer for {}={}", cAlphaFields(2), Alphas(2)));
            }
        }

        //!! Get Week Schedules - regular

        CurrentModuleObject = "Schedule:Week:Daily";
        for (int Loop = 1; Loop <= NumRegWeekSchedules; ++Loop) {
            s_ip->getObjectItem(state,
                                CurrentModuleObject,
                                Loop,
                                Alphas,
                                NumAlphas,
                                Numbers,
                                NumNumbers,
                                Status,
                                lNumericBlanks,
                                lAlphaBlanks,
                                cAlphaFields,
                                cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (s_sched->weekScheduleMap.find(Alphas(1)) != s_sched->weekScheduleMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
                continue;
            }

            auto *weekSched = AddWeekSchedule(state, Alphas(1));

            // Rest of Alphas are processed into schedule nums
            for (int iDayType = 1; iDayType < (int)DayType::Num; ++iDayType) {
                if ((weekSched->dayScheds[iDayType] = GetDaySchedule(state, Alphas(iDayType + 1))) == nullptr) {
                    ShowSevereItemNotFoundAudit(state, eoh, cAlphaFields(iDayType + 1), Alphas(iDayType + 1));
                    ErrorsFound = true;
                }
            } // for (iDayType)
        }

        //!! Get Week Schedules - compact
        Count = NumRegWeekSchedules;
        CurrentModuleObject = "Schedule:Week:Compact";
        for (int Loop = 1; Loop <= NumCptWeekSchedules; ++Loop) {
            s_ip->getObjectItem(state,
                                CurrentModuleObject,
                                Loop,
                                Alphas,
                                NumAlphas,
                                Numbers,
                                NumNumbers,
                                Status,
                                lNumericBlanks,
                                lAlphaBlanks,
                                cAlphaFields,
                                cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (s_sched->weekScheduleMap.find(Alphas(1)) != s_sched->weekScheduleMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
                continue;
            }

            auto *weekSched = AddWeekSchedule(state, Alphas(1));

            std::fill(allDays.begin(), allDays.end(), false);
            // Rest of Alphas are processed into schedule indices
            for (int idx = 2; idx <= NumAlphas; idx += 2) {
                auto *daySched = GetDaySchedule(state, Alphas(idx + 1));
                if (daySched == nullptr) {
                    ShowSevereItemNotFoundAudit(state, eoh, cAlphaFields(idx + 1), Alphas(idx + 1));
                    ShowContinueError(state, format("ref: {} \"{}\"", cAlphaFields(idx), Alphas(idx)));
                    ErrorsFound = true;
                } else {
                    std::fill(theseDays.begin(), theseDays.end(), false);
                    ErrorHere = false;
                    ProcessForDayTypes(state, Alphas(idx), theseDays, allDays, ErrorHere);
                    if (ErrorHere) {
                        ShowContinueError(state, format("{}: {}=\"{}", routineName, CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    } else {
                        for (int iDayType = 1; iDayType < (int)DayType::Num; ++iDayType) {
                            if (theseDays[iDayType]) {
                                weekSched->dayScheds[iDayType] = daySched;
                            }
                        }
                    }
                }
            }

            //  Have processed all named days, check to make sure all given
            for (int iDayType = iDayType_Sun; iDayType < (int)DayType::Num; ++iDayType) {
                if (allDays[iDayType] == true) {
                    continue;
                }
                ShowSevereError(state, format("{}: {}=\"{}\", Missing some day assignments", routineName, CurrentModuleObject, Alphas(1)));
                ErrorsFound = true;
                break;
            }
        }
        NumRegWeekSchedules = Count;

        //!! Get Schedules (all types)

        //!! Get Regular Schedules

        CurrentModuleObject = "Schedule:Year";
        for (int Loop = 1; Loop <= NumRegSchedules; ++Loop) {
            s_ip->getObjectItem(state,
                                CurrentModuleObject,
                                Loop,
                                Alphas,
                                NumAlphas,
                                Numbers,
                                NumNumbers,
                                Status,
                                lNumericBlanks,
                                lAlphaBlanks,
                                cAlphaFields,
                                cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (s_sched->scheduleMap.find(Alphas(1)) != s_sched->scheduleMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
                continue;
            }

            auto *sched = AddScheduleDetailed(state, Alphas(1));

            // Validate ScheduleType
            if (lAlphaBlanks(2)) {
                ShowWarningEmptyField(state, eoh, cAlphaFields(2));
                ShowContinueError(state, "Schedule will not be validated.");
            } else if ((sched->schedTypeNum = GetScheduleTypeNum(state, Alphas(2))) == SchedNum_Invalid) {
                ShowWarningItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ShowContinueError(state, "Schedule will not be validated.");
            }

            int NumPointer = 0;

            std::array<int, 367> daysInYear;
            std::fill(daysInYear.begin(), daysInYear.end(), 0);

            // Rest of Alphas (Weekschedules) are processed into Pointers
            for (int idx = 3; idx <= NumAlphas; ++idx) {
                auto *weekSched = GetWeekSchedule(state, Alphas(idx));
                if (weekSched == nullptr) {
                    ShowSevereItemNotFoundAudit(state, eoh, cAlphaFields(idx), Alphas(idx));
                    ErrorsFound = true;
                    continue;
                }

                // Process for month, day
                int StartMonth = int(Numbers(NumPointer + 1));
                int StartDay = int(Numbers(NumPointer + 2));
                int EndMonth = int(Numbers(NumPointer + 3));
                int EndDay = int(Numbers(NumPointer + 4));
                NumPointer += 4;
                int StartPointer = General::OrdinalDay(StartMonth, StartDay, 1);
                int EndPointer = General::OrdinalDay(EndMonth, EndDay, 1);
                if (StartPointer <= EndPointer) {
                    for (int Count = StartPointer; Count <= EndPointer; ++Count) {
                        ++daysInYear[Count];
                        sched->weekScheds[Count] = weekSched;
                    }
                } else {
                    for (int Count = StartPointer; Count <= 366; ++Count) {
                        ++daysInYear[Count];
                        sched->weekScheds[Count] = weekSched;
                    }
                    for (int Count = 1; Count <= EndPointer; ++Count) {
                        ++daysInYear[Count];
                        sched->weekScheds[Count] = weekSched;
                    }
                }
            }

            // Perform Error checks on this item
            // Do special test for Feb 29.  Make equal to Feb 28.
            if (daysInYear[60] == 0) {
                daysInYear[60] = daysInYear[59];
                sched->weekScheds[60] = sched->weekScheds[59];
            }

            for (int iDay = 1; iDay <= 366; ++iDay) {
                if (daysInYear[iDay] == 0) {
                    ShowSevereCustomAudit(state, eoh, "has missing days in its schedule pointers");
                    ErrorsFound = true;
                    break;
                } else if (daysInYear[iDay] > 1) {
                    ShowSevereCustomAudit(state, eoh, "has overlapping days in its schedule pointers");
                    ErrorsFound = true;
                    break;
                }
            }

            // What does it mean to actuate a schedule?
            if (s_glob->AnyEnergyManagementSystemInModel) { // setup constant schedules as actuators
                SetupEMSActuator(state, "Schedule:Year", sched->Name, "Schedule Value", "[ ]", sched->EMSActuatedOn, sched->EMSVal);
            }
        }

        //!! Get Compact Schedules
        // SCHEDULE:COMPACT,
        //   \memo Irregular object.  Does not follow the usual definition for fields.  Fields A3... are:
        //   \memo Through: Date
        //   \memo For: Applicable days (ref: Weekschedule:Compact)
        //   \memo Interpolate: Yes/No (ref: Dayschedule:interval) -- optional, if not used will be "No"
        //   \memo Until: <Time> (ref: Dayschedule:Interval)
        //   \memo <numeric value>
        //   \memo words "Through","For","Interpolate","Until" must be included.
        //  A1 , \field Name
        //       \required-field
        //       \type alpha
        //       \reference ScheduleNames
        //  A2 , \field ScheduleType
        //       \type object-list
        //       \object-list ScheduleTypeNames
        //  A3 , \field Complex Field #1
        //  A4 , \field Complex Field #2
        //  A5 , \field Complex Field #3

        // When InitConstantScheduleData is called, TimeStepsInHour is 0, so we delay it here
        static_cast<DaySchedule *>(s_sched->daySchedules[SchedNum_AlwaysOff])->tsVals.assign(Constant::iHoursInDay * s_glob->TimeStepsInHour, 0.0);

        SchNum = NumRegSchedules;
        CurrentModuleObject = "Schedule:Compact";
        for (int Loop = 1; Loop <= NumCptSchedules; ++Loop) {
            s_ip->getObjectItem(state,
                                CurrentModuleObject,
                                Loop,
                                Alphas,
                                NumAlphas,
                                Numbers,
                                NumNumbers,
                                Status,
                                lNumericBlanks,
                                lAlphaBlanks,
                                cAlphaFields,
                                cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (s_sched->scheduleMap.find(Alphas(1)) != s_sched->scheduleMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
                continue;
            }

            auto *sched = AddScheduleDetailed(state, Alphas(1));
            sched->type = SchedType::Compact;

            // Validate ScheduleType
            if (lAlphaBlanks(2)) {
                ShowWarningEmptyField(state, eoh, cAlphaFields(2));
                ShowContinueError(state, "Schedule will not be validated.");
            } else if ((sched->schedTypeNum = GetScheduleTypeNum(state, Alphas(2))) == SchedNum_Invalid) {
                ShowWarningItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ShowContinueError(state, "Schedule will not be validated.");
            }

            NumPointer = 0;

            std::array<int, 367> daysInYear;
            std::fill(daysInYear.begin() + 1, daysInYear.end(), 0);
            // Process the "complex" fields -- so named because they are not a 1:1 correspondence
            // as other objects are
            NumField = 3;
            StartPointer = 1;
            WkCount = 0;
            DyCount = 0;
            bool FullYearSet = false;
            while (NumField < NumAlphas) {
                //   Process "Through"
                if (!has_prefix(Alphas(NumField), "THROUGH:") && !has_prefix(Alphas(NumField), "THROUGH")) {
                    ShowSevereCustom(state, eoh, format("Expecting \"Through:\" date, instead found entry={}", Alphas(NumField)));
                    ErrorsFound = true;
                    goto Through_exit;
                }

                int sPos = (Alphas(NumField)[7] == ':') ? 8 : 7;
                Alphas(NumField).erase(0, sPos);
                strip(Alphas(NumField));

                CurrentThrough = Alphas(NumField);
                ErrorHere = false;
                ProcessDateString(state, Alphas(NumField), EndMonth, EndDay, PWeekDay, PDateType, ErrorHere);
                if (PDateType == Weather::DateType::NthDayInMonth || PDateType == Weather::DateType::LastDayInMonth) {
                    ShowSevereCustom(state, eoh, format("Invalid \"Through:\" date, found entry={}", Alphas(NumField)));
                    ErrorsFound = true;
                    goto Through_exit;
                }

                if (ErrorHere) {
                    ShowSevereCustom(state, eoh, "Invalid \"Through:\" date");
                    ErrorsFound = true;
                    goto Through_exit;
                }

                EndPointer = General::OrdinalDay(EndMonth, EndDay, 1);
                if (EndPointer == 366) {
                    if (FullYearSet) {
                        ShowSevereCustom(
                            state, eoh, format("New \"Through\" entry when \"full year\" already set \"Through\" field={}", CurrentThrough));
                        ErrorsFound = true;
                    }
                    FullYearSet = true;
                }

                ++WkCount;

                auto *weekSched = AddWeekSchedule(state, format("{}_wk_{}", Alphas(1), WkCount));
                weekSched->isUsed = true;

                for (int iDay = StartPointer; iDay <= EndPointer; ++iDay) {
                    sched->weekScheds[iDay] = weekSched;
                    ++daysInYear[iDay];
                }

                StartPointer = EndPointer + 1;
                ThruField = NumField;
                std::fill(allDays.begin(), allDays.end(), false);
                ++NumField;

                while (NumField < NumAlphas) { // Continues until next "Through"
                    if (has_prefix(Alphas(NumField), "THROUGH")) {
                        goto For_exit;
                    }
                    //   "For" must be next, adds to "# Day Schedules"
                    if (!has_prefix(Alphas(NumField), "FOR")) {
                        ShowSevereCustom(state, eoh, format("Looking for \"For\" field, found={}", Alphas(NumField)));
                        ErrorsFound = true;
                        goto Through_exit;
                    }

                    ++DyCount;

                    auto *daySched = AddDaySchedule(state, format("{}_dy_{}", Alphas(1), DyCount));

                    daySched->schedTypeNum = sched->schedTypeNum;
                    daySched->isUsed = true;

                    std::fill(theseDays.begin(), theseDays.end(), false);
                    ErrorHere = false;
                    LastFor = Alphas(NumField);
                    ProcessForDayTypes(state, Alphas(NumField), theseDays, allDays, ErrorHere);
                    if (ErrorHere) {
                        ShowContinueError(state, format("ref {}=\"{}\"", CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state, format("ref Through field={}", Alphas(ThruField)));
                        ErrorsFound = true;
                    } else {
                        for (int iDayType = 1; iDayType < (int)DayType::Num; ++iDayType) {
                            if (theseDays[iDayType]) {
                                weekSched->dayScheds[iDayType] = daySched;
                            }
                        }
                    }

                    // Check for "Interpolate"
                    ++NumField;
                    if (has_prefix(Alphas(NumField), "INTERPOLATE") || !has_prefix(Alphas(NumField), "UNTIL")) {
                        if (has(Alphas(NumField), "NO")) {
                            daySched->interpolation = Interpolation::No;
                        } else if (has(Alphas(NumField), "AVERAGE")) {
                            daySched->interpolation = Interpolation::Average;
                        } else if (has(Alphas(NumField), "LINEAR")) {
                            daySched->interpolation = Interpolation::Linear;
                        } else {
                            ShowSevereInvalidKey(state, eoh, cAlphaFields(NumField), Alphas(NumField));
                            ErrorsFound = true;
                        }
                        ++NumField;
                    }

                    NumNumbers = 0;
                    xxcount = 0;
                    UntilFld = NumField;
                    while (true) {
                        if (has_prefix(Alphas(NumField), "FOR")) {
                            break;
                        }
                        if (has_prefix(Alphas(NumField), "THROUGH")) {
                            break;
                        }
                        if (has_prefix(Alphas(NumField), "UNTIL")) {
                            // Process Until/Value pairs for later processing by other routine.
                            ++NumField;
                            ++xxcount;
                            ++NumNumbers;
                            Numbers(NumNumbers) = Util::ProcessNumber(Alphas(NumField), ErrorHere);
                            if (ErrorHere) {
                                ShowSevereCustom(
                                    state, eoh, format("Until field=[{}] has illegal value field=[{}].", Alphas(NumField - 1), Alphas(NumField)));
                                ErrorsFound = true;
                            }
                            ++NumField;
                            Alphas(UntilFld + xxcount) = Alphas(NumField); // In case next is "until"
                        } else {
                            ShowSevereCustom(state, eoh, format("Looking for \"Until\" field, found={}", Alphas(NumField)));
                            ErrorsFound = true;
                            goto Through_exit;
                        }
                        if (Alphas(NumField).empty()) {
                            break;
                        }
                    }

                    // Process Untils, Numbers
                    if (NumNumbers > 0) {
                        NumFields = NumNumbers;
                        ErrorHere = false;
                        ProcessIntervalFields(state,
                                              Alphas({UntilFld, _}),
                                              Numbers,
                                              NumFields,
                                              NumNumbers,
                                              minuteVals,
                                              setMinuteVals,
                                              ErrorHere,
                                              daySched->Name,
                                              CurrentModuleObject + " DaySchedule Fields",
                                              daySched->interpolation);
                        // Depending on value of "Interpolate" field, the value for each time step in each hour gets processed:
                        if (ErrorHere) {
                            ShowContinueError(state, format("ref {}=\"{}\"", CurrentModuleObject, Alphas(1)));
                            ErrorsFound = true;
                        }

                        daySched->populateFromMinuteVals(state, minuteVals);
                    }
                }

            For_exit:;
                for (int iDayType = iDayType_Sun; iDayType < (int)DayType::Num; ++iDayType) {
                    if (allDays[iDayType] == true) {
                        continue;
                    }

                    ShowWarningCustom(state, eoh, format("has missing day types in Through={}", CurrentThrough));
                    ShowContinueError(state, format("Last \"For\" field={}", LastFor));
                    std::string errmsg = "Missing day types=,";
                    for (int kDayType = iDayType_Sun; kDayType < (int)DayType::Num; ++kDayType) {
                        if (allDays[kDayType]) {
                            continue;
                        }
                        errmsg.erase(errmsg.length() - 1);
                        errmsg = format("{} \"{}\",-", errmsg, dayTypeNames[kDayType]);
                    }
                    errmsg.erase(errmsg.length() - 2);
                    ShowContinueError(state, errmsg);
                    ShowContinueError(state, "Missing day types will have 0.0 as Schedule Values");
                    break;
                }
            }

        Through_exit:;
            if (daysInYear[60] == 0) {
                daysInYear[60] = daysInYear[59];
                sched->weekScheds[60] = sched->weekScheds[59];
            }

            if (std::find(daysInYear.begin() + 1, daysInYear.end(), 0) != daysInYear.end()) {
                ShowSevereCustomAudit(state, eoh, "has missing days in its schedule pointers");
                ErrorsFound = true;
            }
            if (std::find_if(daysInYear.begin() + 1, daysInYear.end(), [](int i) { return i > 1; }) != daysInYear.end()) {
                ShowSevereCustomAudit(state, eoh, "has overlapping days in its schedule pointers");
                ErrorsFound = true;
            }

            if (s_glob->AnyEnergyManagementSystemInModel) { // setup constant schedules as actuators
                SetupEMSActuator(state, "Schedule:Compact", sched->Name, "Schedule Value", "[ ]", sched->EMSActuatedOn, sched->EMSVal);
            }
        }

        //  Schedule:File,
        //   \min-fields 5
        //         \memo A Schedule:File points to a text computer file that has 8760-8784 hours of data.
        //    A1 , \field Name
        //         \required-field
        //         \type alpha
        //         \reference ScheduleNames
        //    A2 , \field Schedule Type Limits Name
        //         \type object-list
        //         \object-list ScheduleTypeLimitsNames
        //    A3 , \field File Name
        //         \required-field
        //         \retaincase
        //    N1 , \field Column Number
        //         \required-field
        //         \type integer
        //         \minimum 1
        //    N2 , \field Rows to Skip at Top
        //         \required-field
        //         \type integer
        //         \minimum 0
        //    N3 , \field Number of Hours of Data
        //         \note 8760 hours does not account for leap years, 8784 does.
        //         \note should be either 8760 or 8784
        //         \default 8760
        //         \minimum 8760
        //         \maximum 8784
        //    A4 , \field Column Separator
        //         \type choice
        //         \key Comma
        //         \key Tab
        //         \key Space
        //         \key Semicolon
        //         \default Comma
        //    A5 , \field Interpolate to Timestep
        //         \note when the interval does not match the user specified timestep a "Yes" choice will average between the intervals request (to
        //         \note timestep resolution.  a "No" choice will use the interval value at the simulation timestep without regard to if it matches
        //         \note the boundary or not.
        //         \type choice
        //         \key Yes
        //         \key No
        //         \default No
        //    N4 ; \field Minutes per Item
        //         \note Must be evenly divisible into 60
        //         \type integer
        //         \minimum 1
        //         \maximum 60

        // continue adding to SchNum,AddWeekSch,AddDaySch

        CurrentModuleObject = "Schedule:File";
        for (int Loop = 1; Loop <= NumCommaFileSchedules; ++Loop) {
            s_ip->getObjectItem(state,
                                CurrentModuleObject,
                                Loop,
                                Alphas,
                                NumAlphas,
                                Numbers,
                                NumNumbers,
                                Status,
                                lNumericBlanks,
                                lAlphaBlanks,
                                cAlphaFields,
                                cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (s_sched->scheduleMap.find(Alphas(1)) != s_sched->scheduleMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
                continue;
            }

            auto *sched = AddScheduleDetailed(state, Alphas(1));
            sched->type = SchedType::File;

            // Validate ScheduleType
            if (lAlphaBlanks(2)) {
                ShowWarningEmptyField(state, eoh, cAlphaFields(2));
                ShowContinueError(state, "Schedule will not be validated.");
            } else if ((sched->schedTypeNum = GetScheduleTypeNum(state, Alphas(2))) == SchedNum_Invalid) {
                ShowWarningItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ShowContinueError(state, "Schedule will not be validated.");
            }

            // Numbers(1) - which column
            curcolCount = Numbers(1);
            // Numbers(2) - number of rows to skip
            skiprowCount = Numbers(2);
            if (Numbers(3) == 0) {
                Numbers(3) = 8760.0;
            }
            if (Numbers(3) != 8760 && Numbers(3) != 8784) {
                ShowSevereCustom(
                    state,
                    eoh,
                    format("{} must = 8760 or 8784 (for a leap year).  Value = {:.0T}, Schedule not processed.", cNumericFields(3), Numbers(3)));
                ErrorsFound = true;
                continue;
            }

            if (lAlphaBlanks(4) || Util::SameString(Alphas(4), "comma")) {
                ColumnSep = CharComma;
                Alphas(4) = "comma";
            } else if (Util::SameString(Alphas(4), "semicolon")) {
                ColumnSep = CharSemicolon;
            } else if (Util::SameString(Alphas(4), "tab")) {
                ColumnSep = CharTab;
            } else if (Util::SameString(Alphas(4), "space")) {
                ColumnSep = CharSpace;
            } else {
                ShowSevereInvalidKey(state, eoh, cAlphaFields(4), Alphas(4), "..must be Comma, Semicolon, Tab, or Space.");
                ErrorsFound = true;
                continue;
            }

            // Depending on value of "Interpolate" field, the value for each time step in each hour gets processed:
            Interpolation interp = Interpolation::No;

            if (!lAlphaBlanks(5)) {
                if (BooleanSwitch bs = getYesNoValue(Alphas(5)); bs != BooleanSwitch::Invalid) {
                    interp = static_cast<bool>(bs) ? Interpolation::Average : Interpolation::Linear;
                } else {
                    ShowSevereInvalidKey(state, eoh, cAlphaFields(5), Alphas(5));
                    ErrorsFound = true;
                }
            }

            sched->UseDaylightSaving = true;
            if ((Alphas(6)) == "NO") {
                sched->UseDaylightSaving = false;
            }

            // is it a sub-hourly schedule or not?
            int MinutesPerItem = Constant::iMinutesInHour;
            if (NumNumbers > 3) {
                MinutesPerItem = int(Numbers(4));
                // int NumExpectedItems = 1440 / MinutesPerItem;
                if (mod(Constant::iMinutesInHour, MinutesPerItem) != 0) {
                    ShowSevereCustom(
                        state, eoh, format("Requested {} field value ({}) not evenly divisible into 60", cNumericFields(4), MinutesPerItem));
                    ErrorsFound = true;
                    continue;
                }
            }

            int numHourlyValues = Numbers(3);
            int rowLimitCount = (Numbers(3) * Constant::rMinutesInHour) / MinutesPerItem;
            int hrLimitCount = Constant::iMinutesInHour / MinutesPerItem;

            std::string contextString = format("{}=\"{}\", {}: ", CurrentModuleObject, Alphas(1), cAlphaFields(3));

            state.files.TempFullFilePath.filePath = CheckForActualFilePath(state, Alphas(3), contextString);
            // Setup file reading parameters
            if (state.files.TempFullFilePath.filePath.empty()) {
                ErrorsFound = true;
            } else {
                auto result = s_sched->UniqueProcessedExternalFiles.find(state.files.TempFullFilePath.filePath);
                if (result == s_sched->UniqueProcessedExternalFiles.end()) {
                    FileSystem::FileTypes const ext = FileSystem::getFileType(state.files.TempFullFilePath.filePath);
                    if (FileSystem::is_flat_file_type(ext)) {
                        auto const schedule_data = FileSystem::readFile(state.files.TempFullFilePath.filePath);
                        CsvParser csvParser;
                        auto it = s_sched->UniqueProcessedExternalFiles.emplace(state.files.TempFullFilePath.filePath,
                                                                                csvParser.decode(schedule_data, ColumnSep, skiprowCount));
                        if (csvParser.hasErrors()) {
                            for (const auto &[error, isContinued] : csvParser.errors()) {
                                if (isContinued) {
                                    ShowContinueError(state, error);
                                } else {
                                    ShowSevereError(state, error);
                                }
                            }
                            ShowContinueError(state, fmt::format("Error Occurred in {}", state.files.TempFullFilePath.filePath));
                            ShowFatalError(state, "Program terminates due to previous condition.");
                        }
                        result = it.first;
                    } else if (FileSystem::is_all_json_type(ext)) {
                        auto it = s_sched->UniqueProcessedExternalFiles.emplace(state.files.TempFullFilePath.filePath,
                                                                                FileSystem::readJSON(state.files.TempFullFilePath.filePath));
                        result = it.first;
                    } else {
                        ShowSevereCustom(
                            state,
                            eoh,
                            format("{} = {} has an unknown file extension and cannot be read by this program.", cAlphaFields(3), Alphas(3)));
                        ShowFatalError(state, "Program terminates due to previous condition.");
                    }
                }

                auto const &column_json = result->second["values"][curcolCount - 1];
                rowCnt = column_json.size();
                auto const column_values = column_json.get<std::vector<Real64>>(); // (AUTO_OK_OBJ)

                // schedule values have been filled into the hourlyFileValues array.

                if (numerrors > 0) {
                    ShowWarningCustom(state,
                                      eoh,
                                      format("{} records had errors - these values are set to 0."
                                             "Use Output:Diagnostics,DisplayExtraWarnings; to see individual records in error.",
                                             numerrors));
                }

                if (rowCnt < rowLimitCount) {
                    ShowWarningCustom(state,
                                      eoh,
                                      format("less than {} hourly values read from file."
                                             "..Number read={}.",
                                             numHourlyValues,
                                             (rowCnt * Constant::iMinutesInHour) / MinutesPerItem));
                }

                // process the data into the normal schedule data structures
                // note -- schedules are ALWAYS 366 days so some special measures have to be done at 29 Feb "day of year" (60)
                int iDay = 0;
                int hDay = 0;
                int ifld = 0;
                while (true) {
                    // create string of which day of year
                    ++iDay;
                    ++hDay;
                    if (iDay > 366) {
                        break;
                    }
                    // increment both since a week schedule is being defined for each day so that a day is valid
                    // no matter what the day type that is used in a design day.

                    // define day schedule
                    auto *daySched = AddDaySchedule(state, format("{}_dy_{}", Alphas(1), iDay));
                    daySched->schedTypeNum = sched->schedTypeNum;

                    // define week schedule
                    auto *weekSched = AddWeekSchedule(state, format("{}_wk_{}", Alphas(1), iDay));

                    // for all day types point the week schedule to the newly defined day schedule
                    for (int kDayType = 1; kDayType < (int)DayType::Num; ++kDayType) {
                        weekSched->dayScheds[kDayType] = daySched;
                    }

                    // schedule is pointing to the week schedule
                    sched->weekScheds[iDay] = weekSched;

                    if (MinutesPerItem == Constant::iMinutesInHour) {
                        for (int hr = 0; hr < Constant::iHoursInDay; ++hr) {
                            Real64 curHrVal = column_values[ifld]; // hourlyFileValues((hDay - 1) * 24 + jHour)
                            ++ifld;
                            for (int ts = 0; ts < s_glob->TimeStepsInHour; ++ts) {
                                daySched->tsVals[hr * s_glob->TimeStepsInHour + ts] = curHrVal;
                                daySched->sumTsVals += daySched->tsVals[hr * s_glob->TimeStepsInHour + ts];
                            }
                        }
                    } else { // Minutes Per Item < 60
                        for (int hr = 0; hr < Constant::iHoursInDay; ++hr) {
                            int endMin = MinutesPerItem - 1;
                            int begMin = 0;
                            for (int NumFields = 1; NumFields <= hrLimitCount; ++NumFields) {
                                for (int iMin = begMin; iMin <= endMin; ++iMin) {
                                    minuteVals[hr * Constant::iMinutesInHour + iMin] = column_values[ifld];
                                }

                                ++ifld;
                                begMin = endMin + 1;
                                endMin += MinutesPerItem;
                            }
                        }

                        daySched->interpolation = interp;
                        daySched->populateFromMinuteVals(state, minuteVals);
                    }
                    if (iDay == 59 && rowCnt < 8784 * hrLimitCount) { // 28 Feb
                        // Dup 28 Feb to 29 Feb (60)
                        ++iDay;
                        sched->weekScheds[iDay] = sched->weekScheds[iDay - 1];
                    }
                }
            }

            if (s_glob->AnyEnergyManagementSystemInModel) { // setup constant schedules as actuators
                SetupEMSActuator(state, "Schedule:File", sched->Name, "Schedule Value", "[ ]", sched->EMSActuatedOn, sched->EMSVal);
            }
        }

        if (NumCommaFileShading != 0) {
            auto const &values_json = schedule_file_shading_result->second["values"];
            auto const headers = schedule_file_shading_result->second["header"].get<std::vector<std::string>>();  // (AUTO_OK_OBJ)
            auto const headers_set = schedule_file_shading_result->second["header"].get<std::set<std::string>>(); // (AUTO_OK_OBJ)

            for (auto const &header : headers_set) {
                size_t column = 0;
                auto column_it = std::find(headers.begin(), headers.end(), header);
                if (column_it != headers.end()) {
                    column = std::distance(headers.begin(), column_it);
                }
                if (column == 0) {
                    continue; // Skip timestamp column and any duplicate column, which will be 0 as well since it won't be found.
                }
                auto const column_values = values_json.at(column).get<std::vector<Real64>>(); // (AUTO_OK_OBJ)

                std::string curName = format("{}_shading", header);
                std::string curNameUC = Util::makeUPPER(curName);

                if (s_sched->scheduleMap.find(curNameUC) != s_sched->scheduleMap.end()) {
                    ShowSevereError(state, format("Duplicate schedule name {}", curName));
                    ErrorsFound = true;
                    continue;
                }

                auto *schedShading = AddScheduleDetailed(state, curName);
                schedShading->type = SchedType::File;

                int iDay = 0;
                int ifld = 0;
                while (true) {
                    // create string of which day of year
                    ++iDay;
                    if (iDay > 366) {
                        break;
                    }

                    // day schedule
                    auto *daySched = AddDaySchedule(state, format("{}_dy_{}", curName, iDay));
                    daySched->schedTypeNum = schedShading->schedTypeNum;

                    // define week schedule
                    auto *weekSched = AddWeekSchedule(state, format("{}_wk_{}", curName, iDay));

                    // for all day types point the week schedule to the newly defined day schedule
                    for (int kDayType = 1; kDayType < (int)DayType::Num; ++kDayType) {
                        weekSched->dayScheds[kDayType] = daySched;
                    }

                    // schedule is pointing to the week schedule
                    schedShading->weekScheds[iDay] = weekSched;

                    for (int hr = 0; hr < Constant::iHoursInDay; ++hr) {
                        for (int ts = 0; ts < s_glob->TimeStepsInHour; ++ts) {
                            daySched->tsVals[hr * s_glob->TimeStepsInHour + ts] = column_values[ifld];
                            ++ifld;
                        }
                    }

                    if (iDay == 59 && !state.dataEnvrn->CurrentYearIsLeapYear) { // 28 Feb
                        // Dup 28 Feb to 29 Feb (60)
                        ++iDay;
                        schedShading->weekScheds[iDay] = schedShading->weekScheds[iDay - 1];
                    }
                }
            }
        }

        // Constant Schedules
        CurrentModuleObject = "Schedule:Constant";
        for (int Loop = 1; Loop <= NumConstantSchedules; ++Loop) {
            s_ip->getObjectItem(state,
                                CurrentModuleObject,
                                Loop,
                                Alphas,
                                NumAlphas,
                                Numbers,
                                NumNumbers,
                                Status,
                                lNumericBlanks,
                                lAlphaBlanks,
                                cAlphaFields,
                                cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (s_sched->scheduleMap.find(Alphas(1)) != s_sched->scheduleMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
                continue;
            }

            auto *sched = AddScheduleConstant(state, Alphas(1), Numbers(1));

            // Validate ScheduleType
            if (lAlphaBlanks(2)) { // No warning here for constant schedules
                ShowWarningEmptyField(state, eoh, cAlphaFields(2));
                ShowContinueError(state, "Schedule will not be validated.");
            } else if ((sched->schedTypeNum = GetScheduleTypeNum(state, Alphas(2))) == SchedNum_Invalid) {
                ShowWarningItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ShowContinueError(state, "Schedule will not be validated.");
            }

            if (s_glob->AnyEnergyManagementSystemInModel) { // setup constant schedules as actuators
                SetupEMSActuator(state, "Schedule:Constant", sched->Name, "Schedule Value", "[ ]", sched->EMSActuatedOn, sched->EMSVal);
            }
        }
        // When InitConstantScheduleData is called, TimeStepsInHour is 0, so we delay it here
        static_cast<ScheduleConstant *>(s_sched->schedules[SchedNum_AlwaysOff])->tsVals.assign(Constant::iHoursInDay * s_glob->TimeStepsInHour, 0.0);
        static_cast<ScheduleConstant *>(s_sched->schedules[SchedNum_AlwaysOn])->tsVals.assign(Constant::iHoursInDay * s_glob->TimeStepsInHour, 1.0);

        CurrentModuleObject = "ExternalInterface:Schedule";
        for (int Loop = 1; Loop <= NumExternalInterfaceSchedules; ++Loop) {
            s_ip->getObjectItem(state,
                                CurrentModuleObject,
                                Loop,
                                Alphas,
                                NumAlphas,
                                Numbers,
                                NumNumbers,
                                Status,
                                lNumericBlanks,
                                lAlphaBlanks,
                                cAlphaFields,
                                cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (s_sched->scheduleMap.find(Alphas(1)) != s_sched->scheduleMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
                continue;
            }

            auto *sched = AddScheduleDetailed(state, Alphas(1));
            sched->type = SchedType::External;

            // Validate ScheduleType
            if (lAlphaBlanks(2)) {
                ShowWarningEmptyField(state, eoh, cAlphaFields(2));
                ShowContinueError(state, "Schedule will not be validated.");
            } else if ((sched->schedTypeNum = GetScheduleTypeNum(state, Alphas(2))) == SchedNum_Invalid) {
                ShowWarningItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ShowContinueError(state, "Schedule will not be validated.");
            }

            // TODO: I'm not sure this Jazz is necessary
            // Add day schedule
            auto *daySched = AddDaySchedule(state, format("{}_xi_dy_", Alphas(1)));
            daySched->isUsed = true;
            daySched->schedTypeNum = sched->schedTypeNum;

            //   Initialize the ExternalInterface day schedule for the ExternalInterface compact schedule.
            //   It will be overwritten during run time stepping after the warm up period
            if (NumNumbers < 1) {
                ShowWarningCustom(state, eoh, "Initial value is not numeric or is missing. Fix idf file.");
                NumErrorFlag = true;
            }
            ExternalInterfaceSetSchedule(state, daySched->Num, Numbers(1));

            auto *weekSched = AddWeekSchedule(state, format("{}_xi_wk_", Alphas(1)));
            weekSched->isUsed = true;
            for (int iDayType = 1; iDayType < (int)DayType::Num; ++iDayType) {
                weekSched->dayScheds[iDayType] = daySched;
            }

            for (int iDay = 1; iDay <= 366; ++iDay) {
                sched->weekScheds[iDay] = weekSched;
            }
        } // for (Loop)

        // added for FMU Import
        CurrentModuleObject = "ExternalInterface:FunctionalMockupUnitImport:To:Schedule";
        for (int Loop = 1; Loop <= NumExternalInterfaceFunctionalMockupUnitImportSchedules; ++Loop) {
            s_ip->getObjectItem(state,
                                CurrentModuleObject,
                                Loop,
                                Alphas,
                                NumAlphas,
                                Numbers,
                                NumNumbers,
                                Status,
                                lNumericBlanks,
                                lAlphaBlanks,
                                cAlphaFields,
                                cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (s_sched->scheduleMap.find(Alphas(1)) != s_sched->scheduleMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                if (NumExternalInterfaceSchedules >= 1) {
                    ShowContinueError(
                        state,
                        format("{} defined as an ExternalInterface:Schedule and ExternalInterface:FunctionalMockupUnitImport:To:Schedule."
                               "This will cause the schedule to be overwritten by PtolemyServer and FunctionalMockUpUnitImport)",
                               cAlphaFields(1)));
                }
                ErrorsFound = true;
                continue;
            }

            auto *sched = AddScheduleDetailed(state, Alphas(1));
            sched->type = SchedType::External;

            // Validate ScheduleType
            if (lAlphaBlanks(2)) {
                ShowWarningEmptyField(state, eoh, cAlphaFields(2));
                ShowContinueError(state, "Schedule will not be validated.");
            } else if ((sched->schedTypeNum = GetScheduleTypeNum(state, Alphas(2))) == SchedNum_Invalid) {
                ShowWarningItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ShowContinueError(state, "Schedule will not be validated.");
            }

            // TODO: I'm not sure this Jazz is necessary
            // Add day schedule
            auto *daySched = AddDaySchedule(state, format("{}_xi_dy_", Alphas(1)));
            daySched->isUsed = true;
            daySched->schedTypeNum = sched->schedTypeNum;

            //   Initialize the ExternalInterface day schedule for the ExternalInterface compact schedule.
            //   It will be overwritten during run time stepping after the warm up period
            if (NumNumbers < 1) {
                ShowWarningCustom(state, eoh, "Initial value is not numeric or is missing. Fix idf file.");
                NumErrorFlag = true;
            }
            ExternalInterfaceSetSchedule(state, daySched->Num, Numbers(1));

            auto *weekSched = AddWeekSchedule(state, format("{}_xi_wk_", Alphas(1)));
            weekSched->isUsed = true;
            for (int iDayType = 1; iDayType < (int)DayType::Num; ++iDayType) {
                weekSched->dayScheds[iDayType] = daySched;
            }

            for (int iDay = 1; iDay <= 366; ++iDay) {
                sched->weekScheds[iDay] = weekSched;
            }
        }

        // added for FMU Export
        CurrentModuleObject = "ExternalInterface:FunctionalMockupUnitExport:To:Schedule";
        for (int Loop = 1; Loop <= NumExternalInterfaceFunctionalMockupUnitExportSchedules; ++Loop) {
            s_ip->getObjectItem(state,
                                CurrentModuleObject,
                                Loop,
                                Alphas,
                                NumAlphas,
                                Numbers,
                                NumNumbers,
                                Status,
                                lNumericBlanks,
                                lAlphaBlanks,
                                cAlphaFields,
                                cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (s_sched->scheduleMap.find(Alphas(1)) != s_sched->scheduleMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                if (NumExternalInterfaceSchedules >= 1) {
                    ShowContinueError(
                        state,
                        format("{} defined as an ExternalInterface:Schedule and ExternalInterface:FunctionalMockupUnitImport:To:Schedule."
                               "This will cause the schedule to be overwritten by PtolemyServer and FunctionalMockUpUnitImport)",
                               cAlphaFields(1)));
                }
                ErrorsFound = true;
                continue;
            }

            auto *sched = AddScheduleDetailed(state, Alphas(1));
            sched->type = SchedType::External;

            // Validate ScheduleType
            if (lAlphaBlanks(2)) {
                ShowWarningEmptyField(state, eoh, cAlphaFields(2));
                ShowContinueError(state, "Schedule will not be validated.");
            } else if ((sched->schedTypeNum = GetScheduleTypeNum(state, Alphas(2))) == SchedNum_Invalid) {
                ShowWarningItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ShowContinueError(state, "Schedule will not be validated.");
            }

            // TODO: I'm not sure this Jazz is necessary
            // Add day schedule
            auto *daySched = AddDaySchedule(state, format("{}_xi_dy_", Alphas(1)));
            daySched->isUsed = true;
            daySched->schedTypeNum = sched->schedTypeNum;

            //   Initialize the ExternalInterface day schedule for the ExternalInterface compact schedule.
            //   It will be overwritten during run time stepping after the warm up period
            if (NumNumbers < 1) {
                ShowWarningCustom(state, eoh, "Initial value is not numeric or is missing. Fix idf file.");
                NumErrorFlag = true;
            }
            ExternalInterfaceSetSchedule(state, daySched->Num, Numbers(1));

            auto *weekSched = AddWeekSchedule(state, format("{}_xi_wk_", Alphas(1)));
            weekSched->isUsed = true;
            for (int iDayType = 1; iDayType < (int)DayType::Num; ++iDayType) {
                weekSched->dayScheds[iDayType] = daySched;
            }

            std::fill(sched->weekScheds.begin() + 1, sched->weekScheds.end(), weekSched);
        } // for (Loop)

        // Validate by ScheduleLimitsType
        for (auto *sched : s_sched->schedules) {

            if (sched->schedTypeNum == SchedNum_Invalid) {
                continue;
            }

            auto const *schedType = s_sched->scheduleTypes[sched->schedTypeNum];
            if (!schedType->isLimited) {
                continue;
            }

            if (!sched->checkMinMaxVals(state, Clusive::In, schedType->minVal, Clusive::In, schedType->maxVal)) {
                ErrorObjectHeader eoh{routineName, "Schedule", sched->Name};
                ShowSevereBadMinMax(state, eoh, "", "", Clusive::In, schedType->minVal, Clusive::In, schedType->maxVal);
                ErrorsFound = true;
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, format("{}: Preceding Errors cause termination.", routineName));
        }

        if (s_sched->scheduleTypes.size() + s_sched->daySchedules.size() + s_sched->weekSchedules.size() + s_sched->schedules.size() > 0) {
            CurrentModuleObject = "Output:Schedules";
            NumFields = s_ip->getNumObjectsFound(state, CurrentModuleObject);

            // Report the ScheduleTypeLimits only once, not for each report level.
            if (NumFields > 0) {
                ReportScheduleTypeLimits(state);
            }

            std::set<ReportLevel> reportLevelSet;
            //    RptSchedule=.FALSE.
            for (int Count = 1; Count <= NumFields; ++Count) {
                s_ip->getObjectItem(state, CurrentModuleObject, Count, Alphas, NumAlphas, Numbers, NumNumbers, Status);

                ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

                // IDD only allows Hourly or Timestep as valid values on the required field, anything else should be an error in the input processor
                ReportLevel reportLevel = static_cast<ReportLevel>(getEnumValue(reportLevelNamesUC, Alphas(1)));
                if (reportLevel == ReportLevel::Invalid) {
                    ShowWarningInvalidKey(state, eoh, cAlphaFields(1), Alphas(1), "HOURLY report will be done");
                    reportLevel = ReportLevel::Hourly;
                }
                if (auto [it, inserted] = reportLevelSet.insert(reportLevel); inserted) {
                    // If this is the first time we see this report level, we need to report the details
                    ReportScheduleDetails(state, reportLevel);
                } else {
                    ShowWarningCustom(state,
                                      eoh,
                                      format("Report level {} has already been processed. This report level will not be processed again.",
                                             reportLevelNames[(int)reportLevel]));
                    continue;
                }
            }
        }

        Alphas.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        Numbers.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();

        print(state.files.audit, "{}\n", "  Processing Schedule Input -- Complete");
    } // ProcessScheduleInput()

    void ReportScheduleTypeLimits(EnergyPlusData &state)
    {
        std::string YesNoLimited;
        std::string minValStr;
        std::string maxValStr;
        std::string YesNoContinous;

        std::string_view constexpr scheduleTypeLimitTableName("ScheduleTypeLimits");
        print(state.files.eio, "! <{}>,Name,Limited? {{Yes/No}},Minimum,Maximum,Continuous? {{Yes/No - Discrete}}\n", scheduleTypeLimitTableName);

        for (auto const *schedType : state.dataSched->scheduleTypes) {
            if (schedType->isLimited) {
                YesNoLimited = "Yes";
                minValStr = format("{:.2R}", schedType->minVal);
                strip(minValStr);
                maxValStr = format("{:.2R}", schedType->maxVal);
                strip(maxValStr);
                if (schedType->isReal) {
                    YesNoContinous = "Yes";
                } else {
                    YesNoContinous = "No";
                    minValStr = fmt::to_string((int)schedType->minVal);
                    maxValStr = fmt::to_string((int)schedType->maxVal);
                }
            } else {
                YesNoLimited = "No";
                minValStr = "N/A";
                maxValStr = "N/A";
                YesNoContinous = "N/A";
            }
            print(state.files.eio,
                  "{},{},{},{},{},{}\n",
                  scheduleTypeLimitTableName,
                  schedType->Name,
                  YesNoLimited,
                  minValStr,
                  maxValStr,
                  YesNoContinous);
        }
    }

    void ReportScheduleDetails(EnergyPlusData &state,
                               ReportLevel const LevelOfDetail) // =1: hourly; =2: timestep; = 3: make IDF excerpt
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   January 2003
        //       MODIFIED       February 2008 - add IDF outputs (compact schedules)

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine puts the details of the Schedules on the .eio file (Inits file).

        // SUBROUTINE PARAMETER DEFINITIONS:

        assert(LevelOfDetail != ReportLevel::Invalid);

        constexpr std::array<std::string_view, 12> Months = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        constexpr std::array<std::string_view, 25> HrField = {"00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12",
                                                              "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24"};

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        auto const &s_glob = state.dataGlobal;
        auto const &s_sched = state.dataSched;

        std::vector<std::string> times;
        int const NumTimesInDay = (LevelOfDetail == ReportLevel::TimeStep) ? s_glob->TimeStepsInHour * Constant::iHoursInDay : Constant::iHoursInDay;
        if (LevelOfDetail == ReportLevel::TimeStep) {
            times.reserve(NumTimesInDay);
        } else if (LevelOfDetail == ReportLevel::Hourly) {
            times.reserve(NumTimesInDay);
        } else {
            assert(false); // Invalid report level
        }

        for (int hr = 0; hr < Constant::iHoursInDay; ++hr) {
            if (LevelOfDetail == ReportLevel::TimeStep) {
                for (int ts = 0; ts < s_glob->TimeStepsInHour - 1; ++ts) {
                    times.emplace_back(fmt::format("{}:{:02}", HrField[hr], ((ts + 1)) * s_glob->MinutesInTimeStep));
                }
            }
            times.emplace_back(fmt::format("{}:00", HrField[hr + 1]));
        }
        assert(static_cast<int>(times.size()) == NumTimesInDay);

        std::string_view const &reportLevelName = reportLevelNames[(int)LevelOfDetail];
        std::string const dayScheduleTableName = format("DaySchedule - {}", reportLevelName);
        std::string const weekScheduleTableName = format("WeekSchedule - {}", reportLevelName); // TODO: "Schedule:Week:Daily" ?
        std::string const scheduleTableName = format("Schedule - {}", reportLevelName);         // TODO: "Detailed Schedule" maybe?

        // Schedule Types Header
        {
            // This is just a top level separator, for convenience/neatness, will not match any actual data
            print(state.files.eio, "! Schedule Details Report={} =====================\n", reportLevelName);

            // ! <DaySchedule_Hourly>,Name,ScheduleType,Interpolated {Yes/No},Time (HH:MM) =>,00:15,00:30,..23:45,24.00
            print(state.files.eio, "! <{}>,Name,ScheduleType,Interpolated {{Average/Linear/No}},Time (HH:MM) =>", dayScheduleTableName);
            for (const auto &time : times) {
                print(state.files.eio, ",{}", time);
            }
            print(state.files.eio, "\n");

            // ! <WeekSchedule_Hourly>,Name,Sunday,Monday,...,Saturday,Holiday,SummerDesignDay,WinterDesignDay,CustomDay1,CustomDay2
            print(state.files.eio, "! <{}>,Name", weekScheduleTableName);
            for (int Count = 1; Count < (int)DayType::Num; ++Count) {
                print(state.files.eio, ",{}", dayTypeNames[Count]);
            }
            print(state.files.eio, "\n");

            print(state.files.eio,
                  "! <{}>,Name,ScheduleType,"
                  "Until Date 1,WeekSchedule 1,Until Date 2,WeekSchedule 2,Until Date 3,WeekSchedule 3,"
                  "Until Date 4,WeekSchedule 4,Until Date 5,WeekSchedule 5,Until Date 6,WeekSchedule 6,"
                  "Until Date 7,WeekSchedule 7,Until Date 8,WeekSchedule 8,Until Date 9,WeekSchedule 9,"
                  "\n",
                  scheduleTableName);
        }

        for (auto *daySched : s_sched->daySchedules) {

            print(state.files.eio,
                  "{},{},{},{},{}",
                  dayScheduleTableName,
                  daySched->Name,
                  (daySched->schedTypeNum == SchedNum_Invalid) ? "" : s_sched->scheduleTypes[daySched->schedTypeNum]->Name,
                  interpolationNames[(int)daySched->interpolation],
                  "Values:");

            if (LevelOfDetail == ReportLevel::Hourly) {
                for (int hr = 0; hr < Constant::iHoursInDay; ++hr) {
                    print(state.files.eio, ",{:.2R}", daySched->tsVals[(hr + 1) * s_glob->TimeStepsInHour - 1]);
                }
            } else if (LevelOfDetail == ReportLevel::TimeStep) {
                for (int hr = 0; hr < Constant::iHoursInDay; ++hr) {
                    for (int ts = 0; ts < s_glob->TimeStepsInHour; ++ts) {
                        print(state.files.eio, ",{:.2R}", daySched->tsVals[hr * s_glob->TimeStepsInHour + ts]);
                    }
                }
            } else {
                assert(false);
            }
            print(state.files.eio, "\n");
        }

        for (auto *weekSched : s_sched->weekSchedules) {
            print(state.files.eio, "{},{}", weekScheduleTableName, weekSched->Name);
            for (int DayNum = 1; DayNum < (int)DayType::Num; ++DayNum) {
                print(state.files.eio, ",{}", weekSched->dayScheds[DayNum]->Name);
            }
            print(state.files.eio, "\n");
        }

        for (auto *sched : s_sched->schedules) {

            if (sched->type == SchedType::Constant) {
                continue;
            }

            auto *schedDetailed = dynamic_cast<ScheduleDetailed *>(sched);
            assert(schedDetailed != nullptr);

            print(state.files.eio,
                  "{},{},{}",
                  scheduleTableName,
                  schedDetailed->Name,
                  (sched->schedTypeNum == SchedNum_Invalid) ? "" : s_sched->scheduleTypes[sched->schedTypeNum]->Name);

            int PMon = 0;
            int PDay = 0;
            std::string_view constexpr ThruFmt(",Through {} {:02},{}");
            int DayNum = 1;
            while (DayNum <= 366) {
                auto *weekSched = schedDetailed->weekScheds[DayNum];
                while (schedDetailed->weekScheds[DayNum] == weekSched && DayNum <= 366) {
                    if (DayNum == 366) {
                        General::InvOrdinalDay(DayNum, PMon, PDay, 1);
                        print(state.files.eio, ThruFmt, Months[PMon - 1], PDay, weekSched->Name);
                    }
                    ++DayNum;
                    if (DayNum > 366) {
                        break; // compound If might have a problem unless this included.
                    }
                }
                if (DayNum <= 366) {
                    General::InvOrdinalDay(DayNum - 1, PMon, PDay, 1);
                    print(state.files.eio, ThruFmt, Months[PMon - 1], PDay, weekSched->Name);
                }
            }
            print(state.files.eio, "\n");
        }

    } // ReportScheduleDetails()

    Real64 GetCurrentScheduleValue(EnergyPlusData const &state, int const schedNum)
    {
        // Wrapper for method
        return state.dataSched->schedules[schedNum]->getCurrentVal();
    }

    void UpdateScheduleVals(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   August 2011; adapted from Autodesk (time reduction)

        // PURPOSE OF THIS SUBROUTINE:
        // This routine calculates all the scheduled values as a time reduction measure and
        // stores them in the CurrentValue item of the schedule data structure.

        // METHODOLOGY EMPLOYED:
        // Use internal Schedule data structure to calculate current value.  Note that missing values in

        auto const &s_sched = state.dataSched;
        auto const &s_glob = state.dataGlobal;

        for (auto *sched : s_sched->schedules) {
            if (sched->EMSActuatedOn) {
                sched->currentVal = sched->EMSVal;
            } else {
                sched->currentVal = sched->getHrTsVal(state, s_glob->HourOfDay, s_glob->TimeStep);
            }
        }
    }

    Real64 ScheduleDetailed::getHrTsVal(EnergyPlusData &state,
                                        int hr,
                                        int ts // Negative => unspecified
    ) const
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   January 2003
        // PURPOSE OF THIS FUNCTION:
        // This function provides a method to look up schedule values for any hour, timestep, day
        // of the year (rather than just the "current time").
        auto const &s_glob = state.dataGlobal;

        if (this->EMSActuatedOn) {
            return this->EMSVal;
        }

        //  so, current date, but maybe TimeStep added

        // Hourly Value
        if (hr > Constant::iHoursInDay) {
            ShowFatalError(state, format("LookUpScheduleValue called with thisHour={}", hr));
        }

        int thisHr = hr + state.dataEnvrn->DSTIndicator * this->UseDaylightSaving;

        int thisDayOfYear = state.dataEnvrn->DayOfYear_Schedule;
        int thisDayOfWeek = state.dataEnvrn->DayOfWeek;
        int thisHolidayNum = state.dataEnvrn->HolidayIndex;
        if (thisHr > Constant::iHoursInDay) { // In case HourOfDay is 24 and DSTIndicator is 1, you're actually the next day
            thisDayOfYear += 1;
            thisHr -= Constant::iHoursInDay;
            thisDayOfWeek = state.dataEnvrn->DayOfWeekTomorrow;
            thisHolidayNum = state.dataEnvrn->HolidayIndexTomorrow;
        }

        // In the case where DST is applied on 12/31 at 24:00, which is the case for a Southern Hemisphere location for eg
        // (DayOfYear_Schedule is a bit weird, ScheduleManager always assumes LeapYear)
        if (thisDayOfYear == 367) {
            thisDayOfYear = 1;
        }

        auto const *weekSched = this->weekScheds[thisDayOfYear];
        auto const *daySched = (thisHolidayNum > 0) ? weekSched->dayScheds[thisHolidayNum] : weekSched->dayScheds[thisDayOfWeek];

        // If Unspecified or equal to zero, use NumOfTimeStepInHour, otherwise use supplied
        if (ts <= 0) {
            ts = s_glob->TimeStepsInHour;
        }

        return daySched->tsVals[(thisHr - 1) * s_glob->TimeStepsInHour + (ts - 1)];
    } // ScheduleDetailed::getHrTsVal()

    Real64 ScheduleConstant::getHrTsVal([[maybe_unused]] EnergyPlusData &state, [[maybe_unused]] int hr, [[maybe_unused]] int ts) const
    {
        // cf #10962 - We can't use currentValue as it could be overwritten by the EMS Sensor
        return this->tsVals.front();
    } // ScheduleConstant::getHrTsVal()

    Sched::Schedule *GetScheduleAlwaysOn(EnergyPlusData &state)
    {
        return state.dataSched->schedules[SchedNum_AlwaysOn];
    }

    Sched::Schedule *GetScheduleAlwaysOff(EnergyPlusData &state)
    {
        return state.dataSched->schedules[SchedNum_AlwaysOff];
    }

    Sched::Schedule *GetSchedule(EnergyPlusData &state, std::string const &name)
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   September 1997

        // PURPOSE OF THIS FUNCTION:
        // This function returns the internal pointer to Schedule "ScheduleName".
        auto const &s_sched = state.dataSched;

        auto found = s_sched->scheduleMap.find(name);
        if (found == s_sched->scheduleMap.end()) {
            return nullptr;
        }

        int schedNum = found->second;

        auto *sched = s_sched->schedules[schedNum];

        if (!sched->isUsed) {
            sched->isUsed = true;

            if (sched->type != SchedType::Constant) {

                auto *schedDetailed = dynamic_cast<ScheduleDetailed *>(sched);
                assert(schedDetailed != nullptr);

                schedDetailed->isUsed = true;
                for (int iWeek = 1; iWeek <= 366; ++iWeek) {
                    if (auto *weekSched = schedDetailed->weekScheds[iWeek]; weekSched != nullptr) {
                        if (weekSched->isUsed) {
                            continue;
                        }

                        weekSched->isUsed = true;
                        for (int iDayType = 1; iDayType < (int)DayType::Num; ++iDayType) {
                            auto *daySched = weekSched->dayScheds[iDayType];
                            daySched->isUsed = true;
                        }
                    }
                }
            }
        }
        return sched;
    } // GetSchedule()

    int GetScheduleNum(EnergyPlusData &state, std::string const &name)
    {
        auto *sched = GetSchedule(state, name);
        return (sched == nullptr) ? -1 : sched->Num;
    }

    Sched::WeekSchedule *GetWeekSchedule(EnergyPlusData &state, std::string const &name)
    {
        auto const &s_sched = state.dataSched;

        auto found = s_sched->weekScheduleMap.find(name);
        if (found == s_sched->weekScheduleMap.end()) {
            return nullptr;
        }

        int weekSchedNum = found->second;

        auto *weekSched = s_sched->weekSchedules[weekSchedNum];

        if (!weekSched->isUsed) {
            weekSched->isUsed = true;
            for (int iDayType = 1; iDayType < (int)DayType::Num; ++iDayType) {
                auto *daySched = weekSched->dayScheds[iDayType];
                daySched->isUsed = true;
            }
        }
        return weekSched;
    } // GetWeekSchedule()

    int GetWeekScheduleNum(EnergyPlusData &state, std::string const &name)
    {
        auto *weekSched = GetWeekSchedule(state, name);
        return (weekSched == nullptr) ? -1 : weekSched->Num;
    }

    Sched::DaySchedule *GetDaySchedule(EnergyPlusData &state, std::string const &name)
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   September 1997

        // PURPOSE OF THIS FUNCTION:
        // This function returns the internal pointer to Schedule "ScheduleName".
        auto const &s_sched = state.dataSched;

        auto found = s_sched->dayScheduleMap.find(name);
        if (found == s_sched->dayScheduleMap.end()) {
            return nullptr;
        }

        int daySchedNum = found->second;

        auto *daySched = s_sched->daySchedules[daySchedNum];

        daySched->isUsed = true;

        return daySched;
    } // GetDaySchedule()

    int GetDayScheduleNum(EnergyPlusData &state, std::string const &name)
    {
        auto *daySched = GetDaySchedule(state, name);
        return (daySched == nullptr) ? -1 : daySched->Num;
    }

    void ScheduleConstant::setMinMaxVals([[maybe_unused]] EnergyPlusData &state)
    {
        assert(!isMinMaxSet);
        minVal = maxVal = currentVal;
        isMinMaxSet = true;
    }

    std::vector<Real64> const &
    ScheduleConstant::getDayVals([[maybe_unused]] EnergyPlusData &state, [[maybe_unused]] int jDay, [[maybe_unused]] int dayofWeek)
    {
        assert((int)tsVals.size() == Constant::iHoursInDay * state.dataGlobal->TimeStepsInHour);
        return this->tsVals;
    } // ScheduleConstant::getDayVals()

    std::vector<Real64> const &ScheduleDetailed::getDayVals(EnergyPlusData &state, int jDay, int dayOfWeek)
    {
        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine returns an entire day's worth of schedule values.
        auto const &s_env = state.dataEnvrn;

        // Determine which Week Schedule is used
        auto const *weekSched = this->weekScheds[(jDay == -1) ? state.dataEnvrn->DayOfYear_Schedule : jDay];

        DaySchedule *daySched = nullptr;
        // Now, which day?
        if (dayOfWeek == -1) {
            daySched = weekSched->dayScheds[(s_env->HolidayIndex > 0) ? s_env->HolidayIndex : s_env->DayOfWeek];
        } else if (dayOfWeek <= 7 && s_env->HolidayIndex > 0) {
            daySched = weekSched->dayScheds[s_env->HolidayIndex];
        } else {
            daySched = weekSched->dayScheds[dayOfWeek];
        }

        return daySched->getDayVals(state);
    } // ScheduleDetailed::getDayVals()

    void ExternalInterfaceSetSchedule(EnergyPlusData &state,
                                      int schedNum,
                                      Real64 value // The new value for the schedule
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Michael Wetter
        //       DATE WRITTEN   February 2010

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine sets all values of the schedule referenced by 'ScheduleIndex'
        // to the value specified by 'Value'. The subroutine is used by the ExternalInterface to
        // write real-time data into a schedule so that EnergyPlus modules can use
        // real-time data by referencing a schedule. This allows overwriting setpoint
        // for supervisory controls or internal gains obtained from real-time occupancy
        // measurements.
        auto const &s_glob = state.dataGlobal;
        auto const &s_sched = state.dataSched;
        auto *daySched = s_sched->daySchedules[schedNum];

        for (int hr = 0; hr < Constant::iHoursInDay; ++hr) {
            for (int ts = 0; ts < s_glob->TimeStepsInHour; ++ts) {
                daySched->tsVals[hr * s_glob->TimeStepsInHour + ts] = value;
            }
        }
    } // ExternalInterfaceSetSchedule()

    void ProcessIntervalFields(EnergyPlusData &state,
                               Array1S_string const Untils,
                               Array1S<Real64> const Numbers,
                               int const NumUntils,
                               int const NumNumbers,
                               std::array<Real64, Constant::iMinutesInDay> &minuteVals,
                               std::array<bool, Constant::iMinutesInDay> &setMinuteVals,
                               bool &ErrorsFound,
                               std::string const &DayScheduleName, // Name (used for errors)
                               std::string const &ErrContext,      // Context (used for errors)
                               Interpolation interpolation         // enumeration on how to interpolate values in schedule
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         <author>
        //       DATE WRITTEN   <date_written>

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine processes the "interval" fields with/without optional "until" in front of
        // time (hh:mm).

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int HHField;
        int MMField;

        int begHr = 0;   // starting hour
        int begMin = 0;  // starting minute
        int endHr = -1;  // ending hour
        int endMin = -1; // ending minute
        std::string::size_type sFld;

        int totalMinutes;
        Real64 incrementPerMinute;
        Real64 curValue;

        std::fill(minuteVals.begin(), minuteVals.end(), 0.0);
        std::fill(setMinuteVals.begin(), setMinuteVals.end(), false);

        sFld = 0;

        Real64 StartValue = 0;
        Real64 EndValue = 0;

        if (NumUntils != NumNumbers) {
            ShowSevereError(state,
                            format("ProcessScheduleInput: ProcessIntervalFields, number of Time fields does not match number of value fields, {}={}",
                                   ErrContext,
                                   DayScheduleName));
            ErrorsFound = true;
            return;
        }

        for (int Count = 1; Count <= NumUntils; ++Count) {
            std::string const &until = Untils(Count);
            int Pos = index(until, "UNTIL");
            if (Pos == 0) {
                if (until[5] == ':') {
                    sFld = 6;
                } else {
                    sFld = 5;
                }
                DecodeHHMMField(state, until.substr(sFld), HHField, MMField, ErrorsFound, DayScheduleName, until, interpolation);
            } else if (Pos == (int)std::string::npos) {
                DecodeHHMMField(state, until, HHField, MMField, ErrorsFound, DayScheduleName, until, interpolation);
            } else { // Until found but wasn't first field
                ShowSevereError(state, format("ProcessScheduleInput: ProcessIntervalFields, Invalid \"Until\" field encountered={}", until));
                ShowContinueError(state, format("Occurred in Day Schedule={}", DayScheduleName));
                ErrorsFound = true;
                continue;
            }
            // Field decoded
            if (HHField < 0 || HHField > Constant::iHoursInDay || MMField < 0 || MMField > Constant::iMinutesInHour) {
                ShowSevereError(state, format("ProcessScheduleInput: ProcessIntervalFields, Invalid \"Until\" field encountered={}", until));
                ShowContinueError(state, format("Occurred in Day Schedule={}", DayScheduleName));
                ErrorsFound = true;
                continue;
            }
            if (HHField == Constant::iHoursInDay && MMField > 0 && MMField < Constant::iMinutesInHour) {
                ShowWarningError(state, format("ProcessScheduleInput: ProcessIntervalFields, Invalid \"Until\" field encountered={}", Untils(Count)));
                ShowContinueError(state, format("Occurred in Day Schedule={}", DayScheduleName));
                ShowContinueError(state, "Terminating the field at 24:00");
                MMField = 0;
            }

            // Fill in values
            if (MMField == 0) {
                endHr = HHField - 1;
                endMin = Constant::iMinutesInHour - 1;
            } else if (MMField < Constant::iMinutesInHour) {
                endHr = HHField;
                endMin = MMField - 1;
            }

            if (interpolation == Interpolation::Linear) {
                totalMinutes = (endHr - begHr) * Constant::iMinutesInHour + (endMin - begMin) + 1;
                if (totalMinutes == 0) {
                    totalMinutes = 1; // protect future division
                }
                if (Count == 1) {
                    StartValue = Numbers(Count); // assume first period is flat
                    EndValue = Numbers(Count);
                } else {
                    StartValue = EndValue;
                    EndValue = Numbers(Count);
                }
                incrementPerMinute = (EndValue - StartValue) / totalMinutes;
                curValue = StartValue + incrementPerMinute;
            }

            if (begHr > endHr) {
                if (begHr == endHr + 1 && begMin == 0 && endMin == Constant::iMinutesInHour - 1) {
                    ShowWarningError(state,
                                     format("ProcessScheduleInput: ProcessIntervalFields, Processing time fields, zero time interval detected, {}={}",
                                            ErrContext,
                                            DayScheduleName));
                } else {
                    ShowSevereError(state,
                                    format("ProcessScheduleInput: ProcessIntervalFields, Processing time fields, overlapping times detected, {}={}",
                                           ErrContext,
                                           DayScheduleName));
                    ErrorsFound = true;
                }

            } else if (begHr == endHr) {
                for (int iMin = begMin; iMin <= endMin; ++iMin) {
                    if (setMinuteVals[begHr * Constant::iMinutesInHour + iMin] == true) {
                        ShowSevereError(
                            state,
                            format("ProcessScheduleInput: ProcessIntervalFields, Processing time fields, overlapping times detected, {}={}",
                                   ErrContext,
                                   DayScheduleName));
                        ErrorsFound = true;
                        goto UntilLoop_exit;
                    }
                }

                if (interpolation == Interpolation::Linear) {
                    for (int iMin = begMin; iMin <= endMin; ++iMin) {
                        minuteVals[begHr * Constant::iMinutesInHour + iMin] = curValue;
                        curValue += incrementPerMinute;
                        setMinuteVals[begHr * Constant::iMinutesInHour + iMin] = true;
                    }
                } else {
                    for (int iMin = begMin; iMin <= endMin; ++iMin) {
                        minuteVals[begHr * Constant::iMinutesInHour + iMin] = Numbers(Count);
                        setMinuteVals[begHr * Constant::iMinutesInHour + iMin] = true;
                    }
                }

                begMin = endMin + 1;
                if (begMin >= Constant::iMinutesInHour) {
                    ++begHr;
                    begMin = 0;
                }

            } else { // begHr < endHr
                if (interpolation == Interpolation::Linear) {
                    for (int iMin = begMin; iMin <= Constant::iMinutesInHour - 1; ++iMin) { // for portion of starting hour
                        minuteVals[begHr * Constant::iMinutesInHour + iMin] = curValue;
                        curValue += incrementPerMinute;
                        setMinuteVals[begHr * Constant::iMinutesInHour + iMin] = true;
                    }

                    for (int iHr = begHr + 1; iHr <= endHr - 1; ++iHr) { // for intermediate hours
                        for (int iMin = 0; iMin <= Constant::iMinutesInHour - 1; ++iMin) {
                            minuteVals[iHr * Constant::iMinutesInHour + iMin] = curValue;
                            curValue += incrementPerMinute;
                            setMinuteVals[iHr * Constant::iMinutesInHour + iMin] = true;
                        }
                    }

                    for (int iMin = 0; iMin <= endMin; ++iMin) { // for ending hour
                        minuteVals[endHr * Constant::iMinutesInHour + iMin] = curValue;
                        curValue += incrementPerMinute;
                        setMinuteVals[endHr * Constant::iMinutesInHour + iMin] = true;
                    }

                } else { // either no interpolation or "average" interpolation (average just is when the interval does not match the timestep)
                         // Fill values for first hour (which may not start at minute 0)
                         // For std::fill the end marker has to be 1 past the last position you want to fill
                    for (int iMin = begMin; iMin <= Constant::iMinutesInHour; ++iMin) {
                        minuteVals[begHr * Constant::iMinutesInHour + iMin] = Numbers(Count);
                        setMinuteVals[begHr * Constant::iMinutesInHour + iMin] = true;
                    }

                    // Fill values for middle hours (which start at minute 0 and end in minute 59)
                    if ((begHr + 1) <= (endHr - 1)) {
                        for (int iHr = begHr + 1; iHr <= endHr - 1; ++iHr) {
                            for (int iMin = 0; iMin <= Constant::iMinutesInHour - 1; ++iMin) {
                                minuteVals[iHr * Constant::iMinutesInHour + iMin] = Numbers(Count);
                                setMinuteVals[iHr * Constant::iMinutesInHour + iMin] = true;
                            }
                        }
                    }

                    // Fill values for last hour (which starts at minute 0 but may end on minute that isn't 59)
                    for (int iMin = 0; iMin <= endMin; ++iMin) {
                        minuteVals[endHr * Constant::iMinutesInHour + iMin] = Numbers(Count);
                        setMinuteVals[endHr * Constant::iMinutesInHour + iMin] = true;
                    }
                }

                begHr = endHr;
                begMin = endMin + 1;
                if (begMin >= Constant::iMinutesInHour) {
                    ++begHr;
                    begMin = 0;
                }
            }
        }
    UntilLoop_exit:;

        for (int iMin = 0; iMin < Constant::iMinutesInDay; ++iMin) {
            if (setMinuteVals[iMin] == false) {
                ShowSevereError(state,
                                format("ProcessScheduleInput: ProcessIntervalFields, Processing time fields, incomplete day detected, {}={}",
                                       ErrContext,
                                       DayScheduleName));
                ErrorsFound = true;
            }
        }
    }

    void DecodeHHMMField(EnergyPlusData &state,
                         std::string const &FieldValue,      // Input field value
                         int &RetHH,                         // Returned "hour"
                         int &RetMM,                         // Returned "minute"
                         bool &ErrorsFound,                  // True if errors found in this field
                         std::string const &DayScheduleName, // originating day schedule name
                         std::string const &FullFieldValue,  // Full Input field value
                         Interpolation interpolation         // enumeration on how to interpolate values in schedule
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K Lawrie
        //       DATE WRITTEN   January 2003

        // PURPOSE OF THIS SUBROUTINE:

        // This subroutine decodes a hhmm date field input as part of the "until" time in a schedule
        // representation.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        std::string hHour;
        std::string mMinute;

        std::string String = stripped(FieldValue);
        std::string::size_type const Pos = index(String, ':');
        bool nonIntegral = false;

        auto const &s_glob = state.dataGlobal;
        if (Pos == std::string::npos) {
            ShowSevereError(state,
                            format("ProcessScheduleInput: DecodeHHMMField, Invalid \"until\" field submitted (no : separator in hh:mm)={}",
                                   stripped(FullFieldValue)));
            ShowContinueError(state, format("Occurred in Day Schedule={}", DayScheduleName));
            ErrorsFound = true;
            return;
        } else if (Pos == 0) {
            RetHH = 0;
        } else {
            bool error = false;
            Real64 rRetHH = Util::ProcessNumber(String.substr(0, Pos), error);
            RetHH = int(rRetHH);
            if (double(RetHH) != rRetHH || error || rRetHH < 0.0) {
                if (double(RetHH) != rRetHH && rRetHH >= 0.0) {
                    ShowWarningError(state,
                                     format("ProcessScheduleInput: DecodeHHMMField, Invalid \"until\" field submitted (non-integer numeric in HH)={}",
                                            stripped(FullFieldValue)));
                    ShowContinueError(state, format("Other errors may result. Occurred in Day Schedule={}", DayScheduleName));
                    nonIntegral = true;
                } else {
                    ShowSevereError(state,
                                    format("ProcessScheduleInput: DecodeHHMMField, Invalid \"until\" field submitted (invalid numeric in HH)={}",
                                           stripped(FullFieldValue)));
                    ShowContinueError(
                        state, format("Field values must be integer and represent hours:minutes. Occurred in Day Schedule={}", DayScheduleName));
                    ErrorsFound = true;
                    return;
                }
            }
        }

        String.erase(0, Pos + 1);
        bool error = false;
        Real64 rRetMM = Util::ProcessNumber(String, error);
        RetMM = int(rRetMM);
        if (double(RetMM) != rRetMM || error || rRetMM < 0.0) {
            if (double(RetMM) != rRetMM && rRetMM >= 0.0) {
                ShowWarningError(state,
                                 format("ProcessScheduleInput: DecodeHHMMField, Invalid \"until\" field submitted (non-integer numeric in MM)={}",
                                        stripped(FullFieldValue)));
                ShowContinueError(state, format("Other errors may result. Occurred in Day Schedule={}", DayScheduleName));
                nonIntegral = true;
            } else {
                ShowSevereError(state,
                                format("ProcessScheduleInput: DecodeHHMMField, Invalid \"until\" field submitted (invalid numeric in MM)={}",
                                       stripped(FullFieldValue)));
                ShowContinueError(state,
                                  format("Field values must be integer and represent hours:minutes. Occurred in Day Schedule={}", DayScheduleName));
                ErrorsFound = true;
                return;
            }
        }

        if (nonIntegral) {
            std::string hHour; // these haven't been initialized?
            std::string mMinute;
            ShowContinueError(state, format("Until value to be used will be: {:2.2F}:{:2.2F}", hHour, mMinute));
        }
        if (interpolation == Interpolation::No) {
            if (!isMinuteMultipleOfTimestep(RetMM, s_glob->MinutesInTimeStep)) {
                ShowWarningError(
                    state,
                    format(
                        "ProcessScheduleInput: DecodeHHMMField, Invalid \"until\" field value is not a multiple of the minutes for each timestep: {}",
                        stripped(FullFieldValue)));
                ShowContinueError(state, format("Other errors may result. Occurred in Day Schedule={}", DayScheduleName));
            }
        }
    }

    bool isMinuteMultipleOfTimestep(int minute, int numMinutesPerTimestep)
    {
        if (minute != 0) {
            return (minute % numMinutesPerTimestep == 0);
        } else {
            return true;
        }
    }

    void ProcessForDayTypes(EnergyPlusData &state,
                            std::string const &ForDayField,               // Field containing the "FOR:..."
                            std::array<bool, (int)DayType::Num> &these,   // Array to contain returned "true" days
                            std::array<bool, (int)DayType::Num> &already, // Array of days already done
                            bool &ErrorsFound                             // Will be true if error found.
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   February 2003

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine processes a field "For: day types" and returns
        // those day types (can be multiple) from field.
        // Argument array dimensioning

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool OneValid = false;
        bool DupAssignment = false;

        // Just test for specific days
        if (has(ForDayField, "WEEKDAY")) {
            these[iDayType_Mon] = these[iDayType_Tue] = these[iDayType_Wed] = these[iDayType_Thu] = these[iDayType_Fri] = true;
            if (already[iDayType_Mon] || already[iDayType_Tue] || already[iDayType_Wed] || already[iDayType_Thu] || already[iDayType_Fri]) {
                DupAssignment = true;
            }
            already[iDayType_Mon] = already[iDayType_Tue] = already[iDayType_Wed] = already[iDayType_Thu] = already[iDayType_Fri] = true;
            OneValid = true;
        }
        if (has(ForDayField, "MONDAY")) { // Should this be an else-if
            these[iDayType_Mon] = true;
            if (already[iDayType_Mon]) {
                DupAssignment = true;
            } else {
                already[iDayType_Mon] = true;
            }
            OneValid = true;
        }
        if (has(ForDayField, "TUESDAY")) {
            these[iDayType_Tue] = true;
            if (already[iDayType_Tue]) {
                DupAssignment = true;
            } else {
                already[iDayType_Tue] = true;
            }
            OneValid = true;
        }
        if (has(ForDayField, "WEDNESDAY")) {
            these[iDayType_Wed] = true;
            if (already[iDayType_Wed]) {
                DupAssignment = true;
            } else {
                already[iDayType_Wed] = true;
            }
            OneValid = true;
        }
        if (has(ForDayField, "THURSDAY")) {
            these[iDayType_Thu] = true;
            if (already[iDayType_Thu]) {
                DupAssignment = true;
            } else {
                already[iDayType_Thu] = true;
            }
            OneValid = true;
        }
        if (has(ForDayField, "FRIDAY")) {
            these[iDayType_Fri] = true;
            if (already[iDayType_Fri]) {
                DupAssignment = true;
            } else {
                already[iDayType_Fri] = true;
            }
            OneValid = true;
        }
        if (has(ForDayField, "WEEKEND")) {
            these[iDayType_Sun] = these[iDayType_Sat] = true;
            if (already[iDayType_Sun] || already[iDayType_Sat]) {
                DupAssignment = true;
            }
            already[iDayType_Sun] = already[iDayType_Sat] = true;
            OneValid = true;
        }

        if (has(ForDayField, "SATURDAY")) {
            these[iDayType_Sat] = true;
            if (already[iDayType_Sat]) {
                DupAssignment = true;
            } else {
                already[iDayType_Sat] = true;
            }
            OneValid = true;
        }
        if (has(ForDayField, "SUNDAY")) {
            these[iDayType_Sun] = true;
            if (already[iDayType_Sun]) {
                DupAssignment = true;
            } else {
                already[iDayType_Sun] = true;
            }
            OneValid = true;
        }
        if (has(ForDayField, "CUSTOMDAY1")) {
            these[iDayType_Cus1] = true;
            if (already[iDayType_Cus1]) {
                DupAssignment = true;
            } else {
                already[iDayType_Cus1] = true;
            }
            OneValid = true;
        }
        if (has(ForDayField, "CUSTOMDAY2")) {
            these[iDayType_Cus2] = true;
            if (already[iDayType_Cus2]) {
                DupAssignment = true;
            } else {
                already[iDayType_Cus2] = true;
            }
            OneValid = true;
        }
        if (has(ForDayField, "ALLDAY")) {
            for (int iDay = 0; iDay < (int)DayType::Num; ++iDay) {
                these[iDay] = true;
                if (already[iDay]) {
                    DupAssignment = true;
                } else {
                    already[iDay] = true;
                }
            }
            OneValid = true;
        }
        if (has(ForDayField, "HOLIDAY")) {
            these[iDayType_Hol] = true;
            if (already[iDayType_Hol]) {
                DupAssignment = true;
            } else {
                already[iDayType_Hol] = true;
            }
            OneValid = true;
        }
        if (has(ForDayField, "SUMMER")) {
            these[iDayType_SumDes] = true;
            if (already[iDayType_SumDes]) {
                DupAssignment = true;
            } else {
                already[iDayType_SumDes] = true;
            }
            OneValid = true;
        }
        if (has(ForDayField, "WINTER")) {
            these[iDayType_WinDes] = true;
            if (already[iDayType_WinDes]) {
                DupAssignment = true;
            } else {
                already[iDayType_WinDes] = true;
            }
            OneValid = true;
        }
        if (has(ForDayField, "ALLOTHERDAY")) {
            for (int iDay = 0; iDay < (int)DayType::Num; ++iDay) {
                if (!already[iDay]) {
                    these[iDay] = already[iDay] = true;
                }
            }
            OneValid = true;
        }

        if (DupAssignment) {
            ShowSevereError(state, format("ProcessForDayTypes: Duplicate assignment attempted in \"for\" days field={}", ForDayField));
            ErrorsFound = true;
        }
        if (!OneValid) {
            ShowSevereError(state, format("ProcessForDayTypes: No valid day assignments found in \"for\" days field={}", ForDayField));
            ErrorsFound = true;
        }
    } // ProcessScheduleInput()

    void DaySchedule::setMinMaxVals([[maybe_unused]] EnergyPlusData &state)
    {
        assert(!this->isMinMaxSet);

        auto const &s_glob = state.dataGlobal;

        this->minVal = this->maxVal = this->tsVals[0];
        for (int i = 0; i < Constant::iHoursInDay * s_glob->TimeStepsInHour; ++i) {
            Real64 value = this->tsVals[i];
            if (value < this->minVal) {
                this->minVal = value;
            } else if (value > this->maxVal) {
                this->maxVal = value;
            }
        }

        this->isMinMaxSet = true;
    }

    void WeekSchedule::setMinMaxVals(EnergyPlusData &state)
    {
        assert(!this->isMinMaxSet);

        auto *daySched1 = this->dayScheds[1];
        if (!daySched1->isMinMaxSet) {
            daySched1->setMinMaxVals(state);
        }

        this->minVal = daySched1->minVal;
        this->maxVal = daySched1->maxVal;

        auto *daySchedPrev = daySched1;
        for (int iDay = 2; iDay < (int)DayType::Num; ++iDay) {
            auto *daySched = this->dayScheds[iDay];
            if (daySched == daySchedPrev) {
                continue;
            }

            if (!daySched->isMinMaxSet) {
                daySched->setMinMaxVals(state);
            }
            if (daySched->minVal < this->minVal) {
                this->minVal = daySched->minVal;
            }
            if (daySched->maxVal > this->maxVal) {
                this->maxVal = daySched->maxVal;
            }
            daySchedPrev = daySched;
        }

        this->isMinMaxSet = true;
    } // ScheduleWeek::setMinMaxVals()

    void ScheduleDetailed::setMinMaxVals(EnergyPlusData &state)
    {
        assert(!this->isMinMaxSet);

        auto *weekSched1 = this->weekScheds[1];
        if (!weekSched1->isMinMaxSet) {
            weekSched1->setMinMaxVals(state);
        }

        this->minVal = weekSched1->minVal;
        this->maxVal = weekSched1->maxVal;

        auto *weekSchedPrev = weekSched1;

        for (int iWeek = 2; iWeek <= 366; ++iWeek) {
            auto *weekSched = this->weekScheds[iWeek];
            if (iWeek == 366 && weekSched == nullptr) {
                continue;
            }
            if (weekSched == weekSchedPrev) {
                continue;
            }
            if (!weekSched->isMinMaxSet) {
                weekSched->setMinMaxVals(state);
            }

            if (weekSched->minVal < this->minVal) {
                this->minVal = weekSched->minVal;
            }
            if (weekSched->maxVal > this->maxVal) {
                this->maxVal = weekSched->maxVal;
            }
            weekSchedPrev = weekSched;
        }

        this->isMinMaxSet = true;
    }

    bool CheckScheduleValueMin(EnergyPlusData &state,
                               int const schedNum, // Which Schedule being tested
                               Clusive clu,
                               Real64 const min // Minimum desired value
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   February 2003

        // PURPOSE OF THIS FUNCTION:
        // This function checks the indicated schedule values for validity.

        // METHODOLOGY EMPLOYED:
        // Schedule data structure stores this on first validity check.  If there, then is returned else
        // looks up minimum and maximum values for the schedule and then sets result of function based on
        // requested minimum/maximum checks.

        return state.dataSched->schedules[schedNum]->checkMinVal(state, clu, min);
    }

    bool ScheduleBase::checkMinVal(EnergyPlusData &state, Clusive clu, Real64 min)
    {
        if (!this->isMinMaxSet) { // Set Minimum/Maximums for this schedule
            this->setMinMaxVals(state);
        }

        //  Min/max for schedule has been set.  Test.
        return (clu == Clusive::In) ? (FLT_EPSILON >= min - this->minVal) : (this->minVal > min);
    } // ScheduleDetailed::checkMinVal()

    bool ScheduleBase::checkMaxVal(EnergyPlusData &state, Clusive cluMax, Real64 const max)
    {
        if (!this->isMinMaxSet) {
            this->setMinMaxVals(state);
        }

        return (cluMax == Clusive::Ex) ? (this->maxVal < max) : (this->maxVal - max <= FLT_EPSILON);
    }

    bool ScheduleBase::checkMinMaxVals(EnergyPlusData &state,
                                       Clusive cluMin,   // Minimum indicator ('>', '>=')
                                       Real64 const min, // Minimum desired value
                                       Clusive cluMax,   // Maximum indicator ('<', ',=')
                                       Real64 const max) // Maximum desired value
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   February 2003

        // PURPOSE OF THIS FUNCTION:
        // This function checks the indicated schedule values for validity.

        // METHODOLOGY EMPLOYED:
        // Schedule data structure stores this on first validity check.  If there, then is returned else
        // looks up minimum and maximum values for the schedule and then sets result of function based on
        // requested minimum/maximum checks.

        if (!this->isMinMaxSet) {
            this->setMinMaxVals(state);
        }

        bool minOk = (cluMin == Clusive::Ex) ? (this->minVal > min) : (FLT_EPSILON >= min - this->minVal);
        bool maxOk = (cluMax == Clusive::Ex) ? (this->maxVal < max) : (this->maxVal - max <= FLT_EPSILON);

        return (minOk && maxOk);
    } // ScheduleBase::checkMinMaxVals()

    bool CheckScheduleValueMinMax(EnergyPlusData &state,
                                  int const schedNum, // Which Schedule being tested
                                  Clusive cluMin,     // Minimum indicator ('>', '>=')
                                  Real64 const min,   // Minimum desired value
                                  Clusive cluMax,     // Maximum indicator ('<', ',=')
                                  Real64 const max    // Maximum desired value
    )
    {
        // Wrapper for method
        return state.dataSched->schedules[schedNum]->checkMinMaxVals(state, cluMin, min, cluMax, max);
    } // CheckScheduleValueMinMax()

    bool ScheduleConstant::hasVal([[maybe_unused]] EnergyPlusData &state, Real64 const value) const
    {
        return value == this->currentVal;
    } // ScheduleConstant::hasVal()

    bool ScheduleDetailed::hasVal(EnergyPlusData &state, Real64 const value) const
    {
        auto const &s_sched = state.dataSched;
        auto const &s_glob = state.dataGlobal;

        // These arrays make sure you don't check the same day or week schedule twice
        std::vector<bool> weekSchedChecked;
        weekSchedChecked.resize(s_sched->weekSchedules.size());
        std::fill(weekSchedChecked.begin(), weekSchedChecked.end(), false);

        std::vector<bool> daySchedChecked;
        daySchedChecked.resize(s_sched->daySchedules.size());
        std::fill(daySchedChecked.begin(), daySchedChecked.end(), false);

        for (int iWeek = 1; iWeek <= 366; ++iWeek) {
            auto const *weekSched = this->weekScheds[iWeek];
            if (weekSchedChecked[weekSched->Num]) {
                continue;
            }

            for (int iDay = 1; iDay < (int)DayType::Num; ++iDay) {
                auto const *daySched = weekSched->dayScheds[iDay];
                if (daySchedChecked[daySched->Num]) {
                    continue;
                }

                for (int i = 0; i < Constant::iHoursInDay * s_glob->TimeStepsInHour; ++i) {
                    if (daySched->tsVals[i] == value) {
                        return true;
                    }
                }
                daySchedChecked[daySched->Num] = true;
            }
            weekSchedChecked[weekSched->Num] = true;
        }

        return false;
    } // ScheduleDetailed::hasVal()

    bool CheckScheduleValue(EnergyPlusData &state,
                            int const schedNum, // Which Schedule being tested
                            Real64 const value  // Actual desired value
    )
    {
        // Method wrapper
        return state.dataSched->schedules[schedNum]->hasVal(state, value);
    }

    bool CheckDayScheduleMinValues(EnergyPlusData &state,
                                   int const schedNum, // Which Day Schedule being tested
                                   Clusive cluMin,
                                   Real64 const min)
    {
        // Method wrapper
        return state.dataSched->daySchedules[schedNum]->checkMinVal(state, cluMin, min);
    } // CheckDayScheduleMinValues()

    bool ScheduleConstant::hasFractionalVal([[maybe_unused]] EnergyPlusData &state) const
    {
        return (this->currentVal > 0.0) && (this->currentVal < 1.0);
    } // ScheduleYear::hasFractionalVal()

    bool ScheduleDetailed::hasFractionalVal(EnergyPlusData &state) const
    {
        auto const &s_sched = state.dataSched;
        auto const &s_glob = state.dataGlobal;

        // These arrays make sure you don't check the same day or week schedule twice
        std::vector<bool> weekSchedChecked;
        weekSchedChecked.resize(s_sched->weekSchedules.size());
        std::fill(weekSchedChecked.begin(), weekSchedChecked.end(), false);

        std::vector<bool> daySchedChecked;
        daySchedChecked.resize(s_sched->daySchedules.size());
        std::fill(daySchedChecked.begin(), daySchedChecked.end(), false);

        for (int iWeek = 1; iWeek <= 366; ++iWeek) {
            auto const *weekSched = this->weekScheds[iWeek];
            if (weekSchedChecked[weekSched->Num]) {
                continue;
            }

            for (int iDay = 1; iDay < (int)DayType::Num; ++iDay) {
                auto const *daySched = weekSched->dayScheds[iDay];
                if (daySchedChecked[daySched->Num]) {
                    continue;
                }

                for (int i = 0; i < Constant::iHoursInDay * s_glob->TimeStepsInHour; ++i) {
                    if (daySched->tsVals[i] > 0.0 && daySched->tsVals[i] < 1.0) {
                        return true;
                    }
                }
                daySchedChecked[daySched->Num] = true;
            }
            weekSchedChecked[weekSched->Num] = true;
        }

        return false;
    } // ScheduleDetailed::hasFractionalVal()

    std::pair<Real64, Real64> ScheduleConstant::getMinMaxValsByDayType([[maybe_unused]] EnergyPlusData &state,
                                                                       [[maybe_unused]] DayTypeGroup const days)
    {
        return std::make_pair(this->currentVal, this->currentVal);
    } // ScheduleConstant::getMinMaxValsByDayType()

    std::pair<Real64, Real64> ScheduleDetailed::getMinMaxValsByDayType(EnergyPlusData &state, DayTypeGroup const days)
    {
        // J. Glazer - March 2024
        // finds the minimum and maximum for a specific set of day types for a given schedule
        constexpr std::array<std::array<bool, (int)DayType::Num>, (int)DayTypeGroup::Num> dayTypeFilters = {{
            //  Unused    Sun    Mon    Tues   Wed    Thur   Fri    Sat    Hol    Summer Winter Cust1  Cust2
            {false, false, true, true, true, true, true, false, false, false, false, false, false},     // Weekday
            {false, true, false, false, false, false, false, true, true, false, false, false, false},   // WeekendHoliday
            {false, false, false, false, false, false, false, false, false, true, false, false, false}, // SummerDesign
            {false, false, false, false, false, false, false, false, false, false, true, false, false}  // WinterDesign
        }};

        auto const &s_sched = state.dataSched;

        if (!this->isMinMaxSet) {
            this->setMinMaxVals(state);
        }

        if (!this->MaxMinByDayTypeSet[(int)days]) {

            bool firstSet = true;
            std::array<bool, (int)DayType::Num> const &dayTypeFilter = dayTypeFilters[(int)days];

            // These arrays make sure you don't check the same day or week schedule twice
            std::vector<bool> weekSchedChecked;
            weekSchedChecked.resize(s_sched->weekSchedules.size());
            std::fill(weekSchedChecked.begin(), weekSchedChecked.end(), false);

            std::vector<bool> daySchedChecked;
            daySchedChecked.resize(s_sched->daySchedules.size());
            std::fill(daySchedChecked.begin(), daySchedChecked.end(), false);

            this->MinByDayType[(int)days] = this->MaxByDayType[(int)days] = 0.0;

            for (int iDay = 1; iDay <= 366; ++iDay) {
                auto const *weekSched = this->weekScheds[iDay];
                if (weekSchedChecked[weekSched->Num]) {
                    continue;
                }

                for (int jDayType = 1; jDayType < (int)DayType::Num; ++jDayType) {
                    if (!dayTypeFilter[jDayType]) {
                        continue;
                    }

                    auto *daySched = weekSched->dayScheds[jDayType];
                    if (daySchedChecked[daySched->Num]) {
                        continue;
                    }

                    if (!daySched->isMinMaxSet) {
                        daySched->setMinMaxVals(state);
                    }

                    if (firstSet) {
                        this->MinByDayType[(int)days] = daySched->minVal;
                        this->MaxByDayType[(int)days] = daySched->maxVal;
                        firstSet = false;
                    } else {
                        this->MinByDayType[(int)days] = min(this->MinByDayType[(int)days], daySched->minVal);
                        this->MaxByDayType[(int)days] = max(this->MaxByDayType[(int)days], daySched->maxVal);
                    }

                    daySchedChecked[daySched->Num] = true;
                }
                weekSchedChecked[weekSched->Num] = true;
            }
            this->MaxMinByDayTypeSet[(int)days] = true;
        }
        return std::make_pair(this->MinByDayType[(int)days], this->MaxByDayType[(int)days]);
    } // ScheduleDetailed::getMinMaxValsByDayType()

    void ReportScheduleVals(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   February 2004

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine puts the proper current schedule values into the "reporting"
        // slot for later reporting.
        auto const &s_sched = state.dataSched;

        if (s_sched->DoScheduleReportingSetup) { // CurrentModuleObject='Any Schedule'
            for (auto *sched : s_sched->schedules) {
                // No variables for the built-in AlwaysOn and AlwaysOff schedules
                if (sched->Num == SchedNum_AlwaysOff || sched->Num == SchedNum_AlwaysOn) {
                    continue;
                }

                // Set Up Reporting
                SetupOutputVariable(state,
                                    "Schedule Value",
                                    Constant::Units::None,
                                    sched->currentVal,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    sched->Name);
            }
            s_sched->DoScheduleReportingSetup = false;
        }

        UpdateScheduleVals(state);
    }

    void ReportOrphanSchedules(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   April 2008

        // PURPOSE OF THIS SUBROUTINE:
        // In response to CR7498, report orphan (unused) schedule items.

        bool NeedOrphanMessage = true;
        bool NeedUseMessage = false;
        int NumCount = 0;

        auto const &s_sched = state.dataSched;
        auto const &s_glob = state.dataGlobal;

        for (auto const *sched : s_sched->schedules) {
            if (sched->isUsed) {
                continue;
            }
            if (NeedOrphanMessage && s_glob->DisplayUnusedSchedules) {
                ShowWarningError(state, "The following schedule names are \"Unused Schedules\".  These schedules are in the idf");
                ShowContinueError(state, " file but are never obtained by the simulation and therefore are NOT used.");
                NeedOrphanMessage = false;
            }
            if (s_glob->DisplayUnusedSchedules) {
                ShowMessage(state, format("Schedule:Year or Schedule:Compact or Schedule:File or Schedule:Constant={}", sched->Name));
            } else {
                ++NumCount;
            }
        }

        if (NumCount > 0) {
            ShowMessage(state, format("There are {} unused schedules in input.", NumCount));
            NeedUseMessage = true;
        }

        NeedOrphanMessage = true;
        NumCount = 0;

        for (auto *weekSched : s_sched->weekSchedules) {
            if (weekSched->isUsed) {
                continue;
            }
            if (weekSched->Name.empty()) {
                continue;
            }
            if (NeedOrphanMessage && s_glob->DisplayUnusedSchedules) {
                ShowWarningError(state, "The following week schedule names are \"Unused Schedules\".  These schedules are in the idf");
                ShowContinueError(state, " file but are never obtained by the simulation and therefore are NOT used.");
                NeedOrphanMessage = false;
            }
            if (s_glob->DisplayUnusedSchedules) {
                ShowMessage(state, format("Schedule:Week:Daily or Schedule:Week:Compact={}", weekSched->Name));
            } else {
                ++NumCount;
            }
        }

        if (NumCount > 0) {
            ShowMessage(state, fmt::format("There are {} unused week schedules in input.", NumCount));
            NeedUseMessage = true;
        }

        NeedOrphanMessage = true;
        NumCount = 0;

        for (auto *daySched : s_sched->daySchedules) {
            if (daySched->isUsed) {
                continue;
            }
            if (daySched->Name.empty()) {
                continue;
            }
            if (NeedOrphanMessage && s_glob->DisplayUnusedSchedules) {
                ShowWarningError(state, "The following day schedule names are \"Unused Schedules\".  These schedules are in the idf");
                ShowContinueError(state, " file but are never obtained by the simulation and therefore are NOT used.");
                NeedOrphanMessage = false;
            }

            if (s_glob->DisplayUnusedSchedules) {
                ShowMessage(state, format("Schedule:Day:Hourly or Schedule:Day:Interval or Schedule:Day:List={}", daySched->Name));
            } else {
                ++NumCount;
            }
        }

        if (NumCount > 0) {
            ShowMessage(state, format("There are {} unused day schedules in input.", NumCount));
            NeedUseMessage = true;
        }

        if (NeedUseMessage) {
            ShowMessage(state, "Use Output:Diagnostics,DisplayUnusedSchedules; to see them.");
        }
    } // ReportOrphanSchedules()

    // returns the annual full load hours for a schedule - essentially the sum of the hourly values
    Real64 ScheduleConstant::getAnnualHoursFullLoad([[maybe_unused]] EnergyPlusData &state,
                                                    int const StartDayOfWeek, // Day of week for start of year
                                                    bool const isLeapYear     // true if it is a leap year containing February 29
    )
    {
        if (StartDayOfWeek < iDayType_Sun || StartDayOfWeek > iDayType_Sat) {
            return 0.0; // Assert this instead?
        }

        int DaysInYear = (isLeapYear) ? 366 : 365;
        return DaysInYear * Constant::iHoursInDay * this->currentVal;
    }

    // returns the annual full load hours for a schedule - essentially the sum of the hourly values
    Real64 ScheduleDetailed::getAnnualHoursFullLoad(EnergyPlusData &state,
                                                    int const StartDayOfWeek, // Day of week for start of year
                                                    bool const isLeapYear     // true if it is a leap year containing February 29
    )
    {
        // J. Glazer - July 2017
        // adapted from Linda K. Lawrie original code for ScheduleAverageHoursPerWeek()
        auto const &s_glob = state.dataGlobal;

        int DaysInYear = (isLeapYear) ? 366 : 365;

        int DayT = StartDayOfWeek;
        Real64 TotalHours = 0.0;

        if (DayT < iDayType_Sun || DayT > iDayType_Sat) {
            return TotalHours;
        }

        for (int iDay = 1; iDay <= DaysInYear; ++iDay) {
            auto const *weekSched = this->weekScheds[iDay];
            auto const *daySched = weekSched->dayScheds[DayT];

            TotalHours += daySched->sumTsVals / double(s_glob->TimeStepsInHour);
            ++DayT;
            if (DayT > iDayType_Sat) {
                DayT = iDayType_Sun;
            }
        }

        return TotalHours;
    }

    // returns the average number of hours per week based on the schedule index provided
    Real64 Schedule::getAverageWeeklyHoursFullLoad(EnergyPlusData &state,
                                                   int const StartDayOfWeek, // Day of week for start of year
                                                   bool const isLeapYear     // true if it is a leap year containing February 29
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Linda K. Lawrie
        //       DATE WRITTEN   August 2006
        //       MODIFIED       September 2012; Glazer - CR8849

        // PURPOSE OF THIS FUNCTION:
        // This function returns the "average" hours per week for a schedule over
        // the entire year.

        Real64 WeeksInYear = (isLeapYear) ? (366.0 / 7.0) : (365.0 / 7.0);
        return this->getAnnualHoursFullLoad(state, StartDayOfWeek, isLeapYear) / WeeksInYear;
    }

    // returns the annual hours greater than 1% for a schedule - essentially the number of hours with any operation
    Real64 ScheduleDetailed::getAnnualHoursGreaterThan1Percent(EnergyPlusData &state,
                                                               int const StartDayOfWeek, // Day of week for start of year
                                                               bool const isItLeapYear   // true if it is a leap year containing February 29
    )
    {
        // J. Glazer - July 2017
        // adapted from Linda K. Lawrie original code for ScheduleAverageHoursPerWeek()
        auto const &s_glob = state.dataGlobal;

        int DaysInYear = (isItLeapYear) ? 366 : 365;

        int DayT = StartDayOfWeek;
        Real64 TotalHours = 0.0;

        if (DayT < iDayType_Sun || DayT > iDayType_Sat) {
            return TotalHours;
        }

        for (int iDay = 1; iDay <= DaysInYear; ++iDay) {
            auto const *weekSched = this->weekScheds[iDay];
            auto const *daySched = weekSched->dayScheds[DayT];
            for (int i = 0; i < Constant::iHoursInDay * s_glob->TimeStepsInHour; ++i) {
                if (daySched->tsVals[i] > 0.0) {
                    TotalHours += s_glob->TimeStepZone;
                }
            }

            ++DayT;
            if (DayT > iDayType_Sat) {
                DayT = iDayType_Sun;
            }
        }

        return TotalHours;
    } // ScheduleDetailed::getAnnualHoursGreaterThan1Percent()

    // returns the annual hours greater than 1% for a schedule - essentially the number of hours with any operation
    Real64 ScheduleConstant::getAnnualHoursGreaterThan1Percent([[maybe_unused]] EnergyPlusData &state,
                                                               int const StartDayOfWeek, // Day of week for start of year
                                                               bool const isItLeapYear   // true if it is a leap year containing February 29
    )
    {
        int DaysInYear = (isItLeapYear) ? 366 : 365;

        if (StartDayOfWeek < iDayType_Sun || StartDayOfWeek > iDayType_Sat) {
            return 0.0; // Assert this instead?
        }

        return (this->currentVal > 0.0) ? (Constant::rHoursInDay * DaysInYear) : 0;
    } // ScheduleConstant::getHoursGreaterThan1Percent()

    // returns the temperature value from a schedule at a certain time for the first day of the week in either January or July
    std::tuple<Real64, int, std::string>
    ScheduleDetailed::getValAndCountOnDay(EnergyPlusData &state, bool const isSummer, DayType const dayOfWeek, int const hourOfDay)
    {
        // J.Glazer - Aug 2017

        auto const &s_glob = state.dataGlobal;

        // determine month to use based on hemiphere and season
        int month;
        if (isSummer) {
            month = (state.dataEnvrn->Latitude > 0.) ? 7 : 1;
        } else {
            month = (state.dataEnvrn->Latitude > 0.) ? 1 : 7;
        }

        std::string monthName = (month == 1) ? "January" : "July";

        int jdateSelect = General::nthDayOfWeekOfMonth(state, (int)dayOfWeek, 1, month);

        // determine number of days in year
        int DaysInYear = (state.dataEnvrn->CurrentYearIsLeapYear) ? 366 : 365;

        // should adjust date if lands on a holiday but for now assume that it does not

        // adjust time of day for daylight savings time
        int hourSelect = hourOfDay + state.dataWeather->DSTIndex(jdateSelect);

        // get the value at the selected time
        int constexpr firstTimeStep = 1;
        auto const *weekSched = this->weekScheds[jdateSelect];
        auto const *daySched = weekSched->dayScheds[(int)dayOfWeek];

        Real64 value = daySched->tsVals[(hourSelect - 1) * state.dataGlobal->TimeStepsInHour + (firstTimeStep - 1)];
        int countOfSame = 0;

        // count the number of times with that same value
        for (int jdateOfYear = 1; jdateOfYear <= DaysInYear; ++jdateOfYear) {
            auto const *wSched = this->weekScheds[jdateOfYear];
            if (wSched == weekSched) { // if same week schedule can short circuit rest of testing and increment counter
                ++countOfSame;
                continue;
            }

            auto const *dSched = wSched->dayScheds[(int)dayOfWeek];
            if (dSched == daySched) { // if same day schedule can short circuit rest of testing and increment counter
                ++countOfSame;
                continue;
            }

            if (dSched->tsVals[(hourSelect - 1) * s_glob->TimeStepsInHour + (firstTimeStep - 1)] == value) {
                ++countOfSame;
            }
        }

        return std::make_tuple(value, countOfSame, monthName);
    } // ScheduleDetailed::getValAndCountOnDay()

    // returns the temperature value from a schedule at a certain time for the first day of the week in either January or July
    std::tuple<Real64, int, std::string> ScheduleConstant::getValAndCountOnDay(EnergyPlusData &state,
                                                                               bool const isSummer,
                                                                               [[maybe_unused]] DayType const dayOfWeek,
                                                                               [[maybe_unused]] int const hourOfDay)
    {
        // determine month to use based on hemiphere and season
        int month;
        if (isSummer) {
            month = (state.dataEnvrn->Latitude > 0.) ? 7 : 1;
        } else {
            month = (state.dataEnvrn->Latitude > 0.) ? 1 : 7;
        }

        std::string monthName = (month == 1) ? "January" : "July";
        int DaysInYear = (state.dataEnvrn->CurrentYearIsLeapYear) ? 366 : 365;
        return std::make_tuple(this->currentVal, DaysInYear, monthName);
    } // ScheduleConstant::getValAndCountOnDay()

    void ShowSevereBadMin(EnergyPlusData &state,
                          ErrorObjectHeader const &eoh,
                          std::string_view fieldName,
                          std::string_view fieldVal,
                          Clusive cluMin,
                          Real64 minVal,
                          std::string_view msg)
    {
        ShowSevereError(state, format("{}: {} = {}", eoh.routineName, eoh.objectType, eoh.objectName));
        ShowContinueError(
            state, format("{} = {}, schedule contains values that are {} {}", fieldName, fieldVal, cluMin == Clusive::In ? "<" : "<=", minVal));
        if (!msg.empty()) {
            ShowContinueError(state, format("{}", msg));
        }
    }

    void ShowSevereBadMax(EnergyPlusData &state,
                          ErrorObjectHeader const &eoh,
                          std::string_view fieldName,
                          std::string_view fieldVal,
                          Clusive cluMax,
                          Real64 maxVal,
                          std::string_view msg)
    {
        ShowSevereError(state, format("{}: {} = {}", eoh.routineName, eoh.objectType, eoh.objectName));
        ShowContinueError(
            state, format("{} = {}, schedule contains values that are {} {}", fieldName, fieldVal, cluMax == Clusive::In ? ">" : ">=", maxVal));
        if (!msg.empty()) {
            ShowContinueError(state, format("{}", msg));
        }
    }

    void ShowSevereBadMinMax(EnergyPlusData &state,
                             ErrorObjectHeader const &eoh,
                             std::string_view fieldName,
                             std::string_view fieldVal,
                             Clusive cluMin,
                             Real64 minVal,
                             Clusive cluMax,
                             Real64 maxVal,
                             std::string_view msg)
    {
        ShowSevereError(state, format("{}: {} = {}", eoh.routineName, eoh.objectType, eoh.objectName));
        ShowContinueError(state,
                          format("{} = {}, schedule contains values that are {} {} and/or {} {}",
                                 fieldName,
                                 fieldVal,
                                 cluMin == Clusive::In ? "<" : "<=",
                                 minVal,
                                 cluMax == Clusive::In ? ">" : ">=",
                                 maxVal));
        if (!msg.empty()) {
            ShowContinueError(state, format("{}", msg));
        }
    }

    void ShowWarningBadMin(EnergyPlusData &state,
                           ErrorObjectHeader const &eoh,
                           std::string_view fieldName,
                           std::string_view fieldVal,
                           Clusive cluMin,
                           Real64 minVal,
                           std::string_view msg)
    {
        ShowWarningError(state, format("{}: {} = {}", eoh.routineName, eoh.objectType, eoh.objectName));
        ShowContinueError(
            state, format("{} = {}, schedule contains values that are {} {}", fieldName, fieldVal, cluMin == Clusive::In ? "<" : "<=", minVal));
        if (!msg.empty()) {
            ShowContinueError(state, format("{}", msg));
        }
    }

    void ShowWarningBadMax(EnergyPlusData &state,
                           ErrorObjectHeader const &eoh,
                           std::string_view fieldName,
                           std::string_view fieldVal,
                           Clusive cluMax,
                           Real64 maxVal,
                           std::string_view msg)
    {
        ShowWarningError(state, format("{}: {} = {}", eoh.routineName, eoh.objectType, eoh.objectName));
        ShowContinueError(
            state, format("{} = {}, schedule contains values that are {} {}", fieldName, fieldVal, cluMax == Clusive::In ? ">" : ">=", maxVal));
        if (!msg.empty()) {
            ShowContinueError(state, format("{}", msg));
        }
    }

    void ShowWarningBadMinMax(EnergyPlusData &state,
                              ErrorObjectHeader const &eoh,
                              std::string_view fieldName,
                              std::string_view fieldVal,
                              Clusive cluMin,
                              Real64 minVal,
                              Clusive cluMax,
                              Real64 maxVal,
                              std::string_view msg)
    {
        ShowWarningError(state, format("{}: {} = {}", eoh.routineName, eoh.objectType, eoh.objectName));
        ShowContinueError(state,
                          format("{} = {}, schedule contains values that are {} {} and/or {} {}",
                                 fieldName,
                                 fieldVal,
                                 cluMin == Clusive::In ? "<" : "<=",
                                 minVal,
                                 cluMax == Clusive::In ? ">" : ">=",
                                 maxVal));
        if (!msg.empty()) {
            ShowContinueError(state, format("{}", msg));
        }
    }

} // namespace Sched

} // namespace EnergyPlus
