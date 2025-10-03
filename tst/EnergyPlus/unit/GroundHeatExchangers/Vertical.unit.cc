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
#include "../Fixtures/EnergyPlusFixture.hh"
#include <gtest/gtest.h>

#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/GroundHeatExchangers/ResponseFactors.hh>
#include <EnergyPlus/GroundHeatExchangers/State.hh>
#include <EnergyPlus/GroundHeatExchangers/Vertical.hh>
#include <EnergyPlus/Plant/PlantManager.hh>

using namespace EnergyPlus;
using namespace EnergyPlus::GroundHeatExchangers;

TEST_F(EnergyPlusFixture, GroundHeatExchangerTest_System_GetGFunc)
{

    // Initialization
    GLHEVert thisGLHE;
    Real64 thisGFunc;
    Real64 time;

    int NPairs = 2;

    std::shared_ptr<GLHEResponseFactors> thisRF(new GLHEResponseFactors);
    thisGLHE.myRespFactors = thisRF;

    thisGLHE.myRespFactors->GFNC = std::vector<Real64>(NPairs);

    thisGLHE.myRespFactors->LNTTS.push_back(0.0);
    thisGLHE.myRespFactors->LNTTS.push_back(5.0);
    thisGLHE.myRespFactors->GFNC[0] = 0.0;
    thisGLHE.myRespFactors->GFNC[1] = 5.0;

    time = std::pow(2.7182818284590452353602874, 2.5);

    thisGLHE.bhLength = 1.0;
    thisGLHE.bhRadius = 1.0;

    // Situation when correction is not applied
    thisGLHE.myRespFactors->gRefRatio = 1.0;
    thisGFunc = thisGLHE.getGFunc(time);
    EXPECT_DOUBLE_EQ(2.5, thisGFunc);

    // Situation when correction is applied
    thisGLHE.myRespFactors->gRefRatio = 2.0;
    thisGFunc = thisGLHE.getGFunc(time);
    EXPECT_NEAR(2.5 + 0.6931, thisGFunc, 0.0001);
}

TEST_F(EnergyPlusFixture, GHE_InterpTest2)
{
    std::shared_ptr<GroundHeatExchangers::GLHEResponseFactors> thisRF(new GroundHeatExchangers::GLHEResponseFactors());
    thisRF->GFNC = std::vector<Real64>(8);
    thisRF->LNTTS = std::vector<Real64>{-15.2202, -15.083, -14.9459, -14.8087, -14.6716, -14.5344, -14.3973, -14.2601};
    thisRF->GFNC = {-2.55692, -2.48389, -2.40819, -2.32936, -2.24715, -2.16138, -2.07195, -1.97882};

    GroundHeatExchangers::GLHEVert thisGHE = GroundHeatExchangers::GLHEVert();
    thisGHE.myRespFactors = thisRF;

    double tolerance = 1e-6;

    EXPECT_NEAR(thisGHE.interpGFunc(-15.220200), -2.556920, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-15.187093), -2.539298, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-15.153986), -2.521675, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-15.120879), -2.504053, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-15.087772), -2.486430, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-15.054666), -2.468245, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-15.021559), -2.449965, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-14.988452), -2.431685, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-14.955345), -2.413405, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-14.922238), -2.394595, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-14.889131), -2.375573, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-14.856024), -2.356551, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-14.822917), -2.337529, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-14.789810), -2.318033, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-14.756703), -2.298181, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-14.723597), -2.278329, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-14.690490), -2.258477, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-14.657383), -2.238262, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-14.624276), -2.217566, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-14.591169), -2.196869, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-14.558062), -2.176172, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-14.524955), -2.155219, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-14.491848), -2.133624, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-14.458741), -2.112028, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-14.425634), -2.090433, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-14.392528), -2.068711, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-14.359421), -2.046238, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-14.326314), -2.023765, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-14.293207), -2.001293, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-14.260100), -1.978820, tolerance);
}

TEST_F(EnergyPlusFixture, GroundHeatExchangerTest_System_calcGFunction_UBHWT)
{

    using namespace DataSystemVariables;

    std::string const idf_objects =
        delimited_string({"Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
                          "    KATemps,                 !- Name",
                          "    1.8,                     !- Soil Thermal Conductivity {W/m-K}",
                          "    920,                     !- Soil Density {kg/m3}",
                          "    2200,                    !- Soil Specific Heat {J/kg-K}",
                          "    15.5,                    !- Average Soil Surface Temperature {C}",
                          "    3.2,                     !- Average Amplitude of Surface Temperature {deltaC}",
                          "    8;                       !- Phase Shift of Minimum Surface Temperature {days}",

                          "GroundHeatExchanger:Vertical:Properties,",
                          "    GHE-1 Props,        !- Name",
                          "    1,                  !- Depth of Top of Borehole {m}",
                          "    100,                !- Borehole Length {m}",
                          "    0.109982,           !- Borehole Diameter {m}",
                          "    0.744,              !- Grout Thermal Conductivity {W/m-K}",
                          "    3.90E+06,           !- Grout Thermal Heat Capacity {J/m3-K}",
                          "    0.389,              !- Pipe Thermal Conductivity {W/m-K}",
                          "    1.77E+06,           !- Pipe Thermal Heat Capacity {J/m3-K}",
                          "    0.0267,             !- Pipe Outer Diameter {m}",
                          "    0.00243,            !- Pipe Thickness {m}",
                          "    0.04556;            !- U-Tube Distance {m}",

                          "GroundHeatExchanger:Vertical:Single,",
                          "    GHE-1,              !- Name",
                          "    GHE-1 Props,        !- GHE Properties",
                          "    0,                  !- X Location {m}",
                          "    0;                  !- Y Location {m}",

                          "GroundHeatExchanger:Vertical:Single,",
                          "    GHE-2,              !- Name",
                          "    GHE-1 Props,        !- GHE Properties",
                          "    5.0,                !- X Location {m}",
                          "    0;                  !- Y Location {m}",

                          "GroundHeatExchanger:Vertical:Single,",
                          "    GHE-3,              !- Name",
                          "    GHE-1 Props,        !- GHE Properties",
                          "    0,                  !- X Location {m}",
                          "    5.0;                !- Y Location {m}",

                          "GroundHeatExchanger:Vertical:Single,",
                          "    GHE-4,              !- Name",
                          "    GHE-1 Props,        !- GHE Properties",
                          "    5.0,                !- X Location {m}",
                          "    5.0;                !- Y Location {m}",

                          "GroundHeatExchanger:System,",
                          "    Vertical GHE 1x4 Std,  !- Name",
                          "    GHLE Inlet,         !- Inlet Node Name",
                          "    GHLE Outlet,        !- Outlet Node Name",
                          "    0.00075708,         !- Design Flow Rate {m3/s}",
                          "    Site:GroundTemperature:Undisturbed:KusudaAchenbach,  !- Undisturbed Ground Temperature Model Type",
                          "    KATemps,            !- Undisturbed Ground Temperature Model Name",
                          "    2.423,              !- Ground Thermal Conductivity {W/m-K}",
                          "    2.343E+06,          !- Ground Thermal Heat Capacity {J/m3-K}",
                          "    ,                   !- Response Factors Object Name",
                          "    UBHWTCalc,          !- g-Function Calculation Method",
                          "    ,                   !- GHE Array Object Name",
                          "    GHE-1,              !- GHE Borehole Definition 1",
                          "    GHE-2,              !- GHE Borehole Definition 2",
                          "    GHE-3,              !- GHE Borehole Definition 3",
                          "    GHE-4;              !- GHE Borehole Definition 4",

                          "Branch,",
                          "    Main Floor Cooling Condenser Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Coil:Cooling:WaterToAirHeatPump:EquationFit,  !- Component 1 Object Type",
                          "    Main Floor WAHP Cooling Coil,  !- Component 1 Name",
                          "    Main Floor WAHP Cooling Water Inlet Node,  !- Component 1 Inlet Node Name",
                          "    Main Floor WAHP Cooling Water Outlet Node;  !- Component 1 Outlet Node Name",

                          "Branch,",
                          "    Main Floor Heating Condenser Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Coil:Heating:WaterToAirHeatPump:EquationFit,  !- Component 1 Object Type",
                          "    Main Floor WAHP Heating Coil,  !- Component 1 Name",
                          "    Main Floor WAHP Heating Water Inlet Node,  !- Component 1 Inlet Node Name",
                          "    Main Floor WAHP Heating Water Outlet Node;  !- Component 1 Outlet Node Name",

                          "Branch,",
                          "    GHE-Vert Branch,         !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    GroundHeatExchanger:System,  !- Component 1 Object Type",
                          "    Vertical GHE 1x4 Std,    !- Component 1 Name",
                          "    GHLE Inlet,         !- Component 1 Inlet Node Name",
                          "    GHLE Outlet;        !- Component 1 Outlet Node Name",

                          "Branch,",
                          "    Ground Loop Supply Inlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pump:ConstantSpeed,      !- Component 1 Object Type",
                          "    Ground Loop Supply Pump, !- Component 1 Name",
                          "    Ground Loop Supply Inlet,!- Component 1 Inlet Node Name",
                          "    Ground Loop Pump Outlet; !- Component 1 Outlet Node Name",

                          "Branch,",
                          "    Ground Loop Supply Outlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    Ground Loop Supply Outlet Pipe,  !- Component 1 Name",
                          "    Ground Loop Supply Outlet Pipe Inlet,  !- Component 1 Inlet Node Name",
                          "    Ground Loop Supply Outlet;  !- Component 1 Outlet Node Name",

                          "Branch,",
                          "    Ground Loop Demand Inlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    Ground Loop Demand Inlet Pipe,  !- Component 1 Name",
                          "    Ground Loop Demand Inlet,!- Component 1 Inlet Node Name",
                          "    Ground Loop Demand Inlet Pipe Outlet;  !- Component 1 Outlet Node Name",

                          "Branch,",
                          "    Ground Loop Demand Bypass Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    Ground Loop Demand Side Bypass Pipe,  !- Component 1 Name",
                          "    Ground Loop Demand Bypass Inlet,  !- Component 1 Inlet Node Name",
                          "    Ground Loop Demand Bypass Outlet;  !- Component 1 Outlet Node Name",

                          "Branch,",
                          "    Ground Loop Demand Outlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    Ground Loop Demand Outlet Pipe,  !- Component 1 Name",
                          "    Ground Loop Demand Outlet Pipe Inlet,  !- Component 1 Inlet Node Name",
                          "    Ground Loop Demand Outlet;  !- Component 1 Outlet Node Name",

                          "BranchList,",
                          "    Ground Loop Supply Side Branches,  !- Name",
                          "    Ground Loop Supply Inlet Branch,  !- Branch 1 Name",
                          "    GHE-Vert Branch,         !- Branch 2 Name",
                          "    Ground Loop Supply Outlet Branch;  !- Branch 3 Name",

                          "BranchList,",
                          "    Ground Loop Demand Side Branches,  !- Name",
                          "    Ground Loop Demand Inlet Branch,  !- Branch 1 Name",
                          "    Main Floor Cooling Condenser Branch,  !- Branch 2 Name",
                          "    Main Floor Heating Condenser Branch,  !- Branch 3 Name",
                          "    Ground Loop Demand Bypass Branch,  !- Branch 4 Name",
                          "    Ground Loop Demand Outlet Branch;  !- Branch 5 Name",

                          "Connector:Splitter,",
                          "    Ground Loop Supply Splitter,  !- Name",
                          "    Ground Loop Supply Inlet Branch,  !- Inlet Branch Name",
                          "    GHE-Vert Branch;         !- Outlet Branch 1 Name",

                          "Connector:Splitter,",
                          "    Ground Loop Demand Splitter,  !- Name",
                          "    Ground Loop Demand Inlet Branch,  !- Inlet Branch Name",
                          "    Ground Loop Demand Bypass Branch,  !- Outlet Branch 1 Name",
                          "    Main Floor Cooling Condenser Branch,  !- Outlet Branch 2 Name",
                          "    Main Floor Heating Condenser Branch;  !- Outlet Branch 3 Name",

                          "Connector:Mixer,",
                          "    Ground Loop Supply Mixer,!- Name",
                          "    Ground Loop Supply Outlet Branch,  !- Outlet Branch Name",
                          "    GHE-Vert Branch;         !- Inlet Branch 1 Name",

                          "Connector:Mixer,",
                          "    Ground Loop Demand Mixer,!- Name",
                          "    Ground Loop Demand Outlet Branch,  !- Outlet Branch Name",
                          "    Ground Loop Demand Bypass Branch,  !- Inlet Branch 1 Name",
                          "    Main Floor Cooling Condenser Branch,  !- Inlet Branch 2 Name",
                          "    Main Floor Heating Condenser Branch;  !- Inlet Branch 3 Name",

                          "ConnectorList,",
                          "    Ground Loop Supply Side Connectors,  !- Name",
                          "    Connector:Splitter,      !- Connector 1 Object Type",
                          "    Ground Loop Supply Splitter,  !- Connector 1 Name",
                          "    Connector:Mixer,         !- Connector 2 Object Type",
                          "    Ground Loop Supply Mixer;!- Connector 2 Name",

                          "ConnectorList,",
                          "    Ground Loop Demand Side Connectors,  !- Name",
                          "    Connector:Splitter,      !- Connector 1 Object Type",
                          "    Ground Loop Demand Splitter,  !- Connector 1 Name",
                          "    Connector:Mixer,         !- Connector 2 Object Type",
                          "    Ground Loop Demand Mixer;!- Connector 2 Name",

                          "NodeList,",
                          "    Ground Loop Supply Setpoint Nodes,  !- Name",
                          "    GHLE Outlet,                        !- Node 1 Name",
                          "    Ground Loop Supply Outlet;  !- Node 2 Name",

                          "OutdoorAir:Node,",
                          "    Main Floor WAHP Outside Air Inlet,  !- Name",
                          "    -1;                      !- Height Above Ground {m}",

                          "Pipe:Adiabatic,",
                          "    Ground Loop Supply Outlet Pipe,  !- Name",
                          "    Ground Loop Supply Outlet Pipe Inlet,  !- Inlet Node Name",
                          "    Ground Loop Supply Outlet;  !- Outlet Node Name",

                          "Pipe:Adiabatic,",
                          "    Ground Loop Demand Inlet Pipe,  !- Name",
                          "    Ground Loop Demand Inlet,!- Inlet Node Name",
                          "    Ground Loop Demand Inlet Pipe Outlet;  !- Outlet Node Name",

                          "Pipe:Adiabatic,",
                          "    Ground Loop Demand Side Bypass Pipe,  !- Name",
                          "    Ground Loop Demand Bypass Inlet,  !- Inlet Node Name",
                          "    Ground Loop Demand Bypass Outlet;  !- Outlet Node Name",

                          "Pipe:Adiabatic,",
                          "    Ground Loop Demand Outlet Pipe,  !- Name",
                          "    Ground Loop Demand Outlet Pipe Inlet,  !- Inlet Node Name",
                          "    Ground Loop Demand Outlet;  !- Outlet Node Name",

                          "Pump:ConstantSpeed,",
                          "    Ground Loop Supply Pump, !- Name",
                          "    Ground Loop Supply Inlet,!- Inlet Node Name",
                          "    Ground Loop Pump Outlet, !- Outlet Node Name",
                          "    autosize,                !- Design Flow Rate {m3/s}",
                          "    179352,                  !- Design Pump Head {Pa}",
                          "    autosize,                !- Design Power Consumption {W}",
                          "    0.9,                     !- Motor Efficiency",
                          "    0,                       !- Fraction of Motor Inefficiencies to Fluid Stream",
                          "    Intermittent;            !- Pump Control Type",

                          "PlantLoop,",
                          "    Ground Loop Water Loop,  !- Name",
                          "    Water,                      !- Fluid Type",
                          "    ,                           !- User Defined Fluid Type",
                          "    Only Water Loop Operation,  !- Plant Equipment Operation Scheme Name",
                          "    Ground Loop Supply Outlet,  !- Loop Temperature Setpoint Node Name",
                          "    100,                     !- Maximum Loop Temperature {C}",
                          "    10,                      !- Minimum Loop Temperature {C}",
                          "    autosize,                !- Maximum Loop Flow Rate {m3/s}",
                          "    0,                       !- Minimum Loop Flow Rate {m3/s}",
                          "    autosize,                !- Plant Loop Volume {m3}",
                          "    Ground Loop Supply Inlet,!- Plant Side Inlet Node Name",
                          "    Ground Loop Supply Outlet,  !- Plant Side Outlet Node Name",
                          "    Ground Loop Supply Side Branches,  !- Plant Side Branch List Name",
                          "    Ground Loop Supply Side Connectors,  !- Plant Side Connector List Name",
                          "    Ground Loop Demand Inlet,!- Demand Side Inlet Node Name",
                          "    Ground Loop Demand Outlet,  !- Demand Side Outlet Node Name",
                          "    Ground Loop Demand Side Branches,  !- Demand Side Branch List Name",
                          "    Ground Loop Demand Side Connectors,  !- Demand Side Connector List Name",
                          "    SequentialLoad,          !- Load Distribution Scheme",
                          "    ,                        !- Availability Manager List Name",
                          "    DualSetPointDeadband;    !- Plant Loop Demand Calculation Scheme",

                          "PlantEquipmentList,",
                          "    Only Water Loop All Cooling Equipment,  !- Name",
                          "    GroundHeatExchanger:System,  !- Equipment 1 Object Type",
                          "    Vertical GHE 1x4 Std;    !- Equipment 1 Name",

                          "PlantEquipmentOperation:CoolingLoad,",
                          "    Only Water Loop Cool Operation All Hours,  !- Name",
                          "    0,                       !- Load Range 1 Lower Limit {W}",
                          "    1000000000000000,        !- Load Range 1 Upper Limit {W}",
                          "    Only Water Loop All Cooling Equipment;  !- Range 1 Equipment List Name",

                          "PlantEquipmentOperationSchemes,",
                          "    Only Water Loop Operation,  !- Name",
                          "    PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type",
                          "    Only Water Loop Cool Operation All Hours,  !- Control Scheme 1 Name",
                          "    HVACTemplate-Always 1;   !- Control Scheme 1 Schedule Name",

                          "SetpointManager:Scheduled:DualSetpoint,",
                          "    Ground Loop Temp Manager,!- Name",
                          "    Temperature,             !- Control Variable",
                          "    HVACTemplate-Always 34,  !- High Setpoint Schedule Name",
                          "    HVACTemplate-Always 20,  !- Low Setpoint Schedule Name",
                          "    Ground Loop Supply Setpoint Nodes;  !- Setpoint Node or NodeList Name",

                          "Schedule:Compact,",
                          "    HVACTemplate-Always 4,   !- Name",
                          "    HVACTemplate Any Number, !- Schedule Type Limits Name",
                          "    Through: 12/31,          !- Field 1",
                          "    For: AllDays,            !- Field 2",
                          "    Until: 24:00,4;          !- Field 3",

                          "Schedule:Compact,",
                          "    HVACTemplate-Always 34,  !- Name",
                          "    HVACTemplate Any Number, !- Schedule Type Limits Name",
                          "    Through: 12/31,          !- Field 1",
                          "    For: AllDays,            !- Field 2",
                          "    Until: 24:00,34;         !- Field 3",

                          "Schedule:Compact,",
                          "    HVACTemplate-Always 20,  !- Name",
                          "    HVACTemplate Any Number, !- Schedule Type Limits Name",
                          "    Through: 12/31,          !- Field 1",
                          "    For: AllDays,            !- Field 2",
                          "    Until: 24:00,20;         !- Field 3"});

    // Setup
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    PlantManager::GetPlantLoopData(*state);
    PlantManager::GetPlantInput(*state);
    PlantManager::SetupInitialPlantCallingOrder(*state);
    PlantManager::SetupBranchControlTypes(*state);

    auto &thisGLHE(state->dataGroundHeatExchanger->verticalGLHE[0]);
    thisGLHE.plantLoc.loopNum = 1;
    state->dataLoopNodes->Node(thisGLHE.inletNodeNum).Temp = 20;
    thisGLHE.designFlow = 0.00075708;

    Real64 rho = 998.207; // Density at 20 C using CoolProp
    thisGLHE.designMassFlow = thisGLHE.designFlow * rho;

    thisGLHE.myRespFactors->maxSimYears = 1;

    if constexpr (!Constant::python_cli_enabled) {
        // GTEST_SKIP() << "Skipping because PYTHON_CLI is disabled in CMake.";
        // Kinda silly, I don't really want the skip emitted every time.
    } else {
        thisGLHE.calcGFunctions(*state);

        Real64 constexpr tolerance = 0.1;

        // Test g-function values from GLHEPro
        EXPECT_NEAR(thisGLHE.interpGFunc(-11.939864), 0.37, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-11.802269), 0.48, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-11.664675), 0.59, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-11.52708), 0.69, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-11.389486), 0.79, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-11.251891), 0.89, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-11.114296), 0.99, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-10.976702), 1.09, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-10.839107), 1.18, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-10.701513), 1.27, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-10.563918), 1.36, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-10.426324), 1.44, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-10.288729), 1.53, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-10.151135), 1.61, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-10.01354), 1.69, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-9.875946), 1.77, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-9.738351), 1.85, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-9.600756), 1.93, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-9.463162), 2.00, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-9.325567), 2.08, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-9.187973), 2.15, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-9.050378), 2.23, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-8.912784), 2.30, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-8.775189), 2.37, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-8.637595), 2.45, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-8.5), 2.53, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-7.8), 2.90, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-7.2), 3.17, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-6.5), 3.52, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-5.9), 3.85, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-5.2), 4.37, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-4.5), 5.11, tolerance);
        EXPECT_NEAR(thisGLHE.interpGFunc(-3.963), 5.82, tolerance);
    }
}

TEST_F(EnergyPlusFixture, GHE_InterpTest1)
{
    std::shared_ptr<GroundHeatExchangers::GLHEResponseFactors> thisRF(new GroundHeatExchangers::GLHEResponseFactors());
    thisRF->GFNC = std::vector<Real64>(11);
    thisRF->LNTTS = std::vector<Real64>{-5.0, -4.0, -3.0, -2.0, -1.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0};
    thisRF->GFNC = {0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0};

    GroundHeatExchangers::GLHEVert thisGHE = GroundHeatExchangers::GLHEVert();
    thisGHE.myRespFactors = thisRF;

    double tolerance = 0.01;
    EXPECT_NEAR(thisGHE.interpGFunc(-10.0), -2.50, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-9.5), -2.25, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-9.0), -2.00, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-8.5), -1.75, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-8.0), -1.50, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-7.5), -1.25, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-7.0), -1.00, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-6.5), -0.75, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-6.0), -0.50, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-5.5), -0.25, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-5.0), 0.00, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-4.5), 0.25, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-4.0), 0.50, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-3.5), 0.75, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-3.0), 1.00, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-2.5), 1.25, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-2.0), 1.50, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-1.5), 1.75, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-1.0), 2.00, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(-0.5), 2.25, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(0.0), 2.50, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(0.5), 2.75, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(1.0), 3.00, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(1.5), 3.25, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(2.0), 3.50, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(2.5), 3.75, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(3.0), 4.00, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(3.5), 4.25, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(4.0), 4.50, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(4.5), 4.75, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(5.0), 5.00, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(5.5), 5.25, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(6.0), 5.50, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(6.5), 5.75, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(7.0), 6.00, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(7.5), 6.25, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(8.0), 6.50, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(8.5), 6.75, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(9.0), 7.00, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(9.5), 7.25, tolerance);
    EXPECT_NEAR(thisGHE.interpGFunc(10.0), 7.50, tolerance);
}

TEST_F(EnergyPlusFixture, GroundHeatExchangerTest_System_calc_pipe_conduction_resistance)
{
    using namespace DataSystemVariables;

    std::string const idf_objects =
        delimited_string({"Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
                          "    KATemps,                 !- Name",
                          "    1.8,                     !- Soil Thermal Conductivity {W/m-K}",
                          "    920,                     !- Soil Density {kg/m3}",
                          "    2200,                    !- Soil Specific Heat {J/kg-K}",
                          "    15.5,                    !- Average Soil Surface Temperature {C}",
                          "    3.2,                     !- Average Amplitude of Surface Temperature {deltaC}",
                          "    8;                       !- Phase Shift of Minimum Surface Temperature {days}",

                          "GroundHeatExchanger:Vertical:Properties,",
                          "    GHE-1 Props,        !- Name",
                          "    1,                  !- Depth of Top of Borehole {m}",
                          "    100,                !- Borehole Length {m}",
                          "    0.109982,           !- Borehole Diameter {m}",
                          "    0.744,              !- Grout Thermal Conductivity {W/m-K}",
                          "    3.90E+06,           !- Grout Thermal Heat Capacity {J/m3-K}",
                          "    0.389,              !- Pipe Thermal Conductivity {W/m-K}",
                          "    1.77E+06,           !- Pipe Thermal Heat Capacity {J/m3-K}",
                          "    0.0267,             !- Pipe Outer Diameter {m}",
                          "    0.00243,            !- Pipe Thickness {m}",
                          "    0.04556;            !- U-Tube Distance {m}",

                          "GroundHeatExchanger:Vertical:Array,",
                          "    GHE-Array,          !- Name",
                          "    GHE-1 Props,        !- GHE Properties",
                          "    2,                  !- Number of Boreholes in X Direction",
                          "    2,                  !- Number of Boreholes in Y Direction",
                          "    2;                  !- Borehole Spacing {m}",

                          "GroundHeatExchanger:System,",
                          "    Vertical GHE 1x4 Std,  !- Name",
                          "    GHLE Inlet,         !- Inlet Node Name",
                          "    GHLE Outlet,        !- Outlet Node Name",
                          "    0.0007571,          !- Design Flow Rate {m3/s}",
                          "    Site:GroundTemperature:Undisturbed:KusudaAchenbach,  !- Undisturbed Ground Temperature Model Type",
                          "    KATemps,            !- Undisturbed Ground Temperature Model Name",
                          "    2.423,              !- Ground Thermal Conductivity {W/m-K}",
                          "    2.343E+06,          !- Ground Thermal Heat Capacity {J/m3-K}",
                          "    ,                   !- Response Factors Object Name",
                          "    UHFCalc,            !- g-Function Calculation Method",
                          "    GHE-Array;          !- GHE Array Object Name"});

    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetGroundHeatExchangerInput(*state);

    auto &thisGLHE(state->dataGroundHeatExchanger->verticalGLHE[0]);

    Real64 constexpr tolerance = 0.00001;

    EXPECT_NEAR(thisGLHE.calcPipeConductionResistance(), 0.082204, tolerance);
}

TEST_F(EnergyPlusFixture, GroundHeatExchangerTest_System_friction_factor)
{
    using namespace DataSystemVariables;

    std::string const idf_objects =
        delimited_string({"Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
                          "    KATemps,                 !- Name",
                          "    1.8,                     !- Soil Thermal Conductivity {W/m-K}",
                          "    920,                     !- Soil Density {kg/m3}",
                          "    2200,                    !- Soil Specific Heat {J/kg-K}",
                          "    15.5,                    !- Average Soil Surface Temperature {C}",
                          "    3.2,                     !- Average Amplitude of Surface Temperature {deltaC}",
                          "    8;                       !- Phase Shift of Minimum Surface Temperature {days}",

                          "GroundHeatExchanger:Vertical:Properties,",
                          "    GHE-1 Props,        !- Name",
                          "    1,                  !- Depth of Top of Borehole {m}",
                          "    100,                !- Borehole Length {m}",
                          "    0.109982,           !- Borehole Diameter {m}",
                          "    0.744,              !- Grout Thermal Conductivity {W/m-K}",
                          "    3.90E+06,           !- Grout Thermal Heat Capacity {J/m3-K}",
                          "    0.389,              !- Pipe Thermal Conductivity {W/m-K}",
                          "    1.77E+06,           !- Pipe Thermal Heat Capacity {J/m3-K}",
                          "    0.0267,             !- Pipe Outer Diameter {m}",
                          "    0.00243,            !- Pipe Thickness {m}",
                          "    0.04556;            !- U-Tube Distance {m}",

                          "GroundHeatExchanger:Vertical:Array,",
                          "    GHE-Array,          !- Name",
                          "    GHE-1 Props,        !- GHE Properties",
                          "    2,                  !- Number of Boreholes in X Direction",
                          "    2,                  !- Number of Boreholes in Y Direction",
                          "    2;                  !- Borehole Spacing {m}",

                          "GroundHeatExchanger:System,",
                          "    Vertical GHE 1x4 Std,  !- Name",
                          "    GHLE Inlet,         !- Inlet Node Name",
                          "    GHLE Outlet,        !- Outlet Node Name",
                          "    0.0007571,          !- Design Flow Rate {m3/s}",
                          "    Site:GroundTemperature:Undisturbed:KusudaAchenbach,  !- Undisturbed Ground Temperature Model Type",
                          "    KATemps,            !- Undisturbed Ground Temperature Model Name",
                          "    2.423,              !- Ground Thermal Conductivity {W/m-K}",
                          "    2.343E+06,          !- Ground Thermal Heat Capacity {J/m3-K}",
                          "    ,                   !- Response Factors Object Name",
                          "    UHFCalc,            !- g-Function Calculation Method",
                          "    GHE-Array;          !- GHE Array Object Name"});

    // Setup
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    GetGroundHeatExchangerInput(*state);

    auto &thisGLHE(state->dataGroundHeatExchanger->verticalGLHE[0]);

    Real64 reynoldsNum;

    Real64 constexpr tolerance = 0.000001;

    // laminar tests
    reynoldsNum = 100;
    EXPECT_NEAR(thisGLHE.frictionFactor(reynoldsNum), 64.0 / reynoldsNum, tolerance);

    reynoldsNum = 1000;
    EXPECT_NEAR(thisGLHE.frictionFactor(reynoldsNum), 64.0 / reynoldsNum, tolerance);

    reynoldsNum = 1400;
    EXPECT_NEAR(thisGLHE.frictionFactor(reynoldsNum), 64.0 / reynoldsNum, tolerance);

    // transitional tests
    reynoldsNum = 2000;
    EXPECT_NEAR(thisGLHE.frictionFactor(reynoldsNum), 0.034003503, tolerance);

    reynoldsNum = 3000;
    EXPECT_NEAR(thisGLHE.frictionFactor(reynoldsNum), 0.033446219, tolerance);

    reynoldsNum = 4000;
    EXPECT_NEAR(thisGLHE.frictionFactor(reynoldsNum), 0.03895358, tolerance);

    // turbulent tests
    reynoldsNum = 5000;
    EXPECT_NEAR(thisGLHE.frictionFactor(reynoldsNum), pow(0.79 * log(reynoldsNum) - 1.64, -2.0), tolerance);

    reynoldsNum = 15000;
    EXPECT_NEAR(thisGLHE.frictionFactor(reynoldsNum), pow(0.79 * log(reynoldsNum) - 1.64, -2.0), tolerance);

    reynoldsNum = 25000;
    EXPECT_NEAR(thisGLHE.frictionFactor(reynoldsNum), pow(0.79 * log(reynoldsNum) - 1.64, -2.0), tolerance);
}

TEST_F(EnergyPlusFixture, GroundHeatExchangerTest_System_calc_pipe_convection_resistance)
{
    using namespace DataSystemVariables;

    std::string const idf_objects =
        delimited_string({"Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
                          "    KATemps,                 !- Name",
                          "    1.8,                     !- Soil Thermal Conductivity {W/m-K}",
                          "    920,                     !- Soil Density {kg/m3}",
                          "    2200,                    !- Soil Specific Heat {J/kg-K}",
                          "    15.5,                    !- Average Soil Surface Temperature {C}",
                          "    3.2,                     !- Average Amplitude of Surface Temperature {deltaC}",
                          "    8;                       !- Phase Shift of Minimum Surface Temperature {days}",

                          "GroundHeatExchanger:Vertical:Properties,",
                          "    GHE-1 Props,        !- Name",
                          "    1,                  !- Depth of Top of Borehole {m}",
                          "    100,                !- Borehole Length {m}",
                          "    0.109982,           !- Borehole Diameter {m}",
                          "    0.744,              !- Grout Thermal Conductivity {W/m-K}",
                          "    3.90E+06,           !- Grout Thermal Heat Capacity {J/m3-K}",
                          "    0.389,              !- Pipe Thermal Conductivity {W/m-K}",
                          "    1.77E+06,           !- Pipe Thermal Heat Capacity {J/m3-K}",
                          "    0.0267,             !- Pipe Outer Diameter {m}",
                          "    0.00243,            !- Pipe Thickness {m}",
                          "    0.04556;            !- U-Tube Distance {m}",

                          "GroundHeatExchanger:Vertical:Array,",
                          "    GHE-Array,          !- Name",
                          "    GHE-1 Props,        !- GHE Properties",
                          "    2,                  !- Number of Boreholes in X Direction",
                          "    2,                  !- Number of Boreholes in Y Direction",
                          "    2;                  !- Borehole Spacing {m}",

                          "GroundHeatExchanger:System,",
                          "    Vertical GHE 1x4 Std,  !- Name",
                          "    GHLE Inlet,         !- Inlet Node Name",
                          "    GHLE Outlet,        !- Outlet Node Name",
                          "    0.0007571,          !- Design Flow Rate {m3/s}",
                          "    Site:GroundTemperature:Undisturbed:KusudaAchenbach,  !- Undisturbed Ground Temperature Model Type",
                          "    KATemps,            !- Undisturbed Ground Temperature Model Name",
                          "    2.423,              !- Ground Thermal Conductivity {W/m-K}",
                          "    2.343E+06,          !- Ground Thermal Heat Capacity {J/m3-K}",
                          "    ,                   !- Response Factors Object Name",
                          "    UHFCalc,            !- g-Function Calculation Method",
                          "    GHE-Array;          !- GHE Array Object Name",

                          "Branch,",
                          "    Main Floor Cooling Condenser Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Coil:Cooling:WaterToAirHeatPump:EquationFit,  !- Component 1 Object Type",
                          "    Main Floor WAHP Cooling Coil,  !- Component 1 Name",
                          "    Main Floor WAHP Cooling Water Inlet Node,  !- Component 1 Inlet Node Name",
                          "    Main Floor WAHP Cooling Water Outlet Node;  !- Component 1 Outlet Node Name",

                          "Branch,",
                          "    Main Floor Heating Condenser Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Coil:Heating:WaterToAirHeatPump:EquationFit,  !- Component 1 Object Type",
                          "    Main Floor WAHP Heating Coil,  !- Component 1 Name",
                          "    Main Floor WAHP Heating Water Inlet Node,  !- Component 1 Inlet Node Name",
                          "    Main Floor WAHP Heating Water Outlet Node;  !- Component 1 Outlet Node Name",

                          "Branch,",
                          "    GHE-Vert Branch,         !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    GroundHeatExchanger:System,  !- Component 1 Object Type",
                          "    Vertical GHE 1x4 Std,    !- Component 1 Name",
                          "    GHLE Inlet,         !- Component 1 Inlet Node Name",
                          "    GHLE Outlet;        !- Component 1 Outlet Node Name",

                          "Branch,",
                          "    Ground Loop Supply Inlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pump:ConstantSpeed,      !- Component 1 Object Type",
                          "    Ground Loop Supply Pump, !- Component 1 Name",
                          "    Ground Loop Supply Inlet,!- Component 1 Inlet Node Name",
                          "    Ground Loop Pump Outlet; !- Component 1 Outlet Node Name",

                          "Branch,",
                          "    Ground Loop Supply Outlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    Ground Loop Supply Outlet Pipe,  !- Component 1 Name",
                          "    Ground Loop Supply Outlet Pipe Inlet,  !- Component 1 Inlet Node Name",
                          "    Ground Loop Supply Outlet;  !- Component 1 Outlet Node Name",

                          "Branch,",
                          "    Ground Loop Demand Inlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    Ground Loop Demand Inlet Pipe,  !- Component 1 Name",
                          "    Ground Loop Demand Inlet,!- Component 1 Inlet Node Name",
                          "    Ground Loop Demand Inlet Pipe Outlet;  !- Component 1 Outlet Node Name",

                          "Branch,",
                          "    Ground Loop Demand Bypass Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    Ground Loop Demand Side Bypass Pipe,  !- Component 1 Name",
                          "    Ground Loop Demand Bypass Inlet,  !- Component 1 Inlet Node Name",
                          "    Ground Loop Demand Bypass Outlet;  !- Component 1 Outlet Node Name",

                          "Branch,",
                          "    Ground Loop Demand Outlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    Ground Loop Demand Outlet Pipe,  !- Component 1 Name",
                          "    Ground Loop Demand Outlet Pipe Inlet,  !- Component 1 Inlet Node Name",
                          "    Ground Loop Demand Outlet;  !- Component 1 Outlet Node Name",

                          "BranchList,",
                          "    Ground Loop Supply Side Branches,  !- Name",
                          "    Ground Loop Supply Inlet Branch,  !- Branch 1 Name",
                          "    GHE-Vert Branch,         !- Branch 2 Name",
                          "    Ground Loop Supply Outlet Branch;  !- Branch 3 Name",

                          "BranchList,",
                          "    Ground Loop Demand Side Branches,  !- Name",
                          "    Ground Loop Demand Inlet Branch,  !- Branch 1 Name",
                          "    Main Floor Cooling Condenser Branch,  !- Branch 2 Name",
                          "    Main Floor Heating Condenser Branch,  !- Branch 3 Name",
                          "    Ground Loop Demand Bypass Branch,  !- Branch 4 Name",
                          "    Ground Loop Demand Outlet Branch;  !- Branch 5 Name",

                          "Connector:Splitter,",
                          "    Ground Loop Supply Splitter,  !- Name",
                          "    Ground Loop Supply Inlet Branch,  !- Inlet Branch Name",
                          "    GHE-Vert Branch;         !- Outlet Branch 1 Name",

                          "Connector:Splitter,",
                          "    Ground Loop Demand Splitter,  !- Name",
                          "    Ground Loop Demand Inlet Branch,  !- Inlet Branch Name",
                          "    Ground Loop Demand Bypass Branch,  !- Outlet Branch 1 Name",
                          "    Main Floor Cooling Condenser Branch,  !- Outlet Branch 2 Name",
                          "    Main Floor Heating Condenser Branch;  !- Outlet Branch 3 Name",

                          "Connector:Mixer,",
                          "    Ground Loop Supply Mixer,!- Name",
                          "    Ground Loop Supply Outlet Branch,  !- Outlet Branch Name",
                          "    GHE-Vert Branch;         !- Inlet Branch 1 Name",

                          "Connector:Mixer,",
                          "    Ground Loop Demand Mixer,!- Name",
                          "    Ground Loop Demand Outlet Branch,  !- Outlet Branch Name",
                          "    Ground Loop Demand Bypass Branch,  !- Inlet Branch 1 Name",
                          "    Main Floor Cooling Condenser Branch,  !- Inlet Branch 2 Name",
                          "    Main Floor Heating Condenser Branch;  !- Inlet Branch 3 Name",

                          "ConnectorList,",
                          "    Ground Loop Supply Side Connectors,  !- Name",
                          "    Connector:Splitter,      !- Connector 1 Object Type",
                          "    Ground Loop Supply Splitter,  !- Connector 1 Name",
                          "    Connector:Mixer,         !- Connector 2 Object Type",
                          "    Ground Loop Supply Mixer;!- Connector 2 Name",

                          "ConnectorList,",
                          "    Ground Loop Demand Side Connectors,  !- Name",
                          "    Connector:Splitter,      !- Connector 1 Object Type",
                          "    Ground Loop Demand Splitter,  !- Connector 1 Name",
                          "    Connector:Mixer,         !- Connector 2 Object Type",
                          "    Ground Loop Demand Mixer;!- Connector 2 Name",

                          "NodeList,",
                          "    Ground Loop Supply Setpoint Nodes,  !- Name",
                          "    GHLE Outlet,                        !- Node 1 Name",
                          "    Ground Loop Supply Outlet;  !- Node 2 Name",

                          "OutdoorAir:Node,",
                          "    Main Floor WAHP Outside Air Inlet,  !- Name",
                          "    -1;                      !- Height Above Ground {m}",

                          "Pipe:Adiabatic,",
                          "    Ground Loop Supply Outlet Pipe,  !- Name",
                          "    Ground Loop Supply Outlet Pipe Inlet,  !- Inlet Node Name",
                          "    Ground Loop Supply Outlet;  !- Outlet Node Name",

                          "Pipe:Adiabatic,",
                          "    Ground Loop Demand Inlet Pipe,  !- Name",
                          "    Ground Loop Demand Inlet,!- Inlet Node Name",
                          "    Ground Loop Demand Inlet Pipe Outlet;  !- Outlet Node Name",

                          "Pipe:Adiabatic,",
                          "    Ground Loop Demand Side Bypass Pipe,  !- Name",
                          "    Ground Loop Demand Bypass Inlet,  !- Inlet Node Name",
                          "    Ground Loop Demand Bypass Outlet;  !- Outlet Node Name",

                          "Pipe:Adiabatic,",
                          "    Ground Loop Demand Outlet Pipe,  !- Name",
                          "    Ground Loop Demand Outlet Pipe Inlet,  !- Inlet Node Name",
                          "    Ground Loop Demand Outlet;  !- Outlet Node Name",

                          "Pump:ConstantSpeed,",
                          "    Ground Loop Supply Pump, !- Name",
                          "    Ground Loop Supply Inlet,!- Inlet Node Name",
                          "    Ground Loop Pump Outlet, !- Outlet Node Name",
                          "    autosize,                !- Design Flow Rate {m3/s}",
                          "    179352,                  !- Design Pump Head {Pa}",
                          "    autosize,                !- Design Power Consumption {W}",
                          "    0.9,                     !- Motor Efficiency",
                          "    0,                       !- Fraction of Motor Inefficiencies to Fluid Stream",
                          "    Intermittent;            !- Pump Control Type",

                          "PlantLoop,",
                          "    Ground Loop Water Loop,  !- Name",
                          "    Water,                      !- Fluid Type",
                          "    ,                           !- User Defined Fluid Type",
                          "    Only Water Loop Operation,  !- Plant Equipment Operation Scheme Name",
                          "    Ground Loop Supply Outlet,  !- Loop Temperature Setpoint Node Name",
                          "    100,                     !- Maximum Loop Temperature {C}",
                          "    10,                      !- Minimum Loop Temperature {C}",
                          "    autosize,                !- Maximum Loop Flow Rate {m3/s}",
                          "    0,                       !- Minimum Loop Flow Rate {m3/s}",
                          "    autosize,                !- Plant Loop Volume {m3}",
                          "    Ground Loop Supply Inlet,!- Plant Side Inlet Node Name",
                          "    Ground Loop Supply Outlet,  !- Plant Side Outlet Node Name",
                          "    Ground Loop Supply Side Branches,  !- Plant Side Branch List Name",
                          "    Ground Loop Supply Side Connectors,  !- Plant Side Connector List Name",
                          "    Ground Loop Demand Inlet,!- Demand Side Inlet Node Name",
                          "    Ground Loop Demand Outlet,  !- Demand Side Outlet Node Name",
                          "    Ground Loop Demand Side Branches,  !- Demand Side Branch List Name",
                          "    Ground Loop Demand Side Connectors,  !- Demand Side Connector List Name",
                          "    SequentialLoad,          !- Load Distribution Scheme",
                          "    ,                        !- Availability Manager List Name",
                          "    DualSetPointDeadband;    !- Plant Loop Demand Calculation Scheme",

                          "PlantEquipmentList,",
                          "    Only Water Loop All Cooling Equipment,  !- Name",
                          "    GroundHeatExchanger:System,  !- Equipment 1 Object Type",
                          "    Vertical GHE 1x4 Std;    !- Equipment 1 Name",

                          "PlantEquipmentOperation:CoolingLoad,",
                          "    Only Water Loop Cool Operation All Hours,  !- Name",
                          "    0,                       !- Load Range 1 Lower Limit {W}",
                          "    1000000000000000,        !- Load Range 1 Upper Limit {W}",
                          "    Only Water Loop All Cooling Equipment;  !- Range 1 Equipment List Name",

                          "PlantEquipmentOperationSchemes,",
                          "    Only Water Loop Operation,  !- Name",
                          "    PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type",
                          "    Only Water Loop Cool Operation All Hours,  !- Control Scheme 1 Name",
                          "    HVACTemplate-Always 1;   !- Control Scheme 1 Schedule Name",

                          "SetpointManager:Scheduled:DualSetpoint,",
                          "    Ground Loop Temp Manager,!- Name",
                          "    Temperature,             !- Control Variable",
                          "    HVACTemplate-Always 34,  !- High Setpoint Schedule Name",
                          "    HVACTemplate-Always 20,  !- Low Setpoint Schedule Name",
                          "    Ground Loop Supply Setpoint Nodes;  !- Setpoint Node or NodeList Name",

                          "Schedule:Compact,",
                          "    HVACTemplate-Always 4,   !- Name",
                          "    HVACTemplate Any Number, !- Schedule Type Limits Name",
                          "    Through: 12/31,          !- Field 1",
                          "    For: AllDays,            !- Field 2",
                          "    Until: 24:00,4;          !- Field 3",

                          "Schedule:Compact,",
                          "    HVACTemplate-Always 34,  !- Name",
                          "    HVACTemplate Any Number, !- Schedule Type Limits Name",
                          "    Through: 12/31,          !- Field 1",
                          "    For: AllDays,            !- Field 2",
                          "    Until: 24:00,34;         !- Field 3",

                          "Schedule:Compact,",
                          "    HVACTemplate-Always 20,  !- Name",
                          "    HVACTemplate Any Number, !- Schedule Type Limits Name",
                          "    Through: 12/31,          !- Field 1",
                          "    For: AllDays,            !- Field 2",
                          "    Until: 24:00,20;         !- Field 3"});

    // Setup
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    PlantManager::GetPlantLoopData(*state);
    PlantManager::GetPlantInput(*state);
    PlantManager::SetupInitialPlantCallingOrder(*state);
    PlantManager::SetupBranchControlTypes(*state);

    auto &thisGLHE(state->dataGroundHeatExchanger->verticalGLHE[0]);
    thisGLHE.plantLoc.loopNum = 1;
    state->dataLoopNodes->Node(thisGLHE.inletNodeNum).Temp = 13.0;
    thisGLHE.designFlow = 0.000303 * 4;

    Real64 rho = 999.380058; // Density at 13 C using CoolProp
    thisGLHE.massFlowRate = thisGLHE.designFlow * rho;

    Real64 constexpr tolerance = 0.00001;

    // Turbulent
    EXPECT_NEAR(thisGLHE.calcPipeConvectionResistance(*state), 0.004453, tolerance);

    // Transitional
    thisGLHE.massFlowRate = thisGLHE.designFlow * rho / 4;
    EXPECT_NEAR(thisGLHE.calcPipeConvectionResistance(*state), 0.019185, tolerance);

    // Laminar
    thisGLHE.massFlowRate = thisGLHE.designFlow * rho / 10;
    EXPECT_NEAR(thisGLHE.calcPipeConvectionResistance(*state), 0.135556, tolerance);
}

TEST_F(EnergyPlusFixture, GroundHeatExchangerTest_System_calc_pipe_resistance)
{
    using namespace DataSystemVariables;

    std::string const idf_objects =
        delimited_string({"Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
                          "    KATemps,                 !- Name",
                          "    1.8,                     !- Soil Thermal Conductivity {W/m-K}",
                          "    920,                     !- Soil Density {kg/m3}",
                          "    2200,                    !- Soil Specific Heat {J/kg-K}",
                          "    15.5,                    !- Average Soil Surface Temperature {C}",
                          "    3.2,                     !- Average Amplitude of Surface Temperature {deltaC}",
                          "    8;                       !- Phase Shift of Minimum Surface Temperature {days}",

                          "GroundHeatExchanger:Vertical:Properties,",
                          "    GHE-1 Props,        !- Name",
                          "    1,                  !- Depth of Top of Borehole {m}",
                          "    100,                !- Borehole Length {m}",
                          "    0.109982,           !- Borehole Diameter {m}",
                          "    0.744,              !- Grout Thermal Conductivity {W/m-K}",
                          "    3.90E+06,           !- Grout Thermal Heat Capacity {J/m3-K}",
                          "    0.389,              !- Pipe Thermal Conductivity {W/m-K}",
                          "    1.77E+06,           !- Pipe Thermal Heat Capacity {J/m3-K}",
                          "    0.0267,             !- Pipe Outer Diameter {m}",
                          "    0.00243,            !- Pipe Thickness {m}",
                          "    0.04556;            !- U-Tube Distance {m}",

                          "GroundHeatExchanger:Vertical:Array,",
                          "    GHE-Array,          !- Name",
                          "    GHE-1 Props,        !- GHE Properties",
                          "    2,                  !- Number of Boreholes in X Direction",
                          "    2,                  !- Number of Boreholes in Y Direction",
                          "    2;                  !- Borehole Spacing {m}",

                          "GroundHeatExchanger:System,",
                          "    Vertical GHE 1x4 Std,  !- Name",
                          "    GHLE Inlet,         !- Inlet Node Name",
                          "    GHLE Outlet,        !- Outlet Node Name",
                          "    0.0007571,          !- Design Flow Rate {m3/s}",
                          "    Site:GroundTemperature:Undisturbed:KusudaAchenbach,  !- Undisturbed Ground Temperature Model Type",
                          "    KATemps,            !- Undisturbed Ground Temperature Model Name",
                          "    2.423,              !- Ground Thermal Conductivity {W/m-K}",
                          "    2.343E+06,          !- Ground Thermal Heat Capacity {J/m3-K}",
                          "    ,                   !- Response Factors Object Name",
                          "    UHFCalc,            !- g-Function Calculation Method",
                          "    GHE-Array;          !- GHE Array Object Name",

                          "Branch,",
                          "    Main Floor Cooling Condenser Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Coil:Cooling:WaterToAirHeatPump:EquationFit,  !- Component 1 Object Type",
                          "    Main Floor WAHP Cooling Coil,  !- Component 1 Name",
                          "    Main Floor WAHP Cooling Water Inlet Node,  !- Component 1 Inlet Node Name",
                          "    Main Floor WAHP Cooling Water Outlet Node;  !- Component 1 Outlet Node Name",

                          "Branch,",
                          "    Main Floor Heating Condenser Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Coil:Heating:WaterToAirHeatPump:EquationFit,  !- Component 1 Object Type",
                          "    Main Floor WAHP Heating Coil,  !- Component 1 Name",
                          "    Main Floor WAHP Heating Water Inlet Node,  !- Component 1 Inlet Node Name",
                          "    Main Floor WAHP Heating Water Outlet Node;  !- Component 1 Outlet Node Name",

                          "Branch,",
                          "    GHE-Vert Branch,         !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    GroundHeatExchanger:System,  !- Component 1 Object Type",
                          "    Vertical GHE 1x4 Std,    !- Component 1 Name",
                          "    GHLE Inlet,         !- Component 1 Inlet Node Name",
                          "    GHLE Outlet;        !- Component 1 Outlet Node Name",

                          "Branch,",
                          "    Ground Loop Supply Inlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pump:ConstantSpeed,      !- Component 1 Object Type",
                          "    Ground Loop Supply Pump, !- Component 1 Name",
                          "    Ground Loop Supply Inlet,!- Component 1 Inlet Node Name",
                          "    Ground Loop Pump Outlet; !- Component 1 Outlet Node Name",

                          "Branch,",
                          "    Ground Loop Supply Outlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    Ground Loop Supply Outlet Pipe,  !- Component 1 Name",
                          "    Ground Loop Supply Outlet Pipe Inlet,  !- Component 1 Inlet Node Name",
                          "    Ground Loop Supply Outlet;  !- Component 1 Outlet Node Name",

                          "Branch,",
                          "    Ground Loop Demand Inlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    Ground Loop Demand Inlet Pipe,  !- Component 1 Name",
                          "    Ground Loop Demand Inlet,!- Component 1 Inlet Node Name",
                          "    Ground Loop Demand Inlet Pipe Outlet;  !- Component 1 Outlet Node Name",

                          "Branch,",
                          "    Ground Loop Demand Bypass Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    Ground Loop Demand Side Bypass Pipe,  !- Component 1 Name",
                          "    Ground Loop Demand Bypass Inlet,  !- Component 1 Inlet Node Name",
                          "    Ground Loop Demand Bypass Outlet;  !- Component 1 Outlet Node Name",

                          "Branch,",
                          "    Ground Loop Demand Outlet Branch,  !- Name",
                          "    ,                        !- Pressure Drop Curve Name",
                          "    Pipe:Adiabatic,          !- Component 1 Object Type",
                          "    Ground Loop Demand Outlet Pipe,  !- Component 1 Name",
                          "    Ground Loop Demand Outlet Pipe Inlet,  !- Component 1 Inlet Node Name",
                          "    Ground Loop Demand Outlet;  !- Component 1 Outlet Node Name",

                          "BranchList,",
                          "    Ground Loop Supply Side Branches,  !- Name",
                          "    Ground Loop Supply Inlet Branch,  !- Branch 1 Name",
                          "    GHE-Vert Branch,         !- Branch 2 Name",
                          "    Ground Loop Supply Outlet Branch;  !- Branch 3 Name",

                          "BranchList,",
                          "    Ground Loop Demand Side Branches,  !- Name",
                          "    Ground Loop Demand Inlet Branch,  !- Branch 1 Name",
                          "    Main Floor Cooling Condenser Branch,  !- Branch 2 Name",
                          "    Main Floor Heating Condenser Branch,  !- Branch 3 Name",
                          "    Ground Loop Demand Bypass Branch,  !- Branch 4 Name",
                          "    Ground Loop Demand Outlet Branch;  !- Branch 5 Name",

                          "Connector:Splitter,",
                          "    Ground Loop Supply Splitter,  !- Name",
                          "    Ground Loop Supply Inlet Branch,  !- Inlet Branch Name",
                          "    GHE-Vert Branch;         !- Outlet Branch 1 Name",

                          "Connector:Splitter,",
                          "    Ground Loop Demand Splitter,  !- Name",
                          "    Ground Loop Demand Inlet Branch,  !- Inlet Branch Name",
                          "    Ground Loop Demand Bypass Branch,  !- Outlet Branch 1 Name",
                          "    Main Floor Cooling Condenser Branch,  !- Outlet Branch 2 Name",
                          "    Main Floor Heating Condenser Branch;  !- Outlet Branch 3 Name",

                          "Connector:Mixer,",
                          "    Ground Loop Supply Mixer,!- Name",
                          "    Ground Loop Supply Outlet Branch,  !- Outlet Branch Name",
                          "    GHE-Vert Branch;         !- Inlet Branch 1 Name",

                          "Connector:Mixer,",
                          "    Ground Loop Demand Mixer,!- Name",
                          "    Ground Loop Demand Outlet Branch,  !- Outlet Branch Name",
                          "    Ground Loop Demand Bypass Branch,  !- Inlet Branch 1 Name",
                          "    Main Floor Cooling Condenser Branch,  !- Inlet Branch 2 Name",
                          "    Main Floor Heating Condenser Branch;  !- Inlet Branch 3 Name",

                          "ConnectorList,",
                          "    Ground Loop Supply Side Connectors,  !- Name",
                          "    Connector:Splitter,      !- Connector 1 Object Type",
                          "    Ground Loop Supply Splitter,  !- Connector 1 Name",
                          "    Connector:Mixer,         !- Connector 2 Object Type",
                          "    Ground Loop Supply Mixer;!- Connector 2 Name",

                          "ConnectorList,",
                          "    Ground Loop Demand Side Connectors,  !- Name",
                          "    Connector:Splitter,      !- Connector 1 Object Type",
                          "    Ground Loop Demand Splitter,  !- Connector 1 Name",
                          "    Connector:Mixer,         !- Connector 2 Object Type",
                          "    Ground Loop Demand Mixer;!- Connector 2 Name",

                          "NodeList,",
                          "    Ground Loop Supply Setpoint Nodes,  !- Name",
                          "    GHLE Outlet,                        !- Node 1 Name",
                          "    Ground Loop Supply Outlet;  !- Node 2 Name",

                          "OutdoorAir:Node,",
                          "    Main Floor WAHP Outside Air Inlet,  !- Name",
                          "    -1;                      !- Height Above Ground {m}",

                          "Pipe:Adiabatic,",
                          "    Ground Loop Supply Outlet Pipe,  !- Name",
                          "    Ground Loop Supply Outlet Pipe Inlet,  !- Inlet Node Name",
                          "    Ground Loop Supply Outlet;  !- Outlet Node Name",

                          "Pipe:Adiabatic,",
                          "    Ground Loop Demand Inlet Pipe,  !- Name",
                          "    Ground Loop Demand Inlet,!- Inlet Node Name",
                          "    Ground Loop Demand Inlet Pipe Outlet;  !- Outlet Node Name",

                          "Pipe:Adiabatic,",
                          "    Ground Loop Demand Side Bypass Pipe,  !- Name",
                          "    Ground Loop Demand Bypass Inlet,  !- Inlet Node Name",
                          "    Ground Loop Demand Bypass Outlet;  !- Outlet Node Name",

                          "Pipe:Adiabatic,",
                          "    Ground Loop Demand Outlet Pipe,  !- Name",
                          "    Ground Loop Demand Outlet Pipe Inlet,  !- Inlet Node Name",
                          "    Ground Loop Demand Outlet;  !- Outlet Node Name",

                          "Pump:ConstantSpeed,",
                          "    Ground Loop Supply Pump, !- Name",
                          "    Ground Loop Supply Inlet,!- Inlet Node Name",
                          "    Ground Loop Pump Outlet, !- Outlet Node Name",
                          "    autosize,                !- Design Flow Rate {m3/s}",
                          "    179352,                  !- Design Pump Head {Pa}",
                          "    autosize,                !- Design Power Consumption {W}",
                          "    0.9,                     !- Motor Efficiency",
                          "    0,                       !- Fraction of Motor Inefficiencies to Fluid Stream",
                          "    Intermittent;            !- Pump Control Type",

                          "PlantLoop,",
                          "    Ground Loop Water Loop,  !- Name",
                          "    Water,                      !- Fluid Type",
                          "    ,                           !- User Defined Fluid Type",
                          "    Only Water Loop Operation,  !- Plant Equipment Operation Scheme Name",
                          "    Ground Loop Supply Outlet,  !- Loop Temperature Setpoint Node Name",
                          "    100,                     !- Maximum Loop Temperature {C}",
                          "    10,                      !- Minimum Loop Temperature {C}",
                          "    autosize,                !- Maximum Loop Flow Rate {m3/s}",
                          "    0,                       !- Minimum Loop Flow Rate {m3/s}",
                          "    autosize,                !- Plant Loop Volume {m3}",
                          "    Ground Loop Supply Inlet,!- Plant Side Inlet Node Name",
                          "    Ground Loop Supply Outlet,  !- Plant Side Outlet Node Name",
                          "    Ground Loop Supply Side Branches,  !- Plant Side Branch List Name",
                          "    Ground Loop Supply Side Connectors,  !- Plant Side Connector List Name",
                          "    Ground Loop Demand Inlet,!- Demand Side Inlet Node Name",
                          "    Ground Loop Demand Outlet,  !- Demand Side Outlet Node Name",
                          "    Ground Loop Demand Side Branches,  !- Demand Side Branch List Name",
                          "    Ground Loop Demand Side Connectors,  !- Demand Side Connector List Name",
                          "    SequentialLoad,          !- Load Distribution Scheme",
                          "    ,                        !- Availability Manager List Name",
                          "    DualSetPointDeadband;    !- Plant Loop Demand Calculation Scheme",

                          "PlantEquipmentList,",
                          "    Only Water Loop All Cooling Equipment,  !- Name",
                          "    GroundHeatExchanger:System,  !- Equipment 1 Object Type",
                          "    Vertical GHE 1x4 Std;    !- Equipment 1 Name",

                          "PlantEquipmentOperation:CoolingLoad,",
                          "    Only Water Loop Cool Operation All Hours,  !- Name",
                          "    0,                       !- Load Range 1 Lower Limit {W}",
                          "    1000000000000000,        !- Load Range 1 Upper Limit {W}",
                          "    Only Water Loop All Cooling Equipment;  !- Range 1 Equipment List Name",

                          "PlantEquipmentOperationSchemes,",
                          "    Only Water Loop Operation,  !- Name",
                          "    PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type",
                          "    Only Water Loop Cool Operation All Hours,  !- Control Scheme 1 Name",
                          "    HVACTemplate-Always 1;   !- Control Scheme 1 Schedule Name",

                          "SetpointManager:Scheduled:DualSetpoint,",
                          "    Ground Loop Temp Manager,!- Name",
                          "    Temperature,             !- Control Variable",
                          "    HVACTemplate-Always 34,  !- High Setpoint Schedule Name",
                          "    HVACTemplate-Always 20,  !- Low Setpoint Schedule Name",
                          "    Ground Loop Supply Setpoint Nodes;  !- Setpoint Node or NodeList Name",

                          "Schedule:Compact,",
                          "    HVACTemplate-Always 4,   !- Name",
                          "    HVACTemplate Any Number, !- Schedule Type Limits Name",
                          "    Through: 12/31,          !- Field 1",
                          "    For: AllDays,            !- Field 2",
                          "    Until: 24:00,4;          !- Field 3",

                          "Schedule:Compact,",
                          "    HVACTemplate-Always 34,  !- Name",
                          "    HVACTemplate Any Number, !- Schedule Type Limits Name",
                          "    Through: 12/31,          !- Field 1",
                          "    For: AllDays,            !- Field 2",
                          "    Until: 24:00,34;         !- Field 3",

                          "Schedule:Compact,",
                          "    HVACTemplate-Always 20,  !- Name",
                          "    HVACTemplate Any Number, !- Schedule Type Limits Name",
                          "    Through: 12/31,          !- Field 1",
                          "    For: AllDays,            !- Field 2",
                          "    Until: 24:00,20;         !- Field 3"});

    // Setup
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    PlantManager::GetPlantLoopData(*state);
    PlantManager::GetPlantInput(*state);
    PlantManager::SetupInitialPlantCallingOrder(*state);
    PlantManager::SetupBranchControlTypes(*state);

    auto &thisGLHE(state->dataGroundHeatExchanger->verticalGLHE[0]);
    thisGLHE.plantLoc.loopNum = 1;
    state->dataLoopNodes->Node(thisGLHE.inletNodeNum).Temp = 13.0;
    thisGLHE.designFlow = 0.000303 * 4;

    Real64 rho = 999.380058; // Density at 13 C using CoolProp
    thisGLHE.massFlowRate = thisGLHE.designFlow * rho;

    Real64 constexpr tolerance = 0.00001;

    EXPECT_NEAR(thisGLHE.calcPipeResistance(*state), 0.082204 + 0.004453, tolerance);
}

TEST_F(EnergyPlusFixture, GroundHeatExchangerTest_System_calcBHGroutResistance_1)
{
    using namespace DataSystemVariables;

    std::string const idf_objects = delimited_string({

        "Branch,",
        "    Main Floor Cooling Condenser Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Coil:Cooling:WaterToAirHeatPump:EquationFit,  !- Component 1 Object Type",
        "    Main Floor WAHP Cooling Coil,  !- Component 1 Name",
        "    Main Floor WAHP Cooling Water Inlet Node,  !- Component 1 Inlet Node Name",
        "    Main Floor WAHP Cooling Water Outlet Node;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Main Floor Heating Condenser Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Coil:Heating:WaterToAirHeatPump:EquationFit,  !- Component 1 Object Type",
        "    Main Floor WAHP Heating Coil,  !- Component 1 Name",
        "    Main Floor WAHP Heating Water Inlet Node,  !- Component 1 Inlet Node Name",
        "    Main Floor WAHP Heating Water Outlet Node;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    GHE-Vert Branch,         !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    GroundHeatExchanger:System,  !- Component 1 Object Type",
        "    Vertical GHE 1x4 Std,    !- Component 1 Name",
        "    GHLE Inlet,         !- Component 1 Inlet Node Name",
        "    GHLE Outlet;        !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Supply Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:ConstantSpeed,      !- Component 1 Object Type",
        "    Ground Loop Supply Pump, !- Component 1 Name",
        "    Ground Loop Supply Inlet,!- Component 1 Inlet Node Name",
        "    Ground Loop Pump Outlet; !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Supply Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Ground Loop Supply Outlet Pipe,  !- Component 1 Name",
        "    Ground Loop Supply Outlet Pipe Inlet,  !- Component 1 Inlet Node Name",
        "    Ground Loop Supply Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Demand Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Ground Loop Demand Inlet Pipe,  !- Component 1 Name",
        "    Ground Loop Demand Inlet,!- Component 1 Inlet Node Name",
        "    Ground Loop Demand Inlet Pipe Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Demand Bypass Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Ground Loop Demand Side Bypass Pipe,  !- Component 1 Name",
        "    Ground Loop Demand Bypass Inlet,  !- Component 1 Inlet Node Name",
        "    Ground Loop Demand Bypass Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Demand Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Ground Loop Demand Outlet Pipe,  !- Component 1 Name",
        "    Ground Loop Demand Outlet Pipe Inlet,  !- Component 1 Inlet Node Name",
        "    Ground Loop Demand Outlet;  !- Component 1 Outlet Node Name",

        "BranchList,",
        "    Ground Loop Supply Side Branches,  !- Name",
        "    Ground Loop Supply Inlet Branch,  !- Branch 1 Name",
        "    GHE-Vert Branch,         !- Branch 2 Name",
        "    Ground Loop Supply Outlet Branch;  !- Branch 3 Name",

        "BranchList,",
        "    Ground Loop Demand Side Branches,  !- Name",
        "    Ground Loop Demand Inlet Branch,  !- Branch 1 Name",
        "    Main Floor Cooling Condenser Branch,  !- Branch 2 Name",
        "    Main Floor Heating Condenser Branch,  !- Branch 3 Name",
        "    Ground Loop Demand Bypass Branch,  !- Branch 4 Name",
        "    Ground Loop Demand Outlet Branch;  !- Branch 5 Name",

        "Connector:Splitter,",
        "    Ground Loop Supply Splitter,  !- Name",
        "    Ground Loop Supply Inlet Branch,  !- Inlet Branch Name",
        "    GHE-Vert Branch;         !- Outlet Branch 1 Name",

        "Connector:Splitter,",
        "    Ground Loop Demand Splitter,  !- Name",
        "    Ground Loop Demand Inlet Branch,  !- Inlet Branch Name",
        "    Ground Loop Demand Bypass Branch,  !- Outlet Branch 1 Name",
        "    Main Floor Cooling Condenser Branch,  !- Outlet Branch 2 Name",
        "    Main Floor Heating Condenser Branch;  !- Outlet Branch 3 Name",

        "Connector:Mixer,",
        "    Ground Loop Supply Mixer,!- Name",
        "    Ground Loop Supply Outlet Branch,  !- Outlet Branch Name",
        "    GHE-Vert Branch;         !- Inlet Branch 1 Name",

        "Connector:Mixer,",
        "    Ground Loop Demand Mixer,!- Name",
        "    Ground Loop Demand Outlet Branch,  !- Outlet Branch Name",
        "    Ground Loop Demand Bypass Branch,  !- Inlet Branch 1 Name",
        "    Main Floor Cooling Condenser Branch,  !- Inlet Branch 2 Name",
        "    Main Floor Heating Condenser Branch;  !- Inlet Branch 3 Name",

        "ConnectorList,",
        "    Ground Loop Supply Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    Ground Loop Supply Splitter,  !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    Ground Loop Supply Mixer;!- Connector 2 Name",

        "ConnectorList,",
        "    Ground Loop Demand Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    Ground Loop Demand Splitter,  !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    Ground Loop Demand Mixer;!- Connector 2 Name",

        "NodeList,",
        "    Ground Loop Supply Setpoint Nodes,  !- Name",
        "    GHLE Outlet,                        !- Node 1 Name",
        "    Ground Loop Supply Outlet;  !- Node 2 Name",

        "OutdoorAir:Node,",
        "    Main Floor WAHP Outside Air Inlet,  !- Name",
        "    -1;                      !- Height Above Ground {m}",

        "Pipe:Adiabatic,",
        "    Ground Loop Supply Outlet Pipe,  !- Name",
        "    Ground Loop Supply Outlet Pipe Inlet,  !- Inlet Node Name",
        "    Ground Loop Supply Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Ground Loop Demand Inlet Pipe,  !- Name",
        "    Ground Loop Demand Inlet,!- Inlet Node Name",
        "    Ground Loop Demand Inlet Pipe Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Ground Loop Demand Side Bypass Pipe,  !- Name",
        "    Ground Loop Demand Bypass Inlet,  !- Inlet Node Name",
        "    Ground Loop Demand Bypass Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Ground Loop Demand Outlet Pipe,  !- Name",
        "    Ground Loop Demand Outlet Pipe Inlet,  !- Inlet Node Name",
        "    Ground Loop Demand Outlet;  !- Outlet Node Name",

        "Pump:ConstantSpeed,",
        "    Ground Loop Supply Pump, !- Name",
        "    Ground Loop Supply Inlet,!- Inlet Node Name",
        "    Ground Loop Pump Outlet, !- Outlet Node Name",
        "    autosize,                !- Design Flow Rate {m3/s}",
        "    179352,                  !- Design Pump Head {Pa}",
        "    autosize,                !- Design Power Consumption {W}",
        "    0.9,                     !- Motor Efficiency",
        "    0,                       !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    Intermittent;            !- Pump Control Type",

        "PlantLoop,",
        "    Ground Loop Water Loop,  !- Name",
        "    Water,                      !- Fluid Type",
        "    ,                           !- User Defined Fluid Type",
        "    Only Water Loop Operation,  !- Plant Equipment Operation Scheme Name",
        "    Ground Loop Supply Outlet,  !- Loop Temperature Setpoint Node Name",
        "    100,                     !- Maximum Loop Temperature {C}",
        "    10,                      !- Minimum Loop Temperature {C}",
        "    autosize,                !- Maximum Loop Flow Rate {m3/s}",
        "    0,                       !- Minimum Loop Flow Rate {m3/s}",
        "    autosize,                !- Plant Loop Volume {m3}",
        "    Ground Loop Supply Inlet,!- Plant Side Inlet Node Name",
        "    Ground Loop Supply Outlet,  !- Plant Side Outlet Node Name",
        "    Ground Loop Supply Side Branches,  !- Plant Side Branch List Name",
        "    Ground Loop Supply Side Connectors,  !- Plant Side Connector List Name",
        "    Ground Loop Demand Inlet,!- Demand Side Inlet Node Name",
        "    Ground Loop Demand Outlet,  !- Demand Side Outlet Node Name",
        "    Ground Loop Demand Side Branches,  !- Demand Side Branch List Name",
        "    Ground Loop Demand Side Connectors,  !- Demand Side Connector List Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ,                        !- Availability Manager List Name",
        "    DualSetPointDeadband;    !- Plant Loop Demand Calculation Scheme",

        "PlantEquipmentList,",
        "    Only Water Loop All Cooling Equipment,  !- Name",
        "    GroundHeatExchanger:System,  !- Equipment 1 Object Type",
        "    Vertical GHE 1x4 Std;    !- Equipment 1 Name",

        "PlantEquipmentOperation:CoolingLoad,",
        "    Only Water Loop Cool Operation All Hours,  !- Name",
        "    0,                       !- Load Range 1 Lower Limit {W}",
        "    1000000000000000,        !- Load Range 1 Upper Limit {W}",
        "    Only Water Loop All Cooling Equipment;  !- Range 1 Equipment List Name",

        "PlantEquipmentOperationSchemes,",
        "    Only Water Loop Operation,  !- Name",
        "    PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type",
        "    Only Water Loop Cool Operation All Hours,  !- Control Scheme 1 Name",
        "    HVACTemplate-Always 1;   !- Control Scheme 1 Schedule Name",

        "SetpointManager:Scheduled:DualSetpoint,",
        "    Ground Loop Temp Manager,!- Name",
        "    Temperature,             !- Control Variable",
        "    HVACTemplate-Always 34,  !- High Setpoint Schedule Name",
        "    HVACTemplate-Always 20,  !- Low Setpoint Schedule Name",
        "    Ground Loop Supply Setpoint Nodes;  !- Setpoint Node or NodeList Name",

        "Schedule:Compact,",
        "    HVACTemplate-Always 4,   !- Name",
        "    HVACTemplate Any Number, !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,4;          !- Field 3",

        "Schedule:Compact,",
        "    HVACTemplate-Always 34,  !- Name",
        "    HVACTemplate Any Number, !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,34;         !- Field 3",

        "Schedule:Compact,",
        "    HVACTemplate-Always 20,  !- Name",
        "    HVACTemplate Any Number, !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,20;         !- Field 3",

        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "    KATemps,                 !- Name",
        "    1.8,                     !- Soil Thermal Conductivity {W/m-K}",
        "    920,                     !- Soil Density {kg/m3}",
        "    2200,                    !- Soil Specific Heat {J/kg-K}",
        "    15.5,                    !- Average Soil Surface Temperature {C}",
        "    3.2,                     !- Average Amplitude of Surface Temperature {deltaC}",
        "    8;                       !- Phase Shift of Minimum Surface Temperature {days}",

        "GroundHeatExchanger:Vertical:Properties,",
        "    GHE-1 Props,        !- Name",
        "    1,                  !- Depth of Top of Borehole {m}",
        "    76.2,               !- Borehole Length {m}",
        "    0.096,              !- Borehole Diameter {m}",
        "    0.6,                !- Grout Thermal Conductivity {W/m-K}",
        "    1.0E+06,            !- Grout Thermal Heat Capacity {J/m3-K}",
        "    0.389,              !- Pipe Thermal Conductivity {W/m-K}",
        "    8E+05,              !- Pipe Thermal Heat Capacity {J/m3-K}",
        "    0.032,              !- Pipe Outer Diameter {m}",
        "    0.001627,           !- Pipe Thickness {m}",
        "    0.032;              !- U-Tube Distance {m}",

        "GroundHeatExchanger:Vertical:Array,",
        "    GHE-Array,          !- Name",
        "    GHE-1 Props,        !- GHE Properties",
        "    2,                  !- Number of Boreholes in X Direction",
        "    2,                  !- Number of Boreholes in Y Direction",
        "    2;                  !- Borehole Spacing {m}",

        "GroundHeatExchanger:System,",
        "    Vertical GHE 1x4 Std,  !- Name",
        "    GHLE Inlet,         !- Inlet Node Name",
        "    GHLE Outlet,        !- Outlet Node Name",
        "    0.1,                !- Design Flow Rate {m3/s}",
        "    Site:GroundTemperature:Undisturbed:KusudaAchenbach,  !- Undisturbed Ground Temperature Model Type",
        "    KATemps,            !- Undisturbed Ground Temperature Model Name",
        "    4.0,                !- Ground Thermal Conductivity {W/m-K}",
        "    2.4957E+06,         !- Ground Thermal Heat Capacity {J/m3-K}",
        "    ,                   !- Response Factors Object Name",
        "    UHFCalc,            !- g-Function Calculation Method",
        "    GHE-Array;          !- GHE Array Object Name"});

    // Setup
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    PlantManager::GetPlantLoopData(*state);
    PlantManager::GetPlantInput(*state);
    PlantManager::SetupInitialPlantCallingOrder(*state);
    PlantManager::SetupBranchControlTypes(*state);

    auto &thisGLHE(state->dataGroundHeatExchanger->verticalGLHE[0]);
    thisGLHE.plantLoc.loopNum = 1;
    state->dataLoopNodes->Node(thisGLHE.inletNodeNum).Temp = 20.0;
    thisGLHE.massFlowRate = 1;

    Real64 constexpr tolerance = 0.00001;

    // Flow rate and pipe thickness picked to fix pipe resistance at 0.05
    EXPECT_NEAR(thisGLHE.theta_1, 0.33333, tolerance);
    EXPECT_NEAR(thisGLHE.theta_2, 3.0, tolerance);
    EXPECT_NEAR(thisGLHE.theta_3, 1 / (2 * thisGLHE.theta_1 * thisGLHE.theta_2), tolerance);
    EXPECT_NEAR(thisGLHE.sigma, (thisGLHE.grout.k - thisGLHE.soil.k) / (thisGLHE.grout.k + thisGLHE.soil.k), tolerance);
    EXPECT_NEAR(thisGLHE.calcBHGroutResistance(*state), 0.17701, tolerance);
}

TEST_F(EnergyPlusFixture, GroundHeatExchangerTest_System_calcBHGroutResistance_2)
{
    using namespace DataSystemVariables;

    std::string const idf_objects = delimited_string({

        "Branch,",
        "    Main Floor Cooling Condenser Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Coil:Cooling:WaterToAirHeatPump:EquationFit,  !- Component 1 Object Type",
        "    Main Floor WAHP Cooling Coil,  !- Component 1 Name",
        "    Main Floor WAHP Cooling Water Inlet Node,  !- Component 1 Inlet Node Name",
        "    Main Floor WAHP Cooling Water Outlet Node;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Main Floor Heating Condenser Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Coil:Heating:WaterToAirHeatPump:EquationFit,  !- Component 1 Object Type",
        "    Main Floor WAHP Heating Coil,  !- Component 1 Name",
        "    Main Floor WAHP Heating Water Inlet Node,  !- Component 1 Inlet Node Name",
        "    Main Floor WAHP Heating Water Outlet Node;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    GHE-Vert Branch,         !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    GroundHeatExchanger:System,  !- Component 1 Object Type",
        "    Vertical GHE 1x4 Std,    !- Component 1 Name",
        "    GHLE Inlet,         !- Component 1 Inlet Node Name",
        "    GHLE Outlet;        !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Supply Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:ConstantSpeed,      !- Component 1 Object Type",
        "    Ground Loop Supply Pump, !- Component 1 Name",
        "    Ground Loop Supply Inlet,!- Component 1 Inlet Node Name",
        "    Ground Loop Pump Outlet; !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Supply Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Ground Loop Supply Outlet Pipe,  !- Component 1 Name",
        "    Ground Loop Supply Outlet Pipe Inlet,  !- Component 1 Inlet Node Name",
        "    Ground Loop Supply Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Demand Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Ground Loop Demand Inlet Pipe,  !- Component 1 Name",
        "    Ground Loop Demand Inlet,!- Component 1 Inlet Node Name",
        "    Ground Loop Demand Inlet Pipe Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Demand Bypass Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Ground Loop Demand Side Bypass Pipe,  !- Component 1 Name",
        "    Ground Loop Demand Bypass Inlet,  !- Component 1 Inlet Node Name",
        "    Ground Loop Demand Bypass Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Demand Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Ground Loop Demand Outlet Pipe,  !- Component 1 Name",
        "    Ground Loop Demand Outlet Pipe Inlet,  !- Component 1 Inlet Node Name",
        "    Ground Loop Demand Outlet;  !- Component 1 Outlet Node Name",

        "BranchList,",
        "    Ground Loop Supply Side Branches,  !- Name",
        "    Ground Loop Supply Inlet Branch,  !- Branch 1 Name",
        "    GHE-Vert Branch,         !- Branch 2 Name",
        "    Ground Loop Supply Outlet Branch;  !- Branch 3 Name",

        "BranchList,",
        "    Ground Loop Demand Side Branches,  !- Name",
        "    Ground Loop Demand Inlet Branch,  !- Branch 1 Name",
        "    Main Floor Cooling Condenser Branch,  !- Branch 2 Name",
        "    Main Floor Heating Condenser Branch,  !- Branch 3 Name",
        "    Ground Loop Demand Bypass Branch,  !- Branch 4 Name",
        "    Ground Loop Demand Outlet Branch;  !- Branch 5 Name",

        "Connector:Splitter,",
        "    Ground Loop Supply Splitter,  !- Name",
        "    Ground Loop Supply Inlet Branch,  !- Inlet Branch Name",
        "    GHE-Vert Branch;         !- Outlet Branch 1 Name",

        "Connector:Splitter,",
        "    Ground Loop Demand Splitter,  !- Name",
        "    Ground Loop Demand Inlet Branch,  !- Inlet Branch Name",
        "    Ground Loop Demand Bypass Branch,  !- Outlet Branch 1 Name",
        "    Main Floor Cooling Condenser Branch,  !- Outlet Branch 2 Name",
        "    Main Floor Heating Condenser Branch;  !- Outlet Branch 3 Name",

        "Connector:Mixer,",
        "    Ground Loop Supply Mixer,!- Name",
        "    Ground Loop Supply Outlet Branch,  !- Outlet Branch Name",
        "    GHE-Vert Branch;         !- Inlet Branch 1 Name",

        "Connector:Mixer,",
        "    Ground Loop Demand Mixer,!- Name",
        "    Ground Loop Demand Outlet Branch,  !- Outlet Branch Name",
        "    Ground Loop Demand Bypass Branch,  !- Inlet Branch 1 Name",
        "    Main Floor Cooling Condenser Branch,  !- Inlet Branch 2 Name",
        "    Main Floor Heating Condenser Branch;  !- Inlet Branch 3 Name",

        "ConnectorList,",
        "    Ground Loop Supply Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    Ground Loop Supply Splitter,  !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    Ground Loop Supply Mixer;!- Connector 2 Name",

        "ConnectorList,",
        "    Ground Loop Demand Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    Ground Loop Demand Splitter,  !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    Ground Loop Demand Mixer;!- Connector 2 Name",

        "NodeList,",
        "    Ground Loop Supply Setpoint Nodes,  !- Name",
        "    GHLE Outlet,                        !- Node 1 Name",
        "    Ground Loop Supply Outlet;  !- Node 2 Name",

        "OutdoorAir:Node,",
        "    Main Floor WAHP Outside Air Inlet,  !- Name",
        "    -1;                      !- Height Above Ground {m}",

        "Pipe:Adiabatic,",
        "    Ground Loop Supply Outlet Pipe,  !- Name",
        "    Ground Loop Supply Outlet Pipe Inlet,  !- Inlet Node Name",
        "    Ground Loop Supply Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Ground Loop Demand Inlet Pipe,  !- Name",
        "    Ground Loop Demand Inlet,!- Inlet Node Name",
        "    Ground Loop Demand Inlet Pipe Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Ground Loop Demand Side Bypass Pipe,  !- Name",
        "    Ground Loop Demand Bypass Inlet,  !- Inlet Node Name",
        "    Ground Loop Demand Bypass Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Ground Loop Demand Outlet Pipe,  !- Name",
        "    Ground Loop Demand Outlet Pipe Inlet,  !- Inlet Node Name",
        "    Ground Loop Demand Outlet;  !- Outlet Node Name",

        "Pump:ConstantSpeed,",
        "    Ground Loop Supply Pump, !- Name",
        "    Ground Loop Supply Inlet,!- Inlet Node Name",
        "    Ground Loop Pump Outlet, !- Outlet Node Name",
        "    autosize,                !- Design Flow Rate {m3/s}",
        "    179352,                  !- Design Pump Head {Pa}",
        "    autosize,                !- Design Power Consumption {W}",
        "    0.9,                     !- Motor Efficiency",
        "    0,                       !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    Intermittent;            !- Pump Control Type",

        "PlantLoop,",
        "    Ground Loop Water Loop,  !- Name",
        "    Water,                      !- Fluid Type",
        "    ,                           !- User Defined Fluid Type",
        "    Only Water Loop Operation,  !- Plant Equipment Operation Scheme Name",
        "    Ground Loop Supply Outlet,  !- Loop Temperature Setpoint Node Name",
        "    100,                     !- Maximum Loop Temperature {C}",
        "    10,                      !- Minimum Loop Temperature {C}",
        "    autosize,                !- Maximum Loop Flow Rate {m3/s}",
        "    0,                       !- Minimum Loop Flow Rate {m3/s}",
        "    autosize,                !- Plant Loop Volume {m3}",
        "    Ground Loop Supply Inlet,!- Plant Side Inlet Node Name",
        "    Ground Loop Supply Outlet,  !- Plant Side Outlet Node Name",
        "    Ground Loop Supply Side Branches,  !- Plant Side Branch List Name",
        "    Ground Loop Supply Side Connectors,  !- Plant Side Connector List Name",
        "    Ground Loop Demand Inlet,!- Demand Side Inlet Node Name",
        "    Ground Loop Demand Outlet,  !- Demand Side Outlet Node Name",
        "    Ground Loop Demand Side Branches,  !- Demand Side Branch List Name",
        "    Ground Loop Demand Side Connectors,  !- Demand Side Connector List Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ,                        !- Availability Manager List Name",
        "    DualSetPointDeadband;    !- Plant Loop Demand Calculation Scheme",

        "PlantEquipmentList,",
        "    Only Water Loop All Cooling Equipment,  !- Name",
        "    GroundHeatExchanger:System,  !- Equipment 1 Object Type",
        "    Vertical GHE 1x4 Std;    !- Equipment 1 Name",

        "PlantEquipmentOperation:CoolingLoad,",
        "    Only Water Loop Cool Operation All Hours,  !- Name",
        "    0,                       !- Load Range 1 Lower Limit {W}",
        "    1000000000000000,        !- Load Range 1 Upper Limit {W}",
        "    Only Water Loop All Cooling Equipment;  !- Range 1 Equipment List Name",

        "PlantEquipmentOperationSchemes,",
        "    Only Water Loop Operation,  !- Name",
        "    PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type",
        "    Only Water Loop Cool Operation All Hours,  !- Control Scheme 1 Name",
        "    HVACTemplate-Always 1;   !- Control Scheme 1 Schedule Name",

        "SetpointManager:Scheduled:DualSetpoint,",
        "    Ground Loop Temp Manager,!- Name",
        "    Temperature,             !- Control Variable",
        "    HVACTemplate-Always 34,  !- High Setpoint Schedule Name",
        "    HVACTemplate-Always 20,  !- Low Setpoint Schedule Name",
        "    Ground Loop Supply Setpoint Nodes;  !- Setpoint Node or NodeList Name",

        "Schedule:Compact,",
        "    HVACTemplate-Always 4,   !- Name",
        "    HVACTemplate Any Number, !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,4;          !- Field 3",

        "Schedule:Compact,",
        "    HVACTemplate-Always 34,  !- Name",
        "    HVACTemplate Any Number, !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,34;         !- Field 3",

        "Schedule:Compact,",
        "    HVACTemplate-Always 20,  !- Name",
        "    HVACTemplate Any Number, !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,20;         !- Field 3",

        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "    KATemps,                 !- Name",
        "    1.8,                     !- Soil Thermal Conductivity {W/m-K}",
        "    920,                     !- Soil Density {kg/m3}",
        "    2200,                    !- Soil Specific Heat {J/kg-K}",
        "    15.5,                    !- Average Soil Surface Temperature {C}",
        "    3.2,                     !- Average Amplitude of Surface Temperature {deltaC}",
        "    8;                       !- Phase Shift of Minimum Surface Temperature {days}",

        "GroundHeatExchanger:Vertical:Properties,",
        "    GHE-1 Props,        !- Name",
        "    1,                  !- Depth of Top of Borehole {m}",
        "    76.2,               !- Borehole Length {m}",
        "    0.096,              !- Borehole Diameter {m}",
        "    0.6,                !- Grout Thermal Conductivity {W/m-K}",
        "    1.0E+06,            !- Grout Thermal Heat Capacity {J/m3-K}",
        "    0.389,              !- Pipe Thermal Conductivity {W/m-K}",
        "    8E+05,              !- Pipe Thermal Heat Capacity {J/m3-K}",
        "    0.032,              !- Pipe Outer Diameter {m}",
        "    0.001627,           !- Pipe Thickness {m}",
        "    0.0426666667;       !- U-Tube Distance {m}",

        "GroundHeatExchanger:Vertical:Array,",
        "    GHE-Array,          !- Name",
        "    GHE-1 Props,        !- GHE Properties",
        "    2,                  !- Number of Boreholes in X Direction",
        "    2,                  !- Number of Boreholes in Y Direction",
        "    2;                  !- Borehole Spacing {m}",

        "GroundHeatExchanger:System,",
        "    Vertical GHE 1x4 Std,  !- Name",
        "    GHLE Inlet,         !- Inlet Node Name",
        "    GHLE Outlet,        !- Outlet Node Name",
        "    0.1,                !- Design Flow Rate {m3/s}",
        "    Site:GroundTemperature:Undisturbed:KusudaAchenbach,  !- Undisturbed Ground Temperature Model Type",
        "    KATemps,            !- Undisturbed Ground Temperature Model Name",
        "    1.0,                !- Ground Thermal Conductivity {W/m-K}",
        "    2.4957E+06,         !- Ground Thermal Heat Capacity {J/m3-K}",
        "    ,                   !- Response Factors Object Name",
        "    UHFCalc,            !- g-Function Calculation Method",
        "    GHE-Array;          !- GHE Array Object Name"});

    // Setup
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    PlantManager::GetPlantLoopData(*state);
    PlantManager::GetPlantInput(*state);
    PlantManager::SetupInitialPlantCallingOrder(*state);
    PlantManager::SetupBranchControlTypes(*state);

    auto &thisGLHE(state->dataGroundHeatExchanger->verticalGLHE[0]);
    thisGLHE.plantLoc.loopNum = 1;
    state->dataLoopNodes->Node(thisGLHE.inletNodeNum).Temp = 20.0;
    thisGLHE.massFlowRate = 1;

    Real64 constexpr tolerance = 0.00001;

    // Flow rate and pipe thickness picked to fix pipe resistance at 0.05
    EXPECT_NEAR(thisGLHE.theta_1, 0.44444, tolerance);
    EXPECT_NEAR(thisGLHE.theta_2, 3.0, tolerance);
    EXPECT_NEAR(thisGLHE.theta_3, 1 / (2 * thisGLHE.theta_1 * thisGLHE.theta_2), tolerance);
    EXPECT_NEAR(thisGLHE.sigma, (thisGLHE.grout.k - thisGLHE.soil.k) / (thisGLHE.grout.k + thisGLHE.soil.k), tolerance);
    EXPECT_NEAR(thisGLHE.calcBHGroutResistance(*state), 0.14724, tolerance);
}

TEST_F(EnergyPlusFixture, GroundHeatExchangerTest_System_calcBHGroutResistance_3)
{
    using namespace DataSystemVariables;

    std::string const idf_objects = delimited_string({

        "Branch,",
        "    Main Floor Cooling Condenser Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Coil:Cooling:WaterToAirHeatPump:EquationFit,  !- Component 1 Object Type",
        "    Main Floor WAHP Cooling Coil,  !- Component 1 Name",
        "    Main Floor WAHP Cooling Water Inlet Node,  !- Component 1 Inlet Node Name",
        "    Main Floor WAHP Cooling Water Outlet Node;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Main Floor Heating Condenser Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Coil:Heating:WaterToAirHeatPump:EquationFit,  !- Component 1 Object Type",
        "    Main Floor WAHP Heating Coil,  !- Component 1 Name",
        "    Main Floor WAHP Heating Water Inlet Node,  !- Component 1 Inlet Node Name",
        "    Main Floor WAHP Heating Water Outlet Node;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    GHE-Vert Branch,         !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    GroundHeatExchanger:System,  !- Component 1 Object Type",
        "    Vertical GHE 1x4 Std,    !- Component 1 Name",
        "    GHLE Inlet,         !- Component 1 Inlet Node Name",
        "    GHLE Outlet;        !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Supply Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:ConstantSpeed,      !- Component 1 Object Type",
        "    Ground Loop Supply Pump, !- Component 1 Name",
        "    Ground Loop Supply Inlet,!- Component 1 Inlet Node Name",
        "    Ground Loop Pump Outlet; !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Supply Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Ground Loop Supply Outlet Pipe,  !- Component 1 Name",
        "    Ground Loop Supply Outlet Pipe Inlet,  !- Component 1 Inlet Node Name",
        "    Ground Loop Supply Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Demand Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Ground Loop Demand Inlet Pipe,  !- Component 1 Name",
        "    Ground Loop Demand Inlet,!- Component 1 Inlet Node Name",
        "    Ground Loop Demand Inlet Pipe Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Demand Bypass Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Ground Loop Demand Side Bypass Pipe,  !- Component 1 Name",
        "    Ground Loop Demand Bypass Inlet,  !- Component 1 Inlet Node Name",
        "    Ground Loop Demand Bypass Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Demand Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Ground Loop Demand Outlet Pipe,  !- Component 1 Name",
        "    Ground Loop Demand Outlet Pipe Inlet,  !- Component 1 Inlet Node Name",
        "    Ground Loop Demand Outlet;  !- Component 1 Outlet Node Name",

        "BranchList,",
        "    Ground Loop Supply Side Branches,  !- Name",
        "    Ground Loop Supply Inlet Branch,  !- Branch 1 Name",
        "    GHE-Vert Branch,         !- Branch 2 Name",
        "    Ground Loop Supply Outlet Branch;  !- Branch 3 Name",

        "BranchList,",
        "    Ground Loop Demand Side Branches,  !- Name",
        "    Ground Loop Demand Inlet Branch,  !- Branch 1 Name",
        "    Main Floor Cooling Condenser Branch,  !- Branch 2 Name",
        "    Main Floor Heating Condenser Branch,  !- Branch 3 Name",
        "    Ground Loop Demand Bypass Branch,  !- Branch 4 Name",
        "    Ground Loop Demand Outlet Branch;  !- Branch 5 Name",

        "Connector:Splitter,",
        "    Ground Loop Supply Splitter,  !- Name",
        "    Ground Loop Supply Inlet Branch,  !- Inlet Branch Name",
        "    GHE-Vert Branch;         !- Outlet Branch 1 Name",

        "Connector:Splitter,",
        "    Ground Loop Demand Splitter,  !- Name",
        "    Ground Loop Demand Inlet Branch,  !- Inlet Branch Name",
        "    Ground Loop Demand Bypass Branch,  !- Outlet Branch 1 Name",
        "    Main Floor Cooling Condenser Branch,  !- Outlet Branch 2 Name",
        "    Main Floor Heating Condenser Branch;  !- Outlet Branch 3 Name",

        "Connector:Mixer,",
        "    Ground Loop Supply Mixer,!- Name",
        "    Ground Loop Supply Outlet Branch,  !- Outlet Branch Name",
        "    GHE-Vert Branch;         !- Inlet Branch 1 Name",

        "Connector:Mixer,",
        "    Ground Loop Demand Mixer,!- Name",
        "    Ground Loop Demand Outlet Branch,  !- Outlet Branch Name",
        "    Ground Loop Demand Bypass Branch,  !- Inlet Branch 1 Name",
        "    Main Floor Cooling Condenser Branch,  !- Inlet Branch 2 Name",
        "    Main Floor Heating Condenser Branch;  !- Inlet Branch 3 Name",

        "ConnectorList,",
        "    Ground Loop Supply Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    Ground Loop Supply Splitter,  !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    Ground Loop Supply Mixer;!- Connector 2 Name",

        "ConnectorList,",
        "    Ground Loop Demand Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    Ground Loop Demand Splitter,  !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    Ground Loop Demand Mixer;!- Connector 2 Name",

        "NodeList,",
        "    Ground Loop Supply Setpoint Nodes,  !- Name",
        "    GHLE Outlet,                        !- Node 1 Name",
        "    Ground Loop Supply Outlet;  !- Node 2 Name",

        "OutdoorAir:Node,",
        "    Main Floor WAHP Outside Air Inlet,  !- Name",
        "    -1;                      !- Height Above Ground {m}",

        "Pipe:Adiabatic,",
        "    Ground Loop Supply Outlet Pipe,  !- Name",
        "    Ground Loop Supply Outlet Pipe Inlet,  !- Inlet Node Name",
        "    Ground Loop Supply Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Ground Loop Demand Inlet Pipe,  !- Name",
        "    Ground Loop Demand Inlet,!- Inlet Node Name",
        "    Ground Loop Demand Inlet Pipe Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Ground Loop Demand Side Bypass Pipe,  !- Name",
        "    Ground Loop Demand Bypass Inlet,  !- Inlet Node Name",
        "    Ground Loop Demand Bypass Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Ground Loop Demand Outlet Pipe,  !- Name",
        "    Ground Loop Demand Outlet Pipe Inlet,  !- Inlet Node Name",
        "    Ground Loop Demand Outlet;  !- Outlet Node Name",

        "Pump:ConstantSpeed,",
        "    Ground Loop Supply Pump, !- Name",
        "    Ground Loop Supply Inlet,!- Inlet Node Name",
        "    Ground Loop Pump Outlet, !- Outlet Node Name",
        "    autosize,                !- Design Flow Rate {m3/s}",
        "    179352,                  !- Design Pump Head {Pa}",
        "    autosize,                !- Design Power Consumption {W}",
        "    0.9,                     !- Motor Efficiency",
        "    0,                       !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    Intermittent;            !- Pump Control Type",

        "PlantLoop,",
        "    Ground Loop Water Loop,  !- Name",
        "    Water,                      !- Fluid Type",
        "    ,                           !- User Defined Fluid Type",
        "    Only Water Loop Operation,  !- Plant Equipment Operation Scheme Name",
        "    Ground Loop Supply Outlet,  !- Loop Temperature Setpoint Node Name",
        "    100,                     !- Maximum Loop Temperature {C}",
        "    10,                      !- Minimum Loop Temperature {C}",
        "    autosize,                !- Maximum Loop Flow Rate {m3/s}",
        "    0,                       !- Minimum Loop Flow Rate {m3/s}",
        "    autosize,                !- Plant Loop Volume {m3}",
        "    Ground Loop Supply Inlet,!- Plant Side Inlet Node Name",
        "    Ground Loop Supply Outlet,  !- Plant Side Outlet Node Name",
        "    Ground Loop Supply Side Branches,  !- Plant Side Branch List Name",
        "    Ground Loop Supply Side Connectors,  !- Plant Side Connector List Name",
        "    Ground Loop Demand Inlet,!- Demand Side Inlet Node Name",
        "    Ground Loop Demand Outlet,  !- Demand Side Outlet Node Name",
        "    Ground Loop Demand Side Branches,  !- Demand Side Branch List Name",
        "    Ground Loop Demand Side Connectors,  !- Demand Side Connector List Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ,                        !- Availability Manager List Name",
        "    DualSetPointDeadband;    !- Plant Loop Demand Calculation Scheme",

        "PlantEquipmentList,",
        "    Only Water Loop All Cooling Equipment,  !- Name",
        "    GroundHeatExchanger:System,  !- Equipment 1 Object Type",
        "    Vertical GHE 1x4 Std;    !- Equipment 1 Name",

        "PlantEquipmentOperation:CoolingLoad,",
        "    Only Water Loop Cool Operation All Hours,  !- Name",
        "    0,                       !- Load Range 1 Lower Limit {W}",
        "    1000000000000000,        !- Load Range 1 Upper Limit {W}",
        "    Only Water Loop All Cooling Equipment;  !- Range 1 Equipment List Name",

        "PlantEquipmentOperationSchemes,",
        "    Only Water Loop Operation,  !- Name",
        "    PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type",
        "    Only Water Loop Cool Operation All Hours,  !- Control Scheme 1 Name",
        "    HVACTemplate-Always 1;   !- Control Scheme 1 Schedule Name",

        "SetpointManager:Scheduled:DualSetpoint,",
        "    Ground Loop Temp Manager,!- Name",
        "    Temperature,             !- Control Variable",
        "    HVACTemplate-Always 34,  !- High Setpoint Schedule Name",
        "    HVACTemplate-Always 20,  !- Low Setpoint Schedule Name",
        "    Ground Loop Supply Setpoint Nodes;  !- Setpoint Node or NodeList Name",

        "Schedule:Compact,",
        "    HVACTemplate-Always 4,   !- Name",
        "    HVACTemplate Any Number, !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,4;          !- Field 3",

        "Schedule:Compact,",
        "    HVACTemplate-Always 34,  !- Name",
        "    HVACTemplate Any Number, !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,34;         !- Field 3",

        "Schedule:Compact,",
        "    HVACTemplate-Always 20,  !- Name",
        "    HVACTemplate Any Number, !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,20;         !- Field 3",

        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "    KATemps,                 !- Name",
        "    1.8,                     !- Soil Thermal Conductivity {W/m-K}",
        "    920,                     !- Soil Density {kg/m3}",
        "    2200,                    !- Soil Specific Heat {J/kg-K}",
        "    15.5,                    !- Average Soil Surface Temperature {C}",
        "    3.2,                     !- Average Amplitude of Surface Temperature {deltaC}",
        "    8;                       !- Phase Shift of Minimum Surface Temperature {days}",

        "GroundHeatExchanger:Vertical:Properties,",
        "    GHE-1 Props,        !- Name",
        "    1,                  !- Depth of Top of Borehole {m}",
        "    76.2,               !- Borehole Length {m}",
        "    0.288,              !- Borehole Diameter {m}",
        "    1.8,                !- Grout Thermal Conductivity {W/m-K}",
        "    1.0E+06,            !- Grout Thermal Heat Capacity {J/m3-K}",
        "    0.389,              !- Pipe Thermal Conductivity {W/m-K}",
        "    8E+05,              !- Pipe Thermal Heat Capacity {J/m3-K}",
        "    0.032,              !- Pipe Outer Diameter {m}",
        "    0.001627,           !- Pipe Thickness {m}",
        "    0.10666667;        !- U-Tube Distance {m}",

        "GroundHeatExchanger:Vertical:Array,",
        "    GHE-Array,          !- Name",
        "    GHE-1 Props,        !- GHE Properties",
        "    2,                  !- Number of Boreholes in X Direction",
        "    2,                  !- Number of Boreholes in Y Direction",
        "    2;                  !- Borehole Spacing {m}",

        "GroundHeatExchanger:System,",
        "    Vertical GHE 1x4 Std,  !- Name",
        "    GHLE Inlet,         !- Inlet Node Name",
        "    GHLE Outlet,        !- Outlet Node Name",
        "    0.1,                !- Design Flow Rate {m3/s}",
        "    Site:GroundTemperature:Undisturbed:KusudaAchenbach,  !- Undisturbed Ground Temperature Model Type",
        "    KATemps,            !- Undisturbed Ground Temperature Model Name",
        "    1.0,                !- Ground Thermal Conductivity {W/m-K}",
        "    2.4957E+06,         !- Ground Thermal Heat Capacity {J/m3-K}",
        "    ,                   !- Response Factors Object Name",
        "    UHFCalc,            !- g-Function Calculation Method",
        "    GHE-Array;          !- GHE Array Object Name"});

    // Setup
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    PlantManager::GetPlantLoopData(*state);
    PlantManager::GetPlantInput(*state);
    PlantManager::SetupInitialPlantCallingOrder(*state);
    PlantManager::SetupBranchControlTypes(*state);

    auto &thisGLHE(state->dataGroundHeatExchanger->verticalGLHE[0]);
    thisGLHE.plantLoc.loopNum = 1;
    state->dataLoopNodes->Node(thisGLHE.inletNodeNum).Temp = 20.0;
    thisGLHE.massFlowRate = 1;

    Real64 constexpr tolerance = 0.00001;

    // Flow rate and pipe thickness picked to fix pipe resistance at 0.05
    EXPECT_NEAR(thisGLHE.theta_1, 0.37037, tolerance);
    EXPECT_NEAR(thisGLHE.theta_2, 9.0, tolerance);
    EXPECT_NEAR(thisGLHE.theta_3, 1 / (2 * thisGLHE.theta_1 * thisGLHE.theta_2), tolerance);
    EXPECT_NEAR(thisGLHE.sigma, (thisGLHE.grout.k - thisGLHE.soil.k) / (thisGLHE.grout.k + thisGLHE.soil.k), tolerance);
    EXPECT_NEAR(thisGLHE.calcBHGroutResistance(*state), 0.11038, tolerance);
}

TEST_F(EnergyPlusFixture, GroundHeatExchangerTest_System_calcBHTotalInternalResistance_1)
{
    using namespace DataSystemVariables;

    std::string const idf_objects = delimited_string({

        "Branch,",
        "    Main Floor Cooling Condenser Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Coil:Cooling:WaterToAirHeatPump:EquationFit,  !- Component 1 Object Type",
        "    Main Floor WAHP Cooling Coil,  !- Component 1 Name",
        "    Main Floor WAHP Cooling Water Inlet Node,  !- Component 1 Inlet Node Name",
        "    Main Floor WAHP Cooling Water Outlet Node;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Main Floor Heating Condenser Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Coil:Heating:WaterToAirHeatPump:EquationFit,  !- Component 1 Object Type",
        "    Main Floor WAHP Heating Coil,  !- Component 1 Name",
        "    Main Floor WAHP Heating Water Inlet Node,  !- Component 1 Inlet Node Name",
        "    Main Floor WAHP Heating Water Outlet Node;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    GHE-Vert Branch,         !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    GroundHeatExchanger:System,  !- Component 1 Object Type",
        "    Vertical GHE 1x4 Std,    !- Component 1 Name",
        "    GHLE Inlet,         !- Component 1 Inlet Node Name",
        "    GHLE Outlet;        !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Supply Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:ConstantSpeed,      !- Component 1 Object Type",
        "    Ground Loop Supply Pump, !- Component 1 Name",
        "    Ground Loop Supply Inlet,!- Component 1 Inlet Node Name",
        "    Ground Loop Pump Outlet; !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Supply Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Ground Loop Supply Outlet Pipe,  !- Component 1 Name",
        "    Ground Loop Supply Outlet Pipe Inlet,  !- Component 1 Inlet Node Name",
        "    Ground Loop Supply Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Demand Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Ground Loop Demand Inlet Pipe,  !- Component 1 Name",
        "    Ground Loop Demand Inlet,!- Component 1 Inlet Node Name",
        "    Ground Loop Demand Inlet Pipe Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Demand Bypass Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Ground Loop Demand Side Bypass Pipe,  !- Component 1 Name",
        "    Ground Loop Demand Bypass Inlet,  !- Component 1 Inlet Node Name",
        "    Ground Loop Demand Bypass Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Demand Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Ground Loop Demand Outlet Pipe,  !- Component 1 Name",
        "    Ground Loop Demand Outlet Pipe Inlet,  !- Component 1 Inlet Node Name",
        "    Ground Loop Demand Outlet;  !- Component 1 Outlet Node Name",

        "BranchList,",
        "    Ground Loop Supply Side Branches,  !- Name",
        "    Ground Loop Supply Inlet Branch,  !- Branch 1 Name",
        "    GHE-Vert Branch,         !- Branch 2 Name",
        "    Ground Loop Supply Outlet Branch;  !- Branch 3 Name",

        "BranchList,",
        "    Ground Loop Demand Side Branches,  !- Name",
        "    Ground Loop Demand Inlet Branch,  !- Branch 1 Name",
        "    Main Floor Cooling Condenser Branch,  !- Branch 2 Name",
        "    Main Floor Heating Condenser Branch,  !- Branch 3 Name",
        "    Ground Loop Demand Bypass Branch,  !- Branch 4 Name",
        "    Ground Loop Demand Outlet Branch;  !- Branch 5 Name",

        "Connector:Splitter,",
        "    Ground Loop Supply Splitter,  !- Name",
        "    Ground Loop Supply Inlet Branch,  !- Inlet Branch Name",
        "    GHE-Vert Branch;         !- Outlet Branch 1 Name",

        "Connector:Splitter,",
        "    Ground Loop Demand Splitter,  !- Name",
        "    Ground Loop Demand Inlet Branch,  !- Inlet Branch Name",
        "    Ground Loop Demand Bypass Branch,  !- Outlet Branch 1 Name",
        "    Main Floor Cooling Condenser Branch,  !- Outlet Branch 2 Name",
        "    Main Floor Heating Condenser Branch;  !- Outlet Branch 3 Name",

        "Connector:Mixer,",
        "    Ground Loop Supply Mixer,!- Name",
        "    Ground Loop Supply Outlet Branch,  !- Outlet Branch Name",
        "    GHE-Vert Branch;         !- Inlet Branch 1 Name",

        "Connector:Mixer,",
        "    Ground Loop Demand Mixer,!- Name",
        "    Ground Loop Demand Outlet Branch,  !- Outlet Branch Name",
        "    Ground Loop Demand Bypass Branch,  !- Inlet Branch 1 Name",
        "    Main Floor Cooling Condenser Branch,  !- Inlet Branch 2 Name",
        "    Main Floor Heating Condenser Branch;  !- Inlet Branch 3 Name",

        "ConnectorList,",
        "    Ground Loop Supply Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    Ground Loop Supply Splitter,  !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    Ground Loop Supply Mixer;!- Connector 2 Name",

        "ConnectorList,",
        "    Ground Loop Demand Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    Ground Loop Demand Splitter,  !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    Ground Loop Demand Mixer;!- Connector 2 Name",

        "NodeList,",
        "    Ground Loop Supply Setpoint Nodes,  !- Name",
        "    GHLE Outlet,                        !- Node 1 Name",
        "    Ground Loop Supply Outlet;  !- Node 2 Name",

        "OutdoorAir:Node,",
        "    Main Floor WAHP Outside Air Inlet,  !- Name",
        "    -1;                      !- Height Above Ground {m}",

        "Pipe:Adiabatic,",
        "    Ground Loop Supply Outlet Pipe,  !- Name",
        "    Ground Loop Supply Outlet Pipe Inlet,  !- Inlet Node Name",
        "    Ground Loop Supply Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Ground Loop Demand Inlet Pipe,  !- Name",
        "    Ground Loop Demand Inlet,!- Inlet Node Name",
        "    Ground Loop Demand Inlet Pipe Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Ground Loop Demand Side Bypass Pipe,  !- Name",
        "    Ground Loop Demand Bypass Inlet,  !- Inlet Node Name",
        "    Ground Loop Demand Bypass Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Ground Loop Demand Outlet Pipe,  !- Name",
        "    Ground Loop Demand Outlet Pipe Inlet,  !- Inlet Node Name",
        "    Ground Loop Demand Outlet;  !- Outlet Node Name",

        "Pump:ConstantSpeed,",
        "    Ground Loop Supply Pump, !- Name",
        "    Ground Loop Supply Inlet,!- Inlet Node Name",
        "    Ground Loop Pump Outlet, !- Outlet Node Name",
        "    autosize,                !- Design Flow Rate {m3/s}",
        "    179352,                  !- Design Pump Head {Pa}",
        "    autosize,                !- Design Power Consumption {W}",
        "    0.9,                     !- Motor Efficiency",
        "    0,                       !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    Intermittent;            !- Pump Control Type",

        "PlantLoop,",
        "    Ground Loop Water Loop,  !- Name",
        "    Water,                      !- Fluid Type",
        "    ,                           !- User Defined Fluid Type",
        "    Only Water Loop Operation,  !- Plant Equipment Operation Scheme Name",
        "    Ground Loop Supply Outlet,  !- Loop Temperature Setpoint Node Name",
        "    100,                     !- Maximum Loop Temperature {C}",
        "    10,                      !- Minimum Loop Temperature {C}",
        "    autosize,                !- Maximum Loop Flow Rate {m3/s}",
        "    0,                       !- Minimum Loop Flow Rate {m3/s}",
        "    autosize,                !- Plant Loop Volume {m3}",
        "    Ground Loop Supply Inlet,!- Plant Side Inlet Node Name",
        "    Ground Loop Supply Outlet,  !- Plant Side Outlet Node Name",
        "    Ground Loop Supply Side Branches,  !- Plant Side Branch List Name",
        "    Ground Loop Supply Side Connectors,  !- Plant Side Connector List Name",
        "    Ground Loop Demand Inlet,!- Demand Side Inlet Node Name",
        "    Ground Loop Demand Outlet,  !- Demand Side Outlet Node Name",
        "    Ground Loop Demand Side Branches,  !- Demand Side Branch List Name",
        "    Ground Loop Demand Side Connectors,  !- Demand Side Connector List Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ,                        !- Availability Manager List Name",
        "    DualSetPointDeadband;    !- Plant Loop Demand Calculation Scheme",

        "PlantEquipmentList,",
        "    Only Water Loop All Cooling Equipment,  !- Name",
        "    GroundHeatExchanger:System,  !- Equipment 1 Object Type",
        "    Vertical GHE 1x4 Std;    !- Equipment 1 Name",

        "PlantEquipmentOperation:CoolingLoad,",
        "    Only Water Loop Cool Operation All Hours,  !- Name",
        "    0,                       !- Load Range 1 Lower Limit {W}",
        "    1000000000000000,        !- Load Range 1 Upper Limit {W}",
        "    Only Water Loop All Cooling Equipment;  !- Range 1 Equipment List Name",

        "PlantEquipmentOperationSchemes,",
        "    Only Water Loop Operation,  !- Name",
        "    PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type",
        "    Only Water Loop Cool Operation All Hours,  !- Control Scheme 1 Name",
        "    HVACTemplate-Always 1;   !- Control Scheme 1 Schedule Name",

        "SetpointManager:Scheduled:DualSetpoint,",
        "    Ground Loop Temp Manager,!- Name",
        "    Temperature,             !- Control Variable",
        "    HVACTemplate-Always 34,  !- High Setpoint Schedule Name",
        "    HVACTemplate-Always 20,  !- Low Setpoint Schedule Name",
        "    Ground Loop Supply Setpoint Nodes;  !- Setpoint Node or NodeList Name",

        "Schedule:Compact,",
        "    HVACTemplate-Always 4,   !- Name",
        "    HVACTemplate Any Number, !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,4;          !- Field 3",

        "Schedule:Compact,",
        "    HVACTemplate-Always 34,  !- Name",
        "    HVACTemplate Any Number, !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,34;         !- Field 3",

        "Schedule:Compact,",
        "    HVACTemplate-Always 20,  !- Name",
        "    HVACTemplate Any Number, !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,20;         !- Field 3",

        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "    KATemps,                 !- Name",
        "    1.8,                     !- Soil Thermal Conductivity {W/m-K}",
        "    920,                     !- Soil Density {kg/m3}",
        "    2200,                    !- Soil Specific Heat {J/kg-K}",
        "    15.5,                    !- Average Soil Surface Temperature {C}",
        "    3.2,                     !- Average Amplitude of Surface Temperature {deltaC}",
        "    8;                       !- Phase Shift of Minimum Surface Temperature {days}",

        "GroundHeatExchanger:Vertical:Properties,",
        "    GHE-1 Props,        !- Name",
        "    1,                  !- Depth of Top of Borehole {m}",
        "    76.2,               !- Borehole Length {m}",
        "    0.096,              !- Borehole Diameter {m}",
        "    0.6,                !- Grout Thermal Conductivity {W/m-K}",
        "    1.0E+06,            !- Grout Thermal Heat Capacity {J/m3-K}",
        "    0.389,              !- Pipe Thermal Conductivity {W/m-K}",
        "    8E+05,              !- Pipe Thermal Heat Capacity {J/m3-K}",
        "    0.032,              !- Pipe Outer Diameter {m}",
        "    0.0016279,          !- Pipe Thickness {m}",
        "    0.032;              !- U-Tube Distance {m}",

        "GroundHeatExchanger:Vertical:Array,",
        "    GHE-Array,          !- Name",
        "    GHE-1 Props,        !- GHE Properties",
        "    2,                  !- Number of Boreholes in X Direction",
        "    2,                  !- Number of Boreholes in Y Direction",
        "    2;                  !- Borehole Spacing {m}",

        "GroundHeatExchanger:System,",
        "    Vertical GHE 1x4 Std,  !- Name",
        "    GHLE Inlet,         !- Inlet Node Name",
        "    GHLE Outlet,        !- Outlet Node Name",
        "    0.1,                !- Design Flow Rate {m3/s}",
        "    Site:GroundTemperature:Undisturbed:KusudaAchenbach,  !- Undisturbed Ground Temperature Model Type",
        "    KATemps,            !- Undisturbed Ground Temperature Model Name",
        "    4.0,                !- Ground Thermal Conductivity {W/m-K}",
        "    2.4957E+06,         !- Ground Thermal Heat Capacity {J/m3-K}",
        "    ,                   !- Response Factors Object Name",
        "    UHFCalc,            !- g-Function Calculation Method",
        "    GHE-Array;          !- GHE Array Object Name"});

    // Setup
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    PlantManager::GetPlantLoopData(*state);
    PlantManager::GetPlantInput(*state);
    PlantManager::SetupInitialPlantCallingOrder(*state);
    PlantManager::SetupBranchControlTypes(*state);

    auto &thisGLHE(state->dataGroundHeatExchanger->verticalGLHE[0]);
    thisGLHE.plantLoc.loopNum = 1;
    state->dataLoopNodes->Node(thisGLHE.inletNodeNum).Temp = 20.0;
    thisGLHE.massFlowRate = 1;

    Real64 constexpr tolerance = 0.00001;

    // Flow rate and pipe thickness picked to fix pipe resistance at 0.05
    EXPECT_NEAR(thisGLHE.theta_1, 0.33333, tolerance);
    EXPECT_NEAR(thisGLHE.theta_2, 3.0, tolerance);
    EXPECT_NEAR(thisGLHE.theta_3, 1 / (2 * thisGLHE.theta_1 * thisGLHE.theta_2), tolerance);
    EXPECT_NEAR(thisGLHE.sigma, (thisGLHE.grout.k - thisGLHE.soil.k) / (thisGLHE.grout.k + thisGLHE.soil.k), tolerance);
    EXPECT_NEAR(thisGLHE.calcBHTotalInternalResistance(*state), 0.32365, tolerance);
}

TEST_F(EnergyPlusFixture, GroundHeatExchangerTest_System_calcBHTotalInternalResistance_2)
{
    using namespace DataSystemVariables;

    std::string const idf_objects = delimited_string({

        "Branch,",
        "    Main Floor Cooling Condenser Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Coil:Cooling:WaterToAirHeatPump:EquationFit,  !- Component 1 Object Type",
        "    Main Floor WAHP Cooling Coil,  !- Component 1 Name",
        "    Main Floor WAHP Cooling Water Inlet Node,  !- Component 1 Inlet Node Name",
        "    Main Floor WAHP Cooling Water Outlet Node;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Main Floor Heating Condenser Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Coil:Heating:WaterToAirHeatPump:EquationFit,  !- Component 1 Object Type",
        "    Main Floor WAHP Heating Coil,  !- Component 1 Name",
        "    Main Floor WAHP Heating Water Inlet Node,  !- Component 1 Inlet Node Name",
        "    Main Floor WAHP Heating Water Outlet Node;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    GHE-Vert Branch,         !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    GroundHeatExchanger:System,  !- Component 1 Object Type",
        "    Vertical GHE 1x4 Std,    !- Component 1 Name",
        "    GHLE Inlet,         !- Component 1 Inlet Node Name",
        "    GHLE Outlet;        !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Supply Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:ConstantSpeed,      !- Component 1 Object Type",
        "    Ground Loop Supply Pump, !- Component 1 Name",
        "    Ground Loop Supply Inlet,!- Component 1 Inlet Node Name",
        "    Ground Loop Pump Outlet; !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Supply Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Ground Loop Supply Outlet Pipe,  !- Component 1 Name",
        "    Ground Loop Supply Outlet Pipe Inlet,  !- Component 1 Inlet Node Name",
        "    Ground Loop Supply Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Demand Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Ground Loop Demand Inlet Pipe,  !- Component 1 Name",
        "    Ground Loop Demand Inlet,!- Component 1 Inlet Node Name",
        "    Ground Loop Demand Inlet Pipe Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Demand Bypass Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Ground Loop Demand Side Bypass Pipe,  !- Component 1 Name",
        "    Ground Loop Demand Bypass Inlet,  !- Component 1 Inlet Node Name",
        "    Ground Loop Demand Bypass Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Demand Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Ground Loop Demand Outlet Pipe,  !- Component 1 Name",
        "    Ground Loop Demand Outlet Pipe Inlet,  !- Component 1 Inlet Node Name",
        "    Ground Loop Demand Outlet;  !- Component 1 Outlet Node Name",

        "BranchList,",
        "    Ground Loop Supply Side Branches,  !- Name",
        "    Ground Loop Supply Inlet Branch,  !- Branch 1 Name",
        "    GHE-Vert Branch,         !- Branch 2 Name",
        "    Ground Loop Supply Outlet Branch;  !- Branch 3 Name",

        "BranchList,",
        "    Ground Loop Demand Side Branches,  !- Name",
        "    Ground Loop Demand Inlet Branch,  !- Branch 1 Name",
        "    Main Floor Cooling Condenser Branch,  !- Branch 2 Name",
        "    Main Floor Heating Condenser Branch,  !- Branch 3 Name",
        "    Ground Loop Demand Bypass Branch,  !- Branch 4 Name",
        "    Ground Loop Demand Outlet Branch;  !- Branch 5 Name",

        "Connector:Splitter,",
        "    Ground Loop Supply Splitter,  !- Name",
        "    Ground Loop Supply Inlet Branch,  !- Inlet Branch Name",
        "    GHE-Vert Branch;         !- Outlet Branch 1 Name",

        "Connector:Splitter,",
        "    Ground Loop Demand Splitter,  !- Name",
        "    Ground Loop Demand Inlet Branch,  !- Inlet Branch Name",
        "    Ground Loop Demand Bypass Branch,  !- Outlet Branch 1 Name",
        "    Main Floor Cooling Condenser Branch,  !- Outlet Branch 2 Name",
        "    Main Floor Heating Condenser Branch;  !- Outlet Branch 3 Name",

        "Connector:Mixer,",
        "    Ground Loop Supply Mixer,!- Name",
        "    Ground Loop Supply Outlet Branch,  !- Outlet Branch Name",
        "    GHE-Vert Branch;         !- Inlet Branch 1 Name",

        "Connector:Mixer,",
        "    Ground Loop Demand Mixer,!- Name",
        "    Ground Loop Demand Outlet Branch,  !- Outlet Branch Name",
        "    Ground Loop Demand Bypass Branch,  !- Inlet Branch 1 Name",
        "    Main Floor Cooling Condenser Branch,  !- Inlet Branch 2 Name",
        "    Main Floor Heating Condenser Branch;  !- Inlet Branch 3 Name",

        "ConnectorList,",
        "    Ground Loop Supply Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    Ground Loop Supply Splitter,  !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    Ground Loop Supply Mixer;!- Connector 2 Name",

        "ConnectorList,",
        "    Ground Loop Demand Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    Ground Loop Demand Splitter,  !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    Ground Loop Demand Mixer;!- Connector 2 Name",

        "NodeList,",
        "    Ground Loop Supply Setpoint Nodes,  !- Name",
        "    GHLE Outlet,                        !- Node 1 Name",
        "    Ground Loop Supply Outlet;  !- Node 2 Name",

        "OutdoorAir:Node,",
        "    Main Floor WAHP Outside Air Inlet,  !- Name",
        "    -1;                      !- Height Above Ground {m}",

        "Pipe:Adiabatic,",
        "    Ground Loop Supply Outlet Pipe,  !- Name",
        "    Ground Loop Supply Outlet Pipe Inlet,  !- Inlet Node Name",
        "    Ground Loop Supply Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Ground Loop Demand Inlet Pipe,  !- Name",
        "    Ground Loop Demand Inlet,!- Inlet Node Name",
        "    Ground Loop Demand Inlet Pipe Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Ground Loop Demand Side Bypass Pipe,  !- Name",
        "    Ground Loop Demand Bypass Inlet,  !- Inlet Node Name",
        "    Ground Loop Demand Bypass Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Ground Loop Demand Outlet Pipe,  !- Name",
        "    Ground Loop Demand Outlet Pipe Inlet,  !- Inlet Node Name",
        "    Ground Loop Demand Outlet;  !- Outlet Node Name",

        "Pump:ConstantSpeed,",
        "    Ground Loop Supply Pump, !- Name",
        "    Ground Loop Supply Inlet,!- Inlet Node Name",
        "    Ground Loop Pump Outlet, !- Outlet Node Name",
        "    autosize,                !- Design Flow Rate {m3/s}",
        "    179352,                  !- Design Pump Head {Pa}",
        "    autosize,                !- Design Power Consumption {W}",
        "    0.9,                     !- Motor Efficiency",
        "    0,                       !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    Intermittent;            !- Pump Control Type",

        "PlantLoop,",
        "    Ground Loop Water Loop,  !- Name",
        "    Water,                      !- Fluid Type",
        "    ,                           !- User Defined Fluid Type",
        "    Only Water Loop Operation,  !- Plant Equipment Operation Scheme Name",
        "    Ground Loop Supply Outlet,  !- Loop Temperature Setpoint Node Name",
        "    100,                     !- Maximum Loop Temperature {C}",
        "    10,                      !- Minimum Loop Temperature {C}",
        "    autosize,                !- Maximum Loop Flow Rate {m3/s}",
        "    0,                       !- Minimum Loop Flow Rate {m3/s}",
        "    autosize,                !- Plant Loop Volume {m3}",
        "    Ground Loop Supply Inlet,!- Plant Side Inlet Node Name",
        "    Ground Loop Supply Outlet,  !- Plant Side Outlet Node Name",
        "    Ground Loop Supply Side Branches,  !- Plant Side Branch List Name",
        "    Ground Loop Supply Side Connectors,  !- Plant Side Connector List Name",
        "    Ground Loop Demand Inlet,!- Demand Side Inlet Node Name",
        "    Ground Loop Demand Outlet,  !- Demand Side Outlet Node Name",
        "    Ground Loop Demand Side Branches,  !- Demand Side Branch List Name",
        "    Ground Loop Demand Side Connectors,  !- Demand Side Connector List Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ,                        !- Availability Manager List Name",
        "    DualSetPointDeadband;    !- Plant Loop Demand Calculation Scheme",

        "PlantEquipmentList,",
        "    Only Water Loop All Cooling Equipment,  !- Name",
        "    GroundHeatExchanger:System,  !- Equipment 1 Object Type",
        "    Vertical GHE 1x4 Std;    !- Equipment 1 Name",

        "PlantEquipmentOperation:CoolingLoad,",
        "    Only Water Loop Cool Operation All Hours,  !- Name",
        "    0,                       !- Load Range 1 Lower Limit {W}",
        "    1000000000000000,        !- Load Range 1 Upper Limit {W}",
        "    Only Water Loop All Cooling Equipment;  !- Range 1 Equipment List Name",

        "PlantEquipmentOperationSchemes,",
        "    Only Water Loop Operation,  !- Name",
        "    PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type",
        "    Only Water Loop Cool Operation All Hours,  !- Control Scheme 1 Name",
        "    HVACTemplate-Always 1;   !- Control Scheme 1 Schedule Name",

        "SetpointManager:Scheduled:DualSetpoint,",
        "    Ground Loop Temp Manager,!- Name",
        "    Temperature,             !- Control Variable",
        "    HVACTemplate-Always 34,  !- High Setpoint Schedule Name",
        "    HVACTemplate-Always 20,  !- Low Setpoint Schedule Name",
        "    Ground Loop Supply Setpoint Nodes;  !- Setpoint Node or NodeList Name",

        "Schedule:Compact,",
        "    HVACTemplate-Always 4,   !- Name",
        "    HVACTemplate Any Number, !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,4;          !- Field 3",

        "Schedule:Compact,",
        "    HVACTemplate-Always 34,  !- Name",
        "    HVACTemplate Any Number, !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,34;         !- Field 3",

        "Schedule:Compact,",
        "    HVACTemplate-Always 20,  !- Name",
        "    HVACTemplate Any Number, !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,20;         !- Field 3",

        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "    KATemps,                 !- Name",
        "    1.8,                     !- Soil Thermal Conductivity {W/m-K}",
        "    920,                     !- Soil Density {kg/m3}",
        "    2200,                    !- Soil Specific Heat {J/kg-K}",
        "    15.5,                    !- Average Soil Surface Temperature {C}",
        "    3.2,                     !- Average Amplitude of Surface Temperature {deltaC}",
        "    8;                       !- Phase Shift of Minimum Surface Temperature {days}",

        "GroundHeatExchanger:Vertical:Properties,",
        "    GHE-1 Props,        !- Name",
        "    1,                  !- Depth of Top of Borehole {m}",
        "    76.2,               !- Borehole Length {m}",
        "    0.192,              !- Borehole Diameter {m}",
        "    3.6,                !- Grout Thermal Conductivity {W/m-K}",
        "    1.0E+06,            !- Grout Thermal Heat Capacity {J/m3-K}",
        "    0.389,              !- Pipe Thermal Conductivity {W/m-K}",
        "   8E+05,              !- Pipe Thermal Heat Capacity {J/m3-K}",
        "    0.032,              !- Pipe Outer Diameter {m}",
        "    0.0016279,          !- Pipe Thickness {m}",
        "    0.032;              !- U-Tube Distance {m}",

        "GroundHeatExchanger:Vertical:Array,",
        "    GHE-Array,          !- Name",
        "    GHE-1 Props,        !- GHE Properties",
        "    2,                  !- Number of Boreholes in X Direction",
        "    2,                  !- Number of Boreholes in Y Direction",
        "    2;                  !- Borehole Spacing {m}",

        "GroundHeatExchanger:System,",
        "    Vertical GHE 1x4 Std,  !- Name",
        "    GHLE Inlet,         !- Inlet Node Name",
        "    GHLE Outlet,        !- Outlet Node Name",
        "    0.1,                !- Design Flow Rate {m3/s}",
        "    Site:GroundTemperature:Undisturbed:KusudaAchenbach,  !- Undisturbed Ground Temperature Model Type",
        "    KATemps,            !- Undisturbed Ground Temperature Model Name",
        "    3.0,                !- Ground Thermal Conductivity {W/m-K}",
        "    2.4957E+06,         !- Ground Thermal Heat Capacity {J/m3-K}",
        "    ,                   !- Response Factors Object Name",
        "    UHFCalc,            !- g-Function Calculation Method",
        "    GHE-Array;          !- GHE Array Object Name"});

    // Setup
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);
    PlantManager::GetPlantLoopData(*state);
    PlantManager::GetPlantInput(*state);
    PlantManager::SetupInitialPlantCallingOrder(*state);
    PlantManager::SetupBranchControlTypes(*state);

    auto &thisGLHE(state->dataGroundHeatExchanger->verticalGLHE[0]);
    thisGLHE.plantLoc.loopNum = 1;
    state->dataLoopNodes->Node(thisGLHE.inletNodeNum).Temp = 20.0;
    thisGLHE.massFlowRate = 1;

    Real64 constexpr tolerance = 0.00001;

    // Flow rate and pipe thickness picked to fix pipe resistance at 0.05
    EXPECT_NEAR(thisGLHE.theta_1, 0.166667, tolerance);
    EXPECT_NEAR(thisGLHE.theta_2, 6.0, tolerance);
    EXPECT_NEAR(thisGLHE.theta_3, 1 / (2 * thisGLHE.theta_1 * thisGLHE.theta_2), tolerance);
    EXPECT_NEAR(thisGLHE.sigma, (thisGLHE.grout.k - thisGLHE.soil.k) / (thisGLHE.grout.k + thisGLHE.soil.k), tolerance);
    EXPECT_NEAR(thisGLHE.calcBHTotalInternalResistance(*state), 0.16310, tolerance);
}

TEST_F(EnergyPlusFixture, GroundHeatExchangerTest_System_calcBHTotalInternalResistance_3)
{
    using namespace DataSystemVariables;

    std::string const idf_objects = delimited_string({

        "Branch,",
        "    Main Floor Cooling Condenser Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Coil:Cooling:WaterToAirHeatPump:EquationFit,  !- Component 1 Object Type",
        "    Main Floor WAHP Cooling Coil,  !- Component 1 Name",
        "    Main Floor WAHP Cooling Water Inlet Node,  !- Component 1 Inlet Node Name",
        "    Main Floor WAHP Cooling Water Outlet Node;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Main Floor Heating Condenser Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Coil:Heating:WaterToAirHeatPump:EquationFit,  !- Component 1 Object Type",
        "    Main Floor WAHP Heating Coil,  !- Component 1 Name",
        "    Main Floor WAHP Heating Water Inlet Node,  !- Component 1 Inlet Node Name",
        "    Main Floor WAHP Heating Water Outlet Node;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    GHE-Vert Branch,         !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    GroundHeatExchanger:System,  !- Component 1 Object Type",
        "    Vertical GHE 1x4 Std,    !- Component 1 Name",
        "    GHLE Inlet,         !- Component 1 Inlet Node Name",
        "    GHLE Outlet;        !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Supply Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pump:ConstantSpeed,      !- Component 1 Object Type",
        "    Ground Loop Supply Pump, !- Component 1 Name",
        "    Ground Loop Supply Inlet,!- Component 1 Inlet Node Name",
        "    Ground Loop Pump Outlet; !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Supply Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Ground Loop Supply Outlet Pipe,  !- Component 1 Name",
        "    Ground Loop Supply Outlet Pipe Inlet,  !- Component 1 Inlet Node Name",
        "    Ground Loop Supply Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Demand Inlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Ground Loop Demand Inlet Pipe,  !- Component 1 Name",
        "    Ground Loop Demand Inlet,!- Component 1 Inlet Node Name",
        "    Ground Loop Demand Inlet Pipe Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Demand Bypass Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Ground Loop Demand Side Bypass Pipe,  !- Component 1 Name",
        "    Ground Loop Demand Bypass Inlet,  !- Component 1 Inlet Node Name",
        "    Ground Loop Demand Bypass Outlet;  !- Component 1 Outlet Node Name",

        "Branch,",
        "    Ground Loop Demand Outlet Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    Ground Loop Demand Outlet Pipe,  !- Component 1 Name",
        "    Ground Loop Demand Outlet Pipe Inlet,  !- Component 1 Inlet Node Name",
        "    Ground Loop Demand Outlet;  !- Component 1 Outlet Node Name",

        "BranchList,",
        "    Ground Loop Supply Side Branches,  !- Name",
        "    Ground Loop Supply Inlet Branch,  !- Branch 1 Name",
        "    GHE-Vert Branch,         !- Branch 2 Name",
        "    Ground Loop Supply Outlet Branch;  !- Branch 3 Name",

        "BranchList,",
        "    Ground Loop Demand Side Branches,  !- Name",
        "    Ground Loop Demand Inlet Branch,  !- Branch 1 Name",
        "    Main Floor Cooling Condenser Branch,  !- Branch 2 Name",
        "    Main Floor Heating Condenser Branch,  !- Branch 3 Name",
        "    Ground Loop Demand Bypass Branch,  !- Branch 4 Name",
        "    Ground Loop Demand Outlet Branch;  !- Branch 5 Name",

        "Connector:Splitter,",
        "    Ground Loop Supply Splitter,  !- Name",
        "    Ground Loop Supply Inlet Branch,  !- Inlet Branch Name",
        "    GHE-Vert Branch;         !- Outlet Branch 1 Name",

        "Connector:Splitter,",
        "    Ground Loop Demand Splitter,  !- Name",
        "    Ground Loop Demand Inlet Branch,  !- Inlet Branch Name",
        "    Ground Loop Demand Bypass Branch,  !- Outlet Branch 1 Name",
        "    Main Floor Cooling Condenser Branch,  !- Outlet Branch 2 Name",
        "    Main Floor Heating Condenser Branch;  !- Outlet Branch 3 Name",

        "Connector:Mixer,",
        "    Ground Loop Supply Mixer,!- Name",
        "    Ground Loop Supply Outlet Branch,  !- Outlet Branch Name",
        "    GHE-Vert Branch;         !- Inlet Branch 1 Name",

        "Connector:Mixer,",
        "    Ground Loop Demand Mixer,!- Name",
        "    Ground Loop Demand Outlet Branch,  !- Outlet Branch Name",
        "    Ground Loop Demand Bypass Branch,  !- Inlet Branch 1 Name",
        "    Main Floor Cooling Condenser Branch,  !- Inlet Branch 2 Name",
        "    Main Floor Heating Condenser Branch;  !- Inlet Branch 3 Name",

        "ConnectorList,",
        "    Ground Loop Supply Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    Ground Loop Supply Splitter,  !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    Ground Loop Supply Mixer;!- Connector 2 Name",

        "ConnectorList,",
        "    Ground Loop Demand Side Connectors,  !- Name",
        "    Connector:Splitter,      !- Connector 1 Object Type",
        "    Ground Loop Demand Splitter,  !- Connector 1 Name",
        "    Connector:Mixer,         !- Connector 2 Object Type",
        "    Ground Loop Demand Mixer;!- Connector 2 Name",

        "NodeList,",
        "    Ground Loop Supply Setpoint Nodes,  !- Name",
        "    GHLE Outlet,                        !- Node 1 Name",
        "    Ground Loop Supply Outlet;  !- Node 2 Name",

        "OutdoorAir:Node,",
        "    Main Floor WAHP Outside Air Inlet,  !- Name",
        "    -1;                      !- Height Above Ground {m}",

        "Pipe:Adiabatic,",
        "    Ground Loop Supply Outlet Pipe,  !- Name",
        "    Ground Loop Supply Outlet Pipe Inlet,  !- Inlet Node Name",
        "    Ground Loop Supply Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Ground Loop Demand Inlet Pipe,  !- Name",
        "    Ground Loop Demand Inlet,!- Inlet Node Name",
        "    Ground Loop Demand Inlet Pipe Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Ground Loop Demand Side Bypass Pipe,  !- Name",
        "    Ground Loop Demand Bypass Inlet,  !- Inlet Node Name",
        "    Ground Loop Demand Bypass Outlet;  !- Outlet Node Name",

        "Pipe:Adiabatic,",
        "    Ground Loop Demand Outlet Pipe,  !- Name",
        "    Ground Loop Demand Outlet Pipe Inlet,  !- Inlet Node Name",
        "    Ground Loop Demand Outlet;  !- Outlet Node Name",

        "Pump:ConstantSpeed,",
        "    Ground Loop Supply Pump, !- Name",
        "    Ground Loop Supply Inlet,!- Inlet Node Name",
        "    Ground Loop Pump Outlet, !- Outlet Node Name",
        "    autosize,                !- Design Flow Rate {m3/s}",
        "    179352,                  !- Design Pump Head {Pa}",
        "    autosize,                !- Design Power Consumption {W}",
        "    0.9,                     !- Motor Efficiency",
        "    0,                       !- Fraction of Motor Inefficiencies to Fluid Stream",
        "    Intermittent;            !- Pump Control Type",

        "PlantLoop,",
        "    Ground Loop Water Loop,  !- Name",
        "    Water,                      !- Fluid Type",
        "    ,                           !- User Defined Fluid Type",
        "    Only Water Loop Operation,  !- Plant Equipment Operation Scheme Name",
        "    Ground Loop Supply Outlet,  !- Loop Temperature Setpoint Node Name",
        "    100,                     !- Maximum Loop Temperature {C}",
        "    10,                      !- Minimum Loop Temperature {C}",
        "    autosize,                !- Maximum Loop Flow Rate {m3/s}",
        "    0,                       !- Minimum Loop Flow Rate {m3/s}",
        "    autosize,                !- Plant Loop Volume {m3}",
        "    Ground Loop Supply Inlet,!- Plant Side Inlet Node Name",
        "    Ground Loop Supply Outlet,  !- Plant Side Outlet Node Name",
        "    Ground Loop Supply Side Branches,  !- Plant Side Branch List Name",
        "    Ground Loop Supply Side Connectors,  !- Plant Side Connector List Name",
        "    Ground Loop Demand Inlet,!- Demand Side Inlet Node Name",
        "    Ground Loop Demand Outlet,  !- Demand Side Outlet Node Name",
        "    Ground Loop Demand Side Branches,  !- Demand Side Branch List Name",
        "    Ground Loop Demand Side Connectors,  !- Demand Side Connector List Name",
        "    SequentialLoad,          !- Load Distribution Scheme",
        "    ,                        !- Availability Manager List Name",
        "    DualSetPointDeadband;    !- Plant Loop Demand Calculation Scheme",

        "PlantEquipmentList,",
        "    Only Water Loop All Cooling Equipment,  !- Name",
        "    GroundHeatExchanger:System,  !- Equipment 1 Object Type",
        "    Vertical GHE 1x4 Std;    !- Equipment 1 Name",

        "PlantEquipmentOperation:CoolingLoad,",
        "    Only Water Loop Cool Operation All Hours,  !- Name",
        "    0,                       !- Load Range 1 Lower Limit {W}",
        "    1000000000000000,        !- Load Range 1 Upper Limit {W}",
        "    Only Water Loop All Cooling Equipment;  !- Range 1 Equipment List Name",

        "PlantEquipmentOperationSchemes,",
        "    Only Water Loop Operation,  !- Name",
        "    PlantEquipmentOperation:CoolingLoad,  !- Control Scheme 1 Object Type",
        "    Only Water Loop Cool Operation All Hours,  !- Control Scheme 1 Name",
        "    HVACTemplate-Always 1;   !- Control Scheme 1 Schedule Name",

        "SetpointManager:Scheduled:DualSetpoint,",
        "    Ground Loop Temp Manager,!- Name",
        "    Temperature,             !- Control Variable",
        "    HVACTemplate-Always 34,  !- High Setpoint Schedule Name",
        "    HVACTemplate-Always 20,  !- Low Setpoint Schedule Name",
        "    Ground Loop Supply Setpoint Nodes;  !- Setpoint Node or NodeList Name",

        "Schedule:Compact,",
        "    HVACTemplate-Always 4,   !- Name",
        "    HVACTemplate Any Number, !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,4;          !- Field 3",

        "Schedule:Compact,",
        "    HVACTemplate-Always 34,  !- Name",
        "    HVACTemplate Any Number, !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,34;         !- Field 3",

        "Schedule:Compact,",
        "    HVACTemplate-Always 20,  !- Name",
        "    HVACTemplate Any Number, !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,20;         !- Field 3",

        "Site:GroundTemperature:Undisturbed:KusudaAchenbach,",
        "    KATemps,                 !- Name",
        "    1.8,                     !- Soil Thermal Conductivity {W/m-K}",
        "    920,                     !- Soil Density {kg/m3}",
        "    2200,                    !- Soil Specific Heat {J/kg-K}",
        "    15.5,                    !- Average Soil Surface Temperature {C}",
        "    3.2,                     !- Average Amplitude of Surface Temperature {deltaC}",
        "    8;                       !- Phase Shift of Minimum Surface Temperature {days}",

        "GroundHeatExchanger:Vertical:Properties,",
        "    GHE-1 Props,        !- Name",
        "    1,                  !- Depth of Top of Borehole {m}",
        "    76.2,               !- Borehole Length {m}",
        "    0.288,              !- Borehole Diameter {m}",
        "    3.0,                !- Grout Thermal Conductivity {W/m-K}",
        "    1.0E+06,            !- Grout Thermal Heat Capacity {J/m3-K}",
        "    0.389,              !- Pipe Thermal Conductivity {W/m-K}",
        "    8E+05,              !- Pipe Thermal Heat Capacity {J/m3-K}",
        "    0.032,              !- Pipe Outer Diameter {m}",
        "    0.0016279,          !- Pipe Thickness {m}",
        "    0.1066667;          !- U-Tube Distance {m}",

        "GroundHeatExchanger:Vertical:Array,",
        "    GHE-Array,          !- Name",
        "    GHE-1 Props,        !- GHE Properties",
        "    2,                  !- Number of Boreholes in X Direction",
        "    2,                  !- Number of Boreholes in Y Direction",
        "    2;                  !- Borehole Spacing {m}",

        "GroundHeatExchanger:System,",
        "    Vertical GHE 1x4 Std,  !- Name",
        "    GHLE Inlet,         !- Inlet Node Name",
        "    GHLE Outlet,        !- Outlet Node Name",
        "    0.1,                !- Design Flow Rate {m3/s}",
        "    Site:GroundTemperature:Undisturbed:KusudaAchenbach,  !- Undisturbed Ground Temperature Model Type",
        "    KATemps,            !- Undisturbed Ground Temperature Model Name",
        "    1.0,                !- Ground Thermal Conductivity {W/m-K}",
        "    2.4957E+06,         !- Ground Thermal Heat Capacity {J/m3-K}",
        "    ,                   !- Response Factors Object Name",
        "    UHFCalc,            !- g-Function Calculation Method",
        "    GHE-Array;          !- GHE Array Object Name"});

    // Setup
    ASSERT_TRUE(process_idf(idf_objects));
    state->init_state(*state);

    PlantManager::GetPlantLoopData(*state);
    PlantManager::GetPlantInput(*state);
    PlantManager::SetupInitialPlantCallingOrder(*state);
    PlantManager::SetupBranchControlTypes(*state);

    auto &thisGLHE(state->dataGroundHeatExchanger->verticalGLHE[0]);
    thisGLHE.plantLoc.loopNum = 1;
    state->dataLoopNodes->Node(thisGLHE.inletNodeNum).Temp = 20.0;
    thisGLHE.massFlowRate = 1;

    Real64 constexpr tolerance = 0.00001;

    // Flow rate and pipe thickness picked to fix pipe resistance at 0.05
    EXPECT_NEAR(thisGLHE.theta_1, 0.37037, tolerance);
    EXPECT_NEAR(thisGLHE.theta_2, 9.0, tolerance);
    EXPECT_NEAR(thisGLHE.theta_3, 1 / (2 * thisGLHE.theta_1 * thisGLHE.theta_2), tolerance);
    EXPECT_NEAR(thisGLHE.sigma, (thisGLHE.grout.k - thisGLHE.soil.k) / (thisGLHE.grout.k + thisGLHE.soil.k), tolerance);
    EXPECT_NEAR(thisGLHE.calcBHTotalInternalResistance(*state), 0.31582, tolerance);
}
