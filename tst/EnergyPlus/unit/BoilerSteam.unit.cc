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

// EnergyPlus::BoilerSteam Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/BoilerSteam.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Psychrometrics.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;
using namespace EnergyPlus::BoilerSteam;
using namespace EnergyPlus::DataBranchAirLoopPlant;
using namespace EnergyPlus::DataEnvironment;
using namespace EnergyPlus::DataSizing;
using namespace EnergyPlus::Psychrometrics;

TEST_F(EnergyPlusFixture, BoilerSteam_GetInput)
{

    std::string const idf_objects = delimited_string({
        "  Boiler:Steam,                                                                                            ",
        "    Steam Boiler Plant Boiler,  !- Name                                                                    ",
        "    NaturalGas,                !- Fuel Type                                                                ",
        "    160000,                    !- Maximum Operating Pressure{ Pa }                                         ",
        "    0.8,                       !- Theoretical Efficiency                                                   ",
        "    115,                       !- Design Outlet Steam Temperature{ C }                                     ",
        "    autosize,                  !- Nominal Capacity{ W }                                                    ",
        "    0.00001,                   !- Minimum Part Load Ratio                                                  ",
        "    1.0,                       !- Maximum Part Load Ratio                                                  ",
        "    0.2,                       !- Optimum Part Load Ratio                                                  ",
        "    0.8,                       !- Coefficient 1 of Fuel Use Function of Part Load Ratio Curve              ",
        "    0.1,                       !- Coefficient 2 of Fuel Use Function of Part Load Ratio Curve              ",
        "    0.1,                       !- Coefficient 3 of Fuel Use Function of Part Load Ratio Curve              ",
        "    Steam Boiler Plant Boiler Inlet Node,  !- Water Inlet Node Name                                        ",
        "    Steam Boiler Plant Boiler Outlet Node;  !- Steam Outlet Node Name                                      ",
    });

    ASSERT_TRUE(process_idf(idf_objects, false));

    state->init_state(*state);

    GetBoilerInput(*state);
    auto &thisBoiler = state->dataBoilerSteam->Boiler((int)state->dataBoilerSteam->Boiler.size());
    EXPECT_EQ(thisBoiler.Name, "STEAM BOILER PLANT BOILER");
    EXPECT_ENUM_EQ(thisBoiler.FuelType, Constant::eFuel::NaturalGas);
    EXPECT_EQ(thisBoiler.BoilerMaxOperPress, 160000);
    EXPECT_EQ(thisBoiler.NomEffic, 0.8);
    EXPECT_EQ(thisBoiler.TempUpLimitBoilerOut, 115);
    EXPECT_EQ(thisBoiler.NomCap, AutoSize);
    EXPECT_EQ(thisBoiler.MinPartLoadRat, 0.00001);
    EXPECT_EQ(thisBoiler.MaxPartLoadRat, 1.0);
    EXPECT_EQ(thisBoiler.OptPartLoadRat, 0.2);
    EXPECT_EQ(thisBoiler.FullLoadCoef[0], 0.8);
    EXPECT_EQ(thisBoiler.FullLoadCoef[1], 0.1);
    EXPECT_EQ(thisBoiler.FullLoadCoef[2], 0.1);
    EXPECT_EQ(thisBoiler.SizFac, 1.0);
}

TEST_F(EnergyPlusFixture, BoilerSteam_Simulate)
{

    std::string const idf_objects = delimited_string({
        "  Boiler:Steam,                                                                                            ",
        "    Boiler,                    !- Name                                                                    ",
        "    NaturalGas,                !- Fuel Type                                                                ",
        "    160000,                    !- Maximum Operating Pressure{ Pa }                                         ",
        "    0.8,                       !- Theoretical Efficiency                                                   ",
        "    115,                       !- Design Outlet Steam Temperature{ C }                                     ",
        "    1250,                      !- Nominal Capacity{ W }                                                    ",
        "    0.00001,                   !- Minimum Part Load Ratio                                                  ",
        "    1.0,                       !- Maximum Part Load Ratio                                                  ",
        "    0.2,                       !- Optimum Part Load Ratio                                                  ",
        "    0.8,                       !- Coefficient 1 of Fuel Use Function of Part Load Ratio Curve              ",
        "    0.1,                       !- Coefficient 2 of Fuel Use Function of Part Load Ratio Curve              ",
        "    0.1,                       !- Coefficient 3 of Fuel Use Function of Part Load Ratio Curve              ",
        "    Steam Boiler Plant Boiler Inlet Node,  !- Water Inlet Node Name                                        ",
        "    Steam Boiler Plant Boiler Outlet Node;  !- Steam Outlet Node Name                                      ",
    });

    ASSERT_TRUE(process_idf(idf_objects, false));

    state->init_state(*state);

    BoilerSpecs *ptr = BoilerSteam::BoilerSpecs::factory(*state, "BOILER");
    EXPECT_EQ(ptr->Name, "BOILER");

    PlantLocation pl{1, EnergyPlus::DataPlant::LoopSideLocation::Demand, 1, 1};
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);

    Real64 max, opt, min = 0.0;
    ptr->getDesignCapacities(*state, pl, max, min, opt);
    EXPECT_NEAR(max, 1250, 1.0);
    EXPECT_NEAR(min, 0.0, 1.0);
    EXPECT_NEAR(opt, 250.0, 1.0);

    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    state->dataPlnt->PlantLoop(1).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Demand).TempSetPoint = 2.0;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).MyLoad = 1000;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).ON = true;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = ptr->BoilerInletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = ptr->BoilerOutletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::Boiler_Steam;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = ptr->Name;
    state->dataPlnt->PlantLoop(1).MaxVolFlowRate = 3;
    state->dataPlnt->PlantLoop(1).MaxMassFlowRate = 3;
    state->dataSize->CurLoopNum = 1;

    state->dataLoopNodes->Node(ptr->BoilerOutletNodeNum).MassFlowRateMaxAvail = 5;
    state->dataLoopNodes->Node(ptr->BoilerOutletNodeNum).MassFlowRateMax = 5;
    state->dataLoopNodes->Node(ptr->BoilerInletNodeNum).Temp = 20;
    state->dataLoopNodes->Node(ptr->BoilerInletNodeNum).MassFlowRateMaxAvail = 5;
    state->dataLoopNodes->Node(ptr->BoilerInletNodeNum).MassFlowRateMax = 5;

    Real64 curLoad = 1000.0;
    ptr->simulate(*state, pl, true, curLoad, true);
}

TEST_F(EnergyPlusFixture, BoilerSteam_BoilerEfficiency)
{
    std::string const idf_objects = delimited_string({
        "  Boiler:Steam,                                                                                            ",
        "    Steam Boiler Plant Boiler,  !- Name                                                                    ",
        "    NaturalGas,                !- Fuel Type                                                                ",
        "    160000,                    !- Maximum Operating Pressure{ Pa }                                         ",
        "    0.8,                       !- Theoretical Efficiency                                                   ",
        "    115,                       !- Design Outlet Steam Temperature{ C }                                     ",
        "    autosize,                  !- Nominal Capacity{ W }                                                    ",
        "    0.00001,                   !- Minimum Part Load Ratio                                                  ",
        "    1.0,                       !- Maximum Part Load Ratio                                                  ",
        "    0.2,                       !- Optimum Part Load Ratio                                                  ",
        "    0.8,                       !- Coefficient 1 of Fuel Use Function of Part Load Ratio Curve              ",
        "    0.1,                       !- Coefficient 2 of Fuel Use Function of Part Load Ratio Curve              ",
        "    0.1,                       !- Coefficient 3 of Fuel Use Function of Part Load Ratio Curve              ",
        "    Steam Boiler Plant Boiler Inlet Node,  !- Water Inlet Node Name                                        ",
        "    Steam Boiler Plant Boiler Outlet Node;  !- Steam Outlet Node Name                                      ",
    });

    EXPECT_TRUE(process_idf(idf_objects, false));

    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    bool RunFlag(true);
    Real64 MyLoad(1000000.0);

    state->dataPlnt->TotNumLoops = 2;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataGlobal->TimeStep = 1;

    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }

    GetBoilerInput(*state);
    auto &thisBoiler = state->dataBoilerSteam->Boiler((int)state->dataBoilerSteam->Boiler.size());

    state->dataPlnt->PlantLoop(1).Name = "SteamLoop";
    state->dataPlnt->PlantLoop(1).PlantSizNum = 1;
    state->dataPlnt->PlantLoop(1).FluidName = "STEAM";
    state->dataPlnt->PlantLoop(1).steam = Fluid::GetSteam(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = thisBoiler.Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type = DataPlant::PlantEquipmentType::Boiler_Steam;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = thisBoiler.BoilerInletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = thisBoiler.BoilerOutletNodeNum;

    state->dataSize->PlantSizData.allocate(1);
    state->dataSize->PlantSizData(1).DesVolFlowRate = 0.1;
    state->dataSize->PlantSizData(1).DeltaT = 10;

    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    state->dataPlnt->PlantFirstSizesOkayToReport = true;
    state->dataPlnt->PlantFinalSizesOkayToReport = true;

    state->dataGlobal->BeginEnvrnFlag = true;
    thisBoiler.initialize(*state);
    thisBoiler.calculate(*state, MyLoad, RunFlag, DataBranchAirLoopPlant::ControlType::SeriesActive);

    // check boiler fuel used and the resultant boiler efficiency
    EXPECT_EQ(thisBoiler.BoilerLoad, 1000000);
    EXPECT_NEAR(thisBoiler.FuelUsed, 1562498, 1.0);
    Real64 ExpectedBoilerEff = thisBoiler.BoilerLoad / thisBoiler.FuelUsed;
    EXPECT_NEAR(thisBoiler.BoilerEff, ExpectedBoilerEff, 0.01);
}
