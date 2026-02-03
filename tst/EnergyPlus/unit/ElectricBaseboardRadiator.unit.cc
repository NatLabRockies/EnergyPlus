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

// EnergyPlus::Standalone ERV Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/ElectricBaseboardRadiator.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SurfaceGeometry.hh>

using namespace EnergyPlus;

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, RadConvElecBaseboard_Test1)
{
    // this unit test is related to issue #5806, the input is configured to allow running get input on two electric radiative convective baseboards
    // and check that they have their zone index pointers setup
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
        "    SPACE2-1 in node,       !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    SPACE2-1 Node,           !- Zone Air Node Name",
        "    SPACE2-1 ret node;       !- Zone Return Air Node Name",

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
        "    ZoneHVAC:Baseboard:RadiantConvective:Electric,  !- Zone Equipment 1 Object Type",
        "    SPACE2-1 Baseboard,      !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "  ZoneHVAC:EquipmentList,",
        "    SPACE4-1 Eq,             !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:Baseboard:RadiantConvective:Electric,  !- Zone Equipment 1 Object Type",
        "    SPACE4-1 Baseboard,      !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        " ZoneHVAC:Baseboard:RadiantConvective:Electric,",
        "    SPACE2-1 Baseboard,      !- Name",
        "    CONSTANT-1.0,    !- Availability Schedule Name",
        "    HeatingDesignCapacity,   !- Heating Design Capacity Method",
        "    1000.0,                !- Heating Design Capacity {W}",
        "    ,                        !- Heating Design Capacity Per Floor Area {W/m2}",
        "    ,                        !- Fraction of Autosized Heating Design Capacity",
        "    0.97,                    !- Efficiency",
        "    0.2,                     !- Fraction Radiant",
        "    0.3,                     !- Fraction of Radiant Energy Incident on People",
        "    RIGHT-1,                 !- Surface 1 Name",
        "    0.7;                     !- Fraction of Radiant Energy to Surface 1",

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
        "    30.5,0.0,2.4,  !- X,Y,Z ==> Vertex 1 {m}",
        "    30.5,0.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    30.5,15.2,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    30.5,15.2,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

        "  ZoneHVAC:Baseboard:RadiantConvective:Electric,",
        "    SPACE4-1 Baseboard,      !- Name",
        "    CONSTANT-1.0,    !- Availability Schedule Name",
        "    HeatingDesignCapacity,   !- Heating Design Capacity Method",
        "    1000.0,                !- Heating Design Capacity {W}",
        "    ,                        !- Heating Design Capacity Per Floor Area {W/m2}",
        "    ,                        !- Fraction of Autosized Heating Design Capacity",
        "    0.97,                    !- Efficiency",
        "    0.2,                     !- Fraction Radiant",
        "    0.3,                     !- Fraction of Radiant Energy Incident on People",
        "    LEFT-1,                  !- Surface 1 Name",
        "    0.7;                     !- Fraction of Radiant Energy to Surface 1",

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
        "    0.0,0.0,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0.0,0.0,2.4;  !- X,Y,Z ==> Vertex 4 {m}",

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

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(2);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(2);
    state->dataSurfaceGeometry->CosZoneRelNorth(1) = std::cos(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosZoneRelNorth(2) = std::cos(-state->dataHeatBal->Zone(2).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(1) = std::sin(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(2) = std::sin(-state->dataHeatBal->Zone(2).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;

    SurfaceGeometry::GetSurfaceData(*state, errorsFound);
    ASSERT_FALSE(errorsFound);

    DataZoneEquipment::GetZoneEquipmentData(*state);

    ElectricBaseboardRadiator::GetElectricBaseboardInput(*state);
    EXPECT_EQ(state->dataElectBaseboardRad->ElecBaseboard(1).ZonePtr, 1);
    EXPECT_EQ(state->dataElectBaseboardRad->ElecBaseboard(2).ZonePtr, 2);

    int surfNumRight1 = Util::FindItemInList("RIGHT-1", state->dataSurface->Surface);
    int surfNumLeft1 = Util::FindItemInList("LEFT-1", state->dataSurface->Surface);

    EXPECT_EQ(state->dataSurface->allGetsRadiantHeatSurfaceList[0], surfNumRight1);
    EXPECT_EQ(state->dataSurface->allGetsRadiantHeatSurfaceList[1], surfNumLeft1);
    EXPECT_TRUE(state->dataSurface->surfIntConv(surfNumRight1).getsRadiantHeat);
    EXPECT_TRUE(state->dataSurface->surfIntConv(surfNumLeft1).getsRadiantHeat);
}

TEST_F(EnergyPlusFixture, ElectricBaseboardRadConv_SizingTest)
{
    // this unit test is related to issue #5890
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
        "    ZoneHVAC:Baseboard:RadiantConvective:Electric,  !- Zone Equipment 1 Object Type",
        "    SPACE2-1 Baseboard,      !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "  ZoneHVAC:EquipmentList,",
        "    SPACE3-1 Eq,             !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:Baseboard:RadiantConvective:Electric,  !- Zone Equipment 1 Object Type",
        "    SPACE3-1 Baseboard,      !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        "  ZoneHVAC:EquipmentList,",
        "    SPACE4-1 Eq,             !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:Baseboard:RadiantConvective:Electric,  !- Zone Equipment 1 Object Type",
        "    SPACE4-1 Baseboard,      !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1;                       !- Zone Equipment 1 Heating or No-Load Sequence",

        " ZoneHVAC:Baseboard:RadiantConvective:Electric,",
        "    SPACE2-1 Baseboard,      !- Name",
        "    CONSTANT-1.0,               !- Availability Schedule Name",
        "    HeatingDesignCapacity,   !- Heating Design Capacity Method",
        "    1000.0,                  !- Heating Design Capacity {W}",
        "    ,                        !- Heating Design Capacity Per Floor Area {W/m2}",
        "    ,                        !- Fraction of Autosized Heating Design Capacity",
        "    0.97,                    !- Efficiency",
        "    0.2,                     !- Fraction Radiant",
        "    0.3,                     !- Fraction of Radiant Energy Incident on People",
        "    RIGHT-1,                 !- Surface 1 Name",
        "    0.7;                     !- Fraction of Radiant Energy to Surface 1",

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

        "  ZoneHVAC:Baseboard:RadiantConvective:Electric,",
        "    SPACE3-1 Baseboard,      !- Name",
        "    CONSTANT-1.0,               !- Availability Schedule Name",
        "    CapacityPerFloorArea,    !- Heating Design Capacity Method",
        "    ,                        !- Heating Design Capacity {W}",
        "    30.0,                    !- Heating Design Capacity Per Floor Area {W/m2}",
        "    ,                        !- Fraction of Autosized Heating Design Capacity",
        "    0.97,                    !- Efficiency",
        "    0.2,                     !- Fraction Radiant",
        "    0.3,                     !- Fraction of Radiant Energy Incident on People",
        "    FRONT-1,                 !- Surface 1 Name",
        "    0.7;                     !- Fraction of Radiant Energy to Surface 1",

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

        "  ZoneHVAC:Baseboard:RadiantConvective:Electric,",
        "    SPACE4-1 Baseboard,      !- Name",
        "    CONSTANT-1.0,               !- Availability Schedule Name",
        "    FractionOfAutosizedHeatingCapacity,   !- Heating Design Capacity Method",
        "    ,                        !- Heating Design Capacity {W}",
        "    ,                        !- Heating Design Capacity Per Floor Area {W/m2}",
        "    0.5,                     !- Fraction of Autosized Heating Design Capacity",
        "    0.97,                    !- Efficiency",
        "    0.2,                     !- Fraction Radiant",
        "    0.3,                     !- Fraction of Radiant Energy Incident on People",
        "    LEFT-1,                  !- Surface 1 Name",
        "    0.7;                     !- Fraction of Radiant Energy to Surface 1",

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

    DataZoneEquipment::GetZoneEquipmentData(*state);
    // get electric baseboard inputs
    ElectricBaseboardRadiator::GetElectricBaseboardInput(*state);

    EXPECT_EQ(state->dataElectBaseboardRad->ElecBaseboard(1).ZonePtr, 1);
    EXPECT_EQ(state->dataElectBaseboardRad->ElecBaseboard(2).ZonePtr, 2);
    EXPECT_EQ(state->dataElectBaseboardRad->ElecBaseboard(3).ZonePtr, 3);

    state->dataSize->FinalZoneSizing.allocate(3);
    state->dataSize->ZoneEqSizing.allocate(3);
    state->dataSize->ZoneSizingRunDone = true;

    BaseboardNum = 1;
    CntrlZoneNum = 1;
    state->dataSize->CurZoneEqNum = CntrlZoneNum;
    FirstHVACIteration = true;
    state->dataSize->ZoneEqSizing(CntrlZoneNum).SizingMethod.allocate(HVAC::NumOfSizingTypes);
    state->dataSize->ZoneEqSizing(CntrlZoneNum).SizingMethod(HVAC::HeatingCapacitySizing) =
        state->dataElectBaseboardRad->ElecBaseboard(BaseboardNum).HeatingCapMethod;
    state->dataSize->FinalZoneSizing(CntrlZoneNum).NonAirSysDesHeatLoad = 2000.0;
    // do electric baseboard sizing
    ElectricBaseboardRadiator::SizeElectricBaseboard(*state, BaseboardNum);
    // check user specified hardsized nominal capacity
    EXPECT_EQ(state->dataElectBaseboardRad->ElecBaseboard(BaseboardNum).ScaledHeatingCapacity, 1000.0);
    EXPECT_EQ(state->dataElectBaseboardRad->ElecBaseboard(BaseboardNum).NominalCapacity, 1000.0);
    // check nominal capacity autosize
    state->dataElectBaseboardRad->ElecBaseboard(BaseboardNum).ScaledHeatingCapacity = DataSizing::AutoSize;
    ElectricBaseboardRadiator::SizeElectricBaseboard(*state, BaseboardNum);
    EXPECT_EQ(state->dataElectBaseboardRad->ElecBaseboard(BaseboardNum).NominalCapacity, 2000.0);

    BaseboardNum = 2;
    CntrlZoneNum = 2;
    state->dataSize->CurZoneEqNum = CntrlZoneNum;
    FirstHVACIteration = true;
    state->dataSize->ZoneEqSizing(CntrlZoneNum).SizingMethod.allocate(HVAC::NumOfSizingTypes);
    state->dataSize->ZoneEqSizing(CntrlZoneNum).SizingMethod(HVAC::HeatingCapacitySizing) =
        state->dataElectBaseboardRad->ElecBaseboard(BaseboardNum).HeatingCapMethod;
    state->dataSize->FinalZoneSizing(CntrlZoneNum).NonAirSysDesHeatLoad = 2000.0;
    state->dataHeatBal->Zone(CntrlZoneNum).FloorArea = 100.0;
    // do electric baseboard sizing
    ElectricBaseboardRadiator::SizeElectricBaseboard(*state, BaseboardNum);
    // check user specified hardsized nominal capacity
    EXPECT_EQ(state->dataElectBaseboardRad->ElecBaseboard(BaseboardNum).ScaledHeatingCapacity, 30.0);
    EXPECT_EQ(state->dataElectBaseboardRad->ElecBaseboard(BaseboardNum).NominalCapacity, 3000.0);
    // check nominal capacity autosize
    state->dataElectBaseboardRad->ElecBaseboard(BaseboardNum).HeatingCapMethod = DataSizing::HeatingDesignCapacity;
    state->dataElectBaseboardRad->ElecBaseboard(BaseboardNum).ScaledHeatingCapacity = DataSizing::AutoSize;
    ElectricBaseboardRadiator::SizeElectricBaseboard(*state, BaseboardNum);
    EXPECT_EQ(state->dataElectBaseboardRad->ElecBaseboard(BaseboardNum).NominalCapacity, 2000.0);

    BaseboardNum = 3;
    CntrlZoneNum = 3;
    state->dataSize->CurZoneEqNum = CntrlZoneNum;
    FirstHVACIteration = true;
    state->dataSize->ZoneEqSizing(CntrlZoneNum).SizingMethod.allocate(HVAC::NumOfSizingTypes);
    state->dataSize->ZoneEqSizing(CntrlZoneNum).SizingMethod(HVAC::HeatingCapacitySizing) =
        state->dataElectBaseboardRad->ElecBaseboard(BaseboardNum).HeatingCapMethod;
    state->dataSize->FinalZoneSizing(CntrlZoneNum).NonAirSysDesHeatLoad = 3000.0;
    state->dataHeatBal->Zone(CntrlZoneNum).FloorArea = 100.0;
    // do electric baseboard sizing
    ElectricBaseboardRadiator::SizeElectricBaseboard(*state, BaseboardNum);
    // check user specified hardsized nominal capacity
    EXPECT_EQ(state->dataElectBaseboardRad->ElecBaseboard(BaseboardNum).ScaledHeatingCapacity, 0.50);
    EXPECT_EQ(state->dataElectBaseboardRad->ElecBaseboard(BaseboardNum).NominalCapacity, 1500.0);
    // check nominal capacity autosize
    state->dataElectBaseboardRad->ElecBaseboard(BaseboardNum).HeatingCapMethod = DataSizing::HeatingDesignCapacity;
    state->dataElectBaseboardRad->ElecBaseboard(BaseboardNum).ScaledHeatingCapacity = DataSizing::AutoSize;
    ElectricBaseboardRadiator::SizeElectricBaseboard(*state, BaseboardNum);
    EXPECT_EQ(state->dataElectBaseboardRad->ElecBaseboard(BaseboardNum).NominalCapacity, 3000.0);
}

TEST_F(EnergyPlusFixture, RadConvElecBaseboard_UpdateElectricBaseboardOff)
{
    // this unit test is related to issue #6276, unit was producing negative heating, new routines turning things off or calculating things when on
    // were added
    Real64 LoadMet;
    Real64 QBBCap;
    Real64 RadHeat;
    Real64 QBBElecRadSrc;
    Real64 ElecUseRate;
    Real64 AirOutletTemp;
    Real64 AirInletTemp;

    // Set conditions
    LoadMet = -1000.0;
    QBBCap = 500.0;
    RadHeat = 300.0;
    QBBElecRadSrc = 300.0;
    ElecUseRate = 600.0;
    AirOutletTemp = 25.0;
    AirInletTemp = 23.0;

    // Call the Off routine
    ElectricBaseboardRadiator::UpdateElectricBaseboardOff(LoadMet, QBBCap, RadHeat, QBBElecRadSrc, ElecUseRate, AirOutletTemp, AirInletTemp);

    // Check that it zeroed things out properly
    EXPECT_EQ(LoadMet, 0.0);
    EXPECT_EQ(QBBCap, 0.0);
    EXPECT_EQ(RadHeat, 0.0);
    EXPECT_EQ(QBBElecRadSrc, 0.0);
    EXPECT_EQ(ElecUseRate, 0.0);
    EXPECT_EQ(AirOutletTemp, AirInletTemp);
}

TEST_F(EnergyPlusFixture, RadConvElecBaseboard_UpdateElectricBaseboardOn)
{
    // this unit test is related to issue #6276, unit was producing negative heating, new routines turning things off or calculating things when on
    // were added
    Real64 AirOutletTemp;
    Real64 ElecUseRate;
    Real64 AirInletTemp;
    Real64 QBBCap;
    Real64 CapacitanceAir;
    Real64 Effic;

    // Set conditions
    AirOutletTemp = 0.0;
    ElecUseRate = 0.0;
    AirInletTemp = 20.0;
    QBBCap = 1200.0;
    CapacitanceAir = 1000.0;
    Effic = 0.5;

    // Call the On routine
    ElectricBaseboardRadiator::UpdateElectricBaseboardOn(AirOutletTemp, ElecUseRate, AirInletTemp, QBBCap, CapacitanceAir, Effic);

    // Check that the output variables (AirOutletTemp and ElecUseRate) were calculated properly
    // AirOutletTemp = AirInletTemp + QBBCap / CapacitanceAir;
    // ElecUseRate = QBBCap / Effic;
    EXPECT_NEAR(AirOutletTemp, 21.2, 0.0001);
    EXPECT_NEAR(ElecUseRate, 2400.0, 0.0001);
}

} // namespace EnergyPlus
