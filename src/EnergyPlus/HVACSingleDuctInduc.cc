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

// C++ Headers
#include <cmath>
#include <format>

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
                ShowFatalError(state, std::format("SimIndUnit: Induction Unit not found={}", CompName));
            }
            CompIndex = IUNum;
        } else {
            IUNum = CompIndex;
            if (IUNum > state.dataHVACSingleDuctInduc->NumIndUnits || IUNum < 1) {
                ShowFatalError(state,
                               std::format("SimIndUnit: Invalid CompIndex passed={}, Number of Induction Units={}, System name={}",
                                           CompIndex,
                                           state.dataHVACSingleDuctInduc->NumIndUnits,
                                           CompName));
            }
            if (state.dataHVACSingleDuctInduc->CheckEquipName(IUNum)) {
                if (CompName != state.dataHVACSingleDuctInduc->IndUnit(IUNum).Name) {
                    ShowFatalError(
                        state,
                        std::format("SimIndUnit: Invalid CompIndex passed={}, Induction Unit name={}, stored Induction Unit for that index={}",
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
            ShowSevereError(state, std::format("Illegal Induction Unit Type used={}", indUnit.UnitType));
            ShowContinueError(state, std::format("Occurs in Induction Unit={}", indUnit.Name));
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

            auto &indUnit = state.dataHVACSingleDuctInduc->IndUnit(IUNum);
            indUnit.Name = Alphas(1);
            indUnit.UnitType = CurrentModuleObject;
            indUnit.UnitType_Num = SingleDuct_CV::FourPipeInduc;

            if (lAlphaBlanks(2)) {
                indUnit.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((indUnit.availSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ErrorsFound = true;
            }
            indUnit.MaxTotAirVolFlow = Numbers(1);
            indUnit.InducRatio = Numbers(2);
            if (lNumericBlanks(2)) {
                indUnit.InducRatio = 2.5;
            }

            indUnit.PriAirInNode = GetOnlySingleNode(state,
                                                     Alphas(3),
                                                     ErrorsFound,
                                                     Node::ConnectionObjectType::AirTerminalSingleDuctConstantVolumeFourPipeInduction,
                                                     Alphas(1),
                                                     Node::FluidType::Air,
                                                     Node::ConnectionType::Inlet,
                                                     Node::CompFluidStream::Primary,
                                                     Node::ObjectIsParent,
                                                     cAlphaFields(3));
            indUnit.SecAirInNode = GetOnlySingleNode(state,
                                                     Alphas(4),
                                                     ErrorsFound,
                                                     Node::ConnectionObjectType::AirTerminalSingleDuctConstantVolumeFourPipeInduction,
                                                     Alphas(1),
                                                     Node::FluidType::Air,
                                                     Node::ConnectionType::Inlet,
                                                     Node::CompFluidStream::Primary,
                                                     Node::ObjectIsParent,
                                                     cAlphaFields(4));
            indUnit.OutAirNode = GetOnlySingleNode(state,
                                                   Alphas(5),
                                                   ErrorsFound,
                                                   Node::ConnectionObjectType::AirTerminalSingleDuctConstantVolumeFourPipeInduction,
                                                   Alphas(1),
                                                   Node::FluidType::Air,
                                                   Node::ConnectionType::Outlet,
                                                   Node::CompFluidStream::Primary,
                                                   Node::ObjectIsParent,
                                                   cAlphaFields(5));

            indUnit.HCoilType = Alphas(6); // type (key) of heating coil
            if (Util::SameString(indUnit.HCoilType, "Coil:Heating:Water")) {
                indUnit.HeatingCoilType = DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
            }

            indUnit.HCoil = Alphas(7); // name of heating coil object
            bool IsNotOK = false;
            indUnit.HWControlNode = WaterCoils::GetCoilWaterInletNode(state, indUnit.HCoilType, indUnit.HCoil, IsNotOK);
            if (IsNotOK) {
                ShowContinueError(state, std::format("In {} = {}", CurrentModuleObject, indUnit.Name));
                ShowContinueError(state, "..Only Coil:Heating:Water is allowed.");
                ErrorsFound = true;
            }
            indUnit.MaxVolHotWaterFlow = Numbers(3);
            indUnit.MinVolHotWaterFlow = Numbers(4);
            indUnit.HotControlOffset = Numbers(5);

            indUnit.CCoilType = Alphas(8); // type (key) of cooling coil

            if (Util::SameString(indUnit.CCoilType, "Coil:Cooling:Water")) {
                indUnit.CoolingCoilType = DataPlant::PlantEquipmentType::CoilWaterCooling;
            } else if (Util::SameString(indUnit.CCoilType, "Coil:Cooling:Water:DetailedGeometry")) {
                indUnit.CoolingCoilType = DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling;
            }

            indUnit.CCoil = Alphas(9); // name of cooling coil object
            IsNotOK = false;
            indUnit.CWControlNode = WaterCoils::GetCoilWaterInletNode(state, indUnit.CCoilType, indUnit.CCoil, IsNotOK);
            if (IsNotOK) {
                ShowContinueError(state, std::format("In {} = {}", CurrentModuleObject, indUnit.Name));
                ShowContinueError(state, "..Only Coil:Cooling:Water or Coil:Cooling:Water:DetailedGeometry is allowed.");
                ErrorsFound = true;
            }
            indUnit.MaxVolColdWaterFlow = Numbers(6);
            indUnit.MinVolColdWaterFlow = Numbers(7);
            indUnit.ColdControlOffset = Numbers(8);

            // Get the Zone Mixer name and check that it is OK
            bool errFlag = false;
            indUnit.MixerName = Alphas(10);
            MixerComponent::GetZoneMixerIndex(state, indUnit.MixerName, indUnit.Mixer_Num, errFlag, CurrentModuleObject);
            if (errFlag) {
                ShowContinueError(state, std::format("...specified in {} = {}", CurrentModuleObject, indUnit.Name));
                ErrorsFound = true;
            }

            // Add heating coil to component sets array
            Node::SetUpCompSets(state, indUnit.UnitType, indUnit.Name, indUnit.HCoilType, indUnit.HCoil, Alphas(4), "UNDEFINED");
            // Add cooling coil to component sets array
            Node::SetUpCompSets(state, indUnit.UnitType, indUnit.Name, indUnit.CCoilType, indUnit.CCoil, "UNDEFINED", "UNDEFINED");

            // Register component set data
            Node::TestCompSet(state,
                              indUnit.UnitType,
                              indUnit.Name,
                              state.dataLoopNodes->NodeID(indUnit.PriAirInNode),
                              state.dataLoopNodes->NodeID(indUnit.OutAirNode),
                              "Air Nodes");

            for (int ADUNum = 1; ADUNum <= (int)state.dataDefineEquipment->AirDistUnit.size(); ++ADUNum) {
                if (indUnit.OutAirNode == state.dataDefineEquipment->AirDistUnit(ADUNum).OutletNodeNum) {
                    indUnit.ADUNum = ADUNum;
                }
            }
            // one assumes if there isn't one assigned, it's an error?
            if (indUnit.ADUNum == 0) {
                ShowSevereError(state,
                                std::format("{}No matching Air Distribution Unit, for Unit = [{},{}].", RoutineName, indUnit.UnitType, indUnit.Name));
                ShowContinueError(state, std::format("...should have outlet node={}", state.dataLoopNodes->NodeID(indUnit.OutAirNode)));
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
                        if (indUnit.OutAirNode == zoneEquipConfig.InletNode(SupAirIn)) {
                            if (zoneEquipConfig.AirDistUnitCool(SupAirIn).OutNode > 0) {
                                ShowSevereError(state, "Error in connecting a terminal unit to a zone");
                                ShowContinueError(
                                    state, std::format("{} already connects to another zone", state.dataLoopNodes->NodeID(indUnit.OutAirNode)));
                                ShowContinueError(state, std::format("Occurs for terminal unit {} = {}", indUnit.UnitType, indUnit.Name));
                                ShowContinueError(state, "Check terminal unit node names for errors");
                                ErrorsFound = true;
                            } else {
                                zoneEquipConfig.AirDistUnitCool(SupAirIn).InNode = indUnit.PriAirInNode;
                                zoneEquipConfig.AirDistUnitCool(SupAirIn).OutNode = indUnit.OutAirNode;
                                state.dataDefineEquipment->AirDistUnit(indUnit.ADUNum).TermUnitSizingNum =
                                    zoneEquipConfig.AirDistUnitCool(SupAirIn).TermUnitSizingIndex;
                                state.dataDefineEquipment->AirDistUnit(indUnit.ADUNum).ZoneEqNum = CtrlZone;
                                indUnit.CtrlZoneNum = CtrlZone;
                            }
                            indUnit.CtrlZoneInNodeIndex = SupAirIn;
                            AirNodeFound = true;
                            break;
                        }
                    }
                }
                if (!AirNodeFound) {
                    ShowSevereError(state, std::format("The outlet air node from the {} = {}", CurrentModuleObject, indUnit.Name));
                    ShowContinueError(state, std::format("did not have a matching Zone Equipment Inlet Node, Node ={}", Alphas(3)));
                    ErrorsFound = true;
                }
            }
            // report variable for all single duct air terminals
            SetupOutputVariable(state,
                                "Zone Air Terminal Outdoor Air Volume Flow Rate",
                                Constant::Units::m3_s,
                                indUnit.OutdoorAirFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                indUnit.Name);
        }

        Alphas.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        Numbers.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();
        if (ErrorsFound) {
            ShowFatalError(state, std::format("{}Errors found in getting input. Preceding conditions cause termination.", RoutineName));
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

        auto &indUnit = state.dataHVACSingleDuctInduc->IndUnit(IUNum);

        if (state.dataHVACSingleDuctInduc->MyPlantScanFlag(IUNum) && allocated(state.dataPlnt->PlantLoop)) {
            bool errFlag = false;
            if (indUnit.HeatingCoilType == DataPlant::PlantEquipmentType::CoilWaterSimpleHeating) {
                errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(state, indUnit.HCoil, indUnit.HeatingCoilType, indUnit.HWPlantLoc, errFlag, _, _, _, _, _);
            }
            if (errFlag) {
                ShowContinueError(state, std::format("Reference Unit=\"{}\", type={}", indUnit.Name, indUnit.UnitType));
            }
            if (indUnit.CoolingCoilType == DataPlant::PlantEquipmentType::CoilWaterCooling ||
                indUnit.CoolingCoilType == DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling) {
                errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(state, indUnit.CCoil, indUnit.CoolingCoilType, indUnit.CWPlantLoc, errFlag, _, _, _, _, _);
            }
            if (errFlag) {
                ShowContinueError(state, std::format("Reference Unit=\"{}\", type={}", indUnit.Name, indUnit.UnitType));
                ShowFatalError(state, "InitIndUnit: Program terminated for previous conditions.");
            }
            state.dataHVACSingleDuctInduc->MyPlantScanFlag(IUNum) = false;
        } else if (state.dataHVACSingleDuctInduc->MyPlantScanFlag(IUNum) && !state.dataGlobal->AnyPlantInModel) {
            state.dataHVACSingleDuctInduc->MyPlantScanFlag(IUNum) = false;
        }

        if (state.dataHVACSingleDuctInduc->MyAirDistInitFlag(IUNum)) {
            // save the induction ratio in the term unit sizing array for use in the system sizing calculation
            if (state.dataSize->CurTermUnitSizingNum > 0) {
                state.dataSize->TermUnitSizing(state.dataSize->CurTermUnitSizingNum).InducRat = indUnit.InducRatio;
            }
            if (indUnit.AirLoopNum == 0) {
                if ((indUnit.CtrlZoneNum > 0) && (indUnit.CtrlZoneInNodeIndex > 0)) {
                    indUnit.AirLoopNum = state.dataZoneEquip->ZoneEquipConfig(indUnit.CtrlZoneNum).InletNodeAirLoopNum(indUnit.CtrlZoneInNodeIndex);
                    state.dataDefineEquipment->AirDistUnit(indUnit.ADUNum).AirLoopNum = indUnit.AirLoopNum;
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
                                std::format("InitIndUnit: ADU=[Air Distribution Unit,{}] is not on any ZoneHVAC:EquipmentList.",
                                            state.dataDefineEquipment->AirDistUnit(state.dataHVACSingleDuctInduc->IndUnit(Loop).ADUNum).Name));
                ShowContinueError(state,
                                  std::format("...Unit=[{},{}] will not be simulated.",
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
            PriNode = indUnit.PriAirInNode;
            SecNode = indUnit.SecAirInNode;
            int OutletNode = indUnit.OutAirNode;
            IndRat = indUnit.InducRatio;
            // set the mass flow rates from the input volume flow rates
            if (Util::SameString(indUnit.UnitType, "AirTerminal:SingleDuct:ConstantVolume:FourPipeInduction")) {
                indUnit.MaxTotAirMassFlow = RhoAir * indUnit.MaxTotAirVolFlow;
                indUnit.MaxPriAirMassFlow = indUnit.MaxTotAirMassFlow / (1.0 + IndRat);
                indUnit.MaxSecAirMassFlow = IndRat * indUnit.MaxTotAirMassFlow / (1.0 + IndRat);
                state.dataLoopNodes->Node(PriNode).MassFlowRateMax = indUnit.MaxPriAirMassFlow;
                state.dataLoopNodes->Node(PriNode).MassFlowRateMin = indUnit.MaxPriAirMassFlow;
                state.dataLoopNodes->Node(SecNode).MassFlowRateMax = indUnit.MaxSecAirMassFlow;
                state.dataLoopNodes->Node(SecNode).MassFlowRateMin = indUnit.MaxSecAirMassFlow;
                state.dataLoopNodes->Node(OutletNode).MassFlowRateMax = indUnit.MaxTotAirMassFlow;
            }

            int HotConNode = indUnit.HWControlNode;
            if (HotConNode > 0 && !state.dataHVACSingleDuctInduc->MyPlantScanFlag(IUNum)) {

                rho = indUnit.HWPlantLoc.loop->glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                indUnit.MaxHotWaterFlow = rho * indUnit.MaxVolHotWaterFlow;
                indUnit.MinHotWaterFlow = rho * indUnit.MinVolHotWaterFlow;
                // get component outlet node from plant structure
                int HWOutletNode = DataPlant::CompData::getPlantComponent(state, indUnit.HWPlantLoc).NodeNumOut;
                PlantUtilities::InitComponentNodes(state, indUnit.MinHotWaterFlow, indUnit.MaxHotWaterFlow, HotConNode, HWOutletNode);
            }

            int ColdConNode = indUnit.CWControlNode;
            if (ColdConNode > 0) {
                rho = indUnit.CWPlantLoc.loop->glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);
                indUnit.MaxColdWaterFlow = rho * indUnit.MaxVolColdWaterFlow;
                indUnit.MinColdWaterFlow = rho * indUnit.MinVolColdWaterFlow;

                int CWOutletNode = DataPlant::CompData::getPlantComponent(state, indUnit.CWPlantLoc).NodeNumOut;
                PlantUtilities::InitComponentNodes(state, indUnit.MinColdWaterFlow, indUnit.MaxColdWaterFlow, ColdConNode, CWOutletNode);
            }

            state.dataHVACSingleDuctInduc->MyEnvrnFlag(IUNum) = false;
        } // end one time inits

        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataHVACSingleDuctInduc->MyEnvrnFlag(IUNum) = true;
        }

        PriNode = indUnit.PriAirInNode;
        SecNode = indUnit.SecAirInNode;

        // Do the start of HVAC time step initializations
        if (FirstHVACIteration) {
            // check for upstream zero flow. If nonzero and schedule ON, set primary flow to max
            if (indUnit.availSched->getCurrentVal() > 0.0 && state.dataLoopNodes->Node(PriNode).MassFlowRate > 0.0) {
                if (Util::SameString(indUnit.UnitType, "AirTerminal:SingleDuct:ConstantVolume:FourPipeInduction")) {
                    state.dataLoopNodes->Node(PriNode).MassFlowRate = indUnit.MaxPriAirMassFlow;
                    state.dataLoopNodes->Node(SecNode).MassFlowRate = indUnit.MaxSecAirMassFlow;
                }
            } else {
                state.dataLoopNodes->Node(PriNode).MassFlowRate = 0.0;
                state.dataLoopNodes->Node(SecNode).MassFlowRate = 0.0;
            }
            // reset the max and min avail flows
            if (indUnit.availSched->getCurrentVal() > 0.0 && state.dataLoopNodes->Node(PriNode).MassFlowRateMaxAvail > 0.0) {
                if (Util::SameString(indUnit.UnitType, "AirTerminal:SingleDuct:ConstantVolume:FourPipeInduction")) {
                    state.dataLoopNodes->Node(PriNode).MassFlowRateMaxAvail = indUnit.MaxPriAirMassFlow;
                    state.dataLoopNodes->Node(PriNode).MassFlowRateMinAvail = indUnit.MaxPriAirMassFlow;
                    state.dataLoopNodes->Node(SecNode).MassFlowRateMaxAvail = indUnit.MaxSecAirMassFlow;
                    state.dataLoopNodes->Node(SecNode).MassFlowRateMinAvail = indUnit.MaxSecAirMassFlow;
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

        auto &indUnit = state.dataHVACSingleDuctInduc->IndUnit(IUNum);

        if (indUnit.MaxTotAirVolFlow == DataSizing::AutoSize) {
            IsAutoSize = true;
        }

        if (state.dataSize->CurZoneEqNum > 0) {
            if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // simulation continue
                if (indUnit.MaxTotAirVolFlow > 0.0) {
                    BaseSizer::reportSizerOutput(
                        state, indUnit.UnitType, indUnit.Name, "User-Specified Maximum Total Air Flow Rate [m3/s]", indUnit.MaxTotAirVolFlow);
                }
            } else {
                CheckZoneSizing(state, indUnit.UnitType, indUnit.Name);
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
                    indUnit.MaxTotAirVolFlow = MaxTotAirVolFlowDes;
                    BaseSizer::reportSizerOutput(
                        state, indUnit.UnitType, indUnit.Name, "Design Size Maximum Total Air Flow Rate [m3/s]", MaxTotAirVolFlowDes);
                } else {
                    if (indUnit.MaxTotAirVolFlow > 0.0 && MaxTotAirVolFlowDes > 0.0) {
                        MaxTotAirVolFlowUser = indUnit.MaxTotAirVolFlow;
                        BaseSizer::reportSizerOutput(state,
                                                     indUnit.UnitType,
                                                     indUnit.Name,
                                                     "Design Size Maximum Total Air Flow Rate [m3/s]",
                                                     MaxTotAirVolFlowDes,
                                                     "User-Specified Maximum Total Air Flow Rate [m3/s]",
                                                     MaxTotAirVolFlowUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(MaxTotAirVolFlowDes - MaxTotAirVolFlowUser) / MaxTotAirVolFlowUser) >
                                state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state,
                                            std::format("SizeHVACSingleDuctInduction: Potential issue with equipment sizing for {} = \"{}\".",
                                                        indUnit.UnitType,
                                                        indUnit.Name));
                                ShowContinueError(state,
                                                  std::format("User-Specified Maximum Total Air Flow Rate of {:#G} [m3/s]", MaxTotAirVolFlowUser));
                                ShowContinueError(
                                    state, std::format("differs from Design Size Maximum Total Air Flow Rate of {:#G} [m3/s]", MaxTotAirVolFlowDes));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                }
            }
        }

        IsAutoSize = false;
        if (indUnit.MaxVolHotWaterFlow == DataSizing::AutoSize) {
            IsAutoSize = true;
        }
        if ((state.dataSize->CurZoneEqNum > 0) && (state.dataSize->CurTermUnitSizingNum > 0)) {
            if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // simulation continue
                if (indUnit.MaxVolHotWaterFlow > 0.0) {
                    BaseSizer::reportSizerOutput(
                        state, indUnit.UnitType, indUnit.Name, "User-Specified Maximum Hot Water Flow Rate [m3/s]", indUnit.MaxVolHotWaterFlow);
                }
            } else {
                CheckZoneSizing(state, indUnit.UnitType, indUnit.Name);

                if (Util::SameString(indUnit.HCoilType, "Coil:Heating:Water")) {

                    int CoilWaterInletNode = WaterCoils::GetCoilWaterInletNode(state, "Coil:Heating:Water", indUnit.HCoil, ErrorsFound);
                    int CoilWaterOutletNode = WaterCoils::GetCoilWaterOutletNode(state, "Coil:Heating:Water", indUnit.HCoil, ErrorsFound);
                    if (IsAutoSize) {
                        int PltSizHeatNum = PlantUtilities::MyPlantSizingIndex(
                            state, "Coil:Heating:Water", indUnit.HCoil, CoilWaterInletNode, CoilWaterOutletNode, ErrorsFound);
                        if (PltSizHeatNum > 0) {

                            if (state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesHeatMassFlow >=
                                HVAC::SmallAirVolFlow) {
                                DesPriVolFlow = indUnit.MaxTotAirVolFlow / (1.0 + indUnit.InducRatio);
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
                                indUnit.DesHeatingLoad = DesCoilLoad;
                                Cp = indUnit.HWPlantLoc.loop->glycol->getSpecificHeat(state, Constant::HWInitConvTemp, RoutineName);

                                rho = indUnit.HWPlantLoc.loop->glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);

                                MaxVolHotWaterFlowDes = DesCoilLoad / (state.dataSize->PlantSizData(PltSizHeatNum).DeltaT * Cp * rho);
                                MaxVolHotWaterFlowDes = max(MaxVolHotWaterFlowDes, 0.0);
                            } else {
                                MaxVolHotWaterFlowDes = 0.0;
                            }
                        } else {
                            ShowSevereError(state, "Autosizing of water flow requires a heating loop Sizing:Plant object");
                            ShowContinueError(state, std::format("Occurs in {} Object={}", indUnit.UnitType, indUnit.Name));
                            ErrorsFound = true;
                        }
                        indUnit.MaxVolHotWaterFlow = MaxVolHotWaterFlowDes;
                        BaseSizer::reportSizerOutput(
                            state, indUnit.UnitType, indUnit.Name, "Design Size Maximum Hot Water Flow Rate [m3/s]", MaxVolHotWaterFlowDes);
                        BaseSizer::reportSizerOutput(
                            state,
                            indUnit.UnitType,
                            indUnit.Name,
                            "Design Size Inlet Air Temperature [C]",
                            state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesHeatCoilInTempTU);
                        BaseSizer::reportSizerOutput(
                            state,
                            indUnit.UnitType,
                            indUnit.Name,
                            "Design Size Inlet Air Humidity Ratio [kgWater/kgDryAir]",
                            state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesHeatCoilInHumRatTU);
                    } else {
                        if (indUnit.MaxVolHotWaterFlow > 0.0 && MaxVolHotWaterFlowDes > 0.0) {
                            MaxVolHotWaterFlowUser = indUnit.MaxVolHotWaterFlow;
                            BaseSizer::reportSizerOutput(state,
                                                         indUnit.UnitType,
                                                         indUnit.Name,
                                                         "Design Size Maximum Hot Water Flow Rate [m3/s]",
                                                         MaxVolHotWaterFlowDes,
                                                         "User-Specified Maximum Hot Water Flow Rate [m3/s]",
                                                         MaxVolHotWaterFlowUser);
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                if ((std::abs(MaxVolHotWaterFlowDes - MaxVolHotWaterFlowUser) / MaxVolHotWaterFlowUser) >
                                    state.dataSize->AutoVsHardSizingThreshold) {
                                    ShowMessage(state,
                                                std::format("SizeHVACSingleDuctInduction: Potential issue with equipment sizing for {} = \"{}\".",
                                                            indUnit.UnitType,
                                                            indUnit.Name));
                                    ShowContinueError(
                                        state, std::format("User-Specified Maximum Hot Water Flow Rate of {:#G} [m3/s]", MaxVolHotWaterFlowUser));
                                    ShowContinueError(
                                        state,
                                        std::format("differs from Design Size Maximum Hot Water Flow Rate of {:#G} [m3/s]", MaxVolHotWaterFlowDes));
                                    ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                    ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                                }
                            }
                        }
                    }
                } else {
                    indUnit.MaxVolHotWaterFlow = 0.0;
                }
            }
        }

        IsAutoSize = false;
        if (indUnit.MaxVolColdWaterFlow == DataSizing::AutoSize) {
            IsAutoSize = true;
        }
        if ((state.dataSize->CurZoneEqNum > 0) && (state.dataSize->CurTermUnitSizingNum > 0)) {
            if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // simulation continue
                if (indUnit.MaxVolColdWaterFlow > 0.0) {
                    BaseSizer::reportSizerOutput(
                        state, indUnit.UnitType, indUnit.Name, "User-Specified Maximum Cold Water Flow Rate [m3/s]", indUnit.MaxVolColdWaterFlow);
                }
            } else {
                CheckZoneSizing(state, indUnit.UnitType, indUnit.Name);

                if (Util::SameString(indUnit.CCoilType, "Coil:Cooling:Water") ||
                    Util::SameString(indUnit.CCoilType, "Coil:Cooling:Water:DetailedGeometry")) {

                    int CoilWaterInletNode = WaterCoils::GetCoilWaterInletNode(state, indUnit.CCoilType, indUnit.CCoil, ErrorsFound);
                    int CoilWaterOutletNode = WaterCoils::GetCoilWaterOutletNode(state, indUnit.CCoilType, indUnit.CCoil, ErrorsFound);
                    if (IsAutoSize) {
                        int PltSizCoolNum = PlantUtilities::MyPlantSizingIndex(
                            state, indUnit.CCoilType, indUnit.CCoil, CoilWaterInletNode, CoilWaterOutletNode, ErrorsFound);
                        if (PltSizCoolNum > 0) {

                            if (state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesCoolMassFlow >=
                                HVAC::SmallAirVolFlow) {
                                DesPriVolFlow = indUnit.MaxTotAirVolFlow / (1.0 + indUnit.InducRatio);
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
                                indUnit.DesCoolingLoad = DesCoilLoad;
                                Cp = indUnit.CWPlantLoc.loop->glycol->getSpecificHeat(state, 5.0, RoutineName);

                                rho = indUnit.CWPlantLoc.loop->glycol->getDensity(state, 5.0, RoutineName);

                                MaxVolColdWaterFlowDes = DesCoilLoad / (state.dataSize->PlantSizData(PltSizCoolNum).DeltaT * Cp * rho);
                                MaxVolColdWaterFlowDes = max(MaxVolColdWaterFlowDes, 0.0);
                            } else {
                                MaxVolColdWaterFlowDes = 0.0;
                            }
                        } else {
                            ShowSevereError(state, "Autosizing of water flow requires a cooling loop Sizing:Plant object");
                            ShowContinueError(state, std::format("Occurs in {} Object={}", indUnit.UnitType, indUnit.Name));
                            ErrorsFound = true;
                        }
                        indUnit.MaxVolColdWaterFlow = MaxVolColdWaterFlowDes;
                        BaseSizer::reportSizerOutput(
                            state, indUnit.UnitType, indUnit.Name, "Design Size Maximum Cold Water Flow Rate [m3/s]", MaxVolColdWaterFlowDes);
                    } else {
                        if (indUnit.MaxVolColdWaterFlow > 0.0 && MaxVolColdWaterFlowDes > 0.0) {
                            MaxVolColdWaterFlowUser = indUnit.MaxVolColdWaterFlow;
                            BaseSizer::reportSizerOutput(state,
                                                         indUnit.UnitType,
                                                         indUnit.Name,
                                                         "Design Size Maximum Cold Water Flow Rate [m3/s]",
                                                         MaxVolColdWaterFlowDes,
                                                         "User-Specified Maximum Cold Water Flow Rate [m3/s]",
                                                         MaxVolColdWaterFlowUser);
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                if ((std::abs(MaxVolColdWaterFlowDes - MaxVolColdWaterFlowUser) / MaxVolColdWaterFlowUser) >
                                    state.dataSize->AutoVsHardSizingThreshold) {
                                    ShowMessage(state,
                                                std::format("SizeHVACSingleDuctInduction: Potential issue with equipment sizing for {} = \"{}\".",
                                                            indUnit.UnitType,
                                                            indUnit.Name));
                                    ShowContinueError(
                                        state, std::format("User-Specified Maximum Cold Water Flow Rate of {:#G} [m3/s]", MaxVolColdWaterFlowUser));
                                    ShowContinueError(
                                        state,
                                        std::format("differs from Design Size Maximum Cold Water Flow Rate of {:#G} [m3/s]", MaxVolColdWaterFlowDes));
                                    ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                    ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                                }
                            }
                        }
                    }
                } else {
                    indUnit.MaxVolColdWaterFlow = 0.0;
                }
            }
        }

        if (state.dataSize->CurTermUnitSizingNum > 0) {
            auto &termUnitSizing = state.dataSize->TermUnitSizing(state.dataSize->CurTermUnitSizingNum);

            // note we save the induced air flow for use by the hw and cw coil sizing routines
            termUnitSizing.AirVolFlow = indUnit.MaxTotAirVolFlow * indUnit.InducRatio / (1.0 + indUnit.InducRatio);
            // save the max hot and cold water flows for use in coil sizing
            termUnitSizing.MaxHWVolFlow = indUnit.MaxVolHotWaterFlow;
            termUnitSizing.MaxCWVolFlow = indUnit.MaxVolColdWaterFlow;
            // save the design load used for reporting
            termUnitSizing.DesCoolingLoad = indUnit.DesCoolingLoad;
            termUnitSizing.DesHeatingLoad = indUnit.DesHeatingLoad;
            // save the induction ratio for use in subsequent sizing calcs
            termUnitSizing.InducRat = indUnit.InducRatio;
            if (Util::SameString(indUnit.HCoilType, "Coil:Heating:Water")) {
                WaterCoils::SetCoilDesFlow(state, indUnit.HCoilType, indUnit.HCoil, termUnitSizing.AirVolFlow, ErrorsFound);
            }
            if (Util::SameString(indUnit.CCoilType, "Coil:Cooling:Water:DetailedGeometry")) {
                WaterCoils::SetCoilDesFlow(state, indUnit.CCoilType, indUnit.CCoil, termUnitSizing.AirVolFlow, ErrorsFound);
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

        auto &indUnit = state.dataHVACSingleDuctInduc->IndUnit(IUNum);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 HWFlow;   // hot water flow [kg/s]
        Real64 CWFlow;   // cold water flow [kg/s]
        Real64 QPriOnly; // unit output with no zone coils active
        Real64 ErrTolerance;

        bool UnitOn = true;
        Real64 PowerMet = 0.0;
        Real64 InducRat = indUnit.InducRatio;
        int PriNode = indUnit.PriAirInNode;
        int SecNode = indUnit.SecAirInNode;
        int OutletNode = indUnit.OutAirNode;
        int HotControlNode = indUnit.HWControlNode;
        int HWOutletNode = DataPlant::CompData::getPlantComponent(state, indUnit.HWPlantLoc).NodeNumOut;
        int ColdControlNode = indUnit.CWControlNode;
        int CWOutletNode = DataPlant::CompData::getPlantComponent(state, indUnit.CWPlantLoc).NodeNumOut;
        Real64 PriAirMassFlow = state.dataLoopNodes->Node(PriNode).MassFlowRateMaxAvail;
        Real64 SecAirMassFlow = InducRat * PriAirMassFlow;
        Real64 QToHeatSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToHeatSP;
        Real64 QToCoolSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToCoolSP;
        // On the first HVAC iteration the system values are given to the controller, but after that
        // the demand limits are in place and there needs to be feedback to the Zone Equipment

        Real64 MaxHotWaterFlow = indUnit.MaxHotWaterFlow;
        PlantUtilities::SetComponentFlowRate(state, MaxHotWaterFlow, HotControlNode, HWOutletNode, indUnit.HWPlantLoc);

        Real64 MinHotWaterFlow = indUnit.MinHotWaterFlow;
        PlantUtilities::SetComponentFlowRate(state, MinHotWaterFlow, HotControlNode, HWOutletNode, indUnit.HWPlantLoc);

        Real64 MaxColdWaterFlow = indUnit.MaxColdWaterFlow;
        PlantUtilities::SetComponentFlowRate(state, MaxColdWaterFlow, ColdControlNode, CWOutletNode, indUnit.CWPlantLoc);

        Real64 MinColdWaterFlow = indUnit.MinColdWaterFlow;
        PlantUtilities::SetComponentFlowRate(state, MinColdWaterFlow, ColdControlNode, CWOutletNode, indUnit.CWPlantLoc);

        if (indUnit.availSched->getCurrentVal() <= 0.0) {
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
                    ErrTolerance = indUnit.HotControlOffset;
                    auto f = // (AUTO_OK_LAMBDA)
                        [&state, IUNum, FirstHVACIteration, ZoneNodeNum, MinColdWaterFlow, QToHeatSetPt, QPriOnly, PowerMet](Real64 const HWFlow) {
                            Real64 UnitOutput;
                            CalcFourPipeIndUnit(state, IUNum, FirstHVACIteration, ZoneNodeNum, HWFlow, MinColdWaterFlow, UnitOutput);
                            return (QToHeatSetPt - UnitOutput) / (PowerMet - QPriOnly);
                        };
                    General::SolveRoot(state, ErrTolerance, SolveMaxIter, SolFlag, HWFlow, f, MinHotWaterFlow, MaxHotWaterFlow);
                    if (SolFlag == -1) {
                        if (indUnit.HWCoilFailNum1 == 0) {
                            ShowWarningMessage(
                                state,
                                std::format("SimFourPipeIndUnit: Hot water coil control failed for {}=\"{}\"", indUnit.UnitType, indUnit.Name));
                            ShowContinueErrorTimeStamp(state, "");
                            ShowContinueError(state,
                                              std::format("  Iteration limit [{}] exceeded in calculating hot water mass flow rate", SolveMaxIter));
                        }
                        ShowRecurringWarningErrorAtEnd(
                            state,
                            std::format("SimFourPipeIndUnit: Hot water coil control failed (iteration limit [{}]) for {}=\"{}\"",
                                        SolveMaxIter,
                                        indUnit.UnitType,
                                        indUnit.Name),
                            indUnit.HWCoilFailNum1);
                    } else if (SolFlag == -2) {
                        if (indUnit.HWCoilFailNum2 == 0) {
                            ShowWarningMessage(state,
                                               std::format("SimFourPipeIndUnit: Hot water coil control failed (maximum flow limits) for {}=\"{}\"",
                                                           indUnit.UnitType,
                                                           indUnit.Name));
                            ShowContinueErrorTimeStamp(state, "");
                            ShowContinueError(state, "...Bad hot water maximum flow rate limits");
                            ShowContinueError(state, std::format("...Given minimum water flow rate={:#G} kg/s", MinHotWaterFlow));
                            ShowContinueError(state, std::format("...Given maximum water flow rate={:#G} kg/s", MaxHotWaterFlow));
                        }
                        ShowRecurringWarningErrorAtEnd(state,
                                                       "SimFourPipeIndUnit: Hot water coil control failed (flow limits) for " + indUnit.UnitType +
                                                           "=\"" + indUnit.Name + "\"",
                                                       indUnit.HWCoilFailNum2,
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
                    ErrTolerance = indUnit.ColdControlOffset;
                    auto f = // (AUTO_OK_LAMBDA)
                        [&state, IUNum, FirstHVACIteration, ZoneNodeNum, MinHotWaterFlow, QToCoolSetPt, QPriOnly, PowerMet](Real64 const CWFlow) {
                            Real64 UnitOutput;
                            CalcFourPipeIndUnit(state, IUNum, FirstHVACIteration, ZoneNodeNum, MinHotWaterFlow, CWFlow, UnitOutput);
                            return (QToCoolSetPt - UnitOutput) / (PowerMet - QPriOnly);
                        };
                    General::SolveRoot(state, ErrTolerance, SolveMaxIter, SolFlag, CWFlow, f, MinColdWaterFlow, MaxColdWaterFlow);
                    if (SolFlag == -1) {
                        if (indUnit.CWCoilFailNum1 == 0) {
                            ShowWarningMessage(
                                state,
                                std::format("SimFourPipeIndUnit: Cold water coil control failed for {}=\"{}\"", indUnit.UnitType, indUnit.Name));
                            ShowContinueErrorTimeStamp(state, "");
                            ShowContinueError(state,
                                              std::format("  Iteration limit [{}] exceeded in calculating cold water mass flow rate", SolveMaxIter));
                        }
                        ShowRecurringWarningErrorAtEnd(
                            state,
                            std::format("SimFourPipeIndUnit: Cold water coil control failed (iteration limit [{}]) for {}=\"{}",
                                        SolveMaxIter,
                                        indUnit.UnitType,
                                        indUnit.Name),
                            indUnit.CWCoilFailNum1);
                    } else if (SolFlag == -2) {
                        if (indUnit.CWCoilFailNum2 == 0) {
                            ShowWarningMessage(state,
                                               std::format("SimFourPipeIndUnit: Cold water coil control failed (maximum flow limits) for {}=\"{}\"",
                                                           indUnit.UnitType,
                                                           indUnit.Name));
                            ShowContinueErrorTimeStamp(state, "");
                            ShowContinueError(state, "...Bad cold water maximum flow rate limits");
                            ShowContinueError(state, std::format("...Given minimum water flow rate={:#G} kg/s", MinColdWaterFlow));
                            ShowContinueError(state, std::format("...Given maximum water flow rate={:#G} kg/s", MaxColdWaterFlow));
                        }
                        ShowRecurringWarningErrorAtEnd(state,
                                                       "SimFourPipeIndUnit: Cold water coil control failed (flow limits) for " + indUnit.UnitType +
                                                           "=\"" + indUnit.Name + "\"",
                                                       indUnit.CWCoilFailNum2,
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
        state.dataLoopNodes->Node(OutletNode).MassFlowRateMax = indUnit.MaxTotAirMassFlow;

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

        auto &indUnit = state.dataHVACSingleDuctInduc->IndUnit(IUNum);

        PriNode = indUnit.PriAirInNode;
        OutletNode = indUnit.OutAirNode;
        PriAirMassFlow = state.dataLoopNodes->Node(PriNode).MassFlowRateMaxAvail;
        InducRat = indUnit.InducRatio;
        SecAirMassFlow = InducRat * PriAirMassFlow;
        TotAirMassFlow = PriAirMassFlow + SecAirMassFlow;
        HotControlNode = indUnit.HWControlNode;
        HWOutletNode = DataPlant::CompData::getPlantComponent(state, indUnit.HWPlantLoc).NodeNumOut;

        ColdControlNode = indUnit.CWControlNode;
        CWOutletNode = DataPlant::CompData::getPlantComponent(state, indUnit.CWPlantLoc).NodeNumOut;

        mdotHW = HWFlow;
        PlantUtilities::SetComponentFlowRate(state, mdotHW, HotControlNode, HWOutletNode, indUnit.HWPlantLoc);

        //  Node(HotControlNode)%MassFlowRate = HWFlow

        mdotCW = CWFlow;
        PlantUtilities::SetComponentFlowRate(state, mdotCW, ColdControlNode, CWOutletNode, indUnit.CWPlantLoc);
        //  Node(ColdControlNode)%MassFlowRate = CWFlow

        WaterCoils::SimulateWaterCoilComponents(state, indUnit.HCoil, FirstHVACIteration, indUnit.HCoil_Num);
        WaterCoils::SimulateWaterCoilComponents(state, indUnit.CCoil, FirstHVACIteration, indUnit.CCoil_Num);
        MixerComponent::SimAirMixer(state, indUnit.MixerName, indUnit.Mixer_Num);
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
