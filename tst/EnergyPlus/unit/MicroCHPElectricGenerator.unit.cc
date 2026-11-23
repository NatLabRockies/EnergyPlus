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

// EnergyPlus::FuelCellElectricGenerator Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// C++ Headers
#include <memory>
#include <vector>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataGenerators.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/MicroCHPElectricGenerator.hh>
#include <EnergyPlus/Plant/PlantManager.hh>

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, MicroCHPTest_InitGeneratorDynamics)
{

    std::string const idf_objects = delimited_string({
        "ElectricLoadCenter:Distribution,",
        "Electric Load Center Distribution 1,    !- Name",
        "Electric Load Center Distribution 1 Generators, !- Generator List Name",
        "TrackSchedule,                          !- Generator Operation Scheme Type",
        "0,                                      !- Generator Demand Limit Scheme Purchased Electric Demand Limit {W}",
        "Generator Demand,                       !- Generator Track Schedule Name Scheme Schedule Name",
        ",                                       !- Generator Track Meter Scheme Meter Name",
        "AlternatingCurrent;                     !- Electrical Buss Type",

        "ElectricLoadCenter:Generators,",
        "Electric Load Center Distribution 1 Generators, !- Name",
        "Generator Fuel Cell 1,                  !- Generator Name 1",
        "Generator:FuelCell,                     !- Generator Object Type 1",
        "3400,                                   !- Generator Rated Electric Power Output 1 {W}",
        ",                                       !- Generator Availability Schedule Name 1",
        ";                                       !- Generator Rated Thermal to Electrical Power Ratio 1",

        "Generator:MicroCHP,",
        "MicroCoGen1,             !- Name",
        "SENERTECH5_5KW2,         !- Performance Parameters Name",
        ",                        !- Zone Name",
        "MICROCHP SENERTECH Pump-MicroCoGen1Node,  !- Cooling Water Inlet Node Name",
        "MICROCHP1 SENERTECH Supply Equipment Outlet Node,  !- Cooling Water Outlet Node Name",
        "MicroCoGen1 air inlet node,  !- Air Inlet Node Name",
        ",                        !- Air Outlet Node Name",
        "NATURALGAS2,             !- Generator Fuel Supply Name",
        ";                        !- Availability Schedule Name",
        "",
        "",
        "Generator:MicroCHP,",
        "MicroCoGen2,             !- Name",
        "SENERTECH5_5KW2,         !- Performance Parameters Name",
        ",                        !- Zone Name",
        "MICROCHP SENERTECH Pump-MicroCoGen2Node,  !- Cooling Water Inlet Node Name",
        "MICROCHP2 SENERTECH Supply Equipment Outlet Node,  !- Cooling Water Outlet Node Name",
        "MicroCoGen2 air inlet node,  !- Air Inlet Node Name",
        ",                        !- Air Outlet Node Name",
        "NATURALGAS2,             !- Generator Fuel Supply Name",
        ";                        !- Availability Schedule Name",
        "",
        "Generator:MicroCHP:NonNormalizedParameters,",
        "SENERTECH5_5KW2,         !- Name",
        "5500,                    !- Maximum Electric Power {W}",
        "0.0000,                  !- Minimum Electric Power {W}",
        "0.0000,                  !- Minimum Cooling Water Flow Rate {kg/s}",
        "80.0000,                 !- Maximum Cooling Water Temperature {C}",
        "SenerTechElEff,          !- Electrical Efficiency Curve Name",
        "SenerTechThermEff,       !- Thermal Efficiency Curve Name",
        "InternalControl,         !- Cooling Water Flow Rate Mode",
        "SenerTechCoolWaterflow,  !- Cooling Water Flow Rate Curve Name",
        "SenerTechAirFlow,        !- Air Flow Rate Curve Name",
        "1000000000.0000,         !- Maximum Net Electrical Power Rate of Change {W/s}",
        "1000000000.0000,         !- Maximum Fuel Flow Rate of Change {kg/s2}",
        "741.0000,                !- Heat Exchanger U-Factor Times Area Value {W/K}",
        "13.7000,                 !- Skin Loss U-Factor Times Area Value {W/K}",
        "0.5000,                  !- Skin Loss Radiative Fraction",
        "63605.6000,              !- Aggregated Thermal Mass of Energy Conversion Portion of Generator {W/K}",
        "1000.7000,               !- Aggregated Thermal Mass of Heat Recovery Portion of Generator {W/K}",
        "0.0000,                  !- Standby Power {W}",
        "TimeDelay,               !- Warm Up Mode",
        ",                        !- Warm Up Fuel Flow Rate Coefficient",
        ",                        !- Nominal Engine Operating Temperature {C}",
        ",                        !- Warm Up Power Coefficient",
        ",                        !- Warm Up Fuel Flow Rate Limit Ratio",
        "60.0000,                 !- Warm Up Delay Time {s}",
        "0.0000,                  !- Cool Down Power {W}",
        "60.0000,                 !- Cool Down Delay Time {s}",
        "OptionalCoolDown;        !- Restart Mode",
        "",
        "Generator:FuelSupply,",
        "NATURALGAS2,             !- Name",
        "TemperatureFromAirNode,  !- Fuel Temperature Modeling Mode",
        "MicroCoGen2 air inlet node,  !- Fuel Temperature Reference Node Name",
        ",                        !- Fuel Temperature Schedule Name",
        "NullCubic,               !- Compressor Power Multiplier Function of Fuel Rate Curve Name",
        "1.0000,                  !- Compressor Heat Loss Factor",
        "GaseousConstituents,     !- Fuel Type",
        ",                        !- Liquid Generic Fuel Lower Heating Value {kJ/kg}",
        ",                        !- Liquid Generic Fuel Higher Heating Value {kJ/kg}",
        ",                        !- Liquid Generic Fuel Molecular Weight {g/mol}",
        ",                        !- Liquid Generic Fuel CO2 Emission Factor",
        "8,                       !- Number of Constituents in Gaseous Constituent Fuel Supply",
        "METHANE,                 !- Constituent 1 Name",
        "0.9490,                  !- Constituent 1 Molar Fraction",
        "CarbonDioxide,           !- Constituent 2 Name",
        "0.0070,                  !- Constituent 2 Molar Fraction",
        "NITROGEN,                !- Constituent 3 Name",
        "0.0160,                  !- Constituent 3 Molar Fraction",
        "ETHANE,                  !- Constituent 4 Name",
        "0.0250,                  !- Constituent 4 Molar Fraction",
        "PROPANE,                 !- Constituent 5 Name",
        "0.0020,                  !- Constituent 5 Molar Fraction",
        "BUTANE,                  !- Constituent 6 Name",
        "0.0006,                  !- Constituent 6 Molar Fraction",
        "PENTANE,                 !- Constituent 7 Name",
        "0.0002,                  !- Constituent 7 Molar Fraction",
        "OXYGEN,                  !- Constituent 8 Name",
        "0.0002;                  !- Constituent 8 Molar Fraction",
        "  Schedule:Compact,",
        "    ALWAYS_ON,               !- Name",
        "    On/Off,                  !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1;          !- Field 3",
        "Curve:Cubic,",
        "    NullCubic,               !- Name",
        "    0.0000,                  !- Coefficient1 Constant",
        "    0.0000,                  !- Coefficient2 x",
        "    0.0000,                  !- Coefficient3 x * *2",
        "    0.0000,                  !- Coefficient4 x * *3",
        "    0.0000,                  !- Minimum Value of x",
        "    0.0000;                  !- Maximum Value of x",
        "Curve:Quadratic,",
        "SenerTechAirFlow,        !- Name",
        "0.0000,                  !- Coefficient1 Constant",
        "2.0000,                  !- Coefficient2 x",
        "-10000.0000,             !- Coefficient3 x**2",
        "0.0000,                  !- Minimum Value of x",
        "1000000000000.0000;      !- Maximum Value of x",
        "",
        "Curve:Biquadratic,",
        "SenerTechCoolWaterflow,  !- Name",
        "0.4000,                  !- Coefficient1 Constant",
        "0.0000,                  !- Coefficient2 x",
        "0.0000,                  !- Coefficient3 x**2",
        "0.0000,                  !- Coefficient4 y",
        "0.0000,                  !- Coefficient5 y**2",
        "0.0000,                  !- Coefficient6 x*y",
        "0.0000,                  !- Minimum Value of x",
        "1.0E12,                  !- Maximum Value of x",
        "0.0000,                  !- Minimum Value of y",
        "1.0E12;                  !- Maximum Value of y",
        "",
        "Curve:Triquadratic,",
        "SenerTechThermEff,       !- Name",
        "0.6600,                  !- Coefficient1 Constant",
        "0.0000,                  !- Coefficient2 x**2",
        "0.0000,                  !- Coefficient3 x",
        "0.0000,                  !- Coefficient4 y**2",
        "0.0000,                  !- Coefficient5 y",
        "0.0000,                  !- Coefficient6 z**2",
        "0.0000,                  !- Coefficient7 z",
        "0.0000,                  !- Coefficient8 x**2*y**2",
        "0.0000,                  !- Coefficient9 x*y",
        "0.0000,                  !- Coefficient10 x*y**2",
        "0.0000,                  !- Coefficient11 x**2*y",
        "0.0000,                  !- Coefficient12 x**2*z**2",
        "0.0000,                  !- Coefficient13 x*z",
        "0.0000,                  !- Coefficient14 x*z**2",
        "0.0000,                  !- Coefficient15 x**2*z",
        "0.0000,                  !- Coefficient16 y**2*z**2",
        "0.0000,                  !- Coefficient17 y*z",
        "0.0000,                  !- Coefficient18 y*z**2",
        "0.0000,                  !- Coefficient19 y**2*z",
        "0.0000,                  !- Coefficient20 x**2*y**2*z**2",
        "0.0000,                  !- Coefficient21 x**2*y**2*z",
        "0.0000,                  !- Coefficient22 x**2*y*z**2",
        "0.0000,                  !- Coefficient23 x*y**2*z**2",
        "0.0000,                  !- Coefficient24 x**2*y*z",
        "0.0000,                  !- Coefficient25 x*y**2*z",
        "0.0000,                  !- Coefficient26 x*y*z**2",
        "0.0000,                  !- Coefficient27 x*y*z",
        "0.0000,                  !- Minimum Value of x",
        "1000000000.0000,         !- Maximum Value of x",
        "0.0000,                  !- Minimum Value of y",
        "1000000000.0000,         !- Maximum Value of y",
        "0.0000,                  !- Minimum Value of z",
        "1000000000.0000;         !- Maximum Value of z",
        "",
        "Curve:Triquadratic,",
        "SenerTechElEff,          !- Name",
        "0.2700,                  !- Coefficient1 Constant",
        "0.0000,                  !- Coefficient2 x**2",
        "0.0000,                  !- Coefficient3 x",
        "0.0000,                  !- Coefficient4 y**2",
        "0.0000,                  !- Coefficient5 y",
        "0.0000,                  !- Coefficient6 z**2",
        "0.0000,                  !- Coefficient7 z",
        "0.0000,                  !- Coefficient8 x**2*y**2",
        "0.0000,                  !- Coefficient9 x*y",
        "0.0000,                  !- Coefficient10 x*y**2",
        "0.0000,                  !- Coefficient11 x**2*y",
        "0.0000,                  !- Coefficient12 x**2*z**2",
        "0.0000,                  !- Coefficient13 x*z",
        "0.0000,                  !- Coefficient14 x*z**2",
        "0.0000,                  !- Coefficient15 x**2*z",
        "0.0000,                  !- Coefficient16 y**2*z**2",
        "0.0000,                  !- Coefficient17 y*z",
        "0.0000,                  !- Coefficient18 y*z**2",
        "0.0000,                  !- Coefficient19 y**2*z",
        "0.0000,                  !- Coefficient20 x**2*y**2*z**2",
        "0.0000,                  !- Coefficient21 x**2*y**2*z",
        "0.0000,                  !- Coefficient22 x**2*y*z**2",
        "0.0000,                  !- Coefficient23 x*y**2*z**2",
        "0.0000,                  !- Coefficient24 x**2*y*z",
        "0.0000,                  !- Coefficient25 x*y**2*z",
        "0.0000,                  !- Coefficient26 x*y*z**2",
        "0.0000,                  !- Coefficient27 x*y*z",
        "0.0000,                  !- Minimum Value of x",
        "1000000000.0000,         !- Maximum Value of x",
        "0.0000,                  !- Minimum Value of y",
        "1000000000.0000,         !- Maximum Value of y",
        "0.0000,                  !- Minimum Value of z",
        "1000000000.0000;         !- Maximum Value of z",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);

    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);

    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(2);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 2;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(2).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(2).Comp.allocate(1);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";

    auto &MicroCHP1(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1));
    auto &MicroCHP2(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(2).Comp(1));
    MicroCHP1.TypeOf = "GENERATOR:MICROCHP";
    MicroCHP1.Type = DataPlant::PlantEquipmentType::Generator_MicroCHP;
    MicroCHP1.Name = "MICROCOGEN1";
    MicroCHP2.TypeOf = "GENERATOR:MICROCHP";
    MicroCHP2.Type = DataPlant::PlantEquipmentType::Generator_MicroCHP;
    MicroCHP2.Name = "MICROCOGEN2";
    MicroCHP1.compPtr = MicroCHPElectricGenerator::MicroCHPDataStruct::factory(*state, "MICROCOGEN1");
    MicroCHP2.compPtr = MicroCHPElectricGenerator::MicroCHPDataStruct::factory(*state, "MICROCOGEN2");
    EXPECT_EQ(state->dataCHPElectGen->NumMicroCHPs, 2);

    bool FirstHVACIteration = true;
    bool GetCompSizFac = false;
    dynamic_cast<MicroCHPElectricGenerator::MicroCHPDataStruct *>(MicroCHP1.compPtr)->InitMicroCHPNoNormalizeGenerators(*state);
    dynamic_cast<MicroCHPElectricGenerator::MicroCHPDataStruct *>(MicroCHP2.compPtr)->InitMicroCHPNoNormalizeGenerators(*state);
    MicroCHP1.initLoopEquip(*state, GetCompSizFac);
    MicroCHP1.simulate(*state, FirstHVACIteration);
    MicroCHP2.initLoopEquip(*state, GetCompSizFac);
    MicroCHP2.simulate(*state, FirstHVACIteration);
    EXPECT_EQ(state->dataGenerator->GeneratorDynamics(1).Name, MicroCHP1.Name);
    EXPECT_EQ(state->dataGenerator->GeneratorDynamics(2).Name, MicroCHP2.Name);
}
