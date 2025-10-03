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

// EnergyPlus::WaterToAirHeatPumpSimple Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

#include "Fixtures/EnergyPlusFixture.hh"

// EnergyPlus Headers
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/WaterToAirHeatPumpSimple.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::DataEnvironment;
using namespace EnergyPlus::DataLoopNode;
using namespace EnergyPlus::DataPlant;
using namespace EnergyPlus::DataSizing;
using namespace EnergyPlus::Psychrometrics;
using namespace EnergyPlus::WaterToAirHeatPumpSimple;
using namespace EnergyPlus::Curve;

TEST_F(EnergyPlusFixture, WaterToAirHeatPumpSimpleTest_SizeHVACWaterToAir)
{
    state->dataFluid->init_state(*state);
    // This unit test is intended to check if supply air Humidity ratio used in the cooling sizing calculation is
    // reset to the minimum of entering mixed air humidity ratio and the user specified supply air design Humidity
    // ratio such that the total cooling capacity is always greater than or equal to the sensible cooling capacity.
    // This test was added to test bug issue #4893 fix, a defect that resulted in SHR greater than 1.0.

    int HPNum(1);

    state->dataSize->SysSizingRunDone = true;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->CurSysNum = 0;
    state->dataSize->CurZoneEqNum = 1;

    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP.allocate(HPNum);
    state->dataSize->FinalZoneSizing.allocate(state->dataSize->CurZoneEqNum);
    state->dataSize->ZoneEqSizing.allocate(state->dataSize->CurZoneEqNum);
    state->dataSize->DesDayWeath.allocate(1);
    state->dataSize->DesDayWeath(1).Temp.allocate(24);

    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WAHPType = WatertoAirHP::Cooling;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedAirVolFlowRate = AutoSize;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal = AutoSize;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolSens = AutoSize;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedWaterVolFlowRate = 0.0;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum = 1;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterOutletNodeNum = 2;

    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow = 0.20;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow = 0.20;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDesTemp = 13.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDesHumRat = 0.0075;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).TimeStepNumAtCoolMax = 15;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDDNum = 1;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInTemp = 25.5;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInHumRat = 0.0045;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneRetTempAtCoolPeak = 25.5;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneHumRatAtCoolPeak = 0.0045;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).OAVolFlow = 0.0;

    auto *curve1 = AddCurve(*state, "Curve1");
    curve1->curveType = CurveType::QuadLinear;
    curve1->coeff[0] = -9.149069561;
    curve1->coeff[1] = 10.878140260;
    curve1->coeff[2] = -1.718780157;
    curve1->coeff[3] = 0.746414818;
    curve1->coeff[4] = 0.0;
    curve1->inputLimits[0].min = 0.0;
    curve1->inputLimits[0].max = 80.0;
    curve1->inputLimits[1].min = 0.0;
    curve1->inputLimits[1].max = 100.0;
    curve1->inputLimits[2].min = 0.0;
    curve1->inputLimits[2].max = 2.0;
    curve1->inputLimits[3].min = 0.0;
    curve1->inputLimits[3].max = 2.0;

    auto *curve2 = AddCurve(*state, "Curve2");
    curve2->curveType = CurveType::QuintLinear;
    curve2->coeff[0] = -5.462690012;
    curve2->coeff[1] = 17.95968138;
    curve2->coeff[2] = -11.87818402;
    curve2->coeff[3] = -0.980163419;
    curve2->coeff[4] = 0.767285761;
    curve2->coeff[5] = 0.0;
    curve2->inputLimits[0].min = 0.0;
    curve2->inputLimits[0].max = 100.0;
    curve2->inputLimits[1].min = 0.0;
    curve2->inputLimits[1].max = 100.0;
    curve2->inputLimits[2].min = 0.0;
    curve2->inputLimits[2].max = 100.0;
    curve2->inputLimits[3].min = 0.0;
    curve2->inputLimits[3].max = 1.0;
    curve2->inputLimits[4].min = 0.0;
    curve2->inputLimits[4].max = 1.0;

    auto *curve3 = AddCurve(*state, "Curve3");
    curve3->curveType = CurveType::QuadLinear;
    curve3->coeff[0] = -3.205409884;
    curve3->coeff[1] = -0.976409399;
    curve3->coeff[2] = 3.97892546;
    curve3->coeff[3] = 0.938181818;
    curve3->coeff[4] = 0.0;
    curve3->inputLimits[0].min = -100;
    curve3->inputLimits[0].max = 100;
    curve3->inputLimits[1].min = -100;
    curve3->inputLimits[1].max = 100;
    curve3->inputLimits[2].min = 0;
    curve3->inputLimits[2].max = 100;
    curve3->inputLimits[3].min = 0;
    curve3->inputLimits[3].max = 38;

    // performance curve coefficients
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).TotalCoolCapCurve = curve1;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SensCoolCapCurve = curve2;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CoolPowCurve = curve3;

    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCOPCoolAtRatedCdts = 5.12;

    state->dataSize->DesDayWeath(1).Temp(15) = 32.0;
    state->dataEnvrn->StdBaroPress = 101325.0;
    state->dataSize->ZoneEqDXCoil = true;

    // create and attach a plant loop
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->PlantLoop(1).Name = "Condenser Water Loop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    auto &loopside(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand));
    loopside.TotalBranches = 1;
    loopside.Branch.allocate(1);
    auto &loopsidebranch(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
    loopsidebranch.TotalComponents = 1;
    loopsidebranch.Comp.allocate(1);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WAHPPlantType;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).plantLoc.loopNum = 1;

    // plant loop design leaving water temperature (design entering water temperature for WAHP coil)
    state->dataSize->PlantSizData.allocate(1);
    state->dataSize->PlantSizData(1).ExitTemp = 29.4;

    WaterToAirHeatPumpSimple::SizeHVACWaterToAir(*state, HPNum);

    // check that the design outlet air humidity ratio did not change
    EXPECT_DOUBLE_EQ(0.0075, state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDesHumRat);

    // check that the total cooling capacity is >= the sensible cooling capacity
    EXPECT_GE(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal,
              state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolSens);

    if (state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal != 0.0) {
        ShowMessage(*state,
                    format("SizeHVACWaterToAir: Rated Sensible Heat Ratio = {:.2R} [-]",
                           state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolSens /
                               state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedCapCoolTotal));
    }
}

TEST_F(EnergyPlusFixture, WaterToAirHeatPumpSimple_TestAirFlow)
{

    std::string const idf_objects = delimited_string({

        " Coil:Cooling:WaterToAirHeatPump:EquationFit,",
        "   Sys 5 Heat Pump Cooling Mode,  !- Name",
        "   Sys 5 Water to Air Heat Pump Source Side1 Inlet Node,  !- Water Inlet Node Name",
        "   Sys 5 Water to Air Heat Pump Source Side1 Outlet Node,  !- Water Outlet Node Name",
        "   Sys 5 Cooling Coil Air Inlet Node,  !- Air Inlet Node Name",
        "   Sys 5 Heating Coil Air Inlet Node,  !- Air Outlet Node Name",
        "   2.0,                     !- Rated Air Flow Rate {m3/s}",
        "   0.0033,                  !- Rated Water Flow Rate {m3/s}",
        "   20000,                   !- Gross Rated Total Cooling Capacity {W}",
        "   16000,                   !- Gross Rated Sensible Cooling Capacity {W}",
        "   7.007757577,             !- Gross Rated Cooling COP",
        "   ,                        !- Rated Entering Water Temperature",
        "   ,                        !- Rated Entering Air Dry-Bulb Temperature",
        "   ,                        !- Rated Entering Air Wet-Bulb Temperature",
        "   TotCoolCapCurve,         !- Total Cooling Capacity Curve Name",
        "   SensCoolCapCurve,        !- Sensible Cooling Capacity Curve Name",
        "   CoolPowCurve,            !- Cooling Power Consumption Curve Name",
        "   PLFFPLR,                 !- Part Load Fraction Correlation Curve Name",
        "   0,                       !- Nominal Time for Condensate Removal to Begin {s}",
        "   0;                       !- Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity {dimensionless}",

        " Coil:Heating:WaterToAirHeatPump:EquationFit,",
        "  Sys 5 Heat Pump Heating Mode,  !- Name",
        "  Sys 5 Water to Air Heat Pump Source Side2 Inlet Node,  !- Water Inlet Node Name",
        "  Sys 5 Water to Air Heat Pump Source Side2 Outlet Node,  !- Water Outlet Node Name",
        "  Sys 5 Heating Coil Air Inlet Node,  !- Air Inlet Node Name",
        "  Sys 5 SuppHeating Coil Air Inlet Node,  !- Air Outlet Node Name",
        "  1.0,                      !- Rated Air Flow Rate {m3/s}",
        "  0.0033,                   !- Rated Water Flow Rate {m3/s}",
        "  20000,                    !- Gross Rated Heating Capacity {W}",
        "  3.167053691,              !- Gross Rated Heating COP",
        "  ,                         !- Rated Entering Water Temperature",
        "  ,                         !- Rated Entering Air Dry-Bulb Temperature",
        "  ,                         !- Ratio of Rated Heating Capacity to Rated Cooling Capacity",
        "  HeatCapCurve,             !- Heating Capacity Curve Name",
        "  HeatPowCurve,             !- Heating Power Curve Name",
        "  PLFFPLR;                  !- Part Load Fraction Correlation Curve Name",

        "Curve:QuintLinear,",
        "  SensCoolCapCurve,     ! Curve Name",
        "  0,           ! CoefficientC1",
        "  0.2,           ! CoefficientC2",
        "  0.2,          ! CoefficientC3",
        "  0.2,          ! CoefficientC4",
        "  0.2,          ! CoefficientC5",
        "  0.2,           ! CoefficientC6",
        "  0.,                   ! Minimum Value of v",
        "  100.,                 ! Maximum Value of v",
        "  0.,                   ! Minimum Value of w",
        "  100.,                 ! Maximum Value of w",
        "  0.,                   ! Minimum Value of x",
        "  100.,                 ! Maximum Value of x",
        "  0.,                   ! Minimum Value of y",
        "  100.,                 ! Maximum Value of y",
        "  0,                    ! Minimum Value of z",
        "  100,                  ! Maximum Value of z",
        "  0.,                   ! Minimum Curve Output",
        "  38.;                  ! Maximum Curve Output",

        "Curve:QuadLinear,",
        "  TotCoolCapCurve,      ! Curve Name",
        "  0,          ! CoefficientC1",
        "  0.25,           ! CoefficientC2",
        "  0.25,          ! CoefficientC3",
        "  0.25,           ! CoefficientC4",
        "  0.25,          ! CoefficientC5",
        "  0.,                   ! Minimum Value of w",
        "  100.,                 ! Maximum Value of w",
        "  0.,                   ! Minimum Value of x",
        "  100.,                 ! Maximum Value of x",
        "  0.,                   ! Minimum Value of y",
        "  100.,                 ! Maximum Value of y",
        "  0,                    ! Minimum Value of z",
        "  100,                  ! Maximum Value of z",
        "  0.,                   ! Minimum Curve Output",
        "  38.;                  ! Maximum Curve Output",

        "Curve:QuadLinear,",
        "  CoolPowCurve,      ! Curve Name",
        "  0,          ! CoefficientC1",
        "  0.25,           ! CoefficientC2",
        "  0.25,          ! CoefficientC3",
        "  0.25,           ! CoefficientC4",
        "  0.25,          ! CoefficientC5",
        "  0.,                   ! Minimum Value of w",
        "  100.,                 ! Maximum Value of w",
        "  0.,                   ! Minimum Value of x",
        "  100.,                 ! Maximum Value of x",
        "  0.,                   ! Minimum Value of y",
        "  100.,                 ! Maximum Value of y",
        "  0,                    ! Minimum Value of z",
        "  100,                  ! Maximum Value of z",
        "  0.,                   ! Minimum Curve Output",
        "  38.;                  ! Maximum Curve Output",

        "Curve:QuadLinear,",
        "  HeatCapCurve,         ! Curve Name",
        "  0,          ! CoefficientC1",
        "  0.25,           ! CoefficientC2",
        "  0.25,          ! CoefficientC3",
        "  0.25,           ! CoefficientC4",
        "  0.25,          ! CoefficientC5",
        "  0.,                   ! Minimum Value of w",
        "  100.,                 ! Maximum Value of w",
        "  0.,                   ! Minimum Value of x",
        "  100.,                 ! Maximum Value of x",
        "  0.,                   ! Minimum Value of y",
        "  100.,                 ! Maximum Value of y",
        "  0,                    ! Minimum Value of z",
        "  100,                  ! Maximum Value of z",
        "  0.,                   ! Minimum Curve Output",
        "  38.;                  ! Maximum Curve Output",

        "Curve:QuadLinear,",
        "  HeatPowCurve,         ! Curve Name",
        "  0,          ! CoefficientC1",
        "  0.25,           ! CoefficientC2",
        "  0.25,          ! CoefficientC3",
        "  0.25,           ! CoefficientC4",
        "  0.25,          ! CoefficientC5",
        "  0.,                   ! Minimum Value of w",
        "  100.,                 ! Maximum Value of w",
        "  0.,                   ! Minimum Value of x",
        "  100.,                 ! Maximum Value of x",
        "  0.,                   ! Minimum Value of y",
        "  100.,                 ! Maximum Value of y",
        "  0,                    ! Minimum Value of z",
        "  100,                  ! Maximum Value of z",
        "  0.,                   ! Minimum Curve Output",
        "  38.;                  ! Maximum Curve Output",

        "Curve:Quadratic, PLFFPLR, 0.85, 0.83, 0.0, 0.0, 0.3, 0.85, 1.0, Dimensionless, Dimensionless; ",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetSimpleWatertoAirHPInput(*state);

    int HPNum(1);
    Real64 ActualAirflow(1.0);
    Real64 DesignWaterflow(15.0);
    Real64 CpAir = PsyCpAirFnW(0.007);
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum).Temp = 5.0;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum).Enthalpy = 44650.0;

    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).DesignWaterMassFlowRate = DesignWaterflow;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum).MassFlowRate = DesignWaterflow;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum).MassFlowRateMax = DesignWaterflow;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum).MassFlowRateMaxAvail =
        DesignWaterflow;

    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).MassFlowRate =
        ActualAirflow; // rated condition
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).Temp = 26.0;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).HumRat = 0.007;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).Enthalpy = 43970.75;

    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }

    state->dataPlnt->PlantLoop(1).Name = "ChilledWaterLoop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WAHPPlantType;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum;

    HVAC::CompressorOp compressorOp = HVAC::CompressorOp::On;
    HVAC::FanOp fanOp = HVAC::FanOp::Cycling;
    bool FirstHVACIteration(true);
    Real64 SensLoad(38000.0);
    Real64 LatentLoad(0.0);
    Real64 PartLoadRatio(1.0);
    Real64 OnOffAirFlowRatio(1.0);
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).plantLoc.loopNum = 1;

    state->dataEnvrn->OutBaroPress = 101325.0;

    InitSimpleWatertoAirHP(*state, HPNum, SensLoad, LatentLoad, fanOp, OnOffAirFlowRatio, FirstHVACIteration);
    CalcHPCoolingSimple(*state, HPNum, fanOp, SensLoad, LatentLoad, compressorOp, PartLoadRatio, OnOffAirFlowRatio);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirMassFlowRate, 1.0);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QLoadTotal, 20000 * 0.85781, 0.1);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QSensible, 16000 * 0.89755, 0.1);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->Winput, 2853.98 * 0.85781, 0.1);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->LoadSideInletDBTemp, 26.0);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->LoadSideOutletDBTemp, 26.0 - (14360.848 / 1.0 / CpAir), 0.0001);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->LoadSideInletEnth, 43970.75);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirEnthalpy, 43970.75 - (17156.275 / 1.0), 0.1);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirDBTemp, 26.0 - (14360.848 / 1.0 / CpAir), 0.0001);

    PartLoadRatio = 0.5;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).MassFlowRate =
        ActualAirflow * PartLoadRatio;
    InitSimpleWatertoAirHP(*state, HPNum, SensLoad, LatentLoad, fanOp, OnOffAirFlowRatio, FirstHVACIteration);
    CalcHPCoolingSimple(*state, HPNum, fanOp, SensLoad, LatentLoad, compressorOp, PartLoadRatio, OnOffAirFlowRatio);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirMassFlowRate, 0.5);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QLoadTotal, 20000 * 0.85781 * 0.5, 0.1);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QSensible, 16000 * 0.89755 * 0.5, 0.1);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->Winput, 2853.98 * 0.85781 * 0.5, 0.1);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->LoadSideInletDBTemp, 26.0);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->LoadSideOutletDBTemp, 26.0 - (14360.848 / 1.0 / CpAir), 0.0001);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->LoadSideInletEnth, 43970.75);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirEnthalpy, 43970.75 - (17156.275 / 1.0), 0.1);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirDBTemp, 26.0 - (14360.848 / 1.0 / CpAir), 0.0001);

    // constant fan
    fanOp = HVAC::FanOp::Continuous;
    PartLoadRatio = 1.0;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).MassFlowRate =
        ActualAirflow * PartLoadRatio;
    InitSimpleWatertoAirHP(*state, HPNum, SensLoad, LatentLoad, fanOp, OnOffAirFlowRatio, FirstHVACIteration);
    CalcHPCoolingSimple(*state, HPNum, fanOp, SensLoad, LatentLoad, compressorOp, PartLoadRatio, OnOffAirFlowRatio);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirMassFlowRate, 1.0);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QLoadTotal, 20000 * 0.85781, 0.1);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QSensible, 16000 * 0.89755, 0.1);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->Winput, 2853.98 * 0.85781, 0.1);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->LoadSideInletDBTemp, 26.0);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->LoadSideOutletDBTemp, 26.0 - (14360.848 / 1.0 / CpAir), 0.0001);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->LoadSideInletEnth, 43970.75);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirEnthalpy, 43970.75 - (17156.275 / 1.0), 0.1);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirDBTemp, 26.0 - (14360.848 / 1.0 / CpAir), 0.0001);

    PartLoadRatio = 0.5;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).MassFlowRate = ActualAirflow;
    InitSimpleWatertoAirHP(*state, HPNum, SensLoad, LatentLoad, fanOp, OnOffAirFlowRatio, FirstHVACIteration);
    CalcHPCoolingSimple(*state, HPNum, fanOp, SensLoad, LatentLoad, compressorOp, PartLoadRatio, OnOffAirFlowRatio);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirMassFlowRate, 1.0);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QLoadTotal, 20000 * 0.85781 * 0.5, 0.1);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QSensible, 16000 * 0.89755 * 0.5, 0.1);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->Winput, 2853.98 * 0.85781 * 0.5, 0.1);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->LoadSideInletEnth, 43970.75);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->LoadSideInletDBTemp, 26.0);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->LoadSideOutletDBTemp, 26.0 - (14360.848 / 1.0 / CpAir), 0.0001);
    EXPECT_NEAR(
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirEnthalpy, (43970.75 - (17156.275 / 1.0)) * 0.5 + 43970.75 * 0.5, 0.1);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirDBTemp, 18.95267, 0.0001);

    HPNum = 2;

    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).plantLoc.loopNum = 2;
    state->dataPlnt->PlantLoop(2).Name = "HotWaterLoop";
    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WAHPPlantType;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum;

    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).DesignWaterMassFlowRate = DesignWaterflow;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum).MassFlowRate = DesignWaterflow;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum).MassFlowRateMax = DesignWaterflow;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum).MassFlowRateMaxAvail =
        DesignWaterflow;

    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum).Temp = 35.0;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum).Enthalpy = 43950.0;

    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).MassFlowRate = ActualAirflow;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).Temp = 15.0;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).HumRat = 0.004;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).Enthalpy = PsyHFnTdbW(15.0, 0.004);

    CpAir = PsyCpAirFnW(0.004);
    // cycling fan
    fanOp = HVAC::FanOp::Cycling;
    InitSimpleWatertoAirHP(*state, HPNum, SensLoad, LatentLoad, fanOp, OnOffAirFlowRatio, FirstHVACIteration);

    PartLoadRatio = 1.0;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).MassFlowRate =
        ActualAirflow * PartLoadRatio;
    InitSimpleWatertoAirHP(*state, HPNum, SensLoad, LatentLoad, fanOp, OnOffAirFlowRatio, FirstHVACIteration);
    CalcHPHeatingSimple(*state, HPNum, fanOp, SensLoad, compressorOp, PartLoadRatio, OnOffAirFlowRatio);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirMassFlowRate, 1.0);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QLoadTotal, 20000 * 0.981844, 0.1);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QSensible, 20000 * 0.981844, 0.1);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->Winput, 6315.01766 * 0.981844, 0.1);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->LoadSideInletEnth, PsyHFnTdbW(15.0, 0.004));
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->LoadSideInletDBTemp, 15.0);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->LoadSideOutletDBTemp, 15.0 + (19636.8798 / 1.0 / CpAir), 0.0001);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirEnthalpy, PsyHFnTdbW(15.0, 0.004) + (19636.8798 / 1.0), 0.1);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirDBTemp, 15.0 + (19636.8798 / 1.0 / CpAir), 0.0001);

    PartLoadRatio = 0.5;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).MassFlowRate =
        ActualAirflow * PartLoadRatio;
    InitSimpleWatertoAirHP(*state, HPNum, SensLoad, LatentLoad, fanOp, OnOffAirFlowRatio, FirstHVACIteration);
    CalcHPHeatingSimple(*state, HPNum, fanOp, SensLoad, compressorOp, PartLoadRatio, OnOffAirFlowRatio);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirMassFlowRate, 0.5);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QLoadTotal, 20000 * 0.981844 * 0.5, 0.1);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QSensible, 20000 * 0.981844 * 0.5, 0.1);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->Winput, 6315.01766 * 0.981844 * 0.5, 0.1);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->LoadSideInletEnth, PsyHFnTdbW(15.0, 0.004));
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->LoadSideInletDBTemp, 15.0);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->LoadSideOutletDBTemp, 15.0 + (19636.8798 / 1.0 / CpAir), 0.0001);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirEnthalpy, PsyHFnTdbW(15.0, 0.004) + (19636.8798 / 1.0), 0.1);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirDBTemp, 15.0 + (19636.8798 / 1.0 / CpAir), 0.0001);

    // constant fan
    fanOp = HVAC::FanOp::Continuous;
    PartLoadRatio = 1.0;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).MassFlowRate =
        ActualAirflow * PartLoadRatio;
    InitSimpleWatertoAirHP(*state, HPNum, SensLoad, LatentLoad, fanOp, OnOffAirFlowRatio, FirstHVACIteration);
    CalcHPHeatingSimple(*state, HPNum, fanOp, SensLoad, compressorOp, PartLoadRatio, OnOffAirFlowRatio);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirMassFlowRate, 1.0);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QLoadTotal, 20000 * 0.981844, 0.1);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QSensible, 20000 * 0.981844, 0.1);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->Winput, 6315.01766 * 0.981844, 0.1);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->LoadSideInletEnth, PsyHFnTdbW(15.0, 0.004));
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->LoadSideInletDBTemp, 15.0);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->LoadSideOutletDBTemp, 15.0 + (19636.8798 / 1.0 / CpAir), 0.0001);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirEnthalpy, PsyHFnTdbW(15.0, 0.004) + (19636.8798 / 1.0), 0.1);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirDBTemp, 15.0 + (19636.8798 / 1.0 / CpAir), 0.0001);

    PartLoadRatio = 0.5;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).MassFlowRate = ActualAirflow;
    InitSimpleWatertoAirHP(*state, HPNum, SensLoad, LatentLoad, fanOp, OnOffAirFlowRatio, FirstHVACIteration);
    CalcHPHeatingSimple(*state, HPNum, fanOp, SensLoad, compressorOp, PartLoadRatio, OnOffAirFlowRatio);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirMassFlowRate, 1.0);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QLoadTotal, 20000 * 0.981844 * 0.5, 0.1);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).QSensible, 20000 * 0.981844 * 0.5, 0.1);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->Winput, 6315.01766 * 0.981844 * 0.5, 0.1);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->LoadSideInletEnth, PsyHFnTdbW(15.0, 0.004));
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->LoadSideInletDBTemp, 15.0);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->LoadSideOutletDBTemp, 15.0 + (19636.8798 / 1.0 / CpAir), 0.0001);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirEnthalpy,
                (PsyHFnTdbW(15.0, 0.004) + (19636.8798 / 1.0)) * 0.5 + 0.5 * PsyHFnTdbW(15.0, 0.004),
                0.1);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletAirDBTemp, 24.69937, 0.0001);

    // test CheckSimpleWAHPRatedCurvesOutputs with null pointers
    HPNum = 1;
    // ensure RatedEntAirDry/WetbulbTemp is not autosize so code will test valid versus null curve pointer
    EXPECT_NE(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedEntAirDrybulbTemp, DataSizing::AutoSize);
    EXPECT_NE(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedEntAirWetbulbTemp, DataSizing::AutoSize);
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).SensCoolCapCurve = nullptr;
    WaterToAirHeatPumpSimple::CheckSimpleWAHPRatedCurvesOutputs(*state, state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);

    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).TotalCoolCapCurve = nullptr;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).CoolPowCurve = nullptr;
    WaterToAirHeatPumpSimple::CheckSimpleWAHPRatedCurvesOutputs(*state, state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);

    HPNum = 2;
    // ensure RatedEntAirDry/WetbulbTemp is not autosize so code will test valid versus null curve pointer
    EXPECT_NE(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedEntAirDrybulbTemp, DataSizing::AutoSize);
    EXPECT_NE(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).RatedEntAirWetbulbTemp, DataSizing::AutoSize);
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).HeatCapCurve = nullptr;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).HeatPowCurve = nullptr;
    WaterToAirHeatPumpSimple::CheckSimpleWAHPRatedCurvesOutputs(*state, state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name);
}

TEST_F(EnergyPlusFixture, WaterToAirHeatPumpSimple_TestWaterFlowControl)
{

    std::string const idf_objects = delimited_string({

        " Coil:Cooling:WaterToAirHeatPump:EquationFit,",
        "   Sys 5 Heat Pump Cooling Mode,  !- Name",
        "   Sys 5 Water to Air Heat Pump Source Side1 Inlet Node,  !- Water Inlet Node Name",
        "   Sys 5 Water to Air Heat Pump Source Side1 Outlet Node,  !- Water Outlet Node Name",
        "   Sys 5 Cooling Coil Air Inlet Node,  !- Air Inlet Node Name",
        "   Sys 5 Heating Coil Air Inlet Node,  !- Air Outlet Node Name",
        "   1.0,                     !- Rated Air Flow Rate {m3/s}",
        "   0.0033,                  !- Rated Water Flow Rate {m3/s}",
        "   23125.59,                !- Gross Rated Total Cooling Capacity {W}",
        "   16267,                   !- Gross Rated Sensible Cooling Capacity {W}",
        "   7.007757577,             !- Gross Rated Cooling COP",
        "   ,                        !- Rated Entering Water Temperature",
        "   ,                        !- Rated Entering Air Dry-Bulb Temperature",
        "   ,                        !- Rated Entering Air Wet-Bulb Temperature",
        "   TotCoolCapCurve,         !- Total Cooling Capacity Curve Name",
        "   SensCoolCapCurve,        !- Sensible Cooling Capacity Curve Name",
        "   CoolPowCurve,            !- Cooling Power Consumption Curve Name",
        "   PLFFPLR,                 !- Part Load Fraction Correlation Curve Name",
        "   0,                       !- Nominal Time for Condensate Removal to Begin {s}",
        "   0;                       !- Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity {dimensionless}",

        " Coil:Heating:WaterToAirHeatPump:EquationFit,",
        "  Sys 5 Heat Pump Heating Mode,  !- Name",
        "  Sys 5 Water to Air Heat Pump Source Side2 Inlet Node,  !- Water Inlet Node Name",
        "  Sys 5 Water to Air Heat Pump Source Side2 Outlet Node,  !- Water Outlet Node Name",
        "  Sys 5 Heating Coil Air Inlet Node,  !- Air Inlet Node Name",
        "  Sys 5 SuppHeating Coil Air Inlet Node,  !- Air Outlet Node Name",
        "  1.0,                      !- Rated Air Flow Rate {m3/s}",

        "  0.0033,                   !- Rated Water Flow Rate {m3/s}",
        "  19156.73,                 !- Gross Rated Heating Capacity {W}",
        "  3.167053691,              !- Gross Rated Heating COP",
        "  ,                         !- Rated Entering Water Temperature",
        "  ,                         !- Rated Entering Air Dry-Bulb Temperature",
        "  ,                         !- Ratio of Rated Heating Capacity to Rated Cooling Capacity",
        "  HeatCapCurve,             !- Heating Capacity Curve Name",
        "  HeatPowCurve,             !- Heating Power Curve Name",
        "  PLFFPLR;                  !- Part Load Fraction Correlation Curve Name",

        "Curve:QuintLinear,",
        "  SensCoolCapCurve,     ! Curve Name",
        "  2.24209455,           ! CoefficientC1",
        "  7.28913391,           ! CoefficientC2",
        "  -9.06079896,          ! CoefficientC3",
        "  -0.36729404,          ! CoefficientC4",
        "  0.218826161,          ! CoefficientC5",
        "  0.00901534,           ! CoefficientC6",
        "  0.,                   ! Minimum Value of v",
        "  100.,                 ! Maximum Value of v",
        "  0.,                   ! Minimum Value of w",
        "  100.,                 ! Maximum Value of w",
        "  0.,                   ! Minimum Value of x",
        "  100.,                 ! Maximum Value of x",
        "  0.,                   ! Minimum Value of y",
        "  100.,                 ! Maximum Value of y",
        "  0,                    ! Minimum Value of z",
        "  100,                  ! Maximum Value of z",
        "  0.,                   ! Minimum Curve Output",
        "  38.;                  ! Maximum Curve Output",

        "Curve:QuadLinear,",
        "  TotCoolCapCurve,      ! Curve Name",
        "  -0.68126221,          ! CoefficientC1",
        "  1.99529297,           ! CoefficientC2",
        "  -0.93611888,          ! CoefficientC3",
        "  0.02081177,           ! CoefficientC4",
        "  0.008438868,          ! CoefficientC5",
        "  0.,                   ! Minimum Value of w",
        "  100.,                 ! Maximum Value of w",
        "  0.,                   ! Minimum Value of x",
        "  100.,                 ! Maximum Value of x",
        "  0.,                   ! Minimum Value of y",
        "  100.,                 ! Maximum Value of y",
        "  0,                    ! Minimum Value of z",
        "  100,                  ! Maximum Value of z",
        "  0.,                   ! Minimum Curve Output",
        "  38.;                  ! Maximum Curve Output",

        "Curve:QuadLinear,",
        "  CoolPowCurve,      ! Curve Name",
        "  -3.20456384,          ! CoefficientC1",
        "  0.47656454,           ! CoefficientC2",
        "  3.16734236,           ! CoefficientC3",
        "  0.10244637,           ! CoefficientC4",
        "  -0.038132556,         ! CoefficientC5",
        "  0.,                   ! Minimum Value of w",
        "  100.,                 ! Maximum Value of w",
        "  0.,                   ! Minimum Value of x",
        "  100.,                 ! Maximum Value of x",
        "  0.,                   ! Minimum Value of y",
        "  100.,                 ! Maximum Value of y",
        "  0,                    ! Minimum Value of z",
        "  100,                  ! Maximum Value of z",
        "  0.,                   ! Minimum Curve Output",
        "  38.;                  ! Maximum Curve Output",

        "Curve:QuadLinear,",
        "  HeatCapCurve,         ! Curve Name",
        "  -5.50102734,          ! CoefficientC1",
        "  -0.96688754,          ! CoefficientC2",
        "  7.70755007,           ! CoefficientC3",
        "  0.031928881,          ! CoefficientC4",
        "  0.028112522,          ! CoefficientC5",
        "  0.,                   ! Minimum Value of w",
        "  100.,                 ! Maximum Value of w",
        "  0.,                   ! Minimum Value of x",
        "  100.,                 ! Maximum Value of x",
        "  0.,                   ! Minimum Value of y",
        "  100.,                 ! Maximum Value of y",
        "  0,                    ! Minimum Value of z",
        "  100,                  ! Maximum Value of z",
        "  0.,                   ! Minimum Curve Output",
        "  38.;                  ! Maximum Curve Output",

        "Curve:QuadLinear,",
        "  HeatPowCurve,         ! Curve Name",
        "  -7.47517858,          ! CoefficientC1",
        "  6.40876653,           ! CoefficientC2",
        "  1.99711665,           ! CoefficientC3",
        "  -0.050682973,         ! CoefficientC4",
        "  0.011385145,          ! CoefficientC5",
        "  0.,                   ! Minimum Value of w",
        "  100.,                 ! Maximum Value of w",
        "  0.,                   ! Minimum Value of x",
        "  100.,                 ! Maximum Value of x",
        "  0.,                   ! Minimum Value of y",
        "  100.,                 ! Maximum Value of y",
        "  0,                    ! Minimum Value of z",
        "  100,                  ! Maximum Value of z",
        "  0.,                   ! Minimum Curve Output",
        "  38.;                  ! Maximum Curve Output",

        "Curve:Quadratic, PLFFPLR, 0.85, 0.83, 0.0, 0.0, 0.3, 0.85, 1.0, Dimensionless, Dimensionless; ",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetSimpleWatertoAirHPInput(*state);

    int HPNum(1);
    Real64 DesignAirflow(2.0);
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum).Temp = 5.0;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum).Enthalpy = 44650.0;

    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).DesignWaterMassFlowRate = 15.0;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum).MassFlowRate =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).DesignWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum).MassFlowRateMax =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).DesignWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum).MassFlowRateMaxAvail =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).DesignWaterMassFlowRate;

    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).MassFlowRate = DesignAirflow;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).Temp = 26.0;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).HumRat = 0.007;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).Enthalpy = 43970.75;

    state->dataPlnt->TotNumLoops = 2;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);

    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }

    state->dataPlnt->PlantLoop(1).Name = "ChilledWaterLoop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WAHPPlantType;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum;

    HVAC::CompressorOp compressorOp = HVAC::CompressorOp::On;
    HVAC::FanOp fanOp = HVAC::FanOp::Cycling;
    bool FirstHVACIteration(true);
    Real64 SensLoad(38000.0);
    Real64 LatentLoad(0.0);
    Real64 PartLoadRatio(1.0);
    Real64 OnOffAirFlowRatio(1.0);
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).plantLoc.loopNum = 1;

    state->dataEnvrn->OutBaroPress = 101325.0;

    InitSimpleWatertoAirHP(*state, HPNum, SensLoad, LatentLoad, fanOp, OnOffAirFlowRatio, FirstHVACIteration);
    CalcHPCoolingSimple(*state, HPNum, fanOp, SensLoad, LatentLoad, compressorOp, PartLoadRatio, OnOffAirFlowRatio);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate, 15.0);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletWaterTemp, 5.0);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterTemp, 5.20387, 0.00001);

    PartLoadRatio = 0.5;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).MassFlowRate =
        DesignAirflow * PartLoadRatio;
    InitSimpleWatertoAirHP(*state, HPNum, SensLoad, LatentLoad, fanOp, OnOffAirFlowRatio, FirstHVACIteration);
    CalcHPCoolingSimple(*state, HPNum, fanOp, SensLoad, LatentLoad, compressorOp, PartLoadRatio, OnOffAirFlowRatio);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate, 15.0);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletWaterTemp, 5.0);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterTemp, 5.10193, 0.00001);

    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterCyclingMode = HVAC::WaterFlow::Cycling;
    PartLoadRatio = 1.0;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).MassFlowRate =
        DesignAirflow * PartLoadRatio;
    InitSimpleWatertoAirHP(*state, HPNum, SensLoad, LatentLoad, fanOp, OnOffAirFlowRatio, FirstHVACIteration);
    CalcHPCoolingSimple(*state, HPNum, fanOp, SensLoad, LatentLoad, compressorOp, PartLoadRatio, OnOffAirFlowRatio);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate, 15.0);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletWaterTemp, 5.0);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterTemp, 5.20387, 0.00001);

    PartLoadRatio = 0.5;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).MassFlowRate =
        DesignAirflow * PartLoadRatio;
    InitSimpleWatertoAirHP(*state, HPNum, SensLoad, LatentLoad, fanOp, OnOffAirFlowRatio, FirstHVACIteration);
    CalcHPCoolingSimple(*state, HPNum, fanOp, SensLoad, LatentLoad, compressorOp, PartLoadRatio, OnOffAirFlowRatio);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate, 7.5);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletWaterTemp, 5.0);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterTemp, 5.20387, 0.00001);

    // test reduced flow at coil water inlet node
    PartLoadRatio = 0.25;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).MassFlowRate =
        DesignAirflow * PartLoadRatio;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum).MassFlowRate = 3.75;
    InitSimpleWatertoAirHP(*state, HPNum, SensLoad, LatentLoad, fanOp, OnOffAirFlowRatio, FirstHVACIteration);
    CalcHPCoolingSimple(*state, HPNum, fanOp, SensLoad, LatentLoad, compressorOp, PartLoadRatio, OnOffAirFlowRatio);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate, 3.75);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletWaterTemp, 5.0);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterTemp, 5.20387, 0.00001);
    UpdateSimpleWatertoAirHP(*state, HPNum);
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum).MassFlowRate, 3.75);
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterOutletNodeNum).MassFlowRate, 3.75);
    EXPECT_NEAR(state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterOutletNodeNum).Temp, 5.20387, 0.00001);

    HPNum = 2;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).plantLoc.loopNum = 2;
    state->dataPlnt->PlantLoop(2).Name = "HotWaterLoop";
    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WAHPPlantType;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum;

    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum).Temp = 35.0;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum).Enthalpy = 43950.0;

    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).DesignWaterMassFlowRate = 15.0;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum).MassFlowRate =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).DesignWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum).MassFlowRateMax =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).DesignWaterMassFlowRate;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum).MassFlowRateMaxAvail =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).DesignWaterMassFlowRate;

    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).MassFlowRate = DesignAirflow;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).Temp = 15.0;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).HumRat = 0.004;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).Enthalpy = PsyHFnTdbW(15.0, 0.004);

    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).DesignWaterMassFlowRate = 15.0;

    InitSimpleWatertoAirHP(*state, HPNum, SensLoad, LatentLoad, fanOp, OnOffAirFlowRatio, FirstHVACIteration);

    PartLoadRatio = 1.0;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).MassFlowRate =
        DesignAirflow * PartLoadRatio;
    InitSimpleWatertoAirHP(*state, HPNum, SensLoad, LatentLoad, fanOp, OnOffAirFlowRatio, FirstHVACIteration);
    CalcHPHeatingSimple(*state, HPNum, fanOp, SensLoad, compressorOp, PartLoadRatio, OnOffAirFlowRatio);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate, 15.0);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletWaterTemp, 35.0);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterTemp, 34.50472, 0.00001);

    PartLoadRatio = 0.5;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).MassFlowRate =
        DesignAirflow * PartLoadRatio;
    InitSimpleWatertoAirHP(*state, HPNum, SensLoad, LatentLoad, fanOp, OnOffAirFlowRatio, FirstHVACIteration);
    CalcHPHeatingSimple(*state, HPNum, fanOp, SensLoad, compressorOp, PartLoadRatio, OnOffAirFlowRatio);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate, 15.0);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletWaterTemp, 35.0);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterTemp, 34.75236, 0.00001);

    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterCyclingMode = HVAC::WaterFlow::Cycling;
    PartLoadRatio = 1.0;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).MassFlowRate =
        DesignAirflow * PartLoadRatio;
    InitSimpleWatertoAirHP(*state, HPNum, SensLoad, LatentLoad, fanOp, OnOffAirFlowRatio, FirstHVACIteration);
    CalcHPHeatingSimple(*state, HPNum, fanOp, SensLoad, compressorOp, PartLoadRatio, OnOffAirFlowRatio);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate, 15.0);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletWaterTemp, 35.0);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterTemp, 34.50472, 0.00001);

    PartLoadRatio = 0.5;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).MassFlowRate =
        DesignAirflow * PartLoadRatio;
    InitSimpleWatertoAirHP(*state, HPNum, SensLoad, LatentLoad, fanOp, OnOffAirFlowRatio, FirstHVACIteration);
    CalcHPHeatingSimple(*state, HPNum, fanOp, SensLoad, compressorOp, PartLoadRatio, OnOffAirFlowRatio);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate, 7.5);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletWaterTemp, 35.0);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterTemp, 34.50472, 0.00001);

    // test reduced flow at coil water inlet node
    PartLoadRatio = 0.25;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).AirInletNodeNum).MassFlowRate =
        DesignAirflow * PartLoadRatio;
    state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum).MassFlowRate = 3.75;
    InitSimpleWatertoAirHP(*state, HPNum, SensLoad, LatentLoad, fanOp, OnOffAirFlowRatio, FirstHVACIteration);
    CalcHPHeatingSimple(*state, HPNum, fanOp, SensLoad, compressorOp, PartLoadRatio, OnOffAirFlowRatio);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterMassFlowRate, 3.75);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).InletWaterTemp, 35.0);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).OutletWaterTemp, 34.50472, 0.00001);
    UpdateSimpleWatertoAirHP(*state, HPNum);
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterInletNodeNum).MassFlowRate, 3.75);
    EXPECT_EQ(state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterOutletNodeNum).MassFlowRate, 3.75);
    EXPECT_NEAR(
        state->dataLoopNodes->Node(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).WaterOutletNodeNum).Temp, 34.50472, 0.00001);
}

TEST_F(EnergyPlusFixture, WaterToAirHeatPumpSimpleTest_CheckSimpleWAHPRatedCurvesOutputs)
{
    int HPNum(2);
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP.allocate(HPNum);
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).Name = "WAHP";
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).WAHPType = WatertoAirHP::Cooling;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedAirVolFlowRate = AutoSize;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedCapCoolTotal = AutoSize;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedCapCoolSens = AutoSize;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedWaterVolFlowRate = 0.0;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).WaterInletNodeNum = 1;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).WaterOutletNodeNum = 2;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedEntWaterTemp = 30.0;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedEntAirWetbulbTemp = 19.0;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedEntAirDrybulbTemp = 27.0;

    auto *curve1 = AddCurve(*state, "Curve1");
    curve1->curveType = CurveType::QuadLinear;
    curve1->coeff[0] = -9.32564313298629;
    curve1->coeff[1] = 11.088084240584;
    curve1->coeff[2] = -1.75195196204063;
    curve1->coeff[3] = 0.760820340847872;
    curve1->coeff[4] = 0.0;
    curve1->inputLimits[0].min = 0.0;
    curve1->inputLimits[0].max = 80.0;
    curve1->inputLimits[1].min = 0.0;
    curve1->inputLimits[1].max = 100.0;
    curve1->inputLimits[2].min = 0.0;
    curve1->inputLimits[2].max = 2.0;
    curve1->inputLimits[3].min = 0.0;
    curve1->inputLimits[3].max = 2.0;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).TotalCoolCapCurve = curve1;

    auto *curve2 = AddCurve(*state, "Curve2");
    curve2->curveType = CurveType::QuintLinear;
    curve2->coeff[0] = -5.26562830117273;
    curve2->coeff[1] = 17.3118017582604;
    curve2->coeff[2] = -11.4496890368762;
    curve2->coeff[3] = -0.944804890543481;
    curve2->coeff[4] = 0.739606605780884;
    curve2->coeff[5] = 0.0;
    curve2->inputLimits[0].min = 0.0;
    curve2->inputLimits[0].max = 100.0;
    curve2->inputLimits[1].min = 0.0;
    curve2->inputLimits[1].max = 100.0;
    curve2->inputLimits[2].min = 0.0;
    curve2->inputLimits[2].max = 100.0;
    curve2->inputLimits[3].min = 0.0;
    curve2->inputLimits[3].max = 1.0;
    curve2->inputLimits[4].min = 0.0;
    curve2->inputLimits[4].max = 1.0;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).SensCoolCapCurve = curve2;

    auto *curve3 = AddCurve(*state, "Curve3");
    curve3->curveType = CurveType::QuadLinear;
    curve3->coeff[0] = -3.25323327026219;
    curve3->coeff[1] = -0.990977022339372;
    curve3->coeff[2] = 4.03828937789764;
    curve3->coeff[3] = 0.952179101682919;
    curve3->coeff[4] = 0.0;
    curve3->inputLimits[0].min = -100;
    curve3->inputLimits[0].max = 100;
    curve3->inputLimits[1].min = -100;
    curve3->inputLimits[1].max = 100;
    curve3->inputLimits[2].min = 0;
    curve3->inputLimits[2].max = 100;
    curve3->inputLimits[3].min = 0;
    curve3->inputLimits[3].max = 38;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).CoolPowCurve = curve3;

    CheckSimpleWAHPRatedCurvesOutputs(*state, "WAHP");

    EXPECT_TRUE(compare_err_stream("", true));

    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).Name = "WAHP 2";
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).WAHPType = WatertoAirHP::Cooling;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedAirVolFlowRate = AutoSize;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedCapCoolTotal = AutoSize;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedCapCoolSens = AutoSize;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedWaterVolFlowRate = 0.0;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).WaterInletNodeNum = 1;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).WaterOutletNodeNum = 2;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedEntWaterTemp = 30.0;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedEntAirWetbulbTemp = 19.0;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedEntAirDrybulbTemp = 27.0;

    auto *curve4 = AddCurve(*state, "Curve4");
    curve4->curveType = CurveType::QuadLinear;
    curve4->coeff[0] = -0.68126221;
    curve4->coeff[1] = 1.99529297;
    curve4->coeff[2] = -0.93611888;
    curve4->coeff[3] = 0.02081177;
    curve4->coeff[4] = 0.008438868;
    curve4->inputLimits[0].min = 0.0;
    curve4->inputLimits[0].max = 80.0;
    curve4->inputLimits[1].min = 0.0;
    curve4->inputLimits[1].max = 100.0;
    curve4->inputLimits[2].min = 0.0;
    curve4->inputLimits[2].max = 2.0;
    curve4->inputLimits[3].min = 0.0;
    curve4->inputLimits[3].max = 2.0;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).TotalCoolCapCurve = curve4;

    auto *curve5 = AddCurve(*state, "Curve5");
    curve5->curveType = CurveType::QuintLinear;
    curve5->coeff[0] = 2.24209455;
    curve5->coeff[1] = 7.28913391;
    curve5->coeff[2] = -9.06079896;
    curve5->coeff[3] = -0.36729404;
    curve5->coeff[4] = 0.218826161;
    curve5->coeff[5] = 0.00901534;
    curve5->inputLimits[0].min = 0.0;
    curve5->inputLimits[0].max = 100.0;
    curve5->inputLimits[1].min = 0.0;
    curve5->inputLimits[1].max = 100.0;
    curve5->inputLimits[2].min = 0.0;
    curve5->inputLimits[2].max = 100.0;
    curve5->inputLimits[3].min = 0.0;
    curve5->inputLimits[3].max = 1.0;
    curve5->inputLimits[4].min = 0.0;
    curve5->inputLimits[4].max = 1.0;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).SensCoolCapCurve = curve5;

    auto *curve6 = AddCurve(*state, "Curve6");
    curve6->curveType = CurveType::QuadLinear;
    curve6->coeff[0] = -3.20456384;
    curve6->coeff[1] = 0.47656454;
    curve6->coeff[2] = 3.16734236;
    curve6->coeff[3] = 0.10244637;
    curve6->coeff[4] = -0.038132556;
    curve6->inputLimits[0].min = -100;
    curve6->inputLimits[0].max = 100;
    curve6->inputLimits[1].min = -100;
    curve6->inputLimits[1].max = 100;
    curve6->inputLimits[2].min = 0;
    curve6->inputLimits[2].max = 100;
    curve6->inputLimits[3].min = 0;
    curve6->inputLimits[3].max = 38;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).CoolPowCurve = curve6;

    CheckSimpleWAHPRatedCurvesOutputs(*state, "WAHP 2");

    std::string const error_string = delimited_string(
        {"   ** Warning ** CheckSimpleWAHPRatedCurvesOutputs: Coil:Cooling:WaterToAirHeatPump:EquationFit=\"WAHP 2\"\n   **   ~~~   ** Total cooling "
         "capacity as a function of temperature curve output is not equal to 1.0 (+ or - 2%) at rated conditions.\n   **   ~~~   ** Curve output at "
         "rated conditions = 0.404\n   ** Warning ** CheckSimpleWAHPRatedCurvesOutputs: Coil:Cooling:WaterToAirHeatPump:EquationFit=\"WAHP 2\"\n   "
         "**  "
         " ~~~   ** Cooling power consumption as a function of temperature curve output is not equal to 1.0 (+ or - 2%) at rated conditions.\n   **  "
         " ~~~   ** Curve output at rated conditions = 0.742\n   ** Warning ** "
         "CheckSimpleWAHPRatedCurvesOutputs: Coil:Cooling:WaterToAirHeatPump:EquationFit=\"WAHP 2\"\n   **   ~~~   ** Sensible cooling capacity as a "
         "function of temperature curve output is not equal to 1.0 (+ or - 2%) at rated conditions.\n   **   ~~~   ** Curve output at rated "
         "conditions = 0.454"});

    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, WaterToAirHeatPumpSimpleTest_SizeHVACWaterToAirRatedConditions)
{
    state->dataFluid->init_state(*state);
    // This unit test is intended to check if the power calculated during the sizing routine
    // uses the user-specified COP at rated conditions

    int HPNum(2);

    state->dataSize->SysSizingRunDone = true;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->CurSysNum = 0;
    state->dataSize->CurZoneEqNum = 1;

    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP.allocate(HPNum);
    state->dataSize->FinalZoneSizing.allocate(state->dataSize->CurZoneEqNum);
    state->dataSize->ZoneEqSizing.allocate(state->dataSize->CurZoneEqNum);
    state->dataSize->DesDayWeath.allocate(1);
    state->dataSize->DesDayWeath(1).Temp.allocate(24);

    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).WAHPType = WatertoAirHP::Cooling;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedAirVolFlowRate = AutoSize;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedCapCoolTotal = AutoSize;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedCapCoolSens = AutoSize;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedWaterVolFlowRate = AutoSize;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).WaterInletNodeNum = 1;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).WaterOutletNodeNum = 2;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedEntWaterTemp = 30.0;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedEntAirWetbulbTemp = 19.0;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedEntAirDrybulbTemp = 27.0;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).CompanionHeatingCoilNum = 2;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).WAHPPlantType = DataPlant::PlantEquipmentType::CoilWAHPCoolingEquationFit;

    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).WAHPType = WatertoAirHP::Heating;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).RatedAirVolFlowRate = AutoSize;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).RatedCapHeat = AutoSize;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).RatedWaterVolFlowRate = AutoSize;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).WaterInletNodeNum = 3;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).WaterOutletNodeNum = 4;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).RatedEntWaterTemp = 20.0;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).RatedEntAirDrybulbTemp = 20.0;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).CompanionCoolingCoilNum = 1;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).WAHPPlantType = DataPlant::PlantEquipmentType::CoilWAHPHeatingEquationFit;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).RatioRatedHeatRatedTotCoolCap = 1.23;

    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow = 0.20;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow = 0.20;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDesTemp = 13.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).HeatDesTemp = 40;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDesHumRat = 0.0075;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).TimeStepNumAtCoolMax = 15;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDDNum = 1;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).HeatOutTemp = 2.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInTemp = 25.5;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInTemp = 2.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInHumRat = 0.0045;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInHumRat = 0.0045;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneRetTempAtCoolPeak = 25.5;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneHumRatAtCoolPeak = 0.0045;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneRetTempAtHeatPeak = 15.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneHumRatAtHeatPeak = 0.0045;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).OAVolFlow = 0.0;

    auto *curve1 = AddCurve(*state, "Curve1");
    curve1->curveType = CurveType::QuadLinear;
    curve1->coeff[0] = -9.32564313298629;
    curve1->coeff[1] = 11.088084240584;
    curve1->coeff[2] = -1.75195196204063;
    curve1->coeff[3] = 0.760820340847872;
    curve1->coeff[4] = 0.0;
    curve1->inputLimits[0].min = 0.0;
    curve1->inputLimits[0].max = 80.0;
    curve1->inputLimits[1].min = 0.0;
    curve1->inputLimits[1].max = 100.0;
    curve1->inputLimits[2].min = 0.0;
    curve1->inputLimits[2].max = 2.0;
    curve1->inputLimits[3].min = 0.0;
    curve1->inputLimits[3].max = 2.0;

    auto *curve2 = AddCurve(*state, "Curve2");
    curve2->curveType = CurveType::QuintLinear;
    curve2->coeff[0] = -5.26562830117273;
    curve2->coeff[1] = 17.3118017582604;
    curve2->coeff[2] = -11.4496890368762;
    curve2->coeff[3] = -0.944804890543481;
    curve2->coeff[4] = 0.739606605780884;
    curve2->coeff[5] = 0.0;
    curve2->inputLimits[0].min = 0.0;
    curve2->inputLimits[0].max = 100.0;
    curve2->inputLimits[1].min = 0.0;
    curve2->inputLimits[1].max = 100.0;
    curve2->inputLimits[2].min = 0.0;
    curve2->inputLimits[2].max = 100.0;
    curve2->inputLimits[3].min = 0.0;
    curve2->inputLimits[3].max = 1.0;
    curve2->inputLimits[4].min = 0.0;
    curve2->inputLimits[4].max = 1.0;

    auto *curve3 = AddCurve(*state, "Curve3");
    curve3->curveType = CurveType::QuadLinear;
    curve3->coeff[0] = -3.25323327026219;
    curve3->coeff[1] = -0.990977022339372;
    curve3->coeff[2] = 4.03828937789764;
    curve3->coeff[3] = 0.952179101682919;
    curve3->coeff[4] = 0.0;
    curve3->inputLimits[0].min = -100;
    curve3->inputLimits[0].max = 100;
    curve3->inputLimits[1].min = -100;
    curve3->inputLimits[1].max = 100;
    curve3->inputLimits[2].min = 0;
    curve3->inputLimits[2].max = 100;
    curve3->inputLimits[3].min = 0;
    curve3->inputLimits[3].max = 38;

    auto *curve4 = AddCurve(*state, "Curve4");
    curve4->curveType = CurveType::QuadLinear;
    curve4->coeff[0] = -1.30782327125798;
    curve4->coeff[1] = -2.37467612404102;
    curve4->coeff[2] = 4.00919247797279;
    curve4->coeff[3] = 0.615580752610271;
    curve4->coeff[4] = 0.0;
    curve4->inputLimits[0].min = -100;
    curve4->inputLimits[0].max = 100;
    curve4->inputLimits[1].min = -100;
    curve4->inputLimits[1].max = 100;
    curve4->inputLimits[2].min = 0;
    curve4->inputLimits[2].max = 100;
    curve4->inputLimits[3].min = 0;
    curve4->inputLimits[3].max = 38;

    auto *curve5 = AddCurve(*state, "Curve5");
    curve5->curveType = CurveType::QuadLinear;
    curve5->coeff[0] = -2.17352461285805;
    curve5->coeff[1] = 0.830808361346509;
    curve5->coeff[2] = 1.5682782658283;
    curve5->coeff[3] = 0.689709515714146;
    curve5->coeff[4] = 0.0;
    curve5->inputLimits[0].min = -100;
    curve5->inputLimits[0].max = 100;
    curve5->inputLimits[1].min = -100;
    curve5->inputLimits[1].max = 100;
    curve5->inputLimits[2].min = 0;
    curve5->inputLimits[2].max = 100;
    curve5->inputLimits[3].min = 0;
    curve5->inputLimits[3].max = 38;

    // performance curve coefficients
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).TotalCoolCapCurve = curve1;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).SensCoolCapCurve = curve2;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).CoolPowCurve = curve3;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).HeatCapCurve = curve4;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).HeatPowCurve = curve5;

    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedCOPCoolAtRatedCdts = 5.12;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).RatedCOPHeatAtRatedCdts = 3.0;

    state->dataSize->DesDayWeath(1).Temp(15) = 32.0;
    state->dataEnvrn->StdBaroPress = 101325.0;
    state->dataSize->ZoneEqDXCoil = true;

    // create and attach a plant loop
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->PlantLoop(1).Name = "Condenser Water Loop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    auto &loopside(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand));
    loopside.TotalBranches = 1;
    loopside.Branch.allocate(1);
    auto &loopsidebranch(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));

    loopsidebranch.TotalComponents = 2;
    loopsidebranch.Comp.allocate(2);

    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).WAHPPlantType;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).WaterInletNodeNum;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).plantLoc.loopNum = 1;

    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(2).Name =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(2).Type =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).WAHPPlantType;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(2).NodeNumIn =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).WaterInletNodeNum;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).plantLoc.loopNum = 1;

    // plant loop design leaving water temperature (design entering water temperature for WAHP coil)
    state->dataSize->NumPltSizInput = 1;
    state->dataSize->PlantSizData.allocate(1);
    state->dataSize->PlantSizData(1).PlantLoopName = "Condenser Water Loop";
    state->dataSize->PlantSizData(1).ExitTemp = 29.4;
    state->dataSize->PlantSizData(1).DeltaT = 5.56;

    WaterToAirHeatPumpSimple::SizeHVACWaterToAir(*state, 1);
    WaterToAirHeatPumpSimple::SizeHVACWaterToAir(*state, 2);

    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedCapCoolAtRatedCdts /
                    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedPowerCoolAtRatedCdts,
                5.12,
                0.00001);

    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedCapCoolTotal -
                    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedCapCoolAtRatedCdts,
                0.0,
                0.00001);

    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).RatedCapHeatAtRatedCdts /
                    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).RatedPowerHeatAtRatedCdts,
                3.0,
                0.00001);

    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).RatedCapHeat -
                    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).RatedCapHeatAtRatedCdts,
                0.0,
                0.00001);

    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).RatedCapHeatAtRatedCdts /
                    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedCapCoolAtRatedCdts,
                1.23,
                0.00001);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedWaterVolFlowRate -
                    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).RatedWaterVolFlowRate,
                0.0,
                0.00001);
    EXPECT_TRUE(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedWaterVolFlowRate > 0.0);
    Real64 waterVolFlowRate = max(((1 - 1 / state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).RatedCOPHeatAtRatedCdts) *
                                   state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).RatedCapHeat),
                                  ((1 + 1 / state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedCOPCoolAtRatedCdts) *
                                   state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedCapCoolTotal)) /
                              (state->dataSize->PlantSizData(1).DeltaT * 4179.88 * 995.768);
    EXPECT_NEAR(waterVolFlowRate - state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).RatedWaterVolFlowRate, 0.0, 0.00001);
}

TEST_F(EnergyPlusFixture, WaterToAirHeatPumpSimpleTest_SizeHVACWaterToAirRatedConditionsNoDesHtgAirFlow)
{
    state->dataFluid->init_state(*state);
    // This unit test is similar as above but checks if the capacities can still be
    // correctly calculated when the design heating air flow rate is 0

    int HPNum(2);

    state->dataSize->SysSizingRunDone = true;
    state->dataSize->ZoneSizingRunDone = true;
    state->dataSize->CurSysNum = 0;
    state->dataSize->CurZoneEqNum = 1;

    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP.allocate(HPNum);
    state->dataSize->FinalZoneSizing.allocate(state->dataSize->CurZoneEqNum);
    state->dataSize->ZoneEqSizing.allocate(state->dataSize->CurZoneEqNum);
    state->dataSize->DesDayWeath.allocate(1);
    state->dataSize->DesDayWeath(1).Temp.allocate(24);

    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).WAHPType = WatertoAirHP::Cooling;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedAirVolFlowRate = AutoSize;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedCapCoolTotal = AutoSize;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedCapCoolSens = AutoSize;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedWaterVolFlowRate = AutoSize;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).WaterInletNodeNum = 1;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).WaterOutletNodeNum = 2;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedEntWaterTemp = 30.0;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedEntAirWetbulbTemp = 19.0;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedEntAirDrybulbTemp = 27.0;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).CompanionHeatingCoilNum = 2;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).WAHPPlantType = DataPlant::PlantEquipmentType::CoilWAHPCoolingEquationFit;

    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).WAHPType = WatertoAirHP::Heating;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).RatedAirVolFlowRate = AutoSize;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).RatedCapHeat = AutoSize;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).RatedWaterVolFlowRate = 0.000185;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).WaterInletNodeNum = 3;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).WaterOutletNodeNum = 4;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).RatedEntWaterTemp = 20.0;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).RatedEntAirDrybulbTemp = 20.0;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).CompanionCoolingCoilNum = 1;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).WAHPPlantType = DataPlant::PlantEquipmentType::CoilWAHPHeatingEquationFit;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).RatioRatedHeatRatedTotCoolCap = 1.23;

    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow = 0.20;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow = 0.0004;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDesTemp = 13.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).HeatDesTemp = 40;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDesHumRat = 0.0075;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).TimeStepNumAtCoolMax = 15;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDDNum = 1;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).HeatOutTemp = 2.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInTemp = 25.5;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInTemp = 2.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInHumRat = 0.0045;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInHumRat = 0.0045;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneRetTempAtCoolPeak = 25.5;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneHumRatAtCoolPeak = 0.0045;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneRetTempAtHeatPeak = 15.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneHumRatAtHeatPeak = 0.0045;
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).OAVolFlow = 0.0;

    auto *curve1 = AddCurve(*state, "Curve1");
    curve1->curveType = CurveType::QuadLinear;
    curve1->coeff[0] = -9.32564313298629;
    curve1->coeff[1] = 11.088084240584;
    curve1->coeff[2] = -1.75195196204063;
    curve1->coeff[3] = 0.760820340847872;
    curve1->coeff[4] = 0.0;
    curve1->inputLimits[0].min = 0.0;
    curve1->inputLimits[0].max = 80.0;
    curve1->inputLimits[1].min = 0.0;
    curve1->inputLimits[1].max = 100.0;
    curve1->inputLimits[2].min = 0.0;
    curve1->inputLimits[2].max = 2.0;
    curve1->inputLimits[3].min = 0.0;
    curve1->inputLimits[3].max = 2.0;

    auto *curve2 = AddCurve(*state, "Curve2");
    curve2->curveType = CurveType::QuintLinear;
    curve2->coeff[0] = -5.26562830117273;
    curve2->coeff[1] = 17.3118017582604;
    curve2->coeff[2] = -11.4496890368762;
    curve2->coeff[3] = -0.944804890543481;
    curve2->coeff[4] = 0.739606605780884;
    curve2->coeff[5] = 0.0;
    curve2->inputLimits[0].min = 0.0;
    curve2->inputLimits[0].max = 100.0;
    curve2->inputLimits[1].min = 0.0;
    curve2->inputLimits[1].max = 100.0;
    curve2->inputLimits[2].min = 0.0;
    curve2->inputLimits[2].max = 100.0;
    curve2->inputLimits[3].min = 0.0;
    curve2->inputLimits[3].max = 1.0;
    curve2->inputLimits[4].min = 0.0;
    curve2->inputLimits[4].max = 1.0;

    auto *curve3 = AddCurve(*state, "Curve3");
    curve3->curveType = CurveType::QuadLinear;
    curve3->coeff[0] = -3.25323327026219;
    curve3->coeff[1] = -0.990977022339372;
    curve3->coeff[2] = 4.03828937789764;
    curve3->coeff[3] = 0.952179101682919;
    curve3->coeff[4] = 0.0;
    curve3->inputLimits[0].min = -100;
    curve3->inputLimits[0].max = 100;
    curve3->inputLimits[1].min = -100;
    curve3->inputLimits[1].max = 100;
    curve3->inputLimits[2].min = 0;
    curve3->inputLimits[2].max = 100;
    curve3->inputLimits[3].min = 0;
    curve3->inputLimits[3].max = 38;

    auto *curve4 = AddCurve(*state, "Curve4");
    curve4->curveType = CurveType::QuadLinear;
    curve4->coeff[0] = -1.30782327125798;
    curve4->coeff[1] = -2.37467612404102;
    curve4->coeff[2] = 4.00919247797279;
    curve4->coeff[3] = 0.615580752610271;
    curve4->coeff[4] = 0.0;
    curve4->inputLimits[0].min = -100;
    curve4->inputLimits[0].max = 100;
    curve4->inputLimits[1].min = -100;
    curve4->inputLimits[1].max = 100;
    curve4->inputLimits[2].min = 0;
    curve4->inputLimits[2].max = 100;
    curve4->inputLimits[3].min = 0;
    curve4->inputLimits[3].max = 38;

    auto *curve5 = AddCurve(*state, "Curve5");
    curve5->curveType = CurveType::QuadLinear;
    curve5->coeff[0] = -2.17352461285805;
    curve5->coeff[1] = 0.830808361346509;
    curve5->coeff[2] = 1.5682782658283;
    curve5->coeff[3] = 0.689709515714146;
    curve5->coeff[4] = 0.0;
    curve5->inputLimits[0].min = -100;
    curve5->inputLimits[0].max = 100;
    curve5->inputLimits[1].min = -100;
    curve5->inputLimits[1].max = 100;
    curve5->inputLimits[2].min = 0;
    curve5->inputLimits[2].max = 100;
    curve5->inputLimits[3].min = 0;
    curve5->inputLimits[3].max = 38;

    // performance curve coefficients
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).TotalCoolCapCurve = curve1;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).SensCoolCapCurve = curve2;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).CoolPowCurve = curve3;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).HeatCapCurve = curve4;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).HeatPowCurve = curve5;

    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedCOPCoolAtRatedCdts = 5.12;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).RatedCOPHeatAtRatedCdts = 3.0;

    state->dataSize->DesDayWeath(1).Temp(15) = 32.0;
    state->dataEnvrn->StdBaroPress = 101325.0;
    state->dataSize->ZoneEqDXCoil = true;

    // create and attach a plant loop
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->PlantLoop(1).Name = "Condenser Water Loop";
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    auto &loopside(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand));
    loopside.TotalBranches = 1;
    loopside.Branch.allocate(1);
    auto &loopsidebranch(state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));

    loopsidebranch.TotalComponents = 2;
    loopsidebranch.Comp.allocate(2);

    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).WAHPPlantType;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).WaterInletNodeNum;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).plantLoc.loopNum = 1;

    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(2).Name =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(2).Type =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).WAHPPlantType;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(2).NodeNumIn =
        state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).WaterInletNodeNum;
    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).plantLoc.loopNum = 1;

    // plant loop design leaving water temperature (design entering water temperature for WAHP coil)
    state->dataSize->NumPltSizInput = 1;
    state->dataSize->PlantSizData.allocate(1);
    state->dataSize->PlantSizData(1).PlantLoopName = "Condenser Water Loop";
    state->dataSize->PlantSizData(1).ExitTemp = 29.4;
    state->dataSize->PlantSizData(1).DeltaT = 5.56;

    WaterToAirHeatPumpSimple::SizeHVACWaterToAir(*state, 1);
    WaterToAirHeatPumpSimple::SizeHVACWaterToAir(*state, 2);

    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).RatedCapHeatAtRatedCdts /
                    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedCapCoolAtRatedCdts,
                1.23,
                0.00001);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).RatedWaterVolFlowRate - 0.000185, 0.0, 0.000001);
    EXPECT_NEAR(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(2).RatedWaterVolFlowRate -
                    state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(1).RatedWaterVolFlowRate,
                0.0,
                0.000001);
}

TEST_F(EnergyPlusFixture, EquationFit_Initialization)
{
    std::string const idf_objects = delimited_string({

        " Coil:Cooling:WaterToAirHeatPump:EquationFit,",
        "   Sys 5 Heat Pump Cooling Mode,  !- Name",
        "   Sys 5 Water to Air Heat Pump Source Side1 Inlet Node,  !- Water Inlet Node Name",
        "   Sys 5 Water to Air Heat Pump Source Side1 Outlet Node,  !- Water Outlet Node Name",
        "   Sys 5 Cooling Coil Air Inlet Node,  !- Air Inlet Node Name",
        "   Sys 5 Heating Coil Air Inlet Node,  !- Air Outlet Node Name",
        "   2.0,                     !- Rated Air Flow Rate {m3/s}",
        "   0.0033,                  !- Rated Water Flow Rate {m3/s}",
        "   20000,                   !- Gross Rated Total Cooling Capacity {W}",
        "   16000,                   !- Gross Rated Sensible Cooling Capacity {W}",
        "   7.007757577,             !- Gross Rated Cooling COP",
        "   ,                        !- Rated Entering Water Temperature",
        "   ,                        !- Rated Entering Air Dry-Bulb Temperature",
        "   ,                        !- Rated Entering Air Wet-Bulb Temperature",
        "   TotCoolCapCurve,         !- Total Cooling Capacity Curve Name",
        "   SensCoolCapCurve,        !- Sensible Cooling Capacity Curve Name",
        "   CoolPowCurve,            !- Cooling Power Consumption Curve Name",
        "   PLFFPLR,                 !- Part Load Fraction Correlation Curve Name",
        "   0,                       !- Nominal Time for Condensate Removal to Begin {s}",
        "   0;                       !- Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity {dimensionless}",

        "Curve:QuintLinear,",
        "  SensCoolCapCurve,     ! Curve Name",
        "  0,           ! CoefficientC1",
        "  0.2,           ! CoefficientC2",
        "  0.2,          ! CoefficientC3",
        "  0.2,          ! CoefficientC4",
        "  0.2,          ! CoefficientC5",
        "  0.2,           ! CoefficientC6",
        "  0.,                   ! Minimum Value of v",
        "  100.,                 ! Maximum Value of v",
        "  0.,                   ! Minimum Value of w",
        "  100.,                 ! Maximum Value of w",
        "  0.,                   ! Minimum Value of x",
        "  100.,                 ! Maximum Value of x",
        "  0.,                   ! Minimum Value of y",
        "  100.,                 ! Maximum Value of y",
        "  0,                    ! Minimum Value of z",
        "  100,                  ! Maximum Value of z",
        "  0.,                   ! Minimum Curve Output",
        "  38.;                  ! Maximum Curve Output",

        "Curve:QuadLinear,",
        "  TotCoolCapCurve,      ! Curve Name",
        "  0,          ! CoefficientC1",
        "  0.25,           ! CoefficientC2",
        "  0.25,          ! CoefficientC3",
        "  0.25,           ! CoefficientC4",
        "  0.25,          ! CoefficientC5",
        "  0.,                   ! Minimum Value of w",
        "  100.,                 ! Maximum Value of w",
        "  0.,                   ! Minimum Value of x",
        "  100.,                 ! Maximum Value of x",
        "  0.,                   ! Minimum Value of y",
        "  100.,                 ! Maximum Value of y",
        "  0,                    ! Minimum Value of z",
        "  100,                  ! Maximum Value of z",
        "  0.,                   ! Minimum Curve Output",
        "  38.;                  ! Maximum Curve Output",

        "Curve:QuadLinear,",
        "  CoolPowCurve,      ! Curve Name",
        "  0,          ! CoefficientC1",
        "  0.25,           ! CoefficientC2",
        "  0.25,          ! CoefficientC3",
        "  0.25,           ! CoefficientC4",
        "  0.25,          ! CoefficientC5",
        "  0.,                   ! Minimum Value of w",
        "  100.,                 ! Maximum Value of w",
        "  0.,                   ! Minimum Value of x",
        "  100.,                 ! Maximum Value of x",
        "  0.,                   ! Minimum Value of y",
        "  100.,                 ! Maximum Value of y",
        "  0,                    ! Minimum Value of z",
        "  100,                  ! Maximum Value of z",
        "  0.,                   ! Minimum Curve Output",
        "  38.;                  ! Maximum Curve Output",

        "Curve:Quadratic, PLFFPLR, 0.85, 0.83, 0.0, 0.0, 0.3, 0.85, 1.0, Dimensionless, Dimensionless; ",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    std::string CurrentModuleObject = "Coil:Cooling:DX:VariableSpeed";
    int num_coils = state->dataInputProcessing->inputProcessor->getNumObjectsFound(*state, CurrentModuleObject);
    ASSERT_EQ(0, num_coils);
    CurrentModuleObject = "Coil:Cooling:WaterToAirHeatPump:EquationFit";
    num_coils = state->dataInputProcessing->inputProcessor->getNumObjectsFound(*state, CurrentModuleObject);
    ASSERT_EQ(1, num_coils);
    int TotalArgs = 0;
    int NumAlphas = 0;
    int NumNumbers = 0;
    state->dataInputProcessing->inputProcessor->getObjectDefMaxArgs(*state, CurrentModuleObject, TotalArgs, NumAlphas, NumNumbers);
    EXPECT_EQ(TotalArgs, 22);
    EXPECT_EQ(NumAlphas, 9);
    EXPECT_EQ(NumNumbers, 13);

    WaterToAirHeatPumpSimple::GetSimpleWatertoAirHPInput(*state);
    int HPNum(1);
    EXPECT_EQ(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum).Name, "SYS 5 HEAT PUMP COOLING MODE");
    auto &thisCoil(state->dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(HPNum));
    EXPECT_NEAR(thisCoil.RatedCOPCoolAtRatedCdts, 7.00776, 0.01);
}
