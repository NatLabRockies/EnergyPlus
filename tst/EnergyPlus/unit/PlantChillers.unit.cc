// EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the University of Illinois,
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

// EnergyPlus::Standalone ERV Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantChillers.hh>
#include <EnergyPlus/PlantUtilities.hh>

using namespace EnergyPlus;
using namespace PlantChillers;

TEST_F(EnergyPlusFixture, GTChiller_HeatRecoveryAutosizeTest)
{
    state->init_state(*state);

    // This needs to go first so that the plantLoc objects can reference it
    state->dataPlnt->PlantLoop.allocate(2);
    state->dataSize->PlantSizData.allocate(1);
    state->dataPlnt->PlantLoop(1).PlantSizNum = 1;
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataSize->PlantSizData(1).DesVolFlowRate = 1.0;
    state->dataSize->PlantSizData(1).DeltaT = 5.0;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;

    // unit test for autosizing heat recovery in Chiller:CombustionTurbine
    state->dataPlantChillers->GTChiller.allocate(1);

    state->dataPlantChillers->GTChiller(1).SizFac = 1.0;
    state->dataPlantChillers->GTChiller(1).DesignHeatRecVolFlowRateWasAutoSized = true;
    state->dataPlantChillers->GTChiller(1).HeatRecCapacityFraction = 0.5;
    state->dataPlantChillers->GTChiller(1).HeatRecActive = true;
    state->dataPlantChillers->GTChiller(1).CondenserType = DataPlant::CondenserType::WaterCooled;
    state->dataPlantChillers->GTChiller(1).CWPlantLoc.loopNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataPlantChillers->GTChiller(1).CWPlantLoc);
    state->dataPlantChillers->GTChiller(1).CDPlantLoc.loopNum = 2;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataPlantChillers->GTChiller(1).CDPlantLoc);
    state->dataPlantChillers->GTChiller(1).EvapVolFlowRate = 1.0;
    state->dataPlantChillers->GTChiller(1).CondVolFlowRate = 1.0;
    state->dataPlantChillers->GTChiller(1).NomCap = 10000;
    state->dataPlantChillers->GTChiller(1).COP = 3.0;
    state->dataPlantChillers->GTChiller(1).engineCapacityScalar = 1.0;

    // now call sizing routine
    state->dataPlantChillers->GTChiller(1).size(*state);
    // see if heat recovery flow rate is as expected
    EXPECT_NEAR(state->dataPlantChillers->GTChiller(1).DesignHeatRecVolFlowRate, 0.5, 0.00001);

    state->dataPlantChillers->GTChiller.deallocate();
    state->dataSize->PlantSizData.deallocate();
    state->dataPlnt->PlantLoop.deallocate();
}

TEST_F(EnergyPlusFixture, EngineDrivenChiller_HeatRecoveryAutosizeTest)
{
    state->init_state(*state);
    // unit test for autosizing heat recovery in Chiller:EngineDriven
    // This needs to go first so that the plantLoc can reference it
    state->dataPlnt->PlantLoop.allocate(2);
    state->dataSize->PlantSizData.allocate(1);
    state->dataPlnt->PlantLoop(1).PlantSizNum = 1;
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataSize->PlantSizData(1).DesVolFlowRate = 1.0;
    state->dataSize->PlantSizData(1).DeltaT = 5.0;
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;

    state->dataPlantChillers->EngineDrivenChiller.allocate(1);

    state->dataPlantChillers->EngineDrivenChiller(1).SizFac = 1.0;
    state->dataPlantChillers->EngineDrivenChiller(1).DesignHeatRecVolFlowRateWasAutoSized = true;
    state->dataPlantChillers->EngineDrivenChiller(1).HeatRecCapacityFraction = 0.5;
    state->dataPlantChillers->EngineDrivenChiller(1).HeatRecActive = true;
    state->dataPlantChillers->EngineDrivenChiller(1).CondenserType = DataPlant::CondenserType::WaterCooled;
    state->dataPlantChillers->EngineDrivenChiller(1).CWPlantLoc.loopNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataPlantChillers->EngineDrivenChiller(1).CWPlantLoc);
    state->dataPlantChillers->EngineDrivenChiller(1).CDPlantLoc.loopNum = 2;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataPlantChillers->EngineDrivenChiller(1).CDPlantLoc);
    state->dataPlantChillers->EngineDrivenChiller(1).EvapVolFlowRate = 1.0;
    state->dataPlantChillers->EngineDrivenChiller(1).CondVolFlowRate = 1.0;
    state->dataPlantChillers->EngineDrivenChiller(1).NomCap = 10000;
    state->dataPlantChillers->EngineDrivenChiller(1).COP = 3.0;

    // now call sizing routine
    state->dataPlantChillers->EngineDrivenChiller(1).size(*state);
    // see if heat recovery flow rate is as expected
    EXPECT_NEAR(state->dataPlantChillers->EngineDrivenChiller(1).DesignHeatRecVolFlowRate, 0.5, 0.00001);

    state->dataPlantChillers->EngineDrivenChiller.deallocate();
    state->dataSize->PlantSizData.deallocate();
    state->dataPlnt->PlantLoop.deallocate();
}

TEST_F(EnergyPlusFixture, EngineDrivenChiller_Fueltype)
{
    std::string const idf_objects = delimited_string({
        "Chiller:EngineDriven,",
        "  Big Chiller,             !- Name",
        "  WaterCooled,             !- Condenser Type",
        "  100000,                  !- Nominal Capacity {W}",
        "  2.75,                    !- Nominal COP {W/W}",
        "  Big Chiller Inlet Node,  !- Chilled Water Inlet Node Name",
        "  Big Chiller Outlet Node, !- Chilled Water Outlet Node Name",
        "  Big Chiller Condenser Inlet Node,  !- Condenser Inlet Node Name",
        "  Big Chiller Condenser Outlet Node,  !- Condenser Outlet Node Name",
        "  0.15,                    !- Minimum Part Load Ratio",
        "  1.0,                     !- Maximum Part Load Ratio",
        "  0.65,                    !- Optimum Part Load Ratio",
        "  35.0,                    !- Design Condenser Inlet Temperature {C}",
        "  2.778,                   !- Temperature Rise Coefficient",
        "  6.67,                    !- Design Chilled Water Outlet Temperature {C}",
        "  0.0011,                  !- Design Chilled Water Flow Rate {m3/s}",
        "  0.0011,                  !- Design Condenser Water Flow Rate {m3/s}",
        "  0.9949,                  !- Coefficient 1 of Capacity Ratio Curve",
        "  -0.045954,               !- Coefficient 2 of Capacity Ratio Curve",
        "  -0.0013543,              !- Coefficient 3 of Capacity Ratio Curve",
        "  2.333,                   !- Coefficient 1 of Power Ratio Curve",
        "  -1.975,                  !- Coefficient 2 of Power Ratio Curve",
        "  0.6121,                  !- Coefficient 3 of Power Ratio Curve",
        "  0.03303,                 !- Coefficient 1 of Full Load Ratio Curve",
        "  0.6852,                  !- Coefficient 2 of Full Load Ratio Curve",
        "  0.2818,                  !- Coefficient 3 of Full Load Ratio Curve",
        "  5,                       !- Chilled Water Outlet Temperature Lower Limit {C}",
        "  Fuel Use Curve,          !- Fuel Use Curve Name",
        "  Jacket Heat Recovery Curve,  !- Jacket Heat Recovery Curve Name",
        "  Lube Heat Recovery Curve,!- Lube Heat Recovery Curve Name",
        "  Total Exhaust Energy Curve,  !- Total Exhaust Energy Curve Name",
        "  Exhaust Temperature Curve,  !- Exhaust Temperature Curve Name",
        "  0.01516,                 !- Coefficient 1 of U-Factor Times Area Curve",
        "  0.9,                     !- Coefficient 2 of U-Factor Times Area Curve",
        "  0.00063,                 !- Maximum Exhaust Flow per Unit of Power Output {(kg/s)/W}",
        "  150,                     !- Design Minimum Exhaust Temperature {C}",
        "  Diesel,                  !- Fuel Type",
        "  45500,                   !- Fuel Higher Heating Value {kJ/kg}",
        "  0.0,                     !- Design Heat Recovery Water Flow Rate {m3/s}",
        "  ,                   !- Heat Recovery Inlet Node Name",
        "  ,                  !- Heat Recovery Outlet Node Name",
        "  LeavingSetpointModulated,!- Chiller Flow Mode",
        "  60.0,                    !- Maximum Temperature for Heat Recovery at Heat Recovery Outlet Node {C}",
        " ;                       !- Sizing Factor",

        " Curve:Quadratic,",
        "  Fuel Use Curve,          !- Name",
        "  1.3,                     !- Coefficient1 Constant",
        "  0.6318,                  !- Coefficient2 x",
        "  -0.4165,                 !- Coefficient3 x**2",
        "  0,                       !- Minimum Value of x",
        "  1;                       !- Maximum Value of x",

        " Curve:Quadratic,",
        "  Jacket Heat Recovery Curve,  !- Name",
        "  0.25,                    !- Coefficient1 Constant",
        "  0,                       !- Coefficient2 x",
        "  0,                       !- Coefficient3 x**2",
        "  0,                       !- Minimum Value of x",
        "  1;                       !- Maximum Value of x",

        " Curve:Quadratic,",
        "  Lube Heat Recovery Curve,!- Name",
        "  0.15,                    !- Coefficient1 Constant",
        "  0,                       !- Coefficient2 x",
        "  0,                       !- Coefficient3 x**2",
        "  0,                       !- Minimum Value of x",
        "  1;                       !- Maximum Value of x",

        " Curve:Quadratic,",
        "  Total Exhaust Energy Curve,  !- Name",
        "  0.1,                     !- Coefficient1 Constant",
        "  0,                       !- Coefficient2 x",
        "  0,                       !- Coefficient3 x**2",
        "  0,                       !- Minimum Value of x",
        "  1;                       !- Maximum Value of x",

        " Curve:Quadratic,",
        "  Exhaust Temperature Curve,  !- Name",
        "  392.4,                   !- Coefficient1 Constant",
        "  33.33,                   !- Coefficient2 x",
        "  0,                       !- Coefficient3 x**2",
        "  0,                       !- Minimum Value of x",
        "  1;                       !- Maximum Value of x",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    EngineDrivenChillerSpecs::getInput(*state);

    EXPECT_EQ(1, state->dataPlantChillers->NumEngineDrivenChillers);
    EXPECT_ENUM_EQ(state->dataPlantChillers->EngineDrivenChiller(1).FuelType, Constant::eFuel::Diesel);
}

TEST_F(EnergyPlusFixture, CombustionTurbineChiller_Fueltype)
{
    std::string const idf_objects = delimited_string({
        "Chiller:CombustionTurbine,",
        "  Big Chiller,             !- Name",
        "  WaterCooled,             !- Condenser Type",
        "  30000,                   !- Nominal Capacity {W}",
        "  2.75,                    !- Nominal COP {W/W}",
        "  Big Chiller Inlet Node,  !- Chilled Water Inlet Node Name",
        "  Big Chiller Outlet Node, !- Chilled Water Outlet Node Name",
        "  Big Chiller Condenser Inlet Node,  !- Condenser Inlet Node Name",
        "  Big Chiller Condenser Outlet Node,  !- Condenser Outlet Node Name",
        "  0.15,                    !- Minimum Part Load Ratio",
        "  1.0,                     !- Maximum Part Load Ratio",
        "  0.65,                    !- Optimum Part Load Ratio",
        "  35.0,                    !- Design Condenser Inlet Temperature {C}",
        "  2.778,                   !- Temperature Rise Coefficient",
        "  6.67,                    !- Design Chilled Water Outlet Temperature {C}",
        "  0.0011,                  !- Design Chilled Water Flow Rate {m3/s}",
        "  0.0011,                  !- Design Condenser Water Flow Rate {m3/s}",
        "  0.9949,                  !- Coefficient 1 of Capacity Ratio Curve",
        "  -0.045954,               !- Coefficient 2 of Capacity Ratio Curve",
        "  -0.0013543,              !- Coefficient 3 of Capacity Ratio Curve",
        "  2.333,                   !- Coefficient 1 of Power Ratio Curve",
        "  -1.975,                  !- Coefficient 2 of Power Ratio Curve",
        "  0.6121,                  !- Coefficient 3 of Power Ratio Curve",
        "  0.03303,                 !- Coefficient 1 of Full Load Ratio Curve",
        "  0.6852,                  !- Coefficient 2 of Full Load Ratio Curve",
        "  0.2818,                  !- Coefficient 3 of Full Load Ratio Curve",
        "  5,                       !- Chilled Water Outlet Temperature Lower Limit {C}",
        "  9.41,                    !- Coefficient 1 of Fuel Input Curve",
        "  -9.48,                   !- Coefficient 2 of Fuel Input Curve",
        "  4.32,                    !- Coefficient 3 of Fuel Input Curve",
        "  1.0044,                  !- Coefficient 1 of Temperature Based Fuel Input Curve",
        "  -0.0008,                 !- Coefficient 2 of Temperature Based Fuel Input Curve",
        "  0,                       !- Coefficient 3 of Temperature Based Fuel Input Curve",
        "  15.63518363,             !- Coefficient 1 of Exhaust Flow Curve",
        "  -0.03059999,             !- Coefficient 2 of Exhaust Flow Curve",
        "  -0.0002,                 !- Coefficient 3 of Exhaust Flow Curve",
        "  916.992,                 !- Coefficient 1 of Exhaust Gas Temperature Curve",
        "  307.998,                 !- Coefficient 2 of Exhaust Gas Temperature Curve",
        "  79.992,                  !- Coefficient 3 of Exhaust Gas Temperature Curve",
        "  1.005,                   !- Coefficient 1 of Temperature Based Exhaust Gas Temperature Curve",
        "  0.0018,                  !- Coefficient 2 of Temperature Based Exhaust Gas Temperature Curve",
        "  0,                       !- Coefficient 3 of Temperature Based Exhaust Gas Temperature Curve",
        "  0.223,                   !- Coefficient 1 of Recovery Lube Heat Curve",
        "  -0.4,                    !- Coefficient 2 of Recovery Lube Heat Curve",
        "  0.2286,                  !- Coefficient 3 of Recovery Lube Heat Curve",
        "  0.01907045,              !- Coefficient 1 of U-Factor Times Area Curve",
        "  0.9,                     !- Coefficient 2 of U-Factor Times Area Curve",
        "  50000,                   !- Gas Turbine Engine Capacity {W}",
        "  0.00000504,              !- Maximum Exhaust Flow per Unit of Power Output {(kg/s)/W}",
        "  150,                     !- Design Steam Saturation Temperature {C}",
        "  43500,                   !- Fuel Higher Heating Value {kJ/kg}",
        "  0.0,                     !- Design Heat Recovery Water Flow Rate {m3/s}",
        "  ,                        !- Heat Recovery Inlet Node Name",
        "  ,                        !- Heat Recovery Outlet Node Name",
        "  LeavingSetpointModulated,!- Chiller Flow Mode",
        "  NATURALGAS,              !- Fuel Type",
        "  80.0;                    !- Heat Recovery Maximum Temperature {C}",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GTChillerSpecs::getInput(*state);

    EXPECT_EQ(1, state->dataPlantChillers->NumGTChillers);
    EXPECT_ENUM_EQ(state->dataPlantChillers->GTChiller(1).FuelType, Constant::eFuel::NaturalGas);
}
