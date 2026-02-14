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
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/OutsideEnergySources.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>

using namespace EnergyPlus;
using namespace OutsideEnergySources;

TEST_F(EnergyPlusFixture, DistrictCoolingandHeating)
{
    std::string const idf_objects = delimited_string({

        "  DistrictCooling,",
        "    Purchased Cooling,              !- Name                            ",
        "    Purchased Cooling Inlet Node,   !- Chilled Water Inlet Node Name   ",
        "    Purchased Cooling Outlet Node,  !- Chilled Water Outlet Node Name  ",
        "    900000;                         !- Nominal Capacity {W}            ",

        "  DistrictHeating:Water,",
        "    Purchased Heating,           !- Name                        ",
        "    Purchased Heat Inlet Node,   !- Hot Water Inlet Node Name   ",
        "    Purchased Heat Outlet Node,  !- Hot Water Outlet Node Name  ",
        "    1000000;                     !- Nominal Capacity {W}        ",

        "  DistrictHeating:Steam,",
        "    DistrictHeating Steam,              !- Name                    ",
        "    DistrictHeating Steam Inlet Node,   !- Steam Inlet Node Name   ",
        "    DistrictHeating Steam Outlet Node,  !- Steam Outlet Node Name  ",
        "    1100000;                            !- Nominal Capacity {W}    ",
    });

    ASSERT_TRUE(process_idf(idf_objects, false));
    state->init_state(*state);

    GetOutsideEnergySourcesInput(*state);

    // GetOutsideEnergySourcesInput() finds DistrictHeating:Water, DistrictCooling, and DistrictHeating:Steam, respectively
    auto &thisDistrictHeatingWater = state->dataOutsideEnergySrcs->EnergySource(1);
    auto &thisDistrictCooling = state->dataOutsideEnergySrcs->EnergySource(2);
    auto &thisDistrictHeatingSteam = state->dataOutsideEnergySrcs->EnergySource(3);

    // Tests for GetOutsideEnergySourcesInput()
    EXPECT_ENUM_EQ(thisDistrictHeatingWater.EnergyType, DataPlant::PlantEquipmentType::PurchHotWater);
    EXPECT_ENUM_EQ(thisDistrictCooling.EnergyType, DataPlant::PlantEquipmentType::PurchChilledWater);
    EXPECT_ENUM_EQ(thisDistrictHeatingSteam.EnergyType, DataPlant::PlantEquipmentType::PurchSteam);

    EXPECT_EQ(thisDistrictHeatingWater.NomCap, 1000000.0);
    EXPECT_EQ(thisDistrictCooling.NomCap, 900000.0);
    EXPECT_EQ(thisDistrictHeatingSteam.NomCap, 1100000.0);

    // Test setup for calculate()
    bool RunFlag(true);
    Real64 MyLoad(1000000.0);
    bool firstHVAC = true;
    state->dataGlobal->BeginEnvrnFlag = true;
    std::string_view RoutineName("OutsideEnergySourcesTests");

    state->dataPlnt->TotNumLoops = 3;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    auto &thisHotWaterLoop(state->dataPlnt->PlantLoop(1));
    auto &thisChilledWaterLoop(state->dataPlnt->PlantLoop(2));
    auto &thisSteamLoop(state->dataPlnt->PlantLoop(3));

    // Setup for DistrictHeating:Water
    PlantLocation locHotWater(1, DataPlant::LoopSideLocation::Supply, 1, 1);
    thisHotWaterLoop.Name = "HotWaterLoop";
    thisHotWaterLoop.FluidName = "WATER";
    thisHotWaterLoop.glycol = Fluid::GetWater(*state);
    thisHotWaterLoop.MinTemp = 1.0;
    thisHotWaterLoop.MaxTemp = 99.0;
    thisHotWaterLoop.MinMassFlowRate = 0.001;
    thisHotWaterLoop.MaxMassFlowRate = 20;
    thisHotWaterLoop.LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    thisHotWaterLoop.LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    thisHotWaterLoop.LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    thisHotWaterLoop.LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    thisHotWaterLoop.LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    thisHotWaterLoop.LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).Name = thisDistrictHeatingWater.Name;
    thisHotWaterLoop.LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).Type = DataPlant::PlantEquipmentType::PurchHotWater;

    state->dataLoopNodes->Node(thisDistrictHeatingWater.InletNodeNum).Temp = 55.0;
    thisDistrictHeatingWater.plantLoc = locHotWater;
    thisDistrictHeatingWater.plantLoc.loopNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, thisDistrictHeatingWater.plantLoc);

    thisDistrictHeatingWater.BeginEnvrnInitFlag = true;
    thisDistrictHeatingWater.simulate(*state, locHotWater, firstHVAC, MyLoad, RunFlag);

    Real64 Cp = thisHotWaterLoop.glycol->getSpecificHeat(*state, thisDistrictHeatingWater.InletTemp, RoutineName);
    Real64 calOutletTemp =
        (MyLoad + thisHotWaterLoop.MaxMassFlowRate * Cp * thisDistrictHeatingWater.InletTemp) / (thisHotWaterLoop.MaxMassFlowRate * Cp);

    EXPECT_EQ(thisDistrictHeatingWater.OutletTemp,
              calOutletTemp); // check if the water outlet temperature from simulate() is equal to the calculation

    // Setup for DistrictCooling
    MyLoad = -900000.0;
    PlantLocation locChilledWater(2, DataPlant::LoopSideLocation::Supply, 1, 1);
    thisChilledWaterLoop.Name = "ChilledWaterLoop";
    thisChilledWaterLoop.FluidName = "WATER";
    thisChilledWaterLoop.glycol = Fluid::GetWater(*state);

    thisChilledWaterLoop.MinTemp = 1.0;
    thisChilledWaterLoop.MaxTemp = 99.0;
    thisChilledWaterLoop.MinMassFlowRate = 0.001;
    thisChilledWaterLoop.MaxMassFlowRate = 20;
    thisChilledWaterLoop.LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    thisChilledWaterLoop.LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    thisChilledWaterLoop.LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    thisChilledWaterLoop.LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    thisChilledWaterLoop.LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    thisChilledWaterLoop.LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).Name = thisDistrictCooling.Name;
    thisChilledWaterLoop.LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).Type = DataPlant::PlantEquipmentType::PurchChilledWater;
    thisChilledWaterLoop.LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).NodeNumIn = thisDistrictCooling.InletNodeNum;
    thisChilledWaterLoop.LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).NodeNumOut = thisDistrictCooling.OutletNodeNum;

    state->dataLoopNodes->Node(thisDistrictCooling.InletNodeNum).Temp = 65.0;
    thisDistrictCooling.plantLoc = locChilledWater;
    thisDistrictCooling.plantLoc.loopNum = 2;
    PlantUtilities::SetPlantLocationLinks(*state, thisDistrictCooling.plantLoc);
    thisDistrictCooling.BeginEnvrnInitFlag = true;
    thisDistrictCooling.simulate(*state, locChilledWater, firstHVAC, MyLoad, RunFlag);

    Cp = thisChilledWaterLoop.glycol->getSpecificHeat(*state, thisDistrictCooling.InletTemp, RoutineName);
    calOutletTemp =
        (MyLoad + thisChilledWaterLoop.MaxMassFlowRate * Cp * thisDistrictCooling.InletTemp) / (thisChilledWaterLoop.MaxMassFlowRate * Cp);

    EXPECT_EQ(thisDistrictCooling.OutletTemp, calOutletTemp); // check if the water outlet temperature from simulate() is equal to the calculation

    // Setup for DistrictHeating:Steam
    MyLoad = 1100000.0;
    PlantLocation locSteam(3, DataPlant::LoopSideLocation::Supply, 1, 1);
    thisSteamLoop.Name = "SteamLoop";
    thisSteamLoop.FluidName = "STEAM";
    thisSteamLoop.steam = Fluid::GetSteam(*state);
    thisSteamLoop.glycol = Fluid::GetWater(*state);
    thisSteamLoop.MinMassFlowRate = 0.00001;
    thisSteamLoop.MaxMassFlowRate = 20;
    thisSteamLoop.TempSetPointNodeNum = thisDistrictHeatingSteam.OutletNodeNum;
    thisSteamLoop.LoopSide(DataPlant::LoopSideLocation::Supply).Branch.allocate(1);
    thisSteamLoop.LoopSide(DataPlant::LoopSideLocation::Supply).TotalBranches = 1;
    thisSteamLoop.LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    thisSteamLoop.LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).TotalComponents = 1;
    thisSteamLoop.LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp.allocate(1);
    thisSteamLoop.LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).Name = thisDistrictHeatingSteam.Name;
    thisSteamLoop.LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).Type = DataPlant::PlantEquipmentType::PurchSteam;

    state->dataLoopNodes->Node(thisDistrictHeatingSteam.InletNodeNum).Temp = 95.0; // Temperature of inlet condensate after subcooling
    state->dataLoopNodes->Node(thisDistrictHeatingSteam.OutletNodeNum).TempSetPoint = 105.0;
    thisDistrictHeatingSteam.plantLoc = locSteam;
    thisDistrictHeatingSteam.plantLoc.loopNum = 3;
    PlantUtilities::SetPlantLocationLinks(*state, thisDistrictHeatingSteam.plantLoc);
    thisDistrictHeatingSteam.BeginEnvrnInitFlag = true;
    thisDistrictHeatingSteam.simulate(*state, locSteam, firstHVAC, MyLoad, RunFlag);

    Real64 SatTempAtmPress = thisSteamLoop.steam->getSatTemperature(*state, DataEnvironment::StdPressureSeaLevel, RoutineName);
    Real64 CpCondensate = thisSteamLoop.glycol->getSpecificHeat(*state, thisDistrictHeatingSteam.InletTemp, RoutineName);
    Real64 deltaTsensible = SatTempAtmPress - thisDistrictHeatingSteam.InletTemp;
    Real64 EnthSteamInDry = thisSteamLoop.steam->getSatEnthalpy(*state, thisDistrictHeatingSteam.InletTemp, 1.0, RoutineName);
    Real64 EnthSteamOutWet = thisSteamLoop.steam->getSatEnthalpy(*state, thisDistrictHeatingSteam.InletTemp, 0.0, RoutineName);
    Real64 LatentHeatSteam = EnthSteamInDry - EnthSteamOutWet;
    Real64 calOutletMdot = MyLoad / (LatentHeatSteam + (CpCondensate * deltaTsensible));

    EXPECT_EQ(thisDistrictHeatingSteam.MassFlowRate,
              calOutletMdot); // check if the steam outlet mass flow rate from simulate() is equal to the calculation
}
