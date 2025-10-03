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

// EnergyPlus::Low Temperature Radiant Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataSurfaceLists.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/LowTempRadiantSystem.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/PlantManager.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SizingManager.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/WeatherManager.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::LowTempRadiantSystem;
using namespace EnergyPlus::DataHeatBalance;
using namespace EnergyPlus::DataZoneEquipment;
using namespace EnergyPlus::DataSizing;

using namespace EnergyPlus::DataSurfaces;
using namespace EnergyPlus::DataSurfaceLists;
using namespace EnergyPlus::HeatBalanceManager;
using namespace EnergyPlus::PlantManager;
using namespace EnergyPlus::SizingManager;
using namespace EnergyPlus::SurfaceGeometry;
using namespace EnergyPlus::General;

Real64 constexpr CpWater = 4180.0;  // For estimating the expected result
Real64 constexpr RhoWater = 1000.0; // For estimating the expected result

class LowTempRadiantSystemTest : public EnergyPlusFixture
{
public:
    int RadSysNum;
    SystemType systemType;
    Real64 ExpectedResult1;
    Real64 ExpectedResult2;
    Real64 ExpectedResult3;
    int DesignObjectNum;

protected:
    virtual void SetUp()
    {
        EnergyPlusFixture::SetUp(); // Sets up the base fixture first.

        state->dataFluid->init_state(*state);

        state->dataLowTempRadSys->ElecRadSys.allocate(1);
        state->dataLowTempRadSys->HydrRadSys.allocate(1);
        state->dataLowTempRadSys->CFloRadSys.allocate(1);
        state->dataSize->FinalZoneSizing.allocate(1);
        state->dataSize->ZoneEqSizing.allocate(1);
        state->dataHeatBal->Zone.allocate(1);
        state->dataSize->CurZoneEqNum = 1;
        state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod.allocate(25);
        state->dataSize->ZoneSizingRunDone = true;

        state->dataSize->CurSysNum = 0;
        RadSysNum = 1;
        systemType = SystemType::Electric;
        state->dataLowTempRadSys->ElecRadSysNumericFields.allocate(1);
        state->dataLowTempRadSys->ElecRadSysNumericFields(RadSysNum).FieldNames.allocate(1);
        state->dataLowTempRadSys->HydronicRadiantSysNumericFields.allocate(1);
        state->dataLowTempRadSys->HydronicRadiantSysNumericFields(RadSysNum).FieldNames.allocate(15);
        state->dataLowTempRadSys->HydrRadSys(RadSysNum).NumCircuits.allocate(1);
        state->dataLowTempRadSys->CFloRadSys(RadSysNum).NumCircuits.allocate(1);
        // set up plant loop
        state->dataPlnt->TotNumLoops = 2;
        state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
        state->dataSize->PlantSizData.allocate(state->dataPlnt->TotNumLoops);
        state->dataSize->NumPltSizInput = state->dataPlnt->TotNumLoops;

        for (int loopindex = 1; loopindex <= state->dataPlnt->TotNumLoops; ++loopindex) {
            auto &loopside(state->dataPlnt->PlantLoop(loopindex).LoopSide(DataPlant::LoopSideLocation::Demand));
            loopside.TotalBranches = 1;
            loopside.Branch.allocate(1);
            auto &loopsidebranch(state->dataPlnt->PlantLoop(loopindex).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
            loopsidebranch.TotalComponents = 1;
            loopsidebranch.Comp.allocate(1);
        }
        state->dataPlnt->PlantLoop(1).Name = "Hot Water Loop";
        state->dataPlnt->PlantLoop(1).FluidName = "WATER";
        state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);

        state->dataPlnt->PlantLoop(2).Name = "Chilled Water Loop";
        state->dataPlnt->PlantLoop(2).FluidName = "WATER";
        state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);

        state->dataSize->PlantSizData(1).PlantLoopName = "Hot Water Loop";
        state->dataSize->PlantSizData(1).ExitTemp = 80.0;
        state->dataSize->PlantSizData(1).DeltaT = 10.0;

        state->dataSize->PlantSizData(2).PlantLoopName = "Chilled Water Loop";
        state->dataSize->PlantSizData(2).ExitTemp = 6.0;
        state->dataSize->PlantSizData(2).DeltaT = 5.0;

        ExpectedResult1 = 0.0;
        ExpectedResult2 = 0.0;
        ExpectedResult3 = 0.0;
    }

    virtual void TearDown()
    {
        EnergyPlusFixture::TearDown(); // Remember to tear down the base fixture after cleaning up derived fixture!
    }
};

TEST_F(LowTempRadiantSystemTest, SizeLowTempRadiantElectric)
{
    systemType = SystemType::Electric;
    state->dataLowTempRadSys->ElecRadSys(RadSysNum).Name = "LowTempElectric 1";
    state->dataLowTempRadSys->ElecRadSys(RadSysNum).ZonePtr = 1;
    state->dataLowTempRadSys->ElecRadSysNumericFields(RadSysNum).FieldNames(1) = "Heating Design Capacity";

    // Electric - HeatingDesignCapacity method
    state->dataLowTempRadSys->ElecRadSys(RadSysNum).MaxElecPower = AutoSize;
    state->dataLowTempRadSys->ElecRadSys(RadSysNum).HeatingCapMethod = HeatingDesignCapacity;
    state->dataLowTempRadSys->ElecRadSys(RadSysNum).ScaledHeatingCapacity = AutoSize;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesHeatLoad = 1200.0;
    SizeLowTempRadiantSystem(*state, RadSysNum, systemType);
    EXPECT_NEAR(1200.0, state->dataLowTempRadSys->ElecRadSys(RadSysNum).MaxElecPower, 0.1);

    // Electric - CapacityPerFloorArea method - hold until scalable sizing issue is resolved
    state->dataLowTempRadSys->ElecRadSys(RadSysNum).MaxElecPower = AutoSize;
    state->dataLowTempRadSys->ElecRadSys(RadSysNum).HeatingCapMethod = CapacityPerFloorArea;
    state->dataLowTempRadSys->ElecRadSys(RadSysNum).ScaledHeatingCapacity = 1.5;
    state->dataHeatBal->Zone(1).FloorArea = 500.0;
    SizeLowTempRadiantSystem(*state, RadSysNum, systemType);
    EXPECT_NEAR(750.0, state->dataLowTempRadSys->ElecRadSys(RadSysNum).MaxElecPower, 0.1);

    // Electric - FractionOfAutosizedHeatingCapacity method - hold until scalable sizing issue is resolved
    state->dataLowTempRadSys->ElecRadSys(RadSysNum).MaxElecPower = AutoSize;
    state->dataLowTempRadSys->ElecRadSys(RadSysNum).HeatingCapMethod = FractionOfAutosizedHeatingCapacity;
    state->dataLowTempRadSys->ElecRadSys(RadSysNum).ScaledHeatingCapacity = 10.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesHeatLoad = 880.0;
    SizeLowTempRadiantSystem(*state, RadSysNum, systemType);
    EXPECT_NEAR(8800.0, state->dataLowTempRadSys->ElecRadSys(RadSysNum).MaxElecPower, 0.1);
}

TEST_F(LowTempRadiantSystemTest, SizeLowTempRadiantVariableFlow)
{
    systemType = SystemType::Hydronic;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).Name = "LowTempVarFlow 1";
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).ZonePtr = 1;
    state->dataLowTempRadSys->HydronicRadiantSysNumericFields(RadSysNum).FieldNames(3) = "Heating Design Capacity";
    state->dataLowTempRadSys->HydronicRadiantSysNumericFields(RadSysNum).FieldNames(8) = "Cooling Design Capacity";

    state->dataLowTempRadSys->HydrRadSys(RadSysNum).HotWaterInNode = 1;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).HotWaterOutNode = 2;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).HWPlantLoc.loopNum = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataLowTempRadSys->HydrRadSys(RadSysNum).HotWaterInNode;

    state->dataLowTempRadSys->HydrRadSys(RadSysNum).ColdWaterInNode = 3;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).ColdWaterOutNode = 4;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).CWPlantLoc.loopNum = 2;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataLowTempRadSys->HydrRadSys(RadSysNum).ColdWaterInNode;

    // Hydronic - HeatingDesignCapacity/CoolingDesignCapacity method
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxHeat = AutoSize;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).HeatingCapMethod = HeatingDesignCapacity;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledHeatingCapacity = AutoSize;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesHeatLoad = 1200.0;
    ExpectedResult1 = state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesHeatLoad;
    ExpectedResult1 = ExpectedResult1 / (state->dataSize->PlantSizData(1).DeltaT * RhoWater * CpWater);

    state->dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxCool = AutoSize;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).CoolingCapMethod = CoolingDesignCapacity;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledCoolingCapacity = AutoSize;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesCoolLoad = 2200.0;
    ExpectedResult2 = state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesCoolLoad;
    ExpectedResult2 = ExpectedResult2 / (state->dataSize->PlantSizData(2).DeltaT * RhoWater * CpWater);

    state->dataLowTempRadSys->HydrRadSys(RadSysNum).NumCircCalcMethod = CircuitCalc::Invalid;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).NumOfSurfaces = 1;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).TubeLength = AutoSize;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).TotalSurfaceArea = 1500.0;
    ExpectedResult3 = state->dataLowTempRadSys->HydrRadSys(RadSysNum).TotalSurfaceArea / 0.15;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).SurfacePtr.allocate(1);
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).SurfacePtr(1) = 1;
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).Construction = 1;
    state->dataSurface->Surface(1).Area = 1500.0;
    state->dataConstruction->Construct.allocate(1);
    state->dataConstruction->Construct(1).ThicknessPerpend = 0.075;

    SizeLowTempRadiantSystem(*state, RadSysNum, systemType);
    EXPECT_NEAR(ExpectedResult1, state->dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxHeat, 0.1);
    EXPECT_NEAR(ExpectedResult2, state->dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxCool, 0.1);
    EXPECT_NEAR(ExpectedResult3, state->dataLowTempRadSys->HydrRadSys(RadSysNum).TubeLength, 0.1);

    // Hydronic - CapacityPerFloorArea method
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxHeat = AutoSize;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).HeatingCapMethod = CapacityPerFloorArea;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledHeatingCapacity = 10.0;
    state->dataHeatBal->Zone(1).FloorArea = 500.0;
    ExpectedResult1 = state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledHeatingCapacity * state->dataHeatBal->Zone(1).FloorArea;
    ExpectedResult1 = ExpectedResult1 / (state->dataSize->PlantSizData(1).DeltaT * RhoWater * CpWater);

    state->dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxCool = AutoSize;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).CoolingCapMethod = CapacityPerFloorArea;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledCoolingCapacity = 20.0;
    ExpectedResult2 = state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledCoolingCapacity * state->dataHeatBal->Zone(1).FloorArea;
    ExpectedResult2 = ExpectedResult2 / (state->dataSize->PlantSizData(2).DeltaT * RhoWater * CpWater);

    SizeLowTempRadiantSystem(*state, RadSysNum, systemType);
    EXPECT_NEAR(ExpectedResult1, state->dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxHeat, 0.1);
    EXPECT_NEAR(ExpectedResult2, state->dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxCool, 0.1);

    // Hydronic - FractionOfAutosizedHeating/CoolingCapacity method
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxHeat = AutoSize;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).HeatingCapMethod = FractionOfAutosizedHeatingCapacity;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledHeatingCapacity = 1.2;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesHeatLoad = 880.0;
    ExpectedResult1 = state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledHeatingCapacity *
                      state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesHeatLoad;
    ExpectedResult1 = ExpectedResult1 / (state->dataSize->PlantSizData(1).DeltaT * RhoWater * CpWater);

    state->dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxCool = AutoSize;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).CoolingCapMethod = FractionOfAutosizedCoolingCapacity;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledCoolingCapacity = 1.5;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesCoolLoad = 1200.0;
    ExpectedResult2 = state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledCoolingCapacity *
                      state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesCoolLoad;
    ExpectedResult2 = ExpectedResult2 / (state->dataSize->PlantSizData(2).DeltaT * RhoWater * CpWater);

    SizeLowTempRadiantSystem(*state, RadSysNum, systemType);
    EXPECT_NEAR(ExpectedResult1, state->dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxHeat, 0.1);
    EXPECT_NEAR(ExpectedResult2, state->dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxCool, 0.1);
}

TEST_F(LowTempRadiantSystemTest, SizeCapacityLowTempRadiantVariableFlow)
{
    systemType = SystemType::Hydronic;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).Name = "LowTempVarFlow 1";
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).ZonePtr = 1;
    state->dataLowTempRadSys->HydronicRadiantSysNumericFields(RadSysNum).FieldNames(3) = "Heating Design Capacity";
    state->dataLowTempRadSys->HydronicRadiantSysNumericFields(RadSysNum).FieldNames(8) = "Cooling Design Capacity";

    state->dataLowTempRadSys->HydrRadSys(RadSysNum).HotWaterInNode = 1;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).HotWaterOutNode = 2;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).HWPlantLoc.loopNum = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataLowTempRadSys->HydrRadSys(RadSysNum).HotWaterInNode;

    state->dataLowTempRadSys->HydrRadSys(RadSysNum).ColdWaterInNode = 3;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).ColdWaterOutNode = 4;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).CWPlantLoc.loopNum = 2;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataLowTempRadSys->HydrRadSys(RadSysNum).ColdWaterInNode;

    // Hydronic - HeatingDesignCapacity/CoolingDesignCapacity Autosize Method
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).HeatingCapMethod = HeatingDesignCapacity;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledHeatingCapacity = AutoSize;
    state->dataSize->FinalZoneSizing.allocate(state->dataSize->CurZoneEqNum);
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesHeatLoad = 1200.0;
    ExpectedResult1 = state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesHeatLoad;

    state->dataLowTempRadSys->HydrRadSys(RadSysNum).CoolingCapMethod = CoolingDesignCapacity;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledCoolingCapacity = AutoSize;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesCoolLoad = 2200.0;
    ExpectedResult2 = state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesCoolLoad;

    state->dataLowTempRadSys->HydrRadSys(RadSysNum).SurfacePtr.allocate(1);
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).SurfacePtr(1) = 1;
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).Construction = 1;
    state->dataSurface->Surface(1).Area = 1500.0;
    state->dataConstruction->Construct.allocate(1);
    state->dataConstruction->Construct(1).ThicknessPerpend = 0.075;

    SizeLowTempRadiantSystem(*state, RadSysNum, systemType);
    EXPECT_NEAR(ExpectedResult1, state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledHeatingCapacity, 0.1);
    EXPECT_NEAR(ExpectedResult2, state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledCoolingCapacity, 0.1);

    // Hydronic - CapacityPerFloorArea Capacity Sizing Method
    state->dataHeatBal->Zone(1).FloorArea = 50.0;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).HeatingCapMethod = CapacityPerFloorArea;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledHeatingCapacity = 200.0;
    ExpectedResult1 = state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledHeatingCapacity * state->dataHeatBal->Zone(1).FloorArea;

    state->dataLowTempRadSys->HydrRadSys(RadSysNum).CoolingCapMethod = CapacityPerFloorArea;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledCoolingCapacity = 250.0;
    ExpectedResult2 = state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledCoolingCapacity * state->dataHeatBal->Zone(1).FloorArea;

    SizeLowTempRadiantSystem(*state, RadSysNum, systemType);
    EXPECT_NEAR(ExpectedResult1, state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledHeatingCapacity, 0.1);
    EXPECT_NEAR(ExpectedResult2, state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledCoolingCapacity, 0.1);

    // Hydronic - FractionOfAutosizedHeating/CoolingCapacity Sizing Method
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).HeatingCapMethod = FractionOfAutosizedHeatingCapacity;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledHeatingCapacity = 1.2;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesHeatLoad = 880.0;
    ExpectedResult1 = state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledHeatingCapacity *
                      state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesHeatLoad;

    state->dataLowTempRadSys->HydrRadSys(RadSysNum).CoolingCapMethod = FractionOfAutosizedCoolingCapacity;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledCoolingCapacity = 1.5;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesCoolLoad = 1200.0;
    ExpectedResult2 = state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledCoolingCapacity *
                      state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesCoolLoad;

    SizeLowTempRadiantSystem(*state, RadSysNum, systemType);
    EXPECT_NEAR(ExpectedResult1, state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledHeatingCapacity, 0.1);
    EXPECT_NEAR(ExpectedResult2, state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledCoolingCapacity, 0.1);
}

TEST_F(LowTempRadiantSystemTest, SizeLowTempRadiantConstantFlow)
{
    systemType = SystemType::ConstantFlow;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).Name = "LowTempConstantFlow 1";
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ZonePtr = 1;
    state->dataLowTempRadSys->HydronicRadiantSysNumericFields(RadSysNum).FieldNames(2) = "Rated Flow Rate";
    state->dataLowTempRadSys->HydronicRadiantSysNumericFields(RadSysNum).FieldNames(3) = "Total length of pipe embedded in surface";

    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterInNode = 1;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterOutNode = 2;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HWPlantLoc.loopNum = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterInNode;

    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode = 3;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterOutNode = 4;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).CWPlantLoc.loopNum = 2;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode;

    // Hydronic - Hot water volume flow rate autosize
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterOutNode = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax = AutoSize;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesHeatLoad = 1200.0;
    ExpectedResult1 = state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesHeatLoad;
    ExpectedResult1 = ExpectedResult1 / (state->dataSize->PlantSizData(1).DeltaT * RhoWater * CpWater);

    state->dataLowTempRadSys->CFloRadSys(RadSysNum).SurfacePtr.allocate(1);
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).SurfacePtr(1) = 1;
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).Construction = 1;
    state->dataSurface->Surface(1).Area = 150.0;
    state->dataConstruction->Construct.allocate(1);
    state->dataConstruction->Construct(1).ThicknessPerpend = 0.075;

    SizeLowTempRadiantSystem(*state, RadSysNum, systemType);
    EXPECT_NEAR(ExpectedResult1, state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax, 0.001);

    // Hydronic - cold water volume flow rate autosize
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterInNode = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterOutNode = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode = 3;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterOutNode = 4;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax = AutoSize;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesCoolLoad = 2200.0;
    ExpectedResult2 = state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesCoolLoad;
    ExpectedResult2 = ExpectedResult2 / (state->dataSize->PlantSizData(2).DeltaT * RhoWater * CpWater);

    SizeLowTempRadiantSystem(*state, RadSysNum, systemType);
    EXPECT_NEAR(ExpectedResult2, state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax, 0.001);

    // Hydronic - maximum water volume flow rate autosize
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax = AutoSize;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterInNode = 1;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterOutNode = 2;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode = 3;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterOutNode = 4;

    // Hydronic - embeded tube length autosize
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).NumCircCalcMethod = CircuitCalc::Invalid;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).NumOfSurfaces = 1;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).TubeLength = AutoSize;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).TotalSurfaceArea = 150.0;
    ExpectedResult3 = state->dataLowTempRadSys->CFloRadSys(RadSysNum).TotalSurfaceArea / 0.15;

    SizeLowTempRadiantSystem(*state, RadSysNum, systemType);
    EXPECT_NEAR(std::max(ExpectedResult1, ExpectedResult2), state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax, 0.001);
    EXPECT_NEAR(ExpectedResult3, state->dataLowTempRadSys->CFloRadSys(RadSysNum).TubeLength, 0.1);
}

TEST_F(LowTempRadiantSystemTest, AutosizeLowTempRadiantVariableFlowTest)
{

    int RadSysNum(1);
    Real64 HeatingCapacity;
    Real64 CoolingCapacity;
    Real64 HotWaterFlowRate;
    Real64 ChilledWaterFlowRate;
    Real64 TubeLengthDes;
    Real64 Density;
    Real64 Cp;
    bool ErrorsFound = false;

    std::string const idf_objects = delimited_string({
        "  Building,",
        "    NONE,                    !- Name",
        "    0.0000000E+00,           !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    3.9999999E-02,           !- Loads Convergence Tolerance Value",
        "    0.4000000,               !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullInteriorAndExterior, !- Solar Distribution",
        "    25,                      !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",

        "  Zone,",
        "    West Zone,               !- Name",
        "    0.0000000E+00,           !- Direction of Relative North {deg}",
        "    0.0000000E+00,           !- X Origin {m}",
        "    0.0000000E+00,           !- Y Origin {m}",
        "    0.0000000E+00,           !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.5,                     !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  Site:GroundTemperature:BuildingSurface,20.03,20.03,20.13,20.30,20.43,20.52,20.62,20.77,20.78,20.55,20.44,20.20;",

        "  ZoneHVAC:EquipmentConnections,",
        "    West Zone,               !- Zone Name",
        "    Zone1Equipment,          !- Zone Conditioning Equipment List Name",
        "    Zone1Inlets,             !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    Zone 1 Node,             !- Zone Air Node Name",
        "    Zone 1 Outlet Node;      !- Zone Return Air Node Name",

        "  ZoneHVAC:EquipmentList,",
        "    Zone1Equipment,          !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:LowTemperatureRadiant:VariableFlow,  !- Zone Equipment 1 Object Type",
        "    West Zone Radiant Floor, !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "  ZoneHVAC:LowTemperatureRadiant:VariableFlow,",
        "    West Zone Radiant Floor, !- Name",
        "    West Zone Radiant Floor Design,    	!- Design Object Name",
        "    RadiantSysAvailSched,    !- Availability Schedule Name",
        "    West Zone,               !- Zone Name",
        "    Zn001:Flr001,            !- Surface Name or Radiant Surface Group Name",
        "    autosize,                !- Hydronic Tubing Length {m}",
        "    ,                        !- Heating Design Capacity {W}",
        "    autosize,                !- Maximum Hot Water Flow {m3/s}",
        "    West Zone Radiant Water Inlet Node,  !- Heating Water Inlet Node Name",
        "    West Zone Radiant Water Outlet Node, !- Heating Water Outlet Node Name",
        "    ,                        !- Cooling Design Capacity {W}",
        "    autosize,                !- Maximum Cold Water Flow {m3/s}",
        "    Zone 1 Cooling Water Inlet Node,     !- Cooling Water Inlet Node Name",
        "    Zone 1 Cooling Water Outlet Node,    !- Cooling Water Outlet Node Name",
        "    ,                        !- Number of Circuits",
        "    ;                        !- Circuit Length {m}",

        "  ZoneHVAC:LowTemperatureRadiant:VariableFlow:Design,",
        "    West Zone Radiant Floor Design, !- Name",
        "    ConvectionOnly,          !- Fluid to Radiant Surface Heat Transfer Model",
        "    0.012,                   !- Hydronic Tubing Inside Diameter {m}",
        "    0.016,                   !- Hydronic Tubing Outside Diameter {m}",
        "    0.35,                    !- Hydronic Tubing Conductivity {W/m-K}",
        "    MeanAirTemperature,      !- Temperature Control Type",
        "    HalfFlowPower,           !- Setpoint Type",
        "    FractionOfAutosizedHeatingCapacity,  !- Heating Design Capacity Method",
        "    ,                        !- Heating Design Capacity Per Floor Area {W/m2}",
        "    0.9,                     !- Fraction of Autosized Heating Design Capacity",
        "    2.0,                     !- Heating Control Throttling Range {deltaC}",
        "    Radiant Heating Setpoints,  !- Heating Control Temperature Schedule Name",
        "    FractionOfAutosizedCoolingCapacity,   !- Cooling Design Capacity Method",
        "    ,                        !- Cooling Design Capacity Per Floor Area {W/m2}",
        "    1.2,                     !- Fraction of Autosized Cooling Design Capacity",
        "    2.0,                     !- Cooling Control Throttling Range {deltaC}",
        "    Radiant Cooling Setpoints,           !- Cooling Control Temperature Schedule Name",
        "    ,                        !- Condensation Control Type",
        "    ;                        !- Condensation Control Dewpoint Offset {C}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Flr001,            !- Name",
        "    Floor,                   !- Surface Type",
        "    Slab Floor with Radiant, !- Construction Name",
        "    West Zone,               !- Zone Name",
        "    ,                        !- Space Name",
        "    Ground,                  !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    1.000000,                !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0, 0.0, 0.0,           !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0, 6.096000,0.0,       !- X,Y,Z ==> Vertex 2 {m}",
        "    6.096000,6.096000,0.0,   !- X,Y,Z ==> Vertex 3 {m}",
        "    6.096000, 0.0, 0.0;      !- X,Y,Z ==> Vertex 4 {m}",

        "  ConstructionProperty:InternalHeatSource,",
        "    Radiant Source,          !- Name",
        "    Slab Floor with Radiant, !- Construction Name",
        "    4,                       !- Source Present After Layer Number",
        "    4,                       !- Temperature Calculation Requested After Layer Number",
        "    1,                       !- Dimensions for the CTF Calculation",
        "    0.1524,                  !- Tube Spacing {m}",
        "    0.0;                     !- Two-Dimensional Position of Interior Temperature Calculation Request",

        "  Construction,",
        "    Slab Floor with Radiant, !- Name",
        "    CONCRETE - DRIED SAND AND GRAVEL 4 IN,  !- Outside Layer",
        "    INS - EXPANDED EXT POLYSTYRENE R12 2 IN,  !- Layer 2",
        "    GYP1,                    !- Layer 3",
        "    GYP2,                    !- Layer 4",
        "    FINISH FLOORING - TILE 1 / 16 IN;  !- Layer 5",

        "  Material,",
        "    CONCRETE - DRIED SAND AND GRAVEL 4 IN,  !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1000000,               !- Thickness {m}",
        "    1.290000,                !- Conductivity {W/m-K}",
        "    2242.580,                !- Density {kg/m3}",
        "    830.00000,               !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6000000,               !- Solar Absorptance",
        "    0.6000000;               !- Visible Absorptance",

        "  Material,",
        "    INS - EXPANDED EXT POLYSTYRENE R12 2 IN,  !- Name",
        "    Rough,                   !- Roughness",
        "    5.0000001E-02,           !- Thickness {m}",
        "    2.0000000E-02,           !- Conductivity {W/m-K}",
        "    56.06000,                !- Density {kg/m3}",
        "    1210.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.5000000,               !- Solar Absorptance",
        "    0.5000000;               !- Visible Absorptance",

        "  Material,",
        "    GYP1,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    1.2700000E-02,           !- Thickness {m}",
        "    7.8450000E-01,           !- Conductivity {W/m-K}",
        "    1842.1221,               !- Density {kg/m3}",
        "    988.000,                 !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.5000000,               !- Solar Absorptance",
        "    0.5000000;               !- Visible Absorptance",

        "  Material,",
        "    GYP2,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    1.9050000E-02,           !- Thickness {m}",
        "    7.8450000E-01,           !- Conductivity {W/m-K}",
        "    1842.1221,               !- Density {kg/m3}",
        "    988.000,                 !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.5000000,               !- Solar Absorptance",
        "    0.5000000;               !- Visible Absorptance",

        "  Material,",
        "    FINISH FLOORING - TILE 1 / 16 IN,  !- Name",
        "    Smooth,                  !- Roughness",
        "    1.6000000E-03,           !- Thickness {m}",
        "    0.1700000,               !- Conductivity {W/m-K}",
        "    1922.210,                !- Density {kg/m3}",
        "    1250.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.5000000,               !- Solar Absorptance",
        "    0.5000000;               !- Visible Absorptance",

        "  Schedule:Compact,",
        "    RADIANTSYSAVAILSCHED,    !- Name",
        "    FRACTION,                !- Schedule Type Limits Name",
        "    Through: 3/31,           !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,1.00,       !- Field 3",
        "    Through: 9/30,           !- Field 5",
        "    For: Alldays,            !- Field 6",
        "    Until: 24:00,0.00,       !- Field 7",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 24:00,1.00;       !- Field 11",

        "  Schedule:Compact,",
        "    HW LOOP TEMP SCHEDULE,   !- Name",
        "    TEMPERATURE,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,60.00;      !- Field 3",

        "  Schedule:Compact,",
        "    RADIANT HEATING SETPOINTS,  !- Name",
        "    TEMPERATURE,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 7:00,12.00,       !- Field 3",
        "    Until: 17:00,17.00,      !- Field 5",
        "    Until: 24:00,12.00;      !- Field 7",

        "  Sizing:Plant,",
        "    Hot Water Loop,          !- Plant or Condenser Loop Name",
        "    heating,                 !- Loop Type",
        "    60.,                     !- Design Loop Exit Temperature {C}",
        "    10;                      !- Loop Design Temperature Difference {deltaC}",

        "  PlantLoop,",
        "    Hot Water Loop,          !- Name",
        "    Water,                   !- Fluid Type",
        "    ,                        !- User Defined Fluid Type",
        "    Hot Loop Operation,      !- Plant Equipment Operation Scheme Name",
        "    HW Supply Outlet Node,   !- Loop Temperature Setpoint Node Name",
        "    100,                     !- Maximum Loop Temperature {C}",
        "    10,                      !- Minimum Loop Temperature {C}",
        "    0.0043,                  !- Maximum Loop Flow Rate {m3/s}",
        "    0,                       !- Minimum Loop Flow Rate {m3/s}",
        "    autocalculate,           !- Plant Loop Volume {m3}",
        "    HW Supply Inlet Node,    !- Plant Side Inlet Node Name",
        "    HW Supply Outlet Node,   !- Plant Side Outlet Node Name",
        "    Heating Supply Side Branches,  !- Plant Side Branch List Name",
        "    Heating Supply Side Connectors,  !- Plant Side Connector List Name",
        "    HW Demand Inlet Node,    !- Demand Side Inlet Node Name",
        "    HW Demand Outlet Node,   !- Demand Side Outlet Node Name",
        "    Heating Demand Side Branches,  !- Demand Side Branch List Name",
        "    Heating Demand Side Connectors,  !- Demand Side Connector List Name",
        "    Optimal,                 !- Load Distribution Scheme",
        "    ,                        !- Availability Manager List Name",
        "    ,                        !- Plant Loop Demand Calculation Scheme",
        "    ,                        !- Common Pipe Simulation",
        "    ,                        !- Pressure Simulation Type",
        "    2.0;                     !- Loop Circulation Time {minutes}",

        "  SetpointManager:Scheduled,",
        "    Hot Water Loop Setpoint Manager,  !- Name",
        "    Temperature,             !- Control Variable",
        "    HW Loop Temp Schedule,   !- Schedule Name",
        "    Hot Water Loop Setpoint Node List;  !- Setpoint Node or NodeList Name",

        "  NodeList,",
        "    Hot Water Loop Setpoint Node List,  !- Name",
        "    HW Supply Outlet Node;   !- Node 1 Name",

        "  BranchList,",
        "    Heating Supply Side Branches,  !- Name",
        "    Heating Supply Inlet Branch,  !- Branch 1 Name",
        "    Heating Purchased Hot Water Branch,  !- Branch 2 Name",
        "    Heating Supply Bypass Branch,  !- Branch 3 Name",
        "    Heating Supply Outlet Branch;  !- Branch 4 Name",

        "  ConnectorList,",
        "    Heating Supply Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    Heating Supply Splitter, !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    Heating Supply Mixer;    !- Connector 2 Name",

        "  Branch,",
        "    Heating Supply Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:VariableSpeed,      !- Component 1 Object Type",
        "    HW Circ Pump,            !- Component 1 Name",
        "    HW Supply Inlet Node,    !- Component 1 Inlet Node Name",
        "    HW Pump Outlet Node;     !- Component 1 Outlet Node Name",

        "  Branch,",
        "    Heating Purchased Hot Water Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    DistrictHeating:Water,         !- Component 1 Object Type",
        "    Purchased Heating,       !- Component 1 Name",
        "    Purchased Heat Inlet Node,  !- Component 1 Inlet Node Name",
        "    Purchased Heat Outlet Node;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    Heating Supply Bypass Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Heating Supply Side Bypass,  !- Component 1 Name",
        "    Heating Supply Bypass Inlet Node,  !- Component 1 Inlet Node Name",
        "    Heating Supply Bypass Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Heating Supply Side Bypass,  !- Name",
        "    Heating Supply Bypass Inlet Node,  !- Inlet Node Name",
        "    Heating Supply Bypass Outlet Node;  !- Outlet Node Name",

        "  Branch,",
        "    Heating Supply Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Heating Supply Outlet,   !- Component 1 Name",
        "    Heating Supply Exit Pipe Inlet Node,  !- Component 1 Inlet Node Name",
        "    HW Supply Outlet Node;   !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Heating Supply Outlet,   !- Name",
        "    Heating Supply Exit Pipe Inlet Node,  !- Inlet Node Name",
        "    HW Supply Outlet Node;   !- Outlet Node Name",

        "  BranchList,",
        "    Heating Demand Side Branches,  !- Name",
        "    Reheat Inlet Branch,     !- Branch 1 Name",
        "    Zone 1 Radiant Branch,   !- Branch 5 Name",
        "    Reheat Bypass Branch,    !- Branch 8 Name",
        "    Reheat Outlet Branch;    !- Branch 9 Name",

        "  ConnectorList,",
        "    Heating Demand Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    Reheat Splitter,         !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    Reheat Mixer;            !- Connector 2 Name",

        "  Branch,",
        "    Reheat Inlet Branch,     !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Reheat Inlet Pipe,       !- Component 1 Name",
        "    HW Demand Inlet Node,    !- Component 1 Inlet Node Name",
        "    HW Demand Entrance Pipe Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Reheat Inlet Pipe,       !- Name",
        "    HW Demand Inlet Node,    !- Inlet Node Name",
        "    HW Demand Entrance Pipe Outlet Node;  !- Outlet Node Name",

        "  Branch,",
        "    Reheat Outlet Branch,    !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Reheat Outlet Pipe,      !- Component 1 Name",
        "    HW Demand Exit Pipe Inlet Node,  !- Component 1 Inlet Node Name",
        "    HW Demand Outlet Node;   !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Reheat Outlet Pipe,      !- Name",
        "    HW Demand Exit Pipe Inlet Node,  !- Inlet Node Name",
        "    HW Demand Outlet Node;   !- Outlet Node Name",

        "  Branch,",
        "    Zone 1 Radiant Branch,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    ZoneHVAC:LowTemperatureRadiant:VariableFlow,  !- Component 1 Object Type",
        "    West Zone Radiant Floor, !- Component 1 Name",
        "    West Zone Radiant Water Inlet Node,  !- Component 1 Inlet Node Name",
        "    West Zone Radiant Water Outlet Node;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    Reheat Bypass Branch,    !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Reheat Bypass,           !- Component 1 Name",
        "    Reheat Bypass Inlet Node,!- Component 1 Inlet Node Name",
        "    Reheat Bypass Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Reheat Bypass,           !- Name",
        "    Reheat Bypass Inlet Node,!- Inlet Node Name",
        "    Reheat Bypass Outlet Node;  !- Outlet Node Name",

        "  Connector:Splitter,",
        "    Reheat Splitter,         !- Name",
        "    Reheat Inlet Branch,     !- Inlet Branch Name",
        "    Zone 1 Radiant Branch,   !- Outlet Branch 4 Name",
        "    Reheat Bypass Branch;    !- Outlet Branch 7 Name",

        "  Connector:Mixer,",
        "    Reheat Mixer,            !- Name",
        "    Reheat Outlet Branch,    !- Outlet Branch Name",
        "    Zone 1 Radiant Branch,   !- Inlet Branch 4 Name",
        "    Reheat Bypass Branch;    !- Inlet Branch 7 Name",

        "  Connector:Splitter,",
        "    Heating Supply Splitter, !- Name",
        "    Heating Supply Inlet Branch,  !- Inlet Branch Name",
        "    Heating Purchased Hot Water Branch,  !- Outlet Branch 1 Name",
        "    Heating Supply Bypass Branch;  !- Outlet Branch 2 Name",

        "  Connector:Mixer,",
        "    Heating Supply Mixer,    !- Name",
        "    Heating Supply Outlet Branch,  !- Outlet Branch Name",
        "    Heating Purchased Hot Water Branch,  !- Inlet Branch 1 Name",
        "    Heating Supply Bypass Branch;  !- Inlet Branch 2 Name",

        "  PlantEquipmentOperationSchemes,",
        "    Hot Loop Operation,      !- Name",
        "    PlantEquipmentOperation:HeatingLoad,  !- Control Scheme 1 Object Type",
        "    Purchased Only,          !- Control Scheme 1 Name",
        "    ON;                      !- Control Scheme 1 Schedule Name",

        "  PlantEquipmentOperation:HeatingLoad,",
        "    Purchased Only,          !- Name",
        "    0,                       !- Load Range 1 Lower Limit {W}",
        "    1000000,                 !- Load Range 1 Upper Limit {W}",
        "    heating plant;           !- Range 1 Equipment List Name",

        "  PlantEquipmentList,",
        "    heating plant,           !- Name",
        "    DistrictHeating:Water,         !- Equipment 1 Object Type",
        "    Purchased Heating;       !- Equipment 1 Name",

        "  Pump:VariableSpeed,",
        "    HW Circ Pump,            !- Name",
        "    HW Supply Inlet Node,    !- Inlet Node Name",
        "    HW Pump Outlet Node,     !- Outlet Node Name",
        "    .0043,                   !- Rated Flow Rate {m3/s}",
        "    300000,                  !- Rated Pump Head {Pa}",
        "    2000,                    !- Rated Power Consumption {W}",
        "    .87,                     !- Motor Efficiency",
        "    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "    1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 3 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 4 of the Part Load Performance Curve",
        "    0,                       !- Minimum Flow Rate {m3/s}",
        "    INTERMITTENT;            !- Pump Control Type",

        "  DistrictHeating:Water,",
        "    Purchased Heating,       !- Name",
        "    Purchased Heat Inlet Node,  !- Hot Water Inlet Node Name",
        "    Purchased Heat Outlet Node,  !- Hot Water Outlet Node Name",
        "    1000000;                 !- Nominal Capacity {W}",

        "  Sizing:Plant,",
        "    Chilled Water Loop,      !- Plant or Condenser Loop Name",
        "    cooling,                 !- Loop Type",
        "    6.7,                     !- Design Loop Exit Temperature {C}",
        "    2;                       !- Loop Design Temperature Difference {deltaC}",

        "  PlantLoop,",
        "    Chilled Water Loop,      !- Name",
        "    Water,                   !- Fluid Type",
        "    ,                        !- User Defined Fluid Type",
        "    CW Loop Operation,       !- Plant Equipment Operation Scheme Name",
        "    CW Supply Outlet Node,   !- Loop Temperature Setpoint Node Name",
        "    98,                      !- Maximum Loop Temperature {C}",
        "    1,                       !- Minimum Loop Temperature {C}",
        "    0.0011,                  !- Maximum Loop Flow Rate {m3/s}",
        "    0,                       !- Minimum Loop Flow Rate {m3/s}",
        "    autocalculate,           !- Plant Loop Volume {m3}",
        "    CW Supply Inlet Node,    !- Plant Side Inlet Node Name",
        "    CW Supply Outlet Node,   !- Plant Side Outlet Node Name",
        "    Cooling Supply Side Branches,  !- Plant Side Branch List Name",
        "    Cooling Supply Side Connectors,  !- Plant Side Connector List Name",
        "    CW Demand Inlet Node,    !- Demand Side Inlet Node Name",
        "    CW Demand Outlet Node,   !- Demand Side Outlet Node Name",
        "    Cooling Demand Side Branches,  !- Demand Side Branch List Name",
        "    Cooling Demand Side Connectors,  !- Demand Side Connector List Name",
        "    Optimal,                 !- Load Distribution Scheme",
        "    ,                        !- Availability Manager List Name",
        "    ,                        !- Plant Loop Demand Calculation Scheme",
        "    ,                        !- Common Pipe Simulation",
        "    ,                        !- Pressure Simulation Type",
        "    2.0;                     !- Loop Circulation Time {minutes}",

        "  SetpointManager:Scheduled,",
        "    Chilled Water Loop Setpoint Manager,  !- Name",
        "    Temperature,             !- Control Variable",
        "    CW Loop Temp Schedule,   !- Schedule Name",
        "    Chilled Water Loop Setpoint Node List;  !- Setpoint Node or NodeList Name",

        "	Schedule:Compact,",
        "	 CW LOOP TEMP SCHEDULE, !- Name",
        "	 TEMPERATURE,           !- Schedule Type Limits Name",
        "	 Through: 12/31,        !- Field 1",
        "	 For: Alldays,          !- Field 2",
        "	 Until: 24:00, 10.0;    !- Field 3",

        "  NodeList,",
        "    Chilled Water Loop Setpoint Node List,  !- Name",
        "    CW Supply Outlet Node;   !- Node 1 Name",

        "  BranchList,",
        "    Cooling Supply Side Branches,  !- Name",
        "    CW Pump Branch,          !- Branch 1 Name",
        "    Purchased Cooling Branch,!- Branch 4 Name",
        "    Supply Bypass Branch,    !- Branch 5 Name",
        "    Cooling Supply Outlet;   !- Branch 6 Name",

        "  BranchList,",
        "    Cooling Demand Side Branches,  !- Name",
        "    Cooling Demand Inlet,    !- Branch 1 Name",
        "    Zone 1 Cooling Branch,   !- Branch 2 Name",
        "    Demand Bypass Branch,    !- Branch 3 Name",
        "    Cooling Demand Outlet;   !- Branch 4 Name",

        "  ConnectorList,",
        "    Cooling Supply Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    CW Loop Splitter,        !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    CW Loop Mixer;           !- Connector 2 Name",

        "  ConnectorList,",
        "    Cooling Demand Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    CW Demand Splitter,      !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    CW Demand Mixer;         !- Connector 2 Name",

        "  Branch,",
        "    Cooling Demand Inlet,    !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Demand Side Inlet Pipe,  !- Component 1 Name",
        "    CW Demand Inlet Node,    !- Component 1 Inlet Node Name",
        "    CW Demand Entrance Pipe Outlet Node;  !- Component 1 Outlet Node Name",

        "  Schedule:Compact,",
        "    RADIANT COOLING SETPOINTS,  !- Name",
        "    TEMPERATURE,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,26.0;       !- Field 3",

        "  Pipe:Adiabatic,",
        "    Demand Side Inlet Pipe,  !- Name",
        "    CW Demand Inlet Node,    !- Inlet Node Name",
        "    CW Demand Entrance Pipe Outlet Node;  !- Outlet Node Name",

        "  Branch,",
        "    Zone 1 Cooling Branch,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    ZoneHVAC:LowTemperatureRadiant:VariableFlow,  !- Component 1 Object Type",
        "    West Zone Radiant Floor,   !- Component 1 Name",
        "    Zone 1 Cooling Water Inlet Node,  !- Component 1 Inlet Node Name",
        "    Zone 1 Cooling Water Outlet Node; !- Component 1 Outlet Node Name",

        "  Branch,",
        "    Demand Bypass Branch,    !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Demand Side Bypass,      !- Component 1 Name",
        "    CW Demand Bypass Inlet Node,  !- Component 1 Inlet Node Name",
        "    CW Demand Bypass Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Demand Side Bypass,      !- Name",
        "    CW Demand Bypass Inlet Node,  !- Inlet Node Name",
        "    CW Demand Bypass Outlet Node;  !- Outlet Node Name",

        "  Branch,",
        "    Cooling Demand Outlet,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    CW Demand Side Outlet Pipe,  !- Component 1 Name",
        "    CW Demand Exit Pipe Inlet Node,  !- Component 1 Inlet Node Name",
        "    CW Demand Outlet Node;   !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    CW Demand Side Outlet Pipe,  !- Name",
        "    CW Demand Exit Pipe Inlet Node,  !- Inlet Node Name",
        "    CW Demand Outlet Node;   !- Outlet Node Name",

        "  Branch,",
        "    Cooling Supply Outlet,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Supply Side Outlet Pipe, !- Component 1 Name",
        "    Supply Side Exit Pipe Inlet Node,  !- Component 1 Inlet Node Name",
        "    CW Supply Outlet Node;   !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Supply Side Outlet Pipe, !- Name",
        "    Supply Side Exit Pipe Inlet Node,  !- Inlet Node Name",
        "    CW Supply Outlet Node;   !- Outlet Node Name",

        "  Branch,",
        "    CW Pump Branch,          !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:VariableSpeed,      !- Component 1 Object Type",
        "    Circ Pump,               !- Component 1 Name",
        "    CW Supply Inlet Node,    !- Component 1 Inlet Node Name",
        "    CW Pump Outlet Node;     !- Component 1 Outlet Node Name",

        "  Branch,",
        "    Purchased Cooling Branch,!- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    DistrictCooling,         !- Component 1 Object Type",
        "    Purchased Cooling,       !- Component 1 Name",
        "    Purchased Cooling Inlet Node,  !- Component 1 Inlet Node Name",
        "    Purchased Cooling Outlet Node; !- Component 1 Outlet Node Name",

        "  Branch,",
        "    Supply Bypass Branch,    !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Supply Side Bypass,      !- Component 1 Name",
        "    CW Supply Bypass Inlet Node,  !- Component 1 Inlet Node Name",
        "    CW Supply Bypass Outlet Node; !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Supply Side Bypass,      !- Name",
        "    CW Supply Bypass Inlet Node,  !- Inlet Node Name",
        "    CW Supply Bypass Outlet Node; !- Outlet Node Name",

        "  Connector:Splitter,",
        "    CW Loop Splitter,        !- Name",
        "    CW Pump Branch,          !- Inlet Branch Name",
        "    Purchased Cooling Branch,!- Outlet Branch 3 Name",
        "    Supply Bypass Branch;    !- Outlet Branch 4 Name",

        "  Connector:Mixer,",
        "    CW Loop Mixer,           !- Name",
        "    Cooling Supply Outlet,   !- Outlet Branch Name",
        "    Purchased Cooling Branch,!- Inlet Branch 3 Name",
        "    Supply Bypass Branch;    !- Inlet Branch 4 Name",

        "  Connector:Splitter,",
        "    CW Demand Splitter,      !- Name",
        "    Cooling Demand Inlet,    !- Inlet Branch Name",
        "    Demand Bypass Branch,    !- Outlet Branch 1 Name",
        "    Zone 1 Cooling Branch;   !- Outlet Branch 2 Name",

        "  Connector:Mixer,",
        "    CW Demand Mixer,         !- Name",
        "    Cooling Demand Outlet,   !- Outlet Branch Name",
        "    Zone 1 Cooling Branch,   !- Inlet Branch 1 Name",
        "    Demand Bypass Branch;    !- Inlet Branch 2 Name",

        "  PlantEquipmentOperationSchemes,",
        "    CW Loop Operation,       !- Name",
        "    PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type",
        "    Always Operation,        !- Control Scheme 1 Name",
        "    Always;                  !- Control Scheme 1 Schedule Name",

        "  Schedule:Compact,",
        "    Always,                  !- Name",
        "    FRACTION,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "  PlantEquipmentOperation:CoolingLoad,",
        "    Always Operation,        !- Name",
        "    0,                       !- Load Range 1 Lower Limit {W}",
        "    70000,                   !- Load Range 1 Upper Limit {W}",
        "    Purchased Only;          !- Range 3 Equipment List Name",

        "  PlantEquipmentList,",
        "    Purchased Only,          !- Name",
        "    DistrictCooling,         !- Equipment 1 Object Type",
        "    Purchased Cooling;       !- Equipment 1 Name",

        "  DistrictCooling,",
        "    Purchased Cooling,             !- Name",
        "    Purchased Cooling Inlet Node,  !- Chilled Water Inlet Node Name",
        "    Purchased Cooling Outlet Node, !- Chilled Water Outlet Node Name",
        "    680000;                        !- Nominal Capacity {W}",

        "  Pump:VariableSpeed,",
        "    Circ Pump,               !- Name",
        "    CW Supply Inlet Node,    !- Inlet Node Name",
        "    CW Pump Outlet Node,     !- Outlet Node Name",
        "    .0011,                   !- Rated Flow Rate {m3/s}",
        "    300000,                  !- Rated Pump Head {Pa}",
        "    500,                     !- Rated Power Consumption {W}",
        "    .87,                     !- Motor Efficiency",
        "    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "    1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 3 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 4 of the Part Load Performance Curve",
        "    0,                       !- Minimum Flow Rate {m3/s}",
        "    INTERMITTENT;            !- Pump Control Type",

    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetProjectControlData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    EXPECT_EQ("WEST ZONE", state->dataHeatBal->Zone(1).Name);

    HeatBalanceManager::SetPreConstructionInputParameters(*state);
    Material::GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    GetPlantSizingInput(*state);
    GetPlantLoopData(*state);
    GetPlantInput(*state);
    SetupInitialPlantCallingOrder(*state);
    SetupBranchControlTypes(*state);
    state->dataSurface->WorldCoordSystem = true;
    GetSurfaceListsInputs(*state);

    ErrorsFound = false;
    SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    GetLowTempRadiantSystem(*state);
    EXPECT_EQ(1, state->dataLowTempRadSys->NumOfHydrLowTempRadSys);
    EXPECT_EQ("WEST ZONE RADIANT FLOOR", state->dataLowTempRadSys->RadSysTypes(RadSysNum).Name);
    EXPECT_ENUM_EQ(SystemType::Hydronic, state->dataLowTempRadSys->RadSysTypes(RadSysNum).systemType);

    ErrorsFound = false;
    PlantUtilities::ScanPlantLoopsForObject(*state,
                                            state->dataLowTempRadSys->HydrRadSys(RadSysNum).Name,
                                            DataPlant::PlantEquipmentType::LowTempRadiant_VarFlow,
                                            state->dataLowTempRadSys->HydrRadSys(RadSysNum).HWPlantLoc,
                                            ErrorsFound,
                                            _,
                                            _,
                                            _,
                                            state->dataLowTempRadSys->HydrRadSys(RadSysNum).HotWaterInNode,
                                            _);
    EXPECT_FALSE(ErrorsFound);

    ErrorsFound = false;
    PlantUtilities::ScanPlantLoopsForObject(*state,
                                            state->dataLowTempRadSys->HydrRadSys(RadSysNum).Name,
                                            DataPlant::PlantEquipmentType::LowTempRadiant_VarFlow,
                                            state->dataLowTempRadSys->HydrRadSys(RadSysNum).CWPlantLoc,
                                            ErrorsFound,
                                            _,
                                            _,
                                            _,
                                            state->dataLowTempRadSys->HydrRadSys(RadSysNum).ColdWaterInNode,
                                            _);
    EXPECT_FALSE(ErrorsFound);

    state->dataSize->CurZoneEqNum = 1;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->FinalZoneSizing.allocate(state->dataSize->CurZoneEqNum);
    state->dataSize->ZoneEqSizing.allocate(state->dataSize->CurZoneEqNum);

    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod.allocate(25);
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod(HVAC::HeatingCapacitySizing) =
        DataSizing::FractionOfAutosizedHeatingCapacity;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod(HVAC::CoolingCapacitySizing) =
        DataSizing::FractionOfAutosizedCoolingCapacity;
    // heating capacity sizing calculation
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesHeatLoad = 10000.0;
    HeatingCapacity = state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesHeatLoad *
                      state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledHeatingCapacity;
    // cooling capacity sizing calculation
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesCoolLoad = 10000.0;
    CoolingCapacity = state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesCoolLoad *
                      state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledCoolingCapacity;
    // hot water flow rate sizing calculation
    Density = state->dataPlnt->PlantLoop(state->dataLowTempRadSys->HydrRadSys(RadSysNum).HWPlantLoc.loopNum)
                  .glycol->getDensity(*state, 60.0, "AutosizeLowTempRadiantVariableFlowTest");
    Cp = state->dataPlnt->PlantLoop(state->dataLowTempRadSys->HydrRadSys(RadSysNum).HWPlantLoc.loopNum)
             .glycol->getSpecificHeat(*state, 60.0, "AutosizeLowTempRadiantVariableFlowTest");
    HotWaterFlowRate = HeatingCapacity / (state->dataSize->PlantSizData(1).DeltaT * Cp * Density);
    // chilled water flow rate sizing calculation
    Density = state->dataPlnt->PlantLoop(state->dataLowTempRadSys->HydrRadSys(RadSysNum).CWPlantLoc.loopNum)
                  .glycol->getDensity(*state, 5.05, "AutosizeLowTempRadiantVariableFlowTest");
    Cp = state->dataPlnt->PlantLoop(state->dataLowTempRadSys->HydrRadSys(RadSysNum).CWPlantLoc.loopNum)
             .glycol->getSpecificHeat(*state, 5.05, "AutosizeLowTempRadiantVariableFlowTest");
    ChilledWaterFlowRate = CoolingCapacity / (state->dataSize->PlantSizData(2).DeltaT * Cp * Density);
    // tuble length sizing calculation
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).TotalSurfaceArea =
        state->dataSurface->Surface(state->dataLowTempRadSys->HydrRadSys(RadSysNum).SurfacePtr(1)).Area;
    TubeLengthDes =
        state->dataLowTempRadSys->HydrRadSys(RadSysNum).TotalSurfaceArea / 0.1524; // tube length uses the construction perpendicular spacing
    // do autosize calculations
    SizeLowTempRadiantSystem(*state, RadSysNum, state->dataLowTempRadSys->RadSysTypes(RadSysNum).systemType);
    // Test autosized heat/cool capacity
    EXPECT_EQ(HeatingCapacity, state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledHeatingCapacity);
    EXPECT_EQ(CoolingCapacity, state->dataLowTempRadSys->HydrRadSys(RadSysNum).ScaledCoolingCapacity);
    // Test autosized heat/cool flow rate
    EXPECT_EQ(HotWaterFlowRate, state->dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxHeat);
    EXPECT_EQ(ChilledWaterFlowRate, state->dataLowTempRadSys->HydrRadSys(RadSysNum).WaterVolFlowMaxCool);
    // Test autosized tube length
    EXPECT_EQ(TubeLengthDes, state->dataLowTempRadSys->HydrRadSys(RadSysNum).TubeLength);
}

TEST_F(LowTempRadiantSystemTest, SimulateCapacityPerFloorAreaError)
{

    bool ErrorsFound = false;
    std::string const idf_objects = delimited_string({

        "  Building,",
        "    NONE,                    !- Name",
        "    0.0000000E+00,           !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    3.9999999E-02,           !- Loads Convergence Tolerance Value",
        "    0.4000000,               !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullInteriorAndExterior, !- Solar Distribution",
        "    25,                      !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",

        "  ScheduleTypeLimits,Fraction, 0.0 , 1.0 ,CONTINUOUS;",
        "  ScheduleTypeLimits,Temperature,-60,200,CONTINUOUS;",

        "  Zone,",
        "    West Zone,               !- Name",
        "    0.0000000E+00,           !- Direction of Relative North {deg}",
        "    0.0000000E+00,           !- X Origin {m}",
        "    0.0000000E+00,           !- Y Origin {m}",
        "    0.0000000E+00,           !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.5,                     !- Ceiling Height {m}",
        "    autocalculate;              !- Volume {m3}",

        "  ZoneHVAC:EquipmentConnections,",
        "    West Zone,               !- Zone Name",
        "    Zone1Equipment,          !- Zone Conditioning Equipment List Name",
        "    Zone1Inlets,             !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    Zone 1 Node,             !- Zone Air Node Name",
        "    Zone 1 Outlet Node;      !- Zone Return Air Node Name",

        "  ZoneHVAC:EquipmentList,",
        "    Zone1Equipment,          !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:LowTemperatureRadiant:VariableFlow,  !- Zone Equipment 1 Object Type",
        "    West Zone Radiant Floor, !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "  ZoneHVAC:LowTemperatureRadiant:VariableFlow,",
        "    West Zone Radiant Floor, !- Name",
        "    West Zone Radiant Floor Design, !- Design Object Name",
        "    RadiantSysAvailSched,    !- Availability Schedule Name",
        "    West Zone,               !- Zone Name",
        "    Zn001:Flr001,            !- Surface Name or Radiant Surface Group Name",
        "    autosize,                !- Hydronic Tubing Length {m}",
        "    ,                        !- Heating Design Capacity {W}",
        "    autosize,                !- Maximum Hot Water Flow {m3/s}",
        "    West Zone Radiant Water Inlet Node,  !- Heating Water Inlet Node Name",
        "    West Zone Radiant Water Outlet Node, !- Heating Water Outlet Node Name",
        "    ,                        !- Cooling Design Capacity {W}",
        "    autosize,                !- Maximum Cold Water Flow {m3/s}",
        "    Zone 1 Cooling Water Inlet Node,     !- Cooling Water Inlet Node Name",
        "    Zone 1 Cooling Water Outlet Node,    !- Cooling Water Outlet Node Name",
        "    ,                        !- Number of Circuits",
        "    ;                        !- Circuit Length {m}",

        "  ZoneHVAC:LowTemperatureRadiant:VariableFlow:Design,",
        "    West Zone Radiant Floor Design, !- Name",
        "    ConvectionOnly,          !- Fluid to Radiant Surface Heat Transfer Model",
        "    0.012,                   !- Hydronic Tubing Inside Diameter {m}",
        "    0.016,                   !- Hydronic Tubing Outside Diameter {m}",
        "    0.35,                    !- Hydronic Tubing Conductivity {W/m-K}",
        "    MeanAirTemperature,      !- Temperature Control Type",
        "    HalfFlowPower,           !- Setpoint Type",
        "    FractionOfAutosizedHeatingCapacity,  !- Heating Design Capacity Method",
        "    ,                        !- Heating Design Capacity Per Floor Area {W/m2}",
        "    0.9,                     !- Fraction of Autosized Heating Design Capacity",
        "    2.0,                     !- Heating Control Throttling Range {deltaC}",
        "    Radiant Heating Setpoints,  !- Heating Control Temperature Schedule Name",
        "    CapacityPerFloorArea,    !- Cooling Design Capacity Method",
        "    0.0,                        !- Cooling Design Capacity Per Floor Area {W/m2}",
        "    ,                        !- Fraction of Autosized Cooling Design Capacity",
        "    2.0,                     !- Cooling Control Throttling Range {deltaC}",
        "    Radiant Cooling Setpoints,           !- Cooling Control Temperature Schedule Name",
        "    ,                        !- Condensation Control Type",
        "    ;                        !- Condensation Control Dewpoint Offset {C}",

        "  Site:GroundTemperature:BuildingSurface,20.03,20.03,20.13,20.30,20.43,20.52,20.62,20.77,20.78,20.55,20.44,20.20;",

        "  BuildingSurface:Detailed,",
        "    Zn001:Flr001,            !- Name",
        "    Floor,                   !- Surface Type",
        "    Slab Floor with Radiant, !- Construction Name",
        "    West Zone,               !- Zone Name",
        "    ,                        !- Space Name",
        "    Adiabatic,                  !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    1.000000,                !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0, 0.0, 0.0,           !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0, 6.0,0.0,       !- X,Y,Z ==> Vertex 2 {m}",
        "    6.0,6.0,0.0,   !- X,Y,Z ==> Vertex 3 {m}",
        "    6.0, 0.0, 0.0;      !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall001,            !- Name",
        "    Wall,                   !- Surface Type",
        "    Slab Floor with Radiant, !- Construction Name",
        "    West Zone,               !- Zone Name",
        "    ,                        !- Space Name",
        "    Adiabatic,                  !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0, 0.0, 6.0,           !- X,Y,Z ==> Vertex 1 {m}",
        "    6.0, 0.0, 6.0,       !- X,Y,Z ==> Vertex 2 {m}",
        "    6.0, 0.0,0.0,   !- X,Y,Z ==> Vertex 3 {m}",
        "    0.0, 0.0, 0.0;      !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall002,            !- Name",
        "    Wall,                   !- Surface Type",
        "    Slab, !- Construction Name",
        "    West Zone,               !- Zone Name",
        "    ,                        !- Space Name",
        "    Adiabatic,                  !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0, 6.0, 0.0,           !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0, 6.0, 6.0,       !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0, 0.0, 6.0,   !- X,Y,Z ==> Vertex 3 {m}",
        "    0.0, 0.0, 0.0;      !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall003,            !- Name",
        "    Wall,                   !- Surface Type",
        "    Slab, !- Construction Name",
        "    West Zone,               !- Zone Name",
        "    ,                        !- Space Name",
        "    Adiabatic,                  !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    6.0, 6.0, 0.0,           !- X,Y,Z ==> Vertex 1 {m}",
        "    6.0, 6.0, 6.0,       !- X,Y,Z ==> Vertex 2 {m}",
        "    6.0, 0.0, 6.0,   !- X,Y,Z ==> Vertex 3 {m}",
        "    6.0, 0.0, 0.0;      !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall004,            !- Name",
        "    Wall,                   !- Surface Type",
        "    Slab, !- Construction Name",
        "    West Zone,               !- Zone Name",
        "    ,                        !- Space Name",
        "    Adiabatic,                  !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0, 6.0, 0.0,           !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0, 6.0, 6.0,       !- X,Y,Z ==> Vertex 2 {m}",
        "    6.0, 6.0, 6.0,   !- X,Y,Z ==> Vertex 3 {m}",
        "    6.0, 6.0, 0.0;      !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:C001,            !- Name",
        "    CEILING,                   !- Surface Type",
        "    Slab, !- Construction Name",
        "    West Zone,               !- Zone Name",
        "    ,                        !- Space Name",
        "    Adiabatic,                  !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0, 0.0, 6.0,           !- X,Y,Z ==> Vertex 1 {m}",
        "    6.0, 0.0, 6.0,       !- X,Y,Z ==> Vertex 2 {m}",
        "    6.0, 6.0, 6.0,   !- X,Y,Z ==> Vertex 3 {m}",
        "    0.0, 6.0, 6.0;      !- X,Y,Z ==> Vertex 4 {m}",

        "  ConstructionProperty:InternalHeatSource,",
        "    Radiant Source,          !- Name",
        "    Slab Floor with Radiant, !- Construction Name",
        "    3,                       !- Source Present After Layer Number",
        "    3,                       !- Temperature Calculation Requested After Layer Number",
        "    1,                       !- Dimensions for the CTF Calculation",
        "    0.1524,                  !- Tube Spacing {m}",
        "    0.0;                     !- Two-Dimensional Position of Interior Temperature Calculation Request",

        "  Construction,",
        "    Slab Floor with Radiant, !- Name",
        "    CONCRETE - DRIED SAND AND GRAVEL 4 IN,  !- Outside Layer",
        "    INS - EXPANDED EXT POLYSTYRENE R12 2 IN,  !- Layer 2",
        "    GYP1,                    !- Layer 3",
        "    GYP2,                    !- Layer 4",
        "    FINISH FLOORING - TILE 1 / 16 IN;  !- Layer 5",

        "Construction,",
        "Slab,              !- Name",
        "CONCRETE - DRIED SAND AND GRAVEL 4 IN,                    !- Outside Layer",
        "GYP1,                    !- Layer 2",
        "FINISH FLOORING - TILE 1 / 16 IN;                    !- Layer 3",

        "  Material,",
        "    CONCRETE - DRIED SAND AND GRAVEL 4 IN,  !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1000000,               !- Thickness {m}",
        "    1.290000,                !- Conductivity {W/m-K}",
        "    2242.580,                !- Density {kg/m3}",
        "    830.00000,               !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6000000,               !- Solar Absorptance",
        "    0.6000000;               !- Visible Absorptance",

        "  Material,",
        "    INS - EXPANDED EXT POLYSTYRENE R12 2 IN,  !- Name",
        "    Rough,                   !- Roughness",
        "    5.0000001E-02,           !- Thickness {m}",
        "    2.0000000E-02,           !- Conductivity {W/m-K}",
        "    56.06000,                !- Density {kg/m3}",
        "    1210.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.5000000,               !- Solar Absorptance",
        "    0.5000000;               !- Visible Absorptance",

        "  Material,",
        "    GYP1,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    1.2700000E-02,           !- Thickness {m}",
        "    7.8450000E-01,           !- Conductivity {W/m-K}",
        "    1842.1221,               !- Density {kg/m3}",
        "    988.000,                 !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.5000000,               !- Solar Absorptance",
        "    0.5000000;               !- Visible Absorptance",

        "  Material,",
        "    GYP2,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    1.9050000E-02,           !- Thickness {m}",
        "    7.8450000E-01,           !- Conductivity {W/m-K}",
        "    1842.1221,               !- Density {kg/m3}",
        "    988.000,                 !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.5000000,               !- Solar Absorptance",
        "    0.5000000;               !- Visible Absorptance",

        "  Material,",
        "    FINISH FLOORING - TILE 1 / 16 IN,  !- Name",
        "    Smooth,                  !- Roughness",
        "    1.6000000E-03,           !- Thickness {m}",
        "    0.1700000,               !- Conductivity {W/m-K}",
        "    1922.210,                !- Density {kg/m3}",
        "    1250.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.5000000,               !- Solar Absorptance",
        "    0.5000000;               !- Visible Absorptance",

        "  Schedule:Compact,",
        "    RADIANTSYSAVAILSCHED,    !- Name",
        "    FRACTION,                !- Schedule Type Limits Name",
        "    Through: 3/31,           !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,1.00,       !- Field 3",
        "    Through: 9/30,           !- Field 5",
        "    For: Alldays,            !- Field 6",
        "    Until: 24:00,0.00,       !- Field 7",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 24:00,1.00;       !- Field 11",

        "  Schedule:Compact,",
        "    HW LOOP TEMP SCHEDULE,   !- Name",
        "    TEMPERATURE,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,60.00;      !- Field 3",

        "  Schedule:Compact,",
        "    RADIANT HEATING SETPOINTS,  !- Name",
        "    TEMPERATURE,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 7:00,12.00,       !- Field 3",
        "    Until: 17:00,17.00,      !- Field 5",
        "    Until: 24:00,12.00;      !- Field 7",

        "  Sizing:Plant,",
        "    Hot Water Loop,          !- Plant or Condenser Loop Name",
        "    heating,                 !- Loop Type",
        "    60.,                     !- Design Loop Exit Temperature {C}",
        "    10;                      !- Loop Design Temperature Difference {deltaC}",

        "  PlantLoop,",
        "    Hot Water Loop,          !- Name",
        "    Water,                   !- Fluid Type",
        "    ,                        !- User Defined Fluid Type",
        "    Hot Loop Operation,      !- Plant Equipment Operation Scheme Name",
        "    HW Supply Outlet Node,   !- Loop Temperature Setpoint Node Name",
        "    100,                     !- Maximum Loop Temperature {C}",
        "    10,                      !- Minimum Loop Temperature {C}",
        "    0.0043,                  !- Maximum Loop Flow Rate {m3/s}",
        "    0,                       !- Minimum Loop Flow Rate {m3/s}",
        "    autocalculate,           !- Plant Loop Volume {m3}",
        "    HW Supply Inlet Node,    !- Plant Side Inlet Node Name",
        "    HW Supply Outlet Node,   !- Plant Side Outlet Node Name",
        "    Heating Supply Side Branches,  !- Plant Side Branch List Name",
        "    Heating Supply Side Connectors,  !- Plant Side Connector List Name",
        "    HW Demand Inlet Node,    !- Demand Side Inlet Node Name",
        "    HW Demand Outlet Node,   !- Demand Side Outlet Node Name",
        "    Heating Demand Side Branches,  !- Demand Side Branch List Name",
        "    Heating Demand Side Connectors,  !- Demand Side Connector List Name",
        "    Optimal,                 !- Load Distribution Scheme",
        "    ,                        !- Availability Manager List Name",
        "    ,                        !- Plant Loop Demand Calculation Scheme",
        "    ,                        !- Common Pipe Simulation",
        "    ,                        !- Pressure Simulation Type",
        "    2.0;                     !- Loop Circulation Time {minutes}",

        "  SetpointManager:Scheduled,",
        "    Hot Water Loop Setpoint Manager,  !- Name",
        "    Temperature,             !- Control Variable",
        "    HW Loop Temp Schedule,   !- Schedule Name",
        "    Hot Water Loop Setpoint Node List;  !- Setpoint Node or NodeList Name",

        "  NodeList,",
        "    Hot Water Loop Setpoint Node List,  !- Name",
        "    HW Supply Outlet Node;   !- Node 1 Name",

        "  BranchList,",
        "    Heating Supply Side Branches,  !- Name",
        "    Heating Supply Inlet Branch,  !- Branch 1 Name",
        "    Heating Purchased Hot Water Branch,  !- Branch 2 Name",
        "    Heating Supply Bypass Branch,  !- Branch 3 Name",
        "    Heating Supply Outlet Branch;  !- Branch 4 Name",

        "  ConnectorList,",
        "    Heating Supply Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    Heating Supply Splitter, !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    Heating Supply Mixer;    !- Connector 2 Name",

        "  Branch,",
        "    Heating Supply Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:VariableSpeed,      !- Component 1 Object Type",
        "    HW Circ Pump,            !- Component 1 Name",
        "    HW Supply Inlet Node,    !- Component 1 Inlet Node Name",
        "    HW Pump Outlet Node;     !- Component 1 Outlet Node Name",

        "  Branch,",
        "    Heating Purchased Hot Water Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    DistrictHeating:Water,         !- Component 1 Object Type",
        "    Purchased Heating,       !- Component 1 Name",
        "    Purchased Heat Inlet Node,  !- Component 1 Inlet Node Name",
        "    Purchased Heat Outlet Node;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    Heating Supply Bypass Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Heating Supply Side Bypass,  !- Component 1 Name",
        "    Heating Supply Bypass Inlet Node,  !- Component 1 Inlet Node Name",
        "    Heating Supply Bypass Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Heating Supply Side Bypass,  !- Name",
        "    Heating Supply Bypass Inlet Node,  !- Inlet Node Name",
        "    Heating Supply Bypass Outlet Node;  !- Outlet Node Name",

        "  Branch,",
        "    Heating Supply Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Heating Supply Outlet,   !- Component 1 Name",
        "    Heating Supply Exit Pipe Inlet Node,  !- Component 1 Inlet Node Name",
        "    HW Supply Outlet Node;   !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Heating Supply Outlet,   !- Name",
        "    Heating Supply Exit Pipe Inlet Node,  !- Inlet Node Name",
        "    HW Supply Outlet Node;   !- Outlet Node Name",

        "  BranchList,",
        "    Heating Demand Side Branches,  !- Name",
        "    Reheat Inlet Branch,     !- Branch 1 Name",
        "    Zone 1 Radiant Branch,   !- Branch 5 Name",
        "    Reheat Bypass Branch,    !- Branch 8 Name",
        "    Reheat Outlet Branch;    !- Branch 9 Name",

        "  ConnectorList,",
        "    Heating Demand Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    Reheat Splitter,         !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    Reheat Mixer;            !- Connector 2 Name",

        "  Branch,",
        "    Reheat Inlet Branch,     !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Reheat Inlet Pipe,       !- Component 1 Name",
        "    HW Demand Inlet Node,    !- Component 1 Inlet Node Name",
        "    HW Demand Entrance Pipe Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Reheat Inlet Pipe,       !- Name",
        "    HW Demand Inlet Node,    !- Inlet Node Name",
        "    HW Demand Entrance Pipe Outlet Node;  !- Outlet Node Name",

        "  Branch,",
        "    Reheat Outlet Branch,    !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Reheat Outlet Pipe,      !- Component 1 Name",
        "    HW Demand Exit Pipe Inlet Node,  !- Component 1 Inlet Node Name",
        "    HW Demand Outlet Node;   !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Reheat Outlet Pipe,      !- Name",
        "    HW Demand Exit Pipe Inlet Node,  !- Inlet Node Name",
        "    HW Demand Outlet Node;   !- Outlet Node Name",

        "  Branch,",
        "    Zone 1 Radiant Branch,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    ZoneHVAC:LowTemperatureRadiant:VariableFlow,  !- Component 1 Object Type",
        "    West Zone Radiant Floor, !- Component 1 Name",
        "    West Zone Radiant Water Inlet Node,  !- Component 1 Inlet Node Name",
        "    West Zone Radiant Water Outlet Node;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    Reheat Bypass Branch,    !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Reheat Bypass,           !- Component 1 Name",
        "    Reheat Bypass Inlet Node,!- Component 1 Inlet Node Name",
        "    Reheat Bypass Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Reheat Bypass,           !- Name",
        "    Reheat Bypass Inlet Node,!- Inlet Node Name",
        "    Reheat Bypass Outlet Node;  !- Outlet Node Name",

        "  Connector:Splitter,",
        "    Reheat Splitter,         !- Name",
        "    Reheat Inlet Branch,     !- Inlet Branch Name",
        "    Zone 1 Radiant Branch,   !- Outlet Branch 4 Name",
        "    Reheat Bypass Branch;    !- Outlet Branch 7 Name",

        "  Connector:Mixer,",
        "    Reheat Mixer,            !- Name",
        "    Reheat Outlet Branch,    !- Outlet Branch Name",
        "    Zone 1 Radiant Branch,   !- Inlet Branch 4 Name",
        "    Reheat Bypass Branch;    !- Inlet Branch 7 Name",

        "  Connector:Splitter,",
        "    Heating Supply Splitter, !- Name",
        "    Heating Supply Inlet Branch,  !- Inlet Branch Name",
        "    Heating Purchased Hot Water Branch,  !- Outlet Branch 1 Name",
        "    Heating Supply Bypass Branch;  !- Outlet Branch 2 Name",

        "  Connector:Mixer,",
        "    Heating Supply Mixer,    !- Name",
        "    Heating Supply Outlet Branch,  !- Outlet Branch Name",
        "    Heating Purchased Hot Water Branch,  !- Inlet Branch 1 Name",
        "    Heating Supply Bypass Branch;  !- Inlet Branch 2 Name",

        "  PlantEquipmentOperationSchemes,",
        "    Hot Loop Operation,      !- Name",
        "    PlantEquipmentOperation:HeatingLoad,  !- Control Scheme 1 Object Type",
        "    Purchased Only,          !- Control Scheme 1 Name",
        "    ON;                      !- Control Scheme 1 Schedule Name",

        "  PlantEquipmentOperation:HeatingLoad,",
        "    Purchased Only,          !- Name",
        "    0,                       !- Load Range 1 Lower Limit {W}",
        "    1000000,                 !- Load Range 1 Upper Limit {W}",
        "    heating plant;           !- Range 1 Equipment List Name",

        "  PlantEquipmentList,",
        "    heating plant,           !- Name",
        "    DistrictHeating:Water,         !- Equipment 1 Object Type",
        "    Purchased Heating;       !- Equipment 1 Name",

        "  Pump:VariableSpeed,",
        "    HW Circ Pump,            !- Name",
        "    HW Supply Inlet Node,    !- Inlet Node Name",
        "    HW Pump Outlet Node,     !- Outlet Node Name",
        "    .0043,                   !- Rated Flow Rate {m3/s}",
        "    300000,                  !- Rated Pump Head {Pa}",
        "    2000,                    !- Rated Power Consumption {W}",
        "    .87,                     !- Motor Efficiency",
        "    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "    1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 3 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 4 of the Part Load Performance Curve",
        "    0,                       !- Minimum Flow Rate {m3/s}",
        "    INTERMITTENT;            !- Pump Control Type",

        "  DistrictHeating:Water,",
        "    Purchased Heating,       !- Name",
        "    Purchased Heat Inlet Node,  !- Hot Water Inlet Node Name",
        "    Purchased Heat Outlet Node,  !- Hot Water Outlet Node Name",
        "    1000000;                 !- Nominal Capacity {W}",

        "  Sizing:Plant,",
        "    Chilled Water Loop,      !- Plant or Condenser Loop Name",
        "    cooling,                 !- Loop Type",
        "    6.7,                     !- Design Loop Exit Temperature {C}",
        "    2;                       !- Loop Design Temperature Difference {deltaC}",

        "  PlantLoop,",
        "    Chilled Water Loop,      !- Name",
        "    Water,                   !- Fluid Type",
        "    ,                        !- User Defined Fluid Type",
        "    CW Loop Operation,       !- Plant Equipment Operation Scheme Name",
        "    CW Supply Outlet Node,   !- Loop Temperature Setpoint Node Name",
        "    98,                      !- Maximum Loop Temperature {C}",
        "    1,                       !- Minimum Loop Temperature {C}",
        "    0.0011,                  !- Maximum Loop Flow Rate {m3/s}",
        "    0,                       !- Minimum Loop Flow Rate {m3/s}",
        "    autocalculate,           !- Plant Loop Volume {m3}",
        "    CW Supply Inlet Node,    !- Plant Side Inlet Node Name",
        "    CW Supply Outlet Node,   !- Plant Side Outlet Node Name",
        "    Cooling Supply Side Branches,  !- Plant Side Branch List Name",
        "    Cooling Supply Side Connectors,  !- Plant Side Connector List Name",
        "    CW Demand Inlet Node,    !- Demand Side Inlet Node Name",
        "    CW Demand Outlet Node,   !- Demand Side Outlet Node Name",
        "    Cooling Demand Side Branches,  !- Demand Side Branch List Name",
        "    Cooling Demand Side Connectors,  !- Demand Side Connector List Name",
        "    Optimal,                 !- Load Distribution Scheme",
        "    ,                        !- Availability Manager List Name",
        "    ,                        !- Plant Loop Demand Calculation Scheme",
        "    ,                        !- Common Pipe Simulation",
        "    ,                        !- Pressure Simulation Type",
        "    2.0;                     !- Loop Circulation Time {minutes}",

        "  SetpointManager:Scheduled,",
        "    Chilled Water Loop Setpoint Manager,  !- Name",
        "    Temperature,             !- Control Variable",
        "    CW Loop Temp Schedule,   !- Schedule Name",
        "    Chilled Water Loop Setpoint Node List;  !- Setpoint Node or NodeList Name",

        "	Schedule:Compact,",
        "	 CW LOOP TEMP SCHEDULE, !- Name",
        "	 TEMPERATURE,           !- Schedule Type Limits Name",
        "	 Through: 12/31,        !- Field 1",
        "	 For: Alldays,          !- Field 2",
        "	 Until: 24:00, 10.0;    !- Field 3",

        "  NodeList,",
        "    Chilled Water Loop Setpoint Node List,  !- Name",
        "    CW Supply Outlet Node;   !- Node 1 Name",

        "  BranchList,",
        "    Cooling Supply Side Branches,  !- Name",
        "    CW Pump Branch,          !- Branch 1 Name",
        "    Purchased Cooling Branch,!- Branch 4 Name",
        "    Supply Bypass Branch,    !- Branch 5 Name",
        "    Cooling Supply Outlet;   !- Branch 6 Name",

        "  BranchList,",
        "    Cooling Demand Side Branches,  !- Name",
        "    Cooling Demand Inlet,    !- Branch 1 Name",
        "    Zone 1 Cooling Branch,   !- Branch 2 Name",
        "    Demand Bypass Branch,    !- Branch 3 Name",
        "    Cooling Demand Outlet;   !- Branch 4 Name",

        "  ConnectorList,",
        "    Cooling Supply Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    CW Loop Splitter,        !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    CW Loop Mixer;           !- Connector 2 Name",

        "  ConnectorList,",
        "    Cooling Demand Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    CW Demand Splitter,      !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    CW Demand Mixer;         !- Connector 2 Name",

        "  Branch,",
        "    Cooling Demand Inlet,    !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Demand Side Inlet Pipe,  !- Component 1 Name",
        "    CW Demand Inlet Node,    !- Component 1 Inlet Node Name",
        "    CW Demand Entrance Pipe Outlet Node;  !- Component 1 Outlet Node Name",

        "  Schedule:Compact,",
        "    RADIANT COOLING SETPOINTS,  !- Name",
        "    TEMPERATURE,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,26.0;       !- Field 3",

        "  Pipe:Adiabatic,",
        "    Demand Side Inlet Pipe,  !- Name",
        "    CW Demand Inlet Node,    !- Inlet Node Name",
        "    CW Demand Entrance Pipe Outlet Node;  !- Outlet Node Name",

        "  Branch,",
        "    Zone 1 Cooling Branch,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    ZoneHVAC:LowTemperatureRadiant:VariableFlow,  !- Component 1 Object Type",
        "    West Zone Radiant Floor,   !- Component 1 Name",
        "    Zone 1 Cooling Water Inlet Node,  !- Component 1 Inlet Node Name",
        "    Zone 1 Cooling Water Outlet Node; !- Component 1 Outlet Node Name",

        "  Branch,",
        "    Demand Bypass Branch,    !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Demand Side Bypass,      !- Component 1 Name",
        "    CW Demand Bypass Inlet Node,  !- Component 1 Inlet Node Name",
        "    CW Demand Bypass Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Demand Side Bypass,      !- Name",
        "    CW Demand Bypass Inlet Node,  !- Inlet Node Name",
        "    CW Demand Bypass Outlet Node;  !- Outlet Node Name",

        "  Branch,",
        "    Cooling Demand Outlet,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    CW Demand Side Outlet Pipe,  !- Component 1 Name",
        "    CW Demand Exit Pipe Inlet Node,  !- Component 1 Inlet Node Name",
        "    CW Demand Outlet Node;   !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    CW Demand Side Outlet Pipe,  !- Name",
        "    CW Demand Exit Pipe Inlet Node,  !- Inlet Node Name",
        "    CW Demand Outlet Node;   !- Outlet Node Name",

        "  Branch,",
        "    Cooling Supply Outlet,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Supply Side Outlet Pipe, !- Component 1 Name",
        "    Supply Side Exit Pipe Inlet Node,  !- Component 1 Inlet Node Name",
        "    CW Supply Outlet Node;   !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Supply Side Outlet Pipe, !- Name",
        "    Supply Side Exit Pipe Inlet Node,  !- Inlet Node Name",
        "    CW Supply Outlet Node;   !- Outlet Node Name",

        "  Branch,",
        "    CW Pump Branch,          !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:VariableSpeed,      !- Component 1 Object Type",
        "    Circ Pump,               !- Component 1 Name",
        "    CW Supply Inlet Node,    !- Component 1 Inlet Node Name",
        "    CW Pump Outlet Node;     !- Component 1 Outlet Node Name",

        "  Branch,",
        "    Purchased Cooling Branch,!- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    DistrictCooling,         !- Component 1 Object Type",
        "    Purchased Cooling,       !- Component 1 Name",
        "    Purchased Cooling Inlet Node,  !- Component 1 Inlet Node Name",
        "    Purchased Cooling Outlet Node; !- Component 1 Outlet Node Name",

        "  Branch,",
        "    Supply Bypass Branch,    !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Supply Side Bypass,      !- Component 1 Name",
        "    CW Supply Bypass Inlet Node,  !- Component 1 Inlet Node Name",
        "    CW Supply Bypass Outlet Node; !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Supply Side Bypass,      !- Name",
        "    CW Supply Bypass Inlet Node,  !- Inlet Node Name",
        "    CW Supply Bypass Outlet Node; !- Outlet Node Name",

        "  Connector:Splitter,",
        "    CW Loop Splitter,        !- Name",
        "    CW Pump Branch,          !- Inlet Branch Name",
        "    Purchased Cooling Branch,!- Outlet Branch 3 Name",
        "    Supply Bypass Branch;    !- Outlet Branch 4 Name",

        "  Connector:Mixer,",
        "    CW Loop Mixer,           !- Name",
        "    Cooling Supply Outlet,   !- Outlet Branch Name",
        "    Purchased Cooling Branch,!- Inlet Branch 3 Name",
        "    Supply Bypass Branch;    !- Inlet Branch 4 Name",

        "  Connector:Splitter,",
        "    CW Demand Splitter,      !- Name",
        "    Cooling Demand Inlet,    !- Inlet Branch Name",
        "    Demand Bypass Branch,    !- Outlet Branch 1 Name",
        "    Zone 1 Cooling Branch;   !- Outlet Branch 2 Name",

        "  Connector:Mixer,",
        "    CW Demand Mixer,         !- Name",
        "    Cooling Demand Outlet,   !- Outlet Branch Name",
        "    Zone 1 Cooling Branch,   !- Inlet Branch 1 Name",
        "    Demand Bypass Branch;    !- Inlet Branch 2 Name",

        "  PlantEquipmentOperationSchemes,",
        "    CW Loop Operation,       !- Name",
        "    PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type",
        "    Always Operation,        !- Control Scheme 1 Name",
        "    Always;                  !- Control Scheme 1 Schedule Name",

        "  Schedule:Compact,",
        "    Always,                  !- Name",
        "    FRACTION,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "  PlantEquipmentOperation:CoolingLoad,",
        "    Always Operation,        !- Name",
        "    0,                       !- Load Range 1 Lower Limit {W}",
        "    70000,                   !- Load Range 1 Upper Limit {W}",
        "    Purchased Only;          !- Range 3 Equipment List Name",

        "  PlantEquipmentList,",
        "    Purchased Only,          !- Name",
        "    DistrictCooling,         !- Equipment 1 Object Type",
        "    Purchased Cooling;       !- Equipment 1 Name",

        "  DistrictCooling,",
        "    Purchased Cooling,             !- Name",
        "    Purchased Cooling Inlet Node,  !- Chilled Water Inlet Node Name",
        "    Purchased Cooling Outlet Node, !- Chilled Water Outlet Node Name",
        "    680000;                        !- Nominal Capacity {W}",

        "  Pump:VariableSpeed,",
        "    Circ Pump,               !- Name",
        "    CW Supply Inlet Node,    !- Inlet Node Name",
        "    CW Pump Outlet Node,     !- Outlet Node Name",
        "    .0011,                   !- Rated Flow Rate {m3/s}",
        "    300000,                  !- Rated Pump Head {Pa}",
        "    500,                     !- Rated Power Consumption {W}",
        "    .87,                     !- Motor Efficiency",
        "    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "    1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 3 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 4 of the Part Load Performance Curve",
        "    0,                       !- Minimum Flow Rate {m3/s}",
        "    INTERMITTENT;            !- Pump Control Type",

    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetProjectControlData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    EXPECT_EQ("WEST ZONE", state->dataHeatBal->Zone(1).Name);

    HeatBalanceManager::SetPreConstructionInputParameters(*state);
    Material::GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    GetPlantSizingInput(*state);
    GetPlantLoopData(*state);
    GetPlantInput(*state);
    SetupInitialPlantCallingOrder(*state);
    SetupBranchControlTypes(*state);
    state->dataSurface->WorldCoordSystem = true;
    GetSurfaceListsInputs(*state);
    ErrorsFound = false;
    SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    std::string const error_string =
        delimited_string({"   ** Severe  ** ZoneHVAC:LowTemperatureRadiant:VariableFlow:Design = WEST ZONE RADIANT FLOOR DESIGN",
                          "   **   ~~~   ** Input for Cooling Design Capacity Method = CAPACITYPERFLOORAREA",
                          "   **   ~~~   ** Illegal Cooling Design Capacity Per Floor Area = 0.0000000",
                          "   **  Fatal  ** GetLowTempRadiantSystem: Errors found in input. Preceding conditions cause termination.",
                          "   ...Summary of Errors that led to program termination:",
                          "   ..... Reference severe error count=1",
                          "   ..... Last severe error=ZoneHVAC:LowTemperatureRadiant:VariableFlow:Design = WEST ZONE RADIANT FLOOR DESIGN"});
    EXPECT_ANY_THROW(GetLowTempRadiantSystem(*state));

    compare_err_stream(error_string, true);
}

TEST_F(LowTempRadiantSystemTest, InitLowTempRadiantSystem)
{

    bool InitErrorFound;

    RadSysNum = 1;
    systemType = SystemType::ConstantFlow;
    state->dataLowTempRadSys->NumOfCFloLowTempRadSys = 1;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).NumOfSurfaces = 0;
    state->dataLowTempRadSys->TotalNumOfRadSystems = 0;
    state->dataGlobal->BeginEnvrnFlag = false;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterInNode = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode = 0;
    state->dataGlobal->BeginTimeStepFlag = false;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).volFlowSched = nullptr;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).EMSOverrideOnWaterMdot = false;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterMassFlowRate = 1.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HotDesignWaterMassFlowRate = 2.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdDesignWaterMassFlowRate = 3.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).CWPlantLoc.loopNum = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HWPlantLoc.loopNum = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax = 1.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).NomPumpHead = 1.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).NomPowerUse = 1.0;

    DesignObjectNum = 1;
    state->dataLowTempRadSys->NumOfCFloLowTempRadSysDes = 1;
    state->dataLowTempRadSys->CflowRadiantSysDesign.allocate(state->dataLowTempRadSys->NumOfCFloLowTempRadSysDes);
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).DesignObjectPtr = 1;
    state->dataLowTempRadSys->CflowRadiantSysDesign(DesignObjectNum).MotorEffic = 1.2;

    state->dataLowTempRadSys->CFloRadSys(RadSysNum).CoolingSystem = true;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HeatingSystem = false;
    InitLowTempRadiantSystem(*state, false, RadSysNum, systemType, InitErrorFound);
    EXPECT_EQ(3.0, state->dataLowTempRadSys->CFloRadSys(RadSysNum).ChWaterMassFlowRate);
    EXPECT_EQ(0.0, state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterMassFlowRate);

    state->dataLowTempRadSys->CFloRadSys(RadSysNum).CoolingSystem = false;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HeatingSystem = true;
    InitLowTempRadiantSystem(*state, false, RadSysNum, systemType, InitErrorFound);
    EXPECT_EQ(2.0, state->dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterMassFlowRate);
    EXPECT_EQ(0.0, state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterMassFlowRate);
}

TEST_F(LowTempRadiantSystemTest, InitLowTempRadiantSystemCFloPump)
{

    bool InitErrorFound;
    Real64 actualEfficiencyPercentage;

    // Test 1: with autosize for max flow, nothing should happen
    state->dataLowTempRadSys->clear_state();
    RadSysNum = 1;
    systemType = SystemType::ConstantFlow;
    state->dataLowTempRadSys->NumOfCFloLowTempRadSys = 1;
    state->dataLowTempRadSys->CFloRadSys.allocate(state->dataLowTempRadSys->NumOfCFloLowTempRadSys);
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).NumOfSurfaces = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).Name = "NoNameRadSys";
    state->dataLowTempRadSys->TotalNumOfRadSystems = 0;
    state->dataGlobal->BeginEnvrnFlag = false;
    state->dataZoneEquip->ZoneEquipInputsFilled = false;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterInNode = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode = 0;
    state->dataGlobal->BeginTimeStepFlag = false;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).volFlowSched = nullptr;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).EMSOverrideOnWaterMdot = false;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterMassFlowRate = 1.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HotDesignWaterMassFlowRate = 2.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdDesignWaterMassFlowRate = 3.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).CWPlantLoc.loopNum = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HWPlantLoc.loopNum = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).NomPumpHead = 1.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).NomPowerUse = 1.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).PumpEffic = 0.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).CoolingSystem = false;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HeatingSystem = false;

    DesignObjectNum = 1;
    state->dataLowTempRadSys->NumOfCFloLowTempRadSysDes = 1;
    state->dataLowTempRadSys->CflowRadiantSysDesign.allocate(state->dataLowTempRadSys->NumOfCFloLowTempRadSysDes);
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).DesignObjectPtr = 1;
    state->dataLowTempRadSys->CflowRadiantSysDesign(DesignObjectNum).MotorEffic = 1.0;

    state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax = AutoSize;
    InitLowTempRadiantSystem(*state, false, RadSysNum, systemType, InitErrorFound);
    EXPECT_EQ(state->dataLowTempRadSys->CFloRadSys(RadSysNum).PumpEffic, 0.0);
    EXPECT_EQ(InitErrorFound, false);

    // Test 2: pump efficiency below 50%
    state->dataLowTempRadSys->clear_state();
    RadSysNum = 1;
    systemType = SystemType::ConstantFlow;
    state->dataLowTempRadSys->NumOfCFloLowTempRadSys = 1;
    state->dataLowTempRadSys->CFloRadSys.allocate(state->dataLowTempRadSys->NumOfCFloLowTempRadSys);
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).NumOfSurfaces = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).Name = "NoNameRadSys";
    state->dataLowTempRadSys->TotalNumOfRadSystems = 0;
    state->dataGlobal->BeginEnvrnFlag = false;
    state->dataZoneEquip->ZoneEquipInputsFilled = false;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterInNode = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode = 0;
    state->dataGlobal->BeginTimeStepFlag = false;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).volFlowSched = nullptr;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).EMSOverrideOnWaterMdot = false;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterMassFlowRate = 1.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HotDesignWaterMassFlowRate = 2.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdDesignWaterMassFlowRate = 3.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).CWPlantLoc.loopNum = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HWPlantLoc.loopNum = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).NomPumpHead = 1.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).NomPowerUse = 1.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).PumpEffic = 0.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).CoolingSystem = false;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HeatingSystem = false;

    DesignObjectNum = 1;
    state->dataLowTempRadSys->NumOfCFloLowTempRadSysDes = 1;
    state->dataLowTempRadSys->CflowRadiantSysDesign.allocate(state->dataLowTempRadSys->NumOfCFloLowTempRadSysDes);
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).DesignObjectPtr = 1;
    state->dataLowTempRadSys->CflowRadiantSysDesign(DesignObjectNum).MotorEffic = 1.0;

    state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax =
        0.4; // because of how other parameters are set, this value is equal to the pump efficiency
    InitLowTempRadiantSystem(*state, false, RadSysNum, systemType, InitErrorFound);
    actualEfficiencyPercentage = state->dataLowTempRadSys->CFloRadSys(RadSysNum).PumpEffic * 100.0;
    std::string const error_string02 =
        delimited_string({format("   ** Warning ** Check input.  Calc Pump Efficiency={:.5R}% which is less than 50%, for pump in radiant system {}",
                                 actualEfficiencyPercentage,
                                 state->dataLowTempRadSys->CFloRadSys(RadSysNum).Name)});
    EXPECT_EQ(state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax, state->dataLowTempRadSys->CFloRadSys(RadSysNum).PumpEffic);
    EXPECT_TRUE(compare_err_stream(error_string02, true));
    EXPECT_EQ(InitErrorFound, false);

    // Test 3: pump efficiency between 95% and 100%
    state->dataLowTempRadSys->clear_state();
    RadSysNum = 1;
    systemType = SystemType::ConstantFlow;
    state->dataLowTempRadSys->NumOfCFloLowTempRadSys = 1;
    state->dataLowTempRadSys->CFloRadSys.allocate(state->dataLowTempRadSys->NumOfCFloLowTempRadSys);
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).NumOfSurfaces = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).Name = "NoNameRadSys";
    state->dataLowTempRadSys->TotalNumOfRadSystems = 0;
    state->dataGlobal->BeginEnvrnFlag = false;
    state->dataZoneEquip->ZoneEquipInputsFilled = false;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterInNode = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode = 0;
    state->dataGlobal->BeginTimeStepFlag = false;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).volFlowSched = nullptr;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).EMSOverrideOnWaterMdot = false;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterMassFlowRate = 1.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HotDesignWaterMassFlowRate = 2.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdDesignWaterMassFlowRate = 3.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).CWPlantLoc.loopNum = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HWPlantLoc.loopNum = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).NomPumpHead = 1.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).NomPowerUse = 1.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).PumpEffic = 0.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).CoolingSystem = false;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HeatingSystem = false;

    DesignObjectNum = 1;
    state->dataLowTempRadSys->NumOfCFloLowTempRadSysDes = 1;
    state->dataLowTempRadSys->CflowRadiantSysDesign.allocate(state->dataLowTempRadSys->NumOfCFloLowTempRadSysDes);
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).DesignObjectPtr = 1;
    state->dataLowTempRadSys->CflowRadiantSysDesign(DesignObjectNum).MotorEffic = 1.0;

    state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax =
        0.98; // because of how other parameters are set, this value is equal to the pump efficiency
    InitLowTempRadiantSystem(*state, false, RadSysNum, systemType, InitErrorFound);
    actualEfficiencyPercentage = state->dataLowTempRadSys->CFloRadSys(RadSysNum).PumpEffic * 100.0;
    std::string const error_string03 =
        delimited_string({format("   ** Warning ** Check input.  Calc Pump Efficiency={:.5R}% is approaching 100%, for pump in radiant system {}",
                                 actualEfficiencyPercentage,
                                 state->dataLowTempRadSys->CFloRadSys(RadSysNum).Name)});
    EXPECT_EQ(state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax, state->dataLowTempRadSys->CFloRadSys(RadSysNum).PumpEffic);
    EXPECT_TRUE(compare_err_stream(error_string03, true));
    EXPECT_EQ(InitErrorFound, false);

    // Test 4: pump efficiency over 100%
    state->dataLowTempRadSys->clear_state();
    RadSysNum = 1;
    systemType = SystemType::ConstantFlow;
    state->dataLowTempRadSys->NumOfCFloLowTempRadSys = 1;
    state->dataLowTempRadSys->CFloRadSys.allocate(state->dataLowTempRadSys->NumOfCFloLowTempRadSys);
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).NumOfSurfaces = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).Name = "NoNameRadSys";
    state->dataLowTempRadSys->TotalNumOfRadSystems = 0;
    state->dataGlobal->BeginEnvrnFlag = false;
    state->dataZoneEquip->ZoneEquipInputsFilled = false;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterInNode = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode = 0;
    state->dataGlobal->BeginTimeStepFlag = false;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).volFlowSched = nullptr;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).EMSOverrideOnWaterMdot = false;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterMassFlowRate = 1.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HotDesignWaterMassFlowRate = 2.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdDesignWaterMassFlowRate = 3.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).CWPlantLoc.loopNum = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HWPlantLoc.loopNum = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).NomPumpHead = 1.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).NomPowerUse = 1.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).PumpEffic = 0.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).CoolingSystem = false;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HeatingSystem = false;

    DesignObjectNum = 1;
    state->dataLowTempRadSys->NumOfCFloLowTempRadSysDes = 1;
    state->dataLowTempRadSys->CflowRadiantSysDesign.allocate(state->dataLowTempRadSys->NumOfCFloLowTempRadSysDes);
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).DesignObjectPtr = 1;
    state->dataLowTempRadSys->CflowRadiantSysDesign(DesignObjectNum).MotorEffic = 1.0;

    state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax =
        1.23; // because of how other parameters are set, this value is equal to the pump efficiency
    InitLowTempRadiantSystem(*state, false, RadSysNum, systemType, InitErrorFound);
    actualEfficiencyPercentage = state->dataLowTempRadSys->CFloRadSys(RadSysNum).PumpEffic * 100.0;
    std::string const error_string04 = delimited_string(
        {format("   ** Severe  ** Check input.  Calc Pump Efficiency={:.5R}% which is bigger than 100%, for pump in radiant system {}",
                actualEfficiencyPercentage,
                state->dataLowTempRadSys->CFloRadSys(RadSysNum).Name)});
    EXPECT_EQ(state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax, state->dataLowTempRadSys->CFloRadSys(RadSysNum).PumpEffic);
    EXPECT_TRUE(compare_err_stream(error_string04, true));
    EXPECT_EQ(InitErrorFound, true);
}

TEST_F(LowTempRadiantSystemTest, LowTempElecRadSurfaceGroupTest)
{

    int RadSysNum(1);

    std::string const idf_objects = delimited_string({

        "  ZoneHVAC:LowTemperatureRadiant:Electric,",
        "    West Zone Radiant Floor, !- Name",
        "    RadiantSysAvailSched,    !- Availability Schedule Name",
        "    West Zone,               !- Zone Name",
        "    West Zone Surface Group, !- Surface Name or Radiant Surface Group Name",
        "    heatingdesigncapacity,   !- Heating Design Capacity Method",
        "    100,                     !- Heating Design Capacity{ W }",
        "    ,                        !- Heating Design Capacity Per Floor Area{ W/m2 }",
        "    1.0,                     !- Fraction of Autosized Heating Design Capacity",
        "    MeanAirTemperature,      !- Temperature Control Type",
        "    HalfFlowPower,           !- Setpoint Type",
        "    2.0,                     !- Heating Throttling Range {deltaC}",
        "    Radiant Heating Setpoints;  !- Heating Control Temperature Schedule Name",

        "  ZoneHVAC:LowTemperatureRadiant:Electric,",
        "    East Zone Radiant Floor, !- Name",
        "    RadiantSysAvailSched,    !- Availability Schedule Name",
        "    East Zone,               !- Zone Name",
        "    East Zone Surface Group, !- Surface Name or Radiant Surface Group Name",
        "    heatingdesigncapacity,   !- Heating Design Capacity Method",
        "    100,                     !- Heating Design Capacity{ W }",
        "    ,                        !- Heating Design Capacity Per Floor Area{ W/m2 }",
        "    1.0,                     !- Fraction of Autosized Heating Design Capacity",
        "    MeanAirTemperature,      !- Temperature Control Type",
        "    HalfFlowPower,           !- Setpoint Type",
        "    2.0,                     !- Heating Throttling Range {deltaC}",
        "    Radiant Heating Setpoints;  !- Heating Control Temperature Schedule Name",

        "  ZoneHVAC:LowTemperatureRadiant:SurfaceGroup,",
        "    East Zone Surface Group, !- Name",
        "    Zn002:Flr001,             !- Surface 1 Name",
        "     0.5,                     !- Flow Fraction for Surface 1",
        "    Zn002:Flr002,             !- Surface 2 Name",
        "     0.5;                     !- Flow Fraction for Surface 2",

        "  ZoneHVAC:LowTemperatureRadiant:SurfaceGroup,",
        "    West Zone Surface Group, !- Name",
        "    Zn001:Flr001,             !- Surface 1 Name",
        "     0.5,                     !- Flow Fraction for Surface 1",
        "    Zn001:Flr002,             !- Surface 2 Name",
        "     0.5;                     !- Flow Fraction for Surface 2",

        "  Schedule:Compact,",
        "    RADIANTSYSAVAILSCHED,    !- Name",
        "    FRACTION,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,1.00;       !- Field 3",

        "  Schedule:Compact,",
        "    Radiant Heating Setpoints,   !- Name",
        "    TEMPERATURE,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,20.0;       !- Field 3",

    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataHeatBal->Zone.allocate(2);
    state->dataHeatBal->Zone(1).Name = "WEST ZONE";
    state->dataHeatBal->Zone(2).Name = "EAST ZONE";

    state->dataSurface->TotSurfaces = 4;
    state->dataSurface->Surface.allocate(4);
    state->dataSurface->Surface(1).Name = "ZN001:FLR001";
    state->dataSurface->Surface(1).ZoneName = "WEST ZONE";
    state->dataSurface->Surface(1).Zone = 1;
    state->dataSurface->Surface(1).Construction = 1;
    state->dataSurface->Surface(2).Name = "ZN001:FLR002";
    state->dataSurface->Surface(2).ZoneName = "WEST ZONE";
    state->dataSurface->Surface(2).Zone = 1;
    state->dataSurface->Surface(2).Construction = 1;
    state->dataSurface->Surface(3).Name = "ZN002:FLR001";
    state->dataSurface->Surface(3).ZoneName = "EAST ZONE";
    state->dataSurface->Surface(3).Zone = 2;
    state->dataSurface->Surface(3).Construction = 1;
    state->dataSurface->Surface(4).Name = "ZN002:FLR002";
    state->dataSurface->Surface(4).ZoneName = "EAST ZONE";
    state->dataSurface->Surface(4).Zone = 2;
    state->dataSurface->Surface(4).Construction = 1;
    state->dataConstruction->Construct.allocate(1);
    state->dataConstruction->Construct(1).SourceSinkPresent = true;
    state->dataSurface->SurfIsRadSurfOrVentSlabOrPool.allocate(state->dataSurface->TotSurfaces);
    state->dataSurface->SurfIsRadSurfOrVentSlabOrPool = false;

    GetLowTempRadiantSystem(*state);
    EXPECT_EQ(2, state->dataLowTempRadSys->NumOfElecLowTempRadSys);
    EXPECT_EQ("WEST ZONE RADIANT FLOOR", state->dataLowTempRadSys->RadSysTypes(RadSysNum).Name);
    EXPECT_EQ("EAST ZONE RADIANT FLOOR", state->dataLowTempRadSys->RadSysTypes(RadSysNum + 1).Name);
    EXPECT_ENUM_EQ(SystemType::Electric, state->dataLowTempRadSys->RadSysTypes(RadSysNum).systemType);
    EXPECT_EQ(state->dataLowTempRadSys->ElecRadSys(1).ZoneName, "WEST ZONE");
    EXPECT_EQ(state->dataLowTempRadSys->ElecRadSys(1).SurfListName, "WEST ZONE SURFACE GROUP");
    // the 2nd surface list group holds data for 1st elec rad sys (#5958)
    EXPECT_EQ(state->dataSurfLists->SurfList(2).Name, "WEST ZONE SURFACE GROUP");
    EXPECT_EQ(state->dataLowTempRadSys->ElecRadSys(1).NumOfSurfaces, 2);
    // surface ptr's are not set correctly when elec rad sys "index" (e.g., state->dataLowTempRadSys->ElecRadSys(N)) is not the same as surface group
    // "index" #5958 fixes this issue
    EXPECT_EQ(state->dataLowTempRadSys->ElecRadSys(1).SurfacePtr(1), 1);
    EXPECT_EQ(state->dataLowTempRadSys->ElecRadSys(1).SurfacePtr(2), 2);
}

TEST_F(LowTempRadiantSystemTest, CalcLowTempCFloRadiantSystem_OperationMode)
{
    // # ZoneHVAC:LowTemperatureRadiant:VariableFlow array bounds error #5905

    Real64 Load;

    RadSysNum = 1;
    state->dataLowTempRadSys->clear_state();
    systemType = SystemType::ConstantFlow;
    state->dataLowTempRadSys->NumOfCFloLowTempRadSys = 1;
    state->dataLowTempRadSys->CFloRadSys.allocate(state->dataLowTempRadSys->NumOfCFloLowTempRadSys);
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).SurfacePtr.allocate(1);
    state->dataLowTempRadSys->NumOfCFloLowTempRadSysDes = 1;
    state->dataLowTempRadSys->CflowRadiantSysDesign.allocate(state->dataLowTempRadSys->NumOfCFloLowTempRadSysDes);

    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 21.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).NumOfSurfaces = 0;
    state->dataLowTempRadSys->TotalNumOfRadSystems = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ZonePtr = 1;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).availSched = Sched::AddScheduleConstant(*state, "Avail");
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).controlType = CtrlType::MAT;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).hotCtrlHiTempSched = Sched::AddScheduleConstant(*state, "Hi Temp");
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).coldCtrlLoTempSched = Sched::AddScheduleConstant(*state, "Lo Temp");
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).SurfacePtr(1) = 1;

    state->dataLowTempRadSys->CFloRadSys(RadSysNum).availSched->currentVal = 1;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).hotCtrlHiTempSched->currentVal = 22.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).coldCtrlLoTempSched->currentVal = 25.0;

    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterInNode = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).volFlowSched = nullptr;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).EMSOverrideOnWaterMdot = false;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterMassFlowRate = 1.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HotDesignWaterMassFlowRate = 2.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdDesignWaterMassFlowRate = 3.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).CWPlantLoc.loopNum = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HWPlantLoc.loopNum = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax = 1.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).NomPumpHead = 1.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).NomPowerUse = 1.0;
    DesignObjectNum = 1;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).DesignObjectPtr = 1;
    state->dataLowTempRadSys->CflowRadiantSysDesign(DesignObjectNum).MotorEffic = 1.2;

    // heating
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).CoolingSystem = true;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HeatingSystem = false;
    Load = 1000.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).calculateLowTemperatureRadiantSystem(*state, Load);
    EXPECT_ENUM_EQ(OpMode::None, state->dataLowTempRadSys->CFloRadSys(RadSysNum).opMode);

    // Cooling
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).CoolingSystem = false;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HeatingSystem = true;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 26.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).calculateLowTemperatureRadiantSystem(*state, Load);
    EXPECT_ENUM_EQ(OpMode::None, state->dataLowTempRadSys->CFloRadSys(RadSysNum).opMode);
}

TEST_F(LowTempRadiantSystemTest, CalcLowTempHydrRadiantSystem_OperationMode)
{
    // # ZoneHVAC:LowTemperatureRadiant:VariableFlow array bounds error #5905

    Real64 Load;

    RadSysNum = 1;
    state->dataLowTempRadSys->clear_state();

    //	SystemType = SystemType::ConstantFlowSystem;
    state->dataLowTempRadSys->NumOfHydrLowTempRadSys = 1;
    state->dataLowTempRadSys->HydrRadSys.allocate(state->dataLowTempRadSys->NumOfHydrLowTempRadSys);
    state->dataLowTempRadSys->NumOfHydrLowTempRadSysDes = 1;
    state->dataLowTempRadSys->HydronicRadiantSysDesign.allocate(state->dataLowTempRadSys->NumOfHydrLowTempRadSys);
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).SurfacePtr.allocate(1);
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).SurfacePtr(1) = 1;

    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 21.0;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).NumOfSurfaces = 0;
    state->dataLowTempRadSys->TotalNumOfRadSystems = 0;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).ZonePtr = 1;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).availSched = Sched::AddScheduleConstant(*state, "Avail");
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).availSched->currentVal = 1;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).controlType = CtrlType::MAT;

    DesignObjectNum = 1;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).DesignObjectPtr = 1;
    state->dataLowTempRadSys->HydronicRadiantSysDesign(DesignObjectNum).heatSetptSched = Sched::AddScheduleConstant(*state, "Hot Setpt");
    state->dataLowTempRadSys->HydronicRadiantSysDesign(DesignObjectNum).heatSetptSched->currentVal = 22.0;
    state->dataLowTempRadSys->HydronicRadiantSysDesign(DesignObjectNum).coolSetptSched = Sched::AddScheduleConstant(*state, "Cold Setpt");
    state->dataLowTempRadSys->HydronicRadiantSysDesign(DesignObjectNum).coolSetptSched->currentVal = 25.0;

    state->dataLowTempRadSys->HydrRadSys(RadSysNum).HotWaterInNode = 0;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).ColdWaterInNode = 0;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).EMSOverrideOnWaterMdot = false;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).WaterMassFlowRate = 1.0;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).CWPlantLoc.loopNum = 0;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).HWPlantLoc.loopNum = 0;

    // heating
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).opMode = OpMode::None;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).CoolingSystem = true;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).HeatingSystem = false;
    Load = 1000.0;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).calculateLowTemperatureRadiantSystem(*state, Load);
    EXPECT_ENUM_EQ(OpMode::None, state->dataLowTempRadSys->HydrRadSys(RadSysNum).opMode);

    // Cooling
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).CoolingSystem = false;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).HeatingSystem = true;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 26.0;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).calculateLowTemperatureRadiantSystem(*state, Load);
    EXPECT_EQ(OpMode::None, state->dataLowTempRadSys->HydrRadSys(RadSysNum).opMode);
}

TEST_F(LowTempRadiantSystemTest, SizeRadSysTubeLengthTest)
{
    // # Low Temperature Radiant System (variable and constant flow) autosizing tube length issue #6202
    Real64 FuncCalc;
    SystemType RadSysType;

    RadSysNum = 1;
    state->dataLowTempRadSys->clear_state();

    state->dataLowTempRadSys->HydrRadSys.allocate(3);
    state->dataLowTempRadSys->CFloRadSys.allocate(3);

    state->dataLowTempRadSys->HydrRadSys(1).NumOfSurfaces = 1;
    state->dataLowTempRadSys->HydrRadSys(1).SurfacePtr.allocate(1);
    state->dataLowTempRadSys->HydrRadSys(1).SurfacePtr(1) = 1;
    state->dataLowTempRadSys->HydrRadSys(2).NumOfSurfaces = 2;
    state->dataLowTempRadSys->HydrRadSys(2).SurfacePtr.allocate(2);
    state->dataLowTempRadSys->HydrRadSys(2).SurfacePtr(1) = 1;
    state->dataLowTempRadSys->HydrRadSys(2).SurfacePtr(2) = 2;
    state->dataLowTempRadSys->HydrRadSys(3).NumOfSurfaces = 1;
    state->dataLowTempRadSys->HydrRadSys(3).SurfacePtr.allocate(1);
    state->dataLowTempRadSys->HydrRadSys(3).SurfacePtr(1) = 3;

    state->dataLowTempRadSys->CFloRadSys(1).NumOfSurfaces = 1;
    state->dataLowTempRadSys->CFloRadSys(1).SurfacePtr.allocate(1);
    state->dataLowTempRadSys->CFloRadSys(1).SurfacePtr(1) = 1;
    state->dataLowTempRadSys->CFloRadSys(2).NumOfSurfaces = 2;
    state->dataLowTempRadSys->CFloRadSys(2).SurfacePtr.allocate(2);
    state->dataLowTempRadSys->CFloRadSys(2).SurfacePtr(1) = 1;
    state->dataLowTempRadSys->CFloRadSys(2).SurfacePtr(2) = 2;
    state->dataLowTempRadSys->CFloRadSys(3).NumOfSurfaces = 1;
    state->dataLowTempRadSys->CFloRadSys(3).SurfacePtr.allocate(1);
    state->dataLowTempRadSys->CFloRadSys(3).SurfacePtr(1) = 3;

    state->dataSurface->Surface.allocate(3);
    state->dataSurface->Surface(1).Construction = 1;
    state->dataSurface->Surface(1).Area = 100.0;
    state->dataSurface->Surface(2).Construction = 2;
    state->dataSurface->Surface(2).Area = 200.0;
    state->dataSurface->Surface(3).Construction = 3;
    state->dataSurface->Surface(3).Area = 300.0;

    state->dataConstruction->Construct.allocate(3);
    state->dataConstruction->Construct(1).ThicknessPerpend = 0.05;
    state->dataConstruction->Construct(2).ThicknessPerpend = 0.125;

    // Test 1: Hydronic radiant system 1 (one surface)
    RadSysType = SystemType::Hydronic;
    RadSysNum = 1;
    FuncCalc = state->dataLowTempRadSys->HydrRadSys(RadSysNum).sizeRadiantSystemTubeLength(*state);
    EXPECT_NEAR(FuncCalc, 1000.0, 0.1);

    // Test 2: Hydronic radiant system 2 (two surfaces)
    RadSysType = SystemType::Hydronic;
    RadSysNum = 2;
    FuncCalc = state->dataLowTempRadSys->HydrRadSys(RadSysNum).sizeRadiantSystemTubeLength(*state);
    EXPECT_NEAR(FuncCalc, 1800.0, 0.1);

    // Test 3: Constant flow radiant system 1 (one surface)
    RadSysType = SystemType::ConstantFlow;
    RadSysNum = 1;
    FuncCalc = state->dataLowTempRadSys->CFloRadSys(RadSysNum).sizeRadiantSystemTubeLength(*state);
    EXPECT_NEAR(FuncCalc, 1000.0, 0.1);

    // Test 4: Constant flow radiant system 2 (two surfaces)
    RadSysType = SystemType::ConstantFlow;
    RadSysNum = 2;
    FuncCalc = state->dataLowTempRadSys->CFloRadSys(RadSysNum).sizeRadiantSystemTubeLength(*state);
    EXPECT_NEAR(FuncCalc, 1800.0, 0.1);

    // Test 5: Hydronic radiant system 3 (thickness out of range, low side)
    RadSysType = SystemType::Hydronic;
    RadSysNum = 3;
    state->dataConstruction->Construct(3).ThicknessPerpend = 0.004;
    FuncCalc = state->dataLowTempRadSys->HydrRadSys(RadSysNum).sizeRadiantSystemTubeLength(*state);
    EXPECT_NEAR(FuncCalc, 2000.0, 0.1);

    // Test 6: Hydronic radiant system 3 (thickness out of range, high side)
    RadSysType = SystemType::Hydronic;
    RadSysNum = 3;
    state->dataConstruction->Construct(3).ThicknessPerpend = 0.6;
    FuncCalc = state->dataLowTempRadSys->HydrRadSys(RadSysNum).sizeRadiantSystemTubeLength(*state);
    EXPECT_NEAR(FuncCalc, 2000.0, 0.1);

    // Test 7: Constant flow radiant system 3 (thickness out of range, low side)
    RadSysType = SystemType::ConstantFlow;
    RadSysNum = 3;
    state->dataConstruction->Construct(3).ThicknessPerpend = 0.004;
    FuncCalc = state->dataLowTempRadSys->CFloRadSys(RadSysNum).sizeRadiantSystemTubeLength(*state);
    EXPECT_NEAR(FuncCalc, 2000.0, 0.1);

    // Test 8: Constant flow radiant system 3 (thickness out of range, high side)
    RadSysType = SystemType::ConstantFlow;
    RadSysNum = 3;
    state->dataConstruction->Construct(3).ThicknessPerpend = 0.6;
    FuncCalc = state->dataLowTempRadSys->CFloRadSys(RadSysNum).sizeRadiantSystemTubeLength(*state);
    EXPECT_NEAR(FuncCalc, 2000.0, 0.1);
}

TEST_F(LowTempRadiantSystemTest, LowTempRadConFlowSystemAutoSizeTempTest)
{
    state->init_state(*state);
    Real64 Density;
    Real64 Cp;

    systemType = SystemType::ConstantFlow;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).Name = "LowTempConstantFlow";
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ZonePtr = 1;
    state->dataLowTempRadSys->HydronicRadiantSysNumericFields(RadSysNum).FieldNames(2) = "Rated Flow Rate";
    state->dataLowTempRadSys->HydronicRadiantSysNumericFields(RadSysNum).FieldNames(3) = "Total length of pipe embedded in surface";

    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterInNode = 1;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterOutNode = 2;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HWPlantLoc.loopNum = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterInNode;

    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode = 3;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterOutNode = 4;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).CWPlantLoc.loopNum = 2;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode;

    state->dataLowTempRadSys->CFloRadSys(RadSysNum).SurfacePtr.allocate(1);
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).SurfacePtr(1) = 1;
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).Construction = 1;
    state->dataSurface->Surface(1).Area = 150.0;
    state->dataConstruction->Construct.allocate(1);
    state->dataConstruction->Construct(1).ThicknessPerpend = 0.075;

    // Hydronic - Hot water volume flow rate autosize
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterOutNode = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax = AutoSize;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesHeatLoad = 1000.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesCoolLoad = 1000.0;

    // hot water volume flow rate sizing calculation
    Density = state->dataPlnt->PlantLoop(state->dataLowTempRadSys->CFloRadSys(RadSysNum).HWPlantLoc.loopNum)
                  .glycol->getDensity(*state, 60.0, "LowTempRadConFlowSystemAutoSizeTempTest");
    Cp = state->dataPlnt->PlantLoop(state->dataLowTempRadSys->CFloRadSys(RadSysNum).HWPlantLoc.loopNum)
             .glycol->getSpecificHeat(*state, 60.0, "LowTempRadConFlowSystemAutoSizeTempTest");
    Real64 HeatingLoad = state->dataSize->FinalZoneSizing(1).NonAirSysDesHeatLoad;
    Real64 DesHotWaterVolFlowRate = HeatingLoad / (state->dataSize->PlantSizData(1).DeltaT * Density * Cp);

    SizeLowTempRadiantSystem(*state, RadSysNum, systemType);
    // check hot water design flow rate calculated here and autosized flow are identical
    EXPECT_DOUBLE_EQ(DesHotWaterVolFlowRate, state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax);

    // Hydronic - cold water volume flow rate autosize
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterInNode = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HotWaterOutNode = 0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterInNode = 3;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).ColdWaterOutNode = 4;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax = AutoSize;

    // chilled water volume flow rate sizing calculation
    Density = state->dataPlnt->PlantLoop(state->dataLowTempRadSys->CFloRadSys(RadSysNum).CWPlantLoc.loopNum)
                  .glycol->getDensity(*state, 5.05, "LowTempRadConFlowSystemAutoSizeTempTest");
    Cp = state->dataPlnt->PlantLoop(state->dataLowTempRadSys->CFloRadSys(RadSysNum).CWPlantLoc.loopNum)
             .glycol->getSpecificHeat(*state, 5.05, "LowTempRadConFlowSystemAutoSizeTempTest");
    Real64 CoolingLoad = state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).NonAirSysDesCoolLoad;
    Real64 DesChilledWaterVolFlowRate = CoolingLoad / (state->dataSize->PlantSizData(2).DeltaT * Density * Cp);

    SizeLowTempRadiantSystem(*state, RadSysNum, systemType);
    // check chilled water design flow rate calculated here and autosized flow are identical
    EXPECT_DOUBLE_EQ(DesChilledWaterVolFlowRate, state->dataLowTempRadSys->CFloRadSys(RadSysNum).WaterVolFlowMax);
}

TEST_F(LowTempRadiantSystemTest, LowTempRadCalcRadSysHXEffectTermTest)
{
    state->init_state(*state);
    int RadSysNum;
    SystemType RadSysType;
    Real64 Temperature;
    Real64 WaterMassFlow;
    Real64 FlowFraction;
    Real64 NumCircs;
    Real64 TubeLength;
    Real64 TubeDiameter;
    Real64 HXEffectFuncResult;
    int SurfNum;

    // Set values of items that will stay constant for all calls to HX Effectiveness function
    RadSysNum = 1;
    SurfNum = 1;
    WaterMassFlow = 0.1;
    FlowFraction = 1.0;
    NumCircs = 1;
    TubeLength = 10.0;
    TubeDiameter = 0.05;
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).TubeLength = TubeLength;

    DesignObjectNum = 1;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).DesignObjectPtr = 1;
    state->dataLowTempRadSys->HydronicRadiantSysDesign.allocate(1);
    state->dataLowTempRadSys->HydronicRadiantSysDesign(DesignObjectNum).TubeDiameterInner = TubeDiameter;
    state->dataLowTempRadSys->HydronicRadiantSysDesign(DesignObjectNum).FluidToSlabHeatTransfer = FluidToSlabHeatTransferType::ConvectionOnly;

    state->dataLowTempRadSys->CFloRadSys(RadSysNum).TubeLength = TubeLength;
    DesignObjectNum = 1;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).DesignObjectPtr = 1;
    state->dataLowTempRadSys->CflowRadiantSysDesign.allocate(1);
    state->dataLowTempRadSys->CflowRadiantSysDesign(DesignObjectNum).TubeDiameterInner = TubeDiameter;
    state->dataLowTempRadSys->CflowRadiantSysDesign(DesignObjectNum).FluidToSlabHeatTransfer = FluidToSlabHeatTransferType::ConvectionOnly;

    // Test 1: Heating for Hydronic System
    HXEffectFuncResult = 0.0;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).opMode = OpMode::Heat;
    RadSysType = SystemType::Hydronic;
    Temperature = 10.0;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).HWPlantLoc.loopNum = 1;
    HXEffectFuncResult =
        state->dataLowTempRadSys->HydrRadSys(RadSysNum).calculateHXEffectivenessTerm(*state,
                                                                                     SurfNum,
                                                                                     Temperature,
                                                                                     WaterMassFlow,
                                                                                     FlowFraction,
                                                                                     NumCircs,
                                                                                     state->dataLowTempRadSys->HydrRadSys(RadSysNum).DesignObjectPtr,
                                                                                     RadSysType);
    EXPECT_NEAR(HXEffectFuncResult, 62.344, 0.001);

    // Test 2: Cooling for Hydronic System
    HXEffectFuncResult = 0.0;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).opMode = OpMode::Cool;
    RadSysType = SystemType::Hydronic;
    Temperature = 10.0;
    state->dataLowTempRadSys->HydrRadSys(RadSysNum).CWPlantLoc.loopNum = 1;
    HXEffectFuncResult =
        state->dataLowTempRadSys->HydrRadSys(RadSysNum).calculateHXEffectivenessTerm(*state,
                                                                                     SurfNum,
                                                                                     Temperature,
                                                                                     WaterMassFlow,
                                                                                     FlowFraction,
                                                                                     NumCircs,
                                                                                     state->dataLowTempRadSys->HydrRadSys(RadSysNum).DesignObjectPtr,
                                                                                     RadSysType);
    EXPECT_NEAR(HXEffectFuncResult, 62.344, 0.001);

    // Test 3: Heating for Constant Flow System
    HXEffectFuncResult = 0.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).opMode = OpMode::Heat;
    RadSysType = SystemType::ConstantFlow;
    Temperature = 10.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).HWPlantLoc.loopNum = 1;
    HXEffectFuncResult =
        state->dataLowTempRadSys->CFloRadSys(RadSysNum).calculateHXEffectivenessTerm(*state,
                                                                                     SurfNum,
                                                                                     Temperature,
                                                                                     WaterMassFlow,
                                                                                     FlowFraction,
                                                                                     NumCircs,
                                                                                     state->dataLowTempRadSys->CFloRadSys(RadSysNum).DesignObjectPtr,
                                                                                     RadSysType);
    EXPECT_NEAR(HXEffectFuncResult, 62.344, 0.001);

    // Test 4: Cooling for Constant Flow System
    HXEffectFuncResult = 0.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).opMode = OpMode::Cool;
    RadSysType = SystemType::ConstantFlow;
    Temperature = 10.0;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).CWPlantLoc.loopNum = 1;
    HXEffectFuncResult =
        state->dataLowTempRadSys->CFloRadSys(RadSysNum).calculateHXEffectivenessTerm(*state,
                                                                                     SurfNum,
                                                                                     Temperature,
                                                                                     WaterMassFlow,
                                                                                     FlowFraction,
                                                                                     NumCircs,
                                                                                     state->dataLowTempRadSys->CFloRadSys(RadSysNum).DesignObjectPtr,
                                                                                     RadSysType);
    EXPECT_NEAR(HXEffectFuncResult, 62.344, 0.001);
}

#ifdef GET_OUT
// No point in unit testing getEnumValue
TEST_F(LowTempRadiantSystemTest, processRadiantSystemControlInputTest)
{
    static constexpr std::string_view meanAirTemperature("MeanAirTemperature");
    static constexpr std::string_view meanRadiantTemperature("MeanRadiantTemperature");
    static constexpr std::string_view operativeTemperature("OperativeTemperature");
    static constexpr std::string_view outsideAirDryBulbTemperature("OutdoorDryBulbTemperature");
    static constexpr std::string_view outsideAirWetBulbTemperature("OutdoorWetBulbTemperature");
    static constexpr std::string_view surfaceFaceTemperature("SurfaceFaceTemperature");
    static constexpr std::string_view surfaceInteriorTemperature("SurfaceInteriorTemperature");

    std::string inputFunction;
    std::string textField2Pass("FieldName");
    LowTempRadiantControlTypes expectedResult;
    LowTempRadiantControlTypes actualFunctionAnswer;

    state->dataLowTempRadSys->HydrRadSys.allocate(1);
    state->dataLowTempRadSys->HydrRadSys(1).Name = "VariableFlowRadSys1";
    state->dataLowTempRadSys->CFloRadSys.allocate(1);
    state->dataLowTempRadSys->CFloRadSys(1).Name = "ConstantFlowRadSys1";
    state->dataLowTempRadSys->ElecRadSys.allocate(1);
    state->dataLowTempRadSys->ElecRadSys(1).Name = "ElectricRadSys1";

    // Test 1: MAT test (done for all three types of systems)
    inputFunction = meanAirTemperature;
    expectedResult = LowTempRadiantControlTypes::MATControl;
    actualFunctionAnswer = LowTempRadiantControlTypes::MRTControl; // reset
    actualFunctionAnswer =
        state->dataLowTempRadSys->HydrRadSys(1).processRadiantSystemControlInput(*state, inputFunction, textField2Pass, SystemType::HydronicSystem);
    EXPECT_ENUM_EQ(expectedResult, actualFunctionAnswer);
    actualFunctionAnswer = LowTempRadiantControlTypes::MRTControl; // reset
    actualFunctionAnswer = state->dataLowTempRadSys->CFloRadSys(1).processRadiantSystemControlInput(
        *state, inputFunction, textField2Pass, SystemType::ConstantFlowSystem);
    EXPECT_ENUM_EQ(expectedResult, actualFunctionAnswer);
    actualFunctionAnswer = LowTempRadiantControlTypes::MRTControl; // reset
    actualFunctionAnswer =
        state->dataLowTempRadSys->ElecRadSys(1).processRadiantSystemControlInput(*state, inputFunction, textField2Pass, SystemType::Electric);
    EXPECT_ENUM_EQ(expectedResult, actualFunctionAnswer);

    // Test 2: MRT test (done for all three types of systems)
    inputFunction = meanRadiantTemperature;
    expectedResult = LowTempRadiantControlTypes::MRTControl;
    actualFunctionAnswer = LowTempRadiantControlTypes::MATControl; // reset
    actualFunctionAnswer =
        state->dataLowTempRadSys->HydrRadSys(1).processRadiantSystemControlInput(*state, inputFunction, textField2Pass, SystemType::HydronicSystem);
    EXPECT_ENUM_EQ(expectedResult, actualFunctionAnswer);
    actualFunctionAnswer = LowTempRadiantControlTypes::MATControl; // reset
    actualFunctionAnswer = state->dataLowTempRadSys->CFloRadSys(1).processRadiantSystemControlInput(
        *state, inputFunction, textField2Pass, SystemType::ConstantFlowSystem);
    EXPECT_ENUM_EQ(expectedResult, actualFunctionAnswer);
    actualFunctionAnswer = LowTempRadiantControlTypes::MATControl; // reset
    actualFunctionAnswer =
        state->dataLowTempRadSys->ElecRadSys(1).processRadiantSystemControlInput(*state, inputFunction, textField2Pass, SystemType::Electric);
    EXPECT_ENUM_EQ(expectedResult, actualFunctionAnswer);

    // Test 3: Operative Temperature test (done for all three types of systems)
    inputFunction = operativeTemperature;
    expectedResult = LowTempRadiantControlTypes::OperativeControl;
    actualFunctionAnswer = LowTempRadiantControlTypes::MATControl; // reset
    actualFunctionAnswer =
        state->dataLowTempRadSys->HydrRadSys(1).processRadiantSystemControlInput(*state, inputFunction, textField2Pass, SystemType::HydronicSystem);
    EXPECT_ENUM_EQ(expectedResult, actualFunctionAnswer);
    actualFunctionAnswer = LowTempRadiantControlTypes::MATControl; // reset
    actualFunctionAnswer = state->dataLowTempRadSys->CFloRadSys(1).processRadiantSystemControlInput(
        *state, inputFunction, textField2Pass, SystemType::ConstantFlowSystem);
    EXPECT_ENUM_EQ(expectedResult, actualFunctionAnswer);
    actualFunctionAnswer = LowTempRadiantControlTypes::MATControl; // reset
    actualFunctionAnswer =
        state->dataLowTempRadSys->ElecRadSys(1).processRadiantSystemControlInput(*state, inputFunction, textField2Pass, SystemType::Electric);
    EXPECT_ENUM_EQ(expectedResult, actualFunctionAnswer);

    // Test 4: Outside Dry-Bulb Temperature test (done for all three types of systems)
    inputFunction = outsideAirDryBulbTemperature;
    expectedResult = LowTempRadiantControlTypes::ODBControl;
    actualFunctionAnswer = LowTempRadiantControlTypes::MATControl; // reset
    actualFunctionAnswer =
        state->dataLowTempRadSys->HydrRadSys(1).processRadiantSystemControlInput(*state, inputFunction, textField2Pass, SystemType::HydronicSystem);
    EXPECT_ENUM_EQ(expectedResult, actualFunctionAnswer);
    actualFunctionAnswer = LowTempRadiantControlTypes::MATControl; // reset
    actualFunctionAnswer = state->dataLowTempRadSys->CFloRadSys(1).processRadiantSystemControlInput(
        *state, inputFunction, textField2Pass, SystemType::ConstantFlowSystem);
    EXPECT_ENUM_EQ(expectedResult, actualFunctionAnswer);
    actualFunctionAnswer = LowTempRadiantControlTypes::MATControl; // reset
    actualFunctionAnswer =
        state->dataLowTempRadSys->ElecRadSys(1).processRadiantSystemControlInput(*state, inputFunction, textField2Pass, SystemType::Electric);
    EXPECT_ENUM_EQ(expectedResult, actualFunctionAnswer);

    // Test 5: Outside Wet-Bulb Temperature test (done for all three types of systems)
    inputFunction = outsideAirWetBulbTemperature;
    expectedResult = LowTempRadiantControlTypes::OWBControl;
    actualFunctionAnswer = LowTempRadiantControlTypes::MATControl; // reset
    actualFunctionAnswer =
        state->dataLowTempRadSys->HydrRadSys(1).processRadiantSystemControlInput(*state, inputFunction, textField2Pass, SystemType::HydronicSystem);
    EXPECT_ENUM_EQ(expectedResult, actualFunctionAnswer);
    actualFunctionAnswer = LowTempRadiantControlTypes::MATControl; // reset
    actualFunctionAnswer = state->dataLowTempRadSys->CFloRadSys(1).processRadiantSystemControlInput(
        *state, inputFunction, textField2Pass, SystemType::ConstantFlowSystem);
    EXPECT_ENUM_EQ(expectedResult, actualFunctionAnswer);
    actualFunctionAnswer = LowTempRadiantControlTypes::MATControl; // reset
    actualFunctionAnswer =
        state->dataLowTempRadSys->ElecRadSys(1).processRadiantSystemControlInput(*state, inputFunction, textField2Pass, SystemType::Electric);
    EXPECT_ENUM_EQ(expectedResult, actualFunctionAnswer);

    // Test 6: Inside Face Surface Temperature test (done for all three types of systems)
    inputFunction = surfaceFaceTemperature;
    expectedResult = LowTempRadiantControlTypes::SurfFaceTempControl;
    actualFunctionAnswer = LowTempRadiantControlTypes::MATControl; // reset
    actualFunctionAnswer =
        state->dataLowTempRadSys->HydrRadSys(1).processRadiantSystemControlInput(*state, inputFunction, textField2Pass, SystemType::HydronicSystem);
    EXPECT_ENUM_EQ(expectedResult, actualFunctionAnswer);
    actualFunctionAnswer = LowTempRadiantControlTypes::MATControl; // reset
    actualFunctionAnswer = state->dataLowTempRadSys->CFloRadSys(1).processRadiantSystemControlInput(
        *state, inputFunction, textField2Pass, SystemType::ConstantFlowSystem);
    EXPECT_ENUM_EQ(expectedResult, actualFunctionAnswer);
    actualFunctionAnswer = LowTempRadiantControlTypes::MATControl; // reset
    actualFunctionAnswer =
        state->dataLowTempRadSys->ElecRadSys(1).processRadiantSystemControlInput(*state, inputFunction, textField2Pass, SystemType::Electric);
    EXPECT_ENUM_EQ(expectedResult, actualFunctionAnswer);

    // Test 7: Inside Face Surface Temperature test (done for all three types of systems)
    inputFunction = surfaceInteriorTemperature;
    expectedResult = LowTempRadiantControlTypes::SurfIntTempControl;
    actualFunctionAnswer = LowTempRadiantControlTypes::MATControl; // reset
    actualFunctionAnswer =
        state->dataLowTempRadSys->HydrRadSys(1).processRadiantSystemControlInput(*state, inputFunction, textField2Pass, SystemType::HydronicSystem);
    EXPECT_ENUM_EQ(expectedResult, actualFunctionAnswer);
    actualFunctionAnswer = LowTempRadiantControlTypes::MATControl; // reset
    actualFunctionAnswer = state->dataLowTempRadSys->CFloRadSys(1).processRadiantSystemControlInput(
        *state, inputFunction, textField2Pass, SystemType::ConstantFlowSystem);
    EXPECT_ENUM_EQ(expectedResult, actualFunctionAnswer);
    actualFunctionAnswer = LowTempRadiantControlTypes::MATControl; // reset
    actualFunctionAnswer =
        state->dataLowTempRadSys->ElecRadSys(1).processRadiantSystemControlInput(*state, inputFunction, textField2Pass, SystemType::Electric);
    EXPECT_ENUM_EQ(expectedResult, actualFunctionAnswer);
}
#endif // GET_OUT

TEST_F(LowTempRadiantSystemTest, setRadiantSystemControlTemperatureTest)
{

    Real64 expectedResult;
    Real64 actualResult;
    Real64 acceptibleError = 0.001;

    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataHeatBal->Zone.allocate(1);
    state->dataHeatBalSurf->SurfTempIn.allocate(1);
    state->dataHeatBalSurf->SurfTempUserLoc.allocate(1);
    state->dataLowTempRadSys->HydrRadSys.allocate(1);
    state->dataLowTempRadSys->CFloRadSys.allocate(1);
    state->dataLowTempRadSys->ElecRadSys.allocate(1);

    // Test Data
    auto &zoneHB1 = state->dataZoneTempPredictorCorrector->zoneHeatBalance(1);
    zoneHB1.MAT = 23.456;
    zoneHB1.MRT = 12.345;
    state->dataHeatBal->Zone(1).OutDryBulbTemp = 34.567;
    state->dataHeatBal->Zone(1).OutWetBulbTemp = 1.234;
    state->dataHeatBalSurf->SurfTempIn(1) = 5.678;
    state->dataHeatBalSurf->SurfTempUserLoc(1) = 7.890;
    state->dataLowTempRadSys->HydrRadSys(1).ZonePtr = 1;
    state->dataLowTempRadSys->HydrRadSys(1).SurfacePtr.allocate(1);
    state->dataLowTempRadSys->HydrRadSys(1).SurfacePtr(1) = 1;
    state->dataLowTempRadSys->CFloRadSys(1).ZonePtr = 1;
    state->dataLowTempRadSys->CFloRadSys(1).SurfacePtr.allocate(1);
    state->dataLowTempRadSys->CFloRadSys(1).SurfacePtr(1) = 1;
    state->dataLowTempRadSys->ElecRadSys(1).ZonePtr = 1;
    state->dataLowTempRadSys->ElecRadSys(1).SurfacePtr.allocate(1);
    state->dataLowTempRadSys->ElecRadSys(1).SurfacePtr(1) = 1;

    // Test 1: MAT Control
    state->dataLowTempRadSys->HydrRadSys(1).controlType = CtrlType::MAT;
    expectedResult = zoneHB1.MAT;
    actualResult = 0.0; // reset
    actualResult =
        state->dataLowTempRadSys->HydrRadSys(1).setRadiantSystemControlTemperature(*state, state->dataLowTempRadSys->HydrRadSys(1).controlType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);
    state->dataLowTempRadSys->CFloRadSys(1).controlType = CtrlType::MAT;
    expectedResult = zoneHB1.MAT;
    actualResult = 0.0; // reset
    actualResult =
        state->dataLowTempRadSys->CFloRadSys(1).setRadiantSystemControlTemperature(*state, state->dataLowTempRadSys->CFloRadSys(1).controlType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);
    state->dataLowTempRadSys->ElecRadSys(1).controlType = CtrlType::MAT;
    expectedResult = zoneHB1.MAT;
    actualResult = 0.0; // reset
    actualResult =
        state->dataLowTempRadSys->ElecRadSys(1).setRadiantSystemControlTemperature(*state, state->dataLowTempRadSys->ElecRadSys(1).controlType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);

    // Test 2: MRT Control
    state->dataLowTempRadSys->HydrRadSys(1).controlType = CtrlType::MRT;
    expectedResult = zoneHB1.MRT;
    actualResult = 0.0; // reset
    actualResult =
        state->dataLowTempRadSys->HydrRadSys(1).setRadiantSystemControlTemperature(*state, state->dataLowTempRadSys->HydrRadSys(1).controlType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);
    state->dataLowTempRadSys->CFloRadSys(1).controlType = CtrlType::MRT;
    expectedResult = zoneHB1.MRT;
    actualResult = 0.0; // reset
    actualResult =
        state->dataLowTempRadSys->CFloRadSys(1).setRadiantSystemControlTemperature(*state, state->dataLowTempRadSys->CFloRadSys(1).controlType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);
    state->dataLowTempRadSys->ElecRadSys(1).controlType = CtrlType::MRT;
    expectedResult = zoneHB1.MRT;
    actualResult = 0.0; // reset
    actualResult =
        state->dataLowTempRadSys->ElecRadSys(1).setRadiantSystemControlTemperature(*state, state->dataLowTempRadSys->ElecRadSys(1).controlType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);

    // Test 3: Operative Temperature Control
    state->dataLowTempRadSys->HydrRadSys(1).controlType = CtrlType::Operative;
    expectedResult = (zoneHB1.MAT + zoneHB1.MRT) / 2.0;
    actualResult = 0.0; // reset
    actualResult =
        state->dataLowTempRadSys->HydrRadSys(1).setRadiantSystemControlTemperature(*state, state->dataLowTempRadSys->HydrRadSys(1).controlType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);
    state->dataLowTempRadSys->CFloRadSys(1).controlType = CtrlType::Operative;
    expectedResult = (zoneHB1.MAT + zoneHB1.MRT) / 2.0;
    actualResult = 0.0; // reset
    actualResult =
        state->dataLowTempRadSys->CFloRadSys(1).setRadiantSystemControlTemperature(*state, state->dataLowTempRadSys->CFloRadSys(1).controlType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);
    state->dataLowTempRadSys->ElecRadSys(1).controlType = CtrlType::Operative;
    expectedResult = (zoneHB1.MAT + zoneHB1.MRT) / 2.0;
    actualResult = 0.0; // reset
    actualResult =
        state->dataLowTempRadSys->ElecRadSys(1).setRadiantSystemControlTemperature(*state, state->dataLowTempRadSys->ElecRadSys(1).controlType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);

    // Test 4: ODB Temperature Control
    state->dataLowTempRadSys->HydrRadSys(1).controlType = CtrlType::ODB;
    expectedResult = state->dataHeatBal->Zone(1).OutDryBulbTemp;
    actualResult = 0.0; // reset
    actualResult =
        state->dataLowTempRadSys->HydrRadSys(1).setRadiantSystemControlTemperature(*state, state->dataLowTempRadSys->HydrRadSys(1).controlType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);
    state->dataLowTempRadSys->CFloRadSys(1).controlType = CtrlType::ODB;
    expectedResult = state->dataHeatBal->Zone(1).OutDryBulbTemp;
    actualResult = 0.0; // reset
    actualResult =
        state->dataLowTempRadSys->CFloRadSys(1).setRadiantSystemControlTemperature(*state, state->dataLowTempRadSys->CFloRadSys(1).controlType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);
    state->dataLowTempRadSys->ElecRadSys(1).controlType = CtrlType::ODB;
    expectedResult = state->dataHeatBal->Zone(1).OutDryBulbTemp;
    actualResult = 0.0; // reset
    actualResult =
        state->dataLowTempRadSys->ElecRadSys(1).setRadiantSystemControlTemperature(*state, state->dataLowTempRadSys->ElecRadSys(1).controlType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);

    // Test 5: OWB Temperature Control
    state->dataLowTempRadSys->HydrRadSys(1).controlType = CtrlType::OWB;
    expectedResult = state->dataHeatBal->Zone(1).OutWetBulbTemp;
    actualResult = 0.0; // reset
    actualResult =
        state->dataLowTempRadSys->HydrRadSys(1).setRadiantSystemControlTemperature(*state, state->dataLowTempRadSys->HydrRadSys(1).controlType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);
    state->dataLowTempRadSys->CFloRadSys(1).controlType = CtrlType::OWB;
    expectedResult = state->dataHeatBal->Zone(1).OutWetBulbTemp;
    actualResult = 0.0; // reset
    actualResult =
        state->dataLowTempRadSys->CFloRadSys(1).setRadiantSystemControlTemperature(*state, state->dataLowTempRadSys->CFloRadSys(1).controlType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);
    state->dataLowTempRadSys->ElecRadSys(1).controlType = CtrlType::OWB;
    expectedResult = state->dataHeatBal->Zone(1).OutWetBulbTemp;
    actualResult = 0.0; // reset
    actualResult =
        state->dataLowTempRadSys->ElecRadSys(1).setRadiantSystemControlTemperature(*state, state->dataLowTempRadSys->ElecRadSys(1).controlType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);

    // Test 6: Surface Inside Face Temperature Control
    state->dataLowTempRadSys->HydrRadSys(1).controlType = CtrlType::SurfFaceTemp;
    expectedResult = state->dataHeatBalSurf->SurfTempIn(1);
    actualResult = 0.0; // reset
    actualResult =
        state->dataLowTempRadSys->HydrRadSys(1).setRadiantSystemControlTemperature(*state, state->dataLowTempRadSys->HydrRadSys(1).controlType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);
    state->dataLowTempRadSys->CFloRadSys(1).controlType = CtrlType::SurfFaceTemp;
    expectedResult = state->dataHeatBalSurf->SurfTempIn(1);
    actualResult = 0.0; // reset
    actualResult =
        state->dataLowTempRadSys->CFloRadSys(1).setRadiantSystemControlTemperature(*state, state->dataLowTempRadSys->CFloRadSys(1).controlType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);
    state->dataLowTempRadSys->ElecRadSys(1).controlType = CtrlType::SurfFaceTemp;
    expectedResult = state->dataHeatBalSurf->SurfTempIn(1);
    actualResult = 0.0; // reset
    actualResult =
        state->dataLowTempRadSys->ElecRadSys(1).setRadiantSystemControlTemperature(*state, state->dataLowTempRadSys->ElecRadSys(1).controlType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);

    // Test 7: Surface Inside (within the slab) Temperature Control
    state->dataLowTempRadSys->HydrRadSys(1).controlType = CtrlType::SurfIntTemp;
    expectedResult = state->dataHeatBalSurf->SurfTempUserLoc(1);
    actualResult = 0.0; // reset
    actualResult =
        state->dataLowTempRadSys->HydrRadSys(1).setRadiantSystemControlTemperature(*state, state->dataLowTempRadSys->HydrRadSys(1).controlType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);
    state->dataLowTempRadSys->CFloRadSys(1).controlType = CtrlType::SurfIntTemp;
    expectedResult = state->dataHeatBalSurf->SurfTempUserLoc(1);
    actualResult = 0.0; // reset
    actualResult =
        state->dataLowTempRadSys->CFloRadSys(1).setRadiantSystemControlTemperature(*state, state->dataLowTempRadSys->CFloRadSys(1).controlType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);
    state->dataLowTempRadSys->ElecRadSys(1).controlType = CtrlType::SurfIntTemp;
    expectedResult = state->dataHeatBalSurf->SurfTempUserLoc(1);
    actualResult = 0.0; // reset
    actualResult =
        state->dataLowTempRadSys->ElecRadSys(1).setRadiantSystemControlTemperature(*state, state->dataLowTempRadSys->ElecRadSys(1).controlType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);

    // Test 8: Running Mean Outdoor Air Temperature Control (Constant Flow System Only)
    state->dataLowTempRadSys->CFloRadSys(1).controlType = CtrlType::RunningMeanODB;
    state->dataLowTempRadSys->CFloRadSys(1).todayRunningMeanOutdoorDryBulbTemperature = 12.345;
    expectedResult = state->dataLowTempRadSys->CFloRadSys(1).todayRunningMeanOutdoorDryBulbTemperature;
    actualResult = 0.0; // reset
    actualResult =
        state->dataLowTempRadSys->CFloRadSys(1).setRadiantSystemControlTemperature(*state, state->dataLowTempRadSys->CFloRadSys(1).controlType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);
}

TEST_F(LowTempRadiantSystemTest, calculateOperationalFractionTest)
{
    Real64 offTemperature;
    Real64 controlTemperature;
    Real64 throttlingRange;
    Real64 functionResult;
    Real64 expectedResult;

    state->dataLowTempRadSys->HydrRadSys.allocate(1);
    auto &thisRadSys(state->dataLowTempRadSys->HydrRadSys(1));

    // Test 1: Temperature Difference is 0-->answer should be 0.0
    offTemperature = 15.0;
    controlTemperature = 15.0;
    throttlingRange = 1.0;
    expectedResult = 0.0;
    functionResult = thisRadSys.calculateOperationalFraction(offTemperature, controlTemperature, throttlingRange);
    EXPECT_NEAR(expectedResult, functionResult, 0.001);

    // Test 2a: Temperature Difference is not zero and positive, throttling range is zero-->answer should be 1.0
    offTemperature = 16.0;
    controlTemperature = 15.0;
    throttlingRange = 0.0;
    expectedResult = 1.0;
    functionResult = thisRadSys.calculateOperationalFraction(offTemperature, controlTemperature, throttlingRange);
    EXPECT_NEAR(expectedResult, functionResult, 0.001);

    // Test 2b: Temperature Difference is not zero and negtive, throttling range is zero-->answer should be 1.0
    offTemperature = 14.0;
    controlTemperature = 15.0;
    throttlingRange = 0.0;
    expectedResult = 1.0;
    functionResult = thisRadSys.calculateOperationalFraction(offTemperature, controlTemperature, throttlingRange);
    EXPECT_NEAR(expectedResult, functionResult, 0.001);

    // Test 3a: Temperature Difference is not zero and positive, throttling range is non-zero but greater than temperature difference
    offTemperature = 16.0;
    controlTemperature = 15.0;
    throttlingRange = 2.0;
    expectedResult = 0.5;
    functionResult = thisRadSys.calculateOperationalFraction(offTemperature, controlTemperature, throttlingRange);
    EXPECT_NEAR(expectedResult, functionResult, 0.001);

    // Test 3b: Temperature Difference is not zero and negative, throttling range is non-zero but greater than temperature difference
    offTemperature = 14.0;
    controlTemperature = 15.0;
    throttlingRange = 2.0;
    expectedResult = 0.5;
    functionResult = thisRadSys.calculateOperationalFraction(offTemperature, controlTemperature, throttlingRange);
    EXPECT_NEAR(expectedResult, functionResult, 0.001);
}

TEST_F(LowTempRadiantSystemTest, calculateOperationalFractionMaxLimitTest)
{
    Real64 offTemperature;
    Real64 controlTemperature;
    Real64 throttlingRange;
    Real64 functionResult;
    Real64 expectedResult;

    state->dataLowTempRadSys->HydrRadSys.allocate(1);
    auto &thisRadSys(state->dataLowTempRadSys->HydrRadSys(1));
    state->dataLowTempRadSys->ElecRadSys.allocate(1);
    auto &thisElecRadSys(state->dataLowTempRadSys->ElecRadSys(1));

    // Test A: Hydronic variable flow system, temperature Difference is not zero and positive,
    //         throttling range is non-zero but less than temperature difference, limit to 1.0 max
    offTemperature = 22.0;
    controlTemperature = 21.0;
    throttlingRange = 0.5;
    expectedResult = 1.0; // delta T/throttlingRange = 2.0 but this needs to be limited to 1.0
    functionResult = thisRadSys.calculateOperationalFraction(offTemperature, controlTemperature, throttlingRange);
    EXPECT_NEAR(expectedResult, functionResult, 0.001);

    // Test B: Hydronic variable flow system, temperature Difference is not zero and negative,
    //         throttling range is non-zero but less than temperature difference, limit to 1.0 max
    offTemperature = 24.0;
    controlTemperature = 25.0;
    throttlingRange = 0.5;
    expectedResult = 1.0; // delta T/throttlingRange = 2.0 but this needs to be limited to 1.0
    functionResult = thisRadSys.calculateOperationalFraction(offTemperature, controlTemperature, throttlingRange);
    EXPECT_NEAR(expectedResult, functionResult, 0.001);

    // Test C: Electric system, temperature Difference is not zero and positive, throttling range
    //         is non-zero but less than temperature difference, limit to 1.0 max
    offTemperature = 23.0;
    controlTemperature = 20.0;
    throttlingRange = 1.0;
    expectedResult = 1.0; // delta T/throttlingRange = 3.0 but this needs to be limited to 1.0
    functionResult = thisElecRadSys.calculateOperationalFraction(offTemperature, controlTemperature, throttlingRange);
    EXPECT_NEAR(expectedResult, functionResult, 0.001);
}

TEST_F(LowTempRadiantSystemTest, setOffTemperatureLowTemperatureRadiantSystemTest)
{
    state->init_state(*state);

    Real64 expectedResult;
    Real64 actualResult;
    Real64 acceptibleError = 0.001;
    Real64 throttlingRange;

    state->dataLowTempRadSys->HydrRadSys.allocate(1);

    // Test 1: zeroFlow and no throttling range
    throttlingRange = 0.0;
    state->dataLowTempRadSys->HydrRadSys(1).setpointType = SetpointType::ZeroFlowPower;
    expectedResult = 1.0;
    actualResult = state->dataLowTempRadSys->HydrRadSys(1).setOffTemperatureLowTemperatureRadiantSystem(
        *state, Sched::GetScheduleAlwaysOn(*state), throttlingRange, state->dataLowTempRadSys->HydrRadSys(1).setpointType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);

    // Test 2: zeroFlow and positive throttling range
    throttlingRange = 0.5;
    state->dataLowTempRadSys->HydrRadSys(1).setpointType = SetpointType::ZeroFlowPower;
    expectedResult = 1.0;
    actualResult = state->dataLowTempRadSys->HydrRadSys(1).setOffTemperatureLowTemperatureRadiantSystem(
        *state, Sched::GetScheduleAlwaysOn(*state), throttlingRange, state->dataLowTempRadSys->HydrRadSys(1).setpointType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);

    // Test 3: zeroFlow and negative throttling range (cooling situation)
    throttlingRange = -0.5;
    state->dataLowTempRadSys->HydrRadSys(1).setpointType = SetpointType::ZeroFlowPower;
    expectedResult = 1.0;
    actualResult = state->dataLowTempRadSys->HydrRadSys(1).setOffTemperatureLowTemperatureRadiantSystem(
        *state, Sched::GetScheduleAlwaysOn(*state), throttlingRange, state->dataLowTempRadSys->HydrRadSys(1).setpointType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);

    // Test 4: halfFlow and no throttling range
    throttlingRange = 0.0;
    state->dataLowTempRadSys->HydrRadSys(1).setpointType = SetpointType::HalfFlowPower;
    expectedResult = 1.0;
    actualResult = state->dataLowTempRadSys->HydrRadSys(1).setOffTemperatureLowTemperatureRadiantSystem(
        *state, Sched::GetScheduleAlwaysOn(*state), throttlingRange, state->dataLowTempRadSys->HydrRadSys(1).setpointType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);

    // Test 5: halfFlow and positive throttling range
    throttlingRange = 0.5;
    state->dataLowTempRadSys->HydrRadSys(1).setpointType = SetpointType::HalfFlowPower;
    expectedResult = 1.25;
    actualResult = state->dataLowTempRadSys->HydrRadSys(1).setOffTemperatureLowTemperatureRadiantSystem(
        *state, Sched::GetScheduleAlwaysOn(*state), throttlingRange, state->dataLowTempRadSys->HydrRadSys(1).setpointType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);

    // Test 5: halfFlow and negative throttling range (cooling situation)
    throttlingRange = -0.5;
    state->dataLowTempRadSys->HydrRadSys(1).setpointType = SetpointType::HalfFlowPower;
    expectedResult = 0.75;
    actualResult = state->dataLowTempRadSys->HydrRadSys(1).setOffTemperatureLowTemperatureRadiantSystem(
        *state, Sched::GetScheduleAlwaysOn(*state), throttlingRange, state->dataLowTempRadSys->HydrRadSys(1).setpointType);
    EXPECT_NEAR(expectedResult, actualResult, acceptibleError);
}

TEST_F(LowTempRadiantSystemTest, errorCheckZonesAndConstructionsTest)
{
    state->init_state(*state);

    bool actualErrorsFound;
    std::string const Alpha1("Zone Name");
    std::string const Alpha2("An Amazing Zone");
    std::string const Alpha3("Hydronic Radiant System");
    std::string const Alpha4("An Excellent Radiant System");

    state->dataLowTempRadSys->HydrRadSys.allocate(1);
    auto &thisRadSys(state->dataLowTempRadSys->HydrRadSys(1));
    thisRadSys.NumOfSurfaces = 3;
    thisRadSys.SurfacePtr.allocate(thisRadSys.NumOfSurfaces);
    thisRadSys.SurfacePtr(1) = 1;
    thisRadSys.SurfacePtr(2) = 2;
    thisRadSys.SurfacePtr(3) = 3;
    thisRadSys.ZonePtr = 1;
    state->dataSurface->Surface.allocate(3);
    state->dataHeatBal->Zone.allocate(3);
    state->dataConstruction->Construct.allocate(2);
    state->dataConstruction->Construct(1).SourceSinkPresent = true;
    state->dataConstruction->Construct(2).SourceSinkPresent = false;

    // Test 1a: Surfaces are in the same zones, zone multipliers are all the same, and the construct has a source/sink.
    //          Everything is "ok" so the result should be the error flag is FALSE.
    actualErrorsFound = false;
    state->dataSurface->Surface(1).Zone = 1;
    state->dataSurface->Surface(2).Zone = 1;
    state->dataSurface->Surface(3).Zone = 1;
    state->dataHeatBal->Zone(1).Multiplier = 1.0;
    state->dataHeatBal->Zone(1).ListMultiplier = 1.0;
    state->dataHeatBal->Zone(2).Multiplier = 1.0;
    state->dataHeatBal->Zone(2).ListMultiplier = 1.0;
    state->dataHeatBal->Zone(3).Multiplier = 1.0;
    state->dataHeatBal->Zone(3).ListMultiplier = 1.0;
    state->dataSurface->Surface(1).Construction = 1;
    state->dataSurface->Surface(2).Construction = 1;
    state->dataSurface->Surface(3).Construction = 1;
    thisRadSys.errorCheckZonesAndConstructions(*state, actualErrorsFound);
    EXPECT_FALSE(actualErrorsFound);

    // Test 1b: Surfaces are in different zones, zone multipliers are all the same, and the construct has a source/sink.
    //          Surfaces being in different zones is "ok" so the result should be the error flag is FALSE.
    actualErrorsFound = false;
    state->dataSurface->Surface(1).Zone = 1;
    state->dataSurface->Surface(2).Zone = 2;
    state->dataSurface->Surface(3).Zone = 3;
    state->dataHeatBal->Zone(1).Multiplier = 1.0;
    state->dataHeatBal->Zone(1).ListMultiplier = 1.0;
    state->dataHeatBal->Zone(2).Multiplier = 1.0;
    state->dataHeatBal->Zone(2).ListMultiplier = 1.0;
    state->dataHeatBal->Zone(3).Multiplier = 1.0;
    state->dataHeatBal->Zone(3).ListMultiplier = 1.0;
    state->dataSurface->Surface(1).Construction = 1;
    state->dataSurface->Surface(2).Construction = 1;
    state->dataSurface->Surface(3).Construction = 1;
    thisRadSys.errorCheckZonesAndConstructions(*state, actualErrorsFound);
    EXPECT_FALSE(actualErrorsFound);

    // Test 2: Surfaces are in different zones, zone multipliers are NOT all the same (one is 7 instead of 2), and the construct has a source/sink.
    //         Zone multipliers can NOT be different so the result should be the error flag is TRUE.
    actualErrorsFound = false;
    state->dataSurface->Surface(1).Zone = 1;
    state->dataSurface->Surface(2).Zone = 2;
    state->dataSurface->Surface(3).Zone = 3;
    state->dataHeatBal->Zone(1).Multiplier = 2.0;
    state->dataHeatBal->Zone(1).ListMultiplier = 1.0;
    state->dataHeatBal->Zone(2).Multiplier = 2.0;
    state->dataHeatBal->Zone(2).ListMultiplier = 1.0;
    state->dataHeatBal->Zone(3).Multiplier = 7.0;
    state->dataHeatBal->Zone(3).ListMultiplier = 1.0;
    state->dataSurface->Surface(1).Construction = 1;
    state->dataSurface->Surface(2).Construction = 1;
    state->dataSurface->Surface(3).Construction = 1;
    thisRadSys.errorCheckZonesAndConstructions(*state, actualErrorsFound);
    EXPECT_TRUE(actualErrorsFound);

    // Test 3: Surfaces are in the same zones, zone multipliers are all the same, and one construct does NOT have a source/sink.
    //         Surface constructions MUST have a source/sink to be used for a radiant system so the result should be the error flag is TRUE.
    actualErrorsFound = false;
    state->dataSurface->Surface(1).Zone = 1;
    state->dataSurface->Surface(2).Zone = 1;
    state->dataSurface->Surface(3).Zone = 1;
    state->dataHeatBal->Zone(1).Multiplier = 2.0;
    state->dataHeatBal->Zone(1).ListMultiplier = 1.0;
    state->dataHeatBal->Zone(2).Multiplier = 2.0;
    state->dataHeatBal->Zone(2).ListMultiplier = 1.0;
    state->dataHeatBal->Zone(3).Multiplier = 2.0;
    state->dataHeatBal->Zone(3).ListMultiplier = 1.0;
    state->dataSurface->Surface(1).Construction = 1;
    state->dataSurface->Surface(2).Construction = 1;
    state->dataSurface->Surface(3).Construction = 2;
    thisRadSys.errorCheckZonesAndConstructions(*state, actualErrorsFound);
    EXPECT_TRUE(actualErrorsFound);
}

TEST_F(LowTempRadiantSystemTest, calculateRunningMeanAverageTemperatureTest)
{
    state->init_state(*state);
    // This tests both calculateRunningMeanAverageTemperature and calculateCurrentDailyAverageODB
    // because calculateCurrentDailyAverageODB is called by calculateRunningMeanAverageTemperature
    Real64 expectedResult;
    Real64 acceptibleError = 0.001;

    state->dataLowTempRadSys->CFloRadSys.allocate(1);
    auto &thisCFloSys(state->dataLowTempRadSys->CFloRadSys(1));
    state->dataLowTempRadSys->CflowRadiantSysDesign.allocate(1);
    auto &thisRadSysDesign(state->dataLowTempRadSys->CflowRadiantSysDesign(1));
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataWeather->wvarsHrTsToday.allocate(state->dataGlobal->TimeStepsInHour, Constant::iHoursInDay);
    for (int hourNumber = 1; hourNumber <= Constant::iHoursInDay; ++hourNumber) {
        state->dataWeather->wvarsHrTsToday(state->dataGlobal->TimeStepsInHour, hourNumber).OutDryBulbTemp = double(hourNumber);
    }

    // Test 1: First day of the simulation and it's in warmup-->everything set to the same temperature
    state->dataGlobal->DayOfSim = 1;
    state->dataGlobal->WarmupFlag = true;
    state->dataGlobal->NumOfDayInEnvrn = 366;
    thisCFloSys.todayAverageOutdoorDryBulbTemperature = -9999.9;
    thisCFloSys.yesterdayAverageOutdoorDryBulbTemperature = -9999.9;
    thisCFloSys.todayRunningMeanOutdoorDryBulbTemperature = -9999.9;
    thisCFloSys.yesterdayRunningMeanOutdoorDryBulbTemperature = -9999.9;
    thisRadSysDesign.runningMeanOutdoorAirTemperatureWeightingFactor = 0.5;
    thisCFloSys.DesignObjectPtr = 1;
    expectedResult = 12.5;
    thisCFloSys.calculateRunningMeanAverageTemperature(*state, 1);
    EXPECT_NEAR(expectedResult, thisCFloSys.todayAverageOutdoorDryBulbTemperature, acceptibleError);
    EXPECT_NEAR(expectedResult, thisCFloSys.yesterdayAverageOutdoorDryBulbTemperature, acceptibleError);
    EXPECT_NEAR(expectedResult, thisCFloSys.todayRunningMeanOutdoorDryBulbTemperature, acceptibleError);
    EXPECT_NEAR(expectedResult, thisCFloSys.yesterdayRunningMeanOutdoorDryBulbTemperature, acceptibleError);

    // Test 2: Not first dsy of simulation but still in warmup-->should not do anything because in warmup same day repeated over and over
    state->dataGlobal->DayOfSim = 2;
    state->dataGlobal->WarmupFlag = true;
    state->dataGlobal->NumOfDayInEnvrn = 366;
    thisCFloSys.todayAverageOutdoorDryBulbTemperature = -9999.9;
    thisCFloSys.yesterdayAverageOutdoorDryBulbTemperature = -9999.9;
    thisCFloSys.todayRunningMeanOutdoorDryBulbTemperature = -9999.9;
    thisCFloSys.yesterdayRunningMeanOutdoorDryBulbTemperature = -9999.9;
    thisCFloSys.runningMeanOutdoorAirTemperatureWeightingFactor = 0.5;
    expectedResult = -9999.9;
    thisCFloSys.calculateRunningMeanAverageTemperature(*state, 1);
    EXPECT_NEAR(expectedResult, thisCFloSys.todayAverageOutdoorDryBulbTemperature, acceptibleError);
    EXPECT_NEAR(expectedResult, thisCFloSys.yesterdayAverageOutdoorDryBulbTemperature, acceptibleError);
    EXPECT_NEAR(expectedResult, thisCFloSys.todayRunningMeanOutdoorDryBulbTemperature, acceptibleError);
    EXPECT_NEAR(expectedResult, thisCFloSys.yesterdayRunningMeanOutdoorDryBulbTemperature, acceptibleError);

    // Test 3: Not in warmup but number of days of simulation only 1-->should not do anything because it's a single day which means no real history
    state->dataGlobal->DayOfSim = 1;
    state->dataGlobal->WarmupFlag = false;
    state->dataGlobal->NumOfDayInEnvrn = 1;
    thisCFloSys.todayAverageOutdoorDryBulbTemperature = 12.345;
    thisCFloSys.yesterdayAverageOutdoorDryBulbTemperature = 12.345;
    thisCFloSys.todayRunningMeanOutdoorDryBulbTemperature = 12.345;
    thisCFloSys.yesterdayRunningMeanOutdoorDryBulbTemperature = 12.345;
    thisCFloSys.runningMeanOutdoorAirTemperatureWeightingFactor = 0.5;
    expectedResult = 12.345;
    thisCFloSys.calculateRunningMeanAverageTemperature(*state, 1);
    EXPECT_NEAR(expectedResult, thisCFloSys.todayAverageOutdoorDryBulbTemperature, acceptibleError);
    EXPECT_NEAR(expectedResult, thisCFloSys.yesterdayAverageOutdoorDryBulbTemperature, acceptibleError);
    EXPECT_NEAR(expectedResult, thisCFloSys.todayRunningMeanOutdoorDryBulbTemperature, acceptibleError);
    EXPECT_NEAR(expectedResult, thisCFloSys.yesterdayRunningMeanOutdoorDryBulbTemperature, acceptibleError);

    // Test 4: Not in warmup and number of days of simulation greater than 1-->apply the formula for running mean temperature and shift data
    state->dataGlobal->DayOfSim = 1;
    state->dataGlobal->WarmupFlag = false;
    state->dataGlobal->NumOfDayInEnvrn = 366;
    thisCFloSys.todayAverageOutdoorDryBulbTemperature = 15.0;
    thisCFloSys.yesterdayAverageOutdoorDryBulbTemperature = 10.0;
    thisCFloSys.todayRunningMeanOutdoorDryBulbTemperature = 14.5;
    thisCFloSys.yesterdayRunningMeanOutdoorDryBulbTemperature = 5.0;
    thisCFloSys.runningMeanOutdoorAirTemperatureWeightingFactor = 0.5;
    thisCFloSys.calculateRunningMeanAverageTemperature(*state, 1);
    expectedResult = 12.5; // Average of TodayOutDryBulbTemp(firstTimeStepIndex,hourNumber)
    EXPECT_NEAR(expectedResult, thisCFloSys.todayAverageOutdoorDryBulbTemperature, acceptibleError);
    expectedResult = 15.0; // Should transfer what was todayAverageOutdoorDryBulbTemperature (see above)
    EXPECT_NEAR(expectedResult, thisCFloSys.yesterdayAverageOutdoorDryBulbTemperature, acceptibleError);
    expectedResult = 14.5; // Should transfer what was todayRunningMeanOutdoorDryBulbTemperature (see above)
    EXPECT_NEAR(expectedResult, thisCFloSys.yesterdayRunningMeanOutdoorDryBulbTemperature, acceptibleError);
    expectedResult = 14.75; // Should be weighted average the "yesterday" values using the weighting factor
    EXPECT_NEAR(expectedResult, thisCFloSys.todayRunningMeanOutdoorDryBulbTemperature, acceptibleError);
}

TEST_F(LowTempRadiantSystemTest, updateOperatingModeHistoryTest)
{
    state->init_state(*state);
    int expectedResult = 1;
    int resetResult = -9999;
    state->dataLowTempRadSys->HydrRadSys.allocate(1);
    state->dataGlobal->TimeStepsInHour = 6;
    state->dataGlobal->DayOfSim = 2;
    state->dataGlobal->HourOfDay = 4;
    state->dataGlobal->TimeStep = 5;
    auto &thisRadSys(state->dataLowTempRadSys->HydrRadSys(1));

    // Test 1: Operating Mode different, beginning of day-->lastOperatingMode should switch, last parameters should get set appropriately
    thisRadSys.lastOpMode = OpMode::Heat;
    thisRadSys.opMode = OpMode::Cool;
    thisRadSys.lastDayOfSim = resetResult;
    thisRadSys.lastHourOfDay = resetResult;
    thisRadSys.lastTimeStep = resetResult;
    state->dataGlobal->BeginDayFlag = true;
    state->dataGlobal->BeginHourFlag = false;
    state->dataGlobal->BeginTimeStepFlag = false;
    thisRadSys.updateOperatingModeHistory(*state);
    expectedResult = 1;
    EXPECT_EQ(thisRadSys.lastDayOfSim, expectedResult);
    expectedResult = Constant::iHoursInDay;
    EXPECT_EQ(thisRadSys.lastHourOfDay, expectedResult);
    expectedResult = state->dataGlobal->TimeStepsInHour;
    EXPECT_EQ(thisRadSys.lastTimeStep, expectedResult);
    EXPECT_ENUM_EQ(thisRadSys.lastOpMode, OpMode::Cool);
    EXPECT_ENUM_EQ(thisRadSys.opMode, OpMode::None);

    // Test 2: Operating Mode different, beginning of hour-->lastOperatingMode should switch, last parameters should get set appropriately
    thisRadSys.lastOpMode = OpMode::Heat;
    thisRadSys.opMode = OpMode::Cool;
    thisRadSys.lastDayOfSim = resetResult;
    thisRadSys.lastHourOfDay = resetResult;
    thisRadSys.lastTimeStep = resetResult;
    state->dataGlobal->BeginDayFlag = false;
    state->dataGlobal->BeginHourFlag = true;
    state->dataGlobal->BeginTimeStepFlag = false;
    thisRadSys.updateOperatingModeHistory(*state);
    expectedResult = 2;
    EXPECT_EQ(thisRadSys.lastDayOfSim, expectedResult);
    expectedResult = 3;
    EXPECT_EQ(thisRadSys.lastHourOfDay, expectedResult);
    expectedResult = state->dataGlobal->TimeStepsInHour;
    EXPECT_EQ(thisRadSys.lastTimeStep, expectedResult);
    EXPECT_EQ(thisRadSys.lastOpMode, OpMode::Cool);
    EXPECT_EQ(thisRadSys.opMode, OpMode::None);

    // Test 3: Operating Mode different, beginning of time step-->lastOpMode should switch, last parameters should get set appropriately
    thisRadSys.lastOpMode = OpMode::Heat;
    thisRadSys.opMode = OpMode::Cool;
    thisRadSys.lastDayOfSim = resetResult;
    thisRadSys.lastHourOfDay = resetResult;
    thisRadSys.lastTimeStep = resetResult;
    state->dataGlobal->BeginDayFlag = false;
    state->dataGlobal->BeginHourFlag = false;
    state->dataGlobal->BeginTimeStepFlag = true;
    thisRadSys.updateOperatingModeHistory(*state);
    expectedResult = 2;
    EXPECT_EQ(thisRadSys.lastDayOfSim, expectedResult);
    expectedResult = 4;
    EXPECT_EQ(thisRadSys.lastHourOfDay, expectedResult);
    expectedResult = 4;
    EXPECT_EQ(thisRadSys.lastTimeStep, expectedResult);
    EXPECT_ENUM_EQ(thisRadSys.lastOpMode, OpMode::Cool);
    EXPECT_ENUM_EQ(thisRadSys.opMode, OpMode::None);

    // Test 4: Operating Mode different, not beginning of day, hour, or time step-->lastOpMode should switch, last parameters should get set
    // appropriately
    thisRadSys.lastOpMode = OpMode::Heat;
    thisRadSys.opMode = OpMode::Cool;
    thisRadSys.lastDayOfSim = resetResult;
    thisRadSys.lastHourOfDay = resetResult;
    thisRadSys.lastTimeStep = resetResult;
    state->dataGlobal->BeginDayFlag = false;
    state->dataGlobal->BeginHourFlag = false;
    state->dataGlobal->BeginTimeStepFlag = false;
    thisRadSys.updateOperatingModeHistory(*state);
    expectedResult = 2;
    EXPECT_EQ(thisRadSys.lastDayOfSim, expectedResult);
    expectedResult = 4;
    EXPECT_EQ(thisRadSys.lastHourOfDay, expectedResult);
    expectedResult = 5;
    EXPECT_EQ(thisRadSys.lastTimeStep, expectedResult);
    EXPECT_ENUM_EQ(thisRadSys.lastOpMode, OpMode::Cool);
    EXPECT_ENUM_EQ(thisRadSys.opMode, OpMode::None);
}

TEST_F(LowTempRadiantSystemTest, setOperatingModeBasedOnChangeoverDelayTest)
{
    state->init_state(*state);
    state->dataLowTempRadSys->HydrRadSys.allocate(1);
    auto &thisRadSys(state->dataLowTempRadSys->HydrRadSys(1));
    state->dataGlobal->TimeStepsInHour = 6;
    state->dataGlobal->MinutesInTimeStep = 10.0;

    // Test 1: lastOpMode is NotOperating-->don't do anything to opMode
    thisRadSys.lastOpMode = OpMode::None;
    thisRadSys.opMode = OpMode::Heat;
    thisRadSys.setOperatingModeBasedOnChangeoverDelay(*state);
    EXPECT_ENUM_EQ(thisRadSys.opMode, OpMode::Heat);

    // Test 2: lastOpMode is not NotOperating, opMode is NotOperating-->don't do anything to opMode
    thisRadSys.lastOpMode = OpMode::Heat;
    thisRadSys.opMode = OpMode::None;
    thisRadSys.setOperatingModeBasedOnChangeoverDelay(*state);
    EXPECT_ENUM_EQ(thisRadSys.opMode, OpMode::None);

    // Test 3: lastOpMode and opMode are both the same (and not NotOperating)-->don't do anything to opMode
    thisRadSys.lastOpMode = OpMode::Heat;
    thisRadSys.opMode = OpMode::Heat;
    thisRadSys.setOperatingModeBasedOnChangeoverDelay(*state);
    EXPECT_ENUM_EQ(thisRadSys.opMode, OpMode::Heat);

    // Test 4: lastOpMode and opMode are different and neither is not NotOperating plus the schedule index is zero (no delay)-->don't do
    // anything to opMode
    thisRadSys.lastOpMode = OpMode::Heat;
    thisRadSys.opMode = OpMode::Cool;
    thisRadSys.changeoverDelaySched = nullptr;
    thisRadSys.setOperatingModeBasedOnChangeoverDelay(*state);
    EXPECT_ENUM_EQ(thisRadSys.opMode, OpMode::Cool);

    // Test 5a: lastOpMode and opMode are different and neither is not NotOperating, the
    //          schedule index is non-zero and schedule value is non zero, but it hasn't been long enough
    //          to switch over yet-->change opMode to NotOperating
    thisRadSys.lastOpMode = OpMode::Heat;
    thisRadSys.opMode = OpMode::Cool;
    thisRadSys.changeoverDelaySched = Sched::GetScheduleAlwaysOn(*state);
    state->dataGlobal->DayOfSim = 2;
    state->dataGlobal->HourOfDay = 1;
    state->dataGlobal->TimeStep = 1;
    thisRadSys.lastDayOfSim = 1;
    thisRadSys.lastHourOfDay = 24;
    thisRadSys.lastTimeStep = 2;
    thisRadSys.setOperatingModeBasedOnChangeoverDelay(*state);
    EXPECT_ENUM_EQ(thisRadSys.opMode, OpMode::None);

    // Test 6b: lastOpMode and opMode are different and neither is not NotOperating, the
    //          schedule index is non-zero and schedule value is non zero, but it has been long enough
    //          to switch over yet-->don't do anything to opMode
    thisRadSys.lastOpMode = OpMode::Heat;
    thisRadSys.opMode = OpMode::Cool;
    state->dataGlobal->DayOfSim = 2;
    state->dataGlobal->HourOfDay = 1;
    state->dataGlobal->TimeStep = 4;
    thisRadSys.lastDayOfSim = 1;
    thisRadSys.lastHourOfDay = 22;
    thisRadSys.lastTimeStep = 3;
    thisRadSys.setOperatingModeBasedOnChangeoverDelay(*state);
    EXPECT_ENUM_EQ(thisRadSys.opMode, OpMode::Cool);
}

#ifdef GET_OUT
// No need to unit test getEnumValue
TEST_F(LowTempRadiantSystemTest, getFluidToSlabHeatTransferInputTest)
{
    state->init_state(*state);
    state->dataLowTempRadSys->CFloRadSys.allocate(1);
    auto &thisCFloSys(state->dataLowTempRadSys->CFloRadSys(1));
    state->dataLowTempRadSys->CflowRadiantSysDesign.allocate(1);
    //    auto &thisRadSysDesign(CflowRadiantSysDesign(1));
    std::string userInput;

    // Test 1: Input is ConvectionOnly--so this field needs to get reset to ConvectionOnly
    userInput = "ConvectionOnly";
    DesignObjectNum = 1;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).DesignObjectPtr = 1;
    state->dataLowTempRadSys->CflowRadiantSysDesign(DesignObjectNum).FluidToSlabHeatTransfer = FluidToSlabHeatTransferType::ISOStandard;
    state->dataLowTempRadSys->CflowRadiantSysDesign(DesignObjectNum).FluidToSlabHeatTransfer =
        thisCFloSys.getFluidToSlabHeatTransferInput(*state, userInput);
    EXPECT_ENUM_EQ(FluidToSlabHeatTransferTypes::ConvectionOnly,
                   state->dataLowTempRadSys->CflowRadiantSysDesign(DesignObjectNum).FluidToSlabHeatTransfer);

    // Test 2: Input is ISOStandard--so this field needs to get reset to ISOStandard
    userInput = "ISOStandard";
    state->dataLowTempRadSys->CflowRadiantSysDesign(DesignObjectNum).FluidToSlabHeatTransfer = FluidToSlabHeatTransferTypes::ConvectionOnly;
    state->dataLowTempRadSys->CflowRadiantSysDesign(DesignObjectNum).FluidToSlabHeatTransfer =
        thisCFloSys.getFluidToSlabHeatTransferInput(*state, userInput);
    EXPECT_ENUM_EQ(FluidToSlabHeatTransferTypes::ISOStandard,
                   state->dataLowTempRadSys->CflowRadiantSysDesign(DesignObjectNum).FluidToSlabHeatTransfer);

    // Test 3: Input is ISOStandard--so this field needs to get reset to ConvectionOnly (the default)
    userInput = "WeWantSomethingElse!";
    state->dataLowTempRadSys->CflowRadiantSysDesign(DesignObjectNum).FluidToSlabHeatTransfer = FluidToSlabHeatTransferTypes::ISOStandard;
    state->dataLowTempRadSys->CflowRadiantSysDesign(DesignObjectNum).FluidToSlabHeatTransfer =
        thisCFloSys.getFluidToSlabHeatTransferInput(*state, userInput);
    EXPECT_ENUM_EQ(FluidToSlabHeatTransferTypes::ConvectionOnly,
                   state->dataLowTempRadSys->CflowRadiantSysDesign(DesignObjectNum).FluidToSlabHeatTransfer);
}
#endif // GET_OUT

TEST_F(LowTempRadiantSystemTest, calculateUFromISOStandardTest)
{
    state->init_state(*state);
    // Test of the ISO Standard 11855-2 Method for calculating the U-value for heat transfer
    // between the fluid being circulated through a radiant system and the radiant system
    // material that the pipe/tube is embedded within
    int SurfNum = 1;
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).Construction = 1;
    state->dataConstruction->Construct.allocate(1);
    state->dataConstruction->Construct(1).ThicknessPerpend = 0.5;
    state->dataLowTempRadSys->CFloRadSys.allocate(1);
    auto &thisCFloSys(state->dataLowTempRadSys->CFloRadSys(1));
    state->dataLowTempRadSys->CflowRadiantSysDesign.allocate(1);
    thisCFloSys.TubeLength = 100.0;
    DesignObjectNum = 1;
    state->dataLowTempRadSys->CflowRadiantSysDesign(DesignObjectNum).TubeDiameterInner = 0.01;
    state->dataLowTempRadSys->CflowRadiantSysDesign(DesignObjectNum).TubeDiameterOuter = 0.011;
    state->dataLowTempRadSys->CflowRadiantSysDesign(DesignObjectNum).ConstFlowTubeConductivity = 0.5;
    state->dataLowTempRadSys->CFloRadSys(RadSysNum).DesignObjectPtr = 1;
    Real64 WaterMassFlow = 0.001;

    Real64 expectedResult = 28.00687;
    Real64 allowedDifference = 0.00001;
    Real64 actualResult = thisCFloSys.calculateUFromISOStandard(
        *state, SurfNum, WaterMassFlow, SystemType::ConstantFlow, state->dataLowTempRadSys->CFloRadSys(RadSysNum).DesignObjectPtr);
    EXPECT_NEAR(expectedResult, actualResult, allowedDifference);
}

TEST_F(LowTempRadiantSystemTest, GetLowTempRadiantSystem_MultipleTypes)
{
    // #8564 - GetLowTempRadiantSystem fails when you have more a LowTempRadVarFlow AND at least one other type
    std::string const idf_objects = delimited_string({

        "ZoneHVAC:LowTemperatureRadiant:VariableFlow,",
        "  West Zone Radiant Floor,                !- Name",
        "  West Zone Radiant Floor Design,         !- Design Object",
        "  RadiantSysAvailSched,                   !- Availability Schedule Name",
        "  West Zone,                              !- Zone Name",
        "  West Zone Surface Group,                !- Surface Name or Radiant Surface Group Name",
        "  Autosize,                               !- Hydronic Tubing Length {m}",
        "  Autosize,                               !- Heating Design Capacity {W}",
        "  Autosize,                               !- Maximum Hot Water Flow {m3/s}",
        "  Node 276,                               !- Heating Water Inlet Node Name",
        "  Node 277,                               !- Heating Water Outlet Node Name",
        "  Autosize,                               !- Cooling Design Capacity {W}",
        "  Autosize,                               !- Maximum Cold Water Flow {m3/s}",
        "  Node 278,                               !- Cooling Water Inlet Node Name",
        "  Node 279,                               !- Cooling Water Outlet Node Name",
        "  OnePerSurface,                          !- Number of Circuits",
        "  106.7;                                  !- Circuit Length {m}",

        "ZoneHVAC:LowTemperatureRadiant:VariableFlow:Design,",
        "  West Zone Radiant Floor Design,         !- Name",
        "  ConvectionOnly,                         !- Fluid to Radiant Surface Heat Transfer Model",
        "  0.013,                                  !- Hydronic Tubing Inside Diameter {m}",
        "  0.016,                                  !- Hydronic Tubing Outside Diameter {m}",
        "  0.35,                                   !- Hydronic Tubing Conductivity {W/m-K}",
        "  MeanAirTemperature,                     !- Temperature Control Type",
        "  HalfFlowPower,                          !- Setpoint Control Type",
        "  HeatingDesignCapacity,                  !- Heating Design Capacity Method",
        "  0,                                      !- Heating Design Capacity Per Floor Area {W/m2}",
        "  1,                                      !- Fraction of Autosized Heating Design Capacity",
        "  0.5,                                    !- Heating Control Throttling Range {deltaC}",
        "  Radiant Heating Setpoints,              !- Heating Control Temperature Schedule Name",
        "  CoolingDesignCapacity,                  !- Cooling Design Capacity Method",
        "  0,                                      !- Cooling Design Capacity Per Floor Area {W/m2}",
        "  1,                                      !- Fraction of Autosized Cooling Design Capacity",
        "  0.5,                                    !- Cooling Control Throttling Range {deltaC}",
        "  Radiant Cooling Setpoints,              !- Cooling Control Temperature Schedule Name",
        "  SimpleOff,                              !- Condensation Control Type",
        "  1;                                      !- Condensation Control Dewpoint Offset {C}",

        "ZoneHVAC:LowTemperatureRadiant:ConstantFlow,",
        "  South Zone LowTempRad,                  !- Name",
        "  South Zone LowTempRad Design,           !- Design Object",
        "  RadiantSysAvailSched,                   !- Availability Schedule Name",
        "  South Zone,                             !- Zone Name",
        "  South Zone Surface Group,               !- Surface Name or Radiant Surface Group Name",
        "  Autosize,                               !- Hydronic Tubing Length {m}",
        "  Autosize,                               !- Rated Flow Rate {m3/s}",
        "  ,                                       !- Pump Flow Rate Schedule Name",
        "  179352,                                 !- Rated Pump Head {Pa}",
        "  ,                                       !- Rated Power Consumption {W}",
        "  ,                                       !- Heating Water Inlet Node Name",
        "  ,                                       !- Heating Water Outlet Node Name",
        "  Radiant Heating Setpoints,              !- Heating High Water Temperature Schedule Name", // I'm not testing schedules...
        "  Radiant Heating Setpoints,              !- Heating Low Water Temperature Schedule Name",
        "  Radiant Heating Setpoints,              !- Heating High Control Temperature Schedule Name",
        "  Radiant Heating Setpoints,              !- Heating Low Control Temperature Schedule Name",
        "  ,                                       !- Cooling Water Inlet Node Name",
        "  ,                                       !- Cooling Water Outlet Node Name",
        "  Radiant Cooling Setpoints,              !- Cooling High Water Temperature Schedule Name",
        "  Radiant Cooling Setpoints,              !- Cooling Low Water Temperature Schedule Name",
        "  Radiant Cooling Setpoints,              !- Cooling High Control Temperature Schedule Name",
        "  Radiant Cooling Setpoints,              !- Cooling Low Control Temperature Schedule Name",
        "  OnePerSurface,                          !- Number of Circuits",
        "  106.7;                                  !- Circuit Length {m}",

        "ZoneHVAC:LowTemperatureRadiant:ConstantFlow:Design,",
        "  South Zone LowTempRad Design, !- Name",
        "  ConvectionOnly,                         !- Fluid to Radiant Surface Heat Transfer Model",
        "  0.013,                                  !- Hydronic Tubing Inside Diameter {m}",
        "  0.016,                                  !- Hydronic Tubing Outside Diameter {m}",
        "  0.35,                                   !- Hydronic Tubing Conductivity {W/m-K}",
        "  MeanAirTemperature,                     !- Temperature Control Type",
        "  0.8,                                    !- Running Mean Outdoor Dry-Bulb Temperature Weighting Factor",
        "  0.9,                                    !- Motor Efficiency",
        "  0,                                      !- Fraction of Motor Inefficiencies to Fluid Stream",
        "  SimpleOff,                              !- Condensation Control Type",
        "  1;                                      !- Condensation Control Dewpoint Offset {C}",

        "  ZoneHVAC:LowTemperatureRadiant:Electric,",
        "    East Zone Radiant Floor, !- Name",
        "    RadiantSysAvailSched,    !- Availability Schedule Name",
        "    East Zone,               !- Zone Name",
        "    East Zone Surface Group, !- Surface Name or Radiant Surface Group Name",
        "    heatingdesigncapacity,   !- Heating Design Capacity Method",
        "    100,                     !- Heating Design Capacity{ W }",
        "    ,                        !- Heating Design Capacity Per Floor Area{ W/m2 }",
        "    1.0,                     !- Fraction of Autosized Heating Design Capacity",
        "    MeanAirTemperature,      !- Temperature Control Type",
        "    HalfFlowPower,           !- Setpoint Type",
        "    2.0,                     !- Heating Throttling Range {deltaC}",
        "    Radiant Heating Setpoints;  !- Heating Control Temperature Schedule Name",

        "  ZoneHVAC:LowTemperatureRadiant:SurfaceGroup,",
        "    East Zone Surface Group, !- Name",
        "    Zn002:Flr001,             !- Surface 1 Name",
        "     0.5,                     !- Flow Fraction for Surface 1",
        "    Zn002:Flr002,             !- Surface 2 Name",
        "     0.5;                     !- Flow Fraction for Surface 2",

        "  ZoneHVAC:LowTemperatureRadiant:SurfaceGroup,",
        "    West Zone Surface Group, !- Name",
        "    Zn001:Flr001,             !- Surface 1 Name",
        "     0.5,                     !- Flow Fraction for Surface 1",
        "    Zn001:Flr002,             !- Surface 2 Name",
        "     0.5;                     !- Flow Fraction for Surface 2",

        "  ZoneHVAC:LowTemperatureRadiant:SurfaceGroup,",
        "    South Zone Surface Group, !- Name",
        "    Zn003:Flr001,             !- Surface 1 Name",
        "     0.5,                     !- Flow Fraction for Surface 1",
        "    Zn003:Flr002,             !- Surface 2 Name",
        "     0.5;                     !- Flow Fraction for Surface 2",

        "  Schedule:Compact,",
        "    RADIANTSYSAVAILSCHED,    !- Name",
        "    FRACTION,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,1.00;       !- Field 3",

        "  Schedule:Compact,",
        "    Radiant Heating Setpoints,   !- Name",
        "    TEMPERATURE,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,20.0;       !- Field 3",

        "  Schedule:Compact,",
        "    Radiant Cooling Setpoints,   !- Name",
        "    TEMPERATURE,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,15.0;       !- Field 3",

    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataHeatBal->Zone.allocate(3);
    state->dataHeatBal->Zone(1).Name = "WEST ZONE";
    state->dataHeatBal->Zone(2).Name = "EAST ZONE";
    state->dataHeatBal->Zone(3).Name = "SOUTH ZONE";

    state->dataSurface->TotSurfaces = 6;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);
    state->dataSurface->Surface(1).Name = "ZN001:FLR001";
    state->dataSurface->Surface(1).ZoneName = "WEST ZONE";
    state->dataSurface->Surface(1).Zone = 1;
    state->dataSurface->Surface(1).Construction = 1;
    state->dataSurface->Surface(2).Name = "ZN001:FLR002";
    state->dataSurface->Surface(2).ZoneName = "WEST ZONE";
    state->dataSurface->Surface(2).Zone = 1;
    state->dataSurface->Surface(2).Construction = 1;
    state->dataSurface->Surface(3).Name = "ZN002:FLR001";
    state->dataSurface->Surface(3).ZoneName = "EAST ZONE";
    state->dataSurface->Surface(3).Zone = 2;
    state->dataSurface->Surface(3).Construction = 1;
    state->dataSurface->Surface(4).Name = "ZN002:FLR002";
    state->dataSurface->Surface(4).ZoneName = "EAST ZONE";
    state->dataSurface->Surface(4).Zone = 2;
    state->dataSurface->Surface(4).Construction = 1;
    state->dataSurface->Surface(5).Name = "ZN003:FLR001";
    state->dataSurface->Surface(5).ZoneName = "SOUTH ZONE";
    state->dataSurface->Surface(5).Zone = 3;
    state->dataSurface->Surface(5).Construction = 1;
    state->dataSurface->Surface(6).Name = "ZN003:FLR002";
    state->dataSurface->Surface(6).ZoneName = "SOUTH ZONE";
    state->dataSurface->Surface(6).Zone = 3;
    state->dataSurface->Surface(6).Construction = 1;
    state->dataSurface->surfIntConv.allocate(state->dataSurface->TotSurfaces);
    std::fill(state->dataSurface->surfIntConv.begin(), state->dataSurface->surfIntConv.begin(), DataSurfaces::SurfIntConv());
    state->dataConstruction->Construct.allocate(1);
    state->dataConstruction->Construct(1).SourceSinkPresent = true;
    state->dataSurface->SurfIsRadSurfOrVentSlabOrPool.allocate(state->dataSurface->TotSurfaces);
    state->dataSurface->SurfIsRadSurfOrVentSlabOrPool = false;

    GetLowTempRadiantSystem(*state);

    EXPECT_EQ(3, state->dataLowTempRadSys->TotalNumOfRadSystems);

    EXPECT_EQ(1, state->dataLowTempRadSys->NumOfHydrLowTempRadSys);
    EXPECT_EQ(1, state->dataLowTempRadSys->NumOfHydrLowTempRadSysDes);

    EXPECT_EQ(1, state->dataLowTempRadSys->NumOfCFloLowTempRadSys);
    EXPECT_EQ(1, state->dataLowTempRadSys->NumOfCFloLowTempRadSysDes);

    EXPECT_EQ(1, state->dataLowTempRadSys->NumOfElecLowTempRadSys);

    EXPECT_EQ("WEST ZONE RADIANT FLOOR", state->dataLowTempRadSys->RadSysTypes(1).Name);
    EXPECT_ENUM_EQ(SystemType::Hydronic, state->dataLowTempRadSys->RadSysTypes(1).systemType);
    EXPECT_EQ(state->dataLowTempRadSys->HydrRadSys(1).ZoneName, "WEST ZONE");
    EXPECT_EQ(state->dataLowTempRadSys->HydrRadSys(1).SurfListName, "WEST ZONE SURFACE GROUP");
    EXPECT_EQ("WEST ZONE RADIANT FLOOR DESIGN", state->dataLowTempRadSys->HydrRadSys(1).designObjectName);
    EXPECT_EQ(1, state->dataLowTempRadSys->HydrRadSys(1).DesignObjectPtr);

    EXPECT_EQ("SOUTH ZONE LOWTEMPRAD", state->dataLowTempRadSys->RadSysTypes(2).Name);
    EXPECT_ENUM_EQ(SystemType::ConstantFlow, state->dataLowTempRadSys->RadSysTypes(2).systemType);
    EXPECT_EQ(state->dataLowTempRadSys->CFloRadSys(1).ZoneName, "SOUTH ZONE");
    EXPECT_EQ(state->dataLowTempRadSys->CFloRadSys(1).SurfListName, "SOUTH ZONE SURFACE GROUP");
    EXPECT_EQ("SOUTH ZONE LOWTEMPRAD DESIGN", state->dataLowTempRadSys->CFloRadSys(1).designObjectName);
    EXPECT_EQ(1, state->dataLowTempRadSys->CFloRadSys(1).DesignObjectPtr);

    EXPECT_EQ("EAST ZONE RADIANT FLOOR", state->dataLowTempRadSys->RadSysTypes(3).Name);
    EXPECT_ENUM_EQ(SystemType::Electric, state->dataLowTempRadSys->RadSysTypes(3).systemType);
    EXPECT_EQ(state->dataLowTempRadSys->ElecRadSys(1).ZoneName, "EAST ZONE");
    EXPECT_EQ(state->dataLowTempRadSys->ElecRadSys(1).SurfListName, "EAST ZONE SURFACE GROUP");
}

TEST_F(LowTempRadiantSystemTest, VariableFlowCoolingOnlyInputTest)
{

    bool ErrorsFound = false;
    std::string const idf_objects = delimited_string({

        "  Building,",
        "    NONE,                    !- Name",
        "    0.0000000E+00,           !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    3.9999999E-02,           !- Loads Convergence Tolerance Value",
        "    0.4000000,               !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullInteriorAndExterior, !- Solar Distribution",
        "    25,                      !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",

        "  ScheduleTypeLimits,Fraction, 0.0 , 1.0 ,CONTINUOUS;",
        "  ScheduleTypeLimits,Temperature,-60,200,CONTINUOUS;",

        "  Zone,",
        "    West Zone,               !- Name",
        "    0.0000000E+00,           !- Direction of Relative North {deg}",
        "    0.0000000E+00,           !- X Origin {m}",
        "    0.0000000E+00,           !- Y Origin {m}",
        "    0.0000000E+00,           !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.5,                     !- Ceiling Height {m}",
        "    autocalculate;              !- Volume {m3}",

        "  ZoneHVAC:EquipmentConnections,",
        "    West Zone,               !- Zone Name",
        "    Zone1Equipment,          !- Zone Conditioning Equipment List Name",
        "    Zone1Inlets,             !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    Zone 1 Node,             !- Zone Air Node Name",
        "    Zone 1 Outlet Node;      !- Zone Return Air Node Name",

        "  ZoneHVAC:EquipmentList,",
        "    Zone1Equipment,          !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:LowTemperatureRadiant:VariableFlow,  !- Zone Equipment 1 Object Type",
        "    West Zone Radiant Floor, !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "  ZoneHVAC:LowTemperatureRadiant:VariableFlow,",
        "    West Zone Radiant Floor, !- Name",
        "    West Zone Radiant Floor Design, !- Design Object Name",
        "    RadiantSysAvailSched,    !- Availability Schedule Name",
        "    West Zone,               !- Zone Name",
        "    Zn001:Flr001,            !- Surface Name or Radiant Surface Group Name",
        "    autosize,                !- Hydronic Tubing Length {m}",
        "    ,                        !- Heating Design Capacity {W}",
        "    ,                        !- Maximum Hot Water Flow {m3/s}",
        "    ,                        !- Heating Water Inlet Node Name",
        "    ,                        !- Heating Water Outlet Node Name",
        "    ,                        !- Cooling Design Capacity {W}",
        "    autosize,                !- Maximum Cold Water Flow {m3/s}",
        "    Zone 1 Cooling Water Inlet Node,     !- Cooling Water Inlet Node Name",
        "    Zone 1 Cooling Water Outlet Node,    !- Cooling Water Outlet Node Name",
        "    ,                        !- Number of Circuits",
        "    ;                        !- Circuit Length {m}",

        "  ZoneHVAC:LowTemperatureRadiant:VariableFlow:Design,",
        "    West Zone Radiant Floor Design, !- Name",
        "    ConvectionOnly,          !- Fluid to Radiant Surface Heat Transfer Model",
        "    0.012,                   !- Hydronic Tubing Inside Diameter {m}",
        "    0.016,                   !- Hydronic Tubing Outside Diameter {m}",
        "    0.35,                    !- Hydronic Tubing Conductivity {W/m-K}",
        "    MeanAirTemperature,      !- Temperature Control Type",
        "    HalfFlowPower,           !- Setpoint Type",
        "    FractionOfAutosizedHeatingCapacity,  !- Heating Design Capacity Method",
        "    ,                        !- Heating Design Capacity Per Floor Area {W/m2}",
        "    0.9,                     !- Fraction of Autosized Heating Design Capacity",
        "    2.0,                     !- Heating Control Throttling Range {deltaC}",
        "    Radiant Heating Setpoints,  !- Heating Control Temperature Schedule Name",
        "    CapacityPerFloorArea,    !- Cooling Design Capacity Method",
        "    10.0,                        !- Cooling Design Capacity Per Floor Area {W/m2}",
        "    ,                        !- Fraction of Autosized Cooling Design Capacity",
        "    2.0,                     !- Cooling Control Throttling Range {deltaC}",
        "    Radiant Cooling Setpoints,           !- Cooling Control Temperature Schedule Name",
        "    ,                        !- Condensation Control Type",
        "    ;                        !- Condensation Control Dewpoint Offset {C}",

        "  Site:GroundTemperature:BuildingSurface,20.03,20.03,20.13,20.30,20.43,20.52,20.62,20.77,20.78,20.55,20.44,20.20;",

        "  BuildingSurface:Detailed,",
        "    Zn001:Flr001,            !- Name",
        "    Floor,                   !- Surface Type",
        "    Slab Floor with Radiant, !- Construction Name",
        "    West Zone,               !- Zone Name",
        "    ,                        !- Space Name",
        "    Adiabatic,                  !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    1.000000,                !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0, 0.0, 0.0,           !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0, 6.0,0.0,       !- X,Y,Z ==> Vertex 2 {m}",
        "    6.0,6.0,0.0,   !- X,Y,Z ==> Vertex 3 {m}",
        "    6.0, 0.0, 0.0;      !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall001,            !- Name",
        "    Wall,                   !- Surface Type",
        "    Slab Floor with Radiant, !- Construction Name",
        "    West Zone,               !- Zone Name",
        "    ,                        !- Space Name",
        "    Adiabatic,                  !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0, 0.0, 6.0,           !- X,Y,Z ==> Vertex 1 {m}",
        "    6.0, 0.0, 6.0,       !- X,Y,Z ==> Vertex 2 {m}",
        "    6.0, 0.0,0.0,   !- X,Y,Z ==> Vertex 3 {m}",
        "    0.0, 0.0, 0.0;      !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall002,            !- Name",
        "    Wall,                   !- Surface Type",
        "    Slab, !- Construction Name",
        "    West Zone,               !- Zone Name",
        "    ,                        !- Space Name",
        "    Adiabatic,                  !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0, 6.0, 0.0,           !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0, 6.0, 6.0,       !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0, 0.0, 6.0,   !- X,Y,Z ==> Vertex 3 {m}",
        "    0.0, 0.0, 0.0;      !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall003,            !- Name",
        "    Wall,                   !- Surface Type",
        "    Slab, !- Construction Name",
        "    West Zone,               !- Zone Name",
        "    ,                        !- Space Name",
        "    Adiabatic,                  !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    6.0, 6.0, 0.0,           !- X,Y,Z ==> Vertex 1 {m}",
        "    6.0, 6.0, 6.0,       !- X,Y,Z ==> Vertex 2 {m}",
        "    6.0, 0.0, 6.0,   !- X,Y,Z ==> Vertex 3 {m}",
        "    6.0, 0.0, 0.0;      !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall004,            !- Name",
        "    Wall,                   !- Surface Type",
        "    Slab, !- Construction Name",
        "    West Zone,               !- Zone Name",
        "    ,                        !- Space Name",
        "    Adiabatic,                  !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0, 6.0, 0.0,           !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0, 6.0, 6.0,       !- X,Y,Z ==> Vertex 2 {m}",
        "    6.0, 6.0, 6.0,   !- X,Y,Z ==> Vertex 3 {m}",
        "    6.0, 6.0, 0.0;      !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:C001,            !- Name",
        "    CEILING,                   !- Surface Type",
        "    Slab, !- Construction Name",
        "    West Zone,               !- Zone Name",
        "    ,                        !- Space Name",
        "    Adiabatic,                  !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    ,                !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0, 0.0, 6.0,           !- X,Y,Z ==> Vertex 1 {m}",
        "    6.0, 0.0, 6.0,       !- X,Y,Z ==> Vertex 2 {m}",
        "    6.0, 6.0, 6.0,   !- X,Y,Z ==> Vertex 3 {m}",
        "    0.0, 6.0, 6.0;      !- X,Y,Z ==> Vertex 4 {m}",

        "  ConstructionProperty:InternalHeatSource,",
        "    Radiant Source,          !- Name",
        "    Slab Floor with Radiant, !- Construction Name",
        "    3,                       !- Source Present After Layer Number",
        "    3,                       !- Temperature Calculation Requested After Layer Number",
        "    1,                       !- Dimensions for the CTF Calculation",
        "    0.1524,                  !- Tube Spacing {m}",
        "    0.0;                     !- Two-Dimensional Position of Interior Temperature Calculation Request",

        "  Construction,",
        "    Slab Floor with Radiant, !- Name",
        "    CONCRETE - DRIED SAND AND GRAVEL 4 IN,  !- Outside Layer",
        "    INS - EXPANDED EXT POLYSTYRENE R12 2 IN,  !- Layer 2",
        "    GYP1,                    !- Layer 3",
        "    GYP2,                    !- Layer 4",
        "    FINISH FLOORING - TILE 1 / 16 IN;  !- Layer 5",

        "Construction,",
        "Slab,              !- Name",
        "CONCRETE - DRIED SAND AND GRAVEL 4 IN,                    !- Outside Layer",
        "GYP1,                    !- Layer 2",
        "FINISH FLOORING - TILE 1 / 16 IN;                    !- Layer 3",

        "  Material,",
        "    CONCRETE - DRIED SAND AND GRAVEL 4 IN,  !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1000000,               !- Thickness {m}",
        "    1.290000,                !- Conductivity {W/m-K}",
        "    2242.580,                !- Density {kg/m3}",
        "    830.00000,               !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6000000,               !- Solar Absorptance",
        "    0.6000000;               !- Visible Absorptance",

        "  Material,",
        "    INS - EXPANDED EXT POLYSTYRENE R12 2 IN,  !- Name",
        "    Rough,                   !- Roughness",
        "    5.0000001E-02,           !- Thickness {m}",
        "    2.0000000E-02,           !- Conductivity {W/m-K}",
        "    56.06000,                !- Density {kg/m3}",
        "    1210.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.5000000,               !- Solar Absorptance",
        "    0.5000000;               !- Visible Absorptance",

        "  Material,",
        "    GYP1,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    1.2700000E-02,           !- Thickness {m}",
        "    7.8450000E-01,           !- Conductivity {W/m-K}",
        "    1842.1221,               !- Density {kg/m3}",
        "    988.000,                 !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.5000000,               !- Solar Absorptance",
        "    0.5000000;               !- Visible Absorptance",

        "  Material,",
        "    GYP2,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    1.9050000E-02,           !- Thickness {m}",
        "    7.8450000E-01,           !- Conductivity {W/m-K}",
        "    1842.1221,               !- Density {kg/m3}",
        "    988.000,                 !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.5000000,               !- Solar Absorptance",
        "    0.5000000;               !- Visible Absorptance",

        "  Material,",
        "    FINISH FLOORING - TILE 1 / 16 IN,  !- Name",
        "    Smooth,                  !- Roughness",
        "    1.6000000E-03,           !- Thickness {m}",
        "    0.1700000,               !- Conductivity {W/m-K}",
        "    1922.210,                !- Density {kg/m3}",
        "    1250.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.5000000,               !- Solar Absorptance",
        "    0.5000000;               !- Visible Absorptance",

        "  Schedule:Compact,",
        "    RADIANTSYSAVAILSCHED,    !- Name",
        "    FRACTION,                !- Schedule Type Limits Name",
        "    Through: 3/31,           !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,1.00,       !- Field 3",
        "    Through: 9/30,           !- Field 5",
        "    For: Alldays,            !- Field 6",
        "    Until: 24:00,0.00,       !- Field 7",
        "    Through: 12/31,          !- Field 9",
        "    For: Alldays,            !- Field 10",
        "    Until: 24:00,1.00;       !- Field 11",

        "  Schedule:Compact,",
        "    HW LOOP TEMP SCHEDULE,   !- Name",
        "    TEMPERATURE,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,60.00;      !- Field 3",

        "  Schedule:Compact,",
        "    RADIANT HEATING SETPOINTS,  !- Name",
        "    TEMPERATURE,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 7:00,12.00,       !- Field 3",
        "    Until: 17:00,17.00,      !- Field 5",
        "    Until: 24:00,12.00;      !- Field 7",

        "  Sizing:Plant,",
        "    Hot Water Loop,          !- Plant or Condenser Loop Name",
        "    heating,                 !- Loop Type",
        "    60.,                     !- Design Loop Exit Temperature {C}",
        "    10;                      !- Loop Design Temperature Difference {deltaC}",

        "  PlantLoop,",
        "    Hot Water Loop,          !- Name",
        "    Water,                   !- Fluid Type",
        "    ,                        !- User Defined Fluid Type",
        "    Hot Loop Operation,      !- Plant Equipment Operation Scheme Name",
        "    HW Supply Outlet Node,   !- Loop Temperature Setpoint Node Name",
        "    100,                     !- Maximum Loop Temperature {C}",
        "    10,                      !- Minimum Loop Temperature {C}",
        "    0.0043,                  !- Maximum Loop Flow Rate {m3/s}",
        "    0,                       !- Minimum Loop Flow Rate {m3/s}",
        "    autocalculate,           !- Plant Loop Volume {m3}",
        "    HW Supply Inlet Node,    !- Plant Side Inlet Node Name",
        "    HW Supply Outlet Node,   !- Plant Side Outlet Node Name",
        "    Heating Supply Side Branches,  !- Plant Side Branch List Name",
        "    Heating Supply Side Connectors,  !- Plant Side Connector List Name",
        "    HW Demand Inlet Node,    !- Demand Side Inlet Node Name",
        "    HW Demand Outlet Node,   !- Demand Side Outlet Node Name",
        "    Heating Demand Side Branches,  !- Demand Side Branch List Name",
        "    Heating Demand Side Connectors,  !- Demand Side Connector List Name",
        "    Optimal,                 !- Load Distribution Scheme",
        "    ,                        !- Availability Manager List Name",
        "    ,                        !- Plant Loop Demand Calculation Scheme",
        "    ,                        !- Common Pipe Simulation",
        "    ,                        !- Pressure Simulation Type",
        "    2.0;                     !- Loop Circulation Time {minutes}",

        "  SetpointManager:Scheduled,",
        "    Hot Water Loop Setpoint Manager,  !- Name",
        "    Temperature,             !- Control Variable",
        "    HW Loop Temp Schedule,   !- Schedule Name",
        "    Hot Water Loop Setpoint Node List;  !- Setpoint Node or NodeList Name",

        "  NodeList,",
        "    Hot Water Loop Setpoint Node List,  !- Name",
        "    HW Supply Outlet Node;   !- Node 1 Name",

        "  BranchList,",
        "    Heating Supply Side Branches,  !- Name",
        "    Heating Supply Inlet Branch,  !- Branch 1 Name",
        "    Heating Purchased Hot Water Branch,  !- Branch 2 Name",
        "    Heating Supply Bypass Branch,  !- Branch 3 Name",
        "    Heating Supply Outlet Branch;  !- Branch 4 Name",

        "  ConnectorList,",
        "    Heating Supply Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    Heating Supply Splitter, !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    Heating Supply Mixer;    !- Connector 2 Name",

        "  Branch,",
        "    Heating Supply Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:VariableSpeed,      !- Component 1 Object Type",
        "    HW Circ Pump,            !- Component 1 Name",
        "    HW Supply Inlet Node,    !- Component 1 Inlet Node Name",
        "    HW Pump Outlet Node;     !- Component 1 Outlet Node Name",

        "  Branch,",
        "    Heating Purchased Hot Water Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    DistrictHeating:Water,         !- Component 1 Object Type",
        "    Purchased Heating,       !- Component 1 Name",
        "    Purchased Heat Inlet Node,  !- Component 1 Inlet Node Name",
        "    Purchased Heat Outlet Node;  !- Component 1 Outlet Node Name",

        "  Branch,",
        "    Heating Supply Bypass Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Heating Supply Side Bypass,  !- Component 1 Name",
        "    Heating Supply Bypass Inlet Node,  !- Component 1 Inlet Node Name",
        "    Heating Supply Bypass Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Heating Supply Side Bypass,  !- Name",
        "    Heating Supply Bypass Inlet Node,  !- Inlet Node Name",
        "    Heating Supply Bypass Outlet Node;  !- Outlet Node Name",

        "  Branch,",
        "    Heating Supply Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Heating Supply Outlet,   !- Component 1 Name",
        "    Heating Supply Exit Pipe Inlet Node,  !- Component 1 Inlet Node Name",
        "    HW Supply Outlet Node;   !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Heating Supply Outlet,   !- Name",
        "    Heating Supply Exit Pipe Inlet Node,  !- Inlet Node Name",
        "    HW Supply Outlet Node;   !- Outlet Node Name",

        "  BranchList,",
        "    Heating Demand Side Branches,  !- Name",
        "    Reheat Inlet Branch,     !- Branch 1 Name",
        "    Reheat Bypass Branch,    !- Branch 8 Name",
        "    Reheat Outlet Branch;    !- Branch 9 Name",

        "  ConnectorList,",
        "    Heating Demand Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    Reheat Splitter,         !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    Reheat Mixer;            !- Connector 2 Name",

        "  Branch,",
        "    Reheat Inlet Branch,     !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Reheat Inlet Pipe,       !- Component 1 Name",
        "    HW Demand Inlet Node,    !- Component 1 Inlet Node Name",
        "    HW Demand Entrance Pipe Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Reheat Inlet Pipe,       !- Name",
        "    HW Demand Inlet Node,    !- Inlet Node Name",
        "    HW Demand Entrance Pipe Outlet Node;  !- Outlet Node Name",

        "  Branch,",
        "    Reheat Outlet Branch,    !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Reheat Outlet Pipe,      !- Component 1 Name",
        "    HW Demand Exit Pipe Inlet Node,  !- Component 1 Inlet Node Name",
        "    HW Demand Outlet Node;   !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Reheat Outlet Pipe,      !- Name",
        "    HW Demand Exit Pipe Inlet Node,  !- Inlet Node Name",
        "    HW Demand Outlet Node;   !- Outlet Node Name",

        "  Branch,",
        "    Reheat Bypass Branch,    !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Reheat Bypass,           !- Component 1 Name",
        "    Reheat Bypass Inlet Node,!- Component 1 Inlet Node Name",
        "    Reheat Bypass Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Reheat Bypass,           !- Name",
        "    Reheat Bypass Inlet Node,!- Inlet Node Name",
        "    Reheat Bypass Outlet Node;  !- Outlet Node Name",

        "  Connector:Splitter,",
        "    Reheat Splitter,         !- Name",
        "    Reheat Inlet Branch,     !- Inlet Branch Name",
        "    Reheat Bypass Branch;    !- Outlet Branch 7 Name",

        "  Connector:Mixer,",
        "    Reheat Mixer,            !- Name",
        "    Reheat Outlet Branch,    !- Outlet Branch Name",
        "    Reheat Bypass Branch;    !- Inlet Branch 7 Name",

        "  Connector:Splitter,",
        "    Heating Supply Splitter, !- Name",
        "    Heating Supply Inlet Branch,  !- Inlet Branch Name",
        "    Heating Purchased Hot Water Branch,  !- Outlet Branch 1 Name",
        "    Heating Supply Bypass Branch;  !- Outlet Branch 2 Name",

        "  Connector:Mixer,",
        "    Heating Supply Mixer,    !- Name",
        "    Heating Supply Outlet Branch,  !- Outlet Branch Name",
        "    Heating Purchased Hot Water Branch,  !- Inlet Branch 1 Name",
        "    Heating Supply Bypass Branch;  !- Inlet Branch 2 Name",

        "  PlantEquipmentOperationSchemes,",
        "    Hot Loop Operation,      !- Name",
        "    PlantEquipmentOperation:HeatingLoad,  !- Control Scheme 1 Object Type",
        "    Purchased Only,          !- Control Scheme 1 Name",
        "    ON;                      !- Control Scheme 1 Schedule Name",

        "  PlantEquipmentOperation:HeatingLoad,",
        "    Purchased Only,          !- Name",
        "    0,                       !- Load Range 1 Lower Limit {W}",
        "    1000000,                 !- Load Range 1 Upper Limit {W}",
        "    heating plant;           !- Range 1 Equipment List Name",

        "  PlantEquipmentList,",
        "    heating plant,           !- Name",
        "    DistrictHeating:Water,         !- Equipment 1 Object Type",
        "    Purchased Heating;       !- Equipment 1 Name",

        "  Pump:VariableSpeed,",
        "    HW Circ Pump,            !- Name",
        "    HW Supply Inlet Node,    !- Inlet Node Name",
        "    HW Pump Outlet Node,     !- Outlet Node Name",
        "    .0043,                   !- Rated Flow Rate {m3/s}",
        "    300000,                  !- Rated Pump Head {Pa}",
        "    2000,                    !- Rated Power Consumption {W}",
        "    .87,                     !- Motor Efficiency",
        "    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "    1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 3 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 4 of the Part Load Performance Curve",
        "    0,                       !- Minimum Flow Rate {m3/s}",
        "    INTERMITTENT;            !- Pump Control Type",

        "  DistrictHeating:Water,",
        "    Purchased Heating,       !- Name",
        "    Purchased Heat Inlet Node,  !- Hot Water Inlet Node Name",
        "    Purchased Heat Outlet Node,  !- Hot Water Outlet Node Name",
        "    1000000;                 !- Nominal Capacity {W}",

        "  Sizing:Plant,",
        "    Chilled Water Loop,      !- Plant or Condenser Loop Name",
        "    cooling,                 !- Loop Type",
        "    6.7,                     !- Design Loop Exit Temperature {C}",
        "    2;                       !- Loop Design Temperature Difference {deltaC}",

        "  PlantLoop,",
        "    Chilled Water Loop,      !- Name",
        "    Water,                   !- Fluid Type",
        "    ,                        !- User Defined Fluid Type",
        "    CW Loop Operation,       !- Plant Equipment Operation Scheme Name",
        "    CW Supply Outlet Node,   !- Loop Temperature Setpoint Node Name",
        "    98,                      !- Maximum Loop Temperature {C}",
        "    1,                       !- Minimum Loop Temperature {C}",
        "    0.0011,                  !- Maximum Loop Flow Rate {m3/s}",
        "    0,                       !- Minimum Loop Flow Rate {m3/s}",
        "    autocalculate,           !- Plant Loop Volume {m3}",
        "    CW Supply Inlet Node,    !- Plant Side Inlet Node Name",
        "    CW Supply Outlet Node,   !- Plant Side Outlet Node Name",
        "    Cooling Supply Side Branches,  !- Plant Side Branch List Name",
        "    Cooling Supply Side Connectors,  !- Plant Side Connector List Name",
        "    CW Demand Inlet Node,    !- Demand Side Inlet Node Name",
        "    CW Demand Outlet Node,   !- Demand Side Outlet Node Name",
        "    Cooling Demand Side Branches,  !- Demand Side Branch List Name",
        "    Cooling Demand Side Connectors,  !- Demand Side Connector List Name",
        "    Optimal,                 !- Load Distribution Scheme",
        "    ,                        !- Availability Manager List Name",
        "    ,                        !- Plant Loop Demand Calculation Scheme",
        "    ,                        !- Common Pipe Simulation",
        "    ,                        !- Pressure Simulation Type",
        "    2.0;                     !- Loop Circulation Time {minutes}",

        "  SetpointManager:Scheduled,",
        "    Chilled Water Loop Setpoint Manager,  !- Name",
        "    Temperature,             !- Control Variable",
        "    CW Loop Temp Schedule,   !- Schedule Name",
        "    Chilled Water Loop Setpoint Node List;  !- Setpoint Node or NodeList Name",

        "    Schedule:Compact,",
        "     CW LOOP TEMP SCHEDULE, !- Name",
        "     TEMPERATURE,           !- Schedule Type Limits Name",
        "     Through: 12/31,        !- Field 1",
        "     For: Alldays,          !- Field 2",
        "     Until: 24:00, 10.0;    !- Field 3",

        "  NodeList,",
        "    Chilled Water Loop Setpoint Node List,  !- Name",
        "    CW Supply Outlet Node;   !- Node 1 Name",

        "  BranchList,",
        "    Cooling Supply Side Branches,  !- Name",
        "    CW Pump Branch,          !- Branch 1 Name",
        "    Purchased Cooling Branch,!- Branch 4 Name",
        "    Supply Bypass Branch,    !- Branch 5 Name",
        "    Cooling Supply Outlet;   !- Branch 6 Name",

        "  BranchList,",
        "    Cooling Demand Side Branches,  !- Name",
        "    Cooling Demand Inlet,    !- Branch 1 Name",
        "    Zone 1 Cooling Branch,   !- Branch 2 Name",
        "    Demand Bypass Branch,    !- Branch 3 Name",
        "    Cooling Demand Outlet;   !- Branch 4 Name",

        "  ConnectorList,",
        "    Cooling Supply Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    CW Loop Splitter,        !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    CW Loop Mixer;           !- Connector 2 Name",

        "  ConnectorList,",
        "    Cooling Demand Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    CW Demand Splitter,      !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    CW Demand Mixer;         !- Connector 2 Name",

        "  Branch,",
        "    Cooling Demand Inlet,    !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Demand Side Inlet Pipe,  !- Component 1 Name",
        "    CW Demand Inlet Node,    !- Component 1 Inlet Node Name",
        "    CW Demand Entrance Pipe Outlet Node;  !- Component 1 Outlet Node Name",

        "  Schedule:Compact,",
        "    RADIANT COOLING SETPOINTS,  !- Name",
        "    TEMPERATURE,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,26.0;       !- Field 3",

        "  Pipe:Adiabatic,",
        "    Demand Side Inlet Pipe,  !- Name",
        "    CW Demand Inlet Node,    !- Inlet Node Name",
        "    CW Demand Entrance Pipe Outlet Node;  !- Outlet Node Name",

        "  Branch,",
        "    Zone 1 Cooling Branch,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    ZoneHVAC:LowTemperatureRadiant:VariableFlow,  !- Component 1 Object Type",
        "    West Zone Radiant Floor,   !- Component 1 Name",
        "    Zone 1 Cooling Water Inlet Node,  !- Component 1 Inlet Node Name",
        "    Zone 1 Cooling Water Outlet Node; !- Component 1 Outlet Node Name",

        "  Branch,",
        "    Demand Bypass Branch,    !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Demand Side Bypass,      !- Component 1 Name",
        "    CW Demand Bypass Inlet Node,  !- Component 1 Inlet Node Name",
        "    CW Demand Bypass Outlet Node;  !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Demand Side Bypass,      !- Name",
        "    CW Demand Bypass Inlet Node,  !- Inlet Node Name",
        "    CW Demand Bypass Outlet Node;  !- Outlet Node Name",

        "  Branch,",
        "    Cooling Demand Outlet,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    CW Demand Side Outlet Pipe,  !- Component 1 Name",
        "    CW Demand Exit Pipe Inlet Node,  !- Component 1 Inlet Node Name",
        "    CW Demand Outlet Node;   !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    CW Demand Side Outlet Pipe,  !- Name",
        "    CW Demand Exit Pipe Inlet Node,  !- Inlet Node Name",
        "    CW Demand Outlet Node;   !- Outlet Node Name",

        "  Branch,",
        "    Cooling Supply Outlet,   !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Supply Side Outlet Pipe, !- Component 1 Name",
        "    Supply Side Exit Pipe Inlet Node,  !- Component 1 Inlet Node Name",
        "    CW Supply Outlet Node;   !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Supply Side Outlet Pipe, !- Name",
        "    Supply Side Exit Pipe Inlet Node,  !- Inlet Node Name",
        "    CW Supply Outlet Node;   !- Outlet Node Name",

        "  Branch,",
        "    CW Pump Branch,          !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:VariableSpeed,      !- Component 1 Object Type",
        "    Circ Pump,               !- Component 1 Name",
        "    CW Supply Inlet Node,    !- Component 1 Inlet Node Name",
        "    CW Pump Outlet Node;     !- Component 1 Outlet Node Name",

        "  Branch,",
        "    Purchased Cooling Branch,!- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    DistrictCooling,         !- Component 1 Object Type",
        "    Purchased Cooling,       !- Component 1 Name",
        "    Purchased Cooling Inlet Node,  !- Component 1 Inlet Node Name",
        "    Purchased Cooling Outlet Node; !- Component 1 Outlet Node Name",

        "  Branch,",
        "    Supply Bypass Branch,    !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Supply Side Bypass,      !- Component 1 Name",
        "    CW Supply Bypass Inlet Node,  !- Component 1 Inlet Node Name",
        "    CW Supply Bypass Outlet Node; !- Component 1 Outlet Node Name",

        "  Pipe:Adiabatic,",
        "    Supply Side Bypass,      !- Name",
        "    CW Supply Bypass Inlet Node,  !- Inlet Node Name",
        "    CW Supply Bypass Outlet Node; !- Outlet Node Name",

        "  Connector:Splitter,",
        "    CW Loop Splitter,        !- Name",
        "    CW Pump Branch,          !- Inlet Branch Name",
        "    Purchased Cooling Branch,!- Outlet Branch 3 Name",
        "    Supply Bypass Branch;    !- Outlet Branch 4 Name",

        "  Connector:Mixer,",
        "    CW Loop Mixer,           !- Name",
        "    Cooling Supply Outlet,   !- Outlet Branch Name",
        "    Purchased Cooling Branch,!- Inlet Branch 3 Name",
        "    Supply Bypass Branch;    !- Inlet Branch 4 Name",

        "  Connector:Splitter,",
        "    CW Demand Splitter,      !- Name",
        "    Cooling Demand Inlet,    !- Inlet Branch Name",
        "    Demand Bypass Branch,    !- Outlet Branch 1 Name",
        "    Zone 1 Cooling Branch;   !- Outlet Branch 2 Name",

        "  Connector:Mixer,",
        "    CW Demand Mixer,         !- Name",
        "    Cooling Demand Outlet,   !- Outlet Branch Name",
        "    Zone 1 Cooling Branch,   !- Inlet Branch 1 Name",
        "    Demand Bypass Branch;    !- Inlet Branch 2 Name",

        "  PlantEquipmentOperationSchemes,",
        "    CW Loop Operation,       !- Name",
        "    PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type",
        "    Always Operation,        !- Control Scheme 1 Name",
        "    Always;                  !- Control Scheme 1 Schedule Name",

        "  Schedule:Compact,",
        "    Always,                  !- Name",
        "    FRACTION,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: Alldays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "  PlantEquipmentOperation:CoolingLoad,",
        "    Always Operation,        !- Name",
        "    0,                       !- Load Range 1 Lower Limit {W}",
        "    70000,                   !- Load Range 1 Upper Limit {W}",
        "    Purchased Only;          !- Range 3 Equipment List Name",

        "  PlantEquipmentList,",
        "    Purchased Only,          !- Name",
        "    DistrictCooling,         !- Equipment 1 Object Type",
        "    Purchased Cooling;       !- Equipment 1 Name",

        "  DistrictCooling,",
        "    Purchased Cooling,             !- Name",
        "    Purchased Cooling Inlet Node,  !- Chilled Water Inlet Node Name",
        "    Purchased Cooling Outlet Node, !- Chilled Water Outlet Node Name",
        "    680000;                        !- Nominal Capacity {W}",

        "  Pump:VariableSpeed,",
        "    Circ Pump,               !- Name",
        "    CW Supply Inlet Node,    !- Inlet Node Name",
        "    CW Pump Outlet Node,     !- Outlet Node Name",
        "    .0011,                   !- Rated Flow Rate {m3/s}",
        "    300000,                  !- Rated Pump Head {Pa}",
        "    500,                     !- Rated Power Consumption {W}",
        "    .87,                     !- Motor Efficiency",
        "    0.0,                     !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    0,                       !- Coefficient 1 of the Part Load Performance Curve",
        "    1,                       !- Coefficient 2 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 3 of the Part Load Performance Curve",
        "    0,                       !- Coefficient 4 of the Part Load Performance Curve",
        "    0,                       !- Minimum Flow Rate {m3/s}",
        "    INTERMITTENT;            !- Pump Control Type",

    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetProjectControlData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    EXPECT_EQ("WEST ZONE", state->dataHeatBal->Zone(1).Name);

    HeatBalanceManager::SetPreConstructionInputParameters(*state);
    Material::GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    GetPlantSizingInput(*state);
    GetPlantLoopData(*state);
    GetPlantInput(*state);
    SetupInitialPlantCallingOrder(*state);
    SetupBranchControlTypes(*state);
    state->dataSurface->WorldCoordSystem = true;
    GetSurfaceListsInputs(*state);
    ErrorsFound = false;
    SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    // This is the test: prior to the fix for 8885, in a cooling only variable flow low temp radiant
    // system, a severe error would be thrown because there was a bug in the reading of the cooling nodes.
    // Post-fix, there should be no warnings thrown.
    EXPECT_NO_THROW(GetLowTempRadiantSystem(*state));
    compare_err_stream("");
}

TEST_F(LowTempRadiantSystemTest, UpdateRadSysSourceValAvgTest)
{
    bool isItOn;
    Real64 errTol = 0.0001;

    auto &hRS = state->dataLowTempRadSys->HydrRadSys;
    auto &cRS = state->dataLowTempRadSys->CFloRadSys;
    auto &eRS = state->dataLowTempRadSys->ElecRadSys;
    auto &surf = state->dataSurface->Surface;
    auto &dataLTRS = state->dataLowTempRadSys;
    auto &totSurfaces = state->dataSurface->TotSurfaces;
    auto &qRadSysSource = state->dataHeatBalFanSys->QRadSysSource;

    dataLTRS->NumOfHydrLowTempRadSys = 2;
    dataLTRS->NumOfCFloLowTempRadSys = 1;
    dataLTRS->NumOfElecLowTempRadSys = 1;
    hRS.allocate(dataLTRS->NumOfHydrLowTempRadSys);
    cRS.allocate(dataLTRS->NumOfCFloLowTempRadSys);
    eRS.allocate(dataLTRS->NumOfElecLowTempRadSys);

    hRS(1).NumOfSurfaces = 1;
    hRS(1).SurfacePtr.allocate(hRS(1).NumOfSurfaces);
    hRS(1).QRadSysSrcAvg.allocate(hRS(1).NumOfSurfaces);
    hRS(2).NumOfSurfaces = 2;
    hRS(2).SurfacePtr.allocate(hRS(2).NumOfSurfaces);
    hRS(2).QRadSysSrcAvg.allocate(hRS(2).NumOfSurfaces);
    cRS(1).NumOfSurfaces = 1;
    cRS(1).SurfacePtr.allocate(cRS(1).NumOfSurfaces);
    cRS(1).QRadSysSrcAvg.allocate(cRS(1).NumOfSurfaces);
    eRS(1).NumOfSurfaces = 1;
    eRS(1).SurfacePtr.allocate(eRS(1).NumOfSurfaces);
    eRS(1).QRadSysSrcAvg.allocate(eRS(1).NumOfSurfaces);

    hRS(1).SurfacePtr(1) = 1;
    hRS(2).SurfacePtr(1) = 2;
    hRS(2).SurfacePtr(2) = 3;
    cRS(1).SurfacePtr(1) = 5;
    eRS(1).SurfacePtr(1) = 6;
    hRS(1).QRadSysSrcAvg(1) = 100.0;
    hRS(2).QRadSysSrcAvg(1) = 200.0;
    hRS(2).QRadSysSrcAvg(2) = 300.0;
    cRS(1).QRadSysSrcAvg(1) = 400.0;
    eRS(1).QRadSysSrcAvg(1) = 500.0;

    totSurfaces = 6;
    surf.allocate(totSurfaces);
    surf(1).ExtBoundCond = 0;
    surf(2).ExtBoundCond = 0;
    surf(3).ExtBoundCond = 4; // interzone surface test
    surf(4).ExtBoundCond = 3; // interzone surface test
    surf(5).ExtBoundCond = 0;
    surf(6).ExtBoundCond = 0;
    qRadSysSource.allocate(totSurfaces);

    // Test 1: No radiant systems--should come back with flag false and nothing set in QRadSysSource
    isItOn = false;
    dataLTRS->NumOfHydrLowTempRadSys = 0;
    dataLTRS->NumOfCFloLowTempRadSys = 0;
    dataLTRS->NumOfElecLowTempRadSys = 0;
    dataLTRS->TotalNumOfRadSystems = dataLTRS->NumOfHydrLowTempRadSys + dataLTRS->NumOfCFloLowTempRadSys + dataLTRS->NumOfElecLowTempRadSys;
    qRadSysSource = 0.0;
    UpdateRadSysSourceValAvg(*state, isItOn);
    EXPECT_FALSE(isItOn);
    for (int surfNum = 1; surfNum <= totSurfaces; ++surfNum) {
        EXPECT_NEAR(qRadSysSource(surfNum), 0.0, errTol);
    }

    // Test 2: Only Hydronic Radiant System--should come back with flag true and only hydronic variables set in QRadSysSource
    isItOn = false;
    dataLTRS->NumOfHydrLowTempRadSys = 2;
    dataLTRS->NumOfCFloLowTempRadSys = 0;
    dataLTRS->NumOfElecLowTempRadSys = 0;
    dataLTRS->TotalNumOfRadSystems = dataLTRS->NumOfHydrLowTempRadSys + dataLTRS->NumOfCFloLowTempRadSys + dataLTRS->NumOfElecLowTempRadSys;
    qRadSysSource = 0.0;
    UpdateRadSysSourceValAvg(*state, isItOn);
    EXPECT_TRUE(isItOn);
    EXPECT_NEAR(qRadSysSource(1), 100.0, errTol);
    EXPECT_NEAR(qRadSysSource(2), 200.0, errTol);
    EXPECT_NEAR(qRadSysSource(3), 300.0, errTol);
    EXPECT_NEAR(qRadSysSource(4), 300.0, errTol);
    EXPECT_NEAR(qRadSysSource(5), 0.0, errTol);
    EXPECT_NEAR(qRadSysSource(6), 0.0, errTol);

    // Test 3: Only Constant Flow Radiant System--should come back with flag true and only constant flow variables set in QRadSysSource
    isItOn = false;
    dataLTRS->NumOfHydrLowTempRadSys = 0;
    dataLTRS->NumOfCFloLowTempRadSys = 1;
    dataLTRS->NumOfElecLowTempRadSys = 0;
    dataLTRS->TotalNumOfRadSystems = dataLTRS->NumOfHydrLowTempRadSys + dataLTRS->NumOfCFloLowTempRadSys + dataLTRS->NumOfElecLowTempRadSys;
    qRadSysSource = 0.0;
    UpdateRadSysSourceValAvg(*state, isItOn);
    EXPECT_TRUE(isItOn);
    EXPECT_NEAR(qRadSysSource(1), 0.0, errTol);
    EXPECT_NEAR(qRadSysSource(2), 0.0, errTol);
    EXPECT_NEAR(qRadSysSource(3), 0.0, errTol);
    EXPECT_NEAR(qRadSysSource(4), 0.0, errTol);
    EXPECT_NEAR(qRadSysSource(5), 400.0, errTol);
    EXPECT_NEAR(qRadSysSource(6), 0.0, errTol);

    // Test 4: Only Electric Radiant System--should come back with flag true and only electric variables set in qRadSysSrc (QRadSysSource)
    isItOn = false;
    dataLTRS->NumOfHydrLowTempRadSys = 0;
    dataLTRS->NumOfCFloLowTempRadSys = 0;
    dataLTRS->NumOfElecLowTempRadSys = 1;
    dataLTRS->TotalNumOfRadSystems = dataLTRS->NumOfHydrLowTempRadSys + dataLTRS->NumOfCFloLowTempRadSys + dataLTRS->NumOfElecLowTempRadSys;
    qRadSysSource = 0.0;
    UpdateRadSysSourceValAvg(*state, isItOn);
    EXPECT_TRUE(isItOn);
    EXPECT_NEAR(qRadSysSource(1), 0.0, errTol);
    EXPECT_NEAR(qRadSysSource(2), 0.0, errTol);
    EXPECT_NEAR(qRadSysSource(3), 0.0, errTol);
    EXPECT_NEAR(qRadSysSource(4), 0.0, errTol);
    EXPECT_NEAR(qRadSysSource(5), 0.0, errTol);
    EXPECT_NEAR(qRadSysSource(6), 500.0, errTol);

    // Test 5: All Radiant System--should come back with flag true and all variables set in qRadSysSrc (QRadSysSource)
    isItOn = false;
    dataLTRS->NumOfHydrLowTempRadSys = 2;
    dataLTRS->NumOfCFloLowTempRadSys = 1;
    dataLTRS->NumOfElecLowTempRadSys = 1;
    dataLTRS->TotalNumOfRadSystems = dataLTRS->NumOfHydrLowTempRadSys + dataLTRS->NumOfCFloLowTempRadSys + dataLTRS->NumOfElecLowTempRadSys;
    qRadSysSource = 0.0;
    UpdateRadSysSourceValAvg(*state, isItOn);
    EXPECT_TRUE(isItOn);
    EXPECT_NEAR(qRadSysSource(1), 100.0, errTol);
    EXPECT_NEAR(qRadSysSource(2), 200.0, errTol);
    EXPECT_NEAR(qRadSysSource(3), 300.0, errTol);
    EXPECT_NEAR(qRadSysSource(4), 300.0, errTol);
    EXPECT_NEAR(qRadSysSource(5), 400.0, errTol);
    EXPECT_NEAR(qRadSysSource(6), 500.0, errTol);
}
