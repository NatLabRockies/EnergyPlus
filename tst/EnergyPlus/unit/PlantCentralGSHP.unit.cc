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

// EnergyPlus::PlantCentralGSHP Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchNodeConnections.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantCentralGSHP.hh>
#include <EnergyPlus/PlantUtilities.hh>

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, ChillerHeater_Autosize)
{
    state->init_state(*state);
    // Allocate One Wrapper with One module (=distinct ChillerHeaterPerformance:Electric:EIR)
    // but with a number of identical number module of 2 in CentralHeatPumpSystem
    int NumWrappers = 1;
    state->dataPlantCentralGSHP->numWrappers = NumWrappers;
    state->dataPlantCentralGSHP->Wrapper.allocate(NumWrappers);

    int NumberOfComp = 1;
    state->dataPlantCentralGSHP->Wrapper(1).NumOfComp = NumberOfComp;
    state->dataPlantCentralGSHP->Wrapper(1).WrapperComp.allocate(NumberOfComp);

    state->dataPlantCentralGSHP->Wrapper(1).WrapperComp(1).WrapperPerformanceObjectType = "CHILLERHEATERPERFORMANCE:ELECTRIC:EIR";
    state->dataPlantCentralGSHP->Wrapper(1).WrapperComp(1).WrapperIdenticalObjectNum = 2;
    state->dataPlantCentralGSHP->Wrapper(1).WrapperComp(1).chSched = Sched::GetScheduleAlwaysOn(*state);
    state->dataPlantCentralGSHP->Wrapper(1).ChillerHeaterNums = 2;
    state->dataPlantCentralGSHP->Wrapper(1).ChillerHeater.allocate(2);
    // First test in SizeWrapper, so need to set that
    state->dataPlantCentralGSHP->Wrapper(1).ControlMode = PlantCentralGSHP::CondenserType::SmartMixing;

    int NumChillerHeaters = 1;
    state->dataPlantCentralGSHP->numChillerHeaters = NumChillerHeaters;
    state->dataPlantCentralGSHP->ChillerHeater.allocate(NumChillerHeaters);
    state->dataPlantCentralGSHP->ChillerHeater(1).ConstantFlow = false;
    state->dataPlantCentralGSHP->ChillerHeater(1).VariableFlow = true;
    state->dataPlantCentralGSHP->ChillerHeater(1).condenserType = PlantCentralGSHP::CondenserType::WaterCooled;

    state->dataPlantCentralGSHP->ChillerHeater(1).SizFac = 1.2;

    state->dataPlantCentralGSHP->ChillerHeater(1).RefCapCooling = DataSizing::AutoSize;
    state->dataPlantCentralGSHP->ChillerHeater(1).RefCapCoolingWasAutoSized = true;

    state->dataPlantCentralGSHP->ChillerHeater(1).EvapVolFlowRate = DataSizing::AutoSize;
    state->dataPlantCentralGSHP->ChillerHeater(1).EvapVolFlowRateWasAutoSized = true;

    state->dataPlantCentralGSHP->ChillerHeater(1).CondVolFlowRate = DataSizing::AutoSize;
    state->dataPlantCentralGSHP->ChillerHeater(1).CondVolFlowRateWasAutoSized = true;

    // Needed for calcs
    state->dataPlantCentralGSHP->ChillerHeater(1).RefCOPCooling = 1.5;
    state->dataPlantCentralGSHP->ChillerHeater(1).OpenMotorEff = 0.98;
    state->dataPlantCentralGSHP->ChillerHeater(1).TempRefCondInCooling = 29.4;
    state->dataPlantCentralGSHP->ChillerHeater(1).ClgHtgToCoolingCapRatio = 0.74;
    state->dataPlantCentralGSHP->ChillerHeater(1).ClgHtgtoCogPowerRatio = 1.38;

    // Add the References onto the wrapper
    state->dataPlantCentralGSHP->Wrapper(1).ChillerHeater(1) = state->dataPlantCentralGSHP->ChillerHeater(1);
    state->dataPlantCentralGSHP->Wrapper(1).ChillerHeater(2) = state->dataPlantCentralGSHP->ChillerHeater(1);

    // De-allocate temporary arrays (happens in GetInput too...)
    state->dataPlantCentralGSHP->ChillerHeater.deallocate();

    state->dataPlnt->PlantLoop.allocate(2);
    state->dataSize->PlantSizData.allocate(2);

    // Chilled Water Loop
    int PltSizNum = 1;
    state->dataPlnt->PlantLoop(PltSizNum).PlantSizNum = 1;
    state->dataPlnt->PlantLoop(PltSizNum).FluidName = "WATER";
    state->dataPlnt->PlantLoop(PltSizNum).glycol = Fluid::GetWater(*state);
    state->dataSize->PlantSizData(PltSizNum).DesVolFlowRate = 1.0;
    state->dataSize->PlantSizData(PltSizNum).DeltaT = 10.0;
    state->dataSize->PlantSizData(PltSizNum).LoopType = DataSizing::TypeOfPlantLoop::Cooling;
    // Assign to the wrapper
    state->dataPlantCentralGSHP->Wrapper(1).CWPlantLoc.loopNum = PltSizNum;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataPlantCentralGSHP->Wrapper(1).CWPlantLoc);

    // Condenser Loop
    int PltSizCondNum = 2;
    state->dataPlnt->PlantLoop(PltSizCondNum).PlantSizNum = PltSizCondNum;
    state->dataPlnt->PlantLoop(PltSizCondNum).FluidName = "WATER";
    state->dataPlnt->PlantLoop(PltSizCondNum).glycol = Fluid::GetWater(*state);
    state->dataSize->PlantSizData(PltSizCondNum).DeltaT = 5.6;
    state->dataSize->PlantSizData(PltSizCondNum).LoopType = DataSizing::TypeOfPlantLoop::Condenser;
    // Assign to the wrapper
    state->dataPlantCentralGSHP->Wrapper(1).GLHEPlantLoc.loopNum = PltSizCondNum;
    PlantUtilities::SetPlantLocationLinks(*state, state->dataPlantCentralGSHP->Wrapper(1).GLHEPlantLoc);

    // Calculate expected values
    Real64 rho_evap = state->dataPlnt->PlantLoop(PltSizNum).glycol->getDensity(*state, Constant::CWInitConvTemp, "ChillerHeater_Autosize_TEST");

    Real64 Cp_evap = state->dataPlnt->PlantLoop(PltSizNum).glycol->getSpecificHeat(*state, Constant::CWInitConvTemp, "ChillerHeater_Autosize_TEST");

    Real64 rho_cond = state->dataPlnt->PlantLoop(PltSizCondNum).glycol->getDensity(*state, Constant::CWInitConvTemp, "ChillerHeater_Autosize_TEST");

    Real64 Cp_cond = state->dataPlnt->PlantLoop(PltSizCondNum)
                         .glycol->getSpecificHeat(
                             *state, state->dataPlantCentralGSHP->Wrapper(1).ChillerHeater(1).TempRefCondInCooling, "ChillerHeater_Autosize_TEST");

    // Note: Each individual chiller heater module is sized to be capable of supporting the total load on the wrapper

    // Flow is multiplied by the SizFac
    Real64 EvapVolFlowRateExpected =
        state->dataSize->PlantSizData(PltSizNum).DesVolFlowRate * state->dataPlantCentralGSHP->Wrapper(1).ChillerHeater(1).SizFac;

    Real64 RefCapCoolingExpected = rho_evap * Cp_evap * EvapVolFlowRateExpected * state->dataSize->PlantSizData(PltSizNum).DeltaT;

    Real64 CondVolFlowRateExpected = RefCapCoolingExpected *
                                     (1.0 + (1.0 / state->dataPlantCentralGSHP->Wrapper(1).ChillerHeater(1).RefCOPCooling) *
                                                state->dataPlantCentralGSHP->Wrapper(1).ChillerHeater(1).OpenMotorEff) /
                                     (rho_cond * Cp_cond * state->dataSize->PlantSizData(PltSizCondNum).DeltaT);

    // now call sizing routine
    state->dataPlnt->PlantFirstSizesOkayToFinalize = true;
    state->dataPlantCentralGSHP->Wrapper(1).SizeWrapper(*state);

    // Careful of actually using PlantCentralGSHP::Wrapper(1).ChillerHeater(1) and not PlantCentralGSHP::ChillerHeater since this array isn't used
    // anymore by the module
    EXPECT_DOUBLE_EQ(EvapVolFlowRateExpected, state->dataPlantCentralGSHP->Wrapper(1).ChillerHeater(1).EvapVolFlowRate);
    EXPECT_DOUBLE_EQ(RefCapCoolingExpected, state->dataPlantCentralGSHP->Wrapper(1).ChillerHeater(1).RefCapCooling);

    EXPECT_DOUBLE_EQ(CondVolFlowRateExpected, state->dataPlantCentralGSHP->Wrapper(1).ChillerHeater(1).CondVolFlowRate);
    EXPECT_DOUBLE_EQ(CondVolFlowRateExpected, state->dataPlantCentralGSHP->Wrapper(1).ChillerHeater(2).CondVolFlowRate);

    // Ensure that stuff that other quantities that depends on RefCapCooling are also initialized properly
    // Heating Cap
    Real64 RefCapClgHtgExpected = RefCapCoolingExpected * state->dataPlantCentralGSHP->Wrapper(1).ChillerHeater(1).ClgHtgToCoolingCapRatio;
    EXPECT_DOUBLE_EQ(RefCapClgHtgExpected, state->dataPlantCentralGSHP->Wrapper(1).ChillerHeater(1).RefCapClgHtg);

    // Heating Power: Calc cooling Power = Cap / COP, and multiply by ratio
    Real64 RefPowerClgHtgExpected = (RefCapCoolingExpected / state->dataPlantCentralGSHP->Wrapper(1).ChillerHeater(1).RefCOPCooling) *
                                    state->dataPlantCentralGSHP->Wrapper(1).ChillerHeater(1).ClgHtgtoCogPowerRatio;
    EXPECT_DOUBLE_EQ(RefPowerClgHtgExpected, state->dataPlantCentralGSHP->Wrapper(1).ChillerHeater(1).RefPowerClgHtg);

    // Heating COP = Heating Cap / Heating Power
    Real64 RefCOPClgHtgExpected = RefCapClgHtgExpected / RefPowerClgHtgExpected;
    EXPECT_DOUBLE_EQ(RefCOPClgHtgExpected, state->dataPlantCentralGSHP->Wrapper(1).ChillerHeater(1).RefCOPClgHtg);
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_Control_Schedule_fix)
{
    std::string const idf_objects = delimited_string({

        "Schedule:Compact,",
        "Always1, !-Name",
        "On/Off, !-Schedule Type Limits Name",
        "Through: 12/31, !-Field 1",
        "For: AllDays, !-Field 2",
        "Until: 24:00, 1; !-Field 3 ",

        "CentralHeatPumpSystem,",
        "ChW_Loop HeatPump1, !-Name",
        "SmartMixing, !-Control Method",
        "ChW_Loop HeatPump1 ChW Inlet, !-Cooling Loop Inlet Node Name",
        "ChW_Loop HeatPump1 ChW Outlet, !-Cooling Loop Outlet Node Name",
        "ChW_Loop HeatPump1 Cnd Inlet, !-Source Loop Inlet Node Name",
        "ChW_Loop HeatPump1 Cnd Outlet, !-Source Loop Outlet Node Name",
        "ChW_Loop HeatPump1 HHW Inlet, !-Heating Loop Inlet Node Name",
        "ChW_Loop HeatPump1 HHW Outlet, !-Heating Loop Outlet Node Name",
        "460,  !-Ancillary Power{W}",
        ",  !-Ancillary Operation Schedule Name",
        "ChillerHeaterPerformance:Electric:EIR, !-Chiller Heater Modules Performance Component Object Type 1",
        "ChW_Loop HeatPump1 Module, !-Chiller Heater Modules Performance Component Name 1",
        "Always_1_typo, !-Chiller Heater Modules Control Schedule Name 1",
        "2; !-Number of Chiller Heater Modules 1",

        "ChillerHeaterPerformance:Electric:EIR,",
        "    ChW_Loop HeatPump1 Module,  !- Name",
        "    autosize,                !- Reference Cooling Mode Evaporator Capacity {W}",
        "    1.5,                     !- Reference Cooling Mode COP {W/W}",
        "    6.67,                    !- Reference Cooling Mode Leaving Chilled Water Temperature {C}",
        "    29.4,                    !- Reference Cooling Mode Entering Condenser Fluid Temperature {C}",
        "    35.0,                    !- Reference Cooling Mode Leaving Condenser Water Temperature {C}",
        "    0.74,                    !- Reference Heating Mode Cooling Capacity Ratio",
        "    0.925,                   !- Reference Heating Mode Cooling Power Input Ratio",
        "    6.67,                    !- Reference Heating Mode Leaving Chilled Water Temperature {C}",
        "    60,                      !- Reference Heating Mode Leaving Condenser Water Temperature {C}",
        "    29.4,                    !- Reference Heating Mode Entering Condenser Fluid Temperature {C}",
        "    5,                       !- Heating Mode Entering Chilled Water Temperature Low Limit {C}",
        "    VariableFlow,            !- Chilled Water Flow Mode Type",
        "    autosize,                !- Design Chilled Water Flow Rate {m3/s}",
        "    autosize,                !- Design Condenser Water Flow Rate {m3/s}",
        "    0.01684,                 !- Design Hot Water Flow Rate {m3/s}",
        "    1,                       !- Compressor Motor Efficiency",
        "    WaterCooled,             !- Condenser Type",
        "    EnteringCondenser,       !- Cooling Mode Temperature Curve Condenser Water Independent Variable",
        "    ChillerHeaterClgCapFT,   !- Cooling Mode Cooling Capacity Function of Temperature Curve Name",
        "    ChillerHeaterClgEIRFT,   !- Cooling Mode Electric Input to Cooling Output Ratio Function of Temperature Curve Name",
        "    ChillerHeaterClgEIRFPLR, !- Cooling Mode Electric Input to Cooling Output Ratio Function of Part Load Ratio Curve Name",
        "    1,                       !- Cooling Mode Cooling Capacity Optimum Part Load Ratio",
        "    LeavingCondenser,        !- Heating Mode Temperature Curve Condenser Water Independent Variable",
        "    ChillerHeaterHtgCapFT,   !- Heating Mode Cooling Capacity Function of Temperature Curve Name",
        "    ChillerHeaterHtgEIRFT,   !- Heating Mode Electric Input to Cooling Output Ratio Function of Temperature Curve Name",
        "    ChillerHeaterHtgEIRFPLR, !- Heating Mode Electric Input to Cooling Output Ratio Function of Part Load Ratio Curve Name",
        "    1,                       !- Heating Mode Cooling Capacity Optimum Part Load Ratio",
        "    1;                       !- Sizing Factor",

        "Curve:Biquadratic,",
        "    ChillerHeaterClgCapFT,   !- Name",
        "    0.950829,                !- Coefficient1 Constant",
        "    3.419327E-02,            !- Coefficient2 x",
        "    2.66642E-04,             !- Coefficient3 x**2",
        "    -1.733397E-03,           !- Coefficient4 y",
        "    -1.762417E-04,           !- Coefficient5 y**2",
        "    -3.69198E-05,            !- Coefficient6 x*y",
        "    4.44,                    !- Minimum Value of x",
        "    12.78,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    29.44,                   !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Curve:Biquadratic,",
        "    ChillerHeaterHtgCapFT,   !- Name",
        "    0.9415266,               !- Coefficient1 Constant",
        "    5.527431E-02,            !- Coefficient2 x",
        "    3.573558E-04,            !- Coefficient3 x**2",
        "    1.258391E-03,            !- Coefficient4 y",
        "    -6.420546E-05,           !- Coefficient5 y**2",
        "    -5.350989E-04,           !- Coefficient6 x*y",
        "    4.44,                    !- Minimum Value of x",
        "    15.56,                   !- Maximum Value of x",
        "    35,                      !- Minimum Value of y",
        "    57.22,                   !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Curve:Biquadratic,",
        "    ChillerHeaterClgEIRFT,   !- Name",
        "    0.7362431,               !- Coefficient1 Constant",
        "    2.136491E-02,            !- Coefficient2 x",
        "    3.638909E-04,            !- Coefficient3 x**2",
        "    -4.284947E-03,           !- Coefficient4 y",
        "    3.389817E-04,            !- Coefficient5 y**2",
        "    -3.632396E-04,           !- Coefficient6 x*y",
        "    4.44,                    !- Minimum Value of x",
        "    12.78,                   !- Maximum Value of x",
        "    12.78,                   !- Minimum Value of y",
        "    29.44,                   !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Curve:Biquadratic,",
        "    ChillerHeaterHtgEIRFT,   !- Name",
        "    0.2286246,               !- Coefficient1 Constant",
        "    2.498714E-02,            !- Coefficient2 x",
        "    -1.267106E-05,           !- Coefficient3 x**2",
        "    9.327184E-03,            !- Coefficient4 y",
        "    5.892037E-05,            !- Coefficient5 y**2",
        "    -3.268512E-04,           !- Coefficient6 x*y",
        "    4.44,                    !- Minimum Value of x",
        "    15.56,                   !- Maximum Value of x",
        "    35.0,                    !- Minimum Value of y",
        "    57.22,                   !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        " Curve:Cubic,",
        "     ChillerHeaterClgEIRFPLR, !- Name",
        "     0.0,                     !- Coefficient1 Constant",
        "     1.22895,                 !- Coefficient2 x",
        "     -0.751383,               !- Coefficient3 x**2",
        "     0.517396,                !- Coefficient4 x**3",
        "     0.2,                     !- Minimum Value of x",
        "     1;                       !- Maximum Value of x",

        "Curve:Cubic,",
        "    ChillerHeaterHtgEIRFPLR, !- Name",
        "    0.0,                     !- Coefficient1 Constant",
        "    1.12853,                 !- Coefficient2 x",
        "    -0.0264962,              !- Coefficient3 x**2",
        "    -0.103811,               !- Coefficient4 x**3",
        "    0.3,                     !- Minimum Value of x",
        "    1;                       !- Maximum Value of x"

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->init_state(*state);

    // May not need for direct wrapper input processing call (need when caling factory)
    state->dataPlantCentralGSHP->getWrapperInputFlag = true;

    // call the central heat pump system input processing function
    PlantCentralGSHP::GetWrapperInput(*state);

    // verify that under this scenario of not finding a schedule match, ScheduleAlwaysOn is the treated default
    EXPECT_EQ(state->dataPlantCentralGSHP->Wrapper(1).WrapperComp(1).chSched, Sched::GetScheduleAlwaysOn(*state));

    // verify that node names were processed correctly
    EXPECT_EQ(state->dataBranchNodeConnections->NumOfNodeConnections, 6);

    EXPECT_EQ(state->dataBranchNodeConnections->NodeConnections(1).NodeName, "CHW_LOOP HEATPUMP1 CHW INLET");
    EXPECT_ENUM_EQ(state->dataBranchNodeConnections->NodeConnections(1).ConnectionType, DataLoopNode::ConnectionType::Inlet);
    EXPECT_ENUM_EQ(state->dataBranchNodeConnections->NodeConnections(1).FluidStream, NodeInputManager::CompFluidStream::Primary);

    EXPECT_EQ(state->dataBranchNodeConnections->NodeConnections(2).NodeName, "CHW_LOOP HEATPUMP1 CHW OUTLET");
    EXPECT_ENUM_EQ(state->dataBranchNodeConnections->NodeConnections(2).ConnectionType, DataLoopNode::ConnectionType::Outlet);
    EXPECT_ENUM_EQ(state->dataBranchNodeConnections->NodeConnections(2).FluidStream, NodeInputManager::CompFluidStream::Primary);

    EXPECT_EQ(state->dataBranchNodeConnections->NodeConnections(3).NodeName, "CHW_LOOP HEATPUMP1 CND INLET");
    EXPECT_ENUM_EQ(state->dataBranchNodeConnections->NodeConnections(3).ConnectionType, DataLoopNode::ConnectionType::Inlet);
    EXPECT_ENUM_EQ(state->dataBranchNodeConnections->NodeConnections(3).FluidStream, NodeInputManager::CompFluidStream::Secondary);

    EXPECT_EQ(state->dataBranchNodeConnections->NodeConnections(4).NodeName, "CHW_LOOP HEATPUMP1 CND OUTLET");
    EXPECT_ENUM_EQ(state->dataBranchNodeConnections->NodeConnections(4).ConnectionType, DataLoopNode::ConnectionType::Outlet);
    EXPECT_ENUM_EQ(state->dataBranchNodeConnections->NodeConnections(4).FluidStream, NodeInputManager::CompFluidStream::Secondary);

    EXPECT_EQ(state->dataBranchNodeConnections->NodeConnections(5).NodeName, "CHW_LOOP HEATPUMP1 HHW INLET");
    EXPECT_ENUM_EQ(state->dataBranchNodeConnections->NodeConnections(5).ConnectionType, DataLoopNode::ConnectionType::Inlet);
    EXPECT_ENUM_EQ(state->dataBranchNodeConnections->NodeConnections(5).FluidStream, NodeInputManager::CompFluidStream::Tertiary);

    EXPECT_EQ(state->dataBranchNodeConnections->NodeConnections(6).NodeName, "CHW_LOOP HEATPUMP1 HHW OUTLET");
    EXPECT_ENUM_EQ(state->dataBranchNodeConnections->NodeConnections(6).ConnectionType, DataLoopNode::ConnectionType::Outlet);
    EXPECT_ENUM_EQ(state->dataBranchNodeConnections->NodeConnections(6).FluidStream, NodeInputManager::CompFluidStream::Tertiary);
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_adjustChillerHeaterCondFlowTemp)
{
    state->dataFluid->init_state(*state);
    state->dataPlantCentralGSHP->Wrapper.allocate(1);
    state->dataPlantCentralGSHP->Wrapper(1).WrapperComp.allocate(1);
    state->dataPlantCentralGSHP->Wrapper(1).ChillerHeater.allocate(1);
    auto &thisWrap = state->dataPlantCentralGSHP->Wrapper(1);
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    thisWrap.HWPlantLoc.loopNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, thisWrap.HWPlantLoc);

    Real64 qCondenser;
    Real64 condMassFlowRate;
    Real64 condOutletTemp;
    Real64 condInletTemp;
    Real64 condDeltaTemp;
    Real64 expCondenser;
    Real64 expMassFlowRate;
    Real64 expOutletTemp;
    Real64 constexpr allowedTolerance = 0.0001;

    // Test 1: Variable Flow--qCondenser is less than what the conditions say (mass flow reduced, nothing else changes)
    qCondenser = 1000.0;
    condMassFlowRate = 1.0;
    condOutletTemp = 60.0;
    condInletTemp = 59.0;
    condDeltaTemp = 1.0;
    thisWrap.VariableFlowCH = true;
    expCondenser = 1000.0;
    expMassFlowRate = 0.23897;
    expOutletTemp = 60.0;
    thisWrap.adjustChillerHeaterCondFlowTemp(*state, qCondenser, condMassFlowRate, condOutletTemp, condInletTemp, condDeltaTemp);
    EXPECT_NEAR(qCondenser, expCondenser, allowedTolerance);
    EXPECT_NEAR(condMassFlowRate, expMassFlowRate, allowedTolerance);
    EXPECT_NEAR(condOutletTemp, expOutletTemp, allowedTolerance);

    // Test 2: Variable Flow--qCondenser is greater than what conditions say (load reduced, nothing else changes)
    qCondenser = 5000.0;
    condMassFlowRate = 1.0;
    condOutletTemp = 60.0;
    condInletTemp = 59.0;
    condDeltaTemp = 1.0;
    thisWrap.VariableFlowCH = true;
    expCondenser = 4184.6;
    expMassFlowRate = 1.0;
    expOutletTemp = 60.0;
    thisWrap.adjustChillerHeaterCondFlowTemp(*state, qCondenser, condMassFlowRate, condOutletTemp, condInletTemp, condDeltaTemp);
    EXPECT_NEAR(qCondenser, expCondenser, allowedTolerance);
    EXPECT_NEAR(condMassFlowRate, expMassFlowRate, allowedTolerance);
    EXPECT_NEAR(condOutletTemp, expOutletTemp, allowedTolerance);

    // Test 3: Constant Flow--Outlet Temp greater than calculated outlet temp (outlet temp changes, nothing else changes)
    qCondenser = 1000.0;
    condMassFlowRate = 1.0;
    condOutletTemp = 60.0;
    condInletTemp = 59.0;
    condDeltaTemp = 1.0;
    thisWrap.VariableFlowCH = false;
    expCondenser = 1000.0;
    expMassFlowRate = 1.0;
    expOutletTemp = 59.23897;
    thisWrap.adjustChillerHeaterCondFlowTemp(*state, qCondenser, condMassFlowRate, condOutletTemp, condInletTemp, condDeltaTemp);
    EXPECT_NEAR(qCondenser, expCondenser, allowedTolerance);
    EXPECT_NEAR(condMassFlowRate, expMassFlowRate, allowedTolerance);
    EXPECT_NEAR(condOutletTemp, expOutletTemp, allowedTolerance);

    // Test 4: Constant Flow--Outlet Temp less than calculated outlet temp (load changes, nothing else changes)
    qCondenser = 8369.2;
    condMassFlowRate = 1.0;
    condOutletTemp = 60.0;
    condInletTemp = 59.0;
    condDeltaTemp = 1.0;
    thisWrap.VariableFlowCH = false;
    expCondenser = 4184.6;
    expMassFlowRate = 1.0;
    expOutletTemp = 60.0;
    thisWrap.adjustChillerHeaterCondFlowTemp(*state, qCondenser, condMassFlowRate, condOutletTemp, condInletTemp, condDeltaTemp);
    EXPECT_NEAR(qCondenser, expCondenser, allowedTolerance);
    EXPECT_NEAR(condMassFlowRate, expMassFlowRate, allowedTolerance);
    EXPECT_NEAR(condOutletTemp, expOutletTemp, allowedTolerance);
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_adjustChillerHeaterEvapFlowTemp)
{
    state->dataFluid->init_state(*state);
    state->dataPlantCentralGSHP->Wrapper.allocate(1);
    state->dataPlantCentralGSHP->Wrapper(1).WrapperComp.allocate(1);
    state->dataPlantCentralGSHP->Wrapper(1).ChillerHeater.allocate(1);
    auto &thisWrap = state->dataPlantCentralGSHP->Wrapper(1);
    state->dataPlnt->PlantLoop.allocate(1);
    state->dataPlnt->PlantLoop(1).FluidName = "WATER";
    state->dataPlnt->PlantLoop(1).glycol = Fluid::GetWater(*state);
    thisWrap.HWPlantLoc.loopNum = 1;
    PlantUtilities::SetPlantLocationLinks(*state, thisWrap.HWPlantLoc);

    Real64 qEvaporator;
    Real64 evapMassFlowRate;
    Real64 evapOutletTemp;
    Real64 evapInletTemp;
    Real64 expMassFlowRate;
    Real64 expOutletTemp;
    Real64 constexpr allowedTolerance = 0.0001;

    // Test 1a: qEvaporator is too low, flow rate set to zero and outlet temp set to inlet temp
    qEvaporator = 0.00001;
    evapMassFlowRate = 1.0;
    evapOutletTemp = 34.0;
    evapInletTemp = 35.0;
    thisWrap.VariableFlowCH = false;
    expMassFlowRate = 0.0;
    expOutletTemp = 35.0;
    thisWrap.adjustChillerHeaterEvapFlowTemp(*state, qEvaporator, evapMassFlowRate, evapOutletTemp, evapInletTemp);
    EXPECT_NEAR(evapMassFlowRate, expMassFlowRate, allowedTolerance);
    EXPECT_NEAR(evapOutletTemp, expOutletTemp, allowedTolerance);

    // Test 1b: delta T is zero, load and flow rate set to zero and outlet temp set to inlet temp
    qEvaporator = 1000.0;
    evapMassFlowRate = 1.0;
    evapOutletTemp = 35.0;
    evapInletTemp = 35.0;
    thisWrap.VariableFlowCH = false;
    expMassFlowRate = 0.0;
    expOutletTemp = 35.0;
    thisWrap.adjustChillerHeaterEvapFlowTemp(*state, qEvaporator, evapMassFlowRate, evapOutletTemp, evapInletTemp);
    EXPECT_NEAR(evapMassFlowRate, expMassFlowRate, allowedTolerance);
    EXPECT_NEAR(evapOutletTemp, expOutletTemp, allowedTolerance);

    // Test 2a: Variable Flow, Load higher than max flow rate passed in, keep flow rate and adjust outlet temp
    qEvaporator = 5000.0;
    evapMassFlowRate = 1.0;
    evapOutletTemp = 34.0;
    evapInletTemp = 35.0;
    thisWrap.VariableFlowCH = true;
    expMassFlowRate = 1.0;
    expOutletTemp = 33.80383;
    thisWrap.adjustChillerHeaterEvapFlowTemp(*state, qEvaporator, evapMassFlowRate, evapOutletTemp, evapInletTemp);
    EXPECT_NEAR(evapMassFlowRate, expMassFlowRate, allowedTolerance);
    EXPECT_NEAR(evapOutletTemp, expOutletTemp, allowedTolerance);

    // Test 2b: Variable Flow, Load lower than max flow rate passed in, adjust flow rate and keep outlet temp
    qEvaporator = 1045.0;
    evapMassFlowRate = 1.0;
    evapOutletTemp = 34.0;
    evapInletTemp = 35.0;
    thisWrap.VariableFlowCH = true;
    expMassFlowRate = 0.25;
    expOutletTemp = 34.0;
    thisWrap.adjustChillerHeaterEvapFlowTemp(*state, qEvaporator, evapMassFlowRate, evapOutletTemp, evapInletTemp);
    EXPECT_NEAR(evapMassFlowRate, expMassFlowRate, allowedTolerance);
    EXPECT_NEAR(evapOutletTemp, expOutletTemp, allowedTolerance);

    // Test 3: Constant Flow--adjust outlet temperature
    qEvaporator = 2090.0;
    evapMassFlowRate = 1.0;
    evapOutletTemp = 34.0;
    evapInletTemp = 35.0;
    thisWrap.VariableFlowCH = false;
    expMassFlowRate = 1.0;
    expOutletTemp = 34.5;
    thisWrap.adjustChillerHeaterEvapFlowTemp(*state, qEvaporator, evapMassFlowRate, evapOutletTemp, evapInletTemp);
    EXPECT_NEAR(evapMassFlowRate, expMassFlowRate, allowedTolerance);
    EXPECT_NEAR(evapOutletTemp, expOutletTemp, allowedTolerance);
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_setChillerHeaterCondTemp)
{
    state->dataPlantCentralGSHP->Wrapper.allocate(1);
    state->dataPlantCentralGSHP->Wrapper(1).WrapperComp.allocate(1);
    state->dataPlantCentralGSHP->Wrapper(1).ChillerHeater.allocate(1);
    auto &thisWrap = state->dataPlantCentralGSHP->Wrapper(1);
    auto &thisCH = thisWrap.ChillerHeater(1);

    Real64 functionAnswer;
    Real64 expectedAnswer;
    Real64 constexpr allowedTolerance = 0.001;
    Real64 condEnterTemp;
    Real64 condLeaveTemp;
    int chillNum = 1;

    // Test 1: get the condenser entering temperature
    functionAnswer = 0.0;
    thisCH.CondMode = EnergyPlus::PlantCentralGSHP::CondenserModeTemperature::EnteringCondenser;
    condEnterTemp = 55.5;
    condLeaveTemp = 44.4;
    expectedAnswer = 55.5;
    functionAnswer = thisWrap.setChillerHeaterCondTemp(*state, chillNum, condEnterTemp, condLeaveTemp);
    EXPECT_NEAR(functionAnswer, expectedAnswer, allowedTolerance);

    // Test 2: get the condenser leaving temperature
    functionAnswer = 0.0;
    thisCH.CondMode = EnergyPlus::PlantCentralGSHP::CondenserModeTemperature::LeavingCondenser;
    condEnterTemp = 55.5;
    condLeaveTemp = 44.4;
    expectedAnswer = 44.4;
    functionAnswer = thisWrap.setChillerHeaterCondTemp(*state, chillNum, condEnterTemp, condLeaveTemp);
    EXPECT_NEAR(functionAnswer, expectedAnswer, allowedTolerance);
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_checkEvapOutletTemp)
{
    state->dataPlantCentralGSHP->Wrapper.allocate(1);
    state->dataPlantCentralGSHP->Wrapper(1).WrapperComp.allocate(1);
    state->dataPlantCentralGSHP->Wrapper(1).ChillerHeater.allocate(1);
    auto &thisWrap = state->dataPlantCentralGSHP->Wrapper(1);
    auto &thisCH = thisWrap.ChillerHeater(1);

    int chNum = 1;
    Real64 evapOutletTemp;
    Real64 lowTempLimitEout;
    Real64 evapInletTemp;
    Real64 qEvaporator;
    Real64 evapMassFlowRate;
    Real64 Cp = 4000.0;
    Real64 expQEvap;
    Real64 expTout;
    Real64 constexpr allowedTolerance = 0.0001;

    // Test 1a: Evaporator outlet temperature lower the evaporator outlet low temperature limit, adjust outlet and load
    thisCH.EvapOutletNode.TempMin = 5.0;
    evapInletTemp = 10.0;
    evapOutletTemp = 8.0;
    lowTempLimitEout = 9.0;
    qEvaporator = 4000.0;
    evapMassFlowRate = 0.5;
    expQEvap = 2000.0;
    expTout = 9.0;
    thisWrap.checkEvapOutletTemp(*state, chNum, evapOutletTemp, lowTempLimitEout, evapInletTemp, qEvaporator, evapMassFlowRate, Cp);
    EXPECT_NEAR(qEvaporator, expQEvap, allowedTolerance);
    EXPECT_NEAR(evapOutletTemp, expTout, allowedTolerance);

    // Test 1b: Evaporator outlet temperature lower the evaporator outlet low temperature limit and inlet temp at or below lowTempLimitEout,
    //          zero flow and set outlet temperature to inlet temperature
    thisCH.EvapOutletNode.TempMin = 5.0;
    evapInletTemp = 8.0;
    evapOutletTemp = 7.0;
    lowTempLimitEout = 9.0;
    qEvaporator = 2000.0;
    evapMassFlowRate = 0.5;
    expQEvap = 0.0;
    expTout = 8.0;
    thisWrap.checkEvapOutletTemp(*state, chNum, evapOutletTemp, lowTempLimitEout, evapInletTemp, qEvaporator, evapMassFlowRate, Cp);
    EXPECT_NEAR(qEvaporator, expQEvap, allowedTolerance);
    EXPECT_NEAR(evapOutletTemp, expTout, allowedTolerance);

    // Test 2a: Evaporator outlet temperature lower the node minimum temperature limit, adjust outlet and load
    thisCH.EvapOutletNode.TempMin = 9.0;
    evapInletTemp = 10.0;
    evapOutletTemp = 8.0;
    lowTempLimitEout = 5.0;
    qEvaporator = 4000.0;
    evapMassFlowRate = 0.5;
    expQEvap = 2000.0;
    expTout = 9.0;
    thisWrap.checkEvapOutletTemp(*state, chNum, evapOutletTemp, lowTempLimitEout, evapInletTemp, qEvaporator, evapMassFlowRate, Cp);
    EXPECT_NEAR(qEvaporator, expQEvap, allowedTolerance);
    EXPECT_NEAR(evapOutletTemp, expTout, allowedTolerance);

    // Test 2b: Evaporator outlet temperature lower the node minimum temperature limit and inlet temp at or below node temperature limt,
    //          zero flow and set outlet temperature to inlet temperature
    thisCH.EvapOutletNode.TempMin = 9.0;
    evapInletTemp = 8.0;
    evapOutletTemp = 7.0;
    lowTempLimitEout = 5.0;
    qEvaporator = 2000.0;
    evapMassFlowRate = 0.5;
    expQEvap = 0.0;
    expTout = 8.0;
    thisWrap.checkEvapOutletTemp(*state, chNum, evapOutletTemp, lowTempLimitEout, evapInletTemp, qEvaporator, evapMassFlowRate, Cp);
    EXPECT_NEAR(qEvaporator, expQEvap, allowedTolerance);
    EXPECT_NEAR(evapOutletTemp, expTout, allowedTolerance);

    // Test 3: Everything is fine, no changes to anything
    thisCH.EvapOutletNode.TempMin = 5.0;
    evapInletTemp = 8.0;
    evapOutletTemp = 6.0;
    lowTempLimitEout = 5.0;
    qEvaporator = 4000.0;
    evapMassFlowRate = 0.5;
    expQEvap = 4000.0;
    expTout = 6.0;
    thisWrap.checkEvapOutletTemp(*state, chNum, evapOutletTemp, lowTempLimitEout, evapInletTemp, qEvaporator, evapMassFlowRate, Cp);
    EXPECT_NEAR(qEvaporator, expQEvap, allowedTolerance);
    EXPECT_NEAR(evapOutletTemp, expTout, allowedTolerance);
}

TEST_F(EnergyPlusFixture, Test_CentralHeatPumpSystem_calcPLRAndCyclingRatio)
{
    state->dataPlantCentralGSHP->Wrapper.allocate(1);
    state->dataPlantCentralGSHP->Wrapper(1).WrapperComp.allocate(1);
    state->dataPlantCentralGSHP->Wrapper(1).ChillerHeater.allocate(1);
    auto &thisWrap = state->dataPlantCentralGSHP->Wrapper(1);

    Real64 availChillerCap;
    Real64 actualPartLoadRatio;
    Real64 minPartLoadRatio;
    Real64 maxPartLoadRatio;
    Real64 qEvaporator;
    Real64 frac;
    Real64 expPLR;
    Real64 expFrac;
    Real64 expFalseLoad;
    Real64 constexpr allowedTolerance = 0.0001;

    // Test 1: available chiller capacity less than zero (PLR should be zero, frac should be 1.0)
    availChillerCap = -10000.0;
    actualPartLoadRatio = -1.0;
    minPartLoadRatio = 0.1;
    maxPartLoadRatio = 1.0;
    qEvaporator = 50000.0;
    frac = -1.0;
    expPLR = 0.0;
    expFrac = 1.0;
    expFalseLoad = 0.0;
    state->dataPlantCentralGSHP->ChillerCyclingRatio = -1.0;
    state->dataPlantCentralGSHP->ChillerPartLoadRatio = -1.0;
    state->dataPlantCentralGSHP->ChillerFalseLoadRate = -1.0;
    thisWrap.calcPLRAndCyclingRatio(*state, availChillerCap, actualPartLoadRatio, minPartLoadRatio, maxPartLoadRatio, qEvaporator, frac);
    EXPECT_NEAR(frac, expFrac, allowedTolerance);
    EXPECT_NEAR(actualPartLoadRatio, expPLR, allowedTolerance);
    EXPECT_NEAR(state->dataPlantCentralGSHP->ChillerCyclingRatio, expFrac, allowedTolerance);
    EXPECT_NEAR(state->dataPlantCentralGSHP->ChillerPartLoadRatio, expPLR, allowedTolerance);
    EXPECT_NEAR(state->dataPlantCentralGSHP->ChillerFalseLoadRate, expFalseLoad, allowedTolerance);

    // Test 2a: valid chiller capacity and evaporator load, negative minPLR
    availChillerCap = 50000.0;
    actualPartLoadRatio = -1.0;
    minPartLoadRatio = -0.1;
    maxPartLoadRatio = 1.0;
    qEvaporator = 10000.0;
    frac = -1.0;
    expPLR = 0.2;
    expFrac = 1.0;
    expFalseLoad = 0.0;
    state->dataPlantCentralGSHP->ChillerCyclingRatio = -1.0;
    state->dataPlantCentralGSHP->ChillerPartLoadRatio = -1.0;
    state->dataPlantCentralGSHP->ChillerFalseLoadRate = -1.0;
    thisWrap.calcPLRAndCyclingRatio(*state, availChillerCap, actualPartLoadRatio, minPartLoadRatio, maxPartLoadRatio, qEvaporator, frac);
    EXPECT_NEAR(frac, expFrac, allowedTolerance);
    EXPECT_NEAR(actualPartLoadRatio, expPLR, allowedTolerance);
    EXPECT_NEAR(state->dataPlantCentralGSHP->ChillerCyclingRatio, expFrac, allowedTolerance);
    EXPECT_NEAR(state->dataPlantCentralGSHP->ChillerPartLoadRatio, expPLR, allowedTolerance);
    EXPECT_NEAR(state->dataPlantCentralGSHP->ChillerFalseLoadRate, expFalseLoad, allowedTolerance);

    // Test 2b: valid chiller capacity and evaporator load, actualPLR lower then minPLR
    availChillerCap = 50000.0;
    actualPartLoadRatio = -1.0;
    minPartLoadRatio = 0.4;
    maxPartLoadRatio = 1.0;
    qEvaporator = 10000.0;
    frac = -1.0;
    expPLR = 0.4;
    expFrac = 0.5;
    expFalseLoad = 0.0;
    state->dataPlantCentralGSHP->ChillerCyclingRatio = -1.0;
    state->dataPlantCentralGSHP->ChillerPartLoadRatio = -1.0;
    state->dataPlantCentralGSHP->ChillerFalseLoadRate = -1.0;
    thisWrap.calcPLRAndCyclingRatio(*state, availChillerCap, actualPartLoadRatio, minPartLoadRatio, maxPartLoadRatio, qEvaporator, frac);
    EXPECT_NEAR(frac, expFrac, allowedTolerance);
    EXPECT_NEAR(actualPartLoadRatio, expPLR, allowedTolerance);
    EXPECT_NEAR(state->dataPlantCentralGSHP->ChillerCyclingRatio, expFrac, allowedTolerance);
    EXPECT_NEAR(state->dataPlantCentralGSHP->ChillerPartLoadRatio, expPLR, allowedTolerance);
    EXPECT_NEAR(state->dataPlantCentralGSHP->ChillerFalseLoadRate, expFalseLoad, allowedTolerance);

    // Test 2c: valid chiller capacity and evaporator load, actualPLR higher then minPLR
    availChillerCap = 50000.0;
    actualPartLoadRatio = -1.0;
    minPartLoadRatio = 0.4;
    maxPartLoadRatio = 1.0;
    qEvaporator = 30000.0;
    frac = -1.0;
    expPLR = 0.6;
    expFrac = 1.0;
    expFalseLoad = 0.0;
    state->dataPlantCentralGSHP->ChillerCyclingRatio = -1.0;
    state->dataPlantCentralGSHP->ChillerPartLoadRatio = -1.0;
    state->dataPlantCentralGSHP->ChillerFalseLoadRate = -1.0;
    thisWrap.calcPLRAndCyclingRatio(*state, availChillerCap, actualPartLoadRatio, minPartLoadRatio, maxPartLoadRatio, qEvaporator, frac);
    EXPECT_NEAR(frac, expFrac, allowedTolerance);
    EXPECT_NEAR(actualPartLoadRatio, expPLR, allowedTolerance);
    EXPECT_NEAR(state->dataPlantCentralGSHP->ChillerCyclingRatio, expFrac, allowedTolerance);
    EXPECT_NEAR(state->dataPlantCentralGSHP->ChillerPartLoadRatio, expPLR, allowedTolerance);
    EXPECT_NEAR(state->dataPlantCentralGSHP->ChillerFalseLoadRate, expFalseLoad, allowedTolerance);

    // Test 2d: valid chiller capacity and evaporator load, actualPLR higher then maxPLR
    availChillerCap = 50000.0;
    actualPartLoadRatio = -1.0;
    minPartLoadRatio = 0.4;
    maxPartLoadRatio = 1.0;
    qEvaporator = 60000.0;
    frac = -1.0;
    expPLR = 1.0;
    expFrac = 1.0;
    expFalseLoad = 0.0;
    state->dataPlantCentralGSHP->ChillerCyclingRatio = -1.0;
    state->dataPlantCentralGSHP->ChillerPartLoadRatio = -1.0;
    state->dataPlantCentralGSHP->ChillerFalseLoadRate = -1.0;
    thisWrap.calcPLRAndCyclingRatio(*state, availChillerCap, actualPartLoadRatio, minPartLoadRatio, maxPartLoadRatio, qEvaporator, frac);
    EXPECT_NEAR(frac, expFrac, allowedTolerance);
    EXPECT_NEAR(actualPartLoadRatio, expPLR, allowedTolerance);
    EXPECT_NEAR(state->dataPlantCentralGSHP->ChillerCyclingRatio, expFrac, allowedTolerance);
    EXPECT_NEAR(state->dataPlantCentralGSHP->ChillerPartLoadRatio, expPLR, allowedTolerance);
    EXPECT_NEAR(state->dataPlantCentralGSHP->ChillerFalseLoadRate, expFalseLoad, allowedTolerance);
}
