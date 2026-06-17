// EnergyPlus, Copyright (c) 1996-present, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Energy Innovation, LLC, and other
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

#ifndef PollutionModule_hh_INCLUDED
#define PollutionModule_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

namespace Pollution {

    enum class Pollutant
    {
        Invalid = -1,
        CO2,
        CO,
        CH4,
        NOx,
        N2O,
        SO2,
        PM,
        PM10,
        PM2_5,
        NH3,
        NMVOC,
        Hg,
        Pb,
        Water,
        NuclearHigh,
        NuclearLow,
        Num
    };

    constexpr std::array<std::string_view, (int)Pollutant::Num> pollNames = {"CO2",
                                                                             "CO",
                                                                             "CH4",
                                                                             "NOx",
                                                                             "N2O",
                                                                             "SO2",
                                                                             "PM",
                                                                             "PM10",
                                                                             "PM2.5",
                                                                             "NH3",
                                                                             "NMVOC",
                                                                             "Hg",
                                                                             "Pb",
                                                                             "WaterEnvironmentalFactors",
                                                                             "Nuclear High",
                                                                             "Nuclear Low"};

    constexpr std::array<Constant::eResource, (int)Pollutant::Num> poll2Resource = {Constant::eResource::CO2,
                                                                                    Constant::eResource::CO,
                                                                                    Constant::eResource::CH4,
                                                                                    Constant::eResource::NOx,
                                                                                    Constant::eResource::N2O,
                                                                                    Constant::eResource::SO2,
                                                                                    Constant::eResource::PM,
                                                                                    Constant::eResource::PM10,
                                                                                    Constant::eResource::PM2_5,
                                                                                    Constant::eResource::NH3,
                                                                                    Constant::eResource::NMVOC,
                                                                                    Constant::eResource::Hg,
                                                                                    Constant::eResource::Pb,
                                                                                    Constant::eResource::WaterEnvironmentalFactors,
                                                                                    Constant::eResource::NuclearHigh,
                                                                                    Constant::eResource::NuclearLow};

    constexpr std::array<Constant::Units, (int)Pollutant::Num> pollUnits = {
        Constant::Units::kg, // CO2
        Constant::Units::kg, // CO
        Constant::Units::kg, // CH4
        Constant::Units::kg, // NOx
        Constant::Units::kg, // N2O
        Constant::Units::kg, // SO2
        Constant::Units::kg, // PM
        Constant::Units::kg, // PM10
        Constant::Units::kg, // PM2_5
        Constant::Units::kg, // NH3
        Constant::Units::kg, // NMVOC
        Constant::Units::kg, // Hg
        Constant::Units::kg, // Pb
        Constant::Units::L,  // Water
        Constant::Units::kg, // NuclearHigh
        Constant::Units::m3, // NuclearLow
    };

    constexpr std::array<std::string_view, (int)Pollutant::Num> poll2outVarStrs = {
        "CO2 Emissions Mass",             // CO2
        "CO Emissions Mass",              // CO
        "CH4 Emissions Mass",             // CH4
        "NOx Emissions Mass",             // NOx
        "N2O Emissions Mass",             // N2O
        "SO2 Emissions Mass",             // SO2
        "PM Emissions Mass",              // PM
        "PM10 Emissions Mass",            // PM10
        "PM2.5 Emissions Mass",           // PM2_5
        "NH3 Emissions Mass",             // NH3
        "NMVOC Emissions Mass",           // NMVOC
        "Hg Emissions Mass",              // Hg
        "Pb Emissions Mass",              // Pb
        "Water Consumption Volume",       // Water
        "Nuclear High Level Waste Mass",  // NuclearHigh
        "Nuclear Low Level Waste Volume", // NuclearLow
    };

    enum class PollFuel
    {
        Invalid = -1,
        Electricity,
        NaturalGas,
        FuelOil1,
        FuelOil2,
        Coal,
        Gasoline,
        Propane,
        Diesel,
        OtherFuel1,
        OtherFuel2,
        Num
    };

    constexpr std::array<Real64, (int)PollFuel::Num> pollFuelFactors = {
        3.167, // Electricity
        1.084, // NaturalGas
        1.05,  // FuelOil1
        1.05,  // FuelOil2
        1.05,  // Coal
        1.05,  // Gasoline
        1.05,  // Propane
        1.05,  // Diesel
        1.0,   // OtherFuel1
        1.0    // OtherFuel2
    };

    constexpr std::array<PollFuel, (int)Constant::eFuel::Num> fuel2pollFuel = {
        PollFuel::Electricity, // Electricity
        PollFuel::NaturalGas,  // NaturalGas
        PollFuel::Gasoline,    // Gasoline
        PollFuel::Diesel,      // Diesel
        PollFuel::Coal,        // Coal
        PollFuel::Propane,     // Propane
        PollFuel::FuelOil1,    // FuelOilNo1
        PollFuel::FuelOil2,    // FuelOilNo2
        PollFuel::OtherFuel1,  // OtherFuel1
        PollFuel::OtherFuel2,  // OtherFuel2
        PollFuel::Electricity, // DistrictCooling
        PollFuel::NaturalGas,  // DistrictHeatingWater
        PollFuel::NaturalGas,  // DistrictHeatingSteam
    };

    constexpr std::array<Constant::eFuel, (int)PollFuel::Num> pollFuel2fuel = {
        Constant::eFuel::Electricity, // Electricity
        Constant::eFuel::NaturalGas,  // NaturalGas
        Constant::eFuel::FuelOilNo1,  // FuelOil1
        Constant::eFuel::FuelOilNo2,  // FuelOil2
        Constant::eFuel::Coal,        // Coal
        Constant::eFuel::Gasoline,    // Gasoline
        Constant::eFuel::Propane,     // Propane
        Constant::eFuel::Diesel,      // Diesel
        Constant::eFuel::OtherFuel1,  // OtherFuel1
        Constant::eFuel::OtherFuel2   // OtherFuel2
    };

    constexpr std::array<std::string_view, (int)PollFuel::Num> pollFuelNamesUC = {
        Constant::eFuelNamesUC[(int)pollFuel2fuel[(int)PollFuel::Electricity]], // Electricity
        Constant::eFuelNamesUC[(int)pollFuel2fuel[(int)PollFuel::NaturalGas]],  // NaturalGas
        Constant::eFuelNamesUC[(int)pollFuel2fuel[(int)PollFuel::FuelOil1]],    // FuelOil1
        Constant::eFuelNamesUC[(int)pollFuel2fuel[(int)PollFuel::FuelOil2]],    // FuelOil2
        Constant::eFuelNamesUC[(int)pollFuel2fuel[(int)PollFuel::Coal]],        // Coal
        Constant::eFuelNamesUC[(int)pollFuel2fuel[(int)PollFuel::Gasoline]],    // Gasoline
        Constant::eFuelNamesUC[(int)pollFuel2fuel[(int)PollFuel::Propane]],     // Propane
        Constant::eFuelNamesUC[(int)pollFuel2fuel[(int)PollFuel::Diesel]],      // Diesel
        Constant::eFuelNamesUC[(int)pollFuel2fuel[(int)PollFuel::OtherFuel1]],  // OtherFuel1
        Constant::eFuelNamesUC[(int)pollFuel2fuel[(int)PollFuel::OtherFuel2]]   // OtherFuel2
    };

    enum class PollFuelComponent
    {
        Invalid = -1,
        Electricity,
        NaturalGas,
        FuelOil1,
        FuelOil2,
        Coal,
        Gasoline,
        Propane,
        Diesel,
        OtherFuel1,
        OtherFuel2,
        ElectricitySurplusSold,
        ElectricityPurchased,
        Num
    };

    constexpr std::array<PollFuel, (int)PollFuelComponent::Num> pollFuelComp2pollFuel = {PollFuel::Electricity,
                                                                                         PollFuel::NaturalGas,
                                                                                         PollFuel::FuelOil1,
                                                                                         PollFuel::FuelOil2,
                                                                                         PollFuel::Coal,
                                                                                         PollFuel::Gasoline,
                                                                                         PollFuel::Propane,
                                                                                         PollFuel::Diesel,
                                                                                         PollFuel::OtherFuel1,
                                                                                         PollFuel::OtherFuel2,
                                                                                         PollFuel::Electricity,
                                                                                         PollFuel::Electricity};

    constexpr std::array<PollFuelComponent, (int)PollFuel::Num> pollFuel2pollFuelComponent = {
        PollFuelComponent::Electricity,
        PollFuelComponent::NaturalGas,
        PollFuelComponent::FuelOil1,
        PollFuelComponent::FuelOil2,
        PollFuelComponent::Coal,
        PollFuelComponent::Gasoline,
        PollFuelComponent::Propane,
        PollFuelComponent::Diesel,
        PollFuelComponent::OtherFuel1,
        PollFuelComponent::OtherFuel2,
    };

    enum class PollFacilityMeter
    {
        Invalid = -1,
        Electricity,
        NaturalGas,
        FuelOil1,
        FuelOil2,
        Coal,
        Gasoline,
        Propane,
        Diesel,
        OtherFuel1,
        OtherFuel2,
        ElectricitySurplusSold,
        ElectricityPurchased,
        ElectricityProduced,
        Steam,
        HeatPurchased,
        CoolPurchased,
        Num
    };

    constexpr std::array<std::string_view, (int)PollFacilityMeter::Num> pollFacilityMeterNames = {"Electricity:Facility",
                                                                                                  "NaturalGas:Facility",
                                                                                                  "FuelOilNo1:Facility",
                                                                                                  "FuelOilNo2:Facility",
                                                                                                  "Coal:Facility",
                                                                                                  "Gasoline:Facility",
                                                                                                  "Propane:Facility",
                                                                                                  "Diesel:Facility",
                                                                                                  "OtherFuel1:Facility",
                                                                                                  "OtherFuel2:Facility",
                                                                                                  "ElectricitySurplusSold:Facility",
                                                                                                  "ElectricityPurchased:Facility",
                                                                                                  "ElectricityProduced:Facility",
                                                                                                  "DistrictHeatingSteam:Facility",
                                                                                                  "DistrictHeatingWater:Facility",
                                                                                                  "DistrictCooling:Facility"};

    struct ComponentProps
    {
        Real64 sourceVal = 0.0;
        std::array<Real64, (int)Pollutant::Num> pollutantVals = {0.0};
    };

    struct CoefficientProps
    {
        bool used = false;
        Real64 sourceCoeff = 0.0;
        std::array<Real64, (int)Pollutant::Num> pollutantCoeffs = {0.0};
        Sched::Schedule *sourceSched = nullptr;
        std::array<Sched::Schedule *, (int)Pollutant::Num> pollutantScheds = {0};
    };

    void CalculatePollution(EnergyPlusData &state);

    void SetupPollutionCalculations(EnergyPlusData &state);

    void GetPollutionFactorInput(EnergyPlusData &state);

    void SetupPollutionMeterReporting(EnergyPlusData &state);

    void CheckPollutionMeterReporting(EnergyPlusData &state);

    void CalcPollution(EnergyPlusData &state);

    void ReadEnergyMeters(EnergyPlusData &state);

    void
    GetFuelFactorInfo(EnergyPlusData &state,
                      Constant::eFuel fuel,         // input fuel name  (standard from Tabular reports)
                      bool &fuelFactorUsed,         // return value true if user has entered this fuel
                      Real64 &fuelSourceFactor,     // if used, the source factor
                      bool &fuelFactorScheduleUsed, // if true, schedules for this fuel are used
                      Sched::Schedule **ffSched // if schedules for this fuel are used, return schedule pointer (need pointer to pointer to do this)
    );

    void GetEnvironmentalImpactFactorInfo(EnergyPlusData &state,
                                          Real64 &efficiencyDistrictHeatingWater,  // if entered, the efficiency of District Heating Water
                                          Real64 &efficiencyDistrictCooling,       // if entered, the efficiency of District Cooling
                                          Real64 &sourceFactorDistrictHeatingSteam // if entered, the source factor for Dictrict Heating Steam
    );

} // namespace Pollution

struct PollutionData : BaseGlobalStruct
{

    bool PollutionReportSetup = false;
    bool GetInputFlagPollution = true;
    int NumEnvImpactFactors = 0;
    int NumFuelFactors = 0;

    std::array<Pollution::ComponentProps, (int)Pollution::PollFuelComponent::Num> pollComps;

    // Meters, meter values, and grouped meter values
    std::array<int, (int)Pollution::PollFacilityMeter::Num> facilityMeterNums = {-1};
    std::array<Real64, (int)Pollution::PollFacilityMeter::Num> facilityMeterVals = {0.0};
    std::array<Real64, (int)Pollution::PollFuelComponent::Num> facilityMeterFuelComponentVals = {0.0};

    std::array<Real64, (int)Pollution::Pollutant::Num> pollutantVals = {0.0};

    std::vector<Pollution::PollFuel> pollFuelFactorList;

    // Total Carbon Equivalent Components
    Real64 TotCarbonEquivFromN2O = 0.0;
    Real64 TotCarbonEquivFromCH4 = 0.0;
    Real64 TotCarbonEquivFromCO2 = 0.0;
    // Fuel Type Coefficients
    std::array<Pollution::CoefficientProps, (int)Pollution::PollFuel::Num> pollCoeffs;
    // Total Carbon Equivalent Coeffs
    Real64 CarbonEquivN2O = 0.0;
    Real64 CarbonEquivCH4 = 0.0;
    Real64 CarbonEquivCO2 = 0.0;
    Real64 PurchHeatEffic = 0.0;
    Real64 PurchCoolCOP = 0.0;
    Real64 SteamConvEffic = 0.0;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->PollutionReportSetup = false;
        this->GetInputFlagPollution = true;
        this->NumEnvImpactFactors = 0;
        this->NumFuelFactors = 0;

        this->pollFuelFactorList.clear();
    }
};

} // namespace EnergyPlus

#endif
