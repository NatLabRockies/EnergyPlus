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

// EnergyPlus::SurfaceGroundHeatExchanger Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/SurfaceGroundHeatExchanger.hh>

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, test_eoshiftArrayPos)
{
    const std::array<Real64, 19> A({1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0});
    std::array<Real64, 19> B = SurfaceGroundHeatExchanger::eoshiftArray(A, 2, 0.0);
    std::array<Real64, 19> target = {3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0, 0.0, 0.0};
    for (size_t i = 0; i < B.size(); i++) {
        EXPECT_EQ(B[i], target[i]);
    }
}

TEST_F(EnergyPlusFixture, test_eoshiftArrayNeg)
{
    const std::array<Real64, 19> A({1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0});
    std::array<Real64, 19> B = SurfaceGroundHeatExchanger::eoshiftArray(A, -1, 0.0);
    std::array<Real64, 19> target = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0};
    for (size_t i = 0; i < B.size(); i++) {
        EXPECT_EQ(B[i], target[i]);
    }
}

TEST(SurfaceGroundHeatExchanger, EstimateSnowAndFreezingRainAccumulation)
{
    EXPECT_DOUBLE_EQ(SurfaceGroundHeatExchanger::estimateSnowIceAccumulation(0.1, 0.002, true), 12.0);
    EXPECT_DOUBLE_EQ(SurfaceGroundHeatExchanger::estimateSnowIceAccumulation(-0.1, 0.002, false), 0.0);
    EXPECT_DOUBLE_EQ(SurfaceGroundHeatExchanger::estimateSnowIceAccumulation(0.0, 0.002, false), 0.0);
}

TEST(SurfaceGroundHeatExchanger, MeltFluxIsLimitedByAvailableSnowAndIceMass)
{
    constexpr Real64 IceMeltEnthalpy = 335000.0;
    constexpr Real64 TimeStepSec = 3600.0;

    EXPECT_DOUBLE_EQ(SurfaceGroundHeatExchanger::calculateSnowIceMeltFlux(0.0, 1.0, TimeStepSec), 0.0);
    EXPECT_DOUBLE_EQ(SurfaceGroundHeatExchanger::calculateSnowIceMeltFlux(100.0, 0.0, TimeStepSec), 0.0);
    EXPECT_DOUBLE_EQ(SurfaceGroundHeatExchanger::calculateSnowIceMeltFlux(100.0, 2.0, TimeStepSec), 100.0);
    EXPECT_DOUBLE_EQ(SurfaceGroundHeatExchanger::calculateSnowIceMeltFlux(1000.0, 1.0, TimeStepSec), IceMeltEnthalpy / TimeStepSec);
}

TEST(SurfaceGroundHeatExchanger, SnowMeltPinsSurfaceToFreezingUntilMassIsDepleted)
{
    SurfaceGroundHeatExchanger::SurfaceGroundHeatExchangerData surface;
    surface.TopRoughness = Material::SurfaceRoughness::MediumRough;
    surface.TopThermAbs = 0.9;
    surface.TopSolarAbs = 0.9;

    Real64 surfaceTemperature = 0.0;
    Real64 meltFlux = 0.0;
    surface.CalcTopSurfTemp(0.0, surfaceTemperature, 20.0, 20.0, 0.0, 0.0, 1.0, false, false, 1000.0, 3600.0, meltFlux);
    EXPECT_NEAR(surfaceTemperature, 0.0, 1.0e-8);
    EXPECT_GT(meltFlux, 0.0);

    surface.CalcTopSurfTemp(0.0, surfaceTemperature, 20.0, 20.0, 0.0, 0.0, 1.0, false, false, 0.001, 3600.0, meltFlux);
    EXPECT_GT(surfaceTemperature, 0.0);
    EXPECT_NEAR(meltFlux * 3600.0, 0.001 * 335000.0, 1.0e-8);
}

TEST_F(EnergyPlusFixture, SurfaceGroundHeatExchangerTracksWeatherAccumulationAndFreezingRain)
{
    SurfaceGroundHeatExchanger::SurfaceGroundHeatExchangerData surface;

    state->dataEnvrn->SnowDepth = 0.1;
    surface.UpdateSnowIceAccumulation(*state);
    EXPECT_DOUBLE_EQ(surface.SnowIceMass, 10.0);

    state->dataEnvrn->SnowDepth = 0.15;
    surface.UpdateSnowIceAccumulation(*state);
    EXPECT_DOUBLE_EQ(surface.SnowIceMass, 15.0);

    state->dataEnvrn->SnowDepth = 0.05;
    surface.UpdateSnowIceAccumulation(*state);
    EXPECT_DOUBLE_EQ(surface.SnowIceMass, 15.0);

    state->dataEnvrn->IsRain = true;
    state->dataEnvrn->OutDryBulbTemp = -1.0;
    state->dataEnvrn->LiquidPrecipitation = 0.002;
    surface.UpdateSnowIceAccumulation(*state);
    EXPECT_DOUBLE_EQ(surface.SnowIceMass, 17.0);
}

TEST_F(EnergyPlusFixture, SurfaceGroundHeatExchangerUsesShadedSurfaceSolarRadiation)
{
    SurfaceGroundHeatExchanger::SurfaceGroundHeatExchangerData surface;
    state->dataHeatBal->SurfQRadSWOutIncident.allocate(1);
    state->dataHeatBal->SurfQRadSWOutIncident(1) = 250.0;
    surface.ShadingSurfaceNum = 1;
    EXPECT_DOUBLE_EQ(surface.IncidentSolarRadiation(*state), 250.0);

    surface.ShadingSurfaceNum = 0;
    state->dataEnvrn->SOLCOS.z = 0.5;
    state->dataEnvrn->BeamSolarRad = 400.0;
    state->dataEnvrn->DifSolarRad = 100.0;
    EXPECT_DOUBLE_EQ(surface.IncidentSolarRadiation(*state), 300.0);
}
