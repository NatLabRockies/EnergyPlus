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

// EnergyPlus::AirflowNetwork component-related unit tests

// Google test headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <AirflowNetwork/Elements.hpp>
#include <AirflowNetwork/Properties.hpp>
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/UtilityRoutines.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;
using namespace AirflowNetwork;
using namespace DataHeatBalance;

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, AirflowNetwork_SolverTest_HorizontalOpening)
{

    int i = 1;
    int j = 1;
    int n;
    int m;
    int NF;
    std::array<Real64, 2> F{{0.0, 0.0}};
    std::array<Real64, 2> DF{{0.0, 0.0}};

    n = 1;
    m = 2;

    state->afn->AirflowNetworkCompData.allocate(j);
    state->afn->AirflowNetworkCompData(j).TypeNum = 1;
    state->afn->MultizoneSurfaceData.allocate(i);
    state->afn->MultizoneSurfaceData(i).Width = 10.0;
    state->afn->MultizoneSurfaceData(i).Height = 5.0;
    state->afn->MultizoneSurfaceData(i).OpenFactor = 1.0;

    state->afn->node_states.clear();
    for (int it = 0; it < 2; ++it) {
        state->afn->node_states.emplace_back(AirState(AIRDENSITY_CONSTEXPR(20.0, 101325.0, 0.0)));
    }
    state->afn->node_states[0].density = 1.2;
    state->afn->node_states[1].density = 1.18;

    state->afn->MultizoneCompHorOpeningData.allocate(1);
    state->afn->MultizoneCompHorOpeningData(1).FlowCoef = 0.1;
    state->afn->MultizoneCompHorOpeningData(1).FlowExpo = 0.5;
    state->afn->MultizoneCompHorOpeningData(1).Slope = 90.0;
    state->afn->MultizoneCompHorOpeningData(1).DischCoeff = 0.2;

    state->afn->AirflowNetworkLinkageData.allocate(i);
    state->afn->AirflowNetworkLinkageData(i).NodeHeights[0] = 4.0;
    state->afn->AirflowNetworkLinkageData(i).NodeHeights[1] = 2.0;

    Real64 multiplier = 1.0;
    Real64 control = 1.0;

    NF = state->afn->MultizoneCompHorOpeningData(1).calculate(
        *state, 1, 0.05, 1, multiplier, control, state->afn->node_states[0], state->afn->node_states[1], F, DF);
    EXPECT_NEAR(3.47863, F[0], 0.00001);
    EXPECT_NEAR(34.7863, DF[0], 0.0001);
    EXPECT_NEAR(2.96657, F[1], 0.00001);
    EXPECT_EQ(0.0, DF[1]);

    NF = state->afn->MultizoneCompHorOpeningData(1).calculate(
        *state, 1, -0.05, 1, multiplier, control, state->afn->node_states[0], state->afn->node_states[1], F, DF);
    EXPECT_NEAR(-3.42065, F[0], 0.00001);
    EXPECT_NEAR(34.20649, DF[0], 0.0001);
    EXPECT_NEAR(2.96657, F[1], 0.00001);
    EXPECT_EQ(0.0, DF[1]);

    state->afn->AirflowNetworkLinkageData.deallocate();

    state->afn->MultizoneCompHorOpeningData.deallocate();
    state->afn->MultizoneSurfaceData.deallocate();
    state->afn->AirflowNetworkCompData.deallocate();
}

TEST_F(EnergyPlusFixture, AirflowNetwork_SolverTest_Coil)
{

    int NF;
    std::array<Real64, 2> F;
    std::array<Real64, 2> DF;

    state->afn->AirflowNetworkCompData.allocate(1);
    state->afn->AirflowNetworkCompData[0].TypeNum = 1;

    state->afn->DisSysCompCoilData.allocate(1);
    state->afn->DisSysCompCoilData[0].hydraulicDiameter = 1.0;
    state->afn->DisSysCompCoilData[0].L = 1.0;

    state->afn->node_states.clear();
    for (int it = 0; it < 2; ++it) {
        state->afn->node_states.emplace_back(AirState(AIRDENSITY_CONSTEXPR(20.0, 101325.0, 0.0)));
    }
    state->afn->node_states[0].density = 1.2;
    state->afn->node_states[1].density = 1.2;

    state->afn->node_states[0].viscosity = 1.0e-5;
    state->afn->node_states[1].viscosity = 1.0e-5;

    F[1] = DF[1] = 0.0;

    Real64 multiplier = 1.0;
    Real64 control = 1.0;

    NF = state->afn->DisSysCompCoilData[0].calculate(
        *state, 1, 0.05, 1, multiplier, control, state->afn->node_states[0], state->afn->node_states[1], F, DF);
    EXPECT_NEAR(-294.5243112740431, F[0], 0.00001);
    EXPECT_NEAR(5890.4862254808613, DF[0], 0.0001);
    EXPECT_EQ(0.0, F[1]);
    EXPECT_EQ(0.0, DF[1]);

    NF = state->afn->DisSysCompCoilData[0].calculate(
        *state, 1, -0.05, 1, multiplier, control, state->afn->node_states[0], state->afn->node_states[1], F, DF);
    EXPECT_NEAR(294.5243112740431, F[0], 0.00001);
    EXPECT_NEAR(5890.4862254808613, DF[0], 0.0001);
    EXPECT_EQ(0.0, F[1]);
    EXPECT_EQ(0.0, DF[1]);

    state->afn->DisSysCompCoilData.deallocate();
    state->afn->AirflowNetworkCompData.deallocate();
}

TEST_F(EnergyPlusFixture, AirflowNetwork_SolverTest_Crack)
{

    int NF;
    std::array<Real64, 2> F = {0.0, 0.0};
    std::array<Real64, 2> DF = {0.0, 0.0};

    AirflowNetwork::SurfaceCrack crack;
    crack.coefficient = 0.001;
    crack.exponent = 0.65;

    AirflowNetwork::AirState state0, state1;
    Real64 sqrt_density = state0.sqrt_density; // = state1.sqrtDensity
    Real64 viscosity = state0.viscosity;       // = state1.viscosity

    Real64 dp{10.0};

    // Linear
    NF = crack.calculate(*state, true, dp, 0, 1.0, 1.0, state0, state1, F, DF);
    EXPECT_EQ(0.01 * sqrt_density / viscosity, F[0]);
    EXPECT_EQ(0.0, F[1]);
    EXPECT_EQ(0.001 * sqrt_density / viscosity, DF[0]);
    EXPECT_EQ(0.0, DF[1]);
    EXPECT_EQ(1, NF);

    NF = crack.calculate(*state, true, -dp, 0, 1.0, 1.0, state0, state1, F, DF);
    EXPECT_EQ(-0.01 * sqrt_density / viscosity, F[0]);
    EXPECT_EQ(0.0, F[1]);
    EXPECT_EQ(0.001 * sqrt_density / viscosity, DF[0]);
    EXPECT_EQ(0.0, DF[1]);
    EXPECT_EQ(1, NF);

    // Nonlinear
    NF = crack.calculate(*state, false, dp, 0, 1.0, 1.0, state0, state1, F, DF);
    EXPECT_EQ(0.001 * std::pow(10.0, 0.65), F[0]);
    EXPECT_EQ(0.0, F[1]);
    EXPECT_DOUBLE_EQ(0.000065 * std::pow(10.0, 0.65), DF[0]);
    EXPECT_EQ(0.0, DF[1]);
    EXPECT_EQ(1, NF);

    NF = crack.calculate(*state, false, -dp, 0, 1.0, 1.0, state0, state1, F, DF);
    EXPECT_EQ(-0.001 * std::pow(10.0, 0.65), F[0]);
    EXPECT_EQ(0.0, F[1]);
    EXPECT_DOUBLE_EQ(0.000065 * std::pow(10.0, 0.65), DF[0]);
    EXPECT_EQ(0.0, DF[1]);
    EXPECT_EQ(1, NF);
}

TEST_F(EnergyPlusFixture, AirflowNetwork_SolverTest_GenericCrack)
{

    std::array<Real64, 2> F = {0.0, 0.0};
    std::array<Real64, 2> DF = {0.0, 0.0};

    Real64 coef{0.001};
    Real64 expo{0.65};

    AirflowNetwork::AirState state0, state1;
    Real64 sqrt_density = state0.sqrt_density; // = state1.sqrtDensity
    Real64 viscosity = state0.viscosity;       // = state1.viscosity

    Real64 dp{10.0};

    // Linear
    AirflowNetwork::generic_crack(coef, expo, true, dp, state0, state1, F, DF);
    EXPECT_EQ(0.01 * sqrt_density / viscosity, F[0]);
    EXPECT_EQ(0.0, F[1]);
    EXPECT_EQ(0.001 * sqrt_density / viscosity, DF[0]);
    EXPECT_EQ(0.0, DF[1]);

    AirflowNetwork::generic_crack(coef, expo, true, -dp, state0, state1, F, DF);
    EXPECT_EQ(-0.01 * sqrt_density / viscosity, F[0]);
    EXPECT_EQ(0.0, F[1]);
    EXPECT_EQ(0.001 * sqrt_density / viscosity, DF[0]);
    EXPECT_EQ(0.0, DF[1]);

    // Nonlinear
    AirflowNetwork::generic_crack(coef, expo, false, dp, state0, state1, F, DF);
    EXPECT_EQ(0.001 * std::pow(10.0, 0.65), F[0]);
    EXPECT_EQ(0.0, F[1]);
    EXPECT_DOUBLE_EQ(0.000065 * std::pow(10.0, 0.65), DF[0]);
    EXPECT_EQ(0.0, DF[1]);

    AirflowNetwork::generic_crack(coef, expo, false, -dp, state0, state1, F, DF);
    EXPECT_EQ(-0.001 * std::pow(10.0, 0.65), F[0]);
    EXPECT_EQ(0.0, F[1]);
    EXPECT_DOUBLE_EQ(0.000065 * std::pow(10.0, 0.65), DF[0]);
    EXPECT_EQ(0.0, DF[1]);
}

TEST_F(EnergyPlusFixture, AirflowNetwork_SolverTest_SpecifiedMassFlow)
{

    int NF;
    std::array<Real64, 2> F = {0.0, 0.0};
    std::array<Real64, 2> DF = {0.0, 0.0};

    AirflowNetwork::SpecifiedMassFlow element;
    element.mass_flow = 0.1;

    AirflowNetwork::AirState state0, state1;

    Real64 dp{10.0};
    Real64 f = element.mass_flow;

    // Linear
    NF = element.calculate(*state, true, dp, 0, 1.0, 1.0, state0, state1, F, DF);
    EXPECT_EQ(f, F[0]);
    EXPECT_EQ(0.0, F[1]);
    EXPECT_EQ(0.0, DF[0]);
    EXPECT_EQ(0.0, DF[1]);
    EXPECT_EQ(1, NF);

    NF = element.calculate(*state, true, -dp, 0, 1.0, 1.0, state0, state1, F, DF);
    EXPECT_EQ(f, F[0]);
    EXPECT_EQ(0.0, F[1]);
    EXPECT_EQ(0.0, DF[0]);
    EXPECT_EQ(0.0, DF[1]);
    EXPECT_EQ(1, NF);

    // Nonlinear tests
    NF = element.calculate(*state, false, dp, 0, 1.0, 1.0, state0, state1, F, DF);
    EXPECT_EQ(f, F[0]);
    EXPECT_EQ(0.0, F[1]);
    EXPECT_EQ(0.0, DF[0]);
    EXPECT_EQ(0.0, DF[1]);
    EXPECT_EQ(1, NF);

    NF = element.calculate(*state, false, -dp, 0, 1.0, 1.0, state0, state1, F, DF);
    EXPECT_EQ(f, F[0]);
    EXPECT_EQ(0.0, F[1]);
    EXPECT_EQ(0.0, DF[0]);
    EXPECT_EQ(0.0, DF[1]);
    EXPECT_EQ(1, NF);
}

TEST_F(EnergyPlusFixture, AirflowNetwork_SolverTest_SpecifiedVolumeFlow)
{

    int NF;
    std::array<Real64, 2> F = {0.0, 0.0};
    std::array<Real64, 2> DF = {0.0, 0.0};

    AirflowNetwork::SpecifiedVolumeFlow element;
    element.volume_flow = 0.1;

    AirflowNetwork::AirState state0, state1;
    Real64 density = state0.density; // = state1.density

    Real64 dp{10.0};
    Real64 f = element.volume_flow * density;

    // Linear
    NF = element.calculate(*state, true, dp, 0, 1.0, 1.0, state0, state1, F, DF);
    EXPECT_EQ(f, F[0]);
    EXPECT_EQ(0.0, F[1]);
    EXPECT_EQ(0.0, DF[0]);
    EXPECT_EQ(0.0, DF[1]);
    EXPECT_EQ(1, NF);

    NF = element.calculate(*state, true, -dp, 0, 1.0, 1.0, state0, state1, F, DF);
    EXPECT_EQ(f, F[0]);
    EXPECT_EQ(0.0, F[1]);
    EXPECT_EQ(0.0, DF[0]);
    EXPECT_EQ(0.0, DF[1]);
    EXPECT_EQ(1, NF);

    // Nonlinear tests
    NF = element.calculate(*state, false, dp, 0, 1.0, 1.0, state0, state1, F, DF);
    EXPECT_EQ(f, F[0]);
    EXPECT_EQ(0.0, F[1]);
    EXPECT_EQ(0.0, DF[0]);
    EXPECT_EQ(0.0, DF[1]);
    EXPECT_EQ(1, NF);

    NF = element.calculate(*state, false, -dp, 0, 1.0, 1.0, state0, state1, F, DF);
    EXPECT_EQ(f, F[0]);
    EXPECT_EQ(0.0, F[1]);
    EXPECT_EQ(0.0, DF[0]);
    EXPECT_EQ(0.0, DF[1]);
    EXPECT_EQ(1, NF);
}

TEST_F(EnergyPlusFixture, AirflowNetwork_TestTriangularWindowWarning)
{

    // Unit test for #5384

    state->dataHeatBal->Zone.allocate(1);
    state->dataHeatBal->Zone(1).Name = "WEST_ZONE";

    state->dataSurface->Surface.allocate(3);
    state->dataSurface->Surface(1).Name = "SURFACE_1";
    state->dataSurface->Surface(1).Zone = 1;
    state->dataSurface->Surface(1).ZoneName = "WEST_ZONE";
    state->dataSurface->Surface(1).Azimuth = 0.0;
    state->dataSurface->Surface(1).ExtBoundCond = 0;
    state->dataSurface->Surface(1).HeatTransSurf = true;
    state->dataSurface->Surface(1).Tilt = 90.0;
    state->dataSurface->Surface(1).Sides = 4;
    state->dataSurface->Surface(2).Name = "SURFACE_2";
    state->dataSurface->Surface(2).Zone = 1;
    state->dataSurface->Surface(2).ZoneName = "WEST_ZONE";
    state->dataSurface->Surface(2).Azimuth = 180.0;
    state->dataSurface->Surface(2).ExtBoundCond = 0;
    state->dataSurface->Surface(2).HeatTransSurf = true;
    state->dataSurface->Surface(2).Tilt = 90.0;
    state->dataSurface->Surface(2).Sides = 4;
    state->dataSurface->Surface(3).Name = "WINDOW1";
    state->dataSurface->Surface(3).Zone = 1;
    state->dataSurface->Surface(3).ZoneName = "WEST_ZONE";
    state->dataSurface->Surface(3).Azimuth = 180.0;
    state->dataSurface->Surface(3).ExtBoundCond = 0;
    state->dataSurface->Surface(3).HeatTransSurf = true;
    state->dataSurface->Surface(3).Tilt = 90.0;
    state->dataSurface->Surface(3).Sides = 3;
    state->dataSurface->Surface(3).Vertex.allocate(3);
    state->dataSurface->Surface(3).Vertex(1).x = 3.0;
    state->dataSurface->Surface(3).Vertex(2).x = 3.0;
    state->dataSurface->Surface(3).Vertex(3).x = 1.0;
    state->dataSurface->Surface(3).Vertex(1).y = 10.778;
    state->dataSurface->Surface(3).Vertex(2).y = 10.778;
    state->dataSurface->Surface(3).Vertex(3).y = 10.778;
    state->dataSurface->Surface(3).Vertex(1).z = 2.0;
    state->dataSurface->Surface(3).Vertex(2).z = 1.0;
    state->dataSurface->Surface(3).Vertex(3).z = 1.0;

    SurfaceGeometry::AllocateSurfaceWindows(*state, 3);
    state->dataSurface->Surface(1).OriginalClass = DataSurfaces::SurfaceClass::Window;
    state->dataSurface->Surface(2).OriginalClass = DataSurfaces::SurfaceClass::Window;
    state->dataSurface->Surface(3).OriginalClass = DataSurfaces::SurfaceClass::Window;
    state->dataGlobal->NumOfZones = 1;

    std::string const idf_objects = delimited_string({
        "Schedule:Constant,OnSch,,1.0;",
        "Schedule:Constant,Aula people sched,,0.0;",
        "Schedule:Constant,Sempre 21,,21.0;",
        "AirflowNetwork:SimulationControl,",
        "  NaturalVentilation, !- Name",
        "  MultizoneWithoutDistribution, !- AirflowNetwork Control",
        "  SurfaceAverageCalculation, !- Wind Pressure Coefficient Type",
        "  , !- Height Selection for Local Wind Pressure Calculation",
        "  LOWRISE, !- Building Type",
        "  1000, !- Maximum Number of Iterations{ dimensionless }",
        "  LinearInitializationMethod, !- Initialization Type",
        "  0.0001, !- Relative Airflow Convergence Tolerance{ dimensionless }",
        "  0.0001, !- Absolute Airflow Convergence Tolerance{ kg / s }",
        "  -0.5, !- Convergence Acceleration Limit{ dimensionless }",
        "  90, !- Azimuth Angle of Long Axis of Building{ deg }",
        "  0.36;                    !- Ratio of Building Width Along Short Axis to Width Along Long Axis",
        "AirflowNetwork:MultiZone:Zone,",
        "  WEST_ZONE, !- Zone Name",
        "  Temperature, !- Ventilation Control Mode",
        "  Sempre 21, !- Ventilation Control Zone Temperature Setpoint Schedule Name",
        "  1, !- Minimum Venting Open Factor{ dimensionless }",
        "  , !- Indoor and Outdoor Temperature Difference Lower Limit For Maximum Venting Open Factor{ deltaC }",
        "  100, !- Indoor and Outdoor Temperature Difference Upper Limit for Minimum Venting Open Factor{ deltaC }",
        "  , !- Indoor and Outdoor Enthalpy Difference Lower Limit For Maximum Venting Open Factor{ deltaJ / kg }",
        "  300000, !- Indoor and Outdoor Enthalpy Difference Upper Limit for Minimum Venting Open Factor{ deltaJ / kg }",
        "  Aula people sched, !- Venting Availability Schedule Name",
        "  Standard;                !- Single Sided Wind Pressure Coefficient Algorithm",
        "AirflowNetwork:MultiZone:Surface,",
        "  Surface_1, !- Surface Name",
        "  CR-1, !- Leakage Component Name",
        "  , !- External Node Name",
        "  1; !- Window / Door Opening Factor, or Crack Factor{ dimensionless }",
        "AirflowNetwork:MultiZone:Surface,",
        "  Surface_2, !- Surface Name",
        "  CR-1, !- Leakage Component Name",
        "  , !- External Node Name",
        "  1; !- Window / Door Opening Factor, or Crack Factor{ dimensionless }",
        "AirflowNetwork:MultiZone:Surface,",
        "  Window1, !- Surface Name",
        "  Simple Window, !- Leakage Component Name",
        "  , !- External Node Name",
        "  1; !- Window / Door Opening Factor, or Crack Factor{ dimensionless }",
        "AirflowNetwork:MultiZone:Component:SimpleOpening,",
        "  Simple Window, !- Name",
        "  0.0010, !- Air Mass Flow Coefficient When Opening is Closed{ kg / s - m }",
        "  0.65, !- Air Mass Flow Exponent When Opening is Closed{ dimensionless }",
        "  0.01, !- Minimum Density Difference for Two - Way Flow{ kg / m3 }",
        "  0.78;                    !- Discharge Coefficient{ dimensionless }",
        "AirflowNetwork:MultiZone:ReferenceCrackConditions,",
        "  ReferenceCrackConditions, !- Name",
        "  20.0, !- Reference Temperature{ C }",
        "  101320, !- Reference Barometric Pressure{ Pa }",
        "  0.005;                   !- Reference Humidity Ratio{ kgWater / kgDryAir }",
        "AirflowNetwork:MultiZone:Surface:Crack,",
        "  CR-1, !- Name",
        "  0.01, !- Air Mass Flow Coefficient at Reference Conditions{ kg / s }",
        "  0.667, !- Air Mass Flow Exponent{ dimensionless }",
        "  ReferenceCrackConditions; !- Reference Crack Conditions",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    state->afn->get_input();
    std::string const error_string = delimited_string({
        "   ** Warning ** ProcessScheduleInput: Schedule:Constant = ONSCH",
        "   **   ~~~   ** Schedule Type Limits Name is empty.",
        "   **   ~~~   ** Schedule will not be validated.",
        "   ** Warning ** ProcessScheduleInput: Schedule:Constant = AULA PEOPLE SCHED",
        "   **   ~~~   ** Schedule Type Limits Name is empty.",
        "   **   ~~~   ** Schedule will not be validated.",
        "   ** Warning ** ProcessScheduleInput: Schedule:Constant = SEMPRE 21",
        "   **   ~~~   ** Schedule Type Limits Name is empty.",
        "   **   ~~~   ** Schedule will not be validated.",
        "   ** Warning ** AirflowNetwork::Solver::get_input: AirflowNetwork:MultiZone:Surface=\"WINDOW1\".",
        "   **   ~~~   ** The opening is a Triangular subsurface. A rectangular subsurface will be used with equivalent width and height.",
    });

    EXPECT_TRUE(compare_err_stream(error_string, true));

    state->afn->AirflowNetworkNodeData.deallocate();
    state->afn->AirflowNetworkCompData.deallocate();
    state->afn->MultizoneExternalNodeData.deallocate();
    state->dataHeatBal->Zone.deallocate();
    state->dataSurface->Surface.deallocate();
    state->dataSurface->SurfaceWindow.deallocate();
}

TEST_F(EnergyPlusFixture, AirflowNetwork_UserDefinedDuctViewFactors)
{

    std::string const idf_objects = delimited_string({
        "  SimulationControl,",
        "    No,                      !- Do Zone Sizing Calculation",
        "    No,                      !- Do System Sizing Calculation",
        "    No,                      !- Do Plant Sizing Calculation",
        "    No,                      !- Run Simulation for Sizing Periods",
        "    Yes;                     !- Run Simulation for Weather File Run Periods",

        "  Building,",
        "    Exercise 1A,             !- Name",
        "    0.0,                     !- North Axis {deg}",
        "    Country,                 !- Terrain",
        "    0.04,                    !- Loads Convergence Tolerance Value",
        "    0.4,                     !- Temperature Convergence Tolerance Value {deltaC}",
        "    FullInteriorAndExterior, !- Solar Distribution",
        "    ,                        !- Maximum Number of Warmup Days",
        "    6;                       !- Minimum Number of Warmup Days",

        "  SurfaceConvectionAlgorithm:Inside,",
        "    TARP;                    !- Algorithm",

        "  SurfaceConvectionAlgorithm:Outside,",
        "    TARP;                    !- Algorithm",

        "  HeatBalanceAlgorithm,",
        "    ConductionTransferFunction;  !- Algorithm",

        "  Timestep,",
        "    4;                       !- Number of Timesteps per Hour",

        "  Site:Location,",
        "    Pheonix,                 !- Name",
        "    33.43,                   !- Latitude {deg}",
        "    -112.02,                 !- Longitude {deg}",
        "    -7.0,                    !- Time Zone {hr}",
        "    339.0;                   !- Elevation {m}",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Cooling .4% Conditions DB=>MWB,  !- Name",
        "    7,                       !- Month",
        "    21,                      !- Day of Month",
        "    SummerDesignDay,         !- Day Type",
        "    32.80000,                !- Maximum Dry-Bulb Temperature {C}",
        "    10.90000,                !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    23.60000,                !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.21,                !- Barometric Pressure {Pa}",
        "    0.0,                     !- Wind Speed {m/s}",
        "    0.0,                     !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    1.000000;                !- Sky Clearness",

        "  SizingPeriod:DesignDay,",
        "    CHICAGO_IL_USA Heating 99.6% Conditions,  !- Name",
        "    1,                       !- Month",
        "    21,                      !- Day of Month",
        "    WinterDesignDay,         !- Day Type",
        "    -21.20000,               !- Maximum Dry-Bulb Temperature {C}",
        "    0.0,                     !- Daily Dry-Bulb Temperature Range {deltaC}",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Type",
        "    ,                        !- Dry-Bulb Temperature Range Modifier Day Schedule Name",
        "    Wetbulb,                 !- Humidity Condition Type",
        "    -21.20000,               !- Wetbulb or DewPoint at Maximum Dry-Bulb {C}",
        "    ,                        !- Humidity Condition Day Schedule Name",
        "    ,                        !- Humidity Ratio at Maximum Dry-Bulb {kgWater/kgDryAir}",
        "    ,                        !- Enthalpy at Maximum Dry-Bulb {J/kg}",
        "    ,                        !- Daily Wet-Bulb Temperature Range {deltaC}",
        "    99063.21,                !- Barometric Pressure {Pa}",
        "    4.600000,                !- Wind Speed {m/s}",
        "    270.0000,                !- Wind Direction {deg}",
        "    No,                      !- Rain Indicator",
        "    No,                      !- Snow Indicator",
        "    No,                      !- Daylight Saving Time Indicator",
        "    ASHRAEClearSky,          !- Solar Model Indicator",
        "    ,                        !- Beam Solar Day Schedule Name",
        "    ,                        !- Diffuse Solar Day Schedule Name",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Beam Irradiance (taub) {dimensionless}",
        "    ,                        !- ASHRAE Clear Sky Optical Depth for Diffuse Irradiance (taud) {dimensionless}",
        "    0.0;                     !- Sky Clearness",

        "  RunPeriod,",
        "    RP1,                     !- Name",
        "    1,                       !- Begin Month",
        "    1,                       !- Begin Day of Month",
        "    ,                        !- Begin Year",
        "    12,                      !- End Month",
        "    31,                      !- End Day of Month",
        "    ,                        !- End Year",
        "    Tuesday,                 !- Day of Week for Start Day",
        "    Yes,                     !- Use Weather File Holidays and Special Days",
        "    Yes,                     !- Use Weather File Daylight Saving Period",
        "    No,                      !- Apply Weekend Holiday Rule",
        "    Yes,                     !- Use Weather File Rain Indicators",
        "    Yes;                     !- Use Weather File Snow Indicators",

        "  Site:GroundTemperature:BuildingSurface,",
        "    23.0,                    !- January Ground Temperature {C}",
        "    23.0,                    !- February Ground Temperature {C}",
        "    23.0,                    !- March Ground Temperature {C}",
        "    23.0,                    !- April Ground Temperature {C}",
        "    23.0,                    !- May Ground Temperature {C}",
        "    23.0,                    !- June Ground Temperature {C}",
        "    23.0,                    !- July Ground Temperature {C}",
        "    23.0,                    !- August Ground Temperature {C}",
        "    23.0,                    !- September Ground Temperature {C}",
        "    23.0,                    !- October Ground Temperature {C}",
        "    23.0,                    !- November Ground Temperature {C}",
        "    23.0;                    !- December Ground Temperature {C}",

        "  ScheduleTypeLimits,",
        "    Temperature,             !- Name",
        "    -60,                     !- Lower Limit Value",
        "    200,                     !- Upper Limit Value",
        "    CONTINUOUS,              !- Numeric Type",
        "    Temperature;             !- Unit Type",

        "  ScheduleTypeLimits,",
        "    Control Type,            !- Name",
        "    0,                       !- Lower Limit Value",
        "    4,                       !- Upper Limit Value",
        "    DISCRETE;                !- Numeric Type",

        "  ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    CONTINUOUS;              !- Numeric Type",

        "  Schedule:Compact,",
        "    HVACAvailSched,          !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    1.0;                     !- Field 4",

        "  Schedule:Compact,",
        "    Dual Heating Setpoints,  !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    23.0;                    !- Field 4",

        "  Schedule:Compact,",
        "    Dual Cooling Setpoints,  !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    23.0;                    !- Field 4",

        "  Schedule:Compact,",
        "    Dual Zone Control Type Sched,  !- Name",
        "    Control Type,            !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,            !- Field 3",
        "    4;                       !- Field 4",

        "  Material,",
        "    Gypsum Board,            !- Name",
        "    MediumSmooth,            !- Roughness",
        "    0.0127,                  !- Thickness {m}",
        "    0.160158849,             !- Conductivity {W/m-K}",
        "    800.923168698,           !- Density {kg/m3}",
        "    1087.84,                 !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.9,                     !- Solar Absorptance",
        "    1.0;                     !- Visible Absorptance",

        "  Material,",
        "    Gypsum Board Wall,       !- Name",
        "    MediumSmooth,            !- Roughness",
        "    0.0127,                  !- Thickness {m}",
        "    0.160158849,             !- Conductivity {W/m-K}",
        "    800.923168698,           !- Density {kg/m3}",
        "    1087.84,                 !- Specific Heat {J/kg-K}",
        "    1e-6,                    !- Thermal Absorptance",
        "    1e-6,                    !- Solar Absorptance",
        "    1.0;                     !- Visible Absorptance",

        "  Material,",
        "    R-19 Insulation,         !- Name",
        "    Rough,                   !- Roughness",
        "    0.88871384,              !- Thickness {m}",
        "    0.25745056,              !- Conductivity {W/m-K}",
        "    3.05091836,              !- Density {kg/m3}",
        "    794.96,                  !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.9,                     !- Solar Absorptance",
        "    1.0;                     !- Visible Absorptance",

        "  Material,",
        "    R-A Lot,                 !- Name",
        "    Rough,                   !- Roughness",
        "    1.25,                    !- Thickness {m}",
        "    0.001,                   !- Conductivity {W/m-K}",
        "    3.05091836,              !- Density {kg/m3}",
        "    794.96,                  !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.9,                     !- Solar Absorptance",
        "    1.0;                     !- Visible Absorptance",

        "  Material,",
        "    Shingles,                !- Name",
        "    Rough,                   !- Roughness",
        "    0.006348984,             !- Thickness {m}",
        "    0.081932979,             !- Conductivity {W/m-K}",
        "    1121.292436177,          !- Density {kg/m3}",
        "    1256.04,                 !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.9,                     !- Solar Absorptance",
        "    1.0;                     !- Visible Absorptance",

        "  Material,",
        "    Felt,                    !- Name",
        "    Rough,                   !- Roughness",
        "    0.00216408,              !- Thickness {m}",
        "    0.081932979,             !- Conductivity {W/m-K}",
        "    1121.292436177,          !- Density {kg/m3}",
        "    1507.248,                !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.9,                     !- Solar Absorptance",
        "    1.0;                     !- Visible Absorptance",

        "  Material,",
        "    Plywood,                 !- Name",
        "    Rough,                   !- Roughness",
        "    0.012701016,             !- Thickness {m}",
        "    0.11544,                 !- Conductivity {W/m-K}",
        "    544.627754714,           !- Density {kg/m3}",
        "    1214.172,                !- Specific Heat {J/kg-K}",
        "    0.9,                     !- Thermal Absorptance",
        "    0.9,                     !- Solar Absorptance",
        "    1.0;                     !- Visible Absorptance",

        "  Material,",
        "    Hardboard Siding-Gable,  !- Name",
        "    MediumSmooth,            !- Roughness",
        "    0.0111125,               !- Thickness {m}",
        "    0.214957246,             !- Conductivity {W/m-K}",
        "    640.736,                 !- Density {kg/m3}",
        "    1172.304,                !- Specific Heat {J/kg-K}",
        "    0.90,                    !- Thermal Absorptance",
        "    0.7,                     !- Solar Absorptance",
        "    1.0;                     !- Visible Absorptance",

        "  Material,",
        "    Studs,                   !- Name",
        "    Rough,                   !- Roughness",
        "    0.0003137,               !- Thickness {m}",
        "    0.02189835,              !- Conductivity {W/m-K}",
        "    448.516974471,           !- Density {kg/m3}",
        "    1632.852,                !- Specific Heat {J/kg-K}",
        "    0.90,                    !- Thermal Absorptance",
        "    0.7,                     !- Solar Absorptance",
        "    1.0;                     !- Visible Absorptance",

        "  Material,",
        "    Hardboard Siding-Eave,   !- Name",
        "    MediumSmooth,            !- Roughness",
        "    0.0111125,               !- Thickness {m}",
        "    0.214957246,             !- Conductivity {W/m-K}",
        "    640.736,                 !- Density {kg/m3}",
        "    1172.304,                !- Specific Heat {J/kg-K}",
        "    0.90,                    !- Thermal Absorptance",
        "    0.7,                     !- Solar Absorptance",
        "    1.0;                     !- Visible Absorptance",

        "  Material,",
        "    HF-C5,                   !- Name",
        "    MediumRough,             !- Roughness",
        "    0.1015000,               !- Thickness {m}",
        "    1.729600,                !- Conductivity {W/m-K}",
        "    2243.000,                !- Density {kg/m3}",
        "    837.0000,                !- Specific Heat {J/kg-K}",
        "    0.9000000,               !- Thermal Absorptance",
        "    0.6500000,               !- Solar Absorptance",
        "    1.0;                     !- Visible Absorptance",

        "  Construction,",
        "    CeilingConstruction,     !- Name",
        "    R-19 Insulation,         !- Outside Layer",
        "    Gypsum Board;            !- Layer 2",

        "  Construction,",
        "    Reverse:CeilingConstruction,  !- Name",
        "    Gypsum Board,            !- Outside Layer",
        "    R-19 Insulation;         !- Layer 2",

        "  Construction,",
        "    Roof,                    !- Name",
        "    Shingles,                !- Outside Layer",
        "    Felt,                    !- Layer 2",
        "    Plywood;                 !- Layer 3",

        "  Construction,",
        "    Gables,                  !- Name",
        "    Hardboard Siding-Eave;   !- Outside Layer",

        "  Construction,",
        "    Eave Walls,              !- Name",
        "    Hardboard Siding-Eave;   !- Outside Layer",

        "  Construction,",
        "    Walls,                   !- Name",
        "    Hardboard Siding-Eave,   !- Outside Layer",
        "    R-A Lot,                 !- Layer 2",
        "    Gypsum Board Wall;       !- Layer 3",

        "  Construction,",
        "    LTFLOOR,                 !- Name",
        "    HF-C5;                   !- Outside Layer",

        "  GlobalGeometryRules,",
        "    UpperLeftCorner,         !- Starting Vertex Position",
        "    Counterclockwise,        !- Vertex Entry Direction",
        "    World;                   !- Coordinate System",

        "  Zone,",
        "    OCCUPIED ZONE,           !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  Zone,",
        "    ATTIC ZONE,              !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  BuildingSurface:Detailed,",
        "    North Wall,              !- Name",
        "    Wall,                    !- Surface Type",
        "    Walls,                   !- Construction Name",
        "    OCCUPIED ZONE,           !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50,                    !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    16.764,                  !- Vertex 1 X-coordinate {m}",
        "    8.5344,                  !- Vertex 1 Y-coordinate {m}",
        "    2.70,                    !- Vertex 1 Z-coordinate {m}",
        "    16.764,                  !- Vertex 2 X-coordinate {m}",
        "    8.5344,                  !- Vertex 2 Y-coordinate {m}",
        "    0,                       !- Vertex 2 Z-coordinate {m}",
        "    0,                       !- Vertex 3 X-coordinate {m}",
        "    8.5344,                  !- Vertex 3 Y-coordinate {m}",
        "    0,                       !- Vertex 3 Z-coordinate {m}",
        "    0,                       !- Vertex 4 X-coordinate {m}",
        "    8.5344,                  !- Vertex 4 Y-coordinate {m}",
        "    2.70;                    !- Vertex 4 Z-coordinate {m}",

        "  BuildingSurface:Detailed,",
        "    North Wall Attic,        !- Name",
        "    Wall,                    !- Surface Type",
        "    Eave Walls,              !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50,                    !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    16.764,                  !- Vertex 1 X-coordinate {m}",
        "    8.5344,                  !- Vertex 1 Y-coordinate {m}",
        "    2.7254,                  !- Vertex 1 Z-coordinate {m}",
        "    16.764,                  !- Vertex 2 X-coordinate {m}",
        "    8.5344,                  !- Vertex 2 Y-coordinate {m}",
        "    2.70,                    !- Vertex 2 Z-coordinate {m}",
        "    0,                       !- Vertex 3 X-coordinate {m}",
        "    8.5344,                  !- Vertex 3 Y-coordinate {m}",
        "    2.70,                    !- Vertex 3 Z-coordinate {m}",
        "    0,                       !- Vertex 4 X-coordinate {m}",
        "    8.5344,                  !- Vertex 4 Y-coordinate {m}",
        "    2.7254;                  !- Vertex 4 Z-coordinate {m}",

        "  BuildingSurface:Detailed,",
        "    East Wall,               !- Name",
        "    Wall,                    !- Surface Type",
        "    Walls,                   !- Construction Name",
        "    OCCUPIED ZONE,           !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50,                    !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    16.764,                  !- Vertex 1 X-coordinate {m}",
        "    0,                       !- Vertex 1 Y-coordinate {m}",
        "    2.70,                    !- Vertex 1 Z-coordinate {m}",
        "    16.764,                  !- Vertex 2 X-coordinate {m}",
        "    0,                       !- Vertex 2 Y-coordinate {m}",
        "    0,                       !- Vertex 2 Z-coordinate {m}",
        "    16.764,                  !- Vertex 3 X-coordinate {m}",
        "    8.5344,                  !- Vertex 3 Y-coordinate {m}",
        "    0,                       !- Vertex 3 Z-coordinate {m}",
        "    16.764,                  !- Vertex 4 X-coordinate {m}",
        "    8.5344,                  !- Vertex 4 Y-coordinate {m}",
        "    2.70;                    !- Vertex 4 Z-coordinate {m}",

        "  BuildingSurface:Detailed,",
        "    East Wall Attic,         !- Name",
        "    Wall,                    !- Surface Type",
        "    Gables,              !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50,                    !- View Factor to Ground",
        "    5,                       !- Number of Vertices",
        "    16.764,                  !- Vertex 1 X-coordinate {m}",
        "    0,                       !- Vertex 1 Y-coordinate {m}",
        "    2.7254,                  !- Vertex 1 Z-coordinate {m}",
        "    16.764,                  !- Vertex 2 X-coordinate {m}",
        "    0,                       !- Vertex 2 Y-coordinate {m}",
        "    2.70,                    !- Vertex 2 Z-coordinate {m}",
        "    16.764,                  !- Vertex 3 X-coordinate {m}",
        "    8.5344,                  !- Vertex 3 Y-coordinate {m}",
        "    2.70,                    !- Vertex 3 Z-coordinate {m}",
        "    16.764,                  !- Vertex 4 X-coordinate {m}",
        "    8.5344,                  !- Vertex 4 Y-coordinate {m}",
        "    2.7254,                  !- Vertex 4 Z-coordinate {m}",
        "    16.764,                  !- Vertex 5 X-coordinate {m}",
        "    4.2672,                  !- Vertex 5 Y-coordinate {m}",
        "    4.5034;                  !- Vertex 5 Z-coordinate {m}",

        "  BuildingSurface:Detailed,",
        "    South Wall,              !- Name",
        "    Wall,                    !- Surface Type",
        "    Walls,                   !- Construction Name",
        "    OCCUPIED ZONE,           !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50,                    !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,                       !- Vertex 1 X-coordinate {m}",
        "    0,                       !- Vertex 1 Y-coordinate {m}",
        "    2.70,                    !- Vertex 1 Z-coordinate {m}",
        "    0,                       !- Vertex 2 X-coordinate {m}",
        "    0,                       !- Vertex 2 Y-coordinate {m}",
        "    0,                       !- Vertex 2 Z-coordinate {m}",
        "    16.764,                  !- Vertex 3 X-coordinate {m}",
        "    0,                       !- Vertex 3 Y-coordinate {m}",
        "    0,                       !- Vertex 3 Z-coordinate {m}",
        "    16.764,                  !- Vertex 4 X-coordinate {m}",
        "    0,                       !- Vertex 4 Y-coordinate {m}",
        "    2.70;                    !- Vertex 4 Z-coordinate {m}",

        "  BuildingSurface:Detailed,",
        "    South Wall Attic,        !- Name",
        "    Wall,                    !- Surface Type",
        "    Eave Walls,              !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50,                    !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,                       !- Vertex 1 X-coordinate {m}",
        "    0,                       !- Vertex 1 Y-coordinate {m}",
        "    2.7254,                  !- Vertex 1 Z-coordinate {m}",
        "    0,                       !- Vertex 2 X-coordinate {m}",
        "    0,                       !- Vertex 2 Y-coordinate {m}",
        "    2.70,                    !- Vertex 2 Z-coordinate {m}",
        "    16.764,                  !- Vertex 3 X-coordinate {m}",
        "    0,                       !- Vertex 3 Y-coordinate {m}",
        "    2.70,                    !- Vertex 3 Z-coordinate {m}",
        "    16.764,                  !- Vertex 4 X-coordinate {m}",
        "    0,                       !- Vertex 4 Y-coordinate {m}",
        "    2.7254;                  !- Vertex 4 Z-coordinate {m}",

        "  BuildingSurface:Detailed,",
        "    West Wall,               !- Name",
        "    Wall,                    !- Surface Type",
        "    Walls,                   !- Construction Name",
        "    OCCUPIED ZONE,           !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50,                    !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,                       !- Vertex 1 X-coordinate {m}",
        "    8.5344,                  !- Vertex 1 Y-coordinate {m}",
        "    2.70,                    !- Vertex 1 Z-coordinate {m}",
        "    0,                       !- Vertex 2 X-coordinate {m}",
        "    8.5344,                  !- Vertex 2 Y-coordinate {m}",
        "    0,                       !- Vertex 2 Z-coordinate {m}",
        "    0,                       !- Vertex 3 X-coordinate {m}",
        "    0,                       !- Vertex 3 Y-coordinate {m}",
        "    0,                       !- Vertex 3 Z-coordinate {m}",
        "    0,                       !- Vertex 4 X-coordinate {m}",
        "    0,                       !- Vertex 4 Y-coordinate {m}",
        "    2.70;                    !- Vertex 4 Z-coordinate {m}",

        "  BuildingSurface:Detailed,",
        "    West Wall Attic,         !- Name",
        "    Wall,                    !- Surface Type",
        "    Gables,              !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.50,                    !- View Factor to Ground",
        "    5,                       !- Number of Vertices",
        "    0,                       !- Vertex 1 X-coordinate {m}",
        "    8.5344,                  !- Vertex 1 Y-coordinate {m}",
        "    2.7254,                  !- Vertex 1 Z-coordinate {m}",
        "    0,                       !- Vertex 2 X-coordinate {m}",
        "    8.5344,                  !- Vertex 2 Y-coordinate {m}",
        "    2.70,                    !- Vertex 2 Z-coordinate {m}",
        "    0,                       !- Vertex 3 X-coordinate {m}",
        "    0,                       !- Vertex 3 Y-coordinate {m}",
        "    2.70,                    !- Vertex 3 Z-coordinate {m}",
        "    0,                       !- Vertex 4 X-coordinate {m}",
        "    0,                       !- Vertex 4 Y-coordinate {m}",
        "    2.7254,                  !- Vertex 4 Z-coordinate {m}",
        "    0,                       !- Vertex 5 X-coordinate {m}",
        "    4.2672,                  !- Vertex 5 Y-coordinate {m}",
        "    4.5034;                  !- Vertex 5 Z-coordinate {m}",

        "  BuildingSurface:Detailed,",
        "    Zone Floor,              !- Name",
        "    Floor,                   !- Surface Type",
        "    LTFLOOR,                 !- Construction Name",
        "    OCCUPIED ZONE,           !- Zone Name",
        "    ,                        !- Space Name",
        "    Ground,                  !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,                       !- Vertex 1 X-coordinate {m}",
        "    0,                       !- Vertex 1 Y-coordinate {m}",
        "    0,                       !- Vertex 1 Z-coordinate {m}",
        "    0,                       !- Vertex 2 X-coordinate {m}",
        "    8.5344,                  !- Vertex 2 Y-coordinate {m}",
        "    0,                       !- Vertex 2 Z-coordinate {m}",
        "    16.764,                  !- Vertex 3 X-coordinate {m}",
        "    8.5344,                  !- Vertex 3 Y-coordinate {m}",
        "    0,                       !- Vertex 3 Z-coordinate {m}",
        "    16.764,                  !- Vertex 4 X-coordinate {m}",
        "    0,                       !- Vertex 4 Y-coordinate {m}",
        "    0;                       !- Vertex 4 Z-coordinate {m}",

        "  BuildingSurface:Detailed,",
        "    Zone Ceiling,            !- Name",
        "    Ceiling,                 !- Surface Type",
        "    CeilingConstruction,     !- Construction Name",
        "    OCCUPIED ZONE,           !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Attic Floor,             !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,                       !- Vertex 1 X-coordinate {m}",
        "    8.5344,                  !- Vertex 1 Y-coordinate {m}",
        "    2.70,                    !- Vertex 1 Z-coordinate {m}",
        "    0,                       !- Vertex 2 X-coordinate {m}",
        "    0,                       !- Vertex 2 Y-coordinate {m}",
        "    2.70,                    !- Vertex 2 Z-coordinate {m}",
        "    16.764,                  !- Vertex 3 X-coordinate {m}",
        "    0,                       !- Vertex 3 Y-coordinate {m}",
        "    2.70,                    !- Vertex 3 Z-coordinate {m}",
        "    16.764,                  !- Vertex 4 X-coordinate {m}",
        "    8.5344,                  !- Vertex 4 Y-coordinate {m}",
        "    2.70;                    !- Vertex 4 Z-coordinate {m}",

        "  BuildingSurface:Detailed,",
        "    Attic Floor,             !- Name",
        "    Floor,                   !- Surface Type",
        "    Reverse:CeilingConstruction,  !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Zone Ceiling,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    16.764,                  !- Vertex 1 X-coordinate {m}",
        "    8.5344,                  !- Vertex 1 Y-coordinate {m}",
        "    2.70,                    !- Vertex 1 Z-coordinate {m}",
        "    16.764,                  !- Vertex 2 X-coordinate {m}",
        "    0,                       !- Vertex 2 Y-coordinate {m}",
        "    2.70,                    !- Vertex 2 Z-coordinate {m}",
        "    0,                       !- Vertex 3 X-coordinate {m}",
        "    0,                       !- Vertex 3 Y-coordinate {m}",
        "    2.70,                    !- Vertex 3 Z-coordinate {m}",
        "    0,                       !- Vertex 4 X-coordinate {m}",
        "    8.5344,                  !- Vertex 4 Y-coordinate {m}",
        "    2.70;                    !- Vertex 4 Z-coordinate {m}",

        "  BuildingSurface:Detailed,",
        "    Attic Roof South,        !- Name",
        "    Roof,                    !- Surface Type",
        "    Roof,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,                       !- Vertex 1 X-coordinate {m}",
        "    4.2672,                  !- Vertex 1 Y-coordinate {m}",
        "    4.5034,                  !- Vertex 1 Z-coordinate {m}",
        "    0,                       !- Vertex 2 X-coordinate {m}",
        "    0,                       !- Vertex 2 Y-coordinate {m}",
        "    2.7254,                  !- Vertex 2 Z-coordinate {m}",
        "    16.764,                  !- Vertex 3 X-coordinate {m}",
        "    0,                       !- Vertex 3 Y-coordinate {m}",
        "    2.7254,                  !- Vertex 3 Z-coordinate {m}",
        "    16.764,                  !- Vertex 4 X-coordinate {m}",
        "    4.2672,                  !- Vertex 4 Y-coordinate {m}",
        "    4.5034;                  !- Vertex 4 Z-coordinate {m}",

        "  BuildingSurface:Detailed,",
        "    Attic Roof North,        !- Name",
        "    Roof,                    !- Surface Type",
        "    Roof,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    16.764,                  !- Vertex 1 X-coordinate {m}",
        "    4.2672,                  !- Vertex 1 Y-coordinate {m}",
        "    4.5034,                  !- Vertex 1 Z-coordinate {m}",
        "    16.764,                  !- Vertex 2 X-coordinate {m}",
        "    8.5344,                  !- Vertex 2 Y-coordinate {m}",
        "    2.7254,                  !- Vertex 2 Z-coordinate {m}",
        "    0,                       !- Vertex 3 X-coordinate {m}",
        "    8.5344,                  !- Vertex 3 Y-coordinate {m}",
        "    2.7254,                  !- Vertex 3 Z-coordinate {m}",
        "    0,                       !- Vertex 4 X-coordinate {m}",
        "    4.2672,                  !- Vertex 4 Y-coordinate {m}",
        "    4.5034;                  !- Vertex 4 Z-coordinate {m}",

        "  ZoneProperty:UserViewFactors:BySurfaceName,",
        "    ATTIC ZONE,              !- Zone Name",
        "    Attic Floor,		!=From Surface 1",
        "    Attic Floor,		!=To Surface 1",
        "    0.000000,",
        "    Attic Floor,		!=From Surface 1",
        "    Attic Roof South,		!=To Surface 2",
        "    0.476288,",
        "    Attic Floor,		!=From Surface 1",
        "    Attic Roof North,		!=To Surface 3",
        "    0.476288,",
        "    Attic Floor,		!=From Surface 1",
        "    East Wall Attic,		!=To Surface 4",
        "    0.023712,",
        "    Attic Floor,		!=From Surface 1",
        "    West Wall Attic,		!=To Surface 5",
        "    0.023712,",
        "    Attic Floor,		!=From Surface 1",
        "    North Wall Attic,		!=To Surface 6",
        "    0.000000,",
        "    Attic Floor,		!=From Surface 1",
        "    South Wall Attic,		!=To Surface 7",
        "    0.000000,",
        "    Attic Roof South,		!=From Surface 2",
        "    Attic Floor,		!=To Surface 1",
        "    0.879300,",
        "    Attic Roof South,		!=From Surface 2",
        "    Attic Roof South,		!=To Surface 2",
        "    0.000000,",
        "    Attic Roof South,		!=From Surface 2",
        "    Attic Roof North,		!=To Surface 3",
        "    0.067378,",
        "    Attic Roof South,		!=From Surface 2",
        "    East Wall Attic,		!=To Surface 4",
        "    0.026661,",
        "    Attic Roof South,		!=From Surface 2",
        "    West Wall Attic,		!=To Surface 5",
        "    0.026661,",
        "    Attic Roof South,		!=From Surface 2",
        "    North Wall Attic,		!=To Surface 6",
        "    0.000000,",
        "    Attic Roof South,		!=From Surface 2",
        "    South Wall Attic,		!=To Surface 7",
        "    0.000000,",
        "    Attic Roof North,		!=From Surface 3",
        "    Attic Floor,		!=To Surface 1",
        "    0.879300,",
        "    Attic Roof North,		!=From Surface 3",
        "    Attic Roof South,		!=To Surface 2",
        "    0.067378,",
        "    Attic Roof North,		!=From Surface 3",
        "    Attic Roof North,		!=To Surface 3",
        "    0.000000,",
        "    Attic Roof North,		!=From Surface 3",
        "    East Wall Attic,		!=To Surface 4",
        "    0.026661,",
        "    Attic Roof North,		!=From Surface 3",
        "    West Wall Attic,		!=To Surface 5",
        "    0.026661,",
        "    Attic Roof North,		!=From Surface 3",
        "    North Wall Attic,		!=To Surface 6",
        "    0.000000,",
        "    Attic Roof North,		!=From Surface 3",
        "    South Wall Attic,		!=To Surface 7",
        "    0.000000,",
        "    East Wall Attic,		!=From Surface 4",
        "    Attic Floor,		!=To Surface 1",
        "    0.447134,",
        "    East Wall Attic,		!=From Surface 4",
        "    Attic Roof South,		!=To Surface 2",
        "    0.272318,",
        "    East Wall Attic,		!=From Surface 4",
        "    Attic Roof North,		!=To Surface 3",
        "    0.272318,",
        "    East Wall Attic,		!=From Surface 4",
        "    East Wall Attic,		!=To Surface 4",
        "    0.000000,",
        "    East Wall Attic,		!=From Surface 4",
        "    West Wall Attic,		!=To Surface 5",
        "    0.008231,",
        "    East Wall Attic,		!=From Surface 4",
        "    North Wall Attic,		!=To Surface 6",
        "    0.000000,",
        "    East Wall Attic,		!=From Surface 4",
        "    South Wall Attic,		!=To Surface 7",
        "    0.000000,",
        "    West Wall Attic,		!=From Surface 5",
        "    Attic Floor,		!=To Surface 1",
        "    0.447134,",
        "    West Wall Attic,		!=From Surface 5",
        "    Attic Roof South,		!=To Surface 2",
        "    0.272318,",
        "    West Wall Attic,		!=From Surface 5",
        "    Attic Roof North,		!=To Surface 3",
        "    0.272318,",
        "    West Wall Attic,		!=From Surface 5",
        "    East Wall Attic,		!=To Surface 4",
        "    0.008231,",
        "    West Wall Attic,		!=From Surface 5",
        "    West Wall Attic,		!=To Surface 5",
        "    0.000000,",
        "    West Wall Attic,		!=From Surface 5",
        "    North Wall Attic,		!=To Surface 6",
        "    0.000000,",
        "    West Wall Attic,		!=From Surface 5",
        "    South Wall Attic,		!=To Surface 7",
        "    0.000000,",
        "    North Wall Attic,		!=From Surface 6",
        "    Attic Floor,		!=To Surface 1",
        "    0.000000,",
        "    North Wall Attic,		!=From Surface 6",
        "    Attic Roof South,		!=To Surface 2",
        "    0.000000,",
        "    North Wall Attic,		!=From Surface 6",
        "    Attic Roof North,		!=To Surface 3",
        "    0.000000,",
        "    North Wall Attic,		!=From Surface 6",
        "    East Wall Attic,		!=To Surface 4",
        "    0.000000,",
        "    North Wall Attic,		!=From Surface 6",
        "    West Wall Attic,		!=To Surface 5",
        "    0.000000,",
        "    North Wall Attic,		!=From Surface 6",
        "    North Wall Attic,		!=To Surface 6",
        "    1.000000,",
        "    North Wall Attic,		!=From Surface 6",
        "    South Wall Attic,		!=To Surface 7",
        "    0.000000,",
        "    South Wall Attic,		!=From Surface 7",
        "    Attic Floor,		!=To Surface 1",
        "    0.000000,",
        "    South Wall Attic,		!=From Surface 7",
        "    Attic Roof South,		!=To Surface 2",
        "    0.000000,",
        "    South Wall Attic,		!=From Surface 7",
        "    Attic Roof North,		!=To Surface 3",
        "    0.000000,",
        "    South Wall Attic,		!=From Surface 7",
        "    East Wall Attic,		!=To Surface 4",
        "    0.000000,",
        "    South Wall Attic,		!=From Surface 7",
        "    West Wall Attic,		!=To Surface 5",
        "    0.000000,",
        "    South Wall Attic,		!=From Surface 7",
        "    North Wall Attic,		!=To Surface 6",
        "    0.000000,",
        "    South Wall Attic,		!=From Surface 7",
        "    South Wall Attic,		!=To Surface 7",
        "    1.000000;",

        "  AirflowNetwork:SimulationControl,",
        "    House AirflowNetwork,    !- Name",
        "    MultizoneWithDistribution,  !- AirflowNetwork Control",
        "    SurfaceAverageCalculation,  !- Wind Pressure Coefficient Type",
        "    ,                        !- Height Selection for Local Wind Pressure Calculation",
        "    LOWRISE,                 !- Building Type",
        "    500,                     !- Maximum Number of Iterations {dimensionless}",
        "    ZeroNodePressures,       !- Initialization Type",
        "    1.0E-04,                 !- Relative Airflow Convergence Tolerance {dimensionless}",
        "    1.0E-06,                 !- Absolute Airflow Convergence Tolerance {kg/s}",
        "    -0.5,                    !- Convergence Acceleration Limit {dimensionless}",
        "    0.0,                     !- Azimuth Angle of Long Axis of Building {deg}",
        "    1.0;                     !- Ratio of Building Width Along Short Axis to Width Along Long Axis",

        "  AirflowNetwork:MultiZone:Zone,",
        "    OCCUPIED ZONE,           !- Zone Name",
        "    NOVENT,                  !- Ventilation Control Mode",
        "    ,                        !- Ventilation Control Zone Temperature Setpoint Schedule Name",
        "    1.0,                     !- Minimum Venting Open Factor {dimensionless}",
        "    0.0,                     !- Indoor and Outdoor Temperature Difference Lower Limit For Maximum Venting Open Factor {deltaC}",
        "    100.0,                   !- Indoor and Outdoor Temperature Difference Upper Limit for Minimum Venting Open Factor {deltaC}",
        "    0.0,                     !- Indoor and Outdoor Enthalpy Difference Lower Limit For Maximum Venting Open Factor {deltaJ/kg}",
        "    300000.0;                !- Indoor and Outdoor Enthalpy Difference Upper Limit for Minimum Venting Open Factor {deltaJ/kg}",

        "  AirflowNetwork:MultiZone:Zone,",
        "    ATTIC ZONE,              !- Zone Name",
        "    NoVent,                  !- Ventilation Control Mode",
        "    ,                        !- Ventilation Control Zone Temperature Setpoint Schedule Name",
        "    1.0,                     !- Minimum Venting Open Factor {dimensionless}",
        "    0.0,                     !- Indoor and Outdoor Temperature Difference Lower Limit For Maximum Venting Open Factor {deltaC}",
        "    100.0,                   !- Indoor and Outdoor Temperature Difference Upper Limit for Minimum Venting Open Factor {deltaC}",
        "    0.0,                     !- Indoor and Outdoor Enthalpy Difference Lower Limit For Maximum Venting Open Factor {deltaJ/kg}",
        "    300000.0;                !- Indoor and Outdoor Enthalpy Difference Upper Limit for Minimum Venting Open Factor {deltaJ/kg}",

        "  AirflowNetwork:MultiZone:Surface,",
        "    North Wall Attic,        !- Surface Name",
        "    NorthEaveLeak,           !- Leakage Component Name",
        "    ,                        !- External Node Name",
        "    1.0;                     !- Window/Door Opening Factor, or Crack Factor {dimensionless}",

        "  AirflowNetwork:MultiZone:Surface,",
        "    South Wall Attic,        !- Surface Name",
        "    SouthEaveLeak,           !- Leakage Component Name",
        "    ,                        !- External Node Name",
        "    1.0;                     !- Window/Door Opening Factor, or Crack Factor {dimensionless}",

        "  AirflowNetwork:MultiZone:Surface,",
        "    East Wall,               !- Surface Name",
        "    EastLeak,                !- Leakage Component Name",
        "    ,                        !- External Node Name",
        "    1.0;                     !- Window/Door Opening Factor, or Crack Factor {dimensionless}",

        "  AirflowNetwork:MultiZone:Surface,",
        "    West Wall,               !- Surface Name",
        "    WestLeak,                !- Leakage Component Name",
        "    ,                        !- External Node Name",
        "    1.0;                     !- Window/Door Opening Factor, or Crack Factor {dimensionless}",

        "  AirflowNetwork:MultiZone:ReferenceCrackConditions,",
        "    ReferenceCrackConditions,!- Name",
        "    20.0,                    !- Reference Temperature {C}",
        "    101325,                  !- Reference Barometric Pressure {Pa}",
        "    0.0;                     !- Reference Humidity Ratio {kgWater/kgDryAir}",

        "  AirflowNetwork:MultiZone:Surface:Crack,",
        "    NorthEaveLeak,           !- Name",
        "    0.2,                     !- Air Mass Flow Coefficient at Reference Conditions {kg/s}",
        "    0.65,                    !- Air Mass Flow Exponent {dimensionless}",
        "    ReferenceCrackConditions;!- Reference Crack Conditions",

        "  AirflowNetwork:MultiZone:Surface:Crack,",
        "    SouthEaveLeak,           !- Name",
        "    0.2,                     !- Air Mass Flow Coefficient at Reference Conditions {kg/s}",
        "    0.65,                    !- Air Mass Flow Exponent {dimensionless}",
        "    ReferenceCrackConditions;!- Reference Crack Conditions",

        "  AirflowNetwork:MultiZone:Surface:Crack,",
        "    EastLeak,                !- Name",
        "    0.05,                    !- Air Mass Flow Coefficient at Reference Conditions {kg/s}",
        "    0.65,                    !- Air Mass Flow Exponent {dimensionless}",
        "    ReferenceCrackConditions;!- Reference Crack Conditions",

        "  AirflowNetwork:MultiZone:Surface:Crack,",
        "    WestLeak,                !- Name",
        "    0.03,                    !- Air Mass Flow Coefficient at Reference Conditions {kg/s}",
        "    0.65,                    !- Air Mass Flow Exponent {dimensionless}",
        "    ReferenceCrackConditions;!- Reference Crack Conditions",

        "  AirflowNetwork:Distribution:Node,",
        "    EquipmentInletNode,      !- Name",
        "    Zone Equipment Inlet Node,  !- Component Name or Node Name",
        "    Other,                   !- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Node,",
        "    SplitterNode,            !- Name",
        "    ,                        !- Component Name or Node Name",
        "    AirLoopHVAC:ZoneSplitter,!- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Node,",
        "    ZoneSupplyNode,          !- Name",
        "    ,                        !- Component Name or Node Name",
        "    Other,                   !- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Node,",
        "    ZoneSupplyRegisterNode,  !- Name",
        "    Zone Inlet Node,         !- Component Name or Node Name",
        "    Other,                   !- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Node,",
        "    ZoneOutletNode,          !- Name",
        "    Zone Outlet Node,        !- Component Name or Node Name",
        "    Other,                   !- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Node,",
        "    ZoneReturnNode,          !- Name",
        "    ,                        !- Component Name or Node Name",
        "    Other,                   !- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Node,",
        "    MixerNode,               !- Name",
        "    ,                        !- Component Name or Node Name",
        "    AirLoopHVAC:ZoneMixer,   !- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Node,",
        "    MainReturnNode,          !- Name",
        "    Return Air Mixer Outlet, !- Component Name or Node Name",
        "    Other,                   !- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Node,",
        "    MainInletNode,           !- Name",
        "    ,                        !- Component Name or Node Name",
        "    Other,                   !- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Node,",
        "    FanInletNode,            !- Name",
        "    Air Loop Inlet Node,     !- Component Name or Node Name",
        "    Other,                   !- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Node,",
        "    FanOutletNode,           !- Name",
        "    Cooling Coil Air Inlet Node,  !- Component Name or Node Name",
        "    Other,                   !- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Node,",
        "    HeatingInletNode,        !- Name",
        "    Heating Coil Air Inlet Node,  !- Component Name or Node Name",
        "    Other,                   !- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Node,",
        "    HeatingOutletNode,       !- Name",
        "    Air Loop Outlet Node,    !- Component Name or Node Name",
        "    Other,                   !- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Component:Duct,",
        "    MainTrunk,               !- Name",
        "    2.0,                     !- Duct Length {m}",
        "    0.4064,                  !- Hydraulic Diameter {m}",
        "    0.1297,                  !- Cross Section Area {m2}",
        "    0.0009,                  !- Surface Roughness {m}",
        "    0.01,                    !- Coefficient for Local Dynamic Loss Due to Fitting {dimensionless}",
        "    0.7139,                  !- Heat Transmittance Coefficient (U-Factor) for Duct Construction {W/m2-K}",
        "    0.0000001;               !- Overall Moisture Transmittance Coefficient from Air to Air {kg/m2}",

        "  AirflowNetwork:Distribution:Component:Duct,",
        "    ZoneSupply,              !- Name",
        "    16.76,                   !- Duct Length {m}",
        "    0.3048,                  !- Hydraulic Diameter {m}",
        "    0.073205,                !- Cross Section Area {m2}",
        "    0.0009,                  !- Surface Roughness {m}",
        "    0.91,                    !- Coefficient for Local Dynamic Loss Due to Fitting {dimensionless}",
        "    0.00613207547169811,     !- Overall Heat Transmittance Coefficient (U-Factor) from Air to Air {W/m2-K}",
        "    0.0000001,               !- Overall Moisture Transmittance Coefficient from Air to Air {kg/m2}",
        "    0.0325,                  !- Outside Convection Coefficient {W/m2-K}",
        "    0.1625;                  !- Inside Convection Coefficient {W/m2-K}",

        "  AirflowNetwork:Distribution:Component:Duct,",
        "    ZoneReturn,              !- Name",
        "    3.0,                     !- Duct Length {m}",
        "    0.50,                    !- Hydraulic Diameter {m}",
        "    0.1963,                  !- Cross Section Area {m2}",
        "    0.0009,                  !- Surface Roughness {m}",
        "    0.01,                    !- Coefficient for Local Dynamic Loss Due to Fitting {dimensionless}",
        "    0.00122641509433962,     !- Overall Heat Transmittance Coefficient (U-Factor) from Air to Air {W/m2-K}",
        "    0.0000001,               !- Overall Moisture Transmittance Coefficient from Air to Air {kg/m2}",
        "    0.0065,                  !- Outside Convection Coefficient {W/m2-K}",
        "    0.0325;                  !- Inside Convection Coefficient {W/m2-K}",

        "  AirflowNetwork:Distribution:Component:Duct,",
        "    ZoneConnectionDuct,      !- Name",
        "    0.1,                     !- Duct Length {m}",
        "    1.0,                     !- Hydraulic Diameter {m}",
        "    0.7854,                  !- Cross Section Area {m2}",
        "    0.0001,                  !- Surface Roughness {m}",
        "    0.00,                    !- Coefficient for Local Dynamic Loss Due to Fitting {dimensionless}",
        "    0.00122641509433962,     !- Overall Heat Transmittance Coefficient (U-Factor) from Air to Air {W/m2-K}",
        "    0.0000001,               !- Overall Moisture Transmittance Coefficient from Air to Air {kg/m2}",
        "    0.0065,                  !- Outside Convection Coefficient {W/m2-K}",
        "    0.0325;                  !- Inside Convection Coefficient {W/m2-K}",

        "  AirflowNetwork:Distribution:Component:Duct,",
        "    MainReturn,              !- Name",
        "    1.0,                     !- Duct Length {m}",
        "    0.50,                    !- Hydraulic Diameter {m}",
        "    0.1963,                  !- Cross Section Area {m2}",
        "    0.0009,                  !- Surface Roughness {m}",
        "    0.01,                    !- Coefficient for Local Dynamic Loss Due to Fitting {dimensionless}",
        "    0.00122641509433962,     !- Overall Heat Transmittance Coefficient (U-Factor) from Air to Air {W/m2-K}",
        "    0.0000001,               !- Overall Moisture Transmittance Coefficient from Air to Air {kg/m2}",
        "    0.0065,                  !- Outside Convection Coefficient {W/m2-K}",
        "    0.0325;                  !- Inside Convection Coefficient {W/m2-K}",

        "  AirflowNetwork:Distribution:Component:Duct,",
        "    AirLoopReturn,           !- Name",
        "    0.1,                     !- Duct Length {m}",
        "    1.00,                    !- Hydraulic Diameter {m}",
        "    0.7854,                  !- Cross Section Area {m2}",
        "    0.0001,                  !- Surface Roughness {m}",
        "    0.00,                    !- Coefficient for Local Dynamic Loss Due to Fitting {dimensionless}",
        "    0.00122641509433962,     !- Overall Heat Transmittance Coefficient (U-Factor) from Air to Air {W/m2-K}",
        "    0.0000001,               !- Overall Moisture Transmittance Coefficient from Air to Air {kg/m2}",
        "    0.0065,                  !- Outside Convection Coefficient {W/m2-K}",
        "    0.0325;                  !- Inside Convection Coefficient {W/m2-K}",

        "  AirflowNetwork:Distribution:Component:Duct,",
        "    AirLoopSupply,           !- Name",
        "    0.1,                     !- Duct Length {m}",
        "    1.00,                    !- Hydraulic Diameter {m}",
        "    0.7854,                  !- Cross Section Area {m2}",
        "    0.0001,                  !- Surface Roughness {m}",
        "    0.00,                    !- Coefficient for Local Dynamic Loss Due to Fitting {dimensionless}",
        "    0.00122641509433962,     !- Overall Heat Transmittance Coefficient (U-Factor) from Air to Air {W/m2-K}",
        "    0.0000001,               !- Overall Moisture Transmittance Coefficient from Air to Air {kg/m2}",
        "    0.0065,                  !- Outside Convection Coefficient {W/m2-K}",
        "    0.0325;                  !- Inside Convection Coefficient {W/m2-K}",

        "  AirflowNetwork:Distribution:Component:Fan,",
        "    Supply Fan 1,            !- Fan Name",
        "    Fan:ConstantVolume;      !- Supply Fan Object Type",

        "  AirflowNetwork:Distribution:Component:Coil,",
        "    ACDXCoil 1,              !- Coil Name",
        "    Coil:Cooling:DX:SingleSpeed,  !- Coil Object Type",
        "    0.1,                     !- Air Path Length {m}",
        "    1.000;                   !- Air Path Hydraulic Diameter {m}",

        "  AirflowNetwork:Distribution:Component:Coil,",
        "    Main Heating Coil 1,     !- Coil Name",
        "    Coil:Heating:Fuel,       !- Coil Object Type",
        "    0.1,                     !- Air Path Length {m}",
        "    1.000;                   !- Air Path Hydraulic Diameter {m}",

        "  AirflowNetwork:Distribution:DuctViewFactors,",
        "    ZoneSupplyLink1,         !- Name of Linkage",
        "    1.0,                     !- Surface Exposure Fraction",
        "    0.9,                     !- Duct surface emittance",
        "    Attic Floor,             !- Surface 1",
        "    0.483577,                !- View Factor for Surface 1",
        "    Attic Roof North,        !- Surface 2",
        "    0.237692,                !- View Factor for Surface 2",
        "    Attic Roof South,        !- Surface 3",
        "    0.237692,                !- View Factor for Surface 3",
        "    East Wall Attic,         !- Surface 4",
        "    0.02052,                 !- View Factor for Surface 4",
        "    West Wall Attic,         !- Surface 5",
        "    0.02052;                 !- View Factor for Surface 5",

        "  AirflowNetwork:Distribution:Linkage,",
        "    Main Link,               !- Name",
        "    EquipmentInletNode,      !- Node 1 Name",
        "    SplitterNode,            !- Node 2 Name",
        "    MainTrunk,               !- Component Name",
        "    Attic Zone;              !- Thermal Zone Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    ZoneSupplyLink1,         !- Name",
        "    SplitterNode,            !- Node 1 Name",
        "    ZoneSupplyNode,          !- Node 2 Name",
        "    ZoneSupply,              !- Component Name",
        "    Attic Zone;              !- Thermal Zone Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    ZoneSupply1Link2,        !- Name",
        "    ZoneSupplyNode,          !- Node 1 Name",
        "    ZoneSupplyRegisterNode,  !- Node 2 Name",
        "    ZoneSupply,              !- Component Name",
        "    Attic Zone;              !- Thermal Zone Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    ZoneSupplyConnectionLink,!- Name",
        "    ZoneSupplyRegisterNode,  !- Node 1 Name",
        "    OCCUPIED ZONE,           !- Node 2 Name",
        "    ZoneConnectionDuct;      !- Component Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    ZoneReturnConnectionLink,!- Name",
        "    OCCUPIED ZONE,           !- Node 1 Name",
        "    ZoneOutletNode,          !- Node 2 Name",
        "    ZoneConnectionDuct;      !- Component Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    ZoneReturn1Link,         !- Name",
        "    ZoneOutletNode,          !- Node 1 Name",
        "    ZoneReturnNode,          !- Node 2 Name",
        "    ZoneReturn,              !- Component Name",
        "    Attic Zone;              !- Thermal Zone Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    ZoneReturn2Link,         !- Name",
        "    ZoneReturnNode,          !- Node 1 Name",
        "    MixerNode,               !- Node 2 Name",
        "    ZoneReturn,              !- Component Name",
        "    OCCUPIED Zone;           !- Thermal Zone Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    ReturnMixerLink,         !- Name",
        "    MixerNode,               !- Node 1 Name",
        "    MainReturnNode,          !- Node 2 Name",
        "    MainReturn,              !- Component Name",
        "    OCCUPIED Zone;           !- Thermal Zone Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    SystemReturnLink,        !- Name",
        "    MainReturnNode,          !- Node 1 Name",
        "    MainInletNode,           !- Node 2 Name",
        "    AirLoopReturn;           !- Component Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    SystemInletLink,         !- Name",
        "    MainInletNode,           !- Node 1 Name",
        "    FanInletNode,            !- Node 2 Name",
        "    MainReturn,              !- Component Name",
        "    OCCUPIED ZONE;           !- Thermal Zone Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    SupplyFanLink,           !- Name",
        "    FanInletNode,            !- Node 1 Name",
        "    FanOutletNode,           !- Node 2 Name",
        "    Supply Fan 1;            !- Component Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    CoolingCoilLink,         !- Name",
        "    FanOutletNode,           !- Node 1 Name",
        "    HeatingInletNode,        !- Node 2 Name",
        "    ACDXCoil 1;              !- Component Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    HeatingCoilLink,         !- Name",
        "    HeatingInletNode,        !- Node 1 Name",
        "    HeatingOutletNode,       !- Node 2 Name",
        "    Main Heating Coil 1;     !- Component Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    EquipmentAirLoopLink,    !- Name",
        "    HeatingOutletNode,       !- Node 1 Name",
        "    EquipmentInletNode,      !- Node 2 Name",
        "    AirLoopSupply;           !- Component Name",

        "  ZoneControl:Thermostat,",
        "    Zone Thermostat,         !- Name",
        "    OCCUPIED ZONE,           !- Zone or ZoneList Name",
        "    Dual Zone Control Type Sched,  !- Control Type Schedule Name",
        "    ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        "    Setpoints;               !- Control 1 Name",

        "  ThermostatSetpoint:DualSetpoint,",
        "    Setpoints,               !- Name",
        "    Dual Heating Setpoints,  !- Heating Setpoint Temperature Schedule Name",
        "    Dual Cooling Setpoints;  !- Cooling Setpoint Temperature Schedule Name",

        "  ZoneHVAC:AirDistributionUnit,",
        "    ZoneDirectAirADU,        !- Name",
        "    Zone1NoReheatAirOutletNode,  !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:ConstantVolume:NoReheat,  !- Air Terminal Object Type",
        "    ZoneDirectAir;           !- Air Terminal Name",

        "  AirTerminal:SingleDuct:ConstantVolume:NoReheat,",
        "    ZoneDirectAir,           !- Name",
        "    HVACAvailSched,          !- Availability Schedule Name",
        "    Zone Inlet Node 2AT,     !- Air Inlet Node Name",
        "    Zone Inlet Node,         !- Zone Supply Air Node Name",
        "    2.36;                    !- Maximum Air Flow Rate {m3/s}",

        "  ZoneHVAC:EquipmentList,",
        "    ZoneEquipment,           !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    ZoneDirectAirADU,        !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Fraction",
        "    ;                        !- Zone Equipment 1 Sequential Heating Fraction",

        "  ZoneHVAC:EquipmentConnections,",
        "    OCCUPIED ZONE,           !- Zone Name",
        "    ZoneEquipment,           !- Zone Conditioning Equipment List Name",
        "    ZoneInlets,              !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    Zone Node,               !- Zone Air Node Name",
        "    Zone Outlet Node;        !- Zone Return Air Node Name",

        "  Fan:ConstantVolume,",
        "    Supply Fan 1,            !- Name",
        "    HVACAvailSched,          !- Availability Schedule Name",
        "    0.7,                     !- Fan Total Efficiency",
        "    400.0,                   !- Pressure Rise {Pa}",
        "    2.36,                    !- Maximum Flow Rate {m3/s}",
        "    0.9,                     !- Motor Efficiency",
        "    1.0,                     !- Motor In Airstream Fraction",
        "    Air Loop Inlet Node,     !- Air Inlet Node Name",
        "    Cooling Coil Air Inlet Node;  !- Air Outlet Node Name",

        "  Coil:Cooling:DX:SingleSpeed,",
        "    ACDXCoil 1,              !- Name",
        "    HVACAvailSched,          !- Availability Schedule Name",
        "    21000,                   !- Gross Rated Total Cooling Capacity {W}",
        "    0.8,                     !- Gross Rated Sensible Heat Ratio",
        "    3.0,                     !- Gross Rated Cooling COP {W/W}",
        "    2.36,                    !- Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    Cooling Coil Air Inlet Node,  !- Air Inlet Node Name",
        "    Heating Coil Air Inlet Node,  !- Air Outlet Node Name",
        "    WindACCoolCapFT,         !- Total Cooling Capacity Function of Temperature Curve Name",
        "    WindACCoolCapFFF,        !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    WindACEIRFT,             !- Energy Input Ratio Function of Temperature Curve Name",
        "    WindACEIRFFF,            !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    WindACPLFFPLR;           !- Part Load Fraction Correlation Curve Name",

        "  Coil:Heating:Fuel,",
        "    Main Heating Coil 1,     !- Name",
        "    HVACAvailSched,          !- Availability Schedule Name",
        "    NaturalGas,              !- Fuel Type",
        "    0.8,                     !- Burner Efficiency",
        "    8000000,                 !- Nominal Capacity {W}",
        "    Heating Coil Air Inlet Node,  !- Air Inlet Node Name",
        "    Air Loop Outlet Node,    !- Air Outlet Node Name",
        "    Air Loop Outlet Node;    !- Temperature Setpoint Node Name",

        "  CoilSystem:Cooling:DX,",
        "    DX Cooling Coil System 1,!- Name",
        "    HVACAvailSched,          !- Availability Schedule Name",
        "    Cooling Coil Air Inlet Node,  !- DX Cooling Coil System Inlet Node Name",
        "    Heating Coil Air Inlet Node,  !- DX Cooling Coil System Outlet Node Name",
        "    Heating Coil Air Inlet Node,  !- DX Cooling Coil System Sensor Node Name",
        "    Coil:Cooling:DX:SingleSpeed,  !- Cooling Coil Object Type",
        "    ACDXCoil 1;              !- Cooling Coil Name",

        "  AirLoopHVAC,",
        "    Typical Residential System,  !- Name",
        "    ,                        !- Controller List Name",
        "    Reheat System 1 Avail List,  !- Availability Manager List Name",
        "    2.36,                    !- Design Supply Air Flow Rate {m3/s}",
        "    Air Loop Branches,       !- Branch List Name",
        "    ,                        !- Connector List Name",
        "    Air Loop Inlet Node,     !- Supply Side Inlet Node Name",
        "    Return Air Mixer Outlet, !- Demand Side Outlet Node Name",
        "    Zone Equipment Inlet Node,  !- Demand Side Inlet Node Names",
        "    Air Loop Outlet Node;    !- Supply Side Outlet Node Names",

        "  AirLoopHVAC:ZoneSplitter,",
        "    Zone Supply Air Splitter,!- Name",
        "    Zone Equipment Inlet Node,  !- Inlet Node Name",
        "    Zone Inlet Node 2AT;      !- Outlet 1 Node Name",

        "  AirLoopHVAC:SupplyPath,",
        "    TermReheatSupplyPath,    !- Name",
        "    Zone Equipment Inlet Node,  !- Supply Air Path Inlet Node Name",
        "    AirLoopHVAC:ZoneSplitter,!- Component 1 Object Type",
        "    Zone Supply Air Splitter;!- Component 1 Name",

        "  AirLoopHVAC:ZoneMixer,",
        "    Zone Return Air Mixer,   !- Name",
        "    Return Air Mixer Outlet, !- Outlet Node Name",
        "    Zone Outlet Node;        !- Inlet 1 Node Name",

        "  AirLoopHVAC:ReturnPath,",
        "    TermReheatReturnPath,    !- Name",
        "    Return Air Mixer Outlet, !- Return Air Path Outlet Node Name",
        "    AirLoopHVAC:ZoneMixer,   !- Component 1 Object Type",
        "    Zone Return Air Mixer;   !- Component 1 Name",

        "  Branch,",
        "    Air Loop Main Branch,    !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Fan:ConstantVolume,      !- Component 1 Object Type",
        "    Supply Fan 1,            !- Component 1 Name",
        "    Air Loop Inlet Node,     !- Component 1 Inlet Node Name",
        "    Cooling Coil Air Inlet Node,  !- Component 1 Outlet Node Name",
        "    CoilSystem:Cooling:DX,   !- Component 2 Object Type",
        "    DX Cooling Coil System 1,!- Component 2 Name",
        "    Cooling Coil Air Inlet Node,  !- Component 2 Inlet Node Name",
        "    Heating Coil Air Inlet Node,  !- Component 2 Outlet Node Name",
        "    Coil:Heating:Fuel,       !- Component 3 Object Type",
        "    Main Heating Coil 1,     !- Component 3 Name",
        "    Heating Coil Air Inlet Node,  !- Component 3 Inlet Node Name",
        "    Air Loop Outlet Node;    !- Component 3 Outlet Node Name",

        "  BranchList,",
        "    Air Loop Branches,       !- Name",
        "    Air Loop Main Branch;    !- Branch 1 Name",

        "  NodeList,",
        "    ZoneInlets,              !- Name",
        "    Zone Inlet Node;         !- Node 1 Name",

        "  NodeList,",
        "    Supply Air Temp Nodes,   !- Name",
        "    Heating Coil Air Inlet Node,  !- Node 1 Name",
        "    Air Loop Outlet Node;    !- Node 2 Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    // Read objects
    HeatBalanceManager::GetHeatBalanceInput(*state);
    HeatBalanceManager::AllocateHeatBalArrays(*state);
    state->dataEnvrn->OutBaroPress = 101000;
    state->dataHVACGlobal->TimeStepSys = state->dataGlobal->TimeStepZone;
    state->dataHVACGlobal->TimeStepSysSec = state->dataHVACGlobal->TimeStepSys * Constant::rSecsInHour;

    // Read AirflowNetwork inputs
    state->afn->get_input();
    state->afn->initialize();

    // Check inputs
    EXPECT_EQ(state->afn->AirflowNetworkLinkageViewFactorData(1).LinkageName, "ZONESUPPLYLINK1");
    EXPECT_EQ(state->afn->AirflowNetworkLinkageViewFactorData(1).DuctExposureFraction, 1.0);
    EXPECT_EQ(state->afn->AirflowNetworkLinkageViewFactorData(1).DuctEmittance, 0.9);
    EXPECT_EQ(state->afn->AirflowNetworkLinkageViewFactorData(1).LinkageSurfaceData(1).SurfaceName, "ATTIC FLOOR");
    EXPECT_EQ(state->afn->AirflowNetworkLinkageViewFactorData(1).LinkageSurfaceData(1).ViewFactor, 0.483577);
    EXPECT_EQ(state->afn->AirflowNetworkLinkageViewFactorData(1).LinkageSurfaceData(2).SurfaceName, "ATTIC ROOF NORTH");
    EXPECT_EQ(state->afn->AirflowNetworkLinkageViewFactorData(1).LinkageSurfaceData(2).ViewFactor, 0.237692);
    EXPECT_EQ(state->afn->AirflowNetworkLinkageViewFactorData(1).LinkageSurfaceData(3).SurfaceName, "ATTIC ROOF SOUTH");
    EXPECT_EQ(state->afn->AirflowNetworkLinkageViewFactorData(1).LinkageSurfaceData(3).ViewFactor, 0.237692);
    EXPECT_EQ(state->afn->AirflowNetworkLinkageViewFactorData(1).LinkageSurfaceData(4).SurfaceName, "EAST WALL ATTIC");
    EXPECT_EQ(state->afn->AirflowNetworkLinkageViewFactorData(1).LinkageSurfaceData(4).ViewFactor, 0.02052);
    EXPECT_EQ(state->afn->AirflowNetworkLinkageViewFactorData(1).LinkageSurfaceData(5).SurfaceName, "WEST WALL ATTIC");
    EXPECT_EQ(state->afn->AirflowNetworkLinkageViewFactorData(1).LinkageSurfaceData(5).ViewFactor, 0.02052);

    Real64 constexpr tol = 0.01;

    // Outside convection coefficients
    // Calculate convection resistance given a convection coefficient
    EXPECT_NEAR(state->afn->duct_outside_convection_resistance(20, 10, 0.001, 101000, 1, 2, 5), 0.2, tol);
    EXPECT_NEAR(state->afn->duct_outside_convection_resistance(20, 10, 0.001, 101000, 1, 2, 20), 0.05, tol);
    EXPECT_NEAR(state->afn->duct_outside_convection_resistance(20, 10, 0.001, 101000, 1, 2, 0.1), 10, tol);

    //// Calculate convection resistance from correlation
    EXPECT_NEAR(state->afn->duct_outside_convection_resistance(20, 10, 0.001, 101000, 0.1, 2, 0), 0.2297, tol);
    EXPECT_NEAR(state->afn->duct_outside_convection_resistance(20, 10, 0.001, 101000, 1.0, 2, 0), 0.4093, tol);
    EXPECT_NEAR(state->afn->duct_outside_convection_resistance(20, 10, 0.001, 101000, 1.5, 2, 0), 0.4531, tol);

    EXPECT_NEAR(state->afn->duct_outside_convection_resistance(10, 20, 0.001, 101000, 0.1, 2, 0), 0.2368, tol);
    EXPECT_NEAR(state->afn->duct_outside_convection_resistance(10, 20, 0.001, 101000, 1.0, 2, 0), 0.4218, tol);
    EXPECT_NEAR(state->afn->duct_outside_convection_resistance(10, 20, 0.001, 101000, 1.5, 2, 0), 0.4670, tol);

    // Calculate convection resistance given a convection coefficient
    EXPECT_NEAR(state->afn->duct_inside_convection_resistance(20, 0.1, 1, 5), 0.2, tol);
    EXPECT_NEAR(state->afn->duct_inside_convection_resistance(20, 0.1, 1, 20), 0.05, tol);
    EXPECT_NEAR(state->afn->duct_inside_convection_resistance(20, 0.1, 1, 0.1), 10, tol);

    // Calculate convection resistance from correlation
    EXPECT_NEAR(state->afn->duct_inside_convection_resistance(20, 0.1, 1, 0), 1.611, tol);
    EXPECT_NEAR(state->afn->duct_inside_convection_resistance(20, 1.0, 1, 0), 0.2554, tol);
    EXPECT_NEAR(state->afn->duct_inside_convection_resistance(40, 0.1, 1, 0), 1.5879, tol);
    EXPECT_NEAR(state->afn->duct_inside_convection_resistance(40, 1.0, 1, 0), 0.2516, tol);
}

TEST_F(EnergyPlusFixture, AirflowNetwork_TestPolygonalWindows)
{

    // Unit test for a new feature

    state->dataHeatBal->Zone.allocate(1);
    state->dataHeatBal->Zone(1).Name = "ZONE 1";

    state->dataSurface->Surface.allocate(14);
    // Rectangular base surface
    state->dataSurface->Surface(1).Name = "LIVING:NORTH";
    state->dataSurface->Surface(1).Zone = 1;
    state->dataSurface->Surface(1).ZoneName = "ZONE 1";
    state->dataSurface->Surface(1).Azimuth = 180.0;
    state->dataSurface->Surface(1).ExtBoundCond = 0;
    state->dataSurface->Surface(1).HeatTransSurf = true;
    state->dataSurface->Surface(1).Tilt = 90.0;
    state->dataSurface->Surface(1).Sides = 4;
    state->dataSurface->Surface(1).Area = 25.17;
    state->dataSurface->Surface(1).Vertex.allocate(4);
    state->dataSurface->Surface(1).Vertex(1).x = 10.323;
    state->dataSurface->Surface(1).Vertex(2).x = 10.323;
    state->dataSurface->Surface(1).Vertex(3).x = 0.0;
    state->dataSurface->Surface(1).Vertex(4).x = 0.0;
    state->dataSurface->Surface(1).Vertex(1).y = 10.778;
    state->dataSurface->Surface(1).Vertex(2).y = 10.778;
    state->dataSurface->Surface(1).Vertex(3).y = 10.778;
    state->dataSurface->Surface(1).Vertex(4).y = 10.778;
    state->dataSurface->Surface(1).Vertex(1).z = 2.4384;
    state->dataSurface->Surface(1).Vertex(2).z = 0.0;
    state->dataSurface->Surface(1).Vertex(3).z = 0.0;
    state->dataSurface->Surface(1).Vertex(4).z = 2.4384;

    // Rectangular base surface
    state->dataSurface->Surface(2).Name = "LIVING:SOUTH";
    state->dataSurface->Surface(2).Zone = 1;
    state->dataSurface->Surface(2).ZoneName = "ZONE 1";
    state->dataSurface->Surface(2).Azimuth = 0.0;
    state->dataSurface->Surface(2).ExtBoundCond = 0;
    state->dataSurface->Surface(2).HeatTransSurf = true;
    state->dataSurface->Surface(2).Tilt = 90.0;
    state->dataSurface->Surface(2).Sides = 4;
    state->dataSurface->Surface(2).Width = 10.323;
    state->dataSurface->Surface(2).Height = 2.4384;
    state->dataSurface->Surface(2).Area = 25.17;
    state->dataSurface->Surface(2).Vertex.allocate(4);
    state->dataSurface->Surface(2).Vertex(1).x = 10.323;
    state->dataSurface->Surface(2).Vertex(2).x = 10.323;
    state->dataSurface->Surface(2).Vertex(3).x = 0.0;
    state->dataSurface->Surface(2).Vertex(4).x = 0.0;
    state->dataSurface->Surface(2).Vertex(1).y = 0.0;
    state->dataSurface->Surface(2).Vertex(2).y = 0.0;
    state->dataSurface->Surface(2).Vertex(3).y = 0.0;
    state->dataSurface->Surface(2).Vertex(4).y = 0.0;
    state->dataSurface->Surface(2).Vertex(1).z = 2.4384;
    state->dataSurface->Surface(2).Vertex(2).z = 0.0;
    state->dataSurface->Surface(2).Vertex(3).z = 0.0;
    state->dataSurface->Surface(2).Vertex(4).z = 2.4384;

    // Polygonal base surface
    state->dataSurface->Surface(3).Name = "LIVING:EAST";
    state->dataSurface->Surface(3).Zone = 1;
    state->dataSurface->Surface(3).ZoneName = "ZONE 1";
    state->dataSurface->Surface(3).Azimuth = 90.0;
    state->dataSurface->Surface(3).ExtBoundCond = 0;
    state->dataSurface->Surface(3).HeatTransSurf = true;
    state->dataSurface->Surface(3).Tilt = 90.0;
    state->dataSurface->Surface(3).Sides = 5;
    state->dataSurface->Surface(3).Area = 25.17;
    state->dataSurface->Surface(3).Vertex.allocate(5);
    state->dataSurface->Surface(3).Vertex(1).x = 10.0;
    state->dataSurface->Surface(3).Vertex(2).x = 10.0;
    state->dataSurface->Surface(3).Vertex(3).x = 10.0;
    state->dataSurface->Surface(3).Vertex(4).x = 10.0;
    state->dataSurface->Surface(3).Vertex(5).x = 10.0;
    state->dataSurface->Surface(3).Vertex(1).y = 0.0;
    state->dataSurface->Surface(3).Vertex(2).y = 0.0;
    state->dataSurface->Surface(3).Vertex(3).y = 10.0;
    state->dataSurface->Surface(3).Vertex(4).y = 10.0;
    state->dataSurface->Surface(3).Vertex(5).y = 5.0;
    state->dataSurface->Surface(3).Vertex(1).z = 2.0;
    state->dataSurface->Surface(3).Vertex(2).z = 0.0;
    state->dataSurface->Surface(3).Vertex(3).z = 0.0;
    state->dataSurface->Surface(3).Vertex(4).z = 2.0;
    state->dataSurface->Surface(3).Vertex(5).z = 2.5;

    // Triangular window sub surface
    state->dataSurface->Surface(4).Name = "NORTH:WINDOW1";
    state->dataSurface->Surface(4).Zone = 1;
    state->dataSurface->Surface(4).ZoneName = "ZONE 1";
    state->dataSurface->Surface(4).Azimuth = 180.0;
    state->dataSurface->Surface(4).ExtBoundCond = 0;
    state->dataSurface->Surface(4).HeatTransSurf = true;
    state->dataSurface->Surface(4).Tilt = 90.0;
    state->dataSurface->Surface(4).Sides = 3;
    state->dataSurface->Surface(4).Area = 1.0;
    state->dataSurface->Surface(4).BaseSurf = 1;
    state->dataSurface->Surface(4).Vertex.allocate(3);
    state->dataSurface->Surface(4).Vertex(1).x = 3.0;
    state->dataSurface->Surface(4).Vertex(2).x = 3.0;
    state->dataSurface->Surface(4).Vertex(3).x = 1.0;
    state->dataSurface->Surface(4).Vertex(1).y = 10.778;
    state->dataSurface->Surface(4).Vertex(2).y = 10.778;
    state->dataSurface->Surface(4).Vertex(3).y = 10.778;
    state->dataSurface->Surface(4).Vertex(1).z = 2.0;
    state->dataSurface->Surface(4).Vertex(2).z = 1.0;
    state->dataSurface->Surface(4).Vertex(3).z = 1.0;

    // Polygonal window sub surface
    state->dataSurface->Surface(5).Name = "NORTH:WINDOW2";
    state->dataSurface->Surface(5).Zone = 1;
    state->dataSurface->Surface(5).ZoneName = "ZONE 1";
    state->dataSurface->Surface(5).Azimuth = 180.0;
    state->dataSurface->Surface(5).ExtBoundCond = 0;
    state->dataSurface->Surface(5).HeatTransSurf = true;
    state->dataSurface->Surface(5).Tilt = 90.0;
    state->dataSurface->Surface(5).Sides = 5;
    state->dataSurface->Surface(5).Area = 2.5;
    state->dataSurface->Surface(5).BaseSurf = 1;
    state->dataSurface->Surface(5).Vertex.allocate(5);
    state->dataSurface->Surface(5).Vertex(1).x = 5.0;
    state->dataSurface->Surface(5).Vertex(2).x = 5.0;
    state->dataSurface->Surface(5).Vertex(3).x = 3.0;
    state->dataSurface->Surface(5).Vertex(4).x = 3.0;
    state->dataSurface->Surface(5).Vertex(5).x = 4.0;
    state->dataSurface->Surface(5).Vertex(1).y = 10.778;
    state->dataSurface->Surface(5).Vertex(2).y = 10.778;
    state->dataSurface->Surface(5).Vertex(3).y = 10.778;
    state->dataSurface->Surface(5).Vertex(4).y = 10.778;
    state->dataSurface->Surface(5).Vertex(5).y = 10.778;
    state->dataSurface->Surface(5).Vertex(1).z = 2.0;
    state->dataSurface->Surface(5).Vertex(2).z = 1.0;
    state->dataSurface->Surface(5).Vertex(3).z = 1.0;
    state->dataSurface->Surface(5).Vertex(4).z = 2.0;
    state->dataSurface->Surface(5).Vertex(5).z = 2.5;

    // Triangular window sub surface
    state->dataSurface->Surface(6).Name = "SOUTH:WINDOW1";
    state->dataSurface->Surface(6).Zone = 1;
    state->dataSurface->Surface(6).ZoneName = "ZONE 1";
    state->dataSurface->Surface(6).Azimuth = 0.0;
    state->dataSurface->Surface(6).ExtBoundCond = 0;
    state->dataSurface->Surface(6).HeatTransSurf = true;
    state->dataSurface->Surface(6).Tilt = 90.0;
    state->dataSurface->Surface(6).Sides = 3;
    state->dataSurface->Surface(6).Area = 0.5;
    state->dataSurface->Surface(6).BaseSurf = 2;
    state->dataSurface->Surface(6).Vertex.allocate(3);
    state->dataSurface->Surface(6).Vertex(1).x = 9.0;
    state->dataSurface->Surface(6).Vertex(2).x = 9.0;
    state->dataSurface->Surface(6).Vertex(3).x = 8.0;
    state->dataSurface->Surface(6).Vertex(1).y = 0.0;
    state->dataSurface->Surface(6).Vertex(2).y = 0.0;
    state->dataSurface->Surface(6).Vertex(3).y = 0.0;
    state->dataSurface->Surface(6).Vertex(1).z = 2.0;
    state->dataSurface->Surface(6).Vertex(2).z = 1.0;
    state->dataSurface->Surface(6).Vertex(3).z = 1.0;

    // Triangular window sub surface
    state->dataSurface->Surface(9).Name = "SOUTH:WINDOW2";
    state->dataSurface->Surface(9).Zone = 1;
    state->dataSurface->Surface(9).ZoneName = "ZONE 1";
    state->dataSurface->Surface(9).Azimuth = 0.0;
    state->dataSurface->Surface(9).ExtBoundCond = 0;
    state->dataSurface->Surface(9).HeatTransSurf = true;
    state->dataSurface->Surface(9).Tilt = 90.0;
    state->dataSurface->Surface(9).Sides = 3;
    state->dataSurface->Surface(9).Area = 0.5;
    state->dataSurface->Surface(9).BaseSurf = 2;
    state->dataSurface->Surface(9).Vertex.allocate(3);
    state->dataSurface->Surface(9).Vertex(1).x = 7.0;
    state->dataSurface->Surface(9).Vertex(2).x = 7.0;
    state->dataSurface->Surface(9).Vertex(3).x = 6.0;
    state->dataSurface->Surface(9).Vertex(1).y = 0.0;
    state->dataSurface->Surface(9).Vertex(2).y = 0.0;
    state->dataSurface->Surface(9).Vertex(3).y = 0.0;
    state->dataSurface->Surface(9).Vertex(1).z = 2.0;
    state->dataSurface->Surface(9).Vertex(2).z = 1.0;
    state->dataSurface->Surface(9).Vertex(3).z = 1.0;

    state->dataSurface->Surface(10).Name = "EAST:WINDOW1";
    state->dataSurface->Surface(10).Zone = 1;
    state->dataSurface->Surface(10).ZoneName = "ZONE 1";
    state->dataSurface->Surface(10).Azimuth = 90.0;
    state->dataSurface->Surface(10).ExtBoundCond = 0;
    state->dataSurface->Surface(10).HeatTransSurf = true;
    state->dataSurface->Surface(10).Tilt = 90.0;
    state->dataSurface->Surface(10).Sides = 3;
    state->dataSurface->Surface(10).Area = 0.5;
    state->dataSurface->Surface(10).BaseSurf = 3;
    state->dataSurface->Surface(10).Vertex.allocate(3);
    state->dataSurface->Surface(10).Vertex(1).x = 10.0;
    state->dataSurface->Surface(10).Vertex(2).x = 10.0;
    state->dataSurface->Surface(10).Vertex(3).x = 10.0;
    state->dataSurface->Surface(10).Vertex(1).y = 1.0;
    state->dataSurface->Surface(10).Vertex(2).y = 1.0;
    state->dataSurface->Surface(10).Vertex(3).y = 2.0;
    state->dataSurface->Surface(10).Vertex(1).z = 2.0;
    state->dataSurface->Surface(10).Vertex(2).z = 1.0;
    state->dataSurface->Surface(10).Vertex(3).z = 1.0;

    // Polygonal horizontal base surface
    state->dataSurface->Surface(7).Name = "ROOF-POLY";
    state->dataSurface->Surface(7).Zone = 1;
    state->dataSurface->Surface(7).ZoneName = "ZONE 1";
    state->dataSurface->Surface(7).Azimuth = 0.0;
    state->dataSurface->Surface(7).ExtBoundCond = 0;
    state->dataSurface->Surface(7).HeatTransSurf = true;
    state->dataSurface->Surface(7).Tilt = 0.0;
    state->dataSurface->Surface(7).Sides = 5;
    state->dataSurface->Surface(7).Area = 55.0;
    state->dataSurface->Surface(7).Vertex.allocate(5);
    state->dataSurface->Surface(7).Vertex(1).x = 0.0;
    state->dataSurface->Surface(7).Vertex(2).x = 0.0;
    state->dataSurface->Surface(7).Vertex(3).x = 10.0;
    state->dataSurface->Surface(7).Vertex(4).x = 10.0;
    state->dataSurface->Surface(7).Vertex(5).x = 5.0;
    state->dataSurface->Surface(7).Vertex(1).y = 10.0;
    state->dataSurface->Surface(7).Vertex(2).y = 5.0;
    state->dataSurface->Surface(7).Vertex(3).y = 5.0;
    state->dataSurface->Surface(7).Vertex(4).y = 10.0;
    state->dataSurface->Surface(7).Vertex(5).y = 11.0;
    state->dataSurface->Surface(7).Vertex(1).z = 2.4384;
    state->dataSurface->Surface(7).Vertex(2).z = 2.4384;
    state->dataSurface->Surface(7).Vertex(3).z = 2.4384;
    state->dataSurface->Surface(7).Vertex(4).z = 2.4384;
    state->dataSurface->Surface(7).Vertex(5).z = 2.4384;

    // Polygonal horizontal base surface
    state->dataSurface->Surface(13).Name = "ROOF-POLY-WINDOW1";
    state->dataSurface->Surface(13).Zone = 1;
    state->dataSurface->Surface(13).ZoneName = "ZONE 1";
    state->dataSurface->Surface(13).Azimuth = 0.0;
    state->dataSurface->Surface(13).ExtBoundCond = 0;
    state->dataSurface->Surface(13).HeatTransSurf = true;
    state->dataSurface->Surface(13).Tilt = 0.0;
    state->dataSurface->Surface(13).Sides = 3;
    state->dataSurface->Surface(13).Area = 1;
    state->dataSurface->Surface(13).BaseSurf = 7;
    state->dataSurface->Surface(13).Vertex.allocate(3);
    state->dataSurface->Surface(13).Vertex(1).x = 8.0;
    state->dataSurface->Surface(13).Vertex(2).x = 8.0;
    state->dataSurface->Surface(13).Vertex(3).x = 9.0;
    state->dataSurface->Surface(13).Vertex(1).y = 9.0;
    state->dataSurface->Surface(13).Vertex(2).y = 7.0;
    state->dataSurface->Surface(13).Vertex(3).y = 7.0;
    state->dataSurface->Surface(13).Vertex(1).z = 2.4384;
    state->dataSurface->Surface(13).Vertex(2).z = 2.4384;
    state->dataSurface->Surface(13).Vertex(3).z = 2.4384;

    // Polygonal horizontal base surface
    state->dataSurface->Surface(14).Name = "ROOF-POLY-WINDOW2";
    state->dataSurface->Surface(14).Zone = 1;
    state->dataSurface->Surface(14).ZoneName = "ZONE 1";
    state->dataSurface->Surface(14).Azimuth = 0.0;
    state->dataSurface->Surface(14).ExtBoundCond = 0;
    state->dataSurface->Surface(14).HeatTransSurf = true;
    state->dataSurface->Surface(14).Tilt = 0.0;
    state->dataSurface->Surface(14).Sides = 3;
    state->dataSurface->Surface(14).Area = 1;
    state->dataSurface->Surface(14).BaseSurf = 7;
    state->dataSurface->Surface(14).Vertex.allocate(3);
    state->dataSurface->Surface(14).Vertex(1).x = 6.0;
    state->dataSurface->Surface(14).Vertex(2).x = 6.0;
    state->dataSurface->Surface(14).Vertex(3).x = 7.0;
    state->dataSurface->Surface(14).Vertex(1).y = 9.0;
    state->dataSurface->Surface(14).Vertex(2).y = 7.0;
    state->dataSurface->Surface(14).Vertex(3).y = 7.0;
    state->dataSurface->Surface(14).Vertex(1).z = 2.4384;
    state->dataSurface->Surface(14).Vertex(2).z = 2.4384;
    state->dataSurface->Surface(14).Vertex(3).z = 2.4384;

    // Rectangular horizontal base surface
    state->dataSurface->Surface(8).Name = "ROOF-REC";
    state->dataSurface->Surface(8).Zone = 1;
    state->dataSurface->Surface(8).ZoneName = "ZONE 1";
    state->dataSurface->Surface(8).Azimuth = 0.0;
    state->dataSurface->Surface(8).ExtBoundCond = 0;
    state->dataSurface->Surface(8).HeatTransSurf = true;
    state->dataSurface->Surface(8).Tilt = 0.0;
    state->dataSurface->Surface(8).Sides = 4;
    state->dataSurface->Surface(8).Area = 50.0;
    state->dataSurface->Surface(8).Width = 10.0;
    state->dataSurface->Surface(8).Height = 5.0;
    state->dataSurface->Surface(8).Vertex.allocate(4);
    state->dataSurface->Surface(8).Vertex(1).x = 0.0;
    state->dataSurface->Surface(8).Vertex(2).x = 0.0;
    state->dataSurface->Surface(8).Vertex(3).x = 10.0;
    state->dataSurface->Surface(8).Vertex(4).x = 10.0;
    state->dataSurface->Surface(8).Vertex(1).y = 5.0;
    state->dataSurface->Surface(8).Vertex(2).y = 0.0;
    state->dataSurface->Surface(8).Vertex(3).y = 0.0;
    state->dataSurface->Surface(8).Vertex(4).y = 5.0;
    state->dataSurface->Surface(8).Vertex(1).z = 2.4384;
    state->dataSurface->Surface(8).Vertex(2).z = 2.4384;
    state->dataSurface->Surface(8).Vertex(3).z = 2.4384;
    state->dataSurface->Surface(8).Vertex(4).z = 2.4384;

    // Rectangular horizontal base surface
    state->dataSurface->Surface(12).Name = "ROOF-REC-WINDOW1";
    state->dataSurface->Surface(12).Zone = 1;
    state->dataSurface->Surface(12).ZoneName = "ZONE 1";
    state->dataSurface->Surface(12).Azimuth = 0.0;
    state->dataSurface->Surface(12).ExtBoundCond = 0;
    state->dataSurface->Surface(12).HeatTransSurf = true;
    state->dataSurface->Surface(12).Tilt = 0.0;
    state->dataSurface->Surface(12).Sides = 3;
    state->dataSurface->Surface(12).Area = 0.5;
    state->dataSurface->Surface(12).BaseSurf = 8;
    state->dataSurface->Surface(12).Vertex.allocate(3);
    state->dataSurface->Surface(12).Vertex(1).x = 8.0;
    state->dataSurface->Surface(12).Vertex(2).x = 8.0;
    state->dataSurface->Surface(12).Vertex(3).x = 9.0;
    state->dataSurface->Surface(12).Vertex(1).y = 4.0;
    state->dataSurface->Surface(12).Vertex(2).y = 3.0;
    state->dataSurface->Surface(12).Vertex(3).y = 3.0;
    state->dataSurface->Surface(12).Vertex(1).z = 2.4384;
    state->dataSurface->Surface(12).Vertex(2).z = 2.4384;
    state->dataSurface->Surface(12).Vertex(3).z = 2.4384;

    state->dataSurface->Surface(11).Name = "ROOF-REC-WINDOW2";
    state->dataSurface->Surface(11).Zone = 1;
    state->dataSurface->Surface(11).ZoneName = "ZONE 1";
    state->dataSurface->Surface(11).Azimuth = 0.0;
    state->dataSurface->Surface(11).ExtBoundCond = 0;
    state->dataSurface->Surface(11).HeatTransSurf = true;
    state->dataSurface->Surface(11).Tilt = 0.0;
    state->dataSurface->Surface(11).Sides = 3;
    state->dataSurface->Surface(11).Area = 0.5;
    state->dataSurface->Surface(11).BaseSurf = 8;
    state->dataSurface->Surface(11).Vertex.allocate(3);
    state->dataSurface->Surface(11).Vertex(1).x = 7.0;
    state->dataSurface->Surface(11).Vertex(2).x = 7.0;
    state->dataSurface->Surface(11).Vertex(3).x = 8.0;
    state->dataSurface->Surface(11).Vertex(1).y = 4.0;
    state->dataSurface->Surface(11).Vertex(2).y = 3.0;
    state->dataSurface->Surface(11).Vertex(3).y = 3.0;
    state->dataSurface->Surface(11).Vertex(1).z = 2.4384;
    state->dataSurface->Surface(11).Vertex(2).z = 2.4384;
    state->dataSurface->Surface(11).Vertex(3).z = 2.4384;

    SurfaceGeometry::AllocateSurfaceWindows(*state, 14);
    state->dataSurface->Surface(4).OriginalClass = DataSurfaces::SurfaceClass::Window;
    state->dataSurface->Surface(5).OriginalClass = DataSurfaces::SurfaceClass::Window;
    state->dataSurface->Surface(6).OriginalClass = DataSurfaces::SurfaceClass::Window;
    state->dataSurface->Surface(9).OriginalClass = DataSurfaces::SurfaceClass::Window;
    state->dataSurface->Surface(10).OriginalClass = DataSurfaces::SurfaceClass::Window;
    state->dataSurface->Surface(11).OriginalClass = DataSurfaces::SurfaceClass::Window;
    state->dataSurface->Surface(12).OriginalClass = DataSurfaces::SurfaceClass::Window;
    state->dataSurface->Surface(13).OriginalClass = DataSurfaces::SurfaceClass::Window;
    state->dataSurface->Surface(14).OriginalClass = DataSurfaces::SurfaceClass::Window;
    state->dataGlobal->NumOfZones = 1;

    std::string const idf_objects = delimited_string({
        "Schedule:Constant,OnSch,,1.0;",
        "Schedule:Constant,Aula people sched,,0.0;",
        "Schedule:Constant,Sempre 21,,21.0;",
        "AirflowNetwork:SimulationControl,",
        "  NaturalVentilation, !- Name",
        "  MultizoneWithoutDistribution, !- AirflowNetwork Control",
        "  SurfaceAverageCalculation, !- Wind Pressure Coefficient Type",
        "  , !- Height Selection for Local Wind Pressure Calculation",
        "  LOWRISE, !- Building Type",
        "  1000, !- Maximum Number of Iterations{ dimensionless }",
        "  LinearInitializationMethod, !- Initialization Type",
        "  0.0001, !- Relative Airflow Convergence Tolerance{ dimensionless }",
        "  0.0001, !- Absolute Airflow Convergence Tolerance{ kg / s }",
        "  -0.5, !- Convergence Acceleration Limit{ dimensionless }",
        "  90, !- Azimuth Angle of Long Axis of Building{ deg }",
        "  0.36;                    !- Ratio of Building Width Along Short Axis to Width Along Long Axis",
        "AirflowNetwork:MultiZone:Zone,",
        "  ZONE 1, !- Zone Name",
        "  Temperature, !- Ventilation Control Mode",
        "  Sempre 21, !- Ventilation Control Zone Temperature Setpoint Schedule Name",
        "  1, !- Minimum Venting Open Factor{ dimensionless }",
        "  , !- Indoor and Outdoor Temperature Difference Lower Limit For Maximum Venting Open Factor{ deltaC }",
        "  100, !- Indoor and Outdoor Temperature Difference Upper Limit for Minimum Venting Open Factor{ deltaC }",
        "  , !- Indoor and Outdoor Enthalpy Difference Lower Limit For Maximum Venting Open Factor{ deltaJ / kg }",
        "  300000, !- Indoor and Outdoor Enthalpy Difference Upper Limit for Minimum Venting Open Factor{ deltaJ / kg }",
        "  Aula people sched, !- Venting Availability Schedule Name",
        "  Standard;                !- Single Sided Wind Pressure Coefficient Algorithm",
        "AirflowNetwork:MultiZone:Surface,",
        "  NORTH:WINDOW1, !- Surface Name",
        "  Simple Window, !- Leakage Component Name",
        "  , !- External Node Name",
        "  1, !- Window / Door Opening Factor, or Crack Factor{ dimensionless }",
        "  ZoneLevel, !- Ventilation Control Mode",
        "  , !- Ventilation Control Zone Temperature Setpoint Schedule Name",
        "  , !- Minimum Venting Open Factor{ dimensionless }",
        "  , !- Indoor and Outdoor Temperature Difference Lower Limit For Maximum Venting Open Factor{ deltaC }",
        "  100, !- Indoor and Outdoor Temperature Difference Upper Limit for Minimum Venting Open Factor{ deltaC }",
        "  , !- Indoor and Outdoor Enthalpy Difference Lower Limit For Maximum Venting Open Factor{ deltaJ / kg }",
        "  300000, !- Indoor and Outdoor Enthalpy Difference Upper Limit for Minimum Venting Open Factor{ deltaJ / kg }",
        "  Aula people sched;       !- Venting Availability Schedule Name",
        "AirflowNetwork:MultiZone:Surface,",
        "  NORTH:WINDOW2, !- Surface Name",
        "  Simple Window, !- Leakage Component Name",
        "  , !- External Node Name",
        "  1, !- Window / Door Opening Factor, or Crack Factor{ dimensionless }",
        "  Temperature, !- Ventilation Control Mode",
        "  Sempre 21, !- Ventilation Control Zone Temperature Setpoint Schedule Name",
        "  1, !- Minimum Venting Open Factor{ dimensionless }",
        "  , !- Indoor and Outdoor Temperature Difference Lower Limit For Maximum Venting Open Factor{ deltaC }",
        "  100, !- Indoor and Outdoor Temperature Difference Upper Limit for Minimum Venting Open Factor{ deltaC }",
        "  , !- Indoor and Outdoor Enthalpy Difference Lower Limit For Maximum Venting Open Factor{ deltaJ / kg }",
        "  300000, !- Indoor and Outdoor Enthalpy Difference Upper Limit for Minimum Venting Open Factor{ deltaJ / kg }",
        "  Aula people sched;       !- Venting Availability Schedule Name",
        "AirflowNetwork:MultiZone:Surface,",
        "  SOUTH:WINDOW1, !- Surface Name",
        "  Simple Window, !- Leakage Component Name",
        "  , !- External Node Name",
        "  1, !- Window / Door Opening Factor, or Crack Factor{ dimensionless }",
        "  ZoneLevel, !- Ventilation Control Mode",
        "  , !- Ventilation Control Zone Temperature Setpoint Schedule Name",
        "  , !- Minimum Venting Open Factor{ dimensionless }",
        "  , !- Indoor and Outdoor Temperature Difference Lower Limit For Maximum Venting Open Factor{ deltaC }",
        "  100, !- Indoor and Outdoor Temperature Difference Upper Limit for Minimum Venting Open Factor{ deltaC }",
        "  , !- Indoor and Outdoor Enthalpy Difference Lower Limit For Maximum Venting Open Factor{ deltaJ / kg }",
        "  300000, !- Indoor and Outdoor Enthalpy Difference Upper Limit for Minimum Venting Open Factor{ deltaJ / kg }",
        "  Aula people sched,       !- Venting Availability Schedule Name",
        "  ,       !- Occupant Ventilation Control Name",
        "  BaseSurfaceAspectRatio;       !- Equivalent Rectangle Method",
        "AirflowNetwork:MultiZone:Surface,",
        "  SOUTH:WINDOW2, !- Surface Name",
        "  Simple Window, !- Leakage Component Name",
        "  , !- External Node Name",
        "  1, !- Window / Door Opening Factor, or Crack Factor{ dimensionless }",
        "  Temperature, !- Ventilation Control Mode",
        "  Sempre 21, !- Ventilation Control Zone Temperature Setpoint Schedule Name",
        "  1, !- Minimum Venting Open Factor{ dimensionless }",
        "  , !- Indoor and Outdoor Temperature Difference Lower Limit For Maximum Venting Open Factor{ deltaC }",
        "  100, !- Indoor and Outdoor Temperature Difference Upper Limit for Minimum Venting Open Factor{ deltaC }",
        "  , !- Indoor and Outdoor Enthalpy Difference Lower Limit For Maximum Venting Open Factor{ deltaJ / kg }",
        "  300000, !- Indoor and Outdoor Enthalpy Difference Upper Limit for Minimum Venting Open Factor{ deltaJ / kg }",
        "  Aula people sched,       !- Venting Availability Schedule Name",
        "  ,       !- Occupant Ventilation Control Name",
        "  UserDefinedAspectRatio,       !- Equivalent Rectangle Method",
        "  1.0;       !- Equivalent Rectangle Aspect Ratio",
        "AirflowNetwork:MultiZone:Surface,",
        "  EAST:WINDOW1, !- Surface Name",
        "  Simple Window, !- Leakage Component Name",
        "  , !- External Node Name",
        "  1, !- Window / Door Opening Factor, or Crack Factor{ dimensionless }",
        "  ZoneLevel, !- Ventilation Control Mode",
        "  , !- Ventilation Control Zone Temperature Setpoint Schedule Name",
        "  , !- Minimum Venting Open Factor{ dimensionless }",
        "  , !- Indoor and Outdoor Temperature Difference Lower Limit For Maximum Venting Open Factor{ deltaC }",
        "  100, !- Indoor and Outdoor Temperature Difference Upper Limit for Minimum Venting Open Factor{ deltaC }",
        "  , !- Indoor and Outdoor Enthalpy Difference Lower Limit For Maximum Venting Open Factor{ deltaJ / kg }",
        "  300000, !- Indoor and Outdoor Enthalpy Difference Upper Limit for Minimum Venting Open Factor{ deltaJ / kg }",
        "  Aula people sched,       !- Venting Availability Schedule Name",
        "  ,       !- Occupant Ventilation Control Name",
        "  BaseSurfaceAspectRatio;       !- Equivalent Rectangle Method",

        "AirflowNetwork:MultiZone:Surface,",
        "  ROOF-REC-WINDOW1, !- Surface Name",
        "  CR-1, !- Leakage Component Name",
        "  , !- External Node Name",
        "  1, !- Window / Door Opening Factor, or Crack Factor{ dimensionless }",
        "  ZoneLevel, !- Ventilation Control Mode",
        "  , !- Ventilation Control Zone Temperature Setpoint Schedule Name",
        "  , !- Minimum Venting Open Factor{ dimensionless }",
        "  , !- Indoor and Outdoor Temperature Difference Lower Limit For Maximum Venting Open Factor{ deltaC }",
        "  100, !- Indoor and Outdoor Temperature Difference Upper Limit for Minimum Venting Open Factor{ deltaC }",
        "  , !- Indoor and Outdoor Enthalpy Difference Lower Limit For Maximum Venting Open Factor{ deltaJ / kg }",
        "  300000, !- Indoor and Outdoor Enthalpy Difference Upper Limit for Minimum Venting Open Factor{ deltaJ / kg }",
        "  Aula people sched;       !- Venting Availability Schedule Name",
        "AirflowNetwork:MultiZone:Surface,",
        "  ROOF-REC-WINDOW2, !- Surface Name",
        "  CR-1, !- Leakage Component Name",
        "  , !- External Node Name",
        "  1, !- Window / Door Opening Factor, or Crack Factor{ dimensionless }",
        "  ZoneLevel, !- Ventilation Control Mode",
        "  , !- Ventilation Control Zone Temperature Setpoint Schedule Name",
        "  , !- Minimum Venting Open Factor{ dimensionless }",
        "  , !- Indoor and Outdoor Temperature Difference Lower Limit For Maximum Venting Open Factor{ deltaC }",
        "  100, !- Indoor and Outdoor Temperature Difference Upper Limit for Minimum Venting Open Factor{ deltaC }",
        "  , !- Indoor and Outdoor Enthalpy Difference Lower Limit For Maximum Venting Open Factor{ deltaJ / kg }",
        "  300000, !- Indoor and Outdoor Enthalpy Difference Upper Limit for Minimum Venting Open Factor{ deltaJ / kg }",
        "  Aula people sched,       !- Venting Availability Schedule Name",
        "  ,       !- Occupant Ventilation Control Name",
        "  BaseSurfaceAspectRatio;       !- Equivalent Rectangle Method",

        "AirflowNetwork:MultiZone:Surface,",
        "  ROOF-POLY-WINDOW1, !- Surface Name",
        "  CR-1, !- Leakage Component Name",
        "  , !- External Node Name",
        "  1, !- Window / Door Opening Factor, or Crack Factor{ dimensionless }",
        "  ZoneLevel, !- Ventilation Control Mode",
        "  , !- Ventilation Control Zone Temperature Setpoint Schedule Name",
        "  , !- Minimum Venting Open Factor{ dimensionless }",
        "  , !- Indoor and Outdoor Temperature Difference Lower Limit For Maximum Venting Open Factor{ deltaC }",
        "  100, !- Indoor and Outdoor Temperature Difference Upper Limit for Minimum Venting Open Factor{ deltaC }",
        "  , !- Indoor and Outdoor Enthalpy Difference Lower Limit For Maximum Venting Open Factor{ deltaJ / kg }",
        "  300000, !- Indoor and Outdoor Enthalpy Difference Upper Limit for Minimum Venting Open Factor{ deltaJ / kg }",
        "  Aula people sched,       !- Venting Availability Schedule Name",
        "  ,       !- Occupant Ventilation Control Name",
        "  BaseSurfaceAspectRatio;       !- Equivalent Rectangle Method",
        "AirflowNetwork:MultiZone:Surface,",
        "  ROOF-POLY-WINDOW2, !- Surface Name",
        "  CR-1, !- Leakage Component Name",
        "  , !- External Node Name",
        "  1, !- Window / Door Opening Factor, or Crack Factor{ dimensionless }",
        "  ZoneLevel, !- Ventilation Control Mode",
        "  , !- Ventilation Control Zone Temperature Setpoint Schedule Name",
        "  , !- Minimum Venting Open Factor{ dimensionless }",
        "  , !- Indoor and Outdoor Temperature Difference Lower Limit For Maximum Venting Open Factor{ deltaC }",
        "  100, !- Indoor and Outdoor Temperature Difference Upper Limit for Minimum Venting Open Factor{ deltaC }",
        "  , !- Indoor and Outdoor Enthalpy Difference Lower Limit For Maximum Venting Open Factor{ deltaJ / kg }",
        "  300000, !- Indoor and Outdoor Enthalpy Difference Upper Limit for Minimum Venting Open Factor{ deltaJ / kg }",
        "  Aula people sched;       !- Venting Availability Schedule Name",

        "AirflowNetwork:MultiZone:Component:SimpleOpening,",
        "  Simple Window, !- Name",
        "  0.0010, !- Air Mass Flow Coefficient When Opening is Closed{ kg / s - m }",
        "  0.65, !- Air Mass Flow Exponent When Opening is Closed{ dimensionless }",
        "  0.01, !- Minimum Density Difference for Two - Way Flow{ kg / m3 }",
        "  0.78;                    !- Discharge Coefficient{ dimensionless }",
        "AirflowNetwork:MultiZone:Surface:Crack,",
        "  CR-1, !- Name",
        "  0.05, !- Air Mass Flow Coefficient at Reference Conditions{ kg / s }",
        "  0.667;                   !- Air Mass Flow Exponent{ dimensionless }",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->afn->get_input();

    // Choice: Height; Base Surface: Vertical Rectangular
    EXPECT_NEAR(1.0, state->afn->MultizoneSurfaceData(1).Width, 0.0001);
    EXPECT_NEAR(1.0, state->afn->MultizoneSurfaceData(1).Height, 0.0001);
    // Choice: Height; Base Surface: Vertical Polygon
    EXPECT_NEAR(1.666667, state->afn->MultizoneSurfaceData(2).Width, 0.0001);
    EXPECT_NEAR(1.5, state->afn->MultizoneSurfaceData(2).Height, 0.0001);
    // Choice: Base aspect ratio; Base Surface: Vertical Rectangular
    EXPECT_NEAR(1.454907, state->afn->MultizoneSurfaceData(3).Width, 0.0001);
    EXPECT_NEAR(0.343664, state->afn->MultizoneSurfaceData(3).Height, 0.0001);
    // Choice: User aspect ratio; Base Surface: Vertical Rectangular
    EXPECT_NEAR(0.70711, state->afn->MultizoneSurfaceData(4).Width, 0.0001);
    EXPECT_NEAR(0.70711, state->afn->MultizoneSurfaceData(4).Height, 0.0001);
    // Choice: Base aspect ratio --> Height; Base Surface: Vertical Polygon
    EXPECT_NEAR(0.5, state->afn->MultizoneSurfaceData(5).Width, 0.0001);
    EXPECT_NEAR(1.0, state->afn->MultizoneSurfaceData(5).Height, 0.0001);
    // Choice: Height --> Base aspect ratio; Base Surface: Horizontal Rectangular
    EXPECT_NEAR(1.0, state->afn->MultizoneSurfaceData(6).Width, 0.0001);
    EXPECT_NEAR(0.5, state->afn->MultizoneSurfaceData(6).Height, 0.0001);
    // Choice: Base aspect ratio; Base Surface: Horizontal Rectangular
    EXPECT_NEAR(1.0, state->afn->MultizoneSurfaceData(7).Width, 0.0001);
    EXPECT_NEAR(0.5, state->afn->MultizoneSurfaceData(7).Height, 0.0001);
    // Choice: Base aspect ratio --> User Aspect Ratio; Base Surface: Horizontal Polygon
    EXPECT_NEAR(1.0, state->afn->MultizoneSurfaceData(8).Width, 0.0001);
    EXPECT_NEAR(1.0, state->afn->MultizoneSurfaceData(8).Height, 0.0001);
    // Choice: Height --> User Aspect Ratio; Base Surface: Horizontal Polygon
    EXPECT_NEAR(1.0, state->afn->MultizoneSurfaceData(9).Width, 0.0001);
    EXPECT_NEAR(1.0, state->afn->MultizoneSurfaceData(9).Height, 0.0001);

    state->dataHeatBal->Zone.deallocate();
    state->dataSurface->Surface.deallocate();
    state->dataSurface->SurfaceWindow.deallocate();
}

TEST_F(EnergyPlusFixture, AirflowNetwork_TestFanModel)
{

    // Unit test for a new feature of AFN Fan Model
    int i;

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
        "    RunPeriod 1,             !- Name",
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
        "    RunPeriod 2,             !- Name",
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

        "  WindowMaterial:Glazing,",
        "    CLEAR 3MM,               !- Name",
        "    SpectralAverage,         !- Optical Data Type",
        "    ,                        !- Window Glass Spectral Data Set Name",
        "    0.003,                   !- Thickness {m}",
        "    0.837,                   !- Solar Transmittance at Normal Incidence",
        "    0.075,                   !- Front Side Solar Reflectance at Normal Incidence",
        "    0.075,                   !- Back Side Solar Reflectance at Normal Incidence",
        "    0.898,                   !- Visible Transmittance at Normal Incidence",
        "    0.081,                   !- Front Side Visible Reflectance at Normal Incidence",
        "    0.081,                   !- Back Side Visible Reflectance at Normal Incidence",
        "    0.0,                     !- Infrared Transmittance at Normal Incidence",
        "    0.84,                    !- Front Side Infrared Hemispherical Emissivity",
        "    0.84,                    !- Back Side Infrared Hemispherical Emissivity",
        "    0.9;                     !- Conductivity {W/m-K}",

        "  WindowMaterial:Gas,",
        "    AIR 6MM,                 !- Name",
        "    AIR,                     !- Gas Type",
        "    0.006;                   !- Thickness {m}",

        "  Construction,",
        "    EXTWALL:LIVING,          !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    CB11,                    !- Layer 2",
        "    GP01;                    !- Layer 3",

        "  Construction,",
        "    INTERIORWall,            !- Name",
        "    GP01,                    !- Outside Layer",
        "    IN02,                    !- Layer 2",
        "    GP01;                    !- Layer 3",

        "  Construction,",
        "    FLOOR:GARAGE,            !- Name",
        "    CC03;                    !- Outside Layer",

        "  Construction,",
        "    FLOOR:LIVING,            !- Name",
        "    CC03,                    !- Outside Layer",
        "    CP02;                    !- Layer 2",

        "  Construction,",
        "    ROOF,                    !- Name",
        "    AR02,                    !- Outside Layer",
        "    PW03;                    !- Layer 2",

        "  Construction,",
        "    EXTWALL:GARAGE,          !- Name",
        "    A1 - 1 IN STUCCO,        !- Outside Layer",
        "    CB11;                    !- Layer 2",

        "  Construction,",
        "    CEILING:LIVING,          !- Name",
        "    IN05,                    !- Outside Layer",
        "    GP01;                    !- Layer 2",

        "  Construction,",
        "    reverseCEILING:LIVING,   !- Name",
        "    GP01,                    !- Outside Layer",
        "    IN05;                    !- Layer 2",

        "  Construction,",
        "    GABLE,                   !- Name",
        "    PW03;                    !- Outside Layer",

        "  Construction,",
        "    Dbl Clr 3mm/6mm Air,     !- Name",
        "    CLEAR 3MM,               !- Outside Layer",
        "    AIR 6MM,                 !- Layer 2",
        "    CLEAR 3MM;               !- Layer 3",

        "  Construction,",
        "    Garage:SteelDoor,        !- Name",
        "    HF-A3;                   !- Outside Layer",

        "  Construction,",
        "    CEILING:Garage,          !- Name",
        "    GP01;                    !- Outside Layer",

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

        "  Zone,",
        "    GARAGE ZONE,             !- Name",
        "    0,                       !- Direction of Relative North {deg}",
        "    0,                       !- X Origin {m}",
        "    0,                       !- Y Origin {m}",
        "    0,                       !- Z Origin {m}",
        "    1,                       !- Type",
        "    1,                       !- Multiplier",
        "    autocalculate,           !- Ceiling Height {m}",
        "    autocalculate;           !- Volume {m3}",

        "  Zone,",
        "    ATTIC ZONE,              !- Name",
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
        "    10.323,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.323,10.778,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,10.778,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

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
        "    17.242,0,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

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
        "    0,0,2.4383,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,0,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

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
        "    0,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,10.778,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,0,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,0,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Garage:Interior,         !- Name",
        "    WALL,                    !- Surface Type",
        "    INTERIORWall,            !- Construction Name",
        "    GARAGE ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Living:Interior,         !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.323,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.323,10.778,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Interior,         !- Name",
        "    WALL,                    !- Surface Type",
        "    INTERIORWall,            !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Garage:Interior,         !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    17.242,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,10.778,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.323,10.778,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.323,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

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
        "    0,10.778,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,0,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Living:Ceiling,          !- Name",
        "    CEILING,                 !- Surface Type",
        "    CEILING:LIVING,          !- Construction Name",
        "    LIVING ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Attic:LivingFloor,       !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,0,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,0,2.4384,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Attic:LivingFloor,       !- Name",
        "    FLOOR,                   !- Surface Type",
        "    reverseCEILING:LIVING,   !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Living:Ceiling,          !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0,0,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,10.778,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,2.4384,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,0,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    NorthRoof1,              !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.9,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    13.782,5.389,4.6838,  !- X,Y,Z ==> Vertex 1 {m}",
        "    13.782,7.3172,3.8804,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,7.3172,3.8804,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,5.389,4.6838;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    SouthRoof,               !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5000000,               !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    0.000000,5.389000,4.683800,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.000000,0.000000,2.438400,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.24200,0.000000,2.438400,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.24200,5.389000,4.683800;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    NorthRoof2,              !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.9,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    13.782,7.3172,3.8804,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.332,10.778,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,10.778,2.4384,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0.0,7.3172,3.8804;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    NorthRoof3,              !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.9,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    17.242,5.389,4.6838,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,7.3172,3.8804,  !- X,Y,Z ==> Vertex 2 {m}",
        "    13.782,7.3172,3.8804,  !- X,Y,Z ==> Vertex 3 {m}",
        "    13.782,5.389,4.6838;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    NorthRoof4,              !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.9,                     !- View Factor to Ground",
        "    3,                       !- Number of Vertices",
        "    17.242,7.3172,3.8804,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,10.778,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    13.782,7.3172,3.8804;  !- X,Y,Z ==> Vertex 3 {m}",

        "  BuildingSurface:Detailed,",
        "    EastGable,               !- Name",
        "    WALL,                    !- Surface Type",
        "    GABLE,                   !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    3,                       !- Number of Vertices",
        "    17.242,5.389,4.6838,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,0.0,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,2.4384;  !- X,Y,Z ==> Vertex 3 {m}",

        "  BuildingSurface:Detailed,",
        "    WestGable,               !- Name",
        "    WALL,                    !- Surface Type",
        "    GABLE,                   !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    3,                       !- Number of Vertices",
        "    0.0,5.389,4.6838,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0.0,10.778,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0.0,0.0,2.4384;  !- X,Y,Z ==> Vertex 3 {m}",

        "  BuildingSurface:Detailed,",
        "    EastRoof,                !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.9,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    13.782,16.876,3.8804,  !- X,Y,Z ==> Vertex 1 {m}",
        "    13.782,7.3172,3.8804,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,2.4384,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,16.876,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    WestRoof,                !- Name",
        "    ROOF,                    !- Surface Type",
        "    ROOF,                    !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.9,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.323,16.876,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.323,10.778,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    13.782,7.3172,3.8804,  !- X,Y,Z ==> Vertex 3 {m}",
        "    13.782,16.876,3.8804;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Attic:NorthGable,        !- Name",
        "    WALL,                    !- Surface Type",
        "    GABLE,                   !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    3,                       !- Number of Vertices",
        "    13.782,16.876,3.8804,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,16.876,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.323,16.876,2.4384;  !- X,Y,Z ==> Vertex 3 {m}",

        "  BuildingSurface:Detailed,",
        "    Garage:EastWall,         !- Name",
        "    WALL,                    !- Surface Type",
        "    EXTWALL:GARAGE,          !- Construction Name",
        "    GARAGE ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    17.242,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,10.778,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,16.876,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,16.876,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Garage:WestWall,         !- Name",
        "    WALL,                    !- Surface Type",
        "    EXTWALL:GARAGE,          !- Construction Name",
        "    GARAGE ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.323,16.876,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.323,16.876,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.323,10.778,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.323,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Garage:FrontDoor,        !- Name",
        "    WALL,                    !- Surface Type",
        "    Garage:SteelDoor,        !- Construction Name",
        "    GARAGE ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Outdoors,                !- Outside Boundary Condition",
        "    ,                        !- Outside Boundary Condition Object",
        "    SunExposed,              !- Sun Exposure",
        "    WindExposed,             !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    17.242,16.876,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,16.876,0.0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    10.323,16.876,0.0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    10.323,16.876,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Attic:GarageFloor,       !- Name",
        "    FLOOR,                   !- Surface Type",
        "    CEILING:Garage,          !- Construction Name",
        "    ATTIC ZONE,              !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Garage:Ceiling,          !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.323,10.778,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.323,16.876,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,16.876,2.4384,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,10.778,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Garage:Ceiling,          !- Name",
        "    CEILING,                 !- Surface Type",
        "    CEILING:Garage,          !- Construction Name",
        "    GARAGE ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Attic:GarageFloor,       !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0.5,                     !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.323,16.876,2.4384,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.323,10.778,2.4384,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,10.778,2.4384,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,16.876,2.4384;  !- X,Y,Z ==> Vertex 4 {m}",

        "  BuildingSurface:Detailed,",
        "    Garage:Floor,            !- Name",
        "    FLOOR,                   !- Surface Type",
        "    FLOOR:GARAGE,            !- Construction Name",
        "    GARAGE ZONE,             !- Zone Name",
        "    ,                        !- Space Name",
        "    Surface,                 !- Outside Boundary Condition",
        "    Garage:Floor,            !- Outside Boundary Condition Object",
        "    NoSun,                   !- Sun Exposure",
        "    NoWind,                  !- Wind Exposure",
        "    0,                       !- View Factor to Ground",
        "    4,                       !- Number of Vertices",
        "    10.323,10.778,0,  !- X,Y,Z ==> Vertex 1 {m}",
        "    10.323,16.876,0,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,16.876,0,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,10.778,0;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    NorthWindow,             !- Name",
        "    Window,                  !- Surface Type",
        "    Dbl Clr 3mm/6mm Air,     !- Construction Name",
        "    Living:North,            !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    6.572,10.778,2.1336,  !- X,Y,Z ==> Vertex 1 {m}",
        "    6.572,10.778,0.6096,  !- X,Y,Z ==> Vertex 2 {m}",
        "    2,10.778,0.6096,  !- X,Y,Z ==> Vertex 3 {m}",
        "    2,10.778,2.1336;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    EastWindow,              !- Name",
        "    Window,                  !- Surface Type",
        "    Dbl Clr 3mm/6mm Air,     !- Construction Name",
        "    Living:East,             !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    17.242,2,2.1336,  !- X,Y,Z ==> Vertex 1 {m}",
        "    17.242,2,0.6096,  !- X,Y,Z ==> Vertex 2 {m}",
        "    17.242,6.572,0.6096,  !- X,Y,Z ==> Vertex 3 {m}",
        "    17.242,6.572,2.1336;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    SouthWindow,             !- Name",
        "    Window,                  !- Surface Type",
        "    Dbl Clr 3mm/6mm Air,     !- Construction Name",
        "    Living:South,            !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    2,0,2.1336,  !- X,Y,Z ==> Vertex 1 {m}",
        "    2,0,0.6096,  !- X,Y,Z ==> Vertex 2 {m}",
        "    6.572,0,0.6096,  !- X,Y,Z ==> Vertex 3 {m}",
        "    6.572,0,2.1336;  !- X,Y,Z ==> Vertex 4 {m}",

        "  FenestrationSurface:Detailed,",
        "    WestWindow,              !- Name",
        "    Window,                  !- Surface Type",
        "    Dbl Clr 3mm/6mm Air,     !- Construction Name",
        "    Living:West,             !- Building Surface Name",
        "    ,                        !- Outside Boundary Condition Object",
        "    0.5000000,               !- View Factor to Ground",
        "    ,                        !- Frame and Divider Name",
        "    1.0,                     !- Multiplier",
        "    4,                       !- Number of Vertices",
        "    0,6.572,2.1336,  !- X,Y,Z ==> Vertex 1 {m}",
        "    0,6.572,0.6096,  !- X,Y,Z ==> Vertex 2 {m}",
        "    0,2,0.6096,  !- X,Y,Z ==> Vertex 3 {m}",
        "    0,2,2.1336;  !- X,Y,Z ==> Vertex 4 {m}",

        "  ScheduleTypeLimits,",
        "    Any Number;              !- Name",

        "  ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    CONTINUOUS;              !- Numeric Type",

        "  ScheduleTypeLimits,",
        "    Temperature,             !- Name",
        "    -60,                     !- Lower Limit Value",
        "    200,                     !- Upper Limit Value",
        "    CONTINUOUS,              !- Numeric Type",
        "    Temperature;             !- Unit Type",

        "  ScheduleTypeLimits,",
        "    Control Type,            !- Name",
        "    0,                       !- Lower Limit Value",
        "    4,                       !- Upper Limit Value",
        "    DISCRETE;                !- Numeric Type",

        "  ScheduleTypeLimits,",
        "    On/Off,                  !- Name",
        "    0,                       !- Lower Limit Value",
        "    1,                       !- Upper Limit Value",
        "    DISCRETE;                !- Numeric Type",

        "  Schedule:Compact,",
        "    WindowVentSched,         !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 3/31,           !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,25.55,      !- Field 3",
        "    Through: 9/30,           !- Field 5",
        "    For: AllDays,            !- Field 6",
        "    Until: 24:00,21.11,      !- Field 7",
        "    Through: 12/31,          !- Field 9",
        "    For: AllDays,            !- Field 10",
        "    Until: 24:00,25.55;      !- Field 11",

        "  Schedule:Compact,",
        "    Activity Sch,            !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,131.8;      !- Field 3",

        "  Schedule:Compact,",
        "    Work Eff Sch,            !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,0.0;        !- Field 3",

        "  Schedule:Compact,",
        "    Clothing Sch,            !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "  Schedule:Compact,",
        "    Air Velo Sch,            !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,0.137;      !- Field 3",

        "  Schedule:Compact,",
        "    HOUSE OCCUPANCY,         !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays,           !- Field 2",
        "    Until: 6:00,1.0,         !- Field 3",
        "    Until: 7:00,0.10,        !- Field 5",
        "    Until: 8:00,0.50,        !- Field 7",
        "    Until: 12:00,1.00,       !- Field 9",
        "    Until: 13:00,0.50,       !- Field 11",
        "    Until: 16:00,1.00,       !- Field 13",
        "    Until: 17:00,0.50,       !- Field 15",
        "    Until: 18:00,0.10,       !- Field 17",
        "    Until: 24:00,1.0,        !- Field 19",
        "    For: AllOtherDays,       !- Field 21",
        "    Until: 24:00,0.0;        !- Field 22",

        "  Schedule:Compact,",
        "    INTERMITTENT,            !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays,           !- Field 2",
        "    Until: 8:00,0.0,         !- Field 3",
        "    Until: 18:00,1.00,       !- Field 5",
        "    Until: 24:00,0.0,        !- Field 7",
        "    For: AllOtherDays,       !- Field 9",
        "    Until: 24:00,0.0;        !- Field 10",

        "  Schedule:Compact,",
        "    HOUSE LIGHTING,          !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: WeekDays,           !- Field 2",
        "    Until: 6:00,0.05,        !- Field 3",
        "    Until: 7:00,0.20,        !- Field 5",
        "    Until: 17:00,1.00,       !- Field 7",
        "    Until: 18:00,0.50,       !- Field 9",
        "    Until: 24:00,0.05,       !- Field 11",
        "    For: AllOtherDays,       !- Field 13",
        "    Until: 24:00,0.05;       !- Field 14",

        "  Schedule:Compact,",
        "    ReportSch,               !- Name",
        "    on/off,                  !- Schedule Type Limits Name",
        "    Through: 1/20,           !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until:  24:00,0.0,       !- Field 3",
        "    Through: 1/21,           !- Field 5",
        "    For: AllDays,            !- Field 6",
        "    Until:  24:00,1.0,       !- Field 7",
        "    Through: 7/20,           !- Field 9",
        "    For: AllDays,            !- Field 10",
        "    Until:  24:00,0.0,       !- Field 11",
        "    Through: 7/21,           !- Field 13",
        "    For: AllDays,            !- Field 14",
        "    Until:  24:00,1.0,       !- Field 15",
        "    Through: 12/31,          !- Field 17",
        "    For: AllDays,            !- Field 18",
        "    Until:  24:00,0.0;       !- Field 19",

        "  Schedule:Compact,",
        "    HVACAvailSched,          !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "  Schedule:Compact,",
        "    Dual Heating Setpoints,  !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,22.0;       !- Field 3",

        "  Schedule:Compact,",
        "    Dual Cooling Setpoints,  !- Name",
        "    Temperature,             !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,26.6;       !- Field 3",

        "  Schedule:Compact,",
        "    Dual Zone Control Type Sched,  !- Name",
        "    Control Type,            !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,4;          !- Field 3",

        "  Schedule:Compact,",
        "    CyclingFanSchedule,      !- Name",
        "    Any Number,              !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "  People,",
        "    LIVING ZONE People,      !- Name",
        "    LIVING ZONE,             !- Zone or ZoneList Name",
        "    HOUSE OCCUPANCY,         !- Number of People Schedule Name",
        "    people,                  !- Number of People Calculation Method",
        "    3.000000,                !- Number of People",
        "    ,                        !- People per Zone Floor Area {person/m2}",
        "    ,                        !- Zone Floor Area per Person {m2/person}",
        "    0.3000000,               !- Fraction Radiant",
        "    ,                        !- Sensible Heat Fraction",
        "    Activity Sch,            !- Activity Level Schedule Name",
        "    3.82E-8,                 !- Carbon Dioxide Generation Rate {m3/s-W}",
        "    ,                        !- Enable ASHRAE 55 Comfort Warnings",
        "    EnclosureAveraged,            !- Mean Radiant Temperature Calculation Type",
        "    ,                        !- Surface Name/Angle Factor List Name",
        "    Work Eff Sch,            !- Work Efficiency Schedule Name",
        "    ClothingInsulationSchedule,  !- Clothing Insulation Calculation Method",
        "    ,                        !- Clothing Insulation Calculation Method Schedule Name",
        "    Clothing Sch,            !- Clothing Insulation Schedule Name",
        "    Air Velo Sch,            !- Air Velocity Schedule Name",
        "    FANGER;                  !- Thermal Comfort Model 1 Type",

        "  Lights,",
        "    LIVING ZONE Lights,      !- Name",
        "    LIVING ZONE,             !- Zone or ZoneList Name",
        "    HOUSE LIGHTING,          !- Schedule Name",
        "    LightingLevel,           !- Design Level Calculation Method",
        "    1000,                    !- Lighting Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Return Air Fraction",
        "    0.2000000,               !- Fraction Radiant",
        "    0.2000000,               !- Fraction Visible",
        "    0,                       !- Fraction Replaceable",
        "    GeneralLights;           !- End-Use Subcategory",

        "  ElectricEquipment,",
        "    LIVING ZONE ElecEq,      !- Name",
        "    LIVING ZONE,             !- Zone or ZoneList Name",
        "    INTERMITTENT,            !- Schedule Name",
        "    EquipmentLevel,          !- Design Level Calculation Method",
        "    500,                     !- Design Level {W}",
        "    ,                        !- Watts per Zone Floor Area {W/m2}",
        "    ,                        !- Watts per Person {W/person}",
        "    0,                       !- Fraction Latent",
        "    0.3000000,               !- Fraction Radiant",
        "    0;                       !- Fraction Lost",

        "  Curve:Biquadratic,",
        "    WindACCoolCapFT,         !- Name",
        "    0.942587793,             !- Coefficient1 Constant",
        "    0.009543347,             !- Coefficient2 x",
        "    0.000683770,             !- Coefficient3 x**2",
        "    -0.011042676,            !- Coefficient4 y",
        "    0.000005249,             !- Coefficient5 y**2",
        "    -0.000009720,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    WindACEIRFT,             !- Name",
        "    0.342414409,             !- Coefficient1 Constant",
        "    0.034885008,             !- Coefficient2 x",
        "    -0.000623700,            !- Coefficient3 x**2",
        "    0.004977216,             !- Coefficient4 y",
        "    0.000437951,             !- Coefficient5 y**2",
        "    -0.000728028,            !- Coefficient6 x*y",
        "    12.77778,                !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    18.0,                    !- Minimum Value of y",
        "    46.11111,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Quadratic,",
        "    HPACCOOLPLFFPLR,         !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "  Curve:Cubic,",
        "    HPACHeatCapFT,           !- Name",
        "    0.758746,                !- Coefficient1 Constant",
        "    0.027626,                !- Coefficient2 x",
        "    0.000148716,             !- Coefficient3 x**2",
        "    0.0000034992,            !- Coefficient4 x**3",
        "    -20.0,                   !- Minimum Value of x",
        "    20.0,                    !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Cubic,",
        "    HPACHeatCapFFF,          !- Name",
        "    0.84,                    !- Coefficient1 Constant",
        "    0.16,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Cubic,",
        "    HPACHeatEIRFT,           !- Name",
        "    1.19248,                 !- Coefficient1 Constant",
        "    -0.0300438,              !- Coefficient2 x",
        "    0.00103745,              !- Coefficient3 x**2",
        "    -0.000023328,            !- Coefficient4 x**3",
        "    -20.0,                   !- Minimum Value of x",
        "    20.0,                    !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Quadratic,",
        "    HPACHeatEIRFFF,          !- Name",
        "    1.3824,                  !- Coefficient1 Constant",
        "    -0.4336,                 !- Coefficient2 x",
        "    0.0512,                  !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    WindACCoolCapFFF,        !- Name",
        "    0.8,                     !- Coefficient1 Constant",
        "    0.2,                     !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    WindACEIRFFF,            !- Name",
        "    1.1552,                  !- Coefficient1 Constant",
        "    -0.1808,                 !- Coefficient2 x",
        "    0.0256,                  !- Coefficient3 x**2",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    WindACPLFFPLR,           !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.15,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "  NodeList,",
        "    ZoneInlets,              !- Name",
        "    LIVING ZONE NoReheat Air Outlet Node;         !- Node 1 Name",

        "  NodeList,",
        "    Supply Air Temp Nodes,   !- Name",
        "    Heating Coil Air Inlet Node,  !- Node 1 Name",
        "    Air Loop Outlet Node;    !- Node 2 Name",

        "  BranchList,",
        "    Air Loop Branches,       !- Name",
        "    Air Loop Main Branch;    !- Branch 1 Name",

        "  Branch,",
        "    Air Loop Main Branch,    !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    AirLoopHVAC:UnitarySystem,  !- Component 1 Object Type",
        "    DXAC Heat Pump 1,        !- Component 1 Name",
        "    Air Loop Inlet Node,     !- Component 1 Inlet Node Name",
        "    Air Loop Outlet Node;    !- Component 1 Outlet Node Name",

        "  AirLoopHVAC,",
        "    Typical Residential System,  !- Name",
        "    ,                        !- Controller List Name",
        "    Reheat System 1 Avail List,  !- Availability Manager List Name",
        "    1.18,                    !- Design Supply Air Flow Rate {m3/s}",
        "    Air Loop Branches,       !- Branch List Name",
        "    ,                        !- Connector List Name",
        "    Air Loop Inlet Node,     !- Supply Side Inlet Node Name",
        "    Return Air Mixer Outlet, !- Demand Side Outlet Node Name",
        "    Zone Equipment Inlet Node,  !- Demand Side Inlet Node Names",
        "    Air Loop Outlet Node;    !- Supply Side Outlet Node Names",

        "  AirflowNetwork:SimulationControl,",
        "    House AirflowNetwork,    !- Name",
        "    MultizoneWithDistribution,  !- AirflowNetwork Control",
        "    INPUT,                   !- Wind Pressure Coefficient Type",
        "    ExternalNode,            !- Height Selection for Local Wind Pressure Calculation",
        "    LOWRISE,                 !- Building Type",
        "    500,                     !- Maximum Number of Iterations {dimensionless}",
        "    ZeroNodePressures,       !- Initialization Type",
        "    1.0E-05,                 !- Relative Airflow Convergence Tolerance {dimensionless}",
        "    1.0E-06,                 !- Absolute Airflow Convergence Tolerance {kg/s}",
        "    -0.5,                    !- Convergence Acceleration Limit {dimensionless}",
        "    0.0,                     !- Azimuth Angle of Long Axis of Building {deg}",
        "    1.0;                     !- Ratio of Building Width Along Short Axis to Width Along Long Axis",

        "  AirflowNetwork:MultiZone:Zone,",
        "    LIVING ZONE,             !- Zone Name",
        "    Temperature,             !- Ventilation Control Mode",
        "    WindowVentSched,         !- Ventilation Control Zone Temperature Setpoint Schedule Name",
        "    0.3,                     !- Minimum Venting Open Factor {dimensionless}",
        "    5.0,                     !- Indoor and Outdoor Temperature Difference Lower Limit For Maximum Venting Open Factor {deltaC}",
        "    10.0,                    !- Indoor and Outdoor Temperature Difference Upper Limit for Minimum Venting Open Factor {deltaC}",
        "    0.0,                     !- Indoor and Outdoor Enthalpy Difference Lower Limit For Maximum Venting Open Factor {deltaJ/kg}",
        "    300000.0;                !- Indoor and Outdoor Enthalpy Difference Upper Limit for Minimum Venting Open Factor {deltaJ/kg}",

        "  AirflowNetwork:MultiZone:Zone,",
        "    GARAGE ZONE,             !- Zone Name",
        "    NoVent,                  !- Ventilation Control Mode",
        "    ,                        !- Ventilation Control Zone Temperature Setpoint Schedule Name",
        "    1.0,                     !- Minimum Venting Open Factor {dimensionless}",
        "    0.0,                     !- Indoor and Outdoor Temperature Difference Lower Limit For Maximum Venting Open Factor {deltaC}",
        "    100.0,                   !- Indoor and Outdoor Temperature Difference Upper Limit for Minimum Venting Open Factor {deltaC}",
        "    0.0,                     !- Indoor and Outdoor Enthalpy Difference Lower Limit For Maximum Venting Open Factor {deltaJ/kg}",
        "    300000.0;                !- Indoor and Outdoor Enthalpy Difference Upper Limit for Minimum Venting Open Factor {deltaJ/kg}",

        "  AirflowNetwork:MultiZone:Zone,",
        "    ATTIC ZONE,              !- Zone Name",
        "    NoVent,                  !- Ventilation Control Mode",
        "    ,                        !- Ventilation Control Zone Temperature Setpoint Schedule Name",
        "    1.0,                     !- Minimum Venting Open Factor {dimensionless}",
        "    0.0,                     !- Indoor and Outdoor Temperature Difference Lower Limit For Maximum Venting Open Factor {deltaC}",
        "    100.0,                   !- Indoor and Outdoor Temperature Difference Upper Limit for Minimum Venting Open Factor {deltaC}",
        "    0.0,                     !- Indoor and Outdoor Enthalpy Difference Lower Limit For Maximum Venting Open Factor {deltaJ/kg}",
        "    300000.0;                !- Indoor and Outdoor Enthalpy Difference Upper Limit for Minimum Venting Open Factor {deltaJ/kg}",

        "  AirflowNetwork:MultiZone:Surface,",
        "    Living:West,             !- Surface Name",
        "    CR-1,                    !- Leakage Component Name",
        "    WFacade,                 !- External Node Name",
        "    1.0;                     !- Window/Door Opening Factor, or Crack Factor {dimensionless}",

        "  AirflowNetwork:MultiZone:Surface,",
        "    Living:Interior,         !- Surface Name",
        "    CR-1,                    !- Leakage Component Name",
        "    ,                        !- External Node Name",
        "    1.0;                     !- Window/Door Opening Factor, or Crack Factor {dimensionless}",

        "  AirflowNetwork:MultiZone:Surface,",
        "    Garage:FrontDoor,        !- Surface Name",
        "    CRcri,                   !- Leakage Component Name",
        "    NFacade,                 !- External Node Name",
        "    1.0;                     !- Window/Door Opening Factor, or Crack Factor {dimensionless}",

        "  AirflowNetwork:MultiZone:Surface,",
        "    EastRoof,                !- Surface Name",
        "    AtticLeak,               !- Leakage Component Name",
        "    SFacade,                 !- External Node Name",
        "    1.0;                     !- Window/Door Opening Factor, or Crack Factor {dimensionless}",

        "  AirflowNetwork:MultiZone:Surface,",
        "    Living:Ceiling,          !- Surface Name",
        "    InterCondZoneLeak,       !- Leakage Component Name",
        "    ,                        !- External Node Name",
        "    1.0;                     !- Window/Door Opening Factor, or Crack Factor {dimensionless}",

        "  AirflowNetwork:MultiZone:Surface,",
        "    Garage:Ceiling,          !- Surface Name",
        "    InterZoneLeak,           !- Leakage Component Name",
        "    ,                        !- External Node Name",
        "    1.0;                     !- Window/Door Opening Factor, or Crack Factor {dimensionless}",

        "  AirflowNetwork:MultiZone:ReferenceCrackConditions,",
        "    ReferenceCrackConditions,!- Name",
        "    20.0,                    !- Reference Temperature {C}",
        "    101325,                  !- Reference Barometric Pressure {Pa}",
        "    0.0;                     !- Reference Humidity Ratio {kgWater/kgDryAir}",

        "  AirflowNetwork:MultiZone:Surface:Crack,",
        "    CR-1,                    !- Name",
        "    0.05,                    !- Air Mass Flow Coefficient at Reference Conditions {kg/s}",
        "    0.667;                   !- Air Mass Flow Exponent {dimensionless}",

        "  AirflowNetwork:MultiZone:Surface:Crack,",
        "    CRcri,                   !- Name",
        "    0.05,                    !- Air Mass Flow Coefficient at Reference Conditions {kg/s}",
        "    0.667;                   !- Air Mass Flow Exponent {dimensionless}",

        "  AirflowNetwork:MultiZone:Surface:Crack,",
        "    AtticLeak,               !- Name",
        "    0.05,                    !- Air Mass Flow Coefficient at Reference Conditions {kg/s}",
        "    0.65;                    !- Air Mass Flow Exponent {dimensionless}",

        "  AirflowNetwork:MultiZone:Surface:Crack,",
        "    InterZoneLeak,           !- Name",
        "    0.05,                    !- Air Mass Flow Coefficient at Reference Conditions {kg/s}",
        "    0.65;                    !- Air Mass Flow Exponent {dimensionless}",

        "  AirflowNetwork:MultiZone:Surface:Crack,",
        "    InterCondZoneLeak,       !- Name",
        "    0.02,                    !- Air Mass Flow Coefficient at Reference Conditions {kg/s}",
        "    0.65;                    !- Air Mass Flow Exponent {dimensionless}",

        "  AirflowNetwork:MultiZone:ExternalNode,",
        "    NFacade,                 !- Name",
        "    1.524,                   !- External Node Height {m}",
        "    NFacade_WPCValue;        !- Wind Pressure Coefficient Curve Name",

        "  AirflowNetwork:MultiZone:ExternalNode,",
        "    EFacade,                 !- Name",
        "    1.524,                   !- External Node Height {m}",
        "    EFacade_WPCValue;        !- Wind Pressure Coefficient Curve Name",

        "  AirflowNetwork:MultiZone:ExternalNode,",
        "    SFacade,                 !- Name",
        "    1.524,                   !- External Node Height {m}",
        "    SFacade_WPCValue;        !- Wind Pressure Coefficient Curve Name",

        "  AirflowNetwork:MultiZone:ExternalNode,",
        "    WFacade,                 !- Name",
        "    1.524,                   !- External Node Height {m}",
        "    WFacade_WPCValue;        !- Wind Pressure Coefficient Curve Name",

        "  AirflowNetwork:MultiZone:WindPressureCoefficientArray,",
        "    Every 30 Degrees,        !- Name",
        "    0,                       !- Wind Direction 1 {deg}",
        "    30,                      !- Wind Direction 2 {deg}",
        "    60,                      !- Wind Direction 3 {deg}",
        "    90,                      !- Wind Direction 4 {deg}",
        "    120,                     !- Wind Direction 5 {deg}",
        "    150,                     !- Wind Direction 6 {deg}",
        "    180,                     !- Wind Direction 7 {deg}",
        "    210,                     !- Wind Direction 8 {deg}",
        "    240,                     !- Wind Direction 9 {deg}",
        "    270,                     !- Wind Direction 10 {deg}",
        "    300,                     !- Wind Direction 11 {deg}",
        "    330;                     !- Wind Direction 12 {deg}",

        "  AirflowNetwork:MultiZone:WindPressureCoefficientValues,",
        "    NFacade_WPCValue,        !- Name",
        "    Every 30 Degrees,        !- AirflowNetwork:MultiZone:WindPressureCoefficientArray Name",
        "    0.60,                    !- Wind Pressure Coefficient Value 1 {dimensionless}",
        "    0.48,                    !- Wind Pressure Coefficient Value 2 {dimensionless}",
        "    0.04,                    !- Wind Pressure Coefficient Value 3 {dimensionless}",
        "    -0.56,                   !- Wind Pressure Coefficient Value 4 {dimensionless}",
        "    -0.56,                   !- Wind Pressure Coefficient Value 5 {dimensionless}",
        "    -0.42,                   !- Wind Pressure Coefficient Value 6 {dimensionless}",
        "    -0.37,                   !- Wind Pressure Coefficient Value 7 {dimensionless}",
        "    -0.42,                   !- Wind Pressure Coefficient Value 8 {dimensionless}",
        "    -0.56,                   !- Wind Pressure Coefficient Value 9 {dimensionless}",
        "    -0.56,                   !- Wind Pressure Coefficient Value 10 {dimensionless}",
        "    0.04,                    !- Wind Pressure Coefficient Value 11 {dimensionless}",
        "    0.48;                    !- Wind Pressure Coefficient Value 12 {dimensionless}",

        "  AirflowNetwork:MultiZone:WindPressureCoefficientValues,",
        "    EFacade_WPCValue,        !- Name",
        "    Every 30 Degrees,        !- AirflowNetwork:MultiZone:WindPressureCoefficientArray Name",
        "    -0.56,                   !- Wind Pressure Coefficient Value 1 {dimensionless}",
        "    0.04,                    !- Wind Pressure Coefficient Value 2 {dimensionless}",
        "    0.48,                    !- Wind Pressure Coefficient Value 3 {dimensionless}",
        "    0.60,                    !- Wind Pressure Coefficient Value 4 {dimensionless}",
        "    0.48,                    !- Wind Pressure Coefficient Value 5 {dimensionless}",
        "    0.04,                    !- Wind Pressure Coefficient Value 6 {dimensionless}",
        "    -0.56,                   !- Wind Pressure Coefficient Value 7 {dimensionless}",
        "    -0.56,                   !- Wind Pressure Coefficient Value 8 {dimensionless}",
        "    -0.42,                   !- Wind Pressure Coefficient Value 9 {dimensionless}",
        "    -0.37,                   !- Wind Pressure Coefficient Value 10 {dimensionless}",
        "    -0.42,                   !- Wind Pressure Coefficient Value 11 {dimensionless}",
        "    -0.56;                   !- Wind Pressure Coefficient Value 12 {dimensionless}",

        "  AirflowNetwork:MultiZone:WindPressureCoefficientValues,",
        "    SFacade_WPCValue,        !- Name",
        "    Every 30 Degrees,        !- AirflowNetwork:MultiZone:WindPressureCoefficientArray Name",
        "    -0.37,                   !- Wind Pressure Coefficient Value 1 {dimensionless}",
        "    -0.42,                   !- Wind Pressure Coefficient Value 2 {dimensionless}",
        "    -0.56,                   !- Wind Pressure Coefficient Value 3 {dimensionless}",
        "    -0.56,                   !- Wind Pressure Coefficient Value 4 {dimensionless}",
        "    0.04,                    !- Wind Pressure Coefficient Value 5 {dimensionless}",
        "    0.48,                    !- Wind Pressure Coefficient Value 6 {dimensionless}",
        "    0.60,                    !- Wind Pressure Coefficient Value 7 {dimensionless}",
        "    0.48,                    !- Wind Pressure Coefficient Value 8 {dimensionless}",
        "    0.04,                    !- Wind Pressure Coefficient Value 9 {dimensionless}",
        "    -0.56,                   !- Wind Pressure Coefficient Value 10 {dimensionless}",
        "    -0.56,                   !- Wind Pressure Coefficient Value 11 {dimensionless}",
        "    -0.42;                   !- Wind Pressure Coefficient Value 12 {dimensionless}",

        "  AirflowNetwork:MultiZone:WindPressureCoefficientValues,",
        "    WFacade_WPCValue,        !- Name",
        "    Every 30 Degrees,        !- AirflowNetwork:MultiZone:WindPressureCoefficientArray Name",
        "    -0.56,                   !- Wind Pressure Coefficient Value 1 {dimensionless}",
        "    -0.56,                   !- Wind Pressure Coefficient Value 2 {dimensionless}",
        "    -0.42,                   !- Wind Pressure Coefficient Value 3 {dimensionless}",
        "    -0.37,                   !- Wind Pressure Coefficient Value 4 {dimensionless}",
        "    -0.42,                   !- Wind Pressure Coefficient Value 5 {dimensionless}",
        "    -0.56,                   !- Wind Pressure Coefficient Value 6 {dimensionless}",
        "    -0.56,                   !- Wind Pressure Coefficient Value 7 {dimensionless}",
        "    0.04,                    !- Wind Pressure Coefficient Value 8 {dimensionless}",
        "    0.48,                    !- Wind Pressure Coefficient Value 9 {dimensionless}",
        "    0.60,                    !- Wind Pressure Coefficient Value 10 {dimensionless}",
        "    0.48,                    !- Wind Pressure Coefficient Value 11 {dimensionless}",
        "    0.04;                    !- Wind Pressure Coefficient Value 12 {dimensionless}",

        "  AirflowNetwork:Distribution:Node,",
        "    EquipmentInletNode,      !- Name",
        "    Zone Equipment Inlet Node,  !- Component Name or Node Name",
        "    Other,                   !- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Node,",
        "    SplitterNode,            !- Name",
        "    ,                        !- Component Name or Node Name",
        "    AirLoopHVAC:ZoneSplitter,!- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Node,",
        "    ZoneSupplyNode,          !- Name",
        "    ,                        !- Component Name or Node Name",
        "    Other,                   !- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Node,",
        "    ZoneSupplyRegisterNode,  !- Name",
        "    LIVING ZONE NoReheat Air Outlet Node,         !- Component Name or Node Name",
        "    Other,                   !- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Node,",
        "    ZoneOutletNode,          !- Name",
        "    Zone Outlet Node,        !- Component Name or Node Name",
        "    Other,                   !- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Node,",
        "    ZoneReturnNode,          !- Name",
        "    ,                        !- Component Name or Node Name",
        "    Other,                   !- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Node,",
        "    MixerNode,               !- Name",
        "    ,                        !- Component Name or Node Name",
        "    AirLoopHVAC:ZoneMixer,   !- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Node,",
        "    MainReturnNode,          !- Name",
        "    Return Air Mixer Outlet, !- Component Name or Node Name",
        "    Other,                   !- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Node,",
        "    MainInletNode,           !- Name",
        "    ,                        !- Component Name or Node Name",
        "    Other,                   !- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Node,",
        "    FanInletNode,            !- Name",
        "    Air Loop Inlet Node,     !- Component Name or Node Name",
        "    Other,                   !- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Node,",
        "    CoolingInletNode,        !- Name",
        "    Cooling Coil Air Inlet Node,  !- Component Name or Node Name",
        "    Other,                   !- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Node,",
        "    HeatingInletNode,        !- Name",
        "    Heating Coil Air Inlet Node,  !- Component Name or Node Name",
        "    Other,                   !- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Node,",
        "    SuppHeatingInletNode,    !- Name",
        "    SuppHeating Coil Air Inlet Node,  !- Component Name or Node Name",
        "    Other,                   !- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Node,",
        "    AirLoopOutletNode,       !- Name",
        "    Air Loop Outlet Node,    !- Component Name or Node Name",
        "    Other,                   !- Component Object Type or Node Type",
        "    3.0;                     !- Node Height {m}",

        "  AirflowNetwork:Distribution:Component:LeakageRatio,",
        "    SupplyLeak,              !- Name",
        "    0.1,                     !- Effective Leakage Ratio {dimensionless}",
        "    1.9,                     !- Maximum Flow Rate {m3/s}",
        "    60,                      !- Reference Pressure Difference {Pa}",
        "    0.65;                    !- Air Mass Flow Exponent {dimensionless}",

        "  AirflowNetwork:Distribution:Component:LeakageRatio,",
        "    ReturnLeak,              !- Name",
        "    0.05,                    !- Effective Leakage Ratio {dimensionless}",
        "    1.9,                     !- Maximum Flow Rate {m3/s}",
        "    15,                      !- Reference Pressure Difference {Pa}",
        "    0.65;                    !- Air Mass Flow Exponent {dimensionless}",

        "  AirflowNetwork:Distribution:Component:Duct,",
        "    MainTruck,               !- Name",
        "    2.0,                     !- Duct Length {m}",
        "    0.4064,                  !- Hydraulic Diameter {m}",
        "    0.1297,                  !- Cross Section Area {m2}",
        "    0.0009,                  !- Surface Roughness {m}",
        "    0.01,                    !- Coefficient for Local Dynamic Loss Due to Fitting {dimensionless}",
        "    0.946792,                !- Heat Transmittance Coefficient (U-Factor) for Duct Wall Construction {W/m2-K}",
        "    0.0000001,               !- Overall Moisture Transmittance Coefficient from Air to Air {kg/m2}",
        "    5.018000,                !- Outside Convection Coefficient {W/m2-K}",
        "    25.090000;               !- Inside Convection Coefficient {W/m2-K}",

        "  AirflowNetwork:Distribution:Component:Duct,",
        "    ZoneSupply,              !- Name",
        "    10.0,                    !- Duct Length {m}",
        "    0.4064,                  !- Hydraulic Diameter {m}",
        "    0.1297,                  !- Cross Section Area {m2}",
        "    0.0009,                  !- Surface Roughness {m}",
        "    0.91,                    !- Coefficient for Local Dynamic Loss Due to Fitting {dimensionless}",
        "    0.946792,                !- Heat Transmittance Coefficient (U-Factor) for Duct Wall Construction {W/m2-K}",
        "    0.0000001,               !- Overall Moisture Transmittance Coefficient from Air to Air {kg/m2}",
        "    5.018000,                !- Outside Convection Coefficient {W/m2-K}",
        "    25.090000;               !- Inside Convection Coefficient {W/m2-K}",

        "  AirflowNetwork:Distribution:Component:Duct,",
        "    ZoneReturn,              !- Name",
        "    3.0,                     !- Duct Length {m}",
        "    0.50,                    !- Hydraulic Diameter {m}",
        "    0.1963,                  !- Cross Section Area {m2}",
        "    0.0009,                  !- Surface Roughness {m}",
        "    0.01,                    !- Coefficient for Local Dynamic Loss Due to Fitting {dimensionless}",
        "    0.001226,                !- Heat Transmittance Coefficient (U-Factor) for Duct Wall Construction {W/m2-K}",
        "    0.0000001,               !- Overall Moisture Transmittance Coefficient from Air to Air {kg/m2}",
        "    0.006500,                !- Outside Convection Coefficient {W/m2-K}",
        "    0.032500;                !- Inside Convection Coefficient {W/m2-K}",

        "  AirflowNetwork:Distribution:Component:Duct,",
        "    ZoneConnectionDuct,      !- Name",
        "    0.1,                     !- Duct Length {m}",
        "    1.0,                     !- Hydraulic Diameter {m}",
        "    0.7854,                  !- Cross Section Area {m2}",
        "    0.0001,                  !- Surface Roughness {m}",
        "    0.00,                    !- Coefficient for Local Dynamic Loss Due to Fitting {dimensionless}",
        "    0.001226,                !- Heat Transmittance Coefficient (U-Factor) for Duct Wall Construction {W/m2-K}",
        "    0.0000001,               !- Overall Moisture Transmittance Coefficient from Air to Air {kg/m2}",
        "    0.006500,                !- Outside Convection Coefficient {W/m2-K}",
        "    0.032500;                !- Inside Convection Coefficient {W/m2-K}",

        "  AirflowNetwork:Distribution:Component:Duct,",
        "    MainReturn,              !- Name",
        "    1.0,                     !- Duct Length {m}",
        "    0.50,                    !- Hydraulic Diameter {m}",
        "    0.1963,                  !- Cross Section Area {m2}",
        "    0.0009,                  !- Surface Roughness {m}",
        "    0.01,                    !- Coefficient for Local Dynamic Loss Due to Fitting {dimensionless}",
        "    0.001226,                !- Heat Transmittance Coefficient (U-Factor) for Duct Wall Construction {W/m2-K}",
        "    0.0000001,               !- Overall Moisture Transmittance Coefficient from Air to Air {kg/m2}",
        "    0.006500,                !- Outside Convection Coefficient {W/m2-K}",
        "    0.032500;                !- Inside Convection Coefficient {W/m2-K}",

        "  AirflowNetwork:Distribution:Component:Duct,",
        "    AirLoopReturn,           !- Name",
        "    0.1,                     !- Duct Length {m}",
        "    1.00,                    !- Hydraulic Diameter {m}",
        "    0.7854,                  !- Cross Section Area {m2}",
        "    0.0001,                  !- Surface Roughness {m}",
        "    0.00,                    !- Coefficient for Local Dynamic Loss Due to Fitting {dimensionless}",
        "    0.001226,                !- Heat Transmittance Coefficient (U-Factor) for Duct Wall Construction {W/m2-K}",
        "    0.0000001,               !- Overall Moisture Transmittance Coefficient from Air to Air {kg/m2}",
        "    0.006500,                !- Outside Convection Coefficient {W/m2-K}",
        "    0.032500;                !- Inside Convection Coefficient {W/m2-K}",

        "  AirflowNetwork:Distribution:Component:Duct,",
        "    AirLoopSupply,           !- Name",
        "    0.1,                     !- Duct Length {m}",
        "    1.00,                    !- Hydraulic Diameter {m}",
        "    0.7854,                  !- Cross Section Area {m2}",
        "    0.0001,                  !- Surface Roughness {m}",
        "    0.00,                    !- Coefficient for Local Dynamic Loss Due to Fitting {dimensionless}",
        "    0.001226,                !- Heat Transmittance Coefficient (U-Factor) for Duct Wall Construction {W/m2-K}",
        "    0.0000001,               !- Overall Moisture Transmittance Coefficient from Air to Air {kg/m2}",
        "    0.006500,                !- Outside Convection Coefficient {W/m2-K}",
        "    0.032500;                !- Inside Convection Coefficient {W/m2-K}",

        "  AirflowNetwork:Distribution:Component:Fan,",
        "    Supply Fan 1,            !- Fan Name",
        "    Fan:SystemModel;         !- Supply Fan Object Type",

        "  AirflowNetwork:Distribution:Component:Coil,",
        "    ACDXCoil 1,              !- Coil Name",
        "    Coil:Cooling:DX:SingleSpeed,  !- Coil Object Type",
        "    0.1,                     !- Air Path Length {m}",
        "    1.000;                   !- Air Path Hydraulic Diameter {m}",

        "  AirflowNetwork:Distribution:Component:Coil,",
        "    Supp Heating Coil 1,     !- Coil Name",
        "    Coil:Heating:Fuel,       !- Coil Object Type",
        "    0.1,                     !- Air Path Length {m}",
        "    1.000;                   !- Air Path Hydraulic Diameter {m}",

        "  AirflowNetwork:Distribution:Component:Coil,",
        "    Heat Pump DX Heating Coil 1,  !- Coil Name",
        "    Coil:Heating:DX:SingleSpeed,  !- Coil Object Type",
        "    0.1,                     !- Air Path Length {m}",
        "    1.000;                   !- Air Path Hydraulic Diameter {m}",

        "  AirflowNetwork:Distribution:Linkage,",
        "    Main Link,               !- Name",
        "    EquipmentInletNode,      !- Node 1 Name",
        "    SplitterNode,            !- Node 2 Name",
        "    MainTruck,               !- Component Name",
        "    Attic Zone;              !- Thermal Zone Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    ZoneSupplyLink1,         !- Name",
        "    SplitterNode,            !- Node 1 Name",
        "    ZoneSupplyNode,          !- Node 2 Name",
        "    ZoneSupply,              !- Component Name",
        "    Attic Zone;              !- Thermal Zone Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    ZoneSupply1Link2,        !- Name",
        "    ZoneSupplyNode,          !- Node 1 Name",
        "    ZoneSupplyRegisterNode,  !- Node 2 Name",
        "    ZoneSupply,              !- Component Name",
        "    Attic Zone;              !- Thermal Zone Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    ZoneSupplyConnectionLink,!- Name",
        "    ZoneSupplyRegisterNode,  !- Node 1 Name",
        "    LIVING ZONE,             !- Node 2 Name",
        "    ZoneConnectionDuct;      !- Component Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    ZoneReturnConnectionLink,!- Name",
        "    LIVING ZONE,             !- Node 1 Name",
        "    ZoneOutletNode,          !- Node 2 Name",
        "    ZoneConnectionDuct;      !- Component Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    ZoneReturn1Link,         !- Name",
        "    ZoneOutletNode,          !- Node 1 Name",
        "    ZoneReturnNode,          !- Node 2 Name",
        "    ZoneReturn,              !- Component Name",
        "    Attic Zone;              !- Thermal Zone Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    ZoneReturn2Link,         !- Name",
        "    ZoneReturnNode,          !- Node 1 Name",
        "    MixerNode,               !- Node 2 Name",
        "    ZoneReturn,              !- Component Name",
        "    Garage Zone;             !- Thermal Zone Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    ReturnMixerLink,         !- Name",
        "    MixerNode,               !- Node 1 Name",
        "    MainReturnNode,          !- Node 2 Name",
        "    MainReturn,              !- Component Name",
        "    Garage Zone;             !- Thermal Zone Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    SystemReturnLink,        !- Name",
        "    MainReturnNode,          !- Node 1 Name",
        "    MainInletNode,           !- Node 2 Name",
        "    AirLoopReturn;           !- Component Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    SystemInletLink,         !- Name",
        "    MainInletNode,           !- Node 1 Name",
        "    FanInletNode,            !- Node 2 Name",
        "    MainReturn,              !- Component Name",
        "    GARAGE ZONE;             !- Thermal Zone Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    SupplyFanLink,           !- Name",
        "    FanInletNode,            !- Node 1 Name",
        "    CoolingInletNode,        !- Node 2 Name",
        "    Supply Fan 1;            !- Component Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    CoolingCoilLink,         !- Name",
        "    CoolingInletNode,        !- Node 1 Name",
        "    HeatingInletNode,        !- Node 2 Name",
        "    ACDXCoil 1;              !- Component Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    HeatingCoilLink,         !- Name",
        "    HeatingInletNode,        !- Node 1 Name",
        "    SuppHeatingInletNode,    !- Node 2 Name",
        "    Heat Pump DX Heating Coil 1;  !- Component Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    SuppHeatingCoilLink,     !- Name",
        "    SuppHeatingInletNode,    !- Node 1 Name",
        "    AirLoopOutletNode,       !- Node 2 Name",
        "    Supp Heating Coil 1;     !- Component Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    EquipmentAirLoopLink,    !- Name",
        "    AirLoopOutletNode,       !- Node 1 Name",
        "    EquipmentInletNode,      !- Node 2 Name",
        "    AirLoopSupply;           !- Component Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    ZoneSupplyLeakLink,      !- Name",
        "    ZoneSupplyNode,          !- Node 1 Name",
        "    Attic Zone,              !- Node 2 Name",
        "    SupplyLeak;              !- Component Name",

        "  AirflowNetwork:Distribution:Linkage,",
        "    ZoneReturnLeakLink,      !- Name",
        "    Garage Zone,             !- Node 1 Name",
        "    ZoneReturnNode,          !- Node 2 Name",
        "    ReturnLeak;              !- Component Name",

        "  AvailabilityManagerAssignmentList,",
        "    Reheat System 1 Avail List,  !- Name",
        "    AvailabilityManager:Scheduled,  !- Availability Manager 1 Object Type",
        "    Reheat System 1 Avail;   !- Availability Manager 1 Name",

        "  AvailabilityManager:Scheduled,",
        "    Reheat System 1 Avail,   !- Name",
        "    HVACAvailSched;          !- Schedule Name",

        "  ZoneHVAC:EquipmentConnections,",
        "    LIVING ZONE,             !- Zone Name",
        "    ZoneEquipment,           !- Zone Conditioning Equipment List Name",
        "    ZoneInlets,              !- Zone Air Inlet Node or NodeList Name",
        "    ,                        !- Zone Air Exhaust Node or NodeList Name",
        "    Zone Node,               !- Zone Air Node Name",
        "    Zone Outlet Node;        !- Zone Return Air Node or NodeList Name",

        "  ZoneHVAC:EquipmentList,",
        "    ZoneEquipment,           !- Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ZoneHVAC:AirDistributionUnit,  !- Zone Equipment 1 Object Type",
        "    LIVINGZONENoReheat,           !- Zone Equipment 1 Name",
        "    1,                       !- Zone Equipment 1 Cooling Sequence",
        "    1,                       !- Zone Equipment 1 Heating or No-Load Sequence",
        "    ,                        !- Zone Equipment 1 Sequential Cooling Load Fraction",
        "    ;                        !- Zone Equipment 1 Sequential Heating Load Fraction",

        "  AirLoopHVAC:UnitarySystem,",
        "    DXAC Heat Pump 1,      !- Name",
        "    Load,                !- Control Type",
        "    LIVING ZONE,             !- Controlling Zone or Thermostat Location",
        "    ,                        !- Dehumidification Control Type",
        "    HVACAvailSched,    !- Availability Schedule Name",
        "    Air Loop Inlet Node,     !- Air Inlet Node Name",
        "    Air Loop Outlet Node,    !- Air Outlet Node Name",
        "    Fan:SystemModel,               !- Supply Fan Object Type",
        "    Supply Fan 1,            !- Supply Fan Name",
        "    BlowThrough,             !- Fan Placement",
        "    CyclingFanSchedule,      !- Supply Air Fan Operating Mode Schedule Name",
        "    Coil:Heating:DX:SingleSpeed,  !- Heating Coil Object Type",
        "    Heat Pump DX Heating Coil 1,  !- Heating Coil Name",
        "    1.0,                        !- DX Heating Coil Sizing Ratio",
        "    Coil:Cooling:DX:SingleSpeed,   !- Cooling Coil Object Type",
        "    ACDXCoil 1,                        !- Cooling Coil Name",
        "    ,                        !- Use DOAS DX Cooling Coil",
        "    ,                        !- Minimum Supply Air Temperature",
        "    ,                        !- Latent Load Control",
        "    Coil:Heating:Fuel,       !- Supplemental Heating Coil Object Type",
        "    Supp Heating Coil 1,     !- Supplemental Heating Coil Name",
        "    SupplyAirFlowRate,       !- Cooling Supply Air Flow Rate Method",
        "    1.18,                    !- Cooling Supply Air Flow Rate",
        "    ,                        !- Cooling Supply Air Flow Rate Per Floor Area",
        "    ,                        !- Cooling Fraction of Autosized Cooling Supply Air Flow Rate",
        "    ,                        !- Cooling Supply Air Flow Rate Per Unit of Capacity",
        "    SupplyAirFlowRate,       !- Heating Supply Air Flow Rate Method",
        "    1.18,                    !- Heating Supply Air Flow Rate",
        "    ,                        !- Heating Supply Air Flow Rate Per Floor Area",
        "    ,                        !- Heating Fraction of Autosized Cooling Supply Air Flow Rate",
        "    ,                        !- Heating Supply Air Flow Rate Per Unit of Capacity",
        "    SupplyAirFlowRate,       !- No Load Supply Air Flow Rate Method",
        "    0;                       !- No Load Supply Air Flow Rate {m3/s}",

        "  ZoneHVAC:AirDistributionUnit,",
        "    LIVINGZONENoReheat,         !- Name",
        "    Zone 1 NoReheat Air Outlet Node,  !- Air Distribution Unit Outlet Node Name",
        "    AirTerminal:SingleDuct:ConstantVolume:NoReheat,  !- Air Terminal Object Type",
        "    No Reheat LIVING ZONE;           !- Air Terminal Name",

        "  AirTerminal:SingleDuct:ConstantVolume:NoReheat,",
        "    No Reheat LIVING ZONE,           !- Name",
        "    HVACAvailSched,    !- Availability Schedule Name",
        "    LIVING ZONE NoReheat Air Inlet Node,  !- Air Inlet Node Name",
        "    LIVING ZONE NoReheat Air Outlet Node,  !- Air Outlet Node Name",
        "    1.18;                    !- Maximum Air Flow Rate {m3/s}",

        "  ZoneControl:Thermostat,",
        "    Zone Thermostat,         !- Name",
        "    LIVING ZONE,             !- Zone or ZoneList Name",
        "    Dual Zone Control Type Sched,  !- Control Type Schedule Name",
        "    ThermostatSetpoint:DualSetpoint,  !- Control 1 Object Type",
        "    Setpoints;               !- Control 1 Name",

        "  ThermostatSetpoint:DualSetpoint,",
        "    Setpoints,               !- Name",
        "    Dual Heating Setpoints,  !- Heating Setpoint Temperature Schedule Name",
        "    Dual Cooling Setpoints;  !- Cooling Setpoint Temperature Schedule Name",

        "  AirLoopHVAC:SupplyPath,",
        "    TermReheatSupplyPath,    !- Name",
        "    Zone Equipment Inlet Node,  !- Supply Air Path Inlet Node Name",
        "    AirLoopHVAC:ZoneSplitter,!- Component 1 Object Type",
        "    Zone Supply Air Splitter;!- Component 1 Name",

        "  AirLoopHVAC:ReturnPath,",
        "    TermReheatReturnPath,    !- Name",
        "    Return Air Mixer Outlet, !- Return Air Path Outlet Node Name",
        "    AirLoopHVAC:ZoneMixer,   !- Component 1 Object Type",
        "    Zone Return Air Mixer;   !- Component 1 Name",

        "  AirLoopHVAC:ZoneSplitter,",
        "    Zone Supply Air Splitter,!- Name",
        "    Zone Equipment Inlet Node,  !- Inlet Node Name",
        "    LIVING ZONE NoReheat Air Inlet Node;         !- Outlet 1 Node Name",

        "  AirLoopHVAC:ZoneMixer,",
        "    Zone Return Air Mixer,   !- Name",
        "    Return Air Mixer Outlet, !- Outlet Node Name",
        "    Zone Outlet Node;        !- Inlet 1 Node Name",

        "  Coil:Heating:Fuel,",
        "    Supp Heating Coil 1,     !- Name",
        "    HVACAvailSched,          !- Availability Schedule Name",
        "    NaturalGas,              !- Fuel Type",
        "    0.8,                     !- Burner Efficiency",
        "    25000,                   !- Nominal Capacity {W}",
        "    SuppHeating Coil Air Inlet Node,  !- Air Inlet Node Name",
        "    Air Loop Outlet Node;    !- Air Outlet Node Name",

        "  Coil:Cooling:DX:SingleSpeed,",
        "    ACDXCoil 1,              !- Name",
        "    HVACAvailSched,          !- Availability Schedule Name",
        "    21000,                   !- Gross Rated Total Cooling Capacity {W}",
        "    0.8,                     !- Gross Rated Sensible Heat Ratio",
        "    3.0,                     !- Gross Rated Cooling COP {W/W}",
        "    1.18,                    !- Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    Cooling Coil Air Inlet Node,  !- Air Inlet Node Name",
        "    Heating Coil Air Inlet Node,  !- Air Outlet Node Name",
        "    WindACCoolCapFT,         !- Total Cooling Capacity Function of Temperature Curve Name",
        "    WindACCoolCapFFF,        !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "    WindACEIRFT,             !- Energy Input Ratio Function of Temperature Curve Name",
        "    WindACEIRFFF,            !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    WindACPLFFPLR;           !- Part Load Fraction Correlation Curve Name",

        "  Coil:Heating:DX:SingleSpeed,",
        "    Heat Pump DX Heating Coil 1,  !- Name",
        "    HVACAvailSched,          !- Availability Schedule Name",
        "    21000,                   !- Gross Rated Heating Capacity {W}",
        "    2.75,                    !- Gross Rated Heating COP {W/W}",
        "    1.18,                    !- Rated Air Flow Rate {m3/s}",
        "    ,                        !- 2017 Rated Supply Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Supply Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    Heating Coil Air Inlet Node,  !- Air Inlet Node Name",
        "    SuppHeating Coil Air Inlet Node,  !- Air Outlet Node Name",
        "    HPACHeatCapFT,           !- Heating Capacity Function of Temperature Curve Name",
        "    HPACHeatCapFFF,          !- Heating Capacity Function of Flow Fraction Curve Name",
        "    HPACHeatEIRFT,           !- Energy Input Ratio Function of Temperature Curve Name",
        "    HPACHeatEIRFFF,          !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "    HPACCOOLPLFFPLR,         !- Part Load Fraction Correlation Curve Name",
        "    ,                        !- Defrost Energy Input Ratio Function of Temperature Curve Name",
        "    -5.0,                    !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Outdoor Dry-Bulb Temperature to Turn On Compressor {C}",
        "    5.0,                     !- Maximum Outdoor Dry-Bulb Temperature for Defrost Operation {C}",
        "    200.0,                   !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10.0,                    !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    Resistive,               !- Defrost Strategy",
        "    TIMED,                   !- Defrost Control",
        "    0.166667,                !- Defrost Time Period Fraction",
        "    20000;                   !- Resistive Defrost Heater Capacity {W}",

        "  Fan:SystemModel,",
        "    Supply Fan 1,            !- Name",
        "    HVACAvailSched,          !- Availability Schedule Name",
        "    Air Loop Inlet Node,     !- Air Inlet Node Name",
        "    Cooling Coil Air Inlet Node,  !- Air Outlet Node Name",
        "    1.18,                    !- Design Maximum Air Flow Rate {m3/s}",
        "    Discrete,                !- Speed Control Method",
        "    0.0,                     !- Electric Power Minimum Flow Rate Fraction",
        "    400.0,                   !- Design Pressure Rise {Pa}",
        "    0.9,                   !- Motor Efficiency",
        "    1.0,                     !- Motor In Air Stream Fraction",
        "    674.29,                !- Design Electric Power Consumption {W}",
        "    TotalEfficiencyAndPressure,  !- Design Power Sizing Method",
        "    ,                        !- Electric Power Per Unit Flow Rate {W/(m3/s)}",
        "    ,                        !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}",
        "    0.7,                 !- Fan Total Efficiency",
        "    ,                        !- Electric Power Function of Flow Fraction Curve Name",
        "    ,                        !- Night Ventilation Mode Pressure Rise {Pa}",
        "    ,                        !- Night Ventilation Mode Flow Fraction",
        "    ,                        !- Motor Loss Zone Name",
        "    ,                        !- Motor Loss Radiative Fraction",
        "    Fan Energy;              !- End-Use Subcategory",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataIPShortCut->lNumericFieldBlanks.allocate(1000);
    state->dataIPShortCut->lAlphaFieldBlanks.allocate(1000);
    state->dataIPShortCut->cAlphaFieldNames.allocate(1000);
    state->dataIPShortCut->cNumericFieldNames.allocate(1000);
    state->dataIPShortCut->cAlphaArgs.allocate(1000);
    state->dataIPShortCut->rNumericArgs.allocate(1000);
    state->dataIPShortCut->lNumericFieldBlanks = false;
    state->dataIPShortCut->lAlphaFieldBlanks = false;
    state->dataIPShortCut->cAlphaFieldNames = " ";
    state->dataIPShortCut->cNumericFieldNames = " ";
    state->dataIPShortCut->cAlphaArgs = " ";
    state->dataIPShortCut->rNumericArgs = 0.0;

    bool ErrorsFound = false;
    // Read objects
    HeatBalanceManager::GetProjectControlData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    HeatBalanceManager::GetZoneData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    Material::GetWindowGlassSpectralData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    Material::GetMaterialData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    HeatBalanceManager::GetConstructData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);
    SurfaceGeometry::GetGeometryParameters(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    state->dataSurfaceGeometry->CosBldgRotAppGonly = 1.0;
    state->dataSurfaceGeometry->SinBldgRotAppGonly = 0.0;
    SurfaceGeometry::GetSurfaceData(*state, ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    // Read AirflowNetwork inputs
    state->afn->get_input();

    Sched::GetSchedule(*state, "WINDOWVENTSCHED")->currentVal = 1.0;
    Sched::GetSchedule(*state, "ACTIVITY SCH")->currentVal = 100.0;
    Sched::GetSchedule(*state, "WORK EFF SCH")->currentVal = 1.0;
    Sched::GetSchedule(*state, "CLOTHING SCH")->currentVal = 1.0;
    Sched::GetSchedule(*state, "AIR VELO SCH")->currentVal = 0.1;
    Sched::GetSchedule(*state, "HOUSE OCCUPANCY")->currentVal = 1.0;
    Sched::GetSchedule(*state, "INTERMITTENT")->currentVal = 1.0;
    Sched::GetSchedule(*state, "HOUSE LIGHTING")->currentVal = 1.0;
    Sched::GetSchedule(*state, "REPORTSCH")->currentVal = 1.0;
    Sched::GetSchedule(*state, "HVACAVAILSCHED")->currentVal = 1.0;
    Sched::GetSchedule(*state, "DUAL HEATING SETPOINTS")->currentVal = 21.0;
    Sched::GetSchedule(*state, "DUAL COOLING SETPOINTS")->currentVal = 25.0;
    Sched::GetSchedule(*state, "DUAL ZONE CONTROL TYPE SCHED")->currentVal = 1.0;
    Sched::GetSchedule(*state, "CYCLINGFANSCHEDULE")->currentVal = 1.0;

    state->afn->AirflowNetworkFanActivated = true;
    state->dataEnvrn->OutDryBulbTemp = -17.29025;
    state->dataEnvrn->OutHumRat = 0.0008389;
    state->dataEnvrn->OutBaroPress = 99063.0;
    state->dataEnvrn->WindSpeed = 4.9;
    state->dataEnvrn->WindDir = 270.0;

    for (i = 1; i <= 21; ++i) {
        state->afn->AirflowNetworkNodeSimu(i).TZ = 23.0;
        state->afn->AirflowNetworkNodeSimu(i).WZ = 0.0008400;
        if ((i >= 4 && i <= 7)) {
            state->afn->AirflowNetworkNodeSimu(i).TZ =
                DataEnvironment::OutDryBulbTempAt(*state, state->afn->AirflowNetworkNodeData(i).NodeHeight); // AirflowNetworkNodeData vals differ
            state->afn->AirflowNetworkNodeSimu(i).WZ = state->dataEnvrn->OutHumRat;
        }
    }
    state->dataAirLoop->AirLoopAFNInfo.allocate(1);
    state->dataAirLoop->AirLoopAFNInfo(1).LoopFanOperationMode = HVAC::FanOp::Cycling;
    state->dataAirLoop->AirLoopAFNInfo(1).LoopOnOffFanPartLoadRatio = 0.0;
    state->dataAirLoop->AirLoopAFNInfo(1).LoopSystemOnMassFlowrate = 1.23;
    state->afn->AirflowNetworkLinkageData(17).AirLoopNum = 1;
    state->dataLoopNodes->Node(4).MassFlowRate = 1.23;

    state->afn->calculate_balance();
    // Fan:SystemModel
    EXPECT_NEAR(1.23, state->afn->AirflowNetworkLinkSimu(20).FLOW, 0.0001);
    EXPECT_TRUE(state->afn->DisSysCompCVFData(1).FanModelFlag);

    for (i = 1; i <= 21; ++i) {
        state->afn->AirflowNetworkNodeSimu(i).TZ = 23.0;
        state->afn->AirflowNetworkNodeSimu(i).WZ = 0.0008400;
        if ((i >= 4 && i <= 7)) {
            state->afn->AirflowNetworkNodeSimu(i).TZ =
                DataEnvironment::OutDryBulbTempAt(*state, state->afn->AirflowNetworkNodeData(i).NodeHeight); // AirflowNetworkNodeData vals differ
            state->afn->AirflowNetworkNodeSimu(i).WZ = state->dataEnvrn->OutHumRat;
        }
    }
    // Fan:OnOff
    state->afn->DisSysCompCVFData(1).FanModelFlag = false;
    state->afn->calculate_balance();
    EXPECT_NEAR(1.23, state->afn->AirflowNetworkLinkSimu(20).FLOW, 0.0001);

    state->dataAirLoop->AirLoopAFNInfo.deallocate();
}

TEST_F(EnergyPlusFixture, AirflowNetwork_get_people_indexTest)
{
    // Unit test added for fix to Defect #11027
    int expectedIndexResult;
    int actualIndexResult;
    bool errFlag = false;

    state->afn->MultizoneSurfaceData.allocate(10);
    state->afn->MultizoneZoneData.allocate(3);
    state->dataHeatBal->TotPeople = 3;
    state->dataHeatBal->People.allocate(state->dataHeatBal->TotPeople);
    state->dataSurface->Surface.allocate(10);
    state->afn->AirflowNetworkNumOfZones = 3;

    state->dataHeatBal->People(1).ZonePtr = 2;
    state->dataHeatBal->People(1).AdaptiveASH55 = true;
    state->dataHeatBal->People(1).AdaptiveCEN15251 = false;
    state->dataHeatBal->People(2).ZonePtr = 3;
    state->dataHeatBal->People(2).AdaptiveASH55 = false;
    state->dataHeatBal->People(2).AdaptiveCEN15251 = true;
    state->dataHeatBal->People(3).ZonePtr = 1;
    state->dataHeatBal->People(3).AdaptiveASH55 = false;
    state->dataHeatBal->People(3).AdaptiveCEN15251 = false;

    state->afn->MultizoneZoneData(1).ZoneNum = 3;
    state->afn->MultizoneZoneData(2).ZoneNum = 1;
    state->afn->MultizoneZoneData(3).ZoneNum = 2;

    state->afn->MultizoneSurfaceData(1).ZonePtr = 1;
    state->afn->MultizoneSurfaceData(2).ZonePtr = 2;
    state->afn->MultizoneSurfaceData(3).ZonePtr = 3;
    state->afn->MultizoneSurfaceData(4).ZonePtr = 1;
    state->afn->MultizoneSurfaceData(5).ZonePtr = 2;
    state->afn->MultizoneSurfaceData(6).ZonePtr = 3;
    state->afn->MultizoneSurfaceData(7).ZonePtr = 1;
    state->afn->MultizoneSurfaceData(8).ZonePtr = 2;
    state->afn->MultizoneSurfaceData(9).ZonePtr = 3;
    state->afn->MultizoneSurfaceData(10).ZonePtr = 1;

    // Tests S1A/C: Surface--AFN Surface 1 (points to Zone 1 which is People 3 which does not use ASH55 or CEN15251)
    expectedIndexResult = 0;
    actualIndexResult = state->afn->get_people_index(state->afn->MultizoneSurfaceData(1).ZonePtr, VentControlType::ASH55, errFlag);
    EXPECT_EQ(expectedIndexResult, actualIndexResult);
    EXPECT_TRUE(errFlag);
    errFlag = false; // reset
    expectedIndexResult = 0;
    actualIndexResult = state->afn->get_people_index(state->afn->MultizoneSurfaceData(1).ZonePtr, VentControlType::CEN15251, errFlag);
    EXPECT_EQ(expectedIndexResult, actualIndexResult);
    EXPECT_TRUE(errFlag);
    errFlag = false; // reset

    // Tests S2A/C: Surface--AFN Surface 2 (points to Zone 2 which is People 1 which uses ASH55)
    expectedIndexResult = 1;
    actualIndexResult = state->afn->get_people_index(state->afn->MultizoneSurfaceData(2).ZonePtr, VentControlType::ASH55, errFlag);
    EXPECT_EQ(expectedIndexResult, actualIndexResult);
    EXPECT_FALSE(errFlag);
    expectedIndexResult = 0;
    actualIndexResult = state->afn->get_people_index(state->afn->MultizoneSurfaceData(2).ZonePtr, VentControlType::CEN15251, errFlag);
    EXPECT_EQ(expectedIndexResult, actualIndexResult);
    EXPECT_TRUE(errFlag);
    errFlag = false; // reset

    // Tests S3A/C: Surface--AFN Surface 3 (points to Zone 3 which is People 2 which uses CEN15251)    expectedIndexResult = 0;
    actualIndexResult = state->afn->get_people_index(state->afn->MultizoneSurfaceData(3).ZonePtr, VentControlType::ASH55, errFlag);
    EXPECT_EQ(expectedIndexResult, actualIndexResult);
    EXPECT_TRUE(errFlag);
    errFlag = false; // reset
    expectedIndexResult = 2;
    actualIndexResult = state->afn->get_people_index(state->afn->MultizoneSurfaceData(3).ZonePtr, VentControlType::CEN15251, errFlag);
    EXPECT_EQ(expectedIndexResult, actualIndexResult);
    EXPECT_FALSE(errFlag);

    // Tests S10C: Surface--AFN Surface 10 (points to Zone 2 but RAFN is set so this goes to Zone 1 which is People 3 which is rest to use CEN15251)
    state->dataHeatBal->People(3).AdaptiveCEN15251 = true;
    expectedIndexResult = 3;
    actualIndexResult = state->afn->get_people_index(state->afn->MultizoneSurfaceData(10).ZonePtr, VentControlType::CEN15251, errFlag);
    EXPECT_EQ(expectedIndexResult, actualIndexResult);
    EXPECT_FALSE(errFlag);
    state->dataHeatBal->People(3).AdaptiveCEN15251 = false;

    // Tests Z1A/C: Zone--Array 1 (points to Zone 3 which is People 2 which uses CEN15251)
    expectedIndexResult = 0;
    actualIndexResult = state->afn->get_people_index(state->afn->MultizoneZoneData(1).ZoneNum, VentControlType::ASH55, errFlag);
    EXPECT_EQ(expectedIndexResult, actualIndexResult);
    EXPECT_TRUE(errFlag);
    errFlag = false; // reset
    expectedIndexResult = 2;
    actualIndexResult = state->afn->get_people_index(state->afn->MultizoneZoneData(1).ZoneNum, VentControlType::CEN15251, errFlag);
    EXPECT_EQ(expectedIndexResult, actualIndexResult);
    EXPECT_FALSE(errFlag);

    // Tests Z2A/C: Zone--Array 2 (points to Zone 1 which is People 3 which uses neither ASH55 nor CEN15251)
    expectedIndexResult = 0;
    actualIndexResult = state->afn->get_people_index(state->afn->MultizoneZoneData(2).ZoneNum, VentControlType::ASH55, errFlag);
    EXPECT_EQ(expectedIndexResult, actualIndexResult);
    EXPECT_TRUE(errFlag);
    errFlag = false; // reset
    actualIndexResult = state->afn->get_people_index(state->afn->MultizoneZoneData(2).ZoneNum, VentControlType::CEN15251, errFlag);
    EXPECT_EQ(expectedIndexResult, actualIndexResult);
    EXPECT_TRUE(errFlag);
    errFlag = false; // reset

    // Tests Z3A/C: Zone--Array 3 (points to Zone 2 which is People 1 which uses ASH55)
    expectedIndexResult = 1;
    actualIndexResult = state->afn->get_people_index(state->afn->MultizoneZoneData(3).ZoneNum, VentControlType::ASH55, errFlag);
    EXPECT_EQ(expectedIndexResult, actualIndexResult);
    EXPECT_FALSE(errFlag);
    expectedIndexResult = 0;
    actualIndexResult = state->afn->get_people_index(state->afn->MultizoneZoneData(3).ZoneNum, VentControlType::CEN15251, errFlag);
    EXPECT_EQ(expectedIndexResult, actualIndexResult);
    EXPECT_TRUE(errFlag);
    errFlag = false; // reset (not really necessary unless more tests are added)
}

} // namespace EnergyPlus
