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

// EnergyPlus::WindowManager unit tests

// C++ Headers
#include <iostream>

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include "Windows-CalcEngine/src/Common/src/FenestrationCommon.hpp"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/WindowManager.hh>
#include <EnergyPlus/WindowManagerExteriorData.hh>
#include <WCEMultiLayerOptics.hpp>

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, WCEClear)
{

    state->dataIPShortCut->lAlphaFieldBlanks = true;
    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({"WindowsCalculationEngine,",
                                                      "ExternalWindowsModel;",
                                                      "WindowMaterial:Glazing,",
                                                      "Glass_102_LayerAvg,      !- Name",
                                                      "SpectralAverage,         !- Optical Data Type",
                                                      ",                        !- Window Glass Spectral Data Set Name",
                                                      "0.003048,                !- Thickness {m}",
                                                      "0.833848,                !- Solar Transmittance at Normal Incidence",
                                                      "7.476376e-002,           !- Front Side Solar Reflectance at Normal Incidence",
                                                      "7.485449e-002,           !- Back Side Solar Reflectance at Normal Incidence",
                                                      "0.899260,                !- Visible Transmittance at Normal Incidence",
                                                      "0.082563,                !- Front Side Visible Reflectance at Normal Incidence",
                                                      "0.082564,                !- Back Side Visible Reflectance at Normal Incidence",
                                                      "0.000000,                !- Infrared Transmittance at Normal Incidence",
                                                      "0.840000,                !- Front Side Infrared Hemispherical Emissivity",
                                                      "0.840000,                !- Back Side Infrared Hemispherical Emissivity",
                                                      "1.000000;                !- Conductivity {W/m-K}",
                                                      "CONSTRUCTION,",
                                                      "GlzSys_1,                !- Name",
                                                      "Glass_102_LayerAvg;      !- Outside Layer"});

    ASSERT_TRUE(process_idf(idf_objects));

    Material::GetMaterialData(*state, ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound);
    Window::initWindowModel(*state);
    Window::InitWindowOpticalCalculations(*state);
    HeatBalanceManager::InitHeatBalance(*state);

    auto aWinConstSimp = Window::CWindowConstructionsSimplified::instance(*state);
    auto solarLayer = aWinConstSimp.getEquivalentLayer(*state, FenestrationCommon::WavelengthRange::Solar, 1);

    constexpr Real64 minLambda{0.3};
    constexpr Real64 maxLambda{2.5};

    // Transmittance Front
    const auto Tfront = solarLayer->getPropertySimple(
        minLambda, maxLambda, FenestrationCommon::PropertySimple::T, FenestrationCommon::Side::Front, FenestrationCommon::Scattering::DirectDirect);
    EXPECT_NEAR(0.833848, Tfront, 1e-6);

    // Reflectance Front
    const auto Rfront = solarLayer->getPropertySimple(
        minLambda, maxLambda, FenestrationCommon::PropertySimple::R, FenestrationCommon::Side::Front, FenestrationCommon::Scattering::DirectDirect);
    EXPECT_NEAR(0.074764, Rfront, 1e-6);

    // Transmittance Back
    const auto Tback = solarLayer->getPropertySimple(
        minLambda, maxLambda, FenestrationCommon::PropertySimple::T, FenestrationCommon::Side::Back, FenestrationCommon::Scattering::DirectDirect);
    EXPECT_NEAR(0.833848, Tback, 1e-6);

    // Reflectance Back
    const auto Rback = solarLayer->getPropertySimple(
        minLambda, maxLambda, FenestrationCommon::PropertySimple::R, FenestrationCommon::Side::Back, FenestrationCommon::Scattering::DirectDirect);
    EXPECT_NEAR(0.074854, Rback, 1e-6);
}

TEST_F(EnergyPlusFixture, WCEVenetian)
{

    state->dataIPShortCut->lAlphaFieldBlanks = true;
    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({"WindowsCalculationEngine,",
                                                      "ExternalWindowsModel;",
                                                      "WindowMaterial:Glazing,",
                                                      "Glass_102_LayerAvg,      !- Name",
                                                      "SpectralAverage,         !- Optical Data Type",
                                                      ",                        !- Window Glass Spectral Data Set Name",
                                                      "0.003048,                !- Thickness {m}",
                                                      "0.833848,                !- Solar Transmittance at Normal Incidence",
                                                      "7.476376e-002,           !- Front Side Solar Reflectance at Normal Incidence",
                                                      "7.485449e-002,           !- Back Side Solar Reflectance at Normal Incidence",
                                                      "0.899260,                !- Visible Transmittance at Normal Incidence",
                                                      "0.082563,                !- Front Side Visible Reflectance at Normal Incidence",
                                                      "0.082564,                !- Back Side Visible Reflectance at Normal Incidence",
                                                      "0.000000,                !- Infrared Transmittance at Normal Incidence",
                                                      "0.840000,                !- Front Side Infrared Hemispherical Emissivity",
                                                      "0.840000,                !- Back Side Infrared Hemispherical Emissivity",
                                                      "1.000000;                !- Conductivity {W/m-K}",
                                                      "CONSTRUCTION,",
                                                      "GlzSys_1_withShade,      !- Name",
                                                      "Glass_102_LayerAvg,      !- Outside Layer",
                                                      "VenBlind_ShdDvc_25;      !- Layer 2",
                                                      "CONSTRUCTION,",
                                                      "GlzSys_1,                !- Name",
                                                      "Glass_102_LayerAvg;      !- Outside Layer",
                                                      "WindowMaterial:Blind,",
                                                      "VenBlind_ShdDvc_25,      !- Name",
                                                      "HORIZONTAL,              !- Slat Orientation",
                                                      "0.016,                   !- Slat Width {m}",
                                                      "0.012,                   !- Slat Separation {m}",
                                                      "0.0006,                  !- Slat Thickness {m}",
                                                      "135,                     !- Slat Angle {deg}",
                                                      "160,                     !- Slat Conductivity {W/m-K}",
                                                      ",                        !- Slat Beam Solar Transmittance",
                                                      "0.7,                     !- Front Side Slat Beam Solar Reflectance",
                                                      "0.7,                     !- Back Side Slat Beam Solar Reflectance",
                                                      "0,                       !- Slat Diffuse Solar Transmittance",
                                                      "0.7,                     !- Front Side Slat Diffuse Solar Reflectance",
                                                      "0.7,                     !- Back Side Slat Diffuse Solar Reflectance",
                                                      "0,                       !- Slat Beam Visible Transmittance",
                                                      "0.7,                     !- Front Side Slat Beam Visible Reflectance",
                                                      "0.7,                     !- Back Side Slat Beam Visible Reflectance",
                                                      "0,                       !- Slat Diffuse Visible Transmittance",
                                                      "0.7,                     !- Front Side Slat Diffuse Visible Reflectance",
                                                      "0.7,                     !- Back Side Slat Diffuse Visible Reflectance",
                                                      "0,                       !- Slat Infrared Hemispherical Transmittance",
                                                      "0.9,                     !- Front Side Slat Infrared Hemispherical Emissivity",
                                                      "0.9,                     !- Back Side Slat Infrared Hemispherical Emissivity",
                                                      "0.0127,                  !- Blind to Glass Distance {m}",
                                                      "0.0,                     !- Blind Top Opening Multiplier",
                                                      "0.0,                     !- Blind Bottom Opening Multiplier",
                                                      "0.0,                     !- Blind Left Side Opening Multiplier",
                                                      "0.0,                     !- Blind Right Side Opening Multiplier",
                                                      "0,                       !- Minimum Slat Angle {deg}",
                                                      "0;                       !- Maximum Slat Angle {deg}"});

    ASSERT_TRUE(process_idf(idf_objects));

    Material::GetMaterialData(*state, ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound);
    Window::initWindowModel(*state);
    Window::InitWindowOpticalCalculations(*state);
    HeatBalanceManager::InitHeatBalance(*state);

    auto aWinConstSimp = Window::CWindowConstructionsSimplified::instance(*state);
    auto solarLayer = aWinConstSimp.getEquivalentLayer(*state, FenestrationCommon::WavelengthRange::Solar, 1);

    constexpr Real64 minLambda{0.3};
    constexpr Real64 maxLambda{2.5};

    // Transmittance Front
    const auto Tfront = solarLayer->getPropertySimple(
        minLambda, maxLambda, FenestrationCommon::PropertySimple::T, FenestrationCommon::Side::Front, FenestrationCommon::Scattering::DirectDirect);
    EXPECT_NEAR(0.833829, Tfront, 1e-6);

    // Reflectance Front
    const auto Rfront = solarLayer->getPropertySimple(
        minLambda, maxLambda, FenestrationCommon::PropertySimple::R, FenestrationCommon::Side::Front, FenestrationCommon::Scattering::DirectDirect);
    EXPECT_NEAR(0.074764, Rfront, 1e-6);

    // Transmittance Back
    const auto Tback = solarLayer->getPropertySimple(
        minLambda, maxLambda, FenestrationCommon::PropertySimple::T, FenestrationCommon::Side::Back, FenestrationCommon::Scattering::DirectDirect);
    EXPECT_NEAR(0.833848, Tback, 1e-6);

    // Reflectance Back
    const auto Rback = solarLayer->getPropertySimple(
        minLambda, maxLambda, FenestrationCommon::PropertySimple::R, FenestrationCommon::Side::Back, FenestrationCommon::Scattering::DirectDirect);
    EXPECT_NEAR(0.074853, Rback, 1e-6);
}

TEST_F(EnergyPlusFixture, WCEShade)
{

    state->dataIPShortCut->lAlphaFieldBlanks = true;
    bool ErrorsFound(false);

    std::string const idf_objects = delimited_string({"WindowsCalculationEngine,",
                                                      "ExternalWindowsModel;",
                                                      "WindowMaterial:Glazing,",
                                                      "Glass_102_LayerAvg,      !- Name",
                                                      "SpectralAverage,         !- Optical Data Type",
                                                      ",                        !- Window Glass Spectral Data Set Name",
                                                      "0.003048,                !- Thickness {m}",
                                                      "0.833848,                !- Solar Transmittance at Normal Incidence",
                                                      "7.476376e-002,           !- Front Side Solar Reflectance at Normal Incidence",
                                                      "7.485449e-002,           !- Back Side Solar Reflectance at Normal Incidence",
                                                      "0.899260,                !- Visible Transmittance at Normal Incidence",
                                                      "0.082563,                !- Front Side Visible Reflectance at Normal Incidence",
                                                      "0.082564,                !- Back Side Visible Reflectance at Normal Incidence",
                                                      "0.000000,                !- Infrared Transmittance at Normal Incidence",
                                                      "0.840000,                !- Front Side Infrared Hemispherical Emissivity",
                                                      "0.840000,                !- Back Side Infrared Hemispherical Emissivity",
                                                      "1.000000;                !- Conductivity {W/m-K}",
                                                      "CONSTRUCTION,",
                                                      "GlzSys_1_withShade,      !- Name",
                                                      "Glass_102_LayerAvg,      !- Outside Layer",
                                                      "Shade_1;                 !- Layer 2",
                                                      "WindowMaterial:Shade,",
                                                      "Shade_1, !- Name",
                                                      "0.35, !- Solar Transmittance{ dimensionless }",
                                                      "0.2, !- Solar Reflectance{ dimensionless }",
                                                      "0.8, !- Visible Transmittance{ dimensionless }",
                                                      "0.05, !- Visible Reflectance{ dimensionless }",
                                                      "0.9, !- Infrared Hemispherical Emissivity{ dimensionless }",
                                                      "0, !- Infrared Transmittance{ dimensionless }",
                                                      "0.1, !- Thickness{ m }",
                                                      "1, !- Conductivity{ W / m - K }",
                                                      "0.016, !- Shade to Glass Distance{ m }",
                                                      "0.0, !- Top Opening Multiplier",
                                                      "0.0, !- Bottom Opening Multiplier",
                                                      "0.0, !- Left - Side Opening Multiplier",
                                                      "0.0, !- Right - Side Opening Multiplier",
                                                      "0;                       !- Airflow Permeability{ dimensionless }"});

    ASSERT_TRUE(process_idf(idf_objects));

    Material::GetMaterialData(*state, ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound);
    Window::initWindowModel(*state);
    Window::InitWindowOpticalCalculations(*state);
    HeatBalanceManager::InitHeatBalance(*state);

    auto aWinConstSimp = Window::CWindowConstructionsSimplified::instance(*state);
    auto solarLayer = aWinConstSimp.getEquivalentLayer(*state, FenestrationCommon::WavelengthRange::Solar, 1);

    constexpr Real64 minLambda{0.3};
    constexpr Real64 maxLambda{2.5};

    // Transmittance Front
    const auto Tfront_dir_dir = solarLayer->getPropertySimple(
        minLambda, maxLambda, FenestrationCommon::PropertySimple::T, FenestrationCommon::Side::Front, FenestrationCommon::Scattering::DirectDirect);
    EXPECT_NEAR(0.0, Tfront_dir_dir, 1e-6);

    const auto Tfront_dif = solarLayer->getPropertySimple(
        minLambda, maxLambda, FenestrationCommon::PropertySimple::T, FenestrationCommon::Side::Front, FenestrationCommon::Scattering::DiffuseDiffuse);
    EXPECT_NEAR(0.296282, Tfront_dif, 1e-6);

    // Reflectance Front
    const auto Rfront = solarLayer->getPropertySimple(
        minLambda, maxLambda, FenestrationCommon::PropertySimple::R, FenestrationCommon::Side::Front, FenestrationCommon::Scattering::DirectDirect);
    EXPECT_NEAR(0.074764, Rfront, 1e-6);

    // Transmittance Back
    const auto Tback = solarLayer->getPropertySimple(
        minLambda, maxLambda, FenestrationCommon::PropertySimple::T, FenestrationCommon::Side::Back, FenestrationCommon::Scattering::DirectDirect);
    EXPECT_NEAR(0.0, Tback, 1e-6);

    // Reflectance Back
    const auto Rback = solarLayer->getPropertySimple(
        minLambda, maxLambda, FenestrationCommon::PropertySimple::R, FenestrationCommon::Side::Back, FenestrationCommon::Scattering::DirectDirect);
    EXPECT_NEAR(0.0, Rback, 1e-6);
}
