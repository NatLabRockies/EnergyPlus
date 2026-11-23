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

// EnergyPlus::PlantChillers, Chiller:ConstantCOP Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantChillers.hh>
#include <EnergyPlus/Psychrometrics.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::PlantChillers;

TEST_F(EnergyPlusFixture, ChillerConstantCOP_WaterCooled_Autosize)
{

    state->dataPlnt->TotNumLoops = 4;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;

    std::string const idf_objects = delimited_string({
        "  Chiller:ConstantCOP,",
        "    Chiller,                 !- Name",
        "    autosize,                !- Nominal Capacity {W}",
        "    4.0,                     !- Nominal COP {W/W}",
        "    autosize,                !- Design Chilled Water Flow Rate {m3/s}",
        "    autosize,                !- Design Condenser Water Flow Rate {m3/s}",
        "    Chiller ChW Inlet,       !- Chilled Water Inlet Node Name",
        "    Chiller ChW Outlet,      !- Chilled Water Outlet Node Name",
        "    Chiller Cnd Inlet,       !- Condenser Inlet Node Name",
        "    Chiller Cnd Outlet,      !- Condenser Outlet Node Name",
        "    WaterCooled,             !- Condenser Type",
        "    ConstantFlow,            !- Chiller Flow Mode",
        "    1,                       !- Sizing Factor",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    2,                       !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- temperature difference described above",
        "    ThermoCapFracCurve;      !- Thermosiphon Capacity Fraction Curve Name",

        "Curve:Linear, ThermoCapFracCurve, 0.0, 0.06, 0.0, 10.0, 0.0, 1.0, Dimensionless, Dimensionless;",
    });

    EXPECT_TRUE(process_idf(idf_objects, false));

    state->init_state(*state);

    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }

    ConstCOPChillerSpecs::getInput(*state);

    auto &thisChiller = state->dataPlantChillers->ConstCOPChiller(1);

    state->dataPlnt->PlantLoop(1).Name = "ChilledWaterLoop";
    state->dataPlnt->PlantLoop(1).PlantSizNum = 1;
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = thisChiller.Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::Chiller_ConstCOP;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = thisChiller.EvapInletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = thisChiller.EvapOutletNodeNum;

    state->dataPlnt->PlantLoop(2).Name = "CondenserWaterLoop";
    state->dataPlnt->PlantLoop(2).PlantSizNum = 2;
    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);

    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = thisChiller.Name;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::Chiller_ConstCOP;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = thisChiller.CondInletNodeNum;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = thisChiller.CondOutletNodeNum;

    state->dataSize->PlantSizData.allocate(2);
    state->dataSize->PlantSizData(1).DesVolFlowRate = 0.001;
    state->dataSize->PlantSizData(1).DeltaT = 5.0;

    state->dataSize->PlantSizData(2).DesVolFlowRate = 0.001;
    state->dataSize->PlantSizData(2).DeltaT = 5.0;

    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    state->dataPlnt->PlantFirstSizesOkayToReport = true;
    state->dataPlnt->PlantFinalSizesOkayToReport = true;

    bool RunFlag(true);
    Real64 MyLoad(-20000.0);

    thisChiller.initialize(*state, RunFlag, MyLoad);
    thisChiller.size(*state);

    // run init again after sizing is complete to set mass flow rate
    state->dataGlobal->BeginEnvrnFlag = true;
    thisChiller.initialize(*state, RunFlag, MyLoad);

    // check autocalculate chiller nominal capacity
    EXPECT_NEAR(thisChiller.NomCap, 20987.5090557, 0.000001);
    // check autocalculate chiller side evap water flow rate
    EXPECT_NEAR(thisChiller.EvapVolFlowRate, 0.001, 0.000001);
    EXPECT_NEAR(thisChiller.EvapMassFlowRateMax, 0.999898, 0.0000001);
    // check autocalculate chiller side cond water flow rate
    EXPECT_NEAR(thisChiller.CondVolFlowRate, 0.0012606164769923673, 0.0000001);
    EXPECT_NEAR(thisChiller.CondMassFlowRateMax, 1.2604878941117141, 0.0000001);

    // test thermosiphon model
    DataBranchAirLoopPlant::ControlType const EquipFlowCtrl = DataBranchAirLoopPlant::ControlType::SeriesActive;
    state->dataLoopNodes->Node(thisChiller.EvapInletNodeNum).Temp = 10.0;
    state->dataLoopNodes->Node(thisChiller.EvapOutletNodeNum).Temp = 6.0;
    state->dataLoopNodes->Node(thisChiller.EvapOutletNodeNum).TempSetPoint = 6.0;
    state->dataLoopNodes->Node(thisChiller.CondInletNodeNum).Temp = 12.0; // condenser inlet temp > evap outlet temp

    thisChiller.initialize(*state, RunFlag, MyLoad);
    thisChiller.calculate(*state, MyLoad, RunFlag, EquipFlowCtrl);
    EXPECT_GT(thisChiller.partLoadRatio, 0.95);   // load is large
    EXPECT_EQ(thisChiller.thermosiphonStatus, 0); // thermosiphon is off
    EXPECT_GT(thisChiller.Power, 4000.0);         // power is non-zero

    state->dataLoopNodes->Node(thisChiller.CondInletNodeNum).Temp = 5.0; // condenser inlet temp < evap outlet temp

    thisChiller.initialize(*state, RunFlag, MyLoad);
    thisChiller.calculate(*state, MyLoad, RunFlag, EquipFlowCtrl);
    EXPECT_GT(thisChiller.partLoadRatio, 0.95);   // load is large
    EXPECT_EQ(thisChiller.thermosiphonStatus, 0); // thermosiphon is off
    EXPECT_GT(thisChiller.Power, 4000.0);         // power is non-zero

    MyLoad /= 15.0; // reduce load such that thermosiphon can meet load
    thisChiller.initialize(*state, RunFlag, MyLoad);
    thisChiller.calculate(*state, MyLoad, RunFlag, EquipFlowCtrl);
    Real64 dT = thisChiller.EvapOutletTemp - thisChiller.CondInletTemp;
    Real64 thermosiphonCapFrac = Curve::CurveValue(*state, thisChiller.thermosiphonTempCurveIndex, dT);
    EXPECT_LT(thisChiller.partLoadRatio, 0.065);               // load is small
    EXPECT_GT(thermosiphonCapFrac, thisChiller.partLoadRatio); // thermosiphon capacity can meet load
    EXPECT_EQ(thisChiller.thermosiphonStatus, 1);              // thermosiphon is on
    EXPECT_EQ(thisChiller.Power, 0.0);                         // power is zero
}

TEST_F(EnergyPlusFixture, ChillerConstantCOP_Default_Des_Cond_Evap_Temps)
{
    // Unit test for PR 10158 that fixes Issue 10157)
    state->dataPlnt->TotNumLoops = 12;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;

    std::string const idf_objects = delimited_string({
        "  Chiller:ConstantCOP,",
        "    Chiller_1_WaterCooled,   !- Name",
        "    autosize,                !- Nominal Capacity {W}",
        "    4.0,                     !- Nominal COP {W/W}",
        "    autosize,                !- Design Chilled Water Flow Rate {m3/s}",
        "    autosize,                !- Design Condenser Water Flow Rate {m3/s}",
        "    Chiller 1 ChW Inlet,     !- Chilled Water Inlet Node Name",
        "    Chiller 1 ChW Outlet,    !- Chilled Water Outlet Node Name",
        "    Chiller 1 Cnd Inlet,     !- Condenser Inlet Node Name",
        "    Chiller 1 Cnd Outlet,    !- Condenser Outlet Node Name",
        "    WaterCooled,             !- Condenser Type",
        "    ConstantFlow,            !- Chiller Flow Mode",
        "    1,                       !- Sizing Factor",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    2;                       !- Basin Heater Setpoint Temperature {C}",

        "  Chiller:ConstantCOP,",
        "    Chiller_2_AirCooled,     !- Name",
        "    autosize,                !- Nominal Capacity {W}",
        "    4.0,                     !- Nominal COP {W/W}",
        "    autosize,                !- Design Chilled Water Flow Rate {m3/s}",
        "    autosize,                !- Design Condenser Water Flow Rate {m3/s}",
        "    Chiller 2 ChW Inlet,     !- Chilled Water Inlet Node Name",
        "    Chiller 2 ChW Outlet,    !- Chilled Water Outlet Node Name",
        "    Chiller 2 Cnd Inlet,     !- Condenser Inlet Node Name",
        "    Chiller 2 Cnd Outlet,    !- Condenser Outlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    ConstantFlow,            !- Chiller Flow Mode",
        "    1,                       !- Sizing Factor",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    2;                       !- Basin Heater Setpoint Temperature {C}",

        "  Chiller:ConstantCOP,",
        "    Chiller_3_EvapCooled,    !- Name",
        "    autosize,                !- Nominal Capacity {W}",
        "    4.0,                     !- Nominal COP {W/W}",
        "    autosize,                !- Design Chilled Water Flow Rate {m3/s}",
        "    autosize,                !- Design Condenser Water Flow Rate {m3/s}",
        "    Chiller 3 ChW Inlet,     !- Chilled Water Inlet Node Name",
        "    Chiller 3 ChW Outlet,    !- Chilled Water Outlet Node Name",
        "    Chiller 3 Cnd Inlet,     !- Condenser Inlet Node Name",
        "    Chiller 3 Cnd Outlet,    !- Condenser Outlet Node Name",
        "    EvaporativelyCooled,     !- Condenser Type",
        "    ConstantFlow,            !- Chiller Flow Mode",
        "    1,                       !- Sizing Factor",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    2;                       !- Basin Heater Setpoint Temperature {C}",
    });

    EXPECT_TRUE(process_idf(idf_objects, false));

    state->init_state(*state);

    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }

    ConstCOPChillerSpecs::getInput(*state);

    auto &thisChiller_1 = state->dataPlantChillers->ConstCOPChiller(1);

    EXPECT_NEAR(thisChiller_1.TempDesCondIn, 29.44, 1e-3);
    EXPECT_NEAR(thisChiller_1.TempDesEvapOut, 6.67, 1e-3);

    auto &thisChiller_2 = state->dataPlantChillers->ConstCOPChiller(2);

    EXPECT_NEAR(thisChiller_2.TempDesCondIn, 35.0, 1e-3);
    EXPECT_NEAR(thisChiller_2.TempDesEvapOut, 6.67, 1e-3);

    auto &thisChiller_3 = state->dataPlantChillers->ConstCOPChiller(3);

    EXPECT_NEAR(thisChiller_3.TempDesCondIn, 35.0, 1e-3);
    EXPECT_NEAR(thisChiller_3.TempDesEvapOut, 6.67, 1e-3);
}
