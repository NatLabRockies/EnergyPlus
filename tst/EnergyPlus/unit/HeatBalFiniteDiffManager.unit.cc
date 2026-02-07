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

// EnergyPlus::HeatBalFiniteDiffManager Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/HeatBalFiniteDiffManager.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/PhaseChangeModeling/HysteresisModel.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus::HeatBalFiniteDiffManager;
using namespace EnergyPlus::HeatBalanceManager;

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, HeatBalFiniteDiffManager_CalcNodeHeatFluxTest)
{
    auto &SurfaceFD = state->dataHeatBalFiniteDiffMgr->SurfaceFD;
    int constexpr numNodes(4);
    Real64 constexpr allowedTolerance = 0.0001;
    int nodeNum(0);
    SurfaceFD.allocate(1);
    int constexpr SurfNum(1);
    auto &surfFD = SurfaceFD(SurfNum);
    surfFD.QDreport.allocate(numNodes + 1);
    surfFD.TDpriortimestep.allocate(numNodes + 1);
    surfFD.TDT.allocate(numNodes + 1);
    surfFD.CpDelXRhoS1.allocate(numNodes + 1);
    surfFD.CpDelXRhoS2.allocate(numNodes + 1);
    state->dataHeatBalSurf->SurfOpaqInsFaceCondFlux.allocate(1);
    state->dataHeatBalSurf->SurfOpaqOutFaceCondFlux.allocate(1);
    state->dataGlobal->TimeStepZoneSec = 600.0;

    Real64 expectedResult1(0.0);
    Real64 expectedResult2(0.0);
    Real64 expectedResult3(0.0);
    Real64 expectedResult4(0.0);
    Real64 expectedResult5(0.0);
    Real64 expectedResultO(1.0);

    // Steady-state case
    state->dataHeatBalSurf->SurfOpaqInsFaceCondFlux(SurfNum) = 100.0;
    nodeNum = 1;
    surfFD.TDpriortimestep(nodeNum) = 20.0;
    surfFD.TDT(nodeNum) = 20.0;
    surfFD.CpDelXRhoS1(nodeNum) = 1000.0;
    surfFD.CpDelXRhoS2(nodeNum) = 2000.0;
    expectedResult1 = state->dataHeatBalSurf->SurfOpaqInsFaceCondFlux(SurfNum);

    nodeNum = 2;
    surfFD.TDpriortimestep(nodeNum) = 22.0;
    surfFD.TDT(nodeNum) = 22.0;
    surfFD.CpDelXRhoS1(nodeNum) = 1000.0;
    surfFD.CpDelXRhoS2(nodeNum) = 2000.0;
    expectedResult2 = state->dataHeatBalSurf->SurfOpaqInsFaceCondFlux(SurfNum);

    nodeNum = 3;
    surfFD.TDpriortimestep(nodeNum) = 23.0;
    surfFD.TDT(nodeNum) = 23.0;
    surfFD.CpDelXRhoS1(nodeNum) = 1000.0;
    surfFD.CpDelXRhoS2(nodeNum) = 2000.0;
    expectedResult3 = state->dataHeatBalSurf->SurfOpaqInsFaceCondFlux(SurfNum);

    nodeNum = 4;
    surfFD.TDpriortimestep(nodeNum) = 26.0;
    surfFD.TDT(nodeNum) = 26.0;
    surfFD.CpDelXRhoS1(nodeNum) = 1000.0;
    surfFD.CpDelXRhoS2(nodeNum) = 2000.0;
    expectedResult4 = state->dataHeatBalSurf->SurfOpaqInsFaceCondFlux(SurfNum);

    nodeNum = 5;
    surfFD.TDpriortimestep(nodeNum) = 27.0;
    surfFD.TDT(nodeNum) = 27.0;
    surfFD.CpDelXRhoS1(nodeNum) = 1000.0;
    surfFD.CpDelXRhoS2(nodeNum) = 2000.0;
    expectedResult5 = state->dataHeatBalSurf->SurfOpaqInsFaceCondFlux(SurfNum);

    state->dataEnvrn->IsRain = false;
    state->dataHeatBalSurf->SurfOpaqOutFaceCondFlux(SurfNum) = 77.0;
    expectedResultO = 77.0;

    CalcNodeHeatFlux(*state, SurfNum, numNodes);
    EXPECT_NEAR(surfFD.QDreport(1), expectedResult1, allowedTolerance);
    EXPECT_NEAR(surfFD.QDreport(2), expectedResult2, allowedTolerance);
    EXPECT_NEAR(surfFD.QDreport(3), expectedResult3, allowedTolerance);
    EXPECT_NEAR(surfFD.QDreport(4), expectedResult4, allowedTolerance);
    EXPECT_NEAR(surfFD.QDreport(5), expectedResult5, allowedTolerance);
    EXPECT_NEAR(state->dataHeatBalSurf->SurfOpaqOutFaceCondFlux(SurfNum), expectedResultO, allowedTolerance);

    state->dataEnvrn->IsRain = true;
    state->dataHeatBalSurf->SurfOpaqOutFaceCondFlux(SurfNum) = 77.0;
    expectedResultO = -state->dataHeatBalSurf->SurfOpaqInsFaceCondFlux(SurfNum);

    CalcNodeHeatFlux(*state, SurfNum, numNodes);
    EXPECT_NEAR(surfFD.QDreport(1), expectedResult1, allowedTolerance);
    EXPECT_NEAR(surfFD.QDreport(2), expectedResult2, allowedTolerance);
    EXPECT_NEAR(surfFD.QDreport(3), expectedResult3, allowedTolerance);
    EXPECT_NEAR(surfFD.QDreport(4), expectedResult4, allowedTolerance);
    EXPECT_NEAR(surfFD.QDreport(5), expectedResult5, allowedTolerance);
    EXPECT_NEAR(state->dataHeatBalSurf->SurfOpaqOutFaceCondFlux(SurfNum), expectedResultO, allowedTolerance);

    // Reset
    surfFD.QDreport = 0.0;
    expectedResult1 = 0.0;
    expectedResult2 = 0.0;
    expectedResult3 = 0.0;
    expectedResult4 = 0.0;
    expectedResult5 = 0.0;

    // Unsteady-state case
    state->dataGlobal->TimeStepZoneSec = 600.0;
    state->dataHeatBalSurf->SurfOpaqInsFaceCondFlux(SurfNum) = -200.0;

    nodeNum = 5;
    surfFD.TDpriortimestep(nodeNum) = 27.5;
    surfFD.TDT(nodeNum) = 27.0;
    surfFD.CpDelXRhoS1(nodeNum) = 0.0;
    surfFD.CpDelXRhoS2(nodeNum) = 0.0;
    expectedResult5 = state->dataHeatBalSurf->SurfOpaqInsFaceCondFlux(SurfNum);

    nodeNum = 4;
    surfFD.TDpriortimestep(nodeNum) = 26.0;
    surfFD.TDT(nodeNum) = 26.0;
    surfFD.CpDelXRhoS1(nodeNum) = 0.0;
    surfFD.CpDelXRhoS2(nodeNum) = 2000.0;
    expectedResult4 = expectedResult5; // r-layer with zero heat capacity, so flux passes through

    nodeNum = 3;
    surfFD.TDpriortimestep(nodeNum) = 23.0;
    surfFD.TDT(nodeNum) = 23.0;
    surfFD.CpDelXRhoS1(nodeNum) = 1000.0;
    surfFD.CpDelXRhoS2(nodeNum) = 2000.0;
    expectedResult3 = expectedResult4; // no change in temperature at nodes 4 and 3, so flux passes through

    nodeNum = 2;
    surfFD.TDpriortimestep(nodeNum) = 22.2;
    surfFD.TDT(nodeNum) = 22.0;
    surfFD.CpDelXRhoS1(nodeNum) = 1000.0;
    surfFD.CpDelXRhoS2(nodeNum) = 2000.0;
    expectedResult2 =
        expectedResult3 + (surfFD.TDT(nodeNum) - surfFD.TDpriortimestep(nodeNum)) * surfFD.CpDelXRhoS2(nodeNum) / state->dataGlobal->TimeStepZoneSec;

    nodeNum = 1;
    surfFD.TDpriortimestep(nodeNum) = 20.1;
    surfFD.TDT(nodeNum) = 20.0;
    surfFD.CpDelXRhoS1(nodeNum) = 1000.0;
    surfFD.CpDelXRhoS2(nodeNum) = 2000.0;
    expectedResult1 = expectedResult2 + (surfFD.TDT(nodeNum + 1) - surfFD.TDpriortimestep(nodeNum + 1)) * surfFD.CpDelXRhoS1(nodeNum + 1) /
                                            state->dataGlobal->TimeStepZoneSec;
    expectedResult1 =
        expectedResult1 + (surfFD.TDT(nodeNum) - surfFD.TDpriortimestep(nodeNum)) * surfFD.CpDelXRhoS2(nodeNum) / state->dataGlobal->TimeStepZoneSec;

    state->dataEnvrn->IsRain = false;
    state->dataHeatBalSurf->SurfOpaqOutFaceCondFlux(SurfNum) = 123.0;
    expectedResultO = 123.0;

    CalcNodeHeatFlux(*state, SurfNum, numNodes);
    EXPECT_NEAR(surfFD.QDreport(1), expectedResult1, allowedTolerance);
    EXPECT_NEAR(surfFD.QDreport(2), expectedResult2, allowedTolerance);
    EXPECT_NEAR(surfFD.QDreport(3), expectedResult3, allowedTolerance);
    EXPECT_NEAR(surfFD.QDreport(4), expectedResult4, allowedTolerance);
    EXPECT_NEAR(surfFD.QDreport(5), expectedResult5, allowedTolerance);
    EXPECT_NEAR(state->dataHeatBalSurf->SurfOpaqOutFaceCondFlux(SurfNum), expectedResultO, allowedTolerance);

    state->dataEnvrn->IsRain = true;
    state->dataHeatBalSurf->SurfOpaqOutFaceCondFlux(SurfNum) = 123.0;
    expectedResultO = -expectedResult1;

    CalcNodeHeatFlux(*state, SurfNum, numNodes);
    EXPECT_NEAR(surfFD.QDreport(1), expectedResult1, allowedTolerance);
    EXPECT_NEAR(surfFD.QDreport(2), expectedResult2, allowedTolerance);
    EXPECT_NEAR(surfFD.QDreport(3), expectedResult3, allowedTolerance);
    EXPECT_NEAR(surfFD.QDreport(4), expectedResult4, allowedTolerance);
    EXPECT_NEAR(surfFD.QDreport(5), expectedResult5, allowedTolerance);
    EXPECT_NEAR(state->dataHeatBalSurf->SurfOpaqOutFaceCondFlux(SurfNum), expectedResultO, allowedTolerance);
}

TEST_F(EnergyPlusFixture, HeatBalFiniteDiffManager_adjustPropertiesForPhaseChange)
{
    auto &s_mat = state->dataMaterial;

    // create a single PCM object in the input and process it
    std::string const idf_objects = delimited_string({"  MaterialProperty:PhaseChangeHysteresis,",
                                                      "    PCMNAME,   !- Name",
                                                      "    10000,                   !- Latent Heat during the Entire Phase Change Process {J/kg}",
                                                      "    1.5,                     !- Liquid State Thermal Conductivity {W/m-K}",
                                                      "    2200,                    !- Liquid State Density {kg/m3}",
                                                      "    2000,                    !- Liquid State Specific Heat {J/kg-K}",
                                                      "    1,                       !- High Temperature Difference of Melting Curve {deltaC}",
                                                      "    20,                      !- Peak Melting Temperature {C}",
                                                      "    1,                       !- Low Temperature Difference of Melting Curve {deltaC}",
                                                      "    1.8,                     !- Solid State Thermal Conductivity {W/m-K}",
                                                      "    2300,                    !- Solid State Density {kg/m3}",
                                                      "    2000,                    !- Solid State Specific Heat {J/kg-K}",
                                                      "    1,                       !- High Temperature Difference of Freezing Curve {deltaC}",
                                                      "    23,                      !- Peak Freezing Temperature {C}",
                                                      "    1;                       !- Low Temperature Difference of Freezing Curve {deltaC}"});
    ASSERT_TRUE(process_idf(idf_objects, false));

    // allocate a finite difference surface object and needed member variables
    int constexpr surfaceIndex = 1;
    int constexpr finiteDiffLayerIndex = 1;
    auto &SurfaceFD = state->dataHeatBalFiniteDiffMgr->SurfaceFD;
    SurfaceFD.allocate(1);
    SurfaceFD(surfaceIndex).PhaseChangeTemperatureReverse.allocate(1);
    SurfaceFD(surfaceIndex).PhaseChangeTemperatureReverse(finiteDiffLayerIndex) = 20.0;
    SurfaceFD(surfaceIndex).PhaseChangeState.allocate(1);
    SurfaceFD(surfaceIndex).PhaseChangeState(finiteDiffLayerIndex) = Material::Phase::Liquid;
    SurfaceFD(surfaceIndex).PhaseChangeStateOld.allocate(1);
    SurfaceFD(surfaceIndex).PhaseChangeStateOld(finiteDiffLayerIndex) = Material::Phase::Melting;
    SurfaceFD(surfaceIndex).PhaseChangeStateRep.allocate(1);
    SurfaceFD(surfaceIndex).PhaseChangeStateRep(finiteDiffLayerIndex) = Material::phaseInts[(int)Material::Phase::Liquid];
    SurfaceFD(surfaceIndex).PhaseChangeStateOldRep.allocate(1);
    SurfaceFD(surfaceIndex).PhaseChangeStateOldRep(finiteDiffLayerIndex) = Material::phaseInts[(int)Material::Phase::Melting];

    // create a materials data object and assign the phase change variable based on above IDF processing
    auto *mat = new Material::MaterialBase;
    mat->Name = "PCMNAME";
    mat->group = Material::Group::Regular;
    s_mat->materials.push_back(mat);
    mat->Num = s_mat->materials.isize();
    s_mat->materialMap.insert_or_assign(mat->Name, mat->Num);

    bool ErrorsFound;
    Material::GetHysteresisData(*state, ErrorsFound);

    auto *matPC = dynamic_cast<Material::MaterialPhaseChange *>(s_mat->materials(Material::GetMaterialNum(*state, "PCMNAME")));

    // create local variables to calculate and call the new worker function
    Real64 newSpecificHeat, newDensity, newThermalConductivity;
    adjustPropertiesForPhaseChange(
        *state, finiteDiffLayerIndex, surfaceIndex, matPC, 20.0, 20.1, newSpecificHeat, newDensity, newThermalConductivity);

    // check the values are correct
    EXPECT_NEAR(10187.3, newSpecificHeat, 0.1);
    EXPECT_NEAR(2250, newDensity, 0.1);
    EXPECT_NEAR(1.65, newThermalConductivity, 0.1);

    // deallocate
    SurfaceFD.deallocate();
}

// I'm not sure how this test was intended to work, there doesn't appear to be anything setting the constructions
// to finite difference, and there aren't any surfaces.  So it fails when enabled.  Feel free to fix it up and get it running.
// TEST_F(EnergyPlusFixture, HeatBalFiniteDiffManager_skipNotUsedConstructionAndAirLayer)
// {
//     bool ErrorsFound(false);
//     // create three construction objects with one object not in use and another object assigned to surfaces, and one object as air wall.
//     std::string const idf_objects = delimited_string({
//         "Material,",
//         "   MAT - CC05 4 HW CONCRETE, !- Name",
//         "   Rough, !- Roughness",
//         "   0.1016, !- Thickness{ m }",
//         "   1.311, !- Conductivity{ W / m - K }",
//         "   2240, !- Density{ kg / m3 }",
//         "   836.800000000001, !- Specific Heat{ J / kg - K }",
//         "   0.9, !- Thermal Absorptance",
//         "   0.85, !- Solar Absorptance",
//         "   0.85;                    !- Visible Absorptance",
//         "Material:AirGap,",
//         "   F05 Ceiling air space resistance, !- Name",
//         "   0.18;                    !- Thermal Resistance{ m2 - K / W }",
//         "Material:NoMass,",
//         "   CP02 CARPET PAD, !- Name",
//         "   Smooth, !- Roughness",
//         "   0.1, !- Thermal Resistance{ m2 - K / W }",
//         "   0.9, !- Thermal Absorptance",
//         "   0.8, !- Solar Absorptance",
//         "   0.8;                     !- Visible Absorptance",
//
//         "Material,",
//         "   F16 Acoustic tile, !- Name",
//         "   MediumSmooth, !- Roughness",
//         "   0.0191, !- Thickness{ m }",
//         "   0.06, !- Conductivity{ W / m - K }",
//         "   368, !- Density{ kg / m3 }",
//         "   590.000000000002, !- Specific Heat{ J / kg - K }",
//         "   0.9, !- Thermal Absorptance",
//         "   0.3, !- Solar Absorptance",
//         "   0.3;                     !- Visible Absorptance",
//
//         "Material,",
//         "   M11 100mm lightweight concrete, !- Name",
//         "   MediumRough, !- Roughness",
//         "   0.1016, !- Thickness{ m }",
//         "   0.53, !- Conductivity{ W / m - K }",
//         "   1280, !- Density{ kg / m3 }",
//         "   840.000000000002, !- Specific Heat{ J / kg - K }",
//         "   0.9, !- Thermal Absorptance",
//         "   0.5, !- Solar Absorptance",
//         "   0.5;                     !- Visible Absorptance",
//
//         "Construction,",
//         "  ExtSlabCarpet 4in ClimateZone 1 - 8, !- Name",
//         "  MAT - CC05 4 HW CONCRETE, !- Outside Layer",
//         "  CP02 CARPET PAD;         !- Layer 2",
//         "Construction,",
//         "   Interior Floor, !- Name",
//         "   F16 Acoustic tile, !- Outside Layer",
//         "   F05 Ceiling air space resistance, !- Layer 2",
//         "   M11 100mm lightweight concrete;  !- Layer 3",
//         "Construction:AirBoundary,",
//         "   Air Wall_ConstructionAirBoundary,  !- Name",
//         "   None,                    !- Air Exchange Method",
//         "   0;                       !- Simple Mixing Air Changes per Hour {1 / hr}",
//         "Output:Constructions,",
//         "Constructions;",
//         "Output:Constructions,",
//         "Materials;",
//     });
//
//     ASSERT_TRUE(process_idf(idf_objects));
//
//     state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
//     state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
//
//     ErrorsFound = false;
//     Material::GetMaterialData(*state, ErrorsFound); // read material data
//     EXPECT_FALSE(ErrorsFound);                      // expect no errors
//
//     ErrorsFound = false;
//     HeatBalanceManager::GetConstructData(*state, ErrorsFound); // read construction data
//     EXPECT_FALSE(ErrorsFound);                                 // expect no errors
//
//     // allocate properties for construction objects when it is used or not for building surfaces in the model
//
//     state->dataConstruction->Construct(1).IsUsed = false;
//     state->dataConstruction->Construct(2).IsUsed = true;
//     state->dataConstruction->Construct(3).IsUsed = true;
//
//     // call the function for initialization of finite difference calculation
//     InitialInitHeatBalFiniteDiff(*state);
//     auto &ConstructFD = state->dataHeatBalFiniteDiffMgr->ConstructFD;
//     // check the values are correct
//     EXPECT_EQ(0, ConstructFD(1).Name.size());
//     EXPECT_EQ(3, ConstructFD(2).Name.size());
//     EXPECT_EQ(0, ConstructFD(3).Name.size());
//     EXPECT_EQ("F16 ACOUSTIC TILE", ConstructFD(2).Name(1));
//     EXPECT_EQ("F05 CEILING AIR SPACE RESISTANCE", ConstructFD(2).Name(2));
//     EXPECT_EQ("M11 100MM LIGHTWEIGHT CONCRETE", ConstructFD(2).Name(3));
//
//     // deallocate
//     ConstructFD.deallocate();
// }

TEST_F(EnergyPlusFixture, HeatBalFiniteDiffManager_findAnySurfacesUsingConstructionAndCondFDTest)
{
    bool ErrorsFound(false);
    std::string const idf_objects = delimited_string({
        "Material:AirGap,",
        "   F05 Ceiling air space resistance, !- Name",
        "   0.18;                    !- Thermal Resistance{ m2 - K / W }",
        "Material,",
        "   Reg Mat F05 Ceiling air space resistance, !- Name",
        "   VerySmooth, !- Roughness",
        "   0.36, !- Thickness{ m }",
        "   2.00, !- Conductivity{ W / m - K }",
        "   1.23, !- Density{ kg / m3 }",
        "   1000.0, !- Specific Heat{ J / kg - K }",
        "   0.9, !- Thermal Absorptance",
        "   0.7, !- Solar Absorptance",
        "   0.7; !- Visible Absorptance",
        "Material,",
        "   F16 Acoustic tile, !- Name",
        "   MediumSmooth, !- Roughness",
        "   0.0191, !- Thickness{ m }",
        "   0.06, !- Conductivity{ W / m - K }",
        "   368, !- Density{ kg / m3 }",
        "   590.000000000002, !- Specific Heat{ J / kg - K }",
        "   0.9, !- Thermal Absorptance",
        "   0.3, !- Solar Absorptance",
        "   0.3;                     !- Visible Absorptance",
        "Material,",
        "   M11 100mm lightweight concrete, !- Name",
        "   MediumRough, !- Roughness",
        "   0.1016, !- Thickness{ m }",
        "   0.53, !- Conductivity{ W / m - K }",
        "   1280, !- Density{ kg / m3 }",
        "   840.000000000002, !- Specific Heat{ J / kg - K }",
        "   0.9, !- Thermal Absorptance",
        "   0.5, !- Solar Absorptance",
        "   0.5;                     !- Visible Absorptance",
        "Construction,",
        "   Interior Floor, !- Name",
        "   F16 Acoustic tile, !- Outside Layer",
        "   F05 Ceiling air space resistance, !- Layer 2",
        "   M11 100mm lightweight concrete;  !- Layer 3",
        "Construction,",
        "   Interior Floor All Reg Mats,  !- Name",
        "   F16 Acoustic tile, !- Outside Layer",
        "   Reg Mat F05 Ceiling air space resistance, !- Layer 2",
        "   M11 100mm lightweight concrete;  !- Layer 3",
        "Output:Constructions,",
        "Constructions;",
        "Output:Constructions,",
        "Materials;",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    ErrorsFound = false;
    Material::GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);                      // expect no errors

    ErrorsFound = false;
    HeatBalanceManager::GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);                                 // expect no errors

    // allocate properties for construction objects when it is used or not for building surfaces in the model
    state->dataConstruction->Construct(1).IsUsed = true;
    state->dataConstruction->Construct(2).IsUsed = true;

    auto &thisData = state->dataSurface;
    thisData->TotSurfaces = 2;
    thisData->Surface.allocate(thisData->TotSurfaces);
    thisData->Surface(1).Construction = 1;
    thisData->Surface(2).Construction = 2;
    thisData->Surface(1).HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::CondFD;
    thisData->Surface(2).HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::CondFD;
    state->dataHeatBalSurf->SurfOpaqInsFaceCondFlux.allocate(thisData->TotSurfaces);
    state->dataHeatBalSurf->SurfOpaqOutFaceCondFlux.allocate(thisData->TotSurfaces);
    state->dataGlobal->TimeStepZoneSec = 600.0;
    state->dataGlobal->TimeStepsInHour = 6;

    // call the function for initialization of finite difference calculation
    std::string const error_string = delimited_string({"   ** Severe  ** InitialInitHeatBalFiniteDiff: Found Material that is too thin and/or too "
                                                       "highly conductive, material name = Reg Mat F05 Ceiling air space resistance",
                                                       "   **   ~~~   ** High conductivity Material layers are not well supported by Conduction "
                                                       "Finite Difference, material conductivity = 2.000 [W/m-K]",
                                                       "   **   ~~~   ** Material thermal diffusivity = 1.626E-003 [m2/s]",
                                                       "   **   ~~~   ** Material with this thermal diffusivity should have thickness > 1.71080 [m]",
                                                       "   **  Fatal  ** Preceding conditions cause termination.",
                                                       "   ...Summary of Errors that led to program termination:",
                                                       "   ..... Reference severe error count=1",
                                                       "   ..... Last severe error=InitialInitHeatBalFiniteDiff: Found Material that is too thin "
                                                       "and/or too highly conductive, material name = Reg Mat F05 Ceiling air space resistance"});
    EXPECT_ANY_THROW(InitialInitHeatBalFiniteDiff(*state));

    compare_err_stream(error_string, true);

    // test new function directly
    thisData->Surface(2).HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::CTF;
    EXPECT_TRUE(EnergyPlus::HeatBalFiniteDiffManager::findAnySurfacesUsingConstructionAndCondFD(*state, 1));
    EXPECT_FALSE(EnergyPlus::HeatBalFiniteDiffManager::findAnySurfacesUsingConstructionAndCondFD(*state, 2));
}

TEST_F(EnergyPlusFixture, HeatBalFiniteDiffManager_CheckFDNodeTempLimitsTest)
{
    int surfNum;
    int nodeNum;
    Real64 nodeTemp;
    Real64 expectedAnswer;

    auto &thisData = state->dataSurface;
    auto &thisSurf = thisData->Surface;
    auto &thisSurfFD = state->dataHeatBalFiniteDiffMgr->SurfaceFD;
    thisData->TotSurfaces = 2;
    thisSurf.allocate(thisData->TotSurfaces);
    thisSurfFD.allocate(thisData->TotSurfaces);

    thisSurf(1).Name = "CONDFD SURFACE 1";
    thisSurf(2).Name = "CONDFD SURFACE 2";
    thisSurfFD(1).indexNodeMinTempLimit = 0;
    thisSurfFD(1).indexNodeMaxTempLimit = 0;
    thisSurfFD(2).indexNodeMinTempLimit = 0;
    thisSurfFD(2).indexNodeMaxTempLimit = 0;

    // Test 1-Surface 1: Temperature is within the max and min limits.  Don't do anything to the temperature.  No error messages
    surfNum = 1;
    nodeNum = 1;
    nodeTemp = 1.23;
    expectedAnswer = nodeTemp;
    EnergyPlus::HeatBalFiniteDiffManager::CheckFDNodeTempLimits(*state, surfNum, nodeNum, nodeTemp);
    EXPECT_EQ(nodeTemp, expectedAnswer);
    EXPECT_EQ(thisSurfFD(1).indexNodeMinTempLimit, 0);
    EXPECT_EQ(thisSurfFD(1).indexNodeMaxTempLimit, 0);

    // Test 1-Surface 2: Temperature is within the max and min limits.  Don't do anything to the temperature.  No error messages
    surfNum = 2;
    nodeNum = 2;
    nodeTemp = 4.56;
    expectedAnswer = nodeTemp;
    EnergyPlus::HeatBalFiniteDiffManager::CheckFDNodeTempLimits(*state, surfNum, nodeNum, nodeTemp);
    EXPECT_EQ(nodeTemp, expectedAnswer);
    EXPECT_EQ(thisSurfFD(2).indexNodeMinTempLimit, 0);
    EXPECT_EQ(thisSurfFD(2).indexNodeMaxTempLimit, 0);

    // Test 2-Surface 1: Temperature is below the minimum value.  Gets reset, error messages
    surfNum = 1;
    nodeNum = 3;
    nodeTemp = -3000.0;
    expectedAnswer = DataHeatBalSurface::MinSurfaceTempLimit;
    EnergyPlus::HeatBalFiniteDiffManager::CheckFDNodeTempLimits(*state, surfNum, nodeNum, nodeTemp);
    EXPECT_EQ(nodeTemp, expectedAnswer);
    EXPECT_EQ(thisSurfFD(1).indexNodeMinTempLimit, 1);
    EXPECT_EQ(thisSurfFD(1).indexNodeMaxTempLimit, 0);
    std::string const error_string_21 = delimited_string({"   ** Severe  ** Node temperature (low) out of bounds [-3000.00] for "
                                                          "surface=CONDFD SURFACE 1, node=3",
                                                          "   **   ~~~   **  Environment=, at Simulation time= 00:00 - 00:00",
                                                          "   **   ~~~   ** Value has been reset to the lower limit value of -100.00."});
    compare_err_stream(error_string_21, true);

    // Test 2-Surface 2: Temperature is below the minimum value.  Gets reset, error messages
    surfNum = 2;
    nodeNum = 4;
    nodeTemp = -4000.0;
    expectedAnswer = DataHeatBalSurface::MinSurfaceTempLimit;
    EnergyPlus::HeatBalFiniteDiffManager::CheckFDNodeTempLimits(*state, surfNum, nodeNum, nodeTemp);
    EXPECT_EQ(nodeTemp, expectedAnswer);
    EXPECT_EQ(thisSurfFD(2).indexNodeMinTempLimit, 2);
    EXPECT_EQ(thisSurfFD(2).indexNodeMaxTempLimit, 0);
    std::string const error_string_22 = delimited_string({"   ** Severe  ** Node temperature (low) out of bounds [-4000.00] for "
                                                          "surface=CONDFD SURFACE 2, node=4",
                                                          "   **   ~~~   **  Environment=, at Simulation time= 00:00 - 00:00",
                                                          "   **   ~~~   ** Value has been reset to the lower limit value of -100.00."});
    compare_err_stream(error_string_22, true);

    // Test 3-Surface 1: Temperature is below the minimum value for a second time.  Gets reset, but NO error messages
    surfNum = 1;
    nodeNum = 5;
    nodeTemp = -3000.0;
    expectedAnswer = DataHeatBalSurface::MinSurfaceTempLimit;
    EnergyPlus::HeatBalFiniteDiffManager::CheckFDNodeTempLimits(*state, surfNum, nodeNum, nodeTemp);
    EXPECT_EQ(nodeTemp, expectedAnswer);
    EXPECT_EQ(thisSurfFD(1).indexNodeMinTempLimit, 1);
    EXPECT_EQ(thisSurfFD(1).indexNodeMaxTempLimit, 0);

    // Test 3-Surface 2: Temperature is below the minimum value for a second time.  Gets reset, but NO error messages
    surfNum = 2;
    nodeNum = 6;
    nodeTemp = -4000.0;
    expectedAnswer = DataHeatBalSurface::MinSurfaceTempLimit;
    EnergyPlus::HeatBalFiniteDiffManager::CheckFDNodeTempLimits(*state, surfNum, nodeNum, nodeTemp);
    EXPECT_EQ(nodeTemp, expectedAnswer);
    EXPECT_EQ(thisSurfFD(2).indexNodeMinTempLimit, 2);
    EXPECT_EQ(thisSurfFD(2).indexNodeMaxTempLimit, 0);

    // Test 4-Surface 1: Temperature is above the maximum value.  Gets reset, error message.
    surfNum = 1;
    nodeNum = 7;
    nodeTemp = 3000.0;
    expectedAnswer = state->dataHeatBalSurf->MaxSurfaceTempLimit;
    EnergyPlus::HeatBalFiniteDiffManager::CheckFDNodeTempLimits(*state, surfNum, nodeNum, nodeTemp);
    EXPECT_EQ(nodeTemp, expectedAnswer);
    EXPECT_EQ(thisSurfFD(1).indexNodeMinTempLimit, 1);
    EXPECT_EQ(thisSurfFD(1).indexNodeMaxTempLimit, 3);
    std::string const error_string_41 = delimited_string({"   ** Severe  ** Node temperature (high) out of bounds [3000.00] for "
                                                          "surface=CONDFD SURFACE 1, node=7",
                                                          "   **   ~~~   **  Environment=, at Simulation time= 00:00 - 00:00",
                                                          "   **   ~~~   ** Value has been reset to the upper limit value of 200.00."});
    compare_err_stream(error_string_41, true);

    // Test 4-Surface 2: Temperature is above the maximum value.  Gets reset, error message.
    surfNum = 2;
    nodeNum = 8;
    nodeTemp = 4000.0;
    expectedAnswer = state->dataHeatBalSurf->MaxSurfaceTempLimit;
    EnergyPlus::HeatBalFiniteDiffManager::CheckFDNodeTempLimits(*state, surfNum, nodeNum, nodeTemp);
    EXPECT_EQ(nodeTemp, expectedAnswer);
    EXPECT_EQ(thisSurfFD(2).indexNodeMinTempLimit, 2);
    EXPECT_EQ(thisSurfFD(2).indexNodeMaxTempLimit, 4);
    std::string const error_string_42 = delimited_string({"   ** Severe  ** Node temperature (high) out of bounds [4000.00] for "
                                                          "surface=CONDFD SURFACE 2, node=8",
                                                          "   **   ~~~   **  Environment=, at Simulation time= 00:00 - 00:00",
                                                          "   **   ~~~   ** Value has been reset to the upper limit value of 200.00."});
    compare_err_stream(error_string_42, true);

    // Test 5-Surface 1: Temperature is above the maximum value for a second time.  Gets reset, NO error message.
    surfNum = 1;
    nodeNum = 9;
    nodeTemp = 3000.0;
    expectedAnswer = state->dataHeatBalSurf->MaxSurfaceTempLimit;
    EnergyPlus::HeatBalFiniteDiffManager::CheckFDNodeTempLimits(*state, surfNum, nodeNum, nodeTemp);
    EXPECT_EQ(nodeTemp, expectedAnswer);
    EXPECT_EQ(thisSurfFD(1).indexNodeMinTempLimit, 1);
    EXPECT_EQ(thisSurfFD(1).indexNodeMaxTempLimit, 3);

    // Test 5-Surface 2: Temperature is above the maximum value for a second time.  Gets reset, NO error message.
    surfNum = 2;
    nodeNum = 10;
    nodeTemp = 4000.0;
    expectedAnswer = state->dataHeatBalSurf->MaxSurfaceTempLimit;
    EnergyPlus::HeatBalFiniteDiffManager::CheckFDNodeTempLimits(*state, surfNum, nodeNum, nodeTemp);
    EXPECT_EQ(nodeTemp, expectedAnswer);
    EXPECT_EQ(thisSurfFD(2).indexNodeMinTempLimit, 2);
    EXPECT_EQ(thisSurfFD(2).indexNodeMaxTempLimit, 4);
}

TEST_F(EnergyPlusFixture, HeatBalFiniteDiffManager_setSizeMaxPropertiesTest)
{
    std::string const idf_objects = delimited_string({
        "MaterialProperty:VariableThermalConductivity,",
        "    PCMPlasterBoard , !- Name",
        "    0,    !- Temperature 1 {C}",
        "    4.2,  !- Thermal Conductivity 1 {W/m-K}",
        "    22,   !- Temperature 2 {C}",
        "    4.2,  !- Thermal Conductivity 2 {W/m-K}",
        "    22.1, !- Temperature 3 {C}",
        "    2.5,  !- Thermal Conductivity 3 {W/m-K}",
        "    100,  !- Temperature 4 {C}",
        "    2.5;  !- Thermal Conductivity 4 {W/m-K}",
        "MaterialProperty:PhaseChange,",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD , !- Name",
        "    0.0,    !- Temperature coefficient ,thermal conductivity(W/m K2)",
        "    -20.,   !- Temperature 1, C",
        "    0.01,   !- Enthalpy 1 at –20C, (J/kg)",
        "    20.,    !- Temperature 2, C",
        "    33400,  !- Enthalpy 2, (J/kg)",
        "    20.5,   !- temperature 3, C",
        "    70000,  !- Enthalpy 3, (J/kg)",
        "    100.,   !- Temperature 4, C",
        "    137000; !- Enthalpy 4, (J/kg)",
        "MaterialProperty:VariableThermalConductivity,",
        "    PCMPlasterBoard 2, !- Name",
        "    0,    !- Temperature 1 {C}",
        "    4.2,  !- Thermal Conductivity 1 {W/m-K}",
        "    22,   !- Temperature 2 {C}",
        "    4.2,  !- Thermal Conductivity 2 {W/m-K}",
        "    22.1, !- Temperature 3 {C}",
        "    2.5,  !- Thermal Conductivity 3 {W/m-K}",
        "    100,  !- Temperature 4 {C}",
        "    2.5,  !- Thermal Conductivity 4 {W/m-K}",
        "    0,    !- Temperature 5 {C}",
        "    4.2,  !- Thermal Conductivity 5 {W/m-K}",
        "    22,   !- Temperature 6 {C}",
        "    4.2,  !- Thermal Conductivity 6 {W/m-K}",
        "    22.1, !- Temperature 7 {C}",
        "    2.5,  !- Thermal Conductivity 7 {W/m-K}",
        "    100,  !- Temperature 8 {C}",
        "    2.5,  !- Thermal Conductivity 8 {W/m-K}",
        "    0,    !- Temperature 9 {C}",
        "    4.2,  !- Thermal Conductivity 9 {W/m-K}",
        "    22,   !- Temperature 10 {C}",
        "    4.2,  !- Thermal Conductivity 10 {W/m-K}",
        "    0,    !- Temperature 11 {C}",
        "    4.2,  !- Thermal Conductivity 11 {W/m-K}",
        "    22,   !- Temperature 12 {C}",
        "    4.2,  !- Thermal Conductivity 12 {W/m-K}",
        "    22.1, !- Temperature 13 {C}",
        "    2.5,  !- Thermal Conductivity 13 {W/m-K}",
        "    100,  !- Temperature 14 {C}",
        "    2.5,  !- Thermal Conductivity 14 {W/m-K}",
        "    0,    !- Temperature 15 {C}",
        "    4.2,  !- Thermal Conductivity 15 {W/m-K}",
        "    22,   !- Temperature 16 {C}",
        "    4.2,  !- Thermal Conductivity 16 {W/m-K}",
        "    22.1, !- Temperature 17 {C}",
        "    2.5,  !- Thermal Conductivity 17 {W/m-K}",
        "    100,  !- Temperature 18 {C}",
        "    2.5,  !- Thermal Conductivity 18 {W/m-K}",
        "    0,    !- Temperature 19 {C}",
        "    4.2,  !- Thermal Conductivity 19 {W/m-K}",
        "    22,   !- Temperature 20 {C}",
        "    4.2,  !- Thermal Conductivity 20 {W/m-K}",
        "    0,    !- Temperature 21 {C}",
        "    4.2,  !- Thermal Conductivity 21 {W/m-K}",
        "    22,   !- Temperature 22 {C}",
        "    4.2,  !- Thermal Conductivity 22 {W/m-K}",
        "    22.1, !- Temperature 23 {C}",
        "    2.5,  !- Thermal Conductivity 23 {W/m-K}",
        "    100,  !- Temperature 24 {C}",
        "    2.5,  !- Thermal Conductivity 24 {W/m-K}",
        "    0,    !- Temperature 25 {C}",
        "    4.2,  !- Thermal Conductivity 25 {W/m-K}",
        "    22,   !- Temperature 26 {C}",
        "    4.2,  !- Thermal Conductivity 26 {W/m-K}",
        "    22.1, !- Temperature 27 {C}",
        "    2.5,  !- Thermal Conductivity 27 {W/m-K}",
        "    100,  !- Temperature 28 {C}",
        "    2.5,  !- Thermal Conductivity 28 {W/m-K}",
        "    0,    !- Temperature 29 {C}",
        "    4.2,  !- Thermal Conductivity 29 {W/m-K}",
        "    22,   !- Temperature 30 {C}",
        "    4.2,  !- Thermal Conductivity 30 {W/m-K}",
        "    0,    !- Temperature 31 {C}",
        "    4.2,  !- Thermal Conductivity 31 {W/m-K}",
        "    22,   !- Temperature 32 {C}",
        "    4.2,  !- Thermal Conductivity 32 {W/m-K}",
        "    22.1, !- Temperature 33 {C}",
        "    2.5,  !- Thermal Conductivity 33 {W/m-K}",
        "    100,  !- Temperature 34 {C}",
        "    2.5,  !- Thermal Conductivity 34 {W/m-K}",
        "    0,    !- Temperature 35 {C}",
        "    4.2,  !- Thermal Conductivity 35 {W/m-K}",
        "    22,   !- Temperature 36 {C}",
        "    4.2,  !- Thermal Conductivity 36 {W/m-K}",
        "    22.1, !- Temperature 37 {C}",
        "    2.5,  !- Thermal Conductivity 37 {W/m-K}",
        "    100,  !- Temperature 38 {C}",
        "    2.5,  !- Thermal Conductivity 38 {W/m-K}",
        "    0,    !- Temperature 39 {C}",
        "    4.2,  !- Thermal Conductivity 39 {W/m-K}",
        "    22,   !- Temperature 40 {C}",
        "    4.2,  !- Thermal Conductivity 40 {W/m-K}",
        "    0,    !- Temperature 41 {C}",
        "    4.2,  !- Thermal Conductivity 41 {W/m-K}",
        "    22,   !- Temperature 42 {C}",
        "    4.2,  !- Thermal Conductivity 42 {W/m-K}",
        "    22.1, !- Temperature 43 {C}",
        "    2.5,  !- Thermal Conductivity 43 {W/m-K}",
        "    100,  !- Temperature 44 {C}",
        "    2.5,  !- Thermal Conductivity 44 {W/m-K}",
        "    0,    !- Temperature 45 {C}",
        "    4.2,  !- Thermal Conductivity 45 {W/m-K}",
        "    22,   !- Temperature 46 {C}",
        "    4.2,  !- Thermal Conductivity 46 {W/m-K}",
        "    22.1, !- Temperature 47 {C}",
        "    2.5,  !- Thermal Conductivity 47 {W/m-K}",
        "    100,  !- Temperature 48 {C}",
        "    2.5,  !- Thermal Conductivity 48 {W/m-K}",
        "    0,    !- Temperature 49 {C}",
        "    4.2,  !- Thermal Conductivity 49 {W/m-K}",
        "    22,   !- Temperature 50 {C}",
        "    4.2;  !- Thermal Conductivity 10 {W/m-K}",
        "MaterialProperty:VariableThermalConductivity,",
        "    PCMPlasterBoard3, !- Name",
        "    0,    !- Temperature 1 {C}",
        "    7.11, !- Thermal Conductivity 1 {W/m-K}",
        "    100,  !- Temperature 2 {C}",
        "    2.3;  !- Thermal Conductivity 2 {W/m-K}",
        "MaterialProperty:PhaseChange,",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD c, !- Name",
        "    0.0,    !- Temperature coefficient ,thermal conductivity(W/m K2)",
        "    -20.,   !- Temperature 1, C",
        "    0.01,   !- Enthalpy 1 at –20C, (J/kg)",
        "    20.,    !- Temperature 2, C",
        "    33400,  !- Enthalpy 2, (J/kg)",
        "    20.5,   !- Temperature 3, C",
        "    70000,  !- Enthalpy 3, (J/kg)",
        "    100.,   !- Temperature 4, C",
        "    137000, !- Enthalpy 4, (J/kg)",
        "    -20.,   !- Temperature 5, C",
        "    0.01,   !- Enthalpy 5 at –20C, (J/kg)",
        "    20.,    !- Temperature 6, C",
        "    33400,  !- Enthalpy 6, (J/kg)",
        "    20.5,   !- Temperature 7, C",
        "    70000,  !- Enthalpy 7, (J/kg)",
        "    100.,   !- Temperature 8, C",
        "    137000, !- Enthalpy 8, (J/kg)",
        "    -20.,   !- Temperature 9, C",
        "    0.01,   !- Enthalpy 9 at –20C, (J/kg)",
        "    20.,    !- Temperature 10, C",
        "    33400,  !- Enthalpy 10, (J/kg)",
        "    -20.,   !- Temperature 11, C",
        "    0.01,   !- Enthalpy 11 at –20C, (J/kg)",
        "    20.,    !- Temperature 12, C",
        "    33400,  !- Enthalpy 12, (J/kg)",
        "    20.5,   !- Temperature 13, C",
        "    70000,  !- Enthalpy 13, (J/kg)",
        "    100.,   !- Temperature 14, C",
        "    137000, !- Enthalpy 14, (J/kg)",
        "    -20.,   !- Temperature 15, C",
        "    0.01,   !- Enthalpy 15 at –20C, (J/kg)",
        "    20.,    !- Temperature 16, C",
        "    33400,  !- Enthalpy 16, (J/kg)",
        "    20.5,   !- Temperature 17, C",
        "    70000,  !- Enthalpy 17, (J/kg)",
        "    100.,   !- Temperature 18, C",
        "    137000, !- Enthalpy 18, (J/kg)",
        "    -20.,   !- Temperature 19, C",
        "    0.01,   !- Enthalpy 19 at –20C, (J/kg)",
        "    20.,    !- Temperature 20, C",
        "    33400,  !- Enthalpy 20, (J/kg)",
        "    -20.,   !- Temperature 21, C",
        "    0.01,   !- Enthalpy 21 at –20C, (J/kg)",
        "    20.,    !- Temperature 22, C",
        "    33400,  !- Enthalpy 22, (J/kg)",
        "    20.5,   !- Temperature 23, C",
        "    70000,  !- Enthalpy 23, (J/kg)",
        "    100.,   !- Temperature 24, C",
        "    137000, !- Enthalpy 24, (J/kg)",
        "    -20.,   !- Temperature 25, C",
        "    0.01,   !- Enthalpy 25 at –20C, (J/kg)",
        "    20.,    !- Temperature 26, C",
        "    33400,  !- Enthalpy 26, (J/kg)",
        "    20.5,   !- Temperature 27, C",
        "    70000,  !- Enthalpy 27, (J/kg)",
        "    100.,   !- Temperature 28, C",
        "    137000, !- Enthalpy 28, (J/kg)",
        "    -20.,   !- Temperature 29, C",
        "    0.01,   !- Enthalpy 29 at –20C, (J/kg)",
        "    20.,    !- Temperature 30, C",
        "    33400,  !- Enthalpy 30, (J/kg)",
        "    -20.,   !- Temperature 31, C",
        "    0.01,   !- Enthalpy 31 at –20C, (J/kg)",
        "    20.,    !- Temperature 32, C",
        "    33400,  !- Enthalpy 32, (J/kg)",
        "    20.5,   !- Temperature 33, C",
        "    70000,  !- Enthalpy 33, (J/kg)",
        "    100.,   !- Temperature 34, C",
        "    137000, !- Enthalpy 34, (J/kg)",
        "    -20.,   !- Temperature 35, C",
        "    0.01,   !- Enthalpy 35 at –20C, (J/kg)",
        "    20.,    !- Temperature 36, C",
        "    33400,  !- Enthalpy 36, (J/kg)",
        "    20.5,   !- Temperature 37, C",
        "    70000,  !- Enthalpy 37, (J/kg)",
        "    100.,   !- Temperature 38, C",
        "    137000, !- Enthalpy 38, (J/kg)",
        "    -20.,   !- Temperature 39, C",
        "    0.01,   !- Enthalpy 39 at –20C, (J/kg)",
        "    20.,    !- Temperature 40, C",
        "    33400,  !- Enthalpy 40, (J/kg)",
        "    -20.,   !- Temperature 41, C",
        "    0.01,   !- Enthalpy 41 at –20C, (J/kg)",
        "    20.,    !- Temperature 42, C",
        "    33400,  !- Enthalpy 42, (J/kg)",
        "    20.5,   !- Temperature 43, C",
        "    70000,  !- Enthalpy 43, (J/kg)",
        "    100.,   !- Temperature 44, C",
        "    137000, !- Enthalpy 44, (J/kg)",
        "    -20.,   !- Temperature 45, C",
        "    0.01,   !- Enthalpy 45 at –20C, (J/kg)",
        "    20.,    !- Temperature 46, C",
        "    33400,  !- Enthalpy 46, (J/kg)",
        "    20.5,   !- Temperature 47, C",
        "    70000,  !- Enthalpy 47, (J/kg)",
        "    100.,   !- Temperature 48, C",
        "    137000, !- Enthalpy 48, (J/kg)",
        "    -20.,   !- Temperature 49, C",
        "    0.01,   !- Enthalpy 49 at –20C, (J/kg)",
        "    20.,    !- Temperature 50, C",
        "    33400;  !- Enthalpy 50, (J/kg)",
        "MaterialProperty:PhaseChange,",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD b, !- Name",
        "    1.2,    !- Temperature coefficient ,thermal conductivity(W/m K2)",
        "    -20.,   !- Temperature 1, C",
        "    0.001,  !- Enthalpy 1 at –20C, (J/kg)",
        "    100.0,  !- Temperature 2, C",
        "    233400; !- Enthalpy 2, (J/kg)",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    // Test Data
    int functionAnswer = 0;
    int expectedAnswer = 101; // 50 sets of properties for the largest input objects, plus one additional real parameter for phase change

    // int function call
    functionAnswer = EnergyPlus::HeatBalFiniteDiffManager::setSizeMaxProperties(*state);

    // Check answer vs. expected
    EXPECT_EQ(functionAnswer, expectedAnswer);
}

} // namespace EnergyPlus
