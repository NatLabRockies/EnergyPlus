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

#ifndef WeatherManager_hh_INCLUDED
#define WeatherManager_hh_INCLUDED

// C++ Headers
#include <vector>

// ObjexxFCL Headers
#include <ObjexxFCL/Array2D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EPVector.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/GroundTemperatureModeling/BaseGroundTemperatureModel.hh>
#include <EnergyPlus/ScheduleManager.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace Weather {

    enum class EpwHeaderType
    {
        Invalid = -1,
        Location = 0, // epw Headers are assumed to be in this order
        DesignConditions,
        TypicalExtremePeriods,
        GroundTemperatures,
        HolidaysDST,
        Comments1,
        Comments2,
        DataPeriods,
        Num
    };

    // Following are Date Types read in from EPW file or IDF
    enum class DateType
    {
        Invalid = -1,
        MonthDay = 1,
        NthDayInMonth = 2,
        LastDayInMonth = 3,
        Num
    };

    // Water mains temperatures calculation methods
    enum class WaterMainsTempCalcMethod
    {
        Invalid = -1,
        Schedule,
        Correlation,
        CorrelationFromWeatherFile,
        FixedDefault,
        Num
    };

    enum class DesDaySolarModel
    {
        Invalid = -1,
        ASHRAE_ClearSky,     // Design Day solar model ASHRAE ClearSky (default)
        Zhang_Huang,         // Design Day solar model Zhang Huang
        SolarModel_Schedule, // Design Day solar model (beam and diffuse) from user entered schedule
        ASHRAE_Tau,          // Design Day solar model ASHRAE tau (per 2009 HOF)
        ASHRAE_Tau2017,      // Design Day solar model ASHRAE tau (per 2013 and 2017 HOF)
        Num
    };

    static constexpr std::array<std::string_view, (int)DesDaySolarModel::Num> DesDaySolarModelNames = {
        "ASHRAEClearSky", "ZhangHuang", "Schedule", "ASHRAETau", "ASHRAETau2017"};

    static constexpr std::array<std::string_view, (int)DesDaySolarModel::Num> DesDaySolarModelNamesUC = {
        "ASHRAECLEARSKY", "ZHANGHUANG", "SCHEDULE", "ASHRAETAU", "ASHRAETAU2017"};

    // Design Day Humidity Indicating Type
    enum class DesDayHumIndType
    {
        Invalid = -1,
        WetBulb,   // Wetbulb (default)
        DewPoint,  // Dewpoint
        Enthalpy,  // Enthalpy
        HumRatio,  // Humidity Ratio
        RelHumSch, // relhum schedule
        WBProfDef, // Wetbulb default profile
        WBProfDif, // Wetbulb difference profile
        WBProfMul, // Wetbulb multiplier profile
        Num
    };

    static constexpr std::array<std::string_view, (int)DesDayHumIndType::Num> DesDayHumIndTypeNamesUC = {"WETBULB",
                                                                                                         "DEWPOINT",
                                                                                                         "ENTHALPY",
                                                                                                         "HUMIDITYRATIO",
                                                                                                         "RELATIVEHUMIDITYSCHEDULE",
                                                                                                         "WETBULBPROFILEDEFAULTMULTIPLIERS",
                                                                                                         "WETBULBPROFILEDIFFERENCESCHEDULE",
                                                                                                         "WETBULBPROFILEMULTIPLIERSCHEDULE"};

    // Design Day DryBulb Range Type
    enum class DesDayDryBulbRangeType
    {
        Invalid = -1,
        Default,    // Default Multipliers
        Multiplier, // Multiplier Schedule
        Difference, // Difference Schedule
        Profile,    // Temperature Profile
        Num
    };

    static constexpr std::array<std::string_view, (int)DesDayDryBulbRangeType::Num> DesDayDryBulbRangeTypeNamesUC = {
        "DEFAULTMULTIPLIERS", "MULTIPLIERSCHEDULE", "DIFFERENCESCHEDULE", "TEMPERATUREPROFILESCHEDULE"};

    enum class SkyTempModel
    {
        Invalid = -1,
        ClarkAllen,    // Use Clark & Allen model for sky emissivity calculation
        ScheduleValue, // User entered Schedule value for Weather Property
        DryBulbDelta,  // User entered DryBulb difference Schedule value for Weather Property
        DewPointDelta, // User entered Dewpoint difference Schedule value for Weather Property
        Brunt,         // Use Brunt model for sky emissivity calculation
        Idso,          // Use Isdo model for sky emissivity calculation
        BerdahlMartin, // Use Martin & Berdahl model for sky emissivity calculation
        Num
    };

    struct EnvironmentData
    {
        // Members
        std::string Title = "";                                         // Environment name
        std::string cKindOfEnvrn = "";                                  // kind of environment
        Constant::KindOfSim KindOfEnvrn = Constant::KindOfSim::Invalid; // Type of environment (see Parameters for KindOfSim in DataGlobals)
        int DesignDayNum = 0;                                           // index in DesignDay structure and DesignDayInput
        int RunPeriodDesignNum = 0;                                     // for WeatherFileDays, index in  RunPeriodDesign and RunPeriodDesignInput
        int SeedEnvrnNum = 0;           // for HVAC sizing sim, new environments are copies of original environments, this is the index for original
        int HVACSizingIterationNum = 0; // environments for HVAC sizing simulations are associated with iteration
        int TotalDays = 0;              // Number of days in environment
        int StartJDay = 0;              // Day of year of first day of environment
        int StartMonth = 0;
        int StartDay = 0;
        int StartYear = 0;
        int StartDate = 0;
        int EndMonth = 0;
        int EndDay = 0;
        int EndJDay = 0;
        int EndYear = 0;
        int EndDate = 0;
        int DayOfWeek = 0;             // Starting Day of Week for the (Weather) RunPeriod (User Input)
        bool UseDST = false;           // True if DaylightSavingTime is used for this RunPeriod
        bool UseHolidays = false;      // True if Holidays are used for this RunPeriod (from WeatherFile)
        bool ApplyWeekendRule = false; // True if "Weekend Rule" is to be applied to RunPeriod
        bool UseRain = true;           // True if Rain from weather file should be used (set rain to true)
        bool UseSnow = true;           // True if Snow from weather file should be used (set Snow to true)
        Array1D_int MonWeekDay = Array1D_int(12, 0);
        bool SetWeekDays = false;                             // true when weekdays will be reset (after first year or on repeat)
        int NumSimYears = 1;                                  // Total Number of times this period to be performed
        int CurrentCycle = 0;                                 // Current cycle through weather file in NumSimYears repeats
        int WP_Type1 = 0;                                     // WeatherProperties SkyTemperature Pointer
        SkyTempModel skyTempModel = SkyTempModel::ClarkAllen; // WeatherProperties SkyTemperature CalculationType
        bool UseWeatherFileHorizontalIR = true;               // If false, horizontal IR and sky temperature are calculated with WP models
        int CurrentYear = 0;                                  // Current year
        bool IsLeapYear = false;                              // True if current year is leap year.
        bool RollDayTypeOnRepeat = true;                      // If repeating run period, increment day type on repeat.
        bool TreatYearsAsConsecutive = true;                  // When year rolls over, increment year and recalculate Leap Year
        bool MatchYear = false;                               // for actual weather will be true
        bool ActualWeather = false;                           // true when using actual weather data
        int RawSimDays = 0;                                   // number of basic sim days.
        bool firstHrInterpUseHr1 = false;                     // true when using Hour 1 for first hour interpolations; false to use Hour 24

        Real64 maxCoolingOATSizing = -1000.0;  // max outdoor dry-bulb for DesignDay or RunPeriodDesign type weather
        Real64 maxCoolingOADPSizing = -1000.0; // outdoor dew point at max outdoor dry-bulb for DesignDay or RunPeriodDesign type weather
        Real64 minHeatingOATSizing = 1000.0;   // min outdoor dry-bulb for DesignDay or RunPeriodDesign type weather
        Real64 minHeatingOADPSizing = 1000.0;  // outdoor dew point at min outdoor dry-bulb for DesignDay or RunPeriodDesign type weather
    };

    struct DesignDayData
    {
        // Members
        std::string Title = "";                                          // Environment name
        Real64 MaxDryBulb = 0.0;                                         // Maximum Dry-Bulb Temperature (C)
        Real64 DailyDBRange = 0.0;                                       // Daily Temperature Range (deltaC)
        Real64 HumIndValue = 0.0;                                        // Humidity Indicating Value at Max Dry-bulb Temperature
        DesDayHumIndType HumIndType = DesDayHumIndType::WetBulb;         // Humidity Indicating type  (see Parameters)
        Real64 PressBarom = 0.0;                                         // Atmospheric/Barometric Pressure (Pascals)
        Real64 WindSpeed = 0.0;                                          // Wind Speed (m/s)
        Real64 WindDir = 0.0;                                            // Wind Direction (degrees clockwise from North, N=0, E=90, S=180, W=270)
        Real64 SkyClear = 0.0;                                           // Sky Clearness (0 to 1)
        int RainInd = 0;                                                 // Rain Indicator (1 = raining and surfaces are wet, else 0)
        int SnowInd = 0;                                                 // Snow Indicator (1 = snow on ground, else  0)
        int DayOfMonth = 0;                                              // Day of Month ( 1 - 31 )
        int Month = 0;                                                   // Month of Year ( 1 - 12 )
        int DayType = 0;                                                 // Day Type Sunday = 1 - Saturday = 7
        int DSTIndicator = 0;                                            // Daylight Saving Time Period Indicator (1=yes, 0=no) for this DesignDay
        DesDaySolarModel solarModel = DesDaySolarModel::ASHRAE_ClearSky; // Solar Model for creating solar values for design day.
        DesDayDryBulbRangeType dryBulbRangeType = DesDayDryBulbRangeType::Default; // Drybulb Range Type (see Parameters)
        Sched::DaySchedule *tempRangeSched = nullptr;                              // day schedule for dry-bulb temperature range multipliers
        Sched::DaySchedule *humIndSched = nullptr; // day schedule that specifies relative humidity (%) or wet-bulb range multipliers per HumIndType
        Sched::DaySchedule *beamSolarSched = nullptr;    // day schedule for beam solar
        Sched::DaySchedule *diffuseSolarSched = nullptr; // day schedule for diffuse solar
        Real64 TauB = 0.0;                               // beam pseudo optical depth for ASHRAE tau model
        Real64 TauD = 0.0;                               // diffuse pseudo optical depth for ASHRAE tau model
        Real64 DailyWBRange = 0.0;                       // daily range of wetbulb (deltaC)
        bool PressureEntered = false;                    // true if a pressure was entered in design day data
        bool DewPointNeedsSet = false;                   // true if the Dewpoint humidicating value needs to be set (after location determined)
        int maxWarmupDays = -1;                          // Maximum warmup days between sizing periods
        bool suppressBegEnvReset = false; // true if this design day should be run without thermal history being reset at begin environment
    };

    struct ReportPeriodData
    {
        std::string title = "";
        std::string reportName = "";
        int startYear = 2017;
        int startMonth = 1;
        int startDay = 1;
        int startHour = 1;
        int startJulianDate = 2457755;
        int endYear = 2017;
        int endMonth = 12;
        int endDay = 31;
        int endHour = 24;
        int endJulianDate = 2458119;
        Real64 totalElectricityUse = 0.0; // What is this doing here?
    };

    struct RunPeriodData
    {
        // Members
        std::string title;
        std::string periodType;
        int totalDays = 365; // total number of days in requested period
        int startMonth = 1;
        int startDay = 1;
        int startJulianDate = 2457755; // Calculated start date (Julian or ordinal) for a weather file run period
        int startYear = 2017;          // entered in "consecutive"/real runperiod object
        int endMonth = 12;
        int endDay = 31;
        int endJulianDate = 2458119;                                   // Calculated end date (Julian or ordinal) for a weather file run period
        int endYear = 2017;                                            // entered in "consecutive"/real runperiod object
        int dayOfWeek = 1;                                             // Day of Week that the RunPeriod will start on (User Input)
        Sched::DayType startWeekDay = Sched::DayType::Sunday;          // Day of the week that the RunPeriod will start on (User Input)
        bool useDST = false;                                           // True if DaylightSavingTime is used for this RunPeriod
        bool useHolidays = false;                                      // True if Holidays are used for this RunPeriod (from WeatherFile)
        bool applyWeekendRule = false;                                 // True if "Weekend Rule" is to be applied to RunPeriod
        bool useRain = true;                                           // True if Rain from weather file should be used (set rain to true)
        bool useSnow = true;                                           // True if Snow from weather file should be used (set Snow to true)
        Array1D_int monWeekDay = {1, 4, 4, 7, 2, 5, 7, 3, 6, 1, 4, 6}; // Weekday for first day of each month
        int numSimYears = 1;                                           // Total Number of years of simulation to be performed
        bool isLeapYear = false;                                       // True if Begin Year is leap year.
        bool RollDayTypeOnRepeat = true;                               // If repeating run period, increment day type on repeat.
        bool TreatYearsAsConsecutive = true;                           // When year rolls over, increment year and recalculate Leap Year
        bool actualWeather = false;                                    // true when using actual weather data
        bool firstHrInterpUsingHr1 = false;                            // true for using Hour 1 for first hour interpolate; false for using Hour 24
    };

    struct DayWeatherVariables // Derived Type for Storing Weather "Header" Data
    {
        // Members
        int DayOfYear = 0;                // Day of year for weather data
        int DayOfYear_Schedule = 0;       // Day of year in schedule
        int Year = 0;                     // Year of weather data
        int Month = 0;                    // Month of weather data
        int DayOfMonth = 0;               // Day of month for weather data
        int DayOfWeek = 0;                // Day of week for weather data
        int DaylightSavingIndex = 0;      // Daylight Saving Time Period indicator (0=no,1=yes)
        int HolidayIndex = 0;             // Holiday indicator (0=no holiday, non-zero=holiday type)
        Real64 SinSolarDeclinAngle = 0.0; // Sine of the solar declination angle
        Real64 CosSolarDeclinAngle = 0.0; // Cosine of the solar declination angle
        Real64 EquationOfTime = 0.0;      // Value of the equation of time formula
    };

    struct SpecialDayData
    {
        // Members
        std::string Name = "";                 // Name
        DateType dateType = DateType::Invalid; // Date type as read in from IDF
        int Month = 0;                         // Start Month
        int Day = 0;                           // Start Day of month or Count for DateTypes=NthDayOfMonth
        int WeekDay = 0;                       // For Date types=NthDayOfMonth and LastDayOfMonth
        int CompDate = 0;                      // Start Date in "compressed date" format, only if Month/Day
        bool WthrFile = false;                 // True if this Special Day came from weather file (EPW)
        int Duration = 0;                      // Number of days this special Day is used for
        int DayType = 0;                       // Day Type desigation for this Special Day period
        int ActStMon = 0;
        int ActStDay = 0;
        bool Used = false; // Set to true in a run period after use (NthDayOfMonth and LastDayOfMonth only)
    };

    struct DataPeriodData
    {
        // Members
        std::string Name = "";      // DataPeriod Title
        std::string DayOfWeek = ""; // Start Day of Week for DataPeriod
        int NumYearsData = 1;       // Number of years for which data is present in EPW.
        int WeekDay = 0;
        int StMon = 0;
        int StDay = 0;
        int StYear = 0;
        int EnMon = 0;
        int EnDay = 0;
        int EnYear = 0;
        int NumDays = 0;
        Array1D_int MonWeekDay = Array1D_int(12, 0);
        int DataStJDay = 0;
        int DataEnJDay = 0;
        bool HasYearData = false;
    };

    struct DSTPeriod
    {
        // Members
        DateType StDateType = DateType::Invalid; // Start Date type as from EPW or IDF
        int StWeekDay = 0;                       // For DateTypes=NthDayOfMonth or LastDayOfMonth
        int StMon = 0;                           // DaylightSavingTime (DST) Start Month
        int StDay = 0;                           // DaylightSavingTime (DST) Start Day
        DateType EnDateType = DateType::Invalid; // End Date type as from EPW or IDF
        int EnMon = 0;                           // DaylightSavingTime (DST) End Month
        int EnDay = 0;                           // DaylightSavingTime (DST) End Day
        int EnWeekDay = 0;                       // For DateTypes=NthDayOfMonth or LastDayOfMonth
    };

    // This Derived type carries the counts of missing data items in the weather reading process. It will count only items that are on the source file
    // -- not those that are derived from data on the source file.
    struct WeatherVarCounts
    {
        // Members
        // Comments below illustrate the data that is being counted:
        int OutDryBulbTemp = 0;  // Dry Bulb Temperature (C)
        int OutDewPointTemp = 0; // Dew Point Temperature (C)
        int OutRelHum = 0;       // Relative Humidity (%)
        int OutBaroPress = 0;    // Atmospheric Pressure (Pa)
        int WindDir = 0;         // Wind Direction (deg)
        int WindSpeed = 0;       // Wind Speed/Velocity (m/s)
        int BeamSolarRad = 0;    // Direct Radiation (wh/m2)
        int DifSolarRad = 0;     // Diffuse Radiation (wh/m2)
        int TotalSkyCover = 0;   // Total Sky Cover (tenths)
        int OpaqueSkyCover = 0;  // Opaque Sky Cover (tenths)
        int Visibility = 0;      // Visibility (km)
        int Ceiling = 0;         // Ceiling Height (m)
        int LiquidPrecip = 0;    // Precipitable Water (mm)
        int WaterPrecip = 0;     // Precipitable Water (mm)
        int AerOptDepth = 0;     // Aerosol Optical Depth
        int SnowDepth = 0;       // Snow Depth (cm)
        int DaysLastSnow = 0;    // Number of Days since last snow
        int WeathCodes = 0;      // Weather codes invalid
        int Albedo = 0;          // Albedo
    };

    struct TypicalExtremeData
    {
        // Members
        std::string Title = "";       // Environment name
        std::string ShortTitle = "";  // Environment name
        std::string MatchValue = "";  // String to be matched for input/running these periods for design.
        std::string MatchValue1 = ""; // String to be also matched (synonym)
        std::string MatchValue2 = ""; // String to be also matched (synonym)
        std::string TEType = "";      // Typical or Extreme
        int TotalDays = 0;            // Number of days in environment
        int StartJDay = 0;            // Day of year of first day of environment
        int StartMonth = 0;
        int StartDay = 0;
        int EndMonth = 0;
        int EndDay = 0;
        int EndJDay = 0;
    };

    struct WeatherProperties
    {
        // Members
        std::string Name = "";  // Reference Name
        bool IsSchedule = true; // Default is using Schedule
        SkyTempModel skyTempModel = SkyTempModel::ClarkAllen;
        Sched::DayOrYearSchedule *sched = nullptr; // schedule when used
        bool UsedForEnvrn = false;
        bool UseWeatherFileHorizontalIR = true; // If false, horizontal IR and sky temperature are calculated with WP models
    };

    struct UnderwaterBoundary
    {
        std::string Name = "";
        Real64 distanceFromLeadingEdge = 0.0;
        int OSCMIndex = 0;
        Sched::Schedule *waterTempSched = nullptr;
        Sched::Schedule *velocitySched = nullptr;
    };

    // Functions
    void ManageWeather(EnergyPlusData &state);

    void ResetEnvironmentCounter(EnergyPlusData &state);

    bool GetNextEnvironment(EnergyPlusData &state, bool &Available, bool &ErrorsFound);

    void AddDesignSetToEnvironmentStruct(
        EnergyPlusData &state, int HVACSizingIterCount // Counter for number of times HVAC Sizing Simulation of Design Period set is being rerun
    );

    bool CheckIfAnyUnderwaterBoundaries(EnergyPlusData &state);

    Real64 calculateWaterBoundaryConvectionCoefficient(Real64 curWaterTemp, Real64 curWaterVelocity, Real64 distanceFromLeadingEdge);

    void UpdateUnderwaterBoundaries(EnergyPlusData &state);

    void ReadVariableLocationOrientation(EnergyPlusData &state);

    void UpdateLocationAndOrientation(EnergyPlusData &state);

    void SetupWeekDaysByMonth(EnergyPlusData &state, int StMon, int StDay, int StWeekDay, Array1D_int &WeekDays);

    void ResetWeekDaysByMonth(EnergyPlusData &state,
                              Array1D_int &WeekDays,
                              int AddLeapYear,
                              int StartMonth,
                              int StartMonthDay,
                              int EndMonth,
                              int EndMonthDay,
                              bool Rollover,
                              bool MidSimReset = false);

    void SetDSTDateRanges(EnergyPlusData &state,
                          Array1D_int const &MonWeekDay, // Weekday of each day 1 of month
                          Array1D_int &DSTIdx,           // DST Index for each julian day (1:366)
                          ObjexxFCL::Optional_int DSTActStMon = _,
                          ObjexxFCL::Optional_int DSTActStDay = _,
                          ObjexxFCL::Optional_int DSTActEnMon = _,
                          ObjexxFCL::Optional_int DSTActEnDay = _);

    void SetSpecialDayDates(EnergyPlusData &state, Array1D_int const &MonWeekDay); // Weekday of each day 1 of month

    void InitializeWeather(EnergyPlusData &state, bool &printEnvrnStamp); // Set to true when the environment header should be printed

    void UpdateWeatherData(EnergyPlusData &state);

    void SetCurrentWeather(EnergyPlusData &state);

    void ReadWeatherForDay(EnergyPlusData &state,
                           int DayToRead,          // =1 when starting out, otherwise signifies next day
                           int Environ,            // Environment being simulated
                           bool BackSpaceAfterRead // True if weather file is to be backspaced after read
    );

    void ReadEPlusWeatherForDay(EnergyPlusData &state,
                                int DayToRead,          // =1 when starting out, otherwise signifies next day
                                int Environ,            // Environment being simulated
                                bool BackSpaceAfterRead // True if weather file is to be backspaced after read
    );

    Real64 interpolateWindDirection(Real64 prevHrWindDir, Real64 curHrWindDir, Real64 curHrWeight);

    void SetDayOfWeekInitialValues(int EnvironDayOfWeek, // Starting Day of Week for the (Weather) RunPeriod (User Input)
                                   int &currentDayOfWeek // Current Day of Week
    );

    void ErrorInterpretWeatherDataLine(
        EnergyPlusData &state, int WYear, int WMonth, int WDay, int WHour, int WMinute, std::string_view SaveLine, std::string_view Line);

    void InterpretWeatherDataLine(EnergyPlusData &state,
                                  std::string_view Line,
                                  bool &ErrorFound, // True if an error is found, false otherwise
                                  int &WYear,
                                  int &WMonth,
                                  int &WDay,
                                  int &WHour,
                                  int &WMinute,
                                  Real64 &DryBulb,           // DryBulb
                                  Real64 &DewPoint,          // DewPoint
                                  Real64 &RelHum,            // RelHum
                                  Real64 &AtmPress,          // AtmPress
                                  Real64 &ETHoriz,           // ETHoriz
                                  Real64 &ETDirect,          // ETDirect
                                  Real64 &IRHoriz,           // IRHoriz
                                  Real64 &GLBHoriz,          // GLBHoriz
                                  Real64 &DirectRad,         // DirectRad
                                  Real64 &DiffuseRad,        // DiffuseRad
                                  Real64 &GLBHorizIllum,     // GLBHorizIllum
                                  Real64 &DirectNrmIllum,    // DirectNrmIllum
                                  Real64 &DiffuseHorizIllum, // DiffuseHorizIllum
                                  Real64 &ZenLum,            // ZenLum
                                  Real64 &WindDir,           // WindDir
                                  Real64 &WindSpeed,         // WindSpeed
                                  Real64 &TotalSkyCover,     // TotalSkyCover
                                  Real64 &OpaqueSkyCover,    // OpaqueSkyCover
                                  Real64 &Visibility,        // Visibility
                                  Real64 &CeilHeight,        // CeilHeight
                                  int &WObs,                 // PresWeathObs
                                  Array1D_int &WCodesArr,    // PresWeathConds
                                  Real64 &PrecipWater,       // PrecipWater
                                  Real64 &AerosolOptDepth,   // AerosolOptDepth
                                  Real64 &SnowDepth,         // SnowDepth
                                  Real64 &DaysSinceLastSnow, // DaysSinceLastSnow
                                  Real64 &Albedo,            // Albedo
                                  Real64 &LiquidPrecip       // LiquidPrecip
    );

    void SetUpDesignDay(EnergyPlusData &state, int EnvrnNum); // Environment number passed into the routine

    Real64 AirMass(Real64 CosZen); // COS( solar zenith), 0 - 1

    // Calculate sky temperature from weather data
    Real64 CalcSkyEmissivity(EnergyPlusData &state, SkyTempModel skyTempModel, Real64 OSky, Real64 DryBulb, Real64 DewPoint, Real64 RelHum);

    void ASHRAETauModel([[maybe_unused]] EnergyPlusData &state,
                        DesDaySolarModel TauModel, // ASHRAETau solar model type ASHRAE_Tau or ASHRAE_Tau2017
                        Real64 ETR,                // extraterrestrial normal irradiance, W/m2
                        Real64 CosZen,             // COS( solar zenith angle), 0 - 1
                        Real64 TauB,               // beam tau factor
                        Real64 TauD,               // dif tau factor
                        Real64 &IDirN,             // returned: direct (beam) irradiance on normal surface, W/m2
                        Real64 &IDifH,             // returned: diffuse irradiance on horiz surface, W/m2
                        Real64 &IGlbH              // returned: global irradiance on horiz surface, W/m2
    );

    void AllocateWeatherData(EnergyPlusData &state);

    void CalculateDailySolarCoeffs(EnergyPlusData const &state,
                                   int DayOfYear,                 // Day of year (1 - 366)
                                   Real64 &A,                     // ASHRAE "A" - Apparent solar irradiation at air mass = 0 [W/M**2]
                                   Real64 &B,                     // ASHRAE "B" - Atmospheric extinction coefficient
                                   Real64 &C,                     // ASHRAE "C" - Diffuse radiation factor
                                   Real64 &AnnVarSolConstant,     // Annual variation in the solar constant
                                   Real64 &EquationOfTime,        // Equation of Time
                                   Real64 &SineSolarDeclination,  // Sine of Solar Declination
                                   Real64 &CosineSolarDeclination // Cosine of Solar Declination
    );

    void CalculateSunDirectionCosines(EnergyPlusData const &state,
                                      Real64 TimeValue,    // Current Time of Day
                                      Real64 EqOfTime,     // Equation of Time
                                      Real64 SinSolDeclin, // Sine of Solar Declination
                                      Real64 CosSolDeclin, // Cosine of Solar Declination
                                      Vector3<Real64> &SUNCOS);

    void DetermineSunUpDown(EnergyPlusData &state, Vector3<Real64> &SUNCOS);

    void OpenWeatherFile(EnergyPlusData &state, bool &ErrorsFound);

    void OpenEPlusWeatherFile(EnergyPlusData &state,
                              bool &ErrorsFound, // Will be set to true if errors found
                              bool ProcessHeader // Set to true when headers should be processed (rather than just read)
    );

    void CloseWeatherFile(EnergyPlusData &state);

    void ResolveLocationInformation(EnergyPlusData &state, bool &ErrorsFound); // Set to true if no location evident

    void CheckLocationValidity(EnergyPlusData &state);

    void CheckWeatherFileValidity(EnergyPlusData &state);

    void ReportOutputFileHeaders(EnergyPlusData &state);

    void ReportWeatherAndTimeInformation(EnergyPlusData &state,
                                         bool &printEnvrnStamp); // Set to true when the environment header should be printed

    void ReadUserWeatherInput(EnergyPlusData &state);

    void GroupReportPeriodByType(EnergyPlusData &state, const int nReportPeriods);

    void GetReportPeriodData(EnergyPlusData &state,
                             int nReportPeriods, // Total number of Report Periods requested
                             bool &ErrorsFound);

    void GetRunPeriodData(EnergyPlusData &state,
                          int nRunPeriods, // Total number of Run Periods requested
                          bool &ErrorsFound);

    void GetRunPeriodDesignData(EnergyPlusData &state, bool &ErrorsFound);

    void GetSpecialDayPeriodData(EnergyPlusData &state, bool &ErrorsFound); // will be set to true if severe errors are found in inputs

    void CalcSpecialDayTypes(EnergyPlusData &state);

    void GetDSTData(EnergyPlusData &state, bool &ErrorsFound); // will be set to true if severe errors are found in inputs

    void GetDesignDayData(EnergyPlusData &state,
                          int TotDesDays, // Total number of Design days to Setup
                          bool &ErrorsFound);

    void GetLocationInfo(EnergyPlusData &state, bool &ErrorsFound);

    void GetWeatherProperties(EnergyPlusData &state, bool &ErrorsFound);

    void GetGroundTemps(EnergyPlusData &state);

    void GetGroundReflectances(EnergyPlusData &state, bool &ErrorsFound);

    void GetSnowGroundRefModifiers(EnergyPlusData &state, bool &ErrorsFound);

    void GetWaterMainsTemperatures(EnergyPlusData &state, bool &ErrorsFound);

    void CalcWaterMainsTemp(EnergyPlusData &state);

    Real64 WaterMainsTempFromCorrelation(EnergyPlusData const &state,
                                         Real64 AnnualOAAvgDryBulbTemp,         // annual average OA drybulb temperature
                                         Real64 MonthlyOAAvgDryBulbTempMaxDiff, // monthly daily average OA drybulb temperature maximum difference
                                         Real64 TemperatureMultiplier,          // temperature multiplier
                                         Real64 TemperatureOffset               // temperature offset
    );

    void GetWeatherStation(EnergyPlusData &state, bool &ErrorsFound);

    void DayltgCurrentExtHorizIllum(EnergyPlusData &state);

    void DayltgLuminousEfficacy(EnergyPlusData &state,
                                Real64 &DiffLumEff, // Luminous efficacy of sky diffuse solar radiation (lum/W)
                                Real64 &DirLumEff   // Luminous efficacy of beam solar radiation (lum/W)
    );

    Real64 GetSTM(Real64 Longitude); // Longitude from user input

    void ProcessEPWHeader(EnergyPlusData &state, EpwHeaderType const headerType, std::string &Line, bool &ErrorsFound);

    void SkipEPlusWFHeader(EnergyPlusData &state);

    void ReportMissing_RangeData(EnergyPlusData &state);

    void SetupInterpolationValues(EnergyPlusData &state);

    void SetupEnvironmentTypes(EnergyPlusData &state);

    bool isLeapYear(int Year);

    struct GregorianDate
    {
        int year = 0;
        int month = 0;
        int day = 0;
    };

    int computeJulianDate(int gyyyy, int gmm, int gdd);

    int computeJulianDate(GregorianDate const &gdate);

    GregorianDate computeGregorianDate(int jdate);

    Sched::DayType calculateDayOfWeek(EnergyPlusData &state, int year, int month, int day);

    int calculateDayOfYear(int Month, int Day, bool leapYear = false);

    bool validMonthDay(int month, int day, int leapYearAdd = 0);

    // derived type for processing and storing Dry-bulb weather or stat file
    struct AnnualMonthlyDryBulbWeatherData
    {
        // Members
        bool OADryBulbWeatherDataProcessed = false;  // if false stat or weather file OA Dry-bulb temp is not processed yet
        Real64 AnnualAvgOADryBulbTemp = 0.0;         // annual average outdoor air temperature (C)
        Real64 MonthlyAvgOADryBulbTempMaxDiff = 0.0; // monthly daily average OA drybulb temperature maximum difference (deltaC)
        Array1D<Real64> MonthlyDailyAverageDryBulbTemp = Array1D<Real64>(12, 0.0); // monthly-daily average outdoor air temperatures (C)

        void CalcAnnualAndMonthlyDryBulbTemp(EnergyPlusData &state); // true if this is CorrelationFromWeatherFile
    };

    void ReportWaterMainsTempParameters(EnergyPlusData &state);
    void calcSky(EnergyPlusData &state,
                 Real64 &TmrHorizIRSky,
                 Real64 &TmrSkyTemp,
                 Real64 OpaqueSkyCover,
                 Real64 DryBulb,
                 Real64 DewPoint,
                 Real64 RelHum,
                 Real64 IRHoriz);

    struct WeatherVars
    {
        bool IsRain = false;
        bool IsSnow = false;
        Real64 OutDryBulbTemp = 0.0;
        Real64 OutDewPointTemp = 0.0;
        Real64 OutBaroPress = 0.0;
        Real64 OutRelHum = 0.0;
        Real64 WindSpeed = 0.0;
        Real64 WindDir = 0.0;
        Real64 SkyTemp = 0.0;
        Real64 HorizIRSky = 0.0;
        Real64 BeamSolarRad = 0.0;
        Real64 DifSolarRad = 0.0;
        Real64 Albedo = 0.0;
        Real64 WaterPrecip = 0.0; // What is the difference between WaterPrecip and LiquidPrecip?
        Real64 LiquidPrecip = 0.0;
        Real64 TotalSkyCover = 0.0;
        Real64 OpaqueSkyCover = 0.0;
    };

    // This struct carries the default missing data for those data elements that would be best replaced with the previous hour's data for missing
    // data.
    struct ExtWeatherVars : public WeatherVars
    {
        // Members
        Real64 Visibility = 0.0;  // Visibility (km)
        Real64 Ceiling = 0.0;     // Ceiling Height (m)
        Real64 AerOptDepth = 0.0; // Aerosol Optical Depth
        Real64 SnowDepth = 0.0;   // Snow Depth (cm)
        int DaysLastSnow = 0;     // Number of Days since last snow
    };

    struct DesDayMods
    {
        Real64 OutDryBulbTemp = 0.0;
        Real64 OutRelHum = 0.0;
        Real64 BeamSolarRad = 0.0;
        Real64 DifSolarRad = 0.0;
        Real64 SkyTemp = 0.0;
    };

    struct SPSiteSchedules
    {
        Real64 OutDryBulbTemp = 0.0;
        Real64 OutRelHum = 0.0;
        Real64 BeamSolarRad = 0.0;
        Real64 DifSolarRad = 0.0;
        Real64 SkyTemp = 0.0;
    };

} // namespace Weather

struct WeatherManagerData : BaseGlobalStruct
{

    // These were static variables within different functions. They were pulled out into the namespace
    // to facilitate easier unit testing of those functions.
    // These are purposefully not in the header file as an extern variable. No one outside of this should
    // use these. They are cleared by clear_state() for use by unit tests, but normal simulations should be unaffected.
    // This is purposefully in an anonymous namespace so nothing outside this implementation file can use it.
    bool GetBranchInputOneTimeFlag = true;
    bool GetEnvironmentFirstCall = true;
    bool PrntEnvHeaders = true;
    bool FirstCall = true;                 // Some things should only be done once
    bool WaterMainsParameterReport = true; // should only be done once
    bool PrintEnvrnStamp = false;          // Set to true when the environment header should be printed
    bool PrintDDHeader = true;

    int YearOfSim = 1;                           // The Present year of Simulation.
    int NumDaysInYear = 365;                     // TODO: removed const from this until leap year behavior is reviewed
    int EnvironmentReportNbr = 0;                // Report number for the environment stamp
    std::string EnvironmentReportChr = "";       // Report number for the environment stamp (character -- for printing)
    bool WeatherFileExists = false;              // Set to true if a weather file exists
    std::string LocationTitle = "";              // Location Title from input File
    bool LocationGathered = false;               // flag to show if Location exists on Input File (we assume one is there and correct on weather file)
    bool keepUserSiteLocationDefinition = false; // flag based on user input to set whether to keep the user site location definition (true)
                                                 // or override with the site information given on weather file (false/default)

    Real64 WeatherFileLatitude = 0.0;
    Real64 WeatherFileLongitude = 0.0;
    Real64 WeatherFileTimeZone = 0.0;
    Real64 WeatherFileElevation = 0.0;
    Array1D<Real64> GroundTempsFCFromEPWHeader = Array1D<Real64>(12, 0.0); // F or C factor method NOLINT(cert-err58-cpp)
    Array1D<Real64> GroundReflectances =
        Array1D<Real64>(12, 0.2);             // User Specified Ground Reflectances !EPTeam: Using DP causes big diffs NOLINT(cert-err58-cpp)
    Real64 SnowGndRefModifier = 1.0;          // Modifier to ground reflectance during snow
    Real64 SnowGndRefModifierForDayltg = 1.0; // Modifier to ground reflectance during snow for daylighting
    Weather::WaterMainsTempCalcMethod WaterMainsTempsMethod =
        Weather::WaterMainsTempCalcMethod::FixedDefault; // Water mains temperature calculation method
    Sched::Schedule *waterMainsTempSched = nullptr;      // Water mains temperature schedule
    Real64 WaterMainsTempsAnnualAvgAirTemp = 0.0;        // Annual average outdoor air temperature (C)
    Real64 WaterMainsTempsMaxDiffAirTemp = 0.0;          // Maximum difference in monthly average outdoor air temperatures (deltaC)
    Real64 WaterMainsTempsMultiplier = 1.0;              // Temperature multiplier
    Real64 WaterMainsTempsOffset = 0.0;                  // Temperature offset (C)
    bool wthFCGroundTemps = false;

    int TotRunPers = 0;           // Total number of Run Periods (Weather data) to Setup
    int TotRunDesPers = 0;        // Total number of Run Design Periods (Weather data) to Setup
    int TotReportPers = 0;        // Total number of reporting periods
    int TotThermalReportPers = 0; // Total number of thermal reporting periods
    int TotCO2ReportPers = 0;     // Total number of CO2 reporting periods
    int TotVisualReportPers = 0;  // Total number of visual reporting periods

    int NumSpecialDays = 0;
    Array1D_int SpecialDayTypes = Array1D<int>(366, 0); // To hold holiday types given in input file NOLINT(cert-err58-cpp)
    Array1D_int WeekDayTypes = Array1D<int>(366, 0);    // To hold Week day types using specified first day NOLINT(cert-err58-cpp)
    Array1D_int DSTIndex = Array1D<int>(366, 0);        // To hold DST Index based on weather file or input NOLINT(cert-err58-cpp)

    int NumDataPeriods = 0;

    int NumIntervalsPerHour = 1;

    bool UseDaylightSaving = true;       // True if user says to use Weather File specified DaylightSaving Period
    bool UseSpecialDays = true;          // True if user says to use Weather File specified Special Days for current RunPeriod
    bool UseRainValues = true;           // True if rain values from weather file are to be used
    bool UseSnowValues = true;           // True if snow values from weather file are to be used
    bool EPWDaylightSaving = false;      // True if a DaylightSaving Time Period is input (EPW files)
    bool IDFDaylightSaving = false;      // True if a DaylightSaving Time Period is input (IDF files)
    bool DaylightSavingIsActive = false; // True if a DaylightSavingPeriod should be used for Environment
    bool WFAllowsLeapYears = false;      // True if the Weather File (WF) header has "Yes" for Leap Years
    int curSimDayForEndOfRunPeriod = 0;  // normal=number days in sim, but different when repeating runperiods or multi-year files
    int Envrn = 0;                       // Counter for environments
    int NumOfEnvrn = 0;                  // Number of environments to be simulated
    int NumEPWTypExtSets = 0;            // Number of Typical/Extreme on weather file.
    int NumWPSkyTemperatures = 0;        // Number of WeatherProperty:SkyTemperature items in input file

    Array2D<Weather::WeatherVars> wvarsHrTsToday;
    Array2D<Weather::WeatherVars> wvarsHrTsTomorrow;

    EPVector<Array2D<Weather::DesDayMods>> desDayMods;

    int RptIsRain = 0;  // Rain Report Value
    int RptIsSnow = 0;  // Snow Report Value
    int RptDayType = 0; // DayType Report Value

    Real64 HrAngle = 0.0;            // Current Hour Angle
    Real64 SolarAltitudeAngle = 0.0; // Angle of Solar Altitude (degrees)
    Real64 SolarAzimuthAngle = 0.0;  // Angle of Solar Azimuth (degrees)
    Real64 HorizIRSky = 0.0;         // Horizontal Infrared Radiation Intensity (W/m2)
    Real64 TimeStepFraction = 0.0;   // Fraction of hour each time step represents

    EPVector<Weather::SPSiteSchedules> spSiteSchedules;
    std::vector<int> spSiteSchedNums;

    // Number of hours of missing data
    Array1D<Real64> Interpolation;      // Interpolation values based on Number of Time Steps in Hour NOLINT(cert-err58-cpp)
    Array1D<Real64> SolarInterpolation; // Solar Interpolation values based on Number of Time Steps in Hour NOLINT(cert-err58-cpp)
    Array1D_int EndDayOfMonth = Array1D_int(12, {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}); // NOLINT(cert-err58-cpp)
    Array1D_int EndDayOfMonthWithLeapDay =
        Array1D_int(12, {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}); // end day of the month including Feb 29 for leap years instead of Feb 28
    int LeapYearAdd = 0;                       // Set during environment if leap year is active (adds 1 to number days in Feb)
    bool DatesShouldBeReset = false;           // True when weekdays should be reset
    bool StartDatesCycleShouldBeReset = false; // True when start dates on repeat should be reset
    bool Jan1DatesShouldBeReset = false;       // True if Jan 1 should signal reset of dates
    bool RPReadAllWeatherData = false;         // True if need to read all weather data prior to simulation

    // Object Data
    // NOLINTNEXTLINE(cert-err58-cpp)
    Weather::DayWeatherVariables TodayVariables; // Today's daily weather variables | Derived Type for Storing Weather "Header" Data | Day of year for
                                                 // weather data | Year of weather data | Month of weather data | Day of month for weather data | Day
                                                 // of week for weather data | Daylight Saving Time Period indicator (0=no,1=yes) | Holiday indicator
                                                 // (0=no holiday, non-zero=holiday type) | Sine of the solar declination angle | Cosine of the solar
                                                 // declination angle | Value of the equation of time formula
    // NOLINTNEXTLINE(cert-err58-cpp)
    Weather::DayWeatherVariables TomorrowVariables; // Tomorrow's daily weather variables | Derived Type for Storing Weather "Header" Data | Day of
                                                    // year for weather data | Year of weather data | Month of weather data | Day of month for weather
                                                    // data | Day of week for weather data | Daylight Saving Time Period indicator (0=no,1=yes) |
                                                    // Holiday indicator (0=no holiday, non-zero=holiday type) | Sine of the solar declination angle |
                                                    // Cosine of the solar declination angle | Value of the equation of time formula
    // NOLINTNEXTLINE(cert-err58-cpp)
    EPVector<Weather::DayWeatherVariables> DesignDay; // Design day environments
    // NOLINTNEXTLINE(cert-err58-cpp)
    Weather::ExtWeatherVars wvarsMissing; // Dry Bulb Temperature (C) | Dew Point Temperature (C) | Relative Humidity (%) | Atmospheric Pressure (Pa)
                                          // | Wind Direction (deg) | Wind Speed/Velocity (m/s) | Total Sky Cover (tenths) | Opaque Sky Cover (tenths)
                                          // | Visibility (km) | Ceiling Height (m) | Precipitable Water (mm) | Aerosol Optical Depth | Snow Depth
                                          // (cm) | Number of Days since last snow | Albedo | Rain/Liquid Precipitation (mm)
    Weather::WeatherVarCounts wvarsMissedCounts;           // NOLINT(cert-err58-cpp)
    Weather::WeatherVarCounts wvarsOutOfRangeCounts;       // NOLINT(cert-err58-cpp)
    EPVector<Weather::DesignDayData> DesDayInput;          // Design day Input Data NOLINT(cert-err58-cpp)
    Array1D<Weather::EnvironmentData> Environment;         // Environment data NOLINT(cert-err58-cpp)
    Array1D<Weather::RunPeriodData> RunPeriodInput;        // NOLINT(cert-err58-cpp)
    EPVector<Weather::RunPeriodData> RunPeriodDesignInput; // NOLINT(cert-err58-cpp)
    Array1D<Weather::ReportPeriodData> ReportPeriodInput;
    Array1D<Weather::ReportPeriodData> ThermalReportPeriodInput;
    Array1D<Weather::ReportPeriodData> CO2ReportPeriodInput;
    Array1D<Weather::ReportPeriodData> VisualReportPeriodInput;
    EPVector<Weather::TypicalExtremeData> TypicalExtremePeriods; // NOLINT(cert-err58-cpp)
    Weather::DSTPeriod EPWDST;                                   // Daylight Saving Period Data from EPW file NOLINT(cert-err58-cpp)
    Weather::DSTPeriod IDFDST;                                   // Daylight Saving Period Data from IDF file NOLINT(cert-err58-cpp)
    Weather::DSTPeriod DST;                                      // Daylight Saving Period Data, if active NOLINT(cert-err58-cpp)
    EPVector<Weather::WeatherProperties> WPSkyTemperature;       // NOLINT(cert-err58-cpp)
    EPVector<Weather::SpecialDayData> SpecialDays;               // NOLINT(cert-err58-cpp)
    EPVector<Weather::DataPeriodData> DataPeriods;               // NOLINT(cert-err58-cpp)

    GroundTemp::BaseGroundTempsModel *siteShallowGroundTempsPtr;         // non-owning pointer
    GroundTemp::BaseGroundTempsModel *siteBuildingSurfaceGroundTempsPtr; // non-owning pointer
    GroundTemp::BaseGroundTempsModel *siteFCFactorMethodGroundTempsPtr;  // non-owning pointer
    GroundTemp::BaseGroundTempsModel *siteDeepGroundTempsPtr;            // non-owning pointer

    std::vector<Weather::UnderwaterBoundary> underwaterBoundaries;
    Weather::AnnualMonthlyDryBulbWeatherData OADryBulbAverage; // processes outside air drybulb temperature

    // SetCurrentWeather static vars
    int NextHour = 1;

    // ReadEPlusWeatherForDay static vars
    int CurDayOfWeek = 1;
    Real64 ReadEPlusWeatherCurTime = 1.0;
    bool LastHourSet = false;

    Weather::WeatherVars wvarsLastHr;
    Weather::WeatherVars wvarsNextHr;

    Real64 IsRainThreshold = 0.8; // precipitation threshold (mm) for timestep IsRain (divided later by NumOfTimeStepInHour)

    // ProcessEPWHeader static vars
    std::string EPWHeaderTitle = "";

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) WeatherManagerData();
    }
};
} // namespace EnergyPlus

#endif
