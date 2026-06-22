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

// EnergyPlus::MoistureBalanceEMPD Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataMoistureBalance.hh>
#include <EnergyPlus/DataMoistureBalanceEMPD.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/MoistureBalanceEMPDManager.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, CheckEMPDCalc)
{
    std::string const idf_objects =
        delimited_string({"Material,",
                          "Concrete,                !- Name",
                          "Rough,                   !- Roughness",
                          "0.152,                   !- Thickness {m}",
                          "0.3,                     !- Conductivity {W/m-K}",
                          "1000,                    !- Density {kg/m3}",
                          "950,                     !- Specific Heat {J/kg-K}",
                          "0.900000,                !- Thermal Absorptance",
                          "0.600000,                !- Solar Absorptance",
                          "0.600000;                !- Visible Absorptance",
                          "MaterialProperty:MoisturePenetrationDepth:Settings,",
                          "Concrete,                !- Name",
                          "6.554,                     !- Water Vapor Diffusion Resistance Factor {dimensionless} (mu)",
                          "0.0661,                   !- Moisture Equation Coefficient a {dimensionless} (MoistACoeff)",
                          "1,                       !- Moisture Equation Coefficient b {dimensionless} (MoistBCoeff)",
                          "0,                       !- Moisture Equation Coefficient c {dimensionless} (MoistCCoeff)",
                          "1,                       !- Moisture Equation Coefficient d {dimensionless} (MoistDCoeff)",
                          "0.006701,                    !- Surface-layer penetration depth {m} (dEMPD)",
                          "0.013402,                    !- Deep-layer penetration depth {m} (dEPMDdeep)",
                          "0,                       !- Coating layer permeability {m} (CoatingThickness)",
                          "1;                       !- Coating layer water vapor diffusion resistance factor {dimensionless} (muCoating)"});

    ASSERT_TRUE(process_idf(idf_objects));

    bool errors_found(false);
    Material::GetMaterialData(*state, errors_found);
    ASSERT_FALSE(errors_found) << "Errors in GetMaterialData";

    // Surface
    state->dataSurface->TotSurfaces = 1;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);
    DataSurfaces::SurfaceData &surface = state->dataSurface->Surface(1);
    surface.Name = "Surface1";
    surface.Area = 1.0;
    surface.HeatTransSurf = true;

    // Zone
    surface.Zone = 1;
    state->dataMstBal->RhoVaporAirIn.allocate(1);
    state->dataMstBal->HMassConvInFD.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 20.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.0061285406810457849;

    // Construction
    surface.Construction = 1;
    state->dataConstruction->Construct.allocate(1);
    Construction::ConstructionProps &construction = state->dataConstruction->Construct(1);
    construction.TotLayers = 1;
    construction.LayerPoint(construction.TotLayers) = Material::GetMaterialNum(*state, "CONCRETE");

    // Initialize and get inputs
    MoistureBalanceEMPDManager::InitMoistureBalanceEMPD(*state);

    // Set up conditions
    state->dataGlobal->TimeStepZone = 0.25;
    state->dataEnvrn->OutBaroPress = 101325.;
    state->dataMstBalEMPD->RVSurface(1) = 0.007077173214149593;
    state->dataMstBalEMPD->RVSurfaceOld(1) = state->dataMstBalEMPD->RVSurface(1);
    state->dataMstBal->HMassConvInFD(1) = 0.0016826898264131584;
    state->dataMstBal->RhoVaporAirIn(1) = 0.0073097913062508896;
    state->dataMstBalEMPD->RVSurfLayer(1) = 0.007038850125652322;
    state->dataMstBalEMPD->RVDeepLayer(1) = 0.0051334905162138695;
    state->dataMstBalEMPD->RVdeepOld(1) = 0.0051334905162138695;
    state->dataMstBalEMPD->RVSurfLayerOld(1) = 0.007038850125652322;

    // Do calcs
    Real64 Tsat(0.0);
    MoistureBalanceEMPDManager::CalcMoistureBalanceEMPD(*state, 1, 19.907302679986064, 19.901185713164697, Tsat);

    auto const &report_vars = state->dataMoistureBalEMPD->EMPDReportVars(1);
    EXPECT_DOUBLE_EQ(6.3445188238394508, Tsat);
    EXPECT_DOUBLE_EQ(0.0071762141417078054, state->dataMstBalEMPD->RVSurface(1));
    EXPECT_DOUBLE_EQ(0.00000076900234067835945, report_vars.mass_flux_deep);
    EXPECT_DOUBLE_EQ(-0.00000019077843350248091, report_vars.mass_flux_zone);
    EXPECT_DOUBLE_EQ(0.0070186500259181136, state->dataMstBalEMPD->RVSurfLayer(1));
    EXPECT_DOUBLE_EQ(0.0051469229632164605, state->dataMstBalEMPD->RVDeepLayer(1));
    EXPECT_DOUBLE_EQ(-0.47694608375620229, state->dataMstBalEMPD->HeatFluxLatent(1));
}

TEST_F(EnergyPlusFixture, EMPDAutocalcDepth)
{
    std::string const idf_objects =
        delimited_string({"Material,",
                          "Concrete,                !- Name",
                          "Rough,                   !- Roughness",
                          "0.152,                   !- Thickness {m}",
                          "0.3,                     !- Conductivity {W/m-K}",
                          "850,                     !- Density {kg/m3}",
                          "950,                     !- Specific Heat {J/kg-K}",
                          "0.900000,                !- Thermal Absorptance",
                          "0.600000,                !- Solar Absorptance",
                          "0.600000;                !- Visible Absorptance",
                          "MaterialProperty:MoisturePenetrationDepth:Settings,",
                          "Concrete,                !- Name",
                          "8,                     !- Water Vapor Diffusion Resistance Factor {dimensionless} (mu)",
                          "0.012,                   !- Moisture Equation Coefficient a {dimensionless} (MoistACoeff)",
                          "1,                       !- Moisture Equation Coefficient b {dimensionless} (MoistBCoeff)",
                          "0,                       !- Moisture Equation Coefficient c {dimensionless} (MoistCCoeff)",
                          "1,                       !- Moisture Equation Coefficient d {dimensionless} (MoistDCoeff)",
                          ",                    !- Surface-layer penetration depth {m} (dEMPD)",
                          "autocalculate,                    !- Deep-layer penetration depth {m} (dEPMDdeep)",
                          "0,                       !- Coating layer permeability {m} (CoatingThickness)",
                          "1;                       !- Coating layer water vapor diffusion resistance factor {dimensionless} (muCoating)"});

    ASSERT_TRUE(process_idf(idf_objects));

    bool errors_found(false);
    Material::GetMaterialData(*state, errors_found);
    ASSERT_FALSE(errors_found) << "Errors in GetMaterialData";
    MoistureBalanceEMPDManager::GetMoistureBalanceEMPDInput(*state);

    auto const *matEMPD = dynamic_cast<MoistureBalanceEMPDManager::MaterialEMPD const *>(state->dataMaterial->materials(1));
    ASSERT_NEAR(matEMPD->surfaceDepth, 0.014143, 0.000001);
    ASSERT_NEAR(matEMPD->deepDepth, 0.064810, 0.000001);
}

TEST_F(EnergyPlusFixture, EMPDRcoating)
{
    std::string const idf_objects =
        delimited_string({"Material,",
                          "Concrete,                !- Name",
                          "Rough,                   !- Roughness",
                          "0.152,                   !- Thickness {m}",
                          "0.3,                     !- Conductivity {W/m-K}",
                          "1000,                    !- Density {kg/m3}",
                          "950,                     !- Specific Heat {J/kg-K}",
                          "0.900000,                !- Thermal Absorptance",
                          "0.600000,                !- Solar Absorptance",
                          "0.600000;                !- Visible Absorptance",
                          "MaterialProperty:MoisturePenetrationDepth:Settings,",
                          "Concrete,                !- Name",
                          "6.554,                     !- Water Vapor Diffusion Resistance Factor {dimensionless} (mu)",
                          "0.0661,                   !- Moisture Equation Coefficient a {dimensionless} (MoistACoeff)",
                          "1,                       !- Moisture Equation Coefficient b {dimensionless} (MoistBCoeff)",
                          "0,                       !- Moisture Equation Coefficient c {dimensionless} (MoistCCoeff)",
                          "1,                       !- Moisture Equation Coefficient d {dimensionless} (MoistDCoeff)",
                          "0.006701,                    !- Surface-layer penetration depth {m} (dEMPD)",
                          "0.013402,                    !- Deep-layer penetration depth {m} (dEPMDdeep)",
                          "0.002,                       !- Coating layer permeability {m} (CoatingThickness)",
                          "1;                       !- Coating layer water vapor diffusion resistance factor {dimensionless} (muCoating)"});

    ASSERT_TRUE(process_idf(idf_objects));

    bool errors_found(false);
    Material::GetMaterialData(*state, errors_found);
    ASSERT_FALSE(errors_found) << "Errors in GetMaterialData";

    // Surface
    state->dataSurface->TotSurfaces = 1;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);
    DataSurfaces::SurfaceData &surface = state->dataSurface->Surface(1);
    surface.Name = "Surface1";
    surface.Area = 1.0;
    surface.HeatTransSurf = true;

    // Zone
    surface.Zone = 1;
    state->dataMstBal->RhoVaporAirIn.allocate(1);
    state->dataMstBal->HMassConvInFD.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 20.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.0061285406810457849;

    // Construction
    surface.Construction = 1;
    state->dataConstruction->Construct.allocate(1);
    Construction::ConstructionProps &construction = state->dataConstruction->Construct(1);
    construction.TotLayers = 1;
    construction.LayerPoint(construction.TotLayers) = Material::GetMaterialNum(*state, "CONCRETE");

    // Initialize and get inputs
    MoistureBalanceEMPDManager::InitMoistureBalanceEMPD(*state);

    // Set up conditions
    state->dataGlobal->TimeStepZone = 0.25;
    state->dataEnvrn->OutBaroPress = 101325.;
    state->dataMstBalEMPD->RVSurface(1) = 0.007077173214149593;
    state->dataMstBalEMPD->RVSurfaceOld(1) = state->dataMstBalEMPD->RVSurface(1);
    state->dataMstBal->HMassConvInFD(1) = 0.0016826898264131584;
    state->dataMstBal->RhoVaporAirIn(1) = 0.0073097913062508896;
    state->dataMstBalEMPD->RVSurfLayer(1) = 0.007038850125652322;
    state->dataMstBalEMPD->RVDeepLayer(1) = 0.0051334905162138695;
    state->dataMstBalEMPD->RVdeepOld(1) = 0.0051334905162138695;
    state->dataMstBalEMPD->RVSurfLayerOld(1) = 0.007038850125652322;

    // Do calcs
    Real64 Tsat(0.0);
    MoistureBalanceEMPDManager::CalcMoistureBalanceEMPD(*state, 1, 19.907302679986064, 19.901185713164697, Tsat);

    auto const &report_vars = state->dataMoistureBalEMPD->EMPDReportVars(1);
    EXPECT_DOUBLE_EQ(6.3445188238394508, Tsat);
    EXPECT_DOUBLE_EQ(0.0071815819413115663, state->dataMstBalEMPD->RVSurface(1));
    EXPECT_DOUBLE_EQ(0.00000076900234067835945, report_vars.mass_flux_deep);
    EXPECT_DOUBLE_EQ(-1.8118197009111738e-07, report_vars.mass_flux_zone);
    EXPECT_DOUBLE_EQ(0.0070183147759991828, state->dataMstBalEMPD->RVSurfLayer(1));
    EXPECT_DOUBLE_EQ(0.0051469229632164605, state->dataMstBalEMPD->RVDeepLayer(1));
    EXPECT_DOUBLE_EQ(-0.45295492522779346, state->dataMstBalEMPD->HeatFluxLatent(1));
}

TEST_F(EnergyPlusFixture, CheckEMPDCalc_Slope)
{
    std::string const idf_objects =
        delimited_string({"Material,",
                          "WOOD,                    !- Name",
                          "MediumSmooth,            !- Roughness",
                          "1.9099999E-02,           !- Thickness {m}",
                          "0.1150000,               !- Conductivity {W/m-K}",
                          "513.0000,                !- Density {kg/m3}",
                          "1381.000,                !- Specific Heat {J/kg-K}",
                          "0.900000,                !- Thermal Absorptance",
                          "0.780000,                !- Solar Absorptance",
                          "0.780000;                !- Visible Absorptance",

                          "MaterialProperty:MoisturePenetrationDepth:Settings,",
                          "WOOD,                    !- Name",
                          "150,                     !- Water Vapor Diffusion Resistance Factor {dimensionless} (mu)",
                          "0.204,                   !- Moisture Equation Coefficient a {dimensionless} (MoistACoeff)",
                          "2.32,                    !- Moisture Equation Coefficient b {dimensionless} (MoistBCoeff)",
                          "0.43,                    !- Moisture Equation Coefficient c {dimensionless} (MoistCCoeff)",
                          "72,                      !- Moisture Equation Coefficient d {dimensionless} (MoistDCoeff)",
                          "0.0011,                  !- Surface-layer penetration depth {m} (dEMPD)",
                          "0.004,                   !- Deep-layer penetration depth {m} (dEPMDdeep)",
                          "0,                       !- Coating layer permeability {m} (CoatingThickness)",
                          "0;                       !- Coating layer water vapor diffusion resistance factor {dimensionless} (muCoating)"});

    ASSERT_TRUE(process_idf(idf_objects));

    bool errors_found(false);
    Material::GetMaterialData(*state, errors_found);
    ASSERT_FALSE(errors_found) << "Errors in GetMaterialData";

    // Surface
    int surfNum = 1;
    state->dataSurface->TotSurfaces = 1;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);
    DataSurfaces::SurfaceData &surface = state->dataSurface->Surface(surfNum);
    surface.Name = "SurfaceWood";
    surface.Area = 1.0;
    surface.HeatTransSurf = true;

    // Zone
    int zoneNum = 1;
    surface.Zone = 1;
    state->dataMstBal->RhoVaporAirIn.allocate(surfNum);
    state->dataMstBal->HMassConvInFD.allocate(surfNum);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(zoneNum);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(zoneNum).MAT = 20.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(zoneNum).airHumRat = 0.0061285406810457849;

    // Construction
    int constNum = 1;
    surface.Construction = constNum;
    state->dataConstruction->Construct.allocate(constNum);
    Construction::ConstructionProps &construction = state->dataConstruction->Construct(constNum);
    construction.TotLayers = constNum;
    construction.LayerPoint(construction.TotLayers) = Material::GetMaterialNum(*state, "WOOD");

    // Initialize and get inputs
    MoistureBalanceEMPDManager::InitMoistureBalanceEMPD(*state);

    // Set up conditions
    state->dataGlobal->TimeStepZone = 0.25;
    state->dataEnvrn->OutBaroPress = 101325.;
    state->dataMstBalEMPD->RVSurface(surfNum) = 0.0070277983586713262;
    state->dataMstBalEMPD->RVSurfaceOld(surfNum) = state->dataMstBalEMPD->RVSurface(surfNum);
    state->dataMstBal->HMassConvInFD(surfNum) = 0.0016826898264131584;
    state->dataMstBal->RhoVaporAirIn(surfNum) = 0.0073097913062508896;
    state->dataMstBalEMPD->RVSurfLayer(surfNum) = 0.0070277983586713262;
    state->dataMstBalEMPD->RVDeepLayer(surfNum) = 0.0051402944814058216;
    state->dataMstBalEMPD->RVdeepOld(surfNum) = 0.0051402944814058216;
    state->dataMstBalEMPD->RVSurfLayerOld(surfNum) = 0.0070277983586713262;

    using Psychrometrics::PsyRhFnTdbRhov;

    auto const *matEMPD = dynamic_cast<MoistureBalanceEMPDManager::MaterialEMPD const *>(state->dataMaterial->materials(1));

    Real64 Tsat(0.0);
    state->dataHeatBalSurf->SurfTempIn.allocate(surfNum);
    state->dataHeatBalSurf->SurfTempIn(surfNum) = 20.0;

    // Calculate average vapor density [kg/m^3]
    Real64 Taver = state->dataHeatBalSurf->SurfTempIn(surfNum);
    // Calculate RH for use in material property calculations.
    Real64 RV_Deep_Old = state->dataMstBalEMPD->RVdeepOld(surfNum);
    Real64 RVaver = state->dataMstBalEMPD->RVSurfLayerOld(surfNum);
    Real64 RHaver = RVaver * 461.52 * (Taver + Constant::Kelvin) * std::exp(-23.7093 + 4111.0 / (Taver + 237.7));
    Real64 dU_dRH = matEMPD->moistACoeff * matEMPD->moistBCoeff * pow(RHaver, matEMPD->moistBCoeff - 1) +
                    matEMPD->moistCCoeff * matEMPD->moistDCoeff * pow(RHaver, matEMPD->moistDCoeff - 1);

    // Convert stored vapor density to RH.
    Real64 RH_deep_layer_old = PsyRhFnTdbRhov(*state, Taver, RV_Deep_Old);
    Real64 RH_surf_layer_old = PsyRhFnTdbRhov(*state, Taver, RVaver);
    Real64 mass_flux_surf_deep_max =
        matEMPD->deepDepth * matEMPD->Density * dU_dRH * (RH_surf_layer_old - RH_deep_layer_old) / (state->dataGlobal->TimeStepZone * 3600.0);
    Real64 hm_deep_layer = 6.9551289450635225e-05;
    Real64 mass_flux_surf_deep_result = hm_deep_layer * (RVaver - RV_Deep_Old);
    if (std::abs(mass_flux_surf_deep_max) < std::abs(mass_flux_surf_deep_result)) {
        mass_flux_surf_deep_result = mass_flux_surf_deep_max;
    }

    // Calculate and verify it against the results determined above
    MoistureBalanceEMPDManager::CalcMoistureBalanceEMPD(*state, 1, Taver, Taver, Tsat);
    auto const &report_vars = state->dataMoistureBalEMPD->EMPDReportVars(surfNum);
    EXPECT_DOUBLE_EQ(mass_flux_surf_deep_result, report_vars.mass_flux_deep);
}

TEST_F(EnergyPlusFixture, CheckEMPDWarningMissingOnInsideLayer)
{
    std::string const idf_objects =
        delimited_string({"  Material,",
                          "    000_G01a 19mm gypsum board,  !- Name",
                          "    MediumSmooth,            !- Roughness",
                          "    0.019,                   !- Thickness {m}",
                          "    0.16,                    !- Conductivity {W/m-K}",
                          "    800,                     !- Density {kg/m3}",
                          "    1090,                    !- Specific Heat {J/kg-K}",
                          "    0.9,                     !- Thermal Absorptance",
                          "    0.7,                     !- Solar Absorptance",
                          "    0.7;                     !- Visible Absorptance",

                          "!-  MaterialProperty:MoisturePenetrationDepth:Settings,",
                          "!-    000_G01a 19mm gypsum board,  !- Name",
                          "!-    8.9,                     !- Water Vapor Diffusion Resistance Factor {dimensionless}",
                          "!-    0.0069,                  !- Moisture Equation Coefficient a {dimensionless}",
                          "!-    0.9066,                  !- Moisture Equation Coefficient b {dimensionless}",
                          "!-    0.0404,                  !- Moisture Equation Coefficient c {dimensionless}",
                          "!-    22.1121,                 !- Moisture Equation Coefficient d {dimensionless}",
                          "!-    Autocalculate,           !- Surface Layer Penetration Depth {m}",
                          "!-    Autocalculate,           !- Deep Layer Penetration Depth {m}",
                          "!-    0.005,                   !- Coating Layer Thickness {m}",
                          "!-    140;                     !- Coating Layer Water Vapor Diffusion Resistance Factor {dimensionless}",

                          "  Material,",
                          "    1/2IN Gypsum,            !- Name",
                          "    Smooth,                  !- Roughness",
                          "    0.0127,                  !- Thickness {m}",
                          "    0.16,                    !- Conductivity {W/m-K}",
                          "    784.9,                   !- Density {kg/m3}",
                          "    830,                     !- Specific Heat {J/kg-K}",
                          "    0.9,                     !- Thermal Absorptance",
                          "    0.92,                    !- Solar Absorptance",
                          "    0.92;                    !- Visible Absorptance",

                          "  MaterialProperty:MoisturePenetrationDepth:Settings,",
                          "    1/2IN Gypsum,            !- Name",
                          "    8.9,                     !- Water Vapor Diffusion Resistance Factor {dimensionless}",
                          "    0.0069,                  !- Moisture Equation Coefficient a {dimensionless}",
                          "    0.9066,                  !- Moisture Equation Coefficient b {dimensionless}",
                          "    0.0404,                  !- Moisture Equation Coefficient c {dimensionless}",
                          "    22.1121,                 !- Moisture Equation Coefficient d {dimensionless}",
                          "    Autocalculate,           !- Surface Layer Penetration Depth {m}",
                          "    Autocalculate,           !- Deep Layer Penetration Depth {m}",
                          "    0.005,                   !- Coating Layer Thickness {m}",
                          "    140;                     !- Coating Layer Water Vapor Diffusion Resistance Factor {dimensionless}"});

    ASSERT_TRUE(process_idf(idf_objects));

    bool errors_found(false);
    Material::GetMaterialData(*state, errors_found);
    ASSERT_FALSE(errors_found) << "Errors in GetMaterialData";

    // Surface
    state->dataSurface->TotSurfaces = 1;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);
    DataSurfaces::SurfaceData &surface = state->dataSurface->Surface(1);
    surface.Name = "Surface 26";
    surface.Area = 1.0;
    surface.HeatTransSurf = true;
    surface.HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::EMPD;

    // Zone
    surface.Zone = 1;
    state->dataMstBal->RhoVaporAirIn.allocate(1);
    state->dataMstBal->HMassConvInFD.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 20.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.0061285406810457849;

    // Construction
    surface.Construction = 1;
    state->dataConstruction->Construct.allocate(1);
    Construction::ConstructionProps &construction = state->dataConstruction->Construct(1);
    construction.Name = "000_Interior Wall";
    construction.TotLayers = 1;
    construction.LayerPoint(construction.TotLayers) = Material::GetMaterialNum(*state, "000_G01A 19MM GYPSUM BOARD");

    // Initialize and get inputs
    state->dataGlobal->DisplayExtraWarnings = true;
    MoistureBalanceEMPDManager::InitMoistureBalanceEMPD(*state);

    EXPECT_TRUE(compare_err_stream_substring(
        delimited_string({"   ************* GetMoistureBalanceEMPDInput: EMPD properties are not assigned to the inside layer in Surface=Surface 26",
                          "   **   ~~~   ** with Construction=000_Interior Wall"})));
}

TEST_F(EnergyPlusFixture, CheckEMPDNoSegFault)
{
    // Regression test for #10777: CalcMoistureBalanceEMPD should not segfault when an EMPD surface's
    // inside-layer material lacks EMPD properties (dynamic_cast returns nullptr), so the routine bails out cleanly.
    std::string const idf_objects =
        delimited_string({"  Material,",
                          "    1/2IN Gypsum,            !- Name",
                          "    Smooth,                  !- Roughness",
                          "    0.0127,                  !- Thickness {m}",
                          "    0.16,                    !- Conductivity {W/m-K}",
                          "    784.9,                   !- Density {kg/m3}",
                          "    830,                     !- Specific Heat {J/kg-K}",
                          "    0.9,                     !- Thermal Absorptance",
                          "    0.92,                    !- Solar Absorptance",
                          "    0.92;                    !- Visible Absorptance",

                          "  MaterialProperty:MoisturePenetrationDepth:Settings,",
                          "    1/2IN Gypsum,            !- Name",
                          "    8.9,                     !- Water Vapor Diffusion Resistance Factor {dimensionless}",
                          "    0.0069,                  !- Moisture Equation Coefficient a {dimensionless}",
                          "    0.9066,                  !- Moisture Equation Coefficient b {dimensionless}",
                          "    0.0404,                  !- Moisture Equation Coefficient c {dimensionless}",
                          "    22.1121,                 !- Moisture Equation Coefficient d {dimensionless}",
                          "    Autocalculate,           !- Surface Layer Penetration Depth {m}",
                          "    Autocalculate,           !- Deep Layer Penetration Depth {m}",
                          "    0.005,                   !- Coating Layer Thickness {m}",
                          "    140;                     !- Coating Layer Water Vapor Diffusion Resistance Factor {dimensionless}",

                          "  Material:NoMass,",
                          "    CP02 CARPET PAD,         !- Name",
                          "    VeryRough,               !- Roughness",
                          "    0.2165,                  !- Thermal Resistance {m2-K/W}",
                          "    0.9,                     !- Thermal Absorptance",
                          "    0.7,                     !- Solar Absorptance",
                          "    0.8;                     !- Visible Absorptance"});

    ASSERT_TRUE(process_idf(idf_objects));

    bool errors_found(false);
    Material::GetMaterialData(*state, errors_found);
    ASSERT_FALSE(errors_found) << "Errors in GetMaterialData";

    // Surface
    int surfNum = 1;
    state->dataSurface->TotSurfaces = 1;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);
    DataSurfaces::SurfaceData &surface = state->dataSurface->Surface(surfNum);
    surface.Name = "Surface 25";
    surface.Area = 1.0;
    surface.HeatTransSurf = true;
    surface.HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::EMPD;

    // Zone
    surface.Zone = 1;
    state->dataMstBal->RhoVaporAirIn.allocate(1);
    state->dataMstBal->HMassConvInFD.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 20.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.0061285406810457849;

    // Construction
    surface.Construction = 1;
    state->dataConstruction->Construct.allocate(1);
    Construction::ConstructionProps &construction = state->dataConstruction->Construct(1);
    construction.Name = "000_ExtSlabCarpet_4in_ClimateZone 1-8";
    construction.TotLayers = 1;
    construction.LayerPoint(construction.TotLayers) = Material::GetMaterialNum(*state, "CP02 CARPET PAD");

    // Initialize and get inputs
    MoistureBalanceEMPDManager::InitMoistureBalanceEMPD(*state);

    // Set up conditions
    state->dataGlobal->TimeStepZone = 0.25;
    state->dataEnvrn->OutBaroPress = 101325.;
    state->dataMstBalEMPD->RVSurface(surfNum) = 0.0070277983586713262;
    state->dataMstBalEMPD->RVSurfaceOld(surfNum) = state->dataMstBalEMPD->RVSurface(surfNum);
    state->dataMstBal->HMassConvInFD(surfNum) = 0.0016826898264131584;
    state->dataMstBal->RhoVaporAirIn(surfNum) = 0.0073097913062508896;
    state->dataMstBalEMPD->RVSurfLayer(surfNum) = 0.0070277983586713262;
    state->dataMstBalEMPD->RVDeepLayer(surfNum) = 0.0051402944814058216;
    state->dataMstBalEMPD->RVdeepOld(surfNum) = 0.0051402944814058216;
    state->dataMstBalEMPD->RVSurfLayerOld(surfNum) = 0.0070277983586713262;

    Real64 Tsat(0.0);
    state->dataHeatBalSurf->SurfTempIn.allocate(surfNum);
    state->dataHeatBalSurf->SurfTempIn(surfNum) = 20.0;
    Real64 Taver = state->dataHeatBalSurf->SurfTempIn(surfNum);
    MoistureBalanceEMPDManager::CalcMoistureBalanceEMPD(*state, 1, Taver, Taver, Tsat);

    EXPECT_TRUE(compare_err_stream_substring(
        delimited_string({"   ************* GetMoistureBalanceEMPDInput: EMPD properties are not assigned to the inside layer of Surfaces",
                          "   **   ~~~   ** ...use Output:Diagnostics,DisplayExtraWarnings; to show more details on individual surfaces."})));
}
