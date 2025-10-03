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

// EnergyPlus::HeatBalanceSurfaceManager Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/ConfiguredFunctions.hh>
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/ConvectionCoefficients.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataMoistureBalance.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/DaylightingDevices.hh>
#include <EnergyPlus/DaylightingManager.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HeatBalanceIntRadExchange.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/HeatBalanceSurfaceManager.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputReportTabular.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SolarShading.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/ThermalComfort.hh>
#include <EnergyPlus/WeatherManager.hh>
#include <EnergyPlus/WindowManager.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus::HeatBalanceSurfaceManager;

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_CalcOutsideSurfTemp)
{

    int SurfNum;      // Surface number DO loop counter
    int ZoneNum;      // Zone number the current surface is attached to
    int ConstrNum;    // Construction index for the current surface
    Real64 HMovInsul; // "Convection" coefficient of movable insulation
    Real64 TempExt;   // Exterior temperature boundary condition
    bool ErrorFlag;   // Movable insulation error flag

    SurfNum = 1;
    ZoneNum = 1;
    ConstrNum = 1;
    HMovInsul = 1.0;
    TempExt = 23.0;
    ErrorFlag = false;

    state->dataGlobal->TimeStepsInHour = 4;
    state->dataGlobal->TimeStepZoneSec = 900.0;

    state->dataConstruction->Construct.allocate(ConstrNum);
    state->dataConstruction->Construct(ConstrNum).Name = "TestConstruct";
    state->dataConstruction->Construct(ConstrNum).CTFCross[0] = 0.0;
    state->dataConstruction->Construct(ConstrNum).CTFOutside[0] = 1.0;
    state->dataConstruction->Construct(ConstrNum).SourceSinkPresent = true;
    auto *p = new Material::MaterialBase;
    state->dataMaterial->materials.push_back(p);
    state->dataMaterial->materials(1)->Name = "TestMaterial";

    state->dataSurface->TotSurfaces = SurfNum;
    state->dataGlobal->NumOfZones = ZoneNum;

    state->dataSurface->Surface.allocate(SurfNum);
    state->dataSurface->SurfaceWindow.allocate(SurfNum);
    state->dataHeatBal->Zone.allocate(ZoneNum);

    state->dataSurface->Surface(SurfNum).Class = DataSurfaces::SurfaceClass::Wall;
    state->dataSurface->Surface(SurfNum).Area = 10.0;
    Window::initWindowModel(*state);
    SurfaceGeometry::AllocateSurfaceWindows(*state, SurfNum);
    SolarShading::AllocateModuleArrays(*state);
    AllocateSurfaceHeatBalArrays(*state);
    SurfaceGeometry::AllocateSurfaceArrays(*state);

    state->dataHeatBalSurf->SurfHConvExt(SurfNum) = 1.0;
    state->dataHeatBalSurf->SurfHAirExt(SurfNum) = 1.0;
    state->dataHeatBalSurf->SurfHSkyExt(SurfNum) = 1.0;
    state->dataHeatBalSurf->SurfHGrdExt(SurfNum) = 1.0;

    state->dataHeatBalSurf->SurfCTFConstOutPart(SurfNum) = 1.0;
    state->dataHeatBalSurf->SurfOpaqQRadSWOutAbs(SurfNum) = 1.0;
    state->dataHeatBalSurf->SurfTempIn(SurfNum) = 1.0;
    state->dataHeatBalSurf->SurfQRadSWOutMvIns(SurfNum) = 1.0;
    state->dataHeatBalSurf->SurfQRadLWOutSrdSurfs(SurfNum) = 1.0;
    state->dataHeatBalSurf->SurfQAdditionalHeatSourceOutside(SurfNum) = 0.0;
    state->dataSurface->extMovInsuls(SurfNum).matNum = 1;
    state->dataSurface->Surface(SurfNum).SurfHasSurroundingSurfProperty = false;

    state->dataSurface->SurfOutDryBulbTemp = 0;
    state->dataEnvrn->SkyTemp = 23.0;
    state->dataEnvrn->OutDryBulbTemp = 23.0;

    state->dataGlobal->HourOfDay = 1;
    state->dataGlobal->TimeStep = 1;

    state->dataHeatBal->space.allocate(1);
    state->dataHeatBal->Zone(ZoneNum).spaceIndexes.emplace_back(ZoneNum);
    state->dataHeatBal->space(ZoneNum).HTSurfaceFirst = 1;
    state->dataHeatBal->space(ZoneNum).HTSurfaceLast = 1;
    state->dataHeatBal->space(ZoneNum).OpaqOrIntMassSurfaceFirst = 1;
    state->dataHeatBal->space(ZoneNum).OpaqOrIntMassSurfaceLast = 1;
    state->dataHeatBal->space(ZoneNum).OpaqOrWinSurfaceFirst = 1;
    state->dataHeatBal->space(ZoneNum).OpaqOrWinSurfaceLast = 1;

    CalcOutsideSurfTemp(*state, SurfNum, ZoneNum, ConstrNum, HMovInsul, TempExt, ErrorFlag);

    state->dataHeatBalSurf->SurfTempOut(SurfNum) = state->dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum);

    ReportSurfaceHeatBalance(*state);

    std::string const error_string = delimited_string({
        "   ** Severe  ** Exterior movable insulation is not valid with embedded sources/sinks",
        "   **   ~~~   ** Construction TestConstruct contains an internal source or sink but also uses",
        "   **   ~~~   ** exterior movable insulation TestMaterial for a surface with that construction.",
        "   **   ~~~   ** This is not currently allowed because the heat balance equations do not currently accommodate this combination.",
    });

    EXPECT_TRUE(ErrorFlag);
    EXPECT_TRUE(compare_err_stream(error_string, true));
    EXPECT_EQ(10.0 * 1.0 * (state->dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum) - state->dataSurface->SurfOutDryBulbTemp(SurfNum)),
              state->dataHeatBalSurf->SurfQAirExtReport(SurfNum));
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_TestSurfTempCalcHeatBalanceInsideSurf)
{

    Real64 surfTemp;
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->SurfLowTempErrCount.allocate(1);
    state->dataSurface->SurfHighTempErrCount.allocate(1);
    DataHeatBalance::ZoneData testZone;
    int cntWarmupSurfTemp = 0;
    state->dataSurface->Surface(1).Name = "TestSurface";
    testZone.Name = "TestZone";
    testZone.InternalHeatGains = 2.5;
    testZone.NominalInfilVent = 0.5;
    testZone.NominalMixing = 0.7;

    // no error
    surfTemp = 26;
    state->dataGlobal->WarmupFlag = true;
    state->dataSurface->SurfLowTempErrCount(1) = 0;
    state->dataSurface->SurfHighTempErrCount(1) = 0;
    testZone.TempOutOfBoundsReported = true;
    testZone.FloorArea = 1000;
    testZone.IsControlled = true;
    TestSurfTempCalcHeatBalanceInsideSurf(*state, surfTemp, 1, testZone, cntWarmupSurfTemp);
    EXPECT_TRUE(compare_err_stream("", true));

    // to hot - first time
    surfTemp = 201;
    state->dataGlobal->WarmupFlag = false;
    state->dataSurface->SurfLowTempErrCount(1) = 0;
    state->dataSurface->SurfHighTempErrCount(1) = 0;
    testZone.TempOutOfBoundsReported = false;
    testZone.FloorArea = 1000;
    testZone.IsControlled = true;
    TestSurfTempCalcHeatBalanceInsideSurf(*state, surfTemp, 1, testZone, cntWarmupSurfTemp);
    std::string const error_string01 =
        delimited_string({"   ** Severe  ** Temperature (high) out of bounds (201.00] for zone=\"TestZone\", for surface=\"TestSurface\"",
                          "   **   ~~~   **  Environment=, at Simulation time= 00:00 - 00:00",
                          "   **   ~~~   ** Zone=\"TestZone\", Diagnostic Details:",
                          "   **   ~~~   ** ...Internal Heat Gain [2.500E-003] W/m2",
                          "   **   ~~~   ** ...Infiltration/Ventilation [0.500] m3/s",
                          "   **   ~~~   ** ...Mixing/Cross Mixing [0.700] m3/s",
                          "   **   ~~~   ** ...Zone is part of HVAC controlled system."});
    EXPECT_TRUE(compare_err_stream(error_string01, true));
    EXPECT_TRUE(testZone.TempOutOfBoundsReported);

    // to hot - subsequent times
    surfTemp = 201;
    state->dataGlobal->WarmupFlag = false;
    state->dataSurface->SurfLowTempErrCount(1) = 0;
    state->dataSurface->SurfHighTempErrCount(1) = 0;
    testZone.TempOutOfBoundsReported = true;
    testZone.FloorArea = 1000;
    testZone.IsControlled = true;
    TestSurfTempCalcHeatBalanceInsideSurf(*state, surfTemp, 1, testZone, cntWarmupSurfTemp);
    std::string const error_string02 = delimited_string({
        "   ** Severe  ** Temperature (high) out of bounds (201.00] for zone=\"TestZone\", for surface=\"TestSurface\"",
        "   **   ~~~   **  Environment=, at Simulation time= 00:00 - 00:00",
    });
    EXPECT_TRUE(compare_err_stream(error_string02, true));
    EXPECT_TRUE(testZone.TempOutOfBoundsReported);

    // to cold - first time
    surfTemp = -101;
    state->dataGlobal->WarmupFlag = false;
    state->dataSurface->SurfLowTempErrCount(1) = 0;
    state->dataSurface->SurfHighTempErrCount(1) = 0;
    testZone.TempOutOfBoundsReported = false;
    testZone.FloorArea = 1000;
    testZone.IsControlled = true;
    TestSurfTempCalcHeatBalanceInsideSurf(*state, surfTemp, 1, testZone, cntWarmupSurfTemp);
    std::string const error_string03 =
        delimited_string({"   ** Severe  ** Temperature (low) out of bounds [-101.00] for zone=\"TestZone\", for surface=\"TestSurface\"",
                          "   **   ~~~   **  Environment=, at Simulation time= 00:00 - 00:00",
                          "   **   ~~~   ** Zone=\"TestZone\", Diagnostic Details:",
                          "   **   ~~~   ** ...Internal Heat Gain [2.500E-003] W/m2",
                          "   **   ~~~   ** ...Infiltration/Ventilation [0.500] m3/s",
                          "   **   ~~~   ** ...Mixing/Cross Mixing [0.700] m3/s",
                          "   **   ~~~   ** ...Zone is part of HVAC controlled system."});
    EXPECT_TRUE(compare_err_stream(error_string03, true));
    EXPECT_TRUE(testZone.TempOutOfBoundsReported);

    // to cold - subsequent times
    surfTemp = -101;
    state->dataGlobal->WarmupFlag = false;
    state->dataSurface->SurfLowTempErrCount(1) = 0;
    state->dataSurface->SurfHighTempErrCount(1) = 0;
    testZone.TempOutOfBoundsReported = true;
    testZone.FloorArea = 1000;
    testZone.IsControlled = true;
    TestSurfTempCalcHeatBalanceInsideSurf(*state, surfTemp, 1, testZone, cntWarmupSurfTemp);
    std::string const error_string04 =
        delimited_string({"   ** Severe  ** Temperature (low) out of bounds [-101.00] for zone=\"TestZone\", for surface=\"TestSurface\"",
                          "   **   ~~~   **  Environment=, at Simulation time= 00:00 - 00:00"});
    EXPECT_TRUE(compare_err_stream(error_string04, true));
    EXPECT_TRUE(testZone.TempOutOfBoundsReported);
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_ComputeIntThermalAbsorpFactors)
{

    state->dataSurface->TotSurfaces = 1;
    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->TotConstructs = 1;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->space.allocate(1);
    state->dataHeatBal->Zone(1).spaceIndexes.emplace_back(1);
    state->dataHeatBal->space(1).WindowSurfaceFirst = 1;
    state->dataHeatBal->space(1).WindowSurfaceLast = 1;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);
    state->dataSurface->SurfaceWindow.allocate(state->dataSurface->TotSurfaces);
    state->dataSurface->surfShades.allocate(state->dataSurface->TotSurfaces);
    SurfaceGeometry::AllocateSurfaceWindows(*state, state->dataSurface->TotSurfaces);
    state->dataConstruction->Construct.allocate(state->dataHeatBal->TotConstructs);

    state->dataSurface->surfShades(1).effShadeEmi = 0.1;
    state->dataSurface->surfShades(1).effGlassEmi = 0.1;

    state->dataSurface->Surface(1).HeatTransSurf = true;
    state->dataSurface->Surface(1).Construction = 1;
    state->dataSurface->Surface(1).Class = DataSurfaces::SurfaceClass::Window;
    state->dataSurface->Surface(1).Area = 1;
    state->dataSurface->SurfWinShadingFlag(1) = DataSurfaces::WinShadingType::IntBlind;
    state->dataConstruction->Construct(1).InsideAbsorpThermal = 0.9;
    state->dataHeatBalSurf->SurfAbsThermalInt.allocate(1);

    state->dataViewFactor->NumOfRadiantEnclosures = 1;
    state->dataViewFactor->EnclRadInfo.allocate(1);
    state->dataViewFactor->EnclRadInfo(1).radReCalc = true;
    state->dataViewFactor->EnclRadInfo(1).spaceNums.emplace_back(1);
    state->dataViewFactor->EnclRadInfo(1).SurfacePtr.allocate(1);
    state->dataViewFactor->EnclRadInfo(1).SurfacePtr(1) = 1;

    ComputeIntThermalAbsorpFactors(*state);

    EXPECT_EQ(0.2, state->dataHeatBalSurf->SurfAbsThermalInt(1));
    EXPECT_EQ(5, state->dataViewFactor->EnclRadInfo(1).radThermAbsMult);
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_UpdateFinalThermalHistories)
{
    state->dataSurface->TotSurfaces = 1;
    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->TotConstructs = 1;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);
    state->dataSurface->SurfaceWindow.allocate(state->dataSurface->TotSurfaces);
    state->dataConstruction->Construct.allocate(state->dataHeatBal->TotConstructs);
    state->dataHeatBal->AnyInternalHeatSourceInInput = true;
    state->dataHeatBal->SimpleCTFOnly = false;

    AllocateSurfaceHeatBalArrays(*state); // allocates a host of variables related to CTF calculations

    state->dataSurface->Surface(1).Class = DataSurfaces::SurfaceClass::Wall;
    state->dataSurface->Surface(1).HeatTransSurf = true;
    state->dataSurface->Surface(1).HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::CTF;
    state->dataSurface->Surface(1).ExtBoundCond = 1;
    state->dataSurface->Surface(1).Construction = 1;
    state->dataHeatBal->space.allocate(1);
    state->dataHeatBal->Zone(1).spaceIndexes.emplace_back(1);
    state->dataHeatBal->space(1).OpaqOrIntMassSurfaceFirst = 1;
    state->dataHeatBal->space(1).OpaqOrIntMassSurfaceLast = 1;
    state->dataHeatBal->space(1).HTSurfaceFirst = 1;
    state->dataHeatBal->space(1).HTSurfaceLast = 1;

    state->dataConstruction->Construct(1).NumCTFTerms = 2;
    state->dataConstruction->Construct(1).SourceSinkPresent = true;
    state->dataConstruction->Construct(1).NumHistories = 1;
    state->dataConstruction->Construct(1).CTFTUserOut[0] = 0.5;
    state->dataConstruction->Construct(1).CTFTUserIn[0] = 0.25;
    state->dataConstruction->Construct(1).CTFTUserSource[0] = 0.25;

    state->dataHeatBalSurf->SurfCurrNumHist(1) = 0;
    state->dataHeatBalSurf->SurfOutsideTempHist(1)(1) = 20.0;
    state->dataHeatBalSurf->SurfTempIn(1) = 10.0;

    state->dataHeatBalFanSys->CTFTuserConstPart(1) = 0.0;

    UpdateThermalHistories(*state); // First check to see if it is calculating the user location temperature properly

    EXPECT_EQ(12.5, state->dataHeatBalSurf->SurfTempUserLoc(1));
    EXPECT_EQ(0.0, state->dataHeatBalSurf->SurfTuserHist(1, 3));

    UpdateThermalHistories(*state);

    EXPECT_EQ(12.5, state->dataHeatBalSurf->SurfTuserHist(1, 3)); // Now check to see that it is shifting the temperature history properly
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_TestSurfTempCalcHeatBalanceInsideSurfAirRefT)
{

    std::string const idf_objects = delimited_string({
        "  Building,",
        "    House with AirflowNetwork simulation,  !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    0.001,                   !- Loads Convergence Tolerance Value",
        "    0.0050000,               !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullInteriorAndExterior, !- Solar Distribution",
        "    25,                      !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",

        "  SimulationControl,",
        "    No,                      !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    Yes,                     !- Run Simulation for Sizing Periods",
        "    No;                      !- Run Simulation for Weather File Run Periods",

        "  RunPeriod,",
        "    WinterDay,               !- Name",
        "    1,                       !- Begin Month",
        "    14,                      !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    1,                       !- End Month",
        "    14,                      !- End Day of Month",
        "    ,                        !- End Year",
        "    Tuesday,                 !- Day of Week for Start Day",
        "    Yes,                     !- Use Weather File Holidays and Special Days",
        "    Yes,                     !- Use Weather File Daylight Saving Period",
        "    No,                      !- Apply Weekend Holiday Rule",
        "    Yes,                     !- Use Weather File Rain Indicators",
        "    Yes;                     !- Use Weather File Snow Indicators",

        "  RunPeriod,",
        "    SummerDay,               !- Name",
        "    7,                       !- Begin Month",
        "    7,                       !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    7,                       !- End Month",
        "    7,                       !- End Day of Month",
        "    ,                        !- End Year",
        "    Tuesday,                 !- Day of Week for Start Day",
        "    Yes,                     !- Use Weather File Holidays and Special Days",
        "    Yes,                     !- Use Weather File Daylight Saving Period",
        "    No,                      !- Apply Weekend Holiday Rule",
        "    Yes,                     !- Use Weather File Rain Indicators",
        "    No;                      !- Use Weather File Snow Indicators",

        "  Site:Location,",
        "    CHICAGO_IL_USA TMY2-94846,  !- Name",
        "    41.78,                   !- Latitude {deg}",
        "    -87.75,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190.00;                  !- Elevation {m}",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Heating 99% Design Conditions DB,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    WinterDesignDay,         !- Day Type",
        "    -17.3,                   !- Maximum Dry-Bulb Temperature {C}",
        "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    -17.3,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    4.9,                     !- Wind Speed {m/s}",
        "    270,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    0.0;                     !- Sky Clearness",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Cooling 1% Design Conditions DB/MCWB,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    31.5,                    !- Maximum Dry-Bulb Temperature {C}",
        "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    23.0,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    5.3,                     !- Wind Speed {m/s}",
        "    230,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    1.0;                     !- Sky Clearness",

        "  Site:GroundTemperature:BuildingSurface,20.03,20.03,20.13,20.30,20.43,20.52,20.62,20.77,20.78,20.55,20.44,20.20;",

        "  Material,",
        "    A1 - 1 IN STUCCO,        !- Name",
        "    Smooth,                  !- Roughness",
        "    2.5389841E-02,           !- Thickness {m}",
        "    0.6918309,               !- Conductivity {W/m-K}",
        "    1858.142,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Material,",
        "    CB11,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    0.2032000,               !- Thickness {m}",
        "    1.048000,                !- Conductivity {W/m-K}",
        "    1105.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.2000000,               !- Solar Absorptance",
        "    0.2000000;               !- Visible Absorptance",

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
        "    IN05,                    !- Name",
        "    Rough,                   !- Roughness",
        "    0.2458000,               !- Thickness {m}",
        "    4.3000001E-02,           !- Conductivity {W/m-K}",
        "    10.00000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    PW03,                    !- Name",
        "    MediumSmooth,            !- Roughness",
        "    1.2700000E-02,           !- Thickness {m}",
        "    0.1150000,               !- Conductivity {W/m-K}",
        "    545.0000,                !- Density {kg/m3}",
        "    1213.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7800000,               !- Solar Absorptance",
        "    0.7800000;               !- Visible Absorptance",

        "  Material,",
        "    CC03,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1016000,               !- Thickness {m}",
        "    1.310000,                !- Conductivity {W/m-K}",
        "    2243.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

        "  Material,",
        "    HF-A3,                   !- Name",
        "    Smooth,                  !- Roughness",
        "    1.5000000E-03,           !- Thickness {m}",
        "    44.96960,                !- Conductivity {W/m-K}",
        "    7689.000,                !- Density {kg/m3}",
        "    418.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.2000000,               !- Solar Absorptance",
        "    0.2000000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    AR02,                    !- Name",
        "    VeryRough,               !- Roughness",
        "    7.8000002E-02,           !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7000000,               !- Solar Absorptance",
        "    0.7000000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    CP02,                    !- Name",
        "    Rough,                   !- Roughness",
        "    0.2170000,               !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Construction,",
        "    EXTWALL:LIVING,          !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    GP01;                    !- Layer 3",

        "  Construction,",
        "    FLOOR:LIVING,            !- Name",
        "    CC03,                    !- Outside Layer",
        "    CP02;                    !- Layer 2",

        "  Construction,",
        "    ROOF,                    !- Name",
        "    AR02,                    !- Outside Layer",
        "    PW03;                    !- Layer 2",

        "  Zone,",
        "    LIVING ZONE,             !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    World;                   !- Coordinate System",

        "  BuildingSurface:Detailed,",
        "    Living:North,            !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    1,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    1,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:East,             !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    1,0,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    1,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:South,            !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,0,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:West,             !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,0,1;  !- X,Y,Z ==> Vertex 4 {m}",
        "  BuildingSurface:Detailed,",
        "    Living:Floor,            !- Name",
        "    FLOOR,                   !- Surface Type",
        "    FLOOR:LIVING,            !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Living:Floor,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,0,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Ceiling,          !- Name",
        "    ROOF,                 !- Surface Type",
        "    ROOF,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,1,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,0,1,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,1,1;  !- X,Y,Z ==> Vertex 4 {m}",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    bool ErrorsFound = false;

    HeatBalanceManager::GetProjectControlData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    Material::GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    SurfaceGeometry::GetGeometryParameters(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    state->dataSurfaceGeometry->CosBldgRotAppGonly = 1.0;
    state->dataSurfaceGeometry->SinBldgRotAppGonly = 0.0;
    SurfaceGeometry::SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    state->dataZoneEquip->ZoneEquipConfig.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneName = "LIVING ZONE";
    state->dataHeatBal->Zone(1).IsControlled = true;
    state->dataZoneEquip->ZoneEquipConfig(1).NumInletNodes = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode.allocate(2);
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(1) = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(2) = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1) = 3;
    state->dataZoneEquip->ZoneEquipConfig(1).NumReturnNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode(1) = 4;
    state->dataZoneEquip->ZoneEquipConfig(1).FixedReturnFlow.allocate(1);

    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataHeatBal->Zone(1).SystemZoneNodeNumber = 5;
    state->dataEnvrn->OutBaroPress = 101325.0;

    state->dataLoopNodes->Node.allocate(4);

    state->dataHeatBalSurf->SurfTempInTmp.allocate(6);
    state->dataHeatBalSurf->SurfTempInTmp(1) = 15.0;
    state->dataHeatBalSurf->SurfTempInTmp(2) = 20.0;
    state->dataHeatBalSurf->SurfTempInTmp(3) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(4) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(5) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(6) = 25.0;

    // allocate surface level adj ratio data member
    state->dataHeatBalSurf->SurfWinCoeffAdjRatio.dimension(6, 1.0);

    state->dataLoopNodes->Node(1).Temp = 20.0;
    state->dataLoopNodes->Node(2).Temp = 20.0;
    state->dataLoopNodes->Node(3).Temp = 20.0;
    state->dataLoopNodes->Node(4).Temp = 20.0;
    state->dataLoopNodes->Node(1).MassFlowRate = 0.1;
    state->dataLoopNodes->Node(2).MassFlowRate = 0.1;
    state->dataLoopNodes->Node(3).MassFlowRate = 0.1;
    state->dataLoopNodes->Node(4).MassFlowRate = 0.1;

    state->dataHeatBalSurf->SurfHConvInt.allocate(6);
    state->dataHeatBalSurf->SurfHConvInt(1) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(2) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(3) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(4) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(5) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(6) = 0.5;
    state->dataMstBal->HConvInFD.allocate(6);
    state->dataMstBal->RhoVaporAirIn.allocate(6);
    state->dataMstBal->HMassConvInFD.allocate(6);
    state->dataGlobal->BeginSimFlag = true;
    state->dataGlobal->KickOffSimulation = true;
    state->dataGlobal->TimeStepZoneSec = 900;
    SolarShading::AllocateModuleArrays(*state);
    HeatBalanceManager::AllocateZoneHeatBalArrays(*state);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.001;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 24.0;
    state->dataZoneTempPredictorCorrector->spaceHeatBalance(1).airHumRat = 0.001;
    state->dataZoneTempPredictorCorrector->spaceHeatBalance(1).MAT = 24.0;
    AllocateSurfaceHeatBalArrays(*state);
    createFacilityElectricPowerServiceObject(*state);

    for (int loop = 1; loop <= state->dataSurface->TotSurfaces; ++loop) {
        state->dataHeatBalSurf->SurfOutsideTempHist(1)(loop) = 20.0;
    }
    state->dataSurface->SurfTAirRef(1) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    state->dataSurface->SurfTAirRef(2) = DataSurfaces::RefAirTemp::AdjacentAirTemp;
    state->dataSurface->SurfTAirRef(3) = DataSurfaces::RefAirTemp::ZoneSupplyAirTemp;

    // with supply air
    HeatBalanceIntRadExchange::InitSolarViewFactors(*state);
    CalcHeatBalanceInsideSurf(*state);
    EXPECT_EQ(24.0, state->dataHeatBal->SurfTempEffBulkAir(1));
    EXPECT_EQ(23.0, state->dataHeatBal->SurfTempEffBulkAir(2));
    EXPECT_EQ(20.0, state->dataHeatBal->SurfTempEffBulkAir(3));

    // Supply air flow rate = 0
    state->dataLoopNodes->Node(1).MassFlowRate = 0.0;
    state->dataLoopNodes->Node(2).MassFlowRate = 0.0;
    state->dataLoopNodes->Node(3).MassFlowRate = 0.0;
    state->dataLoopNodes->Node(4).MassFlowRate = 0.0;
    CalcHeatBalanceInsideSurf(*state);
    EXPECT_EQ(24.0, state->dataHeatBal->SurfTempEffBulkAir(1));
    EXPECT_EQ(23.0, state->dataHeatBal->SurfTempEffBulkAir(2));
    EXPECT_EQ(24.0, state->dataHeatBal->SurfTempEffBulkAir(3));
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_TestSurfTempCalcHeatBalanceInsideSurfKiva)
{

    // Create Kiva foundation and set parameters
    Kiva::Foundation fnd;

    fnd.reductionStrategy = Kiva::Foundation::RS_AP;

    Kiva::Material concrete(1.95, 2400.0, 900.0);

    Kiva::Layer tempLayer;
    tempLayer.thickness = 0.10;
    tempLayer.material = concrete;

    fnd.slab.interior.emissivity = 0.8;
    fnd.slab.layers.push_back(tempLayer);

    tempLayer.thickness = 0.2;
    tempLayer.material = concrete;

    fnd.wall.layers.push_back(tempLayer);

    fnd.wall.heightAboveGrade = 0.1;
    fnd.wall.depthBelowSlab = 0.2;
    fnd.wall.interior.emissivity = 0.8;
    fnd.wall.exterior.emissivity = 0.8;
    fnd.wall.interior.absorptivity = 0.8;
    fnd.wall.exterior.absorptivity = 0.8;

    fnd.foundationDepth = 0.0;
    fnd.numericalScheme = Kiva::Foundation::NS_ADI;

    fnd.polygon.outer().push_back(Kiva::Point(-6.0, -6.0));
    fnd.polygon.outer().push_back(Kiva::Point(-6.0, 6.0));
    fnd.polygon.outer().push_back(Kiva::Point(6.0, 6.0));
    fnd.polygon.outer().push_back(Kiva::Point(6.0, -6.0));

    // Create Kiva weather data
    HeatBalanceKivaManager::KivaWeatherData kivaweather;
    kivaweather.annualAverageDrybulbTemp = 10.0;
    kivaweather.intervalsPerHour = 1;
    kivaweather.dryBulb = {10.0};
    kivaweather.windSpeed = {0.0};
    kivaweather.skyEmissivity = {0.0};

    HeatBalanceKivaManager::KivaManager km;

    std::string const idf_objects = delimited_string({
        "Material,",
        "  1/2IN Gypsum,            !- Name",
        "  Smooth,                  !- Roughness",
        "  0.0127,                  !- Thickness {m}",
        "  0.1600,                  !- Conductivity {W/m-K}",
        "  784.9000,                !- Density {kg/m3}",
        "  830.0000,                !- Specific Heat {J/kg-K}",
        "  0.9000,                  !- Thermal Absorptance",
        "  0.9200,                  !- Solar Absorptance",
        "  0.9200;                  !- Visible Absorptance",
        " ",
        "Material,",
        "  MAT-CC05 4 HW CONCRETE,  !- Name",
        "  Rough,                   !- Roughness",
        "  0.1016,                  !- Thickness {m}",
        "  1.3110,                  !- Conductivity {W/m-K}",
        "  2240.0000,               !- Density {kg/m3}",
        "  836.8000,                !- Specific Heat {J/kg-K}",
        "  0.9000,                  !- Thermal Absorptance",
        "  0.7000,                  !- Solar Absorptance",
        "  0.7000;                  !- Visible Absorptance",
        " ",
        "Material:NoMass,",
        "  CP02 CARPET PAD,         !- Name",
        "  VeryRough,               !- Roughness",
        "  0.2165,                  !- Thermal Resistance {m2-K/W}",
        "  0.9000,                  !- Thermal Absorptance",
        "  0.7000,                  !- Solar Absorptance",
        "  0.8000;                  !- Visible Absorptance",
        " ",
        "Material,",
        "  Std AC02,                !- Name",
        "  MediumSmooth,            !- Roughness",
        "  1.2700000E-02,           !- Thickness {m}",
        "  5.7000000E-02,           !- Conductivity {W/m-K}",
        "  288.0000,                !- Density {kg/m3}",
        "  1339.000,                !- Specific Heat {J/kg-K}",
        "  0.9000000,               !- Thermal Absorptance",
        "  0.7000000,               !- Solar Absorptance",
        "  0.2000000;               !- Visible Absorptance",
        " ",
        "Construction,",
        "  int-walls,               !- Name",
        "  1/2IN Gypsum,            !- Outside Layer",
        "  1/2IN Gypsum;            !- Layer 2",
        " ",
        "Construction,",
        "  INT-FLOOR-TOPSIDE,       !- Name",
        "  MAT-CC05 4 HW CONCRETE,  !- Outside Layer",
        "  CP02 CARPET PAD;         !- Layer 2",
        " ",
        "Construction,",
        "  DropCeiling,             !- Name",
        "  Std AC02;                !- Outside Layer",
        " ",
        "Zone,",
        "  Core_bottom,             !- Name",
        "  0.0000,                  !- Direction of Relative North {deg}",
        "  0.0000,                  !- X Origin {m}",
        "  0.0000,                  !- Y Origin {m}",
        "  0.0000,                  !- Z Origin {m}",
        "  1,                       !- Type",
        "  1,                       !- Multiplier",
        "  ,                        !- Ceiling Height {m}",
        "  ,                        !- Volume {m3}",
        "  autocalculate,           !- Floor Area {m2}",
        "  ,                        !- Zone Inside Convection Algorithm",
        "  ,                        !- Zone Outside Convection Algorithm",
        "  Yes;                     !- Part of Total Floor Area",
        " ",
        "BuildingSurface:Detailed,",
        "  Core_bot_ZN_5_Ceiling,   !- Name",
        "  Ceiling,                 !- Surface Type",
        "  DropCeiling,             !- Construction Name",
        "  Core_bottom,             !- Zone Name",
        "  ,                        !- Space Name",
        "  Adiabatic,               !- Outside Boundary Condition",
        "  ,                        !- Outside Boundary Condition Object",
        "  NoSun,                   !- Sun Exposure",
        "  NoWind,                  !- Wind Exposure",
        "  AutoCalculate,           !- View Factor to Ground",
        "  4,                       !- Number of Vertices",
        "  4.5732,44.1650,2.7440,  !- X,Y,Z ==> Vertex 1 {m}",
        "  4.5732,4.5732,2.7440,  !- X,Y,Z ==> Vertex 2 {m}",
        "  68.5340,4.5732,2.7440,  !- X,Y,Z ==> Vertex 3 {m}",
        "  68.5340,44.1650,2.7440;  !- X,Y,Z ==> Vertex 4 {m}",
        " ",
        "BuildingSurface:Detailed,",
        "  Core_bot_ZN_5_Floor,     !- Name",
        "  Floor,                   !- Surface Type",
        "  INT-FLOOR-TOPSIDE,       !- Construction Name",
        "  Core_bottom,             !- Zone Name",
        "  ,                        !- Space Name",
        "  Adiabatic,               !- Outside Boundary Condition",
        "  ,                        !- Outside Boundary Condition Object",
        "  NoSun,                   !- Sun Exposure",
        "  NoWind,                  !- Wind Exposure",
        "  AutoCalculate,           !- View Factor to Ground",
        "  4,                       !- Number of Vertices",
        "  68.5340,44.1650,0.0000,  !- X,Y,Z ==> Vertex 1 {m}",
        "  68.5340,4.5732,0.0000,  !- X,Y,Z ==> Vertex 2 {m}",
        "  4.5732,4.5732,0.0000,  !- X,Y,Z ==> Vertex 3 {m}",
        "  4.5732,44.1650,0.0000;  !- X,Y,Z ==> Vertex 4 {m}",
        " ",
        "BuildingSurface:Detailed,",
        "  Core_bot_ZN_5_Wall_East, !- Name",
        "  Wall,                    !- Surface Type",
        "  int-walls,               !- Construction Name",
        "  Core_bottom,             !- Zone Name",
        "  ,                        !- Space Name",
        "  Adiabatic,               !- Outside Boundary Condition",
        "  ,                        !- Outside Boundary Condition Object",
        "  NoSun,                   !- Sun Exposure",
        "  NoWind,                  !- Wind Exposure",
        "  AutoCalculate,           !- View Factor to Ground",
        "  4,                       !- Number of Vertices",
        "  68.5340,4.5732,2.7440,  !- X,Y,Z ==> Vertex 1 {m}",
        "  68.5340,4.5732,0.0000,  !- X,Y,Z ==> Vertex 2 {m}",
        "  68.5340,44.1650,0.0000,  !- X,Y,Z ==> Vertex 3 {m}",
        "  68.5340,44.1650,2.7440;  !- X,Y,Z ==> Vertex 4 {m}",
        " ",
        "BuildingSurface:Detailed,",
        "  Core_bot_ZN_5_Wall_North,!- Name",
        "  Wall,                    !- Surface Type",
        "  int-walls,               !- Construction Name",
        "  Core_bottom,             !- Zone Name",
        "  ,                        !- Space Name",
        "  Adiabatic,               !- Outside Boundary Condition",
        "  ,                        !- Outside Boundary Condition Object",
        "  NoSun,                   !- Sun Exposure",
        "  NoWind,                  !- Wind Exposure",
        "  AutoCalculate,           !- View Factor to Ground",
        "  4,                       !- Number of Vertices",
        "  68.5340,44.1650,2.7440,  !- X,Y,Z ==> Vertex 1 {m}",
        "  68.5340,44.1650,0.0000,  !- X,Y,Z ==> Vertex 2 {m}",
        "  4.5732,44.1650,0.0000,  !- X,Y,Z ==> Vertex 3 {m}",
        "  4.5732,44.1650,2.7440;  !- X,Y,Z ==> Vertex 4 {m}",
        " ",
        "BuildingSurface:Detailed,",
        "  Core_bot_ZN_5_Wall_South,!- Name",
        "  Wall,                    !- Surface Type",
        "  int-walls,               !- Construction Name",
        "  Core_bottom,             !- Zone Name",
        "  ,                        !- Space Name",
        "  Adiabatic,               !- Outside Boundary Condition",
        "  ,                        !- Outside Boundary Condition Object",
        "  NoSun,                   !- Sun Exposure",
        "  NoWind,                  !- Wind Exposure",
        "  AutoCalculate,           !- View Factor to Ground",
        "  4,                       !- Number of Vertices",
        "  4.5732,4.5732,2.7440,  !- X,Y,Z ==> Vertex 1 {m}",
        "  4.5732,4.5732,0.0000,  !- X,Y,Z ==> Vertex 2 {m}",
        "  68.5340,4.5732,0.0000,  !- X,Y,Z ==> Vertex 3 {m}",
        "  68.5340,4.5732,2.7440;  !- X,Y,Z ==> Vertex 4 {m}",
        " ",
        "BuildingSurface:Detailed,",
        "  Core_bot_ZN_5_Wall_West, !- Name",
        "  Wall,                    !- Surface Type",
        "  int-walls,               !- Construction Name",
        "  Core_bottom,             !- Zone Name",
        "  ,                        !- Space Name",
        "  Adiabatic,               !- Outside Boundary Condition",
        "  ,                        !- Outside Boundary Condition Object",
        "  NoSun,                   !- Sun Exposure",
        "  NoWind,                  !- Wind Exposure",
        "  AutoCalculate,           !- View Factor to Ground",
        "  4,                       !- Number of Vertices",
        "  4.5732,44.1650,2.7440,  !- X,Y,Z ==> Vertex 1 {m}",
        "  4.5732,44.1650,0.0000,  !- X,Y,Z ==> Vertex 2 {m}",
        "  4.5732,4.5732,0.0000,  !- X,Y,Z ==> Vertex 3 {m}",
        "  4.5732,4.5732,2.7440;  !- X,Y,Z ==> Vertex 4 {m}",
        " ",
        "People,",
        "  Core_bottom People,      !- Name",
        "  Core_bottom,             !- Zone or ZoneList Name",
        "  Core_bottom Occupancy,   !- Number of People Schedule Name",
        "  People,                  !- Number of People Calculation Method",
        "  4,                       !- Number of People",
        "  ,                        !- People per Zone Floor Area",
        "  ,                        !- Zone Floor Area per Person",
        "  0.9,                     !- Fraction Radiant",
        "  0.1,                     !- Sensible Heat Fraction",
        "  Core_bottom Activity,    !- Activity Level Schedule Name",
        "  3.82e-08,                !- Carbon Dioxide Generation Rate",

        "  Yes,                     !- Enable ASHRAE 55 Comfort Warnings",
        "  EnclosureAveraged,            !- Mean Radiant Temperature Calculation Type",
        "  ,                        !- Surface NameAngle Factor List Name",
        "  Work Eff Sched,          !- Work Efficiency Schedule Name",
        "  ClothingInsulationSchedule,  !- Clothing Insulation Calculation Method",
        "  ,                        !- Clothing Insulation Calculation Method Schedule Name",
        "  Clothing Schedule,       !- Clothing Insulation Schedule Name",
        "  Air Velocity Schedule,   !- Air Velocity Schedule Name",
        "  Fanger;                  !- Thermal Comfort Model 1 Type",
        " ",
        "Schedule:Compact,",
        "  Core_bottom Occupancy,   !- Name",
        "  Fraction,                !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 1",
        "  For: Alldays,            !- Field 2",
        "  Until: 07:00,            !- Field 3",
        "  0,                       !- Field 4",
        "  Until: 21:00,            !- Field 13",
        "  1,                       !- Field 14",
        "  Until: 24:00,            !- Field 15",
        "  0;                       !- Field 16",
        " ",
        "Schedule:Compact,",
        "  Core_bottom Activity,    !- Name",
        "  Activity Level,          !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 1",
        "  For: Alldays,            !- Field 2",
        "  Until: 24:00,            !- Field 3",
        "  166;                     !- Field 4",
        " ",
        "Schedule:Compact,",
        "  Work Eff Sched,          !- Name",
        "  Dimensionless,           !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 24:00,            !- Field 3",
        "  0;                       !- Field 4",
        " ",
        "Schedule:Compact,",
        "  Clothing Schedule,       !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 24:00,            !- Field 3",
        "  0.5;                     !- Field 4",
        " ",
        "Schedule:Compact,",
        "  Air Velocity Schedule,   !- Name",
        "  Velocity,                !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 24:00,            !- Field 3",
        "  0.129999995231628;       !- Field 4",
        " ",
        "ZoneControl:Thermostat,",
        "  Core_bottom Thermostat,  !- Name",
        "  Core_bottom,             !- Zone or ZoneList Name",
        "  Dual Zone Control Type Sched,  !- Control Type Schedule Name",
        "  ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        "  Core_bottom DualSPSched; !- Control 1 Name",
        " ",
        "ZoneControl:Thermostat:ThermalComfort,",
        "  Core_bottom Comfort,     !- Name",
        "  Core_bottom,             !- Zone or ZoneList Name",
        "  SpecificObject,          !- Averaging Method",
        "  Core_bottom People,      !- Specific People Name",
        "  0,                       !- Minimum DryBulb Temperature Setpoint",
        "  50,                      !- Maximum DryBulb Temperature Setpoint",
        "  Comfort Control,         !- Thermal Comfort Control Type Schedule Name",
        "  ThermostatSetpoint:ThermalComfort:Fanger:SingleHeating,    !- Thermal Comfort Control 1 Object Type",
        "  Single Htg PMV,          !- Thermal Comfort Control 1 Name",
        "  ThermostatSetpoint:ThermalComfort:Fanger:SingleCooling,    !- Thermal Comfort Control 2 Object Type",
        "  Single Cooling PMV;      !- Thermal Comfort Control 2 Name,",
        " ",
        "Schedule:Compact,",
        "  Comfort Control,          !- Name",
        "  Control Type,             !- Schedule Type Limits Name",
        "  Through: 5/31,            !- Field 1",
        "  For: AllDays,             !- Field 2",
        "  Until: 24:00,             !- Field 3",
        "  1,                        !- Field 4",
        "  Through: 8/31,            !- Field 5",
        "  For: AllDays,             !- Field 6",
        "  Until: 24:00,             !- Field 7",
        "  2,                        !- Field 8",
        "  Through: 12/31,           !- Field 9",
        "  For: AllDays,             !- Field 10",
        "  Until: 24:00,             !- Field 11",
        "  1;                        !- Field 12",
        " ",
        "THERMOSTATSETPOINT:THERMALCOMFORT:FANGER:SINGLEHEATING,",
        "  Single Htg PMV,          !- Name",
        "  Single Htg PMV;          !- Fanger Thermal Comfort Schedule Name",
        " ",
        "THERMOSTATSETPOINT:THERMALCOMFORT:FANGER:SINGLECOOLING,",
        "  Single Cooling PMV,      !- Name",
        "  Single Cooling PMV;      !- Fanger Thermal Comfort Schedule Name",
        " ",
        "Schedule:Constant,",
        "  Dual Zone Control Type Sched,  !- Name",
        "  Control Type,            !- Schedule Type Limits Name",
        "  4;                       !- Field 1",
        " ",
        "ThermostatSetpoint:DualSetpoint,",
        "  Core_bottom DualSPSched, !- Name",
        "  HTGSETP_SCH,             !- Heating Setpoint Temperature Schedule Name",
        "  CLGSETP_SCH;             !- Cooling Setpoint Temperature Schedule Name",
        " ",
        "Schedule:Constant,",
        "  CLGSETP_SCH,             !- Name",
        "  Temperature,             !- Schedule Type Limits Name",
        "  24.0;                    !- Field 1",
        " ",
        "Schedule:Constant,",
        "  HTGSETP_SCH,             !- Name",
        "  Temperature,             !- Schedule Type Limits Name",
        "  20.0;                    !- Field 1",
        " ",
        "Schedule:Compact,",
        "  Single Htg PMV,          !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 6:00,             !- Field 3",
        "  -0.5,                    !- Field 4",
        "  Until: 23:00,            !- Field 5",
        "  -0.2,                    !- Field 6",
        "  Until: 24:00,            !- Field 7",
        "  -0.5;                    !- Field 8",
        " ",
        "Schedule:Compact,",
        "  Single Cooling PMV,      !- Name",
        "  Any Number,              !- Schedule Type Limits Name",
        "  Through: 12/31,          !- Field 1",
        "  For: AllDays,            !- Field 2",
        "  Until: 6:00,             !- Field 3",
        "  0.5,                     !- Field 4",
        "  Until: 23:00,            !- Field 5",
        "  0.2,                     !- Field 6",
        "  Until: 24:00,            !- Field 7",
        "  0.5;                     !- Field 8",
        " ",
        "ScheduleTypeLimits,",
        "  Fraction,                 !- Name",
        "  0,                        !- Lower Limit Value",
        "  1,                        !- Upper Limit Value",
        "  Continuous,               !- Numeric Type",
        "  Dimensionless;            !- Unit Type",
        " ",
        "ScheduleTypeLimits,",
        "  Temperature,              !- Name",
        "  -60,                      !- Lower Limit Value",
        "  200,                      !- Upper Limit Value",
        "  Continuous,               !- Numeric Type",
        "  Temperature;              !- Unit Type",
        " ",
        "ScheduleTypeLimits,",
        "  Control Type,             !- Name",
        "  0,                        !- Lower Limit Value",
        "  4,                        !- Upper Limit Value",
        "  Discrete,                 !- Numeric Type",
        "  Dimensionless;            !- Unit Type",
        " ",
        "ScheduleTypeLimits,",
        "  On/Off,                   !- Name",
        "  0,                        !- Lower Limit Value",
        "  1,                        !- Upper Limit Value",
        "  Discrete,                 !- Numeric Type",
        "  Dimensionless;            !- Unit Type",
        " ",
        "ScheduleTypeLimits,",
        "  Any Number,               !- Name",
        "  ,                         !- Lower Limit Value",
        "  ,                         !- Upper Limit Value",
        "  Continuous;               !- Numeric Type",
        " ",
        "ScheduleTypeLimits,",
        "  Velocity,                 !- Name",
        "  ,                         !- Lower Limit Value",
        "  ,                         !- Upper Limit Value",
        "  Continuous,               !- Numeric Type",
        "  Velocity;                 !- Unit Type",
        " ",
        "ScheduleTypeLimits,",
        "  Activity Level,           !- Name",
        "  0,                        !- Lower Limit Value",
        "  ,                         !- Upper Limit Value",
        "  Continuous,               !- Numeric Type",
        "  ActivityLevel;            !- Unit Type",
        " ",
        "ScheduleTypeLimits,",
        "  Dimensionless,            !- Name",
        "  -1,                       !- Lower Limit Value",
        "  1,                        !- Upper Limit Value",
        "  Continuous;               !- Numeric Type",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    bool ErrorsFound(false); // If errors detected in input

    state->dataEnvrn->DayOfYear_Schedule = 1; // must initialize this to get schedules initialized
    state->dataEnvrn->DayOfWeek = 1;          // must initialize this to get schedules initialized
    state->dataGlobal->HourOfDay = 1;         // must initialize this to get schedules initialized
    state->dataGlobal->TimeStep = 1;          // must initialize this to get schedules initialized

    state->files.inputWeatherFilePath.filePath = configured_source_directory() / "tst/EnergyPlus/unit/Resources/HeatBalanceKivaManagerOSkyTest.epw";
    state->dataWeather->WeatherFileExists = true;
    HeatBalanceManager::GetHeatBalanceInput(*state);

    state->dataSurfaceGeometry->CosBldgRotAppGonly = 1.0;
    state->dataSurfaceGeometry->SinBldgRotAppGonly = 0.0;
    SurfaceGeometry::SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    HeatBalanceIntRadExchange::InitSolarViewFactors(*state);
    EXPECT_TRUE(compare_err_stream(""));
    EXPECT_FALSE(has_err_output(true));

    state->dataZoneEquip->ZoneEquipConfig.allocate(1);
    std::vector<int> controlledZoneEquipConfigNums;
    controlledZoneEquipConfigNums.push_back(1);
    state->dataHeatBal->Zone(1).IsControlled = true;
    state->dataZoneEquip->ZoneEquipConfig(1).NumInletNodes = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode.allocate(2);
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(1) = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(2) = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1) = 3;
    state->dataZoneEquip->ZoneEquipConfig(1).NumReturnNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode(1) = 4;
    state->dataZoneEquip->ZoneEquipConfig(1).FixedReturnFlow.allocate(1);

    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataHeatBal->Zone(1).SystemZoneNodeNumber = 5;
    state->dataEnvrn->OutBaroPress = 101325.0;

    state->dataLoopNodes->Node.allocate(4);

    state->dataHeatBalSurf->SurfTempInTmp.allocate(6);
    state->dataHeatBalSurf->SurfTempInTmp(1) = 15.0;
    state->dataHeatBalSurf->SurfTempInTmp(2) = 20.0;
    state->dataHeatBalSurf->SurfTempInTmp(3) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(4) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(5) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(6) = 25.0;

    // allocate surface level adj ratio data member
    state->dataHeatBalSurf->SurfWinCoeffAdjRatio.dimension(6, 1.0);

    state->dataLoopNodes->Node(1).Temp = 20.0;
    state->dataLoopNodes->Node(2).Temp = 20.0;
    state->dataLoopNodes->Node(3).Temp = 20.0;
    state->dataLoopNodes->Node(4).Temp = 20.0;
    state->dataLoopNodes->Node(1).MassFlowRate = 0.1;
    state->dataLoopNodes->Node(2).MassFlowRate = 0.1;
    state->dataLoopNodes->Node(3).MassFlowRate = 0.1;
    state->dataLoopNodes->Node(4).MassFlowRate = 0.1;

    state->dataHeatBalSurf->SurfHConvInt.allocate(6);
    state->dataHeatBalSurf->SurfHConvInt(1) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(2) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(3) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(4) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(5) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(6) = 0.5;
    state->dataMstBal->HConvInFD.allocate(6);
    state->dataMstBal->RhoVaporAirIn.allocate(6);
    state->dataMstBal->HMassConvInFD.allocate(6);

    state->dataHeatBal->ZoneWinHeatGain.allocate(1);
    state->dataHeatBal->ZoneWinHeatGainRep.allocate(1);
    state->dataHeatBal->ZoneWinHeatGainRepEnergy.allocate(1);

    Sched::GetSchedule(*state, "CORE_BOTTOM OCCUPANCY")->currentVal = -0.1;
    Sched::GetSchedule(*state, "CORE_BOTTOM ACTIVITY")->currentVal = 0.1;

    state->dataGlobal->BeginSimFlag = true;
    state->dataGlobal->KickOffSimulation = true;
    state->dataGlobal->TimeStepZoneSec = 900;

    state->dataHeatBalSurf->SurfWinCoeffAdjRatio.dimension(6, 1.0);
    AllocateSurfaceHeatBalArrays(*state);
    createFacilityElectricPowerServiceObject(*state);
    HeatBalanceManager::AllocateZoneHeatBalArrays(*state);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 24.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.001;
    state->dataZoneTempPredictorCorrector->spaceHeatBalance(1).MAT = 24.0;
    state->dataZoneTempPredictorCorrector->spaceHeatBalance(1).airHumRat = 0.001;
    SolarShading::AllocateModuleArrays(*state);
    SolarShading::DetermineShadowingCombinations(*state);
    for (int loop = 1; loop <= state->dataSurface->TotSurfaces; ++loop) {
        state->dataHeatBalSurf->SurfOutsideTempHist(1)(loop) = 20.0;
    }
    state->dataSurface->SurfTAirRef(1) = DataSurfaces::ZoneMeanAirTemp;
    state->dataSurface->SurfTAirRef(2) = DataSurfaces::AdjacentAirTemp;
    state->dataSurface->SurfTAirRef(3) = DataSurfaces::ZoneSupplyAirTemp;

    InitSurfaceHeatBalance(*state);

    state->dataSurface->Surface(5).HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::Kiva;
    state->dataSurface->AllHTKivaSurfaceList = {5};
    CalcHeatBalanceInsideSurf(*state);

    ReportSurfaceHeatBalance(*state);

    // Check that the inside face surface conduction = -(convection + radiation)
    EXPECT_NEAR(state->dataHeatBalSurf->SurfOpaqInsFaceCond(5), -21.7, 0.1);
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_TestSurfPropertyLocalEnv)
{

    std::string const idf_objects =
        delimited_string({"  Building,",
                          "    House with Local Air Nodes,  !- Name",
                          "    0,                       !- North Axis {deg}",
                          "    Suburbs,                 !- Terrain",
                          "    0.001,                   !- Loads Convergence Tolerance Value",
                          "    0.0050000,               !- Temperature Convergence Tolerance Value {deltaC}",
                          "    FullInteriorAndExterior, !- Solar Distribution",
                          "    25,                      !- Maximum Number of Warmup Days",
                          "    6;                       !- Minimum Number of Warmup Days",

                          "  Timestep,6;",

                          "  SurfaceConvectionAlgorithm:Inside,TARP;",

                          "  SurfaceConvectionAlgorithm:Outside,DOE-2;",

                          "  HeatBalanceAlgorithm,ConductionTransferFunction;",

                          "  SimulationControl,",
                          "    No,                      !- Do Zone Sizing Calculation",
                          "    No,                      !- Do System Sizing Calculation",
                          "    No,                      !- Do Plant Sizing Calculation",
                          "    Yes,                     !- Run Simulation for Sizing Periods",
                          "    Yes;                     !- Run Simulation for Weather File Run Periods",

                          "  RunPeriod,",
                          "    WinterDay,               !- Name",
                          "    1,                       !- Begin Month",
                          "    14,                      !- Begin Day of Month",
                          "    ,                        !- Begin Year",
                          "    1,                       !- End Month",
                          "    14,                      !- End Day of Month",
                          "    ,                        !- End Year",
                          "    Tuesday,                 !- Day of Week for Start Day",
                          "    Yes,                     !- Use Weather File Holidays and Special Days",
                          "    Yes,                     !- Use Weather File Daylight Saving Period",
                          "    No,                      !- Apply Weekend Holiday Rule",
                          "    Yes,                     !- Use Weather File Rain Indicators",
                          "    Yes;                     !- Use Weather File Snow Indicators",

                          "  RunPeriod,",
                          "    SummerDay,               !- Name",
                          "    7,                       !- Begin Month",
                          "    7,                       !- Begin Day of Month",
                          "    ,                        !- Begin Year",
                          "    7,                       !- End Month",
                          "    7,                       !- End Day of Month",
                          "    ,                        !- End Year",
                          "    Tuesday,                 !- Day of Week for Start Day",
                          "    Yes,                     !- Use Weather File Holidays and Special Days",
                          "    Yes,                     !- Use Weather File Daylight Saving Period",
                          "    No,                      !- Apply Weekend Holiday Rule",
                          "    Yes,                     !- Use Weather File Rain Indicators",
                          "    No;                      !- Use Weather File Snow Indicators",

                          "  Site:Location,",
                          "    CHICAGO_IL_USA TMY2-94846,  !- Name",
                          "    41.78,                   !- Latitude {deg}",
                          "    -87.75,                  !- Longitude {deg}",
                          "    -6.00,                   !- Time Zone {hr}",
                          "    190.00;                  !- Elevation {m}",

                          "  SizingPeriod:DesignDay,",
                          "    CHICAGO_IL_USA Annual Heating 99% Design Conditions DB,  !- Name",
                          "    1,                       !- Month",
                          "    21,                      !- Day of Month",
                          "    WinterDesignDay,         !- Day Type",
                          "    -17.3,                   !- Maximum Dry-Bulb Temperature {C}",
                          "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
                          "    Wetbulb,                 !- Humidity Condition Type",
                          "    -17.3,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
                          "    ,                        !- Humidity Condition Day Schedule Name",
                          "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
                          "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
                          "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
                          "    99063.,                  !- Barometric Pressure {Pa}",
                          "    4.9,                     !- Wind Speed {m/s}",
                          "    270,                     !- Wind Direction {deg}",
                          "    No,                      !- Rain Indicator",
                          "    No,                      !- Snow Indicator",
                          "    No,                      !- Daylight Saving Time Indicator",
                          "    ASHRAEClearSky,          !- Solar Model Indicator",
                          "    ,                        !- Beam Solar Day Schedule Name",
                          "    ,                        !- Diffuse Solar Day Schedule Name",
                          "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
                          "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
                          "    0.0;                     !- Sky Clearness",

                          "  SizingPeriod:DesignDay,",
                          "    CHICAGO_IL_USA Annual Cooling 1% Design Conditions DB/MCWB,  !- Name",
                          "    7,                       !- Month",
                          "    21,                      !- Day of Month",
                          "    SummerDesignDay,         !- Day Type",
                          "    31.5,                    !- Maximum Dry-Bulb Temperature {C}",
                          "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
                          "    Wetbulb,                 !- Humidity Condition Type",
                          "    23.0,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
                          "    ,                        !- Humidity Condition Day Schedule Name",
                          "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
                          "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
                          "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
                          "    99063.,                  !- Barometric Pressure {Pa}",
                          "    5.3,                     !- Wind Speed {m/s}",
                          "    230,                     !- Wind Direction {deg}",
                          "    No,                      !- Rain Indicator",
                          "    No,                      !- Snow Indicator",
                          "    No,                      !- Daylight Saving Time Indicator",
                          "    ASHRAEClearSky,          !- Solar Model Indicator",
                          "    ,                        !- Beam Solar Day Schedule Name",
                          "    ,                        !- Diffuse Solar Day Schedule Name",
                          "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
                          "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
                          "    1.0;                     !- Sky Clearness",

                          "  Site:GroundTemperature:BuildingSurface,20.03,20.03,20.13,20.30,20.43,20.52,20.62,20.77,20.78,20.55,20.44,20.20;",

                          "  Material,",
                          "    A1 - 1 IN STUCCO,        !- Name",
                          "    Smooth,                  !- Roughness",
                          "    2.5389841E-02,           !- Thickness {m}",
                          "    0.6918309,               !- Conductivity {W/m-K}",
                          "    1858.142,                !- Density {kg/m3}",
                          "    836.8000,                !- Specific Heat {J/kg-K}",
                          "    0.9000000,               !- Thermal Absorptance",
                          "    0.9200000,               !- Solar Absorptance",
                          "    0.9200000;               !- Visible Absorptance",

                          "  Material,",
                          "    CB11,                    !- Name",
                          "    MediumRough,             !- Roughness",
                          "    0.2032000,               !- Thickness {m}",
                          "    1.048000,                !- Conductivity {W/m-K}",
                          "    1105.000,                !- Density {kg/m3}",
                          "    837.0000,                !- Specific Heat {J/kg-K}",
                          "    0.9000000,               !- Thermal Absorptance",
                          "    0.2000000,               !- Solar Absorptance",
                          "    0.2000000;               !- Visible Absorptance",

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
                          "    IN05,                    !- Name",
                          "    Rough,                   !- Roughness",
                          "    0.2458000,               !- Thickness {m}",
                          "    4.3000001E-02,           !- Conductivity {W/m-K}",
                          "    10.00000,                !- Density {kg/m3}",
                          "    837.0000,                !- Specific Heat {J/kg-K}",
                          "    0.9000000,               !- Thermal Absorptance",
                          "    0.7500000,               !- Solar Absorptance",
                          "    0.7500000;               !- Visible Absorptance",

                          "  Material,",
                          "    PW03,                    !- Name",
                          "    MediumSmooth,            !- Roughness",
                          "    1.2700000E-02,           !- Thickness {m}",
                          "    0.1150000,               !- Conductivity {W/m-K}",
                          "    545.0000,                !- Density {kg/m3}",
                          "    1213.000,                !- Specific Heat {J/kg-K}",
                          "    0.9000000,               !- Thermal Absorptance",
                          "    0.7800000,               !- Solar Absorptance",
                          "    0.7800000;               !- Visible Absorptance",

                          "  Material,",
                          "    CC03,                    !- Name",
                          "    MediumRough,             !- Roughness",
                          "    0.1016000,               !- Thickness {m}",
                          "    1.310000,                !- Conductivity {W/m-K}",
                          "    2243.000,                !- Density {kg/m3}",
                          "    837.0000,                !- Specific Heat {J/kg-K}",
                          "    0.9000000,               !- Thermal Absorptance",
                          "    0.6500000,               !- Solar Absorptance",
                          "    0.6500000;               !- Visible Absorptance",

                          "  Material,",
                          "    HF-A3,                   !- Name",
                          "    Smooth,                  !- Roughness",
                          "    1.5000000E-03,           !- Thickness {m}",
                          "    44.96960,                !- Conductivity {W/m-K}",
                          "    7689.000,                !- Density {kg/m3}",
                          "    418.0000,                !- Specific Heat {J/kg-K}",
                          "    0.9000000,               !- Thermal Absorptance",
                          "    0.2000000,               !- Solar Absorptance",
                          "    0.2000000;               !- Visible Absorptance",

                          "  Material:NoMass,",
                          "    AR02,                    !- Name",
                          "    VeryRough,               !- Roughness",
                          "    7.8000002E-02,           !- Thermal Resistance {m2-K/W}",
                          "    0.9000000,               !- Thermal Absorptance",
                          "    0.7000000,               !- Solar Absorptance",
                          "    0.7000000;               !- Visible Absorptance",

                          "  Material:NoMass,",
                          "    CP02,                    !- Name",
                          "    Rough,                   !- Roughness",
                          "    0.2170000,               !- Thermal Resistance {m2-K/W}",
                          "    0.9000000,               !- Thermal Absorptance",
                          "    0.7500000,               !- Solar Absorptance",
                          "    0.7500000;               !- Visible Absorptance",

                          "  Construction,",
                          "    EXTWALL:LIVING,          !- Name",
                          "    A1 - 1 IN STUCCO,        !- Outside Layer",
                          "    GP01;                    !- Layer 3",

                          "  Construction,",
                          "    FLOOR:LIVING,            !- Name",
                          "    CC03,                    !- Outside Layer",
                          "    CP02;                    !- Layer 2",

                          "  Construction,",
                          "    ROOF,                    !- Name",
                          "    AR02,                    !- Outside Layer",
                          "    PW03;                    !- Layer 2",

                          "  Zone,",
                          "    LIVING ZONE,             !- Name",
                          "    0,                       !- Direction of Relative North {deg}",
                          "    0,                       !- X Origin {m}",
                          "    0,                       !- Y Origin {m}",
                          "    0,                       !- Z Origin {m}",
                          "    1,                       !- Type",
                          "    1,                       !- Multiplier",
                          "    autocalculate,           !- Ceiling Height {m}",
                          "    autocalculate;           !- Volume {m3}",

                          "  GlobalGeometryRules,",
                          "    UpperLeftCorner,         !- Starting Vertex Position",
                          "    CounterClockWise,        !- Vertex Entry Direction",
                          "    World;                   !- Coordinate System",

                          "  BuildingSurface:Detailed,",
                          "    Living:North,            !- Name",
                          "    Wall,                    !- Surface Type",
                          "    EXTWALL:LIVING,          !- Construction Name",
                          "    LIVING ZONE,             !- Zone Name",
                          "    ,                        !- Space Name",
                          "    Outdoors,                !- Outside Boundary Condition",
                          "    ,                        !- Outside Boundary Condition Object",
                          "    SunExposed,              !- Sun Exposure",
                          "    WindExposed,             !- Wind Exposure",
                          "    0.5000000,               !- View Factor to Ground",
                          "    4,                       !- Number of Vertices",
                          "    1,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
                          "    1,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
                          "    0,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
                          "    0,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

                          "  BuildingSurface:Detailed,",
                          "    Living:East,             !- Name",
                          "    Wall,                    !- Surface Type",
                          "    EXTWALL:LIVING,          !- Construction Name",
                          "    LIVING ZONE,             !- Zone Name",
                          "    ,                        !- Space Name",
                          "    Outdoors,                !- Outside Boundary Condition",
                          "    ,                        !- Outside Boundary Condition Object",
                          "    SunExposed,              !- Sun Exposure",
                          "    WindExposed,             !- Wind Exposure",
                          "    0.5000000,               !- View Factor to Ground",
                          "    4,                       !- Number of Vertices",
                          "    1,0,1,  !- X,Y,Z ==> Vertex 1 {m}",
                          "    1,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
                          "    1,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
                          "    1,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

                          "  BuildingSurface:Detailed,",
                          "    Living:South,            !- Name",
                          "    Wall,                    !- Surface Type",
                          "    EXTWALL:LIVING,          !- Construction Name",
                          "    LIVING ZONE,             !- Zone Name",
                          "    ,                        !- Space Name",
                          "    Outdoors,                !- Outside Boundary Condition",
                          "    ,                        !- Outside Boundary Condition Object",
                          "    SunExposed,              !- Sun Exposure",
                          "    WindExposed,             !- Wind Exposure",
                          "    0.5000000,               !- View Factor to Ground",
                          "    4,                       !- Number of Vertices",
                          "    0,0,1,  !- X,Y,Z ==> Vertex 1 {m}",
                          "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
                          "    1,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
                          "    1,0,1;  !- X,Y,Z ==> Vertex 4 {m}",

                          "  BuildingSurface:Detailed,",
                          "    Living:West,             !- Name",
                          "    Wall,                    !- Surface Type",
                          "    EXTWALL:LIVING,          !- Construction Name",
                          "    LIVING ZONE,             !- Zone Name",
                          "    ,                        !- Space Name",
                          "    Outdoors,                !- Outside Boundary Condition",
                          "    ,                        !- Outside Boundary Condition Object",
                          "    SunExposed,              !- Sun Exposure",
                          "    WindExposed,             !- Wind Exposure",
                          "    0.5000000,               !- View Factor to Ground",
                          "    4,                       !- Number of Vertices",
                          "    0,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
                          "    0,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
                          "    0,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
                          "    0,0,1;  !- X,Y,Z ==> Vertex 4 {m}",
                          "  BuildingSurface:Detailed,",
                          "    Living:Floor,            !- Name",
                          "    FLOOR,                   !- Surface Type",
                          "    FLOOR:LIVING,            !- Construction Name",
                          "    LIVING ZONE,             !- Zone Name",
                          "    ,                        !- Space Name",
                          "    Surface,                 !- Outside Boundary Condition",
                          "    Living:Floor,            !- Outside Boundary Condition Object",
                          "    NoSun,                   !- Sun Exposure",
                          "    NoWind,                  !- Wind Exposure",
                          "    0,                       !- View Factor to Ground",
                          "    4,                       !- Number of Vertices",
                          "    0,0,0,  !- X,Y,Z ==> Vertex 1 {m}",
                          "    0,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
                          "    1,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
                          "    1,0,0;  !- X,Y,Z ==> Vertex 4 {m}",

                          "  BuildingSurface:Detailed,",
                          "    Living:Ceiling,          !- Name",
                          "    ROOF,                 !- Surface Type",
                          "    ROOF,          !- Construction Name",
                          "    LIVING ZONE,             !- Zone Name",
                          "    ,                        !- Space Name",
                          "    Outdoors,                !- Outside Boundary Condition",
                          "    ,                        !- Outside Boundary Condition Object",
                          "    SunExposed,              !- Sun Exposure",
                          "    WindExposed,             !- Wind Exposure",
                          "    0,                       !- View Factor to Ground",
                          "    4,                       !- Number of Vertices",
                          "    0,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
                          "    0,0,1,  !- X,Y,Z ==> Vertex 2 {m}",
                          "    1,0,1,  !- X,Y,Z ==> Vertex 3 {m}",
                          "    1,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

                          "  SurfaceProperty:LocalEnvironment,",
                          "    LocEnv:Living:North,          !- Name",
                          "    Living:North,                 !- Exterior Surface Name",
                          "    ,                             !- External Shading Fraction Schedule Name",
                          "    ,                             !- Surrounding Surfaces Object Name",
                          "    OutdoorAirNode:0001;          !- Outdoor Air Node Name",

                          "  OutdoorAir:Node,",
                          "    OutdoorAirNode:0001,          !- Name",
                          "    ,                             !- Height Above Ground",
                          "    OutdoorAirNodeDryBulb:0001,   !- Drybulb Temperature Schedule Name",
                          "    OutdoorAirNodeWetBulb:0001,   !- Wetbulb Schedule Name",
                          "    OutdoorAirNodeWindSpeed:0001, !- Wind Speed Schedule Name",
                          "    OutdoorAirNodeWindDir:0001;   !- Wind Direction Schedule Name",

                          "  ScheduleTypeLimits,",
                          "    Any Number;                   !- Name",

                          "  Schedule:Compact,",
                          "    OutdoorAirNodeDryBulb:0001,   !- Name",
                          "    Any Number,                   !- Schedule Type Limits Name",
                          "    Through: 12/31,               !- Field 1",
                          "    For: AllDays,                 !- Field 2",
                          "    Until: 24:00, 15.0;           !- Field 3",

                          "  Schedule:Compact,",
                          "    OutdoorAirNodeWetBulb:0001,   !- Name",
                          "    Any Number,                   !- Schedule Type Limits Name",
                          "    Through: 12/31,               !- Field 1",
                          "    For: AllDays,                 !- Field 2",
                          "    Until: 24:00, 12.0;           !- Field 3",

                          "  Schedule:Compact,",
                          "    OutdoorAirNodeWindSpeed:0001, !- Name",
                          "    Any Number,                   !- Schedule Type Limits Name",
                          "    Through: 12/31,               !- Field 1",
                          "    For: AllDays,                 !- Field 2",
                          "    Until: 24:00, 1.23;           !- Field 3",

                          "  Schedule:Compact,",
                          "    OutdoorAirNodeWindDir:0001,   !- Name",
                          "    Any Number,                   !- Schedule Type Limits Name",
                          "    Through: 12/31,               !- Field 1",
                          "    For: AllDays,                 !- Field 2",
                          "    Until: 24:00, 90;             !- Field 3"});

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    bool ErrorsFound = false;

    HeatBalanceManager::GetProjectControlData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    Material::GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    SurfaceGeometry::GetGeometryParameters(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    state->dataSurfaceGeometry->CosBldgRotAppGonly = 1.0;
    state->dataSurfaceGeometry->SinBldgRotAppGonly = 0.0;
    SurfaceGeometry::SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    HeatBalanceIntRadExchange::InitSolarViewFactors(*state);
    EXPECT_FALSE(has_err_output(true));

    EXPECT_TRUE(state->dataGlobal->AnyLocalEnvironmentsInModel);

    state->dataZoneEquip->ZoneEquipConfig.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneName = "LIVING ZONE";
    std::vector<int> controlledZoneEquipConfigNums;
    controlledZoneEquipConfigNums.push_back(1);
    state->dataHeatBal->Zone(1).IsControlled = true;

    state->dataZoneEquip->ZoneEquipConfig(1).NumInletNodes = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode.allocate(2);
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(1) = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(2) = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1) = 3;
    state->dataZoneEquip->ZoneEquipConfig(1).NumReturnNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode(1) = 4;
    state->dataZoneEquip->ZoneEquipConfig(1).FixedReturnFlow.allocate(1);

    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataHeatBal->Zone(1).SystemZoneNodeNumber = 5;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->spaceHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 24.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.001;
    state->dataZoneTempPredictorCorrector->spaceHeatBalance(1).MAT = 24.0;
    state->dataZoneTempPredictorCorrector->spaceHeatBalance(1).airHumRat = 0.001;

    state->dataLoopNodes->Node.allocate(4);

    state->dataHeatBalSurf->SurfTempInTmp.allocate(6);
    state->dataHeatBalSurf->SurfTempInTmp(1) = 15.0;
    state->dataHeatBalSurf->SurfTempInTmp(2) = 20.0;
    state->dataHeatBalSurf->SurfTempInTmp(3) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(4) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(5) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(6) = 25.0;

    state->dataLoopNodes->Node(1).Temp = 20.0;
    state->dataLoopNodes->Node(2).Temp = 20.0;
    state->dataLoopNodes->Node(3).Temp = 20.0;
    state->dataLoopNodes->Node(4).Temp = 20.0;
    state->dataLoopNodes->Node(1).MassFlowRate = 0.1;
    state->dataLoopNodes->Node(2).MassFlowRate = 0.1;
    state->dataLoopNodes->Node(3).MassFlowRate = 0.1;
    state->dataLoopNodes->Node(4).MassFlowRate = 0.1;

    state->dataHeatBalSurf->SurfHConvInt.allocate(6);
    state->dataHeatBalSurf->SurfHConvInt(1) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(2) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(3) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(4) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(5) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(6) = 0.5;
    state->dataMstBal->HConvInFD.allocate(6);
    state->dataMstBal->RhoVaporAirIn.allocate(6);
    state->dataMstBal->HMassConvInFD.allocate(6);

    state->dataGlobal->BeginSimFlag = true;
    state->dataGlobal->KickOffSimulation = true;
    state->dataGlobal->TimeStepZoneSec = 900;
    state->dataHeatBal->ZoneWinHeatGain.allocate(1);
    state->dataHeatBal->ZoneWinHeatGainRep.allocate(1);
    state->dataHeatBal->ZoneWinHeatGainRepEnergy.allocate(1);

    // Set up
    state->dataHeatBalSurf->SurfWinCoeffAdjRatio.dimension(6, 1.0);

    AllocateSurfaceHeatBalArrays(*state);
    createFacilityElectricPowerServiceObject(*state);
    HeatBalanceManager::AllocateZoneHeatBalArrays(*state);
    SolarShading::AllocateModuleArrays(*state);
    SolarShading::DetermineShadowingCombinations(*state);
    OutAirNodeManager::GetOutAirNodesInput(*state);

    Sched::GetSchedule(*state, "OUTDOORAIRNODEDRYBULB:0001")->currentVal = 25.0;
    Sched::GetSchedule(*state, "OUTDOORAIRNODEWETBULB:0001")->currentVal = 20.0;
    Sched::GetSchedule(*state, "OUTDOORAIRNODEWINDSPEED:0001")->currentVal = 1.5;
    Sched::GetSchedule(*state, "OUTDOORAIRNODEWINDDIR:0001")->currentVal = 90.0;
    for (int loop = 1; loop <= state->dataSurface->TotSurfaces; ++loop) {
        state->dataHeatBalSurf->SurfOutsideTempHist(1)(loop) = 20.0;
    }
    state->dataSurface->SurfTAirRef(1) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    state->dataSurface->SurfTAirRef(2) = DataSurfaces::RefAirTemp::AdjacentAirTemp;
    state->dataSurface->SurfTAirRef(3) = DataSurfaces::RefAirTemp::ZoneSupplyAirTemp;

    OutAirNodeManager::InitOutAirNodes(*state);

    // Test if local nodes data correctly overwritten
    EXPECT_EQ(25.0, state->dataLoopNodes->Node(1).OutAirDryBulb);
    EXPECT_EQ(20.0, state->dataLoopNodes->Node(1).OutAirWetBulb);
    EXPECT_EQ(1.5, state->dataLoopNodes->Node(1).OutAirWindSpeed);
    EXPECT_EQ(90.0, state->dataLoopNodes->Node(1).OutAirWindDir);
    EXPECT_NEAR(0.012611481326656135, state->dataLoopNodes->Node(1).HumRat, 0.000000000000001);
    EXPECT_NEAR(57247.660939392081, state->dataLoopNodes->Node(1).Enthalpy, 0.000000001);

    InitSurfaceHeatBalance(*state);

    // Test if local value correctly overwritten
    EXPECT_EQ(25.0, state->dataSurface->SurfOutDryBulbTemp(1));
    EXPECT_EQ(20.0, state->dataSurface->SurfOutWetBulbTemp(1));
    EXPECT_EQ(1.5, state->dataSurface->SurfOutWindSpeed(1));
    EXPECT_EQ(90.0, state->dataSurface->SurfOutWindDir(1));

    // Test if local value used in surface hc calculation
    // Surface(1) - local; Surface(2) - global;
    for (int SurfNum = 1; SurfNum <= 6; SurfNum++) {
        state->dataSurface->surfExtConv(SurfNum).model = Convect::HcExt::ASHRAESimple;
    }
    CalcHeatBalanceOutsideSurf(*state);
    Real64 HExt_Expect_Surf1 = Convect::CalcASHRAESimpExtConvCoeff(Material::SurfaceRoughness::Smooth, 1.5);
    Real64 HExt_Expect_Surf2 = Convect::CalcASHRAESimpExtConvCoeff(Material::SurfaceRoughness::Smooth, 0.0);
    EXPECT_EQ(HExt_Expect_Surf1, state->dataHeatBalSurf->SurfHConvExt(1));
    EXPECT_EQ(HExt_Expect_Surf2, state->dataHeatBalSurf->SurfHConvExt(2));
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_TestSurfPropertySrdSurfLWR)
{

    std::string const idf_objects = delimited_string({
        "  Building,",
        "    House with Local Air Nodes,  !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    0.001,                   !- Loads Convergence Tolerance Value",
        "    0.0050000,               !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullInteriorAndExterior, !- Solar Distribution",
        "    25,                      !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",

        "  Timestep,6;",

        "  SurfaceConvectionAlgorithm:Inside,TARP;",

        "  SurfaceConvectionAlgorithm:Outside,DOE-2;",

        "  HeatBalanceAlgorithm,ConductionTransferFunction;",

        "  SimulationControl,",
        "    No,                      !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    Yes,                     !- Run Simulation for Sizing Periods",
        "    Yes;                     !- Run Simulation for Weather File Run Periods",

        "  RunPeriod,",
        "    WinterDay,               !- Name",
        "    1,                       !- Begin Month",
        "    14,                      !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    1,                       !- End Month",
        "    14,                      !- End Day of Month",
        "    ,                        !- End Year",
        "    Tuesday,                 !- Day of Week for Start Day",
        "    Yes,                     !- Use Weather File Holidays and Special Days",
        "    Yes,                     !- Use Weather File Daylight Saving Period",
        "    No,                      !- Apply Weekend Holiday Rule",
        "    Yes,                     !- Use Weather File Rain Indicators",
        "    Yes;                     !- Use Weather File Snow Indicators",

        "  RunPeriod,",
        "    SummerDay,               !- Name",
        "    7,                       !- Begin Month",
        "    7,                       !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    7,                       !- End Month",
        "    7,                       !- End Day of Month",
        "    ,                        !- End Year",
        "    Tuesday,                 !- Day of Week for Start Day",
        "    No,                      !- Apply Weekend Holiday Rule",
        "    Yes,                     !- Use Weather File Rain Indicators",
        "    No;                      !- Use Weather File Snow Indicators",

        "  Site:Location,",
        "    CHICAGO_IL_USA TMY2-94846,  !- Name",
        "    41.78,                   !- Latitude {deg}",
        "    -87.75,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190.00;                  !- Elevation {m}",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Heating 99% Design Conditions DB,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    WinterDesignDay,         !- Day Type",
        "    -17.3,                   !- Maximum Dry-Bulb Temperature {C}",
        "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    -17.3,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    4.9,                     !- Wind Speed {m/s}",
        "    270,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    0.0;                     !- Sky Clearness",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Cooling 1% Design Conditions DB/MCWB,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    31.5,                    !- Maximum Dry-Bulb Temperature {C}",
        "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    23.0,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    5.3,                     !- Wind Speed {m/s}",
        "    230,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    1.0;                     !- Sky Clearness",

        "  Site:GroundTemperature:BuildingSurface,20.03,20.03,20.13,20.30,20.43,20.52,20.62,20.77,20.78,20.55,20.44,20.20;",

        "  Material,",
        "    A1 - 1 IN STUCCO,        !- Name",
        "    Smooth,                  !- Roughness",
        "    2.5389841E-02,           !- Thickness {m}",
        "    0.6918309,               !- Conductivity {W/m-K}",
        "    1858.142,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Material,",
        "    CB11,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    0.2032000,               !- Thickness {m}",
        "    1.048000,                !- Conductivity {W/m-K}",
        "    1105.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.2000000,               !- Solar Absorptance",
        "    0.2000000;               !- Visible Absorptance",

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
        "    IN05,                    !- Name",
        "    Rough,                   !- Roughness",
        "    0.2458000,               !- Thickness {m}",
        "    4.3000001E-02,           !- Conductivity {W/m-K}",
        "    10.00000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    PW03,                    !- Name",
        "    MediumSmooth,            !- Roughness",
        "    1.2700000E-02,           !- Thickness {m}",
        "    0.1150000,               !- Conductivity {W/m-K}",
        "    545.0000,                !- Density {kg/m3}",
        "    1213.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7800000,               !- Solar Absorptance",
        "    0.7800000;               !- Visible Absorptance",

        "  Material,",
        "    CC03,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1016000,               !- Thickness {m}",
        "    1.310000,                !- Conductivity {W/m-K}",
        "    2243.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

        "  Material,",
        "    HF-A3,                   !- Name",
        "    Smooth,                  !- Roughness",
        "    1.5000000E-03,           !- Thickness {m}",
        "    44.96960,                !- Conductivity {W/m-K}",
        "    7689.000,                !- Density {kg/m3}",
        "    418.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.2000000,               !- Solar Absorptance",
        "    0.2000000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    AR02,                    !- Name",
        "    VeryRough,               !- Roughness",
        "    7.8000002E-02,           !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7000000,               !- Solar Absorptance",
        "    0.7000000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    CP02,                    !- Name",
        "    Rough,                   !- Roughness",
        "    0.2170000,               !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Construction,",
        "    EXTWALL:LIVING,          !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    GP01;                    !- Layer 3",

        "  Construction,",
        "    FLOOR:LIVING,            !- Name",
        "    CC03,                    !- Outside Layer",
        "    CP02;                    !- Layer 2",

        "  Construction,",
        "    ROOF,                    !- Name",
        "    AR02,                    !- Outside Layer",
        "    PW03;                    !- Layer 2",

        "  Zone,",
        "    LIVING ZONE,             !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    World;                   !- Coordinate System",

        "  BuildingSurface:Detailed,",
        "    Living:North,            !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    1,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    1,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:East,             !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    1,0,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    1,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:South,            !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,0,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:West,             !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,0,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Floor,            !- Name",
        "    FLOOR,                   !- Surface Type",
        "    FLOOR:LIVING,            !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Living:Floor,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,0,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Ceiling,          !- Name",
        "    ROOF,                 !- Surface Type",
        "    ROOF,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,1,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,0,1,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  SurfaceProperty:LocalEnvironment,",
        "    LocEnv:Living:North,          !- Name",
        "    Living:North,                 !- Exterior Surface Name",
        "    ,                             !- External Shading Fraction Schedule Name",
        "    SrdSurfs:Living:North,        !- Surrounding Surfaces Object Name",
        "    ;                             !- Outdoor Air Node Name",

        "  SurfaceProperty:LocalEnvironment,",
        "    LocEnv:Living:East,           !- Name",
        "    Living:East,                  !- Exterior Surface Name",
        "    ,                             !- External Shading Fraction Schedule Name",
        "    SrdSurfs:Living:East,         !- Surrounding Surfaces Object Name",
        "    ;                             !- Outdoor Air Node Name",

        "  SurfaceProperty:LocalEnvironment,",
        "    LocEnv:Living:South,          !- Name",
        "    Living:South,                 !- Exterior Surface Name",
        "    ,                             !- External Shading Fraction Schedule Name",
        "    SrdSurfs:Living:South,        !- Surrounding Surfaces Object Name",
        "    ;                             !- Outdoor Air Node Name",

        "  SurfaceProperty:SurroundingSurfaces,",
        "    SrdSurfs:Living:North, !- Name",
        "    0.3,",
        "    Sky Temp Sch,",
        "    0.1,",
        "    Ground Temp Sch,",
        "    SurroundingSurface1,",
        "    0.6,",
        "    Surrounding Temp Sch 1;",

        "  SurfaceProperty:SurroundingSurfaces,",
        "    SrdSurfs:Living:East, !- Name",
        "    0.2,",
        "    ,",
        "    0.2,",
        "    ,",
        "    SurroundingSurface1,",
        "    0.3,",
        "    Surrounding Temp Sch 1,",
        "    SurroundingSurface2,",
        "    0.3,",
        "    Surrounding Temp Sch 1;",

        "  SurfaceProperty:SurroundingSurfaces,",
        "    SrdSurfs:Living:South,     !- Name",
        "    ,",
        "    ,",
        "    0.25,",
        "    ,",
        "    SurroundingSurface1,",
        "    0.5,",
        "    Surrounding Temp Sch 1;",

        "  ScheduleTypeLimits,",
        "    Any Number;                   !- Name",

        "  Schedule:Compact,",
        "    Surrounding Temp Sch 1,       !- Name",
        "    Any Number,                   !- Schedule Type Limits Name",
        "    Through: 12/31,               !- Field 1",
        "    For: AllDays,                 !- Field 2",
        "    Until: 24:00, 15.0;           !- Field 3",

        "  Schedule:Compact,",
        "    Sky Temp Sch,                 !- Name",
        "    Any Number,                   !- Schedule Type Limits Name",
        "    Through: 12/31,               !- Field 1",
        "    For: AllDays,                 !- Field 2",
        "    Until: 24:00, 15.0;           !- Field 3",

        "  Schedule:Compact,",
        "    Ground Temp Sch,              !- Name",
        "    Any Number,                   !- Schedule Type Limits Name",
        "    Through: 12/31,               !- Field 1",
        "    For: AllDays,                 !- Field 2",
        "    Until: 24:00, 15.0;           !- Field 3",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound = false;
    HeatBalanceManager::GetProjectControlData(*state, ErrorsFound); // read project control data
    EXPECT_FALSE(ErrorsFound);                                      // expect no errors
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    Material::GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    SurfaceGeometry::GetGeometryParameters(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    state->dataSurfaceGeometry->CosBldgRotAppGonly = 1.0;
    state->dataSurfaceGeometry->SinBldgRotAppGonly = 0.0;
    SurfaceGeometry::SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    HeatBalanceIntRadExchange::InitSolarViewFactors(*state);
    EXPECT_FALSE(has_err_output(true));

    EXPECT_TRUE(state->dataGlobal->AnyLocalEnvironmentsInModel);

    state->dataZoneEquip->ZoneEquipConfig.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneName = "LIVING ZONE";
    std::vector<int> controlledZoneEquipConfigNums;
    controlledZoneEquipConfigNums.push_back(1);
    state->dataHeatBal->Zone(1).IsControlled = true;

    state->dataZoneEquip->ZoneEquipConfig(1).NumInletNodes = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode.allocate(2);
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(1) = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(2) = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1) = 3;
    state->dataZoneEquip->ZoneEquipConfig(1).NumReturnNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode(1) = 4;
    state->dataZoneEquip->ZoneEquipConfig(1).FixedReturnFlow.allocate(1);

    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataHeatBal->Zone(1).SystemZoneNodeNumber = 5;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->spaceHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 24.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.001;
    state->dataZoneTempPredictorCorrector->spaceHeatBalance(1).MAT = 24.0;
    state->dataZoneTempPredictorCorrector->spaceHeatBalance(1).airHumRat = 0.001;

    state->dataLoopNodes->Node.allocate(4);

    state->dataHeatBalSurf->SurfTempInTmp.allocate(6);
    state->dataHeatBalSurf->SurfTempInTmp(1) = 15.0;
    state->dataHeatBalSurf->SurfTempInTmp(2) = 20.0;
    state->dataHeatBalSurf->SurfTempInTmp(3) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(4) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(5) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(6) = 25.0;

    state->dataLoopNodes->Node(1).Temp = 20.0;
    state->dataLoopNodes->Node(2).Temp = 20.0;
    state->dataLoopNodes->Node(3).Temp = 20.0;
    state->dataLoopNodes->Node(4).Temp = 20.0;
    state->dataLoopNodes->Node(1).MassFlowRate = 0.1;
    state->dataLoopNodes->Node(2).MassFlowRate = 0.1;
    state->dataLoopNodes->Node(3).MassFlowRate = 0.1;
    state->dataLoopNodes->Node(4).MassFlowRate = 0.1;

    state->dataHeatBalSurf->SurfHConvInt.allocate(6);
    state->dataHeatBalSurf->SurfHConvInt(1) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(2) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(3) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(4) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(5) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(6) = 0.5;
    state->dataMstBal->HConvInFD.allocate(6);
    state->dataMstBal->RhoVaporAirIn.allocate(6);
    state->dataMstBal->HMassConvInFD.allocate(6);

    state->dataGlobal->BeginSimFlag = true;
    state->dataGlobal->KickOffSimulation = true;
    state->dataGlobal->TimeStepZoneSec = 900;
    state->dataHeatBal->ZoneWinHeatGain.allocate(1);
    state->dataHeatBal->ZoneWinHeatGainRep.allocate(1);
    state->dataHeatBal->ZoneWinHeatGainRepEnergy.allocate(1);

    // Set up
    state->dataHeatBalSurf->SurfWinCoeffAdjRatio.dimension(6, 1.0);

    AllocateSurfaceHeatBalArrays(*state);
    createFacilityElectricPowerServiceObject(*state);
    HeatBalanceManager::AllocateZoneHeatBalArrays(*state);
    SolarShading::AllocateModuleArrays(*state);
    SolarShading::DetermineShadowingCombinations(*state);

    state->dataSurface->SurfTAirRef(1) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    state->dataSurface->SurfTAirRef(2) = DataSurfaces::RefAirTemp::AdjacentAirTemp;
    state->dataSurface->SurfTAirRef(3) = DataSurfaces::RefAirTemp::ZoneSupplyAirTemp;

    InitSurfacePropertyViewFactors(*state);

    InitSurfaceHeatBalance(*state);

    state->dataSurface->SurfAirSkyRadSplit.allocate(6);
    Sched::GetSchedule(*state, "SURROUNDING TEMP SCH 1")->currentVal = 25.0; // Srd Srfs Temp
    Sched::GetSchedule(*state, "SKY TEMP SCH")->currentVal = 15.0;           // Sky temp
    Sched::GetSchedule(*state, "GROUND TEMP SCH")->currentVal = 22.0;        // Grd temp

    for (int SurfNum = 1; SurfNum <= 6; SurfNum++) {
        state->dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum) = 20; // Surf temp
        state->dataSurface->SurfOutDryBulbTemp(SurfNum) = 22;         // Air temp
        state->dataSurface->surfExtConv(SurfNum).model = Convect::HcExt::MoWiTTHcOutside;
        state->dataSurface->SurfAirSkyRadSplit(SurfNum) = 1.0;
    }
    CalcHeatBalanceOutsideSurf(*state);

    // Test if local value correctly overwritten
    // Surface(1-3) - local; Surface(4-6) - global;
    EXPECT_DOUBLE_EQ(0.3, state->dataSurface->Surface(1).ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.1, state->dataSurface->Surface(1).ViewFactorGroundIR);
    EXPECT_DOUBLE_EQ(0.2, state->dataSurface->Surface(2).ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.2, state->dataSurface->Surface(2).ViewFactorGroundIR);
    EXPECT_DOUBLE_EQ(0.25, state->dataSurface->Surface(3).ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.25, state->dataSurface->Surface(3).ViewFactorGroundIR);
    // Test if sky and grd view factor and temperature correctly overwritten
    EXPECT_DOUBLE_EQ((Constant::StefanBoltzmann * 0.9 * 0.3 * (pow_4(20.0 + Constant::Kelvin) - pow_4(15.0 + Constant::Kelvin)) / (20.0 - 15.0)),
                     state->dataHeatBalSurf->SurfHSkyExt(1));
    EXPECT_DOUBLE_EQ((Constant::StefanBoltzmann * 0.9 * 0.1 * (pow_4(20.0 + Constant::Kelvin) - pow_4(22.0 + Constant::Kelvin)) / (20.0 - 22.0)),
                     state->dataHeatBalSurf->SurfHGrdExt(1));

    // Test if LWR from surrounding surfaces correctly calculated
    EXPECT_DOUBLE_EQ(Constant::StefanBoltzmann * 0.9 * 0.6 * (pow_4(25.0 + Constant::Kelvin) - pow_4(20.0 + Constant::Kelvin)),
                     state->dataHeatBalSurf->SurfQRadLWOutSrdSurfs(1));
    EXPECT_NEAR(Constant::StefanBoltzmann * 0.9 *
                    (0.3 * (pow_4(25.0 + Constant::Kelvin) - pow_4(20.0 + Constant::Kelvin)) +
                     0.3 * (pow_4(25.0 + Constant::Kelvin) - pow_4(20.0 + Constant::Kelvin))),
                state->dataHeatBalSurf->SurfQRadLWOutSrdSurfs(2),
                0.0000000001);
    EXPECT_NEAR(Constant::StefanBoltzmann * 0.9 * 0.5 * (pow_4(25.0 + Constant::Kelvin) - pow_4(20.0 + Constant::Kelvin)),
                state->dataHeatBalSurf->SurfQRadLWOutSrdSurfs(3),
                0.0000000001);
    EXPECT_DOUBLE_EQ(0.0, state->dataHeatBalSurf->SurfQRadLWOutSrdSurfs(4));
    // LWR Exchange Coefficient check for CTF method heat balance algorithm
    EXPECT_EQ(state->dataHeatBal->OverallHeatTransferSolutionAlgo, DataSurfaces::HeatTransferModel::CTF);
    // HSurrFD coeffcient are zeros since the heat balance algorithm is not CondFD
    EXPECT_DOUBLE_EQ(0.0, state->dataMstBal->HSurrFD(1));
    EXPECT_DOUBLE_EQ(0.0, state->dataMstBal->HSurrFD(2));
    EXPECT_DOUBLE_EQ(0.0, state->dataMstBal->HSurrFD(3));
    EXPECT_DOUBLE_EQ(0.0, state->dataMstBal->HSurrFD(4));
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_SurfaceCOnstructionIndexTest)
{

    std::string const idf_objects = delimited_string({
        " Output:Variable,Perimeter_ZN_1_wall_south_Window_1,Surface Window Transmitted Solar Radiation Rate,timestep;",
        " Output:Variable,*,SURFACE CONSTRUCTION INDEX,timestep;",
        " Output:Diagnostics, DisplayAdvancedReportVariables;",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->DisplayAdvancedReportVariables = true;

    state->dataSurface->TotSurfaces = 1;
    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->TotConstructs = 1;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);
    state->dataSurface->SurfaceWindow.allocate(state->dataSurface->TotSurfaces);
    state->dataConstruction->Construct.allocate(state->dataHeatBal->TotConstructs);
    state->dataHeatBal->AnyInternalHeatSourceInInput = true;

    state->dataSurface->Surface(1).Class = DataSurfaces::SurfaceClass::Wall;
    state->dataSurface->Surface(1).HeatTransSurf = true;
    state->dataSurface->Surface(1).HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::CTF;
    state->dataSurface->Surface(1).ExtBoundCond = 1;
    state->dataSurface->Surface(1).Construction = 1;

    state->dataConstruction->Construct(1).NumCTFTerms = 2;
    state->dataConstruction->Construct(1).SourceSinkPresent = true;
    state->dataConstruction->Construct(1).NumHistories = 1;
    state->dataConstruction->Construct(1).CTFTUserOut[0] = 0.5;
    state->dataConstruction->Construct(1).CTFTUserIn[0] = 0.25;
    state->dataConstruction->Construct(1).CTFTUserSource[0] = 0.25;
    SurfaceGeometry::AllocateSurfaceArrays(*state);
    AllocateSurfaceHeatBalArrays(*state); // allocates a host of variables related to CTF calculations
    OutputProcessor::GetReportVariableInput(*state);

    EXPECT_EQ(state->dataOutputProcessor->reqVars[1]->name, "SURFACE CONSTRUCTION INDEX");
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_TestSurfTempCalcHeatBalanceAddSourceTerm)
{

    std::string const idf_objects =
        delimited_string({"  Building,",
                          "    House with AirflowNetwork simulation,  !- Name",
                          "    0,                       !- North Axis {deg}",
                          "    Suburbs,                 !- Terrain",
                          "    0.001,                   !- Loads Convergence Tolerance Value",
                          "    0.0050000,               !- Temperature Convergence Tolerance Value {deltaC}",
                          "    FullInteriorAndExterior, !- Solar Distribution",
                          "    25,                      !- Maximum Number of Warmup Days",
                          "    6;                       !- Minimum Number of Warmup Days",

                          "  Timestep,6;",

                          "  SurfaceConvectionAlgorithm:Inside,TARP;",

                          "  SurfaceConvectionAlgorithm:Outside,DOE-2;",

                          "  HeatBalanceAlgorithm,ConductionTransferFunction;",

                          "  SimulationControl,",
                          "    No,                      !- Do Zone Sizing Calculation",
                          "    No,                      !- Do System Sizing Calculation",
                          "    No,                      !- Do Plant Sizing Calculation",
                          "    Yes,                     !- Run Simulation for Sizing Periods",
                          "    No;                      !- Run Simulation for Weather File Run Periods",

                          "  RunPeriod,",
                          "    WinterDay,               !- Name",
                          "    1,                       !- Begin Month",
                          "    14,                      !- Begin Day of Month",
                          "    ,                        !- Begin Year",
                          "    1,                       !- End Month",
                          "    14,                      !- End Day of Month",
                          "    ,                        !- End Year",
                          "    Tuesday,                 !- Day of Week for Start Day",
                          "    Yes,                     !- Use Weather File Holidays and Special Days",
                          "    Yes,                     !- Use Weather File Daylight Saving Period",
                          "    No,                      !- Apply Weekend Holiday Rule",
                          "    Yes,                     !- Use Weather File Rain Indicators",
                          "    Yes;                     !- Use Weather File Snow Indicators",

                          "  RunPeriod,",
                          "    SummerDay,               !- Name",
                          "    7,                       !- Begin Month",
                          "    7,                       !- Begin Day of Month",
                          "    ,                        !- Begin Year",
                          "    7,                       !- End Month",
                          "    7,                       !- End Day of Month",
                          "    ,                        !- End Year",
                          "    Tuesday,                 !- Day of Week for Start Day",
                          "    Yes,                     !- Use Weather File Holidays and Special Days",
                          "    Yes,                     !- Use Weather File Daylight Saving Period",
                          "    No,                      !- Apply Weekend Holiday Rule",
                          "    Yes,                     !- Use Weather File Rain Indicators",
                          "    No;                      !- Use Weather File Snow Indicators",

                          "  Site:Location,",
                          "    CHICAGO_IL_USA TMY2-94846,  !- Name",
                          "    41.78,                   !- Latitude {deg}",
                          "    -87.75,                  !- Longitude {deg}",
                          "    -6.00,                   !- Time Zone {hr}",
                          "    190.00;                  !- Elevation {m}",

                          "  SizingPeriod:DesignDay,",
                          "    CHICAGO_IL_USA Annual Heating 99% Design Conditions DB,  !- Name",
                          "    1,                       !- Month",
                          "    21,                      !- Day of Month",
                          "    WinterDesignDay,         !- Day Type",
                          "    -17.3,                   !- Maximum Dry-Bulb Temperature {C}",
                          "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
                          "    Wetbulb,                 !- Humidity Condition Type",
                          "    -17.3,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
                          "    ,                        !- Humidity Condition Day Schedule Name",
                          "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
                          "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
                          "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
                          "    99063.,                  !- Barometric Pressure {Pa}",
                          "    4.9,                     !- Wind Speed {m/s}",
                          "    270,                     !- Wind Direction {deg}",
                          "    No,                      !- Rain Indicator",
                          "    No,                      !- Snow Indicator",
                          "    No,                      !- Daylight Saving Time Indicator",
                          "    ASHRAEClearSky,          !- Solar Model Indicator",
                          "    ,                        !- Beam Solar Day Schedule Name",
                          "    ,                        !- Diffuse Solar Day Schedule Name",
                          "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
                          "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
                          "    0.0;                     !- Sky Clearness",

                          "  SizingPeriod:DesignDay,",
                          "    CHICAGO_IL_USA Annual Cooling 1% Design Conditions DB/MCWB,  !- Name",
                          "    7,                       !- Month",
                          "    21,                      !- Day of Month",
                          "    SummerDesignDay,         !- Day Type",
                          "    31.5,                    !- Maximum Dry-Bulb Temperature {C}",
                          "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
                          "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
                          "    Wetbulb,                 !- Humidity Condition Type",
                          "    23.0,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
                          "    ,                        !- Humidity Condition Day Schedule Name",
                          "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
                          "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
                          "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
                          "    99063.,                  !- Barometric Pressure {Pa}",
                          "    5.3,                     !- Wind Speed {m/s}",
                          "    230,                     !- Wind Direction {deg}",
                          "    No,                      !- Rain Indicator",
                          "    No,                      !- Snow Indicator",
                          "    No,                      !- Daylight Saving Time Indicator",
                          "    ASHRAEClearSky,          !- Solar Model Indicator",
                          "    ,                        !- Beam Solar Day Schedule Name",
                          "    ,                        !- Diffuse Solar Day Schedule Name",
                          "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
                          "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
                          "    1.0;                     !- Sky Clearness",

                          "  Site:GroundTemperature:BuildingSurface,20.03,20.03,20.13,20.30,20.43,20.52,20.62,20.77,20.78,20.55,20.44,20.20;",

                          "  Material,",
                          "    A1 - 1 IN STUCCO,        !- Name",
                          "    Smooth,                  !- Roughness",
                          "    2.5389841E-02,           !- Thickness {m}",
                          "    0.6918309,               !- Conductivity {W/m-K}",
                          "    1858.142,                !- Density {kg/m3}",
                          "    836.8000,                !- Specific Heat {J/kg-K}",
                          "    0.9000000,               !- Thermal Absorptance",
                          "    0.9200000,               !- Solar Absorptance",
                          "    0.9200000;               !- Visible Absorptance",

                          "  Material,",
                          "    CB11,                    !- Name",
                          "    MediumRough,             !- Roughness",
                          "    0.2032000,               !- Thickness {m}",
                          "    1.048000,                !- Conductivity {W/m-K}",
                          "    1105.000,                !- Density {kg/m3}",
                          "    837.0000,                !- Specific Heat {J/kg-K}",
                          "    0.9000000,               !- Thermal Absorptance",
                          "    0.2000000,               !- Solar Absorptance",
                          "    0.2000000;               !- Visible Absorptance",

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
                          "    IN05,                    !- Name",
                          "    Rough,                   !- Roughness",
                          "    0.2458000,               !- Thickness {m}",
                          "    4.3000001E-02,           !- Conductivity {W/m-K}",
                          "    10.00000,                !- Density {kg/m3}",
                          "    837.0000,                !- Specific Heat {J/kg-K}",
                          "    0.9000000,               !- Thermal Absorptance",
                          "    0.7500000,               !- Solar Absorptance",
                          "    0.7500000;               !- Visible Absorptance",

                          "  Material,",
                          "    PW03,                    !- Name",
                          "    MediumSmooth,            !- Roughness",
                          "    1.2700000E-02,           !- Thickness {m}",
                          "    0.1150000,               !- Conductivity {W/m-K}",
                          "    545.0000,                !- Density {kg/m3}",
                          "    1213.000,                !- Specific Heat {J/kg-K}",
                          "    0.9000000,               !- Thermal Absorptance",
                          "    0.7800000,               !- Solar Absorptance",
                          "    0.7800000;               !- Visible Absorptance",

                          "  Material,",
                          "    CC03,                    !- Name",
                          "    MediumRough,             !- Roughness",
                          "    0.1016000,               !- Thickness {m}",
                          "    1.310000,                !- Conductivity {W/m-K}",
                          "    2243.000,                !- Density {kg/m3}",
                          "    837.0000,                !- Specific Heat {J/kg-K}",
                          "    0.9000000,               !- Thermal Absorptance",
                          "    0.6500000,               !- Solar Absorptance",
                          "    0.6500000;               !- Visible Absorptance",

                          "  Material,",
                          "    HF-A3,                   !- Name",
                          "    Smooth,                  !- Roughness",
                          "    1.5000000E-03,           !- Thickness {m}",
                          "    44.96960,                !- Conductivity {W/m-K}",
                          "    7689.000,                !- Density {kg/m3}",
                          "    418.0000,                !- Specific Heat {J/kg-K}",
                          "    0.9000000,               !- Thermal Absorptance",
                          "    0.2000000,               !- Solar Absorptance",
                          "    0.2000000;               !- Visible Absorptance",

                          "  Material:NoMass,",
                          "    AR02,                    !- Name",
                          "    VeryRough,               !- Roughness",
                          "    7.8000002E-02,           !- Thermal Resistance {m2-K/W}",
                          "    0.9000000,               !- Thermal Absorptance",
                          "    0.7000000,               !- Solar Absorptance",
                          "    0.7000000;               !- Visible Absorptance",

                          "  Material:NoMass,",
                          "    CP02,                    !- Name",
                          "    Rough,                   !- Roughness",
                          "    0.2170000,               !- Thermal Resistance {m2-K/W}",
                          "    0.9000000,               !- Thermal Absorptance",
                          "    0.7500000,               !- Solar Absorptance",
                          "    0.7500000;               !- Visible Absorptance",

                          "  Construction,",
                          "    EXTWALL:LIVING,          !- Name",
                          "    A1 - 1 IN STUCCO,        !- Outside Layer",
                          "    GP01;                    !- Layer 3",

                          "  Construction,",
                          "    FLOOR:LIVING,            !- Name",
                          "    CC03,                    !- Outside Layer",
                          "    CP02;                    !- Layer 2",

                          "  Construction,",
                          "    ROOF,                    !- Name",
                          "    AR02,                    !- Outside Layer",
                          "    PW03;                    !- Layer 2",

                          "  Zone,",
                          "    LIVING ZONE,             !- Name",
                          "    0,                       !- Direction of Relative North {deg}",
                          "    0,                       !- X Origin {m}",
                          "    0,                       !- Y Origin {m}",
                          "    0,                       !- Z Origin {m}",
                          "    1,                       !- Type",
                          "    1,                       !- Multiplier",
                          "    autocalculate,           !- Ceiling Height {m}",
                          "    autocalculate;           !- Volume {m3}",

                          "  GlobalGeometryRules,",
                          "    UpperLeftCorner,         !- Starting Vertex Position",
                          "    CounterClockWise,        !- Vertex Entry Direction",
                          "    World;                   !- Coordinate System",

                          "  BuildingSurface:Detailed,",
                          "    Living:North,            !- Name",
                          "    Wall,                    !- Surface Type",
                          "    EXTWALL:LIVING,          !- Construction Name",
                          "    LIVING ZONE,             !- Zone Name",
                          "    ,                        !- Space Name",
                          "    Outdoors,                !- Outside Boundary Condition",
                          "    ,                        !- Outside Boundary Condition Object",
                          "    SunExposed,              !- Sun Exposure",
                          "    WindExposed,             !- Wind Exposure",
                          "    0.5000000,               !- View Factor to Ground",
                          "    4,                       !- Number of Vertices",
                          "    1,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
                          "    1,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
                          "    0,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
                          "    0,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

                          "  BuildingSurface:Detailed,",
                          "    Living:East,             !- Name",
                          "    Wall,                    !- Surface Type",
                          "    EXTWALL:LIVING,          !- Construction Name",
                          "    LIVING ZONE,             !- Zone Name",
                          "    ,                        !- Space Name",
                          "    Outdoors,                !- Outside Boundary Condition",
                          "    ,                        !- Outside Boundary Condition Object",
                          "    SunExposed,              !- Sun Exposure",
                          "    WindExposed,             !- Wind Exposure",
                          "    0.5000000,               !- View Factor to Ground",
                          "    4,                       !- Number of Vertices",
                          "    1,0,1,  !- X,Y,Z ==> Vertex 1 {m}",
                          "    1,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
                          "    1,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
                          "    1,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

                          "  BuildingSurface:Detailed,",
                          "    Living:South,            !- Name",
                          "    Wall,                    !- Surface Type",
                          "    EXTWALL:LIVING,          !- Construction Name",
                          "    LIVING ZONE,             !- Zone Name",
                          "    ,                        !- Space Name",
                          "    Outdoors,                !- Outside Boundary Condition",
                          "    ,                        !- Outside Boundary Condition Object",
                          "    SunExposed,              !- Sun Exposure",
                          "    WindExposed,             !- Wind Exposure",
                          "    0.5000000,               !- View Factor to Ground",
                          "    4,                       !- Number of Vertices",
                          "    0,0,1,  !- X,Y,Z ==> Vertex 1 {m}",
                          "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
                          "    1,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
                          "    1,0,1;  !- X,Y,Z ==> Vertex 4 {m}",

                          "  BuildingSurface:Detailed,",
                          "    Living:West,             !- Name",
                          "    Wall,                    !- Surface Type",
                          "    EXTWALL:LIVING,          !- Construction Name",
                          "    LIVING ZONE,             !- Zone Name",
                          "    ,                        !- Space Name",
                          "    Outdoors,                !- Outside Boundary Condition",
                          "    ,                        !- Outside Boundary Condition Object",
                          "    SunExposed,              !- Sun Exposure",
                          "    WindExposed,             !- Wind Exposure",
                          "    0.5000000,               !- View Factor to Ground",
                          "    4,                       !- Number of Vertices",
                          "    0,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
                          "    0,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
                          "    0,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
                          "    0,0,1;  !- X,Y,Z ==> Vertex 4 {m}",
                          "  BuildingSurface:Detailed,",
                          "    Living:Floor,            !- Name",
                          "    FLOOR,                   !- Surface Type",
                          "    FLOOR:LIVING,            !- Construction Name",
                          "    LIVING ZONE,             !- Zone Name",
                          "    ,                        !- Space Name",
                          "    Surface,                 !- Outside Boundary Condition",
                          "    Living:Floor,            !- Outside Boundary Condition Object",
                          "    NoSun,                   !- Sun Exposure",
                          "    NoWind,                  !- Wind Exposure",
                          "    0,                       !- View Factor to Ground",
                          "    4,                       !- Number of Vertices",
                          "    0,0,0,  !- X,Y,Z ==> Vertex 1 {m}",
                          "    0,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
                          "    1,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
                          "    1,0,0;  !- X,Y,Z ==> Vertex 4 {m}",

                          "  BuildingSurface:Detailed,",
                          "    Living:Ceiling,          !- Name",
                          "    ROOF,                 !- Surface Type",
                          "    ROOF,          !- Construction Name",
                          "    LIVING ZONE,             !- Zone Name",
                          "    ,                        !- Space Name",
                          "    Outdoors,                !- Outside Boundary Condition",
                          "    ,                        !- Outside Boundary Condition Object",
                          "    SunExposed,              !- Sun Exposure",
                          "    WindExposed,             !- Wind Exposure",
                          "    0,                       !- View Factor to Ground",
                          "    4,                       !- Number of Vertices",
                          "    0,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
                          "    0,0,1,  !- X,Y,Z ==> Vertex 2 {m}",
                          "    1,0,1,  !- X,Y,Z ==> Vertex 3 {m}",
                          "    1,1,1;  !- X,Y,Z ==> Vertex 4 {m}",
                          "",
                          "  Schedule:Compact,",
                          "    Sche_Q_Evap_Cool,        !- Name",
                          "	 Any Number,              !- Schedule Type Limits Name",
                          "    Through: 12/31,",
                          "    For: AllDays,",
                          "    Until: 24:00, -0.1;",
                          "",
                          "  Schedule:Compact,",
                          "    Sche_Q_Add_Heat,         !- Name",
                          "	 Any Number,              !- Schedule Type Limits Name",
                          "    Through: 12/31,",
                          "    For: AllDays,",
                          "    Until: 24:00, 0.1;",
                          "",
                          "  SurfaceProperty:HeatBalanceSourceTerm,",
                          "    Living:North,               !- Surface Name",
                          "	 ,                           !- Inside Face Heat Source Term Schedule Name",
                          "    Sche_Q_Evap_Cool;           !- Outside Face Heat Source Term Schedule Name",
                          "",
                          "  SurfaceProperty:HeatBalanceSourceTerm,",
                          "    Living:Ceiling,             !- Surface Name",
                          "	 Sche_Q_Add_Heat,            !- Inside Face Heat Source Term Schedule Name",
                          "    ;                           !- Outside Face Heat Source Term Schedule Name"});

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    bool ErrorsFound = false;

    HeatBalanceManager::GetProjectControlData(*state, ErrorsFound); // read project control data
    EXPECT_FALSE(ErrorsFound);                                      // expect no errors
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    Material::GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    SurfaceGeometry::GetGeometryParameters(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    state->dataSurfaceGeometry->CosBldgRotAppGonly = 1.0;
    state->dataSurfaceGeometry->SinBldgRotAppGonly = 0.0;
    SurfaceGeometry::SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    // Clear schedule type warnings
    EXPECT_TRUE(has_err_output(true));

    HeatBalanceIntRadExchange::InitSolarViewFactors(*state);
    EXPECT_TRUE(compare_err_stream(""));
    EXPECT_FALSE(has_err_output(true));

    state->dataZoneEquip->ZoneEquipConfig.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneName = "LIVING ZONE";
    std::vector<int> controlledZoneEquipConfigNums;
    controlledZoneEquipConfigNums.push_back(1);
    state->dataHeatBal->Zone(1).IsControlled = true;
    state->dataZoneEquip->ZoneEquipConfig(1).NumInletNodes = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode.allocate(2);
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(1) = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(2) = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1) = 3;
    state->dataZoneEquip->ZoneEquipConfig(1).NumReturnNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode(1) = 4;
    state->dataZoneEquip->ZoneEquipConfig(1).FixedReturnFlow.allocate(1);

    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataHeatBal->Zone(1).SystemZoneNodeNumber = 5;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->spaceHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 24.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.001;
    state->dataZoneTempPredictorCorrector->spaceHeatBalance(1).MAT = 24.0;
    state->dataZoneTempPredictorCorrector->spaceHeatBalance(1).airHumRat = 0.001;

    state->dataLoopNodes->Node.allocate(4);

    state->dataHeatBalSurf->SurfTempInTmp.allocate(6);
    state->dataHeatBalSurf->SurfTempInTmp(1) = 15.0;
    state->dataHeatBalSurf->SurfTempInTmp(2) = 20.0;
    state->dataHeatBalSurf->SurfTempInTmp(3) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(4) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(5) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(6) = 25.0;

    // allocate surface level adj ratio data member
    state->dataHeatBalSurf->SurfWinCoeffAdjRatio.dimension(6, 1.0);

    state->dataLoopNodes->Node(1).Temp = 20.0;
    state->dataLoopNodes->Node(2).Temp = 20.0;
    state->dataLoopNodes->Node(3).Temp = 20.0;
    state->dataLoopNodes->Node(4).Temp = 20.0;
    state->dataLoopNodes->Node(1).MassFlowRate = 0.1;
    state->dataLoopNodes->Node(2).MassFlowRate = 0.1;
    state->dataLoopNodes->Node(3).MassFlowRate = 0.1;
    state->dataLoopNodes->Node(4).MassFlowRate = 0.1;

    state->dataHeatBalSurf->SurfHConvInt.allocate(6);
    state->dataHeatBalSurf->SurfHConvInt(1) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(2) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(3) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(4) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(5) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(6) = 0.5;
    state->dataMstBal->HConvInFD.allocate(6);
    state->dataMstBal->RhoVaporAirIn.allocate(6);
    state->dataMstBal->HMassConvInFD.allocate(6);

    state->dataGlobal->BeginSimFlag = true;
    state->dataGlobal->KickOffSimulation = true;
    state->dataGlobal->TimeStepZoneSec = 900;
    state->dataHeatBal->ZoneWinHeatGain.allocate(1);
    state->dataHeatBal->ZoneWinHeatGainRep.allocate(1);
    state->dataHeatBal->ZoneWinHeatGainRepEnergy.allocate(1);

    Sched::GetSchedule(*state, "SCHE_Q_EVAP_COOL")->currentVal = -0.1;
    Sched::GetSchedule(*state, "SCHE_Q_ADD_HEAT")->currentVal = 0.1;

    state->dataHeatBalSurf->SurfWinCoeffAdjRatio.dimension(6, 1.0);
    AllocateSurfaceHeatBalArrays(*state);
    createFacilityElectricPowerServiceObject(*state);
    HeatBalanceManager::AllocateZoneHeatBalArrays(*state);
    SolarShading::AllocateModuleArrays(*state);
    SolarShading::DetermineShadowingCombinations(*state);
    for (int loop = 1; loop <= state->dataSurface->TotSurfaces; ++loop) {
        state->dataHeatBalSurf->SurfOutsideTempHist(1)(loop) = 20.0;
    }
    state->dataSurface->SurfTAirRef(1) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    state->dataSurface->SurfTAirRef(2) = DataSurfaces::RefAirTemp::AdjacentAirTemp;
    state->dataSurface->SurfTAirRef(3) = DataSurfaces::RefAirTemp::ZoneSupplyAirTemp;

    InitSurfaceHeatBalance(*state);
    for (int SurfNum = 1; SurfNum <= state->dataSurface->TotSurfaces; SurfNum++) {
        state->dataSurface->surfExtConv(SurfNum).model = Convect::HcExt::ASHRAESimple;
    }

    // Test Additional Heat Source Calculation
    CalcHeatBalanceOutsideSurf(*state);
    EXPECT_EQ(-0.1, state->dataHeatBalSurf->SurfQAdditionalHeatSourceOutside(1));
    CalcHeatBalanceInsideSurf(*state);
    EXPECT_EQ(0.1, state->dataHeatBalSurf->SurfQAdditionalHeatSourceInside(6));
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_TestReportIntMovInsInsideSurfTemp)
{

    Real64 ExpectedResult1;
    Real64 ExpectedResult2;

    state->dataSurface->TotSurfaces = 2;
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);
    state->dataHeatBalSurf->SurfTempIn.allocate(state->dataSurface->TotSurfaces);
    state->dataHeatBalSurf->SurfTempInTmp.allocate(state->dataSurface->TotSurfaces);
    state->dataHeatBalSurf->SurfTempInMovInsRep.allocate(state->dataSurface->TotSurfaces);
    state->dataSurface->intMovInsuls.allocate(state->dataSurface->TotSurfaces);
    state->dataSurface->AnyMovableInsulation = true;
    state->dataSurface->intMovInsulSurfNums.push_back(1);
    state->dataSurface->intMovInsulSurfNums.push_back(2);
    // Test 1 Data: Surface does NOT have movable insulation
    state->dataSurface->intMovInsuls(1).present = false; // No movable insulation
    state->dataHeatBalSurf->SurfTempIn(1) = 23.0;
    state->dataHeatBalSurf->SurfTempInTmp(1) = 12.3;
    state->dataHeatBalSurf->SurfTempInMovInsRep(1) = 1.23;
    ExpectedResult1 = 23.0; // SurfTempInMovInsRep should be set to SurfTempIn

    // Test 2 Data: Surface has movable insulation
    state->dataSurface->intMovInsuls(2).present = true;
    state->dataHeatBalSurf->SurfTempIn(2) = 123.0;
    state->dataHeatBalSurf->SurfTempInTmp(2) = 12.3;
    state->dataHeatBalSurf->SurfTempInMovInsRep(2) = 1.23;
    ExpectedResult2 = 12.3; // SurfTempInMovInsRep should be set to SurfTempIn

    // Now call the subroutine which will run all of the test cases at once and then make the comparisons
    HeatBalanceSurfaceManager::ReportIntMovInsInsideSurfTemp(*state);
    EXPECT_NEAR(state->dataHeatBalSurf->SurfTempInMovInsRep(1), ExpectedResult1, 0.00001);
    EXPECT_NEAR(state->dataHeatBalSurf->SurfTempInMovInsRep(2), ExpectedResult2, 0.00001);
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_OutsideSurfHeatBalanceWhenRainFlag)
{
    state->dataSurface->Surface.allocate(1);
    state->dataSurface->SurfOutWetBulbTemp.allocate(1);
    state->dataSurface->SurfOutDryBulbTemp.allocate(1);
    state->dataHeatBalSurf->SurfHConvExt.allocate(1);
    state->dataHeatBalSurf->SurfOutsideTempHist.allocate(1);
    state->dataHeatBalSurf->SurfOutsideTempHist(1).allocate(1);

    state->dataSurface->Surface(1).Area = 58.197;
    state->dataHeatBalSurf->SurfHConvExt(1) = 1000;
    state->dataHeatBalSurf->SurfOutsideTempHist(1)(1) = 6.71793958923051;
    state->dataSurface->SurfOutWetBulbTemp(1) = 6.66143784594778;
    state->dataSurface->SurfOutDryBulbTemp(1) = 7.2;

    // If Rain Flag = on, GetSurfQdotConvOutRep uses Outdoor Air Wet Bulb Temp.
    state->dataEnvrn->IsRain = true;
    Real64 ExpectedQconvPerArea1 = -1000 * (6.71793958923051 - 6.66143784594778);

    EXPECT_NEAR(ExpectedQconvPerArea1, GetQdotConvOutPerArea(*state, 1), 0.01);

    // Otherwise, GetSurfQdotConvOutRep uses Outdoor Air Dry Bulb Temp.
    state->dataEnvrn->IsRain = false;
    state->dataHeatBalSurf->SurfHConvExt(1) = 5.65361106051348;
    Real64 ExpectedQconvPerArea2 = -5.65361106051348 * (6.71793958923051 - 7.2);

    EXPECT_NEAR(ExpectedQconvPerArea2, GetQdotConvOutPerArea(*state, 1), 0.01);
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_TestInterzoneRadFactorCalc)
{

    state->dataSurface->TotSurfaces = 2;
    state->dataGlobal->NumOfZones = 2;
    state->dataHeatBal->TotConstructs = 1;
    state->dataViewFactor->NumOfSolarEnclosures = 3;

    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);
    state->dataConstruction->Construct.allocate(state->dataHeatBal->TotConstructs);
    state->dataViewFactor->EnclSolInfo.allocate(state->dataViewFactor->NumOfSolarEnclosures);
    state->dataConstruction->Construct(1).TransDiff = 0.1;
    state->dataViewFactor->EnclSolInfo(1).solVMULT = 1.0;
    state->dataViewFactor->EnclSolInfo(2).solVMULT = 1.0;
    state->dataViewFactor->EnclSolInfo(3).solVMULT = 1.0;

    state->dataSurface->Surface(1).HeatTransSurf = true;
    state->dataSurface->Surface(1).Construction = 1;
    state->dataSurface->Surface(1).ExtBoundCond = 2;
    state->dataSurface->Surface(1).Area = 1.0;
    state->dataSurface->Surface(1).Zone = 1;
    state->dataSurface->Surface(1).Class = DataSurfaces::SurfaceClass::Window;
    state->dataSurface->AllHTWindowSurfaceList.push_back(1);

    state->dataSurface->Surface(2).HeatTransSurf = true;
    state->dataSurface->Surface(2).Construction = 1;
    state->dataSurface->Surface(2).ExtBoundCond = 1;
    state->dataSurface->Surface(2).Area = 1.0;
    state->dataSurface->Surface(2).Zone = 2;
    state->dataSurface->Surface(2).Class = DataSurfaces::SurfaceClass::Window;
    state->dataSurface->AllHTWindowSurfaceList.push_back(2);

    state->dataSurface->Surface(1).SolarEnclIndex = 1;
    state->dataSurface->Surface(2).SolarEnclIndex = 2;

    ComputeDifSolExcZonesWIZWindows(*state);

    EXPECT_EQ(1, state->dataHeatBalSurf->ZoneFractDifShortZtoZ(1, 1));
    EXPECT_EQ(1, state->dataHeatBalSurf->ZoneFractDifShortZtoZ(2, 2));
    EXPECT_EQ(1, state->dataHeatBalSurf->ZoneFractDifShortZtoZ(3, 3));
    EXPECT_FALSE(state->dataHeatBalSurf->EnclSolRecDifShortFromZ(1));
    EXPECT_FALSE(state->dataHeatBalSurf->EnclSolRecDifShortFromZ(2));
    EXPECT_FALSE(state->dataHeatBalSurf->EnclSolRecDifShortFromZ(3));

    state->dataViewFactor->EnclSolInfo(1).HasInterZoneWindow = true;
    state->dataViewFactor->EnclSolInfo(2).HasInterZoneWindow = true;
    state->dataViewFactor->EnclSolInfo(3).HasInterZoneWindow = false;

    ComputeDifSolExcZonesWIZWindows(*state);

    EXPECT_TRUE(state->dataHeatBalSurf->EnclSolRecDifShortFromZ(1));
    EXPECT_TRUE(state->dataHeatBalSurf->EnclSolRecDifShortFromZ(2));
    EXPECT_FALSE(state->dataHeatBalSurf->EnclSolRecDifShortFromZ(3));

    state->dataGlobal->KickOffSimulation = true;
    ComputeDifSolExcZonesWIZWindows(*state);

    EXPECT_EQ(1, state->dataHeatBalSurf->ZoneFractDifShortZtoZ(1, 1));
    EXPECT_EQ(1, state->dataHeatBalSurf->ZoneFractDifShortZtoZ(2, 2));
    EXPECT_EQ(1, state->dataHeatBalSurf->ZoneFractDifShortZtoZ(3, 3));
    EXPECT_FALSE(state->dataHeatBalSurf->EnclSolRecDifShortFromZ(1));
    EXPECT_FALSE(state->dataHeatBalSurf->EnclSolRecDifShortFromZ(2));
    EXPECT_FALSE(state->dataHeatBalSurf->EnclSolRecDifShortFromZ(3));
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_TestResilienceMetricReport)
{
    state->dataGlobal->NumOfZones = 1;
    state->dataGlobal->KindOfSim = Constant::KindOfSim::RunPeriodWeather;
    state->dataOutRptTab->displayThermalResilienceSummary = true;
    state->dataEnvrn->Month = 7;
    state->dataEnvrn->DayOfMonth = 1;
    state->dataGlobal->TimeStepsInHour = 4;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 15; // must initialize this to get schedules initialized

    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->TimeStepZone = 1;
    state->dataEnvrn->OutBaroPress = 101325.0;

    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->Resilience.allocate(state->dataGlobal->NumOfZones);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(state->dataGlobal->NumOfZones);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZTAV = 0.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRatAvg = 0.0;

    state->dataHeatBalFanSys->zoneTstatSetpts.allocate(state->dataGlobal->NumOfZones);
    for (auto &zoneTstatSetpt : state->dataHeatBalFanSys->zoneTstatSetpts) {
        zoneTstatSetpt.setptLo = 22.0;
        zoneTstatSetpt.setptHi = 28.0;
    }

    state->dataHeatBal->TotPeople = 1;
    state->dataHeatBal->People.allocate(state->dataHeatBal->TotPeople);
    state->dataHeatBal->People(1).ZonePtr = 1;
    state->dataHeatBal->People(1).Pierce = true;
    state->dataHeatBal->People(1).NumberOfPeople = 2;
    state->dataHeatBal->People(1).sched = Sched::AddScheduleConstant(*state, "Occupancy");
    state->dataHeatBal->People(1).ColdStressTempThresh = 15.0;
    state->dataHeatBal->People(1).HeatStressTempThresh = 29.5;
    state->dataHeatBalFanSys->zoneTstatSetpts(1).setptHi = 27.5;
    state->dataHeatBalFanSys->zoneTstatSetpts(1).setptLo = 15.0;

    state->dataThermalComforts->ThermalComfortData.allocate(state->dataHeatBal->TotPeople);

    state->dataThermalComforts->ThermalComfortData(1).PierceSET = 31;
    state->dataHeatBal->People(1).sched->currentVal = 0;

    // Heat Index Case 1: Zone T < 80 F;
    state->dataGlobal->HourOfDay = 1;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZTAV = 25;
    state->dataThermalComforts->ThermalComfortData(1).FangerPMV = -4.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRatAvg = 0.00988; // RH = 50%
    CalcThermalResilience(*state);
    ReportThermalResilience(*state);
    EXPECT_NEAR(25, state->dataHeatBal->Resilience(1).ZoneHeatIndex, 0.5);
    EXPECT_NEAR(28, state->dataHeatBal->Resilience(1).ZoneHumidex, 1);

    // Heat Index Case 2: Zone RH > 85, 80 < T < 87 F;
    state->dataGlobal->HourOfDay = 2;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZTAV = 27;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRatAvg = 0.02035; // RH = 90%
    CalcThermalResilience(*state);
    ReportThermalResilience(*state);
    EXPECT_NEAR(31, state->dataHeatBal->Resilience(1).ZoneHeatIndex, 0.5);
    EXPECT_NEAR(39, state->dataHeatBal->Resilience(1).ZoneHumidex, 1);

    // Heat Index Case 3: < Zone RH > 85, 80 < T < 87 F;
    state->dataGlobal->HourOfDay = 3;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZTAV = 27;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRatAvg = 0.0022; // RH = 10%
    CalcThermalResilience(*state);
    ReportThermalResilience(*state);
    EXPECT_NEAR(26, state->dataHeatBal->Resilience(1).ZoneHeatIndex, 0.5);
    EXPECT_NEAR(23, state->dataHeatBal->Resilience(1).ZoneHumidex, 1);

    // Heat Index Case 4: Rothfusz regression, other than the above conditions;
    state->dataGlobal->HourOfDay = 4;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZTAV = 30;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRatAvg = 0.01604; // RH = 60%
    CalcThermalResilience(*state);
    ReportThermalResilience(*state);
    EXPECT_NEAR(33, state->dataHeatBal->Resilience(1).ZoneHeatIndex, 0.5);
    EXPECT_NEAR(38, state->dataHeatBal->Resilience(1).ZoneHumidex, 1);

    // Test categorization of the first 4 hours.
    EXPECT_NEAR(2.0, state->dataHeatBal->Resilience(1).ZoneHeatIndexHourBins[0], 1e-8); // Safe: Heat Index <= 80 °F (26.7 °C).
    EXPECT_NEAR(1.0, state->dataHeatBal->Resilience(1).ZoneHeatIndexHourBins[1], 1e-8); // Caution: (80, 90 °F] / (26.7, 32.2 °C]
    EXPECT_NEAR(1.0, state->dataHeatBal->Resilience(1).ZoneHeatIndexHourBins[2], 1e-8); // Extreme Caution (90, 105 °F] / (32.2, 40.6 °C]
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneHeatIndexHourBins[3], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneHeatIndexOccuHourBins[0], 1e-8);     // # of People = 0
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneHeatIndexOccupiedHourBins[0], 1e-8); // # of People = 0

    EXPECT_NEAR(2.0, state->dataHeatBal->Resilience(1).ZoneHumidexHourBins[0], 1e-8);         // Humidex <= 29
    EXPECT_NEAR(2.0, state->dataHeatBal->Resilience(1).ZoneHumidexHourBins[1], 1e-8);         // Humidex (29, 40]
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneHumidexOccuHourBins[0], 1e-8);     // # of People = 0
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneHumidexOccupiedHourBins[0], 1e-8); // # of People = 0

    // SET Degree-Hr Test values
    //    hour  PierceSET  OccuSchedule  TotalHighSet	HiSetOccuHour	HiSetOccupied	TotalLowSet	LowOccuHour Low Occupied
    //    1     31         0	         1	        0	        0	        0	        0	    0
    //    2     31         0	         2	        0	        0	        0	        0	    0
    //    3     31         0	         3	        0	        0	        0	        0	    0
    //    4     31         0	         4	        0	        0	        0	        0	    0
    //    5     11.2       0.4           4	        0	        0	        1	        0.8	    1
    //    6     11.2       0.4           4	        0	        0	        2	        1.6	    2
    //    7     11.2       0.4           4	        0	        0	        3	        2.4	    3
    //    8     32         1	         6	        4	        2	        3	        2.4	    3
    //    9     32         1	         8	        8	        4	        3	        2.4	    3
    //    10    32         1	         10	        12	        6	        3	        2.4	    3
    //    11    25         1	         10	        12	        6	        3	        2.4	    3
    //    12    25         1	         10	        12	        6	        3	        2.4	    3
    //    13    11.2       0.4	         10	        12	        6	        4	        3.2	    4
    //    14    11.2       0.4	         10	        12	        6	        5	        4	    5
    //    15    11.2       0.4	         10	        12	        6	        6	        4.8	    6
    //    16    11.2       0.4	         10	        12	        6	        7	        5.6	    7
    //    17    11.2       0.4	         10	        12	        6	        8	        6.4	    8
    //    18    11.2       0.4	         10	        12	        6	        9	        7.2	    9
    //    19    11.2       0	         10	        12	        6	        10	        7.2	    9
    //    20    11.2       0	         10	        12	        6	        11	        7.2	    9
    // Cooling SET Degree-Hours
    EXPECT_NEAR(4.0, state->dataHeatBal->Resilience(1).ZoneHighSETHours[0], 1e-8); // SET Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneHighSETHours[1], 1e-8); // SET OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneHighSETHours[2], 1e-8); // SET OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneHighSETHours[3], 1e-8); // Longest SET > 30°C Duration [hr]
    // Heating SET Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneLowSETHours[0], 1e-8); // SET Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneLowSETHours[1], 1e-8); // SET OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneLowSETHours[2], 1e-8); // SET OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneLowSETHours[3], 1e-8); // Longest SET ≤ 12.2°C Duration [hr]

    // Hour of safety table data
    //                  Cold                                          Heat
    //    hour	ZTAV	HOS exceed-hr exceed-occuhour exceed occupied HOS exceed-hr exceed-occuhour exceed-occupied
    //    1	25	1	0	 0	      0	              1	    0	     0	                  0
    //    2	27	2	0	 0	      0	              2	    0	     0	                  0
    //    3	27	3	0	 0	      0	              3	    0	     0	                  0
    //    4	30	4	0	 0	      0	              3	    1	     0	                  0
    //    5	31	5	0	 0	      0	              3	    2	     0.8	          1
    //    6	31	6	0	 0	      0	              3	    3	     1.6	          2
    //    7	31	7	0	 0	      0	              3	    4	     2.4	          3
    //    8	28	8	0	 0	      0	              3	    4	     2.4	          3
    //    9	28	9	0	 0	      0	              3	    4	     2.4	          3
    //    10	28	10	0	 0	      0	              3	    4	     2.4	          3
    //    11	31	11	0	 0	      0	              3	    5	     4.4	          4
    //    12	31	12	0	 0	      0	              3	    6	     6.4	          5
    //    13	30	13	0	 0	      0	              3	    7	     7.2	          6
    //    14	30	14	0	 0	      0	              3	    8	     8	                  7
    //    15	30	15	0	 0	      0	              3	    9	     8.8	          8
    //    16	30	16	0	 0	      0	              3	    10	     9.6	          9
    //    17	30	17	0	 0	      0	              3	    11	     10.4	          10
    //    18	30	18	0	 0	      0	              3	    12	     11.2	          11
    //    19	12	18	1	 0	      0	              3	    12	     11.2	          11
    //    20	12	18	2	 0	      0	              3	    12	     11.2	          11

    // Hours of Safety for Cold Events
    EXPECT_NEAR(4.0, state->dataHeatBal->Resilience(1).ZoneColdHourOfSafetyBins[0], 1e-8); // Hours of safety
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneColdHourOfSafetyBins[2], 1e-8); // Safe Temperature Exceedance Hours [hr]
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneColdHourOfSafetyBins[3], 1e-8); // Safe Temperature Exceedance OccupantHours [hr]
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneColdHourOfSafetyBins[4], 1e-8); // Safe Temperature Exceedance OccupiedHours [hr]
    // Hours of Safety for Heat Events
    EXPECT_NEAR(3.0, state->dataHeatBal->Resilience(1).ZoneHeatHourOfSafetyBins[0], 1e-8); // Hours of safety
    EXPECT_NEAR(1.0, state->dataHeatBal->Resilience(1).ZoneHeatHourOfSafetyBins[2], 1e-8); // Safe Temperature Exceedance Hours [hr]
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneHeatHourOfSafetyBins[3], 1e-8); // Safe Temperature Exceedance OccupantHours [hr]
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneHeatHourOfSafetyBins[4], 1e-8); // Safe Temperature Exceedance OccupiedHours [hr]

    //    Unmet Degree Hour table data
    //    cooling setpoint	27.5
    //    heating setpoint	15
    //    number of people	2
    //
    //                              Cooling unmet                         Heating unmet
    //    hour  OccuSche  ZTAV      deg-hr  deg-occu-hr  deg-occupied-hr  deg-hr deg-occu-hr deg-occupied-hr
    //    1	    0	      25	0	0	     0	              0	     0	         0
    //    2	    0	      27	0	0	     0	              0	     0	         0
    //    3	    0	      27	0	0	     0	              0	     0	         0
    //    4	    0	      30	2.5	0	     0	              0	     0	         0
    //    5	    0.4	      31	6	2.8	     3.5	      0	     0	         0
    //    6	    0.4	      31	9.5	5.6	     7	              0	     0	         0
    //    7	    0.4	      31	13	8.4	     10.5	      0	     0	         0
    //    8	    1	      28	13.5	9.4	     11	              0	     0	         0
    //    9	    1	      28	14	10.4	     11.5	      0	     0	         0
    //    10    1	      28	14.5	11.4	     12	              0      0	         0
    //    11    1	      31	18	18.4	     15.5	      0	     0	         0
    //    12    1	      31	21.5	25.4	     19	              0	     0	         0
    //    13    0.4	      30	24	27.4	     21.5	      0	     0	         0
    //    14    0.4	      30	26.5	29.4	     24	              0	     0	         0
    //    15    0.4	      30	29	31.4	     26.5	      0	     0	         0
    //    16    0.4	      30	31.5	33.4	     29	              0	     0	         0
    //    17    0.4	      30	34	35.4	     31.5	      0	     0	         0
    //    18    0.4	      30	36.5	37.4	     34	              0	     0	         0
    //    19    0	      12	36.5	37.4	     34	              3	     0	         0
    //    20    0	      12	36.5	37.4	     34	              6	     0	         0

    // Unmet Degree-Hours
    EXPECT_NEAR(2.5, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[0], 1e-8); // Cooling Setpoint Unmet Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[1], 1e-8); // Cooling Setpoint Unmet Occupant-Weighted Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[2], 1e-8); // Cooling Setpoint Unmet Occupied Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[3], 1e-8); // Heating Setpoint Unmet Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[4], 1e-8); // Heating Setpoint Unmet Occupant-Weighted Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[5], 1e-8); // Heating Setpoint Unmet Occupied Degree-Hours

    // Discomfort-weighted Exceedance table data
    //    VeryHotPMVThresh 	3
    //    WarmPMVThresh 	0.7
    //    CoolPMVThresh 	-0.7
    //    VeryColdPMVThresh 	-3
    //    num people	2
    //                          Exceedance Occupant Hour         Exceedance Occupied Hour
    //    hour	OccuSche  PMV	VeryCold cool   warm    VeryHot  VeryCold cool  warm    VeryHot
    //    1	0	 -4	0	 0	0	0	 0	  0	0	0
    //    2	0	-4	0	 0	0	0	 0	  0	0	0
    //    3	0	-4	0	 0	0	0	 0	  0	0	0
    //    4	0	-4	0	 0	0	0	 0	  0	0	0
    //    5	0.4	-3.5	0.4	 2.24	0	0	 0.5	  2.8	0	0
    //    6	0.4	-3.5	0.8	 4.48	0	0	 1	  5.6	0	0
    //    7	0.4	-3.5	1.2	 6.72	0	0	 1.5	  8.4	0	0
    //    8	1	-1.2	1.2	 7.72	0	0	 1.5	  8.9	0	0
    //    9	1	-1.2	1.2	 8.72	0	0	 1.5	  9.4	0	0
    //    10	1	-1.2	1.2	 9.72	0	0	 1.5	  9.9	0	0
    //    11	1	0.5	1.2	 9.72	0	0	 1.5	  9.9	0	0
    //    12	1	0.5	1.2	 9.72	0	0	 1.5	  9.9	0	0
    //    13	0.4	1.2	1.2	 9.72	0.4	0	 1.5	  9.9	0.5	0
    //    14	0.4	1.2	1.2	 9.72	0.8	0	 1.5	  9.9	1	0
    //    15	0.4	1.2	1.2	 9.72	1.2	0	 1.5	  9.9	1.5	0
    //    16	0.4	1.2	1.2	 9.72	1.6	0	 1.5	  9.9	2	0
    //    17	0.4	1.2	1.2	 9.72	2	0	 1.5	  9.9	2.5	0
    //    18	0.4	1.2	1.2	 9.72	2.4	0	 1.5	  9.9	3	0
    //    19	0	1.2	1.2	 9.72	2.4	0	 1.5	  9.9	3	0
    //    20	0	1.2	1.2	 9.72	2.4	0	 1.5	  9.9	3	0

    // Discomfort-weighted Exceedance OccupantHours and OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccuHourBins[0], 1e-8);     // Very-cold Exceedance OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccuHourBins[1], 1e-8);     // Cool Exceedance OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccuHourBins[2], 1e-8);     // Warm Exceedance OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccuHourBins[3], 1e-8);     // Very-hot Exceedance OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccupiedHourBins[0], 1e-8); // Very-cold Exceedance OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccupiedHourBins[1], 1e-8); // Cool Exceedance OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccupiedHourBins[2], 1e-8); // Warm Exceedance OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccupiedHourBins[3], 1e-8); // Very-hot Exceedance OccupiedHours

    state->dataThermalComforts->ThermalComfortData(1).PierceSET = 11.2;
    state->dataHeatBal->People(1).sched->currentVal = 0.4;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZTAV = 31;
    state->dataThermalComforts->ThermalComfortData(1).FangerPMV = -3.5;
    for (int hour = 5; hour <= 7; hour++) {
        state->dataGlobal->HourOfDay = hour;
        //        CalcThermalResilience(*state);
        ReportThermalResilience(*state);
    }
    // Test SET-hours calculation - Heating unmet
    // Cooling SET Degree-Hours
    EXPECT_NEAR(4.0, state->dataHeatBal->Resilience(1).ZoneHighSETHours[0], 1e-8); // SET Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneHighSETHours[1], 1e-8); // SET OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneHighSETHours[2], 1e-8); // SET OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneHighSETHours[3], 1e-8); // Longest SET > 30°C Duration [hr]
    // Heating SET Degree-Hours
    EXPECT_NEAR(3.0, state->dataHeatBal->Resilience(1).ZoneLowSETHours[0], 1e-8); // SET Degree-Hours
    EXPECT_NEAR(2.4, state->dataHeatBal->Resilience(1).ZoneLowSETHours[1], 1e-8); // SET OccupantHours
    EXPECT_NEAR(3.0, state->dataHeatBal->Resilience(1).ZoneLowSETHours[2], 1e-8); // SET OccupiedHours
    EXPECT_NEAR(3.0, state->dataHeatBal->Resilience(1).ZoneLowSETHours[3], 1e-8); // Longest SET ≤ 12.2°C Duration [hr]

    // Hours of Safety for Cold Events
    EXPECT_NEAR(7.0, state->dataHeatBal->Resilience(1).ZoneColdHourOfSafetyBins[0], 1e-8); // Hours of safety
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneColdHourOfSafetyBins[2], 1e-8); // Safe Temperature Exceedance Hours [hr]
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneColdHourOfSafetyBins[3], 1e-8); // Safe Temperature Exceedance OccupantHours [hr]
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneColdHourOfSafetyBins[4], 1e-8); // Safe Temperature Exceedance OccupiedHours [hr]
    // Hours of Safety for Heat Events
    EXPECT_NEAR(3.0, state->dataHeatBal->Resilience(1).ZoneHeatHourOfSafetyBins[0], 1e-8); // Hours of safety
    EXPECT_NEAR(4.0, state->dataHeatBal->Resilience(1).ZoneHeatHourOfSafetyBins[2], 1e-8); // Safe Temperature Exceedance Hours [hr]
    EXPECT_NEAR(2.4, state->dataHeatBal->Resilience(1).ZoneHeatHourOfSafetyBins[3], 1e-8); // Safe Temperature Exceedance OccupantHours [hr]
    EXPECT_NEAR(3.0, state->dataHeatBal->Resilience(1).ZoneHeatHourOfSafetyBins[4], 1e-8); // Safe Temperature Exceedance OccupiedHours [hr]

    // Unmet Degree-Hours
    EXPECT_NEAR(13.0, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[0], 1e-8); // Cooling Setpoint Unmet Degree-Hours
    EXPECT_NEAR(8.4, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[1], 1e-8);  // Cooling Setpoint Unmet Occupant-Weighted Degree-Hours
    EXPECT_NEAR(10.5, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[2], 1e-8); // Cooling Setpoint Unmet Occupied Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[3], 1e-8);  // Heating Setpoint Unmet Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[4], 1e-8);  // Heating Setpoint Unmet Occupant-Weighted Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[5], 1e-8);  // Heating Setpoint Unmet Occupied Degree-Hours

    // Discomfort-weighted Exceedance OccupantHours and OccupiedHours
    EXPECT_NEAR(1.2, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccuHourBins[0], 1e-8);     // Very-cold Exceedance OccupantHours
    EXPECT_NEAR(6.72, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccuHourBins[1], 1e-8);    // Cool Exceedance OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccuHourBins[2], 1e-8);     // Warm Exceedance OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccuHourBins[3], 1e-8);     // Very-hot Exceedance OccupantHours
    EXPECT_NEAR(1.5, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccupiedHourBins[0], 1e-8); // Very-cold Exceedance OccupiedHours
    EXPECT_NEAR(8.4, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccupiedHourBins[1], 1e-8); // Cool Exceedance OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccupiedHourBins[2], 1e-8); // Warm Exceedance OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccupiedHourBins[3], 1e-8); // Very-hot Exceedance OccupiedHours

    state->dataThermalComforts->ThermalComfortData(1).PierceSET = 32;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZTAV = 28;
    state->dataHeatBal->People(1).sched->currentVal = 1.0;
    state->dataThermalComforts->ThermalComfortData(1).FangerPMV = -1.2;
    for (int hour = 8; hour <= 10; hour++) {
        state->dataGlobal->HourOfDay = hour;
        ReportThermalResilience(*state);
    }
    // Test SET-hours calculation - Cooling unmet
    // Cooling SET Degree-Hours
    EXPECT_NEAR(10.0, state->dataHeatBal->Resilience(1).ZoneHighSETHours[0], 1e-8); // SET Degree-Hours
    EXPECT_NEAR(12.0, state->dataHeatBal->Resilience(1).ZoneHighSETHours[1], 1e-8); // SET OccupantHours
    EXPECT_NEAR(6.0, state->dataHeatBal->Resilience(1).ZoneHighSETHours[2], 1e-8);  // SET OccupiedHours
    EXPECT_NEAR(3.0, state->dataHeatBal->Resilience(1).ZoneHighSETHours[3], 1e-8);  // Longest SET > 30°C Duration [hr]
    // Heating SET Degree-Hours
    EXPECT_NEAR(3.0, state->dataHeatBal->Resilience(1).ZoneLowSETHours[0], 1e-8); // SET Degree-Hours
    EXPECT_NEAR(2.4, state->dataHeatBal->Resilience(1).ZoneLowSETHours[1], 1e-8); // SET OccupantHours
    EXPECT_NEAR(3.0, state->dataHeatBal->Resilience(1).ZoneLowSETHours[2], 1e-8); // SET OccupiedHours
    EXPECT_NEAR(3.0, state->dataHeatBal->Resilience(1).ZoneLowSETHours[3], 1e-8); // Longest SET ≤ 12.2°C Duration [hr]

    // Hours of Safety for Cold Events
    EXPECT_NEAR(10.0, state->dataHeatBal->Resilience(1).ZoneColdHourOfSafetyBins[0], 1e-8); // Hours of safety
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneColdHourOfSafetyBins[2], 1e-8);  // Safe Temperature Exceedance Hours [hr]
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneColdHourOfSafetyBins[3], 1e-8);  // Safe Temperature Exceedance OccupantHours [hr]
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneColdHourOfSafetyBins[4], 1e-8);  // Safe Temperature Exceedance OccupiedHours [hr]
    // Hours of Safety for Heat Events
    EXPECT_NEAR(3.0, state->dataHeatBal->Resilience(1).ZoneHeatHourOfSafetyBins[0], 1e-8); // Hours of safety
    EXPECT_NEAR(4.0, state->dataHeatBal->Resilience(1).ZoneHeatHourOfSafetyBins[2], 1e-8); // Safe Temperature Exceedance Hours [hr]
    EXPECT_NEAR(2.4, state->dataHeatBal->Resilience(1).ZoneHeatHourOfSafetyBins[3], 1e-8); // Safe Temperature Exceedance OccupantHours [hr]
    EXPECT_NEAR(3.0, state->dataHeatBal->Resilience(1).ZoneHeatHourOfSafetyBins[4], 1e-8); // Safe Temperature Exceedance OccupiedHours [hr]

    // Unmet Degree-Hours
    EXPECT_NEAR(14.5, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[0], 1e-8); // Cooling Setpoint Unmet Degree-Hours
    EXPECT_NEAR(11.4, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[1], 1e-8); // Cooling Setpoint Unmet Occupant-Weighted Degree-Hours
    EXPECT_NEAR(12.0, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[2], 1e-8); // Cooling Setpoint Unmet Occupied Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[3], 1e-8);  // Heating Setpoint Unmet Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[4], 1e-8);  // Heating Setpoint Unmet Occupant-Weighted Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[5], 1e-8);  // Heating Setpoint Unmet Occupied Degree-Hours

    // Discomfort-weighted Exceedance OccupantHours and OccupiedHours
    EXPECT_NEAR(1.2, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccuHourBins[0], 1e-8);     // Very-cold Exceedance OccupantHours
    EXPECT_NEAR(9.72, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccuHourBins[1], 1e-8);    // Cool Exceedance OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccuHourBins[2], 1e-8);     // Warm Exceedance OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccuHourBins[3], 1e-8);     // Very-hot Exceedance OccupantHours
    EXPECT_NEAR(1.5, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccupiedHourBins[0], 1e-8); // Very-cold Exceedance OccupiedHours
    EXPECT_NEAR(9.9, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccupiedHourBins[1], 1e-8); // Cool Exceedance OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccupiedHourBins[2], 1e-8); // Warm Exceedance OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccupiedHourBins[3], 1e-8); // Very-hot Exceedance OccupiedHours

    state->dataThermalComforts->ThermalComfortData(1).PierceSET = 25;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZTAV = 31;
    state->dataThermalComforts->ThermalComfortData(1).FangerPMV = 0.5;
    for (int hour = 11; hour <= 12; hour++) {
        state->dataGlobal->HourOfDay = hour;
        ReportThermalResilience(*state);
    }
    // Cooling SET Degree-Hours
    EXPECT_NEAR(10.0, state->dataHeatBal->Resilience(1).ZoneHighSETHours[0], 1e-8); // SET Degree-Hours
    EXPECT_NEAR(12.0, state->dataHeatBal->Resilience(1).ZoneHighSETHours[1], 1e-8); // SET OccupantHours
    EXPECT_NEAR(6.0, state->dataHeatBal->Resilience(1).ZoneHighSETHours[2], 1e-8);  // SET OccupiedHours
    EXPECT_NEAR(3.0, state->dataHeatBal->Resilience(1).ZoneHighSETHours[3], 1e-8);  // Longest SET > 30°C Duration [hr]
    // Heating SET Degree-Hours
    EXPECT_NEAR(3.0, state->dataHeatBal->Resilience(1).ZoneLowSETHours[0], 1e-8); // SET Degree-Hours
    EXPECT_NEAR(2.4, state->dataHeatBal->Resilience(1).ZoneLowSETHours[1], 1e-8); // SET OccupantHours
    EXPECT_NEAR(3.0, state->dataHeatBal->Resilience(1).ZoneLowSETHours[2], 1e-8); // SET OccupiedHours
    EXPECT_NEAR(3.0, state->dataHeatBal->Resilience(1).ZoneLowSETHours[3], 1e-8); // Longest SET ≤ 12.2°C Duration [hr]

    // Hours of Safety for Cold Events
    EXPECT_NEAR(12.0, state->dataHeatBal->Resilience(1).ZoneColdHourOfSafetyBins[0], 1e-8); // Hours of safety
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneColdHourOfSafetyBins[2], 1e-8);  // Safe Temperature Exceedance Hours [hr]
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneColdHourOfSafetyBins[3], 1e-8);  // Safe Temperature Exceedance OccupantHours [hr]
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneColdHourOfSafetyBins[4], 1e-8);  // Safe Temperature Exceedance OccupiedHours [hr]
    // Hours of Safety for Heat Events
    EXPECT_NEAR(3.0, state->dataHeatBal->Resilience(1).ZoneHeatHourOfSafetyBins[0], 1e-8); // Hours of safety
    EXPECT_NEAR(6.0, state->dataHeatBal->Resilience(1).ZoneHeatHourOfSafetyBins[2], 1e-8); // Safe Temperature Exceedance Hours [hr]
    EXPECT_NEAR(6.4, state->dataHeatBal->Resilience(1).ZoneHeatHourOfSafetyBins[3], 1e-8); // Safe Temperature Exceedance OccupantHours [hr]
    EXPECT_NEAR(5.0, state->dataHeatBal->Resilience(1).ZoneHeatHourOfSafetyBins[4], 1e-8); // Safe Temperature Exceedance OccupiedHours [hr]

    // Unmet Degree-Hours
    EXPECT_NEAR(21.5, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[0], 1e-8); // Cooling Setpoint Unmet Degree-Hours
    EXPECT_NEAR(25.4, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[1], 1e-8); // Cooling Setpoint Unmet Occupant-Weighted Degree-Hours
    EXPECT_NEAR(19.0, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[2], 1e-8); // Cooling Setpoint Unmet Occupied Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[3], 1e-8);  // Heating Setpoint Unmet Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[4], 1e-8);  // Heating Setpoint Unmet Occupant-Weighted Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[5], 1e-8);  // Heating Setpoint Unmet Occupied Degree-Hours

    // Discomfort-weighted Exceedance OccupantHours and OccupiedHours
    EXPECT_NEAR(1.2, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccuHourBins[0], 1e-8);     // Very-cold Exceedance OccupantHours
    EXPECT_NEAR(9.72, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccuHourBins[1], 1e-8);    // Cool Exceedance OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccuHourBins[2], 1e-8);     // Warm Exceedance OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccuHourBins[3], 1e-8);     // Very-hot Exceedance OccupantHours
    EXPECT_NEAR(1.5, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccupiedHourBins[0], 1e-8); // Very-cold Exceedance OccupiedHours
    EXPECT_NEAR(9.9, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccupiedHourBins[1], 1e-8); // Cool Exceedance OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccupiedHourBins[2], 1e-8); // Warm Exceedance OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccupiedHourBins[3], 1e-8); // Very-hot Exceedance OccupiedHours

    state->dataThermalComforts->ThermalComfortData(1).PierceSET = 11.2;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZTAV = 30;
    state->dataHeatBal->People(1).sched->currentVal = 0.4;
    state->dataThermalComforts->ThermalComfortData(1).FangerPMV = 1.2;
    for (int hour = 13; hour <= 18; hour++) {
        state->dataGlobal->HourOfDay = hour;
        ReportThermalResilience(*state);
    }

    // Cooling SET Degree-Hours
    EXPECT_NEAR(10.0, state->dataHeatBal->Resilience(1).ZoneHighSETHours[0], 1e-8); // SET Degree-Hours
    EXPECT_NEAR(12.0, state->dataHeatBal->Resilience(1).ZoneHighSETHours[1], 1e-8); // SET OccupantHours
    EXPECT_NEAR(6.0, state->dataHeatBal->Resilience(1).ZoneHighSETHours[2], 1e-8);  // SET OccupiedHours
    EXPECT_NEAR(3.0, state->dataHeatBal->Resilience(1).ZoneHighSETHours[3], 1e-8);  // Longest SET > 30°C Duration [hr]
    // Heating SET Degree-Hours
    EXPECT_NEAR(9.0, state->dataHeatBal->Resilience(1).ZoneLowSETHours[0], 1e-8); // SET Degree-Hours
    EXPECT_NEAR(7.2, state->dataHeatBal->Resilience(1).ZoneLowSETHours[1], 1e-8); // SET OccupantHours
    EXPECT_NEAR(9.0, state->dataHeatBal->Resilience(1).ZoneLowSETHours[2], 1e-8); // SET OccupiedHours
    EXPECT_NEAR(6.0, state->dataHeatBal->Resilience(1).ZoneLowSETHours[3], 1e-8); // Longest SET ≤ 12.2°C Duration [hr]

    // Hours of Safety for Cold Events
    EXPECT_NEAR(18.0, state->dataHeatBal->Resilience(1).ZoneColdHourOfSafetyBins[0], 1e-8); // Hours of safety
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneColdHourOfSafetyBins[2], 1e-8);  // Safe Temperature Exceedance Hours [hr]
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneColdHourOfSafetyBins[3], 1e-8);  // Safe Temperature Exceedance OccupantHours [hr]
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneColdHourOfSafetyBins[4], 1e-8);  // Safe Temperature Exceedance OccupiedHours [hr]
    // Hours of Safety for Heat Events
    EXPECT_NEAR(3.0, state->dataHeatBal->Resilience(1).ZoneHeatHourOfSafetyBins[0], 1e-8);  // Hours of safety
    EXPECT_NEAR(12.0, state->dataHeatBal->Resilience(1).ZoneHeatHourOfSafetyBins[2], 1e-8); // Safe Temperature Exceedance Hours [hr]
    EXPECT_NEAR(11.2, state->dataHeatBal->Resilience(1).ZoneHeatHourOfSafetyBins[3], 1e-8); // Safe Temperature Exceedance OccupantHours [hr]
    EXPECT_NEAR(11.0, state->dataHeatBal->Resilience(1).ZoneHeatHourOfSafetyBins[4], 1e-8); // Safe Temperature Exceedance OccupiedHours [hr]

    // Unmet Degree-Hours
    EXPECT_NEAR(36.5, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[0], 1e-8); // Cooling Setpoint Unmet Degree-Hours
    EXPECT_NEAR(37.4, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[1], 1e-8); // Cooling Setpoint Unmet Occupant-Weighted Degree-Hours
    EXPECT_NEAR(34.0, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[2], 1e-8); // Cooling Setpoint Unmet Occupied Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[3], 1e-8);  // Heating Setpoint Unmet Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[4], 1e-8);  // Heating Setpoint Unmet Occupant-Weighted Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[5], 1e-8);  // Heating Setpoint Unmet Occupied Degree-Hours

    // Discomfort-weighted Exceedance OccupantHours and OccupiedHours
    EXPECT_NEAR(1.2, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccuHourBins[0], 1e-8);     // Very-cold Exceedance OccupantHours
    EXPECT_NEAR(9.72, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccuHourBins[1], 1e-8);    // Cool Exceedance OccupantHours
    EXPECT_NEAR(2.4, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccuHourBins[2], 1e-8);     // Warm Exceedance OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccuHourBins[3], 1e-8);     // Very-hot Exceedance OccupantHours
    EXPECT_NEAR(1.5, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccupiedHourBins[0], 1e-8); // Very-cold Exceedance OccupiedHours
    EXPECT_NEAR(9.9, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccupiedHourBins[1], 1e-8); // Cool Exceedance OccupiedHours
    EXPECT_NEAR(3.0, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccupiedHourBins[2], 1e-8); // Warm Exceedance OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccupiedHourBins[3], 1e-8); // Very-hot Exceedance OccupiedHours

    state->dataHeatBal->People(1).sched->currentVal = 0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZTAV = 12;
    state->dataThermalComforts->ThermalComfortData(1).FangerPMV = 1.2;
    for (int hour = 19; hour <= 20; hour++) {
        state->dataGlobal->HourOfDay = hour;
        ReportThermalResilience(*state);
    }

    // Test SET longest duration calculation
    // Cooling Unmet Duration: Hour 1 - 4 (no occupants), Hour 8 - 10;
    // Heating Unmet Duration: Hour 5 - 7, Hour 13 - 18, Hour 18 - 20 (no occupants);
    // Cooling SET Degree-Hours
    EXPECT_NEAR(10.0, state->dataHeatBal->Resilience(1).ZoneHighSETHours[0], 1e-8); // SET Degree-Hours
    EXPECT_NEAR(12.0, state->dataHeatBal->Resilience(1).ZoneHighSETHours[1], 1e-8); // SET OccupantHours
    EXPECT_NEAR(6.0, state->dataHeatBal->Resilience(1).ZoneHighSETHours[2], 1e-8);  // SET OccupiedHours
    EXPECT_NEAR(3.0, state->dataHeatBal->Resilience(1).ZoneHighSETHours[3], 1e-8);  // Longest SET > 30°C Duration [hr]
    // Heating SET Degree-Hours
    EXPECT_NEAR(11.0, state->dataHeatBal->Resilience(1).ZoneLowSETHours[0], 1e-8); // SET Degree-Hours
    EXPECT_NEAR(7.2, state->dataHeatBal->Resilience(1).ZoneLowSETHours[1], 1e-8);  // SET OccupantHours
    EXPECT_NEAR(9.0, state->dataHeatBal->Resilience(1).ZoneLowSETHours[2], 1e-8);  // SET OccupiedHours
    EXPECT_NEAR(6.0, state->dataHeatBal->Resilience(1).ZoneLowSETHours[3], 1e-8);  // Longest SET ≤ 12.2°C Duration [hr]

    // Hours of Safety for Cold Events
    EXPECT_NEAR(18.0, state->dataHeatBal->Resilience(1).ZoneColdHourOfSafetyBins[0], 1e-8); // Hours of safety
    EXPECT_NEAR(2.0, state->dataHeatBal->Resilience(1).ZoneColdHourOfSafetyBins[2], 1e-8);  // Safe Temperature Exceedance Hours [hr]
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneColdHourOfSafetyBins[3], 1e-8);  // Safe Temperature Exceedance OccupantHours [hr]
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneColdHourOfSafetyBins[4], 1e-8);  // Safe Temperature Exceedance OccupiedHours [hr]
    // Hours of Safety for Heat Events
    EXPECT_NEAR(3.0, state->dataHeatBal->Resilience(1).ZoneHeatHourOfSafetyBins[0], 1e-8);  // Hours of safety
    EXPECT_NEAR(12.0, state->dataHeatBal->Resilience(1).ZoneHeatHourOfSafetyBins[2], 1e-8); // Safe Temperature Exceedance Hours [hr]
    EXPECT_NEAR(11.2, state->dataHeatBal->Resilience(1).ZoneHeatHourOfSafetyBins[3], 1e-8); // Safe Temperature Exceedance OccupantHours [hr]
    EXPECT_NEAR(11.0, state->dataHeatBal->Resilience(1).ZoneHeatHourOfSafetyBins[4], 1e-8); // Safe Temperature Exceedance OccupiedHours [hr]

    // Unmet Degree-Hours
    EXPECT_NEAR(36.5, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[0], 1e-8); // Cooling Setpoint Unmet Degree-Hours
    EXPECT_NEAR(37.4, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[1], 1e-8); // Cooling Setpoint Unmet Occupant-Weighted Degree-Hours
    EXPECT_NEAR(34.0, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[2], 1e-8); // Cooling Setpoint Unmet Occupied Degree-Hours
    EXPECT_NEAR(6.0, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[3], 1e-8);  // Heating Setpoint Unmet Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[4], 1e-8);  // Heating Setpoint Unmet Occupant-Weighted Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneUnmetDegreeHourBins[5], 1e-8);  // Heating Setpoint Unmet Occupied Degree-Hours

    // Discomfort-weighted Exceedance OccupantHours and OccupiedHours
    EXPECT_NEAR(1.2, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccuHourBins[0], 1e-8);     // Very-cold Exceedance OccupantHours
    EXPECT_NEAR(9.72, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccuHourBins[1], 1e-8);    // Cool Exceedance OccupantHours
    EXPECT_NEAR(2.4, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccuHourBins[2], 1e-8);     // Warm Exceedance OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccuHourBins[3], 1e-8);     // Very-hot Exceedance OccupantHours
    EXPECT_NEAR(1.5, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccupiedHourBins[0], 1e-8); // Very-cold Exceedance OccupiedHours
    EXPECT_NEAR(9.9, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccupiedHourBins[1], 1e-8); // Cool Exceedance OccupiedHours
    EXPECT_NEAR(3.0, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccupiedHourBins[2], 1e-8); // Warm Exceedance OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBal->Resilience(1).ZoneDiscomfortWtExceedOccupiedHourBins[3], 1e-8); // Very-hot Exceedance OccupiedHours

    state->dataContaminantBalance->ZoneAirCO2Avg.allocate(state->dataGlobal->NumOfZones);
    state->dataContaminantBalance->Contaminant.CO2Simulation = true;
    state->dataHeatBal->People(1).sched->currentVal = 1;
    state->dataOutRptTab->displayCO2ResilienceSummary = true;
    state->dataContaminantBalance->ZoneAirCO2Avg(1) = 1100;
    ReportCO2Resilience(*state);
    EXPECT_EQ(1, state->dataHeatBal->Resilience(1).ZoneCO2LevelHourBins[1]);
    EXPECT_EQ(2, state->dataHeatBal->Resilience(1).ZoneCO2LevelOccuHourBins[1]);
    EXPECT_EQ(1, state->dataHeatBal->Resilience(1).ZoneCO2LevelOccupiedHourBins[1]);

    state->dataDayltg->ZoneDaylight.allocate(state->dataGlobal->NumOfZones);
    int totDaylightingControls = state->dataGlobal->NumOfZones;
    state->dataDayltg->daylightControl.allocate(totDaylightingControls);
    state->dataDayltg->daylightControl(1).DaylightMethod = Dayltg::DaylightingMethod::SplitFlux;
    state->dataDayltg->daylightControl(1).zoneIndex = 1;
    state->dataDayltg->daylightControl(1).TotalDaylRefPoints = 1;
    state->dataDayltg->ZoneDaylight(1).totRefPts = 1;
    state->dataDayltg->daylightControl(1).refPts.allocate(1);
    state->dataDayltg->daylightControl(1).PowerReductionFactor = 0.5;
    state->dataDayltg->daylightControl(1).refPts(1).lums[(int)DataSurfaces::Lum::Illum] = 300;
    state->dataDayltg->daylightControl(1).refPts(1).illumSetPoint = 400;
    state->dataOutRptTab->displayVisualResilienceSummary = true;

    ReportVisualResilience(*state);
    EXPECT_EQ(1, state->dataHeatBal->Resilience(1).ZoneLightingLevelHourBins[2]);
    EXPECT_EQ(2, state->dataHeatBal->Resilience(1).ZoneLightingLevelOccuHourBins[2]);
    EXPECT_EQ(1, state->dataHeatBal->Resilience(1).ZoneLightingLevelOccupiedHourBins[2]);
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_TestThermalResilienceReportRepPeriod)
{
    std::string const idf_objects = delimited_string({"Output:Table:ReportPeriod,",
                                                      "ReportPeriod_1,               !- field Name,",
                                                      "ThermalResilienceSummary,     !- field Report Name,",
                                                      ",                             !- Begin Year",
                                                      "1,                            !- Begin Month",
                                                      "1,                            !- Begin Day of Month",
                                                      "1,                            !- Begin Hour of Day",
                                                      ",                             !- End Year",
                                                      "1,                            !- End Month",
                                                      "1,                            !- End Day of Month",
                                                      "10;                           !- End Hour of Day",

                                                      "Output:Table:ReportPeriod,",
                                                      "ReportPeriod_2,               !- field Name,",
                                                      "ThermalResilienceSummary,     !- field Report Name,",
                                                      ",                             !- Begin Year",
                                                      "1,                            !- Begin Month",
                                                      "1,                            !- Begin Day of Month",
                                                      "13,                           !- Begin Hour of Day",
                                                      ",                             !- End Year",
                                                      "1,                            !- End Month",
                                                      "1,                            !- End Day of Month",
                                                      "20;                           !- End Hour of Day"});

    ASSERT_TRUE(process_idf(idf_objects));
    bool ErrorsFound = false;
    state->dataWeather->TotReportPers = 2;

    Weather::GetReportPeriodData(*state, state->dataWeather->TotReportPers, ErrorsFound);
    state->dataWeather->TotThermalReportPers = 0;
    state->dataWeather->TotCO2ReportPers = 0;
    state->dataWeather->TotVisualReportPers = 0;
    Weather::GroupReportPeriodByType(*state, state->dataWeather->TotReportPers);

    EXPECT_EQ(state->dataWeather->TotThermalReportPers, 2);

    state->dataGlobal->NumOfZones = 1;
    state->dataGlobal->KindOfSim = Constant::KindOfSim::RunPeriodWeather;
    state->dataOutRptTab->displayThermalResilienceSummary = true;
    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 1;
    state->dataGlobal->TimeStepsInHour = 4;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 15; // must initialize this to get schedules initialized

    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->TimeStepZone = 1;
    state->dataEnvrn->OutBaroPress = 101325.0;

    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->Resilience.allocate(state->dataGlobal->NumOfZones);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(state->dataGlobal->NumOfZones);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZTAV = 0.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRatAvg = 0.0;

    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);

    state->dataHeatBalFanSys->zoneTstatSetpts.allocate(state->dataGlobal->NumOfZones);
    for (auto &zoneTstatSetpt : state->dataHeatBalFanSys->zoneTstatSetpts) {
        zoneTstatSetpt.setptLo = 22.0;
        zoneTstatSetpt.setptHi = 28.0;
    }

    state->dataHeatBalFanSys->ZoneHeatIndexHourBinsRepPeriod.allocate(state->dataGlobal->NumOfZones, state->dataWeather->TotThermalReportPers);
    state->dataHeatBalFanSys->ZoneHumidexHourBinsRepPeriod.allocate(state->dataGlobal->NumOfZones, state->dataWeather->TotThermalReportPers);
    state->dataHeatBalFanSys->ZoneHeatIndexOccuHourBinsRepPeriod.allocate(state->dataGlobal->NumOfZones, state->dataWeather->TotThermalReportPers);
    state->dataHeatBalFanSys->ZoneHeatIndexOccupiedHourBinsRepPeriod.allocate(state->dataGlobal->NumOfZones,
                                                                              state->dataWeather->TotThermalReportPers);
    state->dataHeatBalFanSys->ZoneHumidexOccuHourBinsRepPeriod.allocate(state->dataGlobal->NumOfZones, state->dataWeather->TotThermalReportPers);
    state->dataHeatBalFanSys->ZoneHumidexOccupiedHourBinsRepPeriod.allocate(state->dataGlobal->NumOfZones, state->dataWeather->TotThermalReportPers);
    state->dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod.allocate(state->dataGlobal->NumOfZones, state->dataWeather->TotThermalReportPers);
    state->dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod.allocate(state->dataGlobal->NumOfZones, state->dataWeather->TotThermalReportPers);
    state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod.allocate(state->dataGlobal->NumOfZones, state->dataWeather->TotThermalReportPers);
    state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod.allocate(state->dataGlobal->NumOfZones,
                                                                                   state->dataWeather->TotThermalReportPers);
    state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod.allocate(state->dataGlobal->NumOfZones,
                                                                                       state->dataWeather->TotThermalReportPers);
    state->dataHeatBalFanSys->CrossedColdThreshRepPeriod.allocate(state->dataGlobal->NumOfZones, state->dataWeather->TotThermalReportPers);
    state->dataHeatBalFanSys->CrossedHeatThreshRepPeriod.allocate(state->dataGlobal->NumOfZones, state->dataWeather->TotThermalReportPers);
    state->dataHeatBalFanSys->CrossedColdThreshRepPeriod = false;
    state->dataHeatBalFanSys->CrossedHeatThreshRepPeriod = false;

    state->dataHeatBalFanSys->zoneTstatSetpts.allocate(state->dataGlobal->NumOfZones);
    for (auto &zoneTstatSetpt : state->dataHeatBalFanSys->zoneTstatSetpts) {
        zoneTstatSetpt.setptLo = 22.0;
        zoneTstatSetpt.setptHi = 28.0;
    }

    state->dataHeatBal->TotPeople = 1;
    state->dataHeatBal->People.allocate(state->dataHeatBal->TotPeople);
    state->dataHeatBal->People(1).ZonePtr = 1;
    state->dataHeatBal->People(1).Pierce = true;
    state->dataHeatBal->People(1).NumberOfPeople = 2;
    state->dataHeatBal->People(1).sched = Sched::AddScheduleConstant(*state, "Occupancy");
    state->dataHeatBal->People(1).ColdStressTempThresh = 15.0;
    state->dataHeatBal->People(1).HeatStressTempThresh = 29.5;
    state->dataHeatBalFanSys->zoneTstatSetpts(1).setptHi = 27.5;
    state->dataHeatBalFanSys->zoneTstatSetpts(1).setptLo = 15.0;

    state->dataThermalComforts->ThermalComfortData.allocate(state->dataHeatBal->TotPeople);
    state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod.allocate(state->dataGlobal->NumOfZones, state->dataWeather->TotThermalReportPers);
    state->dataHeatBalFanSys->ZoneHighSETHoursRepPeriod.allocate(state->dataGlobal->NumOfZones, state->dataWeather->TotThermalReportPers);
    state->dataHeatBalFanSys->lowSETLongestHoursRepPeriod.allocate(state->dataGlobal->NumOfZones, state->dataWeather->TotThermalReportPers);
    state->dataHeatBalFanSys->highSETLongestHoursRepPeriod.allocate(state->dataGlobal->NumOfZones, state->dataWeather->TotThermalReportPers);
    state->dataHeatBalFanSys->lowSETLongestStartRepPeriod.allocate(state->dataGlobal->NumOfZones, state->dataWeather->TotThermalReportPers);
    state->dataHeatBalFanSys->highSETLongestStartRepPeriod.allocate(state->dataGlobal->NumOfZones, state->dataWeather->TotThermalReportPers);
    state->dataThermalComforts->ThermalComfortData(1).PierceSET = 31;
    state->dataHeatBal->People(1).sched->currentVal = 0;

    // ---------------------------------------------------------------------
    // Report Period I start
    // ---------------------------------------------------------------------
    // Heat Index Case 1: Zone T < 80 F;
    state->dataGlobal->HourOfDay = 1;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZTAV = 25;
    state->dataThermalComforts->ThermalComfortData(1).FangerPMV = -4.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRatAvg = 0.00988; // RH = 50%
    CalcThermalResilience(*state);
    ReportThermalResilience(*state);
    EXPECT_NEAR(25, state->dataHeatBal->Resilience(1).ZoneHeatIndex, 0.5);
    EXPECT_NEAR(28, state->dataHeatBal->Resilience(1).ZoneHumidex, 1);

    // Heat Index Case 2: Zone RH > 85, 80 < T < 87 F;
    state->dataGlobal->HourOfDay = 2;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZTAV = 27;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRatAvg = 0.02035; // RH = 90%
    CalcThermalResilience(*state);
    ReportThermalResilience(*state);
    EXPECT_NEAR(31, state->dataHeatBal->Resilience(1).ZoneHeatIndex, 0.5);
    EXPECT_NEAR(39, state->dataHeatBal->Resilience(1).ZoneHumidex, 1);

    // Heat Index Case 3: < Zone RH > 85, 80 < T < 87 F;
    state->dataGlobal->HourOfDay = 3;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZTAV = 27;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRatAvg = 0.0022; // RH = 10%
    CalcThermalResilience(*state);
    ReportThermalResilience(*state);
    EXPECT_NEAR(26, state->dataHeatBal->Resilience(1).ZoneHeatIndex, 0.5);
    EXPECT_NEAR(23, state->dataHeatBal->Resilience(1).ZoneHumidex, 1);

    // Heat Index Case 4: Rothfusz regression, other than the above conditions;
    state->dataGlobal->HourOfDay = 4;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZTAV = 30;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRatAvg = 0.01604; // RH = 60%
    CalcThermalResilience(*state);
    ReportThermalResilience(*state);
    EXPECT_NEAR(33, state->dataHeatBal->Resilience(1).ZoneHeatIndex, 0.5);
    EXPECT_NEAR(38, state->dataHeatBal->Resilience(1).ZoneHumidex, 1);

    // Test categorization of the first 4 hours.
    EXPECT_NEAR(2.0, state->dataHeatBalFanSys->ZoneHeatIndexHourBinsRepPeriod(1, 1)[0], 1e-8); // Safe: Heat Index <= 80 °F (32.2 °C).
    EXPECT_NEAR(1.0, state->dataHeatBalFanSys->ZoneHeatIndexHourBinsRepPeriod(1, 1)[1], 1e-8); // Caution: (80, 90 °F] / (26.7, 32.2 °C]
    EXPECT_NEAR(1.0, state->dataHeatBalFanSys->ZoneHeatIndexHourBinsRepPeriod(1, 1)[2], 1e-8); // Extreme Caution (90, 105 °F] / (32.2, 40.6 °C]
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneHeatIndexHourBinsRepPeriod(1, 1)[3], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneHeatIndexOccuHourBinsRepPeriod(1, 1)[0], 1e-8);     // # of People = 0
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneHeatIndexOccupiedHourBinsRepPeriod(1, 1)[0], 1e-8); // # of People = 0

    EXPECT_NEAR(2.0, state->dataHeatBalFanSys->ZoneHumidexHourBinsRepPeriod(1, 1)[0], 1e-8);         // Humidex <= 29
    EXPECT_NEAR(2.0, state->dataHeatBalFanSys->ZoneHumidexHourBinsRepPeriod(1, 1)[1], 1e-8);         // Humidex (29, 40]
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneHumidexOccuHourBinsRepPeriod(1, 1)[0], 1e-8);     // # of People = 0
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneHumidexOccupiedHourBinsRepPeriod(1, 1)[0], 1e-8); // # of People = 0

    // SET Degree-Hr Table Data: reporting period I
    //    hour  PierceSET  OccuSchedule  TotalHighSet	HiSetOccuHour	HiSetOccupied	TotalLowSet	LowOccuHour Low Occupied
    //    1     31         0	         1	        0	        0	        0	        0	    0
    //    2     31         0	         2	        0	        0	        0	        0	    0
    //    3     31         0	         3	        0	        0	        0	        0	    0
    //    4     31         0	         4	        0	        0	        0	        0	    0
    //    5     11.2       0.4           4	        0	        0	        1	        0.8	    1
    //    6     11.2       0.4           4	        0	        0	        2	        1.6	    2
    //    7     11.2       0.4           4	        0	        0	        3	        2.4	    3
    //    8     32         1	         6	        4	        2	        3	        2.4	    3
    //    9     32         1	         8	        8	        4	        3	        2.4	    3
    //    10    32         1	         10	        12	        6	        3	        2.4	    3

    // Cooling SET Degree-Hours
    EXPECT_NEAR(4.0, state->dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(1, 1)[0], 1e-8); // SET Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(1, 1)[1], 1e-8); // SET OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(1, 1)[2], 1e-8); // SET OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(1, 1)[3], 1e-8); // Longest SET > 30°C Duration [hr]
    // Heating SET Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(1, 1)[0], 1e-8); // SET Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(1, 1)[1], 1e-8); // SET OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(1, 1)[2], 1e-8); // SET OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(1, 1)[3], 1e-8); // Longest SET ≤ 12.2°C Duration [hr]

    // Hour of safety table data: reporting period I
    //                  Cold                                          Heat
    //    hour	ZTAV	HOS exceed-hr exceed-occuhour exceed occupied HOS exceed-hr exceed-occuhour exceed-occupied
    //    1	25	1	0	 0	      0	              1	    0	     0	                  0
    //    2	27	2	0	 0	      0	              2	    0	     0	                  0
    //    3	27	3	0	 0	      0	              3	    0	     0	                  0
    //    4	30	4	0	 0	      0	              3	    1	     0	                  0
    //    5	31	5	0	 0	      0	              3	    2	     0.8	          1
    //    6	31	6	0	 0	      0	              3	    3	     1.6	          2
    //    7	31	7	0	 0	      0	              3	    4	     2.4	          3
    //    8	28	8	0	 0	      0	              3	    4	     2.4	          3
    //    9	28	9	0	 0	      0	              3	    4	     2.4	          3
    //    10	28	10	0	 0	      0	              3	    4	     2.4	          3

    // Hours of Safety for Cold Events
    EXPECT_NEAR(4.0, state->dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod(1, 1)[0], 1e-8); // Hours of safety
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod(1, 1)[2], 1e-8); // Safe Temperature Exceedance Hours [hr]
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod(1, 1)[3], 1e-8); // Safe Temperature Exceedance OccupantHours [hr]
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod(1, 1)[4], 1e-8); // Safe Temperature Exceedance OccupiedHours [hr]
    // Hours of Safety for Heat Events
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod(1, 1)[0], 1e-8); // Hours of safety
    EXPECT_NEAR(1.0, state->dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod(1, 1)[2], 1e-8); // Safe Temperature Exceedance Hours [hr]
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod(1, 1)[3], 1e-8); // Safe Temperature Exceedance OccupantHours [hr]
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod(1, 1)[4], 1e-8); // Safe Temperature Exceedance OccupiedHours [hr]

    //    Unmet Degree Hour table data: reporting period I
    //    cooling setpoint	27.5
    //    heating setpoint	15
    //    number of people	2
    //
    //                              Cooling unmet                         Heating unmet
    //    hour  OccuSche  ZTAV      deg-hr  deg-occu-hr  deg-occupied-hr  deg-hr deg-occu-hr deg-occupied-hr
    //    1	    0	      25	0	0	     0	              0	     0	         0
    //    2	    0	      27	0	0	     0	              0	     0	         0
    //    3	    0	      27	0	0	     0	              0	     0	         0
    //    4	    0	      30	2.5	0	     0	              0	     0	         0
    //    5	    0.4	      31	6	2.8	     3.5	      0	     0	         0
    //    6	    0.4	      31	9.5	5.6	     7	              0	     0	         0
    //    7	    0.4	      31	13	8.4	     10.5	      0	     0	         0
    //    8	    1	      28	13.5	9.4	     11	              0	     0	         0
    //    9	    1	      28	14	10.4	     11.5	      0	     0	         0
    //    10        1	      28	14.5	11.4	     12	              0      0	         0

    // Unmet Degree-Hours
    EXPECT_NEAR(2.5, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 1)[0], 1e-8); // Cooling Setpoint Unmet Degree-Hours
    EXPECT_NEAR(
        0.0, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 1)[1], 1e-8); // Cooling Setpoint Unmet Occupant-Weighted Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 1)[2], 1e-8); // Cooling Setpoint Unmet Occupied Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 1)[3], 1e-8); // Heating Setpoint Unmet Degree-Hours
    EXPECT_NEAR(
        0.0, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 1)[4], 1e-8); // Heating Setpoint Unmet Occupant-Weighted Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 1)[5], 1e-8); // Heating Setpoint Unmet Occupied Degree-Hours

    // Discomfort-weighted Exceedance table data: reporting period I
    //    VeryHotPMVThresh 	3
    //    WarmPMVThresh 	0.7
    //    CoolPMVThresh 	-0.7
    //    VeryColdPMVThresh 	-3
    //    num people	2
    //                          Exceedance Occupant Hour         Exceedance Occupied Hour
    //    hour	OccuSche  PMV	VeryCold cool   warm    VeryHot  VeryCold cool  warm    VeryHot
    //    1	0	 -4	0	 0	0	0	 0	  0	0	0
    //    2	0	-4	0	 0	0	0	 0	  0	0	0
    //    3	0	-4	0	 0	0	0	 0	  0	0	0
    //    4	0	-4	0	 0	0	0	 0	  0	0	0
    //    5	0.4	-3.5	0.4	 2.24	0	0	 0.5	  2.8	0	0
    //    6	0.4	-3.5	0.8	 4.48	0	0	 1	  5.6	0	0
    //    7	0.4	-3.5	1.2	 6.72	0	0	 1.5	  8.4	0	0
    //    8	1	-1.2	1.2	 7.72	0	0	 1.5	  8.9	0	0
    //    9	1	-1.2	1.2	 8.72	0	0	 1.5	  9.4	0	0
    //    10	1	-1.2	1.2	 9.72	0	0	 1.5	  9.9	0	0

    // Discomfort-weighted Exceedance OccupantHours and OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod(1, 1)[0], 1e-8);     // Very-cold Exceedance OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod(1, 1)[1], 1e-8);     // Cool Exceedance OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod(1, 1)[2], 1e-8);     // Warm Exceedance OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod(1, 1)[3], 1e-8);     // Very-hot Exceedance OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod(1, 1)[0], 1e-8); // Very-cold Exceedance OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod(1, 1)[1], 1e-8); // Cool Exceedance OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod(1, 1)[2], 1e-8); // Warm Exceedance OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod(1, 1)[3], 1e-8); // Very-hot Exceedance OccupiedHours

    state->dataThermalComforts->ThermalComfortData(1).PierceSET = 11.2;
    state->dataHeatBal->People(1).sched->currentVal = 0.4;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZTAV = 31;
    state->dataThermalComforts->ThermalComfortData(1).FangerPMV = -3.5;
    for (int hour = 5; hour <= 7; hour++) {
        state->dataGlobal->HourOfDay = hour;
        //        CalcThermalResilience(*state);
        ReportThermalResilience(*state);
    }
    // Test SET-hours calculation - Heating unmet
    // Cooling SET Degree-Hours
    EXPECT_NEAR(4.0, state->dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(1, 1)[0], 1e-8); // SET Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(1, 1)[1], 1e-8); // SET OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(1, 1)[2], 1e-8); // SET OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(1, 1)[3], 1e-8); // Longest SET > 30°C Duration [hr]
    // Heating SET Degree-Hours
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(1, 1)[0], 1e-8); // SET Degree-Hours
    EXPECT_NEAR(2.4, state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(1, 1)[1], 1e-8); // SET OccupantHours
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(1, 1)[2], 1e-8); // SET OccupiedHours
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(1, 1)[3], 1e-8); // Longest SET ≤ 12.2°C Duration [hr]

    // Hours of Safety for Cold Events
    EXPECT_NEAR(7.0, state->dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod(1, 1)[0], 1e-8); // Hours of safety
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod(1, 1)[2], 1e-8); // Safe Temperature Exceedance Hours [hr]
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod(1, 1)[3], 1e-8); // Safe Temperature Exceedance OccupantHours [hr]
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod(1, 1)[4], 1e-8); // Safe Temperature Exceedance OccupiedHours [hr]
    // Hours of Safety for Heat Events
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod(1, 1)[0], 1e-8); // Hours of safety
    EXPECT_NEAR(4.0, state->dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod(1, 1)[2], 1e-8); // Safe Temperature Exceedance Hours [hr]
    EXPECT_NEAR(2.4, state->dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod(1, 1)[3], 1e-8); // Safe Temperature Exceedance OccupantHours [hr]
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod(1, 1)[4], 1e-8); // Safe Temperature Exceedance OccupiedHours [hr]

    // Unmet Degree-Hours
    EXPECT_NEAR(13.0, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 1)[0], 1e-8); // Cooling Setpoint Unmet Degree-Hours
    EXPECT_NEAR(
        8.4, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 1)[1], 1e-8); // Cooling Setpoint Unmet Occupant-Weighted Degree-Hours
    EXPECT_NEAR(10.5, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 1)[2], 1e-8); // Cooling Setpoint Unmet Occupied Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 1)[3], 1e-8);  // Heating Setpoint Unmet Degree-Hours
    EXPECT_NEAR(
        0.0, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 1)[4], 1e-8); // Heating Setpoint Unmet Occupant-Weighted Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 1)[5], 1e-8); // Heating Setpoint Unmet Occupied Degree-Hours

    // Discomfort-weighted Exceedance OccupantHours and OccupiedHours
    EXPECT_NEAR(1.2, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod(1, 1)[0], 1e-8);     // Very-cold Exceedance OccupantHours
    EXPECT_NEAR(6.72, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod(1, 1)[1], 1e-8);    // Cool Exceedance OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod(1, 1)[2], 1e-8);     // Warm Exceedance OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod(1, 1)[3], 1e-8);     // Very-hot Exceedance OccupantHours
    EXPECT_NEAR(1.5, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod(1, 1)[0], 1e-8); // Very-cold Exceedance OccupiedHours
    EXPECT_NEAR(8.4, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod(1, 1)[1], 1e-8); // Cool Exceedance OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod(1, 1)[2], 1e-8); // Warm Exceedance OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod(1, 1)[3], 1e-8); // Very-hot Exceedance OccupiedHours

    state->dataThermalComforts->ThermalComfortData(1).PierceSET = 32;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZTAV = 28;
    state->dataHeatBal->People(1).sched->currentVal = 1.0;
    state->dataThermalComforts->ThermalComfortData(1).FangerPMV = -1.2;
    for (int hour = 8; hour <= 10; hour++) {
        state->dataGlobal->HourOfDay = hour;
        ReportThermalResilience(*state);
    }
    // Test SET-hours calculation - Cooling unmet
    // Cooling SET Degree-Hours
    EXPECT_NEAR(10.0, state->dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(1, 1)[0], 1e-8); // SET Degree-Hours
    EXPECT_NEAR(12.0, state->dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(1, 1)[1], 1e-8); // SET OccupantHours
    EXPECT_NEAR(6.0, state->dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(1, 1)[2], 1e-8);  // SET OccupiedHours
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(1, 1)[3], 1e-8);  // Longest SET > 30°C Duration [hr]
    // Heating SET Degree-Hours
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(1, 1)[0], 1e-8); // SET Degree-Hours
    EXPECT_NEAR(2.4, state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(1, 1)[1], 1e-8); // SET OccupantHours
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(1, 1)[2], 1e-8); // SET OccupiedHours
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(1, 1)[3], 1e-8); // Longest SET ≤ 12.2°C Duration [hr]

    // Hours of Safety for Cold Events
    EXPECT_NEAR(10.0, state->dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod(1, 1)[0], 1e-8); // Hours of safety
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod(1, 1)[2], 1e-8);  // Safe Temperature Exceedance Hours [hr]
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod(1, 1)[3], 1e-8);  // Safe Temperature Exceedance OccupantHours [hr]
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod(1, 1)[4], 1e-8);  // Safe Temperature Exceedance OccupiedHours [hr]
    // Hours of Safety for Heat Events
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod(1, 1)[0], 1e-8); // Hours of safety
    EXPECT_NEAR(4.0, state->dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod(1, 1)[2], 1e-8); // Safe Temperature Exceedance Hours [hr]
    EXPECT_NEAR(2.4, state->dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod(1, 1)[3], 1e-8); // Safe Temperature Exceedance OccupantHours [hr]
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod(1, 1)[4], 1e-8); // Safe Temperature Exceedance OccupiedHours [hr]

    // Unmet Degree-Hours
    EXPECT_NEAR(14.5, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 1)[0], 1e-8); // Cooling Setpoint Unmet Degree-Hours
    EXPECT_NEAR(
        11.4, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 1)[1], 1e-8); // Cooling Setpoint Unmet Occupant-Weighted Degree-Hours
    EXPECT_NEAR(12.0, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 1)[2], 1e-8); // Cooling Setpoint Unmet Occupied Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 1)[3], 1e-8);  // Heating Setpoint Unmet Degree-Hours
    EXPECT_NEAR(
        0.0, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 1)[4], 1e-8); // Heating Setpoint Unmet Occupant-Weighted Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 1)[5], 1e-8); // Heating Setpoint Unmet Occupied Degree-Hours

    // Discomfort-weighted Exceedance OccupantHours and OccupiedHours
    EXPECT_NEAR(1.2, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod(1, 1)[0], 1e-8);     // Very-cold Exceedance OccupantHours
    EXPECT_NEAR(9.72, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod(1, 1)[1], 1e-8);    // Cool Exceedance OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod(1, 1)[2], 1e-8);     // Warm Exceedance OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod(1, 1)[3], 1e-8);     // Very-hot Exceedance OccupantHours
    EXPECT_NEAR(1.5, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod(1, 1)[0], 1e-8); // Very-cold Exceedance OccupiedHours
    EXPECT_NEAR(9.9, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod(1, 1)[1], 1e-8); // Cool Exceedance OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod(1, 1)[2], 1e-8); // Warm Exceedance OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod(1, 1)[3], 1e-8); // Very-hot Exceedance OccupiedHours

    // ---------------------------------------------------------------------
    // Report Period I end
    // ---------------------------------------------------------------------

    state->dataThermalComforts->ThermalComfortData(1).PierceSET = 25;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZTAV = 31;
    state->dataThermalComforts->ThermalComfortData(1).FangerPMV = 0.5;
    for (int hour = 11; hour <= 12; hour++) {
        state->dataGlobal->HourOfDay = hour;
        ReportThermalResilience(*state);
    }

    // ---------------------------------------------------------------------
    // Report Period II start
    // ---------------------------------------------------------------------

    state->dataThermalComforts->ThermalComfortData(1).PierceSET = 11.2;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZTAV = 30;
    state->dataHeatBal->People(1).sched->currentVal = 0.4;
    state->dataThermalComforts->ThermalComfortData(1).FangerPMV = 1.2;
    for (int hour = 13; hour <= 18; hour++) {
        state->dataGlobal->HourOfDay = hour;
        ReportThermalResilience(*state);
    }

    // Cooling SET Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(1, 2)[0], 1e-8); // SET Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(1, 2)[1], 1e-8); // SET OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(1, 2)[2], 1e-8); // SET OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(1, 2)[3], 1e-8); // Longest SET > 30°C Duration [hr]
    // Heating SET Degree-Hours
    EXPECT_NEAR(6.0, state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(1, 2)[0], 1e-8); // SET Degree-Hours
    EXPECT_NEAR(4.8, state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(1, 2)[1], 1e-8); // SET OccupantHours
    EXPECT_NEAR(6.0, state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(1, 2)[2], 1e-8); // SET OccupiedHours
    EXPECT_NEAR(6.0, state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(1, 2)[3], 1e-8); // Longest SET ≤ 12.2°C Duration [hr]

    // Hours of Safety for Cold Events
    EXPECT_NEAR(6.0, state->dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod(1, 2)[0], 1e-8); // Hours of safety
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod(1, 2)[2], 1e-8); // Safe Temperature Exceedance Hours [hr]
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod(1, 2)[3], 1e-8); // Safe Temperature Exceedance OccupantHours [hr]
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod(1, 2)[4], 1e-8); // Safe Temperature Exceedance OccupiedHours [hr]
    // Hours of Safety for Heat Events
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod(1, 2)[0], 1e-8); // Hours of safety
    EXPECT_NEAR(6.0, state->dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod(1, 2)[2], 1e-8); // Safe Temperature Exceedance Hours [hr]
    EXPECT_NEAR(4.8, state->dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod(1, 2)[3], 1e-8); // Safe Temperature Exceedance OccupantHours [hr]
    EXPECT_NEAR(6.0, state->dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod(1, 2)[4], 1e-8); // Safe Temperature Exceedance OccupiedHours [hr]

    // Unmet Degree-Hours
    EXPECT_NEAR(15.0, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 2)[0], 1e-8); // Cooling Setpoint Unmet Degree-Hours
    EXPECT_NEAR(
        12.0, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 2)[1], 1e-8); // Cooling Setpoint Unmet Occupant-Weighted Degree-Hours
    EXPECT_NEAR(15.0, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 2)[2], 1e-8); // Cooling Setpoint Unmet Occupied Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 2)[3], 1e-8);  // Heating Setpoint Unmet Degree-Hours
    EXPECT_NEAR(
        0.0, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 2)[4], 1e-8); // Heating Setpoint Unmet Occupant-Weighted Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 2)[5], 1e-8); // Heating Setpoint Unmet Occupied Degree-Hours

    // Discomfort-weighted Exceedance OccupantHours and OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod(1, 2)[0], 1e-8);     // Very-cold Exceedance OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod(1, 2)[1], 1e-8);     // Cool Exceedance OccupantHours
    EXPECT_NEAR(2.4, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod(1, 2)[2], 1e-8);     // Warm Exceedance OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod(1, 2)[3], 1e-8);     // Very-hot Exceedance OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod(1, 2)[0], 1e-8); // Very-cold Exceedance OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod(1, 2)[1], 1e-8); // Cool Exceedance OccupiedHours
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod(1, 2)[2], 1e-8); // Warm Exceedance OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod(1, 2)[3], 1e-8); // Very-hot Exceedance OccupiedHours

    state->dataHeatBal->People(1).sched->currentVal = 0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).ZTAV = 12;
    state->dataThermalComforts->ThermalComfortData(1).FangerPMV = 1.2;
    for (int hour = 19; hour <= 20; hour++) {
        state->dataGlobal->HourOfDay = hour;
        ReportThermalResilience(*state);
    }

    // Cooling SET Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(1, 2)[0], 1e-8); // SET Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(1, 2)[1], 1e-8); // SET OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(1, 2)[2], 1e-8); // SET OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(1, 2)[3], 1e-8); // Longest SET > 30°C Duration [hr]
    // Heating SET Degree-Hours
    EXPECT_NEAR(8.0, state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(1, 2)[0], 1e-8); // SET Degree-Hours
    EXPECT_NEAR(4.8, state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(1, 2)[1], 1e-8); // SET OccupantHours
    EXPECT_NEAR(6.0, state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(1, 2)[2], 1e-8); // SET OccupiedHours
    EXPECT_NEAR(6.0, state->dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(1, 2)[3], 1e-8); // Longest SET ≤ 12.2°C Duration [hr]

    // Hours of Safety for Cold Events
    EXPECT_NEAR(6.0, state->dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod(1, 2)[0], 1e-8); // Hours of safety
    EXPECT_NEAR(2.0, state->dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod(1, 2)[2], 1e-8); // Safe Temperature Exceedance Hours [hr]
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod(1, 2)[3], 1e-8); // Safe Temperature Exceedance OccupantHours [hr]
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod(1, 2)[4], 1e-8); // Safe Temperature Exceedance OccupiedHours [hr]
    // Hours of Safety for Heat Events
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod(1, 2)[0], 1e-8); // Hours of safety
    EXPECT_NEAR(6.0, state->dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod(1, 2)[2], 1e-8); // Safe Temperature Exceedance Hours [hr]
    EXPECT_NEAR(4.8, state->dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod(1, 2)[3], 1e-8); // Safe Temperature Exceedance OccupantHours [hr]
    EXPECT_NEAR(6.0, state->dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod(1, 2)[4], 1e-8); // Safe Temperature Exceedance OccupiedHours [hr]

    // Unmet Degree-Hours
    EXPECT_NEAR(15.0, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 2)[0], 1e-8); // Cooling Setpoint Unmet Degree-Hours
    EXPECT_NEAR(
        12.0, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 2)[1], 1e-8); // Cooling Setpoint Unmet Occupant-Weighted Degree-Hours
    EXPECT_NEAR(15.0, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 2)[2], 1e-8); // Cooling Setpoint Unmet Occupied Degree-Hours
    EXPECT_NEAR(6.0, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 2)[3], 1e-8);  // Heating Setpoint Unmet Degree-Hours
    EXPECT_NEAR(
        0.0, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 2)[4], 1e-8); // Heating Setpoint Unmet Occupant-Weighted Degree-Hours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(1, 2)[5], 1e-8); // Heating Setpoint Unmet Occupied Degree-Hours

    // Discomfort-weighted Exceedance OccupantHours and OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod(1, 2)[0], 1e-8);     // Very-cold Exceedance OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod(1, 2)[1], 1e-8);     // Cool Exceedance OccupantHours
    EXPECT_NEAR(2.4, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod(1, 2)[2], 1e-8);     // Warm Exceedance OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod(1, 2)[3], 1e-8);     // Very-hot Exceedance OccupantHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod(1, 2)[0], 1e-8); // Very-cold Exceedance OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod(1, 2)[1], 1e-8); // Cool Exceedance OccupiedHours
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod(1, 2)[2], 1e-8); // Warm Exceedance OccupiedHours
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod(1, 2)[3], 1e-8); // Very-hot Exceedance OccupiedHours

    // ---------------------------------------------------------------------
    // Report Period II end
    // ---------------------------------------------------------------------
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_TestCO2ResilienceReportRepPeriod)
{
    std::string const idf_objects = delimited_string({"Output:Table:ReportPeriod,",
                                                      "ReportPeriod_1,               !- field Name,",
                                                      "CO2ResilienceSummary,     !- field Report Name,",
                                                      ",                             !- Begin Year",
                                                      "1,                            !- Begin Month",
                                                      "1,                            !- Begin Day of Month",
                                                      "1,                            !- Begin Hour of Day",
                                                      ",                             !- End Year",
                                                      "1,                            !- End Month",
                                                      "1,                            !- End Day of Month",
                                                      "10;                           !- End Hour of Day",

                                                      "Output:Table:ReportPeriod,",
                                                      "ReportPeriod_2,               !- field Name,",
                                                      "CO2ResilienceSummary,     !- field Report Name,",
                                                      ",                             !- Begin Year",
                                                      "1,                            !- Begin Month",
                                                      "1,                            !- Begin Day of Month",
                                                      "13,                           !- Begin Hour of Day",
                                                      ",                             !- End Year",
                                                      "1,                            !- End Month",
                                                      "1,                            !- End Day of Month",
                                                      "20;                           !- End Hour of Day"});

    ASSERT_TRUE(process_idf(idf_objects));
    bool ErrorsFound = false;
    state->dataWeather->TotReportPers = 2;

    Weather::GetReportPeriodData(*state, state->dataWeather->TotReportPers, ErrorsFound);
    state->dataWeather->TotThermalReportPers = 0;
    state->dataWeather->TotCO2ReportPers = 0;
    state->dataWeather->TotVisualReportPers = 0;
    Weather::GroupReportPeriodByType(*state, state->dataWeather->TotReportPers);

    EXPECT_EQ(state->dataWeather->TotCO2ReportPers, 2);

    state->dataHeatBalFanSys->ZoneCO2LevelHourBinsRepPeriod.allocate(1, state->dataWeather->TotCO2ReportPers);
    state->dataHeatBalFanSys->ZoneCO2LevelOccuHourBinsRepPeriod.allocate(1, state->dataWeather->TotCO2ReportPers);
    state->dataHeatBalFanSys->ZoneCO2LevelOccupiedHourBinsRepPeriod.allocate(1, state->dataWeather->TotCO2ReportPers);

    state->dataGlobal->NumOfZones = 1;
    state->dataGlobal->KindOfSim = Constant::KindOfSim::RunPeriodWeather;
    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 1;
    state->dataGlobal->TimeStepsInHour = 4;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 15; // must initialize this to get schedules initialized

    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->TimeStepZone = 1;

    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->Resilience.allocate(state->dataGlobal->NumOfZones);

    state->dataHeatBal->TotPeople = 1;
    state->dataHeatBal->People.allocate(state->dataHeatBal->TotPeople);
    state->dataHeatBal->People(1).ZonePtr = 1;
    state->dataHeatBal->People(1).NumberOfPeople = 2;
    state->dataHeatBal->People(1).sched = Sched::AddScheduleConstant(*state, "Occupancy");

    int NoBins = 3;

    for (int i = 1; i <= state->dataWeather->TotCO2ReportPers; i++) {
        state->dataHeatBalFanSys->ZoneCO2LevelHourBinsRepPeriod(1, i).assign(NoBins, 0.0);
        state->dataHeatBalFanSys->ZoneCO2LevelOccuHourBinsRepPeriod(1, i).assign(NoBins, 0.0);
        state->dataHeatBalFanSys->ZoneCO2LevelOccupiedHourBinsRepPeriod(1, i).assign(NoBins, 0.0);
    }

    state->dataContaminantBalance->ZoneAirCO2Avg.allocate(state->dataGlobal->NumOfZones);
    state->dataContaminantBalance->Contaminant.CO2Simulation = true;
    state->dataOutRptTab->displayCO2ResilienceSummary = true;

    state->dataHeatBal->People(1).sched->currentVal = 0;
    state->dataContaminantBalance->ZoneAirCO2Avg(1) = 900;
    for (int hour = 1; hour <= 4; hour++) {
        state->dataGlobal->HourOfDay = hour;
        ReportCO2Resilience(*state);
    }
    EXPECT_NEAR(4.0, state->dataHeatBalFanSys->ZoneCO2LevelHourBinsRepPeriod(1, 1)[0], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneCO2LevelHourBinsRepPeriod(1, 1)[1], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneCO2LevelHourBinsRepPeriod(1, 1)[2], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneCO2LevelOccuHourBinsRepPeriod(1, 1)[0], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneCO2LevelOccuHourBinsRepPeriod(1, 1)[1], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneCO2LevelOccuHourBinsRepPeriod(1, 1)[2], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneCO2LevelOccupiedHourBinsRepPeriod(1, 1)[0], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneCO2LevelOccupiedHourBinsRepPeriod(1, 1)[1], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneCO2LevelOccupiedHourBinsRepPeriod(1, 1)[2], 1e-8);

    state->dataHeatBal->People(1).sched->currentVal = 0.4;
    state->dataContaminantBalance->ZoneAirCO2Avg(1) = 1100;
    for (int hour = 5; hour <= 7; hour++) {
        state->dataGlobal->HourOfDay = hour;
        ReportCO2Resilience(*state);
    }
    EXPECT_NEAR(4.0, state->dataHeatBalFanSys->ZoneCO2LevelHourBinsRepPeriod(1, 1)[0], 1e-8);
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneCO2LevelHourBinsRepPeriod(1, 1)[1], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneCO2LevelHourBinsRepPeriod(1, 1)[2], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneCO2LevelOccuHourBinsRepPeriod(1, 1)[0], 1e-8);
    EXPECT_NEAR(2.4, state->dataHeatBalFanSys->ZoneCO2LevelOccuHourBinsRepPeriod(1, 1)[1], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneCO2LevelOccuHourBinsRepPeriod(1, 1)[2], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneCO2LevelOccupiedHourBinsRepPeriod(1, 1)[0], 1e-8);
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneCO2LevelOccupiedHourBinsRepPeriod(1, 1)[1], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneCO2LevelOccupiedHourBinsRepPeriod(1, 1)[2], 1e-8);

    state->dataHeatBal->People(1).sched->currentVal = 1.0;
    state->dataContaminantBalance->ZoneAirCO2Avg(1) = 5500;
    state->dataGlobal->HourOfDay = 8;
    ReportCO2Resilience(*state);

    EXPECT_NEAR(4.0, state->dataHeatBalFanSys->ZoneCO2LevelHourBinsRepPeriod(1, 1)[0], 1e-8);
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneCO2LevelHourBinsRepPeriod(1, 1)[1], 1e-8);
    EXPECT_NEAR(1.0, state->dataHeatBalFanSys->ZoneCO2LevelHourBinsRepPeriod(1, 1)[2], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneCO2LevelOccuHourBinsRepPeriod(1, 1)[0], 1e-8);
    EXPECT_NEAR(2.4, state->dataHeatBalFanSys->ZoneCO2LevelOccuHourBinsRepPeriod(1, 1)[1], 1e-8);
    EXPECT_NEAR(2.0, state->dataHeatBalFanSys->ZoneCO2LevelOccuHourBinsRepPeriod(1, 1)[2], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneCO2LevelOccupiedHourBinsRepPeriod(1, 1)[0], 1e-8);
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneCO2LevelOccupiedHourBinsRepPeriod(1, 1)[1], 1e-8);
    EXPECT_NEAR(1.0, state->dataHeatBalFanSys->ZoneCO2LevelOccupiedHourBinsRepPeriod(1, 1)[2], 1e-8);

    state->dataHeatBal->People(1).sched->currentVal = 1.0;
    state->dataContaminantBalance->ZoneAirCO2Avg(1) = 2000;
    for (int hour = 9; hour <= 10; hour++) {
        state->dataGlobal->HourOfDay = hour;
        ReportCO2Resilience(*state);
    }
    EXPECT_NEAR(4.0, state->dataHeatBalFanSys->ZoneCO2LevelHourBinsRepPeriod(1, 1)[0], 1e-8);
    EXPECT_NEAR(5.0, state->dataHeatBalFanSys->ZoneCO2LevelHourBinsRepPeriod(1, 1)[1], 1e-8);
    EXPECT_NEAR(1.0, state->dataHeatBalFanSys->ZoneCO2LevelHourBinsRepPeriod(1, 1)[2], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneCO2LevelOccuHourBinsRepPeriod(1, 1)[0], 1e-8);
    EXPECT_NEAR(6.4, state->dataHeatBalFanSys->ZoneCO2LevelOccuHourBinsRepPeriod(1, 1)[1], 1e-8);
    EXPECT_NEAR(2.0, state->dataHeatBalFanSys->ZoneCO2LevelOccuHourBinsRepPeriod(1, 1)[2], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneCO2LevelOccupiedHourBinsRepPeriod(1, 1)[0], 1e-8);
    EXPECT_NEAR(5.0, state->dataHeatBalFanSys->ZoneCO2LevelOccupiedHourBinsRepPeriod(1, 1)[1], 1e-8);
    EXPECT_NEAR(1.0, state->dataHeatBalFanSys->ZoneCO2LevelOccupiedHourBinsRepPeriod(1, 1)[2], 1e-8);

    state->dataHeatBal->People(1).sched->currentVal = 1.0;
    state->dataContaminantBalance->ZoneAirCO2Avg(1) = 1500;
    for (int hour = 13; hour <= 18; hour++) {
        state->dataGlobal->HourOfDay = hour;
        ReportCO2Resilience(*state);
    }
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneCO2LevelHourBinsRepPeriod(1, 2)[0], 1e-8);
    EXPECT_NEAR(6.0, state->dataHeatBalFanSys->ZoneCO2LevelHourBinsRepPeriod(1, 2)[1], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneCO2LevelHourBinsRepPeriod(1, 2)[2], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneCO2LevelOccuHourBinsRepPeriod(1, 2)[0], 1e-8);
    EXPECT_NEAR(12.0, state->dataHeatBalFanSys->ZoneCO2LevelOccuHourBinsRepPeriod(1, 2)[1], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneCO2LevelOccuHourBinsRepPeriod(1, 2)[2], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneCO2LevelOccupiedHourBinsRepPeriod(1, 2)[0], 1e-8);
    EXPECT_NEAR(6.0, state->dataHeatBalFanSys->ZoneCO2LevelOccupiedHourBinsRepPeriod(1, 2)[1], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneCO2LevelOccupiedHourBinsRepPeriod(1, 2)[2], 1e-8);
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_TestVisualResilienceReportRepPeriod)
{
    std::string const idf_objects = delimited_string({"Output:Table:ReportPeriod,",
                                                      "ReportPeriod_1,               !- field Name,",
                                                      "VisualResilienceSummary,     !- field Report Name,",
                                                      ",                             !- Begin Year",
                                                      "1,                            !- Begin Month",
                                                      "1,                            !- Begin Day of Month",
                                                      "1,                            !- Begin Hour of Day",
                                                      ",                             !- End Year",
                                                      "1,                            !- End Month",
                                                      "1,                            !- End Day of Month",
                                                      "10;                           !- End Hour of Day",

                                                      "Output:Table:ReportPeriod,",
                                                      "ReportPeriod_2,               !- field Name,",
                                                      "VisualResilienceSummary,     !- field Report Name,",
                                                      ",                             !- Begin Year",
                                                      "1,                            !- Begin Month",
                                                      "1,                            !- Begin Day of Month",
                                                      "13,                           !- Begin Hour of Day",
                                                      ",                             !- End Year",
                                                      "1,                            !- End Month",
                                                      "1,                            !- End Day of Month",
                                                      "20;                           !- End Hour of Day"});

    ASSERT_TRUE(process_idf(idf_objects));
    bool ErrorsFound = false;
    state->dataWeather->TotReportPers = 2;

    Weather::GetReportPeriodData(*state, state->dataWeather->TotReportPers, ErrorsFound);
    state->dataWeather->TotThermalReportPers = 0;
    state->dataWeather->TotCO2ReportPers = 0;
    state->dataWeather->TotVisualReportPers = 0;
    Weather::GroupReportPeriodByType(*state, state->dataWeather->TotReportPers);

    EXPECT_EQ(state->dataWeather->TotVisualReportPers, 2);

    state->dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod.allocate(1, state->dataWeather->TotVisualReportPers);
    state->dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod.allocate(1, state->dataWeather->TotVisualReportPers);
    state->dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod.allocate(1, state->dataWeather->TotVisualReportPers);

    state->dataGlobal->NumOfZones = 1;
    state->dataGlobal->KindOfSim = Constant::KindOfSim::RunPeriodWeather;
    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 1;
    state->dataGlobal->TimeStepsInHour = 4;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 15; // must initialize this to get schedules initialized

    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->TimeStepZone = 1;

    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataHeatBal->Resilience.allocate(state->dataGlobal->NumOfZones);

    state->dataHeatBal->TotPeople = 1;
    state->dataHeatBal->People.allocate(state->dataHeatBal->TotPeople);
    state->dataHeatBal->People(1).ZonePtr = 1;
    state->dataHeatBal->People(1).NumberOfPeople = 2;
    state->dataHeatBal->People(1).sched = Sched::AddScheduleConstant(*state, "Occupancy");

    state->dataDayltg->ZoneDaylight.allocate(state->dataGlobal->NumOfZones);
    int totDaylightingControls = state->dataGlobal->NumOfZones;
    state->dataDayltg->daylightControl.allocate(totDaylightingControls);
    state->dataDayltg->daylightControl(1).DaylightMethod = Dayltg::DaylightingMethod::SplitFlux;
    state->dataDayltg->daylightControl(1).zoneIndex = 1;
    state->dataDayltg->daylightControl(1).TotalDaylRefPoints = 1;
    state->dataDayltg->ZoneDaylight(1).totRefPts = 1;
    state->dataDayltg->daylightControl(1).refPts.allocate(1);
    state->dataDayltg->daylightControl(1).PowerReductionFactor = 0.5;
    state->dataDayltg->daylightControl(1).refPts(1).lums[(int)DataSurfaces::Lum::Illum] = 300;
    state->dataDayltg->daylightControl(1).refPts(1).illumSetPoint = 400;
    state->dataOutRptTab->displayVisualResilienceSummary = true;

    int NoBins = 4;

    for (int i = 1; i <= state->dataWeather->TotVisualReportPers; i++) {
        state->dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod(1, i).assign(NoBins, 0.0);
        state->dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod(1, i).assign(NoBins, 0.0);
        state->dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod(1, i).assign(NoBins, 0.0);
    }

    state->dataOutRptTab->displayVisualResilienceSummary = true;

    state->dataHeatBal->People(1).sched->currentVal = 0;
    state->dataDayltg->daylightControl(1).refPts(1).illumSetPoint = 250;
    for (int hour = 1; hour <= 4; hour++) {
        state->dataGlobal->HourOfDay = hour;
        ReportVisualResilience(*state);
    }

    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod(1, 1)[0], 1e-8);
    EXPECT_NEAR(4.0, state->dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod(1, 1)[1], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod(1, 1)[2], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod(1, 1)[3], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod(1, 1)[0], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod(1, 1)[1], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod(1, 1)[2], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod(1, 1)[3], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod(1, 1)[0], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod(1, 1)[1], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod(1, 1)[2], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod(1, 1)[3], 1e-8);

    state->dataHeatBal->People(1).sched->currentVal = 0.4;
    state->dataDayltg->daylightControl(1).refPts(1).illumSetPoint = 600;
    for (int hour = 5; hour <= 7; hour++) {
        state->dataGlobal->HourOfDay = hour;
        ReportVisualResilience(*state);
    }

    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod(1, 1)[0], 1e-8);
    EXPECT_NEAR(4.0, state->dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod(1, 1)[1], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod(1, 1)[2], 1e-8);
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod(1, 1)[3], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod(1, 1)[0], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod(1, 1)[1], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod(1, 1)[2], 1e-8);
    EXPECT_NEAR(2.4, state->dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod(1, 1)[3], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod(1, 1)[0], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod(1, 1)[1], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod(1, 1)[2], 1e-8);
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod(1, 1)[3], 1e-8);

    state->dataHeatBal->People(1).sched->currentVal = 1.0;
    state->dataDayltg->daylightControl(1).refPts(1).illumSetPoint = 70;
    for (int hour = 8; hour <= 10; hour++) {
        state->dataGlobal->HourOfDay = hour;
        ReportVisualResilience(*state);
    }

    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod(1, 1)[0], 1e-8);
    EXPECT_NEAR(4.0, state->dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod(1, 1)[1], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod(1, 1)[2], 1e-8);
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod(1, 1)[3], 1e-8);
    EXPECT_NEAR(6.0, state->dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod(1, 1)[0], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod(1, 1)[1], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod(1, 1)[2], 1e-8);
    EXPECT_NEAR(2.4, state->dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod(1, 1)[3], 1e-8);
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod(1, 1)[0], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod(1, 1)[1], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod(1, 1)[2], 1e-8);
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod(1, 1)[3], 1e-8);

    state->dataHeatBal->People(1).sched->currentVal = 1.0;
    state->dataDayltg->daylightControl(1).refPts(1).illumSetPoint = 600;
    for (int hour = 13; hour <= 15; hour++) {
        state->dataGlobal->HourOfDay = hour;
        ReportVisualResilience(*state);
    }

    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod(1, 2)[0], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod(1, 2)[1], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod(1, 2)[2], 1e-8);
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod(1, 2)[3], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod(1, 2)[0], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod(1, 2)[1], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod(1, 2)[2], 1e-8);
    EXPECT_NEAR(6.0, state->dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod(1, 2)[3], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod(1, 2)[0], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod(1, 2)[1], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod(1, 2)[2], 1e-8);
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod(1, 2)[3], 1e-8);

    state->dataHeatBal->People(1).sched->currentVal = 1.0;
    state->dataDayltg->daylightControl(1).refPts(1).illumSetPoint = 70;
    for (int hour = 16; hour <= 18; hour++) {
        state->dataGlobal->HourOfDay = hour;
        ReportVisualResilience(*state);
    }

    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod(1, 2)[0], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod(1, 2)[1], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod(1, 2)[2], 1e-8);
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod(1, 2)[3], 1e-8);
    EXPECT_NEAR(6.0, state->dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod(1, 2)[0], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod(1, 2)[1], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod(1, 2)[2], 1e-8);
    EXPECT_NEAR(6.0, state->dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod(1, 2)[3], 1e-8);
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod(1, 2)[0], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod(1, 2)[1], 1e-8);
    EXPECT_NEAR(0.0, state->dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod(1, 2)[2], 1e-8);
    EXPECT_NEAR(3.0, state->dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod(1, 2)[3], 1e-8);
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_TestInitHBInterzoneWindow)
{

    std::string const idf_objects = delimited_string({"  Building,",
                                                      "    House with Local Air Nodes,  !- Name",
                                                      "    0,                       !- North Axis {deg}",
                                                      "    Suburbs,                 !- Terrain",
                                                      "    0.001,                   !- Loads Convergence Tolerance Value",
                                                      "    0.0050000,               !- Temperature Convergence Tolerance Value {deltaC}",
                                                      "    FullInteriorAndExterior, !- Solar Distribution",
                                                      "    25,                      !- Maximum Number of Warmup Days",
                                                      "    6;                       !- Minimum Number of Warmup Days",

                                                      "  Timestep,6;",

                                                      "  SimulationControl,",
                                                      "    No,                      !- Do Zone Sizing Calculation",
                                                      "    No,                      !- Do System Sizing Calculation",
                                                      "    No,                      !- Do Plant Sizing Calculation",
                                                      "    No,                     !- Run Simulation for Sizing Periods",
                                                      "    YES;                     !- Run Simulation for Weather File Run Periods",

                                                      "  RunPeriod,",
                                                      "    WinterDay,               !- Name",
                                                      "    1,                       !- Begin Month",
                                                      "    14,                      !- Begin Day of Month",
                                                      "    ,                        !- Begin Year",
                                                      "    1,                       !- End Month",
                                                      "    14,                      !- End Day of Month",
                                                      "    ,                        !- End Year",
                                                      "    Tuesday,                 !- Day of Week for Start Day",
                                                      "    Yes,                     !- Use Weather File Holidays and Special Days",
                                                      "    Yes,                     !- Use Weather File Daylight Saving Period",
                                                      "    No,                      !- Apply Weekend Holiday Rule",
                                                      "    Yes,                     !- Use Weather File Rain Indicators",
                                                      "    Yes;                     !- Use Weather File Snow Indicators",

                                                      "  Material,",
                                                      "    A1 - 1 IN STUCCO,        !- Name",
                                                      "    Smooth,                  !- Roughness",
                                                      "    2.5389841E-02,           !- Thickness {m}",
                                                      "    0.6918309,               !- Conductivity {W/m-K}",
                                                      "    1858.142,                !- Density {kg/m3}",
                                                      "    836.8000,                !- Specific Heat {J/kg-K}",
                                                      "    0.9000000,               !- Thermal Absorptance",
                                                      "    0.9200000,               !- Solar Absorptance",
                                                      "    0.9200000;               !- Visible Absorptance",

                                                      "  Material,",
                                                      "    CB11,                    !- Name",
                                                      "    MediumRough,             !- Roughness",
                                                      "    0.2032000,               !- Thickness {m}",
                                                      "    1.048000,                !- Conductivity {W/m-K}",
                                                      "    1105.000,                !- Density {kg/m3}",
                                                      "    837.0000,                !- Specific Heat {J/kg-K}",
                                                      "    0.9000000,               !- Thermal Absorptance",
                                                      "    0.2000000,               !- Solar Absorptance",
                                                      "    0.2000000;               !- Visible Absorptance",

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
                                                      "    IN05,                    !- Name",
                                                      "    Rough,                   !- Roughness",
                                                      "    0.2458000,               !- Thickness {m}",
                                                      "    4.3000001E-02,           !- Conductivity {W/m-K}",
                                                      "    10.00000,                !- Density {kg/m3}",
                                                      "    837.0000,                !- Specific Heat {J/kg-K}",
                                                      "    0.9000000,               !- Thermal Absorptance",
                                                      "    0.7500000,               !- Solar Absorptance",
                                                      "    0.7500000;               !- Visible Absorptance",

                                                      "  Material,",
                                                      "    PW03,                    !- Name",
                                                      "    MediumSmooth,            !- Roughness",
                                                      "    1.2700000E-02,           !- Thickness {m}",
                                                      "    0.1150000,               !- Conductivity {W/m-K}",
                                                      "    545.0000,                !- Density {kg/m3}",
                                                      "    1213.000,                !- Specific Heat {J/kg-K}",
                                                      "    0.9000000,               !- Thermal Absorptance",
                                                      "    0.7800000,               !- Solar Absorptance",
                                                      "    0.7800000;               !- Visible Absorptance",

                                                      "  Material,",
                                                      "    CC03,                    !- Name",
                                                      "    MediumRough,             !- Roughness",
                                                      "    0.1016000,               !- Thickness {m}",
                                                      "    1.310000,                !- Conductivity {W/m-K}",
                                                      "    2243.000,                !- Density {kg/m3}",
                                                      "    837.0000,                !- Specific Heat {J/kg-K}",
                                                      "    0.9000000,               !- Thermal Absorptance",
                                                      "    0.6500000,               !- Solar Absorptance",
                                                      "    0.6500000;               !- Visible Absorptance",

                                                      "  Material,",
                                                      "    HF-A3,                   !- Name",
                                                      "    Smooth,                  !- Roughness",
                                                      "    1.5000000E-03,           !- Thickness {m}",
                                                      "    44.96960,                !- Conductivity {W/m-K}",
                                                      "    7689.000,                !- Density {kg/m3}",
                                                      "    418.0000,                !- Specific Heat {J/kg-K}",
                                                      "    0.9000000,               !- Thermal Absorptance",
                                                      "    0.2000000,               !- Solar Absorptance",
                                                      "    0.2000000;               !- Visible Absorptance",

                                                      "  Material:NoMass,",
                                                      "    AR02,                    !- Name",
                                                      "    VeryRough,               !- Roughness",
                                                      "    7.8000002E-02,           !- Thermal Resistance {m2-K/W}",
                                                      "    0.9000000,               !- Thermal Absorptance",
                                                      "    0.7000000,               !- Solar Absorptance",
                                                      "    0.7000000;               !- Visible Absorptance",

                                                      "  Material:NoMass,",
                                                      "    CP02,                    !- Name",
                                                      "    Rough,                   !- Roughness",
                                                      "    0.2170000,               !- Thermal Resistance {m2-K/W}",
                                                      "    0.9000000,               !- Thermal Absorptance",
                                                      "    0.7500000,               !- Solar Absorptance",
                                                      "    0.7500000;               !- Visible Absorptance",

                                                      "  Construction,",
                                                      "    EXTWALL:LIVING,          !- Name",
                                                      "    A1 - 1 IN STUCCO,        !- Outside Layer",
                                                      "    GP01;                    !- Layer 3",

                                                      "  Construction,",
                                                      "    FLOOR:LIVING,            !- Name",
                                                      "    CC03,                    !- Outside Layer",
                                                      "    CP02;                    !- Layer 2",

                                                      "  Construction,",
                                                      "    ROOF,                    !- Name",
                                                      "    AR02,                    !- Outside Layer",
                                                      "    PW03;                    !- Layer 2",

                                                      "  Zone,",
                                                      "    LIVING ZONE,             !- Name",
                                                      "    0,                       !- Direction of Relative North {deg}",
                                                      "    0,                       !- X Origin {m}",
                                                      "    0,                       !- Y Origin {m}",
                                                      "    0,                       !- Z Origin {m}",
                                                      "    1,                       !- Type",
                                                      "    1,                       !- Multiplier",
                                                      "    autocalculate,           !- Ceiling Height {m}",
                                                      "    autocalculate;           !- Volume {m3}",

                                                      "  GlobalGeometryRules,",
                                                      "    UpperLeftCorner,         !- Starting Vertex Position",
                                                      "    CounterClockWise,        !- Vertex Entry Direction",
                                                      "    World;                   !- Coordinate System",

                                                      "  BuildingSurface:Detailed,",
                                                      "    Living:North,            !- Name",
                                                      "    Wall,                    !- Surface Type",
                                                      "    EXTWALL:LIVING,          !- Construction Name",
                                                      "    LIVING ZONE,             !- Zone Name",
                                                      "    ,                        !- Space Name",
                                                      "    Outdoors,                !- Outside Boundary Condition",
                                                      "    ,                        !- Outside Boundary Condition Object",
                                                      "    SunExposed,              !- Sun Exposure",
                                                      "    WindExposed,             !- Wind Exposure",
                                                      "    0.5000000,               !- View Factor to Ground",
                                                      "    4,                       !- Number of Vertices",
                                                      "    1,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
                                                      "    1,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
                                                      "    0,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
                                                      "    0,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

                                                      "  BuildingSurface:Detailed,",
                                                      "    Living:East,             !- Name",
                                                      "    Wall,                    !- Surface Type",
                                                      "    EXTWALL:LIVING,          !- Construction Name",
                                                      "    LIVING ZONE,             !- Zone Name",
                                                      "    ,                        !- Space Name",
                                                      "    Outdoors,                !- Outside Boundary Condition",
                                                      "    ,                        !- Outside Boundary Condition Object",
                                                      "    SunExposed,              !- Sun Exposure",
                                                      "    WindExposed,             !- Wind Exposure",
                                                      "    0.5000000,               !- View Factor to Ground",
                                                      "    4,                       !- Number of Vertices",
                                                      "    1,0,1,  !- X,Y,Z ==> Vertex 1 {m}",
                                                      "    1,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
                                                      "    1,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
                                                      "    1,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

                                                      "  BuildingSurface:Detailed,",
                                                      "    Living:South,            !- Name",
                                                      "    Wall,                    !- Surface Type",
                                                      "    EXTWALL:LIVING,          !- Construction Name",
                                                      "    LIVING ZONE,             !- Zone Name",
                                                      "    ,                        !- Space Name",
                                                      "    Outdoors,                !- Outside Boundary Condition",
                                                      "    ,                        !- Outside Boundary Condition Object",
                                                      "    SunExposed,              !- Sun Exposure",
                                                      "    WindExposed,             !- Wind Exposure",
                                                      "    0.5000000,               !- View Factor to Ground",
                                                      "    4,                       !- Number of Vertices",
                                                      "    0,0,1,  !- X,Y,Z ==> Vertex 1 {m}",
                                                      "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
                                                      "    1,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
                                                      "    1,0,1;  !- X,Y,Z ==> Vertex 4 {m}",

                                                      "  BuildingSurface:Detailed,",
                                                      "    Living:West,             !- Name",
                                                      "    Wall,                    !- Surface Type",
                                                      "    EXTWALL:LIVING,          !- Construction Name",
                                                      "    LIVING ZONE,             !- Zone Name",
                                                      "    ,                        !- Space Name",
                                                      "    Outdoors,                !- Outside Boundary Condition",
                                                      "    ,                        !- Outside Boundary Condition Object",
                                                      "    SunExposed,              !- Sun Exposure",
                                                      "    WindExposed,             !- Wind Exposure",
                                                      "    0.5000000,               !- View Factor to Ground",
                                                      "    4,                       !- Number of Vertices",
                                                      "    0,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
                                                      "    0,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
                                                      "    0,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
                                                      "    0,0,1;  !- X,Y,Z ==> Vertex 4 {m}",

                                                      "  BuildingSurface:Detailed,",
                                                      "    Living:Floor,            !- Name",
                                                      "    FLOOR,                   !- Surface Type",
                                                      "    FLOOR:LIVING,            !- Construction Name",
                                                      "    LIVING ZONE,             !- Zone Name",
                                                      "    ,                        !- Space Name",
                                                      "    Surface,                 !- Outside Boundary Condition",
                                                      "    Living:Floor,            !- Outside Boundary Condition Object",
                                                      "    NoSun,                   !- Sun Exposure",
                                                      "    NoWind,                  !- Wind Exposure",
                                                      "    0,                       !- View Factor to Ground",
                                                      "    4,                       !- Number of Vertices",
                                                      "    0,0,0,  !- X,Y,Z ==> Vertex 1 {m}",
                                                      "    0,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
                                                      "    1,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
                                                      "    1,0,0;  !- X,Y,Z ==> Vertex 4 {m}",

                                                      "  BuildingSurface:Detailed,",
                                                      "    Living:Ceiling,          !- Name",
                                                      "    ROOF,                 !- Surface Type",
                                                      "    ROOF,          !- Construction Name",
                                                      "    LIVING ZONE,             !- Zone Name",
                                                      "    ,                        !- Space Name",
                                                      "    Outdoors,                !- Outside Boundary Condition",
                                                      "    ,                        !- Outside Boundary Condition Object",
                                                      "    SunExposed,              !- Sun Exposure",
                                                      "    WindExposed,             !- Wind Exposure",
                                                      "    0,                       !- View Factor to Ground",
                                                      "    4,                       !- Number of Vertices",
                                                      "    0,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
                                                      "    0,0,1,  !- X,Y,Z ==> Vertex 2 {m}",
                                                      "    1,0,1,  !- X,Y,Z ==> Vertex 3 {m}",
                                                      "    1,1,1;  !- X,Y,Z ==> Vertex 4 {m}"});

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound = false;
    HeatBalanceManager::GetProjectControlData(*state, ErrorsFound); // read project control data
    EXPECT_FALSE(ErrorsFound);                                      // expect no errors
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    Material::GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    SurfaceGeometry::GetGeometryParameters(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    state->dataSurfaceGeometry->CosBldgRotAppGonly = 1.0;
    state->dataSurfaceGeometry->SinBldgRotAppGonly = 0.0;
    SurfaceGeometry::SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    HeatBalanceIntRadExchange::InitSolarViewFactors(*state);
    EXPECT_FALSE(has_err_output(true));

    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);

    state->dataHeatBalSurf->SurfTempInTmp.allocate(6);

    state->dataHeatBalSurf->SurfHConvInt.allocate(6);
    state->dataMstBal->HConvInFD.allocate(6);
    state->dataMstBal->RhoVaporAirIn.allocate(6);
    state->dataMstBal->HMassConvInFD.allocate(6);

    state->dataGlobal->BeginSimFlag = true;
    state->dataGlobal->KickOffSimulation = true;
    state->dataGlobal->TimeStepZoneSec = 900;
    state->dataHeatBal->ZoneWinHeatGain.allocate(1);
    state->dataHeatBal->ZoneWinHeatGainRep.allocate(1);
    state->dataHeatBal->ZoneWinHeatGainRepEnergy.allocate(1);

    AllocateSurfaceHeatBalArrays(*state);
    createFacilityElectricPowerServiceObject(*state);
    HeatBalanceManager::AllocateZoneHeatBalArrays(*state);
    SolarShading::AllocateModuleArrays(*state);
    SolarShading::DetermineShadowingCombinations(*state);

    InitSurfaceHeatBalance(*state);

    EXPECT_FALSE(state->dataHeatBalSurf->InterZoneWindow);
    EXPECT_FALSE(allocated(state->dataHeatBalSurf->ZoneFractDifShortZtoZ));

    state->dataGlobal->BeginSimFlag = false;
    state->dataHeatBalSurf->InterZoneWindow = true;
    InitSurfaceHeatBalance(*state);

    EXPECT_TRUE(allocated(state->dataHeatBalSurf->ZoneFractDifShortZtoZ));
    EXPECT_EQ(1, state->dataHeatBalSurf->ZoneFractDifShortZtoZ(1, 1));

    // bypass internal solar distribution at night
    state->dataHeatBalSurf->InterZoneWindow = false;
    state->dataHeatBal->ZoneBmSolFrIntWinsRep(1) = 10.0;
    state->dataEnvrn->SunIsUp = false;
    InitIntSolarDistribution(*state);
    EXPECT_EQ(0.0, state->dataHeatBal->SurfIntBmIncInsSurfIntensRep(1));
    state->dataEnvrn->SunIsUp = true;
    InitIntSolarDistribution(*state);
    EXPECT_NEAR(1.666667, state->dataHeatBal->SurfIntBmIncInsSurfIntensRep(1), 0.00001);
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_IncSolarMultiplier)
{
    //    test the incident solar multiplier application
    state->dataSurface->TotSurfaces = 1;
    state->dataViewFactor->NumOfSolarEnclosures = 1;
    int totSurf = state->dataSurface->TotSurfaces;
    state->dataHeatBal->TotConstructs = 1;
    int totConstructs = state->dataHeatBal->TotConstructs;
    state->dataSurface->Surface.allocate(totSurf);
    state->dataSurface->surfShades.allocate(totSurf);
    state->dataConstruction->Construct.allocate(totConstructs);
    state->dataGlobal->TimeStepZoneSec = 900;
    state->dataGlobal->TimeStepsInHour = 6;
    state->dataGlobal->HourOfDay = 1;
    state->dataGlobal->TimeStep = 1;

    int SurfNum = 1;
    state->dataSurface->Surface(SurfNum).Class = DataSurfaces::SurfaceClass::Window;
    state->dataSurface->Surface(SurfNum).hasIncSolMultiplier = true;
    state->dataSurface->Surface(SurfNum).ExtSolar = true;
    state->dataSurface->Surface(SurfNum).BaseSurf = SurfNum;
    state->dataSurface->SurfIncSolMultiplier.allocate(totSurf);
    state->dataSurface->SurfIncSolMultiplier(SurfNum).Name = "testing window surface";
    state->dataSurface->SurfIncSolMultiplier(SurfNum).SurfaceIdx = SurfNum;
    state->dataSurface->SurfIncSolMultiplier(SurfNum).sched = nullptr;

    state->dataSurface->Surface(SurfNum).Area = 100.0;

    Window::initWindowModel(*state);
    SurfaceGeometry::AllocateSurfaceWindows(*state, SurfNum);
    state->dataGlobal->numSpaces = 1;
    state->dataViewFactor->EnclSolInfo.allocate(state->dataGlobal->numSpaces);
    auto &thisEnclosure(state->dataViewFactor->EnclSolInfo(1));
    thisEnclosure.Name = "test enclosure";
    thisEnclosure.SurfacePtr.dimension(1, 0);
    thisEnclosure.SurfacePtr(1) = 1;

    int ConstrNum = 1;
    state->dataSurface->Surface(SurfNum).Construction = ConstrNum;
    auto &constr = state->dataConstruction->Construct(ConstrNum);

    state->dataSurface->SurfActiveConstruction(SurfNum) = state->dataSurface->Surface(SurfNum).Construction;
    state->dataConstruction->Construct(ConstrNum).TransDiff = 0.1;
    // Why is this being written and immediately overwritten?
    std::fill(constr.TransSolBeamCoef.begin(), constr.TransSolBeamCoef.end(), 0.1);
    std::fill(constr.TransSolBeamCoef.begin(), constr.TransSolBeamCoef.end(), 0.2);

    state->dataSurface->SurfaceWindow.allocate(totSurf);
    state->dataSurface->SurfaceWindow(SurfNum).OutProjSLFracMult[state->dataGlobal->HourOfDay] = 999.0;
    state->dataSurface->SurfaceWindow(SurfNum).InOutProjSLFracMult[state->dataGlobal->HourOfDay] = 888.0;

    SolarShading::AllocateModuleArrays(*state);
    state->dataHeatBal->SurfSunlitFrac = 1.0;
    state->dataHeatBal->SurfCosIncAng = 1.0;
    SurfaceGeometry::AllocateSurfaceArrays(*state);

    state->dataSolarShading->SurfWinTransBmSolar(1) = 0.8;
    state->dataSolarShading->SurfWinExtBeamAbsByShadFac(1) = 0.0;
    state->dataSurface->Surface(1).ViewFactorGround = 1.0;
    state->dataSurface->SurfWinShadingFlag(1) = DataSurfaces::WinShadingType::NoShade;

    state->dataEnvrn->SunIsUp = true;
    state->dataEnvrn->BeamSolarRad = 0.1;
    state->dataEnvrn->GndSolarRad = 0.2;
    state->dataEnvrn->DifSolarRad = 0.3;
    state->dataEnvrn->SOLCOS(1) = 0.84471127222777276;
    state->dataEnvrn->SOLCOS(2) = -0.53484539135440257;
    state->dataEnvrn->SOLCOS(3) = 0.020081681162033127;

    HeatBalanceManager::AllocateZoneHeatBalArrays(*state);

    state->dataSurface->SurfIncSolMultiplier(SurfNum).Scaler = 0.5;
    HeatBalanceSurfaceManager::InitSolarHeatGains(*state);
    Real64 transmittedSolarHalf = state->dataSurface->SurfWinTransSolar(1);
    state->dataSurface->SurfIncSolMultiplier(SurfNum).Scaler = 1.0;
    HeatBalanceSurfaceManager::InitSolarHeatGains(*state);
    Real64 transmittedSolarWhole = state->dataSurface->SurfWinTransSolar(1);
    EXPECT_EQ(transmittedSolarWhole * 0.5, transmittedSolarHalf);
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_TestInitHBDaylightingNoExtWindow)
{

    std::string const idf_objects1 = R"IDF(
    Building,
        Building,                !- Name
        0,                       !- North Axis {deg}
        Suburbs,                 !- Terrain
        0.04,                    !- Loads Convergence Tolerance Value {W}
        0.4,                     !- Temperature Convergence Tolerance Value {deltaC}
        FullExterior,            !- Solar Distribution
        25,                      !- Maximum Number of Warmup Days
        6;                       !- Minimum Number of Warmup Days

    Timestep,6;

    SimulationControl,
        No,                      !- Do Zone Sizing Calculation
        No,                      !- Do System Sizing Calculation
        No,                      !- Do Plant Sizing Calculation
        No,                     !- Run Simulation for Sizing Periods
        YES;                     !- Run Simulation for Weather File Run Periods

    RunPeriod,
        TESTSim,                 !- Name
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
        Yes,                     !- Use Weather File Snow Indicators
        No;                      !- Treat Weather as Actual

    Site:GroundTemperature:BuildingSurface,
        18,                      !- January Ground Temperature {C}
        18,                      !- February Ground Temperature {C}
        18,                      !- March Ground Temperature {C}
        18,                      !- April Ground Temperature {C}
        18,                      !- May Ground Temperature {C}
        18,                      !- June Ground Temperature {C}
        18,                      !- July Ground Temperature {C}
        18,                      !- August Ground Temperature {C}
        18,                      !- September Ground Temperature {C}
        18,                      !- October Ground Temperature {C}
        18,                      !- November Ground Temperature {C}
        18;                      !- December Ground Temperature {C}

    ! Materials
    Material,
        defaultMat_0.25_defaultConstruction ,  !- Name
        Rough,                   !- Roughness
        0.25,                    !- Thickness {m}
        2.3,                     !- Conductivity {W/m-K}
        2400,                    !- Density {kg/m3}
        840,                     !- Specific Heat {J/kg-K}
        0.9,                     !- Thermal Absorptance
        0.7,                     !- Solar Absorptance
        0.7;                     !- Visible Absorptance

    ! Materials
    Material,
        Mineral Wool_0.194_UVal_0.2_Mass ,  !- Name
        Rough,                   !- Roughness
        0.194,                   !- Thickness {m}
        0.041,                   !- Conductivity {W/m-K}
        155,                     !- Density {kg/m3}
        1130,                    !- Specific Heat {J/kg-K}
        0.9,                     !- Thermal Absorptance
        0.7,                     !- Solar Absorptance
        0.7;                     !- Visible Absorptance

    Material,
        General Concrete_0.2_UVal_0.2_Mass ,  !- Name
        Rough,                   !- Roughness
        0.2,                     !- Thickness {m}
        2,                       !- Conductivity {W/m-K}
        2400,                    !- Density {kg/m3}
        950,                     !- Specific Heat {J/kg-K}
        0.9,                     !- Thermal Absorptance
        0.7,                     !- Solar Absorptance
        0.7;                     !- Visible Absorptance

    Construction,
        defaultConstruction,     !- Name
        defaultMat_0.25_defaultConstruction;  !- Outside Layer

    Construction,
        defaultConstruction_FLIPPED,  !- Name
        defaultMat_0.25_defaultConstruction;  !- Outside Layer

    ! UVal_0.2_Mass Type: Facade
    Construction,
        UVal_0.2_Mass,           !- Name
        Mineral Wool_0.194_UVal_0.2_Mass,  !- Outside Layer
        General Concrete_0.2_UVal_0.2_Mass;  !- Layer 2

    Construction,
        UVal_0.2_Mass_FLIPPED,   !- Name
        General Concrete_0.2_UVal_0.2_Mass,  !- Outside Layer
        Mineral Wool_0.194_UVal_0.2_Mass;  !- Layer 2

    Zone,
        Zone_0,                  !- Name
        0,                       !- Direction of Relative North {deg}
        0,                       !- X Origin {m}
        0,                       !- Y Origin {m}
        0,                       !- Z Origin {m}
        1,                       !- Type
        1,                       !- Multiplier
        ,                        !- Ceiling Height {m}
        8197.0573178193,         !- Volume {m3}
        631.643058536491,        !- Floor Area {m2}
        TARP,                    !- Zone Inside Convection Algorithm
        DOE-2;                   !- Zone Outside Convection Algorithm

    GlobalGeometryRules,
        LowerLeftCorner,         !- Starting Vertex Position
        CounterClockWise,        !- Vertex Entry Direction
        Relative;                !- Coordinate System

    BuildingSurface:Detailed,
        Zone_0:f0,               !- Name
        Wall,                    !- Surface Type
        UVal_0.2_Mass,           !- Construction Name
        Zone_0,                  !- Zone Name
        ,                        !- Space Name
        Outdoors,                !- Outside Boundary Condition
        ,                        !- Outside Boundary Condition Object
        SunExposed,              !- Sun Exposure
        WindExposed,             !- Wind Exposure
        Autocalculate,           !- View Factor to Ground
        4,                       !- Number of Vertices
        -2.6475,                 !- Vertex 1 X-coordinate {m}
        16.8426,                 !- Vertex 1 Y-coordinate {m}
        0.0000,                  !- Vertex 1 Z-coordinate {m}
        24.7463,                 !- Vertex 2 X-coordinate {m}
        16.8426,                 !- Vertex 2 Y-coordinate {m}
        0.0000,                  !- Vertex 2 Z-coordinate {m}
        24.7463,                 !- Vertex 3 X-coordinate {m}
        16.8426,                 !- Vertex 3 Y-coordinate {m}
        12.9774,                 !- Vertex 3 Z-coordinate {m}
        -2.6475,                 !- Vertex 4 X-coordinate {m}
        16.8426,                 !- Vertex 4 Y-coordinate {m}
        12.9774;                 !- Vertex 4 Z-coordinate {m}

    ! FACE NORMAL  1 3.46944695195361E-17 0
    BuildingSurface:Detailed,
        Zone_0:f1,               !- Name
        Wall,                    !- Surface Type
        UVal_0.2_Mass,           !- Construction Name
        Zone_0,                  !- Zone Name
        ,                        !- Space Name
        Outdoors,                !- Outside Boundary Condition
        ,                        !- Outside Boundary Condition Object
        SunExposed,              !- Sun Exposure
        WindExposed,             !- Wind Exposure
        Autocalculate,           !- View Factor to Ground
        4,                       !- Number of Vertices
        24.7463,                 !- Vertex 1 X-coordinate {m}
        16.8426,                 !- Vertex 1 Y-coordinate {m}
        0.0000,                  !- Vertex 1 Z-coordinate {m}
        24.7463,                 !- Vertex 2 X-coordinate {m}
        39.9005,                 !- Vertex 2 Y-coordinate {m}
        0.0000,                  !- Vertex 2 Z-coordinate {m}
        24.7463,                 !- Vertex 3 X-coordinate {m}
        39.9005,                 !- Vertex 3 Y-coordinate {m}
        12.9774,                 !- Vertex 3 Z-coordinate {m}
        24.7463,                 !- Vertex 4 X-coordinate {m}
        16.8426,                 !- Vertex 4 Y-coordinate {m}
        12.9774;                 !- Vertex 4 Z-coordinate {m}

    ! FACE NORMAL  -4.16333634234434E-17 1 0
    BuildingSurface:Detailed,
        Zone_0:f2,               !- Name
        Wall,                    !- Surface Type
        defaultConstruction,     !- Construction Name
        Zone_0,                  !- Zone Name
        ,                        !- Space Name
        Surface,                 !- Outside Boundary Condition
        Zone_0:f2,               !- Outside Boundary Condition Object
        NoSun,                   !- Sun Exposure
        NoWind,                  !- Wind Exposure
        Autocalculate,           !- View Factor to Ground
        4,                       !- Number of Vertices
        24.7463,                 !- Vertex 1 X-coordinate {m}
        39.9005,                 !- Vertex 1 Y-coordinate {m}
        0.0000,                  !- Vertex 1 Z-coordinate {m}
        -2.6475,                 !- Vertex 2 X-coordinate {m}
        39.9005,                 !- Vertex 2 Y-coordinate {m}
        0.0000,                  !- Vertex 2 Z-coordinate {m}
        -2.6475,                 !- Vertex 3 X-coordinate {m}
        39.9005,                 !- Vertex 3 Y-coordinate {m}
        12.9774,                 !- Vertex 3 Z-coordinate {m}
        24.7463,                 !- Vertex 4 X-coordinate {m}
        39.9005,                 !- Vertex 4 Y-coordinate {m}
        12.9774;                 !- Vertex 4 Z-coordinate {m}

    ! FACE NORMAL  -1 6.07153216591882E-18 0
    BuildingSurface:Detailed,
        Zone_0:f3,               !- Name
        Wall,                    !- Surface Type
        UVal_0.2_Mass,           !- Construction Name
        Zone_0,                  !- Zone Name
        ,                        !- Space Name
        Outdoors,                !- Outside Boundary Condition
        ,                        !- Outside Boundary Condition Object
        SunExposed,              !- Sun Exposure
        WindExposed,             !- Wind Exposure
        Autocalculate,           !- View Factor to Ground
        4,                       !- Number of Vertices
        -2.6475,                 !- Vertex 1 X-coordinate {m}
        39.9005,                 !- Vertex 1 Y-coordinate {m}
        0.0000,                  !- Vertex 1 Z-coordinate {m}
        -2.6475,                 !- Vertex 2 X-coordinate {m}
        16.8426,                 !- Vertex 2 Y-coordinate {m}
        0.0000,                  !- Vertex 2 Z-coordinate {m}
        -2.6475,                 !- Vertex 3 X-coordinate {m}
        16.8426,                 !- Vertex 3 Y-coordinate {m}
        12.9774,                 !- Vertex 3 Z-coordinate {m}
        -2.6475,                 !- Vertex 4 X-coordinate {m}
        39.9005,                 !- Vertex 4 Y-coordinate {m}
        12.9774;                 !- Vertex 4 Z-coordinate {m}

    ! FACE NORMAL  0 0 -1
    BuildingSurface:Detailed,
        Zone_0:f4,               !- Name
        Floor,                   !- Surface Type
        defaultConstruction,     !- Construction Name
        Zone_0,                  !- Zone Name
        ,                        !- Space Name
        Surface,                 !- Outside Boundary Condition
        Zone_0:f4,               !- Outside Boundary Condition Object
        NoSun,                   !- Sun Exposure
        NoWind,                  !- Wind Exposure
        Autocalculate,           !- View Factor to Ground
        4,                       !- Number of Vertices
        -2.6475,                 !- Vertex 1 X-coordinate {m}
        39.9005,                 !- Vertex 1 Y-coordinate {m}
        0.0000,                  !- Vertex 1 Z-coordinate {m}
        24.7463,                 !- Vertex 2 X-coordinate {m}
        39.9005,                 !- Vertex 2 Y-coordinate {m}
        0.0000,                  !- Vertex 2 Z-coordinate {m}
        24.7463,                 !- Vertex 3 X-coordinate {m}
        30.1033,                 !- Vertex 3 Y-coordinate {m}
        0.0000,                  !- Vertex 3 Z-coordinate {m}
        -2.6475,                 !- Vertex 4 X-coordinate {m}
        30.1033,                 !- Vertex 4 Y-coordinate {m}
        0.0000;                  !- Vertex 4 Z-coordinate {m}

    ! FACE NORMAL  0 0 1
    BuildingSurface:Detailed,
        Zone_0:f5,               !- Name
        Roof,                    !- Surface Type
        UVal_0.2_Mass,           !- Construction Name
        Zone_0,                  !- Zone Name
        ,                        !- Space Name
        Outdoors,                !- Outside Boundary Condition
        ,                        !- Outside Boundary Condition Object
        SunExposed,              !- Sun Exposure
        WindExposed,             !- Wind Exposure
        0,                       !- View Factor to Ground
        4,                       !- Number of Vertices
        24.7463,                 !- Vertex 1 X-coordinate {m}
        16.8426,                 !- Vertex 1 Y-coordinate {m}
        12.9774,                 !- Vertex 1 Z-coordinate {m}
        24.7463,                 !- Vertex 2 X-coordinate {m}
        39.9005,                 !- Vertex 2 Y-coordinate {m}
        12.9774,                 !- Vertex 2 Z-coordinate {m}
        -2.6475,                 !- Vertex 3 X-coordinate {m}
        39.9005,                 !- Vertex 3 Y-coordinate {m}
        12.9774,                 !- Vertex 3 Z-coordinate {m}
        -2.6475,                 !- Vertex 4 X-coordinate {m}
        16.8426,                 !- Vertex 4 Y-coordinate {m}
        12.9774;                 !- Vertex 4 Z-coordinate {m}

    ! FACE NORMAL  0 0 -1
    BuildingSurface:Detailed,
        Zone_0:f6,               !- Name
        Floor,                   !- Surface Type
        defaultConstruction,     !- Construction Name
        Zone_0,                  !- Zone Name
        ,                        !- Space Name
        Surface,                 !- Outside Boundary Condition
        Zone_0:f6,               !- Outside Boundary Condition Object
        NoSun,                   !- Sun Exposure
        NoWind,                  !- Wind Exposure
        Autocalculate,           !- View Factor to Ground
        4,                       !- Number of Vertices
        24.7463,                 !- Vertex 1 X-coordinate {m}
        30.1033,                 !- Vertex 1 Y-coordinate {m}
        0.0000,                  !- Vertex 1 Z-coordinate {m}
        24.7463,                 !- Vertex 2 X-coordinate {m}
        16.8426,                 !- Vertex 2 Y-coordinate {m}
        0.0000,                  !- Vertex 2 Z-coordinate {m}
        -2.6475,                 !- Vertex 3 X-coordinate {m}
        16.8426,                 !- Vertex 3 Y-coordinate {m}
        0.0000,                  !- Vertex 3 Z-coordinate {m}
        -2.6475,                 !- Vertex 4 X-coordinate {m}
        30.1033,                 !- Vertex 4 Y-coordinate {m}
        0.0000;                  !- Vertex 4 Z-coordinate {m}
    )IDF";

    std::string const idf_objects2 = R"IDF(
    Daylighting:Controls,
        Zone_0_DaylCtrl,         !- Name
        Zone_0,                  !- Zone Name
        SplitFlux,               !- Daylighting Method
        lightsOffice,            !- Availability Schedule Name
        Continuous,              !- Lighting Control Type
        0.1,                     !- Minimum Input Power Fraction for Continuous or ContinuousOff Dimming Control
        0.1,                     !- Minimum Light Output Fraction for Continuous or ContinuousOff Dimming Control
        3,                       !- Number of Stepped Control Steps
        1,                       !- Probability Lighting will be Reset When Needed in Manual Stepped Control
        ,                        !- Glare Calculation Daylighting Reference Point Name
        ,                        !- Glare Calculation Azimuth Angle of View Direction Clockwise from Zone y-Axis {deg}
        22,                      !- Maximum Allowable Discomfort Glare Index
        ,                        !- DElight Gridding Resolution {m2}
        Zone_0_DaylCtrlRefPt1,   !- Daylighting Reference Point 1 Name
        1,                       !- Fraction of Zone Controlled by Reference Point 1
        500;                     !- Illuminance Setpoint at Reference Point 1 {lux}

    Daylighting:ReferencePoint,
        Zone_0_DaylCtrlRefPt1,   !- Name
        Zone_0,                  !- Zone Name
        11.0493979164892,        !- X-Coordinate of Reference Point {m}
        28.3715348505495,        !- Y-Coordinate of Reference Point {m}
        0.85;                    !- Z-Coordinate of Reference Point {m}
    )IDF";

    std::string const idf_objects = idf_objects1 + idf_objects2;
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound = false;
    HeatBalanceManager::GetProjectControlData(*state, ErrorsFound); // read project control data
    EXPECT_FALSE(ErrorsFound);                                      // expect no errors
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    Material::GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    SurfaceGeometry::GetGeometryParameters(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    SurfaceGeometry::SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);

    state->dataHeatBalSurf->SurfTempInTmp.allocate(6);

    state->dataHeatBalSurf->SurfHConvInt.allocate(6);
    state->dataMstBal->HConvInFD.allocate(6);
    state->dataMstBal->RhoVaporAirIn.allocate(6);
    state->dataMstBal->HMassConvInFD.allocate(6);

    state->dataGlobal->BeginSimFlag = true;
    state->dataGlobal->KickOffSimulation = true;
    state->dataGlobal->TimeStepZoneSec = 900;
    state->dataGlobal->TimeStepsInHour = 6;
    state->dataGlobal->HourOfDay = 1;
    state->dataGlobal->TimeStep = 1;

    AllocateSurfaceHeatBalArrays(*state);
    createFacilityElectricPowerServiceObject(*state);
    HeatBalanceManager::AllocateZoneHeatBalArrays(*state);
    SolarShading::AllocateModuleArrays(*state);
    SolarShading::DetermineShadowingCombinations(*state);

    state->dataEnvrn->SunIsUp = true;
    state->dataEnvrn->BeamSolarRad = 50;
    state->dataEnvrn->GndSolarRad = 50;
    state->dataEnvrn->DifSolarRad = 0;
    state->dataEnvrn->SOLCOS(1) = 0.84471127222777276;
    state->dataEnvrn->SOLCOS(2) = -0.53484539135440257;
    state->dataEnvrn->SOLCOS(3) = 0.020081681162033127;

    for (auto &thisSurf : state->dataSurface->Surface) {
        thisSurf.SolarEnclIndex = 1;
        thisSurf.RadEnclIndex = 1;
    }
    state->dataViewFactor->EnclSolInfo(1).TotalEnclosureDaylRefPoints = 1;
    state->dataDayltg->enclDaylight.allocate(1);
    state->dataDayltg->enclDaylight(1).hasSplitFluxDaylighting = true;
    InitSurfaceHeatBalance(*state);
    EXPECT_FALSE(has_err_output(true));
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_TestTDDSurfWinHeatGain)
{

    std::string const idf_objects = delimited_string({
        "  Zone,",
        "    Daylit Zone,             !- Name",
        "    0.0,                     !- Direction of Relative North {deg}",
        "    0.0,                     !- X Origin {m}",
        "    0.0,                     !- Y Origin {m}",
        "    0.0,                     !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  BuildingSurface:Detailed,",
        "    Daylit South Wall,       !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Daylit Zone,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0,0.0,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,0.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5.0,0.0,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    5.0,0.0,2.5;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Daylit West Wall,        !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Daylit Zone,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0,10.0,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,10.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,0.0,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0.0,0.0,2.5;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Daylit North Wall,       !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Daylit Zone,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    5.0,10.0,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5.0,10.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,10.0,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0.0,10.0,2.5;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Daylit East Wall,        !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Daylit Zone,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    5.0,0.0,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5.0,0.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5.0,10.0,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    5.0,10.0,2.5;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Daylit Floor,            !- Name",
        "    Floor,                   !- Surface Type",
        "    FLOOR SLAB 8 IN,         !- Construction Name",
        "    Daylit Zone,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Daylit Floor,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    1.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0,0.0,0.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,10.0,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5.0,10.0,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    5.0,0.0,0.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Daylit Ceiling,          !- Name",
        "    Roof,                    !- Surface Type",
        "    CEILING IN ZONE,         !- Construction Name",
        "    Daylit Zone,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Daylit Attic Floor,      !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0,10.0,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,0.0,2.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5.0,0.0,2.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    5.0,10.0,2.5;  !- X,Y,Z ==> Vertex 4 {m}",

        "  Zone,",
        "    Daylit Attic Zone,       !- Name",
        "    0.0,                     !- Direction of Relative North {deg}",
        "    0.0,                     !- X Origin {m}",
        "    0.0,                     !- Y Origin {m}",
        "    0.0,                     !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  BuildingSurface:Detailed,",
        "    Daylit Attic South Wall, !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Daylit Attic Zone,       !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0,0.0,3.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,0.0,2.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5.0,0.0,2.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    5.0,0.0,3.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Daylit Attic West Wall,  !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Daylit Attic Zone,       !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0,10.0,5.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,10.0,2.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,0.0,2.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0.0,0.0,3.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Daylit Attic North Wall, !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Daylit Attic Zone,       !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    5.0,10.0,5.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5.0,10.0,2.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,10.0,2.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0.0,10.0,5.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Daylit Attic East Wall,  !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL80,               !- Construction Name",
        "    Daylit Attic Zone,       !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    5.0,0.0,3.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    5.0,0.0,2.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5.0,10.0,2.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    5.0,10.0,5.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Daylit Attic Floor,      !- Name",
        "    Floor,                   !- Surface Type",
        "    CEILING IN ATTIC,        !- Construction Name",
        "    Daylit Attic Zone,       !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Daylit Ceiling,          !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0,0.0,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,10.0,2.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5.0,10.0,2.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    5.0,0.0,2.5;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Daylit Attic Roof,       !- Name",
        "    Roof,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    Daylit Attic Zone,       !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.0,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.0,10.0,5.0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,0.0,3.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    5.0,0.0,3.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    5.0,10.0,5.0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  DaylightingDevice:Tubular,",
        "    Pipe1,                   !- Name",
        "    Dome1,                   !- Dome Name",
        "    Diffuser1,               !- Diffuser Name",
        "    TDD Pipe,                !- Construction Name",
        "    0.3556,                  !- Diameter {m}",
        "    1.4,                     !- Total Length {m}",
        "    0.28,                    !- Effective Thermal Resistance {m2-K/W}",
        "    Daylit Attic Zone,       !- Transition Zone 1 Name",
        "    1.1;                     !- Transition Zone 1 Length {m}",

        "  FenestrationSurface:Detailed,",
        "    Dome1,                   !- Name",
        "    TubularDaylightDome,     !- Surface Type",
        "    TDD Dome,                !- Construction Name",
        "    Daylit Attic Roof,       !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.0,                     !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    2.3425,3.209,3.64,  !- X,Y,Z ==> Vertex 1 {m}",
        "    2.3425,2.906,3.58,  !- X,Y,Z ==> Vertex 2 {m}",
        "    2.6575,2.906,3.58,  !- X,Y,Z ==> Vertex 3 {m}",
        "    2.6575,3.209,3.64;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    Diffuser1,               !- Name",
        "    TubularDaylightDiffuser, !- Surface Type",
        "    TDD Diffuser,            !- Construction Name",
        "    Daylit Ceiling,          !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.0,                     !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    2.3425,3.1575,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    2.3425,2.8425,2.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    2.6575,2.8425,2.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    2.6575,3.1575,2.5;  !- X,Y,Z ==> Vertex 4 {m}",

        "  DaylightingDevice:Tubular,",
        "    Pipe2,                   !- Name",
        "    Dome2,                   !- Dome Name",
        "    Diffuser2,               !- Diffuser Name",
        "    TDD Pipe,                !- Construction Name",
        "    0.3556,                  !- Diameter {m}",
        "    2.2,                     !- Total Length {m}",
        "    0.28,                    !- Effective Thermal Resistance {m2-K/W}",
        "    Daylit Attic Zone,       !- Transition Zone 1 Name",
        "    1.9;                     !- Transition Zone 1 Length {m}",

        "  FenestrationSurface:Detailed,",
        "    Dome2,                   !- Name",
        "    TubularDaylightDome,     !- Surface Type",
        "    TDD Dome,                !- Construction Name",
        "    Daylit Attic Roof,       !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.0,                     !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    2.3425,7.209134615385,4.441826923077,  !- X,Y,Z ==> Vertex 1 {m}",
        "    2.3425,6.90625,4.38125,  !- X,Y,Z ==> Vertex 2 {m}",
        "    2.6575,6.90625,4.38125,  !- X,Y,Z ==> Vertex 3 {m}",
        "    2.6575,7.209134615385,4.441826923077;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    Diffuser2,               !- Name",
        "    TubularDaylightDiffuser, !- Surface Type",
        "    TDD Diffuser,            !- Construction Name",
        "    Daylit Ceiling,          !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.0,                     !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    2.3425,7.1575,2.5,  !- X,Y,Z ==> Vertex 1 {m}",
        "    2.3425,6.8425,2.5,  !- X,Y,Z ==> Vertex 2 {m}",
        "    2.6575,6.8425,2.5,  !- X,Y,Z ==> Vertex 3 {m}",
        "    2.6575,7.1575,2.5;  !- X,Y,Z ==> Vertex 4 {m}",

        "  Material,",
        "    A1 - 1 IN STUCCO,        !- Name",
        "    Smooth,                  !- Roughness",
        "    2.5389841E-02,           !- Thickness {m}",
        "    0.6918309,               !- Conductivity {W/m-K}",
        "    1858.142,                !- Density {kg/m3}",
        "    836.8,                   !- Specific Heat {J/kg-K}",
        "    0.90,                    !- Thermal Absorptance",
        "    0.92,                    !- Solar Absorptance",
        "    0.92;                    !- Visible Absorptance",

        "  Material,",
        "    C4 - 4 IN COMMON BRICK,  !- Name",
        "    Rough,                   !- Roughness",
        "    0.1014984,               !- Thickness {m}",
        "    0.7264224,               !- Conductivity {W/m-K}",
        "    1922.216,                !- Density {kg/m3}",
        "    836.8,                   !- Specific Heat {J/kg-K}",
        "    0.90,                    !- Thermal Absorptance",
        "    0.76,                    !- Solar Absorptance",
        "    0.76;                    !- Visible Absorptance",

        "  Material,",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD,  !- Name",
        "    Smooth,                  !- Roughness",
        "    1.9050000E-02,           !- Thickness {m}",
        "    0.7264224,               !- Conductivity {W/m-K}",
        "    1601.846,                !- Density {kg/m3}",
        "    836.8,                   !- Specific Heat {J/kg-K}",
        "    0.90,                    !- Thermal Absorptance",
        "    0.92,                    !- Solar Absorptance",
        "    0.92;                    !- Visible Absorptance",

        "  Material,",
        "    C6 - 8 IN CLAY TILE,     !- Name",
        "    Smooth,                  !- Roughness",
        "    0.2033016,               !- Thickness {m}",
        "    0.5707605,               !- Conductivity {W/m-K}",
        "    1121.292,                !- Density {kg/m3}",
        "    836.8,                   !- Specific Heat {J/kg-K}",
        "    0.90,                    !- Thermal Absorptance",
        "    0.82,                    !- Solar Absorptance",
        "    0.82;                    !- Visible Absorptance",

        "  Material,",
        "    C10 - 8 IN HW CONCRETE,  !- Name",
        "    MediumRough,             !- Roughness",
        "    0.2033016,               !- Thickness {m}",
        "    1.729577,                !- Conductivity {W/m-K}",
        "    2242.585,                !- Density {kg/m3}",
        "    836.8,                   !- Specific Heat {J/kg-K}",
        "    0.90,                    !- Thermal Absorptance",
        "    0.65,                    !- Solar Absorptance",
        "    0.65;                    !- Visible Absorptance",

        "  Material,",
        "    E2 - 1 / 2 IN SLAG OR STONE,  !- Name",
        "    Rough,                   !- Roughness",
        "    1.2710161E-02,           !- Thickness {m}",
        "    1.435549,                !- Conductivity {W/m-K}",
        "    881.0155,                !- Density {kg/m3}",
        "    1673.6,                  !- Specific Heat {J/kg-K}",
        "    0.90,                    !- Thermal Absorptance",
        "    0.55,                    !- Solar Absorptance",
        "    0.55;                    !- Visible Absorptance",

        "  Material,",
        "    E3 - 3 / 8 IN FELT AND MEMBRANE,  !- Name",
        "    Rough,                   !- Roughness",
        "    9.5402403E-03,           !- Thickness {m}",
        "    0.1902535,               !- Conductivity {W/m-K}",
        "    1121.292,                !- Density {kg/m3}",
        "    1673.6,                  !- Specific Heat {J/kg-K}",
        "    0.90,                    !- Thermal Absorptance",
        "    0.75,                    !- Solar Absorptance",
        "    0.75;                    !- Visible Absorptance",

        "  Material,",
        "    B5 - 1 IN DENSE INSULATION,  !- Name",
        "    VeryRough,               !- Roughness",
        "    2.5389841E-02,           !- Thickness {m}",
        "    4.3239430E-02,           !- Conductivity {W/m-K}",
        "    91.30524,                !- Density {kg/m3}",
        "    836.8,                   !- Specific Heat {J/kg-K}",
        "    0.90,                    !- Thermal Absorptance",
        "    0.50,                    !- Solar Absorptance",
        "    0.50;                    !- Visible Absorptance",

        "  Material,",
        "    C12 - 2 IN HW CONCRETE,  !- Name",
        "    MediumRough,             !- Roughness",
        "    5.0901599E-02,           !- Thickness {m}",
        "    1.729577,                !- Conductivity {W/m-K}",
        "    2242.585,                !- Density {kg/m3}",
        "    836.8,                   !- Specific Heat {J/kg-K}",
        "    0.90,                    !- Thermal Absorptance",
        "    0.65,                    !- Solar Absorptance",
        "    0.65;                    !- Visible Absorptance",

        "  Material,",
        "    ROOFING - ASPHALT SHINGLES,  !- Name",
        "    VeryRough,               !- Roughness",
        "    3.1999999E-03,           !- Thickness {m}",
        "    2.9999999E-02,           !- Conductivity {W/m-K}",
        "    1121.29,                 !- Density {kg/m3}",
        "    830.0,                   !- Specific Heat {J/kg-K}",
        "    0.90,                    !- Thermal Absorptance",
        "    0.70,                    !- Solar Absorptance",
        "    0.70;                    !- Visible Absorptance",

        "  Material,",
        "    BB46 - 5 / 8 IN PLYWOOD, !- Name",
        "    Smooth,                  !- Roughness",
        "    9.9999998E-03,           !- Thickness {m}",
        "    0.110,                   !- Conductivity {W/m-K}",
        "    544.62,                  !- Density {kg/m3}",
        "    1210.0,                  !- Specific Heat {J/kg-K}",
        "    0.90,                    !- Thermal Absorptance",
        "    0.70,                    !- Solar Absorptance",
        "    0.70;                    !- Visible Absorptance",

        "  Material,",
        "    INS - GLASS FIBER BONDED 3 IN,  !- Name",
        "    VeryRough,               !- Roughness",
        "    7.000E-02,               !- Thickness {m}",
        "    2.9999999E-02,           !- Conductivity {W/m-K}",
        "    96.11,                   !- Density {kg/m3}",
        "    790.0,                   !- Specific Heat {J/kg-K}",
        "    0.90,                    !- Thermal Absorptance",
        "    0.50,                    !- Solar Absorptance",
        "    0.50;                    !- Visible Absorptance",

        "  WindowMaterial:Glazing,",
        "    Clear Acrylic Plastic,   !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.003,                   !- Thickness {m}",
        "    0.92,                    !- Solar Transmittance at Normal Incidence",
        "    0.05,                    !- Front Side Solar Reflectance at Normal Incidence",
        "    0.05,                    !- Back Side Solar Reflectance at Normal Incidence",
        "    0.92,                    !- Visible Transmittance at Normal Incidence",
        "    0.05,                    !- Front Side Visible Reflectance at Normal Incidence",
        "    0.05,                    !- Back Side Visible Reflectance at Normal Incidence",
        "    0.00,                    !- Infrared Transmittance at Normal Incidence",
        "    0.90,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.90,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.90;                    !- Conductivity {W/m-K}",

        "  WindowMaterial:Glazing,",
        "    Diffusing Acrylic Plastic,  !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.0022,                  !- Thickness {m}",
        "    0.90,                    !- Solar Transmittance at Normal Incidence",
        "    0.08,                    !- Front Side Solar Reflectance at Normal Incidence",
        "    0.08,                    !- Back Side Solar Reflectance at Normal Incidence",
        "    0.90,                    !- Visible Transmittance at Normal Incidence",
        "    0.08,                    !- Front Side Visible Reflectance at Normal Incidence",
        "    0.08,                    !- Back Side Visible Reflectance at Normal Incidence",
        "    0.00,                    !- Infrared Transmittance at Normal Incidence",
        "    0.90,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.90,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.90;                    !- Conductivity {W/m-K}",

        "  Material,",
        "    Very High Reflectivity Surface,  !- Name",
        "    Smooth,                  !- Roughness",
        "    0.0005,                  !- Thickness {m}",
        "    237,                     !- Conductivity {W/m-K}",
        "    2702,                    !- Density {kg/m3}",
        "    903,                     !- Specific Heat {J/kg-K}",
        "    0.90,                    !- Thermal Absorptance",
        "    0.05,                    !- Solar Absorptance",
        "    0.05;                    !- Visible Absorptance",

        "  Construction,",
        "    EXTWALL80,               !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    C4 - 4 IN COMMON BRICK,  !- Layer 2",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 3",

        "  Construction,",
        "    FLOOR SLAB 8 IN,         !- Name",
        "    C10 - 8 IN HW CONCRETE;  !- Outside Layer",

        "  Construction,",
        "    ROOF,                    !- Name",
        "    ROOFING - ASPHALT SHINGLES,  !- Outside Layer",
        "    E3 - 3 / 8 IN FELT AND MEMBRANE,  !- Layer 2",
        "    BB46 - 5 / 8 IN PLYWOOD; !- Layer 3",

        "  Construction,",
        "    CEILING IN ZONE,         !- Name",
        "    INS - GLASS FIBER BONDED 3 IN,  !- Outside Layer",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD;  !- Layer 2",

        "  Construction,",
        "    CEILING IN ATTIC,        !- Name",
        "    E1 - 3 / 4 IN PLASTER OR GYP BOARD,  !- Outside Layer",
        "    INS - GLASS FIBER BONDED 3 IN;  !- Layer 2",

        "  Construction,",
        "    TDD Pipe,                !- Name",
        "    Very High Reflectivity Surface;  !- Outside Layer",

        "  Construction,",
        "    TDD Dome,                !- Name",
        "    Clear Acrylic Plastic;   !- Outside Layer",

        "  Construction,",
        "    TDD Diffuser,            !- Name",
        "    Diffusing Acrylic Plastic;  !- Outside Layer",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound = false;

    HeatBalanceManager::GetProjectControlData(*state, ErrorsFound); // read project control data
    EXPECT_FALSE(ErrorsFound);                                      // expect no errors
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    Material::GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    SurfaceGeometry::GetGeometryParameters(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    state->dataSurfaceGeometry->CosZoneRelNorth.allocate(2);
    state->dataSurfaceGeometry->SinZoneRelNorth.allocate(2);

    state->dataSurfaceGeometry->CosZoneRelNorth(1) = std::cos(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(1) = std::sin(-state->dataHeatBal->Zone(1).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosZoneRelNorth(2) = std::cos(-state->dataHeatBal->Zone(2).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->SinZoneRelNorth(2) = std::sin(-state->dataHeatBal->Zone(2).RelNorth * Constant::DegToRad);
    state->dataSurfaceGeometry->CosBldgRelNorth = 1.0;
    state->dataSurfaceGeometry->SinBldgRelNorth = 0.0;
    int const HoursInDay(24);
    state->dataSurface->SurfSunCosHourly.allocate(HoursInDay);
    for (int hour = 1; hour <= HoursInDay; hour++) {
        state->dataSurface->SurfSunCosHourly(hour) = 0.0;
    }
    //    SurfaceGeometry::GetSurfaceData(*state, ErrorsFound);
    //    EXPECT_FALSE(ErrorsFound);

    SurfaceGeometry::SetupZoneGeometry(*state, ErrorsFound); // this calls GetSurfaceData()
    EXPECT_FALSE(ErrorsFound);                               // expect no errors
    HeatBalanceIntRadExchange::InitSolarViewFactors(*state);

    state->dataZoneEquip->ZoneEquipConfig.allocate(2);
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneName = "Daylit Zone";
    state->dataHeatBal->Zone(1).IsControlled = true;
    state->dataZoneEquip->ZoneEquipConfig(1).NumInletNodes = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode.allocate(2);
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(1) = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(2) = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1) = 3;
    state->dataZoneEquip->ZoneEquipConfig(1).NumReturnNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode(1) = 4;
    state->dataZoneEquip->ZoneEquipConfig(1).FixedReturnFlow.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(2).ZoneName = "Daylit Attic Zone";
    state->dataHeatBal->Zone(2).IsControlled = true;
    state->dataZoneEquip->ZoneEquipConfig(2).NumInletNodes = 2;
    state->dataZoneEquip->ZoneEquipConfig(2).InletNode.allocate(2);
    state->dataZoneEquip->ZoneEquipConfig(2).InletNode(1) = 6;
    state->dataZoneEquip->ZoneEquipConfig(2).InletNode(2) = 7;
    state->dataZoneEquip->ZoneEquipConfig(2).NumExhaustNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(2).ExhaustNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(2).ExhaustNode(1) = 8;
    state->dataZoneEquip->ZoneEquipConfig(2).NumReturnNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(2).ReturnNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(2).ReturnNode(1) = 9;
    state->dataZoneEquip->ZoneEquipConfig(2).FixedReturnFlow.allocate(1);

    state->dataSize->ZoneEqSizing.allocate(2);
    state->dataHeatBal->Zone(1).SystemZoneNodeNumber = 5;
    state->dataHeatBal->Zone(2).SystemZoneNodeNumber = 10;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(2);
    for (auto &thisZoneHB : state->dataZoneTempPredictorCorrector->zoneHeatBalance) {
        thisZoneHB.MAT = 24.0;
        thisZoneHB.airHumRat = 0.001;
        thisZoneHB.airHumRatAvg = 0.001;
    }

    state->dataLoopNodes->Node.allocate(4);

    state->dataHeatBalSurf->SurfTempInTmp.allocate(6);
    state->dataHeatBalSurf->SurfTempInTmp(1) = 15.0;
    state->dataHeatBalSurf->SurfTempInTmp(2) = 20.0;
    state->dataHeatBalSurf->SurfTempInTmp(3) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(4) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(5) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(6) = 25.0;

    state->dataLoopNodes->Node(1).Temp = 20.0;
    state->dataLoopNodes->Node(2).Temp = 20.0;
    state->dataLoopNodes->Node(3).Temp = 20.0;
    state->dataLoopNodes->Node(4).Temp = 20.0;
    state->dataLoopNodes->Node(1).MassFlowRate = 0.1;
    state->dataLoopNodes->Node(2).MassFlowRate = 0.1;
    state->dataLoopNodes->Node(3).MassFlowRate = 0.1;
    state->dataLoopNodes->Node(4).MassFlowRate = 0.1;

    state->dataHeatBalSurf->SurfWinCoeffAdjRatio.dimension(state->dataSurface->TotSurfaces, 1.0);
    state->dataHeatBalSurf->SurfHConvInt.dimension(state->dataSurface->TotSurfaces, 0.5);
    state->dataMstBal->HConvInFD.allocate(state->dataSurface->TotSurfaces);
    state->dataMstBal->RhoVaporAirIn.allocate(state->dataSurface->TotSurfaces);
    state->dataMstBal->HMassConvInFD.allocate(state->dataSurface->TotSurfaces);

    SolarShading::AllocateModuleArrays(*state);
    HeatBalanceManager::AllocateZoneHeatBalArrays(*state);
    AllocateSurfaceHeatBalArrays(*state);
    createFacilityElectricPowerServiceObject(*state);

    for (int loop = 1; loop <= state->dataSurface->TotSurfaces; ++loop) {
        state->dataHeatBalSurf->SurfOutsideTempHist(1)(loop) = 20.0;
    }

    state->dataConstruction->Construct(state->dataSurface->Surface(8).Construction).TransDiff = 0.001; // required for GetTDDInput function to work.
    Dayltg::GetTDDInput(*state);

    state->dataGlobal->TimeStepZoneSec = 60.0;

    CalcHeatBalanceInsideSurf(*state);
    EXPECT_NEAR(31.79, state->dataSurface->SurfWinHeatGain(7), 0.1);
    EXPECT_NEAR(31.79, state->dataSurface->SurfWinHeatGain(8), 0.1);
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_TestSurfPropertyViewFactorsInit)
{

    std::string const idf_objects = delimited_string({
        "  Building,",
        "    House with Local Air Nodes,  !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    0.001,                   !- Loads Convergence Tolerance Value",
        "    0.0050000,               !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullInteriorAndExterior, !- Solar Distribution",
        "    25,                      !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",

        "  Timestep,6;",

        "  SurfaceConvectionAlgorithm:Inside,TARP;",

        "  SurfaceConvectionAlgorithm:Outside,DOE-2;",

        "  HeatBalanceAlgorithm,ConductionTransferFunction;",

        "  SimulationControl,",
        "    No,                      !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    Yes,                     !- Run Simulation for Sizing Periods",
        "    Yes;                     !- Run Simulation for Weather File Run Periods",

        "  RunPeriod,",
        "    WinterDay,               !- Name",
        "    1,                       !- Begin Month",
        "    14,                      !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    1,                       !- End Month",
        "    14,                      !- End Day of Month",
        "    ,                        !- End Year",
        "    Tuesday,                 !- Day of Week for Start Day",
        "    Yes,                     !- Use Weather File Holidays and Special Days",
        "    Yes,                     !- Use Weather File Daylight Saving Period",
        "    No,                      !- Apply Weekend Holiday Rule",
        "    Yes,                     !- Use Weather File Rain Indicators",
        "    Yes;                     !- Use Weather File Snow Indicators",

        "  RunPeriod,",
        "    SummerDay,               !- Name",
        "    7,                       !- Begin Month",
        "    7,                       !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    7,                       !- End Month",
        "    7,                       !- End Day of Month",
        "    ,                        !- End Year",
        "    Tuesday,                 !- Day of Week for Start Day",
        "    No,                      !- Apply Weekend Holiday Rule",
        "    Yes,                     !- Use Weather File Rain Indicators",
        "    No;                      !- Use Weather File Snow Indicators",

        "  Site:Location,",
        "    CHICAGO_IL_USA TMY2-94846,  !- Name",
        "    41.78,                   !- Latitude {deg}",
        "    -87.75,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190.00;                  !- Elevation {m}",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Heating 99% Design Conditions DB,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    WinterDesignDay,         !- Day Type",
        "    -17.3,                   !- Maximum Dry-Bulb Temperature {C}",
        "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    -17.3,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    4.9,                     !- Wind Speed {m/s}",
        "    270,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    0.0;                     !- Sky Clearness",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Cooling 1% Design Conditions DB/MCWB,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    31.5,                    !- Maximum Dry-Bulb Temperature {C}",
        "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    23.0,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    5.3,                     !- Wind Speed {m/s}",
        "    230,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    1.0;                     !- Sky Clearness",

        "  Site:GroundTemperature:BuildingSurface,20.03,20.03,20.13,20.30,20.43,20.52,20.62,20.77,20.78,20.55,20.44,20.20;",

        "  Material,",
        "    A1 - 1 IN STUCCO,        !- Name",
        "    Smooth,                  !- Roughness",
        "    2.5389841E-02,           !- Thickness {m}",
        "    0.6918309,               !- Conductivity {W/m-K}",
        "    1858.142,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Material,",
        "    CB11,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    0.2032000,               !- Thickness {m}",
        "    1.048000,                !- Conductivity {W/m-K}",
        "    1105.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.2000000,               !- Solar Absorptance",
        "    0.2000000;               !- Visible Absorptance",

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
        "    IN05,                    !- Name",
        "    Rough,                   !- Roughness",
        "    0.2458000,               !- Thickness {m}",
        "    4.3000001E-02,           !- Conductivity {W/m-K}",
        "    10.00000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    PW03,                    !- Name",
        "    MediumSmooth,            !- Roughness",
        "    1.2700000E-02,           !- Thickness {m}",
        "    0.1150000,               !- Conductivity {W/m-K}",
        "    545.0000,                !- Density {kg/m3}",
        "    1213.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7800000,               !- Solar Absorptance",
        "    0.7800000;               !- Visible Absorptance",

        "  Material,",
        "    CC03,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1016000,               !- Thickness {m}",
        "    1.310000,                !- Conductivity {W/m-K}",
        "    2243.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

        "  Material,",
        "    HF-A3,                   !- Name",
        "    Smooth,                  !- Roughness",
        "    1.5000000E-03,           !- Thickness {m}",
        "    44.96960,                !- Conductivity {W/m-K}",
        "    7689.000,                !- Density {kg/m3}",
        "    418.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.2000000,               !- Solar Absorptance",
        "    0.2000000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    AR02,                    !- Name",
        "    VeryRough,               !- Roughness",
        "    7.8000002E-02,           !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7000000,               !- Solar Absorptance",
        "    0.7000000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    CP02,                    !- Name",
        "    Rough,                   !- Roughness",
        "    0.2170000,               !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Construction,",
        "    EXTWALL:LIVING,          !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    GP01;                    !- Layer 3",

        "  Construction,",
        "    FLOOR:LIVING,            !- Name",
        "    CC03,                    !- Outside Layer",
        "    CP02;                    !- Layer 2",

        "  Construction,",
        "    ROOF,                    !- Name",
        "    AR02,                    !- Outside Layer",
        "    PW03;                    !- Layer 2",

        "  Zone,",
        "    LIVING ZONE,             !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    World;                   !- Coordinate System",

        "  BuildingSurface:Detailed,",
        "    Living:North,            !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    1,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    1,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:East,             !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    1,0,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    1,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:South,            !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,0,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:West,             !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,0,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Floor,            !- Name",
        "    FLOOR,                   !- Surface Type",
        "    FLOOR:LIVING,            !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Living:Floor,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,0,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Ceiling,          !- Name",
        "    ROOF,                 !- Surface Type",
        "    ROOF,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,1,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,0,1,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  SurfaceProperty:LocalEnvironment,",
        "    LocEnv:Living:North,          !- Name",
        "    Living:North,                 !- Exterior Surface Name",
        "    ,                             !- External Shading Fraction Schedule Name",
        "    SrdSurfs:Living:North,        !- Surrounding Surfaces Object Name",
        "    ,                             !- Outdoor Air Node Name",
        "    GndSurfs:Living:North;        !- Ground Surfaces Object Name",

        "  SurfaceProperty:LocalEnvironment,",
        "    LocEnv:Living:East,           !- Name",
        "    Living:East,                  !- Exterior Surface Name",
        "    ,                             !- External Shading Fraction Schedule Name",
        "    SrdSurfs:Living:East,         !- Surrounding Surfaces Object Name",
        "    ,                             !- Outdoor Air Node Name",
        "    GndSurfs:Living:East;         !- Ground Surfaces Object Name",

        "  SurfaceProperty:LocalEnvironment,",
        "    LocEnv:Living:South,          !- Name",
        "    Living:South,                 !- Exterior Surface Name",
        "    ,                             !- External Shading Fraction Schedule Name",
        "    SrdSurfs:Living:South,        !- Surrounding Surfaces Object Name",
        "    ,                             !- Outdoor Air Node Name",
        "    GndSurfs:Living:South;        !- Ground Surfaces Object Name",

        "  SurfaceProperty:SurroundingSurfaces,",
        "    SrdSurfs:Living:North, !- Name",
        "    0.3,",
        "    Sky Temp Sch,",
        "    ,",
        "    ,",
        "    SurroundingSurface1,",
        "    0.6,",
        "    Surrounding Temp Sch 1;",

        "  SurfaceProperty:SurroundingSurfaces,",
        "    SrdSurfs:Living:East, !- Name",
        "    0.2,",
        "    ,",
        "    ,",
        "    ,",
        "    SurroundingSurface1,",
        "    0.3,",
        "    Surrounding Temp Sch 1,",
        "    SurroundingSurface2,",
        "    0.3,",
        "    Surrounding Temp Sch 1;",

        "  SurfaceProperty:SurroundingSurfaces,",
        "    SrdSurfs:Living:South, !- Name",
        "    ,",
        "    ,",
        "    ,",
        "    ,",
        "    SurroundingSurface1,",
        "    0.4,",
        "    Surrounding Temp Sch 1;",

        "  SurfaceProperty:GroundSurfaces,",
        "    GndSurfs:Living:North,        !-Name",
        "    GndSurfs NorthGrassArea,      !-Ground Surface 1 Name",
        "    0.1,                          !-Ground Surface 1 View Factor",
        "    Ground Temp Sch,              !-Ground Surface 1 Temperature Schedule Name",
        "    ;                             !-Ground Surface 1 Reflectance Schedule Name",

        "  SurfaceProperty:GroundSurfaces,",
        "    GndSurfs:Living:East,         !-Name",
        "    GndSurfs EastGrassArea,       !-Ground Surface 1 Name",
        "    ,                             !-Ground Surface 1 View Factor",
        "    Ground Temp Sch,              !-Ground Surface 1 Temperature Schedule Name",
        "    ;                             !-Ground Surface 1 Reflectance Schedule Name",

        "  SurfaceProperty:GroundSurfaces,",
        "    GndSurfs:Living:South,        !-Name",
        "    GndSurfs SouthGrassArea,      !-Ground Surface 1 Name",
        "    ,                             !-Ground Surface 1 View Factor",
        "    Ground Temp Sch,              !-Ground Surface 1 Temperature Schedule Name",
        "    ;                             !-Ground Surface 1 Reflectance Schedule Name",

        "  ScheduleTypeLimits,",
        "    Any Number;                   !- Name",

        "  Schedule:Compact,",
        "    Surrounding Temp Sch 1,       !- Name",
        "    Any Number,                   !- Schedule Type Limits Name",
        "    Through: 12/31,               !- Field 1",
        "    For: AllDays,                 !- Field 2",
        "    Until: 24:00, 15.0;           !- Field 3",

        "  Schedule:Compact,",
        "    Sky Temp Sch,                 !- Name",
        "    Any Number,                   !- Schedule Type Limits Name",
        "    Through: 12/31,               !- Field 1",
        "    For: AllDays,                 !- Field 2",
        "    Until: 24:00, 15.0;           !- Field 3",

        "  Schedule:Compact,",
        "    Ground Temp Sch,              !- Name",
        "    Any Number,                   !- Schedule Type Limits Name",
        "    Through: 12/31,               !- Field 1",
        "    For: AllDays,                 !- Field 2",
        "    Until: 24:00, 15.0;           !- Field 3",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound = false;
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    Material::GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    SurfaceGeometry::GetGeometryParameters(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    state->dataSurfaceGeometry->CosBldgRotAppGonly = 1.0;
    state->dataSurfaceGeometry->SinBldgRotAppGonly = 0.0;
    SurfaceGeometry::SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    HeatBalanceIntRadExchange::InitSolarViewFactors(*state);
    EXPECT_FALSE(has_err_output(true));
    EXPECT_TRUE(state->dataGlobal->AnyLocalEnvironmentsInModel);

    auto &Surface_1 = state->dataSurface->Surface(1);
    auto &Surface_2 = state->dataSurface->Surface(2);
    auto &Surface_3 = state->dataSurface->Surface(3);

    // test surface property sky and ground view factors inputs
    int SrdSurfsNum = state->dataSurface->Surface(1).SurfSurroundingSurfacesNum;
    auto &SrdSurfsProperty_1 = state->dataSurface->SurroundingSurfsProperty(SrdSurfsNum);
    SrdSurfsNum = state->dataSurface->Surface(2).SurfSurroundingSurfacesNum;
    auto &SrdSurfsProperty_2 = state->dataSurface->SurroundingSurfsProperty(SrdSurfsNum);
    SrdSurfsNum = state->dataSurface->Surface(3).SurfSurroundingSurfacesNum;
    auto &SrdSurfsProperty_3 = state->dataSurface->SurroundingSurfsProperty(SrdSurfsNum);

    int GndSurfsNum = Surface_1.SurfPropertyGndSurfIndex;
    auto &GndSurfsProperty_1 = state->dataSurface->GroundSurfsProperty(GndSurfsNum);
    GndSurfsNum = Surface_2.SurfPropertyGndSurfIndex;
    auto &GndSurfsProperty_2 = state->dataSurface->GroundSurfsProperty(GndSurfsNum);
    GndSurfsNum = Surface_3.SurfPropertyGndSurfIndex;
    auto &GndSurfsProperty_3 = state->dataSurface->GroundSurfsProperty(GndSurfsNum);

    // check surface property view factors get inputs
    EXPECT_DOUBLE_EQ(0.3, SrdSurfsProperty_1.SkyViewFactor);
    EXPECT_TRUE(SrdSurfsProperty_1.IsSkyViewFactorSet);
    EXPECT_DOUBLE_EQ(0.1, GndSurfsProperty_1.GndSurfs(1).ViewFactor);
    EXPECT_TRUE(GndSurfsProperty_1.IsGroundViewFactorSet);
    EXPECT_DOUBLE_EQ(0.2, SrdSurfsProperty_2.SkyViewFactor);
    EXPECT_TRUE(SrdSurfsProperty_2.IsSkyViewFactorSet);
    EXPECT_DOUBLE_EQ(0.0, GndSurfsProperty_2.GndSurfs(1).ViewFactor);
    EXPECT_FALSE(GndSurfsProperty_2.IsGroundViewFactorSet);
    EXPECT_DOUBLE_EQ(0.0, SrdSurfsProperty_3.SkyViewFactor);
    EXPECT_FALSE(SrdSurfsProperty_3.IsSkyViewFactorSet);
    EXPECT_DOUBLE_EQ(0.0, GndSurfsProperty_3.GndSurfs(1).ViewFactor);
    EXPECT_FALSE(GndSurfsProperty_3.IsGroundViewFactorSet);
    // check exterior surfaces default view factors
    EXPECT_DOUBLE_EQ(0.5, Surface_1.ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.5, Surface_1.ViewFactorGroundIR);
    EXPECT_DOUBLE_EQ(0.5, Surface_2.ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.5, Surface_2.ViewFactorGroundIR);
    EXPECT_DOUBLE_EQ(0.5, Surface_3.ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.5, Surface_3.ViewFactorGroundIR);

    // reset sky and ground view factors
    InitSurfacePropertyViewFactors(*state);
    // test blank view factor fields flags reset to true
    EXPECT_TRUE(GndSurfsProperty_2.IsGroundViewFactorSet);
    EXPECT_TRUE(SrdSurfsProperty_3.IsSkyViewFactorSet);
    EXPECT_TRUE(GndSurfsProperty_3.IsGroundViewFactorSet);

    // test surface property sky and ground view factors reset
    EXPECT_DOUBLE_EQ(0.3, SrdSurfsProperty_1.SkyViewFactor);
    EXPECT_DOUBLE_EQ(0.1, GndSurfsProperty_1.GndSurfs(1).ViewFactor);
    EXPECT_DOUBLE_EQ(0.2, SrdSurfsProperty_2.SkyViewFactor);
    EXPECT_DOUBLE_EQ(0.2, GndSurfsProperty_2.GndSurfs(1).ViewFactor);
    EXPECT_DOUBLE_EQ(0.3, SrdSurfsProperty_3.SkyViewFactor);
    EXPECT_DOUBLE_EQ(0.3, GndSurfsProperty_3.GndSurfs(1).ViewFactor);
    // test exterior surfaces sky and ground view factors reset
    EXPECT_DOUBLE_EQ(0.3, Surface_1.ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.1, Surface_1.ViewFactorGroundIR);
    EXPECT_DOUBLE_EQ(0.2, Surface_2.ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.2, Surface_2.ViewFactorGroundIR);
    EXPECT_DOUBLE_EQ(0.3, Surface_3.ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.3, Surface_3.ViewFactorGroundIR);
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_TestSurfPropertySurfToGndLWR)
{

    std::string const idf_objects = delimited_string({
        "  Building,",
        "    House with Local Air Nodes,  !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    0.001,                   !- Loads Convergence Tolerance Value",
        "    0.0050000,               !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullInteriorAndExterior, !- Solar Distribution",
        "    25,                      !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",

        "  Timestep,6;",

        "  SurfaceConvectionAlgorithm:Inside,TARP;",

        "  SurfaceConvectionAlgorithm:Outside,DOE-2;",

        "  HeatBalanceAlgorithm,ConductionTransferFunction;",

        "  SimulationControl,",
        "    No,                      !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    Yes,                     !- Run Simulation for Sizing Periods",
        "    Yes;                     !- Run Simulation for Weather File Run Periods",

        "  RunPeriod,",
        "    WinterDay,               !- Name",
        "    1,                       !- Begin Month",
        "    14,                      !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    1,                       !- End Month",
        "    14,                      !- End Day of Month",
        "    ,                        !- End Year",
        "    Tuesday,                 !- Day of Week for Start Day",
        "    Yes,                     !- Use Weather File Holidays and Special Days",
        "    Yes,                     !- Use Weather File Daylight Saving Period",
        "    No,                      !- Apply Weekend Holiday Rule",
        "    Yes,                     !- Use Weather File Rain Indicators",
        "    Yes;                     !- Use Weather File Snow Indicators",

        "  RunPeriod,",
        "    SummerDay,               !- Name",
        "    7,                       !- Begin Month",
        "    7,                       !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    7,                       !- End Month",
        "    7,                       !- End Day of Month",
        "    ,                        !- End Year",
        "    Tuesday,                 !- Day of Week for Start Day",
        "    No,                      !- Apply Weekend Holiday Rule",
        "    Yes,                     !- Use Weather File Rain Indicators",
        "    No;                      !- Use Weather File Snow Indicators",

        "  Site:Location,",
        "    CHICAGO_IL_USA TMY2-94846,  !- Name",
        "    41.78,                   !- Latitude {deg}",
        "    -87.75,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190.00;                  !- Elevation {m}",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Heating 99% Design Conditions DB,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    WinterDesignDay,         !- Day Type",
        "    -17.3,                   !- Maximum Dry-Bulb Temperature {C}",
        "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    -17.3,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    4.9,                     !- Wind Speed {m/s}",
        "    270,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    0.0;                     !- Sky Clearness",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Cooling 1% Design Conditions DB/MCWB,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    31.5,                    !- Maximum Dry-Bulb Temperature {C}",
        "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    23.0,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    5.3,                     !- Wind Speed {m/s}",
        "    230,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    1.0;                     !- Sky Clearness",

        "  Site:GroundTemperature:BuildingSurface,20.03,20.03,20.13,20.30,20.43,20.52,20.62,20.77,20.78,20.55,20.44,20.20;",

        "  Material,",
        "    A1 - 1 IN STUCCO,        !- Name",
        "    Smooth,                  !- Roughness",
        "    2.5389841E-02,           !- Thickness {m}",
        "    0.6918309,               !- Conductivity {W/m-K}",
        "    1858.142,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Material,",
        "    CB11,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    0.2032000,               !- Thickness {m}",
        "    1.048000,                !- Conductivity {W/m-K}",
        "    1105.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.2000000,               !- Solar Absorptance",
        "    0.2000000;               !- Visible Absorptance",

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
        "    IN05,                    !- Name",
        "    Rough,                   !- Roughness",
        "    0.2458000,               !- Thickness {m}",
        "    4.3000001E-02,           !- Conductivity {W/m-K}",
        "    10.00000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    PW03,                    !- Name",
        "    MediumSmooth,            !- Roughness",
        "    1.2700000E-02,           !- Thickness {m}",
        "    0.1150000,               !- Conductivity {W/m-K}",
        "    545.0000,                !- Density {kg/m3}",
        "    1213.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7800000,               !- Solar Absorptance",
        "    0.7800000;               !- Visible Absorptance",

        "  Material,",
        "    CC03,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1016000,               !- Thickness {m}",
        "    1.310000,                !- Conductivity {W/m-K}",
        "    2243.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

        "  Material,",
        "    HF-A3,                   !- Name",
        "    Smooth,                  !- Roughness",
        "    1.5000000E-03,           !- Thickness {m}",
        "    44.96960,                !- Conductivity {W/m-K}",
        "    7689.000,                !- Density {kg/m3}",
        "    418.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.2000000,               !- Solar Absorptance",
        "    0.2000000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    AR02,                    !- Name",
        "    VeryRough,               !- Roughness",
        "    7.8000002E-02,           !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7000000,               !- Solar Absorptance",
        "    0.7000000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    CP02,                    !- Name",
        "    Rough,                   !- Roughness",
        "    0.2170000,               !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Construction,",
        "    EXTWALL:LIVING,          !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    GP01;                    !- Layer 3",

        "  Construction,",
        "    FLOOR:LIVING,            !- Name",
        "    CC03,                    !- Outside Layer",
        "    CP02;                    !- Layer 2",

        "  Construction,",
        "    ROOF,                    !- Name",
        "    AR02,                    !- Outside Layer",
        "    PW03;                    !- Layer 2",

        "  Zone,",
        "    LIVING ZONE,             !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    World;                   !- Coordinate System",

        "  BuildingSurface:Detailed,",
        "    Living:North,            !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    1,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    1,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:East,             !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    1,0,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    1,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:South,            !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,0,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:West,             !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,0,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Floor,            !- Name",
        "    FLOOR,                   !- Surface Type",
        "    FLOOR:LIVING,            !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Living:Floor,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,0,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Ceiling,          !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,1,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,0,1,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  SurfaceProperty:LocalEnvironment,",
        "    LocEnv:Living:North,          !- Name",
        "    Living:North,                 !- Exterior Surface Name",
        "    ,                             !- External Shading Fraction Schedule Name",
        "    SrdSurfs:Living:North,        !- Surrounding Surfaces Object Name",
        "    ,                             !- Outdoor Air Node Name",
        "    GndSurfs:Living:North;        !- Ground Surfaces Object Name",

        "  SurfaceProperty:LocalEnvironment,",
        "    LocEnv:Living:East,           !- Name",
        "    Living:East,                  !- Exterior Surface Name",
        "    ,                             !- External Shading Fraction Schedule Name",
        "    SrdSurfs:Living:East,         !- Surrounding Surfaces Object Name",
        "    ,                             !- Outdoor Air Node Name",
        "    GndSurfs:Living:East;         !- Ground Surfaces Object Name",

        "  SurfaceProperty:LocalEnvironment,",
        "    LocEnv:Living:South,          !- Name",
        "    Living:South,                 !- Exterior Surface Name",
        "    ,                             !- External Shading Fraction Schedule Name",
        "    SrdSurfs:Living:South,        !- Surrounding Surfaces Object Name",
        "    ,                             !- Outdoor Air Node Name",
        "    GndSurfs:Living:South;        !- Ground Surfaces Object Name",

        "  SurfaceProperty:SurroundingSurfaces,",
        "    SrdSurfs:Living:North,        !- Name",
        "    0.3,                          !- Sky View Factor",
        "    Sky Temp Sch,                 !- Sky Temperature Schedule Name",
        "    ,                             !- Ground View Factor",
        "    ,                             !- Ground Temperature Schedule Name",
        "    SurroundingSurface1,          !- Surrounding Surface 1 Name",
        "    0.2,                          !- Surrounding Surface 1 View Factor",
        "    Surrounding Temp Sch 1;       !- Surrounding Surface 1 Temperature Schedule Name",

        "  SurfaceProperty:GroundSurfaces,",
        "    GndSurfs:Living:North,        !-Name",
        "    GndSurfs GrassArea,           !-Ground Surface 1 Name",
        "    0.2,                          !-Ground Surface 1 View Factor",
        "    Ground Temp Sch,              !-Ground Surface 1 Temperature Schedule Name",
        "    ,                             !-Ground Surface 1 Reflectance Schedule Name",
        "    GndSurfs ParkingArea,         !-Ground Surface 2 Name",
        "    0.2,                          !-Ground Surface 2 View Factor",
        "    Ground Temp Sch,              !-Ground Surface 2 Temperature Schedule Name",
        "    ,                             !-Ground Surface 2 Reflectance Schedule Name",
        "    GndSurfs LakeArea,            !-Ground Surface 3 Name",
        "    0.1,                          !-Ground Surface 3 View Factor",
        "    Ground Temp Sch,              !-Ground Surface 3 Temperature Schedule Name",
        "    ;                             !-Ground Surface 3 Reflectance Schedule Name",

        "  SurfaceProperty:SurroundingSurfaces,",
        "    SrdSurfs:Living:East,         !- Name",
        "    0.2,                          !- Sky View Factor",
        "    Sky Temp Sch,                 !- Sky Temperature Schedule Name",
        "    ,                             !- Ground View Factor",
        "    ,                             !- Ground Temperature Schedule Name",
        "    SurroundingSurface1,          !- Surrounding Surface 1 Name",
        "    0.3,                          !- Surrounding Surface 1 View Factor",
        "    Surrounding Temp Sch 1,       !- Surrounding Surface 1 Temperature Schedule Name",
        "    SurroundingSurface1,          !- Surrounding Surface 2 Name",
        "    0.3,                          !- Surrounding Surface 2 View Factor",
        "    Surrounding Temp Sch 1;       !- Surrounding Surface 2 Temperature Schedule Name",

        "  SurfaceProperty:GroundSurfaces,",
        "    GndSurfs:Living:East,         !-Name",
        "    GndSurfs EastGrassArea,       !-Ground Surface 1 Name",
        "    0.2,                          !-Ground Surface 1 View Factor",
        "    Ground Temp Sch,              !-Ground Surface 1 Temperature Schedule Name",
        "    ;                             !-Ground Surface 1 Reflectance Schedule Name",

        "  SurfaceProperty:SurroundingSurfaces,",
        "    SrdSurfs:Living:South,        !- Name",
        "    ,                             !- Sky View Factor",
        "    ,                             !- Sky Temperature Schedule Name",
        "    ,                             !- Ground View Factor",
        "    ,                             !- Ground Temperature Schedule Name",
        "    SurroundingSurface1,          !- Surrounding Surface 1 Name",
        "    0.4,                          !- Surrounding Surface 1 View Factor",
        "    Surrounding Temp Sch 1;       !- Surrounding Surface 1 Temperature Schedule Name",

        "  SurfaceProperty:GroundSurfaces,",
        "    GndSurfs:Living:South,        !-Name",
        "    GndSurfs SouthGrassArea,      !-Ground Surface 1 Name",
        "    ,                             !-Ground Surface 1 View Factor",
        "    Ground Temp Sch,              !-Ground Surface 1 Temperature Schedule Name",
        "    ;                             !-Ground Surface 1 Reflectance Schedule Name",

        "  ScheduleTypeLimits,",
        "    Any Number;                   !- Name",

        "  Schedule:Compact,",
        "    Surrounding Temp Sch 1,       !- Name",
        "    Any Number,                   !- Schedule Type Limits Name",
        "    Through: 12/31,               !- Field 1",
        "    For: AllDays,                 !- Field 2",
        "    Until: 24:00, 15.0;           !- Field 3",

        "  Schedule:Compact,",
        "    Sky Temp Sch,                 !- Name",
        "    Any Number,                   !- Schedule Type Limits Name",
        "    Through: 12/31,               !- Field 1",
        "    For: AllDays,                 !- Field 2",
        "    Until: 24:00, 15.0;           !- Field 3",

        "  Schedule:Compact,",
        "    Ground Temp Sch,              !- Name",
        "    Any Number,                   !- Schedule Type Limits Name",
        "    Through: 12/31,               !- Field 1",
        "    For: AllDays,                 !- Field 2",
        "    Until: 24:00, 15.0;           !- Field 3",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound = false;
    HeatBalanceManager::GetProjectControlData(*state, ErrorsFound); // read project control data
    EXPECT_FALSE(ErrorsFound);                                      // expect no errors
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    Material::GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    SurfaceGeometry::GetGeometryParameters(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    state->dataSurfaceGeometry->CosBldgRotAppGonly = 1.0;
    state->dataSurfaceGeometry->SinBldgRotAppGonly = 0.0;
    SurfaceGeometry::SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    HeatBalanceIntRadExchange::InitSolarViewFactors(*state);
    EXPECT_FALSE(has_err_output(true));
    EXPECT_TRUE(state->dataGlobal->AnyLocalEnvironmentsInModel);

    state->dataEnvrn->OutBaroPress = 101325.0;
    std::vector<int> controlledZoneEquipConfigNums;
    controlledZoneEquipConfigNums.push_back(1);
    state->dataHeatBal->Zone(1).IsControlled = true;
    state->dataHeatBal->Zone(1).SystemZoneNodeNumber = 5;

    state->dataZoneEquip->ZoneEquipConfig.allocate(1);
    auto &zoneEquipConfig = state->dataZoneEquip->ZoneEquipConfig(1);

    zoneEquipConfig.ZoneName = "LIVING ZONE";
    zoneEquipConfig.NumInletNodes = 2;
    zoneEquipConfig.InletNode.allocate(2);
    zoneEquipConfig.InletNode(1) = 1;
    zoneEquipConfig.InletNode(2) = 2;
    zoneEquipConfig.NumReturnNodes = 1;
    zoneEquipConfig.ReturnNode.allocate(1);
    zoneEquipConfig.ReturnNode(1) = 3;
    zoneEquipConfig.NumExhaustNodes = 1;
    zoneEquipConfig.ExhaustNode.allocate(1);
    zoneEquipConfig.ExhaustNode(1) = 4;
    zoneEquipConfig.FixedReturnFlow.allocate(1);

    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->spaceHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 24.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.001;
    state->dataZoneTempPredictorCorrector->spaceHeatBalance(1).MAT = 24.0;
    state->dataZoneTempPredictorCorrector->spaceHeatBalance(1).airHumRat = 0.001;

    state->dataLoopNodes->Node.allocate(4);
    auto &InletNode1 = state->dataLoopNodes->Node(1);
    auto &InletNode2 = state->dataLoopNodes->Node(2);
    auto &ExhaustNode = state->dataLoopNodes->Node(3);
    auto &ReturnNode = state->dataLoopNodes->Node(4);

    InletNode1.Temp = 20.0;
    InletNode2.Temp = 20.0;
    ReturnNode.Temp = 20.0;
    ExhaustNode.Temp = 20.0;
    InletNode1.MassFlowRate = 0.1;
    InletNode2.MassFlowRate = 0.1;
    ReturnNode.MassFlowRate = 0.1;
    ExhaustNode.MassFlowRate = 0.1;

    state->dataHeatBal->ZoneWinHeatGain.allocate(1);
    state->dataHeatBal->ZoneWinHeatGainRep.allocate(1);
    state->dataHeatBal->ZoneWinHeatGainRepEnergy.allocate(1);

    state->dataSurface->TotSurfaces = 6;
    // set convective coefficient adjustment ratio to 1.0
    state->dataHeatBalSurf->SurfWinCoeffAdjRatio.dimension(state->dataSurface->TotSurfaces, 1.0);
    state->dataMstBal->HConvInFD.allocate(state->dataSurface->TotSurfaces);
    state->dataMstBal->RhoVaporAirIn.allocate(state->dataSurface->TotSurfaces);
    state->dataMstBal->HMassConvInFD.allocate(state->dataSurface->TotSurfaces);
    state->dataHeatBalSurf->SurfHConvInt.allocate(state->dataSurface->TotSurfaces);
    state->dataHeatBalSurf->SurfHConvInt(1) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(2) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(3) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(4) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(5) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(6) = 0.5;
    state->dataHeatBalSurf->SurfTempInTmp.allocate(state->dataSurface->TotSurfaces);
    state->dataHeatBalSurf->SurfTempInTmp(1) = 15.0;
    state->dataHeatBalSurf->SurfTempInTmp(2) = 20.0;
    state->dataHeatBalSurf->SurfTempInTmp(3) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(4) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(5) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(6) = 25.0;

    state->dataGlobal->BeginSimFlag = true;
    state->dataGlobal->KickOffSimulation = true;
    state->dataGlobal->TimeStepZoneSec = 900;

    AllocateSurfaceHeatBalArrays(*state);
    createFacilityElectricPowerServiceObject(*state);
    HeatBalanceManager::AllocateZoneHeatBalArrays(*state);
    SolarShading::AllocateModuleArrays(*state);
    SolarShading::DetermineShadowingCombinations(*state);

    state->dataSurface->SurfTAirRef(1) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    state->dataSurface->SurfTAirRef(2) = DataSurfaces::RefAirTemp::AdjacentAirTemp;
    state->dataSurface->SurfTAirRef(3) = DataSurfaces::RefAirTemp::ZoneSupplyAirTemp;

    // default surface view factors
    int SurfNum = Util::FindItemInList("LIVING:NORTH", state->dataSurface->Surface);
    auto &Surface_Living_North = state->dataSurface->Surface(SurfNum);
    SurfNum = Util::FindItemInList("LIVING:EAST", state->dataSurface->Surface);
    auto &Surface_Living_East = state->dataSurface->Surface(SurfNum);
    SurfNum = Util::FindItemInList("LIVING:SOUTH", state->dataSurface->Surface);
    auto &Surface_Living_South = state->dataSurface->Surface(SurfNum);

    // check exterior default surfaces sky and ground view factors
    EXPECT_DOUBLE_EQ(0.5, Surface_Living_North.ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.5, Surface_Living_North.ViewFactorGroundIR);
    EXPECT_DOUBLE_EQ(0.5, Surface_Living_East.ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.5, Surface_Living_East.ViewFactorGroundIR);
    EXPECT_DOUBLE_EQ(0.5, Surface_Living_South.ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.5, Surface_Living_South.ViewFactorGroundIR);

    // reset sky and ground view factors
    InitSurfacePropertyViewFactors(*state);
    // call to reset surface view factors
    InitSurfaceHeatBalance(*state);
    state->dataSurface->SurfAirSkyRadSplit.allocate(6);
    Sched::GetSchedule(*state, "SURROUNDING TEMP SCH 1")->currentVal = 25.0; // Srd Srfs Temp
    Sched::GetSchedule(*state, "SKY TEMP SCH")->currentVal = 15.0;           // Sky temp
    Sched::GetSchedule(*state, "GROUND TEMP SCH")->currentVal = 22.0;        // Grd temp

    for (int SurfNum = 1; SurfNum <= 6; SurfNum++) {
        state->dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum) = 20; // Surf temp
        state->dataSurface->SurfOutDryBulbTemp(SurfNum) = 22;         // Air temp
        state->dataSurface->surfExtConv(SurfNum).model = Convect::HcExt::MoWiTTHcOutside;
        state->dataSurface->SurfAirSkyRadSplit(SurfNum) = 1.0;
    }

    // test reset of surface view factors are correct
    EXPECT_DOUBLE_EQ(0.3, Surface_Living_North.ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.5, Surface_Living_North.ViewFactorGroundIR);
    EXPECT_DOUBLE_EQ(0.2, Surface_Living_East.ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.2, Surface_Living_East.ViewFactorGroundIR);
    EXPECT_DOUBLE_EQ(0.3, Surface_Living_South.ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.3, Surface_Living_South.ViewFactorGroundIR);

    // calculate outside surface heat balance
    CalcHeatBalanceOutsideSurf(*state);

    // define expected result variables
    Real64 result_LWRExchangeCoeff_surf1 = 0.0;
    Real64 result_LWRExchangeCoeff_surf2 = 0.0;
    Real64 result_LWRExchangeCoeff_surf3 = 0.0;
    // set exterior surface and ground surface temperatures
    Real64 surf_TK = 20.0 + Constant::Kelvin;
    Real64 grnd_TK = 22.0 + Constant::Kelvin;
    // calculate LWR exchange coefficent from exterior surface to ground
    result_LWRExchangeCoeff_surf1 = Constant::StefanBoltzmann * 0.9 * 0.5 * (pow_4(surf_TK) - pow_4(grnd_TK)) / (surf_TK - grnd_TK);
    result_LWRExchangeCoeff_surf2 = Constant::StefanBoltzmann * 0.9 * 0.2 * (pow_4(surf_TK) - pow_4(grnd_TK)) / (surf_TK - grnd_TK);
    result_LWRExchangeCoeff_surf3 = Constant::StefanBoltzmann * 0.9 * 0.3 * (pow_4(surf_TK) - pow_4(grnd_TK)) / (surf_TK - grnd_TK);
    // test LWR exchange coefficents b/n exterior wall and ground surfaces
    EXPECT_DOUBLE_EQ(result_LWRExchangeCoeff_surf1, state->dataHeatBalSurf->SurfHGrdExt(1));
    EXPECT_DOUBLE_EQ(result_LWRExchangeCoeff_surf2, state->dataHeatBalSurf->SurfHGrdExt(2));
    EXPECT_DOUBLE_EQ(result_LWRExchangeCoeff_surf3, state->dataHeatBalSurf->SurfHGrdExt(3));
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_TestGroundSurfsAverageTemp)
{
    bool ErrorsFound(false);
    std::string const idf_objects =
        delimited_string({"  Material,",
                          "    Concrete Block,               !- Name",
                          "    MediumRough,                  !- Roughness",
                          "    0.1014984,                    !- Thickness {m}",
                          "    0.3805070,                    !- Conductivity {W/m-K}",
                          "    608.7016,                     !- Density {kg/m3}",
                          "    836.8000;                     !- Specific Heat {J/kg-K}",

                          "  Construction,",
                          "    WallConstruction,             !- Name",
                          "    Concrete Block;               !- Outside Layer",

                          "  WindowMaterial:SimpleGlazingSystem,",
                          "    WindowMaterial,               !- Name",
                          "    5.778,                        !- U-Factor {W/m2-K}",
                          "    0.819,                        !- Solar Heat Gain Coefficient",
                          "    0.881;                        !- Visible Transmittance",

                          "  Construction,",
                          "    WindowConstruction,           !- Name",
                          "    WindowMaterial;               !- Outside Layer",

                          "  WindowProperty:FrameAndDivider,",
                          "    WindowFrame,                  !- Name",
                          "    0.05,                         !- Frame Width {m}",
                          "    0.00,                         !- Frame Outside Projection {m}",
                          "    0.00,                         !- Frame Inside Projection {m}",
                          "    5.0,                          !- Frame Conductance {W/m2-K}",
                          "    1.2,                          !- Ratio of Frame-Edge Glass Conductance to Center-Of-Glass Conductance",
                          "    0.8,                          !- Frame Solar Absorptance",
                          "    0.8,                          !- Frame Visible Absorptance",
                          "    0.9,                          !- Frame Thermal Hemispherical Emissivity",
                          "    DividedLite,                  !- Divider Type",
                          "    0.02,                         !- Divider Width {m}",
                          "    2,                            !- Number of Horizontal Dividers",
                          "    2,                            !- Number of Vertical Dividers",
                          "    0.00,                         !- Divider Outside Projection {m}",
                          "    0.00,                         !- Divider Inside Projection {m}",
                          "    5.0,                          !- Divider Conductance {W/m2-K}",
                          "    1.2,                          !- Ratio of Divider-Edge Glass Conductance to Center-Of-Glass Conductance",
                          "    0.8,                          !- Divider Solar Absorptance",
                          "    0.8,                          !- Divider Visible Absorptance",
                          "    0.9;                          !- Divider Thermal Hemispherical Emissivity",

                          "  FenestrationSurface:Detailed,",
                          "    FenestrationSurface,          !- Name",
                          "    Window,                       !- Surface Type",
                          "    WindowConstruction,           !- Construction Name",
                          "    Wall,                         !- Building Surface Name",
                          "    ,                             !- Outside Boundary Condition Object",
                          "    0.5000000,                    !- View Factor to Ground",
                          "    WindowFrame,                  !- Frame and Divider Name",
                          "    1.0,                          !- Multiplier",
                          "    4,                            !- Number of Vertices",
                          "    0.200000,0.0,9.900000,        !- X,Y,Z ==> Vertex 1 {m}",
                          "    0.200000,0.0,0.1000000,       !- X,Y,Z ==> Vertex 2 {m}",
                          "    9.900000,0.0,0.1000000,       !- X,Y,Z ==> Vertex 3 {m}",
                          "    9.900000,0.0,9.900000;        !- X,Y,Z ==> Vertex 4 {m}",

                          "  SurfaceProperty:LocalEnvironment,",
                          "    LocEnv:FenestrationSurface,   !- Name",
                          "    FenestrationSurface,          !- Exterior Surface Name",
                          "    ,                             !- External Shading Fraction Schedule Name",
                          "    SrdSurfs:FenesSurface,        !- Surrounding Surfaces Object Name",
                          "    ,                             !- Outdoor Air Node Name",
                          "    GndSurfs:FenesSurface;        !- Ground Surfaces Object Name",

                          "  SurfaceProperty:SurroundingSurfaces,",
                          "    SrdSurfs:FenesSurface,        !- Name",
                          "    0.5,                          !- Sky View Factor",
                          "    Sky Temp Sch,                 !- Sky Temperature Schedule Name",
                          "    ,                             !- Ground View Factor",
                          "    ,                             !- Ground Temperature Schedule Name",
                          "    SrdSurfs:Surface 1,           !- Surrounding Surface 1 Name",
                          "    0.1,                          !- Surrounding Surface 1 View Factor",
                          "    Surrounding Temp Sch 1;       !- Surrounding Surface 1 Temperature Schedule Name",

                          "  SurfaceProperty:GroundSurfaces,",
                          "    GndSurfs:FenesSurface,        !-Name",
                          "    GndSurfs GrassArea,           !-Ground Surface 1 Name",
                          "    0.2,                          !-Ground Surface 1 View Factor",
                          "    GrassArea Ground Temp Sch,    !-Ground Surface 1 Temperature Schedule Name",
                          "    ,                             !-Ground Surface 1 Reflectance Schedule Name",
                          "    GndSurfs ParkingArea,         !-Ground Surface 2 Name",
                          "    0.1,                          !-Ground Surface 2 View Factor",
                          "    ParkingArea Ground Temp Sch,  !-Ground Surface 2 Temperature Schedule Name",
                          "    ,                             !-Ground Surface 2 Reflectance Schedule Name",
                          "    GndSurfs LakeArea,            !-Ground Surface 3 Name",
                          "    0.1,                          !-Ground Surface 3 View Factor",
                          "    LakeArea Ground Temp Sch,     !-Ground Surface 3 Temperature Schedule Name",
                          "    ;                             !-Ground Surface 3 Reflectance Schedule Name",

                          "  Schedule:Compact,",
                          "    Surrounding Temp Sch 1,       !- Name",
                          "    Temperature,                  !- Schedule Type Limits Name",
                          "    Through: 12/31,               !- Field 1",
                          "    For: AllDays,                 !- Field 2",
                          "    Until: 24:00, 26.0;           !- Field 3",

                          "  Schedule:Compact,",
                          "    GrassArea Ground Temp Sch,    !- Name",
                          "    Temperature,                  !- Schedule Type Limits Name",
                          "    Through: 12/31,               !- Field 1",
                          "    For: AllDays,                 !- Field 2",
                          "    Until: 24:00, 25.0;           !- Field 3",

                          "  Schedule:Compact,",
                          "    ParkingArea Ground Temp Sch,  !- Name",
                          "    Temperature,                  !- Schedule Type Limits Name",
                          "    Through: 12/31,               !- Field 1",
                          "    For: AllDays,                 !- Field 2",
                          "    Until: 24:00, 28.0;           !- Field 3",

                          "  Schedule:Compact,",
                          "    LakeArea Ground Temp Sch,     !- Name",
                          "    Temperature,                  !- Schedule Type Limits Name",
                          "    Through: 12/31,               !- Field 1",
                          "    For: AllDays,                 !- Field 2",
                          "    Until: 24:00, 22.0;           !- Field 3",

                          "  Schedule:Compact,",
                          "    Sky Temp Sch,                 !- Name",
                          "    Temperature,                  !- Schedule Type Limits Name",
                          "    Through: 12/31,               !- Field 1",
                          "    For: AllDays,                 !- Field 2",
                          "    Until: 24:00, 20.0;           !- Field 3",

                          "  ScheduleTypeLimits,",
                          "    Temperature,             !- Name",
                          "    -60,                     !- Lower Limit Value",
                          "    200,                     !- Upper Limit Value",
                          "    CONTINUOUS,              !- Numeric Type",
                          "    Temperature;             !- Unit Type",

                          "  BuildingSurface:Detailed,",
                          "    Wall,                         !- Name",
                          "    Wall,                         !- Surface Type",
                          "    WallConstruction,             !- Construction Name",
                          "    Zone,                         !- Zone Name",
                          "    ,                             !- Space Name",
                          "    Outdoors,                     !- Outside Boundary Condition",
                          "    ,                             !- Outside Boundary Condition Object",
                          "    SunExposed,                   !- Sun Exposure",
                          "    WindExposed,                  !- Wind Exposure",
                          "    0.5000000,                    !- View Factor to Ground",
                          "    4,                            !- Number of Vertices",
                          "    0.0,0.000000,10.00000,        !- X,Y,Z ==> Vertex 1 {m}",
                          "    0.0,0.000000,0.0,             !- X,Y,Z ==> Vertex 2 {m}",
                          "    10.00000,0.0,0.0,             !- X,Y,Z ==> Vertex 3 {m}",
                          "    10.00000,0.0,10.00000;        !- X,Y,Z ==> Vertex 4 {m}",

                          "  BuildingSurface:Detailed,"
                          "    Floor,                        !- Name",
                          "    Floor,                        !- Surface Type",
                          "    WallConstruction,             !- Construction Name",
                          "    Zone,                         !- Zone Name",
                          "    ,                             !- Space Name",
                          "    Outdoors,                     !- Outside Boundary Condition",
                          "    ,                             !- Outside Boundary Condition Object",
                          "    NoSun,                        !- Sun Exposure",
                          "    NoWind,                       !- Wind Exposure",
                          "    1.0,                          !- View Factor to Ground",
                          "    4,                            !- Number of Vertices",
                          "    0.000000,0.000000,0,          !- X,Y,Z ==> Vertex 1 {m}",
                          "    0.000000,10.000000,0,         !- X,Y,Z ==> Vertex 2 {m}",
                          "    10.00000,10.000000,0,         !- X,Y,Z ==> Vertex 3 {m}",
                          "    10.00000,0.000000,0;          !- X,Y,Z ==> Vertex 4 {m}",

                          "  Zone,"
                          "    Zone,                         !- Name",
                          "    0,                            !- Direction of Relative North {deg}",
                          "    6.000000,                     !- X Origin {m}",
                          "    6.000000,                     !- Y Origin {m}",
                          "    0,                            !- Z Origin {m}",
                          "    1,                            !- Type",
                          "    1,                            !- Multiplier",
                          "    autocalculate,                !- Ceiling Height {m}",
                          "    autocalculate;                !- Volume {m3}"});

    ASSERT_TRUE(process_idf(idf_objects));
    state->dataGlobal->TimeStepZone = 1;
    state->dataGlobal->TimeStepZoneSec = 3600.0;
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    // set global and environmental variables
    state->dataGlobal->BeginSimFlag = true;
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataGlobal->HourOfDay = 15;
    state->dataGlobal->TimeStep = 1;
    state->dataEnvrn->Month = 7;
    state->dataEnvrn->DayOfMonth = 21;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    state->dataEnvrn->OutBaroPress = 100000;

    state->dataHeatBal->ZoneIntGain.allocate(1);
    createFacilityElectricPowerServiceObject(*state);
    HeatBalanceManager::SetPreConstructionInputParameters(*state);
    HeatBalanceManager::GetProjectControlData(*state, ErrorsFound);
    HeatBalanceManager::GetFrameAndDividerData(*state);
    Material::GetMaterialData(*state, ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound);
    HeatBalanceManager::GetBuildingData(*state, ErrorsFound);

    HeatBalanceManager::AllocateHeatBalArrays(*state);
    HeatBalanceSurfaceManager::AllocateSurfaceHeatBalArrays(*state);

    EXPECT_FALSE(ErrorsFound);
    EXPECT_TRUE(state->dataGlobal->AnyLocalEnvironmentsInModel);
    // test surface property object inputs
    int SrdSurfsNum = Util::FindItemInList("SRDSURFS:FENESSURFACE", state->dataSurface->SurroundingSurfsProperty);
    EXPECT_EQ(1, state->dataSurface->SurfLocalEnvironment(SrdSurfsNum).SurroundingSurfsPtr);
    int GndSurfsNum = Util::FindItemInList("GNDSURFS:FENESSURFACE", state->dataSurface->GroundSurfsProperty);
    EXPECT_EQ(1, state->dataSurface->SurfLocalEnvironment(GndSurfsNum).GroundSurfsPtr);
    // set local derived data vars
    int SurfNum = Util::FindItemInList("FENESTRATIONSURFACE", state->dataSurface->Surface);
    SrdSurfsNum = state->dataSurface->Surface(SurfNum).SurfSurroundingSurfacesNum;
    auto &SrdSurfsProperty = state->dataSurface->SurroundingSurfsProperty(SrdSurfsNum);
    GndSurfsNum = state->dataSurface->Surface(SurfNum).SurfPropertyGndSurfIndex;
    auto &GndSurfsProperty = state->dataSurface->GroundSurfsProperty(GndSurfsNum);
    // check sky view factors
    EXPECT_DOUBLE_EQ(0.5, SrdSurfsProperty.SkyViewFactor);
    // check surrounding surfaces view factors
    EXPECT_DOUBLE_EQ(0.1, SrdSurfsProperty.SurroundingSurfs(1).ViewFactor);
    // check ground surfaces view factors
    EXPECT_EQ("GNDSURFS GRASSAREA", GndSurfsProperty.GndSurfs(1).Name);
    EXPECT_DOUBLE_EQ(0.2, GndSurfsProperty.GndSurfs(1).ViewFactor);
    EXPECT_EQ("GNDSURFS PARKINGAREA", GndSurfsProperty.GndSurfs(2).Name);
    EXPECT_DOUBLE_EQ(0.1, GndSurfsProperty.GndSurfs(2).ViewFactor);
    EXPECT_EQ("GNDSURFS LAKEAREA", GndSurfsProperty.GndSurfs(3).Name);
    EXPECT_DOUBLE_EQ(0.1, GndSurfsProperty.GndSurfs(3).ViewFactor);
    EXPECT_DOUBLE_EQ(0.4, GndSurfsProperty.SurfsViewFactorSum);

    Sched::UpdateScheduleVals(*state);

    // check ground temperature values
    Real64 const Tgndsurf_grass = GndSurfsProperty.GndSurfs(1).tempSched->getCurrentVal();
    Real64 const Tgndsurf_parking = GndSurfsProperty.GndSurfs(2).tempSched->getCurrentVal();
    Real64 const Tgndsurf_lake = GndSurfsProperty.GndSurfs(3).tempSched->getCurrentVal();
    EXPECT_DOUBLE_EQ(25.0, Tgndsurf_grass);
    EXPECT_DOUBLE_EQ(28.0, Tgndsurf_parking);
    EXPECT_DOUBLE_EQ(22.0, Tgndsurf_lake);

    // test 1: surface viewing grass, parking and lake areas
    // calculate ground surfaces average temperature
    GetGroundSurfacesTemperatureAverage(*state);
    Real64 dTK = Constant::Kelvin;
    Real64 results_gndSurfsAvgTemp = 0.0;
    results_gndSurfsAvgTemp = root_4((0.2 * pow_4(25.0 + dTK) + 0.1 * pow_4(28.0 + dTK) + 0.1 * pow_4(22.0 + dTK)) / (0.2 + 0.1 + 0.1)) - dTK;
    // check ground surfaces average temperature
    EXPECT_DOUBLE_EQ(results_gndSurfsAvgTemp, GndSurfsProperty.SurfsTempAvg);

    // test 2: surface viewing grass area only
    GndSurfsProperty.GndSurfs(1).ViewFactor = 0.4;
    GndSurfsProperty.GndSurfs(2).ViewFactor = 0.0;
    GndSurfsProperty.GndSurfs(3).ViewFactor = 0.0;
    // calculate ground surfaces average temperature
    GetGroundSurfacesTemperatureAverage(*state);
    results_gndSurfsAvgTemp = 25.0;
    // check grass area ground surface temperature
    EXPECT_DOUBLE_EQ(results_gndSurfsAvgTemp, GndSurfsProperty.SurfsTempAvg);

    // test 3: surface viewing parking area only
    GndSurfsProperty.GndSurfs(1).ViewFactor = 0.0;
    GndSurfsProperty.GndSurfs(2).ViewFactor = 0.4;
    GndSurfsProperty.GndSurfs(3).ViewFactor = 0.0;
    // calculate ground surfaces average temperature
    GetGroundSurfacesTemperatureAverage(*state);
    results_gndSurfsAvgTemp = 28.0;
    // check parking area ground surface temperature
    EXPECT_DOUBLE_EQ(results_gndSurfsAvgTemp, GndSurfsProperty.SurfsTempAvg);

    // test 4: surface viewing lake area only
    GndSurfsProperty.GndSurfs(1).ViewFactor = 0.0;
    GndSurfsProperty.GndSurfs(2).ViewFactor = 0.0;
    GndSurfsProperty.GndSurfs(3).ViewFactor = 0.4;
    // calculate ground surfaces average temperature
    GetGroundSurfacesTemperatureAverage(*state);
    results_gndSurfsAvgTemp = 22.0;
    // check lake area ground surface temperature
    EXPECT_DOUBLE_EQ(results_gndSurfsAvgTemp, GndSurfsProperty.SurfsTempAvg);
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_TestGroundSurfsAverageRefl)
{
    bool ErrorsFound(false);
    std::string const idf_objects =
        delimited_string({"  Material,",
                          "    Concrete Block,               !- Name",
                          "    MediumRough,                  !- Roughness",
                          "    0.1014984,                    !- Thickness {m}",
                          "    0.3805070,                    !- Conductivity {W/m-K}",
                          "    608.7016,                     !- Density {kg/m3}",
                          "    836.8000;                     !- Specific Heat {J/kg-K}",

                          "  Construction,",
                          "    WallConstruction,             !- Name",
                          "    Concrete Block;               !- Outside Layer",

                          "  WindowMaterial:SimpleGlazingSystem,",
                          "    WindowMaterial,               !- Name",
                          "    5.778,                        !- U-Factor {W/m2-K}",
                          "    0.819,                        !- Solar Heat Gain Coefficient",
                          "    0.881;                        !- Visible Transmittance",

                          "  Construction,",
                          "    WindowConstruction,           !- Name",
                          "    WindowMaterial;               !- Outside Layer",

                          "  WindowProperty:FrameAndDivider,",
                          "    WindowFrame,                  !- Name",
                          "    0.05,                         !- Frame Width {m}",
                          "    0.00,                         !- Frame Outside Projection {m}",
                          "    0.00,                         !- Frame Inside Projection {m}",
                          "    5.0,                          !- Frame Conductance {W/m2-K}",
                          "    1.2,                          !- Ratio of Frame-Edge Glass Conductance to Center-Of-Glass Conductance",
                          "    0.8,                          !- Frame Solar Absorptance",
                          "    0.8,                          !- Frame Visible Absorptance",
                          "    0.9,                          !- Frame Thermal Hemispherical Emissivity",
                          "    DividedLite,                  !- Divider Type",
                          "    0.02,                         !- Divider Width {m}",
                          "    2,                            !- Number of Horizontal Dividers",
                          "    2,                            !- Number of Vertical Dividers",
                          "    0.00,                         !- Divider Outside Projection {m}",
                          "    0.00,                         !- Divider Inside Projection {m}",
                          "    5.0,                          !- Divider Conductance {W/m2-K}",
                          "    1.2,                          !- Ratio of Divider-Edge Glass Conductance to Center-Of-Glass Conductance",
                          "    0.8,                          !- Divider Solar Absorptance",
                          "    0.8,                          !- Divider Visible Absorptance",
                          "    0.9;                          !- Divider Thermal Hemispherical Emissivity",

                          "  FenestrationSurface:Detailed,",
                          "    FenestrationSurface,          !- Name",
                          "    Window,                       !- Surface Type",
                          "    WindowConstruction,           !- Construction Name",
                          "    Wall,                         !- Building Surface Name",
                          "    ,                             !- Outside Boundary Condition Object",
                          "    0.5000000,                    !- View Factor to Ground",
                          "    WindowFrame,                  !- Frame and Divider Name",
                          "    1.0,                          !- Multiplier",
                          "    4,                            !- Number of Vertices",
                          "    0.200000,0.0,9.900000,        !- X,Y,Z ==> Vertex 1 {m}",
                          "    0.200000,0.0,0.1000000,       !- X,Y,Z ==> Vertex 2 {m}",
                          "    9.900000,0.0,0.1000000,       !- X,Y,Z ==> Vertex 3 {m}",
                          "    9.900000,0.0,9.900000;        !- X,Y,Z ==> Vertex 4 {m}",

                          "  SurfaceProperty:LocalEnvironment,",
                          "    LocEnv:FenestrationSurface,   !- Name",
                          "    FenestrationSurface,          !- Exterior Surface Name",
                          "    ,                             !- External Shading Fraction Schedule Name",
                          "    SrdSurfs:FenesSurface,        !- Surrounding Surfaces Object Name",
                          "    ,                             !- Outdoor Air Node Name",
                          "    GndSurfs:FenesSurface;        !- Ground Surfaces Object Name",

                          "  SurfaceProperty:SurroundingSurfaces,",
                          "    SrdSurfs:FenesSurface,        !- Name",
                          "    0.5,                          !- Sky View Factor",
                          "    Sky Temp Sch,                 !- Sky Temperature Schedule Name",
                          "    ,                             !- Ground View Factor",
                          "    ,                             !- Ground Temperature Schedule Name",
                          "    SrdSurfs:Surface 1,           !- Surrounding Surface 1 Name",
                          "    0.1,                          !- Surrounding Surface 1 View Factor",
                          "    Surrounding Temp Sch 1;       !- Surrounding Surface 1 Temperature Schedule Name",

                          "  SurfaceProperty:GroundSurfaces,",
                          "    GndSurfs:FenesSurface,        !-Name",
                          "    GndSurfs GrassArea,           !-Ground Surface 1 Name",
                          "    0.2,                          !-Ground Surface 1 View Factor",
                          "    GrassArea Ground Temp Sch,    !-Ground Surface 1 Temperature Schedule Name",
                          "    GrassArea Ground Refl Sch,    !-Ground Surface 1 Reflectance Schedule Name",
                          "    GndSurfs ParkingArea,         !-Ground Surface 2 Name",
                          "    0.1,                          !-Ground Surface 2 View Factor",
                          "    ParkingArea Ground Temp Sch,  !-Ground Surface 2 Temperature Schedule Name",
                          "    ParkingArea Ground Refl Sch,  !-Ground Surface 2 Reflectance Schedule Name",
                          "    GndSurfs LakeArea,            !-Ground Surface 3 Name",
                          "    0.1,                          !-Ground Surface 3 View Factor",
                          "    LakeArea Ground Temp Sch,     !-Ground Surface 3 Temperature Schedule Name",
                          "    LakeArea Ground Refl Sch;     !-Ground Surface 3 Reflectance Schedule Name",

                          "  Schedule:Compact,",
                          "    Surrounding Temp Sch 1,       !- Name",
                          "    Temperature,                  !- Schedule Type Limits Name",
                          "    Through: 12/31,               !- Field 1",
                          "    For: AllDays,                 !- Field 2",
                          "    Until: 24:00, 26.0;           !- Field 3",

                          "  Schedule:Compact,",
                          "    GrassArea Ground Temp Sch,    !- Name",
                          "    Temperature,                  !- Schedule Type Limits Name",
                          "    Through: 12/31,               !- Field 1",
                          "    For: AllDays,                 !- Field 2",
                          "    Until: 24:00, 25.0;           !- Field 3",

                          "  Schedule:Compact,",
                          "    ParkingArea Ground Temp Sch,  !- Name",
                          "    Temperature,                  !- Schedule Type Limits Name",
                          "    Through: 12/31,               !- Field 1",
                          "    For: AllDays,                 !- Field 2",
                          "    Until: 24:00, 28.0;           !- Field 3",

                          "  Schedule:Compact,",
                          "    LakeArea Ground Temp Sch,     !- Name",
                          "    Temperature,                  !- Schedule Type Limits Name",
                          "    Through: 12/31,               !- Field 1",
                          "    For: AllDays,                 !- Field 2",
                          "    Until: 24:00, 22.0;           !- Field 3",

                          "  Schedule:Compact,",
                          "    Sky Temp Sch,                 !- Name",
                          "    Temperature,                  !- Schedule Type Limits Name",
                          "    Through: 12/31,               !- Field 1",
                          "    For: AllDays,                 !- Field 2",
                          "    Until: 24:00, 20.0;           !- Field 3",

                          "  ScheduleTypeLimits,",
                          "    Temperature,                  !- Name",
                          "    -60,                          !- Lower Limit Value",
                          "    200,                          !- Upper Limit Value",
                          "    CONTINUOUS,                   !- Numeric Type",
                          "    Temperature;                  !- Unit Type",

                          "  Schedule:Compact,",
                          "    GrassArea Ground Refl Sch,    !- Name",
                          "    Fraction,                     !- Schedule Type Limits Name",
                          "    Through: 12/31,               !- Field 1",
                          "    For: AllDays,                 !- Field 2",
                          "    Until: 24:00, 0.25;           !- Field 3",

                          "  Schedule:Compact,",
                          "    ParkingArea Ground Refl Sch,  !- Name",
                          "    Fraction,                     !- Schedule Type Limits Name",
                          "    Through: 12/31,               !- Field 1",
                          "    For: AllDays,                 !- Field 2",
                          "    Until: 24:00, 0.50;           !- Field 3",

                          "  Schedule:Compact,",
                          "    LakeArea Ground Refl Sch,     !- Name",
                          "    Fraction,                     !- Schedule Type Limits Name",
                          "    Through: 12/31,               !- Field 1",
                          "    For: AllDays,                 !- Field 2",
                          "    Until: 24:00, 0.10;           !- Field 3",

                          "  ScheduleTypeLimits,",
                          "    Fraction,                     !- Name",
                          "    0,                            !- Lower Limit Value",
                          "    1,                            !- Upper Limit Value",
                          "    CONTINUOUS;                   !- Numeric Type",

                          "  BuildingSurface:Detailed,",
                          "    Wall,                         !- Name",
                          "    Wall,                         !- Surface Type",
                          "    WallConstruction,             !- Construction Name",
                          "    Zone,                         !- Zone Name",
                          "    ,                             !- Space Name",
                          "    Outdoors,                     !- Outside Boundary Condition",
                          "    ,                             !- Outside Boundary Condition Object",
                          "    SunExposed,                   !- Sun Exposure",
                          "    WindExposed,                  !- Wind Exposure",
                          "    0.5000000,                    !- View Factor to Ground",
                          "    4,                            !- Number of Vertices",
                          "    0.0,0.000000,10.00000,        !- X,Y,Z ==> Vertex 1 {m}",
                          "    0.0,0.000000,0.0,             !- X,Y,Z ==> Vertex 2 {m}",
                          "    10.00000,0.0,0.0,             !- X,Y,Z ==> Vertex 3 {m}",
                          "    10.00000,0.0,10.00000;        !- X,Y,Z ==> Vertex 4 {m}",

                          "  BuildingSurface:Detailed,"
                          "    Floor,                        !- Name",
                          "    Floor,                        !- Surface Type",
                          "    WallConstruction,             !- Construction Name",
                          "    Zone,                         !- Zone Name",
                          "    ,                             !- Space Name",
                          "    Outdoors,                     !- Outside Boundary Condition",
                          "    ,                             !- Outside Boundary Condition Object",
                          "    NoSun,                        !- Sun Exposure",
                          "    NoWind,                       !- Wind Exposure",
                          "    1.0,                          !- View Factor to Ground",
                          "    4,                            !- Number of Vertices",
                          "    0.000000,0.000000,0,          !- X,Y,Z ==> Vertex 1 {m}",
                          "    0.000000,10.000000,0,         !- X,Y,Z ==> Vertex 2 {m}",
                          "    10.00000,10.000000,0,         !- X,Y,Z ==> Vertex 3 {m}",
                          "    10.00000,0.000000,0;          !- X,Y,Z ==> Vertex 4 {m}",

                          "  Zone,"
                          "    Zone,                         !- Name",
                          "    0,                            !- Direction of Relative North {deg}",
                          "    6.000000,                     !- X Origin {m}",
                          "    6.000000,                     !- Y Origin {m}",
                          "    0,                            !- Z Origin {m}",
                          "    1,                            !- Type",
                          "    1,                            !- Multiplier",
                          "    autocalculate,                !- Ceiling Height {m}",
                          "    autocalculate;                !- Volume {m3}"});

    ASSERT_TRUE(process_idf(idf_objects));
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->TimeStepZone = 1;
    state->dataGlobal->TimeStepZoneSec = 3600.0;
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    // set global and environmental variables
    state->dataGlobal->BeginSimFlag = true;
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataGlobal->HourOfDay = 15;
    state->dataEnvrn->Month = 7;
    state->dataEnvrn->DayOfMonth = 21;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    state->dataEnvrn->OutBaroPress = 100000;

    state->dataHeatBal->ZoneIntGain.allocate(1);
    createFacilityElectricPowerServiceObject(*state);
    HeatBalanceManager::SetPreConstructionInputParameters(*state);
    HeatBalanceManager::GetProjectControlData(*state, ErrorsFound);
    HeatBalanceManager::GetFrameAndDividerData(*state);
    Material::GetMaterialData(*state, ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound);
    HeatBalanceManager::GetBuildingData(*state, ErrorsFound);

    HeatBalanceManager::AllocateHeatBalArrays(*state);
    HeatBalanceSurfaceManager::AllocateSurfaceHeatBalArrays(*state);

    EXPECT_FALSE(ErrorsFound);
    EXPECT_TRUE(state->dataGlobal->AnyLocalEnvironmentsInModel);
    // test surface property object inputs
    int SrdSurfsNum = Util::FindItemInList("SRDSURFS:FENESSURFACE", state->dataSurface->SurroundingSurfsProperty);
    EXPECT_EQ(1, state->dataSurface->SurfLocalEnvironment(SrdSurfsNum).SurroundingSurfsPtr);
    int GndSurfsNum = Util::FindItemInList("GNDSURFS:FENESSURFACE", state->dataSurface->GroundSurfsProperty);
    EXPECT_EQ(1, state->dataSurface->SurfLocalEnvironment(GndSurfsNum).GroundSurfsPtr);
    // set local derived data vars
    int SurfNum = Util::FindItemInList("FENESTRATIONSURFACE", state->dataSurface->Surface);
    SrdSurfsNum = state->dataSurface->Surface(SurfNum).SurfSurroundingSurfacesNum;
    auto &SrdSurfsProperty = state->dataSurface->SurroundingSurfsProperty(SrdSurfsNum);
    GndSurfsNum = state->dataSurface->Surface(SurfNum).SurfPropertyGndSurfIndex;
    auto &GndSurfsProperty = state->dataSurface->GroundSurfsProperty(GndSurfsNum);
    // check sky view factors
    EXPECT_DOUBLE_EQ(0.5, SrdSurfsProperty.SkyViewFactor);
    // check surrounding surfaces view factors
    EXPECT_DOUBLE_EQ(0.1, SrdSurfsProperty.SurroundingSurfs(1).ViewFactor);
    // check ground surfaces view factors
    EXPECT_EQ("GNDSURFS GRASSAREA", GndSurfsProperty.GndSurfs(1).Name);
    EXPECT_DOUBLE_EQ(0.2, GndSurfsProperty.GndSurfs(1).ViewFactor);
    EXPECT_EQ("GNDSURFS PARKINGAREA", GndSurfsProperty.GndSurfs(2).Name);
    EXPECT_DOUBLE_EQ(0.1, GndSurfsProperty.GndSurfs(2).ViewFactor);
    EXPECT_EQ("GNDSURFS LAKEAREA", GndSurfsProperty.GndSurfs(3).Name);
    EXPECT_DOUBLE_EQ(0.1, GndSurfsProperty.GndSurfs(3).ViewFactor);
    EXPECT_DOUBLE_EQ(0.4, GndSurfsProperty.SurfsViewFactorSum);

    Sched::UpdateScheduleVals(*state);

    // check ground temperature values
    Real64 const Rgndsurf_grass = GndSurfsProperty.GndSurfs(1).reflSched->getCurrentVal();
    Real64 const Rgndsurf_parking = GndSurfsProperty.GndSurfs(2).reflSched->getCurrentVal();
    Real64 const Rgndsurf_lake = GndSurfsProperty.GndSurfs(3).reflSched->getCurrentVal();
    EXPECT_DOUBLE_EQ(0.25, Rgndsurf_grass);
    EXPECT_DOUBLE_EQ(0.5, Rgndsurf_parking);
    EXPECT_DOUBLE_EQ(0.1, Rgndsurf_lake);

    // test 1: surface viewing grass, parking and lake areas
    // calculate ground surfaces average reflectance
    GetGroundSurfacesReflectanceAverage(*state);
    Real64 results_gndSurfsAvgRefl = 0.0;
    results_gndSurfsAvgRefl = (0.2 * Rgndsurf_grass + 0.1 * Rgndsurf_parking + 0.1 * Rgndsurf_lake) / (0.2 + 0.1 + 0.1);
    // check ground surfaces average reflectance
    EXPECT_DOUBLE_EQ(results_gndSurfsAvgRefl, GndSurfsProperty.SurfsReflAvg);

    // test 2: surface viewing grass area only
    GndSurfsProperty.GndSurfs(1).ViewFactor = 0.4;
    GndSurfsProperty.GndSurfs(2).ViewFactor = 0.0;
    GndSurfsProperty.GndSurfs(3).ViewFactor = 0.0;
    // calculate ground surfaces average reflectance
    GetGroundSurfacesReflectanceAverage(*state);
    results_gndSurfsAvgRefl = 0.25;
    // check grass area ground surface reflectance
    EXPECT_DOUBLE_EQ(results_gndSurfsAvgRefl, GndSurfsProperty.SurfsReflAvg);

    // test 3: surface viewing parking area only
    GndSurfsProperty.GndSurfs(1).ViewFactor = 0.0;
    GndSurfsProperty.GndSurfs(2).ViewFactor = 0.4;
    GndSurfsProperty.GndSurfs(3).ViewFactor = 0.0;
    // calculate ground surfaces average reflectance
    GetGroundSurfacesReflectanceAverage(*state);
    results_gndSurfsAvgRefl = 0.50;
    // check parking area ground surface reflectance
    EXPECT_DOUBLE_EQ(results_gndSurfsAvgRefl, GndSurfsProperty.SurfsReflAvg);

    // test 4: surface viewing lake area only
    GndSurfsProperty.GndSurfs(1).ViewFactor = 0.0;
    GndSurfsProperty.GndSurfs(2).ViewFactor = 0.0;
    GndSurfsProperty.GndSurfs(3).ViewFactor = 0.4;
    // calculate ground surfaces average reflectance
    GetGroundSurfacesReflectanceAverage(*state);
    results_gndSurfsAvgRefl = 0.10;
    // check lake area ground surface reflectance
    EXPECT_DOUBLE_EQ(results_gndSurfsAvgRefl, GndSurfsProperty.SurfsReflAvg);
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_TestSurfPropertyViewFactorsReset)
{

    std::string const idf_objects = delimited_string({
        "  Building,",
        "    House with Local Air Nodes,  !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    0.001,                   !- Loads Convergence Tolerance Value",
        "    0.0050000,               !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullInteriorAndExterior, !- Solar Distribution",
        "    25,                      !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",

        "  Timestep,6;",

        "  SurfaceConvectionAlgorithm:Inside,TARP;",

        "  SurfaceConvectionAlgorithm:Outside,DOE-2;",

        "  HeatBalanceAlgorithm,ConductionTransferFunction;",

        "  SimulationControl,",
        "    No,                      !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    Yes,                     !- Run Simulation for Sizing Periods",
        "    Yes;                     !- Run Simulation for Weather File Run Periods",

        "  RunPeriod,",
        "    WinterDay,               !- Name",
        "    1,                       !- Begin Month",
        "    14,                      !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    1,                       !- End Month",
        "    14,                      !- End Day of Month",
        "    ,                        !- End Year",
        "    Tuesday,                 !- Day of Week for Start Day",
        "    Yes,                     !- Use Weather File Holidays and Special Days",
        "    Yes,                     !- Use Weather File Daylight Saving Period",
        "    No,                      !- Apply Weekend Holiday Rule",
        "    Yes,                     !- Use Weather File Rain Indicators",
        "    Yes;                     !- Use Weather File Snow Indicators",

        "  RunPeriod,",
        "    SummerDay,               !- Name",
        "    7,                       !- Begin Month",
        "    7,                       !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    7,                       !- End Month",
        "    7,                       !- End Day of Month",
        "    ,                        !- End Year",
        "    Tuesday,                 !- Day of Week for Start Day",
        "    No,                      !- Apply Weekend Holiday Rule",
        "    Yes,                     !- Use Weather File Rain Indicators",
        "    No;                      !- Use Weather File Snow Indicators",

        "  Site:Location,",
        "    CHICAGO_IL_USA TMY2-94846,  !- Name",
        "    41.78,                   !- Latitude {deg}",
        "    -87.75,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190.00;                  !- Elevation {m}",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Heating 99% Design Conditions DB,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    WinterDesignDay,         !- Day Type",
        "    -17.3,                   !- Maximum Dry-Bulb Temperature {C}",
        "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    -17.3,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    4.9,                     !- Wind Speed {m/s}",
        "    270,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    0.0;                     !- Sky Clearness",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Cooling 1% Design Conditions DB/MCWB,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    31.5,                    !- Maximum Dry-Bulb Temperature {C}",
        "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    23.0,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    5.3,                     !- Wind Speed {m/s}",
        "    230,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    1.0;                     !- Sky Clearness",

        "  Site:GroundTemperature:BuildingSurface,20.03,20.03,20.13,20.30,20.43,20.52,20.62,20.77,20.78,20.55,20.44,20.20;",

        "  Material,",
        "    A1 - 1 IN STUCCO,        !- Name",
        "    Smooth,                  !- Roughness",
        "    2.5389841E-02,           !- Thickness {m}",
        "    0.6918309,               !- Conductivity {W/m-K}",
        "    1858.142,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Material,",
        "    CB11,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    0.2032000,               !- Thickness {m}",
        "    1.048000,                !- Conductivity {W/m-K}",
        "    1105.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.2000000,               !- Solar Absorptance",
        "    0.2000000;               !- Visible Absorptance",

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
        "    IN05,                    !- Name",
        "    Rough,                   !- Roughness",
        "    0.2458000,               !- Thickness {m}",
        "    4.3000001E-02,           !- Conductivity {W/m-K}",
        "    10.00000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    PW03,                    !- Name",
        "    MediumSmooth,            !- Roughness",
        "    1.2700000E-02,           !- Thickness {m}",
        "    0.1150000,               !- Conductivity {W/m-K}",
        "    545.0000,                !- Density {kg/m3}",
        "    1213.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7800000,               !- Solar Absorptance",
        "    0.7800000;               !- Visible Absorptance",

        "  Material,",
        "    CC03,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1016000,               !- Thickness {m}",
        "    1.310000,                !- Conductivity {W/m-K}",
        "    2243.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

        "  Material,",
        "    HF-A3,                   !- Name",
        "    Smooth,                  !- Roughness",
        "    1.5000000E-03,           !- Thickness {m}",
        "    44.96960,                !- Conductivity {W/m-K}",
        "    7689.000,                !- Density {kg/m3}",
        "    418.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.2000000,               !- Solar Absorptance",
        "    0.2000000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    AR02,                    !- Name",
        "    VeryRough,               !- Roughness",
        "    7.8000002E-02,           !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7000000,               !- Solar Absorptance",
        "    0.7000000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    CP02,                    !- Name",
        "    Rough,                   !- Roughness",
        "    0.2170000,               !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Construction,",
        "    EXTWALL:LIVING,          !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    GP01;                    !- Layer 3",

        "  Construction,",
        "    FLOOR:LIVING,            !- Name",
        "    CC03,                    !- Outside Layer",
        "    CP02;                    !- Layer 2",

        "  Construction,",
        "    ROOF,                    !- Name",
        "    AR02,                    !- Outside Layer",
        "    PW03;                    !- Layer 2",

        "  Zone,",
        "    LIVING ZONE,             !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    World;                   !- Coordinate System",

        "  BuildingSurface:Detailed,",
        "    Living:North,            !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    1,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    1,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:East,             !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    1,0,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    1,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:South,            !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,0,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:West,             !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,0,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Floor,            !- Name",
        "    FLOOR,                   !- Surface Type",
        "    FLOOR:LIVING,            !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Living:Floor,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,0,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Ceiling,          !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,1,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,0,1,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  SurfaceProperty:LocalEnvironment,",
        "    LocEnv:Living:North,          !- Name",
        "    Living:North,                 !- Exterior Surface Name",
        "    ,                             !- External Shading Fraction Schedule Name",
        "    SrdSurfs:Living:North,        !- Surrounding Surfaces Object Name",
        "    ,                             !- Outdoor Air Node Name",
        "    GndSurfs:Living:North;        !- Ground Surfaces Object Name",

        "  SurfaceProperty:LocalEnvironment,",
        "    LocEnv:Living:East,           !- Name",
        "    Living:East,                  !- Exterior Surface Name",
        "    ,                             !- External Shading Fraction Schedule Name",
        "    SrdSurfs:Living:East,         !- Surrounding Surfaces Object Name",
        "    ,                             !- Outdoor Air Node Name",
        "    GndSurfs:Living:East;         !- Ground Surfaces Object Name",

        "  SurfaceProperty:LocalEnvironment,",
        "    LocEnv:Living:South,          !- Name",
        "    Living:South,                 !- Exterior Surface Name",
        "    ,                             !- External Shading Fraction Schedule Name",
        "    SrdSurfs:Living:South,        !- Surrounding Surfaces Object Name",
        "    ,                             !- Outdoor Air Node Name",
        "    GndSurfs:Living:South;        !- Ground Surfaces Object Name",

        "  SurfaceProperty:SurroundingSurfaces,",
        "    SrdSurfs:Living:North,        !- Name",
        "    ,                             !- Sky View Factor",
        "    Sky Temp Sch,                 !- Sky Temperature Schedule Name",
        "    ,                             !- Ground View Factor",
        "    ,                             !- Ground Temperature Schedule Name",
        "    SurroundingSurface1,          !- Surrounding Surface 1 Name",
        "    0.2,                          !- Surrounding Surface 1 View Factor",
        "    Surrounding Temp Sch 1;       !- Surrounding Surface 1 Temperature Schedule Name",

        "  SurfaceProperty:GroundSurfaces,",
        "    GndSurfs:Living:North,        !-Name",
        "    GndSurfs NorthGrassArea,      !-Ground Surface 1 Name",
        "    0.5,                          !-Ground Surface 1 View Factor",
        "    Ground Temp Sch,              !-Ground Surface 1 Temperature Schedule Name",
        "    ;                             !-Ground Surface 1 Reflectance Schedule Name",

        "  SurfaceProperty:SurroundingSurfaces,",
        "    SrdSurfs:Living:East,         !- Name",
        "    0.2,                          !- Sky View Factor",
        "    Sky Temp Sch,                 !- Sky Temperature Schedule Name",
        "    ,                             !- Ground View Factor",
        "    ,                             !- Ground Temperature Schedule Name",
        "    SurroundingSurface1,          !- Surrounding Surface 1 Name",
        "    0.3,                          !- Surrounding Surface 1 View Factor",
        "    Surrounding Temp Sch 1,       !- Surrounding Surface 1 Temperature Schedule Name",
        "    SurroundingSurface1,          !- Surrounding Surface 2 Name",
        "    0.3,                          !- Surrounding Surface 2 View Factor",
        "    Surrounding Temp Sch 1;       !- Surrounding Surface 2 Temperature Schedule Name",

        "  SurfaceProperty:GroundSurfaces,",
        "    GndSurfs:Living:East,         !-Name",
        "    GndSurfs EastGrassArea,       !-Ground Surface 1 Name",
        "    ,                             !-Ground Surface 1 View Factor",
        "    Ground Temp Sch,              !-Ground Surface 1 Temperature Schedule Name",
        "    ;                             !-Ground Surface 1 Reflectance Schedule Name",

        "  SurfaceProperty:SurroundingSurfaces,",
        "    SrdSurfs:Living:South,        !- Name",
        "    ,                             !- Sky View Factor",
        "    ,                             !- Sky Temperature Schedule Name",
        "    ,                             !- Ground View Factor",
        "    ,                             !- Ground Temperature Schedule Name",
        "    SurroundingSurface1,          !- Surrounding Surface 1 Name",
        "    0.4,                          !- Surrounding Surface 1 View Factor",
        "    Surrounding Temp Sch 1;       !- Surrounding Surface 1 Temperature Schedule Name",

        "  SurfaceProperty:GroundSurfaces,",
        "    GndSurfs:Living:South,        !-Name",
        "    GndSurfs SouthGrassArea,      !-Ground Surface 1 Name",
        "    ,                             !-Ground Surface 1 View Factor",
        "    Ground Temp Sch,              !-Ground Surface 1 Temperature Schedule Name",
        "    ;                             !-Ground Surface 1 Reflectance Schedule Name",

        "  ScheduleTypeLimits,",
        "    Any Number;                   !- Name",

        "  Schedule:Compact,",
        "    Surrounding Temp Sch 1,       !- Name",
        "    Any Number,                   !- Schedule Type Limits Name",
        "    Through: 12/31,               !- Field 1",
        "    For: AllDays,                 !- Field 2",
        "    Until: 24:00, 15.0;           !- Field 3",

        "  Schedule:Compact,",
        "    Sky Temp Sch,                 !- Name",
        "    Any Number,                   !- Schedule Type Limits Name",
        "    Through: 12/31,               !- Field 1",
        "    For: AllDays,                 !- Field 2",
        "    Until: 24:00, 15.0;           !- Field 3",

        "  Schedule:Compact,",
        "    Ground Temp Sch,              !- Name",
        "    Any Number,                   !- Schedule Type Limits Name",
        "    Through: 12/31,               !- Field 1",
        "    For: AllDays,                 !- Field 2",
        "    Until: 24:00, 15.0;           !- Field 3",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound = false;
    HeatBalanceManager::GetProjectControlData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    Material::GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    SurfaceGeometry::GetGeometryParameters(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    state->dataSurfaceGeometry->CosBldgRotAppGonly = 1.0;
    state->dataSurfaceGeometry->SinBldgRotAppGonly = 0.0;
    SurfaceGeometry::SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    HeatBalanceIntRadExchange::InitSolarViewFactors(*state);
    EXPECT_FALSE(has_err_output(true));
    EXPECT_TRUE(state->dataGlobal->AnyLocalEnvironmentsInModel);

    state->dataEnvrn->OutBaroPress = 101325.0;
    std::vector<int> controlledZoneEquipConfigNums;
    controlledZoneEquipConfigNums.push_back(1);
    state->dataHeatBal->Zone(1).IsControlled = true;
    state->dataHeatBal->Zone(1).SystemZoneNodeNumber = 5;

    state->dataZoneEquip->ZoneEquipConfig.allocate(1);
    auto &zoneEquipConfig = state->dataZoneEquip->ZoneEquipConfig(1);

    zoneEquipConfig.ZoneName = "LIVING ZONE";
    zoneEquipConfig.NumInletNodes = 2;
    zoneEquipConfig.InletNode.allocate(2);
    zoneEquipConfig.InletNode(1) = 1;
    zoneEquipConfig.InletNode(2) = 2;
    zoneEquipConfig.NumReturnNodes = 1;
    zoneEquipConfig.ReturnNode.allocate(1);
    zoneEquipConfig.ReturnNode(1) = 3;
    zoneEquipConfig.NumExhaustNodes = 1;
    zoneEquipConfig.ExhaustNode.allocate(1);
    zoneEquipConfig.ExhaustNode(1) = 4;
    zoneEquipConfig.FixedReturnFlow.allocate(1);

    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->spaceHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 24.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.001;
    state->dataZoneTempPredictorCorrector->spaceHeatBalance(1).MAT = 24.0;
    state->dataZoneTempPredictorCorrector->spaceHeatBalance(1).airHumRat = 0.001;

    state->dataLoopNodes->Node.allocate(4);
    auto &InletNode1 = state->dataLoopNodes->Node(1);
    auto &InletNode2 = state->dataLoopNodes->Node(2);
    auto &ExhaustNode = state->dataLoopNodes->Node(3);
    auto &ReturnNode = state->dataLoopNodes->Node(4);

    InletNode1.Temp = 20.0;
    InletNode2.Temp = 20.0;
    ReturnNode.Temp = 20.0;
    ExhaustNode.Temp = 20.0;
    InletNode1.MassFlowRate = 0.1;
    InletNode2.MassFlowRate = 0.1;
    ReturnNode.MassFlowRate = 0.1;
    ExhaustNode.MassFlowRate = 0.1;

    state->dataHeatBal->ZoneWinHeatGain.allocate(1);
    state->dataHeatBal->ZoneWinHeatGainRep.allocate(1);
    state->dataHeatBal->ZoneWinHeatGainRepEnergy.allocate(1);

    state->dataSurface->TotSurfaces = 6;
    // set convective coefficient adjustment ratio to 1.0
    state->dataHeatBalSurf->SurfWinCoeffAdjRatio.dimension(state->dataSurface->TotSurfaces, 1.0);
    state->dataMstBal->HConvInFD.allocate(state->dataSurface->TotSurfaces);
    state->dataMstBal->RhoVaporAirIn.allocate(state->dataSurface->TotSurfaces);
    state->dataMstBal->HMassConvInFD.allocate(state->dataSurface->TotSurfaces);
    state->dataHeatBalSurf->SurfHConvInt.allocate(state->dataSurface->TotSurfaces);
    state->dataHeatBalSurf->SurfHConvInt(1) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(2) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(3) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(4) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(5) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(6) = 0.5;
    state->dataHeatBalSurf->SurfTempInTmp.allocate(state->dataSurface->TotSurfaces);
    state->dataHeatBalSurf->SurfTempInTmp(1) = 15.0;
    state->dataHeatBalSurf->SurfTempInTmp(2) = 20.0;
    state->dataHeatBalSurf->SurfTempInTmp(3) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(4) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(5) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(6) = 25.0;

    state->dataGlobal->BeginSimFlag = true;
    state->dataGlobal->KickOffSimulation = true;
    state->dataGlobal->TimeStepZoneSec = 900;

    AllocateSurfaceHeatBalArrays(*state);
    createFacilityElectricPowerServiceObject(*state);
    HeatBalanceManager::AllocateZoneHeatBalArrays(*state);
    SolarShading::AllocateModuleArrays(*state);
    SolarShading::DetermineShadowingCombinations(*state);

    state->dataSurface->SurfTAirRef(1) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    state->dataSurface->SurfTAirRef(2) = DataSurfaces::RefAirTemp::AdjacentAirTemp;
    state->dataSurface->SurfTAirRef(3) = DataSurfaces::RefAirTemp::ZoneSupplyAirTemp;

    // check default surface view factors
    int SurfNum = Util::FindItemInList("LIVING:NORTH", state->dataSurface->Surface);
    auto &Surface_Living_North = state->dataSurface->Surface(SurfNum);
    SurfNum = Util::FindItemInList("LIVING:EAST", state->dataSurface->Surface);
    auto &Surface_Living_East = state->dataSurface->Surface(SurfNum);
    SurfNum = Util::FindItemInList("LIVING:SOUTH", state->dataSurface->Surface);
    auto &Surface_Living_South = state->dataSurface->Surface(SurfNum);
    // check exterior default surfaces sky and ground view factors
    EXPECT_DOUBLE_EQ(0.5, Surface_Living_North.ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.5, Surface_Living_North.ViewFactorGroundIR);
    EXPECT_DOUBLE_EQ(0.5, Surface_Living_East.ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.5, Surface_Living_East.ViewFactorGroundIR);
    EXPECT_DOUBLE_EQ(0.5, Surface_Living_South.ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.5, Surface_Living_South.ViewFactorGroundIR);

    // set local derived data vars
    SurfNum = Util::FindItemInList("LIVING:NORTH", state->dataSurface->Surface);
    int SrdSurfsNum = state->dataSurface->Surface(SurfNum).SurfSurroundingSurfacesNum;
    auto &SrdSurfsProperty_1 = state->dataSurface->SurroundingSurfsProperty(SrdSurfsNum);
    int GndSurfsNum = state->dataSurface->Surface(SurfNum).SurfPropertyGndSurfIndex;
    auto &GndSurfsProperty_1 = state->dataSurface->GroundSurfsProperty(GndSurfsNum);
    SurfNum = Util::FindItemInList("LIVING:EAST", state->dataSurface->Surface);
    SrdSurfsNum = state->dataSurface->Surface(SurfNum).SurfSurroundingSurfacesNum;
    auto &SrdSurfsProperty_2 = state->dataSurface->SurroundingSurfsProperty(SrdSurfsNum);
    GndSurfsNum = state->dataSurface->Surface(SurfNum).SurfPropertyGndSurfIndex;
    auto &GndSurfsProperty_2 = state->dataSurface->GroundSurfsProperty(GndSurfsNum);
    SurfNum = Util::FindItemInList("LIVING:SOUTH", state->dataSurface->Surface);
    SrdSurfsNum = state->dataSurface->Surface(SurfNum).SurfSurroundingSurfacesNum;
    auto &SrdSurfsProperty_3 = state->dataSurface->SurroundingSurfsProperty(SrdSurfsNum);
    GndSurfsNum = state->dataSurface->Surface(SurfNum).SurfPropertyGndSurfIndex;
    auto &GndSurfsProperty_3 = state->dataSurface->GroundSurfsProperty(GndSurfsNum);

    // check user input sky view factor is blank
    EXPECT_DOUBLE_EQ(0.0, SrdSurfsProperty_1.SkyViewFactor);
    EXPECT_FALSE(SrdSurfsProperty_1.IsSkyViewFactorSet);
    EXPECT_DOUBLE_EQ(0.2, SrdSurfsProperty_1.SurroundingSurfs(1).ViewFactor);
    // check user input ground view factor is not blank
    EXPECT_DOUBLE_EQ(0.5, GndSurfsProperty_1.GndSurfs(1).ViewFactor);
    EXPECT_TRUE(GndSurfsProperty_1.IsGroundViewFactorSet);
    // check user input sky view factor is NOT blank
    EXPECT_DOUBLE_EQ(0.2, SrdSurfsProperty_2.SkyViewFactor);
    EXPECT_TRUE(SrdSurfsProperty_2.IsSkyViewFactorSet);
    EXPECT_DOUBLE_EQ(0.3, SrdSurfsProperty_2.SurroundingSurfs(1).ViewFactor);
    EXPECT_DOUBLE_EQ(0.3, SrdSurfsProperty_2.SurroundingSurfs(2).ViewFactor);
    // check user input ground view factor is blank
    EXPECT_DOUBLE_EQ(0.0, GndSurfsProperty_2.GndSurfs(1).ViewFactor);
    EXPECT_FALSE(GndSurfsProperty_2.IsGroundViewFactorSet);
    // check user input sky and ground view factors are blank
    EXPECT_DOUBLE_EQ(0.0, SrdSurfsProperty_3.SkyViewFactor);
    EXPECT_FALSE(SrdSurfsProperty_3.IsSkyViewFactorSet);
    EXPECT_DOUBLE_EQ(0.4, SrdSurfsProperty_3.SurroundingSurfs(1).ViewFactor);
    EXPECT_DOUBLE_EQ(0.0, GndSurfsProperty_3.GndSurfs(1).ViewFactor);
    EXPECT_FALSE(GndSurfsProperty_3.IsGroundViewFactorSet);

    // reset sky and ground view factors
    InitSurfacePropertyViewFactors(*state);
    // test rest of user inputs blank view factors
    EXPECT_DOUBLE_EQ(0.3, SrdSurfsProperty_1.SkyViewFactor);
    EXPECT_DOUBLE_EQ(0.2, SrdSurfsProperty_1.SurroundingSurfs(1).ViewFactor);
    EXPECT_DOUBLE_EQ(0.5, GndSurfsProperty_1.GndSurfs(1).ViewFactor);
    EXPECT_TRUE(GndSurfsProperty_1.IsGroundViewFactorSet);
    EXPECT_DOUBLE_EQ(0.2, SrdSurfsProperty_2.SkyViewFactor);
    EXPECT_DOUBLE_EQ(0.3, SrdSurfsProperty_2.SurroundingSurfs(1).ViewFactor);
    EXPECT_DOUBLE_EQ(0.3, SrdSurfsProperty_2.SurroundingSurfs(2).ViewFactor);
    EXPECT_DOUBLE_EQ(0.2, GndSurfsProperty_2.GndSurfs(1).ViewFactor);
    EXPECT_TRUE(GndSurfsProperty_2.IsGroundViewFactorSet);
    EXPECT_DOUBLE_EQ(0.3, SrdSurfsProperty_3.SkyViewFactor);
    EXPECT_DOUBLE_EQ(0.4, SrdSurfsProperty_3.SurroundingSurfs(1).ViewFactor);
    EXPECT_DOUBLE_EQ(0.3, GndSurfsProperty_3.GndSurfs(1).ViewFactor);
    EXPECT_TRUE(GndSurfsProperty_3.IsGroundViewFactorSet);
    // test surface view factors reset are correct
    EXPECT_DOUBLE_EQ(0.3, Surface_Living_North.ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.5, Surface_Living_North.ViewFactorGroundIR);
    EXPECT_DOUBLE_EQ(0.2, Surface_Living_East.ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.2, Surface_Living_East.ViewFactorGroundIR);
    EXPECT_DOUBLE_EQ(0.3, Surface_Living_South.ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.3, Surface_Living_South.ViewFactorGroundIR);
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_TestUpdateVariableAbsorptances)
{
    std::string const idf_objects = delimited_string({
        "Table:IndependentVariable,",
        "THERMAL_ABSORPTANCE_TABLE_INDEPENDENTVARIABLE,  !- Name",
        "Linear,                  !- Interpolation Method",
        "Constant,                !- Extrapolation Method",
        "0.0,                     !- Minimum Value",
        "100.0,                   !- Maximum Value",
        ",                        !- Normalization Reference Value",
        "Temperature,             !- Unit Type",
        ",                        !- External File Name",
        ",                        !- External File Column Number",
        ",                        !- External File Starting Row Number",
        "0.0,                     !- Value 1",
        "20.0,                    !- Value 2",
        "20.1,                    !- Value 3",
        "50.0;                    !- Value 4",

        "Table:IndependentVariableList,",
        "THERMAL_ABSORPTANCE_TABLE_INDEPENDENTVARIABLELIST,  !- Name",
        "THERMAL_ABSORPTANCE_TABLE_INDEPENDENTVARIABLE;      !- Independent Variable 1 Name",

        "Table:Lookup,",
        "THERMAL_ABSORPTANCE_TABLE,  !- Name",
        "THERMAL_ABSORPTANCE_TABLE_INDEPENDENTVARIABLELIST,  !- Independent Variable List Name",
        ",                        !- Normalization Method",
        ",                        !- Normalization Divisor",
        "0.0,                     !- Minimum Output",
        ",                        !- Maximum Output",
        "Dimensionless,           !- Output Unit Type",
        ",                        !- External File Name",
        ",                        !- External File Column Number",
        ",                        !- External File Starting Row Number",
        "0.10,                    !- Output Value 1",
        "0.10,                    !- Output Value 2",
        "0.30,                    !- Output Value 3",
        "0.30;                    !- Output Value 4",

        "Curve:Linear,",
        "SOLAR_ABSORPTANCE_CURVE,    !- Name",
        "0.2,                     !- Coefficient1 Constant",
        "0.01,                   !- Coefficient2 x",
        "-40.0,                   !- Minimum Value of x",
        "60.0;                    !- Maximum Value of x",
        "ScheduleTypeLimits,",
        "  Fraction,                 !- Name",
        "  0,                        !- Lower Limit Value",
        "  1,                        !- Upper Limit Value",
        "  Continuous,               !- Numeric Type",
        "  Dimensionless;            !- Unit Type",

        "Schedule:Constant,",
        "    THERMAL_ABS_SCH,      !- Name",
        "    Fraction,                   !- Schedule Type Limits Name",
        "    0.9;                       !- Hourly Value",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->dataGlobal->TimeStepsInHour = 1;    // must initialize this to get schedules initialized
    state->dataGlobal->MinutesInTimeStep = 60; // must initialize this to get schedules initialized
    state->init_state(*state);

    EXPECT_EQ(state->dataCurveManager->curves(1)->Name, "SOLAR_ABSORPTANCE_CURVE");
    EXPECT_EQ(state->dataCurveManager->curves(2)->Name, "THERMAL_ABSORPTANCE_TABLE");
    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 21;
    state->dataGlobal->HourOfDay = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    Sched::UpdateScheduleVals(*state);
    state->dataSurface->Surface.allocate(3);
    state->dataSurface->Surface(1).Name = "SURF_1_WALL_1";
    state->dataSurface->Surface(1).Construction = 1;
    state->dataSurface->Surface(2).Name = "SURF_2_WALL_1";
    state->dataSurface->Surface(2).Construction = 1;
    state->dataSurface->Surface(3).Name = "SURF_1_WALL_2";
    state->dataSurface->Surface(3).Construction = 2;
    state->dataConstruction->Construct.allocate(2);
    state->dataConstruction->Construct(1).Name = "CONSTRUCT_WALL_1";
    state->dataConstruction->Construct(1).LayerPoint.allocate(1);
    state->dataConstruction->Construct(1).LayerPoint(1) = 1;
    state->dataConstruction->Construct(2).Name = "CONSTRUCT_WALL_2";
    state->dataConstruction->Construct(2).LayerPoint.allocate(1);
    state->dataConstruction->Construct(2).LayerPoint(1) = 2;

    auto &s_mat = state->dataMaterial;
    auto *mat1 = new Material::MaterialBase;
    mat1->Name = "WALL_1";
    mat1->group = Material::Group::Regular;
    mat1->absorpVarCtrlSignal = Material::VariableAbsCtrlSignal::SurfaceTemperature;
    mat1->absorpThermalVarCurve = Curve::GetCurve(*state, "THERMAL_ABSORPTANCE_TABLE");
    mat1->absorpSolarVarCurve = Curve::GetCurve(*state, "SOLAR_ABSORPTANCE_CURVE");
    s_mat->materials.push_back(mat1);

    auto *mat2 = new Material::MaterialBase;
    mat2->Name = "WALL_2";
    mat2->group = Material::Group::Regular;
    mat2->absorpVarCtrlSignal = Material::VariableAbsCtrlSignal::Scheduled;
    mat2->absorpThermalVarSched = Sched::GetSchedule(*state, "THERMAL_ABS_SCH");
    s_mat->materials.push_back(mat2);

    state->dataHeatBalSurf->SurfTempOut.allocate(2);
    state->dataHeatBalSurf->SurfTempOut(1) = 10;
    state->dataHeatBalSurf->SurfTempOut(2) = 30;
    state->dataSurface->AllVaryAbsOpaqSurfaceList = {1, 2, 3};
    state->dataHeatBalSurf->SurfAbsThermalExt.allocate(3);
    state->dataHeatBalSurf->SurfAbsThermalExt = 0.5;
    state->dataHeatBalSurf->SurfAbsSolarExt.allocate(3);
    state->dataHeatBalSurf->SurfAbsSolarExt = 0.8;
    UpdateVariableAbsorptances(*state);
    // controlled by a lookup table
    EXPECT_NEAR(state->dataHeatBalSurf->SurfAbsThermalExt(1), 0.1, 1e-6);
    EXPECT_NEAR(state->dataHeatBalSurf->SurfAbsThermalExt(2), 0.3, 1e-6);
    // controlled by a schedule
    EXPECT_NEAR(state->dataHeatBalSurf->SurfAbsThermalExt(3), 0.9, 1e-6);
    // controlled by a linear function
    //    0.2 + 10 * 0.01 = 0.3
    EXPECT_NEAR(state->dataHeatBalSurf->SurfAbsSolarExt(1), 0.3, 1e-6);
    //    0.2 + 30 * 0.01 = 0.5
    EXPECT_NEAR(state->dataHeatBalSurf->SurfAbsSolarExt(2), 0.5, 1e-6);
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_UpdateThermalHistoriesIZSurfaceCheck)
{
    state->dataSurface->TotSurfaces = 2;
    state->dataGlobal->NumOfZones = 2;
    state->dataHeatBal->TotConstructs = 1;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataSurface->Surface.allocate(state->dataSurface->TotSurfaces);
    state->dataSurface->SurfaceWindow.allocate(state->dataSurface->TotSurfaces);
    state->dataConstruction->Construct.allocate(state->dataHeatBal->TotConstructs);
    state->dataHeatBal->AnyInternalHeatSourceInInput = false;
    state->dataHeatBal->SimpleCTFOnly = false;

    AllocateSurfaceHeatBalArrays(*state); // allocates a host of variables related to CTF calculations

    state->dataSurface->Surface(1).Class = DataSurfaces::SurfaceClass::Wall;
    state->dataSurface->Surface(1).HeatTransSurf = true;
    state->dataSurface->Surface(1).HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::CTF;
    state->dataSurface->Surface(1).Construction = 1;
    state->dataSurface->Surface(2).Class = DataSurfaces::SurfaceClass::Wall;
    state->dataSurface->Surface(2).HeatTransSurf = true;
    state->dataSurface->Surface(2).HeatTransferAlgorithm = DataSurfaces::HeatTransferModel::CTF;
    state->dataSurface->Surface(2).Construction = 1;
    state->dataHeatBal->space.allocate(2);
    state->dataHeatBal->Zone(1).spaceIndexes.emplace_back(1);
    state->dataHeatBal->space(1).OpaqOrIntMassSurfaceFirst = 1;
    state->dataHeatBal->space(1).OpaqOrIntMassSurfaceLast = 1;
    state->dataHeatBal->space(1).HTSurfaceFirst = 1;
    state->dataHeatBal->space(1).HTSurfaceLast = 1;
    state->dataHeatBal->Zone(2).spaceIndexes.emplace_back(2);
    state->dataHeatBal->space(2).OpaqOrIntMassSurfaceFirst = 2;
    state->dataHeatBal->space(2).OpaqOrIntMassSurfaceLast = 2;
    state->dataHeatBal->space(2).HTSurfaceFirst = 2;
    state->dataHeatBal->space(2).HTSurfaceLast = 2;

    state->dataConstruction->Construct(1).NumCTFTerms = 2;
    state->dataConstruction->Construct(1).SourceSinkPresent = false;
    state->dataConstruction->Construct(1).NumHistories = 1;
    state->dataConstruction->Construct(1).CTFOutside[0] = 1.5;
    state->dataConstruction->Construct(1).CTFCross[0] = 1.5;
    state->dataConstruction->Construct(1).CTFInside[0] = 1.5;

    state->dataHeatBalSurf->SurfCurrNumHist(1) = 0;
    state->dataHeatBalSurf->SurfOutsideTempHist(1)(1) = 20.0;
    state->dataHeatBalSurf->SurfTempIn(1) = 10.0;
    state->dataHeatBalSurf->SurfCTFConstInPart(1) = 0.0;
    state->dataHeatBalSurf->SurfCurrNumHist(2) = 0;
    state->dataHeatBalSurf->SurfOutsideTempHist(1)(2) = 10.0;
    state->dataHeatBalSurf->SurfTempIn(2) = 20.0;
    state->dataHeatBalSurf->SurfCTFConstInPart(2) = 0.0;

    // Test 1: Partition--outside should have a non-zero value (interzone and regular partitions treated the same)
    state->dataSurface->Surface(1).ExtBoundCond = 1;
    state->dataSurface->Surface(2).ExtBoundCond = 2;

    UpdateThermalHistories(*state); // Test to make sure that the outside surface flux is being set properly for interzone surfaces

    EXPECT_EQ(15.0, state->dataHeatBalSurf->SurfOpaqInsFaceCondFlux(1));
    EXPECT_EQ(-15.0, state->dataHeatBalSurf->SurfOpaqOutFaceCondFlux(1));
    EXPECT_EQ(-15.0, state->dataHeatBalSurf->SurfOpaqInsFaceCondFlux(2));
    EXPECT_EQ(15.0, state->dataHeatBalSurf->SurfOpaqOutFaceCondFlux(2));

    // Test 2: Interzone Partition--outside should have a non-zero value
    state->dataSurface->Surface(1).ExtBoundCond = 2;
    state->dataSurface->Surface(2).ExtBoundCond = 1;
    state->dataHeatBalSurf->SurfOpaqInsFaceCondFlux = 0.0;
    state->dataHeatBalSurf->SurfOpaqOutFaceCondFlux = 0.0;

    UpdateThermalHistories(*state); // Test to make sure that the outside surface flux is being set properly for interzone surfaces

    EXPECT_EQ(15.0, state->dataHeatBalSurf->SurfOpaqInsFaceCondFlux(1));
    EXPECT_EQ(-15.0, state->dataHeatBalSurf->SurfOpaqOutFaceCondFlux(1));
    EXPECT_EQ(-15.0, state->dataHeatBalSurf->SurfOpaqInsFaceCondFlux(2));
    EXPECT_EQ(15.0, state->dataHeatBalSurf->SurfOpaqOutFaceCondFlux(2));
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_TestSurfQdotRadSolarInRepPerAreaCalc)
{
    Real64 const diffTol = 0.0000000001;
    Real64 expectedResult;

    state->dataHeatBal->Zone.allocate(1);
    state->dataHeatBal->space.allocate(1);
    state->dataSurface->Surface.allocate(1);
    state->dataHeatBal->ZoneWinHeatGain.dimension(1, 0.0);
    state->dataSurface->SurfWinHeatGain.dimension(1, 0.0);
    state->dataHeatBal->ZoneWinHeatGainRep.dimension(1, 0.0);
    state->dataHeatBal->ZoneWinHeatGainRepEnergy.dimension(1, 0.0);
    state->dataHeatBal->ZoneWinHeatLossRep.dimension(1, 0.0);
    state->dataHeatBal->ZoneWinHeatLossRepEnergy.dimension(1, 0.0);
    state->dataHeatBalSurf->SurfQdotRadSolarInRepPerArea.dimension(1, 0.0);
    state->dataHeatBalSurf->SurfOpaqQRadSWInAbs.dimension(1, 0.0);
    state->dataHeatBalSurf->SurfQdotRadLightsInPerArea.dimension(1, 0.0);

    auto &thisZone = state->dataHeatBal->Zone(1);
    auto &thisSpace = state->dataHeatBal->space(1);
    auto &thisSurface = state->dataSurface->Surface(1);
    auto &thisRep = state->dataHeatBalSurf->SurfQdotRadSolarInRepPerArea(1);
    auto &thisQRadSW = state->dataHeatBalSurf->SurfOpaqQRadSWInAbs(1);
    auto &thisLights = state->dataHeatBalSurf->SurfQdotRadLightsInPerArea(1);

    state->dataGlobal->NumOfZones = 1;
    thisZone.spaceIndexes.allocate(1);
    thisZone.spaceIndexes(1) = 1;
    thisSpace.WindowSurfaceFirst = 1;
    thisSpace.WindowSurfaceLast = 0;
    thisSurface.ExtSolar = false;
    state->dataSurface->UseRepresentativeSurfaceCalculations = false;
    thisSpace.OpaqOrWinSurfaceFirst = 1;
    thisSpace.OpaqOrWinSurfaceLast = 0;
    thisSpace.OpaqOrIntMassSurfaceFirst = 1;
    thisSpace.OpaqOrIntMassSurfaceLast = 1;

    // Test 1: all zero values--returns a zero (all are already zero--so just call and check)
    thisRep = -9999.9;
    expectedResult = 0.0;
    UpdateIntermediateSurfaceHeatBalanceResults(*state);
    EXPECT_NEAR(thisRep, expectedResult, diffTol);

    // Test 2: positive values that shouldn't return zero
    thisRep = -9999.9;
    expectedResult = 3.0;
    thisQRadSW = 6.0;
    thisLights = 3.0;
    UpdateIntermediateSurfaceHeatBalanceResults(*state);
    EXPECT_NEAR(thisRep, expectedResult, diffTol);

    // Test 3: positive values that would calculate negative--will return a negative number
    thisRep = -9999.9;
    expectedResult = -0.1;
    thisQRadSW = 6.0;
    thisLights = 6.1;
    UpdateIntermediateSurfaceHeatBalanceResults(*state);
    EXPECT_NEAR(thisRep, expectedResult, diffTol);

    // Test 4: positive values that would calculate a small number--return that number
    thisRep = -9999.9;
    expectedResult = 0.00001;
    thisQRadSW = 6.0;
    thisLights = 5.99999;
    UpdateIntermediateSurfaceHeatBalanceResults(*state);
    EXPECT_NEAR(thisRep, expectedResult, diffTol);
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_SurroundingSurfacesTempTest)
{
    std::string_view constexpr idf_objects = R"IDF(
      Zone,
        Zone,                         !- Name
        0,                            !- Direction of Relative North {deg}
        6.000000,                     !- X Origin {m}
        6.000000,                     !- Y Origin {m}
        0,                            !- Z Origin {m}
        1,                            !- Type
        1,                            !- Multiplier
        autocalculate,                !- Ceiling Height {m}
        autocalculate;                !- Volume {m3}

	  Material,
        Concrete Block,               !- Name
        MediumRough,                  !- Roughness
        0.1014984,                    !- Thickness {m}
        0.3805070,                    !- Conductivity {W/m-K}
        608.7016,                     !- Density {kg/m3}
        836.8000;                     !- Specific Heat {J/kg-K}

      Construction,
        WallConstruction,             !- Name
        Concrete Block;               !- Outside Layer

      WindowMaterial:SimpleGlazingSystem,
        WindowMaterial,               !- Name
        5.778,                        !- U-Factor {W/m2-K}
        0.819,                        !- Solar Heat Gain Coefficient
        0.881;                        !- Visible Transmittance

      Construction,
        WindowConstruction,           !- Name
        WindowMaterial;               !- Outside Layer

      WindowProperty:FrameAndDivider,
        WindowFrame,                  !- Name
        0.05,                         !- Frame Width {m}
        0.00,                         !- Frame Outside Projection {m}
        0.00,                         !- Frame Inside Projection {m}
        5.0,                          !- Frame Conductance {W/m2-K}
        1.2,                          !- Ratio of Frame-Edge Glass Conductance to Center-Of-Glass Conductance
        0.8,                          !- Frame Solar Absorptance
        0.8,                          !- Frame Visible Absorptance
        0.9,                          !- Frame Thermal Hemispherical Emissivity
        DividedLite,                  !- Divider Type
        0.02,                         !- Divider Width {m}
        2,                            !- Number of Horizontal Dividers
        2,                            !- Number of Vertical Dividers
        0.00,                         !- Divider Outside Projection {m}
        0.00,                         !- Divider Inside Projection {m}
        5.0,                          !- Divider Conductance {W/m2-K}
        1.2,                          !- Ratio of Divider-Edge Glass Conductance to Center-Of-Glass Conductance
        0.8,                          !- Divider Solar Absorptance
        0.8,                          !- Divider Visible Absorptance
        0.9;                          !- Divider Thermal Hemispherical Emissivity

      FenestrationSurface:Detailed,
        FenestrationSurface,          !- Name
        Window,                       !- Surface Type
        WindowConstruction,           !- Construction Name
        Wall,                         !- Building Surface Name
        ,                             !- Outside Boundary Condition Object
        0.5000000,                    !- View Factor to Ground
        WindowFrame,                  !- Frame and Divider Name
        1.0,                          !- Multiplier
        4,                            !- Number of Vertices
        0.200000,0.0,9.900000,        !- X,Y,Z ==> Vertex 1 {m}
        0.200000,0.0,0.1000000,       !- X,Y,Z ==> Vertex 2 {m}
        9.900000,0.0,0.1000000,       !- X,Y,Z ==> Vertex 3 {m}
        9.900000,0.0,9.900000;        !- X,Y,Z ==> Vertex 4 {m}

      SurfaceProperty:LocalEnvironment,
        LocEnv:FenestrationSurface,   !- Name
        FenestrationSurface,          !- Exterior Surface Name
        ,                             !- External Shading Fraction Schedule Name
        SrdSurfs:FenesSurface;        !- Surrounding Surfaces Object Name

      SurfaceProperty:SurroundingSurfaces,
        SrdSurfs:FenesSurface,        !- Name
        0.4,                          !- Sky View Factor
        Sky Temp Sch,                 !- Sky Temperature Schedule Name
        ,                             !- Ground View Factor
        ,                             !- Ground Temperature Schedule Name
        SrdSurfs:Surface 1,           !- Surrounding Surface 1 Name
        0.2,                          !- Surrounding Surface 1 View Factor
        Surrounding Temp Sch 1,       !- Surrounding Surface 1 Temperature Schedule Name
        SrdSurfs:Surface 2,           !- Surrounding Surface 2 Name
        0.2,                          !- Surrounding Surface 2 View Factor
        Surrounding Temp Sch 2,       !- Surrounding Surface 2 Temperature Schedule Name
        SrdSurfs:Surface 3,           !- Surrounding Surface 3 Name
        0.1,                          !- Surrounding Surface 3 View Factor
        Surrounding Temp Sch 3;       !- Surrounding Surface 3 Temperature Schedule Name

      Schedule:Compact,
        Surrounding Temp Sch 1,       !- Name
        Any Number,                   !- Schedule Type Limits Name
        Through: 12/31,               !- Field 1
        For: AllDays,                 !- Field 2
        Until: 24:00, 10.0;           !- Field 3

      Schedule:Compact,
        Surrounding Temp Sch 2,       !- Name
        Any Number,                   !- Schedule Type Limits Name
        Through: 12/31,               !- Field 1
        For: AllDays,                 !- Field 2
        Until: 24:00, 12.0;           !- Field 3

      Schedule:Compact,
        Surrounding Temp Sch 3,       !- Name
        Any Number,                   !- Schedule Type Limits Name
        Through: 12/31,               !- Field 1
        For: AllDays,                 !- Field 2
        Until: 24:00, 15.0;           !- Field 3

      ScheduleTypeLimits,
        Any Number;                   !- Name

      BuildingSurface:Detailed,
        Wall,                         !- Name
        Wall,                         !- Surface Type
        WallConstruction,             !- Construction Name
        Zone,                         !- Zone Name
        ,                             !- Space Name
        Outdoors,                     !- Outside Boundary Condition
        ,                             !- Outside Boundary Condition Object
        SunExposed,                   !- Sun Exposure
        WindExposed,                  !- Wind Exposure
        0.1000000,                    !- View Factor to Ground
        4,                            !- Number of Vertices
        0.0,0.000000,10.00000,        !- X,Y,Z ==> Vertex 1 {m}
        0.0,0.000000,0.0,             !- X,Y,Z ==> Vertex 2 {m}
        10.00000,0.0,0.0,             !- X,Y,Z ==> Vertex 3 {m}
        10.00000,0.0,10.00000;        !- X,Y,Z ==> Vertex 4 {m}

      BuildingSurface:Detailed,
        Floor,                        !- Name
        Floor,                        !- Surface Type
        WallConstruction,             !- Construction Name
        Zone,                         !- Zone Name
        ,                             !- Space Name
        Outdoors,                     !- Outside Boundary Condition
        ,                             !- Outside Boundary Condition Object
        NoSun,                        !- Sun Exposure
        NoWind,                       !- Wind Exposure
        1.0,                          !- View Factor to Ground
        4,                            !- Number of Vertices
        0.000000,0.000000,0,          !- X,Y,Z ==> Vertex 1 {m}
        0.000000,10.000000,0,         !- X,Y,Z ==> Vertex 2 {m}
        10.00000,10.000000,0,         !- X,Y,Z ==> Vertex 3 {m}
        10.00000,0.000000,0;          !- X,Y,Z ==> Vertex 4 {m}

    )IDF";

    bool ErrorsFound = false;
    ASSERT_TRUE(process_idf(idf_objects));
    state->dataGlobal->TimeStepZone = 1;
    state->dataGlobal->TimeStepZoneSec = 3600.0;
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    // set global and environmental variables
    state->dataGlobal->BeginSimFlag = true;
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataGlobal->HourOfDay = 15;
    state->dataGlobal->TimeStep = 1;
    state->dataEnvrn->Month = 7;
    state->dataEnvrn->DayOfMonth = 21;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    state->dataEnvrn->OutBaroPress = 101325;

    state->dataHeatBal->ZoneIntGain.allocate(1);
    createFacilityElectricPowerServiceObject(*state);
    HeatBalanceManager::SetPreConstructionInputParameters(*state);
    HeatBalanceManager::GetProjectControlData(*state, ErrorsFound);
    HeatBalanceManager::GetFrameAndDividerData(*state);
    Material::GetMaterialData(*state, ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound);
    HeatBalanceManager::GetBuildingData(*state, ErrorsFound);
    HeatBalanceManager::AllocateHeatBalArrays(*state);
    HeatBalanceSurfaceManager::AllocateSurfaceHeatBalArrays(*state);

    EXPECT_FALSE(ErrorsFound);
    EXPECT_TRUE(state->dataGlobal->AnyLocalEnvironmentsInModel);
    // test surface property object inputs
    int srdSurfsPropNum = Util::FindItemInList("SRDSURFS:FENESSURFACE", state->dataSurface->SurroundingSurfsProperty);
    EXPECT_EQ(1, state->dataSurface->SurfLocalEnvironment(srdSurfsPropNum).SurroundingSurfsPtr);
    // set local derived data vars
    int surfNum = Util::FindItemInList("FENESTRATIONSURFACE", state->dataSurface->Surface);
    auto &surface = state->dataSurface->Surface(surfNum);
    int srdSurfsNum = state->dataSurface->Surface(surfNum).SurfSurroundingSurfacesNum;
    auto &srdSurfsProperty = state->dataSurface->SurroundingSurfsProperty(srdSurfsNum);
    // update schedule values for surrounding surfaces temperature
    Sched::UpdateScheduleVals(*state);
    GetSurroundingSurfacesTemperatureAverage(*state);
    // calculate surrounding surfaces average temperature
    Real64 SrdSurfaceTemp = 0.0;
    Real64 SrdSurfaceTempSum = 0.0;
    for (auto &surdSurfs : srdSurfsProperty.SurroundingSurfs) {
        SrdSurfaceTemp = surdSurfs.tempSched->getCurrentVal() + Constant::Kelvin;
        SrdSurfaceTempSum += surdSurfs.ViewFactor * pow_4(SrdSurfaceTemp);
    }
    Real64 srdSurfacesTemp_result = root_4(SrdSurfaceTempSum / surface.ViewFactorSrdSurfs) - Constant::Kelvin;
    // check average temperature of surrounding surfaces
    EXPECT_DOUBLE_EQ(srdSurfacesTemp_result, surface.SrdSurfTemp);
}

TEST_F(EnergyPlusFixture, CFDHeatBalanceSurfaceManager_TestSurfPropSrdSurfLWR)
{

    std::string const idf_objects = delimited_string({
        "  Building,",
        "    House with Local Air Nodes,  !- Name",
        "    0,                       !- North Axis {deg}",
        "    Suburbs,                 !- Terrain",
        "    0.001,                   !- Loads Convergence Tolerance Value",
        "    0.0050000,               !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullInteriorAndExterior, !- Solar Distribution",
        "    25,                      !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",

        "  Timestep,6;",

        "  SurfaceConvectionAlgorithm:Inside,TARP;",

        "  SurfaceConvectionAlgorithm:Outside,DOE-2;",

        "  HeatBalanceAlgorithm,ConductionFiniteDifference;",

        "  SimulationControl,",
        "    No,                      !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    Yes,                     !- Run Simulation for Sizing Periods",
        "    Yes;                     !- Run Simulation for Weather File Run Periods",

        "  RunPeriod,",
        "    WinterDay,               !- Name",
        "    1,                       !- Begin Month",
        "    14,                      !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    1,                       !- End Month",
        "    14,                      !- End Day of Month",
        "    ,                        !- End Year",
        "    Tuesday,                 !- Day of Week for Start Day",
        "    Yes,                     !- Use Weather File Holidays and Special Days",
        "    Yes,                     !- Use Weather File Daylight Saving Period",
        "    No,                      !- Apply Weekend Holiday Rule",
        "    Yes,                     !- Use Weather File Rain Indicators",
        "    Yes;                     !- Use Weather File Snow Indicators",

        "  RunPeriod,",
        "    SummerDay,               !- Name",
        "    7,                       !- Begin Month",
        "    7,                       !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    7,                       !- End Month",
        "    7,                       !- End Day of Month",
        "    ,                        !- End Year",
        "    Tuesday,                 !- Day of Week for Start Day",
        "    No,                      !- Apply Weekend Holiday Rule",
        "    Yes,                     !- Use Weather File Rain Indicators",
        "    No;                      !- Use Weather File Snow Indicators",

        "  Site:Location,",
        "    CHICAGO_IL_USA TMY2-94846,  !- Name",
        "    41.78,                   !- Latitude {deg}",
        "    -87.75,                  !- Longitude {deg}",
        "    -6.00,                   !- Time Zone {hr}",
        "    190.00;                  !- Elevation {m}",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Heating 99% Design Conditions DB,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    WinterDesignDay,         !- Day Type",
        "    -17.3,                   !- Maximum Dry-Bulb Temperature {C}",
        "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    -17.3,                   !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    4.9,                     !- Wind Speed {m/s}",
        "    270,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    0.0;                     !- Sky Clearness",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Annual Cooling 1% Design Conditions DB/MCWB,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    31.5,                    !- Maximum Dry-Bulb Temperature {C}",
        "    10.7,                    !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    23.0,                    !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.,                  !- Barometric Pressure {Pa}",
        "    5.3,                     !- Wind Speed {m/s}",
        "    230,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    1.0;                     !- Sky Clearness",

        "  Site:GroundTemperature:BuildingSurface,20.03,20.03,20.13,20.30,20.43,20.52,20.62,20.77,20.78,20.55,20.44,20.20;",

        "  Material,",
        "    A1 - 1 IN STUCCO,        !- Name",
        "    Smooth,                  !- Roughness",
        "    2.5389841E-02,           !- Thickness {m}",
        "    0.6918309,               !- Conductivity {W/m-K}",
        "    1858.142,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.9200000,               !- Solar Absorptance",
        "    0.9200000;               !- Visible Absorptance",

        "  Material,",
        "    CB11,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    0.2032000,               !- Thickness {m}",
        "    1.048000,                !- Conductivity {W/m-K}",
        "    1105.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.2000000,               !- Solar Absorptance",
        "    0.2000000;               !- Visible Absorptance",

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
        "    IN05,                    !- Name",
        "    Rough,                   !- Roughness",
        "    0.2458000,               !- Thickness {m}",
        "    4.3000001E-02,           !- Conductivity {W/m-K}",
        "    10.00000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Material,",
        "    PW03,                    !- Name",
        "    MediumSmooth,            !- Roughness",
        "    1.2700000E-02,           !- Thickness {m}",
        "    0.1150000,               !- Conductivity {W/m-K}",
        "    545.0000,                !- Density {kg/m3}",
        "    1213.000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7800000,               !- Solar Absorptance",
        "    0.7800000;               !- Visible Absorptance",

        "  Material,",
        "    CC03,                    !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1016000,               !- Thickness {m}",
        "    1.310000,                !- Conductivity {W/m-K}",
        "    2243.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    0.6500000;               !- Visible Absorptance",

        "  Material,",
        "    HF-A3,                   !- Name",
        "    Smooth,                  !- Roughness",
        "    1.5000000E-03,           !- Thickness {m}",
        "    44.96960,                !- Conductivity {W/m-K}",
        "    7689.000,                !- Density {kg/m3}",
        "    418.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.2000000,               !- Solar Absorptance",
        "    0.2000000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    AR02,                    !- Name",
        "    VeryRough,               !- Roughness",
        "    7.8000002E-02,           !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7000000,               !- Solar Absorptance",
        "    0.7000000;               !- Visible Absorptance",

        "  Material:NoMass,",
        "    CP02,                    !- Name",
        "    Rough,                   !- Roughness",
        "    0.2170000,               !- Thermal Resistance {m2-K/W}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.7500000,               !- Solar Absorptance",
        "    0.7500000;               !- Visible Absorptance",

        "  Construction,",
        "    EXTWALL:LIVING,          !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    GP01;                    !- Layer 3",

        "  Construction,",
        "    FLOOR:LIVING,            !- Name",
        "    CC03,                    !- Outside Layer",
        "    CP02;                    !- Layer 2",

        "  Construction,",
        "    ROOF,                    !- Name",
        "    AR02,                    !- Outside Layer",
        "    PW03;                    !- Layer 2",

        "  Zone,",
        "    LIVING ZONE,             !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    CounterClockWise,        !- Vertex Entry Direction",
        "    World;                   !- Coordinate System",

        "  BuildingSurface:Detailed,",
        "    Living:North,            !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    1,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    1,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:East,             !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    1,0,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    1,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:South,            !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,0,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:West,             !- Name",
        "    Wall,                    !- Surface Type",
        "    EXTWALL:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,0,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Floor,            !- Name",
        "    FLOOR,                   !- Surface Type",
        "    FLOOR:LIVING,            !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Living:Floor,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,1,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,1,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,0,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Ceiling,          !- Name",
        "    ROOF,                 !- Surface Type",
        "    ROOF,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,1,1,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,1,  !- X,Y,Z ==> Vertex 2 {m}",
        "    1,0,1,  !- X,Y,Z ==> Vertex 3 {m}",
        "    1,1,1;  !- X,Y,Z ==> Vertex 4 {m}",

        "  SurfaceProperty:LocalEnvironment,",
        "    LocEnv:Living:North,          !- Name",
        "    Living:North,                 !- Exterior Surface Name",
        "    ,                             !- External Shading Fraction Schedule Name",
        "    SrdSurfs:Living:North,        !- Surrounding Surfaces Object Name",
        "    ;                             !- Outdoor Air Node Name",

        "  SurfaceProperty:LocalEnvironment,",
        "    LocEnv:Living:East,           !- Name",
        "    Living:East,                  !- Exterior Surface Name",
        "    ,                             !- External Shading Fraction Schedule Name",
        "    SrdSurfs:Living:East,         !- Surrounding Surfaces Object Name",
        "    ;                             !- Outdoor Air Node Name",

        "  SurfaceProperty:LocalEnvironment,",
        "    LocEnv:Living:South,          !- Name",
        "    Living:South,                 !- Exterior Surface Name",
        "    ,                             !- External Shading Fraction Schedule Name",
        "    SrdSurfs:Living:South,        !- Surrounding Surfaces Object Name",
        "    ;                             !- Outdoor Air Node Name",

        "  SurfaceProperty:SurroundingSurfaces,",
        "    SrdSurfs:Living:North, !- Name",
        "    0.3,",
        "    Sky Temp Sch,",
        "    0.1,",
        "    Ground Temp Sch,",
        "    SurroundingSurface1,",
        "    0.6,",
        "    Surrounding Temp Sch 1;",

        "  SurfaceProperty:SurroundingSurfaces,",
        "    SrdSurfs:Living:East, !- Name",
        "    0.2,",
        "    ,",
        "    0.2,",
        "    ,",
        "    SurroundingSurface1,",
        "    0.3,",
        "    Surrounding Temp Sch 1,",
        "    SurroundingSurface2,",
        "    0.3,",
        "    Surrounding Temp Sch 1;",

        "  SurfaceProperty:SurroundingSurfaces,",
        "    SrdSurfs:Living:South,     !- Name",
        "    ,",
        "    ,",
        "    0.25,",
        "    ,",
        "    SurroundingSurface1,",
        "    0.5,",
        "    Surrounding Temp Sch 1;",

        "  ScheduleTypeLimits,",
        "    Any Number;                   !- Name",

        "  Schedule:Compact,",
        "    Surrounding Temp Sch 1,       !- Name",
        "    Any Number,                   !- Schedule Type Limits Name",
        "    Through: 12/31,               !- Field 1",
        "    For: AllDays,                 !- Field 2",
        "    Until: 24:00, 15.0;           !- Field 3",

        "  Schedule:Compact,",
        "    Sky Temp Sch,                 !- Name",
        "    Any Number,                   !- Schedule Type Limits Name",
        "    Through: 12/31,               !- Field 1",
        "    For: AllDays,                 !- Field 2",
        "    Until: 24:00, 15.0;           !- Field 3",

        "  Schedule:Compact,",
        "    Ground Temp Sch,              !- Name",
        "    Any Number,                   !- Schedule Type Limits Name",
        "    Through: 12/31,               !- Field 1",
        "    For: AllDays,                 !- Field 2",
        "    Until: 24:00, 15.0;           !- Field 3",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    bool ErrorsFound = false;
    HeatBalanceManager::GetProjectControlData(*state, ErrorsFound); // read project control data
    EXPECT_FALSE(ErrorsFound);                                      // expect no errors
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    Material::GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    SurfaceGeometry::GetGeometryParameters(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    state->dataSurfaceGeometry->CosBldgRotAppGonly = 1.0;
    state->dataSurfaceGeometry->SinBldgRotAppGonly = 0.0;
    SurfaceGeometry::SetupZoneGeometry(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    HeatBalanceIntRadExchange::InitSolarViewFactors(*state);
    EXPECT_TRUE(state->dataGlobal->AnyLocalEnvironmentsInModel);

    state->dataZoneEquip->ZoneEquipConfig.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneName = "LIVING ZONE";
    std::vector<int> controlledZoneEquipConfigNums;
    controlledZoneEquipConfigNums.push_back(1);
    state->dataHeatBal->Zone(1).IsControlled = true;

    state->dataZoneEquip->ZoneEquipConfig(1).NumInletNodes = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode.allocate(2);
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(1) = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(2) = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1) = 3;
    state->dataZoneEquip->ZoneEquipConfig(1).NumReturnNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode(1) = 4;
    state->dataZoneEquip->ZoneEquipConfig(1).FixedReturnFlow.allocate(1);

    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataHeatBal->Zone(1).SystemZoneNodeNumber = 5;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->spaceHeatBalance.allocate(1);
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).MAT = 24.0;
    state->dataZoneTempPredictorCorrector->zoneHeatBalance(1).airHumRat = 0.001;
    state->dataZoneTempPredictorCorrector->spaceHeatBalance(1).MAT = 24.0;
    state->dataZoneTempPredictorCorrector->spaceHeatBalance(1).airHumRat = 0.001;

    state->dataLoopNodes->Node.allocate(4);

    state->dataHeatBalSurf->SurfTempInTmp.allocate(6);
    state->dataHeatBalSurf->SurfTempInTmp(1) = 15.0;
    state->dataHeatBalSurf->SurfTempInTmp(2) = 20.0;
    state->dataHeatBalSurf->SurfTempInTmp(3) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(4) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(5) = 25.0;
    state->dataHeatBalSurf->SurfTempInTmp(6) = 25.0;

    state->dataLoopNodes->Node(1).Temp = 20.0;
    state->dataLoopNodes->Node(2).Temp = 20.0;
    state->dataLoopNodes->Node(3).Temp = 20.0;
    state->dataLoopNodes->Node(4).Temp = 20.0;
    state->dataLoopNodes->Node(1).MassFlowRate = 0.1;
    state->dataLoopNodes->Node(2).MassFlowRate = 0.1;
    state->dataLoopNodes->Node(3).MassFlowRate = 0.1;
    state->dataLoopNodes->Node(4).MassFlowRate = 0.1;

    state->dataHeatBalSurf->SurfHConvInt.allocate(6);
    state->dataHeatBalSurf->SurfHConvInt(1) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(2) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(3) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(4) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(5) = 0.5;
    state->dataHeatBalSurf->SurfHConvInt(6) = 0.5;
    state->dataMstBal->HConvInFD.allocate(6);
    state->dataMstBal->RhoVaporAirIn.allocate(6);
    state->dataMstBal->HMassConvInFD.allocate(6);

    state->dataGlobal->BeginSimFlag = true;
    state->dataGlobal->KickOffSimulation = true;
    state->dataGlobal->TimeStepZoneSec = 900;
    state->dataHeatBal->ZoneWinHeatGain.allocate(1);
    state->dataHeatBal->ZoneWinHeatGainRep.allocate(1);
    state->dataHeatBal->ZoneWinHeatGainRepEnergy.allocate(1);

    // Set up
    state->dataHeatBalSurf->SurfWinCoeffAdjRatio.dimension(6, 1.0);

    AllocateSurfaceHeatBalArrays(*state);
    createFacilityElectricPowerServiceObject(*state);
    HeatBalanceManager::AllocateZoneHeatBalArrays(*state);
    SolarShading::AllocateModuleArrays(*state);
    SolarShading::DetermineShadowingCombinations(*state);

    state->dataSurface->SurfTAirRef(1) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    state->dataSurface->SurfTAirRef(2) = DataSurfaces::RefAirTemp::AdjacentAirTemp;
    state->dataSurface->SurfTAirRef(3) = DataSurfaces::RefAirTemp::ZoneSupplyAirTemp;

    InitSurfacePropertyViewFactors(*state);

    InitSurfaceHeatBalance(*state);

    state->dataSurface->SurfAirSkyRadSplit.allocate(6);
    Sched::GetSchedule(*state, "SURROUNDING TEMP SCH 1")->currentVal = 25.0; // Srd Srfs Temp
    Sched::GetSchedule(*state, "SKY TEMP SCH")->currentVal = 15.0;           // Sky temp
    Sched::GetSchedule(*state, "GROUND TEMP SCH")->currentVal = 22.0;        // Grd temp

    for (int SurfNum = 1; SurfNum <= 6; SurfNum++) {
        state->dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum) = 20; // Surf temp
        state->dataSurface->SurfOutDryBulbTemp(SurfNum) = 22;         // Air temp
        state->dataSurface->surfExtConv(SurfNum).model = Convect::HcExt::MoWiTTHcOutside;
        state->dataSurface->SurfAirSkyRadSplit(SurfNum) = 1.0;
    }
    CalcHeatBalanceOutsideSurf(*state);

    // Test if local value correctly overwritten
    // Surface(1-3) - local; Surface(4-6) - global;
    EXPECT_DOUBLE_EQ(0.3, state->dataSurface->Surface(1).ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.1, state->dataSurface->Surface(1).ViewFactorGroundIR);
    EXPECT_DOUBLE_EQ(0.6, state->dataSurface->Surface(1).ViewFactorSrdSurfs);
    EXPECT_DOUBLE_EQ(0.2, state->dataSurface->Surface(2).ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.2, state->dataSurface->Surface(2).ViewFactorGroundIR);
    EXPECT_DOUBLE_EQ(0.6, state->dataSurface->Surface(2).ViewFactorSrdSurfs);
    EXPECT_DOUBLE_EQ(0.25, state->dataSurface->Surface(3).ViewFactorSkyIR);
    EXPECT_DOUBLE_EQ(0.25, state->dataSurface->Surface(3).ViewFactorGroundIR);
    EXPECT_DOUBLE_EQ(0.50, state->dataSurface->Surface(3).ViewFactorSrdSurfs);
    // Check the heat balance algorithm specified is Conduction Finite Difference method
    EXPECT_EQ(state->dataHeatBal->OverallHeatTransferSolutionAlgo, DataSurfaces::HeatTransferModel::CondFD);
    // Check the LWR Exchange Coefficient, W/m2.C
    EXPECT_DOUBLE_EQ(state->dataMstBal->HSurrFD(1), 3.1650351180116170);
    EXPECT_DOUBLE_EQ(state->dataMstBal->HSurrFD(2), 3.1650351180116170);
    EXPECT_DOUBLE_EQ(state->dataMstBal->HSurrFD(3), 2.6375292650096811);
    EXPECT_DOUBLE_EQ(state->dataMstBal->HSurrFD(4), 0.0);
    // check the LWR exchange flux, W/m2
    Real64 const lwr_flux_Surf1 = 3.1650351180116170 * (25.0 - 20.0);
    Real64 const lwr_flux_Surf2 = 3.1650351180116170 * (25.0 - 20.0);
    Real64 const lwr_flux_Surf3 = 2.6375292650096811 * (25.0 - 20.0);
    EXPECT_DOUBLE_EQ(state->dataHeatBalSurf->SurfQRadLWOutSrdSurfs(1), lwr_flux_Surf1);
    EXPECT_DOUBLE_EQ(state->dataHeatBalSurf->SurfQRadLWOutSrdSurfs(2), lwr_flux_Surf2);
    EXPECT_DOUBLE_EQ(state->dataHeatBalSurf->SurfQRadLWOutSrdSurfs(3), lwr_flux_Surf3);
    EXPECT_DOUBLE_EQ(state->dataHeatBalSurf->SurfQRadLWOutSrdSurfs(4), 0.0);
}

TEST_F(EnergyPlusFixture, HeatBalanceSurfaceManager_ZoneFaceConductionVariableTest)
{
    // Added as part of work on Defect #10812
    auto &dS = state->dataSurface;
    auto &dHB = state->dataHeatBal;
    auto &dHBSurf = state->dataHeatBalSurf;
    auto &dG = state->dataGlobal;

    Real64 constexpr closeEnough = 0.00001;

    // Set up data that doesn't change and then allocate arrays
    dG->BeginSimFlag = false;
    dS->TotSurfaces = 2;
    state->dataViewFactor->NumOfSolarEnclosures = 1;
    dG->NumOfZones = 1;
    dG->DisplayAdvancedReportVariables = true;
    state->dataOutRptTab->displayHeatEmissionsSummary = false;
    dS->AnyMovableInsulation = false;
    dG->ZoneSizingCalc = false;
    dHB->Zone.allocate(dG->NumOfZones);
    dHB->space.allocate(dG->NumOfZones);
    dS->Surface.allocate(dS->TotSurfaces);
    dS->SurfSunlitFrac.allocate(dS->TotSurfaces);
    dS->SurfSunlitFrac = 0.0;
    dS->SurfSunlitArea.allocate(dS->TotSurfaces);
    dS->SurfSunlitArea = 0.0;
    int constexpr NumTimeSteps(6);
    int constexpr HoursInDay(24);
    dHB->SurfSunlitFrac.allocate(HoursInDay, NumTimeSteps, dS->TotSurfaces);
    dHB->SurfSunlitFrac = 0.0;
    dHB->Zone(1).numSpaces = 1;
    dHB->Zone(1).spaceIndexes.allocate(dHB->Zone(1).numSpaces);
    dHB->Zone(1).spaceIndexes(1) = 1;
    dS->Surface(1).Area = 1.0;
    dS->Surface(2).Area = 2.0;
    dG->HourOfDay = 1;
    dG->TimeStep = 1;
    dHB->space(1).OpaqOrIntMassSurfaceFirst = 1;
    dHB->space(1).OpaqOrIntMassSurfaceLast = 2;
    dHB->SurfInitialDifSolInAbsReport.allocate(dS->TotSurfaces);
    dHB->SurfSWInAbsTotalReport.allocate(dS->TotSurfaces);
    dHB->ZoneOpaqSurfInsFaceCond.allocate(dG->NumOfZones);
    dHB->ZoneOpaqSurfInsFaceCondGainRep.allocate(dG->NumOfZones);
    dHB->ZoneOpaqSurfInsFaceCondLossRep.allocate(dG->NumOfZones);
    dHB->ZnOpqSurfInsFaceCondGnRepEnrg.allocate(dG->NumOfZones);
    dHB->ZnOpqSurfInsFaceCondLsRepEnrg.allocate(dG->NumOfZones);
    dHB->ZoneOpaqSurfExtFaceCond.allocate(dG->NumOfZones);
    dHB->ZoneOpaqSurfExtFaceCondGainRep.allocate(dG->NumOfZones);
    dHB->ZoneOpaqSurfExtFaceCondLossRep.allocate(dG->NumOfZones);
    dHB->ZnOpqSurfExtFaceCondGnRepEnrg.allocate(dG->NumOfZones);
    dHB->ZnOpqSurfExtFaceCondLsRepEnrg.allocate(dG->NumOfZones);
    dG->TimeStepZoneSec = 60.0;
    dS->Surface(1).Area = 1.0;
    dS->Surface(2).Area = 10.0;

    AllocateSurfaceHeatBalArrays(*state);

    // Test 1: all positive
    dHBSurf->SurfOpaqInsFaceCondFlux(1) = 1.0;
    dHBSurf->SurfOpaqInsFaceCondFlux(2) = 2.0;
    dHBSurf->SurfOpaqOutFaceCondFlux(1) = 1.0;
    dHBSurf->SurfOpaqOutFaceCondFlux(2) = 2.0;
    dHB->ZoneOpaqSurfInsFaceCond(1) = 0.0;
    dHB->ZoneOpaqSurfInsFaceCondGainRep(1) = 0.0;
    dHB->ZoneOpaqSurfInsFaceCondLossRep(1) = 0.0;
    dHB->ZnOpqSurfInsFaceCondGnRepEnrg(1) = 0.0;
    dHB->ZnOpqSurfInsFaceCondLsRepEnrg(1) = 0.0;
    dHB->ZoneOpaqSurfExtFaceCond(1) = 0.0;
    dHB->ZoneOpaqSurfExtFaceCondGainRep(1) = 0.0;
    dHB->ZoneOpaqSurfExtFaceCondLossRep(1) = 0.0;
    dHB->ZnOpqSurfExtFaceCondGnRepEnrg(1) = 0.0;
    dHB->ZnOpqSurfExtFaceCondLsRepEnrg(1) = 0.0;

    ReportSurfaceHeatBalance(*state);

    EXPECT_NEAR(dHB->ZoneOpaqSurfInsFaceCond(1), 21.0, closeEnough);
    EXPECT_NEAR(dHB->ZoneOpaqSurfInsFaceCondGainRep(1), 21.0, closeEnough);
    EXPECT_NEAR(dHB->ZoneOpaqSurfInsFaceCondLossRep(1), 0.0, closeEnough);
    EXPECT_NEAR(dHB->ZnOpqSurfInsFaceCondGnRepEnrg(1), 1260.0, closeEnough);
    EXPECT_NEAR(dHB->ZnOpqSurfInsFaceCondLsRepEnrg(1), 0.0, closeEnough);
    EXPECT_NEAR(dHB->ZoneOpaqSurfExtFaceCond(1), 21.0, closeEnough);
    EXPECT_NEAR(dHB->ZoneOpaqSurfExtFaceCondGainRep(1), 21.0, closeEnough);
    EXPECT_NEAR(dHB->ZoneOpaqSurfExtFaceCondLossRep(1), 0.0, closeEnough);
    EXPECT_NEAR(dHB->ZnOpqSurfExtFaceCondGnRepEnrg(1), 1260.0, closeEnough);
    EXPECT_NEAR(dHB->ZnOpqSurfExtFaceCondLsRepEnrg(1), 0.0, closeEnough);

    // Test 2: positive and negative, sum is positive
    dHBSurf->SurfOpaqInsFaceCondFlux(1) = -10.0;
    dHBSurf->SurfOpaqInsFaceCondFlux(2) = 2.0;
    dHBSurf->SurfOpaqOutFaceCondFlux(1) = -10.0;
    dHBSurf->SurfOpaqOutFaceCondFlux(2) = 2.0;
    dHB->ZoneOpaqSurfInsFaceCond(1) = 0.0;
    dHB->ZoneOpaqSurfInsFaceCondGainRep(1) = 0.0;
    dHB->ZoneOpaqSurfInsFaceCondLossRep(1) = 0.0;
    dHB->ZnOpqSurfInsFaceCondGnRepEnrg(1) = 0.0;
    dHB->ZnOpqSurfInsFaceCondLsRepEnrg(1) = 0.0;
    dHB->ZoneOpaqSurfExtFaceCond(1) = 0.0;
    dHB->ZoneOpaqSurfExtFaceCondGainRep(1) = 0.0;
    dHB->ZoneOpaqSurfExtFaceCondLossRep(1) = 0.0;
    dHB->ZnOpqSurfExtFaceCondGnRepEnrg(1) = 0.0;
    dHB->ZnOpqSurfExtFaceCondLsRepEnrg(1) = 0.0;

    ReportSurfaceHeatBalance(*state);

    EXPECT_NEAR(dHB->ZoneOpaqSurfInsFaceCond(1), 10.0, closeEnough);
    EXPECT_NEAR(dHB->ZoneOpaqSurfInsFaceCondGainRep(1), 10.0, closeEnough);
    EXPECT_NEAR(dHB->ZoneOpaqSurfInsFaceCondLossRep(1), 0.0, closeEnough);
    EXPECT_NEAR(dHB->ZnOpqSurfInsFaceCondGnRepEnrg(1), 600.0, closeEnough);
    EXPECT_NEAR(dHB->ZnOpqSurfInsFaceCondLsRepEnrg(1), 0.0, closeEnough);
    EXPECT_NEAR(dHB->ZoneOpaqSurfExtFaceCond(1), 10.0, closeEnough);
    EXPECT_NEAR(dHB->ZoneOpaqSurfExtFaceCondGainRep(1), 10.0, closeEnough);
    EXPECT_NEAR(dHB->ZoneOpaqSurfExtFaceCondLossRep(1), 0.0, closeEnough);
    EXPECT_NEAR(dHB->ZnOpqSurfExtFaceCondGnRepEnrg(1), 600.0, closeEnough);
    EXPECT_NEAR(dHB->ZnOpqSurfExtFaceCondLsRepEnrg(1), 0.0, closeEnough);

    // Test 3: positive and negative, sum is negative
    dHBSurf->SurfOpaqInsFaceCondFlux(1) = -30.0;
    dHBSurf->SurfOpaqInsFaceCondFlux(2) = 2.0;
    dHBSurf->SurfOpaqOutFaceCondFlux(1) = -30.0;
    dHBSurf->SurfOpaqOutFaceCondFlux(2) = 2.0;
    dHB->ZoneOpaqSurfInsFaceCond(1) = 0.0;
    dHB->ZoneOpaqSurfInsFaceCondGainRep(1) = 0.0;
    dHB->ZoneOpaqSurfInsFaceCondLossRep(1) = 0.0;
    dHB->ZnOpqSurfInsFaceCondGnRepEnrg(1) = 0.0;
    dHB->ZnOpqSurfInsFaceCondLsRepEnrg(1) = 0.0;
    dHB->ZoneOpaqSurfExtFaceCond(1) = 0.0;
    dHB->ZoneOpaqSurfExtFaceCondGainRep(1) = 0.0;
    dHB->ZoneOpaqSurfExtFaceCondLossRep(1) = 0.0;
    dHB->ZnOpqSurfExtFaceCondGnRepEnrg(1) = 0.0;
    dHB->ZnOpqSurfExtFaceCondLsRepEnrg(1) = 0.0;

    ReportSurfaceHeatBalance(*state);

    EXPECT_NEAR(dHB->ZoneOpaqSurfInsFaceCond(1), -10.0, closeEnough);
    EXPECT_NEAR(dHB->ZoneOpaqSurfInsFaceCondGainRep(1), 0.0, closeEnough);
    EXPECT_NEAR(dHB->ZoneOpaqSurfInsFaceCondLossRep(1), 10.0, closeEnough);
    EXPECT_NEAR(dHB->ZnOpqSurfInsFaceCondGnRepEnrg(1), 0.0, closeEnough);
    EXPECT_NEAR(dHB->ZnOpqSurfInsFaceCondLsRepEnrg(1), 600.0, closeEnough);
    EXPECT_NEAR(dHB->ZoneOpaqSurfExtFaceCond(1), -10.0, closeEnough);
    EXPECT_NEAR(dHB->ZoneOpaqSurfExtFaceCondGainRep(1), 0.0, closeEnough);
    EXPECT_NEAR(dHB->ZoneOpaqSurfExtFaceCondLossRep(1), 10.0, closeEnough);
    EXPECT_NEAR(dHB->ZnOpqSurfExtFaceCondGnRepEnrg(1), 0.0, closeEnough);
    EXPECT_NEAR(dHB->ZnOpqSurfExtFaceCondLsRepEnrg(1), 600.0, closeEnough);

    // Test 4: all negative
    dHBSurf->SurfOpaqInsFaceCondFlux(1) = -1.0;
    dHBSurf->SurfOpaqInsFaceCondFlux(2) = -2.0;
    dHBSurf->SurfOpaqOutFaceCondFlux(1) = -1.0;
    dHBSurf->SurfOpaqOutFaceCondFlux(2) = -2.0;
    dHB->ZoneOpaqSurfInsFaceCond(1) = 0.0;
    dHB->ZoneOpaqSurfInsFaceCondGainRep(1) = 0.0;
    dHB->ZoneOpaqSurfInsFaceCondLossRep(1) = 0.0;
    dHB->ZnOpqSurfInsFaceCondGnRepEnrg(1) = 0.0;
    dHB->ZnOpqSurfInsFaceCondLsRepEnrg(1) = 0.0;
    dHB->ZoneOpaqSurfExtFaceCond(1) = 0.0;
    dHB->ZoneOpaqSurfExtFaceCondGainRep(1) = 0.0;
    dHB->ZoneOpaqSurfExtFaceCondLossRep(1) = 0.0;
    dHB->ZnOpqSurfExtFaceCondGnRepEnrg(1) = 0.0;
    dHB->ZnOpqSurfExtFaceCondLsRepEnrg(1) = 0.0;

    ReportSurfaceHeatBalance(*state);

    EXPECT_NEAR(dHB->ZoneOpaqSurfInsFaceCond(1), -21.0, closeEnough);
    EXPECT_NEAR(dHB->ZoneOpaqSurfInsFaceCondGainRep(1), .0, closeEnough);
    EXPECT_NEAR(dHB->ZoneOpaqSurfInsFaceCondLossRep(1), 21.0, closeEnough);
    EXPECT_NEAR(dHB->ZnOpqSurfInsFaceCondGnRepEnrg(1), 0.0, closeEnough);
    EXPECT_NEAR(dHB->ZnOpqSurfInsFaceCondLsRepEnrg(1), 1260.0, closeEnough);
    EXPECT_NEAR(dHB->ZoneOpaqSurfExtFaceCond(1), -21.0, closeEnough);
    EXPECT_NEAR(dHB->ZoneOpaqSurfExtFaceCondGainRep(1), 0.0, closeEnough);
    EXPECT_NEAR(dHB->ZoneOpaqSurfExtFaceCondLossRep(1), 21.0, closeEnough);
    EXPECT_NEAR(dHB->ZnOpqSurfExtFaceCondGnRepEnrg(1), 0.0, closeEnough);
    EXPECT_NEAR(dHB->ZnOpqSurfExtFaceCondLsRepEnrg(1), 1260.0, closeEnough);
}

} // namespace EnergyPlus
