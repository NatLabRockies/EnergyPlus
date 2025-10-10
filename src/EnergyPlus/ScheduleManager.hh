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

#ifndef ScheduleManager_hh_INCLUDED
#define ScheduleManager_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array1S.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace Sched {

    constexpr int SchedNum_Invalid = -1;
    constexpr int SchedNum_AlwaysOff = 0;
    constexpr int SchedNum_AlwaysOn = 1;

    enum class DayType
    {
        Invalid = -1,
        Unused, // This is annoying.  Will get rid of it later
        Sunday,
        Monday,
        Tuesday,
        Wednesday,
        Thursday,
        Friday,
        Saturday,
        Holiday,
        SummerDesignDay,
        WinterDesignDay,
        CustomDay1,
        CustomDay2,
        Num
    };

    constexpr int iDayType_Sun = (int)DayType::Sunday;
    constexpr int iDayType_Mon = (int)DayType::Monday;
    constexpr int iDayType_Tue = (int)DayType::Tuesday;
    constexpr int iDayType_Wed = (int)DayType::Wednesday;
    constexpr int iDayType_Thu = (int)DayType::Thursday;
    constexpr int iDayType_Fri = (int)DayType::Friday;
    constexpr int iDayType_Sat = (int)DayType::Saturday;
    constexpr int iDayType_Hol = (int)DayType::Holiday;
    constexpr int iDayType_SumDes = (int)DayType::SummerDesignDay;
    constexpr int iDayType_WinDes = (int)DayType::WinterDesignDay;
    constexpr int iDayType_Cus1 = (int)DayType::CustomDay1;
    constexpr int iDayType_Cus2 = (int)DayType::CustomDay2;

    extern const std::array<std::string_view, (int)DayType::Num> dayTypeNames;
    extern const std::array<std::string_view, (int)DayType::Num> dayTypeNamesUC;

    enum class DayTypeGroup
    {
        Invalid = -1,
        Weekday,
        WeekEndHoliday,
        SummerDesignDay,
        WinterDesignDay,
        Num
    };

    enum class SchedType
    {
        Invalid = -1,
        Year,
        Compact,
        File,
        Constant,
        External,
        Num
    };

    enum class ReportLevel
    {
        Invalid = -1,
        Hourly,
        TimeStep,
        Num
    };

    enum class Interpolation
    {
        Invalid = -1,
        No,      // no interpolation
        Average, // interpolation only to resolve time intervals not matching timestep lengths (this was previously interpolate:yes)
        Linear,  // linear interpolation from the previous time to the current time for the entire schedule
        Num
    };

    enum class LimitUnits
    {
        Invalid = -1,
        Dimensionless,
        Temperature,
        DeltaTemperature,
        PrecipitationRate,
        Angle,
        ConvectionCoefficient,
        ActivityLevel,
        Velocity,
        Capacity,
        Power,
        Availability,
        Percent,
        Control,
        Mode,
        Num
    };

    struct ScheduleType
    {
        // Members
        std::string Name;       // Schedule Type Name
        int Num;                // index in vector, useful sometimes
        bool isLimited = false; // True if this Schedule Type has limits
        Real64 minVal = 0.0;    // Minimum for limited schedule
        Real64 maxVal = 0.0;    // Maximum for limited schedule
        bool isReal = true;     // True if this is a "real" schedule, false if integer
        LimitUnits limitUnits = LimitUnits::Invalid;
    };

    struct ScheduleBase
    {
        std::string Name;
        int Num = SchedNum_Invalid;
        bool isUsed = false;

        Real64 maxVal = 0.0; // maximum of all TSValue's
        Real64 minVal = 0.0; // minimum of all TSValue's
        bool isMinMaxSet = false;

        ScheduleBase() = default;

        virtual void can_instantiate() = 0; // abstract base class

        virtual void setMinMaxVals(EnergyPlusData &state) = 0;
        Real64 getMinVal(EnergyPlusData &state);
        Real64 getMaxVal(EnergyPlusData &state);

        bool checkMinMaxVals(EnergyPlusData &state, Clusive cluMin, Real64 const min, Clusive cluMax, Real64 const max);
        bool checkMinVal(EnergyPlusData &state, Clusive cluMin, Real64 const min);
        bool checkMaxVal(EnergyPlusData &state, Clusive cluMax, Real64 const max);
    };

    struct DayOrYearSchedule : ScheduleBase
    {
        DayOrYearSchedule() = default;
        virtual ~DayOrYearSchedule() = default;

        virtual std::vector<Real64> const &getDayVals([[maybe_unused]] EnergyPlusData &state, int jDay = -1, int dayOfWeek = -1) = 0;
    };

    struct DaySchedule : DayOrYearSchedule
    {
        int schedTypeNum = SchedNum_Invalid; // Index of Schedule Type

        Interpolation interpolation = Interpolation::No; // Indicator for interval interpolation. If not "interpolated", False.  Else True
        std::vector<Real64> tsVals;                      // Value array by simulation timestep
        Real64 sumTsVals = 0.0;

        DaySchedule() = default;
        virtual ~DaySchedule() = default;
        void can_instantiate()
        {
            assert(false);
        } // makes class concrete, but don't call this

        bool checkValsForLimitViolations(EnergyPlusData &state) const;
        bool checkValsForBadIntegers(EnergyPlusData &state) const;
        void populateFromMinuteVals(EnergyPlusData &state, std::array<Real64, Constant::iMinutesInDay> const &minuteVals);
        std::vector<Real64> const &getDayVals([[maybe_unused]] EnergyPlusData &state, int jDay = -1, int dayOfWeek = -1)
        {
            return tsVals;
        }
        void setMinMaxVals(EnergyPlusData &state);
    };

    struct WeekSchedule : public ScheduleBase
    {
        // Members
        std::array<DaySchedule *, (int)DayType::Num> dayScheds = {nullptr};

        WeekSchedule() = default;
        virtual ~WeekSchedule() = default;
        void can_instantiate()
        {
            assert(false);
        } // makes class concrete, but don't call this

        void setMinMaxVals(EnergyPlusData &state);
    };

    struct Schedule : public DayOrYearSchedule
    {
        SchedType type = SchedType::Invalid;

        int schedTypeNum = SchedNum_Invalid; // Index of Schedule Type
        bool EMSActuatedOn = false;          // indicates if EMS computed
        Real64 EMSVal = 0.0;                 // EMS value

        Real64 currentVal = 0.0;

        Schedule()
        {
            type = SchedType::Constant;
        }

        virtual ~Schedule() = default;

        Real64 getCurrentVal() const
        {
            return EMSActuatedOn ? EMSVal : currentVal;
        }

        // Looks up a given Schedule value for an hour & timestep, minding whether DST is enabled or not
        // Negative ts => unspecified, will use TimeStepsInHour
        virtual Real64 getHrTsVal(EnergyPlusData &state, int hr, int ts = -1) const = 0;
        virtual bool hasVal(EnergyPlusData &state, Real64 const val) const = 0;
        virtual bool hasFractionalVal(EnergyPlusData &state) const = 0;

        virtual std::pair<Real64, Real64> getMinMaxValsByDayType(EnergyPlusData &state, DayTypeGroup const days) = 0;

        Real64 getAverageWeeklyHoursFullLoad(EnergyPlusData &state, int const startDayOfWeek, bool const isLeapYear);
        virtual Real64 getAnnualHoursFullLoad(EnergyPlusData &state, int const StartDayOfWeek, bool const isLeapYear) = 0;
        virtual Real64 getAnnualHoursGreaterThan1Percent(EnergyPlusData &state, int const StartDayOfWeek, bool const isLeapYear) = 0;
        virtual std::tuple<Real64, int, std::string>
        getValAndCountOnDay(EnergyPlusData &state, bool const isSummer, DayType const dayOfWeek, int const hourOfDay) = 0;
    };

    struct ScheduleConstant : public Schedule
    {
        std::vector<Real64> tsVals;

        ScheduleConstant()
        {
            type = SchedType::Constant;
        }

        virtual ~ScheduleConstant() = default;

        void can_instantiate()
        {
            assert(false);
        } // makes class concrete, but don't call this

        // Looks up a given Schedule value for an hour & timestep, minding whether DST is enabled or not
        // Negative ts => unspecified, will use TimeStepsInHour
        Real64 getHrTsVal(EnergyPlusData &state, int hr, int ts = -1) const;

        std::vector<Real64> const &getDayVals(EnergyPlusData &state, int jDay = -1, int dayOfWeek = -1);

        bool hasVal(EnergyPlusData &state, Real64 const val) const;
        bool hasFractionalVal(EnergyPlusData &state) const;

        void setMinMaxVals(EnergyPlusData &state);

        std::pair<Real64, Real64> getMinMaxValsByDayType(EnergyPlusData &state, DayTypeGroup const days);

        Real64 getAnnualHoursFullLoad(EnergyPlusData &state, int const StartDayOfWeek, bool const isLeapYear);
        Real64 getAnnualHoursGreaterThan1Percent(EnergyPlusData &state, int const StartDayOfWeek, bool const isLeapYear);

        std::tuple<Real64, int, std::string>
        getValAndCountOnDay(EnergyPlusData &state, bool const isSummer, DayType const dayOfWeek, int const hourOfDay);
    };

    struct ScheduleDetailed : public Schedule
    {
        // Members
        std::array<WeekSchedule *, 367> weekScheds = {nullptr};           // one created for each day of possible simulation
        std::array<bool, (int)DayType::Num> MaxMinByDayTypeSet = {false}; // minimum and maximum values by daytype have been stored
        std::array<Real64, (int)DayType::Num> MinByDayType = {0.0};       // minimum values by daytype for this schedule
        std::array<Real64, (int)DayType::Num> MaxByDayType = {0.0};       // maximum values by daytype for this schedule
        bool UseDaylightSaving = true; // Toggles between daylight saving option to be inclused as "No" or "Yes" (default)

        ScheduleDetailed()
        {
            type = SchedType::Year;
        }

        virtual ~ScheduleDetailed() = default;

        void can_instantiate()
        {
            assert(false);
        } // makes class concrete, but don't call this

        std::vector<Real64> const &getDayVals(EnergyPlusData &state, int jDay = -1, int dayOfWeek = -1);

        bool hasVal(EnergyPlusData &state, Real64 const val) const;
        bool hasFractionalVal(EnergyPlusData &state) const;

        void setMinMaxVals(EnergyPlusData &state);

        // Looks up a given Schedule value for an hour & timestep, minding whether DST is enabled or not
        // Negative ts => unspecified, will use TimeStepsInHour
        Real64 getHrTsVal(EnergyPlusData &state, int hr, int ts = -1) const;

        std::pair<Real64, Real64> getMinMaxValsByDayType(EnergyPlusData &state, DayTypeGroup const days);

        Real64 getAnnualHoursFullLoad(EnergyPlusData &state, int const StartDayOfWeek, bool const isLeapYear);
        Real64 getAnnualHoursGreaterThan1Percent(EnergyPlusData &state, int const StartDayOfWeek, bool const isLeapYear);

        std::tuple<Real64, int, std::string>
        getValAndCountOnDay(EnergyPlusData &state, bool const isSummer, DayType const dayOfWeek, int const hourOfDay);
    };

    // Functions
    ScheduleDetailed *AddScheduleDetailed(EnergyPlusData &state, std::string const &name);
    ScheduleConstant *AddScheduleConstant(EnergyPlusData &state, std::string const &name, Real64 value = 0.0);
    DaySchedule *AddDaySchedule(EnergyPlusData &state, std::string const &name);
    WeekSchedule *AddWeekSchedule(EnergyPlusData &state, std::string const &name);

    void ProcessScheduleInput(EnergyPlusData &state);

    void InitConstantScheduleData(EnergyPlusData &state);

    // If any Output:Schedules object is found, report to EIO
    void ReportScheduleTypeLimits(EnergyPlusData &state);

    void ReportScheduleDetails(EnergyPlusData &state, ReportLevel const LevelOfDetail);

    // Returns the CurrentScheduleValue
    Real64 GetHrTsScheduleVal(EnergyPlusData &state, int const schedNum, int const hr, int const ts = -1);
    // Updates each schedule value to the current timestep

    // Uses EMS value if actuated, otherwise calls LookUpScheduleValue with ThisHour=DataGlobals::HourOfDay, ThisTimeStep=DataGlobals::TimeStep
    void UpdateScheduleVals(EnergyPlusData &state);

    int GetScheduleTypeNum(EnergyPlusData &state, std::string const &name);

    int GetDayScheduleNum(EnergyPlusData &state, std::string const &name);
    DaySchedule *GetDaySchedule(EnergyPlusData &state, std::string const &name);

    int GetWeekScheduleNum(EnergyPlusData &state, std::string const &name);
    WeekSchedule *GetWeekSchedule(EnergyPlusData &state, std::string const &name);

    int GetScheduleNum(EnergyPlusData &state, std::string const &name);
    Schedule *GetSchedule(EnergyPlusData &state, std::string const &name);
    Schedule *GetScheduleAlwaysOn(EnergyPlusData &state);
    Schedule *GetScheduleAlwaysOff(EnergyPlusData &state);

    void ExternalInterfaceSetSchedule(EnergyPlusData &state,
                                      int schedNum,
                                      Real64 val // The new value for the schedule
    );

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
    );

    void DecodeHHMMField(EnergyPlusData &state,
                         std::string const &FieldVal,        // Input field value
                         int &RetHH,                         // Returned "hour"
                         int &RetMM,                         // Returned "minute"
                         bool &ErrorsFound,                  // True if errors found in this field
                         std::string const &DayScheduleName, // originating day schedule name
                         std::string const &FullFieldValue,  // Full Input field value
                         Interpolation interpolation         // enumeration on how to interpolate values in schedule
    );

    bool isMinuteMultipleOfTimestep(int minute, int numMinutesPerTimestep);

    void ProcessForDayTypes(EnergyPlusData &state,
                            std::string const &ForDayField,                 // Field containing the "FOR:..."
                            std::array<bool, (int)DayType::Num> &theseDays, // Array to contain returned "true" days
                            std::array<bool, (int)DayType::Num> &allDays,   // Array of days already done
                            bool &ErrorsFound                               // Will be true if error found.
    );

    void ReportScheduleVals(EnergyPlusData &state);

    void ReportOrphanSchedules(EnergyPlusData &state);

    void ShowSevereBadMin(EnergyPlusData &state,
                          ErrorObjectHeader const &eoh,
                          std::string_view schedField,
                          std::string_view schedName,
                          Clusive cluMin,
                          Real64 min,
                          std::string_view msg = {});

    void ShowWarningBadMin(EnergyPlusData &state,
                           ErrorObjectHeader const &eoh,
                           std::string_view schedField,
                           std::string_view schedName,
                           Clusive cluMin,
                           Real64 min,
                           std::string_view msg = {});

    void ShowSevereBadMax(EnergyPlusData &state,
                          ErrorObjectHeader const &eoh,
                          std::string_view schedField,
                          std::string_view schedName,
                          Clusive cluMax,
                          Real64 max,
                          std::string_view msg = {});

    void ShowWarningBadMax(EnergyPlusData &state,
                           ErrorObjectHeader const &eoh,
                           std::string_view schedField,
                           std::string_view schedName,
                           Clusive cluMax,
                           Real64 max,
                           std::string_view msg = {});

    void ShowSevereBadMinMax(EnergyPlusData &state,
                             ErrorObjectHeader const &eoh,
                             std::string_view schedField,
                             std::string_view schedName,
                             Clusive cluMin,
                             Real64 min,
                             Clusive cluMax,
                             Real64 max,
                             std::string_view msg = {});

    void ShowWarningBadMinMax(EnergyPlusData &state,
                              ErrorObjectHeader const &eoh,
                              std::string_view schedField,
                              std::string_view schedName,
                              Clusive cluMin,
                              Real64 min,
                              Clusive cluMax,
                              Real64 max,
                              std::string_view msg = {});

} // namespace Sched

struct ScheduleManagerData : BaseGlobalStruct
{
    bool CheckScheduleValMinMaxRunOnceOnly = true;
    bool DoScheduleReportingSetup = true;
    std::map<fs::path, nlohmann::json> UniqueProcessedExternalFiles;

    // Logical Variables for Module
    bool ScheduleInputProcessed = false;       // This is false until the Schedule Input has been processed.
    bool ScheduleFileShadingProcessed = false; // This is false unless there is a Schedule:File:Shading object.

    // Object Data
    std::vector<Sched::ScheduleType *> scheduleTypes; // Allowed Schedule Types
    std::vector<Sched::Schedule *> schedules;         // Year schedule
    std::vector<Sched::DaySchedule *> daySchedules;
    std::vector<Sched::WeekSchedule *> weekSchedules;

    std::map<std::string, int> scheduleTypeMap;
    std::map<std::string, int> scheduleMap;
    std::map<std::string, int> dayScheduleMap;
    std::map<std::string, int> weekScheduleMap;

    void init_constant_state(EnergyPlusData &state) override
    {
        Sched::InitConstantScheduleData(state);
    }

    void init_state(EnergyPlusData &state) override
    {
        Sched::ProcessScheduleInput(state);
    }

    void clear_state() override
    {
        CheckScheduleValMinMaxRunOnceOnly = true;
        UniqueProcessedExternalFiles.clear();
        DoScheduleReportingSetup = true;

        ScheduleInputProcessed = false;
        ScheduleFileShadingProcessed = false;

        for (int i = 0; i < (int)scheduleTypes.size(); ++i) {
            delete scheduleTypes[i];
        }
        scheduleTypes.clear(); // Allowed Schedule Types
        scheduleTypeMap.clear();

        for (int i = 0; i < (int)schedules.size(); ++i) {
            delete schedules[i];
        }
        schedules.clear(); // Schedule Storage
        scheduleMap.clear();

        for (int i = 0; i < (int)daySchedules.size(); ++i) {
            delete daySchedules[i];
        }
        daySchedules.clear();
        dayScheduleMap.clear();

        for (int i = 0; i < (int)weekSchedules.size(); ++i) {
            delete weekSchedules[i];
        }
        weekSchedules.clear();
        weekScheduleMap.clear();
    }
};

} // namespace EnergyPlus

#endif
