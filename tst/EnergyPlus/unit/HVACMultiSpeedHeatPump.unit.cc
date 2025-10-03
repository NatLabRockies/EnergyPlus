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

// EnergyPlus::MultiSpeedAirToAirHeatPump Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/BranchInputManager.hh>
#include <EnergyPlus/DXCoils.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataZoneControls.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/HVACMultiSpeedHeatPump.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/HeatingCoils.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimAirServingZones.hh>
#include <EnergyPlus/SingleDuct.hh>
#include <EnergyPlus/SplitterComponent.hh>
#include <EnergyPlus/ZoneAirLoopEquipmentManager.hh>
#include <EnergyPlus/ZoneEquipmentManager.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

using namespace EnergyPlus::BranchInputManager;
using namespace EnergyPlus::DataHeatBalance;
using namespace EnergyPlus::DataZoneControls;
using namespace EnergyPlus::DataZoneEquipment;
using namespace EnergyPlus::DataZoneEnergyDemands;
using namespace EnergyPlus::HeatBalanceManager;
using namespace EnergyPlus::HVACMultiSpeedHeatPump;
using namespace EnergyPlus::MixedAir;
using namespace EnergyPlus::SimAirServingZones;
using namespace EnergyPlus::SingleDuct;
using namespace EnergyPlus::SplitterComponent;
using namespace EnergyPlus::ZoneAirLoopEquipmentManager;
using namespace EnergyPlus::ZoneTempPredictorCorrector;

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, HVACMultiSpeedHeatPump_ReportVariableInitTest)
{

    bool ErrorsFound(false);
    int constexpr MSHeatPumpNum(2);
    bool const FirstHVACIteration(true);
    int constexpr AirLoopNum(2);
    Real64 QZnReq(-10000.0);
    Real64 OnOffAirFlowRatio(1.0);

    std::string const idf_objects = delimited_string({

        "!-   ===========  ALL OBJECTS IN CLASS: ZONECONTROL:THERMOSTAT ===========",

        "  ZoneControl:Thermostat,",
        "    Z401TempCtrl,            !- Name",
        "    401,                     !- Zone or ZoneList Name",
        "    Zone Control Type Sched, !- Control Type Schedule Name",
        "    ThermostatSetpoint:DualSetpoint,   !- Control 3 Object Type",
        "    DualSetPoint1;            !- Control 3 Name",

        "  ThermostatSetpoint:DualSetpoint,",
        "    DualSetPoint1,            !- Name",
        "    401HeatingSP,            !- Heating Setpoint Temperature Schedule Name",
        "    401CoolingSP;            !- Cooling Setpoint Temperature Schedule Name",

        "  ZoneControl:Thermostat,",
        "    Z402TempCtrl,            !- Name",
        "    402,                     !- Zone or ZoneList Name",
        "    Zone Control Type Sched, !- Control Type Schedule Name",
        "    ThermostatSetpoint:DualSetpoint,   !- Control 3 Object Type",
        "    DualSetPoint2;            !- Control 3 Name",

        "  ThermostatSetpoint:DualSetpoint,",
        "    DualSetPoint2,            !- Name",
        "    402HeatingSP,            !- Heating Setpoint Temperature Schedule Name",
        "    402CoolingSP;            !- Cooling Setpoint Temperature Schedule Name",

        "!-   ===========  ALL OBJECTS IN CLASS: AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:NOREHEAT ===========",

        "  AirTerminal:SingleDuct:ConstantVolume:NoReheat,",
        "    401directair,            !- Name",
        "    AC-24sched,              !- Availability Schedule Name",
        "    Z401 zone inlet 2AT,     !- Air Inlet Node Name",
        "    Z401 zone inlet,         !- Air Outlet Node Name",
        "    3.209;                   !- Maximum Air Flow Rate {m3/s}",

        "  AirTerminal:SingleDuct:ConstantVolume:NoReheat,",
        "    402directair,            !- Name",
        "    AC-25 sched,             !- Availability Schedule Name",
        "    Z402 zone inlet 2AT,     !- Air Inlet Node Name",
        "    Z402 zone inlet,         !- Air Outlet Node Name",
        "    3.209;                   !- Maximum Air Flow Rate {m3/s}",

        "!-   ===========  ALL OBJECTS IN CLASS: ZONEHVAC:EQUIPMENTLIST ===========",

        "  ZoneHVAC:EquipmentList,",
        "    Z401 terminal list,      !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    401directairADU,         !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "  ZoneHVAC:EquipmentList,",
        "    Z402 terminal list,      !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    402directairADU,         !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "!-   ===========  ALL OBJECTS IN CLASS: ZONEHVAC:AIRDISTRIBUTIONUNIT ===========",

        "  ZoneHVAC:AirDistributionUnit,",
        "    401directairADU,         !- Name",
        "    Z401 zone inlet,         !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:ConstantVolume:NoReheat,  !- Air Terminal Object Type",
        "    401directair;            !- Air Terminal Name",

        "  ZoneHVAC:AirDistributionUnit,",
        "    402directairADU,         !- Name",
        "    Z402 zone inlet,         !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:ConstantVolume:NoReheat,  !- Air Terminal Object Type",
        "    402directair;            !- Air Terminal Name",

        "!-   ===========  ALL OBJECTS IN CLASS: ZONEHVAC:EQUIPMENTCONNECTIONS ===========",

        "  ZoneHVAC:EquipmentConnections,",
        "    401,                     !- Zone Name",
        "    Z401 terminal list,      !- Zone Conditioning Equipment List Name",
        "    Z401 zone inlet,         !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    Z401 air node,           !- Zone Air Node Name",
        "    Z401 outlet node;        !- Zone Return Air Node Name",

        "  ZoneHVAC:EquipmentConnections,",
        "    402,                     !- Zone Name",
        "    Z402 terminal list,      !- Zone Conditioning Equipment List Name",
        "    Z402 zone inlet,         !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    Z402 air node,           !- Zone Air Node Name",
        "    Z402 outlet node;        !- Zone Return Air Node Name",

        "!-   ===========  ALL OBJECTS IN CLASS: FAN:ONOFF ===========",
        "  Fan:OnOff,",
        "    AC24_Fan,                !- Name",
        "    AC-24sched,              !- Availability Schedule Name",
        "    0.25,                    !- Fan Total Efficiency",
        "    249.089,                 !- Pressure Rise {Pa}",
        "    3.209,                   !- Maximum Flow Rate {m3/s}",
        "    0.85,                    !- Motor Efficiency",
        "    1,                       !- Motor In Airstream Fraction",
        "    AC-24 SF inlet air node, !- Air Inlet Node Name",
        "    AC-24 SF outlet air node,!- Air Outlet Node Name",
        "    ,                        !- Fan Power Ratio Function of Speed Ratio Curve Name",
        "    ,                        !- Fan Efficiency Ratio Function of Speed Ratio Curve Name",
        "    General;                 !- End-Use Subcategory",
        "  Fan:OnOff,",
        "    AC25_Fan,                !- Name",
        "    AC-25 sched,             !- Availability Schedule Name",
        "    0.25,                    !- Fan Total Efficiency",
        "    249.089,                 !- Pressure Rise {Pa}",
        "    3.209,                   !- Maximum Flow Rate {m3/s}",
        "    0.85,                    !- Motor Efficiency",
        "    1,                       !- Motor In Airstream Fraction",
        "    AC-25 SF inlet air node, !- Air Inlet Node Name",
        "    AC-25 SF outlet air node,!- Air Outlet Node Name",
        "    ,                        !- Fan Power Ratio Function of Speed Ratio Curve Name",
        "    ,                        !- Fan Efficiency Ratio Function of Speed Ratio Curve Name",
        "    General;                 !- End-Use Subcategory",
        "  Coil:Cooling:DX:MultiSpeed,",
        "    AC24_cooling,            !- Name",
        "    AC-24sched,              !- Availability Schedule Name",
        "    AC-24 SF outlet air node,!- Air Inlet Node Name",
        "    AC-24 HC inlet node,     !- Air Outlet Node Name",
        "    ,                        !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    No,                      !- Apply Part Load Fraction to Speeds Greater than 1",
        "    No,                      !- Apply Latent Degradation to Speeds Greater than 1",
        "    0,                       !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10,                      !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    Electricity,             !- Fuel Type",
        "    2,                       !- Number of Speeds",
        "    53500,                   !- Speed 1 Gross Rated Total Cooling Capacity {W}",
        "    0.737,                   !- Speed 1 Gross Rated Sensible Heat Ratio",
        "    3.42,                    !- Speed 1 Gross Rated Cooling COP {W/W}",
        "    3.209,                   !- Speed 1 Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Speed 1 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Speed 1 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    CoolingTempCurve,        !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "    CoolingFlowCurve,        !- Speed 1 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    EIRTempCurve,            !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFlowCurve,            !- Speed 1 Energy Input Ratio Function of Flow Fraction Curve Name",
        "    PLFCurve,                !- Speed 1 Part Load Fraction Correlation Curve Name",
        "    ,                        !- Speed 1 Nominal Time for Condensate Removal to Begin {s}",
        "    ,                        !- Speed 1 Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity {dimensionless}",
        "    3,                       !- Speed 1 Maximum Cycling Rate {cycles/hr}",
        "    ,                        !- Speed 1 Latent Capacity Time Constant {s}",
        "    0.001,                   !- Speed 1 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    WasteHeatTempCurve,      !- Speed 1 Waste Heat Function of Temperature Curve Name",
        "    0.9,                     !- Speed 1 Evaporative Condenser Effectiveness {dimensionless}",
        "    ,                        !- Speed 1 Evaporative Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 Rated Evaporative Condenser Pump Power Consumption {W}",
        "    67992.5,                 !- Speed 2 Gross Rated Total Cooling Capacity {W}",
        "    0.737,                   !- Speed 2 Gross Rated Sensible Heat Ratio",
        "    3.42,                    !- Speed 2 Gross Rated Cooling COP {W/W}",
        "    3.209,                   !- Speed 2 Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Speed 2 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Speed 2 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    CoolingTempCurve,        !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "    CoolingFlowCurve,        !- Speed 2 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    EIRTempCurve,            !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFlowCurve,            !- Speed 2 Energy Input Ratio Function of Flow Fraction Curve Name",
        "    PLFCurve,                !- Speed 2 Part Load Fraction Correlation Curve Name",
        "    ,                        !- Speed 2 Nominal Time for Condensate Removal to Begin {s}",
        "    ,                        !- Speed 2 Ratio of Initial Moisture Evaporation Rate and steady state Latent Capacity {dimensionless}",
        "    0,                       !- Speed 2 Maximum Cycling Rate {cycles/hr}",
        "    ,                        !- Speed 2 Latent Capacity Time Constant {s}",
        "    0.001,                   !- Speed 2 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    WasteHeatTempCurve,      !- Speed 2 Waste Heat Function of Temperature Curve Name",
        "    0.9,                     !- Speed 2 Evaporative Condenser Effectiveness {dimensionless}",
        "    ,                        !- Speed 2 Evaporative Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 2 Rated Evaporative Condenser Pump Power Consumption {W}",
        "    ,                        !- Speed 3 Gross Rated Total Cooling Capacity {W}",
        "    ,                        !- Speed 3 Gross Rated Sensible Heat Ratio",
        "    3,                       !- Speed 3 Gross Rated Cooling COP {W/W}",
        "    ,                        !- Speed 3 Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Speed 3 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Speed 3 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 3 Total Cooling Capacity Function of Temperature Curve Name",
        "    ,                        !- Speed 3 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    ,                        !- Speed 3 Energy Input Ratio Function of Temperature Curve Name",
        "    ,                        !- Speed 3 Energy Input Ratio Function of Flow Fraction Curve Name",
        "    ,                        !- Speed 3 Part Load Fraction Correlation Curve Name",
        "    ,                        !- Speed 3 Nominal Time for Condensate Removal to Begin {s}",
        "    ,                        !- Speed 3 Ratio of Initial Moisture Evaporation Rate and steady state Latent Capacity {dimensionless}",
        "    ,                        !- Speed 3 Maximum Cycling Rate {cycles/hr}",
        "    ,                        !- Speed 3 Latent Capacity Time Constant {s}",
        "    ,                        !- Speed 3 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    ,                        !- Speed 3 Waste Heat Function of Temperature Curve Name",
        "    0.9,                     !- Speed 3 Evaporative Condenser Effectiveness {dimensionless}",
        "    ,                        !- Speed 3 Evaporative Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 3 Rated Evaporative Condenser Pump Power Consumption {W}",
        "    ,                        !- Speed 4 Gross Rated Total Cooling Capacity {W}",
        "    ,                        !- Speed 4 Gross Rated Sensible Heat Ratio",
        "    3,                       !- Speed 4 Gross Rated Cooling COP {W/W}",
        "    ,                        !- Speed 4 Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Speed 4 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Speed 4 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 4 Total Cooling Capacity Function of Temperature Curve Name",
        "    ,                        !- Speed 4 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    ,                        !- Speed 4 Energy Input Ratio Function of Temperature Curve Name",
        "    ,                        !- Speed 4 Energy Input Ratio Function of Flow Fraction Curve Name",
        "    ,                        !- Speed 4 Part Load Fraction Correlation Curve Name",
        "    ,                        !- Speed 4 Nominal Time for Condensate Removal to Begin {s}",
        "    ,                        !- Speed 4 Ratio of Initial Moisture Evaporation Rate and steady state Latent Capacity {dimensionless}",
        "    ,                        !- Speed 4 Maximum Cycling Rate {cycles/hr}",
        "    ,                        !- Speed 4 Latent Capacity Time Constant {s}",
        "    ,                        !- Speed 4 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    ,                        !- Speed 4 Waste Heat Function of Temperature Curve Name",
        "    0.9;                     !- Speed 4 Evaporative Condenser Effectiveness {dimensionless}",
        "  Coil:Cooling:DX:MultiSpeed,",
        "    AC25_cooling,            !- Name",
        "    AC-25 sched,             !- Availability Schedule Name",
        "    AC-25 SF outlet air node,!- Air Inlet Node Name",
        "    AC-25 HC inlet node,     !- Air Outlet Node Name",
        "    ,                        !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    No,                      !- Apply Part Load Fraction to Speeds Greater than 1",
        "    No,                      !- Apply Latent Degradation to Speeds Greater than 1",
        "    0,                       !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10,                      !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    Electricity,             !- Fuel Type",
        "    2,                       !- Number of Speeds",
        "    53500,                   !- Speed 1 Gross Rated Total Cooling Capacity {W}",
        "    0.737,                   !- Speed 1 Gross Rated Sensible Heat Ratio",
        "    3.42,                    !- Speed 1 Gross Rated Cooling COP {W/W}",
        "    3.209,                   !- Speed 1 Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Speed 1 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Speed 1 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    CoolingTempCurve,        !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "    CoolingFlowCurve,        !- Speed 1 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    EIRTempCurve,            !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFlowCurve,            !- Speed 1 Energy Input Ratio Function of Flow Fraction Curve Name",
        "    PLFCurve,                !- Speed 1 Part Load Fraction Correlation Curve Name",
        "    ,                        !- Speed 1 Nominal Time for Condensate Removal to Begin {s}",
        "    ,                        !- Speed 1 Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity {dimensionless}",
        "    3,                       !- Speed 1 Maximum Cycling Rate {cycles/hr}",
        "    ,                        !- Speed 1 Latent Capacity Time Constant {s}",
        "    0.001,                   !- Speed 1 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    WasteHeatTempCurve,      !- Speed 1 Waste Heat Function of Temperature Curve Name",
        "    0.9,                     !- Speed 1 Evaporative Condenser Effectiveness {dimensionless}",
        "    ,                        !- Speed 1 Evaporative Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 Rated Evaporative Condenser Pump Power Consumption {W}",
        "    67992.5,                 !- Speed 2 Gross Rated Total Cooling Capacity {W}",
        "    0.737,                   !- Speed 2 Gross Rated Sensible Heat Ratio",
        "    3.42,                    !- Speed 2 Gross Rated Cooling COP {W/W}",
        "    3.209,                   !- Speed 2 Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Speed 2 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Speed 2 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    CoolingTempCurve,        !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "    CoolingFlowCurve,        !- Speed 2 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    EIRTempCurve,            !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFlowCurve,            !- Speed 2 Energy Input Ratio Function of Flow Fraction Curve Name",
        "    PLFCurve,                !- Speed 2 Part Load Fraction Correlation Curve Name",
        "    ,                        !- Speed 2 Nominal Time for Condensate Removal to Begin {s}",
        "    ,                        !- Speed 2 Ratio of Initial Moisture Evaporation Rate and steady state Latent Capacity {dimensionless}",
        "    0,                       !- Speed 2 Maximum Cycling Rate {cycles/hr}",
        "    ,                        !- Speed 2 Latent Capacity Time Constant {s}",
        "    0.001,                   !- Speed 2 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    WasteHeatTempCurve,      !- Speed 2 Waste Heat Function of Temperature Curve Name",
        "    0.9,                     !- Speed 2 Evaporative Condenser Effectiveness {dimensionless}",
        "    ,                        !- Speed 2 Evaporative Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 2 Rated Evaporative Condenser Pump Power Consumption {W}",
        "    ,                        !- Speed 3 Gross Rated Total Cooling Capacity {W}",
        "    ,                        !- Speed 3 Gross Rated Sensible Heat Ratio",
        "    3,                       !- Speed 3 Gross Rated Cooling COP {W/W}",
        "    ,                        !- Speed 3 Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Speed 3 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Speed 3 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 3 Total Cooling Capacity Function of Temperature Curve Name",
        "    ,                        !- Speed 3 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    ,                        !- Speed 3 Energy Input Ratio Function of Temperature Curve Name",
        "    ,                        !- Speed 3 Energy Input Ratio Function of Flow Fraction Curve Name",
        "    ,                        !- Speed 3 Part Load Fraction Correlation Curve Name",
        "    ,                        !- Speed 3 Nominal Time for Condensate Removal to Begin {s}",
        "    ,                        !- Speed 3 Ratio of Initial Moisture Evaporation Rate and steady state Latent Capacity {dimensionless}",
        "    ,                        !- Speed 3 Maximum Cycling Rate {cycles/hr}",
        "    ,                        !- Speed 3 Latent Capacity Time Constant {s}",
        "    ,                        !- Speed 3 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    ,                        !- Speed 3 Waste Heat Function of Temperature Curve Name",
        "    0.9,                     !- Speed 3 Evaporative Condenser Effectiveness {dimensionless}",
        "    ,                        !- Speed 3 Evaporative Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 3 Rated Evaporative Condenser Pump Power Consumption {W}",
        "    ,                        !- Speed 4 Gross Rated Total Cooling Capacity {W}",
        "    ,                        !- Speed 4 Gross Rated Sensible Heat Ratio",
        "    3,                       !- Speed 4 Gross Rated Cooling COP {W/W}",
        "    ,                        !- Speed 4 Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Speed 4 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Speed 4 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Speed 4 Total Cooling Capacity Function of Temperature Curve Name",
        "    ,                        !- Speed 4 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    ,                        !- Speed 4 Energy Input Ratio Function of Temperature Curve Name",
        "    ,                        !- Speed 4 Energy Input Ratio Function of Flow Fraction Curve Name",
        "    ,                        !- Speed 4 Part Load Fraction Correlation Curve Name",
        "    ,                        !- Speed 4 Nominal Time for Condensate Removal to Begin {s}",
        "    ,                        !- Speed 4 Ratio of Initial Moisture Evaporation Rate and steady state Latent Capacity {dimensionless}",
        "    ,                        !- Speed 4 Maximum Cycling Rate {cycles/hr}",
        "    ,                        !- Speed 4 Latent Capacity Time Constant {s}",
        "    ,                        !- Speed 4 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    ,                        !- Speed 4 Waste Heat Function of Temperature Curve Name",
        "    0.9;                     !- Speed 4 Evaporative Condenser Effectiveness {dimensionless}",
        "!-   ===========  ALL OBJECTS IN CLASS: COIL:HEATING:ELECTRIC ===========",
        "  Coil:Heating:Electric,",
        "    AC24ElecHeater,          !- Name",
        "    AC-24sched,              !- Availability Schedule Name",
        "    1,                       !- Efficiency",
        "    55000,                   !- Nominal Capacity {W}",
        "    AC-24 RHC inlet node,    !- Air Inlet Node Name",
        "    AC-24 airloop outlet node;  !- Air Outlet Node Name",
        "  Coil:Heating:Electric,",
        "    AC25ElecHeater,          !- Name",
        "    AC-25 sched,             !- Availability Schedule Name",
        "    1,                       !- Efficiency",
        "    55000,                   !- Nominal Capacity {W}",
        "    AC-25 RHC inlet node,    !- Air Inlet Node Name",
        "    AC-25 airloop outlet node;  !- Air Outlet Node Name",

        "!-   ===========  ALL OBJECTS IN CLASS: COIL:HEATING:DX:MULTISPEED ===========",
        "  Coil:Heating:DX:MultiSpeed,",
        "    AC24Heating,             !- Name",
        "    AC-24sched,              !- Availability Schedule Name",
        "    AC-24 HC inlet node,     !- Air Inlet Node Name",
        "    AC-24 RHC inlet node,    !- Air Outlet Node Name",
        "    -8,                      !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Outdoor Dry-Bulb Temperature to Turn On Compressor {C}",
        "    0,                       !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10,                      !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    DefrostTempCurve,        !- Defrost Energy Input Ratio Function of Temperature Curve Name",
        "    4.4,                     !- Maximum Outdoor Dry-Bulb Temperature for Defrost Operation {C}",
        "    ReverseCycle,            !- Defrost Strategy",
        "    OnDemand,                !- Defrost Control",
        "    0.087,                   !- Defrost Time Period Fraction",
        "    0,                       !- Resistive Defrost Heater Capacity {W}",
        "    No,                      !- Apply Part Load Fraction to Speeds Greater than 1",
        "    Electricity,             !- Fuel Type",
        "    ,                        !- Region number for Calculating HSPF",
        "    2,                       !- Number of Speeds",
        "    53500,                   !- Speed 1 Gross Rated Heating Capacity {W}",
        "    2.85,                    !- Speed 1 Gross Rated Heating COP {W/W}",
        "    3.209,                   !- Speed 1 Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Speed 1 Rated Supply Air Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Speed 1 Rated Supply Air Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    HeatingTempCurve,        !- Speed 1 Heating Capacity Function of Temperature Curve Name",
        "    HeatingFlowCurve,        !- Speed 1 Heating Capacity Function of Flow Fraction Curve Name",
        "    HeatingEIRTempCurve,     !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFlowCurve,            !- Speed 1 Energy Input Ratio Function of Flow Fraction Curve Name",
        "    PLFCurve,                !- Speed 1 Part Load Fraction Correlation Curve Name",
        "    0.0001,                  !- Speed 1 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    WasteHeatTempCurve,      !- Speed 1 Waste Heat Function of Temperature Curve Name",
        "    59587.2,                 !- Speed 2 Gross Rated Heating Capacity {W}",
        "    2.85,                    !- Speed 2 Gross Rated Heating COP {W/W}",
        "    3.209,                   !- Speed 2 Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Speed 2 Rated Supply Air Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Speed 2 Rated Supply Air Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    HeatingTempCurve,        !- Speed 2 Heating Capacity Function of Temperature Curve Name",
        "    HeatingFlowCurve,        !- Speed 2 Heating Capacity Function of Flow Fraction Curve Name",
        "    HeatingEIRTempCurve,     !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFlowCurve,            !- Speed 2 Energy Input Ratio Function of Flow Fraction Curve Name",
        "    PLFCurve,                !- Speed 2 Part Load Fraction Correlation Curve Name",
        "    0.0001,                  !- Speed 2 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    WasteHeatTempCurve;      !- Speed 2 Waste Heat Function of Temperature Curve Name",

        "  Coil:Heating:DX:MultiSpeed,",
        "    AC25Heating,             !- Name",
        "    AC-25 sched,             !- Availability Schedule Name",
        "    AC-25 HC inlet node,     !- Air Inlet Node Name",
        "    AC-25 RHC inlet node,    !- Air Outlet Node Name",
        "    -8,                      !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Outdoor Dry-Bulb Temperature to Turn On Compressor {C}",
        "    0,                       !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10,                      !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    DefrostTempCurve,        !- Defrost Energy Input Ratio Function of Temperature Curve Name",
        "    4.4,                     !- Maximum Outdoor Dry-Bulb Temperature for Defrost Operation {C}",
        "    ReverseCycle,            !- Defrost Strategy",
        "    OnDemand,                !- Defrost Control",
        "    0.087,                   !- Defrost Time Period Fraction",
        "    0,                       !- Resistive Defrost Heater Capacity {W}",
        "    No,                      !- Apply Part Load Fraction to Speeds Greater than 1",
        "    Electricity,             !- Fuel Type",
        "    ,                        !- Region number for Calculating HSPF",
        "    2,                       !- Number of Speeds",
        "    53500,                   !- Speed 1 Gross Rated Heating Capacity {W}",
        "    2.85,                    !- Speed 1 Gross Rated Heating COP {W/W}",
        "    3.209,                   !- Speed 1 Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Speed 1 Rated Supply Air Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Speed 1 Rated Supply Air Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    HeatingTempCurve,        !- Speed 1 Heating Capacity Function of Temperature Curve Name",
        "    HeatingFlowCurve,        !- Speed 1 Heating Capacity Function of Flow Fraction Curve Name",
        "    HeatingEIRTempCurve,     !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFlowCurve,            !- Speed 1 Energy Input Ratio Function of Flow Fraction Curve Name",
        "    PLFCurve,                !- Speed 1 Part Load Fraction Correlation Curve Name",
        "    0.0001,                  !- Speed 1 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    WasteHeatTempCurve,      !- Speed 1 Waste Heat Function of Temperature Curve Name",
        "    59587.2,                 !- Speed 2 Gross Rated Heating Capacity {W}",
        "    2.85,                    !- Speed 2 Gross Rated Heating COP {W/W}",
        "    3.209,                   !- Speed 2 Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Speed 2 Rated Supply Air Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Speed 1 Rated Supply Air Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    HeatingTempCurve,        !- Speed 2 Heating Capacity Function of Temperature Curve Name",
        "    HeatingFlowCurve,        !- Speed 2 Heating Capacity Function of Flow Fraction Curve Name",
        "    HeatingEIRTempCurve,     !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFlowCurve,            !- Speed 2 Energy Input Ratio Function of Flow Fraction Curve Name",
        "    PLFCurve,                !- Speed 2 Part Load Fraction Correlation Curve Name",
        "    0.0001,                  !- Speed 2 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    WasteHeatTempCurve;      !- Speed 2 Waste Heat Function of Temperature Curve Name",

        "!-   ===========  ALL OBJECTS IN CLASS: AIRLOOPHVAC:UNITARYHEATPUMP:AIRTOAIR:MULTISPEED ===========",
        "  AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed,",
        "    AC-24 heat pump,         !- Name",
        "    AC-24sched,              !- Availability Schedule Name",
        "    AC-24 SF inlet air node, !- Air Inlet Node Name",
        "    AC-24 airloop outlet node,  !- Air Outlet Node Name",
        "    401,                     !- Controlling Zone or Thermostat Location",
        "    Fan:OnOff,               !- Supply Air Fan Object Type",
        "    AC24_Fan,                !- Supply Air Fan Name",
        "    BlowThrough,             !- Supply Air Fan Placement",
        "    AC-24sched,              !- Supply Air Fan Operating Mode Schedule Name",
        "    Coil:Heating:DX:MultiSpeed,  !- Heating Coil Object Type",
        "    AC24Heating,             !- Heating Coil Name",
        "    ,                        !- DX Heating Coil Sizing Ratio",
        "    Coil:Cooling:DX:MultiSpeed,  !- Cooling Coil Object Type",
        "    AC24_cooling,            !- Cooling Coil Name",
        "    Coil:Heating:Electric,   !- Supplemental Heating Coil Object Type",
        "    AC24ElecHeater,          !- Supplemental Heating Coil Name",
        "    45,                      !- Maximum Supply Air Temperature from Supplemental Heater {C}",
        "    15,                      !- Maximum Outdoor Dry-Bulb Temperature for Supplemental Heater Operation {C}",
        "    0,                       !- Auxiliary On-Cycle Electric Power {W}",
        "    0,                       !- Auxiliary Off-Cycle Electric Power {W}",
        "    0,                       !- Design Heat Recovery Water Flow Rate {m3/s}",
        "    80,                      !- Maximum Temperature for Heat Recovery {C}",
        "    ,                        !- Heat Recovery Water Inlet Node Name",
        "    ,                        !- Heat Recovery Water Outlet Node Name",
        "    3.209,                   !- No Load Supply Air Flow Rate {m3/s}",
        "    2,                       !- Number of Speeds for Heating",
        "    2,                       !- Number of Speeds for Cooling",
        "    3.209,                   !- Heating Speed 1 Supply Air Flow Rate {m3/s}",
        "    3.209,                   !- Heating Speed 2 Supply Air Flow Rate {m3/s}",
        "    ,                        !- Heating Speed 3 Supply Air Flow Rate {m3/s}",
        "    ,                        !- Heating Speed 4 Supply Air Flow Rate {m3/s}",
        "    3.209,                   !- Cooling Speed 1 Supply Air Flow Rate {m3/s}",
        "    3.209;                   !- Cooling Speed 2 Supply Air Flow Rate {m3/s}",

        "  AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed,",
        "    AC-25 heat pump,         !- Name",
        "    AC-25 sched,             !- Availability Schedule Name",
        "    AC-25 SF inlet air node, !- Air Inlet Node Name",
        "    AC-25 airloop outlet node,  !- Air Outlet Node Name",
        "    402,                     !- Controlling Zone or Thermostat Location",
        "    Fan:OnOff,               !- Supply Air Fan Object Type",
        "    AC25_Fan,                !- Supply Air Fan Name",
        "    BlowThrough,             !- Supply Air Fan Placement",
        "    AC-25 sched,             !- Supply Air Fan Operating Mode Schedule Name",
        "    Coil:Heating:DX:MultiSpeed,  !- Heating Coil Object Type",
        "    AC25Heating,             !- Heating Coil Name",
        "    ,                        !- DX Heating Coil Sizing Ratio",
        "    Coil:Cooling:DX:MultiSpeed,  !- Cooling Coil Object Type",
        "    AC25_cooling,            !- Cooling Coil Name",
        "    Coil:Heating:Electric,   !- Supplemental Heating Coil Object Type",
        "    AC25ElecHeater,          !- Supplemental Heating Coil Name",
        "    45,                      !- Maximum Supply Air Temperature from Supplemental Heater {C}",
        "    15,                      !- Maximum Outdoor Dry-Bulb Temperature for Supplemental Heater Operation {C}",
        "    0,                       !- Auxiliary On-Cycle Electric Power {W}",
        "    0,                       !- Auxiliary Off-Cycle Electric Power {W}",
        "    0,                       !- Design Heat Recovery Water Flow Rate {m3/s}",
        "    80,                      !- Maximum Temperature for Heat Recovery {C}",
        "    ,                        !- Heat Recovery Water Inlet Node Name",
        "    ,                        !- Heat Recovery Water Outlet Node Name",
        "    3.209,                   !- No Load Supply Air Flow Rate {m3/s}",
        "    2,                       !- Number of Speeds for Heating",
        "    2,                       !- Number of Speeds for Cooling",
        "    3.209,                   !- Heating Speed 1 Supply Air Flow Rate {m3/s}",
        "    3.209,                   !- Heating Speed 2 Supply Air Flow Rate {m3/s}",
        "    ,                        !- Heating Speed 3 Supply Air Flow Rate {m3/s}",
        "    ,                        !- Heating Speed 4 Supply Air Flow Rate {m3/s}",
        "    3.209,                   !- Cooling Speed 1 Supply Air Flow Rate {m3/s}",
        "    3.209;                   !- Cooling Speed 2 Supply Air Flow Rate {m3/s}",

        "  Timestep,6;",

        "  Zone,",
        "    402,                     !- Name",
        "    48.33,                   !- Direction of Relative North {deg}",
        "    -21.306405,              !- X Origin {m}",
        "    46.910971,               !- Y Origin {m}",
        "    0.0,                     !- Z Origin {m}",
        "    ,                        !- Type",
        "    1;                       !- Multiplier",

        "  Zone,",
        "    401,                     !- Name",
        "    48.33,                   !- Direction of Relative North {deg}",
        "    15.002524,               !- X Origin {m}",
        "    -50.24491,               !- Y Origin {m}",
        "    0.0,                     !- Z Origin {m}",
        "    ,                        !- Type",
        "    1;                       !- Multiplier",

        "  Schedule:Compact,",
        "    ActSchd,                 !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,118;        !- Field 3",

        "  Schedule:Compact,",
        "    Space temp SP,           !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,22.00;      !- Field 3",

        "  Schedule:Compact,",
        "    Zone Control Type Sched, !- Name",
        "    Control Type,            !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,4.00;       !- Field 3",

        "  Schedule:Compact,",
        "    Fan_schd,                !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,1;          !- Field 17",

        "  Schedule:Compact,",
        "    401HeatingSP,            !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,21.11;      !- Field 7",

        "  Schedule:Compact,",
        "    401CoolingSP,            !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,22.2;       !- Field 7",

        "  Schedule:Compact,",
        "    HeatP fan cyc_sched,     !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,0;          !- Field 10",

        "  Schedule:Compact,",
        "    AC-28 sched,             !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,1;          !- Field 10",

        "  Schedule:Compact,",
        "    AC-25 sched,             !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,1.00;       !- Field 3",

        "  Schedule:Compact,",
        "    Econ sched,              !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,1.00;       !- Field 3",

        "  Schedule:Compact,",
        "    HeatingPlant ON,         !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,1.00;       !- Field 3",

        "  Schedule:Compact,",
        "    INFIL_QUARTER_ON_SCH,    !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,1.00;       !- Field 3",

        "  Schedule:Compact,",
        "    402HeatingSP,            !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,21.11;      !- Field 7",

        "  Schedule:Compact,",
        "    402CoolingSP,            !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,22.2;       !- Field 7",

        "  Schedule:Compact,",
        "    HeatingSetpoints,        !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,21.11;      !- Field 7",

        "  Schedule:Compact,",
        "    CoolingSetpoints,        !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,22.2;       !- Field 7",

        "  Schedule:Compact,",
        "    AC-24sched,              !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "	 For: Alldays,			  !- Field 2",
        "    Until: 24:00,1;          !- Field 3",

        "!-   ===========  ALL OBJECTS IN CLASS: CONTROLLER:OUTDOORAIR ===========",

        "  Controller:OutdoorAir,",
        "    AC-24 OA Controller,     !- Name",
        "    AC-24 EA node,           !- Relief Air Outlet Node Name",
        "    AC-24 airloop inlet node,!- Return Air Node Name",
        "    AC-24 SF inlet air node, !- Mixed Air Node Name",
        "    AC-24 OA inlet node,     !- Actuator Node Name",
        "    0.481,                   !- Minimum Outdoor Air Flow Rate {m3/s}",
        "    0.481,                   !- Maximum Outdoor Air Flow Rate {m3/s}",
        "    NoEconomizer,            !- Economizer Control Type",
        "    ,                        !- Economizer Control Action Type",
        "    ,                        !- Economizer Maximum Limit Dry-Bulb Temperature {C}",
        "    ,                        !- Economizer Maximum Limit Enthalpy {J/kg}",
        "    ,                        !- Economizer Maximum Limit Dewpoint Temperature {C}",
        "    ,                        !- Electronic Enthalpy Limit Curve Name",
        "    ,                        !- Economizer Minimum Limit Dry-Bulb Temperature {C}",
        "    ,                        !- Lockout Type",
        "    ;                        !- Minimum Limit Type",

        "  Controller:OutdoorAir,",
        "    AC-25 OA Controller,     !- Name",
        "    AC-25 EA node,           !- Relief Air Outlet Node Name",
        "    AC-25 airloop inlet node,!- Return Air Node Name",
        "    AC-25 SF inlet air node, !- Mixed Air Node Name",
        "    AC-25 OA inlet node,     !- Actuator Node Name",
        "    0.481,                   !- Minimum Outdoor Air Flow Rate {m3/s}",
        "    0.481,                   !- Maximum Outdoor Air Flow Rate {m3/s}",
        "    NoEconomizer,            !- Economizer Control Type",
        "    ,                        !- Economizer Control Action Type",
        "    ,                        !- Economizer Maximum Limit Dry-Bulb Temperature {C}",
        "    ,                        !- Economizer Maximum Limit Enthalpy {J/kg}",
        "    ,                        !- Economizer Maximum Limit Dewpoint Temperature {C}",
        "    ,                        !- Electronic Enthalpy Limit Curve Name",
        "    ,                        !- Economizer Minimum Limit Dry-Bulb Temperature {C}",
        "    ,                        !- Lockout Type",
        "    ;                        !- Minimum Limit Type",

        "!-   ===========  ALL OBJECTS IN CLASS: AIRLOOPHVAC:CONTROLLERLIST ===========",

        "  AirLoopHVAC:ControllerList,",
        "    Z401OA controller list,  !- Name",
        "    Controller:OutdoorAir,   !- Controller 1 Object Type",
        "    AC-24 OA Controller;     !- Controller 1 Name",

        "  AirLoopHVAC:ControllerList,",
        "    Z402OA controller list,  !- Name",
        "    Controller:OutdoorAir,   !- Controller 1 Object Type",
        "    AC-25 OA Controller;     !- Controller 1 Name",

        "!-   ===========  ALL OBJECTS IN CLASS: AIRLOOPHVAC ===========",

        "  AirLoopHVAC,",
        "    Z401 airloop,            !- Name",
        "    ,                        !- Controller List Name",
        "    AC24,                    !- Availability Manager List Name",
        "    3.209,                   !- Design Supply Air Flow Rate {m3/s}",
        "    Z401 branch list,        !- Branch List Name",
        "    ,                        !- Connector List Name",
        "    AC-24 airloop inlet node,!- Supply Side Inlet Node Name",
        "    Z401 RA node,            !- Demand Side Outlet Node Name",
        "    Z401 splitter inlet,     !- Demand Side Inlet Node Names",
        "    AC-24 airloop outlet node;  !- Supply Side Outlet Node Names",

        "  AirLoopHVAC,",
        "    Z402 airloop,            !- Name",
        "    ,                        !- Controller List Name",
        "    AC25,                    !- Availability Manager List Name",
        "    3.209,                   !- Design Supply Air Flow Rate {m3/s}",
        "    Z402 branch list,        !- Branch List Name",
        "    ,                        !- Connector List Name",
        "    AC-25 airloop inlet node,!- Supply Side Inlet Node Name",
        "    Z402 RA node,            !- Demand Side Outlet Node Name",
        "    Z402 splitter inlet,     !- Demand Side Inlet Node Names",
        "    AC-25 airloop outlet node;  !- Supply Side Outlet Node Names",

        "!-   ===========  ALL OBJECTS IN CLASS: AIRLOOPHVAC:OUTDOORAIRSYSTEM:EQUIPMENTLIST ===========",

        "  AirLoopHVAC:OutdoorAirSystem:EquipmentList,",
        "    Z401OA equip list,       !- Name",
        "    OutdoorAir:Mixer,        !- Component 1 Object Type",
        "    AC-24 OA intake;         !- Component 1 Name",

        "  AirLoopHVAC:OutdoorAirSystem:EquipmentList,",
        "    Z402OA equip list,       !- Name",
        "    OutdoorAir:Mixer,        !- Component 1 Object Type",
        "    AC-25 OA intake;         !- Component 1 Name",

        "!-   ===========  ALL OBJECTS IN CLASS: AIRLOOPHVAC:OUTDOORAIRSYSTEM ===========",

        "  AirLoopHVAC:OutdoorAirSystem,",
        "    Z401 OA Sys,             !- Name",
        "    Z401OA controller list,  !- Controller List Name",
        "    Z401OA equip list;       !- Outdoor Air Equipment List Name",

        "  AirLoopHVAC:OutdoorAirSystem,",
        "    Z402 OA Sys,             !- Name",
        "    Z402OA controller list,  !- Controller List Name",
        "    Z402OA equip list;       !- Outdoor Air Equipment List Name",

        "!-   ===========  ALL OBJECTS IN CLASS: OUTDOORAIR:MIXER ===========",

        "  OutdoorAir:Mixer,",
        "    AC-24 OA intake,         !- Name",
        "    AC-24 SF inlet air node, !- Mixed Air Node Name",
        "    AC-24 OA inlet node,     !- Outdoor Air Stream Node Name",
        "    AC-24 EA node,           !- Relief Air Stream Node Name",
        "    AC-24 airloop inlet node;!- Return Air Stream Node Name",

        "  OutdoorAir:Mixer,",
        "    AC-25 OA intake,         !- Name",
        "    AC-25 SF inlet air node, !- Mixed Air Node Name",
        "    AC-25 OA inlet node,     !- Outdoor Air Stream Node Name",
        "    AC-25 EA node,           !- Relief Air Stream Node Name",
        "    AC-25 airloop inlet node;!- Return Air Stream Node Name",

        "!-   ===========  ALL OBJECTS IN CLASS: AIRLOOPHVAC:ZONESPLITTER ===========",

        "  AirLoopHVAC:ZoneSplitter,",
        "    Z401 SA splitter,        !- Name",
        "    Z401 splitter inlet,     !- Inlet Node Name",
        "    Z401 zone inlet 2AT;     !- Outlet 1 Node Name",

        "  AirLoopHVAC:ZoneSplitter,",
        "    Z402 SA splitter,        !- Name",
        "    Z402 splitter inlet,     !- Inlet Node Name",
        "    Z402 zone inlet 2AT;     !- Outlet 1 Node Name",

        "!-   ===========  ALL OBJECTS IN CLASS: AIRLOOPHVAC:SUPPLYPATH ===========",

        "  AirLoopHVAC:SupplyPath,",
        "    Z401SupplyPath,          !- Name",
        "    Z401 splitter inlet,     !- Supply Air Path Inlet Node Name",
        "    AirLoopHVAC:ZoneSplitter,!- Component 1 Object Type",
        "    Z401 SA splitter;        !- Component 1 Name",

        "  AirLoopHVAC:SupplyPath,",
        "    Z402SupplyPath,          !- Name",
        "    Z402 splitter inlet,     !- Supply Air Path Inlet Node Name",
        "    AirLoopHVAC:ZoneSplitter,!- Component 1 Object Type",
        "    Z402 SA splitter;        !- Component 1 Name",

        "!-   ===========  ALL OBJECTS IN CLASS: AIRLOOPHVAC:ZONEMIXER ===========",

        "  AirLoopHVAC:ZoneMixer,",
        "    Z401 RA mixer,           !- Name",
        "    Z401 RA node,            !- Outlet Node Name",
        "    Z401 outlet node;        !- Inlet 1 Node Name",

        "  AirLoopHVAC:ZoneMixer,",
        "    Z402 RA mixer,           !- Name",
        "    Z402 RA node,            !- Outlet Node Name",
        "    Z402 outlet node;        !- Inlet 1 Node Name",

        "!-   ===========  ALL OBJECTS IN CLASS: AIRLOOPHVAC:RETURNPATH ===========",

        "  AirLoopHVAC:ReturnPath,",
        "    Z401RApath,              !- Name",
        "    Z401 RA node,            !- Return Air Path Outlet Node Name",
        "    AirLoopHVAC:ZoneMixer,   !- Component 1 Object Type",
        "    Z401 RA mixer;           !- Component 1 Name",

        "  AirLoopHVAC:ReturnPath,",
        "    Z402RApath,              !- Name",
        "    Z402 RA node,            !- Return Air Path Outlet Node Name",
        "    AirLoopHVAC:ZoneMixer,   !- Component 1 Object Type",
        "    Z402 RA mixer;           !- Component 1 Name",

        "!-   ===========  ALL OBJECTS IN CLASS: BRANCH ===========",

        "  Branch,",
        "    Z401 main branch,        !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    AirLoopHVAC:OutdoorAirSystem,  !- Component 1 Object Type",
        "    Z401 OA Sys,             !- Component 1 Name",
        "    AC-24 airloop inlet node,!- Component 1 Inlet Node Name",
        "    AC-24 SF inlet air node, !- Component 1 Outlet Node Name",
        "    AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed,  !- Component 2 Object Type",
        "    AC-24 heat pump,         !- Component 2 Name",
        "    AC-24 SF inlet air node, !- Component 2 Inlet Node Name",
        "    AC-24 airloop outlet node;  !- Component 2 Outlet Node Name",

        "  Branch,",
        "    Z402 main branch,        !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    AirLoopHVAC:OutdoorAirSystem,  !- Component 1 Object Type",
        "    Z402 OA Sys,             !- Component 1 Name",
        "    AC-25 airloop inlet node,!- Component 1 Inlet Node Name",
        "    AC-25 SF inlet air node, !- Component 1 Outlet Node Name",
        "    AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed,  !- Component 2 Object Type",
        "    AC-25 heat pump,         !- Component 2 Name",
        "    AC-25 SF inlet air node, !- Component 2 Inlet Node Name",
        "    AC-25 airloop outlet node;  !- Component 2 Outlet Node Name",

        "!-   ===========  ALL OBJECTS IN CLASS: BRANCHLIST ===========",

        "  BranchList,",
        "    Z401 branch list,        !- Name",
        "    Z401 main branch;        !- Branch 1 Name",

        "  BranchList,",
        "    Z402 branch list,        !- Name",
        "    Z402 main branch;        !- Branch 1 Name",

        "!-   ===========  ALL OBJECTS IN CLASS: NODELIST ===========",

        "  NodeList,",
        "    AC-24 OA intake list,    !- Name",
        "    AC-24 OA inlet node;     !- Node 1 Name",

        "  NodeList,",
        "    AC-25 OA intake list,    !- Name",
        "    AC-25 OA inlet node;     !- Node 1 Name",

        "!-   ===========  ALL OBJECTS IN CLASS: OUTDOORAIR:NODELIST ===========",

        "  OutdoorAir:NodeList,",
        "    AC-24 OA Intake list;    !- Node or NodeList Name 1",

        "  OutdoorAir:NodeList,",
        "    AC-25 OA Intake list;    !- Node or NodeList Name 1",

        "!-   ===========  ALL OBJECTS IN CLASS: AVAILABILITYMANAGER:SCHEDULED ===========",

        "  AvailabilityManager:Scheduled,",
        "    AC24 OnOff,              !- Name",
        "    AC-24sched;              !- Schedule Name",

        "  AvailabilityManager:Scheduled,",
        "    AC25 OnOff,              !- Name",
        "    AC-25 sched;             !- Schedule Name",

        "!-   ===========  ALL OBJECTS IN CLASS: AVAILABILITYMANAGERASSIGNMENTLIST ===========",

        "  AvailabilityManagerAssignmentList,",
        "    AC24,                    !- Name",
        "    AvailabilityManager:Scheduled,  !- Availability Manager 1 Object Type",
        "    AC24 OnOff;              !- Availability Manager 1 Name",

        "  AvailabilityManagerAssignmentList,",
        "    AC25,                    !- Name",
        "    AvailabilityManager:Scheduled,  !- Availability Manager 1 Object Type",
        "    AC25 OnOff;              !- Availability Manager 1 Name",

        "!-   ===========  ALL OBJECTS IN CLASS: CURVE:QUADRATIC ===========",

        "!-   ===========  ALL OBJECTS IN CLASS: CURVE:QUADRATIC ===========",

        "  Curve:Quadratic,",
        "    CoolingFlowCurve,        !- Name",
        "    1,                       !- Coefficient1 Constant",
        "    0,                       !- Coefficient2 x",
        "    0,                       !- Coefficient3 x**2",
        "    0.1,                     !- Minimum Value of x",
        "    2;                       !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    EIRFlowCurve,            !- Name",
        "    1,                       !- Coefficient1 Constant",
        "    0,                       !- Coefficient2 x",
        "    0,                       !- Coefficient3 x**2",
        "    0.1,                     !- Minimum Value of x",
        "    2;                       !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    PLFCurve,                !- Name",
        "    0.75,                    !- Coefficient1 Constant",
        "    0.25,                    !- Coefficient2 x",
        "    0,                       !- Coefficient3 x**2",
        "    0.1,                     !- Minimum Value of x",
        "    1;                       !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HeatingFlowCurve,        !- Name",
        "    1,                       !- Coefficient1 Constant",
        "    0,                       !- Coefficient2 x",
        "    0,                       !- Coefficient3 x**2",
        "    0.1,                     !- Minimum Value of x",
        "    2;                       !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    BLCoolingFlowCurve,      !- Name",
        "    0.86187,                 !- Coefficient1 Constant",
        "    0.14853,                 !- Coefficient2 x",
        "    -.009899,                !- Coefficient3 x**2",
        "    0.1,                     !- Minimum Value of x",
        "    2;                       !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    BLEIRFlowCurve,          !- Name",
        "    1.1085,                  !- Coefficient1 Constant",
        "    -.1266,                  !- Coefficient2 x",
        "    0.017998,                !- Coefficient3 x**2",
        "    0.1,                     !- Minimum Value of x",
        "    2;                       !- Maximum Value of x",

        "!-   ===========  ALL OBJECTS IN CLASS: CURVE:BIQUADRATIC ===========",

        "  Curve:Biquadratic,",
        "    CoolingTempCurve,        !- Name",
        "    0.66896,                 !- Coefficient1 Constant",
        "    0.023936,                !- Coefficient2 x",
        "    -.00015091,              !- Coefficient3 x**2",
        "    -.00128818,              !- Coefficient4 y",
        "    -.000168897,             !- Coefficient5 y**2",
        "    0.000258167,             !- Coefficient6 x*y",
        "    0,                       !- Minimum Value of x",
        "    20,                      !- Maximum Value of x",
        "    0,                       !- Minimum Value of y",
        "    40;                      !- Maximum Value of y",

        "  Curve:Biquadratic,",
        "    EIRTempCurve,            !- Name",
        "    0.574095,                !- Coefficient1 Constant",
        "    -.003463,                !- Coefficient2 x",
        "    0.00029161,              !- Coefficient3 x**2",
        "    0.012714,                !- Coefficient4 y",
        "    0.00034595,              !- Coefficient5 y**2",
        "    -.000714,                !- Coefficient6 x*y",
        "    0,                       !- Minimum Value of x",
        "    20,                      !- Maximum Value of x",
        "    0,                       !- Minimum Value of y",
        "    40;                      !- Maximum Value of y",

        "  Curve:Biquadratic,",
        "    WasteHeatFrac,           !- Name",
        "    0,                       !- Coefficient1 Constant",
        "    0,                       !- Coefficient2 x",
        "    0,                       !- Coefficient3 x**2",
        "    0,                       !- Coefficient4 y",
        "    0,                       !- Coefficient5 y**2",
        "    0,                       !- Coefficient6 x*y",
        "    0,                       !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    0,                       !- Minimum Value of y",
        "    50;                      !- Maximum Value of y",

        "  Curve:Biquadratic,",
        "    DefrostTempCurve,        !- Name",
        "    0.43264,                 !- Coefficient1 Constant",
        "    0.0013974,               !- Coefficient2 x",
        "    0.00026958,              !- Coefficient3 x**2",
        "    -.02639,                 !- Coefficient4 y",
        "    -.000142377,             !- Coefficient5 y**2",
        "    -.0016066,               !- Coefficient6 x*y",
        "    -10,                     !- Minimum Value of x",
        "    20,                      !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    50;                      !- Maximum Value of y",

        "  Curve:Biquadratic,",
        "    HeatingTempCurve,        !- Name",
        "    0.830527,                !- Coefficient1 Constant",
        "    -.00086702,              !- Coefficient2 x",
        "    -.00016316,              !- Coefficient3 x**2",
        "    0.022539,                !- Coefficient4 y",
        "    0.000142796,             !- Coefficient5 y**2",
        "    0.000046064,             !- Coefficient6 x*y",
        "    -10,                     !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    20;                      !- Maximum Value of y",

        "  Curve:Biquadratic,",
        "    HeatingEIRTempCurve,     !- Name",
        "    0.9111,                  !- Coefficient1 Constant",
        "    -.0106,                  !- Coefficient2 x",
        "    0.000879,                !- Coefficient3 x**2",
        "    0.0051,                  !- Coefficient4 y",
        "    0.00119,                 !- Coefficient5 y**2",
        "    -.00147,                 !- Coefficient6 x*y",
        "    -10,                     !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    20;                      !- Maximum Value of y",

        "  Curve:Biquadratic,",
        "    WasteHeatTempCurve,      !- Name",
        "    1,                       !- Coefficient1 Constant",
        "    0,                       !- Coefficient2 x",
        "    0,                       !- Coefficient3 x**2",
        "    0,                       !- Coefficient4 y",
        "    0,                       !- Coefficient5 y**2",
        "    0,                       !- Coefficient6 x*y",
        "    -10,                     !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    50;                      !- Maximum Value of y",

        "  Curve:Biquadratic,",
        "    AC28HeatingTempCurve,    !- Name",
        "    0.9427,                  !- Coefficient1 Constant",
        "    -.005897,                !- Coefficient2 x",
        "    -.00005806,              !- Coefficient3 x**2",
        "    0.02442,                 !- Coefficient4 y",
        "    0.0001265,               !- Coefficient5 y**2",
        "    -.000006653,             !- Coefficient6 x*y",
        "    -10,                     !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    50;                      !- Maximum Value of y",

        "  Curve:Biquadratic,",
        "    AC28HeatingEIRCurve,     !- Name",
        "    0.84489,                 !- Coefficient1 Constant",
        "    -.002944,                !- Coefficient2 x",
        "    0.0006633,               !- Coefficient3 x**2",
        "    0.0042478,               !- Coefficient4 y",
        "    0.00098998,              !- Coefficient5 y**2",
        "    -.0012759,               !- Coefficient6 x*y",
        "    -10,                     !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    50;                      !- Maximum Value of y",

        "  Curve:Biquadratic,",
        "    AC28DefrostTempCurve,    !- Name",
        "    0.65186,                 !- Coefficient1 Constant",
        "    -.0090539,               !- Coefficient2 x",
        "    0.00032925,              !- Coefficient3 x**2",
        "    0.001465,                !- Coefficient4 y",
        "    0.00043463,              !- Coefficient5 y**2",
        "    -.0031187,               !- Coefficient6 x*y",
        "    -10,                     !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    50;                      !- Maximum Value of y",

        "  Curve:Biquadratic,",
        "    AC28CoolingTempCurve,    !- Name",
        "    1.68985,                 !- Coefficient1 Constant",
        "    -.07506,                 !- Coefficient2 x",
        "    0.003149,                !- Coefficient3 x**2",
        "    -.00541,                 !- Coefficient4 y",
        "    0.00003154,              !- Coefficient5 y**2",
        "    -.0004042,               !- Coefficient6 x*y",
        "    -10,                     !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    50;                      !- Maximum Value of y",

        "  Curve:Biquadratic,",
        "    AC28EIRTempCurve,        !- Name",
        "    0.05573,                 !- Coefficient1 Constant",
        "    0.055223,                !- Coefficient2 x",
        "    -.0018377,               !- Coefficient3 x**2",
        "    0.012791,                !- Coefficient4 y",
        "    0.00019079,              !- Coefficient5 y**2",
        "    -.00016265,              !- Coefficient6 x*y",
        "    -10,                     !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    50;                      !- Maximum Value of y",

        "  Curve:Biquadratic,",
        "    AC29CoolingTempCurve,    !- Name",
        "    1.61164,                 !- Coefficient1 Constant",
        "    -.08591,                 !- Coefficient2 x",
        "    0.0032792,               !- Coefficient3 x**2",
        "    0.0038143,               !- Coefficient4 y",
        "    -.0001393,               !- Coefficient5 y**2",
        "    -.000229,                !- Coefficient6 x*y",
        "    -10,                     !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    50;                      !- Maximum Value of y",

        "  Curve:Biquadratic,",
        "    AC29EIRTempCurve,        !- Name",
        "    0.12954,                 !- Coefficient1 Constant",
        "    0.063575,                !- Coefficient2 x",
        "    -.0019562,               !- Coefficient3 x**2",
        "    0.0089583,               !- Coefficient4 y",
        "    0.00022236,              !- Coefficient5 y**2",
        "    -.0002964,               !- Coefficient6 x*y",
        "    -10,                     !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    50;                      !- Maximum Value of y",

        "  Curve:Biquadratic,",
        "    AC32CoolingTempCurve,    !- Name",
        "    0.48671,                 !- Coefficient1 Constant",
        "    0.075657,                !- Coefficient2 x",
        "    -.0016748,               !- Coefficient3 x**2",
        "    -.0158888,               !- Coefficient4 y",
        "    -.00010629,              !- Coefficient5 y**2",
        "    0.00053274,              !- Coefficient6 x*y",
        "    -10,                     !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    50;                      !- Maximum Value of y",

        "  Curve:Biquadratic,",
        "    BLCoolingTempCurve,      !- Name",
        "    0.54353,                 !- Coefficient1 Constant",
        "    0.020175,                !- Coefficient2 x",
        "    0.00034548,              !- Coefficient3 x**2",
        "    0.00085622,              !- Coefficient4 y",
        "    -.000085034,             !- Coefficient5 y**2",
        "    -.00007994,              !- Coefficient6 x*y",
        "    -10,                     !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    50;                      !- Maximum Value of y",

        "  Curve:Biquadratic,",
        "    BLEIRTempCurve,          !- Name",
        "    1.01728,                 !- Coefficient1 Constant",
        "    -.021724,                !- Coefficient2 x",
        "    0.00025326,              !- Coefficient3 x**2",
        "    0.017851,                !- Coefficient4 y",
        "    0.00014881,              !- Coefficient5 y**2",
        "    -.00043614,              !- Coefficient6 x*y",
        "    -10,                     !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    50;                      !- Maximum Value of y",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    HeatBalanceManager::GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);                            // zones are specified in the idf snippet

    // Get Zone Equipment Configuration data
    DataZoneEquipment::GetZoneEquipmentData(*state);
    state->dataZoneEquip->ZoneEquipList(1).EquipIndex(1) = 2; // 1st zone is 402, so this is 2nd direct air unit
    state->dataZoneEquip->ZoneEquipList(2).EquipIndex(1) = 1; // 2nd zone is 401, so this is 1st direct air unit
    MixedAir::GetOutsideAirSysInputs(*state);
    MixedAir::GetOAControllerInputs(*state);
    SplitterComponent::GetSplitterInput(*state);
    BranchInputManager::GetMixerInput(*state);
    BranchInputManager::ManageBranchInput(*state);
    GetZoneAirLoopEquipment(*state);
    SingleDuct::GetSysInput(*state);

    // Get Air Loop HVAC Data
    SimAirServingZones::GetAirPathData(*state);
    SimAirServingZones::InitAirLoops(*state, FirstHVACIteration);

    ZoneTempPredictorCorrector::GetZoneAirSetPoints(*state);

    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(2);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(2) = false;

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(2);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = -2500;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).OutputRequiredToHeatingSP = -20000;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).OutputRequiredToCoolingSP = -2500;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(2).RemainingOutputRequired = -2500;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(2).OutputRequiredToHeatingSP = -20000;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(2).OutputRequiredToCoolingSP = -2500;

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).SequencedOutputRequired.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).SequencedOutputRequiredToCoolingSP.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).SequencedOutputRequiredToHeatingSP.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(2).SequencedOutputRequired.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(2).SequencedOutputRequiredToCoolingSP.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(2).SequencedOutputRequiredToHeatingSP.allocate(1);

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).SequencedOutputRequired(1) =
        state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).SequencedOutputRequiredToCoolingSP(1) =
        state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).OutputRequiredToCoolingSP;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).SequencedOutputRequiredToHeatingSP(1) =
        state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).OutputRequiredToHeatingSP;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(2).SequencedOutputRequired(1) =
        state->dataZoneEnergyDemand->ZoneSysEnergyDemand(2).RemainingOutputRequired;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(2).SequencedOutputRequiredToCoolingSP(1) =
        state->dataZoneEnergyDemand->ZoneSysEnergyDemand(2).OutputRequiredToCoolingSP;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(2).SequencedOutputRequiredToHeatingSP(1) =
        state->dataZoneEnergyDemand->ZoneSysEnergyDemand(2).OutputRequiredToHeatingSP;

    HVACMultiSpeedHeatPump::GetMSHeatPumpInput(*state);

    state->dataGlobal->SysSizingCalc = true; // disable sizing calculation
    state->dataHVACMultiSpdHP->MSHeatPump(1).TotHeatEnergyRate = 1000.0;
    state->dataHVACMultiSpdHP->MSHeatPump(1).TotCoolEnergyRate = 1000.0;
    state->dataHVACMultiSpdHP->MSHeatPump(2).TotHeatEnergyRate = 1000.0;
    state->dataHVACMultiSpdHP->MSHeatPump(2).TotCoolEnergyRate = 1000.0;
    state->dataHVACMultiSpdHP->MSHeatPump(1).FlowFraction = 1.0;
    state->dataHVACMultiSpdHP->MSHeatPump(2).FlowFraction = 1.0;
    // because sizing isn't occurring, we must set these
    for (auto &dxCoil : state->dataDXCoils->DXCoil) {
        for (int i = 1; i <= dxCoil.NumOfSpeeds; ++i) {
            dxCoil.MSRatedAirMassFlowRate(i) = dxCoil.MSRatedAirVolFlowRate(i) * 1.2;
        }
    }

    Sched::GetSchedule(*state, "AC-24SCHED")->currentVal = 1.0;
    Sched::GetSchedule(*state, "AC-25 SCHED")->currentVal = 1.0;
    state->dataEnvrn->StdRhoAir = 1.2;
    state->dataEnvrn->OutDryBulbTemp = 35.0;
    state->dataEnvrn->OutHumRat = 0.012;
    state->dataEnvrn->StdBaroPress = 101325.0;
    state->dataEnvrn->OutBaroPress = 101325.0;

    // InitMSHeatPump resets the current MSHeatPumpNum only
    HVACMultiSpeedHeatPump::InitMSHeatPump(*state, MSHeatPumpNum, FirstHVACIteration, AirLoopNum, QZnReq, OnOffAirFlowRatio);

    EXPECT_DOUBLE_EQ(1000.0, state->dataHVACMultiSpdHP->MSHeatPump(1).TotHeatEnergyRate);
    EXPECT_DOUBLE_EQ(1000.0, state->dataHVACMultiSpdHP->MSHeatPump(1).TotCoolEnergyRate);
    EXPECT_DOUBLE_EQ(0.0, state->dataHVACMultiSpdHP->MSHeatPump(2).TotHeatEnergyRate);
    EXPECT_DOUBLE_EQ(0.0, state->dataHVACMultiSpdHP->MSHeatPump(2).TotCoolEnergyRate);

    // verify min OAT from coil inputs
    EXPECT_EQ(state->dataHVACMultiSpdHP->MSHeatPump(1).MinOATCompressorCooling, -25.0);
    EXPECT_EQ(state->dataHVACMultiSpdHP->MSHeatPump(1).MinOATCompressorHeating, -8.0);
    EXPECT_EQ(state->dataHVACMultiSpdHP->MSHeatPump(2).MinOATCompressorCooling, -25.0);
    EXPECT_EQ(state->dataHVACMultiSpdHP->MSHeatPump(2).MinOATCompressorHeating, -8.0);

    state->dataLoopNodes->Node(9).Temp = 24.0;
    state->dataLoopNodes->Node(9).HumRat = 0.008;
    state->dataLoopNodes->Node(6).Temp = 24.0;
    state->dataLoopNodes->Node(6).HumRat = 0.008;
    state->dataLoopNodes->Node(16).Temp = 24.0;
    state->dataLoopNodes->Node(16).HumRat = 0.008;
    state->dataLoopNodes->Node(16).Enthalpy = Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(16).Temp, state->dataLoopNodes->Node(16).HumRat);
    state->dataLoopNodes->Node(24).MassFlowRateMax = state->dataLoopNodes->Node(16).MassFlowRateMaxAvail;

    state->dataFans->fans(2)->maxAirMassFlowRate = state->dataLoopNodes->Node(16).MassFlowRateMaxAvail;
    state->dataFans->fans(2)->rhoAirStdInit = state->dataEnvrn->StdRhoAir;
    state->dataDXCoils->DXCoil(2).MSRatedAirMassFlowRate(1) = state->dataDXCoils->DXCoil(2).MSRatedAirVolFlowRate(1) * state->dataEnvrn->StdRhoAir;
    state->dataDXCoils->DXCoil(2).MSRatedAirMassFlowRate(2) = state->dataDXCoils->DXCoil(2).MSRatedAirVolFlowRate(2) * state->dataEnvrn->StdRhoAir;
    state->dataDXCoils->DXCoil(2).MSRatedCBF(1) = 0.2;
    state->dataDXCoils->DXCoil(2).MSRatedCBF(2) = 0.2;

    Real64 QSensUnitOut;
    // Cooling
    SimMSHP(*state, MSHeatPumpNum, FirstHVACIteration, AirLoopNum, QSensUnitOut, QZnReq, OnOffAirFlowRatio);
    // Check outlet conditions
    EXPECT_NEAR(state->dataLoopNodes->Node(22).Temp, 23.363295, 0.0001);
    EXPECT_NEAR(state->dataLoopNodes->Node(22).HumRat, 0.00796611, 0.0001);
    EXPECT_NEAR(state->dataLoopNodes->Node(22).Enthalpy, 43748.243, 0.0001);
    EXPECT_NEAR(state->dataHVACMultiSpdHP->MSHeatPump(2).CompPartLoadRatio, 0.1232, 0.0001);

    // Direct solution
    state->dataGlobal->DoCoilDirectSolutions = true;
    state->dataHVACMultiSpdHP->MSHeatPump(2).FullOutput.allocate(2);
    SimMSHP(*state, MSHeatPumpNum, FirstHVACIteration, AirLoopNum, QSensUnitOut, QZnReq, OnOffAirFlowRatio);
    // Check outlet conditions
    EXPECT_NEAR(state->dataLoopNodes->Node(22).Temp, 23.363295, 0.0001);
    EXPECT_NEAR(state->dataLoopNodes->Node(22).HumRat, 0.00796611, 0.0001);
    EXPECT_NEAR(state->dataLoopNodes->Node(22).Enthalpy, 43748.243, 0.0001);
    EXPECT_NEAR(state->dataHVACMultiSpdHP->MSHeatPump(2).CompPartLoadRatio, 0.1232, 0.0001);

    QZnReq = -10000.00;

    state->dataGlobal->DoCoilDirectSolutions = false;
    SimMSHP(*state, MSHeatPumpNum, FirstHVACIteration, AirLoopNum, QSensUnitOut, QZnReq, OnOffAirFlowRatio);
    EXPECT_NEAR(state->dataLoopNodes->Node(22).Temp, 21.45298, 0.0001);
    EXPECT_NEAR(state->dataLoopNodes->Node(22).HumRat, 0.00792169, 0.0001);
    EXPECT_NEAR(state->dataLoopNodes->Node(22).Enthalpy, 41691.15, 0.0001);
    EXPECT_NEAR(state->dataHVACMultiSpdHP->MSHeatPump(2).CompPartLoadRatio, 0.285417, 0.0001);

    state->dataGlobal->DoCoilDirectSolutions = true;
    SimMSHP(*state, MSHeatPumpNum, FirstHVACIteration, AirLoopNum, QSensUnitOut, QZnReq, OnOffAirFlowRatio);
    EXPECT_NEAR(state->dataLoopNodes->Node(22).Temp, 21.45298, 0.0001);
    EXPECT_NEAR(state->dataLoopNodes->Node(22).HumRat, 0.00792169, 0.0001);
    EXPECT_NEAR(state->dataLoopNodes->Node(22).Enthalpy, 41691.15, 0.0001);
    EXPECT_NEAR(state->dataHVACMultiSpdHP->MSHeatPump(2).CompPartLoadRatio, 0.285417, 0.0001);

    // Heating
    QZnReq = 10000.00;
    state->dataHVACMultiSpdHP->MSHeatPump(2).HeatCoolMode = EnergyPlus::HVACMultiSpeedHeatPump::ModeOfOperation::HeatingMode;
    state->dataEnvrn->OutDryBulbTemp = 5.0;
    state->dataEnvrn->OutHumRat = 0.008;
    state->dataGlobal->DoCoilDirectSolutions = false;
    SimMSHP(*state, MSHeatPumpNum, FirstHVACIteration, AirLoopNum, QSensUnitOut, QZnReq, OnOffAirFlowRatio);
    EXPECT_NEAR(state->dataLoopNodes->Node(22).Temp, 26.546664, 0.0001);
    EXPECT_NEAR(state->dataLoopNodes->Node(22).HumRat, 0.008, 0.0001);
    EXPECT_NEAR(state->dataLoopNodes->Node(22).Enthalpy, 47077.4613, 0.0001);
    EXPECT_NEAR(state->dataHVACMultiSpdHP->MSHeatPump(2).CompPartLoadRatio, 0.1530992, 0.0001);
    state->dataGlobal->DoCoilDirectSolutions = true;
    SimMSHP(*state, MSHeatPumpNum, FirstHVACIteration, AirLoopNum, QSensUnitOut, QZnReq, OnOffAirFlowRatio);
    EXPECT_NEAR(state->dataLoopNodes->Node(22).Temp, 26.546664, 0.0001);
    EXPECT_NEAR(state->dataLoopNodes->Node(22).HumRat, 0.008, 0.0001);
    EXPECT_NEAR(state->dataLoopNodes->Node(22).Enthalpy, 47077.4613, 0.0001);
    EXPECT_NEAR(state->dataHVACMultiSpdHP->MSHeatPump(2).CompPartLoadRatio, 0.1530992, 0.0001);

    state->dataGlobal->DoCoilDirectSolutions = false;

    state->dataHVACMultiSpdHP->MSHeatPump(2).EMSOverrideCoilSpeedNumOn = true;
    state->dataHVACMultiSpdHP->MSHeatPump(2).EMSOverrideCoilSpeedNumValue = 0.1530992;

    SimMSHP(*state, MSHeatPumpNum, FirstHVACIteration, AirLoopNum, QSensUnitOut, QZnReq, OnOffAirFlowRatio);
    EXPECT_EQ(state->dataHVACMultiSpdHP->MSHeatPumpReport(2).SpeedNum, 1);
    EXPECT_NEAR(state->dataHVACMultiSpdHP->MSHeatPumpReport(2).CycRatio, 0.1530992, 0.0001);
    EXPECT_NEAR(state->dataHVACMultiSpdHP->MSHeatPump(2).CompPartLoadRatio, 0.1530992, 0.0001);
    EXPECT_NEAR(state->dataLoopNodes->Node(22).Temp, 26.546664, 0.001);

    QZnReq = 50000.00;
    // when speed overwrite value is exactly 1.0, use full load of speed 1
    state->dataHVACMultiSpdHP->MSHeatPump(2).EMSOverrideCoilSpeedNumValue = 1;
    SimMSHP(*state, MSHeatPumpNum, FirstHVACIteration, AirLoopNum, QSensUnitOut, QZnReq, OnOffAirFlowRatio);
    EXPECT_EQ(state->dataHVACMultiSpdHP->MSHeatPumpReport(2).SpeedNum, 1);
    // when speed overwrite value is 1.2 (between 1.0 and 2.0), use speed 2 with cycling ratio 0.2
    state->dataHVACMultiSpdHP->MSHeatPump(2).EMSOverrideCoilSpeedNumValue = 1.2;
    SimMSHP(*state, MSHeatPumpNum, FirstHVACIteration, AirLoopNum, QSensUnitOut, QZnReq, OnOffAirFlowRatio);
    EXPECT_EQ(state->dataHVACMultiSpdHP->MSHeatPumpReport(2).SpeedNum, 2);
    EXPECT_NEAR(state->dataHVACMultiSpdHP->MSHeatPumpReport(2).SpeedRatio, 0.2, 0.001);
    EXPECT_NEAR(state->dataHVACMultiSpdHP->MSHeatPump(2).CompPartLoadRatio, 0.2, 0.001);
    // SpeedNumValue = 1.2 is overheating under QZnReq = 50000.00
    EXPECT_GT(state->dataLoopNodes->Node(22).Temp, 26.546664);

    state->dataHVACMultiSpdHP->MSHeatPump(2).EMSOverrideCoilSpeedNumValue = 2.2;
    SimMSHP(*state, MSHeatPumpNum, FirstHVACIteration, AirLoopNum, QSensUnitOut, QZnReq, OnOffAirFlowRatio);
    EXPECT_EQ(state->dataHVACMultiSpdHP->MSHeatPumpReport(2).SpeedNum, 2);
    EXPECT_NEAR(state->dataHVACMultiSpdHP->MSHeatPumpReport(2).SpeedRatio, 1.0, 0.001);

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.deallocate();
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.deallocate();
}

TEST_F(EnergyPlusFixture, HVACMultiSpeedHeatPump_HeatRecoveryTest)
{
    state->init_state(*state);
    state->dataLoopNodes->Node.allocate(2);
    state->dataHVACMultiSpdHP->MSHeatPump.allocate(1);
    int HeatRecInNode(1);
    int HeatRecOutNode(2);
    state->dataHVACMultiSpdHP->MSHeatPump(1).HeatRecInletNodeNum = HeatRecInNode;
    state->dataHVACMultiSpdHP->MSHeatPump(1).HeatRecOutletNodeNum = HeatRecOutNode;
    state->dataHVACMultiSpdHP->MSHeatPump(1).MaxHeatRecOutletTemp = 80;
    state->dataHVACMultiSpdHP->MSHeatPump(1).HRPlantLoc.loopNum = 1; // index to plant
    state->dataLoopNodes->Node(HeatRecInNode).Temp = 50.0;
    state->dataHVACGlobal->MSHPWasteHeat = 1000.0;

    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);

    state->dataLoopNodes->Node(HeatRecInNode).MassFlowRate = 0.0; // test heat recovery result with 0 water flow rate
    HVACMultiSpeedHeatPump::MSHPHeatRecovery(*state, 1);

    // outlet temp should equal inlet temp since mass flow rate = 0
    Real64 calculatedOutletTemp = state->dataLoopNodes->Node(HeatRecInNode).Temp;
    EXPECT_DOUBLE_EQ(0.0, state->dataHVACMultiSpdHP->MSHeatPump(1).HeatRecoveryRate);
    EXPECT_DOUBLE_EQ(50.0, state->dataHVACMultiSpdHP->MSHeatPump(1).HeatRecoveryInletTemp);
    EXPECT_DOUBLE_EQ(50.0, state->dataHVACMultiSpdHP->MSHeatPump(1).HeatRecoveryOutletTemp);
    EXPECT_DOUBLE_EQ(0.0, state->dataHVACMultiSpdHP->MSHeatPump(1).HeatRecoveryMassFlowRate);

    state->dataLoopNodes->Node(HeatRecInNode).MassFlowRate =
        0.1; // initialize flow rate and test heat recovery result using 1 kW heat transfer to fluid
    HVACMultiSpeedHeatPump::MSHPHeatRecovery(*state, 1);

    // outlet temp should equal temperature rise due to 1 kW of heat input at 0.1 kg/s
    calculatedOutletTemp = state->dataLoopNodes->Node(HeatRecInNode).Temp +
                           state->dataHVACGlobal->MSHPWasteHeat / (state->dataLoopNodes->Node(HeatRecInNode).MassFlowRate * 4181.0);
    EXPECT_DOUBLE_EQ(1000.0, state->dataHVACMultiSpdHP->MSHeatPump(1).HeatRecoveryRate);
    EXPECT_DOUBLE_EQ(50.0, state->dataHVACMultiSpdHP->MSHeatPump(1).HeatRecoveryInletTemp);
    EXPECT_DOUBLE_EQ(calculatedOutletTemp, state->dataHVACMultiSpdHP->MSHeatPump(1).HeatRecoveryOutletTemp);
    EXPECT_DOUBLE_EQ(52.391772303276724, calculatedOutletTemp);
    EXPECT_DOUBLE_EQ(0.1, state->dataHVACMultiSpdHP->MSHeatPump(1).HeatRecoveryMassFlowRate);

    state->dataHVACGlobal->MSHPWasteHeat = 100000.0; // test very high heat transfer that would limit outlet water temperature
    state->dataLoopNodes->Node(HeatRecInNode).MassFlowRate = 0.1;
    HVACMultiSpeedHeatPump::MSHPHeatRecovery(*state, 1);

    // outlet temp should equal max limit of 80 C since 100 kW would cause outlet water temperature to exceed 80 C
    EXPECT_DOUBLE_EQ(50.0, state->dataHVACMultiSpdHP->MSHeatPump(1).HeatRecoveryInletTemp);
    EXPECT_DOUBLE_EQ(80.0, state->dataHVACMultiSpdHP->MSHeatPump(1).HeatRecoveryOutletTemp);
    EXPECT_DOUBLE_EQ(0.1, state->dataHVACMultiSpdHP->MSHeatPump(1).HeatRecoveryMassFlowRate);
    Real64 QHeatRecovery =
        state->dataLoopNodes->Node(HeatRecInNode).MassFlowRate * 4181.0 *
        (state->dataHVACMultiSpdHP->MSHeatPump(1).HeatRecoveryOutletTemp - state->dataHVACMultiSpdHP->MSHeatPump(1).HeatRecoveryInletTemp);
    EXPECT_DOUBLE_EQ(QHeatRecovery, state->dataHVACMultiSpdHP->MSHeatPump(1).HeatRecoveryRate);
    EXPECT_DOUBLE_EQ(12543.0, QHeatRecovery);
}

TEST_F(EnergyPlusFixture, HVACMSHP_UnitarySystemElectricityRateTest)
{

    bool ErrorsFound(false);
    int constexpr MSHeatPumpNum(1);
    bool const FirstHVACIteration(true);
    int constexpr AirLoopNum(1);
    Real64 QZnReq(-10000.0);
    Real64 OnOffAirFlowRatio(1.0);

    std::string const idf_objects = delimited_string({

        "!-   ===========  ALL OBJECTS IN CLASS: ZONECONTROL:THERMOSTAT ===========",

        "  ZoneControl:Thermostat,",
        "    Z401TempCtrl,            !- Name",
        "    401,                     !- Zone or ZoneList Name",
        "    Zone Control Type Sched, !- Control Type Schedule Name",
        "    ThermostatSetpoint:DualSetpoint,   !- Control 3 Object Type",
        "    DualSetPoint1;            !- Control 3 Name",

        "  ThermostatSetpoint:DualSetpoint,",
        "    DualSetPoint1,            !- Name",
        "    401HeatingSP,            !- Heating Setpoint Temperature Schedule Name",
        "    401CoolingSP;            !- Cooling Setpoint Temperature Schedule Name",

        "!-   ===========  ALL OBJECTS IN CLASS: AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:NOREHEAT ===========",

        "  AirTerminal:SingleDuct:ConstantVolume:NoReheat,",
        "    401directair,            !- Name",
        "    AC-24sched,              !- Availability Schedule Name",
        "    Z401 zone inlet 2AT,     !- Air Inlet Node Name",
        "    Z401 zone inlet,         !- Air Outlet Node Name",
        "    3.209;                   !- Maximum Air Flow Rate {m3/s}",
        "!-   ===========  ALL OBJECTS IN CLASS: ZONEHVAC:EQUIPMENTLIST ===========",

        "  ZoneHVAC:EquipmentList,",
        "    Z401 terminal list,      !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    401directairADU,         !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "!-   ===========  ALL OBJECTS IN CLASS: ZONEHVAC:AIRDISTRIBUTIONUNIT ===========",

        "  ZoneHVAC:AirDistributionUnit,",
        "    401directairADU,         !- Name",
        "    Z401 zone inlet,         !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:ConstantVolume:NoReheat,  !- Air Terminal Object Type",
        "    401directair;            !- Air Terminal Name",

        "!-   ===========  ALL OBJECTS IN CLASS: ZONEHVAC:EQUIPMENTCONNECTIONS ===========",

        "  ZoneHVAC:EquipmentConnections,",
        "    401,                     !- Zone Name",
        "    Z401 terminal list,      !- Zone Conditioning Equipment List Name",
        "    Z401 zone inlet,         !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    Z401 air node,           !- Zone Air Node Name",
        "    Z401 outlet node;        !- Zone Return Air Node Name",

        "!-   ===========  ALL OBJECTS IN CLASS: FAN:ONOFF ===========",
        "  Fan:OnOff,",
        "    AC24_Fan,                !- Name",
        "    AC-24sched,              !- Availability Schedule Name",
        "    0.25,                    !- Fan Total Efficiency",
        "    249.089,                 !- Pressure Rise {Pa}",
        "    3.209,                   !- Maximum Flow Rate {m3/s}",
        "    0.85,                    !- Motor Efficiency",
        "    1,                       !- Motor In Airstream Fraction",
        "    AC-24 SF inlet air node, !- Air Inlet Node Name",
        "    AC-24 SF outlet air node,!- Air Outlet Node Name",
        "    ,                        !- Fan Power Ratio Function of Speed Ratio Curve Name",
        "    ,                        !- Fan Efficiency Ratio Function of Speed Ratio Curve Name",
        "    General;                 !- End-Use Subcategory",
        "  Coil:Cooling:DX:MultiSpeed,",
        "    AC24_cooling,            !- Name",
        "    AC-24sched,              !- Availability Schedule Name",
        "    AC-24 SF outlet air node,!- Air Inlet Node Name",
        "    AC-24 HC inlet node,     !- Air Outlet Node Name",
        "    ,                        !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    No,                      !- Apply Part Load Fraction to Speeds Greater than 1",
        "    No,                      !- Apply Latent Degradation to Speeds Greater than 1",
        "    0,                       !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10,                      !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    Electricity,             !- Fuel Type",
        "    2,                       !- Number of Speeds",
        "    53500,                   !- Speed 1 Gross Rated Total Cooling Capacity {W}",
        "    0.737,                   !- Speed 1 Gross Rated Sensible Heat Ratio",
        "    3.42,                    !- Speed 1 Gross Rated Cooling COP {W/W}",
        "    3.209,                   !- Speed 1 Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Speed 1 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Speed 1 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    CoolingTempCurve,        !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "    CoolingFlowCurve,        !- Speed 1 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    EIRTempCurve,            !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFlowCurve,            !- Speed 1 Energy Input Ratio Function of Flow Fraction Curve Name",
        "    PLFCurve,                !- Speed 1 Part Load Fraction Correlation Curve Name",
        "    ,                        !- Speed 1 Nominal Time for Condensate Removal to Begin {s}",
        "    ,                        !- Speed 1 Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity {dimensionless}",
        "    3,                       !- Speed 1 Maximum Cycling Rate {cycles/hr}",
        "    ,                        !- Speed 1 Latent Capacity Time Constant {s}",
        "    0.001,                   !- Speed 1 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    WasteHeatTempCurve,      !- Speed 1 Waste Heat Function of Temperature Curve Name",
        "    0.9,                     !- Speed 1 Evaporative Condenser Effectiveness {dimensionless}",
        "    ,                        !- Speed 1 Evaporative Condenser Air Flow Rate {m3/s}",
        "    ,                        !- Speed 1 Rated Evaporative Condenser Pump Power Consumption {W}",
        "    67992.5,                 !- Speed 2 Gross Rated Total Cooling Capacity {W}",
        "    0.737,                   !- Speed 2 Gross Rated Sensible Heat Ratio",
        "    3.42,                    !- Speed 2 Gross Rated Cooling COP {W/W}",
        "    3.209,                   !- Speed 2 Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Speed 2 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Speed 2 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    CoolingTempCurve,        !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "    CoolingFlowCurve,        !- Speed 2 Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    EIRTempCurve,            !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFlowCurve,            !- Speed 2 Energy Input Ratio Function of Flow Fraction Curve Name",
        "    PLFCurve,                !- Speed 2 Part Load Fraction Correlation Curve Name",
        "    ,                        !- Speed 2 Nominal Time for Condensate Removal to Begin {s}",
        "    ,                        !- Speed 2 Ratio of Initial Moisture Evaporation Rate and steady state Latent Capacity {dimensionless}",
        "    0,                       !- Speed 2 Maximum Cycling Rate {cycles/hr}",
        "    ,                        !- Speed 2 Latent Capacity Time Constant {s}",
        "    0.001,                   !- Speed 2 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    WasteHeatTempCurve,      !- Speed 2 Waste Heat Function of Temperature Curve Name",
        "    0.9,                     !- Speed 2 Evaporative Condenser Effectiveness {dimensionless}",
        "    ,                        !- Speed 2 Evaporative Condenser Air Flow Rate {m3/s}",
        "    ;                        !- Speed 2 Rated Evaporative Condenser Pump Power Consumption {W}",

        "!-   ===========  ALL OBJECTS IN CLASS: COIL:HEATING:ELECTRIC ===========",
        "  Coil:Heating:Electric,",
        "    AC24ElecHeater,          !- Name",
        "    AC-24sched,              !- Availability Schedule Name",
        "    1,                       !- Efficiency",
        "    55000,                   !- Nominal Capacity {W}",
        "    AC-24 RHC inlet node,    !- Air Inlet Node Name",
        "    AC-24 airloop outlet node;  !- Air Outlet Node Name",

        "!-   ===========  ALL OBJECTS IN CLASS: COIL:HEATING:DX:MULTISPEED ===========",
        "  Coil:Heating:DX:MultiSpeed,",
        "    AC24Heating,             !- Name",
        "    AC-24sched,              !- Availability Schedule Name",
        "    AC-24 HC inlet node,     !- Air Inlet Node Name",
        "    AC-24 RHC inlet node,    !- Air Outlet Node Name",
        "    -8,                      !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Outdoor Dry-Bulb Temperature to Turn On Compressor {C}",
        "    0,                       !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10,                      !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    DefrostTempCurve,        !- Defrost Energy Input Ratio Function of Temperature Curve Name",
        "    4.4,                     !- Maximum Outdoor Dry-Bulb Temperature for Defrost Operation {C}",
        "    ReverseCycle,            !- Defrost Strategy",
        "    OnDemand,                !- Defrost Control",
        "    0.087,                   !- Defrost Time Period Fraction",
        "    0,                       !- Resistive Defrost Heater Capacity {W}",
        "    No,                      !- Apply Part Load Fraction to Speeds Greater than 1",
        "    Electricity,             !- Fuel Type",
        "    ,                        !- Region number for Calculating HSPF",
        "    2,                       !- Number of Speeds",
        "    53500,                   !- Speed 1 Gross Rated Heating Capacity {W}",
        "    2.85,                    !- Speed 1 Gross Rated Heating COP {W/W}",
        "    3.209,                   !- Speed 1 Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Speed 1 Rated Supply Air Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Speed 1 Rated Supply Air Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    HeatingTempCurve,        !- Speed 1 Heating Capacity Function of Temperature Curve Name",
        "    HeatingFlowCurve,        !- Speed 1 Heating Capacity Function of Flow Fraction Curve Name",
        "    HeatingEIRTempCurve,     !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFlowCurve,            !- Speed 1 Energy Input Ratio Function of Flow Fraction Curve Name",
        "    PLFCurve,                !- Speed 1 Part Load Fraction Correlation Curve Name",
        "    0.0001,                  !- Speed 1 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    WasteHeatTempCurve,      !- Speed 1 Waste Heat Function of Temperature Curve Name",
        "    59587.2,                 !- Speed 2 Gross Rated Heating Capacity {W}",
        "    2.85,                    !- Speed 2 Gross Rated Heating COP {W/W}",
        "    3.209,                   !- Speed 2 Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Speed 2 Rated Supply Air Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Speed 2 Rated Supply Air Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    HeatingTempCurve,        !- Speed 2 Heating Capacity Function of Temperature Curve Name",
        "    HeatingFlowCurve,        !- Speed 2 Heating Capacity Function of Flow Fraction Curve Name",
        "    HeatingEIRTempCurve,     !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFlowCurve,            !- Speed 2 Energy Input Ratio Function of Flow Fraction Curve Name",
        "    PLFCurve,                !- Speed 2 Part Load Fraction Correlation Curve Name",
        "    0.0001,                  !- Speed 2 Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    WasteHeatTempCurve;      !- Speed 2 Waste Heat Function of Temperature Curve Name",

        "!-   ===========  ALL OBJECTS IN CLASS: AIRLOOPHVAC:UNITARYHEATPUMP:AIRTOAIR:MULTISPEED ===========",
        "  AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed,",
        "    AC-24 heat pump,         !- Name",
        "    AC-24sched,              !- Availability Schedule Name",
        "    AC-24 SF inlet air node, !- Air Inlet Node Name",
        "    AC-24 airloop outlet node,  !- Air Outlet Node Name",
        "    401,                     !- Controlling Zone or Thermostat Location",
        "    Fan:OnOff,               !- Supply Air Fan Object Type",
        "    AC24_Fan,                !- Supply Air Fan Name",
        "    BlowThrough,             !- Supply Air Fan Placement",
        "    AC-24sched,              !- Supply Air Fan Operating Mode Schedule Name",
        "    Coil:Heating:DX:MultiSpeed,  !- Heating Coil Object Type",
        "    AC24Heating,             !- Heating Coil Name",
        "    ,                        !- DX Heating Coil Sizing Ratio",
        "    Coil:Cooling:DX:MultiSpeed,  !- Cooling Coil Object Type",
        "    AC24_cooling,            !- Cooling Coil Name",
        "    Coil:Heating:Electric,   !- Supplemental Heating Coil Object Type",
        "    AC24ElecHeater,          !- Supplemental Heating Coil Name",
        "    45,                      !- Maximum Supply Air Temperature from Supplemental Heater {C}",
        "    15,                      !- Maximum Outdoor Dry-Bulb Temperature for Supplemental Heater Operation {C}",
        "    0,                       !- Auxiliary On-Cycle Electric Power {W}",
        "    0,                       !- Auxiliary Off-Cycle Electric Power {W}",
        "    0,                       !- Design Heat Recovery Water Flow Rate {m3/s}",
        "    80,                      !- Maximum Temperature for Heat Recovery {C}",
        "    ,                        !- Heat Recovery Water Inlet Node Name",
        "    ,                        !- Heat Recovery Water Outlet Node Name",
        "    3.209,                   !- No Load Supply Air Flow Rate {m3/s}",
        "    2,                       !- Number of Speeds for Heating",
        "    2,                       !- Number of Speeds for Cooling",
        "    3.209,                   !- Heating Speed 1 Supply Air Flow Rate {m3/s}",
        "    3.209,                   !- Heating Speed 2 Supply Air Flow Rate {m3/s}",
        "    ,                        !- Heating Speed 3 Supply Air Flow Rate {m3/s}",
        "    ,                        !- Heating Speed 4 Supply Air Flow Rate {m3/s}",
        "    3.209,                   !- Cooling Speed 1 Supply Air Flow Rate {m3/s}",
        "    3.209;                   !- Cooling Speed 2 Supply Air Flow Rate {m3/s}",

        "  Timestep,6;",

        "  Zone,",
        "    401,                     !- Name",
        "    48.33,                   !- Direction of Relative North {deg}",
        "    15.002524,               !- X Origin {m}",
        "    -50.24491,               !- Y Origin {m}",
        "    0.0,                     !- Z Origin {m}",
        "    ,                        !- Type",
        "    1;                       !- Multiplier",

        "  Schedule:Compact,",
        "    ActSchd,                 !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,118;        !- Field 3",

        "  Schedule:Compact,",
        "    Space temp SP,           !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,22.00;      !- Field 3",

        "  Schedule:Compact,",
        "    Zone Control Type Sched, !- Name",
        "    Control Type,            !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,4.00;       !- Field 3",

        "  Schedule:Compact,",
        "    Fan_schd,                !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,1;          !- Field 17",

        "  Schedule:Compact,",
        "    HeatP fan cyc_sched,     !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,0;          !- Field 10",

        "  Schedule:Compact,",
        "    Econ sched,              !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,1.00;       !- Field 3",

        "  Schedule:Compact,",
        "    HeatingPlant ON,         !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,1.00;       !- Field 3",

        "  Schedule:Compact,",
        "    INFIL_QUARTER_ON_SCH,    !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,1.00;       !- Field 3",

        "  Schedule:Compact,",
        "    401HeatingSP,            !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,21.11;      !- Field 7",

        "  Schedule:Compact,",
        "    401CoolingSP,            !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,22.2;       !- Field 7",

        "  Schedule:Compact,",
        "    AC-24sched,              !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "	 For: Alldays,			  !- Field 2",
        "    Until: 24:00,1;          !- Field 3",

        "!-   ===========  ALL OBJECTS IN CLASS: CONTROLLER:OUTDOORAIR ===========",

        "  Controller:OutdoorAir,",
        "    AC-24 OA Controller,     !- Name",
        "    AC-24 EA node,           !- Relief Air Outlet Node Name",
        "    AC-24 airloop inlet node,!- Return Air Node Name",
        "    AC-24 SF inlet air node, !- Mixed Air Node Name",
        "    AC-24 OA inlet node,     !- Actuator Node Name",
        "    0.481,                   !- Minimum Outdoor Air Flow Rate {m3/s}",
        "    0.481,                   !- Maximum Outdoor Air Flow Rate {m3/s}",
        "    NoEconomizer,            !- Economizer Control Type",
        "    ,                        !- Economizer Control Action Type",
        "    ,                        !- Economizer Maximum Limit Dry-Bulb Temperature {C}",
        "    ,                        !- Economizer Maximum Limit Enthalpy {J/kg}",
        "    ,                        !- Economizer Maximum Limit Dewpoint Temperature {C}",
        "    ,                        !- Electronic Enthalpy Limit Curve Name",
        "    ,                        !- Economizer Minimum Limit Dry-Bulb Temperature {C}",
        "    ,                        !- Lockout Type",
        "    ;                        !- Minimum Limit Type",

        "!-   ===========  ALL OBJECTS IN CLASS: AIRLOOPHVAC:CONTROLLERLIST ===========",

        "  AirLoopHVAC:ControllerList,",
        "    Z401OA controller list,  !- Name",
        "    Controller:OutdoorAir,   !- Controller 1 Object Type",
        "    AC-24 OA Controller;     !- Controller 1 Name",

        "!-   ===========  ALL OBJECTS IN CLASS: AIRLOOPHVAC ===========",

        "  AirLoopHVAC,",
        "    Z401 airloop,            !- Name",
        "    ,                        !- Controller List Name",
        "    AC24,                    !- Availability Manager List Name",
        "    3.209,                   !- Design Supply Air Flow Rate {m3/s}",
        "    Z401 branch list,        !- Branch List Name",
        "    ,                        !- Connector List Name",
        "    AC-24 airloop inlet node,!- Supply Side Inlet Node Name",
        "    Z401 RA node,            !- Demand Side Outlet Node Name",
        "    Z401 splitter inlet,     !- Demand Side Inlet Node Names",
        "    AC-24 airloop outlet node;  !- Supply Side Outlet Node Names",

        "!-   ===========  ALL OBJECTS IN CLASS: AIRLOOPHVAC:OUTDOORAIRSYSTEM:EQUIPMENTLIST ===========",

        "  AirLoopHVAC:OutdoorAirSystem:EquipmentList,",
        "    Z401OA equip list,       !- Name",
        "    OutdoorAir:Mixer,        !- Component 1 Object Type",
        "    AC-24 OA intake;         !- Component 1 Name",

        "!-   ===========  ALL OBJECTS IN CLASS: AIRLOOPHVAC:OUTDOORAIRSYSTEM ===========",

        "  AirLoopHVAC:OutdoorAirSystem,",
        "    Z401 OA Sys,             !- Name",
        "    Z401OA controller list,  !- Controller List Name",
        "    Z401OA equip list;       !- Outdoor Air Equipment List Name",

        "!-   ===========  ALL OBJECTS IN CLASS: OUTDOORAIR:MIXER ===========",

        "  OutdoorAir:Mixer,",
        "    AC-24 OA intake,         !- Name",
        "    AC-24 SF inlet air node, !- Mixed Air Node Name",
        "    AC-24 OA inlet node,     !- Outdoor Air Stream Node Name",
        "    AC-24 EA node,           !- Relief Air Stream Node Name",
        "    AC-24 airloop inlet node;!- Return Air Stream Node Name",

        "!-   ===========  ALL OBJECTS IN CLASS: AIRLOOPHVAC:ZONESPLITTER ===========",

        "  AirLoopHVAC:ZoneSplitter,",
        "    Z401 SA splitter,        !- Name",
        "    Z401 splitter inlet,     !- Inlet Node Name",
        "    Z401 zone inlet 2AT;     !- Outlet 1 Node Name",

        "!-   ===========  ALL OBJECTS IN CLASS: AIRLOOPHVAC:SUPPLYPATH ===========",

        "  AirLoopHVAC:SupplyPath,",
        "    Z401SupplyPath,          !- Name",
        "    Z401 splitter inlet,     !- Supply Air Path Inlet Node Name",
        "    AirLoopHVAC:ZoneSplitter,!- Component 1 Object Type",
        "    Z401 SA splitter;        !- Component 1 Name",

        "!-   ===========  ALL OBJECTS IN CLASS: AIRLOOPHVAC:ZONEMIXER ===========",

        "  AirLoopHVAC:ZoneMixer,",
        "    Z401 RA mixer,           !- Name",
        "    Z401 RA node,            !- Outlet Node Name",
        "    Z401 outlet node;        !- Inlet 1 Node Name",

        "!-   ===========  ALL OBJECTS IN CLASS: AIRLOOPHVAC:RETURNPATH ===========",

        "  AirLoopHVAC:ReturnPath,",
        "    Z401RApath,              !- Name",
        "    Z401 RA node,            !- Return Air Path Outlet Node Name",
        "    AirLoopHVAC:ZoneMixer,   !- Component 1 Object Type",
        "    Z401 RA mixer;           !- Component 1 Name",

        "!-   ===========  ALL OBJECTS IN CLASS: BRANCH ===========",

        "  Branch,",
        "    Z401 main branch,        !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    AirLoopHVAC:OutdoorAirSystem,  !- Component 1 Object Type",
        "    Z401 OA Sys,             !- Component 1 Name",
        "    AC-24 airloop inlet node,!- Component 1 Inlet Node Name",
        "    AC-24 SF inlet air node, !- Component 1 Outlet Node Name",
        "    AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed,  !- Component 2 Object Type",
        "    AC-24 heat pump,         !- Component 2 Name",
        "    AC-24 SF inlet air node, !- Component 2 Inlet Node Name",
        "    AC-24 airloop outlet node;  !- Component 2 Outlet Node Name",

        "!-   ===========  ALL OBJECTS IN CLASS: BRANCHLIST ===========",

        "  BranchList,",
        "    Z401 branch list,        !- Name",
        "    Z401 main branch;        !- Branch 1 Name",

        "!-   ===========  ALL OBJECTS IN CLASS: NODELIST ===========",

        "  NodeList,",
        "    AC-24 OA intake list,    !- Name",
        "    AC-24 OA inlet node;     !- Node 1 Name",

        "!-   ===========  ALL OBJECTS IN CLASS: OUTDOORAIR:NODELIST ===========",

        "  OutdoorAir:NodeList,",
        "    AC-24 OA Intake list;    !- Node or NodeList Name 1",

        "!-   ===========  ALL OBJECTS IN CLASS: AVAILABILITYMANAGER:SCHEDULED ===========",

        "  AvailabilityManager:Scheduled,",
        "    AC24 OnOff,              !- Name",
        "    AC-24sched;              !- Schedule Name",

        "!-   ===========  ALL OBJECTS IN CLASS: AVAILABILITYMANAGERASSIGNMENTLIST ===========",

        "  AvailabilityManagerAssignmentList,",
        "    AC24,                    !- Name",
        "    AvailabilityManager:Scheduled,  !- Availability Manager 1 Object Type",
        "    AC24 OnOff;              !- Availability Manager 1 Name",

        "!-   ===========  ALL OBJECTS IN CLASS: CURVE:QUADRATIC ===========",

        "  Curve:Quadratic,",
        "    CoolingFlowCurve,        !- Name",
        "    1,                       !- Coefficient1 Constant",
        "    0,                       !- Coefficient2 x",
        "    0,                       !- Coefficient3 x**2",
        "    0.1,                     !- Minimum Value of x",
        "    2;                       !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    EIRFlowCurve,            !- Name",
        "    1,                       !- Coefficient1 Constant",
        "    0,                       !- Coefficient2 x",
        "    0,                       !- Coefficient3 x**2",
        "    0.1,                     !- Minimum Value of x",
        "    2;                       !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    PLFCurve,                !- Name",
        "    0.75,                    !- Coefficient1 Constant",
        "    0.25,                    !- Coefficient2 x",
        "    0,                       !- Coefficient3 x**2",
        "    0.1,                     !- Minimum Value of x",
        "    1;                       !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HeatingFlowCurve,        !- Name",
        "    1,                       !- Coefficient1 Constant",
        "    0,                       !- Coefficient2 x",
        "    0,                       !- Coefficient3 x**2",
        "    0.1,                     !- Minimum Value of x",
        "    2;                       !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    BLCoolingFlowCurve,      !- Name",
        "    0.86187,                 !- Coefficient1 Constant",
        "    0.14853,                 !- Coefficient2 x",
        "    -.009899,                !- Coefficient3 x**2",
        "    0.1,                     !- Minimum Value of x",
        "    2;                       !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    BLEIRFlowCurve,          !- Name",
        "    1.1085,                  !- Coefficient1 Constant",
        "    -.1266,                  !- Coefficient2 x",
        "    0.017998,                !- Coefficient3 x**2",
        "    0.1,                     !- Minimum Value of x",
        "    2;                       !- Maximum Value of x",

        "!-   ===========  ALL OBJECTS IN CLASS: CURVE:BIQUADRATIC ===========",

        "  Curve:Biquadratic,",
        "    CoolingTempCurve,        !- Name",
        "    0.66896,                 !- Coefficient1 Constant",
        "    0.023936,                !- Coefficient2 x",
        "    -.00015091,              !- Coefficient3 x**2",
        "    -.00128818,              !- Coefficient4 y",
        "    -.000168897,             !- Coefficient5 y**2",
        "    0.000258167,             !- Coefficient6 x*y",
        "    0,                       !- Minimum Value of x",
        "    20,                      !- Maximum Value of x",
        "    0,                       !- Minimum Value of y",
        "    40;                      !- Maximum Value of y",

        "  Curve:Biquadratic,",
        "    EIRTempCurve,            !- Name",
        "    0.574095,                !- Coefficient1 Constant",
        "    -.003463,                !- Coefficient2 x",
        "    0.00029161,              !- Coefficient3 x**2",
        "    0.012714,                !- Coefficient4 y",
        "    0.00034595,              !- Coefficient5 y**2",
        "    -.000714,                !- Coefficient6 x*y",
        "    0,                       !- Minimum Value of x",
        "    20,                      !- Maximum Value of x",
        "    0,                       !- Minimum Value of y",
        "    40;                      !- Maximum Value of y",

        "  Curve:Biquadratic,",
        "    WasteHeatFrac,           !- Name",
        "    0,                       !- Coefficient1 Constant",
        "    0,                       !- Coefficient2 x",
        "    0,                       !- Coefficient3 x**2",
        "    0,                       !- Coefficient4 y",
        "    0,                       !- Coefficient5 y**2",
        "    0,                       !- Coefficient6 x*y",
        "    0,                       !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    0,                       !- Minimum Value of y",
        "    50;                      !- Maximum Value of y",

        "  Curve:Biquadratic,",
        "    DefrostTempCurve,        !- Name",
        "    0.43264,                 !- Coefficient1 Constant",
        "    0.0013974,               !- Coefficient2 x",
        "    0.00026958,              !- Coefficient3 x**2",
        "    -.02639,                 !- Coefficient4 y",
        "    -.000142377,             !- Coefficient5 y**2",
        "    -.0016066,               !- Coefficient6 x*y",
        "    -10,                     !- Minimum Value of x",
        "    20,                      !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    50;                      !- Maximum Value of y",

        "  Curve:Biquadratic,",
        "    HeatingTempCurve,        !- Name",
        "    0.830527,                !- Coefficient1 Constant",
        "    -.00086702,              !- Coefficient2 x",
        "    -.00016316,              !- Coefficient3 x**2",
        "    0.022539,                !- Coefficient4 y",
        "    0.000142796,             !- Coefficient5 y**2",
        "    0.000046064,             !- Coefficient6 x*y",
        "    -10,                     !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    20;                      !- Maximum Value of y",

        "  Curve:Biquadratic,",
        "    HeatingEIRTempCurve,     !- Name",
        "    0.9111,                  !- Coefficient1 Constant",
        "    -.0106,                  !- Coefficient2 x",
        "    0.000879,                !- Coefficient3 x**2",
        "    0.0051,                  !- Coefficient4 y",
        "    0.00119,                 !- Coefficient5 y**2",
        "    -.00147,                 !- Coefficient6 x*y",
        "    -10,                     !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    20;                      !- Maximum Value of y",

        "  Curve:Biquadratic,",
        "    WasteHeatTempCurve,      !- Name",
        "    1,                       !- Coefficient1 Constant",
        "    0,                       !- Coefficient2 x",
        "    0,                       !- Coefficient3 x**2",
        "    0,                       !- Coefficient4 y",
        "    0,                       !- Coefficient5 y**2",
        "    0,                       !- Coefficient6 x*y",
        "    -10,                     !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    50;                      !- Maximum Value of y",

        "  Curve:Biquadratic,",
        "    BLCoolingTempCurve,      !- Name",
        "    0.54353,                 !- Coefficient1 Constant",
        "    0.020175,                !- Coefficient2 x",
        "    0.00034548,              !- Coefficient3 x**2",
        "    0.00085622,              !- Coefficient4 y",
        "    -.000085034,             !- Coefficient5 y**2",
        "    -.00007994,              !- Coefficient6 x*y",
        "    -10,                     !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    50;                      !- Maximum Value of y",

        "  Curve:Biquadratic,",
        "    BLEIRTempCurve,          !- Name",
        "    1.01728,                 !- Coefficient1 Constant",
        "    -.021724,                !- Coefficient2 x",
        "    0.00025326,              !- Coefficient3 x**2",
        "    0.017851,                !- Coefficient4 y",
        "    0.00014881,              !- Coefficient5 y**2",
        "    -.00043614,              !- Coefficient6 x*y",
        "    -10,                     !- Minimum Value of x",
        "    50,                      !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    50;                      !- Maximum Value of y",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    HeatBalanceManager::GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);                            // zones are specified in the idf snippet

    // Get Zone Equipment Configuration data
    DataZoneEquipment::GetZoneEquipmentData(*state);
    state->dataZoneEquip->ZoneEquipList(1).EquipIndex(1) = 1;
    MixedAir::GetOutsideAirSysInputs(*state);
    MixedAir::GetOAControllerInputs(*state);
    SplitterComponent::GetSplitterInput(*state);
    BranchInputManager::GetMixerInput(*state);
    BranchInputManager::ManageBranchInput(*state);
    GetZoneAirLoopEquipment(*state);
    SingleDuct::GetSysInput(*state);

    // Get Air Loop HVAC Data
    SimAirServingZones::GetAirPathData(*state);
    SimAirServingZones::InitAirLoops(*state, FirstHVACIteration);
    ZoneTempPredictorCorrector::GetZoneAirSetPoints(*state);

    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    auto &zoneSysEnergyDemand = state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1);
    zoneSysEnergyDemand.RemainingOutputRequired = 50000;
    zoneSysEnergyDemand.OutputRequiredToHeatingSP = 50000;
    zoneSysEnergyDemand.OutputRequiredToCoolingSP = 55000.0;
    zoneSysEnergyDemand.SequencedOutputRequired.allocate(1);
    zoneSysEnergyDemand.SequencedOutputRequiredToCoolingSP.allocate(1);
    zoneSysEnergyDemand.SequencedOutputRequiredToHeatingSP.allocate(1);
    zoneSysEnergyDemand.SequencedOutputRequired(1) = zoneSysEnergyDemand.RemainingOutputRequired;
    zoneSysEnergyDemand.SequencedOutputRequiredToCoolingSP(1) = zoneSysEnergyDemand.OutputRequiredToCoolingSP;
    zoneSysEnergyDemand.SequencedOutputRequiredToHeatingSP(1) = zoneSysEnergyDemand.OutputRequiredToHeatingSP;

    HVACMultiSpeedHeatPump::GetMSHeatPumpInput(*state);
    state->dataGlobal->SysSizingCalc = true; // disable sizing calculation
    state->dataHVACMultiSpdHP->FlowFracFlagReady = false;
    auto &msHeatPump = state->dataHVACMultiSpdHP->MSHeatPump(1);
    msHeatPump.HeatCoolMode = EnergyPlus::HVACMultiSpeedHeatPump::ModeOfOperation::HeatingMode;
    msHeatPump.TotHeatEnergyRate = 5000.0;
    msHeatPump.TotCoolEnergyRate = 5000.0;
    msHeatPump.MyFlowFracFlag = false;
    msHeatPump.FlowFraction = 1.0;
    msHeatPump.MyEnvrnFlag = true;
    // verify min OAT from coil inputs
    EXPECT_EQ(msHeatPump.MinOATCompressorCooling, -25.0);
    EXPECT_EQ(msHeatPump.MinOATCompressorHeating, -8.0);
    // set local variables for convenience
    auto *supplyFan = state->dataFans->fans(1);
    auto &dxClgCoilMain = state->dataDXCoils->DXCoil(1);
    auto &dxHtgCoilMain = state->dataDXCoils->DXCoil(2);
    auto &elecHtgCoilSupp = state->dataHeatingCoils->HeatingCoil(msHeatPump.SuppHeatCoilNum);
    Sched::GetSchedule(*state, "AC-24SCHED")->currentVal = 1.0;
    state->dataEnvrn->StdRhoAir = 1.2;
    supplyFan->rhoAirStdInit = state->dataEnvrn->StdRhoAir;
    state->dataGlobal->DoCoilDirectSolutions = false;
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataGlobal->DoCoilDirectSolutions = false;
    // set outdoor air conditions
    state->dataEnvrn->OutDryBulbTemp = 5.0;
    state->dataEnvrn->OutHumRat = 0.005;
    state->dataEnvrn->StdBaroPress = 101325.0;
    state->dataEnvrn->OutBaroPress = 101325.0;

    for (int Mode = 1; Mode <= dxClgCoilMain.NumOfSpeeds; ++Mode) {
        dxClgCoilMain.MSRatedAirMassFlowRate(Mode) = dxClgCoilMain.MSRatedAirVolFlowRate(Mode) * state->dataEnvrn->StdRhoAir;
    }
    for (int Mode = 1; Mode <= dxHtgCoilMain.NumOfSpeeds; ++Mode) {
        dxHtgCoilMain.MSRatedAirMassFlowRate(Mode) = dxHtgCoilMain.MSRatedAirVolFlowRate(Mode) * state->dataEnvrn->StdRhoAir;
    }

    // set zone air conditions
    auto &zoneAirNode =
        state->dataLoopNodes->Node(Util::FindItemInList("Z401 AIR NODE", state->dataLoopNodes->NodeID, state->dataLoopNodes->NumOfNodes));
    zoneAirNode.Temp = 21.1;
    zoneAirNode.HumRat = 0.0035;
    zoneAirNode.Enthalpy = Psychrometrics::PsyHFnTdbW(zoneAirNode.Temp, zoneAirNode.HumRat);
    // set mixed air node conditions
    auto &mixedAirNode =
        state->dataLoopNodes->Node(Util::FindItemInList("AC-24 SF INLET AIR NODE", state->dataLoopNodes->NodeID, state->dataLoopNodes->NumOfNodes));
    mixedAirNode.Temp = 10.0;
    mixedAirNode.HumRat = 0.005;
    mixedAirNode.Enthalpy = Psychrometrics::PsyHFnTdbW(mixedAirNode.Temp, mixedAirNode.HumRat);
    // init ms heat pump
    HVACMultiSpeedHeatPump::InitMSHeatPump(*state, MSHeatPumpNum, FirstHVACIteration, AirLoopNum, QZnReq, OnOffAirFlowRatio);
    // run the unit at zero load
    Real64 QSensUnitOut = 0.0;
    SimMSHP(*state, MSHeatPumpNum, FirstHVACIteration, AirLoopNum, QSensUnitOut, 0.0, OnOffAirFlowRatio);
    // set zone heating load
    QZnReq = zoneSysEnergyDemand.RemainingOutputRequired;
    msHeatPump.HeatCoolMode = EnergyPlus::HVACMultiSpeedHeatPump::ModeOfOperation::HeatingMode;
    SimMSHP(*state, MSHeatPumpNum, FirstHVACIteration, AirLoopNum, QSensUnitOut, QZnReq, OnOffAirFlowRatio);
    // calculate the total electricity rate
    Real64 result_msHeatPump_ElectricityRate = supplyFan->totalPower + state->dataHVACGlobal->DXElecCoolingPower +
                                               state->dataHVACGlobal->DXElecHeatingPower + state->dataHVACGlobal->ElecHeatingCoilPower +
                                               state->dataHVACGlobal->SuppHeatingCoilPower + msHeatPump.AuxElecPower;
    // test results
    EXPECT_NEAR(55366.46, msHeatPump.ElecPower, 0.01);
    EXPECT_NEAR(55366.46, result_msHeatPump_ElectricityRate, 0.01);
    EXPECT_NEAR(3197.31, supplyFan->totalPower, 0.01);
    EXPECT_NEAR(0.0, dxClgCoilMain.ElecCoolingPower, 0.01);
    EXPECT_NEAR(0.0, state->dataHVACGlobal->DXElecCoolingPower, 0.01);
    EXPECT_NEAR(16846.22, dxHtgCoilMain.ElecHeatingPower, 0.01);
    EXPECT_NEAR(16846.22, state->dataHVACGlobal->DXElecHeatingPower, 0.01);
    EXPECT_NEAR(0.0, state->dataHVACGlobal->ElecHeatingCoilPower, 0.01);
    EXPECT_NEAR(35322.93, elecHtgCoilSupp.ElecUseRate, 0.01);
    EXPECT_NEAR(35322.93, state->dataHVACGlobal->SuppHeatingCoilPower, 0.01);
    EXPECT_NEAR(0.0, msHeatPump.AuxElecPower, 0.01);
}

} // namespace EnergyPlus
