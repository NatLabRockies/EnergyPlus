// EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the University of Illinois,
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

// EnergyPlus::SimAirServingZones Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// C++ Headers
#include <string>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/SimAirServingZones.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/SingleDuct.hh>
#include <EnergyPlus/SplitterComponent.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneAirLoopEquipmentManager.hh>

using namespace EnergyPlus;
using namespace DataAirSystems;
using namespace DataSizing;
using namespace SimAirServingZones;

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, SimAirServingZones_ReheatCoilSizing)
{
    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine test the GetHeatingSATempForSizing & GetHeatingSATempHumRatForSizing methods,
    // which are designed to get the proper reheat coil inlet temperature/humidity ratio for sizing
    // depending on the system configurations

    int NumPrimaryAirSys = 4; // total number of air loops
    int AirLoopNum;           // index of air loops
    int CtrlZoneNum;          // index of zones

    // Allocate
    state->dataSize->CalcSysSizing.allocate(NumPrimaryAirSys);
    state->dataSize->FinalSysSizing.allocate(NumPrimaryAirSys);
    state->dataSize->FinalZoneSizing.allocate(NumPrimaryAirSys);
    state->dataAirSystemsData->PrimaryAirSystems.allocate(NumPrimaryAirSys);

    // Inputs: system configurations:
    // 	(1) Central heating coils exist
    // 	(2) No central heating coils, but preheating coils exist
    // 	(3) No central heating coils, but OA heat-exchangers exist
    // 	(4) No central heating coils; No preheating coils or OA heat-exchangers

    state->dataAirSystemsData->PrimaryAirSystems(1).CentralHeatCoilExists = true;
    state->dataAirSystemsData->PrimaryAirSystems(2).CentralHeatCoilExists = false;
    state->dataAirSystemsData->PrimaryAirSystems(3).CentralHeatCoilExists = false;
    state->dataAirSystemsData->PrimaryAirSystems(4).CentralHeatCoilExists = false;

    state->dataAirSystemsData->PrimaryAirSystems(1).NumOAHeatCoils = 0;
    state->dataAirSystemsData->PrimaryAirSystems(2).NumOAHeatCoils = 1;
    state->dataAirSystemsData->PrimaryAirSystems(3).NumOAHeatCoils = 0;
    state->dataAirSystemsData->PrimaryAirSystems(4).NumOAHeatCoils = 0;

    state->dataAirSystemsData->PrimaryAirSystems(1).NumOAHXs = 0;
    state->dataAirSystemsData->PrimaryAirSystems(2).NumOAHXs = 0;
    state->dataAirSystemsData->PrimaryAirSystems(3).NumOAHXs = 1;
    state->dataAirSystemsData->PrimaryAirSystems(4).NumOAHXs = 0;

    // Inputs: sizing parameters
    for (AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) {
        state->dataSize->FinalSysSizing(AirLoopNum).DesOutAirVolFlow = 0.25;
        state->dataSize->FinalSysSizing(AirLoopNum).DesHeatVolFlow = 0.50;

        state->dataSize->FinalSysSizing(AirLoopNum).PreheatTemp = 7;
        state->dataSize->FinalSysSizing(AirLoopNum).HeatRetTemp = 22;
        state->dataSize->FinalSysSizing(AirLoopNum).HeatMixTemp = 10;
        state->dataSize->CalcSysSizing(AirLoopNum).HeatSupTemp = 17;

        state->dataSize->FinalSysSizing(AirLoopNum).PreheatHumRat = 0.003;
        state->dataSize->FinalSysSizing(AirLoopNum).HeatRetHumRat = 0.008;
        state->dataSize->FinalSysSizing(AirLoopNum).HeatMixHumRat = 0.004;
        state->dataSize->CalcSysSizing(AirLoopNum).HeatSupHumRat = 0.006;
    }

    // Run
    for (AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) {
        CtrlZoneNum = AirLoopNum;

        state->dataSize->FinalZoneSizing(CtrlZoneNum).DesHeatCoilInTempTU = GetHeatingSATempForSizing(*state, AirLoopNum);
        state->dataSize->FinalZoneSizing(CtrlZoneNum).DesHeatCoilInHumRatTU = GetHeatingSATempHumRatForSizing(*state, AirLoopNum);
    }

    // Check
    EXPECT_EQ(17.0, state->dataSize->FinalZoneSizing(1).DesHeatCoilInTempTU);
    EXPECT_NEAR(14.5, state->dataSize->FinalZoneSizing(2).DesHeatCoilInTempTU, 0.05);
    EXPECT_NEAR(14.5, state->dataSize->FinalZoneSizing(3).DesHeatCoilInTempTU, 0.05);
    EXPECT_EQ(10.0, state->dataSize->FinalZoneSizing(4).DesHeatCoilInTempTU);
    EXPECT_EQ(0.006, state->dataSize->FinalZoneSizing(1).DesHeatCoilInHumRatTU);
    EXPECT_EQ(0.0055, state->dataSize->FinalZoneSizing(2).DesHeatCoilInHumRatTU);
    EXPECT_EQ(0.0055, state->dataSize->FinalZoneSizing(3).DesHeatCoilInHumRatTU);
    EXPECT_EQ(0.004, state->dataSize->FinalZoneSizing(4).DesHeatCoilInHumRatTU);

    // Clean up
    state->dataSize->CalcSysSizing.deallocate();
    state->dataSize->FinalSysSizing.deallocate();
    state->dataSize->FinalZoneSizing.deallocate();
    state->dataAirSystemsData->PrimaryAirSystems.deallocate();
}

TEST_F(EnergyPlusFixture, SimAirServingZones_LimitZoneVentEff)
{
    int CtrlZoneNum = 1;
    state->dataSize->TermUnitFinalZoneSizing.allocate(1);

    // Test case 1, low OA, low zoneventilationeff, no change in SysCoolingEv
    Real64 StartingDesCoolVolFlow = 1.0;
    Real64 StartingDesCoolVolFlowMin = 0.2;
    Real64 UncorrectedOAFlow = 0.1;
    state->dataSize->TermUnitFinalZoneSizing(CtrlZoneNum).DesCoolVolFlow = StartingDesCoolVolFlow;
    state->dataSize->TermUnitFinalZoneSizing(CtrlZoneNum).DesCoolVolFlowMin = StartingDesCoolVolFlowMin;
    state->dataSize->TermUnitFinalZoneSizing(CtrlZoneNum).ZoneSecondaryRecirculation = 0.0;
    state->dataSize->TermUnitFinalZoneSizing(CtrlZoneNum).ZoneVentilationEff = 0.5;
    Real64 Xs = 0.25;                                                  // uncorrected system outdoor air fraction
    Real64 VozClg = UncorrectedOAFlow;                                 // corrected (for ventilation efficiency) zone outside air flow rate [m3/s]
    Real64 ZoneOAFrac = UncorrectedOAFlow / StartingDesCoolVolFlowMin; // zone OA fraction

    Real64 SysCoolingEv = 1.0 + Xs - ZoneOAFrac; // System level ventilation effectiveness for cooling (from SimAirServingZone::UpdateSysSizing right
                                                 // before call to LimitZoneVentEff)
    Real64 StartingSysCoolingEv = SysCoolingEv;
    LimitZoneVentEff(*state, Xs, VozClg, CtrlZoneNum, SysCoolingEv);
    EXPECT_EQ(StartingSysCoolingEv, SysCoolingEv);
    EXPECT_EQ(StartingDesCoolVolFlow, state->dataSize->TermUnitFinalZoneSizing(CtrlZoneNum).DesCoolVolFlow);
    EXPECT_EQ(StartingDesCoolVolFlowMin, state->dataSize->TermUnitFinalZoneSizing(CtrlZoneNum).DesCoolVolFlowMin);

    // Test case 2, low OA, high zoneventilationeff, increase SysCoolingEv and DesCoolVolFlowMin
    StartingDesCoolVolFlow = 1.0;
    StartingDesCoolVolFlowMin = 0.2;
    UncorrectedOAFlow = 0.1;
    state->dataSize->TermUnitFinalZoneSizing(CtrlZoneNum).DesCoolVolFlow = StartingDesCoolVolFlow;
    state->dataSize->TermUnitFinalZoneSizing(CtrlZoneNum).DesCoolVolFlowMin = StartingDesCoolVolFlowMin;
    state->dataSize->TermUnitFinalZoneSizing(CtrlZoneNum).ZoneSecondaryRecirculation = 0.0;
    state->dataSize->TermUnitFinalZoneSizing(CtrlZoneNum).ZoneVentilationEff = 0.9;
    Xs = 0.25;                                                  // uncorrected system outdoor air fraction
    VozClg = UncorrectedOAFlow;                                 // corrected (for ventilation efficiency) zone outside air flow rate [m3/s]
    ZoneOAFrac = UncorrectedOAFlow / StartingDesCoolVolFlowMin; // zone OA fraction

    SysCoolingEv = 1.0 + Xs - ZoneOAFrac; // System level ventilation effectiveness for cooling (from SimAirServingZone::UpdateSysSizing right before
                                          // call to LimitZoneVentEff)
    StartingSysCoolingEv = SysCoolingEv;
    LimitZoneVentEff(*state, Xs, VozClg, CtrlZoneNum, SysCoolingEv);
    EXPECT_EQ(state->dataSize->TermUnitFinalZoneSizing(CtrlZoneNum).ZoneVentilationEff, SysCoolingEv);
    EXPECT_EQ(StartingDesCoolVolFlow, state->dataSize->TermUnitFinalZoneSizing(CtrlZoneNum).DesCoolVolFlow);
    EXPECT_NEAR(0.2857, state->dataSize->TermUnitFinalZoneSizing(CtrlZoneNum).DesCoolVolFlowMin, 0.001);

    // Test case 3, high OA, high zoneventilationeff, increase SysCoolingEv, DesCoolVolFlowMin, and DesCoolVolFlow
    StartingDesCoolVolFlow = 1.0;
    StartingDesCoolVolFlowMin = 0.8;
    UncorrectedOAFlow = 0.8;
    state->dataSize->TermUnitFinalZoneSizing(CtrlZoneNum).DesCoolVolFlow = StartingDesCoolVolFlow;
    state->dataSize->TermUnitFinalZoneSizing(CtrlZoneNum).DesCoolVolFlowMin = StartingDesCoolVolFlowMin;
    state->dataSize->TermUnitFinalZoneSizing(CtrlZoneNum).ZoneSecondaryRecirculation = 0.0;
    state->dataSize->TermUnitFinalZoneSizing(CtrlZoneNum).ZoneVentilationEff = 0.9;
    Xs = 0.25;                                                  // uncorrected system outdoor air fraction
    VozClg = UncorrectedOAFlow;                                 // corrected (for ventilation efficiency) zone outside air flow rate [m3/s]
    ZoneOAFrac = UncorrectedOAFlow / StartingDesCoolVolFlowMin; // zone OA fraction

    SysCoolingEv = 1.0 + Xs - ZoneOAFrac; // System level ventilation effectiveness for cooling (from SimAirServingZone::UpdateSysSizing right before
                                          // call to LimitZoneVentEff)
    StartingSysCoolingEv = SysCoolingEv;
    LimitZoneVentEff(*state, Xs, VozClg, CtrlZoneNum, SysCoolingEv);
    EXPECT_EQ(state->dataSize->TermUnitFinalZoneSizing(CtrlZoneNum).ZoneVentilationEff, SysCoolingEv);
    EXPECT_NEAR(2.2857, state->dataSize->TermUnitFinalZoneSizing(CtrlZoneNum).DesCoolVolFlow, 0.001);
    EXPECT_NEAR(2.2857, state->dataSize->TermUnitFinalZoneSizing(CtrlZoneNum).DesCoolVolFlowMin, 0.001);
}

TEST_F(EnergyPlusFixture, SizingSystem_FlowPerCapacityMethodTest1)
{
    // this unit test is related to issue #5835
    // when system capacit is hard sized user input
    int AirLoopNum(0);                    // index of air loops
    Real64 ScaledCoolDesignFlowRate(0.0); // system cooling design flow rate
    Real64 ScaledHeatDesignFlowRate(0.0); // system heating design flow rate

    AirLoopNum = 1;
    state->dataSize->CalcSysSizing.allocate(AirLoopNum);
    state->dataSize->FinalSysSizing.allocate(AirLoopNum);

    // set system flow sizing method for cooling
    state->dataSize->FinalSysSizing(AirLoopNum).ScaleCoolSAFMethod = FlowPerCoolingCapacity;
    state->dataSize->FinalSysSizing(AirLoopNum).CoolingCapMethod = CoolingDesignCapacity;
    state->dataSize->FinalSysSizing(AirLoopNum).ScaledCoolingCapacity = 12500.0;
    state->dataSize->FinalSysSizing(AirLoopNum).FlowPerCoolingCapacity = 0.00006041;
    // scale cooling flow rate using user input capacity
    ScaledCoolDesignFlowRate =
        state->dataSize->FinalSysSizing(AirLoopNum).ScaledCoolingCapacity * state->dataSize->FinalSysSizing(AirLoopNum).FlowPerCoolingCapacity;
    // do scalable flow sizing
    UpdateSysSizingForScalableInputs(*state, AirLoopNum);
    EXPECT_DOUBLE_EQ(0.755125, ScaledCoolDesignFlowRate);
    EXPECT_DOUBLE_EQ(0.755125, state->dataSize->FinalSysSizing(AirLoopNum).InpDesCoolAirFlow);

    // set system flow sizing method for heating
    state->dataSize->FinalSysSizing(AirLoopNum).ScaleHeatSAFMethod = FlowPerHeatingCapacity;
    state->dataSize->FinalSysSizing(AirLoopNum).HeatingCapMethod = HeatingDesignCapacity;
    state->dataSize->FinalSysSizing(AirLoopNum).ScaledHeatingCapacity = 14400.0;
    state->dataSize->FinalSysSizing(AirLoopNum).FlowPerHeatingCapacity = 0.00006041;
    // scale heating flow rate using user input capacity
    ScaledHeatDesignFlowRate =
        state->dataSize->FinalSysSizing(AirLoopNum).ScaledHeatingCapacity * state->dataSize->FinalSysSizing(AirLoopNum).FlowPerHeatingCapacity;
    // do scalable flow sizing
    UpdateSysSizingForScalableInputs(*state, AirLoopNum);
    EXPECT_DOUBLE_EQ(0.869904, ScaledHeatDesignFlowRate);
    EXPECT_DOUBLE_EQ(0.869904, state->dataSize->FinalSysSizing(AirLoopNum).InpDesHeatAirFlow);
}

TEST_F(EnergyPlusFixture, SizingSystem_FlowPerCapacityMethodTest2)
{
    // this unit test is related to issue #5835
    // when system capacity is scaled using floor area
    int AirLoopNum(0);                    // index of air loops
    Real64 ScaledCoolDesignFlowRate(0.0); // system cooling design flow rate
    Real64 ScaledHeatDesignFlowRate(0.0); // system heating design flow rate
    Real64 ScaledCoolDesignCapacity(0.0); // system cooling design capacity
    Real64 ScaledHeatDesignCapacity(0.0); // system heating design capacity

    AirLoopNum = 1;
    state->dataSize->CalcSysSizing.allocate(AirLoopNum);
    state->dataSize->FinalSysSizing.allocate(AirLoopNum);

    // set system flow sizing method for cooling
    state->dataSize->FinalSysSizing(AirLoopNum).ScaleCoolSAFMethod = FlowPerCoolingCapacity;
    state->dataSize->FinalSysSizing(AirLoopNum).CoolingCapMethod = CapacityPerFloorArea;
    state->dataSize->FinalSysSizing(AirLoopNum).ScaledCoolingCapacity = 10.4732; // Watts per m2 floor area
    state->dataSize->FinalSysSizing(AirLoopNum).FlowPerCoolingCapacity = 0.00006041;
    state->dataSize->FinalSysSizing(AirLoopNum).FloorAreaOnAirLoopCooled = 61.450534421531373;
    // scale cooling capacity using floor area
    ScaledCoolDesignCapacity =
        state->dataSize->FinalSysSizing(AirLoopNum).ScaledCoolingCapacity * state->dataSize->FinalSysSizing(AirLoopNum).FloorAreaOnAirLoopCooled;
    ScaledCoolDesignFlowRate = state->dataSize->FinalSysSizing(AirLoopNum).FlowPerCoolingCapacity * ScaledCoolDesignCapacity;
    // do scalable flow sizing
    UpdateSysSizingForScalableInputs(*state, AirLoopNum);
    EXPECT_DOUBLE_EQ(0.038878893558427413, ScaledCoolDesignFlowRate);
    EXPECT_DOUBLE_EQ(0.038878893558427413, state->dataSize->FinalSysSizing(AirLoopNum).InpDesCoolAirFlow);

    // set system flow sizing method for heating
    state->dataSize->FinalSysSizing(AirLoopNum).ScaleHeatSAFMethod = FlowPerHeatingCapacity;
    state->dataSize->FinalSysSizing(AirLoopNum).HeatingCapMethod = CapacityPerFloorArea;
    state->dataSize->FinalSysSizing(AirLoopNum).ScaledHeatingCapacity = 32.0050; // Watts per m2 floor area
    state->dataSize->FinalSysSizing(AirLoopNum).FlowPerHeatingCapacity = 0.00006041;
    state->dataSize->FinalSysSizing(AirLoopNum).FloorAreaOnAirLoopCooled = 61.450534421531373;
    // scale heating capacity using floor area
    ScaledHeatDesignCapacity =
        state->dataSize->FinalSysSizing(AirLoopNum).ScaledHeatingCapacity * state->dataSize->FinalSysSizing(AirLoopNum).FloorAreaOnAirLoopCooled;
    ScaledHeatDesignFlowRate = state->dataSize->FinalSysSizing(AirLoopNum).FlowPerHeatingCapacity * ScaledHeatDesignCapacity;
    // do scalable flow sizing
    UpdateSysSizingForScalableInputs(*state, AirLoopNum);
    EXPECT_DOUBLE_EQ(0.11880981823487276, ScaledHeatDesignFlowRate);
    EXPECT_DOUBLE_EQ(0.11880981823487276, state->dataSize->FinalSysSizing(AirLoopNum).InpDesHeatAirFlow);
}

TEST_F(EnergyPlusFixture, GetAirPathData_ControllerLockout1)
{
    // this unit test is related to issue #5973 checks for controller economizer lockout
    std::string const idf_objects = delimited_string({
        " Coil:Cooling:Water,",
        "	AHU cooling coil,	!- Name",
        "	,       			!- Availability Schedule Name",
        "	autosize,			!- Design Water Flow Rate { m3 / s }",
        "	autosize,			!- Design Air Flow Rate { m3 / s }",
        "	autosize,			!- Design Inlet Water Temperature { C }",
        "	autosize,			!- Design Inlet Air Temperature { C }",
        "	autosize,			!- Design Outlet Air Temperature { C }",
        "	autosize,			!- Design Inlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	autosize,			!- Design Outlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	Water Inlet Node,	!- Water Inlet Node Name",
        "	Water Outlet Node,  !- Water Outlet Node Name",
        "	AHU mixed air outlet,		!- Air Inlet Node Name",
        "	AHU cooling coil outlet,	!- Air Outlet Node Name",
        "	SimpleAnalysis,		!- Type of Analysis",
        "	CrossFlow;          !- Heat Exchanger Configuration",

        " Coil:Heating:Water,",
        "	AHU Heating coil, !- Name",
        "	,         !- Availability Schedule Name",
        "	autosize, !- U - Factor Times Area Value { W / K }",
        "	autosize, !- Maximum Water Flow Rate { m3 / s }",
        "	AHU Heating COil HW Inlet, !- Water Inlet Node Name",
        "	AHU Heating COil HW Outlet, !- Water Outlet Node Name",
        "	AHU cooling coil outlet, !- Air Inlet Node Name",
        "	AHU Heating Coil Outlet, !- Air Outlet Node Name",
        "	UFactorTimesAreaAndDesignWaterFlowRate, !- Performance Input Method",
        "	autosize, !- Rated Capacity { W }",
        "	82.2, !- Rated Inlet Water Temperature { C }",
        "	16.6, !- Rated Inlet Air Temperature { C }",
        "	71.1, !- Rated Outlet Water Temperature { C }",
        "	48.8888888888889, !- Rated Outlet Air Temperature { C }",
        "	1;          !- Rated Ratio for Air and Water Convection",

        " Controller:WaterCoil,",
        "	AHU cooling coil controller, !- Name",
        "	TemperatureAndHumidityRatio,		!- Control Variable",
        "	Reverse,			!- Action",
        "	FLOW,				!- Actuator Variable",
        "	AHU cooling coil outlet,	!- Sensor Node Name",
        "	Water Inlet Node,	!- Actuator Node Name",
        "	autosize,			!- Controller Convergence Tolerance { deltaC }",
        "	autosize,			!- Maximum Actuated Flow { m3 / s }",
        "	0.0;				!- Minimum Actuated Flow { m3 / s }",

        " Controller:WaterCoil,",
        "	AHU Heating coil, !- Name",
        "	Temperature,      !- Control Variable",
        "	Normal, !- Action",
        "	Flow,   !- Actuator Variable",
        "	AHU Heating Coil Outlet,   !- Sensor Node Name",
        "	AHU Heating COil HW Inlet, !- Actuator Node Name",
        "	autosize, !- Controller Convergence Tolerance { deltaC }",
        "	autosize, !- Maximum Actuated Flow { m3 / s }",
        "	0;        !- Minimum Actuated Flow { m3 / s }",

        " AirLoopHVAC:ControllerList,",
        "	AHU controllers,    !- Name",
        " Controller:WaterCoil, !- Controller 1 Object Type",
        "	AHU cooling coil controller, !- Controller 1 Name",
        " Controller:WaterCoil, !- Controller 2 Object Type",
        "	AHU Heating coil;   !- Controller 2 Name",

        " Fan:VariableVolume,",
        "   AHU supply fan, !- Name",
        "   AvailSched,     !- Availability Schedule Name",
        "   0.7,            !- Fan Total Efficiency",
        "   996.355828557053, !- Pressure Rise { Pa }",
        "   autosize, !- Maximum Flow Rate { m3 / s }",
        "   Fraction, !- Fan Power Minimum Flow Rate Input Method",
        "   0,        !- Fan Power Minimum Flow Fraction",
        "   0,        !- Fan Power Minimum Air Flow Rate { m3 / s }",
        "   0.95,     !- Motor Efficiency",
        "   1,        !- Motor In Airstream Fraction",
        "   0.35071223, !- Fan Power Coefficient 1",
        "   0.30850535, !- Fan Power Coefficient 2",
        "  -0.54137364, !- Fan Power Coefficient 3",
        "   0.87198823, !- Fan Power Coefficient 4",
        "   0,          !- Fan Power Coefficient 5",
        "   AHU Heating Coil Outlet,  !- Air Inlet Node Name",
        "   AHU Supply fan Outlet, !- Air Outlet Node Name",
        "   General;               !- End - Use Subcategory",

        " Branch,",
        "   AHU Main Branch, !- Name",
        "	,         !- Pressure Drop Curve Name",
        "   AirLoopHVAC:OutdoorAirSystem, !- Component 1 Object Type",
        "   AHU OA system,           !- Component 1 Name",
        "   AHU air loop inlet,      !- Component 1 Inlet Node Name",
        "   AHU mixed air outlet,    !- Component 1 Outlet Node Name",
        " Coil:Cooling:water,        !- Component 2 Object Type",
        "   AHU cooling coil,        !- Component 2 Name",
        "   AHU mixed air outlet,    !- Component 2 Inlet Node Name",
        "   AHU cooling coil outlet, !- Component 2 Outlet Node Name",
        " Coil:Heating:Water,        !- Component 3 Object Type",
        "   AHU Heating coil,        !- Component 3 Name",
        "   AHU cooling coil outlet, !- Component 3 Inlet Node Name",
        "   AHU Heating Coil Outlet, !- Component 3 Outlet Node Name",
        " Fan:VariableVolume,        !- Component 5 Object Type",
        "   AHU Supply Fan,          !- Component 5 Name",
        "   AHU Heating Coil Outlet, !- Component 5 Inlet Node Name",
        "   AHU Supply fan Outlet;   !- Component 5 Outlet Node Name",

        " AirLoopHVAC,",
        "   AHU,                   !- Name",
        "   AHU controllers,       !- Controller List Name",
        "   ,                      !- Availability Manager List Name",
        "   autosize,              !- Design Supply Air Flow Rate { m3 / s }",
        "   AHU Branches,          !- Branch List Name",
        "   ,                      !- Connector List Name",
        "   AHU air loop inlet,    !- Supply Side Inlet Node Name",
        "   , !- Demand Side Outlet Node Name", // blank to avoid need zone equipment inputs
        "   AHU Supply Path Inlet, !- Demand Side Inlet Node Names",
        "   AHU Supply fan Outlet; !- Supply Side Outlet Node Names",

        " BranchList,",
        "   AHU Branches,          !- Name",
        "   AHU Main Branch;       !- Branch 1 Name",

        " AirLoopHVAC:ReturnPath,",
        "   AHU return path,       !- Name",
        "   AHU return air outlet, !- Return Air Path Outlet Node Name",
        " AirLoopHVAC:ZoneMixer,   !- Component 1 Object Type",
        "   AHU zone mixer;        !- Component 1 Name",

        " AirLoopHVAC:ZoneMixer,",
        "   AHU zone mixer,        !- Name",
        "   AHU return air outlet, !- Outlet Node Name",
        "   Main FL1 Return Outlet;!- Inlet 1 Node Name",

        " AirLoopHVAC:OutdoorAirSystem:EquipmentList,",
        "   AHU System equipment,  !- Name",
        " OutdoorAir:Mixer,        !- Component 1 Object Type",
        "   AHU OA Mixing Box;     !- Component 1 Name",

        " AirLoopHVAC:OutdoorAirSystem,",
        "   AHU OA System,             !- Name",
        "   AHU OA system controllers, !- Controller List Name",
        "   AHU System equipment;      !- Outdoor Air Equipment List Name",

        " OutdoorAir:Mixer,",
        "   AHU OA Mixing Box,         !- Name",
        "   AHU mixed air outlet,      !- Mixed Air Node Name",
        "   AHU Outside Air Inlet, !- Outdoor Air Stream Node Name",
        "   AHU relief air outlet,     !- Relief Air Stream Node Name",
        "   AHU air loop inlet;        !- Return Air Stream Node Name",

        " AirLoopHVAC:ControllerList,",
        "   AHU OA system controllers, !- Name",
        " Controller:OutdoorAir,       !- Controller 1 Object Type",
        "   AHU OA Controller;         !- Controller 1 Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    SimAirServingZones::GetAirPathData(*state);

    // 2 controllers on this AHU for 2 water coils on the branch
    // CanBeLockedOutByEcono should be false for both controller in this test
    EXPECT_FALSE(state->dataAirSystemsData->PrimaryAirSystems(1).CanBeLockedOutByEcono(1));
    EXPECT_FALSE(state->dataAirSystemsData->PrimaryAirSystems(1).CanBeLockedOutByEcono(2));
}

TEST_F(EnergyPlusFixture, GetAirPathData_ControllerLockout2)
{
    // this unit test is related to issue #5973 checks for controller economizer lockout
    std::string const idf_objects = delimited_string({
        " Coil:Cooling:Water,",
        "	AHU cooling coil,	!- Name",
        "	,       			!- Availability Schedule Name",
        "	autosize,			!- Design Water Flow Rate { m3 / s }",
        "	autosize,			!- Design Air Flow Rate { m3 / s }",
        "	autosize,			!- Design Inlet Water Temperature { C }",
        "	autosize,			!- Design Inlet Air Temperature { C }",
        "	autosize,			!- Design Outlet Air Temperature { C }",
        "	autosize,			!- Design Inlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	autosize,			!- Design Outlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	Water Inlet Node,	!- Water Inlet Node Name",
        "	Water Outlet Node,  !- Water Outlet Node Name",
        "	AHU Outside Air Inlet,		!- Air Inlet Node Name",
        "	AHU cooling coil outlet,	!- Air Outlet Node Name",
        "	SimpleAnalysis,		!- Type of Analysis",
        "	CrossFlow;          !- Heat Exchanger Configuration",

        " Coil:Heating:Water,",
        "	AHU Heating coil, !- Name",
        "	,         !- Availability Schedule Name",
        "	autosize, !- U - Factor Times Area Value { W / K }",
        "	autosize, !- Maximum Water Flow Rate { m3 / s }",
        "	AHU Heating COil HW Inlet, !- Water Inlet Node Name",
        "	AHU Heating COil HW Outlet, !- Water Outlet Node Name",
        "	AHU cooling coil outlet, !- Air Inlet Node Name",
        "	AHU Heating Coil Outlet, !- Air Outlet Node Name",
        "	UFactorTimesAreaAndDesignWaterFlowRate, !- Performance Input Method",
        "	autosize, !- Rated Capacity { W }",
        "	82.2, !- Rated Inlet Water Temperature { C }",
        "	16.6, !- Rated Inlet Air Temperature { C }",
        "	71.1, !- Rated Outlet Water Temperature { C }",
        "	48.8888888888889, !- Rated Outlet Air Temperature { C }",
        "	1;          !- Rated Ratio for Air and Water Convection",

        " Controller:WaterCoil,",
        "	AHU cooling coil controller, !- Name",
        "	TemperatureAndHumidityRatio,		!- Control Variable",
        "	Reverse,			!- Action",
        "	FLOW,				!- Actuator Variable",
        "	AHU cooling coil outlet,	!- Sensor Node Name",
        "	Water Inlet Node,	!- Actuator Node Name",
        "	autosize,			!- Controller Convergence Tolerance { deltaC }",
        "	autosize,			!- Maximum Actuated Flow { m3 / s }",
        "	0.0;				!- Minimum Actuated Flow { m3 / s }",

        " Controller:WaterCoil,",
        "	AHU Heating coil, !- Name",
        "	Temperature,      !- Control Variable",
        "	Normal, !- Action",
        "	Flow,   !- Actuator Variable",
        "	AHU Heating Coil Outlet,   !- Sensor Node Name",
        "	AHU Heating COil HW Inlet, !- Actuator Node Name",
        "	autosize, !- Controller Convergence Tolerance { deltaC }",
        "	autosize, !- Maximum Actuated Flow { m3 / s }",
        "	0;        !- Minimum Actuated Flow { m3 / s }",

        " Fan:VariableVolume,",
        "   AHU supply fan, !- Name",
        "   ,               !- Availability Schedule Name",
        "   0.7,            !- Fan Total Efficiency",
        "   996.355828557053, !- Pressure Rise { Pa }",
        "   autosize, !- Maximum Flow Rate { m3 / s }",
        "   Fraction, !- Fan Power Minimum Flow Rate Input Method",
        "   0,        !- Fan Power Minimum Flow Fraction",
        "   0,        !- Fan Power Minimum Air Flow Rate { m3 / s }",
        "   0.95,     !- Motor Efficiency",
        "   1,        !- Motor In Airstream Fraction",
        "   0.35071223, !- Fan Power Coefficient 1",
        "   0.30850535, !- Fan Power Coefficient 2",
        "  -0.54137364, !- Fan Power Coefficient 3",
        "   0.87198823, !- Fan Power Coefficient 4",
        "   0,          !- Fan Power Coefficient 5",
        "   AHU mixed air outlet,  !- Air Inlet Node Name",
        "   AHU Supply fan Outlet, !- Air Outlet Node Name",
        "   General;               !- End - Use Subcategory",

        " Branch,",
        "   AHU Main Branch, !- Name",
        "	,         !- Pressure Drop Curve Name",
        "   AirLoopHVAC:OutdoorAirSystem, !- Component 1 Object Type",
        "   AHU OA system,           !- Component 1 Name",
        "   AHU air loop inlet,      !- Component 1 Inlet Node Name",
        "   AHU mixed air outlet,    !- Component 1 Outlet Node Name",
        " Fan:VariableVolume,        !- Component 5 Object Type",
        "   AHU Supply Fan,          !- Component 5 Name",
        "   AHU mixed air outlet,    !- Component 5 Inlet Node Name",
        "   AHU Supply fan Outlet;   !- Component 5 Outlet Node Name",

        " AirLoopHVAC,",
        "   AHU,                   !- Name",
        "   ,                      !- Controller List Name",
        "   ,                      !- Availability Manager List Name",
        "   autosize,              !- Design Supply Air Flow Rate { m3 / s }",
        "   AHU Branches,          !- Branch List Name",
        "   ,                      !- Connector List Name",
        "   AHU air loop inlet,    !- Supply Side Inlet Node Name",
        "   , !- Demand Side Outlet Node Name", // blank to avoid need zone equipment inputs
        "   AHU Supply Path Inlet, !- Demand Side Inlet Node Names",
        "   AHU Supply fan Outlet; !- Supply Side Outlet Node Names",

        " BranchList,",
        "   AHU Branches,          !- Name",
        "   AHU Main Branch;       !- Branch 1 Name",

        " AirLoopHVAC:ReturnPath,",
        "   AHU return path,       !- Name",
        "   AHU return air outlet, !- Return Air Path Outlet Node Name",
        " AirLoopHVAC:ZoneMixer,   !- Component 1 Object Type",
        "   AHU zone mixer;        !- Component 1 Name",

        " AirLoopHVAC:ZoneMixer,",
        "   AHU zone mixer,        !- Name",
        "   AHU return air outlet, !- Outlet Node Name",
        "   Main FL1 Return Outlet;!- Inlet 1 Node Name",

        " AirLoopHVAC:OutdoorAirSystem:EquipmentList,",
        "   AHU System equipment,  !- Name",
        " Coil:Cooling:water,        !- Component 2 Object Type",
        "   AHU cooling coil,        !- Component 2 Name",
        " Coil:Heating:Water,        !- Component 3 Object Type",
        "   AHU Heating coil,        !- Component 3 Name",
        " OutdoorAir:Mixer,        !- Component 1 Object Type",
        "   AHU OA Mixing Box;     !- Component 1 Name",

        " AirLoopHVAC:OutdoorAirSystem,",
        "   AHU OA System,             !- Name",
        "   AHU OA system controllers, !- Controller List Name",
        "   AHU System equipment;      !- Outdoor Air Equipment List Name",

        " OutdoorAir:Mixer,",
        "   AHU OA Mixing Box,         !- Name",
        "   AHU mixed air outlet,      !- Mixed Air Node Name",
        "   AHU Heating Coil Outlet,   !- Outdoor Air Stream Node Name",
        "   AHU relief air outlet,     !- Relief Air Stream Node Name",
        "   AHU air loop inlet;        !- Return Air Stream Node Name",

        " AirLoopHVAC:ControllerList,",
        "   AHU OA system controllers, !- Name",
        " Controller:OutdoorAir,       !- Controller 1 Object Type",
        "   AHU OA Controller,         !- Controller 1 Name",
        " Controller:WaterCoil, !- Controller 2 Object Type",
        "	AHU Heating coil,   !- Controller 2 Name",
        " Controller:WaterCoil, !- Controller 1 Object Type",
        "	AHU cooling coil controller; !- Controller 1 Name",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    SimAirServingZones::GetAirPathData(*state);

    // 2 controllers on this AHU for 2 water coils in the OA system
    // CanBeLockedOutByEcono should be false for the heating coil controller #1 in this test
    // CanBeLockedOutByEcono should be true for the cooling coil controller #2 in this test
    EXPECT_FALSE(state->dataAirSystemsData->PrimaryAirSystems(1).CanBeLockedOutByEcono(1));
    EXPECT_TRUE(state->dataAirSystemsData->PrimaryAirSystems(1).CanBeLockedOutByEcono(2));
}

TEST_F(EnergyPlusFixture, InitAirLoops_1AirLoop2ADU)
{

    std::string const idf_objects = delimited_string({
        "Zone,",
        "  Space1;                  !- Name",

        "Zone,",
        "  Space2;                !- Name",

        "  ZoneHVAC:EquipmentConnections,",
        "    Space1,                   !- Zone Name",
        "    Space1 Eq,                !- Zone Conditioning Equipment List Name",
        "    Space1-1 In Node,         !- Zone Air Inlet Node or NodeList Name",
        "    Space1 Out Node,          !- Zone Air Exhaust Node or NodeList Name",
        "    Space1 Node,              !- Zone Air Node Name",
        "    Space1 Ret Node;          !- Zone Return Air Node Name",

        "  ZoneHVAC:EquipmentConnections,",
        "    Space2,                   !- Zone Name",
        "    Space2 Eq,                !- Zone Conditioning Equipment List Name",
        "    Space2-1 In Node,         !- Zone Air Inlet Node or NodeList Name",
        "    Space2 Out Node,          !- Zone Air Exhaust Node or NodeList Name",
        "    Space2 Node,              !- Zone Air Node Name",
        "    Space2 Ret Node;          !- Zone Return Air Node Name",

        "  ZoneHVAC:EquipmentList,",
        "    Space1 Eq,               !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE1-1 ATU,            !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No - Load Sequence",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SPACE1-1 ATU,            !- Name",
        "    Space1-1 In Node,        !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "    SPACE1-1 VAV Reheat;     !- Air Terminal Name",

        "  ZoneHVAC:EquipmentList,",
        "    Space2 Eq,               !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE2-1 ATU,            !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No - Load Sequence",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SPACE2-1 ATU,            !- Name",
        "    Space2-1 In Node,        !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "    SPACE2-1 VAV Reheat;     !- Air Terminal Name",

        "  AirTerminal:SingleDuct:VAV:NoReheat,",
        "    SPACE1-1 VAV Reheat,     !- Name",
        "    ,                        !- Availability Schedule Name",
        "    Space1-1 In Node,        !- Air Outlet Node Name",
        "    SPACE1-1 ATU In Node,    !- Air Inlet Node Name",
        "    1.0,                     !- Maximum Air Flow Rate {m3/s}",
        "    Constant,                !- Zone Minimum Air Flow Input Method",
        "    0.3;                     !- Constant Minimum Air Flow Fraction",

        "  AirTerminal:SingleDuct:VAV:NoReheat,",
        "    SPACE2-1 VAV Reheat,     !- Name",
        "    ,                        !- Availability Schedule Name",
        "    Space2-1 In Node,        !- Air Outlet Node Name",
        "    SPACE2-1 ATU In Node,    !- Air Inlet Node Name",
        "    1.0,                     !- Maximum Air Flow Rate {m3/s}",
        "    Constant,                !- Zone Minimum Air Flow Input Method",
        "    0.3;                     !- Constant Minimum Air Flow Fraction",

        "  BranchList,",
        "    VAV Sys 1 Branches,      !- Name",
        "    VAV Sys 1 Main Branch;   !- Branch 1 Name",

        "  Branch,",
        "    VAV Sys 1 Main Branch,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Fan:VariableVolume,      !- Component 4 Object Type",
        "    Supply Fan 1,            !- Component 4 Name",
        "    VAV Sys 1 Inlet Node,    !- Component 4 Inlet Node Name",
        "    VAV Sys 1 Outlet Node;   !- Component 4 Outlet Node Name",

        "  AirLoopHVAC,",
        "    VAV Sys 1,               !- Name",
        "    ,                        !- Controller List Name",
        "    VAV Sys 1 Avail List,    !- Availability Manager List Name",
        "    1.0,                     !- Design Supply Air Flow Rate {m3/s}",
        "    VAV Sys 1 Branches,      !- Branch List Name",
        "    ,                        !- Connector List Name",
        "    VAV Sys 1 Inlet Node,    !- Supply Side Inlet Node Name",
        "    Demand Out Node,         !- Demand Side Outlet Node Name",
        "    Zone Eq In Node,         !- Demand Side Inlet Node Names",
        "    VAV Sys 1 Outlet Node;   !- Supply Side Outlet Node Names",

        "  AirLoopHVAC:SupplyPath,",
        "    Zone Supply Air Path 1,  !- Name",
        "    Zone Eq In Node,         !- Supply Air Path Inlet Node Name",
        "    AirLoopHVAC:ZoneSplitter,!- Component 1 Object Type",
        "    Zone Supply Air Splitter;  !- Component 1 Name",

        "  AirLoopHVAC:ZoneSplitter,",
        "    Zone Supply Air Splitter,  !- Name",
        "    Zone Eq In Node,         !- Inlet Node Name",
        "    SPACE1-1 ATU In Node,    !- Outlet 1 Node Name",
        "    SPACE2-1 ATU In Node;    !- Outlet 2 Node Name",

        "  AirLoopHVAC:ReturnPath,",
        "    ReturnAirPath1,          !- Name",
        "    Demand Out Node,         !- Return Air Path Outlet Node Name",
        "    AirLoopHVAC:ZoneMixer,   !- Component 1 Object Type",
        "    Zone Return Air Mixer;   !- Component 1 Name",

        "  AirLoopHVAC:ZoneMixer,",
        "    Zone Return Air Mixer,   !- Name",
        "    Demand Out Node,         !- Outlet Node Name",
        "    Space1 Ret Node,         !- Inlet 1 Node Name",
        "    Space2 Ret Node;         !- Inlet 2 Node Name",

        "  Fan:VariableVolume,",
        "    Supply Fan 1,            !- Name",
        "    ,                        !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    600.0,                   !- Pressure Rise {Pa}",
        "    1.0,                     !- Maximum Flow Rate {m3/s}",
        "    FixedFlowRate,           !- Fan Power Minimum Flow Rate Input Method",
        "    ,                        !- Fan Power Minimum Flow Fraction",
        "    0.35326,                 !- Fan Power Minimum Air Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1.0,                     !- Motor In Airstream Fraction",
        "    0.0015302446,            !- Fan Power Coefficient 1",
        "    0.0052080574,            !- Fan Power Coefficient 2",
        "    1.1086242,               !- Fan Power Coefficient 3",
        "    -0.11635563,             !- Fan Power Coefficient 4",
        "    0.000,                   !- Fan Power Coefficient 5",
        "    VAV Sys 1 Inlet Node,    !- Air Inlet Node Name",
        "    VAV Sys 1 Outlet Node;   !- Air Outlet Node Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound = false;
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    EXPECT_TRUE(compare_err_stream(""));
    DataZoneEquipment::GetZoneEquipmentData(*state);
    EXPECT_TRUE(compare_err_stream(""));
    ASSERT_FALSE(ErrorsFound);
    ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment(*state);
    EXPECT_TRUE(compare_err_stream(""));
    ASSERT_FALSE(ErrorsFound);
    SingleDuct::GetSysInput(*state);
    EXPECT_TRUE(compare_err_stream(""));
    ASSERT_FALSE(ErrorsFound);
    SplitterComponent::GetSplitterInput(*state);
    EXPECT_TRUE(compare_err_stream(""));
    SimAirServingZones::GetAirPathData(*state);
    // Expect warnings about no controllers, clear err_stream
    EXPECT_TRUE(has_err_output(true));
    SimAirServingZones::InitAirLoops(*state, true);
    EXPECT_TRUE(compare_err_stream(""));
    ASSERT_FALSE(ErrorsFound);
    // And finally, all of this gymnastics just to check if the airloopnums get set correctly
    // For this test, both ADUs should be connected airloop 1 which is the only one here
    EXPECT_EQ(state->dataZoneEquip->ZoneEquipConfig(1).InletNodeAirLoopNum(1), 1);
    EXPECT_EQ(state->dataZoneEquip->ZoneEquipConfig(2).InletNodeAirLoopNum(1), 1);
}

TEST_F(EnergyPlusFixture, InitAirLoops_2AirLoop2ADU)
{

    std::string const idf_objects = delimited_string({
        "Zone,",
        "  Space1;                  !- Name",

        "Zone,",
        "  Space2;                !- Name",

        "  ZoneHVAC:EquipmentConnections,",
        "    Space1,                   !- Zone Name",
        "    Space1 Eq,                !- Zone Conditioning Equipment List Name",
        "    Space1-1 In Node,         !- Zone Air Inlet Node or NodeList Name",
        "    Space1 Out Node,          !- Zone Air Exhaust Node or NodeList Name",
        "    Space1 Node,              !- Zone Air Node Name",
        "    Space1 Ret Node;          !- Zone Return Air Node Name",

        "  ZoneHVAC:EquipmentConnections,",
        "    Space2,                   !- Zone Name",
        "    Space2 Eq,                !- Zone Conditioning Equipment List Name",
        "    Space2-1 In Node,         !- Zone Air Inlet Node or NodeList Name",
        "    Space2 Out Node,          !- Zone Air Exhaust Node or NodeList Name",
        "    Space2 Node,              !- Zone Air Node Name",
        "    Space2 Ret Node;          !- Zone Return Air Node Name",

        "  ZoneHVAC:EquipmentList,",
        "    Space1 Eq,               !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE1-1 ATU,            !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No - Load Sequence",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SPACE1-1 ATU,            !- Name",
        "    Space1-1 In Node,        !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "    SPACE1-1 VAV Reheat;     !- Air Terminal Name",

        "  ZoneHVAC:EquipmentList,",
        "    Space2 Eq,               !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE2-1 ATU,            !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No - Load Sequence",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SPACE2-1 ATU,            !- Name",
        "    Space2-1 In Node,        !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "    SPACE2-1 VAV Reheat;     !- Air Terminal Name",

        "  AirTerminal:SingleDuct:VAV:NoReheat,",
        "    SPACE1-1 VAV Reheat,     !- Name",
        "    ,                        !- Availability Schedule Name",
        "    Space1-1 In Node,        !- Air Outlet Node Name",
        "    SPACE1-1 ATU In Node,    !- Air Inlet Node Name",
        "    1.0,                     !- Maximum Air Flow Rate {m3/s}",
        "    Constant,                !- Zone Minimum Air Flow Input Method",
        "    0.3;                     !- Constant Minimum Air Flow Fraction",

        "  AirTerminal:SingleDuct:VAV:NoReheat,",
        "    SPACE2-1 VAV Reheat,     !- Name",
        "    ,                        !- Availability Schedule Name",
        "    Space2-1 In Node,        !- Air Outlet Node Name",
        "    SPACE2-1 ATU In Node,    !- Air Inlet Node Name",
        "    1.0,                     !- Maximum Air Flow Rate {m3/s}",
        "    Constant,                !- Zone Minimum Air Flow Input Method",
        "    0.3;                     !- Constant Minimum Air Flow Fraction",

        "  BranchList,",
        "    VAV Sys 1 Branches,      !- Name",
        "    VAV Sys 1 Main Branch;   !- Branch 1 Name",

        "  Branch,",
        "    VAV Sys 1 Main Branch,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Fan:VariableVolume,      !- Component 4 Object Type",
        "    Supply Fan 1,            !- Component 4 Name",
        "    VAV Sys 1 Inlet Node,    !- Component 4 Inlet Node Name",
        "    VAV Sys 1 Outlet Node;   !- Component 4 Outlet Node Name",

        "  AirLoopHVAC,",
        "    VAV Sys 1,               !- Name",
        "    ,                        !- Controller List Name",
        "    VAV Sys 1 Avail List,    !- Availability Manager List Name",
        "    1.0,                     !- Design Supply Air Flow Rate {m3/s}",
        "    VAV Sys 1 Branches,      !- Branch List Name",
        "    ,                        !- Connector List Name",
        "    VAV Sys 1 Inlet Node,    !- Supply Side Inlet Node Name",
        "    Demand Out Node,         !- Demand Side Outlet Node Name",
        "    Zone Eq In Node,         !- Demand Side Inlet Node Names",
        "    VAV Sys 1 Outlet Node;   !- Supply Side Outlet Node Names",

        "  AirLoopHVAC:SupplyPath,",
        "    Zone Supply Air Path 1,  !- Name",
        "    Zone Eq In Node,         !- Supply Air Path Inlet Node Name",
        "    AirLoopHVAC:ZoneSplitter,!- Component 1 Object Type",
        "    Zone Supply Air Splitter;  !- Component 1 Name",

        "  AirLoopHVAC:ZoneSplitter,",
        "    Zone Supply Air Splitter,  !- Name",
        "    Zone Eq In Node,         !- Inlet Node Name",
        "    SPACE1-1 ATU In Node;    !- Outlet 1 Node Name",

        "  AirLoopHVAC:ReturnPath,",
        "    ReturnAirPath1,          !- Name",
        "    Demand Out Node,         !- Return Air Path Outlet Node Name",
        "    AirLoopHVAC:ZoneMixer,   !- Component 1 Object Type",
        "    Zone Return Air Mixer;   !- Component 1 Name",

        "  AirLoopHVAC:ZoneMixer,",
        "    Zone Return Air Mixer,   !- Name",
        "    Demand Out Node,         !- Outlet Node Name",
        "    Space1 Ret Node;         !- Inlet 1 Node Name",

        "  Fan:VariableVolume,",
        "    Supply Fan 1,            !- Name",
        "    ,                        !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    600.0,                   !- Pressure Rise {Pa}",
        "    1.0,                     !- Maximum Flow Rate {m3/s}",
        "    FixedFlowRate,           !- Fan Power Minimum Flow Rate Input Method",
        "    ,                        !- Fan Power Minimum Flow Fraction",
        "    0.35326,                 !- Fan Power Minimum Air Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1.0,                     !- Motor In Airstream Fraction",
        "    0.0015302446,            !- Fan Power Coefficient 1",
        "    0.0052080574,            !- Fan Power Coefficient 2",
        "    1.1086242,               !- Fan Power Coefficient 3",
        "    -0.11635563,             !- Fan Power Coefficient 4",
        "    0.000,                   !- Fan Power Coefficient 5",
        "    VAV Sys 1 Inlet Node,    !- Air Inlet Node Name",
        "    VAV Sys 1 Outlet Node;   !- Air Outlet Node Name",

        "  BranchList,",
        "    VAV Sys 2 Branches,      !- Name",
        "    VAV Sys 2 Main Branch;   !- Branch 1 Name",

        "  Branch,",
        "    VAV Sys 2 Main Branch,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Fan:VariableVolume,      !- Component 4 Object Type",
        "    Supply Fan 2,            !- Component 4 Name",
        "    VAV Sys 2 Inlet Node,    !- Component 4 Inlet Node Name",
        "    VAV Sys 2 Outlet Node;   !- Component 4 Outlet Node Name",

        "  AirLoopHVAC,",
        "    VAV Sys 2,               !- Name",
        "    ,                        !- Controller List Name",
        "    VAV Sys 2 Avail List,    !- Availability Manager List Name",
        "    1.0,                     !- Design Supply Air Flow Rate {m3/s}",
        "    VAV Sys 2 Branches,      !- Branch List Name",
        "    ,                        !- Connector List Name",
        "    VAV Sys 2 Inlet Node,    !- Supply Side Inlet Node Name",
        "    Demand 2 Out Node,         !- Demand Side Outlet Node Name",
        "    Zone Eq 2 In Node,         !- Demand Side Inlet Node Names",
        "    VAV Sys 2 Outlet Node;   !- Supply Side Outlet Node Names",

        "  AirLoopHVAC:SupplyPath,",
        "    Zone Supply Air Path 2,  !- Name",
        "    Zone Eq 2 In Node,         !- Supply Air Path Inlet Node Name",
        "    AirLoopHVAC:ZoneSplitter,!- Component 1 Object Type",
        "    Zone Supply Air Splitter 2;  !- Component 1 Name",

        "  AirLoopHVAC:ZoneSplitter,",
        "    Zone Supply Air Splitter 2,  !- Name",
        "    Zone Eq 2 In Node,         !- Inlet Node Name",
        "    SPACE2-1 ATU In Node;    !- Outlet 2 Node Name",

        "  AirLoopHVAC:ReturnPath,",
        "    ReturnAirPath2,          !- Name",
        "    Demand 2 Out Node,         !- Return Air Path Outlet Node Name",
        "    AirLoopHVAC:ZoneMixer,   !- Component 1 Object Type",
        "    Zone Return Air Mixer 2;   !- Component 1 Name",

        "  AirLoopHVAC:ZoneMixer,",
        "    Zone Return Air Mixer 2,   !- Name",
        "    Demand 2 Out Node,         !- Outlet Node Name",
        "    Space2 Ret Node;         !- Inlet 2 Node Name",

        "  Fan:VariableVolume,",
        "    Supply Fan 2,            !- Name",
        "    ,                        !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    600.0,                   !- Pressure Rise {Pa}",
        "    1.0,                     !- Maximum Flow Rate {m3/s}",
        "    FixedFlowRate,           !- Fan Power Minimum Flow Rate Input Method",
        "    ,                        !- Fan Power Minimum Flow Fraction",
        "    0.35326,                 !- Fan Power Minimum Air Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1.0,                     !- Motor In Airstream Fraction",
        "    0.0015302446,            !- Fan Power Coefficient 1",
        "    0.0052080574,            !- Fan Power Coefficient 2",
        "    1.1086242,               !- Fan Power Coefficient 3",
        "    -0.11635563,             !- Fan Power Coefficient 4",
        "    0.000,                   !- Fan Power Coefficient 5",
        "    VAV Sys 2 Inlet Node,    !- Air Inlet Node Name",
        "    VAV Sys 2 Outlet Node;   !- Air Outlet Node Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound = false;
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    EXPECT_TRUE(compare_err_stream(""));
    DataZoneEquipment::GetZoneEquipmentData(*state);
    EXPECT_TRUE(compare_err_stream(""));
    ASSERT_FALSE(ErrorsFound);
    ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment(*state);
    EXPECT_TRUE(compare_err_stream(""));
    ASSERT_FALSE(ErrorsFound);
    SingleDuct::GetSysInput(*state);
    EXPECT_TRUE(compare_err_stream(""));
    ASSERT_FALSE(ErrorsFound);
    SplitterComponent::GetSplitterInput(*state);
    EXPECT_TRUE(compare_err_stream(""));
    SimAirServingZones::GetAirPathData(*state);
    // Expect warnings about no controllers, clear err_stream
    EXPECT_TRUE(has_err_output(true));
    SimAirServingZones::InitAirLoops(*state, true);
    EXPECT_TRUE(compare_err_stream(""));
    ASSERT_FALSE(ErrorsFound);
    // And finally, all of this gymnastics just to check if the airloopnums get set correctly
    // For this test, each ADUs should be connected to a different airloop
    EXPECT_EQ(state->dataZoneEquip->ZoneEquipConfig(1).InletNodeAirLoopNum(1), 1);
    EXPECT_EQ(state->dataZoneEquip->ZoneEquipConfig(2).InletNodeAirLoopNum(1), 2);
}

// this test checks whether an AirLoop with 0 airflow will raise the correct severe error
TEST_F(EnergyPlusFixture, SizeAirLoopBranches_0Airflow)
{
    std::string const idf_objects = delimited_string({
        "Zone,",
        "  Space1;                  !- Name",

        "  ZoneHVAC:EquipmentConnections,",
        "    Space1,                   !- Zone Name",
        "    Space1 Eq,                !- Zone Conditioning Equipment List Name",
        "    Space1-1 In Node,         !- Zone Air Inlet Node or NodeList Name",
        "    Space1 Out Node,          !- Zone Air Exhaust Node or NodeList Name",
        "    Space1 Node,              !- Zone Air Node Name",
        "    Space1 Ret Node;          !- Zone Return Air Node Name",

        "  ZoneHVAC:EquipmentList,",
        "    Space1 Eq,               !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE1-1 ATU,            !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No - Load Sequence",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SPACE1-1 ATU,            !- Name",
        "    Space1-1 In Node,        !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "    SPACE1-1 VAV Reheat;     !- Air Terminal Name",

        "  AirTerminal:SingleDuct:VAV:NoReheat,",
        "    SPACE1-1 VAV Reheat,     !- Name",
        "    ,                        !- Availability Schedule Name",
        "    Space1-1 In Node,        !- Air Outlet Node Name",
        "    SPACE1-1 ATU In Node,    !- Air Inlet Node Name",
        "    0,                       !- Maximum Air Flow Rate {m3/s}",
        "    Constant,                !- Zone Minimum Air Flow Input Method",
        "    0.3;                     !- Constant Minimum Air Flow Fraction",

        "  BranchList,",
        "    VAV Sys 1 Branches,      !- Name",
        "    VAV Sys 1 Main Branch;   !- Branch 1 Name",

        "  Branch,",
        "    VAV Sys 1 Main Branch,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Fan:VariableVolume,      !- Component 4 Object Type",
        "    Supply Fan 1,            !- Component 4 Name",
        "    VAV Sys 1 Inlet Node,    !- Component 4 Inlet Node Name",
        "    VAV Sys 1 Outlet Node;   !- Component 4 Outlet Node Name",

        "  AirLoopHVAC,",
        "    VAV Sys 1,               !- Name",
        "    ,                        !- Controller List Name",
        "    VAV Sys 1 Avail List,    !- Availability Manager List Name",
        "    0.0,                     !- Design Supply Air Flow Rate {m3/s}",
        "    VAV Sys 1 Branches,      !- Branch List Name",
        "    ,                        !- Connector List Name",
        "    VAV Sys 1 Inlet Node,    !- Supply Side Inlet Node Name",
        "    Demand Out Node,         !- Demand Side Outlet Node Name",
        "    Zone Eq In Node,         !- Demand Side Inlet Node Names",
        "    VAV Sys 1 Outlet Node;   !- Supply Side Outlet Node Names",

        "  AirLoopHVAC:SupplyPath,",
        "    Zone Supply Air Path 1,  !- Name",
        "    Zone Eq In Node,         !- Supply Air Path Inlet Node Name",
        "    AirLoopHVAC:ZoneSplitter,!- Component 1 Object Type",
        "    Zone Supply Air Splitter;  !- Component 1 Name",

        "  AirLoopHVAC:ZoneSplitter,",
        "    Zone Supply Air Splitter,  !- Name",
        "    Zone Eq In Node,         !- Inlet Node Name",
        "    SPACE1-1 ATU In Node;    !- Outlet 1 Node Name",

        "  AirLoopHVAC:ReturnPath,",
        "    ReturnAirPath1,          !- Name",
        "    Demand Out Node,         !- Return Air Path Outlet Node Name",
        "    AirLoopHVAC:ZoneMixer,   !- Component 1 Object Type",
        "    Zone Return Air Mixer;   !- Component 1 Name",

        "  AirLoopHVAC:ZoneMixer,",
        "    Zone Return Air Mixer,   !- Name",
        "    Demand Out Node,         !- Outlet Node Name",
        "    Space1 Ret Node;         !- Inlet 1 Node Name",

        "  Fan:VariableVolume,",
        "    Supply Fan 1,            !- Name",
        "    ,                        !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    600.0,                   !- Pressure Rise {Pa}",
        "    autosize,                !- Maximum Flow Rate {m3/s}",
        "    FixedFlowRate,           !- Fan Power Minimum Flow Rate Input Method",
        "    ,                        !- Fan Power Minimum Flow Fraction",
        "    0.0,                 !- Fan Power Minimum Air Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1.0,                     !- Motor In Airstream Fraction",
        "    0.0015302446,            !- Fan Power Coefficient 1",
        "    0.0052080574,            !- Fan Power Coefficient 2",
        "    1.1086242,               !- Fan Power Coefficient 3",
        "    -0.11635563,             !- Fan Power Coefficient 4",
        "    0.000,                   !- Fan Power Coefficient 5",
        "    VAV Sys 1 Inlet Node,    !- Air Inlet Node Name",
        "    VAV Sys 1 Outlet Node;   !- Air Outlet Node Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound = false;
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    EXPECT_TRUE(compare_err_stream(""));
    DataZoneEquipment::GetZoneEquipmentData(*state);
    EXPECT_TRUE(compare_err_stream(""));
    ASSERT_FALSE(ErrorsFound);
    ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment(*state);
    EXPECT_TRUE(compare_err_stream(""));
    ASSERT_FALSE(ErrorsFound);
    SingleDuct::GetSysInput(*state);
    EXPECT_TRUE(compare_err_stream(""));
    ASSERT_FALSE(ErrorsFound);
    SplitterComponent::GetSplitterInput(*state);
    EXPECT_TRUE(compare_err_stream(""));
    SimAirServingZones::GetAirPathData(*state);
    // Expect warnings about no controllers, clear err_stream
    EXPECT_TRUE(has_err_output(true));
    SimAirServingZones::InitAirLoops(*state, true);
    // Expect warnings about 0 airflow
    std::string error_msg = delimited_string({
        "   ** Severe  ** SizeAirLoopBranches: AirLoopHVAC VAV SYS 1 has air flow less than 1.0000E-003 m3/s.",
        "   **   ~~~   ** Primary air system volumetric flow rate = 0.0000 m3/s.",
        "   **   ~~~   ** Check flow rate inputs for components in this air loop and,",
        "   **   ~~~   ** if autosized, check Sizing:Zone and Sizing:System objects and related inputs.",
    });
    EXPECT_TRUE(compare_err_stream(error_msg));
}

TEST_F(EnergyPlusFixture, InitAirLoops_2AirLoop3ADUa)
{
    std::string const idf_objects = delimited_string({
        "Zone,",
        "  Space1;                  !- Name",

        "Zone,",
        "  Space2;                !- Name",

        "  ZoneHVAC:EquipmentConnections,",
        "    Space1,                   !- Zone Name",
        "    Space1 Eq,                !- Zone Conditioning Equipment List Name",
        "    Space1-1 In Node,         !- Zone Air Inlet Node or NodeList Name",
        "    Space1 Out Node,          !- Zone Air Exhaust Node or NodeList Name",
        "    Space1 Node,              !- Zone Air Node Name",
        "    Space1 Ret Node;          !- Zone Return Air Node Name",

        "  ZoneHVAC:EquipmentConnections,",
        "    Space2,                   !- Zone Name",
        "    Space2 Eq,                !- Zone Conditioning Equipment List Name",
        "    Space2 Inlet Nodes,       !- Zone Air Inlet Node or NodeList Name",
        "    Space2 Out Node,          !- Zone Air Exhaust Node or NodeList Name",
        "    Space2 Node,              !- Zone Air Node Name",
        "    Space2 Return Nodes;      !- Zone Return Air Node Name",

        "  NodeList,",
        "    Space2 Inlet Nodes,       !- Name",
        "    Space2-1 In Node,         !- Node 1 Name",
        "    Space2-2 In Node;         !- Node 2 Name",

        "  NodeList,",
        "    Space2 Return Nodes,       !- Name",
        "    Space2-1 Ret Node,         !- Node 1 Name",
        "    Space2-2 Ret Node;         !- Node 2 Name",

        "  ZoneHVAC:EquipmentList,",
        "    Space1 Eq,               !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE1-1 ATU,            !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No - Load Sequence",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SPACE1-1 ATU,            !- Name",
        "    Space1-1 In Node,        !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "    SPACE1-1 VAV Reheat;     !- Air Terminal Name",

        "  ZoneHVAC:EquipmentList,",
        "    Space2 Eq,               !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE2-1 ATU,            !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No - Load Sequence",
        "    ,                        !- Zone cooling fraction thingy",
        "    ,                        !- Zone heating fraction thingy",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 2 Object Type",
        "    SPACE2-2 ATU,            !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    2;                       !- Zone Equipment 2 Heating or No - Load Sequence",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SPACE2-1 ATU,            !- Name",
        "    Space2-1 In Node,        !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "    SPACE2-1 VAV Reheat;     !- Air Terminal Name",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SPACE2-2 ATU,            !- Name",
        "    Space2-2 In Node,        !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "    SPACE2-2 VAV Reheat;     !- Air Terminal Name",

        "  AirTerminal:SingleDuct:VAV:NoReheat,",
        "    SPACE1-1 VAV Reheat,     !- Name",
        "    ,                        !- Availability Schedule Name",
        "    Space1-1 In Node,        !- Air Outlet Node Name",
        "    SPACE1-1 ATU In Node,    !- Air Inlet Node Name",
        "    1.0,                     !- Maximum Air Flow Rate {m3/s}",
        "    Constant,                !- Zone Minimum Air Flow Input Method",
        "    0.3;                     !- Constant Minimum Air Flow Fraction",

        "  AirTerminal:SingleDuct:VAV:NoReheat,",
        "    SPACE2-1 VAV Reheat,     !- Name",
        "    ,                        !- Availability Schedule Name",
        "    Space2-1 In Node,        !- Air Outlet Node Name",
        "    SPACE2-1 ATU In Node,    !- Air Inlet Node Name",
        "    1.0,                     !- Maximum Air Flow Rate {m3/s}",
        "    Constant,                !- Zone Minimum Air Flow Input Method",
        "    0.3;                     !- Constant Minimum Air Flow Fraction",

        "  AirTerminal:SingleDuct:VAV:NoReheat,",
        "    SPACE2-2 VAV Reheat,     !- Name",
        "    ,                        !- Availability Schedule Name",
        "    Space2-2 In Node,        !- Air Outlet Node Name",
        "    SPACE2-2 ATU In Node,    !- Air Inlet Node Name",
        "    1.0,                     !- Maximum Air Flow Rate {m3/s}",
        "    Constant,                !- Zone Minimum Air Flow Input Method",
        "    0.3;                     !- Constant Minimum Air Flow Fraction",

        "  BranchList,",
        "    VAV Sys 1 Branches,      !- Name",
        "    VAV Sys 1 Main Branch;   !- Branch 1 Name",

        "  Branch,",
        "    VAV Sys 1 Main Branch,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Fan:VariableVolume,      !- Component 4 Object Type",
        "    Supply Fan 1,            !- Component 4 Name",
        "    VAV Sys 1 Inlet Node,    !- Component 4 Inlet Node Name",
        "    VAV Sys 1 Outlet Node;   !- Component 4 Outlet Node Name",

        "  AirLoopHVAC,",
        "    VAV Sys 1,               !- Name",
        "    ,                        !- Controller List Name",
        "    VAV Sys 1 Avail List,    !- Availability Manager List Name",
        "    1.0,                     !- Design Supply Air Flow Rate {m3/s}",
        "    VAV Sys 1 Branches,      !- Branch List Name",
        "    ,                        !- Connector List Name",
        "    VAV Sys 1 Inlet Node,    !- Supply Side Inlet Node Name",
        "    Demand Out Node,         !- Demand Side Outlet Node Name",
        "    Zone Eq In Node,         !- Demand Side Inlet Node Names",
        "    VAV Sys 1 Outlet Node;   !- Supply Side Outlet Node Names",

        "  AirLoopHVAC:SupplyPath,",
        "    Zone Supply Air Path 1,  !- Name",
        "    Zone Eq In Node,         !- Supply Air Path Inlet Node Name",
        "    AirLoopHVAC:ZoneSplitter,!- Component 1 Object Type",
        "    Zone Supply Air Splitter;  !- Component 1 Name",

        "  AirLoopHVAC:ZoneSplitter,",
        "    Zone Supply Air Splitter,  !- Name",
        "    Zone Eq In Node,         !- Inlet Node Name",
        "    SPACE1-1 ATU In Node,    !- Outlet 1 Node Name",
        "    SPACE2-2 ATU In Node;    !- Outlet 2 Node Name",

        "  AirLoopHVAC:ReturnPath,",
        "    ReturnAirPath1,          !- Name",
        "    Demand Out Node,         !- Return Air Path Outlet Node Name",
        "    AirLoopHVAC:ZoneMixer,   !- Component 1 Object Type",
        "    Zone Return Air Mixer;   !- Component 1 Name",

        "  AirLoopHVAC:ZoneMixer,",
        "    Zone Return Air Mixer,   !- Name",
        "    Demand Out Node,         !- Outlet Node Name",
        "    Space1 Ret Node,         !- Inlet 1 Node Name",
        "    Space2-2 Ret Node;       !- Inlet 2 Node Name",

        "  Fan:VariableVolume,",
        "    Supply Fan 1,            !- Name",
        "    ,                        !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    600.0,                   !- Pressure Rise {Pa}",
        "    1.0,                     !- Maximum Flow Rate {m3/s}",
        "    FixedFlowRate,           !- Fan Power Minimum Flow Rate Input Method",
        "    ,                        !- Fan Power Minimum Flow Fraction",
        "    0.35326,                 !- Fan Power Minimum Air Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1.0,                     !- Motor In Airstream Fraction",
        "    0.0015302446,            !- Fan Power Coefficient 1",
        "    0.0052080574,            !- Fan Power Coefficient 2",
        "    1.1086242,               !- Fan Power Coefficient 3",
        "    -0.11635563,             !- Fan Power Coefficient 4",
        "    0.000,                   !- Fan Power Coefficient 5",
        "    VAV Sys 1 Inlet Node,    !- Air Inlet Node Name",
        "    VAV Sys 1 Outlet Node;   !- Air Outlet Node Name",

        "  BranchList,",
        "    VAV Sys 2 Branches,      !- Name",
        "    VAV Sys 2 Main Branch;   !- Branch 1 Name",

        "  Branch,",
        "    VAV Sys 2 Main Branch,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Fan:VariableVolume,      !- Component 4 Object Type",
        "    Supply Fan 2,            !- Component 4 Name",
        "    VAV Sys 2 Inlet Node,    !- Component 4 Inlet Node Name",
        "    VAV Sys 2 Outlet Node;   !- Component 4 Outlet Node Name",

        "  AirLoopHVAC,",
        "    VAV Sys 2,               !- Name",
        "    ,                        !- Controller List Name",
        "    VAV Sys 2 Avail List,    !- Availability Manager List Name",
        "    1.0,                     !- Design Supply Air Flow Rate {m3/s}",
        "    VAV Sys 2 Branches,      !- Branch List Name",
        "    ,                        !- Connector List Name",
        "    VAV Sys 2 Inlet Node,    !- Supply Side Inlet Node Name",
        "    Demand 2 Out Node,         !- Demand Side Outlet Node Name",
        "    Zone Eq 2 In Node,         !- Demand Side Inlet Node Names",
        "    VAV Sys 2 Outlet Node;   !- Supply Side Outlet Node Names",

        "  AirLoopHVAC:SupplyPath,",
        "    Zone Supply Air Path 2,  !- Name",
        "    Zone Eq 2 In Node,         !- Supply Air Path Inlet Node Name",
        "    AirLoopHVAC:ZoneSplitter,!- Component 1 Object Type",
        "    Zone Supply Air Splitter 2;  !- Component 1 Name",

        "  AirLoopHVAC:ZoneSplitter,",
        "    Zone Supply Air Splitter 2,  !- Name",
        "    Zone Eq 2 In Node,         !- Inlet Node Name",
        "    SPACE2-1 ATU In Node;    !- Outlet 2 Node Name",

        "  AirLoopHVAC:ReturnPath,",
        "    ReturnAirPath2,          !- Name",
        "    Demand 2 Out Node,         !- Return Air Path Outlet Node Name",
        "    AirLoopHVAC:ZoneMixer,   !- Component 1 Object Type",
        "    Zone Return Air Mixer 2;   !- Component 1 Name",

        "  AirLoopHVAC:ZoneMixer,",
        "    Zone Return Air Mixer 2,   !- Name",
        "    Demand 2 Out Node,         !- Outlet Node Name",
        "    Space2-1 Ret Node;         !- Inlet 2 Node Name",

        "  Fan:VariableVolume,",
        "    Supply Fan 2,            !- Name",
        "    ,                        !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    600.0,                   !- Pressure Rise {Pa}",
        "    1.0,                     !- Maximum Flow Rate {m3/s}",
        "    FixedFlowRate,           !- Fan Power Minimum Flow Rate Input Method",
        "    ,                        !- Fan Power Minimum Flow Fraction",
        "    0.35326,                 !- Fan Power Minimum Air Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1.0,                     !- Motor In Airstream Fraction",
        "    0.0015302446,            !- Fan Power Coefficient 1",
        "    0.0052080574,            !- Fan Power Coefficient 2",
        "    1.1086242,               !- Fan Power Coefficient 3",
        "    -0.11635563,             !- Fan Power Coefficient 4",
        "    0.000,                   !- Fan Power Coefficient 5",
        "    VAV Sys 2 Inlet Node,    !- Air Inlet Node Name",
        "    VAV Sys 2 Outlet Node;   !- Air Outlet Node Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound = false;
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    EXPECT_TRUE(compare_err_stream(""));
    DataZoneEquipment::GetZoneEquipmentData(*state);
    EXPECT_TRUE(compare_err_stream(""));
    ASSERT_FALSE(ErrorsFound);
    ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment(*state);
    EXPECT_TRUE(compare_err_stream(""));
    ASSERT_FALSE(ErrorsFound);
    SingleDuct::GetSysInput(*state);
    EXPECT_TRUE(compare_err_stream(""));
    ASSERT_FALSE(ErrorsFound);
    SplitterComponent::GetSplitterInput(*state);
    EXPECT_TRUE(compare_err_stream(""));
    SimAirServingZones::GetAirPathData(*state);
    // Expect warnings about no controllers, clear err_stream
    EXPECT_TRUE(has_err_output(true));
    SimAirServingZones::InitAirLoops(*state, true);
    EXPECT_TRUE(compare_err_stream(""));
    ASSERT_FALSE(ErrorsFound);
    // And finally, all of this gymnastics just to check if the airloopnums get set correctly
    // For this test, each ADU 1-1 and 2-2 should be connected to airloop 1, and ADU 2-1 to airloop 2
    EXPECT_EQ(state->dataZoneEquip->ZoneEquipConfig(1).InletNodeAirLoopNum(1), 1);
    EXPECT_EQ(state->dataZoneEquip->ZoneEquipConfig(2).InletNodeAirLoopNum(1), 2);
    EXPECT_EQ(state->dataZoneEquip->ZoneEquipConfig(2).InletNodeAirLoopNum(2), 1);
}

TEST_F(EnergyPlusFixture, InitAirLoops_2AirLoop3ADUb)
{
    std::string const idf_objects = delimited_string({
        "Zone,",
        "  Space1;                  !- Name",

        "Zone,",
        "  Space2;                !- Name",

        "  ZoneHVAC:EquipmentConnections,",
        "    Space1,                   !- Zone Name",
        "    Space1 Eq,                !- Zone Conditioning Equipment List Name",
        "    Space1-1 In Node,         !- Zone Air Inlet Node or NodeList Name",
        "    Space1 Out Node,          !- Zone Air Exhaust Node or NodeList Name",
        "    Space1 Node,              !- Zone Air Node Name",
        "    Space1 Ret Node;          !- Zone Return Air Node Name",

        "  ZoneHVAC:EquipmentConnections,",
        "    Space2,                   !- Zone Name",
        "    Space2 Eq,                !- Zone Conditioning Equipment List Name",
        "    Space2 Inlet Nodes,       !- Zone Air Inlet Node or NodeList Name",
        "    Space2 Out Node,          !- Zone Air Exhaust Node or NodeList Name",
        "    Space2 Node,              !- Zone Air Node Name",
        "    Space2 Return Nodes;      !- Zone Return Air Node Name",

        "  NodeList,",
        "    Space2 Inlet Nodes,       !- Name",
        "    Space2-1 In Node,         !- Node 1 Name",
        "    Space2-2 In Node;         !- Node 2 Name",

        "  NodeList,",
        "    Space2 Return Nodes,       !- Name",
        "    Space2-1 Ret Node,         !- Node 1 Name",
        "    Space2-2 Ret Node;         !- Node 2 Name",

        "  ZoneHVAC:EquipmentList,",
        "    Space1 Eq,               !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE1-1 ATU,            !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No - Load Sequence",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SPACE1-1 ATU,            !- Name",
        "    Space1-1 In Node,        !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "    SPACE1-1 VAV Reheat;     !- Air Terminal Name",

        "  ZoneHVAC:EquipmentList,",
        "    Space2 Eq,               !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE2-1 ATU,            !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No - Load Sequence",
        "    ,                        !- Zone cooling fraction thingy",
        "    ,                        !- Zone heating fraction thingy",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 2 Object Type",
        "    SPACE2-2 ATU,            !- Zone Equipment 2 Name",
        "    2,                       !- Zone Equipment 2 Cooling Sequence",
        "    2;                       !- Zone Equipment 2 Heating or No - Load Sequence",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SPACE2-1 ATU,            !- Name",
        "    Space2-1 In Node,        !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "    SPACE2-1 VAV Reheat;     !- Air Terminal Name",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SPACE2-2 ATU,            !- Name",
        "    Space2-2 In Node,        !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "    SPACE2-2 VAV Reheat;     !- Air Terminal Name",

        "  AirTerminal:SingleDuct:VAV:NoReheat,",
        "    SPACE1-1 VAV Reheat,     !- Name",
        "    ,                        !- Availability Schedule Name",
        "    Space1-1 In Node,        !- Air Outlet Node Name",
        "    SPACE1-1 ATU In Node,    !- Air Inlet Node Name",
        "    1.0,                     !- Maximum Air Flow Rate {m3/s}",
        "    Constant,                !- Zone Minimum Air Flow Input Method",
        "    0.3;                     !- Constant Minimum Air Flow Fraction",

        "  AirTerminal:SingleDuct:VAV:NoReheat,",
        "    SPACE2-1 VAV Reheat,     !- Name",
        "    ,                        !- Availability Schedule Name",
        "    Space2-1 In Node,        !- Air Outlet Node Name",
        "    SPACE2-1 ATU In Node,    !- Air Inlet Node Name",
        "    1.0,                     !- Maximum Air Flow Rate {m3/s}",
        "    Constant,                !- Zone Minimum Air Flow Input Method",
        "    0.3;                     !- Constant Minimum Air Flow Fraction",

        "  AirTerminal:SingleDuct:VAV:NoReheat,",
        "    SPACE2-2 VAV Reheat,     !- Name",
        "    ,                        !- Availability Schedule Name",
        "    Space2-2 In Node,        !- Air Outlet Node Name",
        "    SPACE2-2 ATU In Node,    !- Air Inlet Node Name",
        "    1.0,                     !- Maximum Air Flow Rate {m3/s}",
        "    Constant,                !- Zone Minimum Air Flow Input Method",
        "    0.3;                     !- Constant Minimum Air Flow Fraction",

        "  BranchList,",
        "    VAV Sys 1 Branches,      !- Name",
        "    VAV Sys 1 Main Branch;   !- Branch 1 Name",

        "  Branch,",
        "    VAV Sys 1 Main Branch,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Fan:VariableVolume,      !- Component 4 Object Type",
        "    Supply Fan 1,            !- Component 4 Name",
        "    VAV Sys 1 Inlet Node,    !- Component 4 Inlet Node Name",
        "    VAV Sys 1 Outlet Node;   !- Component 4 Outlet Node Name",

        "  AirLoopHVAC,",
        "    VAV Sys 1,               !- Name",
        "    ,                        !- Controller List Name",
        "    VAV Sys 1 Avail List,    !- Availability Manager List Name",
        "    1.0,                     !- Design Supply Air Flow Rate {m3/s}",
        "    VAV Sys 1 Branches,      !- Branch List Name",
        "    ,                        !- Connector List Name",
        "    VAV Sys 1 Inlet Node,    !- Supply Side Inlet Node Name",
        "    Demand Out Node,         !- Demand Side Outlet Node Name",
        "    Zone Eq In Node,         !- Demand Side Inlet Node Names",
        "    VAV Sys 1 Outlet Node;   !- Supply Side Outlet Node Names",

        "  AirLoopHVAC:SupplyPath,",
        "    Zone Supply Air Path 1,  !- Name",
        "    Zone Eq In Node,         !- Supply Air Path Inlet Node Name",
        "    AirLoopHVAC:ZoneSplitter,!- Component 1 Object Type",
        "    Zone Supply Air Splitter;  !- Component 1 Name",

        "  AirLoopHVAC:ZoneSplitter,",
        "    Zone Supply Air Splitter,  !- Name",
        "    Zone Eq In Node,         !- Inlet Node Name",
        "    SPACE1-1 ATU In Node;    !- Outlet 1 Node Name",

        "  AirLoopHVAC:ReturnPath,",
        "    ReturnAirPath1,          !- Name",
        "    Demand Out Node,         !- Return Air Path Outlet Node Name",
        "    AirLoopHVAC:ZoneMixer,   !- Component 1 Object Type",
        "    Zone Return Air Mixer;   !- Component 1 Name",

        "  AirLoopHVAC:ZoneMixer,",
        "    Zone Return Air Mixer,   !- Name",
        "    Demand Out Node,         !- Outlet Node Name",
        "    Space1 Ret Node;         !- Inlet 1 Node Name",

        "  Fan:VariableVolume,",
        "    Supply Fan 1,            !- Name",
        "    ,                        !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    600.0,                   !- Pressure Rise {Pa}",
        "    1.0,                     !- Maximum Flow Rate {m3/s}",
        "    FixedFlowRate,           !- Fan Power Minimum Flow Rate Input Method",
        "    ,                        !- Fan Power Minimum Flow Fraction",
        "    0.35326,                 !- Fan Power Minimum Air Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1.0,                     !- Motor In Airstream Fraction",
        "    0.0015302446,            !- Fan Power Coefficient 1",
        "    0.0052080574,            !- Fan Power Coefficient 2",
        "    1.1086242,               !- Fan Power Coefficient 3",
        "    -0.11635563,             !- Fan Power Coefficient 4",
        "    0.000,                   !- Fan Power Coefficient 5",
        "    VAV Sys 1 Inlet Node,    !- Air Inlet Node Name",
        "    VAV Sys 1 Outlet Node;   !- Air Outlet Node Name",

        "  BranchList,",
        "    VAV Sys 2 Branches,      !- Name",
        "    VAV Sys 2 Main Branch;   !- Branch 1 Name",

        "  Branch,",
        "    VAV Sys 2 Main Branch,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Fan:VariableVolume,      !- Component 4 Object Type",
        "    Supply Fan 2,            !- Component 4 Name",
        "    VAV Sys 2 Inlet Node,    !- Component 4 Inlet Node Name",
        "    VAV Sys 2 Outlet Node;   !- Component 4 Outlet Node Name",

        "  AirLoopHVAC,",
        "    VAV Sys 2,               !- Name",
        "    ,                        !- Controller List Name",
        "    VAV Sys 2 Avail List,    !- Availability Manager List Name",
        "    1.0,                     !- Design Supply Air Flow Rate {m3/s}",
        "    VAV Sys 2 Branches,      !- Branch List Name",
        "    ,                        !- Connector List Name",
        "    VAV Sys 2 Inlet Node,    !- Supply Side Inlet Node Name",
        "    Demand 2 Out Node,         !- Demand Side Outlet Node Name",
        "    Zone Eq 2 In Node,         !- Demand Side Inlet Node Names",
        "    VAV Sys 2 Outlet Node;   !- Supply Side Outlet Node Names",

        "  AirLoopHVAC:SupplyPath,",
        "    Zone Supply Air Path 2,  !- Name",
        "    Zone Eq 2 In Node,         !- Supply Air Path Inlet Node Name",
        "    AirLoopHVAC:ZoneSplitter,!- Component 1 Object Type",
        "    Zone Supply Air Splitter 2;  !- Component 1 Name",

        "  AirLoopHVAC:ZoneSplitter,",
        "    Zone Supply Air Splitter 2,  !- Name",
        "    Zone Eq 2 In Node,         !- Inlet Node Name",
        "    SPACE2-2 ATU In Node,    !- Outlet 1 Node Name",
        "    SPACE2-1 ATU In Node;    !- Outlet 2 Node Name",

        "  AirLoopHVAC:ReturnPath,",
        "    ReturnAirPath2,          !- Name",
        "    Demand 2 Out Node,         !- Return Air Path Outlet Node Name",
        "    AirLoopHVAC:ZoneMixer,   !- Component 1 Object Type",
        "    Zone Return Air Mixer 2;   !- Component 1 Name",

        "  AirLoopHVAC:ZoneMixer,",
        "    Zone Return Air Mixer 2,   !- Name",
        "    Demand 2 Out Node,         !- Outlet Node Name",
        "    Space2-1 Ret Node,         !- Inlet 1 Node Name",
        "    Space2-2 Ret Node;         !- Inlet 2 Node Name",

        "  Fan:VariableVolume,",
        "    Supply Fan 2,            !- Name",
        "    ,                        !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    600.0,                   !- Pressure Rise {Pa}",
        "    1.0,                     !- Maximum Flow Rate {m3/s}",
        "    FixedFlowRate,           !- Fan Power Minimum Flow Rate Input Method",
        "    ,                        !- Fan Power Minimum Flow Fraction",
        "    0.35326,                 !- Fan Power Minimum Air Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1.0,                     !- Motor In Airstream Fraction",
        "    0.0015302446,            !- Fan Power Coefficient 1",
        "    0.0052080574,            !- Fan Power Coefficient 2",
        "    1.1086242,               !- Fan Power Coefficient 3",
        "    -0.11635563,             !- Fan Power Coefficient 4",
        "    0.000,                   !- Fan Power Coefficient 5",
        "    VAV Sys 2 Inlet Node,    !- Air Inlet Node Name",
        "    VAV Sys 2 Outlet Node;   !- Air Outlet Node Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound = false;
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    EXPECT_TRUE(compare_err_stream(""));
    DataZoneEquipment::GetZoneEquipmentData(*state);
    EXPECT_TRUE(compare_err_stream(""));
    ASSERT_FALSE(ErrorsFound);
    ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment(*state);
    EXPECT_TRUE(compare_err_stream(""));
    ASSERT_FALSE(ErrorsFound);
    SingleDuct::GetSysInput(*state);
    EXPECT_TRUE(compare_err_stream(""));
    ASSERT_FALSE(ErrorsFound);
    SplitterComponent::GetSplitterInput(*state);
    EXPECT_TRUE(compare_err_stream(""));
    SimAirServingZones::GetAirPathData(*state);
    // Expect warnings about no controllers, clear err_stream
    EXPECT_TRUE(has_err_output(true));
    SimAirServingZones::InitAirLoops(*state, true);
    EXPECT_TRUE(compare_err_stream(""));
    ASSERT_FALSE(ErrorsFound);
    // And finally, all of this gymnastics just to check if the airloopnums get set correctly
    // For this test, ADU 1-1 should be connected to airloop 1, and ADU 2-1 and ADU 2-2 to airloop 2
    // This test should fail before the fix for 7518 is added
    EXPECT_EQ(state->dataZoneEquip->ZoneEquipConfig(1).InletNodeAirLoopNum(1), 1);
    EXPECT_EQ(state->dataZoneEquip->ZoneEquipConfig(2).InletNodeAirLoopNum(1), 2);
    EXPECT_EQ(state->dataZoneEquip->ZoneEquipConfig(2).InletNodeAirLoopNum(2), 2);
}

TEST_F(EnergyPlusFixture, InitAirLoops_1AirLoop2Zones3ADU)
{
    // Test for issue 8048 An air loop serving more terminal units than the number of zones will exceed array bounds
    std::string const idf_objects = delimited_string({
        "Zone,",
        "  Space1;                  !- Name",

        "Zone,",
        "  Space2;                !- Name",

        "  ZoneHVAC:EquipmentConnections,",
        "    Space1,                   !- Zone Name",
        "    Space1 Eq,                !- Zone Conditioning Equipment List Name",
        "    Space1-1 Inlets,         !- Zone Air Inlet Node or NodeList Name",
        "    Space1 Out Node,          !- Zone Air Exhaust Node or NodeList Name",
        "    Space1 Node,              !- Zone Air Node Name",
        "    Space1 Ret Node;          !- Zone Return Air Node Name",
        "  NodeList,",
        "    Space1-1 Inlets,",
        "    Space1-1 In Node,",
        "    Space1-1b In Node;",

        "  ZoneHVAC:EquipmentConnections,",
        "    Space2,                   !- Zone Name",
        "    Space2 Eq,                !- Zone Conditioning Equipment List Name",
        "    Space2-1 In Node,         !- Zone Air Inlet Node or NodeList Name",
        "    Space2 Out Node,          !- Zone Air Exhaust Node or NodeList Name",
        "    Space2 Node,              !- Zone Air Node Name",
        "    Space2 Ret Node;          !- Zone Return Air Node Name",

        "  ZoneHVAC:EquipmentList,",
        "    Space1 Eq,               !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE1-1 ATU,            !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No - Load Sequence",
        "    ,                        !- Zone cooling fraction thingy",
        "    ,                        !- Zone heating fraction thingy",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE1-1b ATU,            !- Zone Equipment 1 Name",
        "    2,                       !- Zone Equipment 1 Cooling Sequence",
        "    2;                       !- Zone Equipment 1 Heating or No - Load Sequence",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SPACE1-1 ATU,            !- Name",
        "    Space1-1 In Node,        !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "    SPACE1-1 VAV Reheat;     !- Air Terminal Name",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SPACE1-1b ATU,            !- Name",
        "    Space1-1b In Node,        !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "    SPACE1-1b VAV Reheat;     !- Air Terminal Name",

        "  ZoneHVAC:EquipmentList,",
        "    Space2 Eq,               !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    SPACE2-1 ATU,            !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No - Load Sequence",

        "  ZoneHVAC:AirDistributionUnit,",
        "    SPACE2-1 ATU,            !- Name",
        "    Space2-1 In Node,        !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:VAV:NoReheat,  !- Air Terminal Object Type",
        "    SPACE2-1 VAV Reheat;     !- Air Terminal Name",

        "  AirTerminal:SingleDuct:VAV:NoReheat,",
        "    SPACE1-1 VAV Reheat,     !- Name",
        "    ,                        !- Availability Schedule Name",
        "    Space1-1 In Node,        !- Air Outlet Node Name",
        "    SPACE1-1 ATU In Node,    !- Air Inlet Node Name",
        "    1.0,                     !- Maximum Air Flow Rate {m3/s}",
        "    Constant,                !- Zone Minimum Air Flow Input Method",
        "    0.3;                     !- Constant Minimum Air Flow Fraction",

        "  AirTerminal:SingleDuct:VAV:NoReheat,",
        "    SPACE1-1b VAV Reheat,     !- Name",
        "    ,                        !- Availability Schedule Name",
        "    Space1-1b In Node,        !- Air Outlet Node Name",
        "    SPACE1-1b ATU In Node,    !- Air Inlet Node Name",
        "    1.0,                     !- Maximum Air Flow Rate {m3/s}",
        "    Constant,                !- Zone Minimum Air Flow Input Method",
        "    0.3;                     !- Constant Minimum Air Flow Fraction",

        "  AirTerminal:SingleDuct:VAV:NoReheat,",
        "    SPACE2-1 VAV Reheat,     !- Name",
        "    ,                        !- Availability Schedule Name",
        "    Space2-1 In Node,        !- Air Outlet Node Name",
        "    SPACE2-1 ATU In Node,    !- Air Inlet Node Name",
        "    1.0,                     !- Maximum Air Flow Rate {m3/s}",
        "    Constant,                !- Zone Minimum Air Flow Input Method",
        "    0.3;                     !- Constant Minimum Air Flow Fraction",

        "  BranchList,",
        "    VAV Sys 1 Branches,      !- Name",
        "    VAV Sys 1 Main Branch;   !- Branch 1 Name",

        "  Branch,",
        "    VAV Sys 1 Main Branch,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Fan:VariableVolume,      !- Component 4 Object Type",
        "    Supply Fan 1,            !- Component 4 Name",
        "    VAV Sys 1 Inlet Node,    !- Component 4 Inlet Node Name",
        "    VAV Sys 1 Outlet Node;   !- Component 4 Outlet Node Name",

        "  AirLoopHVAC,",
        "    VAV Sys 1,               !- Name",
        "    ,                        !- Controller List Name",
        "    VAV Sys 1 Avail List,    !- Availability Manager List Name",
        "    1.0,                     !- Design Supply Air Flow Rate {m3/s}",
        "    VAV Sys 1 Branches,      !- Branch List Name",
        "    ,                        !- Connector List Name",
        "    VAV Sys 1 Inlet Node,    !- Supply Side Inlet Node Name",
        "    Demand Out Node,         !- Demand Side Outlet Node Name",
        "    Zone Eq In Node,         !- Demand Side Inlet Node Names",
        "    VAV Sys 1 Outlet Node;   !- Supply Side Outlet Node Names",

        "  AirLoopHVAC:SupplyPath,",
        "    Zone Supply Air Path 1,  !- Name",
        "    Zone Eq In Node,         !- Supply Air Path Inlet Node Name",
        "    AirLoopHVAC:ZoneSplitter,!- Component 1 Object Type",
        "    Zone Supply Air Splitter;  !- Component 1 Name",

        "  AirLoopHVAC:ZoneSplitter,",
        "    Zone Supply Air Splitter,  !- Name",
        "    Zone Eq In Node,         !- Inlet Node Name",
        "    SPACE1-1 ATU In Node,    !- Outlet 1 Node Name",
        "    SPACE1-1b ATU In Node,    !- Outlet 1 Node Name",
        "    SPACE2-1 ATU In Node;    !- Outlet 2 Node Name",

        "  AirLoopHVAC:ReturnPath,",
        "    ReturnAirPath1,          !- Name",
        "    Demand Out Node,         !- Return Air Path Outlet Node Name",
        "    AirLoopHVAC:ZoneMixer,   !- Component 1 Object Type",
        "    Zone Return Air Mixer;   !- Component 1 Name",

        "  AirLoopHVAC:ZoneMixer,",
        "    Zone Return Air Mixer,   !- Name",
        "    Demand Out Node,         !- Outlet Node Name",
        "    Space1 Ret Node,         !- Inlet 1 Node Name",
        "    Space2 Ret Node;         !- Inlet 2 Node Name",

        "  Fan:VariableVolume,",
        "    Supply Fan 1,            !- Name",
        "    ,                        !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    600.0,                   !- Pressure Rise {Pa}",
        "    1.0,                     !- Maximum Flow Rate {m3/s}",
        "    FixedFlowRate,           !- Fan Power Minimum Flow Rate Input Method",
        "    ,                        !- Fan Power Minimum Flow Fraction",
        "    0.35326,                 !- Fan Power Minimum Air Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1.0,                     !- Motor In Airstream Fraction",
        "    0.0015302446,            !- Fan Power Coefficient 1",
        "    0.0052080574,            !- Fan Power Coefficient 2",
        "    1.1086242,               !- Fan Power Coefficient 3",
        "    -0.11635563,             !- Fan Power Coefficient 4",
        "    0.000,                   !- Fan Power Coefficient 5",
        "    VAV Sys 1 Inlet Node,    !- Air Inlet Node Name",
        "    VAV Sys 1 Outlet Node;   !- Air Outlet Node Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound = false;
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    ASSERT_FALSE(ErrorsFound);
    EXPECT_TRUE(compare_err_stream(""));
    DataZoneEquipment::GetZoneEquipmentData(*state);
    EXPECT_TRUE(compare_err_stream(""));
    ASSERT_FALSE(ErrorsFound);
    ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment(*state);
    EXPECT_TRUE(compare_err_stream(""));
    ASSERT_FALSE(ErrorsFound);
    SingleDuct::GetSysInput(*state);
    EXPECT_TRUE(compare_err_stream(""));
    ASSERT_FALSE(ErrorsFound);
    SplitterComponent::GetSplitterInput(*state);
    EXPECT_TRUE(compare_err_stream(""));
    SimAirServingZones::GetAirPathData(*state);
    // Expect warnings about no controllers, clear err_stream
    EXPECT_TRUE(has_err_output(true));
    SimAirServingZones::InitAirLoops(*state, true);
    EXPECT_TRUE(compare_err_stream(""));
    ASSERT_FALSE(ErrorsFound);
    // And finally, all of this gymnastics just to check if the airloopnums get set correctly
    // For this test, both ADUs should be connected airloop 1 which is the only one here
    EXPECT_EQ(state->dataZoneEquip->ZoneEquipConfig(1).InletNodeAirLoopNum(1), 1);
    EXPECT_EQ(state->dataZoneEquip->ZoneEquipConfig(2).InletNodeAirLoopNum(1), 1);
}

TEST_F(EnergyPlusFixture, AirLoop_ReturnFan_MinFlow)
{
    // This test was disabled primarily because it's an incredibly heavy boot stomping on ManageSimulation just
    // to verify that flow is set to zero.  As of the time of disabling, it often fails on my machine with a
    // segmentation fault, so something isn't being set up properly.  It would be great to strip this down to a
    // much more localized function call and carefully test a few things.

    // Test for #6050 - Return Fan 'Fan Power Minimum Flow Fraction' enforces loop flow, while it should not
    // I am simulating a very simple shoebox building with 1 zone.
    // There is an airLoopHVAC with a Return Fan:VariableVolume, with a 'Fan Power Minimum Flow Fraction' = 0.3
    // The Supply fan has a fraction of zero
    //
    // The supply side of the loop is has follows: only has a Return Fan, a Mixing box, and a SupplyFan
    //                        o   o
    //                        |   |
    //  --- (Return Fan) ---- o-/-o ---- (SupplyFan) ----
    //  |                                               |
    //  o                                               o
    //
    //  The Demand side has only one ATU:VAV:NoReheat with a Minimum flow Fraction of 0 so that it doesn't drive the airflow either

    std::string const idf_objects = delimited_string({

        "Timestep,",
        "  4;                                      !- Number of Timesteps per Hour",

        "SimulationControl,",
        "  No,                                     !- Do Zone Sizing Calculation",
        "  No,                                     !- Do System Sizing Calculation",
        "  No,                                     !- Do Plant Sizing Calculation",
        "  Yes,                                    !- Run Simulation for Sizing Periods",
        "  No,                                    !- Run Simulation for Weather File Run Periods",
        "  No,                                     !- Do HVAC Sizing Simulation for Sizing Periods",
        "  ;                                       !- Maximum Number of HVAC Sizing Simulation Passes",

        "Building,",
        "  Building 1,                             !- Name",
        "  ,                                       !- North Axis {deg}",
        "  ,                                       !- Terrain",
        "  ,                                       !- Loads Convergence Tolerance Value {W}",
        "  ,                                       !- Temperature Convergence Tolerance Value {deltaC}",
        "  ,                                       !- Solar Distribution",
        "  ,                                       !- Maximum Number of Warmup Days",
        "  ;                                       !- Minimum Number of Warmup Days",

        "GlobalGeometryRules,",
        "  UpperLeftCorner,                        !- Starting Vertex Position",
        "  Counterclockwise,                       !- Vertex Entry Direction",
        "  Relative,                               !- Coordinate System",
        "  Relative,                               !- Daylighting Reference Point Coordinate System",
        "  Relative;                               !- Rectangular Surface Coordinate System",

        "Site:Location,",
        "  Chicago Ohare Intl Ap,                  !- Name",
        "  41.98,                                  !- Latitude {deg}",
        "  -87.92,                                 !- Longitude {deg}",
        "  -6,                                     !- Time Zone {hr}",
        "  201;                                    !- Elevation {m}",

        "SizingPeriod:DesignDay,",
        "  Chicago Ohare Intl Ap Ann Clg .4% Condns DB=>MWB, !- Name",
        "  7,                                      !- Month",
        "  21,                                     !- Day of Month",
        "  SummerDesignDay,                        !- Day Type",
        "  33.3,                                   !- Maximum Dry-Bulb Temperature {C}",
        "  10.5,                                   !- Daily Dry-Bulb Temperature Range {deltaC}",
        "  DefaultMultipliers,                     !- Dry-Bulb Temperature Range Modifier Type",
        "  ,                                       !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "  Wetbulb,                                !- Humidity Condition Type",
        "  23.7,                                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "  ,                                       !- Humidity Condition Day Schedule Name",
        "  ,                                       !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "  ,                                       !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "  ,                                       !- Daily Wet-Bulb Temperature Range {deltaC}",
        "  98934,                                  !- Barometric Pressure {Pa}",
        "  5.2,                                    !- Wind Speed {m/s}",
        "  230,                                    !- Wind Direction {deg}",
        "  No,                                     !- Rain Indicator",
        "  No,                                     !- Snow Indicator",
        "  No,                                     !- Daylight Saving Time Indicator",
        "  ASHRAETau,                              !- Solar Model Indicator",
        "  ,                                       !- Beam Solar Day Schedule Name",
        "  ,                                       !- Diffuse Solar Day Schedule Name",
        "  0.455,                                  !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "  2.05;                                   !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",

        "ScheduleTypeLimits,",
        "  Any Number;                             !- Name",

        "Schedule:Constant,",
        "  Always On Discrete,                     !- Name",
        "  Any Number,                             !- Schedule Type Limits Name",
        "  1;                                      !- Hourly Value",

        "Zone,",
        "  Zone1;                                  !- Name",

        "ZoneControl:Thermostat,",
        "  Zone1 Thermostat,                       !- Name",
        "  Zone1,                                  !- Zone or ZoneList Name",
        "  Zone1 Thermostat Schedule,              !- Control Type Schedule Name",
        "  ThermostatSetpoint:DualSetpoint,        !- Control 1 Object Type",
        "  Thermostat Setpoint Dual Setpoint 1;    !- Control 1 Name",

        "Schedule:Compact,",
        "  Zone1 Thermostat Schedule,              !- Name",
        "  Zone1 Thermostat Schedule Type Limits,  !- Schedule Type Limits Name",
        "  Through: 12/31,                         !- Field 1",
        "  For: AllDays,                           !- Field 2",
        "  Until: 24:00,                           !- Field 3",
        "  4;                                      !- Field 4",

        "ScheduleTypeLimits,",
        "  Zone1 Thermostat Schedule Type Limits,  !- Name",
        "  0,                                      !- Lower Limit Value {BasedOnField A3}",
        "  4,                                      !- Upper Limit Value {BasedOnField A3}",
        "  DISCRETE;                               !- Numeric Type",

        "ThermostatSetpoint:DualSetpoint,",
        "  Thermostat Setpoint Dual Setpoint 1,    !- Name",
        "  Schedule 19C,                           !- Heating Setpoint Temperature Schedule Name",
        "  Schedule 26C;                           !- Cooling Setpoint Temperature Schedule Name",

        "Schedule:Constant,",
        "  Schedule 19C,                           !- Name",
        "  Any Number,                             !- Schedule Type Limits Name",
        "  19;                                     !- Hourly Value",

        "Schedule:Constant,",
        "  Schedule 26C,                           !- Name",
        "  Any Number,                             !- Schedule Type Limits Name",
        "  26;                                     !- Hourly Value",

        "Material,",
        "  MAT-CC05 8 HW CONCRETE,                 !- Name",
        "  Rough,                                  !- Roughness",
        "  0.2032,                                 !- Thickness {m}",
        "  1.311,                                  !- Conductivity {W/m-K}",
        "  2240,                                   !- Density {kg/m3}",
        "  836.8,                                  !- Specific Heat {J/kg-K}",
        "  0.9,                                    !- Thermal Absorptance",
        "  0.7,                                    !- Solar Absorptance",
        "  0.7;                                    !- Visible Absorptance",

        "Material:NoMass,",
        "  CP02 CARPET PAD,                        !- Name",
        "  VeryRough,                              !- Roughness",
        "  0.2165,                                 !- Thermal Resistance {m2-K/W}",
        "  0.9,                                    !- Thermal Absorptance",
        "  0.7,                                    !- Solar Absorptance",
        "  0.8;                                    !- Visible Absorptance",

        "Construction,",
        "  Concrete_and_carpet_Construction,                                   !- Name",
        "  MAT-CC05 8 HW CONCRETE,                 !- Layer 1",
        "  CP02 CARPET PAD;                        !- Layer 2",

        "BuildingSurface:Detailed,",
        "  Surface 1,                              !- Name",
        "  Floor,                                  !- Surface Type",
        "  Concrete_and_carpet_Construction,                                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "    ,                        !- Space Name",
        "  Ground,                                 !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 0, 0,                                !- X,Y,Z Vertex 1 {m}",
        "  0, 10, 0,                               !- X,Y,Z Vertex 2 {m}",
        "  10, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  10, 0, 0;                               !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Surface 2,                              !- Name",
        "  Wall,                                   !- Surface Type",
        "  Concrete_and_carpet_Construction,                                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "    ,                        !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 10, 2.5,                             !- X,Y,Z Vertex 1 {m}",
        "  0, 10, 0,                               !- X,Y,Z Vertex 2 {m}",
        "  0, 0, 0,                                !- X,Y,Z Vertex 3 {m}",
        "  0, 0, 2.5;                              !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Surface 3,                              !- Name",
        "  Wall,                                   !- Surface Type",
        "  Concrete_and_carpet_Construction,                                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "    ,                        !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 10, 2.5,                            !- X,Y,Z Vertex 1 {m}",
        "  10, 10, 0,                              !- X,Y,Z Vertex 2 {m}",
        "  0, 10, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  0, 10, 2.5;                             !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Surface 4,                              !- Name",
        "  Wall,                                   !- Surface Type",
        "  Concrete_and_carpet_Construction,                                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "    ,                        !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 0, 2.5,                             !- X,Y,Z Vertex 1 {m}",
        "  10, 0, 0,                               !- X,Y,Z Vertex 2 {m}",
        "  10, 10, 0,                              !- X,Y,Z Vertex 3 {m}",
        "  10, 10, 2.5;                            !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Surface 5,                              !- Name",
        "  Wall,                                   !- Surface Type",
        "  Concrete_and_carpet_Construction,                                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "    ,                        !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  0, 0, 2.5,                              !- X,Y,Z Vertex 1 {m}",
        "  0, 0, 0,                                !- X,Y,Z Vertex 2 {m}",
        "  10, 0, 0,                               !- X,Y,Z Vertex 3 {m}",
        "  10, 0, 2.5;                             !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  Surface 6,                              !- Name",
        "  Roof,                                   !- Surface Type",
        "  Concrete_and_carpet_Construction,                                   !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "    ,                        !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  10, 0, 2.5,                             !- X,Y,Z Vertex 1 {m}",
        "  10, 10, 2.5,                            !- X,Y,Z Vertex 2 {m}",
        "  0, 10, 2.5,                             !- X,Y,Z Vertex 3 {m}",
        "  0, 0, 2.5;                              !- X,Y,Z Vertex 4 {m}",

        "ZoneHVAC:EquipmentConnections,",
        "  Zone1,                                  !- Zone Name",
        "  Zone1 Equipment List,                   !- Zone Conditioning Equipment List Name",
        "  Zone1 Inlet Node List,                  !- Zone Air Inlet Node or NodeList Name",
        "  ,                                       !- Zone Air Exhaust Node or NodeList Name",
        "  Zone1 Air Node,                         !- Zone Air Node Name",
        "  Zone1 Return Node List;                 !- Zone Return Air Node or NodeList Name",

        "NodeList,",
        "  Zone1 Inlet Node List,                  !- Name",
        "  Zone1 ATU Inlet Node;                   !- Node Name 1",

        "NodeList,",
        "  Zone1 Return Node List,                 !- Name",
        "  Zone1 Return Air Node;                  !- Node Name 1",

        "ZoneHVAC:EquipmentList,",
        "  Zone1 Equipment List,                   !- Name",
        "  SequentialLoad,                         !- Load Distribution Scheme",
        "  ZoneHVAC:AirDistributionUnit,           !- Zone Equipment Object Type 1",
        "  ADU VAV No Rht,                         !- Zone Equipment Name 1",
        "  1,                                      !- Zone Equipment Cooling Sequence 1",
        "  1,                                      !- Zone Equipment Heating or No-Load Sequence 1",
        "  ,                                       !- Zone Equipment Sequential Cooling Fraction Schedule Name 1",
        "  ;                                       !- Zone Equipment Sequential Heating Fraction Schedule Name 1",

        "ZoneHVAC:AirDistributionUnit,",
        "  ADU VAV No Rht,                         !- Name",
        "  Zone1 ATU Inlet Node,                   !- Air Distribution Unit Outlet Node Name",
        "  AirTerminal:SingleDuct:VAV:NoReheat,    !- Air Terminal Object Type",
        "  VAV No Rht;                             !- Air Terminal Name",

        // NOTE: Terminal has a min flow of 0
        "AirTerminal:SingleDuct:VAV:NoReheat,",
        "  VAV No Rht,                             !- Name",
        "  Always On Discrete,                     !- Availability Schedule Name",
        "  Zone1 ATU Inlet Node,                   !- Air Outlet Node Name",
        "  ATU VAV No Reheat Inlet Node,           !- Air Inlet Node Name",
        "  1.0,                                    !- Maximum Air Flow Rate {m3/s}",
        "  Constant,                               !- Zone Minimum Air Flow Input Method",
        "  0;                                      !- Constant Minimum Air Flow Fraction", // IMPORTANT

        "OutdoorAir:Node,",
        "  Model Outdoor Air Node;                 !- Name",

        "AirLoopHVAC,",
        "  Air Loop HVAC,                          !- Name",
        "  ,                                       !- Controller List Name",
        "  Air Loop HVACAvailability Manager List, !- Availability Manager List Name",
        "  1.0,                                    !- Design Supply Air Flow Rate {m3/s}",
        "  Air Loop HVAC Supply Branches,          !- Branch List Name",
        "  ,                                       !- Connector List Name",
        "  Supply Side Inlet Node,                 !- Supply Side Inlet Node Name",
        "  Demand Outlet Node,                     !- Demand Side Outlet Node Name",
        "  Air Loop HVAC Demand Inlet Nodes,       !- Demand Side Inlet Node Names",
        "  Air Loop HVAC Supply Outlet Nodes,      !- Supply Side Outlet Node Names",
        "  1;                                      !- Design Return Air Flow Fraction of Supply Air Flow",

        "NodeList,",
        "  Air Loop HVAC Supply Outlet Nodes,      !- Name",
        "  Supply Side Outlet Node;                !- Node Name 1",

        "NodeList,",
        "  Air Loop HVAC Demand Inlet Nodes,       !- Name",
        "  Demand Inlet Node;                      !- Node Name 1",

        "AvailabilityManagerAssignmentList,",
        "  Air Loop HVACAvailability Manager List, !- Name",
        "  AvailabilityManager:Scheduled,          !- Availability Manager Object Type 1",
        "  Air Loop HVAC Availability Manager;     !- Availability Manager Name 1",

        "AvailabilityManager:Scheduled,",
        "  Air Loop HVAC Availability Manager,     !- Name",
        "  Always On Discrete;                     !- Schedule Name",

        "BranchList,",
        "  Air Loop HVAC Supply Branches,          !- Name",
        "  Air Loop HVAC Main Branch;              !- Branch Name 1",

        "Branch,",
        "  Air Loop HVAC Main Branch,              !- Name",
        "  ,                                       !- Pressure Drop Curve Name",
        "  Fan:VariableVolume,                     !- Component Object Type 1",
        "  VSD Return Fan,                         !- Component Name 1",
        "  Supply Side Inlet Node,                 !- Component Inlet Node Name 1",
        "  VSD Return Fan Outlet to Mixing Box Node, !- Component Outlet Node Name 1",
        "  AirLoopHVAC:OutdoorAirSystem,           !- Component Object Type 2",
        "  Mixing Box,                             !- Component Name 2",
        "  VSD Return Fan Outlet to Mixing Box Node, !- Component Inlet Node Name 2",
        "  Supply Side Mixed Air Node,             !- Component Outlet Node Name 2",
        "  Fan:VariableVolume,                     !- Component Object Type 3",
        "  VSD Supply Fan,                         !- Component Name 3",
        "  Supply Side Mixed Air Node,             !- Component Inlet Node Name 3",
        "  Supply Side Outlet Node;                !- Component Outlet Node Name 3",

        // NOTE: The supply has as a min flow fraction of of 0.3
        "Fan:VariableVolume,",
        "  VSD Return Fan,                         !- Name",
        "  Always On Discrete,                     !- Availability Schedule Name",
        "  0.6045,                                 !- Fan Total Efficiency",
        "  1017.592,                               !- Pressure Rise {Pa}",
        "  1.0,                                    !- Maximum Flow Rate {m3/s}",
        "  Fraction,                               !- Fan Power Minimum Flow Rate Input Method", // IMPORTANT
        "  0.3,                                    !- Fan Power Minimum Flow Fraction",          // IMPORTANT
        "  0,                                      !- Fan Power Minimum Air Flow Rate {m3/s}",
        "  0.93,                                   !- Motor Efficiency",
        "  1,                                      !- Motor In Airstream Fraction",
        "  0.040759894,                            !- Fan Power Coefficient 1",
        "  0.08804497,                             !- Fan Power Coefficient 2",
        "  -0.07292612,                            !- Fan Power Coefficient 3",
        "  0.943739823,                            !- Fan Power Coefficient 4",
        "  0,                                      !- Fan Power Coefficient 5",
        "  Supply Side Inlet Node,                 !- Air Inlet Node Name",
        "  VSD Return Fan Outlet to Mixing Box Node, !- Air Outlet Node Name",
        "  General;                                !- End-Use Subcategory",

        "SetpointManager:MixedAir,",
        "  VSD Return Fan Outlet to Mixing Box Node OS Default SPM, !- Name",
        "  Temperature,                            !- Control Variable",
        "  Supply Side Outlet Node,                !- Reference Setpoint Node Name",
        "  Supply Side Mixed Air Node,             !- Fan Inlet Node Name",
        "  Supply Side Outlet Node,                !- Fan Outlet Node Name",
        "  VSD Return Fan Outlet to Mixing Box Node; !- Setpoint Node or NodeList Name",

        "AirLoopHVAC:OutdoorAirSystem,",
        "  Mixing Box,                             !- Name",
        "  Mixing Box Controller List,             !- Controller List Name",
        "  Mixing Box Equipment List;              !- Outdoor Air Equipment List Name",

        "AirLoopHVAC:ControllerList,",
        "  Mixing Box Controller List,             !- Name",
        "  Controller:OutdoorAir,                  !- Controller Object Type 1",
        "  Controller Outdoor Air 1;               !- Controller Name 1",

        "Controller:OutdoorAir,",
        "  Controller Outdoor Air 1,               !- Name",
        "  Relief Air Node,                        !- Relief Air Outlet Node Name",
        "  VSD Return Fan Outlet to Mixing Box Node, !- Return Air Node Name",
        "  Supply Side Mixed Air Node,             !- Mixed Air Node Name",
        "  OA Intake Air Node,                     !- Actuator Node Name",
        "  0,                                      !- Minimum Outdoor Air Flow Rate {m3/s}",
        "  0.5,                                    !- Maximum Outdoor Air Flow Rate {m3/s}",
        "  NoEconomizer,                           !- Economizer Control Type",
        "  ModulateFlow,                           !- Economizer Control Action Type",
        "  28,                                     !- Economizer Maximum Limit Dry-Bulb Temperature {C}",
        "  64000,                                  !- Economizer Maximum Limit Enthalpy {J/kg}",
        "  ,                                       !- Economizer Maximum Limit Dewpoint Temperature {C}",
        "  ,                                       !- Electronic Enthalpy Limit Curve Name",
        "  -100,                                   !- Economizer Minimum Limit Dry-Bulb Temperature {C}",
        "  NoLockout,                              !- Lockout Type",
        "  FixedMinimum,                           !- Minimum Limit Type",
        "  ,                                       !- Minimum Outdoor Air Schedule Name",
        "  ,                                       !- Minimum Fraction of Outdoor Air Schedule Name",
        "  ,                                       !- Maximum Fraction of Outdoor Air Schedule Name",
        "  Controller Mechanical Ventilation 1,    !- Mechanical Ventilation Controller Name",
        "  ,                                       !- Time of Day Economizer Control Schedule Name",
        "  No,                                     !- High Humidity Control",
        "  ,                                       !- Humidistat Control Zone Name",
        "  ,                                       !- High Humidity Outdoor Air Flow Ratio",
        "  Yes,                                    !- Control High Indoor Humidity Based on Outdoor Humidity Ratio",
        "  BypassWhenWithinEconomizerLimits;       !- Heat Recovery Bypass Control Type",

        "AvailabilityManagerAssignmentList,",
        "  Mixing Box Availability Manager List,   !- Name",
        "  AvailabilityManager:Scheduled,          !- Availability Manager Object Type 1",
        "  Mixing Box Availability Manager;        !- Availability Manager Name 1",

        "AvailabilityManager:Scheduled,",
        "  Mixing Box Availability Manager,        !- Name",
        "  Always On Discrete;                     !- Schedule Name",

        "OutdoorAir:NodeList,",
        "  OA Intake Air Node;                     !- Node or NodeList Name 1",

        "AirLoopHVAC:OutdoorAirSystem:EquipmentList,",
        "  Mixing Box Equipment List,              !- Name",
        "  OutdoorAir:Mixer,                       !- Component Object Type 1",
        "  Mixing Box Outdoor Air Mixer;           !- Component Name 1",

        "OutdoorAir:Mixer,",
        "  Mixing Box Outdoor Air Mixer,           !- Name",
        "  Supply Side Mixed Air Node,             !- Mixed Air Node Name",
        "  OA Intake Air Node,                     !- Outdoor Air Stream Node Name",
        "  Relief Air Node,                        !- Relief Air Stream Node Name",
        "  VSD Return Fan Outlet to Mixing Box Node; !- Return Air Stream Node Name",

        "SetpointManager:MixedAir,",
        "  Supply Side Mixed Air Node OS Default SPM, !- Name",
        "  Temperature,                            !- Control Variable",
        "  Supply Side Outlet Node,                !- Reference Setpoint Node Name",
        "  Supply Side Mixed Air Node,             !- Fan Inlet Node Name",
        "  Supply Side Outlet Node,                !- Fan Outlet Node Name",
        "  Supply Side Mixed Air Node;             !- Setpoint Node or NodeList Name",

        // NOTE: The supply has as a min flow of 0
        "Fan:VariableVolume,",
        "  VSD Supply Fan,                         !- Name",
        "  Always On Discrete,                     !- Availability Schedule Name",
        "  0.6045,                                 !- Fan Total Efficiency",
        "  1017.592,                               !- Pressure Rise {Pa}",
        "  1.0,                                    !- Maximum Flow Rate {m3/s}",
        "  FixedFlowRate,                          !- Fan Power Minimum Flow Rate Input Method",
        "  0,                                      !- Fan Power Minimum Flow Fraction",
        "  0,                                      !- Fan Power Minimum Air Flow Rate {m3/s}",
        "  0.93,                                   !- Motor Efficiency",
        "  1,                                      !- Motor In Airstream Fraction",
        "  0.040759894,                            !- Fan Power Coefficient 1",
        "  0.08804497,                             !- Fan Power Coefficient 2",
        "  -0.07292612,                            !- Fan Power Coefficient 3",
        "  0.943739823,                            !- Fan Power Coefficient 4",
        "  0,                                      !- Fan Power Coefficient 5",
        "  Supply Side Mixed Air Node,             !- Air Inlet Node Name",
        "  Supply Side Outlet Node,                !- Air Outlet Node Name",
        "  General;                                !- End-Use Subcategory",

        "SetpointManager:Scheduled,",
        "  Scheduled Deck Temp,                    !- Name",
        "  Temperature,                            !- Control Variable",
        "  Deck Temperature,                       !- Schedule Name",
        "  Supply Side Outlet Node;                !- Setpoint Node or NodeList Name",

        "Schedule:Constant,",
        "  Deck Temperature,                       !- Name",
        "  Any Number,                             !- Schedule Type Limits Name",
        "  14;                                     !- Hourly Value",

        "AirLoopHVAC:SupplyPath,",
        "  Air Loop HVAC Demand Inlet Node Supply Path, !- Name",
        "  Demand Inlet Node,                      !- Supply Air Path Inlet Node Name",
        "  AirLoopHVAC:ZoneSplitter,               !- Component Object Type 1",
        "  Air Loop HVAC Zone Splitter;            !- Component Name 1",

        "AirLoopHVAC:ZoneSplitter,",
        "  Air Loop HVAC Zone Splitter,            !- Name",
        "  Demand Inlet Node,                      !- Inlet Node Name",
        "  ATU VAV No Reheat Inlet Node;           !- Outlet Node Name 1",

        "AirLoopHVAC:ReturnPath,",
        "  Air Loop HVAC Return Path,              !- Name",
        "  Demand Outlet Node,                     !- Return Air Path Outlet Node Name",
        "  AirLoopHVAC:ZoneMixer,                  !- Component Object Type 1",
        "  Air Loop HVAC Zone Mixer;               !- Component Name 1",

        "AirLoopHVAC:ZoneMixer,",
        "  Air Loop HVAC Zone Mixer,               !- Name",
        "  Demand Outlet Node,                     !- Outlet Node Name",
        "  Zone1 Return Air Node;                  !- Inlet Node Name 1",

        "Output:Variable,",
        "  *,                                      !- Key Value",
        "  System Node Mass Flow Rate,             !- Variable Name",
        "  Detailed;                               !- Reporting Frequency",

        "Output:SQLite,",
        "  SimpleAndTabular;                       !- Option Type",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    SimulationManager::ManageSimulation(*state); // run the design days

    int returnFanNode =
        Util::FindItemInList("VSD RETURN FAN OUTLET TO MIXING BOX NODE", state->dataLoopNodes->NodeID, state->dataLoopNodes->NumOfNodes);
    EXPECT_GT(returnFanNode, 0);
    int supplyOutletNode = Util::FindItemInList("SUPPLY SIDE OUTLET NODE", state->dataLoopNodes->NodeID, state->dataLoopNodes->NumOfNodes);
    EXPECT_GT(returnFanNode, 0);
    EXPECT_GT(supplyOutletNode, 0);

    EXPECT_EQ(0, state->dataLoopNodes->Node(returnFanNode).MassFlowRateMin);
    EXPECT_EQ(0, state->dataLoopNodes->Node(supplyOutletNode).MassFlowRateMin);
    EXPECT_EQ(0, state->dataLoopNodes->Node(returnFanNode).MassFlowRate);
    EXPECT_EQ(0, state->dataLoopNodes->Node(supplyOutletNode).MassFlowRate);
}

} // namespace EnergyPlus
