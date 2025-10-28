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

// EnergyPlus::HVACVariableRefrigerantFlow unit tests

// Google test headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/OutdoorAirUnit.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SteamCoils.hh>
#include <EnergyPlus/WaterCoils.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::Curve;
using namespace EnergyPlus::DataEnvironment;
using namespace EnergyPlus::DataHeatBalFanSys;
using namespace EnergyPlus::DataHeatBalance;
using namespace EnergyPlus::DataLoopNode;
using namespace EnergyPlus::DataPlant;
using namespace EnergyPlus::DataSizing;
using namespace EnergyPlus::DataZoneEnergyDemands;
using namespace EnergyPlus::DataZoneEquipment;
using namespace EnergyPlus::Fans;
using namespace EnergyPlus::HeatBalanceManager;
using namespace OutputReportPredefined;
using namespace EnergyPlus::Psychrometrics;
using namespace EnergyPlus::SteamCoils;
using namespace EnergyPlus::WaterCoils;

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, OutdoorAirUnit_AutoSize)
{

    bool ErrorsFound(false);       // function returns true on error
    bool FirstHVACIteration(true); // simulate the first pass through HVAC simulation, use false for next iteration
    int OAUnitNum(1);              // index to ZoneHVAC:OutdoorAirUnit
    int EquipPtr(1);               // index to equipment list
    int CurZoneNum(1);             // index to zone
    Real64 SysOutputProvided(0.0); // function returns sensible capacity [W]
    Real64 LatOutputProvided(0.0); // function returns latent capacity [W]
    int ZoneInletNode(0);

    std::string const idf_objects = delimited_string({
        "Output:Diagnostics, DisplayExtraWarnings;",
        " ",
        "Zone,",
        "  SPACE1-1,                !- Name",
        "  0,                       !- Direction of Relative North {deg}",
        "  0,                       !- X Origin {m}",
        "  0,                       !- Y Origin {m}",
        "  0,                       !- Z Origin {m}",
        "  1,                       !- Type",
        "  1,                       !- Multiplier",
        "  2.5,                     !- Ceiling Height {m}",
        "  250.0;                   !- Volume {m3}",
        " ",
        "ZoneHVAC:EquipmentConnections,",
        "  SPACE1-1,                !- Zone Name",
        "  SPACE1-1 Eq,             !- Zone Conditioning Equipment List Name",
        "  Zone Eq Outlet Node,     !- Zone Air Inlet Node or NodeList Name",
        "  Zone Eq Exhaust Node,    !- Zone Air Exhaust Node or NodeList Name",
        "  SPACE1-1 Node,           !- Zone Air Node Name",
        "  SPACE1-1 Out Node;       !- Zone Return Air Node Name", // not used anywhere else in the example file
        " ",
        "ZoneHVAC:EquipmentList,",
        "  SPACE1-1 Eq,             !- Name",
        "  SequentialLoad,          !- Load Distribution Scheme",
        "  ZoneHVAC:OutdoorAirUnit, !- Zone Equipment 1 Object Type",
        "  Zone1OutAir,             !- Zone Equipment 1 Name",
        "  1,                       !- Zone Equipment 1 Cooling Sequence",
        "  1;                       !- Zone Equipment 1 Heating or No-Load Sequence",
        " ",
        "ZoneHVAC:OutdoorAirUnit,",
        "  Zone1OutAir,             !- Name",
        "  AvailSched,              !- Availability Schedule Name",
        "  SPACE1-1,                !- Zone Name",
        "  autosize,                !- Outdoor Air Flow Rate{ m3 / s }",
        "  AvailSched,              !- Outdoor Air Schedule Name",
        "  Zone1OAUFan,             !- Supply Fan Name",
        "  DrawThrough,             !- Supply Fan Placement",
        "  Zone1OAUextFan,          !- Exhaust Fan Name",
        "  autosize,                !- Exhaust Air Flow Rate{ m3 / s }",
        "  AvailSched,              !- Exhaust Air Schedule Name",
        "  TemperatureControl,      !- Unit Control Type",
        "  OAUHiCtrlTemp,           !- High Air Control Temperature Schedule Name",
        "  OAULoCtrlTemp,           !- Low Air Control Temperature Schedule Name",
        "  Outside Air Inlet Node 1, !- Outdoor Air Node Name",
        "  Zone Eq Outlet Node,     !- AirOutlet Node Name",
        "  Zone Eq Inlet Node,      !- AirInlet Node Name",
        "  Zone Eq Inlet Node,      !- Supply Fan Outlet Node Name",
        "  Zone1OAEQLIST;           !- Outdoor Air Unit List Name",
        " ",
        "Fan:ConstantVolume,",
        "  Zone1OAUFan,             !- Name",
        "   AvailSched,             !- Availability Schedule Name",
        "   0.5,                    !- Fan Total Efficiency",
        "   75.0,                   !- Pressure Rise{ Pa }",
        "   autosize,               !- Maximum Flow Rate{ m3 / s }",
        "   0.9,                    !- Motor Efficiency",
        "   1.0,                    !- Motor In Airstream Fraction",
        "   Heat Coil Outlet Node,  !- Air Inlet Node Name",
        "   Zone Eq Inlet Node;     !- Air Outlet Node Name",
        " ",
        "Fan:ConstantVolume,",
        "   Zone1OAUextFan,         !- Name",
        "   AvailSched,             !- Availability Schedule Name",
        "   0.5,                    !- Fan Total Efficiency",
        "   75.0,                   !- Pressure Rise{ Pa }",
        "   autosize,               !- Maximum Flow Rate{ m3 / s }",
        "   0.9,                    !- Motor Efficiency",
        "   1.0,                    !- Motor In Airstream Fraction",
        "   Zone Eq Exhaust Node,   !- Air Inlet Node Name",
        "   OutAir1;                !- Air Outlet Node Name",
        " ",
        "ZoneHVAC:OutdoorAirUnit:EquipmentList,",
        "  Zone1OAEQLIST,           !- Name",
        "  CoilSystem:Cooling:DX,   !- Component 1 Object Type",
        "  DX Cooling Coil System 1, !- Component 1 Name",
        "  Coil:Heating:Electric,   !- Component 2 Object Type",
        "  Zone1OAUHeatingCoil;     !- Component 2 Name",
        " ",
        "CoilSystem:Cooling:DX,",
        "  DX Cooling Coil System 1, !- Name",
        "  AvailSched,              !- Availability Schedule Name",
        "  Zone Eq Outlet Node,     !- DX Cooling Coil System Inlet Node Name",
        "  Heat Coil Inlet Node,    !- DX Cooling Coil System Outlet Node Name",
        "  Heat Coil Inlet Node,    !- DX Cooling Coil System Sensor Node Name",
        "  Coil:Cooling:DX:SingleSpeed, !- Cooling Coil Object Type",
        "  ACDXCoil 1;              !- Cooling Coil Name",
        " ",
        "Coil:Cooling:DX:SingleSpeed,",
        "  ACDXCoil 1,              !- Name",
        "  AvailSched,              !- Availability Schedule Name",
        "  autosize,                !- Gross Rated Total Cooling Capacity{ W }",
        "  autosize,                !- Gross Rated Sensible Heat Ratio",
        "  3.0,                     !- Gross Rated Cooling COP{ W / W }",
        "  autosize,                !- Rated Air Flow Rate{ m3 / s }",
        "  ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "  ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "  Outside Air Inlet Node 1, !- Air Inlet Node Name",
        "  Heat Coil Inlet Node,    !- Air Outlet Node Name",
        "  BiQuadCurve,             !- Total Cooling Capacity Function of Temperature Curve Name",
        "  QuadraticCurve,          !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "  BiQuadCurve,             !- Energy Input Ratio Function of Temperature Curve Name",
        "  QuadraticCurve,          !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "  QuadraticCurve;          !- Part Load Fraction Correlation Curve Name",
        " ",
        "Coil:Heating:Electric,",
        "  Zone1OAUHeatingCoil, !- Name",
        "  AvailSched, !- Availability Schedule Name",
        "  0.99, !- Efficiency",
        "  autosize, !- Nominal Capacity{ W }",
        "  Heat Coil Inlet Node, !- Air Inlet Node Name",
        "  Heat Coil Outlet Node;               !- Air Outlet Node Name",
        " ",
        "OutdoorAir:NodeList,",
        "  OutsideAirInletNodes;     !- Node or NodeList Name 1",
        " ",
        "NodeList,",
        "  OutsideAirInletNodes, !- Name",
        "  Outside Air Inlet Node 1; !- Node 1 Name",
        " ",
        " ",
        "ScheduleTypeLimits,",
        "  Any Number;              !- Name",
        " ",
        "Schedule:Compact,",
        "  AvailSched,           !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 13",
        "  For: AllDays,            !- Field 14",
        "  Until: 24:00,1.0;        !- Field 15",
        " ",
        "Schedule:Compact,",
        "  OAULoCtrlTemp, !- Name",
        "  Any Number, !- Schedule Type Limits Name",
        "  Through: 12/31, !- Field 1",
        "  For: AllDays, !- Field 2",
        "  Until: 24:00, 10;         !- Field 3",
        " ",
        "Schedule:Compact,",
        "  OAUHiCtrlTemp, !- Name",
        "  Any Number, !- Schedule Type Limits Name",
        "  Through: 12/31, !- Field 1",
        "  For: AllDays, !- Field 2",
        "  Until: 24:00, 15;         !- Field 3",
        " ",
        "Curve:Biquadratic,",
        "  BiQuadCurve,             !- Name",
        "  1.0,                     !- Coefficient1 Constant",
        "  0.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.0,                     !- Coefficient4 y",
        "  0.0,                     !- Coefficient5 y**2",
        "  0.0,                     !- Coefficient6 x*y",
        "  5,                       !- Minimum Value of x",
        "  36,                      !- Maximum Value of x",
        "  5,                       !- Minimum Value of y",
        "  36,                      !- Maximum Value of y",
        "  ,                        !- Minimum Curve Output",
        "  ,                        !- Maximum Curve Output",
        "  Temperature,             !- Input Unit Type for X",
        "  Temperature,             !- Input Unit Type for Y",
        "  Dimensionless;           !- Output Unit Type",
        " ",
        "Curve:Quadratic,",
        "  QuadraticCurve,          !- Name",
        "  1.0,                     !- Coefficient1 Constant",
        "  0.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.0,                     !- Minimum Value of x",
        "  1.5,                     !- Maximum Value of x",
        "  ,                        !- Minimum Curve Output",
        "  ,                        !- Maximum Curve Output",
        "  Dimensionless,           !- Input Unit Type for X",
        "  Dimensionless;           !- Output Unit Type",
        " ",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataGlobal->CurrentTime = 0.25;
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataSize->CurZoneEqNum = 1;
    state->dataEnvrn->OutBaroPress = 101325;            // sea level
    state->dataZoneEquip->ZoneEquipInputsFilled = true; // denotes zone equipment has been read in
    state->dataEnvrn->StdRhoAir = PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->OutBaroPress, 20.0, 0.0);
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).DesignSizeFromParent = false;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod.allocate(25);
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod(HVAC::SystemAirflowSizing) = DataSizing::SupplyAirFlowRate;

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);

    GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);

    GetZoneEquipmentData(*state); // read equipment list and connections

    // Test coil sizing

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(CurZoneNum).RemainingOutputRequired = 0.0; // set load = 0
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(CurZoneNum).RemainingOutputReqToCoolSP = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(CurZoneNum).RemainingOutputReqToHeatSP = 0.0;

    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow = 0.5;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).MinOA = 0.5;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneRetTempAtCoolPeak = 26.66667;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneTempAtCoolPeak = 26.66667;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneHumRatAtCoolPeak = 0.01117049470250416; // AHRI condition at 80 F db / 67 F wb
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDDNum = 1;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).TimeStepNumAtCoolMax = 1;
    state->dataSize->DesDayWeath.allocate(1);
    state->dataSize->DesDayWeath(1).Temp.allocate(1);
    state->dataSize->DesDayWeath(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDDNum)
        .Temp(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).TimeStepNumAtCoolMax) = 35.0;

    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDesTemp = 13.1;                   // 55.58 F
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDesHumRat = 0.009297628698818194; // humrat at 12.77777 C db / 12.6 C wb

    ZoneInletNode = OutdoorAirUnit::GetOutdoorAirUnitZoneInletNode(*state, OAUnitNum);

    // schedule values will get reset to 0 if initialized before GetInput
    Sched::GetSchedule(*state, "AVAILSCHED")->currentVal = 1.0;    // enable the VRF condenser
    Sched::GetSchedule(*state, "OAULOCTRLTEMP")->currentVal = 1.0; // enable the terminal unit
    Sched::GetSchedule(*state, "OAUHICTRLTEMP")->currentVal = 1.0; // turn on fan
    int EAFanInletNode = state->dataFans->fans(2)->inletNodeNum;
    state->dataLoopNodes->Node(EAFanInletNode).MassFlowRate = 0.60215437;         // zone exhaust flow rate
    state->dataLoopNodes->Node(EAFanInletNode).MassFlowRateMaxAvail = 0.60215437; // exhaust fan will not turn on unless max avail is set

    SetPredefinedTables(*state);
    OutdoorAirUnit::SimOutdoorAirUnit(*state,
                                      "ZONE1OUTAIR",
                                      CurZoneNum,
                                      FirstHVACIteration,
                                      SysOutputProvided,
                                      LatOutputProvided,
                                      state->dataZoneEquip->ZoneEquipList(state->dataSize->CurZoneEqNum).EquipIndex(EquipPtr));

    EXPECT_DOUBLE_EQ(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).MinOA,
                     state->dataOutdoorAirUnit->OutAirUnit(OAUnitNum).OutAirVolFlow);
    EXPECT_DOUBLE_EQ(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).MinOA * state->dataEnvrn->StdRhoAir,
                     state->dataOutdoorAirUnit->OutAirUnit(OAUnitNum).OutAirMassFlow);
    EXPECT_DOUBLE_EQ(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).MinOA,
                     state->dataOutdoorAirUnit->OutAirUnit(OAUnitNum).ExtAirVolFlow);
    EXPECT_DOUBLE_EQ(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).MinOA * state->dataEnvrn->StdRhoAir,
                     state->dataOutdoorAirUnit->OutAirUnit(OAUnitNum).ExtAirMassFlow);

    // test that both fans are included in OA unit fan power report
    Real64 SAFanPower = state->dataFans->fans(1)->totalPower;
    Real64 EAFanPower = state->dataFans->fans(2)->totalPower;
    EXPECT_DOUBLE_EQ(SAFanPower, 75.0);
    EXPECT_DOUBLE_EQ(EAFanPower, 75.0);
    EXPECT_DOUBLE_EQ(SAFanPower + EAFanPower, state->dataOutdoorAirUnit->OutAirUnit(OAUnitNum).ElecFanRate);
    compare_err_stream_substring("", true);
    EXPECT_TRUE(compare_err_stream("", true));

    // #6173
    state->dataOutdoorAirUnit->OutAirUnit(OAUnitNum).ExtAirMassFlow = 0.0;
    OutdoorAirUnit::CalcOutdoorAirUnit(*state, OAUnitNum, CurZoneNum, FirstHVACIteration, SysOutputProvided, LatOutputProvided);

    std::string const error_string = delimited_string({
        "   ** Warning ** Air mass flow between zone supply and exhaust is not balanced. Only the first occurrence is reported.",
        "   **   ~~~   ** Occurs in ZoneHVAC:OutdoorAirUnit Object= ZONE1OUTAIR",
        "   **   ~~~   ** Air mass balance is required by other outdoor air units: Fan:ZoneExhaust, ZoneMixing, ZoneCrossMixing, or other air flow "
        "control inputs.",
        "   **   ~~~   ** The outdoor mass flow rate = 0.602 and the exhaust mass flow rate = 0.000.",
        "   **   ~~~   **  Environment=, at Simulation time= 00:00 - 00:15",
    });

    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, OutdoorAirUnit_WaterCoolingCoilAutoSizeTest)
{

    std::string const idf_objects = delimited_string({
        "Zone,",
        "    Thermal Zone 1,          !- Name",
        "    ,                        !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    10,                      !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    ,                        !- Type",
        "    ,                        !- Multiplier",
        "    ,                        !- Ceiling Height {m}",
        "    300,                     !- Volume {m3}",
        "    100;                     !- Floor Area {m2}",

        "ZoneHVAC:EquipmentConnections,",
        "    Thermal Zone 1,          !- Zone Name",
        "    Thermal Zone 1 Equipment List,  !- Zone Conditioning Equipment List Name",
        "    Thermal Zone 1 Inlet Node List,  !- Zone Air Inlet Node or NodeList Name",
        "    Thermal Zone 1 Exhaust Node List,  !- Zone Air Exhaust Node or NodeList Name",
        "    Node 1,                  !- Zone Air Node Name",
        "    Thermal Zone 1 Return Air Node;  !- Zone Return Air Node or NodeList Name",

        "NodeList,",
        "    Thermal Zone 1 Inlet Node List,  !- Name",
        "    Node 5;                  !- Node 1 Name",

        "NodeList,",
        "    Thermal Zone 1 Exhaust Node List,  !- Name",
        "    Node 4;                  !- Node 1 Name",

        "OutdoorAir:Node,",
        "    Model Outdoor Air Node;  !- Name",

        "OutdoorAir:NodeList,",
        "    OAUnit OA Node;          !- Node or NodeList Name 1",

        "	Schedule:Constant,",
        "	FanAndCoilAvailSched, !- Name",
        "	FRACTION, !- Schedule Type",
        "	1;        !- TimeStep Value",

        "	ScheduleTypeLimits,",
        "	Fraction, !- Name",
        "	0.0, !- Lower Limit Value",
        "	1.0, !- Upper Limit Value",
        "	CONTINUOUS;              !- Numeric Type",

        "Schedule:Compact,",
        "    OAULoCtrlTemp,           !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    10;                      !- Field 4",

        "Schedule:Compact,",
        "    OAUHiCtrlTemp,           !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    15;                      !- Field 4",

        "ScheduleTypeLimits,",
        "    Temperature,             !- Name",
        "    -60,                     !- Lower Limit Value",
        "    200,                     !- Upper Limit Value",
        "    CONTINUOUS;              !- Numeric Type",

        "ZoneHVAC:EquipmentList,",
        "    Thermal Zone 1 Equipment List,  !- Name",
        "    ,                        !- Load Distribution Scheme",
        "    ZoneHVAC:OutdoorAirUnit, !- Zone Equipment 1 Object Type",
        "    OAUnit Zone 1,           !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "ZoneHVAC:OutdoorAirUnit,",
        "    OAUnit Zone 1,           !- Name",
        "    FanAndCoilAvailSched,    !- Availability Schedule Name",
        "    Thermal Zone 1,          !- Zone Name",
        "    Autosize,                !- Outdoor Air Flow Rate {m3/s}",
        "    FanAndCoilAvailSched,    !- Outdoor Air Schedule Name",
        "    OAU Supply Fan,          !- Supply Fan Name",
        "    BlowThrough,             !- Supply Fan Placement",
        "    Zone 1 OAU ExhFan,       !- Exhaust Fan Name",
        "    Autosize,                !- Exhaust Air Flow Rate {m3/s}",
        "    FanAndCoilAvailSched,    !- Exhaust Air Schedule Name",
        "    TemperatureControl,      !- Unit Control Type",
        "    OAUHiCtrlTemp,           !- High Air Control Temperature Schedule Name",
        "    OAULoCtrlTemp,           !- Low Air Control Temperature Schedule Name",
        "    OAUnit OA Node,          !- Outdoor Air Node Name",
        "    Node 5,                  !- AirOutlet Node Name",
        "    OAUnit OA Node,          !- AirInlet Node Name",
        "    OAUnit Fan Outlet Node,  !- Supply FanOutlet Node Name",
        "    OAUnitZone1EQLIST;       !- Outdoor Air Unit List Name",

        "ZoneHVAC:OutdoorAirUnit:EquipmentList,",
        "    OAUnitZone1EQLIST,       !- Name",
        "    Coil:Cooling:Water,      !- Component 2 Object Type",
        "    OAU Water Cooling Coil;  !- Component 2 Name",

        "Fan:SystemModel,",
        "    Zone 1 OAU ExhFan,       !- Name",
        "    FanAndCoilAvailSched,    !- Availability Schedule Name",
        "    Node 4,                  !- Air Inlet Node Name",
        "    ZoneOAU Relief Node,     !- Air Outlet Node Name",
        "    Autosize,                !- Design Maximum Air Flow Rate {m3/s}",
        "    Discrete,                !- Speed Control Method",
        "    0.0,                     !- Electric Power Minimum Flow Rate Fraction",
        "    75.0,                    !- Design Pressure Rise {Pa}",
        "    0.9,                     !- Motor Efficiency",
        "    1.0,                     !- Motor In Air Stream Fraction",
        "    AUTOSIZE,                !- Design Electric Power Consumption {W}",
        "    TotalEfficiencyAndPressure,  !- Design Power Sizing Method",
        "    ,                        !- Electric Power Per Unit Flow Rate {W/(m3/s)}",
        "    ,                        !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}",
        "    0.50,                    !- Fan Total Efficiency",
        "    ,                        !- Electric Power Function of Flow Fraction Curve Name",
        "    ,                        !- Night Ventilation Mode Pressure Rise",
        "    ,                        !- Night Ventilation Mode Flow Fraction",
        "    ,                        !- Motor Loss Zone Name",
        "    ,                        !- Motor Loss Radiative Fraction ",
        "    ,                        !- End-Use Subcategory",
        "    1,                       !- Number of Speeds",
        "    1.0,                     !- Speed 1 Flow Fraction",
        "    1.0;                     !- Speed 1 Electric Power Fraction",

        "Fan:SystemModel,",
        "    OAU Supply Fan,          !- Name",
        "    FanAndCoilAvailSched,    !- Availability Schedule Name",
        "    OAUnit OA Node,          !- Air Inlet Node Name",
        "    OAUnit Fan Outlet Node,  !- Air Outlet Node Name",
        "    Autosize,                !- Design Maximum Air Flow Rate {m3/s}",
        "    Discrete,                !- Speed Control Method",
        "    0.0,                     !- Electric Power Minimum Flow Rate Fraction",
        "    75.0,                    !- Design Pressure Rise {Pa}",
        "    0.9,                     !- Motor Efficiency",
        "    1.0,                     !- Motor In Air Stream Fraction",
        "    AUTOSIZE,                !- Design Electric Power Consumption {W}",
        "    TotalEfficiencyAndPressure,  !- Design Power Sizing Method",
        "    ,                        !- Electric Power Per Unit Flow Rate {W/(m3/s)}",
        "    ,                        !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}",
        "    0.50,                    !- Fan Total Efficiency",
        "    ,                        !- Electric Power Function of Flow Fraction Curve Name",
        "    ,                        !- Night Ventilation Mode Pressure Rise",
        "    ,                        !- Night Ventilation Mode Flow Fraction",
        "    ,                        !- Motor Loss Zone Name",
        "    ,                        !- Motor Loss Radiative Fraction ",
        "    ,                        !- End-Use Subcategory",
        "    1,                       !- Number of Speeds",
        "    1.0,                     !- Speed 1 Flow Fraction",
        "    1.0;                     !- Speed 1 Electric Power Fraction",

        "Coil:Cooling:Water,",
        "    OAU Water Cooling Coil,  !- Name",
        "    FanAndCoilAvailSched,    !- Availability Schedule Name",
        "    Autosize,                !- Design Water Flow Rate {m3/s}",
        "    Autosize,                !- Design Air Flow Rate {m3/s}",
        "    Autosize,                !- Design Inlet Water Temperature {C}",
        "    Autosize,                !- Design Inlet Air Temperature {C}",
        "    Autosize,                !- Design Outlet Air Temperature {C}",
        "    Autosize,                !- Design Inlet Air Humidity Ratio {kgWater/kgDryAir}",
        "    Autosize,                !- Design Outlet Air Humidity Ratio {kgWater/kgDryAir}",
        "    Node 11,                 !- Water Inlet Node Name",
        "    Node 27,                 !- Water Outlet Node Name",
        "    Heating Coil Outlet Node,!- Air Inlet Node Name",
        "    Node 5,                  !- Air Outlet Node Name",
        "    SimpleAnalysis,          !- Type of Analysis",
        "    CrossFlow;               !- Heat Exchanger Configuration",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->DoingSizing = true;

    bool ErrorsFound(false);
    GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    EXPECT_EQ("THERMAL ZONE 1", state->dataHeatBal->Zone(1).Name);

    GetZoneEquipmentData(*state);
    Fans::GetFanInput(*state);

    OutdoorAirUnit::GetOutdoorAirUnitInputs(*state);

    int OAUnitNum(1);
    EXPECT_EQ("OAU SUPPLY FAN", state->dataOutdoorAirUnit->OutAirUnit(OAUnitNum).SFanName);
    EXPECT_EQ("ZONE 1 OAU EXHFAN", state->dataOutdoorAirUnit->OutAirUnit(OAUnitNum).ExtFanName);
    EXPECT_EQ((int)HVAC::FanType::SystemModel, (int)state->dataOutdoorAirUnit->OutAirUnit(OAUnitNum).supFanType);
    EXPECT_EQ((int)HVAC::FanType::SystemModel, (int)state->dataOutdoorAirUnit->OutAirUnit(OAUnitNum).extFanType);

    EXPECT_EQ(1, state->dataOutdoorAirUnit->OutAirUnit(OAUnitNum).NumComponents);
    EXPECT_ENUM_EQ(OutdoorAirUnit::CompType::WaterCoil_Cooling, state->dataOutdoorAirUnit->OutAirUnit(OAUnitNum).OAEquip(1).Type);
    EXPECT_ENUM_EQ(DataPlant::PlantEquipmentType::CoilWaterCooling, state->dataOutdoorAirUnit->OutAirUnit(OAUnitNum).OAEquip(1).CoilType);

    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    state->dataSize->NumPltSizInput = 1;
    state->dataSize->PlantSizData.allocate(state->dataSize->NumPltSizInput);

    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }

    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.loopNum = 1;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.branchNum = 1;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.compNum = 1;

    state->dataPlnt->PlantLoop(1).Name = "ChilledWaterLoop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = state->dataWaterCoils->WaterCoil(1).Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilWaterCooling;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut =
        state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum;

    state->dataSize->PlantSizData(1).PlantLoopName = "ChilledWaterLoop";
    state->dataSize->PlantSizData(1).ExitTemp = 6.7;
    state->dataSize->PlantSizData(1).DeltaT = 5.0;
    state->dataSize->PlantSizData(1).LoopType = DataSizing::TypeOfPlantLoop::Cooling;

    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(1);
    state->dataWaterCoils->MyUAAndFlowCalcFlag(1) = true;
    state->dataWaterCoils->MyUAAndFlowCalcFlag(1) = true;

    state->dataGlobal->HourOfDay = 15;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->Month = 7;
    state->dataEnvrn->DayOfMonth = 21;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, state->dataGlobal->HourOfDay);

    Sched::UpdateScheduleVals(*state);

    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;

    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->CurZoneEqNum = 1;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).DesignSizeFromParent = false;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod.allocate(25);
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod(HVAC::SystemAirflowSizing) = DataSizing::SupplyAirFlowRate;

    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).MinOA = 0.5;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow = 0.5;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInTemp = 30.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInHumRat = 0.01;

    state->dataEnvrn->StdRhoAir = PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->OutBaroPress, 30.0, 0.0);

    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDesTemp = 12.8;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDesHumRat = 0.0080;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolDens = state->dataEnvrn->StdRhoAir;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMassFlow =
        state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow *
        state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolDens;

    state->dataOutdoorAirUnit->OutAirUnit(OAUnitNum).OAEquip(1).MaxVolWaterFlow = DataSizing::AutoSize;

    state->dataGlobal->BeginEnvrnFlag = true;
    bool FirstHVACIteration(true);
    int ZoneNum(1);

    OutdoorAirUnit::InitOutdoorAirUnit(*state, OAUnitNum, ZoneNum, FirstHVACIteration);
    EXPECT_EQ(state->dataWaterCoils->WaterCoil(1).MaxWaterVolFlowRate, state->dataOutdoorAirUnit->OutAirUnit(OAUnitNum).OAEquip(1).MaxVolWaterFlow);

    // calculate fan heat to get fan air-side delta T
    state->dataSize->DataFanType = HVAC::FanType::SystemModel;
    state->dataSize->DataFanIndex = Fans::GetFanIndex(*state, "OAU SUPPLY FAN");
    state->dataSize->DataAirFlowUsedForSizing = state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow;
    Real64 FanCoolLoad = DataAirSystems::calcFanDesignHeatGain(*state, state->dataSize->DataFanIndex, state->dataSize->DataAirFlowUsedForSizing);

    // do water flow rate sizing calculation
    Real64 DesAirMassFlow = state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMassFlow;
    Real64 EnthalpyAirIn = PsyHFnTdbW(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInTemp,
                                      state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInHumRat);
    Real64 EnthalpyAirOut = PsyHFnTdbW(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDesTemp,
                                       state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDesHumRat);

    Real64 DesWaterCoolingCoilLoad = DesAirMassFlow * (EnthalpyAirIn - EnthalpyAirOut) + FanCoolLoad;
    Real64 CoilDesWaterDeltaT = state->dataSize->PlantSizData(1).DeltaT;
    Real64 Cp = state->dataPlnt->PlantLoop(1).glycol->getSpecificHeat(*state, Constant::CWInitConvTemp, " ");
    Real64 rho = state->dataPlnt->PlantLoop(1).glycol->getDensity(*state, Constant::CWInitConvTemp, " ");
    Real64 DesCoolingCoilWaterVolFlowRate = DesWaterCoolingCoilLoad / (CoilDesWaterDeltaT * Cp * rho);
    // check water coil water flow rate calc
    EXPECT_EQ(DesWaterCoolingCoilLoad, state->dataWaterCoils->WaterCoil(1).DesWaterCoolingCoilRate);
    EXPECT_EQ(DesCoolingCoilWaterVolFlowRate, state->dataWaterCoils->WaterCoil(1).MaxWaterVolFlowRate);
}

TEST_F(EnergyPlusFixture, OutdoorAirUnit_SteamHeatingCoilAutoSizeTest)
{

    std::string const idf_objects = delimited_string({
        "Zone,",
        "    Thermal Zone 1,          !- Name",
        "    ,                        !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    10,                      !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    ,                        !- Type",
        "    ,                        !- Multiplier",
        "    ,                        !- Ceiling Height {m}",
        "    300,                     !- Volume {m3}",
        "    100;                     !- Floor Area {m2}",

        "ZoneHVAC:EquipmentConnections,",
        "    Thermal Zone 1,          !- Zone Name",
        "    Thermal Zone 1 Equipment List,  !- Zone Conditioning Equipment List Name",
        "    Thermal Zone 1 Inlet Node List,  !- Zone Air Inlet Node or NodeList Name",
        "    Thermal Zone 1 Exhaust Node List,  !- Zone Air Exhaust Node or NodeList Name",
        "    Node 1,                  !- Zone Air Node Name",
        "    Thermal Zone 1 Return Air Node;  !- Zone Return Air Node or NodeList Name",

        "NodeList,",
        "    Thermal Zone 1 Inlet Node List,  !- Name",
        "    Node 5;                  !- Node 1 Name",

        "NodeList,",
        "    Thermal Zone 1 Exhaust Node List,  !- Name",
        "    Node 4;                  !- Node 1 Name",

        "OutdoorAir:Node,",
        "    Model Outdoor Air Node;  !- Name",

        "OutdoorAir:NodeList,",
        "    OAUnit OA Node;          !- Node or NodeList Name 1",

        "	Schedule:Constant,",
        "	FanAndCoilAvailSched, !- Name",
        "	FRACTION, !- Schedule Type",
        "	1;        !- TimeStep Value",

        "	ScheduleTypeLimits,",
        "	Fraction, !- Name",
        "	0.0, !- Lower Limit Value",
        "	1.0, !- Upper Limit Value",
        "	CONTINUOUS;              !- Numeric Type",

        "Schedule:Compact,",
        "    OAULoCtrlTemp,           !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    10;                      !- Field 4",

        "Schedule:Compact,",
        "    OAUHiCtrlTemp,           !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    15;                      !- Field 4",

        "ScheduleTypeLimits,",
        "    Temperature,             !- Name",
        "    -60,                     !- Lower Limit Value",
        "    200,                     !- Upper Limit Value",
        "    CONTINUOUS;              !- Numeric Type",

        "ZoneHVAC:EquipmentList,",
        "    Thermal Zone 1 Equipment List,  !- Name",
        "    ,                        !- Load Distribution Scheme",
        "    ZoneHVAC:OutdoorAirUnit, !- Zone Equipment 1 Object Type",
        "    OAUnit Zone 1,           !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "ZoneHVAC:OutdoorAirUnit,",
        "    OAUnit Zone 1,           !- Name",
        "    FanAndCoilAvailSched,    !- Availability Schedule Name",
        "    Thermal Zone 1,          !- Zone Name",
        "    Autosize,                !- Outdoor Air Flow Rate {m3/s}",
        "    FanAndCoilAvailSched,    !- Outdoor Air Schedule Name",
        "    OAU Supply Fan,          !- Supply Fan Name",
        "    BlowThrough,             !- Supply Fan Placement",
        "    Zone 1 OAU ExhFan,       !- Exhaust Fan Name",
        "    Autosize,                !- Exhaust Air Flow Rate {m3/s}",
        "    FanAndCoilAvailSched,    !- Exhaust Air Schedule Name",
        "    TemperatureControl,      !- Unit Control Type",
        "    OAUHiCtrlTemp,           !- High Air Control Temperature Schedule Name",
        "    OAULoCtrlTemp,           !- Low Air Control Temperature Schedule Name",
        "    OAUnit OA Node,          !- Outdoor Air Node Name",
        "    Node 5,                  !- AirOutlet Node Name",
        "    OAUnit OA Node,          !- AirInlet Node Name",
        "    OAUnit Fan Outlet Node,  !- Supply FanOutlet Node Name",
        "    OAUnitZone1EQLIST;       !- Outdoor Air Unit List Name",

        "ZoneHVAC:OutdoorAirUnit:EquipmentList,",
        "    OAUnitZone1EQLIST,       !- Name",
        "    Coil:Heating:Steam,      !- Component 1 Object Type",
        "    OAU Steam Heating Coil;  !- Component 1 Name",

        "Fan:SystemModel,",
        "    Zone 1 OAU ExhFan,       !- Name",
        "    FanAndCoilAvailSched,    !- Availability Schedule Name",
        "    Node 4,                  !- Air Inlet Node Name",
        "    ZoneOAU Relief Node,     !- Air Outlet Node Name",
        "    Autosize,                !- Design Maximum Air Flow Rate {m3/s}",
        "    Discrete,                !- Speed Control Method",
        "    0.0,                     !- Electric Power Minimum Flow Rate Fraction",
        "    75.0,                    !- Design Pressure Rise {Pa}",
        "    0.9,                     !- Motor Efficiency",
        "    1.0,                     !- Motor In Air Stream Fraction",
        "    AUTOSIZE,                !- Design Electric Power Consumption {W}",
        "    TotalEfficiencyAndPressure,  !- Design Power Sizing Method",
        "    ,                        !- Electric Power Per Unit Flow Rate {W/(m3/s)}",
        "    ,                        !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}",
        "    0.50,                    !- Fan Total Efficiency",
        "    ,                        !- Electric Power Function of Flow Fraction Curve Name",
        "    ,                        !- Night Ventilation Mode Pressure Rise",
        "    ,                        !- Night Ventilation Mode Flow Fraction",
        "    ,                        !- Motor Loss Zone Name",
        "    ,                        !- Motor Loss Radiative Fraction ",
        "    ,                        !- End-Use Subcategory",
        "    1,                       !- Number of Speeds",
        "    1.0,                     !- Speed 1 Flow Fraction",
        "    1.0;                     !- Speed 1 Electric Power Fraction",

        "Fan:SystemModel,",
        "    OAU Supply Fan,          !- Name",
        "    FanAndCoilAvailSched,    !- Availability Schedule Name",
        "    OAUnit OA Node,          !- Air Inlet Node Name",
        "    OAUnit Fan Outlet Node,  !- Air Outlet Node Name",
        "    Autosize,                !- Design Maximum Air Flow Rate {m3/s}",
        "    Discrete,                !- Speed Control Method",
        "    0.0,                     !- Electric Power Minimum Flow Rate Fraction",
        "    75.0,                    !- Design Pressure Rise {Pa}",
        "    0.9,                     !- Motor Efficiency",
        "    1.0,                     !- Motor In Air Stream Fraction",
        "    AUTOSIZE,                !- Design Electric Power Consumption {W}",
        "    TotalEfficiencyAndPressure,  !- Design Power Sizing Method",
        "    ,                        !- Electric Power Per Unit Flow Rate {W/(m3/s)}",
        "    ,                        !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}",
        "    0.50,                    !- Fan Total Efficiency",
        "    ,                        !- Electric Power Function of Flow Fraction Curve Name",
        "    ,                        !- Night Ventilation Mode Pressure Rise",
        "    ,                        !- Night Ventilation Mode Flow Fraction",
        "    ,                        !- Motor Loss Zone Name",
        "    ,                        !- Motor Loss Radiative Fraction ",
        "    ,                        !- End-Use Subcategory",
        "    1,                       !- Number of Speeds",
        "    1.0,                     !- Speed 1 Flow Fraction",
        "    1.0;                     !- Speed 1 Electric Power Fraction",

        "Coil:Heating:Steam,",
        "     OAU Steam Heating Coil, !- Name",
        "    FanAndCoilAvailSched,    !- Availability Schedule Name",
        "    Autosize,                !- Maximum Steam Flow Rate {m3/s}",
        "    5.0,                     !- Degree of SubCooling {C}",
        "    15.0,                    !- Degree of Loop SubCooling {C}",
        "    Node 21,                 !- Water Inlet Node Name",
        "    Node 26,                 !- Water Outlet Node Name",
        "    OAUnit Fan Outlet Node,  !- Air Inlet Node Name",
        "    Heating Coil Outlet Node,!- Air Outlet Node Name",
        "    ZoneLoadControl;         !- Coil Control Type",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->DoingSizing = true;

    bool ErrorsFound(false);
    GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    EXPECT_EQ("THERMAL ZONE 1", state->dataHeatBal->Zone(1).Name);

    GetZoneEquipmentData(*state);
    Fans::GetFanInput(*state);

    OutdoorAirUnit::GetOutdoorAirUnitInputs(*state);

    int OAUnitNum(1);
    EXPECT_EQ("OAU SUPPLY FAN", state->dataOutdoorAirUnit->OutAirUnit(OAUnitNum).SFanName);
    EXPECT_EQ("ZONE 1 OAU EXHFAN", state->dataOutdoorAirUnit->OutAirUnit(OAUnitNum).ExtFanName);
    EXPECT_EQ((int)HVAC::FanType::SystemModel, (int)state->dataOutdoorAirUnit->OutAirUnit(OAUnitNum).supFanType);
    EXPECT_EQ((int)HVAC::FanType::SystemModel, (int)state->dataOutdoorAirUnit->OutAirUnit(OAUnitNum).extFanType);

    EXPECT_EQ(1, state->dataOutdoorAirUnit->OutAirUnit(OAUnitNum).NumComponents);
    EXPECT_ENUM_EQ(OutdoorAirUnit::CompType::SteamCoil_AirHeat, state->dataOutdoorAirUnit->OutAirUnit(OAUnitNum).OAEquip(1).Type);
    EXPECT_ENUM_EQ(DataPlant::PlantEquipmentType::CoilSteamAirHeating, state->dataOutdoorAirUnit->OutAirUnit(OAUnitNum).OAEquip(1).CoilType);

    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    state->dataSize->NumPltSizInput = 1;
    state->dataSize->PlantSizData.allocate(state->dataSize->NumPltSizInput);

    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }

    state->dataSteamCoils->SteamCoil(1).plantLoc.loopNum = 1;
    state->dataSteamCoils->SteamCoil(1).plantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    state->dataSteamCoils->SteamCoil(1).plantLoc.branchNum = 1;
    state->dataSteamCoils->SteamCoil(1).plantLoc.compNum = 1;

    state->dataPlnt->PlantLoop(1).Name = "SteamLoop";
    state->dataPlnt->PlantLoop(1).FluidName = "STEAM";
    state->dataPlnt->PlantLoop(1).steam = Fluid::GetSteam(*state);
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = state->dataSteamCoils->SteamCoil(1).Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilSteamAirHeating;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataSteamCoils->SteamCoil(1).SteamInletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut =
        state->dataSteamCoils->SteamCoil(1).SteamOutletNodeNum;

    state->dataSize->PlantSizData(1).PlantLoopName = "SteamLoop";
    state->dataSize->PlantSizData(1).ExitTemp = 100.0;
    state->dataSize->PlantSizData(1).DeltaT = 5.0;
    state->dataSize->PlantSizData(1).LoopType = DataSizing::TypeOfPlantLoop::Steam;

    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(2);
    state->dataWaterCoils->MyUAAndFlowCalcFlag(1) = true;
    state->dataWaterCoils->MyUAAndFlowCalcFlag(2) = true;

    state->dataGlobal->HourOfDay = 15;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 21;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, state->dataGlobal->HourOfDay);

    Sched::UpdateScheduleVals(*state);

    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;

    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->CurZoneEqNum = 1;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).DesignSizeFromParent = false;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod.allocate(25);
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod(HVAC::SystemAirflowSizing) = DataSizing::SupplyAirFlowRate;

    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).MinOA = 0.5;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow = 0.5;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInTemp = 5.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInHumRat = 0.005;

    state->dataEnvrn->StdRhoAir = PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->OutBaroPress, 5.0, 0.0);
    state->dataOutdoorAirUnit->OutAirUnit(OAUnitNum).OAEquip(1).MaxVolWaterFlow = DataSizing::AutoSize;

    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).HeatDesTemp = 50.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).HeatDesHumRat = 0.0050;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatDens = state->dataEnvrn->StdRhoAir;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMassFlow =
        state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow *
        state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatDens;

    state->dataGlobal->BeginEnvrnFlag = true;
    bool FirstHVACIteration(true);
    int ZoneNum(1);

    OutdoorAirUnit::InitOutdoorAirUnit(*state, OAUnitNum, ZoneNum, FirstHVACIteration);
    EXPECT_EQ(state->dataSteamCoils->SteamCoil(1).MaxSteamVolFlowRate, state->dataOutdoorAirUnit->OutAirUnit(OAUnitNum).OAEquip(1).MaxVolWaterFlow);

    Real64 DesCoilInTemp = state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInTemp;
    Real64 DesCoilOutTemp = state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).HeatDesTemp;
    Real64 DesCoilOutHumRat = state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).HeatDesHumRat;
    Real64 DesAirMassFlow = state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMassFlow;
    // DesVolFlow = DesMassFlow / RhoAirStd;
    Real64 CpAirAvg = PsyCpAirFnW(DesCoilOutHumRat);
    Real64 DesSteamCoilLoad = DesAirMassFlow * CpAirAvg * (DesCoilOutTemp - DesCoilInTemp);

    // do steam flow rate sizing calculation
    Real64 EnthSteamIn = state->dataSteamCoils->SteamCoil(1).steam->getSatEnthalpy(*state, Constant::SteamInitConvTemp, 1.0, "");
    Real64 EnthSteamOut = state->dataSteamCoils->SteamCoil(1).steam->getSatEnthalpy(*state, Constant::SteamInitConvTemp, 0.0, "");
    Real64 SteamDensity = state->dataSteamCoils->SteamCoil(1).steam->getSatDensity(*state, Constant::SteamInitConvTemp, 1.0, "");
    Real64 CpOfCondensate = state->dataSteamCoils->SteamCoil(1).steam->getSatSpecificHeat(*state, Constant::SteamInitConvTemp, 0.0, "");
    Real64 LatentHeatChange = EnthSteamIn - EnthSteamOut;
    Real64 DesMaxSteamVolFlowRate =
        DesSteamCoilLoad / (SteamDensity * (LatentHeatChange + state->dataSteamCoils->SteamCoil(1).DegOfSubcooling * CpOfCondensate));

    // check water coil water flow rate calc
    EXPECT_EQ(DesSteamCoilLoad, state->dataSteamCoils->SteamCoil(1).DesCoilCapacity);
    EXPECT_EQ(DesMaxSteamVolFlowRate, state->dataSteamCoils->SteamCoil(1).MaxSteamVolFlowRate);
}
} // namespace EnergyPlus
