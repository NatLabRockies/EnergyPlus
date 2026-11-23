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

// EnergyPlus::WaterCoils Unit Tests
// FUNCTION INFORMATION:
//       AUTHOR         R Raustad
//       DATE WRITTEN   March 2015
//       MODIFIED
//       RE-ENGINEERED  na

// PURPOSE OF THIS FUNCTION:
// This function performs the sizing calculation on a chilled water coil.
// The water coil should size to use sizing data or Data* globals. After a coil is sized, the Data* variables should all be reset.
// Defect file showed a second coil in the input that autosized the same as the first (incorrect based on zone flow rates).

// C++ Headers
#include <format>

// Google Test Headers
#include <gtest/gtest.h>

// Fixture Headers
#include "Fixtures/EnergyPlusFixture.hh"

// EnergyPlus Headers
#include <EnergyPlus/Boilers.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
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
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/Pumps.hh>
#include <EnergyPlus/ReportCoilSelection.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/SizingManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterCoils.hh>

using namespace EnergyPlus;
using namespace DataAirLoop;
using namespace DataAirSystems;
using namespace DataEnvironment;
using namespace EnergyPlus::DataPlant;
using namespace EnergyPlus::DataSizing;
using namespace EnergyPlus::DataHeatBalance;
using namespace EnergyPlus::DataHeatBalFanSys;
using namespace EnergyPlus::DataZoneEnergyDemands;
using namespace EnergyPlus::DataZoneEquipment;
using namespace EnergyPlus::FanCoilUnits;
using namespace EnergyPlus::Fans;
using namespace EnergyPlus::General;
using namespace EnergyPlus::GlobalNames;
using namespace EnergyPlus::HeatBalanceManager;
using namespace EnergyPlus::MixedAir;
using namespace EnergyPlus::Psychrometrics;
using namespace EnergyPlus::SizingManager;
using namespace EnergyPlus::WaterCoils;
using namespace EnergyPlus::Psychrometrics;

class WaterCoilsTest : public EnergyPlusFixture
{

public:
    //    static void SetUpTestCase()
    //    {
    //        EnergyPlusFixture::SetUpTestCase(); // Sets up the base fixture
    //    }
    static void TearDownTestCase()
    {
    }

    void SetUp() override
    {
        EnergyPlusFixture::SetUp(); // Sets up individual test cases.

        state->dataSize->CurZoneEqNum = 0;
        state->dataSize->CurSysNum = 0;
        state->dataSize->CurOASysNum = 0;
        state->dataWaterCoils->NumWaterCoils = 1;
        state->dataWaterCoils->WaterCoil.allocate(state->dataWaterCoils->NumWaterCoils);
        state->dataWaterCoils->WaterCoilNumericFields.allocate(state->dataWaterCoils->NumWaterCoils);
        state->dataWaterCoils->WaterCoilNumericFields(state->dataWaterCoils->NumWaterCoils).FieldNames.allocate(17); // max N fields for water coil
        state->dataPlnt->TotNumLoops = 1;
        state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
        state->dataSize->PlantSizData.allocate(1);
        state->dataSize->ZoneEqSizing.allocate(1);
        state->dataSize->UnitarySysEqSizing.allocate(1);
        state->dataSize->OASysEqSizing.allocate(1);
        state->dataSize->SysSizInput.allocate(1);
        state->dataSize->ZoneSizingInput.allocate(1);
        state->dataSize->SysSizPeakDDNum.allocate(1);
        state->dataSize->SysSizPeakDDNum(1).TimeStepAtSensCoolPk.allocate(1);
        state->dataSize->SysSizPeakDDNum(1).TimeStepAtCoolFlowPk.allocate(1);
        state->dataSize->SysSizPeakDDNum(1).TimeStepAtTotCoolPk.allocate(1);
        state->dataSize->SysSizPeakDDNum(1).SensCoolPeakDD = 1;
        state->dataSize->SysSizPeakDDNum(1).CoolFlowPeakDD = 1;
        state->dataSize->SysSizPeakDDNum(1).TotCoolPeakDD = 1;
        state->dataSize->FinalSysSizing.allocate(1);
        state->dataSize->CalcSysSizing.allocate(1);
        state->dataSize->FinalZoneSizing.allocate(1);
        state->dataAirSystemsData->PrimaryAirSystems.allocate(1);
        state->dataAirLoop->AirLoopControlInfo.allocate(1);
    }

    void TearDown() override
    {
        EnergyPlusFixture::TearDown(); // Remember to tear down the base fixture after cleaning up derived fixture!
    }
};

TEST_F(WaterCoilsTest, WaterCoolingCoilSizing)
{
    Fluid::GetFluidPropertiesData(*state);
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->OutBaroPress, 20.0, 0.0);

    // set up sizing flags
    state->dataSize->SysSizingRunDone = true;

    // set up plant sizing
    state->dataSize->NumPltSizInput = 1;
    state->dataSize->PlantSizData(1).PlantLoopName = "WaterLoop";

    // set up plant loop
    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {

        auto &loopside(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }
    state->dataPlnt->PlantLoop(1).Name = "WaterLoop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);

    // set up sizing data
    state->dataSize->FinalSysSizing(1).MixTempAtCoolPeak = 20.0;
    state->dataSize->FinalSysSizing(1).CoolSupTemp = 10.0;
    state->dataSize->FinalSysSizing(1).MixHumRatAtCoolPeak = 0.01;
    state->dataSize->FinalSysSizing(1).DesMainVolFlow = 0.00159;
    state->dataSize->FinalSysSizing(1).HeatSupTemp = 25.0;
    state->dataSize->FinalSysSizing(1).HeatOutTemp = 5.0;
    state->dataSize->FinalSysSizing(1).HeatRetTemp = 20.0;

    // set up water coil
    int CoilNum = 1;
    auto &waterCoil1 = state->dataWaterCoils->WaterCoil(CoilNum);
    waterCoil1.Name = "Test Water Cooling Coil";
    waterCoil1.coilType = HVAC::CoilType::CoolingWater;
    waterCoil1.coilReportNum = ReportCoilSelection::getReportIndex(*state, waterCoil1.Name, waterCoil1.coilType);

    waterCoil1.WaterPlantLoc.loopNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, waterCoil1.WaterPlantLoc);
    waterCoil1.WaterCoilType = DataPlant::PlantEquipmentType::CoilWaterCooling;
    waterCoil1.RequestingAutoSize = true;
    waterCoil1.DesAirVolFlowRate = AutoSize;
    waterCoil1.DesInletAirTemp = AutoSize;
    waterCoil1.DesOutletAirTemp = AutoSize;
    waterCoil1.DesInletWaterTemp = AutoSize;
    waterCoil1.DesInletAirHumRat = AutoSize;
    waterCoil1.DesOutletAirHumRat = AutoSize;
    waterCoil1.MaxWaterVolFlowRate = AutoSize;

    state->dataWaterCoils->WaterCoilNumericFields(CoilNum).FieldNames(3) = "Maximum Flow Rate";
    waterCoil1.WaterInletNodeNum = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = waterCoil1.WaterInletNodeNum;

    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurSysNum = 1;
    state->dataSize->CurOASysNum = 0;
    state->dataSize->SysSizInput(1).CoolCapControl = DataSizing::CapacityControl::VAV;
    state->dataSize->PlantSizData(1).ExitTemp = 5.7;
    state->dataSize->PlantSizData(1).DeltaT = 5.0;
    state->dataSize->FinalSysSizing(1).MassFlowAtCoolPeak = state->dataSize->FinalSysSizing(1).DesMainVolFlow * state->dataEnvrn->StdRhoAir;
    state->dataSize->DataWaterLoopNum = 1;

    SizeWaterCoil(*state, CoilNum);

    EXPECT_DOUBLE_EQ(0.00159, waterCoil1.DesAirVolFlowRate);
    // Check that all Data* variables have been reset
    EXPECT_EQ(0, state->dataSize->DataPltSizCoolNum);
    EXPECT_EQ(0, state->dataSize->DataWaterLoopNum);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataConstantUsedForSizing);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataFractionUsedForSizing);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataAirFlowUsedForSizing);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataFlowUsedForSizing);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataWaterFlowUsedForSizing);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataCapacityUsedForSizing);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataDesInletAirTemp);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataDesOutletAirTemp);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataDesOutletAirHumRat);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataDesInletAirHumRat);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataDesInletWaterTemp);

    // set second cooling coil to size at a different air flow, adjust sizing data, and autosize input data
    state->dataSize->FinalSysSizing(1).DesMainVolFlow = 0.00259;
    state->dataSize->FinalSysSizing(1).MassFlowAtCoolPeak = state->dataSize->FinalSysSizing(1).DesMainVolFlow * state->dataEnvrn->StdRhoAir;

    waterCoil1.Name = "Test Water Cooling Coil 2";
    waterCoil1.DesAirVolFlowRate = AutoSize;
    waterCoil1.DesInletAirTemp = AutoSize;
    waterCoil1.DesOutletAirTemp = AutoSize;
    waterCoil1.DesInletWaterTemp = AutoSize;
    waterCoil1.DesInletAirHumRat = AutoSize;
    waterCoil1.DesOutletAirHumRat = AutoSize;
    waterCoil1.MaxWaterVolFlowRate = AutoSize;

    SizeWaterCoil(*state, CoilNum);

    EXPECT_DOUBLE_EQ(0.00259, waterCoil1.DesAirVolFlowRate);
    EXPECT_EQ(0, state->dataSize->DataPltSizCoolNum);
    EXPECT_EQ(0, state->dataSize->DataWaterLoopNum);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataConstantUsedForSizing);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataFractionUsedForSizing);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataAirFlowUsedForSizing);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataFlowUsedForSizing);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataWaterFlowUsedForSizing);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataCapacityUsedForSizing);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataDesInletAirTemp);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataDesOutletAirTemp);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataDesOutletAirHumRat);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataDesInletAirHumRat);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataDesInletWaterTemp);

    // size heating coil
    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurSysNum = 1;
    state->dataSize->CurOASysNum = 0;
    state->dataSize->FinalSysSizing(1).DesMainVolFlow = 0.00359;
    state->dataSize->FinalSysSizing(1).MassFlowAtCoolPeak = state->dataSize->FinalSysSizing(1).DesMainVolFlow * state->dataEnvrn->StdRhoAir;
    state->dataAirLoop->AirLoopControlInfo(1).UnitarySys = true;

    waterCoil1.Name = "Test Water Heating Coil";
    waterCoil1.WaterCoilType = DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
    waterCoil1.DesAirVolFlowRate = AutoSize;
    waterCoil1.DesInletAirTemp = AutoSize;
    waterCoil1.DesOutletAirTemp = AutoSize;
    waterCoil1.DesInletWaterTemp = AutoSize;
    waterCoil1.DesInletAirHumRat = AutoSize;
    waterCoil1.DesOutletAirHumRat = AutoSize;
    waterCoil1.MaxWaterVolFlowRate = AutoSize;

    SizeWaterCoil(*state, CoilNum);

    EXPECT_DOUBLE_EQ(0.00359, waterCoil1.DesAirVolFlowRate);
    EXPECT_EQ(0, state->dataSize->DataPltSizCoolNum);
    EXPECT_EQ(0, state->dataSize->DataWaterLoopNum);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataConstantUsedForSizing);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataFractionUsedForSizing);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataAirFlowUsedForSizing);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataFlowUsedForSizing);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataWaterFlowUsedForSizing);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataCapacityUsedForSizing);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataDesInletAirTemp);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataDesOutletAirTemp);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataDesOutletAirHumRat);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataDesInletAirHumRat);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataDesInletWaterTemp);

    // size 2nd heating coil
    state->dataSize->FinalSysSizing(1).DesMainVolFlow = 0.00459;
    state->dataSize->FinalSysSizing(1).MassFlowAtCoolPeak = state->dataSize->FinalSysSizing(1).DesMainVolFlow * state->dataEnvrn->StdRhoAir;

    waterCoil1.Name = "Test Water Heating Coil 2";
    waterCoil1.WaterCoilType = DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
    waterCoil1.DesAirVolFlowRate = AutoSize;
    waterCoil1.DesInletAirTemp = AutoSize;
    waterCoil1.DesOutletAirTemp = AutoSize;
    waterCoil1.DesInletWaterTemp = AutoSize;
    waterCoil1.DesInletAirHumRat = AutoSize;
    waterCoil1.DesOutletAirHumRat = AutoSize;
    waterCoil1.MaxWaterVolFlowRate = AutoSize;

    SizeWaterCoil(*state, CoilNum);

    EXPECT_DOUBLE_EQ(0.00459, waterCoil1.DesAirVolFlowRate);
    EXPECT_EQ(0, state->dataSize->DataPltSizCoolNum);
    EXPECT_EQ(0, state->dataSize->DataWaterLoopNum);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataConstantUsedForSizing);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataFractionUsedForSizing);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataAirFlowUsedForSizing);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataFlowUsedForSizing);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataWaterFlowUsedForSizing);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataCapacityUsedForSizing);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataDesInletAirTemp);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataDesOutletAirTemp);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataDesOutletAirHumRat);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataDesInletAirHumRat);
    EXPECT_DOUBLE_EQ(0.0, state->dataSize->DataDesInletWaterTemp);

    // size zone water heating coil
    state->dataSize->CurZoneEqNum = 1;
    state->dataSize->CurSysNum = 0;
    state->dataSize->PlantSizData(1).ExitTemp = 60.0;
    state->dataSize->NumZoneSizingInput = 1;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->ZoneSizingInput(1).ZoneNum = state->dataSize->CurZoneEqNum;
    state->dataSize->ZoneEqSizing(1).SizingMethod.allocate(25);
    state->dataSize->ZoneEqSizing(1).SizingMethod(HVAC::SystemAirflowSizing) = DataSizing::SupplyAirFlowRate;
    state->dataSize->FinalZoneSizing(1).ZoneTempAtHeatPeak = 20.0;
    state->dataSize->FinalZoneSizing(1).OutTempAtHeatPeak = -20.0;
    state->dataSize->FinalZoneSizing(1).DesHeatCoilInTemp = -20.0; // simulates zone heating air flow rate <= zone OA flow rate
    state->dataSize->FinalZoneSizing(1).DesHeatCoilInHumRat = 0.005;
    state->dataSize->FinalZoneSizing(1).HeatDesTemp = 30.0;
    state->dataSize->FinalZoneSizing(1).HeatDesHumRat = 0.005;
    state->dataSize->ZoneEqSizing(1).OAVolFlow = 0.01;
    state->dataSize->ZoneEqSizing(1).HeatingAirFlow = true;
    state->dataSize->ZoneEqSizing(1).HeatingAirVolFlow = 0.1;
    state->dataSize->FinalZoneSizing(1).DesHeatMassFlow = state->dataEnvrn->StdRhoAir * state->dataSize->ZoneEqSizing(1).HeatingAirVolFlow;
    state->dataWaterCoils->MySizeFlag.allocate(1);
    state->dataWaterCoils->MySizeFlag(CoilNum) = true;
    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(1);
    state->dataWaterCoils->MyUAAndFlowCalcFlag(CoilNum) = true;

    waterCoil1.UACoil = AutoSize;
    waterCoil1.DesTotWaterCoilLoad = AutoSize;
    waterCoil1.DesAirVolFlowRate = AutoSize;
    waterCoil1.DesInletAirTemp = AutoSize;
    waterCoil1.DesOutletAirTemp = AutoSize;
    waterCoil1.DesInletWaterTemp = AutoSize;
    waterCoil1.DesInletAirHumRat = AutoSize;
    waterCoil1.DesOutletAirHumRat = AutoSize;
    waterCoil1.MaxWaterVolFlowRate = AutoSize;

    SizeWaterCoil(*state, CoilNum);
    EXPECT_NEAR(waterCoil1.InletAirTemp, 16.0, 0.0001); // a mixture of zone air (20 C) and 10% OA (-20 C) = 16 C
    EXPECT_NEAR(waterCoil1.DesTotWaterCoilLoad, 1709.8638, 0.0001);
    EXPECT_NEAR(waterCoil1.UACoil, 51.2456, 0.0001);
    EXPECT_NEAR(waterCoil1.OutletAirTemp, 30.1302, 0.0001); // reasonable delta T above inlet air temp
}

TEST_F(WaterCoilsTest, TdbFnHRhPbTest)
{

    // using IP PsyCalc
    //   http://linricsoftw.web701.discountasp.net/webpsycalc.aspx

    EXPECT_NEAR(25.0, TdbFnHRhPb(*state, 45170., 0.40, 101312.), 0.05);
    EXPECT_NEAR(20.0, TdbFnHRhPb(*state, 34760., 0.40, 101312.), 0.05);
    EXPECT_NEAR(25.0, TdbFnHRhPb(*state, 50290., 0.50, 101312.), 0.05);
    EXPECT_NEAR(20.0, TdbFnHRhPb(*state, 38490., 0.50, 101312.), 0.05);
}

TEST_F(WaterCoilsTest, CoilHeatingWaterUASizing)
{
    Fluid::GetFluidPropertiesData(*state);
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->OutBaroPress, 20.0, 0.0);

    // set up sizing flags
    state->dataSize->SysSizingRunDone = true;

    // set up plant sizing
    state->dataSize->NumPltSizInput = 1;
    state->dataSize->PlantSizData(1).PlantLoopName = "HotWaterLoop";
    state->dataSize->PlantSizData(1).ExitTemp = 60.0;
    state->dataSize->PlantSizData(1).DeltaT = 10.0;

    // set up plant loop
    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {

        auto &loopside(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }
    state->dataPlnt->PlantLoop(1).Name = "HotWaterLoop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);

    // set up sizing data
    state->dataSize->FinalSysSizing(1).DesMainVolFlow = 1.00;
    state->dataSize->FinalSysSizing(1).HeatSupTemp = 40.0;
    state->dataSize->FinalSysSizing(1).HeatOutTemp = 5.0;
    state->dataSize->FinalSysSizing(1).HeatRetTemp = 20.0;
    state->dataSize->FinalSysSizing(1).HeatOAOption = DataSizing::OAControl::AllOA;

    // set up water coil
    int CoilNum = 1;
    auto &waterCoil1 = state->dataWaterCoils->WaterCoil(CoilNum);
    waterCoil1.Name = "Water Heating Coil";
    waterCoil1.WaterPlantLoc.loopNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, waterCoil1.WaterPlantLoc);
    waterCoil1.WaterCoilType = DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
    waterCoil1.WaterCoilModel = WaterCoils::CoilModel::HeatingSimple;
    waterCoil1.availSched = Sched::GetScheduleAlwaysOn(*state);

    waterCoil1.RequestingAutoSize = true;
    waterCoil1.DesAirVolFlowRate = AutoSize;
    waterCoil1.UACoil = AutoSize;
    waterCoil1.MaxWaterVolFlowRate = AutoSize;
    waterCoil1.CoilPerfInpMeth = state->dataWaterCoils->UAandFlow;
    waterCoil1.DesInletAirTemp = AutoSize;
    waterCoil1.DesOutletAirTemp = AutoSize;
    waterCoil1.DesInletWaterTemp = AutoSize;
    waterCoil1.DesInletAirHumRat = AutoSize;
    waterCoil1.DesOutletAirHumRat = AutoSize;

    state->dataWaterCoils->WaterCoilNumericFields(CoilNum).FieldNames(2) = "Maximum Water Flow Rate";
    waterCoil1.WaterInletNodeNum = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = waterCoil1.WaterInletNodeNum;

    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurSysNum = 1;
    state->dataSize->CurOASysNum = 0;
    state->dataSize->DataWaterLoopNum = 1;

    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(1);
    state->dataWaterCoils->MyUAAndFlowCalcFlag(1) = true;

    state->dataWaterCoils->MySizeFlag.allocate(1);
    state->dataWaterCoils->MySizeFlag(1) = true;

    // run water coil sizing
    SizeWaterCoil(*state, CoilNum);
    EXPECT_DOUBLE_EQ(1.0, waterCoil1.DesAirVolFlowRate);

    Real64 CpAirStd = 0.0;
    Real64 DesMassFlow = 0.0;
    Real64 DesCoilHeatingLoad = 0.0;

    CpAirStd = PsyCpAirFnW(0.0);
    DesMassFlow = waterCoil1.DesAirVolFlowRate * state->dataEnvrn->StdRhoAir;
    DesCoilHeatingLoad = CpAirStd * DesMassFlow * (40.0 - 5.0);

    // check heating coil design load
    EXPECT_DOUBLE_EQ(DesCoilHeatingLoad, waterCoil1.DesWaterHeatingCoilRate);

    Real64 Cp = 0;
    Real64 rho = 0;
    Real64 DesWaterFlowRate = 0;

    Cp = state->dataPlnt->PlantLoop(1).glycol->getSpecificHeat(*state, Constant::HWInitConvTemp, "Unit Test");
    rho = state->dataPlnt->PlantLoop(1).glycol->getDensity(*state, Constant::HWInitConvTemp, "Unit Test");
    DesWaterFlowRate = waterCoil1.DesWaterHeatingCoilRate / (10.0 * Cp * rho);

    // check heating coil design water flow rate
    EXPECT_DOUBLE_EQ(DesWaterFlowRate, waterCoil1.MaxWaterVolFlowRate);

    // check coil UA-value sizing
    EXPECT_NEAR(1435.01, waterCoil1.UACoil, 0.01);

    // test single zone VAV reheat coil sizing
    state->dataSize->CurZoneEqNum = 1;
    state->dataSize->CurTermUnitSizingNum = 1;
    state->dataSize->CurSysNum = 0;
    state->dataSize->TermUnitSizing.allocate(1);
    state->dataSize->TermUnitFinalZoneSizing.allocate(1);
    state->dataSize->TermUnitSizing(state->dataSize->CurTermUnitSizingNum).AirVolFlow = waterCoil1.DesAirVolFlowRate / 3.0; // DesAirVolFlowRate = 1.0
    state->dataSize->TermUnitSizing(state->dataSize->CurTermUnitSizingNum).MaxHWVolFlow = waterCoil1.MaxWaterVolFlowRate / 3.0;
    state->dataSize->TermUnitSizing(state->dataSize->CurTermUnitSizingNum).MinPriFlowFrac = 0.5;
    state->dataSize->TermUnitSingDuct = true;

    waterCoil1.DesAirVolFlowRate = AutoSize;
    waterCoil1.UACoil = AutoSize;
    waterCoil1.MaxWaterVolFlowRate = AutoSize;
    waterCoil1.CoilPerfInpMeth = state->dataWaterCoils->UAandFlow;
    waterCoil1.DesInletAirTemp = AutoSize;
    waterCoil1.DesOutletAirTemp = AutoSize;
    waterCoil1.DesInletWaterTemp = AutoSize;
    waterCoil1.DesInletAirHumRat = AutoSize;
    waterCoil1.DesOutletAirHumRat = AutoSize;

    state->dataSize->SysSizingRunDone = false;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->NumZoneSizingInput = 1;
    state->dataSize->ZoneSizingInput.allocate(1);
    state->dataSize->ZoneSizingInput(1).ZoneNum = 1;
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod.allocate(20);
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod(HVAC::HeatingAirflowSizing) = HVAC::HeatingAirflowSizing;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).CoolingAirVolFlow = 0.0;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).HeatingAirVolFlow = 1.0;
    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow = 0.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow = 1.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInTempTU = 10.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneTempAtHeatPeak = 21.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInHumRatTU = 0.006;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneHumRatAtHeatPeak = 0.008;
    state->dataSize->TermUnitFinalZoneSizing(state->dataSize->CurTermUnitSizingNum)
        .copyFromZoneSizing(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum));

    state->dataWaterCoils->MySizeFlag(1) = true;
    // run water coil sizing
    SizeWaterCoil(*state, CoilNum);

    // check coil UA-value sizing
    EXPECT_NEAR(577.686, waterCoil1.UACoil, 0.01); // smaller UA than result above at 1435.00
}

TEST_F(WaterCoilsTest, CoilHeatingWaterLowAirFlowUASizing)
{
    Fluid::GetFluidPropertiesData(*state);
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->OutBaroPress, 20.0, 0.0);

    // set up sizing flags
    state->dataSize->SysSizingRunDone = true;

    // set up plant sizing
    state->dataSize->NumPltSizInput = 1;
    state->dataSize->PlantSizData(1).PlantLoopName = "HotWaterLoop";
    state->dataSize->PlantSizData(1).ExitTemp = 60.0;
    state->dataSize->PlantSizData(1).DeltaT = 10.0;

    // set up plant loop
    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {

        auto &loopside(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }
    state->dataPlnt->PlantLoop(1).Name = "HotWaterLoop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);

    // set up sizing data
    state->dataSize->FinalSysSizing(1).DesMainVolFlow = 1.00;
    state->dataSize->FinalSysSizing(1).HeatSupTemp = 40.0;
    state->dataSize->FinalSysSizing(1).HeatOutTemp = 5.0;
    state->dataSize->FinalSysSizing(1).HeatRetTemp = 20.0;
    state->dataSize->FinalSysSizing(1).HeatOAOption = DataSizing::OAControl::AllOA;

    // set up water coil
    int CoilNum = 1;
    auto &waterCoil1 = state->dataWaterCoils->WaterCoil(CoilNum);
    waterCoil1.Name = "Water Heating Coil";
    waterCoil1.WaterPlantLoc.loopNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, waterCoil1.WaterPlantLoc);
    waterCoil1.WaterCoilType = DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
    waterCoil1.WaterCoilModel = WaterCoils::CoilModel::HeatingSimple;
    waterCoil1.availSched = Sched::GetScheduleAlwaysOn(*state);

    waterCoil1.RequestingAutoSize = true;
    waterCoil1.DesAirVolFlowRate = AutoSize;
    waterCoil1.UACoil = AutoSize;
    waterCoil1.MaxWaterVolFlowRate = AutoSize;
    waterCoil1.CoilPerfInpMeth = state->dataWaterCoils->UAandFlow;
    waterCoil1.DesInletAirTemp = AutoSize;
    waterCoil1.DesOutletAirTemp = AutoSize;
    waterCoil1.DesInletWaterTemp = AutoSize;
    waterCoil1.DesInletAirHumRat = AutoSize;
    waterCoil1.DesOutletAirHumRat = AutoSize;

    state->dataWaterCoils->WaterCoilNumericFields(CoilNum).FieldNames(2) = "Maximum Water Flow Rate";
    waterCoil1.WaterInletNodeNum = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = waterCoil1.WaterInletNodeNum;

    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurSysNum = 1;
    state->dataSize->CurOASysNum = 0;
    state->dataSize->DataWaterLoopNum = 1;

    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(1);
    state->dataWaterCoils->MyUAAndFlowCalcFlag(1) = true;

    state->dataWaterCoils->MySizeFlag.allocate(1);
    state->dataWaterCoils->MySizeFlag(1) = true;

    // run water coil sizing
    SizeWaterCoil(*state, CoilNum);
    EXPECT_DOUBLE_EQ(1.0, waterCoil1.DesAirVolFlowRate);

    Real64 CpAirStd = 0.0;
    Real64 DesMassFlow = 0.0;
    Real64 DesCoilHeatingLoad = 0.0;

    CpAirStd = PsyCpAirFnW(0.0);
    DesMassFlow = waterCoil1.DesAirVolFlowRate * state->dataEnvrn->StdRhoAir;
    DesCoilHeatingLoad = CpAirStd * DesMassFlow * (40.0 - 5.0);

    // check heating coil design load
    EXPECT_DOUBLE_EQ(DesCoilHeatingLoad, waterCoil1.DesWaterHeatingCoilRate);

    Real64 Cp = 0;
    Real64 rho = 0;
    Real64 DesWaterFlowRate = 0;

    Cp = state->dataPlnt->PlantLoop(1).glycol->getSpecificHeat(*state, Constant::HWInitConvTemp, "Unit Test");
    rho = state->dataPlnt->PlantLoop(1).glycol->getDensity(*state, Constant::HWInitConvTemp, "Unit Test");
    DesWaterFlowRate = waterCoil1.DesWaterHeatingCoilRate / (10.0 * Cp * rho);

    // check heating coil design water flow rate
    EXPECT_DOUBLE_EQ(DesWaterFlowRate, waterCoil1.MaxWaterVolFlowRate);

    // check coil UA-value sizing
    EXPECT_NEAR(1435.01, waterCoil1.UACoil, 0.01);

    // test single zone VAV reheat coil sizing
    state->dataSize->CurZoneEqNum = 1;
    state->dataSize->CurSysNum = 0;
    state->dataSize->CurTermUnitSizingNum = 1;
    state->dataSize->TermUnitSizing.allocate(1);
    state->dataSize->TermUnitFinalZoneSizing.allocate(1);
    state->dataSize->TermUnitSizing(state->dataSize->CurTermUnitSizingNum).AirVolFlow =
        waterCoil1.DesAirVolFlowRate / 1500.0; // DesAirVolFlowRate = 1.0 so TU air flow = 0.00067 (lower than 0.001)
    state->dataSize->TermUnitSizing(state->dataSize->CurTermUnitSizingNum).MaxHWVolFlow = waterCoil1.MaxWaterVolFlowRate / 1500.0;
    state->dataSize->TermUnitSizing(state->dataSize->CurTermUnitSizingNum).MinPriFlowFrac = 0.5;
    state->dataSize->TermUnitSingDuct = true;

    waterCoil1.DesAirVolFlowRate = AutoSize;
    waterCoil1.UACoil = AutoSize;
    waterCoil1.MaxWaterVolFlowRate = AutoSize;
    waterCoil1.CoilPerfInpMeth = state->dataWaterCoils->UAandFlow;
    waterCoil1.DesInletAirTemp = AutoSize;
    waterCoil1.DesOutletAirTemp = AutoSize;
    waterCoil1.DesInletWaterTemp = AutoSize;
    waterCoil1.DesInletAirHumRat = AutoSize;
    waterCoil1.DesOutletAirHumRat = AutoSize;

    state->dataSize->SysSizingRunDone = false;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->NumZoneSizingInput = 1;
    state->dataSize->ZoneSizingInput.allocate(1);
    state->dataSize->ZoneSizingInput(1).ZoneNum = 1;
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod.allocate(20);
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod(HVAC::HeatingAirflowSizing) = HVAC::HeatingAirflowSizing;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).CoolingAirVolFlow = 0.0;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).HeatingAirVolFlow = 1.0;
    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow = 0.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow = 0.00095;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInTempTU = 10.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneTempAtHeatPeak = 21.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInHumRatTU = 0.006;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneHumRatAtHeatPeak = 0.008;
    state->dataSize->TermUnitFinalZoneSizing(state->dataSize->CurTermUnitSizingNum)
        .copyFromZoneSizing(state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum));

    state->dataWaterCoils->MySizeFlag(1) = true;
    // run water coil sizing
    SizeWaterCoil(*state, CoilNum);

    // water flow rate should be non-zero, and air flow rate being so small will get set to 0 during sizing
    EXPECT_GT(waterCoil1.InletWaterMassFlowRate, 0.0);
    EXPECT_EQ(0.0, waterCoil1.InletAirMassFlowRate);
    // check coil UA-value sizing
    EXPECT_NEAR(1.0, waterCoil1.UACoil, 0.0001); // TU air flow is too low to size, set to 0, so UA is set to 1.0
}

TEST_F(WaterCoilsTest, CoilHeatingWaterUASizingLowHwaterInletTemp)
{
    Fluid::GetFluidPropertiesData(*state);
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->OutBaroPress, 20.0, 0.0);

    // set up sizing flags
    state->dataSize->SysSizingRunDone = true;

    // set up plant sizing
    state->dataSize->NumPltSizInput = 1;
    state->dataSize->PlantSizData(1).PlantLoopName = "HotWaterLoop";
    state->dataSize->PlantSizData(1).ExitTemp = 40.0; // low heating coil inlet water temp
    state->dataSize->PlantSizData(1).DeltaT = 10.0;

    // set up plant loop
    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {

        auto &loopside(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }
    state->dataPlnt->PlantLoop(1).Name = "HotWaterLoop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);

    // set up sizing data
    state->dataSize->FinalSysSizing(1).DesMainVolFlow = 1.00;
    state->dataSize->FinalSysSizing(1).HeatSupTemp = 40.0;
    state->dataSize->FinalSysSizing(1).HeatOutTemp = 5.0;
    state->dataSize->FinalSysSizing(1).HeatRetTemp = 20.0;
    state->dataSize->FinalSysSizing(1).HeatOAOption = DataSizing::OAControl::AllOA;

    // set up water coil
    int CoilNum = 1;
    auto &waterCoil1 = state->dataWaterCoils->WaterCoil(CoilNum);
    waterCoil1.Name = "Water Heating Coil";
    waterCoil1.WaterPlantLoc.loopNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, waterCoil1.WaterPlantLoc);
    waterCoil1.WaterCoilType = DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
    waterCoil1.WaterCoilModel = WaterCoils::CoilModel::HeatingSimple;
    waterCoil1.availSched = Sched::GetScheduleAlwaysOn(*state);

    waterCoil1.RequestingAutoSize = true;
    waterCoil1.DesAirVolFlowRate = AutoSize;
    waterCoil1.UACoil = AutoSize;
    waterCoil1.MaxWaterVolFlowRate = AutoSize;
    waterCoil1.CoilPerfInpMeth = state->dataWaterCoils->UAandFlow;
    waterCoil1.DesInletAirTemp = AutoSize;
    waterCoil1.DesOutletAirTemp = AutoSize;
    waterCoil1.DesInletWaterTemp = AutoSize;
    waterCoil1.DesInletAirHumRat = AutoSize;
    waterCoil1.DesOutletAirHumRat = AutoSize;

    state->dataWaterCoils->WaterCoilNumericFields(CoilNum).FieldNames(2) = "Maximum Water Flow Rate";
    waterCoil1.WaterInletNodeNum = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = waterCoil1.WaterInletNodeNum;

    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurSysNum = 1;
    state->dataSize->CurOASysNum = 0;

    state->dataSize->DataWaterLoopNum = 1;

    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(1);
    state->dataWaterCoils->MyUAAndFlowCalcFlag(1) = true;

    state->dataWaterCoils->MySizeFlag.allocate(1);
    state->dataWaterCoils->MySizeFlag(1) = true;

    // run water coil sizing
    SizeWaterCoil(*state, CoilNum);
    EXPECT_DOUBLE_EQ(1.0, waterCoil1.DesAirVolFlowRate);

    Real64 CpAirStd = 0.0;
    Real64 DesMassFlow = 0.0;
    Real64 DesCoilHeatingLoad = 0.0;

    CpAirStd = PsyCpAirFnW(0.0);
    DesMassFlow = waterCoil1.DesAirVolFlowRate * state->dataEnvrn->StdRhoAir;
    DesCoilHeatingLoad = CpAirStd * DesMassFlow * (40.0 - 5.0);

    // check heating coil design load
    EXPECT_DOUBLE_EQ(DesCoilHeatingLoad, waterCoil1.DesWaterHeatingCoilRate);

    Real64 Cp = 0;
    Real64 rho = 0;
    Real64 DesWaterFlowRate = 0;

    Cp = state->dataPlnt->PlantLoop(1).glycol->getSpecificHeat(*state, Constant::HWInitConvTemp, "Unit Test");
    rho = state->dataPlnt->PlantLoop(1).glycol->getDensity(*state, Constant::HWInitConvTemp, "Unit Test");
    DesWaterFlowRate = waterCoil1.DesWaterHeatingCoilRate / (10.0 * Cp * rho);

    // check heating coil design water flow rate
    EXPECT_DOUBLE_EQ(DesWaterFlowRate, waterCoil1.MaxWaterVolFlowRate);

    // check coil UA-value sizing for low design loop exit temp
    EXPECT_NEAR(2479.27, waterCoil1.UACoil, 0.01);

    Real64 DesCoilInletWaterTempUsed = 0.0;
    HVAC::FanOp fanOp = HVAC::FanOp::Continuous;
    Real64 UAMax = waterCoil1.DesWaterHeatingCoilRate;

    // check if coil design inlet water temperature is increased above the plant loop exit temp
    EstimateCoilInletWaterTemp(*state, CoilNum, fanOp, 1.0, UAMax, DesCoilInletWaterTempUsed);
    EXPECT_GT(DesCoilInletWaterTempUsed, state->dataSize->PlantSizData(1).ExitTemp);
    EXPECT_NEAR(48.73, DesCoilInletWaterTempUsed, 0.01);
}

TEST_F(WaterCoilsTest, CoilCoolingWaterSimpleSizing)
{
    state->init_state(*state);

    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->OutBaroPress, 20.0, 0.0);
    ShowMessage(*state, "Begin Test: state->dataWaterCoils->WaterCoilsTest, CoilCoolingWaterSimpleSizing");

    // set up sizing flags
    state->dataSize->SysSizingRunDone = true;

    // set up plant sizing
    state->dataSize->NumPltSizInput = 1;
    state->dataSize->PlantSizData(1).PlantLoopName = "WaterLoop";

    // set up plant loop
    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {

        auto &loopside(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }
    state->dataPlnt->PlantLoop(1).Name = "WaterLoop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);

    // set up sizing data
    state->dataSize->FinalSysSizing(1).MixTempAtCoolPeak = 20.0;
    state->dataSize->FinalSysSizing(1).MixHumRatAtCoolPeak = 0.01;
    state->dataSize->FinalSysSizing(1).CoolSupTemp = 10.0;
    state->dataSize->FinalSysSizing(1).CoolSupHumRat = 0.0085;
    state->dataSize->FinalSysSizing(1).DesMainVolFlow = 1.00;
    state->dataSize->FinalSysSizing(1).MassFlowAtCoolPeak = state->dataSize->FinalSysSizing(1).DesMainVolFlow * state->dataEnvrn->StdRhoAir;

    // set up water coil
    int CoilNum = 1;
    auto &waterCoil1 = state->dataWaterCoils->WaterCoil(CoilNum);
    waterCoil1.Name = "Test Simple Water Cooling Coil";
    waterCoil1.WaterPlantLoc.loopNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, waterCoil1.WaterPlantLoc);
    waterCoil1.WaterCoilType = DataPlant::PlantEquipmentType::CoilWaterCooling;
    waterCoil1.WaterCoilModel = WaterCoils::CoilModel::CoolingSimple;

    waterCoil1.RequestingAutoSize = true;
    waterCoil1.DesAirVolFlowRate = AutoSize;
    waterCoil1.DesInletAirTemp = AutoSize;
    waterCoil1.DesOutletAirTemp = AutoSize;
    waterCoil1.DesInletWaterTemp = AutoSize;
    waterCoil1.DesInletAirHumRat = AutoSize;
    waterCoil1.DesOutletAirHumRat = AutoSize;
    waterCoil1.MaxWaterVolFlowRate = AutoSize;

    waterCoil1.DesignWaterDeltaTemp = 6.67;
    waterCoil1.UseDesignWaterDeltaTemp = true;

    state->dataWaterCoils->WaterCoilNumericFields(CoilNum).FieldNames(1) = "Design Water Flow Rate";
    waterCoil1.WaterInletNodeNum = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = waterCoil1.WaterInletNodeNum;

    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurSysNum = 1;
    state->dataSize->CurOASysNum = 0;
    state->dataSize->SysSizInput(1).CoolCapControl = DataSizing::CapacityControl::VAV;
    state->dataSize->PlantSizData(1).ExitTemp = 5.7;
    state->dataSize->PlantSizData(1).DeltaT = 5.0;

    state->dataSize->DataWaterLoopNum = 1;

    // run water coil sizing
    SizeWaterCoil(*state, CoilNum);
    EXPECT_DOUBLE_EQ(1.0, waterCoil1.DesAirVolFlowRate);

    Real64 DesCoilCoolingLoad = 0.0;
    Real64 CoilInEnth = 0.0;
    Real64 CoilOutEnth = 0.0;

    CoilInEnth = PsyHFnTdbW(20.0, 0.01);
    CoilOutEnth = PsyHFnTdbW(10.0, 0.0085);
    DesCoilCoolingLoad = waterCoil1.DesAirVolFlowRate * state->dataEnvrn->StdRhoAir * (CoilInEnth - CoilOutEnth);

    // check cooling coil design load
    EXPECT_DOUBLE_EQ(DesCoilCoolingLoad, waterCoil1.DesWaterCoolingCoilRate);

    Real64 Cp = 0;
    Real64 rho = 0;
    Real64 DesWaterFlowRate = 0;

    Cp = state->dataPlnt->PlantLoop(1).glycol->getSpecificHeat(*state, Constant::CWInitConvTemp, "Unit Test");
    rho = state->dataPlnt->PlantLoop(1).glycol->getDensity(*state, Constant::CWInitConvTemp, "Unit Test");
    DesWaterFlowRate = waterCoil1.DesWaterCoolingCoilRate / (waterCoil1.DesignWaterDeltaTemp * Cp * rho);

    // check cooling coil design water flow rate
    EXPECT_DOUBLE_EQ(DesWaterFlowRate, waterCoil1.MaxWaterVolFlowRate);
}

TEST_F(WaterCoilsTest, CoilCoolingWaterDetailedSizing)
{
    state->init_state(*state);

    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->OutBaroPress, 20.0, 0.0);
    ShowMessage(*state, "Begin Test: state->dataWaterCoils->WaterCoilsTest, CoilCoolingWaterDetailedSizing");

    // set up sizing flags
    state->dataSize->SysSizingRunDone = true;

    // set up plant sizing
    state->dataSize->NumPltSizInput = 1;
    state->dataSize->PlantSizData(1).PlantLoopName = "WaterLoop";

    // set up plant loop
    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {

        auto &loopside(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }
    state->dataPlnt->PlantLoop(1).Name = "WaterLoop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);

    // set up sizing data
    state->dataSize->FinalSysSizing(1).MixTempAtCoolPeak = 20.0;
    state->dataSize->FinalSysSizing(1).MixHumRatAtCoolPeak = 0.01;
    state->dataSize->FinalSysSizing(1).CoolSupTemp = 10.0;
    state->dataSize->FinalSysSizing(1).CoolSupHumRat = 0.0085;
    state->dataSize->FinalSysSizing(1).DesMainVolFlow = 1.00;
    state->dataSize->FinalSysSizing(1).MassFlowAtCoolPeak = state->dataSize->FinalSysSizing(1).DesMainVolFlow * state->dataEnvrn->StdRhoAir;

    // set up water coil
    int CoilNum = 1;
    auto &waterCoil1 = state->dataWaterCoils->WaterCoil(CoilNum);
    waterCoil1.Name = "Test Detailed Water Cooling Coil";
    waterCoil1.WaterPlantLoc.loopNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, waterCoil1.WaterPlantLoc);
    waterCoil1.WaterCoilType = DataPlant::PlantEquipmentType::CoilWaterCooling;
    waterCoil1.WaterCoilModel = WaterCoils::CoilModel::CoolingDetailed;

    waterCoil1.TubeOutsideSurfArea = 6.23816;
    waterCoil1.TotTubeInsideArea = 6.20007018;
    waterCoil1.FinSurfArea = 101.7158224;
    waterCoil1.MinAirFlowArea = 0.810606367;
    waterCoil1.CoilDepth = 0.165097968;
    waterCoil1.FinDiam = 0.43507152;
    waterCoil1.FinThickness = 0.001499982;
    waterCoil1.TubeInsideDiam = 0.014449958;
    waterCoil1.TubeOutsideDiam = 0.015879775;
    waterCoil1.TubeThermConductivity = 385.764854;
    waterCoil1.FinThermConductivity = 203.882537;
    waterCoil1.FinSpacing = 0.001814292;
    waterCoil1.TubeDepthSpacing = 0.02589977;
    waterCoil1.NumOfTubeRows = 6;
    waterCoil1.NumOfTubesPerRow = 16;
    waterCoil1.DesignWaterDeltaTemp = 6.67;
    waterCoil1.UseDesignWaterDeltaTemp = true;

    waterCoil1.RequestingAutoSize = true;
    waterCoil1.DesAirVolFlowRate = AutoSize;
    waterCoil1.MaxWaterVolFlowRate = AutoSize;
    waterCoil1.DesInletAirTemp = AutoSize;
    waterCoil1.DesOutletAirTemp = AutoSize;
    waterCoil1.DesInletWaterTemp = AutoSize;
    waterCoil1.DesInletAirHumRat = AutoSize;
    waterCoil1.DesOutletAirHumRat = AutoSize;
    waterCoil1.MaxWaterVolFlowRate = AutoSize;

    state->dataWaterCoils->WaterCoilNumericFields(CoilNum).FieldNames(1) = "Design Water Flow Rate";
    waterCoil1.WaterInletNodeNum = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = waterCoil1.WaterInletNodeNum;

    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurSysNum = 1;
    state->dataSize->CurOASysNum = 0;
    state->dataSize->SysSizInput(1).CoolCapControl = DataSizing::CapacityControl::VAV;
    state->dataSize->PlantSizData(1).ExitTemp = 5.7;
    state->dataSize->PlantSizData(1).DeltaT = 5.0;
    state->dataSize->DataWaterLoopNum = 1;

    // run water coil sizing
    SizeWaterCoil(*state, CoilNum);
    EXPECT_DOUBLE_EQ(1.0, waterCoil1.DesAirVolFlowRate);

    Real64 DesCoilCoolingLoad = 0.0;
    Real64 CoilInEnth = 0.0;
    Real64 CoilOutEnth = 0.0;

    CoilInEnth = PsyHFnTdbW(state->dataSize->FinalSysSizing(1).MixTempAtCoolPeak, state->dataSize->FinalSysSizing(1).MixHumRatAtCoolPeak);
    CoilOutEnth = PsyHFnTdbW(state->dataSize->FinalSysSizing(1).CoolSupTemp, state->dataSize->FinalSysSizing(1).CoolSupHumRat);
    DesCoilCoolingLoad = waterCoil1.DesAirVolFlowRate * state->dataEnvrn->StdRhoAir * (CoilInEnth - CoilOutEnth);
    // check cooling coil design load
    EXPECT_DOUBLE_EQ(DesCoilCoolingLoad, waterCoil1.DesWaterCoolingCoilRate);

    Real64 Cp = 0;
    Real64 rho = 0;
    Real64 DesWaterFlowRate = 0;

    Cp = state->dataPlnt->PlantLoop(1).glycol->getSpecificHeat(*state, Constant::CWInitConvTemp, "Unit Test");
    rho = state->dataPlnt->PlantLoop(1).glycol->getDensity(*state, Constant::CWInitConvTemp, "Unit Test");
    DesWaterFlowRate = waterCoil1.DesWaterCoolingCoilRate / (6.67 * Cp * rho);
    // check cooling coil design water flow rate
    EXPECT_DOUBLE_EQ(DesWaterFlowRate, waterCoil1.MaxWaterVolFlowRate);
}

TEST_F(WaterCoilsTest, CoilCoolingWaterDetailed_WarningMath)
{
    state->init_state(*state);

    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->OutBaroPress, 20.0, 0.0);

    // set up sizing flags
    state->dataSize->SysSizingRunDone = true;

    // set up plant sizing
    state->dataSize->NumPltSizInput = 1;
    state->dataSize->PlantSizData(1).PlantLoopName = "WaterLoop";

    // set up plant loop
    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
        auto &loopside(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }
    state->dataPlnt->PlantLoop(1).Name = "WaterLoop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);

    // set up sizing data
    state->dataSize->FinalSysSizing(1).MixTempAtCoolPeak = 20.0;
    state->dataSize->FinalSysSizing(1).MixHumRatAtCoolPeak = 0.01;
    state->dataSize->FinalSysSizing(1).CoolSupTemp = 10.0;
    state->dataSize->FinalSysSizing(1).CoolSupHumRat = 0.0085;
    state->dataSize->FinalSysSizing(1).DesMainVolFlow = 1.00;
    state->dataSize->FinalSysSizing(1).MassFlowAtCoolPeak = state->dataSize->FinalSysSizing(1).DesMainVolFlow * state->dataEnvrn->StdRhoAir;

    // set up water coil
    int CoilNum = 1;
    auto &waterCoil1 = state->dataWaterCoils->WaterCoil(CoilNum);
    waterCoil1.Name = "Test Detailed Water Cooling Coil";
    waterCoil1.coilType = HVAC::CoilType::CoolingWaterDetailed;
    waterCoil1.coilReportNum = ReportCoilSelection::getReportIndex(*state, waterCoil1.Name, waterCoil1.coilType);

    waterCoil1.availSched = Sched::GetScheduleAlwaysOff(*state);

    waterCoil1.WaterCoilType = DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling;
    waterCoil1.WaterCoilModel = WaterCoils::CoilModel::CoolingDetailed;

    waterCoil1.TubeOutsideSurfArea = 6.23816;
    waterCoil1.TotTubeInsideArea = 6.20007018;
    waterCoil1.FinSurfArea = 101.7158224;
    waterCoil1.MinAirFlowArea = 0.810606367;
    waterCoil1.CoilDepth = 0.165097968;
    waterCoil1.FinDiam = 0.43507152;
    waterCoil1.FinThickness = 0.001499982;
    waterCoil1.TubeInsideDiam = 0.014449958;
    waterCoil1.TubeOutsideDiam = 0.015879775;
    waterCoil1.TubeThermConductivity = 385.764854;
    waterCoil1.FinThermConductivity = 203.882537;
    waterCoil1.FinSpacing = 0.001814292;
    waterCoil1.TubeDepthSpacing = 0.02589977;
    waterCoil1.NumOfTubeRows = 6;
    waterCoil1.NumOfTubesPerRow = 16;
    waterCoil1.DesignWaterDeltaTemp = 6.67;
    waterCoil1.UseDesignWaterDeltaTemp = true;

    waterCoil1.RequestingAutoSize = true;
    waterCoil1.DesAirVolFlowRate = AutoSize;
    waterCoil1.MaxWaterVolFlowRate = AutoSize;
    waterCoil1.DesInletAirTemp = AutoSize;
    waterCoil1.DesOutletAirTemp = AutoSize;
    waterCoil1.DesInletWaterTemp = AutoSize;
    waterCoil1.DesInletAirHumRat = AutoSize;
    waterCoil1.DesOutletAirHumRat = AutoSize;
    waterCoil1.MaxWaterVolFlowRate = AutoSize;

    state->dataWaterCoils->WaterCoilNumericFields(CoilNum).FieldNames(1) = "Design Water Flow Rate";
    waterCoil1.WaterInletNodeNum = 1;
    waterCoil1.WaterOutletNodeNum = 2;
    waterCoil1.AirInletNodeNum = 3;
    waterCoil1.AirOutletNodeNum = 4;
    waterCoil1.WaterPlantLoc.loopNum = 1;
    waterCoil1.WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    waterCoil1.WaterPlantLoc.branchNum = 1;
    waterCoil1.WaterPlantLoc.compNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, waterCoil1.WaterPlantLoc);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = waterCoil1.Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = waterCoil1.WaterInletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = waterCoil1.WaterOutletNodeNum;

    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurSysNum = 1;
    state->dataSize->CurOASysNum = 0;
    state->dataSize->SysSizInput(1).CoolCapControl = DataSizing::CapacityControl::VAV;
    state->dataSize->PlantSizData(1).ExitTemp = 5.7;
    state->dataSize->PlantSizData(1).DeltaT = 5.0;
    state->dataSize->DataWaterLoopNum = 1;

    // run water coil sizing
    SizeWaterCoil(*state, CoilNum);
    EXPECT_DOUBLE_EQ(1.0, waterCoil1.DesAirVolFlowRate);

    Real64 DesCoilCoolingLoad = 0.0;
    Real64 CoilInEnth = 0.0;
    Real64 CoilOutEnth = 0.0;

    CoilInEnth = PsyHFnTdbW(state->dataSize->FinalSysSizing(1).MixTempAtCoolPeak, state->dataSize->FinalSysSizing(1).MixHumRatAtCoolPeak);
    CoilOutEnth = PsyHFnTdbW(state->dataSize->FinalSysSizing(1).CoolSupTemp, state->dataSize->FinalSysSizing(1).CoolSupHumRat);
    DesCoilCoolingLoad = waterCoil1.DesAirVolFlowRate * state->dataEnvrn->StdRhoAir * (CoilInEnth - CoilOutEnth);
    // check cooling coil design load
    EXPECT_DOUBLE_EQ(DesCoilCoolingLoad, waterCoil1.DesWaterCoolingCoilRate);

    Real64 Cp = 0;
    Real64 rho = 0;
    Real64 DesWaterFlowRate = 0;

    Cp = state->dataPlnt->PlantLoop(1).glycol->getSpecificHeat(*state, Constant::CWInitConvTemp, "Unit Test");
    rho = state->dataPlnt->PlantLoop(1).glycol->getDensity(*state, Constant::CWInitConvTemp, "Unit Test");
    DesWaterFlowRate = waterCoil1.DesWaterCoolingCoilRate / (6.67 * Cp * rho);
    // check cooling coil design water flow rate
    EXPECT_DOUBLE_EQ(DesWaterFlowRate, waterCoil1.MaxWaterVolFlowRate);

    state->dataLoopNodes->Node.allocate(10);

    InitWaterCoil(*state, 1, false);

    // Force getting the warning
    Real64 PartLoadRatio = 1.0;
    Real64 TempAirIn = waterCoil1.InletAirTemp;
    Real64 InletAirHumRat = waterCoil1.InletAirHumRat;
    Real64 TempWaterIn = waterCoil1.InletWaterTemp;
    Real64 AirDensity = Psychrometrics::PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->OutBaroPress, TempAirIn, InletAirHumRat, "RoutineName");
    Real64 testMinAirMassFlow = 5.0 * waterCoil1.MinAirFlowArea * AirDensity;
    waterCoil1.InletAirMassFlowRate = 1.1 * testMinAirMassFlow;
    Real64 AirMassFlow = waterCoil1.InletAirMassFlowRate / PartLoadRatio;

    EXPECT_EQ(0.81060636699999999, waterCoil1.MinAirFlowArea);
    std::string expected_error = delimited_string({
        std::format("   ** Warning ** Version: missing in IDF, processing for EnergyPlus version=\"{}\"", DataStringGlobals::MatchVersion),
        "   ** Warning ** Coil:Cooling:Water:DetailedGeometry in Coil =Test Detailed Water Cooling Coil",
        "   **   ~~~   ** Air Flow Rate Velocity has greatly exceeded upper design guidelines of ~2.5 m/s",
        std::format("   **   ~~~   ** Air Mass Flow Rate[kg/s]={:.6f}", waterCoil1.InletAirMassFlowRate),
        std::format("   **   ~~~   ** Air Face Velocity[m/s]={:.6f}", AirMassFlow / (waterCoil1.MinAirFlowArea * AirDensity)),
        std::format("   **   ~~~   ** Approximate Mass Flow Rate limit for Face Area[kg/s]={:.6f}", 2.5 * waterCoil1.MinAirFlowArea * AirDensity),
        "   **   ~~~   ** Coil:Cooling:Water:DetailedGeometry could be resized/autosized to handle capacity",
    });

    CalcDetailFlatFinCoolingCoil(*state, CoilNum, 2, HVAC::FanOp::Continuous, 1);

    compare_err_stream(expected_error, true);

    AirMassFlow = waterCoil1.InletAirMassFlowRate / PartLoadRatio;
    TempAirIn = waterCoil1.InletAirTemp;
    InletAirHumRat = waterCoil1.InletAirHumRat;
    TempWaterIn = waterCoil1.InletWaterTemp;
    AirDensity = Psychrometrics::PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->OutBaroPress, TempAirIn, InletAirHumRat, "RoutineName");
    testMinAirMassFlow = 44.7 * waterCoil1.MinAirFlowArea * AirDensity;
    waterCoil1.InletAirMassFlowRate = 1.1 * testMinAirMassFlow;
    AirMassFlow = waterCoil1.InletAirMassFlowRate / PartLoadRatio;

    std::string expected_fatal_error = delimited_string({
        "   ** Severe  ** Coil:Cooling:Water:DetailedGeometry in Coil =Test Detailed Water Cooling Coil",
        "   **   ~~~   ** Air Flow Rate Velocity is > 100MPH (44.7m/s) and simulation cannot continue",
        std::format("   **   ~~~   ** Air Mass Flow Rate[kg/s]={:.6f}", waterCoil1.InletAirMassFlowRate),
        std::format("   **   ~~~   ** Air Face Velocity[m/s]={:.6f}", AirMassFlow / (waterCoil1.MinAirFlowArea * AirDensity)),
        std::format("   **   ~~~   ** Approximate Mass Flow Rate limit for Face Area[kg/s]={:.6f}", 44.7 * waterCoil1.MinAirFlowArea * AirDensity),
        "   **  Fatal  ** Coil:Cooling:Water:DetailedGeometry needs to be resized/autosized to handle capacity",
        "   ...Summary of Errors that led to program termination:",
        "   ..... Reference severe error count=1",
        "   ..... Last severe error=Coil:Cooling:Water:DetailedGeometry in Coil =Test Detailed Water Cooling Coil",
    });

    EXPECT_ANY_THROW(CalcDetailFlatFinCoolingCoil(*state, CoilNum, 2, HVAC::FanOp::Continuous, 1));

    compare_err_stream(expected_fatal_error, true);
}

TEST_F(WaterCoilsTest, CoilHeatingWaterSimpleSizing)
{
    state->init_state(*state);

    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->OutBaroPress, 20.0, 0.0);
    ShowMessage(*state, "Begin Test: state->dataWaterCoils->WaterCoilsTest, CoilHeatingWaterSimpleSizing");

    // set up sizing flags
    state->dataSize->SysSizingRunDone = true;

    // set up plant sizing
    state->dataSize->NumPltSizInput = 1;
    state->dataSize->PlantSizData(1).PlantLoopName = "WaterLoop";

    // set up plant loop
    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
        auto &loopside(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }
    state->dataPlnt->PlantLoop(1).Name = "WaterLoop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);

    // set up sizing data
    state->dataSize->FinalSysSizing(1).DesMainVolFlow = 1.00;
    state->dataSize->FinalSysSizing(1).HeatSupTemp = 40.0;
    state->dataSize->FinalSysSizing(1).HeatOutTemp = 5.0;
    state->dataSize->FinalSysSizing(1).HeatRetTemp = 20.0;
    state->dataSize->FinalSysSizing(1).HeatOAOption = DataSizing::OAControl::AllOA;

    // set up water coil
    int CoilNum = 1;
    auto &waterCoil1 = state->dataWaterCoils->WaterCoil(CoilNum);
    waterCoil1.Name = "Test Simple Water Heating Coil";
    waterCoil1.WaterPlantLoc.loopNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, waterCoil1.WaterPlantLoc);

    waterCoil1.WaterCoilType = DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
    waterCoil1.WaterCoilModel = WaterCoils::CoilModel::HeatingSimple;
    waterCoil1.RequestingAutoSize = true;
    waterCoil1.DesAirVolFlowRate = AutoSize;
    waterCoil1.DesInletAirTemp = AutoSize;
    waterCoil1.DesOutletAirTemp = AutoSize;
    waterCoil1.DesInletWaterTemp = AutoSize;
    waterCoil1.DesInletAirHumRat = AutoSize;
    waterCoil1.DesOutletAirHumRat = AutoSize;
    waterCoil1.MaxWaterVolFlowRate = AutoSize;

    waterCoil1.DesignWaterDeltaTemp = 11.0;
    waterCoil1.UseDesignWaterDeltaTemp = true;

    state->dataWaterCoils->WaterCoilNumericFields(CoilNum).FieldNames(2) = "Maximum Water Flow Rate";
    waterCoil1.WaterInletNodeNum = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = waterCoil1.WaterInletNodeNum;

    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurSysNum = 1;
    state->dataSize->CurOASysNum = 0;

    state->dataSize->PlantSizData(1).ExitTemp = 60.0;
    state->dataSize->PlantSizData(1).DeltaT = 10.0;

    state->dataSize->DataWaterLoopNum = 1;

    // run water coil sizing
    SizeWaterCoil(*state, CoilNum);
    EXPECT_DOUBLE_EQ(1.0, waterCoil1.DesAirVolFlowRate);

    Real64 CpAirStd = 0.0;
    Real64 DesMassFlow = 0.0;
    Real64 DesCoilHeatingLoad = 0.0;

    CpAirStd = PsyCpAirFnW(0.0);
    DesMassFlow = state->dataSize->FinalSysSizing(1).DesMainVolFlow * state->dataEnvrn->StdRhoAir;
    DesCoilHeatingLoad = CpAirStd * DesMassFlow * (40.0 - 5.0);

    // check heating coil design load
    EXPECT_DOUBLE_EQ(DesCoilHeatingLoad, waterCoil1.DesWaterHeatingCoilRate);

    Real64 Cp = 0;
    Real64 rho = 0;
    Real64 DesWaterFlowRate = 0;

    Cp = state->dataPlnt->PlantLoop(1).glycol->getSpecificHeat(*state, Constant::HWInitConvTemp, "Unit Test");
    rho = state->dataPlnt->PlantLoop(1).glycol->getDensity(*state, Constant::HWInitConvTemp, "Unit Test");
    DesWaterFlowRate = waterCoil1.DesWaterHeatingCoilRate / (11.0 * Cp * rho);

    // check heating coil design water flow rate
    EXPECT_DOUBLE_EQ(DesWaterFlowRate, waterCoil1.MaxWaterVolFlowRate);
}
TEST_F(WaterCoilsTest, HotWaterHeatingCoilAutoSizeTempTest)
{
    Fluid::GetFluidPropertiesData(*state);
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->OutBaroPress, 20.0, 0.0);

    // set up sizing flags
    state->dataSize->SysSizingRunDone = true;

    // set up plant sizing
    state->dataSize->NumPltSizInput = 1;
    state->dataSize->PlantSizData(1).PlantLoopName = "HotWaterLoop";
    state->dataSize->PlantSizData(1).ExitTemp = 60.0; // hot water coil inlet water temp
    state->dataSize->PlantSizData(1).DeltaT = 10.0;

    // set up plant loop
    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
        auto &loopside(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }
    state->dataPlnt->PlantLoop(1).Name = "HotWaterLoop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);

    // set up sizing data
    state->dataSize->FinalSysSizing(1).DesMainVolFlow = 1.00;
    state->dataSize->FinalSysSizing(1).HeatSupTemp = 40.0;
    state->dataSize->FinalSysSizing(1).HeatOutTemp = 5.0;
    state->dataSize->FinalSysSizing(1).HeatRetTemp = 20.0;
    state->dataSize->FinalSysSizing(1).HeatOAOption = DataSizing::OAControl::AllOA;

    // set up water coil
    int CoilNum = 1;
    auto &waterCoil1 = state->dataWaterCoils->WaterCoil(CoilNum);
    waterCoil1.Name = "Water Heating Coil";
    waterCoil1.coilType = HVAC::CoilType::HeatingWater;
    waterCoil1.coilReportNum = ReportCoilSelection::getReportIndex(*state, waterCoil1.Name, waterCoil1.coilType);
    waterCoil1.WaterPlantLoc.loopNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, waterCoil1.WaterPlantLoc);
    waterCoil1.WaterCoilModel = WaterCoils::CoilModel::HeatingSimple;
    waterCoil1.WaterCoilType = DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
    waterCoil1.availSched = Sched::GetScheduleAlwaysOn(*state);

    waterCoil1.RequestingAutoSize = true;
    waterCoil1.DesAirVolFlowRate = AutoSize;
    waterCoil1.UACoil = AutoSize;
    waterCoil1.MaxWaterVolFlowRate = AutoSize;
    waterCoil1.CoilPerfInpMeth = state->dataWaterCoils->UAandFlow;
    waterCoil1.DesInletAirTemp = AutoSize;
    waterCoil1.DesOutletAirTemp = AutoSize;
    waterCoil1.DesInletWaterTemp = AutoSize;
    waterCoil1.DesInletAirHumRat = AutoSize;
    waterCoil1.DesOutletAirHumRat = AutoSize;

    state->dataWaterCoils->WaterCoilNumericFields(CoilNum).FieldNames(2) = "Maximum Water Flow Rate";
    waterCoil1.WaterInletNodeNum = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = waterCoil1.WaterInletNodeNum;

    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurSysNum = 1;
    state->dataSize->CurOASysNum = 0;

    state->dataSize->DataWaterLoopNum = 1;

    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(1);
    state->dataWaterCoils->MyUAAndFlowCalcFlag(1) = true;

    state->dataWaterCoils->MySizeFlag.allocate(1);
    state->dataWaterCoils->MySizeFlag(1) = true;

    // run water coil sizing
    SizeWaterCoil(*state, CoilNum);
    EXPECT_DOUBLE_EQ(1.0, waterCoil1.DesAirVolFlowRate);

    Real64 CpAirStd(0.0);
    Real64 DesMassFlow(0.0);
    Real64 DesCoilHeatingLoad(0.0);

    CpAirStd = PsyCpAirFnW(0.0);
    DesMassFlow = waterCoil1.DesAirVolFlowRate * state->dataEnvrn->StdRhoAir;
    DesCoilHeatingLoad = DesMassFlow * CpAirStd * (40.0 - 5.0);

    // check heating coil design load
    EXPECT_DOUBLE_EQ(DesCoilHeatingLoad, waterCoil1.DesWaterHeatingCoilRate);

    Real64 Cp(0.0);
    Real64 rho(0.0);
    Real64 DesWaterFlowRate(0.0);

    // now size heating coil hot water flow rate at 60.0C
    Cp = state->dataPlnt->PlantLoop(1).glycol->getSpecificHeat(*state, 60.0, "Unit Test");
    rho = state->dataPlnt->PlantLoop(1).glycol->getDensity(*state, 60.0, "Unit Test");
    DesWaterFlowRate = DesCoilHeatingLoad / (state->dataSize->PlantSizData(1).DeltaT * Cp * rho);

    // check heating coil design water flow rate calculated here and sizing results are identical
    EXPECT_DOUBLE_EQ(DesWaterFlowRate, waterCoil1.MaxWaterVolFlowRate);
}

TEST_F(WaterCoilsTest, FanCoilCoolingWaterFlowTest)
{
    state->dataSize->PlantSizData.allocate(2);
    state->dataSize->NumPltSizInput = 2;

    state->dataSize->PlantSizData(2).PlantLoopName = "ChilledWaterLoop";
    state->dataSize->PlantSizData(2).ExitTemp = 7.22; // chilled water coil inlet water temp
    state->dataSize->PlantSizData(2).DeltaT = 6.67;
    state->dataSize->PlantSizData(1).PlantLoopName = "HotWaterLoop";
    state->dataSize->PlantSizData(1).ExitTemp = 60; // hot water coil inlet water temp
    state->dataSize->PlantSizData(1).DeltaT = 12;
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
        "	Autosize, !- Maximum Flow Rate { m3 / s }",
        "	0.9, !- Motor Efficiency",
        "	1.0, !- Motor In Airstream Fraction",
        "	Zone1FanCoilOAMixerOutletNode, !- Air Inlet Node Name",
        "	Zone1FanCoilFanOutletNode, !- Air Outlet Node Name",
        "	, !- Fan Power Ratio Function of Speed Ratio Curve Name",
        "	;                        !- Fan Efficiency Ratio Function of Speed Ratio Curve Name	",
        "	Coil:Cooling:Water,",
        "	Zone1FanCoilCoolingCoil, !- Name",
        "	FanAndCoilAvailSched, !- Availability Schedule Namev",
        "	Autosize, !- Design Water Flow Rate { m3 / s }",
        "	Autosize, !- Design Air Flow Rate { m3 / s }",
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
        "	Autosize, !- Maximum Water Flow Rate { m3 / s }",
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
        "	0.0002, !- Maximum Cold Water Flow Rate { m3 / s }",
        "	0.0, !- Minimum Cold Water Flow Rate { m3 / s }",
        "	0.001, !- Cooling Convergence Tolerance",
        "	Coil:Heating:Water, !- Heating Coil Object Type",
        "	Zone1FanCoilHeatingCoil, !- Heating Coil Name",
        "	0.0002, !- Maximum Hot Water Flow Rate { m3 / s }",
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

    state->dataFans->fans(1)->inletAirMassFlowRate = AirMassFlow;
    state->dataFans->fans(1)->maxAirMassFlowRate = MaxAirMassFlow;

    state->dataLoopNodes->Node(state->dataFans->fans(1)->inletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->inletNodeNum).MassFlowRateMin = AirMassFlow;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->inletNodeNum).MassFlowRateMax = AirMassFlow;
    state->dataLoopNodes->Node(state->dataFans->fans(1)->inletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

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

    state->dataGlobal->DoingSizing = true;

    state->dataSize->CurZoneEqNum = 1;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->ZoneEqSizing.allocate(state->dataSize->CurZoneEqNum);
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod.allocate(25);
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod(25) = 0;
    state->dataSize->ZoneEqFanCoil = true;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).DesignSizeFromParent = true;

    // User-specified air volume flow rate from the ZoneHVAC:FourPipeFanCoil object
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).AirVolFlow = 0.5;
    // User-specified water flow rate from the ZoneHVAC:FourPipeFanCoil object
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).MaxCWVolFlow = 0.0002;
    // User-specified water flow rate from the ZoneHVAC:FourPipeFanCoil object
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).MaxHWVolFlow = 0.0002;

    // Initial design air volume flow rate based on the design conditions
    state->dataWaterCoils->WaterCoil(2).DesAirVolFlowRate = DataSizing::AutoSize;
    // Initial design water flow rate based on the design conditions
    state->dataWaterCoils->WaterCoil(2).MaxWaterVolFlowRate = DataSizing::AutoSize;

    // normal cooling simulation for constant fan variable flow fan coil
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, LatOutputProvided);

    // Expect final design air volume flow rate to equal the user-specified air volume flow rate from the ZoneHVAC:FourPipeFanCoil object
    EXPECT_EQ(state->dataWaterCoils->WaterCoil(2).DesAirVolFlowRate, 0.5);
    // Expect final design water flow rate to equal the user-specified water flow rate from the ZoneHVAC:FourPipeFanCoil object
    EXPECT_EQ(state->dataWaterCoils->WaterCoil(2).MaxWaterVolFlowRate, 0.0002);
}

namespace {
// Single zone with a cycling fan AirLoopHVAC:UnitarySystem and a Coil:Heating:Water served by a plant loop with a ConstantFlow
// boiler and no demand side bypass (the coil is the only demand side component)
std::string hotWaterLoopWithoutDemandBypassIdf(bool const continuousPump)
{
    std::vector<std::string_view> lines = {
        "  Timestep,6;",
        "  Building,Building,0,Suburbs,0.04,0.4,FullInteriorAndExterior,25,6;",
        "  GlobalGeometryRules,UpperLeftCorner,Counterclockwise,World;",
        "  SimulationControl,No,No,No,Yes,No;",
        "  Site:Location,Rochester,43.90,-92.50,-6.00,398.00;",
        "  SizingPeriod:DesignDay,",
        "    Rochester Htg 99.6% Condns DB,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    WinterDesignDay,         !- Day Type",
        "    -26.2,                   !- Maximum Dry-Bulb Temperature {C}",
        "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    DefaultMultipliers,      !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    -26.2,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    96634.,                  !- Barometric Pressure {Pa}",
        "    5.7,                     !- Wind Speed {m/s}",
        "    310,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    0.00;                    !- Sky Clearness",
        "  ScheduleTypeLimits,Any Number;",
        "  Schedule:Constant,always_avail,Any Number,1;",
        "  Schedule:Constant,fan_cycle,Any Number,0;",
        "  Schedule:Constant,zone_control_type,Any Number,4;",
        "  Schedule:Constant,heating_setpoint,Any Number,21;",
        "  Schedule:Constant,cooling_setpoint,Any Number,26;",
        "  Schedule:Constant,hw_loop_setpoint,Any Number,51.7;",
        "  Material:NoMass,Insulation,MediumRough,2.0;",
        "  Construction,Envelope,Insulation;",
        "  Zone,Zone1;",
        "  BuildingSurface:Detailed,Floor,Floor,Envelope,Zone1,,Adiabatic,,NoSun,NoWind,,4,",
        "    10,0,0, 0,0,0, 0,10,0, 10,10,0;",
        "  BuildingSurface:Detailed,Roof,Roof,Envelope,Zone1,,Outdoors,,SunExposed,WindExposed,,4,",
        "    0,10,3, 0,0,3, 10,0,3, 10,10,3;",
        "  BuildingSurface:Detailed,Wall South,Wall,Envelope,Zone1,,Outdoors,,SunExposed,WindExposed,,4,",
        "    0,0,3, 0,0,0, 10,0,0, 10,0,3;",
        "  BuildingSurface:Detailed,Wall East,Wall,Envelope,Zone1,,Outdoors,,SunExposed,WindExposed,,4,",
        "    10,0,3, 10,0,0, 10,10,0, 10,10,3;",
        "  BuildingSurface:Detailed,Wall North,Wall,Envelope,Zone1,,Outdoors,,SunExposed,WindExposed,,4,",
        "    10,10,3, 10,10,0, 0,10,0, 0,10,3;",
        "  BuildingSurface:Detailed,Wall West,Wall,Envelope,Zone1,,Outdoors,,SunExposed,WindExposed,,4,",
        "    0,10,3, 0,10,0, 0,0,0, 0,0,3;",
        "  ZoneControl:Thermostat,Zone1 Thermostat,Zone1,zone_control_type,,ThermostatSetpoint:DualSetpoint,Zone1 Setpoints;",
        "  ThermostatSetpoint:DualSetpoint,Zone1 Setpoints,heating_setpoint,cooling_setpoint;",
        "  ZoneHVAC:EquipmentConnections,Zone1,Zone1 Equipment,Zone1 Inlet Node,,Zone1 Air Node,Zone1 Outlet Node;",
        "  ZoneHVAC:EquipmentList,Zone1 Equipment,SequentialLoad,ZoneHVAC:AirDistributionUnit,Zone1 ADU,1,1,,;",
        "  ZoneHVAC:AirDistributionUnit,Zone1 ADU,Zone1 Inlet Node,AirTerminal:SingleDuct:ConstantVolume:NoReheat,Zone1 Terminal;",
        "  AirTerminal:SingleDuct:ConstantVolume:NoReheat,Zone1 Terminal,always_avail,Zone1 Terminal Inlet Node,Zone1 Inlet Node,0.326847;",
        "  AirLoopHVAC:SupplyPath,Supply Path,Zone Equipment Inlet Node,AirLoopHVAC:ZoneSplitter,Zone Splitter;",
        "  AirLoopHVAC:ZoneSplitter,Zone Splitter,Zone Equipment Inlet Node,Zone1 Terminal Inlet Node;",
        "  AirLoopHVAC:ReturnPath,Return Path,Return Air Mixer Outlet,AirLoopHVAC:ZoneMixer,Zone Mixer;",
        "  AirLoopHVAC:ZoneMixer,Zone Mixer,Return Air Mixer Outlet,Zone1 Outlet Node;",
        "  AirLoopHVAC,Air Loop,,,0.326847,Air Loop Branches,,Air Loop Inlet Node,Return Air Mixer Outlet,Zone Equipment Inlet Node,Air Loop Outlet "
        "Node;",
        "  BranchList,Air Loop Branches,Air Loop Main Branch;",
        "  Branch,Air Loop Main Branch,,AirLoopHVAC:UnitarySystem,Unitary,Air Loop Inlet Node,Air Loop Outlet Node;",
        "  AirLoopHVAC:UnitarySystem,",
        "    Unitary,                 !- Name",
        "    Load,                    !- Control Type",
        "    Zone1,                   !- Controlling Zone or Thermostat Location",
        "    None,                    !- Dehumidification Control Type",
        "    always_avail,            !- Availability Schedule Name",
        "    Air Loop Inlet Node,     !- Air Inlet Node Name",
        "    Air Loop Outlet Node,    !- Air Outlet Node Name",
        "    Fan:SystemModel,         !- Supply Fan Object Type",
        "    Supply Fan,              !- Supply Fan Name",
        "    BlowThrough,             !- Fan Placement",
        "    fan_cycle,               !- Supply Air Fan Operating Mode Schedule Name",
        "    Coil:Heating:Water,      !- Heating Coil Object Type",
        "    HW Coil,                 !- Heating Coil Name",
        "    ,                        !- DX Heating Coil Sizing Ratio",
        "    ,                        !- Cooling Coil Object Type",
        "    ,                        !- Cooling Coil Name",
        "    ,                        !- Use DOAS DX Cooling Coil",
        "    ,                        !- Minimum Supply Air Temperature {C}",
        "    ,                        !- Latent Load Control",
        "    ,                        !- Supplemental Heating Coil Object Type",
        "    ,                        !- Supplemental Heating Coil Name",
        "    None,                    !- Cooling Supply Air Flow Rate Method",
        "    ,                        !- Cooling Supply Air Flow Rate {m3/s}",
        "    ,                        !- Cooling Supply Air Flow Rate Per Floor Area {m3/s-m2}",
        "    ,                        !- Cooling Fraction of Autosized Cooling Supply Air Flow Rate",
        "    ,                        !- Cooling Supply Air Flow Rate Per Unit of Capacity {m3/s-W}",
        "    SupplyAirFlowRate,       !- Heating Supply Air Flow Rate Method",
        "    0.326847,                !- Heating Supply Air Flow Rate {m3/s}",
        "    ,                        !- Heating Supply Air Flow Rate Per Floor Area {m3/s-m2}",
        "    ,                        !- Heating Fraction of Autosized Heating Supply Air Flow Rate",
        "    ,                        !- Heating Supply Air Flow Rate Per Unit of Capacity {m3/s-W}",
        "    SupplyAirFlowRate,       !- No Load Supply Air Flow Rate Method",
        "    0,                       !- No Load Supply Air Flow Rate {m3/s}",
        "    ,                        !- No Load Supply Air Flow Rate Per Floor Area {m3/s-m2}",
        "    ,                        !- No Load Fraction of Autosized Cooling Supply Air Flow Rate",
        "    ,                        !- No Load Fraction of Autosized Heating Supply Air Flow Rate",
        "    ,                        !- No Load Supply Air Flow Rate Per Unit of Capacity During Cooling Operation {m3/s-W}",
        "    ,                        !- No Load Supply Air Flow Rate Per Unit of Capacity During Heating Operation {m3/s-W}",
        "    ,                        !- No Load Supply Air Flow Rate Control Set To Low Speed",
        "    50;                      !- Maximum Supply Air Temperature {C}",
        "  Fan:SystemModel,Supply Fan,always_avail,Air Loop Inlet Node,HW Coil Air Inlet "
        "Node,0.326847,Discrete,1,500,0.85,1,200,PowerPerFlow,805.1744,;",
        "  Coil:Heating:Water,",
        "    HW Coil,                 !- Name",
        "    always_avail,            !- Availability Schedule Name",
        "    1478.48,                 !- U-Factor Times Area Value {W/K}",
        "    0.000129127,             !- Maximum Water Flow Rate {m3/s}",
        "    HW Coil Water Inlet Node,   !- Water Inlet Node Name",
        "    HW Coil Water Outlet Node,  !- Water Outlet Node Name",
        "    HW Coil Air Inlet Node,  !- Air Inlet Node Name",
        "    Air Loop Outlet Node,    !- Air Outlet Node Name",
        "    UFactorTimesAreaAndDesignWaterFlowRate;  !- Performance Input Method",
        "  PlantLoop,",
        "    HW Loop,                 !- Name",
        "    Water,                   !- Fluid Type",
        "    ,                        !- User Defined Fluid Type",
        "    HW Loop Operation,       !- Plant Equipment Operation Scheme Name",
        "    HW Supply Outlet Node,   !- Loop Temperature Setpoint Node Name",
        "    100,                     !- Maximum Loop Temperature {C}",
        "    0,                       !- Minimum Loop Temperature {C}",
        "    0.000129127,             !- Maximum Loop Flow Rate {m3/s}",
        "    0,                       !- Minimum Loop Flow Rate {m3/s}",
        "    autocalculate,           !- Plant Loop Volume {m3}",
        "    HW Supply Inlet Node,    !- Plant Side Inlet Node Name",
        "    HW Supply Outlet Node,   !- Plant Side Outlet Node Name",
        "    HW Supply Branches,      !- Plant Side Branch List Name",
        "    HW Supply Connectors,    !- Plant Side Connector List Name",
        "    HW Demand Inlet Node,    !- Demand Side Inlet Node Name",
        "    HW Demand Outlet Node,   !- Demand Side Outlet Node Name",
        "    HW Demand Branches,      !- Demand Side Branch List Name",
        "    HW Demand Connectors,    !- Demand Side Connector List Name",
        "    SequentialLoad;          !- Load Distribution Scheme",
        "  SetpointManager:Scheduled,HW Loop Setpoint Manager,Temperature,hw_loop_setpoint,HW Supply Outlet Node;",
        "  PlantEquipmentOperationSchemes,HW Loop Operation,PlantEquipmentOperation:HeatingLoad,HW Heating Operation,always_avail;",
        "  PlantEquipmentOperation:HeatingLoad,HW Heating Operation,0,1E+15,HW Equipment;",
        "  PlantEquipmentList,HW Equipment,Boiler:HotWater,Boiler;",
        "  BranchList,HW Supply Branches,HW Pump Branch,HW Boiler Branch,HW Supply Bypass Branch,HW Supply Outlet Branch;",
        "  ConnectorList,HW Supply Connectors,Connector:Splitter,HW Supply Splitter,Connector:Mixer,HW Supply Mixer;",
        "  Connector:Splitter,HW Supply Splitter,HW Pump Branch,HW Boiler Branch,HW Supply Bypass Branch;",
        "  Connector:Mixer,HW Supply Mixer,HW Supply Outlet Branch,HW Boiler Branch,HW Supply Bypass Branch;",
        "  Branch,HW Pump Branch,,Pump:VariableSpeed,HW Pump,HW Supply Inlet Node,HW Pump Outlet Node;",
        "  Branch,HW Boiler Branch,,Boiler:HotWater,Boiler,HW Boiler Inlet Node,HW Boiler Outlet Node;",
        "  Branch,HW Supply Bypass Branch,,Pipe:Adiabatic,HW Supply Bypass Pipe,HW Supply Bypass Inlet Node,HW Supply Bypass Outlet Node;",
        "  Branch,HW Supply Outlet Branch,,Pipe:Adiabatic,HW Supply Outlet Pipe,HW Supply Outlet Pipe Inlet Node,HW Supply Outlet Node;",
        "  Pipe:Adiabatic,HW Supply Bypass Pipe,HW Supply Bypass Inlet Node,HW Supply Bypass Outlet Node;",
        "  Pipe:Adiabatic,HW Supply Outlet Pipe,HW Supply Outlet Pipe Inlet Node,HW Supply Outlet Node;",
        "  BranchList,HW Demand Branches,HW Demand Inlet Branch,HW Coil Branch,HW Demand Outlet Branch;",
        "  ConnectorList,HW Demand Connectors,Connector:Splitter,HW Demand Splitter,Connector:Mixer,HW Demand Mixer;",
        "  Connector:Splitter,HW Demand Splitter,HW Demand Inlet Branch,HW Coil Branch;",
        "  Connector:Mixer,HW Demand Mixer,HW Demand Outlet Branch,HW Coil Branch;",
        "  Branch,HW Demand Inlet Branch,,Pipe:Adiabatic,HW Demand Inlet Pipe,HW Demand Inlet Node,HW Demand Inlet Pipe Outlet Node;",
        "  Branch,HW Coil Branch,,Coil:Heating:Water,HW Coil,HW Coil Water Inlet Node,HW Coil Water Outlet Node;",
        "  Branch,HW Demand Outlet Branch,,Pipe:Adiabatic,HW Demand Outlet Pipe,HW Demand Outlet Pipe Inlet Node,HW Demand Outlet Node;",
        "  Pipe:Adiabatic,HW Demand Inlet Pipe,HW Demand Inlet Node,HW Demand Inlet Pipe Outlet Node;",
        "  Pipe:Adiabatic,HW Demand Outlet Pipe,HW Demand Outlet Pipe Inlet Node,HW Demand Outlet Node;",
        "  Boiler:HotWater,",
        "    Boiler,                  !- Name",
        "    NaturalGas,              !- Fuel Type",
        "    10569.2,                 !- Nominal Capacity {W}",
        "    1,                       !- Nominal Thermal Efficiency",
        "    ,                        !- Efficiency Curve Temperature Evaluation Variable",
        "    ,                        !- Normalized Boiler Efficiency Curve Name",
        "    0.000129127,             !- Design Water Flow Rate {m3/s}",
        "    ,                        !- Minimum Part Load Ratio",
        "    1,                       !- Maximum Part Load Ratio",
        "    1,                       !- Optimum Part Load Ratio",
        "    HW Boiler Inlet Node,    !- Boiler Water Inlet Node Name",
        "    HW Boiler Outlet Node,   !- Boiler Water Outlet Node Name",
        "    99.9,                    !- Water Outlet Upper Temperature Limit {C}",
        "    ConstantFlow;            !- Boiler Flow Mode",
        "  Pump:VariableSpeed,",
        "    HW Pump,                 !- Name",
        "    HW Supply Inlet Node,    !- Inlet Node Name",
        "    HW Pump Outlet Node,     !- Outlet Node Name",
        "    0.000129127,             !- Design Maximum Flow Rate {m3/s}",
        "    179352,                  !- Design Pump Head {Pa}",
        "    33,                      !- Design Power Consumption {W}",
        "    0.9,                     !- Motor Efficiency",
        "    0,                       !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "    1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 3 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 4 of the Part Load Performance Curve",
    };
    if (continuousPump) {
        lines.insert(lines.end(),
                     {
                         "    autosize,                !- Design Minimum Flow Rate {m3/s}",
                         "    Continuous,              !- Pump Control Type",
                         "    ,,,,,,,,,,,              !- Pump Flow Rate Schedule Name to Skin Loss Radiative Fraction",
                         "    PowerPerFlowPerPressure, !- Design Power Sizing Method",
                         "    348701.1,                !- Design Electric Power per Unit Flow Rate {W/(m3/s)}",
                         "    1.282051282,             !- Design Shaft Power per Unit Flow Rate per Unit Head {W/((m3/s)-Pa)}",
                         "    1;                       !- Design Minimum Flow Rate Fraction",
                     });
    } else {
        lines.insert(lines.end(),
                     {
                         "    0,                       !- Design Minimum Flow Rate {m3/s}",
                         "    Intermittent;            !- Pump Control Type",
                     });
    }
    std::string idf;
    for (auto const &line : lines) {
        idf += std::string(line) + "\n";
    }
    return idf;
}

void checkHotWaterCoilEnergyBalance(EnergyPlus::EnergyPlusData &state)
{
    auto const &coil = state.dataWaterCoils->WaterCoil(1);
    auto const &boiler = state.dataBoilers->Boiler[0];
    Real64 const coilWaterFlow = state.dataLoopNodes->Node(coil.WaterInletNodeNum).MassFlowRate;
    Real64 const cp = coil.WaterPlantLoc.loop->glycol->getSpecificHeat(state, coil.InletWaterTemp, "checkHotWaterCoilEnergyBalance");
    Real64 const coilWaterSideRate = coilWaterFlow * cp * (coil.InletWaterTemp - coil.OutletWaterTemp);

    // the coil is cycling and the loop flow is forced by the boiler (or pump): the coil receives the loop flow
    EXPECT_GT(coil.TotWaterHeatingCoilRate, 1000.0);
    EXPECT_NEAR(boiler.DesMassFlowRate, coilWaterFlow, 1.0e-6);
    // heat removed from the water matches the heat delivered to the air and the heat provided by the boiler
    EXPECT_NEAR(coil.TotWaterHeatingCoilRate, coilWaterSideRate, 0.005 * coil.TotWaterHeatingCoilRate);
    EXPECT_NEAR(coil.TotWaterHeatingCoilRate, boiler.BoilerLoad, 0.02 * coil.TotWaterHeatingCoilRate);
}
} // namespace

TEST_F(EnergyPlusFixture, WaterCoils_CyclingHeatingCoil_ConstantFlowBoilerWithoutDemandBypass)
{
    ASSERT_TRUE(process_idf(hotWaterLoopWithoutDemandBypassIdf(false)));
    SimulationManager::ManageSimulation(*state);
    checkHotWaterCoilEnergyBalance(*state);
}

TEST_F(EnergyPlusFixture, WaterCoils_CyclingHeatingCoil_ContinuousPumpWithoutDemandBypass)
{
    // pump minimum flow rate (5.05 C density) exceeds the coil maximum flow rate set by the unitary system (60 C density)
    ASSERT_TRUE(process_idf(hotWaterLoopWithoutDemandBypassIdf(true)));
    SimulationManager::ManageSimulation(*state);
    EXPECT_GT(state->dataLoopNodes->Node(state->dataPumps->PumpEquip(1).OutletNodeNum).MassFlowRate, 0.0);
    checkHotWaterCoilEnergyBalance(*state);
}
