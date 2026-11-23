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

// EnergyPlus::SZVAV Model Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/DXCoils.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchNodeConnections.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/FanCoilUnits.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/HeatingCoils.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ReportCoilSelection.hh>
#include <EnergyPlus/SZVAVModel.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UnitarySystem.hh>
#include <EnergyPlus/WaterCoils.hh>
#include <EnergyPlus/ZoneAirLoopEquipmentManager.hh>

using namespace EnergyPlus;
using namespace Curve;
using namespace DataEnvironment;
using namespace DataHeatBalance;
using namespace EnergyPlus::DataSizing;
using namespace EnergyPlus::DataHeatBalance;
using namespace EnergyPlus::DataZoneEquipment;
using namespace EnergyPlus::DataZoneEnergyDemands;
using namespace EnergyPlus::FanCoilUnits;
using namespace EnergyPlus::Fans;
using namespace EnergyPlus::HeatBalanceManager;
using namespace EnergyPlus::OutputReportPredefined;
using namespace EnergyPlus::DataPlant;
using namespace EnergyPlus::Psychrometrics;
using namespace EnergyPlus::SZVAVModel;
using namespace EnergyPlus::UnitarySystems;
using namespace EnergyPlus::WaterCoils;

TEST_F(EnergyPlusFixture, SZVAV_PTUnit_Testing)
{

    std::string const idf_objects = delimited_string({

        "  Schedule:Compact, OnSched, Fraction, Through: 12/31, For: AllDays, Until: 24:00, 1.0; ",
        "  ScheduleTypeLimits, Fraction, 0.0, 1.0, CONTINUOUS; ",
        "  Curve:Quadratic, PLF Curve, 0.85, 0.15, 0, 0, 1, 0.0, 1.0, Dimensionless, Dimensionless; ",
        "  Curve:Cubic, CubicCurve, 1.0, 0.0, 0.0, 0.0, 0.76, 1.09, , , Dimensionless, Dimensionless; ",
        "  Curve:Biquadratic, BiquadraticCurve, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 10, 25.6, 7.2, 48.9, , , Temperature, Temperature, Dimensionless; ",

        " Fan:OnOff,",
        "   Test Fan,                      !- Name",
        "   OnSched,                       !- Availability Schedule Name",
        "   0.7,                           !- Fan Total Efficiency",
        "   13,                            !- Pressure Rise {Pa}",
        "   0.2,                           !- Maximum Flow Rate {m3/s}",
        "   0.9,                           !- Motor Efficiency",
        "   1.0,                           !- Motor In Airstream Fraction",
        "   Fan In Node,                   !- Air Inlet Node Name",
        "   Fan Out Node;                  !- Air Outlet Node Name",

        "Coil:Cooling:DX:SingleSpeed,",
        "	CoolingCoil,                   !- Name",
        " 	OnSched,                       !- Availability Schedule Name",
        "	4000.0,                        !- Gross Rated Total Cooling Capacity { W }",
        "	0.75,                          !- Gross Rated Sensible Heat Ratio",
        "	3.1,                           !- Gross Rated Cooling COP { W / W }",
        "	0.20,                          !- Rated Air Flow Rate { m3 / s }",
        "	,                              !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "	,                              !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "	Fan Out Node,                  !- Air Inlet Node Name",
        "	Cooling Coil Out Node,         !- Air Outlet Node Name",
        "	BiquadraticCurve,              !- Total Cooling Capacity Function of Temperature Curve Name",
        "	CubicCurve,                    !- Total Cooling Capacity Function of Flow Fraction Curve Name",
        "	BiquadraticCurve,              !- Energy Input Ratio Function of Temperature Curve Name",
        "	CubicCurve,                    !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "	PLF Curve;                     !- Part Load Fraction Correlation Curve Name",

        "Coil:Heating:DX:SingleSpeed,",
        "   HeatingCoil,                   !- Name",
        "   OnSched,                       !- Availability Schedule Name",
        "   4000.0,                        !- Gross Rated Heating Capacity {W}",
        "   3.1,                           !- Gross Rated Heating COP {W/W}",
        "   0.20,                          !- Rated Air Flow Rate {m3/s}",
        "   ,                              !- 2017 Rated Supply Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "   ,                              !- 2023 Rated Supply Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "   Cooling Coil Out Node,         !- Air Inlet Node Name",
        "   Heating Coil Out Node,         !- Air Outlet Node Name",
        "   BiquadraticCurve,              !- Heating Capacity Function of Temperature Curve Name",
        "   CubicCurve,                    !- Heating Capacity Function of Flow Fraction Curve Name",
        "   BiquadraticCurve,              !- Energy Input Ratio Function of Temperature Curve Name",
        "   CubicCurve,                    !- Energy Input Ratio Function of Flow Fraction Curve Name",
        "   PLF Curve,                     !- Part Load Fraction Correlation Curve Name",
        "   ,                              !- Defrost Energy Input Ratio Function of Temperature Curve Name",
        "   2.0,                           !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "   ,                              !- Outdoor Dry-Bulb Temperature to Turn On Compressor {C}",
        "   5.0,                           !- Maximum Outdoor Dry-Bulb Temperature for Defrost Operation {C}",
        "   200.0,                         !- Crankcase Heater Capacity {W}",
        "   ,                              !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "   0.0,                           !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "   Resistive,                     !- Defrost Strategy",
        "   TIMED,                         !- Defrost Control",
        "   0.166667,                      !- Defrost Time Period Fraction",
        "   1.0;                           !- Resistive Defrost Heater Capacity {W}",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    state->dataEnvrn->StdRhoAir = 1.0;

    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurSysNum = 0;
    state->dataSize->CurOASysNum = 0;
    state->dataLoopNodes->Node.allocate(10);

    bool errFlag;
    UnitarySystems::UnitarySys thisUnit;
    int FanIndex = Fans::GetFanIndex(*state, "TEST FAN");
    thisUnit.AirInNode = state->dataFans->fans(FanIndex)->inletNodeNum;
    thisUnit.CoolCoilInletNodeNum = DXCoils::GetCoilInletNode(*state, "Coil:Cooling:DX:SingleSpeed", "COOLINGCOIL", errFlag);
    thisUnit.CoolCoilOutletNodeNum = DXCoils::GetCoilOutletNode(*state, "Coil:Cooling:DX:SingleSpeed", "COOLINGCOIL", errFlag);
    thisUnit.HeatCoilInletNodeNum = thisUnit.CoolCoilOutletNodeNum;
    thisUnit.HeatCoilOutletNodeNum = DXCoils::GetCoilOutletNode(*state, "Coil:Heating:DX:SingleSpeed", "HEATINGCOIL", errFlag);
    thisUnit.AirOutNode = thisUnit.HeatCoilOutletNodeNum;
    // set zone condition
    int zoneNodeNum = Node::GetOnlySingleNode(*state,
                                              "ZoneNode",
                                              errFlag,
                                              Node::ConnectionObjectType::ZoneHVACPackagedTerminalAirConditioner,
                                              "PTUnit",
                                              Node::FluidType::Air,
                                              Node::ConnectionType::Inlet,
                                              Node::CompFluidStream::Primary,
                                              Node::ObjectIsNotParent);

    state->dataLoopNodes->Node(thisUnit.AirInNode).Temp = 21.0;
    state->dataLoopNodes->Node(thisUnit.AirInNode).HumRat = 0.00773;
    state->dataLoopNodes->Node(thisUnit.AirInNode).Enthalpy = 40747.4;
    thisUnit.NodeNumOfControlledZone = 5;
    state->dataLoopNodes->Node(zoneNodeNum).Temp = 21.0;
    state->dataLoopNodes->Node(zoneNodeNum).HumRat = 0.08;
    thisUnit.ATMixerExists = false;
    thisUnit.DesignMinOutletTemp = 10.0;
    thisUnit.MaxNoCoolHeatAirMassFlow = 0.1;
    thisUnit.MaxCoolAirMassFlow = 0.2;
    state->dataLoopNodes->Node(thisUnit.AirInNode).MassFlowRateMaxAvail = 0.25;
    thisUnit.LowSpeedCoolFanRatio = 0.5;
    thisUnit.LowSpeedHeatFanRatio = 0.5;
    thisUnit.CoolCoilFluidInletNode = 0;
    thisUnit.CoolCoilFluidOutletNodeNum = 0;
    thisUnit.CoolCoilPlantLoc.loopNum = 0;
    thisUnit.CoolCoilPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Invalid;
    thisUnit.CoolCoilPlantLoc.branchNum = 0;
    thisUnit.CoolCoilPlantLoc.compNum = 0;

    thisUnit.DesignMaxOutletTemp = 30.0;
    thisUnit.MaxHeatAirMassFlow = 0.2;
    thisUnit.HeatCoilFluidInletNode = 0;
    thisUnit.HeatCoilFluidOutletNodeNum = 0;
    thisUnit.HeatCoilPlantLoc.loopNum = 0;
    thisUnit.HeatCoilPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Invalid;
    thisUnit.HeatCoilPlantLoc.branchNum = 0;
    thisUnit.HeatCoilPlantLoc.compNum = 0;

    thisUnit.ControlZoneNum = 1;

    state->dataZoneEquip->ZoneEquipConfig.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode = 1;

    thisUnit.m_SimASHRAEModel = true;
    thisUnit.m_CoolingCoilUpstream = true;
    thisUnit.m_FanExists = true;
    thisUnit.m_CoolCoilExists = true;
    thisUnit.m_HeatCoilExists = true;
    thisUnit.m_MinOATCompressorCooling = -10.0;
    thisUnit.m_sysType = UnitarySystems::UnitarySys::SysType::Unitary;
    thisUnit.m_FanName = "TEST FAN";
    thisUnit.m_FanType = HVAC::FanType::OnOff;
    thisUnit.m_CoolingCoilName = "COOLINGCOIL";
    thisUnit.m_HeatingCoilName = "HEATINGCOIL";
    thisUnit.m_coolCoilType = HVAC::CoilType::CoolingDXSingleSpeed;
    thisUnit.m_heatCoilType = HVAC::CoilType::HeatingDXSingleSpeed;
    thisUnit.m_CoolingCoilIndex = 1;
    thisUnit.m_HeatingCoilIndex = 2;
    thisUnit.m_FanIndex = 1;
    thisUnit.m_sysAvailSched = Sched::GetSchedule(*state, "ONSCHED");
    thisUnit.m_fanAvailSched = Sched::GetSchedule(*state, "ONSCHED");
    thisUnit.m_FanPlace = HVAC::FanPlace::BlowThru;
    // ensure constant fan mode is used
    thisUnit.m_FanOpMode = HVAC::FanOp::Continuous;
    state->dataUnitarySystems->CompOnMassFlow = thisUnit.MaxCoolAirMassFlow;
    state->dataUnitarySystems->CompOffMassFlow = thisUnit.MaxNoCoolHeatAirMassFlow;
    state->dataUnitarySystems->unitarySys.push_back(thisUnit);

    state->dataBranchNodeConnections->NumCompSets = 2;
    state->dataBranchNodeConnections->CompSets.allocate(2);
    state->dataBranchNodeConnections->CompSets(1).ComponentObjectType = Node::ConnectionObjectType::CoilCoolingDXSingleSpeed;
    state->dataBranchNodeConnections->CompSets(1).CName = "CoolingCoil";
    state->dataBranchNodeConnections->CompSets(1).ParentObjectType = Node::ConnectionObjectType::ZoneHVACPackagedTerminalHeatPump;
    state->dataBranchNodeConnections->CompSets(1).ParentCName = "AirSystem";
    state->dataBranchNodeConnections->CompSets(2).ComponentObjectType = Node::ConnectionObjectType::CoilHeatingDXSingleSpeed;
    state->dataBranchNodeConnections->CompSets(2).CName = "HeatingCoil";
    state->dataBranchNodeConnections->CompSets(2).ParentObjectType = Node::ConnectionObjectType::ZoneHVACPackagedTerminalHeatPump;
    state->dataBranchNodeConnections->CompSets(2).ParentCName = "AirSystem";

    state->dataEnvrn->OutDryBulbTemp = 30.0;
    state->dataEnvrn->OutBaroPress = 101325.0;

    int UnitNum = 0;
    bool FirstHVACIteration = true;
    bool CoolingLoad = true;
    bool HeatingLoad = false;
    Real64 QZnReq = -200.0;
    Real64 OnOffAirFlowRatio = 1.0;
    bool HXUnitOn = false;
    int AirLoopNum = 1;
    Real64 PLR = 0.0;
    HVAC::CompressorOp CompressorOnFlag = HVAC::CompressorOp::On;
    state->dataGlobal->BeginEnvrnFlag = true;

    auto &SZVAVModel(state->dataUnitarySystems->unitarySys[UnitNum]);
    // first pass through will get objects and reset node data
    SZVAVModel::calcSZVAVModel(
        *state, SZVAVModel, FirstHVACIteration, CoolingLoad, HeatingLoad, QZnReq, OnOffAirFlowRatio, HXUnitOn, AirLoopNum, PLR, CompressorOnFlag);

    // set unit inlet node conditions for cooling
    state->dataLoopNodes->Node(SZVAVModel.AirInNode).Temp = 24.0;
    state->dataLoopNodes->Node(SZVAVModel.AirInNode).HumRat = 0.011;
    state->dataLoopNodes->Node(SZVAVModel.AirInNode).Enthalpy = 52120.0;
    state->dataLoopNodes->Node(SZVAVModel.AirOutNode).Temp = 21.0;
    state->dataLoopNodes->Node(SZVAVModel.AirOutNode).HumRat = 0.08;

    state->dataLoopNodes->Node(zoneNodeNum).Temp = 24.0;
    state->dataLoopNodes->Node(zoneNodeNum).HumRat = 0.011;
    state->dataLoopNodes->Node(zoneNodeNum).Enthalpy = 52120.0;

    // turn the availability schedule on
    Sched::GetSchedule(*state, "ONSCHED")->currentVal = 1.0;

    state->dataUnitarySystems->CoolingLoad = CoolingLoad;
    state->dataUnitarySystems->HeatingLoad = HeatingLoad;
    // set fan inlet max avail so fan doesn't shut down flow
    state->dataLoopNodes->Node(SZVAVModel.AirInNode).MassFlowRateMaxAvail = 0.2;
    state->dataEnvrn->StdRhoAir = 1.2; // fan used this to convert volume to mass flow rate
    state->dataEnvrn->OutBaroPress = 101325.0;
    // second pass through will run model

    // Region 1 of control, low air flow rate, modulate coil capacity
    SZVAVModel::calcSZVAVModel(
        *state, SZVAVModel, FirstHVACIteration, CoolingLoad, HeatingLoad, QZnReq, OnOffAirFlowRatio, HXUnitOn, AirLoopNum, PLR, CompressorOnFlag);

    EXPECT_NEAR(state->dataLoopNodes->Node(SZVAVModel.AirInNode).MassFlowRate, SZVAVModel.MaxNoCoolHeatAirMassFlow, 0.001); // low speed air flow rate
    EXPECT_LT(state->dataLoopNodes->Node(SZVAVModel.AirOutNode).Temp, state->dataLoopNodes->Node(SZVAVModel.AirInNode).Temp); // active cooling
    Real64 AirMassFlow = state->dataLoopNodes->Node(SZVAVModel.AirOutNode).MassFlowRate;
    Real64 MinHumRat = min(state->dataLoopNodes->Node(SZVAVModel.AirOutNode).HumRat, state->dataLoopNodes->Node(zoneNodeNum).HumRat);
    Real64 OutletTemp = state->dataLoopNodes->Node(SZVAVModel.AirOutNode).Temp;
    Real64 InletTemp = state->dataLoopNodes->Node(SZVAVModel.AirInNode).Temp;
    Real64 LoadMet = AirMassFlow * (Psychrometrics::PsyHFnTdbW(OutletTemp, MinHumRat) - Psychrometrics::PsyHFnTdbW(InletTemp, MinHumRat));
    EXPECT_NEAR(LoadMet, QZnReq, 0.2);
    EXPECT_NEAR(LoadMet, -200.0, 0.2);

    // Region 2 of control, modulate air flow rate, modulate coil capacity
    // set sensible load to mid point of coil sensible capacity (e.g., TotCap * SHR = 4000 * 0.7 = 2800 W) for region 2 of control
    QZnReq = -2000.0;
    SZVAVModel::calcSZVAVModel(
        *state, SZVAVModel, FirstHVACIteration, CoolingLoad, HeatingLoad, QZnReq, OnOffAirFlowRatio, HXUnitOn, AirLoopNum, PLR, CompressorOnFlag);

    EXPECT_GT(state->dataLoopNodes->Node(SZVAVModel.AirInNode).MassFlowRate, SZVAVModel.MaxNoCoolHeatAirMassFlow); // air flow higher than low speed
    EXPECT_LT(state->dataLoopNodes->Node(SZVAVModel.AirInNode).MassFlowRate, SZVAVModel.MaxCoolAirMassFlow);       // air flow lower than high speed
    EXPECT_NEAR(state->dataLoopNodes->Node(SZVAVModel.AirInNode).MassFlowRate, 0.177, 0.001); // air flow between low and high speed, issue 9090
    EXPECT_LT(state->dataLoopNodes->Node(SZVAVModel.AirOutNode).Temp, state->dataLoopNodes->Node(1).Temp); // active cooling

    AirMassFlow = state->dataLoopNodes->Node(SZVAVModel.AirOutNode).MassFlowRate;
    MinHumRat = min(state->dataLoopNodes->Node(SZVAVModel.AirOutNode).HumRat, state->dataLoopNodes->Node(zoneNodeNum).HumRat);
    OutletTemp = state->dataLoopNodes->Node(SZVAVModel.AirOutNode).Temp;
    InletTemp = state->dataLoopNodes->Node(SZVAVModel.AirInNode).Temp;
    LoadMet = AirMassFlow * (Psychrometrics::PsyHFnTdbW(OutletTemp, MinHumRat) - Psychrometrics::PsyHFnTdbW(InletTemp, MinHumRat));
    EXPECT_NEAR(LoadMet, QZnReq, 15.0);
    EXPECT_NEAR(LoadMet, -1989.8, 1.0);
    EXPECT_GT(OutletTemp, SZVAVModel.DesignMinOutletTemp); // coil and air flow are modulating to meet the load

    // Region 3 of control, high air flow rate, modulate coil capacity
    QZnReq = -4000.0; // total capacity of coil, not sensible capacity of coil
    SZVAVModel::calcSZVAVModel(
        *state, SZVAVModel, FirstHVACIteration, CoolingLoad, HeatingLoad, QZnReq, OnOffAirFlowRatio, HXUnitOn, AirLoopNum, PLR, CompressorOnFlag);

    EXPECT_EQ(state->dataLoopNodes->Node(1).MassFlowRate, SZVAVModel.MaxCoolAirMassFlow); // high speed air flow rate
    EXPECT_LT(state->dataLoopNodes->Node(4).Temp, state->dataLoopNodes->Node(1).Temp);    // active cooling

    OutletTemp = state->dataLoopNodes->Node(SZVAVModel.AirOutNode).Temp;
    EXPECT_NEAR(OutletTemp, SZVAVModel.DesignMinOutletTemp, 2.0); // outlet temp driven towards design min outlet temp at high speed and full load
    EXPECT_NEAR(OutletTemp, 11.4, 0.1); // outlet node at 11.4 C, could not get to 10 C at full load and high speed, issue 9090

    MinHumRat = min(state->dataLoopNodes->Node(4).HumRat, state->dataLoopNodes->Node(zoneNodeNum).HumRat);
    OutletTemp = state->dataLoopNodes->Node(4).Temp;
    InletTemp = state->dataLoopNodes->Node(1).Temp;
    LoadMet = SZVAVModel.MaxCoolAirMassFlow * (Psychrometrics::PsyHFnTdbW(OutletTemp, MinHumRat) - Psychrometrics::PsyHFnTdbW(InletTemp, MinHumRat));
    EXPECT_NEAR(QZnReq, -4000.0, 0.001); // load is greater than full load sensible capacity
    EXPECT_NEAR(LoadMet, -2567.0, 1.0);
    EXPECT_NEAR(SZVAVModel.m_SensibleLoadMet, -2580.0, 1.0);   // why different than LoadMet?
    EXPECT_NEAR(SZVAVModel.m_CoolingPartLoadFrac, 1.0, 0.001); // coil at full capacity

    // calculate sensible and latent output based on inlet and outlet conditions and mass flow rate to compare to unit test variables
    Real64 SensibleOutput, LatentOutput, TotalOutput;
    CalcZoneSensibleLatentOutput(SZVAVModel.MaxCoolAirMassFlow,
                                 OutletTemp,
                                 state->dataLoopNodes->Node(4).HumRat,
                                 state->dataLoopNodes->Node(zoneNodeNum).Temp,
                                 state->dataLoopNodes->Node(zoneNodeNum).HumRat,
                                 SensibleOutput,
                                 LatentOutput,
                                 TotalOutput);
    EXPECT_NEAR(SZVAVModel.m_SensibleLoadMet, SensibleOutput, 1.0);
    EXPECT_NEAR(SensibleOutput, -2580.0, 1.0);
    EXPECT_NEAR(TotalOutput, -3996.0, 1.0);

    CoolingLoad = false;
    HeatingLoad = true;
    state->dataUnitarySystems->CoolingLoad = CoolingLoad;
    state->dataUnitarySystems->HeatingLoad = HeatingLoad;

    // set unit inlet node conditions for heating
    state->dataLoopNodes->Node(1).Temp = 21.0;
    state->dataLoopNodes->Node(1).HumRat = 0.008;
    state->dataLoopNodes->Node(1).Enthalpy = 41431.0;
    state->dataLoopNodes->Node(5).Temp = 21.0;
    state->dataLoopNodes->Node(5).HumRat = 0.008;
    state->dataLoopNodes->Node(5).Enthalpy = 41431.0;

    // Region 1 of control, low air flow rate, modulate coil capacity
    QZnReq = 200.0;
    SZVAVModel::calcSZVAVModel(
        *state, SZVAVModel, FirstHVACIteration, CoolingLoad, HeatingLoad, QZnReq, OnOffAirFlowRatio, HXUnitOn, AirLoopNum, PLR, CompressorOnFlag);

    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate, SZVAVModel.MaxNoCoolHeatAirMassFlow, 0.001); // low speed air flow rate
    EXPECT_GT(state->dataLoopNodes->Node(4).Temp, state->dataLoopNodes->Node(1).Temp);                   // active heating

    MinHumRat = min(state->dataLoopNodes->Node(4).HumRat, state->dataLoopNodes->Node(zoneNodeNum).HumRat);
    OutletTemp = state->dataLoopNodes->Node(4).Temp;
    InletTemp = state->dataLoopNodes->Node(1).Temp;
    LoadMet =
        SZVAVModel.MaxNoCoolHeatAirMassFlow * (Psychrometrics::PsyHFnTdbW(OutletTemp, MinHumRat) - Psychrometrics::PsyHFnTdbW(InletTemp, MinHumRat));
    EXPECT_NEAR(LoadMet, QZnReq, 0.001);
    EXPECT_NEAR(LoadMet, 200.0, 0.001);

    // Region 2 of control, modulate air flow rate, modulate coil capacity
    QZnReq = 1200.0;
    SZVAVModel::calcSZVAVModel(
        *state, SZVAVModel, FirstHVACIteration, CoolingLoad, HeatingLoad, QZnReq, OnOffAirFlowRatio, HXUnitOn, AirLoopNum, PLR, CompressorOnFlag);

    EXPECT_GT(state->dataLoopNodes->Node(1).MassFlowRate, SZVAVModel.MaxNoCoolHeatAirMassFlow); // air flow higher than low speed
    EXPECT_LT(state->dataLoopNodes->Node(1).MassFlowRate, SZVAVModel.MaxHeatAirMassFlow);       // air flow lower than high speed
    EXPECT_GT(state->dataLoopNodes->Node(4).Temp, state->dataLoopNodes->Node(1).Temp);          // active heating

    AirMassFlow = state->dataLoopNodes->Node(4).MassFlowRate;
    MinHumRat = min(state->dataLoopNodes->Node(4).HumRat, state->dataLoopNodes->Node(zoneNodeNum).HumRat);
    OutletTemp = state->dataLoopNodes->Node(4).Temp;
    InletTemp = state->dataLoopNodes->Node(1).Temp;
    LoadMet = AirMassFlow * (Psychrometrics::PsyHFnTdbW(OutletTemp, MinHumRat) - Psychrometrics::PsyHFnTdbW(InletTemp, MinHumRat));
    EXPECT_NEAR(LoadMet, QZnReq, 0.001);
    EXPECT_NEAR(LoadMet, 1200.0, 0.001);

    // Region 3 of control, high air flow rate, modulate coil capacity
    QZnReq = 2000.0;
    SZVAVModel::calcSZVAVModel(
        *state, SZVAVModel, FirstHVACIteration, CoolingLoad, HeatingLoad, QZnReq, OnOffAirFlowRatio, HXUnitOn, AirLoopNum, PLR, CompressorOnFlag);

    EXPECT_NEAR(state->dataLoopNodes->Node(1).MassFlowRate, SZVAVModel.MaxHeatAirMassFlow, 0.001); // high speed air flow rate
    EXPECT_GT(state->dataLoopNodes->Node(4).Temp, state->dataLoopNodes->Node(1).Temp);             // active heating

    MinHumRat = min(state->dataLoopNodes->Node(4).HumRat, state->dataLoopNodes->Node(zoneNodeNum).HumRat);
    OutletTemp = state->dataLoopNodes->Node(4).Temp;
    InletTemp = state->dataLoopNodes->Node(1).Temp;
    LoadMet = SZVAVModel.MaxHeatAirMassFlow * (Psychrometrics::PsyHFnTdbW(OutletTemp, MinHumRat) - Psychrometrics::PsyHFnTdbW(InletTemp, MinHumRat));
    EXPECT_NEAR(LoadMet, QZnReq, 0.001);
    EXPECT_NEAR(LoadMet, 2000.0, 0.001);
}

TEST_F(EnergyPlusFixture, SZVAV_FanCoilUnit_Testing)
{

    int FanCoilNum(1);
    int ZoneNum(1);
    HVAC::CompressorOp CompressorOnFlag(HVAC::CompressorOp::Off);
    int AirLoopNum(0);
    bool FirstHVACIteration(false);
    bool ErrorsFound(false);
    bool CoolingLoad(false);
    bool HeatingLoad(false);
    bool HXUnitOn(false);
    Real64 QZnReq(0.0);
    Real64 ColdWaterMassFlowRate(0.0);
    Real64 QUnitOut(0.0);
    Real64 QLatOut(0.0);
    Real64 AirMassFlow(0.0);
    Real64 MaxAirMassFlow(0.0);
    Real64 OnOffAirFlowRatio(1.0);
    Real64 PLR(0.0);

    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataWaterCoils->GetWaterCoilsInputFlag = true;
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->dataSize->CurZoneEqNum = 1;

    std::string const idf_objects = delimited_string({

        " Zone,",
        "  WEST ZONE,                 !- Name",
        "  0,                         !- Direction of Relative North { deg }",
        "  0,                         !- X Origin { m }",
        "  0,                         !- Y Origin { m }",
        "  0,                         !- Z Origin { m }",
        "  1,                         !- Type",
        "  1,                         !- Multiplier",
        "  autocalculate,             !- Ceiling Height { m }",
        "  autocalculate;             !- Volume { m3 }",

        " ZoneHVAC:EquipmentConnections,",
        "  WEST ZONE,                 !- Zone Name",
        "  ZoneEquipment,             !- Zone Conditioning Equipment List Name",
        "  FanCoilAirOutletNode,      !- Zone Air Inlet Node or NodeList Name",
        "  FanCoilAirInletNode,       !- Zone Air Exhaust Node or NodeList Name",
        "  Zone Air Node,             !- Zone Air Node Name",
        "  Zone Air Outlet Node;      !- Zone Return Air Node Name",

        " ZoneHVAC:EquipmentList,",
        "  ZoneEquipment,             !- Name",
        "  SequentialLoad,            !- Load Distribution Scheme",
        "  ZoneHVAC:FourPipeFanCoil,  !- Zone Equipment 1 Object Type",
        "  ZoneFanCoil,               !- Zone Equipment 1 Name",
        "  1,                         !- Zone Equipment 1 Cooling Sequence",
        "  1;                         !- Zone Equipment 1 Heating or No - Load Sequence",

        " ZoneHVAC:FourPipeFanCoil,",
        "  ZoneFanCoil,               !- Name",
        "  FanAndCoilAvailSched,      !- Availability Schedule Name",
        "  ASHRAE90VariableFan,       !- Capacity Control Method",
        "  0.5,                       !- Maximum Supply Air Flow Rate { m3 / s }",
        "  0.3,                       !- Low Speed Supply Air Flow Ratio",
        "  0.6,                       !- Medium Speed Supply Air Flow Ratio",
        "  0.0,                       !- Maximum Outdoor Air Flow Rate { m3 / s }",
        "  FanAndCoilAvailSched,      !- Outdoor Air Schedule Name",
        "  FanCoilAirInletNode,       !- Air Inlet Node Name",
        "  FanCoilAirOutletNode,      !- Air Outlet Node Name",
        "  OutdoorAir:Mixer,          !- Outdoor Air Mixer Object Type",
        "  FanCoilOAMixer,            !- Outdoor Air Mixer Name",
        "  Fan:OnOff,                 !- Supply Air Fan Object Type",
        "  FanCoilFan,                !- Supply Air Fan Name",
        "  Coil:Cooling:Water,        !- Cooling Coil Object Type",
        "  FanCoilCoolingCoil,        !- Cooling Coil Name",
        "  0.00014,                   !- Maximum Cold Water Flow Rate { m3 / s }",
        "  0.0,                       !- Minimum Cold Water Flow Rate { m3 / s }",
        "  0.001,                     !- Cooling Convergence Tolerance",
        "  Coil:Heating:Electric,     !- Heating Coil Object Type",
        "  FanCoilElecHeatingCoil,    !- Heating Coil Name",
        "  0.0,                       !- Maximum Hot Water Flow Rate { m3 / s }",
        "  0.0,                       !- Minimum Hot Water Flow Rate { m3 / s }",
        "  0.001,                     !- Heating Convergence Tolerance",
        "  ,                          !- Availability Manager List Name",
        "  ;                          !- Design Specification ZoneHVAC Sizing Object Name",

        " OutdoorAir:NodeList,",
        "  FanCoilOAInNode;           !- Node or NodeList Name 1",

        " OutdoorAir:Mixer,",
        "  FanCoilOAMixer,            !- Name",
        "  FanCoilOAMixerOutletNode,  !- Mixed Air Node Name",
        "  FanCoilOAInNode,           !- Outdoor Air Stream Node Name",
        "  FanCoilExhNode,            !- Relief Air Stream Node Name",
        "  FanCoilAirInletNode;       !- Return Air Stream Node Name",

        " Fan:OnOff,",
        "  FanCoilFan,                !- Name",
        "  FanAndCoilAvailSched,      !- Availability Schedule Name",
        "  0.5,                       !- Fan Total Efficiency",
        "  75,                        !- Design Pressure Rise {Pa}",
        "  0.5,                       !- Maximum Air Flow Rate {m3/s}",
        "  0.9,                       !- Motor Efficiency",
        "  1,                         !- Motor In Air Stream Fraction",
        "  FanCoilOAMixerOutletNode,  !- Air Inlet Node Name",
        "  FanCoilFanOutletNode;      !- Air Outlet Node Name",

        " Coil:Cooling:Water,",
        "  FanCoilCoolingCoil,        !- Name",
        "  FanAndCoilAvailSched,      !- Availability Schedule Namev",
        "  0.0002,                    !- Design Water Flow Rate { m3 / s }",
        "  0.5000,                    !- Design Air Flow Rate { m3 / s }",
        "  7.22,                      !- Design Inlet Water Temperature { Cv }",
        "  24.340,                    !- Design Inlet Air Temperature { C }",
        "  14.000,                    !- Design Outlet Air Temperature { C }",
        "  0.0095,                    !- Design Inlet Air Humidity Ratio { kgWater / kgDryAir }",
        "  0.0090,                    !- Design Outlet Air Humidity Ratio { kgWater / kgDryAir }",
        "  FanCoilChWInletNode,       !- Water Inlet Node Name",
        "  FanCoilChWOutletNode,      !- Water Outlet Node Name",
        "  FanCoilFanOutletNode,      !- Air Inlet Node Name",
        "  FanCoilCCOutletNode,       !- Air Outlet Node Name",
        "  SimpleAnalysis,            !- Type of Analysis",
        "  CrossFlow;                 !- Heat Exchanger Configuration",

        " Coil:Heating:Electric,",
        "  FanCoilElecHeatingCoil,    !- Name",
        "  FanAndCoilAvailSched,      !- Availability Schedule Name",
        "  1,                         !- Efficiency",
        "  10000.0,                    !- Nominal Capacity {W}",
        "  FanCoilCCOutletNode,       !- Air Inlet Node Name",
        "  FanCoilAirOutletNode;      !- Air Outlet Node Name",

        " Schedule:Constant,",
        "  FanAndCoilAvailSched,      !- Name",
        "  FRACTION,                  !- Schedule Type",
        "  1;                         !- TimeStep Value",

        " ScheduleTypeLimits,",
        "  Fraction,                  !- Name",
        "  0.0,                       !- Lower Limit Value",
        "  1.0,                       !- Upper Limit Value",
        "  CONTINUOUS;                !- Numeric Type",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    state->dataEnvrn->StdRhoAir = 1.0;

    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataWaterCoils->GetWaterCoilsInputFlag = true;
    state->dataGlobal->TimeStep = 1;
    state->dataSize->CurZoneEqNum = 1;
    GetZoneData(*state, ErrorsFound);
    EXPECT_EQ("WEST ZONE", state->dataHeatBal->Zone(1).Name);
    GetZoneEquipmentData(*state);
    GetFanCoilUnits(*state);
    auto &thisFanCoil(state->dataFanCoilUnits->FanCoil(1));
    EXPECT_ENUM_EQ(CCM::ASHRAE, thisFanCoil.CapCtrlMeth_Num);
    EXPECT_EQ("OUTDOORAIR:MIXER", thisFanCoil.OAMixType);
    EXPECT_EQ((int)HVAC::FanType::OnOff, (int)thisFanCoil.fanType);
    EXPECT_EQ("COIL:COOLING:WATER", thisFanCoil.CCoilType);
    EXPECT_EQ("COIL:HEATING:ELECTRIC", thisFanCoil.HCoilType);
    state->dataPlnt->TotNumLoops = 1;
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    AirMassFlow = 0.60;
    MaxAirMassFlow = 0.60;
    ColdWaterMassFlowRate = 1.0;
    thisFanCoil.OutAirMassFlow = 0.0;
    thisFanCoil.MaxAirMassFlow = MaxAirMassFlow;
    // outside air mixer
    auto &MixerOA(state->dataMixedAir->OAMixer(1));
    state->dataLoopNodes->Node(MixerOA.RetNode).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(MixerOA.RetNode).MassFlowRateMax = MaxAirMassFlow;
    state->dataLoopNodes->Node(MixerOA.RetNode).Temp = 20.0;
    state->dataLoopNodes->Node(MixerOA.RetNode).Enthalpy = 36000;
    state->dataLoopNodes->Node(MixerOA.RetNode).HumRat =
        PsyWFnTdbH(*state, state->dataLoopNodes->Node(MixerOA.RetNode).Temp, state->dataLoopNodes->Node(MixerOA.RetNode).Enthalpy);
    state->dataLoopNodes->Node(MixerOA.InletNode).Temp = 10.0;
    state->dataLoopNodes->Node(MixerOA.InletNode).Enthalpy = 18000;
    state->dataLoopNodes->Node(MixerOA.InletNode).HumRat =
        PsyWFnTdbH(*state, state->dataLoopNodes->Node(MixerOA.InletNode).Temp, state->dataLoopNodes->Node(MixerOA.InletNode).Enthalpy);
    // chilled water coil
    auto &CWCoil(state->dataWaterCoils->WaterCoil(1));
    CWCoil.UACoilTotal = 470.0;
    CWCoil.UACoilExternal = 611.0;
    CWCoil.UACoilInternal = 2010.0;
    CWCoil.TotCoilOutsideSurfArea = 50.0;
    state->dataLoopNodes->Node(CWCoil.AirInletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(CWCoil.AirInletNodeNum).MassFlowRateMax = AirMassFlow;
    state->dataLoopNodes->Node(CWCoil.AirInletNodeNum).MassFlowRateMaxAvail = AirMassFlow;
    CWCoil.InletWaterMassFlowRate = ColdWaterMassFlowRate;
    CWCoil.MaxWaterMassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(CWCoil.WaterInletNodeNum).MassFlowRate = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(CWCoil.WaterInletNodeNum).MassFlowRateMaxAvail = ColdWaterMassFlowRate;
    state->dataLoopNodes->Node(CWCoil.WaterInletNodeNum).Temp = 6.0;
    CWCoil.WaterPlantLoc.loopNum = 1;
    CWCoil.WaterPlantLoc.loopSideNum = DataPlant::LoopSideLocation::Demand;
    CWCoil.WaterPlantLoc.branchNum = 1;
    CWCoil.WaterPlantLoc.compNum = 1;
    // electric heating coil
    auto &eHCoil(state->dataHeatingCoils->HeatingCoil(1));
    state->dataLoopNodes->Node(eHCoil.AirInletNodeNum).MassFlowRate = AirMassFlow;
    state->dataLoopNodes->Node(eHCoil.AirInletNodeNum).MassFlowRateMaxAvail = AirMassFlow;

    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {

        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }
    // chilled water plant loop
    auto &CWLoop(state->dataPlnt->PlantLoop(1));
    CWLoop.Name = "ChilledWaterLoop";
    CWLoop.FluidName = "WATER";
    CWLoop.glycol = Fluid::GetWater(*state);
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = CWCoil.Name;
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type = DataPlant::PlantEquipmentType::CoilWaterCooling;
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = CWCoil.WaterInletNodeNum;
    CWLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = CWCoil.WaterOutletNodeNum;

    state->dataWaterCoils->MyUAAndFlowCalcFlag.allocate(1);
    state->dataWaterCoils->MyUAAndFlowCalcFlag(1) = true;
    state->dataGlobal->DoingSizing = true;
    state->dataHVACGlobal->TurnFansOff = false;
    state->dataHVACGlobal->TurnFansOn = true;

    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    auto &zSysEDemand(state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1));

    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 21;
    state->dataGlobal->HourOfDay = 1;
    state->dataEnvrn->DSTIndicator = 0;
    state->dataEnvrn->DayOfWeek = 2;
    state->dataEnvrn->HolidayIndex = 0;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    Sched::UpdateScheduleVals(*state);
    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataSize->ZoneSizingRunDone = true;
    thisFanCoil.DesignHeatingCapacity = 10000.0;
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataGlobal->SysSizingCalc = true;
    FirstHVACIteration = true;
    zSysEDemand.RemainingOutputReqToCoolSP = 0.0;
    zSysEDemand.RemainingOutputReqToHeatSP = 0.0;
    zSysEDemand.RemainingOutputRequired = 0.0;
    QZnReq = 0.0;
    QUnitOut = 0.0;
    QLatOut = 0.0;
    // init
    InitFanCoilUnits(*state, FanCoilNum, ZoneNum);
    Sim4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, QLatOut);
    // heating mode tests
    CoolingLoad = false;
    HeatingLoad = true;
    OnOffAirFlowRatio = 1.0;
    HXUnitOn = false;
    AirLoopNum = 0;
    CompressorOnFlag = HVAC::CompressorOp::Off;
    FirstHVACIteration = true;
    auto &SZVAVModel(state->dataFanCoilUnits->FanCoil(FanCoilNum));

    // test 1: 1000 W heating load
    zSysEDemand.RemainingOutputReqToCoolSP = 1000.0;
    zSysEDemand.RemainingOutputReqToHeatSP = 1000.0;
    zSysEDemand.RemainingOutputRequired = 1000.0;
    QZnReq = 1000.0;
    QUnitOut = 0.0;
    QLatOut = 0.0;
    PLR = 0.0;
    SZVAVModel::calcSZVAVModel(*state,
                               SZVAVModel,
                               FanCoilNum,
                               FirstHVACIteration,
                               CoolingLoad,
                               HeatingLoad,
                               QZnReq,
                               OnOffAirFlowRatio,
                               HXUnitOn,
                               AirLoopNum,
                               PLR,
                               CompressorOnFlag);
    Calc4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, PLR);
    EXPECT_NEAR(PLR, 0.092, 0.001);
    EXPECT_NEAR(QUnitOut, 1000.0, 1.0);

    // test 2: 5000 W heating load
    zSysEDemand.RemainingOutputReqToCoolSP = 5000.0;
    zSysEDemand.RemainingOutputReqToHeatSP = 5000.0;
    zSysEDemand.RemainingOutputRequired = 5000.0;
    QZnReq = 5000.0;
    QUnitOut = 0.0;
    QLatOut = 0.0;
    PLR = 0.0;
    SZVAVModel::calcSZVAVModel(*state,
                               SZVAVModel,
                               FanCoilNum,
                               FirstHVACIteration,
                               CoolingLoad,
                               HeatingLoad,
                               QZnReq,
                               OnOffAirFlowRatio,
                               HXUnitOn,
                               AirLoopNum,
                               PLR,
                               CompressorOnFlag);
    Calc4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, PLR);
    EXPECT_NEAR(PLR, 0.492, 0.001);
    EXPECT_NEAR(QUnitOut, 5000.0, 1.0);

    // test 3: 9000 W heating load
    zSysEDemand.RemainingOutputReqToCoolSP = 9000.0;
    zSysEDemand.RemainingOutputReqToHeatSP = 9000.0;
    zSysEDemand.RemainingOutputRequired = 9000.0;
    QZnReq = 9000.0;
    QUnitOut = 0.0;
    QLatOut = 0.0;
    PLR = 0.0;
    SZVAVModel::calcSZVAVModel(*state,
                               SZVAVModel,
                               FanCoilNum,
                               FirstHVACIteration,
                               CoolingLoad,
                               HeatingLoad,
                               QZnReq,
                               OnOffAirFlowRatio,
                               HXUnitOn,
                               AirLoopNum,
                               PLR,
                               CompressorOnFlag);
    Calc4PipeFanCoil(*state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, PLR);
    EXPECT_NEAR(PLR, 0.892, 0.001);
    EXPECT_NEAR(QUnitOut, 9000.0, 1.0);
}

TEST_F(EnergyPlusFixture, SZVAV_UnitarySys_VarSpeed_Testing)
{
    std::string const idf_objects = delimited_string({

        "  Coil:Heating:DX:VariableSpeed,",
        "    Sys 2 Furnace DX Cool Heating Coil,              !- Name",
        "    ,                                                !- Availability Schedule Name",
        "    Sys 2 Furnace DX Cool Cooling Coil Outlet,       !- Indoor Air Inlet Node Name",
        "    Sys 2 Furnace DX Cool Heating Coil Outlet,       !- Indoor Air Outlet Node Name",
        "    3,                                               !- Number of Speeds {dimensionless}",
        "    3,                                               !- Nominal Speed Level {dimensionless}",
        "    4000.00000,                                      !- Rated Heating Capacity At Selected Nominal Speed Level {W}",
        "    autosize,                                        !- Rated Air Flow Rate At Selected Nominal Speed Level {m3/s}",
        "    Sys 3 Heat Pump Air Source Heat Coil PLF,        !- Energy Part Load Fraction Curve Name",
        "    Sys 3 Heat Pump Air Source Heat Coil Defr-FT,    !- Defrost Energy Input Ratio Function of Temperature Curve Name",
        "    -8.0,                                            !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                                                !- Outdoor Dry-Bulb Temperature to Turn On Compressor {C}",
        "    5.0,                                             !- Maximum Outdoor Dry-Bulb Temperature for Defrost Operation {C}",
        "    ,                                                !- Crankcase Heater Capacity {W}",
        "    ,                                                !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10.0,                                            !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ReverseCycle,                                    !- Defrost Strategy",
        "    Timed,                                           !- Defrost Control",
        "    0.058333,                                        !- Defrost Time Period Fraction",
        "    0.0,                                             !- Resistive Defrost Heater Capacity {W}",
        "    1333,                                            !- Speed 1 Reference Unit Gross Rated Heating Capacity {W}",
        "    3.0,                                             !- Speed 1 Reference Unit Gross Rated Heating COP {W/W}",
        "    0.10203099,                                      !- Speed 1 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                                           !- 2017 Speed 1 Rated Supply Air Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    934.4,                                           !- 2023 Speed 1 Rated Supply Air Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    HPACHeatCapFT,                                   !- Speed 1 Heating Capacity Function of Temperature Curve Name",
        "    HPACHeatCapFFF,                                  !- Speed 1 Total  Heating Capacity Function of Air Flow Fraction Curve Name",
        "    HPACHeatEIRFT,                                   !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACHeatEIRFFF,                                  !- Speed 1 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    2666,                                            !- Speed 2 Reference Unit Gross Rated Heating Capacity {W}",
        "    3.0,                                             !- Speed 2 Reference Unit Gross Rated Heating COP {W/W}",
        "    0.127500495,                                     !- Speed 2 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                                           !- 2017 Speed 2 Rated Supply Air Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    934.4,                                           !- 2023 Speed 2 Rated Supply Air Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    HPACHeatCapFT,                                   !- Speed 2 Heating Capacity Function of Temperature Curve Name",
        "    HPACHeatCapFFF,                                  !- Speed 2 Total  Heating Capacity Function of Air Flow Fraction Curve Name",
        "    HPACHeatEIRFT,                                   !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACHeatEIRFFF,                                  !- Speed 2 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    4000.00000,                                      !- Speed 3 Reference Unit Gross Rated Heating Capacity {W}",
        "    3.0,                                             !- Speed 3 Reference Unit Gross Rated Heating COP {W/W}",
        "    0.15297,                                         !- Speed 3 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                                           !- 2017 Speed 3 Rated Supply Air Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    934.4,                                           !- 2023 Speed 3 Rated Supply Air Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    HPACHeatCapFT,                                   !- Speed 3 Heating Capacity Function of Temperature Curve Name",
        "    HPACHeatCapFFF,                                  !- Speed 3 Total  Heating Capacity Function of Air Flow Fraction Curve Name",
        "    HPACHeatEIRFT,                                   !- Speed 3 Energy Input Ratio Function of Temperature Curve Name",
        "    HPACHeatEIRFFF;                                  !- Speed 3 Energy Input Ratio Function of Air Flow Fraction Curve Name",

        "Curve:Biquadratic,",
        "  Sys 3 Heat Pump Air Source Heat Coil Defr-FT,  !- Name",
        "  1.0,                     !- Coefficient1 Constant",
        "  0.0,                     !- Coefficient2 x",
        "  0.0,                     !- Coefficient3 x**2",
        "  0.0,                     !- Coefficient4 y",
        "  0.0,                     !- Coefficient5 y**2",
        "  0.0,                     !- Coefficient6 x*y",
        "  0,                       !- Minimum Value of x",
        "  50,                      !- Maximum Value of x",
        "  0,                       !- Minimum Value of y",
        "  50,                      !- Maximum Value of y",
        "  ,                        !- Minimum Curve Output",
        "  ,                        !- Maximum Curve Output",
        "  Temperature,             !- Input Unit Type for X",
        "  Temperature,             !- Input Unit Type for Y",
        "  Dimensionless;           !- Output Unit Type",

        "Curve:Quadratic,",
        "  Sys 3 Heat Pump Air Source Heat Coil PLF,  !- Name",
        "  0.85,                    !- Coefficient1 Constant",
        "  0.15,                    !- Coefficient2 x",
        "  0,                       !- Coefficient3 x**2",
        "  0,                       !- Minimum Value of x",
        "  1;                       !- Maximum Value of x",

        "  Curve:Biquadratic,",
        "    HPACHeatCapFT,           !- Name",
        "    0.8529681407,            !- Coefficient1 Constant",
        "    -0.0004847169,           !- Coefficient2 x",
        "    -0.0000010693,           !- Coefficient3 x**2",
        "    0.0185542164,            !- Coefficient4 y",
        "    0.0000872425,            !- Coefficient5 y**2",
        "    -0.0000166868,           !- Coefficient6 x*y",
        "    17.78,                   !- Minimum Value of x",
        "    23.33,                   !- Maximum Value of x",
        "    -28.89,                  !- Minimum Value of y",
        "    17.22,                   !- Maximum Value of y",
        "    0.3799,                  !- Minimum Curve Output",
        "    1.1896,                  !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Biquadratic,",
        "    HPACHeatEIRFT,           !- Name",
        "    0.7077081462,            !- Coefficient1 Constant",
        "    0.0148163478,            !- Coefficient2 x",
        "    0.0002622589,            !- Coefficient3 x**2",
        "    -0.0113239622,           !- Coefficient4 y",
        "    0.0002939277,            !- Coefficient5 y**2",
        "    -0.0003605284,           !- Coefficient6 x*y",
        "    17.78,                   !- Minimum Value of x",
        "    23.33,                   !- Maximum Value of x",
        "    -28.89,                  !- Minimum Value of y",
        "    17.22,                   !- Maximum Value of y",
        "    0.8266,                  !- Minimum Curve Output",
        "    2.0277,                  !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "  Curve:Cubic,",
        "    HPACHeatCapFFF,          !- Name",
        "    0.84,                    !- Coefficient1 Constant",
        "    0.16,                    !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Coefficient4 x**3",
        "    0.5,                     !- Minimum Value of x",
        "    1.5;                     !- Maximum Value of x",

        "  Curve:Quadratic,",
        "    HPACHeatEIRFFF,          !- Name",
        "    1.3824,                  !- Coefficient1 Constant",
        "    -0.4336,                 !- Coefficient2 x",
        "    0.0512,                  !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",

        "  Coil:Cooling:DX:VariableSpeed,",
        "    Sys 2 Furnace DX Cool Cooling Coil,                       !- Name",
        "    ,                                                         !- Availability Schedule Name",
        "    Sys 2 Furnace DX Cool Mixed Air Outlet,                   !- Indoor Air Inlet Node Name",
        "    Sys 2 Furnace DX Cool Cooling Coil Outlet,                !- Indoor Air Outlet Node Name",
        "    3,                                                        !- Number of Speeds {dimensionless}",
        "    3,                                                        !- Nominal Speed Level {dimensionless}",
        "    3513.72009,                                               !- Gross Rated Total Cooling Capacity At Selected Nominal Speed Level {W}",
        "    0.15297,                                                  !- Rated Air Flow Rate At Selected Nominal Speed Level {m3/s}",
        "    0,                                                        !- Nominal Time for Condensate to Begin Leaving the Coil {s}",
        "    0,                                                        !- Initial Moisture Evaporation Rate Divided by Steady-State AC Latent "
        "Capacity {dimensionless}",
        "    2.5,                                                      !- Maximum Cycling Rate {cycles/hr}",
        "    60,                                                       !- Latent Capacity Time Constant {s}",
        "    60,                                                       !- Fan Delay Time {s}",
        "    Sys 3 Heat Pump Air Source Heat Coil PLF,                 !- Energy Part Load Fraction Curve Name",
        "    ,                                                         !- Condenser Air Inlet Node Name",
        "    AirCooled,                                                !- Condenser Type",
        "    0.0,                                                      !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "    0.0,                                                      !- Crankcase Heater Capacity {W}",
        "    ,                                                         !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10.0,                                                     !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    -25.0,                                                    !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                                                         !- Supply Water Storage Tank Name",
        "    ,                                                         !- Condensate Collection Water Storage Tank Name",
        "    0.0,                                                      !- Basin Heater Capacity {W/K}",
        "    2.0,                                                      !- Basin Heater Setpoint Temperature {C}",
        "    ,                                                         !- Basin Heater Operating Schedule Name",
        "    1171.24003,                                               !- Speed 1 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.72595,                                                  !- Speed 1 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3,                                                        !- Speed 1 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.10203099,                                               !- Speed 1 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                                                    !- 2017 Speed 1 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    934.4,                                                    !- 2023 Speed 1 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.10203099,                                               !- Speed 1 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                                                         !- Speed 1 Reference Unit Rated Pad Effectiveness of Evap Precooling "
        "{dimensionless}",
        "    Sys 2 Furnace DX Cool Cool Coil Cap-FT,                   !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil Cap-FF,                   !- Speed 1 Total Cooling Capacity Function of Air Flow Fraction Curve "
        "Name",
        "    Sys 2 Furnace DX Cool Cool Coil EIR-FT,                   !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil EIR-FF,                   !- Speed 1 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    2342.48006,                                               !- Speed 2 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.72595,                                                  !- Speed 2 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3,                                                        !- Speed 2 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.127500495,                                              !- Speed 2 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                                                    !- 2017 Speed 2 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    934.4,                                                    !- 2023 Speed 2 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.127500495,                                              !- Speed 2 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                                                         !- Speed 2 Reference Unit Rated Pad Effectiveness of Evap Precooling "
        "{dimensionless}",
        "    Sys 2 Furnace DX Cool Cool Coil Cap-FT,                   !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil Cap-FF,                   !- Speed 2 Total Cooling Capacity Function of Air Flow Fraction Curve "
        "Name",
        "    Sys 2 Furnace DX Cool Cool Coil EIR-FT,                   !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil EIR-FF,                   !- Speed 2 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    3513.72009,                                               !- Speed 2 Reference Unit Gross Rated Total Cooling Capacity {W}",
        "    0.72595,                                                  !- Speed 2 Reference Unit Gross Rated Sensible Heat Ratio {dimensionless}",
        "    3,                                                        !- Speed 2 Reference Unit Gross Rated Cooling COP {W/W}",
        "    0.15297,                                                  !- Speed 2 Reference Unit Rated Air Flow Rate {m3/s}",
        "    773.3,                                                    !- 2017 Speed 2 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    934.4,                                                    !- 2023 Speed 2 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    0.15297,                                                  !- Speed 2 Reference Unit Rated Condenser Air Flow Rate {m3/s}",
        "    ,                                                         !- Speed 2 Reference Unit Rated Pad Effectiveness of Evap Precooling "
        "{dimensionless}",
        "    Sys 2 Furnace DX Cool Cool Coil Cap-FT,                   !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil Cap-FF,                   !- Speed 2 Total Cooling Capacity Function of Air Flow Fraction Curve "
        "Name",
        "    Sys 2 Furnace DX Cool Cool Coil EIR-FT,                   !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    Sys 2 Furnace DX Cool Cool Coil EIR-FF;                   !- Speed 2 Energy Input Ratio Function of Air Flow Fraction Curve Name",

        "Curve:Biquadratic,",
        "  Sys 2 Furnace DX Cool Cool Coil Cap-FT,  !- Name",
        "  0.476428E+00,            !- Coefficient1 Constant",
        "  0.401147E-01,            !- Coefficient2 x",
        "  0.226411E-03,            !- Coefficient3 x**2",
        "  -0.827136E-03,           !- Coefficient4 y",
        "  -0.732240E-05,           !- Coefficient5 y**2",
        "  -0.446278E-03,           !- Coefficient6 x*y",
        "  0.0,                     !- Minimum Value of x",
        "  50.0,                    !- Maximum Value of x",
        "  0.0,                     !- Minimum Value of y",
        "  50.0,                    !- Maximum Value of y",
        "  0.0,                     !- Minimum Curve Output",
        "  5.0,                     !- Maximum Curve Output",
        "  Temperature,             !- Input Unit Type for X",
        "  Temperature,             !- Input Unit Type for Y",
        "  Dimensionless;           !- Output Unit Type",

        "Curve:Cubic,",
        "  Sys 2 Furnace DX Cool Cool Coil Cap-FF,  !- Name",
        "  1.0,!0.47278589,              !- Coefficient1 Constant",
        "  0.0,!1.2433415,               !- Coefficient2 x",
        "  0.0,!-1.0387055,              !- Coefficient3 x**2",
        "  0.0,!0.32257813,              !- Coefficient4 x**3",
        "  0.5,                     !- Minimum Value of x",
        "  1.5;                     !- Maximum Value of x",

        "Curve:Biquadratic,",
        "  Sys 2 Furnace DX Cool Cool Coil EIR-FT,  !- Name",
        "  0.632475E+00,            !- Coefficient1 Constant",
        "  -0.121321E-01,           !- Coefficient2 x",
        "  0.507773E-03,            !- Coefficient3 x**2",
        "  0.155377E-01,            !- Coefficient4 y",
        "  0.272840E-03,            !- Coefficient5 y**2",
        "  -0.679201E-03,           !- Coefficient6 x*y",
        "  0.0,                     !- Minimum Value of x",
        "  50.0,                    !- Maximum Value of x",
        "  0.0,                     !- Minimum Value of y",
        "  50.0,                    !- Maximum Value of y",
        "  0.0,                     !- Minimum Curve Output",
        "  5.0,                     !- Maximum Curve Output",
        "  Temperature,             !- Input Unit Type for X",
        "  Temperature,             !- Input Unit Type for Y",
        "  Dimensionless;           !- Output Unit Type",

        "Curve:Cubic,",
        "  Sys 2 Furnace DX Cool Cool Coil EIR-FF,  !- Name",
        "  0.47278589,              !- Coefficient1 Constant",
        "  1.2433415,               !- Coefficient2 x",
        "  -1.0387055,              !- Coefficient3 x**2",
        "  0.32257813,              !- Coefficient4 x**3",
        "  0.5,                     !- Minimum Value of x",
        "  1.5;                     !- Maximum Value of x",

        "  Fan:SystemModel,",
        "    Sys 2 Furnace DX Cool Supply Fan,           !- Name",
        "    ,                                           !- Availability Schedule Name",
        "    Sys 2 Furnace DX Cool Heating Coil Outlet,  !- Air Inlet Node Name",
        "    Sys 2 Furnace DX Cool Supply Fan Outlet,    !- Air Outlet Node Name",
        "    0.15297,                                    !- Design Maximum Air Flow Rate {m3/s}",
        "    Continuous,                                 !- Speed Control Method",
        "    0,                                          !- Electric Power Minimum Flow Rate Fraction",
        "    697.449079989937,                           !- Design Pressure Rise {Pa}",
        "    0.9,                                        !- Motor Efficiency",
        "    1,                                          !- Motor In Air Stream Fraction",
        "    autosize,                                   !- Design Electric Power Consumption {W}",
        "    PowerPerFlowPerPressure,                    !- Design Power Sizing Method",
        "    ,                                           !- Electric Power Per Unit Flow Rate {W/(m3/s)}",
        "    1.66667,                                    !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}",
        "    0.7,                                        !- Fan Total Efficiency",
        "    fan_curve,                                  !- Electric Power Function of Flow Fraction Curve Name",
        "    ,                                           !- Night Ventilation Mode Pressure Rise {Pa}",
        "    ,                                           !- Night Ventilation Mode Flow Fraction",
        "    ,                                           !- Motor Loss Zone Name",
        "    ,                                           !- Motor Loss Radiative Fraction",
        "    General;                                    !- End-Use Subcategory",

        "  Schedule:Constant, OnSched, , 1.0;"

        "Curve:Quadratic,",
        "  fan_curve,               !- Name",
        "  0,                       !- Coefficient1 Constant",
        "  0,                       !- Coefficient2 x",
        "  1,                       !- Coefficient3 x**2",
        "  0,                       !- Minimum Value of x",
        "  1,                       !- Maximum Value of x",
        "  ,                        !- Minimum Curve Output",
        "  ,                        !- Maximum Curve Output",
        "  Dimensionless,           !- Input Unit Type for X",
        "  Dimensionless;           !- Output Unit Type",

        "  AirLoopHVAC:UnitarySystem,",
        "    Sys 2 Furnace DX Cool Unitary System,      !- Name",
        "    SingleZoneVAV,                             !- Control Type",
        "    SPACE2-1,                                  !- Controlling Zone or Thermostat Location",
        "    None,                                      !- Dehumidification Control Type",
        "    ,                                          !- Availability Schedule Name",
        "    Sys 2 Furnace DX Cool Mixed Air Outlet,    !- Air Inlet Node Name",
        "    Sys 2 Furnace DX Cool Supply Fan Outlet,   !- Air Outlet Node Name",
        "    Fan:SystemModel,                           !- Supply Fan Object Type",
        "    Sys 2 Furnace DX Cool Supply Fan,          !- Supply Fan Name",
        "    DrawThrough,                               !- Fan Placement",
        "    OnSched,                                   !- Supply Air Fan Operating Mode Schedule Name",
        "    Coil:Heating:DX:VariableSpeed,             !- Heating Coil Object Type",
        "    Sys 2 Furnace DX Cool Heating Coil,        !- Heating Coil Name",
        "    1.0,                                       !- DX Heating Coil Sizing Ratio",
        "    Coil:Cooling:DX:VariableSpeed,             !- Cooling Coil Object Type",
        "    Sys 2 Furnace DX Cool Cooling Coil,        !- Cooling Coil Name",
        "    ,                                          !- Use DOAS DX Cooling Coil",
        "    10,                                        !- Minimum Supply Air Temperature {C}",
        "    ,                                          !- Latent Load Control",
        "    ,                                          !- Supplemental Heating Coil Object Type",
        "    ,                                          !- Supplemental Heating Coil Name",
        "    SupplyAirFlowRate,                         !- Cooling Supply Air Flow Rate Method",
        "    0.15297,                                   !- Cooling Supply Air Flow Rate {m3/s}",
        "    ,                                          !- Cooling Supply Air Flow Rate Per Floor Area {m3/s-m2}",
        "    ,                                          !- Cooling Fraction of Autosized Cooling Supply Air Flow Rate",
        "    ,                                          !- Cooling Supply Air Flow Rate Per Unit of Capacity {m3/s-W}",
        "    SupplyAirFlowRate,                         !- Heating Supply Air Flow Rate Method",
        "    0.15297,                                   !- Heating Supply Air Flow Rate {m3/s}",
        "    ,                                          !- Heating Supply Air Flow Rate Per Floor Area {m3/s-m2}",
        "    ,                                          !- Heating Fraction of Autosized Heating Supply Air Flow Rate",
        "    ,                                          !- Heating Supply Air Flow Rate Per Unit of Capacity {m3/s-W}",
        "    SupplyAirFlowRate,                         !- No Load Supply Air Flow Rate Method",
        "    0.10203099,                                !- No Load Supply Air Flow Rate {m3/s}",
        "    ,                                          !- No Load Supply Air Flow Rate Per Floor Area {m3/s-m2}",
        "    ,                                          !- No Load Fraction of Autosized Cooling Supply Air Flow Rate",
        "    ,                                          !- No Load Fraction of Autosized Heating Supply Air Flow Rate",
        "    ,                                          !- No Load Supply Air Flow Rate Per Unit of Capacity During Cooling Operation {m3/s-W}",
        "    ,                                          !- No Load Supply Air Flow Rate Per Unit of Capacity During Heating Operation {m3/s-W}",
        "    ,                                          !- No Load Supply Air Flow Rate Control Set To Low Speed",
        "    35,                                        !- Maximum Supply Air Temperature {C}",
        "    21;                                        !- Maximum Outdoor Dry-Bulb Temperature for Supplemental Heater Operation {C}",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    bool ErrorsFound = false;

    // Zone setup
    int NumNodes = 1;
    state->dataGlobal->SysSizingCalc = false;
    state->dataEnvrn->StdRhoAir = Psychrometrics::PsyRhoAirFnPbTdbW(*state, 101325.0, 20.0, 0.0);
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataGlobal->NumOfZones = 1;
    state->dataHeatBal->Zone.allocate(state->dataGlobal->NumOfZones);
    state->dataZoneEquip->ZoneEquipConfig.allocate(state->dataGlobal->NumOfZones);
    state->dataZoneEquip->ZoneEquipList.allocate(state->dataGlobal->NumOfZones);
    state->dataZoneEquip->ZoneEquipAvail.dimension(state->dataGlobal->NumOfZones, Avail::Status::NoAction);
    state->dataHeatBal->Zone(1).Name = "SPACE2-1";
    state->dataZoneEquip->NumOfZoneEquipLists = 1;
    state->dataHeatBal->Zone(1).IsControlled = true;
    state->dataZoneEquip->ZoneEquipConfig(1).IsControlled = true;
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneName = "SPACE2-1";
    state->dataZoneEquip->ZoneEquipConfig(1).EquipListName = "ZONE2EQUIPMENT";
    state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).NumReturnNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ReturnNode(1) = 21;
    state->dataZoneEquip->ZoneEquipConfig(1).FixedReturnFlow.allocate(1);
    state->dataHeatBal->Zone(1).SystemZoneNodeNumber = state->dataZoneEquip->ZoneEquipConfig(1).ZoneNode;
    state->dataZoneEquip->ZoneEquipConfig(1).returnFlowFracSched = Sched::GetScheduleAlwaysOn(*state);
    state->dataZoneEquip->ZoneEquipList(1).Name = "ZONE2EQUIPMENT";
    int maxEquipCount = 1;
    state->dataZoneEquip->ZoneEquipList(1).NumOfEquipTypes = maxEquipCount;
    state->dataZoneEquip->ZoneEquipList(1).EquipTypeName.allocate(state->dataZoneEquip->ZoneEquipList(1).NumOfEquipTypes);
    state->dataZoneEquip->ZoneEquipList(1).EquipType.allocate(state->dataZoneEquip->ZoneEquipList(1).NumOfEquipTypes);
    state->dataZoneEquip->ZoneEquipList(1).EquipName.allocate(state->dataZoneEquip->ZoneEquipList(1).NumOfEquipTypes);
    state->dataZoneEquip->ZoneEquipList(1).EquipIndex.allocate(state->dataZoneEquip->ZoneEquipList(1).NumOfEquipTypes);
    state->dataZoneEquip->ZoneEquipList(1).EquipIndex = 1;
    state->dataZoneEquip->ZoneEquipList(1).EquipData.allocate(state->dataZoneEquip->ZoneEquipList(1).NumOfEquipTypes);
    state->dataZoneEquip->ZoneEquipList(1).CoolingPriority.allocate(state->dataZoneEquip->ZoneEquipList(1).NumOfEquipTypes);
    state->dataZoneEquip->ZoneEquipList(1).HeatingPriority.allocate(state->dataZoneEquip->ZoneEquipList(1).NumOfEquipTypes);
    state->dataZoneEquip->ZoneEquipList(1).EquipTypeName(1) = "AIRLOOPHVAC:UNITARYSYSTEM";
    state->dataZoneEquip->ZoneEquipList(1).EquipName(1) = "Sys 2 Furnace DX Cool Unitary System";
    state->dataZoneEquip->ZoneEquipList(1).CoolingPriority(1) = 1;
    state->dataZoneEquip->ZoneEquipList(1).HeatingPriority(1) = 1;
    state->dataZoneEquip->ZoneEquipList(1).EquipType(1) = DataZoneEquipment::ZoneEquipType::UnitarySystem;
    state->dataZoneEquip->ZoneEquipConfig(1).NumInletNodes = NumNodes;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode.allocate(NumNodes);
    state->dataZoneEquip->ZoneEquipConfig(1).AirDistUnitCool.allocate(NumNodes);
    state->dataZoneEquip->ZoneEquipConfig(1).AirDistUnitHeat.allocate(NumNodes);
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(1) = 2;
    state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = NumNodes;
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode.allocate(NumNodes);
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1) = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).EquipListIndex = 1;

    state->dataSize->CurSysNum = 0;
    state->dataSize->CurZoneEqNum = 1;

    state->dataSize->FinalZoneSizing.allocate(1);
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolVolFlow = 1.5;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow = 1.2;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInTemp = 25.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneTempAtCoolPeak = 25.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneRetTempAtCoolPeak = 25.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesCoolCoilInHumRat = 0.009;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneHumRatAtCoolPeak = 0.009;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDesTemp = 15.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDesHumRat = 0.006;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).heatCoilSizingMethod = DataSizing::HeatCoilSizMethod::CoolingCapacity;

    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatCoilInTemp = 20.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).ZoneTempAtHeatPeak = 20.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).HeatDesTemp = 30.0;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).HeatDesHumRat = 0.007;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatMassFlow =
        state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).DesHeatVolFlow * state->dataEnvrn->StdRhoAir;

    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).TimeStepNumAtCoolMax = 1;
    state->dataSize->FinalZoneSizing(state->dataSize->CurZoneEqNum).CoolDDNum = 1;
    state->dataSize->DesDayWeath.allocate(1);
    state->dataSize->DesDayWeath(1).Temp.allocate(1);
    state->dataSize->DesDayWeath(1).Temp(1) = 35.0;

    state->dataSize->ZoneEqSizing.allocate(1);
    state->dataSize->ZoneEqSizing(state->dataSize->CurZoneEqNum).SizingMethod.allocate(25);
    state->dataSize->ZoneSizingRunDone = true;

    // System setup
    std::string compName = "Sys 2 Furnace DX Cool Unitary System";
    bool zoneEquipment = true;
    bool FirstHVACIteration = true;
    UnitarySystems::UnitarySys::factory(*state, HVAC::UnitarySysType::Unitary_AnyCoilType, compName, zoneEquipment, 0);
    UnitarySystems::UnitarySys *thisSys = &state->dataUnitarySystems->unitarySys[0];
    state->dataZoneEquip->ZoneEquipInputsFilled = true;
    thisSys->getUnitarySystemInputData(*state, compName, zoneEquipment, 0, ErrorsFound);

    FirstHVACIteration = false;
    state->dataGlobal->BeginEnvrnFlag = false;

    int AirLoopNum = 0;
    int CompIndex = 1;
    bool HeatActive = false;
    bool CoolActive = true;
    int constexpr ZoneOAUnitNum = 0;
    Real64 constexpr OAUCoilOutTemp = 0.0;
    bool const ZoneEquipment = true;
    Real64 sensOut = 0.0;
    Real64 latOut = 0.0;
    int ControlZoneNum = 1;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).SequencedOutputRequired.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).SequencedOutputRequiredToCoolingSP.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).SequencedOutputRequiredToHeatingSP.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand(ControlZoneNum).SequencedOutputRequiredToDehumidSP.allocate(1);
    state->dataHeatBalFanSys->TempControlType.allocate(1);
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputRequired = 0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputReqToCoolSP = 0;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputReqToHeatSP = 0;
    state->dataZoneEnergyDemand->ZoneSysMoistureDemand(ControlZoneNum).RemainingOutputReqToDehumidSP = 0;
    state->dataHeatBalFanSys->TempControlType(ControlZoneNum) = HVAC::SetptType::SingleHeatCool;

    state->dataLoopNodes->Node(1).MassFlowRate = thisSys->m_DesignMassFlowRate;
    state->dataLoopNodes->Node(1).MassFlowRateMaxAvail = thisSys->m_DesignMassFlowRate;
    state->dataLoopNodes->Node(1).Temp = 24.0;
    state->dataLoopNodes->Node(1).HumRat = 0.00922;
    state->dataLoopNodes->Node(1).Enthalpy = 47597.03;
    state->dataLoopNodes->Node(3).MassFlowRateMax = thisSys->m_DesignMassFlowRate;

    state->dataGlobal->BeginEnvrnFlag = true;

    // Test for zone 1 - Heating
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputRequired = 500;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputReqToHeatSP = 500;
    thisSys->simulate(*state,
                      thisSys->Name,
                      FirstHVACIteration,
                      AirLoopNum,
                      CompIndex,
                      HeatActive,
                      CoolActive,
                      ZoneOAUnitNum,
                      OAUCoilOutTemp,
                      ZoneEquipment,
                      sensOut,
                      latOut);
    EXPECT_NEAR(state->dataLoopNodes->Node(thisSys->AirOutNode).MassFlowRate, thisSys->MaxNoCoolHeatAirMassFlow, 0.0001);

    // Test for zone 2 - Heating
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputRequired = 1800;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputReqToHeatSP = 1800;
    thisSys->simulate(*state,
                      thisSys->Name,
                      FirstHVACIteration,
                      AirLoopNum,
                      CompIndex,
                      HeatActive,
                      CoolActive,
                      ZoneOAUnitNum,
                      OAUCoilOutTemp,
                      ZoneEquipment,
                      sensOut,
                      latOut);
    EXPECT_TRUE((thisSys->m_MaxHeatAirVolFlow * state->dataEnvrn->StdRhoAir > state->dataLoopNodes->Node(thisSys->AirOutNode).MassFlowRate) &&
                (state->dataLoopNodes->Node(thisSys->AirOutNode).MassFlowRate > thisSys->MaxNoCoolHeatAirMassFlow));
    EXPECT_NEAR(state->dataLoopNodes->Node(thisSys->AirOutNode).Temp, thisSys->DesignMaxOutletTemp, 1.0);

    // Test for zone 3 - Heating
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputRequired = 3000;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputReqToHeatSP = 3000;
    thisSys->simulate(*state,
                      thisSys->Name,
                      FirstHVACIteration,
                      AirLoopNum,
                      CompIndex,
                      HeatActive,
                      CoolActive,
                      ZoneOAUnitNum,
                      OAUCoilOutTemp,
                      ZoneEquipment,
                      sensOut,
                      latOut);
    EXPECT_NEAR(state->dataLoopNodes->Node(thisSys->AirOutNode).MassFlowRate / state->dataEnvrn->StdRhoAir, thisSys->m_MaxHeatAirVolFlow, 0.0001);

    // Test for zone 1 - Cooling
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputRequired = -500;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputReqToHeatSP = -500;
    thisSys->simulate(*state,
                      thisSys->Name,
                      FirstHVACIteration,
                      AirLoopNum,
                      CompIndex,
                      HeatActive,
                      CoolActive,
                      ZoneOAUnitNum,
                      OAUCoilOutTemp,
                      ZoneEquipment,
                      sensOut,
                      latOut);
    EXPECT_NEAR(state->dataLoopNodes->Node(thisSys->AirOutNode).MassFlowRate, thisSys->MaxNoCoolHeatAirMassFlow, 0.0001);

    // Test for zone 2 - Cooling
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputRequired = -2000;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputReqToHeatSP = -2000;
    thisSys->simulate(*state,
                      thisSys->Name,
                      FirstHVACIteration,
                      AirLoopNum,
                      CompIndex,
                      HeatActive,
                      CoolActive,
                      ZoneOAUnitNum,
                      OAUCoilOutTemp,
                      ZoneEquipment,
                      sensOut,
                      latOut);
    EXPECT_TRUE((thisSys->m_MaxCoolAirVolFlow * state->dataEnvrn->StdRhoAir > state->dataLoopNodes->Node(thisSys->AirOutNode).MassFlowRate) &&
                (state->dataLoopNodes->Node(thisSys->AirOutNode).MassFlowRate > thisSys->MaxNoCoolHeatAirMassFlow));
    EXPECT_NEAR(state->dataLoopNodes->Node(thisSys->AirOutNode).Temp, thisSys->DesignMinOutletTemp, 3.0);

    // Test for zone 3 - Cooling
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputRequired = -3500;
    state->dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlZoneNum).RemainingOutputReqToHeatSP = -3500;
    thisSys->simulate(*state,
                      thisSys->Name,
                      FirstHVACIteration,
                      AirLoopNum,
                      CompIndex,
                      HeatActive,
                      CoolActive,
                      ZoneOAUnitNum,
                      OAUCoilOutTemp,
                      ZoneEquipment,
                      sensOut,
                      latOut);
    EXPECT_NEAR(state->dataLoopNodes->Node(thisSys->AirOutNode).MassFlowRate / state->dataEnvrn->StdRhoAir, thisSys->m_MaxCoolAirVolFlow, 0.0001);
}
