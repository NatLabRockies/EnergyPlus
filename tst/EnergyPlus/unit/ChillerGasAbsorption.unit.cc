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

// Google Test Headers
#include <gtest/gtest.h>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/ChillerGasAbsorption.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;
using namespace EnergyPlus::ChillerGasAbsorption;

TEST_F(EnergyPlusFixture, GasAbsorption_GetInput_Test)
{
    std::string const idf_objects = delimited_string({
        "  ChillerHeater:Absorption:DirectFired,                                                                      ",
        "    Big Chiller,             !- Name                                                                         ",
        "    100000,                  !- Nominal Cooling Capacity {W}                                                 ",
        "    0.8,                     !- Heating to Cooling Capacity Ratio                                            ",
        "    0.97,                    !- Fuel Input to Cooling Output Ratio                                           ",
        "    1.25,                    !- Fuel Input to Heating Output Ratio                                           ",
        "    0.01,                    !- Electric Input to Cooling Output Ratio                                       ",
        "    0.005,                   !- Electric Input to Heating Output Ratio                                       ",
        "    Big Chiller Inlet Node,  !- Chilled Water Inlet Node Name                                                ",
        "    Big Chiller Outlet Node, !- Chilled Water Outlet Node Name                                               ",
        "    Big Chiller Condenser Inlet Node,  !- Condenser Inlet Node Name                                          ",
        "    Big Chiller Condenser Outlet Node,  !- Condenser Outlet Node Name                                        ",
        "    Purchased Heat Inlet Node,  !- Hot Water Inlet Node Name                                                 ",
        "    Purchased Heat Outlet Node,  !- Hot Water Outlet Node Name                                               ",
        "    0.000001,                !- Minimum Part Load Ratio                                                      ",
        "    1.0,                     !- Maximum Part Load Ratio                                                      ",
        "    0.6,                     !- Optimum Part Load Ratio                                                      ",
        "    29,                      !- Design Entering Condenser Water Temperature {C}                              ",
        "    7,                       !- Design Leaving Chilled Water Temperature {C}                                 ",
        "    0.0011,                  !- Design Chilled Water Flow Rate {m3/s}                                        ",
        "    0.0011,                  !- Design Condenser Water Flow Rate {m3/s}                                      ",
        "    0.0043,                  !- Design Hot Water Flow Rate {m3/s}                                            ",
        "    GasAbsFlatBiQuad,        !- Cooling Capacity Function of Temperature Curve Name                          ",
        "    GasAbsFlatBiQuad,        !- Fuel Input to Cooling Output Ratio Function of Temperature Curve Name        ",
        "    GasAbsLinearQuad,        !- Fuel Input to Cooling Output Ratio Function of Part Load Ratio Curve Name    ",
        "    GasAbsFlatBiQuad,        !- Electric Input to Cooling Output Ratio Function of Temperature Curve Name    ",
        "    GasAbsFlatQuad,          !- Electric Input to Cooling Output Ratio Function of Part Load Ratio Curve Name",
        "    GasAbsInvLinearQuad,     !- Heating Capacity Function of Cooling Capacity Curve Name                     ",
        "    GasAbsLinearQuad,        !- Fuel Input to Heat Output Ratio During Heating Only Operation Curve Name     ",
        "    EnteringCondenser,       !- Temperature Curve Input Variable                                             ",
        "    AirCooled,               !- Condenser Type                                                               ",
        "    2,                       !- Chilled Water Temperature Lower Limit {C}                                    ",
        "    0,                       !- Fuel Higher Heating Value {kJ/kg}                                            ",
        "    NaturalGas,              !- Fuel Type                                                                    ",
        "    ;                        !- Sizing Factor                                                                ",
        "                                                                                                             ",
        "  Curve:Biquadratic,                                                                                         ",
        "    GasAbsFlatBiQuad,        !- Name                                                                         ",
        "    1.000000000,             !- Coefficient1 Constant                                                        ",
        "    0.000000000,             !- Coefficient2 x                                                               ",
        "    0.000000000,             !- Coefficient3 x**2                                                            ",
        "    0.000000000,             !- Coefficient4 y                                                               ",
        "    0.000000000,             !- Coefficient5 y**2                                                            ",
        "    0.000000000,             !- Coefficient6 x*y                                                             ",
        "    0.,                      !- Minimum Value of x                                                           ",
        "    50.,                     !- Maximum Value of x                                                           ",
        "    0.,                      !- Minimum Value of y                                                           ",
        "    50.;                     !- Maximum Value of y                                                           ",
        "                                                                                                             ",
        "  Curve:Quadratic,                                                                                           ",
        "    GasAbsFlatQuad,          !- Name                                                                         ",
        "    1.000000000,             !- Coefficient1 Constant                                                        ",
        "    0.000000000,             !- Coefficient2 x                                                               ",
        "    0.000000000,             !- Coefficient3 x**2                                                            ",
        "    0.,                      !- Minimum Value of x                                                           ",
        "    50.;                     !- Maximum Value of x                                                           ",
        "                                                                                                             ",
        "  Curve:Quadratic,                                                                                           ",
        "    GasAbsLinearQuad,        !- Name                                                                         ",
        "    0.000000000,             !- Coefficient1 Constant                                                        ",
        "    1.000000000,             !- Coefficient2 x                                                               ",
        "    0.000000000,             !- Coefficient3 x**2                                                            ",
        "    0.,                      !- Minimum Value of x                                                           ",
        "    50.;                     !- Maximum Value of x                                                           ",
        "                                                                                                             ",
        "  Curve:Quadratic,                                                                                           ",
        "    GasAbsInvLinearQuad,     !- Name                                                                         ",
        "    1.000000000,             !- Coefficient1 Constant                                                        ",
        "    -1.000000000,            !- Coefficient2 x                                                               ",
        "    0.000000000,             !- Coefficient3 x**2                                                            ",
        "    0.,                      !- Minimum Value of x                                                           ",
        "    50.;                     !- Maximum Value of x                                                           ",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetGasAbsorberInput(*state);

    EXPECT_EQ(1u, state->dataChillerGasAbsorption->GasAbsorber.size());
    EXPECT_EQ("BIG CHILLER", state->dataChillerGasAbsorption->GasAbsorber(1).Name);
    EXPECT_EQ(100000., state->dataChillerGasAbsorption->GasAbsorber(1).NomCoolingCap);
    EXPECT_EQ(0.8, state->dataChillerGasAbsorption->GasAbsorber(1).NomHeatCoolRatio);

    EXPECT_EQ(0.97, state->dataChillerGasAbsorption->GasAbsorber(1).FuelCoolRatio);
    EXPECT_EQ(1.25, state->dataChillerGasAbsorption->GasAbsorber(1).FuelHeatRatio);
    EXPECT_EQ(0.01, state->dataChillerGasAbsorption->GasAbsorber(1).ElecCoolRatio);
    EXPECT_EQ(0.005, state->dataChillerGasAbsorption->GasAbsorber(1).ElecHeatRatio);

    EXPECT_EQ(0.000001, state->dataChillerGasAbsorption->GasAbsorber(1).MinPartLoadRat);
    EXPECT_EQ(1.0, state->dataChillerGasAbsorption->GasAbsorber(1).MaxPartLoadRat);
    EXPECT_EQ(0.6, state->dataChillerGasAbsorption->GasAbsorber(1).OptPartLoadRat);

    EXPECT_EQ(29., state->dataChillerGasAbsorption->GasAbsorber(1).TempDesCondReturn);
    EXPECT_EQ(7., state->dataChillerGasAbsorption->GasAbsorber(1).TempDesCHWSupply);
    EXPECT_EQ(0.0011, state->dataChillerGasAbsorption->GasAbsorber(1).EvapVolFlowRate);
    EXPECT_EQ(0.0043, state->dataChillerGasAbsorption->GasAbsorber(1).HeatVolFlowRate);

    EXPECT_TRUE(state->dataChillerGasAbsorption->GasAbsorber(1).isEnterCondensTemp);
    EXPECT_FALSE(state->dataChillerGasAbsorption->GasAbsorber(1).isWaterCooled);

    EXPECT_EQ(2., state->dataChillerGasAbsorption->GasAbsorber(1).CHWLowLimitTemp);
    EXPECT_ENUM_EQ(Constant::eFuel::NaturalGas, state->dataChillerGasAbsorption->GasAbsorber(1).FuelType);
}

TEST_F(EnergyPlusFixture, GasAbsorption_getDesignCapacities_Test)
{
    state->init_state(*state);
    state->dataPlnt->TotNumLoops = 3;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 3;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(3);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 2;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(2);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = 100;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(2).NodeNumIn = 111;

    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 3;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(3);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 2;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(2);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = 200;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(2).NodeNumIn = 222;

    state->dataPlnt->PlantLoop(3).LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 4;
    state->dataPlnt->PlantLoop(3).LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(4);
    state->dataPlnt->PlantLoop(3).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 2;
    state->dataPlnt->PlantLoop(3).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(2);
    state->dataPlnt->PlantLoop(3).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = 300;
    state->dataPlnt->PlantLoop(3).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(2).NodeNumIn = 333;

    GasAbsorberSpecs thisChillerHeater;
    thisChillerHeater.ChillReturnNodeNum = 111;
    thisChillerHeater.HeatReturnNodeNum = 222;
    thisChillerHeater.CondReturnNodeNum = 333;

    PlantLocation loc_1 = PlantLocation(1, DataPlant::LoopSideLocation::Demand, 1, 1);
    PlantUtilities::SetPlantLocationLinks(*state, loc_1);

    Real64 maxload(-1.0);
    Real64 minload(-1.0);
    Real64 optload(-1.0);

    thisChillerHeater.NomCoolingCap = 100000.0;
    thisChillerHeater.MinPartLoadRat = 0.1;
    thisChillerHeater.MaxPartLoadRat = 0.9;
    thisChillerHeater.OptPartLoadRat = 0.8;

    // Chiller
    thisChillerHeater.getDesignCapacities(*state, loc_1, maxload, minload, optload);

    EXPECT_NEAR(minload, 10000.0, 0.001);
    EXPECT_NEAR(maxload, 90000.0, 0.001);
    EXPECT_NEAR(optload, 80000.0, 0.001);

    thisChillerHeater.NomHeatCoolRatio = 0.9;
    PlantLocation loc_2 = PlantLocation(2, DataPlant::LoopSideLocation::Demand, 1, 1);
    PlantUtilities::SetPlantLocationLinks(*state, loc_2);

    // Heater
    thisChillerHeater.getDesignCapacities(*state, loc_2, maxload, minload, optload);

    EXPECT_NEAR(minload, 9000.0, 0.001);
    EXPECT_NEAR(maxload, 81000.0, 0.001);
    EXPECT_NEAR(optload, 72000.0, 0.001);

    PlantLocation loc_3 = PlantLocation(3, DataPlant::LoopSideLocation::Demand, 1, 1);
    PlantUtilities::SetPlantLocationLinks(*state, loc_3);

    // Condenser
    thisChillerHeater.getDesignCapacities(*state, loc_3, maxload, minload, optload);

    EXPECT_NEAR(minload, 0.0, 0.001);
    EXPECT_NEAR(maxload, 0.0, 0.001);
    EXPECT_NEAR(optload, 0.0, 0.001);
}

TEST_F(EnergyPlusFixture, GasAbsorption_calculateHeater_Fix_Test)
{
    std::string const idf_objects = delimited_string({
        "  ChillerHeater:Absorption:DirectFired,                                                                      ",
        "    Big Chiller,             !- Name                                                                         ",
        "    100000,                  !- Nominal Cooling Capacity {W}                                                 ",
        "    0.8,                     !- Heating to Cooling Capacity Ratio                                            ",
        "    0.97,                    !- Fuel Input to Cooling Output Ratio                                           ",
        "    1.25,                    !- Fuel Input to Heating Output Ratio                                           ",
        "    0.01,                    !- Electric Input to Cooling Output Ratio                                       ",
        "    0.005,                   !- Electric Input to Heating Output Ratio                                       ",
        "    Big Chiller Inlet Node,  !- Chilled Water Inlet Node Name                                                ",
        "    Big Chiller Outlet Node, !- Chilled Water Outlet Node Name                                               ",
        "    Big Chiller Condenser Inlet Node,  !- Condenser Inlet Node Name                                          ",
        "    Big Chiller Condenser Outlet Node,  !- Condenser Outlet Node Name                                        ",
        "    Purchased Heat Inlet Node,  !- Hot Water Inlet Node Name                                                 ",
        "    Purchased Heat Outlet Node,  !- Hot Water Outlet Node Name                                               ",
        "    0.000001,                !- Minimum Part Load Ratio                                                      ",
        "    1.0,                     !- Maximum Part Load Ratio                                                      ",
        "    0.6,                     !- Optimum Part Load Ratio                                                      ",
        "    29,                      !- Design Entering Condenser Water Temperature {C}                              ",
        "    7,                       !- Design Leaving Chilled Water Temperature {C}                                 ",
        "    0.0011,                  !- Design Chilled Water Flow Rate {m3/s}                                        ",
        "    0.0011,                  !- Design Condenser Water Flow Rate {m3/s}                                      ",
        "    0.0043,                  !- Design Hot Water Flow Rate {m3/s}                                            ",
        "    GasAbsFlatBiQuad,        !- Cooling Capacity Function of Temperature Curve Name                          ",
        "    GasAbsFlatBiQuad,        !- Fuel Input to Cooling Output Ratio Function of Temperature Curve Name        ",
        "    GasAbsLinearQuad,        !- Fuel Input to Cooling Output Ratio Function of Part Load Ratio Curve Name    ",
        "    GasAbsFlatBiQuad,        !- Electric Input to Cooling Output Ratio Function of Temperature Curve Name    ",
        "    GasAbsFlatQuad,          !- Electric Input to Cooling Output Ratio Function of Part Load Ratio Curve Name",
        "    GasAbsInvLinearQuad,     !- Heating Capacity Function of Cooling Capacity Curve Name                     ",
        "    GasAbsLinearQuad,        !- Fuel Input to Heat Output Ratio During Heating Only Operation Curve Name     ",
        "    EnteringCondenser,       !- Temperature Curve Input Variable                                             ",
        "    AirCooled,               !- Condenser Type                                                               ",
        "    2,                       !- Chilled Water Temperature Lower Limit {C}                                    ",
        "    0,                       !- Fuel Higher Heating Value {kJ/kg}                                            ",
        "    NaturalGas,              !- Fuel Type                                                                    ",
        "    ;                        !- Sizing Factor                                                                ",
        "                                                                                                             ",
        "  Curve:Biquadratic,                                                                                         ",
        "    GasAbsFlatBiQuad,        !- Name                                                                         ",
        "    1.000000000,             !- Coefficient1 Constant                                                        ",
        "    0.000000000,             !- Coefficient2 x                                                               ",
        "    0.000000000,             !- Coefficient3 x**2                                                            ",
        "    0.000000000,             !- Coefficient4 y                                                               ",
        "    0.000000000,             !- Coefficient5 y**2                                                            ",
        "    0.000000000,             !- Coefficient6 x*y                                                             ",
        "    0.,                      !- Minimum Value of x                                                           ",
        "    50.,                     !- Maximum Value of x                                                           ",
        "    0.,                      !- Minimum Value of y                                                           ",
        "    50.;                     !- Maximum Value of y                                                           ",
        "                                                                                                             ",
        "  Curve:Quadratic,                                                                                           ",
        "    GasAbsFlatQuad,          !- Name                                                                         ",
        "    1.000000000,             !- Coefficient1 Constant                                                        ",
        "    0.000000000,             !- Coefficient2 x                                                               ",
        "    0.000000000,             !- Coefficient3 x**2                                                            ",
        "    0.,                      !- Minimum Value of x                                                           ",
        "    50.;                     !- Maximum Value of x                                                           ",
        "                                                                                                             ",
        "  Curve:Quadratic,                                                                                           ",
        "    GasAbsLinearQuad,        !- Name                                                                         ",
        "    0.000000000,             !- Coefficient1 Constant                                                        ",
        "    1.000000000,             !- Coefficient2 x                                                               ",
        "    0.000000000,             !- Coefficient3 x**2                                                            ",
        "    0.,                      !- Minimum Value of x                                                           ",
        "    50.;                     !- Maximum Value of x                                                           ",
        "                                                                                                             ",
        "  Curve:Quadratic,                                                                                           ",
        "    GasAbsInvLinearQuad,     !- Name                                                                         ",
        "    1.000000000,             !- Coefficient1 Constant                                                        ",
        "    -1.000000000,            !- Coefficient2 x                                                               ",
        "    0.000000000,             !- Coefficient3 x**2                                                            ",
        "    0.,                      !- Minimum Value of x                                                           ",
        "    50.;                     !- Maximum Value of x                                                           ",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    compare_err_stream("");
    state->init_state(*state);

    GetGasAbsorberInput(*state);

    auto &thisChillerHeater = state->dataChillerGasAbsorption->GasAbsorber(1);

    Real64 loadinput = 5000.0;
    bool runflaginput = true;

    thisChillerHeater.CoolingLoad = 100000.0;
    thisChillerHeater.CoolPartLoadRatio = 1.0;
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

    thisChillerHeater.HWplantLoc.loopNum = 1;
    thisChillerHeater.HWplantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    PlantUtilities::SetPlantLocationLinks(*state, thisChillerHeater.HWplantLoc);

    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).FlowLock = DataPlant::FlowLock::Locked;
    state->dataLoopNodes->Node(3).Temp = 60.0;
    state->dataLoopNodes->Node(3).MassFlowRate = 0.5;
    state->dataLoopNodes->Node(4).TempSetPoint = 70.0;

    thisChillerHeater.calculateHeater(*state, loadinput, runflaginput);

    EXPECT_NEAR(thisChillerHeater.HeatingLoad, 21085.0, 1e-6);
    EXPECT_NEAR(thisChillerHeater.HeatElectricPower, 400.0, 1e-6);
    EXPECT_NEAR(thisChillerHeater.HotWaterReturnTemp, 60.0, 1e-6);
    EXPECT_NEAR(thisChillerHeater.HotWaterSupplyTemp, 70.0, 1e-6);
    EXPECT_NEAR(thisChillerHeater.HotWaterFlowRate, 0.5, 1e-6);
    EXPECT_NEAR(thisChillerHeater.ElectricPower, 400.0, 1e-6);
    EXPECT_NEAR(thisChillerHeater.HeatPartLoadRatio, 0.0, 1e-6);
    EXPECT_NEAR(thisChillerHeater.HeatingCapacity, 0.0, 1e-6);
    EXPECT_NEAR(thisChillerHeater.FractionOfPeriodRunning, 1.0, 1e-6);
}
