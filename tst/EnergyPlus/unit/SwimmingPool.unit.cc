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

// EnergyPlus::SwimmingPool Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/SwimmingPool.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::SwimmingPool;
using namespace EnergyPlus::DataSurfaces;
using namespace EnergyPlus::DataPlant;

TEST_F(EnergyPlusFixture, SwimmingPool_MakeUpWaterVolFlow)
{
    // Tests for MakeUpWaterVolFlowFunct
    EXPECT_EQ(0.05, MakeUpWaterVolFlowFunct(5, 100));
    EXPECT_NEAR(0.00392, MakeUpWaterVolFlowFunct(0.1, 25.5), .0001);
    EXPECT_EQ(-180, MakeUpWaterVolFlowFunct(-9, .05));
    EXPECT_NE(10, MakeUpWaterVolFlowFunct(10, 0.01));

    // Tests for MakeUpWaterVolFunct
    EXPECT_EQ(0.05, MakeUpWaterVolFunct(5, 100));
    EXPECT_NEAR(0.00392, MakeUpWaterVolFunct(0.1, 25.5), .0001);
    EXPECT_EQ(-180, MakeUpWaterVolFunct(-9, .05));
    EXPECT_NE(10, MakeUpWaterVolFunct(10, 0.01));
}

TEST_F(EnergyPlusFixture, SwimmingPool_CalcSwimmingPoolEvap)
{
    int SurfNum;
    int PoolNum;
    Real64 MAT;
    Real64 HumRat;
    Real64 EvapRate;

    // Tests for CalcSwimmingPoolEvap--Evaporate Rate Calculation for Swimming Pools
    state->dataSwimmingPools->NumSwimmingPools = 1;
    state->dataSwimmingPools->Pool.allocate(1);
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).Area = 10.0;
    SurfNum = 1;
    PoolNum = 1;
    state->dataEnvrn->OutBaroPress = 101400.0;

    auto &thisPool = state->dataSwimmingPools->Pool(PoolNum);

    // Test 1
    state->dataSwimmingPools->Pool(PoolNum).PoolWaterTemp = 30.0;
    MAT = 20.0;
    HumRat = 0.005;
    state->dataSwimmingPools->Pool(PoolNum).CurActivityFactor = 0.5;
    state->dataSwimmingPools->Pool(PoolNum).CurCoverEvapFac = 0.3;
    thisPool.calcSwimmingPoolEvap(*state, EvapRate, SurfNum, MAT, HumRat);
    EXPECT_NEAR(0.000207, EvapRate, 0.000001);
    EXPECT_NEAR(4250.0, state->dataSwimmingPools->Pool(PoolNum).SatPressPoolWaterTemp, 10.0);
    EXPECT_NEAR(810.0, state->dataSwimmingPools->Pool(PoolNum).PartPressZoneAirTemp, 10.0);

    // Test 2
    state->dataSwimmingPools->Pool(PoolNum).PoolWaterTemp = 27.0;
    MAT = 22.0;
    HumRat = 0.010;
    state->dataSwimmingPools->Pool(PoolNum).CurActivityFactor = 1.0;
    state->dataSwimmingPools->Pool(PoolNum).CurCoverEvapFac = 1.0;
    thisPool.calcSwimmingPoolEvap(*state, EvapRate, SurfNum, MAT, HumRat);
    EXPECT_NEAR(0.000788, EvapRate, 0.000001);
    EXPECT_NEAR(3570.0, state->dataSwimmingPools->Pool(PoolNum).SatPressPoolWaterTemp, 10.0);
    EXPECT_NEAR(1600.0, state->dataSwimmingPools->Pool(PoolNum).PartPressZoneAirTemp, 10.0);
}

TEST_F(EnergyPlusFixture, SwimmingPool_InitSwimmingPoolPlantLoopIndex)
{
    // Tests for InitSwimmingPoolPlantLoopIndex
    state->dataSwimmingPools->NumSwimmingPools = 2;
    state->dataPlnt->TotNumLoops = 2;
    state->dataSwimmingPools->Pool.allocate(state->dataSwimmingPools->NumSwimmingPools);
    state->dataSwimmingPools->Pool(1).Name = "FirstPool";
    state->dataSwimmingPools->Pool(2).Name = "SecondPool";
    state->dataSwimmingPools->Pool(1).WaterInletNode = 1;
    state->dataSwimmingPools->Pool(2).WaterInletNode = 11;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::SwimmingPool_Indoor;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = "FirstPool";
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = 1;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::SwimmingPool_Indoor;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).Name = "SecondPool";
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).NodeNumIn = 11;

    // Test 1
    state->dataSwimmingPools->Pool(1).initSwimmingPoolPlantLoopIndex(*state);
    EXPECT_EQ(state->dataSwimmingPools->Pool(1).HWplantLoc.loopNum, 1);
    EXPECT_ENUM_EQ(state->dataSwimmingPools->Pool(1).HWplantLoc.loopSideNum, DataPlant::LoopSideLocation::Demand);
    EXPECT_EQ(state->dataSwimmingPools->Pool(1).HWplantLoc.branchNum, 1);
    EXPECT_EQ(state->dataSwimmingPools->Pool(1).HWplantLoc.compNum, 1);

    // Test 2
    state->dataSwimmingPools->Pool(1).MyPlantScanFlagPool = true;
    state->dataSwimmingPools->Pool(2).initSwimmingPoolPlantLoopIndex(*state);
    EXPECT_EQ(state->dataSwimmingPools->Pool(2).HWplantLoc.loopNum, 2);
    EXPECT_ENUM_EQ(state->dataSwimmingPools->Pool(2).HWplantLoc.loopSideNum, DataPlant::LoopSideLocation::Supply);
    EXPECT_EQ(state->dataSwimmingPools->Pool(2).HWplantLoc.branchNum, 1);
    EXPECT_EQ(state->dataSwimmingPools->Pool(2).HWplantLoc.compNum, 1);
}

TEST_F(EnergyPlusFixture, SwimmingPool_InitSwimmingPoolPlantNodeFlow)
{
    int constexpr PoolNum = 1;

    // Tests for InitSwimmingPoolPlantLoopIndex
    state->dataSwimmingPools->NumSwimmingPools = 1;
    state->dataPlnt->TotNumLoops = 1;
    state->dataSwimmingPools->Pool.allocate(state->dataSwimmingPools->NumSwimmingPools);

    state->dataSwimmingPools->Pool(1).Name = "FirstPool";
    state->dataSwimmingPools->Pool(1).WaterInletNode = 1;
    state->dataSwimmingPools->Pool(1).WaterOutletNode = 2;
    state->dataSwimmingPools->Pool(1).HWplantLoc.loopNum = 1;
    state->dataSwimmingPools->Pool(1).HWplantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    state->dataSwimmingPools->Pool(1).HWplantLoc.branchNum = 1;
    state->dataSwimmingPools->Pool(1).HWplantLoc.compNum = 1;

    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::SwimmingPool_Indoor;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = "FirstPool";
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = 1;

    state->dataLoopNodes->Node.allocate(2);

    // Test 1
    auto &thisPool = state->dataSwimmingPools->Pool(PoolNum);

    state->dataSwimmingPools->Pool(1).WaterMassFlowRate = 0.75;
    state->dataSwimmingPools->Pool(1).WaterMassFlowRateMax = 0.75;
    state->dataSwimmingPools->Pool(1).WaterVolFlowMax = 0.00075;
    state->dataSwimmingPools->Pool(1).MyPlantScanFlagPool = false;
    state->dataSize->SaveNumPlantComps = 0;
    state->dataSize->CompDesWaterFlow.deallocate();
    state->dataLoopNodes->Node(1).MassFlowRate = 0.0;
    state->dataLoopNodes->Node(1).MassFlowRateMax = 0.0;
    thisPool.initSwimmingPoolPlantNodeFlow(*state);
    EXPECT_EQ(state->dataSize->CompDesWaterFlow(1).SupNode, 1);
    EXPECT_EQ(state->dataSize->CompDesWaterFlow(1).DesVolFlowRate, 0.00075);

    // Test 2
    state->dataSwimmingPools->Pool(1).WaterMassFlowRate = 0.5;
    state->dataSwimmingPools->Pool(1).WaterMassFlowRateMax = 2.0;
    state->dataSwimmingPools->Pool(1).WaterVolFlowMax = 0.002;
    state->dataSwimmingPools->Pool(1).MyPlantScanFlagPool = false;
    state->dataSize->SaveNumPlantComps = 0;
    state->dataSize->CompDesWaterFlow.deallocate();
    state->dataLoopNodes->Node(1).MassFlowRate = 0.0;
    state->dataLoopNodes->Node(1).MassFlowRateMax = 0.0;
    thisPool.initSwimmingPoolPlantNodeFlow(*state);
    EXPECT_EQ(state->dataSize->CompDesWaterFlow(1).SupNode, 1);
    EXPECT_EQ(state->dataSize->CompDesWaterFlow(1).DesVolFlowRate, 0.002);
}

TEST_F(EnergyPlusFixture, SwimmingPool_ErrorCheckSetupPoolSurfaceTest)
{

    // Tests for InitSwimmingPoolPlantLoopIndex
    state->dataSwimmingPools->NumSwimmingPools = 1;
    state->dataSwimmingPools->Pool.allocate(state->dataSwimmingPools->NumSwimmingPools);
    state->dataSurface->Surface.allocate(1);
    state->dataConstruction->Construct.allocate(1);
    state->dataSurface->SurfIsPool.allocate(1);
    state->dataSurface->SurfIsRadSurfOrVentSlabOrPool.allocate(1);
    state->dataSurface->intMovInsuls.allocate(1);
    // testing variables
    static constexpr std::string_view Alpha1("FirstString");
    static constexpr std::string_view Alpha2("SecondString");
    static constexpr std::string_view AlphaField2("cSecondString");
    bool ErrFnd = false;

    auto &poolReference = state->dataSwimmingPools->Pool(1);

    // Test 1: SurfacePtr is zero--this is not allowed and should produce an error
    ErrFnd = false;
    poolReference.SurfacePtr = 0;

    poolReference.ErrorCheckSetupPoolSurface(*state, Alpha1, Alpha2, AlphaField2, ErrFnd);

    EXPECT_TRUE(ErrFnd);

    // Test 2: The surface is already pointing to a radiant system, ventilated slab, or other pool--this is not allowed and should produce an error
    ErrFnd = false;
    poolReference.SurfacePtr = 1;
    state->dataSurface->SurfIsRadSurfOrVentSlabOrPool(poolReference.SurfacePtr) = true;

    poolReference.ErrorCheckSetupPoolSurface(*state, Alpha1, Alpha2, AlphaField2, ErrFnd);

    EXPECT_TRUE(ErrFnd);

    // Test 3: The heat transfer method has to be CTFs to simulate the pool, otherwise it should produce and error
    ErrFnd = false;
    poolReference.SurfacePtr = 1;
    state->dataSurface->SurfIsRadSurfOrVentSlabOrPool(poolReference.SurfacePtr) = false;
    state->dataSurface->Surface(poolReference.SurfacePtr).HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::CondFD;

    poolReference.ErrorCheckSetupPoolSurface(*state, Alpha1, Alpha2, AlphaField2, ErrFnd);

    EXPECT_TRUE(ErrFnd);

    // Test 4: The pool surface is defined as a window--this is not allowed and should produce an error
    ErrFnd = false;
    poolReference.SurfacePtr = 1;
    state->dataSurface->SurfIsRadSurfOrVentSlabOrPool(poolReference.SurfacePtr) = false;
    state->dataSurface->Surface(poolReference.SurfacePtr).HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::CTF;
    state->dataSurface->Surface(poolReference.SurfacePtr).Class = DataSurfaces::SurfaceClass::Window;

    poolReference.ErrorCheckSetupPoolSurface(*state, Alpha1, Alpha2, AlphaField2, ErrFnd);

    EXPECT_TRUE(ErrFnd);

    // Test 5: The pool is defined with movable insulation--this is not allowed and should produce an error
    ErrFnd = false;
    poolReference.SurfacePtr = 1;
    state->dataSurface->SurfIsRadSurfOrVentSlabOrPool(poolReference.SurfacePtr) = false;
    state->dataSurface->Surface(poolReference.SurfacePtr).HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::CTF;
    state->dataSurface->Surface(poolReference.SurfacePtr).Class = DataSurfaces::SurfaceClass::Floor;
    state->dataSurface->intMovInsuls(poolReference.SurfacePtr).matNum = 1;

    poolReference.ErrorCheckSetupPoolSurface(*state, Alpha1, Alpha2, AlphaField2, ErrFnd);

    EXPECT_TRUE(ErrFnd);

    // Test 6: The pool is defined with a source/sink (usually associated with radiant systems)--this is not allowed and should produce an error
    ErrFnd = false;
    poolReference.SurfacePtr = 1;
    state->dataSurface->SurfIsRadSurfOrVentSlabOrPool(poolReference.SurfacePtr) = false;
    state->dataSurface->Surface(poolReference.SurfacePtr).HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::CTF;
    state->dataSurface->Surface(poolReference.SurfacePtr).Class = DataSurfaces::SurfaceClass::Floor;
    state->dataSurface->intMovInsuls(poolReference.SurfacePtr).matNum = 1;
    state->dataSurface->Surface(poolReference.SurfacePtr).Construction = 1;
    state->dataConstruction->Construct(state->dataSurface->Surface(poolReference.SurfacePtr).Construction).SourceSinkPresent = true;

    poolReference.ErrorCheckSetupPoolSurface(*state, Alpha1, Alpha2, AlphaField2, ErrFnd);

    EXPECT_TRUE(ErrFnd);

    // Test 7: The pool is not defined as a floor--this is not allowed and should produce an error
    ErrFnd = false;
    poolReference.SurfacePtr = 1;
    state->dataSurface->SurfIsRadSurfOrVentSlabOrPool(poolReference.SurfacePtr) = false;
    state->dataSurface->Surface(poolReference.SurfacePtr).HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::CTF;
    state->dataSurface->Surface(poolReference.SurfacePtr).Class = DataSurfaces::SurfaceClass::Wall;
    state->dataSurface->intMovInsuls(poolReference.SurfacePtr).matNum = 1;
    state->dataConstruction->Construct(state->dataSurface->Surface(poolReference.SurfacePtr).Construction).SourceSinkPresent = false;

    poolReference.ErrorCheckSetupPoolSurface(*state, Alpha1, Alpha2, AlphaField2, ErrFnd);

    EXPECT_TRUE(ErrFnd);

    // Test 8: Everything about the surface that is being used by the "legal"--no error is produced
    ErrFnd = false;
    poolReference.SurfacePtr = 1;
    state->dataSurface->SurfIsRadSurfOrVentSlabOrPool(poolReference.SurfacePtr) = false;
    state->dataSurface->Surface(poolReference.SurfacePtr).HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::CTF;
    state->dataSurface->Surface(poolReference.SurfacePtr).Class = DataSurfaces::SurfaceClass::Floor;
    state->dataSurface->intMovInsuls(poolReference.SurfacePtr).matNum = 0;
    state->dataConstruction->Construct(state->dataSurface->Surface(poolReference.SurfacePtr).Construction).SourceSinkPresent = false;
    state->dataSurface->Surface(poolReference.SurfacePtr).Zone = 7;
    state->dataSurface->SurfIsPool(poolReference.SurfacePtr) = false;
    poolReference.ZonePtr = 0;

    poolReference.ErrorCheckSetupPoolSurface(*state, Alpha1, Alpha2, AlphaField2, ErrFnd);

    EXPECT_FALSE(ErrFnd);
    EXPECT_TRUE(state->dataSurface->SurfIsRadSurfOrVentSlabOrPool(poolReference.SurfacePtr));
    EXPECT_TRUE(state->dataSurface->SurfIsPool(poolReference.SurfacePtr));
    EXPECT_EQ(state->dataSurface->Surface(poolReference.SurfacePtr).Zone, poolReference.ZonePtr);
}

TEST_F(EnergyPlusFixture, SwimmingPool_MultiplePoolUpdatePoolSourceValAvgTest)
{
    // Test of UpdatePoolSourceValAvg for Multiple Swimming Pools (Fix for Defect #9797)
    // The specification of multiple pools in single input file led to weird results and
    // was traced back to improper assignment statements in UpdatePoolSourceValAvg.

    auto &PoolData = state->dataSwimmingPools;
    auto &SurfData = state->dataSurface;
    auto &HBFanData = state->dataHeatBalFanSys;

    Real64 closeEnough = 0.00001;
    PoolData->NumSwimmingPools = 2;
    PoolData->Pool.allocate(PoolData->NumSwimmingPools);
    auto &Pool1Data = state->dataSwimmingPools->Pool(1);
    auto &Pool2Data = state->dataSwimmingPools->Pool(2);

    SurfData->TotSurfaces = 2;
    SurfData->Surface.allocate(SurfData->TotSurfaces);

    Real64 noResult = -9999.0;
    HBFanData->QPoolSurfNumerator.allocate(SurfData->TotSurfaces);
    HBFanData->QPoolSurfNumerator = noResult;
    HBFanData->PoolHeatTransCoefs.allocate(SurfData->TotSurfaces);
    HBFanData->PoolHeatTransCoefs = noResult;

    SurfData->Surface(1).ExtBoundCond = 0; // All connected to exterior
    SurfData->Surface(2).ExtBoundCond = 0; // All connected to exterior
    Pool1Data.SurfacePtr = 1;
    Pool2Data.SurfacePtr = 2;

    // Test 1: both pools off
    Pool1Data.QPoolSrcAvg = 0.0;
    Pool1Data.HeatTransCoefsAvg = 0.0;
    Pool2Data.QPoolSrcAvg = 0.0;
    Pool2Data.HeatTransCoefsAvg = 0.0;

    // Call subroutine
    bool poolOnFlag = false;
    UpdatePoolSourceValAvg(*state, poolOnFlag);

    // Test data transfer
    EXPECT_FALSE(poolOnFlag);
    EXPECT_NEAR(HBFanData->QPoolSurfNumerator(1), 0.0, closeEnough);
    EXPECT_NEAR(HBFanData->QPoolSurfNumerator(2), 0.0, closeEnough);
    EXPECT_NEAR(HBFanData->PoolHeatTransCoefs(1), 0.0, closeEnough);
    EXPECT_NEAR(HBFanData->PoolHeatTransCoefs(2), 0.0, closeEnough);

    // Test 2a: pool 1 on, pool 2 off
    Pool1Data.QPoolSrcAvg = 100.0;
    Pool1Data.HeatTransCoefsAvg = 10.0;
    Pool2Data.QPoolSrcAvg = 0.0;
    Pool2Data.HeatTransCoefsAvg = 0.0;
    HBFanData->QPoolSurfNumerator = noResult;
    HBFanData->PoolHeatTransCoefs = noResult;

    // Call subroutine
    poolOnFlag = false;
    UpdatePoolSourceValAvg(*state, poolOnFlag);

    // Test data transfer
    EXPECT_TRUE(poolOnFlag);
    EXPECT_NEAR(HBFanData->QPoolSurfNumerator(1), Pool1Data.QPoolSrcAvg, closeEnough);
    EXPECT_NEAR(HBFanData->QPoolSurfNumerator(2), 0.0, closeEnough);
    EXPECT_NEAR(HBFanData->PoolHeatTransCoefs(1), Pool1Data.HeatTransCoefsAvg, closeEnough);
    EXPECT_NEAR(HBFanData->PoolHeatTransCoefs(2), 0.0, closeEnough);

    // Test 2b: pool 1 off, pool 2 on
    Pool1Data.QPoolSrcAvg = 0.0;
    Pool1Data.HeatTransCoefsAvg = 0.0;
    Pool2Data.QPoolSrcAvg = 200.0;
    Pool2Data.HeatTransCoefsAvg = 20.0;
    HBFanData->QPoolSurfNumerator = noResult;
    HBFanData->PoolHeatTransCoefs = noResult;

    // Call subroutine
    poolOnFlag = false;
    UpdatePoolSourceValAvg(*state, poolOnFlag);

    // Test data transfer
    EXPECT_TRUE(poolOnFlag);
    EXPECT_NEAR(HBFanData->QPoolSurfNumerator(1), 0.0, closeEnough);
    EXPECT_NEAR(HBFanData->QPoolSurfNumerator(2), Pool2Data.QPoolSrcAvg, closeEnough);
    EXPECT_NEAR(HBFanData->PoolHeatTransCoefs(1), 0.0, closeEnough);
    EXPECT_NEAR(HBFanData->PoolHeatTransCoefs(2), Pool2Data.HeatTransCoefsAvg, closeEnough);

    // Test 3: both pools on
    Pool1Data.QPoolSrcAvg = 100.0;
    Pool1Data.HeatTransCoefsAvg = 10.0;
    Pool2Data.QPoolSrcAvg = 200.0;
    Pool2Data.HeatTransCoefsAvg = 20.0;
    HBFanData->QPoolSurfNumerator = noResult;
    HBFanData->PoolHeatTransCoefs = noResult;

    // Call subroutine
    poolOnFlag = false;
    UpdatePoolSourceValAvg(*state, poolOnFlag);

    // Test data transfer
    EXPECT_TRUE(poolOnFlag);
    EXPECT_NEAR(HBFanData->QPoolSurfNumerator(1), Pool1Data.QPoolSrcAvg, closeEnough);
    EXPECT_NEAR(HBFanData->QPoolSurfNumerator(2), Pool2Data.QPoolSrcAvg, closeEnough);
    EXPECT_NEAR(HBFanData->PoolHeatTransCoefs(1), Pool1Data.HeatTransCoefsAvg, closeEnough);
    EXPECT_NEAR(HBFanData->PoolHeatTransCoefs(2), Pool2Data.HeatTransCoefsAvg, closeEnough);
}

TEST_F(EnergyPlusFixture, SwimmingPool_factoryTest)
{
    // Test of new factory routine as part of the move to make swimming pools a plant loop object called
    // from the plant loop and not zone equipment.
    state->dataSwimmingPools->getSwimmingPoolInput = false;
    state->dataSwimmingPools->NumSwimmingPools = 4;
    state->dataSwimmingPools->Pool.allocate(state->dataSwimmingPools->NumSwimmingPools);

    auto &poolData = state->dataSwimmingPools->Pool;
    poolData(1).Name = "Schwimmbad Nummer Eins";
    poolData(2).Name = "Schwimmbad Nummer Zwei";
    poolData(3).Name = "Schwimmbad Nummer Drei";
    poolData(4).Name = "Schwimmbad Nummer Vier";

    // Test 1: First Pool
    SwimmingPoolData *factoryResult = SwimmingPoolData::factory(*state, poolData(1).Name);
    EXPECT_NE(factoryResult, nullptr);

    // Test 2: First Pool
    factoryResult = SwimmingPoolData::factory(*state, poolData(2).Name);
    EXPECT_NE(factoryResult, nullptr);

    // Test 3: First Pool
    factoryResult = SwimmingPoolData::factory(*state, poolData(3).Name);
    EXPECT_NE(factoryResult, nullptr);

    // Test 4: First Pool
    factoryResult = SwimmingPoolData::factory(*state, poolData(4).Name);
    EXPECT_NE(factoryResult, nullptr);
}

TEST_F(EnergyPlusFixture, SwimmingPool_reportTest)
{
    // Test of modified report routine as part of the move to make swimming pools a plant loop object called
    // from the plant loop and not zone equipment.  Report routine gets called for each pool separately rather
    // than reporting for everything all at once.
    Real64 constexpr closeEnough = 0.000001;
    SwimmingPoolData myPool;

    state->init_state(*state);
    // Test Data
    myPool.Name = "This Pool";
    myPool.glycol = Fluid::GetWater(*state);
    myPool.SurfacePtr = 1;
    state->dataHeatBalSurf->SurfInsideTempHist.allocate(1);
    state->dataHeatBalSurf->SurfInsideTempHist(1).dimension(1, 0);
    state->dataHeatBalSurf->SurfInsideTempHist(1)(1) = 10.0;
    myPool.WaterMassFlowRate = 0.001;
    myPool.WaterInletTemp = 40.0;
    myPool.MiscPowerFactor = 1000.0;
    myPool.RadConvertToConvect = 0.5;
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->Surface(1).Area = 5.0;
    state->dataHVACGlobal->TimeStepSysSec = 60.0;
    myPool.MiscEquipPower = 0.12;
    myPool.MakeUpWaterMassFlowRate = 0.1;
    myPool.EvapHeatLossRate = 0.016;

    // Test of .report routine
    myPool.report(*state);
    Real64 expectedHeatPower = 125.73;
    Real64 expectedMiscEquipPower = 0.0010003;
    Real64 expectedRadConvertToConvectRep = 2.5;
    Real64 expectedMiscEquipEnergy = 0.060018;
    Real64 expectedHeatEnergy = 7543.8;
    Real64 expectedMakeUpWaterMass = 6.0;
    Real64 expectedEvapEnergyLoss = 0.96;
    Real64 expectedMakeUpWaterVolFlowRate = 0.0001003;
    Real64 expectedMakeUpWaterVol = 0.0060018;

    EXPECT_NEAR(state->dataHeatBalSurf->SurfInsideTempHist(1)(1), myPool.PoolWaterTemp, closeEnough);
    EXPECT_NEAR(expectedHeatPower, myPool.HeatPower, closeEnough);
    EXPECT_NEAR(expectedMiscEquipPower, myPool.MiscEquipPower, closeEnough);
    EXPECT_NEAR(expectedRadConvertToConvectRep, myPool.RadConvertToConvectRep, closeEnough);
    EXPECT_NEAR(expectedMiscEquipEnergy, myPool.MiscEquipEnergy, closeEnough);
    EXPECT_NEAR(expectedHeatEnergy, myPool.HeatEnergy, closeEnough);
    EXPECT_NEAR(expectedMakeUpWaterMass, myPool.MakeUpWaterMass, closeEnough);
    EXPECT_NEAR(expectedEvapEnergyLoss, myPool.EvapEnergyLoss, closeEnough);
    EXPECT_NEAR(expectedMakeUpWaterVolFlowRate, myPool.MakeUpWaterVolFlowRate, closeEnough);
    EXPECT_NEAR(expectedMakeUpWaterVol, myPool.MakeUpWaterVol, closeEnough);
}

TEST_F(EnergyPlusFixture, SwimmingPool_calcMassFlowRateTest)
{
    // Test routine added as a solution to Defect #10317
    Real64 constexpr closeEnough = 0.00001;
    Real64 tPoolWater;
    Real64 tInletWaterLoop;
    Real64 calculatedFlowRate;
    Real64 expectedAnswer;
    SwimmingPoolData testPool;

    state->dataHVACGlobal->TimeStepSysSec = 60.0;

    // Test 1: Normal pass through the routine, no limits violated
    testPool.CurSetPtTemp = 27.0;
    testPool.WaterMass = 1000.0;
    testPool.WaterMassFlowRateMax = 20.0;
    tPoolWater = 25.0;
    tInletWaterLoop = 30.0;
    expectedAnswer = 11.111111;
    testPool.calcMassFlowRate(*state, calculatedFlowRate, tPoolWater, tInletWaterLoop);
    EXPECT_NEAR(calculatedFlowRate, expectedAnswer, closeEnough);

    // Test 2: Flow rate larger than max--limit to max
    calculatedFlowRate = 0.0; // reset
    testPool.CurSetPtTemp = 27.0;
    testPool.WaterMass = 1000.0;
    testPool.WaterMassFlowRateMax = 10.0;
    tPoolWater = 25.0;
    tInletWaterLoop = 30.0;
    expectedAnswer = 10.0;
    testPool.calcMassFlowRate(*state, calculatedFlowRate, tPoolWater, tInletWaterLoop);
    EXPECT_NEAR(calculatedFlowRate, expectedAnswer, closeEnough);

    // Test 3: Current setpoint is lower than the pool temperature--flow rate set to zero
    calculatedFlowRate = -9999.9; // reset
    testPool.CurSetPtTemp = 27.0;
    testPool.WaterMass = 1000.0;
    testPool.WaterMassFlowRateMax = 10.0;
    tPoolWater = 32.0;
    tInletWaterLoop = 30.0;
    expectedAnswer = 0.0;
    testPool.calcMassFlowRate(*state, calculatedFlowRate, tPoolWater, tInletWaterLoop);
    EXPECT_NEAR(calculatedFlowRate, expectedAnswer, closeEnough);

    // Test 4: Current setpoint and inlet temperature are equal--flow rate set to max when pool water temperature is lower
    calculatedFlowRate = -9999.9; // reset
    testPool.CurSetPtTemp = 27.0;
    testPool.WaterMass = 1000.0;
    testPool.WaterMassFlowRateMax = 20.0;
    tPoolWater = 25.0;
    tInletWaterLoop = 27.0;
    expectedAnswer = 20.0;
    testPool.calcMassFlowRate(*state, calculatedFlowRate, tPoolWater, tInletWaterLoop);
    EXPECT_NEAR(calculatedFlowRate, expectedAnswer, closeEnough);

    // Test 5: Current setpoint and inlet temperature are equal--flow rate set to zero when pool water temperature is higher
    calculatedFlowRate = -9999.9; // reset
    testPool.CurSetPtTemp = 27.0;
    testPool.WaterMass = 1000.0;
    testPool.WaterMassFlowRateMax = 20.0;
    tPoolWater = 32.0;
    tInletWaterLoop = 27.0;
    expectedAnswer = 0.0;
    testPool.calcMassFlowRate(*state, calculatedFlowRate, tPoolWater, tInletWaterLoop);
    EXPECT_NEAR(calculatedFlowRate, expectedAnswer, closeEnough);

    // Test 6: Water temp is below the setpoint but higher than the pool temp--flow rate set to max (this was the cause of the defect)
    calculatedFlowRate = -9999.9; // reset
    testPool.CurSetPtTemp = 27.0;
    testPool.WaterMass = 1000.0;
    testPool.WaterMassFlowRateMax = 17.0;
    tPoolWater = 25.0;
    tInletWaterLoop = 26.0;
    expectedAnswer = 17.0;
    testPool.calcMassFlowRate(*state, calculatedFlowRate, tPoolWater, tInletWaterLoop);
    EXPECT_NEAR(calculatedFlowRate, expectedAnswer, closeEnough);
}
