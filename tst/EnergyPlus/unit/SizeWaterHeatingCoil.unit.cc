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

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimAirServingZones.hh>
#include <EnergyPlus/SingleDuct.hh>
#include <EnergyPlus/SizingManager.hh>
#include <EnergyPlus/WaterCoils.hh>
#include <EnergyPlus/ZoneAirLoopEquipmentManager.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::GlobalNames;
using namespace EnergyPlus::DataHeatBalance;
using namespace EnergyPlus::DataPlant;
using namespace EnergyPlus::DataSizing;
using namespace EnergyPlus::DataZoneEquipment;
using namespace EnergyPlus::HeatBalanceManager;
using namespace EnergyPlus::Psychrometrics;
using namespace EnergyPlus::SimAirServingZones;
using namespace EnergyPlus::SingleDuct;
using namespace EnergyPlus::SizingManager;
using namespace EnergyPlus::WaterCoils;
using namespace EnergyPlus::ZoneAirLoopEquipmentManager;
using namespace EnergyPlus::DataAirSystems;
using namespace EnergyPlus::DataAirLoop;
using namespace EnergyPlus::OutputReportPredefined;

namespace EnergyPlus {
TEST_F(EnergyPlusFixture, TestSizingRoutineForHotWaterCoils1)
{

    // test whether sizing of zone reheat coil sizing works for performance input method = NominalCapacity
    // with all inputs set by user. The UA for the coil needs to be calculated.

    bool ErrorsFound(false);

    state->dataEnvrn->StdRhoAir = 1.20;

    std::string const idf_objects = delimited_string({
        "	Zone,",
        "	SPACE1-1, !- Name",
        "	0, !- Direction of Relative North { deg }",
        "	0, !- X Origin { m }",
        "	0, !- Y Origin { m }",
        "	0, !- Z Origin { m }",
        "	1, !- Type",
        "	1, !- Multiplier",
        "	2.438400269, !- Ceiling Height {m}",
        "	239.247360229; !- Volume {m3}",
        "	Sizing:Zone,",
        "	SPACE1-1, !- Zone or ZoneList Name",
        "	SupplyAirTemperature, !- Zone Cooling Design Supply Air Temperature Input Method",
        "	14., !- Zone Cooling Design Supply Air Temperature { C }",
        "	, !- Zone Cooling Design Supply Air Temperature Difference { deltaC }",
        "	SupplyAirTemperature, !- Zone Heating Design Supply Air Temperature Input Method",
        "	50., !- Zone Heating Design Supply Air Temperature { C }",
        "	, !- Zone Heating Design Supply Air Temperature Difference { deltaC }",
        "	0.009, !- Zone Cooling Design Supply Air Humidity Ratio { kgWater/kgDryAir }",
        "	0.004, !- Zone Heating Design Supply Air Humidity Ratio { kgWater/kgDryAir }",
        "	SZ DSOA SPACE1-1, !- Design Specification Outdoor Air Object Name",
        "	0.0, !- Zone Heating Sizing Factor",
        "	0.0, !- Zone Cooling Sizing Factor",
        "	DesignDayWithLimit, !- Cooling Design Air Flow Method",
        "	, !- Cooling Design Air Flow Rate { m3/s }",
        "	, !- Cooling Minimum Air Flow per Zone Floor Area { m3/s-m2 }",
        "	, !- Cooling Minimum Air Flow { m3/s }",
        "	, !- Cooling Minimum Air Flow Fraction",
        "	DesignDay, !- Heating Design Air Flow Method",
        "	, !- Heating Design Air Flow Rate { m3/s }",
        "	, !- Heating Maximum Air Flow per Zone Floor Area { m3/s-m2 }",
        "	, !- Heating Maximum Air Flow { m3/s }",
        "	, !- Heating Maximum Air Flow Fraction",
        "	SZ DZAD SPACE1-1;        !- Design Specification Zone Air Distribution Object Name",
        "	DesignSpecification:ZoneAirDistribution,",
        "	SZ DZAD SPACE1-1, !- Name",
        "	1, !- Zone Air Distribution Effectiveness in Cooling Mode { dimensionless }",
        "	1; !- Zone Air Distribution Effectiveness in Heating Mode { dimensionless }",
        "	DesignSpecification:OutdoorAir,",
        "	SZ DSOA SPACE1-1, !- Name",
        "	sum, !- Outdoor Air Method",
        "	0.00236, !- Outdoor Air Flow per Person { m3/s-person }",
        "	0.000305, !- Outdoor Air Flow per Zone Floor Area { m3/s-m2 }",
        "	0.0; !- Outdoor Air Flow per Zone { m3/s }",
        "	ScheduleTypeLimits,",
        "	Fraction, !- Name",
        "	0.0, !- Lower Limit Value",
        "	1.0, !- Upper Limit Value",
        "	CONTINUOUS; !- Numeric Type",
        "	Schedule:Compact,",
        "	ReheatCoilAvailSched, !- Name",
        "	Fraction, !- Schedule Type Limits Name",
        "	Through: 12/31, !- Field 1",
        "	For: AllDays, !- Field 2",
        "	Until: 24:00,1.0; !- Field 3",
        "	ZoneHVAC:EquipmentConnections,",
        "	SPACE1-1, !- Zone Name",
        "	SPACE1-1 Eq, !- Zone Conditioning Equipment List Name",
        "	SPACE1-1 In Node, !- Zone Air Inlet Node or NodeList Name",
        "	, !- Zone Air Exhaust Node or NodeList Name",
        "	SPACE1-1 Node, !- Zone Air Node Name",
        "	SPACE1-1 Out Node; !- Zone Return Air Node Name",
        "	ZoneHVAC:EquipmentList,",
        "	SPACE1-1 Eq, !- Name",
        "   SequentialLoad,          !- Load Distribution Scheme",
        "	ZoneHVAC:AirDistributionUnit, !- Zone Equipment 1 Object Type",
        "	SPACE1-1 ATU, !- Zone Equipment 1 Name",
        "	1, !- Zone Equipment 1 Cooling Sequence",
        "	1; !- Zone Equipment 1 Heating or No - Load Sequence",
        "	ZoneHVAC:AirDistributionUnit,",
        "	SPACE1-1 ATU, !- Name",
        "	SPACE1-1 In Node, !- Air Distribution Unit Outlet Node Name",
        "	AirTerminal:SingleDuct:VAV:Reheat, !- Air Terminal Object Type",
        "	SPACE1-1 VAV Reheat; !- Air Terminal Name",
        "	Coil:Heating:Water,",
        "	Gronk1 Zone Coil, !- Name",
        "	ReheatCoilAvailSched, !- Availability Schedule Name",
        "	, !- U-Factor Times Area Value { W/K }",
        "	, !- Maximum Water Flow Rate { m3/s }",
        "	SPACE1-1 Zone Coil Water In Node, !- Water Inlet Node Name",
        "	SPACE1-1 Zone Coil Water Out Node, !- Water Outlet Node Name",
        "	SPACE1-1 Zone Coil Air In Node, !- Air Inlet Node Name",
        "	SPACE1-1 In Node, !- Air Outlet Node Name",
        "	NominalCapacity, !- Performance Input Method",
        "	10000., !- Rated Capacity { W }",
        "	82.2, !- Rated Inlet Water Temperature { C }",
        "	16.6, !- Rated Inlet Air Temperature { C }",
        "	71.1, !- Rated Outlet Water Temperature { C }",
        "	32.2, !- Rated Outlet Air Temperature { C }",
        "	; !- Rated Ratio for Air and Water Convection",
        "	AirTerminal:SingleDuct:VAV:Reheat,",
        "	SPACE1-1 VAV Reheat, !- Name",
        "	ReheatCoilAvailSched, !- Availability Schedule Name",
        "	SPACE1-1 Zone Coil Air In Node, !- Damper Air Outlet Node Name",
        "	SPACE1-1 ATU In Node, !- Air Inlet Node Name",
        "	autosize, !- Maximum Air Flow Rate { m3/s }",
        "	, !- Zone Minimum Air Flow Input Method",
        "	, !- Constant Minimum Air Flow Fraction",
        "	, !- Fixed Minimum Air Flow Rate { m3/s }",
        "	, !- Minimum Air Flow Fraction Schedule Name",
        "	Coil:Heating:Water, !- Reheat Coil Object Type",
        "	Gronk1 Zone Coil, !- Reheat Coil Name",
        "	autosize, !- Maximum Hot Water or Steam Flow Rate { m3/s }",
        "	0.0, !- Minimum Hot Water or Steam Flow Rate { m3/s }",
        "	SPACE1-1 In Node, !- Air Outlet Node Name",
        "	0.001, !- Convergence Tolerance",
        "	, !- Damper Heating Action",
        "	, !- Maximum Flow per Zone Floor Area During Reheat { m3/s-m2 }",
        "	; !- Maximum Flow Fraction During Reheat",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->TermUnitFinalZoneSizing.allocate(1);
    state->dataSize->CalcFinalZoneSizing.allocate(1);
    state->dataSize->TermUnitSizing.allocate(1);
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    state->dataSize->PlantSizData.allocate(1);
    state->dataWaterCoils->MySizeFlag.allocate(1);
    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(1);
    state->dataSize->NumPltSizInput = 1;
    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {

        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }
    GetZoneData(*state, ErrorsFound);
    EXPECT_EQ("SPACE1-1", state->dataHeatBal->Zone(1).Name);
    GetOARequirements(*state);      // get the OA requirements object
    GetZoneAirDistribution(*state); // get zone air distribution objects
    GetZoneSizingInput(*state);
    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);
    GetWaterCoilInput(*state);
    state->dataWaterCoils->GetWaterCoilsInputFlag = false;
    state->dataWaterCoils->MySizeFlag(1) = true;
    state->dataWaterCoils->MyUAAndFlowCalcFlag(1) = false;
    GetSysInput(*state);
    state->dataSize->TermUnitSingDuct = true;
    state->dataPlnt->PlantLoop(1).Name = "HotWaterLoop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);

    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc = {1, DataPlant::LoopSideLocation::Demand, 1, 1};
    PlantUtilities::SetPlantLocationLinks(*state, state->dataWaterCoils->WaterCoil(1).WaterPlantLoc);

    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = state->dataWaterCoils->WaterCoil(1).Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut =
        state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum;

    state->dataSingleDuct->sd_airterminal(1).HWplantLoc = {1, DataPlant::LoopSideLocation::Demand, 1, 0};
    PlantUtilities::SetPlantLocationLinks(*state, state->dataSingleDuct->sd_airterminal(1).HWplantLoc);

    state->dataSize->PlantSizData(1).DeltaT = 11.0;
    state->dataSize->PlantSizData(1).ExitTemp = 82;
    state->dataSize->PlantSizData(1).PlantLoopName = "HotWaterLoop";
    state->dataSize->PlantSizData(1).LoopType = DataSizing::TypeOfPlantLoop::Heating;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->CurZoneEqNum = 1;
    state->dataSize->CurSysNum = 0;
    state->dataHeatBal->Zone(1).FloorArea = 99.16;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).DesignSizeFromParent = false;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod.allocate(25);
    state->dataSize->CurTermUnitSizingNum = 1;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow = 0.28794;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow = 0.12046;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInTempTU = 16.7;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInHumRatTU = 0.008;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow = 0.28794;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow = 0.12046;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlowFrac =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesCoolMinAirFlowFrac;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesHeatVolFlow = 0.12046;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).HeatSizingFactor = 1.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesHeatLoad = 3191.7;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneTempAtHeatPeak = 21.099;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneHumRatAtHeatPeak = 0.0038485;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInTempTU = 16.6;
    state->dataSize->TermUnitSizing(state->dataSize->CurZoneEqNum).AirVolFlow = 0.12046;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlow =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesCoolMinAirFlow;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlowFrac =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesCoolMinAirFlowFrac;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlow2 =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesCoolMinAirFlowPerArea * state->dataHeatBal->Zone(1).FloorArea;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlowMin =
        max(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlow,
            state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlow2,
            state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow *
                state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlowFrac);
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlow =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlow;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlowFrac =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlowFrac;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlowPerArea =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlowPerArea;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlow2 =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlowPerArea * state->dataHeatBal->Zone(1).FloorArea;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlowMax =
        max(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlow,
            state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlow2,
            max(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow,
                state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow) *
                state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlowFrac);
    state->dataSize->TermUnitFinalZoneSizing(state->dataSize->CurTermUnitSizingNum)
        .copyFromZoneSizing(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum));
    state->dataSingleDuct->sd_airterminal(1).ZoneFloorArea = state->dataHeatBal->Zone(1).FloorArea;
    state->dataSingleDuct->sd_airterminal(1).SizeSys(*state);
    SizeWaterCoil(*state, 1);
    EXPECT_NEAR(state->dataWaterCoils->WaterCoil(1).UACoil, 199.86, 0.01);

    state->dataLoopNodes->Node.deallocate();
    state->dataZoneEquip->ZoneEquipConfig.deallocate();
    state->dataHeatBal->Zone.deallocate();
    state->dataSize->FinalZoneSizing.deallocate();
    state->dataSize->TermUnitFinalZoneSizing.deallocate();
    state->dataSize->CalcFinalZoneSizing.deallocate();
    state->dataSize->TermUnitSizing.deallocate();
    state->dataSingleDuct->sd_airterminal.deallocate();
    state->dataSize->ZoneEqSizing.deallocate();
    state->dataPlnt->PlantLoop.deallocate();
    state->dataSize->PlantSizData.deallocate();
    state->dataWaterCoils->MySizeFlag.deallocate();
    state->dataWaterCoils->MyUAAndFlowCalcFlag.deallocate();
}

TEST_F(EnergyPlusFixture, TestSizingRoutineForHotWaterCoils2)
{

    // test whether autosizing of zone reheat coil works for performance input method = UFactorTimesAreaAndDesignWaterFlowRate,
    // all inputs autosized

    bool ErrorsFound(false);

    state->dataEnvrn->StdRhoAir = 1.20;

    std::string const idf_objects = delimited_string({
        "	Zone,",
        "	SPACE1-1, !- Name",
        "	0, !- Direction of Relative North { deg }",
        "	0, !- X Origin { m }",
        "	0, !- Y Origin { m }",
        "	0, !- Z Origin { m }",
        "	1, !- Type",
        "	1, !- Multiplier",
        "	2.438400269, !- Ceiling Height {m}",
        "	239.247360229; !- Volume {m3}",
        "	Sizing:Zone,",
        "	SPACE1-1, !- Zone or ZoneList Name",
        "	SupplyAirTemperature, !- Zone Cooling Design Supply Air Temperature Input Method",
        "	14., !- Zone Cooling Design Supply Air Temperature { C }",
        "	, !- Zone Cooling Design Supply Air Temperature Difference { deltaC }",
        "	SupplyAirTemperature, !- Zone Heating Design Supply Air Temperature Input Method",
        "	50., !- Zone Heating Design Supply Air Temperature { C }",
        "	, !- Zone Heating Design Supply Air Temperature Difference { deltaC }",
        "	0.009, !- Zone Cooling Design Supply Air Humidity Ratio { kgWater/kgDryAir }",
        "	0.004, !- Zone Heating Design Supply Air Humidity Ratio { kgWater/kgDryAir }",
        "	SZ DSOA SPACE1-1, !- Design Specification Outdoor Air Object Name",
        "	0.0, !- Zone Heating Sizing Factor",
        "	0.0, !- Zone Cooling Sizing Factor",
        "	DesignDayWithLimit, !- Cooling Design Air Flow Method",
        "	, !- Cooling Design Air Flow Rate { m3/s }",
        "	, !- Cooling Minimum Air Flow per Zone Floor Area { m3/s-m2 }",
        "	, !- Cooling Minimum Air Flow { m3/s }",
        "	, !- Cooling Minimum Air Flow Fraction",
        "	DesignDay, !- Heating Design Air Flow Method",
        "	, !- Heating Design Air Flow Rate { m3/s }",
        "	, !- Heating Maximum Air Flow per Zone Floor Area { m3/s-m2 }",
        "	, !- Heating Maximum Air Flow { m3/s }",
        "	, !- Heating Maximum Air Flow Fraction",
        "	SZ DZAD SPACE1-1;        !- Design Specification Zone Air Distribution Object Name",
        "	DesignSpecification:ZoneAirDistribution,",
        "	SZ DZAD SPACE1-1, !- Name",
        "	1, !- Zone Air Distribution Effectiveness in Cooling Mode { dimensionless }",
        "	1; !- Zone Air Distribution Effectiveness in Heating Mode { dimensionless }",
        "	DesignSpecification:OutdoorAir,",
        "	SZ DSOA SPACE1-1, !- Name",
        "	sum, !- Outdoor Air Method",
        "	0.00236, !- Outdoor Air Flow per Person { m3/s-person }",
        "	0.000305, !- Outdoor Air Flow per Zone Floor Area { m3/s-m2 }",
        "	0.0; !- Outdoor Air Flow per Zone { m3/s }",
        "	ScheduleTypeLimits,",
        "	Fraction, !- Name",
        "	0.0, !- Lower Limit Value",
        "	1.0, !- Upper Limit Value",
        "	CONTINUOUS; !- Numeric Type",
        "	Schedule:Compact,",
        "	ReheatCoilAvailSched, !- Name",
        "	Fraction, !- Schedule Type Limits Name",
        "	Through: 12/31, !- Field 1",
        "	For: AllDays, !- Field 2",
        "	Until: 24:00,1.0; !- Field 3",
        "	ZoneHVAC:EquipmentConnections,",
        "	SPACE1-1, !- Zone Name",
        "	SPACE1-1 Eq, !- Zone Conditioning Equipment List Name",
        "	SPACE1-1 In Node, !- Zone Air Inlet Node or NodeList Name",
        "	, !- Zone Air Exhaust Node or NodeList Name",
        "	SPACE1-1 Node, !- Zone Air Node Name",
        "	SPACE1-1 Out Node; !- Zone Return Air Node Name",
        "	ZoneHVAC:EquipmentList,",
        "	SPACE1-1 Eq, !- Name",
        "   SequentialLoad,          !- Load Distribution Scheme",
        "	ZoneHVAC:AirDistributionUnit, !- Zone Equipment 1 Object Type",
        "	SPACE1-1 ATU, !- Zone Equipment 1 Name",
        "	1, !- Zone Equipment 1 Cooling Sequence",
        "	1; !- Zone Equipment 1 Heating or No - Load Sequence",
        "	ZoneHVAC:AirDistributionUnit,",
        "	SPACE1-1 ATU, !- Name",
        "	SPACE1-1 In Node, !- Air Distribution Unit Outlet Node Name",
        "	AirTerminal:SingleDuct:VAV:Reheat, !- Air Terminal Object Type",
        "	SPACE1-1 VAV Reheat; !- Air Terminal Name",
        "	Coil:Heating:Water,",
        "	Gronk1 Zone Coil, !- Name",
        "	ReheatCoilAvailSched, !- Availability Schedule Name",
        "	, !- U-Factor Times Area Value { W/K }",
        "	, !- Maximum Water Flow Rate { m3/s }",
        "	SPACE1-1 Zone Coil Water In Node, !- Water Inlet Node Name",
        "	SPACE1-1 Zone Coil Water Out Node, !- Water Outlet Node Name",
        "	SPACE1-1 Zone Coil Air In Node, !- Air Inlet Node Name",
        "	SPACE1-1 In Node, !- Air Outlet Node Name",
        "	UFactorTimesAreaAndDesignWaterFlowRate, !- Performance Input Method",
        "	, !- Rated Capacity { W }",
        "	82.2, !- Rated Inlet Water Temperature { C }",
        "	16.6, !- Rated Inlet Air Temperature { C }",
        "	71.1, !- Rated Outlet Water Temperature { C }",
        "	32.2, !- Rated Outlet Air Temperature { C }",
        "	; !- Rated Ratio for Air and Water Convection",
        "	AirTerminal:SingleDuct:VAV:Reheat,",
        "	SPACE1-1 VAV Reheat, !- Name",
        "	ReheatCoilAvailSched, !- Availability Schedule Name",
        "	SPACE1-1 Zone Coil Air In Node, !- Damper Air Outlet Node Name",
        "	SPACE1-1 ATU In Node, !- Air Inlet Node Name",
        "	autosize, !- Maximum Air Flow Rate { m3/s }",
        "	, !- Zone Minimum Air Flow Input Method",
        "	, !- Constant Minimum Air Flow Fraction",
        "	, !- Fixed Minimum Air Flow Rate { m3/s }",
        "	, !- Minimum Air Flow Fraction Schedule Name",
        "	Coil:Heating:Water, !- Reheat Coil Object Type",
        "	Gronk1 Zone Coil, !- Reheat Coil Name",
        "	autosize, !- Maximum Hot Water or Steam Flow Rate { m3/s }",
        "	0.0, !- Minimum Hot Water or Steam Flow Rate { m3/s }",
        "	SPACE1-1 In Node, !- Air Outlet Node Name",
        "	0.001, !- Convergence Tolerance",
        "	, !- Damper Heating Action",
        "	, !- Maximum Flow per Zone Floor Area During Reheat { m3/s-m2 }",
        "	; !- Maximum Flow Fraction During Reheat",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->TermUnitFinalZoneSizing.allocate(1);
    state->dataSize->CalcFinalZoneSizing.allocate(1);
    state->dataSize->TermUnitSizing.allocate(1);
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    state->dataSize->PlantSizData.allocate(1);
    state->dataWaterCoils->MySizeFlag.allocate(1);
    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(1);
    state->dataSize->NumPltSizInput = 1;
    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {

        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }
    GetZoneData(*state, ErrorsFound);
    EXPECT_EQ("SPACE1-1", state->dataHeatBal->Zone(1).Name);
    GetOARequirements(*state);      // get the OA requirements object
    GetZoneAirDistribution(*state); // get zone air distribution objects
    GetZoneSizingInput(*state);
    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);
    GetWaterCoilInput(*state);
    state->dataWaterCoils->GetWaterCoilsInputFlag = false;
    state->dataWaterCoils->MySizeFlag(1) = true;
    state->dataWaterCoils->MyUAAndFlowCalcFlag(1) = false;
    GetSysInput(*state);
    state->dataSize->TermUnitSingDuct = true;
    state->dataPlnt->PlantLoop(1).Name = "HotWaterLoop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);

    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc = {1, DataPlant::LoopSideLocation::Demand, 1, 1};
    PlantUtilities::SetPlantLocationLinks(*state, state->dataWaterCoils->WaterCoil(1).WaterPlantLoc);

    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = state->dataWaterCoils->WaterCoil(1).Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut =
        state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum;

    state->dataSingleDuct->sd_airterminal(1).HWplantLoc = {1, DataPlant::LoopSideLocation::Demand, 1, 0};
    PlantUtilities::SetPlantLocationLinks(*state, state->dataSingleDuct->sd_airterminal(1).HWplantLoc);

    state->dataSize->PlantSizData(1).DeltaT = 11.0;
    state->dataSize->PlantSizData(1).ExitTemp = 82;
    state->dataSize->PlantSizData(1).PlantLoopName = "HotWaterLoop";
    state->dataSize->PlantSizData(1).LoopType = DataSizing::TypeOfPlantLoop::Heating;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->CurZoneEqNum = 1;
    state->dataSize->CurSysNum = 0;
    state->dataHeatBal->Zone(1).FloorArea = 99.16;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).DesignSizeFromParent = false;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod.allocate(25);
    state->dataSize->CurTermUnitSizingNum = 1;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow = 0.28794;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow = 0.12046;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInTempTU = 16.6;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInHumRatTU = 0.008;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow = 0.28794;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow = 0.12046;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlowFrac =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesCoolMinAirFlowFrac;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesHeatVolFlow = 0.12046;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).HeatSizingFactor = 1.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesHeatLoad = 3191.7;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneTempAtHeatPeak = 21.099;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneHumRatAtHeatPeak = 0.0038485;
    state->dataSize->TermUnitSizing(state->dataSize->CurTermUnitSizingNum).AirVolFlow = 0.12046;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlow =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesCoolMinAirFlow;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlowFrac =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesCoolMinAirFlowFrac;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlow2 =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesCoolMinAirFlowPerArea * state->dataHeatBal->Zone(1).FloorArea;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlowMin =
        max(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlow,
            state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlow2,
            state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow *
                state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlowFrac);
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlow =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlow;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlowFrac =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlowFrac;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlowPerArea =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlowPerArea;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlow2 =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlowPerArea * state->dataHeatBal->Zone(1).FloorArea;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlowMax =
        max(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlow,
            state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlow2,
            max(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow,
                state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow) *
                state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlowFrac);
    state->dataSize->TermUnitFinalZoneSizing(state->dataSize->CurTermUnitSizingNum)
        .copyFromZoneSizing(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum));
    state->dataSingleDuct->sd_airterminal(1).ZoneFloorArea = state->dataHeatBal->Zone(1).FloorArea;
    state->dataSingleDuct->sd_airterminal(1).ZoneFloorArea = state->dataHeatBal->Zone(1).FloorArea;
    state->dataSingleDuct->sd_airterminal(1).SizeSys(*state);
    SizeWaterCoil(*state, 1);
    EXPECT_NEAR(state->dataWaterCoils->WaterCoil(1).MaxWaterVolFlowRate, .0000850575, 0.000000001);
    EXPECT_NEAR(state->dataWaterCoils->WaterCoil(1).UACoil, 85.97495, 0.01);

    state->dataLoopNodes->Node.deallocate();
    state->dataZoneEquip->ZoneEquipConfig.deallocate();
    state->dataHeatBal->Zone.deallocate();
    state->dataSize->FinalZoneSizing.deallocate();
    state->dataSize->TermUnitFinalZoneSizing.deallocate();
    state->dataSize->CalcFinalZoneSizing.deallocate();
    state->dataSize->TermUnitSizing.deallocate();
    state->dataSingleDuct->sd_airterminal.deallocate();
    state->dataSize->ZoneEqSizing.deallocate();
    state->dataPlnt->PlantLoop.deallocate();
    state->dataSize->PlantSizData.deallocate();
    state->dataWaterCoils->MySizeFlag.deallocate();
    state->dataWaterCoils->MyUAAndFlowCalcFlag.deallocate();
}

TEST_F(EnergyPlusFixture, TestSizingRoutineForHotWaterCoils3)
{

    // test whether sizing of zone reheat coil sizing works for performance input method = NominalCaoacity
    // with all inputs set by user except rated capacity is autosized
    bool ErrorsFound(false);

    state->dataEnvrn->StdRhoAir = 1.20;

    std::string const idf_objects = delimited_string({
        "	Zone,",
        "	SPACE1-1, !- Name",
        "	0, !- Direction of Relative North { deg }",
        "	0, !- X Origin { m }",
        "	0, !- Y Origin { m }",
        "	0, !- Z Origin { m }",
        "	1, !- Type",
        "	1, !- Multiplier",
        "	2.438400269, !- Ceiling Height {m}",
        "	239.247360229; !- Volume {m3}",
        "	Sizing:Zone,",
        "	SPACE1-1, !- Zone or ZoneList Name",
        "	SupplyAirTemperature, !- Zone Cooling Design Supply Air Temperature Input Method",
        "	14., !- Zone Cooling Design Supply Air Temperature { C }",
        "	, !- Zone Cooling Design Supply Air Temperature Difference { deltaC }",
        "	SupplyAirTemperature, !- Zone Heating Design Supply Air Temperature Input Method",
        "	50., !- Zone Heating Design Supply Air Temperature { C }",
        "	, !- Zone Heating Design Supply Air Temperature Difference { deltaC }",
        "	0.009, !- Zone Cooling Design Supply Air Humidity Ratio { kgWater/kgDryAir }",
        "	0.004, !- Zone Heating Design Supply Air Humidity Ratio { kgWater/kgDryAir }",
        "	SZ DSOA SPACE1-1, !- Design Specification Outdoor Air Object Name",
        "	0.0, !- Zone Heating Sizing Factor",
        "	0.0, !- Zone Cooling Sizing Factor",
        "	DesignDayWithLimit, !- Cooling Design Air Flow Method",
        "	, !- Cooling Design Air Flow Rate { m3/s }",
        "	, !- Cooling Minimum Air Flow per Zone Floor Area { m3/s-m2 }",
        "	, !- Cooling Minimum Air Flow { m3/s }",
        "	, !- Cooling Minimum Air Flow Fraction",
        "	DesignDay, !- Heating Design Air Flow Method",
        "	, !- Heating Design Air Flow Rate { m3/s }",
        "	, !- Heating Maximum Air Flow per Zone Floor Area { m3/s-m2 }",
        "	, !- Heating Maximum Air Flow { m3/s }",
        "	, !- Heating Maximum Air Flow Fraction",
        "	SZ DZAD SPACE1-1;        !- Design Specification Zone Air Distribution Object Name",
        "	DesignSpecification:ZoneAirDistribution,",
        "	SZ DZAD SPACE1-1, !- Name",
        "	1, !- Zone Air Distribution Effectiveness in Cooling Mode { dimensionless }",
        "	1; !- Zone Air Distribution Effectiveness in Heating Mode { dimensionless }",
        "	DesignSpecification:OutdoorAir,",
        "	SZ DSOA SPACE1-1, !- Name",
        "	sum, !- Outdoor Air Method",
        "	0.00236, !- Outdoor Air Flow per Person { m3/s-person }",
        "	0.000305, !- Outdoor Air Flow per Zone Floor Area { m3/s-m2 }",
        "	0.0; !- Outdoor Air Flow per Zone { m3/s }",
        "	ScheduleTypeLimits,",
        "	Fraction, !- Name",
        "	0.0, !- Lower Limit Value",
        "	1.0, !- Upper Limit Value",
        "	CONTINUOUS; !- Numeric Type",
        "	Schedule:Compact,",
        "	ReheatCoilAvailSched, !- Name",
        "	Fraction, !- Schedule Type Limits Name",
        "	Through: 12/31, !- Field 1",
        "	For: AllDays, !- Field 2",
        "	Until: 24:00,1.0; !- Field 3",
        "	ZoneHVAC:EquipmentConnections,",
        "	SPACE1-1, !- Zone Name",
        "	SPACE1-1 Eq, !- Zone Conditioning Equipment List Name",
        "	SPACE1-1 In Node, !- Zone Air Inlet Node or NodeList Name",
        "	, !- Zone Air Exhaust Node or NodeList Name",
        "	SPACE1-1 Node, !- Zone Air Node Name",
        "	SPACE1-1 Out Node; !- Zone Return Air Node Name",
        "	ZoneHVAC:EquipmentList,",
        "	SPACE1-1 Eq, !- Name",
        "   SequentialLoad,          !- Load Distribution Scheme",
        "	ZoneHVAC:AirDistributionUnit, !- Zone Equipment 1 Object Type",
        "	SPACE1-1 ATU, !- Zone Equipment 1 Name",
        "	1, !- Zone Equipment 1 Cooling Sequence",
        "	1; !- Zone Equipment 1 Heating or No - Load Sequence",
        "	ZoneHVAC:AirDistributionUnit,",
        "	SPACE1-1 ATU, !- Name",
        "	SPACE1-1 In Node, !- Air Distribution Unit Outlet Node Name",
        "	AirTerminal:SingleDuct:VAV:Reheat, !- Air Terminal Object Type",
        "	SPACE1-1 VAV Reheat; !- Air Terminal Name",
        "	Coil:Heating:Water,",
        "	Gronk1 Zone Coil, !- Name",
        "	ReheatCoilAvailSched, !- Availability Schedule Name",
        "	, !- U-Factor Times Area Value { W/K }",
        "	, !- Maximum Water Flow Rate { m3/s }",
        "	SPACE1-1 Zone Coil Water In Node, !- Water Inlet Node Name",
        "	SPACE1-1 Zone Coil Water Out Node, !- Water Outlet Node Name",
        "	SPACE1-1 Zone Coil Air In Node, !- Air Inlet Node Name",
        "	SPACE1-1 In Node, !- Air Outlet Node Name",
        "	NominalCapacity, !- Performance Input Method",
        "	, !- Rated Capacity { W }",
        "	82.2, !- Rated Inlet Water Temperature { C }",
        "	16.6, !- Rated Inlet Air Temperature { C }",
        "	71.1, !- Rated Outlet Water Temperature { C }",
        "	32.2, !- Rated Outlet Air Temperature { C }",
        "	; !- Rated Ratio for Air and Water Convection",
        "	AirTerminal:SingleDuct:VAV:Reheat,",
        "	SPACE1-1 VAV Reheat, !- Name",
        "	ReheatCoilAvailSched, !- Availability Schedule Name",
        "	SPACE1-1 Zone Coil Air In Node, !- Damper Air Outlet Node Name",
        "	SPACE1-1 ATU In Node, !- Air Inlet Node Name",
        "	autosize, !- Maximum Air Flow Rate { m3/s }",
        "	, !- Zone Minimum Air Flow Input Method",
        "	, !- Constant Minimum Air Flow Fraction",
        "	, !- Fixed Minimum Air Flow Rate { m3/s }",
        "	, !- Minimum Air Flow Fraction Schedule Name",
        "	Coil:Heating:Water, !- Reheat Coil Object Type",
        "	Gronk1 Zone Coil, !- Reheat Coil Name",
        "	autosize, !- Maximum Hot Water or Steam Flow Rate { m3/s }",
        "	0.0, !- Minimum Hot Water or Steam Flow Rate { m3/s }",
        "	SPACE1-1 In Node, !- Air Outlet Node Name",
        "	0.001, !- Convergence Tolerance",
        "	, !- Damper Heating Action",
        "	, !- Maximum Flow per Zone Floor Area During Reheat { m3/s-m2 }",
        "	; !- Maximum Flow Fraction During Reheat",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->TermUnitFinalZoneSizing.allocate(1);
    state->dataSize->CalcFinalZoneSizing.allocate(1);
    state->dataSize->TermUnitSizing.allocate(1);
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    state->dataSize->PlantSizData.allocate(1);
    state->dataWaterCoils->MySizeFlag.allocate(1);
    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(1);
    state->dataSize->NumPltSizInput = 1;
    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {

        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }
    GetZoneData(*state, ErrorsFound);
    EXPECT_EQ("SPACE1-1", state->dataHeatBal->Zone(1).Name);
    GetOARequirements(*state);      // get the OA requirements object
    GetZoneAirDistribution(*state); // get zone air distribution objects
    GetZoneSizingInput(*state);
    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);
    GetWaterCoilInput(*state);
    state->dataWaterCoils->GetWaterCoilsInputFlag = false;
    state->dataWaterCoils->MySizeFlag(1) = true;
    state->dataWaterCoils->MyUAAndFlowCalcFlag(1) = false;
    GetSysInput(*state);
    state->dataSize->TermUnitSingDuct = true;

    state->dataPlnt->PlantLoop(1).Name = "HotWaterLoop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);

    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc = {1, DataPlant::LoopSideLocation::Demand, 1, 1};
    PlantUtilities::SetPlantLocationLinks(*state, state->dataWaterCoils->WaterCoil(1).WaterPlantLoc);

    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = state->dataWaterCoils->WaterCoil(1).Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut =
        state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum;

    state->dataSingleDuct->sd_airterminal(1).HWplantLoc = {1, DataPlant::LoopSideLocation::Demand, 1, 0};
    PlantUtilities::SetPlantLocationLinks(*state, state->dataSingleDuct->sd_airterminal(1).HWplantLoc);

    state->dataSize->PlantSizData(1).DeltaT = 11.0;
    state->dataSize->PlantSizData(1).ExitTemp = 82;
    state->dataSize->PlantSizData(1).PlantLoopName = "HotWaterLoop";
    state->dataSize->PlantSizData(1).LoopType = DataSizing::TypeOfPlantLoop::Heating;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->CurZoneEqNum = 1;
    state->dataSize->CurSysNum = 0;
    state->dataHeatBal->Zone(1).FloorArea = 99.16;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).DesignSizeFromParent = false;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod.allocate(25);
    state->dataSize->CurTermUnitSizingNum = 1;
    state->dataSize->TermUnitFinalZoneSizing(state->dataSize->CurTermUnitSizingNum).DesCoolVolFlow = 0.28794;
    state->dataSize->TermUnitFinalZoneSizing(state->dataSize->CurTermUnitSizingNum).DesHeatVolFlow = 0.12046;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInTempTU = 16.6;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInHumRatTU = 0.008;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow = 0.28794;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow = 0.12046;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlowFrac =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesCoolMinAirFlowFrac;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesHeatVolFlow = 0.12046;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).HeatSizingFactor = 1.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesHeatLoad = 3191.7;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneTempAtHeatPeak = 21.099;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneHumRatAtHeatPeak = 0.0038485;
    state->dataSize->TermUnitSizing(state->dataSize->CurTermUnitSizingNum).AirVolFlow = 0.12046;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlow =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesCoolMinAirFlow;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlowFrac =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesCoolMinAirFlowFrac;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlow2 =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesCoolMinAirFlowPerArea * state->dataHeatBal->Zone(1).FloorArea;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlowMin =
        max(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlow,
            state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlow2,
            state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow *
                state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlowFrac);
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlow =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlow;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlowFrac =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlowFrac;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlowPerArea =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlowPerArea;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlow2 =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlowPerArea * state->dataHeatBal->Zone(1).FloorArea;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlowMax =
        max(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlow,
            state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlow2,
            max(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow,
                state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow) *
                state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlowFrac);
    state->dataSize->TermUnitFinalZoneSizing(state->dataSize->CurTermUnitSizingNum)
        .copyFromZoneSizing(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum));
    state->dataSingleDuct->sd_airterminal(1).ZoneFloorArea = state->dataHeatBal->Zone(1).FloorArea;
    state->dataSingleDuct->sd_airterminal(1).SizeSys(*state);
    SizeWaterCoil(*state, 1);
    EXPECT_NEAR(state->dataWaterCoils->WaterCoil(1).MaxWaterVolFlowRate, .0000850575, 0.000000001);
    EXPECT_NEAR(state->dataWaterCoils->WaterCoil(1).UACoil, 85.97495, 0.02);

    state->dataLoopNodes->Node.deallocate();
    state->dataZoneEquip->ZoneEquipConfig.deallocate();
    state->dataHeatBal->Zone.deallocate();
    state->dataSize->FinalZoneSizing.deallocate();
    state->dataSize->TermUnitFinalZoneSizing.deallocate();
    state->dataSize->CalcFinalZoneSizing.deallocate();
    state->dataSize->TermUnitSizing.deallocate();
    state->dataSingleDuct->sd_airterminal.deallocate();
    state->dataSize->ZoneEqSizing.deallocate();
    state->dataPlnt->PlantLoop.deallocate();
    state->dataSize->PlantSizData.deallocate();
    state->dataWaterCoils->MySizeFlag.deallocate();
    state->dataWaterCoils->MyUAAndFlowCalcFlag.deallocate();
}

TEST_F(EnergyPlusFixture, TestSizingRoutineForHotWaterCoils4)
{

    // test whether autosizing of zone reheat coil works for performance input method = UFactorTimesAreaAndDesignWaterFlowRate,
    // UA is user input.

    bool ErrorsFound(false);

    state->dataEnvrn->StdRhoAir = 1.20;

    std::string const idf_objects = delimited_string({
        "	Zone,",
        "	SPACE1-1, !- Name",
        "	0, !- Direction of Relative North { deg }",
        "	0, !- X Origin { m }",
        "	0, !- Y Origin { m }",
        "	0, !- Z Origin { m }",
        "	1, !- Type",
        "	1, !- Multiplier",
        "	2.438400269, !- Ceiling Height {m}",
        "	239.247360229; !- Volume {m3}",
        "	Sizing:Zone,",
        "	SPACE1-1, !- Zone or ZoneList Name",
        "	SupplyAirTemperature, !- Zone Cooling Design Supply Air Temperature Input Method",
        "	14., !- Zone Cooling Design Supply Air Temperature { C }",
        "	, !- Zone Cooling Design Supply Air Temperature Difference { deltaC }",
        "	SupplyAirTemperature, !- Zone Heating Design Supply Air Temperature Input Method",
        "	50., !- Zone Heating Design Supply Air Temperature { C }",
        "	, !- Zone Heating Design Supply Air Temperature Difference { deltaC }",
        "	0.009, !- Zone Cooling Design Supply Air Humidity Ratio { kgWater/kgDryAir }",
        "	0.004, !- Zone Heating Design Supply Air Humidity Ratio { kgWater/kgDryAir }",
        "	SZ DSOA SPACE1-1, !- Design Specification Outdoor Air Object Name",
        "	0.0, !- Zone Heating Sizing Factor",
        "	0.0, !- Zone Cooling Sizing Factor",
        "	DesignDayWithLimit, !- Cooling Design Air Flow Method",
        "	, !- Cooling Design Air Flow Rate { m3/s }",
        "	, !- Cooling Minimum Air Flow per Zone Floor Area { m3/s-m2 }",
        "	, !- Cooling Minimum Air Flow { m3/s }",
        "	, !- Cooling Minimum Air Flow Fraction",
        "	DesignDay, !- Heating Design Air Flow Method",
        "	, !- Heating Design Air Flow Rate { m3/s }",
        "	, !- Heating Maximum Air Flow per Zone Floor Area { m3/s-m2 }",
        "	, !- Heating Maximum Air Flow { m3/s }",
        "	, !- Heating Maximum Air Flow Fraction",
        "	SZ DZAD SPACE1-1;        !- Design Specification Zone Air Distribution Object Name",
        "	DesignSpecification:ZoneAirDistribution,",
        "	SZ DZAD SPACE1-1, !- Name",
        "	1, !- Zone Air Distribution Effectiveness in Cooling Mode { dimensionless }",
        "	1; !- Zone Air Distribution Effectiveness in Heating Mode { dimensionless }",
        "	DesignSpecification:OutdoorAir,",
        "	SZ DSOA SPACE1-1, !- Name",
        "	sum, !- Outdoor Air Method",
        "	0.00236, !- Outdoor Air Flow per Person { m3/s-person }",
        "	0.000305, !- Outdoor Air Flow per Zone Floor Area { m3/s-m2 }",
        "	0.0; !- Outdoor Air Flow per Zone { m3/s }",
        "	ScheduleTypeLimits,",
        "	Fraction, !- Name",
        "	0.0, !- Lower Limit Value",
        "	1.0, !- Upper Limit Value",
        "	CONTINUOUS; !- Numeric Type",
        "	Schedule:Compact,",
        "	ReheatCoilAvailSched, !- Name",
        "	Fraction, !- Schedule Type Limits Name",
        "	Through: 12/31, !- Field 1",
        "	For: AllDays, !- Field 2",
        "	Until: 24:00,1.0; !- Field 3",
        "	ZoneHVAC:EquipmentConnections,",
        "	SPACE1-1, !- Zone Name",
        "	SPACE1-1 Eq, !- Zone Conditioning Equipment List Name",
        "	SPACE1-1 In Node, !- Zone Air Inlet Node or NodeList Name",
        "	, !- Zone Air Exhaust Node or NodeList Name",
        "	SPACE1-1 Node, !- Zone Air Node Name",
        "	SPACE1-1 Out Node; !- Zone Return Air Node Name",
        "	ZoneHVAC:EquipmentList,",
        "	SPACE1-1 Eq, !- Name",
        "   SequentialLoad,          !- Load Distribution Scheme",
        "	ZoneHVAC:AirDistributionUnit, !- Zone Equipment 1 Object Type",
        "	SPACE1-1 ATU, !- Zone Equipment 1 Name",
        "	1, !- Zone Equipment 1 Cooling Sequence",
        "	1; !- Zone Equipment 1 Heating or No - Load Sequence",
        "	ZoneHVAC:AirDistributionUnit,",
        "	SPACE1-1 ATU, !- Name",
        "	SPACE1-1 In Node, !- Air Distribution Unit Outlet Node Name",
        "	AirTerminal:SingleDuct:VAV:Reheat, !- Air Terminal Object Type",
        "	SPACE1-1 VAV Reheat; !- Air Terminal Name",
        "	Coil:Heating:Water,",
        "	Gronk1 Zone Coil, !- Name",
        "	ReheatCoilAvailSched, !- Availability Schedule Name",
        "	300., !- U-Factor Times Area Value { W/K }",
        "	, !- Maximum Water Flow Rate { m3/s }",
        "	SPACE1-1 Zone Coil Water In Node, !- Water Inlet Node Name",
        "	SPACE1-1 Zone Coil Water Out Node, !- Water Outlet Node Name",
        "	SPACE1-1 Zone Coil Air In Node, !- Air Inlet Node Name",
        "	SPACE1-1 In Node, !- Air Outlet Node Name",
        "	UFactorTimesAreaAndDesignWaterFlowRate, !- Performance Input Method",
        "	, !- Rated Capacity { W }",
        "	82.2, !- Rated Inlet Water Temperature { C }",
        "	16.6, !- Rated Inlet Air Temperature { C }",
        "	71.1, !- Rated Outlet Water Temperature { C }",
        "	32.2, !- Rated Outlet Air Temperature { C }",
        "	; !- Rated Ratio for Air and Water Convection",
        "	AirTerminal:SingleDuct:VAV:Reheat,",
        "	SPACE1-1 VAV Reheat, !- Name",
        "	ReheatCoilAvailSched, !- Availability Schedule Name",
        "	SPACE1-1 Zone Coil Air In Node, !- Damper Air Outlet Node Name",
        "	SPACE1-1 ATU In Node, !- Air Inlet Node Name",
        "	autosize, !- Maximum Air Flow Rate { m3/s }",
        "	, !- Zone Minimum Air Flow Input Method",
        "	, !- Constant Minimum Air Flow Fraction",
        "	, !- Fixed Minimum Air Flow Rate { m3/s }",
        "	, !- Minimum Air Flow Fraction Schedule Name",
        "	Coil:Heating:Water, !- Reheat Coil Object Type",
        "	Gronk1 Zone Coil, !- Reheat Coil Name",
        "	autosize, !- Maximum Hot Water or Steam Flow Rate { m3/s }",
        "	0.0, !- Minimum Hot Water or Steam Flow Rate { m3/s }",
        "	SPACE1-1 In Node, !- Air Outlet Node Name",
        "	0.001, !- Convergence Tolerance",
        "	, !- Damper Heating Action",
        "	, !- Maximum Flow per Zone Floor Area During Reheat { m3/s-m2 }",
        "	; !- Maximum Flow Fraction During Reheat",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->TermUnitFinalZoneSizing.allocate(1);
    state->dataSize->CalcFinalZoneSizing.allocate(1);
    state->dataSize->TermUnitSizing.allocate(1);
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    state->dataSize->PlantSizData.allocate(1);
    state->dataWaterCoils->MySizeFlag.allocate(1);
    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(1);
    state->dataSize->NumPltSizInput = 1;
    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {

        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }
    GetZoneData(*state, ErrorsFound);
    EXPECT_EQ("SPACE1-1", state->dataHeatBal->Zone(1).Name);
    GetOARequirements(*state);      // get the OA requirements object
    GetZoneAirDistribution(*state); // get zone air distribution objects
    GetZoneSizingInput(*state);
    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);
    GetWaterCoilInput(*state);
    state->dataWaterCoils->GetWaterCoilsInputFlag = false;
    state->dataWaterCoils->MySizeFlag(1) = true;
    state->dataWaterCoils->MyUAAndFlowCalcFlag(1) = false;
    GetSysInput(*state);
    state->dataSize->TermUnitSingDuct = true;
    state->dataPlnt->PlantLoop(1).Name = "HotWaterLoop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);

    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc = {1, DataPlant::LoopSideLocation::Demand, 1, 1};
    PlantUtilities::SetPlantLocationLinks(*state, state->dataWaterCoils->WaterCoil(1).WaterPlantLoc);

    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = state->dataWaterCoils->WaterCoil(1).Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut =
        state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum;

    state->dataSingleDuct->sd_airterminal(1).HWplantLoc = {1, DataPlant::LoopSideLocation::Demand, 1, 0};
    PlantUtilities::SetPlantLocationLinks(*state, state->dataSingleDuct->sd_airterminal(1).HWplantLoc);

    state->dataSize->PlantSizData(1).DeltaT = 11.0;
    state->dataSize->PlantSizData(1).ExitTemp = 82;
    state->dataSize->PlantSizData(1).PlantLoopName = "HotWaterLoop";
    state->dataSize->PlantSizData(1).LoopType = DataSizing::TypeOfPlantLoop::Heating;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->CurZoneEqNum = 1;
    state->dataSize->CurSysNum = 0;
    state->dataHeatBal->Zone(1).FloorArea = 99.16;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).DesignSizeFromParent = false;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod.allocate(25);
    state->dataSize->CurTermUnitSizingNum = 1;
    state->dataSize->TermUnitFinalZoneSizing(state->dataSize->CurTermUnitSizingNum).DesCoolVolFlow = 0.28794;
    state->dataSize->TermUnitFinalZoneSizing(state->dataSize->CurTermUnitSizingNum).DesHeatVolFlow = 0.12046;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInTempTU = 16.6;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInHumRatTU = 0.008;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow = 0.28794;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow = 0.12046;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlowFrac =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesCoolMinAirFlowFrac;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesHeatVolFlow = 0.12046;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).HeatSizingFactor = 1.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesHeatLoad = 3191.7;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneTempAtHeatPeak = 21.099;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneHumRatAtHeatPeak = 0.0038485;
    state->dataSize->TermUnitSizing(state->dataSize->CurTermUnitSizingNum).AirVolFlow = 0.12046;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlow =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesCoolMinAirFlow;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlowFrac =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesCoolMinAirFlowFrac;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlow2 =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesCoolMinAirFlowPerArea * state->dataHeatBal->Zone(1).FloorArea;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlowMin =
        max(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlow,
            state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlow2,
            state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow *
                state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolMinAirFlowFrac);
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlow =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlow;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlowFrac =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlowFrac;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlowPerArea =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlowPerArea;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlow2 =
        state->dataSize->ZoneSizingInput(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlowPerArea * state->dataHeatBal->Zone(1).FloorArea;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlowMax =
        max(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlow,
            state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlow2,
            max(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow,
                state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow) *
                state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMaxAirFlowFrac);
    state->dataSize->TermUnitFinalZoneSizing(state->dataSize->CurTermUnitSizingNum)
        .copyFromZoneSizing(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum));
    state->dataSingleDuct->sd_airterminal(1).ZoneFloorArea = state->dataHeatBal->Zone(1).FloorArea;
    state->dataSingleDuct->sd_airterminal(1).SizeSys(*state);
    SizeWaterCoil(*state, 1);
    EXPECT_NEAR(state->dataWaterCoils->WaterCoil(1).MaxWaterVolFlowRate, .0000850575, 0.000000001);
    EXPECT_NEAR(state->dataWaterCoils->WaterCoil(1).UACoil, 300.00, 0.01);

    state->dataLoopNodes->Node.deallocate();
    state->dataZoneEquip->ZoneEquipConfig.deallocate();
    state->dataHeatBal->Zone.deallocate();
    state->dataSize->FinalZoneSizing.deallocate();
    state->dataSize->TermUnitFinalZoneSizing.deallocate();
    state->dataSize->CalcFinalZoneSizing.deallocate();
    state->dataSize->TermUnitSizing.deallocate();
    state->dataSingleDuct->sd_airterminal.deallocate();
    state->dataSize->ZoneEqSizing.deallocate();
    state->dataPlnt->PlantLoop.deallocate();
    state->dataSize->PlantSizData.deallocate();
    state->dataWaterCoils->MySizeFlag.deallocate();
    state->dataWaterCoils->MyUAAndFlowCalcFlag.deallocate();
}

TEST_F(EnergyPlusFixture, TestSizingRoutineForHotWaterCoils5)
{

    // test whether autosizing of system heating coil works when capacity is entered in system sizing array

    bool ErrorsFound(false);

    state->dataEnvrn->StdRhoAir = 1.20;

    std::string const idf_objects = delimited_string({
        "	Zone,",
        "	SPACE1-1, !- Name",
        "	0, !- Direction of Relative North { deg }",
        "	0, !- X Origin { m }",
        "	0, !- Y Origin { m }",
        "	0, !- Z Origin { m }",
        "	1, !- Type",
        "	1, !- Multiplier",
        "	2.438400269, !- Ceiling Height {m}",
        "	239.247360229; !- Volume {m3}",
        "	Sizing:System,",
        "	VAV Sys 1, !- AirLoop Name",
        "	sensible, !- Type of Load to Size On",
        "	autosize, !- Design Outdoor Air Flow Rate { m3/s }",
        "	0.3, !- Central Heating Maximum System Air Flow Ratio",
        "	7.0, !- Preheat Design Temperature { C }",
        "	0.008, !- Preheat Design Humidity Ratio { kgWater/kgDryAir }",
        "	11.0, !- Precool Design Temperature { C }",
        "	0.008, !- Precool Design Humidity Ratio { kgWater/kgDryAir }",
        "	12.8, !- Central Cooling Design Supply Air Temperature { C }",
        "	16.7, !- Central Heating Design Supply Air Temperature { C }",
        "	noncoincident, !- Type of Zone Sum to Use",
        "	no, !- 100% Outdoor Air in Cooling",
        "	no, !- 100% Outdoor Air in Heating",
        "	0.008, !- Central Cooling Design Supply Air Humidity Ratio { kgWater/kgDryAir }",
        "	0.008, !- Central Heating Design Supply Air Humidity Ratio { kgWater/kgDryAir }",
        "	DesignDay, !- Cooling Design Air Flow Method",
        "	0, !- Cooling Design Air Flow Rate { m3/s }",
        "	, !- Supply Air Flow Rate Per Floor Area During Cooling Operation { m3/s-m2 }",
        "	, !- Fraction of Autosized Design Cooling Supply Air Flow Rate",
        "	, !- Design Supply Air Flow Rate Per Unit Cooling Capacity { m3/s-W }",
        "	FlowPerHeatingCapacity, !- Heating Design Air Flow Method",
        "	0, !- Heating Design Air Flow Rate { m3/s }",
        "	, !- Supply Air Flow Rate Per Floor Area During Heating Operation { m3/s-m2 }",
        "	, !- Fraction of Autosized Design Heating Supply Air Flow Rate",
        "	, !- Fraction of Autosized Design Cooling Supply Air Flow Rate",
        "	0.000174194, !- Design Supply Air Flow Rate Per Unit Heating Capacity { m3/s-W }",
        "	, !- System Outdoor Air Method",
        "	1.0, !- Zone Maximum Outdoor Air Fraction { dimensionless }",
        "	CoolingDesignCapacity, !- Cooling Design Capacity Method",
        "	autosize, !- Cooling Design Capacity { W }",
        "	, !- Cooling Design Capacity Per Floor Area { W/m2 }",
        "	, !- Fraction of Autosized Cooling Design Capacity",
        "	HeatingDesignCapacity, !- Heating Design Capacity Method",
        "	12000, !- Heating Design Capacity { W }",
        "	, !- Heating Design Capacity Per Floor Area { W/m2 }",
        "	, !- Fraction of Autosized Heating Design Capacity",
        "	VAV;                     !- Central Cooling Capacity Control Method",
        "	ScheduleTypeLimits,",
        "	Fraction, !- Name",
        "	0.0, !- Lower Limit Value",
        "	1.0, !- Upper Limit Value",
        "	CONTINUOUS; !- Numeric Type",
        "	Schedule:Compact,",
        "	ReheatCoilAvailSched, !- Name",
        "	Fraction, !- Schedule Type Limits Name",
        "	Through: 12/31, !- Field 1",
        "	For: AllDays, !- Field 2",
        "	Until: 24:00,1.0; !- Field 3",
        "	Coil:Heating:Water,",
        "	Gronk1 Zone Coil, !- Name",
        "	ReheatCoilAvailSched, !- Availability Schedule Name",
        "	, !- U-Factor Times Area Value { W/K }",
        "	, !- Maximum Water Flow Rate { m3/s }",
        "	SPACE1-1 Zone Coil Water In Node, !- Water Inlet Node Name",
        "	SPACE1-1 Zone Coil Water Out Node, !- Water Outlet Node Name",
        "	SPACE1-1 Zone Coil Air In Node, !- Air Inlet Node Name",
        "	SPACE1-1 In Node, !- Air Outlet Node Name",
        "	NominalCapacity, !- Performance Input Method",
        "	, !- Rated Capacity { W }",
        "	82.2, !- Rated Inlet Water Temperature { C }",
        "	16.6, !- Rated Inlet Air Temperature { C }",
        "	71.1, !- Rated Outlet Water Temperature { C }",
        "	32.2, !- Rated Outlet Air Temperature { C }",
        "	; !- Rated Ratio for Air and Water Convection",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataSize->FinalSysSizing.allocate(1);
    state->dataSize->UnitarySysEqSizing.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems.allocate(1);
    state->dataAirLoop->AirLoopControlInfo.allocate(1);
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    state->dataSize->PlantSizData.allocate(1);
    state->dataWaterCoils->MySizeFlag.allocate(1);
    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(1);
    state->dataSize->NumPltSizInput = 1;
    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {

        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }
    GetZoneData(*state, ErrorsFound);
    EXPECT_EQ("SPACE1-1", state->dataHeatBal->Zone(1).Name);
    GetWaterCoilInput(*state);
    state->dataWaterCoils->GetWaterCoilsInputFlag = false;
    state->dataWaterCoils->MySizeFlag(1) = true;
    state->dataWaterCoils->MyUAAndFlowCalcFlag(1) = false;
    state->dataSize->TermUnitSingDuct = true;
    state->dataPlnt->PlantLoop(1).Name = "HotWaterLoop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);

    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc = {1, DataPlant::LoopSideLocation::Demand, 1, 1};
    PlantUtilities::SetPlantLocationLinks(*state, state->dataWaterCoils->WaterCoil(1).WaterPlantLoc);

    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = state->dataWaterCoils->WaterCoil(1).Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut =
        state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum;
    state->dataSize->PlantSizData(1).DeltaT = 11.0;
    state->dataSize->PlantSizData(1).ExitTemp = 82;
    state->dataSize->PlantSizData(1).PlantLoopName = "HotWaterLoop";
    state->dataSize->PlantSizData(1).LoopType = DataSizing::TypeOfPlantLoop::Heating;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurSysNum = 1;
    state->dataSize->CurDuctType = HVAC::AirDuctType::Main;
    state->dataHeatBal->Zone(1).FloorArea = 99.16;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).HeatingCapMethod = 9;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).HeatingTotalCapacity = 12000.;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).SysAirMinFlowRat = 0.3;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesMainVolFlow = 3.4;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesOutAirVolFlow = 0.49;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).HeatOutTemp = -17.3;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).HeatRetTemp = 21.3;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).HeatSupTemp = 16.7;
    state->dataSize->UnitarySysEqSizing(state->dataSize->CurSysNum).CoolingCapacity = false;
    state->dataSize->UnitarySysEqSizing(state->dataSize->CurSysNum).HeatingCapacity = false;
    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    SizeWaterCoil(*state, 1);
    EXPECT_NEAR(state->dataWaterCoils->WaterCoil(1).MaxWaterMassFlowRate, .258323, 0.00001);
    EXPECT_NEAR(state->dataWaterCoils->WaterCoil(1).UACoil, 239.835, 0.01);

    state->dataLoopNodes->Node.deallocate();
    state->dataHeatBal->Zone.deallocate();
    state->dataPlnt->PlantLoop.deallocate();
    state->dataSize->PlantSizData.deallocate();
    state->dataWaterCoils->MySizeFlag.deallocate();
    state->dataWaterCoils->MyUAAndFlowCalcFlag.deallocate();
    state->dataSize->FinalSysSizing.deallocate();
    state->dataSize->UnitarySysEqSizing.deallocate();
    state->dataAirSystemsData->PrimaryAirSystems.deallocate();
    state->dataAirLoop->AirLoopControlInfo.deallocate();
}

TEST_F(EnergyPlusFixture, TestSizingRoutineForHotWaterCoils6)
{

    // test whether autosizing of zone reheat coil works for performance input method = UFactorTimesAreaAndDesignWaterFlowRate,
    // UA and water flow are user input with rated capacity autosized. No sizing run.

    bool ErrorsFound(false);

    state->dataEnvrn->StdRhoAir = 1.20;

    std::string const idf_objects = delimited_string({

        " Zone,",
        "	SPACE1-1,      !- Name",
        "	0,             !- Direction of Relative North { deg }",
        "	0,             !- X Origin { m }",
        "	0,             !- Y Origin { m }",
        "	0,             !- Z Origin { m }",
        "	1,             !- Type",
        "	1,             !- Multiplier",
        "	2.438400269,   !- Ceiling Height {m}",
        "	239.247360229; !- Volume {m3}",

        " Sizing:Zone,",
        "	SPACE1-1,      !- Zone or ZoneList Name",
        "	SupplyAirTemperature, !- Zone Cooling Design Supply Air Temperature Input Method",
        "	14.,           !- Zone Cooling Design Supply Air Temperature { C }",
        "	,              !- Zone Cooling Design Supply Air Temperature Difference { deltaC }",
        "	SupplyAirTemperature, !- Zone Heating Design Supply Air Temperature Input Method",
        "	50.,           !- Zone Heating Design Supply Air Temperature { C }",
        "	,              !- Zone Heating Design Supply Air Temperature Difference { deltaC }",
        "	0.009,         !- Zone Cooling Design Supply Air Humidity Ratio { kgWater/kgDryAir }",
        "	0.004,         !- Zone Heating Design Supply Air Humidity Ratio { kgWater/kgDryAir }",
        "	SZ DSOA SPACE1-1, !- Design Specification Outdoor Air Object Name",
        "	0.0,           !- Zone Heating Sizing Factor",
        "	0.0,           !- Zone Cooling Sizing Factor",
        "	DesignDayWithLimit, !- Cooling Design Air Flow Method",
        "	,              !- Cooling Design Air Flow Rate { m3/s }",
        "	,              !- Cooling Minimum Air Flow per Zone Floor Area { m3/s-m2 }",
        "	,              !- Cooling Minimum Air Flow { m3/s }",
        "	,              !- Cooling Minimum Air Flow Fraction",
        "	DesignDay,     !- Heating Design Air Flow Method",
        "	,              !- Heating Design Air Flow Rate { m3/s }",
        "	,              !- Heating Maximum Air Flow per Zone Floor Area { m3/s-m2 }",
        "	,              !- Heating Maximum Air Flow { m3/s }",
        "	,              !- Heating Maximum Air Flow Fraction",
        "	SZ DZAD SPACE1-1;   !- Design Specification Zone Air Distribution Object Name",

        " DesignSpecification:ZoneAirDistribution,",
        "	SZ DZAD SPACE1-1, !- Name",
        "	1,                !- Zone Air Distribution Effectiveness in Cooling Mode { dimensionless }",
        "	1;                !- Zone Air Distribution Effectiveness in Heating Mode { dimensionless }",

        " DesignSpecification:OutdoorAir,",
        "	SZ DSOA SPACE1-1, !- Name",
        "	sum,              !- Outdoor Air Method",
        "	0.00236,          !- Outdoor Air Flow per Person { m3/s-person }",
        "	0.000305,         !- Outdoor Air Flow per Zone Floor Area { m3/s-m2 }",
        "	0.0;              !- Outdoor Air Flow per Zone { m3/s }",

        " ScheduleTypeLimits,",
        "	Fraction,         !- Name",
        "	0.0,              !- Lower Limit Value",
        "	1.0,              !- Upper Limit Value",
        "	CONTINUOUS;       !- Numeric Type",

        " Schedule:Compact,",
        "	ReheatCoilAvailSched, !- Name",
        "	Fraction,         !- Schedule Type Limits Name",
        "	Through: 12/31,   !- Field 1",
        "	For: AllDays,     !- Field 2",
        "	Until: 24:00,1.0; !- Field 3",

        " ZoneHVAC:EquipmentConnections,",
        "	SPACE1-1,         !- Zone Name",
        "	SPACE1-1 Eq,      !- Zone Conditioning Equipment List Name",
        "	SPACE1-1 In Node, !- Zone Air Inlet Node or NodeList Name",
        "	,                 !- Zone Air Exhaust Node or NodeList Name",
        "	SPACE1-1 Node,    !- Zone Air Node Name",
        "	SPACE1-1 Out Node; !- Zone Return Air Node Name",

        " ZoneHVAC:EquipmentList,",
        "	SPACE1-1 Eq,      !- Name",
        "   SequentialLoad,   !- Load Distribution Scheme",
        "	ZoneHVAC:AirDistributionUnit, !- Zone Equipment 1 Object Type",
        "	SPACE1-1 ATU,     !- Zone Equipment 1 Name",
        "	1,                !- Zone Equipment 1 Cooling Sequence",
        "	1;                !- Zone Equipment 1 Heating or No - Load Sequence",

        " ZoneHVAC:AirDistributionUnit,",
        "	SPACE1-1 ATU,     !- Name",
        "	SPACE1-1 In Node, !- Air Distribution Unit Outlet Node Name",
        "	AirTerminal:SingleDuct:VAV:Reheat, !- Air Terminal Object Type",
        "	SPACE1-1 VAV Reheat; !- Air Terminal Name",

        " Coil:Heating:Water,",
        "	Gronk1 Zone Coil, !- Name",
        "	ReheatCoilAvailSched, !- Availability Schedule Name",
        "	300.,             !- U-Factor Times Area Value { W/K }",
        "	0.0000850575,     !- Maximum Water Flow Rate { m3/s }",
        "	SPACE1-1 Zone Coil Water In Node, !- Water Inlet Node Name",
        "	SPACE1-1 Zone Coil Water Out Node, !- Water Outlet Node Name",
        "	SPACE1-1 Zone Coil Air In Node, !- Air Inlet Node Name",
        "	SPACE1-1 In Node, !- Air Outlet Node Name",
        "	UFactorTimesAreaAndDesignWaterFlowRate, !- Performance Input Method",
        "	autosize,         !- Rated Capacity { W }",
        "	82.2,             !- Rated Inlet Water Temperature { C }",
        "	16.6,             !- Rated Inlet Air Temperature { C }",
        "	71.1,             !- Rated Outlet Water Temperature { C }",
        "	32.2,             !- Rated Outlet Air Temperature { C }",
        "	;                 !- Rated Ratio for Air and Water Convection",

        " AirTerminal:SingleDuct:VAV:Reheat,",
        "	SPACE1-1 VAV Reheat,  !- Name",
        "	ReheatCoilAvailSched, !- Availability Schedule Name",
        "	SPACE1-1 Zone Coil Air In Node, !- Damper Air Outlet Node Name",
        "	SPACE1-1 ATU In Node, !- Air Inlet Node Name",
        "	0.12046,              !- Maximum Air Flow Rate { m3/s }",
        "	,                     !- Zone Minimum Air Flow Input Method",
        "	0.3,                  !- Constant Minimum Air Flow Fraction",
        "	,                     !- Fixed Minimum Air Flow Rate { m3/s }",
        "	,                     !- Minimum Air Flow Fraction Schedule Name",
        "	Coil:Heating:Water,   !- Reheat Coil Object Type",
        "	Gronk1 Zone Coil,     !- Reheat Coil Name",
        "	0.0000850575,         !- Maximum Hot Water or Steam Flow Rate { m3/s }",
        "	0.0,                  !- Minimum Hot Water or Steam Flow Rate { m3/s }",
        "	SPACE1-1 In Node,     !- Air Outlet Node Name",
        "	0.001,                !- Convergence Tolerance",
        "	,                     !- Damper Heating Action",
        "	,                     !- Maximum Flow per Zone Floor Area During Reheat { m3/s-m2 }",
        "	;                     !- Maximum Flow Fraction During Reheat",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataSize->TermUnitSizing.allocate(1);
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    state->dataWaterCoils->MySizeFlag.allocate(1);
    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(1);
    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {

        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }
    GetZoneData(*state, ErrorsFound);
    EXPECT_EQ("SPACE1-1", state->dataHeatBal->Zone(1).Name);
    GetOARequirements(*state);      // get the OA requirements object
    GetZoneAirDistribution(*state); // get zone air distribution objects
    GetZoneSizingInput(*state);
    GetZoneEquipmentData(*state);
    GetZoneAirLoopEquipment(*state);
    GetWaterCoilInput(*state);
    state->dataWaterCoils->GetWaterCoilsInputFlag = false;
    state->dataWaterCoils->MySizeFlag(1) = true;
    state->dataWaterCoils->MyUAAndFlowCalcFlag(1) = false;
    GetSysInput(*state);
    state->dataSize->TermUnitSingDuct = true;

    state->dataPlnt->PlantLoop(1).Name = "HotWaterLoop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";

    state->dataWaterCoils->WaterCoil(1).WaterPlantLoc = {1, DataPlant::LoopSideLocation::Demand, 1, 1};
    PlantUtilities::SetPlantLocationLinks(*state, state->dataWaterCoils->WaterCoil(1).WaterPlantLoc);

    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = state->dataWaterCoils->WaterCoil(1).Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterCoils->WaterCoil(1).WaterInletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut =
        state->dataWaterCoils->WaterCoil(1).WaterOutletNodeNum;

    state->dataSingleDuct->sd_airterminal(1).HWplantLoc = {1, DataPlant::LoopSideLocation::Demand, 1, 0};
    PlantUtilities::SetPlantLocationLinks(*state, state->dataSingleDuct->sd_airterminal(1).HWplantLoc);

    state->dataSize->CurZoneEqNum = 1;
    state->dataSize->CurTermUnitSizingNum = 1;
    state->dataSize->CurSysNum = 0;
    state->dataHeatBal->Zone(1).FloorArea = 99.16;
    state->dataSingleDuct->sd_airterminal(1).ZoneFloorArea = state->dataHeatBal->Zone(1).FloorArea;

    state->dataSingleDuct->sd_airterminal(1).SizeSys(*state);
    state->dataGlobal->BeginEnvrnFlag = true;

    // water coil is user input for water flow and UA with performance input method = UFactorTimesAreaAndDesignWaterFlowRate and Rated Capacity =
    // autosize
    EXPECT_NEAR(state->dataWaterCoils->WaterCoil(1).MaxWaterVolFlowRate, .0000850575, 0.000000001); // water flow rate input by user
    EXPECT_NEAR(state->dataWaterCoils->WaterCoil(1).UACoil, 300.00, 0.01);                          // Ua input by user
    EXPECT_EQ(state->dataWaterCoils->WaterCoil(1).DesTotWaterCoilLoad, DataSizing::AutoSize);       // Rated Capacity input by user
    EXPECT_EQ(state->dataWaterCoils->WaterCoil(1).DesWaterHeatingCoilRate, 0.0);                    // model output not yet set

    // sizing will be called and skipped with Init setting DesWaterHeatingCoilRate based on above inputs
    InitWaterCoil(*state, 1, false);
    EXPECT_NEAR(state->dataWaterCoils->WaterCoil(1).DesWaterHeatingCoilRate, 7390.73, 0.01);
    // not set in Init for water heating coils and not used elsewhere other than sizing
    EXPECT_EQ(state->dataWaterCoils->WaterCoil(1).DesTotWaterCoilLoad, DataSizing::AutoSize);

    state->dataSingleDuct->sd_airterminal.deallocate();
}

} // namespace EnergyPlus
