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

// EnergyPlus::EarthTube Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataErrorTracking.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataWater.hh>
#include <EnergyPlus/EcoRoofManager.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SolarShading.hh>
#include <EnergyPlus/WaterManager.hh>
#include <EnergyPlus/WeatherManager.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::EcoRoofManager;
using namespace EnergyPlus::SolarShading;
using namespace EnergyPlus::DataEnvironment;
using namespace EnergyPlus::DataHeatBalance;

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, EcoRoof_CalculateEcoRoofSolarTest)
{
    state->init_state(*state);
    Real64 resultRS;
    Real64 resultf1;
    Real64 expectedRS;
    Real64 expectedf1;
    int SurfNum = 1;
    state->dataSolarShading->SurfAnisoSkyMult.allocate(SurfNum);

    // Test 1: SOLCOS negative
    state->dataEnvrn->SOLCOS(3) = -1.0;
    state->dataEnvrn->BeamSolarRad = 321.0;
    state->dataSolarShading->SurfAnisoSkyMult(SurfNum) = 0.5;
    state->dataEnvrn->DifSolarRad = 124.0;
    expectedRS = 62.0;
    expectedf1 = 3.9956;
    CalculateEcoRoofSolar(*state, resultRS, resultf1, SurfNum);
    EXPECT_NEAR(resultRS, expectedRS, 0.001);
    EXPECT_NEAR(resultf1, expectedf1, 0.001);

    // Test 2: SOLCOS positive
    state->dataEnvrn->SOLCOS(3) = 0.7;
    state->dataEnvrn->BeamSolarRad = 400.0;
    state->dataSolarShading->SurfAnisoSkyMult(SurfNum) = 0.6;
    state->dataEnvrn->DifSolarRad = 100.0;
    expectedRS = 340.0;
    expectedf1 = 1.4004;
    CalculateEcoRoofSolar(*state, resultRS, resultf1, SurfNum);
    EXPECT_NEAR(resultRS, expectedRS, 0.001);
    EXPECT_NEAR(resultf1, expectedf1, 0.001);

    // Test 3: inverse of f1 greater than 1
    state->dataEnvrn->SOLCOS(3) = 1.0;
    state->dataEnvrn->BeamSolarRad = 1500.0;
    state->dataSolarShading->SurfAnisoSkyMult(SurfNum) = 0.0;
    state->dataEnvrn->DifSolarRad = 0.0;
    expectedRS = 1500.0;
    expectedf1 = 1.0;
    CalculateEcoRoofSolar(*state, resultRS, resultf1, SurfNum);
    EXPECT_NEAR(resultRS, expectedRS, 0.001);
    EXPECT_NEAR(resultf1, expectedf1, 0.001);
}

TEST_F(EnergyPlusFixture, EcoRoofManager_UpdateSoilProps)
{
    // with site:precipitation
    std::string const idf_objects = delimited_string({

        "Construction,",
        "ASHRAE 90.1-2004_Sec 5.5-2_Roof,  !- Name",
        "BaseEco,                 !- Outside Layer",
        "ASHRAE 90.1-2004_Sec 5.5-2_Roof Insulation_1,  !- Layer 2",
        "ASHRAE 90.1-2004_Sec 5.5-2_MAT-METAL;  !- Layer 3",

        "Material:RoofVegetation,",
        "BaseEco,                 !- Name",
        "0.5,                     !- Height of Plants {m}",
        "5,                       !- Leaf Area Index {dimensionless}",
        "0.2,                     !- Leaf Reflectivity {dimensionless}",
        "0.95,                    !- Leaf Emissivity",
        "180,                     !- Minimum Stomatal Resistance {s/m}",
        "EcoRoofSoil,             !- Soil Layer Name",
        "MediumSmooth,            !- Roughness",
        "0.18,                    !- Thickness {m}",
        "0.4,                     !- Conductivity of Dry Soil {W/m-K}",
        "641,                     !- Density of Dry Soil {kg/m3}",
        "1100,                    !- Specific Heat of Dry Soil {J/kg-K}",
        "0.95,                    !- Thermal Absorptance",
        "0.8,                     !- Solar Absorptance",
        "0.7,                     !- Visible Absorptance",
        "0.4,                     !- Saturation Volumetric Moisture Content of the Soil Layer",
        "0.01,                    !- Residual Volumetric Moisture Content of the Soil Layer",
        "0.2,                     !- Initial Volumetric Moisture Content of the Soil Layer",
        "Advanced;                !- Moisture Diffusion Calculation Method",

        "Material,",
        "ASHRAE 90.1-2004_Sec 5.5-2_Roof Insulation_1,  !- Name",
        "MediumRough,             !- Roughness",
        "0.1250,                  !- Thickness {m}",
        "0.0490,                  !- Conductivity {W/m-K}",
        "265.0000,                !- Density {kg/m3}",
        "836.8000,                !- Specific Heat {J/kg-K}",
        "0.9000,                  !- Thermal Absorptance",
        "0.7000,                  !- Solar Absorptance",
        "0.7000;                  !- Visible Absorptance",

        "Material,",
        "ASHRAE 90.1-2004_Sec 5.5-2_MAT-METAL,  !- Name",
        "MediumSmooth,            !- Roughness",
        "0.0015,                  !- Thickness {m}",
        "45.0060,                 !- Conductivity {W/m-K}",
        "7680.0000,               !- Density {kg/m3}",
        "418.4000,                !- Specific Heat {J/kg-K}",
        "0.9000,                  !- Thermal Absorptance",
        "0.7000,                  !- Solar Absorptance",
        "0.3000;                  !- Visible Absorptance",

        "RoofIrrigation,",
        "SmartSchedule,           !- Irrigation Model Type",
        "IRRIGATIONSCHD,          !- Irrigation Rate Schedule Name",
        "100;                     !- Irrigation Maximum Saturation Threshold",

        "Schedule:Compact,",
        "IRRIGATIONSCHD,          !- Name",
        "Any Number,              !- Schedule Type Limits Name",
        "Through: 12/31,          !- Field 1",
        "For: Alldays,            !- Field 2",
        "Until: 07:00,0.001,      !- Field 3",
        "Until: 09:00,0.002,      !- Field 4",
        "Until: 24:00,0.003;      !- Field 5",
    });
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound = false;
    // Read objects
    Material::GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    Real64 Moisture = 0.2;
    Real64 MeanRootMoisture = 0.2;
    Real64 Alphag = 0.2;
    Real64 MoistureMax = 0.4;
    Real64 MoistureResidual = 0.01;
    Real64 SoilThickness = 0.18;
    Real64 Vfluxf = 0.0; // Water mass flux from vegetation [m/s]
    Real64 Vfluxg = 0.0; // Water mass flux from soil surface [m/s]
    int ConstrNum = 1;
    int unit = 0;
    Real64 Tg = 10;
    Real64 Tf = 10;
    Real64 Qsoil = 0;

    WaterManager::GetWaterManagerInput(*state);
    state->dataGlobal->TimeStepZoneSec = 900;
    state->dataEnvrn->Year = 2000;
    state->dataEnvrn->EndYear = 2000;
    state->dataEnvrn->Month = 1;
    state->dataGlobal->TimeStep = 2;

    // Without site:precipitation, use epw "LiquidPrecipitation"
    state->dataWaterData->RainFall.ModeID = DataWater::RainfallMode::None;
    state->dataEnvrn->LiquidPrecipitation = 0.005;
    WaterManager::UpdatePrecipitation(*state);
    ASSERT_EQ(state->dataEcoRoofMgr->CurrentPrecipitation, 0.005);

    // fixme: rewrite testcase
    EcoRoofManager::UpdateSoilProps(
        *state, Moisture, MeanRootMoisture, MoistureMax, MoistureResidual, SoilThickness, Vfluxf, Vfluxg, ConstrNum, Alphag, unit, Tg, Tf, Qsoil);
    ASSERT_EQ(state->dataWaterData->Irrigation.ActualAmount, state->dataEcoRoofMgr->CurrentIrrigation);
}

TEST_F(EnergyPlusFixture, EcoRoofManager_initEcoRoofFirstTimeTest)
{
    state->init_state(*state);
    int surfNum = 1;
    int constrNum = 1;
    Real64 expectedAnswer;
    Real64 allowableTolerance = 0.000001;

    state->dataConstruction->Construct.allocate(constrNum);
    auto *mat = new Material::MaterialEcoRoof;
    state->dataMaterial->materials.push_back(mat);
    state->dataSurface->Surface.allocate(surfNum);

    auto &thisConstruct = state->dataConstruction->Construct(constrNum);
    auto &thisEcoRoof = state->dataEcoRoofMgr;

    thisConstruct.LayerPoint.allocate(1);
    thisConstruct.LayerPoint(1) = 1;

    state->dataSurface->Surface(surfNum).HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::CTF;

    mat->LAI = 3.21;
    mat->AbsorpSolar = 0.72;
    thisEcoRoof->FirstEcoSurf = 0;
    thisEcoRoof->EcoRoofbeginFlag = true;

    initEcoRoofFirstTime(*state, surfNum, constrNum);

    // Spot check some answers to test first time initialization routine
    expectedAnswer = 3.21;
    EXPECT_NEAR(thisEcoRoof->LAI, expectedAnswer, allowableTolerance);
    expectedAnswer = 0.28;
    EXPECT_NEAR(thisEcoRoof->Alphag, expectedAnswer, allowableTolerance);
    EXPECT_EQ(thisEcoRoof->FirstEcoSurf, surfNum);
    EXPECT_FALSE(thisEcoRoof->EcoRoofbeginFlag);
}

TEST_F(EnergyPlusFixture, EcoRoofManager_initEcoRoofTest)
{
    state->init_state(*state);
    int surfNum = 1;
    int constrNum = 1;
    Real64 expectedAnswer;
    Real64 allowableTolerance = 0.000001;

    state->dataConstruction->Construct.allocate(constrNum);
    auto *mat = new Material::MaterialEcoRoof;
    state->dataMaterial->materials.push_back(mat);
    state->dataSurface->Surface.allocate(surfNum);

    auto &thisConstruct = state->dataConstruction->Construct(constrNum);
    auto &thisEcoRoof = state->dataEcoRoofMgr;

    thisConstruct.LayerPoint.allocate(1);
    thisConstruct.LayerPoint(1) = 1;

    // Test 1: test of first IF block in routine
    state->dataGlobal->BeginEnvrnFlag = false;
    state->dataGlobal->WarmupFlag = true;
    thisEcoRoof->CalcEcoRoofMyEnvrnFlag = false;
    mat->InitMoisture = 23.0;
    mat->AbsorpSolar = 0.72;
    thisEcoRoof->Moisture = 0.0;
    thisEcoRoof->MeanRootMoisture = 0.0;
    thisEcoRoof->Alphag = 0.0;

    initEcoRoof(*state, surfNum, constrNum);
    expectedAnswer = 23.0;
    EXPECT_NEAR(thisEcoRoof->Moisture, expectedAnswer, allowableTolerance);
    EXPECT_NEAR(thisEcoRoof->MeanRootMoisture, expectedAnswer, allowableTolerance);
    expectedAnswer = 0.28;
    EXPECT_NEAR(thisEcoRoof->Alphag, expectedAnswer, allowableTolerance);
    EXPECT_TRUE(thisEcoRoof->CalcEcoRoofMyEnvrnFlag);

    // Test 2: test of second IF block in routine (spot check)
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataGlobal->WarmupFlag = false;
    thisEcoRoof->CalcEcoRoofMyEnvrnFlag = true;
    thisEcoRoof->Tg = 0.0;
    thisEcoRoof->Tf = 0.0;
    expectedAnswer = 10.0;

    initEcoRoof(*state, surfNum, constrNum);
    EXPECT_NEAR(thisEcoRoof->Tg, expectedAnswer, allowableTolerance);
    EXPECT_NEAR(thisEcoRoof->Tf, expectedAnswer, allowableTolerance);
    EXPECT_FALSE(thisEcoRoof->CalcEcoRoofMyEnvrnFlag);
}

TEST_F(EnergyPlusFixture, EcoRoofManager_initEcoRoofFirstTimeErrorTest)
{
    std::string const idf_objects = delimited_string({
        "Version, " + DataStringGlobals::MatchVersion + ";",
    });
    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);
    int surfNum = 1;
    int constrNum = 1;
    Real64 expectedAnswer;
    Real64 allowableTolerance = 0.000001;

    state->dataConstruction->Construct.allocate(constrNum);
    auto *mat = new Material::MaterialEcoRoof;
    state->dataMaterial->materials.push_back(mat);
    state->dataSurface->Surface.allocate(surfNum);

    auto &thisConstruct = state->dataConstruction->Construct(constrNum);
    auto &thisEcoRoof = state->dataEcoRoofMgr;

    thisConstruct.LayerPoint.allocate(1);
    thisConstruct.LayerPoint(1) = 1;
    state->dataSurface->Surface(surfNum).Name = "ZN6_S_SPACE_2:ROOF";

    // Test 1: Normal Operation (Heat Transfer Model = CTF)-->no error messages
    state->dataSurface->Surface(surfNum).HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::CTF;
    thisEcoRoof->FirstEcoSurf = 0;
    thisEcoRoof->EcoRoofbeginFlag = true;
    initEcoRoofFirstTime(*state, surfNum, constrNum);

    // Test 2: Improper HT Model Selected (Heat Transfer Model = CondFD)-->error messages produced
    state->dataSurface->Surface(surfNum).HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::CondFD;
    thisEcoRoof->FirstEcoSurf = 0;
    thisEcoRoof->EcoRoofbeginFlag = true;

    EXPECT_THROW(initEcoRoofFirstTime(*state, surfNum, constrNum), EnergyPlus::FatalError);

    std::string const error_string = delimited_string({
        "   ** Severe  ** initEcoRoofFirstTime: EcoRoof simulation but HeatBalanceAlgorithm is not ConductionTransferFunction(CTF). EcoRoof model "
        "currently works only with CTF heat balance solution algorithm.",
        "   **   ~~~   ** Occurs for surface named ZN6_S_SPACE_2:ROOF",
        "   **   ~~~   ** Check input syntax for HeatBalanceAlgorithm, SurfaceProperty:HeatTransferAlgorithm,",
        "   **   ~~~   ** SurfaceProperty:HeatTransferAlgorithm:MultipleSurface, and SurfaceProperty:HeatTransferAlgorithm:SurfaceList ",
        "   **   ~~~   ** to verify that the solution method is set to CTF for the surface that is an EcoRoof.",
        "   **  Fatal  ** initEcoRoofFirstTime: Program terminates due to preceding conditions.",
        "   ...Summary of Errors that led to program termination:",
        "   ..... Reference severe error count=1",
        "   ..... Last severe error=initEcoRoofFirstTime: EcoRoof simulation but HeatBalanceAlgorithm is not ConductionTransferFunction(CTF). "
        "EcoRoof model currently works only with CTF heat balance solution algorithm.",
    });
    EXPECT_TRUE(compare_err_stream(error_string, true));

    // Test 3: Improper HT Model Selected (Heat Transfer Model = HAMT)-->error messages produced
    state->dataSurface->Surface(surfNum).HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::HAMT;
    thisEcoRoof->FirstEcoSurf = 0;
    thisEcoRoof->EcoRoofbeginFlag = true;
    state->dataErrTracking->TotalSevereErrors = 0;

    EXPECT_THROW(initEcoRoofFirstTime(*state, surfNum, constrNum), EnergyPlus::FatalError);

    EXPECT_TRUE(compare_err_stream(error_string, true));

    // Test 4: Improper HT Model Selected (Heat Transfer Model = EMPD)-->error messages produced
    state->dataSurface->Surface(surfNum).HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::EMPD;
    thisEcoRoof->FirstEcoSurf = 0;
    thisEcoRoof->EcoRoofbeginFlag = true;
    state->dataErrTracking->TotalSevereErrors = 0;

    EXPECT_THROW(initEcoRoofFirstTime(*state, surfNum, constrNum), EnergyPlus::FatalError);

    EXPECT_TRUE(compare_err_stream(error_string, true));

    // Test 5: Improper HT Model Selected (Heat Transfer Model = Kiva)-->error messages produced
    state->dataSurface->Surface(surfNum).HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::Kiva;
    thisEcoRoof->FirstEcoSurf = 0;
    thisEcoRoof->EcoRoofbeginFlag = true;
    state->dataErrTracking->TotalSevereErrors = 0;

    EXPECT_THROW(initEcoRoofFirstTime(*state, surfNum, constrNum), EnergyPlus::FatalError);

    EXPECT_TRUE(compare_err_stream(error_string, true));
}

} // namespace EnergyPlus
