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
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantLoadProfile.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/ScheduleManager.hh>

using namespace EnergyPlus;
using namespace PlantLoadProfile;

TEST_F(EnergyPlusFixture, LoadProfile_GetInput)
{
    std::string const idf_objects = delimited_string({

        "  Schedule:Compact,",
        "    Load Profile Load Schedule,    !- Name                       ",
        "    Any Number,                    !- Schedule Type Limits Name  ",
        "    THROUGH: 12/31,                !- Field 1                    ",
        "    FOR: AllDays,                  !- Field 2                    ",
        "    UNTIL: 24:00,10000;            !- Field 3                    ",

        "  Schedule:Compact,",
        "    Load Profile Flow Frac Schedule,     !- Name                       ",
        "    Any Number,                          !- Schedule Type Limits Name  ",
        "    THROUGH: 12/31,                      !- Field 1                    ",
        "    FOR: AllDays,                        !- Field 2                    ",
        "    UNTIL: 24:00,1.0;                    !- Field 3                    ",

        "  LoadProfile:Plant,",
        "    Load Profile Water,                     !- Name                              ",
        "    Demand Load Profile Water Inlet Node,   !- Inlet Node Name                   ",
        "    Demand Load Profile Water Outlet Node,  !- Outlet Node Name                  ",
        "    Load Profile Load Schedule,             !- Load Schedule Name                ",
        "    0.002,                                  !- Peak Flow Rate {m3/s}             ",
        "    Load Profile Flow Frac Schedule,        !- Flow Rate Fraction Schedule Name  ",
        "    Water;                                  !- Plant Loop Fluid Type             ",

        "  LoadProfile:Plant,",
        "    Load Profile Steam,                     !- Name                              ",
        "    Demand Load Profile Steam Inlet Node,   !- Inlet Node Name                   ",
        "    Demand Load Profile Steam Outlet Node,  !- Outlet Node Name                  ",
        "    Load Profile Load Schedule,             !- Load Schedule Name                ",
        "    0.008,                                  !- Peak Flow Rate {m3/s}             ",
        "    Load Profile Flow Frac Schedule,        !- Flow Rate Fraction Schedule Name  ",
        "    Steam;                                  !- Plant Loop Fluid Type             ",
    });

    ASSERT_TRUE(process_idf(idf_objects, false));
    state->init_state(*state);

    GetPlantProfileInput(*state);

    // Tests for LoadProfile on Water loop
    EXPECT_EQ(state->dataPlantLoadProfile->PlantProfile(1).Name, "LOAD PROFILE WATER");
    EXPECT_ENUM_EQ(state->dataPlantLoadProfile->PlantProfile(1).FluidType, PlantLoopFluidType::Water);
    EXPECT_EQ(state->dataPlantLoadProfile->PlantProfile(1).PeakVolFlowRate, 0.002);

    // Tests for LoadProfile on Steam loop
    EXPECT_EQ(state->dataPlantLoadProfile->PlantProfile(2).Name, "LOAD PROFILE STEAM");
    EXPECT_ENUM_EQ(state->dataPlantLoadProfile->PlantProfile(2).FluidType, PlantLoopFluidType::Steam);
    EXPECT_EQ(state->dataPlantLoadProfile->PlantProfile(2).PeakVolFlowRate, 0.008);
    EXPECT_EQ(state->dataPlantLoadProfile->PlantProfile(2).DegOfSubcooling,
              5.0); // check if the default value is assigned in cases where there is no input
    EXPECT_EQ(state->dataPlantLoadProfile->PlantProfile(2).LoopSubcoolReturn,
              20.0); // check if the default value is assigned in cases where there is no input
}

TEST_F(EnergyPlusFixture, LoadProfile_initandsimulate_Waterloop)
{
    state->init_state(*state);
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataLoopNodes->Node.allocate(2);
    state->dataPlantLoadProfile->PlantProfile.allocate(1);

    // Test setup for a load profile in a water loop
    auto &thisWaterLoop(state->dataPlnt->PlantLoop(1));
    thisWaterLoop.FluidName = "WATER";
    thisWaterLoop.glycol = Fluid::GetWater(*state);
    thisWaterLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    thisWaterLoop.LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    thisWaterLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    thisWaterLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    thisWaterLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type = DataPlant::PlantEquipmentType::PlantLoadProfile;
    thisWaterLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = "LOAD PROFILE WATER";
    thisWaterLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = 1;
    thisWaterLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = 2;

    state->dataLoopNodes->Node(1).Temp = 60.0;
    state->dataLoopNodes->Node(1).MassFlowRateMax = 10;
    state->dataLoopNodes->Node(1).MassFlowRateMaxAvail = 10;
    state->dataLoopNodes->Node(2).MassFlowRateMax = 10;
    state->dataLoopNodes->Node(2).MassFlowRateMaxAvail = 10;

    auto &thisLoadProfileWaterLoop = state->dataPlantLoadProfile->PlantProfile(1);

    PlantLocation locWater(1, DataPlant::LoopSideLocation::Demand, 1, 1);
    thisLoadProfileWaterLoop.Name = "LOAD PROFILE WATER";
    thisLoadProfileWaterLoop.FluidType = PlantLoopFluidType::Water;

    thisLoadProfileWaterLoop.PeakVolFlowRate = 0.002;
    thisLoadProfileWaterLoop.loadSched = Sched::AddScheduleConstant(*state, "LOAD");
    thisLoadProfileWaterLoop.flowRateFracSched = Sched::AddScheduleConstant(*state, "FLOWRATEFRAC");
    thisLoadProfileWaterLoop.InletNode = 1;
    thisLoadProfileWaterLoop.OutletNode = 2;
    thisLoadProfileWaterLoop.plantLoc = locWater;
    thisLoadProfileWaterLoop.plantLoc.loopNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, thisLoadProfileWaterLoop.plantLoc);

    thisLoadProfileWaterLoop.loadSched->EMSActuatedOn = false;
    thisLoadProfileWaterLoop.loadSched->currentVal = 10000;
    thisLoadProfileWaterLoop.flowRateFracSched->EMSActuatedOn = false;
    thisLoadProfileWaterLoop.flowRateFracSched->currentVal = 0.8;

    // InitPlantProfile()
    thisLoadProfileWaterLoop.InitPlantProfile(*state);

    EXPECT_EQ(thisLoadProfileWaterLoop.InletTemp, 60.0);     // check if the component's inlet temp is set to node data
    EXPECT_EQ(thisLoadProfileWaterLoop.Power, 10000);        // check if the load schedule's current value is properly applied
    EXPECT_EQ(thisLoadProfileWaterLoop.VolFlowRate, 0.0016); // check if the flow rate fraction schedule is properly applied

    // simulate()
    bool firstHVAC = true;
    Real64 curLoad = 10000.0;
    bool runFlag = true;
    std::string_view RoutineName("PlantLoadProfileTests");
    thisLoadProfileWaterLoop.simulate(*state, locWater, firstHVAC, curLoad, runFlag);

    Real64 rhoWater = thisWaterLoop.glycol->getDensity(*state, 60, RoutineName);
    Real64 Cp = thisWaterLoop.glycol->getSpecificHeat(*state, thisLoadProfileWaterLoop.InletTemp, RoutineName);
    Real64 deltaTemp = curLoad / (rhoWater * thisLoadProfileWaterLoop.VolFlowRate * Cp);
    Real64 calOutletTemp = thisLoadProfileWaterLoop.InletTemp - deltaTemp;

    EXPECT_EQ(thisLoadProfileWaterLoop.OutletTemp,
              calOutletTemp); // check if the water outlet temperature from simulate() is equal to the calculation
}

TEST_F(EnergyPlusFixture, LoadProfile_initandsimulate_Steamloop)
{
    state->init_state(*state);
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataLoopNodes->Node.allocate(2);
    state->dataPlantLoadProfile->PlantProfile.allocate(1);

    // Test setup for a load profile in a steam loop
    auto &thisSteamLoop(state->dataPlnt->PlantLoop(1));
    thisSteamLoop.FluidName = "STEAM";
    thisSteamLoop.steam = Fluid::GetSteam(*state);
    thisSteamLoop.glycol = Fluid::GetWater(*state);
    thisSteamLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch.allocate(1);
    thisSteamLoop.LoopSide(DataPlant::LoopSideLocation::Demand).TotalBranches = 1;
    thisSteamLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).TotalComponents = 1;
    thisSteamLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp.allocate(1);
    thisSteamLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type = DataPlant::PlantEquipmentType::PlantLoadProfile;
    thisSteamLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = "LOAD PROFILE STEAM";
    thisSteamLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = 1;
    thisSteamLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = 2;

    std::string_view RoutineName("PlantLoadProfileTests");

    Real64 SatTempAtmPress = state->dataPlnt->PlantLoop(1).steam->getSatTemperature(*state, DataEnvironment::StdPressureSeaLevel, RoutineName);

    state->dataLoopNodes->Node(1).Temp = SatTempAtmPress;
    state->dataLoopNodes->Node(1).MassFlowRateMax = 1;
    state->dataLoopNodes->Node(1).MassFlowRateMaxAvail = 1;
    state->dataLoopNodes->Node(2).MassFlowRateMax = 1;
    state->dataLoopNodes->Node(2).MassFlowRateMaxAvail = 1;

    auto &thisLoadProfileSteamLoop = state->dataPlantLoadProfile->PlantProfile(1);

    PlantLocation locSteam(1, DataPlant::LoopSideLocation::Demand, 1, 1);
    thisLoadProfileSteamLoop.Name = "LOAD PROFILE STEAM";
    thisLoadProfileSteamLoop.FluidType = PlantLoopFluidType::Steam;
    thisLoadProfileSteamLoop.PeakVolFlowRate = 0.008;
    thisLoadProfileSteamLoop.DegOfSubcooling = 3.0;
    thisLoadProfileSteamLoop.loadSched = Sched::AddScheduleConstant(*state, "LOAD");
    thisLoadProfileSteamLoop.flowRateFracSched = Sched::AddScheduleConstant(*state, "FLOWRATEFRAC");
    thisLoadProfileSteamLoop.InletNode = 1;
    thisLoadProfileSteamLoop.OutletNode = 2;
    thisLoadProfileSteamLoop.plantLoc = locSteam;
    thisLoadProfileSteamLoop.plantLoc.loopNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, thisLoadProfileSteamLoop.plantLoc);

    thisLoadProfileSteamLoop.loadSched->EMSActuatedOn = false;
    thisLoadProfileSteamLoop.loadSched->currentVal = 10000;
    thisLoadProfileSteamLoop.flowRateFracSched->EMSActuatedOn = false;
    thisLoadProfileSteamLoop.flowRateFracSched->currentVal = 0.8;

    // InitPlantProfile()
    thisLoadProfileSteamLoop.InitPlantProfile(*state);

    EXPECT_EQ(thisLoadProfileSteamLoop.InletTemp, SatTempAtmPress); // check if the component's inlet temp is set to node data
    EXPECT_EQ(thisLoadProfileSteamLoop.Power, 10000);               // check if the load schedule's current value is properly applied
    EXPECT_EQ(thisLoadProfileSteamLoop.VolFlowRate, 0.0064);        // check if the flow rate fraction schedule is properly applied

    // simulate()
    bool firstHVAC = true;
    Real64 curLoad = 10000.0;
    bool runFlag = true;
    thisLoadProfileSteamLoop.simulate(*state, locSteam, firstHVAC, curLoad, runFlag);

    Real64 EnthSteamIn = thisSteamLoop.steam->getSatEnthalpy(*state, SatTempAtmPress, 1.0, RoutineName);
    Real64 EnthSteamOut = thisSteamLoop.steam->getSatEnthalpy(*state, SatTempAtmPress, 0.0, RoutineName);
    Real64 LatentHeatSteam = EnthSteamIn - EnthSteamOut;
    Real64 CpCondensate = thisSteamLoop.glycol->getSpecificHeat(*state, SatTempAtmPress, RoutineName);
    Real64 calOutletMdot = curLoad / (LatentHeatSteam + thisLoadProfileSteamLoop.DegOfSubcooling * CpCondensate);

    EXPECT_EQ(thisLoadProfileSteamLoop.MassFlowRate, calOutletMdot);
    // check if the Steam outlet mass flow rate from simulate() is equal to the calculation
}
