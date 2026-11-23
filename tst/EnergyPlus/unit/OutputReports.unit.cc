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

// EnergyPlus::OutputReportTabular Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/OutputReports.hh>
#include <EnergyPlus/SurfaceGeometry.hh>

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, OutputReports_SurfaceDetailsReport)
{

    std::string const idf_objects = delimited_string({"Zone,",
                                                      "  Space1,                !- Name",
                                                      "  0.0000,                  !- Direction of Relative North {deg}",
                                                      "  0.0000,                  !- X Origin {m}",
                                                      "  0.0000,                  !- Y Origin {m}",
                                                      "  0.0000,                  !- Z Origin {m}",
                                                      "  1,                       !- Type",
                                                      "  1,                       !- Multiplier",
                                                      "  2.4,                     !- Ceiling Height {m}",
                                                      "  ,                        !- Volume {m3}",
                                                      "  autocalculate,           !- Floor Area {m2}",
                                                      "  ,                        !- Zone Inside Convection Algorithm",
                                                      "  ,                        !- Zone Outside Convection Algorithm",
                                                      "  Yes;                     !- Part of Total Floor Area",
                                                      "BuildingSurface:Detailed,",
                                                      " FRONT-1,                  !- Name",
                                                      " WALL,                     !- Surface Type",
                                                      " INT-WALL-1,               !- Construction Name",
                                                      " Space1,                    !- Zone Name",
                                                      "    ,                        !- Space Name",
                                                      " Outdoors,                 !- Outside Boundary Condition",
                                                      " ,                         !- Outside Boundary Condition Object",
                                                      " SunExposed,               !- Sun Exposure",
                                                      " WindExposed,              !- Wind Exposure",
                                                      " 0.50000,                  !- View Factor to Ground",
                                                      " 4,                        !- Number of Vertices",
                                                      " 0.0, 0.0, 2.4,            !- X, Y, Z == > Vertex 1 {m}",
                                                      " 0.0, 0.0, 0.0,            !- X, Y, Z == > Vertex 2 {m}",
                                                      " 30.5, 0.0, 0.0,           !- X, Y, Z == > Vertex 3 {m}",
                                                      " 30.5, 0.0, 2.4;           !- X, Y, Z == > Vertex 4 {m}",
                                                      "Construction,",
                                                      " INT-WALL-1,               !- Name",
                                                      " GP02,                     !- Outside Layer",
                                                      " AL21,                     !- Layer 2",
                                                      " GP02;                     !- Layer 3",
                                                      " ",
                                                      "Material,",
                                                      " GP02,                     !- Name",
                                                      " MediumSmooth,             !- Roughness",
                                                      " 1.5900001E-02,            !- Thickness{ m }",
                                                      " 0.1600000,                !- Conductivity{ W / m - K }",
                                                      " 801.0000,                 !- Density{ kg / m3 }",
                                                      " 837.0000,                 !- Specific Heat{ J / kg - K }",
                                                      " 0.9000000,                !- Thermal Absorptance",
                                                      " 0.7500000,                !- Solar Absorptance",
                                                      " 0.7500000;                !- Visible Absorptance",
                                                      " ",
                                                      "Material:AirGap,",
                                                      " AL21,                     !- Name",
                                                      " 0.1570000;                !- Thermal Resistance{ m2 - K / W }",
                                                      " ",
                                                      "Output:Surfaces:List,Details;"});

    ASSERT_TRUE(process_idf(idf_objects));

    bool foundErrors(false);
    HeatBalanceManager::GetProjectControlData(*state, foundErrors); // read project control data
    EXPECT_FALSE(foundErrors);                                      // expect no errors

    Material::GetMaterialData(*state, foundErrors); // read material data
    EXPECT_FALSE(foundErrors);                      // expect no errors

    HeatBalanceManager::GetConstructData(*state, foundErrors); // read construction data
    compare_err_stream("");
    EXPECT_FALSE(foundErrors); // expect no errors

    HeatBalanceManager::GetZoneData(*state, foundErrors); // read zone data
    EXPECT_FALSE(foundErrors);                            // expect no errors

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(1);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(1);

    state->dataSurfaceGeometry->CosZoneRelNorth(1) = std::cos(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(1) = std::sin(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;

    SurfaceGeometry::GetSurfaceData(*state, foundErrors); // setup zone geometry and get zone data
    EXPECT_FALSE(foundErrors);                            // expect no errors

    // reset eio stream
    has_eio_output(true);

    DetailsForSurfaces(*state, 10); // 10 = Details Only, Surface details report
    std::string const eiooutput = delimited_string(
        {"! <Zone Surfaces>,Zone Name,# Surfaces",
         "! <Shading Surfaces>,Number of Shading Surfaces,# Surfaces",
         "! <HeatTransfer Surface>,Surface Name,Surface Class,Base Surface,Heat Transfer Algorithm,Construction,Nominal U (w/o film coefs) "
         "{W/m2-K},Nominal U (with film coefs) {W/m2-K},Solar Diffusing,Area (Net) {m2},Area (Gross) {m2},Area (Sunlit Calc) {m2},Azimuth {deg},Tilt "
         "{deg},~Width {m},~Height {m},Reveal "
         "{m},ExtBoundCondition,ExtConvCoeffCalc,IntConvCoeffCalc,SunExposure,WindExposure,ViewFactorToGround,ViewFactorToSky,ViewFactorToGround-IR,"
         "ViewFactorToSky-IR,#Sides",
         "! <Shading Surface>,Surface Name,Surface Class,Base Surface,Heat Transfer Algorithm,Transmittance Schedule,Min Schedule Value,Max Schedule "
         "Value,Solar Diffusing,Area (Net) {m2},Area (Gross) {m2},Area (Sunlit Calc) {m2},Azimuth {deg},Tilt {deg},~Width {m},~Height {m},Reveal "
         "{m},ExtBoundCondition,ExtConvCoeffCalc,IntConvCoeffCalc,SunExposure,WindExposure,ViewFactorToGround,ViewFactorToSky,ViewFactorToGround-IR,"
         "ViewFactorToSky-IR,#Sides",
         "! <Frame/Divider Surface>,Surface Name,Surface Class,Base Surface,Heat Transfer Algorithm,Construction,Nominal U (w/o film coefs) "
         "{W/m2-K},Nominal U (with film coefs) {W/m2-K},Solar Diffusing,Area (Net) {m2},Area (Gross) {m2},Area (Sunlit Calc) {m2},Azimuth {deg},Tilt "
         "{deg},~Width {m},~Height {m},Reveal {m}",
         "Zone Surfaces,SPACE1,1",
         "HeatTransfer Surface,FRONT-1,Wall,,CTF - "
         "ConductionTransferFunction,INT-WALL-1,2.81096,1.97846,,73.20,73.20,73.20,180.00,90.00,30.50,2.40,0.00,ExternalEnvironment,DOE-2,ASHRAETARP,"
         "SunExposed,WindExposed,0.50000,0.50000,0.50000,0.50000,4"},
        "\n");

    EXPECT_TRUE(compare_eio_stream(eiooutput, true));
}
