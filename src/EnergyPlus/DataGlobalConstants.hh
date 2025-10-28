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

#ifndef DataGlobalConstants_hh_INCLUDED
#define DataGlobalConstants_hh_INCLUDED

// EnergyPlus Headers
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

namespace Constant {

#if LINK_WITH_PYTHON && EP_PYTHON_CLI
    static constexpr bool python_cli_enabled = true;
#else
    static constexpr bool python_cli_enabled = false;
#endif

    enum class EndUse
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
        Num
    };

    static constexpr std::array<std::string_view, (int)EndUse::Num> endUseNamesUC = {"HEATING",
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
                                                                                     "COGENERATION"};

    enum class eResource
    {
        Invalid = -1,
        Electricity,
        NaturalGas,
        Gasoline,
        Diesel,
        Coal,
        Propane,
        FuelOilNo1,
        FuelOilNo2,
        OtherFuel1,
        OtherFuel2,
        DistrictCooling,
        DistrictHeatingWater,
        DistrictHeatingSteam,
        Water,
        None, // used for OtherEquipment object
        EnergyTransfer,
        ElectricityProduced,
        ElectricityPurchased,
        ElectricitySurplusSold,
        ElectricityNet,
        SolarWater,
        SolarAir,
        CarbonEquivalent,
        PlantLoopHeatingDemand,
        PlantLoopCoolingDemand,
        OnSiteWater,
        MainsWater,
        RainWater,
        WellWater,
        Condensate,
        WaterEnvironmentalFactors,
        Source,
        Generic, // only used by custom meters
        SO2,
        NOx,
        N2O,
        PM,
        PM2_5,
        PM10,
        CO,
        CO2,
        CH4,
        NH3,
        NMVOC,
        Hg,
        Pb,
        NuclearHigh,
        NuclearLow,
        Num
    };

    enum class eFuel
    {
        Invalid = -1,
        Electricity,
        NaturalGas,
        Gasoline,
        Diesel,
        Coal,
        Propane,
        FuelOilNo1,
        FuelOilNo2,
        OtherFuel1,
        OtherFuel2,
        DistrictCooling,
        DistrictHeatingWater,
        DistrictHeatingSteam,
        Water,
        None, // used for OtherEquipment object
        Num
    };

    constexpr std::array<eFuel, static_cast<int>(eResource::Num)> eResource2eFuel = {eFuel::Electricity,
                                                                                     eFuel::NaturalGas,
                                                                                     eFuel::Gasoline,
                                                                                     eFuel::Diesel,
                                                                                     eFuel::Coal,
                                                                                     eFuel::Propane,
                                                                                     eFuel::FuelOilNo1,
                                                                                     eFuel::FuelOilNo2,
                                                                                     eFuel::OtherFuel1,
                                                                                     eFuel::OtherFuel2,
                                                                                     eFuel::DistrictCooling,
                                                                                     eFuel::DistrictHeatingWater,
                                                                                     eFuel::DistrictHeatingSteam,
                                                                                     eFuel::Water,
                                                                                     eFuel::None,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid,
                                                                                     eFuel::Invalid};

    constexpr std::array<eResource, (int)eFuel::Num> eFuel2eResource = {eResource::Electricity,
                                                                        eResource::NaturalGas,
                                                                        eResource::Gasoline,
                                                                        eResource::Diesel,
                                                                        eResource::Coal,
                                                                        eResource::Propane,
                                                                        eResource::FuelOilNo1,
                                                                        eResource::FuelOilNo2,
                                                                        eResource::OtherFuel1,
                                                                        eResource::OtherFuel2,
                                                                        eResource::DistrictCooling,
                                                                        eResource::DistrictHeatingWater,
                                                                        eResource::DistrictHeatingSteam,
                                                                        eResource::Water,
                                                                        eResource::None};

    static constexpr std::array<std::string_view, (int)eResource::Num> eResourceNamesUC = {"ELECTRICITY",
                                                                                           "NATURALGAS",
                                                                                           "GASOLINE",
                                                                                           "DIESEL",
                                                                                           "COAL",
                                                                                           "PROPANE",
                                                                                           "FUELOILNO1",
                                                                                           "FUELOILNO2",
                                                                                           "OTHERFUEL1",
                                                                                           "OTHERFUEL2",
                                                                                           "DISTRICTCOOLING",
                                                                                           "DISTRICTHEATINGWATER",
                                                                                           "DISTRICTHEATINGSTEAM",
                                                                                           "WATER",
                                                                                           "NONE",
                                                                                           "ENERGYTRANSFER",
                                                                                           "ELECTRICITYPRODUCED",
                                                                                           "ELECTRICITYPURCHASED",
                                                                                           "ELECTRICITYSURPLUSSOLD",
                                                                                           "ELECTRICITYNET",
                                                                                           "SOLARWATER",
                                                                                           "SOLARAIR",
                                                                                           "CARBON EQUIVALENT",
                                                                                           "PLANTLOOPHEATINGDEMAND",
                                                                                           "PLANTLOOPCOOLINGDEMAND",
                                                                                           "ONSITEWATER",
                                                                                           "MAINSWATER",
                                                                                           "RAINWATER",
                                                                                           "WELLWATER",
                                                                                           "CONDENSATE",
                                                                                           "WATERENVIRONMENTALFACTORS",
                                                                                           "SOURCE",
                                                                                           "GENERIC",
                                                                                           "SO2",
                                                                                           "NOX",
                                                                                           "N2O",
                                                                                           "PM",
                                                                                           "PM2.5",
                                                                                           "PM10",
                                                                                           "CO",
                                                                                           "CO2",
                                                                                           "CH4",
                                                                                           "NH3",
                                                                                           "NMVOC",
                                                                                           "HG",
                                                                                           "PB",
                                                                                           "NUCLEAR HIGH",
                                                                                           "NUCLEAR LOW"};

    static constexpr std::array<std::string_view, (int)eResource::Num> eResourceNames = {"Electricity",
                                                                                         "NaturalGas",
                                                                                         "Gasoline",
                                                                                         "Diesel",
                                                                                         "Coal",
                                                                                         "Propane",
                                                                                         "FuelOilNo1",
                                                                                         "FuelOilNo2",
                                                                                         "OtherFuel1",
                                                                                         "OtherFuel2",
                                                                                         "DistrictCooling",
                                                                                         "DistrictHeatingWater",
                                                                                         "DistrictHeatingSteam",
                                                                                         "Water",
                                                                                         "None",
                                                                                         "EnergyTransfer",
                                                                                         "ElectricityProduced",
                                                                                         "ElectricityPurchased",
                                                                                         "ElectricitySurplusSold",
                                                                                         "ElectricityNet",
                                                                                         "SolarWater",
                                                                                         "SolarAir",
                                                                                         "Carbon Equivalent",
                                                                                         "PlantLoopHeatingDemand",
                                                                                         "PlantLoopCoolingDemand",
                                                                                         "OnSiteWater",
                                                                                         "MainsWater",
                                                                                         "RainWater",
                                                                                         "WellWater",
                                                                                         "Condensate",
                                                                                         "WaterEnvironmentalFactors",
                                                                                         "Source",
                                                                                         "Generic",
                                                                                         "SO2",
                                                                                         "NOx",
                                                                                         "N2O",
                                                                                         "PM",
                                                                                         "PM2.5",
                                                                                         "PM10",
                                                                                         "CO",
                                                                                         "CO2",
                                                                                         "CH4",
                                                                                         "NH3",
                                                                                         "NMVOC",
                                                                                         "Hg",
                                                                                         "Pb",
                                                                                         "Nuclear High",
                                                                                         "Nuclear Low"};

    static constexpr std::array<std::string_view, (int)eFuel::Num> eFuelNamesUC = {
        eResourceNamesUC[(int)eFuel2eResource[(int)eFuel::Electricity]],
        eResourceNamesUC[(int)eFuel2eResource[(int)eFuel::NaturalGas]],
        eResourceNamesUC[(int)eFuel2eResource[(int)eFuel::Gasoline]],
        eResourceNamesUC[(int)eFuel2eResource[(int)eFuel::Diesel]],
        eResourceNamesUC[(int)eFuel2eResource[(int)eFuel::Coal]],
        eResourceNamesUC[(int)eFuel2eResource[(int)eFuel::Propane]],
        eResourceNamesUC[(int)eFuel2eResource[(int)eFuel::FuelOilNo1]],
        eResourceNamesUC[(int)eFuel2eResource[(int)eFuel::FuelOilNo2]],
        eResourceNamesUC[(int)eFuel2eResource[(int)eFuel::OtherFuel1]],
        eResourceNamesUC[(int)eFuel2eResource[(int)eFuel::OtherFuel2]],
        eResourceNamesUC[(int)eFuel2eResource[(int)eFuel::DistrictCooling]],
        eResourceNamesUC[(int)eFuel2eResource[(int)eFuel::DistrictHeatingWater]],
        eResourceNamesUC[(int)eFuel2eResource[(int)eFuel::DistrictHeatingSteam]],
        eResourceNamesUC[(int)eFuel2eResource[(int)eFuel::Water]],
        eResourceNamesUC[(int)eFuel2eResource[(int)eFuel::None]]};

    static constexpr std::array<std::string_view, (int)eFuel::Num> eFuelNames = {
        eResourceNames[(int)eFuel2eResource[(int)eFuel::Electricity]],
        eResourceNames[(int)eFuel2eResource[(int)eFuel::NaturalGas]],
        eResourceNames[(int)eFuel2eResource[(int)eFuel::Gasoline]],
        eResourceNames[(int)eFuel2eResource[(int)eFuel::Diesel]],
        eResourceNames[(int)eFuel2eResource[(int)eFuel::Coal]],
        eResourceNames[(int)eFuel2eResource[(int)eFuel::Propane]],
        eResourceNames[(int)eFuel2eResource[(int)eFuel::FuelOilNo1]],
        eResourceNames[(int)eFuel2eResource[(int)eFuel::FuelOilNo2]],
        eResourceNames[(int)eFuel2eResource[(int)eFuel::OtherFuel1]],
        eResourceNames[(int)eFuel2eResource[(int)eFuel::OtherFuel2]],
        eResourceNames[(int)eFuel2eResource[(int)eFuel::DistrictCooling]],
        eResourceNames[(int)eFuel2eResource[(int)eFuel::DistrictHeatingWater]],
        eResourceNames[(int)eFuel2eResource[(int)eFuel::DistrictHeatingSteam]],
        eResourceNames[(int)eFuel2eResource[(int)eFuel::Water]],
        eResourceNames[(int)eFuel2eResource[(int)eFuel::None]]};

    enum class Units : signed int
    {
        Invalid = -1,
        kg_s,
        C,
        kgWater_kgDryAir,
        ppm,
        Pa,
        m3_s,
        None,
        min,
        W,
        J,
        m3,
        kg,
        ach,
        W_W,
        lux,
        lum_W,
        hr,
        cd_m2,
        J_kgWater,
        m_s,
        W_m2,
        m,
        Ah,
        A,
        V,
        deltaC,
        kmol_s,
        umol_m2s,
        rev_min,
        Btu_h_W,
        W_m2K,
        J_kg,
        kg_kg,
        Perc,
        deg,
        s,
        kg_m3,
        kg_m2s,
        J_kgK,
        L,
        K_m,
        m2,
        W_m2C,
        rad,
        J_m2,
        clo,
        W_mK,
        W_K,
        K_W,
        kgWater_s,
        unknown,
        customEMS,
        Num
    };

    constexpr std::array<std::string_view, (int)Units::Num> unitNames = {
        "kg/s",             // kg_s
        "C",                // C
        "kgWater/kgDryAir", // kgWater_kgDryAir
        "ppm",              // ppm
        "Pa",               // Pa
        "m3/s",             // m3_s
        "",                 // None
        "min",              // min
        "W",                // W
        "J",                // J
        "m3",               // m3
        "kg",               // kg
        "ach",              // ach
        "W/W",              // W_W
        "lux",              // lux
        "lum/W",            // lum_W
        "hr",               // hr
        "cd/m2",            // cd_m2
        "J/kgWater",        // J_kgWater
        "m/s",              // m_s
        "W/m2",             // W_m2
        "m",                // m
        "Ah",               // Ah
        "A",                // A
        "V",                // V
        "deltaC",           // deltaC
        "kmol/s",           // kmol_s
        "umol/m2-s",        // umol_m2s (micromol_m2s)
        "rev/min",          // rev_min
        "Btu/h-W",          // Btu_h_W
        "W/m2-K",           // W_m2K
        "J/kg",             // J_kg
        "kg/kg",            // kg_kg
        "%",                // Perc
        "deg",              // deg
        "s",                // s
        "kg/m3",            // kg_m3
        "kg/m2-s",          // kg_m2s
        "J/kg-K",           // J_kgK
        "L",                // L
        "K/m",              // K_m
        "m2",               // m2
        "W/m2-C",           // W_m2C
        "rad",              // rad
        "J/m2",             // J_m2
        "clo",              // clo
        "W/m-K",            // W_mK
        "W/K",              // W_K
        "K/W",              // K_W
        "kgWater/s",        // kgWater_s
        "unknown",          // unknown
        "customEMS"         // customEMS
    };

    constexpr std::array<std::string_view, (int)Units::Num> unitNamesUC = {
        "KG/S",             // kg_s
        "C",                // C
        "KGWATER/KGDRYAIR", // kgWater_kgDryAir
        "PPM",              // ppm
        "PA",               // Pa
        "M3/S",             // m3_s
        "",                 // None
        "MIN",              // min
        "W",                // W
        "J",                // J
        "M3",               // m3
        "KG",               // kg
        "ACH",              // ach
        "W/W",              // W_W
        "LUX",              // lux
        "LUM/W",            // lum_W
        "HR",               // hr
        "CD/M2",            // cd_m2
        "J/KGWATER",        // J_kgWater
        "M/S",              // m_s
        "W/M2",             // W_m2
        "M",                // m
        "AH",               // Ah
        "A",                // A
        "V",                // V
        "DELTAC",           // deltaC
        "KMOL/S",           // kmol_s
        "UMOL/M2-S",        // umol_m2s (micromol_m2s)
        "REV/MIN",          // rev_min
        "BTH/H-W",          // Btu_h_W
        "W/M2-K",           // W_m2K
        "J/KG",             // J_kg
        "KG/KG",            // kg_kg
        "%",                // Perc
        "DEG",              // deg
        "S",                // s
        "KG/M3",            // kg_m3
        "KG/M2-S",          // kg_m2s
        "J/KG-K",           // J_kgK
        "L",                // L
        "K/M",              // K_m
        "M2",               // m2
        "W/M2-C",           // W_m2C
        "RAD",              // rad
        "J/M2",             // J_m2
        "CLO",              // clo
        "W/M-K",            // W_mK
        "W/K",              // W_K
        "K/W",              // K_W
        "KGWATER/S",        // kgWater_s
        "UNKNOWN",          // unknown
        "CUSTOMEMS"         // customEMS
    };

    enum class CallIndicator
    {
        Invalid = -1,
        BeginDay,
        DuringDay,
        EndDay,
        EndZoneSizingCalc,
        EndSysSizingCalc,
        Num
    };

    enum class HeatOrCool
    {
        Invalid = -1,
        NoHeatNoCool,
        CoolingOnly,
        HeatingOnly,
        HeatAndCool,
        Num
    };

    // Parameters for KindOfSim
    enum class KindOfSim
    {
        // TODO: enum check
        Invalid = -1,
        DesignDay = 1,
        RunPeriodDesign = 2,
        RunPeriodWeather = 3,
        HVACSizeDesignDay = 4,       // a regular design day run during HVAC Sizing Simulation
        HVACSizeRunPeriodDesign = 5, // a weather period design day run during HVAC Sizing Simulation
        ReadAllWeatherData = 6,      // a weather period for reading all weather data prior to the simulation
        Num
    };

    Real64 constexpr OneThird = 1.0 / 3.0;   // 1/3 in highest precision
    Real64 constexpr OneFourth = 1.0 / 4.0;  // 1/4 in highest precision
    Real64 constexpr OneFifth = 1.0 / 5.0;   // 1/5 in highest precision
    Real64 constexpr OneSixth = 1.0 / 6.0;   // 1/6 in highest precision
    Real64 constexpr FourFifths = 4.0 / 5.0; // 4/5 in highest precision
    Real64 constexpr OneThousandth = 1.0e-3; // Used as a tolerance in various places
    Real64 constexpr OneMillionth = 1.0e-6;  // Used as a tolerance in various places

    Real64 constexpr OneCentimeter = 0.01;     // Geometric tolerance in meters
    Real64 constexpr TwoCentimeters = 0.02;    // Geometric tolerance in meters
    Real64 constexpr SmallDistance = 1.0e-4;   // Geometric tolerance in meters
    Real64 constexpr MaxEXPArg = 709.78;       // maximum exponent in EXP() function
    Real64 constexpr Pi = 3.14159265358979324; // Pi 3.1415926535897932384626435
    Real64 constexpr PiOvr2 = Pi / 2.0;        // Pi/2
    Real64 constexpr TwoPi = 2.0 * Pi;         // 2*Pi 6.2831853071795864769252868
    Real64 constexpr Gravity = 9.807;
    Real64 constexpr DegToRad = Pi / 180.0; // Why is it DegToRadians and RadToDeg? Why? WHY?
    Real64 constexpr RadToDeg = 180.0 / Pi; // Conversion for Radians to Degrees

    int constexpr iSecsInMinute = 60;                           // Conversion for hours to seconds
    int constexpr iMinutesInHour = 60;                          // Conversion for hours to minutes
    int constexpr iSecsInHour = iSecsInMinute * iMinutesInHour; // Conversion for hours to seconds
    int constexpr iHoursInDay = 24;                             // Number of Hours in Day
    int constexpr iMinutesInDay = iMinutesInHour * iHoursInDay; // Number of seconds in Day
    int constexpr iSecsInDay = iSecsInHour * iHoursInDay;       // Number of seconds in Day

    Real64 constexpr rSecsInMinute = 60.0;                         // Conversion for hours to seconds
    Real64 constexpr rMinutesInHour = 60.0;                        // Conversion for hours to minutes
    Real64 constexpr rSecsInHour = rSecsInMinute * rMinutesInHour; // Conversion for hours to seconds
    Real64 constexpr rHoursInDay = 24.0;                           // Number of Hours in Day
    Real64 constexpr rMinutesInDay = rMinutesInHour * rHoursInDay; // Number of seconds in Day
    Real64 constexpr rSecsInDay = rSecsInHour * rHoursInDay;       // Number of seconds in Day

    Real64 constexpr BigNumber = std::numeric_limits<Real64>::max();      // Max Number real used for initializations
    Real64 constexpr rTinyValue = std::numeric_limits<Real64>::epsilon(); // Tiny value to replace use of TINY(x)
    std::string::size_type constexpr MaxNameLength =
        100;                          // Maximum Name Length in Characters -- should be the same as MaxAlphaArgLength in InputProcessor module
    Real64 constexpr Kelvin = 273.15; // Conversion factor for C to K and K to C
    Real64 constexpr TriplePointOfWaterTempKelvin = 273.16; // The triple point of water, in Kelvin
    Real64 constexpr InitConvTemp = 5.05;                   // [deg C], standard init vol to mass flow conversion temp
    Real64 constexpr AutoCalculate = -99999.0;              // automatically calculate some fields.
    Real64 constexpr CWInitConvTemp = 5.05;                 // [deg C], standard init chilled water vol to mass flow conversion temp
    Real64 constexpr HWInitConvTemp = 60.0;                 // [deg C], standard init hot water vol to mass flow conversion temp
    Real64 constexpr SteamInitConvTemp = 100.0;             // [deg C], standard init steam vol to mass flow conversion temp
    Real64 constexpr StefanBoltzmann = 5.6697E-8;           // Stefan-Boltzmann constant in W/(m2*K4)
    Real64 constexpr UniversalGasConst = 8314.462175;       // Universal Gas Constant (J/mol*K)
    Real64 constexpr convertJtoGJ = 1.0E-9;                 // Conversion factor for J to GJ

    Real64 constexpr MaxCap(1.0e+20); // limit of zone terminal unit capacity

} // namespace Constant

} // namespace EnergyPlus

#endif
