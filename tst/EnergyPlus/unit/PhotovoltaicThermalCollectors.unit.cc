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

// EnergyPlus::ElectricPowerServiceManager Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataPhotovoltaics.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/HeatBalanceSurfaceManager.hh>
#include <EnergyPlus/PhotovoltaicThermalCollectors.hh>
#include <EnergyPlus/Photovoltaics.hh>
#include <EnergyPlus/SolarShading.hh>
#include <EnergyPlus/SurfaceGeometry.hh>

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, BIPVT_calc_k_taoalpha)
{
    state->init_state(*state);
    PhotovoltaicThermalCollectors::PVTCollectorStruct thisBIPVT;
    Real64 theta = 0.0; // lower value
    Real64 glass_thickness = 0.001;
    Real64 refrac_index_glass = 1.0;
    Real64 k_glass = 1.0;
    Real64 k_taoalpha = 0.0;
    k_taoalpha = thisBIPVT.calc_k_taoalpha(theta, glass_thickness, refrac_index_glass, k_glass);
    EXPECT_EQ(k_taoalpha, 1.0);
    theta = Constant::Pi; // higher value
    k_taoalpha = thisBIPVT.calc_k_taoalpha(theta, glass_thickness, refrac_index_glass, k_glass);
    EXPECT_EQ(k_taoalpha, 0.0);
    theta = Constant::Pi / 2.0; // mid-range value
    k_taoalpha = thisBIPVT.calc_k_taoalpha(theta, glass_thickness, refrac_index_glass, k_glass);
    EXPECT_EQ(k_taoalpha, 0.0);
    theta = Constant::Pi / 4.0; // mid-range value
    refrac_index_glass = 2.0;   // higher value
    k_taoalpha = thisBIPVT.calc_k_taoalpha(theta, glass_thickness, refrac_index_glass, k_glass);
    EXPECT_NEAR(k_taoalpha, 0.986, 0.001);
    k_glass = 10.0; // higher value
    k_taoalpha = thisBIPVT.calc_k_taoalpha(theta, glass_thickness, refrac_index_glass, k_glass);
    EXPECT_NEAR(k_taoalpha, 0.986, 0.001);
    k_glass = 32.0; // higher value
    theta = 0.0;
    glass_thickness = 0.006;
    refrac_index_glass = 1.52;
    k_taoalpha = thisBIPVT.calc_k_taoalpha(theta, glass_thickness, refrac_index_glass, k_glass);
    EXPECT_EQ(k_taoalpha, 1.0);
    k_glass = 32.0; // higher value
    theta = Constant::Pi / 4.0;
    glass_thickness = 0.006;
    refrac_index_glass = 1.52;
    k_taoalpha = thisBIPVT.calc_k_taoalpha(theta, glass_thickness, refrac_index_glass, k_glass);
    EXPECT_NEAR(k_taoalpha, 0.965, 0.001);
}

TEST_F(EnergyPlusFixture, BIPVT_calculateBIPVTMaxHeatGain)
{
    std::string const idf_objects = delimited_string({
        "  Zone,",
        "    ZN_1_FLR_1_SEC_1;               !- Name",
        "",
        "  SurfaceProperty:OtherSideConditionsModel,",
        "    OSCM_ZN_1_FLR_1_SEC_1, !- Name",
        "    GapConvectionRadiation; !- Type of Modeling",
        "",
        "  Construction,",
        "    ASHRAE 90.1-2004_Sec 5.5-3ab_IEAD_Roof,  !- Name",
        "    Roof Membrane,           !- Outside Layer",
        "    Roof Insulation,         !- Layer 2",
        "    Metal Decking;           !- Layer 3",
        "",
        "  Material,",
        "    Roof Membrane,           !- Name",
        "    VeryRough,               !- Roughness",
        "    0.0095,                  !- Thickness {m}",
        "    0.1600,                  !- Conductivity {W/m-K}",
        "    1121.2900,               !- Density {kg/m3}",
        "    1460.0000,               !- Specific Heat {J/kg-K}",
        "    0.9000,                  !- Thermal Absorptance",
        "    0.7000,                  !- Solar Absorptance",
        "    0.7000;                  !- Visible Absorptance",
        "",
        "  Material,",
        "    Roof Insulation,         !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1250,                  !- Thickness {m}",
        "    0.0490,                  !- Conductivity {W/m-K}",
        "    265.0000,                !- Density {kg/m3}",
        "    836.8000,                !- Specific Heat {J/kg-K}",
        "    0.9000,                  !- Thermal Absorptance",
        "    0.7000,                  !- Solar Absorptance",
        "    0.7000;                  !- Visible Absorptance",
        "",
        "  Material,",
        "    Metal Decking,           !- Name",
        "    MediumSmooth,            !- Roughness",
        "    0.0015,                  !- Thickness {m}",
        "    45.0060,                 !- Conductivity {W/m-K}",
        "    7680.0000,               !- Density {kg/m3}",
        "    418.4000,                !- Specific Heat {J/kg-K}",
        "    0.9000,                  !- Thermal Absorptance",
        "    0.7000,                  !- Solar Absorptance",
        "    0.3000;                  !- Visible Absorptance",
        "",
        "  BuildingSurface:Detailed,",
        "    ZN_1_FLR_1_SEC_1_Ceiling,!- Name",
        "    ceiling,                 !- Surface Type",
        "    ASHRAE 90.1-2004_Sec 5.5-3ab_IEAD_Roof,  !- Construction Name",
        "    ZN_1_FLR_1_SEC_1,        !- Zone Name",
        "    ,                        !- Space Name",
        "    OtherSideConditionsModel,                !- Outside Boundary Condition",
        "    OSCM_ZN_1_FLR_1_SEC_1,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.0000,                  !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    4.570,0.000,5.009,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.186,0.000,5.009,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.186,4.570,5.009,  !- X,Y,Z ==> Vertex 3 {m}",
        "    4.570,4.570,5.009;  !- X,Y,Z ==> Vertex 4 {m}",

        "  SolarCollector:FlatPlate:PhotovoltaicThermal,",
        "    PVT:ZN_1_FLR_1_SEC_1_Ceiling,  !- Name",
        "    ZN_1_FLR_1_SEC_1_Ceiling,!- Surface Name",
        "    ZN_1_FLR_1_SEC_1_CEILING_BIPVT,    !- Photovoltaic-Thermal Model Performance Name",
        "    PV:ZN_1_FLR_1_SEC_1_Ceiling,  !- Photovoltaic Name",
        "    Air,                     !- Thermal Working Fluid Type",
        "    ,                        !- Water Inlet Node Name",
        "    ,                        !- Water Outlet Node Name",
        "    ZN_1_FLR_1_SEC_1:Sys_OAInlet Node,  !- Air Inlet Node Name",
        "    PVT:ZN_1_FLR_1_SEC_1_Ceiling Outlet,  !- Air Outlet Node Name",
        "    Autosize;                !- Design Flow Rate {m3/s}",
        "                                                                                                                  ",
        "  SolarCollectorPerformance:PhotovoltaicThermal:BIPVT,",
        "    ZN_1_FLR_1_SEC_1_Ceiling_BIPVT, !- Name",
        "    OSCM_ZN_1_FLR_1_SEC_1,  !- Boundary Conditions Model Name",
        "    , !- Availability Schedule Name",
        "    0.1, !- Effective Plenum Gap Thickness Behind PV Modules",
        "    0.957, !- PV Cell Normal Transmittance-Absorptance Product",
        "    0.87, !- Backing Material Normal Transmittance-Absorptance Product",
        "    0.85, !- Cladding Normal Transmittance-Absorptance Product",
        "    0.85, !- Fraction of Collector Gross Area Covered by PV module",
        "    0.9, !- Fraction of PV cell area to PV module area",
        "    0.0044, !- PV Module Thermal Resistance - Top",
        "    0.0039, !- PV Module Thermal Resistance - Bottom",
        "    0.85, !- PV Module Longwave Emissivity",
        "    0.9, !- Backing Material Longwave Emissivity",
        "    0.002, !- Glass Thickness",
        "    1.526, !- Glass Refraction Index",
        "    4.0; !- Glass Extinction Coefficient",

        "  Generator:Photovoltaic,",
        "    PV:ZN_1_FLR_1_SEC_1_Ceiling,  !- Name",
        "    ZN_1_FLR_1_SEC_1_Ceiling,!- Surface Name",
        "    PhotovoltaicPerformance:EquivalentOne-Diode,  !- Photovoltaic Performance Object Type",
        "    SiemensSamplePVModule,  !- Module Performance Name",
        "    PhotovoltaicThermalSolarCollector,  !- Heat Transfer Integration Mode",
        "    17.0,                     !- Number of Series Strings in Parallel {dimensionless}",
        "    4.0;                     !- Number of Modules in Series {dimensionless}",
        "",
        "  PhotovoltaicPerformance:EquivalentOne-Diode,",
        "    SiemensSamplePVModule, !- Name",
        "    CrystallineSilicon, !- Cell Type",
        "    36, !- Number of Cells in Series [-]",
        "    1.0, !- Area Active [m2]",
        "    0.957, !- Transmittance Absorptance Product",
        "    1.12, !- Semiconductor Bandgap [eV]",
        "    1000000, !- Shunt Resistance [Ohms]",
        "    6.5, !- Short Circuit Current [A/K]",
        "    21.6, !- Open Circuit Voltage [V/K]",
        "    25, !- Reference Temperature [C]",
        "    1000, !- Reference Insolation [W/m2]",
        "    5.9, !- Module Current at Maximum Power [A]",
        "    17, !- Module Voltage at Maximum Power [V]",
        "    0.002, !- Temperature Coefficient of Short Circuit Current",
        "    -0.079, !- Temperature Coefficient of Open Circuit Voltage",
        "    20, !- Nominal Operating Cell Temperature Test Ambient Temperature [C]",
        "    45, !- Nominal Operating Cell Temperature Test Cell Temperature [C]",
        "    800, !- Nominal Operating Cell Temperature Test Insolation [W/m2]",
        "    30, !- Module Heat Loss Coefficient [W/m2.K]",
        "    50000; !- Total Heat Capacity [J/m2-K]",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // bool foundErrors = false;
    // HeatBalanceManager::GetMaterialData(*state, foundErrors); // read material data
    // EXPECT_FALSE(foundErrors);                                // expect no errors

    // HeatBalanceManager::GetConstructData(*state, foundErrors); // read construction data
    // compare_err_stream("");
    // EXPECT_FALSE(foundErrors); // expect no errors

    // HeatBalanceManager::GetZoneData(*state, foundErrors); // read zone data
    // EXPECT_FALSE(foundErrors);                            // expect no errors

    // SurfaceGeometry::GetSurfaceData(*state, foundErrors); // setup zone geometry and get zone data
    // EXPECT_FALSE(foundErrors);                            // expect no errors

    HeatBalanceManager::GetHeatBalanceInput(*state); // Gets materials, constructions, zones, surfaces, etc.
    HeatBalanceSurfaceManager::AllocateSurfaceHeatBalArrays(*state);

    Photovoltaics::GetPVInput(*state);
    PhotovoltaicThermalCollectors::GetPVTcollectorsInput(*state);
    state->dataGlobal->BeginSimFlag = true;
    state->dataGlobal->BeginEnvrnFlag = true;
    SolarShading::InitSolarCalculations(*state);

    auto thisBIPVT = state->dataPhotovoltaicThermalCollector->PVT(1);

    Real64 tempSetPoint = 24.0;
    thisBIPVT.OperatingMode = PhotovoltaicThermalCollectors::PVTMode::Heating; // this should be converted to an enum?
    Real64 bypassFraction = 0.0;
    Real64 potentialHeatGain = 0.0;
    Real64 potentialOutletTemp = 0.0;
    Real64 eff = 0.0;
    Real64 tCollector = 0.0;

    // Set up conditions
    int InletNode = Util::FindItemInList("ZN_1_FLR_1_SEC_1:SYS_OAINLET NODE",
                                         state->dataLoopNodes->NodeID,
                                         state->dataLoopNodes->NumOfNodes); // HVAC node associated with inlet of BIPVT
    state->dataLoopNodes->Node(InletNode).HumRat = 0.001;                   // inlet air humidity ratio (kgda/kg)
    state->dataEnvrn->OutHumRat = 0.001;                                    // ambient humidity ratio (kg/kg)
    state->dataEnvrn->SkyTemp = 0.0;                                        // sky temperature (DegC)
    state->dataEnvrn->WindSpeed = 5.0;                                      // wind speed (m/s)
    state->dataEnvrn->WindDir = 0.0;                                        // wind direction (deg)
    state->dataPhotovoltaic->PVarray(thisBIPVT.PVnum).TRNSYSPVcalc.ArrayEfficiency = 0.5;
    state->dataHeatBal->SurfQRadSWOutIncidentGndDiffuse(thisBIPVT.SurfNum) = 0.0; // Exterior ground diffuse solar incident on surface (W/m2)
    state->dataHeatBal->SurfCosIncidenceAngle(thisBIPVT.SurfNum) = 0.5;           // Cosine of beam solar incidence angle

    // case 1: heating mode bypass fraction = 0.0
    thisBIPVT.OperatingMode = PhotovoltaicThermalCollectors::PVTMode::Heating;
    tempSetPoint = 24.0;
    state->dataLoopNodes->Node(InletNode).Temp = 10.0;                              // inlet fluid temperature (DegC)
    state->dataEnvrn->OutDryBulbTemp = 10.0;                                        // ambient temperature (DegC)
    state->dataHeatBalSurf->SurfTempOut(thisBIPVT.SurfNum) = 12.0;                  // temperature of bldg surface (DegC)
    thisBIPVT.MassFlowRate = 0.01;                                                  // fluid mass flow rate (kg/s)
    state->dataHeatBal->SurfQRadSWOutIncidentBeam(thisBIPVT.SurfNum) = 500.0;       // Exterior beam solar incident on surface (W/m2)
    state->dataHeatBal->SurfQRadSWOutIncidentSkyDiffuse(thisBIPVT.SurfNum) = 100.0; // Exterior sky diffuse solar incident on surface (W/m2)
    state->dataHeatBal->SurfQRadSWOutIncident(thisBIPVT.SurfNum) = 568.7;           // total incident solar radiation
    thisBIPVT.calculateBIPVTMaxHeatGain(*state, tempSetPoint, bypassFraction, potentialHeatGain, potentialOutletTemp, eff, tCollector);

    EXPECT_NEAR(bypassFraction, 0.0, 0.001);
    EXPECT_NEAR(potentialHeatGain, 41.68, 0.01);
    EXPECT_NEAR(potentialOutletTemp, 14.14, 0.01);
    EXPECT_NEAR(eff, 0.0013, 0.0001);
    EXPECT_NEAR(tCollector, 21.57, 0.01);

    // case 2: double the channel depth vs case 1
    thisBIPVT.OperatingMode = PhotovoltaicThermalCollectors::PVTMode::Heating;
    tempSetPoint = 24.0;
    state->dataLoopNodes->Node(InletNode).Temp = 10.0;
    state->dataEnvrn->OutDryBulbTemp = 10.0;
    state->dataHeatBalSurf->SurfTempOut(thisBIPVT.SurfNum) = 12.0;
    thisBIPVT.MassFlowRate = 0.01;
    thisBIPVT.BIPVT.PVEffGapWidth = 0.2;
    state->dataHeatBal->SurfQRadSWOutIncidentBeam(thisBIPVT.SurfNum) = 500.0;
    state->dataHeatBal->SurfQRadSWOutIncidentSkyDiffuse(thisBIPVT.SurfNum) = 100.0;
    state->dataHeatBal->SurfQRadSWOutIncident(thisBIPVT.SurfNum) = 568.7;
    thisBIPVT.calculateBIPVTMaxHeatGain(*state, tempSetPoint, bypassFraction, potentialHeatGain, potentialOutletTemp, eff, tCollector);

    EXPECT_NEAR(bypassFraction, 0.0, 0.001);
    EXPECT_NEAR(potentialHeatGain, 41.59, 0.01);
    EXPECT_NEAR(potentialOutletTemp, 14.13, 0.01);
    EXPECT_NEAR(eff, 0.0013, 0.001);
    EXPECT_NEAR(tCollector, 21.64, 0.01);

    // case 3: higher mass flow rate vs case 1
    thisBIPVT.OperatingMode = PhotovoltaicThermalCollectors::PVTMode::Heating;
    tempSetPoint = 24.0;
    state->dataLoopNodes->Node(InletNode).Temp = 10.0;
    state->dataEnvrn->OutDryBulbTemp = 10.0;
    state->dataHeatBalSurf->SurfTempOut(thisBIPVT.SurfNum) = 12.0;
    thisBIPVT.MassFlowRate = 0.1;
    thisBIPVT.BIPVT.PVEffGapWidth = 0.1;
    state->dataHeatBal->SurfQRadSWOutIncidentBeam(thisBIPVT.SurfNum) = 500.0;
    state->dataHeatBal->SurfQRadSWOutIncidentSkyDiffuse(thisBIPVT.SurfNum) = 100.0;
    state->dataHeatBal->SurfQRadSWOutIncident(thisBIPVT.SurfNum) = 568.7;
    thisBIPVT.calculateBIPVTMaxHeatGain(*state, tempSetPoint, bypassFraction, potentialHeatGain, potentialOutletTemp, eff, tCollector);

    EXPECT_NEAR(bypassFraction, 0.0, 0.001);
    EXPECT_NEAR(potentialHeatGain, 524.64, 0.01);
    EXPECT_NEAR(potentialOutletTemp, 15.21, 0.01);
    EXPECT_NEAR(eff, 0.016, 0.001);
    EXPECT_NEAR(tCollector, 20.93, 0.01);

    // case 4: heating mode and bypass fraction > 0.0
    thisBIPVT.OperatingMode = PhotovoltaicThermalCollectors::PVTMode::Heating;
    tempSetPoint = 24.0;
    state->dataLoopNodes->Node(InletNode).Temp = 23.0;
    state->dataEnvrn->OutDryBulbTemp = 23.0;
    state->dataHeatBalSurf->SurfTempOut(thisBIPVT.SurfNum) = 23.0;
    thisBIPVT.MassFlowRate = 0.01;
    state->dataHeatBal->SurfQRadSWOutIncidentBeam(thisBIPVT.SurfNum) = 500.0;
    state->dataHeatBal->SurfQRadSWOutIncidentSkyDiffuse(thisBIPVT.SurfNum) = 100.0;
    state->dataHeatBal->SurfQRadSWOutIncident(thisBIPVT.SurfNum) = 568.7;
    thisBIPVT.calculateBIPVTMaxHeatGain(*state, tempSetPoint, bypassFraction, potentialHeatGain, potentialOutletTemp, eff, tCollector);

    EXPECT_NEAR(bypassFraction, 0.430, 0.001);
    EXPECT_NEAR(potentialHeatGain, 9.27, 0.01);
    EXPECT_NEAR(potentialOutletTemp, 23.92, 0.01);
    EXPECT_NEAR(eff, 0.0003, 0.0001);
    EXPECT_NEAR(tCollector, 31.32, 0.01);

    // case 5: heating mode and bypass fraction = 1.0
    thisBIPVT.OperatingMode = PhotovoltaicThermalCollectors::PVTMode::Heating;
    tempSetPoint = 24.0;
    state->dataLoopNodes->Node(InletNode).Temp = 25.0;
    state->dataEnvrn->OutDryBulbTemp = 25.0;
    state->dataHeatBalSurf->SurfTempOut(thisBIPVT.SurfNum) = 24.0;
    thisBIPVT.MassFlowRate = 0.01;
    state->dataHeatBal->SurfQRadSWOutIncidentBeam(thisBIPVT.SurfNum) = 500.0;
    state->dataHeatBal->SurfQRadSWOutIncidentSkyDiffuse(thisBIPVT.SurfNum) = 100.0;
    state->dataHeatBal->SurfQRadSWOutIncident(thisBIPVT.SurfNum) = 568.7;
    thisBIPVT.calculateBIPVTMaxHeatGain(*state, tempSetPoint, bypassFraction, potentialHeatGain, potentialOutletTemp, eff, tCollector);

    EXPECT_NEAR(bypassFraction, 1.0, 0.001);
    EXPECT_NEAR(potentialHeatGain, 0.0, 0.01);
    EXPECT_NEAR(potentialOutletTemp, 25.0, 0.01);
    EXPECT_NEAR(eff, 0.0, 0.0001);
    EXPECT_NEAR(tCollector, 32.73, 0.01);

    // case 6: cooling mode and bypass fraction = 0.0
    thisBIPVT.OperatingMode = PhotovoltaicThermalCollectors::PVTMode::Cooling;
    tempSetPoint = 13.0;
    state->dataLoopNodes->Node(InletNode).Temp = 30.0;
    state->dataEnvrn->OutDryBulbTemp = 30.0;
    state->dataHeatBalSurf->SurfTempOut(thisBIPVT.SurfNum) = 22.0;
    thisBIPVT.MassFlowRate = 0.01;
    state->dataHeatBal->SurfQRadSWOutIncidentBeam(thisBIPVT.SurfNum) = 500.0;
    state->dataHeatBal->SurfQRadSWOutIncidentSkyDiffuse(thisBIPVT.SurfNum) = 100.0;
    state->dataHeatBal->SurfQRadSWOutIncident(thisBIPVT.SurfNum) = 568.7;
    thisBIPVT.calculateBIPVTMaxHeatGain(*state, tempSetPoint, bypassFraction, potentialHeatGain, potentialOutletTemp, eff, tCollector);

    EXPECT_NEAR(bypassFraction, 0.0, 0.001);
    EXPECT_NEAR(potentialHeatGain, -52.01, 0.01);
    EXPECT_NEAR(potentialOutletTemp, 24.83, 0.01);
    EXPECT_NEAR(eff, 0.0, 0.0001);
    EXPECT_NEAR(tCollector, 34.95, 0.01);

    // case 7: cooling mode and bypass fraction > 0.0
    thisBIPVT.OperatingMode = PhotovoltaicThermalCollectors::PVTMode::Cooling;
    tempSetPoint = 22.0;
    state->dataLoopNodes->Node(InletNode).Temp = 25.0;
    state->dataEnvrn->OutDryBulbTemp = 25.0;
    state->dataHeatBalSurf->SurfTempOut(thisBIPVT.SurfNum) = 20.0;
    thisBIPVT.MassFlowRate = 0.01;
    state->dataHeatBal->SurfQRadSWOutIncidentBeam(thisBIPVT.SurfNum) = 0.0;
    state->dataHeatBal->SurfQRadSWOutIncidentSkyDiffuse(thisBIPVT.SurfNum) = 0.0;
    state->dataHeatBal->SurfQRadSWOutIncident(thisBIPVT.SurfNum) = 0.0;
    thisBIPVT.calculateBIPVTMaxHeatGain(*state, tempSetPoint, bypassFraction, potentialHeatGain, potentialOutletTemp, eff, tCollector);

    EXPECT_NEAR(bypassFraction, 0.414, 0.001);
    EXPECT_NEAR(potentialHeatGain, -30.20, 0.01);
    EXPECT_NEAR(potentialOutletTemp, 22.0, 0.01);
    EXPECT_NEAR(eff, 0.0, 0.0001);
    EXPECT_NEAR(tCollector, 19.33, 0.01);

    // case 8: higher mass flow rate vs case 6
    thisBIPVT.OperatingMode = PhotovoltaicThermalCollectors::PVTMode::Cooling;
    tempSetPoint = 22.0;
    state->dataLoopNodes->Node(InletNode).Temp = 25.0;
    state->dataEnvrn->OutDryBulbTemp = 25.0;
    state->dataHeatBalSurf->SurfTempOut(thisBIPVT.SurfNum) = 20.0;
    thisBIPVT.MassFlowRate = 1.0;
    state->dataHeatBal->SurfQRadSWOutIncidentBeam(thisBIPVT.SurfNum) = 0.0;
    state->dataHeatBal->SurfQRadSWOutIncidentSkyDiffuse(thisBIPVT.SurfNum) = 0.0;
    state->dataHeatBal->SurfQRadSWOutIncident(thisBIPVT.SurfNum) = 0.0;
    thisBIPVT.calculateBIPVTMaxHeatGain(*state, tempSetPoint, bypassFraction, potentialHeatGain, potentialOutletTemp, eff, tCollector);

    EXPECT_NEAR(bypassFraction, 0.248, 0.001);
    EXPECT_NEAR(potentialHeatGain, -3023.06, 0.01);
    EXPECT_NEAR(potentialOutletTemp, 22.0, 0.01);
    EXPECT_NEAR(eff, 0.0, 0.0001);
    EXPECT_NEAR(tCollector, 20.38, 0.01);

    // case 9: cooling mode and bypass fraction = 1.0
    thisBIPVT.OperatingMode = PhotovoltaicThermalCollectors::PVTMode::Cooling;
    tempSetPoint = 22.0;
    state->dataLoopNodes->Node(InletNode).Temp = 20.0;
    state->dataEnvrn->OutDryBulbTemp = 20.0;
    state->dataHeatBalSurf->SurfTempOut(thisBIPVT.SurfNum) = 18.0;
    thisBIPVT.MassFlowRate = 0.01;
    state->dataHeatBal->SurfQRadSWOutIncidentBeam(thisBIPVT.SurfNum) = 0.0;
    state->dataHeatBal->SurfQRadSWOutIncidentSkyDiffuse(thisBIPVT.SurfNum) = 0.0;
    state->dataHeatBal->SurfQRadSWOutIncident(thisBIPVT.SurfNum) = 0.0;
    thisBIPVT.calculateBIPVTMaxHeatGain(*state, tempSetPoint, bypassFraction, potentialHeatGain, potentialOutletTemp, eff, tCollector);

    EXPECT_NEAR(bypassFraction, 1.0, 0.001);
    EXPECT_NEAR(potentialHeatGain, 0.0, 0.01);
    EXPECT_NEAR(potentialOutletTemp, 20.0, 0.01);
    EXPECT_NEAR(eff, 0.0, 0.0001);
    EXPECT_NEAR(tCollector, 15.91, 0.01);

    // case 10: heating mode and bypass fraction > 0.0, more realistic conditions
    thisBIPVT.OperatingMode = PhotovoltaicThermalCollectors::PVTMode::Heating;
    tempSetPoint = 24.0;
    state->dataLoopNodes->Node(InletNode).Temp = -20.0;
    state->dataEnvrn->SkyTemp = -20.0;
    state->dataEnvrn->OutDryBulbTemp = -20.0;
    state->dataHeatBalSurf->SurfTempOut(thisBIPVT.SurfNum) = -18.0;
    thisBIPVT.MassFlowRate = 1.0;
    state->dataHeatBal->SurfQRadSWOutIncidentBeam(thisBIPVT.SurfNum) = 500.0;
    state->dataHeatBal->SurfQRadSWOutIncidentSkyDiffuse(thisBIPVT.SurfNum) = 100.0;
    state->dataHeatBal->SurfQRadSWOutIncident(thisBIPVT.SurfNum) = 568.7;
    state->dataPhotovoltaic->PVarray(thisBIPVT.PVnum).TRNSYSPVcalc.ArrayEfficiency = 0.22;
    thisBIPVT.calculateBIPVTMaxHeatGain(*state, tempSetPoint, bypassFraction, potentialHeatGain, potentialOutletTemp, eff, tCollector);

    EXPECT_NEAR(bypassFraction, 0.0, 0.001);
    EXPECT_NEAR(potentialHeatGain, 6832.41, 0.01);
    EXPECT_NEAR(potentialOutletTemp, -13.21, 0.01);
    EXPECT_NEAR(eff, 0.2084, 0.0001);
    EXPECT_NEAR(tCollector, -5.61, 0.01);

    // case 11: heating mode bypass fraction = 0.0
    thisBIPVT.OperatingMode = PhotovoltaicThermalCollectors::PVTMode::Heating;
    tempSetPoint = 24.0;
    state->dataLoopNodes->Node(InletNode).Temp = 10.0;                              // inlet fluid temperature (DegC)
    state->dataEnvrn->OutDryBulbTemp = 10.0;                                        // ambient temperature (DegC)
    state->dataHeatBalSurf->SurfTempOut(thisBIPVT.SurfNum) = 12.0;                  // temperature of bldg surface (DegC)
    thisBIPVT.MassFlowRate = 1.0;                                                   // fluid mass flow rate (kg/s)
    state->dataHeatBal->SurfQRadSWOutIncidentBeam(thisBIPVT.SurfNum) = 500.0;       // Exterior beam solar incident on surface (W/m2)
    state->dataHeatBal->SurfQRadSWOutIncidentSkyDiffuse(thisBIPVT.SurfNum) = 100.0; // Exterior sky diffuse solar incident on surface (W/m2)
    state->dataHeatBal->SurfQRadSWOutIncident(thisBIPVT.SurfNum) = 568.7;           // total incident solar radiation
    thisBIPVT.calculateBIPVTMaxHeatGain(*state, tempSetPoint, bypassFraction, potentialHeatGain, potentialOutletTemp, eff, tCollector);

    EXPECT_NEAR(bypassFraction, 0.0, 0.001);
    EXPECT_NEAR(potentialHeatGain, 5145.21, 0.01);
    EXPECT_NEAR(potentialOutletTemp, 15.11, 0.01);
    EXPECT_NEAR(eff, 0.1569, 0.0001);
    EXPECT_NEAR(tCollector, 20.24, 0.01);

    //  Add a few more cases with some changes to external conditions and/or BIPVT properties
    // If you want to use different idf inputs, then probably best to copy the entire test
}
