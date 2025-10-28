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
// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHVACSystems.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/HVACHXAssistedCoolingCoil.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/SizingManager.hh>
#include <EnergyPlus/UnitarySystem.hh>

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, HXAssistCCUnitarySystem_VStest1)
{

    bool ErrorsFound(false);
    bool FirstHVACIteration(false);
    Real64 Qsens_sys(0.0); // UnitarySystem delivered sensible capacity wrt zone
    Real64 ZoneTemp(0.0);  // control zone temperature
    int InletNode(0);      // UnitarySystem inlet node number
    int OutletNode(0);     // UnitarySystem outlet node number
    int ControlZoneNum(0); // index to control zone

    std::string const idf_objects = delimited_string({
        "Zone,",
        "  EAST ZONE,              !- Name",
        "  0,                      !- Direction of Relative North{ deg }",
        "  0,                      !- X Origin{ m }",
        "  0,                      !- Y Origin{ m }",
        "  0,                      !- Z Origin{ m }",
        "  1,                      !- Type",
        "  1,                      !- Multiplier",
        "  autocalculate,          !- Ceiling Height{ m }",
        "  autocalculate;          !- Volume{ m3 }",
        "  ",
        "ZoneHVAC:EquipmentConnections,",
        "EAST ZONE,                 !- Zone Name",
        "  Zone2Equipment,          !- Zone Conditioning Equipment List Name",
        "  Zone 2 Inlet Node,       !- Zone Air Inlet Node or NodeList Name",
        "  Zone Exhaust Node,       !- Zone Air Exhaust Node or NodeList Name",
        "  Zone 2 Node,             !- Zone Air Node Name",
        "  Zone 2 Outlet Node;      !- Zone Return Air Node Name",
        "  ",
        "ZoneHVAC:EquipmentList,",
        "  Zone2Equipment,          !- Name",
        "  SequentialLoad,          !- Load Distribution Scheme",
        "  AirLoopHVAC:UnitarySystem, !- Zone Equipment 1 Object Type",
        "  GasHeat DXAC Furnace 1,          !- Zone Equipment 1 Name",
        "  1,                       !- Zone Equipment 1 Cooling Sequence",
        "  1;                       !- Zone Equipment 1 Heating or No - Load Sequence",
        "  ",
        "AirLoopHVAC:UnitarySystem,",
        "  GasHeat DXAC Furnace 1, !- Name",
        "  Load,                   !- Control Type",
        "  East Zone,              !- Controlling Zone or Thermostat Location",
        "  None,                   !- Dehumidification Control Type",
        "  FanAndCoilAvailSched,   !- Availability Schedule Name",
        "  Zone Exhaust Node,         !- Air Inlet Node Name",
        "  Zone 2 Inlet Node,   !- Air Outlet Node Name",
        "  Fan:OnOff,              !- Supply Fan Object Type",
        "  Supply Fan 1,           !- Supply Fan Name",
        "  BlowThrough,            !- Fan Placement",
        "  ContinuousFanSchedule,  !- Supply Air Fan Operating Mode Schedule Name",
        "  Coil:Heating:Fuel,       !- Heating Coil Object Type",
        "  Furnace Heating Coil 1, !- Heating Coil Name",
        "  ,                       !- DX Heating Coil Sizing Ratio",
        "  CoilSystem:Cooling:DX:HeatExchangerAssisted, !- Cooling Coil Object Type",
        "  Heat Exchanger Assisted Cooling Coil 1,     !- Cooling Coil Name",
        "  ,                       !- Use DOAS DX Cooling Coil",
        "  ,                       !- DOAS DX Cooling Coil Leaving Minimum Air Temperature{ C }",
        "  ,                       !- Latent Load Control",
        "  Coil:Heating:Fuel,       !- Supplemental Heating Coil Object Type",
        "  Humidistat Reheat Coil 1, !- Supplemental Heating Coil Name",
        "  SupplyAirFlowRate,      !- Supply Air Flow Rate Method During Cooling Operation",
        "  1.6,                    !- Supply Air Flow Rate During Cooling Operation{ m3/s }",
        "  ,                       !- Supply Air Flow Rate Per Floor Area During Cooling Operation{ m3/s-m2 }",
        "  ,                       !- Fraction of Autosized Design Cooling Supply Air Flow Rate",
        "  ,                       !- Design Supply Air Flow Rate Per Unit of Capacity During Cooling Operation{ m3/s-W }",
        "  SupplyAirFlowRate,      !- Supply air Flow Rate Method During Heating Operation",
        "  1.6,                    !- Supply Air Flow Rate During Heating Operation{ m3/s }",
        "  ,                       !- Supply Air Flow Rate Per Floor Area during Heating Operation{ m3/s-m2 }",
        "  ,                       !- Fraction of Autosized Design Heating Supply Air Flow Rate",
        "  ,                       !- Design Supply Air Flow Rate Per Unit of Capacity During Heating Operation{ m3/s-W }",
        "  SupplyAirFlowRate,      !- Supply Air Flow Rate Method When No Cooling or Heating is Required",
        "  1.6,                    !- Supply Air Flow Rate When No Cooling or Heating is Required{ m3/s }",
        "  ,                       !- Supply Air Flow Rate Per Floor Area When No Cooling or Heating is Required{ m3/s-m2 }",
        "  ,                       !- Fraction of Autosized Design Cooling Supply Air Flow Rate",
        "  ,                       !- Fraction of Autosized Design Heating Supply Air Flow Rate",
        "  ,                       !- Design Supply Air Flow Rate Per Unit of Capacity During Cooling Operation{ m3/s-W }",
        "  ,                       !- Design Supply Air Flow Rate Per Unit of Capacity During Heating Operation{ m3/s-W }",
        "    ,                     !- No Load Supply Air Flow Rate Control Set To Low Speed",
        "  80;                     !- Maximum Supply Air Temperature{ C }",

        "  CoilSystem:Cooling:DX:HeatExchangerAssisted,",
        "    Heat Exchanger Assisted Cooling Coil 1,  !- Name",
        "    HeatExchanger:AirToAir:SensibleAndLatent,  !- Heat Exchanger Object Type",
        "    Heat Exchanger Assisted DX 1,  !- Heat Exchanger Name",
        "    Coil:Cooling:DX:VariableSpeed,  !- Cooling Coil Object Type",
        "    Main Cooling Coil 1;      !- Cooling Coil Name",

        "  HeatExchanger:AirToAir:SensibleAndLatent,",
        "    Heat Exchanger Assisted DX 1,      !- Name",
        "    ,   !- Availability Schedule Name",
        "    1.6,                !- Nominal Supply Air Flow Rate {m3/s}",
        "    0.7,                     !- Sensible Effectiveness at 100% Heating Air Flow {dimensionless}",
        "    0.65,                    !- Latent Effectiveness at 100% Heating Air Flow {dimensionless}",
        "    0.7,                     !- Sensible Effectiveness at 100% Cooling Air Flow {dimensionless}",
        "    0.65,                    !- Latent Effectiveness at 100% Cooling Air Flow {dimensionless}",
        "    DX Cooling Coil Air Inlet Node,   !- Supply Air Inlet Node Name",
        "    Heat Recovery Supply Outlet,  !- Supply Air Outlet Node Name",
        "    Heat Recovery Exhuast Inlet Node,  !- Exhaust Air Inlet Node Name",
        "    Heating Coil Air Inlet Node,  !- Exhaust Air Outlet Node Name",
        "    0,                       !- Nominal Electric Power {W}",
        "    No,                     !- Supply Air Outlet Temperature Control",
        "    Plate,                   !- Heat Exchanger Type",
        "    MinimumExhaustTemperature,  !- Frost Control Type",
        "    1.7,                     !- Threshold Temperature {C}",
        "    0.083,                   !- Initial Defrost Time Fraction {dimensionless}",
        "    0.012,                   !- Rate of Defrost Time Fraction Increase {1/K}",
        "    Yes,                     !- Economizer Lockout",
        "    SenEffectivenessTable,   !- Sensible Effectiveness of Heating Air Flow Curve Name",
        "    LatEffectivenessTable,   !- Latent Effectiveness of Heating Air Flow Curve Name",
        "    SenEffectivenessTable,   !- Sensible Effectiveness of Cooling Air Flow Curve Name",
        "    LatEffectivenessTable;   !- Latent Effectiveness of Cooling Air Flow Curve Name",

        "  Table:IndependentVariable,",
        "    airFlowRatio,  !- Name",
        "    Linear,                  !- Interpolation Method",
        "    Linear,                  !- Extrapolation Method",
        "    0.0,                     !- Minimum Value",
        "    1.0,                     !- Maximum Value",
        "    ,                        !- Normalization Reference Value",
        "    Dimensionless,           !- Unit Type",
        "    ,                        !- External File Name",
        "    ,                        !- External File Column Number",
        "    ,                        !- External File Starting Row Number",
        "    0.75,                    !- Value 1",
        "    1.0;                     !- Value 2",

        "  Table:IndependentVariableList,",
        "    effectiveness_IndependentVariableList,  !- Name",
        "    airFlowRatio;     !- Independent Variable 1 Name",

        "  Table:Lookup,",
        "    SenEffectivenessTable,   !- Name",
        "    effectiveness_IndependentVariableList,  !- Independent Variable List Name",
        "    DivisorOnly,             !- Normalization Method",
        "    0.7,                     !- Normalization Divisor",
        "    0.0,                     !- Minimum Output",
        "    1.0,                     !- Maximum Output",
        "    Dimensionless,           !- Output Unit Type",
        "    ,                        !- External File Name",
        "    ,                        !- External File Column Number",
        "    ,                        !- External File Starting Row Number",
        "    0.75,                    !- Output Value 1",
        "    0.70;                    !- Output Value 2",

        "  Table:Lookup,",
        "    LatEffectivenessTable,   !- Name",
        "    effectiveness_IndependentVariableList,  !- Independent Variable List Name",
        "    DivisorOnly,             !- Normalization Method",
        "    0.65,                    !- Normalization Divisor",
        "    0.0,                     !- Minimum Output",
        "    1.0,                     !- Maximum Output",
        "    Dimensionless,           !- Output Unit Type",
        "    ,                        !- External File Name",
        "    ,                        !- External File Column Number",
        "    ,                        !- External File Starting Row Number",
        "    0.70,                    !- Output Value 1",
        "    0.65;                    !- Output Value 2",

        "  Coil:Cooling:DX:VariableSpeed,",
        "    Main Cooling Coil 1,    !- Name",
        "    ,                       !- Availability Schedule Name",
        "    Heat Recovery Supply Outlet,  !- Indoor Air Inlet Node Name",
        "    Heat Recovery Exhuast Inlet Node,  !- Indoor Air Outlet Node Name",
        "    1,                       !- Number of Speeds {dimensionless}",
        "    1,                       !- Nominal Speed Level {dimensionless}",
        "    32000.0,                 !- Gross Rated Total Cooling Capacity At Selected Nominal Speed Level {w}",
        "    1.6,                     !- Rated Air Flow Rate At Selected Nominal Speed Level {m3/s}",
        "    0.0,                     !- Nominal Time for Condensate to Begin Leaving the Coil {s}",
        "    0.0,                     !- Initial Moisture Evaporation Rate Divided by Steady-State AC Latent Capacity {dimensionless}",
        "    ,                        !- Maximum Cycling Rate",
        "    ,                        !- Latent Capacity Time Constant",
        "    ,                        !- Fan Delay Time",
        "    HPACCOOLPLFFPLR,         !- Energy Part Load Fraction Curve Name",
        "    ,                        !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "    0.0,                     !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    36991.44197,             !- Speed 1 Reference Unit Gross Rated Total Cooling Capacity {w}",
        "    0.75,                    !- Speed 1 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3.866381837,             !- Speed 1 Reference Unit Gross Rated Cooling COP {dimensionless}",
        "    3.776,                   !- Speed 1 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                   !- Speed 7 2017 Rated Evaporator Fan Power Per Volume Flow Rate",
        "    934.4,                   !- Speed 7 2023 Rated Evaporator Fan Power Per Volume Flow Rate",
        "    10.62,                   !- Speed 1 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 Reference Unit Rated Pad Effectiveness of Evap Precooling {dimensionless}",
        "    HPCoolingCAPFTemp4,      !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACFFF,                 !- Speed 1 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    HPCoolingEIRFTemp4,      !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACFFF;                 !- Speed 1 Energy Input Ratio Function of Air Flow Fraction Curve Name",

        "   Curve:Quadratic,",
        "    HPACCOOLPLFFPLR,         !- Name",
        "    1.0,                    !- Coefficient1 Constant",
        "    0.0,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x  ",

        "  Curve:Cubic,",
        "    HPACFFF,                 !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Biquadratic,",
        "    HPCoolingEIRFTemp4,      !- Name",
        "    0.0001514017,            !- Coefficient1 Constant",
        "    0.0655062896,            !- Coefficient2 x",
        "    -0.0020370821,           !- Coefficient3 x**2",
        "    0.0067823041,            !- Coefficient4 y",
        "    0.0004087196,            !- Coefficient5 y**2",
        "    -0.0003552302,           !- Coefficient6 x*y",
        "    13.89,                   !- Minimum Value of x",
        "    22.22,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    51.67,                   !- Maximum Value of y",
        "    0.5141,                  !- Minimum Curve Output",
        "    1.7044,                  !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    HPCoolingCAPFTemp4,      !- Name",
        "    1.3544202152,            !- Coefficient1 Constant",
        "    -0.0493402773,           !- Coefficient2 x",
        "    0.0022649843,            !- Coefficient3 x**2",
        "    0.0008517727,            !- Coefficient4 y",
        "    -0.0000426316,           !- Coefficient5 y**2",
        "    -0.0003364517,           !- Coefficient6 x*y",
        "    13.89,                   !- Minimum Value of x",
        "    22.22,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    51.67,                   !- Maximum Value of y",
        "    0.7923,                  !- Minimum Curve Output",
        "    1.2736,                  !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  OutdoorAir:Node,",
        "    Main Cooling Coil 1 Condenser Node,  !- Name",
        "    -1.0;                    !- Height Above Ground {m}",

        "Fan:OnOff,",
        "  Supply Fan 1,           !- Name",
        "  FanAndCoilAvailSched,   !- Availability Schedule Name",
        "  0.7,                    !- Fan Total Efficiency",
        "  600.0,                  !- Pressure Rise{ Pa }",
        "  1.6,                    !- Maximum Flow Rate{ m3 / s }",
        "  0.9,                    !- Motor Efficiency",
        "  1.0,                    !- Motor In Airstream Fraction",
        "  Zone Exhaust Node,      !- Air Inlet Node Name",
        "  DX Cooling Coil Air Inlet Node;  !- Air Outlet Node Name",

        "Coil:Heating:Fuel,",
        "  Furnace Heating Coil 1, !- Name",
        "  FanAndCoilAvailSched,   !- Availability Schedule Name",
        "  NaturalGas,              !- Fuel Type",
        "  0.8,                    !- Gas Burner Efficiency",
        "  32000,                  !- Nominal Capacity{ W }",
        "  Heating Coil Air Inlet Node, !- Air Inlet Node Name",
        "  Reheat Coil Air Inlet Node;  !- Air Outlet Node Name",
        "  ",
        "Coil:Heating:Fuel,",
        "  Humidistat Reheat Coil 1, !- Name",
        "  FanAndCoilAvailSched, !- Availability Schedule Name",
        "  NaturalGas,              !- Fuel Type",
        "  0.8, !- Gas Burner Efficiency",
        "  32000, !- Nominal Capacity{ W }",
        "  Reheat Coil Air Inlet Node, !- Air Inlet Node Name",
        "  Zone 2 Inlet Node;    !- Air Outlet Node Name",
        "  ",
        "ScheduleTypeLimits,",
        "  Any Number;             !- Name",
        "  ",
        "Schedule:Compact,",
        "  FanAndCoilAvailSched,   !- Name",
        "  Any Number,             !- Schedule Type Limits Name",
        "  Through: 12/31,         !- Field 1",
        "  For: AllDays,           !- Field 2",
        "  Until: 24:00, 1.0;      !- Field 3",
        "  ",
        "Schedule:Compact,",
        "  ContinuousFanSchedule,  !- Name",
        "  Any Number,             !- Schedule Type Limits Name",
        "  Through: 12/31,         !- Field 1",
        "  For: AllDays,           !- Field 2",
        "  Until: 24:00, 1.0;      !- Field 3",
        "  ",
        "Curve:Quadratic,",
        "  CoolCapFFF,       !- Name",
        "  0.8,                    !- Coefficient1 Constant",
        "  0.2,                    !- Coefficient2 x",
        "  0.0,                    !- Coefficient3 x**2",
        "  0.5,                    !- Minimum Value of x",
        "  1.5;                    !- Maximum Value of x",
        "  ",
        "Curve:Quadratic,",
        "  COOLEIRFFF,           !- Name",
        "  1.1552,                 !- Coefficient1 Constant",
        "  -0.1808,                !- Coefficient2 x",
        "  0.0256,                 !- Coefficient3 x**2",
        "  0.5,                    !- Minimum Value of x",
        "  1.5;                    !- Maximum Value of x",
        "  ",
        "Curve:Quadratic,",
        "  PLFFPLR,          !- Name",
        "  0.85,                   !- Coefficient1 Constant",
        "  0.15,                   !- Coefficient2 x",
        "  0.0,                    !- Coefficient3 x**2",
        "  0.0,                    !- Minimum Value of x",
        "  1.0;                    !- Maximum Value of x",
        "  ",
        "Curve:Biquadratic,",
        "  CoolCapFT,        !- Name",
        "  0.942587793,            !- Coefficient1 Constant",
        "  0.009543347,            !- Coefficient2 x",
        "  0.000683770,            !- Coefficient3 x**2",
        "  -0.011042676,           !- Coefficient4 y",
        "  0.000005249,            !- Coefficient5 y**2",
        "  -0.000009720,           !- Coefficient6 x*y",
        "  12.77778,               !- Minimum Value of x",
        "  23.88889,               !- Maximum Value of x",
        "  18.0,                   !- Minimum Value of y",
        "  46.11111,               !- Maximum Value of y",
        "  ,                       !- Minimum Curve Output",
        "  ,                       !- Maximum Curve Output",
        "  Temperature,            !- Input Unit Type for X",
        "  Temperature,            !- Input Unit Type for Y",
        "  Dimensionless;          !- Output Unit Type",
        "  ",
        "Curve:Biquadratic,",
        "  COOLEIRFT,            !- Name",
        "  0.342414409,            !- Coefficient1 Constant",
        "  0.034885008,            !- Coefficient2 x",
        "  -0.000623700,           !- Coefficient3 x**2",
        "  0.004977216,            !- Coefficient4 y",
        "  0.000437951,            !- Coefficient5 y**2",
        "  -0.000728028,           !- Coefficient6 x*y",
        "  12.77778,               !- Minimum Value of x",
        "  23.88889,               !- Maximum Value of x",
        "  18.0,                   !- Minimum Value of y",
        "  46.11111,               !- Maximum Value of y",
        "  ,                       !- Minimum Curve Output",
        "  ,                       !- Maximum Curve Output",
        "  Temperature,            !- Input Unit Type for X",
        "  Temperature,            !- Input Unit Type for Y",
        "  Dimensionless;          !- Output Unit Type",
    });

    ASSERT_TRUE(process_idf(idf_objects)); // read idf objects
    state->init_state(*state);

    HeatBalanceManager::GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);                            // expect no errors

    DataZoneEquipment::GetZoneEquipmentData(*state); // read zone equipment configuration and list objects

    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataZoneEquip->ZoneEquipList(1).EquipIndex.allocate(1);
    state->dataZoneEquip->ZoneEquipList(1).EquipIndex(1) = 1; // initialize equipment index for ZoneHVAC

    int AirLoopNum = 0;
    int CompIndex = 0;
    bool HeatingActive = false;
    bool CoolingActive = false;
    int OAUnitNum = 0;
    Real64 OAUCoilOutTemp = 0.0;
    std::string compName = "GASHEAT DXAC FURNACE 1";
    bool zoneEquipment = true;
    Real64 sensOut = 0.0;
    Real64 latOut = 0.0;
    UnitarySystems::UnitarySys::factory(*state, HVAC::UnitarySysType::Unitary_AnyCoilType, compName, zoneEquipment, 0);
    UnitarySystems::UnitarySys *thisSys = &state->dataUnitarySystems->unitarySys[0];
    state->dataZoneEquip->ZoneEquipInputsFilled = true;                                  // indicate zone data is available
    thisSys->getUnitarySystemInputData(*state, compName, zoneEquipment, 0, ErrorsFound); // get UnitarySystem input from object above

    ASSERT_EQ(1, state->dataUnitarySystems->numUnitarySystems); // only 1 unitary system above so expect 1 as number of unitary system objects

    // DISABLE SIZING - don't call UnitarySystems::sizeUnitarySystem, much more work needed to set up sizing arrays
    state->dataGlobal->SysSizingCalc = false;

    InletNode = thisSys->AirInNode;
    OutletNode = thisSys->AirOutNode;
    ControlZoneNum = thisSys->NodeNumOfControlledZone;

    // set up unitary system inlet condtions
    state->dataLoopNodes->Node(InletNode).Temp = 26.666667;             // AHRI condition 80F dry-bulb temp
    state->dataLoopNodes->Node(InletNode).HumRat = 0.01117049542334198; // AHRI condition at 80F DB/67F WB lb/lb or kg/kg
    state->dataLoopNodes->Node(InletNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(InletNode).Temp, state->dataLoopNodes->Node(InletNode).HumRat);

    // set zone temperature
    state->dataLoopNodes->Node(ControlZoneNum).Temp = 20.0; // set zone temperature during heating season used to determine system delivered capacity
    state->dataEnvrn->OutDryBulbTemp = 35.0;                // initialize weather
    state->dataEnvrn->OutHumRat = 0.1;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutWetBulbTemp = 30.0;

    // initialize other incidentals that are used within the UnitarySystem module during calculations
    state->dataSize->CurZoneEqNum = 1;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputRequired = 1000.0; // heating load
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputReqToCoolSP = 2000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputReqToHeatSP = 1000.0;
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand(ControlZoneNum).OutputRequiredToDehumidifyingSP = 0.0;

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).SequencedOutputRequired.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).SequencedOutputRequiredToCoolingSP.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).SequencedOutputRequiredToHeatingSP.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand(ControlZoneNum).SequencedOutputRequiredToDehumidSP.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).SequencedOutputRequired(1) =
        state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputRequired;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).SequencedOutputRequiredToCoolingSP(1) =
        state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).OutputRequiredToCoolingSP;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).SequencedOutputRequiredToHeatingSP(1) =
        state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).OutputRequiredToHeatingSP;
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand(ControlZoneNum).SequencedOutputRequiredToDehumidSP(1) =
        state->dataZoneEnergyDemand->ZoneSysMoistureDemand(ControlZoneNum).OutputRequiredToDehumidifyingSP;

    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    Sched::GetSchedule(*state, "FANANDCOILAVAILSCHED")->currentVal = 1.0;
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataEnvrn->StdRhoAir = Psychrometrics::PsyRhoAirFnPbTdbW(*state, 101325.0, 20.0, 0.0); // initialize RhoAir
    state->dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = thisSys->m_MaxCoolAirVolFlow * state->dataEnvrn->StdRhoAir;

    OutputReportPredefined::SetPredefinedTables(*state);
    thisSys->simulate(*state,
                      compName,
                      FirstHVACIteration,
                      AirLoopNum,
                      CompIndex,
                      HeatingActive,
                      CoolingActive,
                      OAUnitNum,
                      OAUCoilOutTemp,
                      zoneEquipment,
                      sensOut,
                      latOut);

    ZoneTemp = state->dataLoopNodes->Node(ControlZoneNum).Temp;
    Qsens_sys = state->dataLoopNodes->Node(InletNode).MassFlowRate *
                Psychrometrics::PsyDeltaHSenFnTdb2W2Tdb1W1(state->dataLoopNodes->Node(OutletNode).Temp,
                                                           state->dataLoopNodes->Node(OutletNode).HumRat,
                                                           ZoneTemp,
                                                           state->dataLoopNodes->Node(ControlZoneNum).HumRat);

    EXPECT_NEAR(state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputRequired, Qsens_sys, 1.0); // Watts

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputRequired = -1000.0; // cooling load
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputReqToCoolSP = -1000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputReqToHeatSP = -2000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).SequencedOutputRequired(1) =
        state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputRequired;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).SequencedOutputRequiredToCoolingSP(1) =
        state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).OutputRequiredToCoolingSP;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).SequencedOutputRequiredToHeatingSP(1) =
        state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).OutputRequiredToHeatingSP;

    // set zone temperature
    state->dataLoopNodes->Node(ControlZoneNum).Temp = 24.0; // set zone temperature during cooling season used to determine system delivered capacity
    state->dataEnvrn->OutDryBulbTemp = 35.0;                // initialize weather
    state->dataEnvrn->OutHumRat = 0.1;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutWetBulbTemp = 30.0;

    thisSys->simulate(*state,
                      compName,
                      FirstHVACIteration,
                      AirLoopNum,
                      CompIndex,
                      HeatingActive,
                      CoolingActive,
                      OAUnitNum,
                      OAUCoilOutTemp,
                      zoneEquipment,
                      sensOut,
                      latOut);

    ZoneTemp = state->dataLoopNodes->Node(ControlZoneNum).Temp;
    Qsens_sys = state->dataLoopNodes->Node(InletNode).MassFlowRate *
                Psychrometrics::PsyDeltaHSenFnTdb2W2Tdb1W1(state->dataLoopNodes->Node(OutletNode).Temp,
                                                           state->dataLoopNodes->Node(OutletNode).HumRat,
                                                           ZoneTemp,
                                                           state->dataLoopNodes->Node(ControlZoneNum).HumRat);

    // Test 1: HX is off, cooling load is met, dehumidification control mode = None
    EXPECT_NEAR(state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputRequired, Qsens_sys, 1.0); // Watts
    EXPECT_DOUBLE_EQ(state->dataLoopNodes->Node(InletNode).MassFlowRate, state->dataLoopNodes->Node(OutletNode).MassFlowRate);
    EXPECT_ENUM_EQ(thisSys->m_DehumidControlType_Num, UnitarySystems::UnitarySys::DehumCtrlType::None);
    // coil system delta T > 0, coil system inlet node = 8, outlet node = 4
    int coilSystemInletNode = thisSys->CoolCoilInletNodeNum;
    int coilSystemOutletNode = thisSys->CoolCoilOutletNodeNum;
    int coolCoilInletHXSupplyOutlet = Util::FindItemInList("HEAT RECOVERY SUPPLY OUTLET", state->dataLoopNodes->NodeID);
    int coolCoilOutletHXExhaustInlet = Util::FindItemInList("HEAT RECOVERY EXHUAST INLET NODE", state->dataLoopNodes->NodeID);
    EXPECT_LT(state->dataLoopNodes->Node(coilSystemOutletNode).Temp, state->dataLoopNodes->Node(coilSystemInletNode).Temp);
    // coil delta T > 0, cooling coil inlet node = 6, cooling coil outlet node = 7
    EXPECT_LT(state->dataLoopNodes->Node(coolCoilOutletHXExhaustInlet).Temp, state->dataLoopNodes->Node(coolCoilInletHXSupplyOutlet).Temp);
    // HX delta T = 0 on supply side
    EXPECT_EQ(state->dataLoopNodes->Node(coilSystemInletNode).Temp, state->dataLoopNodes->Node(coolCoilInletHXSupplyOutlet).Temp);
    // HX delta T = 0 on exhaust side
    EXPECT_EQ(state->dataLoopNodes->Node(coilSystemOutletNode).Temp, state->dataLoopNodes->Node(coolCoilOutletHXExhaustInlet).Temp);

    // Test 2: HX is off, cooling load is met, dehumidification control mode = CoolReheat but no moisture load
    thisSys->m_DehumidControlType_Num = UnitarySystems::UnitarySys::DehumCtrlType::CoolReheat;
    thisSys->simulate(*state,
                      compName,
                      FirstHVACIteration,
                      AirLoopNum,
                      CompIndex,
                      HeatingActive,
                      CoolingActive,
                      OAUnitNum,
                      OAUCoilOutTemp,
                      zoneEquipment,
                      sensOut,
                      latOut);

    EXPECT_NEAR(state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputRequired, Qsens_sys, 1.0); // Watts
    EXPECT_DOUBLE_EQ(state->dataLoopNodes->Node(InletNode).MassFlowRate, state->dataLoopNodes->Node(OutletNode).MassFlowRate);
    EXPECT_ENUM_EQ(thisSys->m_DehumidControlType_Num, UnitarySystems::UnitarySys::DehumCtrlType::CoolReheat);
    // coil system delta T > 0, cooling coil inlet node = 8, outlet node = 4
    EXPECT_LT(state->dataLoopNodes->Node(coilSystemOutletNode).Temp, state->dataLoopNodes->Node(coilSystemInletNode).Temp);
    // coil delta T > 0, cooling coil inlet node = 6, outlet node = 7
    EXPECT_LT(state->dataLoopNodes->Node(coolCoilOutletHXExhaustInlet).Temp, state->dataLoopNodes->Node(coolCoilInletHXSupplyOutlet).Temp);
    // HX delta T = 0 on supply side
    EXPECT_EQ(state->dataLoopNodes->Node(coilSystemInletNode).Temp, state->dataLoopNodes->Node(coolCoilInletHXSupplyOutlet).Temp);
    // HX delta T = 0 on exhaust side
    EXPECT_EQ(state->dataLoopNodes->Node(coilSystemOutletNode).Temp, state->dataLoopNodes->Node(coolCoilOutletHXExhaustInlet).Temp);

    // Test 3: HX is off, cooling load is met, dehumidification control mode = Multimode but no moisture load
    thisSys->m_DehumidControlType_Num = UnitarySystems::UnitarySys::DehumCtrlType::Multimode;
    thisSys->simulate(*state,
                      compName,
                      FirstHVACIteration,
                      AirLoopNum,
                      CompIndex,
                      HeatingActive,
                      CoolingActive,
                      OAUnitNum,
                      OAUCoilOutTemp,
                      zoneEquipment,
                      sensOut,
                      latOut);

    EXPECT_NEAR(state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputRequired, Qsens_sys, 1.0); // Watts
    EXPECT_DOUBLE_EQ(state->dataLoopNodes->Node(InletNode).MassFlowRate, state->dataLoopNodes->Node(OutletNode).MassFlowRate);
    EXPECT_ENUM_EQ(thisSys->m_DehumidControlType_Num, UnitarySystems::UnitarySys::DehumCtrlType::Multimode);
    // coil system delta T > 0, cooling coil inlet node = 8, outlet node = 4
    EXPECT_LT(state->dataLoopNodes->Node(coilSystemOutletNode).Temp, state->dataLoopNodes->Node(coilSystemInletNode).Temp);
    // coil delta T > 0, cooling coil inlet node = 6, outlet node = 7
    EXPECT_LT(state->dataLoopNodes->Node(coolCoilOutletHXExhaustInlet).Temp, state->dataLoopNodes->Node(coolCoilInletHXSupplyOutlet).Temp);
    // HX delta T = 0 on supply side
    EXPECT_EQ(state->dataLoopNodes->Node(coilSystemInletNode).Temp, state->dataLoopNodes->Node(coolCoilInletHXSupplyOutlet).Temp);
    // HX delta T = 0 on exhaust side
    EXPECT_EQ(state->dataLoopNodes->Node(coilSystemOutletNode).Temp, state->dataLoopNodes->Node(coolCoilOutletHXExhaustInlet).Temp);

    // Test 4: HX is on, cooling load is met, dehumidification control mode = Multimode and moisture load exists
    thisSys->m_DehumidControlType_Num = UnitarySystems::UnitarySys::DehumCtrlType::Multimode;
    thisSys->m_RunOnLatentLoad = true;
    thisSys->m_Humidistat = true;
    state->dataLoopNodes->Node(thisSys->NodeNumOfControlledZone).HumRat = 0.009; // set zone humidity ratio as reference for latent met
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand(ControlZoneNum).RemainingOutputReqToDehumidSP = -0.000001; // -2 W
    thisSys->simulate(*state,
                      compName,
                      FirstHVACIteration,
                      AirLoopNum,
                      CompIndex,
                      HeatingActive,
                      CoolingActive,
                      OAUnitNum,
                      OAUCoilOutTemp,
                      zoneEquipment,
                      sensOut,
                      latOut);

    EXPECT_NEAR(state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputRequired, Qsens_sys, 1.0); // Watts
    EXPECT_DOUBLE_EQ(state->dataLoopNodes->Node(InletNode).MassFlowRate, state->dataLoopNodes->Node(OutletNode).MassFlowRate);
    EXPECT_ENUM_EQ(thisSys->m_DehumidControlType_Num, UnitarySystems::UnitarySys::DehumCtrlType::Multimode);
    // coil system delta T > 0, cooling coil inlet node = 8, outlet node = 4
    EXPECT_LT(state->dataLoopNodes->Node(coilSystemOutletNode).Temp, state->dataLoopNodes->Node(thisSys->CoolCoilInletNodeNum).Temp);
    // coil delta T > 0, cooling coil inlet node = 6, outlet node = 7
    EXPECT_LT(state->dataLoopNodes->Node(coolCoilOutletHXExhaustInlet).Temp, state->dataLoopNodes->Node(coolCoilInletHXSupplyOutlet).Temp);
    // HX delta T > 0 on supply side
    EXPECT_GT(state->dataLoopNodes->Node(coilSystemInletNode).Temp, state->dataLoopNodes->Node(coolCoilInletHXSupplyOutlet).Temp);
    // HX delta T > 0 on exhaust side
    EXPECT_GT(state->dataLoopNodes->Node(coilSystemOutletNode).Temp, state->dataLoopNodes->Node(coolCoilOutletHXExhaustInlet).Temp);

    // Test 5: HX is off, cooling load is met, dehumidification control mode = CoolReheat and moisture load exists
    // Latent output with HX off is greater than the moisture load so HX did not turn on
    state->dataLoopNodes->Node(thisSys->NodeNumOfControlledZone).HumRat = 0.0092; // set zone humidity ratio as reference for latent met
    thisSys->m_DehumidControlType_Num = UnitarySystems::UnitarySys::DehumCtrlType::CoolReheat;
    thisSys->simulate(*state,
                      compName,
                      FirstHVACIteration,
                      AirLoopNum,
                      CompIndex,
                      HeatingActive,
                      CoolingActive,
                      OAUnitNum,
                      OAUCoilOutTemp,
                      zoneEquipment,
                      sensOut,
                      latOut);

    EXPECT_NEAR(state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputRequired, Qsens_sys, 1.0); // Watts
    EXPECT_DOUBLE_EQ(state->dataLoopNodes->Node(InletNode).MassFlowRate, state->dataLoopNodes->Node(OutletNode).MassFlowRate);
    EXPECT_ENUM_EQ(thisSys->m_DehumidControlType_Num, UnitarySystems::UnitarySys::DehumCtrlType::CoolReheat);
    // coil system delta T > 0, cooling coil inlet node = 8, outlet node = 4
    EXPECT_LT(state->dataLoopNodes->Node(coilSystemOutletNode).Temp, state->dataLoopNodes->Node(thisSys->CoolCoilInletNodeNum).Temp);
    // coil delta T > 0, cooling coil inlet node = 6, outlet node = 7
    EXPECT_LT(state->dataLoopNodes->Node(coolCoilOutletHXExhaustInlet).Temp, state->dataLoopNodes->Node(coolCoilInletHXSupplyOutlet).Temp);
    // HX delta T = 0 on supply side
    EXPECT_EQ(state->dataLoopNodes->Node(coilSystemInletNode).Temp, state->dataLoopNodes->Node(coolCoilInletHXSupplyOutlet).Temp);
    // HX delta T = 0 on exhaust side
    EXPECT_EQ(state->dataLoopNodes->Node(coilSystemOutletNode).Temp, state->dataLoopNodes->Node(coolCoilOutletHXExhaustInlet).Temp);

    // Test 6: HX is on, cooling load is met, dehumidification control mode = CoolReheat and moisture load exists
    // Adjust moisture load to be greater than the latent capacity to meet the sensible load
    state->dataLoopNodes->Node(thisSys->NodeNumOfControlledZone).HumRat = 0.01; // set zone humidity ratio as reference for latent met
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand(ControlZoneNum).RemainingOutputReqToDehumidSP = -0.0002; // -400 W
    thisSys->m_DehumidControlType_Num = UnitarySystems::UnitarySys::DehumCtrlType::CoolReheat;
    thisSys->simulate(*state,
                      compName,
                      FirstHVACIteration,
                      AirLoopNum,
                      CompIndex,
                      HeatingActive,
                      CoolingActive,
                      OAUnitNum,
                      OAUCoilOutTemp,
                      zoneEquipment,
                      sensOut,
                      latOut);

    EXPECT_NEAR(state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputRequired, Qsens_sys, 1.0); // Watts
    EXPECT_DOUBLE_EQ(state->dataLoopNodes->Node(InletNode).MassFlowRate, state->dataLoopNodes->Node(OutletNode).MassFlowRate);
    EXPECT_ENUM_EQ(thisSys->m_DehumidControlType_Num, UnitarySystems::UnitarySys::DehumCtrlType::CoolReheat);
    // coil system delta T > 0, cooling coil inlet node = 8, outlet node = 4
    EXPECT_LT(state->dataLoopNodes->Node(coilSystemOutletNode).Temp, state->dataLoopNodes->Node(thisSys->CoolCoilInletNodeNum).Temp);
    // coil delta T > 0, cooling coil inlet node = 6, outlet node = 7
    EXPECT_LT(state->dataLoopNodes->Node(coolCoilOutletHXExhaustInlet).Temp, state->dataLoopNodes->Node(coolCoilInletHXSupplyOutlet).Temp);
    // HX delta T > 0 on supply side
    EXPECT_GT(state->dataLoopNodes->Node(coilSystemInletNode).Temp, state->dataLoopNodes->Node(coolCoilInletHXSupplyOutlet).Temp);
    // HX delta T > 0 on exhaust side
    EXPECT_GT(state->dataLoopNodes->Node(coilSystemOutletNode).Temp, state->dataLoopNodes->Node(coolCoilOutletHXExhaustInlet).Temp);
}

TEST_F(EnergyPlusFixture, HXAssistCCUnitarySystem_NewDXCoil_Processing_Test)
{

    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({
        "Zone,",
        "  EAST ZONE,              !- Name",
        "  0,                      !- Direction of Relative North{ deg }",
        "  0,                      !- X Origin{ m }",
        "  0,                      !- Y Origin{ m }",
        "  0,                      !- Z Origin{ m }",
        "  1,                      !- Type",
        "  1,                      !- Multiplier",
        "  autocalculate,          !- Ceiling Height{ m }",
        "  autocalculate;          !- Volume{ m3 }",
        "  ",
        "ZoneHVAC:EquipmentConnections,",
        "EAST ZONE,                 !- Zone Name",
        "  Zone2Equipment,          !- Zone Conditioning Equipment List Name",
        "  Zone 2 Inlet Node,       !- Zone Air Inlet Node or NodeList Name",
        "  Zone Exhaust Node,       !- Zone Air Exhaust Node or NodeList Name",
        "  Zone 2 Node,             !- Zone Air Node Name",
        "  Zone 2 Outlet Node;      !- Zone Return Air Node Name",
        "  ",
        "ZoneHVAC:EquipmentList,",
        "  Zone2Equipment,          !- Name",
        "  SequentialLoad,          !- Load Distribution Scheme",
        "  AirLoopHVAC:UnitarySystem, !- Zone Equipment 1 Object Type",
        "  GasHeat DXAC Furnace 1,          !- Zone Equipment 1 Name",
        "  1,                       !- Zone Equipment 1 Cooling Sequence",
        "  1;                       !- Zone Equipment 1 Heating or No - Load Sequence",
        "  ",
        "AirLoopHVAC:UnitarySystem,",
        "  GasHeat DXAC Furnace 1, !- Name",
        "  Load,                   !- Control Type",
        "  East Zone,              !- Controlling Zone or Thermostat Location",
        "  None,                   !- Dehumidification Control Type",
        "  FanAndCoilAvailSched,   !- Availability Schedule Name",
        "  Zone Exhaust Node,         !- Air Inlet Node Name",
        "  Zone 2 Inlet Node,   !- Air Outlet Node Name",
        "  Fan:OnOff,              !- Supply Fan Object Type",
        "  Supply Fan 1,           !- Supply Fan Name",
        "  BlowThrough,            !- Fan Placement",
        "  ContinuousFanSchedule,  !- Supply Air Fan Operating Mode Schedule Name",
        "  Coil:Heating:Fuel,       !- Heating Coil Object Type",
        "  Furnace Heating Coil 1, !- Heating Coil Name",
        "  ,                       !- DX Heating Coil Sizing Ratio",
        "  CoilSystem:Cooling:DX:HeatExchangerAssisted, !- Cooling Coil Object Type",
        "  Heat Exchanger Assisted Cooling Coil 1,     !- Cooling Coil Name",
        "  ,                       !- Use DOAS DX Cooling Coil",
        "  ,                       !- DOAS DX Cooling Coil Leaving Minimum Air Temperature{ C }",
        "  ,                       !- Latent Load Control",
        "  Coil:Heating:Fuel,       !- Supplemental Heating Coil Object Type",
        "  Humidistat Reheat Coil 1, !- Supplemental Heating Coil Name",
        "  SupplyAirFlowRate,      !- Supply Air Flow Rate Method During Cooling Operation",
        "  1.6,                    !- Supply Air Flow Rate During Cooling Operation{ m3/s }",
        "  ,                       !- Supply Air Flow Rate Per Floor Area During Cooling Operation{ m3/s-m2 }",
        "  ,                       !- Fraction of Autosized Design Cooling Supply Air Flow Rate",
        "  ,                       !- Design Supply Air Flow Rate Per Unit of Capacity During Cooling Operation{ m3/s-W }",
        "  SupplyAirFlowRate,      !- Supply air Flow Rate Method During Heating Operation",
        "  1.6,                    !- Supply Air Flow Rate During Heating Operation{ m3/s }",
        "  ,                       !- Supply Air Flow Rate Per Floor Area during Heating Operation{ m3/s-m2 }",
        "  ,                       !- Fraction of Autosized Design Heating Supply Air Flow Rate",
        "  ,                       !- Design Supply Air Flow Rate Per Unit of Capacity During Heating Operation{ m3/s-W }",
        "  SupplyAirFlowRate,      !- Supply Air Flow Rate Method When No Cooling or Heating is Required",
        "  1.6,                    !- Supply Air Flow Rate When No Cooling or Heating is Required{ m3/s }",
        "  ,                       !- Supply Air Flow Rate Per Floor Area When No Cooling or Heating is Required{ m3/s-m2 }",
        "  ,                       !- Fraction of Autosized Design Cooling Supply Air Flow Rate",
        "  ,                       !- Fraction of Autosized Design Heating Supply Air Flow Rate",
        "  ,                       !- Design Supply Air Flow Rate Per Unit of Capacity During Cooling Operation{ m3/s-W }",
        "  ,                       !- Design Supply Air Flow Rate Per Unit of Capacity During Heating Operation{ m3/s-W }",
        "  ,                       !- No Load Supply Air Flow Rate Control Set To Low Speed",
        "  80;                     !- Maximum Supply Air Temperature{ C }",

        "  CoilSystem:Cooling:DX:HeatExchangerAssisted,",
        "    Heat Exchanger Assisted Cooling Coil 1,  !- Name",
        "    HeatExchanger:AirToAir:SensibleAndLatent,  !- Heat Exchanger Object Type",
        "    Heat Exchanger Assisted DX 1,  !- Heat Exchanger Name",
        "    Coil:Cooling:DX,  !- Cooling Coil Object Type",
        "    Main Cooling Coil 1;      !- Cooling Coil Name",

        "  HeatExchanger:AirToAir:SensibleAndLatent,",
        "    Heat Exchanger Assisted DX 1,      !- Name",
        "    ,   !- Availability Schedule Name",
        "    1.6,                !- Nominal Supply Air Flow Rate {m3/s}",
        "    0.7,                     !- Sensible Effectiveness at 100% Heating Air Flow {dimensionless}",
        "    0.65,                    !- Latent Effectiveness at 100% Heating Air Flow {dimensionless}",
        "    0.7,                     !- Sensible Effectiveness at 100% Cooling Air Flow {dimensionless}",
        "    0.65,                    !- Latent Effectiveness at 100% Cooling Air Flow {dimensionless}",
        "    DX Cooling Coil Air Inlet Node,   !- Supply Air Inlet Node Name",
        "    Heat Recovery Supply Outlet,  !- Supply Air Outlet Node Name",
        "    Heat Recovery Exhuast Inlet Node,  !- Exhaust Air Inlet Node Name",
        "    Heating Coil Air Inlet Node,  !- Exhaust Air Outlet Node Name",
        "    0,                       !- Nominal Electric Power {W}",
        "    No,                     !- Supply Air Outlet Temperature Control",
        "    Plate,                   !- Heat Exchanger Type",
        "    MinimumExhaustTemperature,  !- Frost Control Type",
        "    1.7,                     !- Threshold Temperature {C}",
        "    0.083,                   !- Initial Defrost Time Fraction {dimensionless}",
        "    0.012,                   !- Rate of Defrost Time Fraction Increase {1/K}",
        "    Yes,                     !- Economizer Lockout",
        "    SenEffectivenessTable,   !- Sensible Effectiveness of Heating Air Flow Curve Name",
        "    LatEffectivenessTable,   !- Latent Effectiveness of Heating Air Flow Curve Name",
        "    SenEffectivenessTable,   !- Sensible Effectiveness of Cooling Air Flow Curve Name",
        "    LatEffectivenessTable;   !- Latent Effectiveness of Cooling Air Flow Curve Name",

        "  Table:IndependentVariable,",
        "    airFlowRatio,  !- Name",
        "    Linear,                  !- Interpolation Method",
        "    Linear,                  !- Extrapolation Method",
        "    0.0,                     !- Minimum Value",
        "    1.0,                     !- Maximum Value",
        "    ,                        !- Normalization Reference Value",
        "    Dimensionless,           !- Unit Type",
        "    ,                        !- External File Name",
        "    ,                        !- External File Column Number",
        "    ,                        !- External File Starting Row Number",
        "    0.75,                    !- Value 1",
        "    1.0;                     !- Value 2",

        "  Table:IndependentVariableList,",
        "    effectiveness_IndependentVariableList,  !- Name",
        "    airFlowRatio;     !- Independent Variable 1 Name",

        "  Table:Lookup,",
        "    SenEffectivenessTable,   !- Name",
        "    effectiveness_IndependentVariableList,  !- Independent Variable List Name",
        "    DivisorOnly,             !- Normalization Method",
        "    0.7,                     !- Normalization Divisor",
        "    0.0,                     !- Minimum Output",
        "    1.0,                     !- Maximum Output",
        "    Dimensionless,           !- Output Unit Type",
        "    ,                        !- External File Name",
        "    ,                        !- External File Column Number",
        "    ,                        !- External File Starting Row Number",
        "    0.75,                    !- Output Value 1",
        "    0.70;                    !- Output Value 2",

        "  Table:Lookup,",
        "    LatEffectivenessTable,   !- Name",
        "    effectiveness_IndependentVariableList,  !- Independent Variable List Name",
        "    DivisorOnly,             !- Normalization Method",
        "    0.65,                    !- Normalization Divisor",
        "    0.0,                     !- Minimum Output",
        "    1.0,                     !- Maximum Output",
        "    Dimensionless,           !- Output Unit Type",
        "    ,                        !- External File Name",
        "    ,                        !- External File Column Number",
        "    ,                        !- External File Starting Row Number",
        "    0.70,                    !- Output Value 1",
        "    0.65;                    !- Output Value 2",

        "  Coil:Cooling:DX,",
        "    Main Cooling Coil 1,     !- Name",
        "    Heat Recovery Supply Outlet,  !- Evaporator Inlet Node Name",
        "    Heat Recovery Exhuast Inlet Node,  !- Evaporator Outlet Node Name",
        "    ,                        !- Availability Schedule Name",
        "    ,                        !- Condenser Zone Name",
        "    Main Cooling Coil 1 Condenser Inlet Node,  !- Condenser Inlet Node Name",
        "    Main Cooling Coil 1 Condenser Outlet Node,  !- Condenser Outlet Node Name",
        "    Main Cooling Coil 1 Performance;  !- Performance Object Name",

        "  Coil:Cooling:DX:CurveFit:Performance,",
        "     Main Cooling Coil 1 Performance,  !- Name",
        "     0.0,                     !- Crankcase Heater Capacity {W}",
        "     ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "     ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "     10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "     ,                        !- Unit Internal Static Air Pressure {Pa}",
        "     Continuous,              !- Capacity Control Method",
        "     ,                        !- Evaporative Condenser Basin Heater Capacity {W/K}",
        "     ,                        !- Evaporative Condenser Basin Heater Setpoint Temperature {C}",
        "     ,                        !- Evaporative Condenser Basin Heater Operating Schedule Name",
        "     Electricity,             !- Compressor Fuel Type",
        "     Main Cooling Coil 1 Operating Mode;  !- Base Operating Mode",

        "  Coil:Cooling:DX:CurveFit:OperatingMode,",
        "    Main Cooling Coil 1 Operating Mode,  !- Name",
        "    AUTOSIZE,                !- Rated Gross Total Cooling Capacity {W}",
        "    AUTOSIZE,                !- Rated Evaporator Air Flow Rate {m3/s}",
        "    ,                        !- Rated Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Maximum Cycling Rate {cycles/hr}",
        "    0.0,                     !- Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity {dimensionless}",
        "    ,                        !- Latent Capacity Time Constant {s}",
        "    0.0,                     !- Nominal Time for Condensate Removal to Begin {s}",
        "    ,                        !- Apply Latent Degradation to Speeds Greater than 1",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Nominal Evaporative Condenser Pump Power {W}",
        "    1.0,                     !- Nominal Speed Number",
        "    Main Cooling Coil 1 Speed 1 Performance;  !- Speed 1 Name",

        "  Coil:Cooling:DX:CurveFit:Speed,",
        "    Main Cooling Coil 1 Speed 1 Performance,  !- Name",
        "    1.0000,                  !- Gross Total Cooling Capacity Fraction",
        "    1.0000,                  !- Evaporator Air Flow Rate Fraction",
        "    1.0000,                  !- Condenser Air Flow Rate Fraction",
        "    0.75,                    !- Gross Sensible Heat Ratio",
        "    3.866381837,             !- Gross Cooling COP {W/W}",
        "    1.0,                     !- Active Fraction of Coil Face Area",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Evaporative Condenser Pump Power Fraction",
        "    ,                        !- Evaporative Condenser Effectiveness {dimensionless}",
        "    HPCoolingCAPFTemp4,      !- Total Cooling Capacity Modifier Function of Temperature Curve Name",
        "    HPACFFF,                 !- Total Cooling Capacity Modifier Function of Air Flow Fraction Curve Name",
        "    HPCoolingEIRFTemp4,      !- Energy Input Ratio Modifier Function of Temperature Curve Name",
        "    HPACFFF,                 !- Energy Input Ratio Modifier Function of Air Flow Fraction Curve Name",
        "    HPACCOOLPLFFPLR;         !- Part Load Fraction Correlation Curve Name",

        "   Curve:Quadratic,",
        "    HPACCOOLPLFFPLR,         !- Name",
        "    1.0,                    !- Coefficient1 Constant",
        "    0.0,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x  ",

        "  Curve:Cubic,",
        "    HPACFFF,                 !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Biquadratic,",
        "    HPCoolingEIRFTemp4,      !- Name",
        "    0.0001514017,            !- Coefficient1 Constant",
        "    0.0655062896,            !- Coefficient2 x",
        "    -0.0020370821,           !- Coefficient3 x**2",
        "    0.0067823041,            !- Coefficient4 y",
        "    0.0004087196,            !- Coefficient5 y**2",
        "    -0.0003552302,           !- Coefficient6 x*y",
        "    13.89,                   !- Minimum Value of x",
        "    22.22,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    51.67,                   !- Maximum Value of y",
        "    0.5141,                  !- Minimum Curve Output",
        "    1.7044,                  !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    HPCoolingCAPFTemp4,      !- Name",
        "    1.3544202152,            !- Coefficient1 Constant",
        "    -0.0493402773,           !- Coefficient2 x",
        "    0.0022649843,            !- Coefficient3 x**2",
        "    0.0008517727,            !- Coefficient4 y",
        "    -0.0000426316,           !- Coefficient5 y**2",
        "    -0.0003364517,           !- Coefficient6 x*y",
        "    13.89,                   !- Minimum Value of x",
        "    22.22,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    51.67,                   !- Maximum Value of y",
        "    0.7923,                  !- Minimum Curve Output",
        "    1.2736,                  !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  OutdoorAir:Node,",
        "    Main Cooling Coil 1 Condenser Node,  !- Name",
        "    -1.0;                    !- Height Above Ground {m}",

        "Fan:OnOff,",
        "  Supply Fan 1,           !- Name",
        "  FanAndCoilAvailSched,   !- Availability Schedule Name",
        "  0.7,                    !- Fan Total Efficiency",
        "  600.0,                  !- Pressure Rise{ Pa }",
        "  1.6,                    !- Maximum Flow Rate{ m3 / s }",
        "  0.9,                    !- Motor Efficiency",
        "  1.0,                    !- Motor In Airstream Fraction",
        "  Zone Exhaust Node,      !- Air Inlet Node Name",
        "  DX Cooling Coil Air Inlet Node;  !- Air Outlet Node Name",

        "Coil:Heating:Fuel,",
        "  Furnace Heating Coil 1, !- Name",
        "  FanAndCoilAvailSched,   !- Availability Schedule Name",
        "  NaturalGas,              !- Fuel Type",
        "  0.8,                    !- Gas Burner Efficiency",
        "  32000,                  !- Nominal Capacity{ W }",
        "  Heating Coil Air Inlet Node, !- Air Inlet Node Name",
        "  Reheat Coil Air Inlet Node;  !- Air Outlet Node Name",
        "  ",
        "Coil:Heating:Fuel,",
        "  Humidistat Reheat Coil 1, !- Name",
        "  FanAndCoilAvailSched, !- Availability Schedule Name",
        "  NaturalGas,              !- Fuel Type",
        "  0.8, !- Gas Burner Efficiency",
        "  32000, !- Nominal Capacity{ W }",
        "  Reheat Coil Air Inlet Node, !- Air Inlet Node Name",
        "  Zone 2 Inlet Node;    !- Air Outlet Node Name",
        "  ",
        "ScheduleTypeLimits,",
        "  Any Number;             !- Name",
        "  ",
        "Schedule:Compact,",
        "  FanAndCoilAvailSched,   !- Name",
        "  Any Number,             !- Schedule Type Limits Name",
        "  Through: 12/31,         !- Field 1",
        "  For: AllDays,           !- Field 2",
        "  Until: 24:00, 1.0;      !- Field 3",
        "  ",
        "Schedule:Compact,",
        "  ContinuousFanSchedule,  !- Name",
        "  Any Number,             !- Schedule Type Limits Name",
        "  Through: 12/31,         !- Field 1",
        "  For: AllDays,           !- Field 2",
        "  Until: 24:00, 1.0;      !- Field 3",
        "  ",
        "Curve:Quadratic,",
        "  CoolCapFFF,       !- Name",
        "  0.8,                    !- Coefficient1 Constant",
        "  0.2,                    !- Coefficient2 x",
        "  0.0,                    !- Coefficient3 x**2",
        "  0.5,                    !- Minimum Value of x",
        "  1.5;                    !- Maximum Value of x",
        "  ",
        "Curve:Quadratic,",
        "  COOLEIRFFF,           !- Name",
        "  1.1552,                 !- Coefficient1 Constant",
        "  -0.1808,                !- Coefficient2 x",
        "  0.0256,                 !- Coefficient3 x**2",
        "  0.5,                    !- Minimum Value of x",
        "  1.5;                    !- Maximum Value of x",
        "  ",
        "Curve:Quadratic,",
        "  PLFFPLR,          !- Name",
        "  0.85,                   !- Coefficient1 Constant",
        "  0.15,                   !- Coefficient2 x",
        "  0.0,                    !- Coefficient3 x**2",
        "  0.0,                    !- Minimum Value of x",
        "  1.0;                    !- Maximum Value of x",
        "  ",
        "Curve:Biquadratic,",
        "  CoolCapFT,        !- Name",
        "  0.942587793,            !- Coefficient1 Constant",
        "  0.009543347,            !- Coefficient2 x",
        "  0.000683770,            !- Coefficient3 x**2",
        "  -0.011042676,           !- Coefficient4 y",
        "  0.000005249,            !- Coefficient5 y**2",
        "  -0.000009720,           !- Coefficient6 x*y",
        "  12.77778,               !- Minimum Value of x",
        "  23.88889,               !- Maximum Value of x",
        "  18.0,                   !- Minimum Value of y",
        "  46.11111,               !- Maximum Value of y",
        "  ,                       !- Minimum Curve Output",
        "  ,                       !- Maximum Curve Output",
        "  Temperature,            !- Input Unit Type for X",
        "  Temperature,            !- Input Unit Type for Y",
        "  Dimensionless;          !- Output Unit Type",
        "  ",
        "Curve:Biquadratic,",
        "  COOLEIRFT,            !- Name",
        "  0.342414409,            !- Coefficient1 Constant",
        "  0.034885008,            !- Coefficient2 x",
        "  -0.000623700,           !- Coefficient3 x**2",
        "  0.004977216,            !- Coefficient4 y",
        "  0.000437951,            !- Coefficient5 y**2",
        "  -0.000728028,           !- Coefficient6 x*y",
        "  12.77778,               !- Minimum Value of x",
        "  23.88889,               !- Maximum Value of x",
        "  18.0,                   !- Minimum Value of y",
        "  46.11111,               !- Maximum Value of y",
        "  ,                       !- Minimum Curve Output",
        "  ,                       !- Maximum Curve Output",
        "  Temperature,            !- Input Unit Type for X",
        "  Temperature,            !- Input Unit Type for Y",
        "  Dimensionless;          !- Output Unit Type",
    });

    ASSERT_TRUE(process_idf(idf_objects)); // read idf objects
    state->init_state(*state);

    HeatBalanceManager::GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);                            // expect no errors

    DataZoneEquipment::GetZoneEquipmentData(*state); // read zone equipment configuration and list objects

    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataZoneEquip->ZoneEquipList(1).EquipIndex.allocate(1);
    state->dataZoneEquip->ZoneEquipList(1).EquipIndex(1) = 1; // initialize equipment index for ZoneHVAC

    std::string compName = "GASHEAT DXAC FURNACE 1";
    bool zoneEquipment = true;
    UnitarySystems::UnitarySys::factory(*state, HVAC::UnitarySysType::Unitary_AnyCoilType, compName, zoneEquipment, 0);
    UnitarySystems::UnitarySys *thisSys = &state->dataUnitarySystems->unitarySys[0];
    state->dataZoneEquip->ZoneEquipInputsFilled = true;                                  // indicate zone data is available
    thisSys->getUnitarySystemInputData(*state, compName, zoneEquipment, 0, ErrorsFound); // get UnitarySystem input from object above

    ASSERT_EQ(1, state->dataUnitarySystems->numUnitarySystems); // only 1 unitary system above so expect 1 as number of unitary system objects
}
