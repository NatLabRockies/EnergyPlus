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

#ifndef OutputProcessor_hh_INCLUDED
#define OutputProcessor_hh_INCLUDED

// C++ Headers
#include <array>
#include <iosfwd>
#include <map>
#include <type_traits>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DisplayRoutines.hh>
#include <EnergyPlus/EPVector.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/ScheduleManager.hh>

// Third party Headers
#include "re2/re2.h"

namespace EnergyPlus {

// Forward declarations
class InputOutputFile;
struct EnergyPlusData;

namespace OutputProcessor {

    enum class ReportVDD
    {
        Invalid = -1,
        No,  // Don't report the variable dictionaries in any form
        Yes, // Report the variable dictionaries in "report format"
        IDF, // Report the variable dictionaries in "IDF format"
        Num
    };

    constexpr Real64 MinSetValue(99999999999999.0);
    constexpr Real64 MaxSetValue(-99999999999999.0);
    constexpr int IMinSetValue(999999);
    constexpr int IMaxSetValue(-999999);

    enum class VariableType
    {
        Invalid = -1,
        // NotFound, // ref: GetVariableKeyCountandType, 0 = not found // TODO: This is actually used separately from Invalid, need to get rid of it
        Integer,  // ref: GetVariableKeyCountandType, 1 = integer
        Real,     // ref: GetVariableKeyCountandType, 2 = real
        Meter,    // ref: GetVariableKeyCountandType, 3 = meter
        Schedule, // ref: GetVariableKeyCountandType, 4 = schedule
        Num
    };

    enum class MeterType
    {
        Invalid = -1,
        Normal,     // Type value for normal meters
        Custom,     // Type value for custom meters
        CustomDec,  // Type value for custom meters that decrement another meter
        CustomDiff, // Type value for custom meters that difference another meter
        Num
    };

    constexpr int N_WriteTimeStampFormatData(100);

    //  For IP Units (tabular reports) certain resources will be put in sub-tables
    enum class RT_IPUnits
    {
        Invalid = -1,
        OtherJ,
        Electricity,
        Gas,
        Cooling,
        Water,
        OtherKG,
        OtherM3,
        OtherL,
        Num
    };

    enum class ReportFreq
    {
        Invalid = -1,
        EachCall,   // Write out each time UpdatedataandReport is called
        TimeStep,   // Write out at 'EndTimeStepFlag'
        Hour,       // Write out at 'EndHourFlag'
        Day,        // Write out at 'EndDayFlag'
        Month,      // Write out at end of month (must be determined)
        Simulation, // Write out once per environment 'EndEnvrnFlag'
        Year,       // Write out at 'EndYearFlag'
        Num
    };

    static constexpr std::array<std::string_view, (int)ReportFreq::Num> reportFreqNames = {
        "Each Call", // EachCall
        "TimeStep",  // TimeStep
        "Hourly",    // Hourly
        "Daily",     // Daily
        "Monthly",   // Monthly
        "RunPeriod", // Simulation
        "Annual"     // Yearly
    };

    static constexpr std::array<std::string_view, (int)ReportFreq::Num> reportFreqNamesUC = {
        "EACH CALL", // EachCall
        "TIMESTEP",  // TimeStep
        "HOURLY",    // Hourly
        "DAILY",     // Daily
        "MONTHLY",   // Monthly
        "RUNPERIOD", // Simulation
        "ANNUAL"     // Yearly
    };

    // What is this thing?
    constexpr std::array<int, (int)ReportFreq::Num> reportFreqArbitraryInts = {1, 1, 1, 7, 9, 11, 11};

    enum class StoreType
    {
        Invalid = -1,
        Average, // Type value for "averaged" variables // TODO: is this just for printing annual tables
        Sum,     // Type value for "summed" variables
        Num
    };

    constexpr std::array<std::string_view, (int)StoreType::Num> storeTypeNames = {
        //        "UNUSED",
        "Average", // Averaged
        "Sum"      // Summed
    };

    enum class TimeStepType
    {
        Invalid = -1,
        Zone,   // Type value for "zone" timestep variables // TODO: is this just for printing Annual tables?
        System, // Type value for "system" timestep variables
        Num
    };

    constexpr std::array<std::string_view, (int)TimeStepType::Num> timeStepTypeNames = {
        //        "UNUSED",
        "Zone",  // Zone
        "System" // System
    };

    enum class EndUseCat
    {
        Invalid = -1,
        Heating,
        Cooling,
        InteriorLights,
        ExteriorLights,
        InteriorEquipment,
        ExteriorEquipment,
        Fans,
        Pumps,
        HeatRejection,
        Humidification,
        HeatRecovery,
        WaterSystem,
        Refrigeration,
        Cogeneration,
        Baseboard,
        Boilers,
        CarbonEquivalentEmissions,
        Chillers,
        CoalEmissions,
        ColdStorageCharge,
        ColdStorageDischarge,
        Condensate,
        CoolingCoils,
        CoolingPanel,
        DieselEmissions,
        DistrictChilledWater,
        DistrictHotWater,
        ElectricityEmissions,
        ElectricStorage,
        FreeCooling,
        FuelOilNo1Emissions,
        FuelOilNo2Emissions,
        GasolineEmissions,
        HeatingCoils,
        HeatProduced,
        HeatRecoveryForCooling,
        HeatRecoveryForHeating,
        LoopToLoop,
        MainsWater,
        NaturalGasEmissions,
        OtherFuel1Emissions,
        OtherFuel2Emissions,
        Photovoltaic,
        PowerConversion,
        PropaneEmissions,
        PurchasedElectricityEmissions,
        RainWater,
        SoldElectricityEmissions,
        WellWater,
        WindTurbine,
        Num
    };

    static constexpr std::array<std::string_view, (int)EndUseCat::Num> endUseCatNames = {"Heating",
                                                                                         "Cooling",
                                                                                         "InteriorLights",
                                                                                         "ExteriorLights",
                                                                                         "InteriorEquipment",
                                                                                         "ExteriorEquipment",
                                                                                         "Fans",
                                                                                         "Pumps",
                                                                                         "HeatRejection",
                                                                                         "Humidifier",
                                                                                         "HeatRecovery",
                                                                                         "WaterSystems",
                                                                                         "Refrigeration",
                                                                                         "Cogeneration",
                                                                                         "Baseboard",
                                                                                         "Boilers",
                                                                                         "CarbonEquivalentEmissions",
                                                                                         "Chillers",
                                                                                         "CoalEmissions",
                                                                                         "ColdStorageCharge",
                                                                                         "ColdStorageDischarge",
                                                                                         "Condensate",
                                                                                         "CoolingCoils",
                                                                                         "CoolingPanel",
                                                                                         "DieselEmissions",
                                                                                         "DistrictChilledWater",
                                                                                         "DistrictHotWater",
                                                                                         "ElectricityEmissions",
                                                                                         "ElectricStorage",
                                                                                         "FreeCooling",
                                                                                         "FuelOilNo1Emissions",
                                                                                         "FuelOilNo2Emissions",
                                                                                         "GasolineEmissions",
                                                                                         "HeatingCoils",
                                                                                         "HeatProduced",
                                                                                         "HeatRecoveryForCooling",
                                                                                         "HeatRecoveryForHeating",
                                                                                         "LoopToLoop",
                                                                                         "MainsWater",
                                                                                         "NaturalGasEmissions",
                                                                                         "OtherFuel1Emissions",
                                                                                         "OtherFuel2Emissions",
                                                                                         "Photovoltaic",
                                                                                         "PowerConversion",
                                                                                         "PropaneEmissions",
                                                                                         "PurchasedElectricityEmissions",
                                                                                         "RainWater",
                                                                                         "SoldElectricityEmissions",
                                                                                         "WellWater",
                                                                                         "WindTurbine"};

    static constexpr std::array<std::string_view, (int)EndUseCat::Num> endUseCatNamesUC = {"HEATING",
                                                                                           "COOLING",
                                                                                           "INTERIORLIGHTS",
                                                                                           "EXTERIORLIGHTS",
                                                                                           "INTERIOREQUIPMENT",
                                                                                           "EXTERIOREQUIPMENT",
                                                                                           "FANS",
                                                                                           "PUMPS",
                                                                                           "HEATREJECTION",
                                                                                           "HUMIDIFIER",
                                                                                           "HEATRECOVERY",
                                                                                           "WATERSYSTEMS",
                                                                                           "REFRIGERATION",
                                                                                           "COGENERATION",
                                                                                           "BASEBOARD",
                                                                                           "BOILERS",
                                                                                           "CARBONEQUIVALENTEMISSIONS",
                                                                                           "CHILLERS",
                                                                                           "COALEMISSIONS",
                                                                                           "COLDSTORAGECHARGE",
                                                                                           "COLDSTORAGEDISCHARGE",
                                                                                           "CONDENSATE",
                                                                                           "COOLINGCOILS",
                                                                                           "COOLINGPANEL",
                                                                                           "DIESELEMISSIONS",
                                                                                           "DISTRICTCHILLEDWATER",
                                                                                           "DISTRICTHOTWATER",
                                                                                           "ELECTRICITYEMISSIONS",
                                                                                           "ELECTRICSTORAGE",
                                                                                           "FREECOOLING",
                                                                                           "FUELOILNO1EMISSIONS",
                                                                                           "FUELOILNO2EMISSIONS",
                                                                                           "GASOLINEEMISSIONS",
                                                                                           "HEATINGCOILS",
                                                                                           "HEATPRODUCED",
                                                                                           "HEATRECOVERYFORCOOLING",
                                                                                           "HEATRECOVERYFORHEATING",
                                                                                           "LOOPTOLOOP",
                                                                                           "MAINSWATER",
                                                                                           "NATURALGASEMISSIONS",
                                                                                           "OTHERFUEL1EMISSIONS",
                                                                                           "OTHERFUEL2EMISSIONS",
                                                                                           "PHOTOVOLTAIC",
                                                                                           "POWERCONVERSION",
                                                                                           "PROPANEEMISSIONS",
                                                                                           "PURCHASEDELECTRICITYEMISSIONS",
                                                                                           "RAINWATER",
                                                                                           "SOLDELECTRICITYEMISSIONS",
                                                                                           "WELLWATER",
                                                                                           "WINDTURBINE"};

    static constexpr std::array<Constant::EndUse, (int)EndUseCat::Num> endUseCat2endUse = {
        Constant::EndUse::Heating,           // Heating
        Constant::EndUse::Cooling,           // Cooling
        Constant::EndUse::InteriorLights,    // InteriorLights
        Constant::EndUse::ExteriorLights,    // ExteriorLights
        Constant::EndUse::InteriorEquipment, // InteriorEquipment
        Constant::EndUse::ExteriorEquipment, // ExteriorEquipment
        Constant::EndUse::Fans,              // Fans
        Constant::EndUse::Pumps,             // Pumps
        Constant::EndUse::HeatRejection,     // HeatRejection
        Constant::EndUse::Humidification,    // Humidification
        Constant::EndUse::HeatRecovery,      // HeatRecovery
        Constant::EndUse::WaterSystem,       // WaterSystem
        Constant::EndUse::Refrigeration,     // Refrigeration
        Constant::EndUse::Cogeneration,      // Cogeneration
        Constant::EndUse::Invalid,           // Baseboard
        Constant::EndUse::Invalid,           // Boilers
        Constant::EndUse::Invalid,           // CarbonEquivalentEmissions
        Constant::EndUse::Invalid,           // Chillers
        Constant::EndUse::Invalid,           // CoalEmissions
        Constant::EndUse::Invalid,           // ColdStorageCharge
        Constant::EndUse::Invalid,           // ColdStorageDischarge
        Constant::EndUse::Invalid,           // Condensate
        Constant::EndUse::Invalid,           // CoolingCoils
        Constant::EndUse::Invalid,           // CoolingPanel
        Constant::EndUse::Invalid,           // DieselEmissions
        Constant::EndUse::Invalid,           // DistrictChilledWater
        Constant::EndUse::Invalid,           // DistrictHotWater
        Constant::EndUse::Invalid,           // ElectricityEmissions
        Constant::EndUse::Invalid,           // ElectricStorage
        Constant::EndUse::Invalid,           // FreeCooling
        Constant::EndUse::Invalid,           // FuelOilNo1Emissions
        Constant::EndUse::Invalid,           // FuelOilNo2Emissions
        Constant::EndUse::Invalid,           // GasolineEmissions
        Constant::EndUse::Invalid,           // HeatingCoils
        Constant::EndUse::Invalid,           // HeatProduced
        Constant::EndUse::Invalid,           // HeatRecoveryForCooling
        Constant::EndUse::Invalid,           // HeatRecoveryForHeating
        Constant::EndUse::Invalid,           // LoopToLoop
        Constant::EndUse::Invalid,           // MainsWater
        Constant::EndUse::Invalid,           // NaturalGasEmissions
        Constant::EndUse::Invalid,           // OtherFuel1Emissions
        Constant::EndUse::Invalid,           // OtherFuel2Emissions
        Constant::EndUse::Invalid,           // Photovoltaic
        Constant::EndUse::Invalid,           // PowerConversion
        Constant::EndUse::Invalid,           // PropaneEmissions
        Constant::EndUse::Invalid,           // PurchasedElectricityEmissions
        Constant::EndUse::Invalid,           // RainWater
        Constant::EndUse::Invalid,           // SoldElectricityEmissions
        Constant::EndUse::Invalid,           // WellWater,
        Constant::EndUse::Invalid,           // WindTurbine,
    };

    enum class Group
    {
        Invalid = -1,
        Building,
        HVAC,
        Plant,
        Zone,
        SpaceType,
        Num
    };

    static constexpr std::array<std::string_view, (int)Group::Num> groupNames = {"Building", "HVAC", "Plant", "Zone", "SpaceType"};
    static constexpr std::array<std::string_view, (int)Group::Num> groupNamesUC = {"BUILDING", "HVAC", "PLANT", "ZONE", "SPACETYPE"};

    struct TimeSteps
    {
        Real64 *TimeStep = nullptr; // Pointer to the Actual Time Step Variable (Zone or HVAC)
        Real64 CurMinute = 0.0;     // Current minute (decoded from real Time Step Value)
    };

    struct OutVar
    {
        int ddVarNum = -1;
        VariableType varType = VariableType::Invalid;
        TimeStepType timeStepType = TimeStepType::Zone; // Zone or System
        StoreType storeType = StoreType::Average;       // Variable Type (Summed/Non-Static or Average/Static)
        Real64 Value = 0.0;                             // Current Value of the variable (to resolution of Zone Time Step)
        Real64 TSValue = 0.0;                           // Value of this variable at the Zone Time Step
        Real64 EITSValue = 0.0;                         // Value of this variable at the Zone Time Step for external interface
        Real64 StoreValue = 0.0;                        // At end of Zone Time Step, value is placed here for later reporting
        Real64 NumStored = 0.0;                         // Number of hours stored
        bool Stored = false;                            // True when value is stored
        bool Report = false;                            // User has requested reporting of this variable in the IDF
        bool tsStored = false;                          // if stored for this zone timestep
        bool thisTSStored = false;                      // if stored for this zone timestep
        int thisTSCount = 0;
        ReportFreq freq = ReportFreq::Hour; // How often to report this variable
        Real64 MaxValue = -9999.0;          // Maximum reporting (only for Averaged variables, and those greater than Time Step)
        Real64 MinValue = 9999.0;           // Minimum reporting (only for Averaged variables, and those greater than Time Step)
        int maxValueDate = 0;               // Date stamp of maximum
        int minValueDate = 0;               // Date stamp of minimum
        int ReportID = 0;                   // Report variable ID number
        Sched::Schedule *sched = nullptr;   // If scheduled, this is schedule
        int ZoneMult = 1;                   // If metered, Zone Multiplier is applied
        int ZoneListMult = 1;               // If metered, Zone List Multiplier is applied

        std::string keyColonName = "";   // Name of Variable key:variable
        std::string keyColonNameUC = ""; // Name of Variable (Uppercase)
        std::string name = "";           // Name of Variable
        std::string nameUC = "";         // Name of Variable with out key in uppercase
        std::string key = "";            // Name of key only
        std::string keyUC = "";          // Name of key only witht out variable in uppercase

        Constant::Units units = Constant::Units::Invalid; // Units for Variable
        std::string unitNameCustomEMS;                    // name of units when customEMS is used for EMS variables that are unusual

        std::string indexGroup = "";
        int indexGroupKey = -1; // Is this thing even used?

        std::vector<int> meterNums; // Meter Numbers

        virtual ~OutVar() = default;

        std::string multiplierString() const;

        void writeReportData(EnergyPlusData &state);

        void writeOutput(EnergyPlusData &state, // Real variable to write out
                         ReportFreq const freq);

        void writeReportDictionaryItem(EnergyPlusData &state);
    };

    struct OutVarReal : public OutVar
    {
        Real64 *Which = nullptr; // Pointer to the actual variable holding the value

        OutVarReal()
        {
            varType = VariableType::Real;
        }
    };

    struct OutVarInt : public OutVar
    {
        // Members
        int *Which = nullptr; // The POINTER to the actual variable holding the value

        OutVarInt()
        {
            varType = VariableType::Integer;
        }
    };

    struct DDOutVar
    {
        // Members
        std::string name = "";                             // Name of Variable
        TimeStepType timeStepType = TimeStepType::Invalid; // Type whether Zone or HVAC
        StoreType storeType = StoreType::Invalid;          // Variable Type (Summed/Non-Static or Average/Static)
        VariableType variableType = VariableType::Invalid; // Integer, Real.
        int Next = -1;                                     // Next variable of same name (different units)
        bool ReportedOnDDFile = false;                     // true after written to .rdd/.mdd file
        Constant::Units units = Constant::Units::Invalid;  // Units for Variable
        std::string unitNameCustomEMS = "";                // name of units when customEMS is used for EMS variables that are unusual

        std::vector<int> keyOutVarNums;
    };

    struct ReqVar // Structure for requested Report Variables
    {
        // Members
        std::string key = "";               // Could be blank or "*"
        std::string name = "";              // Name of Variable
        ReportFreq freq = ReportFreq::Hour; // Reporting Frequency
        Sched::Schedule *sched = nullptr;   // Schedule
        bool Used = false;                  // True when this combination (key, varname, frequency) has been set

        bool is_simple_string = true; // Whether the Key potentially includes a Regular Expression pattern
        std::shared_ptr<RE2> case_insensitive_pattern = nullptr;
    };

    struct MeterPeriod
    {
        Real64 Value = 0.0;          // Daily Value
        Real64 MaxVal = MaxSetValue; // Maximum Value
        int MaxValDate = -1;         // Date stamp of maximum
        Real64 MinVal = MinSetValue; // Minimum Value
        int MinValDate = -1;         // Date stamp of minimum

        bool Rpt = false;   // Report at End of Day
        bool RptFO = false; // Report at End of Day -- meter file only
        int RptNum = 0;     // Report Number
        bool accRpt = false;
        bool accRptFO = false;
        int accRptNum = 0;

        void resetVals()
        {
            Value = 0.0;
            MaxVal = MaxSetValue;
            MaxValDate = 0;
            MinVal = MinSetValue;
            MinValDate = 0;
        }

        void WriteReportData(EnergyPlusData &state, ReportFreq freq);
    };

    struct Meter
    {
        // Members
        std::string Name = "";                                       // Name of the meter
        MeterType type = MeterType::Invalid;                         // type of meter
        Constant::eResource resource = Constant::eResource::Invalid; // Resource Type of the meter
        EndUseCat endUseCat = EndUseCat::Invalid;                    // End Use of the meter
        std::string EndUseSub = "";                                  // End Use subcategory of the meter
        Group group = Group::Invalid;                                // Group of the meter
        Constant::Units units = Constant::Units::Invalid;            // Units for the Meter
        RT_IPUnits RT_forIPUnits;                                    // Resource type number for IP Units (tabular) reporting

        Real64 CurTSValue = 0.0; // Current TimeStep Value (internal access)

        std::string indexGroup = "";

        std::array<MeterPeriod, (int)ReportFreq::Num> periods;

        MeterPeriod periodLastSM;
        MeterPeriod periodFinYrSM;

        std::vector<int> dstMeterNums; // Destination meters for custom meters

        int decMeterNum = -1;          // for custom decrement meters, the number of the meter being subtracted from
        std::vector<int> srcVarNums;   // Source variables for custom meters
        std::vector<int> srcMeterNums; // Source meters for custom meters

        Meter(std::string_view name)
        {
            Name = std::string(name);
        }
    };

    struct MeteredVar
    {
        int num = -1;
        std::string name = "";
        Constant::eResource resource = Constant::eResource::Invalid;
        Constant::Units units = Constant::Units::Invalid;
        VariableType varType = VariableType::Invalid;
        TimeStepType timeStepType = TimeStepType::Invalid;
        EndUseCat endUseCat = EndUseCat::Invalid;
        Group group = Group::Invalid;
        int rptNum = -1;
    };

    struct MeterData : MeteredVar
    {
        Constant::HeatOrCool heatOrCool = Constant::HeatOrCool::NoHeatNoCool;
        Real64 curMeterReading = 0.0;

        // C++ doesn't figure this out automatically, so we have to do it ourselves
        MeterData &operator=(MeteredVar const &mv)
        {
            MeteredVar::operator=(mv);
            return *this;
        }
    };

    struct EndUseCategoryType
    {
        // Members
        std::string Name;        // End use category name
        std::string DisplayName; // Display name for output table
        int NumSubcategories = 0;
        Array1D_string SubcategoryName; // Array of subcategory names
        int numSpaceTypes = 0;
        Array1D_string spaceTypeName; // Array of space type names
    };

    int DetermineMinuteForReporting(EnergyPlusData const &state);

    void InitializeOutput(EnergyPlusData &state);

    void SetupTimePointers(EnergyPlusData &state,
                           OutputProcessor::TimeStepType timeStepType, // Which timestep is being set up, 'Zone'=1, 'HVAC'=2
                           Real64 &TimeStep                            // The timestep variable.  Used to get the address
    );

    void CheckReportVariable(EnergyPlusData &state,
                             std::string_view const name, // String Name of variable (without units)
                             std::string const &key,      // Associated Key for this variable
                             std::vector<int> &reqVarNums);

    void GetReportVariableInput(EnergyPlusData &state);

    ReportFreq determineFrequency(EnergyPlusData &state, std::string_view const FreqString);

    std::string produceDateString(int DateValue,  // Date of min/max
                                  ReportFreq freq // Reporting Frequency
    );

    // *****************************************************************************
    // The following routines implement Energy Meters in EnergyPlus.
    // *****************************************************************************

    void GetCustomMeterInput(EnergyPlusData &state, bool &ErrorsFound);

    int AddMeter(EnergyPlusData &state,
                 std::string const &Name,          // Name for the meter
                 Constant::Units units,            // Units for the meter
                 Constant::eResource resource,     // ResourceType for the meter
                 EndUseCat endUseCat,              // EndUse for the meter
                 std::string_view const EndUseSub, // EndUse subcategory for the meter
                 Group group,                      // Group for the meter
                 int outVarNum);

    void AttachMeters(EnergyPlusData &state,
                      Constant::Units units,            // Units for this meter
                      Constant::eResource resource,     // Electricity, Gas, etc.
                      EndUseCat endUseCat,              // End-use category (Lights, Heating, etc.)
                      std::string_view const EndUseSub, // End-use subcategory (user-defined, e.g., General Lights, Task Lights, etc.)
                      Group group,                      // Group key (Facility, Zone, Building, etc.)
                      std::string const &ZoneName,      // Zone key only applicable for Building group
                      std::string const &SpaceTypeName, // Space Type key only applicable for Building group
                      int RepVarNum                     // Number of this report variable
    );

    OutputProcessor::RT_IPUnits GetResourceIPUnits(EnergyPlusData &state,
                                                   Constant::eResource resource, // Resource Type
                                                   Constant::Units units,        // Meter units
                                                   bool &ErrorsFound             // true if errors found during subroutine
    );

    void UpdateMeters(EnergyPlusData &state, int TimeStamp); // Current TimeStamp (for max/min)

    void ReportTSMeters(EnergyPlusData &state,
                        Real64 StartMinute,      // Start Minute for TimeStep
                        Real64 EndMinute,        // End Minute for TimeStep
                        bool &PrintESOTimeStamp, // True if the ESO Time Stamp also needs to be printed
                        bool PrintTimeStampToSQL // Print Time Stamp to SQL file
    );

    void ReportMeters(EnergyPlusData &state, ReportFreq freq, bool PrintTimeStampToSQL // Print Time Stamp to SQL file
    );

    void ReportForTabularReports(EnergyPlusData &state);

    std::string DateToStringWithMonth(int codedDate); // word containing encoded month, day, hour, minute

    void ReportMeterDetails(EnergyPlusData &state);

    // *****************************************************************************
    // End of routines for Energy Meters implementation in EnergyPlus.
    // *****************************************************************************

    void WriteTimeStampFormatData(EnergyPlusData &state,
                                  InputOutputFile &outputFile,
                                  ReportFreq freq,                    // Reporting frequency.
                                  int reportID,                       // The ID of the time stamp
                                  std::string const &DayOfSimChr,     // the number of days simulated so far
                                  bool writeToSQL,                    // write to SQLite
                                  int const Month = -1,               // the month of the reporting interval
                                  int const DayOfMonth = -1,          // The day of the reporting interval
                                  int const Hour = -1,                // The hour of the reporting interval
                                  Real64 const EndMinute = -1.0,      // The last minute in the reporting interval
                                  Real64 const StartMinute = -1.0,    // The starting minute of the reporting interval
                                  int const DST = -1,                 // A flag indicating whether daylight savings time is observed
                                  std::string_view const DayType = "" // The day tied for the data (e.g., Monday)
    );

    void WriteYearlyTimeStamp(EnergyPlusData &state,
                              InputOutputFile &outputFile,
                              int reportID,
                              std::string const &yearOfSimChr, // the year of the simulation
                              bool writeToSQL);

    void WriteMeterDictionaryItem(EnergyPlusData &state,
                                  ReportFreq freq, // The reporting interval (e.g., hourly, daily)
                                  StoreType storeType,
                                  int reportID,                  // The reporting ID in for the variable
                                  std::string const &indexGroup, // The reporting group for the variable
                                  std::string const &meterName,  // The variable's meter name
                                  Constant::Units unit,          // The variables units
                                  bool cumulativeMeterFlag,      // A flag indicating cumulative data
                                  bool meterFileOnlyFlag         // A flag indicating whether the data is to be written to standard output
    );

    void WriteCumulativeReportMeterData(EnergyPlusData &state,
                                        int reportID,      // The variable's report ID
                                        Real64 repValue,   // The variable's value
                                        bool meterOnlyFlag // A flag that indicates if the data should be written to standard output
    );

    void WriteNumericData(EnergyPlusData &state,
                          int reportID,   // The variable's reporting ID
                          Real64 repValue // The variable's value
    );

    void WriteNumericData(EnergyPlusData &state,
                          int reportID,    // The variable's reporting ID
                          int32_t repValue // The variable's value
    );

    int DetermineIndexGroupKeyFromMeterName(EnergyPlusData &state, std::string const &meterName); // the meter name

    std::string DetermineIndexGroupFromMeterGroup(Meter const *meter); // the meter

    void SetInternalVariableValue(EnergyPlusData &state,
                                  OutputProcessor::VariableType varType, // 1=integer, 2=real, 3=meter
                                  int keyVarIndex,                       // Array index
                                  Real64 SetRealVal,                     // real value to set, if type is real or meter
                                  int SetIntVal                          // integer value to set if type is integer
    );

    std::string unitStringFromDDitem(EnergyPlusData &state, int ddItemPtr // index provided for DDVariableTypes
    );

    struct APIOutputVariableRequest
    {
        std::string varName;
        std::string varKey;
    };

    std::string standardizeEndUseSub(EndUseCat endUseCat, std::string_view const endUseSubCat);
    void addEndUseSubcategory(EnergyPlusData &state, EndUseCat endUseCat, std::string_view const endUseSubCat);
    void addEndUseSpaceType(EnergyPlusData &state, EndUseCat endUseCat, std::string_view const endUseSpTypeName);
} // namespace OutputProcessor

//==============================================================================================
// *****************************************************************************
// These routines are available outside the OutputProcessor Module (i.e. calling
// routines do not have to "USE OutputProcessor".  But each of these routines
// will use the OutputProcessor and take advantage that everything is PUBLIC
// within the OutputProcessor.
// *****************************************************************************

void SetupOutputVariable(
    EnergyPlusData &state,
    std::string_view const name,                                                // String Name of variable (with units)
    Constant::Units units,                                                      // Actual units corresponding to the actual variable
    Real64 &ActualVariable,                                                     // Actual Variable, used to set up pointer
    OutputProcessor::TimeStepType timeStepType,                                 // Zone, HeatBalance=1, HVAC, System, Plant=2
    OutputProcessor::StoreType variableType,                                    // State, Average=1, NonState, Sum=2
    std::string const &key,                                                     // Associated Key for this variable
    Constant::eResource resource = Constant::eResource::Invalid,                // Meter Resource Type (Electricity, Gas, etc)
    OutputProcessor::Group group = OutputProcessor::Group::Invalid,             // Meter Super Group Key (Building, System, Plant)
    OutputProcessor::EndUseCat endUseCat = OutputProcessor::EndUseCat::Invalid, // Meter End Use Key (Lights, Heating, etc)
    std::string_view const endUseSub = {},                                      // Meter End Use Sub Key (General Lights, Task Lights, etc)
    std::string const &zone = {},                                               // Meter Zone Key (zone name)
    int const zoneMult = 1,                                                     // Zone Multiplier, defaults to 1
    int const zoneListMult = 1,                                                 // Zone List Multiplier, defaults to 1
    std::string const &spaceType = {},                                          // Space type (applicable for Building group only)
    int const indexGroupKey = -999,                                             // Group identifier for SQL output
    std::string_view const customUnitName = {},                                 // the custom name for the units from EMS definition of units
    OutputProcessor::ReportFreq reportFreq = OutputProcessor::ReportFreq::Hour  // Internal use -- causes reporting at this freqency
);

void SetupOutputVariable(EnergyPlusData &state,
                         std::string_view const VariableName,                                 // String Name of variable
                         Constant::Units VariableUnit,                                        // Actual units corresponding to the actual variable
                         int &ActualVariable,                                                 // Actual Variable, used to set up pointer
                         OutputProcessor::TimeStepType TimeStepType,                          // Zone, HeatBalance=1, HVAC, System, Plant=2
                         OutputProcessor::StoreType VariableType,                             // State, Average=1, NonState, Sum=2
                         std::string const &KeyedValue,                                       // Associated Key for this variable
                         int const indexGroupKey = -999,                                      // Group identifier for SQL output
                         OutputProcessor::ReportFreq freq = OutputProcessor::ReportFreq::Hour // Internal use -- causes reporting at this freqency
);

// A helper to be able to cast an enum to an int& for use in the above function
template <typename EnumType, typename = std::enable_if_t<std::is_enum_v<EnumType>>>
void SetupOutputVariable(EnergyPlusData &state,
                         std::string_view const VariableName,                                 // String Name of variable
                         Constant::Units VariableUnit,                                        // Actual units corresponding to the actual variable
                         EnumType &ActualVariable,                                            // Actual Variable, used to set up pointer
                         OutputProcessor::TimeStepType TimeStepType,                          // Zone, HeatBalance=1, HVAC, System, Plant=2
                         OutputProcessor::StoreType VariableType,                             // State, Average=1, NonState, Sum=2
                         std::string const &KeyedValue,                                       // Associated Key for this variable
                         int const indexGroupKey = -999,                                      // Group identifier for SQL output
                         OutputProcessor::ReportFreq freq = OutputProcessor::ReportFreq::Hour // Internal use -- causes reporting at this freqency
)
{
#if defined(__GNUC__) || defined(__clang__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif
    SetupOutputVariable(state, VariableName, VariableUnit, (int &)ActualVariable, TimeStepType, VariableType, KeyedValue, indexGroupKey, freq);
#if defined(__GNUC__) || defined(__clang__)
#    pragma GCC diagnostic pop
#endif
};

void UpdateDataandReport(EnergyPlusData &state, OutputProcessor::TimeStepType TimeStepTypeKey); // What kind of data to update (Zone, HVAC)

void GenOutputVariablesAuditReport(EnergyPlusData &state);

void UpdateMeterReporting(EnergyPlusData &state);

void SetInitialMeterReportingAndOutputNames(EnergyPlusData &state,
                                            int WhichMeter,                   // Which meter number
                                            bool MeterFileOnlyIndicator,      // true if this is a meter file only reporting
                                            OutputProcessor::ReportFreq freq, // at what frequency is the meter reported
                                            bool CumulativeIndicator          // true if this is a Cumulative meter reporting
);

int GetMeterIndex(EnergyPlusData const &state, std::string const &MeterName);

Constant::eResource GetMeterResourceType(EnergyPlusData const &state, int MeterNumber); // Which Meter Number (from GetMeterIndex)

Real64 GetCurrentMeterValue(EnergyPlusData const &state, int MeterNumber); // Which Meter Number (from GetMeterIndex)

Real64 GetInstantMeterValue(EnergyPlusData &state,
                            int MeterNumber,                           // Which Meter Number (from GetMeterIndex)
                            OutputProcessor::TimeStepType TimeStepType // Whether this is zone of HVAC
);

Real64 GetInternalVariableValue(EnergyPlusData &state,
                                OutputProcessor::VariableType varType, // 1=integer, 2=real, 3=meter
                                int keyVarIndex                        // Array index
);

Real64 GetInternalVariableValueExternalInterface(EnergyPlusData &state,
                                                 OutputProcessor::VariableType varType, // 1=integer, 2=REAL(r64), 3=meter
                                                 int keyVarIndex                        // Array index
);

int GetNumMeteredVariables(EnergyPlusData const &state,
                           std::string const &ComponentType, // Given Component Type
                           std::string const &ComponentName  // Given Component Name (user defined)
);

int GetMeteredVariables(EnergyPlusData &state,
                        std::string const &ComponentName,                 // Given Component Name (user defined)
                        Array1D<OutputProcessor::MeteredVar> &meteredVars // Number Found
);

void GetVariableKeyCountandType(EnergyPlusData &state,
                                std::string const &varName, // Standard variable name
                                int &numKeys,               // Number of keys found
                                OutputProcessor::VariableType &varType,
                                OutputProcessor::StoreType &varAvgSum,      // Variable  is Averaged=1 or Summed=2
                                OutputProcessor::TimeStepType &varStepType, // Variable time step is Zone=1 or HVAC=2
                                Constant::Units &varUnits                   // Units enumeration
);

void GetVariableKeys(EnergyPlusData &state,
                     std::string const &varName, // Standard variable name
                     OutputProcessor::VariableType varType,
                     Array1D_string &keyNames,
                     Array1D_int &keyVarNums // Array index for
);

bool ReportingThisVariable(EnergyPlusData &state, std::string const &RepVarName);

void InitPollutionMeterReporting(EnergyPlusData &state, OutputProcessor::ReportFreq freq);

void ProduceRDDMDD(EnergyPlusData &state);

int AddDDOutVar(EnergyPlusData const &state,
                std::string_view const nameUC, // Variable Name
                OutputProcessor::TimeStepType TimeStepType,
                OutputProcessor::StoreType StateType,
                OutputProcessor::VariableType VariableType,
                Constant::Units unitsForVar,
                std::string_view const customUnitName = {} // the custom name for the units from EMS definition of units
);

int initErrorFile(EnergyPlusData &state);

struct OutputProcessorData : BaseGlobalStruct
{
    int NumVariablesForOutput = 0;
    int MaxVariablesForOutput = 0;
    int NumTotalRVariable = 0;
    int NumOfRVariable = 0;
    int NumOfRVariable_Setup = 0;
    int NumOfRVariable_Sum = 0;
    int NumOfRVariable_Meter = 0;
    int NumOfIVariable = 0;
    int NumOfIVariable_Setup = 0;
    int NumTotalIVariable = 0;
    int NumOfIVariable_Sum = 0;
    bool OutputInitialized = false;
    OutputProcessor::ReportVDD ProduceReportVDD = OutputProcessor::ReportVDD::No;
    int NumHoursInMonth = 0;
    int NumHoursInSim = 0;
    std::vector<Real64> meterValues; // This holds the current timestep value for each meter.

    std::array<int, (int)OutputProcessor::ReportFreq::Num> freqStampReportNums = {-1, -1, -1, -1, -1, -1, -1};
    std::array<bool, (int)OutputProcessor::ReportFreq::Num> freqTrackingVariables = {false, false, false, false, false, false, false};

    Real64 TimeStepZoneSec = 0; // Seconds from NumTimeStepInHour
    bool ErrorsLogged = false;
    bool isFinalYear = false;
    bool GetOutputInputFlag = true;
    OutputProcessor::ReportFreq minimumReportFreq = OutputProcessor::ReportFreq::EachCall;
    std::vector<OutputProcessor::APIOutputVariableRequest> apiVarRequests;
    int ReportNumberCounter = 0;        // The report number is used in output reports as a key.
    int LHourP = -1;                    // Helps set hours for timestamp output
    Real64 LStartMin = -1.0;            // Helps set minutes for timestamp output
    Real64 LEndMin = -1.0;              // Helps set minutes for timestamp output
    bool GetMeterIndexFirstCall = true; // trigger setup in GetMeterIndex
    bool InitFlag = true;

    std::array<OutputProcessor::TimeSteps, (int)OutputProcessor::TimeStepType::Num> TimeValue; // Pointers to the actual TimeStep variables

    std::vector<OutputProcessor::OutVar *> outVars;     // Variables array (use NumOfRVariables to traverse)
    std::vector<OutputProcessor::DDOutVar *> ddOutVars; // Variable Types structure (use NumVariablesForOutput to traverse)
    std::map<std::string, int> ddOutVarMap;
    std::vector<OutputProcessor::ReqVar *> reqVars;

    std::vector<OutputProcessor::Meter *> meters;
    std::map<std::string, int> meterMap;

    char stamp[OutputProcessor::N_WriteTimeStampFormatData];
    bool Rept = false;
    bool OpaqSurfWarned = false;

    // End-use stuff
    int MaxNumSubcategories = 1;
    int maxNumEndUseSpaceTypes = 1;
    EPVector<OutputProcessor::EndUseCategoryType> EndUseCategory;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->NumVariablesForOutput = 0;
        this->MaxVariablesForOutput = 0;
        this->NumOfRVariable_Setup = 0;
        this->NumTotalRVariable = 0;
        this->NumOfRVariable_Sum = 0;
        this->NumOfRVariable_Meter = 0;
        this->NumOfIVariable_Setup = 0;
        this->NumTotalIVariable = 0;
        this->NumOfIVariable_Sum = 0;
        this->OutputInitialized = false;
        this->ProduceReportVDD = OutputProcessor::ReportVDD::No;
        this->NumHoursInMonth = 0;
        this->NumHoursInSim = 0;
        this->meterValues.clear();
        this->freqStampReportNums = {-1, -1, -1, -1, -1, -1, -1};
        this->freqTrackingVariables = {false};
        this->TimeStepZoneSec = 0;
        this->ErrorsLogged = false;
        this->isFinalYear = false;
        this->GetOutputInputFlag = true;
        this->minimumReportFreq = OutputProcessor::ReportFreq::EachCall;
        this->apiVarRequests.clear();
        this->ReportNumberCounter = 0;
        this->LHourP = -1;
        this->LStartMin = -1.0;
        this->LEndMin = -1.0;
        this->GetMeterIndexFirstCall = true;
        this->InitFlag = true;

        for (int i = 0; i < (int)OutputProcessor::TimeStepType::Num; ++i) {
            new (&this->TimeValue[i]) OutputProcessor::TimeSteps();
        }

        for (int i = 0; i < (int)this->outVars.size(); ++i) {
            delete this->outVars[i];
        }
        this->outVars.clear();

        for (int i = 0; i < (int)this->ddOutVars.size(); ++i) {
            delete this->ddOutVars[i];
        }
        this->ddOutVars.clear();
        this->ddOutVarMap.clear();

        for (int i = 0; i < (int)this->reqVars.size(); ++i) {
            delete this->reqVars[i];
        }
        this->reqVars.clear();

        for (int i = 0; i < (int)this->meters.size(); ++i) {
            delete this->meters[i];
        }
        this->meters.clear();
        this->meterMap.clear();

        this->Rept = false;
        this->OpaqSurfWarned = false;

        this->MaxNumSubcategories = 1;
        this->maxNumEndUseSpaceTypes = 1;
        this->EndUseCategory.deallocate();
    }
};

} // namespace EnergyPlus

#endif
