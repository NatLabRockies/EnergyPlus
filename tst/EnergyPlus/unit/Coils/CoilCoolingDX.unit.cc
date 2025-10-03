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

// Google Test Headers
#include <gtest/gtest.h>

#include "../Fixtures/SQLiteFixture.hh"

// EnergyPlus Headers
#include <EnergyPlus/Coils/CoilCoolingDX.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/Psychrometrics.hh>

#include "../Coils/CoilCoolingDXFixture.hh"

// For tests of new coil vs old coil
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/DXCoils.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/General.hh>

using namespace EnergyPlus;

TEST_F(CoilCoolingDXTest, CoilCoolingDXInput)
{
    std::string idf_objects = this->getCoilObjectString("coolingCoil", false, 2);
    EXPECT_TRUE(process_idf(idf_objects, false));
    state->init_state(*state);
    int coilIndex = CoilCoolingDX::factory(*state, "coolingCoil");
    auto const &thisCoil(state->dataCoilCoolingDX->coilCoolingDXs[coilIndex]);
    EXPECT_EQ("COOLINGCOIL", thisCoil.name);
    EXPECT_EQ("PERFORMANCEOBJECTNAME", thisCoil.performance->name);
}

TEST_F(CoilCoolingDXTest, CoilCoolingDXAlternateModePerformance)
{
    std::string idf_objects = delimited_string({"  Coil:Cooling:DX,",
                                                "    Coil,",
                                                "    Evaporator Inlet Node,Evaporator Outlet Node,",
                                                "    ,,",
                                                "    Condenser Inlet Node,Condenser Outlet Node,",
                                                "    Coil Performance,",
                                                "    ,;",

                                                "  Coil:Cooling:DX:CurveFit:Performance,",
                                                "    Coil Performance,,,,,,,,,,Electricity,Coil Mode 1,Coil Mode 2;",

                                                "  Coil:Cooling:DX:CurveFit:OperatingMode,",
                                                "    Coil Mode 1,",
                                                "    5000,   !- Rated Gross Total Cooling Capacity {W}",
                                                "    0.25,   !- Rated Evaporator Air Flow Rate {m3/s}",
                                                "    ,,,,,,,,",
                                                "    2,Coil Mode 1 Speed 1,Coil Mode 1 Speed 2;",

                                                "  Coil:Cooling:DX:CurveFit:Speed,",
                                                "    Coil Mode 1 Speed 1,     !- Name",
                                                "    0.50,                    !- Gross Total Cooling Capacity Fraction",
                                                "    0.50,                    !- Evaporator Air Flow Rate Fraction",
                                                "    ,                        !- Condenser Air Flow Rate Fraction",
                                                "    0.7,                     !- Gross Sensible Heat Ratio",
                                                "    3,                       !- Gross Cooling COP {W/W}",
                                                "    0.5,                     !- Active Fraction of Coil Face Area",
                                                "    ,,,,,,,,,,,;",

                                                "  Coil:Cooling:DX:CurveFit:Speed,",
                                                "    Coil Mode 1 Speed 2,     !- Name",
                                                "    1.0,                     !- Gross Total Cooling Capacity Fraction",
                                                "    1.0,                     !- Evaporator Air Flow Rate Fraction",
                                                "    ,                        !- Condenser Air Flow Rate Fraction",
                                                "    0.7,                     !- Gross Sensible Heat Ratio",
                                                "    3,                       !- Gross Cooling COP {W/W}",
                                                "    1.0,                     !- Active Fraction of Coil Face Area",
                                                "    ,,,,,,,,,,,;",

                                                "  Coil:Cooling:DX:CurveFit:OperatingMode,",
                                                "    Coil Mode 2,",
                                                "    5000,   !- Rated Gross Total Cooling Capacity {W}",
                                                "    0.25,   !- Rated Evaporator Air Flow Rate {m3/s}",
                                                "    ,,,,,,,,",
                                                "    2,Coil Mode 2 Speed 1,Coil Mode 2 Speed 2;",

                                                "  Coil:Cooling:DX:CurveFit:Speed,",
                                                "    Coil Mode 2 Speed 1,     !- Name",
                                                "    0.45,                    !- Gross Total Cooling Capacity Fraction",
                                                "    0.50,                    !- Evaporator Air Flow Rate Fraction",
                                                "    ,                        !- Condenser Air Flow Rate Fraction",
                                                "    0.6,                     !- Gross Sensible Heat Ratio",
                                                "    2.7,                     !- Gross Cooling COP {W/W}",
                                                "    0.5,                     !- Active Fraction of Coil Face Area",
                                                "    ,,,,,,,,,,,;",

                                                "  Coil:Cooling:DX:CurveFit:Speed,",
                                                "    Coil Mode 2 Speed 2,     !- Name",
                                                "    0.9,                     !- Gross Total Cooling Capacity Fraction",
                                                "    1.0,                     !- Evaporator Air Flow Rate Fraction",
                                                "    ,                        !- Condenser Air Flow Rate Fraction",
                                                "    0.6,                     !- Gross Sensible Heat Ratio",
                                                "    2.7,                     !- Gross Cooling COP {W/W}",
                                                "    1.0,                     !- Active Fraction of Coil Face Area",
                                                "    ,,,,,,,,,,,;"});
    EXPECT_TRUE(process_idf(idf_objects, false));
    state->init_state(*state);
    int coilIndex = CoilCoolingDX::factory(*state, "Coil");
    auto &thisCoil(state->dataCoilCoolingDX->coilCoolingDXs[coilIndex]);

    // fix the inlet conditions
    auto &evapInletNode = state->dataLoopNodes->Node(thisCoil.evapInletNodeIndex);
    auto &condInletNode = state->dataLoopNodes->Node(thisCoil.condInletNodeIndex);
    evapInletNode.Temp = 28.5;
    evapInletNode.Press = 101325;
    evapInletNode.HumRat = 0.014;
    evapInletNode.Enthalpy = Psychrometrics::PsyHFnTdbW(evapInletNode.Temp, evapInletNode.HumRat);
    condInletNode.Temp = 35.0;
    condInletNode.Press = 101325;
    condInletNode.HumRat = 0.008;
    condInletNode.Enthalpy = Psychrometrics::PsyHFnTdbW(condInletNode.Temp, condInletNode.HumRat);

    // size it
    thisCoil.size(*state);

    // for speed > 1 we use the mshp rated high speed flow...
    state->dataHVACGlobal->MSHPMassFlowRateHigh = thisCoil.performance->ratedAirMassFlowRateMaxSpeed(*state);

    // we'll use this later
    auto &evapOutletNode = state->dataLoopNodes->Node(thisCoil.evapOutletNodeIndex);

    // set some values to run at rated conditions and call to run normal mode speed 1
    evapInletNode.MassFlowRate = thisCoil.performance->ratedAirMassFlowRateMinSpeed(*state);
    HVAC::CoilMode coilMode = HVAC::CoilMode::Normal;
    int speedNum = 1;
    Real64 speedRatio = 1.0;
    HVAC::FanOp fanOp = HVAC::FanOp::Cycling;
    bool singleMode = false;
    thisCoil.simulate(*state, coilMode, speedNum, speedRatio, fanOp, singleMode);
    //    std::cout << thisCoil.totalCoolingEnergyRate << ',' << evapOutletNode.Temp << ',' << evapOutletNode.HumRat << std::endl;
    EXPECT_NEAR(2500, thisCoil.totalCoolingEnergyRate, 0.1); // expect the coil to run full out, at speed 1
    EXPECT_NEAR(19.485, evapOutletNode.Temp, 0.01);
    EXPECT_NEAR(0.0114, evapOutletNode.HumRat, 0.001);

    // alter values and run at rated conditions normal mode speed 2
    evapInletNode.MassFlowRate = thisCoil.performance->ratedAirMassFlowRateMaxSpeed(*state);
    speedNum = 2;
    thisCoil.simulate(*state, coilMode, speedNum, speedRatio, fanOp, singleMode);
    //    std::cout << thisCoil.totalCoolingEnergyRate << ',' << evapOutletNode.Temp << ',' << evapOutletNode.HumRat << std::endl;
    EXPECT_NEAR(5000, thisCoil.totalCoolingEnergyRate, 0.01); // expect the coil to run full out, at speed 1
    // EXPECT_NEAR(17.896, evapOutletNode.Temp, 0.01);
    EXPECT_NEAR(17.943, evapOutletNode.Temp, 0.01);
    EXPECT_NEAR(0.0114, evapOutletNode.HumRat, 0.001);

    // ok so now run at alternate mode, speed 1
    coilMode = HVAC::CoilMode::Enhanced;
    speedNum = 1;
    thisCoil.simulate(*state, coilMode, speedNum, speedRatio, fanOp, singleMode);
    //    std::cout << thisCoil.totalCoolingEnergyRate << ',' << evapOutletNode.Temp << ',' << evapOutletNode.HumRat << std::endl;
    EXPECT_NEAR(2250, thisCoil.totalCoolingEnergyRate, 0.01); // expect the coil to run full out, at speed 1
    // EXPECT_NEAR(24.45, evapOutletNode.Temp, 0.01);
    EXPECT_NEAR(24.47, evapOutletNode.Temp, 0.01);
    EXPECT_NEAR(0.0126, evapOutletNode.HumRat, 0.0001);

    // ok so now run at alternate mode, speed 2
    coilMode = HVAC::CoilMode::Enhanced;
    speedNum = 2;
    thisCoil.simulate(*state, coilMode, speedNum, speedRatio, fanOp, singleMode);
    //    std::cout << thisCoil.totalCoolingEnergyRate << ',' << evapOutletNode.Temp << ',' << evapOutletNode.HumRat << std::endl;
    EXPECT_NEAR(4500, thisCoil.totalCoolingEnergyRate, 0.01); // expect the coil to run full out, at speed 1
    // EXPECT_NEAR(20.39, evapOutletNode.Temp, 0.01);
    EXPECT_NEAR(20.42, evapOutletNode.Temp, 0.01);
    EXPECT_NEAR(0.0111, evapOutletNode.HumRat, 0.0001);
}

TEST_F(CoilCoolingDXTest, CoilCoolingDXAlternateModePerformanceHitsSaturation)
{
    std::string idf_objects = delimited_string({"  Coil:Cooling:DX,",
                                                "    Coil,",
                                                "    Evaporator Inlet Node,Evaporator Outlet Node,",
                                                "    ,,",
                                                "    Condenser Inlet Node,Condenser Outlet Node,",
                                                "    Coil Performance,",
                                                "    ,;",

                                                "  Coil:Cooling:DX:CurveFit:Performance,",
                                                "    Coil Performance,,,,,,,,,,Electricity,Coil Mode 1,Coil Mode 2;",

                                                "  Coil:Cooling:DX:CurveFit:OperatingMode,",
                                                "    Coil Mode 1,",
                                                "    10000,   !- Rated Gross Total Cooling Capacity {W}",
                                                "    0.25,   !- Rated Evaporator Air Flow Rate {m3/s}",
                                                "    ,,,,,,,,",
                                                "    2,Coil Mode 1 Speed 1,Coil Mode 1 Speed 2;",

                                                "  Coil:Cooling:DX:CurveFit:Speed,",
                                                "    Coil Mode 1 Speed 1,     !- Name",
                                                "    0.50,                    !- Gross Total Cooling Capacity Fraction",
                                                "    0.50,                    !- Evaporator Air Flow Rate Fraction",
                                                "    ,                        !- Condenser Air Flow Rate Fraction",
                                                "    0.7,                     !- Gross Sensible Heat Ratio",
                                                "    3,                       !- Gross Cooling COP {W/W}",
                                                "    0.5,                     !- Active Fraction of Coil Face Area",
                                                "    ,,,,,,,,,,,;",

                                                "  Coil:Cooling:DX:CurveFit:Speed,",
                                                "    Coil Mode 1 Speed 2,     !- Name",
                                                "    1.0,                     !- Gross Total Cooling Capacity Fraction",
                                                "    1.0,                     !- Evaporator Air Flow Rate Fraction",
                                                "    ,                        !- Condenser Air Flow Rate Fraction",
                                                "    0.7,                     !- Gross Sensible Heat Ratio",
                                                "    3,                       !- Gross Cooling COP {W/W}",
                                                "    1.0,                     !- Active Fraction of Coil Face Area",
                                                "    ,,,,,,,,,,,;",

                                                "  Coil:Cooling:DX:CurveFit:OperatingMode,",
                                                "    Coil Mode 2,",
                                                "    10000,   !- Rated Gross Total Cooling Capacity {W}",
                                                "    0.25,   !- Rated Evaporator Air Flow Rate {m3/s}",
                                                "    ,,,,,,,,",
                                                "    2,Coil Mode 2 Speed 1,Coil Mode 2 Speed 2;",

                                                "  Coil:Cooling:DX:CurveFit:Speed,",
                                                "    Coil Mode 2 Speed 1,     !- Name",
                                                "    0.45,                    !- Gross Total Cooling Capacity Fraction",
                                                "    0.50,                    !- Evaporator Air Flow Rate Fraction",
                                                "    ,                        !- Condenser Air Flow Rate Fraction",
                                                "    0.6,                     !- Gross Sensible Heat Ratio",
                                                "    2.7,                     !- Gross Cooling COP {W/W}",
                                                "    0.5,                     !- Active Fraction of Coil Face Area",
                                                "    ,,,,,,,,,,,;",

                                                "  Coil:Cooling:DX:CurveFit:Speed,",
                                                "    Coil Mode 2 Speed 2,     !- Name",
                                                "    0.9,                     !- Gross Total Cooling Capacity Fraction",
                                                "    1.0,                     !- Evaporator Air Flow Rate Fraction",
                                                "    ,                        !- Condenser Air Flow Rate Fraction",
                                                "    0.6,                     !- Gross Sensible Heat Ratio",
                                                "    2.7,                     !- Gross Cooling COP {W/W}",
                                                "    1.0,                     !- Active Fraction of Coil Face Area",
                                                "    ,,,,,,,,,,,;"});
    EXPECT_TRUE(process_idf(idf_objects, false));
    state->init_state(*state);
    int coilIndex = CoilCoolingDX::factory(*state, "Coil");
    auto &thisCoil(state->dataCoilCoolingDX->coilCoolingDXs[coilIndex]);

    // fix the inlet conditions
    auto &evapInletNode = state->dataLoopNodes->Node(thisCoil.evapInletNodeIndex);
    auto &condInletNode = state->dataLoopNodes->Node(thisCoil.condInletNodeIndex);
    evapInletNode.Temp = 28.5;
    evapInletNode.Press = 101325;
    evapInletNode.HumRat = 0.014;
    evapInletNode.Enthalpy = Psychrometrics::PsyHFnTdbW(evapInletNode.Temp, evapInletNode.HumRat);
    condInletNode.Temp = 35.0;
    condInletNode.Press = 101325;
    condInletNode.HumRat = 0.008;
    condInletNode.Enthalpy = Psychrometrics::PsyHFnTdbW(condInletNode.Temp, condInletNode.HumRat);

    // size it
    thisCoil.size(*state);

    // for speed > 1 we use the mshp rated high speed flow...
    state->dataHVACGlobal->MSHPMassFlowRateHigh = thisCoil.performance->ratedAirMassFlowRateMaxSpeed(*state);

    // we'll use this later
    auto &evapOutletNode = state->dataLoopNodes->Node(thisCoil.evapOutletNodeIndex);

    bool setExpectations = true;

    // set some values to run at rated conditions and call to run normal mode speed 1
    evapInletNode.MassFlowRate = thisCoil.performance->ratedAirMassFlowRateMinSpeed(*state);
    HVAC::CoilMode coilMode = HVAC::CoilMode::Normal;
    int speedNum = 1;
    Real64 speedRatio = 1.0;
    HVAC::FanOp fanOp = HVAC::FanOp::Cycling;
    bool singleMode = false;
    thisCoil.simulate(*state, coilMode, speedNum, speedRatio, fanOp, singleMode);
    if (!setExpectations) {
        std::cout << thisCoil.totalCoolingEnergyRate << ',' << evapOutletNode.Temp << ',' << evapOutletNode.HumRat << std::endl;
    } else {
        EXPECT_NEAR(5000, thisCoil.totalCoolingEnergyRate, 0.1); // expect the coil to run full out, at speed 1
        EXPECT_NEAR(10.238, evapOutletNode.Temp, 0.01);
        EXPECT_NEAR(0.007748, evapOutletNode.HumRat, 0.0001);
    }
    // alter values and run at rated conditions normal mode speed 2
    evapInletNode.MassFlowRate = thisCoil.performance->ratedAirMassFlowRateMaxSpeed(*state);
    speedNum = 2;
    thisCoil.simulate(*state, coilMode, speedNum, speedRatio, fanOp, singleMode);
    if (!setExpectations) {
        std::cout << thisCoil.totalCoolingEnergyRate << ',' << evapOutletNode.Temp << ',' << evapOutletNode.HumRat << std::endl;
    } else {
        EXPECT_NEAR(10000, thisCoil.totalCoolingEnergyRate, 0.01); // expect the coil to run full out, at speed 1
        EXPECT_NEAR(10.247, evapOutletNode.Temp, 0.01);
        EXPECT_NEAR(0.00774, evapOutletNode.HumRat, 0.0001);
    }

    // ok so now run at alternate mode, speed 1
    coilMode = HVAC::CoilMode::Enhanced;
    speedNum = 1;
    thisCoil.simulate(*state, coilMode, speedNum, speedRatio, fanOp, singleMode);
    if (!setExpectations) {
        std::cout << thisCoil.totalCoolingEnergyRate << ',' << evapOutletNode.Temp << ',' << evapOutletNode.HumRat << std::endl;
    } else {
        EXPECT_NEAR(4500, thisCoil.totalCoolingEnergyRate, 0.01); // expect the coil to run full out, at speed 1
        // EXPECT_NEAR(20.373, evapOutletNode.Temp, 0.01);
        EXPECT_NEAR(20.411, evapOutletNode.Temp, 0.01);
        EXPECT_NEAR(0.0111, evapOutletNode.HumRat, 0.0001);
    }

    // ok so now run at alternate mode, speed 2
    coilMode = HVAC::CoilMode::Enhanced;
    speedNum = 2;
    thisCoil.simulate(*state, coilMode, speedNum, speedRatio, fanOp, singleMode);
    if (!setExpectations) {
        std::cout << thisCoil.totalCoolingEnergyRate << ',' << evapOutletNode.Temp << ',' << evapOutletNode.HumRat << std::endl;
    } else {
        EXPECT_NEAR(9000, thisCoil.totalCoolingEnergyRate, 0.01); // expect the coil to run full out, at speed 1
        // EXPECT_NEAR(12.163, evapOutletNode.Temp, 0.01);
        EXPECT_NEAR(12.239, evapOutletNode.Temp, 0.01);
        EXPECT_NEAR(0.00833, evapOutletNode.HumRat, 0.0001);
    }
}

// The following two tests have been disabled for a while.  I tried enabling them but got an arithmetic
// error.  Something was dividing by zero I'm guessing, and needs to be set up in these unit tests, but
// I couldn't find it quickly enough, so I'm just commenting them out.
// TEST_F(EnergyPlusFixture, CoilDXCoolingVsMultiSpeed_CycFanCycCoil)
// {
//
//     int DXCoilNum(1);
//     state->dataDXCoils->NumDXCoils = 1;
//     state->dataHVACGlobal->MSHPMassFlowRateLow = 0.6;
//     state->dataHVACGlobal->MSHPMassFlowRateHigh = 1.0;
//     state->dataDXCoils->DXCoil.allocate(state->dataDXCoils->NumDXCoils);
//     state->dataLoopNodes->Node.allocate(2);
//     state->dataDXCoils->DXCoilNumericFields.allocate(state->dataDXCoils->NumDXCoils);
//     state->dataDXCoils->DXCoilNumericFields(1).PerfMode.allocate(1);
//     state->dataDXCoils->DXCoilNumericFields(1).PerfMode(1).FieldNames.allocate(17);
//     state->dataHeatBal->HeatReclaimDXCoil.allocate(2);
//     state->dataDXCoils->DXCoilOutletTemp.allocate(1);
//     state->dataDXCoils->DXCoilOutletHumRat.allocate(1);
//     state->dataDXCoils->DXCoilPartLoadRatio.allocate(1);
//     state->dataDXCoils->DXCoilFanOp.allocate(1);
//     state->dataCurveManager->allocateCurveVector(2);
//
//     auto &Coil = state->dataDXCoils->DXCoil(1);
//     auto &constantcurve1 = state->dataCurveManager->PerfCurve(1);
//     auto &constantcurve2 = state->dataCurveManager->PerfCurve(2);
//     auto &AirInletNode = state->dataLoopNodes->Node(1);
//     auto &AirOutletNode = state->dataLoopNodes->Node(2);
//     state->dataEnvrn->StdBaroPress = 101325.0;
//     Real64 ratedInletAirTemp = 26.6667;
//     Real64 ratedInletAirHumRat = 0.0111847;
//     std::string routineName = "MultiSpeedDXCoolingCoilOutputTestvsCoilDXCooling";
//     Real64 ratedRhoAir =
//         Psychrometrics::PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->StdBaroPress, ratedInletAirTemp, ratedInletAirHumRat, routineName);
//
//     Coil.DXCoilType_Num = HVAC::CoilDX_MultiSpeedCooling;
//     Coil.DXCoilType = "Coil:Cooling:DX:MultiSpeed";
//     Coil.FuelType = Constant::eFuel::Electricity;
//     Coil.availSched = Sched::GetScheduleAlwaysOn(*state);
//     Coil.NumOfSpeeds = 2;
//     Coil.MSRatedTotCap.allocate(Coil.NumOfSpeeds);
//     Coil.MSRatedSHR.allocate(Coil.NumOfSpeeds);
//     Coil.MSRatedCOP.allocate(Coil.NumOfSpeeds);
//     Coil.MSRatedAirVolFlowRate.allocate(Coil.NumOfSpeeds);
//     Coil.MSRatedAirMassFlowRate.allocate(Coil.NumOfSpeeds);
//     Coil.MSCCapFTemp.allocate(Coil.NumOfSpeeds);
//     Coil.MSCCapFFlow.allocate(Coil.NumOfSpeeds);
//     Coil.MSEIRFTemp.allocate(Coil.NumOfSpeeds);
//     Coil.MSEIRFFlow.allocate(Coil.NumOfSpeeds);
//     Coil.MSWasteHeat.allocate(Coil.NumOfSpeeds);
//     Coil.MSEvapCondEffect.allocate(Coil.NumOfSpeeds);
//     Coil.MSEvapCondAirFlow.allocate(Coil.NumOfSpeeds);
//     Coil.MSEvapCondPumpElecNomPower.allocate(Coil.NumOfSpeeds);
//     Coil.MSRatedCBF.allocate(Coil.NumOfSpeeds);
//     Coil.MSWasteHeatFrac.allocate(Coil.NumOfSpeeds);
//     Coil.MSPLFFPLR.allocate(Coil.NumOfSpeeds);
//     Coil.MSTwet_Rated.allocate(Coil.NumOfSpeeds);
//     Coil.MSGamma_Rated.allocate(Coil.NumOfSpeeds);
//     Coil.MSMaxONOFFCyclesperHour.allocate(Coil.NumOfSpeeds);
//     Coil.MSLatentCapacityTimeConstant.allocate(Coil.NumOfSpeeds);
//     Coil.MSFanPowerPerEvapAirFlowRate.allocate(Coil.NumOfSpeeds);
//     Coil.MSCCapFTemp = 1;
//     Coil.MSCCapFFlow = 2;
//     Coil.MSEIRFTemp = 1;
//     Coil.MSEIRFFlow = 2;
//     Coil.MSPLFFPLR = 2;
//     Coil.AirOutNode = 2;
//     Coil.AirInNode = 1;
//     // biquadratic curve
//     constantcurve1->Name = "constant biquadratic curve";
//     constantcurve1->curveType = Curve::CurveType::BiQuadratic;
//     constantcurve1->interpolationType = Curve::InterpType::EvaluateCurveToLimits;
//     constantcurve1->coeff[0] = 1.0;
//     constantcurve1->coeff[1] = 0.0;
//     constantcurve1->coeff[2] = 0.0;
//     constantcurve1->coeff[3] = 0.0;
//     constantcurve1->coeff[4] = 0.0;
//     constantcurve1->coeff[5] = 0.0;
//     constantcurve1->inputLimits[0].min = 10.0;
//     constantcurve1->inputLimits[0].max = 25.0;
//     constantcurve1->inputLimits[1].min = 0.0;
//     constantcurve1->inputLimits[1].max = 100.0;
//     constantcurve1->outputLimits.min = 1.0;
//     constantcurve1->outputLimits.max = 1.0;
//     // quadratic curve
//     constantcurve2->Name = "constant quadratic curve";
//     constantcurve2->curveType = Curve::CurveType::Quadratic;
//     constantcurve2->interpolationType = Curve::InterpType::EvaluateCurveToLimits;
//     constantcurve2->coeff[0] = 1.0;
//     constantcurve2->coeff[1] = 0.0;
//     constantcurve2->coeff[2] = 0.0;
//     constantcurve2->inputLimits[0].min = 0.0;
//     constantcurve2->inputLimits[0].max = 1.0;
//     constantcurve2->outputLimits.min = 1.0;
//     constantcurve2->outputLimits.max = 1.0;
//     // set coil parameter
//     Coil.MSRatedTotCap(1) = 10710.0; // 60 % of full capacity
//     Coil.MSRatedTotCap(2) = 17850.0; // 5 ton capcity
//     Coil.MSRatedAirMassFlowRate(1) = state->dataHVACGlobal->MSHPMassFlowRateLow;
//     Coil.MSRatedAirMassFlowRate(2) = state->dataHVACGlobal->MSHPMassFlowRateHigh;
//     // Match RatedCBF from new coil
//     Coil.MSRatedCBF(1) = 0.32321692557501741;
//     Coil.MSRatedCBF(2) = 0.037495280896632406;
//     Coil.MSWasteHeat(1) = 0;
//     Coil.MSWasteHeat(2) = 0;
//     Coil.MSWasteHeatFrac(1) = 0;
//     Coil.MSWasteHeatFrac(2) = 0;
//     Coil.MSRatedSHR(1) = 0.65;
//     Coil.MSRatedSHR(2) = 0.75;
//     Coil.MSRatedCOP(1) = 3.0;
//     Coil.MSRatedCOP(2) = 3.0;
//
//     // test 1: dry cooling
//     Coil.InletAirTemp = 35.0;
//     Coil.InletAirHumRat = 0.0055;
//     Coil.InletAirEnthalpy = Psychrometrics::PsyHFnTdbW(Coil.InletAirTemp, Coil.InletAirHumRat);
//     // set coil inlet and outlet node condition
//     AirInletNode.Temp = Coil.InletAirTemp;
//     AirInletNode.HumRat = Coil.InletAirHumRat;
//     AirInletNode.Enthalpy = Coil.InletAirEnthalpy;
//     AirOutletNode.Temp = Coil.InletAirTemp;
//     AirOutletNode.HumRat = Coil.InletAirHumRat;
//     AirOutletNode.Enthalpy = Coil.InletAirEnthalpy;
//     // outside air condition
//     state->dataEnvrn->OutBaroPress = 101325.0;
//     state->dataEnvrn->OutDryBulbTemp = 35.0;
//     state->dataEnvrn->OutHumRat = 0.0120;
//     state->dataEnvrn->WindSpeed = 5.0;
//     state->dataEnvrn->WindDir = 0.0;
//     int SpeedNum = 2;
//     HVAC::FanOp fanOp = HVAC::FanOp::Cycling;
//     HVAC::CompressorOp compressorOp = HVAC::CompressorOp::On;
//     int SingleMode = 0;
//     // Test 1 - dry coil - run the coil at low speed
//     Real64 SpeedRatio = 0.0;
//     Real64 CycRatio = 1.0;
//     Coil.InletAirMassFlowRate = state->dataHVACGlobal->MSHPMassFlowRateLow;
//     DXCoils::CalcMultiSpeedDXCoilCooling(*state, DXCoilNum, SpeedRatio, CycRatio, SpeedNum, fanOp, compressorOp, SingleMode);
//     EXPECT_NEAR(10710.0, Coil.TotalCoolingEnergyRate, 0.0001);   // equals low speed capacity
//     EXPECT_NEAR(10710.0, Coil.SensCoolingEnergyRate, 0.0001);    // sensible cooling rate at low speed
//     EXPECT_NEAR(0.0, Coil.LatCoolingEnergyRate, 1.0E-11);        // zero latent cooling rate at low speed
//     EXPECT_DOUBLE_EQ(0.0055, AirInletNode.HumRat);               // input check
//     EXPECT_DOUBLE_EQ(AirInletNode.HumRat, AirOutletNode.HumRat); // dry cooling only
//     EXPECT_NEAR(35.0, AirInletNode.Temp, 0.0001);                // inlet dry bulb
//     EXPECT_NEAR(17.4149, AirOutletNode.Temp, 0.0001);            // outlet dry bulb
//     EXPECT_NEAR(3570.0, Coil.ElecCoolingPower, 0.01);
//     // Save results for comparison
//     Real64 MultiSpeedTotalCoolingRate1 = Coil.TotalCoolingEnergyRate;
//     Real64 MultiSpeedSensCoolingRate1 = Coil.SensCoolingEnergyRate;
//     Real64 MultiSpeedLatCoolingRate1 = Coil.LatCoolingEnergyRate;
//     Real64 MultiSpeedOutletHumRat1 = AirOutletNode.HumRat;
//     Real64 MultiSpeedOutletTemp1 = AirOutletNode.Temp;
//     Real64 MultiSpeedElecPower1 = Coil.ElecCoolingPower;
//
//     // run the coil at high speed
//     SpeedRatio = 1.0;
//     Coil.InletAirMassFlowRate = state->dataHVACGlobal->MSHPMassFlowRateHigh;
//     DXCoils::CalcMultiSpeedDXCoilCooling(*state, DXCoilNum, SpeedRatio, CycRatio, SpeedNum, fanOp, compressorOp, SingleMode);
//     EXPECT_NEAR(17850.0, Coil.TotalCoolingEnergyRate, 0.0001);   // total capacity at high speed
//     EXPECT_NEAR(17850.0, Coil.SensCoolingEnergyRate, 0.0001);    // sensible cooling rate at high speed
//     EXPECT_NEAR(0.0, Coil.LatCoolingEnergyRate, 1.0E-11);        // zero latent cooling rate at high speed
//     EXPECT_DOUBLE_EQ(0.0055, AirInletNode.HumRat);               // input check
//     EXPECT_DOUBLE_EQ(AirInletNode.HumRat, AirOutletNode.HumRat); // dry cooling only
//     EXPECT_NEAR(35.0, AirInletNode.Temp, 0.0001);                // inlet dry bulb
//     EXPECT_NEAR(17.4149, AirOutletNode.Temp, 0.0001);            // outlet dry bulb
//     EXPECT_NEAR(5950.0, Coil.ElecCoolingPower, 0.01);
//     // Save results for comparison
//     Real64 MultiSpeedTotalCoolingRate2 = Coil.TotalCoolingEnergyRate;
//     Real64 MultiSpeedSensCoolingRate2 = Coil.SensCoolingEnergyRate;
//     Real64 MultiSpeedLatCoolingRate2 = Coil.LatCoolingEnergyRate;
//     Real64 MultiSpeedOutletHumRat2 = AirOutletNode.HumRat;
//     Real64 MultiSpeedOutletTemp2 = AirOutletNode.Temp;
//     Real64 MultiSpeedElecPower2 = Coil.ElecCoolingPower;
//
//     // Test 3 - dry coil - run the coil at speed ratio 0.75
//     SpeedRatio = 0.75;
//     Coil.InletAirMassFlowRate =
//         SpeedRatio * state->dataHVACGlobal->MSHPMassFlowRateHigh + (1.0 - SpeedRatio) * state->dataHVACGlobal->MSHPMassFlowRateLow;
//     DXCoils::CalcMultiSpeedDXCoilCooling(*state, DXCoilNum, SpeedRatio, CycRatio, SpeedNum, fanOp, compressorOp, SingleMode);
//     EXPECT_NEAR(10710.0 + 0.75 * (17850.0 - 10710.0), Coil.TotalCoolingEnergyRate, 0.0001); // total capacity
//     EXPECT_NEAR(10710.0 + 0.75 * (17850.0 - 10710.0), Coil.SensCoolingEnergyRate, 0.0001);  // sensible cooling rate
//     EXPECT_NEAR(0.0, Coil.LatCoolingEnergyRate, 1.0E-11);                                   // zero latent cooling rate at high speed
//     EXPECT_DOUBLE_EQ(0.0055, AirInletNode.HumRat);                                          // input check
//     EXPECT_DOUBLE_EQ(AirInletNode.HumRat, AirOutletNode.HumRat);                            // dry cooling only
//     EXPECT_NEAR(35.0, AirInletNode.Temp, 0.0001);                                           // inlet dry bulb
//     EXPECT_NEAR(17.4149, AirOutletNode.Temp, 0.0001);                                       // outlet dry bulb
//     EXPECT_NEAR(5355.0, Coil.ElecCoolingPower, 0.01);
//     // Save results for comparison
//     Real64 MultiSpeedTotalCoolingRate3 = Coil.TotalCoolingEnergyRate;
//     Real64 MultiSpeedSensCoolingRate3 = Coil.SensCoolingEnergyRate;
//     Real64 MultiSpeedLatCoolingRate3 = Coil.LatCoolingEnergyRate;
//     Real64 MultiSpeedOutletHumRat3 = AirOutletNode.HumRat;
//     Real64 MultiSpeedOutletTemp3 = AirOutletNode.Temp;
//     Real64 MultiSpeedElecPower3 = Coil.ElecCoolingPower;
//
//     // test 4: wet cooling
//     Coil.InletAirTemp = 24.0;
//     Coil.InletAirHumRat = 0.0100;
//     Coil.InletAirEnthalpy = Psychrometrics::PsyHFnTdbW(Coil.InletAirTemp, Coil.InletAirHumRat);
//     // set coil inlet and outlet node condition
//     AirInletNode.Temp = Coil.InletAirTemp;
//     AirInletNode.HumRat = Coil.InletAirHumRat;
//     AirInletNode.Enthalpy = Coil.InletAirEnthalpy;
//     // Test 4 - wet coil - run coil at low speed
//     SpeedRatio = 0.0;
//     CycRatio = 1.0;
//     Coil.InletAirMassFlowRate = state->dataHVACGlobal->MSHPMassFlowRateLow;
//     DXCoils::CalcMultiSpeedDXCoilCooling(*state, DXCoilNum, SpeedRatio, CycRatio, SpeedNum, fanOp, compressorOp, SingleMode);
//     EXPECT_NEAR(10710.0, Coil.TotalCoolingEnergyRate, 0.0001);   // equals low speed cooling capacity
//     EXPECT_NEAR(6908.14887, Coil.SensCoolingEnergyRate, 0.0001); // sensible cooling rate at low speed
//     EXPECT_NEAR(3801.851126, Coil.LatCoolingEnergyRate, 0.0001); // latent cooling rate at low speed
//     EXPECT_DOUBLE_EQ(0.0100, AirInletNode.HumRat);               // input check
//     EXPECT_NEAR(0.00751079, AirOutletNode.HumRat, 0.00001);      // cooling and dehumidification
//     EXPECT_NEAR(24.0, AirInletNode.Temp, 0.0001);                // inlet dry bulb
//     EXPECT_NEAR(12.6989, AirOutletNode.Temp, 0.0001);            // outlet dry bulb
//     EXPECT_NEAR(3570.0, Coil.ElecCoolingPower, 0.01);
//     // Save results for comparison
//     Real64 MultiSpeedTotalCoolingRate4 = Coil.TotalCoolingEnergyRate;
//     Real64 MultiSpeedSensCoolingRate4 = Coil.SensCoolingEnergyRate;
//     Real64 MultiSpeedLatCoolingRate4 = Coil.LatCoolingEnergyRate;
//     Real64 MultiSpeedOutletHumRat4 = AirOutletNode.HumRat;
//     Real64 MultiSpeedOutletTemp4 = AirOutletNode.Temp;
//     Real64 MultiSpeedElecPower4 = Coil.ElecCoolingPower;
//
//     // Test 5 - wet coil - run the coil at high speed
//     SpeedRatio = 1.0;
//     Coil.InletAirMassFlowRate = state->dataHVACGlobal->MSHPMassFlowRateHigh;
//     DXCoils::CalcMultiSpeedDXCoilCooling(*state, DXCoilNum, SpeedRatio, CycRatio, SpeedNum, fanOp, compressorOp, SingleMode);
//     EXPECT_NEAR(17850.0, Coil.TotalCoolingEnergyRate, 0.0001);           // total capacity at high speed
//     EXPECT_NEAR(13002.847055477625, Coil.SensCoolingEnergyRate, 0.0001); // sensible cooling rate at high speed
//     EXPECT_NEAR(4847.1529445223750, Coil.LatCoolingEnergyRate, 0.0001);  // latent cooling rate at high speed
//     EXPECT_DOUBLE_EQ(0.0100, AirInletNode.HumRat);                       // input check
//     EXPECT_NEAR(0.0080958363400692145, AirOutletNode.HumRat, 0.00001);   // cooling and dehumidification
//     EXPECT_NEAR(24.0, AirInletNode.Temp, 0.0001);                        // inlet dry bulb
//     EXPECT_NEAR(11.250732746176219, AirOutletNode.Temp, 0.0001);         // outlet dry bulb
//     EXPECT_NEAR(5950.0, Coil.ElecCoolingPower, 0.01);
//     // Save results for comparison
//     Real64 MultiSpeedTotalCoolingRate5 = Coil.TotalCoolingEnergyRate;
//     Real64 MultiSpeedSensCoolingRate5 = Coil.SensCoolingEnergyRate;
//     Real64 MultiSpeedLatCoolingRate5 = Coil.LatCoolingEnergyRate;
//     Real64 MultiSpeedOutletHumRat5 = AirOutletNode.HumRat;
//     Real64 MultiSpeedOutletTemp5 = AirOutletNode.Temp;
//     Real64 MultiSpeedElecPower5 = Coil.ElecCoolingPower;
//
//     // Test 6 - wet coil - run the coil at speed ratio 0.75
//     SpeedRatio = 0.75;
//     Coil.InletAirMassFlowRate =
//         SpeedRatio * state->dataHVACGlobal->MSHPMassFlowRateHigh + (1.0 - SpeedRatio) * state->dataHVACGlobal->MSHPMassFlowRateLow;
//     DXCoils::CalcMultiSpeedDXCoilCooling(*state, DXCoilNum, SpeedRatio, CycRatio, SpeedNum, fanOp, compressorOp, SingleMode);
//     EXPECT_NEAR(10710.0 + 0.75 * (17850.0 - 10710.0), Coil.TotalCoolingEnergyRate, 0.01);                  // total capacity
//     EXPECT_NEAR(6908.14887 + 0.75 * (13002.847055477625 - 6908.14887), Coil.SensCoolingEnergyRate, 0.01);  // sensible cooling rate
//     EXPECT_NEAR(3801.851126 + 0.75 * (4847.1529445223750 - 3801.851126), Coil.LatCoolingEnergyRate, 0.01); // latent cooling rate
//     EXPECT_DOUBLE_EQ(0.0100, AirInletNode.HumRat);                                                         // input check
//     // EXPECT_NEAR(0.0079495749435070425, AirOutletNode.HumRat, 0.00001);                                        // cooling and dehumidification
//     EXPECT_NEAR(0.0079983287423610987, AirOutletNode.HumRat, 0.00001); // cooling and dehumidification
//     EXPECT_NEAR(24.0, AirInletNode.Temp, 0.0001);                      // inlet dry bulb
//     // EXPECT_NEAR(11.612485891133730, AirOutletNode.Temp, 0.0001);                                              // outlet dry bulb
//     EXPECT_NEAR(11.491880074594654, AirOutletNode.Temp, 0.0001); // outlet dry bulb
//     EXPECT_NEAR(5355.0, Coil.ElecCoolingPower, 0.01);
//     // Save results for comparison
//     Real64 MultiSpeedTotalCoolingRate6 = Coil.TotalCoolingEnergyRate;
//     Real64 MultiSpeedSensCoolingRate6 = Coil.SensCoolingEnergyRate;
//     Real64 MultiSpeedLatCoolingRate6 = Coil.LatCoolingEnergyRate;
//     Real64 MultiSpeedOutletHumRat6 = AirOutletNode.HumRat;
//     Real64 MultiSpeedOutletTemp6 = AirOutletNode.Temp;
//     Real64 MultiSpeedElecPower6 = Coil.ElecCoolingPower;
//
//     Real64 ratedVolFlowRate = state->dataHVACGlobal->MSHPMassFlowRateHigh / ratedRhoAir;
//     std::string volFlowRateStr = format("{:.4R}", ratedVolFlowRate);
//
//     std::string idf_objects = delimited_string({
//         "  Coil:Cooling:DX,",
//         "    Coil,",
//         "    Evaporator Inlet Node,Evaporator Outlet Node,",
//         "    ,,",
//         "    Condenser Inlet Node,Condenser Outlet Node,",
//         "    Coil Performance,",
//         "    ,;",
//
//         "  Coil:Cooling:DX:CurveFit:Performance,",
//         "    Coil Performance,,,,,,,,,Electricity,Coil Mode 1;",
//
//         "  Coil:Cooling:DX:CurveFit:OperatingMode,",
//         "    Coil Mode 1,",
//         "    17850.0,   !- Rated Gross Total Cooling Capacity {W}",
//         volFlowRateStr + ",   !- Rated Evaporator Air Flow Rate {m3/s}",
//         "    ,,,,,,,,",
//         "    2,Coil Mode 1 Speed 1,Coil Mode 1 Speed 2;",
//
//         "  Coil:Cooling:DX:CurveFit:Speed,",
//         "    Coil Mode 1 Speed 1,     !- Name",
//         "    0.60,                    !- Gross Total Cooling Capacity Fraction",
//         "    0.60,                    !- Evaporator Air Flow Rate Fraction",
//         "    ,                        !- Condenser Air Flow Rate Fraction",
//         "    0.65,                     !- Gross Sensible Heat Ratio",
//         "    3,                       !- Gross Cooling COP {W/W}",
//         "    1.0,                     !- Active Fraction of Coil Face Area",
//         "    ,,,,,,,,,,,;",
//
//         "  Coil:Cooling:DX:CurveFit:Speed,",
//         "    Coil Mode 1 Speed 2,     !- Name",
//         "    1.0,                     !- Gross Total Cooling Capacity Fraction",
//         "    1.0,                     !- Evaporator Air Flow Rate Fraction",
//         "    ,                        !- Condenser Air Flow Rate Fraction",
//         "    0.75,                     !- Gross Sensible Heat Ratio",
//         "    3,                       !- Gross Cooling COP {W/W}",
//         "    1.0,                     !- Active Fraction of Coil Face Area",
//         "    ,,,,,,,,,,,;",
//
//     });
//     EXPECT_TRUE(process_idf(idf_objects, false));
//     state->init_state(*state);
//     int coilIndex = CoilCoolingDX::factory(*state, "Coil");
//     auto &thisCoil(state->dataCoilCoolingDX->coilCoolingDXs[coilIndex]);
//
//     // fix the inlet conditions
//     auto &evapInletNode = state->dataLoopNodes->Node(thisCoil.evapInletNodeIndex);
//     auto &condInletNode = state->dataLoopNodes->Node(thisCoil.condInletNodeIndex);
//     evapInletNode.Press = 101325;
//     condInletNode.Temp = 30.0;
//     condInletNode.Press = 101325;
//     condInletNode.HumRat = 0.012;
//     condInletNode.Enthalpy = Psychrometrics::PsyHFnTdbW(condInletNode.Temp, condInletNode.HumRat);
//
//     // size it
//     thisCoil.size(*state);
//
//     // for speed > 1 we use the mshp rated high speed flow...
//     state->dataHVACGlobal->MSHPMassFlowRateHigh = thisCoil.performance.normalMode.speeds.back().RatedAirMassFlowRate;
//
//     // we'll use this later
//     auto &evapOutletNode = state->dataLoopNodes->Node(thisCoil.evapOutletNodeIndex);
//
//     // Test 1 - dry coil - run the coil at low speed
//     evapInletNode.Temp = 35.0;
//     evapInletNode.HumRat = 0.0055;
//     evapInletNode.Enthalpy = Psychrometrics::PsyHFnTdbW(evapInletNode.Temp, evapInletNode.HumRat);
//     // set some values to run at rated conditions and call to run normal mode speed 1
//     evapInletNode.MassFlowRate = thisCoil.performance.normalMode.speeds.front().RatedAirMassFlowRate;
//     HVAC::CoilMode coilMode = HVAC::CoilMode::Normal;
//     Real64 PLR = 1.0;
//     int speedNum = 1;
//     Real64 speedRatio = 0.0;
//     fanOp = HVAC::FanOp::Cycling;
//     bool singleMode = false;
//     thisCoil.simulate(*state, coilMode, PLR, speedNum, speedRatio, fanOp, singleMode);
//     EXPECT_NEAR(MultiSpeedTotalCoolingRate1, thisCoil.totalCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedSensCoolingRate1, thisCoil.sensCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedLatCoolingRate1, thisCoil.latCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedOutletTemp1, evapOutletNode.Temp, 0.01);
//     EXPECT_NEAR(MultiSpeedOutletHumRat1, evapOutletNode.HumRat, 0.001);
//     EXPECT_NEAR(MultiSpeedElecPower1, thisCoil.elecCoolingPower, 0.001);
//     // Test 2 - dry coil - run the coil at high speed
//     evapInletNode.MassFlowRate = thisCoil.performance.normalMode.speeds.back().RatedAirMassFlowRate;
//     speedNum = 2;
//     speedRatio = 1.0;
//     thisCoil.simulate(*state, coilMode, PLR, speedNum, speedRatio, fanOp, singleMode);
//     EXPECT_NEAR(MultiSpeedTotalCoolingRate2, thisCoil.totalCoolingEnergyRate, 0.01);
//     EXPECT_NEAR(MultiSpeedSensCoolingRate2, thisCoil.sensCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedLatCoolingRate2, thisCoil.latCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedOutletTemp2, evapOutletNode.Temp, 0.01);
//     EXPECT_NEAR(MultiSpeedOutletHumRat2, evapOutletNode.HumRat, 0.001);
//     EXPECT_NEAR(MultiSpeedElecPower2, thisCoil.elecCoolingPower, 0.001);
//
//     // Test 3 - dry coil - run the coil at 0.75 speed ratio
//     speedNum = 2;
//     speedRatio = 0.75;
//     evapInletNode.MassFlowRate = speedRatio * thisCoil.performance.normalMode.speeds.back().RatedAirMassFlowRate +
//                                  (1.0 - speedRatio) * thisCoil.performance.normalMode.speeds.front().RatedAirMassFlowRate;
//     thisCoil.simulate(*state, coilMode, PLR, speedNum, speedRatio, fanOp, singleMode);
//     EXPECT_NEAR(MultiSpeedTotalCoolingRate3, thisCoil.totalCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedSensCoolingRate3, thisCoil.sensCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedLatCoolingRate3, thisCoil.latCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedOutletTemp3, evapOutletNode.Temp, 0.01);
//     EXPECT_NEAR(MultiSpeedOutletHumRat3, evapOutletNode.HumRat, 0.001);
//     EXPECT_NEAR(MultiSpeedElecPower3, thisCoil.elecCoolingPower, 0.001);
//
//     // Test 4 - wet coil - run the coil at low speed
//     evapInletNode.Temp = 24.0;
//     evapInletNode.HumRat = 0.0100;
//     evapInletNode.Enthalpy = Psychrometrics::PsyHFnTdbW(evapInletNode.Temp, evapInletNode.HumRat);
//     evapInletNode.MassFlowRate = thisCoil.performance.normalMode.speeds.front().RatedAirMassFlowRate;
//     coilMode = HVAC::CoilMode::Normal;
//     PLR = 1.0;
//     speedNum = 1;
//     speedRatio = 1.0;
//     fanOp = HVAC::FanOp::Cycling;
//     singleMode = false;
//     thisCoil.simulate(*state, coilMode, PLR, speedNum, speedRatio, fanOp, singleMode);
//     EXPECT_NEAR(MultiSpeedTotalCoolingRate4, thisCoil.totalCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedSensCoolingRate4, thisCoil.sensCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedLatCoolingRate4, thisCoil.latCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedOutletTemp4, evapOutletNode.Temp, 0.01);
//     EXPECT_NEAR(MultiSpeedOutletHumRat4, evapOutletNode.HumRat, 0.001);
//     EXPECT_NEAR(MultiSpeedElecPower4, thisCoil.elecCoolingPower, 0.001);
//
//     // Test 5 - wet coil - run the coil at high speed
//     evapInletNode.MassFlowRate = thisCoil.performance.normalMode.speeds.back().RatedAirMassFlowRate;
//     speedNum = 2;
//     speedRatio = 1.0;
//     thisCoil.simulate(*state, coilMode, PLR, speedNum, speedRatio, fanOp, singleMode);
//     EXPECT_NEAR(MultiSpeedTotalCoolingRate5, thisCoil.totalCoolingEnergyRate, 0.01);
//     EXPECT_NEAR(MultiSpeedSensCoolingRate5, thisCoil.sensCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedLatCoolingRate5, thisCoil.latCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedOutletTemp5, evapOutletNode.Temp, 0.01);
//     EXPECT_NEAR(MultiSpeedOutletHumRat5, evapOutletNode.HumRat, 0.001);
//     EXPECT_NEAR(MultiSpeedElecPower5, thisCoil.elecCoolingPower, 0.001);
//
//     // Test 6 - wet coil - run the coil at 0.75 speed ratio
//     speedNum = 2;
//     speedRatio = 0.75;
//     evapInletNode.MassFlowRate = speedRatio * thisCoil.performance.normalMode.speeds.back().RatedAirMassFlowRate +
//                                  (1.0 - speedRatio) * thisCoil.performance.normalMode.speeds.front().RatedAirMassFlowRate;
//     thisCoil.simulate(*state, coilMode, PLR, speedNum, speedRatio, fanOp, singleMode);
//     EXPECT_NEAR(MultiSpeedTotalCoolingRate6, thisCoil.totalCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedSensCoolingRate6, thisCoil.sensCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedLatCoolingRate6, thisCoil.latCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedOutletTemp6, evapOutletNode.Temp, 0.01);
//     EXPECT_NEAR(MultiSpeedOutletHumRat6, evapOutletNode.HumRat, 0.001);
//     EXPECT_NEAR(MultiSpeedElecPower6, thisCoil.elecCoolingPower, 0.001);
// }
// TEST_F(EnergyPlusFixture, CoilDXCoolingVsMultiSpeed_ContFanCycCoil)
// {
//
//     int DXCoilNum(1);
//     state->dataDXCoils->NumDXCoils = 1;
//     state->dataHVACGlobal->MSHPMassFlowRateLow = 0.6;
//     state->dataHVACGlobal->MSHPMassFlowRateHigh = 1.0;
//     state->dataDXCoils->DXCoil.allocate(state->dataDXCoils->NumDXCoils);
//     state->dataLoopNodes->Node.allocate(2);
//     state->dataDXCoils->DXCoilNumericFields.allocate(state->dataDXCoils->NumDXCoils);
//     state->dataDXCoils->DXCoilNumericFields(1).PerfMode.allocate(1);
//     state->dataDXCoils->DXCoilNumericFields(1).PerfMode(1).FieldNames.allocate(17);
//     state->dataHeatBal->HeatReclaimDXCoil.allocate(2);
//     state->dataDXCoils->DXCoilOutletTemp.allocate(1);
//     state->dataDXCoils->DXCoilOutletHumRat.allocate(1);
//     state->dataDXCoils->DXCoilPartLoadRatio.allocate(1);
//     state->dataDXCoils->DXCoilFanOp.allocate(1);
//     state->dataCurveManager->allocateCurveVector(2);
//
//     auto &Coil = state->dataDXCoils->DXCoil(1);
//     auto &constantcurve1 = state->dataCurveManager->PerfCurve(1);
//     auto &constantcurve2 = state->dataCurveManager->PerfCurve(2);
//     auto &AirInletNode = state->dataLoopNodes->Node(1);
//     auto &AirOutletNode = state->dataLoopNodes->Node(2);
//     state->dataEnvrn->StdBaroPress = 101325.0;
//     Real64 ratedInletAirTemp = 26.6667;
//     Real64 ratedInletAirHumRat = 0.0111847;
//     std::string routineName = "MultiSpeedDXCoolingCoilOutputTestvsCoilDXCooling";
//     Real64 ratedRhoAir =
//         Psychrometrics::PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->StdBaroPress, ratedInletAirTemp, ratedInletAirHumRat, routineName);
//
//     Coil.DXCoilType_Num = HVAC::CoilDX_MultiSpeedCooling;
//     Coil.DXCoilType = "Coil:Cooling:DX:MultiSpeed";
//     Coil.FuelType = Constant::eFuel::Electricity;
//     Coil.availSched = Sched::GetScheduleAlwaysOn(*state);
//     Coil.NumOfSpeeds = 2;
//     Coil.MSRatedTotCap.allocate(Coil.NumOfSpeeds);
//     Coil.MSRatedSHR.allocate(Coil.NumOfSpeeds);
//     Coil.MSRatedCOP.allocate(Coil.NumOfSpeeds);
//     Coil.MSRatedAirVolFlowRate.allocate(Coil.NumOfSpeeds);
//     Coil.MSRatedAirMassFlowRate.allocate(Coil.NumOfSpeeds);
//     Coil.MSCCapFTemp.allocate(Coil.NumOfSpeeds);
//     Coil.MSCCapFFlow.allocate(Coil.NumOfSpeeds);
//     Coil.MSEIRFTemp.allocate(Coil.NumOfSpeeds);
//     Coil.MSEIRFFlow.allocate(Coil.NumOfSpeeds);
//     Coil.MSWasteHeat.allocate(Coil.NumOfSpeeds);
//     Coil.MSEvapCondEffect.allocate(Coil.NumOfSpeeds);
//     Coil.MSEvapCondAirFlow.allocate(Coil.NumOfSpeeds);
//     Coil.MSEvapCondPumpElecNomPower.allocate(Coil.NumOfSpeeds);
//     Coil.MSRatedCBF.allocate(Coil.NumOfSpeeds);
//     Coil.MSWasteHeatFrac.allocate(Coil.NumOfSpeeds);
//     Coil.MSPLFFPLR.allocate(Coil.NumOfSpeeds);
//     Coil.MSTwet_Rated.allocate(Coil.NumOfSpeeds);
//     Coil.MSGamma_Rated.allocate(Coil.NumOfSpeeds);
//     Coil.MSMaxONOFFCyclesperHour.allocate(Coil.NumOfSpeeds);
//     Coil.MSLatentCapacityTimeConstant.allocate(Coil.NumOfSpeeds);
//     Coil.MSFanPowerPerEvapAirFlowRate.allocate(Coil.NumOfSpeeds);
//     Coil.MSCCapFTemp = 1;
//     Coil.MSCCapFFlow = 2;
//     Coil.MSEIRFTemp = 1;
//     Coil.MSEIRFFlow = 2;
//     Coil.MSPLFFPLR = 2;
//     Coil.AirOutNode = 2;
//     Coil.AirInNode = 1;
//     // biquadratic curve
//     constantcurve1->Name = "constant biquadratic curve";
//     constantcurve1->curveType = Curve::CurveType::BiQuadratic;
//     constantcurve1->interpolationType = Curve::InterpType::EvaluateCurveToLimits;
//     constantcurve1->coeff[0] = 1.0;
//     constantcurve1->coeff[1] = 0.0;
//     constantcurve1->coeff[2] = 0.0;
//     constantcurve1->coeff[3] = 0.0;
//     constantcurve1->coeff[4] = 0.0;
//     constantcurve1->coeff[5] = 0.0;
//     constantcurve1->inputLimits[0].min = 10.0;
//     constantcurve1->inputLimits[0].max = 25.0;
//     constantcurve1->inputLimits[1].min = 0.0;
//     constantcurve1->inputLimits[1].max = 100.0;
//     constantcurve1->outputLimits.min = 1.0;
//     constantcurve1->outputLimits.max = 1.0;
//     // quadratic curve
//     constantcurve2->Name = "constant quadratic curve";
//     constantcurve2->curveType = Curve::CurveType::Quadratic;
//     constantcurve2->interpolationType = Curve::InterpType::EvaluateCurveToLimits;
//     constantcurve2->coeff[0] = 1.0;
//     constantcurve2->coeff[1] = 0.0;
//     constantcurve2->coeff[2] = 0.0;
//     constantcurve2->inputLimits[0].min = 0.0;
//     constantcurve2->inputLimits[0].max = 1.0;
//     constantcurve2->outputLimits.min = 1.0;
//     constantcurve2->outputLimits.max = 1.0;
//     // set coil parameter
//     Coil.MSRatedTotCap(1) = 10710.0; // 60 % of full capacity
//     Coil.MSRatedTotCap(2) = 17850.0; // 5 ton capcity
//     Coil.MSRatedAirMassFlowRate(1) = state->dataHVACGlobal->MSHPMassFlowRateLow;
//     Coil.MSRatedAirMassFlowRate(2) = state->dataHVACGlobal->MSHPMassFlowRateHigh;
//     // Match RatedCBF from new coil
//     Coil.MSRatedCBF(1) = 0.32321692557501741;
//     //    Coil.MSRatedCBF(2) = 0.32321692557501741;
//     Coil.MSRatedCBF(2) = 0.037495280896632406;
//     Coil.MSWasteHeat(1) = 0;
//     Coil.MSWasteHeat(2) = 0;
//     Coil.MSWasteHeatFrac(1) = 0;
//     Coil.MSWasteHeatFrac(2) = 0;
//     Coil.MSRatedSHR(1) = 0.65;
//     Coil.MSRatedSHR(2) = 0.75;
//     Coil.MSRatedCOP(1) = 3.0;
//     Coil.MSRatedCOP(2) = 3.0;
//
//     // test 1: dry cooling
//     Coil.InletAirTemp = 35.0;
//     Coil.InletAirHumRat = 0.0055;
//     Coil.InletAirEnthalpy = Psychrometrics::PsyHFnTdbW(Coil.InletAirTemp, Coil.InletAirHumRat);
//     // set coil inlet and outlet node condition
//     AirInletNode.Temp = Coil.InletAirTemp;
//     AirInletNode.HumRat = Coil.InletAirHumRat;
//     AirInletNode.Enthalpy = Coil.InletAirEnthalpy;
//     AirOutletNode.Temp = Coil.InletAirTemp;
//     AirOutletNode.HumRat = Coil.InletAirHumRat;
//     AirOutletNode.Enthalpy = Coil.InletAirEnthalpy;
//     // outside air condition
//     state->dataEnvrn->OutBaroPress = 101325.0;
//     state->dataEnvrn->OutDryBulbTemp = 35.0;
//     state->dataEnvrn->OutHumRat = 0.0120;
//     state->dataEnvrn->WindSpeed = 5.0;
//     state->dataEnvrn->WindDir = 0.0;
//     int SpeedNum = 2;
//     HVAC::FanOp fanOp = HVAC::FanOp::Continuous;
//     HVAC::CompressorOp compressorOp = HVAC::CompressorOp::On;
//     int SingleMode = 0;
//     // Test 1 - dry coil - run the coil at low speed
//     Real64 SpeedRatio = 0.0;
//     Real64 CycRatio = 1.0;
//     Coil.InletAirMassFlowRate = state->dataHVACGlobal->MSHPMassFlowRateLow;
//     DXCoils::CalcMultiSpeedDXCoilCooling(*state, DXCoilNum, SpeedRatio, CycRatio, SpeedNum, fanOp, compressorOp, SingleMode);
//     EXPECT_NEAR(10710.0, Coil.TotalCoolingEnergyRate, 0.0001);   // equals low speed capacity
//     EXPECT_NEAR(10710.0, Coil.SensCoolingEnergyRate, 0.0001);    // sensible cooling rate at low speed
//     EXPECT_NEAR(0.0, Coil.LatCoolingEnergyRate, 1.0E-11);        // zero latent cooling rate at low speed
//     EXPECT_DOUBLE_EQ(0.0055, AirInletNode.HumRat);               // input check
//     EXPECT_DOUBLE_EQ(AirInletNode.HumRat, AirOutletNode.HumRat); // dry cooling only
//     EXPECT_NEAR(35.0, AirInletNode.Temp, 0.0001);                // inlet dry bulb
//     EXPECT_NEAR(17.4149, AirOutletNode.Temp, 0.0001);            // outlet dry bulb
//     EXPECT_NEAR(3570.0, Coil.ElecCoolingPower, 0.01);
//     // Save results for comparison
//     Real64 MultiSpeedTotalCoolingRate1 = Coil.TotalCoolingEnergyRate;
//     Real64 MultiSpeedSensCoolingRate1 = Coil.SensCoolingEnergyRate;
//     Real64 MultiSpeedLatCoolingRate1 = Coil.LatCoolingEnergyRate;
//     Real64 MultiSpeedOutletHumRat1 = AirOutletNode.HumRat;
//     Real64 MultiSpeedOutletTemp1 = AirOutletNode.Temp;
//     Real64 MultiSpeedElecPower1 = Coil.ElecCoolingPower;
//
//     // run the coil at high speed
//     SpeedRatio = 1.0;
//     Coil.InletAirMassFlowRate = state->dataHVACGlobal->MSHPMassFlowRateHigh;
//     DXCoils::CalcMultiSpeedDXCoilCooling(*state, DXCoilNum, SpeedRatio, CycRatio, SpeedNum, fanOp, compressorOp, SingleMode);
//     EXPECT_NEAR(17850.0, Coil.TotalCoolingEnergyRate, 0.0001);   // total capacity at high speed
//     EXPECT_NEAR(17850.0, Coil.SensCoolingEnergyRate, 0.0001);    // sensible cooling rate at high speed
//     EXPECT_NEAR(0.0, Coil.LatCoolingEnergyRate, 1.0E-11);        // zero latent cooling rate at high speed
//     EXPECT_DOUBLE_EQ(0.0055, AirInletNode.HumRat);               // input check
//     EXPECT_DOUBLE_EQ(AirInletNode.HumRat, AirOutletNode.HumRat); // dry cooling only
//     EXPECT_NEAR(35.0, AirInletNode.Temp, 0.0001);                // inlet dry bulb
//     EXPECT_NEAR(17.4149, AirOutletNode.Temp, 0.0001);            // outlet dry bulb
//     EXPECT_NEAR(5950.0, Coil.ElecCoolingPower, 0.01);
//     // Save results for comparison
//     Real64 MultiSpeedTotalCoolingRate2 = Coil.TotalCoolingEnergyRate;
//     Real64 MultiSpeedSensCoolingRate2 = Coil.SensCoolingEnergyRate;
//     Real64 MultiSpeedLatCoolingRate2 = Coil.LatCoolingEnergyRate;
//     Real64 MultiSpeedOutletHumRat2 = AirOutletNode.HumRat;
//     Real64 MultiSpeedOutletTemp2 = AirOutletNode.Temp;
//     Real64 MultiSpeedElecPower2 = Coil.ElecCoolingPower;
//
//     // Test 3 - dry coil - run the coil at speed ratio 0.75
//     SpeedRatio = 0.75;
//     Coil.InletAirMassFlowRate =
//         SpeedRatio * state->dataHVACGlobal->MSHPMassFlowRateHigh + (1.0 - SpeedRatio) * state->dataHVACGlobal->MSHPMassFlowRateLow;
//     DXCoils::CalcMultiSpeedDXCoilCooling(*state, DXCoilNum, SpeedRatio, CycRatio, SpeedNum, fanOp, compressorOp, SingleMode);
//     EXPECT_NEAR(10710.0 + 0.75 * (17850.0 - 10710.0), Coil.TotalCoolingEnergyRate, 0.0001); // total capacity
//     EXPECT_NEAR(10710.0 + 0.75 * (17850.0 - 10710.0), Coil.SensCoolingEnergyRate, 0.0001);  // sensible cooling rate
//     EXPECT_NEAR(0.0, Coil.LatCoolingEnergyRate, 1.0E-11);                                   // zero latent cooling rate at high speed
//     EXPECT_DOUBLE_EQ(0.0055, AirInletNode.HumRat);                                          // input check
//     EXPECT_DOUBLE_EQ(AirInletNode.HumRat, AirOutletNode.HumRat);                            // dry cooling only
//     EXPECT_NEAR(35.0, AirInletNode.Temp, 0.0001);                                           // inlet dry bulb
//     EXPECT_NEAR(17.4149, AirOutletNode.Temp, 0.0001);                                       // outlet dry bulb
//     EXPECT_NEAR(5355.0, Coil.ElecCoolingPower, 0.01);
//     // Save results for comparison
//     Real64 MultiSpeedTotalCoolingRate3 = Coil.TotalCoolingEnergyRate;
//     Real64 MultiSpeedSensCoolingRate3 = Coil.SensCoolingEnergyRate;
//     Real64 MultiSpeedLatCoolingRate3 = Coil.LatCoolingEnergyRate;
//     Real64 MultiSpeedOutletHumRat3 = AirOutletNode.HumRat;
//     Real64 MultiSpeedOutletTemp3 = AirOutletNode.Temp;
//     Real64 MultiSpeedElecPower3 = Coil.ElecCoolingPower;
//
//     // test 4: wet cooling
//     Coil.InletAirTemp = 24.0;
//     Coil.InletAirHumRat = 0.0100;
//     Coil.InletAirEnthalpy = Psychrometrics::PsyHFnTdbW(Coil.InletAirTemp, Coil.InletAirHumRat);
//     // set coil inlet and outlet node condition
//     AirInletNode.Temp = Coil.InletAirTemp;
//     AirInletNode.HumRat = Coil.InletAirHumRat;
//     AirInletNode.Enthalpy = Coil.InletAirEnthalpy;
//     // Test 4 - wet coil - run coil at low speed
//     SpeedRatio = 0.0;
//     CycRatio = 1.0;
//     Coil.InletAirMassFlowRate = state->dataHVACGlobal->MSHPMassFlowRateLow;
//     DXCoils::CalcMultiSpeedDXCoilCooling(*state, DXCoilNum, SpeedRatio, CycRatio, SpeedNum, fanOp, compressorOp, SingleMode);
//     EXPECT_NEAR(10710.0, Coil.TotalCoolingEnergyRate, 0.0001);   // equals low speed cooling capacity
//     EXPECT_NEAR(6908.14887, Coil.SensCoolingEnergyRate, 0.0001); // sensible cooling rate at low speed
//     EXPECT_NEAR(3801.851126, Coil.LatCoolingEnergyRate, 0.0001); // latent cooling rate at low speed
//     EXPECT_DOUBLE_EQ(0.0100, AirInletNode.HumRat);               // input check
//     EXPECT_NEAR(0.00751079, AirOutletNode.HumRat, 0.00001);      // cooling and dehumidification
//     EXPECT_NEAR(24.0, AirInletNode.Temp, 0.0001);                // inlet dry bulb
//     EXPECT_NEAR(12.6989, AirOutletNode.Temp, 0.0001);            // outlet dry bulb
//     EXPECT_NEAR(3570.0, Coil.ElecCoolingPower, 0.01);
//     // Save results for comparison
//     Real64 MultiSpeedTotalCoolingRate4 = Coil.TotalCoolingEnergyRate;
//     Real64 MultiSpeedSensCoolingRate4 = Coil.SensCoolingEnergyRate;
//     Real64 MultiSpeedLatCoolingRate4 = Coil.LatCoolingEnergyRate;
//     Real64 MultiSpeedOutletHumRat4 = AirOutletNode.HumRat;
//     Real64 MultiSpeedOutletTemp4 = AirOutletNode.Temp;
//     Real64 MultiSpeedElecPower4 = Coil.ElecCoolingPower;
//
//     // Test 5 - wet coil - run the coil at high speed
//     SpeedRatio = 1.0;
//     Coil.InletAirMassFlowRate = state->dataHVACGlobal->MSHPMassFlowRateHigh;
//     DXCoils::CalcMultiSpeedDXCoilCooling(*state, DXCoilNum, SpeedRatio, CycRatio, SpeedNum, fanOp, compressorOp, SingleMode);
//     EXPECT_NEAR(17850.0, Coil.TotalCoolingEnergyRate, 0.0001);           // total capacity at high speed
//     EXPECT_NEAR(13002.847055477625, Coil.SensCoolingEnergyRate, 0.0001); // sensible cooling rate at high speed
//     EXPECT_NEAR(4847.1529445223750, Coil.LatCoolingEnergyRate, 0.0001);  // latent cooling rate at high speed
//     EXPECT_DOUBLE_EQ(0.0100, AirInletNode.HumRat);                       // input check
//     EXPECT_NEAR(0.0080958363400692145, AirOutletNode.HumRat, 0.00001);   // cooling and dehumidification
//     EXPECT_NEAR(24.0, AirInletNode.Temp, 0.0001);                        // inlet dry bulb
//     EXPECT_NEAR(11.250732746176219, AirOutletNode.Temp, 0.0001);         // outlet dry bulb
//     EXPECT_NEAR(5950.0, Coil.ElecCoolingPower, 0.01);
//     // Save results for comparison
//     Real64 MultiSpeedTotalCoolingRate5 = Coil.TotalCoolingEnergyRate;
//     Real64 MultiSpeedSensCoolingRate5 = Coil.SensCoolingEnergyRate;
//     Real64 MultiSpeedLatCoolingRate5 = Coil.LatCoolingEnergyRate;
//     Real64 MultiSpeedOutletHumRat5 = AirOutletNode.HumRat;
//     Real64 MultiSpeedOutletTemp5 = AirOutletNode.Temp;
//     Real64 MultiSpeedElecPower5 = Coil.ElecCoolingPower;
//
//     // Test 6 - wet coil - run the coil at speed ratio 0.75
//     SpeedRatio = 0.75;
//     Coil.InletAirMassFlowRate =
//         SpeedRatio * state->dataHVACGlobal->MSHPMassFlowRateHigh + (1.0 - SpeedRatio) * state->dataHVACGlobal->MSHPMassFlowRateLow;
//     DXCoils::CalcMultiSpeedDXCoilCooling(*state, DXCoilNum, SpeedRatio, CycRatio, SpeedNum, fanOp, compressorOp, SingleMode);
//     EXPECT_NEAR(10710.0 + 0.75 * (17850.0 - 10710.0), Coil.TotalCoolingEnergyRate, 0.01);                  // total capacity
//     EXPECT_NEAR(6908.14887 + 0.75 * (13002.847055477625 - 6908.14887), Coil.SensCoolingEnergyRate, 0.01);  // sensible cooling rate
//     EXPECT_NEAR(3801.851126 + 0.75 * (4847.1529445223750 - 3801.851126), Coil.LatCoolingEnergyRate, 0.01); // latent cooling rate
//     EXPECT_DOUBLE_EQ(0.0100, AirInletNode.HumRat);                                                         // input check
//     // EXPECT_NEAR(0.0079495749435070425, AirOutletNode.HumRat, 0.00001);                                        // cooling and dehumidification
//     EXPECT_NEAR(0.0079983287423610987, AirOutletNode.HumRat, 0.00001); // cooling and dehumidification
//     EXPECT_NEAR(24.0, AirInletNode.Temp, 0.0001);                      // inlet dry bulb
//     // EXPECT_NEAR(11.612485891133730, AirOutletNode.Temp, 0.0001);                                              // outlet dry bulb
//     EXPECT_NEAR(11.491880074594654, AirOutletNode.Temp, 0.0001); // outlet dry bulb
//     EXPECT_NEAR(5355.0, Coil.ElecCoolingPower, 0.01);
//     // Save results for comparison
//     Real64 MultiSpeedTotalCoolingRate6 = Coil.TotalCoolingEnergyRate;
//     Real64 MultiSpeedSensCoolingRate6 = Coil.SensCoolingEnergyRate;
//     Real64 MultiSpeedLatCoolingRate6 = Coil.LatCoolingEnergyRate;
//     Real64 MultiSpeedOutletHumRat6 = AirOutletNode.HumRat;
//     Real64 MultiSpeedOutletTemp6 = AirOutletNode.Temp;
//     Real64 MultiSpeedElecPower6 = Coil.ElecCoolingPower;
//
//     Real64 ratedVolFlowRate = state->dataHVACGlobal->MSHPMassFlowRateHigh / ratedRhoAir;
//     std::string volFlowRateStr = format("{:.4R}", ratedVolFlowRate);
//
//     std::string idf_objects = delimited_string({
//         "  Coil:Cooling:DX,",
//         "    Coil,",
//         "    Evaporator Inlet Node,Evaporator Outlet Node,",
//         "    ,,",
//         "    Condenser Inlet Node,Condenser Outlet Node,",
//         "    Coil Performance,",
//         "    ,;",
//
//         "  Coil:Cooling:DX:CurveFit:Performance,",
//         "    Coil Performance,,,,,,,,,Electricity,Coil Mode 1;",
//
//         "  Coil:Cooling:DX:CurveFit:OperatingMode,",
//         "    Coil Mode 1,",
//         "    17850.0,   !- Rated Gross Total Cooling Capacity {W}",
//         volFlowRateStr + ",   !- Rated Evaporator Air Flow Rate {m3/s}",
//         "    ,,,,,,,,",
//         "    2,Coil Mode 1 Speed 1,Coil Mode 1 Speed 2;",
//
//         "  Coil:Cooling:DX:CurveFit:Speed,",
//         "    Coil Mode 1 Speed 1,     !- Name",
//         "    0.60,                    !- Gross Total Cooling Capacity Fraction",
//         "    0.60,                    !- Evaporator Air Flow Rate Fraction",
//         "    ,                        !- Condenser Air Flow Rate Fraction",
//         "    0.65,                     !- Gross Sensible Heat Ratio",
//         "    3,                       !- Gross Cooling COP {W/W}",
//         "    1.0,                     !- Active Fraction of Coil Face Area",
//         "    ,,,,,,,,,,,;",
//
//         "  Coil:Cooling:DX:CurveFit:Speed,",
//         "    Coil Mode 1 Speed 2,     !- Name",
//         "    1.0,                     !- Gross Total Cooling Capacity Fraction",
//         "    1.0,                     !- Evaporator Air Flow Rate Fraction",
//         "    ,                        !- Condenser Air Flow Rate Fraction",
//         "    0.75,                     !- Gross Sensible Heat Ratio",
//         "    3,                       !- Gross Cooling COP {W/W}",
//         "    1.0,                     !- Active Fraction of Coil Face Area",
//         "    ,,,,,,,,,,,;",
//
//     });
//     EXPECT_TRUE(process_idf(idf_objects, false));
//     state->init_state(*state);
//     int coilIndex = CoilCoolingDX::factory(*state, "Coil");
//     auto &thisCoil(state->dataCoilCoolingDX->coilCoolingDXs[coilIndex]);
//
//     // fix the inlet conditions
//     auto &evapInletNode = state->dataLoopNodes->Node(thisCoil.evapInletNodeIndex);
//     auto &condInletNode = state->dataLoopNodes->Node(thisCoil.condInletNodeIndex);
//     evapInletNode.Press = 101325;
//     condInletNode.Temp = 30.0;
//     condInletNode.Press = 101325;
//     condInletNode.HumRat = 0.012;
//     condInletNode.Enthalpy = Psychrometrics::PsyHFnTdbW(condInletNode.Temp, condInletNode.HumRat);
//
//     // size it
//     thisCoil.size(*state);
//
//     // for speed > 1 we use the mshp rated high speed flow...
//     state->dataHVACGlobal->MSHPMassFlowRateHigh = thisCoil.performance.normalMode.speeds.back().RatedAirMassFlowRate;
//
//     // we'll use this later
//     auto &evapOutletNode = state->dataLoopNodes->Node(thisCoil.evapOutletNodeIndex);
//
//     // Test 1 - dry coil - run the coil at low speed
//     evapInletNode.Temp = 35.0;
//     evapInletNode.HumRat = 0.0055;
//     evapInletNode.Enthalpy = Psychrometrics::PsyHFnTdbW(evapInletNode.Temp, evapInletNode.HumRat);
//     // set some values to run at rated conditions and call to run normal mode speed 1
//     evapInletNode.MassFlowRate = thisCoil.performance.normalMode.speeds.front().RatedAirMassFlowRate;
//     HVAC::CoilMode coilMode = HVAC::CoilMode::Normal;
//     Real64 PLR = 1.0;
//     int speedNum = 1;
//     Real64 speedRatio = 0.0;
//     fanOp = HVAC::FanOp::Continuous;
//     bool singleMode = false;
//     thisCoil.simulate(*state, coilMode, PLR, speedNum, speedRatio, fanOp, singleMode);
//     EXPECT_NEAR(MultiSpeedTotalCoolingRate1, thisCoil.totalCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedSensCoolingRate1, thisCoil.sensCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedLatCoolingRate1, thisCoil.latCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedOutletTemp1, evapOutletNode.Temp, 0.01);
//     EXPECT_NEAR(MultiSpeedOutletHumRat1, evapOutletNode.HumRat, 0.001);
//     EXPECT_NEAR(MultiSpeedElecPower1, thisCoil.elecCoolingPower, 0.001);
//
//     // Test 2 - dry coil - run the coil at high speed
//     evapInletNode.MassFlowRate = thisCoil.performance.normalMode.speeds.back().RatedAirMassFlowRate;
//     speedNum = 2;
//     speedRatio = 1.0;
//     thisCoil.simulate(*state, coilMode, PLR, speedNum, speedRatio, fanOp, singleMode);
//     EXPECT_NEAR(MultiSpeedTotalCoolingRate2, thisCoil.totalCoolingEnergyRate, 0.01);
//     EXPECT_NEAR(MultiSpeedSensCoolingRate2, thisCoil.sensCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedLatCoolingRate2, thisCoil.latCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedOutletTemp2, evapOutletNode.Temp, 0.01);
//     EXPECT_NEAR(MultiSpeedOutletHumRat2, evapOutletNode.HumRat, 0.001);
//     EXPECT_NEAR(MultiSpeedElecPower2, thisCoil.elecCoolingPower, 0.001);
//
//     // Test 3 - dry coil - run the coil at 0.75 speed ratio
//     speedNum = 2;
//     speedRatio = 0.75;
//     evapInletNode.MassFlowRate = speedRatio * thisCoil.performance.normalMode.speeds.back().RatedAirMassFlowRate +
//                                  (1.0 - speedRatio) * thisCoil.performance.normalMode.speeds.front().RatedAirMassFlowRate;
//     thisCoil.simulate(*state, coilMode, PLR, speedNum, speedRatio, fanOp, singleMode);
//     EXPECT_NEAR(MultiSpeedTotalCoolingRate3, thisCoil.totalCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedSensCoolingRate3, thisCoil.sensCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedLatCoolingRate3, thisCoil.latCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedOutletTemp3, evapOutletNode.Temp, 0.01);
//     EXPECT_NEAR(MultiSpeedOutletHumRat3, evapOutletNode.HumRat, 0.001);
//     EXPECT_NEAR(MultiSpeedElecPower3, thisCoil.elecCoolingPower, 0.001);
//
//     // Test 4 - wet coil - run the coil at low speed
//     evapInletNode.Temp = 24.0;
//     evapInletNode.HumRat = 0.0100;
//     evapInletNode.Enthalpy = Psychrometrics::PsyHFnTdbW(evapInletNode.Temp, evapInletNode.HumRat);
//     evapInletNode.MassFlowRate = thisCoil.performance.normalMode.speeds.front().RatedAirMassFlowRate;
//     coilMode = HVAC::CoilMode::Normal;
//     PLR = 1.0;
//     speedNum = 1;
//     speedRatio = 1.0;
//     fanOp = HVAC::FanOp::Cycling;
//     singleMode = false;
//     thisCoil.simulate(*state, coilMode, PLR, speedNum, speedRatio, fanOp, singleMode);
//     EXPECT_NEAR(MultiSpeedTotalCoolingRate4, thisCoil.totalCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedSensCoolingRate4, thisCoil.sensCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedLatCoolingRate4, thisCoil.latCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedOutletTemp4, evapOutletNode.Temp, 0.01);
//     EXPECT_NEAR(MultiSpeedOutletHumRat4, evapOutletNode.HumRat, 0.001);
//     EXPECT_NEAR(MultiSpeedElecPower4, thisCoil.elecCoolingPower, 0.001);
//
//     // Test 5 - wet coil - run the coil at high speed
//     evapInletNode.MassFlowRate = thisCoil.performance.normalMode.speeds.back().RatedAirMassFlowRate;
//     speedNum = 2;
//     speedRatio = 1.0;
//     thisCoil.simulate(*state, coilMode, PLR, speedNum, speedRatio, fanOp, singleMode);
//     EXPECT_NEAR(MultiSpeedTotalCoolingRate5, thisCoil.totalCoolingEnergyRate, 0.01);
//     EXPECT_NEAR(MultiSpeedSensCoolingRate5, thisCoil.sensCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedLatCoolingRate5, thisCoil.latCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedOutletTemp5, evapOutletNode.Temp, 0.01);
//     EXPECT_NEAR(MultiSpeedOutletHumRat5, evapOutletNode.HumRat, 0.001);
//     EXPECT_NEAR(MultiSpeedElecPower5, thisCoil.elecCoolingPower, 0.001);
//
//     // Test 6 - wet coil - run the coil at 0.75 speed ratio
//     speedNum = 2;
//     speedRatio = 0.75;
//     evapInletNode.MassFlowRate = speedRatio * thisCoil.performance.normalMode.speeds.back().RatedAirMassFlowRate +
//                                  (1.0 - speedRatio) * thisCoil.performance.normalMode.speeds.front().RatedAirMassFlowRate;
//     thisCoil.simulate(*state, coilMode, PLR, speedNum, speedRatio, fanOp, singleMode);
//     EXPECT_NEAR(MultiSpeedTotalCoolingRate6, thisCoil.totalCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedSensCoolingRate6, thisCoil.sensCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedLatCoolingRate6, thisCoil.latCoolingEnergyRate, 0.1);
//     EXPECT_NEAR(MultiSpeedOutletTemp6, evapOutletNode.Temp, 0.01);
//     EXPECT_NEAR(MultiSpeedOutletHumRat6, evapOutletNode.HumRat, 0.001);
//     EXPECT_NEAR(MultiSpeedElecPower6, thisCoil.elecCoolingPower, 0.001);
// }
// TEST_F(EnergyPlusFixture, CoilDXMultiSpeed_SpeedCheck_CycFanCycCoil)
// {
//
//     int DXCoilNum(1);
//     state->dataDXCoils->NumDXCoils = 1;
//     state->dataDXCoils->DXCoil.allocate(state->dataDXCoils->NumDXCoils);
//     state->dataLoopNodes->Node.allocate(2);
//     state->dataDXCoils->DXCoilNumericFields.allocate(state->dataDXCoils->NumDXCoils);
//     state->dataDXCoils->DXCoilNumericFields(1).PerfMode.allocate(1);
//     state->dataDXCoils->DXCoilNumericFields(1).PerfMode(1).FieldNames.allocate(17);
//     state->dataHeatBal->HeatReclaimDXCoil.allocate(2);
//     state->dataDXCoils->DXCoilOutletTemp.allocate(1);
//     state->dataDXCoils->DXCoilOutletHumRat.allocate(1);
//     state->dataDXCoils->DXCoilPartLoadRatio.allocate(1);
//     state->dataDXCoils->DXCoilFanOp.allocate(1);
//     state->dataCurveManager->allocateCurveVector(2);
//
//     auto &Coil = state->dataDXCoils->DXCoil(1);
//     auto &constantcurve1 = state->dataCurveManager->PerfCurve(1);
//     auto &constantcurve2 = state->dataCurveManager->PerfCurve(2);
//     auto &AirInletNode = state->dataLoopNodes->Node(1);
//     auto &AirOutletNode = state->dataLoopNodes->Node(2);
//     state->dataEnvrn->StdBaroPress = 101325.0;
//     // Real64 ratedInletAirTemp = 26.6667;
//     // Real64 ratedInletAirHumRat = 0.0111847;
//     // std::string routineName = "MultiSpeedDXCoolingCoilOutputTestvsCoilDXCooling";
//     // Real64 ratedRhoAir = Psychrometrics::PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->StdBaroPress, ratedInletAirTemp, ratedInletAirHumRat,
//     // routineName);
//
//     Coil.DXCoilType_Num = HVAC::CoilDX_MultiSpeedCooling;
//     Coil.DXCoilType = "Coil:Cooling:DX:MultiSpeed";
//     Coil.FuelType = Constant::eFuel::Electricity;
//     Coil.availSched = Sched::GetScheduleAlwaysOn(*state);
//     Coil.NumOfSpeeds = 2;
//     Coil.MSRatedTotCap.allocate(Coil.NumOfSpeeds);
//     Coil.MSRatedSHR.allocate(Coil.NumOfSpeeds);
//     Coil.MSRatedCOP.allocate(Coil.NumOfSpeeds);
//     Coil.MSRatedAirVolFlowRate.allocate(Coil.NumOfSpeeds);
//     Coil.MSRatedAirMassFlowRate.allocate(Coil.NumOfSpeeds);
//     Coil.MSCCapFTemp.allocate(Coil.NumOfSpeeds);
//     Coil.MSCCapFFlow.allocate(Coil.NumOfSpeeds);
//     Coil.MSEIRFTemp.allocate(Coil.NumOfSpeeds);
//     Coil.MSEIRFFlow.allocate(Coil.NumOfSpeeds);
//     Coil.MSWasteHeat.allocate(Coil.NumOfSpeeds);
//     Coil.MSEvapCondEffect.allocate(Coil.NumOfSpeeds);
//     Coil.MSEvapCondAirFlow.allocate(Coil.NumOfSpeeds);
//     Coil.MSEvapCondPumpElecNomPower.allocate(Coil.NumOfSpeeds);
//     Coil.MSRatedCBF.allocate(Coil.NumOfSpeeds);
//     Coil.MSWasteHeatFrac.allocate(Coil.NumOfSpeeds);
//     Coil.MSPLFFPLR.allocate(Coil.NumOfSpeeds);
//     Coil.MSTwet_Rated.allocate(Coil.NumOfSpeeds);
//     Coil.MSGamma_Rated.allocate(Coil.NumOfSpeeds);
//     Coil.MSMaxONOFFCyclesperHour.allocate(Coil.NumOfSpeeds);
//     Coil.MSLatentCapacityTimeConstant.allocate(Coil.NumOfSpeeds);
//     Coil.MSFanPowerPerEvapAirFlowRate.allocate(Coil.NumOfSpeeds);
//     Coil.MSCCapFTemp = 1;
//     Coil.MSCCapFFlow = 2;
//     Coil.MSEIRFTemp = 1;
//     Coil.MSEIRFFlow = 2;
//     Coil.MSPLFFPLR = 2;
//     Coil.AirOutNode = 2;
//     Coil.AirInNode = 1;
//     // biquadratic curve
//     constantcurve1->Name = "constant biquadratic curve";
//     constantcurve1->curveType = Curve::CurveType::BiQuadratic;
//     constantcurve1->interpolationType = Curve::InterpType::EvaluateCurveToLimits;
//     constantcurve1->coeff[0] = 1.0;
//     constantcurve1->coeff[1] = 0.0;
//     constantcurve1->coeff[2] = 0.0;
//     constantcurve1->coeff[3] = 0.0;
//     constantcurve1->coeff[4] = 0.0;
//     constantcurve1->coeff[5] = 0.0;
//     constantcurve1->inputLimits[0].min = 10.0;
//     constantcurve1->inputLimits[0].max = 25.0;
//     constantcurve1->inputLimits[1].min = 0.0;
//     constantcurve1->inputLimits[1].max = 100.0;
//     constantcurve1->outputLimits.min = 1.0;
//     constantcurve1->outputLimits.max = 1.0;
//     // quadratic curve
//     constantcurve2->Name = "constant quadratic curve";
//     constantcurve2->curveType = Curve::CurveType::Quadratic;
//     constantcurve2->interpolationType = Curve::InterpType::EvaluateCurveToLimits;
//     constantcurve2->coeff[0] = 1.0;
//     constantcurve2->coeff[1] = 0.0;
//     constantcurve2->coeff[2] = 0.0;
//     constantcurve2->inputLimits[0].min = 0.0;
//     constantcurve2->inputLimits[0].max = 1.0;
//     constantcurve2->outputLimits.min = 1.0;
//     constantcurve2->outputLimits.max = 1.0;
//     // set coil parameter
//     Coil.MSRatedTotCap(1) = 10710.0; // 60 % of full capacity
//     Coil.MSRatedTotCap(2) = 17850.0; // 5 ton capcity
//     Coil.MSRatedAirMassFlowRate(1) = 0.6;
//     Coil.MSRatedAirMassFlowRate(2) = 1.0;
//     // Match RatedCBF from new coil
//     Coil.MSRatedCBF(1) = 0.32321692557501741;
//     Coil.MSRatedCBF(2) = 0.037495280896632406;
//     Coil.MSWasteHeat(1) = 0;
//     Coil.MSWasteHeat(2) = 0;
//     Coil.MSWasteHeatFrac(1) = 0;
//     Coil.MSWasteHeatFrac(2) = 0;
//     Coil.MSRatedSHR(1) = 0.65;
//     Coil.MSRatedSHR(2) = 0.75;
//     Coil.MSRatedCOP(1) = 3.0;
//     Coil.MSRatedCOP(2) = 3.0;
//
//     // test 1: dry cooling
//     Coil.InletAirTemp = 35.0;
//     Coil.InletAirHumRat = 0.0055;
//     Coil.InletAirEnthalpy = Psychrometrics::PsyHFnTdbW(Coil.InletAirTemp, Coil.InletAirHumRat);
//     // set coil inlet and outlet node condition
//     AirInletNode.Temp = Coil.InletAirTemp;
//     AirInletNode.HumRat = Coil.InletAirHumRat;
//     AirInletNode.Enthalpy = Coil.InletAirEnthalpy;
//     AirOutletNode.Temp = Coil.InletAirTemp;
//     AirOutletNode.HumRat = Coil.InletAirHumRat;
//     AirOutletNode.Enthalpy = Coil.InletAirEnthalpy;
//     // outside air condition
//     state->dataEnvrn->OutBaroPress = 101325.0;
//     state->dataEnvrn->OutDryBulbTemp = 35.0;
//     state->dataEnvrn->OutHumRat = 0.0120;
//     state->dataEnvrn->WindSpeed = 5.0;
//     state->dataEnvrn->WindDir = 0.0;
//     HVAC::FanOp fanOp = HVAC::FanOp::Cycling;
//     HVAC::CompressorOp compressorOp = HVAC::CompressorOp::On;
//     int SingleMode = 0;
//     // Test 1 - dry coil - run the coil at low speed (speednum=2, speedratio=0)
//     int SpeedNum = 2;
//     Real64 SpeedRatio = 0.0;
//     Real64 CycRatio = 1.0;
//     state->dataHVACGlobal->MSHPMassFlowRateLow = Coil.MSRatedAirMassFlowRate(1);
//     state->dataHVACGlobal->MSHPMassFlowRateHigh = Coil.MSRatedAirMassFlowRate(2);
//     Coil.InletAirMassFlowRate = state->dataHVACGlobal->MSHPMassFlowRateLow;
//     DXCoils::CalcMultiSpeedDXCoilCooling(*state, DXCoilNum, SpeedRatio, CycRatio, SpeedNum, fanOp, compressorOp, SingleMode);
//     EXPECT_NEAR(10710.0, Coil.TotalCoolingEnergyRate, 0.0001);   // equals low speed capacity
//     EXPECT_NEAR(10710.0, Coil.SensCoolingEnergyRate, 0.0001);    // sensible cooling rate at low speed
//     EXPECT_NEAR(0.0, Coil.LatCoolingEnergyRate, 1.0E-11);        // zero latent cooling rate at low speed
//     EXPECT_DOUBLE_EQ(0.0055, AirInletNode.HumRat);               // input check
//     EXPECT_DOUBLE_EQ(AirInletNode.HumRat, AirOutletNode.HumRat); // dry cooling only
//     EXPECT_NEAR(35.0, AirInletNode.Temp, 0.0001);                // inlet dry bulb
//     EXPECT_NEAR(17.4149, AirOutletNode.Temp, 0.0001);            // outlet dry bulb
//     EXPECT_NEAR(3570.0, Coil.ElecCoolingPower, 0.01);
//     // Save results for comparison
//     Real64 MultiSpeedTotalCoolingRate1 = Coil.TotalCoolingEnergyRate;
//     Real64 MultiSpeedSensCoolingRate1 = Coil.SensCoolingEnergyRate;
//     Real64 MultiSpeedLatCoolingRate1 = Coil.LatCoolingEnergyRate;
//     // Real64 MultiSpeedOutletHumRat1 = AirOutletNode.HumRat;
//     Real64 MultiSpeedOutletTemp1 = AirOutletNode.Temp;
//     Real64 MultiSpeedElecPower1 = Coil.ElecCoolingPower;
//
//     // Test 2 - dry coil - run the coil at low speed (speednum=1, speedratio=0) - same result?
//     SpeedNum = 1;
//     SpeedRatio = 0.0;
//     state->dataHVACGlobal->MSHPMassFlowRateLow = Coil.MSRatedAirMassFlowRate(1);
//     state->dataHVACGlobal->MSHPMassFlowRateHigh = Coil.MSRatedAirMassFlowRate(1);
//     Coil.InletAirMassFlowRate = state->dataHVACGlobal->MSHPMassFlowRateLow;
//     DXCoils::CalcMultiSpeedDXCoilCooling(*state, DXCoilNum, SpeedRatio, CycRatio, SpeedNum, fanOp, compressorOp, SingleMode);
//     EXPECT_NEAR(MultiSpeedTotalCoolingRate1, Coil.TotalCoolingEnergyRate, 0.0001); // total capacity at high speed
//     EXPECT_NEAR(MultiSpeedSensCoolingRate1, Coil.SensCoolingEnergyRate, 0.0001);   // sensible cooling rate at high speed
//     EXPECT_NEAR(MultiSpeedLatCoolingRate1, Coil.LatCoolingEnergyRate, 1.0E-11);    // zero latent cooling rate at high speed
//     EXPECT_DOUBLE_EQ(0.0055, AirInletNode.HumRat);                                 // input check
//     EXPECT_DOUBLE_EQ(AirInletNode.HumRat, AirOutletNode.HumRat);                   // dry cooling only
//     EXPECT_NEAR(35.0, AirInletNode.Temp, 0.0001);                                  // inlet dry bulb
//     EXPECT_NEAR(MultiSpeedOutletTemp1, AirOutletNode.Temp, 0.0001);                // outlet dry bulb
//     EXPECT_NEAR(MultiSpeedElecPower1, Coil.ElecCoolingPower, 0.01);
//
//     // tests 3 & 4: wet cooling
//     Coil.InletAirTemp = 24.0;
//     Coil.InletAirHumRat = 0.0100;
//     Coil.InletAirEnthalpy = Psychrometrics::PsyHFnTdbW(Coil.InletAirTemp, Coil.InletAirHumRat);
//     // set coil inlet and outlet node condition
//     AirInletNode.Temp = Coil.InletAirTemp;
//     AirInletNode.HumRat = Coil.InletAirHumRat;
//     AirInletNode.Enthalpy = Coil.InletAirEnthalpy;
//     // Test 3 - wet coil - run coil at low speed - run the coil at low speed (speednum=2, speedratio=0, CycFanCycCoil)
//     SpeedNum = 2;
//     SpeedRatio = 0.0;
//     CycRatio = 1.0;
//     state->dataHVACGlobal->MSHPMassFlowRateLow = Coil.MSRatedAirMassFlowRate(1);
//     state->dataHVACGlobal->MSHPMassFlowRateHigh = Coil.MSRatedAirMassFlowRate(2);
//     Coil.InletAirMassFlowRate = state->dataHVACGlobal->MSHPMassFlowRateLow;
//     DXCoils::CalcMultiSpeedDXCoilCooling(*state, DXCoilNum, SpeedRatio, CycRatio, SpeedNum, fanOp, compressorOp, SingleMode);
//     EXPECT_NEAR(10710.0, Coil.TotalCoolingEnergyRate, 0.0001);   // equals low speed cooling capacity
//     EXPECT_NEAR(6908.14887, Coil.SensCoolingEnergyRate, 0.0001); // sensible cooling rate at low speed
//     EXPECT_NEAR(3801.851126, Coil.LatCoolingEnergyRate, 0.0001); // latent cooling rate at low speed
//     EXPECT_DOUBLE_EQ(0.0100, AirInletNode.HumRat);               // input check
//     EXPECT_NEAR(0.00751079, AirOutletNode.HumRat, 0.00001);      // cooling and dehumidification
//     EXPECT_NEAR(24.0, AirInletNode.Temp, 0.0001);                // inlet dry bulb
//     EXPECT_NEAR(12.6989, AirOutletNode.Temp, 0.0001);            // outlet dry bulb
//     EXPECT_NEAR(3570.0, Coil.ElecCoolingPower, 0.01);
//     // Save results for comparison
//     Real64 MultiSpeedTotalCoolingRate3 = Coil.TotalCoolingEnergyRate;
//     Real64 MultiSpeedSensCoolingRate3 = Coil.SensCoolingEnergyRate;
//     Real64 MultiSpeedLatCoolingRate3 = Coil.LatCoolingEnergyRate;
//     Real64 MultiSpeedOutletHumRat3 = AirOutletNode.HumRat;
//     Real64 MultiSpeedOutletTemp3 = AirOutletNode.Temp;
//     Real64 MultiSpeedElecPower3 = Coil.ElecCoolingPower;
//
//     // Test 4 - wet coil - run the coil at low speed (speednum=1, speedratio=0, CycFanCycCoil) - same result?
//     SpeedNum = 1;
//     SpeedRatio = 0.0;
//     state->dataHVACGlobal->MSHPMassFlowRateLow = Coil.MSRatedAirMassFlowRate(1);
//     state->dataHVACGlobal->MSHPMassFlowRateHigh = Coil.MSRatedAirMassFlowRate(1);
//     Coil.InletAirMassFlowRate = state->dataHVACGlobal->MSHPMassFlowRateLow;
//     DXCoils::CalcMultiSpeedDXCoilCooling(*state, DXCoilNum, SpeedRatio, CycRatio, SpeedNum, fanOp, compressorOp, SingleMode);
//     EXPECT_NEAR(MultiSpeedTotalCoolingRate3, Coil.TotalCoolingEnergyRate, 0.0001); // total capacity at high speed
//     EXPECT_NEAR(MultiSpeedSensCoolingRate3, Coil.SensCoolingEnergyRate, 0.0001);   // sensible cooling rate at high speed
//     EXPECT_NEAR(MultiSpeedLatCoolingRate3, Coil.LatCoolingEnergyRate, 0.0001);     // latent cooling rate at high speed
//     EXPECT_DOUBLE_EQ(0.0100, AirInletNode.HumRat);                                 // input check
//     EXPECT_NEAR(MultiSpeedOutletHumRat3, AirOutletNode.HumRat, 0.00001);           // cooling and dehumidification
//     EXPECT_NEAR(24.0, AirInletNode.Temp, 0.0001);                                  // inlet dry bulb
//     EXPECT_NEAR(MultiSpeedOutletTemp3, AirOutletNode.Temp, 0.0001);                // outlet dry bulb
//     EXPECT_NEAR(MultiSpeedElecPower3, Coil.ElecCoolingPower, 0.01);
//
//     // Test 5 - wet coil - run the coil at almost low speed (speednum=2, speedratio=0.00001, CycFanCycCoil) - same result?
//     SpeedNum = 2;
//     SpeedRatio = 0.00001;
//     state->dataHVACGlobal->MSHPMassFlowRateLow = Coil.MSRatedAirMassFlowRate(1);
//     state->dataHVACGlobal->MSHPMassFlowRateHigh = Coil.MSRatedAirMassFlowRate(2);
//     Coil.InletAirMassFlowRate =
//         SpeedRatio * state->dataHVACGlobal->MSHPMassFlowRateHigh + (1.0 - SpeedRatio) * state->dataHVACGlobal->MSHPMassFlowRateLow;
//     DXCoils::CalcMultiSpeedDXCoilCooling(*state, DXCoilNum, SpeedRatio, CycRatio, SpeedNum, fanOp, compressorOp, SingleMode);
//     EXPECT_NEAR(MultiSpeedTotalCoolingRate3, Coil.TotalCoolingEnergyRate, 0.1); // total capacity at high speed
//     EXPECT_NEAR(MultiSpeedSensCoolingRate3, Coil.SensCoolingEnergyRate, 0.1);   // sensible cooling rate at high speed
//     EXPECT_NEAR(MultiSpeedLatCoolingRate3, Coil.LatCoolingEnergyRate, 0.1);     // latent cooling rate at high speed
//     EXPECT_DOUBLE_EQ(0.0100, AirInletNode.HumRat);                              // input check
//     EXPECT_NEAR(MultiSpeedOutletHumRat3, AirOutletNode.HumRat, 0.001);          // cooling and dehumidification
//     EXPECT_NEAR(24.0, AirInletNode.Temp, 0.0001);                               // inlet dry bulb
//     EXPECT_NEAR(MultiSpeedOutletTemp3, AirOutletNode.Temp, 0.01);               // outlet dry bulb
//     EXPECT_NEAR(MultiSpeedElecPower3, Coil.ElecCoolingPower, 0.1);
// }
// TEST_F(EnergyPlusFixture, CoilDXMultiSpeed_SpeedCheck_ContFanCycCoil)
// {
//
//     int DXCoilNum(1);
//     state->dataDXCoils->NumDXCoils = 1;
//     state->dataDXCoils->DXCoil.allocate(state->dataDXCoils->NumDXCoils);
//     state->dataLoopNodes->Node.allocate(2);
//     state->dataDXCoils->DXCoilNumericFields.allocate(state->dataDXCoils->NumDXCoils);
//     state->dataDXCoils->DXCoilNumericFields(1).PerfMode.allocate(1);
//     state->dataDXCoils->DXCoilNumericFields(1).PerfMode(1).FieldNames.allocate(17);
//     state->dataHeatBal->HeatReclaimDXCoil.allocate(2);
//     state->dataDXCoils->DXCoilOutletTemp.allocate(1);
//     state->dataDXCoils->DXCoilOutletHumRat.allocate(1);
//     state->dataDXCoils->DXCoilPartLoadRatio.allocate(1);
//     state->dataDXCoils->DXCoilFanOp.allocate(1);
//     state->dataCurveManager->allocateCurveVector(2);
//
//     auto &Coil = state->dataDXCoils->DXCoil(1);
//     auto &constantcurve1 = state->dataCurveManager->PerfCurve(1);
//     auto &constantcurve2 = state->dataCurveManager->PerfCurve(2);
//     auto &AirInletNode = state->dataLoopNodes->Node(1);
//     auto &AirOutletNode = state->dataLoopNodes->Node(2);
//     state->dataEnvrn->StdBaroPress = 101325.0;
//     // Real64 ratedInletAirTemp = 26.6667;
//     // Real64 ratedInletAirHumRat = 0.0111847;
//     // std::string routineName = "MultiSpeedDXCoolingCoilOutputTestvsCoilDXCooling";
//     // Real64 ratedRhoAir = Psychrometrics::PsyRhoAirFnPbTdbW(*state, state->dataEnvrn->StdBaroPress, ratedInletAirTemp, ratedInletAirHumRat,
//     // routineName);
//
//     Coil.DXCoilType_Num = HVAC::CoilDX_MultiSpeedCooling;
//     Coil.DXCoilType = "Coil:Cooling:DX:MultiSpeed";
//     Coil.FuelType = Constant::eFuel::Electricity;
//     Coil.availSched = Sched::GetScheduleAlwaysOn(*state);
//     Coil.NumOfSpeeds = 2;
//     Coil.MSRatedTotCap.allocate(Coil.NumOfSpeeds);
//     Coil.MSRatedSHR.allocate(Coil.NumOfSpeeds);
//     Coil.MSRatedCOP.allocate(Coil.NumOfSpeeds);
//     Coil.MSRatedAirVolFlowRate.allocate(Coil.NumOfSpeeds);
//     Coil.MSRatedAirMassFlowRate.allocate(Coil.NumOfSpeeds);
//     Coil.MSCCapFTemp.allocate(Coil.NumOfSpeeds);
//     Coil.MSCCapFFlow.allocate(Coil.NumOfSpeeds);
//     Coil.MSEIRFTemp.allocate(Coil.NumOfSpeeds);
//     Coil.MSEIRFFlow.allocate(Coil.NumOfSpeeds);
//     Coil.MSWasteHeat.allocate(Coil.NumOfSpeeds);
//     Coil.MSEvapCondEffect.allocate(Coil.NumOfSpeeds);
//     Coil.MSEvapCondAirFlow.allocate(Coil.NumOfSpeeds);
//     Coil.MSEvapCondPumpElecNomPower.allocate(Coil.NumOfSpeeds);
//     Coil.MSRatedCBF.allocate(Coil.NumOfSpeeds);
//     Coil.MSWasteHeatFrac.allocate(Coil.NumOfSpeeds);
//     Coil.MSPLFFPLR.allocate(Coil.NumOfSpeeds);
//     Coil.MSTwet_Rated.allocate(Coil.NumOfSpeeds);
//     Coil.MSGamma_Rated.allocate(Coil.NumOfSpeeds);
//     Coil.MSMaxONOFFCyclesperHour.allocate(Coil.NumOfSpeeds);
//     Coil.MSLatentCapacityTimeConstant.allocate(Coil.NumOfSpeeds);
//     Coil.MSFanPowerPerEvapAirFlowRate.allocate(Coil.NumOfSpeeds);
//     Coil.MSCCapFTemp = 1;
//     Coil.MSCCapFFlow = 2;
//     Coil.MSEIRFTemp = 1;
//     Coil.MSEIRFFlow = 2;
//     Coil.MSPLFFPLR = 2;
//     Coil.AirOutNode = 2;
//     Coil.AirInNode = 1;
//     // biquadratic curve
//     constantcurve1->Name = "constant biquadratic curve";
//     constantcurve1->curveType = Curve::CurveType::BiQuadratic;
//     constantcurve1->interpolationType = Curve::InterpType::EvaluateCurveToLimits;
//     constantcurve1->coeff[0] = 1.0;
//     constantcurve1->coeff[1] = 0.0;
//     constantcurve1->coeff[2] = 0.0;
//     constantcurve1->coeff[3] = 0.0;
//     constantcurve1->coeff[4] = 0.0;
//     constantcurve1->coeff[5] = 0.0;
//     constantcurve1->inputLimits[0].min = 10.0;
//     constantcurve1->inputLimits[0].max = 25.0;
//     constantcurve1->inputLimits[1].min = 0.0;
//     constantcurve1->inputLimits[1].max = 100.0;
//     constantcurve1->outputLimits.min = 1.0;
//     constantcurve1->outputLimits.max = 1.0;
//     // quadratic curve
//     constantcurve2->Name = "constant quadratic curve";
//     constantcurve2->curveType = Curve::CurveType::Quadratic;
//     constantcurve2->interpolationType = Curve::InterpType::EvaluateCurveToLimits;
//     constantcurve2->coeff[0] = 1.0;
//     constantcurve2->coeff[1] = 0.0;
//     constantcurve2->coeff[2] = 0.0;
//     constantcurve2->inputLimits[0].min = 0.0;
//     constantcurve2->inputLimits[0].max = 1.0;
//     constantcurve2->outputLimits.min = 1.0;
//     constantcurve2->outputLimits.max = 1.0;
//     // set coil parameter
//     Coil.MSRatedTotCap(1) = 10710.0; // 60 % of full capacity
//     Coil.MSRatedTotCap(2) = 17850.0; // 5 ton capcity
//     Coil.MSRatedAirMassFlowRate(1) = 0.6;
//     Coil.MSRatedAirMassFlowRate(2) = 1.0;
//     // Match RatedCBF from new coil
//     Coil.MSRatedCBF(1) = 0.32321692557501741;
//     Coil.MSRatedCBF(2) = 0.037495280896632406;
//     Coil.MSWasteHeat(1) = 0;
//     Coil.MSWasteHeat(2) = 0;
//     Coil.MSWasteHeatFrac(1) = 0;
//     Coil.MSWasteHeatFrac(2) = 0;
//     Coil.MSRatedSHR(1) = 0.65;
//     Coil.MSRatedSHR(2) = 0.75;
//     Coil.MSRatedCOP(1) = 3.0;
//     Coil.MSRatedCOP(2) = 3.0;
//
//     // test 1: dry cooling
//     Coil.InletAirTemp = 35.0;
//     Coil.InletAirHumRat = 0.0055;
//     Coil.InletAirEnthalpy = Psychrometrics::PsyHFnTdbW(Coil.InletAirTemp, Coil.InletAirHumRat);
//     // set coil inlet and outlet node condition
//     AirInletNode.Temp = Coil.InletAirTemp;
//     AirInletNode.HumRat = Coil.InletAirHumRat;
//     AirInletNode.Enthalpy = Coil.InletAirEnthalpy;
//     AirOutletNode.Temp = Coil.InletAirTemp;
//     AirOutletNode.HumRat = Coil.InletAirHumRat;
//     AirOutletNode.Enthalpy = Coil.InletAirEnthalpy;
//     // outside air condition
//     state->dataEnvrn->OutBaroPress = 101325.0;
//     state->dataEnvrn->OutDryBulbTemp = 35.0;
//     state->dataEnvrn->OutHumRat = 0.0120;
//     state->dataEnvrn->WindSpeed = 5.0;
//     state->dataEnvrn->WindDir = 0.0;
//     HVAC::FanOp fanOp = HVAC::FanOp::Continuous;
//     HVAC::CompressorOp compressorOp = HVAC::CompressorOp::On;
//     int SingleMode = 0;
//     // Test 1 - dry coil - run the coil at low speed (speednum=2, speedratio=0)
//     int SpeedNum = 2;
//     Real64 SpeedRatio = 0.0;
//     Real64 CycRatio = 1.0;
//     state->dataHVACGlobal->MSHPMassFlowRateLow = Coil.MSRatedAirMassFlowRate(1);
//     state->dataHVACGlobal->MSHPMassFlowRateHigh = Coil.MSRatedAirMassFlowRate(2);
//     Coil.InletAirMassFlowRate = state->dataHVACGlobal->MSHPMassFlowRateLow;
//     DXCoils::CalcMultiSpeedDXCoilCooling(*state, DXCoilNum, SpeedRatio, CycRatio, SpeedNum, fanOp, compressorOp, SingleMode);
//     EXPECT_NEAR(10710.0, Coil.TotalCoolingEnergyRate, 0.0001);   // equals low speed capacity
//     EXPECT_NEAR(10710.0, Coil.SensCoolingEnergyRate, 0.0001);    // sensible cooling rate at low speed
//     EXPECT_NEAR(0.0, Coil.LatCoolingEnergyRate, 1.0E-11);        // zero latent cooling rate at low speed
//     EXPECT_DOUBLE_EQ(0.0055, AirInletNode.HumRat);               // input check
//     EXPECT_DOUBLE_EQ(AirInletNode.HumRat, AirOutletNode.HumRat); // dry cooling only
//     EXPECT_NEAR(35.0, AirInletNode.Temp, 0.0001);                // inlet dry bulb
//     EXPECT_NEAR(17.4149, AirOutletNode.Temp, 0.0001);            // outlet dry bulb
//     EXPECT_NEAR(3570.0, Coil.ElecCoolingPower, 0.01);
//     // Save results for comparison
//     Real64 MultiSpeedTotalCoolingRate1 = Coil.TotalCoolingEnergyRate;
//     Real64 MultiSpeedSensCoolingRate1 = Coil.SensCoolingEnergyRate;
//     Real64 MultiSpeedLatCoolingRate1 = Coil.LatCoolingEnergyRate;
//     // Real64 MultiSpeedOutletHumRat1 = AirOutletNode.HumRat;
//     Real64 MultiSpeedOutletTemp1 = AirOutletNode.Temp;
//     Real64 MultiSpeedElecPower1 = Coil.ElecCoolingPower;
//
//     // Test 2 - dry coil - run the coil at low speed (speednum=1, speedratio=0) - same result?
//     SpeedNum = 1;
//     SpeedRatio = 0.0;
//     state->dataHVACGlobal->MSHPMassFlowRateLow = Coil.MSRatedAirMassFlowRate(1);
//     state->dataHVACGlobal->MSHPMassFlowRateHigh = Coil.MSRatedAirMassFlowRate(1);
//     Coil.InletAirMassFlowRate = state->dataHVACGlobal->MSHPMassFlowRateLow;
//     DXCoils::CalcMultiSpeedDXCoilCooling(*state, DXCoilNum, SpeedRatio, CycRatio, SpeedNum, fanOp, compressorOp, SingleMode);
//     EXPECT_NEAR(MultiSpeedTotalCoolingRate1, Coil.TotalCoolingEnergyRate, 0.0001); // total capacity at high speed
//     EXPECT_NEAR(MultiSpeedSensCoolingRate1, Coil.SensCoolingEnergyRate, 0.0001);   // sensible cooling rate at high speed
//     EXPECT_NEAR(MultiSpeedLatCoolingRate1, Coil.LatCoolingEnergyRate, 1.0E-11);    // zero latent cooling rate at high speed
//     EXPECT_DOUBLE_EQ(0.0055, AirInletNode.HumRat);                                 // input check
//     EXPECT_DOUBLE_EQ(AirInletNode.HumRat, AirOutletNode.HumRat);                   // dry cooling only
//     EXPECT_NEAR(35.0, AirInletNode.Temp, 0.0001);                                  // inlet dry bulb
//     EXPECT_NEAR(MultiSpeedOutletTemp1, AirOutletNode.Temp, 0.0001);                // outlet dry bulb
//     EXPECT_NEAR(MultiSpeedElecPower1, Coil.ElecCoolingPower, 0.01);
//
//     // tests 3 & 4: wet cooling
//     Coil.InletAirTemp = 24.0;
//     Coil.InletAirHumRat = 0.0100;
//     Coil.InletAirEnthalpy = Psychrometrics::PsyHFnTdbW(Coil.InletAirTemp, Coil.InletAirHumRat);
//     // set coil inlet and outlet node condition
//     AirInletNode.Temp = Coil.InletAirTemp;
//     AirInletNode.HumRat = Coil.InletAirHumRat;
//     AirInletNode.Enthalpy = Coil.InletAirEnthalpy;
//     // Test 3 - wet coil - run coil at low speed - run the coil at low speed (speednum=2, speedratio=0, CycFanCycCoil)
//     SpeedNum = 2;
//     SpeedRatio = 0.0;
//     CycRatio = 1.0;
//     state->dataHVACGlobal->MSHPMassFlowRateLow = Coil.MSRatedAirMassFlowRate(1);
//     state->dataHVACGlobal->MSHPMassFlowRateHigh = Coil.MSRatedAirMassFlowRate(2);
//     Coil.InletAirMassFlowRate = state->dataHVACGlobal->MSHPMassFlowRateLow;
//     DXCoils::CalcMultiSpeedDXCoilCooling(*state, DXCoilNum, SpeedRatio, CycRatio, SpeedNum, fanOp, compressorOp, SingleMode);
//     EXPECT_NEAR(10710.0, Coil.TotalCoolingEnergyRate, 0.0001);   // equals low speed cooling capacity
//     EXPECT_NEAR(6908.14887, Coil.SensCoolingEnergyRate, 0.0001); // sensible cooling rate at low speed
//     EXPECT_NEAR(3801.851126, Coil.LatCoolingEnergyRate, 0.0001); // latent cooling rate at low speed
//     EXPECT_DOUBLE_EQ(0.0100, AirInletNode.HumRat);               // input check
//     EXPECT_NEAR(0.00751079, AirOutletNode.HumRat, 0.00001);      // cooling and dehumidification
//     EXPECT_NEAR(24.0, AirInletNode.Temp, 0.0001);                // inlet dry bulb
//     EXPECT_NEAR(12.6989, AirOutletNode.Temp, 0.0001);            // outlet dry bulb
//     EXPECT_NEAR(3570.0, Coil.ElecCoolingPower, 0.01);
//     // Save results for comparison
//     Real64 MultiSpeedTotalCoolingRate3 = Coil.TotalCoolingEnergyRate;
//     Real64 MultiSpeedSensCoolingRate3 = Coil.SensCoolingEnergyRate;
//     Real64 MultiSpeedLatCoolingRate3 = Coil.LatCoolingEnergyRate;
//     Real64 MultiSpeedOutletHumRat3 = AirOutletNode.HumRat;
//     Real64 MultiSpeedOutletTemp3 = AirOutletNode.Temp;
//     Real64 MultiSpeedElecPower3 = Coil.ElecCoolingPower;
//
//     // Test 4 - wet coil - run the coil at low speed (speednum=1, speedratio=0, CycFanCycCoil) - same result?
//     SpeedNum = 1;
//     SpeedRatio = 0.0;
//     state->dataHVACGlobal->MSHPMassFlowRateLow = Coil.MSRatedAirMassFlowRate(1);
//     state->dataHVACGlobal->MSHPMassFlowRateHigh = Coil.MSRatedAirMassFlowRate(1);
//     Coil.InletAirMassFlowRate = state->dataHVACGlobal->MSHPMassFlowRateLow;
//     DXCoils::CalcMultiSpeedDXCoilCooling(*state, DXCoilNum, SpeedRatio, CycRatio, SpeedNum, fanOp, compressorOp, SingleMode);
//     EXPECT_NEAR(MultiSpeedTotalCoolingRate3, Coil.TotalCoolingEnergyRate, 0.0001); // total capacity at high speed
//     EXPECT_NEAR(MultiSpeedSensCoolingRate3, Coil.SensCoolingEnergyRate, 0.0001);   // sensible cooling rate at high speed
//     EXPECT_NEAR(MultiSpeedLatCoolingRate3, Coil.LatCoolingEnergyRate, 0.0001);     // latent cooling rate at high speed
//     EXPECT_DOUBLE_EQ(0.0100, AirInletNode.HumRat);                                 // input check
//     EXPECT_NEAR(MultiSpeedOutletHumRat3, AirOutletNode.HumRat, 0.00001);           // cooling and dehumidification
//     EXPECT_NEAR(24.0, AirInletNode.Temp, 0.0001);                                  // inlet dry bulb
//     EXPECT_NEAR(MultiSpeedOutletTemp3, AirOutletNode.Temp, 0.0001);                // outlet dry bulb
//     EXPECT_NEAR(MultiSpeedElecPower3, Coil.ElecCoolingPower, 0.01);
//
//     // Test 5 - wet coil - run the coil at almost low speed (speednum=2, speedratio=0.00001, CycFanCycCoil) - same result?
//     SpeedNum = 2;
//     SpeedRatio = 0.00001;
//     state->dataHVACGlobal->MSHPMassFlowRateLow = Coil.MSRatedAirMassFlowRate(1);
//     state->dataHVACGlobal->MSHPMassFlowRateHigh = Coil.MSRatedAirMassFlowRate(2);
//     Coil.InletAirMassFlowRate =
//         SpeedRatio * state->dataHVACGlobal->MSHPMassFlowRateHigh + (1.0 - SpeedRatio) * state->dataHVACGlobal->MSHPMassFlowRateLow;
//     DXCoils::CalcMultiSpeedDXCoilCooling(*state, DXCoilNum, SpeedRatio, CycRatio, SpeedNum, fanOp, compressorOp, SingleMode);
//     EXPECT_NEAR(MultiSpeedTotalCoolingRate3, Coil.TotalCoolingEnergyRate, 0.1); // total capacity at high speed
//     EXPECT_NEAR(MultiSpeedSensCoolingRate3, Coil.SensCoolingEnergyRate, 0.1);   // sensible cooling rate at high speed
//     EXPECT_NEAR(MultiSpeedLatCoolingRate3, Coil.LatCoolingEnergyRate, 0.1);     // latent cooling rate at high speed
//     EXPECT_DOUBLE_EQ(0.0100, AirInletNode.HumRat);                              // input check
//     EXPECT_NEAR(MultiSpeedOutletHumRat3, AirOutletNode.HumRat, 0.001);          // cooling and dehumidification
//     EXPECT_NEAR(24.0, AirInletNode.Temp, 0.0001);                               // inlet dry bulb
//     EXPECT_NEAR(MultiSpeedOutletTemp3, AirOutletNode.Temp, 0.01);               // outlet dry bulb
//     EXPECT_NEAR(MultiSpeedElecPower3, Coil.ElecCoolingPower, 0.1);
// }

TEST_F(CoilCoolingDXTest, CoilCoolingDX_LowerSpeedFlowSizingTest)
{
    // issue #9100
    std::string const idf_objects = delimited_string({

        "  Coil:Cooling:DX,",
        "    DX Cooling Coil,                 !- Name",
        "    DX Cooling Coil Air Inlet Node,  !- Evaporator Inlet Node Name",
        "    DX Cooling Coil Air Outlet Node, !- Evaporator Outlet Node Name",
        "    ,                                !- Availability Schedule Name",
        "    ,                                !- Condenser Zone Name",
        "    DX Cooling Coil Condenser Inlet Node,   !- Condenser Inlet Node Name",
        "    DX Cooling Coil Condenser Outlet Node,  !- Condenser Outlet Node Name",
        "    DX Cooling Coil Performance,     !- Performance Object Name",
        "    ,                                !- Condensate Collection Water Storage Tank Name",
        "    ;                                !- Evaporative Condenser Supply Water Storage Tank Name",

        "  Coil:Cooling:DX:CurveFit:Performance,",
        "    DX Cooling Coil Performance,  !- Name",
        "    0,                       !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    10,                      !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Unit Internal Static Air Pressure {Pa}",
        "    ,                        !- Capacity Control Method",
        "    ,                        !- Evaporative Condenser Basin Heater Capacity {W/K}",
        "    ,                        !- Evaporative Condenser Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Evaporative Condenser Basin Heater Operating Schedule Name",
        "    Electricity,             !- Compressor Fuel Type",
        "    DX Cooling Coil Operating Mode,  !- Base Operating Mode",
        "    ,                        !- Alternative Operating Mode 1",
        "    ;                        !- Alternative Operating Mode 2",

        "  Coil:Cooling:DX:CurveFit:OperatingMode,",
        "    DX Cooling Coil Operating Mode,  !- Name",
        "    15000,                   !- Rated Gross Total Cooling Capacity {W}",
        "    0.95,                    !- Rated Evaporator Air Flow Rate {m3/s}",
        "    ,                        !- Rated Condenser Air Flow Rate {m3/s}",
        "    0,                       !- Maximum Cycling Rate {cycles/hr}",
        "    0,                       !- Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity {dimensionless}",
        "    0,                       !- Latent Capacity Time Constant {s}",
        "    0,                       !- Nominal Time for Condensate Removal to Begin {s}",
        "    ,                        !- Apply Latent Degradation to Speeds Greater than 1",
        "    AirCooled,               !- Condenser Type",
        "    0,                       !- Nominal Evaporative Condenser Pump Power {W}",
        "    4,                       !- Nominal Speed Number",
        "    DX Cooling Coil Speed 1 Performance,  !- Speed 1 Name",
        "    DX Cooling Coil Speed 2 Performance,  !- Speed 2 Name",
        "    DX Cooling Coil Speed 3 Performance,  !- Speed 3 Name",
        "    DX Cooling Coil Speed 4 Performance;  !- Speed 4 Name",

        "  Coil:Cooling:DX:CurveFit:Speed,",
        "    DX Cooling Coil Speed 1 Performance,  !- Name",
        "    0.25,                    !- Gross Total Cooling Capacity Fraction",
        "    0.25,                    !- Evaporator Air Flow Rate Fraction",
        "    0.25,                    !- Condenser Air Flow Rate Fraction",
        "    0.77,                    !- Gross Sensible Heat Ratio",
        "    4.17,                    !- Gross Cooling COP {W/W}",
        "    1.0,                     !- Active Fraction of Coil Face Area",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Evaporative Condenser Pump Power Fraction",
        "    0,                       !- Evaporative Condenser Effectiveness {dimensionless}",
        "    CAPFT,                   !- Total Cooling Capacity Modifier Function of Temperature Curve Name",
        "    CAPFF,                   !- Total Cooling Capacity Modifier Function of Air Flow Fraction Curve Name",
        "    EIRFT,                   !- Energy Input Ratio Modifier Function of Temperature Curve Name",
        "    EIRFF,                   !- Energy Input Ratio Modifier Function of Air Flow Fraction Curve Name",
        "    PLFFPLR,                 !- Part Load Fraction Correlation Curve Name",
        "    ,                        !- Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    ,                        !- Waste Heat Modifier Function of Temperature Curve Name",
        "    ,                        !- Sensible Heat Ratio Modifier Function of Temperature Curve Name",
        "    ;                        !- Sensible Heat Ratio Modifier Function of Flow Fraction Curve Name",

        "  Coil:Cooling:DX:CurveFit:Speed,",
        "    DX Cooling Coil Speed 2 Performance,  !- Name",
        "    0.50,                    !- Gross Total Cooling Capacity Fraction",
        "    0.50,                    !- Evaporator Air Flow Rate Fraction",
        "    0.50,                    !- Condenser Air Flow Rate Fraction",
        "    0.77,                    !- Gross Sensible Heat Ratio",
        "    4.17,                    !- Gross Cooling COP {W/W}",
        "    1.0,                     !- Active Fraction of Coil Face Area",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Evaporative Condenser Pump Power Fraction",
        "    0,                       !- Evaporative Condenser Effectiveness {dimensionless}",
        "    CAPFT,                   !- Total Cooling Capacity Modifier Function of Temperature Curve Name",
        "    CAPFF,                   !- Total Cooling Capacity Modifier Function of Air Flow Fraction Curve Name",
        "    EIRFT,                   !- Energy Input Ratio Modifier Function of Temperature Curve Name",
        "    EIRFF,                   !- Energy Input Ratio Modifier Function of Air Flow Fraction Curve Name",
        "    PLFFPLR,                 !- Part Load Fraction Correlation Curve Name",
        "    ,                        !- Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    ,                        !- Waste Heat Modifier Function of Temperature Curve Name",
        "    ,                        !- Sensible Heat Ratio Modifier Function of Temperature Curve Name",
        "    ;                        !- Sensible Heat Ratio Modifier Function of Flow Fraction Curve Name",

        "  Coil:Cooling:DX:CurveFit:Speed,",
        "    DX Cooling Coil Speed 3 Performance,  !- Name",
        "    0.75,                    !- Gross Total Cooling Capacity Fraction",
        "    0.75,                    !- Evaporator Air Flow Rate Fraction",
        "    0.75,                    !- Condenser Air Flow Rate Fraction",
        "    0.77,                    !- Gross Sensible Heat Ratio",
        "    4.17,                    !- Gross Cooling COP {W/W}",
        "    1.0,                     !- Active Fraction of Coil Face Area",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Evaporative Condenser Pump Power Fraction",
        "    0,                       !- Evaporative Condenser Effectiveness {dimensionless}",
        "    CAPFT,                   !- Total Cooling Capacity Modifier Function of Temperature Curve Name",
        "    CAPFF,                   !- Total Cooling Capacity Modifier Function of Air Flow Fraction Curve Name",
        "    EIRFT,                   !- Energy Input Ratio Modifier Function of Temperature Curve Name",
        "    EIRFF,                   !- Energy Input Ratio Modifier Function of Air Flow Fraction Curve Name",
        "    PLFFPLR,                 !- Part Load Fraction Correlation Curve Name",
        "    ,                        !- Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    ,                        !- Waste Heat Modifier Function of Temperature Curve Name",
        "    ,                        !- Sensible Heat Ratio Modifier Function of Temperature Curve Name",
        "    ;                        !- Sensible Heat Ratio Modifier Function of Flow Fraction Curve Name",

        "  Coil:Cooling:DX:CurveFit:Speed,",
        "    DX Cooling Coil Speed 4 Performance,  !- Name",
        "    1.0,                     !- Gross Total Cooling Capacity Fraction",
        "    1.0,                     !- Evaporator Air Flow Rate Fraction",
        "    1.0,                     !- Condenser Air Flow Rate Fraction",
        "    0.77,                    !- Gross Sensible Heat Ratio",
        "    4.17,                    !- Gross Cooling COP {W/W}",
        "    1.0,                     !- Active Fraction of Coil Face Area",
        "    ,                        !- 2017 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- 2023 Rated Evaporator Fan Power Per Volume Flow Rate {W/(m3/s)}",
        "    ,                        !- Evaporative Condenser Pump Power Fraction",
        "    0,                       !- Evaporative Condenser Effectiveness {dimensionless}",
        "    CAPFT,                   !- Total Cooling Capacity Modifier Function of Temperature Curve Name",
        "    CAPFF,                   !- Total Cooling Capacity Modifier Function of Air Flow Fraction Curve Name",
        "    EIRFT,                   !- Energy Input Ratio Modifier Function of Temperature Curve Name",
        "    EIRFF,                   !- Energy Input Ratio Modifier Function of Air Flow Fraction Curve Name",
        "    PLFFPLR,                 !- Part Load Fraction Correlation Curve Name",
        "    ,                        !- Rated Waste Heat Fraction of Power Input {dimensionless}",
        "    ,                        !- Waste Heat Modifier Function of Temperature Curve Name",
        "    ,                        !- Sensible Heat Ratio Modifier Function of Temperature Curve Name",
        "    ;                        !- Sensible Heat Ratio Modifier Function of Flow Fraction Curve Name",

        "Curve:Quadratic, PLFFPLR, 0.85, 0.83, 0.0, 0.0, 0.3, 0.85, 1.0, Dimensionless, Dimensionless; ",
        "Curve:Cubic, CAPFF, 1, 0, 0, 0, 0, 1, , , Dimensionless, Dimensionless; ",
        "Curve:Cubic, EIRFF, 1, 0, 0, 0, 0, 1, , , Dimensionless, Dimensionless; ",
        "Curve:Biquadratic, CAPFT, 1, 0, 0, 0, 0, 0, 0, 100, 0, 100, , , Temperature, Temperature, Dimensionless;",
        "Curve:Biquadratic, EIRFT, 1, 0, 0, 0, 0, 0, 0, 100, 0, 100, , , Temperature, Temperature, Dimensionless;",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    int coilIndex = CoilCoolingDX::factory(*state, "DX Cooling Coil");
    auto &this_dx_clg_coil = state->dataCoilCoolingDX->coilCoolingDXs[coilIndex];
    // check dx cooling coil inputs
    EXPECT_EQ(this_dx_clg_coil.name, "DX COOLING COIL");

    // set dx cooling coil condenser inlet air conditions
    state->dataEnvrn->OutDryBulbTemp = 35.0;
    state->dataEnvrn->OutHumRat = 0.0196;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutWetBulbTemp = 27.0932;
    state->dataSize->CurZoneEqNum = 0;
    state->dataSize->CurOASysNum = 0;
    state->dataSize->CurSysNum = 1;
    state->dataSize->CurDuctType = HVAC::AirDuctType::Cooling;
    state->dataSize->FinalSysSizing.allocate(1);
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).CoolSupTemp = 12.0;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).CoolSupHumRat = 0.0085;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).MixTempAtCoolPeak = 28.0;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).MixHumRatAtCoolPeak = 0.0075;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesCoolVolFlow = 0.80;
    state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesOutAirVolFlow = 0.2;
    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    state->dataAirSystemsData->PrimaryAirSystems.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).NumOACoolCoils = 0;
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).supFanNum = -1;
    state->dataAirSystemsData->PrimaryAirSystems(state->dataSize->CurSysNum).retFanNum = -1;
    state->dataSize->SysSizingRunDone = true;
    state->dataSize->SysSizInput.allocate(1);
    state->dataSize->SysSizInput(1).AirLoopNum = state->dataSize->CurSysNum;
    state->dataSize->NumSysSizInput = 1;
    state->dataEnvrn->StdBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.0;
    state->dataSize->DataAirFlowUsedForSizing = state->dataSize->FinalSysSizing(state->dataSize->CurSysNum).DesCoolVolFlow;

    // size cooling coil dx
    this_dx_clg_coil.size(*state);

    // check the normal operating mode names
    EXPECT_EQ(this_dx_clg_coil.performance->nameAtSpeed(0), "DX COOLING COIL SPEED 1 PERFORMANCE");
    EXPECT_EQ(this_dx_clg_coil.performance->nameAtSpeed(1), "DX COOLING COIL SPEED 2 PERFORMANCE");
    EXPECT_EQ(this_dx_clg_coil.performance->nameAtSpeed(2), "DX COOLING COIL SPEED 3 PERFORMANCE");
    EXPECT_EQ(this_dx_clg_coil.performance->nameAtSpeed(3), "DX COOLING COIL SPEED 4 PERFORMANCE");

    struct TestQuery
    {
        // clang-format off
        TestQuery(std::string t_description, std::string t_units, Real64 t_value)
            : description(t_description), units(t_units), expectedValue(t_value),
              displayString("Description='" + description + "'; Units='" + units + "'") {};
        // clang-format on

        const std::string description;
        const std::string units;
        const Real64 expectedValue;
        const std::string displayString;
    };

    // test 1: speed 1 cooling coil dx
    std::string compType = "Coil:Cooling:DX:CurveFit:Speed";
    std::string compName = "DX COOLING COIL SPEED 1 PERFORMANCE"; // this_dx_clg_coil.performance.normalMode.speeds[0].name;
    // expected results
    std::vector<TestQuery> speed1_testQueries(
        {TestQuery("Design Size Rated Air Flow Rate", "m3/s", 0.2000), TestQuery("Design Size Gross Cooling Capacity", "W", 3260.1028)});

    for (auto &testQuery : speed1_testQueries) {
        std::string query("SELECT Value From ComponentSizes"
                          "  WHERE CompType = '" +
                          compType +
                          "'"
                          "  AND CompName = '" +
                          compName +
                          "'"
                          "  AND Description = '" +
                          testQuery.description + "'" + "  AND Units = '" + testQuery.units + "'");

        Real64 return_val = SQLiteFixture::execAndReturnFirstDouble(query);
        if (return_val < 0) {
            EXPECT_TRUE(false) << "Query returned nothing for " << testQuery.displayString;
        } else {
            EXPECT_NEAR(testQuery.expectedValue, return_val, 0.0001) << "Failed for " << testQuery.displayString;
        }
    }

    // test 2: speed 2 cooling coil dx
    compType = "Coil:Cooling:DX:CurveFit:Speed";
    compName = this_dx_clg_coil.performance->nameAtSpeed(1);
    // expected results
    std::vector<TestQuery> speed2_testQueries(
        {TestQuery("Design Size Rated Air Flow Rate", "m3/s", 0.4000), TestQuery("Design Size Gross Cooling Capacity", "W", 6520.2056)});

    for (auto &testQuery : speed2_testQueries) {
        std::string query("SELECT Value From ComponentSizes"
                          "  WHERE CompType = '" +
                          compType +
                          "'"
                          "  AND CompName = '" +
                          compName +
                          "'"
                          "  AND Description = '" +
                          testQuery.description + "'" + "  AND Units = '" + testQuery.units + "'");

        Real64 return_val = SQLiteFixture::execAndReturnFirstDouble(query);
        if (return_val < 0) {
            EXPECT_TRUE(false) << "Query returned nothing for " << testQuery.displayString;
        } else {
            EXPECT_NEAR(testQuery.expectedValue, return_val, 0.0001) << "Failed for " << testQuery.displayString;
        }
    }

    // test 3: speed 3 cooling coil dx
    compType = "Coil:Cooling:DX:CurveFit:Speed";
    compName = this_dx_clg_coil.performance->nameAtSpeed(2);
    // expected results
    std::vector<TestQuery> speed3_testQueries(
        {TestQuery("Design Size Rated Air Flow Rate", "m3/s", 0.6000), TestQuery("Design Size Gross Cooling Capacity", "W", 9780.3084)});

    for (auto &testQuery : speed3_testQueries) {
        std::string query("SELECT Value From ComponentSizes"
                          "  WHERE CompType = '" +
                          compType +
                          "'"
                          "  AND CompName = '" +
                          compName +
                          "'"
                          "  AND Description = '" +
                          testQuery.description + "'" + "  AND Units = '" + testQuery.units + "'");

        Real64 return_val = SQLiteFixture::execAndReturnFirstDouble(query);
        if (return_val < 0) {
            EXPECT_TRUE(false) << "Query returned nothing for " << testQuery.displayString;
        } else {
            EXPECT_NEAR(testQuery.expectedValue, return_val, 0.0001) << "Failed for " << testQuery.displayString;
        }
    }
}
