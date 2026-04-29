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

// EnergyPlus::FanCoilUnits Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/FanCoilUnits.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HVACSystemRootFindingAlgorithm.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/HeatingCoils.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/WaterCoils.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;
using namespace EnergyPlus::DataZoneEquipment;
using namespace EnergyPlus::DataZoneEquipment;
using namespace EnergyPlus::DataHeatBalance;
using namespace EnergyPlus::DataHeatBalFanSys;
using namespace EnergyPlus::DataPlant;
using namespace EnergyPlus::DataEnvironment;
using namespace EnergyPlus::DataSizing;
using namespace EnergyPlus::FanCoilUnits;
using namespace EnergyPlus::Fans;
using namespace EnergyPlus::GlobalNames;
using namespace EnergyPlus::HeatBalanceManager;
using namespace EnergyPlus::OutputProcessor;
using namespace EnergyPlus::OutputReportPredefined;
using namespace EnergyPlus::Psychrometrics;
using namespace EnergyPlus::WaterCoils;

namespace EnergyPlus {
TEST_F(EnergyPlusFixture, MultiStage4PipeFanCoilHeatingTest)
{

    int FanCoilNum(1);
    int ZoneNum(1);
    bool FirstHVACIteration(false);
    bool ErrorsFound(false);
    Real64 PartLoadRatio(1.0);
    Real64 SpeedRatio(0.0);
    Real64 QZnReq(0.0);
    Real64 HotWaterMassFlowRate(0.0);
    Real64 ColdWaterMassFlowRate(0.0);
    Real64 QUnitOut(0.0);
    Real64 AirMassFlow(0.0);
    Real64 MaxAirMassFlow(0.0);

    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataWaterCoils->GetWaterCoilsInputFlag = true;
    state->dataGlobalNames->NumCoils = 0;
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;

    std::string const idf_objects = delimited_string({
        "	Zone,",
        "	EAST ZONE, !- Name",
        "	0, !- Direction of Relative North { deg }",
        "	0, !- X Origin { m }",
        "	0, !- Y Origin { m }",
        "	0, !- Z Origin { m }",
        "	1, !- Type",
        "	1, !- Multiplier",
        "	autocalculate, !- Ceiling Height { m }",
        "	autocalculate; !- Volume { m3 }",
        "	ZoneHVAC:EquipmentConnections,",
        "	EAST ZONE, !- Zone Name",
        "	Zone1Equipment, !- Zone Conditioning Equipment List Name",
        "	Zone1Inlets, !- Zone Air Inlet Node or NodeList Name",
        "	Zone1Exhausts, !- Zone Air Exhaust Node or NodeList Name",
        "	Zone 1 Node, !- Zone Air Node Name",
        "	Zone 1 Outlet Node;      !- Zone Return Air Node Name",
        "	ZoneHVAC:EquipmentList,",
        "	Zone1Equipment, !- Name",
        "   SequentialLoad,          !- Load Distribution Scheme",
        "	ZoneHVAC:FourPipeFanCoil, !- Zone Equipment 1 Object Type",
        "	Zone1FanCoil, !- Zone Equipment 1 Name",
        "	1, !- Zone Equipment 1 Cooling Sequence",
        "	1;                       !- Zone Equipment 1 Heating or No - Load Sequence",
        "   NodeList,",
        "	Zone1Inlets, !- Name",
        "	Zone1FanCoilAirOutletNode;  !- Node 1 Name",
        "	NodeList,",
        "	Zone1Exhausts, !- Name",
        "	Zone1FanCoilAirInletNode; !- Node 1 Name",
        "	OutdoorAir:NodeList,",
        "	Zone1FanCoilOAInNode;    !- Node or NodeList Name 1",
        "	OutdoorAir:Mixer,",
        "	Zone1FanCoilOAMixer, !- Name",
        "	Zone1FanCoilOAMixerOutletNode, !- Mixed Air Node Name",
        "	Zone1FanCoilOAInNode, !- Outdoor Air Stream Node Name",
        "	Zone1FanCoilExhNode, !- Relief Air Stream Node Name",
        "	Zone1FanCoilAirInletNode; !- Return Air Stream Node Name",
        "	Schedule:Constant,",
        "	FanAndCoilAvailSched, !- Name",
        "	FRACTION, !- Schedule Type",
        "	1;        !- TimeStep Value",
        "	ScheduleTypeLimits,",
        "	Fraction, !- Name",
        "	0.0, !- Lower Limit Value",
        "	1.0, !- Upper Limit Value",
        "	CONTINUOUS;              !- Numeric Type",
        "   Fan:OnOff,",
        "	Zone1FanCoilFan, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Name",
        "	0.5, !- Fan Total Efficiency",
        "	75.0, !- Pressure Rise { Pa }",
        "	0.6, !- Maximum Flow Rate { m3 / s }",
        "	0.9, !- Motor Efficiency",
        "	1.0, !- Motor In Airstream Fraction",
        "	Zone1FanCoilOAMixerOutletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilFanOutletNode, !- Air Outlet Node Name",
        "	, !- Fan Power Ratio Function of Speed Ratio Curve Name",
        "	;                        !- Fan Efficiency Ratio Function of Speed Ratio Curve Name	",
        "	Coil:Cooling:Water,",
        "	Zone1FanCoilCoolingCoil, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Namev",
        "	0.0002, !- Design Water Flow Rate { m3 / s }",
        "	0.5000, !- Design Air Flow Rate { m3 / s }",
        "	7.22,   !- Design Inlet Water Temperature { Cv }",
        "	24.340, !- Design Inlet Air Temperature { C }",
        "	14.000, !- Design Outlet Air Temperature { C }",
        "	0.0095, !- Design Inlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	0.0090, !- Design Outlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	Zone1FanCoilChWInletNode, !- Water Inlet Node Name",
        "	Zone1FanCoilChWOutletNode, !- Water Outlet Node Name",
        "	Zone1FanCoilFanOutletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilCCOutletNode, !- Air Outlet Node Name",
        "	SimpleAnalysis, !- Type of Analysis",
        "	CrossFlow;               !- Heat Exchanger Configuration",
        "	Coil:Heating:Water,",
        "   Zone1FanCoilHeatingCoil, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Name",
        "	150.0,   !- U - Factor Times Area Value { W / K }",
        "	0.00014, !- Maximum Water Flow Rate { m3 / s }",
        "	Zone1FanCoilHWInletNode, !- Water Inlet Node Name",
        "	Zone1FanCoilHWOutletNode, !- Water Outlet Node Name",
        "	Zone1FanCoilCCOutletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilAirOutletNode, !- Air Outlet Node Name",
        "	UFactorTimesAreaAndDesignWaterFlowRate, !- Performance Input Method",
        "	autosize, !- Rated Capacity { W }",
        "	82.2, !- Rated Inlet Water Temperature { C }",
        "	16.6, !- Rated Inlet Air Temperature { C }",
        "	71.1, !- Rated Outlet Water Temperature { C }",
        "	32.2, !- Rated Outlet Air Temperature { C }",
        "	;     !- Rated Ratio for Air and Water Convection",
        "	ZoneHVAC:FourPipeFanCoil,",
        "	Zone1FanCoil, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Name",
        "	MultiSpeedFan, !- Capacity Control Method",
        "	0.5, !- Maximum Supply Air Flow Rate { m3 / s }",
        "	0.3, !- Low Speed Supply Air Flow Ratio",
        "	0.6, !- Medium Speed Supply Air Flow Ratio",
        "	0.0, !- Maximum Outdoor Air Flow Rate { m3 / s }",
        "	FanAndCoilAvailSched, !- Outdoor Air Schedule Name",
        "	Zone1FanCoilAirInletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilAirOutletNode, !- Air Outlet Node Name",
        "	OutdoorAir:Mixer, !- Outdoor Air Mixer Object Type",
        "	Zone1FanCoilOAMixer, !- Outdoor Air Mixer Name",
        "	Fan:OnOff, !- Supply Air Fan Object Type",
        "	Zone1FanCoilFan, !- Supply Air Fan Name",
        "	Coil:Cooling:Water, !- Cooling Coil Object Type",
        "	Zone1FanCoilCoolingCoil, !- Cooling Coil Name",
        "	0.00014, !- Maximum Cold Water Flow Rate { m3 / s }",
        "	0.0, !- Minimum Cold Water Flow Rate { m3 / s }",
        "	0.001, !- Cooling Convergence Tolerance",
        "	Coil:Heating:Water, !- Heating Coil Object Type",
        "	Zone1FanCoilHeatingCoil, !- Heating Coil Name",
        "	0.00014, !- Maximum Hot Water Flow Rate { m3 / s }",
        "	0.0, !- Minimum Hot Water Flow Rate { m3 / s }",
        "	0.001; !- Heating Convergence Tolerance",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);

    EXPECT_EQ("EAST ZONE", state->dataHeatBal->Zone(1).Name);

    GetZoneEquipmentData(*state);
    GetFanInput(*state);
    EXPECT_EQ((int)HVAC::FanType::OnOff, (int)state->dataFans->fans(1)->type);

    GetFanCoilUnits(*state);
    EXPECT_ENUM_EQ(CCM::MultiSpeedFan, state->dataFanCoilUnits->FanCoil(1).CapCtrlMeth_Num);
    EXPECT_EQ("OUTDOORAIR:MIXER", state->dataFanCoilUnits->FanCoil(1).OAMixType);
    EXPECT_EQ((int)HVAC::FanType::OnOff, (int)state->dataFanCoilUnits->FanCoil(1).fanType);
    EXPECT_EQ("COIL:COOLING:WATER", state->dataFanCoilUnits->FanCoil(1).CCoilType);
    EXPECT_EQ("COIL:HEATING:WATER", state->dataFanCoilUnits->FanCoil(1).HCoilType);

    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

    AirMassFlow = 0.60;
    MaxAirMassFlow = 0.60;
    // heating load only
    ColdWaterMassFlowRate = 0.0;
    HotWaterMassFlowRate = 1.0;

    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).MassFlowRateMax = MaxAirMassFlow;

    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Temp = 22.0;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Enthalpy = 36000;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).HumRat =
        PsyWFnTdbH(*state,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Temp,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Enthalpy);

    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Temp = 10.0;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Enthalpy = 18000;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).HumRat =
        PsyWFnTdbH(*state,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Temp,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Enthalpy);

    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMinAvail = AirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMax = MaxAirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMaxAvail = MaxAirMassFlow;

    state->dataFanCoilUnits->FanCoil(1).OutAirMassFlow = 0.0;
    state->dataFanCoilUnits->FanCoil(1).MaxAirMassFlow = MaxAirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).OutsideAirNode).MassFlowRateMax = 0.0;

    auto *fan1 = state->dataFans->fans(1);

    fan1->inletAirMassFlowRate = AirMassFlow;
    fan1->maxAirMassFlowRate = MaxAirMassFlow;

    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRateMax = AirMassFlow;
    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

    state->dataWaterCoils->WaterCoil(2).UACoilTotal = 470.0;
    state->dataWaterCoils->WaterCoil(2).UACoilExternal = 611.0;
    state->dataWaterCoils->WaterCoil(2).UACoilInternal = 2010.0;
    state->dataWaterCoils->WaterCoil(2).TotCoilOutsideSurfArea = 50.0;

    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).AirInletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).AirInletNodeNum).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).AirInletNodeNum).MassFlowRateMax = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).AirInletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

    state->dataWaterCoils->WaterCoil(2).InletWaterMassFlowRate = ColdWaterMassFlowRate;
    state->dataWaterCoils->WaterCoil(2).MaxWaterMassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterInletNodeNum).MassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterInletNodeNum).MassFlowRateMaxAvail = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterInletNodeNum).Temp = 6.0;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterOutletNodeNum).MassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterOutletNodeNum).MassFlowRateMaxAvail = ColdWaterMassFlowRate;

    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).AirInletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).AirInletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum).Temp = 60.0;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum).MassFlowRate = HotWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum).MassFlowRateMaxAvail = HotWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum).MassFlowRate = HotWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum).MassFlowRateMaxAvail = HotWaterMassFlowRate;
    state->dataWaterCoils->WaterCoil(1).InletWaterMassFlowRate = HotWaterMassFlowRate;
    state->dataWaterCoils->WaterCoil(1).MaxWaterMassFlowRate = HotWaterMassFlowRate;

    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }

    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.loopNum = 1;
    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.branchNum = 1;
    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataWaterCoils->WaterCoil(2).WaterPlantLoc);

    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.loopNum = 2;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.branchNum = 1;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataWaterCoils->WaterCoil(1).WaterPlantLoc);

    state->dataPlnt->PlantLoop(2).Name = "ChilledWaterLoop";
    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = state->dataWaterCoils->WaterCoil(2).Name;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilWaterCooling;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterCoils->WaterCoil(2).WaterInletNodeNum;

    state->dataPlnt->PlantLoop(1).Name = "HotWaterLoop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = state->dataWaterCoils->WaterCoil(1).Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum;

    state->dataFanCoilUnits->CoolingLoad = false;
    state->dataFanCoilUnits->HeatingLoad = true;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = 0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 4000.0;
    state->dataFanCoilUnits->FanCoil(1).SpeedFanSel = 2;
    QUnitOut = 0.0;
    QZnReq = 4000.0;

    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(2);
    state->dataWaterCoils->MyUAAndFlowCalcFlag(1) = true;
    state->dataWaterCoils->MyUAAndFlowCalcFlag(2) = true;
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

    CalcMultiStage4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QZnReq, SpeedRatio, PartLoadRatio, QUnitOut);

    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    // expect inlet and outlet node air mass flow rates are equal
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate,
              state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirOutNode).MassFlowRate);

    state->dataGlobal->DoingSizing = false;

    state->dataPlnt->PlantLoop.deallocate();
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.deallocate();
    state->dataFanCoilUnits->FanCoil.deallocate();
    state->dataLoopNodes->Node.deallocate();
    state->dataWaterCoils->WaterCoil.deallocate();
    state->dataZoneEquip->ZoneEquipConfig.deallocate();
    state->dataHeatBal->Zone.deallocate();
}
TEST_F(EnergyPlusFixture, MultiStage4PipeFanCoilCoolingTest)
{

    int FanCoilNum(1);
    int ZoneNum(1);
    bool FirstHVACIteration(false);
    bool ErrorsFound(false);
    Real64 PartLoadRatio(1.0);
    Real64 SpeedRatio(0.0);
    Real64 QZnReq(0.0);
    Real64 HotWaterMassFlowRate(0.0);
    Real64 ColdWaterMassFlowRate(0.0);
    Real64 QUnitOut(0.0);
    Real64 AirMassFlow(0.0);
    Real64 MaxAirMassFlow(0.0);

    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataWaterCoils->GetWaterCoilsInputFlag = true;
    state->dataGlobalNames->NumCoils = 0;
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;

    std::string const idf_objects = delimited_string({
        "	Zone,",
        "	EAST ZONE, !- Name",
        "	0, !- Direction of Relative North { deg }",
        "	0, !- X Origin { m }",
        "	0, !- Y Origin { m }",
        "	0, !- Z Origin { m }",
        "	1, !- Type",
        "	1, !- Multiplier",
        "	autocalculate, !- Ceiling Height { m }",
        "	autocalculate; !- Volume { m3 }",
        "	ZoneHVAC:EquipmentConnections,",
        "	EAST ZONE, !- Zone Name",
        "	Zone1Equipment, !- Zone Conditioning Equipment List Name",
        "	Zone1Inlets, !- Zone Air Inlet Node or NodeList Name",
        "	Zone1Exhausts, !- Zone Air Exhaust Node or NodeList Name",
        "	Zone 1 Node, !- Zone Air Node Name",
        "	Zone 1 Outlet Node;      !- Zone Return Air Node Name",
        "	ZoneHVAC:EquipmentList,",
        "	Zone1Equipment, !- Name",
        "   SequentialLoad,          !- Load Distribution Scheme",
        "	ZoneHVAC:FourPipeFanCoil, !- Zone Equipment 1 Object Type",
        "	Zone1FanCoil, !- Zone Equipment 1 Name",
        "	1, !- Zone Equipment 1 Cooling Sequence",
        "	1;                       !- Zone Equipment 1 Heating or No - Load Sequence",
        "   NodeList,",
        "	Zone1Inlets, !- Name",
        "	Zone1FanCoilAirOutletNode;  !- Node 1 Name",
        "	NodeList,",
        "	Zone1Exhausts, !- Name",
        "	Zone1FanCoilAirInletNode; !- Node 1 Name",
        "	OutdoorAir:NodeList,",
        "	Zone1FanCoilOAInNode;    !- Node or NodeList Name 1",
        "	OutdoorAir:Mixer,",
        "	Zone1FanCoilOAMixer, !- Name",
        "	Zone1FanCoilOAMixerOutletNode, !- Mixed Air Node Name",
        "	Zone1FanCoilOAInNode, !- Outdoor Air Stream Node Name",
        "	Zone1FanCoilExhNode, !- Relief Air Stream Node Name",
        "	Zone1FanCoilAirInletNode; !- Return Air Stream Node Name",
        "	Schedule:Compact,",
        "	FanAndCoilAvailSched, !- Name",
        "	Fraction, !- Schedule Type Limits Name",
        "	Through: 12/31, !- Field 1",
        "	For: AllDays, !- Field 2",
        "	Until: 24:00, 1.0;        !- Field 3",
        "	ScheduleTypeLimits,",
        "	Fraction, !- Name",
        "	0.0, !- Lower Limit Value",
        "	1.0, !- Upper Limit Value",
        "	CONTINUOUS;              !- Numeric Type",
        "   Fan:OnOff,",
        "	Zone1FanCoilFan, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Name",
        "	0.5, !- Fan Total Efficiency",
        "	75.0, !- Pressure Rise { Pa }",
        "	0.6, !- Maximum Flow Rate { m3 / s }",
        "	0.9, !- Motor Efficiency",
        "	1.0, !- Motor In Airstream Fraction",
        "	Zone1FanCoilOAMixerOutletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilFanOutletNode, !- Air Outlet Node Name",
        "	, !- Fan Power Ratio Function of Speed Ratio Curve Name",
        "	;                        !- Fan Efficiency Ratio Function of Speed Ratio Curve Name	",
        "	Coil:Cooling:Water,",
        "	Zone1FanCoilCoolingCoil, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Namev",
        "	0.0002, !- Design Water Flow Rate { m3 / s }",
        "	0.5000, !- Design Air Flow Rate { m3 / s }",
        "	7.22,   !- Design Inlet Water Temperature { Cv }",
        "	24.340, !- Design Inlet Air Temperature { C }",
        "	14.000, !- Design Outlet Air Temperature { C }",
        "	0.0095, !- Design Inlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	0.0090, !- Design Outlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	Zone1FanCoilChWInletNode, !- Water Inlet Node Name",
        "	Zone1FanCoilChWOutletNode, !- Water Outlet Node Name",
        "	Zone1FanCoilFanOutletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilCCOutletNode, !- Air Outlet Node Name",
        "	SimpleAnalysis, !- Type of Analysis",
        "	CrossFlow;               !- Heat Exchanger Configuration",
        "	Coil:Heating:Water,",
        "   Zone1FanCoilHeatingCoil, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Name",
        "	150.0,   !- U - Factor Times Area Value { W / K }",
        "	0.00014, !- Maximum Water Flow Rate { m3 / s }",
        "	Zone1FanCoilHWInletNode, !- Water Inlet Node Name",
        "	Zone1FanCoilHWOutletNode, !- Water Outlet Node Name",
        "	Zone1FanCoilCCOutletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilAirOutletNode, !- Air Outlet Node Name",
        "	UFactorTimesAreaAndDesignWaterFlowRate, !- Performance Input Method",
        "	autosize, !- Rated Capacity { W }",
        "	82.2, !- Rated Inlet Water Temperature { C }",
        "	16.6, !- Rated Inlet Air Temperature { C }",
        "	71.1, !- Rated Outlet Water Temperature { C }",
        "	32.2, !- Rated Outlet Air Temperature { C }",
        "	;     !- Rated Ratio for Air and Water Convection",
        "	ZoneHVAC:FourPipeFanCoil,",
        "	Zone1FanCoil, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Name",
        "	MultiSpeedFan, !- Capacity Control Method",
        "	0.5, !- Maximum Supply Air Flow Rate { m3 / s }",
        "	0.3, !- Low Speed Supply Air Flow Ratio",
        "	0.6, !- Medium Speed Supply Air Flow Ratio",
        "	0.1, !- Maximum Outdoor Air Flow Rate { m3 / s }",
        "	FanAndCoilAvailSched, !- Outdoor Air Schedule Name",
        "	Zone1FanCoilAirInletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilAirOutletNode, !- Air Outlet Node Name",
        "	OutdoorAir:Mixer, !- Outdoor Air Mixer Object Type",
        "	Zone1FanCoilOAMixer, !- Outdoor Air Mixer Name",
        "	Fan:OnOff, !- Supply Air Fan Object Type",
        "	Zone1FanCoilFan, !- Supply Air Fan Name",
        "	Coil:Cooling:Water, !- Cooling Coil Object Type",
        "	Zone1FanCoilCoolingCoil, !- Cooling Coil Name",
        "	0.00014, !- Maximum Cold Water Flow Rate { m3 / s }",
        "	0.0, !- Minimum Cold Water Flow Rate { m3 / s }",
        "	0.001, !- Cooling Convergence Tolerance",
        "	Coil:Heating:Water, !- Heating Coil Object Type",
        "	Zone1FanCoilHeatingCoil, !- Heating Coil Name",
        "	0.00014, !- Maximum Hot Water Flow Rate { m3 / s }",
        "	0.0, !- Minimum Hot Water Flow Rate { m3 / s }",
        "	0.001; !- Heating Convergence Tolerance",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);
    GetZoneData(*state, ErrorsFound);
    EXPECT_EQ("EAST ZONE", state->dataHeatBal->Zone(1).Name);

    GetZoneEquipmentData(*state);
    GetFanInput(*state);
    EXPECT_EQ((int)HVAC::FanType::OnOff, (int)state->dataFans->fans(1)->type);

    GetFanCoilUnits(*state);
    EXPECT_ENUM_EQ(CCM::MultiSpeedFan, state->dataFanCoilUnits->FanCoil(1).CapCtrlMeth_Num);
    EXPECT_EQ("OUTDOORAIR:MIXER", state->dataFanCoilUnits->FanCoil(1).OAMixType);
    EXPECT_EQ((int)HVAC::FanType::OnOff, (int)state->dataFanCoilUnits->FanCoil(1).fanType);
    EXPECT_EQ("COIL:COOLING:WATER", state->dataFanCoilUnits->FanCoil(1).CCoilType);
    EXPECT_EQ("COIL:HEATING:WATER", state->dataFanCoilUnits->FanCoil(1).HCoilType);

    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

    AirMassFlow = 0.60;
    MaxAirMassFlow = 0.60;

    // cooling load only
    HotWaterMassFlowRate = 0.0;
    ColdWaterMassFlowRate = 1.0;

    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).MassFlowRateMax = MaxAirMassFlow;

    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Temp = 24.0;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Enthalpy = 36000;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).HumRat =
        PsyWFnTdbH(*state,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Temp,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Enthalpy);

    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Temp = 30.0;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Enthalpy = 53000;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).HumRat =
        PsyWFnTdbH(*state,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Temp,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Enthalpy);

    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMinAvail = AirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMax = MaxAirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMaxAvail = MaxAirMassFlow;

    state->dataFanCoilUnits->FanCoil(1).OutAirMassFlow = 0.0;
    state->dataFanCoilUnits->FanCoil(1).MaxAirMassFlow = MaxAirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).OutsideAirNode).MassFlowRateMax = 0.0;

    auto *fan1 = state->dataFans->fans(1);
    fan1->inletAirMassFlowRate = AirMassFlow;
    fan1->maxAirMassFlowRate = MaxAirMassFlow;

    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRateMax = AirMassFlow;
    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

    state->dataWaterCoils->WaterCoil(2).UACoilTotal = 470.0;
    state->dataWaterCoils->WaterCoil(2).UACoilExternal = 611.0;
    state->dataWaterCoils->WaterCoil(2).UACoilInternal = 2010.0;
    state->dataWaterCoils->WaterCoil(2).TotCoilOutsideSurfArea = 50.0;

    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).AirInletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).AirInletNodeNum).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).AirInletNodeNum).MassFlowRateMax = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).AirInletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

    state->dataWaterCoils->WaterCoil(2).InletWaterMassFlowRate = ColdWaterMassFlowRate;
    state->dataWaterCoils->WaterCoil(2).MaxWaterMassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterInletNodeNum).MassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterInletNodeNum).MassFlowRateMaxAvail = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterInletNodeNum).Temp = 6.0;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterOutletNodeNum).MassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterOutletNodeNum).MassFlowRateMaxAvail = ColdWaterMassFlowRate;

    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).AirInletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).AirInletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum).Temp = 60.0;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum).MassFlowRate = HotWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum).MassFlowRateMaxAvail = HotWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum).MassFlowRate = HotWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum).MassFlowRateMaxAvail = HotWaterMassFlowRate;
    state->dataWaterCoils->WaterCoil(1).InletWaterMassFlowRate = HotWaterMassFlowRate;
    state->dataWaterCoils->WaterCoil(1).MaxWaterMassFlowRate = HotWaterMassFlowRate;

    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }

    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.loopNum = 1;
    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.branchNum = 1;
    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataWaterCoils->WaterCoil(2).WaterPlantLoc);

    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.loopNum = 2;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.branchNum = 1;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataWaterCoils->WaterCoil(1).WaterPlantLoc);

    state->dataPlnt->PlantLoop(2).Name = "ChilledWaterLoop";
    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = state->dataWaterCoils->WaterCoil(2).Name;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilWaterCooling;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterCoils->WaterCoil(2).WaterInletNodeNum;

    state->dataPlnt->PlantLoop(1).Name = "HotWaterLoop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = state->dataWaterCoils->WaterCoil(1).Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum;

    state->dataFanCoilUnits->HeatingLoad = false;
    state->dataFanCoilUnits->CoolingLoad = true;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -4000.00;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 0.0;
    state->dataFanCoilUnits->FanCoil(1).SpeedFanSel = 2;
    QUnitOut = 0.0;
    QZnReq = -4000.0;

    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(2);
    state->dataWaterCoils->MyUAAndFlowCalcFlag(1) = true;
    state->dataWaterCoils->MyUAAndFlowCalcFlag(2) = true;
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

    CalcMultiStage4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QZnReq, SpeedRatio, PartLoadRatio, QUnitOut);

    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    // expect inlet and outlet node air mass flow rates are equal
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate,
              state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirOutNode).MassFlowRate);

    state->dataGlobal->DoingSizing = false;
    state->dataPlnt->PlantLoop.deallocate();
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.deallocate();
    state->dataFanCoilUnits->FanCoil.deallocate();
    state->dataLoopNodes->Node.deallocate();
    state->dataWaterCoils->WaterCoil.deallocate();
    state->dataZoneEquip->ZoneEquipConfig.deallocate();
    state->dataHeatBal->Zone.deallocate();
}
TEST_F(EnergyPlusFixture, ConstantFanVariableFlowFanCoilHeatingTest)
{

    int FanCoilNum(1);
    int ZoneNum(1);
    bool FirstHVACIteration(true);
    bool ErrorsFound(false);
    Real64 QZnReq(0.0);
    Real64 HotWaterMassFlowRate(0.0);
    Real64 ColdWaterMassFlowRate(0.0);
    Real64 QUnitOut(0.0);
    Real64 AirMassFlow(0.0);
    Real64 MaxAirMassFlow(0.0);
    Real64 LatOutputProvided(0.0);

    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataWaterCoils->GetWaterCoilsInputFlag = true;
    state->dataGlobalNames->NumCoils = 0;
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;

    std::string const idf_objects = delimited_string({
        "	Zone,",
        "	EAST ZONE, !- Name",
        "	0, !- Direction of Relative North { deg }",
        "	0, !- X Origin { m }",
        "	0, !- Y Origin { m }",
        "	0, !- Z Origin { m }",
        "	1, !- Type",
        "	1, !- Multiplier",
        "	autocalculate, !- Ceiling Height { m }",
        "	autocalculate; !- Volume { m3 }",
        "	ZoneHVAC:EquipmentConnections,",
        "	EAST ZONE, !- Zone Name",
        "	Zone1Equipment, !- Zone Conditioning Equipment List Name",
        "	Zone1Inlets, !- Zone Air Inlet Node or NodeList Name",
        "	Zone1Exhausts, !- Zone Air Exhaust Node or NodeList Name",
        "	Zone 1 Node, !- Zone Air Node Name",
        "	Zone 1 Outlet Node;      !- Zone Return Air Node Name",
        "	ZoneHVAC:EquipmentList,",
        "	Zone1Equipment, !- Name",
        "   SequentialLoad,          !- Load Distribution Scheme",
        "	ZoneHVAC:FourPipeFanCoil, !- Zone Equipment 1 Object Type",
        "	Zone1FanCoil, !- Zone Equipment 1 Name",
        "	1, !- Zone Equipment 1 Cooling Sequence",
        "	1;                       !- Zone Equipment 1 Heating or No - Load Sequence",
        "   NodeList,",
        "	Zone1Inlets, !- Name",
        "	Zone1FanCoilAirOutletNode;  !- Node 1 Name",
        "	NodeList,",
        "	Zone1Exhausts, !- Name",
        "	Zone1FanCoilAirInletNode; !- Node 1 Name",
        "	OutdoorAir:NodeList,",
        "	Zone1FanCoilOAInNode;    !- Node or NodeList Name 1",
        "	OutdoorAir:Mixer,",
        "	Zone1FanCoilOAMixer, !- Name",
        "	Zone1FanCoilOAMixerOutletNode, !- Mixed Air Node Name",
        "	Zone1FanCoilOAInNode, !- Outdoor Air Stream Node Name",
        "	Zone1FanCoilExhNode, !- Relief Air Stream Node Name",
        "	Zone1FanCoilAirInletNode; !- Return Air Stream Node Name",
        "	Schedule:Constant,",
        "	FanAndCoilAvailSched, !- Name",
        "	FRACTION, !- Schedule Type",
        "	1;        !- TimeStep Value",
        "	ScheduleTypeLimits,",
        "	Fraction, !- Name",
        "	0.0, !- Lower Limit Value",
        "	1.0, !- Upper Limit Value",
        "	CONTINUOUS;              !- Numeric Type",
        "   Fan:OnOff,",
        "	Zone1FanCoilFan, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Name",
        "	0.5, !- Fan Total Efficiency",
        "	75.0, !- Pressure Rise { Pa }",
        "	0.6, !- Maximum Flow Rate { m3 / s }",
        "	0.9, !- Motor Efficiency",
        "	1.0, !- Motor In Airstream Fraction",
        "	Zone1FanCoilOAMixerOutletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilFanOutletNode, !- Air Outlet Node Name",
        "	, !- Fan Power Ratio Function of Speed Ratio Curve Name",
        "	;                        !- Fan Efficiency Ratio Function of Speed Ratio Curve Name	",
        "	Coil:Cooling:Water,",
        "	Zone1FanCoilCoolingCoil, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Namev",
        "	0.0002, !- Design Water Flow Rate { m3 / s }",
        "	0.5000, !- Design Air Flow Rate { m3 / s }",
        "	7.22,   !- Design Inlet Water Temperature { Cv }",
        "	24.340, !- Design Inlet Air Temperature { C }",
        "	14.000, !- Design Outlet Air Temperature { C }",
        "	0.0095, !- Design Inlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	0.0090, !- Design Outlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	Zone1FanCoilChWInletNode, !- Water Inlet Node Name",
        "	Zone1FanCoilChWOutletNode, !- Water Outlet Node Name",
        "	Zone1FanCoilFanOutletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilCCOutletNode, !- Air Outlet Node Name",
        "	SimpleAnalysis, !- Type of Analysis",
        "	CrossFlow;               !- Heat Exchanger Configuration",
        "	Coil:Heating:Water,",
        "   Zone1FanCoilHeatingCoil, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Name",
        "	150.0,   !- U - Factor Times Area Value { W / K }",
        "	0.00014, !- Maximum Water Flow Rate { m3 / s }",
        "	Zone1FanCoilHWInletNode, !- Water Inlet Node Name",
        "	Zone1FanCoilHWOutletNode, !- Water Outlet Node Name",
        "	Zone1FanCoilCCOutletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilAirOutletNode, !- Air Outlet Node Name",
        "	UFactorTimesAreaAndDesignWaterFlowRate, !- Performance Input Method",
        "	autosize, !- Rated Capacity { W }",
        "	82.2, !- Rated Inlet Water Temperature { C }",
        "	16.6, !- Rated Inlet Air Temperature { C }",
        "	71.1, !- Rated Outlet Water Temperature { C }",
        "	32.2, !- Rated Outlet Air Temperature { C }",
        "	;     !- Rated Ratio for Air and Water Convection",
        "	ZoneHVAC:FourPipeFanCoil,",
        "	Zone1FanCoil, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Name",
        "	ConstantFanVariableFlow, !- Capacity Control Method",
        "	0.5, !- Maximum Supply Air Flow Rate { m3 / s }",
        "	0.3, !- Low Speed Supply Air Flow Ratio",
        "	0.6, !- Medium Speed Supply Air Flow Ratio",
        "	0.0, !- Maximum Outdoor Air Flow Rate { m3 / s }",
        "	FanAndCoilAvailSched, !- Outdoor Air Schedule Name",
        "	Zone1FanCoilAirInletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilAirOutletNode, !- Air Outlet Node Name",
        "	OutdoorAir:Mixer, !- Outdoor Air Mixer Object Type",
        "	Zone1FanCoilOAMixer, !- Outdoor Air Mixer Name",
        "	Fan:OnOff, !- Supply Air Fan Object Type",
        "	Zone1FanCoilFan, !- Supply Air Fan Name",
        "	Coil:Cooling:Water, !- Cooling Coil Object Type",
        "	Zone1FanCoilCoolingCoil, !- Cooling Coil Name",
        "	0.00014, !- Maximum Cold Water Flow Rate { m3 / s }",
        "	0.0, !- Minimum Cold Water Flow Rate { m3 / s }",
        "	0.001, !- Cooling Convergence Tolerance",
        "	Coil:Heating:Water, !- Heating Coil Object Type",
        "	Zone1FanCoilHeatingCoil, !- Heating Coil Name",
        "	0.00014, !- Maximum Hot Water Flow Rate { m3 / s }",
        "	0.0, !- Minimum Hot Water Flow Rate { m3 / s }",
        "	0.001; !- Heating Convergence Tolerance",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    EXPECT_EQ("EAST ZONE", state->dataHeatBal->Zone(1).Name);

    GetZoneEquipmentData(*state);
    GetFanInput(*state);
    EXPECT_EQ((int)HVAC::FanType::OnOff, (int)state->dataFans->fans(1)->type);

    GetFanCoilUnits(*state);
    EXPECT_ENUM_EQ(CCM::ConsFanVarFlow, state->dataFanCoilUnits->FanCoil(1).CapCtrlMeth_Num);
    EXPECT_EQ("OUTDOORAIR:MIXER", state->dataFanCoilUnits->FanCoil(1).OAMixType);
    EXPECT_EQ((int)HVAC::FanType::OnOff, (int)state->dataFanCoilUnits->FanCoil(1).fanType);
    EXPECT_EQ("COIL:COOLING:WATER", state->dataFanCoilUnits->FanCoil(1).CCoilType);
    EXPECT_EQ("COIL:HEATING:WATER", state->dataFanCoilUnits->FanCoil(1).HCoilType);

    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

    AirMassFlow = 0.60;
    MaxAirMassFlow = 0.60;
    // heating load only
    ColdWaterMassFlowRate = 0.0;
    HotWaterMassFlowRate = 0.14;

    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).MassFlowRateMax = MaxAirMassFlow;

    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Temp = 22.0;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Enthalpy = 36000;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).HumRat =
        PsyWFnTdbH(*state,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Temp,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Enthalpy);

    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Temp = 10.0;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Enthalpy = 18000;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).HumRat =
        PsyWFnTdbH(*state,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Temp,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Enthalpy);

    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMinAvail = AirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMax = MaxAirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMaxAvail = MaxAirMassFlow;

    state->dataFanCoilUnits->FanCoil(1).OutAirMassFlow = 0.0;
    state->dataFanCoilUnits->FanCoil(1).MaxAirMassFlow = MaxAirMassFlow;
    state->dataFanCoilUnits->FanCoil(1).MaxCoolCoilFluidFlow = 0.14;
    state->dataFanCoilUnits->FanCoil(1).MaxHeatCoilFluidFlow = 0.14;

    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).OutsideAirNode).MassFlowRateMax = 0.0;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).CoolCoilFluidInletNode).MassFlowRateMax = 0.14;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).HeatCoilFluidInletNode).MassFlowRateMax = 0.14;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).CoolCoilFluidInletNode).MassFlowRateMaxAvail = 0.14;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).HeatCoilFluidInletNode).MassFlowRateMaxAvail = 0.14;

    auto *fan1 = state->dataFans->fans(1);
    fan1->inletAirMassFlowRate = AirMassFlow;
    fan1->maxAirMassFlowRate = MaxAirMassFlow;

    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRateMax = AirMassFlow;
    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

    state->dataWaterCoils->WaterCoil(2).UACoilTotal = 470.0;
    state->dataWaterCoils->WaterCoil(2).UACoilExternal = 611.0;
    state->dataWaterCoils->WaterCoil(2).UACoilInternal = 2010.0;
    state->dataWaterCoils->WaterCoil(2).TotCoilOutsideSurfArea = 50.0;

    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).AirInletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).AirInletNodeNum).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).AirInletNodeNum).MassFlowRateMax = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).AirInletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

    state->dataWaterCoils->WaterCoil(2).InletWaterMassFlowRate = ColdWaterMassFlowRate;
    state->dataWaterCoils->WaterCoil(2).MaxWaterMassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterInletNodeNum).MassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterInletNodeNum).MassFlowRateMaxAvail = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterInletNodeNum).Temp = 6.0;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterOutletNodeNum).MassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterOutletNodeNum).MassFlowRateMaxAvail = ColdWaterMassFlowRate;

    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).AirInletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).AirInletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum).Temp = 60.0;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum).MassFlowRate = HotWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum).MassFlowRateMaxAvail = HotWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum).MassFlowRate = HotWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum).MassFlowRateMaxAvail = HotWaterMassFlowRate;
    state->dataWaterCoils->WaterCoil(1).InletWaterMassFlowRate = HotWaterMassFlowRate;
    state->dataWaterCoils->WaterCoil(1).MaxWaterMassFlowRate = HotWaterMassFlowRate;

    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }

    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;

    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.loopNum = 1;
    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.branchNum = 1;
    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataWaterCoils->WaterCoil(2).WaterPlantLoc);

    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.loopNum = 2;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.branchNum = 1;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataWaterCoils->WaterCoil(1).WaterPlantLoc);

    state->dataPlnt->PlantLoop(2).Name = "ChilledWaterLoop";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = state->dataWaterCoils->WaterCoil(2).Name;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilWaterCooling;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterCoils->WaterCoil(2).WaterInletNodeNum;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut =
        state->dataWaterCoils->WaterCoil(2).WaterOutletNodeNum;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).FlowLock = DataPlant::FlowLock::Unlocked;

    state->dataPlnt->PlantLoop(1).Name = "HotWaterLoop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = state->dataWaterCoils->WaterCoil(1).Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut =
        state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).FlowLock = DataPlant::FlowLock::Unlocked;

    state->dataFanCoilUnits->FanCoil(1).CoolCoilPlantLoc.loopNum = 2;
    state->dataFanCoilUnits->FanCoil(1).HeatCoilPlantLoc.loopNum = 1;
    state->dataFanCoilUnits->FanCoil(1).CoolCoilPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    state->dataFanCoilUnits->FanCoil(1).HeatCoilPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    state->dataFanCoilUnits->FanCoil(1).HeatCoilFluidOutletNodeNum = state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum;
    state->dataFanCoilUnits->FanCoil(1).CoolCoilFluidOutletNodeNum = state->dataWaterCoils->WaterCoil(2).WaterOutletNodeNum;
    state->dataFanCoilUnits->FanCoil(1).CoolCoilPlantLoc.branchNum = 1;
    state->dataFanCoilUnits->FanCoil(1).CoolCoilPlantLoc.compNum = 1;
    state->dataFanCoilUnits->FanCoil(1).HeatCoilPlantLoc.branchNum = 1;
    state->dataFanCoilUnits->FanCoil(1).HeatCoilPlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataFanCoilUnits->FanCoil(1).CoolCoilPlantLoc);
    PlantUtilities::SetPlantLocationLinks(*state, state->dataFanCoilUnits->FanCoil(1).HeatCoilPlantLoc);

    state->dataFanCoilUnits->CoolingLoad = false;
    state->dataFanCoilUnits->HeatingLoad = true;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = 8000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 4000.0;
    state->dataFanCoilUnits->FanCoil(1).SpeedFanSel = 2;
    QUnitOut = 0.0;
    QZnReq = 4000.0;

    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(2);
    state->dataWaterCoils->MyUAAndFlowCalcFlag(1) = true;
    state->dataWaterCoils->MyUAAndFlowCalcFlag(2) = true;
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

    // Normal heating simulation for fan coil with constant fan, variable water flow
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, LatOutputProvided);
    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    // expect inlet and outlet node air mass flow rates are equal
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate,
              state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirOutNode).MassFlowRate);
    FirstHVACIteration = false;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).FlowLock = DataPlant::FlowLock::Locked;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).HeatCoilFluidInletNode).MassFlowRate = 0.2;
    // Simulate with flow lock on and locked flow > demand flow; bypass extra flow
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, LatOutputProvided);
    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    EXPECT_NEAR(55.31, state->dataLoopNodes->Node(10).Temp, 0.1);
    // expect inlet and outlet node air mass flow rates are equal
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate,
              state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirOutNode).MassFlowRate);
    // heating simulation with flow lock on and locked flow < flow required for load; use locked flow
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).HeatCoilFluidInletNode).MassFlowRate = 0.05;
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, LatOutputProvided);
    EXPECT_NEAR(3780.0, QUnitOut, 5.0);
    // expect inlet and outlet node air mass flow rates are equal
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate,
              state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirOutNode).MassFlowRate);
    // normal heating, no flow lock, heating capacity exceeded
    QZnReq = 5000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 5000.00;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).FlowLock = DataPlant::FlowLock::Unlocked;
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, LatOutputProvided);
    EXPECT_NEAR(4420.0, QUnitOut, 5.0);
    // expect inlet and outlet node air mass flow rates are equal
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate,
              state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirOutNode).MassFlowRate);

    // Coil Off Capacity Test #1 - low heating load, no flow lock, setting QUnitOutNoHC when flow lock = 0
    QZnReq = 80.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 80.00;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).FlowLock = DataPlant::FlowLock::Unlocked;
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, LatOutputProvided);
    // FC hits the 80 W target load
    EXPECT_NEAR(80.0, QUnitOut, 1.0);
    EXPECT_NEAR(75.0, state->dataFanCoilUnits->FanCoil(1).QUnitOutNoHC, 1.0);
    // water mass flow rate needed to provide output of 80 W (including 75 W coil off capacity)
    EXPECT_NEAR(0.0000315, state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(FanCoilNum).HeatCoilFluidInletNode).MassFlowRate, 0.000001);
    // expect inlet and outlet node air mass flow rates are equal
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate,
              state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirOutNode).MassFlowRate);

    // Coil Off Capacity Test #2 - lock plant flow after previous call
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).FlowLock = DataPlant::FlowLock::Locked;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Temp =
        25.0; // change inlet air condition so off capacity will change to see if QUnitOutNoHC remains fixed
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Enthalpy = 39000;
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, LatOutputProvided);
    // FC does not hit the 80 W target load since flow is locked at a low value
    EXPECT_NEAR(52.0, QUnitOut, 1.0);
    // off coil capacity is same as just prior to flow being locked
    EXPECT_NEAR(75.0, state->dataFanCoilUnits->FanCoil(1).QUnitOutNoHC, 1.0);
    // same water flow rate as before
    EXPECT_NEAR(0.0000315, state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(FanCoilNum).HeatCoilFluidInletNode).MassFlowRate, 0.000001);
    // expect inlet and outlet node air mass flow rates are equal
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate,
              state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirOutNode).MassFlowRate);

    // Coil Off Capacity Test #3 - unlock plant flow to ensure that water flow rate would have been different had flow not been locked
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).FlowLock = DataPlant::FlowLock::Unlocked;
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, LatOutputProvided);
    // FC hits the 80 W target load
    EXPECT_NEAR(80.0, QUnitOut, 1.0);
    // actual coil off output when inlet air temp = 25 C and h = 39000 J/kg
    EXPECT_NEAR(48.0,
                state->dataFanCoilUnits->FanCoil(1).QUnitOutNoHC,
                1.0); // interesting that this is very different for a heating system (from Coil Off Capacity Test #1)
                      // water flow rate had to increase to get to 80 W since coil off capacity was much different at -1752 W
    EXPECT_NEAR(0.000219, state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(FanCoilNum).HeatCoilFluidInletNode).MassFlowRate, 0.000001);
    // expect inlet and outlet node air mass flow rates are equal
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate,
              state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirOutNode).MassFlowRate);
}

TEST_F(EnergyPlusFixture, ElectricCoilFanCoilHeatingTest)
{

    int FanCoilNum(1);
    int ZoneNum(1);
    bool FirstHVACIteration(true);
    bool ErrorsFound(false);
    Real64 QZnReq(0.0);
    Real64 HotWaterMassFlowRate(0.0);
    Real64 ColdWaterMassFlowRate(0.0);
    Real64 QUnitOut(0.0);
    Real64 AirMassFlow(0.0);
    Real64 MaxAirMassFlow(0.0);
    Real64 LatOutputProvided(0.0);

    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataWaterCoils->GetWaterCoilsInputFlag = true;
    state->dataGlobalNames->NumCoils = 0;
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;

    std::string const idf_objects = delimited_string({
        "	Zone,",
        "	EAST ZONE, !- Name",
        "	0, !- Direction of Relative North { deg }",
        "	0, !- X Origin { m }",
        "	0, !- Y Origin { m }",
        "	0, !- Z Origin { m }",
        "	1, !- Type",
        "	1, !- Multiplier",
        "	autocalculate, !- Ceiling Height { m }",
        "	autocalculate; !- Volume { m3 }",
        "	ZoneHVAC:EquipmentConnections,",
        "	EAST ZONE, !- Zone Name",
        "	Zone1Equipment, !- Zone Conditioning Equipment List Name",
        "	Zone1Inlets, !- Zone Air Inlet Node or NodeList Name",
        "	Zone1Exhausts, !- Zone Air Exhaust Node or NodeList Name",
        "	Zone 1 Node, !- Zone Air Node Name",
        "	Zone 1 Outlet Node;      !- Zone Return Air Node Name",
        "	ZoneHVAC:EquipmentList,",
        "	Zone1Equipment, !- Name",
        "   SequentialLoad,          !- Load Distribution Scheme",
        "	ZoneHVAC:FourPipeFanCoil, !- Zone Equipment 1 Object Type",
        "	Zone1FanCoil, !- Zone Equipment 1 Name",
        "	1, !- Zone Equipment 1 Cooling Sequence",
        "	1;                       !- Zone Equipment 1 Heating or No - Load Sequence",
        "   NodeList,",
        "	Zone1Inlets, !- Name",
        "	Zone1FanCoilAirOutletNode;  !- Node 1 Name",
        "	NodeList,",
        "	Zone1Exhausts, !- Name",
        "	Zone1FanCoilAirInletNode; !- Node 1 Name",
        "	OutdoorAir:NodeList,",
        "	Zone1FanCoilOAInNode;    !- Node or NodeList Name 1",
        "	OutdoorAir:Mixer,",
        "	Zone1FanCoilOAMixer, !- Name",
        "	Zone1FanCoilOAMixerOutletNode, !- Mixed Air Node Name",
        "	Zone1FanCoilOAInNode, !- Outdoor Air Stream Node Name",
        "	Zone1FanCoilExhNode, !- Relief Air Stream Node Name",
        "	Zone1FanCoilAirInletNode; !- Return Air Stream Node Name",
        "	Schedule:Constant,",
        "	FanAndCoilAvailSched, !- Name",
        "	FRACTION, !- Schedule Type",
        "	1;        !- TimeStep Value",
        "	ScheduleTypeLimits,",
        "	Fraction, !- Name",
        "	0.0, !- Lower Limit Value",
        "	1.0, !- Upper Limit Value",
        "	CONTINUOUS;              !- Numeric Type",
        "	Fan:OnOff,",
        "	Zone1FanCoilFan, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Name",
        "	0.5, !- Fan Total Efficiency",
        "	75.0, !- Pressure Rise { Pa }",
        "	0.6, !- Maximum Flow Rate { m3 / s }",
        "	0.9, !- Motor Efficiency",
        "	1.0, !- Motor In Airstream Fraction",
        "	Zone1FanCoilOAMixerOutletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilFanOutletNode, !- Air Outlet Node Name",
        "	, !- Fan Power Ratio Function of Speed Ratio Curve Name",
        "	;                        !- Fan Efficiency Ratio Function of Speed Ratio Curve Name	",
        "	Coil:Cooling:Water,",
        "	Zone1FanCoilCoolingCoil, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Namev",
        "	0.0002, !- Design Water Flow Rate { m3 / s }",
        "	0.5000, !- Design Air Flow Rate { m3 / s }",
        "	7.22,   !- Design Inlet Water Temperature { Cv }",
        "	24.340, !- Design Inlet Air Temperature { C }",
        "	14.000, !- Design Outlet Air Temperature { C }",
        "	0.0095, !- Design Inlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	0.0090, !- Design Outlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	Zone1FanCoilChWInletNode, !- Water Inlet Node Name",
        "	Zone1FanCoilChWOutletNode, !- Water Outlet Node Name",
        "	Zone1FanCoilFanOutletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilCCOutletNode, !- Air Outlet Node Name",
        "	SimpleAnalysis, !- Type of Analysis",
        "	CrossFlow;               !- Heat Exchanger Configuration",
        "	Coil:Heating:Electric,",
        "   Zone1FanCoilHeatingCoil, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Name",
        "	0.9,   !- Efficiency",
        "	4500., !- Nominal Capacity",
        "	Zone1FanCoilCCOutletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilAirOutletNode; !- Air Outlet Node Name",
        "	ZoneHVAC:FourPipeFanCoil,",
        "	Zone1FanCoil, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Name",
        "	ConstantFanVariableFlow, !- Capacity Control Method",
        "	0.5, !- Maximum Supply Air Flow Rate { m3 / s }",
        "	0.3, !- Low Speed Supply Air Flow Ratio",
        "	0.6, !- Medium Speed Supply Air Flow Ratio",
        "	0.0, !- Maximum Outdoor Air Flow Rate { m3 / s }",
        "	FanAndCoilAvailSched, !- Outdoor Air Schedule Name",
        "	Zone1FanCoilAirInletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilAirOutletNode, !- Air Outlet Node Name",
        "	OutdoorAir:Mixer, !- Outdoor Air Mixer Object Type",
        "	Zone1FanCoilOAMixer, !- Outdoor Air Mixer Name",
        "	Fan:OnOff, !- Supply Air Fan Object Type",
        "	Zone1FanCoilFan, !- Supply Air Fan Name",
        "	Coil:Cooling:Water, !- Cooling Coil Object Type",
        "	Zone1FanCoilCoolingCoil, !- Cooling Coil Name",
        "	0.00014, !- Maximum Cold Water Flow Rate { m3 / s }",
        "	0.0, !- Minimum Cold Water Flow Rate { m3 / s }",
        "	0.001, !- Cooling Convergence Tolerance",
        "	Coil:Heating:Electric, !- Heating Coil Object Type",
        "	Zone1FanCoilHeatingCoil, !- Heating Coil Name",
        "	0.00014, !- Maximum Hot Water Flow Rate { m3 / s }",
        "	0.0, !- Minimum Hot Water Flow Rate { m3 / s }",
        "	0.001; !- Heating Convergence Tolerance",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    EXPECT_EQ("EAST ZONE", state->dataHeatBal->Zone(1).Name);

    GetZoneEquipmentData(*state);
    GetFanInput(*state);
    EXPECT_EQ((int)HVAC::FanType::OnOff, (int)state->dataFans->fans(1)->type);

    GetFanCoilUnits(*state);
    EXPECT_ENUM_EQ(CCM::ConsFanVarFlow, state->dataFanCoilUnits->FanCoil(1).CapCtrlMeth_Num);
    EXPECT_EQ("OUTDOORAIR:MIXER", state->dataFanCoilUnits->FanCoil(1).OAMixType);
    EXPECT_EQ((int)HVAC::FanType::OnOff, (int)state->dataFanCoilUnits->FanCoil(1).fanType);
    EXPECT_EQ("COIL:COOLING:WATER", state->dataFanCoilUnits->FanCoil(1).CCoilType);
    EXPECT_EQ("COIL:HEATING:ELECTRIC", state->dataFanCoilUnits->FanCoil(1).HCoilType);

    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

    AirMassFlow = 0.60;
    MaxAirMassFlow = 0.60;
    // heating load only
    ColdWaterMassFlowRate = 0.0;
    HotWaterMassFlowRate = 0.14;

    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).MassFlowRateMax = MaxAirMassFlow;

    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Temp = 22.0;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Enthalpy = 36000;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).HumRat =
        PsyWFnTdbH(*state,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Temp,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Enthalpy);

    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Temp = 10.0;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Enthalpy = 18000;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).HumRat =
        PsyWFnTdbH(*state,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Temp,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Enthalpy);

    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMinAvail = AirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMax = MaxAirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMaxAvail = MaxAirMassFlow;

    state->dataFanCoilUnits->FanCoil(1).OutAirMassFlow = 0.0;
    state->dataFanCoilUnits->FanCoil(1).MaxAirMassFlow = MaxAirMassFlow;
    state->dataFanCoilUnits->FanCoil(1).MaxCoolCoilFluidFlow = 0.14;
    state->dataFanCoilUnits->FanCoil(1).MaxHeatCoilFluidFlow = 0.14;

    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).OutsideAirNode).MassFlowRateMax = 0.0;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).CoolCoilFluidInletNode).MassFlowRateMax = 0.14;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).CoolCoilFluidInletNode).MassFlowRateMaxAvail = 0.14;

    auto *fan1 = state->dataFans->fans(1);
    fan1->inletAirMassFlowRate = AirMassFlow;
    fan1->maxAirMassFlowRate = MaxAirMassFlow;

    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRateMax = AirMassFlow;
    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

    state->dataWaterCoils->WaterCoil(1).UACoilTotal = 470.0;
    state->dataWaterCoils->WaterCoil(1).UACoilExternal = 611.0;
    state->dataWaterCoils->WaterCoil(1).UACoilInternal = 2010.0;
    state->dataWaterCoils->WaterCoil(1).TotCoilOutsideSurfArea = 50.0;

    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).AirInletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).AirInletNodeNum).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).AirInletNodeNum).MassFlowRateMax = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).AirInletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

    state->dataWaterCoils->WaterCoil(1).InletWaterMassFlowRate = ColdWaterMassFlowRate;
    state->dataWaterCoils->WaterCoil(1).MaxWaterMassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum).MassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum).MassFlowRateMaxAvail = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum).Temp = 6.0;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum).MassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum).MassFlowRateMaxAvail = ColdWaterMassFlowRate;

    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).AirInletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).AirInletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }

    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;

    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.loopNum = 1;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.branchNum = 1;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataWaterCoils->WaterCoil(1).WaterPlantLoc);

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
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).FlowLock = DataPlant::FlowLock::Unlocked;

    state->dataFanCoilUnits->FanCoil(1).CoolCoilPlantLoc.loopNum = 1;
    state->dataFanCoilUnits->FanCoil(1).HeatCoilPlantLoc.loopNum = 0;
    state->dataFanCoilUnits->FanCoil(1).CoolCoilPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    state->dataFanCoilUnits->FanCoil(1).HeatCoilPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Invalid;
    state->dataFanCoilUnits->FanCoil(1).HeatCoilFluidOutletNodeNum = 0;
    state->dataFanCoilUnits->FanCoil(1).CoolCoilFluidOutletNodeNum = state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum;
    state->dataFanCoilUnits->FanCoil(1).CoolCoilPlantLoc.branchNum = 1;
    state->dataFanCoilUnits->FanCoil(1).CoolCoilPlantLoc.compNum = 1;
    state->dataFanCoilUnits->FanCoil(1).HeatCoilPlantLoc.branchNum = 0;
    state->dataFanCoilUnits->FanCoil(1).HeatCoilPlantLoc.compNum = 0;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataFanCoilUnits->FanCoil(1).CoolCoilPlantLoc);
    PlantUtilities::SetPlantLocationLinks(*state, state->dataFanCoilUnits->FanCoil(1).HeatCoilPlantLoc);

    state->dataFanCoilUnits->CoolingLoad = false;
    state->dataFanCoilUnits->HeatingLoad = true;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = 8000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 4000.0;
    state->dataFanCoilUnits->FanCoil(1).SpeedFanSel = 2;
    QUnitOut = 0.0;
    QZnReq = 4000.0;

    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(2);
    state->dataWaterCoils->MyUAAndFlowCalcFlag(1) = true;
    state->dataWaterCoils->MyUAAndFlowCalcFlag(2) = true;
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

    // Normal heating simulation for fan coil with constant fan, electric heating
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, LatOutputProvided);
    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    // expect inlet and outlet node air mass flow rates are equal
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate,
              state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirOutNode).MassFlowRate);
    // normal heating, heating capacity exceeded
    QZnReq = 5000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 5000.00;
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, LatOutputProvided);
    EXPECT_NEAR(4575.0, QUnitOut, 5.0);
    // expect inlet and outlet node air mass flow rates are equal
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate,
              state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirOutNode).MassFlowRate);

    state->dataGlobal->DoingSizing = false;
    state->dataPlnt->PlantLoop.deallocate();
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.deallocate();
    state->dataFanCoilUnits->FanCoil.deallocate();
    state->dataLoopNodes->Node.deallocate();
    state->dataWaterCoils->WaterCoil.deallocate();
    state->dataZoneEquip->ZoneEquipConfig.deallocate();
    state->dataHeatBal->Zone.deallocate();
}

TEST_F(EnergyPlusFixture, ConstantFanVariableFlowFanCoilCoolingTest)
{

    int FanCoilNum(1);
    int ZoneNum(1);
    bool FirstHVACIteration(true);
    bool ErrorsFound(false);
    Real64 QZnReq(0.0);
    Real64 HotWaterMassFlowRate(0.0);
    Real64 ColdWaterMassFlowRate(0.0);
    Real64 QUnitOut(0.0);
    Real64 AirMassFlow(0.0);
    Real64 MaxAirMassFlow(0.0);
    Real64 LatOutputProvided(0.0);

    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataWaterCoils->GetWaterCoilsInputFlag = true;
    state->dataGlobalNames->NumCoils = 0;
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;

    std::string const idf_objects = delimited_string({
        "	Zone,",
        "	EAST ZONE, !- Name",
        "	0, !- Direction of Relative North { deg }",
        "	0, !- X Origin { m }",
        "	0, !- Y Origin { m }",
        "	0, !- Z Origin { m }",
        "	1, !- Type",
        "	1, !- Multiplier",
        "	autocalculate, !- Ceiling Height { m }",
        "	autocalculate; !- Volume { m3 }",
        "	ZoneHVAC:EquipmentConnections,",
        "	EAST ZONE, !- Zone Name",
        "	Zone1Equipment, !- Zone Conditioning Equipment List Name",
        "	Zone1Inlets, !- Zone Air Inlet Node or NodeList Name",
        "	Zone1Exhausts, !- Zone Air Exhaust Node or NodeList Name",
        "	Zone 1 Node, !- Zone Air Node Name",
        "	Zone 1 Outlet Node;      !- Zone Return Air Node Name",
        "	ZoneHVAC:EquipmentList,",
        "	Zone1Equipment, !- Name",
        "   SequentialLoad,          !- Load Distribution Scheme",
        "	ZoneHVAC:FourPipeFanCoil, !- Zone Equipment 1 Object Type",
        "	Zone1FanCoil, !- Zone Equipment 1 Name",
        "	1, !- Zone Equipment 1 Cooling Sequence",
        "	1;                       !- Zone Equipment 1 Heating or No - Load Sequence",
        "   NodeList,",
        "	Zone1Inlets, !- Name",
        "	Zone1FanCoilAirOutletNode;  !- Node 1 Name",
        "	NodeList,",
        "	Zone1Exhausts, !- Name",
        "	Zone1FanCoilAirInletNode; !- Node 1 Name",
        "	OutdoorAir:NodeList,",
        "	Zone1FanCoilOAInNode;    !- Node or NodeList Name 1",
        "	OutdoorAir:Mixer,",
        "	Zone1FanCoilOAMixer, !- Name",
        "	Zone1FanCoilOAMixerOutletNode, !- Mixed Air Node Name",
        "	Zone1FanCoilOAInNode, !- Outdoor Air Stream Node Name",
        "	Zone1FanCoilExhNode, !- Relief Air Stream Node Name",
        "	Zone1FanCoilAirInletNode; !- Return Air Stream Node Name",
        "	Schedule:Compact,",
        "	FanAndCoilAvailSched, !- Name",
        "	Fraction, !- Schedule Type Limits Name",
        "	Through: 12/31, !- Field 1",
        "	For: AllDays, !- Field 2",
        "	Until: 24:00, 1.0;        !- Field 3",
        "	ScheduleTypeLimits,",
        "	Fraction, !- Name",
        "	0.0, !- Lower Limit Value",
        "	1.0, !- Upper Limit Value",
        "	CONTINUOUS;              !- Numeric Type",
        "   Fan:OnOff,",
        "	Zone1FanCoilFan, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Name",
        "	0.5, !- Fan Total Efficiency",
        "	75.0, !- Pressure Rise { Pa }",
        "	0.6, !- Maximum Flow Rate { m3 / s }",
        "	0.9, !- Motor Efficiency",
        "	1.0, !- Motor In Airstream Fraction",
        "	Zone1FanCoilOAMixerOutletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilFanOutletNode, !- Air Outlet Node Name",
        "	, !- Fan Power Ratio Function of Speed Ratio Curve Name",
        "	;                        !- Fan Efficiency Ratio Function of Speed Ratio Curve Name	",
        "	Coil:Cooling:Water,",
        "	Zone1FanCoilCoolingCoil, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Namev",
        "	0.0002, !- Design Water Flow Rate { m3 / s }",
        "	0.5000, !- Design Air Flow Rate { m3 / s }",
        "	7.22,   !- Design Inlet Water Temperature { Cv }",
        "	24.340, !- Design Inlet Air Temperature { C }",
        "	14.000, !- Design Outlet Air Temperature { C }",
        "	0.0095, !- Design Inlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	0.0090, !- Design Outlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	Zone1FanCoilChWInletNode, !- Water Inlet Node Name",
        "	Zone1FanCoilChWOutletNode, !- Water Outlet Node Name",
        "	Zone1FanCoilFanOutletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilCCOutletNode, !- Air Outlet Node Name",
        "	SimpleAnalysis, !- Type of Analysis",
        "	CrossFlow;               !- Heat Exchanger Configuration",
        "	Coil:Heating:Water,",
        "   Zone1FanCoilHeatingCoil, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Name",
        "	150.0,   !- U - Factor Times Area Value { W / K }",
        "	0.00014, !- Maximum Water Flow Rate { m3 / s }",
        "	Zone1FanCoilHWInletNode, !- Water Inlet Node Name",
        "	Zone1FanCoilHWOutletNode, !- Water Outlet Node Name",
        "	Zone1FanCoilCCOutletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilAirOutletNode, !- Air Outlet Node Name",
        "	UFactorTimesAreaAndDesignWaterFlowRate, !- Performance Input Method",
        "	autosize, !- Rated Capacity { W }",
        "	82.2, !- Rated Inlet Water Temperature { C }",
        "	16.6, !- Rated Inlet Air Temperature { C }",
        "	71.1, !- Rated Outlet Water Temperature { C }",
        "	32.2, !- Rated Outlet Air Temperature { C }",
        "	;     !- Rated Ratio for Air and Water Convection",
        "	ZoneHVAC:FourPipeFanCoil,",
        "	Zone1FanCoil, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Name",
        "	ConstantFanVariableFlow, !- Capacity Control Method",
        "	0.5, !- Maximum Supply Air Flow Rate { m3 / s }",
        "	0.3, !- Low Speed Supply Air Flow Ratio",
        "	0.6, !- Medium Speed Supply Air Flow Ratio",
        "	0.1, !- Maximum Outdoor Air Flow Rate { m3 / s }",
        "	FanAndCoilAvailSched, !- Outdoor Air Schedule Name",
        "	Zone1FanCoilAirInletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilAirOutletNode, !- Air Outlet Node Name",
        "	OutdoorAir:Mixer, !- Outdoor Air Mixer Object Type",
        "	Zone1FanCoilOAMixer, !- Outdoor Air Mixer Name",
        "	Fan:OnOff, !- Supply Air Fan Object Type",
        "	Zone1FanCoilFan, !- Supply Air Fan Name",
        "	Coil:Cooling:Water, !- Cooling Coil Object Type",
        "	Zone1FanCoilCoolingCoil, !- Cooling Coil Name",
        "	0.00014, !- Maximum Cold Water Flow Rate { m3 / s }",
        "	0.0, !- Minimum Cold Water Flow Rate { m3 / s }",
        "	0.001, !- Cooling Convergence Tolerance",
        "	Coil:Heating:Water, !- Heating Coil Object Type",
        "	Zone1FanCoilHeatingCoil, !- Heating Coil Name",
        "	0.00014, !- Maximum Hot Water Flow Rate { m3 / s }",
        "	0.0, !- Minimum Hot Water Flow Rate { m3 / s }",
        "	0.001; !- Heating Convergence Tolerance",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    EXPECT_EQ("EAST ZONE", state->dataHeatBal->Zone(1).Name);

    GetZoneEquipmentData(*state);
    GetFanInput(*state);
    EXPECT_EQ((int)HVAC::FanType::OnOff, (int)state->dataFans->fans(1)->type);

    GetFanCoilUnits(*state);
    EXPECT_ENUM_EQ(CCM::ConsFanVarFlow, state->dataFanCoilUnits->FanCoil(1).CapCtrlMeth_Num);
    EXPECT_EQ("OUTDOORAIR:MIXER", state->dataFanCoilUnits->FanCoil(1).OAMixType);
    EXPECT_EQ((int)HVAC::FanType::OnOff, (int)state->dataFanCoilUnits->FanCoil(1).fanType);
    EXPECT_EQ("COIL:COOLING:WATER", state->dataFanCoilUnits->FanCoil(1).CCoilType);
    EXPECT_EQ("COIL:HEATING:WATER", state->dataFanCoilUnits->FanCoil(1).HCoilType);

    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

    AirMassFlow = 0.60;
    MaxAirMassFlow = 0.60;

    // cooling load only
    HotWaterMassFlowRate = 0.0;
    ColdWaterMassFlowRate = 0.14;

    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).MassFlowRateMax = MaxAirMassFlow;

    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Temp = 24.0;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Enthalpy = 36000;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).HumRat =
        PsyWFnTdbH(*state,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Temp,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Enthalpy);

    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Temp = 30.0;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Enthalpy = 53000;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).HumRat =
        PsyWFnTdbH(*state,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Temp,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Enthalpy);

    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMinAvail = AirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMax = MaxAirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMaxAvail = MaxAirMassFlow;

    state->dataFanCoilUnits->FanCoil(1).OutAirMassFlow = 0.0;
    state->dataFanCoilUnits->FanCoil(1).MaxAirMassFlow = MaxAirMassFlow;
    state->dataFanCoilUnits->FanCoil(1).MaxCoolCoilFluidFlow = 0.14;
    state->dataFanCoilUnits->FanCoil(1).MaxHeatCoilFluidFlow = 0.14;

    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).OutsideAirNode).MassFlowRateMax = 0.0;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).CoolCoilFluidInletNode).MassFlowRateMax = 0.14;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).HeatCoilFluidInletNode).MassFlowRateMax = 0.14;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).CoolCoilFluidInletNode).MassFlowRateMaxAvail = 0.14;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).HeatCoilFluidInletNode).MassFlowRateMaxAvail = 0.14;

    auto *fan1 = state->dataFans->fans(1);
    fan1->inletAirMassFlowRate = AirMassFlow;
    fan1->maxAirMassFlowRate = MaxAirMassFlow;

    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRateMax = AirMassFlow;
    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

    state->dataWaterCoils->WaterCoil(2).UACoilTotal = 470.0;
    state->dataWaterCoils->WaterCoil(2).UACoilExternal = 611.0;
    state->dataWaterCoils->WaterCoil(2).UACoilInternal = 2010.0;
    state->dataWaterCoils->WaterCoil(2).TotCoilOutsideSurfArea = 50.0;

    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).AirInletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).AirInletNodeNum).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).AirInletNodeNum).MassFlowRateMax = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).AirInletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

    state->dataWaterCoils->WaterCoil(2).InletWaterMassFlowRate = ColdWaterMassFlowRate;
    state->dataWaterCoils->WaterCoil(2).MaxWaterMassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterInletNodeNum).MassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterInletNodeNum).MassFlowRateMaxAvail = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterInletNodeNum).Temp = 6.0;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterOutletNodeNum).MassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterOutletNodeNum).MassFlowRateMaxAvail = ColdWaterMassFlowRate;

    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).AirInletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).AirInletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum).Temp = 60.0;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum).MassFlowRate = HotWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum).MassFlowRateMaxAvail = HotWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum).MassFlowRate = HotWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum).MassFlowRateMaxAvail = HotWaterMassFlowRate;
    state->dataWaterCoils->WaterCoil(1).InletWaterMassFlowRate = HotWaterMassFlowRate;
    state->dataWaterCoils->WaterCoil(1).MaxWaterMassFlowRate = HotWaterMassFlowRate;

    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }

    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;

    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.loopNum = 1;
    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.branchNum = 1;
    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataWaterCoils->WaterCoil(2).WaterPlantLoc);

    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.loopNum = 2;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.branchNum = 1;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataWaterCoils->WaterCoil(1).WaterPlantLoc);

    state->dataPlnt->PlantLoop(2).Name = "ChilledWaterLoop";
    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = state->dataWaterCoils->WaterCoil(2).Name;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilWaterCooling;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterCoils->WaterCoil(2).WaterInletNodeNum;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut =
        state->dataWaterCoils->WaterCoil(2).WaterOutletNodeNum;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).FlowLock = DataPlant::FlowLock::Unlocked;

    state->dataPlnt->PlantLoop(1).Name = "HotWaterLoop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = state->dataWaterCoils->WaterCoil(1).Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut =
        state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).FlowLock = DataPlant::FlowLock::Unlocked;

    state->dataFanCoilUnits->FanCoil(1).CoolCoilPlantLoc.loopNum = 2;
    state->dataFanCoilUnits->FanCoil(1).HeatCoilPlantLoc.loopNum = 1;
    state->dataFanCoilUnits->FanCoil(1).CoolCoilPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    state->dataFanCoilUnits->FanCoil(1).HeatCoilPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    state->dataFanCoilUnits->FanCoil(1).HeatCoilFluidOutletNodeNum = state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum;
    state->dataFanCoilUnits->FanCoil(1).CoolCoilFluidOutletNodeNum = state->dataWaterCoils->WaterCoil(2).WaterOutletNodeNum;
    state->dataFanCoilUnits->FanCoil(1).CoolCoilPlantLoc.branchNum = 1;
    state->dataFanCoilUnits->FanCoil(1).CoolCoilPlantLoc.compNum = 1;
    state->dataFanCoilUnits->FanCoil(1).HeatCoilPlantLoc.branchNum = 1;
    state->dataFanCoilUnits->FanCoil(1).HeatCoilPlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataFanCoilUnits->FanCoil(1).CoolCoilPlantLoc);
    PlantUtilities::SetPlantLocationLinks(*state, state->dataFanCoilUnits->FanCoil(1).HeatCoilPlantLoc);

    state->dataFanCoilUnits->HeatingLoad = false;
    state->dataFanCoilUnits->CoolingLoad = true;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -4000.00;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = -8000.0;
    state->dataFanCoilUnits->FanCoil(1).SpeedFanSel = 2;
    QUnitOut = 0.0;
    QZnReq = -4000.0;

    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(2);
    state->dataWaterCoils->MyUAAndFlowCalcFlag(1) = true;
    state->dataWaterCoils->MyUAAndFlowCalcFlag(2) = true;
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
    // normal cooling simulation for constant fan variable flow fan coil
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, LatOutputProvided);
    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    // expect inlet and outlet node air mass flow rates are equal
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate,
              state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirOutNode).MassFlowRate);

    FirstHVACIteration = false;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).FlowLock = DataPlant::FlowLock::Locked;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).CoolCoilFluidInletNode).MassFlowRate = 0.2;
    // cooling simulation with flow lock on and locked flow > flow that meets load; bypass extra flow
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, LatOutputProvided);
    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    EXPECT_NEAR(10.86, state->dataLoopNodes->Node(13).Temp, 0.1);
    // expect inlet and outlet node air mass flow rates are equal
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate,
              state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirOutNode).MassFlowRate);

    // cooling simulation with flow lock on and locked flow < flow required for load; use locked flow
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).CoolCoilFluidInletNode).MassFlowRate = 0.05;
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, LatOutputProvided);
    EXPECT_NEAR(-3000.0, QUnitOut, 5.0);
    // expect inlet and outlet node air mass flow rates are equal
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate,
              state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirOutNode).MassFlowRate);

    // normal cooling, no flow lock, cooling capacity exceeded
    QZnReq = -5000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -5000.00;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).FlowLock = DataPlant::FlowLock::Unlocked;
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, LatOutputProvided);
    EXPECT_NEAR(-4420.0, QUnitOut, 5.0);
    // expect inlet and outlet node air mass flow rates are equal
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate,
              state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirOutNode).MassFlowRate);
}

TEST_F(EnergyPlusFixture, FanCoil_ASHRAE90VariableFan)
{

    int FanCoilNum(1);
    int ZoneNum(1);
    bool FirstHVACIteration(false);
    bool ErrorsFound(false);
    Real64 QZnReq(0.0);
    Real64 HotWaterMassFlowRate(0.0);
    Real64 ColdWaterMassFlowRate(0.0);
    Real64 QUnitOut(0.0);
    Real64 QLatOut(0.0);
    Real64 AirMassFlow(0.0);
    Real64 MaxAirMassFlow(0.0);

    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataWaterCoils->GetWaterCoilsInputFlag = true;
    state->dataGlobalNames->NumCoils = 0;
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->dataSize->CurZoneEqNum = 1;

    std::string const idf_objects = delimited_string({
        "	Zone,",
        "	EAST ZONE, !- Name",
        "	0, !- Direction of Relative North { deg }",
        "	0, !- X Origin { m }",
        "	0, !- Y Origin { m }",
        "	0, !- Z Origin { m }",
        "	1, !- Type",
        "	1, !- Multiplier",
        "	autocalculate, !- Ceiling Height { m }",
        "	autocalculate; !- Volume { m3 }",
        "	ZoneHVAC:EquipmentConnections,",
        "	EAST ZONE, !- Zone Name",
        "	Zone1Equipment, !- Zone Conditioning Equipment List Name",
        "	Zone1Inlets, !- Zone Air Inlet Node or NodeList Name",
        "	Zone1Exhausts, !- Zone Air Exhaust Node or NodeList Name",
        "	Zone 1 Node, !- Zone Air Node Name",
        "	Zone 1 Outlet Node;      !- Zone Return Air Node Name",
        "	ZoneHVAC:EquipmentList,",
        "	Zone1Equipment, !- Name",
        "   SequentialLoad,          !- Load Distribution Scheme",
        "	ZoneHVAC:FourPipeFanCoil, !- Zone Equipment 1 Object Type",
        "	Zone1FanCoil, !- Zone Equipment 1 Name",
        "	1, !- Zone Equipment 1 Cooling Sequence",
        "	1;                       !- Zone Equipment 1 Heating or No - Load Sequence",
        "   NodeList,",
        "	Zone1Inlets, !- Name",
        "	Zone1FanCoilAirOutletNode;  !- Node 1 Name",
        "	NodeList,",
        "	Zone1Exhausts, !- Name",
        "	Zone1FanCoilAirInletNode; !- Node 1 Name",
        "	OutdoorAir:NodeList,",
        "	Zone1FanCoilOAInNode;    !- Node or NodeList Name 1",
        "	OutdoorAir:Mixer,",
        "	Zone1FanCoilOAMixer, !- Name",
        "	Zone1FanCoilOAMixerOutletNode, !- Mixed Air Node Name",
        "	Zone1FanCoilOAInNode, !- Outdoor Air Stream Node Name",
        "	Zone1FanCoilExhNode, !- Relief Air Stream Node Name",
        "	Zone1FanCoilAirInletNode; !- Return Air Stream Node Name",
        "	Schedule:Constant,",
        "	FanAndCoilAvailSched, !- Name",
        "	FRACTION, !- Schedule Type",
        "	1;        !- TimeStep Value",
        "	ScheduleTypeLimits,",
        "	Fraction, !- Name",
        "	0.0, !- Lower Limit Value",
        "	1.0, !- Upper Limit Value",
        "	CONTINUOUS;              !- Numeric Type",
        "   Fan:OnOff,",
        "	Zone1FanCoilFan, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Name",
        "	0.5, !- Fan Total Efficiency",
        "	75.0, !- Pressure Rise { Pa }",
        "	0.6, !- Maximum Flow Rate { m3 / s }",
        "	0.9, !- Motor Efficiency",
        "	1.0, !- Motor In Airstream Fraction",
        "	Zone1FanCoilOAMixerOutletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilFanOutletNode, !- Air Outlet Node Name",
        "	, !- Fan Power Ratio Function of Speed Ratio Curve Name",
        "	;                        !- Fan Efficiency Ratio Function of Speed Ratio Curve Name	",
        "	Coil:Cooling:Water,",
        "	Zone1FanCoilCoolingCoil, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Namev",
        "	0.0002, !- Design Water Flow Rate { m3 / s }",
        "	0.5000, !- Design Air Flow Rate { m3 / s }",
        "	7.22,   !- Design Inlet Water Temperature { Cv }",
        "	24.340, !- Design Inlet Air Temperature { C }",
        "	14.000, !- Design Outlet Air Temperature { C }",
        "	0.0095, !- Design Inlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	0.0090, !- Design Outlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	Zone1FanCoilChWInletNode, !- Water Inlet Node Name",
        "	Zone1FanCoilChWOutletNode, !- Water Outlet Node Name",
        "	Zone1FanCoilFanOutletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilCCOutletNode, !- Air Outlet Node Name",
        "	SimpleAnalysis, !- Type of Analysis",
        "	CrossFlow;               !- Heat Exchanger Configuration",
        "	Coil:Heating:Water,",
        "   Zone1FanCoilHeatingCoil, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Name",
        "	150.0,   !- U - Factor Times Area Value { W / K }",
        "	0.00014, !- Maximum Water Flow Rate { m3 / s }",
        "	Zone1FanCoilHWInletNode, !- Water Inlet Node Name",
        "	Zone1FanCoilHWOutletNode, !- Water Outlet Node Name",
        "	Zone1FanCoilCCOutletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilAirOutletNode, !- Air Outlet Node Name",
        "	UFactorTimesAreaAndDesignWaterFlowRate, !- Performance Input Method",
        "	autosize, !- Rated Capacity { W }",
        "	82.2, !- Rated Inlet Water Temperature { C }",
        "	16.6, !- Rated Inlet Air Temperature { C }",
        "	71.1, !- Rated Outlet Water Temperature { C }",
        "	32.2, !- Rated Outlet Air Temperature { C }",
        "	;     !- Rated Ratio for Air and Water Convection",
        "	ZoneHVAC:FourPipeFanCoil,",
        "	Zone1FanCoil, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Name",
        "	ASHRAE90VariableFan, !- Capacity Control Method",
        "	0.5, !- Maximum Supply Air Flow Rate { m3 / s }",
        "	0.3, !- Low Speed Supply Air Flow Ratio",
        "	0.6, !- Medium Speed Supply Air Flow Ratio",
        "	0.0, !- Maximum Outdoor Air Flow Rate { m3 / s }",
        "	FanAndCoilAvailSched, !- Outdoor Air Schedule Name",
        "	Zone1FanCoilAirInletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilAirOutletNode, !- Air Outlet Node Name",
        "	OutdoorAir:Mixer, !- Outdoor Air Mixer Object Type",
        "	Zone1FanCoilOAMixer, !- Outdoor Air Mixer Name",
        "	Fan:OnOff, !- Supply Air Fan Object Type",
        "	Zone1FanCoilFan, !- Supply Air Fan Name",
        "	Coil:Cooling:Water, !- Cooling Coil Object Type",
        "	Zone1FanCoilCoolingCoil, !- Cooling Coil Name",
        "	0.00014, !- Maximum Cold Water Flow Rate { m3 / s }",
        "	0.0, !- Minimum Cold Water Flow Rate { m3 / s }",
        "	0.001, !- Cooling Convergence Tolerance",
        "	Coil:Heating:Water, !- Heating Coil Object Type",
        "	Zone1FanCoilHeatingCoil, !- Heating Coil Name",
        "	0.00014, !- Maximum Hot Water Flow Rate { m3 / s }",
        "	0.0, !- Minimum Hot Water Flow Rate { m3 / s }",
        "	0.001, !- Heating Convergence Tolerance",
        "	, !- Availability Manager List Name",
        "	, !- Design Specification ZoneHVAC Sizing Object Name",
        "	, !- Supply Air Fan Operating Mode Schedule Name",
        "	16.0, !- Minimum Supply Air Temperature in Cooling Mode",
        "	28.0; !- Maximum Supply Air Temperature in Heating Mode",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    EXPECT_EQ("EAST ZONE", state->dataHeatBal->Zone(1).Name);

    GetZoneEquipmentData(*state);
    GetFanInput(*state);
    EXPECT_EQ((int)HVAC::FanType::OnOff, (int)state->dataFans->fans(1)->type);

    GetFanCoilUnits(*state);
    EXPECT_ENUM_EQ(CCM::ASHRAE, state->dataFanCoilUnits->FanCoil(1).CapCtrlMeth_Num);
    EXPECT_EQ("OUTDOORAIR:MIXER", state->dataFanCoilUnits->FanCoil(1).OAMixType);
    EXPECT_EQ((int)HVAC::FanType::OnOff, (int)state->dataFanCoilUnits->FanCoil(1).fanType);
    EXPECT_EQ("COIL:COOLING:WATER", state->dataFanCoilUnits->FanCoil(1).CCoilType);
    EXPECT_EQ("COIL:HEATING:WATER", state->dataFanCoilUnits->FanCoil(1).HCoilType);

    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

    AirMassFlow = 0.60;
    MaxAirMassFlow = 0.60;
    // heating load only
    ColdWaterMassFlowRate = 0.0;
    HotWaterMassFlowRate = 1.0;

    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).MassFlowRateMax = MaxAirMassFlow;

    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Temp = 22.0;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Enthalpy = 36000;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).HumRat =
        PsyWFnTdbH(*state,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Temp,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Enthalpy);

    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Temp = 10.0;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Enthalpy = 18000;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).HumRat =
        PsyWFnTdbH(*state,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Temp,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Enthalpy);

    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMinAvail = AirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMax = MaxAirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMaxAvail = MaxAirMassFlow;

    state->dataFanCoilUnits->FanCoil(1).OutAirMassFlow = 0.0;
    state->dataFanCoilUnits->FanCoil(1).MaxAirMassFlow = MaxAirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).OutsideAirNode).MassFlowRateMax = 0.0;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).NodeNumOfControlledZone).Temp = 22.0;

    auto *fan1 = state->dataFans->fans(1);
    fan1->inletAirMassFlowRate = AirMassFlow;
    fan1->maxAirMassFlowRate = MaxAirMassFlow;

    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRateMax = AirMassFlow;
    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

    state->dataWaterCoils->WaterCoil(2).UACoilTotal = 470.0;
    state->dataWaterCoils->WaterCoil(2).UACoilExternal = 611.0;
    state->dataWaterCoils->WaterCoil(2).UACoilInternal = 2010.0;
    state->dataWaterCoils->WaterCoil(2).TotCoilOutsideSurfArea = 50.0;

    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).AirInletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).AirInletNodeNum).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).AirInletNodeNum).MassFlowRateMax = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).AirInletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

    state->dataWaterCoils->WaterCoil(2).InletWaterMassFlowRate = ColdWaterMassFlowRate;
    state->dataWaterCoils->WaterCoil(2).MaxWaterMassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterInletNodeNum).MassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterInletNodeNum).MassFlowRateMaxAvail = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterInletNodeNum).Temp = 6.0;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterOutletNodeNum).MassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterOutletNodeNum).MassFlowRateMaxAvail = ColdWaterMassFlowRate;

    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).AirInletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).AirInletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum).Temp = 60.0;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum).MassFlowRate = HotWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum).MassFlowRateMaxAvail = HotWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum).MassFlowRate = HotWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum).MassFlowRateMaxAvail = HotWaterMassFlowRate;
    state->dataWaterCoils->WaterCoil(1).InletWaterMassFlowRate = HotWaterMassFlowRate;
    state->dataWaterCoils->WaterCoil(1).MaxWaterMassFlowRate = HotWaterMassFlowRate;

    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }

    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.loopNum = 1;
    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.branchNum = 1;
    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataWaterCoils->WaterCoil(2).WaterPlantLoc);

    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.loopNum = 2;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.branchNum = 1;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataWaterCoils->WaterCoil(1).WaterPlantLoc);

    state->dataPlnt->PlantLoop(2).Name = "ChilledWaterLoop";
    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = state->dataWaterCoils->WaterCoil(2).Name;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilWaterCooling;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterCoils->WaterCoil(2).WaterInletNodeNum;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut =
        state->dataWaterCoils->WaterCoil(2).WaterOutletNodeNum;

    state->dataPlnt->PlantLoop(1).Name = "HotWaterLoop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = state->dataWaterCoils->WaterCoil(1).Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut =
        state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum;

    state->dataFanCoilUnits->CoolingLoad = false;
    state->dataFanCoilUnits->HeatingLoad = true;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = 5000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 4000.0;
    state->dataFanCoilUnits->FanCoil(1).SpeedFanSel = 2;
    QUnitOut = 0.0;
    QLatOut = 0.0;
    QZnReq = 4000.0;

    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(2);
    state->dataWaterCoils->MyUAAndFlowCalcFlag(1) = true;
    state->dataWaterCoils->MyUAAndFlowCalcFlag(2) = true;
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
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod.allocate(HVAC::NumOfSizingTypes);
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod = 0;
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow = 0.5;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow = 0.5;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInTemp = 30.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInHumRat = 0.01;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInTemp = 20.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInHumRat = 0.005;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolLoad = 4000.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatLoad = 4000.0;
    state->dataEnvrn->StdRhoAir = 1.2;

    state->dataGlobal->BeginEnvrnFlag = true;
    InitFanCoilUnits(*state, FanCoilNum, ZoneNum);
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);

    // expect full flow and meet capacity
    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate, state->dataFanCoilUnits->FanCoil(1).MaxAirMassFlow, 0.0000000001);
    // expect inlet and outlet node air mass flow rates are equal
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate,
              state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirOutNode).MassFlowRate);

    // expect minimum flow and meet capacity
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 1000.0;
    QZnReq = 1000.0;
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate,
                state->dataFanCoilUnits->FanCoil(1).MaxAirMassFlow * state->dataFanCoilUnits->FanCoil(1).LowSpeedRatio,
                0.0000000001);
    // expect inlet and outlet node air mass flow rates are equal
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate,
              state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirOutNode).MassFlowRate);

    // expect modulated flow and meet capacity
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 2500.0;
    QZnReq = 2500.0;
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    EXPECT_GT(state->dataLoopNodes->Node(1).MassFlowRate,
              state->dataFanCoilUnits->FanCoil(1).MaxAirMassFlow * state->dataFanCoilUnits->FanCoil(1).LowSpeedRatio);
    EXPECT_LT(state->dataLoopNodes->Node(1).MassFlowRate, state->dataFanCoilUnits->FanCoil(1).MaxAirMassFlow);
    // expect inlet and outlet node air mass flow rates are equal
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate,
              state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirOutNode).MassFlowRate);

    // expect full flow and meet capacity
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = -5000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -4000.0;
    QZnReq = -4000.0;
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate, state->dataFanCoilUnits->FanCoil(1).MaxAirMassFlow, 0.0000000001);
    // expect inlet and outlet node air mass flow rates are equal
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate,
              state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirOutNode).MassFlowRate);

    // expect full flow and meet capacity
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = -5000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -4255.0;
    QZnReq = -4255.0;
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate, state->dataFanCoilUnits->FanCoil(1).MaxAirMassFlow, 0.0000000001);
    // expect inlet and outlet node air mass flow rates are equal
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate,
              state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirOutNode).MassFlowRate);

    // expect minimum flow and meet capacity
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -1000.0;
    QZnReq = -1000.0;
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate,
                state->dataFanCoilUnits->FanCoil(1).MaxAirMassFlow * state->dataFanCoilUnits->FanCoil(1).LowSpeedRatio,
                0.0000000001);
    // expect inlet and outlet node air mass flow rates are equal
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate,
              state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirOutNode).MassFlowRate);

    // expect modulated flow and meet capacity
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -2500.0;
    QZnReq = -2500.0;
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    EXPECT_GT(state->dataLoopNodes->Node(1).MassFlowRate,
              state->dataFanCoilUnits->FanCoil(1).MaxAirMassFlow * state->dataFanCoilUnits->FanCoil(1).LowSpeedRatio);
    EXPECT_LT(state->dataLoopNodes->Node(1).MassFlowRate, state->dataFanCoilUnits->FanCoil(1).MaxAirMassFlow);
    // expect inlet and outlet node air mass flow rates are equal
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate,
              state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirOutNode).MassFlowRate);

    state->dataGlobal->DoingSizing = false;

    state->dataPlnt->PlantLoop.deallocate();
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.deallocate();
    state->dataFanCoilUnits->FanCoil.deallocate();
    state->dataLoopNodes->Node.deallocate();
    state->dataWaterCoils->WaterCoil.deallocate();
    state->dataZoneEquip->ZoneEquipConfig.deallocate();
    state->dataHeatBal->Zone.deallocate();
}

TEST_F(EnergyPlusFixture, Test_TightenWaterFlowLimits)
{
    using General::SolveRoot;

    int FanCoilNum(1);
    bool FirstHVACIteration(false);
    bool ErrorsFound(false);
    Real64 QZnReq(-1000.0);
    state->dataPlnt->TotNumLoops = 2;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataWaterCoils->GetWaterCoilsInputFlag = true;
    state->dataGlobalNames->NumCoils = 0;
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;

    std::string const idf_objects = delimited_string({
        " Zone, EAST ZONE, 0, 0, 0, 0, 1, 1, autocalculate, autocalculate;",
        " ZoneHVAC:EquipmentConnections, EAST ZONE, Zone1Equipment, Zone1Inlets, Zone1Exhausts, Zone 1 Node, Zone 1 Outlet Node;",
        " ZoneHVAC:EquipmentList, Zone1Equipment, SequentialLoad, ZoneHVAC:FourPipeFanCoil, Zone1FanCoil, 1, 1;",
        " OutdoorAir:NodeList, Zone1FCOAIn;",
        " OutdoorAir:Mixer, Zone1FanCoilOAMixer, Zone1OAMixOut, Zone1FCOAIn, Zone1FCExh, Zone1FCAirIn;",
        " Fan:ConstantVolume, Zone1FanCoilFan, FCAvailSch, 0.5, 75.0, 0.6, 0.9, 1.0, Zone1OAMixOut, Zone1FCFanOut;",
        " Schedule:Constant, FCAvailSch, FRACTION, 1;",
        " ScheduleTypeLimits, Fraction, 0.0, 1.0, CONTINUOUS;",
        " NodeList, Zone1Inlets, Zone1FCAirOut;",
        " NodeList, Zone1Exhausts, Zone1FCAirIn;",
        " Coil:Cooling:Water, Zone1FCCoolCoil, FCAvailSch, 0.0002, 0.5, 7.22, 24.34, 14.0, 0.0095, 0.009, Zone1FCChWIn, Zone1FCChWOut, ",
        "Zone1FCFanOut, Zone1FCCCOut, SimpleAnalysis, CrossFlow;",
        " Coil:Heating:Water, Zone1FanCoilHeatingCoil, FCAvailSch, 150.0, 0.00014, Zone1FCHWIn, Zone1FCHWOut, Zone1FCCCOut, Zone1FCAirOut, ",
        "UFactorTimesAreaAndDesignWaterFlowRate, autosize, 82.2, 16.6, 71.1, 32.2, ;",

        " ZoneHVAC:FourPipeFanCoil,",
        "  Zone1FanCoil, !- Name",
        "  FCAvailSch, !- Availability Schedule Name",
        "  MultiSpeedFan, !- Capacity Control Method",
        "  0.5, !- Maximum Supply Air Flow Rate { m3 / s }",
        "  0.3, !- Low Speed Supply Air Flow Ratio",
        "  0.6, !- Medium Speed Supply Air Flow Ratio",
        "  0.0, !- Maximum Outdoor Air Flow Rate { m3 / s }",
        "  FCAvailSch, !- Outdoor Air Schedule Name",
        "  Zone1FCAirIn, !- Air Inlet Node Name",
        "  Zone1FCAirOut, !- Air Outlet Node Name",
        "  OutdoorAir:Mixer, !- Outdoor Air Mixer Object Type",
        "  Zone1FanCoilOAMixer, !- Outdoor Air Mixer Name",
        "  Fan:ConstantVolume, !- Supply Air Fan Object Type",
        "  Zone1FanCoilFan, !- Supply Air Fan Name",
        "  Coil:Cooling:Water, !- Cooling Coil Object Type",
        "  Zone1FCCoolCoil, !- Cooling Coil Name",
        "  0.00014, !- Maximum Cold Water Flow Rate { m3 / s }",
        "  0.0, !- Minimum Cold Water Flow Rate { m3 / s }",
        "  0.001, !- Cooling Convergence Tolerance",
        "  Coil:Heating:Water, !- Heating Coil Object Type",
        "  Zone1FanCoilHeatingCoil, !- Heating Coil Name",
        "  0.00014, !- Maximum Hot Water Flow Rate { m3 / s }",
        "  0.0, !- Minimum Hot Water Flow Rate { m3 / s }",
        "  0.001; !- Heating Convergence Tolerance",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    // OutputProcessor::TimeValue.allocate(2);

    GetZoneData(*state, ErrorsFound);
    GetZoneEquipmentData(*state);
    SetPredefinedTables(*state);
    GetFanInput(*state);
    GetFanCoilUnits(*state);

    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }

    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.loopNum = 1;
    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.branchNum = 1;
    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataWaterCoils->WaterCoil(2).WaterPlantLoc);

    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.loopNum = 2;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.branchNum = 1;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataWaterCoils->WaterCoil(1).WaterPlantLoc);

    state->dataPlnt->PlantLoop(2).Name = "ChilledWaterLoop";
    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = state->dataWaterCoils->WaterCoil(2).Name;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilWaterCooling;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterCoils->WaterCoil(2).WaterInletNodeNum;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut =
        state->dataWaterCoils->WaterCoil(2).WaterOutletNodeNum;

    state->dataPlnt->PlantLoop(1).Name = "HotWaterLoop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = state->dataWaterCoils->WaterCoil(1).Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut =
        state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum;

    bool CoolingLoad = true;
    bool HeatingLoad = false;
    int ControlledZoneNum = 1;
    Real64 MinWaterFlow = 0.0;
    Real64 MaxWaterFlow = 1.5;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(FanCoilNum).AirInNode).Temp = 24.0;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(FanCoilNum).AirInNode).HumRat = 0.00946;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(FanCoilNum).AirInNode).Enthalpy = 48228.946;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(FanCoilNum).AirInNode).MassFlowRate = 0.719999999;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(FanCoilNum).AirInNode).MassFlowRateMax = 0.719999999;
    state->dataLoopNodes->Node(6).MassFlowRateMaxAvail = 0.72;
    state->dataLoopNodes->Node(5).MassFlowRateMaxAvail = 0.72;
    state->dataFanCoilUnits->FanCoil(FanCoilNum).CCoilName_Index = 2;
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataEnvrn->DayOfYear_Schedule = 1;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataGlobal->HourOfDay = 1;
    Sched::UpdateScheduleVals(*state);

    // fan coil can hit maximum iterations while trying to find the water mass flow rate to meet the load. In this case RegulaFalsi will return -1.
    // When this happens, this routine will find tighter limits on min/max water flow rate passed to RegulaFalsi
    // This routine is only called when RegulaFalsi returns -1

    // example usage of function
    // if( SolFlag == -1 ) {
    // tighten limits on water flow rate to see if this allows convergence
    //	CoolingLoad = true;
    //	HeatingLoad = false;
    //	TightenWaterFlowLimits(*state,  FanCoilNum, CoolingLoad, HeatingLoad, FanCoil( FanCoilNum ).CoolCoilFluidInletNode, ControlledZoneNum,
    // FirstHVACIteration,
    // QZnReq, MinWaterFlow, MaxWaterFlow );

    // run once to set up fan coil data
    TightenWaterFlowLimits(*state,
                           FanCoilNum,
                           CoolingLoad,
                           HeatingLoad,
                           state->dataFanCoilUnits->FanCoil(FanCoilNum).CoolCoilFluidInletNode,
                           ControlledZoneNum,
                           FirstHVACIteration,
                           QZnReq,
                           MinWaterFlow,
                           MaxWaterFlow);

    // full output of fan coil is around -7178 W, MaxWaterFlow should remain at 1.5 and MinWaterFlow should be set to 0.15
    MinWaterFlow = 0.0;
    MaxWaterFlow = 1.5;
    QZnReq = -8000.0;
    TightenWaterFlowLimits(*state,
                           FanCoilNum,
                           CoolingLoad,
                           HeatingLoad,
                           state->dataFanCoilUnits->FanCoil(FanCoilNum).CoolCoilFluidInletNode,
                           ControlledZoneNum,
                           FirstHVACIteration,
                           QZnReq,
                           MinWaterFlow,
                           MaxWaterFlow);
    EXPECT_NEAR(MinWaterFlow, 0.15, 0.0000001);
    EXPECT_NEAR(MaxWaterFlow, 1.50, 0.0000001);

    // lower output (using 10% of max water flow rate) of fan coil is around -715 W, MaxWaterFlow should be 10% of 1.5 = 0.15 and MinWaterFlow should
    // be 1% = 0.015
    MinWaterFlow = 0.0;
    MaxWaterFlow = 1.5;
    QZnReq = -800.0;
    TightenWaterFlowLimits(*state,
                           FanCoilNum,
                           CoolingLoad,
                           HeatingLoad,
                           state->dataFanCoilUnits->FanCoil(FanCoilNum).CoolCoilFluidInletNode,
                           ControlledZoneNum,
                           FirstHVACIteration,
                           QZnReq,
                           MinWaterFlow,
                           MaxWaterFlow);
    EXPECT_NEAR(MinWaterFlow, 0.015, 0.0000001);
    EXPECT_NEAR(MaxWaterFlow, 0.150, 0.0000001);

    // lower output of fan coil is around 30 W (fan heat is overtaking cooling output), MaxWaterFlow should be 1% of 1.5 = 0.015 and MinWaterFlow
    // should be 0.1% = 0.0015
    MinWaterFlow = 0.0;
    MaxWaterFlow = 1.5;
    QZnReq = -10.0;
    TightenWaterFlowLimits(*state,
                           FanCoilNum,
                           CoolingLoad,
                           HeatingLoad,
                           state->dataFanCoilUnits->FanCoil(FanCoilNum).CoolCoilFluidInletNode,
                           ControlledZoneNum,
                           FirstHVACIteration,
                           QZnReq,
                           MinWaterFlow,
                           MaxWaterFlow);
    EXPECT_NEAR(MinWaterFlow, 0.0015, 0.0000001);
    EXPECT_NEAR(MaxWaterFlow, 0.0150, 0.0000001);

    // lower output of fan coil is around 105 W, MaxWaterFlow should be 0.1% of 1.5 = 0.0015 and MinWaterFlow should be 0.01% = 0.00015
    MinWaterFlow = 0.0;
    MaxWaterFlow = 1.5;
    QZnReq = 40.0;
    TightenWaterFlowLimits(*state,
                           FanCoilNum,
                           CoolingLoad,
                           HeatingLoad,
                           state->dataFanCoilUnits->FanCoil(FanCoilNum).CoolCoilFluidInletNode,
                           ControlledZoneNum,
                           FirstHVACIteration,
                           QZnReq,
                           MinWaterFlow,
                           MaxWaterFlow);
    EXPECT_NEAR(MinWaterFlow, 0.00015, 0.0000001);
    EXPECT_NEAR(MaxWaterFlow, 0.00150, 0.0000001);

    // lower output of fan coil is around 112 W, MaxWaterFlow should be 0.01% of 1.5 = 0.00015 and MinWaterFlow should be 0.01% = 0.000015
    MinWaterFlow = 0.0;
    MaxWaterFlow = 1.5;
    QZnReq = 110.0;
    TightenWaterFlowLimits(*state,
                           FanCoilNum,
                           CoolingLoad,
                           HeatingLoad,
                           state->dataFanCoilUnits->FanCoil(FanCoilNum).CoolCoilFluidInletNode,
                           ControlledZoneNum,
                           FirstHVACIteration,
                           QZnReq,
                           MinWaterFlow,
                           MaxWaterFlow);
    EXPECT_NEAR(MinWaterFlow, 0.000015, 0.0000001);
    EXPECT_NEAR(MaxWaterFlow, 0.000150, 0.0000001);

    // lower output of fan coil is around 112 W, MaxWaterFlow should be 0.001% of 1.5 = 0.000015 and MinWaterFlow should remain at 0.0
    MinWaterFlow = 0.0;
    MaxWaterFlow = 1.5;
    QZnReq = 120.0;
    TightenWaterFlowLimits(*state,
                           FanCoilNum,
                           CoolingLoad,
                           HeatingLoad,
                           state->dataFanCoilUnits->FanCoil(FanCoilNum).CoolCoilFluidInletNode,
                           ControlledZoneNum,
                           FirstHVACIteration,
                           QZnReq,
                           MinWaterFlow,
                           MaxWaterFlow);
    EXPECT_NEAR(MinWaterFlow, 0.000000, 0.0000001);
    EXPECT_NEAR(MaxWaterFlow, 0.000015, 0.0000001);

    MinWaterFlow = 0.0;
    MaxWaterFlow = 1.5;
    Real64 ErrorToler = 0.00001;
    int MaxIte = 4;
    int SolFla;
    Real64 mdot;

    auto f = [this](Real64 const mdot) {
        Real64 constexpr QZnReq2 = -1000.0;
        int FanCoilNum = 1;
        int ControlledZoneNum = 1;
        bool FirstHVACIteration = false;
        Real64 QUnitOut;
        this->state->dataLoopNodes->Node(12).MassFlowRate = mdot;
        Calc4PipeFanCoil(*this->state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut);
        return (QUnitOut - QZnReq2) / QZnReq2;
    };

    state->dataRootFinder->rootAlgo = RootAlgo::Bisection;
    General::SolveRoot(*state, ErrorToler, MaxIte, SolFla, mdot, f, MinWaterFlow, MaxWaterFlow);
    EXPECT_EQ(-1, SolFla);

    MaxIte = 20;
    MinWaterFlow = 0.0;
    MaxWaterFlow = 0.09375;
    state->dataRootFinder->rootAlgo = RootAlgo::RegulaFalsi;
    General::SolveRoot(*state, ErrorToler, MaxIte, SolFla, mdot, f, MinWaterFlow, MaxWaterFlow);
    EXPECT_EQ(3, SolFla);
}

TEST_F(EnergyPlusFixture, FanCoil_CyclingFanMode)
{

    int FanCoilNum(1);
    int ZoneNum(1);
    bool FirstHVACIteration(false);
    bool ErrorsFound(false);
    Real64 QZnReq(0.0);
    Real64 HotWaterMassFlowRate(0.0);
    Real64 ColdWaterMassFlowRate(0.0);
    Real64 QUnitOut(0.0);
    Real64 QLatOut(0.0);
    Real64 AirMassFlow(0.0);
    Real64 MaxAirMassFlow(0.0);

    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataWaterCoils->GetWaterCoilsInputFlag = true;
    state->dataGlobalNames->NumCoils = 0;
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->dataSize->CurZoneEqNum = 1;

    std::string const idf_objects = delimited_string({
        "	Zone,",
        "	EAST ZONE, !- Name",
        "	0, !- Direction of Relative North { deg }",
        "	0, !- X Origin { m }",
        "	0, !- Y Origin { m }",
        "	0, !- Z Origin { m }",
        "	1, !- Type",
        "	1, !- Multiplier",
        "	autocalculate, !- Ceiling Height { m }",
        "	autocalculate; !- Volume { m3 }",

        "	ZoneHVAC:EquipmentConnections,",
        "	EAST ZONE, !- Zone Name",
        "	Zone1Equipment, !- Zone Conditioning Equipment List Name",
        "	Zone1Inlets, !- Zone Air Inlet Node or NodeList Name",
        "	Zone1Exhausts, !- Zone Air Exhaust Node or NodeList Name",
        "	Zone 1 Node, !- Zone Air Node Name",
        "	Zone 1 Outlet Node;      !- Zone Return Air Node Name",

        "	ZoneHVAC:EquipmentList,",
        "	Zone1Equipment, !- Name",
        "   SequentialLoad,          !- Load Distribution Scheme",
        "	ZoneHVAC:FourPipeFanCoil, !- Zone Equipment 1 Object Type",
        "	Zone1FanCoil, !- Zone Equipment 1 Name",
        "	1, !- Zone Equipment 1 Cooling Sequence",
        "	1;                       !- Zone Equipment 1 Heating or No - Load Sequence",

        "   NodeList,",
        "	Zone1Inlets, !- Name",
        "	Zone1FanCoilAirOutletNode;  !- Node 1 Name",

        "	NodeList,",
        "	Zone1Exhausts, !- Name",
        "	Zone1FanCoilAirInletNode; !- Node 1 Name",

        "	OutdoorAir:NodeList,",
        "	Zone1FanCoilOAInNode;    !- Node or NodeList Name 1",

        "	OutdoorAir:Mixer,",
        "	Zone1FanCoilOAMixer, !- Name",
        "	Zone1FanCoilOAMixerOutletNode, !- Mixed Air Node Name",
        "	Zone1FanCoilOAInNode, !- Outdoor Air Stream Node Name",
        "	Zone1FanCoilExhNode, !- Relief Air Stream Node Name",
        "	Zone1FanCoilAirInletNode; !- Return Air Stream Node Name",

        "	Schedule:Constant,",
        "	FanAndCoilAvailSched, !- Name",
        "	FRACTION, !- Schedule Type",
        "	1;        !- TimeStep Value",

        "	ScheduleTypeLimits,",
        "	Fraction, !- Name",
        "	0.0, !- Lower Limit Value",
        "	1.0, !- Upper Limit Value",
        "	CONTINUOUS;              !- Numeric Type",

        "   Fan:OnOff,",
        "	Zone1FanCoilFan, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Name",
        "	0.5, !- Fan Total Efficiency",
        "	75.0, !- Pressure Rise { Pa }",
        "	0.6, !- Maximum Flow Rate { m3 / s }",
        "	0.9, !- Motor Efficiency",
        "	1.0, !- Motor In Airstream Fraction",
        "	Zone1FanCoilOAMixerOutletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilFanOutletNode, !- Air Outlet Node Name",
        "	, !- Fan Power Ratio Function of Speed Ratio Curve Name",
        "	;                        !- Fan Efficiency Ratio Function of Speed Ratio Curve Name	",

        "	Coil:Cooling:Water,",
        "	Zone1FanCoilCoolingCoil, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Namev",
        "	0.0002, !- Design Water Flow Rate { m3 / s }",
        "	0.5000, !- Design Air Flow Rate { m3 / s }",
        "	7.22,   !- Design Inlet Water Temperature { Cv }",
        "	24.340, !- Design Inlet Air Temperature { C }",
        "	14.000, !- Design Outlet Air Temperature { C }",
        "	0.0095, !- Design Inlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	0.0090, !- Design Outlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	Zone1FanCoilChWInletNode, !- Water Inlet Node Name",
        "	Zone1FanCoilChWOutletNode, !- Water Outlet Node Name",
        "	Zone1FanCoilFanOutletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilCCOutletNode, !- Air Outlet Node Name",
        "	SimpleAnalysis, !- Type of Analysis",
        "	CrossFlow;               !- Heat Exchanger Configuration",

        "	Coil:Heating:Water,",
        "   Zone1FanCoilHeatingCoil, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Name",
        "	150.0,   !- U - Factor Times Area Value { W / K }",
        "	0.00014, !- Maximum Water Flow Rate { m3 / s }",
        "	Zone1FanCoilHWInletNode, !- Water Inlet Node Name",
        "	Zone1FanCoilHWOutletNode, !- Water Outlet Node Name",
        "	Zone1FanCoilCCOutletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilAirOutletNode, !- Air Outlet Node Name",
        "	UFactorTimesAreaAndDesignWaterFlowRate, !- Performance Input Method",
        "	autosize, !- Rated Capacity { W }",
        "	82.2, !- Rated Inlet Water Temperature { C }",
        "	16.6, !- Rated Inlet Air Temperature { C }",
        "	71.1, !- Rated Outlet Water Temperature { C }",
        "	32.2, !- Rated Outlet Air Temperature { C }",
        "	;     !- Rated Ratio for Air and Water Convection",

        "	ZoneHVAC:FourPipeFanCoil,",
        "	Zone1FanCoil, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Name",
        "	CyclingFan,           !- Capacity Control Method",
        "	0.5, !- Maximum Supply Air Flow Rate { m3 / s }",
        "	0.3, !- Low Speed Supply Air Flow Ratio",
        "	0.6, !- Medium Speed Supply Air Flow Ratio",
        "	0.0, !- Maximum Outdoor Air Flow Rate { m3 / s }",
        "	FanAndCoilAvailSched, !- Outdoor Air Schedule Name",
        "	Zone1FanCoilAirInletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilAirOutletNode, !- Air Outlet Node Name",
        "	OutdoorAir:Mixer, !- Outdoor Air Mixer Object Type",
        "	Zone1FanCoilOAMixer, !- Outdoor Air Mixer Name",
        "	Fan:OnOff, !- Supply Air Fan Object Type",
        "	Zone1FanCoilFan, !- Supply Air Fan Name",
        "	Coil:Cooling:Water, !- Cooling Coil Object Type",
        "	Zone1FanCoilCoolingCoil, !- Cooling Coil Name",
        "	0.00014, !- Maximum Cold Water Flow Rate { m3 / s }",
        "	0.0, !- Minimum Cold Water Flow Rate { m3 / s }",
        "	0.001, !- Cooling Convergence Tolerance",
        "	Coil:Heating:Water, !- Heating Coil Object Type",
        "	Zone1FanCoilHeatingCoil, !- Heating Coil Name",
        "	0.00014, !- Maximum Hot Water Flow Rate { m3 / s }",
        "	0.0, !- Minimum Hot Water Flow Rate { m3 / s }",
        "	0.001; !- Heating Convergence Tolerance",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    EXPECT_EQ("EAST ZONE", state->dataHeatBal->Zone(1).Name);

    GetZoneEquipmentData(*state);
    GetFanInput(*state);
    EXPECT_EQ((int)HVAC::FanType::OnOff, (int)state->dataFans->fans(1)->type);

    GetFanCoilUnits(*state);
    EXPECT_ENUM_EQ(CCM::CycFan, state->dataFanCoilUnits->FanCoil(1).CapCtrlMeth_Num);
    EXPECT_EQ("OUTDOORAIR:MIXER", state->dataFanCoilUnits->FanCoil(1).OAMixType);
    EXPECT_EQ((int)HVAC::FanType::OnOff, (int)state->dataFanCoilUnits->FanCoil(1).fanType);
    EXPECT_EQ("COIL:COOLING:WATER", state->dataFanCoilUnits->FanCoil(1).CCoilType);
    EXPECT_EQ("COIL:HEATING:WATER", state->dataFanCoilUnits->FanCoil(1).HCoilType);

    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

    AirMassFlow = 0.60;
    MaxAirMassFlow = 0.60;
    // heating load only
    ColdWaterMassFlowRate = 0.0;
    HotWaterMassFlowRate = 1.0;

    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).MassFlowRateMax = MaxAirMassFlow;

    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Temp = 22.0;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Enthalpy = 36000;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).HumRat =
        PsyWFnTdbH(*state,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Temp,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).RetNode).Enthalpy);

    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Temp = 10.0;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Enthalpy = 18000;
    state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).HumRat =
        PsyWFnTdbH(*state,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Temp,
                   state->dataLoopNodes->Node(state->dataMixedAir->OAMixer(1).InletNode).Enthalpy);

    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMinAvail = AirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMax = MaxAirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).AirInNode).MassFlowRateMaxAvail = MaxAirMassFlow;

    state->dataFanCoilUnits->FanCoil(1).OutAirMassFlow = 0.0;
    state->dataFanCoilUnits->FanCoil(1).MaxAirMassFlow = MaxAirMassFlow;
    state->dataLoopNodes->Node(state->dataFanCoilUnits->FanCoil(1).OutsideAirNode).MassFlowRateMax = 0.0;

    auto *fan1 = state->dataFans->fans(1);
    fan1->inletAirMassFlowRate = AirMassFlow;
    fan1->maxAirMassFlowRate = MaxAirMassFlow;

    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRateMax = AirMassFlow;
    state->dataLoopNodes->Node(fan1->inletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

    state->dataWaterCoils->WaterCoil(2).UACoilTotal = 470.0;
    state->dataWaterCoils->WaterCoil(2).UACoilExternal = 611.0;
    state->dataWaterCoils->WaterCoil(2).UACoilInternal = 2010.0;
    state->dataWaterCoils->WaterCoil(2).TotCoilOutsideSurfArea = 50.0;

    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).AirInletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).AirInletNodeNum).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).AirInletNodeNum).MassFlowRateMax = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).AirInletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

    state->dataWaterCoils->WaterCoil(2).InletWaterMassFlowRate = ColdWaterMassFlowRate;
    state->dataWaterCoils->WaterCoil(2).MaxWaterMassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterInletNodeNum).MassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterInletNodeNum).MassFlowRateMaxAvail = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterInletNodeNum).Temp = 6.0;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterOutletNodeNum).MassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(2).WaterOutletNodeNum).MassFlowRateMaxAvail = ColdWaterMassFlowRate;

    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).AirInletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).AirInletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum).Temp = 60.0;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum).MassFlowRate = HotWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum).MassFlowRateMaxAvail = HotWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum).MassFlowRate = HotWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum).MassFlowRateMaxAvail = HotWaterMassFlowRate;
    state->dataWaterCoils->WaterCoil(1).InletWaterMassFlowRate = HotWaterMassFlowRate;
    state->dataWaterCoils->WaterCoil(1).MaxWaterMassFlowRate = HotWaterMassFlowRate;

    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }

    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.loopNum = 1;
    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.branchNum = 1;
    state->dataWaterCoils->WaterCoil(2).WaterPlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataWaterCoils->WaterCoil(2).WaterPlantLoc);

    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.loopNum = 2;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.branchNum = 1;
    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataWaterCoils->WaterCoil(1).WaterPlantLoc);

    state->dataPlnt->PlantLoop(2).Name = "ChilledWaterLoop";
    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = state->dataWaterCoils->WaterCoil(2).Name;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilWaterCooling;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterCoils->WaterCoil(2).WaterInletNodeNum;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut =
        state->dataWaterCoils->WaterCoil(2).WaterOutletNodeNum;

    state->dataPlnt->PlantLoop(1).Name = "HotWaterLoop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = state->dataWaterCoils->WaterCoil(1).Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut =
        state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum;

    state->dataFanCoilUnits->CoolingLoad = false;
    state->dataFanCoilUnits->HeatingLoad = true;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = 0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 4000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = 4000.0;
    state->dataFanCoilUnits->FanCoil(1).SpeedFanSel = 2;
    QUnitOut = 0.0;
    QLatOut = 0.0;
    QZnReq = 4000.0;

    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(2);
    state->dataWaterCoils->MyUAAndFlowCalcFlag(1) = true;
    state->dataWaterCoils->MyUAAndFlowCalcFlag(2) = true;
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
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod.allocate(HVAC::NumOfSizingTypes);
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod = 0;
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow = 0.5;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow = 0.5;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInTemp = 30.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInHumRat = 0.01;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInTemp = 20.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInHumRat = 0.005;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolLoad = 4000.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatLoad = 4000.0;
    state->dataEnvrn->StdRhoAir = 1.2;

    state->dataGlobal->BeginEnvrnFlag = true;
    InitFanCoilUnits(*state, FanCoilNum, ZoneNum);
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    // expect fan speed 3 and near full air and water flow and meet capacity
    EXPECT_EQ(3, state->dataFanCoilUnits->FanCoil(1).SpeedFanSel);
    EXPECT_GT(state->dataFanCoilUnits->FanCoil(1).PLR, 0.95);
    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    // cycling fan proportional to PLR and fan speed ratio (is 1 here)
    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate,
                state->dataFanCoilUnits->FanCoil(1).PLR * state->dataFanCoilUnits->FanCoil(1).MaxAirMassFlow,
                0.0000000001);

    // expect minimum flow and meet capacity
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 1000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = 1000.0;
    QZnReq = 1000.0;
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    // expect fan speed 1 and moderate air and water flow and meet capacity
    EXPECT_EQ(1, state->dataFanCoilUnits->FanCoil(1).SpeedFanSel);
    EXPECT_GT(state->dataFanCoilUnits->FanCoil(1).PLR, 0.6);
    EXPECT_LT(state->dataFanCoilUnits->FanCoil(1).PLR, 0.65);
    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    // cycling fan proportional to PLR and fan speed ratio (is 0.3 here)
    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate,
                state->dataFanCoilUnits->FanCoil(1).PLR * state->dataFanCoilUnits->FanCoil(1).MaxAirMassFlow *
                    state->dataFanCoilUnits->FanCoil(1).LowSpeedRatio,
                0.0000000001);

    // expect modulated flow and meet capacity
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 2500.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = 2500.0;
    QZnReq = 2500.0;
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    // expect fan speed 2 and moderate air and water flow and meet capacity
    EXPECT_EQ(2, state->dataFanCoilUnits->FanCoil(1).SpeedFanSel);
    EXPECT_GT(state->dataFanCoilUnits->FanCoil(1).PLR, 0.8);
    EXPECT_LT(state->dataFanCoilUnits->FanCoil(1).PLR, 0.85);
    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    // cycling fan proportional to PLR and fan speed ratio (is 0.6 here)
    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate,
                state->dataFanCoilUnits->FanCoil(1).PLR * state->dataFanCoilUnits->FanCoil(1).MaxAirMassFlow *
                    state->dataFanCoilUnits->FanCoil(1).MedSpeedRatio,
                0.0000000001);

    // expect full flow and meet capacity
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -4000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = -4000.0;
    QZnReq = -4000.0;
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    // expect fan speed 3 and near full air and water flow and meet capacity
    EXPECT_EQ(3, state->dataFanCoilUnits->FanCoil(1).SpeedFanSel);
    EXPECT_GT(state->dataFanCoilUnits->FanCoil(1).PLR, 0.9);
    EXPECT_NEAR(state->dataFanCoilUnits->FanCoil(1).PLR, 0.95, 0.001);
    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    // cycling fan proportional to PLR and fan speed ratio (is 1 here)
    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate,
                state->dataFanCoilUnits->FanCoil(1).PLR * state->dataFanCoilUnits->FanCoil(1).MaxAirMassFlow,
                0.0000000001);

    // expect minimum flow and meet capacity
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -1000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = -1000.0;
    QZnReq = -1000.0;
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    // expect fan speed 1 and moderate air and water flow and meet capacity
    EXPECT_EQ(1, state->dataFanCoilUnits->FanCoil(1).SpeedFanSel);
    EXPECT_GT(state->dataFanCoilUnits->FanCoil(1).PLR, 0.5);
    EXPECT_LT(state->dataFanCoilUnits->FanCoil(1).PLR, 0.55);
    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    // cycling fan proportional to PLR and fan speed ratio (is 0.3 here)
    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate,
                state->dataFanCoilUnits->FanCoil(1).PLR * state->dataFanCoilUnits->FanCoil(1).MaxAirMassFlow *
                    state->dataFanCoilUnits->FanCoil(1).LowSpeedRatio,
                0.0000000001);

    // expect modulated flow and meet capacity
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -2500.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = -2500.0;
    QZnReq = -2500.0;
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    // expect fan speed 2 and moderate air and water flow and meet capacity
    EXPECT_EQ(2, state->dataFanCoilUnits->FanCoil(1).SpeedFanSel);
    EXPECT_GT(state->dataFanCoilUnits->FanCoil(1).PLR, 0.75);
    EXPECT_LT(state->dataFanCoilUnits->FanCoil(1).PLR, 0.8);
    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate,
                state->dataFanCoilUnits->FanCoil(1).PLR * state->dataFanCoilUnits->FanCoil(1).MaxAirMassFlow *
                    state->dataFanCoilUnits->FanCoil(1).MedSpeedRatio,
                0.0000000001);
}

TEST_F(EnergyPlusFixture, FanCoil_FanSystemModelCyclingFanMode)
{
    int FanCoilNum(1);
    int ZoneNum(1);
    bool FirstHVACIteration(false);
    bool ErrorsFound(false);
    Real64 QZnReq(0.0);
    Real64 HotWaterMassFlowRate(0.0);
    Real64 ColdWaterMassFlowRate(0.0);
    Real64 QUnitOut(0.0);
    Real64 QLatOut(0.0);
    Real64 AirMassFlow(0.0);
    Real64 MaxAirMassFlow(0.0);

    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataWaterCoils->GetWaterCoilsInputFlag = true;
    state->dataGlobalNames->NumCoils = 0;
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->dataSize->CurZoneEqNum = 1;

    std::string const idf_objects = delimited_string({
        "	Zone,",
        "	EAST ZONE,     !- Name",
        "	0,             !- Direction of Relative North { deg }",
        "	0,             !- X Origin { m }",
        "	0,             !- Y Origin { m }",
        "	0,             !- Z Origin { m }",
        "	1,             !- Type",
        "	1,             !- Multiplier",
        "	autocalculate, !- Ceiling Height { m }",
        "	autocalculate; !- Volume { m3 }",

        "	ZoneHVAC:EquipmentConnections,",
        "	EAST ZONE,          !- Zone Name",
        "	Zone1Equipment,     !- Zone Conditioning Equipment List Name",
        "	Zone1Inlets,        !- Zone Air Inlet Node or NodeList Name",
        "	Zone1Exhausts,      !- Zone Air Exhaust Node or NodeList Name",
        "	Zone 1 Node,        !- Zone Air Node Name",
        "	Zone 1 Outlet Node; !- Zone Return Air Node Name",

        "	ZoneHVAC:EquipmentList,",
        "	Zone1Equipment,           !- Name",
        "   SequentialLoad,           !- Load Distribution Scheme",
        "	ZoneHVAC:FourPipeFanCoil, !- Zone Equipment 1 Object Type",
        "	Zone1FanCoil,             !- Zone Equipment 1 Name",
        "	1,                        !- Zone Equipment 1 Cooling Sequence",
        "	1;                        !- Zone Equipment 1 Heating or No - Load Sequence",

        "   NodeList,",
        "	Zone1Inlets,              !- Name",
        "	Zone1FanCoilAirOutletNode;!- Node 1 Name",

        "	NodeList,",
        "	Zone1Exhausts,            !- Name",
        "	Zone1FanCoilAirInletNode; !- Node 1 Name",

        "	OutdoorAir:NodeList,",
        "	Zone1FanCoilOAInNode;     !- Node or NodeList Name 1",

        "	OutdoorAir:Mixer,",
        "	Zone1FanCoilOAMixer,      !- Name",
        "	Zone1FanCoilOAMixerOutletNode, !- Mixed Air Node Name",
        "	Zone1FanCoilOAInNode,     !- Outdoor Air Stream Node Name",
        "	Zone1FanCoilExhNode,      !- Relief Air Stream Node Name",
        "	Zone1FanCoilAirInletNode; !- Return Air Stream Node Name",

        "	Schedule:Constant,",
        "	FanAndCoilAvailSched,     !- Name",
        "	FRACTION,                 !- Schedule Type",
        "	1;                        !- TimeStep Value",

        "	ScheduleTypeLimits,",
        "	Fraction,                 !- Name",
        "	0.0,                      !- Lower Limit Value",
        "	1.0,                      !- Upper Limit Value",
        "	CONTINUOUS;               !- Numeric Type",

        "    Fan:SystemModel,",
        "      Zone1FanCoilFan,         !- Name",
        "      FanAndCoilAvailSched,    !- Availability Schedule Name",
        "      Zone1FanCoilOAMixerOutletNode,  !- Air Inlet Node Name",
        "      Zone1FanCoilFanOutletNode,  !- Air Outlet Node Name",
        "      0.6,                     !- Design Maximum Air Flow Rate {m3/s}",
        "      Discrete,                !- Speed Control Method",
        "      0.0,                     !- Electric Power Minimum Flow Rate Fraction",
        "      75,                      !- Design Pressure Rise {Pa}",
        "      0.9,                     !- Motor Efficiency",
        "      1,                       !- Motor In Air Stream Fraction",
        "      ,                        !- Design Electric Power Consumption {W}",
        "      TotalEfficiencyAndPressure,  !- Design Power Sizing Method",
        "      ,                        !- Electric Power Per Unit Flow Rate {W/(m3/s)}",
        "      ,                        !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}",
        "      0.5,                     !- Fan Total Efficiency",
        "      ,                        !- Electric Power Function of Flow Fraction Curve Name",
        "      ,                        !- Night Ventilation Mode Pressure Rise {Pa}",
        "      ,                        !- Night Ventilation Mode Flow Fraction",
        "      ,                        !- Motor Loss Zone Name",
        "      ,                        !- Motor Loss Radiative Fraction",
        "      General,                 !- End-Use Subcategory",
        "      1,                       !- Number of Speeds",
        "      1.0,                     !- Speed 1 Flow Fraction",
        "      1.0;                     !- Speed 1 Electric Power Fraction",

        "	Coil:Cooling:Water,",
        "	Zone1FanCoilCoolingCoil,  !- Name",
        "	FanAndCoilAvailSched,     !- Availability Schedule Namev",
        "	0.0002,                   !- Design Water Flow Rate { m3 / s }",
        "	0.5000,                   !- Design Air Flow Rate { m3 / s }",
        "	7.22,                     !- Design Inlet Water Temperature { Cv }",
        "	24.340,                   !- Design Inlet Air Temperature { C }",
        "	14.000,                   !- Design Outlet Air Temperature { C }",
        "	0.0095,                   !- Design Inlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	0.0090,                   !- Design Outlet Air Humidity Ratio { kgWater / kgDryAir }",
        "	Zone1FanCoilChWInletNode, !- Water Inlet Node Name",
        "	Zone1FanCoilChWOutletNode,!- Water Outlet Node Name",
        "	Zone1FanCoilFanOutletNode,!- Air Inlet Node Name",
        "	Zone1FanCoilCCOutletNode, !- Air Outlet Node Name",
        "	SimpleAnalysis,           !- Type of Analysis",
        "	CrossFlow;                !- Heat Exchanger Configuration",

        "	Coil:Heating:Water,",
        "   Zone1FanCoilHeatingCoil,   !- Name",
        "	FanAndCoilAvailSched,      !- Availability Schedule Name",
        "	150.0,                     !- U - Factor Times Area Value { W / K }",
        "	0.00014,                   !- Maximum Water Flow Rate { m3 / s }",
        "	Zone1FanCoilHWInletNode,   !- Water Inlet Node Name",
        "	Zone1FanCoilHWOutletNode,  !- Water Outlet Node Name",
        "	Zone1FanCoilCCOutletNode,  !- Air Inlet Node Name",
        "	Zone1FanCoilAirOutletNode, !- Air Outlet Node Name",
        "	UFactorTimesAreaAndDesignWaterFlowRate, !- Performance Input Method",
        "	autosize,                  !- Rated Capacity { W }",
        "	82.2,                      !- Rated Inlet Water Temperature { C }",
        "	16.6,                      !- Rated Inlet Air Temperature { C }",
        "	71.1,                      !- Rated Outlet Water Temperature { C }",
        "	32.2,                      !- Rated Outlet Air Temperature { C }",
        "	;                          !- Rated Ratio for Air and Water Convection",

        "	ZoneHVAC:FourPipeFanCoil,",
        "	Zone1FanCoil,              !- Name",
        "	FanAndCoilAvailSched,      !- Availability Schedule Name",
        "	CyclingFan,                !- Capacity Control Method",
        "	0.5,                       !- Maximum Supply Air Flow Rate { m3 / s }",
        "	0.3,                       !- Low Speed Supply Air Flow Ratio",
        "	0.6,                       !- Medium Speed Supply Air Flow Ratio",
        "	0.0,                       !- Maximum Outdoor Air Flow Rate { m3 / s }",
        "	FanAndCoilAvailSched,      !- Outdoor Air Schedule Name",
        "	Zone1FanCoilAirInletNode,  !- Air Inlet Node Name",
        "	Zone1FanCoilAirOutletNode, !- Air Outlet Node Name",
        "	OutdoorAir:Mixer,          !- Outdoor Air Mixer Object Type",
        "	Zone1FanCoilOAMixer,       !- Outdoor Air Mixer Name",
        "	Fan:SystemModel,           !- Supply Air Fan Object Type",
        "	Zone1FanCoilFan,           !- Supply Air Fan Name",
        "	Coil:Cooling:Water,        !- Cooling Coil Object Type",
        "	Zone1FanCoilCoolingCoil,   !- Cooling Coil Name",
        "	0.00014,                   !- Maximum Cold Water Flow Rate { m3 / s }",
        "	0.0,                       !- Minimum Cold Water Flow Rate { m3 / s }",
        "	0.001,                     !- Cooling Convergence Tolerance",
        "	Coil:Heating:Water,        !- Heating Coil Object Type",
        "	Zone1FanCoilHeatingCoil,   !- Heating Coil Name",
        "	0.00014,                   !- Maximum Hot Water Flow Rate { m3 / s }",
        "	0.0,                       !- Minimum Hot Water Flow Rate { m3 / s }",
        "	0.001;                     !- Heating Convergence Tolerance",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    EXPECT_EQ("EAST ZONE", state->dataHeatBal->Zone(1).Name);

    GetZoneEquipmentData(*state);
    GetFanCoilUnits(*state);

    auto &thisFanCoil(state->dataFanCoilUnits->FanCoil(1));

    EXPECT_ENUM_EQ(CCM::CycFan, thisFanCoil.CapCtrlMeth_Num);
    EXPECT_EQ("OUTDOORAIR:MIXER", thisFanCoil.OAMixType);
    EXPECT_EQ((int)HVAC::FanType::SystemModel, (int)thisFanCoil.fanType);
    EXPECT_EQ("COIL:COOLING:WATER", thisFanCoil.CCoilType);
    EXPECT_EQ("COIL:HEATING:WATER", thisFanCoil.HCoilType);

    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

    AirMassFlow = 0.60;
    MaxAirMassFlow = 0.60;
    ColdWaterMassFlowRate = 1.0;
    HotWaterMassFlowRate = 1.0;

    thisFanCoil.OutAirMassFlow = 0.0;
    thisFanCoil.MaxAirMassFlow = MaxAirMassFlow;
    state->dataLoopNodes->Node(thisFanCoil.OutsideAirNode).MassFlowRateMax = 0.0;

    state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRateMinAvail = AirMassFlow;
    state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRateMax = MaxAirMassFlow;
    state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRateMaxAvail = MaxAirMassFlow;

    // outside air mixer
    auto &MixerOA(state->dataMixedAir->OAMixer(1));
    state->dataLoopNodes->Node(MixerOA.RetNode).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(MixerOA.RetNode).MassFlowRateMax = MaxAirMassFlow;
    state->dataLoopNodes->Node(MixerOA.RetNode).Temp = 22.0;
    state->dataLoopNodes->Node(MixerOA.RetNode).Enthalpy = 36000;
    state->dataLoopNodes->Node(MixerOA.RetNode).HumRat =
        PsyWFnTdbH(*state, state->dataLoopNodes->Node(MixerOA.RetNode).Temp, state->dataLoopNodes->Node(MixerOA.RetNode).Enthalpy);
    state->dataLoopNodes->Node(MixerOA.InletNode).Temp = 10.0;
    state->dataLoopNodes->Node(MixerOA.InletNode).Enthalpy = 18000;
    state->dataLoopNodes->Node(MixerOA.InletNode).HumRat =
        PsyWFnTdbH(*state, state->dataLoopNodes->Node(MixerOA.InletNode).Temp, state->dataLoopNodes->Node(MixerOA.InletNode).Enthalpy);

    // chilled water coil
    auto &CWCoil(state->dataWaterCoils->WaterCoil(2));
    CWCoil.UACoilTotal = 470.0;
    CWCoil.UACoilExternal = 611.0;
    CWCoil.UACoilInternal = 2010.0;
    CWCoil.TotCoilOutsideSurfArea = 50.0;
    state->dataLoopNodes->Node(CWCoil.AirInletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(CWCoil.AirInletNodeNum).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(CWCoil.AirInletNodeNum).MassFlowRateMax = AirMassFlow;
    state->dataLoopNodes->Node(CWCoil.AirInletNodeNum).MassFlowRateMaxAvail = AirMassFlow;
    CWCoil.InletWaterMassFlowRate = ColdWaterMassFlowRate;
    CWCoil.MaxWaterMassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(CWCoil.WaterInletNodeNum).MassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(CWCoil.WaterInletNodeNum).MassFlowRateMaxAvail = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(CWCoil.WaterInletNodeNum).Temp = 6.0;
    state->dataLoopNodes->Node(CWCoil.WaterOutletNodeNum).MassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(CWCoil.WaterOutletNodeNum).MassFlowRateMaxAvail = ColdWaterMassFlowRate;

    // hot water coil
    auto &HWCoil(state->dataWaterCoils->WaterCoil(1));
    HWCoil.InletWaterMassFlowRate = HotWaterMassFlowRate;
    HWCoil.MaxWaterMassFlowRate = HotWaterMassFlowRate;
    state->dataLoopNodes->Node(HWCoil.AirInletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(HWCoil.AirInletNodeNum).MassFlowRateMaxAvail = AirMassFlow;
    state->dataLoopNodes->Node(HWCoil.WaterInletNodeNum).Temp = 60.0;
    state->dataLoopNodes->Node(HWCoil.WaterInletNodeNum).MassFlowRate = HotWaterMassFlowRate;
    state->dataLoopNodes->Node(HWCoil.WaterInletNodeNum).MassFlowRateMaxAvail = HotWaterMassFlowRate;
    state->dataLoopNodes->Node(HWCoil.WaterOutletNodeNum).MassFlowRate = HotWaterMassFlowRate;
    state->dataLoopNodes->Node(HWCoil.WaterOutletNodeNum).MassFlowRateMaxAvail = HotWaterMassFlowRate;

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
    // hot water plant loop
    auto &HWLoop(state->dataPlnt->PlantLoop(1));
    HWLoop.Name = "HotWaterLoop";
    HWLoop.FluidName = "WATER";
    HWLoop.glycol = Fluid::GetWater(*state);
    HWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = HWCoil.Name;
    HWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type = DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
    HWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = HWCoil.WaterInletNodeNum;
    HWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = HWCoil.WaterOutletNodeNum;

    // Set PlantLoc links, now that plant structures are created
    CWCoil.WaterPlantLoc.loopNum = 1;
    CWCoil.WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    CWCoil.WaterPlantLoc.branchNum = 1;
    CWCoil.WaterPlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, CWCoil.WaterPlantLoc);

    HWCoil.WaterPlantLoc.loopNum = 2;
    HWCoil.WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    HWCoil.WaterPlantLoc.branchNum = 1;
    HWCoil.WaterPlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, HWCoil.WaterPlantLoc);

    // heating mode tests
    state->dataFanCoilUnits->CoolingLoad = false;
    state->dataFanCoilUnits->HeatingLoad = true;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = 0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 4000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = 4000.0;
    thisFanCoil.SpeedFanSel = 2;
    QUnitOut = 0.0;
    QLatOut = 0.0;
    QZnReq = 4000.0;

    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(2);
    state->dataWaterCoils->MyUAAndFlowCalcFlag(1) = true;
    state->dataWaterCoils->MyUAAndFlowCalcFlag(2) = true;
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
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod.allocate(HVAC::NumOfSizingTypes);
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod = 0;
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow = 0.5;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow = 0.5;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInTemp = 30.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInHumRat = 0.01;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInTemp = 20.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInHumRat = 0.005;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolLoad = 4000.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatLoad = 4000.0;
    state->dataEnvrn->StdRhoAir = 1.2;

    state->dataGlobal->BeginEnvrnFlag = true;
    InitFanCoilUnits(*state, FanCoilNum, ZoneNum);
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    // expect fan speed 3 and near full air and water flow and meet capacity
    EXPECT_EQ(thisFanCoil.SpeedFanSel, 3);
    EXPECT_NEAR(thisFanCoil.PLR, 0.961, 0.001);
    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    // cycling fan proportional to PLR and fan speed ratio
    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate, thisFanCoil.PLR * thisFanCoil.MaxAirMassFlow, 0.0000000001);
    // expect minimum flow and meet capacity
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 1000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = 1000.0;
    QZnReq = 1000.0;
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    // expect fan speed 1 and moderate air and water flow and meet capacity
    EXPECT_EQ(thisFanCoil.SpeedFanSel, 1);
    EXPECT_NEAR(thisFanCoil.PLR, 0.632, 0.001);
    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    // cycling fan proportional to PLR and fan speed ratio
    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate, thisFanCoil.PLR * thisFanCoil.MaxAirMassFlow * thisFanCoil.LowSpeedRatio, 0.0000000001);
    // expect modulated flow and meet capacity
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 2500.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = 2500.0;
    QZnReq = 2500.0;
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    // expect fan speed 2 and moderate air and water flow and meet capacity
    EXPECT_EQ(thisFanCoil.SpeedFanSel, 2);
    EXPECT_NEAR(thisFanCoil.PLR, 0.850, 0.001);
    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    // cycling fan proportional to PLR and fan speed ratio
    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate, thisFanCoil.PLR * thisFanCoil.MaxAirMassFlow * thisFanCoil.MedSpeedRatio, 0.0000000001);

    // cooling mode tests
    // expect full flow and meet capacity
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToHeatSP = 0.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -4000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = -4000.0;
    QZnReq = -4000.0;
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    // expect fan speed 3 and near full air and water flow and meet capacity
    EXPECT_EQ(3, state->dataFanCoilUnits->FanCoil(1).SpeedFanSel);
    EXPECT_NEAR(state->dataFanCoilUnits->FanCoil(1).PLR, 0.950, 0.001);
    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    // cycling fan proportional to PLR and fan speed ratio
    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate, thisFanCoil.PLR * thisFanCoil.MaxAirMassFlow, 0.0000000001);
    // expect minimum flow and meet capacity
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -1000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = -1000.0;
    QZnReq = -1000.0;
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    // expect fan speed 1 and moderate air and water flow and meet capacity
    EXPECT_EQ(1, state->dataFanCoilUnits->FanCoil(1).SpeedFanSel);
    EXPECT_NEAR(state->dataFanCoilUnits->FanCoil(1).PLR, 0.501, 0.001);
    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    // cycling fan proportional to PLR and fan speed ratio
    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate, thisFanCoil.PLR * thisFanCoil.MaxAirMassFlow * thisFanCoil.LowSpeedRatio, 0.0000000001);
    // expect modulated flow and meet capacity
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputReqToCoolSP = -2500.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).RemainingOutputRequired = -2500.0;
    QZnReq = -2500.0;
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    // expect fan speed 2 and moderate air and water flow and meet capacity
    EXPECT_EQ(2, state->dataFanCoilUnits->FanCoil(1).SpeedFanSel);
    EXPECT_NEAR(state->dataFanCoilUnits->FanCoil(1).PLR, 0.756, 0.001);
    EXPECT_NEAR(QZnReq, QUnitOut, 5.0);
    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate, thisFanCoil.PLR * thisFanCoil.MaxAirMassFlow * thisFanCoil.MedSpeedRatio, 0.0000000001);
}

TEST_F(EnergyPlusFixture, FanCoil_ElecHeatCoilMultiSpeedFanCyclingFanMode)
{
    int FanCoilNum(1);
    int ZoneNum(1);
    bool FirstHVACIteration(false);
    bool ErrorsFound(false);
    Real64 QZnReq(0.0);
    Real64 ColdWaterMassFlowRate(0.0);
    Real64 QUnitOut(0.0);
    Real64 QLatOut(0.0);
    Real64 AirMassFlow(0.0);
    Real64 MaxAirMassFlow(0.0);

    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataWaterCoils->GetWaterCoilsInputFlag = true;
    state->dataGlobalNames->NumCoils = 0;
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->dataSize->CurZoneEqNum = 1;

    std::string const idf_objects = delimited_string({
        " Zone,",
        "  EAST ZONE,     !- Name",
        "  0,             !- Direction of Relative North { deg }",
        "  0,             !- X Origin { m }",
        "  0,             !- Y Origin { m }",
        "  0,             !- Z Origin { m }",
        "  1,             !- Type",
        "  1,             !- Multiplier",
        "  autocalculate, !- Ceiling Height { m }",
        "  autocalculate; !- Volume { m3 }",

        " ZoneHVAC:EquipmentConnections,",
        "  EAST ZONE,          !- Zone Name",
        "  Zone1Equipment,     !- Zone Conditioning Equipment List Name",
        "  Zone1Inlets,        !- Zone Air Inlet Node or NodeList Name",
        "  Zone1Exhausts,      !- Zone Air Exhaust Node or NodeList Name",
        "  Zone 1 Node,        !- Zone Air Node Name",
        "  Zone 1 Outlet Node; !- Zone Return Air Node Name",

        " ZoneHVAC:EquipmentList,",
        "  Zone1Equipment,           !- Name",
        "  SequentialLoad,           !- Load Distribution Scheme",
        "  ZoneHVAC:FourPipeFanCoil, !- Zone Equipment 1 Object Type",
        "  Zone1FanCoil,             !- Zone Equipment 1 Name",
        "  1,                        !- Zone Equipment 1 Cooling Sequence",
        "  1;                        !- Zone Equipment 1 Heating or No - Load Sequence",

        " NodeList,",
        "  Zone1Inlets,              !- Name",
        "  Zone1FanCoilAirOutletNode;!- Node 1 Name",

        " NodeList,",
        "  Zone1Exhausts,            !- Name",
        "  Zone1FanCoilAirInletNode; !- Node 1 Name",

        " OutdoorAir:NodeList,",
        "  Zone1FanCoilOAInNode;     !- Node or NodeList Name 1",

        " OutdoorAir:Mixer,",
        "  Zone1FanCoilOAMixer,      !- Name",
        "  Zone1FanCoilOAMixerOutletNode, !- Mixed Air Node Name",
        "  Zone1FanCoilOAInNode,     !- Outdoor Air Stream Node Name",
        "  Zone1FanCoilExhNode,      !- Relief Air Stream Node Name",
        "  Zone1FanCoilAirInletNode; !- Return Air Stream Node Name",

        " Schedule:Constant,",
        "  FanAndCoilAvailSched,     !- Name",
        "  FRACTION,                 !- Schedule Type",
        "  1;                        !- TimeStep Value",

        " ScheduleTypeLimits,",
        "  Fraction,                 !- Name",
        "  0.0,                      !- Lower Limit Value",
        "  1.0,                      !- Upper Limit Value",
        "  CONTINUOUS;               !- Numeric Type",

        " Fan:SystemModel,",
        "  Zone1FanCoilFan,         !- Name",
        "  FanAndCoilAvailSched,    !- Availability Schedule Name",
        "  Zone1FanCoilOAMixerOutletNode,  !- Air Inlet Node Name",
        "  Zone1FanCoilFanOutletNode,  !- Air Outlet Node Name",
        "  0.5,                     !- Design Maximum Air Flow Rate {m3/s}",
        "  Discrete,                !- Speed Control Method",
        "  0.0,                     !- Electric Power Minimum Flow Rate Fraction",
        "  75,                      !- Design Pressure Rise {Pa}",
        "  0.9,                     !- Motor Efficiency",
        "  1,                       !- Motor In Air Stream Fraction",
        "  ,                        !- Design Electric Power Consumption {W}",
        "  TotalEfficiencyAndPressure,  !- Design Power Sizing Method",
        "  ,                        !- Electric Power Per Unit Flow Rate {W/(m3/s)}",
        "  ,                        !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}",
        "  0.5,                     !- Fan Total Efficiency",
        "  ,                        !- Electric Power Function of Flow Fraction Curve Name",
        "  ,                        !- Night Ventilation Mode Pressure Rise {Pa}",
        "  ,                        !- Night Ventilation Mode Flow Fraction",
        "  ,                        !- Motor Loss Zone Name",
        "  ,                        !- Motor Loss Radiative Fraction",
        "  General,                 !- End-Use Subcategory",
        "  1,                       !- Number of Speeds",
        "  1.0,                     !- Speed 1 Flow Fraction",
        "  1.0;                     !- Speed 1 Electric Power Fraction",

        " Coil:Cooling:Water,",
        "  Zone1FanCoilCoolingCoil,  !- Name",
        "  FanAndCoilAvailSched,     !- Availability Schedule Namev",
        "  0.0002,                   !- Design Water Flow Rate { m3 / s }",
        "  0.5000,                   !- Design Air Flow Rate { m3 / s }",
        "  7.22,                     !- Design Inlet Water Temperature { Cv }",
        "  24.340,                   !- Design Inlet Air Temperature { C }",
        "  14.000,                   !- Design Outlet Air Temperature { C }",
        "  0.0095,                   !- Design Inlet Air Humidity Ratio { kgWater / kgDryAir }",
        "  0.0090,                   !- Design Outlet Air Humidity Ratio { kgWater / kgDryAir }",
        "  Zone1FanCoilChWInletNode, !- Water Inlet Node Name",
        "  Zone1FanCoilChWOutletNode,!- Water Outlet Node Name",
        "  Zone1FanCoilFanOutletNode,!- Air Inlet Node Name",
        "  Zone1FanCoilCCOutletNode, !- Air Outlet Node Name",
        "  SimpleAnalysis,           !- Type of Analysis",
        "  CrossFlow;                !- Heat Exchanger Configuration",

        " Coil:Heating:Electric,",
        "  Zone1FanCoilHeatingCoil,   !- Name",
        "  FanAndCoilAvailSched,      !- Availability Schedule Name",
        "  1,                         !- Efficiency",
        "  10000.0,                    !- Nominal Capacity {W}",
        "  Zone1FanCoilCCOutletNode,  !- Air Inlet Node Name",
        "  Zone1FanCoilAirOutletNode; !- Air Outlet Node Name",

        " ZoneHVAC:FourPipeFanCoil,",
        "  Zone1FanCoil,              !- Name",
        "  FanAndCoilAvailSched,      !- Availability Schedule Name",
        "  MultiSpeedFan,             !- Capacity Control Method",
        "  0.5,                       !- Maximum Supply Air Flow Rate { m3 / s }",
        "  0.3,                       !- Low Speed Supply Air Flow Ratio",
        "  0.6,                       !- Medium Speed Supply Air Flow Ratio",
        "  0.0,                       !- Maximum Outdoor Air Flow Rate { m3 / s }",
        "  FanAndCoilAvailSched,      !- Outdoor Air Schedule Name",
        "  Zone1FanCoilAirInletNode,  !- Air Inlet Node Name",
        "  Zone1FanCoilAirOutletNode, !- Air Outlet Node Name",
        "  OutdoorAir:Mixer,          !- Outdoor Air Mixer Object Type",
        "  Zone1FanCoilOAMixer,       !- Outdoor Air Mixer Name",
        "  Fan:SystemModel,           !- Supply Air Fan Object Type",
        "  Zone1FanCoilFan,           !- Supply Air Fan Name",
        "  Coil:Cooling:Water,        !- Cooling Coil Object Type",
        "  Zone1FanCoilCoolingCoil,   !- Cooling Coil Name",
        "  0.00014,                   !- Maximum Cold Water Flow Rate { m3 / s }",
        "  0.0,                       !- Minimum Cold Water Flow Rate { m3 / s }",
        "  0.001,                     !- Cooling Convergence Tolerance",
        "  Coil:Heating:Electric,     !- Heating Coil Object Type",
        "  Zone1FanCoilHeatingCoil,   !- Heating Coil Name",
        "  0.0,                       !- Maximum Hot Water Flow Rate { m3 / s }",
        "  0.0,                       !- Minimum Hot Water Flow Rate { m3 / s }",
        "  0.001,                     !- Heating Convergence Tolerance",
        "  ,                          !- Availability Manager List Name",
        "  ,                          !- Design Specification ZoneHVAC Sizing Object Name",
        "  CyclingFanOperatingSch;    !- Supply Air Fan Operating Mode Schedule Name",

        " Schedule:Constant,",
        "  CyclingFanOperatingSch,    !- Name",
        "  FRACTION,                  !- Schedule Type",
        "  0;                         !- TimeStep Value",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    EXPECT_EQ("EAST ZONE", state->dataHeatBal->Zone(1).Name);
    GetZoneEquipmentData(*state);
    GetFanCoilUnits(*state);
    auto &thisFanCoil(state->dataFanCoilUnits->FanCoil(1));
    EXPECT_ENUM_EQ(CCM::MultiSpeedFan, thisFanCoil.CapCtrlMeth_Num);
    EXPECT_EQ("OUTDOORAIR:MIXER", thisFanCoil.OAMixType);
    EXPECT_EQ("COIL:COOLING:WATER", thisFanCoil.CCoilType);
    EXPECT_EQ("COIL:HEATING:ELECTRIC", thisFanCoil.HCoilType);
    EXPECT_EQ((int)HVAC::FanType::SystemModel, (int)thisFanCoil.fanType);

    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    AirMassFlow = 0.60;
    MaxAirMassFlow = 0.60;
    ColdWaterMassFlowRate = 1.0;
    thisFanCoil.OutAirMassFlow = 0.0;
    thisFanCoil.MaxAirMassFlow = MaxAirMassFlow;
    state->dataLoopNodes->Node(thisFanCoil.OutsideAirNode).MassFlowRateMax = 0.0;
    state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRateMinAvail = AirMassFlow;
    state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRateMax = MaxAirMassFlow;
    state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRateMaxAvail = MaxAirMassFlow;
    // outside air mixer
    auto &MixerOA(state->dataMixedAir->OAMixer(1));
    state->dataLoopNodes->Node(MixerOA.RetNode).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(MixerOA.RetNode).MassFlowRateMax = MaxAirMassFlow;
    state->dataLoopNodes->Node(MixerOA.RetNode).Temp = 22.0;
    state->dataLoopNodes->Node(MixerOA.RetNode).Enthalpy = 36000;
    state->dataLoopNodes->Node(MixerOA.RetNode).HumRat =
        PsyWFnTdbH(*state, state->dataLoopNodes->Node(MixerOA.RetNode).Temp, state->dataLoopNodes->Node(MixerOA.RetNode).Enthalpy);
    state->dataLoopNodes->Node(MixerOA.InletNode).Temp = 10.0;
    state->dataLoopNodes->Node(MixerOA.InletNode).Enthalpy = 18000;
    state->dataLoopNodes->Node(MixerOA.InletNode).HumRat =
        PsyWFnTdbH(*state, state->dataLoopNodes->Node(MixerOA.InletNode).Temp, state->dataLoopNodes->Node(MixerOA.InletNode).Enthalpy);
    // chilled water coil
    auto &CWCoil(state->dataWaterCoils->WaterCoil(1));
    CWCoil.UACoilTotal = 470.0;
    CWCoil.UACoilExternal = 611.0;
    CWCoil.UACoilInternal = 2010.0;
    CWCoil.TotCoilOutsideSurfArea = 50.0;
    state->dataLoopNodes->Node(CWCoil.AirInletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(CWCoil.AirInletNodeNum).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(CWCoil.AirInletNodeNum).MassFlowRateMax = AirMassFlow;
    state->dataLoopNodes->Node(CWCoil.AirInletNodeNum).MassFlowRateMaxAvail = AirMassFlow;
    CWCoil.InletWaterMassFlowRate = ColdWaterMassFlowRate;
    CWCoil.MaxWaterMassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(CWCoil.WaterInletNodeNum).MassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(CWCoil.WaterInletNodeNum).MassFlowRateMaxAvail = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(CWCoil.WaterInletNodeNum).Temp = 6.0;
    state->dataLoopNodes->Node(CWCoil.WaterOutletNodeNum).MassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(CWCoil.WaterOutletNodeNum).MassFlowRateMaxAvail = ColdWaterMassFlowRate;

    // electric heating coil
    auto &eHCoil(state->dataHeatingCoils->HeatingCoil(1));
    state->dataLoopNodes->Node(eHCoil.AirInletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(eHCoil.AirInletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }
    // chilled water plant loop
    auto &CWLoop(state->dataPlnt->PlantLoop(1));
    CWLoop.Name = "ChilledWaterLoop";
    CWLoop.FluidName = "WATER";
    CWLoop.glycol = Fluid::GetWater(*state);
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = CWCoil.Name;
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type = DataPlant::PlantEquipmentType::CoilWaterCooling;
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = CWCoil.WaterInletNodeNum;
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = CWCoil.WaterOutletNodeNum;

    // Set plantLocation links now that plant objects have been created
    CWCoil.WaterPlantLoc.loopNum = 1;
    CWCoil.WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    CWCoil.WaterPlantLoc.branchNum = 1;
    CWCoil.WaterPlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, CWCoil.WaterPlantLoc);

    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(1);
    state->dataWaterCoils->MyUAAndFlowCalcFlag(1) = true;
    state->dataGlobal->DoingSizing = true;
    state->dataHVACGlobal->TurnFansOff = false;
    state->dataHVACGlobal->TurnFansOn = true;
    // heating mode tests
    state->dataFanCoilUnits->CoolingLoad = false;
    state->dataFanCoilUnits->HeatingLoad = true;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    auto &zSysEDemand(state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1));

    state->dataEnvrn->StdRhoAir = 1.2;
    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 21;
    state->dataGlobal->HourOfDay = 1;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    Sched::UpdateScheduleVals(*state);
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod.allocate(HVAC::NumOfSizingTypes);
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod = 0;
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->FinalZoneSizing.allocate(1);
    auto &fZoneSizing(state->dataSize->FinalZoneSizing(1));
    fZoneSizing.DesCoolVolFlow = 0.5;
    fZoneSizing.DesHeatVolFlow = 0.5;
    fZoneSizing.DesCoolCoilInTemp = 30.0;
    fZoneSizing.DesCoolCoilInHumRat = 0.01;
    fZoneSizing.DesHeatCoilInTemp = 20.0;
    fZoneSizing.DesHeatCoilInHumRat = 0.005;
    fZoneSizing.DesCoolLoad = 10000.0;
    fZoneSizing.DesHeatLoad = 10000.0;
    thisFanCoil.DesignHeatingCapacity = 10000.0;

    // test 1: fancoil unit cycling on/off at speed 1
    zSysEDemand.RemainingOutputReqToCoolSP = 2000.0;
    zSysEDemand.RemainingOutputReqToHeatSP = 2000.0;
    zSysEDemand.RemainingOutputRequired = 2000.0;
    QZnReq = 2000.0;
    QUnitOut = 0.0;
    QLatOut = 0.0;
    state->dataGlobal->BeginEnvrnFlag = true;
    InitFanCoilUnits(*state, FanCoilNum, ZoneNum);
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    Real64 expectedAirFlowRate = thisFanCoil.PLR * thisFanCoil.MaxAirMassFlow * thisFanCoil.LowSpeedRatio;
    // expect fan speed 1 and fan and coil cycling to meet load
    EXPECT_EQ(thisFanCoil.SpeedFanSel, 1);
    EXPECT_EQ(thisFanCoil.SpeedRatio, 0.0);
    EXPECT_NEAR(thisFanCoil.PLR, 0.662, 0.001);
    EXPECT_NEAR(QZnReq, QUnitOut, 1.0);
    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate, expectedAirFlowRate, 0.000001);

    // test 2: fancoil cycling between speed levels 1 and 2
    zSysEDemand.RemainingOutputReqToCoolSP = 4000.0;
    zSysEDemand.RemainingOutputReqToHeatSP = 4000.0;
    zSysEDemand.RemainingOutputRequired = 4000.0;
    QZnReq = 4000.0;
    QUnitOut = 0.0;
    QLatOut = 0.0;
    InitFanCoilUnits(*state, FanCoilNum, ZoneNum);
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    expectedAirFlowRate = (1.0 - thisFanCoil.SpeedRatio) * (thisFanCoil.LowSpeedRatio * thisFanCoil.MaxAirMassFlow) +
                          thisFanCoil.SpeedRatio * (thisFanCoil.MedSpeedRatio * thisFanCoil.MaxAirMassFlow);
    // expect fan speed 2 and fan and fancoil cycling b/n speed 1 and 2
    EXPECT_EQ(thisFanCoil.SpeedFanSel, 2);
    EXPECT_NEAR(thisFanCoil.PLR, 1.0, 0.001);
    EXPECT_NEAR(thisFanCoil.SpeedRatio, 0.323, 0.001);
    EXPECT_NEAR(QZnReq, QUnitOut, 1.0);
    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate, expectedAirFlowRate, 0.000001);

    // test 3: fancoil cycling between speed levels 2 and 3
    zSysEDemand.RemainingOutputReqToCoolSP = 8000.0;
    zSysEDemand.RemainingOutputReqToHeatSP = 8000.0;
    zSysEDemand.RemainingOutputRequired = 8000.0;
    QZnReq = 8000.0;
    QUnitOut = 0.0;
    QLatOut = 0.0;
    InitFanCoilUnits(*state, FanCoilNum, ZoneNum);
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    expectedAirFlowRate = (1.0 - thisFanCoil.SpeedRatio) * (thisFanCoil.MedSpeedRatio * thisFanCoil.MaxAirMassFlow) +
                          thisFanCoil.SpeedRatio * (1.0 * thisFanCoil.MaxAirMassFlow);
    // expect fan speed 3 and fan and fancoil cycling b/n speed 2 and 3
    EXPECT_EQ(thisFanCoil.SpeedFanSel, 3);
    EXPECT_NEAR(thisFanCoil.PLR, 1.0, 0.001);
    EXPECT_NEAR(thisFanCoil.SpeedRatio, 0.485, 0.001);
    EXPECT_NEAR(QZnReq, QUnitOut, 1.0);
    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate, expectedAirFlowRate, 0.000001);

    // test 4: expect fancoil to run at maximum speed / full capacity
    zSysEDemand.RemainingOutputReqToCoolSP = 10200.0;
    zSysEDemand.RemainingOutputReqToHeatSP = 10200.0;
    zSysEDemand.RemainingOutputRequired = 10200;
    QZnReq = 10200;
    QUnitOut = 0.0;
    QLatOut = 0.0;
    InitFanCoilUnits(*state, FanCoilNum, ZoneNum);
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    expectedAirFlowRate = thisFanCoil.MaxAirMassFlow;
    // expect fan speed 3 and fancoil running at max speed
    EXPECT_EQ(thisFanCoil.SpeedFanSel, 3);
    EXPECT_NEAR(thisFanCoil.PLR, 1.0, 0.001);
    EXPECT_NEAR(thisFanCoil.SpeedRatio, 1.0, 0.001);
    EXPECT_NEAR(10075.0, QUnitOut, 1.0);
    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate, expectedAirFlowRate, 0.000001);
}

TEST_F(EnergyPlusFixture, FanCoil_ElecHeatCoilMultiSpeedFanContFanMode)
{
    int FanCoilNum(1);
    int ZoneNum(1);
    bool FirstHVACIteration(false);
    bool ErrorsFound(false);
    Real64 QZnReq(0.0);
    Real64 ColdWaterMassFlowRate(0.0);
    Real64 QUnitOut(0.0);
    Real64 QLatOut(0.0);
    Real64 AirMassFlow(0.0);
    Real64 MaxAirMassFlow(0.0);

    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataWaterCoils->GetWaterCoilsInputFlag = true;
    state->dataGlobalNames->NumCoils = 0;
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->dataSize->CurZoneEqNum = 1;

    std::string const idf_objects = delimited_string({
        " Zone,",
        "  EAST ZONE,     !- Name",
        "  0,             !- Direction of Relative North { deg }",
        "  0,             !- X Origin { m }",
        "  0,             !- Y Origin { m }",
        "  0,             !- Z Origin { m }",
        "  1,             !- Type",
        "  1,             !- Multiplier",
        "  autocalculate, !- Ceiling Height { m }",
        "  autocalculate; !- Volume { m3 }",

        " ZoneHVAC:EquipmentConnections,",
        "  EAST ZONE,          !- Zone Name",
        "  Zone1Equipment,     !- Zone Conditioning Equipment List Name",
        "  Zone1Inlets,        !- Zone Air Inlet Node or NodeList Name",
        "  Zone1Exhausts,      !- Zone Air Exhaust Node or NodeList Name",
        "  Zone 1 Node,        !- Zone Air Node Name",
        "  Zone 1 Outlet Node; !- Zone Return Air Node Name",

        " ZoneHVAC:EquipmentList,",
        "  Zone1Equipment,           !- Name",
        "  SequentialLoad,           !- Load Distribution Scheme",
        "  ZoneHVAC:FourPipeFanCoil, !- Zone Equipment 1 Object Type",
        "  Zone1FanCoil,             !- Zone Equipment 1 Name",
        "  1,                        !- Zone Equipment 1 Cooling Sequence",
        "  1;                        !- Zone Equipment 1 Heating or No - Load Sequence",

        " NodeList,",
        "  Zone1Inlets,              !- Name",
        "  Zone1FanCoilAirOutletNode;!- Node 1 Name",

        " NodeList,",
        "  Zone1Exhausts,            !- Name",
        "  Zone1FanCoilAirInletNode; !- Node 1 Name",

        " OutdoorAir:NodeList,",
        "  Zone1FanCoilOAInNode;     !- Node or NodeList Name 1",

        " OutdoorAir:Mixer,",
        "  Zone1FanCoilOAMixer,      !- Name",
        "  Zone1FanCoilOAMixerOutletNode, !- Mixed Air Node Name",
        "  Zone1FanCoilOAInNode,     !- Outdoor Air Stream Node Name",
        "  Zone1FanCoilExhNode,      !- Relief Air Stream Node Name",
        "  Zone1FanCoilAirInletNode; !- Return Air Stream Node Name",

        " Schedule:Constant,",
        "  FanAndCoilAvailSched,     !- Name",
        "  FRACTION,                 !- Schedule Type",
        "  1;                        !- TimeStep Value",

        " ScheduleTypeLimits,",
        "  Fraction,                 !- Name",
        "  0.0,                      !- Lower Limit Value",
        "  1.0,                      !- Upper Limit Value",
        "  CONTINUOUS;               !- Numeric Type",

        " Fan:SystemModel,",
        "  Zone1FanCoilFan,         !- Name",
        "  FanAndCoilAvailSched,    !- Availability Schedule Name",
        "  Zone1FanCoilOAMixerOutletNode,  !- Air Inlet Node Name",
        "  Zone1FanCoilFanOutletNode,  !- Air Outlet Node Name",
        "  0.5,                     !- Design Maximum Air Flow Rate {m3/s}",
        "  Discrete,                !- Speed Control Method",
        "  0.0,                     !- Electric Power Minimum Flow Rate Fraction",
        "  75,                      !- Design Pressure Rise {Pa}",
        "  0.9,                     !- Motor Efficiency",
        "  1,                       !- Motor In Air Stream Fraction",
        "  ,                        !- Design Electric Power Consumption {W}",
        "  TotalEfficiencyAndPressure,  !- Design Power Sizing Method",
        "  ,                        !- Electric Power Per Unit Flow Rate {W/(m3/s)}",
        "  ,                        !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}",
        "  0.5,                     !- Fan Total Efficiency",
        "  ,                        !- Electric Power Function of Flow Fraction Curve Name",
        "  ,                        !- Night Ventilation Mode Pressure Rise {Pa}",
        "  ,                        !- Night Ventilation Mode Flow Fraction",
        "  ,                        !- Motor Loss Zone Name",
        "  ,                        !- Motor Loss Radiative Fraction",
        "  General,                 !- End-Use Subcategory",
        "  1,                       !- Number of Speeds",
        "  1.0,                     !- Speed 1 Flow Fraction",
        "  1.0;                     !- Speed 1 Electric Power Fraction",

        " Coil:Cooling:Water,",
        "  Zone1FanCoilCoolingCoil,  !- Name",
        "  FanAndCoilAvailSched,     !- Availability Schedule Namev",
        "  0.0002,                   !- Design Water Flow Rate { m3 / s }",
        "  0.5000,                   !- Design Air Flow Rate { m3 / s }",
        "  7.22,                     !- Design Inlet Water Temperature { Cv }",
        "  24.340,                   !- Design Inlet Air Temperature { C }",
        "  14.000,                   !- Design Outlet Air Temperature { C }",
        "  0.0095,                   !- Design Inlet Air Humidity Ratio { kgWater / kgDryAir }",
        "  0.0090,                   !- Design Outlet Air Humidity Ratio { kgWater / kgDryAir }",
        "  Zone1FanCoilChWInletNode, !- Water Inlet Node Name",
        "  Zone1FanCoilChWOutletNode,!- Water Outlet Node Name",
        "  Zone1FanCoilFanOutletNode,!- Air Inlet Node Name",
        "  Zone1FanCoilCCOutletNode, !- Air Outlet Node Name",
        "  SimpleAnalysis,           !- Type of Analysis",
        "  CrossFlow;                !- Heat Exchanger Configuration",

        " Coil:Heating:Electric,",
        "  Zone1FanCoilHeatingCoil,   !- Name",
        "  FanAndCoilAvailSched,      !- Availability Schedule Name",
        "  1,                         !- Efficiency",
        "  10000.0,                    !- Nominal Capacity {W}",
        "  Zone1FanCoilCCOutletNode,  !- Air Inlet Node Name",
        "  Zone1FanCoilAirOutletNode; !- Air Outlet Node Name",

        " ZoneHVAC:FourPipeFanCoil,",
        "  Zone1FanCoil,              !- Name",
        "  FanAndCoilAvailSched,      !- Availability Schedule Name",
        "  MultiSpeedFan,             !- Capacity Control Method",
        "  0.5,                       !- Maximum Supply Air Flow Rate { m3 / s }",
        "  0.3,                       !- Low Speed Supply Air Flow Ratio",
        "  0.6,                       !- Medium Speed Supply Air Flow Ratio",
        "  0.0,                       !- Maximum Outdoor Air Flow Rate { m3 / s }",
        "  FanAndCoilAvailSched,      !- Outdoor Air Schedule Name",
        "  Zone1FanCoilAirInletNode,  !- Air Inlet Node Name",
        "  Zone1FanCoilAirOutletNode, !- Air Outlet Node Name",
        "  OutdoorAir:Mixer,          !- Outdoor Air Mixer Object Type",
        "  Zone1FanCoilOAMixer,       !- Outdoor Air Mixer Name",
        "  Fan:SystemModel,           !- Supply Air Fan Object Type",
        "  Zone1FanCoilFan,           !- Supply Air Fan Name",
        "  Coil:Cooling:Water,        !- Cooling Coil Object Type",
        "  Zone1FanCoilCoolingCoil,   !- Cooling Coil Name",
        "  0.00014,                   !- Maximum Cold Water Flow Rate { m3 / s }",
        "  0.0,                       !- Minimum Cold Water Flow Rate { m3 / s }",
        "  0.001,                     !- Cooling Convergence Tolerance",
        "  Coil:Heating:Electric,     !- Heating Coil Object Type",
        "  Zone1FanCoilHeatingCoil,   !- Heating Coil Name",
        "  0.0,                       !- Maximum Hot Water Flow Rate { m3 / s }",
        "  0.0,                       !- Minimum Hot Water Flow Rate { m3 / s }",
        "  0.001,                     !- Heating Convergence Tolerance",
        "  ,                          !- Availability Manager List Name",
        "  ,                          !- Design Specification ZoneHVAC Sizing Object Name",
        "  ContsFanOperatingSch;      !- Supply Air Fan Operating Mode Schedule Name",

        " Schedule:Constant,",
        "  ContsFanOperatingSch,      !- Name",
        "  FRACTION,                  !- Schedule Type",
        "  1;                         !- TimeStep Value",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    EXPECT_EQ("EAST ZONE", state->dataHeatBal->Zone(1).Name);
    GetZoneEquipmentData(*state);
    GetFanCoilUnits(*state);
    auto &thisFanCoil(state->dataFanCoilUnits->FanCoil(1));
    EXPECT_ENUM_EQ(CCM::MultiSpeedFan, thisFanCoil.CapCtrlMeth_Num);
    EXPECT_EQ("OUTDOORAIR:MIXER", thisFanCoil.OAMixType);
    EXPECT_EQ("COIL:COOLING:WATER", thisFanCoil.CCoilType);
    EXPECT_EQ("COIL:HEATING:ELECTRIC", thisFanCoil.HCoilType);
    EXPECT_EQ((int)HVAC::FanType::SystemModel, (int)thisFanCoil.fanType);

    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    AirMassFlow = 0.60;
    MaxAirMassFlow = 0.60;
    ColdWaterMassFlowRate = 1.0;
    thisFanCoil.OutAirMassFlow = 0.0;
    thisFanCoil.MaxAirMassFlow = MaxAirMassFlow;
    state->dataLoopNodes->Node(thisFanCoil.OutsideAirNode).MassFlowRateMax = 0.0;
    state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRateMinAvail = AirMassFlow;
    state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRateMax = MaxAirMassFlow;
    state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRateMaxAvail = MaxAirMassFlow;
    // outside air mixer
    auto &MixerOA(state->dataMixedAir->OAMixer(1));
    state->dataLoopNodes->Node(MixerOA.RetNode).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(MixerOA.RetNode).MassFlowRateMax = MaxAirMassFlow;
    state->dataLoopNodes->Node(MixerOA.RetNode).Temp = 22.0;
    state->dataLoopNodes->Node(MixerOA.RetNode).Enthalpy = 36000;
    state->dataLoopNodes->Node(MixerOA.RetNode).HumRat =
        PsyWFnTdbH(*state, state->dataLoopNodes->Node(MixerOA.RetNode).Temp, state->dataLoopNodes->Node(MixerOA.RetNode).Enthalpy);
    state->dataLoopNodes->Node(MixerOA.InletNode).Temp = 10.0;
    state->dataLoopNodes->Node(MixerOA.InletNode).Enthalpy = 18000;
    state->dataLoopNodes->Node(MixerOA.InletNode).HumRat =
        PsyWFnTdbH(*state, state->dataLoopNodes->Node(MixerOA.InletNode).Temp, state->dataLoopNodes->Node(MixerOA.InletNode).Enthalpy);
    // chilled water coil
    auto &CWCoil(state->dataWaterCoils->WaterCoil(1));
    CWCoil.UACoilTotal = 470.0;
    CWCoil.UACoilExternal = 611.0;
    CWCoil.UACoilInternal = 2010.0;
    CWCoil.TotCoilOutsideSurfArea = 50.0;
    state->dataLoopNodes->Node(CWCoil.AirInletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(CWCoil.AirInletNodeNum).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(CWCoil.AirInletNodeNum).MassFlowRateMax = AirMassFlow;
    state->dataLoopNodes->Node(CWCoil.AirInletNodeNum).MassFlowRateMaxAvail = AirMassFlow;
    CWCoil.InletWaterMassFlowRate = ColdWaterMassFlowRate;
    CWCoil.MaxWaterMassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(CWCoil.WaterInletNodeNum).MassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(CWCoil.WaterInletNodeNum).MassFlowRateMaxAvail = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(CWCoil.WaterInletNodeNum).Temp = 6.0;
    state->dataLoopNodes->Node(CWCoil.WaterOutletNodeNum).MassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(CWCoil.WaterOutletNodeNum).MassFlowRateMaxAvail = ColdWaterMassFlowRate;

    // electric heating coil
    auto &eHCoil(state->dataHeatingCoils->HeatingCoil(1));
    state->dataLoopNodes->Node(eHCoil.AirInletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(eHCoil.AirInletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }
    // chilled water plant loop
    auto &CWLoop(state->dataPlnt->PlantLoop(1));
    CWLoop.Name = "ChilledWaterLoop";
    CWLoop.FluidName = "WATER";
    CWLoop.glycol = Fluid::GetWater(*state);
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = CWCoil.Name;
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type = DataPlant::PlantEquipmentType::CoilWaterCooling;
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = CWCoil.WaterInletNodeNum;
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = CWCoil.WaterOutletNodeNum;

    CWCoil.WaterPlantLoc.loopNum = 1;
    CWCoil.WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    CWCoil.WaterPlantLoc.branchNum = 1;
    CWCoil.WaterPlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, CWCoil.WaterPlantLoc);

    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(1);
    state->dataWaterCoils->MyUAAndFlowCalcFlag(1) = true;
    state->dataGlobal->DoingSizing = true;
    state->dataHVACGlobal->TurnFansOff = false;
    state->dataHVACGlobal->TurnFansOn = true;
    // heating mode tests
    state->dataFanCoilUnits->CoolingLoad = false;
    state->dataFanCoilUnits->HeatingLoad = true;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    auto &zSysEDemand(state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1));

    state->dataEnvrn->StdRhoAir = 1.2;
    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 21;
    state->dataGlobal->HourOfDay = 1;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    Sched::UpdateScheduleVals(*state);
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod.allocate(HVAC::NumOfSizingTypes);
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod = 0;
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->FinalZoneSizing.allocate(1);
    auto &fZoneSizing(state->dataSize->FinalZoneSizing(1));
    fZoneSizing.DesCoolVolFlow = 0.5;
    fZoneSizing.DesHeatVolFlow = 0.5;
    fZoneSizing.DesCoolCoilInTemp = 30.0;
    fZoneSizing.DesCoolCoilInHumRat = 0.01;
    fZoneSizing.DesHeatCoilInTemp = 20.0;
    fZoneSizing.DesHeatCoilInHumRat = 0.005;
    fZoneSizing.DesCoolLoad = 10000.0;
    fZoneSizing.DesHeatLoad = 10000.0;
    thisFanCoil.DesignHeatingCapacity = 10000.0;

    // test 1: fancoil unit cycling on/off at speed 1
    zSysEDemand.RemainingOutputReqToCoolSP = 2000.0;
    zSysEDemand.RemainingOutputReqToHeatSP = 2000.0;
    zSysEDemand.RemainingOutputRequired = 2000.0;
    QZnReq = 2000.0;
    QUnitOut = 0.0;
    QLatOut = 0.0;
    state->dataGlobal->BeginEnvrnFlag = true;
    InitFanCoilUnits(*state, FanCoilNum, ZoneNum);
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    Real64 expectedAirFlowRate = thisFanCoil.MaxAirMassFlow * thisFanCoil.LowSpeedRatio;
    // expect fan speed 1, fan runs continuously and only heating coil cycle on/off to meet load
    EXPECT_EQ(thisFanCoil.SpeedFanSel, 1);
    EXPECT_EQ(thisFanCoil.SpeedRatio, 0.0);
    EXPECT_NEAR(thisFanCoil.PLR, 0.659, 0.001);
    EXPECT_NEAR(QZnReq, QUnitOut, 1.0);
    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate, expectedAirFlowRate, 0.000001);

    // test 2: fancoil cycling between speed levels 1 and 2
    zSysEDemand.RemainingOutputReqToCoolSP = 4000.0;
    zSysEDemand.RemainingOutputReqToHeatSP = 4000.0;
    zSysEDemand.RemainingOutputRequired = 4000.0;
    QZnReq = 4000.0;
    QUnitOut = 0.0;
    QLatOut = 0.0;
    InitFanCoilUnits(*state, FanCoilNum, ZoneNum);
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    expectedAirFlowRate = (1.0 - thisFanCoil.SpeedRatio) * (thisFanCoil.LowSpeedRatio * thisFanCoil.MaxAirMassFlow) +
                          thisFanCoil.SpeedRatio * (thisFanCoil.MedSpeedRatio * thisFanCoil.MaxAirMassFlow);
    // expect fan speed 2 and fan and fancoil cycling b/n speed 1 and 2
    EXPECT_EQ(thisFanCoil.SpeedFanSel, 2);
    EXPECT_NEAR(thisFanCoil.PLR, 1.0, 0.001);
    EXPECT_NEAR(thisFanCoil.SpeedRatio, 0.323, 0.001);
    EXPECT_NEAR(QZnReq, QUnitOut, 1.0);
    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate, expectedAirFlowRate, 0.000001);

    // test 3: fancoil cycling between speed levels 2 and 3
    zSysEDemand.RemainingOutputReqToCoolSP = 8000.0;
    zSysEDemand.RemainingOutputReqToHeatSP = 8000.0;
    zSysEDemand.RemainingOutputRequired = 8000.0;
    QZnReq = 8000.0;
    QUnitOut = 0.0;
    QLatOut = 0.0;
    InitFanCoilUnits(*state, FanCoilNum, ZoneNum);
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    expectedAirFlowRate = (1.0 - thisFanCoil.SpeedRatio) * (thisFanCoil.MedSpeedRatio * thisFanCoil.MaxAirMassFlow) +
                          thisFanCoil.SpeedRatio * (1.0 * thisFanCoil.MaxAirMassFlow);
    // expect fan speed 3 and fan and fancoil cycling b/n speed 2 and 3
    EXPECT_EQ(thisFanCoil.SpeedFanSel, 3);
    EXPECT_NEAR(thisFanCoil.PLR, 1.0, 0.001);
    EXPECT_NEAR(thisFanCoil.SpeedRatio, 0.485, 0.001);
    EXPECT_NEAR(QZnReq, QUnitOut, 1.0);
    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate, expectedAirFlowRate, 0.000001);

    // test 4: expect fancoil to run at maximum speed / full capacity
    zSysEDemand.RemainingOutputReqToCoolSP = 10200.0;
    zSysEDemand.RemainingOutputReqToHeatSP = 10200.0;
    zSysEDemand.RemainingOutputRequired = 10200;
    QZnReq = 10200;
    QUnitOut = 0.0;
    QLatOut = 0.0;
    InitFanCoilUnits(*state, FanCoilNum, ZoneNum);
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    expectedAirFlowRate = thisFanCoil.MaxAirMassFlow;
    // expect fan speed 3 and fancoil running at max speed
    EXPECT_EQ(thisFanCoil.SpeedFanSel, 3);
    EXPECT_NEAR(thisFanCoil.PLR, 1.0, 0.001);
    EXPECT_NEAR(thisFanCoil.SpeedRatio, 1.0, 0.001);
    EXPECT_NEAR(10075.0, QUnitOut, 1.0);
    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate, expectedAirFlowRate, 0.000001);
}

TEST_F(EnergyPlusFixture, FanCoil_CalcFanCoilElecHeatCoilPLRResidual)
{
    int FanCoilNum(1);
    int ZoneNum(1);
    bool FirstHVACIteration(false);
    bool ErrorsFound(false);
    Real64 QZnReq(0.0);
    Real64 ColdWaterMassFlowRate(0.0);
    Real64 QUnitOut(0.0);
    Real64 QLatOut(0.0);
    Real64 AirMassFlow(0.0);
    Real64 MaxAirMassFlow(0.0);

    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataWaterCoils->GetWaterCoilsInputFlag = true;
    state->dataGlobalNames->NumCoils = 0;
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->dataSize->CurZoneEqNum = 1;

    std::string const idf_objects = delimited_string({
        " Zone,",
        "  EAST ZONE,     !- Name",
        "  0,             !- Direction of Relative North { deg }",
        "  0,             !- X Origin { m }",
        "  0,             !- Y Origin { m }",
        "  0,             !- Z Origin { m }",
        "  1,             !- Type",
        "  1,             !- Multiplier",
        "  autocalculate, !- Ceiling Height { m }",
        "  autocalculate; !- Volume { m3 }",

        " ZoneHVAC:EquipmentConnections,",
        "  EAST ZONE,          !- Zone Name",
        "  Zone1Equipment,     !- Zone Conditioning Equipment List Name",
        "  Zone1Inlets,        !- Zone Air Inlet Node or NodeList Name",
        "  Zone1Exhausts,      !- Zone Air Exhaust Node or NodeList Name",
        "  Zone 1 Node,        !- Zone Air Node Name",
        "  Zone 1 Outlet Node; !- Zone Return Air Node Name",

        " ZoneHVAC:EquipmentList,",
        "  Zone1Equipment,           !- Name",
        "  SequentialLoad,           !- Load Distribution Scheme",
        "  ZoneHVAC:FourPipeFanCoil, !- Zone Equipment 1 Object Type",
        "  Zone1FanCoil,             !- Zone Equipment 1 Name",
        "  1,                        !- Zone Equipment 1 Cooling Sequence",
        "  1;                        !- Zone Equipment 1 Heating or No - Load Sequence",

        " NodeList,",
        "  Zone1Inlets,              !- Name",
        "  Zone1FanCoilAirOutletNode;!- Node 1 Name",

        " NodeList,",
        "  Zone1Exhausts,            !- Name",
        "  Zone1FanCoilAirInletNode; !- Node 1 Name",

        " OutdoorAir:NodeList,",
        "  Zone1FanCoilOAInNode;     !- Node or NodeList Name 1",

        " OutdoorAir:Mixer,",
        "  Zone1FanCoilOAMixer,      !- Name",
        "  Zone1FanCoilOAMixerOutletNode, !- Mixed Air Node Name",
        "  Zone1FanCoilOAInNode,     !- Outdoor Air Stream Node Name",
        "  Zone1FanCoilExhNode,      !- Relief Air Stream Node Name",
        "  Zone1FanCoilAirInletNode; !- Return Air Stream Node Name",

        " Schedule:Constant,",
        "  FanAndCoilAvailSched,     !- Name",
        "  FRACTION,                 !- Schedule Type",
        "  1;                        !- TimeStep Value",

        " ScheduleTypeLimits,",
        "  Fraction,                 !- Name",
        "  0.0,                      !- Lower Limit Value",
        "  1.0,                      !- Upper Limit Value",
        "  CONTINUOUS;               !- Numeric Type",

        " Fan:SystemModel,",
        "  Zone1FanCoilFan,         !- Name",
        "  FanAndCoilAvailSched,    !- Availability Schedule Name",
        "  Zone1FanCoilOAMixerOutletNode,  !- Air Inlet Node Name",
        "  Zone1FanCoilFanOutletNode,  !- Air Outlet Node Name",
        "  0.5,                     !- Design Maximum Air Flow Rate {m3/s}",
        "  Discrete,                !- Speed Control Method",
        "  0.0,                     !- Electric Power Minimum Flow Rate Fraction",
        "  75,                      !- Design Pressure Rise {Pa}",
        "  0.9,                     !- Motor Efficiency",
        "  1,                       !- Motor In Air Stream Fraction",
        "  ,                        !- Design Electric Power Consumption {W}",
        "  TotalEfficiencyAndPressure,  !- Design Power Sizing Method",
        "  ,                        !- Electric Power Per Unit Flow Rate {W/(m3/s)}",
        "  ,                        !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}",
        "  0.5,                     !- Fan Total Efficiency",
        "  ,                        !- Electric Power Function of Flow Fraction Curve Name",
        "  ,                        !- Night Ventilation Mode Pressure Rise {Pa}",
        "  ,                        !- Night Ventilation Mode Flow Fraction",
        "  ,                        !- Motor Loss Zone Name",
        "  ,                        !- Motor Loss Radiative Fraction",
        "  General,                 !- End-Use Subcategory",
        "  1,                       !- Number of Speeds",
        "  1.0,                     !- Speed 1 Flow Fraction",
        "  1.0;                     !- Speed 1 Electric Power Fraction",

        " Coil:Cooling:Water,",
        "  Zone1FanCoilCoolingCoil,  !- Name",
        "  FanAndCoilAvailSched,     !- Availability Schedule Namev",
        "  0.0002,                   !- Design Water Flow Rate { m3 / s }",
        "  0.5000,                   !- Design Air Flow Rate { m3 / s }",
        "  7.22,                     !- Design Inlet Water Temperature { Cv }",
        "  24.340,                   !- Design Inlet Air Temperature { C }",
        "  14.000,                   !- Design Outlet Air Temperature { C }",
        "  0.0095,                   !- Design Inlet Air Humidity Ratio { kgWater / kgDryAir }",
        "  0.0090,                   !- Design Outlet Air Humidity Ratio { kgWater / kgDryAir }",
        "  Zone1FanCoilChWInletNode, !- Water Inlet Node Name",
        "  Zone1FanCoilChWOutletNode,!- Water Outlet Node Name",
        "  Zone1FanCoilFanOutletNode,!- Air Inlet Node Name",
        "  Zone1FanCoilCCOutletNode, !- Air Outlet Node Name",
        "  SimpleAnalysis,           !- Type of Analysis",
        "  CrossFlow;                !- Heat Exchanger Configuration",

        " Coil:Heating:Electric,",
        "  Zone1FanCoilHeatingCoil,   !- Name",
        "  FanAndCoilAvailSched,      !- Availability Schedule Name",
        "  1,                         !- Efficiency",
        "  10000.0,                    !- Nominal Capacity {W}",
        "  Zone1FanCoilCCOutletNode,  !- Air Inlet Node Name",
        "  Zone1FanCoilAirOutletNode; !- Air Outlet Node Name",

        " ZoneHVAC:FourPipeFanCoil,",
        "  Zone1FanCoil,              !- Name",
        "  FanAndCoilAvailSched,      !- Availability Schedule Name",
        "  MultiSpeedFan,             !- Capacity Control Method",
        "  0.5,                       !- Maximum Supply Air Flow Rate { m3 / s }",
        "  0.3,                       !- Low Speed Supply Air Flow Ratio",
        "  0.6,                       !- Medium Speed Supply Air Flow Ratio",
        "  0.0,                       !- Maximum Outdoor Air Flow Rate { m3 / s }",
        "  FanAndCoilAvailSched,      !- Outdoor Air Schedule Name",
        "  Zone1FanCoilAirInletNode,  !- Air Inlet Node Name",
        "  Zone1FanCoilAirOutletNode, !- Air Outlet Node Name",
        "  OutdoorAir:Mixer,          !- Outdoor Air Mixer Object Type",
        "  Zone1FanCoilOAMixer,       !- Outdoor Air Mixer Name",
        "  Fan:SystemModel,           !- Supply Air Fan Object Type",
        "  Zone1FanCoilFan,           !- Supply Air Fan Name",
        "  Coil:Cooling:Water,        !- Cooling Coil Object Type",
        "  Zone1FanCoilCoolingCoil,   !- Cooling Coil Name",
        "  0.00014,                   !- Maximum Cold Water Flow Rate { m3 / s }",
        "  0.0,                       !- Minimum Cold Water Flow Rate { m3 / s }",
        "  0.001,                     !- Cooling Convergence Tolerance",
        "  Coil:Heating:Electric,     !- Heating Coil Object Type",
        "  Zone1FanCoilHeatingCoil,   !- Heating Coil Name",
        "  0.0,                       !- Maximum Hot Water Flow Rate { m3 / s }",
        "  0.0,                       !- Minimum Hot Water Flow Rate { m3 / s }",
        "  0.001,                     !- Heating Convergence Tolerance",
        "  ,                          !- Availability Manager List Name",
        "  ,                          !- Design Specification ZoneHVAC Sizing Object Name",
        "  ContsFanOperatingSch;      !- Supply Air Fan Operating Mode Schedule Name",

        " Schedule:Constant,",
        "  ContsFanOperatingSch,      !- Name",
        "  FRACTION,                  !- Schedule Type",
        "  1;                         !- TimeStep Value",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    EXPECT_EQ("EAST ZONE", state->dataHeatBal->Zone(1).Name);
    GetZoneEquipmentData(*state);
    GetFanCoilUnits(*state);
    auto &thisFanCoil(state->dataFanCoilUnits->FanCoil(1));
    EXPECT_ENUM_EQ(CCM::MultiSpeedFan, thisFanCoil.CapCtrlMeth_Num);
    EXPECT_EQ("OUTDOORAIR:MIXER", thisFanCoil.OAMixType);
    EXPECT_EQ("COIL:COOLING:WATER", thisFanCoil.CCoilType);
    EXPECT_EQ("COIL:HEATING:ELECTRIC", thisFanCoil.HCoilType);
    EXPECT_EQ((int)HVAC::FanType::SystemModel, (int)thisFanCoil.fanType);

    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    AirMassFlow = 0.60;
    MaxAirMassFlow = 0.60;
    ColdWaterMassFlowRate = 1.0;
    thisFanCoil.OutAirMassFlow = 0.0;
    thisFanCoil.MaxAirMassFlow = MaxAirMassFlow;
    state->dataLoopNodes->Node(thisFanCoil.OutsideAirNode).MassFlowRateMax = 0.0;
    state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRateMinAvail = AirMassFlow;
    state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRateMax = MaxAirMassFlow;
    state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRateMaxAvail = MaxAirMassFlow;
    // outside air mixer
    auto &MixerOA(state->dataMixedAir->OAMixer(1));
    state->dataLoopNodes->Node(MixerOA.RetNode).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(MixerOA.RetNode).MassFlowRateMax = MaxAirMassFlow;
    state->dataLoopNodes->Node(MixerOA.RetNode).Temp = 22.0;
    state->dataLoopNodes->Node(MixerOA.RetNode).Enthalpy = 36000;
    state->dataLoopNodes->Node(MixerOA.RetNode).HumRat =
        PsyWFnTdbH(*state, state->dataLoopNodes->Node(MixerOA.RetNode).Temp, state->dataLoopNodes->Node(MixerOA.RetNode).Enthalpy);
    state->dataLoopNodes->Node(MixerOA.InletNode).Temp = 10.0;
    state->dataLoopNodes->Node(MixerOA.InletNode).Enthalpy = 18000;
    state->dataLoopNodes->Node(MixerOA.InletNode).HumRat =
        PsyWFnTdbH(*state, state->dataLoopNodes->Node(MixerOA.InletNode).Temp, state->dataLoopNodes->Node(MixerOA.InletNode).Enthalpy);
    // chilled water coil
    auto &CWCoil(state->dataWaterCoils->WaterCoil(1));
    CWCoil.UACoilTotal = 470.0;
    CWCoil.UACoilExternal = 611.0;
    CWCoil.UACoilInternal = 2010.0;
    CWCoil.TotCoilOutsideSurfArea = 50.0;
    state->dataLoopNodes->Node(CWCoil.AirInletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(CWCoil.AirInletNodeNum).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(CWCoil.AirInletNodeNum).MassFlowRateMax = AirMassFlow;
    state->dataLoopNodes->Node(CWCoil.AirInletNodeNum).MassFlowRateMaxAvail = AirMassFlow;
    CWCoil.InletWaterMassFlowRate = ColdWaterMassFlowRate;
    CWCoil.MaxWaterMassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(CWCoil.WaterInletNodeNum).MassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(CWCoil.WaterInletNodeNum).MassFlowRateMaxAvail = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(CWCoil.WaterInletNodeNum).Temp = 6.0;
    state->dataLoopNodes->Node(CWCoil.WaterOutletNodeNum).MassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(CWCoil.WaterOutletNodeNum).MassFlowRateMaxAvail = ColdWaterMassFlowRate;

    // electric heating coil
    auto &eHCoil(state->dataHeatingCoils->HeatingCoil(1));
    state->dataLoopNodes->Node(eHCoil.AirInletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(eHCoil.AirInletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }
    // chilled water plant loop
    auto &CWLoop(state->dataPlnt->PlantLoop(1));
    CWLoop.Name = "ChilledWaterLoop";
    CWLoop.FluidName = "WATER";
    CWLoop.glycol = Fluid::GetWater(*state);
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = CWCoil.Name;
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type = DataPlant::PlantEquipmentType::CoilWaterCooling;
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = CWCoil.WaterInletNodeNum;
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = CWCoil.WaterOutletNodeNum;

    // Set plant location links now that plant objects are created
    CWCoil.WaterPlantLoc.loopNum = 1;
    CWCoil.WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    CWCoil.WaterPlantLoc.branchNum = 1;
    CWCoil.WaterPlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, CWCoil.WaterPlantLoc);

    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(1);
    state->dataWaterCoils->MyUAAndFlowCalcFlag(1) = true;
    state->dataGlobal->DoingSizing = true;
    state->dataHVACGlobal->TurnFansOff = false;
    state->dataHVACGlobal->TurnFansOn = true;
    // heating mode tests
    state->dataFanCoilUnits->CoolingLoad = false;
    state->dataFanCoilUnits->HeatingLoad = true;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    auto &zSysEDemand(state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1));

    state->dataEnvrn->StdRhoAir = 1.2;
    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 21;
    state->dataGlobal->HourOfDay = 1;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    Sched::UpdateScheduleVals(*state);
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod.allocate(HVAC::NumOfSizingTypes);
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod = 0;
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->FinalZoneSizing.allocate(1);
    auto &fZoneSizing(state->dataSize->FinalZoneSizing(1));
    fZoneSizing.DesCoolVolFlow = 0.5;
    fZoneSizing.DesHeatVolFlow = 0.5;
    fZoneSizing.DesCoolCoilInTemp = 30.0;
    fZoneSizing.DesCoolCoilInHumRat = 0.01;
    fZoneSizing.DesHeatCoilInTemp = 20.0;
    fZoneSizing.DesHeatCoilInHumRat = 0.005;
    fZoneSizing.DesCoolLoad = 10000.0;
    fZoneSizing.DesHeatLoad = 10000.0;
    thisFanCoil.DesignHeatingCapacity = 10000.0;

    // test 1: fancoil unit cycling on/off at speed 1
    zSysEDemand.RemainingOutputReqToCoolSP = 2000.0;
    zSysEDemand.RemainingOutputReqToHeatSP = 2000.0;
    zSysEDemand.RemainingOutputRequired = 2000.0;
    int InletNode = thisFanCoil.AirInNode;
    Real64 QUnitOutMaxLS = 0.0; // low speed maximum output

    InitFanCoilUnits(*state, FanCoilNum, ZoneNum);
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);

    state->dataFanCoilUnits->FanCoil(FanCoilNum).SpeedFanSel = 1;
    state->dataFanCoilUnits->FanCoil(FanCoilNum).SpeedFanRatSel = state->dataFanCoilUnits->FanCoil(FanCoilNum).LowSpeedRatio;
    state->dataFanCoilUnits->FanFlowRatio = state->dataFanCoilUnits->FanCoil(FanCoilNum).SpeedFanRatSel;
    AirMassFlow = state->dataFanCoilUnits->FanCoil(FanCoilNum).LowSpeedRatio * state->dataFanCoilUnits->FanCoil(FanCoilNum).MaxAirMassFlow;
    state->dataLoopNodes->Node(InletNode).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(InletNode).MassFlowRateMax = AirMassFlow;
    state->dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = AirMassFlow;
    state->dataLoopNodes->Node(InletNode).MassFlowRateMinAvail = AirMassFlow;
    Calc4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOutMaxLS, _, 1.0);
    EXPECT_NEAR(QUnitOutMaxLS, 3022.5, 1.0);

    int MaxIter = 10;
    int SolFlag = 0;
    Real64 CyclingRatio = 1.0;
    // test 1: fan runs continuously at low speed and
    // only heating coil cycles On/Off to meet load
    QZnReq = 2000.0;
    auto f = [this, FanCoilNum, FirstHVACIteration, ZoneNum, QZnReq](Real64 const CyclingR) {
        return CalcFanCoilHeatCoilPLRResidual(*this->state, CyclingR, FanCoilNum, FirstHVACIteration, ZoneNum, QZnReq);
    };
    General::SolveRoot(*state, 0.001, MaxIter, SolFlag, CyclingRatio, f, 0.0, 1.0);
    Real64 expectedAirFlowRate = thisFanCoil.MaxAirMassFlow * thisFanCoil.LowSpeedRatio;
    EXPECT_EQ(thisFanCoil.SpeedFanSel, 1);
    EXPECT_EQ(state->dataLoopNodes->Node(InletNode).MassFlowRate, expectedAirFlowRate);
    EXPECT_NEAR(CyclingRatio, 0.659, 0.001);
    // test 2: fan runs continuously at low speed and only
    // the heating coil cycles on/off to meet reduced load
    zSysEDemand.RemainingOutputReqToCoolSP = 1000.0;
    zSysEDemand.RemainingOutputReqToHeatSP = 1000.0;
    zSysEDemand.RemainingOutputRequired = 1000.0;
    QZnReq = 1000.0;
    auto f2 = [this, FanCoilNum, FirstHVACIteration, ZoneNum, QZnReq](Real64 const CyclingR) {
        return CalcFanCoilHeatCoilPLRResidual(*this->state, CyclingR, FanCoilNum, FirstHVACIteration, ZoneNum, QZnReq);
    };
    General::SolveRoot(*state, 0.001, MaxIter, SolFlag, CyclingRatio, f2, 0.0, 1.0);
    expectedAirFlowRate = thisFanCoil.MaxAirMassFlow * thisFanCoil.LowSpeedRatio;
    EXPECT_EQ(thisFanCoil.SpeedFanSel, 1);
    EXPECT_EQ(state->dataLoopNodes->Node(InletNode).MassFlowRate, expectedAirFlowRate);
    EXPECT_NEAR(CyclingRatio, 0.326, 0.001);
}

TEST_F(EnergyPlusFixture, FanCoil_ElectricHeatingCoilASHRAE90VariableFan)
{
    int FanCoilNum(1);
    int ZoneNum(1);
    bool FirstHVACIteration(false);
    bool ErrorsFound(false);
    Real64 QZnReq(0.0);
    Real64 ColdWaterMassFlowRate(0.0);
    Real64 QUnitOut(0.0);
    Real64 QLatOut(0.0);
    Real64 AirMassFlow(0.0);
    Real64 MaxAirMassFlow(0.0);

    std::string const idf_objects = delimited_string({
        " Zone,",
        "  WEST ZONE,                 !- Name",
        "  0,                         !- Direction of Relative North { deg }",
        "  0,                         !- X Origin { m }",
        "  0,                         !- Y Origin { m }",
        "  0,                         !- Z Origin { m }",
        "  1,                         !- Type",
        "  1,                         !- Multiplier",
        "  autocalculate,             !- Ceiling Height { m }",
        "  autocalculate;             !- Volume { m3 }",

        " ZoneHVAC:EquipmentConnections,",
        "  WEST ZONE,                 !- Zone Name",
        "  ZoneEquipment,             !- Zone Conditioning Equipment List Name",
        "  FanCoilAirOutletNode,      !- Zone Air Inlet Node or NodeList Name",
        "  FanCoilAirInletNode,       !- Zone Air Exhaust Node or NodeList Name",
        "  Zone Air Node,             !- Zone Air Node Name",
        "  Zone Air Outlet Node;      !- Zone Return Air Node Name",

        " ZoneHVAC:EquipmentList,",
        "  ZoneEquipment,             !- Name",
        "  SequentialLoad,            !- Load Distribution Scheme",
        "  ZoneHVAC:FourPipeFanCoil,  !- Zone Equipment 1 Object Type",
        "  ZoneFanCoil,               !- Zone Equipment 1 Name",
        "  1,                         !- Zone Equipment 1 Cooling Sequence",
        "  1;                         !- Zone Equipment 1 Heating or No - Load Sequence",

        " ZoneHVAC:FourPipeFanCoil,",
        "  ZoneFanCoil,               !- Name",
        "  FanAndCoilAvailSched,      !- Availability Schedule Name",
        "  ASHRAE90VariableFan,       !- Capacity Control Method",
        "  0.5,                       !- Maximum Supply Air Flow Rate { m3 / s }",
        "  0.3,                       !- Low Speed Supply Air Flow Ratio",
        "  0.6,                       !- Medium Speed Supply Air Flow Ratio",
        "  0.0,                       !- Maximum Outdoor Air Flow Rate { m3 / s }",
        "  FanAndCoilAvailSched,      !- Outdoor Air Schedule Name",
        "  FanCoilAirInletNode,       !- Air Inlet Node Name",
        "  FanCoilAirOutletNode,      !- Air Outlet Node Name",
        "  OutdoorAir:Mixer,          !- Outdoor Air Mixer Object Type",
        "  FanCoilOAMixer,            !- Outdoor Air Mixer Name",
        "  Fan:OnOff,                 !- Supply Air Fan Object Type",
        "  FanCoilFan,                !- Supply Air Fan Name",
        "  Coil:Cooling:Water,        !- Cooling Coil Object Type",
        "  FanCoilCoolingCoil,        !- Cooling Coil Name",
        "  0.00014,                   !- Maximum Cold Water Flow Rate { m3 / s }",
        "  0.0,                       !- Minimum Cold Water Flow Rate { m3 / s }",
        "  0.001,                     !- Cooling Convergence Tolerance",
        "  Coil:Heating:Electric,     !- Heating Coil Object Type",
        "  FanCoilElecHeatingCoil,    !- Heating Coil Name",
        "  0.0,                       !- Maximum Hot Water Flow Rate { m3 / s }",
        "  0.0,                       !- Minimum Hot Water Flow Rate { m3 / s }",
        "  0.001,                     !- Heating Convergence Tolerance",
        "  ,                          !- Availability Manager List Name",
        "  ;                          !- Design Specification ZoneHVAC Sizing Object Name",

        " OutdoorAir:NodeList,",
        "  FanCoilOAInNode;           !- Node or NodeList Name 1",

        " OutdoorAir:Mixer,",
        "  FanCoilOAMixer,            !- Name",
        "  FanCoilOAMixerOutletNode,  !- Mixed Air Node Name",
        "  FanCoilOAInNode,           !- Outdoor Air Stream Node Name",
        "  FanCoilExhNode,            !- Relief Air Stream Node Name",
        "  FanCoilAirInletNode;       !- Return Air Stream Node Name",

        " Fan:OnOff,",
        "  FanCoilFan,                !- Name",
        "  FanAndCoilAvailSched,      !- Availability Schedule Name",
        "  0.5,                       !- Fan Total Efficiency",
        "  75,                        !- Design Pressure Rise {Pa}",
        "  0.5,                       !- Maximum Air Flow Rate {m3/s}",
        "  0.9,                       !- Motor Efficiency",
        "  1,                         !- Motor In Air Stream Fraction",
        "  FanCoilOAMixerOutletNode,  !- Air Inlet Node Name",
        "  FanCoilFanOutletNode;      !- Air Outlet Node Name",

        " Coil:Cooling:Water,",
        "  FanCoilCoolingCoil,        !- Name",
        "  FanAndCoilAvailSched,      !- Availability Schedule Namev",
        "  0.0002,                    !- Design Water Flow Rate { m3 / s }",
        "  0.5000,                    !- Design Air Flow Rate { m3 / s }",
        "  7.22,                      !- Design Inlet Water Temperature { Cv }",
        "  24.340,                    !- Design Inlet Air Temperature { C }",
        "  14.000,                    !- Design Outlet Air Temperature { C }",
        "  0.0095,                    !- Design Inlet Air Humidity Ratio { kgWater / kgDryAir }",
        "  0.0090,                    !- Design Outlet Air Humidity Ratio { kgWater / kgDryAir }",
        "  FanCoilChWInletNode,       !- Water Inlet Node Name",
        "  FanCoilChWOutletNode,      !- Water Outlet Node Name",
        "  FanCoilFanOutletNode,      !- Air Inlet Node Name",
        "  FanCoilCCOutletNode,       !- Air Outlet Node Name",
        "  SimpleAnalysis,            !- Type of Analysis",
        "  CrossFlow;                 !- Heat Exchanger Configuration",

        " Coil:Heating:Electric,",
        "  FanCoilElecHeatingCoil,    !- Name",
        "  FanAndCoilAvailSched,      !- Availability Schedule Name",
        "  1,                         !- Efficiency",
        "  6000.0,                    !- Nominal Capacity {W}",
        "  FanCoilCCOutletNode,       !- Air Inlet Node Name",
        "  FanCoilAirOutletNode;      !- Air Outlet Node Name",

        " Schedule:Constant,",
        "  FanAndCoilAvailSched,      !- Name",
        "  FRACTION,                  !- Schedule Type",
        "  1;                         !- TimeStep Value",

        " ScheduleTypeLimits,",
        "  Fraction,                  !- Name",
        "  0.0,                       !- Lower Limit Value",
        "  1.0,                       !- Upper Limit Value",
        "  CONTINUOUS;                !- Numeric Type",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataWaterCoils->GetWaterCoilsInputFlag = true;
    // NumCoils = 0;
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->dataSize->CurZoneEqNum = 1;

    state->init_state(*state);

    GetZoneData(*state, ErrorsFound);
    EXPECT_EQ("WEST ZONE", state->dataHeatBal->Zone(1).Name);
    GetZoneEquipmentData(*state);
    GetFanCoilUnits(*state);
    auto &thisFanCoil(state->dataFanCoilUnits->FanCoil(1));
    EXPECT_ENUM_EQ(CCM::ASHRAE, thisFanCoil.CapCtrlMeth_Num);
    EXPECT_EQ("OUTDOORAIR:MIXER", thisFanCoil.OAMixType);
    EXPECT_EQ((int)HVAC::FanType::OnOff, (int)thisFanCoil.fanType);
    EXPECT_EQ("COIL:COOLING:WATER", thisFanCoil.CCoilType);
    EXPECT_EQ("COIL:HEATING:ELECTRIC", thisFanCoil.HCoilType);

    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    AirMassFlow = 0.60;
    MaxAirMassFlow = 0.60;
    ColdWaterMassFlowRate = 1.0;
    thisFanCoil.OutAirMassFlow = 0.0;
    thisFanCoil.MaxAirMassFlow = MaxAirMassFlow;
    // outside air mixer
    auto &MixerOA(state->dataMixedAir->OAMixer(1));
    state->dataLoopNodes->Node(MixerOA.RetNode).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(MixerOA.RetNode).MassFlowRateMax = MaxAirMassFlow;
    state->dataLoopNodes->Node(MixerOA.RetNode).Temp = 20.0;
    state->dataLoopNodes->Node(MixerOA.RetNode).Enthalpy = 36000;
    state->dataLoopNodes->Node(MixerOA.RetNode).HumRat =
        PsyWFnTdbH(*state, state->dataLoopNodes->Node(MixerOA.RetNode).Temp, state->dataLoopNodes->Node(MixerOA.RetNode).Enthalpy);
    state->dataLoopNodes->Node(MixerOA.InletNode).Temp = 10.0;
    state->dataLoopNodes->Node(MixerOA.InletNode).Enthalpy = 18000;
    state->dataLoopNodes->Node(MixerOA.InletNode).HumRat =
        PsyWFnTdbH(*state, state->dataLoopNodes->Node(MixerOA.InletNode).Temp, state->dataLoopNodes->Node(MixerOA.InletNode).Enthalpy);
    // chilled water coil
    auto &CWCoil(state->dataWaterCoils->WaterCoil(1));
    CWCoil.UACoilTotal = 470.0;
    CWCoil.UACoilExternal = 611.0;
    CWCoil.UACoilInternal = 2010.0;
    CWCoil.TotCoilOutsideSurfArea = 50.0;
    state->dataLoopNodes->Node(CWCoil.AirInletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(CWCoil.AirInletNodeNum).MassFlowRateMax = AirMassFlow;
    state->dataLoopNodes->Node(CWCoil.AirInletNodeNum).MassFlowRateMaxAvail = AirMassFlow;
    CWCoil.InletWaterMassFlowRate = ColdWaterMassFlowRate;
    CWCoil.MaxWaterMassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(CWCoil.WaterInletNodeNum).MassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(CWCoil.WaterInletNodeNum).MassFlowRateMaxAvail = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(CWCoil.WaterInletNodeNum).Temp = 6.0;
    // electric heating coil
    auto &eHCoil(state->dataHeatingCoils->HeatingCoil(1));
    state->dataLoopNodes->Node(eHCoil.AirInletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(eHCoil.AirInletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }
    // chilled water plant loop
    auto &CWLoop(state->dataPlnt->PlantLoop(1));
    CWLoop.Name = "ChilledWaterLoop";
    CWLoop.FluidName = "WATER";
    CWLoop.glycol = Fluid::GetWater(*state);
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = CWCoil.Name;
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type = DataPlant::PlantEquipmentType::CoilWaterCooling;
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = CWCoil.WaterInletNodeNum;
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = CWCoil.WaterOutletNodeNum;

    // Set plantLocation links now that plant objects are created
    CWCoil.WaterPlantLoc.loopNum = 1;
    CWCoil.WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    CWCoil.WaterPlantLoc.branchNum = 1;
    CWCoil.WaterPlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, CWCoil.WaterPlantLoc);

    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(1);
    state->dataWaterCoils->MyUAAndFlowCalcFlag(1) = true;
    state->dataGlobal->DoingSizing = true;
    state->dataHVACGlobal->TurnFansOff = false;
    state->dataHVACGlobal->TurnFansOn = true;
    // heating mode tests
    state->dataFanCoilUnits->CoolingLoad = false;
    state->dataFanCoilUnits->HeatingLoad = true;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    auto &zSysEDemand(state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1));

    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 21;
    state->dataGlobal->HourOfDay = 1;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    Sched::UpdateScheduleVals(*state);
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(1) = false;
    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataHeatBalFanSys->TempControlType(1) = HVAC::SetptType::DualHeatCool;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataGlobal->SysSizingCalc = true;
    thisFanCoil.DesignHeatingCapacity = 6000.0;

    // test 1: load larger than fancoil full capacity
    zSysEDemand.RemainingOutputReqToCoolSP = 10000.0;
    zSysEDemand.RemainingOutputReqToHeatSP = 10000.0;
    zSysEDemand.RemainingOutputRequired = 10000.0;
    QZnReq = 10000.0;
    QUnitOut = 0.0;
    QLatOut = 0.0;
    state->dataGlobal->BeginEnvrnFlag = true;
    InitFanCoilUnits(*state, FanCoilNum, ZoneNum);
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    // expect output full capacity
    EXPECT_EQ(state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRate, thisFanCoil.MaxAirMassFlow);
    EXPECT_EQ(thisFanCoil.PLR, 1.0);
    EXPECT_NEAR(6075.0, QUnitOut, 1.0);

    // test 2: load smaller than fancoil full capacity
    zSysEDemand.RemainingOutputReqToCoolSP = 3000.0;
    zSysEDemand.RemainingOutputReqToHeatSP = 3000.0;
    zSysEDemand.RemainingOutputRequired = 3000.0;
    QZnReq = 3000.0;
    QUnitOut = 0.0;
    QLatOut = 0.0;
    state->dataGlobal->BeginEnvrnFlag = true;
    InitFanCoilUnits(*state, FanCoilNum, ZoneNum);
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    // expect part load operation with about 3000W output
    EXPECT_NEAR(state->dataLoopNodes->Node(thisFanCoil.AirInNode).MassFlowRate, thisFanCoil.MaxAirMassFlow, 0.00001);
    EXPECT_NEAR(thisFanCoil.PLR, 0.487, 0.001);
    EXPECT_NEAR(3000.0, QUnitOut, 1.0);
}

} // namespace EnergyPlus
