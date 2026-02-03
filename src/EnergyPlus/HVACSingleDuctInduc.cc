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

// C++ Headers
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/HVACSingleDuctInduc.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/MixerComponent.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterCoils.hh>

namespace EnergyPlus {

namespace HVACSingleDuctInduc {

    // Module containing routines dealing terminal 4 pipe induction terminal units

    // MODULE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   June 15 2004
    //       MODIFIED       Brent Griffith, Sept 2010, plant upgrades, fluid props

    // PURPOSE OF THIS MODULE:
    // To encapsulate the data and algorithms needed to simulate 4 pipe induction terminal units

    // METHODOLOGY EMPLOYED:
    // The terminal boxes are modeled as compound components: heating coil, cooling coil and
    // mixer. The combined components are controlled to meet the zone load.

    void SimIndUnit(EnergyPlusData &state,
                    std::string_view CompName,     // name of the terminal unit
                    bool const FirstHVACIteration, // TRUE if first HVAC iteration in time step
                    int const ZoneNum,             // index of zone served by the terminal unit
                    int const ZoneNodeNum,         // zone node number of zone served by the terminal unit
                    int &CompIndex                 // which terminal unit in data structure
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   June 18 2004

        // PURPOSE OF THIS SUBROUTINE:
        // Manages the simulation of a passive (no fan) induction terminal unit.
        // Called from SimZoneAirLoopEquipment in module ZoneAirLoopEquipmentManager.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int IUNum; // index of terminal unit being simulated

        // First time SimIndUnit is called, get the input for all the passive terminal induction units
        if (state.dataHVACSingleDuctInduc->GetIUInputFlag) {
            GetIndUnits(state);
            state.dataHVACSingleDuctInduc->GetIUInputFlag = false;
        }

        // Get the induction unit index
        if (CompIndex == 0) {
            IUNum = Util::FindItemInList(CompName, state.dataHVACSingleDuctInduc->IndUnit);
            if (IUNum == 0) {
                ShowFatalError(state, format("SimIndUnit: Induction Unit not found={}", CompName));
            }
            CompIndex = IUNum;
        } else {
            IUNum = CompIndex;
            if (IUNum > state.dataHVACSingleDuctInduc->NumIndUnits || IUNum < 1) {
                ShowFatalError(state,
                               format("SimIndUnit: Invalid CompIndex passed={}, Number of Induction Units={}, System name={}",
                                      CompIndex,
                                      state.dataHVACSingleDuctInduc->NumIndUnits,
                                      CompName));
            }
            if (state.dataHVACSingleDuctInduc->CheckEquipName(IUNum)) {
                if (CompName != state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name) {
                    ShowFatalError(state,
                                   format("SimIndUnit: Invalid CompIndex passed={}, Induction Unit name={}, stored Induction Unit for that index={}",
                                          CompIndex,
                                          CompName,
                                          state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name));
                }
                state.dataHVACSingleDuctInduc->CheckEquipName(IUNum) = false;
            }
        }

        auto &indUnit = state.dataHVACSingleDuctInduc->IndUnit(IUNum);

        state.dataSize->CurTermUnitSizingNum = state.dataDefineEquipment->AirDistUnit(indUnit.ADUNum).TermUnitSizingNum;
        // initialize the unit
        InitIndUnit(state, IUNum, FirstHVACIteration);

        state.dataSize->TermUnitIU = true;

        // Select the correct unit type
        switch (indUnit.UnitType_Num) {
        case SingleDuct_CV::FourPipeInduc: {
            SimFourPipeIndUnit(state, IUNum, ZoneNum, ZoneNodeNum, FirstHVACIteration);
        } break;
        default: {
            ShowSevereError(state, format("Illegal Induction Unit Type used={}", indUnit.UnitType));
            ShowContinueError(state, format("Occurs in Induction Unit={}", indUnit.Name));
            ShowFatalError(state, "Preceding condition causes termination.");
        } break;
        }

        state.dataSize->TermUnitIU = false;

        // the tasks usually done by the Update and Report routines are not required in a compound terminal unit.

        // Update the current unit's outlet nodes. No update needed

        // Fill the report variables. There are no report variables
        indUnit.ReportIndUnit(state);
    }

    void GetIndUnits(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   June 15 2004

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains input data for passive induction air terminal units and stores it in the
        // induction terminal unit data structures

        // METHODOLOGY EMPLOYED:
        // Uses "Get" routines to read in data.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetIndUnits "); // include trailing blank space
        static constexpr std::string_view routineName = "GetIndUnits";

        Array1D_string Alphas;         // Alpha input items for object
        Array1D_string cAlphaFields;   // Alpha field names
        Array1D_string cNumericFields; // Numeric field names
        Array1D<Real64> Numbers;       // Numeric input items for object
        Array1D_bool lAlphaBlanks;     // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks;   // Logical array, numeric field input BLANK = .TRUE.
        int NumAlphas(0);              // Number of Alphas for each GetObjectItem call
        int NumNumbers(0);             // Number of Numbers for each GetObjectItem call
        int TotalArgs(0);              // Total number of alpha and numeric arguments (max) for a
        //  certain object in the input file
        int IOStatus;            // Used in GetObjectItem
        bool ErrorsFound(false); // Set to true if errors in input, fatal at end of routine

        // find the number of each type of induction unit
        std::string CurrentModuleObject = "AirTerminal:SingleDuct:ConstantVolume:FourPipeInduction";
        state.dataHVACSingleDuctInduc->NumFourPipes = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        state.dataHVACSingleDuctInduc->NumIndUnits = state.dataHVACSingleDuctInduc->NumFourPipes;
        // allocate the data structures
        state.dataHVACSingleDuctInduc->IndUnit.allocate(state.dataHVACSingleDuctInduc->NumIndUnits);
        state.dataHVACSingleDuctInduc->CheckEquipName.dimension(state.dataHVACSingleDuctInduc->NumIndUnits, true);

        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, TotalArgs, NumAlphas, NumNumbers);

        Alphas.allocate(NumAlphas);
        cAlphaFields.allocate(NumAlphas);
        cNumericFields.allocate(NumNumbers);
        Numbers.dimension(NumNumbers, 0.0);
        lAlphaBlanks.dimension(NumAlphas, true);
        lNumericBlanks.dimension(NumNumbers, true);

        // loop over Series PIUs; get and load the input data
        for (int IUIndex = 1; IUIndex <= state.dataHVACSingleDuctInduc->NumFourPipes; ++IUIndex) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     IUIndex,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            int IUNum = IUIndex;
            Util::IsNameEmpty(state, Alphas(1), CurrentModuleObject, ErrorsFound);

            state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name = Alphas(1);
            state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType = CurrentModuleObject;
            state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType_Num = SingleDuct_CV::FourPipeInduc;

            if (lAlphaBlanks(2)) {
                state.dataHVACSingleDuctInduc->IndUnit(IUNum).availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((state.dataHVACSingleDuctInduc->IndUnit(IUNum).availSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ErrorsFound = true;
            }
            state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxTotAirVolFlow = Numbers(1);
            state.dataHVACSingleDuctInduc->IndUnit(IUNum).InducRatio = Numbers(2);
            if (lNumericBlanks(2)) {
                state.dataHVACSingleDuctInduc->IndUnit(IUNum).InducRatio = 2.5;
            }

            state.dataHVACSingleDuctInduc->IndUnit(IUNum).PriAirInNode =
                GetOnlySingleNode(state,
                                  Alphas(3),
                                  ErrorsFound,
                                  DataLoopNode::ConnectionObjectType::AirTerminalSingleDuctConstantVolumeFourPipeInduction,
                                  Alphas(1),
                                  DataLoopNode::NodeFluidType::Air,
                                  DataLoopNode::ConnectionType::Inlet,
                                  NodeInputManager::CompFluidStream::Primary,
                                  DataLoopNode::ObjectIsParent,
                                  cAlphaFields(3));
            state.dataHVACSingleDuctInduc->IndUnit(IUNum).SecAirInNode =
                GetOnlySingleNode(state,
                                  Alphas(4),
                                  ErrorsFound,
                                  DataLoopNode::ConnectionObjectType::AirTerminalSingleDuctConstantVolumeFourPipeInduction,
                                  Alphas(1),
                                  DataLoopNode::NodeFluidType::Air,
                                  DataLoopNode::ConnectionType::Inlet,
                                  NodeInputManager::CompFluidStream::Primary,
                                  DataLoopNode::ObjectIsParent,
                                  cAlphaFields(4));
            state.dataHVACSingleDuctInduc->IndUnit(IUNum).OutAirNode =
                GetOnlySingleNode(state,
                                  Alphas(5),
                                  ErrorsFound,
                                  DataLoopNode::ConnectionObjectType::AirTerminalSingleDuctConstantVolumeFourPipeInduction,
                                  Alphas(1),
                                  DataLoopNode::NodeFluidType::Air,
                                  DataLoopNode::ConnectionType::Outlet,
                                  NodeInputManager::CompFluidStream::Primary,
                                  DataLoopNode::ObjectIsParent,
                                  cAlphaFields(5));

            state.dataHVACSingleDuctInduc->IndUnit(IUNum).HCoilType = Alphas(6); // type (key) of heating coil
            if (Util::SameString(state.dataHVACSingleDuctInduc->IndUnit(IUNum).HCoilType, "Coil:Heating:Water")) {
                state.dataHVACSingleDuctInduc->IndUnit(IUNum).HeatingCoilType = DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
            }

            state.dataHVACSingleDuctInduc->IndUnit(IUNum).HCoil = Alphas(7); // name of heating coil object
            bool IsNotOK = false;
            state.dataHVACSingleDuctInduc->IndUnit(IUNum).HWControlNode = WaterCoils::GetCoilWaterInletNode(
                state, state.dataHVACSingleDuctInduc->IndUnit(IUNum).HCoilType, state.dataHVACSingleDuctInduc->IndUnit(IUNum).HCoil, IsNotOK);
            if (IsNotOK) {
                ShowContinueError(state, format("In {} = {}", CurrentModuleObject, state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name));
                ShowContinueError(state, "..Only Coil:Heating:Water is allowed.");
                ErrorsFound = true;
            }
            state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxVolHotWaterFlow = Numbers(3);
            state.dataHVACSingleDuctInduc->IndUnit(IUNum).MinVolHotWaterFlow = Numbers(4);
            state.dataHVACSingleDuctInduc->IndUnit(IUNum).HotControlOffset = Numbers(5);

            state.dataHVACSingleDuctInduc->IndUnit(IUNum).CCoilType = Alphas(8); // type (key) of cooling coil

            if (Util::SameString(state.dataHVACSingleDuctInduc->IndUnit(IUNum).CCoilType, "Coil:Cooling:Water")) {
                state.dataHVACSingleDuctInduc->IndUnit(IUNum).CoolingCoilType = DataPlant::PlantEquipmentType::CoilWaterCooling;
            } else if (Util::SameString(state.dataHVACSingleDuctInduc->IndUnit(IUNum).CCoilType, "Coil:Cooling:Water:DetailedGeometry")) {
                state.dataHVACSingleDuctInduc->IndUnit(IUNum).CoolingCoilType = DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling;
            }

            state.dataHVACSingleDuctInduc->IndUnit(IUNum).CCoil = Alphas(9); // name of cooling coil object
            IsNotOK = false;
            state.dataHVACSingleDuctInduc->IndUnit(IUNum).CWControlNode = WaterCoils::GetCoilWaterInletNode(
                state, state.dataHVACSingleDuctInduc->IndUnit(IUNum).CCoilType, state.dataHVACSingleDuctInduc->IndUnit(IUNum).CCoil, IsNotOK);
            if (IsNotOK) {
                ShowContinueError(state, format("In {} = {}", CurrentModuleObject, state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name));
                ShowContinueError(state, "..Only Coil:Cooling:Water or Coil:Cooling:Water:DetailedGeometry is allowed.");
                ErrorsFound = true;
            }
            state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxVolColdWaterFlow = Numbers(6);
            state.dataHVACSingleDuctInduc->IndUnit(IUNum).MinVolColdWaterFlow = Numbers(7);
            state.dataHVACSingleDuctInduc->IndUnit(IUNum).ColdControlOffset = Numbers(8);

            // Get the Zone Mixer name and check that it is OK
            bool errFlag = false;
            state.dataHVACSingleDuctInduc->IndUnit(IUNum).MixerName = Alphas(10);
            MixerComponent::GetZoneMixerIndex(state,
                                              state.dataHVACSingleDuctInduc->IndUnit(IUNum).MixerName,
                                              state.dataHVACSingleDuctInduc->IndUnit(IUNum).Mixer_Num,
                                              errFlag,
                                              CurrentModuleObject);
            if (errFlag) {
                ShowContinueError(state, format("...specified in {} = {}", CurrentModuleObject, state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name));
                ErrorsFound = true;
            }

            // Add heating coil to component sets array
            BranchNodeConnections::SetUpCompSets(state,
                                                 state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                                 state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name,
                                                 state.dataHVACSingleDuctInduc->IndUnit(IUNum).HCoilType,
                                                 state.dataHVACSingleDuctInduc->IndUnit(IUNum).HCoil,
                                                 Alphas(4),
                                                 "UNDEFINED");
            // Add cooling coil to component sets array
            BranchNodeConnections::SetUpCompSets(state,
                                                 state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                                 state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name,
                                                 state.dataHVACSingleDuctInduc->IndUnit(IUNum).CCoilType,
                                                 state.dataHVACSingleDuctInduc->IndUnit(IUNum).CCoil,
                                                 "UNDEFINED",
                                                 "UNDEFINED");

            // Register component set data
            BranchNodeConnections::TestCompSet(state,
                                               state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                               state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name,
                                               state.dataLoopNodes->NodeID(state.dataHVACSingleDuctInduc->IndUnit(IUNum).PriAirInNode),
                                               state.dataLoopNodes->NodeID(state.dataHVACSingleDuctInduc->IndUnit(IUNum).OutAirNode),
                                               "Air Nodes");

            for (int ADUNum = 1; ADUNum <= (int)state.dataDefineEquipment->AirDistUnit.size(); ++ADUNum) {
                if (state.dataHVACSingleDuctInduc->IndUnit(IUNum).OutAirNode == state.dataDefineEquipment->AirDistUnit(ADUNum).OutletNodeNum) {
                    state.dataHVACSingleDuctInduc->IndUnit(IUNum).ADUNum = ADUNum;
                }
            }
            // one assumes if there isn't one assigned, it's an error?
            if (state.dataHVACSingleDuctInduc->IndUnit(IUNum).ADUNum == 0) {
                ShowSevereError(state,
                                format("{}No matching Air Distribution Unit, for Unit = [{},{}].",
                                       RoutineName,
                                       state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                       state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name));
                ShowContinueError(
                    state,
                    format("...should have outlet node={}", state.dataLoopNodes->NodeID(state.dataHVACSingleDuctInduc->IndUnit(IUNum).OutAirNode)));
                ErrorsFound = true;
            } else {
                // Fill the Zone Equipment data with the supply air inlet node number of this unit.
                bool AirNodeFound = false;
                for (int CtrlZone = 1; CtrlZone <= state.dataGlobal->NumOfZones; ++CtrlZone) {
                    auto &zoneEquipConfig = state.dataZoneEquip->ZoneEquipConfig(CtrlZone);

                    if (!zoneEquipConfig.IsControlled) {
                        continue;
                    }
                    for (int SupAirIn = 1; SupAirIn <= zoneEquipConfig.NumInletNodes; ++SupAirIn) {
                        if (state.dataHVACSingleDuctInduc->IndUnit(IUNum).OutAirNode == zoneEquipConfig.InletNode(SupAirIn)) {
                            if (zoneEquipConfig.AirDistUnitCool(SupAirIn).OutNode > 0) {
                                ShowSevereError(state, "Error in connecting a terminal unit to a zone");
                                ShowContinueError(state,
                                                  format("{} already connects to another zone",
                                                         state.dataLoopNodes->NodeID(state.dataHVACSingleDuctInduc->IndUnit(IUNum).OutAirNode)));
                                ShowContinueError(state,
                                                  format("Occurs for terminal unit {} = {}",
                                                         state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                                         state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name));
                                ShowContinueError(state, "Check terminal unit node names for errors");
                                ErrorsFound = true;
                            } else {
                                zoneEquipConfig.AirDistUnitCool(SupAirIn).InNode = state.dataHVACSingleDuctInduc->IndUnit(IUNum).PriAirInNode;
                                zoneEquipConfig.AirDistUnitCool(SupAirIn).OutNode = state.dataHVACSingleDuctInduc->IndUnit(IUNum).OutAirNode;
                                state.dataDefineEquipment->AirDistUnit(state.dataHVACSingleDuctInduc->IndUnit(IUNum).ADUNum).TermUnitSizingNum =
                                    zoneEquipConfig.AirDistUnitCool(SupAirIn).TermUnitSizingIndex;
                                state.dataDefineEquipment->AirDistUnit(state.dataHVACSingleDuctInduc->IndUnit(IUNum).ADUNum).ZoneEqNum = CtrlZone;
                                state.dataHVACSingleDuctInduc->IndUnit(IUNum).CtrlZoneNum = CtrlZone;
                            }
                            state.dataHVACSingleDuctInduc->IndUnit(IUNum).CtrlZoneInNodeIndex = SupAirIn;
                            AirNodeFound = true;
                            break;
                        }
                    }
                }
                if (!AirNodeFound) {
                    ShowSevereError(
                        state,
                        format("The outlet air node from the {} = {}", CurrentModuleObject, state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name));
                    ShowContinueError(state, format("did not have a matching Zone Equipment Inlet Node, Node ={}", Alphas(3)));
                    ErrorsFound = true;
                }
            }
            // report variable for all single duct air terminals
            SetupOutputVariable(state,
                                "Zone Air Terminal Outdoor Air Volume Flow Rate",
                                Constant::Units::m3_s,
                                state.dataHVACSingleDuctInduc->IndUnit(IUNum).OutdoorAirFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name);
        }

        Alphas.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        Numbers.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();
        if (ErrorsFound) {
            ShowFatalError(state, format("{}Errors found in getting input. Preceding conditions cause termination.", RoutineName));
        }
    }

    void InitIndUnit(EnergyPlusData &state,
                     int const IUNum,              // number of the current induction unit being simulated
                     bool const FirstHVACIteration // TRUE if first air loop solution this HVAC step
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   June 21 2004

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initialization of the passive induction terminal boxes

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("InitIndUnit");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int PriNode;   // primary air inlet node number
        int SecNode;   // secondary air inlet node number
        Real64 IndRat; // unit induction ratio
        Real64 RhoAir; // air density at outside pressure and standard temperature and humidity
        Real64 rho;    // local fluid density

        auto &ZoneEquipmentListChecked = state.dataHVACSingleDuctInduc->ZoneEquipmentListChecked;

        // Do the one time initializations
        if (state.dataHVACSingleDuctInduc->MyOneTimeFlag) {

            state.dataHVACSingleDuctInduc->MyEnvrnFlag.allocate(state.dataHVACSingleDuctInduc->NumIndUnits);
            state.dataHVACSingleDuctInduc->MySizeFlag.allocate(state.dataHVACSingleDuctInduc->NumIndUnits);
            state.dataHVACSingleDuctInduc->MyPlantScanFlag.allocate(state.dataHVACSingleDuctInduc->NumIndUnits);
            state.dataHVACSingleDuctInduc->MyAirDistInitFlag.allocate(state.dataHVACSingleDuctInduc->NumIndUnits);
            state.dataHVACSingleDuctInduc->MyEnvrnFlag = true;
            state.dataHVACSingleDuctInduc->MySizeFlag = true;
            state.dataHVACSingleDuctInduc->MyPlantScanFlag = true;
            state.dataHVACSingleDuctInduc->MyAirDistInitFlag = true;
            state.dataHVACSingleDuctInduc->MyOneTimeFlag = false;
        }

        if (state.dataHVACSingleDuctInduc->MyPlantScanFlag(IUNum) && allocated(state.dataPlnt->PlantLoop)) {
            bool errFlag = false;
            if (state.dataHVACSingleDuctInduc->IndUnit(IUNum).HeatingCoilType == DataPlant::PlantEquipmentType::CoilWaterSimpleHeating) {
                errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(state,
                                                        state.dataHVACSingleDuctInduc->IndUnit(IUNum).HCoil,
                                                        state.dataHVACSingleDuctInduc->IndUnit(IUNum).HeatingCoilType,
                                                        state.dataHVACSingleDuctInduc->IndUnit(IUNum).HWPlantLoc,
                                                        errFlag,
                                                        _,
                                                        _,
                                                        _,
                                                        _,
                                                        _);
            }
            if (errFlag) {
                ShowContinueError(state,
                                  format("Reference Unit=\"{}\", type={}",
                                         state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name,
                                         state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType));
            }
            if (state.dataHVACSingleDuctInduc->IndUnit(IUNum).CoolingCoilType == DataPlant::PlantEquipmentType::CoilWaterCooling ||
                state.dataHVACSingleDuctInduc->IndUnit(IUNum).CoolingCoilType == DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling) {
                errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(state,
                                                        state.dataHVACSingleDuctInduc->IndUnit(IUNum).CCoil,
                                                        state.dataHVACSingleDuctInduc->IndUnit(IUNum).CoolingCoilType,
                                                        state.dataHVACSingleDuctInduc->IndUnit(IUNum).CWPlantLoc,
                                                        errFlag,
                                                        _,
                                                        _,
                                                        _,
                                                        _,
                                                        _);
            }
            if (errFlag) {
                ShowContinueError(state,
                                  format("Reference Unit=\"{}\", type={}",
                                         state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name,
                                         state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType));
                ShowFatalError(state, "InitIndUnit: Program terminated for previous conditions.");
            }
            state.dataHVACSingleDuctInduc->MyPlantScanFlag(IUNum) = false;
        } else if (state.dataHVACSingleDuctInduc->MyPlantScanFlag(IUNum) && !state.dataGlobal->AnyPlantInModel) {
            state.dataHVACSingleDuctInduc->MyPlantScanFlag(IUNum) = false;
        }

        if (state.dataHVACSingleDuctInduc->MyAirDistInitFlag(IUNum)) {
            // save the induction ratio in the term unit sizing array for use in the system sizing calculation
            if (state.dataSize->CurTermUnitSizingNum > 0) {
                state.dataSize->TermUnitSizing(state.dataSize->CurTermUnitSizingNum).InducRat =
                    state.dataHVACSingleDuctInduc->IndUnit(IUNum).InducRatio;
            }
            if (state.dataHVACSingleDuctInduc->IndUnit(IUNum).AirLoopNum == 0) {
                if ((state.dataHVACSingleDuctInduc->IndUnit(IUNum).CtrlZoneNum > 0) &&
                    (state.dataHVACSingleDuctInduc->IndUnit(IUNum).CtrlZoneInNodeIndex > 0)) {
                    state.dataHVACSingleDuctInduc->IndUnit(IUNum).AirLoopNum =
                        state.dataZoneEquip->ZoneEquipConfig(state.dataHVACSingleDuctInduc->IndUnit(IUNum).CtrlZoneNum)
                            .InletNodeAirLoopNum(state.dataHVACSingleDuctInduc->IndUnit(IUNum).CtrlZoneInNodeIndex);
                    state.dataDefineEquipment->AirDistUnit(state.dataHVACSingleDuctInduc->IndUnit(IUNum).ADUNum).AirLoopNum =
                        state.dataHVACSingleDuctInduc->IndUnit(IUNum).AirLoopNum;
                }
            } else {
                state.dataHVACSingleDuctInduc->MyAirDistInitFlag(IUNum) = false;
            }
        }
        if (!ZoneEquipmentListChecked && state.dataZoneEquip->ZoneEquipInputsFilled) {
            ZoneEquipmentListChecked = true;
            // Check to see if there is a Air Distribution Unit on the Zone Equipment List
            for (int Loop = 1; Loop <= state.dataHVACSingleDuctInduc->NumIndUnits; ++Loop) {
                if (state.dataHVACSingleDuctInduc->IndUnit(Loop).ADUNum == 0) {
                    continue;
                }
                if (DataZoneEquipment::CheckZoneEquipmentList(
                        state,
                        "ZONEHVAC:AIRDISTRIBUTIONUNIT",
                        state.dataDefineEquipment->AirDistUnit(state.dataHVACSingleDuctInduc->IndUnit(Loop).ADUNum).Name)) {
                    continue;
                }
                ShowSevereError(state,
                                format("InitIndUnit: ADU=[Air Distribution Unit,{}] is not on any ZoneHVAC:EquipmentList.",
                                       state.dataDefineEquipment->AirDistUnit(state.dataHVACSingleDuctInduc->IndUnit(Loop).ADUNum).Name));
                ShowContinueError(state,
                                  format("...Unit=[{},{}] will not be simulated.",
                                         state.dataHVACSingleDuctInduc->IndUnit(Loop).UnitType,
                                         state.dataHVACSingleDuctInduc->IndUnit(Loop).Name));
            }
        }

        if (!state.dataGlobal->SysSizingCalc && state.dataHVACSingleDuctInduc->MySizeFlag(IUNum)) {

            SizeIndUnit(state, IUNum);
            state.dataHVACSingleDuctInduc->MySizeFlag(IUNum) = false;
        }

        // Do the Begin Environment initializations
        if (state.dataGlobal->BeginEnvrnFlag && state.dataHVACSingleDuctInduc->MyEnvrnFlag(IUNum)) {
            RhoAir = state.dataEnvrn->StdRhoAir;
            PriNode = state.dataHVACSingleDuctInduc->IndUnit(IUNum).PriAirInNode;
            SecNode = state.dataHVACSingleDuctInduc->IndUnit(IUNum).SecAirInNode;
            int OutletNode = state.dataHVACSingleDuctInduc->IndUnit(IUNum).OutAirNode;
            IndRat = state.dataHVACSingleDuctInduc->IndUnit(IUNum).InducRatio;
            // set the mass flow rates from the input volume flow rates
            if (Util::SameString(state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType, "AirTerminal:SingleDuct:ConstantVolume:FourPipeInduction")) {
                state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxTotAirMassFlow =
                    RhoAir * state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxTotAirVolFlow;
                state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxPriAirMassFlow =
                    state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxTotAirMassFlow / (1.0 + IndRat);
                state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxSecAirMassFlow =
                    IndRat * state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxTotAirMassFlow / (1.0 + IndRat);
                state.dataLoopNodes->Node(PriNode).MassFlowRateMax = state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxPriAirMassFlow;
                state.dataLoopNodes->Node(PriNode).MassFlowRateMin = state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxPriAirMassFlow;
                state.dataLoopNodes->Node(SecNode).MassFlowRateMax = state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxSecAirMassFlow;
                state.dataLoopNodes->Node(SecNode).MassFlowRateMin = state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxSecAirMassFlow;
                state.dataLoopNodes->Node(OutletNode).MassFlowRateMax = state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxTotAirMassFlow;
            }

            int HotConNode = state.dataHVACSingleDuctInduc->IndUnit(IUNum).HWControlNode;
            if (HotConNode > 0 && !state.dataHVACSingleDuctInduc->MyPlantScanFlag(IUNum)) {

                rho = state.dataPlnt->PlantLoop(state.dataHVACSingleDuctInduc->IndUnit(IUNum).HWPlantLoc.loopNum)
                          .glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxHotWaterFlow =
                    rho * state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxVolHotWaterFlow;
                state.dataHVACSingleDuctInduc->IndUnit(IUNum).MinHotWaterFlow =
                    rho * state.dataHVACSingleDuctInduc->IndUnit(IUNum).MinVolHotWaterFlow;
                // get component outlet node from plant structure
                int HWOutletNode = DataPlant::CompData::getPlantComponent(state, state.dataHVACSingleDuctInduc->IndUnit(IUNum).HWPlantLoc).NodeNumOut;
                PlantUtilities::InitComponentNodes(state,
                                                   state.dataHVACSingleDuctInduc->IndUnit(IUNum).MinHotWaterFlow,
                                                   state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxHotWaterFlow,
                                                   HotConNode,
                                                   HWOutletNode);
            }

            int ColdConNode = state.dataHVACSingleDuctInduc->IndUnit(IUNum).CWControlNode;
            if (ColdConNode > 0) {
                rho = state.dataPlnt->PlantLoop(state.dataHVACSingleDuctInduc->IndUnit(IUNum).CWPlantLoc.loopNum)
                          .glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);
                state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxColdWaterFlow =
                    rho * state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxVolColdWaterFlow;
                state.dataHVACSingleDuctInduc->IndUnit(IUNum).MinColdWaterFlow =
                    rho * state.dataHVACSingleDuctInduc->IndUnit(IUNum).MinVolColdWaterFlow;

                int CWOutletNode = DataPlant::CompData::getPlantComponent(state, state.dataHVACSingleDuctInduc->IndUnit(IUNum).CWPlantLoc).NodeNumOut;
                PlantUtilities::InitComponentNodes(state,
                                                   state.dataHVACSingleDuctInduc->IndUnit(IUNum).MinColdWaterFlow,
                                                   state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxColdWaterFlow,
                                                   ColdConNode,
                                                   CWOutletNode);
            }

            state.dataHVACSingleDuctInduc->MyEnvrnFlag(IUNum) = false;
        } // end one time inits

        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataHVACSingleDuctInduc->MyEnvrnFlag(IUNum) = true;
        }

        PriNode = state.dataHVACSingleDuctInduc->IndUnit(IUNum).PriAirInNode;
        SecNode = state.dataHVACSingleDuctInduc->IndUnit(IUNum).SecAirInNode;

        // Do the start of HVAC time step initializations
        if (FirstHVACIteration) {
            // check for upstream zero flow. If nonzero and schedule ON, set primary flow to max
            if (state.dataHVACSingleDuctInduc->IndUnit(IUNum).availSched->getCurrentVal() > 0.0 &&
                state.dataLoopNodes->Node(PriNode).MassFlowRate > 0.0) {
                if (Util::SameString(state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                     "AirTerminal:SingleDuct:ConstantVolume:FourPipeInduction")) {
                    state.dataLoopNodes->Node(PriNode).MassFlowRate = state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxPriAirMassFlow;
                    state.dataLoopNodes->Node(SecNode).MassFlowRate = state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxSecAirMassFlow;
                }
            } else {
                state.dataLoopNodes->Node(PriNode).MassFlowRate = 0.0;
                state.dataLoopNodes->Node(SecNode).MassFlowRate = 0.0;
            }
            // reset the max and min avail flows
            if (state.dataHVACSingleDuctInduc->IndUnit(IUNum).availSched->getCurrentVal() > 0.0 &&
                state.dataLoopNodes->Node(PriNode).MassFlowRateMaxAvail > 0.0) {
                if (Util::SameString(state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                     "AirTerminal:SingleDuct:ConstantVolume:FourPipeInduction")) {
                    state.dataLoopNodes->Node(PriNode).MassFlowRateMaxAvail = state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxPriAirMassFlow;
                    state.dataLoopNodes->Node(PriNode).MassFlowRateMinAvail = state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxPriAirMassFlow;
                    state.dataLoopNodes->Node(SecNode).MassFlowRateMaxAvail = state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxSecAirMassFlow;
                    state.dataLoopNodes->Node(SecNode).MassFlowRateMinAvail = state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxSecAirMassFlow;
                }
            } else {
                state.dataLoopNodes->Node(PriNode).MassFlowRateMaxAvail = 0.0;
                state.dataLoopNodes->Node(PriNode).MassFlowRateMinAvail = 0.0;
                state.dataLoopNodes->Node(SecNode).MassFlowRateMaxAvail = 0.0;
                state.dataLoopNodes->Node(SecNode).MassFlowRateMinAvail = 0.0;
            }
        }
    }

    void SizeIndUnit(EnergyPlusData &state, int const IUNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   June 22 2004
        //       MODIFIED       August 2013 Daeho Kang, add component sizing table entries

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing induction terminal units for which flow rates have not been
        // specified in the input

        // METHODOLOGY EMPLOYED:
        // Accesses zone sizing array for air flow rates and zone and plant sizing arrays to
        // calculate coil water flow rates.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("SizeIndUnit");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 DesCoilLoad;
        Real64 Cp;  // local fluid specific heat
        Real64 rho; // local fluid density

        Real64 DesPriVolFlow = 0.0;
        Real64 CpAir = 0.0;
        Real64 RhoAir = state.dataEnvrn->StdRhoAir;
        bool ErrorsFound = false;
        bool IsAutoSize = false;
        Real64 MaxTotAirVolFlowDes = 0.0;     // Design size maximum air volume flow for reporting
        Real64 MaxTotAirVolFlowUser = 0.0;    // User hard-sized maximum air volume flow for reporting
        Real64 MaxVolHotWaterFlowDes = 0.0;   // Design size maximum hot water flow for reporting
        Real64 MaxVolHotWaterFlowUser = 0.0;  // User hard-sized maximum hot water flow for reporting
        Real64 MaxVolColdWaterFlowDes = 0.0;  // Design size maximum cold water flow for reporting
        Real64 MaxVolColdWaterFlowUser = 0.0; // User hard-sized maximum cold water flow for reporting

        if (state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxTotAirVolFlow == DataSizing::AutoSize) {
            IsAutoSize = true;
        }

        if (state.dataSize->CurZoneEqNum > 0) {
            if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // simulation continue
                if (state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxTotAirVolFlow > 0.0) {
                    BaseSizer::reportSizerOutput(state,
                                                 state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                                 state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name,
                                                 "User-Specified Maximum Total Air Flow Rate [m3/s]",
                                                 state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxTotAirVolFlow);
                }
            } else {
                CheckZoneSizing(state, state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType, state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name);
                if (state.dataSize->CurTermUnitSizingNum > 0) {
                    MaxTotAirVolFlowDes = max(state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesCoolVolFlow,
                                              state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesHeatVolFlow);
                } else {
                    MaxTotAirVolFlowDes = 0.0;
                }
                if (MaxTotAirVolFlowDes < HVAC::SmallAirVolFlow) {
                    MaxTotAirVolFlowDes = 0.0;
                }
                if (IsAutoSize) {
                    state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxTotAirVolFlow = MaxTotAirVolFlowDes;
                    BaseSizer::reportSizerOutput(state,
                                                 state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                                 state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name,
                                                 "Design Size Maximum Total Air Flow Rate [m3/s]",
                                                 MaxTotAirVolFlowDes);
                } else {
                    if (state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxTotAirVolFlow > 0.0 && MaxTotAirVolFlowDes > 0.0) {
                        MaxTotAirVolFlowUser = state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxTotAirVolFlow;
                        BaseSizer::reportSizerOutput(state,
                                                     state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                                     state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name,
                                                     "Design Size Maximum Total Air Flow Rate [m3/s]",
                                                     MaxTotAirVolFlowDes,
                                                     "User-Specified Maximum Total Air Flow Rate [m3/s]",
                                                     MaxTotAirVolFlowUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(MaxTotAirVolFlowDes - MaxTotAirVolFlowUser) / MaxTotAirVolFlowUser) >
                                state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state,
                                            format("SizeHVACSingleDuctInduction: Potential issue with equipment sizing for {} = \"{}\".",
                                                   state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                                   state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name));
                                ShowContinueError(state, format("User-Specified Maximum Total Air Flow Rate of {:.5R} [m3/s]", MaxTotAirVolFlowUser));
                                ShowContinueError(
                                    state, format("differs from Design Size Maximum Total Air Flow Rate of {:.5R} [m3/s]", MaxTotAirVolFlowDes));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                }
            }
        }

        IsAutoSize = false;
        if (state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxVolHotWaterFlow == DataSizing::AutoSize) {
            IsAutoSize = true;
        }
        if ((state.dataSize->CurZoneEqNum > 0) && (state.dataSize->CurTermUnitSizingNum > 0)) {
            if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // simulation continue
                if (state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxVolHotWaterFlow > 0.0) {
                    BaseSizer::reportSizerOutput(state,
                                                 state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                                 state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name,
                                                 "User-Specified Maximum Hot Water Flow Rate [m3/s]",
                                                 state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxVolHotWaterFlow);
                }
            } else {
                CheckZoneSizing(state, state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType, state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name);

                if (Util::SameString(state.dataHVACSingleDuctInduc->IndUnit(IUNum).HCoilType, "Coil:Heating:Water")) {

                    int CoilWaterInletNode = WaterCoils::GetCoilWaterInletNode(
                        state, "Coil:Heating:Water", state.dataHVACSingleDuctInduc->IndUnit(IUNum).HCoil, ErrorsFound);
                    int CoilWaterOutletNode = WaterCoils::GetCoilWaterOutletNode(
                        state, "Coil:Heating:Water", state.dataHVACSingleDuctInduc->IndUnit(IUNum).HCoil, ErrorsFound);
                    if (IsAutoSize) {
                        int PltSizHeatNum = PlantUtilities::MyPlantSizingIndex(state,
                                                                               "Coil:Heating:Water",
                                                                               state.dataHVACSingleDuctInduc->IndUnit(IUNum).HCoil,
                                                                               CoilWaterInletNode,
                                                                               CoilWaterOutletNode,
                                                                               ErrorsFound);
                        if (PltSizHeatNum > 0) {

                            if (state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesHeatMassFlow >=
                                HVAC::SmallAirVolFlow) {
                                DesPriVolFlow = state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxTotAirVolFlow /
                                                (1.0 + state.dataHVACSingleDuctInduc->IndUnit(IUNum).InducRatio);
                                CpAir = Psychrometrics::PsyCpAirFnW(
                                    state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).HeatDesHumRat);
                                // the design heating coil load is the zone load minus whatever the central system does. Note that
                                // DesHeatCoilInTempTU is really the primary air inlet temperature for the unit.
                                if (state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).ZoneTempAtHeatPeak > 0.0) {
                                    DesCoilLoad =
                                        state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).NonAirSysDesHeatLoad -
                                        CpAir * RhoAir * DesPriVolFlow *
                                            (state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesHeatCoilInTempTU -
                                             state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).ZoneTempAtHeatPeak);
                                } else {
                                    DesCoilLoad = CpAir * RhoAir * DesPriVolFlow *
                                                  (state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).ZoneSizThermSetPtLo -
                                                   state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesHeatCoilInTempTU);
                                }
                                state.dataHVACSingleDuctInduc->IndUnit(IUNum).DesHeatingLoad = DesCoilLoad;
                                Cp = state.dataPlnt->PlantLoop(state.dataHVACSingleDuctInduc->IndUnit(IUNum).HWPlantLoc.loopNum)
                                         .glycol->getSpecificHeat(state, Constant::HWInitConvTemp, RoutineName);

                                rho = state.dataPlnt->PlantLoop(state.dataHVACSingleDuctInduc->IndUnit(IUNum).HWPlantLoc.loopNum)
                                          .glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);

                                MaxVolHotWaterFlowDes = DesCoilLoad / (state.dataSize->PlantSizData(PltSizHeatNum).DeltaT * Cp * rho);
                                MaxVolHotWaterFlowDes = max(MaxVolHotWaterFlowDes, 0.0);
                            } else {
                                MaxVolHotWaterFlowDes = 0.0;
                            }
                        } else {
                            ShowSevereError(state, "Autosizing of water flow requires a heating loop Sizing:Plant object");
                            ShowContinueError(state,
                                              format("Occurs in{} Object={}",
                                                     state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                                     state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name));
                            ErrorsFound = true;
                        }
                        state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxVolHotWaterFlow = MaxVolHotWaterFlowDes;
                        BaseSizer::reportSizerOutput(state,
                                                     state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                                     state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name,
                                                     "Design Size Maximum Hot Water Flow Rate [m3/s]",
                                                     MaxVolHotWaterFlowDes);
                        BaseSizer::reportSizerOutput(
                            state,
                            state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                            state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name,
                            "Design Size Inlet Air Temperature [C]",
                            state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesHeatCoilInTempTU);
                        BaseSizer::reportSizerOutput(
                            state,
                            state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                            state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name,
                            "Design Size Inlet Air Humidity Ratio [kgWater/kgDryAir]",
                            state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesHeatCoilInHumRatTU);
                    } else {
                        if (state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxVolHotWaterFlow > 0.0 && MaxVolHotWaterFlowDes > 0.0) {
                            MaxVolHotWaterFlowUser = state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxVolHotWaterFlow;
                            BaseSizer::reportSizerOutput(state,
                                                         state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                                         state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name,
                                                         "Design Size Maximum Hot Water Flow Rate [m3/s]",
                                                         MaxVolHotWaterFlowDes,
                                                         "User-Specified Maximum Hot Water Flow Rate [m3/s]",
                                                         MaxVolHotWaterFlowUser);
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                if ((std::abs(MaxVolHotWaterFlowDes - MaxVolHotWaterFlowUser) / MaxVolHotWaterFlowUser) >
                                    state.dataSize->AutoVsHardSizingThreshold) {
                                    ShowMessage(state,
                                                format("SizeHVACSingleDuctInduction: Potential issue with equipment sizing for {} = \"{}\".",
                                                       state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                                       state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name));
                                    ShowContinueError(state,
                                                      format("User-Specified Maximum Hot Water Flow Rate of {:.5R} [m3/s]", MaxVolHotWaterFlowUser));
                                    ShowContinueError(
                                        state,
                                        format("differs from Design Size Maximum Hot Water Flow Rate of {:.5R} [m3/s]", MaxVolHotWaterFlowDes));
                                    ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                    ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                                }
                            }
                        }
                    }
                } else {
                    state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxVolHotWaterFlow = 0.0;
                }
            }
        }

        IsAutoSize = false;
        if (state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxVolColdWaterFlow == DataSizing::AutoSize) {
            IsAutoSize = true;
        }
        if ((state.dataSize->CurZoneEqNum > 0) && (state.dataSize->CurTermUnitSizingNum > 0)) {
            if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // simulation continue
                if (state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxVolColdWaterFlow > 0.0) {
                    BaseSizer::reportSizerOutput(state,
                                                 state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                                 state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name,
                                                 "User-Specified Maximum Cold Water Flow Rate [m3/s]",
                                                 state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxVolColdWaterFlow);
                }
            } else {
                CheckZoneSizing(state, state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType, state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name);

                if (Util::SameString(state.dataHVACSingleDuctInduc->IndUnit(IUNum).CCoilType, "Coil:Cooling:Water") ||
                    Util::SameString(state.dataHVACSingleDuctInduc->IndUnit(IUNum).CCoilType, "Coil:Cooling:Water:DetailedGeometry")) {

                    int CoilWaterInletNode = WaterCoils::GetCoilWaterInletNode(state,
                                                                               state.dataHVACSingleDuctInduc->IndUnit(IUNum).CCoilType,
                                                                               state.dataHVACSingleDuctInduc->IndUnit(IUNum).CCoil,
                                                                               ErrorsFound);
                    int CoilWaterOutletNode = WaterCoils::GetCoilWaterOutletNode(state,
                                                                                 state.dataHVACSingleDuctInduc->IndUnit(IUNum).CCoilType,
                                                                                 state.dataHVACSingleDuctInduc->IndUnit(IUNum).CCoil,
                                                                                 ErrorsFound);
                    if (IsAutoSize) {
                        int PltSizCoolNum = PlantUtilities::MyPlantSizingIndex(state,
                                                                               state.dataHVACSingleDuctInduc->IndUnit(IUNum).CCoilType,
                                                                               state.dataHVACSingleDuctInduc->IndUnit(IUNum).CCoil,
                                                                               CoilWaterInletNode,
                                                                               CoilWaterOutletNode,
                                                                               ErrorsFound);
                        if (PltSizCoolNum > 0) {

                            if (state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesCoolMassFlow >=
                                HVAC::SmallAirVolFlow) {
                                DesPriVolFlow = state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxTotAirVolFlow /
                                                (1.0 + state.dataHVACSingleDuctInduc->IndUnit(IUNum).InducRatio);
                                CpAir = Psychrometrics::PsyCpAirFnW(
                                    state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).CoolDesHumRat);
                                // the design cooling coil load is the zone load minus whatever the central system does. Note that
                                // DesCoolCoilInTempTU is really the primary air inlet temperature for the unit.
                                if (state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).ZoneTempAtCoolPeak > 0.0) {
                                    DesCoilLoad =
                                        state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).NonAirSysDesCoolLoad -
                                        CpAir * RhoAir * DesPriVolFlow *
                                            (state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).ZoneTempAtCoolPeak -
                                             state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesCoolCoilInTempTU);
                                } else {
                                    DesCoilLoad = CpAir * RhoAir * DesPriVolFlow *
                                                  (state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesCoolCoilInTempTU -
                                                   state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).ZoneSizThermSetPtHi);
                                }
                                state.dataHVACSingleDuctInduc->IndUnit(IUNum).DesCoolingLoad = DesCoilLoad;
                                Cp = state.dataPlnt->PlantLoop(state.dataHVACSingleDuctInduc->IndUnit(IUNum).CWPlantLoc.loopNum)
                                         .glycol->getSpecificHeat(state, 5.0, RoutineName);

                                rho = state.dataPlnt->PlantLoop(state.dataHVACSingleDuctInduc->IndUnit(IUNum).CWPlantLoc.loopNum)
                                          .glycol->getDensity(state, 5.0, RoutineName);

                                MaxVolColdWaterFlowDes = DesCoilLoad / (state.dataSize->PlantSizData(PltSizCoolNum).DeltaT * Cp * rho);
                                MaxVolColdWaterFlowDes = max(MaxVolColdWaterFlowDes, 0.0);
                            } else {
                                MaxVolColdWaterFlowDes = 0.0;
                            }
                        } else {
                            ShowSevereError(state, "Autosizing of water flow requires a cooling loop Sizing:Plant object");
                            ShowContinueError(state,
                                              format("Occurs in{} Object={}",
                                                     state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                                     state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name));
                            ErrorsFound = true;
                        }
                        state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxVolColdWaterFlow = MaxVolColdWaterFlowDes;
                        BaseSizer::reportSizerOutput(state,
                                                     state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                                     state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name,
                                                     "Design Size Maximum Cold Water Flow Rate [m3/s]",
                                                     MaxVolColdWaterFlowDes);
                    } else {
                        if (state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxVolColdWaterFlow > 0.0 && MaxVolColdWaterFlowDes > 0.0) {
                            MaxVolColdWaterFlowUser = state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxVolColdWaterFlow;
                            BaseSizer::reportSizerOutput(state,
                                                         state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                                         state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name,
                                                         "Design Size Maximum Cold Water Flow Rate [m3/s]",
                                                         MaxVolColdWaterFlowDes,
                                                         "User-Specified Maximum Cold Water Flow Rate [m3/s]",
                                                         MaxVolColdWaterFlowUser);
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                if ((std::abs(MaxVolColdWaterFlowDes - MaxVolColdWaterFlowUser) / MaxVolColdWaterFlowUser) >
                                    state.dataSize->AutoVsHardSizingThreshold) {
                                    ShowMessage(state,
                                                format("SizeHVACSingleDuctInduction: Potential issue with equipment sizing for {} = \"{}\".",
                                                       state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                                       state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name));
                                    ShowContinueError(
                                        state, format("User-Specified Maximum Cold Water Flow Rate of {:.5R} [m3/s]", MaxVolColdWaterFlowUser));
                                    ShowContinueError(
                                        state,
                                        format("differs from Design Size Maximum Cold Water Flow Rate of {:.5R} [m3/s]", MaxVolColdWaterFlowDes));
                                    ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                    ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                                }
                            }
                        }
                    }
                } else {
                    state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxVolColdWaterFlow = 0.0;
                }
            }
        }

        if (state.dataSize->CurTermUnitSizingNum > 0) {
            auto &termUnitSizing = state.dataSize->TermUnitSizing(state.dataSize->CurTermUnitSizingNum);

            // note we save the induced air flow for use by the hw and cw coil sizing routines
            termUnitSizing.AirVolFlow = state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxTotAirVolFlow *
                                        state.dataHVACSingleDuctInduc->IndUnit(IUNum).InducRatio /
                                        (1.0 + state.dataHVACSingleDuctInduc->IndUnit(IUNum).InducRatio);
            // save the max hot and cold water flows for use in coil sizing
            termUnitSizing.MaxHWVolFlow = state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxVolHotWaterFlow;
            termUnitSizing.MaxCWVolFlow = state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxVolColdWaterFlow;
            // save the design load used for reporting
            termUnitSizing.DesCoolingLoad = state.dataHVACSingleDuctInduc->IndUnit(IUNum).DesCoolingLoad;
            termUnitSizing.DesHeatingLoad = state.dataHVACSingleDuctInduc->IndUnit(IUNum).DesHeatingLoad;
            // save the induction ratio for use in subsequent sizing calcs
            termUnitSizing.InducRat = state.dataHVACSingleDuctInduc->IndUnit(IUNum).InducRatio;
            if (Util::SameString(state.dataHVACSingleDuctInduc->IndUnit(IUNum).HCoilType, "Coil:Heating:Water")) {
                WaterCoils::SetCoilDesFlow(state,
                                           state.dataHVACSingleDuctInduc->IndUnit(IUNum).HCoilType,
                                           state.dataHVACSingleDuctInduc->IndUnit(IUNum).HCoil,
                                           termUnitSizing.AirVolFlow,
                                           ErrorsFound);
            }
            if (Util::SameString(state.dataHVACSingleDuctInduc->IndUnit(IUNum).CCoilType, "Coil:Cooling:Water:DetailedGeometry")) {
                WaterCoils::SetCoilDesFlow(state,
                                           state.dataHVACSingleDuctInduc->IndUnit(IUNum).CCoilType,
                                           state.dataHVACSingleDuctInduc->IndUnit(IUNum).CCoil,
                                           termUnitSizing.AirVolFlow,
                                           ErrorsFound);
            }
        }
    }

    void SimFourPipeIndUnit(EnergyPlusData &state,
                            int const IUNum,              // number of the current unit being simulated
                            int const ZoneNum,            // number of zone being served
                            int const ZoneNodeNum,        // zone node number
                            bool const FirstHVACIteration // TRUE if 1st HVAC simulation of system timestep
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   June 23 2004

        // PURPOSE OF THIS SUBROUTINE:
        // Simulate a 4 pipe induction unit; adjust its heating or cooling
        // coil outputs to match the zone load.

        // METHODOLOGY EMPLOYED:
        // (1) From the zone load and the primary air inlet conditions calculate the coil load
        //     in the secondary air stream
        // (2) If there is a cooling coil load, set the heating coil off and control the cooling
        //     coil to meet the coil load
        // (3) If there is a heating coil load, control the heating coil to meet the load and keep
        //     the cooling coil off.

        // SUBROUTINE PARAMETER DEFINITIONS:
        int constexpr SolveMaxIter(50);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 HWFlow;   // hot water flow [kg/s]
        Real64 CWFlow;   // cold water flow [kg/s]
        Real64 QPriOnly; // unit output with no zone coils active
        Real64 ErrTolerance;

        bool UnitOn = true;
        Real64 PowerMet = 0.0;
        Real64 InducRat = state.dataHVACSingleDuctInduc->IndUnit(IUNum).InducRatio;
        int PriNode = state.dataHVACSingleDuctInduc->IndUnit(IUNum).PriAirInNode;
        int SecNode = state.dataHVACSingleDuctInduc->IndUnit(IUNum).SecAirInNode;
        int OutletNode = state.dataHVACSingleDuctInduc->IndUnit(IUNum).OutAirNode;
        int HotControlNode = state.dataHVACSingleDuctInduc->IndUnit(IUNum).HWControlNode;
        int HWOutletNode = DataPlant::CompData::getPlantComponent(state, state.dataHVACSingleDuctInduc->IndUnit(IUNum).HWPlantLoc).NodeNumOut;
        int ColdControlNode = state.dataHVACSingleDuctInduc->IndUnit(IUNum).CWControlNode;
        int CWOutletNode = DataPlant::CompData::getPlantComponent(state, state.dataHVACSingleDuctInduc->IndUnit(IUNum).CWPlantLoc).NodeNumOut;
        Real64 PriAirMassFlow = state.dataLoopNodes->Node(PriNode).MassFlowRateMaxAvail;
        Real64 SecAirMassFlow = InducRat * PriAirMassFlow;
        Real64 QToHeatSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToHeatSP;
        Real64 QToCoolSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToCoolSP;
        // On the first HVAC iteration the system values are given to the controller, but after that
        // the demand limits are in place and there needs to be feedback to the Zone Equipment

        Real64 MaxHotWaterFlow = state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxHotWaterFlow;
        PlantUtilities::SetComponentFlowRate(
            state, MaxHotWaterFlow, HotControlNode, HWOutletNode, state.dataHVACSingleDuctInduc->IndUnit(IUNum).HWPlantLoc);

        Real64 MinHotWaterFlow = state.dataHVACSingleDuctInduc->IndUnit(IUNum).MinHotWaterFlow;
        PlantUtilities::SetComponentFlowRate(
            state, MinHotWaterFlow, HotControlNode, HWOutletNode, state.dataHVACSingleDuctInduc->IndUnit(IUNum).HWPlantLoc);

        Real64 MaxColdWaterFlow = state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxColdWaterFlow;
        PlantUtilities::SetComponentFlowRate(
            state, MaxColdWaterFlow, ColdControlNode, CWOutletNode, state.dataHVACSingleDuctInduc->IndUnit(IUNum).CWPlantLoc);

        Real64 MinColdWaterFlow = state.dataHVACSingleDuctInduc->IndUnit(IUNum).MinColdWaterFlow;
        PlantUtilities::SetComponentFlowRate(
            state, MinColdWaterFlow, ColdControlNode, CWOutletNode, state.dataHVACSingleDuctInduc->IndUnit(IUNum).CWPlantLoc);

        if (state.dataHVACSingleDuctInduc->IndUnit(IUNum).availSched->getCurrentVal() <= 0.0) {
            UnitOn = false;
        }
        if (PriAirMassFlow <= HVAC::SmallMassFlow) {
            UnitOn = false;
        }

        // Set the unit's air inlet nodes mass flow rates
        state.dataLoopNodes->Node(PriNode).MassFlowRate = PriAirMassFlow;
        state.dataLoopNodes->Node(SecNode).MassFlowRate = SecAirMassFlow;
        // initialize the water inlet nodes to minimum
        // fire the unit at min water flow
        CalcFourPipeIndUnit(state, IUNum, FirstHVACIteration, ZoneNodeNum, MinHotWaterFlow, MinColdWaterFlow, QPriOnly);
        // the load to be met by the secondary air stream coils is QZnReq-PowerMet

        if (UnitOn) {

            int SolFlag = 0;
            if (QToHeatSetPt - QPriOnly > HVAC::SmallLoad) {
                // heating coil
                // check that it can meet the load
                CalcFourPipeIndUnit(state, IUNum, FirstHVACIteration, ZoneNodeNum, MaxHotWaterFlow, MinColdWaterFlow, PowerMet);
                if (PowerMet > QToHeatSetPt + HVAC::SmallLoad) {
                    ErrTolerance = state.dataHVACSingleDuctInduc->IndUnit(IUNum).HotControlOffset;
                    auto f = // (AUTO_OK_LAMBDA)
                        [&state, IUNum, FirstHVACIteration, ZoneNodeNum, MinColdWaterFlow, QToHeatSetPt, QPriOnly, PowerMet](Real64 const HWFlow) {
                            Real64 UnitOutput;
                            CalcFourPipeIndUnit(state, IUNum, FirstHVACIteration, ZoneNodeNum, HWFlow, MinColdWaterFlow, UnitOutput);
                            return (QToHeatSetPt - UnitOutput) / (PowerMet - QPriOnly);
                        };
                    General::SolveRoot(state, ErrTolerance, SolveMaxIter, SolFlag, HWFlow, f, MinHotWaterFlow, MaxHotWaterFlow);
                    if (SolFlag == -1) {
                        if (state.dataHVACSingleDuctInduc->IndUnit(IUNum).HWCoilFailNum1 == 0) {
                            ShowWarningMessage(state,
                                               format("SimFourPipeIndUnit: Hot water coil control failed for {}=\"{}\"",
                                                      state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                                      state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name));
                            ShowContinueErrorTimeStamp(state, "");
                            ShowContinueError(state, format("  Iteration limit [{}] exceeded in calculating hot water mass flow rate", SolveMaxIter));
                        }
                        ShowRecurringWarningErrorAtEnd(
                            state,
                            format("SimFourPipeIndUnit: Hot water coil control failed (iteration limit [{}]) for {}=\"{}\"",
                                   SolveMaxIter,
                                   state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                   state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name),
                            state.dataHVACSingleDuctInduc->IndUnit(IUNum).HWCoilFailNum1);
                    } else if (SolFlag == -2) {
                        if (state.dataHVACSingleDuctInduc->IndUnit(IUNum).HWCoilFailNum2 == 0) {
                            ShowWarningMessage(state,
                                               format("SimFourPipeIndUnit: Hot water coil control failed (maximum flow limits) for {}=\"{}\"",
                                                      state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                                      state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name));
                            ShowContinueErrorTimeStamp(state, "");
                            ShowContinueError(state, "...Bad hot water maximum flow rate limits");
                            ShowContinueError(state, format("...Given minimum water flow rate={:.3R} kg/s", MinHotWaterFlow));
                            ShowContinueError(state, format("...Given maximum water flow rate={:.3R} kg/s", MaxHotWaterFlow));
                        }
                        ShowRecurringWarningErrorAtEnd(state,
                                                       "SimFourPipeIndUnit: Hot water coil control failed (flow limits) for " +
                                                           state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType + "=\"" +
                                                           state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name + "\"",
                                                       state.dataHVACSingleDuctInduc->IndUnit(IUNum).HWCoilFailNum2,
                                                       MaxHotWaterFlow,
                                                       MinHotWaterFlow,
                                                       _,
                                                       "[kg/s]",
                                                       "[kg/s]");
                    }
                }
            } else if (QToCoolSetPt - QPriOnly < -HVAC::SmallLoad) {
                // cooling coil
                // check that it can meet the load
                CalcFourPipeIndUnit(state, IUNum, FirstHVACIteration, ZoneNodeNum, MinHotWaterFlow, MaxColdWaterFlow, PowerMet);
                if (PowerMet < QToCoolSetPt - HVAC::SmallLoad) {
                    ErrTolerance = state.dataHVACSingleDuctInduc->IndUnit(IUNum).ColdControlOffset;
                    auto f = // (AUTO_OK_LAMBDA)
                        [&state, IUNum, FirstHVACIteration, ZoneNodeNum, MinHotWaterFlow, QToCoolSetPt, QPriOnly, PowerMet](Real64 const CWFlow) {
                            Real64 UnitOutput;
                            CalcFourPipeIndUnit(state, IUNum, FirstHVACIteration, ZoneNodeNum, MinHotWaterFlow, CWFlow, UnitOutput);
                            return (QToCoolSetPt - UnitOutput) / (PowerMet - QPriOnly);
                        };
                    General::SolveRoot(state, ErrTolerance, SolveMaxIter, SolFlag, CWFlow, f, MinColdWaterFlow, MaxColdWaterFlow);
                    if (SolFlag == -1) {
                        if (state.dataHVACSingleDuctInduc->IndUnit(IUNum).CWCoilFailNum1 == 0) {
                            ShowWarningMessage(state,
                                               format("SimFourPipeIndUnit: Cold water coil control failed for {}=\"{}\"",
                                                      state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                                      state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name));
                            ShowContinueErrorTimeStamp(state, "");
                            ShowContinueError(state,
                                              format("  Iteration limit [{}] exceeded in calculating cold water mass flow rate", SolveMaxIter));
                        }
                        ShowRecurringWarningErrorAtEnd(state,
                                                       format("SimFourPipeIndUnit: Cold water coil control failed (iteration limit [{}]) for {}=\"{}",
                                                              SolveMaxIter,
                                                              state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                                              state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name),
                                                       state.dataHVACSingleDuctInduc->IndUnit(IUNum).CWCoilFailNum1);
                    } else if (SolFlag == -2) {
                        if (state.dataHVACSingleDuctInduc->IndUnit(IUNum).CWCoilFailNum2 == 0) {
                            ShowWarningMessage(state,
                                               format("SimFourPipeIndUnit: Cold water coil control failed (maximum flow limits) for {}=\"{}\"",
                                                      state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType,
                                                      state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name));
                            ShowContinueErrorTimeStamp(state, "");
                            ShowContinueError(state, "...Bad cold water maximum flow rate limits");
                            ShowContinueError(state, format("...Given minimum water flow rate={:.3R} kg/s", MinColdWaterFlow));
                            ShowContinueError(state, format("...Given maximum water flow rate={:.3R} kg/s", MaxColdWaterFlow));
                        }
                        ShowRecurringWarningErrorAtEnd(state,
                                                       "SimFourPipeIndUnit: Cold water coil control failed (flow limits) for " +
                                                           state.dataHVACSingleDuctInduc->IndUnit(IUNum).UnitType + "=\"" +
                                                           state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name + "\"",
                                                       state.dataHVACSingleDuctInduc->IndUnit(IUNum).CWCoilFailNum2,
                                                       MaxColdWaterFlow,
                                                       MinColdWaterFlow,
                                                       _,
                                                       "[kg/s]",
                                                       "[kg/s]");
                    }
                }
            } else {
                CalcFourPipeIndUnit(state, IUNum, FirstHVACIteration, ZoneNodeNum, MinHotWaterFlow, MinColdWaterFlow, PowerMet);
            }

        } else {
            // unit off
            CalcFourPipeIndUnit(state, IUNum, FirstHVACIteration, ZoneNodeNum, MinHotWaterFlow, MinColdWaterFlow, PowerMet);
        }
        state.dataLoopNodes->Node(OutletNode).MassFlowRateMax = state.dataHVACSingleDuctInduc->IndUnit(IUNum).MaxTotAirMassFlow;

        // At this point we are done. There is no output to report or pass back up: the output provided is calculated
        // one level up in the calling routine SimZoneAirLoopEquipment. All the inlet and outlet flow rates and
        // conditions have been set by CalcFourPipeIndUnit either explicitly or as a result of the simple component calls.
    }

    void CalcFourPipeIndUnit(EnergyPlusData &state,
                             int const IUNum,               // Unit index
                             bool const FirstHVACIteration, // flag for 1st HVAV iteration in the time step
                             int const ZoneNode,            // zone node number
                             Real64 const HWFlow,           // hot water flow (kg/s)
                             Real64 const CWFlow,           // cold water flow (kg/s)
                             Real64 &LoadMet                // load met by unit (watts)
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   June 2004

        // PURPOSE OF THIS SUBROUTINE:
        // Simulate the components making up the 4 pipe induction unit.

        // METHODOLOGY EMPLOYED:
        // Simulates the unit components sequentially in the air flow direction.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int OutletNode;        // unit air outlet node
        int PriNode;           // unit primary air inlet node
        int HotControlNode;    // the hot water inlet node
        int ColdControlNode;   // the cold water inlet node
        Real64 PriAirMassFlow; // primary air mass flow rate [kg/s]
        Real64 SecAirMassFlow; // secondary air mass flow rate [kg/s]
        Real64 TotAirMassFlow; // total air mass flow rate [kg/s]
        Real64 InducRat;       // induction ratio
        Real64 mdotHW;         // local temporary hot water flow rate [kg/s]
        Real64 mdotCW;         // local temporary cold water flow rate [kg/s]
        int HWOutletNode;
        int CWOutletNode;

        PriNode = state.dataHVACSingleDuctInduc->IndUnit(IUNum).PriAirInNode;
        OutletNode = state.dataHVACSingleDuctInduc->IndUnit(IUNum).OutAirNode;
        PriAirMassFlow = state.dataLoopNodes->Node(PriNode).MassFlowRateMaxAvail;
        InducRat = state.dataHVACSingleDuctInduc->IndUnit(IUNum).InducRatio;
        SecAirMassFlow = InducRat * PriAirMassFlow;
        TotAirMassFlow = PriAirMassFlow + SecAirMassFlow;
        HotControlNode = state.dataHVACSingleDuctInduc->IndUnit(IUNum).HWControlNode;
        HWOutletNode = DataPlant::CompData::getPlantComponent(state, state.dataHVACSingleDuctInduc->IndUnit(IUNum).HWPlantLoc).NodeNumOut;

        ColdControlNode = state.dataHVACSingleDuctInduc->IndUnit(IUNum).CWControlNode;
        CWOutletNode = DataPlant::CompData::getPlantComponent(state, state.dataHVACSingleDuctInduc->IndUnit(IUNum).CWPlantLoc).NodeNumOut;

        mdotHW = HWFlow;
        PlantUtilities::SetComponentFlowRate(state, mdotHW, HotControlNode, HWOutletNode, state.dataHVACSingleDuctInduc->IndUnit(IUNum).HWPlantLoc);

        //  Node(HotControlNode)%MassFlowRate = HWFlow

        mdotCW = CWFlow;
        PlantUtilities::SetComponentFlowRate(state, mdotCW, ColdControlNode, CWOutletNode, state.dataHVACSingleDuctInduc->IndUnit(IUNum).CWPlantLoc);
        //  Node(ColdControlNode)%MassFlowRate = CWFlow

        WaterCoils::SimulateWaterCoilComponents(
            state, state.dataHVACSingleDuctInduc->IndUnit(IUNum).HCoil, FirstHVACIteration, state.dataHVACSingleDuctInduc->IndUnit(IUNum).HCoil_Num);
        WaterCoils::SimulateWaterCoilComponents(
            state, state.dataHVACSingleDuctInduc->IndUnit(IUNum).CCoil, FirstHVACIteration, state.dataHVACSingleDuctInduc->IndUnit(IUNum).CCoil_Num);
        MixerComponent::SimAirMixer(
            state, state.dataHVACSingleDuctInduc->IndUnit(IUNum).MixerName, state.dataHVACSingleDuctInduc->IndUnit(IUNum).Mixer_Num);
        LoadMet = TotAirMassFlow * Psychrometrics::PsyDeltaHSenFnTdb2W2Tdb1W1(state.dataLoopNodes->Node(OutletNode).Temp,
                                                                              state.dataLoopNodes->Node(OutletNode).HumRat,
                                                                              state.dataLoopNodes->Node(ZoneNode).Temp,
                                                                              state.dataLoopNodes->Node(ZoneNode).HumRat);
    }

    bool FourPipeInductionUnitHasMixer(EnergyPlusData &state, std::string_view CompName) // component (mixer) name
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   September 2011

        // PURPOSE OF THIS FUNCTION:
        // Given a mixer name, this routine determines if that mixer is found on PIUnits.

        if (state.dataHVACSingleDuctInduc->GetIUInputFlag) {
            GetIndUnits(state);
            state.dataHVACSingleDuctInduc->GetIUInputFlag = false;
        }

        if (state.dataHVACSingleDuctInduc->NumIndUnits > 0) {
            int ItemNum = Util::FindItemInList(CompName, state.dataHVACSingleDuctInduc->IndUnit, &IndUnitData::MixerName);
            if (ItemNum > 0) {
                return true;
            }
        }

        return false;
    }

    void IndUnitData::ReportIndUnit(EnergyPlusData &state)
    {
        // set zone OA volume flow rate
        this->CalcOutdoorAirVolumeFlowRate(state);
    }

    void IndUnitData::CalcOutdoorAirVolumeFlowRate(EnergyPlusData &state)
    {
        // calculates zone outdoor air volume flow rate using the supply air flow rate and OA fraction
        if (this->AirLoopNum > 0) {
            this->OutdoorAirFlowRate = (state.dataLoopNodes->Node(this->PriAirInNode).MassFlowRate / state.dataEnvrn->StdRhoAir) *
                                       state.dataAirLoop->AirLoopFlow(this->AirLoopNum).OAFrac;
        } else {
            this->OutdoorAirFlowRate = 0.0;
        }
    }

    void IndUnitData::reportTerminalUnit(EnergyPlusData &state)
    {
        // populate the predefined equipment summary report related to air terminals
        auto &orp = state.dataOutRptPredefined;
        auto &adu = state.dataDefineEquipment->AirDistUnit(this->ADUNum);
        if (!state.dataSize->TermUnitFinalZoneSizing.empty()) {
            auto &sizing = state.dataSize->TermUnitFinalZoneSizing(adu.TermUnitSizingNum);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermMinFlow, adu.Name, sizing.DesCoolVolFlowMin);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermMinOutdoorFlow, adu.Name, sizing.MinOA);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermSupCoolingSP, adu.Name, sizing.CoolDesTemp);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermSupHeatingSP, adu.Name, sizing.HeatDesTemp);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermHeatingCap, adu.Name, sizing.DesHeatLoad);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermCoolingCap, adu.Name, sizing.DesCoolLoad);
        }
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermTypeInp, adu.Name, this->UnitType);
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermPrimFlow, adu.Name, this->MaxPriAirMassFlow);
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermSecdFlow, adu.Name, this->MaxSecAirMassFlow);
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermMinFlowSch, adu.Name, "n/a");
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermMaxFlowReh, adu.Name, "n/a");
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermMinOAflowSch, adu.Name, "n/a");
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermHeatCoilType, adu.Name, this->HCoilType);
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermCoolCoilType, adu.Name, this->CCoilType);
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermFanType, adu.Name, "n/a");
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermFanName, adu.Name, "n/a");
    }

} // namespace HVACSingleDuctInduc

} // namespace EnergyPlus
