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

// EnergyPlus::Standalone ERV Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/ChillerElectricEIR.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/Enums.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;
using namespace EnergyPlus::ChillerElectricEIR;

TEST_F(EnergyPlusFixture, ChillerElectricEIR_TestNegativeCurveRoundingError)
{
    state->dataPlnt->TotNumLoops = 2;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.20;

    std::string const idf_objects = delimited_string({
        "Chiller:Electric:EIR,",
        "    Big Chiller,             !- Name",
        "    25000,                   !- Reference Capacity {W}",
        "    2.75,                    !- Reference COP {W/W}",
        "    6.67,                    !- Reference Leaving Chilled Water Temperature {C}",
        "    29.4,                    !- Reference Entering Condenser Fluid Temperature {C}",
        "    0.001075,                !- Reference Chilled Water Flow Rate {m3/s}",
        "    0.001345,                !- Reference Condenser Fluid Flow Rate {m3/s}",
        "    ChillerCentCapFT,        !- Cooling Capacity Function of Temperature Curve Name",
        "    ChillerCentEIRFT,        !- Electric Input to Cooling Output Ratio Function of Temperature Curve Name",
        "    ChillerCentEIRFPLR,      !- Electric Input to Cooling Output Ratio Function of Part Load Ratio Curve Name",
        "    0.15,                    !- Minimum Part Load Ratio",
        "    1.0,                     !- Maximum Part Load Ratio",
        "    1.0,                     !- Optimum Part Load Ratio",
        "    0.25,                    !- Minimum Unloading Ratio",
        "    Big Chiller Inlet Node,  !- Chilled Water Inlet Node Name",
        "    Big Chiller Outlet Node, !- Chilled Water Outlet Node Name",
        "    Big Chiller Condenser Inlet Node,  !- Condenser Inlet Node Name",
        "    Big Chiller Condenser Outlet Node,  !- Condenser Outlet Node Name",
        "    WaterCooled,             !- Condenser Type",
        "    ,                        !- Condenser Fan Power Ratio {W/W}",
        "    ,                        !- Fraction of Compressor Electric Consumption Rejected by Condenser",
        "    5,                       !- Leaving Chilled Water Lower Temperature Limit {C}",
        "    NotModulated,            !- Chiller Flow Mode",
        "    0.0,                     !- Design Heat Recovery Water Flow Rate {m3/s}",
        "    ,                        !- Heat Recovery Inlet Node Name",
        "    ;                        !- Heat Recovery Outlet Node Name",
        "  Curve:Biquadratic,",
        "    ChillerCentCapFT,        !- Name",
        "    0.257896E+00,            !- Coefficient1 Constant",
        "    0.389016E-01,            !- Coefficient2 x",
        "    -0.217080E-03,           !- Coefficient3 x**2",
        "    0.468684E-01,            !- Coefficient4 y",
        "    -0.942840E-03,           !- Coefficient5 y**2",
        "    -0.343440E-03,           !- Coefficient6 x*y",
        "    5.0,                     !- Minimum Value of x",
        "    10.0,                    !- Maximum Value of x",
        "    24.0,                    !- Minimum Value of y",
        "    35.0,                    !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "  Curve:Biquadratic,",
        "    ChillerCentEIRFT,        !- Name",
        "    0.933884E+00,            !- Coefficient1 Constant",
        "    -0.582120E-01,           !- Coefficient2 x",
        "    0.450036E-02,            !- Coefficient3 x**2",
        "    0.243000E-02,            !- Coefficient4 y",
        "    0.486000E-03,            !- Coefficient5 y**2",
        "    -0.121500E-02,           !- Coefficient6 x*y",
        "    5.0,                     !- Minimum Value of x",
        "    10.0,                    !- Maximum Value of x",
        "    24.0,                    !- Minimum Value of y",
        "    35.0,                    !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "  Curve:Quadratic,",
        "    ChillerCentEIRFPLR,      !- Name",
        "    -0.11519596634680696,                     !- Coefficient1 Constant",
        "    0.6582045440515927,                     !- Coefficient2 x",
        "    0.3663655959196781,                     !- Coefficient3 x**2",
        "    0.15,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",
        "  Curve:Biquadratic,",
        "    ChillerRecipCapFT,       !- Name",
        "    0.507883E+00,            !- Coefficient1 Constant",
        "    0.145228E+00,            !- Coefficient2 x",
        "    -0.625644E-02,           !- Coefficient3 x**2",
        "    -0.111780E-02,           !- Coefficient4 y",
        "    -0.129600E-03,           !- Coefficient5 y**2",
        "    -0.281880E-03,           !- Coefficient6 x*y",
        "    5.0,                     !- Minimum Value of x",
        "    10.0,                    !- Maximum Value of x",
        "    24.0,                    !- Minimum Value of y",
        "    35.0,                    !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "  Curve:Biquadratic,",
        "    ChillerRecipEIRFT,       !- Name",
        "    0.103076E+01,            !- Coefficient1 Constant",
        "    -0.103536E+00,           !- Coefficient2 x",
        "    0.710208E-02,            !- Coefficient3 x**2",
        "    0.931860E-02,            !- Coefficient4 y",
        "    0.317520E-03,            !- Coefficient5 y**2",
        "    -0.104328E-02,           !- Coefficient6 x*y",
        "    5.0,                     !- Minimum Value of x",
        "    10.0,                    !- Maximum Value of x",
        "    24.0,                    !- Minimum Value of y",
        "    35.0,                    !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",
        "  Curve:Quadratic,",
        "    ChillerRecipEIRFPLR,     !- Name",
        "    0.088065,                !- Coefficient1 Constant",
        "    1.137742,                !- Coefficient2 x",
        "    -0.225806,               !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    1.0;                     !- Maximum Value of x",
    });

    EXPECT_TRUE(process_idf(idf_objects, false));

    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;

    state->init_state(*state);
    // This should throw a negative value error, check that the display isn't 0 0,
    // but instead displays the negative number
    // An error is expected with this data
    EXPECT_THROW(GetElectricEIRChillerInput(*state), EnergyPlus::FatalError);

    // Check to see if there are two zeros in a row, if so, then the formatting is truncating too much
    EXPECT_EQ(compare_err_stream_substring("0.00,   0.00", true, false), false);
}

TEST_F(EnergyPlusFixture, ChillerElectricEIR_TestOutletNodeConditions)
{
    state->dataChillerElectricEIR->ElectricEIRChiller.allocate(1);
    auto &thisEIR = state->dataChillerElectricEIR->ElectricEIRChiller(1);

    thisEIR.EvapInletNodeNum = 1;
    thisEIR.EvapOutletNodeNum = 2;
    thisEIR.CondInletNodeNum = 3;
    thisEIR.CondOutletNodeNum = 4;
    thisEIR.HeatRecInletNodeNum = 5;
    thisEIR.HeatRecOutletNodeNum = 6;

    state->dataLoopNodes->Node.allocate(6);
    state->dataLoopNodes->Node(thisEIR.EvapInletNodeNum).Temp = 18.0;
    state->dataLoopNodes->Node(thisEIR.CondInletNodeNum).Temp = 35.0;

    thisEIR.update(*state, -2000, true);

    EXPECT_EQ(18, thisEIR.EvapOutletTemp);
    EXPECT_EQ(35, thisEIR.CondOutletTemp);

    state->dataLoopNodes->Node.deallocate();
}

TEST_F(EnergyPlusFixture, ElectricEIRChiller_HeatRecoveryAutosizeTest)
{
    state->init_state(*state);

    state->dataPlnt->PlantLoop.allocate(2);
    state->dataSize->PlantSizData.allocate(2);
    // chilled water loop
    state->dataPlnt->PlantLoop(1).PlantSizNum = 1;
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataSize->PlantSizData(1).DesVolFlowRate = 1.0;
    state->dataSize->PlantSizData(1).DeltaT = 5.0;
    // condenser water loop
    state->dataPlnt->PlantLoop(2).PlantSizNum = 2;
    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataSize->PlantSizData(2).DesVolFlowRate = 1.0;
    state->dataSize->PlantSizData(2).DeltaT = 5.0;

    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    // unit test for autosizing heat recovery in Chiller:Electric:EIR
    state->dataChillerElectricEIR->ElectricEIRChiller.allocate(1);
    auto &thisEIR = state->dataChillerElectricEIR->ElectricEIRChiller(1);

    thisEIR.SizFac = 1.0;
    thisEIR.DesignHeatRecVolFlowRateWasAutoSized = true;
    thisEIR.HeatRecCapacityFraction = 0.5;
    thisEIR.HeatRecActive = true;
    thisEIR.CondenserType = DataPlant::CondenserType::WaterCooled;
    thisEIR.CWPlantLoc.loopNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, thisEIR.CWPlantLoc);
    thisEIR.CDPlantLoc.loopNum = 2;
    PlantUtilities::SetPlantLocationLinks(*state, thisEIR.CDPlantLoc);
    thisEIR.EvapVolFlowRate = 1.0;
    thisEIR.CondVolFlowRate = 1.0;
    thisEIR.RefCap = 10000;
    thisEIR.RefCOP = 3.0;

    // now call sizing routine
    thisEIR.size(*state);
    // see if heat recovery flow rate is as expected
    EXPECT_NEAR(thisEIR.DesignHeatRecVolFlowRate, 0.5, 0.00001);

    state->dataSize->PlantSizData.deallocate();
    state->dataPlnt->PlantLoop.deallocate();
}

TEST_F(EnergyPlusFixture, ChillerElectricEIR_AirCooledChiller)
{

    bool RunFlag(true);
    Real64 MyLoad(-10000.0);

    state->dataPlnt->TotNumLoops = 2;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.20;

    std::string const idf_objects = delimited_string({
        "Chiller:Electric:EIR,",
        "  AirCooledChiller,                   !- Name",
        "  autosize,                           !- Reference Capacity {W}",
        "  5.50,                               !- Reference COP {W/W}",
        "  6.67,                               !- Reference Leaving Chilled Water Temperature {C}",
        "  29.40,                              !- Reference Entering Condenser Fluid Temperature {C}",
        "  autosize,                           !- Reference Chilled Water Flow Rate {m3/s}",
        "  autosize,                           !- Reference Condenser Fluid Flow Rate {m3/s}",
        "  Air cooled CentCapFT,               !- Cooling Capacity Function of Temperature Curve Name",
        "  Air cooled CentEIRFT,               !- Electric Input to Cooling Output Ratio Function of Temperature Curve Name",
        "  Air cooled CentEIRFPLR,             !- Electric Input to Cooling Output Ratio Function of Part Load Ratio Curve Name",
        "  0.25,                               !- Minimum Part Load Ratio",
        "  1.00,                               !- Maximum Part Load Ratio",
        "  1.00,                               !- Optimum Part Load Ratio",
        "  0.25,                               !- Minimum Unloading Ratio",
        "  CHW Inlet Node,                     !- Chilled Water Inlet Node Name",
        "  CHW Outlet Node,                    !- Chilled Water Outlet Node Name",
        "  Outdoor Air Condenser Inlet Node,   !- Condenser Inlet Node Name",
        "  Outdoor Air Condenser Outlet Node,  !- Condenser Outlet Node Name",
        "  AirCooled,                          !- Condenser Type",
        "  0.04,                               !- Condenser Fan Power Ratio {W/W}",
        "  1.00,                               !- Fraction of Compressor Electric Consumption Rejected by Condenser",
        "  5.00,                               !- Leaving Chilled Water Lower Temperature Limit {C}",
        "  NotModulated,                       !- Chiller Flow Mode",
        "  0.0,                                !- Design Heat Recovery Water Flow Rate {m3/s}",
        "  ,                                   !- Heat Recovery Inlet Node Name",
        "  ,                                   !- Heat Recovery Outlet Node Name",
        "  1.00,                               !- Sizing Factor",
        "  0.00,                               !- Basin Heater Capacity {W/K}",
        "  2.00,                               !- Basin Heater Setpoint Temperature {C}",
        "  ,                                   !- Basin Heater Operating Schedule Name",
        "  1.00,                               !- Condenser Heat Recovery Relative Capacity Fraction",
        "  ,                                   !- Heat Recovery Inlet High Temperature Limit Schedule Name",
        "  ,                                   !- Heat Recovery Leaving Temperature Setpoint Node Name",
        "  ,                                   !- End-Use Subcategory",
        "  ,                                   !- Condenser Flow Control",
        "  ,                                   !- Condenser Loop Flow Rate Fraction Function of Loop Part Load Ratio Curve Name",
        "  ,                                   !- Temperature Difference Across Condenser Schedule Name",
        "  ,                                   !- Condenser Minimum Flow Fraction",
        "  ThermoCapFracCurve;                 !- Thermosiphon Capacity Fraction Curve Name",

        "Curve:Linear, ThermoCapFracCurve, 0.0, 0.06, 0.0, 10.0, 0.0, 1.0, Dimensionless, Dimensionless;",
        "Curve:Biquadratic, Air cooled CentCapFT, 0.257896, 0.0389016, -0.00021708, 0.0468684, -0.00094284, -0.00034344, 5, 10, 24, 35, , , , , ;",
        "Curve:Biquadratic, Air cooled CentEIRFT, 0.933884, -0.058212,  0.00450036, 0.00243,    0.000486,   -0.001215,   5, 10, 24, 35, , , , , ;",
        "Curve:Quadratic, Air cooled CentEIRFPLR, 0.222903,  0.313387,  0.46371,    0, 1, , , , ;",

    });

    EXPECT_TRUE(process_idf(idf_objects, false));

    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;

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

    GetElectricEIRChillerInput(*state);
    auto &thisEIR = state->dataChillerElectricEIR->ElectricEIRChiller(1);

    state->dataPlnt->PlantLoop(1).Name = "ChilledWaterLoop";
    state->dataPlnt->PlantLoop(1).PlantSizNum = 1;
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = thisEIR.Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::Chiller_ElectricEIR;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = thisEIR.EvapInletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = thisEIR.EvapOutletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataPlnt->PlantLoop(1).TempSetPointNodeNum = thisEIR.EvapOutletNodeNum;

    state->dataSize->PlantSizData.allocate(1);
    state->dataSize->PlantSizData(1).DesVolFlowRate = 0.001;
    state->dataSize->PlantSizData(1).DeltaT = 5.0;

    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    state->dataPlnt->PlantFirstSizesOkayToReport = true;
    state->dataPlnt->PlantFinalSizesOkayToReport = true;

    thisEIR.initialize(*state, RunFlag, MyLoad);
    thisEIR.size(*state);

    // run through init again after sizing is complete to set mass flow rate
    state->dataGlobal->BeginEnvrnFlag = true;
    thisEIR.initialize(*state, RunFlag, MyLoad);

    // check chiller water side evap flow rate is non-zero
    EXPECT_NEAR(thisEIR.EvapMassFlowRateMax, 0.999898, 0.0000001);

    // check autocalculate for air-cooled or evap-cooled chiller condenser side fluid flow rate
    Real64 CalcCondVolFlow = thisEIR.RefCap * 0.000114;
    EXPECT_EQ(CalcCondVolFlow, thisEIR.CondVolFlowRate);
    EXPECT_NEAR(thisEIR.CondVolFlowRate, 2.3925760323498, 0.0000001);
    EXPECT_NEAR(thisEIR.CondMassFlowRateMax, 2.7918772761695, 0.0000001);

    // test thermosiphon model
    state->dataLoopNodes->Node(thisEIR.EvapInletNodeNum).Temp = 10.0;
    state->dataLoopNodes->Node(thisEIR.EvapOutletNodeNum).Temp = 6.0;
    state->dataLoopNodes->Node(thisEIR.EvapOutletNodeNum).TempSetPoint = 6.0;
    state->dataLoopNodes->Node(thisEIR.CondInletNodeNum).OutAirDryBulb = 12.0; // condenser inlet temp > evap outlet temp

    thisEIR.initialize(*state, RunFlag, MyLoad);
    thisEIR.calculate(*state, MyLoad, RunFlag);
    EXPECT_GT(thisEIR.ChillerPartLoadRatio, 0.4); // load is large
    EXPECT_EQ(thisEIR.thermosiphonStatus, 0);     // thermosiphon is off
    EXPECT_GT(thisEIR.Power, 1500.0);             // power is non-zero

    state->dataLoopNodes->Node(thisEIR.CondInletNodeNum).OutAirDryBulb = 5.0; // condenser inlet temp < evap outlet temp

    thisEIR.initialize(*state, RunFlag, MyLoad);
    thisEIR.calculate(*state, MyLoad, RunFlag);
    EXPECT_GT(thisEIR.ChillerPartLoadRatio, 0.4); // load is large
    EXPECT_EQ(thisEIR.thermosiphonStatus, 0);     // thermosiphon is off
    EXPECT_GT(thisEIR.Power, 1500.0);             // power is non-zero

    MyLoad /= 25.0; // reduce load such that thermosiphon can meet load
    thisEIR.initialize(*state, RunFlag, MyLoad);
    thisEIR.calculate(*state, MyLoad, RunFlag);
    Real64 dT = thisEIR.EvapOutletTemp - thisEIR.CondInletTemp;
    Real64 thermosiphonCapFrac = Curve::CurveValue(*state, thisEIR.thermosiphonTempCurveIndex, dT);
    EXPECT_LT(thisEIR.ChillerPartLoadRatio, 0.3);                 // load is small
    EXPECT_GT(thermosiphonCapFrac, thisEIR.ChillerPartLoadRatio); // thermosiphon capacity can meet load
    EXPECT_EQ(thisEIR.thermosiphonStatus, 1);                     // thermosiphon is on
    EXPECT_EQ(thisEIR.Power, 0.0);                                // power is zero
}

TEST_F(EnergyPlusFixture, ChillerElectricEIR_EvaporativelyCooled_Calculate)
{

    std::string const idf_objects = delimited_string({
        "Chiller:Electric:EIR,",
        "  EvapCooledChiller,                  !- Name",
        "  autosize,                           !- Reference Capacity {W}",
        "  5.50,                               !- Reference COP {W/W}",
        "  6.67,                               !- Reference Leaving Chilled Water Temperature {C}",
        "  29.40,                              !- Reference Entering Condenser Fluid Temperature {C}",
        "  autosize,                           !- Reference Chilled Water Flow Rate {m3/s}",
        "  autosize,                           !- Reference Condenser Fluid Flow Rate {m3/s}",
        "  Evap cooled CentCapFT,              !- Cooling Capacity Function of Temperature Curve Name",
        "  Evap cooled CentEIRFT,              !- Electric Input to Cooling Output Ratio Function of Temperature Curve Name",
        "  Evap cooled CentEIRFPLR,            !- Electric Input to Cooling Output Ratio Function of Part Load Ratio Curve Name",
        "  0.10,                               !- Minimum Part Load Ratio",
        "  1.00,                               !- Maximum Part Load Ratio",
        "  1.00,                               !- Optimum Part Load Ratio",
        "  0.25,                               !- Minimum Unloading Ratio",
        "  CHW Inlet Node,                     !- Chilled Water Inlet Node Name",
        "  CHW Outlet Node,                    !- Chilled Water Outlet Node Name",
        "  OutdoorAir Condenser Inlet Node,    !- Condenser Inlet Node Name",
        "  OutdoorAir Condenser Outlet Node,   !- Condenser Outlet Node Name",
        "  EvaporativelyCooled,                !- Condenser Type",
        "  0.04,                               !- Condenser Fan Power Ratio {W/W}",
        "  1.00,                               !- Fraction of Compressor Electric Consumption Rejected by Condenser",
        "  5.00,                               !- Leaving Chilled Water Lower Temperature Limit {C}",
        "  NotModulated,                       !- Chiller Flow Mode",
        "  0.0,                                !- Design Heat Recovery Water Flow Rate {m3/s}",
        "  ,                                   !- Heat Recovery Inlet Node Name",
        "  ,                                   !- Heat Recovery Outlet Node Name",
        "  1.00,                               !- Sizing Factor",
        "  0.00,                               !- Basin Heater Capacity {W/K}",
        "  2.00,                               !- Basin Heater Setpoint Temperature {C}",
        "  ,                                   !- Basin Heater Operating Schedule Name",
        "  1.00,                               !- Condenser Heat Recovery Relative Capacity Fraction",
        "  ,                                   !- Heat Recovery Inlet High Temperature Limit Schedule Name",
        "  ;                                   !- Heat Recovery Leaving Temperature Setpoint Node Name",

        "Curve:Biquadratic, Evap cooled CentCapFT, 0.257896, 0.0389016, -0.00021708, 0.0468684, -0.00094284, -0.00034344, 5, 10, 24, 35, , , , , ;",
        "Curve:Biquadratic, Evap cooled CentEIRFT, 0.933884, -0.058212,  0.00450036, 0.00243,    0.000486,   -0.001215,   5, 10, 24, 35, , , , , ;",
        "Curve:Quadratic, Evap cooled CentEIRFPLR, 0.222903,  0.313387,  0.46371,    0, 1, , , , ;",

    });

    EXPECT_TRUE(process_idf(idf_objects, false));

    state->dataPlnt->TotNumLoops = 2;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;

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

    GetElectricEIRChillerInput(*state);
    auto &thisEIRChiller = state->dataChillerElectricEIR->ElectricEIRChiller(1);

    state->dataPlnt->PlantLoop(1).Name = "ChilledWaterLoop";
    state->dataPlnt->PlantLoop(1).PlantSizNum = 1;
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = thisEIRChiller.Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::Chiller_ElectricEIR;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = thisEIRChiller.EvapInletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = thisEIRChiller.EvapOutletNodeNum;

    state->dataSize->PlantSizData.allocate(1);
    state->dataSize->PlantSizData(1).DesVolFlowRate = 0.001;
    state->dataSize->PlantSizData(1).DeltaT = 5.0;

    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    state->dataPlnt->PlantFirstSizesOkayToReport = true;
    state->dataPlnt->PlantFinalSizesOkayToReport = true;

    state->dataEnvrn->OutDryBulbTemp = 29.4;
    state->dataEnvrn->OutWetBulbTemp = 23.0;
    state->dataEnvrn->OutHumRat =
        Psychrometrics::PsyWFnTdbTwbPb(*state, state->dataEnvrn->OutDryBulbTemp, state->dataEnvrn->OutWetBulbTemp, state->dataEnvrn->OutBaroPress);
    state->dataLoopNodes->Node(thisEIRChiller.CondInletNodeNum).Temp = state->dataEnvrn->OutDryBulbTemp;
    state->dataLoopNodes->Node(thisEIRChiller.CondInletNodeNum).OutAirWetBulb = state->dataEnvrn->OutWetBulbTemp;
    state->dataLoopNodes->Node(thisEIRChiller.CondInletNodeNum).HumRat = state->dataEnvrn->OutHumRat;

    // set load and run flag
    bool RunFlag(true);
    Real64 MyLoad(-18000.0);
    openOutputFiles(*state);

    state->dataPlnt->PlantLoop(1).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataLoopNodes->Node(thisEIRChiller.EvapOutletNodeNum).TempSetPoint = 6.67;
    state->dataLoopNodes->Node(thisEIRChiller.EvapInletNodeNum).Temp = 16.0;
    // init and size
    thisEIRChiller.initialize(*state, RunFlag, MyLoad);
    thisEIRChiller.size(*state);
    // init again after sizing is complete to set mass flow rate
    state->dataGlobal->BeginEnvrnFlag = true;
    thisEIRChiller.initialize(*state, RunFlag, MyLoad);
    // check chiller water side evap flow rate is non-zero
    EXPECT_NEAR(thisEIRChiller.EvapMassFlowRateMax, 0.999898, 0.0000001);
    // check autocalculate for evaporatively-cooled chiller condenser side fluid flow rate
    Real64 resultCondVolFlowRate = thisEIRChiller.RefCap * 0.000114;
    EXPECT_EQ(resultCondVolFlowRate, thisEIRChiller.CondVolFlowRate);
    EXPECT_NEAR(thisEIRChiller.CondVolFlowRate, 2.3925760323498, 0.0000001);
    EXPECT_NEAR(thisEIRChiller.CondMassFlowRateMax, 2.7918772761695, 0.0000001);
    // run chiller
    thisEIRChiller.calculate(*state, MyLoad, RunFlag);
    // calc evap-cooler water consumption rate
    Real64 EvapCondWaterVolFlowRate = thisEIRChiller.CondMassFlowRate * (thisEIRChiller.CondOutletHumRat - state->dataEnvrn->OutHumRat) /
                                      Psychrometrics::RhoH2O(Constant::InitConvTemp);
    // check evap-cooled condenser water consumption rate
    EXPECT_NEAR(2.31460814, thisEIRChiller.CondMassFlowRate, 0.0000001);
    EXPECT_NEAR(6.22019725E-06, EvapCondWaterVolFlowRate, 0.000000001);
    EXPECT_NEAR(EvapCondWaterVolFlowRate, thisEIRChiller.EvapWaterConsumpRate, 0.000000001);
}

TEST_F(EnergyPlusFixture, ChillerElectricEIR_WaterCooledChillerVariableSpeedCondenser)
{

    bool RunFlag(true);
    state->dataPlnt->TotNumLoops = 2;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->dataGlobal->HourOfDay = 1;
    state->dataEnvrn->DayOfWeek = 1;
    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 1;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);

    std::string const idf_objects = delimited_string({
        "Chiller:Electric:EIR,",
        "  WaterChiller,                       !- Name",
        "  autosize,                           !- Reference Capacity {W}",
        "  1.0,                               !- Reference COP {W/W}",
        "  6.67,                               !- Reference Leaving Chilled Water Temperature {C}",
        "  29.40,                              !- Reference Entering Condenser Fluid Temperature {C}",
        "  autosize,                           !- Reference Chilled Water Flow Rate {m3/s}",
        "  0.001,                              !- Reference Condenser Fluid Flow Rate {m3/s}",
        "  DummyCapfT,                         !- Cooling Capacity Function of Temperature Curve Name",
        "  DummyEIRfT,                         !- Electric Input to Cooling Output Ratio Function of Temperature Curve Name",
        "  DummyEIRfPLR,                       !- Electric Input to Cooling Output Ratio Function of Part Load Ratio Curve Name",
        "  0.10,                               !- Minimum Part Load Ratio",
        "  1.00,                               !- Maximum Part Load Ratio",
        "  1.00,                               !- Optimum Part Load Ratio",
        "  0.25,                               !- Minimum Unloading Ratio",
        "  CHW Inlet Node,                     !- Chilled Water Inlet Node Name",
        "  CHW Outlet Node,                    !- Chilled Water Outlet Node Name",
        "  Condenser Inlet Node,               !- Condenser Inlet Node Name",
        "  Condenser Outlet Node,              !- Condenser Outlet Node Name",
        "  WaterCooled,                        !- Condenser Type",
        "  0.04,                               !- Condenser Fan Power Ratio {W/W}",
        "  1.00,                               !- Fraction of Compressor Electric Consumption Rejected by Condenser",
        "  5.00,                               !- Leaving Chilled Water Lower Temperature Limit {C}",
        "  NotModulated,                       !- Chiller Flow Mode",
        "  0.0,                                !- Design Heat Recovery Water Flow Rate {m3/s}",
        "  ,                                   !- Heat Recovery Inlet Node Name",
        "  ,                                   !- Heat Recovery Outlet Node Name",
        "  1.00,                               !- Sizing Factor",
        "  0.00,                               !- Basin Heater Capacity {W/K}",
        "  2.00,                               !- Basin Heater Setpoint Temperature {C}",
        "  ,                                   !- Basin Heater Operating Schedule Name",
        "  1.00,                               !- Condenser Heat Recovery Relative Capacity Fraction",
        "  ,                                   !- Heat Recovery Inlet High Temperature Limit Schedule Name",
        "  ,                                   !- Heat Recovery Leaving Temperature Setpoint Node Name",
        "  ,                                   !- End-Use Subcategory",
        "  ModulatedLoopPLR,                   !- Condenser Flow Control",
        "  Y=F(X),                             !- Condenser Loop Flow Rate Fraction Function of Loop Part Load Ratio Curve Name",
        "  CondenserdT,                        !- Temperature Difference Across Condenser Schedule Name",
        "  0.35;                               !- Condenser Minimum Flow Fraction",
        "Curve:Linear,Y=F(X),0,1,0,1;",
        "Schedule:Constant,CondenserdT,,10;"
        "Curve:Biquadratic, DummyCapfT, 1, 0, 0, 0, 0, 0, 5, 10, 24, 35, , , , , ;",
        "Curve:Biquadratic, DummyEIRfT, 1, 0,  0, 0, 0, 0,   5, 10, 24, 35, , , , , ;",
        "Curve:Quadratic, DummyEIRfPLR, 1,  0,  0, 0, 1, , , , ;",

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

    GetElectricEIRChillerInput(*state);
    auto &thisChiller = state->dataChillerElectricEIR->ElectricEIRChiller(1);
    state->dataLoopNodes->Node.allocate(10);

    state->dataPlnt->PlantLoop(1).Name = "ChilledWaterLoop";
    state->dataPlnt->PlantLoop(1).PlantSizNum = 1;
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).TempSetPointNodeNum = 10;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = thisChiller.Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::Chiller_ElectricEIR;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = thisChiller.EvapInletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = thisChiller.EvapOutletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Demand).TempSetPoint = 4.4;

    state->dataSize->PlantSizData.allocate(2);
    state->dataSize->PlantSizData(1).DesVolFlowRate = 0.001;
    state->dataSize->PlantSizData(1).DeltaT = 5.0;

    state->dataPlnt->PlantLoop(2).Name = "CondenserWaterLoop";
    state->dataPlnt->PlantLoop(2).PlantSizNum = 1;
    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = thisChiller.Name;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::Chiller_ElectricEIR;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = thisChiller.CondInletNodeNum;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = thisChiller.CondOutletNodeNum;

    state->dataSize->PlantSizData(2).DesVolFlowRate = 0.001;
    state->dataSize->PlantSizData(2).DeltaT = 5.0;

    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    state->dataPlnt->PlantFirstSizesOkayToReport = true;
    state->dataPlnt->PlantFinalSizesOkayToReport = true;

    Real64 MyLoad(0.0);
    thisChiller.initialize(*state, RunFlag, MyLoad);
    thisChiller.size(*state);
    MyLoad = -thisChiller.RefCap;
    state->dataSize->PlantSizData(1).DesCapacity = std::abs(MyLoad) * 2;
    Sched::UpdateScheduleVals(*state);

    // run through init again after sizing is complete to set mass flow rate
    state->dataGlobal->BeginEnvrnFlag = true;
    thisChiller.initialize(*state, RunFlag, MyLoad);

    // set node temperatures
    state->dataLoopNodes->Node(thisChiller.CondInletNodeNum).Temp = 25.0;
    state->dataLoopNodes->Node(thisChiller.EvapInletNodeNum).Temp = 15.0;
    thisChiller.CWPlantLoc.side->UpdatedDemandToLoopSetPoint = MyLoad;
    state->dataLoopNodes->Node(thisChiller.CWPlantLoc.loop->TempSetPointNodeNum).TempSetPoint = 21.0;

    // Test the different control approaches
    thisChiller.calculate(*state, MyLoad, RunFlag);
    EXPECT_NEAR(thisChiller.CondMassFlowRate, thisChiller.CondMassFlowRateMax / 2, 0.00001);

    thisChiller.CondenserFlowControl = DataPlant::CondenserFlowControl::ModulatedChillerPLR;
    MyLoad /= 2;
    thisChiller.calculate(*state, MyLoad, RunFlag);
    EXPECT_NEAR(thisChiller.CondMassFlowRate, thisChiller.CondMassFlowRateMax / 2, 0.00001);

    thisChiller.CondenserFlowControl = DataPlant::CondenserFlowControl::ModulatedDeltaTemperature;
    thisChiller.calculate(*state, MyLoad, RunFlag);
    Real64 Cp = thisChiller.CWPlantLoc.loop->glycol->getSpecificHeat(
        *state, thisChiller.CondInletTemp, "ChillerElectricEIR_WaterCooledChillerVariableSpeedCondenser");
    Real64 ActualCondFlow = 3.0 * std::abs(MyLoad) / (Cp * 10.0);
    EXPECT_NEAR(thisChiller.CondMassFlowRate, ActualCondFlow, 0.00001);

    // Test the minimum condenser flow rate
    MyLoad = -500;
    thisChiller.CondenserFlowControl = DataPlant::CondenserFlowControl::ModulatedChillerPLR;
    thisChiller.calculate(*state, MyLoad, RunFlag);
    EXPECT_NEAR(thisChiller.CondMassFlowRate, thisChiller.CondMassFlowRateMax * 0.35, 0.00001);

    // Test constant flow condenser
    thisChiller.CondenserFlowControl = DataPlant::CondenserFlowControl::ConstantFlow;
    MyLoad = -10000;
    Real64 savedMyLoad = MyLoad;

    // test with condenser flow available
    thisChiller.initialize(*state, RunFlag, MyLoad);
    thisChiller.calculate(*state, MyLoad, RunFlag);
    thisChiller.update(*state, MyLoad, RunFlag);
    Real64 chWOutletTemp = thisChiller.EvapInletTemp + savedMyLoad / (Cp * thisChiller.EvapMassFlowRate);
    Real64 condOutletTemp = thisChiller.CondInletTemp + thisChiller.QCondenser / (Cp * thisChiller.CondMassFlowRate);
    EXPECT_EQ(MyLoad, savedMyLoad);
    EXPECT_NEAR(thisChiller.CondMassFlowRate, thisChiller.CondMassFlowRateMax, 0.00001);
    EXPECT_NEAR(thisChiller.EvapMassFlowRate, thisChiller.EvapMassFlowRateMax, 0.00001);
    EXPECT_NEAR(thisChiller.EvapOutletTemp, chWOutletTemp, 0.1);
    EXPECT_NEAR(thisChiller.CondOutletTemp, condOutletTemp, 0.1);
    EXPECT_NEAR(thisChiller.QEvaporator, -savedMyLoad, 1.0);
    EXPECT_NEAR(thisChiller.QCondenser, (-savedMyLoad + thisChiller.Power), 1.0);
    EXPECT_NEAR(thisChiller.Power, 20987, 1.0);

    // test with no condenser flow available - chiller should be off
    state->dataLoopNodes->Node(thisChiller.CondInletNodeNum).MassFlowRate = 0.0;
    state->dataLoopNodes->Node(thisChiller.CondInletNodeNum).MassFlowRateMaxAvail = 0.0;
    thisChiller.initialize(*state, RunFlag, MyLoad);
    thisChiller.calculate(*state, MyLoad, RunFlag);
    thisChiller.update(*state, MyLoad, RunFlag);
    EXPECT_EQ(MyLoad, 0.0);
    EXPECT_EQ(thisChiller.CondMassFlowRate, 0.0);
    EXPECT_EQ(thisChiller.EvapMassFlowRate, 0.0);
    EXPECT_EQ(thisChiller.EvapOutletTemp, thisChiller.EvapInletTemp);
    EXPECT_EQ(thisChiller.CondOutletTemp, thisChiller.CondInletTemp);
    EXPECT_EQ(thisChiller.QEvaporator, 0.0);
    EXPECT_EQ(thisChiller.QCondenser, 0.0);
    EXPECT_EQ(thisChiller.Power, 0.0);

    // Test
}

TEST_F(EnergyPlusFixture, ChillerElectricEIR_OutputReport)
{

    bool RunFlag(true);
    state->dataPlnt->TotNumLoops = 3;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->StdRhoAir = 1.20;
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->TimeStep = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->dataGlobal->HourOfDay = 1;
    state->dataEnvrn->DayOfWeek = 1;
    state->dataEnvrn->Month = 1;
    state->dataEnvrn->DayOfMonth = 1;
    state->dataEnvrn->DayOfYear_Schedule = General::OrdinalDay(state->dataEnvrn->Month, state->dataEnvrn->DayOfMonth, 1);
    Psychrometrics::InitializePsychRoutines(*state);

    std::string const idf_objects = delimited_string({
        "Chiller:Electric:EIR,",
        "  WaterChiller,                       !- Name",
        "  autosize,                           !- Reference Capacity {W}",
        "  3.5,                                !- Reference COP {W/W}",
        "  5.67,                               !- Reference Leaving Chilled Water Temperature {C}",
        "  28.40,                              !- Reference Entering Condenser Fluid Temperature {C}",
        "  autosize,                           !- Reference Chilled Water Flow Rate {m3/s}",
        "  autosize,                           !- Reference Condenser Fluid Flow Rate {m3/s}",
        "  DummyCapfT,                         !- Cooling Capacity Function of Temperature Curve Name",
        "  DummyEIRfT,                         !- Electric Input to Cooling Output Ratio Function of Temperature Curve Name",
        "  DummyEIRfPLR,                       !- Electric Input to Cooling Output Ratio Function of Part Load Ratio Curve Name",
        "  0.10,                               !- Minimum Part Load Ratio",
        "  1.00,                               !- Maximum Part Load Ratio",
        "  1.00,                               !- Optimum Part Load Ratio",
        "  0.25,                               !- Minimum Unloading Ratio",
        "  CHW Inlet Node,                     !- Chilled Water Inlet Node Name",
        "  CHW Outlet Node,                    !- Chilled Water Outlet Node Name",
        "  Condenser Inlet Node,               !- Condenser Inlet Node Name",
        "  Condenser Outlet Node,              !- Condenser Outlet Node Name",
        "  WaterCooled,                        !- Condenser Type",
        "  0.04,                               !- Condenser Fan Power Ratio {W/W}",
        "  1.00,                               !- Fraction of Compressor Electric Consumption Rejected by Condenser",
        "  5.00,                               !- Leaving Chilled Water Lower Temperature Limit {C}",
        "  NotModulated,                       !- Chiller Flow Mode",
        "  autosize,                           !- Design Heat Recovery Water Flow Rate {m3/s}",
        "  HetRec Inlet Node,                  !- Heat Recovery Inlet Node Name",
        "  HetRec Outlet Node,                 !- Heat Recovery Outlet Node Name",
        "  1.00,                               !- Sizing Factor",
        "  0.00,                               !- Basin Heater Capacity {W/K}",
        "  2.00,                               !- Basin Heater Setpoint Temperature {C}",
        "  ,                                   !- Basin Heater Operating Schedule Name",
        "  0.30,                               !- Condenser Heat Recovery Relative Capacity Fraction",
        "  ,                                   !- Heat Recovery Inlet High Temperature Limit Schedule Name",
        "  HetRec Outlet Node,                 !- Heat Recovery Leaving Temperature Setpoint Node Name",
        "  ,                                   !- End-Use Subcategory",
        "  ModulatedLoopPLR,                   !- Condenser Flow Control",
        "  Y=F(X),                             !- Condenser Loop Flow Rate Fraction Function of Loop Part Load Ratio Curve Name",
        "  CondenserdT,                        !- Temperature Difference Across Condenser Schedule Name",
        "  0.35;                               !- Condenser Minimum Flow Fraction",
        "Curve:Linear,Y=F(X),0,1,0,1;",
        "Schedule:Constant,CondenserdT,,10;"
        "Curve:Biquadratic, DummyCapfT, 1, 0, 0, 0, 0, 0, 5, 10, 24, 35, , , , , ;",
        "Curve:Biquadratic, DummyEIRfT, 1, 0,  0, 0, 0, 0,   5, 10, 24, 35, , , , , ;",
        "Curve:Quadratic, DummyEIRfPLR, 1,  0,  0, 0, 1, , , , ;",

    });

    EXPECT_TRUE(process_idf(idf_objects, false));
    state->init_state(*state);

    OutputReportPredefined::SetPredefinedTables(*state);

    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    state->dataPlnt->PlantLoop.allocate(state->dataPlnt->TotNumLoops);
    state->dataSize->PlantSizData.allocate(state->dataPlnt->TotNumLoops);
    for (int l = 1; l <= state->dataPlnt->TotNumLoops; ++l) {
        auto &loopside(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand));
        loopside.TotalBranches = 1;
        loopside.Branch.allocate(1);
        auto &loopsidebranch(state->dataPlnt->PlantLoop(l).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1));
        loopsidebranch.TotalComponents = 1;
        loopsidebranch.Comp.allocate(1);
    }

    GetElectricEIRChillerInput(*state);
    auto &thisChiller = state->dataChillerElectricEIR->ElectricEIRChiller(1);
    int constexpr num_nodes = 10;
    state->dataLoopNodes->Node.allocate(num_nodes);

    state->dataPlnt->PlantLoop(1).Name = "ChilledWaterLoop";
    state->dataPlnt->PlantLoop(1).PlantSizNum = 1;
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(1).TempSetPointNodeNum = 10;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Name = "WaterChiller Supply Branch";
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = thisChiller.Name;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::Chiller_ElectricEIR;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = thisChiller.EvapInletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = thisChiller.EvapOutletNodeNum;
    state->dataPlnt->PlantLoop(1).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataPlnt->PlantLoop(1).LoopSide(EnergyPlus::DataPlant::LoopSideLocation::Demand).TempSetPoint = 4.4;

    state->dataSize->PlantSizData(1).DesVolFlowRate = 0.02;
    state->dataSize->PlantSizData(1).DeltaT = 5.0;

    state->dataPlnt->PlantLoop(2).Name = "CondenserWaterLoop";
    state->dataPlnt->PlantLoop(2).PlantSizNum = 2;
    state->dataPlnt->PlantLoop(2).FluidName = "WATER";
    state->dataPlnt->PlantLoop(2).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Name = "WaterChiller Condenser Branch";
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = thisChiller.Name;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::Chiller_ElectricEIR;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = thisChiller.CondInletNodeNum;
    state->dataPlnt->PlantLoop(2).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = thisChiller.CondOutletNodeNum;

    state->dataSize->PlantSizData(2).DesVolFlowRate = 0.03;
    state->dataSize->PlantSizData(2).DeltaT = 5.0;

    state->dataPlnt->PlantLoop(3).Name = "HecRecWaterLoop";
    state->dataPlnt->PlantLoop(3).PlantSizNum = 3;
    state->dataPlnt->PlantLoop(3).FluidName = "WATER";
    state->dataPlnt->PlantLoop(3).glycol = Fluid::GetWater(*state);
    state->dataPlnt->PlantLoop(3).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Name = "WaterChiller HecRec Branch";
    state->dataPlnt->PlantLoop(3).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Name = thisChiller.Name;
    state->dataPlnt->PlantLoop(3).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type =
        DataPlant::PlantEquipmentType::Chiller_ElectricEIR;
    state->dataPlnt->PlantLoop(3).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumIn = thisChiller.HeatRecInletNodeNum;
    state->dataPlnt->PlantLoop(3).LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).NodeNumOut = thisChiller.HeatRecOutletNodeNum;
    state->dataPlnt->PlantLoop(3).LoopDemandCalcScheme = DataPlant::LoopDemandCalcScheme::SingleSetPoint;
    state->dataLoopNodes->Node(thisChiller.HeatRecOutletNodeNum).TempSetPoint = 60.0;

    state->dataSize->PlantSizData(3).DesVolFlowRate = 0.03;
    state->dataSize->PlantSizData(3).DeltaT = 5.0;

    for (int n = 1; n <= num_nodes; ++n) {
        state->dataLoopNodes->Node(n).MassFlowRateMaxAvail = 2.0;
        state->dataLoopNodes->Node(n).MassFlowRateMax = 2.0;
    }

    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    state->dataPlnt->PlantFirstSizesOkayToReport = true;
    state->dataPlnt->PlantFinalSizesOkayToReport = false;

    Real64 MyLoad(0.0);
    thisChiller.initialize(*state, RunFlag, MyLoad);
    thisChiller.size(*state);
    // run through init again after sizing is complete to set mass flow rate
    MyLoad = -thisChiller.RefCap;
    state->dataSize->PlantSizData(1).DesCapacity = std::abs(MyLoad) * 2;
    Sched::UpdateScheduleVals(*state);
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataPlnt->PlantFinalSizesOkayToReport = true;
    thisChiller.initialize(*state, RunFlag, MyLoad);
    thisChiller.size(*state);

    // compare_err_stream("");
    auto &orp = *state->dataOutRptPredefined;
    std::string const ChillerName = thisChiller.Name;
    // Type
    EXPECT_EQ("Chiller:Electric:EIR", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchChillerType, ChillerName));
    // Reference Capacity[W]
    EXPECT_EQ("419750.18", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchChillerRefCap, ChillerName));
    // Reference Efficiency [W/W]
    EXPECT_EQ("3.50", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchChillerRefEff, ChillerName));
    // Rated Capacity [W]
    EXPECT_EQ("419750.18", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchChillerRatedCap, ChillerName));
    // Rated Efficiency [W/W]
    EXPECT_EQ("3.50", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchChillerRatedEff, ChillerName));
    // IPLV in SI Units [W/W]
    EXPECT_EQ("2.03", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchChillerIPLVinSI, ChillerName));
    // IPLV in IP Units [Btu/W-h]
    EXPECT_EQ("2.03", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchChillerIPLVinIP, ChillerName));
    // Minimum Part Load Ratio
    EXPECT_EQ("0.10", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchChillerMinPLR, ChillerName));
    // Fuel Type
    EXPECT_EQ("Electricity", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchChillerFuelType, ChillerName));
    // NOTE: Rated = Ref here
    // Rated Entering Condenser Temperature [C]
    EXPECT_EQ("28.40", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchChillerRatedEntCondTemp, ChillerName));
    // Rated Leaving Evaporator Temperature [C]
    EXPECT_EQ("5.67", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchChillerRatedLevEvapTemp, ChillerName));
    // Reference Entering Condenser Temperature [C]
    EXPECT_EQ("28.40", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchChillerRefEntCondTemp, ChillerName));
    // Reference Leaving Evaporator Temperature [C]
    EXPECT_EQ("5.67", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchChillerRefLevEvapTemp, ChillerName));
    // Design Size Reference Chilled Water Flow Rate [kg/s]
    EXPECT_EQ("20.00", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchChillerDesSizeRefCHWFlowRate, ChillerName));
    // Design Size Reference Condenser Fluid Flow Rate [kg/s]
    EXPECT_EQ("25.82", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchChillerDesSizeRefCondFluidFlowRate, ChillerName));
    // Plantloop Name
    EXPECT_EQ("ChilledWaterLoop", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchChillerPlantloopName, ChillerName));
    // Plantloop Branch Name
    EXPECT_EQ("WaterChiller Supply Branch",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchChillerPlantloopBranchName, ChillerName));
    // Condenser Loop Name
    EXPECT_EQ("CondenserWaterLoop", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchChillerCondLoopName, ChillerName));
    // Condenser Loop Branch Name
    EXPECT_EQ("WaterChiller Condenser Branch",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchChillerCondLoopBranchName, ChillerName));
    // Heat Recovery Plantloop Name
    EXPECT_EQ("HecRecWaterLoop", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchChillerHeatRecPlantloopName, ChillerName));
    // Heat Recovery Plantloop Branch Name
    EXPECT_EQ("WaterChiller HecRec Branch",
              OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchChillerHeatRecPlantloopBranchName, ChillerName));
    // Recovery Relative Capacity Fraction
    EXPECT_EQ("0.30", OutputReportPredefined::RetrievePreDefTableEntry(*state, orp.pdchChillerRecRelCapFrac, ChillerName));
}
