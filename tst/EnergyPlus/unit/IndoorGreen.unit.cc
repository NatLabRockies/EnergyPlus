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

#include <exception>

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataViewFactorInformation.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/HeatBalanceIntRadExchange.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/HeatBalanceSurfaceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/IndoorGreen.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/SolarShading.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

#include <algorithm>
#include <iterator>
#include <vector>

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, IndoorGreen_CheckGetInputDataFunction)
{
    bool ErrorsFound(false);
    int IndoorGreenNum(1);
    std::string const idf_objects = delimited_string({

        " Zone,",
        "    SPACE1-1,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0.0,                     !- X Origin {m}",
        "    0.0,                     !- Y Origin {m}",
        "    0.0,                     !- Z Origin {m}",
        "    ,                        !- Type",
        "    ,                        !- Multiplier",
        "    ,                        !- Ceiling Height {m}",
        "    ,                        !- Volume {m3}",
        "    ,                        !- Floor Area {m2}",
        "    ,                        !- Zone Inside Convection Algorithm",
        "    ,                        !- Zone Outside Convection Algorithm",
        "    No;                      !- Part of Total Floor Area",

        " Construction,",
        "    INT-WALL,                !- Name",
        "    GP02;                    !- Outside Layer",

        " Material,",
        "    GP02,                    !- Name",
        "    MediumSmooth,            !- Roughness",
        "    0.1016,                  !- Thickness {m}",
        "    0.16,                    !- Conductivity {W/m-K}",
        "    801,                     !- Density {kg/m3}",
        "    837,                     !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.75,                    !- Solar Absorptance",
        "    0.75;                    !- Visible Absorptance",

        "  BuildingSurface:Detailed,",
        "    SPACE1-1SouthPartition,  !-Name",
        "    WALL,                        !-Surface Type",
        "    INT-WALL,                    !-Construction Name",
        "    SPACE1-1,                    !-Zone Name",
        "    ,                            !-Space Name",
        "    Adiabatic,                   !-Outside Boundary Condition",
        "    ,                            !-Outside Boundary Condition Object",
        "    NoSun,                       !-Sun Exposure",
        "    NoWind,                      !-Wind Exposure",
        "    0,                           !-View Factor to Ground",
        "    4,                           !-Number of Vertices",
        "    5, 1.5, 2,                   !- X,Y,Z ==> Vertex 1 {m}",
        "    5, 1.5, 0.0,                 !- X,Y,Z ==> Vertex 2 {m}",
        "    20, 1.5, 0.0,                !- X,Y,Z ==> Vertex 3 {m}",
        "    20, 1.5, 2;                  !- X,Y,Z ==> Vertex 3 {m}",

        "  Schedule:Compact,",
        "    AlwaysOn,                !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    1.0;                     !- Field 4",

        "  Schedule:Compact,",
        "    AlwaysOff,                !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    0.0;                     !- Field 4",

        "  IndoorLivingWall,",
        "    Space1-1IndoorLivingWall, !-Name",
        "    SPACE1-1SouthPartition, !-Surface Name",
        "    AlwaysOn,                   !-Schedule Name",
        "    Penman-Monteith,            !-ET Calculation Method",
        "    LED,                        !-Lighting Method",
        "    AlwaysOff,                  !-LED Intensity Schedule Name",
        "    ,                           !-Daylighting Control Name",
        "    ,                           !-LED - Daylight Targeted Lighting Intensity Schedule Name",
        "    30,                         !- Total Leaf Area {m2}",
        "    32.5,                       !- LED Nominal Intensity {umol_m2s}",
        "    640,                        !- LED Nominal Power {W}",
        "    0.6;                        !- Radiant Fraction of LED Lights",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    Material::GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    HeatBalanceManager::GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(1);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(1);
    state->dataSurfaceGeometry->CosZoneRelNorth(1) = std::cos(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(1) = std::sin(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;

    SurfaceGeometry::GetSurfaceData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    state->dataHVACGlobal->TimeStepSys = 1;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;

    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60 / state->dataGlobal->TimeStepsInHour;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->HourOfDay = 1;
    state->dataEnvrn->DayOfWeek = 1;
    state->dataEnvrn->DayOfYear_Schedule = 1;
    Sched::UpdateScheduleVals(*state);

    IndoorGreen::GetIndoorGreenInput(*state, ErrorsFound); // setup indoor living wall data
    EXPECT_FALSE(ErrorsFound);                             // expect no errors
    auto &thisindoorgreen = state->dataIndoorGreen->indoorGreens(IndoorGreenNum);
    EXPECT_EQ(thisindoorgreen.LeafArea, 30.0);
    EXPECT_EQ((int)thisindoorgreen.lightingMethod, (int)IndoorGreen::LightingMethod::LED);
    EXPECT_EQ((int)thisindoorgreen.etCalculationMethod, (int)IndoorGreen::ETCalculationMethod::PenmanMonteith);
}
TEST_F(EnergyPlusFixture, IndoorGreen_CheckETFunction)
{
    int IndoorGreenNum(1);
    Real64 ZonePreTemp = 0.0; // C
    Real64 ZonePreHum = 0.001;
    Real64 LAI = 1.0;
    Real64 SwithF = 1.0;
    state->dataIndoorGreen->indoorGreens.allocate(IndoorGreenNum);
    auto &thisindoorgreen = state->dataIndoorGreen->indoorGreens(IndoorGreenNum);
    thisindoorgreen.ZCO2 = 400;  // ppm
    thisindoorgreen.ZPPFD = 0;   // umol/(m2s)
    thisindoorgreen.ZVPD = 2000; // Pa
    thisindoorgreen.ETRate = IndoorGreen::ETBaseFunction(*state, ZonePreTemp, ZonePreHum, thisindoorgreen.ZPPFD, thisindoorgreen.ZVPD, LAI, SwithF);
    EXPECT_EQ(thisindoorgreen.ETRate, 0.0);
}
