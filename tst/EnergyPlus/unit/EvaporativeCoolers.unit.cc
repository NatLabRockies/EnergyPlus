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

// EnergyPlus::Evaporative Cooler Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/EvaporativeCoolers.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/SimAirServingZones.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;
using namespace EnergyPlus::Curve;
using namespace EnergyPlus::DataEnvironment;
using namespace EnergyPlus::Psychrometrics;
using namespace EnergyPlus::DataSizing;
using namespace EnergyPlus::DataAirSystems;
using namespace EnergyPlus::EvaporativeCoolers;

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, EvapCoolers_SecondaryAirOutletCondition)
{
    state->init_state(*state);
    auto &EvapCond(state->dataEvapCoolers->EvapCond);

    EvapCond.allocate(1);
    int constexpr EvapCoolNum(1);
    EvapCond(EvapCoolNum).SecInletEnthalpy = 42000.0;

    // set up arguments
    OperatingMode OperatingMode(EvaporativeCoolers::OperatingMode::None);
    Real64 AirMassFlowSec(0.0);
    Real64 constexpr EDBTSec(20.0);
    Real64 constexpr EWBTSec(15.0);
    Real64 constexpr EHumRatSec(0.0085);
    Real64 QHXTotal(0.0);
    Real64 QHXLatent(0.0);

    // make the call for zero secondary air flow rate
    CalcSecondaryAirOutletCondition(*state, EvapCoolNum, OperatingMode, AirMassFlowSec, EDBTSec, EWBTSec, EHumRatSec, QHXTotal, QHXLatent);

    // check outputs for evap cooler set off
    EXPECT_DOUBLE_EQ(EvapCond(EvapCoolNum).SecOutletEnthalpy, EvapCond(EvapCoolNum).SecInletEnthalpy);
    EXPECT_DOUBLE_EQ(0.0, QHXLatent);

    // dry operating mode and non zero secondary air flow rate
    OperatingMode = EvaporativeCoolers::OperatingMode::DryFull;
    AirMassFlowSec = 2.0;
    QHXTotal = 10206.410750000941;

    // make the call for dry operating mode
    CalcSecondaryAirOutletCondition(*state, EvapCoolNum, OperatingMode, AirMassFlowSec, EDBTSec, EWBTSec, EHumRatSec, QHXTotal, QHXLatent);

    // check outputs for dry operating condition
    EXPECT_NEAR(25.0, EvapCond(EvapCoolNum).SecOutletTemp, 0.000001);
    EXPECT_DOUBLE_EQ(0.0, QHXLatent);

    // wet operating mode and non zero secondary air flow rate
    OperatingMode = EvaporativeCoolers::OperatingMode::WetFull;
    AirMassFlowSec = 2.0;
    QHXTotal = 10206.410750000941;

    // make the call for wet operating condition
    CalcSecondaryAirOutletCondition(*state, EvapCoolNum, OperatingMode, AirMassFlowSec, EDBTSec, EWBTSec, EHumRatSec, QHXTotal, QHXLatent);

    // check outputs for wet operating condition
    EXPECT_DOUBLE_EQ(20.0, EvapCond(EvapCoolNum).SecOutletTemp);
    EXPECT_DOUBLE_EQ(47103.205375000471, EvapCond(EvapCoolNum).SecOutletEnthalpy);
    EXPECT_DOUBLE_EQ(QHXTotal, QHXLatent);

    EvapCond.deallocate();
}

TEST_F(EnergyPlusFixture, EvapCoolers_IndEvapCoolerOutletTemp)
{
    state->init_state(*state);
    auto &EvapCond(state->dataEvapCoolers->EvapCond);

    int constexpr EvapCoolNum(1);
    EvapCond.allocate(EvapCoolNum);

    state->dataEnvrn->OutBaroPress = 101325.0;
    EvapCond(EvapCoolNum).InletMassFlowRate = 1.0;
    EvapCond(EvapCoolNum).InletTemp = 24.0;
    EvapCond(EvapCoolNum).InletHumRat = 0.013;
    EvapCond(EvapCoolNum).DryCoilMaxEfficiency = 0.8;

    // set up arguments
    OperatingMode DryOrWetOperatingMode(EvaporativeCoolers::OperatingMode::DryFull);
    Real64 constexpr AirMassFlowSec(1.0);
    Real64 constexpr EDBTSec(14.0);
    Real64 constexpr EWBTSec(11.0);
    Real64 constexpr EHumRatSec(0.0075);

    // testing full capacity in dry operating mode
    CalcIndirectRDDEvapCoolerOutletTemp(*state, EvapCoolNum, DryOrWetOperatingMode, AirMassFlowSec, EDBTSec, EWBTSec, EHumRatSec);

    EXPECT_DOUBLE_EQ(16.0, EvapCond(EvapCoolNum).OutletTemp);

    // testing full capacity in wet operating mode
    DryOrWetOperatingMode = EvaporativeCoolers::OperatingMode::WetFull;
    EvapCond(EvapCoolNum).WetCoilMaxEfficiency = 0.75;

    CalcIndirectRDDEvapCoolerOutletTemp(*state, EvapCoolNum, DryOrWetOperatingMode, AirMassFlowSec, EDBTSec, EWBTSec, EHumRatSec);

    EXPECT_DOUBLE_EQ(14.25, EvapCond(EvapCoolNum).OutletTemp);

    EvapCond.deallocate();
}

TEST_F(EnergyPlusFixture, EvapCoolers_SizeIndEvapCoolerTest)
{
    state->init_state(*state);
    auto &EvapCond(state->dataEvapCoolers->EvapCond);

    int constexpr EvapCoolNum(1);
    Real64 PrimaryAirDesignFlow(0.0);
    Real64 SecondaryAirDesignFlow(0.0);

    state->dataSize->SysSizingRunDone = true;
    state->dataSize->NumSysSizInput = 1;
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->SysSizInput(1).AirLoopNum = 1;

    // allocate
    state->dataSize->CurSysNum = 1;
    state->dataSize->FinalSysSizing.allocate(state->dataSize->CurSysNum);
    state->dataSize->FinalSysSizing(1).DesMainVolFlow = 1.0;
    state->dataSize->FinalSysSizing(1).DesOutAirVolFlow = 0.4;
    state->dataAirSystemsData->PrimaryAirSystems.allocate(state->dataSize->CurSysNum);
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).Branch.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).Branch(1).Comp.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp(1).Name = "INDRDD EVAP COOLER";

    // Set Primary Air Data
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).NumBranches = 1;
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).Branch(1).TotalComponents = 1;

    std::string const idf_objects = delimited_string({
        "	EvaporativeCooler:Indirect:ResearchSpecial,",
        "	IndRDD Evap Cooler,  !- Name",
        "	,			         !- Availability Schedule Name",
        "	0.750,				 !- Cooler Wetbulb Design Effectiveness",
        "	,					 !- Wetbulb Effectiveness Flow Ratio Modifier Curve Name",
        "	,					 !- Cooler Drybulb Design Effectiveness",
        "	,					 !- Drybulb Effectiveness Flow Ratio Modifier Curve Name",
        "	30.0,				 !- Recirculating Water Pump Design Power { W }",
        "	,					 !- Water Pump Power Sizing Factor",
        "	,					 !- Water Pump Power Modifier Curve Name",
        "	autosize,			 !- Secondary Air Design Flow Rate { m3 / s }",
        "	1.2,				 !- Secondary Air Flow Sizing Factor",
        "	autosize,			 !- Secondary Air Fan Design Power",
        "	207.6,				 !- Secondary Air Fan Sizing Specific Power",
        "	,					 !- Secondary Fan Power Modifier Curve Name",
        "	PriAir Inlet Node,	 !- Primary Air Inlet Node Name",
        "	PriAir Outlet Node,	 !- Primary Air Outlet Node Name",
        "	autosize,			 !- Primary Air Design Air Flow Rate",
        "	0.90,				 !- Dewpoint Effectiveness Factor",
        "	SecAir Inlet Node,   !- Secondary Air Inlet Node Name",
        "	SecAir Outlet Node,  !- Secondary Air Outlet Node Name",
        "	PriAir Outlet Node,	 !- Sensor Node Name",
        "	,					 !- Relief Air Inlet Node Name",
        "	,					 !- Water Supply Storage Tank Name",
        "	0.0,				 !- Drift Loss Fraction",
        "	3;                   !- Blowdown Concentration Ratio",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    GetEvapInput(*state);

    // Set Parameters for Evap Cooler on Main Air Loop System
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).Branch(1).Comp(1).Name = EvapCond(EvapCoolNum).Name;
    EvapCond(EvapCoolNum).DesVolFlowRate = DataSizing::AutoSize;
    EvapCond(EvapCoolNum).IndirectVolFlowRate = DataSizing::AutoSize;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesMainVolFlow = 1.0;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesOutAirVolFlow = 0.2;
    PrimaryAirDesignFlow = state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesMainVolFlow;
    SecondaryAirDesignFlow = PrimaryAirDesignFlow * EvapCond(EvapCoolNum).IndirectVolFlowScalingFactor;

    // Test Indirect Evaporative Cooler Primary/Secondary Air Design Flow Rate on Main Air Loop
    SizeEvapCooler(*state, EvapCoolNum);
    EXPECT_EQ(PrimaryAirDesignFlow, EvapCond(EvapCoolNum).DesVolFlowRate);
    EXPECT_EQ(SecondaryAirDesignFlow, EvapCond(EvapCoolNum).IndirectVolFlowRate);

    // Set Parameters for Evap Cooler on OA System
    EvapCond(EvapCoolNum).Name = "EvapCool On OA System", EvapCond(EvapCoolNum).DesVolFlowRate = DataSizing::AutoSize;
    EvapCond(EvapCoolNum).IndirectVolFlowRate = DataSizing::AutoSize;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesMainVolFlow = 1.0;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesOutAirVolFlow = 0.2;
    PrimaryAirDesignFlow = state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesOutAirVolFlow;
    SecondaryAirDesignFlow = max(PrimaryAirDesignFlow, 0.5 * state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesMainVolFlow);
    SecondaryAirDesignFlow = SecondaryAirDesignFlow * EvapCond(EvapCoolNum).IndirectVolFlowScalingFactor;

    // Test Indirect Evaporative Cooler Primary/Secondary Air Design Flow Rate on OA System
    SizeEvapCooler(*state, EvapCoolNum);
    EXPECT_EQ(0.5, EvapCond(EvapCoolNum).DesVolFlowRate);
    EXPECT_EQ(SecondaryAirDesignFlow, EvapCond(EvapCoolNum).IndirectVolFlowRate);

    EvapCond.deallocate();
    state->dataAirSystemsData->PrimaryAirSystems.deallocate();
    state->dataSize->FinalSysSizing.deallocate();
}

TEST_F(EnergyPlusFixture, EvapCoolers_SizeDirEvapCoolerTest)
{
    state->init_state(*state);
    auto &EvapCond(state->dataEvapCoolers->EvapCond);

    int constexpr EvapCoolNum(1);
    Real64 PrimaryAirDesignFlow(0.0);
    Real64 RecirWaterPumpDesignPower(0.0);

    state->dataSize->SysSizingRunDone = true;
    state->dataSize->NumSysSizInput = 1;
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->SysSizInput(1).AirLoopNum = 1;

    state->dataSize->CurSysNum = 1;
    state->dataSize->FinalSysSizing.allocate(state->dataSize->CurSysNum);
    state->dataSize->FinalSysSizing(1).DesMainVolFlow = 1.0;
    state->dataSize->FinalSysSizing(1).DesOutAirVolFlow = 0.4;
    state->dataAirSystemsData->PrimaryAirSystems.allocate(state->dataSize->CurSysNum);
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).Branch.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).Branch(1).Comp.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp(1).Name = "DIRECTEVAPCOOLER";

    // Set Primary Air Data
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).NumBranches = 1;
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).Branch(1).TotalComponents = 1;

    std::string const idf_objects = delimited_string({
        "	EvaporativeCooler:Direct:ResearchSpecial,",
        "	DirectEvapCooler,    !- Name",
        "	,			         !- Availability Schedule Name",
        "	0.7,				 !- Cooler Design Effectiveness",
        "	,					 !- Effectiveness Flow Ratio Modifier Curve Name",
        "	autosize,			 !- Primary Air Design Flow Rate",
        "	autosize,			 !- Recirculating Water Pump Power Consumption { W }",
        "	55.0,				 !- Water Pump Power Sizing Factor",
        "	,					 !- Water Pump Power Modifier Curve Name",
        "	Fan Outlet Node,     !- Air Inlet Node Name",
        "	Zone Inlet Node,	 !- Air Outlet Node Name",
        "	Zone Inlet Node,	 !- Sensor Node Name",
        "	,					 !- Water Supply Storage Tank Name",
        "	0.0,				 !- Drift Loss Fraction",
        "	3;                   !- Blowdown Concentration Ratio",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    GetEvapInput(*state);
    // check autosized input fields from idf snippet read
    EXPECT_EQ(DataSizing::AutoSize, EvapCond(EvapCoolNum).DesVolFlowRate);
    EXPECT_EQ(DataSizing::AutoSize, EvapCond(EvapCoolNum).RecircPumpPower);

    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).Branch(1).Comp(1).Name = EvapCond(EvapCoolNum).Name;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesMainVolFlow = 0.50;
    PrimaryAirDesignFlow = state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesMainVolFlow;
    RecirWaterPumpDesignPower = PrimaryAirDesignFlow * EvapCond(EvapCoolNum).RecircPumpSizingFactor;

    // Test Direct Evaporative Cooler Primary Air Design Flow Rate sizing
    SizeEvapCooler(*state, 1);
    EXPECT_EQ(PrimaryAirDesignFlow, EvapCond(EvapCoolNum).DesVolFlowRate);

    EXPECT_EQ(RecirWaterPumpDesignPower, EvapCond(EvapCoolNum).RecircPumpPower);

    EvapCond.deallocate();
    state->dataAirSystemsData->PrimaryAirSystems.deallocate();
    state->dataSize->FinalSysSizing.deallocate();
}

TEST_F(EnergyPlusFixture, EvaporativeCoolers_CalcSecondaryAirOutletCondition)
{
    state->init_state(*state);
    auto &EvapCond(state->dataEvapCoolers->EvapCond);

    EvapCond.allocate(1);
    int constexpr EvapCoolNum(1);
    EvapCond(EvapCoolNum).SecInletEnthalpy = 42000.0;

    // set up arguments
    OperatingMode OperatingMode(EvaporativeCoolers::OperatingMode::None);
    Real64 AirMassFlowSec(0.0);
    Real64 constexpr EDBTSec(20.0);
    Real64 constexpr EWBTSec(15.0);
    Real64 constexpr EHumRatSec(0.0085);
    Real64 QHXTotal(0.0);
    Real64 QHXLatent(0.0);

    // make the call for zero secondary air flow rate
    EvaporativeCoolers::CalcSecondaryAirOutletCondition(
        *state, EvapCoolNum, OperatingMode, AirMassFlowSec, EDBTSec, EWBTSec, EHumRatSec, QHXTotal, QHXLatent);

    // check outputs for evap cooler set off
    EXPECT_DOUBLE_EQ(EvapCond(EvapCoolNum).SecOutletEnthalpy, EvapCond(EvapCoolNum).SecInletEnthalpy);
    EXPECT_DOUBLE_EQ(0.0, QHXLatent);

    // dry operating mode and non zero secondary air flow rate
    OperatingMode = EvaporativeCoolers::OperatingMode::DryFull;
    AirMassFlowSec = 2.0;
    QHXTotal = 10206.410750000941;

    // make the call for dry operating mode
    EvaporativeCoolers::CalcSecondaryAirOutletCondition(
        *state, EvapCoolNum, OperatingMode, AirMassFlowSec, EDBTSec, EWBTSec, EHumRatSec, QHXTotal, QHXLatent);

    // check outputs for dry operating condition
    EXPECT_NEAR(25.0, EvapCond(EvapCoolNum).SecOutletTemp, 0.000001);
    EXPECT_DOUBLE_EQ(0.0, QHXLatent);

    // wet operating mode and non zero secondary air flow rate
    OperatingMode = EvaporativeCoolers::OperatingMode::WetFull;
    AirMassFlowSec = 2.0;
    QHXTotal = 10206.410750000941;

    // make the call for wet operating condition
    EvaporativeCoolers::CalcSecondaryAirOutletCondition(
        *state, EvapCoolNum, OperatingMode, AirMassFlowSec, EDBTSec, EWBTSec, EHumRatSec, QHXTotal, QHXLatent);

    // check outputs for wet operating condition
    EXPECT_DOUBLE_EQ(20.0, EvapCond(EvapCoolNum).SecOutletTemp);
    EXPECT_DOUBLE_EQ(47103.205375000471, EvapCond(EvapCoolNum).SecOutletEnthalpy);
    EXPECT_DOUBLE_EQ(QHXTotal, QHXLatent);

    EvapCond.deallocate();
}

TEST_F(EnergyPlusFixture, EvaporativeCoolers_CalcIndirectRDDEvapCoolerOutletTemp)
{
    state->init_state(*state);
    auto &EvapCond(state->dataEvapCoolers->EvapCond);

    state->dataEnvrn->OutBaroPress = 101325.0;
    EvapCond.allocate(1);
    int constexpr EvapCoolNum(1);
    EvapCond(EvapCoolNum).InletMassFlowRate = 1.0;
    EvapCond(EvapCoolNum).InletTemp = 24.0;
    EvapCond(EvapCoolNum).InletHumRat = 0.013;
    EvapCond(EvapCoolNum).DryCoilMaxEfficiency = 0.8;

    // set up arguments
    OperatingMode DryOrWetOperatingMode(EvaporativeCoolers::OperatingMode::DryFull);
    Real64 constexpr AirMassFlowSec(1.0);
    Real64 constexpr EDBTSec(14.0);
    Real64 constexpr EWBTSec(11.0);
    Real64 constexpr EHumRatSec(0.0075);

    // testing full capacity in dry operating mode
    EvaporativeCoolers::CalcIndirectRDDEvapCoolerOutletTemp(*state, EvapCoolNum, DryOrWetOperatingMode, AirMassFlowSec, EDBTSec, EWBTSec, EHumRatSec);

    EXPECT_DOUBLE_EQ(16.0, EvapCond(EvapCoolNum).OutletTemp);

    // testing full capacity in wet operating mode
    DryOrWetOperatingMode = EvaporativeCoolers::OperatingMode::WetFull;
    EvapCond(EvapCoolNum).WetCoilMaxEfficiency = 0.75;

    EvaporativeCoolers::CalcIndirectRDDEvapCoolerOutletTemp(*state, EvapCoolNum, DryOrWetOperatingMode, AirMassFlowSec, EDBTSec, EWBTSec, EHumRatSec);

    EXPECT_DOUBLE_EQ(14.25, EvapCond(EvapCoolNum).OutletTemp);

    EvapCond.deallocate();
}

TEST_F(EnergyPlusFixture, EvaporativeCoolers_IndEvapCoolerPower)
{
    state->init_state(*state);
    auto &EvapCond(state->dataEvapCoolers->EvapCond);

    EvapCond.allocate(1);
    int constexpr EvapCoolNum(1);
    EvapCond(EvapCoolNum).IndirectFanPower = 200.0;
    EvapCond(EvapCoolNum).IndirectRecircPumpPower = 100.0;

    // set up arguments
    OperatingMode DryWetMode(EvaporativeCoolers::OperatingMode::DryFull);
    Real64 FlowRatio(1.0);

    auto *curve = AddCurve(*state, "Curve1");
    EvapCond(EvapCoolNum).FanPowerModifierCurve = curve;

    curve->curveType = CurveType::Quadratic;
    curve->coeff[0] = 0.0;
    curve->coeff[1] = 1.0;
    curve->coeff[2] = 0.0;
    curve->coeff[3] = 0.0;
    curve->coeff[4] = 0.0;
    curve->coeff[5] = 0.0;
    curve->inputLimits[0].min = 0.0;
    curve->inputLimits[0].max = 1.0;
    curve->inputLimits[1].min = 0;
    curve->inputLimits[1].max = 0;

    // make the call for dry full load operating condition
    EvapCond(EvapCoolNum).EvapCoolerPower = EvaporativeCoolers::IndEvapCoolerPower(*state, EvapCoolNum, DryWetMode, FlowRatio);

    // check outputs for dry full load operating condition
    EXPECT_EQ(200.0, EvapCond(EvapCoolNum).EvapCoolerPower);

    // set up arguments for wet modulated operating condition
    DryWetMode = EvaporativeCoolers::OperatingMode::WetModulated;
    FlowRatio = 0.8;
    EvapCond(EvapCoolNum).PartLoadFract = 0.5;

    // make the call for wet modulated operating condition
    EvapCond(EvapCoolNum).EvapCoolerPower = EvaporativeCoolers::IndEvapCoolerPower(*state, EvapCoolNum, DryWetMode, FlowRatio);

    // check outputs for wet modulated operating condition
    // Power expected = curved fan power + linear scaled pump power
    EXPECT_EQ(200 * 0.8 + 100 * 0.8 * 0.5, EvapCond(EvapCoolNum).EvapCoolerPower);

    EvapCond.deallocate();
    state->dataCurveManager->curves.deallocate();
}

TEST_F(EnergyPlusFixture, EvaporativeCoolers_SizeEvapCooler)
{
    state->init_state(*state);
    auto &EvapCond(state->dataEvapCoolers->EvapCond);

    // one-time setup of evap cooler instance
    int constexpr EvapCoolNum(1);
    EvapCond.allocate(EvapCoolNum);
    auto &thisEvapCooler = EvapCond(EvapCoolNum);

    // set up sizing stuff
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->ZoneSizingRunDone = false;
    state->dataSize->CurSysNum = 1;
    state->dataSize->NumSysSizInput = 1;
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->SysSizInput(1).AirLoopNum = 1;
    state->dataAirSystemsData->PrimaryAirSystems.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).NumBranches = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).TotalComponents = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp(1).Name = "MyEvapCooler";
    thisEvapCooler.Name = "MyEvapCooler";
    state->dataSize->FinalSysSizing.allocate(1);
    state->dataSize->FinalSysSizing(1).DesMainVolFlow = 1.0;
    state->dataSize->FinalSysSizing(1).DesOutAirVolFlow = 0.4;

    // set up the structure to size the flow rates for an RDDSpecial
    thisEvapCooler.evapCoolerType = EvapCoolerType::IndirectRDDSpecial;
    thisEvapCooler.DesVolFlowRate = DataSizing::AutoSize;
    thisEvapCooler.PadArea = 0.0;
    thisEvapCooler.PadDepth = 0.0;
    thisEvapCooler.IndirectPadArea = 0.0;
    thisEvapCooler.IndirectPadDepth = 0.0;
    thisEvapCooler.IndirectVolFlowRate = DataSizing::AutoSize;
    thisEvapCooler.IndirectVolFlowScalingFactor = 0.3;

    // make the call for sizing the flow rates
    EvaporativeCoolers::SizeEvapCooler(*state, EvapCoolNum);
    EXPECT_NEAR(0.3, thisEvapCooler.IndirectVolFlowRate, 0.0001);
    EXPECT_NEAR(1.0, thisEvapCooler.DesVolFlowRate, 0.0001);

    // now let's try to size some of the pad properties
    thisEvapCooler.evapCoolerType = EvapCoolerType::DirectCELDEKPAD;
    thisEvapCooler.DesVolFlowRate = 1.0;
    thisEvapCooler.PadArea = DataSizing::AutoSize;
    thisEvapCooler.PadDepth = DataSizing::AutoSize;
    thisEvapCooler.IndirectPadArea = 0.0;
    thisEvapCooler.IndirectPadDepth = 0.0;
    thisEvapCooler.IndirectVolFlowRate = 1.0;

    // make the call for sizing the pad properties
    EvaporativeCoolers::SizeEvapCooler(*state, EvapCoolNum);
    EXPECT_NEAR(0.333333, thisEvapCooler.PadArea, 0.0001);
    EXPECT_NEAR(0.17382, thisEvapCooler.PadDepth, 0.0001);

    // now let's try 'not' finding it on the air loop; thus it is in the OA path
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp(1).Name = "NOT-MyEvapCooler";

    // set up the structure to size the flow rates for an indirect celdekpad
    thisEvapCooler.evapCoolerType = EvapCoolerType::IndirectCELDEKPAD;
    thisEvapCooler.DesVolFlowRate = DataSizing::AutoSize;
    thisEvapCooler.PadArea = 0.0;
    thisEvapCooler.PadDepth = 0.0;
    thisEvapCooler.IndirectPadArea = 0.0;
    thisEvapCooler.IndirectPadDepth = 0.0;
    thisEvapCooler.IndirectVolFlowRate = DataSizing::AutoSize;
    thisEvapCooler.IndirectVolFlowScalingFactor = 0.3;

    // make the call for sizing the flow rates
    EvaporativeCoolers::SizeEvapCooler(*state, EvapCoolNum);
    EXPECT_NEAR(0.5, thisEvapCooler.IndirectVolFlowRate, 0.0001);
    EXPECT_NEAR(0.5, thisEvapCooler.DesVolFlowRate, 0.0001);

    // clean up
    EvapCond.deallocate();
    state->dataSize->FinalSysSizing.deallocate();
    state->dataAirSystemsData->PrimaryAirSystems.deallocate();
    state->dataSize->SysSizInput.deallocate();
}

TEST_F(EnergyPlusFixture, DefaultAutosizeIndEvapCoolerTest)
{
    state->init_state(*state);

    int constexpr EvapCoolNum(1);
    Real64 PrimaryAirDesignFlow(0.0);
    Real64 SecondaryAirDesignFlow(0.0);
    Real64 SecondaryFanPower(0.0);
    Real64 RecirculatingWaterPumpPower(0.0);

    state->dataSize->SysSizingRunDone = true;
    state->dataSize->NumSysSizInput = 1;
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->SysSizInput(1).AirLoopNum = 1;

    // allocate
    state->dataSize->CurSysNum = 1;
    state->dataSize->FinalSysSizing.allocate(state->dataSize->CurSysNum);
    state->dataSize->FinalSysSizing(1).DesMainVolFlow = 1.0;
    state->dataSize->FinalSysSizing(1).DesOutAirVolFlow = 0.4;
    state->dataAirSystemsData->PrimaryAirSystems.allocate(state->dataSize->CurSysNum);
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).Branch.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).Branch(1).Comp.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp(1).Name = "INDRDD EVAP COOLER";

    // Set Primary Air Data
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).NumBranches = 1;
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).Branch(1).TotalComponents = 1;

    std::string const idf_objects = delimited_string({
        "	EvaporativeCooler:Indirect:ResearchSpecial,",
        "	IndRDD Evap Cooler,  !- Name",
        "	,			         !- Availability Schedule Name",
        "	0.750,				 !- Cooler Wetbulb Design Effectiveness",
        "	,					 !- Wetbulb Effectiveness Flow Ratio Modifier Curve Name",
        "	,					 !- Cooler Drybulb Design Effectiveness",
        "	,					 !- Drybulb Effectiveness Flow Ratio Modifier Curve Name",
        "	,   				 !- Recirculating Water Pump Design Power { W }",
        "	,					 !- Water Pump Power Sizing Factor",
        "	,					 !- Water Pump Power Modifier Curve Name",
        "	,        			 !- Secondary Air Design Flow Rate { m3 / s }",
        "	1.2,				 !- Secondary Air Flow Sizing Factor",
        "	,        			 !- Secondary Air Fan Design Power",
        "	207.6,				 !- Secondary Air Fan Sizing Specific Power",
        "	,					 !- Secondary Fan Power Modifier Curve Name",
        "	PriAir Inlet Node,	 !- Primary Air Inlet Node Name",
        "	PriAir Outlet Node,	 !- Primary Air Outlet Node Name",
        "	,       			 !- Primary Air Design Air Flow Rate",
        "	0.90,				 !- Dewpoint Effectiveness Factor",
        "	SecAir Inlet Node,   !- Secondary Air Inlet Node Name",
        "	SecAir Outlet Node,  !- Secondary Air Outlet Node Name",
        "	PriAir Outlet Node,	 !- Sensor Node Name",
        "	,					 !- Relief Air Inlet Node Name",
        "	,					 !- Water Supply Storage Tank Name",
        "	0.0,				 !- Drift Loss Fraction",
        "	3;                   !- Blowdown Concentration Ratio",

        "Schedule:Constant,",
        "  ALWAYS_ON,    !- Name",
        "  ,             !- Schedule Type Limits Name",
        "  1.0;          !- Hourly Value",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    GetEvapInput(*state);

    auto &EvapCond(state->dataEvapCoolers->EvapCond);

    // check blank autosizable input fields default to autosize
    EXPECT_EQ(DataSizing::AutoSize, EvapCond(EvapCoolNum).DesVolFlowRate);
    EXPECT_EQ(DataSizing::AutoSize, EvapCond(EvapCoolNum).IndirectVolFlowRate);
    EXPECT_EQ(DataSizing::AutoSize, EvapCond(EvapCoolNum).IndirectFanPower);
    EXPECT_EQ(DataSizing::AutoSize, EvapCond(EvapCoolNum).IndirectRecircPumpPower);

    // Set Parameters for Evap Cooler on Main Air Loop System
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).Branch(1).Comp(1).Name = EvapCond(EvapCoolNum).Name;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesMainVolFlow = 1.0;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesOutAirVolFlow = 0.2;
    PrimaryAirDesignFlow = state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesMainVolFlow;
    SecondaryAirDesignFlow = PrimaryAirDesignFlow * EvapCond(EvapCoolNum).IndirectVolFlowScalingFactor;

    // Test Indirect Evaporative Cooler Primary/Secondary Air Design Flow Rate on Main Air Loop
    SizeEvapCooler(*state, EvapCoolNum);
    EXPECT_EQ(PrimaryAirDesignFlow, EvapCond(EvapCoolNum).DesVolFlowRate);
    EXPECT_EQ(SecondaryAirDesignFlow, EvapCond(EvapCoolNum).IndirectVolFlowRate);
    // Test Secondary Fan Power and reciculating water pump power
    SecondaryFanPower = SecondaryAirDesignFlow * EvapCond(EvapCoolNum).FanSizingSpecificPower;
    RecirculatingWaterPumpPower = SecondaryAirDesignFlow * EvapCond(EvapCoolNum).RecircPumpSizingFactor;

    // SizeEvapCooler(*state,  EvapCoolNum );
    EXPECT_EQ(SecondaryFanPower, EvapCond(EvapCoolNum).IndirectFanPower);
    EXPECT_EQ(RecirculatingWaterPumpPower, EvapCond(EvapCoolNum).IndirectRecircPumpPower);

    EvapCond.deallocate();
    state->dataAirSystemsData->PrimaryAirSystems.deallocate();
    state->dataSize->FinalSysSizing.deallocate();
}

TEST_F(EnergyPlusFixture, DefaultAutosizeDirEvapCoolerTest)
{
    state->init_state(*state);

    int constexpr EvapCoolNum(1);
    Real64 PrimaryAirDesignFlow(0.0);
    Real64 RecirWaterPumpDesignPower(0.0);

    state->dataSize->SysSizingRunDone = true;
    state->dataSize->NumSysSizInput = 1;
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->SysSizInput(1).AirLoopNum = 1;

    state->dataSize->CurSysNum = 1;
    state->dataSize->FinalSysSizing.allocate(state->dataSize->CurSysNum);
    state->dataSize->FinalSysSizing(1).DesMainVolFlow = 1.0;
    state->dataSize->FinalSysSizing(1).DesOutAirVolFlow = 0.4;
    state->dataAirSystemsData->PrimaryAirSystems.allocate(state->dataSize->CurSysNum);
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).Branch.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).Branch(1).Comp.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp(1).Name = "DIRECTEVAPCOOLER";

    // Set Primary Air Data
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).NumBranches = 1;
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).Branch(1).TotalComponents = 1;

    std::string const idf_objects = delimited_string({
        "	EvaporativeCooler:Direct:ResearchSpecial,",
        "	DirectEvapCooler,    !- Name",
        "	,			         !- Availability Schedule Name",
        "	0.7,				 !- Cooler Design Effectiveness",
        "	,					 !- Effectiveness Flow Ratio Modifier Curve Name",
        "	,          			 !- Primary Air Design Flow Rate",
        "	,               	 !- Recirculating Water Pump Power Consumption { W }",
        "	55.0,				 !- Water Pump Power Sizing Factor",
        "	,					 !- Water Pump Power Modifier Curve Name",
        "	Fan Outlet Node,     !- Air Inlet Node Name",
        "	Zone Inlet Node,	 !- Air Outlet Node Name",
        "	Zone Inlet Node,	 !- Sensor Node Name",
        "	,					 !- Water Supply Storage Tank Name",
        "	0.0,				 !- Drift Loss Fraction",
        "	3;                   !- Blowdown Concentration Ratio",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    GetEvapInput(*state);

    auto &EvapCond(state->dataEvapCoolers->EvapCond);

    // check blank autosizable input fields default to autosize
    EXPECT_EQ(DataSizing::AutoSize, EvapCond(EvapCoolNum).DesVolFlowRate);
    EXPECT_EQ(DataSizing::AutoSize, EvapCond(EvapCoolNum).RecircPumpPower);

    // do local sizing calculations
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).Branch(1).Comp(1).Name = EvapCond(EvapCoolNum).Name;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesMainVolFlow = 0.50;
    PrimaryAirDesignFlow = state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesMainVolFlow;
    RecirWaterPumpDesignPower = PrimaryAirDesignFlow * EvapCond(EvapCoolNum).RecircPumpSizingFactor;

    // Test Direct Evaporative Cooler Primary Air Design Flow Rate sizing
    SizeEvapCooler(*state, 1);
    EXPECT_EQ(PrimaryAirDesignFlow, EvapCond(EvapCoolNum).DesVolFlowRate);
    EXPECT_EQ(RecirWaterPumpDesignPower, EvapCond(EvapCoolNum).RecircPumpPower);

    EvapCond.deallocate();
    state->dataAirSystemsData->PrimaryAirSystems.deallocate();
    state->dataSize->FinalSysSizing.deallocate();
}

TEST_F(EnergyPlusFixture, DirectEvapCoolerResearchSpecialCalcTest)
{
    state->init_state(*state);
    auto &EvapCond(state->dataEvapCoolers->EvapCond);

    // one-time setup of evap cooler instance
    int constexpr EvapCoolNum(1);
    EvapCond.allocate(EvapCoolNum);
    state->dataLoopNodes->Node.allocate(2);
    auto &thisEvapCooler = EvapCond(EvapCoolNum);
    state->dataEnvrn->OutBaroPress = 101325.0;

    auto *curve = AddCurve(*state, "Curve1");
    curve->curveType = CurveType::Quadratic;
    curve->coeff[0] = 0.0;
    curve->coeff[1] = 1.0;
    curve->inputLimits[0].min = 0.0;
    curve->inputLimits[0].max = 1.0;

    // set up the flow rates for a direct RDDSpecial
    thisEvapCooler.evapCoolerType = EvapCoolerType::DirectResearchSpecial;
    thisEvapCooler.Name = "MyDirectEvapCoolerRS";
    thisEvapCooler.availSched = Sched::GetScheduleAlwaysOn(*state);
    thisEvapCooler.PumpPowerModifierCurve = curve;
    thisEvapCooler.DirectEffectiveness = 0.75;
    thisEvapCooler.DesVolFlowRate = 1.0;
    thisEvapCooler.InletNode = 1;
    thisEvapCooler.InletTemp = 25.0;
    thisEvapCooler.InletWetBulbTemp = 21.0;
    thisEvapCooler.InletHumRat = PsyWFnTdbTwbPb(*state, thisEvapCooler.InletTemp, thisEvapCooler.InletWetBulbTemp, state->dataEnvrn->OutBaroPress);

    // set full flow rate test condition
    state->dataLoopNodes->Node(thisEvapCooler.InletNode).MassFlowRateMax = 1.0;
    thisEvapCooler.InletMassFlowRate = 1.0;
    thisEvapCooler.RecircPumpPower = 200.0;
    thisEvapCooler.PartLoadFract = 1.0;

    // check water pump power at full primary air flow
    EvaporativeCoolers::CalcDirectResearchSpecialEvapCooler(*state, EvapCoolNum);
    EXPECT_DOUBLE_EQ(200.0, thisEvapCooler.RecircPumpPower);
    EXPECT_DOUBLE_EQ(200.0, thisEvapCooler.EvapCoolerPower);

    // reduce primary air flow rate by half
    thisEvapCooler.InletMassFlowRate = 0.5;
    // check water pump power at half primary air flow
    EvaporativeCoolers::CalcDirectResearchSpecialEvapCooler(*state, EvapCoolNum);
    EXPECT_DOUBLE_EQ(200.0, thisEvapCooler.RecircPumpPower);
    EXPECT_DOUBLE_EQ(100.0, thisEvapCooler.EvapCoolerPower);
}

TEST_F(EnergyPlusFixture, EvaporativeCoolers_IndirectRDDEvapCoolerOperatingMode)
{
    state->init_state(*state);
    auto &EvapCond(state->dataEvapCoolers->EvapCond);

    state->dataEnvrn->OutBaroPress = 101325.0;
    int constexpr EvapCoolNum(1);
    EvapCond.allocate(EvapCoolNum);
    auto &thisEvapCooler = EvapCond(EvapCoolNum);
    // model inputs
    thisEvapCooler.InletMassFlowRate = 1.0;
    thisEvapCooler.SecInletMassFlowRate = 1.0;
    thisEvapCooler.MinOATDBEvapCooler = -99.0;
    thisEvapCooler.MaxOATDBEvapCooler = 99.0;
    thisEvapCooler.MaxOATWBEvapCooler = 99.0;
    thisEvapCooler.WetCoilMaxEfficiency = 0.8;
    thisEvapCooler.InletTemp = 25.5;
    thisEvapCooler.InletHumRat = 0.0140;
    thisEvapCooler.InletWetBulbTemp =
        PsyTwbFnTdbWPb(*state, EvapCond(EvapCoolNum).InletTemp, EvapCond(EvapCoolNum).InletHumRat, state->dataEnvrn->OutBaroPress);
    thisEvapCooler.SecInletTemp = thisEvapCooler.InletTemp;
    thisEvapCooler.SecInletHumRat = thisEvapCooler.InletHumRat;
    thisEvapCooler.SecInletWetBulbTemp = thisEvapCooler.InletWetBulbTemp;
    // set up arguments
    Real64 constexpr TdbOutSysWetMin(22.0);
    Real64 constexpr TdbOutSysDryMin(25.5);
    // set desired outlet teperature below the wet operation
    // minimum temperature to force it to wet full operation
    thisEvapCooler.DesiredOutletTemp = 21.0;

    // determine operating mode
    OperatingMode Result_WetFullOperatingMode = EvaporativeCoolers::IndirectResearchSpecialEvapCoolerOperatingMode(
        *state, EvapCoolNum, thisEvapCooler.SecInletTemp, thisEvapCooler.SecInletWetBulbTemp, TdbOutSysWetMin, TdbOutSysDryMin);
    // check operating mode
    EXPECT_ENUM_EQ(Result_WetFullOperatingMode, EvaporativeCoolers::OperatingMode::WetFull);
    // get outlet temperature in full wet operating mode
    EvaporativeCoolers::CalcIndirectRDDEvapCoolerOutletTemp(*state,
                                                            EvapCoolNum,
                                                            Result_WetFullOperatingMode,
                                                            thisEvapCooler.SecInletMassFlowRate,
                                                            thisEvapCooler.SecInletTemp,
                                                            thisEvapCooler.SecInletWetBulbTemp,
                                                            thisEvapCooler.SecInletHumRat);
    // test outlet temperature in full wet operating mode
    EXPECT_NEAR(22.036, thisEvapCooler.OutletTemp, 0.001);
}

TEST_F(EnergyPlusFixture, DirectEvapCoolerAutosizeWithoutSysSizingRunDone)
{
    state->init_state(*state);

    int constexpr EvapCoolNum(1);
    state->dataSize->NumSysSizInput = 1;
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->SysSizInput(1).AirLoopNum = 1;

    state->dataSize->CurSysNum = 1;
    state->dataAirSystemsData->PrimaryAirSystems.allocate(state->dataSize->CurSysNum);
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).Branch.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).Branch(1).Comp.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp(1).Name = "DIRECTEVAPCOOLER";

    // Set Primary Air Data
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).NumBranches = 1;
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).Branch(1).TotalComponents = 1;

    std::string const idf_objects = delimited_string({
        "	EvaporativeCooler:Direct:ResearchSpecial,",
        "	DirectEvapCooler,    !- Name",
        "	,			         !- Availability Schedule Name",
        "	0.7,				 !- Cooler Design Effectiveness",
        "	,					 !- Effectiveness Flow Ratio Modifier Curve Name",
        "	,          			 !- Primary Air Design Flow Rate",
        "	440,               	 !- Recirculating Water Pump Power Consumption { W }",
        "	1.0,     			 !- Water Pump Power Sizing Factor",
        "	,					 !- Water Pump Power Modifier Curve Name",
        "	Fan Outlet Node,     !- Air Inlet Node Name",
        "	Zone Inlet Node,	 !- Air Outlet Node Name",
        "	Zone Inlet Node,	 !- Sensor Node Name",
        "	,					 !- Water Supply Storage Tank Name",
        "	0.0,				 !- Drift Loss Fraction",
        "	3;                   !- Blowdown Concentration Ratio",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    GetEvapInput(*state);

    auto &EvapCond(state->dataEvapCoolers->EvapCond);

    // check blank autosizable input fields default to autosize
    EXPECT_EQ(DataSizing::AutoSize, EvapCond(EvapCoolNum).DesVolFlowRate);

    // set component name on primary air branch
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).Branch(1).Comp(1).Name = EvapCond(EvapCoolNum).Name;
    state->dataSize->SysSizingRunDone = false;

    // catch Primary Air Design Flow Rate autosize fatal error message
    ASSERT_THROW(EvaporativeCoolers::SizeEvapCooler(*state, 1), std::runtime_error);

    std::string const error_string = delimited_string({
        EnergyPlus::format("   ** Warning ** Version: missing in IDF, processing for EnergyPlus version=\"{}\"", DataStringGlobals::MatchVersion),
        "   ** Severe  ** For autosizing of EvaporativeCooler:Direct:ResearchSpecial DIRECTEVAPCOOLER, a system sizing run must be done.",
        "   **   ~~~   ** The \"SimulationControl\" object did not have the field \"Do System Sizing Calculation\" set to Yes.",
        "   **  Fatal  ** Program terminates due to previously shown condition(s).",
        "   ...Summary of Errors that led to program termination:",
        "   ..... Reference severe error count=1",
        "   ..... Last severe error=For autosizing of EvaporativeCooler:Direct:ResearchSpecial DIRECTEVAPCOOLER, a system sizing run must be done.",
    });

    EXPECT_TRUE(compare_err_stream(error_string, true));
}

TEST_F(EnergyPlusFixture, EvapCoolerAirLoopPumpCycling)
{

    bool ErrorsFound = false;

    std::string const idf_objects = delimited_string({

        " EvaporativeCooler:Direct:CelDekPad,",
        "    Direct CelDekPad EvapCooler, !- Name",
        "    ,                            !- Availability Schedule Name",
        "    0.6,                         !- Direct Pad Area {m2}",
        "    0.17,                        !- Direct Pad Depth {m}",
        "    60,                          !- Recirculating Water Pump Power Consumption {W}",
        "    ZoneEvapCool Fan outlet,     !- Air Inlet Node Name",
        "    ZoneEvapCool Inlet Node,     !- Air Outlet Node Name",
        "    ;                            !- Control Type",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    EvaporativeCoolers::GetEvapInput(*state);
    ASSERT_FALSE(ErrorsFound);

    auto &EvapCond(state->dataEvapCoolers->EvapCond);

    int AirLoopNum = 1;
    int EvapCoolNum = 1;
    state->dataEnvrn->OutBaroPress = 101325.0;

    // Air loop fan PLR
    state->dataAirLoop->AirLoopFlow.allocate(AirLoopNum);
    state->dataAirLoop->AirLoopControlInfo.allocate(AirLoopNum);
    state->dataAirLoop->AirLoopFlow(1).FanPLR = 0.8;

    // Evap cooler conditions
    state->dataLoopNodes->Node(EvapCond(EvapCoolNum).InletNode).MassFlowRate = 0.5;
    state->dataLoopNodes->Node(EvapCond(EvapCoolNum).InletNode).Temp = 28.0;
    state->dataLoopNodes->Node(EvapCond(EvapCoolNum).InletNode).HumRat = 0.001;
    state->dataLoopNodes->Node(EvapCond(EvapCoolNum).InletNode).Press = state->dataEnvrn->OutBaroPress;

    state->dataGlobal->BeginEnvrnFlag = true;

    // Simulate air loop component calls SimEvapCooler
    // SimEvapCooler calls InitEvapCooler(EvapCoolNum) and CalcDirectEvapCooler
    // temporary arguments
    int airLoopNum = 0;
    int branchNum = 0;
    int compNum = 0;
    SimAirServingZones::SimAirLoopComponent(*state,
                                            EvapCond(EvapCoolNum).Name,
                                            SimAirServingZones::CompType::EvapCooler,
                                            false,
                                            AirLoopNum,
                                            EvapCoolNum,
                                            0,
                                            airLoopNum,
                                            branchNum,
                                            compNum);

    // air loop FanPLR successfully passed for pump power calculation
    EXPECT_EQ(EvapCond(EvapCoolNum).EvapCoolerPower, 60 * 0.8);
}

} // namespace EnergyPlus
