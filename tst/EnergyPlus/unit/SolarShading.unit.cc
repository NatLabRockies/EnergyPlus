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

// EnergyPlus::SolarShading Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataErrorTracking.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataShadowingCombinations.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/DataVectorTypes.hh>
#include <EnergyPlus/DaylightingManager.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/HeatBalanceIntRadExchange.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/HeatBalanceSurfaceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/SizingManager.hh>
#include <EnergyPlus/SolarShading.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WindowManager.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::SolarShading;
using namespace EnergyPlus::DataSurfaces;
using namespace EnergyPlus::DataSystemVariables;
using namespace EnergyPlus::DataHeatBalance;
using namespace EnergyPlus::DataBSDFWindow;
using namespace EnergyPlus::DataVectorTypes;
using namespace EnergyPlus::DataShadowingCombinations;

TEST_F(EnergyPlusFixture, SolarShadingTest_CalcPerSolarBeamTest)
{
    // Test inits for integrated and non-integrated shading calcs

    //	static bool ErrorsFound( false ); // If errors detected in input
    //	static int ZoneNum( 0 ); // Zone number
    //	int NumAlphas( 2 );
    //	int NumNumbers( 9 );
    Real64 AvgEqOfTime(0.0);       // Average value of Equation of Time for period
    Real64 AvgSinSolarDeclin(1.0); // Average value of Sine of Solar Declination for period
    Real64 AvgCosSolarDeclin(0.0); // Average value of Cosine of Solar Declination for period
    int constexpr NumTimeSteps(6);
    int constexpr HoursInDay(24);

    state->dataGlobal->TimeStep = 1;
    state->dataSurface->TotSurfaces = 3;
    state->dataBSDFWindow->MaxBkSurf = 3;
    state->dataSurface->SurfaceWindow.allocate(state->dataSurface->TotSurfaces);
    state->dataHeatBal->SurfSunlitFracHR.allocate(HoursInDay, state->dataSurface->TotSurfaces);
    state->dataHeatBal->SurfSunlitFrac.allocate(HoursInDay, NumTimeSteps, state->dataSurface->TotSurfaces);
    state->dataHeatBal->SurfSunlitFracWithoutReveal.allocate(HoursInDay, NumTimeSteps, state->dataSurface->TotSurfaces);
    state->dataSolarShading->SurfSunCosTheta.allocate(state->dataSurface->TotSurfaces);
    state->dataHeatBal->SurfCosIncAngHR.allocate(HoursInDay, state->dataSurface->TotSurfaces);
    state->dataHeatBal->SurfCosIncAng.allocate(HoursInDay, NumTimeSteps, state->dataSurface->TotSurfaces);
    state->dataSurface->SurfOpaqAO.allocate(state->dataSurface->TotSurfaces);
    state->dataHeatBal->SurfWinBackSurfaces.allocate(HoursInDay, NumTimeSteps, state->dataBSDFWindow->MaxBkSurf, state->dataSurface->TotSurfaces);
    state->dataHeatBal->SurfWinOverlapAreas.allocate(HoursInDay, NumTimeSteps, state->dataBSDFWindow->MaxBkSurf, state->dataSurface->TotSurfaces);
    state->dataSurface->SurfSunCosHourly.allocate(HoursInDay);
    for (int hour = 1; hour <= HoursInDay; hour++) {
        state->dataSurface->SurfSunCosHourly(hour) = 0.0;
    }
    // Test non-integrated option first, CalcPerSolarBeam should set OutProjSLFracMult and InOutProjSLFracMult to 1.0 for all hours
    for (int SurfNum = 1; SurfNum <= state->dataSurface->TotSurfaces; ++SurfNum) {
        for (int Hour = 1; Hour <= HoursInDay; ++Hour) {
            state->dataSurface->SurfaceWindow(SurfNum).OutProjSLFracMult[Hour] = 999.0;
            state->dataSurface->SurfaceWindow(SurfNum).InOutProjSLFracMult[Hour] = 888.0;
        }
    }

    state->dataSysVars->DetailedSolarTimestepIntegration = false;
    CalcPerSolarBeam(*state, AvgEqOfTime, AvgSinSolarDeclin, AvgCosSolarDeclin);

    for (int SurfNum = 1; SurfNum <= state->dataSurface->TotSurfaces; ++SurfNum) {
        for (int Hour = 1; Hour <= HoursInDay; ++Hour) {
            EXPECT_EQ(1.0, state->dataSurface->SurfaceWindow(SurfNum).OutProjSLFracMult[Hour]);
            EXPECT_EQ(1.0, state->dataSurface->SurfaceWindow(SurfNum).InOutProjSLFracMult[Hour]);
        }
    }

    // Test integrated option, CalcPerSolarBeam should set OutProjSLFracMult and InOutProjSLFracMult to 1.0 only for the specified hour
    // Re-initialize to new values
    for (int SurfNum = 1; SurfNum <= state->dataSurface->TotSurfaces; ++SurfNum) {
        for (int Hour = 1; Hour <= HoursInDay; ++Hour) {
            state->dataSurface->SurfaceWindow(SurfNum).OutProjSLFracMult[Hour] = 555.0;
            state->dataSurface->SurfaceWindow(SurfNum).InOutProjSLFracMult[Hour] = 444.0;
        }
    }

    state->dataSysVars->DetailedSolarTimestepIntegration = true;
    state->dataGlobal->HourOfDay = 23;
    CalcPerSolarBeam(*state, AvgEqOfTime, AvgSinSolarDeclin, AvgCosSolarDeclin);

    for (int SurfNum = 1; SurfNum <= state->dataSurface->TotSurfaces; ++SurfNum) {
        for (int Hour = 1; Hour <= HoursInDay; ++Hour) {
            if (Hour == state->dataGlobal->HourOfDay) {
                EXPECT_EQ(1.0, state->dataSurface->SurfaceWindow(SurfNum).OutProjSLFracMult[Hour]);
                EXPECT_EQ(1.0, state->dataSurface->SurfaceWindow(SurfNum).InOutProjSLFracMult[Hour]);
            } else {
                EXPECT_EQ(555.0, state->dataSurface->SurfaceWindow(SurfNum).OutProjSLFracMult[Hour]);
                EXPECT_EQ(444.0, state->dataSurface->SurfaceWindow(SurfNum).InOutProjSLFracMult[Hour]);
            }
        }
    }

    // Clean up
    state->dataSurface->SurfaceWindow.deallocate();
    state->dataHeatBal->SurfSunlitFracHR.deallocate();
    state->dataHeatBal->SurfSunlitFrac.deallocate();
    state->dataHeatBal->SurfSunlitFracWithoutReveal.deallocate();
    state->dataSolarShading->SurfSunCosTheta.deallocate();
    state->dataHeatBal->SurfCosIncAngHR.deallocate();
    state->dataHeatBal->SurfCosIncAng.deallocate();
    state->dataSurface->SurfOpaqAO.deallocate();
    state->dataHeatBal->SurfWinBackSurfaces.deallocate();
    state->dataHeatBal->SurfWinOverlapAreas.deallocate();
}

TEST_F(EnergyPlusFixture, SolarShadingTest_CalcPerSolarBeamTestResetsFrac)
{
    // Test for #11181 - Test inits for integrated and non-integrated shading calcs, including for DETACHED shading surfaces
    // We are going to mimic the case where we have two surfaces:
    // 1) A surface part a zone
    // 2) A detached or building shading surface, not part of any zone
    //
    // We also assume we are in a case where we're starting a new environment, and the SurfSunlitFrac is already filled with numbers
    // The issue with #11181 was that the arrays were NOT being cleared for the detached shading surfaces, so the previous values were retained when
    // the Sun was down, because FigureSolarBeamAtTimestep returns early in that case

    state->dataGlobal->TimeStepsInHour = 4;
    state->dataGlobal->TimeStepZone = 1.0 / state->dataGlobal->TimeStepsInHour;

    // Mimic December 1 in Chicago
    state->dataEnvrn->DayOfYear = 335;
    state->dataEnvrn->Month = 12;
    state->dataEnvrn->DayOfMonth = 1;
    state->dataGlobal->TimeStep = 1;

    Real64 AvgSinSolarDeclin = 0.0;
    Real64 AvgEqOfTime = 0.0;                                                        // Average value of Equation of Time for period
    SolarShading::SUN3(state->dataEnvrn->DayOfYear, AvgSinSolarDeclin, AvgEqOfTime); // Average value of Sine of Solar Declination for period
    Real64 const AvgCosSolarDeclin = std::sqrt(1.0 - pow_2(AvgSinSolarDeclin));      // Average value of Cosine of Solar Declination for period

    EXPECT_NEAR(0.18712816682814074, AvgEqOfTime, 0.0000000000000001);
    EXPECT_NEAR(-0.36958738195178753, AvgSinSolarDeclin, 0.0000000000000001);
    EXPECT_NEAR(0.92919597884516458, AvgCosSolarDeclin, 0.0000000000000001);

    state->dataEnvrn->Latitude = 41.98;
    state->dataEnvrn->Longitude = -87.92;
    state->dataEnvrn->SinLatitude = std::sin(Constant::DegToRad * state->dataEnvrn->Latitude);
    state->dataEnvrn->CosLatitude = std::cos(Constant::DegToRad * state->dataEnvrn->Latitude);
    state->dataEnvrn->TimeZoneNumber = -6.0;
    state->dataEnvrn->TimeZoneMeridian = state->dataEnvrn->TimeZoneNumber * 15.0; // -90.0

    state->dataSurface->TotSurfaces = 2;
    state->dataBSDFWindow->MaxBkSurf = 2;
    int constexpr zoneSurfaceIndex = 1;
    int constexpr shadingSurfaceIndex = 2;

    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);
    state->dataSurface->SurfaceWindow.allocate(state->dataSurface->TotSurfaces);
    EnergyPlus::SurfaceGeometry::AllocateSurfaceWindows(*state, state->dataSurface->TotSurfaces);

    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    auto &thisZone = state->dataHeatBal->Zone(1);
    thisZone.Name = "ZONE 1";
    thisZone.spaceIndexes.emplace_back(1);
    state->dataHeatBal->space.allocate(1);
    auto &thisSpace = state->dataHeatBal->space(1);
    thisSpace.HTSurfaceFirst = zoneSurfaceIndex;
    thisSpace.HTSurfaceLast = zoneSurfaceIndex;

    auto &zoneSurface = state->dataSurface->Surface(zoneSurfaceIndex);
    zoneSurface.Name = "ROOF";
    zoneSurface.Class = DataSurfaces::SurfaceClass::Wall;
    zoneSurface.Zone = 1;
    zoneSurface.spaceNum = 1;
    zoneSurface.BaseSurf = zoneSurfaceIndex;
    zoneSurface.HeatTransSurf = true;
    zoneSurface.ExtSolar = true;
    zoneSurface.OutNormVec = {0.0, 0.0, 1.0};
    zoneSurface.Sides = 4;
    zoneSurface.Vertex.allocate(zoneSurface.Sides);
    zoneSurface.Vertex(1) = {10.0, 0.0, 4.0};
    zoneSurface.Vertex(2) = {10.0, 10.0, 4.0};
    zoneSurface.Vertex(3) = {0.0, 10.0, 4.0};
    zoneSurface.Vertex(4) = {0.0, 0.0, 4.0};
    zoneSurface.Area = 100.0;
    zoneSurface.NetAreaShadowCalc = 100.0;

    auto &shadingSurface = state->dataSurface->Surface(shadingSurfaceIndex);
    shadingSurface.Name = "SHADING";
    shadingSurface.Class = DataSurfaces::SurfaceClass::Detached_B;
    shadingSurface.HeatTransSurf = false;
    shadingSurface.ExtSolar = true;
    shadingSurface.IsShadowing = true;
    shadingSurface.OutNormVec = {0.0, 0.0, 1.0};
    shadingSurface.Sides = 4;
    shadingSurface.Vertex.allocate(shadingSurface.Sides);
    shadingSurface.Vertex(1) = {110.0, 0.0, 4.0};
    shadingSurface.Vertex(2) = {110.0, 10.0, 4.0};
    shadingSurface.Vertex(3) = {100.0, 10.0, 4.0};
    shadingSurface.Vertex(4) = {100.0, 0.0, 4.0};
    shadingSurface.Area = 100.0;
    shadingSurface.NetAreaShadowCalc = 100.0;

    Window::initWindowModel(*state);
    SolarShading::AllocateModuleArrays(*state);

    // Prefill with some values
    state->dataHeatBal->SurfSunlitFrac = 0.5;
    state->dataHeatBal->SurfSunlitFracHR = 0.5;
    // Test non-integrated option first, CalcPerSolarBeam should set OutProjSLFracMult and InOutProjSLFracMult to 1.0 for all hours
    for (int SurfNum = 1; SurfNum <= state->dataSurface->TotSurfaces; ++SurfNum) {
        for (int Hour = 1; Hour <= Constant::iHoursInDay; ++Hour) {
            state->dataSurface->SurfaceWindow(SurfNum).OutProjSLFracMult[Hour] = 999.0;
            state->dataSurface->SurfaceWindow(SurfNum).InOutProjSLFracMult[Hour] = 888.0;
        }
    }

    SolarShading::DetermineShadowingCombinations(*state);

    state->dataSysVars->DetailedSolarTimestepIntegration = false;
    CalcPerSolarBeam(*state, AvgEqOfTime, AvgSinSolarDeclin, AvgCosSolarDeclin);

    int constexpr sunRiseHour = 8;
    int constexpr sunRiseTimeStep = 1;
    int constexpr sunSetHour = 17;
    int constexpr sunSetTimeStep = 1;

    auto isSunUp = [=](int hour, int timestep) {
        if ((hour < sunRiseHour || (hour == sunRiseHour && timestep < sunRiseTimeStep))) {
            return false;
        }
        if (hour > sunSetHour || (hour == sunSetHour && timestep > sunSetTimeStep)) {
            return false;
        }

        return true;
    };

    // Ensure we have sun is Up working properly
    for (int hour = 1; hour <= Constant::iHoursInDay; ++hour) {
        for (int timestep = 1; timestep <= state->dataGlobal->TimeStepsInHour; ++timestep) {
            if (isSunUp(hour, timestep)) {
                EXPECT_GT(state->dataBSDFWindow->SUNCOSTS(timestep, hour).z, 0.0);
            } else {
                EXPECT_LT(state->dataBSDFWindow->SUNCOSTS(timestep, hour).z, 0.0);
            }
        }
    }

    // Check that the fraction Sunlit is properly set to 1.0 when the sun is Up and ZERO otherwis: it does NOT remember previously set values
    for (int surfNum = 1; surfNum <= state->dataSurface->TotSurfaces; ++surfNum) {
        SCOPED_TRACE(fmt::format("Surface {}='{}'", surfNum, state->dataSurface->Surface(surfNum).Name));
        for (int hour = 1; hour <= Constant::iHoursInDay; ++hour) {
            SCOPED_TRACE(fmt::format("Hour={}", hour));
            if (isSunUp(hour, state->dataGlobal->TimeStepsInHour)) {
                EXPECT_DOUBLE_EQ(1.0, state->dataHeatBal->SurfSunlitFracHR(hour, surfNum)) << "Failed when Sun is Up";
            } else {
                EXPECT_DOUBLE_EQ(0.0, state->dataHeatBal->SurfSunlitFracHR(hour, surfNum)) << "Failed when Sun is Down";
            }
            for (int timestep = 1; timestep <= state->dataGlobal->TimeStepsInHour; ++timestep) {
                SCOPED_TRACE(fmt::format("Timestep={}", timestep));
                if (isSunUp(hour, timestep)) {
                    EXPECT_DOUBLE_EQ(1.0, state->dataHeatBal->SurfSunlitFrac(hour, timestep, surfNum)) << "Failed when Sun is Up";
                } else {
                    EXPECT_DOUBLE_EQ(0.0, state->dataHeatBal->SurfSunlitFrac(hour, timestep, surfNum)) << "Failed when Sun is Down";
                }
            }

            EXPECT_EQ(1.0, state->dataSurface->SurfaceWindow(surfNum).OutProjSLFracMult[hour]);
            EXPECT_EQ(1.0, state->dataSurface->SurfaceWindow(surfNum).InOutProjSLFracMult[hour]);
        }
    }

    // Test integrated option, CalcPerSolarBeam should set OutProjSLFracMult and InOutProjSLFracMult to 1.0 only for the specified hour
    // Re-initialize to new values
    for (int surfNum = 1; surfNum <= state->dataSurface->TotSurfaces; ++surfNum) {
        for (int hour = 1; hour <= Constant::iHoursInDay; ++hour) {
            state->dataSurface->SurfaceWindow(surfNum).OutProjSLFracMult[hour] = 555.0;
            state->dataSurface->SurfaceWindow(surfNum).InOutProjSLFracMult[hour] = 444.0;
        }
    }
    state->dataHeatBal->SurfSunlitFrac = 0.5;
    state->dataHeatBal->SurfSunlitFracHR = 0.5;

    state->dataSysVars->DetailedSolarTimestepIntegration = true;
    state->dataGlobal->HourOfDay = 23;
    state->dataGlobal->TimeStep = 3;
    CalcPerSolarBeam(*state, AvgEqOfTime, AvgSinSolarDeclin, AvgCosSolarDeclin);

    for (int surfNum = 1; surfNum <= state->dataSurface->TotSurfaces; ++surfNum) {
        SCOPED_TRACE(fmt::format("Surface {}='{}'", surfNum, state->dataSurface->Surface(surfNum).Name));
        for (int hour = 1; hour <= Constant::iHoursInDay; ++hour) {
            SCOPED_TRACE(fmt::format("Hour={}", hour));
            if (hour == state->dataGlobal->HourOfDay) {
                EXPECT_EQ(1.0, state->dataSurface->SurfaceWindow(surfNum).OutProjSLFracMult[hour]);
                EXPECT_EQ(1.0, state->dataSurface->SurfaceWindow(surfNum).InOutProjSLFracMult[hour]);
                EXPECT_DOUBLE_EQ(0.0, state->dataHeatBal->SurfSunlitFracHR(hour, surfNum));
            } else {
                EXPECT_EQ(555.0, state->dataSurface->SurfaceWindow(surfNum).OutProjSLFracMult[hour]);
                EXPECT_EQ(444.0, state->dataSurface->SurfaceWindow(surfNum).InOutProjSLFracMult[hour]);
                EXPECT_DOUBLE_EQ(0.5, state->dataHeatBal->SurfSunlitFracHR(hour, surfNum));
            }
            for (int timestep = 1; timestep <= state->dataGlobal->TimeStepsInHour; ++timestep) {
                SCOPED_TRACE(fmt::format("Timestep={}", timestep));
                if (hour == state->dataGlobal->HourOfDay && timestep == state->dataGlobal->TimeStep) {
                    EXPECT_DOUBLE_EQ(0.0, state->dataHeatBal->SurfSunlitFrac(hour, timestep, surfNum));
                } else {
                    EXPECT_DOUBLE_EQ(0.5, state->dataHeatBal->SurfSunlitFrac(hour, timestep, surfNum));
                }
            }
        }
    }

    // Clean up (Why is this needed?!)
    state->dataSurface->SurfaceWindow.deallocate();
    state->dataHeatBal->SurfSunlitFracHR.deallocate();
    state->dataHeatBal->SurfSunlitFrac.deallocate();
    state->dataHeatBal->SurfSunlitFracWithoutReveal.deallocate();
    state->dataSolarShading->SurfSunCosTheta.deallocate();
    state->dataHeatBal->SurfCosIncAngHR.deallocate();
    state->dataHeatBal->SurfCosIncAng.deallocate();
    state->dataSurface->SurfOpaqAO.deallocate();
    state->dataHeatBal->SurfWinBackSurfaces.deallocate();
    state->dataHeatBal->SurfWinOverlapAreas.deallocate();
}

TEST_F(EnergyPlusFixture, SolarShadingTest_SurfaceScheduledSolarInc)
{
    int SurfSolIncPtr;
    state->dataSurface->TotSurfIncSolSSG = 4;
    state->dataSurface->SurfIncSolSSG.allocate(state->dataSurface->TotSurfIncSolSSG);
    state->dataSurface->SurfIncSolSSG(1).SurfPtr = 1;
    state->dataSurface->SurfIncSolSSG(1).ConstrPtr = 1;
    state->dataSurface->SurfIncSolSSG(2).SurfPtr = 1;
    state->dataSurface->SurfIncSolSSG(2).ConstrPtr = 2;
    state->dataSurface->SurfIncSolSSG(3).SurfPtr = 4;
    state->dataSurface->SurfIncSolSSG(3).ConstrPtr = 10;
    state->dataSurface->SurfIncSolSSG(4).SurfPtr = 5;
    state->dataSurface->SurfIncSolSSG(4).ConstrPtr = 1;

    // Test retrieving pointer for surface incident solar schedule

    SurfSolIncPtr = -99;
    SurfSolIncPtr = SurfaceScheduledSolarInc(*state, 1, 1);
    EXPECT_EQ(1, SurfSolIncPtr);

    SurfSolIncPtr = -99;
    SurfSolIncPtr = SurfaceScheduledSolarInc(*state, 1, 2);
    EXPECT_EQ(2, SurfSolIncPtr);

    SurfSolIncPtr = -99;
    SurfSolIncPtr = SurfaceScheduledSolarInc(*state, 1, 3);
    EXPECT_EQ(0, SurfSolIncPtr);

    SurfSolIncPtr = -99;
    SurfSolIncPtr = SurfaceScheduledSolarInc(*state, 5, 1);
    EXPECT_EQ(4, SurfSolIncPtr);

    SurfSolIncPtr = -99;
    SurfSolIncPtr = SurfaceScheduledSolarInc(*state, 5, 10);
    EXPECT_EQ(0, SurfSolIncPtr);

    state->dataSurface->SurfIncSolSSG.deallocate();
}

TEST_F(EnergyPlusFixture, SolarShadingTest_polygon_contains_point)
{
    unsigned int numSides = 4;
    Array1D<Vector> Rectangle3d;

    Rectangle3d.allocate(numSides);

    Rectangle3d(1).x = 0.;
    Rectangle3d(1).y = 0.;
    Rectangle3d(1).z = 0.;

    Rectangle3d(2).x = 10.;
    Rectangle3d(2).y = 0.;
    Rectangle3d(2).z = 0.;

    Rectangle3d(3).x = 10.;
    Rectangle3d(3).y = 10.;
    Rectangle3d(3).z = 0.;

    Rectangle3d(4).x = 0.;
    Rectangle3d(4).y = 10.;
    Rectangle3d(4).z = 0.;

    Vector PointInside;

    PointInside.x = 5.;
    PointInside.y = 5.;
    PointInside.z = 0.;

    Vector PointOutside;

    PointOutside.x = 20.;
    PointOutside.y = 20.;
    PointOutside.z = 0.;

    EXPECT_TRUE(polygon_contains_point(numSides, Rectangle3d, PointInside, false, false, true));
    EXPECT_FALSE(polygon_contains_point(numSides, Rectangle3d, PointOutside, false, false, true));

    Rectangle3d(1).x = 0.;
    Rectangle3d(1).y = 0.;
    Rectangle3d(1).z = 0.;

    Rectangle3d(2).x = 10.;
    Rectangle3d(2).y = 0.;
    Rectangle3d(2).z = 0.;

    Rectangle3d(3).x = 10.;
    Rectangle3d(3).y = 0.;
    Rectangle3d(3).z = 10.;

    Rectangle3d(4).x = 0.;
    Rectangle3d(4).y = 0.;
    Rectangle3d(4).z = 10.;

    PointInside.x = 5.;
    PointInside.y = 0.;
    PointInside.z = 5.;

    PointOutside.x = 20.;
    PointOutside.y = 0.;
    PointOutside.z = 20.;

    EXPECT_TRUE(polygon_contains_point(numSides, Rectangle3d, PointInside, false, true, false));
    EXPECT_FALSE(polygon_contains_point(numSides, Rectangle3d, PointOutside, false, true, false));

    Rectangle3d(1).x = 0.;
    Rectangle3d(1).y = 0.;
    Rectangle3d(1).z = 0.;

    Rectangle3d(2).x = 0.;
    Rectangle3d(2).y = 10.;
    Rectangle3d(2).z = 0.;

    Rectangle3d(3).x = 0.;
    Rectangle3d(3).y = 10.;
    Rectangle3d(3).z = 10.;

    Rectangle3d(4).x = 0.;
    Rectangle3d(4).y = 0.;
    Rectangle3d(4).z = 10.;

    PointInside.x = 0.;
    PointInside.y = 5.;
    PointInside.z = 5.;

    PointOutside.x = 0.;
    PointOutside.y = 20.;
    PointOutside.z = 20.;

    EXPECT_TRUE(polygon_contains_point(numSides, Rectangle3d, PointInside, true, false, false));
    EXPECT_FALSE(polygon_contains_point(numSides, Rectangle3d, PointOutside, true, false, false));
}

TEST_F(EnergyPlusFixture, SolarShadingTest_FigureSolarBeamAtTimestep)
{
    std::string const idf_objects = delimited_string({
        "  Building,",
        "    DemoFDT,                 !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    3.9999999E-02,           !- Loads Convergence Tolerance Value",
        "    4.0000002E-03,           !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullExterior,            !- Solar Distribution",
        "    ,                        !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",
        "  ShadowCalculation,",
        "    PolygonClipping,         !- Shading Calculation Method",
        "    Timestep,                !- Shading Calculation Update Frequency Method",
        "    ,                        !- Shading Calculation Update Frequency",
        "    ,                        !- Maximum Figures in Shadow Overlap Calculations",
        "    ,                        !- Polygon Clipping Algorithm",
        "    ,                        !- Pixel Counting Resolution",
        "    DetailedSkyDiffuseModeling;  !- Sky Diffuse Modeling Algorithm",
        "  SurfaceConvectionAlgorithm:Inside,TARP;",
        "  SurfaceConvectionAlgorithm:Outside,TARP;",
        "  HeatBalanceAlgorithm,ConductionTransferFunction;",
        "  Timestep,6;",
        "  RunPeriod,",
        "    RP1,                     !- Name",
        "    1,                       !- Begin Month",
        "    1,                       !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    12,                      !- End Month",
        "    31,                      !- End Day of Month",
        "    ,                        !- End Year",
        "    ,                        !- Day of Week for Start Day",
        "    ,                        !- Use Weather File Holidays and Special Days",
        "    ,                        !- Use Weather File Daylight Saving Period",
        "    ,                        !- Apply Weekend Holiday Rule",
        "    ,                        !- Use Weather File Rain Indicators",
        "    ;                        !- Use Weather File Snow Indicators",
        "  ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    Continuous;              !- Numeric Type",
        "  ScheduleTypeLimits,",
        "    ON/OFF,                  !- Name",
        "    0,                       !- Lower Limit Value",
        "    1,                       !- Upper Limit Value",
        "    Discrete;                !- Numeric Type",
        "  Schedule:Compact,",
        "    SunShading,              !- Name",
        "    ON/OFF,                  !- Schedule Type Limits Name",
        "    Through: 4/30,           !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    until: 24:00,1,          !- Field 3",
        "    Through: 10/31,          !- Field 5",
        "    For: AllDays,            !- Field 6",
        "    until: 24:00,0,          !- Field 7",
        "    Through: 12/31,          !- Field 9",
        "    For: AllDays,            !- Field 10",
        "    until: 24:00,1;          !- Field 11",
        "  Material,",
        "    A2 - 4 IN DENSE FACE BRICK,  !- Name",
        "    Rough,                   !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    1.245296,                !- Conductivity {W/m-K}",
        "    2082.400,                !- Density {kg/m3}",
        "    920.4800,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9300000,               !- Solar Absorptance",
        "    0.9300000;               !- Visible Absorptance",
        "  Material,",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD,  !- Name",
        "    Smooth,                  !- Roughness",
        "    1.9050000E-02,           !- Thickness {m}",
        "    0.7264224,               !- Conductivity {W/m-K}",
        "    1601.846,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",
        "  Material,",
        "    E2 - 1 / 2 IN SLAG OR STONE,  !- Name",
        "    Rough,                   !- Roughness",
        "    1.2710161E-02,           !- Thickness {m}",
        "    1.435549,                !- Conductivity {W/m-K}",
        "    881.0155,                !- Density {kg/m3}",
        "    1673.600,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.5500000,               !- Solar Absorptance",
        "    0.5500000;               !- Visible Absorptance",
        "  Material,",
        "    C12 - 2 IN HW CONCRETE,  !- Name",
        "    MediumRough,             !- Roughness",
        "    5.0901599E-02,           !- Thickness {m}",
        "    1.729577,                !- Conductivity {W/m-K}",
        "    2242.585,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",
        "  Material:NoMass,",
        "    R13LAYER,                !- Name",
        "    Rough,                   !- Roughness",
        "    2.290965,                !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",
        "  WindowMaterial:Glazing,",
        "    GLASS - CLEAR PLATE 1 / 4 IN,  !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.006,                   !- Thickness {m}",
        "    0.80,                    !- Solar Transmittance at Normal Incidence",
        "    0.10,                    !- Front Side Solar Reflectance at Normal Incidence",
        "    0.10,                    !- Back Side Solar Reflectance at Normal Incidence",
        "    0.80,                    !- Visible Transmittance at Normal Incidence",
        "    0.10,                    !- Front Side Visible Reflectance at Normal Incidence",
        "    0.10,                    !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.84,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",
        "  WindowMaterial:Gas,",
        "    AIRGAP,                  !- Name",
        "    AIR,                     !- Gas Type",
        "    0.0125;                  !- Thickness {m}",
        "  Construction,",
        "    R13WALL,                 !- Name",
        "    R13LAYER;                !- Outside Layer",
        "  Construction,",
        "    EXTWALL09,               !- Name",
        "    A2 - 4 IN DENSE FACE BRICK,  !- Outside Layer",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 4",
        "  Construction,",
        "    INTERIOR,                !- Name",
        "    C12 - 2 IN HW CONCRETE;  !- Layer 4",
        "  Construction,",
        "    SLAB FLOOR,              !- Name",
        "    C12 - 2 IN HW CONCRETE;  !- Layer 4",
        "  Construction,",
        "    ROOF31,                  !- Name",
        "    E2 - 1 / 2 IN SLAG OR STONE,  !- Outside Layer",
        "    C12 - 2 IN HW CONCRETE;  !- Layer 4",
        "  Construction,",
        "    DOUBLE PANE HW WINDOW,   !- Name",
        "    GLASS - CLEAR PLATE 1 / 4 IN,  !- Outside Layer",
        "    AIRGAP,                  !- Layer 2",
        "    GLASS - CLEAR PLATE 1 / 4 IN;  !- Layer 3",
        "  Construction,",
        "    PARTITION02,             !- Name",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD,  !- Outside Layer",
        "    C12 - 2 IN HW CONCRETE,  !- Layer 4",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 3",
        "  Construction,",
        "    single PANE HW WINDOW,   !- Name",
        "    GLASS - CLEAR PLATE 1 / 4 IN;  !- Outside Layer",
        "  Construction,",
        "    EXTWALLdemo,             !- Name",
        "    A2 - 4 IN DENSE FACE BRICK,  !- Outside Layer",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 4",
        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    Counterclockwise,        !- Vertex Entry Direction",
        "    Relative;                !- Coordinate System",
        "  Zone,",
        "    ZONE ONE,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    0,                       !- Ceiling Height {m}",
        "    0;                       !- Volume {m3}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-North,        !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALLdemo,             !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    5,5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    -5,5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-East,         !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL09,               !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    5,-5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    5,5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-South,        !- Name",
        "    Wall,                    !- Surface Type",
        "    R13WALL,                 !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,-5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -5,-5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    5,-5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-West,         !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL09,               !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -5,5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    -5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,-5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:roof,              !- Name",
        "    Roof,                    !- Surface Type",
        "    ROOF31,                  !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.0000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,-5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,-5,3,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,5,3,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:floor,             !- Name",
        "    Floor,                   !- Surface Type",
        "    SLAB FLOOR,              !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.0000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,-5,0;  !- X,Y,Z ==> Vertex 4 {m}",
        "  FenestrationSurface:Detailed,",
        "    Zn001:Wall-South:Win001, !- Name",
        "    Window,                  !- Surface Type",
        "    DOUBLE PANE HW WINDOW,   !- Construction Name",
        "    Zn001:Wall-South,        !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    TestFrameAndDivider,     !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-5,0.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-5,0.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;  !- X,Y,Z ==> Vertex 4 {m}",
        "  WindowProperty:FrameAndDivider,",
        "    TestFrameAndDivider,     !- Name",
        "    0.05,                    !- Frame Width {m}",
        "    0.05,                    !- Frame Outside Projection {m}",
        "    0.05,                    !- Frame Inside Projection {m}",
        "    5.0,                     !- Frame Conductance {W/m2-K}",
        "    1.2,                     !- Ratio of Frame-Edge Glass Conductance to Center-Of-Gl",
        "    0.8,                     !- Frame Solar Absorptance",
        "    0.8,                     !- Frame Visible Absorptance",
        "    0.9,                     !- Frame Thermal Hemispherical Emissivity",
        "    DividedLite,             !- Divider Type",
        "    0.02,                    !- Divider Width {m}",
        "    2,                       !- Number of Horizontal Dividers",
        "    2,                       !- Number of Vertical Dividers",
        "    0.02,                    !- Divider Outside Projection {m}",
        "    0.02,                    !- Divider Inside Projection {m}",
        "    5.0,                     !- Divider Conductance {W/m2-K}",
        "    1.2,                     !- Ratio of Divider-Edge Glass Conductance to Center-Of-",
        "    0.8,                     !- Divider Solar Absorptance",
        "    0.8,                     !- Divider Visible Absorptance",
        "    0.9;                     !- Divider Thermal Hemispherical Emissivity",
        "  Shading:Zone:Detailed,",
        "    Zn001:Wall-South:Shade001,  !- Name",
        "    Zn001:Wall-South,        !- Base Surface Name",
        "    SunShading,              !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-6,2.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-6,2.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;  !- X,Y,Z ==> Vertex 4 {m}",
        "  ShadingProperty:Reflectance,",
        "    Zn001:Wall-South:Shade001,  !- Shading Surface Name",
        "    0.2,                     !- Diffuse Solar Reflectance of Unglazed Part of Shading",
        "    0.2;                     !- Diffuse Visible Reflectance of Unglazed Part of Shading",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool FoundError = false;

    HeatBalanceManager::GetProjectControlData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    HeatBalanceManager::SetPreConstructionInputParameters(*state);

    Material::GetMaterialData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    HeatBalanceManager::GetFrameAndDividerData(*state);

    HeatBalanceManager::GetConstructData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    HeatBalanceManager::GetZoneData(*state, FoundError); // Read Zone data from input file
    EXPECT_FALSE(FoundError);

    SurfaceGeometry::GetGeometryParameters(*state, FoundError);
    EXPECT_FALSE(FoundError);

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(1);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(1);

    state->dataSurfaceGeometry->CosZoneRelNorth(1) = std::cos(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(1) = std::sin(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;

    SurfaceGeometry::GetSurfaceData(*state, FoundError); // setup zone geometry and get zone data
    EXPECT_FALSE(FoundError);                            // expect no errors

    //	compare_err_stream( "" ); // just for debugging

    SurfaceGeometry::SetupZoneGeometry(*state, FoundError); // this calls GetSurfaceData()
    EXPECT_FALSE(FoundError);

    SolarShading::AllocateModuleArrays(*state);
    SolarShading::DetermineShadowingCombinations(*state);
    state->dataEnvrn->DayOfYear_Schedule = 168;
    state->dataEnvrn->DayOfWeek = 6;
    state->dataGlobal->TimeStep = 4;
    state->dataGlobal->HourOfDay = 9;

    //	compare_err_stream( "" ); // just for debugging

    state->dataSurface->ShadingTransmittanceVaries = true;
    state->dataSysVars->DetailedSkyDiffuseAlgorithm = true;
    state->dataHeatBal->SolarDistribution = DataHeatBalance::Shadowing::FullExterior;

    state->dataSolarShading->CalcSkyDifShading = true;
    SolarShading::InitSolarCalculations(*state);
    SolarShading::SkyDifSolarShading(*state);
    state->dataSolarShading->CalcSkyDifShading = false;

    state->dataBSDFWindow->SUNCOSTS(4, 9)(1) = 0.1;
    state->dataBSDFWindow->SUNCOSTS(4, 9)(2) = 0.1;
    state->dataBSDFWindow->SUNCOSTS(4, 9)(3) = 0.1;
    FigureSolarBeamAtTimestep(*state, state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep);

    int windowSurfNum = Util::FindItemInList("ZN001:WALL-SOUTH:WIN001", state->dataSurface->Surface);
    EXPECT_NEAR(0.6504, state->dataSolarShading->SurfDifShdgRatioIsoSkyHRTS(4, 9, windowSurfNum), 0.0001);
    EXPECT_NEAR(0.9152, state->dataSolarShading->SurfDifShdgRatioHorizHRTS(4, 9, windowSurfNum), 0.0001);
}

TEST_F(EnergyPlusFixture, SolarShadingTest_ExternalShadingIO)
{
    std::string const idf_objects = delimited_string({

        "  Building,",
        "    DemoFDT,                 !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    3.9999999E-02,           !- Loads Convergence Tolerance Value",
        "    4.0000002E-03,           !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullExterior,            !- Solar Distribution",
        "    ,                        !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",
        "  ShadowCalculation,",
        "    Scheduled,               !- Shading Calculation Method",
        "    Timestep,                !- Shading Calculation Update Frequency Method",
        "    ,                        !- Shading Calculation Update Frequency",
        "    ,                           !- Maximum Figures in Shadow Overlap Calculations",
        "    ,                           !- Polygon Clipping Algorithm",
        "    ,                        !- Pixel Counting Resolution",
        "    DetailedSkyDiffuseModeling, !- Sky Diffuse Modeling Algorithm",
        "    Yes;                        !- Output External Shading Calculation Results",
        "  SurfaceConvectionAlgorithm:Inside,TARP;",
        "  SurfaceConvectionAlgorithm:Outside,TARP;",
        "  HeatBalanceAlgorithm,ConductionTransferFunction;",
        "  Timestep,6;",
        "  RunPeriod,",
        "    RP1,                     !- Name",
        "    1,                       !- Begin Month",
        "    1,                       !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    12,                      !- End Month",
        "    31,                      !- End Day of Month",
        "    ,                        !- End Year",
        "    ,                        !- Day of Week for Start Day",
        "    ,                        !- Use Weather File Holidays and Special Days",
        "    ,                        !- Use Weather File Daylight Saving Period",
        "    ,                        !- Apply Weekend Holiday Rule",
        "    ,                        !- Use Weather File Rain Indicators",
        "    ;                        !- Use Weather File Snow Indicators",
        "  ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    Continuous;              !- Numeric Type",
        "  ScheduleTypeLimits,",
        "    ON/OFF,                  !- Name",
        "    0,                       !- Lower Limit Value",
        "    1,                       !- Upper Limit Value",
        "    Discrete;                !- Numeric Type",
        "  Schedule:Compact,",
        "    SunShading,              !- Name",
        "    ON/OFF,                  !- Schedule Type Limits Name",
        "    Through: 4/30,           !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    until: 24:00,1,          !- Field 3",
        "    Through: 10/31,          !- Field 5",
        "    For: AllDays,            !- Field 6",
        "    until: 24:00,0,          !- Field 7",
        "    Through: 12/31,          !- Field 9",
        "    For: AllDays,            !- Field 10",
        "    until: 24:00,1;          !- Field 11",
        "  Material,",
        "    A2 - 4 IN DENSE FACE BRICK,  !- Name",
        "    Rough,                   !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    1.245296,                !- Conductivity {W/m-K}",
        "    2082.400,                !- Density {kg/m3}",
        "    920.4800,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9300000,               !- Solar Absorptance",
        "    0.9300000;               !- Visible Absorptance",
        "  Material,",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD,  !- Name",
        "    Smooth,                  !- Roughness",
        "    1.9050000E-02,           !- Thickness {m}",
        "    0.7264224,               !- Conductivity {W/m-K}",
        "    1601.846,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",
        "  Material,",
        "    E2 - 1 / 2 IN SLAG OR STONE,  !- Name",
        "    Rough,                   !- Roughness",
        "    1.2710161E-02,           !- Thickness {m}",
        "    1.435549,                !- Conductivity {W/m-K}",
        "    881.0155,                !- Density {kg/m3}",
        "    1673.600,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.5500000,               !- Solar Absorptance",
        "    0.5500000;               !- Visible Absorptance",
        "  Material,",
        "    C12 - 2 IN HW CONCRETE,  !- Name",
        "    MediumRough,             !- Roughness",
        "    5.0901599E-02,           !- Thickness {m}",
        "    1.729577,                !- Conductivity {W/m-K}",
        "    2242.585,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",
        "  Material:NoMass,",
        "    R13LAYER,                !- Name",
        "    Rough,                   !- Roughness",
        "    2.290965,                !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",
        "  WindowMaterial:Glazing,",
        "    GLASS - CLEAR PLATE 1 / 4 IN,  !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.006,                   !- Thickness {m}",
        "    0.80,                    !- Solar Transmittance at Normal Incidence",
        "    0.10,                    !- Front Side Solar Reflectance at Normal Incidence",
        "    0.10,                    !- Back Side Solar Reflectance at Normal Incidence",
        "    0.80,                    !- Visible Transmittance at Normal Incidence",
        "    0.10,                    !- Front Side Visible Reflectance at Normal Incidence",
        "    0.10,                    !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.84,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",
        "  WindowMaterial:Gas,",
        "    AIRGAP,                  !- Name",
        "    AIR,                     !- Gas Type",
        "    0.0125;                  !- Thickness {m}",
        "  Construction,",
        "    R13WALL,                 !- Name",
        "    R13LAYER;                !- Outside Layer",
        "  Construction,",
        "    EXTWALL09,               !- Name",
        "    A2 - 4 IN DENSE FACE BRICK,  !- Outside Layer",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 4",
        "  Construction,",
        "    INTERIOR,                !- Name",
        "    C12 - 2 IN HW CONCRETE;  !- Layer 4",
        "  Construction,",
        "    SLAB FLOOR,              !- Name",
        "    C12 - 2 IN HW CONCRETE;  !- Layer 4",
        "  Construction,",
        "    ROOF31,                  !- Name",
        "    E2 - 1 / 2 IN SLAG OR STONE,  !- Outside Layer",
        "    C12 - 2 IN HW CONCRETE;  !- Layer 4",
        "  Construction,",
        "    DOUBLE PANE HW WINDOW,   !- Name",
        "    GLASS - CLEAR PLATE 1 / 4 IN,  !- Outside Layer",
        "    AIRGAP,                  !- Layer 2",
        "    GLASS - CLEAR PLATE 1 / 4 IN;  !- Layer 3",
        "  Construction,",
        "    PARTITION02,             !- Name",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD,  !- Outside Layer",
        "    C12 - 2 IN HW CONCRETE,  !- Layer 4",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 3",
        "  Construction,",
        "    single PANE HW WINDOW,   !- Name",
        "    GLASS - CLEAR PLATE 1 / 4 IN;  !- Outside Layer",
        "  Construction,",
        "    EXTWALLdemo,             !- Name",
        "    A2 - 4 IN DENSE FACE BRICK,  !- Outside Layer",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 4",
        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    Counterclockwise,        !- Vertex Entry Direction",
        "    Relative;                !- Coordinate System",
        "  Zone,",
        "    ZONE ONE,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    0,                       !- Ceiling Height {m}",
        "    0;                       !- Volume {m3}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-North,        !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALLdemo,             !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    5,5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    -5,5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-East,         !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL09,               !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    5,-5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    5,5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-South,        !- Name",
        "    Wall,                    !- Surface Type",
        "    R13WALL,                 !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,-5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -5,-5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    5,-5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-West,         !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL09,               !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -5,5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    -5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,-5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:roof,              !- Name",
        "    Roof,                    !- Surface Type",
        "    ROOF31,                  !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.0000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,-5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,-5,3,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,5,3,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  SurfaceProperty:LocalEnvironment,",
        "    LocEnv:Zn001:roof,",
        "    Zn001:roof,",
        "    ExtShadingSch:Zn001:roof,",
        "    ,",
        "    ;",
        "  Schedule:Compact,",
        "    ExtShadingSch:Zn001:roof,",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    0.5432;                  !- Field 4",
        "  BuildingSurface:Detailed,",
        "    Zn001:floor,             !- Name",
        "    Floor,                   !- Surface Type",
        "    SLAB FLOOR,              !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.0000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,-5,0;  !- X,Y,Z ==> Vertex 4 {m}",
        "  FenestrationSurface:Detailed,",
        "    Zn001:Wall-South:Win001, !- Name",
        "    Window,                  !- Surface Type",
        "    DOUBLE PANE HW WINDOW,   !- Construction Name",
        "    Zn001:Wall-South,        !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    TestFrameAndDivider,     !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-5,0.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-5,0.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;  !- X,Y,Z ==> Vertex 4 {m}",
        "  WindowProperty:FrameAndDivider,",
        "    TestFrameAndDivider,     !- Name",
        "    0.05,                    !- Frame Width {m}",
        "    0.05,                    !- Frame Outside Projection {m}",
        "    0.05,                    !- Frame Inside Projection {m}",
        "    5.0,                     !- Frame Conductance {W/m2-K}",
        "    1.2,                     !- Ratio of Frame-Edge Glass Conductance to Center-Of-Gl",
        "    0.8,                     !- Frame Solar Absorptance",
        "    0.8,                     !- Frame Visible Absorptance",
        "    0.9,                     !- Frame Thermal Hemispherical Emissivity",
        "    DividedLite,             !- Divider Type",
        "    0.02,                    !- Divider Width {m}",
        "    2,                       !- Number of Horizontal Dividers",
        "    2,                       !- Number of Vertical Dividers",
        "    0.02,                    !- Divider Outside Projection {m}",
        "    0.02,                    !- Divider Inside Projection {m}",
        "    5.0,                     !- Divider Conductance {W/m2-K}",
        "    1.2,                     !- Ratio of Divider-Edge Glass Conductance to Center-Of-",
        "    0.8,                     !- Divider Solar Absorptance",
        "    0.8,                     !- Divider Visible Absorptance",
        "    0.9;                     !- Divider Thermal Hemispherical Emissivity",
        "  Shading:Zone:Detailed,",
        "    Zn001:Wall-South:Shade001,  !- Name",
        "    Zn001:Wall-South,        !- Base Surface Name",
        "    SunShading,              !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-6,2.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-6,2.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;  !- X,Y,Z ==> Vertex 4 {m}",
        "  ShadingProperty:Reflectance,",
        "    Zn001:Wall-South:Shade001,  !- Shading Surface Name",
        "    0.2,                     !- Diffuse Solar Reflectance of Unglazed Part of Shading",
        "    0.2;                     !- Diffuse Visible Reflectance of Unglazed Part of Shading",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool FoundError = false;

    HeatBalanceManager::GetProjectControlData(*state, FoundError); // read project control data
    EXPECT_FALSE(FoundError);                                      // expect no errors

    HeatBalanceManager::SetPreConstructionInputParameters(*state);

    Material::GetMaterialData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    HeatBalanceManager::GetFrameAndDividerData(*state);

    HeatBalanceManager::GetConstructData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    HeatBalanceManager::GetZoneData(*state, FoundError); // Read Zone data from input file
    EXPECT_FALSE(FoundError);

    SurfaceGeometry::GetGeometryParameters(*state, FoundError);
    EXPECT_FALSE(FoundError);

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(1);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(1);

    state->dataSurfaceGeometry->CosZoneRelNorth(1) = std::cos(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(1) = std::sin(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;

    compare_err_stream("");                                 // just for debugging
    SurfaceGeometry::SetupZoneGeometry(*state, FoundError); // this calls GetSurfaceData()
    EXPECT_FALSE(FoundError);

    SolarShading::AllocateModuleArrays(*state);
    SolarShading::DetermineShadowingCombinations(*state);
    state->dataEnvrn->DayOfYear_Schedule = 168;
    state->dataEnvrn->DayOfWeek = 6;
    state->dataGlobal->TimeStep = 4;
    state->dataGlobal->HourOfDay = 9;
    state->dataGlobal->DoingSizing = false;
    state->dataGlobal->KindOfSim = Constant::KindOfSim::RunPeriodWeather;

    compare_err_stream(""); // just for debugging

    state->dataSurface->ShadingTransmittanceVaries = true;
    state->dataSysVars->DetailedSkyDiffuseAlgorithm = true;
    state->dataSysVars->shadingMethod = DataSystemVariables::ShadingMethod::Scheduled;
    state->dataHeatBal->SolarDistribution = DataHeatBalance::Shadowing::FullExterior;

    state->dataSolarShading->CalcSkyDifShading = true;
    SolarShading::InitSolarCalculations(*state);
    SolarShading::SkyDifSolarShading(*state);
    state->dataSolarShading->CalcSkyDifShading = false;

    Sched::UpdateScheduleVals(*state);
    state->dataBSDFWindow->SUNCOSTS(4, 9)(1) = 0.1;
    state->dataBSDFWindow->SUNCOSTS(4, 9)(2) = 0.1;
    state->dataBSDFWindow->SUNCOSTS(4, 9)(3) = 0.1;
    FigureSolarBeamAtTimestep(*state, state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep);

    EXPECT_TRUE(state->dataSysVars->shadingMethod == DataSystemVariables::ShadingMethod::Scheduled);
    EXPECT_DOUBLE_EQ(0.5432, Sched::GetSchedule(*state, "EXTSHADINGSCH:ZN001:ROOF")->getHrTsVal(*state, 9, 4));
    EXPECT_FALSE(state->dataSolarShading->SUNCOS(3) < 0.00001);
    EXPECT_DOUBLE_EQ(0.00001, DataEnvironment::SunIsUpValue);

    EXPECT_FALSE(state->dataSolarShading->SUNCOS(3) < DataEnvironment::SunIsUpValue);

    int surfNum = Util::FindItemInList("ZN001:WALL-SOUTH", state->dataSurface->Surface);
    EXPECT_DOUBLE_EQ(1, state->dataHeatBal->SurfSunlitFrac(9, 4, surfNum));
    surfNum = Util::FindItemInList("ZN001:WALL-SOUTH:WIN001", state->dataSurface->Surface);
    EXPECT_DOUBLE_EQ(1, state->dataHeatBal->SurfSunlitFrac(9, 4, surfNum));
    surfNum = Util::FindItemInList("ZN001:ROOF", state->dataSurface->Surface);
    EXPECT_DOUBLE_EQ(0.5432, state->dataHeatBal->SurfSunlitFrac(9, 4, surfNum));
}

TEST_F(EnergyPlusFixture, SolarShadingTest_DisableGroupSelfShading)
{
    std::string const idf_objects = delimited_string({
        "  Building,",
        "    DemoFDT,                 !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    3.9999999E-02,           !- Loads Convergence Tolerance Value",
        "    4.0000002E-03,           !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullExterior,            !- Solar Distribution",
        "    ,                        !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",
        "  ShadowCalculation,",
        "    PolygonClipping,         !- Shading Calculation Method",
        "    Timestep,                !- Shading Calculation Update Frequency Method",
        "    ,                        !- Shading Calculation Update Frequency",
        "    ,                           !- Maximum Figures in Shadow Overlap Calculations",
        "    ,                           !- Polygon Clipping Algorithm",
        "    ,                           !- Pixel Counting Resolution",
        "    ,                           !- Sky Diffuse Modeling Algorithm",
        "    ,                           !- Output External Shading Calculation Results",
        "    Yes,                        !- Disable Shading within A Zone Group",
        "    ,                           !- Disable Shading between Zone Groups",
        "    ShadingZoneGroup;           !- Shading Group 1 ZoneList Name",
        "  SurfaceConvectionAlgorithm:Inside,TARP;",
        "  SurfaceConvectionAlgorithm:Outside,TARP;",
        "  HeatBalanceAlgorithm,ConductionTransferFunction;",
        "  Timestep,6;",
        "  RunPeriod,",
        "    RP1,                     !- Name",
        "    1,                       !- Begin Month",
        "    1,                       !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    12,                      !- End Month",
        "    31,                      !- End Day of Month",
        "    ,                        !- End Year",
        "    ,                        !- Day of Week for Start Day",
        "    ,                        !- Use Weather File Holidays and Special Days",
        "    ,                        !- Use Weather File Daylight Saving Period",
        "    ,                        !- Apply Weekend Holiday Rule",
        "    ,                        !- Use Weather File Rain Indicators",
        "    ;                        !- Use Weather File Snow Indicators",
        "  ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    Continuous;              !- Numeric Type",
        "  ScheduleTypeLimits,",
        "    ON/OFF,                  !- Name",
        "    0,                       !- Lower Limit Value",
        "    1,                       !- Upper Limit Value",
        "    Discrete;                !- Numeric Type",
        "  Schedule:Compact,",
        "    SunShading,              !- Name",
        "    ON/OFF,                  !- Schedule Type Limits Name",
        "    Through: 4/30,           !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    until: 24:00,1,          !- Field 3",
        "    Through: 10/31,          !- Field 5",
        "    For: AllDays,            !- Field 6",
        "    until: 24:00,0,          !- Field 7",
        "    Through: 12/31,          !- Field 9",
        "    For: AllDays,            !- Field 10",
        "    until: 24:00,1;          !- Field 11",
        "  Material,",
        "    A2 - 4 IN DENSE FACE BRICK,  !- Name",
        "    Rough,                   !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    1.245296,                !- Conductivity {W/m-K}",
        "    2082.400,                !- Density {kg/m3}",
        "    920.4800,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9300000,               !- Solar Absorptance",
        "    0.9300000;               !- Visible Absorptance",
        "  Material,",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD,  !- Name",
        "    Smooth,                  !- Roughness",
        "    1.9050000E-02,           !- Thickness {m}",
        "    0.7264224,               !- Conductivity {W/m-K}",
        "    1601.846,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",
        "  Material,",
        "    E2 - 1 / 2 IN SLAG OR STONE,  !- Name",
        "    Rough,                   !- Roughness",
        "    1.2710161E-02,           !- Thickness {m}",
        "    1.435549,                !- Conductivity {W/m-K}",
        "    881.0155,                !- Density {kg/m3}",
        "    1673.600,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.5500000,               !- Solar Absorptance",
        "    0.5500000;               !- Visible Absorptance",
        "  Material,",
        "    C12 - 2 IN HW CONCRETE,  !- Name",
        "    MediumRough,             !- Roughness",
        "    5.0901599E-02,           !- Thickness {m}",
        "    1.729577,                !- Conductivity {W/m-K}",
        "    2242.585,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",
        "  Material:NoMass,",
        "    R13LAYER,                !- Name",
        "    Rough,                   !- Roughness",
        "    2.290965,                !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",
        "  WindowMaterial:Glazing,",
        "    GLASS - CLEAR PLATE 1 / 4 IN,  !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.006,                   !- Thickness {m}",
        "    0.80,                    !- Solar Transmittance at Normal Incidence",
        "    0.10,                    !- Front Side Solar Reflectance at Normal Incidence",
        "    0.10,                    !- Back Side Solar Reflectance at Normal Incidence",
        "    0.80,                    !- Visible Transmittance at Normal Incidence",
        "    0.10,                    !- Front Side Visible Reflectance at Normal Incidence",
        "    0.10,                    !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.84,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",
        "  WindowMaterial:Gas,",
        "    AIRGAP,                  !- Name",
        "    AIR,                     !- Gas Type",
        "    0.0125;                  !- Thickness {m}",
        "  Construction,",
        "    R13WALL,                 !- Name",
        "    R13LAYER;                !- Outside Layer",
        "  Construction,",
        "    EXTWALL09,               !- Name",
        "    A2 - 4 IN DENSE FACE BRICK,  !- Outside Layer",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 4",
        "  Construction,",
        "    INTERIOR,                !- Name",
        "    C12 - 2 IN HW CONCRETE;  !- Layer 4",
        "  Construction,",
        "    SLAB FLOOR,              !- Name",
        "    C12 - 2 IN HW CONCRETE;  !- Layer 4",
        "  Construction,",
        "    ROOF31,                  !- Name",
        "    E2 - 1 / 2 IN SLAG OR STONE,  !- Outside Layer",
        "    C12 - 2 IN HW CONCRETE;  !- Layer 4",
        "  Construction,",
        "    DOUBLE PANE HW WINDOW,   !- Name",
        "    GLASS - CLEAR PLATE 1 / 4 IN,  !- Outside Layer",
        "    AIRGAP,                  !- Layer 2",
        "    GLASS - CLEAR PLATE 1 / 4 IN;  !- Layer 3",
        "  Construction,",
        "    PARTITION02,             !- Name",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD,  !- Outside Layer",
        "    C12 - 2 IN HW CONCRETE,  !- Layer 4",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 3",
        "  Construction,",
        "    single PANE HW WINDOW,   !- Name",
        "    GLASS - CLEAR PLATE 1 / 4 IN;  !- Outside Layer",
        "  Construction,",
        "    EXTWALLdemo,             !- Name",
        "    A2 - 4 IN DENSE FACE BRICK,  !- Outside Layer",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 4",
        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    Counterclockwise,        !- Vertex Entry Direction",
        "    Relative;                !- Coordinate System",
        "  Zone,",
        "    ZONE ONE,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    0,                       !- Ceiling Height {m}",
        "    0;                       !- Volume {m3}",
        "  ZoneList,",
        "    ShadingZoneGroup,",
        "    ZONE ONE;",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-North,        !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALLdemo,             !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    5,5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    -5,5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-East,         !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL09,               !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    5,-5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    5,5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-South,        !- Name",
        "    Wall,                    !- Surface Type",
        "    R13WALL,                 !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,-5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -5,-5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    5,-5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-West,         !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL09,               !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -5,5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    -5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,-5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:roof,              !- Name",
        "    Roof,                    !- Surface Type",
        "    ROOF31,                  !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.0000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,-5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,-5,3,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,5,3,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  Schedule:Compact,",
        "    ExtShadingSch:Zn001:roof,",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    0.5432;                  !- Field 4",
        "  BuildingSurface:Detailed,",
        "    Zn001:floor,             !- Name",
        "    Floor,                   !- Surface Type",
        "    SLAB FLOOR,              !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.0000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,-5,0;  !- X,Y,Z ==> Vertex 4 {m}",
        "  FenestrationSurface:Detailed,",
        "    Zn001:Wall-South:Win001, !- Name",
        "    Window,                  !- Surface Type",
        "    DOUBLE PANE HW WINDOW,   !- Construction Name",
        "    Zn001:Wall-South,        !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    TestFrameAndDivider,     !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-5,0.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-5,0.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;  !- X,Y,Z ==> Vertex 4 {m}",
        "  WindowProperty:FrameAndDivider,",
        "    TestFrameAndDivider,     !- Name",
        "    0.05,                    !- Frame Width {m}",
        "    0.05,                    !- Frame Outside Projection {m}",
        "    0.05,                    !- Frame Inside Projection {m}",
        "    5.0,                     !- Frame Conductance {W/m2-K}",
        "    1.2,                     !- Ratio of Frame-Edge Glass Conductance to Center-Of-Gl",
        "    0.8,                     !- Frame Solar Absorptance",
        "    0.8,                     !- Frame Visible Absorptance",
        "    0.9,                     !- Frame Thermal Hemispherical Emissivity",
        "    DividedLite,             !- Divider Type",
        "    0.02,                    !- Divider Width {m}",
        "    2,                       !- Number of Horizontal Dividers",
        "    2,                       !- Number of Vertical Dividers",
        "    0.02,                    !- Divider Outside Projection {m}",
        "    0.02,                    !- Divider Inside Projection {m}",
        "    5.0,                     !- Divider Conductance {W/m2-K}",
        "    1.2,                     !- Ratio of Divider-Edge Glass Conductance to Center-Of-",
        "    0.8,                     !- Divider Solar Absorptance",
        "    0.8,                     !- Divider Visible Absorptance",
        "    0.9;                     !- Divider Thermal Hemispherical Emissivity",
        "  Shading:Zone:Detailed,",
        "    Zn001:Wall-South:Shade001,  !- Name",
        "    Zn001:Wall-South,        !- Base Surface Name",
        "    SunShading,              !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-6,2.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-6,2.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;  !- X,Y,Z ==> Vertex 4 {m}",
        "  ShadingProperty:Reflectance,",
        "    Zn001:Wall-South:Shade001,  !- Shading Surface Name",
        "    0.2,                     !- Diffuse Solar Reflectance of Unglazed Part of Shading",
        "    0.2;                     !- Diffuse Visible Reflectance of Unglazed Part of Shading",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool FoundError = false;
    HeatBalanceManager::GetProjectControlData(*state, FoundError); // read project control data
    HeatBalanceManager::SetPreConstructionInputParameters(*state);

    Material::GetMaterialData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    HeatBalanceManager::GetFrameAndDividerData(*state);

    HeatBalanceManager::GetConstructData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    SolarShading::GetShadowingInput(*state);

    HeatBalanceManager::GetZoneData(*state, FoundError); // Read Zone data from input file
    EXPECT_FALSE(FoundError);

    SurfaceGeometry::GetGeometryParameters(*state, FoundError);
    EXPECT_FALSE(FoundError);

    SizingManager::GetZoneSizingInput(*state);

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(1);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(1);

    state->dataSurfaceGeometry->CosZoneRelNorth(1) = std::cos(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(1) = std::sin(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;

    compare_err_stream("");                                 // just for debugging
    SurfaceGeometry::SetupZoneGeometry(*state, FoundError); // this calls GetSurfaceData()
    EXPECT_FALSE(FoundError);

    compare_err_stream(""); // just for debugging

    SolarShading::processShadowingInput(*state);

    for (int SurfNum = 1; SurfNum <= state->dataSurface->TotSurfaces; SurfNum++) {
        if (state->dataSurface->Surface(SurfNum).ExtBoundCond == 0 && state->dataSurface->Surface(SurfNum).Zone != 0) {
            int ZoneSize = state->dataSurface->SurfShadowDisabledZoneList(SurfNum).size();
            EXPECT_EQ(1, ZoneSize);
            std::vector<int> DisabledZones = state->dataSurface->SurfShadowDisabledZoneList(SurfNum);
            for (int i : DisabledZones) {
                EXPECT_EQ(1, i);
            }
        }
    }
}

TEST_F(EnergyPlusFixture, SolarShadingTest_PolygonClippingDirect)
{
    std::string const idf_objects = delimited_string({
        "  Building,",
        "    DemoFDT,                 !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    3.9999999E-02,           !- Loads Convergence Tolerance Value",
        "    4.0000002E-03,           !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullExterior,            !- Solar Distribution",
        "    ,                        !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",
        "  ShadowCalculation,",
        "    PolygonClipping,         !- Shading Calculation Method",
        "    Timestep,                !- Shading Calculation Update Frequency Method",
        "    ,                        !- Shading Calculation Update Frequency",
        "    ,                        !- Maximum Figures in Shadow Overlap Calculations",
        "    ,                        !- Polygon Clipping Algorithm",
        "    ,                        !- Pixel Counting Resolution",
        "    DetailedSkyDiffuseModeling;  !- Sky Diffuse Modeling Algorithm",
        "  SurfaceConvectionAlgorithm:Inside,TARP;",
        "  SurfaceConvectionAlgorithm:Outside,TARP;",
        "  HeatBalanceAlgorithm,ConductionTransferFunction;",
        "  Timestep,6;",
        "  RunPeriod,",
        "    RP1,                     !- Name",
        "    1,                       !- Begin Month",
        "    1,                       !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    12,                      !- End Month",
        "    31,                      !- End Day of Month",
        "    ,                        !- End Year",
        "    ,                        !- Day of Week for Start Day",
        "    ,                        !- Use Weather File Holidays and Special Days",
        "    ,                        !- Use Weather File Daylight Saving Period",
        "    ,                        !- Apply Weekend Holiday Rule",
        "    ,                        !- Use Weather File Rain Indicators",
        "    ;                        !- Use Weather File Snow Indicators",
        "  ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    Continuous;              !- Numeric Type",
        "  ScheduleTypeLimits,",
        "    ON/OFF,                  !- Name",
        "    0,                       !- Lower Limit Value",
        "    1,                       !- Upper Limit Value",
        "    Discrete;                !- Numeric Type",
        "  Schedule:Compact,",
        "    SunShading,              !- Name",
        "    ON/OFF,                  !- Schedule Type Limits Name",
        "    Through: 4/30,           !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    until: 24:00,1,          !- Field 3",
        "    Through: 10/31,          !- Field 5",
        "    For: AllDays,            !- Field 6",
        "    until: 24:00,0,          !- Field 7",
        "    Through: 12/31,          !- Field 9",
        "    For: AllDays,            !- Field 10",
        "    until: 24:00,1;          !- Field 11",
        "  Material,",
        "    A2 - 4 IN DENSE FACE BRICK,  !- Name",
        "    Rough,                   !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    1.245296,                !- Conductivity {W/m-K}",
        "    2082.400,                !- Density {kg/m3}",
        "    920.4800,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9300000,               !- Solar Absorptance",
        "    0.9300000;               !- Visible Absorptance",
        "  Material,",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD,  !- Name",
        "    Smooth,                  !- Roughness",
        "    1.9050000E-02,           !- Thickness {m}",
        "    0.7264224,               !- Conductivity {W/m-K}",
        "    1601.846,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",
        "  Material,",
        "    E2 - 1 / 2 IN SLAG OR STONE,  !- Name",
        "    Rough,                   !- Roughness",
        "    1.2710161E-02,           !- Thickness {m}",
        "    1.435549,                !- Conductivity {W/m-K}",
        "    881.0155,                !- Density {kg/m3}",
        "    1673.600,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.5500000,               !- Solar Absorptance",
        "    0.5500000;               !- Visible Absorptance",
        "  Material,",
        "    C12 - 2 IN HW CONCRETE,  !- Name",
        "    MediumRough,             !- Roughness",
        "    5.0901599E-02,           !- Thickness {m}",
        "    1.729577,                !- Conductivity {W/m-K}",
        "    2242.585,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",
        "  Material:NoMass,",
        "    R13LAYER,                !- Name",
        "    Rough,                   !- Roughness",
        "    2.290965,                !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",
        "  WindowMaterial:Glazing,",
        "    GLASS - CLEAR PLATE 1 / 4 IN,  !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.006,                   !- Thickness {m}",
        "    0.80,                    !- Solar Transmittance at Normal Incidence",
        "    0.10,                    !- Front Side Solar Reflectance at Normal Incidence",
        "    0.10,                    !- Back Side Solar Reflectance at Normal Incidence",
        "    0.80,                    !- Visible Transmittance at Normal Incidence",
        "    0.10,                    !- Front Side Visible Reflectance at Normal Incidence",
        "    0.10,                    !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.84,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",
        "  WindowMaterial:Gas,",
        "    AIRGAP,                  !- Name",
        "    AIR,                     !- Gas Type",
        "    0.0125;                  !- Thickness {m}",
        "  Construction,",
        "    R13WALL,                 !- Name",
        "    R13LAYER;                !- Outside Layer",
        "  Construction,",
        "    EXTWALL09,               !- Name",
        "    A2 - 4 IN DENSE FACE BRICK,  !- Outside Layer",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 4",
        "  Construction,",
        "    INTERIOR,                !- Name",
        "    C12 - 2 IN HW CONCRETE;  !- Layer 4",
        "  Construction,",
        "    SLAB FLOOR,              !- Name",
        "    C12 - 2 IN HW CONCRETE;  !- Layer 4",
        "  Construction,",
        "    ROOF31,                  !- Name",
        "    E2 - 1 / 2 IN SLAG OR STONE,  !- Outside Layer",
        "    C12 - 2 IN HW CONCRETE;  !- Layer 4",
        "  Construction,",
        "    DOUBLE PANE HW WINDOW,   !- Name",
        "    GLASS - CLEAR PLATE 1 / 4 IN,  !- Outside Layer",
        "    AIRGAP,                  !- Layer 2",
        "    GLASS - CLEAR PLATE 1 / 4 IN;  !- Layer 3",
        "  Construction,",
        "    PARTITION02,             !- Name",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD,  !- Outside Layer",
        "    C12 - 2 IN HW CONCRETE,  !- Layer 4",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 3",
        "  Construction,",
        "    single PANE HW WINDOW,   !- Name",
        "    GLASS - CLEAR PLATE 1 / 4 IN;  !- Outside Layer",
        "  Construction,",
        "    EXTWALLdemo,             !- Name",
        "    A2 - 4 IN DENSE FACE BRICK,  !- Outside Layer",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 4",
        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    Counterclockwise,        !- Vertex Entry Direction",
        "    Relative;                !- Coordinate System",
        "  Zone,",
        "    ZONE ONE,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    0,                       !- Ceiling Height {m}",
        "    0;                       !- Volume {m3}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-North,        !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALLdemo,             !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    5,5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    -5,5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-East,         !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL09,               !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    5,-5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    5,5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-South,        !- Name",
        "    Wall,                    !- Surface Type",
        "    R13WALL,                 !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,-5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -5,-5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    5,-5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-West,         !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL09,               !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -5,5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    -5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,-5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:roof,              !- Name",
        "    Roof,                    !- Surface Type",
        "    ROOF31,                  !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.0000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,-5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,-5,3,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,5,3,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:floor,             !- Name",
        "    Floor,                   !- Surface Type",
        "    SLAB FLOOR,              !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.0000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,-5,0;  !- X,Y,Z ==> Vertex 4 {m}",
        "  FenestrationSurface:Detailed,",
        "    Zn001:Wall-South:Win001, !- Name",
        "    Window,                  !- Surface Type",
        "    DOUBLE PANE HW WINDOW,   !- Construction Name",
        "    Zn001:Wall-South,        !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    TestFrameAndDivider,     !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-5,0.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-5,0.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;  !- X,Y,Z ==> Vertex 4 {m}",
        "  WindowProperty:FrameAndDivider,",
        "    TestFrameAndDivider,     !- Name",
        "    0.05,                    !- Frame Width {m}",
        "    0.05,                    !- Frame Outside Projection {m}",
        "    0.05,                    !- Frame Inside Projection {m}",
        "    5.0,                     !- Frame Conductance {W/m2-K}",
        "    1.2,                     !- Ratio of Frame-Edge Glass Conductance to Center-Of-Gl",
        "    0.8,                     !- Frame Solar Absorptance",
        "    0.8,                     !- Frame Visible Absorptance",
        "    0.9,                     !- Frame Thermal Hemispherical Emissivity",
        "    DividedLite,             !- Divider Type",
        "    0.02,                    !- Divider Width {m}",
        "    2,                       !- Number of Horizontal Dividers",
        "    2,                       !- Number of Vertical Dividers",
        "    0.02,                    !- Divider Outside Projection {m}",
        "    0.02,                    !- Divider Inside Projection {m}",
        "    5.0,                     !- Divider Conductance {W/m2-K}",
        "    1.2,                     !- Ratio of Divider-Edge Glass Conductance to Center-Of-",
        "    0.8,                     !- Divider Solar Absorptance",
        "    0.8,                     !- Divider Visible Absorptance",
        "    0.9;                     !- Divider Thermal Hemispherical Emissivity",
        "  Shading:Zone:Detailed,",
        "    Zn001:Wall-South:Shade001,  !- Name",
        "    Zn001:Wall-South,        !- Base Surface Name",
        "    SunShading,              !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-6,2.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-6,2.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;  !- X,Y,Z ==> Vertex 4 {m}",
        "  ShadingProperty:Reflectance,",
        "    Zn001:Wall-South:Shade001,  !- Shading Surface Name",
        "    0.2,                     !- Diffuse Solar Reflectance of Unglazed Part of Shading",
        "    0.2;                     !- Diffuse Visible Reflectance of Unglazed Part of Shading",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool FoundError = false;

    HeatBalanceManager::GetProjectControlData(*state, FoundError); // read project control data
    HeatBalanceManager::SetPreConstructionInputParameters(*state);

    Material::GetMaterialData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    HeatBalanceManager::GetFrameAndDividerData(*state);

    HeatBalanceManager::GetConstructData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    HeatBalanceManager::GetZoneData(*state, FoundError); // Read Zone data from input file
    EXPECT_FALSE(FoundError);

    SurfaceGeometry::GetGeometryParameters(*state, FoundError);
    EXPECT_FALSE(FoundError);

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(1);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(1);

    state->dataSurfaceGeometry->CosZoneRelNorth(1) = std::cos(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(1) = std::sin(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;

    SurfaceGeometry::GetSurfaceData(*state, FoundError); // setup zone geometry and get zone data
    EXPECT_FALSE(FoundError);                            // expect no errors

    //	compare_err_stream( "" ); // just for debugging

    SurfaceGeometry::SetupZoneGeometry(*state, FoundError); // this calls GetSurfaceData()
    EXPECT_FALSE(FoundError);

    SolarShading::AllocateModuleArrays(*state);
    SolarShading::DetermineShadowingCombinations(*state);
    state->dataEnvrn->DayOfYear_Schedule = 168;
    state->dataEnvrn->DayOfWeek = 6;
    state->dataGlobal->TimeStep = 4;
    state->dataGlobal->HourOfDay = 9;

    //	compare_err_stream( "" ); // just for debugging
    EXPECT_FALSE(state->dataSysVars->SlaterBarsky);

    state->dataSurface->ShadingTransmittanceVaries = true;
    state->dataSysVars->DetailedSkyDiffuseAlgorithm = true;
    state->dataHeatBal->SolarDistribution = DataHeatBalance::Shadowing::FullExterior;
    state->dataSysVars->SlaterBarsky = true;

    state->dataSolarShading->CalcSkyDifShading = true;
    SolarShading::InitSolarCalculations(*state);
    SolarShading::SkyDifSolarShading(*state);
    state->dataSolarShading->CalcSkyDifShading = false;

    state->dataBSDFWindow->SUNCOSTS(state->dataGlobal->TimeStep, state->dataGlobal->HourOfDay)(1) = 0.20531446332266728;
    state->dataBSDFWindow->SUNCOSTS(state->dataGlobal->TimeStep, state->dataGlobal->HourOfDay)(2) = -0.84761109808931534;
    state->dataBSDFWindow->SUNCOSTS(state->dataGlobal->TimeStep, state->dataGlobal->HourOfDay)(3) = 0.48928662105799514;

    FigureSolarBeamAtTimestep(*state, state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep);
    int surfNum = Util::FindItemInList("ZN001:WALL-SOUTH:WIN001", state->dataSurface->Surface);
    EXPECT_NEAR(0.6504, state->dataSolarShading->SurfDifShdgRatioIsoSkyHRTS(4, 9, surfNum), 0.0001);
    EXPECT_NEAR(0.9152, state->dataSolarShading->SurfDifShdgRatioHorizHRTS(4, 9, surfNum), 0.0001);

    state->dataSysVars->SlaterBarsky = false;
}

TEST_F(EnergyPlusFixture, SolarShadingTest_CHKBKS)
{
    int numofsurface;
    numofsurface = 4;
    state->dataSurface->Surface.allocate(numofsurface);
    Vector VtxA1(0.0, 0.0, 5.0);
    Vector VtxA2(0.0, 0.0, 0.0);
    Vector VtxA3(5.0, 0.0, 0.0);
    Vector VtxA4(5.0, 0.0, 5.0);

    Vector VtxB1(0.0, -10.0, 5.0);
    Vector VtxB2(0.0, -10.0, 2.5);
    Vector VtxB3(0.0, -5.0, 2.5);
    Vector VtxB4(0.0, -5.0, 0.0);
    Vector VtxB5(0.0, 0.0, 0.0);
    Vector VtxB6(0.0, 0.0, 5.0);

    state->dataSurface->Surface(1).Sides = 4;

    state->dataSurface->Surface(1).Vertex.allocate(4);
    state->dataSurface->Surface(1).Vertex(1) = VtxA1;
    state->dataSurface->Surface(1).Vertex(2) = VtxA2;
    state->dataSurface->Surface(1).Vertex(3) = VtxA3;
    state->dataSurface->Surface(1).Vertex(4) = VtxA4;

    state->dataSurface->Surface(1).Name = "Surf_Back";
    state->dataSurface->Surface(1).ZoneName = "Zone1";

    state->dataSurface->Surface(2).Sides = 6; // receiving
    state->dataSurface->Surface(2).Vertex.allocate(6);

    state->dataSurface->Surface(2).Vertex(1) = VtxB1;
    state->dataSurface->Surface(2).Vertex(2) = VtxB2;
    state->dataSurface->Surface(2).Vertex(3) = VtxB3;
    state->dataSurface->Surface(2).Vertex(4) = VtxB4;
    state->dataSurface->Surface(2).Vertex(5) = VtxB5;
    state->dataSurface->Surface(2).Vertex(6) = VtxB6;

    state->dataSurface->Surface(2).Name = "Surf_Recv";
    state->dataSurface->Surface(2).ZoneName = "Zone1";

    CHKBKS(*state, 1, 2);

    EXPECT_TRUE(this->has_err_output(false));

    std::string const error_string =
        delimited_string({"   ** Severe  ** Problem in interior solar distribution calculation (CHKBKS)",
                          "   **   ~~~   **    Solar Distribution = FullInteriorExterior will not work in Zone=Zone1",
                          "   **   ~~~   **    because one or more of vertices, such as Vertex 3 of back surface=Surf_Back"
                          ", is in front of receiving surface=Surf_Recv",
                          "   **   ~~~   **    (Dot Product indicator=             62.5000)",
                          "   **   ~~~   **    Check surface geometry; if OK, use Solar Distribution = FullExterior instead. Use Output:Diagnostics, "
                          "DisplayExtraWarnings; for more details."});

    EXPECT_TRUE(compare_err_stream(error_string, true));

    state->dataSurface->Surface(3).Sides = 4;

    state->dataSurface->Surface(3).Vertex.allocate(4);
    state->dataSurface->Surface(3).Vertex(1) = VtxA1;
    state->dataSurface->Surface(3).Vertex(2) = VtxA2;
    state->dataSurface->Surface(3).Vertex(3) = VtxA3;
    state->dataSurface->Surface(3).Vertex(4) = VtxA4;

    state->dataSurface->Surface(3).Name = "Surf_Back2";
    state->dataSurface->Surface(3).ZoneName = "Zone2";

    state->dataSurface->Surface(4).Sides = 6; // receiving
    state->dataSurface->Surface(4).Vertex.allocate(6);

    state->dataSurface->Surface(4).Vertex(1) = VtxB6;
    state->dataSurface->Surface(4).Vertex(2) = VtxB5;
    state->dataSurface->Surface(4).Vertex(3) = VtxB4;
    state->dataSurface->Surface(4).Vertex(4) = VtxB3;
    state->dataSurface->Surface(4).Vertex(5) = VtxB2;
    state->dataSurface->Surface(4).Vertex(6) = VtxB1;

    state->dataSurface->Surface(4).Name = "Surf_Recv2";
    state->dataSurface->Surface(4).ZoneName = "Zone2";

    CHKBKS(*state, 3, 4);

    EXPECT_FALSE(this->has_err_output(true));
}

TEST_F(EnergyPlusFixture, SolarShadingTest_NonConvexErrors)
{
    std::string const idf_objects = R"IDF(
RunPeriod,
  Annual,                  !- Name
  1,                       !- Begin Month
  1,                       !- Begin Day of Month
  ,                        !- Begin Year
  12,                      !- End Month
  31,                      !- End Day of Month
  ,                        !- End Year
  Sunday,                  !- Day of Week for Start Day
  No,                      !- Use Weather File Holidays and Special Days
  No,                      !- Use Weather File Daylight Saving Period
  No,                      !- Apply Weekend Holiday Rule
  Yes,                     !- Use Weather File Rain Indicators
  Yes;                     !- Use Weather File Snow Indicators

ShadowCalculation,
  PolygonClipping,  !- Calculation Method
  Periodic,  !- Caclulation Update Frequency Method
  20,  !- Calculation Update Frequency
  15000;  !- Maximum Figures in Shadow Overlap Calculations

Timestep,4;

Building,
  Non-convex,    !- Name
  0,      !- North Axis {deg}
  City,          !- Terrain
  0.5,  !- Loads Convergence Tolerance Value
  0.5,  !- Temperature Convergence Tolerance Value {deltaC}
  FullInteriorAndExterior, !- Solar Distribution
  25,  !- Maximum Number of Warmup Days
  2;  !- Minimum Number of Warmup Days

GlobalGeometryRules,
    UpperLeftCorner,          !- Starting Vertex Position
    Counterclockwise,         !- Vertex Entry Direction
    Relative;                 !- Coordinate System

Zone,
    Zone A,                   !- Name
    0,                        !- Direction of Relative North
    0,                        !- X Origin
    0,                        !- Y Origin
    0,                        !- Z Origin
    ,                         !- Type
    1;                        !- Multiplier

BuildingSurface:Detailed,
    A48756,                   !- Name
    Floor,                    !- Surface Type
    Exterior Wall,            !- Construction Name
    Zone A,                   !- Zone Name
    ,                         !- Space Name
    Outdoors,                 !- Outside Boundary Condition
    ,                         !- Outside Boundary Condition Object
    NoSun,                    !- Sun Exposure
    NoWind,                   !- Wind Exposure
    ,                         !- View Factor to Ground
    4,                        !- Number of Vertices
    6.096, 6.096, 0,                        !- Vertex 1 Zcoordinate
    6.096, 0, 0,                        !- Vertex 2 Zcoordinate
    0, 0, 0,                        !- Vertex 3 Zcoordinate
    0, 6.096, 0;                        !- Vertex 4 Zcoordinate

BuildingSurface:Detailed,
    69C03D,                   !- Name
    Wall,                     !- Surface Type
    Exterior Wall,            !- Construction Name
    Zone A,                   !- Zone Name
    ,                         !- Space Name
    Outdoors,                 !- Outside Boundary Condition
    ,                         !- Outside Boundary Condition Object
    SunExposed,               !- Sun Exposure
    WindExposed,              !- Wind Exposure
    ,                         !- View Factor to Ground
    6,                        !- Number of Vertices
    0, 6.096, 6.096,                    !- Vertex 1 Zcoordinate
    0, 6.096, 0,                        !- Vertex 2 Zcoordinate
    0, 0, 0,                        !- Vertex 3 Zcoordinate
    0, 0, 3.048,                    !- Vertex 4 Zcoordinate
    0, 3.048, 3.048,                    !- Vertex 5 Zcoordinate
    0, 3.048, 6.096;                    !- Vertex 6 Zcoordinate

BuildingSurface:Detailed,
    5BB552,                   !- Name
    Wall,                     !- Surface Type
    Exterior Wall,            !- Construction Name
    Zone A,                   !- Zone Name
    ,                         !- Space Name
    Outdoors,                 !- Outside Boundary Condition
    ,                         !- Outside Boundary Condition Object
    SunExposed,               !- Sun Exposure
    WindExposed,              !- Wind Exposure
    ,                         !- View Factor to Ground
    6,                        !- Number of Vertices
    6.096, 3.048, 6.096,                    !- Vertex 1 Zcoordinate
    6.096, 3.048, 3.048,                    !- Vertex 2 Zcoordinate
    6.096, 0, 3.048,                    !- Vertex 3 Zcoordinate
    6.096, 0, 0,                        !- Vertex 4 Zcoordinate
    6.096, 6.096, 0,                        !- Vertex 5 Zcoordinate
    6.096, 6.096, 6.096;                    !- Vertex 6 Zcoordinate

BuildingSurface:Detailed,
    682F88,                   !- Name
    Wall,                     !- Surface Type
    Exterior Wall,            !- Construction Name
    Zone A,                   !- Zone Name
    ,                         !- Space Name
    Outdoors,                 !- Outside Boundary Condition
    ,                         !- Outside Boundary Condition Object
    SunExposed,               !- Sun Exposure
    WindExposed,              !- Wind Exposure
    ,                         !- View Factor to Ground
    4,                        !- Number of Vertices
    6.096, 6.096, 6.096,                    !- Vertex 1 Zcoordinate
    6.096, 6.096, 0,                        !- Vertex 2 Zcoordinate
    0, 6.096, 0,                        !- Vertex 3 Zcoordinate
    0, 6.096, 6.096;                    !- Vertex 4 Zcoordinate

BuildingSurface:Detailed,
    C292CF,                   !- Name
    Wall,                     !- Surface Type
    Exterior Wall,            !- Construction Name
    Zone A,                   !- Zone Name
    ,                         !- Space Name
    Outdoors,                 !- Outside Boundary Condition
    ,                         !- Outside Boundary Condition Object
    SunExposed,               !- Sun Exposure
    WindExposed,              !- Wind Exposure
    ,                         !- View Factor to Ground
    4,                        !- Number of Vertices
    0, 0, 3.048,                    !- Vertex 1 Zcoordinate
    0, 0, 0,                        !- Vertex 2 Zcoordinate
    6.096, 0, 0,                        !- Vertex 3 Zcoordinate
    6.096, 0, 3.048;                    !- Vertex 4 Zcoordinate

BuildingSurface:Detailed,
    319080,                   !- Name
    Wall,                     !- Surface Type
    Exterior Wall,            !- Construction Name
    Zone A,                   !- Zone Name
    ,                         !- Space Name
    Outdoors,                 !- Outside Boundary Condition
    ,                         !- Outside Boundary Condition Object
    SunExposed,               !- Sun Exposure
    WindExposed,              !- Wind Exposure
    ,                         !- View Factor to Ground
    4,                        !- Number of Vertices
    0, 3.048, 6.096,                    !- Vertex 1 Zcoordinate
    0, 3.048, 3.048,                    !- Vertex 2 Zcoordinate
    6.096, 3.048, 3.048,                    !- Vertex 3 Zcoordinate
    6.096, 3.048, 6.096;                    !- Vertex 4 Zcoordinate

BuildingSurface:Detailed,
    866E33,                   !- Name
    Roof,                     !- Surface Type
    Exterior Wall,            !- Construction Name
    Zone A,                   !- Zone Name
    ,                         !- Space Name
    Outdoors,                 !- Outside Boundary Condition
    ,                         !- Outside Boundary Condition Object
    SunExposed,               !- Sun Exposure
    WindExposed,              !- Wind Exposure
    ,                         !- View Factor to Ground
    4,                        !- Number of Vertices
    0, 3.048, 3.048,                    !- Vertex 1 Zcoordinate
    0, 0, 3.048,                    !- Vertex 2 Zcoordinate
    6.096, 0, 3.048,                    !- Vertex 3 Zcoordinate
    6.096, 3.048, 3.048;                    !- Vertex 4 Zcoordinate

BuildingSurface:Detailed,
    32CB6F,                   !- Name
    Roof,                     !- Surface Type
    Exterior Wall,            !- Construction Name
    Zone A,                   !- Zone Name
    ,                         !- Space Name
    Outdoors,                 !- Outside Boundary Condition
    ,                         !- Outside Boundary Condition Object
    SunExposed,               !- Sun Exposure
    WindExposed,              !- Wind Exposure
    ,                         !- View Factor to Ground
    4,                        !- Number of Vertices
    0, 6.096, 6.096,                    !- Vertex 1 Zcoordinate
    0, 3.048, 6.096,                    !- Vertex 2 Zcoordinate
    6.096, 3.048, 6.096,                    !- Vertex 3 Zcoordinate
    6.096, 6.096, 6.096;                    !- Vertex 4 Zcoordinate

FenestrationSurface:Detailed,
    DAF2B2,                   !- Name
    Window,                   !- Surface Type
    Exterior Window,          !- Construction Name
    319080,                   !- Building Surface Name
    ,                         !- Outside Boundary Condition Object
    ,                         !- View Factor to Ground
    ,                         !- Frame and Divider Name
    ,                         !- Multiplier
    4,                        !- Number of Vertices
    0.6096, 3.048, 5.4864,                   !- Vertex 1 Zcoordinate
    0.6096, 3.048, 3.6576,                   !- Vertex 2 Zcoordinate
    5.4864, 3.048, 3.6576,                   !- Vertex 3 Zcoordinate
    5.4864, 3.048, 5.4864;                   !- Vertex 4 Zcoordinate

Material,
  Wall Continuous Insulation,  !- Name
  MediumSmooth,               !- Roughness
  0.038037600000000005,  !- Thickness {m}
  0.0288,  !- Conductivity {W/m-K}
  32,                      !- Density {kg/m3}(2013 ASHRAE Handbook - Fundamentals Section 26 Table 1 - Extruded polystyrene, smooth skin - aged 180 days)
  1465;                     !- Specific Heat {J/kg-K}(2013 ASHRAE Handbook - Fundamentals Section 26 Table 1 - Extruded polystyrene, smooth skin - aged 180 days)

Material,
  Wall Cavity Effective Insulation,  !- Name
  VeryRough,               !- Roughness
  0.08889999999999999,  !- Thickness {m}
  0.08389829384297165,  !- Conductivity {W/m-K}
  8.17,                      !- Density {kg/m3}(2013 ASHRAE Handbook - Fundamentals Section 26 Table 1 - Glass-fiber batts)
  837;                     !- Specific Heat {J/kg-K}(2013 ASHRAE Handbook - Fundamentals Section 26 Table 1 - Glass-fiber batts)

Construction,
  Exterior Wall,           !- Name
  Stucco,  !- Layer
  Wall Continuous Insulation,  !- Layer
  Wall Cavity Effective Insulation,  !- Layer
  Gypsum 5/8in;  !- Layer

Construction,
  Exterior Window,         !- Name
  Theoretical Glazing;     !- Outside Layer

Material,
  Gypsum 5/8in,           !- Name
  MediumSmooth,            !- Roughness
  0.016,                   !- Thickness {m}
  0.1622,                  !- Conductivity {W/m-K}
  800,                     !- Density {kg/m3}
  1090,                    !- Specific Heat {J/kg-K}
  0.9,                     !- Thermal Absorptance
  0.7,                     !- Solar Absorptance
  0.7;                     !- Visible Absorptance

Material,
  Stucco,           !- Name
  Smooth,                  !- Roughness
  0.01015,                 !- Thickness {m}
  0.72,                    !- Conductivity {W/m-K}
  1856,                    !- Density {kg/m3}
  840,                     !- Specific Heat {J/kg-K}
  0.9,                     !- Thermal Absorptance
  0.7,                     !- Solar Absorptance
  0.7;                     !- Visible Absorptance

WindowMaterial:SimpleGlazingSystem,
  Theoretical Glazing,     !- Name
  3.1232254400908577,  !- U-Factor {W/m2-K}
  0.4,  !- Solar Heat Gain Coefficient
  0.4;  !- Visible Transmittance
)IDF";

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool FoundError = false;

    HeatBalanceManager::GetProjectControlData(*state, FoundError); // read project control data
    EXPECT_FALSE(FoundError);                                      // expect no errors

    HeatBalanceManager::SetPreConstructionInputParameters(*state);

    Material::GetMaterialData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    HeatBalanceManager::GetFrameAndDividerData(*state);

    HeatBalanceManager::GetConstructData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    HeatBalanceManager::GetZoneData(*state, FoundError); // Read Zone data from input file
    EXPECT_FALSE(FoundError);

    SurfaceGeometry::GetGeometryParameters(*state, FoundError);
    EXPECT_FALSE(FoundError);

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(1);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(1);

    state->dataSurfaceGeometry->CosZoneRelNorth(1) = std::cos(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(1) = std::sin(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;

    SurfaceGeometry::GetSurfaceData(*state, FoundError); // setup zone geometry and get zone data
    EXPECT_FALSE(FoundError);                            // expect no errors

    //	compare_err_stream( "" ); // just for debugging

    SurfaceGeometry::SetupZoneGeometry(*state, FoundError); // this calls GetSurfaceData()
    EXPECT_FALSE(FoundError);

    state->dataGlobal->BeginSimFlag = true;
    HeatBalanceManager::InitHeatBalance(*state);
    EXPECT_FALSE(FoundError);

    SolarShading::AllocateModuleArrays(*state);
    SolarShading::DetermineShadowingCombinations(*state);

    std::string error_string = delimited_string({"** Severe  ** Problem in interior solar distribution calculation (CHKBKS)"});

    match_err_stream(error_string);

    error_string =
        delimited_string({"** Severe  ** DetermineShadowingCombinations: There are 1 surfaces which are casting surfaces and are non-convex."});

    match_err_stream(error_string);
}

TEST_F(EnergyPlusFixture, SolarShadingTest_GPUNonConvexErrors)
{
    std::string const idf_objects = R"IDF(
RunPeriod,
  Annual,                  !- Name
  1,                       !- Begin Month
  1,                       !- Begin Day of Month
  ,                        !- Begin Year
  12,                      !- End Month
  31,                      !- End Day of Month
  ,                        !- End Year
  Sunday,                  !- Day of Week for Start Day
  No,                      !- Use Weather File Holidays and Special Days
  No,                      !- Use Weather File Daylight Saving Period
  No,                      !- Apply Weekend Holiday Rule
  Yes,                     !- Use Weather File Rain Indicators
  Yes;                     !- Use Weather File Snow Indicators

ShadowCalculation,
  PixelCounting,  !- Calculation Method
  Periodic,  !- Caclulation Update Frequency Method
  20,  !- Calculation Update Frequency
  15000;  !- Maximum Figures in Shadow Overlap Calculations

Timestep,4;

Building,
  Non-convex,    !- Name
  0,      !- North Axis {deg}
  City,          !- Terrain
  0.5,  !- Loads Convergence Tolerance Value
  0.5,  !- Temperature Convergence Tolerance Value {deltaC}
  FullInteriorAndExterior, !- Solar Distribution
  25,  !- Maximum Number of Warmup Days
  2;  !- Minimum Number of Warmup Days

GlobalGeometryRules,
    UpperLeftCorner,          !- Starting Vertex Position
    Counterclockwise,         !- Vertex Entry Direction
    Relative;                 !- Coordinate System

Zone,
    Zone A,                   !- Name
    0,                        !- Direction of Relative North
    0,                        !- X Origin
    0,                        !- Y Origin
    0,                        !- Z Origin
    ,                         !- Type
    1;                        !- Multiplier

BuildingSurface:Detailed,
    A48756,                   !- Name
    Floor,                    !- Surface Type
    Exterior Wall,            !- Construction Name
    Zone A,                   !- Zone Name
    ,                         !- Space Name
    Outdoors,                 !- Outside Boundary Condition
    ,                         !- Outside Boundary Condition Object
    NoSun,                    !- Sun Exposure
    NoWind,                   !- Wind Exposure
    ,                         !- View Factor to Ground
    4,                        !- Number of Vertices
    6.096, 6.096, 0,                        !- Vertex 1 Zcoordinate
    6.096, 0, 0,                        !- Vertex 2 Zcoordinate
    0, 0, 0,                        !- Vertex 3 Zcoordinate
    0, 6.096, 0;                        !- Vertex 4 Zcoordinate

BuildingSurface:Detailed,
    69C03D,                   !- Name
    Wall,                     !- Surface Type
    Exterior Wall,            !- Construction Name
    Zone A,                   !- Zone Name
    ,                         !- Space Name
    Outdoors,                 !- Outside Boundary Condition
    ,                         !- Outside Boundary Condition Object
    SunExposed,               !- Sun Exposure
    WindExposed,              !- Wind Exposure
    ,                         !- View Factor to Ground
    6,                        !- Number of Vertices
    0, 6.096, 6.096,                    !- Vertex 1 Zcoordinate
    0, 6.096, 0,                        !- Vertex 2 Zcoordinate
    0, 0, 0,                        !- Vertex 3 Zcoordinate
    0, 0, 3.048,                    !- Vertex 4 Zcoordinate
    0, 3.048, 3.048,                    !- Vertex 5 Zcoordinate
    0, 3.048, 6.096;                    !- Vertex 6 Zcoordinate

BuildingSurface:Detailed,
    5BB552,                   !- Name
    Wall,                     !- Surface Type
    Exterior Wall,            !- Construction Name
    Zone A,                   !- Zone Name
    ,                         !- Space Name
    Outdoors,                 !- Outside Boundary Condition
    ,                         !- Outside Boundary Condition Object
    SunExposed,               !- Sun Exposure
    WindExposed,              !- Wind Exposure
    ,                         !- View Factor to Ground
    6,                        !- Number of Vertices
    6.096, 3.048, 6.096,                    !- Vertex 1 Zcoordinate
    6.096, 3.048, 3.048,                    !- Vertex 2 Zcoordinate
    6.096, 0, 3.048,                    !- Vertex 3 Zcoordinate
    6.096, 0, 0,                        !- Vertex 4 Zcoordinate
    6.096, 6.096, 0,                        !- Vertex 5 Zcoordinate
    6.096, 6.096, 6.096;                    !- Vertex 6 Zcoordinate

BuildingSurface:Detailed,
    682F88,                   !- Name
    Wall,                     !- Surface Type
    Exterior Wall,            !- Construction Name
    Zone A,                   !- Zone Name
    ,                         !- Space Name
    Outdoors,                 !- Outside Boundary Condition
    ,                         !- Outside Boundary Condition Object
    SunExposed,               !- Sun Exposure
    WindExposed,              !- Wind Exposure
    ,                         !- View Factor to Ground
    4,                        !- Number of Vertices
    6.096, 6.096, 6.096,                    !- Vertex 1 Zcoordinate
    6.096, 6.096, 0,                        !- Vertex 2 Zcoordinate
    0, 6.096, 0,                        !- Vertex 3 Zcoordinate
    0, 6.096, 6.096;                    !- Vertex 4 Zcoordinate

BuildingSurface:Detailed,
    C292CF,                   !- Name
    Wall,                     !- Surface Type
    Exterior Wall,            !- Construction Name
    Zone A,                   !- Zone Name
    ,                         !- Space Name
    Outdoors,                 !- Outside Boundary Condition
    ,                         !- Outside Boundary Condition Object
    SunExposed,               !- Sun Exposure
    WindExposed,              !- Wind Exposure
    ,                         !- View Factor to Ground
    4,                        !- Number of Vertices
    0, 0, 3.048,                    !- Vertex 1 Zcoordinate
    0, 0, 0,                        !- Vertex 2 Zcoordinate
    6.096, 0, 0,                        !- Vertex 3 Zcoordinate
    6.096, 0, 3.048;                    !- Vertex 4 Zcoordinate

BuildingSurface:Detailed,
    319080,                   !- Name
    Wall,                     !- Surface Type
    Exterior Wall,            !- Construction Name
    Zone A,                   !- Zone Name
    ,                         !- Space Name
    Outdoors,                 !- Outside Boundary Condition
    ,                         !- Outside Boundary Condition Object
    SunExposed,               !- Sun Exposure
    WindExposed,              !- Wind Exposure
    ,                         !- View Factor to Ground
    4,                        !- Number of Vertices
    0, 3.048, 6.096,                    !- Vertex 1 Zcoordinate
    0, 3.048, 3.048,                    !- Vertex 2 Zcoordinate
    6.096, 3.048, 3.048,                    !- Vertex 3 Zcoordinate
    6.096, 3.048, 6.096;                    !- Vertex 4 Zcoordinate

BuildingSurface:Detailed,
    866E33,                   !- Name
    Roof,                     !- Surface Type
    Exterior Wall,            !- Construction Name
    Zone A,                   !- Zone Name
    ,                         !- Space Name
    Outdoors,                 !- Outside Boundary Condition
    ,                         !- Outside Boundary Condition Object
    SunExposed,               !- Sun Exposure
    WindExposed,              !- Wind Exposure
    ,                         !- View Factor to Ground
    4,                        !- Number of Vertices
    0, 3.048, 3.048,                    !- Vertex 1 Zcoordinate
    0, 0, 3.048,                    !- Vertex 2 Zcoordinate
    6.096, 0, 3.048,                    !- Vertex 3 Zcoordinate
    6.096, 3.048, 3.048;                    !- Vertex 4 Zcoordinate

BuildingSurface:Detailed,
    32CB6F,                   !- Name
    Roof,                     !- Surface Type
    Exterior Wall,            !- Construction Name
    Zone A,                   !- Zone Name
    ,                         !- Space Name
    Outdoors,                 !- Outside Boundary Condition
    ,                         !- Outside Boundary Condition Object
    SunExposed,               !- Sun Exposure
    WindExposed,              !- Wind Exposure
    ,                         !- View Factor to Ground
    4,                        !- Number of Vertices
    0, 6.096, 6.096,                    !- Vertex 1 Zcoordinate
    0, 3.048, 6.096,                    !- Vertex 2 Zcoordinate
    6.096, 3.048, 6.096,                    !- Vertex 3 Zcoordinate
    6.096, 6.096, 6.096;                    !- Vertex 4 Zcoordinate

FenestrationSurface:Detailed,
    DAF2B2,                   !- Name
    Window,                   !- Surface Type
    Exterior Window,          !- Construction Name
    319080,                   !- Building Surface Name
    ,                         !- Outside Boundary Condition Object
    ,                         !- View Factor to Ground
    ,                         !- Frame and Divider Name
    ,                         !- Multiplier
    4,                        !- Number of Vertices
    0.6096, 3.048, 5.4864,                   !- Vertex 1 Zcoordinate
    0.6096, 3.048, 3.6576,                   !- Vertex 2 Zcoordinate
    5.4864, 3.048, 3.6576,                   !- Vertex 3 Zcoordinate
    5.4864, 3.048, 5.4864;                   !- Vertex 4 Zcoordinate

Material,
  Wall Continuous Insulation,  !- Name
  MediumSmooth,               !- Roughness
  0.038037600000000005,  !- Thickness {m}
  0.0288,  !- Conductivity {W/m-K}
  32,                      !- Density {kg/m3}(2013 ASHRAE Handbook - Fundamentals Section 26 Table 1 - Extruded polystyrene, smooth skin - aged 180 days)
  1465;                     !- Specific Heat {J/kg-K}(2013 ASHRAE Handbook - Fundamentals Section 26 Table 1 - Extruded polystyrene, smooth skin - aged 180 days)

Material,
  Wall Cavity Effective Insulation,  !- Name
  VeryRough,               !- Roughness
  0.08889999999999999,  !- Thickness {m}
  0.08389829384297165,  !- Conductivity {W/m-K}
  8.17,                      !- Density {kg/m3}(2013 ASHRAE Handbook - Fundamentals Section 26 Table 1 - Glass-fiber batts)
  837;                     !- Specific Heat {J/kg-K}(2013 ASHRAE Handbook - Fundamentals Section 26 Table 1 - Glass-fiber batts)

Construction,
  Exterior Wall,           !- Name
  Stucco,  !- Layer
  Wall Continuous Insulation,  !- Layer
  Wall Cavity Effective Insulation,  !- Layer
  Gypsum 5/8in;  !- Layer

Construction,
  Exterior Window,         !- Name
  Theoretical Glazing;     !- Outside Layer

Material,
  Gypsum 5/8in,           !- Name
  MediumSmooth,            !- Roughness
  0.016,                   !- Thickness {m}
  0.1622,                  !- Conductivity {W/m-K}
  800,                     !- Density {kg/m3}
  1090,                    !- Specific Heat {J/kg-K}
  0.9,                     !- Thermal Absorptance
  0.7,                     !- Solar Absorptance
  0.7;                     !- Visible Absorptance

Material,
  Stucco,           !- Name
  Smooth,                  !- Roughness
  0.01015,                 !- Thickness {m}
  0.72,                    !- Conductivity {W/m-K}
  1856,                    !- Density {kg/m3}
  840,                     !- Specific Heat {J/kg-K}
  0.9,                     !- Thermal Absorptance
  0.7,                     !- Solar Absorptance
  0.7;                     !- Visible Absorptance

WindowMaterial:SimpleGlazingSystem,
  Theoretical Glazing,     !- Name
  3.1232254400908577,  !- U-Factor {W/m2-K}
  0.4,  !- Solar Heat Gain Coefficient
  0.4;  !- Visible Transmittance
)IDF";

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool FoundError = false;

    HeatBalanceManager::GetProjectControlData(*state, FoundError); // read project control data
    EXPECT_FALSE(FoundError);                                      // expect no errors

    HeatBalanceManager::SetPreConstructionInputParameters(*state);

    Material::GetMaterialData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    HeatBalanceManager::GetFrameAndDividerData(*state);

    HeatBalanceManager::GetConstructData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    HeatBalanceManager::GetZoneData(*state, FoundError); // Read Zone data from input file
    EXPECT_FALSE(FoundError);

    SurfaceGeometry::GetGeometryParameters(*state, FoundError);
    EXPECT_FALSE(FoundError);

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(1);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(1);

    state->dataSurfaceGeometry->CosZoneRelNorth(1) = std::cos(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(1) = std::sin(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;

    SurfaceGeometry::GetSurfaceData(*state, FoundError); // setup zone geometry and get zone data
    EXPECT_FALSE(FoundError);                            // expect no errors

    //	compare_err_stream( "" ); // just for debugging

    SurfaceGeometry::SetupZoneGeometry(*state, FoundError); // this calls GetSurfaceData()
    EXPECT_FALSE(FoundError);

    state->dataGlobal->BeginSimFlag = true;
    HeatBalanceManager::InitHeatBalance(*state);
    EXPECT_FALSE(FoundError);

    if (state->dataSolarShading->penumbra) {
        SolarShading::AllocateModuleArrays(*state);
        SolarShading::DetermineShadowingCombinations(*state);

        std::string error_string = delimited_string({"** Severe  ** Problem in interior solar distribution calculation (CHKBKS)"});

        EXPECT_FALSE(match_err_stream(error_string));

        error_string =
            delimited_string({"** Severe  ** DetermineShadowingCombinations: There are 1 surfaces which are casting surfaces and are non-convex."});

        EXPECT_FALSE(match_err_stream(error_string));
    } else {
        EXPECT_FALSE(false) << "Machine cannot create a valid OpenGL instance."; // Replace with GTEST_SKIP() when it's released?
    }
}

TEST_F(EnergyPlusFixture, SolarShadingTest_selectActiveWindowShadingControl)
{
    state->dataSurface->TotSurfaces = 2;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);

    int curSurface = 1;
    state->dataSurface->Surface(curSurface).windowShadingControlList.push_back(57);

    int curIndexActiveWindowShadingControl = selectActiveWindowShadingControlIndex(*state, curSurface);
    int activeWindowShadingControl = state->dataSurface->Surface(curSurface).windowShadingControlList[curIndexActiveWindowShadingControl];
    EXPECT_EQ(activeWindowShadingControl, 57);

    curSurface = 2;
    state->dataSurface->Surface(curSurface).windowShadingControlList.push_back(1);
    state->dataSurface->Surface(curSurface).windowShadingControlList.push_back(2);
    state->dataSurface->Surface(curSurface).windowShadingControlList.push_back(3);

    state->dataSurface->WindowShadingControl.allocate(3);
    auto *sch1 = state->dataSurface->WindowShadingControl(1).sched = Sched::AddScheduleConstant(*state, "SCHED-1");
    auto *sch2 = state->dataSurface->WindowShadingControl(2).sched = Sched::AddScheduleConstant(*state, "SCHED-2");
    auto *sch3 = state->dataSurface->WindowShadingControl(3).sched = Sched::AddScheduleConstant(*state, "SCHED-3");

    sch1->currentVal = 0;
    sch2->currentVal = 0;
    sch3->currentVal = 1;

    curIndexActiveWindowShadingControl = selectActiveWindowShadingControlIndex(*state, curSurface);
    activeWindowShadingControl = state->dataSurface->Surface(curSurface).windowShadingControlList[curIndexActiveWindowShadingControl];
    EXPECT_EQ(activeWindowShadingControl, 3);

    sch1->currentVal = 0;
    sch2->currentVal = 1;
    sch3->currentVal = 0;

    curIndexActiveWindowShadingControl = selectActiveWindowShadingControlIndex(*state, curSurface);
    activeWindowShadingControl = state->dataSurface->Surface(curSurface).windowShadingControlList[curIndexActiveWindowShadingControl];
    EXPECT_EQ(activeWindowShadingControl, 2);

    sch1->currentVal = 1;
    sch2->currentVal = 0;
    sch3->currentVal = 0;

    curIndexActiveWindowShadingControl = selectActiveWindowShadingControlIndex(*state, curSurface);
    activeWindowShadingControl = state->dataSurface->Surface(curSurface).windowShadingControlList[curIndexActiveWindowShadingControl];
    EXPECT_EQ(activeWindowShadingControl, 1);
}

TEST_F(EnergyPlusFixture, WindowShadingManager_Lum_Test)
{
    state->dataSurface->Surface.allocate(2);
    state->dataSurface->SurfaceWindow.allocate(2);
    state->dataSurface->surfShades.allocate(2);

    EnergyPlus::SurfaceGeometry::AllocateSurfaceWindows(*state, 2);
    state->dataConstruction->Construct.allocate(1);
    state->dataSurface->WindowShadingControl.allocate(2);
    state->dataDayltg->ZoneDaylight.allocate(1);

    auto &surf1 = state->dataSurface->Surface(1);
    auto &surf2 = state->dataSurface->Surface(2);
    surf1.Name = "Surface1";
    surf2.Name = "Surface2";
    surf1.Zone = 1;
    surf2.Zone = 1;
    surf1.Class = DataSurfaces::SurfaceClass::Window;
    surf2.Class = DataSurfaces::SurfaceClass::Window;
    surf1.ExtBoundCond = DataSurfaces::ExternalEnvironment;
    surf2.ExtBoundCond = DataSurfaces::ExternalEnvironment;
    surf1.windowShadingControlList.push_back(1);
    surf2.windowShadingControlList.push_back(2);
    surf1.HasShadeControl = true;
    surf2.HasShadeControl = true;

    state->dataSurface->SurfWinHasShadeOrBlindLayer(1) = false;
    state->dataSurface->SurfWinHasShadeOrBlindLayer(2) = false;
    surf1.activeShadedConstruction = 1;
    surf2.activeShadedConstruction = 1;

    state->dataConstruction->Construct(1).Name = "Construction1";

    auto &shCtrl1 = state->dataSurface->WindowShadingControl(1);
    auto &shCtrl2 = state->dataSurface->WindowShadingControl(2);
    shCtrl1.Name = "WindowShadingControl1";
    shCtrl2.Name = "WindowShadingControl2";
    shCtrl1.ShadingType = DataSurfaces::WinShadingType::IntShade;
    shCtrl2.ShadingType = DataSurfaces::WinShadingType::ExtShade;
    shCtrl1.shadingControlType = DataSurfaces::WindowShadingControlType::HiSolar_HiLumin_OffMidNight;
    shCtrl2.shadingControlType = DataSurfaces::WindowShadingControlType::HiSolar_HiLumin_OffNextMorning;
    shCtrl1.SetPoint = 9400;
    shCtrl2.SetPoint = 94.64;
    shCtrl1.SetPoint2 = 2000;
    shCtrl2.SetPoint2 = 2000;

    int SurfNum = 2;
    state->dataSurface->TotSurfaces = SurfNum;
    surf1.activeWindowShadingControl = surf1.windowShadingControlList[SolarShading::selectActiveWindowShadingControlIndex(*state, 1)];
    surf2.activeWindowShadingControl = surf2.windowShadingControlList[SolarShading::selectActiveWindowShadingControlIndex(*state, 2)];

    state->dataHeatBal->Zone.allocate(1);
    state->dataHeatBal->Zone(1).spaceIndexes.allocate(1);
    state->dataHeatBal->Zone(1).spaceIndexes[0] = 1;
    state->dataHeatBal->space.allocate(1);
    state->dataHeatBal->space(1).WindowSurfaceFirst = 1;
    state->dataHeatBal->space(1).WindowSurfaceLast = 2;
    state->dataGlobal->NumOfZones = 1;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(state->dataGlobal->NumOfZones);

    // the following enables calculation when sun is up with SolarOnWindow computed to be 3700
    int constexpr NumTimeSteps(6);
    int constexpr HoursInDay(24);
    state->dataEnvrn->SunIsUp = true;
    state->dataEnvrn->SunIsUpPrevTS = true;
    state->dataSolarShading->SurfAnisoSkyMult.allocate(SurfNum);
    state->dataSolarShading->SurfAnisoSkyMult = 1.0;
    state->dataEnvrn->DifSolarRad = 100.0;
    state->dataEnvrn->BeamSolarRad = 100.0;
    state->dataEnvrn->SOLCOS = 0.5;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->HourOfDay = 10;
    state->dataGlobal->TimeStepsInHour = NumTimeSteps;
    state->dataHeatBal->SurfSunlitFrac.allocate(HoursInDay, NumTimeSteps, state->dataSurface->TotSurfaces);
    state->dataHeatBal->SurfCosIncAng.allocate(HoursInDay, NumTimeSteps, state->dataSurface->TotSurfaces);
    state->dataHeatBal->SurfCosIncAng = 45;
    state->dataHeatBal->SurfSunlitFrac = 0.8;

    SolarShading::WindowShadingManager(*state);

    // solar below setpoint, shading expected to be conditionally off
    EXPECT_ENUM_EQ(state->dataSurface->SurfWinShadingFlag(1), WinShadingType::IntShadeConditionallyOff);

    // solar above setpoint, shading expcted to be on
    EXPECT_ENUM_EQ(state->dataSurface->SurfWinShadingFlag(2), WinShadingType::ExtShade);
}

TEST_F(EnergyPlusFixture, SolarShadingTest_ShadingFlagTest)
{
    WinShadingType ShadingFlag = WinShadingType::IntShade;
    EXPECT_TRUE(IS_SHADED(ShadingFlag));
    EXPECT_TRUE(ANY_SHADE(ShadingFlag));
    EXPECT_TRUE(ANY_SHADE_SCREEN(ShadingFlag));
    EXPECT_TRUE(ANY_INTERIOR_SHADE_BLIND(ShadingFlag));
    EXPECT_FALSE(ANY_BLIND(ShadingFlag));
    EXPECT_FALSE(ANY_EXTERIOR_SHADE_BLIND_SCREEN(ShadingFlag));
    EXPECT_FALSE(ANY_BETWEENGLASS_SHADE_BLIND(ShadingFlag));

    ShadingFlag = WinShadingType::ExtBlind;
    EXPECT_TRUE(ANY_BLIND(ShadingFlag));
    EXPECT_TRUE(ANY_EXTERIOR_SHADE_BLIND_SCREEN(ShadingFlag));

    ShadingFlag = WinShadingType::GlassConditionallyLightened;
    EXPECT_FALSE(IS_SHADED_NO_GLARE_CTRL(ShadingFlag));
}

TEST_F(EnergyPlusFixture, SolarShading_TestSurfsPropertyViewFactor)
{
    std::string const idf_objects = delimited_string({
        "  Building,",
        "    DemoFDT,                 !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    3.9999999E-02,           !- Loads Convergence Tolerance Value",
        "    4.0000002E-03,           !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullExterior,            !- Solar Distribution",
        "    ,                        !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",

        "  ShadowCalculation,",
        "    PolygonClipping,         !- Shading Calculation Method",
        "    Timestep,                !- Shading Calculation Update Frequency Method",
        "    ,                        !- Shading Calculation Update Frequency",
        "    ,                        !- Maximum Figures in Shadow Overlap Calculations",
        "    ,                        !- Polygon Clipping Algorithm",
        "    ,                        !- Pixel Counting Resolution",
        "    DetailedSkyDiffuseModeling;!- Sky Diffuse Modeling Algorithm",

        "  SurfaceConvectionAlgorithm:Inside,TARP;",
        "  SurfaceConvectionAlgorithm:Outside,TARP;",
        "  HeatBalanceAlgorithm,ConductionTransferFunction;",

        "  Timestep,6;",

        "  RunPeriod,",
        "    RP1,                     !- Name",
        "    1,                       !- Begin Month",
        "    1,                       !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    12,                      !- End Month",
        "    31,                      !- End Day of Month",
        "    ,                        !- End Year",
        "    ,                        !- Day of Week for Start Day",
        "    ,                        !- Use Weather File Holidays and Special Days",
        "    ,                        !- Use Weather File Daylight Saving Period",
        "    ,                        !- Apply Weekend Holiday Rule",
        "    ,                        !- Use Weather File Rain Indicators",
        "    ;                        !- Use Weather File Snow Indicators",

        "  ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    Continuous;              !- Numeric Type",

        "  ScheduleTypeLimits,",
        "    ON/OFF,                  !- Name",
        "    0,                       !- Lower Limit Value",
        "    1,                       !- Upper Limit Value",
        "    Discrete;                !- Numeric Type",

        "  Schedule:Compact,",
        "    SunShading,              !- Name",
        "    ON/OFF,                  !- Schedule Type Limits Name",
        "    Through: 4/30,           !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    until: 24:00,1,          !- Field 3",
        "    Through: 10/31,          !- Field 5",
        "    For: AllDays,            !- Field 6",
        "    until: 24:00,0,          !- Field 7",
        "    Through: 12/31,          !- Field 9",
        "    For: AllDays,            !- Field 10",
        "    until: 24:00,1;          !- Field 11",

        "  Material,",
        "    A2 - 4 IN DENSE FACE BRICK,  !- Name",
        "    Rough,                   !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    1.245296,                !- Conductivity {W/m-K}",
        "    2082.400,                !- Density {kg/m3}",
        "    920.4800,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9300000,               !- Solar Absorptance",
        "    0.9300000;               !- Visible Absorptance",

        "  Material,",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD,  !- Name",
        "    Smooth,                  !- Roughness",
        "    1.9050000E-02,           !- Thickness {m}",
        "    0.7264224,               !- Conductivity {W/m-K}",
        "    1601.846,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Material,",
        "    E2 - 1 / 2 IN SLAG OR STONE,  !- Name",
        "    Rough,                   !- Roughness",
        "    1.2710161E-02,           !- Thickness {m}",
        "    1.435549,                !- Conductivity {W/m-K}",
        "    881.0155,                !- Density {kg/m3}",
        "    1673.600,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.5500000,               !- Solar Absorptance",
        "    0.5500000;               !- Visible Absorptance",

        "  Material,",
        "    C12 - 2 IN HW CONCRETE,  !- Name",
        "    MediumRough,             !- Roughness",
        "    5.0901599E-02,           !- Thickness {m}",
        "    1.729577,                !- Conductivity {W/m-K}",
        "    2242.585,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    R13LAYER,                !- Name",
        "    Rough,                   !- Roughness",
        "    2.290965,                !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  WindowMaterial:Glazing,",
        "    GLASS - CLEAR PLATE 1 / 4 IN,  !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.006,                   !- Thickness {m}",
        "    0.80,                    !- Solar Transmittance at Normal Incidence",
        "    0.10,                    !- Front Side Solar Reflectance at Normal Incidence",
        "    0.10,                    !- Back Side Solar Reflectance at Normal Incidence",
        "    0.80,                    !- Visible Transmittance at Normal Incidence",
        "    0.10,                    !- Front Side Visible Reflectance at Normal Incidence",
        "    0.10,                    !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.84,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",

        "  WindowMaterial:Gas,",
        "    AIRGAP,                  !- Name",
        "    AIR,                     !- Gas Type",
        "    0.0125;                  !- Thickness {m}",

        "  Construction,",
        "    R13WALL,                 !- Name",
        "    R13LAYER;                !- Outside Layer",

        "  Construction,",
        "    EXTWALL09,               !- Name",
        "    A2 - 4 IN DENSE FACE BRICK,  !- Outside Layer",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 4",

        "  Construction,",
        "    SLAB FLOOR,              !- Name",
        "    C12 - 2 IN HW CONCRETE;  !- Layer 4",

        "  Construction,",
        "    ROOF31,                  !- Name",
        "    E2 - 1 / 2 IN SLAG OR STONE,  !- Outside Layer",
        "    C12 - 2 IN HW CONCRETE;  !- Layer 4",

        "  Construction,",
        "    DOUBLE PANE HW WINDOW,   !- Name",
        "    GLASS - CLEAR PLATE 1 / 4 IN,  !- Outside Layer",
        "    AIRGAP,                  !- Layer 2",
        "    GLASS - CLEAR PLATE 1 / 4 IN;  !- Layer 3",

        "  Construction,",
        "    EXTWALLdemo,             !- Name",
        "    A2 - 4 IN DENSE FACE BRICK,  !- Outside Layer",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 4",

        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    Counterclockwise,        !- Vertex Entry Direction",
        "    Relative;                !- Coordinate System",

        "  Zone,",
        "    ZONE ONE,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    0,                       !- Ceiling Height {m}",
        "    0;                       !- Volume {m3}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-North,        !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALLdemo,             !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    5,5,3,                   !- X,Y,Z ==> Vertex 1 {m}",
        "    5,5,0,                   !- X,Y,Z ==> Vertex 2 {m}",
        "    -5,5,0,                  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,5,3;                  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-East,         !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL09,               !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    5,-5,3,                  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,-5,0,                  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,5,0,                   !- X,Y,Z ==> Vertex 3 {m}",
        "    5,5,3;                   !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-South,        !- Name",
        "    Wall,                    !- Surface Type",
        "    R13WALL,                 !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,-5,3,                 !- X,Y,Z ==> Vertex 1 {m}",
        "    -5,-5,0,                 !- X,Y,Z ==> Vertex 2 {m}",
        "    5,-5,0,                  !- X,Y,Z ==> Vertex 3 {m}",
        "    5,-5,3;                  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-West,         !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL09,               !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,5,3,                  !- X,Y,Z ==> Vertex 1 {m}",
        "    -5,5,0,                  !- X,Y,Z ==> Vertex 2 {m}",
        "    -5,-5,0,                 !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,-5,3;                 !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:roof,              !- Name",
        "    Roof,                    !- Surface Type",
        "    ROOF31,                  !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.0000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,-5,3,                 !- X,Y,Z ==> Vertex 1 {m}",
        "    5,-5,3,                  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,5,3,                   !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,5,3;                  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:floor,             !- Name",
        "    Floor,                   !- Surface Type",
        "    SLAB FLOOR,              !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.0000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,5,0,                  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,5,0,                   !- X,Y,Z ==> Vertex 2 {m}",
        "    5,-5,0,                  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,-5,0;                 !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    Zn001:Wall-South:Win001, !- Name",
        "    Window,                  !- Surface Type",
        "    DOUBLE PANE HW WINDOW,   !- Construction Name",
        "    Zn001:Wall-South,        !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    TestFrameAndDivider,     !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,               !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-5,0.5,               !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-5,0.5,                !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;                !- X,Y,Z ==> Vertex 4 {m}",

        "  WindowProperty:FrameAndDivider,",
        "    TestFrameAndDivider,     !- Name",
        "    0.05,                    !- Frame Width {m}",
        "    0.05,                    !- Frame Outside Projection {m}",
        "    0.05,                    !- Frame Inside Projection {m}",
        "    5.0,                     !- Frame Conductance {W/m2-K}",
        "    1.2,                     !- Ratio of Frame-Edge Glass Conductance to Center-Of-Gl",
        "    0.8,                     !- Frame Solar Absorptance",
        "    0.8,                     !- Frame Visible Absorptance",
        "    0.9,                     !- Frame Thermal Hemispherical Emissivity",
        "    DividedLite,             !- Divider Type",
        "    0.02,                    !- Divider Width {m}",
        "    2,                       !- Number of Horizontal Dividers",
        "    2,                       !- Number of Vertical Dividers",
        "    0.02,                    !- Divider Outside Projection {m}",
        "    0.02,                    !- Divider Inside Projection {m}",
        "    5.0,                     !- Divider Conductance {W/m2-K}",
        "    1.2,                     !- Ratio of Divider-Edge Glass Conductance to Center-Of-",
        "    0.8,                     !- Divider Solar Absorptance",
        "    0.8,                     !- Divider Visible Absorptance",
        "    0.9;                     !- Divider Thermal Hemispherical Emissivity",

        "  Shading:Zone:Detailed,",
        "    Zn001:Wall-South:Shade001,  !- Name",
        "    Zn001:Wall-South,        !- Base Surface Name",
        "    SunShading,              !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,               !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-6,2.5,               !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-6,2.5,                !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;                !- X,Y,Z ==> Vertex 4 {m}",

        "  ShadingProperty:Reflectance,",
        "    Zn001:Wall-South:Shade001,  !- Shading Surface Name",
        "    0.2,                     !- Diffuse Solar Reflectance of Unglazed Part of Shading",
        "    0.2;                     !- Diffuse Visible Reflectance of Unglazed Part of Shading",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool FoundError = false;

    HeatBalanceManager::GetProjectControlData(*state, FoundError); // read project control data
    EXPECT_FALSE(FoundError);                                      // expect no errors

    HeatBalanceManager::SetPreConstructionInputParameters(*state);

    Material::GetMaterialData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    HeatBalanceManager::GetFrameAndDividerData(*state);

    HeatBalanceManager::GetConstructData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    HeatBalanceManager::GetZoneData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    SurfaceGeometry::GetGeometryParameters(*state, FoundError);
    EXPECT_FALSE(FoundError);

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(1);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(1);

    state->dataSurfaceGeometry->CosZoneRelNorth(1) = std::cos(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(1) = std::sin(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;

    SurfaceGeometry::GetSurfaceData(*state, FoundError);
    EXPECT_FALSE(FoundError);
    SurfaceGeometry::SetupZoneGeometry(*state, FoundError);
    EXPECT_FALSE(FoundError);

    SolarShading::AllocateModuleArrays(*state);
    SolarShading::DetermineShadowingCombinations(*state);
    state->dataEnvrn->DayOfYear_Schedule = 168;
    state->dataEnvrn->DayOfWeek = 6;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->HourOfDay = 9;

    state->dataSurface->ShadingTransmittanceVaries = true;
    state->dataSysVars->DetailedSkyDiffuseAlgorithm = true;
    state->dataHeatBal->SolarDistribution = DataHeatBalance::Shadowing::FullExterior;

    state->dataSolarShading->CalcSkyDifShading = true;
    SolarShading::InitSolarCalculations(*state);
    SolarShading::SkyDifSolarShading(*state);

    int windowSurfNum = Util::FindItemInList("ZN001:WALL-SOUTH:WIN001", state->dataSurface->Surface);
    auto &win_Surface = state->dataSurface->Surface(windowSurfNum);
    // check exterior default surfaces sky and ground view factors
    EXPECT_DOUBLE_EQ(0.5, win_Surface.ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.5, win_Surface.ViewFactorGroundIR);

    // add surface property object and test view factors change
    state->dataGlobal->AnyLocalEnvironmentsInModel = true;
    state->dataSurface->SurroundingSurfsProperty.allocate(1);
    auto &SrdSurfsProperty = state->dataSurface->SurroundingSurfsProperty(1);
    SrdSurfsProperty.TotSurroundingSurface = 1;
    SrdSurfsProperty.SurroundingSurfs.allocate(1);
    SrdSurfsProperty.SurroundingSurfs(1).ViewFactor = 0.2;
    SrdSurfsProperty.SurfsViewFactorSum = SrdSurfsProperty.SurroundingSurfs(1).ViewFactor;
    SrdSurfsProperty.SkyViewFactor = 0.0;
    SrdSurfsProperty.IsSkyViewFactorSet = false;
    SrdSurfsProperty.GroundViewFactor = 0.0;
    SrdSurfsProperty.IsGroundViewFactorSet = false;

    win_Surface.IsSurfPropertyGndSurfacesDefined = true;
    win_Surface.SurfPropertyGndSurfIndex = 1;
    win_Surface.UseSurfPropertyGndSurfTemp = true;
    win_Surface.UseSurfPropertyGndSurfRefl = true;
    win_Surface.SurfHasSurroundingSurfProperty = true;
    win_Surface.SurfSurroundingSurfacesNum = 1;
    win_Surface.ViewFactorSrdSurfs = SrdSurfsProperty.SurfsViewFactorSum;

    state->dataSurface->GroundSurfsProperty.allocate(1);
    auto &GndSurfsProperty = state->dataSurface->GroundSurfsProperty(1);
    state->dataSurface->TotSurfPropGndSurfs = 1;
    GndSurfsProperty.GndSurfs.allocate(1);
    GndSurfsProperty.NumGndSurfs = 1;
    GndSurfsProperty.GndSurfs(1).ViewFactor = 0.0;
    GndSurfsProperty.IsGroundViewFactorSet = false;

    // reset sky and ground view factors
    HeatBalanceSurfaceManager::InitSurfacePropertyViewFactors(*state);
    // check surface property sky and ground view factors
    EXPECT_DOUBLE_EQ(0.4, SrdSurfsProperty.SkyViewFactor);
    EXPECT_TRUE(SrdSurfsProperty.IsSkyViewFactorSet);
    EXPECT_DOUBLE_EQ(0.0, SrdSurfsProperty.GroundViewFactor);
    EXPECT_FALSE(SrdSurfsProperty.IsGroundViewFactorSet);
    Real64 results_Surface_SkyViewFactor = 0.0;
    Real64 results_Surface_GndViewFactor = 0.0;
    // check exterior surfaces sky and ground view factors
    results_Surface_SkyViewFactor = 0.5 * (1 - 0.2);
    results_Surface_GndViewFactor = 0.5 * (1 - 0.2);
    EXPECT_DOUBLE_EQ(results_Surface_SkyViewFactor, win_Surface.ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(results_Surface_GndViewFactor, win_Surface.ViewFactorGroundIR);
    // run SkyDifSolarShading function
    SolarShading::SkyDifSolarShading(*state);
    state->dataSolarShading->CalcSkyDifShading = false;
    // test exterior surface sky and ground view factors
    EXPECT_DOUBLE_EQ(0.40, win_Surface.ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.40, win_Surface.ViewFactorGroundIR);
}

TEST_F(EnergyPlusFixture, SolarShadingTest_CTRANS)
{
    //    state->dataSurface->Surface.allocate(1);
    //    state->dataSurface->Surface(1).Sides = 4;
    //
    //    state->dataSurface->Surface(1).Vertex.allocate(4);
    //    state->dataSurface->Surface(1).Vertex(1) = Vector(+7.0711, +12.2474, +0.0000);
    //    state->dataSurface->Surface(1).Vertex(2) = Vector(-12.2474, +7.0711, +0.0000);
    //    state->dataSurface->Surface(1).Vertex(3) = Vector(-7.0711, -12.2474, +0.0000);
    //    state->dataSurface->Surface(1).Vertex(4) = Vector(+12.2474, -7.0711, +0.0000);

    // Test for #9432
    // So this is a perfect square. Expect I am going to rotate the building by 15 degrees (Not by building north axis, but by vertices adjustments)
    // Surface.Vertex(1) will end up having the highest X and highest Y, and we will trigger a bug where if will fail
    // the XdYu ones (lo X, high Y), that is the top left corner [2] because  the RoofGeo BoundingBoxVertStruct are initialized to Vertex(1) and:
    //     vertex.x <= RoofGeo.XdYuZd.Vertex.x => True
    //     vertex.y => RoofGeo.XdYuZd.Vertex.y => False
    //
    //          y
    //         ▲      ◄──┐ Rotate by 15 degrees
    //       10│         │
    // [2]┌────┼────┐[1]
    //    │    │    │
    //    │    │    │
    //  ──┼────┼────┼────►
    // -10│    │    │10   x
    //    │    │    │
    // [3]└────┼────┘[4]
    //      -10│

    std::string const idf_objects = delimited_string({

        "GlobalGeometryRules,",
        "  UpperLeftCorner,                        !- Starting Vertex Position",
        "  Counterclockwise,                       !- Vertex Entry Direction",
        "  Relative,                               !- Coordinate System",
        "  Relative,                               !- Daylighting Reference Point Coordinate System",
        "  Relative;                               !- Rectangular Surface Coordinate System",

        "Building,",
        "  Building 1,                             !- Name",
        "  20,                                     !- North Axis {deg}",
        "  ,                                       !- Terrain",
        "  ,                                       !- Loads Convergence Tolerance Value {W}",
        "  ,                                       !- Temperature Convergence Tolerance Value {deltaC}",
        "  ,                                       !- Solar Distribution",
        "  ,                                       !- Maximum Number of Warmup Days",
        "  ;                                       !- Minimum Number of Warmup Days",

        "Zone,",
        "  Zone1,                                  !- Name",
        "  0,                                      !- Direction of Relative North {deg}",
        "  0,                                      !- X Origin {m}",
        "  0,                                      !- Y Origin {m}",
        "  0,                                      !- Z Origin {m}",
        "  ,                                       !- Type",
        "  1,                                      !- Multiplier",
        "  ,                                       !- Ceiling Height {m}",
        "  ,                                       !- Volume {m3}",
        "  ,                                       !- Floor Area {m2}",
        "  ,                                       !- Zone Inside Convection Algorithm",
        "  ,                                       !- Zone Outside Convection Algorithm",
        "  Yes;                                    !- Part of Total Floor Area",

        "Material:NoMass,",
        "  R13-IP,                                 !- Name",
        "  Smooth,                                 !- Roughness",
        "  2.28943238786998,                       !- Thermal Resistance {m2-K/W}",
        "  0.9,                                    !- Thermal Absorptance",
        "  0.7,                                    !- Solar Absorptance",
        "  0.7;                                    !- Visible Absorptance",

        "Construction,",
        "  R13 Construction,                       !- Name",
        "  R13-IP;                                 !- Layer 1",

        "BuildingSurface:Detailed,",
        "  1-SOUTH - ABS AZIMUTH 165.00,           !- Name",
        "  Wall,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  -7.07106781186548, -12.2474487139159, 3, !- X,Y,Z Vertex 1 {m}",
        "  -7.07106781186548, -12.2474487139159, 0, !- X,Y,Z Vertex 2 {m}",
        "  12.2474487139159, -7.07106781186548, 0, !- X,Y,Z Vertex 3 {m}",
        "  12.2474487139159, -7.07106781186548, 3; !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  2-WEST - ABS AZIMUTH 255.00,            !- Name",
        "  Wall,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  -12.2474487139159, 7.07106781186548, 3, !- X,Y,Z Vertex 1 {m}",
        "  -12.2474487139159, 7.07106781186548, 0, !- X,Y,Z Vertex 2 {m}",
        "  -7.07106781186548, -12.2474487139159, 0, !- X,Y,Z Vertex 3 {m}",
        "  -7.07106781186548, -12.2474487139159, 3; !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  3-EAST - ABS AZIMUTH 75.00,             !- Name",
        "  Wall,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  12.2474487139159, -7.07106781186548, 3, !- X,Y,Z Vertex 1 {m}",
        "  12.2474487139159, -7.07106781186548, 0, !- X,Y,Z Vertex 2 {m}",
        "  7.07106781186548, 12.2474487139159, 0,  !- X,Y,Z Vertex 3 {m}",
        "  7.07106781186548, 12.2474487139159, 3;  !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  4-NORTH - ABS AZIMUTH 345.00,           !- Name",
        "  Wall,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  7.07106781186548, 12.2474487139159, 3,  !- X,Y,Z Vertex 1 {m}",
        "  7.07106781186548, 12.2474487139159, 0,  !- X,Y,Z Vertex 2 {m}",
        "  -12.2474487139159, 7.07106781186548, 0, !- X,Y,Z Vertex 3 {m}",
        "  -12.2474487139159, 7.07106781186548, 3; !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  FLOOR,                                  !- Name",
        "  Floor,                                  !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Ground,                                 !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  NoSun,                                  !- Sun Exposure",
        "  NoWind,                                 !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  12.2474487139159, -7.07106781186548, 0, !- X,Y,Z Vertex 1 {m}",
        "  -7.07106781186548, -12.2474487139159, 0, !- X,Y,Z Vertex 2 {m}",
        "  -12.2474487139159, 7.07106781186548, 0, !- X,Y,Z Vertex 3 {m}",
        "  7.07106781186548, 12.2474487139159, 0;  !- X,Y,Z Vertex 4 {m}",

        "BuildingSurface:Detailed,",
        "  ROOF,                                   !- Name",
        "  Roof,                                   !- Surface Type",
        "  R13 Construction,                       !- Construction Name",
        "  Zone1,                                  !- Zone Name",
        "  ,                                       !- Space Name",
        "  Outdoors,                               !- Outside Boundary Condition",
        "  ,                                       !- Outside Boundary Condition Object",
        "  SunExposed,                             !- Sun Exposure",
        "  WindExposed,                            !- Wind Exposure",
        "  ,                                       !- View Factor to Ground",
        "  ,                                       !- Number of Vertices",
        "  7.07106781186548, 12.2474487139159, 3,  !- X,Y,Z Vertex 1 {m}",
        "  -12.2474487139159, 7.07106781186548, 3, !- X,Y,Z Vertex 2 {m}",
        "  -7.07106781186548, -12.2474487139159, 3, !- X,Y,Z Vertex 3 {m}",
        "  12.2474487139159, -7.07106781186548, 3; !- X,Y,Z Vertex 4 {m}",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound = false;

    Material::GetMaterialData(*state, ErrorsFound); // read material data
    EXPECT_FALSE(ErrorsFound);                      // expect no errors

    HeatBalanceManager::GetConstructData(*state, ErrorsFound); // read construction data
    EXPECT_FALSE(ErrorsFound);                                 // expect no errors

    HeatBalanceManager::GetZoneData(*state, ErrorsFound); // read zone data
    EXPECT_FALSE(ErrorsFound);                            // expect no errors

    // TODO: I think GetSurfaceData is enough? SetupZoneGeometry => GetSurfaceData => CalcSurfaceCentroid
    SurfaceGeometry::SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound); // expect no errors

    const auto &surfaces = state->dataSurface->Surface;
    const auto it = std::find_if(surfaces.begin(), surfaces.end(), [](const auto &s) { return s.Name == "ROOF"; });
    ASSERT_NE(it, surfaces.end());
    const auto &surface = *it;
    const int surfNum = std::distance(surfaces.begin(), it) + 1;
    EXPECT_EQ(6, surfNum);
    EXPECT_DOUBLE_EQ(-12.247448713915899, state->dataSurface->X0(surfNum));
    EXPECT_DOUBLE_EQ(7.0710678118654799, state->dataSurface->Y0(surfNum));
    EXPECT_DOUBLE_EQ(3.0, state->dataSurface->Z0(surfNum));

    const auto &vertices = surface.Vertex;
    std::vector<Vector> expectedOriVertices{
        {+7.0711, +12.2474, +3.0000},
        {-12.2474, +7.0711, +3.0000},
        {-7.0711, -12.2474, +3.0000},
        {+12.2474, -7.0711, +3.0000},
    };
    for (size_t i = 0; i < 4; ++i) {

        EXPECT_TRUE(SurfaceGeometry::isAlmostEqual3dPt(expectedOriVertices[i], vertices[i]))
            << "Failed for vertice " << i << ", expected=" << expectedOriVertices[i] << ", got=" << vertices[i];
    }

    auto const &lcsx(surface.lcsx);
    auto const &lcsy(surface.lcsy);
    auto const &lcsz(surface.lcsz);

    Vector expected_lcsx(0.25881904510252079, -0.96592582628906831, 0.0);
    Vector expected_lcsy(0.96592582628906831, 0.25881904510252079, -0.0);
    Vector expected_lcsz(0.0, 0.0, 1.0);

    auto compare_double_vectors = [](Vector const &exp, Vector const &actual) {
        ASSERT_EQ(exp.size(), actual.size());
        for (size_t i = 0; i < exp.size(); ++i) {
            EXPECT_NEAR(exp[i], actual[i], 0.0000000000000001) << "Failed at index " << i;
        }
    };

    compare_double_vectors(expected_lcsx, lcsx);
    compare_double_vectors(expected_lcsy, lcsy);
    compare_double_vectors(expected_lcsz, lcsz);

    Array1D<Real64> xs(4, 0.0);
    Array1D<Real64> ys(4, 0.0);
    Array1D<Real64> zs(4, 0.0);

    int const NS = surfNum;   // Surface number whose vertex coordinates are being transformed
    int const NGRS = surfNum; // Base surface number for surface NS
    int NVT = 4;              // Number of vertices for surface NS

    // I'm projecting the surface onto itself...
    SolarShading::CTRANS(*state, NS, NGRS, NVT, xs, ys, zs);

    double expected_perimeter = 20.0 * 4;
    EXPECT_DOUBLE_EQ(expected_perimeter, surface.Perimeter);

    std::vector<Vector> transformedVertices;
    for (int i = 1; i <= 4; ++i) {
        transformedVertices.emplace_back(xs(i), ys(i), zs(i));
    }
    double perimeter = 0.0;
    for (auto it = transformedVertices.begin(); it != transformedVertices.end(); ++it) {

        auto itnext = std::next(it);
        if (itnext == std::end(transformedVertices)) {
            itnext = std::begin(transformedVertices);
        }
        perimeter += SurfaceGeometry::distance(*it, *itnext);
    }
    EXPECT_DOUBLE_EQ(expected_perimeter, perimeter);

    std::vector<Vector> expectedVertices{
        {+0.0000, +20.0000, +0.0000},
        {+0.0000, +0.0000, +0.0000},
        {+20.0000, +0.0000, +0.0000},
        {+20.0000, +20.0000, +0.0000},
    };
    for (size_t i = 0; i < 4; ++i) {
        EXPECT_TRUE(SurfaceGeometry::isAlmostEqual3dPt(expectedVertices[i], transformedVertices[i])) << "Failed for vertice " << i;
    }
}

TEST_F(EnergyPlusFixture, SolarShadingTest_Warn_Pixel_Count_and_TM_Schedule)
{
    // Test the severe warning messages given when there is a combination of PixelCounting type Shadow Calculation Method
    // and schedule shading surface transmittance values
    // Test the addressed Issue 9059 via PR 9653
    std::string const idf_objects = delimited_string({
        "  Building,",
        "    DemoFDT,                 !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    3.9999999E-02,           !- Loads Convergence Tolerance Value",
        "    4.0000002E-03,           !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullExterior,            !- Solar Distribution",
        "    ,                        !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",

        "  ShadowCalculation,",
        "    PixelCounting, !-Shading Calculation Method",
        "    Timestep, !-Shading Calculation Update Frequency Method",
        "    , !-Shading Calculation Update Frequency",
        "    , !-Maximum Figures in Shadow Overlap Calculations",
        "    , !-Polygon Clipping Algorithm",
        "    2048, !- Pixel Counting Resolution",
        "    DetailedSkyDiffuseModeling;!- Sky Diffuse Modeling Algorithm",

        "  SurfaceConvectionAlgorithm:Inside,TARP;",
        "  SurfaceConvectionAlgorithm:Outside,TARP;",
        "  HeatBalanceAlgorithm,ConductionTransferFunction;",

        "  Timestep,6;",

        "  RunPeriod,",
        "    RP1,                     !- Name",
        "    1,                       !- Begin Month",
        "    1,                       !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    12,                      !- End Month",
        "    31,                      !- End Day of Month",
        "    ,                        !- End Year",
        "    ,                        !- Day of Week for Start Day",
        "    ,                        !- Use Weather File Holidays and Special Days",
        "    ,                        !- Use Weather File Daylight Saving Period",
        "    ,                        !- Apply Weekend Holiday Rule",
        "    ,                        !- Use Weather File Rain Indicators",
        "    ;                        !- Use Weather File Snow Indicators",

        "  ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    Continuous;              !- Numeric Type",

        "  ScheduleTypeLimits,",
        "    ON/OFF,                  !- Name",
        "    0,                       !- Lower Limit Value",
        "    1,                       !- Upper Limit Value",
        "    Discrete;                !- Numeric Type",

        "  Schedule:Compact,",
        "    SunShading,              !- Name",
        "    ON/OFF,                  !- Schedule Type Limits Name",
        "    Through: 4/30,           !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    until: 24:00,1,          !- Field 3",
        "    Through: 10/31,          !- Field 5",
        "    For: AllDays,            !- Field 6",
        "    until: 24:00,0,          !- Field 7",
        "    Through: 12/31,          !- Field 9",
        "    For: AllDays,            !- Field 10",
        "    until: 24:00,1;          !- Field 11",

        "  Material,",
        "    A2 - 4 IN DENSE FACE BRICK,  !- Name",
        "    Rough,                   !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    1.245296,                !- Conductivity {W/m-K}",
        "    2082.400,                !- Density {kg/m3}",
        "    920.4800,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9300000,               !- Solar Absorptance",
        "    0.9300000;               !- Visible Absorptance",

        "  Material,",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD,  !- Name",
        "    Smooth,                  !- Roughness",
        "    1.9050000E-02,           !- Thickness {m}",
        "    0.7264224,               !- Conductivity {W/m-K}",
        "    1601.846,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Material,",
        "    E2 - 1 / 2 IN SLAG OR STONE,  !- Name",
        "    Rough,                   !- Roughness",
        "    1.2710161E-02,           !- Thickness {m}",
        "    1.435549,                !- Conductivity {W/m-K}",
        "    881.0155,                !- Density {kg/m3}",
        "    1673.600,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.5500000,               !- Solar Absorptance",
        "    0.5500000;               !- Visible Absorptance",

        "  Material,",
        "    C12 - 2 IN HW CONCRETE,  !- Name",
        "    MediumRough,             !- Roughness",
        "    5.0901599E-02,           !- Thickness {m}",
        "    1.729577,                !- Conductivity {W/m-K}",
        "    2242.585,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    R13LAYER,                !- Name",
        "    Rough,                   !- Roughness",
        "    2.290965,                !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  WindowMaterial:Glazing,",
        "    GLASS - CLEAR PLATE 1 / 4 IN,  !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.006,                   !- Thickness {m}",
        "    0.80,                    !- Solar Transmittance at Normal Incidence",
        "    0.10,                    !- Front Side Solar Reflectance at Normal Incidence",
        "    0.10,                    !- Back Side Solar Reflectance at Normal Incidence",
        "    0.80,                    !- Visible Transmittance at Normal Incidence",
        "    0.10,                    !- Front Side Visible Reflectance at Normal Incidence",
        "    0.10,                    !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.84,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",

        "  WindowMaterial:Gas,",
        "    AIRGAP,                  !- Name",
        "    AIR,                     !- Gas Type",
        "    0.0125;                  !- Thickness {m}",

        "  Construction,",
        "    R13WALL,                 !- Name",
        "    R13LAYER;                !- Outside Layer",

        "  Construction,",
        "    EXTWALL09,               !- Name",
        "    A2 - 4 IN DENSE FACE BRICK,  !- Outside Layer",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 4",

        "  Construction,",
        "    SLAB FLOOR,              !- Name",
        "    C12 - 2 IN HW CONCRETE;  !- Layer 4",

        "  Construction,",
        "    ROOF31,                  !- Name",
        "    E2 - 1 / 2 IN SLAG OR STONE,  !- Outside Layer",
        "    C12 - 2 IN HW CONCRETE;  !- Layer 4",

        "  Construction,",
        "    DOUBLE PANE HW WINDOW,   !- Name",
        "    GLASS - CLEAR PLATE 1 / 4 IN,  !- Outside Layer",
        "    AIRGAP,                  !- Layer 2",
        "    GLASS - CLEAR PLATE 1 / 4 IN;  !- Layer 3",

        "  Construction,",
        "    EXTWALLdemo,             !- Name",
        "    A2 - 4 IN DENSE FACE BRICK,  !- Outside Layer",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 4",

        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    Counterclockwise,        !- Vertex Entry Direction",
        "    Relative;                !- Coordinate System",

        "  Zone,",
        "    ZONE ONE,                !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    0,                       !- Ceiling Height {m}",
        "    0;                       !- Volume {m3}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-North,        !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALLdemo,             !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    5,5,3,                   !- X,Y,Z ==> Vertex 1 {m}",
        "    5,5,0,                   !- X,Y,Z ==> Vertex 2 {m}",
        "    -5,5,0,                  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,5,3;                  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-East,         !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL09,               !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    5,-5,3,                  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,-5,0,                  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,5,0,                   !- X,Y,Z ==> Vertex 3 {m}",
        "    5,5,3;                   !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-South,        !- Name",
        "    Wall,                    !- Surface Type",
        "    R13WALL,                 !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,-5,3,                 !- X,Y,Z ==> Vertex 1 {m}",
        "    -5,-5,0,                 !- X,Y,Z ==> Vertex 2 {m}",
        "    5,-5,0,                  !- X,Y,Z ==> Vertex 3 {m}",
        "    5,-5,3;                  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-West,         !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL09,               !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,5,3,                  !- X,Y,Z ==> Vertex 1 {m}",
        "    -5,5,0,                  !- X,Y,Z ==> Vertex 2 {m}",
        "    -5,-5,0,                 !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,-5,3;                 !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:roof,              !- Name",
        "    Roof,                    !- Surface Type",
        "    ROOF31,                  !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.0000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,-5,3,                 !- X,Y,Z ==> Vertex 1 {m}",
        "    5,-5,3,                  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,5,3,                   !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,5,3;                  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Zn001:floor,             !- Name",
        "    Floor,                   !- Surface Type",
        "    SLAB FLOOR,              !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.0000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,5,0,                  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,5,0,                   !- X,Y,Z ==> Vertex 2 {m}",
        "    5,-5,0,                  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,-5,0;                 !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    Zn001:Wall-South:Win001, !- Name",
        "    Window,                  !- Surface Type",
        "    DOUBLE PANE HW WINDOW,   !- Construction Name",
        "    Zn001:Wall-South,        !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    TestFrameAndDivider,     !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,               !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-5,0.5,               !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-5,0.5,                !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;                !- X,Y,Z ==> Vertex 4 {m}",

        "  WindowProperty:FrameAndDivider,",
        "    TestFrameAndDivider,     !- Name",
        "    0.05,                    !- Frame Width {m}",
        "    0.05,                    !- Frame Outside Projection {m}",
        "    0.05,                    !- Frame Inside Projection {m}",
        "    5.0,                     !- Frame Conductance {W/m2-K}",
        "    1.2,                     !- Ratio of Frame-Edge Glass Conductance to Center-Of-Gl",
        "    0.8,                     !- Frame Solar Absorptance",
        "    0.8,                     !- Frame Visible Absorptance",
        "    0.9,                     !- Frame Thermal Hemispherical Emissivity",
        "    DividedLite,             !- Divider Type",
        "    0.02,                    !- Divider Width {m}",
        "    2,                       !- Number of Horizontal Dividers",
        "    2,                       !- Number of Vertical Dividers",
        "    0.02,                    !- Divider Outside Projection {m}",
        "    0.02,                    !- Divider Inside Projection {m}",
        "    5.0,                     !- Divider Conductance {W/m2-K}",
        "    1.2,                     !- Ratio of Divider-Edge Glass Conductance to Center-Of-",
        "    0.8,                     !- Divider Solar Absorptance",
        "    0.8,                     !- Divider Visible Absorptance",
        "    0.9;                     !- Divider Thermal Hemispherical Emissivity",

        "  Shading:Zone:Detailed,",
        "    Zn001:Wall-South:Shade001,  !- Name",
        "    Zn001:Wall-South,        !- Base Surface Name",
        "    SunShading,              !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,               !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-6,2.5,               !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-6,2.5,                !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;                !- X,Y,Z ==> Vertex 4 {m}",

        "  ShadingProperty:Reflectance,",
        "    Zn001:Wall-South:Shade001,  !- Shading Surface Name",
        "    0.2,                     !- Diffuse Solar Reflectance of Unglazed Part of Shading",
        "    0.2;                     !- Diffuse Visible Reflectance of Unglazed Part of Shading",

        "  Shading:Site:Detailed,",
        "    Zn001:Wall-South:Shade002,  !- Name",
        "    SunShading,              !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    -3,-5,5,               !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-6,5,               !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-6,5,                !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,5;                !- X,Y,Z ==> Vertex 4 {m}",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool FoundError = false;

    HeatBalanceManager::GetProjectControlData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    HeatBalanceManager::SetPreConstructionInputParameters(*state);

    Material::GetMaterialData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    HeatBalanceManager::GetFrameAndDividerData(*state);

    HeatBalanceManager::GetConstructData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    SolarShading::GetShadowingInput(*state);

    HeatBalanceManager::GetZoneData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    SurfaceGeometry::GetGeometryParameters(*state, FoundError);
    EXPECT_FALSE(FoundError);

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(1);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(1);

    state->dataSurfaceGeometry->CosZoneRelNorth(1) = std::cos(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(1) = std::sin(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;

    // This function will call SurfaceGeometry::GetDetShdSurfaceData() and SurfaceGeometry::GetAttShdSurfaceData(),
    // which are the target functions to be tested.
    SurfaceGeometry::GetSurfaceData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    EXPECT_EQ(state->dataSolarShading->anyScheduledShadingSurface, true);

#ifdef EP_NO_OPENGL
    EXPECT_EQ(state->dataErrTracking->AskForSurfacesReport, true);
    EXPECT_EQ(state->dataErrTracking->TotalWarningErrors, 2);
    EXPECT_EQ(state->dataErrTracking->TotalSevereErrors, 0);
#else
    if (!Penumbra::Penumbra::is_valid_context()) {
        EXPECT_EQ(state->dataErrTracking->AskForSurfacesReport, true);
        EXPECT_EQ(state->dataErrTracking->TotalWarningErrors, 2);
        EXPECT_EQ(state->dataErrTracking->TotalSevereErrors, 0);
    } else {
        EXPECT_EQ(state->dataErrTracking->AskForSurfacesReport, true);
        EXPECT_EQ(state->dataErrTracking->TotalWarningErrors, 1);
        EXPECT_EQ(state->dataErrTracking->TotalSevereErrors, 0);
    }
#endif

    SolarShading::processShadowingInput(*state);

#ifdef EP_NO_OPENGL
    EXPECT_EQ(state->dataErrTracking->TotalWarningErrors, 2);
    EXPECT_EQ(state->dataErrTracking->TotalSevereErrors, 0);
    EXPECT_EQ(state->dataErrTracking->LastSevereError, "");
#else
    if (!Penumbra::Penumbra::is_valid_context()) {
        EXPECT_EQ(state->dataErrTracking->TotalWarningErrors, 2);
        EXPECT_EQ(state->dataErrTracking->TotalSevereErrors, 0);
        EXPECT_EQ(state->dataErrTracking->LastSevereError, "");
    } else {
        EXPECT_EQ(state->dataErrTracking->TotalWarningErrors, 1);
        EXPECT_EQ(state->dataErrTracking->TotalSevereErrors, 1);
        EXPECT_EQ(state->dataErrTracking->LastSevereError, "The Shading Calculation Method of choice is \"PixelCounting\"; ");
    }
#endif
}

TEST_F(EnergyPlusFixture, ShadowCalculation_CSV)
{

    // Test for #9753
    state->dataSurface->TotSurfaces = 2;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);
    state->dataSurface->Surface(1).Name = "ZN001:WALL001";
    state->dataSurface->Surface(1).Class = DataSurfaces::SurfaceClass::Wall;
    state->dataSurface->Surface(1).Construction = 1;
    state->dataSurface->Surface(2).Name = "ZN001:WALL002";
    state->dataSurface->Surface(2).Class = DataSurfaces::SurfaceClass::Wall;
    state->dataSurface->Surface(2).Construction = 1;

    state->dataSurface->SurfaceWindow.allocate(state->dataSurface->TotSurfaces);

    state->files.shade.open_as_stringstream();

    HeatBalanceManager::OpenShadingFile(*state);

    std::string expected_values = "Surface Name,ZN001:WALL001,ZN001:WALL002,\n";
    {
        auto const stream_str = state->files.shade.get_output();
        EXPECT_EQ(expected_values, stream_str);
    }
    // reset
    state->files.shade.open_as_stringstream();

    int constexpr NumTimeSteps(2);
    int constexpr HoursInDay(24);

    state->dataGlobal->BeginSimFlag = true;
    state->dataGlobal->DoWeathSim = true;
    state->dataSysVars->ReportExtShadingSunlitFrac = true;

    state->dataGlobal->BeginDayFlag = true;
    state->dataGlobal->WarmupFlag = false;
    state->dataGlobal->KindOfSim = Constant::KindOfSim::RunPeriodWeather;
    state->dataGlobal->TimeStepsInHour = NumTimeSteps;
    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 25;

    state->dataHeatBal->SurfSunlitFrac.allocate(HoursInDay, NumTimeSteps, state->dataSurface->TotSurfaces);

    for (int iHour = 1; iHour <= 24; ++iHour) { // Do for all hours.
        for (int TS = 1; TS <= state->dataGlobal->TimeStepsInHour; ++TS) {
            if (TS == state->dataGlobal->TimeStepsInHour) {
                expected_values += fmt::format(" 01/25 {:02}:00,", iHour);
            } else {
                expected_values += fmt::format(" 01/25 {:02}:30,", iHour - 1);
            }

            for (int SurfNum = 1; SurfNum <= state->dataSurface->TotSurfaces; ++SurfNum) {
                expected_values += fmt::format("{:10.8F},", 0.0);
            }
            expected_values += "\n";
        }
    }

    state->dataHeatBal->TotConstructs = 2;
    state->dataConstruction->Construct.allocate(state->dataHeatBal->TotConstructs);
    state->dataConstruction->Construct(1).TotLayers = 1;
    state->dataConstruction->Construct(1).TypeIsWindow = false;
    state->dataSurface->SurfWinSolarDiffusing.allocate(state->dataSurface->TotSurfaces);
    SurfaceGeometry::AllocateSurfaceWindows(*state, state->dataSurface->TotSurfaces);
    Window::initWindowModel(*state);

    state->dataSolarShading->GetInputFlag = false;
    state->dataSolarShading->ShadowingCalcFrequency = 20; // Default IDD
    HeatBalanceManager::InitHeatBalance(*state);

    {
        auto const stream_str = state->files.shade.get_output();
        EXPECT_EQ(expected_values, stream_str);
    }
}

TEST_F(EnergyPlusFixture, SolarShadingTest_PolygonOverlap)
{
    std::string const idf_objects = delimited_string({
        "  Building,",
        "    DemoFDT,                 !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    3.9999999E-02,           !- Loads Convergence Tolerance Value",
        "    4.0000002E-03,           !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullExterior;            !- Solar Distribution",
        "  ShadowCalculation,",
        "    PolygonClipping,         !- Shading Calculation Method",
        "    Timestep,                !- Shading Calculation Update Frequency Method",
        "    ,                        !- Shading Calculation Update Frequency",
        "    ,                        !- Maximum Figures in Shadow Overlap Calculations",
        "    ,                        !- Polygon Clipping Algorithm",
        "    ,                        !- Pixel Counting Resolution",
        "    DetailedSkyDiffuseModeling;  !- Sky Diffuse Modeling Algorithm",
        "  Timestep,6;",
        "  Site:Location,",
        "    CHICAGO_IL_USA TMY2-94846,  !- Name",
        "    42.00,                   !- Latitude {deg}",
        "    -87.88,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190.00;                  !- Elevation {m}",
        "  Material,",
        "    C12 - 2 IN HW CONCRETE,  !- Name",
        "    MediumRough,             !- Roughness",
        "    5.0901599E-02,           !- Thickness {m}",
        "    1.729577,                !- Conductivity {W/m-K}",
        "    2242.585,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",
        "  WindowMaterial:Glazing,",
        "    GLASS - CLEAR PLATE 1 / 4 IN,  !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.006,                   !- Thickness {m}",
        "    0.80,                    !- Solar Transmittance at Normal Incidence",
        "    0.10,                    !- Front Side Solar Reflectance at Normal Incidence",
        "    0.10,                    !- Back Side Solar Reflectance at Normal Incidence",
        "    0.80,                    !- Visible Transmittance at Normal Incidence",
        "    0.10,                    !- Front Side Visible Reflectance at Normal Incidence",
        "    0.10,                    !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.84,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",
        "  Construction,",
        "    Surfaces,                !- Name",
        "    C12 - 2 IN HW CONCRETE;  !- Layer 4",
        "  Construction,",
        "    Single PANE HW WINDOW,   !- Name",
        "    GLASS - CLEAR PLATE 1 / 4 IN;  !- Outside Layer",
        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    Counterclockwise,        !- Vertex Entry Direction",
        "    Relative;                !- Coordinate System",
        "  Zone,",
        "    ZONE ONE;                !- Name",
        "  FenestrationSurface:Detailed,",
        "    Zn001:Wall-South:Win001, !- Name",
        "    Window,                  !- Surface Type",
        "    Single PANE HW WINDOW,   !- Construction Name",
        "    Zn001:Wall-South,        !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    TestFrameAndDivider,     !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-5,0.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-5,0.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;  !- X,Y,Z ==> Vertex 4 {m}",
        "  WindowProperty:FrameAndDivider,",
        "    TestFrameAndDivider,     !- Name",
        "    0.05,                    !- Frame Width {m}",
        "    0.05,                    !- Frame Outside Projection {m}",
        "    0.05,                    !- Frame Inside Projection {m}",
        "    5.0,                     !- Frame Conductance {W/m2-K}",
        "    1.2,                     !- Ratio of Frame-Edge Glass Conductance to Center-Of-Gl",
        "    0.8,                     !- Frame Solar Absorptance",
        "    0.8,                     !- Frame Visible Absorptance",
        "    0.9,                     !- Frame Thermal Hemispherical Emissivity",
        "    DividedLite,             !- Divider Type",
        "    0.02,                    !- Divider Width {m}",
        "    2,                       !- Number of Horizontal Dividers",
        "    2,                       !- Number of Vertical Dividers",
        "    0.02,                    !- Divider Outside Projection {m}",
        "    0.02,                    !- Divider Inside Projection {m}",
        "    5.0,                     !- Divider Conductance {W/m2-K}",
        "    1.2,                     !- Ratio of Divider-Edge Glass Conductance to Center-Of-",
        "    0.8,                     !- Divider Solar Absorptance",
        "    0.8,                     !- Divider Visible Absorptance",
        "    0.9;                     !- Divider Thermal Hemispherical Emissivity",
        "  ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    Continuous;              !- Numeric Type",
        "  BuildingSurface:Detailed,",
        "    Zn001:floor,             !- Name",
        "    Floor,                   !- Surface Type",
        "    Surfaces,                !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.0000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,-5,0;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-South,        !- Name",
        "    Wall,                    !- Surface Type",
        "    Surfaces,                !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,-5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -5,-5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    5,-5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  Shading:Zone:Detailed,",
        "    Zn001:Wall-South:Shade001,  !- Name",
        "    Zn001:Wall-South,        !- Base Surface Name",
        "    OverhangTransmittance,   !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-6,2.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-6,2.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;  !- X,Y,Z ==> Vertex 4 {m}",
        "  Schedule:Compact,",
        "    OverhangTransmittance,   !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 10/31,          !- Field 5",
        "    For: AllDays,            !- Field 6",
        "    until: 24:00,0.5,          !- Field 7",
        "    Through: 12/31,          !- Field 9",
        "    For: AllDays,            !- Field 10",
        "    until: 24:00,0;          !- Field 11",
        "  Shading:Building:Detailed,",
        "    Zn001:Wall-South:Tree,   !- Name",
        "    TreeTransmittance,       !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    -3,-8,10,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -5,-8,2,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,-8,2,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-8,10;  !- X,Y,Z ==> Vertex 4 {m}",
        "  Schedule:Compact,",
        "    TreeTransmittance,   !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 10/31,          !- Field 5",
        "    For: AllDays,            !- Field 6",
        "    until: 24:00,0.8,          !- Field 7",
        "    Through: 12/31,          !- Field 9",
        "    For: AllDays,            !- Field 10",
        "    until: 24:00,0;          !- Field 11",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool FoundError = false;

    HeatBalanceManager::GetProjectControlData(*state, FoundError); // read project control data
    EXPECT_FALSE(FoundError);                                      // expect no errors

    HeatBalanceManager::SetPreConstructionInputParameters(*state);

    Material::GetMaterialData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    HeatBalanceManager::GetFrameAndDividerData(*state);

    HeatBalanceManager::GetConstructData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    SolarShading::GetShadowingInput(*state);

    HeatBalanceManager::GetZoneData(*state, FoundError); // Read Zone data from input file
    EXPECT_FALSE(FoundError);

    SurfaceGeometry::GetGeometryParameters(*state, FoundError);
    EXPECT_FALSE(FoundError);

    Weather::GetLocationInfo(*state, FoundError);
    EXPECT_FALSE(FoundError);

    Weather::CheckLocationValidity(*state);

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(1);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(1);

    state->dataSurfaceGeometry->CosZoneRelNorth(1) = std::cos(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(1) = std::sin(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;

    SurfaceGeometry::GetSurfaceData(*state, FoundError); // setup zone geometry and get zone data
    EXPECT_FALSE(FoundError);                            // expect no errors

    SolarShading::processShadowingInput(*state);

    //	compare_err_stream( "" ); // just for debugging

    SurfaceGeometry::SetupZoneGeometry(*state, FoundError); // this calls GetSurfaceData()
    EXPECT_FALSE(FoundError);

    SolarShading::AllocateModuleArrays(*state);
    SolarShading::DetermineShadowingCombinations(*state);

    //	compare_err_stream( "" ); // just for debugging

    state->dataSurface->ShadingTransmittanceVaries = true;
    state->dataHeatBal->SolarDistribution = DataHeatBalance::Shadowing::FullExterior;

    // Set up solar for Jan 1
    state->dataEnvrn->DayOfYear_Schedule = 1; // Jan 1
    state->dataEnvrn->DayOfYear = 1;          // Jan 1
    state->dataEnvrn->DayOfWeek = 6;
    state->dataGlobal->TimeStep = 4;
    state->dataGlobal->HourOfDay = 12;
    state->dataGlobal->BeginSimFlag = true;
    state->dataGlobal->BeginEnvrnFlag = true;
    SolarShading::InitSolarCalculations(*state);
    state->dataSolarShading->CalcSkyDifShading = true;
    SolarShading::SkyDifSolarShading(*state);
    state->dataSolarShading->CalcSkyDifShading = false;
    state->dataGlobal->BeginSimFlag = false;
    state->dataGlobal->BeginEnvrnFlag = false;
    HeatBalanceIntRadExchange::InitSolarViewFactors(*state); // prevents crash in GetDaylightingParametersInput
    SolarShading::PerformSolarCalculations(*state);

    // Get surface nums
    int winSurfNum = Util::FindItemInList("ZN001:WALL-SOUTH:WIN001", state->dataSurface->Surface);
    int wallSurfNum = Util::FindItemInList("ZN001:WALL-SOUTH", state->dataSurface->Surface);
    int overhangSurfNum = Util::FindItemInList("ZN001:WALL-SOUTH:SHADE001", state->dataSurface->Surface);
    int treeSurfNum = Util::FindItemInList("ZN001:WALL-SOUTH:TREE", state->dataSurface->Surface);

    // Get shading surface schedule indexes
    auto *overhangSched = dynamic_cast<Sched::ScheduleDetailed *>(state->dataSurface->Surface(overhangSurfNum).shadowSurfSched);
    auto *treeSched = dynamic_cast<Sched::ScheduleDetailed *>(state->dataSurface->Surface(treeSurfNum).shadowSurfSched);

    // Overhang transparent, Tree transparent
    overhangSched->EMSActuatedOn = true;
    treeSched->EMSActuatedOn = true;
    overhangSched->EMSVal = 1.0;
    treeSched->EMSVal = 1.0;
    FigureSolarBeamAtTimestep(*state, state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep);
    ReportSurfaceShading(*state);

    EXPECT_NEAR(1.0, state->dataSurface->SurfSunlitFrac(wallSurfNum), 0.0001);
    EXPECT_NEAR(1.0, state->dataSurface->SurfSunlitFrac(winSurfNum), 0.0001);

    // Overhang opaque, Tree transparent
    overhangSched->EMSActuatedOn = true;
    treeSched->EMSActuatedOn = true;
    overhangSched->EMSVal = 0.0;
    treeSched->EMSVal = 1.0;
    FigureSolarBeamAtTimestep(*state, state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep);
    ReportSurfaceShading(*state);

    Real64 wallSunLitFracOverhangOnly = state->dataSurface->SurfSunlitFrac(wallSurfNum);
    Real64 winSunLitFracOverhangOnly = state->dataSurface->SurfSunlitFrac(winSurfNum);
    EXPECT_NEAR(0.99915, wallSunLitFracOverhangOnly, 0.0001);
    EXPECT_NEAR(0.76955, winSunLitFracOverhangOnly, 0.0001);

    // Overhang tranparent, Tree opaque
    overhangSched->EMSActuatedOn = true;
    treeSched->EMSActuatedOn = true;
    overhangSched->EMSVal = 1.0;
    treeSched->EMSVal = 0.0;
    FigureSolarBeamAtTimestep(*state, state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep);
    ReportSurfaceShading(*state);

    Real64 wallSunLitFracTreeOnly = state->dataSurface->SurfSunlitFrac(wallSurfNum);
    Real64 winSunLitFracTreeOnly = state->dataSurface->SurfSunlitFrac(winSurfNum);
    EXPECT_NEAR(0.38584, wallSunLitFracTreeOnly, 0.0001);
    EXPECT_NEAR(0.05484, winSunLitFracTreeOnly, 0.0001);

    // Overhang opaque, Tree opaque
    overhangSched->EMSActuatedOn = true;
    treeSched->EMSActuatedOn = true;
    overhangSched->EMSVal = 0.0;
    treeSched->EMSVal = 0.0;
    FigureSolarBeamAtTimestep(*state, state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep);
    ReportSurfaceShading(*state);

    Real64 wallSunLitFracOverhangTree = state->dataSurface->SurfSunlitFrac(wallSurfNum);
    Real64 winSunLitFracOverhangTree = state->dataSurface->SurfSunlitFrac(winSurfNum);
    EXPECT_NEAR(0.38584, wallSunLitFracOverhangTree, 0.0001);
    EXPECT_NEAR(0.05484, winSunLitFracOverhangTree, 0.0001);

    // Use the base transmittance schedules (no EMS override)
    // Overhang transmittance = 0.5, Tree transmittance = 0.8
    overhangSched->EMSActuatedOn = false;
    treeSched->EMSActuatedOn = false;
    FigureSolarBeamAtTimestep(*state, state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep);
    ReportSurfaceShading(*state);

    Real64 wallSunLitFracOverhangTreePartial = state->dataSurface->SurfSunlitFrac(wallSurfNum);
    Real64 winSunLitFracOverhangTreePartial = state->dataSurface->SurfSunlitFrac(winSurfNum);

    // Expected results - wall
    Real64 treeShadowFrac = (1 - wallSunLitFracTreeOnly);
    Real64 overhangShadowFrac = (1 - wallSunLitFracOverhangOnly);
    Real64 combinedShadowFrac = (1 - wallSunLitFracOverhangTree);
    Real64 overlapShdowFrac = treeShadowFrac + overhangShadowFrac - combinedShadowFrac;
    Real64 treeOnlyShadowFrac = max(0.0, treeShadowFrac - overlapShdowFrac);
    Real64 overhangOnlyShadowFrac = max(0.0, overhangShadowFrac - overlapShdowFrac);
    Real64 expWallSunlitFrac =
        1.0 - ((1.0 - 0.5) * overhangOnlyShadowFrac) - ((1.0 - 0.8) * treeOnlyShadowFrac) - ((1.0 - 0.5 * 0.8) * overlapShdowFrac);
    EXPECT_NEAR(expWallSunlitFrac, wallSunLitFracOverhangTreePartial, 0.0001);

    // Expected results - window
    treeShadowFrac = (1 - winSunLitFracTreeOnly);
    overhangShadowFrac = (1 - winSunLitFracOverhangOnly);
    combinedShadowFrac = (1 - winSunLitFracOverhangTree);
    overlapShdowFrac = treeShadowFrac + overhangShadowFrac - combinedShadowFrac;
    treeOnlyShadowFrac = max(0.0, treeShadowFrac - overlapShdowFrac);
    overhangOnlyShadowFrac = max(0.0, overhangShadowFrac - overlapShdowFrac);
    Real64 expWinSunlitFrac =
        1.0 - ((1.0 - 0.5) * overhangOnlyShadowFrac) - ((1.0 - 0.8) * treeOnlyShadowFrac) - ((1.0 - 0.5 * 0.8) * overlapShdowFrac);
    EXPECT_NEAR(expWinSunlitFrac, winSunLitFracOverhangTreePartial, 0.0001);

    //  Set up solar for Jul 31
    state->dataEnvrn->DayOfYear_Schedule = 213; // Jul 31
    state->dataEnvrn->DayOfYear = 213;          // Jul 31
    state->dataEnvrn->DayOfWeek = 6;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->HourOfDay = 11;
    SolarShading::PerformSolarCalculations(*state);
    state->dataSolarShading->CalcSkyDifShading = false;

    // Overhang transparent, Tree transparent
    overhangSched->EMSActuatedOn = true;
    treeSched->EMSActuatedOn = true;
    overhangSched->EMSVal = 1.0;
    treeSched->EMSVal = 1.0;
    FigureSolarBeamAtTimestep(*state, state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep);
    ReportSurfaceShading(*state);

    EXPECT_NEAR(1.0, state->dataSurface->SurfSunlitFrac(wallSurfNum), 0.0001);
    EXPECT_NEAR(1.0, state->dataSurface->SurfSunlitFrac(winSurfNum), 0.0001);

    // Overhang opaque, Tree transparent
    overhangSched->EMSActuatedOn = true;
    treeSched->EMSActuatedOn = true;
    overhangSched->EMSVal = 0.0;
    treeSched->EMSVal = 1.0;
    FigureSolarBeamAtTimestep(*state, state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep);
    ReportSurfaceShading(*state);

    wallSunLitFracOverhangOnly = state->dataSurface->SurfSunlitFrac(wallSurfNum);
    winSunLitFracOverhangOnly = state->dataSurface->SurfSunlitFrac(winSurfNum);
    EXPECT_NEAR(0.77626, wallSunLitFracOverhangOnly, 0.0001);
    EXPECT_NEAR(0.08560, winSunLitFracOverhangOnly, 0.0001);

    // Overhang tranparent, Tree opaque
    overhangSched->EMSActuatedOn = true;
    treeSched->EMSActuatedOn = true;
    overhangSched->EMSVal = 1.0;
    treeSched->EMSVal = 0.0;
    FigureSolarBeamAtTimestep(*state, state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep);
    ReportSurfaceShading(*state);

    wallSunLitFracTreeOnly = state->dataSurface->SurfSunlitFrac(wallSurfNum);
    winSunLitFracTreeOnly = state->dataSurface->SurfSunlitFrac(winSurfNum);
    EXPECT_NEAR(0.64961, wallSunLitFracTreeOnly, 0.0001);
    EXPECT_NEAR(0.60655, winSunLitFracTreeOnly, 0.0001);

    // Overhang opaque, Tree opaque
    overhangSched->EMSActuatedOn = true;
    treeSched->EMSActuatedOn = true;
    overhangSched->EMSVal = 0.0;
    treeSched->EMSVal = 0.0;
    FigureSolarBeamAtTimestep(*state, state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep);
    ReportSurfaceShading(*state);
    wallSunLitFracOverhangTree = state->dataSurface->SurfSunlitFrac(wallSurfNum);
    winSunLitFracOverhangTree = state->dataSurface->SurfSunlitFrac(winSurfNum);

    EXPECT_NEAR(0.58989, wallSunLitFracOverhangTree, 0.0001);
    EXPECT_NEAR(0.08560, winSunLitFracOverhangTree, 0.0001);

    // Use the base transmittance schedules (no EMS override)
    // Overhang transmittance = 0.5, Tree transmittance = 0.8
    overhangSched->EMSActuatedOn = false;
    treeSched->EMSActuatedOn = false;
    FigureSolarBeamAtTimestep(*state, state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep);
    ReportSurfaceShading(*state);

    wallSunLitFracOverhangTreePartial = state->dataSurface->SurfSunlitFrac(wallSurfNum);
    winSunLitFracOverhangTreePartial = state->dataSurface->SurfSunlitFrac(winSurfNum);

    // Expected results - wall
    treeShadowFrac = (1 - wallSunLitFracTreeOnly);
    overhangShadowFrac = (1 - wallSunLitFracOverhangOnly);
    combinedShadowFrac = (1 - wallSunLitFracOverhangTree);
    overlapShdowFrac = treeShadowFrac + overhangShadowFrac - combinedShadowFrac;
    treeOnlyShadowFrac = max(0.0, treeShadowFrac - overlapShdowFrac);
    overhangOnlyShadowFrac = max(0.0, overhangShadowFrac - overlapShdowFrac);
    expWallSunlitFrac = 1.0 - ((1.0 - 0.5) * overhangOnlyShadowFrac) - ((1.0 - 0.8) * treeOnlyShadowFrac) - ((1.0 - 0.5 * 0.8) * overlapShdowFrac);
    EXPECT_NEAR(expWallSunlitFrac, wallSunLitFracOverhangTreePartial, 0.0001);

    // Expected results - window
    treeShadowFrac = (1 - winSunLitFracTreeOnly);
    overhangShadowFrac = (1 - winSunLitFracOverhangOnly);
    combinedShadowFrac = (1 - winSunLitFracOverhangTree);
    overlapShdowFrac = treeShadowFrac + overhangShadowFrac - combinedShadowFrac;
    treeOnlyShadowFrac = max(0.0, treeShadowFrac - overlapShdowFrac);
    overhangOnlyShadowFrac = max(0.0, overhangShadowFrac - overlapShdowFrac);
    expWinSunlitFrac = 1.0 - ((1.0 - 0.5) * overhangOnlyShadowFrac) - ((1.0 - 0.8) * treeOnlyShadowFrac) - ((1.0 - 0.5 * 0.8) * overlapShdowFrac);
    EXPECT_NEAR(expWinSunlitFrac, winSunLitFracOverhangTreePartial, 0.0001);
}
TEST_F(EnergyPlusFixture, SolarShadingTest_PolygonOverlap2)
// Tests complete overlap of shadows which completely shade the test surfaces
{
    std::string const idf_objects = delimited_string({
        "  Building,",
        "    DemoFDT,                 !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    3.9999999E-02,           !- Loads Convergence Tolerance Value",
        "    4.0000002E-03,           !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullExterior;            !- Solar Distribution",
        "  ShadowCalculation,",
        "    PolygonClipping,         !- Shading Calculation Method",
        "    Timestep,                !- Shading Calculation Update Frequency Method",
        "    ,                        !- Shading Calculation Update Frequency",
        "    ,                        !- Maximum Figures in Shadow Overlap Calculations",
        "    ,                        !- Polygon Clipping Algorithm",
        "    ,                        !- Pixel Counting Resolution",
        "    DetailedSkyDiffuseModeling;  !- Sky Diffuse Modeling Algorithm",
        "  Timestep,6;",
        "  Site:Location,",
        "    CHICAGO_IL_USA TMY2-94846,  !- Name",
        "    42.00,                   !- Latitude {deg}",
        "    -87.88,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190.00;                  !- Elevation {m}",
        "  Material,",
        "    C12 - 2 IN HW CONCRETE,  !- Name",
        "    MediumRough,             !- Roughness",
        "    5.0901599E-02,           !- Thickness {m}",
        "    1.729577,                !- Conductivity {W/m-K}",
        "    2242.585,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",
        "  WindowMaterial:Glazing,",
        "    GLASS - CLEAR PLATE 1 / 4 IN,  !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.006,                   !- Thickness {m}",
        "    0.80,                    !- Solar Transmittance at Normal Incidence",
        "    0.10,                    !- Front Side Solar Reflectance at Normal Incidence",
        "    0.10,                    !- Back Side Solar Reflectance at Normal Incidence",
        "    0.80,                    !- Visible Transmittance at Normal Incidence",
        "    0.10,                    !- Front Side Visible Reflectance at Normal Incidence",
        "    0.10,                    !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.84,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",
        "  Construction,",
        "    Surfaces,                !- Name",
        "    C12 - 2 IN HW CONCRETE;  !- Layer 4",
        "  Construction,",
        "    Single PANE HW WINDOW,   !- Name",
        "    GLASS - CLEAR PLATE 1 / 4 IN;  !- Outside Layer",
        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    Counterclockwise,        !- Vertex Entry Direction",
        "    Relative;                !- Coordinate System",
        "  Zone,",
        "    ZONE ONE;                !- Name",
        "  FenestrationSurface:Detailed,",
        "    Zn001:Wall-South:Win001, !- Name",
        "    Window,                  !- Surface Type",
        "    Single PANE HW WINDOW,   !- Construction Name",
        "    Zn001:Wall-South,        !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    TestFrameAndDivider,     !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-5,0.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-5,0.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;  !- X,Y,Z ==> Vertex 4 {m}",
        "  WindowProperty:FrameAndDivider,",
        "    TestFrameAndDivider,     !- Name",
        "    0.05,                    !- Frame Width {m}",
        "    0.05,                    !- Frame Outside Projection {m}",
        "    0.05,                    !- Frame Inside Projection {m}",
        "    5.0,                     !- Frame Conductance {W/m2-K}",
        "    1.2,                     !- Ratio of Frame-Edge Glass Conductance to Center-Of-Gl",
        "    0.8,                     !- Frame Solar Absorptance",
        "    0.8,                     !- Frame Visible Absorptance",
        "    0.9,                     !- Frame Thermal Hemispherical Emissivity",
        "    DividedLite,             !- Divider Type",
        "    0.02,                    !- Divider Width {m}",
        "    2,                       !- Number of Horizontal Dividers",
        "    2,                       !- Number of Vertical Dividers",
        "    0.02,                    !- Divider Outside Projection {m}",
        "    0.02,                    !- Divider Inside Projection {m}",
        "    5.0,                     !- Divider Conductance {W/m2-K}",
        "    1.2,                     !- Ratio of Divider-Edge Glass Conductance to Center-Of-",
        "    0.8,                     !- Divider Solar Absorptance",
        "    0.8,                     !- Divider Visible Absorptance",
        "    0.9;                     !- Divider Thermal Hemispherical Emissivity",
        "  ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    Continuous;              !- Numeric Type",
        "  BuildingSurface:Detailed,",
        "    Zn001:floor,             !- Name",
        "    Floor,                   !- Surface Type",
        "    Surfaces,                !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.0000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,-5,0;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-South,        !- Name",
        "    Wall,                    !- Surface Type",
        "    Surfaces,                !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,-5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -5,-5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    5,-5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  Shading:Building:Detailed,",
        "    Zn001:Wall-South:Shade1,   !- Name",
        "    Shade1Transmittance,       !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    -10,-8,20,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -10,-8,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10,-8,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10,-8,20;  !- X,Y,Z ==> Vertex 4 {m}",
        "  Schedule:Compact,",
        "    Shade1Transmittance,   !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 10/31,          !- Field 5",
        "    For: AllDays,            !- Field 6",
        "    until: 24:00,0.5,          !- Field 7",
        "    Through: 12/31,          !- Field 9",
        "    For: AllDays,            !- Field 10",
        "    until: 24:00,0;          !- Field 11",
        "  Shading:Building:Detailed,",
        "    Zn001:Wall-South:Shade2,   !- Name",
        "    Shade2Transmittance,       !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    -10,-9,20,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -10,-9,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10,-9,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10,-9,20;  !- X,Y,Z ==> Vertex 4 {m}",
        "  Schedule:Compact,",
        "    Shade2Transmittance,   !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 10/31,          !- Field 5",
        "    For: AllDays,            !- Field 6",
        "    until: 24:00,0.8,          !- Field 7",
        "    Through: 12/31,          !- Field 9",
        "    For: AllDays,            !- Field 10",
        "    until: 24:00,0;          !- Field 11",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool FoundError = false;

    HeatBalanceManager::GetProjectControlData(*state, FoundError); // read project control data
    EXPECT_FALSE(FoundError);                                      // expect no errors

    HeatBalanceManager::SetPreConstructionInputParameters(*state);

    Material::GetMaterialData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    HeatBalanceManager::GetFrameAndDividerData(*state);

    HeatBalanceManager::GetConstructData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    HeatBalanceManager::GetZoneData(*state, FoundError); // Read Zone data from input file
    EXPECT_FALSE(FoundError);

    SurfaceGeometry::GetGeometryParameters(*state, FoundError);
    EXPECT_FALSE(FoundError);

    Weather::GetLocationInfo(*state, FoundError);
    EXPECT_FALSE(FoundError);

    Weather::CheckLocationValidity(*state);

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(1);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(1);

    state->dataSurfaceGeometry->CosZoneRelNorth(1) = std::cos(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(1) = std::sin(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;

    SurfaceGeometry::GetSurfaceData(*state, FoundError); // setup zone geometry and get zone data
    EXPECT_FALSE(FoundError);                            // expect no errors

    //	compare_err_stream( "" ); // just for debugging

    SurfaceGeometry::SetupZoneGeometry(*state, FoundError); // this calls GetSurfaceData()
    EXPECT_FALSE(FoundError);

    SolarShading::AllocateModuleArrays(*state);
    SolarShading::DetermineShadowingCombinations(*state);

    //	compare_err_stream( "" ); // just for debugging

    state->dataSurface->ShadingTransmittanceVaries = true;
    state->dataHeatBal->SolarDistribution = DataHeatBalance::Shadowing::FullExterior;

    // Set up solar for Jan 1
    state->dataEnvrn->DayOfYear_Schedule = 1; // Jan 1
    state->dataEnvrn->DayOfYear = 1;          // Jan 1
    state->dataEnvrn->DayOfWeek = 6;
    state->dataGlobal->TimeStep = 4;
    state->dataGlobal->HourOfDay = 12;
    state->dataGlobal->BeginSimFlag = true;
    state->dataGlobal->BeginEnvrnFlag = true;
    SolarShading::InitSolarCalculations(*state);
    state->dataSolarShading->CalcSkyDifShading = true;
    SolarShading::SkyDifSolarShading(*state);
    state->dataSolarShading->CalcSkyDifShading = false;
    state->dataGlobal->BeginSimFlag = false;
    state->dataGlobal->BeginEnvrnFlag = false;
    HeatBalanceIntRadExchange::InitSolarViewFactors(*state); // prevents crash in GetDaylightingParametersInput
    state->dataSolarShading->ShadowingDaysLeft = 20;
    SolarShading::PerformSolarCalculations(*state);

    // Get surface nums
    int winSurfNum = Util::FindItemInList("ZN001:WALL-SOUTH:WIN001", state->dataSurface->Surface);
    int wallSurfNum = Util::FindItemInList("ZN001:WALL-SOUTH", state->dataSurface->Surface);
    int shade1SurfNum = Util::FindItemInList("ZN001:WALL-SOUTH:SHADE1", state->dataSurface->Surface);
    int shade2SurfNum = Util::FindItemInList("ZN001:WALL-SOUTH:SHADE2", state->dataSurface->Surface);

    // Get shading surface schedule indexes
    auto *shade1Sched = state->dataSurface->Surface(shade1SurfNum).shadowSurfSched;
    auto *shade2Sched = state->dataSurface->Surface(shade2SurfNum).shadowSurfSched;

    // shade1 transparent, shade2 transparent
    shade1Sched->EMSActuatedOn = true;
    shade2Sched->EMSActuatedOn = true;
    shade1Sched->EMSVal = 1.0;
    shade2Sched->EMSVal = 1.0;

    // Gotten from running 1ZoneUncontrolled.idf with chicago weather on Jan 1 at 12
    state->dataBSDFWindow->SUNCOSTS(state->dataGlobal->TimeStep, state->dataGlobal->HourOfDay)(1) = 0.20531446332266728;
    state->dataBSDFWindow->SUNCOSTS(state->dataGlobal->TimeStep, state->dataGlobal->HourOfDay)(2) = -0.84761109808931534;
    state->dataBSDFWindow->SUNCOSTS(state->dataGlobal->TimeStep, state->dataGlobal->HourOfDay)(3) = 0.48928662105799514;
    FigureSolarBeamAtTimestep(*state, state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep);
    ReportSurfaceShading(*state);

    EXPECT_NEAR(1.0, state->dataSurface->SurfSunlitFrac(wallSurfNum), 0.0001);
    EXPECT_NEAR(1.0, state->dataSurface->SurfSunlitFrac(winSurfNum), 0.0001);

    // shade1 opaque, shade2 transparent
    shade1Sched->EMSActuatedOn = true;
    shade2Sched->EMSActuatedOn = true;
    shade1Sched->EMSVal = 0.0;
    shade2Sched->EMSVal = 1.0;
    FigureSolarBeamAtTimestep(*state, state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep);
    ReportSurfaceShading(*state);

    Real64 wallSunLitFracShade1Only = state->dataSurface->SurfSunlitFrac(wallSurfNum);
    Real64 winSunLitFracShade1Only = state->dataSurface->SurfSunlitFrac(winSurfNum);
    EXPECT_NEAR(0.0, wallSunLitFracShade1Only, 0.0001);
    EXPECT_NEAR(0.0, winSunLitFracShade1Only, 0.0001);

    // shade1 transparent, shade2 opaque
    shade1Sched->EMSActuatedOn = true;
    shade2Sched->EMSActuatedOn = true;
    shade1Sched->EMSVal = 1.0;
    shade2Sched->EMSVal = 0.0;
    FigureSolarBeamAtTimestep(*state, state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep);
    ReportSurfaceShading(*state);

    Real64 wallSunLitFracShade2Only = state->dataSurface->SurfSunlitFrac(wallSurfNum);
    Real64 winSunLitFracShade2Only = state->dataSurface->SurfSunlitFrac(winSurfNum);
    EXPECT_NEAR(0.0, wallSunLitFracShade2Only, 0.0001);
    EXPECT_NEAR(0.0, winSunLitFracShade2Only, 0.0001);

    // shade1 opaque, shade2 opaque
    shade1Sched->EMSActuatedOn = true;
    shade2Sched->EMSActuatedOn = true;
    shade1Sched->EMSVal = 0.0;
    shade2Sched->EMSVal = 0.0;
    FigureSolarBeamAtTimestep(*state, state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep);
    ReportSurfaceShading(*state);

    Real64 wallSunLitFracShade1Shade2 = state->dataSurface->SurfSunlitFrac(wallSurfNum);
    Real64 winSunLitFracShade1Shade2 = state->dataSurface->SurfSunlitFrac(winSurfNum);
    EXPECT_NEAR(0.0, wallSunLitFracShade1Shade2, 0.0001);
    EXPECT_NEAR(0.0, winSunLitFracShade1Shade2, 0.0001);

    // Use the base transmittance schedules (no EMS override)
    // shade1 transmittance = 0.5, shade2 transmittance = 0.8
    shade1Sched->EMSActuatedOn = false;
    shade2Sched->EMSActuatedOn = false;
    FigureSolarBeamAtTimestep(*state, state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep);
    ReportSurfaceShading(*state);

    Real64 wallSunLitFracShade1Shade2Partial = state->dataSurface->SurfSunlitFrac(wallSurfNum);
    Real64 winSunLitFracShade1Shade2Partial = state->dataSurface->SurfSunlitFrac(winSurfNum);

    // Expected results - wall
    Real64 shade2ShadowFrac = (1 - wallSunLitFracShade2Only);
    Real64 shade1ShadowFrac = (1 - wallSunLitFracShade1Only);
    Real64 combinedShadowFrac = (1 - wallSunLitFracShade1Shade2);
    Real64 overlapShdowFrac = shade2ShadowFrac + shade1ShadowFrac - combinedShadowFrac;
    Real64 shade2OnlyShadowFrac = max(0.0, shade2ShadowFrac - overlapShdowFrac);
    Real64 shade1OnlyShadowFrac = max(0.0, shade1ShadowFrac - overlapShdowFrac);
    Real64 expWallSunlitFrac =
        1.0 - ((1.0 - 0.5) * shade1OnlyShadowFrac) - ((1.0 - 0.8) * shade2OnlyShadowFrac) - ((1.0 - 0.5 * 0.8) * overlapShdowFrac);
    EXPECT_NEAR(expWallSunlitFrac, wallSunLitFracShade1Shade2Partial, 0.0001);
    expWallSunlitFrac = 0.5 * 0.8; // again, in simple terms
    EXPECT_NEAR(expWallSunlitFrac, wallSunLitFracShade1Shade2Partial, 0.0001);

    // Expected results - window
    shade2ShadowFrac = (1 - winSunLitFracShade2Only);
    shade1ShadowFrac = (1 - winSunLitFracShade1Only);
    combinedShadowFrac = (1 - winSunLitFracShade1Shade2);
    overlapShdowFrac = shade2ShadowFrac + shade1ShadowFrac - combinedShadowFrac;
    shade2OnlyShadowFrac = max(0.0, shade2ShadowFrac - overlapShdowFrac);
    shade1OnlyShadowFrac = max(0.0, shade1ShadowFrac - overlapShdowFrac);
    Real64 expWinSunlitFrac =
        1.0 - ((1.0 - 0.5) * shade1OnlyShadowFrac) - ((1.0 - 0.8) * shade2OnlyShadowFrac) - ((1.0 - 0.5 * 0.8) * overlapShdowFrac);
    EXPECT_NEAR(expWinSunlitFrac, winSunLitFracShade1Shade2Partial, 0.0001);
    expWinSunlitFrac = 0.5 * 0.8; // again, in simple terms
    EXPECT_NEAR(expWinSunlitFrac, winSunLitFracShade1Shade2Partial, 0.0001);
}
TEST_F(EnergyPlusFixture, SolarShadingTest_PolygonOverlap3)
// Tests complete overlap of 3 shadows which completely shade the test surfaces
{
    std::string const idf_objects = delimited_string({
        "  Building,",
        "    DemoFDT,                 !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    3.9999999E-02,           !- Loads Convergence Tolerance Value",
        "    4.0000002E-03,           !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullExterior;            !- Solar Distribution",
        "  ShadowCalculation,",
        "    PolygonClipping,         !- Shading Calculation Method",
        "    Timestep,                !- Shading Calculation Update Frequency Method",
        "    ,                        !- Shading Calculation Update Frequency",
        "    ,                        !- Maximum Figures in Shadow Overlap Calculations",
        "    ,                        !- Polygon Clipping Algorithm",
        "    ,                        !- Pixel Counting Resolution",
        "    DetailedSkyDiffuseModeling;  !- Sky Diffuse Modeling Algorithm",
        "  Timestep,6;",
        "  Site:Location,",
        "    CHICAGO_IL_USA TMY2-94846,  !- Name",
        "    42.00,                   !- Latitude {deg}",
        "    -87.88,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190.00;                  !- Elevation {m}",
        "  Material,",
        "    C12 - 2 IN HW CONCRETE,  !- Name",
        "    MediumRough,             !- Roughness",
        "    5.0901599E-02,           !- Thickness {m}",
        "    1.729577,                !- Conductivity {W/m-K}",
        "    2242.585,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",
        "  WindowMaterial:Glazing,",
        "    GLASS - CLEAR PLATE 1 / 4 IN,  !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.006,                   !- Thickness {m}",
        "    0.80,                    !- Solar Transmittance at Normal Incidence",
        "    0.10,                    !- Front Side Solar Reflectance at Normal Incidence",
        "    0.10,                    !- Back Side Solar Reflectance at Normal Incidence",
        "    0.80,                    !- Visible Transmittance at Normal Incidence",
        "    0.10,                    !- Front Side Visible Reflectance at Normal Incidence",
        "    0.10,                    !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.84,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",
        "  Construction,",
        "    Surfaces,                !- Name",
        "    C12 - 2 IN HW CONCRETE;  !- Layer 4",
        "  Construction,",
        "    Single PANE HW WINDOW,   !- Name",
        "    GLASS - CLEAR PLATE 1 / 4 IN;  !- Outside Layer",
        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    Counterclockwise,        !- Vertex Entry Direction",
        "    Relative;                !- Coordinate System",
        "  Zone,",
        "    ZONE ONE;                !- Name",
        "  FenestrationSurface:Detailed,",
        "    Zn001:Wall-South:Win001, !- Name",
        "    Window,                  !- Surface Type",
        "    Single PANE HW WINDOW,   !- Construction Name",
        "    Zn001:Wall-South,        !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    TestFrameAndDivider,     !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-5,0.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-5,0.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;  !- X,Y,Z ==> Vertex 4 {m}",
        "  WindowProperty:FrameAndDivider,",
        "    TestFrameAndDivider,     !- Name",
        "    0.05,                    !- Frame Width {m}",
        "    0.05,                    !- Frame Outside Projection {m}",
        "    0.05,                    !- Frame Inside Projection {m}",
        "    5.0,                     !- Frame Conductance {W/m2-K}",
        "    1.2,                     !- Ratio of Frame-Edge Glass Conductance to Center-Of-Gl",
        "    0.8,                     !- Frame Solar Absorptance",
        "    0.8,                     !- Frame Visible Absorptance",
        "    0.9,                     !- Frame Thermal Hemispherical Emissivity",
        "    DividedLite,             !- Divider Type",
        "    0.02,                    !- Divider Width {m}",
        "    2,                       !- Number of Horizontal Dividers",
        "    2,                       !- Number of Vertical Dividers",
        "    0.02,                    !- Divider Outside Projection {m}",
        "    0.02,                    !- Divider Inside Projection {m}",
        "    5.0,                     !- Divider Conductance {W/m2-K}",
        "    1.2,                     !- Ratio of Divider-Edge Glass Conductance to Center-Of-",
        "    0.8,                     !- Divider Solar Absorptance",
        "    0.8,                     !- Divider Visible Absorptance",
        "    0.9;                     !- Divider Thermal Hemispherical Emissivity",
        "  ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    Continuous;              !- Numeric Type",
        "  BuildingSurface:Detailed,",
        "    Zn001:floor,             !- Name",
        "    Floor,                   !- Surface Type",
        "    Surfaces,                !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.0000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,-5,0;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-South,        !- Name",
        "    Wall,                    !- Surface Type",
        "    Surfaces,                !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,-5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -5,-5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    5,-5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  Shading:Building:Detailed,",
        "    Zn001:Wall-South:Shade1,   !- Name",
        "    Shade1Transmittance,       !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    -10,-8,20,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -10,-8,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10,-8,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10,-8,20;  !- X,Y,Z ==> Vertex 4 {m}",
        "  Schedule:Compact,",
        "    Shade1Transmittance,   !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 10/31,          !- Field 5",
        "    For: AllDays,            !- Field 6",
        "    until: 24:00,0.5,          !- Field 7",
        "    Through: 12/31,          !- Field 9",
        "    For: AllDays,            !- Field 10",
        "    until: 24:00,0;          !- Field 11",
        "  Shading:Building:Detailed,",
        "    Zn001:Wall-South:Shade2,   !- Name",
        "    Shade2Transmittance,       !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    -10,-9,20,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -10,-9,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10,-9,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10,-9,20;  !- X,Y,Z ==> Vertex 4 {m}",
        "  Schedule:Compact,",
        "    Shade2Transmittance,   !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 10/31,          !- Field 5",
        "    For: AllDays,            !- Field 6",
        "    until: 24:00,0.8,          !- Field 7",
        "    Through: 12/31,          !- Field 9",
        "    For: AllDays,            !- Field 10",
        "    until: 24:00,0;          !- Field 11",
        "  Shading:Building:Detailed,",
        "    Zn001:Wall-South:Shade3,   !- Name",
        "    Shade3Transmittance,       !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    -10,-9,20,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -10,-9,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10,-9,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10,-9,20;  !- X,Y,Z ==> Vertex 4 {m}",
        "  Schedule:Compact,",
        "    Shade3Transmittance,   !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 10/31,          !- Field 5",
        "    For: AllDays,            !- Field 6",
        "    until: 24:00,0.2,          !- Field 7",
        "    Through: 12/31,          !- Field 9",
        "    For: AllDays,            !- Field 10",
        "    until: 24:00,0;          !- Field 11",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool FoundError = false;

    HeatBalanceManager::GetProjectControlData(*state, FoundError); // read project control data
    EXPECT_FALSE(FoundError);                                      // expect no errors

    HeatBalanceManager::SetPreConstructionInputParameters(*state);

    Material::GetMaterialData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    HeatBalanceManager::GetFrameAndDividerData(*state);

    HeatBalanceManager::GetConstructData(*state, FoundError);
    EXPECT_FALSE(FoundError);

    HeatBalanceManager::GetZoneData(*state, FoundError); // Read Zone data from input file
    EXPECT_FALSE(FoundError);

    SurfaceGeometry::GetGeometryParameters(*state, FoundError);
    EXPECT_FALSE(FoundError);

    Weather::GetLocationInfo(*state, FoundError);
    EXPECT_FALSE(FoundError);

    Weather::CheckLocationValidity(*state);

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(1);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(1);

    state->dataSurfaceGeometry->CosZoneRelNorth(1) = std::cos(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(1) = std::sin(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;

    SurfaceGeometry::GetSurfaceData(*state, FoundError); // setup zone geometry and get zone data
    EXPECT_FALSE(FoundError);                            // expect no errors

    //	compare_err_stream( "" ); // just for debugging

    SurfaceGeometry::SetupZoneGeometry(*state, FoundError); // this calls GetSurfaceData()
    EXPECT_FALSE(FoundError);

    SolarShading::AllocateModuleArrays(*state);
    SolarShading::DetermineShadowingCombinations(*state);

    //	compare_err_stream( "" ); // just for debugging

    state->dataSurface->ShadingTransmittanceVaries = true;
    state->dataHeatBal->SolarDistribution = DataHeatBalance::Shadowing::FullExterior;

    // Set up solar for Jan 1
    state->dataEnvrn->DayOfYear_Schedule = 1; // Jan 1
    state->dataEnvrn->DayOfYear = 1;          // Jan 1
    state->dataEnvrn->DayOfWeek = 6;
    state->dataGlobal->TimeStep = 4;
    state->dataGlobal->HourOfDay = 12;
    state->dataGlobal->BeginSimFlag = true;
    state->dataGlobal->BeginEnvrnFlag = true;
    SolarShading::InitSolarCalculations(*state);
    state->dataSolarShading->CalcSkyDifShading = true;
    SolarShading::SkyDifSolarShading(*state);
    state->dataSolarShading->CalcSkyDifShading = false;
    state->dataGlobal->BeginSimFlag = false;
    state->dataGlobal->BeginEnvrnFlag = false;
    HeatBalanceIntRadExchange::InitSolarViewFactors(*state); // prevents crash in GetDaylightingParametersInput
    state->dataSolarShading->ShadowingDaysLeft = 20;
    SolarShading::PerformSolarCalculations(*state);

    // Get surface nums
    int winSurfNum = Util::FindItemInList("ZN001:WALL-SOUTH:WIN001", state->dataSurface->Surface);
    int wallSurfNum = Util::FindItemInList("ZN001:WALL-SOUTH", state->dataSurface->Surface);

    // Use the base transmittance schedules (no EMS override)
    // shade1 transmittance = 0.5, shade2 transmittance = 0.8
    state->dataBSDFWindow->SUNCOSTS(state->dataGlobal->TimeStep, state->dataGlobal->HourOfDay)(1) = 0.20531446332266728;
    state->dataBSDFWindow->SUNCOSTS(state->dataGlobal->TimeStep, state->dataGlobal->HourOfDay)(2) = -0.84761109808931534;
    state->dataBSDFWindow->SUNCOSTS(state->dataGlobal->TimeStep, state->dataGlobal->HourOfDay)(3) = 0.48928662105799514;
    FigureSolarBeamAtTimestep(*state, state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep);
    ReportSurfaceShading(*state);

    Real64 wallSunLitFracShade1Shade2Partial = state->dataSurface->SurfSunlitFrac(wallSurfNum);
    Real64 winSunLitFracShade1Shade2Partial = state->dataSurface->SurfSunlitFrac(winSurfNum);

    Real64 expWallSunlitFrac = 0.5 * 0.8 * 0.2; // again, in simple terms
    EXPECT_NEAR(expWallSunlitFrac, wallSunLitFracShade1Shade2Partial, 0.0001);

    // Expected results - window
    Real64 expWinSunlitFrac = 0.5 * 0.8 * 0.2; // again, in simple terms
    EXPECT_NEAR(expWinSunlitFrac, winSunLitFracShade1Shade2Partial, 0.0001);
}

TEST_F(EnergyPlusFixture, SolarShadingTest_checkScheduledSurfacePresent)
// Test of check to see if all surfaces have a sunlit schedule when shadow calculations are set to "Scheduled"
{
    auto &surfData = state->dataSurface;
    surfData->TotSurfaces = 5;
    surfData->Surface.allocate(surfData->TotSurfaces);

    // Set up data for test: three of five surfaces are non-shading and two are shading.  Shading surfaces
    // are skipped so they should not report any errors.  Two of the surfaces will be correctly defined while
    // one surface will not be correctly defined and will generate an error.
    surfData->Surface(1).Class = SurfaceClass::Wall;
    surfData->Surface(1).SurfSchedExternalShadingFrac = true;
    surfData->Surface(1).Name = "WALL1OK";
    surfData->Surface(2).Class = SurfaceClass::Roof;
    surfData->Surface(2).SurfSchedExternalShadingFrac = true;
    surfData->Surface(2).Name = "ROOF1OK";
    surfData->Surface(3).Class = SurfaceClass::Window;
    surfData->Surface(3).SurfSchedExternalShadingFrac = false;
    surfData->Surface(3).Name = "WINDOW1NOTOK";
    surfData->Surface(4).Class = SurfaceClass::Shading;
    surfData->Surface(4).SurfSchedExternalShadingFrac = true;
    surfData->Surface(4).Name = "SHADING1OK";
    surfData->Surface(5).Class = SurfaceClass::Overhang;
    surfData->Surface(5).SurfSchedExternalShadingFrac = false;
    surfData->Surface(5).Name = "SHADING2NOTOK";

    checkScheduledSurfacePresent(*state);

    std::string const error_string = delimited_string({
        "   ** Warning ** ShadowCalculation specified Schedule for the Shading Calculation Method but no schedule provided for WINDOW1NOTOK",
        "   **   ~~~   ** When Schedule is selected for the Shading Calculation Method and no schedule is provided for a particular surface,",
        "   **   ~~~   ** EnergyPlus will assume that the surface is not shaded.  Use SurfaceProperty:LocalEnvironment to specify a schedule",
        "   **   ~~~   ** for sunlit fraction if this was not desired.  Otherwise, this surface will not be shaded at all.",
    });
    EXPECT_TRUE(compare_err_stream(error_string, true));
}
TEST_F(EnergyPlusFixture, SolarShadingTest_CalcBeamSolarOnWinRevealSurface)
{
    state->dataGlobal->TimeStepsInHour = 6;

    state->dataSurface->FrameDivider.allocate(2);
    auto &frameDivider1 = state->dataSurface->FrameDivider(1);
    frameDivider1.Name = "FrameDivider1";
    frameDivider1.FrameWidth = 0.1;
    frameDivider1.FrameProjectionOut = 0.0;
    frameDivider1.FrameProjectionIn = 0.00;
    frameDivider1.FrameConductance = 0.04;
    frameDivider1.FrEdgeToCenterGlCondRatio = 1.0;
    frameDivider1.FrameSolAbsorp = 0.8;
    frameDivider1.FrameVisAbsorp = 0.5;
    frameDivider1.FrameEmis = 0.9;
    frameDivider1.DividerType = DataSurfaces::FrameDividerType::DividedLite;
    frameDivider1.DividerWidth = 0.2;
    frameDivider1.HorDividers = 2;
    frameDivider1.VertDividers = 2;
    frameDivider1.DividerProjectionOut = 0.00;
    frameDivider1.DividerProjectionIn = 0.00;
    frameDivider1.DividerConductance = 0.04;
    frameDivider1.DivEdgeToCenterGlCondRatio = 1.0;
    frameDivider1.DividerSolAbsorp = 0.9;
    frameDivider1.DividerVisAbsorp = 0.5;
    frameDivider1.DividerEmis = 0.85;
    frameDivider1.OutsideRevealSolAbs = 0.75;
    frameDivider1.InsideSillDepth = 0.2;
    frameDivider1.InsideSillSolAbs = 0.9;
    frameDivider1.InsideReveal = 0.1;
    frameDivider1.InsideRevealSolAbs = 0.85;

    auto &frameDivider2 = state->dataSurface->FrameDivider(2);
    frameDivider2.Name = "FrameDivider2";
    frameDivider2.FrameWidth = 0.1;
    frameDivider2.FrameProjectionOut = 0.0; // 0.2
    frameDivider2.FrameProjectionIn = 0.00; // 0.05
    frameDivider2.FrameConductance = 0.04;
    frameDivider2.FrEdgeToCenterGlCondRatio = 1.0;
    frameDivider2.FrameSolAbsorp = 0.8;
    frameDivider2.FrameVisAbsorp = 0.5;
    frameDivider2.FrameEmis = 0.9;
    frameDivider2.DividerType = DataSurfaces::FrameDividerType::DividedLite;
    frameDivider2.DividerWidth = 0.2;
    frameDivider2.HorDividers = 2;
    frameDivider2.VertDividers = 2;
    frameDivider2.DividerProjectionOut = 0.00; // 0.04
    frameDivider2.DividerProjectionIn = 0.00;  // 0.01
    frameDivider2.DividerConductance = 0.04;
    frameDivider2.DivEdgeToCenterGlCondRatio = 1.0;
    frameDivider2.DividerSolAbsorp = 0.9;
    frameDivider2.DividerVisAbsorp = 0.5;
    frameDivider2.DividerEmis = 0.85;
    frameDivider2.OutsideRevealSolAbs = 0.75;
    frameDivider2.InsideSillDepth = 0.2;
    frameDivider2.InsideSillSolAbs = 0.9;
    frameDivider2.InsideReveal = 0.1;
    frameDivider2.InsideRevealSolAbs = 0.85;

    int NumSurf = 2;
    state->dataSurface->TotSurfaces = NumSurf;
    state->dataSurface->Surface.allocate(NumSurf);
    state->dataSurface->SurfaceWindow.allocate(NumSurf);
    EnergyPlus::SurfaceGeometry::AllocateSurfaceWindows(*state, NumSurf);
    Window::initWindowModel(*state);
    SolarShading::AllocateModuleArrays(*state);

    auto &surf1 = state->dataSurface->Surface(1);
    auto &surf2 = state->dataSurface->Surface(2);
    surf1.Name = "Surface1";
    surf2.Name = "Surface2";
    surf1.Zone = 1;
    surf2.Zone = 1;
    surf1.spaceNum = 1;
    surf2.spaceNum = 1;
    surf1.Class = DataSurfaces::SurfaceClass::Window;
    surf2.Class = DataSurfaces::SurfaceClass::Window;
    surf1.ExtBoundCond = DataSurfaces::ExternalEnvironment;
    surf2.ExtBoundCond = DataSurfaces::ExternalEnvironment;
    surf1.HasShadeControl = false;
    surf2.HasShadeControl = false;
    surf1.Construction = 1;
    surf2.Construction = 1;
    surf1.FrameDivider = 1;
    surf2.FrameDivider = 2;
    surf1.Sides = 4;
    surf2.Sides = 4;
    surf1.Height = 2.0;
    surf2.Height = 2.0;
    surf1.Width = 1.0;
    surf2.Width = 1.0;
    surf1.SinAzim = 0.0;
    surf2.SinAzim = 0.0;
    surf1.CosAzim = 1.0;
    surf2.CosAzim = 1.0;
    surf1.SinTilt = 1.0;
    surf2.SinTilt = 1.0;
    surf1.CosTilt = 0.0;
    surf2.CosTilt = 0.0;
    surf1.Area = 2.0;
    surf2.Area = 2.0;
    state->dataSurface->SurfWinFrameArea(1) = 0.64;
    state->dataSurface->SurfWinFrameArea(2) = 0.64;

    state->dataSurface->SurfActiveConstruction(1) = 1;
    state->dataSurface->SurfActiveConstruction(2) = 1;

    state->dataHeatBal->TotConstructs = 1;
    state->dataConstruction->Construct.allocate(state->dataHeatBal->TotConstructs);
    auto &construct1 = state->dataConstruction->Construct(1);
    construct1.TotLayers = 1;
    construct1.LayerPoint.allocate(1);
    construct1.LayerPoint(1) = 1;
    construct1.Name = "Construction1";
    construct1.TotGlassLayers = 1;
    construct1.TransSolBeamCoef[0] = 0.9;

    auto &s_mat = state->dataMaterial;

    auto *mat1 = new Material::MaterialGlass;
    mat1->Name = "GLASS";
    mat1->group = Material::Group::Glass;
    s_mat->materials.push_back(mat1);
    mat1->Num = s_mat->materials.isize();
    s_mat->materialMap.insert_or_assign(mat1->Name, mat1->Num);

    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->Zone.allocate(1);
    state->dataHeatBal->Zone(1).spaceIndexes.allocate(1);
    state->dataHeatBal->Zone(1).spaceIndexes[0] = 1;
    state->dataHeatBal->space.allocate(1);
    state->dataHeatBal->space(1).WindowSurfaceFirst = 1;
    state->dataHeatBal->space(1).WindowSurfaceLast = 2;

    state->dataGlobal->HourOfDay = 10;
    state->dataGlobal->TimeStep = 1;
    state->dataHeatBal->SurfCosIncAng(state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep, 1) = 0.5;
    state->dataHeatBal->SurfCosIncAng(state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep, 2) = 0.5;
    state->dataHeatBal->SurfSunlitFracWithoutReveal(state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep, 1) = 1.0;
    state->dataHeatBal->SurfSunlitFracWithoutReveal(state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep, 2) = 1.0;

    Window::W5InitGlassParameters(*state);
    construct1.TransSolBeamCoef[0] = 0.9;
    state->dataSurface->SurfWinTanProfileAngVert(1) = 10.0;
    state->dataSurface->SurfWinTanProfileAngVert(2) = 10.0;
    state->dataSurface->SurfWinTanProfileAngHor(1) = 10.0;
    state->dataSurface->SurfWinTanProfileAngHor(2) = 10.0;
    state->dataEnvrn->SOLCOS(1) = 0.5;
    state->dataEnvrn->SOLCOS(2) = 0.5;
    state->dataEnvrn->SOLCOS(3) = 0.5;
    state->dataEnvrn->SunIsUp = true;
    state->dataEnvrn->DifSolarRad = 200.0;
    state->dataEnvrn->BeamSolarRad = 1000.0;
    state->dataEnvrn->GndSolarRad = 20.0;

    CalcBeamSolarOnWinRevealSurface(*state);
    EXPECT_NEAR(state->dataSurface->SurfWinBmSolAbsdInsReveal(1), 0.0326, 0.001);
    EXPECT_NEAR(state->dataSurface->SurfWinBmSolAbsdInsReveal(2), 0.0326, 0.001);

    // Case 2 add some projection to frame 2
    frameDivider2.FrameProjectionOut = 0.2;
    frameDivider2.FrameProjectionIn = 0.05;
    frameDivider2.DividerProjectionOut = 0.04;
    frameDivider2.DividerProjectionIn = 0.01;

    CalcBeamSolarOnWinRevealSurface(*state);
    EXPECT_NEAR(state->dataSurface->SurfWinBmSolAbsdInsReveal(1), 0.0326, 0.001);
    EXPECT_NEAR(state->dataSurface->SurfWinBmSolAbsdInsReveal(2), 0.0000, 0.001);

    // Case 3 less projection on frame 2
    frameDivider2.FrameProjectionOut = 0.01;
    frameDivider2.FrameProjectionIn = 0.01;
    frameDivider2.DividerProjectionOut = 0.01;
    frameDivider2.DividerProjectionIn = 0.01;

    CalcBeamSolarOnWinRevealSurface(*state);
    EXPECT_NEAR(state->dataSurface->SurfWinBmSolAbsdInsReveal(1), 0.0326, 0.001);
    EXPECT_NEAR(state->dataSurface->SurfWinBmSolAbsdInsReveal(2), 0.0225, 0.001);
}
TEST_F(EnergyPlusFixture, SolarShadingTest_CalcInteriorSolarDistribution_Detailed)
{
    // Test for detailed window model
    std::string const idf_objects = delimited_string({
        "  Building,",
        "    DemoFDT,                 !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    3.9999999E-02,           !- Loads Convergence Tolerance Value",
        "    4.0000002E-03,           !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullInteriorAndExterior,    !- Solar Distribution",
        "    ,                        !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",
        "  ShadowCalculation,",
        "    PolygonClipping,         !- Shading Calculation Method",
        "    Timestep,                !- Shading Calculation Update Frequency Method",
        "    ,                        !- Shading Calculation Update Frequency",
        "    ,                        !- Maximum Figures in Shadow Overlap Calculations",
        "    ,                        !- Polygon Clipping Algorithm",
        "    ,                        !- Pixel Counting Resolution",
        "    DetailedSkyDiffuseModeling;  !- Sky Diffuse Modeling Algorithm",
        "  Timestep,6;",
        "  ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    Continuous;              !- Numeric Type",
        "  ScheduleTypeLimits,",
        "    ON/OFF,                  !- Name",
        "    0,                       !- Lower Limit Value",
        "    1,                       !- Upper Limit Value",
        "    Discrete;                !- Numeric Type",
        "  Schedule:Compact,",
        "    SunShading,              !- Name",
        "    ON/OFF,                  !- Schedule Type Limits Name",
        "    Through: 4/30,           !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    until: 24:00,1,          !- Field 3",
        "    Through: 10/31,          !- Field 5",
        "    For: AllDays,            !- Field 6",
        "    until: 24:00,0,          !- Field 7",
        "    Through: 12/31,          !- Field 9",
        "    For: AllDays,            !- Field 10",
        "    until: 24:00,1;          !- Field 11",
        "  Material,",
        "    C12 - 2 IN HW CONCRETE,  !- Name",
        "    MediumRough,             !- Roughness",
        "    5.0901599E-02,           !- Thickness {m}",
        "    1.729577,                !- Conductivity {W/m-K}",
        "    2242.585,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",
        "  Material:NoMass,",
        "    R13LAYER,                !- Name",
        "    Rough,                   !- Roughness",
        "    2.290965,                !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",
        "  WindowMaterial:Glazing,",
        "    GLASS - CLEAR PLATE 1 / 4 IN,  !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.006,                   !- Thickness {m}",
        "    0.80,                    !- Solar Transmittance at Normal Incidence",
        "    0.10,                    !- Front Side Solar Reflectance at Normal Incidence",
        "    0.10,                    !- Back Side Solar Reflectance at Normal Incidence",
        "    0.80,                    !- Visible Transmittance at Normal Incidence",
        "    0.10,                    !- Front Side Visible Reflectance at Normal Incidence",
        "    0.10,                    !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.84,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",
        "  WindowMaterial:Gas,",
        "    AIRGAP,                  !- Name",
        "    AIR,                     !- Gas Type",
        "    0.0125;                  !- Thickness {m}",
        "  Construction,",
        "    R13WALL,                 !- Name",
        "    R13LAYER;                !- Outside Layer",
        "  Construction,",
        "    SLAB FLOOR,              !- Name",
        "    C12 - 2 IN HW CONCRETE;  !- Layer 4",
        "  Construction,",
        "    DOUBLE PANE HW WINDOW,   !- Name",
        "    GLASS - CLEAR PLATE 1 / 4 IN,  !- Outside Layer",
        "    AIRGAP,                  !- Layer 2",
        "    GLASS - CLEAR PLATE 1 / 4 IN;  !- Layer 3",
        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    Counterclockwise,        !- Vertex Entry Direction",
        "    Relative;                !- Coordinate System",
        "  Zone,",
        "    ZONE ONE;                !- Name",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-North,        !- Name",
        "    Wall,                    !- Surface Type",
        "    R13WALL,             !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    5,5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    -5,5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-East,         !- Name",
        "    Wall,                    !- Surface Type",
        "    R13WALL,               !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    5,-5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    5,5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-South,        !- Name",
        "    Wall,                    !- Surface Type",
        "    R13WALL,                 !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,-5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -5,-5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    5,-5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-West,         !- Name",
        "    Wall,                    !- Surface Type",
        "    R13WALL,               !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -5,5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    -5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,-5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:roof,              !- Name",
        "    Roof,                    !- Surface Type",
        "    R13WALL,                  !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.0000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,-5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,-5,3,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,5,3,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:floor,             !- Name",
        "    Floor,                   !- Surface Type",
        "    SLAB FLOOR,              !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.0000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,-5,0;  !- X,Y,Z ==> Vertex 4 {m}",
        "  FenestrationSurface:Detailed,",
        "    Zn001:Floor:Win001, !- Name",
        "    Window,                  !- Surface Type",
        "    DOUBLE PANE HW WINDOW,   !- Construction Name",
        "    Zn001:Floor,        !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    TestFrameAndDivider,     !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    -4,4,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    4,4,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    4,-4,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -4,-4,0;  !- X,Y,Z ==> Vertex 4 {m}",
        "  FenestrationSurface:Detailed,",
        "    Zn001:Wall-South:Win001, !- Name",
        "    Window,                  !- Surface Type",
        "    DOUBLE PANE HW WINDOW,   !- Construction Name",
        "    Zn001:Wall-South,        !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    TestFrameAndDivider,     !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-5,0.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-5,0.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;  !- X,Y,Z ==> Vertex 4 {m}",
        "  WindowProperty:FrameAndDivider,",
        "    TestFrameAndDivider,     !- Name",
        "    0.05,                    !- Frame Width {m}",
        "    0.05,                    !- Frame Outside Projection {m}",
        "    0.05,                    !- Frame Inside Projection {m}",
        "    5.0,                     !- Frame Conductance {W/m2-K}",
        "    1.2,                     !- Ratio of Frame-Edge Glass Conductance to Center-Of-Gl",
        "    0.8,                     !- Frame Solar Absorptance",
        "    0.8,                     !- Frame Visible Absorptance",
        "    0.9,                     !- Frame Thermal Hemispherical Emissivity",
        "    DividedLite,             !- Divider Type",
        "    0.02,                    !- Divider Width {m}",
        "    2,                       !- Number of Horizontal Dividers",
        "    2,                       !- Number of Vertical Dividers",
        "    0.02,                    !- Divider Outside Projection {m}",
        "    0.02,                    !- Divider Inside Projection {m}",
        "    5.0,                     !- Divider Conductance {W/m2-K}",
        "    1.2,                     !- Ratio of Divider-Edge Glass Conductance to Center-Of-",
        "    0.8,                     !- Divider Solar Absorptance",
        "    0.8,                     !- Divider Visible Absorptance",
        "    0.9;                     !- Divider Thermal Hemispherical Emissivity",
        "  Shading:Zone:Detailed,",
        "    Zn001:Wall-South:Shade001,  !- Name",
        "    Zn001:Wall-South,        !- Base Surface Name",
        "    SunShading,              !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-6,2.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-6,2.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;  !- X,Y,Z ==> Vertex 4 {m}",
        "  ShadingProperty:Reflectance,",
        "    Zn001:Wall-South:Shade001,  !- Shading Surface Name",
        "    0.2,                     !- Diffuse Solar Reflectance of Unglazed Part of Shading",
        "    0.2;                     !- Diffuse Visible Reflectance of Unglazed Part of Shading",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool FoundError = false;

    HeatBalanceManager::GetProjectControlData(*state, FoundError); // read project control data
    EXPECT_FALSE(FoundError);                                      // expect no errors

    HeatBalanceManager::SetPreConstructionInputParameters(*state);
    HeatBalanceManager::GetHeatBalanceInput(*state);

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(1);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(1);

    state->dataSurfaceGeometry->CosZoneRelNorth(1) = std::cos(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(1) = std::sin(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;

    SolarShading::AllocateModuleArrays(*state);
    SolarShading::DetermineShadowingCombinations(*state);
    state->dataEnvrn->DayOfYear_Schedule = 168;
    state->dataEnvrn->DayOfWeek = 6;
    state->dataGlobal->TimeStep = 4;
    state->dataGlobal->HourOfDay = 9;

    //	compare_err_stream( "" ); // just for debugging

    state->dataSurface->ShadingTransmittanceVaries = true;
    state->dataSysVars->DetailedSkyDiffuseAlgorithm = true;
    state->dataHeatBal->SolarDistribution = DataHeatBalance::Shadowing::FullExterior;

    state->dataSolarShading->CalcSkyDifShading = true;
    state->dataGlobal->BeginSimFlag = true;
    HeatBalanceManager::InitHeatBalance(*state);
    EnergyPlus::createFacilityElectricPowerServiceObject(*state);
    HeatBalanceSurfaceManager::InitSurfaceHeatBalance(*state);
    SolarShading::InitSolarCalculations(*state);

    state->dataBSDFWindow->SUNCOSTS(4, 9)(1) = 0.1;
    state->dataBSDFWindow->SUNCOSTS(4, 9)(2) = 0.1;
    state->dataBSDFWindow->SUNCOSTS(4, 9)(3) = 0.1;
    SolarShading::FigureSolarBeamAtTimestep(*state, state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep);

    int windowSurfNum = Util::FindItemInList("ZN001:WALL-SOUTH:WIN001", state->dataSurface->Surface);
    int windowSurfNum2 = Util::FindItemInList("ZN001:FLOOR:WIN001", state->dataSurface->Surface);
    EXPECT_NEAR(0.6504, state->dataSolarShading->SurfDifShdgRatioIsoSkyHRTS(4, 9, windowSurfNum), 0.0001);
    EXPECT_NEAR(0.9152, state->dataSolarShading->SurfDifShdgRatioHorizHRTS(4, 9, windowSurfNum), 0.0001);

    state->dataEnvrn->BeamSolarRad = 100.0;
    state->dataHeatBal->SurfSunlitFrac(state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep, windowSurfNum) = 1.0;
    state->dataHeatBal->SurfCosIncAng(state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep, windowSurfNum) = 1.0;
    state->dataSurface->SurfWinGlTsolBmBm(windowSurfNum) = 0.8;
    state->dataSurface->SurfWinGlTsolBmBm(windowSurfNum2) = 0.5;
    state->dataSurface->SurfSkySolarInc(windowSurfNum) = 50.0;
    state->dataSurface->SurfGndSolarInc(windowSurfNum) = 25.0;

    SolarShading::CalcInteriorSolarDistribution(*state);
    EXPECT_NEAR(1239.94, state->dataSurface->SurfWinTransSolar(windowSurfNum), 0.01);
    EXPECT_NEAR(0.0, state->dataSurface->SurfWinTransSolar(windowSurfNum2), 0.01);
    EXPECT_NEAR(0.646, state->dataSurface->SurfWinGlTsolBmBm(windowSurfNum), 0.01);
    EXPECT_NEAR(0.0, state->dataSurface->SurfWinGlTsolBmBm(windowSurfNum2), 0.01);
    EXPECT_NEAR(0.0, state->dataHeatBalSurf->SurfWinInitialBeamSolInTrans(windowSurfNum), 0.01);
    EXPECT_NEAR(0.0, state->dataHeatBalSurf->SurfWinInitialBeamSolInTrans(windowSurfNum2), 0.01);
    EXPECT_NEAR(0.0, state->dataHeatBalSurf->SurfWinInitialDifSolInTrans(windowSurfNum), 0.01);
    EXPECT_NEAR(1.2, state->dataHeatBalSurf->SurfWinInitialDifSolInTrans(windowSurfNum2), 0.01);
}
TEST_F(EnergyPlusFixture, SolarShadingTest_CalcInteriorSolarDistribution_EQL)
{
    // Test for equivalent layer window model
    std::string const idf_objects = delimited_string({
        "  Building,",
        "    DemoFDT,                 !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    3.9999999E-02,           !- Loads Convergence Tolerance Value",
        "    4.0000002E-03,           !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullInteriorAndExterior,    !- Solar Distribution",
        "    ,                        !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",
        "  ShadowCalculation,",
        "    PolygonClipping,         !- Shading Calculation Method",
        "    Timestep,                !- Shading Calculation Update Frequency Method",
        "    ,                        !- Shading Calculation Update Frequency",
        "    ,                        !- Maximum Figures in Shadow Overlap Calculations",
        "    ,                        !- Polygon Clipping Algorithm",
        "    ,                        !- Pixel Counting Resolution",
        "    DetailedSkyDiffuseModeling;  !- Sky Diffuse Modeling Algorithm",
        "  Timestep,6;",
        "  ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    Continuous;              !- Numeric Type",
        "  ScheduleTypeLimits,",
        "    ON/OFF,                  !- Name",
        "    0,                       !- Lower Limit Value",
        "    1,                       !- Upper Limit Value",
        "    Discrete;                !- Numeric Type",
        "  Schedule:Compact,",
        "    SunShading,              !- Name",
        "    ON/OFF,                  !- Schedule Type Limits Name",
        "    Through: 4/30,           !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    until: 24:00,1,          !- Field 3",
        "    Through: 10/31,          !- Field 5",
        "    For: AllDays,            !- Field 6",
        "    until: 24:00,0,          !- Field 7",
        "    Through: 12/31,          !- Field 9",
        "    For: AllDays,            !- Field 10",
        "    until: 24:00,1;          !- Field 11",
        "  Material,",
        "    C12 - 2 IN HW CONCRETE,  !- Name",
        "    MediumRough,             !- Roughness",
        "    5.0901599E-02,           !- Thickness {m}",
        "    1.729577,                !- Conductivity {W/m-K}",
        "    2242.585,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",
        "  Material:NoMass,",
        "    R13LAYER,                !- Name",
        "    Rough,                   !- Roughness",
        "    2.290965,                !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",
        "  Construction:WindowEquivalentLayer,",
        "  CLR CLR VB,                !- Name",
        "  GLZCLR,                    !- Outside Layer",
        "  Air GAP SealedOut 20mm,    !- Layer 2",
        "  GLZCLR;                    !- Layer 3",
        "  ! Air GAP SealedIndoor 20mm, !- Layer 4",
        "  ! VBU8D6+45SW1;              !- Layer 5",
        "WindowMaterial:Glazing:EquivalentLayer,",
        "  GLZCLR,                    !-  Name",
        "  SpectralAverage,           !-  Optical Data Type",
        "  ,                          !-  Window Glass Spectral Data Set Name",
        "  0.83,                      !-  Front Side Beam-Beam Solar Transmittance",
        "  0.83,                      !-  Back Side Beam-Beam Solar Transmittance",
        "  0.08,                      !-  Front Side Beam-Beam Solar Reflectance",
        "  0.08,                      !-  Back Side Beam-Beam Solar Reflectance",
        "  0.0,                       !-  Front Side Beam-Beam Visible Transmittance",
        "  0.0,                       !-  Back Side Beam-Beam Visible Transmittance",
        "  0.0,                       !-  Front Side Beam-Beam Visible Reflectance",
        "  0.0,                       !-  Back Side Beam-Beam Visible Reflectance",
        "  0.0,                       !-  Front Side Beam-Diffuse Solar Transmittance",
        "  0.0,                       !-  Back Side Beam-Diffuse Solar Transmittance",
        "  0.0,                       !-  Front Side Beam-Diffuse Solar Reflectance",
        "  0.0,                       !-  Back Side Beam-Diffuse Solar Reflectance",
        "  0.0,                       !-  Front Side Beam-Diffuse Visible Transmittance",
        "  0.0,                       !-  Back Side Beam-Diffuse Visible Transmittance",
        "  0.0,                       !-  Front Side Beam-Diffuse Visible Reflectance",
        "  0.0,                       !-  Back Side Beam-Diffuse Visible Reflectance",
        "  0.76,                      !-  Diffuse-Diffuse Solar Transmittance",
        "  0.14,                      !-  Front Side Diffuse-Diffuse Solar Reflectance",
        "  0.14,                      !-  Back Side Diffuse-Diffuse Solar Reflectance",
        "  0.0,                       !-  Diffuse-Diffuse Visible Transmittance",
        "  0.0,                       !-  Front Side Diffuse-Diffuse Visible Reflectance",
        "  0.0,                       !-  Back Side Diffuse-Diffuse Visible Reflectance",
        "  0.0,                       !-  Infrared Transmittance (front and back)",
        "  0.84,                      !-  Front Side Infrared Emissivity",
        "  0.84;                      !-  Back Side Infrared Emissivity",
        "WindowMaterial:Blind:EquivalentLayer,",
        "  VBU8D6+45SW1,           ! - Name",
        "  Horizontal,             ! - Slat Orientation",
        "  0.025,                  ! - Slat Width",
        "  0.025,                  ! - Slat Separation",
        "  0.0,                    ! - Slat Crown",
        "  45.0,                   ! - Slat Angle",
        "  0.0,                    ! - Front Side Slat Beam-Diffuse Solar Transmittance",
        "  0.0,                    ! - Back Side Slat Beam-Diffuse Solar Transmittance",
        "  0.0,                    ! - Front Side Slat Beam-Diffuse Solar Reflectance",
        "  0.0,                    ! - Back Side Slat Beam-Diffuse Solar Reflectance",
        "  0.0,                    ! - Front Side Slat Beam-Diffuse Visible Solar Transmittance",
        "  0.0,                    ! - Back Side Slat Beam-Diffuse Visible Solar Transmittance",
        "  0.0,                    ! - Front Side Slat Beam-Diffuse Visible Solar Reflectance",
        "  0.0,                    ! - Back Side Slat Beam-Diffuse Visible Solar Reflectance",
        "  0.0,                    ! - Slat Diffuse-Diffuse Solar Transmittance",
        "  0.80,                   ! - Front Side Slat Diffuse-Diffuse Solar Reflectance",
        "  0.60,                   ! - Back Side Slat Diffuse-Diffuse Solar Reflectance",
        "  0.0,                    ! - Slat Diffuse-Diffuse Visible Transmittance",
        "  0.0,                    ! - Front Side Slat Diffuse-Diffuse Visible Reflectance",
        "  0.0,                    ! - Back Side Slat Diffuse-Diffuse Visible Reflectance",
        "  0.0,                    ! - Slat Infrared Transmittance",
        "  0.90,                   ! - Front Side Slat Infrared Emissivity",
        "  0.90,                   ! - Back Side Slat Infrared Emissivity",
        "  BlockBeamSolar;         ! - Slat Angle Control",
        " WindowMaterial:Gap:EquivalentLayer,",
        "  Air GAP SealedOut 20mm,    !- Name",
        "  Air,                       !- Gas Type",
        "  0.0200,                    !- Thickness",
        "  Sealed;                    !- Gap Vent Type",
        " WindowMaterial:Gap:EquivalentLayer,",
        "  Air GAP SealedIndoor 20mm, !- Name",
        "  Air,                       !- Gas Type",
        "  0.020,                     !- Thickness",
        "  Sealed;                    !- Gap Vent Type ",
        "  Construction,",
        "    R13WALL,                 !- Name",
        "    R13LAYER;                !- Outside Layer",
        "  Construction,",
        "    SLAB FLOOR,              !- Name",
        "    C12 - 2 IN HW CONCRETE;  !- Layer 4",
        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    Counterclockwise,        !- Vertex Entry Direction",
        "    Relative;                !- Coordinate System",
        "  Zone,",
        "    ZONE ONE;                !- Name",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-North,        !- Name",
        "    Wall,                    !- Surface Type",
        "    R13WALL,             !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    5,5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    -5,5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-East,         !- Name",
        "    Wall,                    !- Surface Type",
        "    R13WALL,               !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    5,-5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    5,5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-South,        !- Name",
        "    Wall,                    !- Surface Type",
        "    R13WALL,                 !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,-5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -5,-5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    5,-5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:Wall-West,         !- Name",
        "    Wall,                    !- Surface Type",
        "    R13WALL,               !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -5,5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    -5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,-5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:roof,              !- Name",
        "    Roof,                    !- Surface Type",
        "    R13WALL,                  !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.0000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,-5,3,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,-5,3,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,5,3,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,5,3;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Zn001:floor,             !- Name",
        "    Floor,                   !- Surface Type",
        "    SLAB FLOOR,              !- Construction Name",
        "    ZONE ONE,                !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.0000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    -5,5,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5,5,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5,-5,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -5,-5,0;  !- X,Y,Z ==> Vertex 4 {m}",
        "  FenestrationSurface:Detailed,",
        "    Zn001:Floor:Win001, !- Name",
        "    Window,                  !- Surface Type",
        "    CLR CLR VB,   !- Construction Name",
        "    Zn001:Floor,        !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    TestFrameAndDivider,     !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    -4,4,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    4,4,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    4,-4,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    -4,-4,0;  !- X,Y,Z ==> Vertex 4 {m}",
        "  FenestrationSurface:Detailed,",
        "    Zn001:Wall-South:Win001, !- Name",
        "    Window,                  !- Surface Type",
        "    CLR CLR VB,   !- Construction Name",
        "    Zn001:Wall-South,        !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    TestFrameAndDivider,     !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-5,0.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-5,0.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;  !- X,Y,Z ==> Vertex 4 {m}",
        "  WindowProperty:FrameAndDivider,",
        "    TestFrameAndDivider,     !- Name",
        "    0.05,                    !- Frame Width {m}",
        "    0.05,                    !- Frame Outside Projection {m}",
        "    0.05,                    !- Frame Inside Projection {m}",
        "    5.0,                     !- Frame Conductance {W/m2-K}",
        "    1.2,                     !- Ratio of Frame-Edge Glass Conductance to Center-Of-Gl",
        "    0.8,                     !- Frame Solar Absorptance",
        "    0.8,                     !- Frame Visible Absorptance",
        "    0.9,                     !- Frame Thermal Hemispherical Emissivity",
        "    DividedLite,             !- Divider Type",
        "    0.02,                    !- Divider Width {m}",
        "    2,                       !- Number of Horizontal Dividers",
        "    2,                       !- Number of Vertical Dividers",
        "    0.02,                    !- Divider Outside Projection {m}",
        "    0.02,                    !- Divider Inside Projection {m}",
        "    5.0,                     !- Divider Conductance {W/m2-K}",
        "    1.2,                     !- Ratio of Divider-Edge Glass Conductance to Center-Of-",
        "    0.8,                     !- Divider Solar Absorptance",
        "    0.8,                     !- Divider Visible Absorptance",
        "    0.9;                     !- Divider Thermal Hemispherical Emissivity",
        "  Shading:Zone:Detailed,",
        "    Zn001:Wall-South:Shade001,  !- Name",
        "    Zn001:Wall-South,        !- Base Surface Name",
        "    SunShading,              !- Transmittance Schedule Name",
        "    4,                       !- Number of Vertices",
        "    -3,-5,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    -3,-6,2.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    3,-6,2.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    3,-5,2.5;  !- X,Y,Z ==> Vertex 4 {m}",
        "  ShadingProperty:Reflectance,",
        "    Zn001:Wall-South:Shade001,  !- Shading Surface Name",
        "    0.2,                     !- Diffuse Solar Reflectance of Unglazed Part of Shading",
        "    0.2;                     !- Diffuse Visible Reflectance of Unglazed Part of Shading",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool FoundError = false;

    HeatBalanceManager::GetProjectControlData(*state, FoundError); // read project control data
    EXPECT_FALSE(FoundError);                                      // expect no errors

    HeatBalanceManager::SetPreConstructionInputParameters(*state);
    HeatBalanceManager::GetHeatBalanceInput(*state);

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(1);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(1);

    state->dataSurfaceGeometry->CosZoneRelNorth(1) = std::cos(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(1) = std::sin(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;

    SolarShading::AllocateModuleArrays(*state);
    SolarShading::DetermineShadowingCombinations(*state);
    state->dataEnvrn->DayOfYear_Schedule = 168;
    state->dataEnvrn->DayOfWeek = 6;
    state->dataGlobal->TimeStep = 4;
    state->dataGlobal->HourOfDay = 9;

    //	compare_err_stream( "" ); // just for debugging

    state->dataSurface->ShadingTransmittanceVaries = true;
    state->dataSysVars->DetailedSkyDiffuseAlgorithm = true;
    state->dataHeatBal->SolarDistribution = DataHeatBalance::Shadowing::FullExterior;

    state->dataSolarShading->CalcSkyDifShading = true;
    state->dataGlobal->BeginSimFlag = true;
    HeatBalanceManager::InitHeatBalance(*state);
    EnergyPlus::createFacilityElectricPowerServiceObject(*state);
    HeatBalanceSurfaceManager::InitSurfaceHeatBalance(*state);
    SolarShading::InitSolarCalculations(*state);

    state->dataBSDFWindow->SUNCOSTS(4, 9)(1) = 0.1;
    state->dataBSDFWindow->SUNCOSTS(4, 9)(2) = 0.1;
    state->dataBSDFWindow->SUNCOSTS(4, 9)(3) = 0.1;
    SolarShading::FigureSolarBeamAtTimestep(*state, state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep);

    int windowSurfNum = Util::FindItemInList("ZN001:WALL-SOUTH:WIN001", state->dataSurface->Surface);
    int windowSurfNum2 = Util::FindItemInList("ZN001:FLOOR:WIN001", state->dataSurface->Surface);
    EXPECT_NEAR(0.6504, state->dataSolarShading->SurfDifShdgRatioIsoSkyHRTS(4, 9, windowSurfNum), 0.0001);
    EXPECT_NEAR(0.9152, state->dataSolarShading->SurfDifShdgRatioHorizHRTS(4, 9, windowSurfNum), 0.0001);

    state->dataEnvrn->BeamSolarRad = 100.0;
    state->dataHeatBal->SurfSunlitFrac(state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep, windowSurfNum) = 1.0;
    state->dataHeatBal->SurfCosIncAng(state->dataGlobal->HourOfDay, state->dataGlobal->TimeStep, windowSurfNum) = 1.0;
    state->dataSurface->SurfWinGlTsolBmBm(windowSurfNum) = 0.8;
    state->dataSurface->SurfWinGlTsolBmBm(windowSurfNum2) = 0.5;
    state->dataSurface->SurfSkySolarInc(windowSurfNum) = 50.0;
    state->dataSurface->SurfGndSolarInc(windowSurfNum) = 25.0;

    SolarShading::CalcInteriorSolarDistribution(*state);
    EXPECT_NEAR(1362.24, state->dataSurface->SurfWinTransSolar(windowSurfNum), 0.01);
    EXPECT_NEAR(0.0, state->dataSurface->SurfWinTransSolar(windowSurfNum2), 0.01);
    EXPECT_NEAR(0.6933, state->dataSurface->SurfWinGlTsolBmBm(windowSurfNum), 0.01);
    EXPECT_NEAR(0.0, state->dataSurface->SurfWinGlTsolBmBm(windowSurfNum2), 0.01);
    EXPECT_NEAR(0.0, state->dataHeatBalSurf->SurfWinInitialBeamSolInTrans(windowSurfNum), 0.01);
    EXPECT_NEAR(0.0, state->dataHeatBalSurf->SurfWinInitialBeamSolInTrans(windowSurfNum2), 0.01);
    EXPECT_NEAR(0.0, state->dataHeatBalSurf->SurfWinInitialDifSolInTrans(windowSurfNum), 0.01);
    EXPECT_NEAR(1.4736, state->dataHeatBalSurf->SurfWinInitialDifSolInTrans(windowSurfNum2), 0.01);
}

TEST_F(EnergyPlusFixture, SolarShadingTest_GetShadowingInputTest1)
{
    // Tests for Defect #10299: Test GetShadowingInput for various combinations of input
    // with a focus put on the correct setting of variables associated with calculation
    // method and polygon clipping algorithm
    std::string const idf_objects = delimited_string({
        "  ShadowCalculation,",
        "    PolygonClipping,             !- Shading Calculation Method",
        "    Timestep,                    !- Shading Calculation Update Frequency Method",
        "    1,                           !- Shading Calculation Update Frequency",
        "    200,                         !- Maximum Figures in Shadow Overlap Calculations",
        "    ConvexWeilerAtherton,        !- Polygon Clipping Algorithm",
        "    512.0,                       !- Pixel Counting Resolution",
        "    DetailedSkyDiffuseModeling;  !- Sky Diffuse Modeling Algorithm",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataSolarShading->anyScheduledShadingSurface = false;

    // Test 1 of 6: Polygon Clipping and ConvexWeilerAtherton
    SolarShading::GetShadowingInput(*state);
    int expectedFrequency = 1;
    int expectedOverlaps = 200;
    EXPECT_TRUE(state->dataSysVars->DetailedSkyDiffuseAlgorithm);
    EXPECT_TRUE(state->dataSysVars->DetailedSolarTimestepIntegration);
    EXPECT_EQ(expectedFrequency, state->dataSolarShading->ShadowingCalcFrequency);
    EXPECT_EQ(expectedOverlaps, state->dataSolarShading->MaxHCS);
    EXPECT_FALSE(state->dataSysVars->SutherlandHodgman);
    EXPECT_FALSE(state->dataSysVars->SlaterBarsky);
    EXPECT_ENUM_EQ(state->dataSysVars->shadingMethod, ShadingMethod::PolygonClipping);
}

TEST_F(EnergyPlusFixture, SolarShadingTest_GetShadowingInputTest2)
{
    // Tests for Defect #10299: Test GetShadowingInput for various combinations of input
    // with a focus put on the correct setting of variables associated with calculation
    // method and polygon clipping algorithm
    std::string const idf_objects = delimited_string({
        "  ShadowCalculation,",
        "    PolygonClipping,           !- Shading Calculation Method",
        "    Periodic,                  !- Shading Calculation Update Frequency Method",
        "    10,                        !- Shading Calculation Update Frequency",
        "    2000,                      !- Maximum Figures in Shadow Overlap Calculations",
        "    SutherlandHodgman,         !- Polygon Clipping Algorithm",
        "    512.0,                     !- Pixel Counting Resolution",
        "    SimpleSkyDiffuseModeling;  !- Sky Diffuse Modeling Algorithm",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataSolarShading->anyScheduledShadingSurface = false;

    // Test 2 of 6: Polygon Clipping and SutherlandHodgman
    SolarShading::GetShadowingInput(*state);
    int expectedFrequency = 10;
    int expectedOverlaps = 2000;
    EXPECT_FALSE(state->dataSysVars->DetailedSkyDiffuseAlgorithm);
    EXPECT_FALSE(state->dataSysVars->DetailedSolarTimestepIntegration);
    EXPECT_EQ(expectedFrequency, state->dataSolarShading->ShadowingCalcFrequency);
    EXPECT_EQ(expectedOverlaps, state->dataSolarShading->MaxHCS);
    EXPECT_TRUE(state->dataSysVars->SutherlandHodgman);
    EXPECT_FALSE(state->dataSysVars->SlaterBarsky);
    EXPECT_ENUM_EQ(state->dataSysVars->shadingMethod, ShadingMethod::PolygonClipping);
}

TEST_F(EnergyPlusFixture, SolarShadingTest_GetShadowingInputTest3)
{
    // Tests for Defect #10299: Test GetShadowingInput for various combinations of input
    // with a focus put on the correct setting of variables associated with calculation
    // method and polygon clipping algorithm
    std::string const idf_objects = delimited_string({
        "  ShadowCalculation,",
        "    PolygonClipping,                   !- Shading Calculation Method",
        "    Timestep,                          !- Shading Calculation Update Frequency Method",
        "    30,                                !- Shading Calculation Update Frequency",
        "    15000,                             !- Maximum Figures in Shadow Overlap Calculations",
        "    SlaterBarskyandSutherlandHodgman,  !- Polygon Clipping Algorithm",
        "    512.0,                             !- Pixel Counting Resolution",
        "    DetailedSkyDiffuseModeling;        !- Sky Diffuse Modeling Algorithm",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->dataSolarShading->anyScheduledShadingSurface = false;

    // Test 3 of 6: Polygon Clipping and SlaterBarskyandSutherlandHodgman
    SolarShading::GetShadowingInput(*state);
    int expectedFrequency = 30;
    int expectedOverlaps = 15000;
    EXPECT_TRUE(state->dataSysVars->DetailedSkyDiffuseAlgorithm);
    EXPECT_TRUE(state->dataSysVars->DetailedSolarTimestepIntegration);
    EXPECT_EQ(expectedFrequency, state->dataSolarShading->ShadowingCalcFrequency);
    EXPECT_EQ(expectedOverlaps, state->dataSolarShading->MaxHCS);
    EXPECT_TRUE(state->dataSysVars->SutherlandHodgman);
    EXPECT_TRUE(state->dataSysVars->SlaterBarsky);
    EXPECT_ENUM_EQ(state->dataSysVars->shadingMethod, ShadingMethod::PolygonClipping);
}

TEST_F(EnergyPlusFixture, SolarShadingTest_GetShadowingInputTest4)
{
    // Tests for Defect #10299: Test GetShadowingInput for various combinations of input
    // with a focus put on the correct setting of variables associated with calculation
    // method and polygon clipping algorithm
    std::string const idf_objects = delimited_string({
        "  ShadowCalculation,",
        "    PixelCounting,               !- Shading Calculation Method",
        "    Periodic,                    !- Shading Calculation Update Frequency Method",
        "    1,                           !- Shading Calculation Update Frequency",
        "    200,                         !- Maximum Figures in Shadow Overlap Calculations",
        "    ConvexWeilerAtherton,        !- Polygon Clipping Algorithm",
        "    512.0,                       !- Pixel Counting Resolution",
        "    DetailedSkyDiffuseModeling;  !- Sky Diffuse Modeling Algorithm",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->dataSolarShading->anyScheduledShadingSurface = false;

    // Test 4 of 6: Pixel Counting and ConvexWeilerAtherton
    SolarShading::GetShadowingInput(*state);
    int expectedFrequency = 1;
    int expectedOverlaps = 200;
    EXPECT_TRUE(state->dataSysVars->DetailedSkyDiffuseAlgorithm);
    EXPECT_FALSE(state->dataSysVars->DetailedSolarTimestepIntegration);
    EXPECT_EQ(expectedFrequency, state->dataSolarShading->ShadowingCalcFrequency);
    EXPECT_EQ(expectedOverlaps, state->dataSolarShading->MaxHCS);
    EXPECT_FALSE(state->dataSysVars->SutherlandHodgman);
    EXPECT_FALSE(state->dataSysVars->SlaterBarsky);

#ifdef EP_NO_OPENGL
    EXPECT_ENUM_EQ(state->dataSysVars->shadingMethod, ShadingMethod::PolygonClipping);
#else
    if (!Penumbra::Penumbra::is_valid_context()) {
        EXPECT_ENUM_EQ(state->dataSysVars->shadingMethod, ShadingMethod::PolygonClipping);
    } else {
        EXPECT_ENUM_EQ(state->dataSysVars->shadingMethod, ShadingMethod::PixelCounting);
    }
#endif
}

TEST_F(EnergyPlusFixture, SolarShadingTest_GetShadowingInputTest5)
{
    // Tests for Defect #10299: Test GetShadowingInput for various combinations of input
    // with a focus put on the correct setting of variables associated with calculation
    // method and polygon clipping algorithm
    std::string const idf_objects = delimited_string({
        "  ShadowCalculation,",
        "    PixelCounting,               !- Shading Calculation Method",
        "    Periodic,                    !- Shading Calculation Update Frequency Method",
        "    10,                          !- Shading Calculation Update Frequency",
        "    2000,                        !- Maximum Figures in Shadow Overlap Calculations",
        "    SutherlandHodgman,           !- Polygon Clipping Algorithm",
        "    512.0,                       !- Pixel Counting Resolution",
        "    DetailedSkyDiffuseModeling;  !- Sky Diffuse Modeling Algorithm",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->dataSolarShading->anyScheduledShadingSurface = false;

    // Test 5 of 6: Pixel Counting and SutherlandHodgman
    SolarShading::GetShadowingInput(*state);
    int expectedFrequency = 10;
    int expectedOverlaps = 2000;
    EXPECT_TRUE(state->dataSysVars->DetailedSkyDiffuseAlgorithm);
    EXPECT_FALSE(state->dataSysVars->DetailedSolarTimestepIntegration);
    EXPECT_EQ(expectedFrequency, state->dataSolarShading->ShadowingCalcFrequency);
    EXPECT_EQ(expectedOverlaps, state->dataSolarShading->MaxHCS);
    EXPECT_TRUE(state->dataSysVars->SutherlandHodgman);
    EXPECT_FALSE(state->dataSysVars->SlaterBarsky);

#ifdef EP_NO_OPENGL
    EXPECT_ENUM_EQ(state->dataSysVars->shadingMethod, ShadingMethod::PolygonClipping);
#else
    if (!Penumbra::Penumbra::is_valid_context()) {
        EXPECT_ENUM_EQ(state->dataSysVars->shadingMethod, ShadingMethod::PolygonClipping);
    } else {
        EXPECT_ENUM_EQ(state->dataSysVars->shadingMethod, ShadingMethod::PixelCounting);
    }
#endif
}

TEST_F(EnergyPlusFixture, SolarShadingTest_GetShadowingInputTest6)
{
    // Tests for Defect #10299: Test GetShadowingInput for various combinations of input
    // with a focus put on the correct setting of variables associated with calculation
    // method and polygon clipping algorithm
    std::string const idf_objects = delimited_string({
        "  ShadowCalculation,",
        "    PixelCounting,                     !- Shading Calculation Method",
        "    Periodic,                          !- Shading Calculation Update Frequency Method",
        "    56,                                !- Shading Calculation Update Frequency",
        "    1234,                              !- Maximum Figures in Shadow Overlap Calculations",
        "    SlaterBarskyandSutherlandHodgman,  !- Polygon Clipping Algorithm",
        "    512.0,                             !- Pixel Counting Resolution",
        "    SimpleSkyDiffuseModeling;          !- Sky Diffuse Modeling Algorithm",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataSolarShading->anyScheduledShadingSurface = false;

    // Test 36of 6: Pixel Counting and SlaterBarskyandSutherlandHodgman
    SolarShading::GetShadowingInput(*state);

    int expectedFrequency = 56;
    int expectedOverlaps = 1234;
    EXPECT_FALSE(state->dataSysVars->DetailedSkyDiffuseAlgorithm);
    EXPECT_FALSE(state->dataSysVars->DetailedSolarTimestepIntegration);
    EXPECT_EQ(expectedFrequency, state->dataSolarShading->ShadowingCalcFrequency);
    EXPECT_EQ(expectedOverlaps, state->dataSolarShading->MaxHCS);
    EXPECT_TRUE(state->dataSysVars->SutherlandHodgman);
    EXPECT_TRUE(state->dataSysVars->SlaterBarsky);

#ifdef EP_NO_OPENGL
    std::string const error_string =
        delimited_string({"   ** Warning ** ShadowCalculation: suspect Shading Calculation Update Frequency",
                          "   **   ~~~   ** Value entered=[56], Shadowing Calculations will be inaccurate.",
                          "   ** Warning ** ShadowCalculation: invalid Shading Calculation Method",
                          "   **   ~~~   ** Value entered=\"PixelCounting\"",
                          "   **   ~~~   ** This version of EnergyPlus was not compiled to use OpenGL (required for PixelCounting)",
                          "   **   ~~~   ** PolygonClipping will be used instead"});
    EXPECT_TRUE(compare_err_stream(error_string, true));
    EXPECT_ENUM_EQ(state->dataSysVars->shadingMethod, ShadingMethod::PolygonClipping);

#else
    if (!Penumbra::Penumbra::is_valid_context()) {
        std::string const error_string = delimited_string({"   ** Warning ** ShadowCalculation: suspect Shading Calculation Update Frequency",
                                                           "   **   ~~~   ** Value entered=[56], Shadowing Calculations will be inaccurate.",
                                                           "   ** Warning ** No GPU found (required for PixelCounting)",
                                                           "   **   ~~~   ** PolygonClipping will be used instead"});
        EXPECT_TRUE(compare_err_stream(error_string, true));
        EXPECT_ENUM_EQ(state->dataSysVars->shadingMethod, ShadingMethod::PolygonClipping);
    } else {
        std::string const error_string = delimited_string({"   ** Warning ** ShadowCalculation: suspect Shading Calculation Update Frequency",
                                                           "   **   ~~~   ** Value entered=[56], Shadowing Calculations will be inaccurate."});
        EXPECT_TRUE(compare_err_stream(error_string, true));
        EXPECT_ENUM_EQ(state->dataSysVars->shadingMethod, ShadingMethod::PixelCounting);
    }
#endif
}

TEST_F(EnergyPlusFixture, CLIPLINE_Throw)
{
    Real64 constexpr minX = 2.0;
    Real64 constexpr maxX = 8.0;
    Real64 constexpr minY = 3.0;
    Real64 constexpr maxY = 6.0;

    Real64 x0 = maxX;
    Real64 x1 = maxX;
    Real64 y0 = 4.5;
    Real64 y1 = 1.0;
    bool visible = false;

    EXPECT_NO_THROW(CLIPLINE(x0, x1, y0, y1, maxX, minX, maxY, minY, visible));

    EXPECT_DOUBLE_EQ(maxX, x0);
    EXPECT_DOUBLE_EQ(4.5, y0);
    EXPECT_DOUBLE_EQ(maxX, x1);
    EXPECT_DOUBLE_EQ(minY, y1); // This is NaN
}

TEST_F(EnergyPlusFixture, CLIPLINE_Full)
{
    Real64 constexpr minX = 2.0;
    Real64 constexpr maxX = 8.0;
    Real64 constexpr minY = 3.0;
    Real64 constexpr maxY = 6.0;

    Real64 constexpr below_x = 0.0;
    Real64 constexpr center_x = 5.0;
    Real64 constexpr greater_x = 10.0;

    Real64 constexpr below_y = 1.0;
    Real64 constexpr center_y = 4.5;
    Real64 constexpr greater_y = 9.0;

    struct Point
    {
        Real64 x = 0.0;
        Real64 y = 0.0;
    };

    struct Line
    {
        Point p0{};
        Point p1{};
    };

    struct TestCase
    {
        Line line_ori;
        bool visible = false;
        Line line_new{}; // Only defined if visible
    };

    // No need to capture the constexpr variables (but MSVC will complain)
    auto testclipline = [&](const TestCase &t) {
        Real64 x0 = t.line_ori.p0.x;
        Real64 y0 = t.line_ori.p0.y;

        Real64 x1 = t.line_ori.p1.x;
        Real64 y1 = t.line_ori.p1.y;
        bool is_rev = x0 > x1;

        std::string const msg = fmt::format("From ({}, {}) to ({}, {})", t.line_ori.p0.x, t.line_ori.p0.y, t.line_ori.p1.x, t.line_ori.p1.y);

        bool visible = false;
        CLIPLINE(x0, x1, y0, y1, maxX, minX, maxY, minY, visible);
        if (t.visible) {
            EXPECT_TRUE(visible) << msg;
            EXPECT_DOUBLE_EQ(t.line_new.p0.x, x0) << msg;
            EXPECT_DOUBLE_EQ(t.line_new.p0.y, y0) << msg;
            EXPECT_DOUBLE_EQ(t.line_new.p1.x, x1) << msg;
            EXPECT_DOUBLE_EQ(t.line_new.p1.y, y1) << msg;
        } else {
            EXPECT_FALSE(visible) << msg;
            if (is_rev) {
                std::swap(x0, x1);
                std::swap(y0, y1);
            }
            EXPECT_DOUBLE_EQ(t.line_ori.p0.x, x0) << msg;
            EXPECT_DOUBLE_EQ(t.line_ori.p0.y, y0) << msg;
            EXPECT_DOUBLE_EQ(t.line_ori.p1.x, x1) << msg;
            EXPECT_DOUBLE_EQ(t.line_ori.p1.y, y1) << msg;
        }
    };

    constexpr std::array<TestCase, 72> test_cases{{
        // From 0 to 3
        TestCase{Line{Point{below_x, below_y}, Point{below_x, center_y}}, false},
        // From 0 to 6
        TestCase{Line{Point{below_x, below_y}, Point{below_x, greater_y}}, false},
        // From 0 to 1
        TestCase{Line{Point{below_x, below_y}, Point{center_x, below_y}}, false},
        // From 0 to 4
        TestCase{Line{Point{below_x, below_y}, Point{center_x, center_y}}, true, Line{Point{2.8571428571428568, minY}, Point{center_x, center_y}}},
        // From 0 to 7
        TestCase{Line{Point{below_x, below_y}, Point{center_x, greater_y}}, true, Line{Point{minX, 4.2}, Point{3.125, maxY}}},
        // From 0 to 2
        TestCase{Line{Point{below_x, below_y}, Point{greater_x, below_y}}, false},
        // From 0 to 5
        TestCase{
            Line{Point{below_x, below_y}, Point{greater_x, center_y}}, true, Line{Point{5.7142857142857135, minY}, Point{maxX, 3.8000000000000003}}},
        // From 0 to 8
        TestCase{Line{Point{below_x, below_y}, Point{greater_x, greater_y}}, true, Line{Point{2.5, minY}, Point{6.25, maxY}}},
        // From 3 to 0
        TestCase{Line{Point{below_x, center_y}, Point{below_x, below_y}}, false},
        // From 3 to 6
        TestCase{Line{Point{below_x, center_y}, Point{below_x, greater_y}}, false},
        // From 3 to 1
        TestCase{
            Line{Point{below_x, center_y}, Point{center_x, below_y}}, true, Line{Point{minX, 3.0999999999999996}, Point{2.142857142857143, minY}}},
        // From 3 to 4
        TestCase{Line{Point{below_x, center_y}, Point{center_x, center_y}}, true, Line{Point{minX, center_y}, Point{center_x, center_y}}},
        // From 3 to 7
        TestCase{Line{Point{below_x, center_y}, Point{center_x, greater_y}}, false},
        // From 3 to 2
        TestCase{Line{Point{below_x, center_y}, Point{greater_x, below_y}}, true, Line{Point{minX, 3.8}, Point{4.285714285714286, minY}}},
        // From 3 to 5
        TestCase{Line{Point{below_x, center_y}, Point{greater_x, center_y}}, true, Line{Point{minX, center_y}, Point{maxX, center_y}}},
        // From 3 to 8
        TestCase{Line{Point{below_x, center_y}, Point{greater_x, greater_y}}, true, Line{Point{minX, 5.4}, Point{3.333333333333333, maxY}}},
        // From 6 to 0
        TestCase{Line{Point{below_x, greater_y}, Point{below_x, below_y}}, false},
        // From 6 to 3
        TestCase{Line{Point{below_x, greater_y}, Point{below_x, center_y}}, false},
        // From 6 to 1
        TestCase{Line{Point{below_x, greater_y}, Point{center_x, below_y}}, true, Line{Point{minX, 5.8}, Point{3.75, minY}}},
        // From 6 to 4
        TestCase{Line{Point{below_x, greater_y}, Point{center_x, center_y}}, true, Line{Point{3.333333333333333, maxY}, Point{center_x, center_y}}},
        // From 6 to 7
        TestCase{Line{Point{below_x, greater_y}, Point{center_x, greater_y}}, false},
        // From 6 to 2
        TestCase{Line{Point{below_x, greater_y}, Point{greater_x, below_y}}, true, Line{Point{3.75, maxY}, Point{7.5, minY}}},
        // From 6 to 5
        TestCase{Line{Point{below_x, greater_y}, Point{greater_x, center_y}}, true, Line{Point{6.666666666666666, maxY}, Point{maxX, 5.4}}},
        // From 6 to 8
        TestCase{Line{Point{below_x, greater_y}, Point{greater_x, greater_y}}, false},
        // From 1 to 0
        TestCase{Line{Point{center_x, below_y}, Point{below_x, below_y}}, false},
        // From 1 to 3
        TestCase{Line{Point{center_x, below_y}, Point{below_x, center_y}}, true, Line{Point{2.1428571428571432, minY}, Point{minX, 3.1}}},
        // From 1 to 6
        TestCase{Line{Point{center_x, below_y}, Point{below_x, greater_y}}, true, Line{Point{3.75, minY}, Point{minX, 5.8}}},
        // From 1 to 4
        TestCase{Line{Point{center_x, below_y}, Point{center_x, center_y}}, true, Line{Point{center_x, minY}, Point{center_x, center_y}}},
        // From 1 to 7
        TestCase{Line{Point{center_x, below_y}, Point{center_x, greater_y}}, true, Line{Point{center_x, minY}, Point{center_x, maxY}}},
        // From 1 to 2
        TestCase{Line{Point{center_x, below_y}, Point{greater_x, below_y}}, false},
        // From 1 to 5
        TestCase{Line{Point{center_x, below_y}, Point{greater_x, center_y}}, true, Line{Point{7.857142857142857, minY}, Point{maxX, 3.1}}},
        // From 1 to 8
        TestCase{Line{Point{center_x, below_y}, Point{greater_x, greater_y}}, true, Line{Point{6.25, minY}, Point{maxX, 5.8}}},
        // From 4 to 0
        TestCase{Line{Point{center_x, center_y}, Point{below_x, below_y}}, true, Line{Point{center_x, center_y}, Point{2.857142857142857, minY}}},
        // From 4 to 3
        TestCase{Line{Point{center_x, center_y}, Point{below_x, center_y}}, true, Line{Point{center_x, center_y}, Point{minX, center_y}}},
        // From 4 to 6
        TestCase{Line{Point{center_x, center_y}, Point{below_x, greater_y}}, true, Line{Point{center_x, center_y}, Point{3.3333333333333335, maxY}}},
        // From 4 to 1
        TestCase{Line{Point{center_x, center_y}, Point{center_x, below_y}}, true, Line{Point{center_x, center_y}, Point{center_x, minY}}},
        // From 4 to 7
        TestCase{Line{Point{center_x, center_y}, Point{center_x, greater_y}}, true, Line{Point{center_x, center_y}, Point{center_x, maxY}}},
        // From 4 to 2
        TestCase{Line{Point{center_x, center_y}, Point{greater_x, below_y}}, true, Line{Point{center_x, center_y}, Point{7.142857142857142, minY}}},
        // From 4 to 5
        TestCase{Line{Point{center_x, center_y}, Point{greater_x, center_y}}, true, Line{Point{center_x, center_y}, Point{maxX, center_y}}},
        // From 4 to 8
        TestCase{Line{Point{center_x, center_y}, Point{greater_x, greater_y}}, true, Line{Point{center_x, center_y}, Point{6.666666666666666, maxY}}},
        // From 7 to 0
        TestCase{Line{Point{center_x, greater_y}, Point{below_x, below_y}}, true, Line{Point{3.125, maxY}, Point{minX, 4.2}}},
        // From 7 to 3
        TestCase{Line{Point{center_x, greater_y}, Point{below_x, center_y}}, false},
        // From 7 to 6
        TestCase{Line{Point{center_x, greater_y}, Point{below_x, greater_y}}, false},
        // From 7 to 1
        TestCase{Line{Point{center_x, greater_y}, Point{center_x, below_y}}, true, Line{Point{center_x, maxY}, Point{center_x, minY}}},
        // From 7 to 4
        TestCase{Line{Point{center_x, greater_y}, Point{center_x, center_y}}, true, Line{Point{center_x, maxY}, Point{center_x, center_y}}},
        // From 7 to 2
        TestCase{Line{Point{center_x, greater_y}, Point{greater_x, below_y}}, true, Line{Point{6.875, maxY}, Point{maxX, 4.2}}},
        // From 7 to 5
        TestCase{Line{Point{center_x, greater_y}, Point{greater_x, center_y}}, false},
        // From 7 to 8
        TestCase{Line{Point{center_x, greater_y}, Point{greater_x, greater_y}}, false},
        // From 2 to 0
        TestCase{Line{Point{greater_x, below_y}, Point{below_x, below_y}}, false},
        // From 2 to 3
        TestCase{
            Line{Point{greater_x, below_y}, Point{below_x, center_y}}, true, Line{Point{4.2857142857142865, minY}, Point{minX, 3.8000000000000003}}},
        // From 2 to 6
        TestCase{Line{Point{greater_x, below_y}, Point{below_x, greater_y}}, true, Line{Point{7.5, minY}, Point{3.75, maxY}}},
        // From 2 to 1
        TestCase{Line{Point{greater_x, below_y}, Point{center_x, below_y}}, false},
        // From 2 to 4
        TestCase{Line{Point{greater_x, below_y}, Point{center_x, center_y}}, true, Line{Point{7.142857142857143, minY}, Point{center_x, center_y}}},
        // From 2 to 7
        TestCase{Line{Point{greater_x, below_y}, Point{center_x, greater_y}}, true, Line{Point{maxX, 4.2}, Point{6.875, maxY}}},
        // From 2 to 5
        TestCase{Line{Point{greater_x, below_y}, Point{greater_x, center_y}}, false},
        // From 2 to 8
        TestCase{Line{Point{greater_x, below_y}, Point{greater_x, greater_y}}, false},
        // From 5 to 0
        TestCase{Line{Point{greater_x, center_y}, Point{below_x, below_y}}, true, Line{Point{maxX, 3.8}, Point{5.714285714285714, minY}}},
        // From 5 to 3
        TestCase{Line{Point{greater_x, center_y}, Point{below_x, center_y}}, true, Line{Point{maxX, center_y}, Point{minX, center_y}}},
        // From 5 to 6
        TestCase{Line{Point{greater_x, center_y}, Point{below_x, greater_y}}, true, Line{Point{maxX, 5.4}, Point{6.666666666666667, maxY}}},
        // From 5 to 1
        TestCase{
            Line{Point{greater_x, center_y}, Point{center_x, below_y}}, true, Line{Point{maxX, 3.0999999999999996}, Point{7.857142857142858, minY}}},
        // From 5 to 4
        TestCase{Line{Point{greater_x, center_y}, Point{center_x, center_y}}, true, Line{Point{maxX, center_y}, Point{center_x, center_y}}},
        // From 5 to 7
        TestCase{Line{Point{greater_x, center_y}, Point{center_x, greater_y}}, false},
        // From 5 to 2
        TestCase{Line{Point{greater_x, center_y}, Point{greater_x, below_y}}, false},
        // From 5 to 8
        TestCase{Line{Point{greater_x, center_y}, Point{greater_x, greater_y}}, false},
        // From 8 to 0
        TestCase{Line{Point{greater_x, greater_y}, Point{below_x, below_y}}, true, Line{Point{6.25, maxY}, Point{2.5, minY}}},
        // From 8 to 3
        TestCase{Line{Point{greater_x, greater_y}, Point{below_x, center_y}}, true, Line{Point{3.333333333333334, maxY}, Point{minX, 5.4}}},
        // From 8 to 6
        TestCase{Line{Point{greater_x, greater_y}, Point{below_x, greater_y}}, false},
        // From 8 to 1
        TestCase{Line{Point{greater_x, greater_y}, Point{center_x, below_y}}, true, Line{Point{maxX, 5.8}, Point{6.25, minY}}},
        // From 8 to 4
        TestCase{Line{Point{greater_x, greater_y}, Point{center_x, center_y}}, true, Line{Point{6.666666666666667, maxY}, Point{center_x, center_y}}},
        // From 8 to 7
        TestCase{Line{Point{greater_x, greater_y}, Point{center_x, greater_y}}, false},
        // From 8 to 2
        TestCase{Line{Point{greater_x, greater_y}, Point{greater_x, below_y}}, false},
        // From 8 to 5
        TestCase{Line{Point{greater_x, greater_y}, Point{greater_x, center_y}}, false},
    }};

    size_t i = 0;
    for (const auto &t : test_cases) {
        ++i;
        std::string const msg =
            fmt::format("test_case {}: From ({}, {}) to ({}, {})", i, t.line_ori.p0.x, t.line_ori.p0.y, t.line_ori.p1.x, t.line_ori.p1.y);
        SCOPED_TRACE(msg);
        testclipline(t);
    }

    constexpr std::array<TestCase, 24> boundary_lines{
        TestCase{Line{Point{minX, below_y}, Point{minX, center_y}}, true, Line{Point{minX, minY}, Point{minX, center_y}}},
        TestCase{Line{Point{minX, below_y}, Point{minX, greater_y}}, true, Line{Point{minX, minY}, Point{minX, maxY}}},
        TestCase{Line{Point{minX, center_y}, Point{minX, below_y}}, true, Line{Point{minX, center_y}, Point{minX, minY}}},
        TestCase{Line{Point{minX, center_y}, Point{minX, greater_y}}, true, Line{Point{minX, center_y}, Point{minX, maxY}}},
        TestCase{Line{Point{minX, greater_y}, Point{minX, below_y}}, true, Line{Point{minX, maxY}, Point{minX, minY}}},
        TestCase{Line{Point{minX, greater_y}, Point{minX, center_y}}, true, Line{Point{minX, maxY}, Point{minX, center_y}}},
        TestCase{Line{Point{maxX, below_y}, Point{maxX, center_y}}, true, Line{Point{maxX, minY}, Point{maxX, center_y}}},
        TestCase{Line{Point{maxX, below_y}, Point{maxX, greater_y}}, true, Line{Point{maxX, minY}, Point{maxX, maxY}}},
        TestCase{Line{Point{maxX, center_y}, Point{maxX, below_y}}, true, Line{Point{maxX, center_y}, Point{maxX, minY}}},
        TestCase{Line{Point{maxX, center_y}, Point{maxX, greater_y}}, true, Line{Point{maxX, center_y}, Point{maxX, maxY}}},
        TestCase{Line{Point{maxX, greater_y}, Point{maxX, below_y}}, true, Line{Point{maxX, maxY}, Point{maxX, minY}}},
        TestCase{Line{Point{maxX, greater_y}, Point{maxX, center_y}}, true, Line{Point{maxX, maxY}, Point{maxX, center_y}}},
        TestCase{Line{Point{below_x, minY}, Point{center_x, minY}}, true, Line{Point{minX, minY}, Point{center_x, minY}}},
        TestCase{Line{Point{below_x, minY}, Point{greater_x, minY}}, true, Line{Point{minX, minY}, Point{maxX, minY}}},
        TestCase{Line{Point{center_x, minY}, Point{below_x, minY}}, true, Line{Point{center_x, minY}, Point{minX, minY}}},
        TestCase{Line{Point{center_x, minY}, Point{greater_x, minY}}, true, Line{Point{center_x, minY}, Point{maxX, minY}}},
        TestCase{Line{Point{greater_x, minY}, Point{below_x, minY}}, true, Line{Point{maxX, minY}, Point{minX, minY}}},
        TestCase{Line{Point{greater_x, minY}, Point{center_x, minY}}, true, Line{Point{maxX, minY}, Point{center_x, minY}}},
        TestCase{Line{Point{below_x, maxY}, Point{center_x, maxY}}, true, Line{Point{minX, maxY}, Point{center_x, maxY}}},
        TestCase{Line{Point{below_x, maxY}, Point{greater_x, maxY}}, true, Line{Point{minX, maxY}, Point{maxX, maxY}}},
        TestCase{Line{Point{center_x, maxY}, Point{below_x, maxY}}, true, Line{Point{center_x, maxY}, Point{minX, maxY}}},
        TestCase{Line{Point{center_x, maxY}, Point{greater_x, maxY}}, true, Line{Point{center_x, maxY}, Point{maxX, maxY}}},
        TestCase{Line{Point{greater_x, maxY}, Point{below_x, maxY}}, true, Line{Point{maxX, maxY}, Point{minX, maxY}}},
        TestCase{Line{Point{greater_x, maxY}, Point{center_x, maxY}}, true, Line{Point{maxX, maxY}, Point{center_x, maxY}}},
    };
    i = 0;
    for (const auto &t : boundary_lines) {
        ++i;
        std::string const msg =
            fmt::format("Boundary Line {}: From ({}, {}) to ({}, {})", i, t.line_ori.p0.x, t.line_ori.p0.y, t.line_ori.p1.x, t.line_ori.p1.y);
        SCOPED_TRACE(msg);
        testclipline(t);
    }
}
