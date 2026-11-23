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

// EnergyPlus::AirTerminal:SingleDuct:Mixer Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// ObjexxFCL Headers

#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/BranchInputManager.hh>
#include <EnergyPlus/DXCoils.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/FanCoilUnits.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HVACVariableRefrigerantFlow.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/SingleDuct.hh>
#include <EnergyPlus/SizingManager.hh>
#include <EnergyPlus/SystemAvailabilityManager.hh>
#include <EnergyPlus/UnitVentilator.hh>
#include <EnergyPlus/UnitarySystem.hh>
#include <EnergyPlus/WaterCoils.hh>
#include <EnergyPlus/ZoneAirLoopEquipmentManager.hh>
#include <EnergyPlus/ZoneEquipmentManager.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

// EnergyPlus Headers
using namespace EnergyPlus::BranchInputManager;
using namespace EnergyPlus::DataDefineEquip;
using namespace EnergyPlus::DataEnvironment;
using namespace EnergyPlus::DataHeatBalFanSys;
using namespace EnergyPlus::DataPlant;
using namespace EnergyPlus::DataSizing;
using namespace EnergyPlus::DataZoneEquipment;
using namespace EnergyPlus::DXCoils;
using namespace EnergyPlus::Fans;
using namespace EnergyPlus::FanCoilUnits;
using namespace EnergyPlus::General;
using namespace EnergyPlus::HeatBalanceManager;
using namespace EnergyPlus::HVACVariableRefrigerantFlow;
using namespace EnergyPlus::OutputReportPredefined;
using namespace EnergyPlus::Psychrometrics;
using namespace EnergyPlus::SingleDuct;
using namespace EnergyPlus::UnitVentilator;
using namespace EnergyPlus::WaterCoils;
using namespace EnergyPlus::ZoneAirLoopEquipmentManager;
using namespace EnergyPlus::ZoneTempPredictorCorrector;
using namespace EnergyPlus::DataZoneEnergyDemands;

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, AirTerminalSingleDuctMixer_GetInputPTAC_InletSide)
{

    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({
        "AirTerminal:SingleDuct:Mixer,",
        "    SPACE1-1 DOAS Air Terminal,  !- Name",
        "    ZoneHVAC:PackagedTerminalAirConditioner,     !- ZoneHVAC Terminal Unit Object Type",
        "    SPACE1-1 PTAC,      !- ZoneHVAC Terminal Unit Name",
        "    SPACE1-1 Heat Pump Inlet,!- Terminal Unit Outlet Node Name",
        "    SPACE1-1 Air Terminal Mixer Primary Inlet,   !- Terminal Unit Primary Air Inlet Node Name",
        "    SPACE1-1 Air Terminal Mixer Secondary Inlet, !- Terminal Unit Secondary Air Inlet Node Name",
        "    InletSide;                                   !- Terminal Unit Connection Type",

        "ZoneHVAC:AirDistributionUnit,",
        "    SPACE1-1 DOAS ATU,       !- Name",
        "    SPACE1-1 Heat Pump Inlet,!- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:Mixer,  !- Air Terminal Object Type",
        "    SPACE1-1 DOAS Air Terminal;  !- Air Terminal Name",

        "Schedule:Compact,",
        "    FanAvailSched,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays CustomDay1 CustomDay2,  !- Field 2",
        "    Until: 7:00,             !- Field 3",
        "    0.0,                     !- Field 4",
        "    Until: 21:00,            !- Field 5",
        "    1.0,                     !- Field 6",
        "    Until: 24:00,            !- Field 7",
        "    0.0,                     !- Field 8",
        "    For: Weekends Holiday,   !- Field 9",
        "    Until: 24:00,            !- Field 10",
        "    0.0,                     !- Field 11",
        "    For: SummerDesignDay,    !- Field 12",
        "    Until: 24:00,            !- Field 13",
        "    1.0,                     !- Field 14",
        "    For: WinterDesignDay,    !- Field 15",
        "    Until: 24:00,            !- Field 16",
        "    1.0;                     !- Field 17",

        "Schedule:Compact,",
        "    CyclingFanSch,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    0.0;                     !- Field 4",

        "ZoneHVAC:EquipmentList,",
        "    SPACE1-1 Equipment,      !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE1-1 DOAS ATU,       !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction",
        "    ,                        !- Zone Equipment 1 Sequential Heating Fraction",
        "    ZoneHVAC:PackagedTerminalAirConditioner,  !- Zone Equipment 2 Object Type",
        "    SPACE1-1 PTAC,           !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    2,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 2 Sequential Cooling Fraction",
        "    ;                        !- Zone Equipment 2 Sequential Heating Fraction",

        "  ZoneHVAC:PackagedTerminalAirConditioner,",
        "    SPACE1-1 PTAC,      !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    SPACE1-1 Heat Pump Inlet,!- Air Inlet Node Name",
        "    SPACE1-1 Supply Inlet,   !- Air Outlet Node Name",
        "    ,                        !- Outdoor Air Mixer Object Type",
        "    ,                        !- Outdoor Air Mixer Name",
        "    0.300,                   !- Supply Air Flow Rate During Cooling Operation {m3/s}",
        "    0.300,                   !- Supply Air Flow Rate During Heating Operation {m3/s}",
        "    ,                        !- Supply Air Flow Rate When No Cooling or Heating is Needed {m3/s}",
        "    ,                        !- No Load Supply Air Flow Rate Control Set To Low Speed",
        "    0,                       !- Outdoor Air Flow Rate During Cooling Operation {m3/s}",
        "    0,                       !- Outdoor Air Flow Rate During Heating Operation {m3/s}",
        "    0,                       !- Outdoor Air Flow Rate When No Cooling or Heating is Needed {m3/s}",
        "    Fan:OnOff,               !- Supply Air Fan Object Type",
        "    SPACE1-1 Supply Fan,     !- Supply Air Fan Name",
        "    Coil:Heating:Fuel,        !- Heating Coil Object Type",
        "    SPACE1-1 Heating Coil,   !- Heating Coil Name",
        "    Coil:Cooling:DX:SingleSpeed,  !- Cooling Coil Object Type",
        "    SPACE1-1 PTAC CCoil,     !- Cooling Coil Name",
        "    BlowThrough,             !- Fan Placement",
        "    CyclingFanSch;           !- Supply Air Fan Operating Mode Schedule Name",

        "Fan:OnOff,",
        "    SPACE1-1 Supply Fan,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    75,                      !- Pressure Rise {Pa}",
        "    0.300,                   !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1,                       !- Motor In Airstream Fraction",
        "    SPACE1-1 Heat Pump Inlet,!- Air Inlet Node Name",
        "    SPACE1-1 Zone Unit Fan Outlet;  !- Air Outlet Node Name",

        "Coil:Heating:Fuel,",
        "    SPACE1-1 Heating Coil,   !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    NaturalGas,              !- Fuel Type",
        "    0.8,                     !- Gas Burner Efficiency",
        "    10000.0,                 !- Nominal Capacity {W}",
        "    SPACE1-1 Cooling Coil Outlet,  !- Air Inlet Node Name",
        "    SPACE1-1 Supply Inlet;   !- Air Outlet Node Name",

        "  Coil:Cooling:DX:SingleSpeed,",
        "    SPACE1-1 PTAC CCoil,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    6500.0,                  !- Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Gross Rated Sensible Heat Ratio",
        "    3.0,                     !- Gross Rated Cooling COP {W/W}",
        "    0.300,                   !- Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    SPACE1-1 Zone Unit Fan Outlet, !- Air Inlet Node Name",
        "    SPACE1-1 Cooling Coil Outlet,  !- Air Outlet Node Name",
        "    HPACCoolCapFT,           !- Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACPLFFPLR;             !- Part Load Fraction Correlation Curve Name",

        "  Curve:Quadratic,",
        "    HPACCoolCapFFF,          !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACEIRFFF,              !- Name",
        "    1.1552,                  !- Coefficient1 Constant",
        "    -0.1808,                 !- Coefficient2 x",
        "    0.0256,                  !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACPLFFPLR,             !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "  Curve:Cubic,",
        "    FanEffRatioCurve,        !- Name",
        "    0.33856828,              !- Coefficient1 Constant",
        "    1.72644131,              !- Coefficient2 x",
        "    -1.49280132,             !- Coefficient3 x**2",
        "    0.42776208,              !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    0.3,                     !- Minimum Curve Output",
        "    1.0;                     !- Maximum Curve Output",

        "  Curve:Exponent,",
        "    FanPowerRatioCurve,      !- Name",
        "    0.0,                     !- Coefficient1 Constant",
        "    1.0,                     !- Coefficient2 Constant",
        "    3.0,                     !- Coefficient3 Constant",
        "    0.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    0.01,                    !- Minimum Curve Output",
        "    1.5;                     !- Maximum Curve Output",

        "  Curve:Biquadratic,",
        "    HPACCoolCapFT,           !- Name",
        "    0.942587793,             !- Coefficient1 Constant",
        "    0.009543347,             !- Coefficient2 x",
        "    0.000683770,             !- Coefficient3 x**2",
        "    -0.011042676,            !- Coefficient4 y",
        "    0.000005249,             !- Coefficient5 y**2",
        "    -0.000009720,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    HPACEIRFT,               !- Name",
        "    0.342414409,             !- Coefficient1 Constant",
        "    0.034885008,             !- Coefficient2 x",
        "    -0.000623700,            !- Coefficient3 x**2",
        "    0.004977216,             !- Coefficient4 y",
        "    0.000437951,             !- Coefficient5 y**2",
        "    -0.000728028,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Zone,",
        "    SPACE1-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    239.247360229;           !- Volume {m3}",

        "ZoneHVAC:EquipmentConnections,",
        "    SPACE1-1,                !- Zone Name",
        "    SPACE1-1 Equipment,      !- Zone Conditioning Equipment List Name",
        "    SPACE1-1 Inlets,         !- Zone Air Inlet Node or NodeList Name",
        "    SPACE1-1 Air Terminal Mixer Secondary Inlet,  !- Zone Air Exhaust Node or NodeList Name",
        "    SPACE1-1 Zone Air Node,  !- Zone Air Node Name",
        "    SPACE1-1 Return Outlet;  !- Zone Return Air Node Name",

        "NodeList,",
        "    SPACE1-1 Inlets,         !- Name",
        "    SPACE1-1 Supply Inlet;   !- Node 1 Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);
    UnitarySystems::UnitarySys thisSys;
    UnitarySystems::UnitarySys::factory(*state, HVAC::UnitarySysType::Unitary_AnyCoilType, "SPACE1-1 PTAC", true, 0);
    state->dataZoneEquip->ZoneEquipInputsFilled = true;
    thisSys.getUnitarySystemInput(*state, "SPACE1-1 PTAC", true, 0);
    state->dataUnitarySystems->getInputOnceFlag = false;

    ASSERT_EQ(1, state->dataSingleDuct->NumATMixers);
    EXPECT_EQ("SPACE1-1 DOAS AIR TERMINAL", state->dataSingleDuct->SysATMixer(1).Name);                      // single duct air terminal mixer name
    EXPECT_EQ((int)HVAC::MixerType::InletSide, (int)state->dataSingleDuct->SysATMixer(1).type);              // air terminal mixer connection type
    EXPECT_EQ("AIRTERMINAL:SINGLEDUCT:MIXER", state->dataDefineEquipment->AirDistUnit(1).EquipType(1));      // Air distribution unit equipment type
    EXPECT_EQ("ZoneHVAC:PackagedTerminalAirConditioner", state->dataUnitarySystems->unitarySys[0].UnitType); // zoneHVAC equipment type
}

TEST_F(EnergyPlusFixture, AirTerminalSingleDuctMixer_SimPTAC_ATMInletSide)
{

    bool ErrorsFound(false);
    bool FirstHVACIteration(false);
    Real64 HVACInletMassFlowRate(0.0);
    Real64 PrimaryAirMassFlowRate(0.0);
    Real64 SecondaryAirMassFlowRate(0.0);
    Real64 QUnitOut(0.0);
    Real64 QZnReq(0.0);
    int PTUnitNum(1);

    std::string const idf_objects = delimited_string({
        "AirTerminal:SingleDuct:Mixer,",
        "    SPACE1-1 DOAS Air Terminal,  !- Name",
        "    ZoneHVAC:PackagedTerminalAirConditioner,     !- ZoneHVAC Terminal Unit Object Type",
        "    SPACE1-1 PTAC,      !- ZoneHVAC Terminal Unit Name",
        "    SPACE1-1 Heat Pump Inlet,!- Terminal Unit Outlet Node Name",
        "    SPACE1-1 Air Terminal Mixer Primary Inlet,   !- Terminal Unit Primary Air Inlet Node Name",
        "    SPACE1-1 Air Terminal Mixer Secondary Inlet, !- Terminal Unit Secondary Air Inlet Node Name",
        "    InletSide;                                   !- Terminal Unit Connection Type",

        "ZoneHVAC:AirDistributionUnit,",
        "    SPACE1-1 DOAS ATU,       !- Name",
        "    SPACE1-1 Heat Pump Inlet,!- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:Mixer,  !- Air Terminal Object Type",
        "    SPACE1-1 DOAS Air Terminal;  !- Air Terminal Name",

        "Schedule:Compact,",
        "    FanAvailSched,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 16",
        "    1.0;                     !- Field 17",

        "Schedule:Compact,",
        "    CyclingFanSch,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    0.0;                     !- Field 4",

        "ZoneHVAC:EquipmentList,",
        "    SPACE1-1 Equipment,      !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE1-1 DOAS ATU,       !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction",
        "    ,                        !- Zone Equipment 1 Sequential Heating Fraction",
        "    ZoneHVAC:PackagedTerminalAirConditioner,  !- Zone Equipment 2 Object Type",
        "    SPACE1-1 PTAC,           !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    2,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 2 Sequential Cooling Fraction",
        "    ;                        !- Zone Equipment 2 Sequential Heating Fraction",

        "  ZoneHVAC:PackagedTerminalAirConditioner,",
        "    SPACE1-1 PTAC,      !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    SPACE1-1 Heat Pump Inlet,!- Air Inlet Node Name",
        "    SPACE1-1 Supply Inlet,   !- Air Outlet Node Name",
        "    ,                        !- Outdoor Air Mixer Object Type",
        "    ,                        !- Outdoor Air Mixer Name",
        "    0.500,                   !- Supply Air Flow Rate During Cooling Operation {m3/s}",
        "    0.500,                   !- Supply Air Flow Rate During Heating Operation {m3/s}",
        "    ,                        !- Supply Air Flow Rate When No Cooling or Heating is Needed {m3/s}",
        "    ,                        !- No Load Supply Air Flow Rate Control Set To Low Speed",
        "    0,                       !- Outdoor Air Flow Rate During Cooling Operation {m3/s}",
        "    0,                       !- Outdoor Air Flow Rate During Heating Operation {m3/s}",
        "    0,                       !- Outdoor Air Flow Rate When No Cooling or Heating is Needed {m3/s}",
        "    Fan:OnOff,               !- Supply Air Fan Object Type",
        "    SPACE1-1 Supply Fan,     !- Supply Air Fan Name",
        "    Coil:Heating:Fuel,        !- Heating Coil Object Type",
        "    SPACE1-1 Heating Coil,   !- Heating Coil Name",
        "    Coil:Cooling:DX:SingleSpeed,  !- Cooling Coil Object Type",
        "    SPACE1-1 PTAC CCoil,     !- Cooling Coil Name",
        "    BlowThrough,             !- Fan Placement",
        "    CyclingFanSch;           !- Supply Air Fan Operating Mode Schedule Name",

        "Fan:OnOff,",
        "    SPACE1-1 Supply Fan,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    75,                      !- Pressure Rise {Pa}",
        "    0.500,                   !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1,                       !- Motor In Airstream Fraction",
        "    SPACE1-1 Heat Pump Inlet,!- Air Inlet Node Name",
        "    SPACE1-1 Zone Unit Fan Outlet;  !- Air Outlet Node Name",

        "Coil:Heating:Fuel,",
        "    SPACE1-1 Heating Coil,   !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    NaturalGas,              !- Fuel Type",
        "    0.8,                     !- Gas Burner Efficiency",
        "    10000.0,                 !- Nominal Capacity {W}",
        "    SPACE1-1 Cooling Coil Outlet,  !- Air Inlet Node Name",
        "    SPACE1-1 Supply Inlet;   !- Air Outlet Node Name",

        "  Coil:Cooling:DX:SingleSpeed,",
        "    SPACE1-1 PTAC CCoil,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    6680.0,                  !- Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Gross Rated Sensible Heat Ratio",
        "    3.0,                     !- Gross Rated Cooling COP {W/W}",
        "    0.500,                   !- Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    SPACE1-1 Zone Unit Fan Outlet, !- Air Inlet Node Name",
        "    SPACE1-1 Cooling Coil Outlet,  !- Air Outlet Node Name",
        "    HPACCoolCapFT,           !- Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACPLFFPLR;             !- Part Load Fraction Correlation Curve Name",

        "  Curve:Quadratic,",
        "    HPACCoolCapFFF,          !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACEIRFFF,              !- Name",
        "    1.1552,                  !- Coefficient1 Constant",
        "    -0.1808,                 !- Coefficient2 x",
        "    0.0256,                  !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACPLFFPLR,             !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "  Curve:Cubic,",
        "    FanEffRatioCurve,        !- Name",
        "    0.33856828,              !- Coefficient1 Constant",
        "    1.72644131,              !- Coefficient2 x",
        "    -1.49280132,             !- Coefficient3 x**2",
        "    0.42776208,              !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    0.3,                     !- Minimum Curve Output",
        "    1.0;                     !- Maximum Curve Output",

        "  Curve:Exponent,",
        "    FanPowerRatioCurve,      !- Name",
        "    0.0,                     !- Coefficient1 Constant",
        "    1.0,                     !- Coefficient2 Constant",
        "    3.0,                     !- Coefficient3 Constant",
        "    0.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    0.01,                    !- Minimum Curve Output",
        "    1.5;                     !- Maximum Curve Output",

        "  Curve:Biquadratic,",
        "    HPACCoolCapFT,           !- Name",
        "    0.942587793,             !- Coefficient1 Constant",
        "    0.009543347,             !- Coefficient2 x",
        "    0.000683770,             !- Coefficient3 x**2",
        "    -0.011042676,            !- Coefficient4 y",
        "    0.000005249,             !- Coefficient5 y**2",
        "    -0.000009720,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    HPACEIRFT,               !- Name",
        "    0.342414409,             !- Coefficient1 Constant",
        "    0.034885008,             !- Coefficient2 x",
        "    -0.000623700,            !- Coefficient3 x**2",
        "    0.004977216,             !- Coefficient4 y",
        "    0.000437951,             !- Coefficient5 y**2",
        "    -0.000728028,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Zone,",
        "    SPACE1-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    239.247360229;           !- Volume {m3}",

        "ZoneHVAC:EquipmentConnections,",
        "    SPACE1-1,                !- Zone Name",
        "    SPACE1-1 Equipment,      !- Zone Conditioning Equipment List Name",
        "    SPACE1-1 Inlets,         !- Zone Air Inlet Node or NodeList Name",
        "    SPACE1-1 Air Terminal Mixer Secondary Inlet,  !- Zone Air Exhaust Node or NodeList Name",
        "    SPACE1-1 Zone Air Node,  !- Zone Air Node Name",
        "    SPACE1-1 Return Outlet;  !- Zone Return Air Node Name",

        "NodeList,",
        "    SPACE1-1 Inlets,         !- Name",
        "    SPACE1-1 Supply Inlet;   !- Node 1 Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);
    HVACSystemData *mySys;
    mySys = UnitarySystems::UnitarySys::factory(*state, HVAC::UnitarySysType::Unitary_AnyCoilType, "SPACE1-1 PTAC", true, 0);
    UnitarySystems::UnitarySys thisSys;
    state->dataZoneEquip->ZoneEquipInputsFilled = true;
    thisSys = state->dataUnitarySystems->unitarySys[0];
    thisSys.getUnitarySystemInput(*state, "SPACE1-1 PTAC", true, 0);
    state->dataUnitarySystems->getInputOnceFlag = false;

    // get input test for terminal air single duct mixer on inlet side of PTAC
    ASSERT_EQ(1, state->dataSingleDuct->NumATMixers);
    EXPECT_EQ("SPACE1-1 DOAS AIR TERMINAL", state->dataSingleDuct->SysATMixer(1).Name);                      // single duct air terminal mixer name
    EXPECT_EQ((int)HVAC::MixerType::InletSide, (int)state->dataSingleDuct->SysATMixer(1).type);              // air terminal mixer connection type
    EXPECT_EQ("AIRTERMINAL:SINGLEDUCT:MIXER", state->dataDefineEquipment->AirDistUnit(1).EquipType(1));      // Air distribution unit equipment type
    EXPECT_EQ("ZoneHVAC:PackagedTerminalAirConditioner", state->dataUnitarySystems->unitarySys[0].UnitType); // zoneHVAC equipment type

    state->dataGlobal->BeginEnvrnFlag = false;

    // set input variables
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutDryBulbTemp = 35.0;
    state->dataEnvrn->OutHumRat = 0.0098;
    state->dataEnvrn->OutEnthalpy = Psychrometrics::PsyHFnTdbW(state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutHumRat);
    state->dataEnvrn->StdRhoAir = 1.20;
    HVACInletMassFlowRate = 0.50;
    PrimaryAirMassFlowRate = 0.1;

    // set zoneNode air condition
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp = 24.0;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat = 0.0075;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp,
                                   state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat);

    state->dataUnitarySystems->unitarySys[0].MaxCoolAirMassFlow = HVACInletMassFlowRate; // supply air mass flow rate
    state->dataHVACGlobal->TurnFansOff = false;
    state->dataHVACGlobal->TurnFansOn = true;

    state->dataUnitarySystems->unitarySys[0].m_FanOpMode = HVAC::FanOp::Cycling;
    // initialize mass flow rates
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].AirInNode).MassFlowRate = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].m_ATMixerPriNode).MassFlowRate = PrimaryAirMassFlowRate;
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].m_ATMixerPriNode).MassFlowRateMaxAvail = PrimaryAirMassFlowRate;

    // set fan parameters
    state->dataFans->fans(1)->maxAirMassFlowRate = HVACInletMassFlowRate;
    state->dataFans->fans(1)->inletAirMassFlowRate = HVACInletMassFlowRate;
    state->dataFans->fans(1)->rhoAirStdInit = state->dataEnvrn->StdRhoAir;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->inletNodeNum).MassFlowRateMaxAvail = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->outletNodeNum).MassFlowRateMax = HVACInletMassFlowRate;

    // set DX coil rated performance parameters
    state->dataDXCoils->DXCoil(1).RatedCBF(1) = 0.05;
    state->dataDXCoils->DXCoil(1).RatedAirMassFlowRate(1) = HVACInletMassFlowRate;

    // primary air condition set at outdoor air condition
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].m_ATMixerPriNode).Temp = state->dataEnvrn->OutDryBulbTemp;
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].m_ATMixerPriNode).HumRat = state->dataEnvrn->OutHumRat;
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].m_ATMixerPriNode).Enthalpy = state->dataEnvrn->OutEnthalpy;

    // set secondary air (recirculating air) conditions to zone air node
    state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).Temp =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp;
    state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).HumRat =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat;
    state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).Enthalpy =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy;

    state->dataUnitarySystems->unitarySys[0].ControlZoneNum = 1;
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataGlobal->SysSizingCalc = true;

    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand(1).RemainingOutputReqToDehumidSP = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -5000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = -5000.0;
    QZnReq = state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP;

    state->dataUnitarySystems->unitarySys[0].m_sysAvailSched->currentVal = 1.0; // unit is always available
    state->dataUnitarySystems->unitarySys[0].m_fanAvailSched->currentVal = 1.0; // fan is always available

    // set secondary air mass flow rate to zero
    state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).MassFlowRate = 0.0;
    // simulate PTAC zoneHVAC equipment
    bool HeatActive = false;
    bool CoolActive = false;
    Real64 latOut = 0.0;
    mySys->simulate(*state,
                    state->dataUnitarySystems->unitarySys[0].Name,
                    FirstHVACIteration,
                    0,
                    PTUnitNum,
                    HeatActive,
                    CoolActive,
                    0,
                    0,
                    true,
                    QUnitOut,
                    latOut);
    // apply mass conservation to determine secondary air mass flow rate
    SecondaryAirMassFlowRate = state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].AirInNode).MassFlowRate - PrimaryAirMassFlowRate;
    // check the terminal air mixer secondary air mass flow rate
    ASSERT_EQ(SecondaryAirMassFlowRate, state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).MassFlowRate);
    // check the cooling output delivered is within 2.0 Watt of zone cooling load
    ASSERT_NEAR(QZnReq, QUnitOut, 2.0);
}

TEST_F(EnergyPlusFixture, AirTerminalSingleDuctMixer_SimPTAC_ATMSupplySide)
{

    bool ErrorsFound(false);
    bool FirstHVACIteration(false);
    Real64 HVACInletMassFlowRate(0.0);
    Real64 PrimaryAirMassFlowRate(0.0);
    Real64 SecondaryAirMassFlowRate(0.0);
    Real64 ATMixerOutletMassFlowRate(0.0);
    Real64 QUnitOut(0.0);
    Real64 QZnReq(0.0);
    int PTUnitNum(1);

    std::string const idf_objects = delimited_string({

        "AirTerminal:SingleDuct:Mixer,",
        "    SPACE1-1 DOAS Air Terminal,  !- Name",
        "    ZoneHVAC:PackagedTerminalAirConditioner,  !- ZoneHVAC Terminal Unit Object Type",
        "    SPACE1-1 PTAC,           !- ZoneHVAC Terminal Unit Name",
        "    SPACE1-1 Supply Inlet,   !- Terminal Unit Outlet Node Name",
        "    SPACE1-1 Air Terminal Mixer Primary Inlet,  !- Terminal Unit Primary Air Inlet Node Name",
        "    SPACE1-1 PTAC Outlet,    !- Terminal Unit Secondary Air Inlet Node Name",
        "    SupplySide;              !- Terminal Unit Connection Type",

        "ZoneHVAC:AirDistributionUnit,",
        "    SPACE1-1 DOAS ATU,       !- Name",
        "    SPACE1-1 Supply Inlet,   !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:Mixer,  !- Air Terminal Object Type",
        "    SPACE1-1 DOAS Air Terminal;  !- Air Terminal Name",

        "Schedule:Compact,",
        "    FanAvailSched,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 16",
        "    1.0;                     !- Field 17",

        "Schedule:Compact,",
        "    CyclingFanSch,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    0.0;                     !- Field 4",

        "ZoneHVAC:EquipmentList,",
        "    SPACE1-1 Equipment,      !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE1-1 DOAS ATU,       !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction",
        "    ,                        !- Zone Equipment 1 Sequential Heating Fraction",
        "    ZoneHVAC:PackagedTerminalAirConditioner,  !- Zone Equipment 2 Object Type",
        "    SPACE1-1 PTAC,           !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    2,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 2 Sequential Cooling Fraction",
        "    ;                        !- Zone Equipment 2 Sequential Heating Fraction",

        "  ZoneHVAC:PackagedTerminalAirConditioner,",
        "    SPACE1-1 PTAC,           !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    SPACE1-1 PTAC Inlet,     !- Air Inlet Node Name",
        "    SPACE1-1 PTAC Outlet,    !- Air Outlet Node Name",
        "    ,                        !- Outdoor Air Mixer Object Type",
        "    ,                        !- Outdoor Air Mixer Name",
        "    0.500,                   !- Supply Air Flow Rate During Cooling Operation {m3/s}",
        "    0.500,                   !- Supply Air Flow Rate During Heating Operation {m3/s}",
        "    ,                        !- Supply Air Flow Rate When No Cooling or Heating is Needed {m3/s}",
        "    ,                        !- No Load Supply Air Flow Rate Control Set To Low Speed",
        "    0,                       !- Outdoor Air Flow Rate During Cooling Operation {m3/s}",
        "    0,                       !- Outdoor Air Flow Rate During Heating Operation {m3/s}",
        "    0,                       !- Outdoor Air Flow Rate When No Cooling or Heating is Needed {m3/s}",
        "    Fan:OnOff,               !- Supply Air Fan Object Type",
        "    SPACE1-1 Supply Fan,     !- Supply Air Fan Name",
        "    Coil:Heating:Fuel,        !- Heating Coil Object Type",
        "    SPACE1-1 Heating Coil,   !- Heating Coil Name",
        "    Coil:Cooling:DX:SingleSpeed,  !- Cooling Coil Object Type",
        "    SPACE1-1 PTAC CCoil,     !- Cooling Coil Name",
        "    BlowThrough,             !- Fan Placement",
        "    CyclingFanSch;           !- Supply Air Fan Operating Mode Schedule Name",

        "Fan:OnOff,",
        "    SPACE1-1 Supply Fan,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    75,                      !- Pressure Rise {Pa}",
        "    0.500,                   !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1,                       !- Motor In Airstream Fraction",
        "    SPACE1-1 PTAC Inlet,     !- Air Inlet Node Name",
        "    SPACE1-1 Zone Unit Fan Outlet;  !- Air Outlet Node Name",

        "Coil:Heating:Fuel,",
        "    SPACE1-1 Heating Coil,   !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    NaturalGas,              !- Fuel Type",
        "    0.8,                     !- Gas Burner Efficiency",
        "    10000.0,                 !- Nominal Capacity {W}",
        "    SPACE1-1 Cooling Coil Outlet,  !- Air Inlet Node Name",
        "    SPACE1-1 PTAC Outlet;    !- Air Outlet Node Name",

        "  Coil:Cooling:DX:SingleSpeed,",
        "    SPACE1-1 PTAC CCoil,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    7030.0,                  !- Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Gross Rated Sensible Heat Ratio",
        "    3.0,                     !- Gross Rated Cooling COP {W/W}",
        "    0.500,                   !- Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    SPACE1-1 Zone Unit Fan Outlet, !- Air Inlet Node Name",
        "    SPACE1-1 Cooling Coil Outlet,  !- Air Outlet Node Name",
        "    HPACCoolCapFT,           !- Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACPLFFPLR;             !- Part Load Fraction Correlation Curve Name",

        "  Curve:Quadratic,",
        "    HPACCoolCapFFF,          !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACEIRFFF,              !- Name",
        "    1.1552,                  !- Coefficient1 Constant",
        "    -0.1808,                 !- Coefficient2 x",
        "    0.0256,                  !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACPLFFPLR,             !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "  Curve:Cubic,",
        "    FanEffRatioCurve,        !- Name",
        "    0.33856828,              !- Coefficient1 Constant",
        "    1.72644131,              !- Coefficient2 x",
        "    -1.49280132,             !- Coefficient3 x**2",
        "    0.42776208,              !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    0.3,                     !- Minimum Curve Output",
        "    1.0;                     !- Maximum Curve Output",

        "  Curve:Exponent,",
        "    FanPowerRatioCurve,      !- Name",
        "    0.0,                     !- Coefficient1 Constant",
        "    1.0,                     !- Coefficient2 Constant",
        "    3.0,                     !- Coefficient3 Constant",
        "    0.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    0.01,                    !- Minimum Curve Output",
        "    1.5;                     !- Maximum Curve Output",

        "  Curve:Biquadratic,",
        "    HPACCoolCapFT,           !- Name",
        "    0.942587793,             !- Coefficient1 Constant",
        "    0.009543347,             !- Coefficient2 x",
        "    0.000683770,             !- Coefficient3 x**2",
        "    -0.011042676,            !- Coefficient4 y",
        "    0.000005249,             !- Coefficient5 y**2",
        "    -0.000009720,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    HPACEIRFT,               !- Name",
        "    0.342414409,             !- Coefficient1 Constant",
        "    0.034885008,             !- Coefficient2 x",
        "    -0.000623700,            !- Coefficient3 x**2",
        "    0.004977216,             !- Coefficient4 y",
        "    0.000437951,             !- Coefficient5 y**2",
        "    -0.000728028,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Zone,",
        "    SPACE1-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    239.247360229;           !- Volume {m3}",

        "ZoneHVAC:EquipmentConnections,",
        "    SPACE1-1,                !- Zone Name",
        "    SPACE1-1 Equipment,      !- Zone Conditioning Equipment List Name",
        "    SPACE1-1 Inlets,         !- Zone Air Inlet Node or NodeList Name",
        "    SPACE1-1 PTAC Inlet,     !- Zone Air Exhaust Node or NodeList Name",
        "    SPACE1-1 Zone Air Node,  !- Zone Air Node Name",
        "    SPACE1-1 Return Outlet;  !- Zone Return Air Node Name",

        "NodeList,",
        "    SPACE1-1 Inlets,         !- Name",
        "    SPACE1-1 Supply Inlet;   !- Node 1 Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);
    HVACSystemData *mySys;
    mySys = UnitarySystems::UnitarySys::factory(*state, HVAC::UnitarySysType::Unitary_AnyCoilType, "SPACE1-1 PTAC", true, 0);
    UnitarySystems::UnitarySys thisSys;
    state->dataZoneEquip->ZoneEquipInputsFilled = true;
    thisSys = state->dataUnitarySystems->unitarySys[0];
    thisSys.getUnitarySystemInput(*state, "SPACE1-1 PTAC", true, 0);
    state->dataUnitarySystems->getInputOnceFlag = false;

    // get input test for terminal air single duct mixer on supply side of PTAC
    ASSERT_EQ(1, state->dataSingleDuct->NumATMixers);
    EXPECT_EQ("SPACE1-1 DOAS AIR TERMINAL", state->dataSingleDuct->SysATMixer(1).Name);                 // single duct air terminal mixer name
    EXPECT_EQ((int)HVAC::MixerType::SupplySide, (int)state->dataSingleDuct->SysATMixer(1).type);        // air terminal mixer connection type
    EXPECT_EQ("AIRTERMINAL:SINGLEDUCT:MIXER", state->dataDefineEquipment->AirDistUnit(1).EquipType(1)); // Air distribution unit equipment type
    EXPECT_EQ(UnitarySystems::UnitarySys::SysType::PackagedAC, state->dataUnitarySystems->unitarySys[0].m_sysType); // zoneHVAC equipment type

    // set input variables
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutDryBulbTemp = 35.0;
    state->dataEnvrn->OutHumRat = 0.0098;
    state->dataEnvrn->OutEnthalpy = Psychrometrics::PsyHFnTdbW(state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutHumRat);
    state->dataEnvrn->StdRhoAir = 1.20;
    HVACInletMassFlowRate = 0.50;
    PrimaryAirMassFlowRate = 0.1;

    state->dataGlobal->BeginEnvrnFlag = false;

    // set zoneNode air condition
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp = 24.0;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat = 0.0075;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp,
                                   state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat);

    state->dataUnitarySystems->unitarySys[0].MaxCoolAirMassFlow = HVACInletMassFlowRate; // supply air mass flow rate
    state->dataHVACGlobal->TurnFansOff = false;
    state->dataHVACGlobal->TurnFansOn = true;

    PTUnitNum = 0;
    state->dataUnitarySystems->unitarySys[0].m_FanOpMode = HVAC::FanOp::Cycling;
    // initialize mass flow rates
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].AirInNode).MassFlowRate = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].m_ATMixerPriNode).MassFlowRate = PrimaryAirMassFlowRate;
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].m_ATMixerPriNode).MassFlowRateMaxAvail = PrimaryAirMassFlowRate;

    // set fan parameters
    state->dataFans->fans(1)->maxAirMassFlowRate = HVACInletMassFlowRate;
    state->dataFans->fans(1)->inletAirMassFlowRate = HVACInletMassFlowRate;
    state->dataFans->fans(1)->rhoAirStdInit = state->dataEnvrn->StdRhoAir;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->inletNodeNum).MassFlowRateMaxAvail = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->outletNodeNum).MassFlowRateMax = HVACInletMassFlowRate;

    // set DX coil rated performance parameters
    state->dataDXCoils->DXCoil(1).RatedCBF(1) = 0.05;
    state->dataDXCoils->DXCoil(1).RatedAirMassFlowRate(1) = HVACInletMassFlowRate;

    // primary air condition at outside air condition
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].m_ATMixerPriNode).Temp = state->dataEnvrn->OutDryBulbTemp;
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].m_ATMixerPriNode).HumRat = state->dataEnvrn->OutHumRat;
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].m_ATMixerPriNode).Enthalpy = state->dataEnvrn->OutEnthalpy;

    // set PTUnit inlet condition to zone air node
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].AirInNode).Temp =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp;
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].AirInNode).HumRat =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat;
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].AirInNode).Enthalpy =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy;

    state->dataUnitarySystems->unitarySys[0].ControlZoneNum = 1;
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataGlobal->SysSizingCalc = true;

    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand(1).RemainingOutputReqToDehumidSP = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -5000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = -5000.0;
    QZnReq = state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP;

    state->dataUnitarySystems->unitarySys[0].m_sysAvailSched->currentVal = 1.0; // unit is always available
    state->dataUnitarySystems->unitarySys[0].m_fanAvailSched->currentVal = 1.0; // fan is always available

    // set secondary air mass flow rate to zero
    state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).MassFlowRate = 0.0;
    // simulate PTAC zoneHVAC equipment
    bool HeatActive = false;
    bool CoolActive = false;
    Real64 latOut = 0.0;
    mySys->simulate(*state,
                    state->dataUnitarySystems->unitarySys[0].Name,
                    FirstHVACIteration,
                    0,
                    PTUnitNum,
                    HeatActive,
                    CoolActive,
                    0,
                    0,
                    true,
                    QUnitOut,
                    latOut);
    // apply mass conservation to determine secondary mass flow rate
    SecondaryAirMassFlowRate = state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).MassFlowRate;
    // check the terminal air mixer secondary air mass flow rate
    ASSERT_EQ(SecondaryAirMassFlowRate, state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).MassFlowRate);
    // check the terminal air mixer outlet air mass flow rate
    ATMixerOutletMassFlowRate = SecondaryAirMassFlowRate + PrimaryAirMassFlowRate;
    ASSERT_EQ(ATMixerOutletMassFlowRate, state->dataSingleDuct->SysATMixer(1).MixedAirMassFlowRate);
    // check the cooling output delivered is within 2.0 Watt of zone cooling load
    ASSERT_NEAR(QZnReq, QUnitOut, 2.0);
}

TEST_F(EnergyPlusFixture, AirTerminalSingleDuctMixer_SimPTHP_ATMInletSide)
{

    bool ErrorsFound(false);
    bool FirstHVACIteration(false);
    Real64 HVACInletMassFlowRate(0.0);
    Real64 PrimaryAirMassFlowRate(0.0);
    Real64 SecondaryAirMassFlowRate(0.0);
    Real64 QUnitOut(0.0);
    Real64 QZnReq(0.0);
    int PTUnitNum(1);

    std::string const idf_objects = delimited_string({
        "AirTerminal:SingleDuct:Mixer,",
        "    SPACE1-1 DOAS Air Terminal,  !- Name",
        "    ZoneHVAC:PackagedTerminalHeatPump,  !- ZoneHVAC Terminal Unit Object Type",
        "    SPACE1-1 Heat Pump,      !- ZoneHVAC Terminal Unit Name",
        "    SPACE1-1 Heat Pump Inlet,!- Terminal Unit Outlet Node Name",
        "    SPACE1-1 Air Terminal Mixer Primary Inlet,   !- Terminal Unit Primary Air Inlet Node Name",
        "    SPACE1-1 Air Terminal Mixer Secondary Inlet, !- Terminal Unit Secondary Air Inlet Node Name",
        "    InletSide;                                   !- Terminal Unit Connection Type",

        "ZoneHVAC:AirDistributionUnit,",
        "    SPACE1-1 DOAS ATU,       !- Name",
        "    SPACE1-1 Heat Pump Inlet,!- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:Mixer,  !- Air Terminal Object Type",
        "    SPACE1-1 DOAS Air Terminal;  !- Air Terminal Name",

        "Schedule:Compact,",
        "    FanAvailSched,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 16",
        "    1.0;                     !- Field 17",

        "Schedule:Compact,",
        "    CyclingFanSch,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    0.0;                     !- Field 4",

        "ZoneHVAC:EquipmentList,",
        "    SPACE1-1 Equipment,      !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE1-1 DOAS ATU,       !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction",
        "    ,                        !- Zone Equipment 1 Sequential Heating Fraction",
        "    ZoneHVAC:PackagedTerminalHeatPump,  !- Zone Equipment 2 Object Type",
        "    SPACE1-1 Heat Pump,      !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    2,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 2 Sequential Cooling Fraction",
        "    ;                        !- Zone Equipment 2 Sequential Heating Fraction",

        "  ZoneHVAC:PackagedTerminalHeatPump,",
        "    SPACE1-1 Heat Pump,      !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    SPACE1-1 Heat Pump Inlet,!- Air Inlet Node Name",
        "    SPACE1-1 Supply Inlet,   !- Air Outlet Node Name",
        "    ,                        !- Outdoor Air Mixer Object Type",
        "    ,                        !- Outdoor Air Mixer Name",
        "    0.500,                   !- Supply Air Flow Rate During Cooling Operation {m3/s}",
        "    0.500,                   !- Supply Air Flow Rate During Heating Operation {m3/s}",
        "    ,                        !- Supply Air Flow Rate When No Cooling or Heating is Needed {m3/s}",
        "    ,                        !- No Load Supply Air Flow Rate Control Set To Low Speed",
        "    0,                       !- Outdoor Air Flow Rate During Cooling Operation {m3/s}",
        "    0,                       !- Outdoor Air Flow Rate During Heating Operation {m3/s}",
        "    0,                       !- Outdoor Air Flow Rate When No Cooling or Heating is Needed {m3/s}",
        "    Fan:OnOff,               !- Supply Air Fan Object Type",
        "    SPACE1-1 Supply Fan,     !- Supply Air Fan Name",
        "    Coil:Heating:DX:SingleSpeed,  !- Heating Coil Object Type",
        "    SPACE1-1 HP Heating Mode,     !- Heating Coil Name",
        "    0.001,                   !- Heating Convergence Tolerance {dimensionless}",
        "    Coil:Cooling:DX:SingleSpeed,  !- Cooling Coil Object Type",
        "    SPACE1-1 HP Cooling Mode,     !- Cooling Coil Name",
        "    0.001,                   !- Cooling Convergence Tolerance {dimensionless}",
        "    Coil:Heating:Fuel,        !- Supplemental Heating Coil Object Type",
        "    SPACE1-1 HP Supp Coil,   !- Supplemental Heating Coil Name",
        "    50.0,                    !- Maximum Supply Air Temperature from Supplemental Heater {C}",
        "    20.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Supplemental Heater Operation {C}",
        "    BlowThrough,             !- Fan Placement",
        "    CyclingFanSch;           !- Supply Air Fan Operating Mode Schedule Name",

        "Fan:OnOff,",
        "    SPACE1-1 Supply Fan,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    75,                      !- Pressure Rise {Pa}",
        "    0.500,                   !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1,                       !- Motor In Airstream Fraction",
        "    SPACE1-1 Heat Pump Inlet,!- Air Inlet Node Name",
        "    SPACE1-1 Zone Unit Fan Outlet;  !- Air Outlet Node Name",

        "  Coil:Heating:DX:SingleSpeed,",
        "    SPACE1-1 HP Heating Mode,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    7000.0,                  !- Gross Rated Heating Capacity {W}",
        "    3.75,                    !- Gross Rated Heating COP {W/W}",
        "    0.500,                   !- Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Rated Supply Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Supply Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    SPACE1-1 Cooling Coil Outlet,  !- Air Inlet Node Name",
        "    SPACE1-1 Heating Coil Outlet,  !- Air Outlet Node Name",
        "    HPACHeatCapFT,           !- Heating Capacity Function of Temperature Curve Name",
        "    HPACHeatCapFFF,          !- Heating Capacity Function of Flow Fraction Curve Name",
        "    HPACHeatEIRFT,           !- Energy Input Ratio Function of Temperature Curve Name",
        "    HPACHeatEIRFFF,          !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACCOOLPLFFPLR,         !- Part Load Fraction Correlation Curve Name",
        "    ,                        !- Defrost Energy Input Ratio Function of Temperature Curve Name",
        "    2.0,                     !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Outdoor Dry-Bulb Temperature to Turn On Compressor {C}",
        "    5.0,                     !- Maximum Outdoor Dry-Bulb Temperature for Defrost Operation {C}",
        "    200.0,                   !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    Resistive,               !- Defrost Strategy",
        "    TIMED,                   !- Defrost Control",
        "    0.166667,                !- Defrost Time Period Fraction",
        "    Autosize;                !- Resistive Defrost Heater Capacity {W}",

        "  Coil:Cooling:DX:SingleSpeed,",
        "    SPACE1-1 HP Cooling Mode,!- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    6680.0,                  !- Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Gross Rated Sensible Heat Ratio",
        "    3.0,                     !- Gross Rated Cooling COP {W/W}",
        "    0.500,                   !- Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    SPACE1-1 Zone Unit Fan Outlet, !- Air Inlet Node Name",
        "    SPACE1-1 Cooling Coil Outlet,  !- Air Outlet Node Name",
        "    HPACCoolCapFT,           !- Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACPLFFPLR;             !- Part Load Fraction Correlation Curve Name",

        "Coil:Heating:Fuel,",
        "    SPACE1-1 HP Supp Coil,   !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    NaturalGas,              !- Fuel Type",
        "    0.8,                     !- Gas Burner Efficiency",
        "    10000.0,                 !- Nominal Capacity {W}",
        "    SPACE1-1 Heating Coil Outlet,  !- Air Inlet Node Name",
        "    SPACE1-1 Supply Inlet;   !- Air Outlet Node Name",

        "  Curve:Quadratic,",
        "    HPACCoolCapFFF,          !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACEIRFFF,              !- Name",
        "    1.1552,                  !- Coefficient1 Constant",
        "    -0.1808,                 !- Coefficient2 x",
        "    0.0256,                  !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACPLFFPLR,             !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACHeatEIRFFF,          !- Name",
        "    1.3824,                  !- Coefficient1 Constant",
        "    -0.4336,                 !- Coefficient2 x",
        "    0.0512,                  !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACCOOLPLFFPLR,         !- Name",
        "    0.75,                    !- Coefficient1 Constant",
        "    0.25,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "  Curve:Cubic,",
        "    HPACHeatCapFT,           !- Name",
        "    0.758746,                !- Coefficient1 Constant",
        "    0.027626,                !- Coefficient2 x",
        "    0.000148716,             !- Coefficient3 x**2",
        "    0.0000034992,            !- Coefficient4 x**3",
        "    -20.0,                   !- Minimum Value of x",
        "    20.0,                    !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Cubic,",
        "    HPACHeatCapFFF,          !- Name",
        "    0.84,                    !- Coefficient1 Constant",
        "    0.16,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Cubic,",
        "    HPACHeatEIRFT,           !- Name",
        "    1.19248,                 !- Coefficient1 Constant",
        "    -0.0300438,              !- Coefficient2 x",
        "    0.00103745,              !- Coefficient3 x**2",
        "    -0.000023328,            !- Coefficient4 x**3",
        "    -20.0,                   !- Minimum Value of x",
        "    20.0,                    !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Cubic,",
        "    FanEffRatioCurve,        !- Name",
        "    0.33856828,              !- Coefficient1 Constant",
        "    1.72644131,              !- Coefficient2 x",
        "    -1.49280132,             !- Coefficient3 x**2",
        "    0.42776208,              !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    0.3,                     !- Minimum Curve Output",
        "    1.0;                     !- Maximum Curve Output",

        "  Curve:Exponent,",
        "    FanPowerRatioCurve,      !- Name",
        "    0.0,                     !- Coefficient1 Constant",
        "    1.0,                     !- Coefficient2 Constant",
        "    3.0,                     !- Coefficient3 Constant",
        "    0.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    0.01,                    !- Minimum Curve Output",
        "    1.5;                     !- Maximum Curve Output",

        "  Curve:Biquadratic,",
        "    HPACCoolCapFT,           !- Name",
        "    0.942587793,             !- Coefficient1 Constant",
        "    0.009543347,             !- Coefficient2 x",
        "    0.000683770,             !- Coefficient3 x**2",
        "    -0.011042676,            !- Coefficient4 y",
        "    0.000005249,             !- Coefficient5 y**2",
        "    -0.000009720,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    HPACEIRFT,               !- Name",
        "    0.342414409,             !- Coefficient1 Constant",
        "    0.034885008,             !- Coefficient2 x",
        "    -0.000623700,            !- Coefficient3 x**2",
        "    0.004977216,             !- Coefficient4 y",
        "    0.000437951,             !- Coefficient5 y**2",
        "    -0.000728028,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Zone,",
        "    SPACE1-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    239.247360229;           !- Volume {m3}",

        "ZoneHVAC:EquipmentConnections,",
        "    SPACE1-1,                !- Zone Name",
        "    SPACE1-1 Equipment,      !- Zone Conditioning Equipment List Name",
        "    SPACE1-1 Inlets,         !- Zone Air Inlet Node or NodeList Name",
        "    SPACE1-1 Air Terminal Mixer Secondary Inlet,  !- Zone Air Exhaust Node or NodeList Name",
        "    SPACE1-1 Zone Air Node,  !- Zone Air Node Name",
        "    SPACE1-1 Return Outlet;  !- Zone Return Air Node Name",

        "NodeList,",
        "    SPACE1-1 Inlets,         !- Name",
        "    SPACE1-1 Supply Inlet;   !- Node 1 Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);
    HVACSystemData *mySys;
    mySys = UnitarySystems::UnitarySys::factory(*state, HVAC::UnitarySysType::Unitary_AnyCoilType, "SPACE1-1 Heat Pump", true, 0);
    UnitarySystems::UnitarySys thisSys;
    state->dataZoneEquip->ZoneEquipInputsFilled = true;
    thisSys = state->dataUnitarySystems->unitarySys[0];
    thisSys.getUnitarySystemInput(*state, "SPACE1-1 Heat Pump", true, 0);
    state->dataUnitarySystems->getInputOnceFlag = false;

    // get input test for terminal air single duct mixer on inlet side of PTHP
    ASSERT_EQ(1, state->dataSingleDuct->NumATMixers);
    EXPECT_EQ("SPACE1-1 DOAS AIR TERMINAL", state->dataSingleDuct->SysATMixer(1).Name);                 // single duct air terminal mixer name
    EXPECT_EQ((int)HVAC::MixerType::InletSide, (int)state->dataSingleDuct->SysATMixer(1).type);         // air terminal mixer connection type
    EXPECT_EQ("AIRTERMINAL:SINGLEDUCT:MIXER", state->dataDefineEquipment->AirDistUnit(1).EquipType(1)); // Air distribution unit equipment type
    EXPECT_EQ("ZoneHVAC:PackagedTerminalHeatPump", state->dataUnitarySystems->unitarySys[0].UnitType);  // zoneHVAC equipment type

    state->dataGlobal->BeginEnvrnFlag = false;

    // set input variables
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutDryBulbTemp = 35.0;
    state->dataEnvrn->OutHumRat = 0.0098;
    state->dataEnvrn->OutEnthalpy = Psychrometrics::PsyHFnTdbW(state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutHumRat);
    state->dataEnvrn->StdRhoAir = 1.20;
    HVACInletMassFlowRate = 0.50;
    PrimaryAirMassFlowRate = 0.1;

    // set zoneNode air condition
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp = 24.0;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat = 0.0075;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp,
                                   state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat);

    state->dataUnitarySystems->unitarySys[0].MaxCoolAirMassFlow = HVACInletMassFlowRate; // supply air mass flow rate
    state->dataHVACGlobal->TurnFansOff = false;
    state->dataHVACGlobal->TurnFansOn = true;

    PTUnitNum = 0;
    state->dataUnitarySystems->unitarySys[0].m_FanOpMode = HVAC::FanOp::Cycling;
    // initialize mass flow rates
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].AirInNode).MassFlowRate = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].m_ATMixerPriNode).MassFlowRate = PrimaryAirMassFlowRate;
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].m_ATMixerPriNode).MassFlowRateMaxAvail = PrimaryAirMassFlowRate;

    // set fan parameters
    state->dataFans->fans(1)->maxAirMassFlowRate = HVACInletMassFlowRate;
    state->dataFans->fans(1)->inletAirMassFlowRate = HVACInletMassFlowRate;
    state->dataFans->fans(1)->rhoAirStdInit = state->dataEnvrn->StdRhoAir;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->inletNodeNum).MassFlowRateMaxAvail = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->outletNodeNum).MassFlowRateMax = HVACInletMassFlowRate;

    // set DX coil rated performance parameters
    state->dataDXCoils->DXCoil(1).RatedCBF(1) = 0.05;
    state->dataDXCoils->DXCoil(1).RatedAirMassFlowRate(1) = HVACInletMassFlowRate;

    // primary air condition set at outdoor air condition
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].m_ATMixerPriNode).Temp = state->dataEnvrn->OutDryBulbTemp;
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].m_ATMixerPriNode).HumRat = state->dataEnvrn->OutHumRat;
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].m_ATMixerPriNode).Enthalpy = state->dataEnvrn->OutEnthalpy;

    // set secondary air (recirculating air) conditions to zone air node
    state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).Temp =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp;
    state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).HumRat =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat;
    state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).Enthalpy =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy;

    state->dataUnitarySystems->unitarySys[0].ControlZoneNum = 1;
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataGlobal->SysSizingCalc = true;

    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand(1).RemainingOutputReqToDehumidSP = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -5000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = -5000.0;
    QZnReq = state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP;

    state->dataUnitarySystems->unitarySys[0].m_sysAvailSched->currentVal = 1.0; // unit is always available
    state->dataUnitarySystems->unitarySys[0].m_fanAvailSched->currentVal = 1.0; // fan is always available

    // set secondary air mass flow rate to zero
    state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).MassFlowRate = 0.0;
    // simulate PTHP zoneHVAC equipment
    bool HeatActive = false;
    bool CoolActive = false;
    Real64 latOut = 0.0;
    mySys->simulate(*state,
                    state->dataUnitarySystems->unitarySys[0].Name,
                    FirstHVACIteration,
                    0,
                    PTUnitNum,
                    HeatActive,
                    CoolActive,
                    0,
                    0,
                    true,
                    QUnitOut,
                    latOut);
    // apply mass conservation to determine secondary air mass flow rate
    SecondaryAirMassFlowRate = state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].AirInNode).MassFlowRate - PrimaryAirMassFlowRate;
    // check the terminal air mixer secondary air mass flow rate
    ASSERT_EQ(SecondaryAirMassFlowRate, state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).MassFlowRate);
    // check the cooling output delivered is within 2.0 Watt of zone cooling load
    ASSERT_NEAR(QZnReq, QUnitOut, 2.0);
}

TEST_F(EnergyPlusFixture, AirTerminalSingleDuctMixer_SimPTHP_ATMSupplySide)
{

    bool ErrorsFound(false);
    bool FirstHVACIteration(false);
    Real64 HVACInletMassFlowRate(0.0);
    Real64 PrimaryAirMassFlowRate(0.0);
    Real64 SecondaryAirMassFlowRate(0.0);
    Real64 ATMixerOutletMassFlowRate(0.0);
    Real64 QUnitOut(0.0);
    Real64 QZnReq(0.0);
    int PTUnitNum(1);

    std::string const idf_objects = delimited_string({

        "AirTerminal:SingleDuct:Mixer,",
        "    SPACE1-1 DOAS Air Terminal,  !- Name",
        "    ZoneHVAC:PackagedTerminalHeatPump,  !- ZoneHVAC Terminal Unit Object Type",
        "    SPACE1-1 Heat Pump,         !- ZoneHVAC Terminal Unit Name",
        "    SPACE1-1 Supply Inlet,      !- Terminal Unit Outlet Node Name",
        "    SPACE1-1 Air Terminal Mixer Primary Inlet,  !- Terminal Unit Primary Air Inlet Node Name",
        "    SPACE1-1 Heat Pump Outlet,  !- Terminal Unit Secondary Air Inlet Node Name",
        "    SupplySide;                 !- Terminal Unit Connection Type",

        "ZoneHVAC:AirDistributionUnit,",
        "    SPACE1-1 DOAS ATU,       !- Name",
        "    SPACE1-1 Supply Inlet,   !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:Mixer,  !- Air Terminal Object Type",
        "    SPACE1-1 DOAS Air Terminal;  !- Air Terminal Name",

        "Schedule:Compact,",
        "    FanAvailSched,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 16",
        "    1.0;                     !- Field 17",

        "Schedule:Compact,",
        "    CyclingFanSch,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    0.0;                     !- Field 4",

        "ZoneHVAC:EquipmentList,",
        "    SPACE1-1 Equipment,      !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE1-1 DOAS ATU,       !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction",
        "    ,                        !- Zone Equipment 1 Sequential Heating Fraction",
        "    ZoneHVAC:PackagedTerminalHeatPump,  !- Zone Equipment 2 Object Type",
        "    SPACE1-1 Heat Pump,      !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    2,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 2 Sequential Cooling Fraction",
        "    ;                        !- Zone Equipment 2 Sequential Heating Fraction",

        "  ZoneHVAC:PackagedTerminalHeatPump,",
        "    SPACE1-1 Heat Pump,      !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    SPACE1-1 Heat Pump Inlet,!- Air Inlet Node Name",
        "    SPACE1-1 Heat Pump Outlet,  !- Air Outlet Node Name",
        "    ,                        !- Outdoor Air Mixer Object Type",
        "    ,                        !- Outdoor Air Mixer Name",
        "    0.500,                   !- Supply Air Flow Rate During Cooling Operation {m3/s}",
        "    0.500,                   !- Supply Air Flow Rate During Heating Operation {m3/s}",
        "    ,                        !- Supply Air Flow Rate When No Cooling or Heating is Needed {m3/s}",
        "    ,                        !- No Load Supply Air Flow Rate Control Set To Low Speed",
        "    0,                       !- Outdoor Air Flow Rate During Cooling Operation {m3/s}",
        "    0,                       !- Outdoor Air Flow Rate During Heating Operation {m3/s}",
        "    0,                       !- Outdoor Air Flow Rate When No Cooling or Heating is Needed {m3/s}",
        "    Fan:OnOff,               !- Supply Air Fan Object Type",
        "    SPACE1-1 Supply Fan,     !- Supply Air Fan Name",
        "    Coil:Heating:DX:SingleSpeed,  !- Heating Coil Object Type",
        "    SPACE1-1 HP Heating Mode,     !- Heating Coil Name",
        "    0.001,                    !- Heating Convergence Tolerance {dimensionless}",
        "    Coil:Cooling:DX:SingleSpeed,  !- Cooling Coil Object Type",
        "    SPACE1-1 HP Cooling Mode,     !- Cooling Coil Name",
        "    0.001,                   !- Cooling Convergence Tolerance {dimensionless}",
        "    Coil:Heating:Fuel,        !- Supplemental Heating Coil Object Type",
        "    SPACE1-1 HP Supp Coil,   !- Supplemental Heating Coil Name",
        "    50.0,                    !- Maximum Supply Air Temperature from Supplemental Heater {C}",
        "    20.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Supplemental Heater Operation {C}",
        "    BlowThrough,             !- Fan Placement",
        "    CyclingFanSch;           !- Supply Air Fan Operating Mode Schedule Name",

        "Fan:OnOff,",
        "    SPACE1-1 Supply Fan,     !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    75,                      !- Pressure Rise {Pa}",
        "    0.500,                   !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1,                       !- Motor In Airstream Fraction",
        "    SPACE1-1 Heat Pump Inlet,!- Air Inlet Node Name",
        "    SPACE1-1 Zone Unit Fan Outlet;  !- Air Outlet Node Name",

        "  Coil:Heating:DX:SingleSpeed,",
        "    SPACE1-1 HP Heating Mode,!- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    7000.0,                  !- Gross Rated Heating Capacity {W}",
        "    3.75,                    !- Gross Rated Heating COP {W/W}",
        "    0.500,                   !- Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Rated Supply Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Supply Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    SPACE1-1 Cooling Coil Outlet,  !- Air Inlet Node Name",
        "    SPACE1-1 Heating Coil Outlet,  !- Air Outlet Node Name",
        "    HPACHeatCapFT,           !- Heating Capacity Function of Temperature Curve Name",
        "    HPACHeatCapFFF,          !- Heating Capacity Function of Flow Fraction Curve Name",
        "    HPACHeatEIRFT,           !- Energy Input Ratio Function of Temperature Curve Name",
        "    HPACHeatEIRFFF,          !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACCOOLPLFFPLR,         !- Part Load Fraction Correlation Curve Name",
        "    ,                        !- Defrost Energy Input Ratio Function of Temperature Curve Name",
        "    2.0,                     !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Outdoor Dry-Bulb Temperature to Turn On Compressor {C}",
        "    5.0,                     !- Maximum Outdoor Dry-Bulb Temperature for Defrost Operation {C}",
        "    200.0,                   !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    Resistive,               !- Defrost Strategy",
        "    TIMED,                   !- Defrost Control",
        "    0.166667,                !- Defrost Time Period Fraction",
        "    Autosize;                !- Resistive Defrost Heater Capacity {W}",

        "  Coil:Cooling:DX:SingleSpeed,",
        "    SPACE1-1 HP Cooling Mode,!- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    7030.0,                  !- Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Gross Rated Sensible Heat Ratio",
        "    3.0,                     !- Gross Rated Cooling COP {W/W}",
        "    0.500,                   !- Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    SPACE1-1 Zone Unit Fan Outlet, !- Air Inlet Node Name",
        "    SPACE1-1 Cooling Coil Outlet,  !- Air Outlet Node Name",
        "    HPACCoolCapFT,           !- Total Cooling Capacity Function of Temperature Curve Name",
        "    HPACCoolCapFFF,          !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    HPACEIRFT,               !- Energy Input Ratio Function of Temperature Curve Name",
        "    HPACEIRFFF,              !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACPLFFPLR;             !- Part Load Fraction Correlation Curve Name",

        "Coil:Heating:Fuel,",
        "    SPACE1-1 HP Supp Coil,   !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    NaturalGas,              !- Fuel Type",
        "    0.8,                     !- Gas Burner Efficiency",
        "    10000.0,                 !- Nominal Capacity {W}",
        "    SPACE1-1 Heating Coil Outlet,  !- Air Inlet Node Name",
        "    SPACE1-1 Heat Pump Outlet;  !- Air Outlet Node Name",

        "  Curve:Quadratic,",
        "    HPACCoolCapFFF,          !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACEIRFFF,              !- Name",
        "    1.1552,                  !- Coefficient1 Constant",
        "    -0.1808,                 !- Coefficient2 x",
        "    0.0256,                  !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACPLFFPLR,             !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACHeatEIRFFF,          !- Name",
        "    1.3824,                  !- Coefficient1 Constant",
        "    -0.4336,                 !- Coefficient2 x",
        "    0.0512,                  !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACCOOLPLFFPLR,         !- Name",
        "    0.75,                    !- Coefficient1 Constant",
        "    0.25,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "  Curve:Cubic,",
        "    HPACHeatCapFT,           !- Name",
        "    0.758746,                !- Coefficient1 Constant",
        "    0.027626,                !- Coefficient2 x",
        "    0.000148716,             !- Coefficient3 x**2",
        "    0.0000034992,            !- Coefficient4 x**3",
        "    -20.0,                   !- Minimum Value of x",
        "    20.0,                    !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Cubic,",
        "    HPACHeatCapFFF,          !- Name",
        "    0.84,                    !- Coefficient1 Constant",
        "    0.16,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Cubic,",
        "    HPACHeatEIRFT,           !- Name",
        "    1.19248,                 !- Coefficient1 Constant",
        "    -0.0300438,              !- Coefficient2 x",
        "    0.00103745,              !- Coefficient3 x**2",
        "    -0.000023328,            !- Coefficient4 x**3",
        "    -20.0,                   !- Minimum Value of x",
        "    20.0,                    !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Cubic,",
        "    FanEffRatioCurve,        !- Name",
        "    0.33856828,              !- Coefficient1 Constant",
        "    1.72644131,              !- Coefficient2 x",
        "    -1.49280132,             !- Coefficient3 x**2",
        "    0.42776208,              !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    0.3,                     !- Minimum Curve Output",
        "    1.0;                     !- Maximum Curve Output",

        "  Curve:Exponent,",
        "    FanPowerRatioCurve,      !- Name",
        "    0.0,                     !- Coefficient1 Constant",
        "    1.0,                     !- Coefficient2 Constant",
        "    3.0,                     !- Coefficient3 Constant",
        "    0.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    0.01,                    !- Minimum Curve Output",
        "    1.5;                     !- Maximum Curve Output",

        "  Curve:Biquadratic,",
        "    HPACCoolCapFT,           !- Name",
        "    0.942587793,             !- Coefficient1 Constant",
        "    0.009543347,             !- Coefficient2 x",
        "    0.000683770,             !- Coefficient3 x**2",
        "    -0.011042676,            !- Coefficient4 y",
        "    0.000005249,             !- Coefficient5 y**2",
        "    -0.000009720,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    HPACEIRFT,               !- Name",
        "    0.342414409,             !- Coefficient1 Constant",
        "    0.034885008,             !- Coefficient2 x",
        "    -0.000623700,            !- Coefficient3 x**2",
        "    0.004977216,             !- Coefficient4 y",
        "    0.000437951,             !- Coefficient5 y**2",
        "    -0.000728028,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Zone,",
        "    SPACE1-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    239.247360229;           !- Volume {m3}",

        "ZoneHVAC:EquipmentConnections,",
        "    SPACE1-1,                !- Zone Name",
        "    SPACE1-1 Equipment,      !- Zone Conditioning Equipment List Name",
        "    SPACE1-1 Inlets,         !- Zone Air Inlet Node or NodeList Name",
        "    SPACE1-1 Heat Pump Inlet,!- Zone Air Exhaust Node or NodeList Name",
        "    SPACE1-1 Zone Air Node,  !- Zone Air Node Name",
        "    SPACE1-1 Return Outlet;  !- Zone Return Air Node Name",

        "NodeList,",
        "    SPACE1-1 Inlets,         !- Name",
        "    SPACE1-1 Supply Inlet;   !- Node 1 Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);
    HVACSystemData *mySys;
    mySys = UnitarySystems::UnitarySys::factory(*state, HVAC::UnitarySysType::Unitary_AnyCoilType, "SPACE1-1 Heat Pump", true, 0);
    UnitarySystems::UnitarySys thisSys;
    state->dataZoneEquip->ZoneEquipInputsFilled = true;
    thisSys = state->dataUnitarySystems->unitarySys[0];
    thisSys.getUnitarySystemInput(*state, "SPACE1-1 Heat Pump", true, 0);
    state->dataUnitarySystems->getInputOnceFlag = false;

    // get input test for terminal air single duct mixer on supply side of PTHP
    ASSERT_EQ(1, state->dataSingleDuct->NumATMixers);
    EXPECT_EQ("SPACE1-1 DOAS AIR TERMINAL", state->dataSingleDuct->SysATMixer(1).Name);                 // single duct air terminal mixer name
    EXPECT_EQ((int)HVAC::MixerType::SupplySide, (int)state->dataSingleDuct->SysATMixer(1).type);        // air terminal mixer connection type
    EXPECT_EQ("AIRTERMINAL:SINGLEDUCT:MIXER", state->dataDefineEquipment->AirDistUnit(1).EquipType(1)); // Air distribution unit equipment type
    EXPECT_EQ("ZoneHVAC:PackagedTerminalHeatPump", state->dataUnitarySystems->unitarySys[0].UnitType);  // zoneHVAC equipment type

    // set input variables
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutDryBulbTemp = 35.0;
    state->dataEnvrn->OutHumRat = 0.0098;
    state->dataEnvrn->OutEnthalpy = Psychrometrics::PsyHFnTdbW(state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutHumRat);
    state->dataEnvrn->StdRhoAir = 1.20;
    HVACInletMassFlowRate = 0.50;
    PrimaryAirMassFlowRate = 0.1;

    state->dataGlobal->BeginEnvrnFlag = false;

    // set zoneNode air condition
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp = 24.0;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat = 0.0075;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp,
                                   state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat);

    state->dataUnitarySystems->unitarySys[0].MaxCoolAirMassFlow = HVACInletMassFlowRate; // supply air mass flow rate
    state->dataHVACGlobal->TurnFansOff = false;
    state->dataHVACGlobal->TurnFansOn = true;

    PTUnitNum = 1;
    state->dataUnitarySystems->unitarySys[0].m_FanOpMode = HVAC::FanOp::Cycling;
    // initialize mass flow rates
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].AirInNode).MassFlowRate = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].m_ATMixerPriNode).MassFlowRate = PrimaryAirMassFlowRate;
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].m_ATMixerPriNode).MassFlowRateMaxAvail = PrimaryAirMassFlowRate;

    // set fan parameters
    state->dataFans->fans(1)->maxAirMassFlowRate = HVACInletMassFlowRate;
    state->dataFans->fans(1)->inletAirMassFlowRate = HVACInletMassFlowRate;
    state->dataFans->fans(1)->rhoAirStdInit = state->dataEnvrn->StdRhoAir;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->inletNodeNum).MassFlowRateMaxAvail = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->outletNodeNum).MassFlowRateMax = HVACInletMassFlowRate;

    // set DX coil rated performance parameters
    state->dataDXCoils->DXCoil(1).RatedCBF(1) = 0.05;
    state->dataDXCoils->DXCoil(1).RatedAirMassFlowRate(1) = HVACInletMassFlowRate;

    // primary air condition at outside air condition
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].m_ATMixerPriNode).Temp = state->dataEnvrn->OutDryBulbTemp;
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].m_ATMixerPriNode).HumRat = state->dataEnvrn->OutHumRat;
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].m_ATMixerPriNode).Enthalpy = state->dataEnvrn->OutEnthalpy;

    // set PTUnit inlet condition to zone air node
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].AirInNode).Temp =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp;
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].AirInNode).HumRat =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat;
    state->dataLoopNodes->Node(state->dataUnitarySystems->unitarySys[0].AirInNode).Enthalpy =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy;

    state->dataUnitarySystems->unitarySys[0].ControlZoneNum = 1;
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataGlobal->SysSizingCalc = true;

    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand(1).RemainingOutputReqToDehumidSP = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -5000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = -5000.0;
    QZnReq = state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP;

    state->dataUnitarySystems->unitarySys[0].m_sysAvailSched->currentVal = 1.0; // unit is always available
    state->dataUnitarySystems->unitarySys[0].m_fanAvailSched->currentVal = 1.0; // fan is always available

    // set secondary air mass flow rate to zero
    state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).MassFlowRate = 0.0;
    // simulate PTHP zoneHVAC equipment
    bool HeatActive = false;
    bool CoolActive = false;
    Real64 latOut = 0.0;
    mySys->simulate(*state,
                    state->dataUnitarySystems->unitarySys[0].Name,
                    FirstHVACIteration,
                    0,
                    PTUnitNum,
                    HeatActive,
                    CoolActive,
                    0,
                    0,
                    true,
                    QUnitOut,
                    latOut);
    // apply mass conservation to determine secondary mass flow rate
    SecondaryAirMassFlowRate = state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).MassFlowRate;
    // check the terminal air mixer secondary air mass flow rate
    ASSERT_EQ(SecondaryAirMassFlowRate, state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).MassFlowRate);
    // check the terminal air mixer outlet air mass flow rate
    ATMixerOutletMassFlowRate = SecondaryAirMassFlowRate + PrimaryAirMassFlowRate;
    ASSERT_EQ(ATMixerOutletMassFlowRate, state->dataSingleDuct->SysATMixer(1).MixedAirMassFlowRate);
    // check the cooling output delivered is within 2.0 Watt of zone cooling load
    ASSERT_NEAR(QZnReq, QUnitOut, 2.0);
}

TEST_F(EnergyPlusFixture, AirTerminalSingleDuctMixer_SimVRF_ATMInletSide)
{

    bool ErrorsFound(false);
    bool FirstHVACIteration(false);
    Real64 HVACInletMassFlowRate(0.0);
    Real64 PrimaryAirMassFlowRate(0.0);
    Real64 SecondaryAirMassFlowRate(0.0);
    Real64 OnOffAirFlowRatio(1.0);
    Real64 LatOutputProvided(0.0);
    Real64 QUnitOutVRFTU(0.0);
    Real64 QZnReq(0.0);
    int VRFNum(1);
    int VRFTUNum(1);

    std::string const idf_objects = delimited_string({

        "AirTerminal:SingleDuct:Mixer,",
        "    SPACE1-1 DOAS Air Terminal,  !- Name",
        "    ZoneHVAC:TerminalUnit:VariableRefrigerantFlow,  !- ZoneHVAC Terminal Unit Object Type",
        "    TU1,                         !- ZoneHVAC Terminal Unit Name",
        "    TU1 Inlet Node,              !- Terminal Unit Outlet Node Name",
        "    SPACE1-1 Air Terminal Mixer Primary Inlet,    !- Terminal Unit Primary Air Inlet Node Name",
        "    SPACE1-1 Air Terminal Mixer Secondary Inlet,  !- Terminal Unit Secondary Air Inlet Node Name",
        "    InletSide;                                    !- Terminal Unit Connection Type",

        "ZoneHVAC:AirDistributionUnit,",
        "    SPACE1-1 DOAS ATU,       !- Name",
        "    TU1 Inlet Node,          !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:Mixer,  !- Air Terminal Object Type",
        "    SPACE1-1 DOAS Air Terminal;  !- Air Terminal Name",

        "Schedule:Compact,",
        "    FanAvailSched,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    1.0;                     !- Field 4",

        "  Schedule:Compact,",
        "    VRFAvailSched,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    1.0;                     !- Field 4",

        "Schedule:Compact,",
        "    CyclingFanSch,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    0.0;                     !- Field 4",

        "ZoneHVAC:EquipmentList,",
        "    SPACE1-1 Eq,             !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE1-1 DOAS ATU,       !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction",
        "    ,                        !- Zone Equipment 1 Sequential Heating Fraction",
        "    ZoneHVAC:TerminalUnit:VariableRefrigerantFlow,  !- Zone Equipment 2 Object Type",
        "    TU1,                     !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    2,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 2 Sequential Cooling Fraction",
        "    ;                        !- Zone Equipment 2 Sequential Heating Fraction",

        "  ZoneHVAC:TerminalUnit:VariableRefrigerantFlow,",
        "    TU1,                     !- Zone Terminal Unit Name",
        "    VRFAvailSched,           !- Terminal Unit Availability Schedule",
        "    TU1 Inlet Node,          !- Terminal Unit Air Inlet Node Name",
        "    TU1 Outlet Node,         !- Terminal Unit Air Outlet Node Name",
        "    0.500,                   !- Supply Air Flow Rate During Cooling Operation {m3/s}",
        "    0.500,                   !- Supply Air Flow Rate When No Cooling is Needed {m3/s}",
        "    0.500,                   !- Supply Air Flow Rate During Heating Operation {m3/s}",
        "    0.500,                   !- Supply Air Flow Rate When No Heating is Needed {m3/s}",
        "    0,                       !- Outdoor Air Flow Rate During Cooling Operation {m3/s}",
        "    0,                       !- Outdoor Air Flow Rate During Heating Operation {m3/s}",
        "    0,                       !- Outdoor Air Flow Rate When No Cooling or Heating is Needed {m3/s}",
        "    VRFFanSchedule,          !- Supply Air Fan Operating Mode Schedule Name",
        "    drawthrough,             !- Supply Air Fan Placement",
        "    Fan:ConstantVolume,      !- Supply Air Fan Object Type",
        "    TU1 VRF Supply Fan,      !- Supply Air Fan Object Name",
        "    ,                        !- Outside Air Mixer Object Type",
        "    ,                        !- Outside Air Mixer Object Name",
        "    COIL:Cooling:DX:VariableRefrigerantFlow,  !- Cooling Coil Object Type",
        "    TU1 VRF DX Cooling Coil, !- Cooling Coil Object Name",
        "    COIL:Heating:DX:VariableRefrigerantFlow,  !- Heating Coil Object Type",
        "    TU1 VRF DX Heating Coil, !- Heating Coil Object Name",
        "    30,                      !- Zone Terminal Unit On Parasitic Electric Energy Use {W}",
        "    20;                      !- Zone Terminal Unit Off Parasitic Electric Energy Use {W}",

        "  Fan:ConstantVolume,",
        "    TU1 VRF Supply Fan,      !- Name",
        "    VRFAvailSched,           !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    600.0,                   !- Pressure Rise {Pa}",
        "    0.500,                   !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1.0,                     !- Motor In Airstream Fraction",
        "    TU1 VRF DX HCoil Outlet Node,  !- Air Inlet Node Name",
        "    TU1 Outlet Node;         !- Air Outlet Node Name",

        "  COIL:Heating:DX:VariableRefrigerantFlow,",
        "    TU1 VRF DX Heating Coil, !- Name",
        "    VRFAvailSched,           !- Availability Schedule",
        "    7000.0,                  !- Gross Rated Heating Capacity {W}",
        "    0.500,                   !- Rated Air Flow Rate {m3/s}",
        "    TU1 VRF DX CCoil Outlet Node,  !- Coil Air Inlet Node",
        "    TU1 VRF DX HCoil Outlet Node,  !- Coil Air Outlet Node",
        "    VRFTUHeatCapFT,          !- Heating Capacity Ratio Modifier Function of Temperature Curve Name",
        "    VRFACCoolCapFFF;         !- Heating Capacity Modifier Function of Flow Fraction Curve Name",

        "  Curve:Cubic,",
        "    VRFTUHeatCapFT,          !- Name",
        "    -0.390708928227928,      !- Coefficient1 Constant",
        "    0.261815023760162,       !- Coefficient2 x",
        "    -0.0130431603151873,     !- Coefficient3 x**2",
        "    0.000178131745997821,    !- Coefficient4 x**3",
        "    0.0,                     !- Minimum Value of x",
        "    50.0,                    !- Maximum Value of x",
        "    0.5,                     !- Minimum Curve Output",
        "    1.5,                     !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  COIL:Cooling:DX:VariableRefrigerantFlow,",
        "    TU1 VRF DX Cooling Coil, !- Name",
        "    VRFAvailSched,           !- Availability Schedule Name",
        "    6600.0,                  !- Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Gross Rated Sensible Heat Ratio",
        "    0.500,                   !- Rated Air Flow Rate {m3/s}",
        "    VRFTUCoolCapFT,          !- Cooling Capacity Ratio Modifier Function of Temperature Curve Name",
        "    VRFACCoolCapFFF,         !- Cooling Capacity Modifier Curve Function of Flow Fraction Name",
        "    TU1 Inlet Node,          !- Coil Air Inlet Node",
        "    TU1 VRF DX CCoil Outlet Node,  !- Coil Air Outlet Node",
        "    ;                        !- Name of Water Storage Tank for Condensate Collection",

        "  Curve:Cubic,",
        "    VRFTUCoolCapFT,          !- Name",
        "    0.504547273506488,       !- Coefficient1 Constant",
        "    0.0288891279198444,      !- Coefficient2 x",
        "    -0.000010819418650677,   !- Coefficient3 x**2",
        "    0.0000101359395177008,   !- Coefficient4 x**3",
        "    0.0,                     !- Minimum Value of x",
        "    50.0,                    !- Maximum Value of x",
        "    0.5,                     !- Minimum Curve Output",
        "    1.5,                     !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Quadratic,",
        "    VRFACCoolCapFFF,         !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "Zone,",
        "    SPACE1-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    239.247360229;           !- Volume {m3}",

        "  ZoneHVAC:EquipmentConnections,",
        "    SPACE1-1,                !- Zone Name",
        "    SPACE1-1 Eq,             !- Zone Conditioning Equipment List Name",
        "    SPACE1-1 In Nodes,       !- Zone Air Inlet Node or NodeList Name",
        "    SPACE1-1 Air Terminal Mixer Secondary Inlet, !- Zone Air Exhaust Node or NodeList Name",
        "    SPACE1-1 Node,           !- Zone Air Node Name",
        "    SPACE1-1 RETURN OUTLET;  !- Zone Return Air Node Name",

        "  NodeList,",
        "    SPACE1-1 In Nodes,       !- Name",
        "    TU1 Outlet Node;         !- Node 1 Name",

        "  AirConditioner:VariableRefrigerantFlow,",
        "    VRF Heat Pump,           !- Heat Pump Name",
        "    VRFAvailSched,           !- Availability Schedule Name",
        "    15000.0,                 !- Gross Rated Total Cooling Capacity {W}",
        "    3.2917,                  !- Gross Rated Cooling COP {W/W}",
        "    -5,                      !- Minimum Outdoor Temperature in Cooling Mode {C}",
        "    43,                      !- Maximum Outdoor Temperature in Cooling Mode {C}",
        "    VRFCoolCapFT,            !- Cooling Capacity Ratio Modifier Function of Low Temperature Curve Name",
        "    VRFCoolCapFTBoundary,    !- Cooling Capacity Ratio Boundary Curve Name",
        "    VRFCoolCapFTHi,          !- Cooling Capacity Ratio Modifier Function of High Temperature Curve Name",
        "    VRFCoolEIRFT,            !- Cooling Energy Input Ratio Modifier Function of Low Temperature Curve Name",
        "    VRFCoolEIRFTBoundary,    !- Cooling Energy Input Ratio Boundary Curve Name",
        "    VRFCoolEIRFTHi,          !- Cooling Energy Input Ratio Modifier Function of High Temperature Curve Name",
        "    CoolingEIRLowPLR,        !- Cooling Energy Input Ratio Modifier Function of Low Part-Load Ratio Curve Name",
        "    CoolingEIRHiPLR,         !- Cooling Energy Input Ratio Modifier Function of High Part-Load Ratio Curve Name",
        "    CoolingCombRatio,        !- Cooling Combination Ratio Correction Factor Curve Name",
        "    VRFCPLFFPLR,             !- Cooling Part-Load Fraction Correlation Curve Name",
        "    15000.0,                 !- Gross Rated Heating Capacity {W}",
        "    ,                        !- Rated Heating Capacity Sizing Ratio {W/W}",
        "    3.5484,                  !- Gross Rated Heating COP {W/W}",
        "    -20,                     !- Minimum Outdoor Temperature in Heating Mode {C}",
        "    20,                      !- Maximum Outdoor Temperature in Heating Mode {C}",
        "    VRFHeatCapFT,            !- Heating Capacity Ratio Modifier Function of Low Temperature Curve Name",
        "    VRFHeatCapFTBoundary,    !- Heating Capacity Ratio Boundary Curve Name",
        "    VRFHeatCapFTHi,          !- Heating Capacity Ratio Modifier Function of High Temperature Curve Name",
        "    VRFHeatEIRFT,            !- Heating Energy Input Ratio Modifier Function of Low Temperature Curve Name",
        "    VRFHeatEIRFTBoundary,    !- Heating Energy Input Ratio Boundary Curve Name",
        "    VRFHeatEIRFTHi,          !- Heating Energy Input Ratio Modifier Function of High Temperature Curve Name",
        "    WetBulbTemperature,      !- Heating Performance Curve Outdoor Temperature Type",
        "    HeatingEIRLowPLR,        !- Heating Energy Input Ratio Modifier Function of Low Part-Load Ratio Curve Name",
        "    HeatingEIRHiPLR,         !- Heating Energy Input Ratio Modifier Function of High Part-Load Ratio Curve Name",
        "    HeatingCombRatio,        !- Heating Combination Ratio Correction Factor Curve Name",
        "    VRFCPLFFPLR,             !- Heating Part-Load Fraction Correlation Curve Name",
        "    0.25,                    !- Minimum Heat Pump Part-Load Ratio {dimensionless}",
        "    SPACE1-1,                !- Zone Name for Master Thermostat Location",
        "    LoadPriority,            !- Master Thermostat Priority Control Type",
        "    ,                        !- Thermostat Priority Schedule Name",
        "    VRF Heat Pump TU List,   !- Zone Terminal Unit List Name",
        "    No,                      !- Heat Pump Waste Heat Recovery",
        "    30,                      !- Equivalent Piping Length used for Piping Correction Factor in Cooling Mode {m}",
        "    10,                      !- Vertical Height used for Piping Correction Factor {m}",
        "    CoolingLengthCorrectionFactor,  !- Piping Correction Factor for Length in Cooling Mode Curve Name",
        "    -0.000386,               !- Piping Correction Factor for Height in Cooling Mode Coefficient {1/m}",
        "    30,                      !- Equivalent Piping Length used for Piping Correction Factor in Heating Mode {m}",
        "    ,                        !- Piping Correction Factor for Length in Heating Mode Curve Name",
        "    ,                        !- Piping Correction Factor for Height in Heating Mode Coefficient {1/m}",
        "    15,                      !- Crankcase Heater Power per Compressor {W}",
        "    3,                       !- Number of Compressors {dimensionless}",
        "    0.33,                    !- Ratio of Compressor Size to Total Compressor Capacity {W/W}",
        "    7,                       !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater {C}",
        "    Resistive,               !- Defrost Strategy",
        "    Timed,                   !- Defrost Control",
        "    ,                        !- Defrost Energy Input Ratio Modifier Function of Temperature Curve Name",
        "    ,                        !- Defrost Time Period Fraction {dimensionless}",
        "    15000.0,                 !- Resistive Defrost Heater Capacity {W}",
        "    7,                       !- Maximum Outdoor Dry-bulb Temperature for Defrost Operation {C}",
        "    AirCooled,               !- Condenser Type",
        "    MyVRFOANode,             !- Condenser Inlet Node Name",
        "    ,                        !- Condenser Outlet Node Name",
        "    ,                        !- Water Condenser Volume Flow Rate {m3/s}",
        "    ,                        !- Evaporative Condenser Effectiveness {dimensionless}",
        "    ,                        !- Evaporative Condenser Air Flow Rate {m3/s}",
        "    0,                       !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    0,                       !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    Electricity;             !- Fuel Type",

        "  Curve:Biquadratic,",
        "    VRFCoolCapFT,            !- Name",
        "    0.576882692,             !- Coefficient1 Constant",
        "    0.017447952,             !- Coefficient2 x",
        "    0.000583269,             !- Coefficient3 x**2",
        "    -1.76324E-06,            !- Coefficient4 y",
        "    -7.474E-09,              !- Coefficient5 y**2",
        "    -1.30413E-07,            !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    24,                      !- Maximum Value of x",
        "    -5,                      !- Minimum Value of y",
        "    23,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Cubic,",
        "    VRFCoolCapFTBoundary,    !- Name",
        "    25.73473775,             !- Coefficient1 Constant",
        "    -0.03150043,             !- Coefficient2 x",
        "    -0.01416595,             !- Coefficient3 x**2",
        "    0,                       !- Coefficient4 x**3",
        "    11,                      !- Minimum Value of x",
        "    30,                      !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature;             !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    VRFCoolCapFTHi,          !- Name",
        "    0.6867358,               !- Coefficient1 Constant",
        "    0.0207631,               !- Coefficient2 x",
        "    0.0005447,               !- Coefficient3 x**2",
        "    -0.0016218,              !- Coefficient4 y",
        "    -4.259E-07,              !- Coefficient5 y**2",
        "    -0.0003392,              !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    24,                      !- Maximum Value of x",
        "    16,                      !- Minimum Value of y",
        "    43,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    VRFCoolEIRFT,            !- Name",
        "    0.989010541,             !- Coefficient1 Constant",
        "    -0.02347967,             !- Coefficient2 x",
        "    0.000199711,             !- Coefficient3 x**2",
        "    0.005968336,             !- Coefficient4 y",
        "    -1.0289E-07,             !- Coefficient5 y**2",
        "    -0.00015686,             !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    24,                      !- Maximum Value of x",
        "    -5,                      !- Minimum Value of y",
        "    23,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Cubic,",
        "    VRFCoolEIRFTBoundary,    !- Name",
        "    25.73473775,             !- Coefficient1 Constant",
        "    -0.03150043,             !- Coefficient2 x",
        "    -0.01416595,             !- Coefficient3 x**2",
        "    0,                       !- Coefficient4 x**3",
        "    15,                      !- Minimum Value of x",
        "    24,                      !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature;             !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    VRFCoolEIRFTHi,          !- Name",
        "    0.14351470,              !- Coefficient1 Constant",
        "    0.01860035,              !- Coefficient2 x",
        "    -0.0003954,              !- Coefficient3 x**2",
        "    0.02485219,              !- Coefficient4 y",
        "    0.00016329,              !- Coefficient5 y**2",
        "    -0.0006244,              !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    24,                      !- Maximum Value of x",
        "    16,                      !- Minimum Value of y",
        "    43,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Cubic,",
        "    CoolingEIRLowPLR,        !- Name",
        "    0.4628123,               !- Coefficient1 Constant",
        "    -1.0402406,              !- Coefficient2 x",
        "    2.17490997,              !- Coefficient3 x**2",
        "    -0.5974817,              !- Coefficient4 x**3",
        "    0,                       !- Minimum Value of x",
        "    1,                       !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature;             !- Output Unit Type",

        "  Curve:Quadratic,",
        "    CoolingEIRHiPLR,         !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    1.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Linear,",
        "    CoolingCombRatio,        !- Name",
        "    0.618055,                !- Coefficient1 Constant",
        "    0.381945,                !- Coefficient2 x",
        "    1.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    1.0,                     !- Minimum Curve Output",
        "    1.2,                     !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  CURVE:QUADRATIC,",
        "    VRFCPLFFPLR,             !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0,                     !- Maximum Value of x",
        "    0.85,                    !- Minimum Curve Output",
        "    1.0,                     !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    VRFHeatCapFT,            !- Name",
        "    1.014599599,             !- Coefficient1 Constant",
        "    -0.002506703,            !- Coefficient2 x",
        "    -0.000141599,            !- Coefficient3 x**2",
        "    0.026931595,             !- Coefficient4 y",
        "    1.83538E-06,             !- Coefficient5 y**2",
        "    -0.000358147,            !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    27,                      !- Maximum Value of x",
        "    -20,                     !- Minimum Value of y",
        "    15,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Cubic,",
        "    VRFHeatCapFTBoundary,    !- Name",
        "    -7.6000882,              !- Coefficient1 Constant",
        "    3.05090016,              !- Coefficient2 x",
        "    -0.1162844,              !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 x**3",
        "    15,                      !- Minimum Value of x",
        "    27,                      !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature;             !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    VRFHeatCapFTHi,          !- Name",
        "    1.161134821,             !- Coefficient1 Constant",
        "    0.027478868,             !- Coefficient2 x",
        "    -0.00168795,             !- Coefficient3 x**2",
        "    0.001783378,             !- Coefficient4 y",
        "    2.03208E-06,             !- Coefficient5 y**2",
        "    -6.8969E-05,             !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    27,                      !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    15,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    VRFHeatEIRFT,            !- Name",
        "    0.87465501,              !- Coefficient1 Constant",
        "    -0.01319754,             !- Coefficient2 x",
        "    0.00110307,              !- Coefficient3 x**2",
        "    -0.0133118,              !- Coefficient4 y",
        "    0.00089017,              !- Coefficient5 y**2",
        "    -0.00012766,             !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    27,                      !- Maximum Value of x",
        "    -20,                     !- Minimum Value of y",
        "    12,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Cubic,",
        "    VRFHeatEIRFTBoundary,    !- Name",
        "    -7.6000882,              !- Coefficient1 Constant",
        "    3.05090016,              !- Coefficient2 x",
        "    -0.1162844,              !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 x**3",
        "    15,                      !- Minimum Value of x",
        "    27,                      !- Maximum Value of x",
        "    -20,                     !- Minimum Curve Output",
        "    15,                      !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature;             !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    VRFHeatEIRFTHi,          !- Name",
        "    2.504005146,             !- Coefficient1 Constant",
        "    -0.05736767,             !- Coefficient2 x",
        "    4.07336E-05,             !- Coefficient3 x**2",
        "    -0.12959669,             !- Coefficient4 y",
        "    0.00135839,              !- Coefficient5 y**2",
        "    0.00317047,              !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    27,                      !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    15,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Cubic,",
        "    HeatingEIRLowPLR,        !- Name",
        "    0.1400093,               !- Coefficient1 Constant",
        "    0.6415002,               !- Coefficient2 x",
        "    0.1339047,               !- Coefficient3 x**2",
        "    0.0845859,               !- Coefficient4 x**3",
        "    0,                       !- Minimum Value of x",
        "    1,                       !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Quadratic,",
        "    HeatingEIRHiPLR,         !- Name",
        "    2.4294355,               !- Coefficient1 Constant",
        "    -2.235887,               !- Coefficient2 x",
        "    0.8064516,               !- Coefficient3 x**2",
        "    1.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Linear,",
        "    HeatingCombRatio,        !- Name",
        "    0.96034,                 !- Coefficient1 Constant",
        "    0.03966,                 !- Coefficient2 x",
        "    1.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    1.0,                     !- Minimum Curve Output",
        "    1.023,                   !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    CoolingLengthCorrectionFactor,  !- Name",
        "    1.0693794,               !- Coefficient1 Constant",
        "    -0.0014951,              !- Coefficient2 x",
        "    2.56E-06,                !- Coefficient3 x**2",
        "    -0.1151104,              !- Coefficient4 y",
        "    0.0511169,               !- Coefficient5 y**2",
        "    -0.0004369,              !- Coefficient6 x*y",
        "    8,                       !- Minimum Value of x",
        "    175,                     !- Maximum Value of x",
        "    0.5,                     !- Minimum Value of y",
        "    1.5,                     !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  ZoneTerminalUnitList,",
        "    VRF Heat Pump TU List,   !- Zone Terminal Unit List Name",
        "    TU1;                     !- Zone Terminal Unit Name 1",

        "  OutdoorAir:NodeList,",
        "    OutsideAirInletNodes;    !- Node or NodeList Name 1",

        "  NodeList,",
        "    OutsideAirInletNodes,    !- Name",
        "    MyVRFOANode;             !- Node 1 Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    // set input variables
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutDryBulbTemp = 35.0;
    state->dataEnvrn->OutHumRat = 0.0098;
    state->dataEnvrn->OutEnthalpy = Psychrometrics::PsyHFnTdbW(state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutHumRat);
    state->dataEnvrn->StdRhoAir = 1.20;

    state->dataGlobal->TimeStep = 1;

    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);

    GetVRFInput(*state);
    state->dataHVACVarRefFlow->GetVRFInputFlag = false;

    // get input test for terminal air single duct mixer on inlet side of VRF terminal unit
    ASSERT_EQ(1, state->dataSingleDuct->NumATMixers);
    EXPECT_EQ("SPACE1-1 DOAS AIR TERMINAL", state->dataSingleDuct->SysATMixer(1).Name);                 // single duct air terminal mixer name
    EXPECT_EQ((int)HVAC::MixerType::InletSide, (int)state->dataSingleDuct->SysATMixer(1).type);         // air terminal mixer connection type
    EXPECT_EQ("AIRTERMINAL:SINGLEDUCT:MIXER", state->dataDefineEquipment->AirDistUnit(1).EquipType(1)); // Air distribution unit equipment type
    EXPECT_EQ("TU1", state->dataHVACVarRefFlow->VRFTU(1).Name);                                         // zoneHVAC equipment name
    // EXPECT_EQ( "ZoneHVAC:TerminalUnit:VariableRefrigerantFlow", VRFTU( 1 ).Name ); // zoneHVAC equipment type

    state->dataGlobal->BeginEnvrnFlag = false;

    HVACInletMassFlowRate = 0.50;
    PrimaryAirMassFlowRate = 0.1;
    SecondaryAirMassFlowRate = HVACInletMassFlowRate - PrimaryAirMassFlowRate; // secondary air flow is VRFTU flow less primary air flow

    // set zoneNode air condition
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp = 24.0;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat = 0.0075;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp,
                                   state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat);

    state->dataHVACVarRefFlow->CoolingLoad.allocate(1);
    state->dataHVACVarRefFlow->HeatingLoad.allocate(1);
    state->dataHVACVarRefFlow->HeatingLoad(1) = false;
    state->dataHVACVarRefFlow->CoolingLoad(1) = true;
    state->dataHVACVarRefFlow->CompOnMassFlow = HVACInletMassFlowRate;    // supply air mass flow rate
    state->dataHVACVarRefFlow->OACompOnMassFlow = PrimaryAirMassFlowRate; // OA mass flow rate
    state->dataHVACVarRefFlow->CompOnFlowRatio = 1.0;                     // compressor is on
    state->dataHVACGlobal->TurnFansOff = false;
    state->dataHVACGlobal->TurnFansOn = true;

    VRFNum = 1;
    VRFTUNum = 1;
    state->dataHVACVarRefFlow->VRFTU(VRFTUNum).fanOp = HVAC::FanOp::Continuous;
    state->dataHVACVarRefFlow->VRFTU(VRFTUNum).isInZone = true;
    state->dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneAirNode = state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode;
    // VRFTU( VRFTUNum ).VRFTUOutletNodeNum
    // initialize mass flow rates
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUInletNodeNum).MassFlowRate = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerPriNode).MassFlowRate = PrimaryAirMassFlowRate;
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerPriNode).MassFlowRateMaxAvail = PrimaryAirMassFlowRate;

    // set fan parameters
    state->dataFans->fans(1)->maxAirMassFlowRate = HVACInletMassFlowRate;
    state->dataFans->fans(1)->inletAirMassFlowRate = HVACInletMassFlowRate;
    state->dataFans->fans(1)->rhoAirStdInit = state->dataEnvrn->StdRhoAir;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->inletNodeNum).MassFlowRateMaxAvail = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->outletNodeNum).MassFlowRateMax = HVACInletMassFlowRate;

    // set DX coil rated performance parameters
    state->dataDXCoils->DXCoil(1).RatedCBF(1) = 0.05;
    state->dataDXCoils->DXCoil(1).RatedAirMassFlowRate(1) = HVACInletMassFlowRate;

    // primary air condition set at outdoor air condition
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerPriNode).Temp = state->dataEnvrn->OutDryBulbTemp;
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerPriNode).HumRat = state->dataEnvrn->OutHumRat;
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerPriNode).Enthalpy = state->dataEnvrn->OutEnthalpy;

    // set secondary air (recirculating air) conditions to zone air node
    state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).Temp =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp;
    state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).HumRat =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat;
    state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).Enthalpy =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy;

    state->dataHVACVarRefFlow->VRFTU(1).ZoneNum = 1;
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataGlobal->SysSizingCalc = true;

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -5000.0;
    QZnReq = state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP;

    state->dataHVACVarRefFlow->VRFTU(VRFTUNum).availSched->currentVal = 1.0;    // unit is always available
    state->dataHVACVarRefFlow->VRFTU(VRFTUNum).fanAvailSched->currentVal = 1.0; // fan is always available

    // set secondary air mass flow rate to zero
    state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).MassFlowRate = 0.0;
    // Simulate zoneHVAC equipment (VRF terminal unit)
    SimVRF(*state, VRFTUNum, FirstHVACIteration, OnOffAirFlowRatio, QUnitOutVRFTU, LatOutputProvided, QZnReq);
    // check the terminal air mixer secondary air mass flow rate
    ASSERT_EQ(SecondaryAirMassFlowRate, state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).MassFlowRate);
    // check the terminal air mixer outlet flow rate must be equal to VRFTU mass flow rate
    ASSERT_EQ(HVACInletMassFlowRate, state->dataSingleDuct->SysATMixer(1).MixedAirMassFlowRate);
    // check the cooling output delivered is within 2.0 Watt of zone cooling load
    ASSERT_NEAR(QZnReq, QUnitOutVRFTU, 2.0);
}

TEST_F(EnergyPlusFixture, AirTerminalSingleDuctMixer_SimVRF_ATMSupplySide)
{

    bool ErrorsFound(false);
    bool FirstHVACIteration(false);
    Real64 HVACInletMassFlowRate(0.0);
    Real64 PrimaryAirMassFlowRate(0.0);
    Real64 SecondaryAirMassFlowRate(0.0);
    Real64 ATMixerOutletMassFlowRate(0.0);
    Real64 OnOffAirFlowRatio(1.0);
    Real64 LatOutputProvided(0.0);
    Real64 QUnitOutVRFTU(0.0);
    Real64 QZnReq(0.0);
    int VRFNum(1);
    int VRFTUNum(1);

    std::string const idf_objects = delimited_string({

        "AirTerminal:SingleDuct:Mixer,",
        "    SPACE1-1 DOAS Air Terminal,   !- Name",
        "    ZoneHVAC:TerminalUnit:VariableRefrigerantFlow,  !- ZoneHVAC Terminal Unit Object Type",
        "    TU1,                          !- ZoneHVAC Terminal Unit Name",
        "    TU1 Outlet Node,              !- Terminal Unit Outlet Node Name",
        "    SPACE1-1 Air Terminal Mixer Primary Inlet,    !- Terminal Unit Primary Air Inlet Node Name",
        "    SPACE1-1 Air Terminal Mixer Secondary Inlet,  !- Terminal Unit Secondary Air Inlet Node Name",
        "    SupplySide;                                   !- Terminal Unit Connection Type",

        "ZoneHVAC:AirDistributionUnit,",
        "    SPACE1-1 DOAS ATU,       !- Name",
        "    TU1 Outlet Node,         !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:Mixer,  !- Air Terminal Object Type",
        "    SPACE1-1 DOAS Air Terminal;    !- Air Terminal Name",

        "Schedule:Compact,",
        "    FanAvailSched,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    1.0;                     !- Field 4",

        "  Schedule:Compact,",
        "    VRFAvailSched,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    1.0;                     !- Field 4",

        "Schedule:Compact,",
        "    CyclingFanSch,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    0.0;                     !- Field 4",

        "ZoneHVAC:EquipmentList,",
        "    SPACE1-1 Eq,             !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE1-1 DOAS ATU,       !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction",
        "    ,                        !- Zone Equipment 1 Sequential Heating Fraction",
        "    ZoneHVAC:TerminalUnit:VariableRefrigerantFlow,  !- Zone Equipment 2 Object Type",
        "    TU1,                     !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    2,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 2 Sequential Cooling Fraction",
        "    ;                        !- Zone Equipment 2 Sequential Heating Fraction",

        "  ZoneHVAC:TerminalUnit:VariableRefrigerantFlow,",
        "    TU1,                     !- Zone Terminal Unit Name",
        "    VRFAvailSched,           !- Terminal Unit Availability Schedule",
        "    zTU1 Inlet Node,         !- Terminal Unit Air Inlet Node Name",
        "    SPACE1-1 Air Terminal Mixer Secondary Inlet, !- Terminal Unit Air Outlet Node Name",
        "    0.500,                   !- Supply Air Flow Rate During Cooling Operation {m3/s}",
        "    0.500,                   !- Supply Air Flow Rate When No Cooling is Needed {m3/s}",
        "    0.500,                   !- Supply Air Flow Rate During Heating Operation {m3/s}",
        "    0.500,                   !- Supply Air Flow Rate When No Heating is Needed {m3/s}",
        "    0,                       !- Outdoor Air Flow Rate During Cooling Operation {m3/s}",
        "    0,                       !- Outdoor Air Flow Rate During Heating Operation {m3/s}",
        "    0,                       !- Outdoor Air Flow Rate When No Cooling or Heating is Needed {m3/s}",
        "    VRFFanSchedule,          !- Supply Air Fan Operating Mode Schedule Name",
        "    drawthrough,             !- Supply Air Fan Placement",
        "    Fan:ConstantVolume,      !- Supply Air Fan Object Type",
        "    TU1 VRF Supply Fan,      !- Supply Air Fan Object Name",
        "    ,                        !- Outside Air Mixer Object Type",
        "    ,                        !- Outside Air Mixer Object Name",
        "    COIL:Cooling:DX:VariableRefrigerantFlow,  !- Cooling Coil Object Type",
        "    TU1 VRF DX Cooling Coil, !- Cooling Coil Object Name",
        "    COIL:Heating:DX:VariableRefrigerantFlow,  !- Heating Coil Object Type",
        "    TU1 VRF DX Heating Coil, !- Heating Coil Object Name",
        "    30,                      !- Zone Terminal Unit On Parasitic Electric Energy Use {W}",
        "    20;                      !- Zone Terminal Unit Off Parasitic Electric Energy Use {W}",

        "  Fan:ConstantVolume,",
        "    TU1 VRF Supply Fan,      !- Name",
        "    VRFAvailSched,           !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    600.0,                   !- Pressure Rise {Pa}",
        "    0.500,                   !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1.0,                     !- Motor In Airstream Fraction",
        "    TU1 VRF DX HCoil Outlet Node,  !- Air Inlet Node Name",
        "    SPACE1-1 Air Terminal Mixer Secondary Inlet; !- Air Outlet Node Name",

        "  COIL:Heating:DX:VariableRefrigerantFlow,",
        "    TU1 VRF DX Heating Coil, !- Name",
        "    VRFAvailSched,           !- Availability Schedule",
        "    7000.0,                  !- Gross Rated Heating Capacity {W}",
        "    0.500,                   !- Rated Air Flow Rate {m3/s}",
        "    TU1 VRF DX CCoil Outlet Node,  !- Coil Air Inlet Node",
        "    TU1 VRF DX HCoil Outlet Node,  !- Coil Air Outlet Node",
        "    VRFTUHeatCapFT,          !- Heating Capacity Ratio Modifier Function of Temperature Curve Name",
        "    VRFACCoolCapFFF;         !- Heating Capacity Modifier Function of Flow Fraction Curve Name",

        "  Curve:Cubic,",
        "    VRFTUHeatCapFT,          !- Name",
        "    -0.390708928227928,      !- Coefficient1 Constant",
        "    0.261815023760162,       !- Coefficient2 x",
        "    -0.0130431603151873,     !- Coefficient3 x**2",
        "    0.000178131745997821,    !- Coefficient4 x**3",
        "    0.0,                     !- Minimum Value of x",
        "    50.0,                    !- Maximum Value of x",
        "    0.5,                     !- Minimum Curve Output",
        "    1.5,                     !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  COIL:Cooling:DX:VariableRefrigerantFlow,",
        "    TU1 VRF DX Cooling Coil, !- Name",
        "    VRFAvailSched,           !- Availability Schedule Name",
        "    6600.0,                  !- Gross Rated Total Cooling Capacity {W}",
        "    0.75,                    !- Gross Rated Sensible Heat Ratio",
        "    0.500,                   !- Rated Air Flow Rate {m3/s}",
        "    VRFTUCoolCapFT,          !- Cooling Capacity Ratio Modifier Function of Temperature Curve Name",
        "    VRFACCoolCapFFF,         !- Cooling Capacity Modifier Curve Function of Flow Fraction Name",
        "    zTU1 Inlet Node,         !- Coil Air Inlet Node",
        "    TU1 VRF DX CCoil Outlet Node,  !- Coil Air Outlet Node",
        "    ;                        !- Name of Water Storage Tank for Condensate Collection",

        "  Curve:Cubic,",
        "    VRFTUCoolCapFT,          !- Name",
        "    0.504547273506488,       !- Coefficient1 Constant",
        "    0.0288891279198444,      !- Coefficient2 x",
        "    -0.000010819418650677,   !- Coefficient3 x**2",
        "    0.0000101359395177008,   !- Coefficient4 x**3",
        "    0.0,                     !- Minimum Value of x",
        "    50.0,                    !- Maximum Value of x",
        "    0.5,                     !- Minimum Curve Output",
        "    1.5,                     !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Quadratic,",
        "    VRFACCoolCapFFF,         !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "Zone,",
        "    SPACE1-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    239.247360229;           !- Volume {m3}",

        "  ZoneHVAC:EquipmentConnections,",
        "    SPACE1-1,                !- Zone Name",
        "    SPACE1-1 Eq,             !- Zone Conditioning Equipment List Name",
        "    SPACE1-1 In Nodes,       !- Zone Air Inlet Node or NodeList Name",
        "    zTU1 Inlet Node,         !- Zone Air Exhaust Node or NodeList Name",
        "    SPACE1-1 Node,           !- Zone Air Node Name",
        "    SPACE1-1 RETURN OUTLET;  !- Zone Return Air Node Name",

        "  NodeList,",
        "    SPACE1-1 In Nodes,       !- Name",
        "    TU1 Outlet Node;         !- Node 1 Name",

        "  AirConditioner:VariableRefrigerantFlow,",
        "    VRF Heat Pump,           !- Heat Pump Name",
        "    VRFAvailSched,           !- Availability Schedule Name",
        "    15000.0,                 !- Gross Rated Total Cooling Capacity {W}",
        "    3.2917,                  !- Gross Rated Cooling COP {W/W}",
        "    -5,                      !- Minimum Outdoor Temperature in Cooling Mode {C}",
        "    43,                      !- Maximum Outdoor Temperature in Cooling Mode {C}",
        "    VRFCoolCapFT,            !- Cooling Capacity Ratio Modifier Function of Low Temperature Curve Name",
        "    VRFCoolCapFTBoundary,    !- Cooling Capacity Ratio Boundary Curve Name",
        "    VRFCoolCapFTHi,          !- Cooling Capacity Ratio Modifier Function of High Temperature Curve Name",
        "    VRFCoolEIRFT,            !- Cooling Energy Input Ratio Modifier Function of Low Temperature Curve Name",
        "    VRFCoolEIRFTBoundary,    !- Cooling Energy Input Ratio Boundary Curve Name",
        "    VRFCoolEIRFTHi,          !- Cooling Energy Input Ratio Modifier Function of High Temperature Curve Name",
        "    CoolingEIRLowPLR,        !- Cooling Energy Input Ratio Modifier Function of Low Part-Load Ratio Curve Name",
        "    CoolingEIRHiPLR,         !- Cooling Energy Input Ratio Modifier Function of High Part-Load Ratio Curve Name",
        "    CoolingCombRatio,        !- Cooling Combination Ratio Correction Factor Curve Name",
        "    VRFCPLFFPLR,             !- Cooling Part-Load Fraction Correlation Curve Name",
        "    15000.0,                 !- Gross Rated Heating Capacity {W}",
        "    ,                        !- Rated Heating Capacity Sizing Ratio {W/W}",
        "    3.5484,                  !- Gross Rated Heating COP {W/W}",
        "    -20,                     !- Minimum Outdoor Temperature in Heating Mode {C}",
        "    20,                      !- Maximum Outdoor Temperature in Heating Mode {C}",
        "    VRFHeatCapFT,            !- Heating Capacity Ratio Modifier Function of Low Temperature Curve Name",
        "    VRFHeatCapFTBoundary,    !- Heating Capacity Ratio Boundary Curve Name",
        "    VRFHeatCapFTHi,          !- Heating Capacity Ratio Modifier Function of High Temperature Curve Name",
        "    VRFHeatEIRFT,            !- Heating Energy Input Ratio Modifier Function of Low Temperature Curve Name",
        "    VRFHeatEIRFTBoundary,    !- Heating Energy Input Ratio Boundary Curve Name",
        "    VRFHeatEIRFTHi,          !- Heating Energy Input Ratio Modifier Function of High Temperature Curve Name",
        "    WetBulbTemperature,      !- Heating Performance Curve Outdoor Temperature Type",
        "    HeatingEIRLowPLR,        !- Heating Energy Input Ratio Modifier Function of Low Part-Load Ratio Curve Name",
        "    HeatingEIRHiPLR,         !- Heating Energy Input Ratio Modifier Function of High Part-Load Ratio Curve Name",
        "    HeatingCombRatio,        !- Heating Combination Ratio Correction Factor Curve Name",
        "    VRFCPLFFPLR,             !- Heating Part-Load Fraction Correlation Curve Name",
        "    0.25,                    !- Minimum Heat Pump Part-Load Ratio {dimensionless}",
        "    SPACE1-1,                !- Zone Name for Master Thermostat Location",
        "    LoadPriority,            !- Master Thermostat Priority Control Type",
        "    ,                        !- Thermostat Priority Schedule Name",
        "    VRF Heat Pump TU List,   !- Zone Terminal Unit List Name",
        "    No,                      !- Heat Pump Waste Heat Recovery",
        "    30,                      !- Equivalent Piping Length used for Piping Correction Factor in Cooling Mode {m}",
        "    10,                      !- Vertical Height used for Piping Correction Factor {m}",
        "    CoolingLengthCorrectionFactor,  !- Piping Correction Factor for Length in Cooling Mode Curve Name",
        "    -0.000386,               !- Piping Correction Factor for Height in Cooling Mode Coefficient {1/m}",
        "    30,                      !- Equivalent Piping Length used for Piping Correction Factor in Heating Mode {m}",
        "    ,                        !- Piping Correction Factor for Length in Heating Mode Curve Name",
        "    ,                        !- Piping Correction Factor for Height in Heating Mode Coefficient {1/m}",
        "    15,                      !- Crankcase Heater Power per Compressor {W}",
        "    3,                       !- Number of Compressors {dimensionless}",
        "    0.33,                    !- Ratio of Compressor Size to Total Compressor Capacity {W/W}",
        "    7,                       !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater {C}",
        "    Resistive,               !- Defrost Strategy",
        "    Timed,                   !- Defrost Control",
        "    ,                        !- Defrost Energy Input Ratio Modifier Function of Temperature Curve Name",
        "    ,                        !- Defrost Time Period Fraction {dimensionless}",
        "    15000.0,                 !- Resistive Defrost Heater Capacity {W}",
        "    7,                       !- Maximum Outdoor Dry-bulb Temperature for Defrost Operation {C}",
        "    AirCooled,               !- Condenser Type",
        "    MyVRFOANode,             !- Condenser Inlet Node Name",
        "    ,                        !- Condenser Outlet Node Name",
        "    ,                        !- Water Condenser Volume Flow Rate {m3/s}",
        "    ,                        !- Evaporative Condenser Effectiveness {dimensionless}",
        "    ,                        !- Evaporative Condenser Air Flow Rate {m3/s}",
        "    0,                       !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    0,                       !- Basin Heater Capacity {W/K}",
        "    ,                        !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    Electricity;             !- Fuel Type",

        "  Curve:Biquadratic,",
        "    VRFCoolCapFT,            !- Name",
        "    0.576882692,             !- Coefficient1 Constant",
        "    0.017447952,             !- Coefficient2 x",
        "    0.000583269,             !- Coefficient3 x**2",
        "    -1.76324E-06,            !- Coefficient4 y",
        "    -7.474E-09,              !- Coefficient5 y**2",
        "    -1.30413E-07,            !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    24,                      !- Maximum Value of x",
        "    -5,                      !- Minimum Value of y",
        "    23,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Cubic,",
        "    VRFCoolCapFTBoundary,    !- Name",
        "    25.73473775,             !- Coefficient1 Constant",
        "    -0.03150043,             !- Coefficient2 x",
        "    -0.01416595,             !- Coefficient3 x**2",
        "    0,                       !- Coefficient4 x**3",
        "    11,                      !- Minimum Value of x",
        "    30,                      !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature;             !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    VRFCoolCapFTHi,          !- Name",
        "    0.6867358,               !- Coefficient1 Constant",
        "    0.0207631,               !- Coefficient2 x",
        "    0.0005447,               !- Coefficient3 x**2",
        "    -0.0016218,              !- Coefficient4 y",
        "    -4.259E-07,              !- Coefficient5 y**2",
        "    -0.0003392,              !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    24,                      !- Maximum Value of x",
        "    16,                      !- Minimum Value of y",
        "    43,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    VRFCoolEIRFT,            !- Name",
        "    0.989010541,             !- Coefficient1 Constant",
        "    -0.02347967,             !- Coefficient2 x",
        "    0.000199711,             !- Coefficient3 x**2",
        "    0.005968336,             !- Coefficient4 y",
        "    -1.0289E-07,             !- Coefficient5 y**2",
        "    -0.00015686,             !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    24,                      !- Maximum Value of x",
        "    -5,                      !- Minimum Value of y",
        "    23,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Cubic,",
        "    VRFCoolEIRFTBoundary,    !- Name",
        "    25.73473775,             !- Coefficient1 Constant",
        "    -0.03150043,             !- Coefficient2 x",
        "    -0.01416595,             !- Coefficient3 x**2",
        "    0,                       !- Coefficient4 x**3",
        "    15,                      !- Minimum Value of x",
        "    24,                      !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature;             !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    VRFCoolEIRFTHi,          !- Name",
        "    0.14351470,              !- Coefficient1 Constant",
        "    0.01860035,              !- Coefficient2 x",
        "    -0.0003954,              !- Coefficient3 x**2",
        "    0.02485219,              !- Coefficient4 y",
        "    0.00016329,              !- Coefficient5 y**2",
        "    -0.0006244,              !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    24,                      !- Maximum Value of x",
        "    16,                      !- Minimum Value of y",
        "    43,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Cubic,",
        "    CoolingEIRLowPLR,        !- Name",
        "    0.4628123,               !- Coefficient1 Constant",
        "    -1.0402406,              !- Coefficient2 x",
        "    2.17490997,              !- Coefficient3 x**2",
        "    -0.5974817,              !- Coefficient4 x**3",
        "    0,                       !- Minimum Value of x",
        "    1,                       !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature;             !- Output Unit Type",

        "  Curve:Quadratic,",
        "    CoolingEIRHiPLR,         !- Name",
        "    1.0,                     !- Coefficient1 Constant",
        "    0.0,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    1.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Linear,",
        "    CoolingCombRatio,        !- Name",
        "    0.618055,                !- Coefficient1 Constant",
        "    0.381945,                !- Coefficient2 x",
        "    1.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    1.0,                     !- Minimum Curve Output",
        "    1.2,                     !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  CURVE:QUADRATIC,",
        "    VRFCPLFFPLR,             !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0,                     !- Maximum Value of x",
        "    0.85,                    !- Minimum Curve Output",
        "    1.0,                     !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    VRFHeatCapFT,            !- Name",
        "    1.014599599,             !- Coefficient1 Constant",
        "    -0.002506703,            !- Coefficient2 x",
        "    -0.000141599,            !- Coefficient3 x**2",
        "    0.026931595,             !- Coefficient4 y",
        "    1.83538E-06,             !- Coefficient5 y**2",
        "    -0.000358147,            !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    27,                      !- Maximum Value of x",
        "    -20,                     !- Minimum Value of y",
        "    15,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Cubic,",
        "    VRFHeatCapFTBoundary,    !- Name",
        "    -7.6000882,              !- Coefficient1 Constant",
        "    3.05090016,              !- Coefficient2 x",
        "    -0.1162844,              !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 x**3",
        "    15,                      !- Minimum Value of x",
        "    27,                      !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature;             !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    VRFHeatCapFTHi,          !- Name",
        "    1.161134821,             !- Coefficient1 Constant",
        "    0.027478868,             !- Coefficient2 x",
        "    -0.00168795,             !- Coefficient3 x**2",
        "    0.001783378,             !- Coefficient4 y",
        "    2.03208E-06,             !- Coefficient5 y**2",
        "    -6.8969E-05,             !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    27,                      !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    15,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    VRFHeatEIRFT,            !- Name",
        "    0.87465501,              !- Coefficient1 Constant",
        "    -0.01319754,             !- Coefficient2 x",
        "    0.00110307,              !- Coefficient3 x**2",
        "    -0.0133118,              !- Coefficient4 y",
        "    0.00089017,              !- Coefficient5 y**2",
        "    -0.00012766,             !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    27,                      !- Maximum Value of x",
        "    -20,                     !- Minimum Value of y",
        "    12,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Cubic,",
        "    VRFHeatEIRFTBoundary,    !- Name",
        "    -7.6000882,              !- Coefficient1 Constant",
        "    3.05090016,              !- Coefficient2 x",
        "    -0.1162844,              !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 x**3",
        "    15,                      !- Minimum Value of x",
        "    27,                      !- Maximum Value of x",
        "    -20,                     !- Minimum Curve Output",
        "    15,                      !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature;             !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    VRFHeatEIRFTHi,          !- Name",
        "    2.504005146,             !- Coefficient1 Constant",
        "    -0.05736767,             !- Coefficient2 x",
        "    4.07336E-05,             !- Coefficient3 x**2",
        "    -0.12959669,             !- Coefficient4 y",
        "    0.00135839,              !- Coefficient5 y**2",
        "    0.00317047,              !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    27,                      !- Maximum Value of x",
        "    -10,                     !- Minimum Value of y",
        "    15,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Cubic,",
        "    HeatingEIRLowPLR,        !- Name",
        "    0.1400093,               !- Coefficient1 Constant",
        "    0.6415002,               !- Coefficient2 x",
        "    0.1339047,               !- Coefficient3 x**2",
        "    0.0845859,               !- Coefficient4 x**3",
        "    0,                       !- Minimum Value of x",
        "    1,                       !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Quadratic,",
        "    HeatingEIRHiPLR,         !- Name",
        "    2.4294355,               !- Coefficient1 Constant",
        "    -2.235887,               !- Coefficient2 x",
        "    0.8064516,               !- Coefficient3 x**2",
        "    1.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Linear,",
        "    HeatingCombRatio,        !- Name",
        "    0.96034,                 !- Coefficient1 Constant",
        "    0.03966,                 !- Coefficient2 x",
        "    1.0,                     !- Minimum Value of x",
        "    1.5,                     !- Maximum Value of x",
        "    1.0,                     !- Minimum Curve Output",
        "    1.023,                   !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    CoolingLengthCorrectionFactor,  !- Name",
        "    1.0693794,               !- Coefficient1 Constant",
        "    -0.0014951,              !- Coefficient2 x",
        "    2.56E-06,                !- Coefficient3 x**2",
        "    -0.1151104,              !- Coefficient4 y",
        "    0.0511169,               !- Coefficient5 y**2",
        "    -0.0004369,              !- Coefficient6 x*y",
        "    8,                       !- Minimum Value of x",
        "    175,                     !- Maximum Value of x",
        "    0.5,                     !- Minimum Value of y",
        "    1.5,                     !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  ZoneTerminalUnitList,",
        "    VRF Heat Pump TU List,   !- Zone Terminal Unit List Name",
        "    TU1;                     !- Zone Terminal Unit Name 1",

        "  OutdoorAir:NodeList,",
        "    OutsideAirInletNodes;    !- Node or NodeList Name 1",

        "  NodeList,",
        "    OutsideAirInletNodes,    !- Name",
        "    MyVRFOANode;             !- Node 1 Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    // set input variables before input processing
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutDryBulbTemp = 35.0;
    state->dataEnvrn->OutHumRat = 0.0098;
    state->dataEnvrn->OutEnthalpy = Psychrometrics::PsyHFnTdbW(state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutHumRat);
    state->dataEnvrn->StdRhoAir = 1.20;

    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);

    GetVRFInput(*state);
    state->dataHVACVarRefFlow->GetVRFInputFlag = false;

    // get input test for terminal air single duct mixer on inlet side of VRF terminal unit
    ASSERT_EQ(1, state->dataSingleDuct->NumATMixers);
    EXPECT_EQ("SPACE1-1 DOAS AIR TERMINAL", state->dataSingleDuct->SysATMixer(1).Name);                 // single duct air terminal mixer name
    EXPECT_EQ((int)HVAC::MixerType::SupplySide, (int)state->dataSingleDuct->SysATMixer(1).type);        // air terminal mixer connection type
    EXPECT_EQ("AIRTERMINAL:SINGLEDUCT:MIXER", state->dataDefineEquipment->AirDistUnit(1).EquipType(1)); // Air distribution unit equipment type
    EXPECT_EQ("TU1", state->dataHVACVarRefFlow->VRFTU(1).Name);                                         // zoneHVAC equipment name

    state->dataGlobal->BeginEnvrnFlag = false;

    HVACInletMassFlowRate = 0.50;
    PrimaryAirMassFlowRate = 0.1;
    SecondaryAirMassFlowRate = HVACInletMassFlowRate; // secondary air mass flow rate is the same as that of the VRF terminal unit

    // set zoneNode air condition
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp = 24.0;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat = 0.0075;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp,
                                   state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat);

    state->dataHVACVarRefFlow->CoolingLoad.allocate(1);
    state->dataHVACVarRefFlow->HeatingLoad.allocate(1);
    state->dataHVACVarRefFlow->HeatingLoad(1) = false;
    state->dataHVACVarRefFlow->CoolingLoad(1) = true;
    state->dataHVACVarRefFlow->CompOnMassFlow = HVACInletMassFlowRate;    // supply air mass flow rate
    state->dataHVACVarRefFlow->OACompOnMassFlow = PrimaryAirMassFlowRate; // OA mass flow rate
    state->dataHVACVarRefFlow->CompOnFlowRatio = 1.0;                     // compressor is on
    state->dataHVACGlobal->TurnFansOff = false;
    state->dataHVACGlobal->TurnFansOn = true;

    VRFNum = 1;
    VRFTUNum = 1;
    state->dataHVACVarRefFlow->VRFTU(VRFTUNum).fanOp = HVAC::FanOp::Continuous;

    // initialize mass flow rates
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUInletNodeNum).MassFlowRate = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerPriNode).MassFlowRate = PrimaryAirMassFlowRate;
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerPriNode).MassFlowRateMaxAvail = PrimaryAirMassFlowRate;

    // set fan parameters
    state->dataFans->fans(1)->maxAirMassFlowRate = HVACInletMassFlowRate;
    state->dataFans->fans(1)->inletAirMassFlowRate = HVACInletMassFlowRate;
    state->dataFans->fans(1)->rhoAirStdInit = state->dataEnvrn->StdRhoAir;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->inletNodeNum).MassFlowRateMaxAvail = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->outletNodeNum).MassFlowRateMax = HVACInletMassFlowRate;

    // set DX coil rated performance parameters
    state->dataDXCoils->DXCoil(1).RatedCBF(1) = 0.05;
    state->dataDXCoils->DXCoil(1).RatedAirMassFlowRate(1) = HVACInletMassFlowRate;

    // primary air condition set at outdoor air condition
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerPriNode).Temp = state->dataEnvrn->OutDryBulbTemp;
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerPriNode).HumRat = state->dataEnvrn->OutHumRat;
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerPriNode).Enthalpy = state->dataEnvrn->OutEnthalpy;

    // set VRF terminal unit inlet condition to zone air node
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUInletNodeNum).Temp =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp;
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUInletNodeNum).HumRat =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat;
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUInletNodeNum).Enthalpy =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy;

    state->dataHVACVarRefFlow->VRFTU(1).ZoneNum = 1;
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataGlobal->SysSizingCalc = true;

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -4000.0;
    QZnReq = state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP;

    state->dataHVACVarRefFlow->VRFTU(VRFTUNum).availSched->currentVal = 1.0;    // unit is always available
    state->dataHVACVarRefFlow->VRFTU(VRFTUNum).fanAvailSched->currentVal = 1.0; // fan is always available

    // set secondary air mass flow rate to zero
    state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).MassFlowRate = 0.0;
    // simulate zoneHVAC equipment (VRF terminal unit)
    SimVRF(*state, VRFTUNum, FirstHVACIteration, OnOffAirFlowRatio, QUnitOutVRFTU, LatOutputProvided, QZnReq);

    // check the terminal air mixer secondary air mass flow rate
    ASSERT_EQ(SecondaryAirMassFlowRate, state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).MassFlowRate);
    // check the terminal air mixer outlet air mass flow rate
    ATMixerOutletMassFlowRate = SecondaryAirMassFlowRate + PrimaryAirMassFlowRate;
    ASSERT_EQ(ATMixerOutletMassFlowRate, state->dataSingleDuct->SysATMixer(1).MixedAirMassFlowRate);
    // check the cooling output delivered is within 4.0 Watt of zone cooling load
    ASSERT_NEAR(QZnReq, QUnitOutVRFTU, 4.0);
}

TEST_F(EnergyPlusFixture, AirTerminalSingleDuctMixer_SimVRFfluidCntrl_ATMInletSide)
{

    bool ErrorsFound(false);
    bool FirstHVACIteration(false);
    Real64 HVACInletMassFlowRate(0.0);
    Real64 PrimaryAirMassFlowRate(0.0);
    Real64 SecondaryAirMassFlowRate(0.0);
    Real64 OnOffAirFlowRatio(1.0);
    Real64 LatOutputProvided(0.0);
    Real64 QUnitOutVRFTU(0.0);
    Real64 QZnReq(0.0);
    int VRFNum(1);
    int VRFTUNum(1);

    std::string const idf_objects = delimited_string({

        "AirTerminal:SingleDuct:Mixer,",
        "    SPACE1-1 DOAS Air Terminal,  !- Name",
        "    ZoneHVAC:TerminalUnit:VariableRefrigerantFlow,  !- ZoneHVAC Terminal Unit Object Type",
        "    TU1,                         !- ZoneHVAC Terminal Unit Name",
        "    TU1 Inlet Node,              !- Terminal Unit Outlet Node Name",
        "    SPACE1-1 Air Terminal Mixer Primary Inlet,    !- Terminal Unit Primary Air Inlet Node Name",
        "    SPACE1-1 Air Terminal Mixer Secondary Inlet,  !- Terminal Unit Secondary Air Inlet Node Name",
        "    InletSide;                                    !- Terminal Unit Connection Type",

        "ZoneHVAC:AirDistributionUnit,",
        "    SPACE1-1 DOAS ATU,       !- Name",
        "    TU1 Inlet Node,          !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:Mixer,  !- Air Terminal Object Type",
        "    SPACE1-1 DOAS Air Terminal;  !- Air Terminal Name",

        "Schedule:Compact,",
        "    FanAvailSched,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    1.0;                     !- Field 4",

        "  Schedule:Compact,",
        "    VRFAvailSched,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    1.0;                     !- Field 4",

        "Schedule:Compact,",
        "    CyclingFanSch,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    0.0;                     !- Field 4",

        "ZoneHVAC:EquipmentList,",
        "    SPACE1-1 Eq,             !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE1-1 DOAS ATU,       !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction",
        "    ,                        !- Zone Equipment 1 Sequential Heating Fraction",
        "    ZoneHVAC:TerminalUnit:VariableRefrigerantFlow,  !- Zone Equipment 2 Object Type",
        "    TU1,                     !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    2,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 2 Sequential Cooling Fraction",
        "    ;                        !- Zone Equipment 2 Sequential Heating Fraction",

        "  ZoneHVAC:TerminalUnit:VariableRefrigerantFlow,",
        "    TU1,                     !- Zone Terminal Unit Name",
        "    VRFAvailSched,           !- Terminal Unit Availability Schedule",
        "    TU1 Inlet Node,          !- Terminal Unit Air Inlet Node Name",
        "    TU1 Outlet Node,         !- Terminal Unit Air Outlet Node Name",
        "    0.500,                   !- Supply Air Flow Rate During Cooling Operation {m3/s}",
        "    0.500,                   !- Supply Air Flow Rate When No Cooling is Needed {m3/s}",
        "    0.500,                   !- Supply Air Flow Rate During Heating Operation {m3/s}",
        "    0.500,                   !- Supply Air Flow Rate When No Heating is Needed {m3/s}",
        "    0,                       !- Outdoor Air Flow Rate During Cooling Operation {m3/s}",
        "    0,                       !- Outdoor Air Flow Rate During Heating Operation {m3/s}",
        "    0,                       !- Outdoor Air Flow Rate When No Cooling or Heating is Needed {m3/s}",
        "    VRFFanSchedule,          !- Supply Air Fan Operating Mode Schedule Name",
        "    drawthrough,             !- Supply Air Fan Placement",
        "    Fan:SystemModel,      !- Supply Air Fan Object Type",
        "    TU1 VRF Supply Fan,      !- Supply Air Fan Object Name",
        "    ,                        !- Outside Air Mixer Object Type",
        "    ,                        !- Outside Air Mixer Object Name",
        "    Coil:Cooling:DX:VariableRefrigerantFlow:FluidTemperatureControl,  !- Cooling Coil Object Type",
        "    TU1 VRF DX Cooling Coil, !- Cooling Coil Object Name",
        "    COIL:HEATING:DX:VARIABLEREFRIGERANTFLOW:FluidTemperatureControl,  !- Heating Coil Object Type",
        "    TU1 VRF DX Heating Coil, !- Heating Coil Object Name",
        "    30,                      !- Zone Terminal Unit On Parasitic Electric Energy Use {W}",
        "    20;                      !- Zone Terminal Unit Off Parasitic Electric Energy Use {W}",

        "  Fan:SystemModel,",
        "    TU1 VRF Supply Fan,      !- Name",
        "    VRFAvailSched,           !- Availability Schedule Name",
        "    TU1 VRF DX HCoil Outlet Node,  !- Air Inlet Node Name",
        "    TU1 Outlet Node,         !- Air Outlet Node Name",
        "    0.500,                   !- Design Maximum Air Flow Rate {m3/s}",
        "    Continuous,              !- Speed Control Method",
        "    0.0,                   !- Electric Power Minimum Flow Rate Fraction",
        "    600,                     !- Design Pressure Rise {Pa}",
        "    0.9,                     !- Motor Efficiency",
        "    1.0,                     !- Motor In Air Stream Fraction",
        "    autosize,               !- Design Electric Power Consumption {W}",
        "    TotalEfficiencyAndPressure,                        !- Design Power Sizing Method",
        "    ,                        !- Electric Power Per Unit Flow Rate {W/(m3/s)}",
        "    ,                        !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}",
        "    0.7,                     !- Fan Total Efficiency",
        "    TU1 Fan Power Curve,         !- Electric Power Function of Flow Fraction Curve Name",
        "    ,",
        "    ,",
        "    ,",
        "    ,",
        "    General;                 !- End-Use Subcategory",

        "  Curve:Quartic,",
        "    TU1 Fan Power Curve , !- Name",
        "    0.059,            !- Coefficient1 Constant",
        "    0,                !- Coefficient2 x       ",
        "    0,                !- Coefficient3 x**2    ",
        "    0.928,            !- Coefficient4 x**3    ",
        "    0,                !- Coefficient5 x**4    ",
        "    0.0,              !- Minimum Value of x",
        "    1.0,              !- Maximum Value of x",
        "    0.0,              !- Minimum Curve Output",
        "    1.0,              !- Maximum Curve Output",
        "    Dimensionless,    !- Input Unit Type for X",
        "    Dimensionless;    !- Output Unit Type",

        "  Coil:Heating:DX:VariableRefrigerantFlow:FluidTemperatureControl,",
        "    TU1 VRF DX Heating Coil, !- Name",
        "    VRFAvailSched,           !- Availability Schedule",
        "    TU1 VRF DX CCoil Outlet Node,  !- Coil Air Inlet Node",
        "    TU1 VRF DX HCoil Outlet Node,  !- Coil Air Outlet Node",
        "    6500.0,                  !- Rated Total Heating Capacity {W}",
        "    5,                       !- Indoor Unit Reference Subcooling Degrees Setpoint {C}    ",
        "    IUCondTempCurve;         !- Indoor Unit Condensing Temperature Function of Subcooling Curve Name",

        "  Curve:Quadratic,",
        "    IUCondTempCurve,         !- Name",
        "    -1.85,                   !- Coefficient1 Constant",
        "    0.411,                   !- Coefficient2 x",
        "    0.0196,                  !- Coefficient3 x**2",
        "    0,                       !- Minimum Value of x    ",
        "    20,                      !- Maximum Value of x    ",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature;             !- Output Unit Type",

        "  Coil:Cooling:DX:VariableRefrigerantFlow:FluidTemperatureControl,",
        "    TU1 VRF DX Cooling Coil, !- Name",
        "    VRFAvailSched,           !- Availability Schedule Name",
        "    TU1 INLET NODE,          !- Coil Air Inlet Node",
        "    TU1 VRF DX CCoil Outlet Node,  !- Coil Air Outlet Node",
        "    6600.0,                  !- Rated Total Cooling Capacity {W}",
        "    0.750,                   !- Rated Sensible Heat Ratio",
        "    3,                       !- Indoor Unit Reference Superheating Degrees Setpoint {C}    ",
        "    IUEvapTempCurve,         !- Indoor Unit Evaporating Temperature Function of Superheating Curve Name    ",
        "    ;                        !- Name of Water Storage Tank for Condensate Collection",

        "  Curve:Quadratic,",
        "    IUEvapTempCurve,         !- Name",
        "    0,                       !- Coefficient1 Constant",
        "    0.843,                   !- Coefficient2 x",
        "    0,                       !- Coefficient3 x**2",
        "    0,                       !- Minimum Value of x    ",
        "    15,                      !- Maximum Value of x    ",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature;             !- Output Unit Type",

        "Zone,",
        "    SPACE1-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    239.247360229;           !- Volume {m3}",

        "  ZoneHVAC:EquipmentConnections,",
        "    SPACE1-1,                !- Zone Name",
        "    SPACE1-1 Eq,             !- Zone Conditioning Equipment List Name",
        "    SPACE1-1 In Nodes,       !- Zone Air Inlet Node or NodeList Name",
        "    SPACE1-1 Air Terminal Mixer Secondary Inlet, !- Zone Air Exhaust Node or NodeList Name",
        "    SPACE1-1 Node,           !- Zone Air Node Name",
        "    SPACE1-1 RETURN OUTLET;  !- Zone Return Air Node Name",

        "  NodeList,",
        "    SPACE1-1 In Nodes,       !- Name",
        "    TU1 Outlet Node;         !- Node 1 Name",

        "  AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl,",
        "    VRF Heat Pump,           !- Heat Pump Name",
        "    VRFAvailSched,           !- Availability Schedule Name",
        "    VRF Heat Pump TU List,   !- Zone Terminal Unit List Name",
        "    R410A,                   !- Refrigerant Type",
        "    41300,                   !- Rated Evaporative Capacity {W}",
        "    0.344,                   !- Rated Compressor Power Per Unit of Rated Evaporative Capacity {W/W}",
        "    -5,                      !- Minimum Outdoor Air Temperature in Cooling Mode {C}",
        "    43,                      !- Maximum Outdoor Air Temperature in Cooling Mode {C}",
        "    -20,                     !- Minimum Outdoor Air Temperature in Heating Mode {C}",
        "    22,                      !- Maximum Outdoor Air Temperature in Heating Mode {C}",
        "    3,                       !- Reference Outdoor Unit Superheating Degrees {C}",
        "    3,                       !- Reference Outdoor Unit Subcooling Degrees {C}",
        "    ConstantTemp,            !- Refrigerant Temperature Control Algorithm for Indoor Unit",
        "    6,                       !- Reference Evaporating Temperature for Indoor Unit {C}",
        "    44,                      !- Reference Condensing Temperature for Indoor Unit {C}",
        "    6,                       !- Variable Evaporating Temperature Minimum for Indoor Unit {C}",
        "    13,                      !- Variable Evaporating Temperature Maximum for Indoor Unit {C}",
        "    42,                      !- Variable Condensing Temperature Minimum for Indoor Unit {C}",
        "    46,                      !- Variable Condensing Temperature Maximum for Indoor Unit {C}",
        "    4.12E-3,                 !- Outdoor Unit Fan Power Per Unit of Rated Evaporative Capacity {W/W}",
        "    7.26E-5,                 !- Outdoor Unit Fan Flow Rate Per Unit of Rated Evaporative Capacity {m3/s-W}",
        "    OUEvapTempCurve,         !- Outdoor Unit Evaporating Temperature Function of Superheating Curve Name",
        "    OUCondTempCurve,         !- Outdoor Unit Condensing Temperature Function of Subcooling Curve Name",
        "    0.0508,                  !- Diameter of main pipe connecting outdoor unit to indoor units {m}",
        "    30,                      !- Length of main pipe connecting outdoor unit to indoor units {m}",
        "    36,                      !- Equivalent length of main pipe connecting outdoor unit to indoor units {m}",
        "    5,                       !- Height difference between the outdoor unit node and indoor unit node of the main pipe {m}",
        "    0.02,                    !- Insulation thickness of the main pipe {m}",
        "    0.032,                   !- Thermal conductivity of the main pipe insulation material {W/m-K}",
        "    33,                      !- Crankcase Heater Power per Compressor {W}",
        "    1,                       !- Number of Compressors",
        "    0.33,                    !- Ratio of Compressor Size to Total Compressor Capacity",
        "    7,                       !- Maximum Outdoor Dry-bulb Temperature for Crankcase Heater {C}",
        "    ,                        !- Defrost Strategy",
        "    ,                        !- Defrost Control",
        "    ,                        !- Defrost Energy Input Ratio Modifier Function of Temperature Curve Name",
        "    ,                        !- Defrost Time Period Fraction",
        "    ,                        !- Resistive Defrost Heater Capacity {W}",
        "    ,                        !- Maximum Outdoor Dry-bulb Temperature for Defrost Operation {C}",
        "    4500000,                 !- Compressor maximum delta Pressure {Pa}",
        "    3,                       !- Number of Compressor Loading Index Entries",
        "    1500,                    !- Compressor Speed at Loading Index 1 {rev/min}",
        "    MinSpdCooling,           !- Loading Index 1 Evaporative Capacity Multiplier Function of Temperature Curve Name",
        "    MinSpdPower,             !- Loading Index 1 Compressor Power Multiplier Function of Temperature Curve Name",
        "    3600,                    !- Compressor Speed at Loading Index 2 {rev/min}",
        "    Spd1Cooling,             !- Loading Index 2 Evaporative Capacity Multiplier Function of Temperature Curve Name",
        "    Spd1Power,               !- Loading Index 2 Compressor Power Multiplier Function of Temperature Curve Name",
        "    6000,                    !- Compressor Speed at Loading Index 3 {rev/min}",
        "    Spd2Cooling,             !- Loading Index 3 Evaporative Capacity Multiplier Function of Temperature Curve Name",
        "    Spd2Power;               !- Loading Index 3 Compressor Power Multiplier Function of Temperature Curve Name",

        "  Curve:Quadratic,",
        "    OUEvapTempCurve,         !- Name",
        "    0,                       !- Coefficient1 Constant",
        "    6.05E-1,                 !- Coefficient2 x",
        "    2.50E-2,                 !- Coefficient3 x**2",
        "    0,                       !- Minimum Value of x    ",
        "    15,                      !- Maximum Value of x    ",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature;             !- Output Unit Type",

        "  Curve:Quadratic,",
        "    OUCondTempCurve,         !- Name",
        "    0,                       !- Coefficient1 Constant",
        "    -2.91,                   !- Coefficient2 x",
        "    1.180,                   !- Coefficient3 x**2",
        "    0,                       !- Minimum Value of x    ",
        "    20,                      !- Maximum Value of x    ",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature;             !- Output Unit Type",
        "	",
        "  Curve:Biquadratic,",
        "    MinSpdCooling,           !- Name",
        "    3.19E-01,                !- Coefficient1 Constant",
        "    -1.26E-03,               !- Coefficient2 x",
        "    -2.15E-05,               !- Coefficient3 x**2",
        "    1.20E-02,                !- Coefficient4 y",
        "    1.05E-04,                !- Coefficient5 y**2",
        "    -8.66E-05,               !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    65,                      !- Maximum Value of x",
        "    -30,                     !- Minimum Value of y",
        "    15,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    MinSpdPower,             !- Name",
        "    8.79E-02 ,               !- Coefficient1 Constant",
        "    -1.72E-04,               !- Coefficient2 x",
        "    6.93E-05 ,               !- Coefficient3 x**2",
        "    -3.38E-05,               !- Coefficient4 y",
        "    -8.10E-06,               !- Coefficient5 y**2",
        "    -1.04E-05,               !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    65,                      !- Maximum Value of x",
        "    -30,                     !- Minimum Value of y",
        "    15,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "	",
        "  Curve:Biquadratic,",
        "    Spd1Cooling,             !- Name",
        "    8.12E-01 ,               !- Coefficient1 Constant",
        "    -4.23E-03,               !- Coefficient2 x",
        "    -4.11E-05,               !- Coefficient3 x**2",
        "    2.97E-02 ,               !- Coefficient4 y",
        "    2.67E-04 ,               !- Coefficient5 y**2",
        "    -2.23E-04,               !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    65,                      !- Maximum Value of x",
        "    -30,                     !- Minimum Value of y",
        "    15,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    Spd1Power,               !- Name",
        "    3.26E-01 ,               !- Coefficient1 Constant",
        "    -2.20E-03,               !- Coefficient2 x",
        "    1.42E-04 ,               !- Coefficient3 x**2",
        "    2.82E-03 ,               !- Coefficient4 y",
        "    2.86E-05 ,               !- Coefficient5 y**2",
        "    -3.50E-05,               !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    65,                      !- Maximum Value of x",
        "    -30,                     !- Minimum Value of y",
        "    15,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    Spd2Cooling,             !- Name",
        "    1.32E+00 ,               !- Coefficient1 Constant",
        "    -6.20E-03,               !- Coefficient2 x",
        "    -7.10E-05,               !- Coefficient3 x**2",
        "    4.89E-02 ,               !- Coefficient4 y",
        "    4.59E-04 ,               !- Coefficient5 y**2",
        "    -3.67E-04,               !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    65,                      !- Maximum Value of x",
        "    -30,                     !- Minimum Value of y",
        "    15,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    Spd2Power,               !- Name",
        "    6.56E-01 ,               !- Coefficient1 Constant",
        "    -3.71E-03,               !- Coefficient2 x",
        "    2.07E-04 ,               !- Coefficient3 x**2",
        "    1.05E-02 ,               !- Coefficient4 y",
        "    7.36E-05 ,               !- Coefficient5 y**2",
        "    -1.57E-04,               !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    65,                      !- Maximum Value of x",
        "    -30,                     !- Minimum Value of y",
        "    15,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  ZoneTerminalUnitList,",
        "    VRF Heat Pump TU List,   !- Zone Terminal Unit List Name",
        "    TU1;                     !- Zone Terminal Unit Name 1",

        "  OutdoorAir:NodeList,",
        "    OutsideAirInletNodes;    !- Node or NodeList Name 1",

        "  NodeList,",
        "    OutsideAirInletNodes,    !- Name",
        "    MyVRFOANode;             !- Node 1 Name",

        "  FluidProperties:Name,",
        "    R410a,                   !- Fluid Name",
        "    Refrigerant;             !- Fluid Type",

        "  FluidProperties:Temperatures,",
        "    R410aSaturatedTemperatures,  !- Name",
        "    -72.000,-69.000,-66.000,-63.000,-60.000,-57.000,-54.000,",
        "    -51.000,-48.000,-45.000,-42.000,-39.000,-36.000,-33.000,",
        "    -30.000,-27.000,-24.000,-21.000,-18.000,-15.000,-12.000,",
        "    -9.000,-6.000,-3.000,0.000,3.000,6.000,9.000,",
        "    12.000,15.000,18.000,21.000,24.000,27.000,30.000,",
        "    33.000,36.000,39.000,42.000,45.000,48.000,51.000,",
        "    54.000,57.000,60.000,63.000,66.000,69.000;",

        "  FluidProperties:Temperatures,",
        "    R410aSuperHeatTemperatures,  !- Name",
        "    -72.000,-66.000,-60.000,-54.000,-48.000,-45.000,-42.000,",
        "    -39.000,-36.000,-33.000,-30.000,-27.000,-24.000,-21.000,",
        "    -18.000,-15.000,-12.000,-9.000,-6.000,-3.000,0.000,",
        "    3.000,6.000,9.000,12.000,15.000,18.000,21.000,",
        "    24.000,27.000,30.000,33.000,36.000,39.000,42.000,",
        "    45.000,48.000,51.000,54.000,57.000,60.000,63.000,",
        "    66.000,69.000;",

        "  FluidProperties:Saturated,",
        "    R410a,                   !- Name",
        "    Pressure,                !- Fluid Property Type",
        "    FluidGas,                !- Fluid Phase",
        "    R410aSaturatedTemperatures,  !- Temperature Values Name",
        "    3.1238E+04,3.7717E+04,4.5248E+04,5.3954E+04,6.3963E+04,7.5412E+04,8.8445E+04,",
        "    1.0321E+05,1.1988E+05,1.3860E+05,1.5955E+05,1.8292E+05,2.0888E+05,2.3762E+05,",
        "    2.6935E+05,3.0426E+05,3.4257E+05,3.8449E+05,4.3024E+05,4.8004E+05,5.3412E+05,",
        "    5.9273E+05,6.5609E+05,7.2446E+05,7.9808E+05,8.7722E+05,9.6214E+05,1.0531E+06,",
        "    1.1504E+06,1.2543E+06,1.3651E+06,1.4831E+06,1.6086E+06,1.7419E+06,1.8834E+06,",
        "    2.0334E+06,2.1923E+06,2.3604E+06,2.5382E+06,2.7261E+06,2.9246E+06,3.1341E+06,",
        "    3.3552E+06,3.5886E+06,3.8348E+06,4.0949E+06,4.3697E+06,4.6607E+06;",

        "  FluidProperties:Saturated,",
        "    R410a,                   !- Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    Fluid,                   !- Fluid Phase",
        "    R410aSaturatedTemperatures,  !- Temperature Values Name",
        "    9.8535E+04,1.0259E+05,1.0665E+05,1.1072E+05,1.1479E+05,1.1888E+05,1.2297E+05,",
        "    1.2707E+05,1.3119E+05,1.3532E+05,1.3947E+05,1.4363E+05,1.4782E+05,1.5202E+05,",
        "    1.5624E+05,1.6048E+05,1.6475E+05,1.6904E+05,1.7337E+05,1.7772E+05,1.8210E+05,",
        "    1.8652E+05,1.9097E+05,1.9547E+05,2.0000E+05,2.0458E+05,2.0920E+05,2.1388E+05,",
        "    2.1861E+05,2.2340E+05,2.2825E+05,2.3316E+05,2.3815E+05,2.4322E+05,2.4838E+05,",
        "    2.5363E+05,2.5899E+05,2.6447E+05,2.7008E+05,2.7585E+05,2.8180E+05,2.8797E+05,",
        "    2.9441E+05,3.0120E+05,3.0848E+05,3.1650E+05,3.2578E+05,3.3815E+05;",

        "  FluidProperties:Saturated,",
        "    R410a,                   !- Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    FluidGas,                !- Fluid Phase",
        "    R410aSaturatedTemperatures,  !- Temperature Values Name",
        "    3.8813E+05,3.8981E+05,3.9148E+05,3.9313E+05,3.9476E+05,3.9637E+05,3.9796E+05,",
        "    3.9953E+05,4.0108E+05,4.0260E+05,4.0410E+05,4.0557E+05,4.0701E+05,4.0842E+05,",
        "    4.0980E+05,4.1114E+05,4.1245E+05,4.1373E+05,4.1496E+05,4.1615E+05,4.1730E+05,",
        "    4.1840E+05,4.1945E+05,4.2045E+05,4.2139E+05,4.2227E+05,4.2308E+05,4.2382E+05,",
        "    4.2448E+05,4.2507E+05,4.2556E+05,4.2595E+05,4.2624E+05,4.2641E+05,4.2646E+05,",
        "    4.2635E+05,4.2609E+05,4.2564E+05,4.2498E+05,4.2408E+05,4.2290E+05,4.2137E+05,",
        "    4.1941E+05,4.1692E+05,4.1370E+05,4.0942E+05,4.0343E+05,3.9373E+05;",

        "  FluidProperties:Saturated,",
        "    R410a,                   !- Name",
        "    Density,                 !- Fluid Property Type",
        "    Fluid,                   !- Fluid Phase",
        "    R410aSaturatedTemperatures,  !- Temperature Values Name",
        "    1.4127E+03,1.4036E+03,1.3946E+03,1.3854E+03,1.3762E+03,1.3669E+03,1.3576E+03,",
        "    1.3482E+03,1.3387E+03,1.3291E+03,1.3194E+03,1.3097E+03,1.2998E+03,1.2898E+03,",
        "    1.2797E+03,1.2694E+03,1.2591E+03,1.2486E+03,1.2379E+03,1.2271E+03,1.2160E+03,",
        "    1.2048E+03,1.1934E+03,1.1818E+03,1.1699E+03,1.1578E+03,1.1454E+03,1.1328E+03,",
        "    1.1197E+03,1.1064E+03,1.0927E+03,1.0785E+03,1.0639E+03,1.0488E+03,1.0331E+03,",
        "    1.0167E+03,9.9971E+02,9.8187E+02,9.6308E+02,9.4319E+02,9.2198E+02,8.9916E+02,",
        "    8.7429E+02,8.4672E+02,8.1537E+02,7.7825E+02,7.3095E+02,6.5903E+02;",

        "  FluidProperties:Saturated,",
        "    R410a,                   !- Name",
        "    Density,                 !- Fluid Property Type",
        "    FluidGas,                !- Fluid Phase",
        "    R410aSaturatedTemperatures,  !- Temperature Values Name",
        "    1.3845E+00,1.6517E+00,1.9588E+00,2.3100E+00,2.7097E+00,3.1627E+00,3.6737E+00,",
        "    4.2482E+00,4.8916E+00,5.6098E+00,6.4088E+00,7.2952E+00,8.2758E+00,9.3578E+00,",
        "    1.0549E+01,1.1857E+01,1.3292E+01,1.4861E+01,1.6576E+01,1.8447E+01,2.0485E+01,",
        "    2.2702E+01,2.5113E+01,2.7732E+01,3.0575E+01,3.3659E+01,3.7005E+01,4.0634E+01,",
        "    4.4571E+01,4.8844E+01,5.3483E+01,5.8525E+01,6.4012E+01,6.9991E+01,7.6520E+01,",
        "    8.3666E+01,9.1511E+01,1.0016E+02,1.0973E+02,1.2038E+02,1.3233E+02,1.4585E+02,",
        "    1.6135E+02,1.7940E+02,2.0095E+02,2.2766E+02,2.6301E+02,3.1759E+02;",

        "  FluidProperties:Saturated,",
        "    R410a,                   !- Name",
        "    SpecificHeat,            !- Fluid Property Type",
        "    Fluid,                   !- Fluid Phase",
        "    R410aSaturatedTemperatures,  !- Temperature Values Name",
        "    1.3499E+03,1.3515E+03,1.3534E+03,1.3557E+03,1.3584E+03,1.3614E+03,1.3648E+03,",
        "    1.3686E+03,1.3728E+03,1.3774E+03,1.3825E+03,1.3881E+03,1.3941E+03,1.4007E+03,",
        "    1.4078E+03,1.4155E+03,1.4238E+03,1.4327E+03,1.4424E+03,1.4527E+03,1.4639E+03,",
        "    1.4759E+03,1.4888E+03,1.5027E+03,1.5177E+03,1.5340E+03,1.5515E+03,1.5706E+03,",
        "    1.5914E+03,1.6141E+03,1.6390E+03,1.6664E+03,1.6968E+03,1.7307E+03,1.7689E+03,",
        "    1.8123E+03,1.8622E+03,1.9204E+03,1.9895E+03,2.0732E+03,2.1774E+03,2.3116E+03,",
        "    2.4924E+03,2.7507E+03,3.1534E+03,3.8723E+03,5.5190E+03,1.2701E+04;",

        "  FluidProperties:Saturated,",
        "    R410a,                   !- Name",
        "    SpecificHeat,            !- Fluid Property Type",
        "    FluidGas,                !- Fluid Phase",
        "    R410aSaturatedTemperatures,  !- Temperature Values Name",
        "    7.2387E+02,7.3519E+02,7.4693E+02,7.5910E+02,7.7167E+02,7.8465E+02,7.9802E+02,",
        "    8.1178E+02,8.2594E+02,8.4050E+02,8.5546E+02,8.7085E+02,8.8668E+02,9.0298E+02,",
        "    9.1979E+02,9.3715E+02,9.5511E+02,9.7372E+02,9.9307E+02,1.0132E+03,1.0343E+03,",
        "    1.0564E+03,1.0796E+03,1.1042E+03,1.1302E+03,1.1580E+03,1.1877E+03,1.2196E+03,",
        "    1.2541E+03,1.2917E+03,1.3329E+03,1.3783E+03,1.4287E+03,1.4853E+03,1.5494E+03,",
        "    1.6228E+03,1.7078E+03,1.8078E+03,1.9274E+03,2.0735E+03,2.2562E+03,2.4922E+03,",
        "    2.8094E+03,3.2596E+03,3.9504E+03,5.1465E+03,7.7185E+03,1.7076E+04;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.1238E+04,              !- Pressure {Pa}",
        "    3.8813E+05,3.9245E+05,3.9675E+05,4.0105E+05,4.0536E+05,4.0753E+05,4.0970E+05,",
        "    4.1189E+05,4.1408E+05,4.1628E+05,4.1849E+05,4.2071E+05,4.2294E+05,4.2518E+05,",
        "    4.2743E+05,4.2969E+05,4.3196E+05,4.3425E+05,4.3655E+05,4.3885E+05,4.4118E+05,",
        "    4.4351E+05,4.4586E+05,4.4821E+05,4.5058E+05,4.5297E+05,4.5536E+05,4.5777E+05,",
        "    4.6020E+05,4.6263E+05,4.6508E+05,4.6754E+05,4.7002E+05,4.7251E+05,4.7501E+05,",
        "    4.7752E+05,4.8005E+05,4.8259E+05,4.8515E+05,4.8772E+05,4.9030E+05,4.9290E+05,",
        "    4.9551E+05,4.9813E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.1238E+04,              !- Pressure {Pa}",
        "    1.3845E+00,1.3404E+00,1.2997E+00,1.2617E+00,1.2262E+00,1.2092E+00,1.1928E+00,",
        "    1.1768E+00,1.1613E+00,1.1462E+00,1.1316E+00,1.1173E+00,1.1034E+00,1.0898E+00,",
        "    1.0766E+00,1.0638E+00,1.0512E+00,1.0390E+00,1.0271E+00,1.0154E+00,1.0040E+00,",
        "    9.9285E-01,9.8197E-01,9.7133E-01,9.6093E-01,9.5075E-01,9.4079E-01,9.3104E-01,",
        "    9.2150E-01,9.1215E-01,9.0299E-01,8.9403E-01,8.8524E-01,8.7662E-01,8.6817E-01,",
        "    8.5989E-01,8.5177E-01,8.4380E-01,8.3598E-01,8.2831E-01,8.2077E-01,8.1338E-01,",
        "    8.0612E-01,7.9899E-01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    4.5248E+04,              !- Pressure {Pa}",
        "    0.0000E+00,3.9148E+05,3.9593E+05,4.0034E+05,4.0474E+05,4.0694E+05,4.0915E+05,",
        "    4.1136E+05,4.1358E+05,4.1580E+05,4.1803E+05,4.2027E+05,4.2252E+05,4.2478E+05,",
        "    4.2705E+05,4.2933E+05,4.3161E+05,4.3391E+05,4.3622E+05,4.3854E+05,4.4088E+05,",
        "    4.4322E+05,4.4558E+05,4.4794E+05,4.5032E+05,4.5272E+05,4.5512E+05,4.5754E+05,",
        "    4.5997E+05,4.6241E+05,4.6486E+05,4.6733E+05,4.6981E+05,4.7231E+05,4.7481E+05,",
        "    4.7733E+05,4.7987E+05,4.8241E+05,4.8497E+05,4.8755E+05,4.9013E+05,4.9273E+05,",
        "    4.9535E+05,4.9797E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    4.5248E+04,              !- Pressure {Pa}",
        "    0.0000E+00,1.9588E+00,1.8968E+00,1.8395E+00,1.7863E+00,1.7610E+00,1.7365E+00,",
        "    1.7128E+00,1.6898E+00,1.6674E+00,1.6457E+00,1.6246E+00,1.6041E+00,1.5842E+00,",
        "    1.5647E+00,1.5458E+00,1.5273E+00,1.5093E+00,1.4918E+00,1.4747E+00,1.4580E+00,",
        "    1.4416E+00,1.4257E+00,1.4101E+00,1.3949E+00,1.3800E+00,1.3654E+00,1.3512E+00,",
        "    1.3372E+00,1.3236E+00,1.3102E+00,1.2971E+00,1.2843E+00,1.2717E+00,1.2594E+00,",
        "    1.2473E+00,1.2355E+00,1.2239E+00,1.2125E+00,1.2013E+00,1.1903E+00,1.1796E+00,",
        "    1.1690E+00,1.1586E+00;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    6.3963E+04,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,3.9476E+05,3.9935E+05,4.0388E+05,4.0614E+05,4.0839E+05,",
        "    4.1064E+05,4.1290E+05,4.1516E+05,4.1742E+05,4.1969E+05,4.2196E+05,4.2425E+05,",
        "    4.2654E+05,4.2884E+05,4.3114E+05,4.3346E+05,4.3579E+05,4.3813E+05,4.4047E+05,",
        "    4.4283E+05,4.4520E+05,4.4758E+05,4.4997E+05,4.5238E+05,4.5479E+05,4.5722E+05,",
        "    4.5966E+05,4.6211E+05,4.6457E+05,4.6705E+05,4.6954E+05,4.7204E+05,4.7455E+05,",
        "    4.7708E+05,4.7962E+05,4.8217E+05,4.8474E+05,4.8732E+05,4.8991E+05,4.9252E+05,",
        "    4.9513E+05,4.9777E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    6.3963E+04,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,2.7097E+00,2.6240E+00,2.5451E+00,2.5078E+00,2.4718E+00,",
        "    2.4370E+00,2.4034E+00,2.3708E+00,2.3393E+00,2.3086E+00,2.2789E+00,2.2500E+00,",
        "    2.2219E+00,2.1945E+00,2.1679E+00,2.1420E+00,2.1167E+00,2.0921E+00,2.0681E+00,",
        "    2.0446E+00,2.0217E+00,1.9994E+00,1.9776E+00,1.9562E+00,1.9354E+00,1.9150E+00,",
        "    1.8950E+00,1.8755E+00,1.8564E+00,1.8377E+00,1.8194E+00,1.8014E+00,1.7839E+00,",
        "    1.7666E+00,1.7497E+00,1.7332E+00,1.7169E+00,1.7010E+00,1.6854E+00,1.6700E+00,",
        "    1.6550E+00,1.6402E+00;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    8.8445E+04,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,3.9796E+05,4.0270E+05,4.0503E+05,4.0736E+05,",
        "    4.0967E+05,4.1198E+05,4.1429E+05,4.1660E+05,4.1891E+05,4.2122E+05,4.2354E+05,",
        "    4.2586E+05,4.2819E+05,4.3052E+05,4.3286E+05,4.3521E+05,4.3757E+05,4.3994E+05,",
        "    4.4232E+05,4.4470E+05,4.4710E+05,4.4951E+05,4.5193E+05,4.5436E+05,4.5680E+05,",
        "    4.5925E+05,4.6171E+05,4.6419E+05,4.6668E+05,4.6918E+05,4.7169E+05,4.7421E+05,",
        "    4.7675E+05,4.7930E+05,4.8186E+05,4.8443E+05,4.8702E+05,4.8962E+05,4.9223E+05,",
        "    4.9486E+05,4.9749E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    8.8445E+04,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,3.6737E+00,3.5570E+00,3.5024E+00,3.4500E+00,",
        "    3.3995E+00,3.3509E+00,3.3039E+00,3.2585E+00,3.2146E+00,3.1720E+00,3.1308E+00,",
        "    3.0907E+00,3.0518E+00,3.0140E+00,2.9772E+00,2.9414E+00,2.9065E+00,2.8726E+00,",
        "    2.8395E+00,2.8072E+00,2.7757E+00,2.7449E+00,2.7149E+00,2.6856E+00,2.6569E+00,",
        "    2.6289E+00,2.6015E+00,2.5747E+00,2.5485E+00,2.5228E+00,2.4977E+00,2.4731E+00,",
        "    2.4490E+00,2.4254E+00,2.4022E+00,2.3795E+00,2.3573E+00,2.3354E+00,2.3140E+00,",
        "    2.2930E+00,2.2724E+00;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.1988E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.0108E+05,4.0354E+05,4.0597E+05,",
        "    4.0838E+05,4.1077E+05,4.1315E+05,4.1552E+05,4.1788E+05,4.2025E+05,4.2261E+05,",
        "    4.2497E+05,4.2734E+05,4.2971E+05,4.3209E+05,4.3447E+05,4.3685E+05,4.3925E+05,",
        "    4.4165E+05,4.4406E+05,4.4648E+05,4.4891E+05,4.5135E+05,4.5380E+05,4.5626E+05,",
        "    4.5873E+05,4.6121E+05,4.6370E+05,4.6620E+05,4.6871E+05,4.7124E+05,4.7377E+05,",
        "    4.7632E+05,4.7888E+05,4.8145E+05,4.8404E+05,4.8663E+05,4.8924E+05,4.9186E+05,",
        "    4.9450E+05,4.9715E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.1988E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.8918E+00,4.8116E+00,4.7352E+00,",
        "    4.6621E+00,4.5920E+00,4.5247E+00,4.4599E+00,4.3974E+00,4.3370E+00,4.2787E+00,",
        "    4.2221E+00,4.1674E+00,4.1143E+00,4.0627E+00,4.0126E+00,3.9639E+00,3.9165E+00,",
        "    3.8704E+00,3.8255E+00,3.7817E+00,3.7390E+00,3.6974E+00,3.6567E+00,3.6171E+00,",
        "    3.5783E+00,3.5405E+00,3.5035E+00,3.4673E+00,3.4319E+00,3.3973E+00,3.3634E+00,",
        "    3.3302E+00,3.2977E+00,3.2659E+00,3.2347E+00,3.2041E+00,3.1742E+00,3.1448E+00,",
        "    3.1160E+00,3.0877E+00;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.3860E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.0260E+05,4.0510E+05,",
        "    4.0757E+05,4.1002E+05,4.1244E+05,4.1485E+05,4.1726E+05,4.1965E+05,4.2204E+05,",
        "    4.2444E+05,4.2683E+05,4.2922E+05,4.3162E+05,4.3402E+05,4.3642E+05,4.3883E+05,",
        "    4.4125E+05,4.4368E+05,4.4611E+05,4.4855E+05,4.5100E+05,4.5346E+05,4.5593E+05,",
        "    4.5841E+05,4.6090E+05,4.6340E+05,4.6591E+05,4.6843E+05,4.7097E+05,4.7351E+05,",
        "    4.7606E+05,4.7863E+05,4.8121E+05,4.8380E+05,4.8640E+05,4.8902E+05,4.9165E+05,",
        "    4.9428E+05,4.9694E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.3860E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,5.6098E+00,5.5173E+00,",
        "    5.4293E+00,5.3451E+00,5.2645E+00,5.1871E+00,5.1127E+00,5.0409E+00,4.9717E+00,",
        "    4.9047E+00,4.8399E+00,4.7772E+00,4.7163E+00,4.6573E+00,4.5999E+00,4.5442E+00,",
        "    4.4900E+00,4.4372E+00,4.3859E+00,4.3358E+00,4.2870E+00,4.2394E+00,4.1930E+00,",
        "    4.1476E+00,4.1033E+00,4.0601E+00,4.0178E+00,3.9765E+00,3.9360E+00,3.8965E+00,",
        "    3.8578E+00,3.8199E+00,3.7828E+00,3.7464E+00,3.7108E+00,3.6759E+00,3.6417E+00,",
        "    3.6081E+00,3.5752E+00;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.5955E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.0410E+05,",
        "    4.0664E+05,4.0915E+05,4.1163E+05,4.1409E+05,4.1654E+05,4.1898E+05,4.2140E+05,",
        "    4.2383E+05,4.2625E+05,4.2867E+05,4.3109E+05,4.3351E+05,4.3593E+05,4.3836E+05,",
        "    4.4080E+05,4.4324E+05,4.4569E+05,4.4815E+05,4.5061E+05,4.5309E+05,4.5557E+05,",
        "    4.5806E+05,4.6056E+05,4.6307E+05,4.6559E+05,4.6812E+05,4.7066E+05,4.7321E+05,",
        "    4.7578E+05,4.7835E+05,4.8094E+05,4.8354E+05,4.8615E+05,4.8877E+05,4.9140E+05,",
        "    4.9404E+05,4.9670E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.5955E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,6.4087E+00,",
        "    6.3023E+00,6.2010E+00,6.1045E+00,6.0120E+00,5.9233E+00,5.8380E+00,5.7559E+00,",
        "    5.6767E+00,5.6001E+00,5.5261E+00,5.4544E+00,5.3850E+00,5.3176E+00,5.2521E+00,",
        "    5.1885E+00,5.1267E+00,5.0666E+00,5.0080E+00,4.9509E+00,4.8953E+00,4.8411E+00,",
        "    4.7882E+00,4.7366E+00,4.6862E+00,4.6369E+00,4.5888E+00,4.5417E+00,4.4957E+00,",
        "    4.4507E+00,4.4066E+00,4.3635E+00,4.3212E+00,4.2799E+00,4.2393E+00,4.1996E+00,",
        "    4.1607E+00,4.1225E+00;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.8292E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    4.0557E+05,4.0816E+05,4.1071E+05,4.1323E+05,4.1573E+05,4.1821E+05,4.2068E+05,",
        "    4.2313E+05,4.2559E+05,4.2804E+05,4.3049E+05,4.3293E+05,4.3538E+05,4.3784E+05,",
        "    4.4029E+05,4.4275E+05,4.4522E+05,4.4769E+05,4.5017E+05,4.5266E+05,4.5516E+05,",
        "    4.5766E+05,4.6017E+05,4.6270E+05,4.6523E+05,4.6777E+05,4.7032E+05,4.7288E+05,",
        "    4.7546E+05,4.7804E+05,4.8063E+05,4.8324E+05,4.8586E+05,4.8848E+05,4.9112E+05,",
        "    4.9378E+05,4.9644E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.8292E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    7.2953E+00,7.1732E+00,7.0571E+00,6.9465E+00,6.8408E+00,6.7394E+00,6.6420E+00,",
        "    6.5482E+00,6.4578E+00,6.3706E+00,6.2862E+00,6.2046E+00,6.1255E+00,6.0488E+00,",
        "    5.9743E+00,5.9020E+00,5.8317E+00,5.7633E+00,5.6968E+00,5.6320E+00,5.5688E+00,",
        "    5.5072E+00,5.4472E+00,5.3885E+00,5.3313E+00,5.2754E+00,5.2208E+00,5.1674E+00,",
        "    5.1152E+00,5.0641E+00,5.0141E+00,4.9652E+00,4.9173E+00,4.8703E+00,4.8244E+00,",
        "    4.7793E+00,4.7352E+00;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.0888E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,4.0701E+05,4.0964E+05,4.1224E+05,4.1480E+05,4.1733E+05,4.1985E+05,",
        "    4.2235E+05,4.2485E+05,4.2733E+05,4.2981E+05,4.3229E+05,4.3477E+05,4.3724E+05,",
        "    4.3972E+05,4.4221E+05,4.4469E+05,4.4719E+05,4.4968E+05,4.5219E+05,4.5470E+05,",
        "    4.5722E+05,4.5974E+05,4.6228E+05,4.6482E+05,4.6738E+05,4.6994E+05,4.7251E+05,",
        "    4.7510E+05,4.7769E+05,4.8029E+05,4.8291E+05,4.8553E+05,4.8817E+05,4.9082E+05,",
        "    4.9348E+05,4.9615E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.0888E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,8.2759E+00,8.1361E+00,8.0034E+00,7.8770E+00,7.7563E+00,7.6407E+00,",
        "    7.5297E+00,7.4230E+00,7.3201E+00,7.2209E+00,7.1251E+00,7.0323E+00,6.9425E+00,",
        "    6.8555E+00,6.7710E+00,6.6890E+00,6.6093E+00,6.5318E+00,6.4564E+00,6.3830E+00,",
        "    6.3115E+00,6.2417E+00,6.1738E+00,6.1074E+00,6.0426E+00,5.9794E+00,5.9176E+00,",
        "    5.8572E+00,5.7981E+00,5.7404E+00,5.6839E+00,5.6286E+00,5.5744E+00,5.5214E+00,",
        "    5.4694E+00,5.4185E+00;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.3762E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,4.0842E+05,4.1110E+05,4.1374E+05,4.1634E+05,4.1892E+05,",
        "    4.2147E+05,4.2401E+05,4.2654E+05,4.2905E+05,4.3157E+05,4.3407E+05,4.3658E+05,",
        "    4.3909E+05,4.4159E+05,4.4410E+05,4.4662E+05,4.4914E+05,4.5166E+05,4.5419E+05,",
        "    4.5672E+05,4.5927E+05,4.6182E+05,4.6437E+05,4.6694E+05,4.6952E+05,4.7210E+05,",
        "    4.7470E+05,4.7730E+05,4.7992E+05,4.8254E+05,4.8517E+05,4.8782E+05,4.9048E+05,",
        "    4.9315E+05,4.9582E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.3762E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,9.3578E+00,9.1979E+00,9.0465E+00,8.9024E+00,8.7650E+00,",
        "    8.6335E+00,8.5073E+00,8.3861E+00,8.2694E+00,8.1569E+00,8.0482E+00,7.9431E+00,",
        "    7.8414E+00,7.7429E+00,7.6473E+00,7.5546E+00,7.4645E+00,7.3769E+00,7.2917E+00,",
        "    7.2088E+00,7.1280E+00,7.0493E+00,6.9726E+00,6.8977E+00,6.8246E+00,6.7533E+00,",
        "    6.6836E+00,6.6155E+00,6.5489E+00,6.4838E+00,6.4200E+00,6.3577E+00,6.2967E+00,",
        "    6.2369E+00,6.1783E+00;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.6935E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,4.0980E+05,4.1253E+05,4.1521E+05,4.1786E+05,",
        "    4.2047E+05,4.2307E+05,4.2564E+05,4.2820E+05,4.3075E+05,4.3330E+05,4.3584E+05,",
        "    4.3837E+05,4.4091E+05,4.4345E+05,4.4599E+05,4.4853E+05,4.5107E+05,4.5362E+05,",
        "    4.5617E+05,4.5873E+05,4.6130E+05,4.6388E+05,4.6646E+05,4.6905E+05,4.7165E+05,",
        "    4.7425E+05,4.7687E+05,4.7950E+05,4.8213E+05,4.8478E+05,4.8743E+05,4.9010E+05,",
        "    4.9278E+05,4.9546E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.6935E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,1.0549E+01,1.0367E+01,1.0194E+01,1.0030E+01,",
        "    9.8741E+00,9.7248E+00,9.5817E+00,9.4443E+00,9.3122E+00,9.1848E+00,9.0619E+00,",
        "    8.9431E+00,8.8282E+00,8.7170E+00,8.6091E+00,8.5045E+00,8.4029E+00,8.3042E+00,",
        "    8.2081E+00,8.1147E+00,8.0237E+00,7.9351E+00,7.8487E+00,7.7644E+00,7.6822E+00,",
        "    7.6019E+00,7.5235E+00,7.4469E+00,7.3721E+00,7.2989E+00,7.2273E+00,7.1572E+00,",
        "    7.0886E+00,7.0214E+00;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.0426E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.1114E+05,4.1392E+05,4.1665E+05,",
        "    4.1934E+05,4.2200E+05,4.2463E+05,4.2725E+05,4.2984E+05,4.3243E+05,4.3501E+05,",
        "    4.3758E+05,4.4015E+05,4.4272E+05,4.4528E+05,4.4785E+05,4.5042E+05,4.5299E+05,",
        "    4.5556E+05,4.5814E+05,4.6073E+05,4.6332E+05,4.6592E+05,4.6853E+05,4.7114E+05,",
        "    4.7376E+05,4.7639E+05,4.7903E+05,4.8168E+05,4.8434E+05,4.8701E+05,4.8968E+05,",
        "    4.9237E+05,4.9507E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.0426E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,1.1857E+01,1.1650E+01,1.1453E+01,",
        "    1.1267E+01,1.1090E+01,1.0921E+01,1.0759E+01,1.0604E+01,1.0454E+01,1.0310E+01,",
        "    1.0172E+01,1.0038E+01,9.9083E+00,9.7830E+00,9.6615E+00,9.5438E+00,9.4294E+00,",
        "    9.3184E+00,9.2104E+00,9.1054E+00,9.0032E+00,8.9037E+00,8.8067E+00,8.7121E+00,",
        "    8.6198E+00,8.5297E+00,8.4418E+00,8.3559E+00,8.2719E+00,8.1898E+00,8.1095E+00,",
        "    8.0310E+00,7.9541E+00;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.4257E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.1245E+05,4.1529E+05,",
        "    4.1807E+05,4.2080E+05,4.2350E+05,4.2617E+05,4.2883E+05,4.3146E+05,4.3408E+05,",
        "    4.3670E+05,4.3930E+05,4.4190E+05,4.4450E+05,4.4709E+05,4.4969E+05,4.5229E+05,",
        "    4.5489E+05,4.5749E+05,4.6010E+05,4.6271E+05,4.6533E+05,4.6795E+05,4.7058E+05,",
        "    4.7322E+05,4.7587E+05,4.7852E+05,4.8118E+05,4.8385E+05,4.8653E+05,4.8922E+05,",
        "    4.9192E+05,4.9463E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.4257E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,1.3292E+01,1.3056E+01,",
        "    1.2833E+01,1.2622E+01,1.2421E+01,1.2230E+01,1.2047E+01,1.1871E+01,1.1703E+01,",
        "    1.1541E+01,1.1385E+01,1.1234E+01,1.1088E+01,1.0947E+01,1.0811E+01,1.0678E+01,",
        "    1.0550E+01,1.0425E+01,1.0304E+01,1.0187E+01,1.0072E+01,9.9605E+00,9.8518E+00,",
        "    9.7459E+00,9.6426E+00,9.5417E+00,9.4433E+00,9.3472E+00,9.2533E+00,9.1615E+00,",
        "    9.0717E+00,8.9839E+00;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.8449E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.1373E+05,",
        "    4.1661E+05,4.1944E+05,4.2222E+05,4.2497E+05,4.2768E+05,4.3038E+05,4.3305E+05,",
        "    4.3571E+05,4.3836E+05,4.4100E+05,4.4363E+05,4.4626E+05,4.4889E+05,4.5151E+05,",
        "    4.5414E+05,4.5677E+05,4.5940E+05,4.6203E+05,4.6467E+05,4.6732E+05,4.6997E+05,",
        "    4.7262E+05,4.7529E+05,4.7796E+05,4.8063E+05,4.8332E+05,4.8601E+05,4.8872E+05,",
        "    4.9143E+05,4.9415E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.8449E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,1.4861E+01,",
        "    1.4593E+01,1.4341E+01,1.4102E+01,1.3875E+01,1.3659E+01,1.3452E+01,1.3255E+01,",
        "    1.3065E+01,1.2882E+01,1.2707E+01,1.2537E+01,1.2374E+01,1.2216E+01,1.2063E+01,",
        "    1.1914E+01,1.1771E+01,1.1631E+01,1.1495E+01,1.1364E+01,1.1236E+01,1.1111E+01,",
        "    1.0989E+01,1.0871E+01,1.0755E+01,1.0643E+01,1.0533E+01,1.0426E+01,1.0321E+01,",
        "    1.0218E+01,1.0118E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    4.3024E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    4.1496E+05,4.1790E+05,4.2078E+05,4.2361E+05,4.2641E+05,4.2916E+05,4.3190E+05,",
        "    4.3461E+05,4.3731E+05,4.3999E+05,4.4267E+05,4.4533E+05,4.4800E+05,4.5066E+05,",
        "    4.5331E+05,4.5597E+05,4.5863E+05,4.6129E+05,4.6395E+05,4.6662E+05,4.6929E+05,",
        "    4.7197E+05,4.7465E+05,4.7734E+05,4.8003E+05,4.8273E+05,4.8544E+05,4.8816E+05,",
        "    4.9089E+05,4.9362E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    4.3024E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    1.6576E+01,1.6272E+01,1.5986E+01,1.5716E+01,1.5460E+01,1.5216E+01,1.4983E+01,",
        "    1.4761E+01,1.4547E+01,1.4343E+01,1.4145E+01,1.3955E+01,1.3772E+01,1.3595E+01,",
        "    1.3424E+01,1.3258E+01,1.3097E+01,1.2941E+01,1.2789E+01,1.2642E+01,1.2499E+01,",
        "    1.2360E+01,1.2224E+01,1.2092E+01,1.1964E+01,1.1838E+01,1.1716E+01,1.1596E+01,",
        "    1.1480E+01,1.1365E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    4.8004E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,4.1615E+05,4.1915E+05,4.2209E+05,4.2497E+05,4.2781E+05,4.3061E+05,",
        "    4.3339E+05,4.3614E+05,4.3888E+05,4.4160E+05,4.4431E+05,4.4701E+05,4.4971E+05,",
        "    4.5240E+05,4.5509E+05,4.5778E+05,4.6047E+05,4.6316E+05,4.6585E+05,4.6855E+05,",
        "    4.7124E+05,4.7395E+05,4.7666E+05,4.7937E+05,4.8209E+05,4.8482E+05,4.8755E+05,",
        "    4.9029E+05,4.9304E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    4.8004E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,1.8447E+01,1.8102E+01,1.7778E+01,1.7473E+01,1.7184E+01,1.6910E+01,",
        "    1.6648E+01,1.6398E+01,1.6158E+01,1.5928E+01,1.5707E+01,1.5495E+01,1.5289E+01,",
        "    1.5091E+01,1.4900E+01,1.4715E+01,1.4535E+01,1.4361E+01,1.4192E+01,1.4028E+01,",
        "    1.3869E+01,1.3714E+01,1.3563E+01,1.3416E+01,1.3273E+01,1.3133E+01,1.2997E+01,",
        "    1.2864E+01,1.2734E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    5.3412E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,4.1730E+05,4.2036E+05,4.2335E+05,4.2629E+05,4.2917E+05,",
        "    4.3202E+05,4.3485E+05,4.3764E+05,4.4042E+05,4.4318E+05,4.4593E+05,4.4867E+05,",
        "    4.5140E+05,4.5413E+05,4.5685E+05,4.5957E+05,4.6229E+05,4.6501E+05,4.6773E+05,",
        "    4.7045E+05,4.7318E+05,4.7591E+05,4.7865E+05,4.8139E+05,4.8413E+05,4.8689E+05,",
        "    4.8965E+05,4.9241E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    5.3412E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,2.0485E+01,2.0094E+01,1.9728E+01,1.9383E+01,1.9058E+01,",
        "    1.8749E+01,1.8455E+01,1.8174E+01,1.7905E+01,1.7648E+01,1.7400E+01,1.7162E+01,",
        "    1.6933E+01,1.6712E+01,1.6498E+01,1.6292E+01,1.6092E+01,1.5898E+01,1.5710E+01,",
        "    1.5527E+01,1.5350E+01,1.5178E+01,1.5010E+01,1.4847E+01,1.4688E+01,1.4533E+01,",
        "    1.4382E+01,1.4234E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    5.9273E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,4.1840E+05,4.2153E+05,4.2458E+05,4.2756E+05,",
        "    4.3050E+05,4.3340E+05,4.3627E+05,4.3911E+05,4.4193E+05,4.4473E+05,4.4752E+05,",
        "    4.5029E+05,4.5306E+05,4.5582E+05,4.5858E+05,4.6133E+05,4.6408E+05,4.6683E+05,",
        "    4.6959E+05,4.7234E+05,4.7509E+05,4.7785E+05,4.8062E+05,4.8338E+05,4.8616E+05,",
        "    4.8894E+05,4.9172E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    5.9273E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,2.2703E+01,2.2260E+01,2.1846E+01,2.1458E+01,",
        "    2.1091E+01,2.0744E+01,2.0413E+01,2.0098E+01,1.9798E+01,1.9509E+01,1.9233E+01,",
        "    1.8967E+01,1.8711E+01,1.8465E+01,1.8227E+01,1.7996E+01,1.7774E+01,1.7558E+01,",
        "    1.7349E+01,1.7146E+01,1.6950E+01,1.6758E+01,1.6572E+01,1.6391E+01,1.6215E+01,",
        "    1.6043E+01,1.5876E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    6.5609E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.1945E+05,4.2264E+05,4.2575E+05,",
        "    4.2880E+05,4.3179E+05,4.3474E+05,4.3765E+05,4.4054E+05,4.4340E+05,4.4625E+05,",
        "    4.4907E+05,4.5189E+05,4.5469E+05,4.5749E+05,4.6028E+05,4.6307E+05,4.6585E+05,",
        "    4.6864E+05,4.7142E+05,4.7420E+05,4.7699E+05,4.7978E+05,4.8257E+05,4.8536E+05,",
        "    4.8816E+05,4.9097E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    6.5609E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,2.5113E+01,2.4612E+01,2.4145E+01,",
        "    2.3707E+01,2.3294E+01,2.2904E+01,2.2533E+01,2.2180E+01,2.1844E+01,2.1522E+01,",
        "    2.1213E+01,2.0916E+01,2.0631E+01,2.0356E+01,2.0091E+01,1.9836E+01,1.9588E+01,",
        "    1.9349E+01,1.9117E+01,1.8892E+01,1.8673E+01,1.8461E+01,1.8255E+01,1.8055E+01,",
        "    1.7860E+01,1.7670E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    7.2446E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.2045E+05,4.2371E+05,",
        "    4.2688E+05,4.2999E+05,4.3304E+05,4.3604E+05,4.3900E+05,4.4194E+05,4.4484E+05,",
        "    4.4773E+05,4.5060E+05,4.5345E+05,4.5630E+05,4.5913E+05,4.6196E+05,4.6478E+05,",
        "    4.6760E+05,4.7041E+05,4.7323E+05,4.7604E+05,4.7886E+05,4.8168E+05,4.8450E+05,",
        "    4.8732E+05,4.9015E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    7.2446E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,2.7732E+01,2.7164E+01,",
        "    2.6636E+01,2.6143E+01,2.5678E+01,2.5240E+01,2.4825E+01,2.4430E+01,2.4053E+01,",
        "    2.3694E+01,2.3349E+01,2.3019E+01,2.2701E+01,2.2396E+01,2.2101E+01,2.1817E+01,",
        "    2.1542E+01,2.1277E+01,2.1019E+01,2.0770E+01,2.0529E+01,2.0294E+01,2.0066E+01,",
        "    1.9844E+01,1.9629E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    7.9808E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.2139E+05,",
        "    4.2472E+05,4.2797E+05,4.3113E+05,4.3424E+05,4.3730E+05,4.4031E+05,4.4329E+05,",
        "    4.4625E+05,4.4918E+05,4.5209E+05,4.5499E+05,4.5787E+05,4.6074E+05,4.6361E+05,",
        "    4.6646E+05,4.6932E+05,4.7217E+05,4.7502E+05,4.7786E+05,4.8071E+05,4.8356E+05,",
        "    4.8641E+05,4.8926E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    7.9808E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,3.0574E+01,",
        "    2.9931E+01,2.9335E+01,2.8779E+01,2.8257E+01,2.7765E+01,2.7299E+01,2.6857E+01,",
        "    2.6436E+01,2.6035E+01,2.5651E+01,2.5283E+01,2.4930E+01,2.4590E+01,2.4263E+01,",
        "    2.3948E+01,2.3644E+01,2.3349E+01,2.3065E+01,2.2789E+01,2.2521E+01,2.2262E+01,",
        "    2.2010E+01,2.1766E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    8.7722E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    4.2227E+05,4.2568E+05,4.2899E+05,4.3223E+05,4.3540E+05,4.3851E+05,4.4158E+05,",
        "    4.4461E+05,4.4761E+05,4.5059E+05,4.5355E+05,4.5649E+05,4.5941E+05,4.6232E+05,",
        "    4.6523E+05,4.6812E+05,4.7101E+05,4.7389E+05,4.7678E+05,4.7966E+05,4.8254E+05,",
        "    4.8542E+05,4.8830E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    8.7722E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    3.3659E+01,3.2930E+01,3.2256E+01,3.1629E+01,3.1042E+01,3.0490E+01,2.9968E+01,",
        "    2.9474E+01,2.9004E+01,2.8557E+01,2.8129E+01,2.7720E+01,2.7327E+01,2.6950E+01,",
        "    2.6587E+01,2.6238E+01,2.5900E+01,2.5575E+01,2.5260E+01,2.4955E+01,2.4660E+01,",
        "    2.4374E+01,2.4096E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    9.6214E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,4.2308E+05,4.2658E+05,4.2997E+05,4.3327E+05,4.3650E+05,4.3968E+05,",
        "    4.4280E+05,4.4589E+05,4.4894E+05,4.5197E+05,4.5497E+05,4.5795E+05,4.6092E+05,",
        "    4.6387E+05,4.6681E+05,4.6975E+05,4.7267E+05,4.7559E+05,4.7851E+05,4.8142E+05,",
        "    4.8434E+05,4.8725E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    9.6214E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,3.7005E+01,3.6178E+01,3.5416E+01,3.4709E+01,3.4049E+01,3.3429E+01,",
        "    3.2845E+01,3.2292E+01,3.1768E+01,3.1269E+01,3.0793E+01,3.0338E+01,2.9902E+01,",
        "    2.9484E+01,2.9082E+01,2.8694E+01,2.8321E+01,2.7961E+01,2.7613E+01,2.7277E+01,",
        "    2.6951E+01,2.6636E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.0531E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,4.2382E+05,4.2741E+05,4.3088E+05,4.3426E+05,4.3756E+05,",
        "    4.4079E+05,4.4398E+05,4.4712E+05,4.5023E+05,4.5330E+05,4.5635E+05,4.5938E+05,",
        "    4.6239E+05,4.6539E+05,4.6837E+05,4.7134E+05,4.7430E+05,4.7726E+05,4.8021E+05,",
        "    4.8316E+05,4.8611E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.0531E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,4.0634E+01,3.9695E+01,3.8832E+01,3.8035E+01,3.7292E+01,",
        "    3.6597E+01,3.5943E+01,3.5325E+01,3.4740E+01,3.4184E+01,3.3654E+01,3.3149E+01,",
        "    3.2665E+01,3.2201E+01,3.1755E+01,3.1327E+01,3.0915E+01,3.0517E+01,3.0133E+01,",
        "    2.9762E+01,2.9403E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.1504E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,4.2448E+05,4.2817E+05,4.3173E+05,4.3518E+05,",
        "    4.3855E+05,4.4186E+05,4.4511E+05,4.4831E+05,4.5147E+05,4.5459E+05,4.5769E+05,",
        "    4.6077E+05,4.6383E+05,4.6686E+05,4.6989E+05,4.7290E+05,4.7591E+05,4.7890E+05,",
        "    4.8189E+05,4.8488E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.1504E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,4.4572E+01,4.3503E+01,4.2527E+01,4.1626E+01,",
        "    4.0790E+01,4.0010E+01,3.9277E+01,3.8587E+01,3.7934E+01,3.7314E+01,3.6725E+01,",
        "    3.6164E+01,3.5627E+01,3.5113E+01,3.4620E+01,3.4146E+01,3.3691E+01,3.3252E+01,",
        "    3.2828E+01,3.2419E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.2543E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.2507E+05,4.2886E+05,4.3251E+05,",
        "    4.3605E+05,4.3949E+05,4.4287E+05,4.4618E+05,4.4944E+05,4.5266E+05,4.5584E+05,",
        "    4.5899E+05,4.6212E+05,4.6522E+05,4.6831E+05,4.7138E+05,4.7443E+05,4.7748E+05,",
        "    4.8051E+05,4.8354E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.2543E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.8844E+01,4.7627E+01,4.6519E+01,",
        "    4.5502E+01,4.4560E+01,4.3684E+01,4.2863E+01,4.2091E+01,4.1363E+01,4.0673E+01,",
        "    4.0018E+01,3.9394E+01,3.8799E+01,3.8230E+01,3.7685E+01,3.7161E+01,3.6658E+01,",
        "    3.6174E+01,3.5708E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.3651E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.2556E+05,4.2947E+05,",
        "    4.3322E+05,4.3684E+05,4.4037E+05,4.4382E+05,4.4720E+05,4.5053E+05,4.5381E+05,",
        "    4.5705E+05,4.6025E+05,4.6343E+05,4.6658E+05,4.6971E+05,4.7283E+05,4.7592E+05,",
        "    4.7901E+05,4.8209E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.3651E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,5.3484E+01,5.2094E+01,",
        "    5.0835E+01,4.9684E+01,4.8623E+01,4.7638E+01,4.6718E+01,4.5855E+01,4.5042E+01,",
        "    4.4274E+01,4.3546E+01,4.2854E+01,4.2194E+01,4.1564E+01,4.0961E+01,4.0383E+01,",
        "    3.9828E+01,3.9294E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.4831E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.2595E+05,",
        "    4.2999E+05,4.3385E+05,4.3757E+05,4.4119E+05,4.4471E+05,4.4817E+05,4.5156E+05,",
        "    4.5490E+05,4.5820E+05,4.6146E+05,4.6469E+05,4.6790E+05,4.7108E+05,4.7424E+05,",
        "    4.7738E+05,4.8051E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.4831E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,5.8526E+01,",
        "    5.6935E+01,5.5502E+01,5.4199E+01,5.3001E+01,5.1893E+01,5.0861E+01,4.9896E+01,",
        "    4.8989E+01,4.8133E+01,4.7324E+01,4.6555E+01,4.5824E+01,4.5127E+01,4.4461E+01,",
        "    4.3823E+01,4.3211E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.6086E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    4.2624E+05,4.3041E+05,4.3439E+05,4.3822E+05,4.4193E+05,4.4554E+05,4.4907E+05,",
        "    4.5254E+05,4.5595E+05,4.5931E+05,4.6263E+05,4.6591E+05,4.6917E+05,4.7240E+05,",
        "    4.7561E+05,4.7880E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.6086E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    6.4013E+01,6.2185E+01,6.0551E+01,5.9071E+01,5.7718E+01,5.6470E+01,5.5313E+01,",
        "    5.4232E+01,5.3220E+01,5.2267E+01,5.1367E+01,5.0514E+01,4.9704E+01,4.8933E+01,",
        "    4.8196E+01,4.7492E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.7419E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,4.2642E+05,4.3074E+05,4.3485E+05,4.3879E+05,4.4260E+05,4.4630E+05,",
        "    4.4991E+05,4.5345E+05,4.5693E+05,4.6036E+05,4.6374E+05,4.6709E+05,4.7040E+05,",
        "    4.7368E+05,4.7694E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.7419E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,6.9990E+01,6.7883E+01,6.6014E+01,6.4331E+01,6.2800E+01,6.1394E+01,",
        "    6.0094E+01,5.8884E+01,5.7753E+01,5.6691E+01,5.5691E+01,5.4744E+01,5.3847E+01,",
        "    5.2994E+01,5.2181E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.8834E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,4.2646E+05,4.3096E+05,4.3521E+05,4.3927E+05,4.4319E+05,",
        "    4.4699E+05,4.5069E+05,4.5431E+05,4.5786E+05,4.6136E+05,4.6481E+05,4.6821E+05,",
        "    4.7158E+05,4.7492E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.8834E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,7.6519E+01,7.4080E+01,7.1935E+01,7.0017E+01,6.8281E+01,",
        "    6.6694E+01,6.5232E+01,6.3876E+01,6.2613E+01,6.1429E+01,6.0316E+01,5.9266E+01,",
        "    5.8272E+01,5.7328E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.0334E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,4.2635E+05,4.3105E+05,4.3546E+05,4.3966E+05,",
        "    4.4369E+05,4.4760E+05,4.5139E+05,4.5510E+05,4.5873E+05,4.6230E+05,4.6582E+05,",
        "    4.6929E+05,4.7272E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.0334E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,8.3665E+01,8.0827E+01,7.8356E+01,7.6164E+01,",
        "    7.4192E+01,7.2398E+01,7.0753E+01,6.9232E+01,6.7819E+01,6.6499E+01,6.5261E+01,",
        "    6.4095E+01,6.2993E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.1923E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.2609E+05,4.3101E+05,4.3560E+05,",
        "    4.3995E+05,4.4411E+05,4.4812E+05,4.5202E+05,4.5582E+05,4.5953E+05,4.6318E+05,",
        "    4.6677E+05,4.7030E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.1923E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,9.1511E+01,8.8190E+01,8.5332E+01,",
        "    8.2819E+01,8.0573E+01,7.8542E+01,7.6687E+01,7.4980E+01,7.3398E+01,7.1925E+01,",
        "    7.0547E+01,6.9252E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.3604E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.2564E+05,4.3082E+05,",
        "    4.3561E+05,4.4013E+05,4.4443E+05,4.4856E+05,4.5257E+05,4.5646E+05,4.6027E+05,",
        "    4.6400E+05,4.6766E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.3604E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,1.0015E+02,9.6239E+01,",
        "    9.2918E+01,9.0027E+01,8.7463E+01,8.5159E+01,8.3065E+01,8.1145E+01,7.9374E+01,",
        "    7.7729E+01,7.6195E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.5382E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.2498E+05,",
        "    4.3047E+05,4.3549E+05,4.4019E+05,4.4464E+05,4.4891E+05,4.5303E+05,4.5703E+05,",
        "    4.6093E+05,4.6475E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.5382E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,1.0972E+02,",
        "    1.0507E+02,1.0119E+02,9.7851E+01,9.4915E+01,9.2295E+01,8.9926E+01,8.7766E+01,",
        "    8.5780E+01,8.3942E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.7261E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    4.2408E+05,4.2993E+05,4.3522E+05,4.4012E+05,4.4475E+05,4.4916E+05,4.5341E+05,",
        "    4.5752E+05,4.6152E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.7261E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    1.2038E+02,1.1479E+02,1.1023E+02,1.0635E+02,1.0298E+02,9.9995E+01,9.7312E+01,",
        "    9.4877E+01,9.2647E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.9246E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,4.2290E+05,4.2918E+05,4.3478E+05,4.3992E+05,4.4473E+05,4.4931E+05,",
        "    4.5369E+05,4.5792E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.9246E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,1.3233E+02,1.2554E+02,1.2013E+02,1.1562E+02,1.1173E+02,1.0831E+02,",
        "    1.0527E+02,1.0252E+02;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.1341E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,4.2137E+05,4.2820E+05,4.3416E+05,4.3957E+05,4.4459E+05,",
        "    4.4934E+05,4.5387E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.1341E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,1.4585E+02,1.3748E+02,1.3102E+02,1.2572E+02,1.2122E+02,",
        "    1.1731E+02,1.1384E+02;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.3552E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,4.1941E+05,4.2695E+05,4.3334E+05,4.3905E+05,",
        "    4.4432E+05,4.4926E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.3552E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,1.6135E+02,1.5082E+02,1.4302E+02,1.3677E+02,",
        "    1.3154E+02,1.2704E+02;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.5886E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.1692E+05,4.2539E+05,4.3229E+05,",
        "    4.3836E+05,4.4389E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.5886E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,1.7941E+02,1.6585E+02,1.5632E+02,",
        "    1.4890E+02,1.4279E+02;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.8348E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.1370E+05,4.2346E+05,",
        "    4.3100E+05,4.3748E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.8348E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,2.0095E+02,1.8289E+02,",
        "    1.7111E+02,1.6223E+02;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    4.0949E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.0942E+05,",
        "    4.2109E+05,4.2943E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    4.0949E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,2.2766E+02,",
        "    2.0246E+02,1.8765E+02;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    4.3697E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    4.0343E+05,4.1823E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    4.3697E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    2.6302E+02,2.2513E+02;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    4.6607E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,3.9373E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    4.6607E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,3.1758E+02;",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);

    GetVRFInput(*state);
    state->dataHVACVarRefFlow->GetVRFInputFlag = false;

    // get input test for terminal air single duct mixer on inlet side of VRF terminal unit
    ASSERT_EQ(1, state->dataSingleDuct->NumATMixers);
    EXPECT_EQ("SPACE1-1 DOAS AIR TERMINAL", state->dataSingleDuct->SysATMixer(1).Name);                 // single duct air terminal mixer name
    EXPECT_EQ((int)HVAC::MixerType::InletSide, (int)state->dataSingleDuct->SysATMixer(1).type);         // air terminal mixer connection type
    EXPECT_EQ("AIRTERMINAL:SINGLEDUCT:MIXER", state->dataDefineEquipment->AirDistUnit(1).EquipType(1)); // Air distribution unit equipment type
    EXPECT_EQ("TU1", state->dataHVACVarRefFlow->VRFTU(1).Name);                                         // zoneHVAC equipment name

    state->dataGlobal->BeginEnvrnFlag = false;

    // set input variables
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutDryBulbTemp = 35.0;
    state->dataEnvrn->OutHumRat = 0.0098;
    state->dataEnvrn->OutEnthalpy = Psychrometrics::PsyHFnTdbW(state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutHumRat);
    state->dataEnvrn->OutWetBulbTemp =
        PsyTwbFnTdbWPb(*state, state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutHumRat, state->dataEnvrn->OutBaroPress);
    state->dataEnvrn->StdRhoAir = 1.20;
    HVACInletMassFlowRate = 0.50;
    PrimaryAirMassFlowRate = 0.1;
    SecondaryAirMassFlowRate = HVACInletMassFlowRate - PrimaryAirMassFlowRate; // secondary air flow is VRFTU flow less primary air flow

    // set zoneNode air condition
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp = 24.0;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat = 0.0075;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp,
                                   state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat);

    state->dataHVACVarRefFlow->CoolingLoad.allocate(1);
    state->dataHVACVarRefFlow->HeatingLoad.allocate(1);
    state->dataHVACVarRefFlow->HeatingLoad(1) = false;
    state->dataHVACVarRefFlow->CoolingLoad(1) = true;
    state->dataHVACVarRefFlow->CompOnMassFlow = HVACInletMassFlowRate;    // supply air mass flow rate
    state->dataHVACVarRefFlow->OACompOnMassFlow = PrimaryAirMassFlowRate; // OA mass flow rate
    state->dataHVACVarRefFlow->CompOnFlowRatio = 1.0;                     // compressor is on
    state->dataHVACGlobal->TurnFansOff = false;
    state->dataHVACGlobal->TurnFansOn = true;

    VRFNum = 1;
    VRFTUNum = 1;
    state->dataHVACVarRefFlow->VRFTU(VRFTUNum).fanOp = HVAC::FanOp::Cycling;
    state->dataHVACVarRefFlow->VRFTU(VRFTUNum).isInZone = true;
    state->dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneAirNode = state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode;
    // initialize mass flow rates
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUInletNodeNum).MassFlowRate = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerPriNode).MassFlowRate = PrimaryAirMassFlowRate;
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerPriNode).MassFlowRateMaxAvail = PrimaryAirMassFlowRate;

    // set fan parameters
    state->dataFans->fans(1)->maxAirMassFlowRate = HVACInletMassFlowRate;
    state->dataFans->fans(1)->inletAirMassFlowRate = HVACInletMassFlowRate;
    state->dataFans->fans(1)->rhoAirStdInit = state->dataEnvrn->StdRhoAir;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->inletNodeNum).MassFlowRateMaxAvail = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->outletNodeNum).MassFlowRateMax = HVACInletMassFlowRate;

    // set DX coil rated performance parameters
    state->dataDXCoils->DXCoil(1).RatedCBF(1) = 0.05;
    state->dataDXCoils->DXCoil(1).RatedAirMassFlowRate(1) = HVACInletMassFlowRate;

    // primary air condition set at outdoor air condition
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerPriNode).Temp = state->dataEnvrn->OutDryBulbTemp;
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerPriNode).HumRat = state->dataEnvrn->OutHumRat;
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerPriNode).Enthalpy = state->dataEnvrn->OutEnthalpy;

    // set secondary air (recirculating air) conditions to zone air node
    state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).Temp =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp;
    state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).HumRat =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat;
    state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).Enthalpy =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy;

    state->dataHVACVarRefFlow->VRFTU(1).ZoneNum = 1;
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataGlobal->SysSizingCalc = true;

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -5000.0;
    QZnReq = state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP;

    state->dataHVACVarRefFlow->VRFTU(VRFTUNum).availSched->currentVal = 1.0;    // unit is always available
    state->dataHVACVarRefFlow->VRFTU(VRFTUNum).fanAvailSched->currentVal = 1.0; // fan is always available

    // set secondary air mass flow rate to zero
    state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).MassFlowRate = 0.0;
    state->dataFans->fans(1)->set_size(*state); // add sizing
    state->dataHVACVarRefFlow->VRF(1).VRFCondCyclingRatio = 1.0;
    // Simulate zoneHVAC equipment (VRF terminal unit)
    SimVRF(*state, VRFTUNum, FirstHVACIteration, OnOffAirFlowRatio, QUnitOutVRFTU, LatOutputProvided, QZnReq);

    // check the terminal air mixer secondary air mass flow rate, requires updating the secondary flow
    SecondaryAirMassFlowRate =
        state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUInletNodeNum).MassFlowRate - PrimaryAirMassFlowRate;
    ASSERT_EQ(SecondaryAirMassFlowRate, state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).MassFlowRate);
    // check the terminal air mixer outlet flow rate must be equal to VRFTU mass flow rate
    HVACInletMassFlowRate = state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUInletNodeNum).MassFlowRate;
    ASSERT_EQ(HVACInletMassFlowRate, state->dataSingleDuct->SysATMixer(1).MixedAirMassFlowRate);
    // check the cooling output delivered is within 5.0 Watt of zone cooling load
    ASSERT_NEAR(QZnReq, QUnitOutVRFTU, 5.0);
    EXPECT_NEAR(0.965, state->dataDXCoils->DXCoil(1).PartLoadRatio, 0.001);
}

TEST_F(EnergyPlusFixture, AirTerminalSingleDuctMixer_SimVRFfluidCntrl_ATMSupplySide)
{

    bool ErrorsFound(false);
    bool FirstHVACIteration(false);
    Real64 HVACInletMassFlowRate(0.0);
    Real64 PrimaryAirMassFlowRate(0.0);
    Real64 SecondaryAirMassFlowRate(0.0);
    Real64 ATMixerOutletMassFlowRate(0.0);
    Real64 OnOffAirFlowRatio(1.0);
    Real64 LatOutputProvided(0.0);
    Real64 QUnitOutVRFTU(0.0);
    Real64 QZnReq(0.0);
    int VRFNum(1);
    int VRFTUNum(1);

    std::string const idf_objects = delimited_string({

        "AirTerminal:SingleDuct:Mixer,",
        "    SPACE1-1 DOAS Air Terminal,  !- Name",
        "    ZoneHVAC:TerminalUnit:VariableRefrigerantFlow,  !- ZoneHVAC Terminal Unit Object Type",
        "    TU1,                         !- ZoneHVAC Terminal Unit Name",
        "    TU1 Outlet Node,             !- Terminal Unit Outlet Node Name",
        "    SPACE1-1 Air Terminal Mixer Primary Inlet,    !- Terminal Unit Primary Air Inlet Node Name",
        "    SPACE1-1 Air Terminal Mixer Secondary Inlet,  !- Terminal Unit Secondary Air Inlet Node Name",
        "    SupplySide;                                   !- Terminal Unit Connection Type",

        "ZoneHVAC:AirDistributionUnit,",
        "    SPACE1-1 DOAS ATU,       !- Name",
        "    TU1 Outlet Node,         !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:Mixer,  !- Air Terminal Object Type",
        "    SPACE1-1 DOAS Air Terminal;  !- Air Terminal Name",

        "Schedule:Compact,",
        "    FanAvailSched,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    1.0;                     !- Field 4",

        "  Schedule:Compact,",
        "    VRFAvailSched,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    1.0;                     !- Field 4",

        "Schedule:Compact,",
        "    CyclingFanSch,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    0.0;                     !- Field 4",

        "ZoneHVAC:EquipmentList,",
        "    SPACE1-1 Eq,             !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE1-1 DOAS ATU,       !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction",
        "    ,                        !- Zone Equipment 1 Sequential Heating Fraction",
        "    ZoneHVAC:TerminalUnit:VariableRefrigerantFlow,  !- Zone Equipment 2 Object Type",
        "    TU1,                     !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    2,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 2 Sequential Cooling Fraction",
        "    ;                        !- Zone Equipment 2 Sequential Heating Fraction",

        "  ZoneHVAC:TerminalUnit:VariableRefrigerantFlow,",
        "    TU1,                     !- Zone Terminal Unit Name",
        "    VRFAvailSched,           !- Terminal Unit Availability Schedule",
        "    zTU1 Inlet Node,         !- Terminal Unit Air Inlet Node Name",
        "    SPACE1-1 Air Terminal Mixer Secondary Inlet, !- Terminal Unit Air Outlet Node Name",
        "    0.500,                   !- Supply Air Flow Rate During Cooling Operation {m3/s}",
        "    0.500,                   !- Supply Air Flow Rate When No Cooling is Needed {m3/s}",
        "    0.500,                   !- Supply Air Flow Rate During Heating Operation {m3/s}",
        "    0.500,                   !- Supply Air Flow Rate When No Heating is Needed {m3/s}",
        "    0,                       !- Outdoor Air Flow Rate During Cooling Operation {m3/s}",
        "    0,                       !- Outdoor Air Flow Rate During Heating Operation {m3/s}",
        "    0,                       !- Outdoor Air Flow Rate When No Cooling or Heating is Needed {m3/s}",
        "    VRFFanSchedule,          !- Supply Air Fan Operating Mode Schedule Name",
        "    drawthrough,             !- Supply Air Fan Placement",
        "    Fan:SystemModel,      !- Supply Air Fan Object Type",
        "    TU1 VRF Supply Fan,      !- Supply Air Fan Object Name",
        "    ,                        !- Outside Air Mixer Object Type",
        "    ,                        !- Outside Air Mixer Object Name",
        "    Coil:Cooling:DX:VariableRefrigerantFlow:FluidTemperatureControl,  !- Cooling Coil Object Type",
        "    TU1 VRF DX Cooling Coil, !- Cooling Coil Object Name",
        "    COIL:HEATING:DX:VARIABLEREFRIGERANTFLOW:FluidTemperatureControl,  !- Heating Coil Object Type",
        "    TU1 VRF DX Heating Coil, !- Heating Coil Object Name",
        "    30,                      !- Zone Terminal Unit On Parasitic Electric Energy Use {W}",
        "    20;                      !- Zone Terminal Unit Off Parasitic Electric Energy Use {W}",

        "  Fan:SystemModel,",
        "    TU1 VRF Supply Fan,      !- Name",
        "    VRFAvailSched,           !- Availability Schedule Name",
        "    TU1 VRF DX HCoil Outlet Node,  !- Air Inlet Node Name",
        "    SPACE1-1 AIR TERMINAL MIXER SECONDARY INLET, !- Air Outlet Node Name",
        "    0.500,                   !- Design Maximum Air Flow Rate {m3/s}",
        "    Continuous,              !- Speed Control Method",
        "    0.0,                   !- Electric Power Minimum Flow Rate Fraction",
        "    600,                     !- Design Pressure Rise {Pa}",
        "    0.9,                     !- Motor Efficiency",
        "    1.0,                     !- Motor In Air Stream Fraction",
        "    autosize,               !- Design Electric Power Consumption {W}",
        "    TotalEfficiencyAndPressure,                        !- Design Power Sizing Method",
        "    ,                        !- Electric Power Per Unit Flow Rate {W/(m3/s)}",
        "    ,                        !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}",
        "    0.7,                     !- Fan Total Efficiency",
        "    TU1 Fan Power Curve,         !- Electric Power Function of Flow Fraction Curve Name",
        "    ,",
        "    ,",
        "    ,",
        "    ,",
        "    General;                 !- End-Use Subcategory",

        "  Curve:Quartic,",
        "    TU1 Fan Power Curve , !- Name",
        "    0.059,            !- Coefficient1 Constant",
        "    0,                !- Coefficient2 x       ",
        "    0,                !- Coefficient3 x**2    ",
        "    0.928,            !- Coefficient4 x**3    ",
        "    0,                !- Coefficient5 x**4    ",
        "    0.0,              !- Minimum Value of x",
        "    1.0,              !- Maximum Value of x",
        "    0.0,              !- Minimum Curve Output",
        "    1.0,              !- Maximum Curve Output",
        "    Dimensionless,    !- Input Unit Type for X",
        "    Dimensionless;    !- Output Unit Type",

        "  Coil:Heating:DX:VariableRefrigerantFlow:FluidTemperatureControl,",
        "    TU1 VRF DX Heating Coil, !- Name",
        "    VRFAvailSched,           !- Availability Schedule",
        "    TU1 VRF DX CCoil Outlet Node,  !- Coil Air Inlet Node",
        "    TU1 VRF DX HCoil Outlet Node,  !- Coil Air Outlet Node",
        "    6600.0,                  !- Rated Total Heating Capacity {W}",
        "    5,                       !- Indoor Unit Reference Subcooling Degrees Setpoint {C}    ",
        "    IUCondTempCurve;         !- Indoor Unit Condensing Temperature Function of Subcooling Curve Name",

        "  Curve:Quadratic,",
        "    IUCondTempCurve,         !- Name",
        "    -1.85,                   !- Coefficient1 Constant",
        "    0.411,                   !- Coefficient2 x",
        "    0.0196,                  !- Coefficient3 x**2",
        "    0,                       !- Minimum Value of x    ",
        "    20,                      !- Maximum Value of x    ",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature;             !- Output Unit Type",

        "  Coil:Cooling:DX:VariableRefrigerantFlow:FluidTemperatureControl,",
        "    TU1 VRF DX Cooling Coil, !- Name",
        "    VRFAvailSched,           !- Availability Schedule Name",
        "    zTU1 INLET NODE,         !- Coil Air Inlet Node",
        "    TU1 VRF DX CCoil Outlet Node,  !- Coil Air Outlet Node",
        "    6600.0,                  !- Rated Total Cooling Capacity {W}",
        "    0.500,                   !- Rated Sensible Heat Ratio",
        "    3,                       !- Indoor Unit Reference Superheating Degrees Setpoint {C}    ",
        "    IUEvapTempCurve,         !- Indoor Unit Evaporating Temperature Function of Superheating Curve Name    ",
        "    ;                        !- Name of Water Storage Tank for Condensate Collection",

        "  Curve:Quadratic,",
        "    IUEvapTempCurve,         !- Name",
        "    0,                       !- Coefficient1 Constant",
        "    0.843,                   !- Coefficient2 x",
        "    0,                       !- Coefficient3 x**2",
        "    0,                       !- Minimum Value of x    ",
        "    15,                      !- Maximum Value of x    ",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature;             !- Output Unit Type",

        "Zone,",
        "    SPACE1-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    239.247360229;           !- Volume {m3}",

        "  ZoneHVAC:EquipmentConnections,",
        "    SPACE1-1,                !- Zone Name",
        "    SPACE1-1 Eq,             !- Zone Conditioning Equipment List Name",
        "    SPACE1-1 In Nodes,       !- Zone Air Inlet Node or NodeList Name",
        "    zTU1 Inlet Node,         !- Zone Air Exhaust Node or NodeList Name",
        "    SPACE1-1 Node,           !- Zone Air Node Name",
        "    SPACE1-1 Return Outlet;  !- Zone Return Air Node Name",

        "  NodeList,",
        "    SPACE1-1 In Nodes,       !- Name",
        "    TU1 Outlet Node;         !- Node 1 Name",

        "  AirConditioner:VariableRefrigerantFlow:FluidTemperatureControl,",
        "    VRF Heat Pump,           !- Heat Pump Name",
        "    VRFAvailSched,           !- Availability Schedule Name",
        "    VRF Heat Pump TU List,   !- Zone Terminal Unit List Name",
        "    R410A,                   !- Refrigerant Type",
        "    41300,                   !- Rated Evaporative Capacity {W}",
        "    0.344,                   !- Rated Compressor Power Per Unit of Rated Evaporative Capacity {W/W}",
        "    -5,                      !- Minimum Outdoor Air Temperature in Cooling Mode {C}",
        "    43,                      !- Maximum Outdoor Air Temperature in Cooling Mode {C}",
        "    -20,                     !- Minimum Outdoor Air Temperature in Heating Mode {C}",
        "    22,                      !- Maximum Outdoor Air Temperature in Heating Mode {C}",
        "    3,                       !- Reference Outdoor Unit Superheating Degrees {C}",
        "    3,                       !- Reference Outdoor Unit Subcooling Degrees {C}",
        "    ConstantTemp,            !- Refrigerant Temperature Control Algorithm for Indoor Unit",
        "    6,                       !- Reference Evaporating Temperature for Indoor Unit {C}",
        "    44,                      !- Reference Condensing Temperature for Indoor Unit {C}",
        "    6,                       !- Variable Evaporating Temperature Minimum for Indoor Unit {C}",
        "    13,                      !- Variable Evaporating Temperature Maximum for Indoor Unit {C}",
        "    42,                      !- Variable Condensing Temperature Minimum for Indoor Unit {C}",
        "    46,                      !- Variable Condensing Temperature Maximum for Indoor Unit {C}",
        "    4.12E-3,                 !- Outdoor Unit Fan Power Per Unit of Rated Evaporative Capacity {W/W}",
        "    7.26E-5,                 !- Outdoor Unit Fan Flow Rate Per Unit of Rated Evaporative Capacity {m3/s-W}",
        "    OUEvapTempCurve,         !- Outdoor Unit Evaporating Temperature Function of Superheating Curve Name",
        "    OUCondTempCurve,         !- Outdoor Unit Condensing Temperature Function of Subcooling Curve Name",
        "    0.0508,                  !- Diameter of main pipe connecting outdoor unit to indoor units {m}",
        "    30,                      !- Length of main pipe connecting outdoor unit to indoor units {m}",
        "    36,                      !- Equivalent length of main pipe connecting outdoor unit to indoor units {m}",
        "    5,                       !- Height difference between the outdoor unit node and indoor unit node of the main pipe {m}",
        "    0.02,                    !- Insulation thickness of the main pipe {m}",
        "    0.032,                   !- Thermal conductivity of the main pipe insulation material {W/m-K}",
        "    33,                      !- Crankcase Heater Power per Compressor {W}",
        "    1,                       !- Number of Compressors",
        "    0.33,                    !- Ratio of Compressor Size to Total Compressor Capacity",
        "    7,                       !- Maximum Outdoor Dry-bulb Temperature for Crankcase Heater {C}",
        "    ,                        !- Defrost Strategy",
        "    ,                        !- Defrost Control",
        "    ,                        !- Defrost Energy Input Ratio Modifier Function of Temperature Curve Name",
        "    ,                        !- Defrost Time Period Fraction",
        "    ,                        !- Resistive Defrost Heater Capacity {W}",
        "    ,                        !- Maximum Outdoor Dry-bulb Temperature for Defrost Operation {C}",
        "    4500000,                 !- Compressor maximum delta Pressure {Pa}",
        "    3,                       !- Number of Compressor Loading Index Entries",
        "    1500,                    !- Compressor Speed at Loading Index 1 {rev/min}",
        "    MinSpdCooling,           !- Loading Index 1 Evaporative Capacity Multiplier Function of Temperature Curve Name",
        "    MinSpdPower,             !- Loading Index 1 Compressor Power Multiplier Function of Temperature Curve Name",
        "    3600,                    !- Compressor Speed at Loading Index 2 {rev/min}",
        "    Spd1Cooling,             !- Loading Index 2 Evaporative Capacity Multiplier Function of Temperature Curve Name",
        "    Spd1Power,               !- Loading Index 2 Compressor Power Multiplier Function of Temperature Curve Name",
        "    6000,                    !- Compressor Speed at Loading Index 3 {rev/min}",
        "    Spd2Cooling,             !- Loading Index 3 Evaporative Capacity Multiplier Function of Temperature Curve Name",
        "    Spd2Power;               !- Loading Index 3 Compressor Power Multiplier Function of Temperature Curve Name",

        "  Curve:Quadratic,",
        "    OUEvapTempCurve,         !- Name",
        "    0,                       !- Coefficient1 Constant",
        "    6.05E-1,                 !- Coefficient2 x",
        "    2.50E-2,                 !- Coefficient3 x**2",
        "    0,                       !- Minimum Value of x    ",
        "    15,                      !- Maximum Value of x    ",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature;             !- Output Unit Type",

        "  Curve:Quadratic,",
        "    OUCondTempCurve,         !- Name",
        "    0,                       !- Coefficient1 Constant",
        "    -2.91,                   !- Coefficient2 x",
        "    1.180,                   !- Coefficient3 x**2",
        "    0,                       !- Minimum Value of x    ",
        "    20,                      !- Maximum Value of x    ",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature;             !- Output Unit Type",
        "	",
        "  Curve:Biquadratic,",
        "    MinSpdCooling,           !- Name",
        "    3.19E-01,                !- Coefficient1 Constant",
        "    -1.26E-03,               !- Coefficient2 x",
        "    -2.15E-05,               !- Coefficient3 x**2",
        "    1.20E-02,                !- Coefficient4 y",
        "    1.05E-04,                !- Coefficient5 y**2",
        "    -8.66E-05,               !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    65,                      !- Maximum Value of x",
        "    -30,                     !- Minimum Value of y",
        "    15,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    MinSpdPower,             !- Name",
        "    8.79E-02 ,               !- Coefficient1 Constant",
        "    -1.72E-04,               !- Coefficient2 x",
        "    6.93E-05 ,               !- Coefficient3 x**2",
        "    -3.38E-05,               !- Coefficient4 y",
        "    -8.10E-06,               !- Coefficient5 y**2",
        "    -1.04E-05,               !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    65,                      !- Maximum Value of x",
        "    -30,                     !- Minimum Value of y",
        "    15,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "	",
        "  Curve:Biquadratic,",
        "    Spd1Cooling,             !- Name",
        "    8.12E-01 ,               !- Coefficient1 Constant",
        "    -4.23E-03,               !- Coefficient2 x",
        "    -4.11E-05,               !- Coefficient3 x**2",
        "    2.97E-02 ,               !- Coefficient4 y",
        "    2.67E-04 ,               !- Coefficient5 y**2",
        "    -2.23E-04,               !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    65,                      !- Maximum Value of x",
        "    -30,                     !- Minimum Value of y",
        "    15,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    Spd1Power,               !- Name",
        "    3.26E-01 ,               !- Coefficient1 Constant",
        "    -2.20E-03,               !- Coefficient2 x",
        "    1.42E-04 ,               !- Coefficient3 x**2",
        "    2.82E-03 ,               !- Coefficient4 y",
        "    2.86E-05 ,               !- Coefficient5 y**2",
        "    -3.50E-05,               !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    65,                      !- Maximum Value of x",
        "    -30,                     !- Minimum Value of y",
        "    15,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    Spd2Cooling,             !- Name",
        "    1.32E+00 ,               !- Coefficient1 Constant",
        "    -6.20E-03,               !- Coefficient2 x",
        "    -7.10E-05,               !- Coefficient3 x**2",
        "    4.89E-02 ,               !- Coefficient4 y",
        "    4.59E-04 ,               !- Coefficient5 y**2",
        "    -3.67E-04,               !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    65,                      !- Maximum Value of x",
        "    -30,                     !- Minimum Value of y",
        "    15,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    Spd2Power,               !- Name",
        "    6.56E-01 ,               !- Coefficient1 Constant",
        "    -3.71E-03,               !- Coefficient2 x",
        "    2.07E-04 ,               !- Coefficient3 x**2",
        "    1.05E-02 ,               !- Coefficient4 y",
        "    7.36E-05 ,               !- Coefficient5 y**2",
        "    -1.57E-04,               !- Coefficient6 x*y",
        "    15,                      !- Minimum Value of x",
        "    65,                      !- Maximum Value of x",
        "    -30,                     !- Minimum Value of y",
        "    15,                      !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  ZoneTerminalUnitList,",
        "    VRF Heat Pump TU List,   !- Zone Terminal Unit List Name",
        "    TU1;                     !- Zone Terminal Unit Name 1",

        "  OutdoorAir:NodeList,",
        "    OutsideAirInletNodes;    !- Node or NodeList Name 1",

        "  NodeList,",
        "    OutsideAirInletNodes,    !- Name",
        "    MyVRFOANode;             !- Node 1 Name",

        "  FluidProperties:Name,",
        "    R410a,                   !- Fluid Name",
        "    Refrigerant;             !- Fluid Type",

        "  FluidProperties:Temperatures,",
        "    R410aSaturatedTemperatures,  !- Name",
        "    -72.000,-69.000,-66.000,-63.000,-60.000,-57.000,-54.000,",
        "    -51.000,-48.000,-45.000,-42.000,-39.000,-36.000,-33.000,",
        "    -30.000,-27.000,-24.000,-21.000,-18.000,-15.000,-12.000,",
        "    -9.000,-6.000,-3.000,0.000,3.000,6.000,9.000,",
        "    12.000,15.000,18.000,21.000,24.000,27.000,30.000,",
        "    33.000,36.000,39.000,42.000,45.000,48.000,51.000,",
        "    54.000,57.000,60.000,63.000,66.000,69.000;",

        "  FluidProperties:Temperatures,",
        "    R410aSuperHeatTemperatures,  !- Name",
        "    -72.000,-66.000,-60.000,-54.000,-48.000,-45.000,-42.000,",
        "    -39.000,-36.000,-33.000,-30.000,-27.000,-24.000,-21.000,",
        "    -18.000,-15.000,-12.000,-9.000,-6.000,-3.000,0.000,",
        "    3.000,6.000,9.000,12.000,15.000,18.000,21.000,",
        "    24.000,27.000,30.000,33.000,36.000,39.000,42.000,",
        "    45.000,48.000,51.000,54.000,57.000,60.000,63.000,",
        "    66.000,69.000;",

        "  FluidProperties:Saturated,",
        "    R410a,                   !- Name",
        "    Pressure,                !- Fluid Property Type",
        "    FluidGas,                !- Fluid Phase",
        "    R410aSaturatedTemperatures,  !- Temperature Values Name",
        "    3.1238E+04,3.7717E+04,4.5248E+04,5.3954E+04,6.3963E+04,7.5412E+04,8.8445E+04,",
        "    1.0321E+05,1.1988E+05,1.3860E+05,1.5955E+05,1.8292E+05,2.0888E+05,2.3762E+05,",
        "    2.6935E+05,3.0426E+05,3.4257E+05,3.8449E+05,4.3024E+05,4.8004E+05,5.3412E+05,",
        "    5.9273E+05,6.5609E+05,7.2446E+05,7.9808E+05,8.7722E+05,9.6214E+05,1.0531E+06,",
        "    1.1504E+06,1.2543E+06,1.3651E+06,1.4831E+06,1.6086E+06,1.7419E+06,1.8834E+06,",
        "    2.0334E+06,2.1923E+06,2.3604E+06,2.5382E+06,2.7261E+06,2.9246E+06,3.1341E+06,",
        "    3.3552E+06,3.5886E+06,3.8348E+06,4.0949E+06,4.3697E+06,4.6607E+06;",

        "  FluidProperties:Saturated,",
        "    R410a,                   !- Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    Fluid,                   !- Fluid Phase",
        "    R410aSaturatedTemperatures,  !- Temperature Values Name",
        "    9.8535E+04,1.0259E+05,1.0665E+05,1.1072E+05,1.1479E+05,1.1888E+05,1.2297E+05,",
        "    1.2707E+05,1.3119E+05,1.3532E+05,1.3947E+05,1.4363E+05,1.4782E+05,1.5202E+05,",
        "    1.5624E+05,1.6048E+05,1.6475E+05,1.6904E+05,1.7337E+05,1.7772E+05,1.8210E+05,",
        "    1.8652E+05,1.9097E+05,1.9547E+05,2.0000E+05,2.0458E+05,2.0920E+05,2.1388E+05,",
        "    2.1861E+05,2.2340E+05,2.2825E+05,2.3316E+05,2.3815E+05,2.4322E+05,2.4838E+05,",
        "    2.5363E+05,2.5899E+05,2.6447E+05,2.7008E+05,2.7585E+05,2.8180E+05,2.8797E+05,",
        "    2.9441E+05,3.0120E+05,3.0848E+05,3.1650E+05,3.2578E+05,3.3815E+05;",

        "  FluidProperties:Saturated,",
        "    R410a,                   !- Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    FluidGas,                !- Fluid Phase",
        "    R410aSaturatedTemperatures,  !- Temperature Values Name",
        "    3.8813E+05,3.8981E+05,3.9148E+05,3.9313E+05,3.9476E+05,3.9637E+05,3.9796E+05,",
        "    3.9953E+05,4.0108E+05,4.0260E+05,4.0410E+05,4.0557E+05,4.0701E+05,4.0842E+05,",
        "    4.0980E+05,4.1114E+05,4.1245E+05,4.1373E+05,4.1496E+05,4.1615E+05,4.1730E+05,",
        "    4.1840E+05,4.1945E+05,4.2045E+05,4.2139E+05,4.2227E+05,4.2308E+05,4.2382E+05,",
        "    4.2448E+05,4.2507E+05,4.2556E+05,4.2595E+05,4.2624E+05,4.2641E+05,4.2646E+05,",
        "    4.2635E+05,4.2609E+05,4.2564E+05,4.2498E+05,4.2408E+05,4.2290E+05,4.2137E+05,",
        "    4.1941E+05,4.1692E+05,4.1370E+05,4.0942E+05,4.0343E+05,3.9373E+05;",

        "  FluidProperties:Saturated,",
        "    R410a,                   !- Name",
        "    Density,                 !- Fluid Property Type",
        "    Fluid,                   !- Fluid Phase",
        "    R410aSaturatedTemperatures,  !- Temperature Values Name",
        "    1.4127E+03,1.4036E+03,1.3946E+03,1.3854E+03,1.3762E+03,1.3669E+03,1.3576E+03,",
        "    1.3482E+03,1.3387E+03,1.3291E+03,1.3194E+03,1.3097E+03,1.2998E+03,1.2898E+03,",
        "    1.2797E+03,1.2694E+03,1.2591E+03,1.2486E+03,1.2379E+03,1.2271E+03,1.2160E+03,",
        "    1.2048E+03,1.1934E+03,1.1818E+03,1.1699E+03,1.1578E+03,1.1454E+03,1.1328E+03,",
        "    1.1197E+03,1.1064E+03,1.0927E+03,1.0785E+03,1.0639E+03,1.0488E+03,1.0331E+03,",
        "    1.0167E+03,9.9971E+02,9.8187E+02,9.6308E+02,9.4319E+02,9.2198E+02,8.9916E+02,",
        "    8.7429E+02,8.4672E+02,8.1537E+02,7.7825E+02,7.3095E+02,6.5903E+02;",

        "  FluidProperties:Saturated,",
        "    R410a,                   !- Name",
        "    Density,                 !- Fluid Property Type",
        "    FluidGas,                !- Fluid Phase",
        "    R410aSaturatedTemperatures,  !- Temperature Values Name",
        "    1.3845E+00,1.6517E+00,1.9588E+00,2.3100E+00,2.7097E+00,3.1627E+00,3.6737E+00,",
        "    4.2482E+00,4.8916E+00,5.6098E+00,6.4088E+00,7.2952E+00,8.2758E+00,9.3578E+00,",
        "    1.0549E+01,1.1857E+01,1.3292E+01,1.4861E+01,1.6576E+01,1.8447E+01,2.0485E+01,",
        "    2.2702E+01,2.5113E+01,2.7732E+01,3.0575E+01,3.3659E+01,3.7005E+01,4.0634E+01,",
        "    4.4571E+01,4.8844E+01,5.3483E+01,5.8525E+01,6.4012E+01,6.9991E+01,7.6520E+01,",
        "    8.3666E+01,9.1511E+01,1.0016E+02,1.0973E+02,1.2038E+02,1.3233E+02,1.4585E+02,",
        "    1.6135E+02,1.7940E+02,2.0095E+02,2.2766E+02,2.6301E+02,3.1759E+02;",

        "  FluidProperties:Saturated,",
        "    R410a,                   !- Name",
        "    SpecificHeat,            !- Fluid Property Type",
        "    Fluid,                   !- Fluid Phase",
        "    R410aSaturatedTemperatures,  !- Temperature Values Name",
        "    1.3499E+03,1.3515E+03,1.3534E+03,1.3557E+03,1.3584E+03,1.3614E+03,1.3648E+03,",
        "    1.3686E+03,1.3728E+03,1.3774E+03,1.3825E+03,1.3881E+03,1.3941E+03,1.4007E+03,",
        "    1.4078E+03,1.4155E+03,1.4238E+03,1.4327E+03,1.4424E+03,1.4527E+03,1.4639E+03,",
        "    1.4759E+03,1.4888E+03,1.5027E+03,1.5177E+03,1.5340E+03,1.5515E+03,1.5706E+03,",
        "    1.5914E+03,1.6141E+03,1.6390E+03,1.6664E+03,1.6968E+03,1.7307E+03,1.7689E+03,",
        "    1.8123E+03,1.8622E+03,1.9204E+03,1.9895E+03,2.0732E+03,2.1774E+03,2.3116E+03,",
        "    2.4924E+03,2.7507E+03,3.1534E+03,3.8723E+03,5.5190E+03,1.2701E+04;",

        "  FluidProperties:Saturated,",
        "    R410a,                   !- Name",
        "    SpecificHeat,            !- Fluid Property Type",
        "    FluidGas,                !- Fluid Phase",
        "    R410aSaturatedTemperatures,  !- Temperature Values Name",
        "    7.2387E+02,7.3519E+02,7.4693E+02,7.5910E+02,7.7167E+02,7.8465E+02,7.9802E+02,",
        "    8.1178E+02,8.2594E+02,8.4050E+02,8.5546E+02,8.7085E+02,8.8668E+02,9.0298E+02,",
        "    9.1979E+02,9.3715E+02,9.5511E+02,9.7372E+02,9.9307E+02,1.0132E+03,1.0343E+03,",
        "    1.0564E+03,1.0796E+03,1.1042E+03,1.1302E+03,1.1580E+03,1.1877E+03,1.2196E+03,",
        "    1.2541E+03,1.2917E+03,1.3329E+03,1.3783E+03,1.4287E+03,1.4853E+03,1.5494E+03,",
        "    1.6228E+03,1.7078E+03,1.8078E+03,1.9274E+03,2.0735E+03,2.2562E+03,2.4922E+03,",
        "    2.8094E+03,3.2596E+03,3.9504E+03,5.1465E+03,7.7185E+03,1.7076E+04;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.1238E+04,              !- Pressure {Pa}",
        "    3.8813E+05,3.9245E+05,3.9675E+05,4.0105E+05,4.0536E+05,4.0753E+05,4.0970E+05,",
        "    4.1189E+05,4.1408E+05,4.1628E+05,4.1849E+05,4.2071E+05,4.2294E+05,4.2518E+05,",
        "    4.2743E+05,4.2969E+05,4.3196E+05,4.3425E+05,4.3655E+05,4.3885E+05,4.4118E+05,",
        "    4.4351E+05,4.4586E+05,4.4821E+05,4.5058E+05,4.5297E+05,4.5536E+05,4.5777E+05,",
        "    4.6020E+05,4.6263E+05,4.6508E+05,4.6754E+05,4.7002E+05,4.7251E+05,4.7501E+05,",
        "    4.7752E+05,4.8005E+05,4.8259E+05,4.8515E+05,4.8772E+05,4.9030E+05,4.9290E+05,",
        "    4.9551E+05,4.9813E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.1238E+04,              !- Pressure {Pa}",
        "    1.3845E+00,1.3404E+00,1.2997E+00,1.2617E+00,1.2262E+00,1.2092E+00,1.1928E+00,",
        "    1.1768E+00,1.1613E+00,1.1462E+00,1.1316E+00,1.1173E+00,1.1034E+00,1.0898E+00,",
        "    1.0766E+00,1.0638E+00,1.0512E+00,1.0390E+00,1.0271E+00,1.0154E+00,1.0040E+00,",
        "    9.9285E-01,9.8197E-01,9.7133E-01,9.6093E-01,9.5075E-01,9.4079E-01,9.3104E-01,",
        "    9.2150E-01,9.1215E-01,9.0299E-01,8.9403E-01,8.8524E-01,8.7662E-01,8.6817E-01,",
        "    8.5989E-01,8.5177E-01,8.4380E-01,8.3598E-01,8.2831E-01,8.2077E-01,8.1338E-01,",
        "    8.0612E-01,7.9899E-01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    4.5248E+04,              !- Pressure {Pa}",
        "    0.0000E+00,3.9148E+05,3.9593E+05,4.0034E+05,4.0474E+05,4.0694E+05,4.0915E+05,",
        "    4.1136E+05,4.1358E+05,4.1580E+05,4.1803E+05,4.2027E+05,4.2252E+05,4.2478E+05,",
        "    4.2705E+05,4.2933E+05,4.3161E+05,4.3391E+05,4.3622E+05,4.3854E+05,4.4088E+05,",
        "    4.4322E+05,4.4558E+05,4.4794E+05,4.5032E+05,4.5272E+05,4.5512E+05,4.5754E+05,",
        "    4.5997E+05,4.6241E+05,4.6486E+05,4.6733E+05,4.6981E+05,4.7231E+05,4.7481E+05,",
        "    4.7733E+05,4.7987E+05,4.8241E+05,4.8497E+05,4.8755E+05,4.9013E+05,4.9273E+05,",
        "    4.9535E+05,4.9797E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    4.5248E+04,              !- Pressure {Pa}",
        "    0.0000E+00,1.9588E+00,1.8968E+00,1.8395E+00,1.7863E+00,1.7610E+00,1.7365E+00,",
        "    1.7128E+00,1.6898E+00,1.6674E+00,1.6457E+00,1.6246E+00,1.6041E+00,1.5842E+00,",
        "    1.5647E+00,1.5458E+00,1.5273E+00,1.5093E+00,1.4918E+00,1.4747E+00,1.4580E+00,",
        "    1.4416E+00,1.4257E+00,1.4101E+00,1.3949E+00,1.3800E+00,1.3654E+00,1.3512E+00,",
        "    1.3372E+00,1.3236E+00,1.3102E+00,1.2971E+00,1.2843E+00,1.2717E+00,1.2594E+00,",
        "    1.2473E+00,1.2355E+00,1.2239E+00,1.2125E+00,1.2013E+00,1.1903E+00,1.1796E+00,",
        "    1.1690E+00,1.1586E+00;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    6.3963E+04,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,3.9476E+05,3.9935E+05,4.0388E+05,4.0614E+05,4.0839E+05,",
        "    4.1064E+05,4.1290E+05,4.1516E+05,4.1742E+05,4.1969E+05,4.2196E+05,4.2425E+05,",
        "    4.2654E+05,4.2884E+05,4.3114E+05,4.3346E+05,4.3579E+05,4.3813E+05,4.4047E+05,",
        "    4.4283E+05,4.4520E+05,4.4758E+05,4.4997E+05,4.5238E+05,4.5479E+05,4.5722E+05,",
        "    4.5966E+05,4.6211E+05,4.6457E+05,4.6705E+05,4.6954E+05,4.7204E+05,4.7455E+05,",
        "    4.7708E+05,4.7962E+05,4.8217E+05,4.8474E+05,4.8732E+05,4.8991E+05,4.9252E+05,",
        "    4.9513E+05,4.9777E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    6.3963E+04,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,2.7097E+00,2.6240E+00,2.5451E+00,2.5078E+00,2.4718E+00,",
        "    2.4370E+00,2.4034E+00,2.3708E+00,2.3393E+00,2.3086E+00,2.2789E+00,2.2500E+00,",
        "    2.2219E+00,2.1945E+00,2.1679E+00,2.1420E+00,2.1167E+00,2.0921E+00,2.0681E+00,",
        "    2.0446E+00,2.0217E+00,1.9994E+00,1.9776E+00,1.9562E+00,1.9354E+00,1.9150E+00,",
        "    1.8950E+00,1.8755E+00,1.8564E+00,1.8377E+00,1.8194E+00,1.8014E+00,1.7839E+00,",
        "    1.7666E+00,1.7497E+00,1.7332E+00,1.7169E+00,1.7010E+00,1.6854E+00,1.6700E+00,",
        "    1.6550E+00,1.6402E+00;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    8.8445E+04,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,3.9796E+05,4.0270E+05,4.0503E+05,4.0736E+05,",
        "    4.0967E+05,4.1198E+05,4.1429E+05,4.1660E+05,4.1891E+05,4.2122E+05,4.2354E+05,",
        "    4.2586E+05,4.2819E+05,4.3052E+05,4.3286E+05,4.3521E+05,4.3757E+05,4.3994E+05,",
        "    4.4232E+05,4.4470E+05,4.4710E+05,4.4951E+05,4.5193E+05,4.5436E+05,4.5680E+05,",
        "    4.5925E+05,4.6171E+05,4.6419E+05,4.6668E+05,4.6918E+05,4.7169E+05,4.7421E+05,",
        "    4.7675E+05,4.7930E+05,4.8186E+05,4.8443E+05,4.8702E+05,4.8962E+05,4.9223E+05,",
        "    4.9486E+05,4.9749E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    8.8445E+04,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,3.6737E+00,3.5570E+00,3.5024E+00,3.4500E+00,",
        "    3.3995E+00,3.3509E+00,3.3039E+00,3.2585E+00,3.2146E+00,3.1720E+00,3.1308E+00,",
        "    3.0907E+00,3.0518E+00,3.0140E+00,2.9772E+00,2.9414E+00,2.9065E+00,2.8726E+00,",
        "    2.8395E+00,2.8072E+00,2.7757E+00,2.7449E+00,2.7149E+00,2.6856E+00,2.6569E+00,",
        "    2.6289E+00,2.6015E+00,2.5747E+00,2.5485E+00,2.5228E+00,2.4977E+00,2.4731E+00,",
        "    2.4490E+00,2.4254E+00,2.4022E+00,2.3795E+00,2.3573E+00,2.3354E+00,2.3140E+00,",
        "    2.2930E+00,2.2724E+00;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.1988E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.0108E+05,4.0354E+05,4.0597E+05,",
        "    4.0838E+05,4.1077E+05,4.1315E+05,4.1552E+05,4.1788E+05,4.2025E+05,4.2261E+05,",
        "    4.2497E+05,4.2734E+05,4.2971E+05,4.3209E+05,4.3447E+05,4.3685E+05,4.3925E+05,",
        "    4.4165E+05,4.4406E+05,4.4648E+05,4.4891E+05,4.5135E+05,4.5380E+05,4.5626E+05,",
        "    4.5873E+05,4.6121E+05,4.6370E+05,4.6620E+05,4.6871E+05,4.7124E+05,4.7377E+05,",
        "    4.7632E+05,4.7888E+05,4.8145E+05,4.8404E+05,4.8663E+05,4.8924E+05,4.9186E+05,",
        "    4.9450E+05,4.9715E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.1988E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.8918E+00,4.8116E+00,4.7352E+00,",
        "    4.6621E+00,4.5920E+00,4.5247E+00,4.4599E+00,4.3974E+00,4.3370E+00,4.2787E+00,",
        "    4.2221E+00,4.1674E+00,4.1143E+00,4.0627E+00,4.0126E+00,3.9639E+00,3.9165E+00,",
        "    3.8704E+00,3.8255E+00,3.7817E+00,3.7390E+00,3.6974E+00,3.6567E+00,3.6171E+00,",
        "    3.5783E+00,3.5405E+00,3.5035E+00,3.4673E+00,3.4319E+00,3.3973E+00,3.3634E+00,",
        "    3.3302E+00,3.2977E+00,3.2659E+00,3.2347E+00,3.2041E+00,3.1742E+00,3.1448E+00,",
        "    3.1160E+00,3.0877E+00;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.3860E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.0260E+05,4.0510E+05,",
        "    4.0757E+05,4.1002E+05,4.1244E+05,4.1485E+05,4.1726E+05,4.1965E+05,4.2204E+05,",
        "    4.2444E+05,4.2683E+05,4.2922E+05,4.3162E+05,4.3402E+05,4.3642E+05,4.3883E+05,",
        "    4.4125E+05,4.4368E+05,4.4611E+05,4.4855E+05,4.5100E+05,4.5346E+05,4.5593E+05,",
        "    4.5841E+05,4.6090E+05,4.6340E+05,4.6591E+05,4.6843E+05,4.7097E+05,4.7351E+05,",
        "    4.7606E+05,4.7863E+05,4.8121E+05,4.8380E+05,4.8640E+05,4.8902E+05,4.9165E+05,",
        "    4.9428E+05,4.9694E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.3860E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,5.6098E+00,5.5173E+00,",
        "    5.4293E+00,5.3451E+00,5.2645E+00,5.1871E+00,5.1127E+00,5.0409E+00,4.9717E+00,",
        "    4.9047E+00,4.8399E+00,4.7772E+00,4.7163E+00,4.6573E+00,4.5999E+00,4.5442E+00,",
        "    4.4900E+00,4.4372E+00,4.3859E+00,4.3358E+00,4.2870E+00,4.2394E+00,4.1930E+00,",
        "    4.1476E+00,4.1033E+00,4.0601E+00,4.0178E+00,3.9765E+00,3.9360E+00,3.8965E+00,",
        "    3.8578E+00,3.8199E+00,3.7828E+00,3.7464E+00,3.7108E+00,3.6759E+00,3.6417E+00,",
        "    3.6081E+00,3.5752E+00;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.5955E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.0410E+05,",
        "    4.0664E+05,4.0915E+05,4.1163E+05,4.1409E+05,4.1654E+05,4.1898E+05,4.2140E+05,",
        "    4.2383E+05,4.2625E+05,4.2867E+05,4.3109E+05,4.3351E+05,4.3593E+05,4.3836E+05,",
        "    4.4080E+05,4.4324E+05,4.4569E+05,4.4815E+05,4.5061E+05,4.5309E+05,4.5557E+05,",
        "    4.5806E+05,4.6056E+05,4.6307E+05,4.6559E+05,4.6812E+05,4.7066E+05,4.7321E+05,",
        "    4.7578E+05,4.7835E+05,4.8094E+05,4.8354E+05,4.8615E+05,4.8877E+05,4.9140E+05,",
        "    4.9404E+05,4.9670E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.5955E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,6.4087E+00,",
        "    6.3023E+00,6.2010E+00,6.1045E+00,6.0120E+00,5.9233E+00,5.8380E+00,5.7559E+00,",
        "    5.6767E+00,5.6001E+00,5.5261E+00,5.4544E+00,5.3850E+00,5.3176E+00,5.2521E+00,",
        "    5.1885E+00,5.1267E+00,5.0666E+00,5.0080E+00,4.9509E+00,4.8953E+00,4.8411E+00,",
        "    4.7882E+00,4.7366E+00,4.6862E+00,4.6369E+00,4.5888E+00,4.5417E+00,4.4957E+00,",
        "    4.4507E+00,4.4066E+00,4.3635E+00,4.3212E+00,4.2799E+00,4.2393E+00,4.1996E+00,",
        "    4.1607E+00,4.1225E+00;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.8292E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    4.0557E+05,4.0816E+05,4.1071E+05,4.1323E+05,4.1573E+05,4.1821E+05,4.2068E+05,",
        "    4.2313E+05,4.2559E+05,4.2804E+05,4.3049E+05,4.3293E+05,4.3538E+05,4.3784E+05,",
        "    4.4029E+05,4.4275E+05,4.4522E+05,4.4769E+05,4.5017E+05,4.5266E+05,4.5516E+05,",
        "    4.5766E+05,4.6017E+05,4.6270E+05,4.6523E+05,4.6777E+05,4.7032E+05,4.7288E+05,",
        "    4.7546E+05,4.7804E+05,4.8063E+05,4.8324E+05,4.8586E+05,4.8848E+05,4.9112E+05,",
        "    4.9378E+05,4.9644E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.8292E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    7.2953E+00,7.1732E+00,7.0571E+00,6.9465E+00,6.8408E+00,6.7394E+00,6.6420E+00,",
        "    6.5482E+00,6.4578E+00,6.3706E+00,6.2862E+00,6.2046E+00,6.1255E+00,6.0488E+00,",
        "    5.9743E+00,5.9020E+00,5.8317E+00,5.7633E+00,5.6968E+00,5.6320E+00,5.5688E+00,",
        "    5.5072E+00,5.4472E+00,5.3885E+00,5.3313E+00,5.2754E+00,5.2208E+00,5.1674E+00,",
        "    5.1152E+00,5.0641E+00,5.0141E+00,4.9652E+00,4.9173E+00,4.8703E+00,4.8244E+00,",
        "    4.7793E+00,4.7352E+00;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.0888E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,4.0701E+05,4.0964E+05,4.1224E+05,4.1480E+05,4.1733E+05,4.1985E+05,",
        "    4.2235E+05,4.2485E+05,4.2733E+05,4.2981E+05,4.3229E+05,4.3477E+05,4.3724E+05,",
        "    4.3972E+05,4.4221E+05,4.4469E+05,4.4719E+05,4.4968E+05,4.5219E+05,4.5470E+05,",
        "    4.5722E+05,4.5974E+05,4.6228E+05,4.6482E+05,4.6738E+05,4.6994E+05,4.7251E+05,",
        "    4.7510E+05,4.7769E+05,4.8029E+05,4.8291E+05,4.8553E+05,4.8817E+05,4.9082E+05,",
        "    4.9348E+05,4.9615E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.0888E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,8.2759E+00,8.1361E+00,8.0034E+00,7.8770E+00,7.7563E+00,7.6407E+00,",
        "    7.5297E+00,7.4230E+00,7.3201E+00,7.2209E+00,7.1251E+00,7.0323E+00,6.9425E+00,",
        "    6.8555E+00,6.7710E+00,6.6890E+00,6.6093E+00,6.5318E+00,6.4564E+00,6.3830E+00,",
        "    6.3115E+00,6.2417E+00,6.1738E+00,6.1074E+00,6.0426E+00,5.9794E+00,5.9176E+00,",
        "    5.8572E+00,5.7981E+00,5.7404E+00,5.6839E+00,5.6286E+00,5.5744E+00,5.5214E+00,",
        "    5.4694E+00,5.4185E+00;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.3762E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,4.0842E+05,4.1110E+05,4.1374E+05,4.1634E+05,4.1892E+05,",
        "    4.2147E+05,4.2401E+05,4.2654E+05,4.2905E+05,4.3157E+05,4.3407E+05,4.3658E+05,",
        "    4.3909E+05,4.4159E+05,4.4410E+05,4.4662E+05,4.4914E+05,4.5166E+05,4.5419E+05,",
        "    4.5672E+05,4.5927E+05,4.6182E+05,4.6437E+05,4.6694E+05,4.6952E+05,4.7210E+05,",
        "    4.7470E+05,4.7730E+05,4.7992E+05,4.8254E+05,4.8517E+05,4.8782E+05,4.9048E+05,",
        "    4.9315E+05,4.9582E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.3762E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,9.3578E+00,9.1979E+00,9.0465E+00,8.9024E+00,8.7650E+00,",
        "    8.6335E+00,8.5073E+00,8.3861E+00,8.2694E+00,8.1569E+00,8.0482E+00,7.9431E+00,",
        "    7.8414E+00,7.7429E+00,7.6473E+00,7.5546E+00,7.4645E+00,7.3769E+00,7.2917E+00,",
        "    7.2088E+00,7.1280E+00,7.0493E+00,6.9726E+00,6.8977E+00,6.8246E+00,6.7533E+00,",
        "    6.6836E+00,6.6155E+00,6.5489E+00,6.4838E+00,6.4200E+00,6.3577E+00,6.2967E+00,",
        "    6.2369E+00,6.1783E+00;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.6935E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,4.0980E+05,4.1253E+05,4.1521E+05,4.1786E+05,",
        "    4.2047E+05,4.2307E+05,4.2564E+05,4.2820E+05,4.3075E+05,4.3330E+05,4.3584E+05,",
        "    4.3837E+05,4.4091E+05,4.4345E+05,4.4599E+05,4.4853E+05,4.5107E+05,4.5362E+05,",
        "    4.5617E+05,4.5873E+05,4.6130E+05,4.6388E+05,4.6646E+05,4.6905E+05,4.7165E+05,",
        "    4.7425E+05,4.7687E+05,4.7950E+05,4.8213E+05,4.8478E+05,4.8743E+05,4.9010E+05,",
        "    4.9278E+05,4.9546E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.6935E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,1.0549E+01,1.0367E+01,1.0194E+01,1.0030E+01,",
        "    9.8741E+00,9.7248E+00,9.5817E+00,9.4443E+00,9.3122E+00,9.1848E+00,9.0619E+00,",
        "    8.9431E+00,8.8282E+00,8.7170E+00,8.6091E+00,8.5045E+00,8.4029E+00,8.3042E+00,",
        "    8.2081E+00,8.1147E+00,8.0237E+00,7.9351E+00,7.8487E+00,7.7644E+00,7.6822E+00,",
        "    7.6019E+00,7.5235E+00,7.4469E+00,7.3721E+00,7.2989E+00,7.2273E+00,7.1572E+00,",
        "    7.0886E+00,7.0214E+00;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.0426E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.1114E+05,4.1392E+05,4.1665E+05,",
        "    4.1934E+05,4.2200E+05,4.2463E+05,4.2725E+05,4.2984E+05,4.3243E+05,4.3501E+05,",
        "    4.3758E+05,4.4015E+05,4.4272E+05,4.4528E+05,4.4785E+05,4.5042E+05,4.5299E+05,",
        "    4.5556E+05,4.5814E+05,4.6073E+05,4.6332E+05,4.6592E+05,4.6853E+05,4.7114E+05,",
        "    4.7376E+05,4.7639E+05,4.7903E+05,4.8168E+05,4.8434E+05,4.8701E+05,4.8968E+05,",
        "    4.9237E+05,4.9507E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.0426E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,1.1857E+01,1.1650E+01,1.1453E+01,",
        "    1.1267E+01,1.1090E+01,1.0921E+01,1.0759E+01,1.0604E+01,1.0454E+01,1.0310E+01,",
        "    1.0172E+01,1.0038E+01,9.9083E+00,9.7830E+00,9.6615E+00,9.5438E+00,9.4294E+00,",
        "    9.3184E+00,9.2104E+00,9.1054E+00,9.0032E+00,8.9037E+00,8.8067E+00,8.7121E+00,",
        "    8.6198E+00,8.5297E+00,8.4418E+00,8.3559E+00,8.2719E+00,8.1898E+00,8.1095E+00,",
        "    8.0310E+00,7.9541E+00;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.4257E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.1245E+05,4.1529E+05,",
        "    4.1807E+05,4.2080E+05,4.2350E+05,4.2617E+05,4.2883E+05,4.3146E+05,4.3408E+05,",
        "    4.3670E+05,4.3930E+05,4.4190E+05,4.4450E+05,4.4709E+05,4.4969E+05,4.5229E+05,",
        "    4.5489E+05,4.5749E+05,4.6010E+05,4.6271E+05,4.6533E+05,4.6795E+05,4.7058E+05,",
        "    4.7322E+05,4.7587E+05,4.7852E+05,4.8118E+05,4.8385E+05,4.8653E+05,4.8922E+05,",
        "    4.9192E+05,4.9463E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.4257E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,1.3292E+01,1.3056E+01,",
        "    1.2833E+01,1.2622E+01,1.2421E+01,1.2230E+01,1.2047E+01,1.1871E+01,1.1703E+01,",
        "    1.1541E+01,1.1385E+01,1.1234E+01,1.1088E+01,1.0947E+01,1.0811E+01,1.0678E+01,",
        "    1.0550E+01,1.0425E+01,1.0304E+01,1.0187E+01,1.0072E+01,9.9605E+00,9.8518E+00,",
        "    9.7459E+00,9.6426E+00,9.5417E+00,9.4433E+00,9.3472E+00,9.2533E+00,9.1615E+00,",
        "    9.0717E+00,8.9839E+00;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.8449E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.1373E+05,",
        "    4.1661E+05,4.1944E+05,4.2222E+05,4.2497E+05,4.2768E+05,4.3038E+05,4.3305E+05,",
        "    4.3571E+05,4.3836E+05,4.4100E+05,4.4363E+05,4.4626E+05,4.4889E+05,4.5151E+05,",
        "    4.5414E+05,4.5677E+05,4.5940E+05,4.6203E+05,4.6467E+05,4.6732E+05,4.6997E+05,",
        "    4.7262E+05,4.7529E+05,4.7796E+05,4.8063E+05,4.8332E+05,4.8601E+05,4.8872E+05,",
        "    4.9143E+05,4.9415E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.8449E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,1.4861E+01,",
        "    1.4593E+01,1.4341E+01,1.4102E+01,1.3875E+01,1.3659E+01,1.3452E+01,1.3255E+01,",
        "    1.3065E+01,1.2882E+01,1.2707E+01,1.2537E+01,1.2374E+01,1.2216E+01,1.2063E+01,",
        "    1.1914E+01,1.1771E+01,1.1631E+01,1.1495E+01,1.1364E+01,1.1236E+01,1.1111E+01,",
        "    1.0989E+01,1.0871E+01,1.0755E+01,1.0643E+01,1.0533E+01,1.0426E+01,1.0321E+01,",
        "    1.0218E+01,1.0118E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    4.3024E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    4.1496E+05,4.1790E+05,4.2078E+05,4.2361E+05,4.2641E+05,4.2916E+05,4.3190E+05,",
        "    4.3461E+05,4.3731E+05,4.3999E+05,4.4267E+05,4.4533E+05,4.4800E+05,4.5066E+05,",
        "    4.5331E+05,4.5597E+05,4.5863E+05,4.6129E+05,4.6395E+05,4.6662E+05,4.6929E+05,",
        "    4.7197E+05,4.7465E+05,4.7734E+05,4.8003E+05,4.8273E+05,4.8544E+05,4.8816E+05,",
        "    4.9089E+05,4.9362E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    4.3024E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    1.6576E+01,1.6272E+01,1.5986E+01,1.5716E+01,1.5460E+01,1.5216E+01,1.4983E+01,",
        "    1.4761E+01,1.4547E+01,1.4343E+01,1.4145E+01,1.3955E+01,1.3772E+01,1.3595E+01,",
        "    1.3424E+01,1.3258E+01,1.3097E+01,1.2941E+01,1.2789E+01,1.2642E+01,1.2499E+01,",
        "    1.2360E+01,1.2224E+01,1.2092E+01,1.1964E+01,1.1838E+01,1.1716E+01,1.1596E+01,",
        "    1.1480E+01,1.1365E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    4.8004E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,4.1615E+05,4.1915E+05,4.2209E+05,4.2497E+05,4.2781E+05,4.3061E+05,",
        "    4.3339E+05,4.3614E+05,4.3888E+05,4.4160E+05,4.4431E+05,4.4701E+05,4.4971E+05,",
        "    4.5240E+05,4.5509E+05,4.5778E+05,4.6047E+05,4.6316E+05,4.6585E+05,4.6855E+05,",
        "    4.7124E+05,4.7395E+05,4.7666E+05,4.7937E+05,4.8209E+05,4.8482E+05,4.8755E+05,",
        "    4.9029E+05,4.9304E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    4.8004E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,1.8447E+01,1.8102E+01,1.7778E+01,1.7473E+01,1.7184E+01,1.6910E+01,",
        "    1.6648E+01,1.6398E+01,1.6158E+01,1.5928E+01,1.5707E+01,1.5495E+01,1.5289E+01,",
        "    1.5091E+01,1.4900E+01,1.4715E+01,1.4535E+01,1.4361E+01,1.4192E+01,1.4028E+01,",
        "    1.3869E+01,1.3714E+01,1.3563E+01,1.3416E+01,1.3273E+01,1.3133E+01,1.2997E+01,",
        "    1.2864E+01,1.2734E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    5.3412E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,4.1730E+05,4.2036E+05,4.2335E+05,4.2629E+05,4.2917E+05,",
        "    4.3202E+05,4.3485E+05,4.3764E+05,4.4042E+05,4.4318E+05,4.4593E+05,4.4867E+05,",
        "    4.5140E+05,4.5413E+05,4.5685E+05,4.5957E+05,4.6229E+05,4.6501E+05,4.6773E+05,",
        "    4.7045E+05,4.7318E+05,4.7591E+05,4.7865E+05,4.8139E+05,4.8413E+05,4.8689E+05,",
        "    4.8965E+05,4.9241E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    5.3412E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,2.0485E+01,2.0094E+01,1.9728E+01,1.9383E+01,1.9058E+01,",
        "    1.8749E+01,1.8455E+01,1.8174E+01,1.7905E+01,1.7648E+01,1.7400E+01,1.7162E+01,",
        "    1.6933E+01,1.6712E+01,1.6498E+01,1.6292E+01,1.6092E+01,1.5898E+01,1.5710E+01,",
        "    1.5527E+01,1.5350E+01,1.5178E+01,1.5010E+01,1.4847E+01,1.4688E+01,1.4533E+01,",
        "    1.4382E+01,1.4234E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    5.9273E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,4.1840E+05,4.2153E+05,4.2458E+05,4.2756E+05,",
        "    4.3050E+05,4.3340E+05,4.3627E+05,4.3911E+05,4.4193E+05,4.4473E+05,4.4752E+05,",
        "    4.5029E+05,4.5306E+05,4.5582E+05,4.5858E+05,4.6133E+05,4.6408E+05,4.6683E+05,",
        "    4.6959E+05,4.7234E+05,4.7509E+05,4.7785E+05,4.8062E+05,4.8338E+05,4.8616E+05,",
        "    4.8894E+05,4.9172E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    5.9273E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,2.2703E+01,2.2260E+01,2.1846E+01,2.1458E+01,",
        "    2.1091E+01,2.0744E+01,2.0413E+01,2.0098E+01,1.9798E+01,1.9509E+01,1.9233E+01,",
        "    1.8967E+01,1.8711E+01,1.8465E+01,1.8227E+01,1.7996E+01,1.7774E+01,1.7558E+01,",
        "    1.7349E+01,1.7146E+01,1.6950E+01,1.6758E+01,1.6572E+01,1.6391E+01,1.6215E+01,",
        "    1.6043E+01,1.5876E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    6.5609E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.1945E+05,4.2264E+05,4.2575E+05,",
        "    4.2880E+05,4.3179E+05,4.3474E+05,4.3765E+05,4.4054E+05,4.4340E+05,4.4625E+05,",
        "    4.4907E+05,4.5189E+05,4.5469E+05,4.5749E+05,4.6028E+05,4.6307E+05,4.6585E+05,",
        "    4.6864E+05,4.7142E+05,4.7420E+05,4.7699E+05,4.7978E+05,4.8257E+05,4.8536E+05,",
        "    4.8816E+05,4.9097E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    6.5609E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,2.5113E+01,2.4612E+01,2.4145E+01,",
        "    2.3707E+01,2.3294E+01,2.2904E+01,2.2533E+01,2.2180E+01,2.1844E+01,2.1522E+01,",
        "    2.1213E+01,2.0916E+01,2.0631E+01,2.0356E+01,2.0091E+01,1.9836E+01,1.9588E+01,",
        "    1.9349E+01,1.9117E+01,1.8892E+01,1.8673E+01,1.8461E+01,1.8255E+01,1.8055E+01,",
        "    1.7860E+01,1.7670E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    7.2446E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.2045E+05,4.2371E+05,",
        "    4.2688E+05,4.2999E+05,4.3304E+05,4.3604E+05,4.3900E+05,4.4194E+05,4.4484E+05,",
        "    4.4773E+05,4.5060E+05,4.5345E+05,4.5630E+05,4.5913E+05,4.6196E+05,4.6478E+05,",
        "    4.6760E+05,4.7041E+05,4.7323E+05,4.7604E+05,4.7886E+05,4.8168E+05,4.8450E+05,",
        "    4.8732E+05,4.9015E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    7.2446E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,2.7732E+01,2.7164E+01,",
        "    2.6636E+01,2.6143E+01,2.5678E+01,2.5240E+01,2.4825E+01,2.4430E+01,2.4053E+01,",
        "    2.3694E+01,2.3349E+01,2.3019E+01,2.2701E+01,2.2396E+01,2.2101E+01,2.1817E+01,",
        "    2.1542E+01,2.1277E+01,2.1019E+01,2.0770E+01,2.0529E+01,2.0294E+01,2.0066E+01,",
        "    1.9844E+01,1.9629E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    7.9808E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.2139E+05,",
        "    4.2472E+05,4.2797E+05,4.3113E+05,4.3424E+05,4.3730E+05,4.4031E+05,4.4329E+05,",
        "    4.4625E+05,4.4918E+05,4.5209E+05,4.5499E+05,4.5787E+05,4.6074E+05,4.6361E+05,",
        "    4.6646E+05,4.6932E+05,4.7217E+05,4.7502E+05,4.7786E+05,4.8071E+05,4.8356E+05,",
        "    4.8641E+05,4.8926E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    7.9808E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,3.0574E+01,",
        "    2.9931E+01,2.9335E+01,2.8779E+01,2.8257E+01,2.7765E+01,2.7299E+01,2.6857E+01,",
        "    2.6436E+01,2.6035E+01,2.5651E+01,2.5283E+01,2.4930E+01,2.4590E+01,2.4263E+01,",
        "    2.3948E+01,2.3644E+01,2.3349E+01,2.3065E+01,2.2789E+01,2.2521E+01,2.2262E+01,",
        "    2.2010E+01,2.1766E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    8.7722E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    4.2227E+05,4.2568E+05,4.2899E+05,4.3223E+05,4.3540E+05,4.3851E+05,4.4158E+05,",
        "    4.4461E+05,4.4761E+05,4.5059E+05,4.5355E+05,4.5649E+05,4.5941E+05,4.6232E+05,",
        "    4.6523E+05,4.6812E+05,4.7101E+05,4.7389E+05,4.7678E+05,4.7966E+05,4.8254E+05,",
        "    4.8542E+05,4.8830E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    8.7722E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    3.3659E+01,3.2930E+01,3.2256E+01,3.1629E+01,3.1042E+01,3.0490E+01,2.9968E+01,",
        "    2.9474E+01,2.9004E+01,2.8557E+01,2.8129E+01,2.7720E+01,2.7327E+01,2.6950E+01,",
        "    2.6587E+01,2.6238E+01,2.5900E+01,2.5575E+01,2.5260E+01,2.4955E+01,2.4660E+01,",
        "    2.4374E+01,2.4096E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    9.6214E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,4.2308E+05,4.2658E+05,4.2997E+05,4.3327E+05,4.3650E+05,4.3968E+05,",
        "    4.4280E+05,4.4589E+05,4.4894E+05,4.5197E+05,4.5497E+05,4.5795E+05,4.6092E+05,",
        "    4.6387E+05,4.6681E+05,4.6975E+05,4.7267E+05,4.7559E+05,4.7851E+05,4.8142E+05,",
        "    4.8434E+05,4.8725E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    9.6214E+05,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,3.7005E+01,3.6178E+01,3.5416E+01,3.4709E+01,3.4049E+01,3.3429E+01,",
        "    3.2845E+01,3.2292E+01,3.1768E+01,3.1269E+01,3.0793E+01,3.0338E+01,2.9902E+01,",
        "    2.9484E+01,2.9082E+01,2.8694E+01,2.8321E+01,2.7961E+01,2.7613E+01,2.7277E+01,",
        "    2.6951E+01,2.6636E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.0531E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,4.2382E+05,4.2741E+05,4.3088E+05,4.3426E+05,4.3756E+05,",
        "    4.4079E+05,4.4398E+05,4.4712E+05,4.5023E+05,4.5330E+05,4.5635E+05,4.5938E+05,",
        "    4.6239E+05,4.6539E+05,4.6837E+05,4.7134E+05,4.7430E+05,4.7726E+05,4.8021E+05,",
        "    4.8316E+05,4.8611E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.0531E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,4.0634E+01,3.9695E+01,3.8832E+01,3.8035E+01,3.7292E+01,",
        "    3.6597E+01,3.5943E+01,3.5325E+01,3.4740E+01,3.4184E+01,3.3654E+01,3.3149E+01,",
        "    3.2665E+01,3.2201E+01,3.1755E+01,3.1327E+01,3.0915E+01,3.0517E+01,3.0133E+01,",
        "    2.9762E+01,2.9403E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.1504E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,4.2448E+05,4.2817E+05,4.3173E+05,4.3518E+05,",
        "    4.3855E+05,4.4186E+05,4.4511E+05,4.4831E+05,4.5147E+05,4.5459E+05,4.5769E+05,",
        "    4.6077E+05,4.6383E+05,4.6686E+05,4.6989E+05,4.7290E+05,4.7591E+05,4.7890E+05,",
        "    4.8189E+05,4.8488E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.1504E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,4.4572E+01,4.3503E+01,4.2527E+01,4.1626E+01,",
        "    4.0790E+01,4.0010E+01,3.9277E+01,3.8587E+01,3.7934E+01,3.7314E+01,3.6725E+01,",
        "    3.6164E+01,3.5627E+01,3.5113E+01,3.4620E+01,3.4146E+01,3.3691E+01,3.3252E+01,",
        "    3.2828E+01,3.2419E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.2543E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.2507E+05,4.2886E+05,4.3251E+05,",
        "    4.3605E+05,4.3949E+05,4.4287E+05,4.4618E+05,4.4944E+05,4.5266E+05,4.5584E+05,",
        "    4.5899E+05,4.6212E+05,4.6522E+05,4.6831E+05,4.7138E+05,4.7443E+05,4.7748E+05,",
        "    4.8051E+05,4.8354E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.2543E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.8844E+01,4.7627E+01,4.6519E+01,",
        "    4.5502E+01,4.4560E+01,4.3684E+01,4.2863E+01,4.2091E+01,4.1363E+01,4.0673E+01,",
        "    4.0018E+01,3.9394E+01,3.8799E+01,3.8230E+01,3.7685E+01,3.7161E+01,3.6658E+01,",
        "    3.6174E+01,3.5708E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.3651E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.2556E+05,4.2947E+05,",
        "    4.3322E+05,4.3684E+05,4.4037E+05,4.4382E+05,4.4720E+05,4.5053E+05,4.5381E+05,",
        "    4.5705E+05,4.6025E+05,4.6343E+05,4.6658E+05,4.6971E+05,4.7283E+05,4.7592E+05,",
        "    4.7901E+05,4.8209E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.3651E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,5.3484E+01,5.2094E+01,",
        "    5.0835E+01,4.9684E+01,4.8623E+01,4.7638E+01,4.6718E+01,4.5855E+01,4.5042E+01,",
        "    4.4274E+01,4.3546E+01,4.2854E+01,4.2194E+01,4.1564E+01,4.0961E+01,4.0383E+01,",
        "    3.9828E+01,3.9294E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.4831E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.2595E+05,",
        "    4.2999E+05,4.3385E+05,4.3757E+05,4.4119E+05,4.4471E+05,4.4817E+05,4.5156E+05,",
        "    4.5490E+05,4.5820E+05,4.6146E+05,4.6469E+05,4.6790E+05,4.7108E+05,4.7424E+05,",
        "    4.7738E+05,4.8051E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.4831E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,5.8526E+01,",
        "    5.6935E+01,5.5502E+01,5.4199E+01,5.3001E+01,5.1893E+01,5.0861E+01,4.9896E+01,",
        "    4.8989E+01,4.8133E+01,4.7324E+01,4.6555E+01,4.5824E+01,4.5127E+01,4.4461E+01,",
        "    4.3823E+01,4.3211E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.6086E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    4.2624E+05,4.3041E+05,4.3439E+05,4.3822E+05,4.4193E+05,4.4554E+05,4.4907E+05,",
        "    4.5254E+05,4.5595E+05,4.5931E+05,4.6263E+05,4.6591E+05,4.6917E+05,4.7240E+05,",
        "    4.7561E+05,4.7880E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.6086E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    6.4013E+01,6.2185E+01,6.0551E+01,5.9071E+01,5.7718E+01,5.6470E+01,5.5313E+01,",
        "    5.4232E+01,5.3220E+01,5.2267E+01,5.1367E+01,5.0514E+01,4.9704E+01,4.8933E+01,",
        "    4.8196E+01,4.7492E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.7419E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,4.2642E+05,4.3074E+05,4.3485E+05,4.3879E+05,4.4260E+05,4.4630E+05,",
        "    4.4991E+05,4.5345E+05,4.5693E+05,4.6036E+05,4.6374E+05,4.6709E+05,4.7040E+05,",
        "    4.7368E+05,4.7694E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.7419E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,6.9990E+01,6.7883E+01,6.6014E+01,6.4331E+01,6.2800E+01,6.1394E+01,",
        "    6.0094E+01,5.8884E+01,5.7753E+01,5.6691E+01,5.5691E+01,5.4744E+01,5.3847E+01,",
        "    5.2994E+01,5.2181E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.8834E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,4.2646E+05,4.3096E+05,4.3521E+05,4.3927E+05,4.4319E+05,",
        "    4.4699E+05,4.5069E+05,4.5431E+05,4.5786E+05,4.6136E+05,4.6481E+05,4.6821E+05,",
        "    4.7158E+05,4.7492E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    1.8834E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,7.6519E+01,7.4080E+01,7.1935E+01,7.0017E+01,6.8281E+01,",
        "    6.6694E+01,6.5232E+01,6.3876E+01,6.2613E+01,6.1429E+01,6.0316E+01,5.9266E+01,",
        "    5.8272E+01,5.7328E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.0334E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,4.2635E+05,4.3105E+05,4.3546E+05,4.3966E+05,",
        "    4.4369E+05,4.4760E+05,4.5139E+05,4.5510E+05,4.5873E+05,4.6230E+05,4.6582E+05,",
        "    4.6929E+05,4.7272E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.0334E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,8.3665E+01,8.0827E+01,7.8356E+01,7.6164E+01,",
        "    7.4192E+01,7.2398E+01,7.0753E+01,6.9232E+01,6.7819E+01,6.6499E+01,6.5261E+01,",
        "    6.4095E+01,6.2993E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.1923E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.2609E+05,4.3101E+05,4.3560E+05,",
        "    4.3995E+05,4.4411E+05,4.4812E+05,4.5202E+05,4.5582E+05,4.5953E+05,4.6318E+05,",
        "    4.6677E+05,4.7030E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.1923E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,9.1511E+01,8.8190E+01,8.5332E+01,",
        "    8.2819E+01,8.0573E+01,7.8542E+01,7.6687E+01,7.4980E+01,7.3398E+01,7.1925E+01,",
        "    7.0547E+01,6.9252E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.3604E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.2564E+05,4.3082E+05,",
        "    4.3561E+05,4.4013E+05,4.4443E+05,4.4856E+05,4.5257E+05,4.5646E+05,4.6027E+05,",
        "    4.6400E+05,4.6766E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.3604E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,1.0015E+02,9.6239E+01,",
        "    9.2918E+01,9.0027E+01,8.7463E+01,8.5159E+01,8.3065E+01,8.1145E+01,7.9374E+01,",
        "    7.7729E+01,7.6195E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.5382E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.2498E+05,",
        "    4.3047E+05,4.3549E+05,4.4019E+05,4.4464E+05,4.4891E+05,4.5303E+05,4.5703E+05,",
        "    4.6093E+05,4.6475E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.5382E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,1.0972E+02,",
        "    1.0507E+02,1.0119E+02,9.7851E+01,9.4915E+01,9.2295E+01,8.9926E+01,8.7766E+01,",
        "    8.5780E+01,8.3942E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.7261E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    4.2408E+05,4.2993E+05,4.3522E+05,4.4012E+05,4.4475E+05,4.4916E+05,4.5341E+05,",
        "    4.5752E+05,4.6152E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.7261E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    1.2038E+02,1.1479E+02,1.1023E+02,1.0635E+02,1.0298E+02,9.9995E+01,9.7312E+01,",
        "    9.4877E+01,9.2647E+01;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.9246E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,4.2290E+05,4.2918E+05,4.3478E+05,4.3992E+05,4.4473E+05,4.4931E+05,",
        "    4.5369E+05,4.5792E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    2.9246E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,1.3233E+02,1.2554E+02,1.2013E+02,1.1562E+02,1.1173E+02,1.0831E+02,",
        "    1.0527E+02,1.0252E+02;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.1341E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,4.2137E+05,4.2820E+05,4.3416E+05,4.3957E+05,4.4459E+05,",
        "    4.4934E+05,4.5387E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.1341E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,1.4585E+02,1.3748E+02,1.3102E+02,1.2572E+02,1.2122E+02,",
        "    1.1731E+02,1.1384E+02;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.3552E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,4.1941E+05,4.2695E+05,4.3334E+05,4.3905E+05,",
        "    4.4432E+05,4.4926E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.3552E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,1.6135E+02,1.5082E+02,1.4302E+02,1.3677E+02,",
        "    1.3154E+02,1.2704E+02;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.5886E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.1692E+05,4.2539E+05,4.3229E+05,",
        "    4.3836E+05,4.4389E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.5886E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,1.7941E+02,1.6585E+02,1.5632E+02,",
        "    1.4890E+02,1.4279E+02;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.8348E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.1370E+05,4.2346E+05,",
        "    4.3100E+05,4.3748E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    3.8348E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,2.0095E+02,1.8289E+02,",
        "    1.7111E+02,1.6223E+02;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    4.0949E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,4.0942E+05,",
        "    4.2109E+05,4.2943E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    4.0949E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,2.2766E+02,",
        "    2.0246E+02,1.8765E+02;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    4.3697E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    4.0343E+05,4.1823E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    4.3697E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    2.6302E+02,2.2513E+02;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Enthalpy,                !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    4.6607E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,3.9373E+05;",

        "  FluidProperties:Superheated,",
        "    R410a,                   !- Fluid Name",
        "    Density,                 !- Fluid Property Type",
        "    R410aSuperHeatTemperatures,  !- Temperature Values Name",
        "    4.6607E+06,              !- Pressure {Pa}",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,0.0000E+00,",
        "    0.0000E+00,3.1758E+02;",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);

    GetVRFInput(*state);
    state->dataHVACVarRefFlow->GetVRFInputFlag = false;

    // get input test for terminal air single duct mixer on supply side of VRF terminal unit
    ASSERT_EQ(1, state->dataSingleDuct->NumATMixers);
    EXPECT_EQ("SPACE1-1 DOAS AIR TERMINAL", state->dataSingleDuct->SysATMixer(1).Name);                 // single duct air terminal mixer name
    EXPECT_EQ((int)HVAC::MixerType::SupplySide, (int)state->dataSingleDuct->SysATMixer(1).type);        // air terminal mixer connection type
    EXPECT_EQ("AIRTERMINAL:SINGLEDUCT:MIXER", state->dataDefineEquipment->AirDistUnit(1).EquipType(1)); // Air distribution unit equipment type
    EXPECT_EQ("TU1", state->dataHVACVarRefFlow->VRFTU(1).Name);                                         // zoneHVAC equipment name

    state->dataGlobal->BeginEnvrnFlag = false;

    // set input variables
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutDryBulbTemp = 35.0;
    state->dataEnvrn->OutHumRat = 0.0098;
    state->dataEnvrn->OutEnthalpy = Psychrometrics::PsyHFnTdbW(state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutHumRat);
    state->dataEnvrn->StdRhoAir = 1.20;
    HVACInletMassFlowRate = 0.50;
    PrimaryAirMassFlowRate = 0.1;

    // set zoneNode air condition
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp = 24.0;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat = 0.0075;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp,
                                   state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat);

    state->dataHVACVarRefFlow->CoolingLoad.allocate(1);
    state->dataHVACVarRefFlow->HeatingLoad.allocate(1);
    state->dataHVACVarRefFlow->HeatingLoad(1) = false;
    state->dataHVACVarRefFlow->CoolingLoad(1) = true;
    state->dataHVACVarRefFlow->CompOnMassFlow = HVACInletMassFlowRate;    // supply air mass flow rate
    state->dataHVACVarRefFlow->OACompOnMassFlow = PrimaryAirMassFlowRate; // OA mass flow rate
    state->dataHVACVarRefFlow->CompOnFlowRatio = 1.0;                     // compressor is on
    state->dataHVACGlobal->TurnFansOff = false;
    state->dataHVACGlobal->TurnFansOn = true;

    VRFNum = 1;
    VRFTUNum = 1;
    state->dataHVACVarRefFlow->VRFTU(VRFTUNum).fanOp = HVAC::FanOp::Cycling;
    state->dataHVACVarRefFlow->VRFTU(VRFTUNum).isInZone = true;
    state->dataHVACVarRefFlow->VRFTU(VRFTUNum).ZoneAirNode = state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode;
    // initialize mass flow rates
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUInletNodeNum).MassFlowRate = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerPriNode).MassFlowRate = PrimaryAirMassFlowRate;
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerPriNode).MassFlowRateMaxAvail = PrimaryAirMassFlowRate;

    // set fan parameters
    state->dataFans->fans(1)->maxAirMassFlowRate = HVACInletMassFlowRate;
    state->dataFans->fans(1)->inletAirMassFlowRate = HVACInletMassFlowRate;
    state->dataFans->fans(1)->rhoAirStdInit = state->dataEnvrn->StdRhoAir;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->inletNodeNum).MassFlowRateMaxAvail = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->outletNodeNum).MassFlowRateMax = HVACInletMassFlowRate;

    // set DX coil rated performance parameters
    state->dataDXCoils->DXCoil(1).RatedCBF(1) = 0.05;
    state->dataDXCoils->DXCoil(1).RatedAirMassFlowRate(1) = HVACInletMassFlowRate;

    // primary air condition set at outdoor air condition
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerPriNode).Temp = state->dataEnvrn->OutDryBulbTemp;
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerPriNode).HumRat = state->dataEnvrn->OutHumRat;
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).ATMixerPriNode).Enthalpy = state->dataEnvrn->OutEnthalpy;

    // set VRF terminal unit inlet condition to zone air node
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUInletNodeNum).Temp =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp;
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUInletNodeNum).HumRat =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat;
    state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUInletNodeNum).Enthalpy =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy;

    state->dataHVACVarRefFlow->VRFTU(1).ZoneNum = 1;
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataGlobal->SysSizingCalc = true;

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -4000.0;
    QZnReq = state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP;

    state->dataHVACVarRefFlow->VRFTU(VRFTUNum).availSched->currentVal = 1.0;    // unit is always available
    state->dataHVACVarRefFlow->VRFTU(VRFTUNum).fanAvailSched->currentVal = 1.0; // fan is always available
    state->dataHVACVarRefFlow->VRF(1).VRFCondCyclingRatio = 1.0;
    state->dataFans->fans(1)->set_size(*state); // add fan sizing

    // set secondary air mass flow rate to zero
    state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).MassFlowRate = 0.0;
    // Simulate zoneHVAC equipment (VRF terminal unit)
    SimVRF(*state, VRFTUNum, FirstHVACIteration, OnOffAirFlowRatio, QUnitOutVRFTU, LatOutputProvided, QZnReq);

    // check the terminal air mixer secondary air mass flow rate, requires updating the secondary flow
    SecondaryAirMassFlowRate = state->dataLoopNodes->Node(state->dataHVACVarRefFlow->VRFTU(VRFTUNum).VRFTUInletNodeNum).MassFlowRate;
    ASSERT_EQ(SecondaryAirMassFlowRate, state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).MassFlowRate);
    // check the terminal air mixer outlet flow rate must be equal to the mass flow rate of VRFTU + the primary air
    ATMixerOutletMassFlowRate = SecondaryAirMassFlowRate + PrimaryAirMassFlowRate;
    ASSERT_EQ(ATMixerOutletMassFlowRate, state->dataSingleDuct->SysATMixer(1).MixedAirMassFlowRate);
    //    changed the threshold from 2 to 3 as the fan was changed from hard-sized to autosize, a little more error is expected
    ASSERT_NEAR(QZnReq, QUnitOutVRFTU, 3.0);
}

TEST_F(EnergyPlusFixture, AirTerminalSingleDuctMixer_SimUnitVent_ATMInletSide)
{

    bool ErrorsFound(false);
    bool FirstHVACIteration(false);
    Real64 HVACInletMassFlowRate(0.0);
    Real64 PrimaryAirMassFlowRate(0.0);
    Real64 SecondaryAirMassFlowRate(0.0);
    Real64 LatOutputProvided(0.0);
    Real64 QUnitOut(0.0);
    state->dataUnitVentilators->QZnReq = 0.0;
    int ZoneNum(1);
    int UnitVentNum(1);

    std::string const idf_objects = delimited_string({

        "  AirTerminal:SingleDuct:Mixer,",
        "    SPACE1-1 DOAS Air Terminal,  !- Name",
        "    ZoneHVAC:UnitVentilator,     !- ZoneHVAC Terminal Unit Object Type",
        "    SPACE1-1 Unit Vent,          !- ZoneHVAC Terminal Unit Name",
        "    SPACE1-1 Unit Vent Inlet,    !- Terminal Unit Outlet Node Name",
        "    SPACE1-1 Mixer Primary Inlet,   !- Terminal Unit Primary Air Inlet Node Name",
        "    SPACE1-1 Mixer Secondary Inlet, !- Terminal Unit Secondary Air Inlet Node Name",
        "    InletSide;                      !- Terminal Unit Connection Type",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SPACE1-1 DOAS ATU,           !- Name",
        "    SPACE1-1 Unit Vent Inlet,    !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:Mixer,  !- Air Terminal Object Type",
        "    SPACE1-1 DOAS Air Terminal;  !- Air Terminal Name",

        "  Schedule:Compact,",
        "    FanAvailSched,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 16",
        "    1.0;                     !- Field 17",

        "  ZoneHVAC:EquipmentList,",
        "    SPACE1-1 Equipment,      !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE1-1 DOAS ATU,       !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction",
        "    ,                        !- Zone Equipment 1 Sequential Heating Fraction",
        "    ZoneHVAC:UnitVentilator, !- Zone Equipment 2 Object Type",
        "    SPACE1-1 Unit Vent,      !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    2,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 2 Sequential Cooling Fraction",
        "    ;                        !- Zone Equipment 2 Sequential Heating Fraction",

        "  ZoneHVAC:UnitVentilator,",
        "    SPACE1-1 Unit Vent,      !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    0.50,                    !- Maximum Supply Air Flow Rate {m3/s}",
        "    VariablePercent,         !- Outdoor Air Control Type",
        "    0.20,                    !- Minimum Outdoor Air Flow Rate {m3/s}",
        "    U2MinOASched,            !- Minimum Outdoor Air Schedule Name",
        "    0.50,                    !- Maximum Outdoor Air Flow Rate {m3/s}",
        "    UnitVentMaxOA,           !- Maximum Outdoor Air Fraction or Temperature Schedule Name",
        "    SPACE1-1 Unit Vent Inlet,!- Air Inlet Node Name",
        "    SPACE1-1 Supply Inlet,   !- Air Outlet Node Name",
        "    ,                        !- Outdoor Air Node Name",
        "    ,                        !- Exhaust Air Node Name",
        "    ,                        !- Mixed Air Node Name",
        "    Fan:ConstantVolume,      !- Supply Air Fan Object Type",
        "    Zone1UnitVentFan,        !- Supply Air Fan Name",
        "    HEATING,                 !- Coil Option",
        "    ,                        !- Supply Air Fan Operating Mode Schedule Name",
        "    Coil:Heating:Fuel,       !- Heating Coil Object Type",
        "    Zone1UnitVentHeatingCoil,!- Heating Coil Name",
        "    0.001;                   !- Heating Convergence Tolerance",

        "  Schedule:Compact,",
        "    UnitVentMaxOA,           !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "  Schedule:Compact,",
        "    U2MinOASched,            !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00, 0.5;       !- Field 3",

        "  Fan:ConstantVolume,",
        "    Zone1UnitVentFan,        !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    0.5,                     !- Fan Total Efficiency",
        "    0.0,                     !- Pressure Rise {Pa}",
        "    0.50,                    !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1.0,                     !- Motor In Airstream Fraction",
        "    SPACE1-1 Unit Vent Inlet,!- Air Inlet Node Name",
        "    SPACE1-1 Fan Outlet;     !- Air Outlet Node Name",

        "  Coil:Heating:Fuel,",
        "    Zone1UnitVentHeatingCoil,!- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    NaturalGas,              !- Fuel Type",
        "    0.8,                     !- Gas Burner Efficiency",
        "    10000.0,                 !- Nominal Capacity {W}",
        "    SPACE1-1 Fan Outlet,     !- Air Inlet Node Name",
        "    SPACE1-1 Supply Inlet;   !- Air Outlet Node Name",

        "  Zone,",
        "    SPACE1-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    239.247360229;           !- Volume {m3}",

        "  ZoneHVAC:EquipmentConnections,",
        "    SPACE1-1,                !- Zone Name",
        "    SPACE1-1 Equipment,      !- Zone Conditioning Equipment List Name",
        "    SPACE1-1 Inlets,         !- Zone Air Inlet Node or NodeList Name",
        "    SPACE1-1 Mixer Secondary Inlet,  !- Zone Air Exhaust Node or NodeList Name",
        "    SPACE1-1 Zone Air Node,  !- Zone Air Node Name",
        "    SPACE1-1 Return Outlet;  !- Zone Return Air Node Name",

        "  NodeList,",
        "    SPACE1-1 Inlets,         !- Name",
        "    SPACE1-1 Supply Inlet;   !- Node 1 Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);
    GetUnitVentilatorInput(*state);
    state->dataUnitVentilators->GetUnitVentilatorInputFlag = false;

    // get input test for terminal air single duct mixer on inlet side of PTHP
    ASSERT_EQ(1, state->dataSingleDuct->NumATMixers);
    EXPECT_EQ("SPACE1-1 DOAS AIR TERMINAL", state->dataSingleDuct->SysATMixer(1).Name);                 // single duct air terminal mixer name
    EXPECT_EQ((int)HVAC::MixerType::InletSide, (int)state->dataSingleDuct->SysATMixer(1).type);         // air terminal mixer connection type
    EXPECT_EQ("AIRTERMINAL:SINGLEDUCT:MIXER", state->dataDefineEquipment->AirDistUnit(1).EquipType(1)); // Air distribution unit equipment type

    state->dataGlobal->BeginEnvrnFlag = false;

    // set input variables
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutDryBulbTemp = 10.0;
    state->dataEnvrn->OutHumRat = 0.0070;
    state->dataEnvrn->OutEnthalpy = Psychrometrics::PsyHFnTdbW(state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutHumRat);
    state->dataEnvrn->StdRhoAir = 1.00;
    HVACInletMassFlowRate = 0.50;
    PrimaryAirMassFlowRate = 0.1;

    // set zoneNode air condition
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp = 24.0;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat = 0.0070;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp,
                                   state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat);

    state->dataHVACGlobal->TurnFansOff = false;
    state->dataHVACGlobal->TurnFansOn = true;

    UnitVentNum = 1;
    // set the mass flow rates from the input volume flow rates
    state->dataUnitVentilators->UnitVent(UnitVentNum).MaxAirMassFlow =
        state->dataEnvrn->StdRhoAir * state->dataUnitVentilators->UnitVent(UnitVentNum).MaxAirVolFlow;
    state->dataUnitVentilators->UnitVent(UnitVentNum).OutAirMassFlow =
        state->dataEnvrn->StdRhoAir * state->dataUnitVentilators->UnitVent(UnitVentNum).OutAirVolFlow;
    state->dataUnitVentilators->UnitVent(UnitVentNum).MinOutAirMassFlow =
        state->dataEnvrn->StdRhoAir * state->dataUnitVentilators->UnitVent(UnitVentNum).MinOutAirVolFlow;

    state->dataUnitVentilators->UnitVent(UnitVentNum).fanOp = HVAC::FanOp::Cycling;
    // initialize mass flow rates
    state->dataLoopNodes->Node(state->dataUnitVentilators->UnitVent(UnitVentNum).AirInNode).MassFlowRate = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(state->dataUnitVentilators->UnitVent(UnitVentNum).AirInNode).MassFlowRateMax = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(state->dataUnitVentilators->UnitVent(UnitVentNum).ATMixerPriNode).MassFlowRate = PrimaryAirMassFlowRate;
    state->dataLoopNodes->Node(state->dataUnitVentilators->UnitVent(UnitVentNum).ATMixerPriNode).MassFlowRateMaxAvail = PrimaryAirMassFlowRate;

    // set fan parameters
    state->dataFans->fans(1)->maxAirMassFlowRate = HVACInletMassFlowRate;
    state->dataFans->fans(1)->inletAirMassFlowRate = HVACInletMassFlowRate;
    state->dataFans->fans(1)->rhoAirStdInit = state->dataEnvrn->StdRhoAir;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->inletNodeNum).MassFlowRateMaxAvail = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->outletNodeNum).MassFlowRateMax = HVACInletMassFlowRate;

    // primary air condition set at outdoor air condition
    state->dataLoopNodes->Node(state->dataUnitVentilators->UnitVent(UnitVentNum).ATMixerPriNode).Temp = state->dataEnvrn->OutDryBulbTemp;
    state->dataLoopNodes->Node(state->dataUnitVentilators->UnitVent(UnitVentNum).ATMixerPriNode).HumRat = state->dataEnvrn->OutHumRat;
    state->dataLoopNodes->Node(state->dataUnitVentilators->UnitVent(UnitVentNum).ATMixerPriNode).Enthalpy = state->dataEnvrn->OutEnthalpy;

    // set secondary air (recirculating air) conditions to zone air node
    state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).Temp =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp;
    state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).HumRat =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat;
    state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).Enthalpy =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy;

    state->dataUnitVentilators->UnitVent(1).ZonePtr = 1;
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataGlobal->SysSizingCalc = true;

    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = 5000.0;
    state->dataUnitVentilators->QZnReq = state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired;

    state->dataUnitVentilators->UnitVent(UnitVentNum).availSched->currentVal = 1.0;    // unit is always available
    state->dataUnitVentilators->UnitVent(UnitVentNum).fanAvailSched->currentVal = 1.0; // fan is always available
    state->dataUnitVentilators->UnitVent(UnitVentNum).minOASched->currentVal = 0.5;    // min OA fraction is always available

    // set secondary air mass flow rate to zero
    state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).MassFlowRate = 0.0;
    // simulate Unit Ventilator zoneHVAC equipment
    SimUnitVentilator(
        *state, state->dataUnitVentilators->UnitVent(UnitVentNum).Name, ZoneNum, FirstHVACIteration, QUnitOut, LatOutputProvided, UnitVentNum);
    // apply mass conservation to determine secondary air mass flow rate
    SecondaryAirMassFlowRate =
        state->dataLoopNodes->Node(state->dataUnitVentilators->UnitVent(UnitVentNum).AirInNode).MassFlowRate - PrimaryAirMassFlowRate;
    // check the air mixer secondary air mass flow rate
    ASSERT_EQ(SecondaryAirMassFlowRate, state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).MassFlowRate);
    // check the cooling output delivered is within 2.0 Watt of zone cooling load
    ASSERT_NEAR(state->dataUnitVentilators->QZnReq, QUnitOut, 0.001);
}

TEST_F(EnergyPlusFixture, AirTerminalSingleDuctMixer_SimUnitVent_ATMSupplySide)
{

    bool ErrorsFound(false);
    bool FirstHVACIteration(false);
    Real64 HVACInletMassFlowRate(0.0);
    Real64 PrimaryAirMassFlowRate(0.0);
    Real64 SecondaryAirMassFlowRate(0.0);
    Real64 ATMixerOutletMassFlowRate(0.0);
    Real64 LatOutputProvided(0.0);
    Real64 QUnitOut(0.0);
    Real64 QZnReq(0.0);
    int ZoneNum(1);
    int UnitVentNum(1);

    std::string const idf_objects = delimited_string({

        "  AirTerminal:SingleDuct:Mixer,",
        "    SPACE1-1 DOAS Air Terminal, !- Name",
        "    ZoneHVAC:UnitVentilator,    !- ZoneHVAC Terminal Unit Object Type",
        "    SPACE1-1 Unit Vent,         !- ZoneHVAC Terminal Unit Name",
        "    SPACE1-1 Supply Inlet,      !- Terminal Unit Outlet Node Name",
        "    SPACE1-1 Primary Air Inlet, !- Terminal Unit Primary Air Inlet Node Name",
        "    SPACE1-1 Unit Vent Outlet,  !- Terminal Unit Secondary Air Inlet Node Name",
        "    SupplySide;                 !- Terminal Unit Connection Type",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SPACE1-1 DOAS ATU,       !- Name",
        "    SPACE1-1 Supply Inlet,   !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:Mixer,  !- Air Terminal Object Type",
        "    SPACE1-1 DOAS Air Terminal;  !- Air Terminal Name",

        "  Schedule:Compact,",
        "    FanAvailSched,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 16",
        "    1.0;                     !- Field 17",

        "  ZoneHVAC:EquipmentList,",
        "    SPACE1-1 Equipment,      !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE1-1 DOAS ATU,       !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction",
        "    ,                        !- Zone Equipment 1 Sequential Heating Fraction",
        "    ZoneHVAC:UnitVentilator, !- Zone Equipment 2 Object Type",
        "    SPACE1-1 Unit Vent,      !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    2,                       !- Zone Equipment 2 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 2 Sequential Cooling Fraction",
        "    ;                        !- Zone Equipment 2 Sequential Heating Fraction",

        "  ZoneHVAC:UnitVentilator,",
        "    SPACE1-1 Unit Vent,      !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    0.50,                    !- Maximum Supply Air Flow Rate {m3/s}",
        "    VariablePercent,         !- Outdoor Air Control Type",
        "    0.20,                    !- Minimum Outdoor Air Flow Rate {m3/s}",
        "    U2MinOASched,            !- Minimum Outdoor Air Schedule Name",
        "    0.50,                    !- Maximum Outdoor Air Flow Rate {m3/s}",
        "    UnitVentMaxOA,           !- Maximum Outdoor Air Fraction or Temperature Schedule Name",
        "    SPACE1-1 Unit Vent Inlet,  !- Air Inlet Node Name",
        "    SPACE1-1 Unit Vent Outlet, !- Air Outlet Node Name",
        "    ,                        !- Outdoor Air Node Name",
        "    ,                        !- Exhaust Air Node Name",
        "    ,                        !- Mixed Air Node Name",
        "    Fan:ConstantVolume,      !- Supply Air Fan Object Type",
        "    Zone1UnitVentFan,        !- Supply Air Fan Name",
        "    HEATING,                 !- Coil Option",
        "    ,                        !- Supply Air Fan Operating Mode Schedule Name",
        "    Coil:Heating:Fuel,       !- Heating Coil Object Type",
        "    Zone1UnitVentHeatingCoil,!- Heating Coil Name",
        "    0.001;                   !- Heating Convergence Tolerance",

        "  Schedule:Compact,",
        "    UnitVentMaxOA,           !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "  Schedule:Compact,",
        "    U2MinOASched,            !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,0.5;        !- Field 3",

        "  Fan:ConstantVolume,",
        "    Zone1UnitVentFan,        !- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    0.5,                     !- Fan Total Efficiency",
        "    0.0,                     !- Pressure Rise {Pa}",
        "    0.50,                    !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1.0,                     !- Motor In Airstream Fraction",
        "    SPACE1-1 Unit Vent Inlet,!- Air Inlet Node Name",
        "    SPACE1-1 Fan Outlet;     !- Air Outlet Node Name",

        "  Coil:Heating:Fuel,",
        "    Zone1UnitVentHeatingCoil,!- Name",
        "    FanAvailSched,           !- Availability Schedule Name",
        "    NaturalGas,              !- Fuel Type",
        "    0.8,                     !- Gas Burner Efficiency",
        "    10000.0,                 !- Nominal Capacity {W}",
        "    SPACE1-1 Fan Outlet,     !- Air Inlet Node Name",
        "    SPACE1-1 Unit Vent Outlet;  !- Air Outlet Node Name",

        "  Zone,",
        "    SPACE1-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    239.247360229;           !- Volume {m3}",

        "  ZoneHVAC:EquipmentConnections,",
        "    SPACE1-1,                !- Zone Name",
        "    SPACE1-1 Equipment,      !- Zone Conditioning Equipment List Name",
        "    SPACE1-1 Inlets,         !- Zone Air Inlet Node or NodeList Name",
        "    SPACE1-1 Unit Vent Inlet,!- Zone Air Exhaust Node or NodeList Name",
        "    SPACE1-1 Zone Air Node,  !- Zone Air Node Name",
        "    SPACE1-1 Return Outlet;  !- Zone Return Air Node Name",

        "  NodeList,",
        "    SPACE1-1 Inlets,         !- Name",
        "    SPACE1-1 Supply Inlet;   !- Node 1 Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);
    GetUnitVentilatorInput(*state);
    state->dataUnitVentilators->GetUnitVentilatorInputFlag = false;

    // get input test for terminal air single duct mixer on supply side of PTHP
    ASSERT_EQ(1, state->dataSingleDuct->NumATMixers);
    EXPECT_EQ("SPACE1-1 DOAS AIR TERMINAL", state->dataSingleDuct->SysATMixer(1).Name);                 // single duct air terminal mixer name
    EXPECT_EQ((int)HVAC::MixerType::SupplySide, (int)state->dataSingleDuct->SysATMixer(1).type);        // air terminal mixer connection type
    EXPECT_EQ("AIRTERMINAL:SINGLEDUCT:MIXER", state->dataDefineEquipment->AirDistUnit(1).EquipType(1)); // Air distribution unit equipment type

    // set input variables
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutDryBulbTemp = 10.0;
    state->dataEnvrn->OutHumRat = 0.0070;
    state->dataEnvrn->OutEnthalpy = Psychrometrics::PsyHFnTdbW(state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutHumRat);
    state->dataEnvrn->StdRhoAir = 1.00;
    HVACInletMassFlowRate = 0.50;
    PrimaryAirMassFlowRate = 0.1;

    state->dataGlobal->BeginEnvrnFlag = false;

    // set zoneNode air condition
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp = 24.0;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat = 0.0070;
    state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy =
        Psychrometrics::PsyHFnTdbW(state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp,
                                   state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat);

    state->dataHVACGlobal->TurnFansOff = false;
    state->dataHVACGlobal->TurnFansOn = true;

    UnitVentNum = 1;
    // set the mass flow rates from the input volume flow rates
    state->dataUnitVentilators->UnitVent(UnitVentNum).MaxAirMassFlow =
        state->dataEnvrn->StdRhoAir * state->dataUnitVentilators->UnitVent(UnitVentNum).MaxAirVolFlow;
    state->dataUnitVentilators->UnitVent(UnitVentNum).OutAirMassFlow =
        state->dataEnvrn->StdRhoAir * state->dataUnitVentilators->UnitVent(UnitVentNum).OutAirVolFlow;
    state->dataUnitVentilators->UnitVent(UnitVentNum).MinOutAirMassFlow =
        state->dataEnvrn->StdRhoAir * state->dataUnitVentilators->UnitVent(UnitVentNum).MinOutAirVolFlow;

    state->dataUnitVentilators->UnitVent(UnitVentNum).fanOp = HVAC::FanOp::Cycling;
    // initialize mass flow rates
    state->dataLoopNodes->Node(state->dataUnitVentilators->UnitVent(UnitVentNum).AirInNode).MassFlowRate = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(state->dataUnitVentilators->UnitVent(UnitVentNum).AirInNode).MassFlowRateMax = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(state->dataUnitVentilators->UnitVent(UnitVentNum).ATMixerPriNode).MassFlowRate = PrimaryAirMassFlowRate;
    state->dataLoopNodes->Node(state->dataUnitVentilators->UnitVent(UnitVentNum).ATMixerPriNode).MassFlowRateMaxAvail = PrimaryAirMassFlowRate;

    // set fan parameters
    state->dataFans->fans(1)->maxAirMassFlowRate = HVACInletMassFlowRate;
    state->dataFans->fans(1)->inletAirMassFlowRate = HVACInletMassFlowRate;
    state->dataFans->fans(1)->rhoAirStdInit = state->dataEnvrn->StdRhoAir;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->inletNodeNum).MassFlowRateMaxAvail = HVACInletMassFlowRate;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->outletNodeNum).MassFlowRateMax = HVACInletMassFlowRate;

    // primary air condition at outside air condition
    state->dataLoopNodes->Node(state->dataUnitVentilators->UnitVent(UnitVentNum).ATMixerPriNode).Temp = state->dataEnvrn->OutDryBulbTemp;
    state->dataLoopNodes->Node(state->dataUnitVentilators->UnitVent(UnitVentNum).ATMixerPriNode).HumRat = state->dataEnvrn->OutHumRat;
    state->dataLoopNodes->Node(state->dataUnitVentilators->UnitVent(UnitVentNum).ATMixerPriNode).Enthalpy = state->dataEnvrn->OutEnthalpy;

    // set UnitVent inlet condition to zone air node
    state->dataLoopNodes->Node(state->dataUnitVentilators->UnitVent(UnitVentNum).AirInNode).Temp =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Temp;
    state->dataLoopNodes->Node(state->dataUnitVentilators->UnitVent(UnitVentNum).AirInNode).HumRat =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).HumRat;
    state->dataLoopNodes->Node(state->dataUnitVentilators->UnitVent(UnitVentNum).AirInNode).Enthalpy =
        state->dataLoopNodes->Node(state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode).Enthalpy;

    state->dataUnitVentilators->UnitVent(1).ZonePtr = 1;
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataGlobal->SysSizingCalc = true;

    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputRequired = 5000.0;
    QZnReq = state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired;

    state->dataUnitVentilators->UnitVent(UnitVentNum).availSched->currentVal = 1.0;    // unit is always available
    state->dataUnitVentilators->UnitVent(UnitVentNum).fanAvailSched->currentVal = 1.0; // fan is always available
    state->dataUnitVentilators->UnitVent(UnitVentNum).minOASched->currentVal = 0.5;    // min OA fraction is always available

    // set secondary air mass flow rate to zero
    state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).MassFlowRate = 0.0;
    // simulate Unit Ventilator ZoneHVAC equipment
    SimUnitVentilator(
        *state, state->dataUnitVentilators->UnitVent(UnitVentNum).Name, ZoneNum, FirstHVACIteration, QUnitOut, LatOutputProvided, UnitVentNum);
    // apply mass conservation to determine secondary mass flow rate
    SecondaryAirMassFlowRate = state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).MassFlowRate;
    // check the terminal air mixer secondary air mass flow rate
    ASSERT_EQ(SecondaryAirMassFlowRate, state->dataLoopNodes->Node(state->dataSingleDuct->SysATMixer(1).SecInNode).MassFlowRate);
    // check the air mixer outlet air mass flow rate
    ATMixerOutletMassFlowRate = SecondaryAirMassFlowRate + PrimaryAirMassFlowRate;
    ASSERT_EQ(ATMixerOutletMassFlowRate, state->dataSingleDuct->SysATMixer(1).MixedAirMassFlowRate);
    // check the cooling output delivered is within 2.0 Watt of zone cooling load
    ASSERT_NEAR(QZnReq, QUnitOut, 0.001);
}
TEST_F(EnergyPlusFixture, AirTerminalSingleDuctMixer_GetInputDOASpecs)
{

    // This test was added for #6399 to confirm that the correct SysATMixer( ATMixerNum ).OARequirementsPtr
    // is found when searching Sizing:Zone objects. The defect is exposed when there are more Sizing:Zone objects than
    // DesignSpecification:OutdoorAir objects. In this test, there are 2 zones and one DSOA object.
    // In this test, the first mixer declares "DSOA 1" directly, and the second mixer leaves it blank so
    // it searches the sizing objects. Both mixers should find OARequirementsPtr = 1.

    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({

        "DesignSpecification:OutdoorAir,",
        "  DSOA 1,  !- Name",
        "  sum,                     !- Outdoor Air Method",
        "  0.0,                     !- Outdoor Air Flow per Person {m3/s-person}",
        "  0.0009,                  !- Outdoor Air Flow per Zone Floor Area {m3/s-m2}",
        "  0;                       !- Outdoor Air Flow per Zone {m3/s}",

        "Sizing:Zone,",
        " SPACE1-1,                 !- Zone or ZoneList Name",
        " SupplyAirTemperature,     !- Zone Cooling Design Supply Air Temperature Input Method",
        " 12.,                      !- Zone Cooling Design Supply Air Temperature{ C }",
        " ,                         !- Zone Cooling Design Supply Air Temperature Difference{ deltaC }",
        " SupplyAirTemperature,     !- Zone Heating Design Supply Air Temperature Input Method",
        " 50.,                      !- Zone Heating Design Supply Air Temperature{ C }",
        " ,                         !- Zone Heating Design Supply Air Temperature Difference{ deltaC }",
        " 0.008,                    !- Zone Cooling Design Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        " 0.008,                    !- Zone Heating Design Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        " DSOA 1,                   !- Design Specification Outdoor Air Object Name",
        " 0.0,                      !- Zone Heating Sizing Factor",
        " 0.0,                      !- Zone Cooling Sizing Factor",
        " DesignDay,                !- Cooling Design Air Flow Method",
        " 0,                        !- Cooling Design Air Flow Rate{ m3 / s }",
        " ,                         !- Cooling Minimum Air Flow per Zone Floor Area{ m3 / s - m2 }",
        " ,                         !- Cooling Minimum Air Flow{ m3 / s }",
        " ,                         !- Cooling Minimum Air Flow Fraction",
        " DesignDay,                !- Heating Design Air Flow Method",
        " 0,                        !- Heating Design Air Flow Rate{ m3 / s }",
        " ,                         !- Heating Maximum Air Flow per Zone Floor Area{ m3 / s - m2 }",
        " ,                         !- Heating Maximum Air Flow{ m3 / s }",
        " ;                         !- Heating Maximum Air Flow Fraction",

        "Sizing:Zone,",
        " SPACE1-2,                 !- Zone or ZoneList Name",
        " SupplyAirTemperature,     !- Zone Cooling Design Supply Air Temperature Input Method",
        " 12.,                      !- Zone Cooling Design Supply Air Temperature{ C }",
        " ,                         !- Zone Cooling Design Supply Air Temperature Difference{ deltaC }",
        " SupplyAirTemperature,     !- Zone Heating Design Supply Air Temperature Input Method",
        " 50.,                      !- Zone Heating Design Supply Air Temperature{ C }",
        " ,                         !- Zone Heating Design Supply Air Temperature Difference{ deltaC }",
        " 0.008,                    !- Zone Cooling Design Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        " 0.008,                    !- Zone Heating Design Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        " DSOA 1,                   !- Design Specification Outdoor Air Object Name",
        " 0.0,                      !- Zone Heating Sizing Factor",
        " 0.0,                      !- Zone Cooling Sizing Factor",
        " DesignDay,                !- Cooling Design Air Flow Method",
        " 0,                        !- Cooling Design Air Flow Rate{ m3 / s }",
        " ,                         !- Cooling Minimum Air Flow per Zone Floor Area{ m3 / s - m2 }",
        " ,                         !- Cooling Minimum Air Flow{ m3 / s }",
        " ,                         !- Cooling Minimum Air Flow Fraction",
        " DesignDay,                !- Heating Design Air Flow Method",
        " 0,                        !- Heating Design Air Flow Rate{ m3 / s }",
        " ,                         !- Heating Maximum Air Flow per Zone Floor Area{ m3 / s - m2 }",
        " ,                         !- Heating Maximum Air Flow{ m3 / s }",
        " ;                         !- Heating Maximum Air Flow Fraction",

        "AirTerminal:SingleDuct:Mixer,",
        "    SPACE1-1 DOAS Air Terminal,  !- Name",
        "    ZoneHVAC:PackagedTerminalAirConditioner,     !- ZoneHVAC Terminal Unit Object Type",
        "    SPACE1-1 PTAC,      !- ZoneHVAC Terminal Unit Name",
        "    SPACE1-1 Heat Pump Inlet,!- Terminal Unit Outlet Node Name",
        "    SPACE1-1 Air Terminal Mixer Primary Inlet,   !- Terminal Unit Primary Air Inlet Node Name",
        "    SPACE1-1 Air Terminal Mixer Secondary Inlet, !- Terminal Unit Secondary Air Inlet Node Name",
        "    InletSide,          !- Terminal Unit Connection Type",
        "    DSOA 1;             !- Design Specification Outdoor Air Object Name",

        "ZoneHVAC:AirDistributionUnit,",
        "    SPACE1-1 DOAS ATU,       !- Name",
        "    SPACE1-1 Heat Pump Inlet,!- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:Mixer,  !- Air Terminal Object Type",
        "    SPACE1-1 DOAS Air Terminal;  !- Air Terminal Name",

        "ZoneHVAC:EquipmentList,",
        "    SPACE1-1 Equipment,      !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE1-1 DOAS ATU,       !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction",
        "    ;                        !- Zone Equipment 1 Sequential Heating Fraction",

        "Zone,",
        "    SPACE1-1;                !- Name",

        "ZoneHVAC:EquipmentConnections,",
        "    SPACE1-1,                !- Zone Name",
        "    SPACE1-1 Equipment,      !- Zone Conditioning Equipment List Name",
        "    ,                        !- Zone Air Inlet Node or NodeList Name",
        "    SPACE1-1 Air Terminal Mixer Secondary Inlet,  !- Zone Air Exhaust Node or NodeList Name",
        "    SPACE1-1 Zone Air Node,  !- Zone Air Node Name",
        "    SPACE1-1 Return Outlet;  !- Zone Return Air Node Name",

        "AirTerminal:SingleDuct:Mixer,",
        "    SPACE1-2 DOAS Air Terminal,  !- Name",
        "    ZoneHVAC:PackagedTerminalAirConditioner,     !- ZoneHVAC Terminal Unit Object Type",
        "    SPACE1-2 PTAC,      !- ZoneHVAC Terminal Unit Name",
        "    SPACE1-2 Supply Inlet,!- Terminal Unit Outlet Node Name",
        "    SPACE1-2 Air Terminal Mixer Primary Inlet,   !- Terminal Unit Primary Air Inlet Node Name",
        "    SPACE1-2 Air Terminal Mixer Secondary Inlet, !- Terminal Unit Secondary Air Inlet Node Name",
        "    SupplySide,         !- Terminal Unit Connection Type",
        "    ;                   !- Design Specification Outdoor Air Object Name",

        "ZoneHVAC:AirDistributionUnit,",
        "    SPACE1-2 DOAS ATU,       !- Name",
        "    SPACE1-2 Supply Inlet,   !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:Mixer,  !- Air Terminal Object Type",
        "    SPACE1-2 DOAS Air Terminal;  !- Air Terminal Name",

        "ZoneHVAC:EquipmentList,",
        "    SPACE1-2 Equipment,      !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE1-2 DOAS ATU,       !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction",
        "    ;                        !- Zone Equipment 1 Sequential Heating Fraction",

        "Zone,",
        "    SPACE1-2;                !- Name",

        "ZoneHVAC:EquipmentConnections,",
        "    SPACE1-2,                !- Zone Name",
        "    SPACE1-2 Equipment,      !- Zone Conditioning Equipment List Name",
        "    SPACE1-2 Supply Inlet,   !- Zone Air Inlet Node or NodeList Name",
        "    SPACE1-2 Air Terminal Mixer Secondary Inlet,  !- Zone Air Exhaust Node or NodeList Name",
        "    SPACE1-2 Zone Air Node,  !- Zone Air Node Name",
        "    SPACE1-2 Return Outlet;  !- Zone Return Air Node Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    SizingManager::GetOARequirements(*state);
    SizingManager::GetZoneSizingInput(*state);
    GetZoneEquipmentData(*state);
    ZoneEquipmentManager::SetUpZoneSizingArrays(*state);
    GetZoneAirLoopEquipment(*state);
    GetATMixers(*state);

    ASSERT_EQ(2, state->dataSingleDuct->NumATMixers);
    EXPECT_EQ("SPACE1-1 DOAS AIR TERMINAL", state->dataSingleDuct->SysATMixer(1).Name);                 // single duct air terminal mixer name
    EXPECT_EQ((int)HVAC::MixerType::InletSide, (int)state->dataSingleDuct->SysATMixer(1).type);         // air terminal mixer connection type
    EXPECT_EQ("AIRTERMINAL:SINGLEDUCT:MIXER", state->dataDefineEquipment->AirDistUnit(1).EquipType(1)); // Air distribution unit equipment type
    EXPECT_EQ(1, state->dataSingleDuct->SysATMixer(1).OARequirementsPtr); // design spec OA pointer - for both mixers this pointer should be 1

    EXPECT_EQ("SPACE1-2 DOAS AIR TERMINAL", state->dataSingleDuct->SysATMixer(2).Name);                 // single duct air terminal mixer name
    EXPECT_EQ((int)HVAC::MixerType::SupplySide, (int)state->dataSingleDuct->SysATMixer(2).type);        // air terminal mixer connection type
    EXPECT_EQ("AIRTERMINAL:SINGLEDUCT:MIXER", state->dataDefineEquipment->AirDistUnit(2).EquipType(1)); // Air distribution unit equipment type
    // design spec OA pointer - for both mixers this pointer should be 1
    // before the fix, this was 2 which later caused an array bounds error
    EXPECT_EQ(1, state->dataSingleDuct->SysATMixer(2).OARequirementsPtr);
}

TEST_F(EnergyPlusFixture, AirTerminalSingleDuctMixer_SimFCU_ATMInletSideTest)
{

    std::string const idf_objects = delimited_string({

        "  Schedule:Compact,",
        "    FanAvailSched,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 16",
        "    1.0;                     !- Field 17",

        " ScheduleTypeLimits,",
        "     Fraction,                !- Name",
        "     0,                       !- Lower Limit Value",
        "     1,                       !- Upper Limit Value",
        "     CONTINUOUS;              !- Numeric Type",

        " Zone,",
        "     Zone One,                !- Name",
        "     0,                       !- Direction of Relative North {deg}",
        "     0,                       !- X Origin {m}",
        "     0,                       !- Y Origin {m}",
        "     0,                       !- Z Origin {m}",
        "     ,                        !- Type",
        "     1;                       !- Multiplier",

        " ZoneHVAC:FourPipeFanCoil,",
        "     FCU VarFan VarFluidFlow, !- Name",
        "     FanAvailSched,           !- Availability Schedule Name",
        "     VariableFanVariableFlow, !- Capacity Control Method",
        "     Autosize,                !- Maximum Supply Air Flow Rate {m3/s}",
        "     ,                        !- Low Speed Supply Air Flow Ratio",
        "     ,                        !- Medium Speed Supply Air Flow Ratio",
        "     Autosize,                !- Maximum Outdoor Air Flow Rate {m3/s}",
        "     ,                        !- Outdoor Air Schedule Name",
        "     Node 5,                  !- Air Inlet Node Name",
        "     Node 63,                 !- Air Outlet Node Name",
        "     ,                        !- Outdoor Air Mixer Object Type",
        "     ,                        !- Outdoor Air Mixer Name",
        "     Fan:VariableVolume,      !- Supply Air Fan Object Type",
        "     FCU VarFan,              !- Supply Air Fan Name",
        "     Coil:Cooling:Water,      !- Cooling Coil Object Type",
        "     FCU Cooling Coil,        !- Cooling Coil Name",
        "     Autosize,                !- Maximum Cold Water Flow Rate {m3/s}",
        "     ,                        !- Minimum Cold Water Flow Rate {m3/s}",
        "     ,                        !- Cooling Convergence Tolerance",
        "     Coil:Heating:Water,      !- Heating Coil Object Type",
        "     FCU Heating Coil,        !- Heating Coil Name",
        "     Autosize,                !- Maximum Hot Water Flow Rate {m3/s}",
        "     ,                        !- Minimum Hot Water Flow Rate {m3/s}",
        "     ,                        !- Heating Convergence Tolerance",
        "     ,                        !- Availability Manager List Name",
        "     ,                        !- Design Specification ZoneHVAC Sizing Object Name",
        "     ,                        !- Supply Air Fan Operating Mode Schedule Name",
        "     Autosize,                !- Minimum Supply Air Temperature in Cooling Mode {C}",
        "     Autosize;                !- Maximum Supply Air Temperature in Heating Mode {C}",

        " AirTerminal:SingleDuct:Mixer,",
        "     Inlet Side Mixer,        !- Name",
        "     ZoneHVAC:FourPipeFanCoil,!- ZoneHVAC Unit Object Type",
        "     FCU VarFan VarFluidFlow, !- ZoneHVAC Unit Object Name",
        "     Node 5,                  !- Mixer Outlet Node Name",
        "     Node 62,                 !- Mixer Primary Air Inlet Node Name",
        "     Node 64,                 !- Mixer Secondary Air Inlet Node Name",
        "     InletSide;               !- Mixer Connection Type",

        " ZoneHVAC:AirDistributionUnit,",
        "     ADU Inlet Side Mixer,    !- Name",
        "     Node 5,                  !- Air Distribution Unit Outlet Node Name",
        "     AirTerminal:SingleDuct:Mixer,  !- Air Terminal Object Type",
        "     Inlet Side Mixer;        !- Air Terminal Name",

        " ZoneHVAC:EquipmentList,",
        "     Zone one Equipment List, !- Name",
        "     SequentialLoad,          !- Load Distribution Scheme",
        "     ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "     ADU Inlet Side Mixer,    !- Zone Equipment 1 Name",
        "     1,                       !- Zone Equipment 1 Cooling Sequence",
        "     1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "     ,                        !- Zone Equipment 1 Sequential Cooling Fraction Schedule Name",
        "     ,                        !- Zone Equipment 1 Sequential Heating Fraction Schedule Name",
        "     ZoneHVAC:FourPipeFanCoil,!- Zone Equipment 2 Object Type",
        "     FCU VarFan VarFluidFlow, !- Zone Equipment 2 Name",
        "     2,                       !- Zone Equipment 2 Cooling Sequence",
        "     2;                       !- Zone Equipment 2 Heating or No-Load Sequence",

        " ZoneHVAC:EquipmentConnections,",
        "     Zone One,                !- Zone Name",
        "     Zone one Equipment List, !- Zone Conditioning Equipment List Name",
        "     Node 63,                 !- Zone Air Inlet Node or NodeList Name",
        "     Node 64,                 !- Zone Air Exhaust Node or NodeList Name",
        "     Zone one Air Node,       !- Zone Air Node Name",
        "     Node 61;                 !- Zone Return Air Node or NodeList Name",

        " Fan:VariableVolume,",
        "     FCU VarFan,              !- Name",
        "     FanAvailSched,           !- Availability Schedule Name",
        "     0.6045,                  !- Fan Total Efficiency",
        "     600.0,                   !- Pressure Rise {Pa}",
        "     Autosize,                !- Maximum Flow Rate {m3/s}",
        "     FixedFlowRate,           !- Fan Power Minimum Flow Rate Input Method",
        "     0,                       !- Fan Power Minimum Flow Fraction",
        "     0,                       !- Fan Power Minimum Air Flow Rate {m3/s}",
        "     0.93,                    !- Motor Efficiency",
        "     1,                       !- Motor In Airstream Fraction",
        "     0.040759894,             !- Fan Power Coefficient 1",
        "     0.08804497,              !- Fan Power Coefficient 2",
        "     -0.07292612,             !- Fan Power Coefficient 3",
        "     0.943739823,             !- Fan Power Coefficient 4",
        "     0,                       !- Fan Power Coefficient 5",
        "     Node 5,                  !- Air Inlet Node Name",
        "     FCU Fan Outlet Node,     !- Air Outlet Node Name",
        "     General;                 !- End-Use Subcategory",

        " Coil:Cooling:Water,",
        "     FCU Cooling Coil,        !- Name",
        "     FanAvailSched,           !- Availability Schedule Name",
        "     Autosize,                !- Design Water Flow Rate {m3/s}",
        "     Autosize,                !- Design Air Flow Rate {m3/s}",
        "     Autosize,                !- Design Inlet Water Temperature {C}",
        "     Autosize,                !- Design Inlet Air Temperature {C}",
        "     Autosize,                !- Design Outlet Air Temperature {C}",
        "     Autosize,                !- Design Inlet Air Humidity Ratio {kgWater/kgDryAir}",
        "     Autosize,                !- Design Outlet Air Humidity Ratio {kgWater/kgDryAir}",
        "     Node 66,                 !- Water Inlet Node Name",
        "     Node 68,                 !- Water Outlet Node Name",
        "     FCU Fan Outlet Node,     !- Air Inlet Node Name",
        "     FCU CCoil Outlet Node,   !- Air Outlet Node Name",
        "     SimpleAnalysis,          !- Type of Analysis",
        "     CrossFlow;               !- Heat Exchanger Configuration",

        " Coil:Heating:Water,",
        "     FCU Heating Coil,        !- Name",
        "     FanAvailSched,           !- Availability Schedule Name",
        "     Autosize,                !- U-Factor Times Area Value {W/K}",
        "     Autosize,                !- Maximum Water Flow Rate {m3/s}",
        "     Node 67,                 !- Water Inlet Node Name",
        "     Node 65,                 !- Water Outlet Node Name",
        "     FCU CCoil Outlet Node,   !- Air Inlet Node Name",
        "     Node 63,                 !- Air Outlet Node Name",
        "     UFactorTimesAreaAndDesignWaterFlowRate,  !- Performance Input Method",
        "     Autosize,                !- Rated Capacity {W}",
        "     82.2,                    !- Rated Inlet Water Temperature {C}",
        "     16.6,                    !- Rated Inlet Air Temperature {C}",
        "     71.1,                    !- Rated Outlet Water Temperature {C}",
        "     32.2,                    !- Rated Outlet Air Temperature {C}",
        "     0.5;                     !- Rated Ratio for Air and Water Convection",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    bool ErrorsFound(false);
    bool FirstHVACIteration(false);
    Real64 PrimaryAirMassFlowRate(0.0);
    Real64 SecondaryAirMassFlowRate(0.0);
    Real64 DesignHeatAirVolFlow(0.50);
    Real64 DesignCoolAirVolFlow(0.60);
    Real64 QUnitOut(0.0);
    Real64 QLatOut(0.0);
    Real64 QZnReq(0.0);
    int ZoneNum(1);
    int FanCoilNum(1);

    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    state->dataSize->CurZoneEqNum = 1;

    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = Psychrometrics::PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->OutBaroPress, 20.0, 0.0);
    state->dataWaterCoils->GetWaterCoilsInputFlag = true;
    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    GetZoneEquipmentData(*state);
    GetFanCoilUnits(*state);

    auto &thisFanCoil(state->dataFanCoilUnits->FanCoil(1));
    auto &thisATMixer(state->dataSingleDuct->SysATMixer(1));
    auto *thisFan = state->dataFans->fans(1);

    // get input test for terminal air single duct mixer on inlet side of PTAC
    ASSERT_EQ(1, state->dataSingleDuct->NumATMixers);
    EXPECT_EQ("INLET SIDE MIXER", thisATMixer.Name);                                                    // single duct air terminal mixer name
    EXPECT_EQ((int)HVAC::MixerType::InletSide, (int)thisATMixer.type);                                  // air terminal mixer connection type
    EXPECT_EQ("AIRTERMINAL:SINGLEDUCT:MIXER", state->dataDefineEquipment->AirDistUnit(1).EquipType(1)); // Air distribution unit equipment type
    EXPECT_EQ((int)HVAC::FanType::VAV, (int)thisFanCoil.fanType);
    EXPECT_EQ("COIL:COOLING:WATER", thisFanCoil.CCoilType);
    EXPECT_EQ("FCU COOLING COIL", thisFanCoil.CCoilName);
    EXPECT_EQ("COIL:HEATING:WATER", thisFanCoil.HCoilType);
    EXPECT_EQ("FCU HEATING COIL", thisFanCoil.HCoilName);

    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    state->dataSize->NumPltSizInput = 2;
    state->dataSize->PlantSizData.allocate(state->dataSize->NumPltSizInput);
    // chilled water coil
    auto &CWCoil(state->dataWaterCoils->WaterCoil(2));
    thisFanCoil.CCoilName_Index = 2;
    state->dataLoopNodes->Node(CWCoil.WaterInletNodeNum).Temp = 6.0;
    CWCoil.WaterPlantLoc.loopNum = 2;
    CWCoil.WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    CWCoil.WaterPlantLoc.branchNum = 1;
    CWCoil.WaterPlantLoc.compNum = 1;
    // hot water coil
    auto &HWCoil(state->dataWaterCoils->WaterCoil(1));
    thisFanCoil.HCoilName_Index = 1;
    state->dataLoopNodes->Node(HWCoil.WaterInletNodeNum).Temp = 60.0;
    HWCoil.WaterPlantLoc.loopNum = 1;
    HWCoil.WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    HWCoil.WaterPlantLoc.branchNum = 1;
    HWCoil.WaterPlantLoc.compNum = 1;
    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {

        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }
    // chilled water plant loop
    auto &CWLoop(state->dataPlnt->PlantLoop(2));
    CWLoop.Name = "ChilledWaterLoop";
    CWLoop.FluidName = "WATER";
    CWLoop.glycol = Fluid::GetWater(*state);
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = CWCoil.Name;
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type = DataPlant::PlantEquipmentType::CoilWaterCooling;
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = CWCoil.WaterInletNodeNum;
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = CWCoil.WaterOutletNodeNum;
    auto &CWLoopSizingData(state->dataSize->PlantSizData(2));
    // Chilled Water Loop
    CWLoop.PlantSizNum = 2;
    CWLoopSizingData.PlantLoopName = CWLoop.Name;
    CWLoopSizingData.DesVolFlowRate = 1.0;
    CWLoopSizingData.DeltaT = 5.6;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    // hot water plant loop
    auto &HWLoop(state->dataPlnt->PlantLoop(1));
    HWLoop.Name = "HotWaterLoop";
    HWLoop.FluidName = "WATER";
    HWLoop.glycol = Fluid::GetWater(*state);
    HWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = HWCoil.Name;
    HWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type = DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
    HWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = HWCoil.WaterInletNodeNum;
    HWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = HWCoil.WaterOutletNodeNum;
    auto &HWLoopSizingData(state->dataSize->PlantSizData(1));
    // Hot Water Loop
    HWLoop.PlantSizNum = 1;
    HWLoopSizingData.PlantLoopName = HWLoop.Name;
    HWLoopSizingData.DesVolFlowRate = 1.0;
    HWLoopSizingData.DeltaT = 10.0;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataGlobal->DoingSizing = true;
    state->dataHVACGlobal->TurnFansOff = false;
    state->dataHVACGlobal->TurnFansOn = true;
    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 21;
    state->dataGlobal->HourOfDay = 1;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    Sched::UpdateScheduleVals(*state);

    state->dataSize->ZoneEqSizing.allocate(1);
    auto &zoneEqSizing(state->dataSize->ZoneEqSizing(1));
    zoneEqSizing.SizingMethod.allocate(HVAC::NumOfSizingTypes);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;
    state->dataSize->ZoneSizingRunDone = true;

    state->dataSize->FinalZoneSizing.allocate(1);
    auto &finalZoneSizing(state->dataSize->FinalZoneSizing(1));
    finalZoneSizing.DesCoolVolFlow = DesignHeatAirVolFlow;
    finalZoneSizing.DesCoolMassFlow = finalZoneSizing.DesCoolVolFlow * state->dataEnvrn->StdRhoAir;
    finalZoneSizing.DesHeatVolFlow = DesignCoolAirVolFlow;
    finalZoneSizing.DesHeatMassFlow = finalZoneSizing.DesHeatVolFlow * state->dataEnvrn->StdRhoAir;
    finalZoneSizing.ZoneTempAtHeatPeak = 20.0;
    finalZoneSizing.ZoneRetTempAtHeatPeak = finalZoneSizing.ZoneTempAtHeatPeak;
    zoneEqSizing.ATMixerHeatPriDryBulb = 4.0;
    finalZoneSizing.ZoneHumRatAtHeatPeak = 0.075;
    zoneEqSizing.ATMixerHeatPriHumRat = 0.005;
    finalZoneSizing.ZoneTempAtCoolPeak = 24.0;
    finalZoneSizing.ZoneRetTempAtCoolPeak = finalZoneSizing.ZoneTempAtCoolPeak;
    zoneEqSizing.ATMixerCoolPriDryBulb = 30.0;
    finalZoneSizing.ZoneHumRatAtCoolPeak = 0.0075;
    zoneEqSizing.ATMixerCoolPriHumRat = 0.0095;
    finalZoneSizing.DesCoolLoad = 10000.0;
    finalZoneSizing.DesHeatLoad = 10000.0;
    finalZoneSizing.CoolDesTemp = 12.8;
    finalZoneSizing.CoolDesHumRat = 0.0085;
    finalZoneSizing.HeatDesTemp = 40.0;
    finalZoneSizing.HeatDesHumRat = 0.0075;

    // heating mode tests
    state->dataFanCoilUnits->CoolingLoad = false;
    state->dataFanCoilUnits->HeatingLoad = true;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    auto &zoneSysEnergyDemand(state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1));
    auto &zoneEquipConfig(state->dataZoneEquip->ZoneEquipConfig(1));

    // set zone air node conditions
    state->dataLoopNodes->Node(zoneEquipConfig.ZoneNode).Temp = 20.0;
    state->dataLoopNodes->Node(zoneEquipConfig.ZoneNode).HumRat = 0.0075;
    state->dataLoopNodes->Node(zoneEquipConfig.ZoneNode).Enthalpy = Psychrometrics::PsyHFnTdbW(
        state->dataLoopNodes->Node(zoneEquipConfig.ZoneNode).Temp, state->dataLoopNodes->Node(zoneEquipConfig.ZoneNode).HumRat);
    // primary air conditions
    state->dataEnvrn->OutDryBulbTemp = 5.0;
    state->dataEnvrn->OutHumRat = 0.005;
    state->dataEnvrn->OutEnthalpy = Psychrometrics::PsyHFnTdbW(state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutHumRat);
    // primary air condition set at outdoor air condition
    state->dataLoopNodes->Node(thisFanCoil.ATMixerPriNode).Temp = state->dataEnvrn->OutDryBulbTemp;
    state->dataLoopNodes->Node(thisFanCoil.ATMixerPriNode).HumRat = state->dataEnvrn->OutHumRat;
    state->dataLoopNodes->Node(thisFanCoil.ATMixerPriNode).Enthalpy = state->dataEnvrn->OutEnthalpy;
    // initialize air terminal mixer primary air mass flow rate
    PrimaryAirMassFlowRate = 0.1;
    state->dataLoopNodes->Node(thisFanCoil.ATMixerPriNode).MassFlowRate = PrimaryAirMassFlowRate;
    state->dataLoopNodes->Node(thisFanCoil.ATMixerPriNode).MassFlowRateMaxAvail = PrimaryAirMassFlowRate;
    // set secondary air (recirculating air) conditions to zone air node
    state->dataLoopNodes->Node(thisATMixer.SecInNode).Temp = state->dataLoopNodes->Node(zoneEquipConfig.ZoneNode).Temp;
    state->dataLoopNodes->Node(thisATMixer.SecInNode).HumRat = state->dataLoopNodes->Node(zoneEquipConfig.ZoneNode).HumRat;
    state->dataLoopNodes->Node(thisATMixer.SecInNode).Enthalpy = state->dataLoopNodes->Node(zoneEquipConfig.ZoneNode).Enthalpy;

    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataSize->ZoneEqFanCoil = true;

    // set predicted heating load
    zoneSysEnergyDemand.RemainingOutputReqToCoolSP = 0;
    zoneSysEnergyDemand.RemainingOutputReqToHeatSP = 4000.0;
    zoneSysEnergyDemand.RemainingOutputRequired = 4000.0;
    QZnReq = zoneSysEnergyDemand.RemainingOutputRequired;
    QUnitOut = 0.0;
    QLatOut = 0.0;

    InitFanCoilUnits(*state, FanCoilNum, ZoneNum);
    EXPECT_EQ(state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRateMinAvail, 0.0); // check init value
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    SecondaryAirMassFlowRate = state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRate - PrimaryAirMassFlowRate;
    // check results in heating mode operation
    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    EXPECT_NEAR(thisFanCoil.PLR, 0.18700, 0.0001); // Why was the precision on this 0.00001?
    // check mass flow rates
    EXPECT_NEAR(PrimaryAirMassFlowRate, 0.1, 0.0001); // user input
    EXPECT_NEAR(SecondaryAirMassFlowRate, 0.035129, 0.00003);
    EXPECT_NEAR(state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRate, thisFan->inletAirMassFlowRate, 0.000001);
    EXPECT_NEAR(state->dataLoopNodes->Node(thisFanCoil.ATMixerPriNode).MassFlowRate, 0.1, 0.000001);
    EXPECT_NEAR(state->dataLoopNodes->Node(thisFanCoil.ATMixerSecNode).MassFlowRate, 0.035129, 0.00003);
    EXPECT_NEAR(state->dataLoopNodes->Node(thisFanCoil.ATMixerOutNode).MassFlowRate, 0.135129, 0.00003);

    // set zone air node conditions
    state->dataLoopNodes->Node(zoneEquipConfig.ZoneNode).Temp = 24.0;
    state->dataLoopNodes->Node(zoneEquipConfig.ZoneNode).HumRat = 0.0085;
    state->dataLoopNodes->Node(zoneEquipConfig.ZoneNode).Enthalpy = Psychrometrics::PsyHFnTdbW(
        state->dataLoopNodes->Node(zoneEquipConfig.ZoneNode).Temp, state->dataLoopNodes->Node(zoneEquipConfig.ZoneNode).HumRat);
    // primary air conditions
    state->dataEnvrn->OutDryBulbTemp = 28.0;
    state->dataEnvrn->OutHumRat = 0.0095;
    state->dataEnvrn->OutEnthalpy = Psychrometrics::PsyHFnTdbW(state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutHumRat);
    // primary air condition set at outdoor air condition
    state->dataLoopNodes->Node(thisFanCoil.ATMixerPriNode).Temp = state->dataEnvrn->OutDryBulbTemp;
    state->dataLoopNodes->Node(thisFanCoil.ATMixerPriNode).HumRat = state->dataEnvrn->OutHumRat;
    state->dataLoopNodes->Node(thisFanCoil.ATMixerPriNode).Enthalpy = state->dataEnvrn->OutEnthalpy;
    // initialize air terminal mixer primary air mass flow rate
    PrimaryAirMassFlowRate = 0.2;
    state->dataLoopNodes->Node(thisFanCoil.ATMixerPriNode).MassFlowRate = PrimaryAirMassFlowRate;
    state->dataLoopNodes->Node(thisFanCoil.ATMixerPriNode).MassFlowRateMaxAvail = PrimaryAirMassFlowRate;

    state->dataSize->SysSizingRunDone = true;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataGlobal->SysSizingCalc = true;
    state->dataGlobal->BeginEnvrnFlag = true;
    // set predicted cooling load
    zoneSysEnergyDemand.RemainingOutputReqToHeatSP = 0.0;
    zoneSysEnergyDemand.RemainingOutputReqToCoolSP = -5000.0;
    zoneSysEnergyDemand.RemainingOutputRequired = -5000.0;
    QZnReq = zoneSysEnergyDemand.RemainingOutputRequired;
    QUnitOut = 0.0;
    QLatOut = 0.0;
    InitFanCoilUnits(*state, FanCoilNum, ZoneNum);
    EXPECT_EQ(state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRateMinAvail, 0.0); // check init value
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    SecondaryAirMassFlowRate = state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRate - PrimaryAirMassFlowRate;
    // check results in cooling mode operation
    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    // Also, tolerance was 0.00001, why?  Why is tolerance of less
    // than 1/10th or even 1/100th of a percent needed on anything?
    // Also, the tolerance of finding water coil UA is 0.001, how
    // can this be lower?
    EXPECT_NEAR(thisFanCoil.PLR, 0.76235, 0.001); // Was 0.78843

    // check mass flow rates
    EXPECT_NEAR(PrimaryAirMassFlowRate, 0.2, 0.000001);
    // Tolerance here was 0.000001, why?  Why is tolerance of less
    // than 1/10th or even 1/100th of a percent needed on anything?
    EXPECT_NEAR(SecondaryAirMassFlowRate, 0.350865, 0.001);
    EXPECT_NEAR(state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRate, thisFan->inletAirMassFlowRate, 0.000001);
    EXPECT_NEAR(state->dataLoopNodes->Node(thisFanCoil.ATMixerPriNode).MassFlowRate, 0.2, 0.0001);
    EXPECT_NEAR(state->dataLoopNodes->Node(thisFanCoil.ATMixerSecNode).MassFlowRate, 0.350865, 0.0005); // Was 0.369714
    EXPECT_NEAR(state->dataLoopNodes->Node(thisFanCoil.ATMixerOutNode).MassFlowRate, 0.550865, 0.0005); // Was 0.569714
}

TEST_F(EnergyPlusFixture, AirTerminalSingleDuctMixer_FCU_NightCycleTest)
{

    std::string const idf_objects = delimited_string({

        "  Schedule:Compact,",
        "    FanAvailSched,           !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 16",
        "    0.0;                     !- Field 17",

        " ScheduleTypeLimits,",
        "     Fraction,                !- Name",
        "     0,                       !- Lower Limit Value",
        "     1,                       !- Upper Limit Value",
        "     CONTINUOUS;              !- Numeric Type",

        " Zone,",
        "     Zone One,                !- Name",
        "     0,                       !- Direction of Relative North {deg}",
        "     0,                       !- X Origin {m}",
        "     0,                       !- Y Origin {m}",
        "     0,                       !- Z Origin {m}",
        "     ,                        !- Type",
        "     1;                       !- Multiplier",

        " ZoneHVAC:FourPipeFanCoil,",
        "     FCU VarFan VarFluidFlow, !- Name",
        "     FanAvailSched,           !- Availability Schedule Name",
        "     VariableFanVariableFlow, !- Capacity Control Method",
        "     Autosize,                !- Maximum Supply Air Flow Rate {m3/s}",
        "     ,                        !- Low Speed Supply Air Flow Ratio",
        "     ,                        !- Medium Speed Supply Air Flow Ratio",
        "     Autosize,                !- Maximum Outdoor Air Flow Rate {m3/s}",
        "     ,                        !- Outdoor Air Schedule Name",
        "     Node 5,                  !- Air Inlet Node Name",
        "     Node 63,                 !- Air Outlet Node Name",
        "     ,                        !- Outdoor Air Mixer Object Type",
        "     ,                        !- Outdoor Air Mixer Name",
        "     Fan:VariableVolume,      !- Supply Air Fan Object Type",
        "     FCU VarFan,              !- Supply Air Fan Name",
        "     Coil:Cooling:Water,      !- Cooling Coil Object Type",
        "     FCU Cooling Coil,        !- Cooling Coil Name",
        "     Autosize,                !- Maximum Cold Water Flow Rate {m3/s}",
        "     ,                        !- Minimum Cold Water Flow Rate {m3/s}",
        "     ,                        !- Cooling Convergence Tolerance",
        "     Coil:Heating:Water,      !- Heating Coil Object Type",
        "     FCU Heating Coil,        !- Heating Coil Name",
        "     Autosize,                !- Maximum Hot Water Flow Rate {m3/s}",
        "     ,                        !- Minimum Hot Water Flow Rate {m3/s}",
        "     ,                        !- Heating Convergence Tolerance",
        "     ,                        !- Availability Manager List Name",
        "     ,                        !- Design Specification ZoneHVAC Sizing Object Name",
        "     ,                        !- Supply Air Fan Operating Mode Schedule Name",
        "     Autosize,                !- Minimum Supply Air Temperature in Cooling Mode {C}",
        "     Autosize;                !- Maximum Supply Air Temperature in Heating Mode {C}",

        " AirTerminal:SingleDuct:Mixer,",
        "     Inlet Side Mixer,        !- Name",
        "     ZoneHVAC:FourPipeFanCoil,!- ZoneHVAC Unit Object Type",
        "     FCU VarFan VarFluidFlow, !- ZoneHVAC Unit Object Name",
        "     Node 5,                  !- Mixer Outlet Node Name",
        "     Node 62,                 !- Mixer Primary Air Inlet Node Name",
        "     Node 64,                 !- Mixer Secondary Air Inlet Node Name",
        "     InletSide;               !- Mixer Connection Type",

        " ZoneHVAC:AirDistributionUnit,",
        "     ADU Inlet Side Mixer,    !- Name",
        "     Node 5,                  !- Air Distribution Unit Outlet Node Name",
        "     AirTerminal:SingleDuct:Mixer,  !- Air Terminal Object Type",
        "     Inlet Side Mixer;        !- Air Terminal Name",

        " ZoneHVAC:EquipmentList,",
        "     Zone one Equipment List, !- Name",
        "     SequentialLoad,          !- Load Distribution Scheme",
        "     ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "     ADU Inlet Side Mixer,    !- Zone Equipment 1 Name",
        "     1,                       !- Zone Equipment 1 Cooling Sequence",
        "     1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "     ,                        !- Zone Equipment 1 Sequential Cooling Fraction Schedule Name",
        "     ,                        !- Zone Equipment 1 Sequential Heating Fraction Schedule Name",
        "     ZoneHVAC:FourPipeFanCoil,!- Zone Equipment 2 Object Type",
        "     FCU VarFan VarFluidFlow, !- Zone Equipment 2 Name",
        "     2,                       !- Zone Equipment 2 Cooling Sequence",
        "     2;                       !- Zone Equipment 2 Heating or No-Load Sequence",

        " ZoneHVAC:EquipmentConnections,",
        "     Zone One,                !- Zone Name",
        "     Zone one Equipment List, !- Zone Conditioning Equipment List Name",
        "     Node 63,                 !- Zone Air Inlet Node or NodeList Name",
        "     Node 64,                 !- Zone Air Exhaust Node or NodeList Name",
        "     Zone one Air Node,       !- Zone Air Node Name",
        "     Node 61;                 !- Zone Return Air Node or NodeList Name",

        " Fan:VariableVolume,",
        "     FCU VarFan,              !- Name",
        "     Constant-1.0,            !- Availability Schedule Name",
        "     0.6045,                  !- Fan Total Efficiency",
        "     600.0,                   !- Pressure Rise {Pa}",
        "     Autosize,                !- Maximum Flow Rate {m3/s}",
        "     FixedFlowRate,           !- Fan Power Minimum Flow Rate Input Method",
        "     0,                       !- Fan Power Minimum Flow Fraction",
        "     0,                       !- Fan Power Minimum Air Flow Rate {m3/s}",
        "     0.93,                    !- Motor Efficiency",
        "     1,                       !- Motor In Airstream Fraction",
        "     0.040759894,             !- Fan Power Coefficient 1",
        "     0.08804497,              !- Fan Power Coefficient 2",
        "     -0.07292612,             !- Fan Power Coefficient 3",
        "     0.943739823,             !- Fan Power Coefficient 4",
        "     0,                       !- Fan Power Coefficient 5",
        "     Node 5,                  !- Air Inlet Node Name",
        "     FCU Fan Outlet Node,     !- Air Outlet Node Name",
        "     General;                 !- End-Use Subcategory",

        " Coil:Cooling:Water,",
        "     FCU Cooling Coil,        !- Name",
        "     Constant-1.0,            !- Availability Schedule Name",
        "     Autosize,                !- Design Water Flow Rate {m3/s}",
        "     Autosize,                !- Design Air Flow Rate {m3/s}",
        "     Autosize,                !- Design Inlet Water Temperature {C}",
        "     Autosize,                !- Design Inlet Air Temperature {C}",
        "     Autosize,                !- Design Outlet Air Temperature {C}",
        "     Autosize,                !- Design Inlet Air Humidity Ratio {kgWater/kgDryAir}",
        "     Autosize,                !- Design Outlet Air Humidity Ratio {kgWater/kgDryAir}",
        "     Node 66,                 !- Water Inlet Node Name",
        "     Node 68,                 !- Water Outlet Node Name",
        "     FCU Fan Outlet Node,     !- Air Inlet Node Name",
        "     FCU CCoil Outlet Node,   !- Air Outlet Node Name",
        "     SimpleAnalysis,          !- Type of Analysis",
        "     CrossFlow;               !- Heat Exchanger Configuration",

        " Coil:Heating:Water,",
        "     FCU Heating Coil,        !- Name",
        "     Constant-1.0,            !- Availability Schedule Name",
        "     Autosize,                !- U-Factor Times Area Value {W/K}",
        "     Autosize,                !- Maximum Water Flow Rate {m3/s}",
        "     Node 67,                 !- Water Inlet Node Name",
        "     Node 65,                 !- Water Outlet Node Name",
        "     FCU CCoil Outlet Node,   !- Air Inlet Node Name",
        "     Node 63,                 !- Air Outlet Node Name",
        "     UFactorTimesAreaAndDesignWaterFlowRate,  !- Performance Input Method",
        "     Autosize,                !- Rated Capacity {W}",
        "     82.2,                    !- Rated Inlet Water Temperature {C}",
        "     16.6,                    !- Rated Inlet Air Temperature {C}",
        "     71.1,                    !- Rated Outlet Water Temperature {C}",
        "     32.2,                    !- Rated Outlet Air Temperature {C}",
        "     0.5;                     !- Rated Ratio for Air and Water Convection",

        " AvailabilityManagerAssignmentList,",
        "    Zone Availability Manager,     !- Name",
        "    AvailabilityManager:NightCycle,!- Availability Manager 1 Object Type",
        "    NightCycle AvailMgr;           !- Availability Manager 1 Name",

        " AvailabilityManager:NightCycle,",
        "    NightCycle AvailMgr,     !- Name",
        "    Constant-1.0,            !- Applicability Schedule Name",
        "    FanAvailSched,           !- Fan Schedule Name",
        "    CycleOnControlZone,      !- Control Type",
        "    0.2,                     !- Thermostat Tolerance {deltaC}",
        "    ThermostatWithMinimumRunTime, !- Cycling Run Time Control Type",
        "    300.0,                   !- Cycling Run Time {s}",
        "    Zone one;                !- Control Zone or Zone List Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    bool ErrorsFound(false);
    bool FirstHVACIteration(false);
    Real64 PrimaryAirMassFlowRate(0.0);
    Real64 DesignHeatAirVolFlow(0.50);
    Real64 DesignCoolAirVolFlow(0.60);
    Real64 QUnitOut(0.0);
    Real64 QLatOut(0.0);
    Real64 QZnReq(0.0);
    int ZoneNum(1);
    int FanCoilNum(1);

    state->dataSize->CurZoneEqNum = 1;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = Psychrometrics::PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->OutBaroPress, 20.0, 0.0);
    state->dataWaterCoils->GetWaterCoilsInputFlag = true;
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);

    GetZoneEquipmentData(*state);
    GetFanCoilUnits(*state);
    Avail::GetSysAvailManagerInputs(*state);

    auto &thisFanCoil(state->dataFanCoilUnits->FanCoil(1));
    auto &thisATMixer(state->dataSingleDuct->SysATMixer(1));
    auto &thisAvaiManager(state->dataAvail->NightCycleData(1));

    // get input test for terminal air single duct mixer on inlet side of PTAC
    ASSERT_EQ(1, state->dataSingleDuct->NumATMixers);
    EXPECT_EQ("INLET SIDE MIXER", thisATMixer.Name);                                                    // single duct air terminal mixer name
    EXPECT_EQ((int)HVAC::MixerType::InletSide, (int)thisATMixer.type);                                  // air terminal mixer connection type
    EXPECT_EQ("AIRTERMINAL:SINGLEDUCT:MIXER", state->dataDefineEquipment->AirDistUnit(1).EquipType(1)); // Air distribution unit equipment type
    EXPECT_EQ((int)HVAC::FanType::VAV, (int)thisFanCoil.fanType);
    EXPECT_EQ("COIL:COOLING:WATER", thisFanCoil.CCoilType);
    EXPECT_EQ("FCU COOLING COIL", thisFanCoil.CCoilName);
    EXPECT_EQ("COIL:HEATING:WATER", thisFanCoil.HCoilType);
    EXPECT_EQ("FCU HEATING COIL", thisFanCoil.HCoilName);
    EXPECT_EQ("NIGHTCYCLE AVAILMGR", thisAvaiManager.Name);
    EXPECT_EQ((int)Avail::ManagerType::NightCycle, (int)thisAvaiManager.type);

    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    state->dataSize->NumPltSizInput = 2;
    state->dataSize->PlantSizData.allocate(state->dataSize->NumPltSizInput);
    // chilled water coil
    auto &CWCoil(state->dataWaterCoils->WaterCoil(2));
    thisFanCoil.CCoilName_Index = 2;
    state->dataLoopNodes->Node(CWCoil.WaterInletNodeNum).Temp = 6.0;
    CWCoil.WaterPlantLoc.loopNum = 2;
    CWCoil.WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    CWCoil.WaterPlantLoc.branchNum = 1;
    CWCoil.WaterPlantLoc.compNum = 1;
    // hot water coil
    auto &HWCoil(state->dataWaterCoils->WaterCoil(1));
    thisFanCoil.HCoilName_Index = 1;
    state->dataLoopNodes->Node(HWCoil.WaterInletNodeNum).Temp = 60.0;
    HWCoil.WaterPlantLoc.loopNum = 1;
    HWCoil.WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    HWCoil.WaterPlantLoc.branchNum = 1;
    HWCoil.WaterPlantLoc.compNum = 1;
    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {

        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }
    // chilled water plant loop
    auto &CWLoop(state->dataPlnt->PlantLoop(2));
    CWLoop.Name = "ChilledWaterLoop";
    CWLoop.FluidName = "WATER";
    CWLoop.glycol = Fluid::GetWater(*state);
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = CWCoil.Name;
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type = DataPlant::PlantEquipmentType::CoilWaterCooling;
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = CWCoil.WaterInletNodeNum;
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = CWCoil.WaterOutletNodeNum;
    auto &CWLoopSizingData(state->dataSize->PlantSizData(2));
    // Chilled Water Loop
    CWLoop.PlantSizNum = 2;
    CWLoopSizingData.PlantLoopName = CWLoop.Name;
    CWLoopSizingData.DesVolFlowRate = 1.0;
    CWLoopSizingData.DeltaT = 5.6;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    // hot water plant loop
    auto &HWLoop(state->dataPlnt->PlantLoop(1));
    HWLoop.Name = "HotWaterLoop";
    HWLoop.FluidName = "WATER";
    HWLoop.glycol = Fluid::GetWater(*state);
    HWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = HWCoil.Name;
    HWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type = DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
    HWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = HWCoil.WaterInletNodeNum;
    HWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = HWCoil.WaterOutletNodeNum;
    auto &HWLoopSizingData(state->dataSize->PlantSizData(1));
    // Hot Water Loop
    HWLoop.PlantSizNum = 1;
    HWLoopSizingData.PlantLoopName = HWLoop.Name;
    HWLoopSizingData.DesVolFlowRate = 1.0;
    HWLoopSizingData.DeltaT = 10.0;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataGlobal->DoingSizing = true;
    state->dataHVACGlobal->TurnFansOff = false;
    state->dataHVACGlobal->TurnFansOn = false;
    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 21;
    state->dataGlobal->HourOfDay = 1;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    Sched::UpdateScheduleVals(*state);

    state->dataSize->ZoneEqSizing.allocate(1);
    auto &zoneEqSizing(state->dataSize->ZoneEqSizing(1));
    zoneEqSizing.SizingMethod.allocate(HVAC::NumOfSizingTypes);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;
    state->dataSize->ZoneSizingRunDone = true;

    state->dataSize->FinalZoneSizing.allocate(1);
    auto &finalZoneSizing(state->dataSize->FinalZoneSizing(1));
    finalZoneSizing.DesCoolVolFlow = DesignHeatAirVolFlow;
    finalZoneSizing.DesCoolMassFlow = finalZoneSizing.DesCoolVolFlow * state->dataEnvrn->StdRhoAir;
    finalZoneSizing.DesHeatVolFlow = DesignCoolAirVolFlow;
    finalZoneSizing.DesHeatMassFlow = finalZoneSizing.DesHeatVolFlow * state->dataEnvrn->StdRhoAir;
    finalZoneSizing.ZoneTempAtHeatPeak = 20.0;
    finalZoneSizing.ZoneRetTempAtHeatPeak = finalZoneSizing.ZoneTempAtHeatPeak;
    zoneEqSizing.ATMixerHeatPriDryBulb = 4.0;
    finalZoneSizing.ZoneHumRatAtHeatPeak = 0.075;
    zoneEqSizing.ATMixerHeatPriHumRat = 0.005;
    finalZoneSizing.ZoneTempAtCoolPeak = 24.0;
    finalZoneSizing.ZoneRetTempAtCoolPeak = finalZoneSizing.ZoneTempAtCoolPeak;
    zoneEqSizing.ATMixerCoolPriDryBulb = 30.0;
    finalZoneSizing.ZoneHumRatAtCoolPeak = 0.0075;
    zoneEqSizing.ATMixerCoolPriHumRat = 0.0095;
    finalZoneSizing.DesCoolLoad = 10000.0;
    finalZoneSizing.DesHeatLoad = 10000.0;
    finalZoneSizing.CoolDesTemp = 12.8;
    finalZoneSizing.CoolDesHumRat = 0.0085;
    finalZoneSizing.HeatDesTemp = 40.0;
    finalZoneSizing.HeatDesHumRat = 0.0075;

    // heating mode tests
    state->dataFanCoilUnits->CoolingLoad = false;
    state->dataFanCoilUnits->HeatingLoad = true;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    auto &zoneSysEnergyDemand(state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1));
    auto &zoneEquipConfig(state->dataZoneEquip->ZoneEquipConfig(1));

    // set zone air node conditions
    state->dataLoopNodes->Node(zoneEquipConfig.ZoneNode).Temp = 20.0;
    state->dataLoopNodes->Node(zoneEquipConfig.ZoneNode).HumRat = 0.0075;
    state->dataLoopNodes->Node(zoneEquipConfig.ZoneNode).Enthalpy = Psychrometrics::PsyHFnTdbW(
        state->dataLoopNodes->Node(zoneEquipConfig.ZoneNode).Temp, state->dataLoopNodes->Node(zoneEquipConfig.ZoneNode).HumRat);
    // primary air conditions
    state->dataEnvrn->OutDryBulbTemp = 5.0;
    state->dataEnvrn->OutHumRat = 0.005;
    state->dataEnvrn->OutEnthalpy = Psychrometrics::PsyHFnTdbW(state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutHumRat);
    // primary air condition set at outdoor air condition
    state->dataLoopNodes->Node(thisFanCoil.ATMixerPriNode).Temp = state->dataEnvrn->OutDryBulbTemp;
    state->dataLoopNodes->Node(thisFanCoil.ATMixerPriNode).HumRat = state->dataEnvrn->OutHumRat;
    state->dataLoopNodes->Node(thisFanCoil.ATMixerPriNode).Enthalpy = state->dataEnvrn->OutEnthalpy;
    // initialize air terminal mixer primary air mass flow rate
    PrimaryAirMassFlowRate = 0.1;
    state->dataLoopNodes->Node(thisFanCoil.ATMixerPriNode).MassFlowRate = PrimaryAirMassFlowRate;
    state->dataLoopNodes->Node(thisFanCoil.ATMixerPriNode).MassFlowRateMaxAvail = PrimaryAirMassFlowRate;
    // set secondary air (recirculating air) conditions to zone air node
    state->dataLoopNodes->Node(thisATMixer.SecInNode).Temp = state->dataLoopNodes->Node(zoneEquipConfig.ZoneNode).Temp;
    state->dataLoopNodes->Node(thisATMixer.SecInNode).HumRat = state->dataLoopNodes->Node(zoneEquipConfig.ZoneNode).HumRat;
    state->dataLoopNodes->Node(thisATMixer.SecInNode).Enthalpy = state->dataLoopNodes->Node(zoneEquipConfig.ZoneNode).Enthalpy;

    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataSize->ZoneEqFanCoil = true;
    // check availability manager Night Cycle parameters
    EXPECT_EQ((int)Avail::CyclingRunTimeControl::ThermostatWithMinimumRunTime, (int)state->dataAvail->NightCycleData(1).cyclingRunTimeControl);
    EXPECT_EQ((int)Avail::Status::NoAction, (int)state->dataAvail->NightCycleData(1).availStatus);

    // set predicted heating load
    zoneSysEnergyDemand.RemainingOutputReqToCoolSP = 4000.0;
    zoneSysEnergyDemand.RemainingOutputReqToHeatSP = 4000.0;
    zoneSysEnergyDemand.RemainingOutputRequired = 4000.0;
    QZnReq = zoneSysEnergyDemand.RemainingOutputRequired;
    QUnitOut = 0.0;
    QLatOut = 0.0;
    InitFanCoilUnits(*state, FanCoilNum, ZoneNum);
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    // check results when the fan coil unit is not available
    EXPECT_NEAR(0.0, QUnitOut, 0.1); // fan coil unit is off
    EXPECT_NEAR(thisFanCoil.PLR, 0.0, 0.00001);
    EXPECT_NEAR(state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRate, 0.0, 0.000001);

    int SysAvailNum = 1;
    int PriAirSysNum = 0;
    int constexpr ZoneEquipType = 1;
    int constexpr CompNum = 1;
    // current time is within the run time period, starting time is less than stopping time
    state->dataGlobal->SimTimeSteps = 0;
    state->dataAvail->ZoneComp(1).ZoneCompAvailMgrs(1).StartTime = 0.0;
    state->dataAvail->ZoneComp(1).ZoneCompAvailMgrs(1).StopTime = 4.0;
    state->dataAvail->NightCycleData(1).availStatus = Avail::Status::NoAction;
    // run CalcNCycSysAvailMgr to the availability of the fan coil unit on
    Avail::Status availStatus = Avail::CalcNCycSysAvailMgr(*state, SysAvailNum, PriAirSysNum, ZoneEquipType, CompNum);
    // check that the NightCycle has turned on the equipment
    EXPECT_EQ((int)Avail::Status::CycleOn, (int)availStatus);
    EXPECT_EQ((int)Avail::Status::CycleOn, (int)state->dataAvail->NightCycleData(1).availStatus);
    // set zone equipment is CyclOn based on night cycle manager status
    if (state->dataAvail->NightCycleData(1).availStatus == Avail::Status::CycleOn) {
        state->dataHVACGlobal->TurnFansOn = true;
        state->dataHVACGlobal->TurnFansOff = false;
    }
    InitFanCoilUnits(*state, FanCoilNum, ZoneNum);
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    EXPECT_NEAR(QZnReq, QUnitOut, 3.0);
    EXPECT_NEAR(thisFanCoil.PLR, 0.187, 0.001);
    EXPECT_NEAR(state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRate,
                thisFanCoil.PLR * state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRateMax,
                0.000001);
}

} // namespace EnergyPlus
