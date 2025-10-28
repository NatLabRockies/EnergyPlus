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

// EnergyPlus::Evaporative Cooler Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/EvaporativeFluidCoolers.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/SimAirServingZones.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;
using namespace EnergyPlus::DataEnvironment;
using namespace EnergyPlus::EvaporativeFluidCoolers;

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, EvapFluidCoolerSpecs_getDesignCapacitiesTest)
{
    state->init_state(*state);
    Real64 MaxLoad;
    Real64 MinLoad;
    Real64 OptLoad;
    Real64 ExpectedMaxLoad;
    Real64 ExpectedMinLoad;
    Real64 ExpectedOptLoad;

    state->dataEnvrn->OutDryBulbTemp = 20.0;
    state->dataEnvrn->OutHumRat = 0.02;
    state->dataEnvrn->OutBaroPress = 101325.;
    state->dataEnvrn->OutWetBulbTemp = 8.0;

    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).FlowLock = DataPlant::FlowLock::Locked;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).MyLoad = 1.0;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).ON = false;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).CurOpSchemeType = DataPlant::OpScheme::Invalid;
    state->dataPlnt->PlantLoop(1).PlantSizNum = 1;
    state->dataPlnt->PlantFinalSizesOkayToReport = false;
    state->dataSize->SaveNumPlantComps = 0;

    state->dataSize->PlantSizData.allocate(1);
    state->dataSize->PlantSizData(1).DeltaT = 5.0;
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataSize->PlantSizData(1).ExitTemp = 20.0;

    // Set up information required to actually run the routines that get called as a result of running this test.
    // In general, values set here attempt to avoid as much code as possible so that only the defect code is run.
    // Obviously, not everything can be skipped so some of this information is needed to avoid crashes in other routines.
    state->dataEvapFluidCoolers->SimpleEvapFluidCooler.allocate(1);
    auto &thisEFC = state->dataEvapFluidCoolers->SimpleEvapFluidCooler(1);
    thisEFC.Type = DataPlant::PlantEquipmentType::EvapFluidCooler_TwoSpd;
    thisEFC.MyOneTimeFlag = false;
    thisEFC.OneTimeFlagForEachEvapFluidCooler = false;
    thisEFC.MyEnvrnFlag = false;
    state->dataGlobal->BeginEnvrnFlag = true;
    thisEFC.WaterInletNodeNum = 1;
    thisEFC.WaterOutletNodeNum = 2;
    thisEFC.OutdoorAirInletNodeNum = 0;
    thisEFC.plantLoc.loopNum = 1;
    thisEFC.plantLoc.loopSideNum = DataPlant::LoopSideLocation::Supply;
    thisEFC.plantLoc.branchNum = 1;
    thisEFC.plantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, thisEFC.plantLoc);

    thisEFC.DesignWaterFlowRateWasAutoSized = false;
    thisEFC.LowSpeedAirFlowRateWasAutoSized = false;
    thisEFC.HighSpeedEvapFluidCoolerUAWasAutoSized = false;
    thisEFC.PerformanceInputMethod_Num = PIM::UFactor;

    thisEFC.DesignWaterFlowRate = 0.001;

    state->dataLoopNodes->Node.allocate(2);
    state->dataLoopNodes->Node(thisEFC.WaterInletNodeNum).Temp = 20.0;
    state->dataLoopNodes->Node(1).Temp = 23.0;
    state->dataLoopNodes->Node(1).MassFlowRateRequest = 0.05;
    state->dataLoopNodes->Node(1).MassFlowRateMinAvail = 0.0;
    state->dataLoopNodes->Node(1).MassFlowRateMin = 0.0;
    state->dataLoopNodes->Node(1).MassFlowRateMax = 0.05;
    state->dataLoopNodes->Node(1).MassFlowRateMaxAvail = 0.05;

    // Now set the specific data for the actual test
    MaxLoad = 0.0;
    OptLoad = 0.0;
    MinLoad = 999.9;
    thisEFC.HighSpeedStandardDesignCapacity = 1.0;
    thisEFC.HeatRejectCapNomCapSizingRatio = 2.0;
    ExpectedMaxLoad = 20902.8677;
    ExpectedOptLoad = 10451.4338;
    ExpectedMinLoad = 0.0;

    // Call the routine to be tested and see if the fix is correct
    PlantLocation loc = PlantLocation(1, DataPlant::LoopSideLocation::Supply, 1, 1);
    PlantUtilities::SetPlantLocationLinks(*state, loc);

    thisEFC.onInitLoopEquip(*state, loc);
    thisEFC.getDesignCapacities(*state, loc, MaxLoad, MinLoad, OptLoad);
    EXPECT_NEAR(MaxLoad, ExpectedMaxLoad, 0.01);
    EXPECT_NEAR(MinLoad, ExpectedMinLoad, 0.01);
    EXPECT_NEAR(OptLoad, ExpectedOptLoad, 0.01);
}

TEST_F(EnergyPlusFixture, ExerciseSingleSpeedEvapFluidCooler)
{
    std::string const idf_objects = delimited_string({"EvaporativeFluidcooler:SingleSpeed,",
                                                      "Big EvaporativeFluidCooler,  !- Name",
                                                      "Condenser EvaporativeFluidcooler Inlet Node,  !- Water Inlet Node Name",
                                                      "Condenser EvaporativeFluidcooler Outlet Node,  !- Water Outlet Node Name",
                                                      "3.02,                    !- Design Air Flow Rate {m3/s}",
                                                      "2250,                    !- Design Air Flow Rate Fan Power {W}",
                                                      "0.002208,                !- Design Spray Water Flow Rate {m3/s}",
                                                      "UserSpecifiedDesignCapacity,  !- Performance Input Method",
                                                      ",                        !- Outdoor Air Inlet Node Name",
                                                      ",                        !- Heat Rejection Capacity and Nominal Capacity Sizing Ratio",
                                                      "1000,                        !- Standard Design Capacity {W}",
                                                      ",                        !- Design Air Flow Rate U-factor Times Area Value {W/K}",
                                                      "0.001703,                !- Design Water Flow Rate {m3/s}",
                                                      "87921,                   !- User Specified Design Capacity {W}",
                                                      "46.11,                   !- Design Entering Water Temperature {C}",
                                                      "35,                      !- Design Entering Air Temperature {C}",
                                                      "25.6;                    !- Design Entering Air Wet-bulb Temperature {C}"});

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    EvapFluidCoolerSpecs *ptr =
        EvapFluidCoolerSpecs::factory(*state, DataPlant::PlantEquipmentType::EvapFluidCooler_SingleSpd, "BIG EVAPORATIVEFLUIDCOOLER");

    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);

    PlantLocation loc{1, EnergyPlus::DataPlant::LoopSideLocation::Supply, 1, 1};
    PlantUtilities::SetPlantLocationLinks(*state, loc);

    Real64 max, opt, min = 0.0;
    ptr->getDesignCapacities(*state, loc, max, min, opt);
    EXPECT_NEAR(max, 1250, 1.0);
    EXPECT_NEAR(min, 0.0, 1.0);
    EXPECT_NEAR(opt, 1000.0, 1.0);

    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).TempSetPoint = 2.0;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).MyLoad = 1000;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).ON = true;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).NodeNumIn = ptr->WaterInletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).NodeNumOut = ptr->WaterOutletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::EvapFluidCooler_SingleSpd;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).Name = ptr->Name;
    state->dataPlnt->PlantLoop(1).MaxVolFlowRate = 3;
    state->dataPlnt->PlantLoop(1).MaxMassFlowRate = 3;
    state->dataSize->CurLoopNum = 1;

    state->dataLoopNodes->Node(ptr->WaterOutletNodeNum).MassFlowRateMaxAvail = 5;
    state->dataLoopNodes->Node(ptr->WaterOutletNodeNum).MassFlowRateMax = 5;
    state->dataLoopNodes->Node(ptr->WaterInletNodeNum).Temp = 20;
    state->dataLoopNodes->Node(ptr->WaterInletNodeNum).MassFlowRateMaxAvail = 5;
    state->dataLoopNodes->Node(ptr->WaterInletNodeNum).MassFlowRateMax = 5;

    bool firstHVAC = true;
    Real64 curLoad = 0.0;
    ptr->plantLoc.loopNum = 1;
    ptr->plantLoc.loopSideNum = EnergyPlus::DataPlant::LoopSideLocation::Supply;
    ptr->plantLoc.branchNum = 1;
    ptr->plantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, ptr->plantLoc);

    ptr->DesWaterMassFlowRate = 3.141;
    ptr->WaterMassFlowRate = 3.141;
    ptr->onInitLoopEquip(*state, loc);
    ptr->simulate(*state, loc, firstHVAC, curLoad, true);
}
TEST_F(EnergyPlusFixture, ExerciseTwoSpeedEvapFluidCooler)
{
    std::string const idf_objects = delimited_string({"EvaporativeFluidCooler:TwoSpeed,",
                                                      "Central Tower,           !- Name",
                                                      "Central Tower Inlet Node,!- Water Inlet Node Name",
                                                      "Central Tower Outlet Node,  !- Water Outlet Node Name",
                                                      "autosize,                !- High Fan Speed Air Flow Rate {m3/s}",
                                                      "autosize,                !- High Fan Speed Fan Power {W}",
                                                      "autocalculate,           !- Low Fan Speed Air Flow Rate {m3/s}",
                                                      ",                        !- Low Fan Speed Air Flow Rate Sizing Factor",
                                                      "autocalculate,           !- Low Fan Speed Fan Power {W}",
                                                      ",                        !- Low Fan Speed Fan Power Sizing Factor",
                                                      "0.002208,                !- Design Spray Water Flow Rate {m3/s}",
                                                      "UFactorTimesAreaAndDesignWaterFlowRate,  !- Performance Input Method",
                                                      ",                        !- Outdoor Air Inlet Node Name",
                                                      "1.25,                    !- Heat Rejection Capacity and Nominal Capacity Sizing Ratio",
                                                      "1000,                        !- High Speed Standard Design Capacity {W}",
                                                      "1000,                        !- Low Speed Standard Design Capacity {W}",
                                                      "0.5,                     !- Low Speed Standard Capacity Sizing Factor",
                                                      "autosize,                !- High Fan Speed U-factor Times Area Value {W/K}",
                                                      "autocalculate,           !- Low Fan Speed U-factor Times Area Value {W/K}",
                                                      "0.6,                     !- Low Fan Speed U-Factor Times Area Sizing Factor",
                                                      "autosize,                !- Design Water Flow Rate {m3/s}",
                                                      ",                        !- High Speed User Specified Design Capacity {W}",
                                                      ",                        !- Low Speed User Specified Design Capacity {W}",
                                                      "0.5,                     !- Low Speed User Specified Design Capacity Sizing Factor",
                                                      ",                        !- Design Entering Water Temperature {C}",
                                                      ",                        !- Design Entering Air Temperature {C}",
                                                      ",                        !- Design Entering Air Wet-bulb Temperature {C}",
                                                      "1,                       !- High Speed Sizing Factor",
                                                      "SaturatedExit,           !- Evaporation Loss Mode",
                                                      ",                        !- Evaporation Loss Factor {percent/K}",
                                                      "0.008,                   !- Drift Loss Percent {percent}",
                                                      "ConcentrationRatio,      !- Blowdown Calculation Mode",
                                                      "3;                       !- Blowdown Concentration Ratio"});

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    EvapFluidCoolerSpecs *ptr = EvapFluidCoolerSpecs::factory(*state, DataPlant::PlantEquipmentType::EvapFluidCooler_TwoSpd, "CENTRAL TOWER");

    PlantLocation pl{1, EnergyPlus::DataPlant::LoopSideLocation::Supply, 1, 1};
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);

    Real64 max, opt, min = 0.0;
    ptr->getDesignCapacities(*state, pl, max, min, opt);
    EXPECT_NEAR(max, 1250, 1.0);
    EXPECT_NEAR(min, 0.0, 1.0);
    EXPECT_NEAR(opt, 1000.0, 1.0);

    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).TempSetPoint = 2.0;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).MyLoad = 1000;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).ON = true;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).NodeNumIn = ptr->WaterInletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).NodeNumOut = ptr->WaterOutletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::EvapFluidCooler_TwoSpd;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).Name = ptr->Name;
    state->dataPlnt->PlantLoop(1).MaxVolFlowRate = 3;
    state->dataPlnt->PlantLoop(1).MaxMassFlowRate = 3;
    state->dataSize->CurLoopNum = 1;

    state->dataPlnt->PlantLoop(pl.loopNum).PlantSizNum = 1;
    state->dataSize->PlantSizData.allocate(1);
    state->dataSize->PlantSizData(1).ExitTemp = 35;

    state->dataLoopNodes->Node(ptr->WaterOutletNodeNum).MassFlowRateMaxAvail = 5;
    state->dataLoopNodes->Node(ptr->WaterOutletNodeNum).MassFlowRateMax = 5;
    state->dataLoopNodes->Node(ptr->WaterInletNodeNum).Temp = 20;
    state->dataLoopNodes->Node(ptr->WaterInletNodeNum).MassFlowRateMaxAvail = 5;
    state->dataLoopNodes->Node(ptr->WaterInletNodeNum).MassFlowRateMax = 5;

    bool firstHVAC = true;
    Real64 curLoad = 0.0;
    ptr->plantLoc.loopNum = 1;
    ptr->plantLoc.loopSideNum = EnergyPlus::DataPlant::LoopSideLocation::Supply;
    ptr->plantLoc.branchNum = 1;
    ptr->plantLoc.compNum = 1;
    ptr->DesWaterMassFlowRate = 3.141;
    ptr->WaterMassFlowRate = 3.141;
    ptr->onInitLoopEquip(*state, pl);
    ptr->simulate(*state, pl, firstHVAC, curLoad, true);
}

TEST_F(EnergyPlusFixture, EvapFluidCooler_SizeWhenPlantSizingIndexIsZeroAndAutosized)
{
    // Test for #10817
    std::string const idf_objects = delimited_string({
        "EvaporativeFluidcooler:SingleSpeed,",
        "  Big EvaporativeFluidCooler,  !- Name",
        "  Condenser EvaporativeFluidcooler Inlet Node,  !- Water Inlet Node Name",
        "  Condenser EvaporativeFluidcooler Outlet Node,  !- Water Outlet Node Name",
        "  Autosize,                !- Design Air Flow Rate {m3/s}",
        "  Autosize,                !- Design Air Flow Rate Fan Power {W}",
        "  0.002208,                !- Design Spray Water Flow Rate {m3/s}",
        "  UFactorTimesAreaAndDesignWaterFlowRate,  !- Performance Input Method",
        "  ,                        !- Outdoor Air Inlet Node Name",
        "  ,                        !- Heat Rejection Capacity and Nominal Capacity Sizing Ratio",
        "  ,                        !- Standard Design Capacity {W}",
        "  Autosize,                !- Design Air Flow Rate U-factor Times Area Value {W/K}",
        "  Autosize,                !- Design Water Flow Rate {m3/s}",
        "  ,                        !- User Specified Design Capacity {W}",
        "  46.11,                   !- Design Entering Water Temperature {C}",
        "  35,                      !- Design Entering Air Temperature {C}",
        "  25.6;                    !- Design Entering Air Wet-bulb Temperature {C}",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    EvapFluidCoolerSpecs *ptr =
        EvapFluidCoolerSpecs::factory(*state, DataPlant::PlantEquipmentType::EvapFluidCooler_SingleSpd, "BIG EVAPORATIVEFLUIDCOOLER");

    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->PlantLoop(1).PlantSizNum = 0;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);

    ptr->plantLoc.loopNum = 1;
    ptr->plantLoc.loopSideNum = DataPlant::LoopSideLocation::Supply;
    ptr->plantLoc.branchNum = 1;
    ptr->plantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, ptr->plantLoc);

    // Necessary to trigger the crash from #
    state->dataPlnt->PlantFirstSizesOkayToFinalize = false;

    EXPECT_TRUE(ptr->DesignWaterFlowRateWasAutoSized);
    EXPECT_TRUE(ptr->HighSpeedAirFlowRateWasAutoSized);
    EXPECT_TRUE(ptr->HighSpeedFanPowerWasAutoSized);
    EXPECT_TRUE(ptr->HighSpeedEvapFluidCoolerUAWasAutoSized);

    ptr->SizeEvapFluidCooler(*state);
}

} // namespace EnergyPlus
