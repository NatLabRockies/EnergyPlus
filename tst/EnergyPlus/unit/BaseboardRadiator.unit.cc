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

// EnergyPlus::Standalone ERV Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/BaseboardRadiator.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataErrorTracking.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SurfaceGeometry.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::DataPlant;
using namespace EnergyPlus::DataSizing;
using namespace EnergyPlus::DataZoneEnergyDemands;

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, BaseboardConvWater_SizingTest)
{
    int BaseboardNum(0);
    int CntrlZoneNum(0);
    bool FirstHVACIteration(false);

    std::string const idf_objects = delimited_string({
        "  Zone,",
        "    SPACE2-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    103.311355591;           !- Volume {m3}",

        "  Zone,",
        "    SPACE3-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    103.311355591;           !- Volume {m3}",

        "  Zone,",
        "    SPACE4-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    2.438400269,             !- Ceiling Height {m}",
        "    103.311355591;           !- Volume {m3}",

        "  ZoneHVAC:EquipmentConnections,",
        "    SPACE2-1,                !- Zone Name",
        "    SPACE2-1 Eq,             !- Zone Conditioning Equipment List Name",
        "    SPACE2-1 in node,        !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    SPACE2-1 Node,           !- Zone Air Node Name",
        "    SPACE2-1 ret node;       !- Zone Return Air Node Name",

        "  ZoneHVAC:EquipmentConnections,",
        "    SPACE3-1,                !- Zone Name",
        "    SPACE3-1 Eq,             !- Zone Conditioning Equipment List Name",
        "    SPACE3-1 in node,        !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    SPACE3-1 Node,           !- Zone Air Node Name",
        "    SPACE3-1 ret node;       !- Zone Return Air Node Name",

        "  ZoneHVAC:EquipmentConnections,",
        "    SPACE4-1,                !- Zone Name",
        "    SPACE4-1 Eq,             !- Zone Conditioning Equipment List Name",
        "    SPACE4-1 in node,       !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    SPACE4-1 Node,           !- Zone Air Node Name",
        "    SPACE4-1 ret node;       !- Zone Return Air Node Name",

        "  ZoneHVAC:EquipmentList,",
        "    SPACE2-1 Eq,             !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:Baseboard:Convective:Water,  !- Zone Equipment 1 Object Type",
        "    SPACE2-1 Baseboard,      !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "  ZoneHVAC:EquipmentList,",
        "    SPACE3-1 Eq,             !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:Baseboard:Convective:Water,  !- Zone Equipment 1 Object Type",
        "    SPACE3-1 Baseboard,      !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "  ZoneHVAC:EquipmentList,",
        "    SPACE4-1 Eq,             !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:Baseboard:Convective:Water,  !- Zone Equipment 1 Object Type",
        "    SPACE4-1 Baseboard,      !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        " ZoneHVAC:Baseboard:Convective:Water,",
        "    SPACE2-1 Baseboard,      !- Name",
        "    CONSTANT-1.0,               !- Availability Schedule Name",
        "    SPACE2-1 Baseboard Inlet Node,   !- Inlet Node Name",
        "    SPACE2-1 Baseboard Outlet Node,  !- Outlet Node Name",
        "    HeatingDesignCapacity,   !- Heating Design Capacity Method",
        "    1000.0,                  !- Heating Design Capacity {W}",
        "    ,                        !- Heating Design Capacity Per Floor Area {W/m2}",
        "    ,                        !- Fraction of Autosized Heating Design Capacity",
        "    autosize,                !- U-Factor Times Area Value",
        "    autosize,                !- Maximum Water Flow Rate",
        "    0.001;                   !- Convergence Tolerance",

        "  BuildingSurface:Detailed,",
        "    RIGHT-1,                 !- Name",
        "    WALL,                    !- Surface Type",
        "    WALL-1,                  !- Construction Name",
        "    SPACE2-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50000,                 !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    30.5,0.0,2.4,   !- X,Y,Z ==> Vertex 1 {m}",
        "    30.5,0.0,0.0,   !- X,Y,Z ==> Vertex 2 {m}",
        "    30.5,15.2,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    30.5,15.2,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  ZoneHVAC:Baseboard:Convective:Water,",
        "    SPACE3-1 Baseboard,      !- Name",
        "    CONSTANT-1.0,               !- Availability Schedule Name",
        "    SPACE3-1 Baseboard Inlet Node,   !- Inlet Node Name",
        "    SPACE3-1 Baseboard Outlet Node,  !- Outlet Node Name",
        "    CapacityPerFloorArea,    !- Heating Design Capacity Method",
        "    ,                        !- Heating Design Capacity {W}",
        "    40.0,                    !- Heating Design Capacity Per Floor Area {W/m2}",
        "    ,                        !- Fraction of Autosized Heating Design Capacity",
        "    autosize,                !- U-Factor Times Area Value",
        "    autosize,                !- Maximum Water Flow Rate",
        "    0.001;                   !- Convergence Tolerance",

        "  BuildingSurface:Detailed,",
        "    FRONT-1,                  !- Name",
        "    WALL,                    !- Surface Type",
        "    WALL-1,                  !- Construction Name",
        "    SPACE3-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50000,                 !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0, 0.0, 2.4,    !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0, 0.0, 0.0,    !- X,Y,Z ==> Vertex 2 {m}",
        "    20.0, 0.0, 0.0,   !- X,Y,Z ==> Vertex 3 {m}",
        "    20.0, 0.0, 2.4;   !- X,Y,Z ==> Vertex 4 {m}",

        "  ZoneHVAC:Baseboard:Convective:Water,",
        "    SPACE4-1 Baseboard,      !- Name",
        "    CONSTANT-1.0,               !- Availability Schedule Name",
        "    SPACE4-1 Baseboard Inlet Node,   !- Inlet Node Name",
        "    SPACE4-1 Baseboard Outlet Node,  !- Outlet Node Name",
        "    FractionOfAutosizedHeatingCapacity,   !- Heating Design Capacity Method",
        "    ,                        !- Heating Design Capacity {W}",
        "    ,                        !- Heating Design Capacity Per Floor Area {W/m2}",
        "    0.5,                     !- Fraction of Autosized Heating Design Capacity",
        "    autosize,                !- U-Factor Times Area Value",
        "    autosize,                !- Maximum Water Flow Rate",
        "    0.001;                   !- Convergence Tolerance",

        "  BuildingSurface:Detailed,",
        "    LEFT-1,                  !- Name",
        "    WALL,                    !- Surface Type",
        "    WALL-1,                  !- Construction Name",
        "    SPACE4-1,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50000,                 !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0,15.2,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,15.2,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,0.0,0.0,   !- X,Y,Z ==> Vertex 3 {m}",
        "    0.0,0.0,2.4;   !- X,Y,Z ==> Vertex 4 {m}",

        "SurfaceConvectionAlgorithm:Inside,TARP;",

        "SurfaceConvectionAlgorithm:Outside,DOE-2;",

        "HeatBalanceAlgorithm,ConductionTransferFunction;",

        "ZoneAirHeatBalanceAlgorithm,",
        "    AnalyticalSolution;      !- Algorithm",

        "  Construction,",
        "    WALL-1,                  !- Name",
        "    WD01,                    !- Outside Layer",
        "    PW03,                    !- Layer 2",
        "    IN02,                    !- Layer 3",
        "    GP01;                    !- Layer 4",

        "  Material,",
        "    WD01,                    !- Name",
        "    MediumSmooth,            !- Roughness",
        "    1.9099999E-02,           !- Thickness {m}",
        "    0.1150000,               !- Conductivity {W/m-K}",
        "    513.0000,                !- Density {kg/m3}",
        "    1381.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7800000,               !- Solar Absorptance",
        "    0.7800000;               !- Visible Absorptance",

        "  Material,",
        "    PW03,                    !- Name",
        "    MediumSmooth,            !- Roughness",
        "    1.2700000E-02,           !- Thickness {m}",
        "    0.1150000,               !- Conductivity {W/m-K}",
        "    545.0000,                !- Density {kg/m3",
        "    1213.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7800000,               !- Solar Absorptance",
        "    0.7800000;               !- Visible Absorptance",

        "  Material,",
        "    IN02,                    !- Name",
        "    Rough,                   !- Roughness",
        "    9.0099998E-02,           !- Thickness {m}",
        "    4.3000001E-02,           !- Conductivity {W/m-K}",
        "    10.00000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    GP01,                    !- Name",
        "    MediumSmooth,            !- Roughness",
        "    1.2700000E-02,           !- Thickness {m}",
        "    0.1600000,               !- Conductivity {W/m-K}",
        "    801.0000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized

    state->init_state(*state);

    bool errorsFound(false);
    HeatBalanceManager::GetProjectControlData(*state, errorsFound); // read project control data
    EXPECT_FALSE(errorsFound);                                      // expect no errors

    errorsFound = false;
    Material::GetMaterialData(*state, errorsFound); // read material data
    EXPECT_FALSE(errorsFound);                      // expect no errors

    errorsFound = false;
    HeatBalanceManager::GetConstructData(*state, errorsFound); // read construction data
    EXPECT_FALSE(errorsFound);                                 // expect no errors

    HeatBalanceManager::GetZoneData(*state, errorsFound);
    ASSERT_FALSE(errorsFound);

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(3);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(3);
    state->dataSurfaceGeometry->CosZoneRelNorth(1) = std::cos(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosZoneRelNorth(2) = std::cos(-state->dataHeatBal->Zone(2).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosZoneRelNorth(3) = std::cos(-state->dataHeatBal->Zone(3).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(1) = std::sin(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(2) = std::sin(-state->dataHeatBal->Zone(2).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(3) = std::sin(-state->dataHeatBal->Zone(3).RelNorth * Constant::DegToRad);

    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;

    SurfaceGeometry::GetSurfaceData(*state, errorsFound);
    ASSERT_FALSE(errorsFound);

    state->dataSize->ZoneSizingInput.allocate(3);
    state->dataSize->NumZoneSizingInput = 3;
    state->dataSize->ZoneSizingInput(1).ZoneNum = 1;
    state->dataSize->ZoneSizingInput(2).ZoneNum = 2;
    state->dataSize->ZoneSizingInput(3).ZoneNum = 3;

    int TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(TotNumLoops);
    state->dataSize->PlantSizData.allocate(TotNumLoops);
    state->dataSize->PlantSizData(1).DeltaT = 10.0;
    // unit test results are based on a low HW temp at 40 C. Baseboard does not have the capacity to meet the zone load.
    state->dataSize->PlantSizData(1).ExitTemp = 40.0;

    for (int l = 1; l <= TotNumLoops; ++l) {
        auto &loop(state->dataPlnt->PlantLoop(l));
        loop.PlantSizNum = 1;
        loop.FluidName = "WATER";
        loop.glycol = Fluid::GetWater(*state);
        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }

    state->dataSize->ZoneSizingRunDone = true;
    DataZoneEquipment::GetZoneEquipmentData(*state);
    // get electric baseboard inputs
    BaseboardRadiator::GetBaseboardInput(*state);

    state->dataSize->FinalZoneSizing.allocate(3);
    state->dataSize->ZoneEqSizing.allocate(3);
    state->dataSize->ZoneSizingRunDone = true;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(3);
    state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(3);

    BaseboardNum = 1;
    CntrlZoneNum = 1;
    state->dataSize->CurZoneEqNum = CntrlZoneNum;
    FirstHVACIteration = true;
    state->dataSize->ZoneEqSizing(CntrlZoneNum).SizingMethod.allocate(HVAC::NumOfSizingTypes);
    state->dataSize->ZoneEqSizing(CntrlZoneNum).SizingMethod(HVAC::HeatingCapacitySizing) =
        state->dataBaseboardRadiator->baseboards(BaseboardNum).HeatingCapMethod;
    state->dataSize->FinalZoneSizing(CntrlZoneNum).NonAirSysDesHeatLoad = 2000.0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(CntrlZoneNum).RemainingOutputReqToHeatSP = 2000.0;
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(CntrlZoneNum) = false;
    state->dataSize->FinalZoneSizing(CntrlZoneNum).ZoneTempAtHeatPeak = 20.0;
    state->dataSize->FinalZoneSizing(CntrlZoneNum).ZoneHumRatAtHeatPeak = 0.005;
    // do baseboard sizing
    state->dataBaseboardRadiator->baseboards(BaseboardNum).plantLoc.loopNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataBaseboardRadiator->baseboards(BaseboardNum).plantLoc);

    state->dataBaseboardRadiator->baseboards(BaseboardNum).ZonePtr = 1;
    state->dataBaseboardRadiator->baseboards(BaseboardNum).SizeBaseboard(*state, BaseboardNum);
    // check UA value
    EXPECT_EQ(state->dataBaseboardRadiator->baseboards(BaseboardNum).ScaledHeatingCapacity, 1000.0);
    EXPECT_EQ(state->dataBaseboardRadiator->baseboards(BaseboardNum).UA, 1000.0);
    // check UA value with autosized scaled capacity
    state->dataBaseboardRadiator->baseboards(BaseboardNum).ScaledHeatingCapacity = DataSizing::AutoSize;
    state->dataBaseboardRadiator->baseboards(BaseboardNum).WaterVolFlowRateMax = DataSizing::AutoSize;
    state->dataBaseboardRadiator->baseboards(BaseboardNum).UA = DataSizing::AutoSize; // reset to autosize to test new calculation
    state->files.eio.open_as_stringstream();
    state->dataBaseboardRadiator->baseboards(BaseboardNum).SizeBaseboard(*state, BaseboardNum);
    EXPECT_EQ(state->dataZoneEnergyDemand->ZoneSysEnergyDemand(CntrlZoneNum).RemainingOutputReqToHeatSP, 2000.0); // design load = 2000
    EXPECT_EQ(state->dataBaseboardRadiator->baseboards(BaseboardNum).UA, 2000.0);                                 // UA = design load
    EXPECT_NEAR(state->dataBaseboardRadiator->baseboards(BaseboardNum).WaterVolFlowRateMax, 4.86063E-05, 0.0000001);
    EXPECT_TRUE(compare_eio_stream_substring("Component Sizing Information, ZoneHVAC:Baseboard:Convective:Water, SPACE2-1 BASEBOARD, "
                                             "Design Size Maximum Water Flow Rate [m3/s], 4.86063E-05",
                                             false));
    EXPECT_TRUE(compare_eio_stream_substring("Component Sizing Information, ZoneHVAC:Baseboard:Convective:Water, SPACE2-1 BASEBOARD, "
                                             "Design Size Heating Load [W], 2000",
                                             false));
    EXPECT_TRUE(compare_eio_stream_substring("Component Sizing Information, ZoneHVAC:Baseboard:Convective:Water, SPACE2-1 BASEBOARD, "
                                             "Design Size U-Factor Times Area Value [W/K], 2000",
                                             true));

    BaseboardNum = 2;
    CntrlZoneNum = 2;
    state->dataSize->CurZoneEqNum = CntrlZoneNum;
    FirstHVACIteration = true;
    state->dataSize->ZoneEqSizing(CntrlZoneNum).SizingMethod.allocate(HVAC::NumOfSizingTypes);
    state->dataSize->ZoneEqSizing(CntrlZoneNum).SizingMethod(HVAC::HeatingCapacitySizing) =
        state->dataBaseboardRadiator->baseboards(BaseboardNum).HeatingCapMethod;
    state->dataSize->FinalZoneSizing(CntrlZoneNum).NonAirSysDesHeatLoad = 2000.0;
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(CntrlZoneNum) = false;
    state->dataSize->FinalZoneSizing(CntrlZoneNum).ZoneTempAtHeatPeak = 20.0;
    state->dataSize->FinalZoneSizing(CntrlZoneNum).ZoneHumRatAtHeatPeak = 0.005;
    state->dataHeatBal->Zone(CntrlZoneNum).FloorArea = 100.0;
    // do baseboard sizing
    state->dataBaseboardRadiator->baseboards(BaseboardNum).plantLoc.loopNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataBaseboardRadiator->baseboards(BaseboardNum).plantLoc);

    state->dataBaseboardRadiator->baseboards(BaseboardNum).ZonePtr = 2;
    state->dataBaseboardRadiator->baseboards(BaseboardNum).SizeBaseboard(*state, BaseboardNum);
    // check UA value
    EXPECT_EQ(state->dataBaseboardRadiator->baseboards(BaseboardNum).ScaledHeatingCapacity, 40.0);
    EXPECT_EQ(state->dataBaseboardRadiator->baseboards(BaseboardNum).UA, 4000.0);
    state->dataBaseboardRadiator->baseboards(BaseboardNum).UA = DataSizing::AutoSize; // reset to autosize to test new calculation
    state->dataBaseboardRadiator->baseboards(BaseboardNum).WaterVolFlowRateMax = DataSizing::AutoSize;
    // check UA value with autosized UA
    state->dataBaseboardRadiator->baseboards(BaseboardNum).HeatingCapMethod = DataSizing::HeatingDesignCapacity;
    state->dataBaseboardRadiator->baseboards(BaseboardNum).ScaledHeatingCapacity = DataSizing::AutoSize;
    state->files.eio.open_as_stringstream();
    state->dataBaseboardRadiator->baseboards(BaseboardNum).SizeBaseboard(*state, BaseboardNum);
    EXPECT_EQ(state->dataBaseboardRadiator->baseboards(BaseboardNum).UA, 2000.0);
    EXPECT_NEAR(state->dataBaseboardRadiator->baseboards(BaseboardNum).WaterVolFlowRateMax, 4.86063E-05, 0.0000001);
    EXPECT_TRUE(compare_eio_stream_substring("Component Sizing Information, ZoneHVAC:Baseboard:Convective:Water, SPACE3-1 BASEBOARD, "
                                             "Design Size Maximum Water Flow Rate [m3/s], 4.86063E-05",
                                             false));
    EXPECT_TRUE(compare_eio_stream_substring("Component Sizing Information, ZoneHVAC:Baseboard:Convective:Water, SPACE3-1 BASEBOARD, "
                                             "Design Size Heating Load [W], 2000",
                                             false));
    EXPECT_TRUE(compare_eio_stream_substring("Component Sizing Information, ZoneHVAC:Baseboard:Convective:Water, SPACE3-1 BASEBOARD, "
                                             "Design Size U-Factor Times Area Value [W/K], 2000",
                                             true));

    BaseboardNum = 3;
    CntrlZoneNum = 3;
    state->dataSize->CurZoneEqNum = CntrlZoneNum;
    FirstHVACIteration = true;
    state->dataSize->ZoneEqSizing(CntrlZoneNum).SizingMethod.allocate(HVAC::NumOfSizingTypes);
    state->dataSize->ZoneEqSizing(CntrlZoneNum).SizingMethod(HVAC::HeatingCapacitySizing) =
        state->dataBaseboardRadiator->baseboards(BaseboardNum).HeatingCapMethod;
    state->dataSize->FinalZoneSizing(CntrlZoneNum).NonAirSysDesHeatLoad = 3000.0;
    state->dataZoneEnergyDemand->CurDeadBandOrSetback(CntrlZoneNum) = false;
    state->dataSize->FinalZoneSizing(CntrlZoneNum).ZoneTempAtHeatPeak = 20.0;
    state->dataSize->FinalZoneSizing(CntrlZoneNum).ZoneHumRatAtHeatPeak = 0.005;
    state->dataHeatBal->Zone(CntrlZoneNum).FloorArea = 100.0;
    // do baseboard sizing
    state->dataBaseboardRadiator->baseboards(BaseboardNum).plantLoc.loopNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataBaseboardRadiator->baseboards(BaseboardNum).plantLoc);

    state->dataBaseboardRadiator->baseboards(BaseboardNum).ZonePtr = 3;
    state->dataBaseboardRadiator->baseboards(BaseboardNum).SizeBaseboard(*state, BaseboardNum);
    // check UA value
    EXPECT_EQ(state->dataBaseboardRadiator->baseboards(BaseboardNum).ScaledHeatingCapacity, 0.50);
    EXPECT_EQ(state->dataBaseboardRadiator->baseboards(BaseboardNum).UA, 1500.0);
    state->dataBaseboardRadiator->baseboards(BaseboardNum).UA = DataSizing::AutoSize; // reset to autosize to test new calculation
    state->dataBaseboardRadiator->baseboards(BaseboardNum).WaterVolFlowRateMax = DataSizing::AutoSize;
    // check UA value with autosized scaled capacity
    state->dataBaseboardRadiator->baseboards(BaseboardNum).HeatingCapMethod = DataSizing::HeatingDesignCapacity;
    state->dataBaseboardRadiator->baseboards(BaseboardNum).ScaledHeatingCapacity = DataSizing::AutoSize;
    state->files.eio.open_as_stringstream();
    state->dataBaseboardRadiator->baseboards(BaseboardNum).SizeBaseboard(*state, BaseboardNum);
    EXPECT_EQ(state->dataBaseboardRadiator->baseboards(BaseboardNum).UA, 3000.0);
    EXPECT_NEAR(state->dataBaseboardRadiator->baseboards(BaseboardNum).WaterVolFlowRateMax, 7.29095E-05, 0.0000001);
    EXPECT_TRUE(compare_eio_stream_substring("Component Sizing Information, ZoneHVAC:Baseboard:Convective:Water, SPACE4-1 BASEBOARD, "
                                             "Design Size Maximum Water Flow Rate [m3/s], 7.29095E-05",
                                             false));
    EXPECT_TRUE(compare_eio_stream_substring("Component Sizing Information, ZoneHVAC:Baseboard:Convective:Water, SPACE4-1 BASEBOARD, "
                                             "Design Size Heating Load [W], 3000",
                                             false));
    EXPECT_TRUE(compare_eio_stream_substring("Component Sizing Information, ZoneHVAC:Baseboard:Convective:Water, SPACE4-1 BASEBOARD, "
                                             "Design Size U-Factor Times Area Value [W/K], 3000"));
}

TEST_F(EnergyPlusFixture, BaseboardConvWater_checkForZoneSizingTest)
{
    state->dataBaseboardRadiator->baseboards.allocate(1);
    auto &thisBB = state->dataBaseboardRadiator->baseboards(1);
    state->dataSize->ZoneSizingRunDone = false;

    std::string const error_string =
        delimited_string({"   ** Severe  ** For autosizing of ZoneHVAC:Baseboard:Convective:Water , a zone sizing run must be done.\n"
                          "   **   ~~~   ** No \"Sizing:Zone\" objects were entered.\n"
                          "   **   ~~~   ** The \"SimulationControl\" object did not have the field \"Do Zone Sizing Calculation\" set to Yes.\n"
                          "   **  Fatal  ** Program terminates due to previously shown condition(s).\n"
                          "   ...Summary of Errors that led to program termination:\n"
                          "   ..... Reference severe error count=1\n"
                          "   ..... Last severe error=For autosizing of ZoneHVAC:Baseboard:Convective:Water , a zone sizing run must be done."});

    // Test 1A: UA autosized so MySizeFlag should stay true
    thisBB.UA = DataSizing::AutoSize;
    thisBB.WaterVolFlowRateMax = 0.001;
    thisBB.HeatingCapMethod = DataSizing::FractionOfAutosizedHeatingCapacity;
    thisBB.ScaledHeatingCapacity = 1.0;
    ASSERT_THROW(thisBB.checkForZoneSizing(*state), std::runtime_error);
    EXPECT_TRUE(compare_err_stream(error_string, true));

    // Test 1B: WaterVolFlowRateMax autosized so MySizeFlag should stay true
    state->dataErrTracking->TotalSevereErrors = 0;
    thisBB.UA = 0.5;
    thisBB.WaterVolFlowRateMax = DataSizing::AutoSize;
    thisBB.HeatingCapMethod = DataSizing::FractionOfAutosizedHeatingCapacity;
    thisBB.ScaledHeatingCapacity = 1.0;
    ASSERT_THROW(thisBB.checkForZoneSizing(*state), std::runtime_error);
    EXPECT_TRUE(compare_err_stream(error_string, true));

    // Test 1C: Heating Capacity autosized for method HeatingDesignCapacity so MySizeFlag should stay true
    state->dataErrTracking->TotalSevereErrors = 0;
    thisBB.UA = 0.5;
    thisBB.WaterVolFlowRateMax = 0.001;
    thisBB.HeatingCapMethod = DataSizing::HeatingDesignCapacity;
    thisBB.ScaledHeatingCapacity = DataSizing::AutoSize;
    ASSERT_THROW(thisBB.checkForZoneSizing(*state), std::runtime_error);
    EXPECT_TRUE(compare_err_stream(error_string, true));

    // Test 2A: Heating Capacity not autosized for method HeatingDesignCapacity and UA and WaterVolFlowRateMax not autosized
    //          so MySizeFlag should be changed to false
    state->dataErrTracking->TotalSevereErrors = 0;
    thisBB.UA = 0.5;
    thisBB.WaterVolFlowRateMax = 0.001;
    thisBB.HeatingCapMethod = DataSizing::HeatingDesignCapacity;
    thisBB.ScaledHeatingCapacity = 1000.0;
    thisBB.checkForZoneSizing(*state);
    EXPECT_TRUE(compare_err_stream("", true));

    // Test 2B: CapacityPerFloorArea method and UA and WaterVolFlowRateMax not autosized
    //          so MySizeFlag should be changed to false
    state->dataErrTracking->TotalSevereErrors = 0;
    thisBB.UA = 0.5;
    thisBB.WaterVolFlowRateMax = 0.001;
    thisBB.HeatingCapMethod = DataSizing::CapacityPerFloorArea;
    thisBB.ScaledHeatingCapacity = DataSizing::AutoSize; // this value does not mater since it is not really valid for this method
    thisBB.checkForZoneSizing(*state);
    EXPECT_TRUE(compare_err_stream("", true));
}

} // namespace EnergyPlus
