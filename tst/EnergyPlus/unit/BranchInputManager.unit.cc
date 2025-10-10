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

#include "Fixtures/EnergyPlusFixture.hh"

// EnergyPlus Headers
#include <EnergyPlus/BranchInputManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>

using namespace EnergyPlus;
using namespace BranchInputManager;

namespace EnergyPlus {

// EnergyPlus::GetBranchInput Unit Tests

TEST_F(EnergyPlusFixture, GetBranchInput_One_SingleComponentBranch)
{

    std::string const idf_objects = delimited_string({
        "Branch,",
        "VAV Sys 1 Main Branch,   !- Name",
        ",                        !- Pressure Drop Curve Name",
        "AirLoopHVAC:OutdoorAirSystem,  !- Component 1 Object Type",
        "OA Sys 1,                !- Component 1 Name",
        "VAV Sys 1 Inlet Node,    !- Component 1 Inlet Node Name",
        "Mixed Air Node 1;        !- Component 1 Outlet Node Name",

        "AirLoopHVAC:OutdoorAirSystem,",
        "OA Sys 1,                !- Name",
        "OA Sys 1 Controllers,    !- Controller List Name",
        "OA Sys 1 Equipment;      !- Outdoor Air Equipment List Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    static constexpr std::string_view RoutineName("GetBranchInput: ");
    std::string CurrentModuleObject = "Branch";
    int NumOfBranches = state->dataInputProcessing->inputProcessor->getNumObjectsFound(*state, CurrentModuleObject);
    int NumParams;
    int NumAlphas;           // Used to retrieve names from IDF
    int NumNumbers;          // Used to retrieve numbers from IDF
    bool IsNotOK;            // Flag to verify name
    bool IsBlank;            // Flag for blank name
    Array1D_string Alphas;   // Used to retrieve names from IDF
    Array1D_int NodeNums;    // Possible Array of Node Numbers (only 1 allowed)
    Array1D<Real64> Numbers; // Used to retrieve numbers from IDF
    Array1D_string cAlphaFields;
    Array1D_string cNumericFields;
    Array1D_bool lNumericBlanks;
    Array1D_bool lAlphaBlanks;
    int IOStat; // Could be used in the Get Routines, not currently checked

    if (NumOfBranches > 0) {
        state->dataBranchInputManager->Branch.allocate(NumOfBranches);
        for (auto &e : state->dataBranchInputManager->Branch) {
            e.AssignedLoopName.clear();
        }
        state->dataInputProcessing->inputProcessor->getObjectDefMaxArgs(*state, "NodeList", NumParams, NumAlphas, NumNumbers);
        NodeNums.dimension(NumParams, 0);
        state->dataInputProcessing->inputProcessor->getObjectDefMaxArgs(*state, CurrentModuleObject, NumParams, NumAlphas, NumNumbers);
        Alphas.allocate(NumAlphas);
        Numbers.dimension(NumNumbers, 0.0);
        cAlphaFields.allocate(NumAlphas);
        cNumericFields.allocate(NumNumbers);
        lAlphaBlanks.dimension(NumAlphas, true);
        lNumericBlanks.dimension(NumNumbers, true);
        int BCount = 0;
        for (int Count = 1; Count <= NumOfBranches; ++Count) {

            state->dataInputProcessing->inputProcessor->getObjectItem(*state,
                                                                      CurrentModuleObject,
                                                                      Count,
                                                                      Alphas,
                                                                      NumAlphas,
                                                                      Numbers,
                                                                      NumNumbers,
                                                                      IOStat,
                                                                      lNumericBlanks,
                                                                      lAlphaBlanks,
                                                                      cAlphaFields,
                                                                      cNumericFields);
            IsNotOK = false;
            IsBlank = false;
            Util::VerifyName(*state, Alphas(1), state->dataBranchInputManager->Branch, BCount, IsNotOK, IsBlank, CurrentModuleObject + " Name");
            if (IsNotOK) {
                if (IsBlank) {
                    continue;
                } else {
                    Alphas(1) = Alphas(1) + "--dup";
                }
            }
            ++BCount;

            GetSingleBranchInput(*state, RoutineName, BCount, Alphas, cAlphaFields, NumAlphas, NodeNums, lAlphaBlanks);
        }

        EXPECT_EQ(NumOfBranches, 1);

        EXPECT_TRUE(Util::SameString(Alphas(1), "VAV Sys 1 Main Branch"));

        EXPECT_TRUE(Util::SameString(Alphas(3), "AirLoopHVAC:OutdoorAirSystem"));
        EXPECT_TRUE(Util::SameString(Alphas(4), "OA Sys 1"));
        EXPECT_TRUE(Util::SameString(Alphas(5), "VAV Sys 1 Inlet Node"));
        EXPECT_TRUE(Util::SameString(Alphas(6), "Mixed Air Node 1"));

        NodeNums.deallocate();
        Alphas.deallocate();
        Numbers.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();
    }
}

TEST_F(EnergyPlusFixture, GetBranchInput_One_FourComponentBranch)
{

    std::string const idf_objects = delimited_string({
        "Branch,",
        "VAV Sys 1 Main Branch,   !- Name",
        ",                        !- Pressure Drop Curve Name",
        "AirLoopHVAC:OutdoorAirSystem,  !- Component 1 Object Type",
        "OA Sys 1,                !- Component 1 Name",
        "VAV Sys 1 Inlet Node,    !- Component 1 Inlet Node Name",
        "Mixed Air Node 1,        !- Component 1 Outlet Node Name",
        "Coil:Cooling:Water,      !- Component 2 Object Type",
        "Main Cooling Coil 1,     !- Component 2 Name",
        "Mixed Air Node 1,        !- Component 2 Inlet Node Name",
        "Main Cooling Coil 1 Outlet Node,  !- Component 2 Outlet Node Name",
        "Coil:Heating:Water,      !- Component 3 Object Type",
        "Main Heating Coil 1,     !- Component 3 Name",
        "Main Cooling Coil 1 Outlet Node,  !- Component 3 Inlet Node Name",
        "Main Heating Coil 1 Outlet Node,  !- Component 3 Outlet Node Name",
        "Fan:VariableVolume,      !- Component 4 Object Type",
        "Supply Fan 1,            !- Component 4 Name",
        "Main Heating Coil 1 Outlet Node,  !- Component 4 Inlet Node Name",
        "VAV Sys 1 Outlet Node;   !- Component 4 Outlet Node Name",

        "AirLoopHVAC:OutdoorAirSystem,",
        "OA Sys 1,                !- Name",
        "OA Sys 1 Controllers,    !- Controller List Name",
        "OA Sys 1 Equipment;      !- Outdoor Air Equipment List Name",

        "Coil:Cooling:Water,",
        "Main Cooling Coil 1,     !- Name",
        "CoolingCoilAvailSched,   !- Availability Schedule Name",
        "0.0033,                  !- Design Water Flow Rate {m3/s}",
        "2.284,                   !- Design Air Flow Rate {m3/s}",
        "7.222,                   !- Design Inlet Water Temperature {C}",
        "26.667,                  !- Design Inlet Air Temperature {C}",
        "14.389,                  !- Design Outlet Air Temperature {C}",
        "0.0167,                  !- Design Inlet Air Humidity Ratio {kgWater/kgDryAir}",
        "0.0099,                  !- Design Outlet Air Humidity Ratio {kgWater/kgDryAir}",
        "Main Cooling Coil 1 Water Inlet Node,  !- Water Inlet Node Name",
        "Main Cooling Coil 1 Water Outlet Node,  !- Water Outlet Node Name",
        "Mixed Air Node 1,        !- Air Inlet Node Name",
        "Main Cooling Coil 1 Outlet Node,  !- Air Outlet Node Name",
        "SimpleAnalysis,          !- Type of Analysis",
        "CrossFlow;               !- Heat Exchanger Configuration",

        "Coil:Heating:Water,",
        "Main Heating Coil 1,     !- Name",
        "ReheatCoilAvailSched,    !- Availability Schedule Name",
        "5000.0,                  !- U-Factor Times Area Value {W/K}",
        "0.0043,                  !- Maximum Water Flow Rate {m3/s}",
        "Main Heating Coil 1 Water Inlet Node,  !- Water Inlet Node Name",
        "Main Heating Coil 1 Water Outlet Node,  !- Water Outlet Node Name",
        "Main Cooling Coil 1 Outlet Node,  !- Air Inlet Node Name",
        "Main Heating Coil 1 Outlet Node,  !- Air Outlet Node Name",
        "UFactorTimesAreaAndDesignWaterFlowRate,  !- Performance Input Method",
        "autosize,                !- Rated Capacity {W}",
        "82.2,                    !- Rated Inlet Water Temperature {C}",
        "16.6,                    !- Rated Inlet Air Temperature {C}",
        "71.1,                    !- Rated Outlet Water Temperature {C}",
        "32.2,                    !- Rated Outlet Air Temperature {C}",
        ";                        !- Rated Ratio for Air and Water Convection",

        "Fan:VariableVolume,",
        "Supply Fan 1,            !- Name",
        "FanAvailSched,           !- Availability Schedule Name",
        "0.7,                     !- Fan Total Efficiency",
        "600.0,                   !- Pressure Rise {Pa}",
        "autosize,                !- Maximum Flow Rate {m3/s}",
        "Fraction,                !- Fan Power Minimum Flow Rate Input Method",
        "0.25,                    !- Fan Power Minimum Flow Fraction",
        ",                        !- Fan Power Minimum Air Flow Rate {m3/s}",
        "0.9,                     !- Motor Efficiency",
        "1.0,                     !- Motor In Airstream Fraction",
        "0.35071223,              !- Fan Power Coefficient 1",
        "0.30850535,              !- Fan Power Coefficient 2",
        "-0.54137364,             !- Fan Power Coefficient 3",
        "0.87198823,              !- Fan Power Coefficient 4",
        "0.000,                   !- Fan Power Coefficient 5",
        "Main Heating Coil 1 Outlet Node,  !- Air Inlet Node Name",
        "VAV Sys 1 Outlet Node;   !- Air Outlet Node Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));

    static constexpr std::string_view RoutineName("GetBranchInput: ");
    std::string CurrentModuleObject = "Branch";
    int NumOfBranches = state->dataInputProcessing->inputProcessor->getNumObjectsFound(*state, CurrentModuleObject);
    int NumParams;
    int NumAlphas;           // Used to retrieve names from IDF
    int NumNumbers;          // Used to retrieve numbers from IDF
    bool IsNotOK;            // Flag to verify name
    bool IsBlank;            // Flag for blank name
    Array1D_string Alphas;   // Used to retrieve names from IDF
    Array1D_int NodeNums;    // Possible Array of Node Numbers (only 1 allowed)
    Array1D<Real64> Numbers; // Used to retrieve numbers from IDF
    Array1D_string cAlphaFields;
    Array1D_string cNumericFields;
    Array1D_bool lNumericBlanks;
    Array1D_bool lAlphaBlanks;
    int IOStat; // Could be used in the Get Routines, not currently checked

    if (NumOfBranches > 0) {
        state->dataBranchInputManager->Branch.allocate(NumOfBranches);
        for (auto &e : state->dataBranchInputManager->Branch) {
            e.AssignedLoopName.clear();
        }
        state->dataInputProcessing->inputProcessor->getObjectDefMaxArgs(*state, "NodeList", NumParams, NumAlphas, NumNumbers);
        NodeNums.dimension(NumParams, 0);
        state->dataInputProcessing->inputProcessor->getObjectDefMaxArgs(*state, CurrentModuleObject, NumParams, NumAlphas, NumNumbers);
        Alphas.allocate(NumAlphas);
        Numbers.dimension(NumNumbers, 0.0);
        cAlphaFields.allocate(NumAlphas);
        cNumericFields.allocate(NumNumbers);
        lAlphaBlanks.dimension(NumAlphas, true);
        lNumericBlanks.dimension(NumNumbers, true);
        int BCount = 0;
        for (int Count = 1; Count <= NumOfBranches; ++Count) {

            state->dataInputProcessing->inputProcessor->getObjectItem(*state,
                                                                      CurrentModuleObject,
                                                                      Count,
                                                                      Alphas,
                                                                      NumAlphas,
                                                                      Numbers,
                                                                      NumNumbers,
                                                                      IOStat,
                                                                      lNumericBlanks,
                                                                      lAlphaBlanks,
                                                                      cAlphaFields,
                                                                      cNumericFields);
            IsNotOK = false;
            IsBlank = false;
            Util::VerifyName(*state, Alphas(1), state->dataBranchInputManager->Branch, BCount, IsNotOK, IsBlank, CurrentModuleObject + " Name");
            if (IsNotOK) {
                if (IsBlank) {
                    continue;
                } else {
                    Alphas(1) = Alphas(1) + "--dup";
                }
            }
            ++BCount;

            GetSingleBranchInput(*state, RoutineName, BCount, Alphas, cAlphaFields, NumAlphas, NodeNums, lAlphaBlanks);
        }

        EXPECT_EQ(NumOfBranches, 1);

        EXPECT_TRUE(Util::SameString(Alphas(1), "VAV Sys 1 Main Branch"));

        EXPECT_TRUE(Util::SameString(Alphas(3), "AirLoopHVAC:OutdoorAirSystem"));
        EXPECT_TRUE(Util::SameString(Alphas(4), "OA Sys 1"));
        EXPECT_TRUE(Util::SameString(Alphas(5), "VAV Sys 1 Inlet Node"));
        EXPECT_TRUE(Util::SameString(Alphas(6), "Mixed Air Node 1"));

        EXPECT_TRUE(Util::SameString(Alphas(7), "Coil:Cooling:Water"));
        EXPECT_TRUE(Util::SameString(Alphas(8), "Main Cooling Coil 1"));
        EXPECT_TRUE(Util::SameString(Alphas(9), "Mixed Air Node 1"));
        EXPECT_TRUE(Util::SameString(Alphas(10), "Main Cooling Coil 1 Outlet Node"));

        EXPECT_TRUE(Util::SameString(Alphas(11), "Coil:Heating:Water"));
        EXPECT_TRUE(Util::SameString(Alphas(12), "Main Heating Coil 1"));
        EXPECT_TRUE(Util::SameString(Alphas(13), "Main Cooling Coil 1 Outlet Node"));
        EXPECT_TRUE(Util::SameString(Alphas(14), "Main Heating Coil 1 Outlet Node"));

        EXPECT_TRUE(Util::SameString(Alphas(15), "Fan:VariableVolume"));
        EXPECT_TRUE(Util::SameString(Alphas(16), "Supply Fan 1"));
        EXPECT_TRUE(Util::SameString(Alphas(17), "Main Heating Coil 1 Outlet Node"));
        EXPECT_TRUE(Util::SameString(Alphas(18), "VAV Sys 1 Outlet Node"));

        NodeNums.deallocate();
        Alphas.deallocate();
        Numbers.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();
    }
}

// EnergyPlus::BranchNodeConnections Unit Tests

TEST_F(EnergyPlusFixture, BranchInputManager_FindAirLoopBranchConnection)
{

    std::string const idf_objects = delimited_string({

        "AirLoopHVAC,",
        "  DOAS,                    !- Name",
        "  ,                        !- Controller List Name",
        "  DOAS Availability Managers,  !- Availability Manager List Name",
        "  autosize,                !- Design Supply Air Flow Rate {m3/s}",
        "  DOAS Branches,           !- Branch List Name",
        "  ,                        !- Connector List Name",
        "  DOAS Air Loop Inlet,     !- Supply Side Inlet Node Name",
        "  DOAS Return Air Outlet,  !- Demand Side Outlet Node Name",
        "  DOAS Supply Path Inlet,  !- Demand Side Inlet Node Names",
        "  DOAS Supply Fan Outlet;  !- Supply Side Outlet Node Names",

        "AirLoopHVAC,",
        "  Air Loop 1,                    !- Name",
        "  ,                        !- Controller List Name",
        "  Air Loop 1 Availability Managers,  !- Availability Manager List Name",
        "  50.0,                !- Design Supply Air Flow Rate {m3/s}",
        "  Air Loop 1 Branches,           !- Branch List Name",
        "  ,                        !- Connector List Name",
        "  Air Loop 1 Air Loop Inlet,     !- Supply Side Inlet Node Name",
        "  Air Loop 1 Return Air Outlet,  !- Demand Side Outlet Node Name",
        "  Air Loop 1 Supply Path Inlet,  !- Demand Side Inlet Node Names",
        "  Air Loop 1 Supply Fan Outlet;  !- Supply Side Outlet Node Names",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    std::string BranchListName;
    std::string FoundLoopName;
    int FoundLoopNum;
    std::string LoopType;
    Real64 FoundLoopVolFlowRate;
    bool MatchedLoop;

    // Case 1 Find Air Loop 1 Branches
    // Note the strings need to be uppercase at this point
    BranchListName = "AIR LOOP 1 BRANCHES";
    FoundLoopName = "None";
    FoundLoopNum = 0;
    LoopType = "None";
    FoundLoopVolFlowRate = 0.0;
    MatchedLoop = false;

    FindAirLoopBranchConnection(*state, BranchListName, FoundLoopName, FoundLoopNum, LoopType, FoundLoopVolFlowRate, MatchedLoop);

    EXPECT_EQ("AIR LOOP 1", FoundLoopName);
    EXPECT_EQ(2, FoundLoopNum);
    EXPECT_EQ("Air", LoopType);
    EXPECT_EQ(50.0, FoundLoopVolFlowRate);
    EXPECT_TRUE(MatchedLoop);

    // Case 2 Find DOAS Branches
    BranchListName = "DOAS BRANCHES";
    FoundLoopName = "None";
    FoundLoopNum = 0;
    LoopType = "None";
    FoundLoopVolFlowRate = 0.0;
    MatchedLoop = false;

    FindAirLoopBranchConnection(*state, BranchListName, FoundLoopName, FoundLoopNum, LoopType, FoundLoopVolFlowRate, MatchedLoop);

    EXPECT_EQ("DOAS", FoundLoopName);
    EXPECT_EQ(1, FoundLoopNum);
    EXPECT_EQ("Air", LoopType);
    EXPECT_EQ(DataSizing::AutoSize, FoundLoopVolFlowRate);
    EXPECT_TRUE(MatchedLoop);

    // Case 3 Not found
    BranchListName = "Not There";
    FoundLoopName = "None";
    FoundLoopNum = 0;
    LoopType = "None";
    FoundLoopVolFlowRate = 0.0;
    MatchedLoop = false;

    FindAirLoopBranchConnection(*state, BranchListName, FoundLoopName, FoundLoopNum, LoopType, FoundLoopVolFlowRate, MatchedLoop);

    EXPECT_EQ("None", FoundLoopName);
    EXPECT_EQ(0, FoundLoopNum);
    EXPECT_EQ("None", LoopType);
    EXPECT_EQ(0.0, FoundLoopVolFlowRate);
    EXPECT_FALSE(MatchedLoop);
}

TEST_F(EnergyPlusFixture, BranchInputManager_GetAirBranchIndex)
{

    std::string const idf_objects = delimited_string({

        "Branch,",
        "  DOAS Main Branch,        !- Name",
        "  ,                        !- Pressure Drop Curve Name",
        "  AirLoopHVAC:OutdoorAirSystem,  !- Component 1 Object Type",
        "  DOAS OA System,          !- Component 1 Name",
        "  DOAS Air Loop Inlet,     !- Component 1 Inlet Node Name",
        "  DOAS Mixed Air Outlet,   !- Component 1 Outlet Node Name",
        "  CoilSystem:Cooling:DX,   !- Component 2 Object Type",
        "  DOAS Cooling Coil,       !- Component 2 Name",
        "  DOAS Mixed Air Outlet,   !- Component 2 Inlet Node Name",
        "  DOAS Cooling Coil Outlet,!- Component 2 Outlet Node Name",
        "  Coil:Heating:Fuel,        !- Component 2 Object Type",
        "  DOAS Heating Coil,       !- Component 2 Name",
        "  DOAS Cooling Coil Outlet,  !- Component 2 Inlet Node Name",
        "  DOAS Heating Coil Outlet,!- Component 2 Outlet Node Name",
        "  Fan:VariableVolume,      !- Component 3 Object Type",
        "  DOAS Supply Fan,         !- Component 3 Name",
        "  DOAS Heating Coil Outlet,!- Component 3 Inlet Node Name",
        "  DOAS Supply Fan Outlet;  !- Component 3 Outlet Node Name",

        "  Branch,",
        "    TowerWaterSys Demand Bypass Branch,  !- Name",
        "    ,                        !- Pressure Drop Curve Name",
        "    Pipe:Adiabatic,          !- Component 1 Object Type",
        "    TowerWaterSys Demand Bypass Pipe,  !- Component 1 Name",
        "    TowerWaterSys Demand Bypass Pipe Inlet Node,  !- Component 1 Inlet Node Name",
        "    TowerWaterSys Demand Bypass Pipe Outlet Node;  !- Component 1 Outlet Node Name",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    std::string CompType;
    std::string CompName;
    int BranchIndex;

    // Case 1 Find OA System on DOAS branch
    // Note the strings need to be uppercase at this point
    CompType = "AIRLOOPHVAC:OUTDOORAIRSYSTEM";
    CompName = "DOAS OA SYSTEM";

    BranchIndex = GetAirBranchIndex(*state, CompType, CompName);

    EXPECT_EQ(1, BranchIndex);

    // Case 3 Find pipe
    CompType = "PIPE:ADIABATIC";
    CompName = "TOWERWATERSYS DEMAND BYPASS PIPE";

    BranchIndex = GetAirBranchIndex(*state, CompType, CompName);

    EXPECT_EQ(2, BranchIndex);

    // Case 4 Not found
    CompType = "PIPE:ADIABATIC";
    CompName = "TOWERWATERSYS DEMAND BYPASS PIPE NOT THERE";

    BranchIndex = GetAirBranchIndex(*state, CompType, CompName);

    EXPECT_EQ(0, BranchIndex);
}

} // namespace EnergyPlus
